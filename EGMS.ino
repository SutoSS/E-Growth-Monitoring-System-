#include <HX711_ADC.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
//#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Keypad.h>
#include <FS.h>
#include <SD.h>
#include "math.h"

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTLN(x)  Serial.println (x)
#define DEBUG_PRINT(x)  Serial.print (x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif

#define TFT_CS_PIN         10
#define TFT_RST_PIN        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC_PIN         2
const int HX711_DOUT_PIN = 21; //mcu > HX711 dout pin
const int HX711_SCK_PIN = 22; //mcu > HX711 sck pin
const int BACKLIGHT_PIN = 4;
const int TRIG_PIN = 15;
const int ECHO_PIN = 35;


unsigned long lastTime, currentTime = 0;
int period = 500;

#define TEXT_COLOR        ST77XX_WHITE
#define HEADER_COLOR      ST77XX_YELLOW
#define HIGHLIGHT_COLOR   ST77XX_GREEN
#define BACKGROUND_COLOR  0x3A0E
#define BOX_COLOR         ST77XX_BLACK
#define ST77XX_BLACK      ST7735_BLACK
#define ST77XX_WHITE      ST7735_WHITE
#define ST77XX_YELLOW     ST7735_YELLOW
#define ST77XX_GREEN      ST7735_GREEN
#define ST77XX_BLUE       ST7735_BLUE
#define ST77XX_RED        ST7735_RED

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int dutyCycle;

// defines variables
long duration;
float distances;
float lastDistance, distanceNow, distanceSum, distanceMean = 0;
float dataLast , dataNow, dataLoadCell = 0;
float calValueDistanceSensor;

int ulangUkur=0;
int state = 0;
boolean flagKosongkanScreen, flagAnakTimbangan, flagKeyRead, flagShowCursor, flagDisplayKalibrasi, flagReadLoadCell = 0;
boolean flagInputUsiaScreen, flagInputJenisKelaminScreen, flagPersiapanScreen, flagPengukuranScreen, flagReadUltrasonic = 0;
boolean flagDisplayKalibrasiJarak, flagDisplayKalibrasiBerat, flagDisplayKalibrasiBerat2 = 0;
boolean FLAG_SD_AVAILABLE = 1;
char keyNow, keyLast;
char keyInNow, keyInLast;
int k;
char dataPosyandu[200];



String noID;
String loadRef;
String usiaTahun;
String usiaBulan;
int noIDInt;
int usiaTahunInt;
int usiaBulanInt;
String jenisKelamin, klasifikasiTB;
int klasifikasiTinggi;
int klasifikasiBerat;
bool FLAG_TARE = 0;



String klasifikasiBB;

int cursorPosStartX, cursorPosStartY, charPosStartX, charPosStartY, cursorNum, maxCursor;
bool cursorState = 0;

/*-------------------
  //Keypad Init
  --------------------*/
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads

//old keypad
//char hexaKeys[ROWS][COLS] = {
//  {'D', '#', '0', '*'},
//  {'C', '9', '8', '7'},
//  {'B', '6', '5', '4'},
//  {'A', '3', '2', '1'}
//};
//new keypad
char hexaKeys[ROWS][COLS] = {
  {'1', '4', '7', '*'},
  {'2', '5', '8', '0'},
  {'3', '6', '9', '#'},
  {'A', 'B', 'C', 'D'}
};
byte rowPins[ROWS] = {12, 14, 27, 26}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {25, 33, 32, 13}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
//String keyNow, keyLast;

typedef struct
{
  int tahun;
  int bulan;
  float awal;
  float stunted;
  float pendek;
  float normal;
  float tinggi;
  float sangatTinggi;
} TinggiLaki;

TinggiLaki dataTabelTinggiLaki[61] = {
  {0, 0  , 44.2,  46.1  , 48.0 , 51.8 , 53.7 , 55.6 },
  {0, 1 , 48.9,  50.8  , 52.8 , 56.7 , 58.6 , 60.6 },
  {0, 2 , 52.4,  54.4  , 56.4 , 60.4 , 62.4 , 64.4 },
  {0, 3 , 55.3,  57.3  , 59.4 , 63.5 , 65.5 , 67.6 },
  {0, 4 , 57.6,  59.7  , 61.8 , 66.0 , 68.0 , 70.1 },
  {0, 5 , 59.6,  61.7  , 63.8 , 68.0 , 70.1 , 72.2 },
  {0, 6 , 61.2,  63.3  , 65.5 , 69.8 , 71.9 , 74.0 },
  {0, 7 , 62.7,  64.8  , 67.0 , 71.3 , 73.5 , 75.7 },
  {0, 8 , 64.0,  66.2  , 68.4 , 72.8 , 75.0 , 77.2 },
  {0, 9 , 65.2,  67.5  , 69.7 , 74.2 , 76.5 , 78.7 },
  {0, 10  , 66.4,  68.7  , 71.0 , 75.6 , 77.9 , 80.1 },
  {0, 11  , 67.6,  69.9  , 72.2 , 76.9 , 79.2 , 81.5 },
  {0, 12  , 68.6,  71.0  , 73.4 , 78.1 , 80.5 , 82.9 },
  {1, 1 , 69.6,  72.1    , 74.5 , 79.3 , 81.8 , 84.2 },
  {1, 2 , 70.6,  73.1    , 75.6 , 80.5 , 83.0 , 85.5 },
  {1, 3 , 71.6,  74.1    , 76.6 , 81.7 , 84.2 , 86.7 },
  {1, 4 , 72.5,  75.0    , 77.6 , 82.8 , 85.4 , 88.0 },
  {1, 5 , 73.3,  76.0    , 78.6 , 83.9 , 86.5 , 89.2 },
  {1, 6 , 74.2,  76.9    , 79.6 , 85.0 , 87.7 , 90.4 },
  {1, 7 , 75.0,  77.7    , 80.5 , 86.0 , 88.8 , 91.5 },
  {1, 8 , 75.8,  78.6    , 81.4 , 87.0 , 89.8 , 92.6 },
  {1, 9 , 76.5,  79.4    , 82.3 , 88.0 , 90.9 , 93.8 },
  {1, 10  , 77.2,  80.2    , 83.1 , 89.0 , 91.9 , 94.9 },
  {1, 11  , 78.0,  81.0    , 83.9 , 89.9 , 92.9 , 95.9 },
  {2, 0 , 78.0,  81.0  , 84.1 , 90.2 , 93.2 , 96.3 },
  {2, 1 , 78.6,  81.7  , 84.9 , 91.1 , 94.2 , 97.3 },
  {2, 2 , 79.3,  82.5  , 85.6 , 92.0 , 95.2 , 98.3 },
  {2, 3 , 79.9,  83.1  , 86.4 , 92.9 , 96.1 , 99.3 },
  {2, 4 , 80.5,  83.8  , 87.1 , 93.7 , 97.0 , 100.3},
  {2, 5 , 81.1,  84.5  , 87.8 , 94.5 , 97.9 , 101.2},
  {2, 6 , 81.7,  85.1  , 88.5 , 95.3 , 98.7 , 102.1},
  {2, 7 , 82.3,  85.7  , 89.2 , 96.1 , 99.6 , 103.0},
  {2, 8 , 82.8,  86.4  , 89.9 , 96.9 , 100.4  , 103.9},
  {2, 9 , 83.4,  86.9  , 90.5 , 97.6 , 101.2  , 104.8},
  {2, 10  , 83.9,  87.5  , 91.1 , 98.4 , 102.0  , 105.6},
  {2, 11  , 84.4,  88.1  , 91.8 , 99.1 , 102.7  , 106.4},
  {3, 0 , 85.0,  88.7  , 92.4 , 99.8 , 103.5  , 107.2},
  {3, 1 , 85.5,  89.2  , 93.0 , 100.5  , 104.2  , 108.0},
  {3, 2 , 86.0,  89.8  , 93.6 , 101.2  , 105.0  , 108.0},
  {3, 3 , 86.5,  90.3  , 94.2 , 101.8  , 105.7  , 109.5},
  {3, 4 , 87.0,  90.9  , 94.7 , 102.5  , 106.4  , 110.3},
  {3, 5 , 87.5,  91.4  , 95.3 , 103.2  , 107.1  , 111.0},
  {3, 6 , 88.0,  91.9  , 95.9 , 103.8  , 107.8  , 111.7},
  {3, 7 , 88.4,  92.4  , 96.4 , 104.5  , 108.5  , 112.5},
  {3, 8 , 88.9,  93.0  , 97.0 , 105.1  , 109.1  , 113.2},
  {3, 9 , 89.4,  93.5  , 97.5 , 105.7  , 109.8  , 113.9},
  {3, 10  , 89.8,  94.0  , 98.1 , 106.3  , 110.4  , 114.6},
  {3, 11  , 90.3,  94.4  , 98.6 , 106.9  , 111.1  , 115.2},
  {4, 0 , 90.7,  94.9  , 99.1 , 107.5  , 111.7  , 115.9},
  {4, 1 , 91.2,  95.4  , 99.7 , 108.1  , 112.4  , 116.6},
  {4, 2 , 91.6,  95.9  , 100.2  , 108.7  , 113.0  , 117.3},
  {4, 3 , 92.1,  96.4  , 100.7  , 109.3  , 113.6  , 117.9},
  {4, 4 , 92.5,  96.9  , 101.2  , 109.9  , 114.2  , 118.6},
  {4, 5 , 93.0,  97.4  , 101.7  , 110.5  , 114.9  , 119.2},
  {4, 6 , 93.4,  97.8  , 102.3  , 111.1  , 115.5  , 119.9},
  {4, 7 , 93.9,  98.3  , 102.8  , 111.7  , 116.1  , 120.6},
  {4, 8 , 94.3,  98.8  , 103.3  , 112.3  , 116.7  , 121.2},
  {4, 9 , 94.7,  99.3  , 103.8  , 112.8  , 117.4  , 121.9},
  {4, 10  , 95.2,  99.7  , 104.3  , 113.4  , 118.0  , 122.6},
  {4, 11  , 95.6,  100.2 , 104.8  , 114.0  , 118.6  , 123.2},
  {5, 0 , 96.1,  100.7 , 105.3  , 114.6  , 119.2  , 123.9}
};

typedef struct
{
  int tahun;
  int bulan;
  float awal;
  float kurangGizi;
  float kurus;
  float normal;
  float gemuk;
  float obesitas;
} BeratLaki;

BeratLaki dataTabelBeratLaki[61] = {
  {0  , 0    , 2.1    , 2.5    , 2.9    , 3.9    , 4.4    , 5.0 },
  {0  , 1    , 2.9    , 3.4    , 3.9    , 5.1    , 5.8    , 6.6 },
  {0  , 2    , 3.8    , 4.3    , 4.9    , 6.3    , 7.1    , 8.0 },
  {0  , 3    , 4.4    , 5.0    , 5.7    , 7.2    , 8.0    , 9.0 },
  {0  , 4    , 4.9    , 5.6    , 6.2    , 7.8    , 8.7    , 9.7 },
  {0  , 5    , 5.3    , 6.0    , 6.7    , 8.4    , 9.3    , 10.4},
  {0  , 6    , 5.7    , 6.4    , 7.1    , 8.8    , 9.8    , 10.9},
  {0  , 7    , 5.9    , 6.7    , 7.4    , 9.2    , 10.3   , 11.4},
  {0  , 8    , 6.2    , 6.9    , 7.7    , 9.6    , 10.7   , 11.9},
  {0  , 9    , 6.4    , 7.1    , 8.0    , 9.9    , 11.0   , 12.3},
  {0  , 10   , 6.6    , 7.4    , 8.2    , 10.2   , 11.4   , 12.7},
  {0  , 11   , 6.8    , 7.6    , 8.4    , 10.5   , 11.7   , 13.0},
  {0  , 12   , 6.9    , 7.7    , 8.6    , 10.8   , 12.0   , 13.3},
  {1  , 1    , 7.1    , 7.9    , 8.8    , 11.0   , 12.3   , 13.7},
  {1  , 2    , 7.2    , 8.1    , 9.0    , 11.3   , 12.6   , 14.0},
  {1  , 3    , 7.4    , 8.3    , 9.2    , 11.5   , 12.8   , 14.3},
  {1  , 4    , 7.5    , 8.4    , 9.4    , 11.7   , 13.1   , 14.6},
  {1  , 5    , 7.7    , 8.6    , 9.6    , 12.0   , 13.4   , 14.9},
  {1  , 6    , 7.8    , 8.8    , 9.8    , 12.2   , 13.7   , 15.3},
  {1  , 7    , 8.0    , 8.9    , 10.0   , 12.5   , 13.9   , 15.6},
  {1  , 8    , 8.1    , 9.1    , 10.1   , 12.7   , 14.2   , 15.9},
  {1  , 9    , 8.2    , 9.2    , 10.3   , 12.9   , 14.5   , 16.2},
  {1  , 10   , 8.4    , 9.4    , 10.5   , 13.2   , 14.7   , 16.5},
  {1  , 11   , 8.5    , 9.5    , 10.7   , 13.4   , 15.0   , 16.8},
  {2  , 0    , 8.6    , 9.7    , 10.8   , 13.6   , 15.3   , 17.1},
  {2  , 1    , 8.8    , 9.8    , 11.0   , 13.9   , 15.5   , 17.5},
  {2  , 2    , 8.9    , 10.0   , 11.2   , 14.1   , 15.8   , 17.8},
  {2  , 3    , 9.0    , 10.1   , 11.3   , 14.3   , 16.1   , 18.1},
  {2  , 4    , 9.1    , 10.2   , 11.5   , 14.5   , 16.3   , 18.4},
  {2  , 5    , 9.2    , 10.4   , 11.7   , 14.8   , 16.6   , 18.7},
  {2  , 6    , 9.4    , 10.5   , 11.8   , 15.0   , 16.9   , 19.0},
  {2  , 7    , 9.5    , 10.7   , 12.0   , 15.2   , 17.1   , 19.3},
  {2  , 8    , 9.6    , 10.8   , 12.1   , 15.4   , 17.4   , 19.6},
  {2  , 9    , 9.7    , 10.9   , 12.3   , 15.6   , 17.6   , 19.9},
  {2  , 10   , 9.8    , 11.0   , 12.4   , 15.8   , 17.8   , 20.2},
  {2  , 11   , 9.9    , 11.2   , 12.6   , 16.0   , 18.1   , 20.4},
  {3  , 0    , 10.0   , 11.3   , 12.7   , 16.2   , 18.3   , 20.7},
  {3  , 1    , 10.1   , 11.4   , 12.9   , 16.4   , 18.6   , 21.0},
  {3  , 2    , 10.2   , 11.5   , 13.0   , 16.6   , 18.8   , 21.3},
  {3  , 3    , 10.3   , 11.6   , 13.1   , 16.8   , 19.0   , 21.6},
  {3  , 4    , 10.4   , 11.8   , 13.3   , 17.0   , 19.3   , 21.9},
  {3  , 5    , 10.5   , 11.9   , 13.4   , 17.2   , 19.5   , 22.1},
  {3  , 6    , 10.6   , 12.0   , 13.6   , 17.4   , 19.7   , 22.4},
  {3  , 7    , 10.7   , 12.1   , 13.7   , 17.6   , 20.0   , 22.7},
  {3  , 8    , 10.8   , 12.2   , 13.8   , 17.8   , 20.2   , 23.0},
  {3  , 9    , 10.9   , 12.4   , 14.0   , 18.0   , 20.5   , 23.3},
  {3  , 10   , 11.0   , 12.5   , 14.1   , 18.2   , 20.7   , 23.6},
  {3  , 11   , 11.1   , 12.6   , 14.3   , 18.4   , 20.9   , 23.9},
  {4  , 0    , 11.2   , 12.7   , 14.4   , 18.6   , 21.2   , 24.2},
  {4  , 1    , 11.3   , 12.8   , 14.5   , 18.8   , 21.4   , 24.5},
  {4  , 2    , 11.4   , 12.9   , 14.7   , 19.0   , 21.7   , 24.8},
  {4  , 3    , 11.5   , 13.1   , 14.8   , 19.2   , 21.9   , 25.1},
  {4  , 4    , 11.6   , 13.2   , 15.0   , 19.4   , 22.2   , 25.4},
  {4  , 5    , 11.7   , 13.3   , 15.1   , 19.6   , 22.4   , 25.7},
  {4  , 6    , 11.8   , 13.4   , 15.2   , 19.8   , 22.7   , 26.0},
  {4  , 7    , 11.9   , 13.5   , 15.4   , 20.0   , 22.9   , 26.3},
  {4  , 8    , 12.0   , 13.6   , 15.5   , 20.2   , 23.2   , 26.6},
  {4  , 9    , 12.1   , 13.7   , 15.6   , 20.4   , 23.4   , 26.9},
  {4  , 10   , 12.2   , 13.8   , 15.8   , 20.6   , 23.7   , 27.2},
  {4  , 11   , 12.3   , 14.0   , 15.9   , 20.8   , 23.9   , 27.6},
  {5  , 0    , 12.4   , 14.1   , 16.0   , 21.0   , 24.2   , 27.9}
};

typedef struct
{
  int tahun;
  int bulan;
  float awal;
  float stunted;
  float pendek;
  float normal;
  float tinggi;
  float sangatTinggi;
} TinggiPerempuan;

TinggiPerempuan dataTabelTinggiPerempuan[61] = {
  {0  , 0    , 43.6   , 45.4   , 47.3   , 51.0   , 52.9   , 54.7 },
  {0  , 1    , 47.8   , 49.8   , 51.7   , 55.6   , 57.6   , 59.5 },
  {0  , 2    , 51.0   , 53.0   , 55.0   , 59.1   , 61.1   , 63.2 },
  {0  , 3    , 53.5   , 55.6   , 57.7   , 61.9   , 64.0   , 66.1 },
  {0  , 4    , 55.6   , 57.8   , 59.9   , 64.3   , 66.4   , 68.6 },
  {0  , 5    , 57.4   , 59.6   , 61.8   , 66.2   , 68.5   , 70.7 },
  {0  , 6    , 58.9   , 61.2   , 63.5   , 68.0   , 70.3   , 72.5 },
  {0  , 7    , 60.3   , 62.7   , 65.0   , 69.6   , 71.9   , 74.2 },
  {0  , 8    , 61.7   , 64.0   , 66.4   , 71.1   , 73.5   , 75.8 },
  {0  , 9    , 62.9   , 65.3   , 67.7   , 72.6   , 75.0   , 77.4 },
  {0  , 10   , 64.1   , 66.5   , 69.0   , 73.9   , 76.4   , 78.9 },
  {0  , 11   , 65.2   , 67.7   , 70.3   , 75.3   , 77.8   , 80.3 },
  {0  , 12   , 66.3   , 68.9   , 71.4   , 76.6   , 79.2   , 81.7 },
  {1  , 1    , 67.3   , 70.0   , 72.6   , 77.8   , 80.5   , 83.1 },
  {1  , 2    , 68.3   , 71.0   , 73.7   , 79.1   , 81.7   , 84.4 },
  {1  , 3    , 69.3   , 72.0   , 74.8   , 80.2   , 83.0   , 85.7 },
  {1  , 4    , 70.2   , 73.0   , 75.8   , 81.4   , 84.2   , 87.0 },
  {1  , 5    , 71.1   , 74.0   , 76.8   , 82.5   , 85.4   , 88.2 },
  {1  , 6    , 72.0   , 74.9   , 77.8   , 83.6   , 86.5   , 89.4 },
  {1  , 7    , 72.8   , 75.8   , 78.8   , 84.7   , 87.6   , 90.6 },
  {1  , 8    , 73.7   , 76.7   , 79.7   , 85.7   , 88.7   , 91.7 },
  {1  , 9    , 74.5   , 77.5   , 80.6   , 86.7   , 89.8   , 92.9 },
  {1  , 10   , 75.2   , 78.4   , 81.5   , 87.7   , 90.8   , 94.0 },
  {1  , 11   , 76.0   , 79.2   , 82.3   , 88.7   , 91.9   , 95.0 },
  {2  , 0    , 76.7   , 80.0   , 83.2   , 89.6   , 92.9   , 96.1 },
  {2  , 1    , 76.8   , 80.0   , 83.3   , 89.9   , 93.1   , 96.4 },
  {2  , 2    , 77.5   , 80.8   , 84.1   , 90.8   , 94.1   , 97.4 },
  {2  , 3    , 78.1   , 81.5   , 84.9   , 91.7   , 95.0   , 98.4 },
  {2  , 4    , 78.8   , 82.2   , 85.7   , 92.5   , 96.0   , 99.4 },
  {2  , 5    , 79.5   , 82.9   , 86.4   , 93.4   , 96.9   , 100.3},
  {2  , 6    , 80.1   , 83.6   , 87.1   , 94.2   , 97.7   , 101.3},
  {2  , 7    , 80.7   , 84.3   , 87.9   , 95.0   , 98.6   , 102.2},
  {2  , 8    , 81.3   , 84.9   , 88.6   , 95.8   , 99.4   , 103.1},
  {2  , 9    , 81.9   , 85.6   , 89.3   , 96.6   , 100.3  , 103.9},
  {2  , 10   , 82.5   , 86.2   , 90.6   , 97.4   , 101.1  , 104.8},
  {2  , 11   , 83.1   , 86.8   , 91.2   , 98.1   , 101.9  , 105.6},
  {3  , 0    , 83.6   , 87.4   , 91.9   , 98.9   , 102.7  , 106.5},
  {3  , 1    , 84.2   , 88.0   , 91.9   , 99.6   , 103.4  , 107.3},
  {3  , 2    , 84.7   , 88.6   , 92.5   , 100.3  , 104.2  , 108.1},
  {3  , 3    , 85.3   , 89.2   , 93.1   , 101.0  , 105.0  , 108.9},
  {3  , 4    , 85.8   , 89.8   , 93.8   , 101.7  , 105.7  , 109.7},
  {3  , 5    , 86.3   , 90.4   , 94.4   , 102.4  , 106.4  , 110.5},
  {3  , 6    , 86.8   , 90.9   , 95.0   , 103.1  , 107.2  , 111.2},
  {3  , 7    , 87.4   , 91.5   , 95.6   , 103.8  , 107.9  , 112.0},
  {3  , 8    , 87.9   , 92.0   , 96.2   , 104.5  , 108.6  , 112.7},
  {3  , 9    , 88.4   , 92.5   , 96.7   , 105.1  , 109.3  , 113.5},
  {3  , 10   , 88.9   , 93.1   , 97.3   , 105.8  , 110.0  , 114.2},
  {3  , 11   , 89.3   , 93.6   , 97.9   , 106.4  , 110.7  , 114.9},
  {4  , 0    , 89.8   , 94.1   , 98.4   , 107.0  , 111.3  , 115.7},
  {4  , 1    , 90.3   , 94.6   , 99.0   , 107.7  , 112.0  , 116.4},
  {4  , 2    , 90.7   , 95.1   , 99.5   , 108.3  , 112.7  , 117.1},
  {4  , 3    , 91.2   , 95.6   , 100.1  , 108.9  , 113.3  , 117.7},
  {4  , 4    , 91.7   , 96.1   , 100.6  , 109.5  , 114.0  , 118.4},
  {4  , 5    , 92.1   , 96.6   , 101.1  , 110.1  , 114.6  , 119.1},
  {4  , 6    , 92.6   , 97.1   , 101.6  , 110.7  , 115.2  , 119.8},
  {4  , 7    , 93.0   , 97.6   , 102.2  , 111.3  , 115.9  , 120.4},
  {4  , 8    , 93.4   , 98.1   , 102.7  , 111.9  , 116.5  , 121.1},
  {4  , 9    , 93.9   , 98.5   , 103.2  , 112.5  , 117.1  , 121.8},
  {4  , 10   , 94.3   , 99.0   , 103.7  , 113.0  , 117.7  , 122.4},
  {4  , 11   , 94.7   , 99.5   , 104.2  , 113.6  , 118.3  , 123.1},
  {5  , 0    , 95.2   , 99.9   , 104.7  , 114.2  , 118.9  , 123.7}
};

typedef struct
{
  int tahun;
  int bulan;
  float awal;
  float kurangGizi;
  float kurus;
  float normal;
  float gemuk;
  float obesitas;
} BeratPerempuan;

BeratPerempuan dataTabelBeratPerempuan[61] = {
  {0  , 0    , 2.0    , 2.4    , 2.8    , 3.7    , 4.2    , 4.8 },
  {0  , 1    , 2.7    , 3.2    , 3.6    , 4.8    , 5.5    , 6.2 },
  {0  , 2    , 3.4    , 3.9    , 4.5    , 5.8    , 6.6    , 7.5 },
  {0  , 3    , 4.0    , 4.5    , 5.2    , 6.6    , 7.5    , 8.5 },
  {0  , 4    , 4.4    , 5.0    , 5.7    , 7.3    , 8.2    , 9.3 },
  {0  , 5    , 4.8    , 5.4    , 6.1    , 7.8    , 8.8    , 10.0},
  {0  , 6    , 5.1    , 5.7    , 6.5    , 8.2    , 9.3    , 10.6},
  {0  , 7    , 5.3    , 6.0    , 6.8    , 8.6    , 9.8    , 11.1},
  {0  , 8    , 5.6    , 6.3    , 7.0    , 9.0    , 10.2   , 11.6},
  {0  , 9    , 5.8    , 6.5    , 7.3    , 9.3    , 10.5   , 12.0},
  {0  , 10   , 5.9    , 6.7    , 7.5    , 9.6    , 10.9   , 12.4},
  {0  , 11   , 6.1    , 6.9    , 7.7    , 9.9    , 11.2   , 12.8},
  {0  , 12   , 6.3    , 7.0    , 7.9    , 10.1   , 11.5   , 13.1},
  {1  , 1    , 6.4    , 7.2    , 8.1    , 10.4   , 11.8   , 13.5},
  {1  , 2    , 6.6    , 7.4    , 8.3    , 10.6   , 12.1   , 13.8},
  {1  , 3    , 6.7    , 7.6    , 8.5    , 10.9   , 12.4   , 14.1},
  {1  , 4    , 6.9    , 7.7    , 8.7    , 11.1   , 12.6   , 14.5},
  {1  , 5    , 7.0    , 7.9    , 8.9    , 11.4   , 12.9   , 14.8},
  {1  , 6    , 7.2    , 8.1    , 9.1    , 11.6   , 13.2   , 15.1},
  {1  , 7    , 7.3    , 8.2    , 9.2    , 11.8   , 13.5   , 15.4},
  {1  , 8    , 7.5    , 8.4    , 9.4    , 12.1   , 13.7   , 15.7},
  {1  , 9    , 7.6    , 8.6    , 9.6    , 12.3   , 14.0   , 16.0},
  {1  , 10   , 7.8    , 8.7    , 9.8    , 12.5   , 14.3   , 16.4},
  {1  , 11   , 7.9    , 8.9    , 10.0   , 12.8   , 14.6   , 16.7},
  {2  , 0    , 8.1    , 9.0    , 10.2   , 13.0   , 14.8   , 17.0},
  {2  , 1    , 8.2    , 9.2    , 10.3   , 13.3   , 15.1   , 17.3},
  {2  , 2    , 8.4    , 9.4    , 10.5   , 13.5   , 15.4   , 17.7},
  {2  , 3    , 8.5    , 9.5    , 10.7   , 13.7   , 15.7   , 18.0},
  {2  , 4    , 8.6    , 9.7    , 10.9   , 14.0   , 16.0   , 18.3},
  {2  , 5    , 8.8    , 9.8    , 11.1   , 14.2   , 16.2   , 18.7},
  {2  , 6    , 8.9    , 10.0   , 11.2   , 14.4   , 16.5   , 19.0},
  {2  , 7    , 9.0    , 10.1   , 11.4   , 14.7   , 16.8   , 19.3},
  {2  , 8    , 9.1    , 10.3   , 11.6   , 14.9   , 17.1   , 19.6},
  {2  , 9    , 9.3    , 10.4   , 11.7   , 15.1   , 17.3   , 20.0},
  {2  , 10   , 9.4    , 10.5   , 11.9   , 15.4   , 17.6   , 20.3},
  {2  , 11   , 9.5    , 10.7   , 12.0   , 15.6   , 17.9   , 20.6},
  {3  , 0    , 9.6    , 10.8   , 12.2   , 15.8   , 18.1   , 20.9},
  {3  , 1    , 9.7    , 10.9   , 12.4   , 16.0   , 18.4   , 21.3},
  {3  , 2    , 9.8    , 11.1   , 12.5   , 16.3   , 18.7   , 21.6},
  {3  , 3    , 9.9    , 11.2   , 12.7   , 16.5   , 19.0   , 22.0},
  {3  , 4    , 10.1   , 11.3   , 12.8   , 16.7   , 19.2   , 22.3},
  {3  , 5    , 10.2   , 11.5   , 13.0   , 16.9   , 19.5   , 22.7},
  {3  , 6    , 10.3   , 11.6   , 13.1   , 17.2   , 19.8   , 23.0},
  {3  , 7    , 10.4   , 11.7   , 13.3   , 17.4   , 20.1   , 23.4},
  {3  , 8    , 10.5   , 11.8   , 13.4   , 17.6   , 20.4   , 23.7},
  {3  , 9    , 10.6   , 12.0   , 13.6   , 17.8   , 20.7   , 24.1},
  {3  , 10   , 10.7   , 12.1   , 13.7   , 18.1   , 20.9   , 24.5},
  {3  , 11   , 10.8   , 12.2   , 13.9   , 18.3   , 21.2   , 24.8},
  {4  , 0    , 10.9   , 12.3   , 14.0   , 18.5   , 21.5   , 25.2},
  {4  , 1    , 11.0   , 12.4   , 14.2   , 18.8   , 21.8   , 25.5},
  {4  , 2    , 11.1   , 12.6   , 14.3   , 19.0   , 22.1   , 25.9},
  {4  , 3    , 11.2   , 12.7   , 14.5   , 19.2   , 22.4   , 26.3},
  {4  , 4    , 11.3   , 12.8   , 14.6   , 19.4   , 22.6   , 26.6},
  {4  , 5    , 11.4   , 12.9   , 14.8   , 19.7   , 22.9   , 27.0},
  {4  , 6    , 11.5   , 13.0   , 14.9   , 19.9   , 23.2   , 27.4},
  {4  , 7    , 11.6   , 13.2   , 15.1   , 20.1   , 23.5   , 27.7},
  {4  , 8    , 11.7   , 13.3   , 15.2   , 20.3   , 23.8   , 28.1},
  {4  , 9    , 11.8   , 13.4   , 15.3   , 20.6   , 24.1   , 28.5},
  {4  , 10   , 11.9   , 13.5   , 15.5   , 20.8   , 24.4   , 28.8},
  {4  , 11   , 12.0   , 13.6   , 15.6   , 21.0   , 24.6   , 29.2},
  {5  , 0    , 12.1   , 13.7   , 15.8   , 21.2   , 24.9   , 29.5}
};

//HX711 constructor:
HX711_ADC LoadCell(HX711_DOUT_PIN, HX711_SCK_PIN);
const int calVal_eepromAdress = 0;
const int distance_eepromAdress = 5;
unsigned long time1, time2, time3 = 0;
const int serialPrintInterval1 = 100;
const int serialPrintInterval2 = 500;


void displayKosongkanScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 54, 8, BOX_COLOR);
  tft.setCursor(27, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.println("KOSONGKAN");
  tft.setCursor(57, 40);
  tft.setTextSize(2);
  tft.print("AREA");
  // tft.setCursor(10, 70);
  tft.setTextColor(TEXT_COLOR);
  // tft.setTextSize(1);
  // tft.print("");
  tft.setCursor(15, 77);
  tft.print(">");
  tft.setCursor(30, 77);
  tft.print("TEKAN:");
  
  //  tft.setTextColor(HIGHLIGHT_COLOR);
  //  tft.print("'D'");
  tft.fillRoundRect(100, 74, 20, 20, 1,ST77XX_GREEN);
  
}

void displayProsesBerhasil() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setCursor(62, 52);
  tft.setTextSize(3);
  tft.setTextColor(HEADER_COLOR);
  tft.print("OK!");
  // tft.setCursor(32, 70);
  // tft.print("BERHASIL!");
}

void displayProsesBerhasil2() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setCursor(47, 28);
  tft.setTextSize(2);
  tft.setTextColor(HEADER_COLOR);
  tft.print("ANGKAT");
  tft.setCursor(57, 52);
  tft.print("ANAK");
  tft.setCursor(27, 76);
  tft.print("TIMBANGAN");
  // tft.setCursor(32, 70);
  // tft.print("BERHASIL!");
}

void displayAnakTimbangan() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 68, 8, BOX_COLOR);
  tft.setCursor(33, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.println("LETAKKAN");
  tft.setTextSize(2);
  tft.setCursor(57, 32);
  tft.print("ANAK");
  tft.setCursor(27, 53);
  tft.print("TIMBANGAN");

  // tft.setCursor(10, 83);
  // tft.setTextColor(TEXT_COLOR);
  // // tft.setTextSize(1);
  // tft.print(">");
  // tft.setCursor(20, 83);
  // tft.print("Jika siap, lalu tekan");
  // tft.setCursor(20, 98);
  // tft.print("tombol ");
  tft.setCursor(15, 86);
  tft.print(">");
  tft.setCursor(30, 86);
  tft.print("TEKAN:");
  tft.fillRoundRect(100, 82, 20, 20, 1,ST77XX_GREEN);
  //  tft.setTextColor(HIGHLIGHT_COLOR);
  //  tft.print("'D'");
  // tft.fillRoundRect(60, 97, 10, 10, 1,ST77XX_GREEN);
}

void displayInputIDScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 28, 8, BOX_COLOR);
  tft.setCursor(20, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("INPUT NoID");

  tft.setCursor(20, 50);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.println("Silakan input No ID");
  tft.setCursor(20, 62);
  tft.println("maximum 4 karakter");

}

void displayInputUsiaScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 28, 8, BOX_COLOR);
  tft.setCursor(20, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("KETIK USIA");

  // tft.setCursor(20, 40);
  // tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  // tft.println("Silakan input usia");
  // tft.setTextSize(1);
  tft.setCursor(18, 44);
  tft.println("> Tahun:");
  tft.setCursor(18, 82);
  tft.println("> Bulan:");
  displayFooter2();
}

void displayInputJKScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 54, 8, BOX_COLOR);
  tft.setCursor(33, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.println("SILAHKAN");
  tft.setCursor(57, 40);
  tft.setTextSize(2);
  tft.print("NAIK");
  tft.setCursor(10, 70);
  // tft.setTextSize(1);
  tft.fillRoundRect(5, 62, 150, 53, 8, ST77XX_BLACK);
  tft.setTextColor(TEXT_COLOR);
  tft.setCursor(10, 68);
  tft.print("Laki-laki ");
  tft.setCursor(10, 95);
  tft.setTextColor(TEXT_COLOR);
  tft.print("Perempuan ");
  tft.fillRoundRect(125, 64, 20, 20, 1,ST77XX_BLUE);
  tft.fillRoundRect(125, 92, 20, 20, 1,ST77XX_RED);
  displayFooter3();
}

void displayAturPosisi() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 28, 8, BOX_COLOR);
  tft.setCursor(25, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("PERSIAPAN");
  tft.setCursor(20, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.print("Silakan atur posisi");
  tft.setTextSize(1);
  tft.setCursor(20, 55);
  tft.print("anak agar siap.");
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("BACK:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(" |      ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(" |  NEXT:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  drawReturn(36, 117, ST77XX_GREEN);
  drawEnter(145, 117 , ST77XX_GREEN);

}

void displayPengukuranScreen() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 117, 28, 4, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("NoID:");
  tft.setCursor(45, 10);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(noID);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 20);
  tft.print("Umur:");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(45, 20);
  tft.print(usiaTahun);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(60, 20);
  tft.print("Th");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(80, 20);
  tft.print(usiaBulan);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(95, 20);
  tft.print("Bln");
  tft.fillRoundRect(127, 5, 28, 28, 4, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(136, 11);
  tft.print(jenisKelamin);
  tft.fillRoundRect(5, 37, 150, 35, 4, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 41);
  tft.print("Tinggi(cm)");
  //  tft.setTextSize(2);
  //  tft.setTextColor(ST77XX_YELLOW);
  //  tft.setCursor(10, 55);
  //  tft.print("120.24");
  //  labelStunted(95,40);
  tft.fillRoundRect(5, 75, 150, 35, 4, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 80);
  tft.print("Berat(Kg)");
  //  tft.setTextSize(2);
  //  tft.setTextColor(ST77XX_YELLOW);
  //  tft.setCursor(10, 94);
  //  tft.print("12.12");
  //  labelNormal(95, 78);
  //  displayFooter();

  tft.setCursor(10, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("BACK:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("| ULANG:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print(" ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(" | OK:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print(" ");
  tft.fillRoundRect(41, 117, 10, 10, 1,ST77XX_WHITE);
  tft.fillRoundRect(99, 117, 10, 10, 1,ST77XX_YELLOW);
  tft.fillRoundRect(145, 117, 10, 10, 1,ST77XX_GREEN);
  // drawReturn(41, 117, ST77XX_GREEN);
  // drawBackspace(99, 118, ST77XX_GREEN);
  // drawEnter(145, 117 , ST77XX_GREEN);

}

void displayMenuKalibrasiJarak() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 28, 8, BOX_COLOR);
  tft.setCursor(25, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("KAL JARAK");
  tft.setCursor(15, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.print("Pastikan tidak ada");
  tft.setTextSize(1);
  tft.setCursor(15, 55);
  tft.print("benda dibawah sensor");
  tft.setCursor(15, 75);
  tft.print("tekan    /u mulai");
  //  tft.setTextColor(ST77XX_GREEN);
  //  tft.print("'D'");
  //  tft.setTextColor(TEXT_COLOR);
  //  tft.print(" /u memulai");
  drawEnter(51, 75 , ST77XX_GREEN);
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("BACK:");
  //  tft.setTextColor(ST77XX_GREEN);
  //  tft.print("# ");
  drawReturn(36, 117, ST77XX_GREEN);

}

void displayKalibrasiSukses() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setCursor(20, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.print("Kalibrasi berhasil!!");
}

void displayMenuKalibrasiBerat() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 28, 8, BOX_COLOR);
  tft.setCursor(25, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("KAL TARE");
  tft.setCursor(15, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.print("Zero Calibration..");
  tft.setCursor(15, 55);
  tft.print("Pastikan tidak ada");
  tft.setTextSize(1);
  tft.setCursor(15, 70);
  tft.print("benda ditimbangan!!");
  tft.setCursor(15, 90);
  tft.print("tekan    /u mulai");
  //  tft.setTextColor(ST77XX_GREEN);
  //  tft.print("'D'");
  //  tft.setTextColor(TEXT_COLOR);
  //  tft.print("/u memulai");
  drawEnter(51, 90 , ST77XX_GREEN);
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("BACK:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("# ");
}

void displayMenuKalibrasiBerat2() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 28, 8, BOX_COLOR);
  tft.setCursor(25, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("KAL BEBAN");
  tft.setCursor(15, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
//  tft.print("Load Calibration..");
//  tft.setCursor(15, 55);
  tft.print("1. Letakkan beban ref");
  tft.setTextSize(1);
  tft.setCursor(15, 55);
  tft.print("2. input berat ref (gr)");
  tft.setCursor(15, 70);
//  tft.print("   cth 1.2 Kg = 1200 gr");
//  tft.setCursor(15, 75);
  tft.print("3. tekan    /u mulai");
  //  tft.setTextColor(ST77XX_GREEN);
  //  tft.print("'D'");
  //  tft.setTextColor(TEXT_COLOR);
  //  tft.print(" /u memulai");
  drawEnter(71, 70 , ST77XX_GREEN);
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("BACK:");
  drawReturn(36, 117, ST77XX_GREEN);
}

void displayCursor(int PosX, int PosY, int num, int maxCur, bool stateCursor) {
  PosX = PosX + (num * 20);
  if (num < maxCur) {
    if (stateCursor == 1) {
      tft.fillRoundRect(PosX, PosY, 16, 25, 1, HIGHLIGHT_COLOR);
    }
    if (stateCursor == 0) {
      tft.fillRoundRect(PosX, PosY, 16, 25, 1, BACKGROUND_COLOR);
    }
  }
  else {
    tft.fillRoundRect(PosX, PosY, 16, 25, 1, BACKGROUND_COLOR);
  }
}

void displayTextKeypad(int PosX, int PosY, int num) {
  PosX = PosX + (num * 20);
  tft.setCursor(PosX, PosY);
  tft.setTextColor(HIGHLIGHT_COLOR);
  //    tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print(keyInNow);
}
//void displayFooter() {
//  tft.setCursor(5, 118);
//  tft.setTextColor(ST77XX_YELLOW);
//  tft.setTextSize(1);
//  tft.print("BACK:");
//  tft.setTextColor(ST77XX_GREEN);
//  tft.print("# ");
//  tft.setTextColor(ST77XX_YELLOW);
//  tft.print(" |  DEL:");
//  tft.setTextColor(ST77XX_GREEN);
//  tft.print("C ");
//  tft.setTextColor(ST77XX_YELLOW);
//  tft.print(" |  OK:");
//  tft.setTextColor(ST77XX_GREEN);
//  tft.print("D ");
//}

void displayFooter() {
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("DEL:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(" | OK: ");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(" | BACK:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  tft.fillRoundRect(30, 117, 10, 10, 1,ST77XX_YELLOW);
  tft.fillRoundRect(80, 117, 10, 10, 1,ST77XX_GREEN);
  tft.fillRoundRect(145, 117, 10, 10, 1,ST77XX_WHITE);
}

void displayFooter2() {
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("HAPUS:");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(" | OK: ");
  tft.setTextColor(ST77XX_GREEN);
  tft.print("  ");
  tft.fillRoundRect(42, 116, 10, 10, 1,ST77XX_YELLOW);
  tft.fillRoundRect(90, 116, 10, 10, 1,ST77XX_GREEN);
}

void displayFooter3() {
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("KEMBALI:");
  tft.fillRoundRect(53, 116, 10, 10, 1,ST77XX_WHITE);
  
}

void drawBackspace(int x, int y, uint16_t color) {
  tft.drawPixel(x + 2, y, color);

  tft.drawPixel(x + 1, y + 1, color); tft.drawPixel(x + 2, y + 1, color);

  for (int ln = x; ln < x + 9; ln++) {
    tft.drawPixel(ln, y + 2, color);
  }
  for (int ln = x; ln < x + 9; ln++) {
    tft.drawPixel(ln, y + 3, color);
  }
  tft.drawPixel(x + 1, y + 4, color); tft.drawPixel(x + 2, y + 4, color);

  tft.drawPixel(x + 2, y + 5, color);

}

void drawReturn(int x, int y, uint16_t color) {
  tft.drawPixel(x + 2, y, color);

  tft.drawPixel(x + 1, y + 1, color); tft.drawPixel(x + 2, y + 1, color);

  for (int ln = x; ln < x + 8; ln++) {
    tft.drawPixel(ln, y + 2, color);
  }

  for (int ln = x; ln < x + 9; ln++) {
    tft.drawPixel(ln, y + 3, color);
  }

  tft.drawPixel(x + 1, y + 4, color); tft.drawPixel(x + 2, y + 4, color);
  tft.drawPixel(x + 7, y + 4, color); tft.drawPixel(x + 8, y + 4, color);

  tft.drawPixel(x + 2, y + 5, color);
  tft.drawPixel(x + 7, y + 5, color); tft.drawPixel(x + 8, y + 5, color);

  tft.drawPixel(x + 7, y + 6, color); tft.drawPixel(x + 8, y + 6, color);

  for (int ln = x + 1; ln < x + 9; ln++) {
    tft.drawPixel(ln, y + 7, color);
  }
  for (int ln = x + 1; ln < x + 8; ln++) {
    tft.drawPixel(ln, y + 8, color);
  }
}

void drawEnter(int x, int y, uint16_t color) {
  tft.drawPixel(x + 7, y, color); tft.drawPixel(x + 8, y, color);

  tft.drawPixel(x + 7, y + 1, color); tft.drawPixel(x + 8, y + 1, color);

  tft.drawPixel(x + 2, y + 2, color); tft.drawPixel(x + 7, y + 2, color); tft.drawPixel(x + 8, y + 2, color);

  tft.drawPixel(x + 1, y + 3, color); tft.drawPixel(x + 2, y + 3, color);
  tft.drawPixel(x + 7, y + 3, color); tft.drawPixel(x + 8, y + 3, color);

  for (int ln = x; ln < x + 9; ln++) {
    tft.drawPixel(ln, y + 4, color);
  }

  for (int ln = x; ln < x + 8; ln++) {
    tft.drawPixel(ln, y + 5, color);
  }

  tft.drawPixel(x + 1, y + 6, color); tft.drawPixel(x + 2, y + 6, color);

  tft.drawPixel(x + 2, y + 7, color);


}

void displayDataInvalid() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setCursor(20, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.print("Data belum lengkap");
  tft.setTextSize(1);
  tft.setCursor(20, 55);
  tft.print("atau salah...");
  tft.setCursor(20, 70);
  tft.print("Mohon diisi lagi..");
}

void displayMenuKalibrasi() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.fillRoundRect(5, 5, 150, 28, 8, BOX_COLOR);
  tft.setCursor(20, 11);
  tft.setTextColor(HEADER_COLOR);
  tft.setTextWrap(true);
  tft.setTextSize(2);
  tft.print("KALIBRASI");

  tft.setCursor(15, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.setCursor(15, 40);
  tft.print("> Sensor jarak");
  tft.setCursor(125, 40);
  tft.setTextColor(HIGHLIGHT_COLOR);
  tft.print("'1'");
  tft.setCursor(15, 55);
  tft.setTextColor(TEXT_COLOR);
  tft.print("> Kalibrasi Tare");
  tft.setCursor(125, 55);
  tft.setTextColor(HIGHLIGHT_COLOR);
  tft.print("'2'");
  tft.setCursor(15, 70);
  tft.setTextColor(TEXT_COLOR);
  tft.print("> Kalibrasi beban");
  tft.setCursor(125, 70);
  tft.setTextColor(HIGHLIGHT_COLOR);
  tft.print("'3'");
  tft.setCursor(5, 118);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("BACK:");
  //  tft.setTextColor(ST77XX_GREEN);
  //  tft.print("# ");
  drawReturn(36, 117, ST77XX_GREEN);
}

void displayFinalOkSave() {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setCursor(20, 40);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR);
  tft.print("Pengukuran selesai");
  tft.setTextSize(1);
  tft.setCursor(20, 55);
  tft.print("Data telah disimpan..");
  tft.setCursor(20, 70);
  tft.print("Kembali ke awal..");

}
void labelTinggi(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, 0xFDA0);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 12));
  tft.print("TINGGI");
}
void labelPendek(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, 0xFDA0);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 12));
  tft.print("PENDEK");
}
void labelSangatTinggi(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, ST77XX_RED);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 5));
  tft.print("SANGAT");
  tft.setCursor((posX + 10), (posY + 17));
  tft.print("TINGGI");
}
void labelStunted(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, ST77XX_RED);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 7), (posY + 12));
  tft.print("STUNTED");
}
void labelNormal(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, ST77XX_GREEN);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 12));
  tft.print("NORMAL");
}
void labelKurangGizi(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, ST77XX_RED);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 5));
  tft.print("KURANG");
  tft.setCursor((posX + 10), (posY + 17));
  tft.print("GIZI");
}
void labelObesitas(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, ST77XX_RED);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 12));
  tft.print("OBESE");
}
void labelKurus(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, 0xFDA0);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 12));
  tft.print("KURUS");
}
void labelGemuk(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, 0xFDA0);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor((posX + 10), (posY + 12));
  tft.print("GEMUK");
}
void labelNetral(int posX, int posY) {
  tft.fillRoundRect(posX, posY, 55, 30, 4, ST77XX_BLACK);
}
void distancePrintTest(float textLast, float textNow) {
  tft.setCursor(10, 55);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(textLast);
  tft.setCursor(10, 55);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println(textNow);
}
void weightPrintTest(float textLast1, float textNow1) {
  tft.setCursor(10, 94);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(textLast1);
  tft.setCursor(10, 94);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println(textNow1);
}

void hitungTinggiBadan() {
  //  int
//  for (int i = 0; i < 9; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(ECHO_PIN, HIGH);
    // Calculating the distance
    //  distance = duration * 0.034 / 2;
    distances = duration / 58.2;
  Serial.print("CATAT TINGGI: ");
  Serial.println(distances);
  distances=(1.0073*distances) - 0.6373; //1.0073x - 0.6373
  distanceMean = distances;
  lastDistance = distanceNow;
  distanceNow = 140 - distanceMean-1.5;
  if (distanceNow<10){
    distanceNow=0;
  }
  DEBUG_PRINTLN(distances);
  DEBUG_PRINTLN(distanceNow);
  distanceSum = 0;
  distancePrintTest(lastDistance, distanceNow);
}

void hitungTinggiBadanTanpaReg() {
  //  int
//  for (int i = 0; i < 9; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(ECHO_PIN, HIGH);
    // Calculating the distance
    //  distance = duration * 0.034 / 2;
    distances = duration / 58.2;
  Serial.print("CATAT TINGGI: ");
  Serial.println(distances);
  distanceMean = distances;
  lastDistance = distanceNow;
  distanceNow = 140 - distanceMean-1.5;
  if (distanceNow<10){
    distanceNow=0;
  }
  DEBUG_PRINTLN(distances);
  DEBUG_PRINTLN(distanceNow);
  distanceSum = 0;
  distancePrintTest(lastDistance, distanceNow);
}

void calSensorJarak() {
 
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(ECHO_PIN, HIGH);
    // Calculating the distance
     distances = duration * 0.034 / 2;
    // distances = duration / 58.2;

  
  distanceMean = distances;
  DEBUG_PRINTLN(distanceMean);
  calValueDistanceSensor=distanceMean;
  EEPROM.writeFloat(distance_eepromAdress, distanceMean);
  EEPROM.commit();
  calValueDistanceSensor=140;
  // DEBUG_PRINTLN(calValueDistanceSensor);
  distanceSum = 0;
}
void loadCellZeroCal() {
  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (!FLAG_TARE) {
      LoadCell.tareNoDelay();
      FLAG_TARE = 1;
    }
    if (LoadCell.getTareStatus() == true) {
      // Serial.println(mbn);
      Serial.println("Tare complete");
      _resume = true;
      FLAG_TARE = 0;
    }
  }
}
void loadCellLoadCal(float x) {
  bool _resume = false;
  while (_resume == false) {
    LoadCell.update();

    if (x != 0) {
      Serial.print("Known mass is: ");
      Serial.println(x);
      _resume = true;
    } else {
      break;
    }
  }
  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue = LoadCell.getNewCalibration(x);
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Value ");
  Serial.print(newCalibrationValue);
  Serial.print(" saved to EEPROM address: ");
  Serial.println(calVal_eepromAdress);
   _resume = true;
}


void hitungBeratBadan() {
  //  for (int i; i<5; i++){
  //  static boolean newDataReady = 0;
  //    const int serialPrintInterval = 0;
  //    while(!LoadCell.update()){
  //
  //    }
  //    float i = LoadCell.getData();
  //  }
  dataLast = dataNow;
  dataNow = dataLoadCell;
  Serial.print("CATAT BERAT: ");
  Serial.println(dataNow);
  weightPrintTest(dataLast, dataNow);
}

int klasifikasiTinggiBadan(int tahun, int bulan, String jk) {
  int hasil;
  if (jk == "L") {
    for (int a = 0; a <= 61; a++) {
      if (dataTabelTinggiLaki[a].tahun == tahun) {
        if (dataTabelTinggiLaki[a].bulan == bulan) {
          if (distanceNow <= dataTabelTinggiLaki[a].stunted) {
            hasil = 0;
            labelNetral(95, 40);
            labelStunted(95, 40);
            klasifikasiTB = "Stunted";
            break;
          }
          if (distanceNow > dataTabelTinggiLaki[a].stunted && distanceNow <= dataTabelTinggiLaki[a].pendek) {
            hasil = 1;
            labelNetral(95, 40);
            labelPendek(95, 40);
            klasifikasiTB = "Pendek";
            break;
          }
          if (distanceNow > dataTabelTinggiLaki[a].pendek && distanceNow <= dataTabelTinggiLaki[a].normal) {
            hasil = 2;
            labelNetral(95, 40);
            labelNormal(95, 40);
            klasifikasiTB = "Normal";
            break;
          }
          if (distanceNow > dataTabelTinggiLaki[a].normal && distanceNow <= dataTabelTinggiLaki[a].tinggi) {
            hasil = 3;
            labelNetral(95, 40);
            labelTinggi(95, 40);
            klasifikasiTB = "Tinggi";
            break;
          }
          if (distanceNow > dataTabelTinggiLaki[a].tinggi) {
            hasil = 4;
            labelNetral(95, 40);
            labelSangatTinggi(95, 40);
            klasifikasiTB = "Sangat Tinggi";
            break;
          }
        }
      }
    }
  }
  if (jk == "P") {
    for (int a = 0; a <= 61; a++) {
      if (dataTabelTinggiPerempuan[a].tahun == tahun) {
        if (dataTabelTinggiPerempuan[a].bulan == bulan) {
          if (distanceNow <= dataTabelTinggiPerempuan[a].stunted) {
            hasil = 0;
            labelNetral(95, 40);
            labelStunted(95, 40);
            klasifikasiTB = "Stunted";
            break;
          }
          if (distanceNow > dataTabelTinggiPerempuan[a].stunted && distanceNow <= dataTabelTinggiPerempuan[a].pendek) {
            hasil = 1;
            labelNetral(95, 40);
            labelPendek(95, 40);
            klasifikasiTB = "Pendek";
            break;
          }
          if (distanceNow > dataTabelTinggiPerempuan[a].pendek && distanceNow <= dataTabelTinggiPerempuan[a].normal) {
            hasil = 2;
            labelNetral(95, 40);
            labelNormal(95, 40);
            klasifikasiTB = "Normal";
            break;
          }
          if (distanceNow > dataTabelTinggiPerempuan[a].normal && distanceNow <= dataTabelTinggiPerempuan[a].tinggi) {
            hasil = 3;
            labelNetral(95, 40);
            labelTinggi(95, 40);
            klasifikasiTB = "Tinggi";
            break;
          }
          if (distanceNow > dataTabelTinggiPerempuan[a].tinggi) {
            hasil = 4;
            labelNetral(95, 40);
            labelSangatTinggi(95, 40);
            klasifikasiTB = "Sangat Tinggi";
            break;
          }
        }
      }
    }
  }

  return hasil;
}

int klasifikasiBeratBadan(int tahun, int bulan, String jk) {
  int hasil;
  if (jk == "L") {
    for (int a = 0; a <= 61; a++) {
      if (dataTabelBeratLaki[a].tahun == tahun) {
        if (dataTabelBeratLaki[a].bulan == bulan) {
          if (dataNow <= dataTabelBeratLaki[a].kurangGizi) {
            hasil = 0;
            labelNetral(95, 78);
            labelKurangGizi(95, 78);
            klasifikasiBB = "Kurang Gizi";
            break;
          }
          if (dataNow > dataTabelBeratLaki[a].kurangGizi && dataNow <= dataTabelBeratLaki[a].kurus) {
            hasil = 1;
            labelNetral(95, 78);
            labelKurus(95, 78);
            klasifikasiBB = "Kurus";
            break;
          }
          if (dataNow > dataTabelBeratLaki[a].kurus && dataNow <= dataTabelBeratLaki[a].normal) {
            hasil = 2;
            labelNetral(95, 78);
            labelNormal(95, 78);
            klasifikasiBB = "Normal";
            break;
          }
          if (dataNow > dataTabelBeratLaki[a].normal && dataNow <= dataTabelBeratLaki[a].gemuk) {
            hasil = 3;
            labelNetral(95, 78);
            labelGemuk(95, 78);
            klasifikasiBB = "Gemuk";
            break;
          }
          if (dataNow > dataTabelBeratLaki[a].obesitas) {
            hasil = 4;
            labelNetral(95, 78);
            labelObesitas(95, 78);
            klasifikasiBB = "Obesitas";
            break;
          }
        }
      }
    }
  }
  if (jk == "P") {
    for (int a = 0; a <= 61; a++) {
      if (dataTabelBeratPerempuan[a].tahun == tahun) {
        if (dataTabelBeratPerempuan[a].bulan == bulan) {
          if (dataNow <= dataTabelBeratPerempuan[a].kurangGizi) {
            hasil = 0;
            labelNetral(95, 78);
            labelKurangGizi(95, 78);
            klasifikasiBB = "Kurang Gizi";
            break;
          }
          if (dataNow > dataTabelBeratPerempuan[a].kurangGizi && dataNow <= dataTabelBeratPerempuan[a].kurus) {
            hasil = 1;
            labelNetral(95, 78);
            labelKurus(95, 78);
            klasifikasiBB = "Kurus";
            break;
          }
          if (dataNow > dataTabelBeratPerempuan[a].kurus && dataNow <= dataTabelBeratPerempuan[a].normal) {
            hasil = 2;
            labelNetral(95, 78);
            labelNormal(95, 78);
            klasifikasiBB = "Normal";
            break;
          }
          if (dataNow > dataTabelBeratPerempuan[a].normal && dataNow <= dataTabelBeratPerempuan[a].gemuk) {
            hasil = 3;
            labelNetral(95, 78);
            labelGemuk(95, 78);
            klasifikasiBB = "Gemuk";
            break;
          }
          if (dataNow > dataTabelBeratPerempuan[a].obesitas) {
            hasil = 4;
            labelNetral(95, 78);
            labelObesitas(95, 78);
            klasifikasiBB = "Obesitas";
            break;
          }
        }
      }
    }
  }
  Serial.println(hasil);
  return hasil;
}
void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(BACKLIGHT_PIN, ledChannel);
  dutyCycle = 180;
  ledcWrite(ledChannel, dutyCycle);
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(1);
  tft.fillScreen(0x3A0E);
  tft.setCursor(20, 40);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Memulai...");


  LoadCell.begin();

  float calibrationValue;
  EEPROM.begin(512);
  EEPROM.get(calVal_eepromAdress, calibrationValue);
  DEBUG_PRINTLN(calibrationValue);
  calValueDistanceSensor = EEPROM.readFloat(distance_eepromAdress);
  DEBUG_PRINTLN(calValueDistanceSensor);
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    tft.setCursor(15, 40);
    tft.setTextColor(0x3A0E);
    tft.setTextSize(1);
    tft.println("Timeout, periksa kembali koneksi timbangan...");
    LoadCell.begin();
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }

  delay(1000);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    FLAG_SD_AVAILABLE = 0;
    return;
  }
  Serial.println("SD prog");
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

}

void loop() {
  char customKey = customKeypad.getKey();
  if (customKey) {
    keyLast = keyNow;
    keyNow = customKey;
    DEBUG_PRINTLN(keyNow);
    //      keypadPrintTest(keyLast, keyNow);
  }
  if (flagKeyRead == 1) {
    keyNow = '.';
    flagKeyRead = 0;
  }
  switch (state) {
    case 0:
      if (!flagKosongkanScreen) {
        DEBUG_PRINTLN("SCREEN-KOSONGKAN");
        displayKosongkanScreen();
        flagKosongkanScreen = 1;
        noID="1";
      }
      if (keyNow == 'D') {
        calSensorJarak();
        loadCellZeroCal();
        state = 1;
        flagKosongkanScreen = 0;
        flagKeyRead = 1;
        displayProsesBerhasil();
        delay(2000);
      }
      break;
    case 1:
      if (!flagAnakTimbangan) {
        DEBUG_PRINTLN("SCREEN-ANAK-TIMBANGAN");
        displayAnakTimbangan();
        flagAnakTimbangan = 1;
      }
      if (keyNow == 'D') {
        DEBUG_PRINTLN("OK");
        float loadRefFloat=1.00;
        loadCellLoadCal(loadRefFloat);
        state = 3;
        flagKeyRead = 1;
        flagShowCursor = 0;
        flagAnakTimbangan = 0;
        displayProsesBerhasil2();
        delay(2000);
      }
      break;
    case 2:
      if (noIDInt != 0) {
        state = 3;
      } else {
        displayDataInvalid();
        noID = "";
        delay(2000);
        state = 1;
      }
      break;
    case 3:
      if (!flagInputUsiaScreen) {
        DEBUG_PRINTLN("Input usia screen");
        displayInputUsiaScreen();
        flagInputUsiaScreen = 1;
        cursorPosStartX = 117;
        cursorPosStartY = 38;
        charPosStartX = 118;
        charPosStartY = 44;
        cursorNum = 0;
        flagShowCursor = 1;
        maxCursor = 3;

      }
      if (keyNow == 'D') {
        DEBUG_PRINTLN("OK");
        usiaTahunInt = usiaTahun.toInt();
        usiaBulanInt = usiaBulan.toInt();
        state = 4;
        flagKeyRead = 1;
        flagShowCursor = 0;
        flagInputUsiaScreen = 0;
      }
      if (keyNow == 'C') {
        DEBUG_PRINTLN("DEL");
        if (cursorNum == 1) {
          flagShowCursor = 0;
          usiaTahun.remove((cursorNum - 1), 1);
          displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
          cursorNum--;
          flagKeyRead = 1;
          cursorPosStartX = 117;
          cursorPosStartY = 38;
          charPosStartX = 118;
          charPosStartY = 44;
          flagShowCursor = 1;
          DEBUG_PRINTLN(usiaTahun);
        }
        if (cursorNum == 2 || cursorNum == 3) {
          flagShowCursor = 0;
          usiaBulan.remove((cursorNum - 2), 1);
          displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
          cursorNum--;
          flagKeyRead = 1;
          flagShowCursor = 1;
          DEBUG_PRINTLN(usiaBulan);
        }

      }
      k = (int)keyNow;
      if (k >= 48 && k <=  57) {
        //          noID = noID +keyNow;
        if (cursorNum > 0 && cursorNum < 3) {
          usiaBulan.concat(keyNow);
          keyInLast = keyInNow;
          keyInNow = keyNow;
          flagShowCursor = 0;
          displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
          displayTextKeypad(charPosStartX, charPosStartY, cursorNum);
          cursorNum++;
          flagShowCursor = 1;
          flagKeyRead = 1;
          DEBUG_PRINTLN(usiaBulan);
        }
        if (cursorNum == 0) {
          usiaTahun.concat(keyNow);
          keyInLast = keyInNow;
          keyInNow = keyNow;
          flagShowCursor = 0;
          displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
          displayTextKeypad(charPosStartX, charPosStartY, cursorNum);
          cursorPosStartX = 98;
          cursorPosStartY = 77;
          charPosStartX = 99;
          charPosStartY = 83;
          cursorNum++;
          flagShowCursor = 1;
          flagKeyRead = 1;
          DEBUG_PRINTLN(usiaTahun);
        }
        ;
      }
      break;
    case 4:
      if (usiaTahun != NULL && usiaBulan != NULL && usiaTahunInt <= 5 && usiaBulanInt < 12) {
        if (usiaTahunInt == 5 && usiaBulanInt > 0) {
          displayDataInvalid();
          usiaTahun = "";
          usiaBulan = "";
          delay(2000);
          state = 3;
          break;
        }
        state = 5;
      } else {
        displayDataInvalid();
        usiaTahun = "";
        usiaBulan = "";
        delay(2000);
        state = 3;
      }
      break;
    case 5:
      if (!flagInputJenisKelaminScreen) {
        DEBUG_PRINTLN("Input JK screen");
        displayInputJKScreen();
        flagReadLoadCell = 1;
        flagReadUltrasonic = 1;
        flagInputJenisKelaminScreen = 1;
        // cursorPosStartX = 74;
        // cursorPosStartY = 70;
        // charPosStartX = 75;
        // charPosStartY = 76;
        // cursorNum = 0;
        // flagShowCursor = 1;
        // maxCursor = 1;
      }
      if (keyNow == '#') {
        DEBUG_PRINTLN("BACK");
        state = 3;
        flagKeyRead = 1;
        flagShowCursor = 0;
        jenisKelamin = "";
        usiaTahun = "";
        usiaBulan = "";
        flagInputJenisKelaminScreen = 0;
      }
      if (keyNow == 'A') {
        jenisKelamin = "L";
        
        flagShowCursor = 0;
        // displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
        // displayTextKeypad(charPosStartX, charPosStartY, cursorNum);
        // cursorNum++;
        flagShowCursor = 0;
        flagKeyRead = 1;
        DEBUG_PRINTLN(jenisKelamin);

        state = 8;
        flagKeyRead = 1;
        flagInputJenisKelaminScreen = 0;
      }
      if (keyNow == 'B') {
        jenisKelamin = "P";
        
        flagShowCursor = 0;
        // displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
        // displayTextKeypad(charPosStartX, charPosStartY, cursorNum);
        // cursorNum++;
        flagShowCursor = 0;
        flagKeyRead = 1;
        DEBUG_PRINTLN(jenisKelamin);

        state = 8;
        flagKeyRead = 1;
        flagInputJenisKelaminScreen = 0;
      }
      break;
    case 6:
      if (jenisKelamin == "L" || jenisKelamin == "P") {
        state = 7;
      } else {
        displayDataInvalid();
        jenisKelamin = "";
        delay(2000);
        state = 5;
      }
      break;
    case 7:
      if (!flagPersiapanScreen) {
        displayAturPosisi();
        DEBUG_PRINTLN("Persiapan screen");
        flagPersiapanScreen = 1;
        flagReadLoadCell = 1;
        flagReadUltrasonic = 1;
      }
      if (keyNow == '#') {
        DEBUG_PRINTLN("BACK");
        state = 5;
        flagKeyRead = 1;
        flagPersiapanScreen = 0;
      }
      if (keyNow == 'D') {
        DEBUG_PRINTLN("OK");
        state = 8;
        flagKeyRead = 1;
        flagPersiapanScreen = 0;
      }
      break;
    case 8:
      if (!flagPengukuranScreen) {
        flagReadLoadCell = 1;
        flagReadUltrasonic = 1;
        DEBUG_PRINTLN("Display Pengukuran Screen");
        displayPengukuranScreen();
        DEBUG_PRINTLN("hitung tb");
        hitungTinggiBadanTanpaReg();
        //        int klasifikasiTinggi;
        DEBUG_PRINTLN("klasi tb");
        klasifikasiTinggi = klasifikasiTinggiBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);
        DEBUG_PRINTLN("hitung bb");
        hitungBeratBadan();
        //        int klasifikasiBerat;
        DEBUG_PRINTLN("klasi bb");
        klasifikasiBerat = klasifikasiBeratBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);

        DEBUG_PRINTLN("pengukuran screen");
        flagPengukuranScreen = 1;

        // if (ulangUkur==0){
        //   flagKeyRead = 1;
        //   hitungTinggiBadan();
        //   klasifikasiTinggi = klasifikasiTinggiBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);
        //   hitungBeratBadan();
        //   klasifikasiBerat = klasifikasiBeratBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);
        //   ulangUkur=ulangUkur+1;
        // }
      }
      if (keyNow == '#') {
        DEBUG_PRINTLN("BACK");
        state = 4;
        flagKeyRead = 1;
        delay(20);
        flagPengukuranScreen = 0;
      }
      if (keyNow == 'C') {
//        DEBUG_PRINTLN("ULANG");
        flagKeyRead = 1;
        //          float tb=0;
        hitungTinggiBadanTanpaReg();
        //          DEBUG_PRINTLN(tb);

        klasifikasiTinggi = klasifikasiTinggiBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);

        hitungBeratBadan();
        //        int klasifikasiBerat;
        klasifikasiBerat = klasifikasiBeratBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);

      }
      //Tambahan--------------------------------------------------------------------------------
      if (keyNow == 'B')
      {
        flagKeyRead = 1;
        //          float tb=0;
        
        //          DEBUG_PRINTLN(tb);
        hitungTinggiBadan();
        klasifikasiTinggi = klasifikasiTinggiBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);

        hitungBeratBadan();
        //        int klasifikasiBerat;
        klasifikasiBerat = klasifikasiBeratBadan(usiaTahunInt, usiaBulanInt, jenisKelamin);
      }
      //---------------------------------------------------------------------------------------
      if (keyNow == 'D') {
        DEBUG_PRINTLN("OK n Save");
        state = 3;
        flagKeyRead = 1;
        flagReadLoadCell = 0;
        flagReadUltrasonic = 0;
        displayFinalOkSave();
        delay(1000);
        String tinggi = (String)distanceNow;
        String berat = (String)dataNow;
        const char* brt = berat.c_str();
        const char* ktb = klasifikasiTB.c_str();
        const char* kbb = klasifikasiBB.c_str();
        //        snprintf(dataPosyandu,200, "%s;%s;%s;%s;%s;%s;%s;%s\n", noID, usiaTahun, usiaBulan, jenisKelamin, tinggi, ktb, berat, kbb);
        snprintf(dataPosyandu, 200, "%s;%s;%s;%s;%s;", noID, usiaTahun, usiaBulan, jenisKelamin, tinggi);
        DEBUG_PRINTLN(dataPosyandu);
        appendFile(SD, "/log-posyandu.txt", dataPosyandu);
        appendFile(SD, "/log-posyandu.txt", ktb);
        appendFile(SD, "/log-posyandu.txt", ";");
        appendFile(SD, "/log-posyandu.txt", brt);
        appendFile(SD, "/log-posyandu.txt", ";");
        appendFile(SD, "/log-posyandu.txt", kbb);
        appendFile(SD, "/log-posyandu.txt", "\n");
        //        snprintf(dataPosyandu2,20, "%s;%s;%s;%s;%s;", noID, usiaTahun, usiaBulan, jenisKelamin, tinggi);
        //        sprintf(dataPosyandu2, "%f;%s;%f;%s\n", distanceNow, klasifikasiTB, dataNow, klasifikasiBB);
        //        appendFile(SD, "/log-posyandu.txt",dataPosyandu2);
        readFile(SD, "/log-posyandu.txt");
        //        DEBUG_PRINTLN(klasifikasiTB);
        //        DEBUG_PRINTLN(klasifikasiBB);
        delay(1000);
        flagPengukuranScreen = 0;
        noID = "1";
        usiaTahun = "";
        usiaBulan = "";
        jenisKelamin = "";
      }
      break;
    case 10:
      if (flagDisplayKalibrasi == 0) {
        displayMenuKalibrasi();
        flagDisplayKalibrasi = 1;
      }
      if (keyNow == '1') {
        state = 11;
        flagKeyRead = 1;
        flagDisplayKalibrasi = 0;
      }
      if (keyNow == '2') {
        state = 12;
        flagKeyRead = 1;
        flagDisplayKalibrasi = 0;
      }
      if (keyNow == '3') {
        state = 13;
        flagKeyRead = 1;
        flagDisplayKalibrasi = 0;
      }
      if (keyNow == '#') {
        DEBUG_PRINTLN("BACK");
        state = 0;
        flagKeyRead = 1;
        flagDisplayKalibrasi = 0;
      }
      break;
    case 11:
      if (flagDisplayKalibrasiJarak == 0) {
        displayMenuKalibrasiJarak();
        flagDisplayKalibrasiJarak = 1;
      }
      if (keyNow == '#') {
        DEBUG_PRINTLN("BACK");
        state = 10;
        flagKeyRead = 1;
        flagDisplayKalibrasiJarak = 0;
      }
      if (keyNow == 'D') {
        DEBUG_PRINTLN("CAL");
        calSensorJarak();
        state = 0;
        flagKeyRead = 1;
        flagDisplayKalibrasiJarak = 0;
        displayKalibrasiSukses();
        delay(2000);
      }
      break;
    case 12:
      if (flagDisplayKalibrasiBerat == 0) {
        displayMenuKalibrasiBerat();
        flagDisplayKalibrasiBerat = 1;
      }
      if (keyNow == '#') {
        DEBUG_PRINTLN("BACK");
        state = 10;
        flagKeyRead = 1;
        flagDisplayKalibrasiBerat = 0;
      }
      if (keyNow == 'D') {
        DEBUG_PRINTLN("CAL ZERO");
        loadCellZeroCal();
        state = 10;
        flagKeyRead = 1;
        flagDisplayKalibrasiBerat = 0;
        displayKalibrasiSukses();
        delay(2000);
      }
      break;
    case 13:
      if (flagDisplayKalibrasiBerat2 == 0) {
        displayMenuKalibrasiBerat2();
        flagDisplayKalibrasiBerat2 = 1;
        displayFooter();
        cursorPosStartX = 10;
        cursorPosStartY = 90;
        charPosStartX = 11;
        charPosStartY = 96;
        cursorNum = 0;
        flagShowCursor = 1;
        maxCursor = 6;
      }
      if (keyNow == '#') {
        DEBUG_PRINTLN("BACK");
        state = 10;
        flagKeyRead = 1;
        flagShowCursor = 0;
        loadRef = "";
        flagDisplayKalibrasiBerat2 = 0;
      }
      if (keyNow == 'D') {
        DEBUG_PRINTLN("CAL MASS");
        int loadRefInt = loadRef.toInt();
        // float loadRefFloat = float(loadRefInt) / 1000.0 ;
        // Serial.println(loadRefFloat);
        // loadCellLoadCal(loadRefFloat);
        state = 10;
        flagKeyRead = 1;
        flagDisplayKalibrasiBerat2 = 0;
        displayKalibrasiSukses();
        delay(2000);
      }
      if (keyNow == 'C') {
        DEBUG_PRINTLN("DEL");
        if (cursorNum > 0) {
          flagShowCursor = 0;
          loadRef.remove((cursorNum - 1), 1);
          displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
          cursorNum--;
        }
        flagKeyRead = 1;
        flagShowCursor = 1;
        DEBUG_PRINTLN(loadRef);
      }
      k = (int)keyNow;
      if ((k >= 48 && k <=  57) || k == 42) {
        //          noID = noID +keyNow;
        if (cursorNum < maxCursor) {
          loadRef.concat(keyNow);
          keyInLast = keyInNow;
          keyInNow = keyNow;
          flagShowCursor = 0;
          displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
          displayTextKeypad(charPosStartX, charPosStartY, cursorNum);
          cursorNum++;
          flagShowCursor = 1;
          //          if (keyNow == '*') {
          //            loadRef.concat('.');
          //          } else {
          //            loadRef.concat(keyNow);
          //            keyInLast = keyInNow;
          //            keyInNow = keyNow;
          //            flagShowCursor = 0;
          //            displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, 0);
          //            displayTextKeypad(charPosStartX, charPosStartY, cursorNum);
          //            cursorNum++;
          //            flagShowCursor = 1;
        }
        flagKeyRead = 1;
        DEBUG_PRINTLN(loadRef);

      }
      break;
  }

  if (flagShowCursor == 1) {
    //      DEBUG_PRINTLN("blink");
    if (millis() > time2 + serialPrintInterval2) {
      cursorState = !cursorState;
      displayCursor(cursorPosStartX, cursorPosStartY, cursorNum, maxCursor, cursorState);
      time2 = millis();
    }
  }
  if (flagReadLoadCell == 1) {
    static boolean newDataReady = 0;
    //increase value to slow down serial print activity
    // check for new data/start next conversion:
    if (LoadCell.update()) newDataReady = true;

    // get smoothed value from the dataset:
    if (newDataReady) {
      if (millis() > time1 + serialPrintInterval1) {
        float i = LoadCell.getData();
        if (i < 0) {
          i = 0;
        }
        dataLoadCell = i;

      }
    }
  }


}
