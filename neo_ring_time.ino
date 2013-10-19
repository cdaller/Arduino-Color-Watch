#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "RTClib.h"

/* 

  Defines a "Color-Time-Watch" 
  
  this is licences under Apache License
  
  Written by Christof Dallermassl (christof (at) dallermassl (dot) at)

*/

/*
  Additional Idreas:
  * Leds fade every couple of seconds
  
*/

// ring properties
#define PIN 4
#define PIXEL 16

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel ring = Adafruit_NeoPixel(PIXEL, PIN, NEO_GRB + NEO_KHZ800);
RTC_DS1307 rtc;

uint32_t color;

const uint32_t RED = ring.Color(255, 0, 0);
const uint32_t GREEN = ring.Color(0, 255, 0);
const uint32_t BLUE = ring.Color(0, 0, 255);
const uint32_t MAGENTA = ring.Color(255, 0, 255);
const uint32_t YELLOW = ring.Color(255, 255, 0);
const uint32_t PINK = ring.Color(219,112,147);
const uint32_t SKY_BLUE = ring.Color(135,206,255);
const uint32_t MAIZE = ring.Color(128,158,10);
const uint32_t LAVENDER = ring.Color(88,2,163);
const uint32_t SEA_FOAM = ring.Color(32,178,170);
const uint32_t SPRING = ring.Color(102,205,0);
const uint32_t DARK_ORANGE = ring.Color(237,180,6);
const uint32_t ORANGE = ring.Color(237,120,6);
const uint32_t WHITE = ring.Color(255,255,255);
const uint32_t BLACK = ring.Color(0,0,0);

uint32_t colorArray[12];
uint8_t brightnessArray[] = { 
// 0h - 6h 
5, 5, 5, 5, 5, 10, 
// 7h - 12h
20, 30, 30, 30, 30, 30, 
// 13 - 18h
30, 30, 30, 30, 30, 30, 
// 19 - 23h
30, 30, 30, 20, 20, 10};

const uint32_t SECONDS_PER_LED = 60 * 60 / ring.numPixels(); // = 225sec/pixel for 16 pixels

// fading every minute
uint32_t currentColor[PIXEL];
long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  
  
  uint8_t index = 0;
  colorArray[index++] = YELLOW;
  colorArray[index++] = LAVENDER;
  colorArray[index++] = SPRING;
  colorArray[index++] = ORANGE;
  colorArray[index++] = GREEN;
  colorArray[index++] = SEA_FOAM;
  colorArray[index++] = RED;
  colorArray[index++] = MAIZE;
  colorArray[index++] = MAGENTA;
  colorArray[index++] = BLUE;
  colorArray[index++] = PINK;
  colorArray[index++] = SKY_BLUE;
  
  // rtc setup
  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  //rtc.adjust(DateTime(__DATE__, __TIME__));

  // ring setup
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'
  ring.setBrightness(brightnessArray[rtc.now().hour()]);
  // initial colors:
  for (index = 0; index < 12; index++) {
    ring.setPixelColor(15 - index, colorArray[index]);
    ring.show();
    delay(50);
  }
  delay(1000);
}

void loop() {
  setColor();
  delay(1000);
}

void setColor() {
  DateTime now = rtc.now();
  uint32_t colorIndex = now.hour();
  if (colorIndex >= 12) {
    colorIndex = colorIndex - 12;
  }
  Serial.print("Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.println();
   
  uint32_t colorIndex1 = colorIndex;
  uint32_t colorIndex2 = colorIndex + 1;
  if (colorIndex2 == 12) {
    colorIndex2 = 0;
  }
  Serial.print("Colorindex1: ");
  Serial.println(colorIndex1, DEC);
  Serial.print("Colorindex2: ");
  Serial.println(colorIndex2, DEC);
   
  uint32_t color1 = colorArray[colorIndex1];
  uint32_t color2 = colorArray[colorIndex2];      

  uint32_t seconds_since_full_hour = now.minute() * 60 + now.second();
  uint32_t minutesLeds = seconds_since_full_hour / SECONDS_PER_LED;
  Serial.print("Number of minute leds: ");
  Serial.println(minutesLeds, DEC);
  
  // color1 = this hour's color
  // color2 = next hour's color
  // pixels are indexed counter clockwise!
  for (int index = 0; index < ring.numPixels(); index++) {
    if (index < (ring.numPixels() - minutesLeds)) {
      currentColor[index] = color1;
    } else {
      currentColor[index] = color2;
    }
    ring.setPixelColor(index, currentColor[index]);
  }      

  // set brightness (darker during night)
  ring.setBrightness(brightnessArray[now.hour()]);
  ring.show();   

  // every minute, fade the leds to black and back again:
  if (millis() - lastMillis > 60000) { // every minute
    lastMillis = millis();
    Serial.print("Fading, lastMillis = ");
    Serial.println(lastMillis);
    // fade to black
    for (int stepCount = 0; stepCount < 100; stepCount++) {
      for (int index = 0; index < ring.numPixels(); index++) {
        uint32_t color = crossFadeColor(currentColor[index], 0x000000, 100, stepCount);
        ring.setPixelColor(index, color);
      }
      ring.show();
      delay(10);
    }
    // fade back to color:
    for (int stepCount = 0; stepCount < 100; stepCount++) {
      for (int index = 0; index < ring.numPixels(); index++) {
        uint32_t color = crossFadeColor(0x000000, currentColor[index], 100, stepCount);
        ring.setPixelColor(index, color);
      }
      ring.show();
      delay(10);
    }
  }  
  
}


uint32_t crossFadeColor(uint32_t fromColor, uint32_t toColor, int steps, int stepNumber) {
//  Serial.print("fromColor: ");
//  Serial.print(fromColor, HEX);
//  Serial.print(" toColor: ");
//  Serial.println(toColor, HEX);
  uint32_t red = crossFadeChannel(fromColor, toColor, steps, stepNumber, 0xff0000, 16);
  uint32_t green = crossFadeChannel(fromColor, toColor, steps, stepNumber, 0x00ff00, 8);
  uint32_t blue = crossFadeChannel(fromColor, toColor, steps, stepNumber, 0x0000ff, 0);
  
  uint32_t resultColor = red | green | blue;  
  return resultColor;
}

uint32_t crossFadeChannel(uint32_t fromColor, uint32_t toColor, int steps, int stepNumber, uint32_t mask, uint8_t bitShift) {
  uint8_t fromChannel = (fromColor & mask) >> bitShift;
  uint8_t toChannel = (toColor & mask) >> bitShift; 
  uint32_t resultChannel = crossFadeNumber(fromChannel, toChannel, steps, stepNumber);
  resultChannel = resultChannel << bitShift;
//  Serial.print("from channel :");
//  Serial.println(fromChannel, HEX);
//  Serial.print("to channel :");
//  Serial.println(toChannel, HEX);
//  Serial.print("result channel :");
//  Serial.println(resultChannel, HEX);
  return resultChannel;
}

int crossFadeNumber(int from, int to, int steps, int stepNumber) {
  int diff = to - from;
  int result = from + stepNumber * diff / steps;
//  Serial.print("result number :");
//  Serial.println(result, HEX);
  return result;
}

