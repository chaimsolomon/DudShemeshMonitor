#include <DS3231.h>

#include <EEPROM.h>
#include <Adafruit_SleepyDog.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Connect to a Arduino Nano compatible device:
// Generic I2C-LCD bridge and DS3231 clock module to I2C pins
// DS18B20 sensors (1Wire) to Pin 5 (wire a 4k7 as pull-up to VCC (5V) to this pin too, wire 5V to the VCC pin of the sensor)

// Sensor 1 should be connected to the input of teh Dud Shemesh and Sensor 2 should be connected to the return.
// Stick the sensor between the pipe and the foam insulation of the pipe.
// This is intended for central Dud Shemesh systems there every apartment has a seperate Dud Shemesh that is fed by a central setup on the roof.
// The assumption is that the maximum return temperature will be an indicator of how full the Dud is.

LiquidCrystal_I2C lcd(0x27,16,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

OneWire oneWire(5);
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress;
// Current temperatures
float temp1, temp2;
// Maximum temperatures
float day_max_1, day_max_2;

DS3231 clock;
RTCDateTime rdt;
// Day of week of the last day - used for clearing after midnight
int last_day;

void setup()
{
  int countdownMS = Watchdog.enable(20000); // Enable watchdog
  clock.begin();
  // Use this for initial time setting (we don't care if it's off a few minutes - the time is only used for resetting the temperatures)
  //clock.setDateTime(__DATE__, __TIME__);
  // Temperature sensor initialization
  // On power up - set the last day to today - once the day is off, it will reset the maximum values
  rdt = clock.getDateTime();
  last_day = rdt.dayOfWeek;

  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, 12); // Use 12 bit resolution
  
  lcd.init();                      // initialize the lcd 
  lcd.init();
  // Print a splash screen message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("DudMonitor");
  lcd.setCursor(0,1);
  lcd.print("Initializing");
  lcd.setCursor(0,0);
  
  //LED
  pinMode(13, OUTPUT);
}


void loop()
{
sensors.requestTemperatures(); // Send the command to get temperatures
// Read temperature values
temp1 = sensors.getTempCByIndex(0);
temp2 = sensors.getTempCByIndex(1);

// If the day is different than the last_day then we have a new day - reset the max values and use the current day for last_day
rdt = clock.getDateTime();
if (last_day != rdt.dayOfWeek) {
  day_max_1 = 0;
  day_max_2 = 0;
  last_day = rdt.dayOfWeek;
}

// Determine maximum temperature 
if (temp2 > day_max_2) {
  day_max_2 = temp2;
  day_max_1 = temp1;
}

// First line: In-temperature, Out-temperature and difference
lcd.clear();
lcd.setCursor(0,0);
lcd.print(temp1);
lcd.setCursor(6,0);
lcd.print(temp2);
lcd.setCursor(12,0);
lcd.print(temp2 - temp1);
// Second line: same but max temperature
lcd.setCursor(0,1);
lcd.print(day_max_1);
lcd.setCursor(6,1);
lcd.print(day_max_2);
lcd.setCursor(12,1);
lcd.print(day_max_2 - day_max_1);

Watchdog.reset();
}
