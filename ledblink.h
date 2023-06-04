#ifndef ledblink_h
#define ledblink_h
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

// use: Blinkred(0, 1000);
// change 0 to Led number that should be used in the stripe
// change 1000 to the delay Time the Blink should have

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
// Functions

// Blinkgreen
void Blinkgreen(int wich_led, int blink_delay)
{
  strip.setPixelColor(wich_led, 0, 20, 0);
  strip.show();
  delay(1000);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Light Green On");
  }
  else
  {
  }
  strip.setPixelColor(wich_led, 0, 0, 0);
  strip.show();
  delay(1000);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Light Green Off");
  }
  else
  {
  }
}

//Blinkyellow
void Blinkyellow(int wich_led, int blink_delay)
{
  strip.setPixelColor(wich_led, 255, 234, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Light Yellow On");
  }
  else
  {
  }
  strip.setPixelColor(wich_led, 0, 0, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Light Yellow Off");
  }
  else
  {
  }
}

//Blinkred
void Blinkred(int wich_led, int blink_delay)
{
  strip.setPixelColor(wich_led, 255, 0, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Red On");
  }
  else
  {
  }
  strip.setPixelColor(wich_led, 0, 0, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Red Off");
  }
  else
  {
  }
}

//Blinkblue
void Blinkblue(int wich_led, int blink_delay)
{
  strip.setPixelColor(wich_led, 0, 0, 255);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Blue On");
  }
  else
  {
  }
  strip.setPixelColor(wich_led, 0, 0, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Blue Off");
  }
  else
  {
  }
}

//Blinkorange
void Blinkorange(int wich_led, int blink_delay)
{
  strip.setPixelColor(wich_led, 251, 129, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Orange On");
  }
  else
  {
  }
  strip.setPixelColor(wich_led, 0, 0, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> Orange Off");
  }
  else
  {
  }
}

//Blinkwhite
void Blinkwhite(int wich_led, int blink_delay)
{
  strip.setPixelColor(wich_led, 255, 255, 255);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> White On");
  }
  else
  {
  }
  strip.setPixelColor(wich_led, 0, 0, 0);
  strip.show();
  delay(blink_delay);
  if (Debug_LED_Console == true)
  {
    Serial.println("Neopixel -> Blink -> White Off");
  }
  else
  {
  }
}

#endif