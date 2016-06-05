/* fastledstrandtestrig
 * 
 * Uses the FastLED library to quickly test a variety of types of LED strips.
 * User input via a potentiometer and a button.
 * Displays via I2C LCD.
 * Adapted from Adafruit's strandtest.
 * 
 * Liz Corson
 * 5 June 2016
 */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h> 
#include "FastLED.h"

#define POT_PIN A0
#define BUTTON_PIN 13
#define NUM_CHIPSETS 4
#define MAX_LEDS 144

#define DATA_PIN 6
#define CLOCK_PIN 12
#define ORIG_NUM_LEDS 144

int NUM_LEDS;

// Define the array of leds
CRGB leds[ORIG_NUM_LEDS];

//For checking for button presses via interrupts
volatile unsigned long buttonTime;
volatile byte buttonFlag = 0;

//Declare LCD - may be 0x27 instead
LiquidCrystal_I2C lcd(0x3F,16,2);

void setup() {
  //Set up inputs
  pinMode(POT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

  //Set up interrupt function
  Timer1.initialize(200000); // 200 ms
  Timer1.attachInterrupt(buttonCheck);  

  // SELECT CHIPSET
  lcd.init();                      // initialize the lcd 
  lcd.clear();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Choose Chipset");
  int chipsetNum;
  
  while (buttonFlag == 0) {
    int potval = analogRead(POT_PIN);
    lcd.setCursor(0,1);
    chipsetNum = map(potval,0,1023,1,NUM_CHIPSETS);

    switch (chipsetNum) {
      case 1:
        lcd.print("WS2811          ");
        break;
      case 2:
        lcd.print("WS2812          ");
        break;
      case 3:
        lcd.print("WS2812B         ");
        break;  
      case 4:
        lcd.print("Neopixel        ");
        break;  
    }
  }

  buttonFlag = 0;

  // SET NUM LEDS
  lcd.clear();
  lcd.setCursor(0,0);
  switch (chipsetNum) {  
    case 1:
      lcd.print("WS2811 LEDs");
      break;
    case 2:
      lcd.print("WS2812 LEDs");
      break;
    case 3:
      lcd.print("WS2812B LEDs");
      break;  
    case 4:
      lcd.print("Neopixel LEDs");
      break;  
  }
  
  int numLEDs;
  
  while (buttonFlag == 0) {
    int potval = analogRead(POT_PIN);
    lcd.setCursor(0,1);
    numLEDs = map(potval,0,1023,1,MAX_LEDS);
    lcd.setCursor(0,1);
    lcd.print("Count: ");
    lcd.print(numLEDs);  
    lcd.print("     ");
  }

  NUM_LEDS = numLEDs;

  buttonFlag = 0;

  //Initialize LEDs
  switch (chipsetNum) {  
    case 1:
      FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
      break;
    case 2:
      FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
      break;
    case 3:
      FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
      break;  
    case 4:
      FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
      break;  
  }
}

//check for button press
void buttonCheck() {
  int buttonRead = digitalRead(BUTTON_PIN);
  if (buttonRead == HIGH) {
    buttonFlag = 1; //set the flag that tells the loop() that a button was pushed
    buttonTime = millis();
  }
}

// LED animations!
void loop() {
  //Color wipes: ensure correct RGB vs GRB order
  lcd.setCursor(0,1);
  lcd.print("Color Wipe: Red ");
  colorWipe(255,0,0,50);
  lcd.setCursor(0,1);
  lcd.print("Color Wipe:Green");
  colorWipe(0,255,0,50);
  lcd.setCursor(0,1);
  lcd.print("Color Wipe: Blue");
  colorWipe(0,0,255,50);

  // Send a theater pixel chase in...
  lcd.setCursor(0,1);
  lcd.print("Thtr Chase:White");
  theaterChase(127, 127, 127, 50); // White
  lcd.setCursor(0,1);
  lcd.print("Thtr Chase: Red ");
  theaterChase(127, 0, 0, 50); // Red
  lcd.setCursor(0,1);
  lcd.print("Thtr Chase: Blue");
  theaterChase(0, 0, 127, 50); // Blue

  //Pretty rainbow effect!
  lcd.setCursor(0,1);
  lcd.print("Rainbow Cycle   ");
  rainbowCycle(20);   
}

void colorWipe(int r, int g, int b, int wait) {
    for(int i = 0; i < NUM_LEDS; i++ ){
      leds[i].setRGB(r,g,b);                              
      FastLED.show();
      delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(int r, int g, int b, int wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < NUM_LEDS; i=i+3) {
        leds[i+q].setRGB(r,g,b); //turn every third pixel on
      }
      FastLED.show();

      delay(wait);

      for (uint16_t i=0; i < NUM_LEDS; i=i+3) {
        leds[i+q].setRGB(0,0,0);
      }
    }
  }
}

void rainbowCycle(int SpeedDelay) {   
  byte *c;   
  uint16_t i, j;   
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel     
    for(i=0; i< NUM_LEDS; i++) {       
      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);       
      leds[i].setRGB(*c, *(c+1), *(c+2));
      //setPixel(i, *c, *(c+1), *(c+2));     
    }     
    FastLED.show();    
    delay(SpeedDelay);   
  } 
} 

byte * Wheel(byte WheelPos) {   
  static byte c[3];     
  if(WheelPos < 85) {    
    c[0]=WheelPos * 3;    
    c[1]=255 - WheelPos * 3;    
    c[2]=0;   
  } else if(WheelPos < 170) {    
    WheelPos -= 85;    
    c[0]=255 - WheelPos * 3;    
    c[1]=0;    
    c[2]=WheelPos * 3;   
  } else {    
    WheelPos -= 170;    
    c[0]=0;    
    c[1]=WheelPos * 3;    
    c[2]=255 - WheelPos * 3;   
  }   
  return c; 
}

//credit to Tweaking4All.com for Wheel and rainbowCycle (http://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/)
