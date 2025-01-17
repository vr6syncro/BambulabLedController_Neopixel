#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <cstring>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "eeprom_utils.h"
#include "variables.h"
#include "html.h"
#include <Adafruit_NeoPixel.h>
#include "handle.h"
#include "effects.h"

bool rawdata = false;

const char* wifiname = "Bambulab Led Controller Neopixel";
const char* setuppage = html_setuppage;
const char* finishedpage = html_finishpage;

char Printerip[Max_ipLength + 1] = "";
char Printercode[Max_accessCode + 1] = "";
char PrinterID[Max_DeviceId + 1] = "";
char EspPassword[Max_EspPassword + 1] = "";
char DeviceName[20];

ESP8266WebServer server(80);
IPAddress apIP(192, 168, 1, 1);

WiFiClientSecure WiFiClient;
WiFiManager wifiManager;
PubSubClient mqttClient(WiFiClient);

char* generateRandomString(int length) {
  static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  int charsetLength = strlen(charset);

  char* randomString = new char[length + 1];
  for (int i = 0; i < length; i++) {
    int randomIndex = random(0, charsetLength);
    randomString[i] = charset[randomIndex];
  }
  randomString[length] = '\0';

  return randomString;
}

/* Some Debugging Stuff
change Neopixel Brightness with brightness=xxx (0 - 255)
change currentstage with currentstage=x (-1 - 21), will be overwritten on next mqtt loop!
turn on off the rawdata with rawdata=true/false
*/

void handleSerialInput() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Check if the input matches the brightness command
    if (input.startsWith("brightness=")) {
      String brightnessStr = input.substring(11);
      int brightness = brightnessStr.toInt();

      // Adjust brightness value if needed
      if (brightness < 0) brightness = 0;
      if (brightness > 255) brightness = 255;

      strip.setBrightness(brightness);
      strip.show();

      Serial.print("LED brightness set to ");
      Serial.println(brightness);
    }
    else if (input.startsWith("currentstage=")) {
      String stageStr = input.substring(13);
      int stage = stageStr.toInt();

      CurrentStage = stage;

      Serial.print("Current stage set to ");
      Serial.println(CurrentStage);
    }
    else if (input == "rawdata=true") {
      rawdata = true;
      Serial.println("JSON data output is enabled.");
    }
    else if (input == "rawdata=false") {
      rawdata = false;
      Serial.println("JSON data output is disabled.");
    }
  }
}

void replaceSubstring(char* string, const char* substring, const char* newSubstring) {
  char* substringStart = strstr(string, substring);
  if (substringStart) {
    char* substringEnd = substringStart + strlen(substring);
    memmove(substringStart + strlen(newSubstring), substringEnd, strlen(substringEnd) + 1);
    memcpy(substringStart, newSubstring, strlen(newSubstring));
  }
}

void handleSetTemperature() {
  if (!server.hasArg("api_key")) {
    return server.send(400, "text/plain", "Missing API key parameter.");
  };
  char shortened_key[7];
  strncpy(shortened_key, EspPassword, 7);
  shortened_key[7] = '\0';
  char received_api_key[8];

  server.arg("api_key").toCharArray(received_api_key, 8);
  if (!strcmp(received_api_key, shortened_key) == 0) {
    return server.send(401, "text/plain", "Unauthorized access.");
  }
  char mqttTopic[50];
  strcpy(mqttTopic, "device/");
  strcat(mqttTopic, PrinterID);
  strcat(mqttTopic, "/request");
  if (server.hasArg("bedtemp")) {
    float bedtemp = server.arg("bedtemp").toFloat();
    String message = "{\"print\":{\"sequence_id\":\"2026\",\"command\":\"gcode_line\",\"param\":\"M140 S" + String(bedtemp) + "\\n\"}}";
    mqttClient.publish(mqttTopic, message.c_str());
  }
  if (server.hasArg("nozzletemp")) {
    float bedtemp = server.arg("nozzletemp").toFloat();
    String message = "{\"print\":{\"sequence_id\":\"2026\",\"command\":\"gcode_line\",\"param\":\"M104 S" + String(bedtemp) + "\\n\"}}";
    mqttClient.publish(mqttTopic, message.c_str());
  }
  return server.send(200, "text/plain", "Ok");
}

void handleSetupRoot() { //Function to handle the setuppage
  if (!server.authenticate("BLLC", EspPassword)) {
    return server.requestAuthentication();
  }
  replaceSubstring((char*)setuppage, "ipinputvalue", Printerip);
  replaceSubstring((char*)setuppage, "idinputvalue", PrinterID);
  replaceSubstring((char*)setuppage, "codeinputvalue", Printercode);
  server.send(200, "text/html", setuppage);
}

void SetupWebpage() { //Function to start webpage system
  Serial.println(F("Starting Web server"));
  server.on("/", handleSetupRoot);
  server.on("/setupmqtt", savemqttdata);
  // server.on("/settemp", handleSetTemperature);
  server.begin();
  Serial.println(F("Web server started"));
}

void savemqttdata() {
  char iparg[Max_ipLength + 1];
  char codearg[Max_accessCode + 1];
  char idarg[Max_DeviceId + 1];

  // Copy the arguments from server to char arrays
  server.arg("ip").toCharArray(iparg, Max_ipLength + 1);
  server.arg("code").toCharArray(codearg, Max_accessCode + 1);
  server.arg("id").toCharArray(idarg, Max_DeviceId + 1);

  if (strlen(iparg) == 0 || strlen(codearg) == 0 || strlen(idarg) == 0) {
    return handleSetupRoot();
  }

  server.send(200, "text/html", finishedpage);

  Serial.println(F("Printer IP:"));
  Serial.println(iparg);
  Serial.println(F("Printer Code:"));
  Serial.println(codearg);
  Serial.println(F("Printer Id:"));
  Serial.println(idarg);

  writeToEEPROM(iparg, codearg, idarg, EspPassword);
  delay(1000); //wait for page to load
  ESP.restart();
}


void PrinterCallback(char* topic, byte* payload, unsigned int length) { //Function to handle the MQTT Data from the mqtt broker
  Serial.print(F("Message arrived in topic: "));
  Serial.println(topic);
  Serial.print(F("Message Length: "));
  Serial.println(length);
  Serial.print(F("Message:"));

  StaticJsonDocument<10000> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  if (!doc.containsKey("print")) {
    return;
  }

  if (rawdata) {
    Serial.println(F("===== JSON Data ====="));
    serializeJsonPretty(doc, Serial);
    Serial.println(F("======================"));
  }

  if (doc["print"].containsKey("stg_cur")) {
    CurrentStage = doc["print"]["stg_cur"];
  }

  Serial.print(F("stg_cur: "));
  Serial.println(CurrentStage);

  if (doc["print"].containsKey("gcode_state")) {
    if (doc["print"]["gcode_state"] == "FINISH" && finishstartms <= 0) {
      finishstartms = millis();
    } else if (doc["print"]["gcode_state"] != "FINISH" && finishstartms > 0) {
      finishstartms = 0;
    }
  }

  if (doc["print"].containsKey("hms")) {
    hasHMSerror = false;
    for (const auto& hms : doc["print"]["hms"].as<JsonArray>()) {
      if (hms["code"] == 131073) {
        hasHMSerror = true;
      };
    }
  }

  Serial.print(F("HMS error: "));
  Serial.println(hasHMSerror);

  if (doc["print"].containsKey("lights_report")) {
    if (doc["print"]["lights_report"][0]["node"] == "chamber_light") {
      ledstate = doc["print"]["lights_report"][0]["mode"] == "on";
      Serial.print("Ledchanged: ");
      Serial.println(ledstate);
    }
  }

  Serial.print(F("cur_led: "));
  Serial.println(ledstate);


  Serial.println(F(" - - - - - - - - - - - -"));

  handleLed();
}

void setup() { // Setup function
  Serial.begin(115200);
  EEPROM.begin(512);

  //clearEEPROM();

  //  Neopixel initialize
  strip.begin();
  strip.clear();
  strip.setBrightness(LED_Brightness);
  strip.show();

  WiFiClient.setInsecure();
  mqttClient.setBufferSize(10000);

  if (wifiManager.getWiFiIsSaved()) wifiManager.setEnableConfigPortal(false);
  wifiManager.autoConnect(wifiname);

  WiFi.hostname("bambuledcontroller");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Failed to connect to WiFi, creating access point..."));
    wifiManager.setAPCallback([](WiFiManager * mgr) {
      Serial.println(F("Access point created, connect to:"));
      Serial.print(mgr->getConfigPortalSSID());
    });
    wifiManager.setConfigPortalTimeout(300);
    wifiManager.startConfigPortal(wifiname);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    Serial.println(F("Connecting to Wi-Fi..."));
  }

  readFromEEPROM(Printerip, Printercode, PrinterID, EspPassword);

  if (strchr(EspPassword, '#') == NULL) { //Isue with eeprom giving �, so adding a # to check if the eeprom is empty or not
    Serial.println(F("No Password has been set, Resetting"));
    memset(EspPassword, 0, Max_EspPassword);
    memset(Printercode, '_', Max_accessCode);
    memset(PrinterID, '_', Max_DeviceId);
    memset(Printerip, '_', Max_ipLength);
    char* newEspPassword = generateRandomString(Max_EspPassword - 1);
    strcat(newEspPassword, "#");
    strcat(EspPassword, newEspPassword);
    writeToEEPROM(Printerip, Printercode, PrinterID, EspPassword);
    readFromEEPROM(Printerip, Printercode, PrinterID, EspPassword); //This will auto clear the eeprom
  };

  Serial.print(F("Connected to WiFi, IP address: "));
  Serial.println(WiFi.localIP());
  Serial.println(F("-------------------------------------"));
  Serial.print(F("Head over to http://"));
  Serial.println(WiFi.localIP());
  Serial.print(F("Login Details User: BLLC, Password: "));
  Serial.println(String(EspPassword));
  Serial.println(F(" To configure the mqtt settings."));
  Serial.println(F("-------------------------------------"));

  SetupWebpage();

  strcpy(DeviceName, "ESP8266MQTT");
  char* randomString = generateRandomString(4);
  strcat(DeviceName, randomString);

  mqttClient.setServer(Printerip, 8883);
  mqttClient.setCallback(PrinterCallback);
}

void loop() { //Loop function
  server.handleClient();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Connection lost! Reconnecting..."));
    //wifiManager.autoConnect(wifiname);
    //Serial.println(F("Connected to WiFi!"));
    ESP.restart();
  }
  if (WiFi.status() == WL_CONNECTED && strlen(Printerip) > 0 && (lastmqttconnectionattempt <= 0 || millis() - lastmqttconnectionattempt >= 10000)) {
    if (!mqttClient.connected()) {

      Serial.print(F("Connecting with device name:"));
      Serial.println(DeviceName);
      Serial.println(F("Connecting to mqtt"));

      if (mqttClient.connect(DeviceName, "bblp", Printercode)) {
        Serial.println(F("Connected to MQTT"));
        Led_off();
        char mqttTopic[50];
        strcpy(mqttTopic, "device/");
        strcat(mqttTopic, PrinterID);
        strcat(mqttTopic, "/report");
        Serial.println("Topic: ");
        Serial.println(mqttTopic);
        mqttClient.subscribe(mqttTopic);
        lastmqttconnectionattempt;
      } else {
        Led_off();
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 10 seconds");
        lastmqttconnectionattempt = millis();
      }
    }
  }
  //Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
  mqttClient.loop();
  handleSerialInput(); // debug
}
