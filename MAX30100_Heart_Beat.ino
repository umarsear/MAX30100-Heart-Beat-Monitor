// Sample implementation of the MAX30100 PulseOximeter
// Using the following module
// http://www.ebay.com/itm/-/391709438817?ssPageName=STRK:MESE:IT
// can not gaurantee if the app will work with other implementations of the module. 

#include "MAX30100_PulseOximeter.h"
#include <U8g2lib.h>
#include <Wire.h>

#define REPORTING_PERIOD_MS     500
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

const int numReadings=10;
float filterweight=0.5;
uint32_t tsLastReport = 0;
uint32_t last_beat=0;
int readIndex=0;
int average_beat=0;
int average_SpO2=0;
bool calculation_complete=false;
bool calculating=false;
bool initialized=false;
byte beat=0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
  show_beat();
  last_beat=millis();
}

void show_beat() 
{
  u8g2.setFont(u8g2_font_cursor_tf);
  u8g2.setCursor(8,10);
  if (beat==0) {
    u8g2.print("_");
    beat=1;
  } 
  else
  {
    u8g2.print("^");
    beat=0;
  }
  u8g2.sendBuffer();
}

void initial_display() 
{
  if (not initialized) 
  {
    u8g2.clearBuffer();
    show_beat();
    u8g2.setCursor(24,12);
    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf);
    u8g2.print("Place finger");  
    u8g2.setCursor(0,30);
    u8g2.print("on the sensor");
    u8g2.sendBuffer(); 
    initialized=true;
  }
}

void display_calculating(int j)
{
  if (not calculating) {
    u8g2.clearBuffer();
    calculating=true;
    initialized=false;
  }
  show_beat();
  u8g2.setCursor(24,12);
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf);
  u8g2.print("Measuring"); 
  u8g2.setCursor(0,30);
  for (int i=0;i<=j;i++) {
    u8g2.print(". ");
  }
  u8g2.sendBuffer();
}
void display_values()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf);
 
  u8g2.setCursor(65,12);  
  u8g2.print(average_beat);
  u8g2.print(" Bpm");
  u8g2.setCursor(0,30);
  u8g2.print("SpO2 ");
  u8g2.setCursor(65,30);  
  u8g2.print(average_SpO2);
  u8g2.print("%"); 
  u8g2.sendBuffer();
}

void calculate_average(int beat, int SpO2) 
{
  if (readIndex==numReadings) {
    calculation_complete=true;
    calculating=false;
    initialized=false;
    readIndex=0;
    display_values();
  }
  
  if (not calculation_complete and beat>30 and beat<220 and SpO2>50) {
    average_beat = filterweight * (beat) + (1 - filterweight ) * average_beat;
    average_SpO2 = filterweight * (SpO2) + (1 - filterweight ) * average_SpO2;
    readIndex++;
    display_calculating(readIndex);
  }
}

void setup()
{
    Serial.begin(115200);
    u8g2.begin();
    pox.begin();
    pox.setOnBeatDetectedCallback(onBeatDetected);
    initial_display();
}

void loop()
{
    // Make sure to call update as fast as possible
    pox.update();
    if ((millis() - tsLastReport > REPORTING_PERIOD_MS) and (not calculation_complete)) {
        calculate_average(pox.getHeartRate(),pox.getSpO2());
        tsLastReport = millis();
    }
    if ((millis()-last_beat>10000)) {
      calculation_complete=false;
      average_beat=0;
      average_SpO2=0;
      initial_display();
    }
}
