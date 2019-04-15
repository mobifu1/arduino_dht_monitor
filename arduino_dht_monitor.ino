//----------------------------------------------------
//-----------------DHT-Klima-Monitor------------------
//----------------------------------------------------
//-----------------Aurduino Uno-Board-----------------
//----------------------------------------------------

#include "DHT.h"

#define DHTPIN_0 A5  // Digital pin connected to the DHT sensor
#define DHTPIN_1 10  // Digital pin connected to the DHT sensor

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
DHT dht_0(DHTPIN_0, DHTTYPE);
DHT dht_1(DHTPIN_1, DHTTYPE);

//-----------Timer------------------------------
#include <TimerOne.h>

//----------EEPROM------------------------------
#include <EEPROM.h>

//------- TFT DISPLAY --------------------------
// This sketch has been Refurbished by BUHOSOFT
// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.
//#include <Adafruit_GFX.h>    // Core graphics library

#include <Adafruit_TFTLCD.h> // Hardware-specific library

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).
// Assign human-readable names to some common 16-bit color values:

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFBE0
#define GRAY    0x7BEF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

int const table_length = 480;//max.240 values * 2bytes(Integer)
int table_index = 0;
int eeprom_start_temp_0 = 0; //Arduino Uno = 1024 EEprom Bytes
int eeprom_start_humy_0 = 500;
int eeprom_table_index = 1000;

bool start_measure = true;
int counter_0 = 0;
int counter_1 = 0;
bool heart_beat_flag = false;

float t_0;
float h_0;
float t_1;
float h_1;

int table_temp_1[240] = {};
int table_humy_1[240] = {};
//#########################################################################
//#########################################################################
void setup() {

  Serial.begin(9600);

  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(1);//1=90 2=180 3=270
  tft.fillScreen(BLACK);
  ScreenText(WHITE, 5, 10 , 2, F("DHT22-Sensor-Monitor"));
  ScreenText(WHITE, 5, 50 , 2, F("V0.2-Beta"));
  delay(3000);
  tft.fillScreen(BLACK);

  dht_0.begin();
  dht_1.begin();

  Timer1.initialize(1000000);   //1sec
  Timer1.attachInterrupt(count_to_start);

  table_index = read_eeprom_int(eeprom_table_index);
  if (table_index < 0 || table_index > table_length) table_index = 0;

}
//----------------------------------------------------------------------------------
void loop() {

  if (start_measure == true) {
    start_measure = false;
    tft.fillScreen(BLACK);
    display_scale();
    measure_dht();
    save_values();
    display_chart();
  }
}
//----------------------------------------------------------------------------------
void count_to_start() {

  counter_0++;//every second
  counter_1++;
  if (counter_0 == 1800) { //in seconds 5min=300sec / 30min=1800sec / 60min=3600sec
    counter_0 = 0;
    start_measure = true;
  }

  if (counter_1 == 30) {// 30 seconds
    counter_1 = 0;
    measure_dht();
  }

  heart_beat();
}
//----------------------------------------------------------------------------------
void heart_beat() {

  if (heart_beat_flag == true) {
    SetFilledCircle(WHITE , 310, 230, 4);
    heart_beat_flag = false;
  }
  else {
    SetFilledCircle(BLACK , 310, 230, 4);
    heart_beat_flag = true;
  }
}
//----------------------------------------------------------------------------------
void measure_dht() {

  t_0 = dht_0.readTemperature();  // Read temperature as Fahrenheit (isFahrenheit = true)
  h_0 = dht_0.readHumidity();     // Read temperature as Celsius (the default)

  display_values(0, int(t_0), int(h_0));

  //----------------------------------------------------

  t_1 = dht_1.readTemperature();  // Read temperature as Fahrenheit (isFahrenheit = true)
  h_1 = dht_1.readHumidity();     // Read temperature as Celsius (the default)

  display_values(1, int(t_1), int(h_1));
}
//--------------------------------------------------------------------------------------
void save_values() {

  int address;

  //------save values in to eeprom table---

  address = eeprom_start_temp_0 + table_index;
  if (t_0 > -40 && t_0 < 80) {
    write_eeprom_int(address, int(t_0));
  }
  else {
    write_eeprom_int(address, int(0));
  }

  address = eeprom_start_humy_0 + table_index;
  if (h_0 >= 0 && h_0 <= 100) {
    write_eeprom_int(address, int(h_0));
  }
  else {
    write_eeprom_int(address, int(0));
  }

  //------save values in to ram table---

  if (t_1 > -40 && t_1 < 80) {
    table_temp_1[table_index / 2] = int(t_1);
  }
  else {
    table_temp_1[table_index / 2] = int(0);
  }

  if (h_1 >= 0 && h_1 <= 100) {
    table_humy_1[table_index / 2] = int(h_1);
  }
  else {
    table_humy_1[table_index / 2] = int(0);
  }

  //-------------------------------------------------------

  table_index += 2;
  if (table_index == table_length) { //from 0 to length -1
    table_index = 0;
  }
  write_eeprom_int(eeprom_table_index, table_index);
}
//--------------------------------------------------------------------------------------
void display_values(int sensor, int t, int h) {

  if (sensor == 0) {

    SetFilledRect(BLACK , 260, 0, 59, 119);

    ScreenText(WHITE, 5, 10, 2 , F("ODG o"));

    if (t < 10) {
      ScreenText(ORANGE, 260, 55, 2 , String(int(t)) + F(" C"));
    }
    else {
      ScreenText(WHITE, 260, 55, 2 , String(int(t)) + F(" C"));
    }

    if (h > 30) {
      ScreenText(ORANGE, 260, 20, 2 , String(int(h)) + F(" %"));
    }
    else {
      ScreenText(WHITE, 260, 20, 2 , String(int(h)) + F(" %"));
    }
    float taupunkt_0 = taupunkt(t, h);
    ScreenText(WHITE, 260, 90, 1 , "TP:" + String(int(taupunkt_0)) + F(" C"));
  }

  //--------------------------------------------------------------------------

  if (sensor == 1) {

    SetFilledRect(BLACK , 260, 121, 59, 100);

    ScreenText(WHITE, 5, 130, 2 , F("ODG u"));

    if (t < 10) {
      ScreenText(ORANGE, 260, 175, 2 , String(int(t)) + F(" C"));
    }
    else {
      ScreenText(WHITE, 260, 175, 2 , String(int(t)) + F(" C"));
    }

    if (h > 30) {
      ScreenText(ORANGE, 260, 140, 2 , String(int(h)) + F(" %"));
    }
    else {
      ScreenText(WHITE, 260, 140, 2 , String(int(h)) + F(" %"));
    }
    float taupunkt_1 = taupunkt(t, h);
    ScreenText(WHITE, 260, 210, 1 , "TP:" + String(int(taupunkt_1)) + F(" C"));
  }
}
//--------------------------------------------------------------------------------------
void display_chart() {

  int value;
  int end_address;
  int counter;

  counter = 0;
  end_address = eeprom_start_temp_0 + table_length;
  for (int x = eeprom_start_temp_0; x < end_address; x += 2) {
    value = read_eeprom_int(x);
    SetPoint(RED, counter, 100 - value);//-10 to +60 °C
    //Serial.println(String(value) + "," + String(x) + "," + String(counter));
    counter++;
  }

  counter = 0;
  end_address = eeprom_start_humy_0 + table_length;
  for (int x = eeprom_start_humy_0; x < end_address; x += 2) {
    value = read_eeprom_int(x);
    SetPoint(BLUE, counter, 100 - value); // 10-100 %
    //Serial.println(String(value) + "," + String(x) + "," + String(counter));
    counter++;
  }

  //-------------------------------------------------------------

  for (int x = 0; x < (table_length / 2); x++) {
    value = table_temp_1[x];
    SetPoint(RED, x, 220 - value);//-10 to +60 °C
    //Serial.println(String(value) + "," + String(x));
  }

  for (int x = 0; x < (table_length / 2); x++) {
    value = table_humy_1[x];
    SetPoint(BLUE, x, 220 - value); // 10-100 %
    //Serial.println(String(value) + "," + String(x));
  }
}
//--------------------------------------------------------------------------------------
void display_scale() {

  SetLines(GRAY , table_index / 2  , 0, table_index / 2, 239);

  SetLines(WHITE , 240 , 0, 240, 239);
  SetLines(WHITE , 0 , 119, 319, 119);

  SetLines(WHITE , 240 , 100, 245, 100);//0
  SetLines(GRAY , 0 , 100, 239, 100);//0

  SetLines(WHITE , 240 , 75, 245, 75);//25
  SetLines(GRAY , 0 , 75, 239, 75);//25

  SetLines(WHITE , 240 , 50, 245, 50);//50
  SetLines(GRAY , 0 , 50, 239, 50);//50

  SetLines(WHITE , 240 , 25, 245, 25);//75
  SetLines(GRAY , 0 , 25, 239, 25);//75

  SetLines(WHITE , 240 , 220, 245, 220);//0
  SetLines(GRAY , 0 , 220, 239, 220);//0

  SetLines(WHITE , 240 , 195, 245, 195);//25
  SetLines(GRAY , 0 , 195, 239, 195);//25

  SetLines(WHITE , 240 , 170, 245, 170);//50
  SetLines(GRAY , 0 , 170, 239, 170);//50

  SetLines(WHITE , 240 , 145, 245, 145);//75
  SetLines(GRAY , 0 , 145, 239, 145);//75
}
//--------------------------------------------------------------------------------------
float taupunkt(float t, float h) {

  float taupunkt_temp;

  if (t > 0 ) {
    //Formel über 0°C
    taupunkt_temp = 243.12 * ((17.62 * t) / (243.12 + t) + log(h / 100)) / ((17.62 * 243.12) / (243.12 + t) - log(h / 100));
  }
  else {
    //Formel unter 0°C
    taupunkt_temp = 272.62 * ((22.46 * t) / (272.62 + t) + log(h / 100)) / ((22.46 * 272.62) / (272.62 + t) - log(h / 100));
  }

  return taupunkt_temp;
}
//----------------------------------------------
//--------------GRAFIK-ROUTINEN-----------------
//----------------------------------------------
void ScreenText(uint16_t color, int xtpos, int ytpos, int text_size , String text) {
  tft.setCursor(xtpos, ytpos);
  tft.setTextColor(color);
  tft.setTextSize(text_size);
  tft.println(text);
}

void SetLines(uint16_t color , int xl1pos, int yl1pos, int xl2pos, int yl2pos) {
  tft.drawLine(xl1pos, yl1pos, xl2pos, yl2pos, color);
}

void SetPoint(uint16_t color, int xppos, int yppos) {
  tft.drawPixel(xppos, yppos, color);
}

void SetRect(uint16_t color , int xr1pos, int yr1pos, int xr2width, int yr2hight) {
  tft.drawRect(xr1pos, yr1pos, xr2width, yr2hight, color);
}

void SetFilledRect(uint16_t color , int xr1pos, int yr1pos, int xr2width, int yr2hight) {
  tft.fillRect(xr1pos, yr1pos, xr2width, yr2hight, color);
}

void SetCircle(uint16_t color , int xcpos, int ycpos, int radius) {
  tft.drawCircle(xcpos, ycpos, radius, color);
}
void SetFilledCircle(uint16_t color , int xcpos, int ycpos, int radius) {
  tft.fillCircle(xcpos, ycpos, radius, color);
}
//----------------------------------------------
//----------EEPROM-ROUTINEN---------------------
//----------------------------------------------
void write_eeprom_string(int address, String value) {

  int len = value.length();
  len++;
  if (len < 16) {
    int address_end = address + len;
    char buf[len];
    byte count = 0;
    value.toCharArray(buf, len);
    for (int i = address ; i < address_end ; i++) {
      EEPROM.update(i, buf[count]);
      count++;
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void write_eeprom_int(int address, int value) {

  EEPROM.update(address, highByte(value));
  EEPROM.update(address + 1, lowByte(value));
}
//----------------------------------------------------------------------------------------------------------------------
void write_eeprom_byte(int address, byte value) {

  //EEPROM.write(address, value);
  EEPROM.update(address, value);
}
//----------------------------------------------------------------------------------------------------------------------
void write_eeprom_bool(int address, boolean value) {

  //EEPROM.write(address, value);
  if (value == true)EEPROM.update(address, 1);
  if (value == false)EEPROM.update(address, 0);
}
//----------------------------------------------------------------------------------------------------------------------
String read_eeprom_string(int address) {

  String value;
  byte count = 0;
  char buf[16];
  for (int i = address ; i < (address + 15) ; i++) {
    buf[count] = EEPROM.read(i);
    if (buf[count] == 0) break; //endmark of string
    value += buf[count];
    count++;
  }
  return value;
}
//----------------------------------------------------------------------------------------------------------------------
int read_eeprom_int(int address) {

  int value;
  int value_1;
  value = EEPROM.read(address); //highByte(value));
  value = value << 8;
  value_1 = EEPROM.read(address + 1); //lowByte(value));
  value = value + value_1;
  return value;
}
//----------------------------------------------------------------------------------------------------------------------
byte read_eeprom_byte(int address) {

  byte value;
  value = EEPROM.read(address);
  return value;
}
//----------------------------------------------------------------------------------------------------------------------
boolean read_eeprom_bool(int address) {

  byte value;
  boolean bool_value;
  value = EEPROM.read(address);
  if (value == 1)bool_value = true;
  if (value != 1)bool_value = false;
  return bool_value;
}
