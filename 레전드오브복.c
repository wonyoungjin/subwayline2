#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>

#define dead 1
#define allive 0

//던전 포탈 좌표
typedef struct{
    int y;
    int x;
} DeonjeonDoorXY;

//유저팀 수호대 (타수호대 )의 위치 좌표
typedef struct 
{
    int y;
    int x;
} Player_GuardianXY;

//타수호대 구조체 모음
typedef struct 
{
    Player_GuardianXY guardianXY1;
    Player_GuardianXY guardianXY2;
    Player_GuardianXY guardianXY3;
} StructOfGuardians;

//몬스터 좌표 구조체
typedef struct
{
    int y;
    int x;
} MonsterXY;

typedef struct
{
    char name[20];          //이름
    char class_name[20];    //클래스 이름
    int class;              //클래스 종류 1.전사 2.백마법사 3.흑마법사 4.적마법사 5.궁수 6.검사
    int lv;                 //레벨
    int max_hp;             //최대 체력
    int max_mp;             //최대 마나
    int hp;                 //현재 체력
    int mp;                 //현재 마나
    int atk;                //공격력
    int total_atk;          //최종 공격력(공격력+무기 공격력)
    int def;                //방어력
    int total_def;          //최종 방어력(방어력+무기 방어력)
    int exp;                //현재 경험치
    char *head_eqm;         // 머리
    char *body_eqm;         // 상의
    char *pants_eqm;        // 하의
    char *cape_eqm;         // 망토
    char *glove_eqm;        // 장갑
    char *left_eqm;         // 방패
    char *right_eqm;        // 무기
    int state;              //상태(전투 불능)
    int area;               //현재 있는 구역 1.불 2.눈 3.숲 4.물
    int floor;              //던전 층수
} Player;

typedef struct
{
    char name[20];      //몬스터 이름
    char skill[20];     //몬스터 스킬 이름
    int hp;             //체력
    int atk;            //공격력
    int def;            //방어력
    int exp;            //잡았을때 주는 경험치
    int state;          //상태(전투 불능)
    int type;           //몬스터 타입(1.불 2.숲 3.눈 4.물 5.수호대)
    int cd;             //쿨다운 남은 턴
    int class;          //수호대 클래스
    int floor;          //던전 층수
} Monster;

typedef struct
{
    char name[50];  // 보스 이름
    char skill[50]; // 보스 스킬 이름
    int hp;         // 체력
    int atk;        // 공격력
    int def;        // 방어력
    int exp;        // 경험치
    int state;      // 상태(전투 불능) hp가 0이되면 state의 값이0으로돼서 전투 불능상태
    int type;       // 마법 물리 0이면 물리 1이면 마법
} Boss;

typedef struct // 소비 아이템 구조체
{
    char name[30]; // 이름
    int count;  // 수량 (수량 변수로 동기화 필요. 우선 0으로 기입. 추후 전투로직 완성시 변수 받아서 삽입.)
    int hp;     // HP
    int mp;     // MP
} Consumption;

typedef struct
{
    char *head_name;  // 머리
    char *body_name;  // 상의
    char *pants_name; // 하의
    char *cape_name;  // 망토
    char *glove_name; // 장갑
    char *left_name;  // 방패
    char *right_name; // 무기
    int def;          // 방어력
    int atk;          //  공격력
    int count;        // 수량    전투가 끝나면 아이템 수량을 받아야함.
} Equipment;

typedef struct // 스킬 구성
{
    char *skillname; // 스킬 이름
    int skillatk;    // 스킬 공격력
    int skilldef;    // 스킬 방어력
    int skillheal;   // 스킬 회복력
    int skillmp;     // 스킬 사용시 마나 사용량
    int s_unlock;    // 스킬 레벨 제한
} Skillstat;

typedef struct // 스킬 구성
{
    char *skillname; // 스킬 이름
    int skillatk;    // 스킬 공격력  mob[turn_atk].atk
} Mobskillstat;

void Opening();  //오프닝 출력 함수
Consumption itemList(Player *chr, int turn_atk, int n); // 소비 아이템 리스트 정의
void clearScreen(); //화면 지우는 함수
void Item_Drop(Consumption *item, Equipment *armor);  //아이템 드롭함수
int Battle(Player * chr, Consumption *ptr, Equipment* armor, int (*battlemap)[24], int m_type);  //전투함수
void Equipment_List(Equipment *armor); //장비리스트 함수
void Init_Guardian(Monster* guard, Player* chr); //수호대, 캐릭터
void lmt_Equipment(int slt_player, int slt_equiment, Player *chr, Equipment *armor); // 장비 착용에 직업별 제한을 거는 함수
void Equipment_menu(Player *chr, Equipment *armor); // 장비창 메뉴 구성
void Map(int (*map)[24], Player *chr, Consumption *ptr);  //일반 맵화면 출력함수
int Move(int (*map)[24], char* hotkey, Equipment* equip, Player *chr, int** arr, Consumption* ptr, int (*battlemap)[24]);  //플레이어 이동 함수
void Battlemap(int (*battlemap)[24], Player *chr, Consumption *ptr, Monster *mob, int m_max);  //전투화면 맵 출력함수
int** MonsterAppearanceProbability(); //몬스터 확률 좌표 배열 생성 함수
int** RandNumIntoMonsterAppearanceProbability(int** arr); //몬스터 확률 좌표 난수 담기 함수
void memoryFreeForMonsters(int** monsterProbArr);  //몬스터 확률 좌표 2D배열 메모리 동적할당 해제 함수
void memoryFreeForGuardians(int** arr); //수호대 확률 좌표 2D배열 메모리 동적할당 해제 함수
void Item_Drop(Consumption *item, Equipment *armor); // 아이템 드롭 시스템
int RandomUpdateGuardiansXY(); //타수호대 좌표를 랜덤으로 갱신해주는 함수
int RandomUpdateGuardiansXYInDeonJeonMap();  //던전맵에서 타수호대 좌표를 랜덤으로 갱신해주는 함수
int RandomCreateDoorXY();  //던전 포탈을 한정된 영역 내에서 랜덤한 좌표값 생성하는 함수
int** RandomCreatePlayer_GuardianXY();  //유저,타수호대 좌표 랜덤 생성 함수
void changeHotkeys(char* hotkeyArr, int* hotKeyCursor);  //단축키 설정하는 함수
void dun1(int r, int **ptr, Player* chr, Consumption*item, Equipment*armor, int (*battlemap)[24]); // 15 ~ 18랜덤 배열 던전
void dunp(Player* chr, Consumption*item, Equipment*armor, int (*battlemap)[24]);
void Init_Boss(Monster* boss, Player*chr);
void Renewal(Player*chr);


Mobskillstat Nmob(Monster *mob, int turn_atk, int n) // 일반 몬스터 스킬 정의
{
    srand((unsigned)time(NULL));
    double normal1 = rand() % 31 + 20; // 20 ~ 50 값
    double normal2 = rand() % 41 + 20; // 20 ~ 60 값
    double normal3 = rand() % 51 + 30; // 30 ~ 80 값
    double normal4 = rand() % 61 + 30; // 30 ~ 90 값

    Mobskillstat normal[4] = {{"불뿜기", mob[turn_atk].atk * (normal1 / 100)},
                              {"얼음고드름", mob[turn_atk].atk * (normal3 / 100)},
                              {"나무던지기", mob[turn_atk].atk * (normal4 / 100)},
                              {"물대포", mob[turn_atk].atk * (normal2 / 100)}};

    return normal[n];
}

Skillstat SwordManSkill(Player *chr, int turn_atk) // 검사 스킬 정의
{
    srand((unsigned)time(NULL));
    int luck = rand() % 49 + 2; // 1~50 값

    Skillstat swordman = {"럭키스트라이크", chr[turn_atk].total_atk * luck, 0, chr[turn_atk].max_mp / 2, 10}; // 단일
    return swordman;
}

Skillstat WhiteWizardSkill(Player *chr, int caster, int target, int n) // 백법사 스킬 정의
{
    srand((unsigned)time(NULL));
    double heal = rand() % 41 + 30;    // 30~70 값
    double gheal = rand() % 41 + 60;   // 60~100 값
    double healall = rand() % 41 + 40; // 40~80 값
    double atkup = rand() % 21 + 50;   // 50~70 값

    Skillstat whitewizard[6] = {{"힐", 0, 0, chr[target].max_hp * (heal / 100), chr[caster].max_mp / 10, 1},           // 단일
                                {"그레이터힐", 0, 0, chr[target].max_hp * (gheal / 100), chr[caster].max_mp / 10, 15}, // 단일
                                {"힐올", 0, 0, chr[target].max_hp * (healall / 100), chr[caster].max_mp / 10, 30},     // 전체
                                {"공격력 업!", chr[target].total_atk/2 +10, 0, 0, chr[caster].max_mp / 10, 10},        // 전체
                                {"방어력 업!", 0, chr[target].total_def / 2, 0, chr[caster].max_mp / 10, 15}};         // 전체

    return whitewizard[n];
}

Skillstat blackWizard(Player *chr, int turn_atk, int n) // 흑법사 스킬 정의
{
    srand((unsigned)time(NULL));
    double black = rand() % 21 + 50; // 50~70 값

    Skillstat blackwizard[4] = {{"파이어볼", (chr[turn_atk].atk) + (chr[turn_atk].total_atk * (30 / 100)), 0, 0, chr[turn_atk].max_mp / 10, 1},         // 단일
                                {"파이어월", (chr[turn_atk].atk) + (chr[turn_atk].total_atk * (black / 100)), 0, 0, chr[turn_atk].max_mp / 10, 15},     // 전체
                                {"썬더브레이커", (chr[turn_atk].atk) + (chr[turn_atk].total_atk * (black / 100)), 0, 0, chr[turn_atk].max_mp / 10, 20}, // 전체
                                {"블리자드", (chr[turn_atk].atk) + (chr[turn_atk].total_atk * (black / 100)), 0, 0, chr[turn_atk].max_mp / 10, 25}};    // 전체

    return blackwizard[n];
}

Skillstat RedWizardSkill(Player *chr, int caster, int target, int n) // 적법사 스킬 정의
{
    srand((unsigned)time(NULL));
    double red = rand() % 81 + 20; // 20~80 값

    Skillstat redwizard[4] = {{"힐", 0, 0, chr[target].max_hp * (red / 100), chr[caster].max_mp / 10, 1},            // 단일
                              {"파이어볼", chr[caster].total_atk * (red / 100), 0, 0, chr[caster].max_mp / 10, 1},   // 단일
                              {"힐올", 0, 0, chr[target].max_hp * (red / 100), chr[caster].max_mp / 10, 20},         // 전체
                              {"파이어월", chr[caster].total_atk * (red / 100), 0, 0, chr[caster].max_mp / 10, 20}}; // 전체

    return redwizard[n];
}

Skillstat ArcherSkill(Player *chr, int turn_atk, int n) // 궁수 스킬 정의
{
    srand((unsigned)time(NULL));
    double archeratk1 = rand() % 31 + 20; // 20~50 값
    double archeratk2 = rand() % 21 + 40; // 40~60 값
    double archeratk3 = rand() % 21 + 50; // 50~70 값

    Skillstat archer[3] = {{"집중타", (chr[turn_atk].atk) + (chr[turn_atk].total_atk * (archeratk1 / 100)), 0, 0, chr[turn_atk].max_mp / 10, 10},    // 단일
                           {"듀얼샷", (chr[turn_atk].atk) + (chr[turn_atk].total_atk * (archeratk2 / 100)), 0, 0, chr[turn_atk].max_mp / 10, 15},    // 단일
                           {"마스터샷", (chr[turn_atk].atk) + (chr[turn_atk].total_atk * (archeratk3 / 100)), 0, 0, chr[turn_atk].max_mp / 10, 20}}; // 단일

    return archer[n];
}

int getch()     //엔터를 누르지 않아도 1문자씩 키입력을 받기 위한 함수(wasd)
{
    int c;
    struct termios oldattr, newattr;

    tcgetattr(STDIN_FILENO, &oldattr);           // 현재 터미널 설정 읽음
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);         // CANONICAL과 ECHO 끔
    newattr.c_cc[VMIN] = 1;                      // 최소 입력 문자 수를 1로 설정
    newattr.c_cc[VTIME] = 0;                     // 최소 읽기 대기 시간을 0으로 설정
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);  // 터미널에 설정 입력
    c = getchar();                               // 키보드 입력 읽음
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);  // 원래의 설정으로 복구
    return c;
}

int PutKey(int max)
{
    char put;
    while(1)
    {
        put=getch();
        put=atoi(&put);     //입력받은 키(문자)를 정수로 변환
        if(put<1||put>max)
        continue;
        else
        break;
    }
    
    return put;
}

void Init_Player(Player* chr)   //스테이터스 초기화
{
    const char* name[5]={"이시영", "이영광", "원영진", "박영진", "박영선"};
    const char* class_name[6]={"전사", "백마법사", "흑마법사", "적마법사", "궁수", "검사"};
    int check[5]={0, };
    int class, i, j, overlap=0;

    for(i=0; i<5; i++)
    {
        class=rand()%6+1;
        for(j=0; j<5; j++)      //중복처리
        {
            if(check[j]==class)
            {
                i--;
                overlap=1;
                break;
            }
        }
        if(overlap==1)
        {
            overlap=0;
            continue;
        }

        strcpy(chr[i].name, name[i]);                           //캐릭터 이름
        strcpy(chr[i].class_name, class_name[class-1]);         //직업 이름
        chr[i].class=class;                                     //직업 종류
        switch(class)
        {
            case 1:     //전사
            chr[i].lv=1;
            chr[i].max_hp=300;
            chr[i].max_mp=0;
            chr[i].hp=chr[i].max_hp;
            chr[i].mp=chr[i].max_mp;
            chr[i].atk=150;
            chr[i].total_atk=chr[i].atk;
            chr[i].def=200;
            chr[i].total_def=chr[i].def;
            chr[i].exp=0;
            chr[i].state=allive;
            chr[i].area=1;
            break;

            case 2:     //백마법사
            chr[i].lv=1;
            chr[i].max_hp=200;
            chr[i].max_mp=150;
            chr[i].hp=chr[i].max_hp;
            chr[i].mp=chr[i].max_mp;
            chr[i].atk=100;
            chr[i].total_atk=chr[i].atk;
            chr[i].def=100;
            chr[i].total_def=chr[i].def;
            chr[i].exp=0;
            chr[i].state=allive;
            chr[i].area=1;
            break;

            case 3:     //흑마법사
            chr[i].lv=1;
            chr[i].max_hp=200;
            chr[i].max_mp=150;
            chr[i].hp=chr[i].max_hp;
            chr[i].mp=chr[i].max_mp;
            chr[i].atk=500;
            chr[i].total_atk=chr[i].atk;
            chr[i].def=100;
            chr[i].total_def=chr[i].def;
            chr[i].exp=0;
            chr[i].state=allive;
            chr[i].area=1;
            break;

            case 4:     //적마법사
            chr[i].lv=1;
            chr[i].max_hp=150;
            chr[i].max_mp=100;
            chr[i].hp=chr[i].max_hp;
            chr[i].mp=chr[i].max_mp;
            chr[i].atk=380;
            chr[i].total_atk=chr[i].atk;
            chr[i].def=80;
            chr[i].total_def=chr[i].def;
            chr[i].exp=0;
            chr[i].state=allive;
            chr[i].area=1;
            break;

            case 5:     //궁수
            chr[i].lv=1;
            chr[i].max_hp=150;
            chr[i].max_mp=150;
            chr[i].hp=chr[i].max_hp;
            chr[i].mp=chr[i].max_mp;
            chr[i].atk=450;
            chr[i].total_atk=chr[i].atk;
            chr[i].def=120;
            chr[i].total_def=chr[i].def;
            chr[i].exp=0;
            chr[i].state=allive;
            chr[i].area=1;
            break;

            case 6:     //검사
            chr[i].lv=1;
            chr[i].max_hp=150;
            chr[i].max_mp=150;
            chr[i].hp=chr[i].max_hp;
            chr[i].mp=chr[i].max_mp;
            chr[i].atk=300;
            chr[i].total_atk=chr[i].atk;
            chr[i].def=150;
            chr[i].total_def=chr[i].def;
            chr[i].exp=0;
            chr[i].state=allive;
            chr[i].area=1;
            break;

            default:
            printf("INIT ERROR!!\n");
            break;
        }
        check[i]=class;
    }
}

void Init_Monster(Monster* mob, int area, int m_max, int chr_lv)        //능력치 초기화
{
    const char* name[4]={"불 슬라임", "눈 슬라임", "숲 슬라임", "물 슬라임"};
    const char* num[10]={"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    int i;
    int rand_hp;

    for(i=0; i<m_max; i++)
    {
        rand_hp=rand()%800+200;               //수치는 플레이어 레벨에 맞춰 능동적으로 변화할 예정
        strcpy(mob[i].name, name[area-1]);    //출현 몬스터 이름
        strcat(mob[i].name, num[i]);          //몬스터 번호
        mob[i].hp=rand_hp+50*chr_lv;          //200~1000랜덤
        mob[i].atk=80+3*chr_lv;               //세부 수치 수정 예정
        mob[i].def=40+3*chr_lv;               //세부 수치 수정 예정
        mob[i].exp=5+rand()%5;                //세부 수치 수정 예정
        mob[i].state=allive;                  //사망 상태
        mob[i].type=area;
        mob[i].cd=0;
        mob[i].class=0;
    }
}

void Init_Guardian(Monster* guard, Player* chr) //수호대, 캐릭터
{
    int lv, class, overlap=0;
    int i,j;
    int check[6]={0, };
    const char* class_name[6]={"전사", "백마법사", "흑마법사", "적마법사", "궁수", "검사"};
    const char* class_skill[6]={"", "힐", "파이어월", "파이어볼", "집중타", "럭키스트라이크"};

    for(i=0; i<5; i++)
    {
        class=rand()%6+1;
        for(j=0; j<5; j++)      //중복처리
        {
            if(check[j]==class)
            overlap=1;
        }
        if(overlap==1)
        {
            i--;
            overlap=0;
            continue;
        }

        strcpy(guard[i].name, class_name[class-1]);
        strcpy(guard[i].skill, class_skill[class-1]);
        lv=rand()%5+chr->lv;

        switch(class)
        {
            case 1: //전사
            guard[i].hp = 250 + 50*lv;
            guard[i].atk = 100 + 20*lv;
            guard[i].def = 160 + 40*lv;
            guard[i].exp = 30;
            guard[i].state = allive;
            guard[i].class = class;
            guard[i].cd=0;
            guard[i].type=0;
            break;
            case 2: //백마법사
            guard[i].hp = 150 + 50*lv;
            guard[i].atk = 100 + 20*lv;
            guard[i].def = 90 + 40*lv;
            guard[i].exp = 30;
            guard[i].state = allive;
            guard[i].class = class;
            guard[i].cd=0;
            guard[i].type=0;
            break;
            case 3: //흑마법사
            guard[i].hp = 150 + 50*lv;
            guard[i].atk = 250 + 20*lv;
            guard[i].def = 90 + 40*lv;
            guard[i].exp = 30;
            guard[i].state = allive;
            guard[i].class = class;
            guard[i].cd=0;
            guard[i].type=0;
            break;
            case 4: //적마법사
            guard[i].hp = 100 + 50*lv;
            guard[i].atk = 200 + 20*lv;
            guard[i].def = 80 + 40*lv;
            guard[i].exp = 30;
            guard[i].state = allive;
            guard[i].class = class;
            guard[i].cd=0;
            guard[i].type=0;
            break;
            case 5: //궁수
            guard[i].hp = 100 + 50*lv;
            guard[i].atk = 230 + 20*lv;
            guard[i].def = 90 + 40*lv;
            guard[i].exp = 30;
            guard[i].state = allive;
            guard[i].class = class;
            guard[i].cd=0;
            guard[i].type=0;
            break;
            case 6: //검사
            guard[i].hp = 100 + 50*lv;
            guard[i].atk = 220 + 20*lv;
            guard[i].def = 100 + 40*lv;
            guard[i].exp = 30;
            guard[i].state = allive;
            guard[i].class = class;
            guard[i].cd=0;
            guard[i].type=0;
            break;
        }
        
    }
}

void Init_Boss(Monster* boss, Player*chr)
{
    const char* name[5]={"리북녘", "또리우홍걸", "초이하아무운", "코로나일구공주", "로드오브보기"};
    const char* skill_name[5]={"파이썬 어택", "프로젝트 어택", "스택메모리 어택", "자가격리 어택", "하이퍼스트라이크"};
    int floor=chr->floor-1;

    strcpy(boss[floor].name, name[floor]);
    strcpy(boss[floor].skill, skill_name[floor]);
    
    switch(floor)
    {
        case 0: //1층
        boss[floor].hp=rand()%10000+25000;
        break;

        case 1: //2층
        boss[floor].hp=rand()%10000+45000;
        break;

        case 2: //3층
        boss[floor].hp=rand()%10000+65000;
        break;

        case 3: //4층
        boss[floor].hp=rand()%10000+75000;
        break;
        
        case 4: //5층
        boss[floor].hp=rand()%914999+85000;
        break;
    }
    boss[floor].atk=20*chr->lv;
    boss[floor].def=12*chr->lv;
    boss[floor].exp=rand()%50+50;
    boss[floor].type=0;
    boss[floor].cd=0;
    boss[floor].class=7;
    boss[floor].floor=chr->floor;
}

void Renewal(Player*chr)
{
    int i;
    for(i=0; i<5; i++)
    {
        switch(chr[i].class)
        {
            case 1: //전사
            chr[i].max_hp=300 + 200*chr->lv;
            chr[i].atk=150 + 15*chr->lv;
            chr[i].def=200 + 25*chr->lv;
            break;

            case 2: //백마법사
            chr[i].max_hp=200 + 80*chr->lv;
            chr[i].max_mp=150 + 100*chr->lv;
            chr[i].atk=100 + 10*chr->lv;
            chr[i].def=100 + 8*chr->lv;
            break;

            case 3: //흑마법사
            chr[i].max_hp=200 + 80*chr->lv;
            chr[i].max_mp=150 + 120*chr->lv;
            chr[i].atk=100 + 35*chr->lv;
            chr[i].def=100 + 5*chr->lv;
            break;

            case 4: //적마법사
            chr[i].max_hp=150 + 65*chr->lv;
            chr[i].max_mp=100 + 95*chr->lv;
            chr[i].atk=100 + 20*chr->lv;
            chr[i].def=100 + 7*chr->lv;
            break;

            case 5: //궁수
            chr[i].max_hp=150 + 50*chr->lv;
            chr[i].max_mp=150 + 70*chr->lv;
            chr[i].atk=450 + 15*chr->lv;
            chr[i].def=120 + 5*chr->lv;
            break;

            case 6: //검사
            chr[i].max_hp=150 + 50*chr->lv;
            chr[i].max_mp=150 + 80*chr->lv;
            chr[i].atk=300 + 30*chr->lv;
            chr[i].def=150 + 3*chr->lv;
            break;
        }
    }
}
//전투 관련  함수
int Battle(Player * chr, Consumption *ptr, Equipment* armor, int (*battlemap)[24], int m_type)    // 몬스터 타입 1.일반몹 2.수호대 3.보스몹
{
    // 전투 시스템 관련 변수
    int turn=1, end=1, death=0, exp=0, i, result=0, win=0;
    // 턴수, 전투 종료, 죽은 적 수, 잡았을 때 경험치, , 전투 결과 1.도망 2.승리 3.패배

    // 플레이어 관련 변수
    int run, p_atk=0, menu, s_item, s_target, s_skill, s_unlock=1, sword=0;
    //도망확률, 공격순서, 행동 선택, 아이템 선택, 공격할 적 선택, 스킬 선택, 해금된 스킬
    Skillstat skill[10];        // 스킬 수치 받아올 구조체 배열
    Skillstat* pskill=skill;    // 구조체 배열을 가리키는 구조체 포인터변수
    Mobskillstat mskill[10];
    Mobskillstat* pmskill=mskill;

    int buff_atk[5]={0, };      // 공격버프 전 공격력 저장
    int buff_def[5]={0, };      // 방어버프 전 방어력 저장

    int white_dur1=0;           // 공격버프 지속시간
    int white_dur2=0;           // 방어버프 지속시간
    int sword_buff=0;           // 검사 버프 전 공격력 저장
    int sword_dur=0;            // 검사 버프 지속시간

    int white_cd[5]={0, };      // 백마법사 스킬 쿨타임
    int black_cd[5]={0, };      // 흑마법사 스킬 쿨타임
    int red_cd[5]={0, };        // 적마법사 스킬 쿨타임
    int archer_cd[5]={0, };     // 궁수 스킬 쿨타임
    int sword_cd=0;             // 검사 스킬 쿨타임

    // 몬스터 관련 변수
    int m_atk=0, m_skill, m_target, m_max;
    // 공격순서, 스킬 발동 확률, 타겟, 생성된 몹 수

    // 수호대 관련 변수
    int guard_buff=0, guard_class=0;
    // 수호대 검사 버프 전 공격력 저장, 수호대 클래스(검사)저장
    switch(m_type)
    {
        case 1:
        m_max=rand()%10+1;  //일반몹은 1~10마리
        break;
        case 2:
        m_max=5;            //수호대 하나당 5명 고정
        break;
        case 3:
        m_max=rand()%6+1;   //0은 보스자리+ 0~6마리 일반몹
        break;
    }
    Monster mob[m_max]; //랜덤 수 몹 생성   보스전일때 0번은 보스자리
    Monster* pmob=mob;

    switch(m_type)
    {
        case 1:
        Init_Monster(pmob, chr->area, m_max, chr->lv); //몹 초기화
        printf("야생의 몬스터가 등장했다!!\n");
        break;
        case 2:
        Init_Guardian(mob, chr);
        printf("적(?)과 조우했다!!\n");
        break;
        case 3:
        Init_Monster(pmob, chr->area, m_max, chr->lv);
        Init_Boss(mob, chr);    //0번 덮어쓰기
        printf("보스와 마주쳤다!!\n");
        break;
    }

    for(i=0; i<5; i++)
    {
        if(chr[i].class==6)
        {
            sword_buff=chr[i].total_atk;
            buff_def[i] = chr[i].total_def;
            sword=i;
        }
        else
        {
            buff_atk[i] = chr[i].total_atk;
            buff_def[i] = chr[i].total_def;
        }
    }
    Battlemap(battlemap, chr, ptr, mob, m_max);
    usleep(500000);
    while(end)      //0이면 종료
    {
        if(chr[p_atk].state==dead)
        {
            p_atk++;
            continue;
        }
        printf("%s의 턴 !\n", chr[p_atk].name);
        usleep(500000);
        Battlemap(battlemap, chr, ptr, mob, m_max);
        printf("현재 턴: %d\n", turn);
        printf("1.일반 공격 2.스킬 사용 3.아이템 사용 4.도망가기\n");
        menu=PutKey(4);
        switch(menu)
        {
            case 1:     //  일반공격
            printf("누구를 공격?\n");
            s_target=PutKey(m_max);
            s_target-=1;
            if(mob[s_target].state==dead)   //사망한 몹 공격 못하게 체크
            {
                printf("이미 죽은 대상입니다.\n");
                usleep(500000);
                continue;
            }

            if(chr[p_atk].total_atk - mob[s_target].def/2<=0)
            printf("공격이 통하지 않았다.\n");
            else
            {
                printf("%s의 공격!\n%s에게 %d의 대미지를 입혔다!\n", chr[p_atk].name, mob[s_target].name, chr[p_atk].total_atk-mob[s_target].def/2);
                mob[s_target].hp -= chr[p_atk].total_atk - mob[s_target].def/2; //대미지 적용
            }
            break;
            case 2:     //  스킬사용
            switch(chr[p_atk].class)
            {
                case 1: //전사
                printf("사용할 수 있는 스킬이 없습니다.\n");
                usleep(500000);
                continue;
                case 2: //백마법사
                if(chr[p_atk].lv<10)       //레벨에 따라 사용 가능한 스킬 해금
                {
                    printf("1.힐\n");
                    s_unlock=1;
                }
                else if(chr[p_atk].lv>=10&&chr[p_atk].lv<15)
                {
                    printf("1.힐 2.공격력 업!\n");
                    s_unlock=2;
                }
                else if(chr[p_atk].lv>=15&&chr[p_atk].lv<30)
                {
                    printf("1.힐 2.공격력 업! 3.그레이터 힐 4.방어력 업!\n");
                    s_unlock=4;
                }
                else
                {
                    printf("1.힐 2.공격력 업! 3.그레이터 힐 4.방어력 업! 5.힐 올\n");
                    s_unlock=5;
                }

                s_skill=PutKey(s_unlock);
                switch(s_skill)
                {
                    case 1: //힐
                    *pskill=WhiteWizardSkill(chr, p_atk, 0, 0);
                    if(chr[p_atk].mp<pskill->skillmp)
                    {
                        printf("MP가 부족합니다.\n");
                        usleep(500000);
                        continue;
                    }
                    printf("누구를? \n");
                    s_target=PutKey(5);
                    s_target-=1;
                    *pskill=WhiteWizardSkill(chr, p_atk, s_target, 0);
                    if(chr[s_target].state==dead)
                    {
                        printf("죽은 대상입니다.\n");
                        usleep(500000);
                        continue;
                    }
                    else if(chr[s_target].hp>=chr[s_target].max_hp)
                    {
                        printf("이미 최대 체력입니다.\n");
                        usleep(500000);
                        continue;
                    }
                    chr[s_target].hp+=pskill->skillheal;
                    chr[p_atk].mp-=pskill->skillmp;
                    if(chr[s_target].hp>chr[s_target].max_hp)
                    {
                        printf("%s의 HP를 %d 회복했다.\n", chr[s_target].name, chr[s_target].hp-chr[s_target].max_hp);
                        chr[s_target].hp=chr[s_target].max_hp;
                    }
                    else
                    printf("%s의 HP를 %d 회복했다.\n", chr[s_target].name, pskill->skillheal);
                    white_cd[0]=2;
                    break;
                    case 2: //공격력업
                    *pskill=WhiteWizardSkill(chr, p_atk, 0, 3);
                    if(chr[p_atk].mp < pskill->skillmp)
                    {
                        printf("MP가 부족합니다.\n");
                        usleep(500000);
                        continue;
                    }
                    else if(white_dur1>0)
                    {
                        printf("이미 적용된 스킬입니다.\n");
                        usleep(500000);
                        continue;
                    }
                    else if(white_cd[1]>0)
                    {
                        printf("쿨타임 중 입니다.\n남은 쿨타임: %d턴\n", white_cd[1]);
                        usleep(500000);
                        continue;
                    }
                    chr[p_atk].mp-=pskill->skillmp;
                    for(i=0; i<5; i++)
                    {
                        *pskill=WhiteWizardSkill(chr, p_atk, i, 3);
                        if(chr[i].state==dead)
                        continue;
                        else
                        {
                            buff_atk[i] = chr[i].total_atk;
                            chr[i].total_atk += pskill->skillatk;
                        }    
                    }
                    printf("수호대의 공격력이 상승했다!\n");
                    usleep(500000);
                    white_dur1=2;
                    white_cd[1]=2;
                    break;
                    case 3: // 그레이터 힐
                    *pskill=WhiteWizardSkill(chr, p_atk, 0, 1);
                    if(chr[p_atk].mp<pskill->skillmp)
                    {
                        printf("MP가 부족합니다.\n");
                        usleep(500000);
                        continue;
                    }
                    printf("누구를? \n");
                    s_target=PutKey(5);
                    s_target-=1;
                    if(chr[s_target].state==dead)
                    {
                        printf("죽은 대상입니다.\n");
                        usleep(500000);
                        continue;
                    }
                    else if(chr[s_target].hp>=chr[s_target].max_hp)
                    {
                        printf("이미 최대 체력입니다.\n");
                        usleep(500000);
                        continue;
                    }
                    *pskill=WhiteWizardSkill(chr, p_atk, s_target, 1);
                    chr[s_target].hp+=pskill->skillheal;
                    if(chr[s_target].hp>chr[s_target].max_hp)
                    {
                        printf("%s의 HP를 %d 회복했다.\n", chr[s_target].name, chr[s_target].hp-chr[s_target].max_hp);
                        chr[s_target].hp=chr[s_target].max_hp;
                    }
                    else
                    printf("%s의 HP를 %d 회복했다.\n", chr[s_target].name, pskill->skillheal);
                    chr[p_atk].mp-=pskill->skillmp;
                    white_cd[2]=2;
                    break;
                    case 4: // 방어력업
                    *pskill=WhiteWizardSkill(chr, p_atk, 0, 4);
                    if(chr[p_atk].mp < pskill->skillmp)
                    {
                        printf("MP가 부족합니다.\n");
                        usleep(500000);
                        continue;
                    }
                    else if(white_dur2>0)
                    {
                        printf("이미 적용된 스킬입니다.\n");
                        usleep(500000);
                        continue;
                    }
                    else if(white_cd[3]>0)
                    {
                        printf("쿨타임 중 입니다.\n남은 쿨타임: %d턴\n", white_cd[3]);
                        usleep(500000);
                        continue;
                    }
                    chr[p_atk].mp-=pskill->skillmp;
                    for(i=0; i<5; i++)
                    {
                        *pskill=WhiteWizardSkill(chr, p_atk, i, 4);
                        if(chr[i].state==dead)
                        continue;
                        else
                        {
                            buff_def[i] = chr[i].total_def;
                            chr[i].total_def += pskill->skilldef;
                        }
                    }
                    white_dur2=2;
                    white_cd[3]=2;
                    case 5: //힐 올
                    *pskill=WhiteWizardSkill(chr, p_atk, i, 2);
                    if(chr[p_atk].mp<pskill->skillmp)
                    {
                        printf("MP가 부족합니다.\n");
                        usleep(500000);
                        continue;
                    }
                    else if(white_cd[4]>0)
                    {
                        printf("쿨타임 중 입니다.\n남은 쿨타임: %d턴\n", white_cd[4]);
                        usleep(500000);
                        continue;
                    }

                    for(i=0; i<5; i++)
                    {
                        *pskill=WhiteWizardSkill(chr, p_atk, i, 2);
                        if(chr[i].state==dead)
                        continue;
                        else
                        {
                            chr[i].hp+=pskill->skillheal;
                            if(chr[i].hp>=chr[i].max_hp)
                            {
                                printf("%s의 HP를 %d 회복했다.\n", chr[i].name, chr[i].hp-chr[i].max_hp);
                                chr[i].hp=chr[i].max_hp;
                            }
                            else
                            printf("%s의 HP를 %d 회복했다.\n", chr[i].name, pskill->skillheal);
                        }
                    }
                    white_cd[4]=2;
                    break;
                }
                break;

                case 3: //흑마법사
                if(chr[p_atk].lv<15)       //레벨에 따라 사용 가능한 스킬 해금
                {
                    printf("1.파이어볼\n");
                    s_unlock=1;
                }
                else if(chr[p_atk].lv>=15&&chr[p_atk].lv<20)
                {
                    printf("1.파이어볼 2.파이어월\n");
                    s_unlock=2;
                }
                else if(chr[p_atk].lv>=20&&chr[p_atk].lv<25)
                {
                    printf("1.파이어볼 2.파이어월 3.썬더브레이커\n");
                    s_unlock=3;
                }
                else
                {
                    printf("1.파이어볼 2.파이어월 3.썬더브레이커 4.블리자드\n");
                    s_unlock=4;
                }
                s_skill=PutKey(s_unlock);
                *pskill=blackWizard(chr, p_atk, s_skill-1);
                if(chr[p_atk].mp<pskill->skillmp)
                {
                    printf("MP가 부족합니다.\n");
                    usleep(500000);
                    continue;
                }
                else if(black_cd[s_skill-1]>0)
                {
                    printf("쿨타임 중 입니다.\n남은 쿨타임: %d턴\n", black_cd[s_skill-1]);
                    usleep(500000);
                    continue;
                }
                switch(s_skill)
                {
                    case 1: //파이어볼
                    printf("누구를?\n");
                    s_target=PutKey(m_max);
                    s_target-=1;
                    if(mob[s_target].state==dead)
                    {
                        printf("이미 죽은 대상입니다\n");
                        usleep(500000);
                        continue;
                    }
                    chr[p_atk].mp-=pskill->skillmp;
                    mob[s_target].hp-=(chr[p_atk].total_atk+ pskill->skillatk)-mob[s_target].def/2;
                    printf("%s의 %s ! %s은(는) %d의 피해를 입었다.\n", chr[p_atk].name, pskill->skillname, mob[s_target].name, 
                    (chr[p_atk].total_atk+ pskill->skillatk)-mob[s_target].def/2);
                    black_cd[0]=2;
                    break;
                    case 2: //파이어월
                    chr[p_atk].mp-=pskill->skillmp;
                    for(i=0; i<m_max; i++)
                    {
                        if(mob[i].state==dead)
                        continue;
                        else
                        mob[i].hp-=(chr[p_atk].total_atk+pskill->skillatk)-mob[s_target].def/2;
                    }
                    printf("%s의 %s !\n", chr[p_atk].name, pskill->skillname);
                    black_cd[1]=2;
                    break;
                    case 3: //썬더브레이커
                    chr[p_atk].mp-=pskill->skillmp;
                    for(i=0; i<m_max; i++)
                    {
                        if(mob[i].state==dead)
                        continue;
                        else
                        mob[i].hp-=(chr[p_atk].total_atk+pskill->skillatk)-mob[s_target].def/2;
                    }
                    printf("%s의 %s !\n", chr[p_atk].name, pskill->skillname);
                    black_cd[2]=2;
                    break;
                    case 4: //블리자드
                    chr[p_atk].mp-=pskill->skillmp;
                    for(i=0; i<m_max; i++)
                    {
                        if(mob[i].state==dead)
                        continue;
                        else
                        mob[i].hp-=(chr[p_atk].total_atk+pskill->skillatk)-mob[s_target].def/2;
                    }
                    printf("%s의 %s !\n", chr[p_atk].name, pskill->skillname);
                    black_cd[3]=2;
                    break;
                }
                break;

                case 4: //적마법사
                if(chr[p_atk].lv<20)       //레벨에 따라 사용 가능한 스킬 해금
                {
                    printf("1.힐 2.파이어볼\n");
                    s_unlock=2;
                }
                else
                {
                    printf("1.힐 2.파이어볼 3.힐올 4.파이어월\n");
                    s_unlock=4;
                }
                s_skill=PutKey(s_unlock);
                *pskill=RedWizardSkill(chr, p_atk, 0, s_skill-1);
                if(chr[p_atk].mp<pskill->skillmp)
                {
                    printf("MP가 부족합니다.\n");
                    usleep(500000);
                    continue;
                }
                else if(red_cd[s_skill-1]>0)
                {
                    printf("쿨타임 중 입니다.\n남은 쿨타임: %d턴\n", red_cd[s_skill-1]);
                    usleep(500000);
                    continue;
                }
                switch(s_skill)
                {
                    case 1: //힐
                    printf("누구를?\n");
                    s_target=PutKey(5);
                    s_target-=1;
                    *pskill=RedWizardSkill(chr, p_atk, s_target, s_skill-1);
                    if(chr[s_target].state==dead)
                    {
                        printf("이미 죽은 대상입니다\n");
                        usleep(500000);
                        continue;
                    }
                    else if(chr[s_target].hp>=chr[s_target].max_hp)
                    {
                        printf("이미 최대 체력입니다.\n");
                        usleep(500000);
                        continue;
                    }
                    chr[p_atk].mp-=pskill->skillmp;
                    chr[s_target].hp+=pskill->skillheal;

                    if(chr[s_target].hp>chr[s_target].max_hp)
                    {
                        printf("%s의 HP를 %d 회복했다.\n", chr[s_target].name, chr[s_target].hp-chr[s_target].max_hp);
                        chr[s_target].hp=chr[s_target].max_hp;
                    }
                    else
                    printf("%s의 HP를 %d 회복했다.\n", chr[s_target].name, pskill->skillheal);
                    break;

                    case 2: //파이어볼
                    printf("누구를?\n");
                    s_target=PutKey(m_max);
                    s_target-=1;
                    if(mob[s_target].state==dead)
                    {
                        printf("이미 죽은 대상입니다\n");
                        usleep(500000);
                        continue;
                    }
                    chr[p_atk].mp-=pskill->skillmp;
                    mob[s_target].hp-=(chr[p_atk].total_atk+ pskill->skillatk)-mob[s_target].def/2;
                    printf("%s의 %s ! %s은(는) %d의 피해를 입었다.\n", chr[p_atk].name, pskill->skillname, mob[s_target].name, 
                    (chr[p_atk].total_atk+ pskill->skillatk)-mob[s_target].def/2);
                    break;

                    case 3: //힐올
                    for(i=0; i<5; i++)
                    {
                        *pskill=RedWizardSkill(chr, p_atk, i, s_skill-1);
                        if(chr[i].state==dead)
                        continue;
                        else
                        {
                            chr[i].hp+=pskill->skillheal;
                            if(chr[i].hp>=chr[i].max_hp)
                            {
                                printf("%s의 HP를 %d 회복했다.\n", chr[i].name, chr[i].hp-chr[i].max_hp);
                                chr[i].hp=chr[i].max_hp;
                            }
                            else
                            printf("%s의 HP를 %d 회복했다.\n", chr[i].name, pskill->skillheal);
                        }
                        usleep(500000);
                    }
                    break;

                    case 4: //파이어월
                    chr[p_atk].mp-=pskill->skillmp;
                    for(i=0; i<m_max; i++)
                    {
                        if(mob[i].state==dead)
                        continue;
                        else
                        mob[i].hp-=(chr[p_atk].total_atk+pskill->skillatk)-mob[s_target].def/2;
                    }
                    printf("%s의 %s !\n", chr[p_atk].name, pskill->skillname);
                    break;
                }
                red_cd[s_skill-1]=2;
                break;

                case 5: //궁수
                if(chr[p_atk].lv<10)       //레벨에 따라 사용 가능한 스킬 해금
                {
                    printf("사용할 수 있는 스킬이 없습니다.\n");
                    usleep(500000);
                    continue;
                }
                else if(chr[p_atk].lv>=10&&chr[p_atk].lv<15)
                {
                    printf("1.집중타\n");
                    s_unlock=1;
                }
                else if(chr[p_atk].lv>=15&&chr[p_atk].lv<20)
                {
                    printf("1.집중타 2.듀얼샷\n");
                    s_unlock=2;
                }
                else
                {
                    printf("1.집중타 2.듀얼샷 3.마스터샷\n");
                    s_unlock=3;
                }
                s_skill=PutKey(s_unlock);
                s_skill-=1;
                *pskill=ArcherSkill(chr, p_atk, s_skill);
                if(chr[p_atk].mp<pskill->skillmp)
                {
                    printf("MP가 부족합니다.\n");
                    usleep(500000);
                    continue;
                }
                else if(archer_cd[s_skill]>0)
                {
                    printf("쿨타임 중 입니다.\n남은 쿨타임: %d턴\n", archer_cd[s_skill]);
                    usleep(500000);
                    continue;
                }
                printf("누구를?\n");
                s_target=PutKey(m_max);
                s_target-=1;
                if(mob[s_target].state==dead)
                {
                    printf("이미 죽은 대상입니다.\n");
                    usleep(500000);
                    continue;
                }
                chr[p_atk].mp-=pskill->skillmp;
                mob[s_target].hp-=(chr[p_atk].total_atk+ pskill->skillatk)-mob[s_target].def/2;
                printf("%s의 %s ! %s은(는) %d의 피해를 입었다.\n", chr[p_atk].name, pskill->skillname, mob[s_target].name, 
                (chr[p_atk].total_atk + pskill->skillatk)-mob[s_target].def/2);
                break;

                case 6: //검사
                if(chr[p_atk].lv<10)       //레벨에 따라 사용 가능한 스킬 해금
                {
                    printf("사용할 수 있는 스킬이 없습니다.\n");
                    usleep(500000);
                    continue;
                }
                else
                {
                    printf("1.럭키스트라이크\n");
                    s_unlock=1;
                }
                s_skill=PutKey(s_unlock);
                *pskill=SwordManSkill(chr, p_atk);
                if(chr[p_atk].mp < pskill->skillmp)
                {
                    printf("MP가 부족합니다.\n");
                    usleep(500000);
                    continue;
                }
                else if(sword_dur>0)
                {
                    printf("이미 적용된 스킬입니다.\n");
                    usleep(500000);
                    continue;
                }
                else if(sword_cd>0)
                {
                    printf("쿨타임 중 입니다.\n남은 쿨타임: %d턴\n", sword_cd);
                    usleep(500000);
                    continue;
                }
                chr[p_atk].mp-=pskill->skillmp;
                sword_buff=chr[p_atk].total_atk;
                chr[p_atk].total_atk=pskill->skillatk;
                printf("%s의 %s ! 공격력이 %d 상승했다.\n", chr[p_atk].name, pskill->skillname, chr[p_atk].total_atk-sword_buff);
                break;
            }
            usleep(500000);
            break;
            case 3:     //  아이템 사용
            printf("무슨 아이템?\n");
            scanf("%d", &s_item);
            if(s_item<0||s_item>10)
            {
                printf("1~10중에 선택해주세요\n");
                usleep(500000);
                continue;
            }
            if(ptr[s_item-1].count<=0)
            {
                printf("아이템이 부족합니다.\n");
                usleep(500000);
                continue;
            }
            else
            {
                printf("누구에게?\n");
                s_target=PutKey(5);
                s_target-=1;
                *ptr=itemList(chr, s_target, s_item-1);
                switch(s_item)
                {
                    case 1: // HP 하
                    case 2: // HP 중
                    case 3: // HP 상
                    chr[s_target].hp+=ptr->hp;
                    ptr->count--;
                    if(chr[s_target].hp>chr[s_target].max_hp);
                    chr[s_target].hp=chr[s_target].max_hp;
                    printf("%s에게 %s를 사용합니다.", chr[s_target].name, ptr->name);
                    break;

                    case 4: // MP 하
                    case 5: // MP 중
                    case 6: // MP 상
                    chr[s_target].mp+=ptr->mp;
                    ptr->count--;
                    if(chr[s_target].mp>chr[s_target].max_mp);
                    chr[s_target].mp=chr[s_target].max_mp;
                    printf("%s에게 %s를 사용합니다.", chr[s_target].name, ptr->name);
                    break;

                    case 7: // ALL 하
                    case 8: // ALL 중
                    case 9: // ALL 상
                    chr[s_target].hp+=ptr->hp;
                    chr[s_target].mp+=ptr->mp;
                    ptr->count--;
                    if(chr[s_target].hp>chr[s_target].max_hp);
                    chr[s_target].hp=chr[s_target].max_hp;
                    if(chr[s_target].mp>chr[s_target].max_mp);
                    chr[s_target].mp=chr[s_target].max_mp;
                    printf("%s에게 %s를 사용합니다.", chr[s_target].name, ptr->name);
                    break;

                    case 10: // 부활물약
                    if(chr[s_target].state==dead)   //대상이 죽었으면 부활
                    {
                        chr[s_target].state=allive;
                        chr[s_target].hp=chr[s_target].max_hp/10;
                        chr[s_target].mp=chr[s_target].max_mp/10;
                    }
                    else
                    {
                        printf("살아있는 대상에게는 사용할 수 없습니다.\n");
                        usleep(500000);
                        continue;
                    }
                    break;
                }
            }
            //아이템 효과 함수
            break;
            case 4:     //  도망가기
            printf("수호대는 등을 돌리고 도망쳤다!\n");
            usleep(500000);
            run=rand()%10+1;
            if(run<=3)
            {
                printf("수호대는 무사히 도망쳤다!\n");
                end=0;      //전투(반복문) 종료
                continue;
            }
            else
            printf("수호대는 도망치지 못했다.\n");
            usleep(500000);
            break;
            default:    //  예외처리
            break;
        }
        usleep(700000);
        Battlemap(battlemap, chr, ptr, mob, m_max);

        for(i=0; i<m_max; i++)  //몬스터 hp체크해 사망여부 갱신, 정산
        {
            if(mob[i].hp<=0)
            mob[i].state=dead;

            if(mob[i].state==dead)
            {
                exp+=mob[i].exp;
                death++;
            }
        }
        if(death==m_max)
        {
            //전투결과 처리result
            break;
        }
        else
        {
            death=0;
            exp=0;
        }

        //몬스터 턴 시작
        while(1)
        {
            if(m_atk>=m_max)
            m_atk=0;
            if(mob[m_atk].state==dead)
            m_atk++;
            else
            break;
        }

        m_skill=rand()%30+1;
        while(1)
        {
            m_target=rand()%5;
            if(chr[m_target].state==dead)                           //사망상태인 캐릭터 공격하지 않음
            continue;
            else if(mob[m_atk].class>0&&mob[m_target].state==dead)  //힐쓸때 사망상태인 수호대에게 사용불가
            continue;
            else
            break;
        }
        if(mob[m_atk].class==1) //전사면 스킬을 발동하지 않음
        m_skill=1;
        if(m_skill<30&&mob[m_atk].cd<=1)
        {
            switch(mob[m_atk].class)
            {
                case 2:     //백마법사
                mob[m_target].hp+= mob[m_target].hp/(rand()%15+10);
                printf("%s의 %s !\n", mob[m_atk].name, mob[m_atk].skill);
                break;

                case 3:     //흑마법사
                printf("%s의 %s !\n", mob[m_atk].name, mob[m_atk].skill);
                for(i=0; i<5; i++)
                {
                    if(chr[i].state==dead)
                    continue;
                    else
                    {
                        if(mob[m_atk].atk + (mob[m_atk].atk*(2/10)) - chr[i].total_def/2 <= 0)
                        continue;
                        else
                        chr[i].hp -= mob[m_atk].atk + (mob[m_atk].atk*(2/10)) - chr[i].total_def/2;
                    }
                }
                break;

                case 4:     //적마법사
                printf("%s의 %s !\n", mob[m_atk].name, mob[m_atk].skill);
                if(chr[m_target].hp/(rand()%15+10) - chr[m_target].total_def <= 0)
                printf("피해는 없었다.\n");
                else
                chr[m_target].hp-=chr[m_target].hp/(rand()%15+10) - chr[m_target].total_def;
                break;

                case 5:     //궁수
                printf("%s의 %s !\n", mob[m_atk].name, mob[m_atk].skill);
                if(chr[m_target].hp/(rand()%20+13) - chr[m_target].total_def <= 0)
                printf("피해는 없었다.\n");
                else
                chr[m_target].hp-=chr[m_target].hp/(rand()%20+13) - chr[m_target].total_def;
                break;

                case 6:     //검사
                guard_buff=mob[m_atk].atk;
                mob[m_atk].atk*=rand()%3+1;
                printf("%s의 %s !\n", mob[m_atk].name, mob[m_atk].skill);
                printf("적의 공격력이 증가했다.\n");
                break;

                case 7:     //던전 보스
                if((mob[m_atk].atk*2+rand()%100) - chr[m_target].total_def/2 <=0)
                printf("%s의 %s ! 효과는 없었다.\n", mob[m_atk].name, mob[m_atk].skill);
                else
                {
                    chr[m_target].hp -= (mob[m_atk].atk*2+rand()%100) - chr[m_target].total_def/2;
                    printf("%s의 %s !\n%s는 %d의 피해를 입었다.\n", mob[m_atk].name, mob[m_atk].skill, chr[m_target].name,
                    (mob[m_atk].atk*2+rand()%100) - chr[m_target].total_def/2);
                }
                break;

                default:    //수호대가 아닐때
                *pmskill=Nmob(mob, m_atk, mob[m_atk].type-1);
                printf("%s의 %s 발동!\n", mob[m_atk].name, pmskill->skillname);
                usleep(500000);

                if((mob[m_atk].atk+pmskill->skillatk) - (chr[m_target].total_def/2)<=0)
                printf("%s은(는) 아무런 대미지도 입지 않았다.\n", chr[m_target].name);
                else
                {
                    chr[m_target].hp-= (mob[m_atk].atk+pmskill->skillatk)-chr[m_target].total_def/2;
                    printf("%s은(는) %d의 피해를  입었다!\n", chr[m_target].name, (mob[m_atk].atk+pmskill->skillatk)-chr[m_target].total_def/2);
                }
                break;
            }
            mob[m_atk].cd=3;
        }
        else
        {
            printf("%s의 공격!\n", mob[m_atk].name);
            usleep(500000);
            if(mob[m_atk].atk<chr[m_target].def/2)
            printf("%s은(는) 아무런 대미지도 입지 않았다.\n", chr[m_target].name);
            else
            {
                chr[m_target].hp-= mob[m_atk].atk - chr[m_target].def / 2;
                printf("%s은(는) %d의 피해를 입었다!\n",chr[m_target].name, mob[m_atk].atk - chr[m_target].def/2);
            }
        }
        usleep(500000);
        for(i=0; i<5; i++)  //  캐릭터 hp체크해 사망 상태로 만들어줌
        {
            if(chr[i].state==allive&&chr[i].hp<=0)
            chr[i].state=dead;
        }
        //공격 순서 변경
        p_atk++;
        m_atk++;
        Battlemap(battlemap, chr, ptr, mob, m_max);
        for(i=0; i<5; i++)
        {
            if(p_atk>=5)
            p_atk=0;
            if(chr[p_atk].state==dead)
            p_atk++;
            else
            break;
        }

        if(white_dur1>0)
        white_dur1--;
        if(white_dur2>0)
        white_dur2--;
        if(sword_dur>0)
        sword_dur--;
        if(sword_cd>0)
        sword_cd--;
            
        for(i=0; i<5; i++)  //한 턴이 끝나면 지속시간, 쿨타임이 줄어든다
        {
            if(chr[i].state==dead)
            {
                death++;
                if(death==5)    //패배
                {
                    printf("수호대는 전멸했다.\n");
                    exit(0);
                    end=0;
                    result=3;
                    continue;
                }
            }
            
            if(white_dur1==0)
            chr[i].total_atk=buff_atk[i];
            if(white_dur2==0)
            chr[i].total_def=buff_def[i];
            if(sword_dur==0)
            chr[sword].total_atk=sword_buff;

            if(mob[guard_class].cd==1)
            mob[guard_class].atk=guard_buff;

            if(white_cd[i]>0)
            white_cd[i]--;
            if(black_cd[i]>0)
            black_cd[i]--;
            if(red_cd[i]>0)
            red_cd[i]--;
            if(archer_cd[i]>0)
            archer_cd[i]--;
        }
        death=0;
        for(i=0; i<m_max; i++)
        {
            if(mob[i].cd>1)
            mob[i].cd--;
        }
        turn++;
    }
    if(end!=0)  //승리
    {
        printf("적을 물리쳤다! \n");
        usleep(500000);
        for(i=0; i<5; i++)
        {
            chr[i].exp+=exp/5;
            if(chr[i].exp>=100)
            {
                printf("수호대의 레벨이 올랐다.\n");
                usleep(500000);
                chr[i].lv+=1;
                chr[i].exp=0;
                Renewal(chr);
            }
        }
    }
    else if(result!=3)  //도망
    result=end;

    switch(result)
    {
        case 2: //승리
        Item_Drop(ptr, armor);
        win=1;
        break;

        case 3: //패배
        break;
    }

    switch(m_max) 
    {
        case 1:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            break;
        case 2:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            break;
        case 3:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            break;
        case 4:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            battlemap[6][16]=0;
            battlemap[7][15]=0;
            battlemap[7][16]=0;
            break;
        case 5:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            battlemap[6][16]=0;
            battlemap[7][15]=0;
            battlemap[7][16]=0;
            battlemap[6][20]=0;
            battlemap[7][19]=0;
            battlemap[7][20]=0;
            break;
        case 6:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            battlemap[6][16]=0;
            battlemap[7][15]=0;
            battlemap[7][16]=0;
            battlemap[6][20]=0;
            battlemap[7][19]=0;
            battlemap[7][20]=0;
            battlemap[15][4]=0;
            battlemap[16][3]=0;
            battlemap[16][4]=0;
            break;
        case 7:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            battlemap[6][16]=0;
            battlemap[7][15]=0;
            battlemap[7][16]=0;
            battlemap[6][20]=0;
            battlemap[7][19]=0;
            battlemap[7][20]=0;
            battlemap[15][4]=0;
            battlemap[16][3]=0;
            battlemap[16][4]=0;
            battlemap[15][8]=0;
            battlemap[16][7]=0;
            battlemap[16][8]=0;
            break;
        case 8:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            battlemap[6][16]=0;
            battlemap[7][15]=0;
            battlemap[7][16]=0;
            battlemap[6][20]=0;
            battlemap[7][19]=0;
            battlemap[7][20]=0;
            battlemap[15][4]=0;
            battlemap[16][3]=0;
            battlemap[16][4]=0;
            battlemap[15][8]=0;
            battlemap[16][7]=0;
            battlemap[16][8]=0;
            battlemap[15][12]=0;
            battlemap[16][11]=0;
            battlemap[16][12]=0;
            break;
        case 9:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            battlemap[6][16]=0;
            battlemap[7][15]=0;
            battlemap[7][16]=0;
            battlemap[6][20]=0;
            battlemap[7][19]=0;
            battlemap[7][20]=0;
            battlemap[15][4]=0;
            battlemap[16][3]=0;
            battlemap[16][4]=0;
            battlemap[15][8]=0;
            battlemap[16][7]=0;
            battlemap[16][8]=0;
            battlemap[15][12]=0;
            battlemap[16][11]=0;
            battlemap[16][12]=0;
            battlemap[15][16]=0;
            battlemap[16][15]=0;
            battlemap[16][16]=0;
            break;
        case 10:
            battlemap[6][4]=0;
            battlemap[7][3]=0;
            battlemap[7][4]=0;
            battlemap[6][8]=0;
            battlemap[7][7]=0;
            battlemap[7][8]=0;
            battlemap[6][12]=0;
            battlemap[7][11]=0;
            battlemap[7][12]=0;    
            battlemap[6][16]=0;
            battlemap[7][15]=0;
            battlemap[7][16]=0;
            battlemap[6][20]=0;
            battlemap[7][19]=0;
            battlemap[7][20]=0;
            battlemap[15][4]=0;
            battlemap[16][3]=0;
            battlemap[16][4]=0;
            battlemap[15][8]=0;
            battlemap[16][7]=0;
            battlemap[16][8]=0;
            battlemap[15][12]=0;
            battlemap[16][11]=0;
            battlemap[16][12]=0;
            battlemap[15][16]=0;
            battlemap[16][15]=0;
            battlemap[16][16]=0;
            battlemap[15][20]=0;
            battlemap[16][19]=0;
            battlemap[16][20]=0;        
            break;        
    };
    
    return win;
}

Consumption itemList(Player *chr, int turn_atk, int n) // 소비 아이템 리스트 정의
{
    double num1=0.3;
    double num2=0.6;
    double num3=0.8;

    Consumption item[11] = {{"초급 HP회복 포션", 0, chr[turn_atk].max_hp*num1, 0},
                            {"중급 HP회복 포션", 0, chr[turn_atk].max_hp*num2, 0},
                            {"상급 HP회복 포션", 0, chr[turn_atk].max_hp*num3, 0},
                            {"초급 MP회복 포션", 0, 0, chr[turn_atk].max_mp*num1},
                            {"중급 MP회복 포션", 0, 0, chr[turn_atk].max_mp*num2},
                            {"상급 MP회복 포션", 0, 0, chr[turn_atk].max_mp*num3},
                            {"초급 엘릭서", 0, chr[turn_atk].max_hp * (25 / 100), chr[turn_atk].max_mp * (25 / 100)},
                            {"중급 엘릭서", 0, chr[turn_atk].max_hp * (50 / 100), chr[turn_atk].max_mp * (50 / 100)},
                            {"상급 엘릭서", 0, chr[turn_atk].max_hp * (75 / 100), chr[turn_atk].max_mp * (75 / 100)},
                            {"부활 포션", 0, chr[turn_atk].max_hp, chr[turn_atk].max_mp},
                            {"텐트", 0, chr[turn_atk].max_hp, chr[turn_atk].max_mp}};

   return item[n];
}

void Equipment_List(Equipment *armor)
{
    printf("=======================================================\n");
    printf("\n");
    printf("                    < Equipment >\n");
    printf("\n");
    printf("\n");
    printf("                      - Armor -\n");
    printf("\n");
    for (int i = 0; i < 62; i++)
    {
        if (i == 41)
        {
            printf("\n");
            printf("                      - Shield -\n");
            printf("\n");
        }
        if (i == 44)
        {
            printf("\n");
            printf("                      - Weapon -\n");
            printf("\n");
        }
        if (armor[i].count >= 1)
        {
            if (i < 6)
            {
                printf("                    %s\n", armor[i].head_name);
            }
            if (i >= 6 && i < 18)
            {
                printf("                    %s\n", armor[i].body_name);
            }
            if (i >= 18 && i < 26)
            {
                printf("                    %s\n", armor[i].pants_name);
            }
            if (i >= 26 && i < 32)
            {
                printf("                    %s\n", armor[i].cape_name);
            }
            if (i >= 32 && i < 41)
            {
                printf("                    %s\n", armor[i].glove_name);
            }
            if (i >= 41 && i < 44)
            {
                printf("                    %s\n", armor[i].left_name);
            }
            if (i >= 44 && i <= 62)
            {
                printf("                    %s\n", armor[i].right_name);
            }
        }
    }
    printf("\n");
    printf("=======================================================\n");
}

void lmt_Equipment(int slt_player, int slt_equiment, Player *chr, Equipment *armor) // 장비 착용에 직업별 제한을 거는 함수
{
    if (chr[slt_player].class == 1) //전사
    {
        if (slt_equiment >= 4 && slt_equiment <= 5) // 머리
        {
            chr[slt_player].head_eqm = armor[slt_equiment].head_name; // 캐릭터 구조체에 있는 아이템 배열에 장비 이름 삽입
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 15 && slt_equiment <= 17) // 상의
        {
            chr[slt_player].body_eqm = armor[slt_equiment].body_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 24 && slt_equiment <= 25) // 하의
        {
            chr[slt_player].pants_eqm = armor[slt_equiment].pants_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 26 && slt_equiment <= 31) // 망토
        {
            chr[slt_player].cape_eqm = armor[slt_equiment].cape_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 32 && slt_equiment <= 34) // 장갑
        {
            chr[slt_player].glove_eqm = armor[slt_equiment].glove_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment == 41 || slt_equiment == 43) // 방패
        {
            chr[slt_player].left_eqm = armor[slt_equiment].left_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 44 && slt_equiment <= 46) // 무기
        {
            chr[slt_player].right_eqm = armor[slt_equiment].right_name;
            chr[slt_player].total_atk = (armor[slt_equiment].atk + chr[slt_player].atk);
            armor[slt_equiment].count = 0;
        }
        else
        {
            printf("착용 가능한 직업이 아닙니다.\n");
        }
    }

    if (chr[slt_player].class == 2)
    {
        if (slt_equiment >= 0 && slt_equiment <= 1) // 머리
        {
            chr[slt_player].head_eqm = armor[slt_equiment].head_name; // 캐릭터 구조체에 있는 아이템 배열에 장비 이름 삽입
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 6 && slt_equiment <= 8) // 상의
        {
            chr[slt_player].body_eqm = armor[slt_equiment].body_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 18 && slt_equiment <= 19) // 하의
        {
            chr[slt_player].pants_eqm = armor[slt_equiment].pants_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 26 && slt_equiment <= 31) // 망토
        {
            chr[slt_player].cape_eqm = armor[slt_equiment].cape_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 38 && slt_equiment <= 40) // 장갑
        {
            chr[slt_player].glove_eqm = armor[slt_equiment].glove_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment == 41) // 방패
        {
            chr[slt_player].left_eqm = armor[slt_equiment].left_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 53 && slt_equiment <= 55) // 무기
        {
            chr[slt_player].right_eqm = armor[slt_equiment].right_name;
            chr[slt_player].total_atk = (armor[slt_equiment].atk + chr[slt_player].atk);
            armor[slt_equiment].count = 0;
        }
        else
        {
            printf("착용 가능한 직업이 아닙니다.\n");
        }
    }
    if (chr[slt_player].class == 3)
    {
        if (slt_equiment >= 0 && slt_equiment <= 1) // 머리
        {
            chr[slt_player].head_eqm = armor[slt_equiment].head_name; // 캐릭터 구조체에 있는 아이템 배열에 장비 이름 삽입
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 6 && slt_equiment <= 8) // 상의
        {
            chr[slt_player].body_eqm = armor[slt_equiment].body_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 18 && slt_equiment <= 19) // 하의
        {
            chr[slt_player].pants_eqm = armor[slt_equiment].pants_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 26 && slt_equiment <= 31) // 망토
        {
            chr[slt_player].cape_eqm = armor[slt_equiment].cape_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 32 && slt_equiment <= 34) // 장갑
        {
            chr[slt_player].glove_eqm = armor[slt_equiment].glove_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment == 41) // 방패
        {
            chr[slt_player].left_eqm = armor[slt_equiment].left_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 56 && slt_equiment <= 58) // 무기
        {
            chr[slt_player].right_eqm = armor[slt_equiment].right_name;
            chr[slt_player].total_atk = (armor[slt_equiment].atk + chr[slt_player].atk);
            armor[slt_equiment].count = 0;
        }
        else
        {
            printf("착용 가능한 직업이 아닙니다.\n");
        }
    }
    if (chr[slt_player].class == 4)
    {
        if (slt_equiment >= 2 && slt_equiment <= 3) // 머리
        {
            chr[slt_player].head_eqm = armor[slt_equiment].head_name; // 캐릭터 구조체에 있는 아이템 배열에 장비 이름 삽입
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 9 && slt_equiment <= 11) // 상의
        {
            chr[slt_player].body_eqm = armor[slt_equiment].body_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 20 && slt_equiment <= 21) // 하의
        {
            chr[slt_player].pants_eqm = armor[slt_equiment].pants_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 26 && slt_equiment <= 31) // 망토
        {
            chr[slt_player].cape_eqm = armor[slt_equiment].cape_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 35 && slt_equiment <= 37) // 장갑
        {
            chr[slt_player].glove_eqm = armor[slt_equiment].glove_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment == 42) // 방패
        {
            chr[slt_player].left_eqm = armor[slt_equiment].left_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 59 && slt_equiment <= 61) // 무기
        {
            chr[slt_player].right_eqm = armor[slt_equiment].right_name;
            chr[slt_player].total_atk = (armor[slt_equiment].atk + chr[slt_player].atk);
            armor[slt_equiment].count = 0;
        }
        else
        {
            printf("착용 가능한 직업이 아닙니다.\n");
        }
    }
    if (chr[slt_player].class == 5)
    {
        if (slt_equiment >= 2 && slt_equiment <= 3) // 머리
        {
            chr[slt_player].head_eqm = armor[slt_equiment].head_name; // 캐릭터 구조체에 있는 아이템 배열에 장비 이름 삽입
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 9 && slt_equiment <= 11) // 상의
        {
            chr[slt_player].body_eqm = armor[slt_equiment].body_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 20 && slt_equiment <= 21) // 하의
        {
            chr[slt_player].pants_eqm = armor[slt_equiment].pants_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 26 && slt_equiment <= 31) // 망토
        {
            chr[slt_player].cape_eqm = armor[slt_equiment].cape_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 35 && slt_equiment <= 37) // 장갑
        {
            chr[slt_player].glove_eqm = armor[slt_equiment].glove_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 50 && slt_equiment <= 52) // 무기
        {
            chr[slt_player].right_eqm = armor[slt_equiment].right_name;
            chr[slt_player].total_atk = (armor[slt_equiment].atk + chr[slt_player].atk);
            armor[slt_equiment].count = 0;
        }
        else
        {
            printf("착용 가능한 직업이 아닙니다.\n");
        }
    }
    if (chr[slt_player].class == 6)
    {
        if (slt_equiment >= 2 && slt_equiment <= 3) // 머리
        {
            chr[slt_player].head_eqm = armor[slt_equiment].head_name; // 캐릭터 구조체에 있는 아이템 배열에 장비 이름 삽입
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 12 && slt_equiment <= 14) // 상의
        {
            chr[slt_player].body_eqm = armor[slt_equiment].body_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 22 && slt_equiment <= 23) // 하의
        {
            chr[slt_player].pants_eqm = armor[slt_equiment].pants_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 29 && slt_equiment <= 31) // 망토
        {
            chr[slt_player].cape_eqm = armor[slt_equiment].cape_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 35 && slt_equiment <= 37) // 장갑
        {
            chr[slt_player].glove_eqm = armor[slt_equiment].glove_name;
            chr[slt_player].total_def = (armor[slt_equiment].def + chr[slt_player].def);
            armor[slt_equiment].count = 0;
        }
        else if (slt_equiment >= 47 && slt_equiment <= 49) // 무기
        {
            chr[slt_player].right_eqm = armor[slt_equiment].right_name;
            chr[slt_player].total_atk = (armor[slt_equiment].atk + chr[slt_player].atk);
            armor[slt_equiment].count = 0;
        }
        else
        {
            printf("착용 가능한 직업이 아닙니다.\n");
        }
    }
}

void Equipment_menu(Player *chr, Equipment *armor) // 장비창 메뉴 구성
{
    int slt, eqm_slt, chr_slt; // 선택
    Equipment_List(armor);

    printf("1) 장비 착용     2) 장착 장비 보기     3) 나가기\n");
    scanf("%d", &slt);
    printf("\n\n");
    switch (slt)
    {
        case 1: // 착용 가능장비 보기 & 장비 착용
        {
            for(int i =0; i<5; i++)
            {
                if(chr[i].class == 1)
                {
                    printf("전사는 중갑, 철, 모든방패, 한손검 착용이 가능합니다.\n");
                }
                if(chr[i].class == 2)
                {
                    printf("백법사는 천&가죽 장비와 방패, 숏스태프 착용이 가능합니다.\n");
                }
                if(chr[i].class == 3)
                {
                    printf("흑법사는 천&가죽 장비와 방패, 롱스태프 착용이 가능합니다.\n");
                }
                if(chr[i].class == 4)
                {
                    printf("적법사는 가죽류 장비와 사슬방패, 룬스태프 착용이 가능합니다.\n");
                }
                if(chr[i].class == 5)
                {
                    printf("궁수는 가죽 장비(방패 착용 불가), 활 착용이 가능합니다\n");
                }
                if(chr[i].class == 6)
                {
                    printf("검사는 경갑장비와 가죽투구(방패 착용 불가), 양손검 착용이 가능합니다.\n");
                }
            }
            usleep(500000);
            printf("장비를 착용할 캐릭터를 골라주세요.\n");
            for (int i = 0; i<5; i++)
            {
                printf("%d %s\n", i+1, chr[i].class_name);
            }
            scanf("%d", &chr_slt);

            system("clear");
            Equipment_List(armor);

            printf("착용할 장비를 선택해주세요.\n");
            scanf("%d", &eqm_slt);

            lmt_Equipment(chr_slt-1, eqm_slt, chr, armor);
            usleep(650000);
            break;
        }
        case 2: // 착용된 장비 보기
        {
            printf("\n\n");
            for(int i=0; i < 5; i++)
            {
                printf("%s\t : 상의 : %s\t하의 : %s\t머리 : %s\t망토 : %s\t장갑 : %s\t방패 : %s\t무기 : %s\n",chr[i].name, chr[i].body_eqm, chr[i].pants_eqm, chr[i].head_eqm, chr[i].cape_eqm, chr[i].glove_eqm, chr[i].left_eqm, chr[i].right_eqm);
            }
            printf("\n");
            scanf("%d", &slt);
            break;
        }
        case 3: // 나가기
        {
            break;
        }
    }
}

void Map(int (*map)[24], Player *chr, Consumption *ptr)  //일반 맵화면 출력함수
{
    int x, y;
    int key;

    int itemlist[24][3]={
        {9,9,6},
        {0,10,8},
        {11,31,8},
        {12,32,8},
        {13,33,8},
        {14,34,8},
        {15,35,8},
        {16,36,8},
        {17,37,8},
        {18,38,8},
        {19,39,8},
        {20,40,8},
        {21,41,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {1,2,8},
        {3,4,8},
        {0,0,8},
        {9,9,6}
    };

        for(int i=0; i<24; i++)
        {
            if(map[0][i]==9)
                printf("───");
            else if(map[0][i]==7)
                printf("╭");    
            else  if(map[0][i]==6)
                printf("╮");

        }
        for(int i=0; i<3; i++)
        {
            if(itemlist[0][i]==9)
                printf("─────────────");
            else if(itemlist[0][i]==6)
                printf("╮\n");
        }
        for(int i=0; i<24; i++)
        {
            if(map[1][i]==1) 
                printf("🔥 ");
            else if(map[1][i]==2)    
                printf("❄  ");
            else if(map[1][i]==8)
                printf("│");        
        }
        for(int i=0; i<3; i++)
        {
            if(itemlist[1][i]==10)
                printf("  보유 아이템 리스트   ");
            else if(itemlist[1][i]==8)
                printf("  │");
            else if(itemlist[1][i]==0)
                printf(" ");            
        }
        printf("\n");     
        for(int i=2; i<22; i++)
        {
            for(int j=0; j<24; j++)
            {
                
                if(map[i][j]==0)
                    printf("   ");
                else if(map[i][j]==1)
                    printf("🔥");
                else if(map[i][j]==2)    
                    printf(" ❄  ");
                else if(map[i][j]==3)
                    printf("🌳");
                else if (map[i][j]==4)
                    printf(" 💧 ");    
                else if(map[i][j]==5)
                {
                    printf("⚪ ");   
                    x=i;
                    y=j;
                }
                else if(map[i][j]==8)
                    printf("│");
                else if(map[i][j]==10)
                    printf("🚪 ");
            }
            for(int k=0; k<3; k++)
            {
                if(itemlist[i][k]==11)
                    printf("  HP하         ");
                else if(itemlist[i][k]==12)
                    printf("  HP중        ");
                else if(itemlist[i][k]==13)
                    printf("  HP상        ");
                else if(itemlist[i][k]==14)
                    printf("  MP하        ");
                else if(itemlist[i][k]==15)
                    printf("  MP중        ");
                else if(itemlist[i][k]==16)
                    printf("  MP상        ");
                else if(itemlist[i][k]==17)
                    printf("  ALL하       ");    
                else if(itemlist[i][k]==18)
                    printf("  All중       ");  
                else if(itemlist[i][k]==19)
                    printf("  All상       ");
                else if(itemlist[i][k]==20)
                    printf("  부활물약    ");
                else if(itemlist[i][k]==21)
                    printf("  텐트      ");      
                else if(itemlist[i][k]==0)
                    printf("             ");
                else if(itemlist[i][k]==8)
                    printf("│");        
                else if(itemlist[i][k]==31)
                    printf("      %2d   ", ptr[0].count);
                else if(itemlist[i][k]==32)
                    printf("       %2d   ", ptr[1].count);
                else if(itemlist[i][k]==33)    
                    printf("       %2d   ", ptr[2].count);
                else if(itemlist[i][k]==34)
                    printf("       %2d   ", ptr[3].count);
                else if(itemlist[i][k]==35)
                    printf("       %2d   ", ptr[4].count);
                else if(itemlist[i][k]==36)
                    printf("       %2d   ", ptr[5].count);
                else if(itemlist[i][k]==37)    
                    printf("       %2d   ", ptr[6].count);
                else if(itemlist[i][k]==38)
                    printf("       %2d   ", ptr[7].count);    
                else if(itemlist[i][k]==39)
                    printf("       %2d   ", ptr[8].count);
                else if(itemlist[i][k]==40)
                    printf("       %2d   ", ptr[9].count);
                else if(itemlist[i][k]==41)    
                    printf("         %2d   ", ptr[10].count);    
                else if(itemlist[i][k]==1)
                    printf("  메뉴창 : ");
                else if(itemlist[i][k]==2)
                    printf(" M             ");  \
                else if(itemlist[i][k]==3)
                    printf("  조작키 : ");
                else if(itemlist[i][k]==4)
                    printf(" w a s d       ");            
            }      
            printf("\n");
        }
        for(int i=0; i<24; i++)
        {
            if(map[22][i]==3)
                printf("🌳 ");
            else if(map[22][i]==4)   
                printf("💧 "); 
            else if(map[22][i]==8)
                printf("│");    
        }
        if(itemlist[22][2]==8)
            printf("                          │\n");
        for(int i=0; i<24; i++)
        {
            if(map[23][i]==9)
                printf("───");
            if(map[23][i]==7)
                printf("╰");
            if(map[23][i]==6)
                printf("╯");        
        }
        for(int i=0; i<3; i++)
        {
            if(itemlist[23][i]==9)
                printf("─────────────");
            else if(itemlist[23][i]==6)
                printf("╯\n"); 
        }

        printf("╭──────────────────╮");
        printf("─────────────────╮");
        printf("──────────────────╮");
        printf("─────────────────╮");
        printf("───────────────────╮\n");
        printf("│%10s   이시영 │%10s  이영광 │%10s   원영진 │%10s  박영진 │%10s   박영선  │\n", chr[0].class_name, chr[1].class_name, chr[2].class_name, chr[3].class_name, chr[4].class_name);
        printf("│ HP        %5d  │ HP       %5d  │ HP         %5d │ HP        %5d │ HP         %5d  │\n", chr[0].hp, chr[1].hp, chr[2].hp, chr[3].hp, chr[4].hp);
        printf("│ MP        %5d  │ MP       %5d  │ MP         %5d │ MP        %5d │ MP         %5d  │\n", chr[0].mp, chr[1].mp, chr[2].mp, chr[3].mp, chr[4].mp);
        printf("│ atk       %5d  │ atk      %5d  │ atk        %5d │ atk       %5d │ atk        %5d  │\n", chr[0].total_atk, chr[1].total_atk, chr[2].total_atk, chr[3].total_atk, chr[4].total_atk);
        printf("│ def       %5d  │ def      %5d  │ def        %5d │ def       %5d │ def        %5d  │\n", chr[0].total_def, chr[1].total_def, chr[2].total_def, chr[3].total_def, chr[4].total_def);
        printf("│ exp       %5d  │ exp      %5d  │ exp        %5d │ exp       %5d │ exp        %5d  │\n", chr[0].exp, chr[1].exp, chr[2].exp, chr[3].exp, chr[4].exp);
        printf("╰──────────────────╯");
        printf("─────────────────╯");
        printf("──────────────────╯");
        printf("─────────────────╯");
        printf("───────────────────╯\n");

        if((x>=2&&x<=11)&&(y>=2&&y<=11))
        {
            chr->area=1;
        }
        else if((x>=2 && x<=11) &&(y>=12 && y<=21))
        {
            chr->area=2;
        }
        else if((x>=12 && x<=21) &&(y>=2 && y<=11))
        {
            chr->area=3;
        }
        else if((x>=12 && x<=21) &&(y>=12 && y<=21))
        {
            chr->area=4;
        }
}

int Move(int (*map)[24], char* hotkey, Equipment* equip, Player *chr, int** arr, Consumption*ptr, int (*battlemap)[24])  //플레이어 이동 함수
{
    int x, y;
    int key, equip_s;
    int menu=0, cursor=0, end=0;
    int battle_count=0;
    int join=0;
    for(int i=0; i<24; i++)  //플레이어 찾기
    {
        for(int j=0; j<24; j++)
        {
            if(map[i][j] == 5)
            {
                x = i;
                y = j;
            }
        }
    }
    key=getch();

    if(key == 'a')  //a는 왼쪽방향
    {
        if(map[x][y-1] == 0)  //0(빈공간)일때만 이동가능
        {
            map[x][y] = 0;  //현재위치는 0으로 바뀜
            map[x][y-1] = 5;  //왼쪽으로 이동
        }
        else if(map[x][y-1] == 10)
        {
            map[x][y] = 0;
            system("clear");
            join=1;
        }
    }
    else if(key == 'd')  //d는 오른쪽방향
    {
        if(map[x][y+1] == 0)
        {
            map[x][y] = 0;
            map[x][y+1] = 5;
        }
        else if(map[x][y+1] == 10)
        {
            map[x][y] = 0;
            system("clear");
            join=1;
        }
    }
    else if(key == 'w')  //w는 위쪽방향
    {
        if(map[x-1][y] == 0)
        {
            map[x][y] = 0;
            map[x-1][y] = 5;
        }
        else if(map[x-1][y] == 10)
        {
            map[x][y] = 0;
            system("clear");
            join=1;
        }
    }
    else if(key == 's')  //s는 아래방향
    {
        if(map[x+1][y] == 0)
        {
            map[x][y] = 0;
            map[x+1][y] = 5;
        }
        else if(map[x+1][y] == 10)
        {
            map[x][y] = 0;
            system("clear");
            join=1;
        }
    }
    else if(key == 'm') // 메뉴
    {
        while(1)
        {
            system("clear");
            if(cursor==0)
            printf("💎 단축키 설정\n장비창\n닫기\n");
            else if(cursor==1)
            printf("단축키 설정\n💎 장비창\n닫기\n");
            else if(cursor==2)
            printf("단축키 설정\n장비창\n💎 닫기\n");
            key=getch();
            if(key ==hotkey[0])          //커서 위로 이동 w
            {
                cursor--;
                if((cursor)<0)
                cursor=0;
            }
            else if(key==hotkey[1])      // 커서 아래로 이동 s
            {
                cursor++;
                if(cursor>2)
                cursor=2;
            }
            else if(key==' '&&cursor==0)    //단축키 설정 메뉴
            {
                while(end==0)
                {
                    clearScreen();
                    if(cursor==0)
                    {
                        printf("💎위: %c\n", hotkey[0]);
                        printf("아래: %c\n", hotkey[1]);
                        printf("왼쪽: %c\n", hotkey[2]);
                        printf("오른쪽: %c\n", hotkey[3]);
                        printf("나가기\n");
                    }
                    else if(cursor==1)
                    {
                        printf("위: %c\n", hotkey[0]);
                        printf("💎아래: %c\n", hotkey[1]);
                        printf("왼쪽: %c\n", hotkey[2]);
                        printf("오른쪽: %c\n", hotkey[3]);
                        printf("나가기\n");
                    }
                    else if(cursor==2)
                    {
                        printf("위: %c\n", hotkey[0]);
                        printf("아래: %c\n", hotkey[1]);
                        printf("💎왼쪽: %c\n", hotkey[2]);
                        printf("오른쪽: %c\n", hotkey[3]);
                        printf("나가기\n");
                    }
                    else if(cursor==3)
                    {
                        printf("위: %c\n", hotkey[0]);
                        printf("아래: %c\n", hotkey[1]);
                        printf("왼쪽: %c\n", hotkey[2]);
                        printf("💎오른쪽: %c\n", hotkey[3]);
                        printf("나가기\n");
                    }
                    else if(cursor==4)
                    {
                        printf("위: %c\n", hotkey[0]);
                        printf("아래: %c\n", hotkey[1]);
                        printf("왼쪽: %c\n", hotkey[2]);
                        printf("오른쪽: %c\n", hotkey[3]);
                        printf("💎나가기\n");
                    }
                    key=getch();
                    if(key ==hotkey[0])          //커서 위로 이동 w
                    {
                        cursor--;
                        if((cursor)<0)
                        cursor=0;
                    }
                    else if(key==hotkey[1])      // 커서 아래로 이동 s
                    {
                        cursor++;
                        if(cursor>4)
                        cursor=4;
                    }
                    else if(key==' ')
                    {
                        if(cursor!=4)
                        {
                            printf("바꿀 키를 입력해주세요\n");
                            key=getch();
                        }
                        switch(cursor)
                        {
                            case 0: //위
                            hotkey[cursor]=key;
                            break;
                            
                            case 1: //아래
                            hotkey[cursor]=key;
                            break;

                            case 2: //왼쪽
                            hotkey[cursor]=key;
                            break;

                            case 3: //오른쪽
                            hotkey[cursor]=key;
                            break;

                            case 4: //나가기
                            end=1;
                            continue;
                        }
                    }
                }
                if(end==1)
                {
                    end=0;
                    cursor=0;
                    continue;
                }
            }
            else if(key==' '&&cursor==1)    //인벤토리
            {
                clearScreen();
                Equipment_menu(chr, equip);
            }
            else if(key==' '&&cursor==2)    //닫기
            {
                break;
            }
        }
    }
    if(join==1)
    {
        dunp(chr, ptr, equip, battlemap);
        chr->floor=1;
        return 0;
    }
    if(arr[x][y]<2){// 몬스터 출현(30%)
        battle_count=Battle(chr, ptr, equip, battlemap, 1);
    }
    else if(arr[x][y]>=3&&arr[x][y]<4)
    {// 수호대 출현(10%)
        if(chr->lv>=15&&chr->lv<=20)
        battle_count=Battle(chr, ptr, equip, battlemap, 2);
    }
    return battle_count;
}

void Battlemap(int (*battlemap)[24], Player *chr, Consumption *ptr, Monster *mob, int m_max)  //전투화면 맵 출력함수
{
    int itemlist[24][3]={
        {9,9,6},
        {10,0,8},
        {11,31,8},
        {12,32,8},
        {13,33,8},
        {14,34,8},
        {15,35,8},
        {16,36,8},
        {17,37,8},
        {18,38,8},
        {19,39,8},
        {20,40,8},
        {21,41,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {0,0,8},
        {9,9,7}
    };
    
    int num=0, m_hp=0;

    system("clear");

    //몬스터 (6,4)  (6,8)  (6,12)  (6,16)  (6,20)
    //HP (7,3)  (7,7)  (7,11)  (7,15)  (7,19)
    //숫자 (7,4)  (7,8)  (7,12)  (7,16)  (7,20)

    //몬스터 (15,4)  (15,8)  (15,12)  (15,16)  (15,20)
    //HP (16,3)  (16,7)  (16,11)  (16,15)  (16,19)
    //숫자 (16,4)  (16,8)  (16,12)  (16,16)  (16,20)
    switch(m_max) 
    {
        case 1:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            break;
        case 2:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            break;
        case 3:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            break;
        case 4:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            battlemap[6][16]=10;
            battlemap[7][15]=12;
            battlemap[7][16]=11;
            break;
        case 5:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            battlemap[6][16]=10;
            battlemap[7][15]=12;
            battlemap[7][16]=11;
            battlemap[6][20]=10;
            battlemap[7][19]=12;
            battlemap[7][20]=11;
            break;
        case 6:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            battlemap[6][16]=10;
            battlemap[7][15]=12;
            battlemap[7][16]=11;
            battlemap[6][20]=10;
            battlemap[7][19]=12;
            battlemap[7][20]=11;
            battlemap[15][4]=10;
            battlemap[16][3]=12;
            battlemap[16][4]=11;
            break;
        case 7:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            battlemap[6][16]=10;
            battlemap[7][15]=12;
            battlemap[7][16]=11;
            battlemap[6][20]=10;
            battlemap[7][19]=12;
            battlemap[7][20]=11;
            battlemap[15][4]=10;
            battlemap[16][3]=12;
            battlemap[16][4]=11;
            battlemap[15][8]=10;
            battlemap[16][7]=12;
            battlemap[16][8]=11;
            break;
        case 8:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            battlemap[6][16]=10;
            battlemap[7][15]=12;
            battlemap[7][16]=11;
            battlemap[6][20]=10;
            battlemap[7][19]=12;
            battlemap[7][20]=11;
            battlemap[15][4]=10;
            battlemap[16][3]=12;
            battlemap[16][4]=11;
            battlemap[15][8]=10;
            battlemap[16][7]=12;
            battlemap[16][8]=11;
            battlemap[15][12]=10;
            battlemap[16][11]=12;
            battlemap[16][12]=11;
            break;
        case 9:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            battlemap[6][16]=10;
            battlemap[7][15]=12;
            battlemap[7][16]=11;
            battlemap[6][20]=10;
            battlemap[7][19]=12;
            battlemap[7][20]=11;
            battlemap[15][4]=10;
            battlemap[16][3]=12;
            battlemap[16][4]=11;
            battlemap[15][8]=10;
            battlemap[16][7]=12;
            battlemap[16][8]=11;
            battlemap[15][12]=10;
            battlemap[16][11]=12;
            battlemap[16][12]=11;
            battlemap[15][16]=10;
            battlemap[16][15]=12;
            battlemap[16][16]=11;
            break;
        case 10:
            battlemap[6][4]=10;
            battlemap[7][3]=12;
            battlemap[7][4]=11;
            battlemap[6][8]=10;
            battlemap[7][7]=12;
            battlemap[7][8]=11;
            battlemap[6][12]=10;
            battlemap[7][11]=12;
            battlemap[7][12]=11;    
            battlemap[6][16]=10;
            battlemap[7][15]=12;
            battlemap[7][16]=11;
            battlemap[6][20]=10;
            battlemap[7][19]=12;
            battlemap[7][20]=11;
            battlemap[15][4]=10;
            battlemap[16][3]=12;
            battlemap[16][4]=11;
            battlemap[15][8]=10;
            battlemap[16][7]=12;
            battlemap[16][8]=11;
            battlemap[15][12]=10;
            battlemap[16][11]=12;
            battlemap[16][12]=11;
            battlemap[15][16]=10;
            battlemap[16][15]=12;
            battlemap[16][16]=11;
            battlemap[15][20]=10;
            battlemap[16][19]=12;
            battlemap[16][20]=11;        
            break;        
    };

    for (int i=0; i<24; i++)
    {
        for(int j=0; j<24; j++)
        {
            if(battlemap[i][j]==0)
                printf("   ");
            else if(battlemap[i][j]==1)
                printf("╭");
            else if(battlemap[i][j]==2)
                printf("╮");
            else if(battlemap[i][j]==3)       
                printf("╰");
            else if(battlemap[i][j]==4)
                printf("╯"); 
            else if(battlemap[i][j]==8)
                printf("│");
            else if(battlemap[i][j]==9)
                printf("───");   
            else if(battlemap[i][j]==10)
            {
                if(mob[num].type==1)
                    printf("😡 ");    //불슬라임
                else if(mob[num].type==2)
                    printf("🥶 ");    //눈슬라임
                else if(mob[num].type==3)
                    printf("🤢 ");    //숲슬라임
                else if(mob[num].type==4)
                    printf("😭 ");    //물슬라임
                else if(mob[num].class>0&&mob[num].class<7)
                    printf("⚫ ");
                else if(mob->floor>0&&mob->floor<6)
                    printf("👿 ");

                num++;                        
            }    
            else if(battlemap[i][j]==11)
            {
                printf("%4d", mob[m_hp].hp);   //몬스터 체력
                m_hp++;
            }
            else if(battlemap[i][j]==12)
                printf("HP");                                                              
        }
        
        if(i==1)
        {
            for(int i=0; i<3; i++)
            {
                if(itemlist[1][i]==10)
                    printf("   보유 아이템 리스트  ");
                else if(itemlist[1][i]==8)
                    printf("  │");
                else if(itemlist[1][i]==0)
                    printf(" ");            
            }  
        }

        for(int k=0; k<3; k++)
        {
            if(itemlist[i][k]==11)
                printf("  HP하         ");
            else if(itemlist[i][k]==12)
                printf("  HP중        ");
            else if(itemlist[i][k]==13)
                printf("  HP상        ");
            else if(itemlist[i][k]==14)
                printf("  MP하        ");
            else if(itemlist[i][k]==15)
                printf("  MP중        ");
            else if(itemlist[i][k]==16)
                printf("  MP상        ");
            else if(itemlist[i][k]==17)
                printf("  ALL하       ");    
            else if(itemlist[i][k]==18)
                printf("  All중       ");  
            else if(itemlist[i][k]==19)
                printf("  All상       ");
            else if(itemlist[i][k]==20)
                printf("  부활물약    ");
            else if(itemlist[i][k]==21)
                printf("  텐트      ");                  
            else if(itemlist[i][k]==0)
                printf("             ");
            else if(itemlist[i][k]==8 && i!=1)
                printf("│");          
            else if(itemlist[i][k]==9)
                printf("─────────────");
            else if(itemlist[i][k]==6)
                printf("╮");
            else if(itemlist[i][k]==7)
                printf("╯");
            else if(itemlist[i][k]==31)
                printf("      %2d   ", ptr[0].count);
            else if(itemlist[i][k]==32)
                printf("       %2d   ", ptr[1].count);
            else if(itemlist[i][k]==33)    
                printf("       %2d   ", ptr[2].count);
            else if(itemlist[i][k]==34)
                printf("       %2d   ", ptr[3].count);
            else if(itemlist[i][k]==35)
                printf("       %2d   ", ptr[4].count);
            else if(itemlist[i][k]==36)
                printf("       %2d   ", ptr[5].count);
            else if(itemlist[i][k]==37)    
                printf("       %2d   ", ptr[6].count);
            else if(itemlist[i][k]==38)
                printf("       %2d   ", ptr[7].count);    
            else if(itemlist[i][k]==39)
                printf("       %2d   ", ptr[8].count);
            else if(itemlist[i][k]==40)
                printf("       %2d   ", ptr[9].count);
            else if(itemlist[i][k]==41)    
                printf("         %2d   ", ptr[10].count);
        }
        printf("\n");
    }
    
    printf("╭──────────────────╮");
    printf("─────────────────╮");
    printf("──────────────────╮");
    printf("─────────────────╮");
    printf("───────────────────╮\n");
    printf("│%10s   이시영 │%10s  이영광 │%10s   원영진 │%10s  박영진 │%10s   박영선  │\n", chr[0].class_name, chr[1].class_name, chr[2].class_name, chr[3].class_name, chr[4].class_name);
    printf("│ HP        %5d  │ HP       %5d  │ HP         %5d │ HP        %5d │ HP         %5d  │\n", chr[0].hp, chr[1].hp, chr[2].hp, chr[3].hp, chr[4].hp);
    printf("│ MP        %5d  │ MP       %5d  │ MP         %5d │ MP        %5d │ MP         %5d  │\n", chr[0].mp, chr[1].mp, chr[2].mp, chr[3].mp, chr[4].mp);
    printf("│ atk       %5d  │ atk      %5d  │ atk        %5d │ atk       %5d │ atk        %5d  │\n", chr[0].total_atk, chr[1].total_atk, chr[2].total_atk, chr[3].total_atk, chr[4].total_atk);
    printf("│ def       %5d  │ def      %5d  │ def        %5d │ def       %5d │ def        %5d  │\n", chr[0].total_def, chr[1].total_def, chr[2].total_def, chr[3].total_def, chr[4].total_def);
    printf("│ exp       %5d  │ exp      %5d  │ exp        %5d │ exp       %5d │ exp        %5d  │\n", chr[0].exp, chr[1].exp, chr[2].exp, chr[3].exp, chr[4].exp);
    printf("╰──────────────────╯");
    printf("─────────────────╯");
    printf("──────────────────╯");
    printf("─────────────────╯");
    printf("───────────────────╯\n");
}

//화면 지우는 함수
void clearScreen(){
    printf("\033[2J\033[H");
}

//오프닝
void Opening(){
    for(int i = 0 ; i <2 ; i++){
        clearScreen();
        printf("□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□\n□■□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□□□■■■■■■□□□□□■■■■■■□□□□□□□□□□□□□□□□□■\n□■□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□□□■□□□□□□□□□□■□□□■■□□□□□□□□□□□□□□□□□□\n□■□□□□□□■■■■■■□■■■■■■□□■■■■■■□■■■■■■□■■■■■■□□□□□■■■■■■□■□□□□□□□□□□■□□□■■□□■■■■■■□■■■■■■□□■\n□■□□□□□□■■□□■■□■■□□■■□□■■□□■■□■■□□■■□■■□□■■□□□□□■■□□■■□■□□□□□□□□□□■■■■■■□□■■□□■■□■■□□■■□□■\n□■□□□□□□■□□□□■□■□□□□■□□■□□□□■□■□□□□■□■□□□□■□□□□□■□□□□■□■■■■■■□□□□□■□□□□■■□■□□□□■□■□□□□■□□■\n□■□□□□□□■■■■■■□■□□□□■□□■■■■■■□■□□□□■□■□□□□■□□□□□■□□□□■□■□□□□□□□□□□■□□□□□■□■□□□□■□■□□□□■□□■\n□■□□□□□□■□□□□□□■□□□□■□□■□□□□□□■□□□□■□■□□□□■□□□□□■□□□□■□■□□□□□□□□□□■□□□□□■□■□□□□■□■□□□□■□□■\n□■□□□□□□■■□□□■□■■□□■■□□■■□□□■□■□□□□■□■■□□■■□□□□□■■□□■■□■□□□□□□□□□□■□□□□■■□■■□□■■□■■□□■■□□■\n□■■■■■■□■■■■■■□■■■■■■□□■■■■■■□■□□□□■□■■■■■■□□□□□■■■■■■□■□□□□□□□□□□■■■■■■■□■■■■■■□■■■■■■□□■\n□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□■□□□\n□□□□□□□□□□□□□□□□■□□■■□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□■□□■■□□□\n□□□□□□□□□□□□□□□■■■■■■□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□□■■■■■■□□□\n");
        sleep(1);
        clearScreen();
        printf("■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■□■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n■□■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■□■■■■■■■■■■■■□□□□□□■■■■■□□□□□□■■■■■■■■■■■■■■■■■□\n■□■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■□■■■■■■■■■■■■□■■■■■■■■■■□■■■□□■■■■■■■■■■■■■■■■■■\n■□■■■■■■□□□□□□■□□□□□□■■□□□□□□■□□□□□□■□□□□□□■■■■■□□□□□□■□■■■■■■■■■■□■■■□□■■□□□□□□■□□□□□□■■□\n■□■■■■■■□□■■□□■□□■■□□■■□□■■□□■□□■■□□■□□■■□□■■■■■□□■■□□■□■■■■■■■■■■□□□□□□■■□□■■□□■□□■■□□■■□\n■□■■■■■■□■■■■□■□■■■■□■■□■■■■□■□■■■■□■□■■■■□■■■■■□■■■■□■□□□□□□■■■■■□■■■■□□■□■■■■□■□■■■■□■■□\n■□■■■■■■□□□□□□■□■■■■□■■□□□□□□■□■■■■□■□■■■■□■■■■■□■■■■□■□■■■■■■■■■■□■■■■■□■□■■■■□■□■■■■□■■□\n■□■■■■■■□■■■■■■□■■■■□■■□■■■■■■□■■■■□■□■■■■□■■■■■□■■■■□■□■■■■■■■■■■□■■■■■□■□■■■■□■□■■■■□■■□\n■□■■■■■■□□■■■□■□□■■□□■■□□■■■□■□■■■■□■□□■■□□■■■■■□□■■□□■□■■■■■■■■■■□■■■■□□■□□■■□□■□□■■□□■■□\n■□□□□□□■□□□□□□■□□□□□□■■□□□□□□■□■■■■□■□□□□□□■■■■■□□□□□□■□■■■■■■■■■■□□□□□□□■□□□□□□■□□□□□□■■□\n■■■■■■■■■■■■■■■■■■■■□■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■□■■■\n■■■■■■■■■■■■■■■■□■■□□■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■□■■□□■■■\n■■■■■■■■■■■■■■■□□□□□□■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■□□□□□□■■■\n");
        sleep(1);
    }
    printf("Press any Key >>");
    getch();
    clearScreen();
    printf("\n꿈꾸는 실습실 에서 공주를 구한 복이는 용사의 칭호를 받으며 행복한 나날을 보내지만\n혹시나 하는 악의 무리의 공격에 대비해 4개의 수호대를 결성하고 혹시나하는 악의 침공에 대비했다 \n다음(아무키나 누르세요.)>>");
    getch();
    clearScreen();
    printf("\n어느날밤 하늘에서 떨어진 운석을 조사 하로 용사복이는 길을 떠났다. \n용사 복이가 떠난지 수개월 .....\n다음(아무키나 누르세요.)>>");
    getch();
    clearScreen();
    printf("\n용사복이가 돌아오지 않자 그에 의해 결성된 4개의 수호대는 각각 리더의 명령에 따라 \n용사 복이 행방과 운석이 떨어 진 곳을 조사 하기위해 \n각자의 지역에서 비슷한 시기에 출발하게되는데....\n다음(아무키나 누르세요.)>>");
    getch();
    clearScreen();
    printf("\n그로부터 몇주뒤 각 수호대의 존재는 서로 몰랐던 수호대는 \n각 지역과 미궁의 던젼에서 서로의 존재를 확인 하고 \n적인지 아군인지 모르는 상황에서 전투가 벌어 지고 \n다음(아무키나 누르세요.)>>");
    getch();
    clearScreen();
    printf("\n각 지역 몬스터와 미궁의 던젼의 몬스터와 \n보스몬스터와도 싸우면서 용사복이의 행방과 미궁의 던젼을 조사 해 나간다\n다음(아무키나 누르세요.)>>");
    getch();
    clearScreen();
}

//몬스터 확률 좌표 배열 생성 함수
int** MonsterAppearanceProbability(){
    int** arr = (int**)malloc((sizeof(int*)*24)*24);
    for(int i = 0 ; i < 24 ; i++){
        arr[i]=(int*)malloc(sizeof(int)*24);
    } 
    return arr;
}

//몬스터 확률 좌표 난수 담기 함수
int** RandNumIntoMonsterAppearanceProbability(int** arr){
    //확률 담기(33%)
    for(int i = 2 ; i < 22 ; i++){
        for(int j = 2 ; j <22 ; j++){
            arr[i][j]=rand()%10+1;//1~10
            //초기 생성에서 수호대와 겹치지 않게 생성
            // if(i==2&&j==11||i==11&&j==2||i==11&&j==21||i==21&&j==11){
            //     arr[i][j]=;
            // }
        }
    }
    return arr;
}

//몬스터 확률 좌표 2D배열 메모리 동적할당 해제 함수
void memoryFreeForMonsters(int** monsterProbArr){
    for(int i = 0 ; i < 20 ; i++){
        free(monsterProbArr[i]);
    }
    free(monsterProbArr);
}

//수호대 확률 좌표 2D배열 메모리 동적할당 해제 함수
void memoryFreeForGuardians(int** arr){
    for(int i = 0 ; i < 4 ; i++){
        free(arr[i]);
    }
    free(arr);
}

void Item_Drop(Consumption *item, Equipment *armor) // 아이템 드롭 시스템
{
    srand((unsigned)time(NULL));
    int drop_eqm =rand()%62;
    int drop_csmp = rand()%11;

    if(drop_csmp > 0 && drop_csmp <= 8 || drop_csmp == 10)
    {
        printf("%s을(를) 얻었습니다.\n", item[drop_csmp].name); // 소비 아이템
        item[drop_csmp].count++;
    }

    if(drop_eqm <= 5) // 모자 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].head_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 5 && drop_eqm <= 17) // 상의 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].body_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 17 && drop_eqm <= 25) // 하의 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].pants_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 25 && drop_eqm <= 31) // 망토 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].cape_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 31 && drop_eqm <= 40) // 장갑 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].glove_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 40 && drop_eqm <= 43) // 방패 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].left_name);
        armor[drop_eqm].count++;
    }
    else // 무기 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].right_name);
        armor[drop_eqm].count++;
    }

    if(drop_csmp > 0 && drop_csmp <= 8 || drop_csmp == 10)
    {
        printf("%s을(를) 얻었습니다.\n", item[drop_csmp].name); // 소비 아이템
        item[drop_csmp].count++;
    }

    if(drop_eqm <= 5) // 모자 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].head_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 5 && drop_eqm <= 17) // 상의 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].body_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 17 && drop_eqm <= 25) // 하의 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].pants_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 25 && drop_eqm <= 31) // 망토 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].cape_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 31 && drop_eqm <= 40) // 장갑 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].glove_name);
        armor[drop_eqm].count++;
    }
    else if(drop_eqm > 40 && drop_eqm <= 43) // 방패 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].left_name);
        armor[drop_eqm].count++;
    }
    else // 무기 아이템 드롭
    {
        printf("%s을(를) 얻었습니다.\n", armor[drop_eqm].right_name);
        armor[drop_eqm].count++;
    }
    printf("%s을(를) 얻었습니다.\n", item[9].name);
    item[9].count++;

    usleep(500000);
}

//수호대 구조체 생성
StructOfGuardians* createGuardObject(int** arr){

    StructOfGuardians* guardian = (StructOfGuardians*)malloc(sizeof(StructOfGuardians));
    guardian->guardianXY1.y = arr[1][0];
    guardian->guardianXY1.x = arr[1][1];
    guardian->guardianXY2.y = arr[2][0];
    guardian->guardianXY2.x = arr[2][1];
    guardian->guardianXY3.y = arr[3][0];
    guardian->guardianXY3.x = arr[3][1];

    if (guardian == NULL) {
        printf("메모리 할당 오류\n");
    }

    memoryFreeForGuardians(arr);

    return guardian;
}

//타수호대 좌표를 랜덤으로 갱신해주는 함수
int RandomUpdateGuardiansXY(){
    
    return rand()%20+2;//2~21
}

//던전맵에서 타수호대 좌표를 랜덤으로 갱신해주는 함수
int RandomUpdateGuardiansXYInDeonJeonMap(){
    return rand()%9+6;//6~14
}

//던전 포탈을 한정된 영역 내에서 랜덤한 좌표값 생성하는 함수
int RandomCreateDoorXY(){
    return rand()%20+2;
}

//유저,타수호대 좌표 랜덤 생성 함수
int** RandomCreatePlayer_GuardianXY(){
    //랜덤 좌표 담기 위한 2차원 배열 동적할당
    int**arr = (int **)malloc(4*sizeof(int*));
    for(int i = 0 ; i<4;i++){
        arr[i] = (int*)malloc(2*sizeof(int));
    }

    //유저와 타수호대의 좌표를 담은 2차원 배열
    int randArr[4][2] = {
        {21,11},//8
        {11,2},
        {2,11},
        {11,21}
    };

    //유저와 타수호대의 좌표를 담은 2차원 배열을 랜덤으로 꺼낼 친구
    int randInt[4]={5};
    
        while (1) {
            randInt[0]=rand()%4;
            randInt[1]=rand()%4;
            randInt[2]=rand()%4;
            randInt[3]=rand()%4;
            if(randInt[0]!=randInt[1]&&randInt[0]!=randInt[2]&&randInt[0]!=randInt[3]&&randInt[1]!=randInt[2]&&randInt[1]!=randInt[3]&&randInt[2]!=randInt[3]) break;
        }

    //동적할당된 2D배열을 가리키는 배열 포인터에 무작위 좌표 대입
    for(int i = 0 ; i< 4 ; i++){
        for(int j = 0 ; j<2;j++){
            arr[i][j]=randArr[randInt[i]][j];
        }
    }
    
    return arr;
}

//단축키 설정하는 함수
void changeHotkeys(char* hotkeyArr, int* hotKeyCursor){
    char ch = getch();

    if((*hotKeyCursor)==0){
        hotkeyArr[0]=ch;
    }else if((*hotKeyCursor)==1){
        hotkeyArr[1]=ch;
    }else if((*hotKeyCursor)==2){
        hotkeyArr[2]=ch;
    }else if((*hotKeyCursor)==3){
        hotkeyArr[3]=ch;
    }
}

void dun1(int r, int **ptr, Player* chr, Consumption*item, Equipment*armor, int (*battlemap)[24]) // 15 ~ 18랜덤 배열 던전
{
    int i, j, x1, y1, b = 3, x, y, x2, y2;
    char in;
    int meet;
    int key;
    int map[r][r];
    int l = r - 2;

    srand((int)time(NULL));
    x = rand() % l + 1; // 배열에 맞는 랜덤 출현
    y = rand() % l + 1;
    x1 = rand() % l + 1;
    y1 = rand() % l + 1;
    x2 = rand() % l + 1;
    y2 = rand() % l + 1;

    for (i = 0; i < r; i++) // 움직일공간
    {
        for (j = 0; j < r; j++)
        {
            map[i][j] = 0;
        }
    }
    for (i = 0; i < r; i++) // 벽
    {
        map[0][i] = 1;
        map[i][0] = 1;
        map[r - 1][i] = 1;
        map[i][r - 1] = 1;
    }

    map[x][y] = 2;       // 플레이어 출력
    map[x1][y1] = **ptr; // 보스 출력
    map[x2][y2] = 8;     // 다음 입구 출현
    
    switch(**ptr)
    {
        case 3:
        chr->floor=1;
        break;
        case 4:
        chr->floor=2;
        break;
        case 5:
        chr->floor=3;
        break;
        case 6:
        chr->floor=4;
        break;
        case 7:
        chr->floor=5;
        break;
    }

    while (1)
    {
        system("clear");

        meet = rand() % 10;     // 몬스터와 만날확률
        for (i = 0; i < r; i++) // 사용자위치 찾기
        {
            for (j = 0; j < r; j++)
            {
                if (map[i][j] == 2)
                {
                    x = i;
                    y = j;
                }
            }
        }
        for (i = 0; i < r; i++)
        {
            for (j = 0; j < r; j++)
            {
                if (map[i][j] == 1)
                {
                    printf("\x1b[41m");
                    printf("   ");
                    printf("\x1b[0m");
                }
                if (map[i][j] == 0)
                {
                    printf("   ");
                }
                if (map[i][j] == 3)
                {
                    printf(" 🐗");
                }
                if (map[i][j] == 4)
                {
                    printf(" 🦍");
                }
                if (map[i][j] == 5)
                {
                    printf(" 🐺");
                }
                if (map[i][j] == 6)
                {
                    printf(" 🦈");
                }
                if (map[i][j] == 7)
                {
                    printf(" 🐉");
                }
                if (map[i][j] == 8)
                {
                    printf(" 🪜 ");
                }
                if (map[i][j] == 2)
                {
                    printf("⚪ ");
                }
            }
            printf("\n");
        }
        key = getch(); // 보스방 진행방향 반대로 눌르거나 90도로 눌러야 탈출가능 진행방향 계속 누르면 탈출이 안됌

        if (meet<1) // 수호대 만날 확률 10%
        {
            if(**ptr==3&&chr->lv>=20&&chr->lv<=25)
            Battle(chr, item, armor, battlemap, 2);
            else if(**ptr==4&&chr->lv>=25&&chr->lv<=30)
            Battle(chr, item, armor, battlemap, 2);
            else if(**ptr==5&&chr->lv>=30&&chr->lv<=35)
            Battle(chr, item, armor, battlemap, 2);
            else if(**ptr==6&&chr->lv>=35&&chr->lv<=40)
            Battle(chr, item, armor, battlemap, 2);
            else if(**ptr==7&&chr->lv>=45&&chr->lv<=50)
            Battle(chr, item, armor, battlemap, 2);
        }
        else if(meet>8)
        {
            //Battle(chr, item, armor, battlemap, 1);
        }

        if (key == 'a') // a는 왼쪽방향
        {
            if (map[x][y - 1] != 1)
            {
                map[x][y] = 0;
                map[x][y - 1] = 2;
                if (map[x][y - 1] == map[x1][y1]) // 보스와 조우
                {
                    Battle(chr, item, armor, battlemap, 3);
                    map[x1][y1] = 0;
                    x1 = -2;
                    y1 = -2;
                }
                if (map[x][y - 1] == map[x2][y2])
                {
                    if (x1 == -2)
                    {
                        break;
                    }
                    else
                    {
                        map[x2][y2] = 8;
                    }
                }
            }
        }
        else if (key == 's') // s는 아래방향
        {
            if (map[x + 1][y] != 1)
            {

                map[x][y] = 0;
                map[x + 1][y] = 2;
                if (map[x + 1][y] == map[x1][y1]) // 보스와 조우
                {
                    Battle(chr, item, armor, battlemap, 3);
                    map[x1][y1] = 0;
                    x1 = -2;
                    y1 = -2;
                }
                if (map[x + 1][y] == map[x2][y2])
                {
                    if (x1 == -2)
                    {
                        break;
                    }
                    else
                    {
                        map[x2][y2] = 8;
                    }
                }
            }
        }
        else if (key == 'd') // d는 오른쪽방향
        {
            if (map[x][y + 1] != 1)
            {

                map[x][y] = 0;
                map[x][y + 1] = 2;
                if (map[x][y + 1] == map[x1][y1]) // 보스와 조우
                {
                    Battle(chr, item, armor, battlemap, 3);
                    map[x1][y1] = 0;
                    x1 = -2;
                    y1 = -2;
                }
                if (map[x][y + 1] == map[x2][y2])
                {
                    if (x1 == -2)
                    {
                        break;
                    }
                    else
                    {
                        map[x2][y2] = 8;
                    }
                }
            }
        }
        else if (key == 'w') // w는 위쪽방향
        {
            if (map[x - 1][y] != 1)
            {

                map[x][y] = 0;
                map[x - 1][y] = 2;
                if (map[x - 1][y] == map[x1][y1]) // 보스와 조우
                {
                    Battle(chr, item, armor, battlemap, 3);
                    map[x1][y1] = 0;
                    x1 = -2;
                    y1 = -2;
                }
                if (map[x - 1][y] == map[x2][y2])
                {
                    if (x1 == -2)
                    {
                        break;
                    }
                    else
                    {
                        map[x2][y2] = 8;
                    }
                }
            }
        }
    }
}

void dunp(Player* chr, Consumption*item, Equipment*armor, int (*battlemap)[24])  
{
    int r, i;
    int *ptr;
    int *ptr1;
    int mon = 3;
    int boss = 0;
    ptr = &mon;
    // ptr1 = &boss;
    srand((int)time(NULL));
    for (i = 0; i < 5; i++)
    {
        r = rand() % 4 + 15;
        dun1(r, &ptr, chr, item, armor, battlemap);
        mon++;
    }
}

int main(void)
{
    int map[24][24]={                                        //수호대 출발지
        {7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,6},   //(2,11) (21,11)  (11,2)  (11,21)
        {8,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,8},   //불의구역 (2,2)~(11,11)  -> area=1
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},   //눈의구역 (2,12)~(11,21) -> area=2
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},   //숲의구역 (12,2)~(21,11) -> area=3
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},   //물의구역 (12,12)~(21,21) -> area=4
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},
        {8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,8},
        {8,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,8},
        {7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,6}
    };

     int battlemap[24][24]={
        {1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,2},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8},
        {3,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,4}
    };

    Equipment armor[62] = {{"빛바랜 천두건", "", "", "", "", "", "", 5, 0, 0},
                           {"견고한 천두건", "", "", "", "", "", "", 10, 0, 0},
                           {"빛바랜 가죽투구", "", "", "", "", "", "", 10, 0, 0},
                           {"견고한 가죽투구", "", "", "", "", "", "", 15, 0},
                           {"빛바랜 철투구", "", "", "", "", "", "", 15, 0, 0},
                           {"견고한 철투구", "", "", "", "", "", "", 20, 0, 0},
                           {"", "마모된 천갑옷", "", "", "", "", "", 5, 0, 0},
                           {"", "빛바랜 천갑옷", "", "", "", "", "", 10, 0, 0},
                           {"", "견고한 천갑옷", "", "", "", "", "", 15, 0, 0},
                           {"", "마모된 가죽갑옷", "", "", "", "", "", 5, 0, 0},
                           {"", "빛바랜 가죽갑옷", "", "", "", "", "", 10, 0, 0},
                           {"", "견고한 가죽갑옷", "", "", "", "", "", 15, 0, 0},
                           {"", "마모된 경갑갑옷", "", "", "", "", "", 10, 0, 0},
                           {"", "빛바랜 경갑갑옷", "", "", "", "", "", 15, 0, 0},
                           {"", "견고한 경갑갑옷", "", "", "", "", "", 20, 0, 0},
                           {"", "마모된 중갑갑옷", "", "", "", "", "", 15, 0, 0},
                           {"", "빛바랜 중갑갑옷", "", "", "", "", "", 20, 0, 0},
                           {"", "견고한 중갑갑옷", "", "", "", "", "", 25, 0, 0},
                           {"", "", "빛바랜 천바지", "", "", "", "", 5, 0, 0},
                           {"", "", "견고한 천바지", "", "", "", "", 10, 0, 0},
                           {"", "", "빛바랜 가죽바지", "", "", "", "", 10, 0, 0},
                           {"", "", "견고한 가죽바지", "", "", "", "", 15, 0, 0},
                           {"", "", "빛바랜 경갑바지", "", "", "", "", 15, 0, 0},
                           {"", "", "견고한 경갑바지", "", "", "", "", 20, 0, 0},
                           {"", "", "빛바랜 중갑바지", "", "", "", "", 20, 0, 0},
                           {"", "", "견고한 중갑바지", "", "", "", "", 25, 0, 0},
                           {"", "", "", "마모된 천망토", "", "", "", 5, 0, 0},
                           {"", "", "", "빛바랜 천망토", "", "", "", 10, 0, 0},
                           {"", "", "", "견고한 천망토", "", "", "", 15, 0, 0},
                           {"", "", "", "마모된 가죽망토", "", "", "", 10, 0, 0},
                           {"", "", "", "빛바랜 가죽망토", "", "", "", 15, 0, 0},
                           {"", "", "", "견고한 가죽망토", "", "", "", 20, 0, 0},
                           {"", "", "", "", "마모된 천장갑", "", "", 5, 0, 0},
                           {"", "", "", "", "빛바랜 천장갑", "", "", 10, 0, 0},
                           {"", "", "", "", "견고한 천장갑", "", "", 15, 0, 0},
                           {"", "", "", "", "마모된 가죽장갑", "", "", 10, 0, 0},
                           {"", "", "", "", "빛바랜 가죽장갑", "", "", 15, 0, 0},
                           {"", "", "", "", "견고한 가죽장갑", "", "", 20, 0, 0},
                           {"", "", "", "", "마모된 사슬장갑", "", "", 15, 0, 0},
                           {"", "", "", "", "빛바랜 사슬장갑", "", "", 20, 0, 0},
                           {"", "", "", "", "견고한 사슬장갑", "", "", 25, 0, 0},
                           {"", "", "", "", "", "가죽방패", "", 10, 0, 0},
                           {"", "", "", "", "", "사슬방패", "", 20, 0, 0},
                           {"", "", "", "", "", "철방패", "", 30, 0, 0},
                           {"", "", "", "", "", "", "마모된 한손검", 0, 10, 0},
                           {"", "", "", "", "", "", "빛바랜 한손검", 0, 20, 0},
                           {"", "", "", "", "", "", "견고한 한손검", 0, 30, 0},
                           {"", "", "", "", "", "", "마모된 양손검", 0, 10, 0},
                           {"", "", "", "", "", "", "빛바랜 양손검", 0, 20, 0},
                           {"", "", "", "", "", "", "견고한 양손검", 0, 30, 0},
                           {"", "", "", "", "", "", "마모된 장궁", 0, 10, 0},
                           {"", "", "", "", "", "", "빛바랜 장궁", 0, 20, 0},
                           {"", "", "", "", "", "", "견고한 장궁", 0, 30, 0},
                           {"", "", "", "", "", "", "마모된 숏스태프", 0, 10, 0},
                           {"", "", "", "", "", "", "빛바랜 숏스태프", 0, 20, 0},
                           {"", "", "", "", "", "", "견고한 숏스태프", 0, 30, 0},
                           {"", "", "", "", "", "", "마모된 롱스태프", 0, 10, 0},
                           {"", "", "", "", "", "", "빛바랜 롱스태프", 0, 20, 0},
                           {"", "", "", "", "", "", "견고한 롱스태프", 0, 30, 0},
                           {"", "", "", "", "", "", "마모된 룬스태프", 0, 10, 0},
                           {"", "", "", "", "", "", "빛바랜 룬스태프", 0, 20, 0},
                           {"", "", "", "", "", "", "견고한 룬스태프", 0, 30, 0}};

    Player chr[5];
    Player * pchr=chr;
    Monster mob[5];
    Mobskillstat skill[5];
    Consumption *ptr = (Consumption *)calloc(11, sizeof(Consumption));
    char hotkeyArr[]={'w','s','a','d'};
    int battle_result;  //전투 결과 1.도망 2.승리 3.패배
    int battle_count=0;
    DeonjeonDoorXY door = {RandomCreateDoorXY(),RandomCreateDoorXY()};

    door.x=RandomCreateDoorXY();
    door.y=RandomCreateDoorXY();
    map[door.y][door.x]=10;

    //Opening();
    srand((unsigned int)time(NULL));
    Init_Player(pchr);

    int i;
    i = rand () % 4;   //수호대 출발지 랜덤설정
    if(i==0)
        map[2][11]=5;
    else if(i==1)
        map[21][11]=5;
    else if(i==2)
        map[11][2]=5;
    else if(i==3)
        map[11][21]=5; 

    int** arr = MonsterAppearanceProbability();

    while(1)
    {
        RandNumIntoMonsterAppearanceProbability(arr);
        system("clear");
        Map(map, chr, ptr);
        battle_count+=Move(map, hotkeyArr, armor, chr, arr, ptr, battlemap);
        if(battle_count==11)
        {
            battle_count=0;
            map[door.y][door.x]=0;
            while(1)
            {
                door.x=RandomCreateDoorXY();
                door.y=RandomCreateDoorXY();
                if(map[door.y][door.x]!=0)
                continue;
                else
                {
                    map[door.y][door.x]=10;
                    break;
                }
            }
        }
    }
    free(ptr);
    
    return 0;
}