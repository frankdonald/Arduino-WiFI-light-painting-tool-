#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(25, PIN, NEO_GRB + NEO_KHZ800);
uint8_t pixel_count;

char Input_buffer[450];
boolean command_flag=false;
int i,j;
boolean stringComplete=false;
boolean request_processed=false;
boolean WiFiConnect=false;
uint8_t command_count=0;
char send_bytes[]="AT+CIPSEND= ,843\r\n";
boolean client_request=false;
boolean flag=true;
char connection_id;
uint8_t page_input_pos;
uint8_t RGB[3];

void setup() {
  strip.begin();
  strip.show();
  pinMode(12, OUTPUT);
  digitalWrite(12,LOW);
  Serial.begin(9600);
  delay(5000);
  Serial.print("AT+RST\r\n");
  delay(200);
 while(WiFiConnect==false)                             //Wait till Connecting to WiFi hotspot
  {
    serial_check();
    WiFiCheck();                                        //WiFi check subroutine
  }
  
  while(command_flag==false)                           //Looping for commands 
  {
    command_input();
  }
  clear_buffer();                                        //Clearing the array
  while(client_request==false)                          //Waiting for any client/browser to initiate the request
  {
    serial_check();
    brow_req();                                         //sub routine to check browser request
    brow_resp();                                        //sub routine for response from Arduino
  }
  flag=false;
  digitalWrite(12,LOW);
}

void loop() {
  serial_check();
  page_input_pos=Search_webrequest();                  //Monitor webpage indefinitely
  activate();
}


void activate()
{
  if(flag==true)
  {
    if(Input_buffer[page_input_pos]=='R'&&Input_buffer[page_input_pos+1]=='e')
    drawColors(255,0,0);
    else if(Input_buffer[page_input_pos]=='G'&&Input_buffer[page_input_pos+1]=='r')
    drawColors(0,255,0);
    else if(Input_buffer[page_input_pos]=='B'&&Input_buffer[page_input_pos+1]=='l')
    drawColors(0,0,255);
    else if(Input_buffer[page_input_pos]=='W'&&Input_buffer[page_input_pos+1]=='h')
    drawColors(255,255,255);
    else if(Input_buffer[page_input_pos]=='G'&&Input_buffer[page_input_pos+1]=='o')
    drawColors(255, 190, 0);
    else if(Input_buffer[page_input_pos]=='R'&&Input_buffer[page_input_pos+1]=='a')
    rainbow();
    else if(Input_buffer[page_input_pos]=='?')
    custom_color();
    else if(Input_buffer[page_input_pos]=='O'&&Input_buffer[page_input_pos+1]=='f')
    drawColors(0,0,0);
  }
}

void drawColors(uint8_t R,uint8_t G,uint8_t B)
{
  for(pixel_count=0;pixel_count<strip.numPixels(); pixel_count++)
  strip.setPixelColor(pixel_count,R,G,B);
  strip.show();
}

void rainbow()
{
 for(pixel_count=0;pixel_count<strip.numPixels(); pixel_count++)
 {
  if(pixel_count==0||pixel_count==7||pixel_count==14||pixel_count==21)
  strip.setPixelColor(pixel_count, 148, 0, 211);
  else if(pixel_count==1||pixel_count==8||pixel_count==15||pixel_count==22)
  strip.setPixelColor(pixel_count, 75, 0, 130);
  else if(pixel_count==2||pixel_count==9||pixel_count==16||pixel_count==23)
  strip.setPixelColor(pixel_count, 0, 0, 255);
  else if(pixel_count==3||pixel_count==10||pixel_count==17||pixel_count==24)
  strip.setPixelColor(pixel_count, 0, 255,0);
  else if(pixel_count==4||pixel_count==11||pixel_count==18)
  strip.setPixelColor(pixel_count, 255, 255, 0);
  else if(pixel_count==5||pixel_count==12||pixel_count==19)
  strip.setPixelColor(pixel_count, 255, 127,0);
  else if(pixel_count==6||pixel_count==13||pixel_count==20)
  strip.setPixelColor(pixel_count, 255, 0, 0);
 }
 strip.show();
}

void custom_color()
{
  command_count=0;
  command_count=page_input_pos+1;
  for(;Input_buffer[command_count]!=' ';command_count++)
  {
    if(Input_buffer[command_count]=='R')
    RGB[0]=values('&');
    else if(Input_buffer[command_count]=='G')
    RGB[1]=values('&');
    else if(Input_buffer[command_count]=='B')
    RGB[2]=values(' ');
  }
  drawColors(RGB[0],RGB[1],RGB[2]);
  strip.show();
}

int values(char a)
{
  i=j=0;
  char numbers[3];
  i=command_count+2;
  while(Input_buffer[i]!=a)
  {
    numbers[j]=Input_buffer[i];
    i++;
    j++;
  }
  return atoi(numbers);
}

void WiFiCheck()                                        //Check whether WiFi is connected 
{
  if(stringComplete==true&&Input_buffer[j-4]=='I'
  )   //Check for the status WIFI GOT IP 
  {
   clear_buffer();
   WiFiConnect=true;
   delay(500);
  }
  else
  {
    clear_buffer();
    stringComplete=false;
  }
}

int Search_webrequest()                                   //repeated loop to check button inputs from Webpage
{
  if(stringComplete==true)
  {
    for(i=0;i<j-2;i++)
    {
      if(Input_buffer[i]=='G'&&Input_buffer[i+1]=='E'&&Input_buffer[i+2]=='T')
      {
        flag=true;
        return i+5;
      }
    }
  }
}

void brow_req()                                               //sub routine to monitor the browser request
{
  if(stringComplete==true&&request_processed==false)          //Checking for presence of char in input buffer and check whether the client request has already processed
   {
      if(flag==true)
      connection_id=(char)Input_buffer[0];                    
      send_bytes[11]=connection_id;                           //Adding connection ID to the CIPSEND command
      Serial.print(send_bytes);
      clear_buffer();
      delay(1000);  
      request_processed=true;                                 //Changing the flag to true indicating that Client request is processed
   }
}

void brow_resp()                                             //Sub routine to respond to client request
{
  if(request_processed==true&&stringComplete==true)         //Checking the flag on client request process
  {
    serial_check();
    if(Input_buffer[j-2]=='>')                              //Checking for the Send data signal from ESP  module
    {
      memset(Input_buffer,'\0', sizeof(Input_buffer));
      Serial.print("<html><body><h1>Light Painting</h1><h2>Colors</h2><p><a href=\"Red\"><button><b>RED</b></button></a></br></p><p><a href=\"Gre\"><button><b>GREEN</b></button></a></br></p><p><a href=\"Blu\"><button><b>BLUE</b></button></a></br></p><p><a href=\"Whi\"><button><b>WHITE</b></button></a></br></p><p><a href=\"Gol\"><button><b>GOLD</b></button></a></br></p><p><a href=\"Rai\"><button><b>RAINBOW</b></button></a></br></p><h2>Custom Color input</h2><form method=\"GET\"><div><label for='R'>Red</label><input name='R' id='R' type=\"number\" min=\"0\" max=\"255\"></div><div><label for='G'>Green</label><input name='G' id='G' type=\"number\" min=\"0\" max=\"255\"></div><div><label for='B'>Blue</label><input name='B' id='B' type=\"number\" min=\"0\" max=\"255\"></div><br/><br/><button>Submit</button></form><br/><p><a href=\"Off\"><button><b>TURN OFF</b></button></a></p></body></html>");                                         //Sending webpage code to Client browser 
      delay(2000);
      serial_check();
      //while(command_response_check==false);
      clear_buffer();
      request_processed=false;
      client_request=true;
    }
    else
    {
      clear_buffer();
      request_processed=false;                             //If request response didnt turn out successful mark the flags so that another CIPSEND command can be passed
      flag=false;   
    }
  }
}

void command_input()                                      //Sub routine for commands to create server using ESP module
{
  serial_check();
  if(command_response_check(Input_buffer)==true)          //Checking the response and Tracking the counts of command to handle in case of error response from ESP
  {command_count=command_count+1;}
  else 
  delay(1000);
  clear_buffer();
  switch(command_count)                                  //Enter commands sequentially in creating server
  {
    case 0: { Serial.print("AT\r\n");
              delay(500);
              break;
              }
    case 1: { 
              Serial.print("AT+CWMODE=3\r\n");
              delay(1000);
              break;}   
    case 2: {
              Serial.print("AT+CIPSTA=\"192.168.43.253\"\r\n");
              delay(1000);
              break; 
              }
    case 3: {
              Serial.print("AT+CIPMUX=1\r\n");
              delay(1000);
              break;
            }
    case 4: {
              Serial.print("AT+CIPSERVER=1,80\r\n");
              delay(1000);
              break;
            }
    case 5: {
             command_flag=true;
             digitalWrite(12,HIGH);
             break;  
            }
  }
}

void clear_buffer()                                             //Clearing buffer
{
  memset(Input_buffer, '\0', sizeof(Input_buffer));
  stringComplete=false;
}

boolean command_response_check(char *a)                         //Checking for OK Response from ESP
{
  if(a[j-4]=='O'&&a[j-3]=='K'&&a[j-2]=='\r'&&a[j-1]=='\n')
  return true;
  else
  { delay(1000);
    return false; }  
}

void serial_check() {                                           //Serial char available check
  i=0;
  while (Serial.available()) {
    delay(2);
    char inChar = Serial.read();
    Input_buffer[i]=(char)inChar;
    i=i+1;
    stringComplete = true;
    j=i;
   }       
}