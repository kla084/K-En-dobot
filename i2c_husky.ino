#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

HUSKYLENS huskylens;
//HUSKYLENS green line >> SDA; blue line >> SCL
void printResult(HUSKYLENSResult result);

int max_items_on_convoy = 7; //จำนวน items มากที่สุดบน convoy
int max_color = 4; //จำนวนสีที่ตองแยก

int count[4] ;// จำนวน items ต่อสี
int total_count = 0; //จำนวน items ทั้งหมด
int i;
int xlower = 100; //จอ 320 x 240 pixel
int xupper = 160;

bool detect[4] ;
bool SW_Down[4];
bool SW_Positive[4], SW_Mem_Positive[4];
bool SW_Negative[4], SW_Mem_Negative[4] = {1,1,1,1};

unsigned long previous_millis[7];
bool timer_flag[7] ; //flag จับเวลา
unsigned long time_out = 30000;
int order; // ลำดับในคิว 

struct ITEMS_INFO {
  int id;
  int x;
  int y;
  unsigned long elapsed_time;
  int counter;

} item_info[7];

void Switch() {
  for (int i = 0 ; i < max_color ; i ++ ){
    SW_Positive[i] = 0 ;
    SW_Negative[i] = 0 ;
  }

    for (int i = 0 ; i < max_color ; i ++ ){
        SW_Down[i]  = detect[i];

        if (SW_Mem_Positive[i]  == 0 && SW_Down[i]  == 1 ) {
          SW_Positive[i]  = 1 ; 
        }
        SW_Mem_Positive[i]  = SW_Down [i] ;

        if (SW_Mem_Negative[i]  == 0 && SW_Down[i]  == 0 ) {
          SW_Negative[i]  = 1 ; count[i]++; total_count++;
        }
        SW_Mem_Negative[i]  = !SW_Down[i]  ;
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    while (!huskylens.begin(Wire))
    {
        Serial.println(F("Begin failed!"));
        Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
        Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
}


void loop() {
    Switch();

    order = (total_count-1) % (max_items_on_convoy);

    for (i=0; i < max_items_on_convoy;i++){

      if (timer_flag[i] == 0){
        previous_millis[i] = millis();
        item_info[i] = {0};
      }
      else if (millis()-previous_millis[i] >= time_out) {
        timer_flag[i] = 0;
      }
      else{
        item_info[i].elapsed_time =  millis()-previous_millis[i];
      }

    }

    if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    
    else
    {
        HUSKYLENSResult result = huskylens.read();
        switch (result.ID){
          case 1:
            if (result.xCenter > xlower && result.xCenter <= xupper){
              detect[0] = true;
            } else {
              detect[0] = false;
            }
            // if (SW_Negative[0] == 1){
            //   timer_flag[order] = 1;
            //   item_info[order].id = result.ID;
            //   item_info[order].x = result.xCenter;
            //   item_info[order].y = result.yCenter;
            //   item_info[order].counter = count[0];

            // }      

            break;
          case 2:
            if (result.xCenter > xlower && result.xCenter <= xupper){
              detect[1] = true;
            } else {
              detect[1] = false;
            }
            // if (SW_Negative[1] == 1){
            //   timer_flag[order] = 1;
            //   item_info[order].id = result.ID;
            //   item_info[order].x = result.xCenter;
            //   item_info[order].y = result.yCenter;
            //   item_info[order].counter = count[1];

            // }   

            
            break;
          case 3:
            if (result.xCenter > xlower && result.xCenter <= xupper){
              detect[2] = true;
            } else {
              detect[2] = false;
            }
            // if (SW_Negative[2] == 1){
            //   timer_flag[order] = 1;
            //   item_info[order].id = result.ID;
            //   item_info[order].x = result.xCenter;
            //   item_info[order].y = result.yCenter;
            //   item_info[order].counter = count[2];

            // }   
            break;
          case 4:
            if (result.xCenter > xlower && result.xCenter <= xupper){
              detect[3] = true;
            } else {
              detect[3] = false;
            }
            // if (SW_Negative[3] == 1){
            //   timer_flag[order] = 1;
            //   item_info[order].id = result.ID;
            //   item_info[order].x = result.xCenter;
            //   item_info[order].y = result.yCenter;
            //   item_info[order].counter = count[3];

            // }   
            break;

          default:
            break;
        }

        for(i=0;i<max_color;i++){  //กรณีของไม่กระจุก 
          if (SW_Negative[i] == 1){
              timer_flag[order] = 1;
              item_info[order].id = result.ID;
              item_info[order].x = result.xCenter;
              item_info[order].y = result.yCenter;
              item_info[order].counter = count[i];

            }             
        }


        printResult(result);

        //ตกแต่งหน้าจอ
        for(i=0;i<10;i++){
          huskylens.customText("|",160,i*24); // เส้น detect
        }
        huskylens.customText(String()+millis(),0,0);
        // huskylens.customText(String()+"Red : "+ count[0],0,24);
        // huskylens.customText(String()+"Orange : "+ count[1],0,48);
        // huskylens.customText(String()+"Green : "+ count[2],240,0);
        // huskylens.customText(String()+"Purple : "+ count[3],240,24);
        // huskylens.customText(String()+"Total : "+ total_count,240,48);
        huskylens.customText(String()+ result.xCenter + "," + result.yCenter, result.xCenter, result.yCenter);
    }
}


void printResult(HUSKYLENSResult result){
    for(i=0;i<max_items_on_convoy;i++){
      Serial.print(String() + "[" + item_info[i].id +"," + item_info[i].x +"," + item_info[i].y +"," + item_info[i].elapsed_time +"," + item_info[i].counter + "] ");
    }
    for(i=0;i<max_color;i++){
      Serial.print(count[i]);
    }   
    Serial.print(" ");
    for(i=0;i<max_items_on_convoy;i++){
      Serial.print(timer_flag[i]);
    }   
    Serial.print(String() + " " +total_count);
    Serial.println(" ");


}