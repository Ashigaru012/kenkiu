/***************************************************************************************
子機xbeeのUARTからシリアル情報を送信する


                                                       Copyright (c) 2013 Wataru KUNINO
***************************************************************************************/

#include "../libs/xbee.c"
#include <string.h>
#include <pthread.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>


#define SLEEP_TIME 8
#define max 10
#define normal 10 //正常パケット数
#define attack 50 //攻撃パケット数

typedef struct stack{
    char **date;
    int top;
}stack;

//グローバル変数
stack buff;                          //buffer
byte dev[]={0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF}; // 宛先XBeeアドレス(ブロードキャスト)


void arrayDelete(char **array, int n, int k,char *s)
{
    int i;
	
    for (i = k+1; i < n; i++) {
        array[i-1] = array[i];
    }

    strcpy( array[n-1], s );
}
int push(stack *p,char *s){
    int cnt = 0;
    char *s1 = "normal";

    srand(time(NULL));
    if(p->top >= max-1){

        /*RDOFの実装*/
        p->top=max-1;
        int k = abs(rand() % max-2); //ランダムで配列中の要素を選ぶために乱数生成
        for( int y = k+1; y < max; ++y ){
            printf("y:%d,k:%d\n",y,k);
            if(y == k+1){
                if(strcmp(p->date[y-1],s1)==0){
                    cnt += 1;
                }
            }
            memmove( p->date[y - 1], p->date[y], sizeof(char*) * max) - 1;
        }
        strcpy( p->date[p->top], s );

        
    }else{
    strcpy( p->date[p->top++], s );
    }

    return cnt;
}

void show(stack *p){
    printf("stack:");
    for(int i=0;i<p->top;i++){
        printf("%s",p->date[i]);
    }
    
    printf("\n\n");
}

void* ping(){
    while (1)
    {
            if(xbee_ping(dev) == 0x00){
        /*スタックの初期化*/
        //buff.max = MAX_DATA; //スタックのサイズ
        buff.top = 0; //次に値を格納する場所の番号
        buff.date = (char**)malloc(sizeof(char*) * max);
        for(int i = 0; i < max; i++){
            buff.date[i] = (char*)malloc(sizeof(char) * 32);
        }
              
        }
    }
}

void *input(){
    
}
int main(int argc,char **argv){
    pthread_t thread1;
    char s[32];                                 // 文字入力用
    byte com=0;                                 // シリアルCOMポート番号
    int packet_cnt = 0;                         //送信パケットのカウンタ
    int normal_packet_cnt = 0;                  //破棄された正常パケットのカウンタ
    int normal_success = 0;

    char *s1 = "normal";
    char *s2 = "attack";

    char packet_kind[3][10] = {"normal","attack","attack"};

    for(int i =0; i < 2; i++){
        printf("kind:%s\n",packet_kind[i]);
    }

    int attack_cnt = 0;
    int normal_cnt = 0;

    char x,input[101],*p;



    /*スタックの初期化*/
    //buff.max = MAX_DATA; //スタックのサイズ
    //buff.ptr = 0; //次に値を格納する場所の番号
    //buff.stk = malloc(buff.max * sizeof(*buff.stk));

    /*スタックの初期化*/
    //buff.max = MAX_DATA; //スタックのサイズ
    buff.top = 0; //次に値を格納する場所の番号
    buff.date = (char**)malloc(sizeof(char*) * max);
    for(int i = 0; i < max; i++){
        buff.date[i] = (char*)malloc(sizeof(char) * 32);
    }


    if(argc==2) com=(byte)atoi(argv[1]);        // 引数があれば変数comに代入する
    xbee_init( com );                           // XBee用COMポートの初期化
    printf("Waiting for XBee Commissoning\n");  // 待ち受け中の表示
    if(xbee_atnj(30) != 0){                     // デバイスの参加受け入れを開始
        printf("Found a Device\n");             // XBee子機デバイスの発見表示
        xbee_from( dev );                       // 見つけた子機のアドレスを変数devへ
        xbee_ratnj(dev,0);                      // 子機に対して孫機の受け入れ制限を設定
    }else{                                      // 子機が見つからなかった場合
        printf("no Devices\n");                 // 見つからなかったことを表示
        exit(-1);                               // 異常終了
    }

    //ping関数を非同期処理
    
    
    //pthread_join(thread1, NULL);

    xbee_end_device(dev,SLEEP_TIME,15,0);                       //ZEDのスリープ設定
    //srand(time(NULL));
    sleep(30);
    while(1){
        /* データ送信*/
        
        //printf("TX-> ");                        // 文字入力欄の表示
        //gets( s );                          // 入力文字を変数sに代入
        if(normal_cnt == normal && attack_cnt == attack){
            break;
        }

        srand((unsigned int) time(NULL)); 
        int k = abs(rand()%3);
        printf("乱数：%d\n",k);
        strcpy(s,packet_kind[k]);
        
        if(normal_cnt == normal){
            strcpy(s,s2);
        }
        if(attack_cnt == attack){
            strcpy(s,s1);
        }

        if(xbee_ping(dev) == 0xFF){
            normal_packet_cnt += push(&buff,s);
        }
        if(xbee_ping(dev) == 0x00){
            for(int i = 0; i < max; i++){
                if(strcmp(buff.date[i],s1)==0){
                    normal_success += 1;
                }
            }

            /*スタックの初期化*/
            //buff.max = MAX_DATA; //スタックのサイズ
            buff.top = 0; //次に値を格納する場所の番号
            buff.date = (char**)malloc(sizeof(char*) * max);
            for(int i = 0; i < max; i++){
                buff.date[i] = (char*)malloc(sizeof(char) * 32);
            }
              
        }
        show(&buff);
        xbee_uart( dev , s );                   // 変数sの文字を送信
        //packet_cnt+=1;
        if(strcmp(s,s1)==0){
            normal_cnt += 1;
        }
        if(strcmp(s,s2)==0){
            attack_cnt += 1;
        }

        printf("正常パケットの送信回数は%dです\n正常パケットの到達した回数は%dです。\n正常パケットの破棄率は%lfパーセントです\n",normal_cnt,normal_success,((float)normal_success / (float)normal_cnt) * 100 );

        //正常パケットの破棄率は(normal_packet_cnt / normal_cnt)で、正常パケットの到達率はそれに1引けばでる
        
    }
}
