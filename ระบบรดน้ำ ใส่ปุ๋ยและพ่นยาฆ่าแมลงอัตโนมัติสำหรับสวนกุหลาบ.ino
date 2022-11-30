
#define BLYNK_PRINT Serial

/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID   "TMPLNhHL4syp"

#define BLYNK_FIRMWARE_VERSION  "0.1.1"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <TimeLib.h>

BlynkTimer timer;
DHT dht;

#define TRUE                      (1)
#define FALSE                     (0)

// ################# What you need to modify #########################
char auth[] = "CLzJIQv-MQEYJcD9kegx4-nyuXQK8oJZ";

// Your WiFi credentials.
char ssid[] = "momay_2.4G";
char pass[] = "Mm0946214774";

// ################# Preprocessor ####################################
#define RELAY_ACTIVE_HIGH         (TRUE) 
#define WATERING_4_RELAY          (TRUE)
#define LINE_NOTIFY_SUPPORTED     (TRUE)

#define MAX_WORKING_TIME_SEC      (3600)  // 3600 Sec = 1 Hour  

// ###################################################################

#if LINE_NOTIFY_SUPPORTED
  #include <TridentTD_LineNotify.h> 
  #define LINE_TOKEN   "sX5rKLFJt90JIWQ8vagEByUzSZxOCFfEZIdSJazb1Ja"
#endif

#if WATERING_4_RELAY
  #define VALVE_1_OUT             D5
  #define VALVE_2_OUT             D6
  #define VALVE_3_OUT             D7
  #define VALVE_4_OUT             D8
  #define MOISURE_READ            A0
  #define DHT_READ_PIN            D0

  #define TIMER_NUMBER            2
  #define VALVE_NUMBER            4  
#else
  #define VALVE_1_OUT             D1
  #define VALVE_2_OUT             D2
  #define MOISURE_READ            A0
  #define DHT_READ_PIN            D3

  #define TIMER_NUMBER            2
  #define VALVE_NUMBER            2  
#endif

#if RELAY_ACTIVE_HIGH
  #define VALVE_ON                1
  #define VALVE_OFF               0
#else
  #define VALVE_ON                0
  #define VALVE_OFF               1
#endif

#define BLYNK_TEMP              V0
#define BLYNK_RH                V1
#define BLYNK_SOIL_MOISTURE     V2

#define BLYNK_VALVE1            V3
#define BLYNK_VALVE2            V4
#define BLYNK_VALVE3            V5
#define BLYNK_VALVE4            V6
#define BLYNK_ALL_VALVE         V7

#define BLYNK_TIMER_1_1         V8
#define BLYNK_TIMER_1_2         V9
#define BLYNK_TIMER_2_1         V10
#define BLYNK_TIMER_2_2         V11
#define BLYNK_TIMER_3_1         V12
#define BLYNK_TIMER_3_2         V13
#define BLYNK_TIMER_4_1         V14
#define BLYNK_TIMER_4_2         V15

#define BLYNK_EN_TIMER_1_1      V16
#define BLYNK_EN_TIMER_1_2      V17
#define BLYNK_EN_TIMER_2_1      V18
#define BLYNK_EN_TIMER_2_2      V19
#define BLYNK_EN_TIMER_3_1      V20
#define BLYNK_EN_TIMER_3_2      V21
#define BLYNK_EN_TIMER_4_1      V22
#define BLYNK_EN_TIMER_4_2      V23

int soil_moisture;
float humidity; 
float temperature; 

struct str_command
{
  unsigned char start_time_hour[TIMER_NUMBER];
  unsigned char start_time_min[TIMER_NUMBER];
  unsigned char stop_time_hour[TIMER_NUMBER];
  unsigned char stop_time_min[TIMER_NUMBER];
  unsigned char day_timer[TIMER_NUMBER];
  unsigned long timer_on_sec[TIMER_NUMBER];
  unsigned long working_time_sec;
  bool flag_timer_en[TIMER_NUMBER];
  bool flag_timer_on;
  bool flag_timer_set;
  bool flag_valve_set;
  bool flag_valve_cmd;
  bool flag_valve_status;
  bool flag_blynk_valve_update;

#if LINE_NOTIFY_SUPPORTED
  bool flag_line_valve_time_limited;
  bool flag_line_valve_update;
#endif
};

struct str_command valve[VALVE_NUMBER];

bool flag_blynk_guage_update;

bool flag_blynk_all_valve_set;
bool flag_blynk_all_valve_cmd;
bool flag_blynk_all_valve_update;


unsigned char rtc_hour;
unsigned char rtc_min;
unsigned char rtc_sec;
unsigned char rtc_weekday;

unsigned char server_hour;
unsigned char server_min;
unsigned char server_sec;
unsigned char server_weekday;

bool rtc_synchronized;

// ######################################################################
BLYNK_CONNECTED()
{
  Serial.print("BLYNK SERVER CONNECTED !!!");
  Blynk.syncVirtual(BLYNK_TIMER_1_1);
  Blynk.syncVirtual(BLYNK_TIMER_1_2);
  Blynk.syncVirtual(BLYNK_TIMER_2_1);
  Blynk.syncVirtual(BLYNK_TIMER_2_2);
  Blynk.syncVirtual(BLYNK_TIMER_3_1);
  Blynk.syncVirtual(BLYNK_TIMER_3_2);
  Blynk.syncVirtual(BLYNK_TIMER_4_1);
  Blynk.syncVirtual(BLYNK_TIMER_4_2);
  
  Blynk.syncVirtual(BLYNK_EN_TIMER_1_1);
  Blynk.syncVirtual(BLYNK_EN_TIMER_1_2);
  Blynk.syncVirtual(BLYNK_EN_TIMER_2_1);
  Blynk.syncVirtual(BLYNK_EN_TIMER_2_2);
  Blynk.syncVirtual(BLYNK_EN_TIMER_3_1);
  Blynk.syncVirtual(BLYNK_EN_TIMER_3_2);
  Blynk.syncVirtual(BLYNK_EN_TIMER_4_1);
  Blynk.syncVirtual(BLYNK_EN_TIMER_4_2);
  
  //Blynk.syncVirtual(BLYNK_VALVE1);
  //Blynk.syncVirtual(BLYNK_VALVE2);
  //Blynk.syncVirtual(BLYNK_VALVE3);
  //Blynk.syncVirtual(BLYNK_VALVE4);
  //Blynk.syncVirtual(BLYNK_ALL_VALVE);

  Blynk.sendInternal("rtc", "sync");
}

// ######################################################################
void blynk_valve_set(unsigned char ch, unsigned char value)
{
  valve[ch].flag_valve_set = value;
  valve[ch].flag_valve_cmd = 1;
  
  Serial.println("Valve " + String(ch+1) + " Set: " + String(value));
}

// ######################################################################
void blynk_timer_en_set(unsigned char ch, unsigned char idx, unsigned char value)
{
  valve[ch].flag_timer_en[idx] = value;
  Serial.println("Timer " + String(ch+1) + " Idx " + String(idx+1) + " Set: " + String(value));
}

// ######################################################################
void blynk_timer_set(unsigned char ch, unsigned char idx, unsigned char start_hr, unsigned char start_min,
                     unsigned char stop_hr, unsigned char stop_min, unsigned char dow)
{
  valve[ch].start_time_hour[idx] = start_hr;
  valve[ch].start_time_min[idx] = start_min;
  valve[ch].stop_time_hour[idx] = stop_hr;
  valve[ch].stop_time_min[idx] = stop_min;
  valve[ch].day_timer[idx] = dow;

  Serial.println();
  Serial.println("Timer " + String(ch+1) + " Idx " + String(idx + 1) + ":");
  Serial.println("Start Time: " + String(start_hr) + ":" + String(start_min));
  Serial.println("Stop Time: " + String(stop_hr) + ":" + String(stop_min));
  Serial.print("Day set: ");
  Serial.println(dow, HEX);
  Serial.println();
  
}

// ######################################################################
BLYNK_WRITE (BLYNK_VALVE1)
{
  int val = param.asInt();  // assigning incomming value from pin to a var
  blynk_valve_set(0, val);
}

// ######################################################################
BLYNK_WRITE (BLYNK_VALVE2)
{
  int val = param.asInt();  // assigning incomming value from pin to a var
  blynk_valve_set(1, val);
}

// ######################################################################
#if WATERING_4_RELAY
  BLYNK_WRITE (BLYNK_VALVE3)
  {
    int val = param.asInt();  // assigning incomming value from pin to a var
    blynk_valve_set(2, val);
  }

// ######################################################################
  BLYNK_WRITE (BLYNK_VALVE4)
  {
    int val = param.asInt();  // assigning incomming value from pin to a var
    blynk_valve_set(3, val);
  }
#endif
// ######################################################################
BLYNK_WRITE (BLYNK_ALL_VALVE)
{
  int val = param.asInt();  // assigning incomming value from pin to a var
  
  flag_blynk_all_valve_set = val;
  flag_blynk_all_valve_cmd = 1;
}

// ######################################################################
BLYNK_WRITE (BLYNK_EN_TIMER_1_1)
{
  int val = param.asInt();  // assigning incomming value from pin to a var
  blynk_timer_en_set(0,0,val);
}

// ######################################################################
BLYNK_WRITE (BLYNK_EN_TIMER_1_2)
{
  int val = param.asInt();  // assigning incomming value from pin to a var
  blynk_timer_en_set(0,1,val);
}

// ######################################################################
BLYNK_WRITE (BLYNK_EN_TIMER_2_1)
{
  int val = param.asInt();  // assigning incomming value from pin to a var
  blynk_timer_en_set(1,0,val);
}

// ######################################################################
BLYNK_WRITE (BLYNK_EN_TIMER_2_2)
{
  int val = param.asInt();  // assigning incomming value from pin to a var
  blynk_timer_en_set(1,1,val);
}

// ######################################################################
#if WATERING_4_RELAY
  BLYNK_WRITE (BLYNK_EN_TIMER_3_1)
  {
    int val = param.asInt();  // assigning incomming value from pin to a var
    blynk_timer_en_set(2,0,val);
  }

// ######################################################################
  BLYNK_WRITE (BLYNK_EN_TIMER_3_2)
  {
    int val = param.asInt();  // assigning incomming value from pin to a var
    blynk_timer_en_set(2,1,val);
  }

// ######################################################################
  BLYNK_WRITE (BLYNK_EN_TIMER_4_1)
  {
    int val = param.asInt();  // assigning incomming value from pin to a var
    blynk_timer_en_set(3,0,val);
  }

// ######################################################################
  BLYNK_WRITE (BLYNK_EN_TIMER_4_2)
  {
    int val = param.asInt();  // assigning incomming value from pin to a var
    blynk_timer_en_set(3,1,val);
  }
#endif
// ######################################################################
BLYNK_WRITE(BLYNK_TIMER_1_1)
{
  unsigned char week_day;
  unsigned char start_hr, start_min, stop_hr, stop_min;
  
  TimeInputParam  t(param);
  
  if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
  {
     start_hr = t.getStartHour();
     start_min = t.getStartMinute();
    
     stop_hr = t.getStopHour();
     stop_min = t.getStopMinute();

     week_day = 0;
     
     for (int i = 1; i <= 7; i++)
     {
       if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
         week_day |= (0x01 << (i-1));
     }

     blynk_timer_set(0, 0, start_hr, start_min, stop_hr, stop_min, week_day);
  }
  else
  {
    Serial.println("Disabled Timer 1-1");
  }
}

// ######################################################################
BLYNK_WRITE(BLYNK_TIMER_1_2)
{
  unsigned char week_day;
  unsigned char start_hr, start_min, stop_hr, stop_min;
  
  TimeInputParam  t(param);
  
  if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
  {
     start_hr = t.getStartHour();
     start_min = t.getStartMinute();
    
     stop_hr = t.getStopHour();
     stop_min = t.getStopMinute();

     week_day = 0;
     
     for (int i = 1; i <= 7; i++)
     {
       if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
         week_day |= (0x01 << (i-1));
     }

     blynk_timer_set(0, 1, start_hr, start_min, stop_hr, stop_min, week_day);
  }
  else
  {
    Serial.println("Disabled Timer 1-2");
  }
}

// ######################################################################
BLYNK_WRITE(BLYNK_TIMER_2_1)
{
  unsigned char week_day;
  unsigned char start_hr, start_min, stop_hr, stop_min;
  
  TimeInputParam  t(param);
  
  if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
  {
     start_hr = t.getStartHour();
     start_min = t.getStartMinute();
    
     stop_hr = t.getStopHour();
     stop_min = t.getStopMinute();

     week_day = 0;
     
     for (int i = 1; i <= 7; i++)
     {
       if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
         week_day |= (0x01 << (i-1));
     }

     blynk_timer_set(1, 0, start_hr, start_min, stop_hr, stop_min, week_day);
  }
  else
  {
    Serial.println("Disabled Timer 2-1");
  }
}

// ######################################################################
BLYNK_WRITE(BLYNK_TIMER_2_2)
{
  unsigned char week_day;
  unsigned char start_hr, start_min, stop_hr, stop_min;
  
  TimeInputParam  t(param);
  
  if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
  {
     start_hr = t.getStartHour();
     start_min = t.getStartMinute();
    
     stop_hr = t.getStopHour();
     stop_min = t.getStopMinute();

     week_day = 0;
     
     for (int i = 1; i <= 7; i++)
     {
       if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
         week_day |= (0x01 << (i-1));
     }

     blynk_timer_set(1, 1, start_hr, start_min, stop_hr, stop_min, week_day);
  }
  else
  {
    Serial.println("Disabled Timer 2-2");
  }
}

// ######################################################################
#if WATERING_4_RELAY
  BLYNK_WRITE(BLYNK_TIMER_3_1)
  {
    unsigned char week_day;
    unsigned char start_hr, start_min, stop_hr, stop_min;
    
    TimeInputParam  t(param);
    
    if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
    {
       start_hr = t.getStartHour();
       start_min = t.getStartMinute();
      
       stop_hr = t.getStopHour();
       stop_min = t.getStopMinute();
  
       week_day = 0;
       
       for (int i = 1; i <= 7; i++)
       {
         if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
           week_day |= (0x01 << (i-1));
       }
  
       blynk_timer_set(2, 0, start_hr, start_min, stop_hr, stop_min, week_day);
    }
    else
    {
      Serial.println("Disabled Timer 3-1");
    }
  }

// ######################################################################
  BLYNK_WRITE(BLYNK_TIMER_3_2)
  {
    unsigned char week_day;
    unsigned char start_hr, start_min, stop_hr, stop_min;
    
    TimeInputParam  t(param);
    
    if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
    {
       start_hr = t.getStartHour();
       start_min = t.getStartMinute();
      
       stop_hr = t.getStopHour();
       stop_min = t.getStopMinute();
  
       week_day = 0;
       
       for (int i = 1; i <= 7; i++)
       {
         if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
           week_day |= (0x01 << (i-1));
       }
  
       blynk_timer_set(2, 1, start_hr, start_min, stop_hr, stop_min, week_day);
    }
    else
    {
      Serial.println("Disabled Timer 3-2");
    }
  }

// ######################################################################
  BLYNK_WRITE(BLYNK_TIMER_4_1)
  {
    unsigned char week_day;
    unsigned char start_hr, start_min, stop_hr, stop_min;
    
    TimeInputParam  t(param);
    
    if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
    {
       start_hr = t.getStartHour();
       start_min = t.getStartMinute();
      
       stop_hr = t.getStopHour();
       stop_min = t.getStopMinute();
  
       week_day = 0;
       
       for (int i = 1; i <= 7; i++)
       {
         if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
           week_day |= (0x01 << (i-1));
       }
  
       blynk_timer_set(3, 0, start_hr, start_min, stop_hr, stop_min, week_day);
    }
    else
    {
      Serial.println("Disabled Timer 4-1");
    }
  }

// ######################################################################
  BLYNK_WRITE(BLYNK_TIMER_4_2)
  {
    unsigned char week_day;
    unsigned char start_hr, start_min, stop_hr, stop_min;
    
    TimeInputParam  t(param);
    
    if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
    {
       start_hr = t.getStartHour();
       start_min = t.getStartMinute();
      
       stop_hr = t.getStopHour();
       stop_min = t.getStopMinute();
  
       week_day = 0;
       
       for (int i = 1; i <= 7; i++)
       {
         if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
           week_day |= (0x01 << (i-1));
       }
  
       blynk_timer_set(3, 1, start_hr, start_min, stop_hr, stop_min, week_day);
    }
    else
    {
      Serial.println("Disabled Timer 4-2");
    }
  }
#endif
// ######################################################################
BLYNK_WRITE(InternalPinRTC) 
{
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  unsigned long blynkTime = param.asLong(); 
  
  if (blynkTime >= DEFAULT_TIME) 
  {
    setTime(blynkTime);

    server_hour = hour();
    server_min = minute();
    server_sec = second();
    
    server_weekday = weekday();
  
    if ( server_weekday == 1 )
      server_weekday = 7;
    else
      server_weekday -= 1; 
      
    rtc_synchronized = 1;
  }
}

// ######################################################################
unsigned char time_10_sec;
void checkTime() 
{
  unsigned long start_time_sec_buf;
  unsigned long stop_time_sec_buf;
  unsigned long server_time_sec;
  unsigned long real_time_sec;
  unsigned char i, j;
  bool flag_timer_on_buf;
  bool flag_timer_on_temp;
    
  time_10_sec++;
  if(time_10_sec >= 10)
  {
    time_10_sec = 0;
    Blynk.sendInternal("rtc", "sync"); 
  }

  // แปลงเวลาให้อยู่ในรูปแบบของวินาที
  server_time_sec = (server_hour * 60 * 60) + (server_min * 60) + server_sec; 
  real_time_sec = (rtc_hour * 60 * 60) + (rtc_min * 60) + rtc_sec;

  // ซิงค์เวลากับ Server เมื่อเวลาปัจจุบันของบอร์ด มีค่าต่างจาก Server มากกว่า 30 วินาที
  if   ( /*Blynk.connected()*/ rtc_synchronized && 
       (!(server_hour == 23 && server_min == 59 && rtc_hour == 0)) &&
       (((server_time_sec > real_time_sec) && ((server_time_sec - real_time_sec) > 30)) || 
       ((server_time_sec < real_time_sec) && ((real_time_sec - server_time_sec) > 30))) )
       {
          rtc_hour = server_hour;
          rtc_min = server_min;
          rtc_sec = server_sec;
          rtc_weekday = server_weekday;
       }

  // เวลาปัจจุบันของบอร์ด
  rtc_sec++;
  if (rtc_sec >= 60)
  {
    rtc_sec = 0;
    rtc_min++;
    if (rtc_min >= 60)
    {
      rtc_min = 0;
      rtc_hour++;
      if (rtc_hour >= 24)
      {
        rtc_hour = 0;
        rtc_weekday++;
        if ( rtc_weekday > 7 )
         rtc_weekday = 1;
      }
    }
  }

  // ส่งค่าเวลาต่างๆ ออกทาง Serial monitor เพื่อตรวจสอบ
  Serial.println("Server time: " + String(server_hour) + ":" + String(server_min) + ":" + String(server_sec));
  Serial.println(String("Server Weekday: ") + String(server_weekday));
  Serial.println("Real Time: " + String(rtc_hour) + ":" + String(rtc_min) + ":" + String(rtc_sec));
  Serial.println(String("RTC Weekday: ") + String(rtc_weekday));
  Serial.print("Blynk connection status: ");
  Serial.println (Blynk.connected());
  Serial.print("MAX VALVES : ");
  Serial.println(VALVE_NUMBER);
  Serial.println();

  // Count down timer for stop
  for (i=0; i<VALVE_NUMBER; i++)
  {
    flag_timer_on_buf = valve[i].flag_timer_on; 
    flag_timer_on_temp = 0;
    
    for (j=0; j<TIMER_NUMBER; j++)
    {
      if ( valve[i].timer_on_sec[j] )
      {
        valve[i].timer_on_sec[j]--;
      }

      if ( valve[i].timer_on_sec[j] )
        flag_timer_on_temp = 1;      
    }

    if ( flag_timer_on_temp == 0 )
    {
      valve[i].flag_timer_on = 0;
    }

    if ( flag_timer_on_buf != valve[i].flag_timer_on )
      valve[i].flag_timer_set = 1;
  }

  // Check for start
  for (i=0; i<VALVE_NUMBER; i++)
  {
    for (j=0; j<TIMER_NUMBER; j++)
    {      
      if ( /*rtc_synchronized &&*/ valve[i].flag_timer_en[j] &&  
           (valve[i].day_timer[j] == 0x00 || (valve[i].day_timer[j] & (0x01 << (rtc_weekday - 1) ))) &&
           (( rtc_hour == valve[i].start_time_hour[j] ) && ( rtc_min == valve[i].start_time_min[j] ) && (rtc_sec == 0 )) )
          {
            start_time_sec_buf = (valve[i].start_time_hour[j] * 60 * 60) + (valve[i].start_time_min[j] * 60);
            stop_time_sec_buf = (valve[i].stop_time_hour[j] * 60 * 60) + (valve[i].stop_time_min[j] * 60);
  
            // คำนวณเวลาที่ต้องทำงาน หน่วยเป็นวินาที
            if ( stop_time_sec_buf >= start_time_sec_buf )
            {
              valve[i].timer_on_sec[j] = stop_time_sec_buf - start_time_sec_buf;
            }
            else
            {
              valve[i].timer_on_sec[j] = (stop_time_sec_buf + (24 * 60 * 60)) - start_time_sec_buf;
            }  
            
            valve[i].flag_timer_on = 1;
            valve[i].flag_timer_set = 1;
          }                       
    }
  }

  // Check for working time limitation
  if ( MAX_WORKING_TIME_SEC > 0 )
  {
    for (i=0; i<VALVE_NUMBER; i++)
    {
      if ( valve[i].flag_valve_status )
      {
        valve[i].working_time_sec++;
        if ( valve[i].working_time_sec >= MAX_WORKING_TIME_SEC )
        {
          valve[i].flag_valve_set = 0;
          valve[i].flag_valve_cmd = 1;
#if LINE_NOTIFY_SUPPORTED
          valve[i].flag_line_valve_time_limited = 1;
#endif          
        }
      }
      else
      {
        valve[i].working_time_sec = 0;
      }
    }
  }

  // To re-synchronize time again when server is connected
  if ( !Blynk.connected() )
    rtc_synchronized = 0;
      
  humidity = dht.getHumidity(); 
  temperature = dht.getTemperature(); 

  soil_moisture = analogRead(MOISURE_READ);
  soil_moisture = 1023 - soil_moisture; //map(soil_moisture, 1023, 0, 0, 1023);

  flag_blynk_guage_update = 1;
}

// ######################################################################
void fn_valve_mng (void)
{
  bool time_set_overflow;
  long start_timer_sec;
  long stop_timer_sec;
  bool flag_timer_on_buf[TIMER_NUMBER];
  bool flag_timer_on_temp;
  unsigned char i,j;
  unsigned char valve_on_count;

  for (i=0; i<VALVE_NUMBER; i++)
  {    
    if ( flag_blynk_all_valve_cmd )
    {
      valve[i].flag_valve_status = flag_blynk_all_valve_set;
      valve[i].flag_valve_set = flag_blynk_all_valve_set;
      valve[i].flag_blynk_valve_update = 1;
      valve[i].flag_valve_cmd = 0;

      // cancel timers
      valve[i].flag_timer_on = 0;
      valve[i].flag_timer_set = 0;
      for (j=0; j<TIMER_NUMBER; j++)
      {
        valve[i].timer_on_sec[j] = 0;
      }     
    }
    else if ( valve[i].flag_valve_cmd )
    {
      valve[i].flag_valve_cmd = 0;
      valve[i].flag_valve_status = valve[i].flag_valve_set;
      valve[i].flag_blynk_valve_update = 1;
      
      // cancel timers
      valve[i].flag_timer_on = 0;
      valve[i].flag_timer_set = 0;
      for (j=0; j<TIMER_NUMBER; j++)
      {
        valve[i].timer_on_sec[j] = 0;
      }         
    }
    else if ( valve[i].flag_timer_set )
    {
      valve[i].flag_timer_set = 0;
      valve[i].flag_blynk_valve_update = 1;  

#if LINE_NOTIFY_SUPPORTED
      valve[i].flag_line_valve_update = 1;
#endif

      if ( valve[i].flag_timer_on )
      {         
        valve[i].flag_valve_status = 1;
      }
      else
      {
        valve[i].flag_valve_status = 0;
      }      
    }

    // how many valves working
    if ( valve[i].flag_valve_status )
      valve_on_count++;
  }

   flag_blynk_all_valve_cmd = 0;

  // check for update ALL VALVES button
  if ( flag_blynk_all_valve_set == 0 )
  {
    if ( valve_on_count == VALVE_NUMBER ) // ALL VALVES = OFF but every valves = ON
    {
      flag_blynk_all_valve_set = 1;
      flag_blynk_all_valve_update = 1;
    }
  }
  else
  {
    if ( valve_on_count == 0 ) // ALL VALVES = ON but every valves = OFF
    {
      flag_blynk_all_valve_set = 0;
      flag_blynk_all_valve_update = 1;
    }    
  }

  // HARDWARE CONTROL
#if RELAY_ACTIVE_HIGH
  digitalWrite(VALVE_1_OUT, valve[0].flag_valve_status);  // Relay active HIGH
  digitalWrite(VALVE_2_OUT, valve[1].flag_valve_status);  // Relay active HIGH
  #if WATERING_4_RELAY
    digitalWrite(VALVE_3_OUT, valve[2].flag_valve_status);  // Relay active HIGH
    digitalWrite(VALVE_4_OUT, valve[3].flag_valve_status);  // Relay active HIGH
  #endif
#else
  digitalWrite(VALVE_1_OUT, !valve[0].flag_valve_status);  // Relay active LOW
  digitalWrite(VALVE_2_OUT, !valve[1].flag_valve_status);  // Relay active LOW
  #if WATERING_4_RELAY
    digitalWrite(VALVE_3_OUT, !valve[2].flag_valve_status);  // Relay active LOW
    digitalWrite(VALVE_4_OUT, !valve[3].flag_valve_status);  // Relay active LOW
  #endif
#endif
}

// ######################################################################
void update_blynk_data(void)
{
  if ( flag_blynk_guage_update )
  {
    flag_blynk_guage_update = 0;
    Blynk.virtualWrite(BLYNK_TEMP, temperature);
    Blynk.virtualWrite(BLYNK_RH, humidity);
    Blynk.virtualWrite(BLYNK_SOIL_MOISTURE, soil_moisture);
  }
  
  if ( valve[0].flag_blynk_valve_update )
  {
    valve[0].flag_blynk_valve_update = 0;
    Blynk.virtualWrite(BLYNK_VALVE1, valve[0].flag_valve_status);
  }

  if ( valve[1].flag_blynk_valve_update )
  {
    valve[1].flag_blynk_valve_update = 0;
    Blynk.virtualWrite(BLYNK_VALVE2, valve[1].flag_valve_status);
  }

#if WATERING_4_RELAY
  if ( valve[2].flag_blynk_valve_update )
  {
    valve[2].flag_blynk_valve_update = 0;
    Blynk.virtualWrite(BLYNK_VALVE3, valve[2].flag_valve_status);
  }

  if ( valve[3].flag_blynk_valve_update )
  {
    valve[3].flag_blynk_valve_update = 0;
    Blynk.virtualWrite(BLYNK_VALVE4, valve[3].flag_valve_status);
  }
#endif

  if ( flag_blynk_all_valve_update )
  {
    flag_blynk_all_valve_update = 0;
    Blynk.virtualWrite(BLYNK_ALL_VALVE, flag_blynk_all_valve_set);
  }
}

// ######################################################################
#if LINE_NOTIFY_SUPPORTED
void update_line_notify()
{
  String Line_msg_buff = "";
  
  for (int i=0; i<VALVE_NUMBER; i++)
  {
    if ( valve[i].flag_line_valve_time_limited )
    {
      valve[i].flag_line_valve_time_limited = 0;
      Line_msg_buff = "Valve " + String(i+1) + " OFF เนื่องจากทำงานเกินเวลาที่กำหนด";
      LINE.notify(Line_msg_buff);    
      break;  
    }
    
    if ( valve[i].flag_line_valve_update )
    {
      valve[i].flag_line_valve_update = 0;
      Line_msg_buff = "Valve " + String(i+1);
      if ( valve[i].flag_valve_status )
        Line_msg_buff += " ON";
      else
        Line_msg_buff += " OFF";
      LINE.notify(Line_msg_buff); 
      break;
    }
  }
}
#endif

// ######################################################################
void setup()
{
  pinMode(VALVE_1_OUT, OUTPUT);
  pinMode(VALVE_2_OUT, OUTPUT);
  
#if WATERING_4_RELAY  
  pinMode(VALVE_3_OUT, OUTPUT);
  pinMode(VALVE_4_OUT, OUTPUT);
#endif

  pinMode(MOISURE_READ, INPUT);

  digitalWrite(VALVE_1_OUT, VALVE_OFF);  
  digitalWrite(VALVE_2_OUT, VALVE_OFF);  

#if WATERING_4_RELAY  
  digitalWrite(VALVE_3_OUT, VALVE_OFF);  
  digitalWrite(VALVE_4_OUT, VALVE_OFF);  
#endif
    
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
 
   // DHT initialization
  dht.setup(DHT_READ_PIN); 

#if LINE_NOTIFY_SUPPORTED
    // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);
  // ตัวอย่างส่งข้อความ
  LINE.notify("อุปกรณ์รดน้ำต้นไม้เชื่อมต่อ WiFi สำเร็จ");
#endif

  timer.setInterval(1000L, checkTime);  // กำหนดให้ checkTime ถูกเรียกทุกๆ 1 วินาที
    
}

// ######################################################################
void loop()
{  
  Blynk.run();
  timer.run(); 
  fn_valve_mng();
  update_blynk_data();
#if LINE_NOTIFY_SUPPORTED
  update_line_notify();
#endif
  delay(50);
}
