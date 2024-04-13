/*
* ������־
* 1.ʹ��vs2022������Ŀ
* 2.�����ز�
* 3.��ʼ����
* 4.ʵ����Ϸ����������
* 5.ʵ��ֲ�����Ŀ���
*
*
*/
#include <iostream>
#include <stdio.h>
#include <graphics.h> //easyxͼ�ο�
#include <time.h>
#include <mmsystem.h>
#include <math.h>
#include <thread>

#include "vector2.h"
#include "tools.h"
using namespace std;
#pragma comment(lib, "winmm.lib")

#define WIN_WIDTH 900
#define WIN_HEIGHT 600
#define ZM_MAX 20
#define _CRT_SECURE_NO_WARNINGS
enum { WAN_DOU, XIANG_RI_KUI, PLANTE_COUNT };
IMAGE imgBg;//����ͼƬ,
IMAGE imgbar;
IMAGE imgplantCard[PLANTE_COUNT];
IMAGE* imgPlant[PLANTE_COUNT][20];//ֲ���֡��
IMAGE imgshovel;
IMAGE imgshovelSlot;
IMAGE imgPause;
IMAGE imgMenu;
IMAGE imgSelect;
IMAGE imgSelect1;
IMAGE imgSelect2;
IMAGE imgbegin[3];
int shovelX, shovelY;
int curShovel;
// ����ÿ��ֲ�����ȴʱ��
const int coolDownTimePlant1 = 5; // ��һ��ֲ����ȴʱ��Ϊ5��
const int coolDownTimePlant2 = 3; // �ڶ���ֲ����ȴʱ��Ϊ3��
// ����������洢ÿ��ֲ���ϴ���ֲ��ʱ���
std::chrono::time_point<std::chrono::steady_clock> lastPlantTimePlant1;
std::chrono::time_point<std::chrono::steady_clock> lastPlantTimePlant2;

int curX, curY;//��ǰѡ��ֲ�����ƶ������е�λ��
int curPlante; // 0:û��ѡ�У�1��ѡ���һ��ֲ��
int mouseX, mouseY;//��ʼ��������

enum { GOING, WIN, FAIL };
int killCount;//�Ѿ�ɱ���Ľ�ʬ
int zmCount;//�Ѿ����ֵĽ�ʬ
int gameStatus;


struct shovel {
	int type;
	int x, y;
};

struct shovel maps[3][9];

struct plant {
	int type;		// 0����ʾû��ֲ�� 1����һ��ֲ��
	int frameIndex; //����֡���
	bool catched;//�Ƿ񱻽�ʬ����
	int blood;//����������

	int timer;
	int x, y;
	int timers;//��ȴ
	int shootTime;
};

struct plant map[3][9];

enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };

struct sunshineBall {
	int x, y;
	int frameIndex; //��ǰͼƬ֡�����
	int destY;//Ʈ���Ŀ��λ��y����
	bool used;//�Ƿ���ʹ��
	int timer;

	float xoff;
	float yoff;

	float t;//����������ʱ���
	vector2 p1, p2, p3, p4;
	vector2 pCur;//��ǰʱ���������λ��
	float speed;
	int status;
};

struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];
int sunshine;

struct zombie {
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;
	bool dead;
	bool eating;
};

struct zombie zombienum[10];//��һ��

IMAGE imgZombieWalk[22];
IMAGE imgZombieDead[10];
IMAGE imgZombieEat[21];
IMAGE imgZombieStand[11];
bool isPaused = false;
bool isexits = false;
bool isresume = false;

//�ӵ�������
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//�Ƿ�����ײ
	int frameIndex;//֡���
};

struct bullet bullets[40];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];

int zCount = sizeof(zombienum) / sizeof(zombienum[0]);
int ballmax = sizeof(balls) / sizeof(balls[0]);


class music {
public:
	void BulletbeatZombie() {
		PlaySound("res/audio/splat1.wav", NULL, SND_FILENAME | SND_ASYNC);
	}
	void SelectPlant() {
		PlaySound("res/audio/seedlift.wav", NULL, SND_FILENAME | SND_ASYNC);
	}
	void PutPlant() {
		PlaySound("res/audio/plant2.wav", NULL, SND_FILENAME | SND_ASYNC);
	}
	void SelectShovel() {
		PlaySound("res/audio/shovel.wav", NULL, SND_FILENAME | SND_ASYNC);
	}
	void chomp() {
		int row;
		for (int i = 0; i < zCount; i++) {
			row = zombienum[i].row;
			if (zombienum[i].dead) mciSendString("close res/audio/chomp.mp3", 0, 0, 0);
		}
		for (int k = 0; k < 9; k++) {
			if (map[row][k].blood > 150) {
				mciSendString("close res/audio/chomp.mp3", 0, 0, 0);
				mciSendString("play res/audio/swallow.mp3", 0, 0, 0);
			}
		}
	}
};

bool fileExist(const char* name) {
	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}
//��ʼ����Ϸ
void gameInit() {
	//���ر���ͼƬ��ֱ�ӵ��ñȽ���,���ص��ڴ����档
	//����ִ��󣬽��ַ����޸�Ϊ���ֽ��ַ���
	loadimage(&imgBg, "res/bg.jpg");//����ͼƬ
	loadimage(&imgbar, "res/bar5.png");//��Ƭ��
	loadimage(&imgshovel, "res/shovel.png");//����
	loadimage(&imgshovelSlot, "res/shovelSlot.png");//���Ӹ�
	loadimage(&imgPause, "res/pause.png");//��ͣͼ��
	loadimage(&imgMenu, "res/selectmenu.png");
	loadimage(&imgSelect, "res/select.png");
	loadimage(&imgSelect1, "res/select.png", 
	imgSelect.getwidth() * 0.5,
	imgSelect.getheight() * 0.5,true);
	memset(imgPlant, 0, sizeof(imgPlant));
	memset(map, 0, sizeof(map));
	memset(map, 0, sizeof(maps));

	killCount = 0;
	zmCount = 0;
	gameStatus = GOING;

	char name[64];//����ʾ�ļ���

	for (int i = 0; i < 3; i++) {
		sprintf_s(name, sizeof(name), "res/begin_%d.png", i + 1);
		loadimage(&imgbegin[i], name);
	}

	for (int i = 0; i < PLANTE_COUNT; i++) {
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgplantCard[i], name);

		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			//���ж�����ļ��Ƿ����
			if (fileExist(name)) {
				imgPlant[i][j] = new IMAGE;//�����ڴ棬��ֹ����
				loadimage(imgPlant[i][j], name);
			}
			else {
				break;
			}
		}

	};//����ֲ����ļ���
	curPlante = 0;
	sunshine = 50;//�����ʼֵ

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//�����������
	srand(time(NULL));

	//������Ϸͼ�δ���
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	//��ʼ����ͨ��ʬ����
	memset(zombienum, 0, sizeof(zombienum));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
		loadimage(&imgZombieWalk[i], name);
	}

	//��ʼ����ʬ��ֲ��
	for (int i = 0; i < 21; i++) {
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZombieEat[i], name);
	}

	//��ʼ����ͨ��ʬ����������
	for (int i = 0; i < 10; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZombieDead[i], name);
	}

	//��ʼ���㶹�ӵ�
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//��ʼ���㶹�ӵ���ײ֡ͼƬ����
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
			imgBulletBlast[3].getwidth() * k,
			imgBulletBlast[3].getheight() * k, true);
	}

	for (int i = 0; i < 11; i++) {
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZombieStand[i], name);
	}

}

void drawdragshovel() {
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 9; j++) {
			if (maps[i][j].type > 0) {
				putimagePNG(maps[i][j].x, maps[i][j].y, &imgshovel);
			}
		}
	if (curShovel) {
		IMAGE img = imgshovel;
		putimagePNG(shovelX - img.getwidth() / 2, shovelY - img.getheight() / 2, &img);
	}
}

// �������ڼ��ֲ���Ƿ�����ȴ��
bool isCoolingDown(int plantType) {
	auto currentTime = std::chrono::steady_clock::now();
	if (plantType == 1) {
		return std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastPlantTimePlant1).count() < coolDownTimePlant1;
	}
	else if (plantType == 2) {
		return std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastPlantTimePlant2).count() < coolDownTimePlant2;
	}
	return false;
}

void drawCard() {
	for (INT i = 0; i < PLANTE_COUNT; i++) {
		int x = 226 + i * 66;
		int y = 6;
		static int j = 0;
		auto currentTime = std::chrono::steady_clock::now();
		IMAGE *img = &imgplantCard[i];
		int cardWidth =img->getwidth() ; // ���ƵĿ��
		int cardHeight = img->getheight(); // ���Ƶĸ߶�
		if (isCoolingDown(i+1)) {
			auto currentTime = std::chrono::steady_clock::now();
			if (i + 1 == 1) {
				auto elapsedTime =	std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastPlantTimePlant1).count();
				j = coolDownTimePlant1 - elapsedTime;
							
			}
			else if (i + 1 == 2) {
				auto elapsedTime =	std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastPlantTimePlant2).count();
				j = coolDownTimePlant2 - elapsedTime;
			
			}


			float progress = static_cast<float>(j) / ((i == 0) ? coolDownTimePlant1 : coolDownTimePlant2); // ��ȡ��ȴ����
			int maskedHeight = static_cast<int>(cardHeight * (1 - progress)); // �������ָ߶�
			int visibleHeight = maskedHeight - cardHeight ; // ����ɼ��߶�
			putimage(x, y, &imgplantCard[i]); // �Ȼ��ƿ���
			setfillcolor(LIGHTGRAY); // ����������ɫΪ��ɫ
			solidrectangle(x + cardWidth -2, y -2 +cardHeight - maskedHeight, x , y ); // ���ƽ�����
			
		}else putimage(x, y, &imgplantCard[i]);
		
	}
}

void createPlante() {
	static int count = 0;
	if (++count < 3)return;
	count = 0;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				map[i][j].frameIndex++;
				int plantType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgPlant[plantType][index] == NULL) {
					map[i][j].frameIndex = 0;
				}
			}
		}
}

void drawdragPlante() {
	//��Ⱦ�϶�ʱ��ֲ��


	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				//int x = 256 + j * 81;
				//int y = 179 + i * 102;
				int planttype = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				//putimagePNG(x, y, imgPlant[planttype][index]);
				putimagePNG(map[i][j].x, map[i][j].y, imgPlant[planttype][index]);
			}
		}
	if (curPlante == 1 && sunshine >= 100) {
		IMAGE* img = imgPlant[curPlante - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}
	else if (curPlante == 2 && sunshine >= 50) {
		IMAGE* img = imgPlant[curPlante - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}
}

//�ռ�����
void collectSunshine(ExMessage* msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int width = imgSunshineBall[0].getwidth();
	int heigth = imgSunshineBall[0].getheight();
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			//int x = balls[i].x;
			//int y = balls[i].y;
			int x = balls[i].pCur.x;
			int y = balls[i].pCur.y;
			if (msg->x > x && msg->x < x + width
				&& msg->y >y && msg->y < y + heigth) {
				//balls[i].used = false;
				balls[i].status = SUNSHINE_COLLECT;
				//sunshine += 50;
				//mciSendString("play res/sunshine.mp3", 0, 0, 0);
				PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC);
				//�����������ƫ����
				//float destY = 0;
				//float destX = 262;
				//float angle = atan(y - destY) / (x - destX);
				//balls[i].xoff = 4 * cos(angle);
				//balls[i].yoff = 4 * sin(angle);
				balls[i].p1 = balls[i].pCur;//���
				balls[i].p4 = vector2(140, 2);//�յ�
				balls[i].t = 0;
				float off = 8;
				float distance = dis(balls[i].p1 - balls[i].p4);//��������
				balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
}

void createSunshine() {
	static int count = 0;
	static int fre = 100;
	count++;
	if (count >= fre) {
		fre = 200 + rand() % 400;
		count = 0;
		int ballmax = sizeof(balls) / sizeof(balls[0]);

		int i;
		for (i = 0; i < ballmax && balls[i].used; i++);
		if (i >= ballmax) return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		//balls[i].x = 260 + rand() % (900 - 260);
		//balls[i].y = 60;
		//balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;
		//balls[i].xoff = 0;
		//balls[i].yoff = 0;
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(148 + rand() % (900 - 148), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}

	//���տ���������
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == XIANG_RI_KUI + 1) {
				map[i][j].timer++;
				if (map[i][j].timer > 500) {
					map[i][j].timer = 0;
					int k;
					for (k = 0; k < ballmax && balls[k].used; k++);
					if (k >= ballmax) return;

					balls[k].used = true;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);
					int w = (10 + rand() % 51) * (rand() % 2 ? 1 : -1);
					int p4_y = map[i][j].y + imgPlant[XIANG_RI_KUI][0]->getheight() - imgSunshineBall->getheight() + 20;
					balls[k].p4 = vector2(map[i][j].x + w, p4_y);
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 60);
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;
					balls[k].t = 0;
				}
			}
		}
	}
}

void updateSunshine() {
	int ballmax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballmax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;//��ֹԽ�磬����ʹӵ�һ���ٿ�ʼ
			if (balls[i].status == SUNSHINE_DOWN) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);//һ�α���������
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 100) {
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
		}
	}
}

void drawSunshine() {
	int ballmax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballmax; i++) {
		//if (balls[i].used || balls[i].xoff) {
		if (balls[i].used) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			//putimagePNG(balls[i].x, balls[i].y, img);
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}
	//��������
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;//�����Ч��
	settextstyle(&f);
	setbkmode(TRANSPARENT);// ���屳��͸��
	setcolor(BLACK);
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(164, 67, scoreText);//�������
}

void createZombie() {
	if (zmCount >= ZM_MAX) {
		return;
	}


	static int ZmFre = 105;
	static int count = 0;
	count++;
	if (count > ZmFre) {
		if (ZmFre == 105) {
			mciSendString("play res/audio/abegin.mp3 ", 0, 0, 0);

		}
		count = 0;
		if (zmCount >= 10) {
			ZmFre = rand() % 200 + 400;
		}
		else {
			ZmFre = rand() % 200 + 800;
		}
		
		int i;
		int zmMax = sizeof(zombienum) / sizeof(zombienum[0]);
		for (i = 0; i < zmMax && zombienum[i].used; i++);
		if (i < zmMax) {
			memset(&zombienum[i], 0, sizeof(zombienum[i]));
			zombienum[i].used = true;
			zombienum[i].x = WIN_WIDTH;
			zombienum[i].row = rand() % 3;
			zombienum[i].y = 172 + (1 + zombienum[i].row) * 100;
			zombienum[i].speed = 2;
			zombienum[i].blood = 1000;
			zombienum[i].dead = false;
			zmCount++;
		}

	}
}

void updateZombie() {
	int zmMax = sizeof(zombienum) / sizeof(zombienum[0]);

	static int count = 0;
	count++;
	if (count > 6) {
		count = 0;
		//���½�ʬ��λ��
		for (int i = 0; i < zmMax; i++) {
			if (zombienum[i].used) {
				zombienum[i].x -= zombienum[i].speed;
				if (zombienum[i].x < 80) {
					//printf("Zombies ate your brains\n");
					//MessageBox(NULL, "over", "over", 0);//���Ż�
					//exit(0);//���Ż�
					gameStatus = FAIL;
				}
			}
		}
	}
	static int count2 = 0;
	count2++;
	if (count2 > 3) {
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zombienum[i].used) {
				if (zombienum[i].dead) {
					zombienum[i].frameIndex++;
					if (zombienum[i].frameIndex >= 10) {
						zombienum[i].used = false;
						killCount++;
						if (killCount == ZM_MAX) {
							gameStatus = WIN;
						}
					}
				}
				else if (zombienum[i].eating) {
					zombienum[i].frameIndex = (zombienum[i].frameIndex + 1) % 21;
				}
				else {
					zombienum[i].frameIndex = (zombienum[i].frameIndex + 1) % 22;
				}

			}
		}
	}

}

void drawZombie() {
	int zmCount = sizeof(zombienum) / sizeof(zombienum[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zombienum[i].used) {
			//IMAGE* img = &imgZombieWalk[zombienum[i].frameIndex];
			//IMAGE* img = (zombienum[i].dead) ? imgZombieDead : imgZombieWalk;
			IMAGE* img = NULL;
			if (zombienum[i].dead) img = imgZombieDead;
			else if (zombienum[i].eating) img = imgZombieEat;
			else img = imgZombieWalk;

			img += zombienum[i].frameIndex;

			putimagePNG(zombienum[i].x, zombienum[i].y - img->getheight(), img);
		}
	}
}

void shoot() {
	static int count = 0;
	if (++count < 5)return;
	count = 0;
	int lines[3] = { 0 };
	int zmCount = sizeof(zombienum) / sizeof(zombienum[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - 40;
	for (int i = 0; i < zmCount; i++) {
		if (zombienum[i].used && zombienum[i].x < dangerX) {
			lines[zombienum[i].row] = 1;//�ж���һ���Ƿ��н�ʬ
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == WAN_DOU + 1 && lines[i]) {
				//static int count1 = 0;
				//count1++;
				map[i][j].shootTime++;
				if (map[i][j].shootTime > 20) {
					map[i][j].shootTime = 0;
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);//shenhoupanding
					if (k < bulletMax) {
						int zwX = 144 + j * 81;
						int zwY = 179 + i * 102 + 14;

						for (int n = 0; n < zmCount; n++) {
							if (zombienum[n].used && zombienum[n].x > zwX && zombienum[n].row == i) {
								bullets[k].used = true;
								bullets[k].row = i;
								bullets[k].speed = 12;

								bullets[k].blast = false;
								bullets[k].frameIndex = 0;

								lines[i] = 0;
							}
							bullets[k].x = zwX + imgPlant[map[i][j].type - 1][0]->getwidth() - 10;
							bullets[k].y = zwY + 5;
						}

					}
				}
			}
		}
	}

}

void updatebullet() {
	static int count = 0;
	if (++count < 3)return;
	count = 0;
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}
			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;

				}
			}
		}
	}
}

void drawbullet() {
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else {
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}

	}
}

void checkBulletZombie() {
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zCount = sizeof(zombienum) / sizeof(zombienum[0]);
	music t;
	for (int i = 0; i < bCount; i++) {
		if (bullets[i].used == false || bullets[i].blast)continue;
		for (int k = 0; k < zCount; k++) {
			//if (zombienum[i].used == false)continue;
			if (zombienum[k].used == false)continue;
			int x1 = zombienum[k].x + 80;
			int x2 = zombienum[k].x + 110;
			int x = bullets[i].x;
			if (zombienum[k].dead == false && bullets[i].row == zombienum[k].row && x > x1 && x < x2) {
				thread thd(&music::BulletbeatZombie, &t);
				zombienum[k].blood -= 100;
				bullets[i].blast = true;
				bullets[i].speed = 0;
				thd.detach();
				if (zombienum[k].blood <= 0) {
					zombienum[k].dead = true;
					zombienum[k].speed = 0;
					zombienum[k].frameIndex = 0; 
					mciSendString("close res/audio/chomp.mp3", 0, 0, 0);
				}
				break;
			}
		}
	}
}

void checkZombiePlant() {
	music T, t;
	for (int i = 0; i < zCount; i++) {
		if (zombienum[i].dead)  continue; 
		int row = zombienum[i].row;
		for (int k = 0; k < 9; k++) {
			if (map[row][k].type == 0)continue;
			int ZHIWUX = 144 + k * 81;
			int x1 = ZHIWUX + 10;
			int x2 = ZHIWUX + 60;
			int x3 = zombienum[i].x + 80;
			if (x3 > x1 && x3 < x2) {
				if (map[row][k].catched) {
					//zombienum[i].frameIndex++;
					map[row][k].blood++;
					//if (zombienum[i].frameIndex > 100)
					if (map[row][k].blood > 150) {
						map[row][k].blood = 0;
						map[row][k].type = 0;
						zombienum[i].eating = false;
						map[row][k].catched = false;
						zombienum[i].frameIndex = 0;
						zombienum[i].speed = 2;
						mciSendString("close res/audio/chomp.mp3", 0, 0, 0);
						mciSendString("play res/audio/swallow.mp3", 0, 0, 0);
					}
				}
				else {
					mciSendString("play res/audio/chomp.mp3 repeat", 0, 0, 0);
					map[row][k].catched = true;
					map[row][k].blood = 0;
					zombienum[i].eating = true;
					zombienum[i].speed = 0;
					zombienum[i].frameIndex = 0;
				}

			}
		}
	}
}

void collisionCheck() {
	checkBulletZombie();//�ӵ��Խ�ʬ����ײ���
	checkZombiePlant();//��ʬ�Ƿ�����ֲ��
}//��ײ���

void showSettingsMenu() {
	LOGFONT s;
	gettextstyle(&s);
	s.lfHeight = 40;
	s.lfWeight = 35;
	strcpy(s.lfFaceName, "Segoe UI Black");
	s.lfQuality = ANTIALIASED_QUALITY;//�����Ч��
	settextstyle(&s);
	setbkmode(TRANSPARENT);// ���屳��͸��
	setcolor(BLACK);
	BeginBatchDraw();
	putimagePNG(244, 59, &imgMenu);
	putimagePNG(364, 308, &imgSelect1);
	outtextxy(384, 308, "�˳���Ϸ");
	putimagePNG(364, 376, &imgSelect1);
	outtextxy(384, 376, "���¿�ʼ");
		// ����Ϸ��ͣʱ��ʾ������Ϸ��ť
	putimagePNG(279, 449, &imgSelect);
	outtextxy(389, 469, "������Ϸ");
	EndBatchDraw();
}

void userClick() {
	
	ExMessage msg;//�ж��Ƿ��в�����Ϣ
	static int status = 0;
	int w = 0;;
	music T, t;
	if (peekmessage(&msg)) {
		auto currentTime = std::chrono::steady_clock::now();
		if (msg.message == WM_LBUTTONDOWN && sunshine >= 0) {
			if (msg.x >830  && msg.x <900  && msg.y <96 ) {
				// �û���������ð�ť����ʾ���ò˵�
				isPaused = !isPaused;
			}
			else if (msg.x > 279 && msg.x < 279 + imgSelect.getwidth() 
				&& msg.y > 449 && msg.y < 449 + imgSelect.getheight() && isPaused) {
				// �û�����˷�����Ϸ��ť������Ϸ״̬��Ϊ����ͣ
				isPaused = false;
			}
			else if (msg.x > 364 && msg.x < 364 + imgSelect.getwidth()
				&& msg.y > 308 && msg.y < 308 + imgSelect.getheight() && isPaused) {
				// �û�������˳���Ϸ��ť������Ϸ�˳�
				isexits = true;
			}
			else if (msg.x > 364 && msg.x < 364 + imgSelect.getwidth()
				&& msg.y > 376 && msg.y < 376 + imgSelect.getheight() && isPaused) {
				// �û�����˷�����Ϸ��ť������Ϸ״̬��������
				isresume = true;
				mciSendString("close res/audio/Ultimate_Battle.mp3", 0, 0, 0);
			}
			 if (msg.x > 338 - 112 && msg.x < 338 - 112 + 65 * PLANTE_COUNT && msg.y < 96) {

				int index = (msg.x - 338 + 112) / 65;
				curShovel = 0;
				curPlante = index + 1;
				if (isCoolingDown(curPlante)) { // ��������ֲ���Ƿ�����ȴ��
					// �������ȴ�У�����ѡ����ʾ��ʾ��Ϣ����ֱ�Ӻ��Ըôε��
					// �������ʹ������Ϸ�������ʾ��������ʾ��ʾ��Ϣ�����絯����ʾ������ڽ�������ʾ������ʾ
					// ��ʾ��ȴ�е���ʾ��Ϣ
					curX = -100;
					curY = -100;
					return; // ��ִ�к�������ֲ����
				}
				if (curPlante == 1 && sunshine >= 100 ) {
					thread thd(&music::SelectPlant, &T);
					status = 1;
					thd.detach();
					curX = msg.x;
					curY = msg.y;
				}
				else if (curPlante == 2 && sunshine >= 50) {
					thread thd(&music::SelectPlant, &T);
					status = 1;
					thd.detach();
					curX = msg.x;
					curY = msg.y;
				}

			}
			else if (msg.x > 750 && msg.x < 750 + 68 && msg.y < 96) {
				
				
				curShovel = 1;
				thread thd(&music::SelectShovel, &T);
				status = 1;
				thd.detach();
				shovelX = msg.x;
				shovelY = msg.y;
			}
			else {
				collectSunshine(&msg);
				curShovel = 0;
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {
			if (curShovel) {
				shovelX = msg.x;
				shovelY = msg.y;
			}
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP && status == 1) {
			if (msg.x > 144 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 144) / 81;
				if (curShovel && map[row][col].type) {
					for (int i = 0; i < zCount; i++) {
						map[row][col].blood = 0;
						map[row][col].type = 0;
						curShovel = 0;
						mciSendString("close res/audio/chomp.mp3", 0, 0, 0);
						mciSendString("play res/shove.mp3", 0, 0, 0);
						//PlaySound("res/shove.wav", NULL, SND_FILENAME | SND_ASYNC);
					}

				}
				else {
					curShovel = 0;
				}
				if (map[row][col].type == 0) {
					map[row][col].type = curPlante;
					map[row][col].frameIndex = 0;
					map[row][col].shootTime = 0;
					//int x = 256 + j * 81;
					//int y = 179 + i * 102;
					map[row][col].x = 144 + col * 81;
					map[row][col].y = 179 + row * 102;
					if (map[row][col].type == 1 && status == 1) {
						sunshine -= 100;
						lastPlantTimePlant1 = std::chrono::steady_clock::now(); // ���·���ֲ��1��ʱ��
					}
					if (map[row][col].type == 2 && status == 1) {
						sunshine -= 50;
						lastPlantTimePlant2 = std::chrono::steady_clock::now();
					}
					if (map[row][col].type) {
						thread thd(&music::PutPlant, &t);
						thd.detach();
					}

				}

			}//ǰ������ֲ��Χ
			curShovel = 0;
			curPlante = 0;
			status = 0;
		}

	}
}

void updateGame() {
	createPlante();

	createSunshine();//��������
	updateSunshine();//���������״̬

	createZombie();//���콩ʬ
	updateZombie();//���½�ʬ״̬

	shoot();//�����㶹
	updatebullet();//�����㶹�ӵ�

	collisionCheck();//�ӵ���ײ���
}

//������Ϸ����
void updateWindow() {
	BeginBatchDraw();//���壬��ֹһֱˢ��
	if (isPaused) {
		showSettingsMenu();
		return;
	}
	else {
	putimage(-112, 0, &imgBg);
	//putimage(250, 0, &imgbar);
	putimagePNG(138, 0, &imgbar);
	putimagePNG(750, 0, &imgshovelSlot);
	putimagePNG(750, 0, &imgshovel);
	putimagePNG(830, 0, &imgPause);

	

	drawCard();

	drawdragPlante();

	drawdragshovel();

	drawSunshine();

	drawZombie();
	drawbullet();
	}
	


	EndBatchDraw();//����˫����
}

void startUI() {
	mciSendString("play res/bg.mp3", 0, 0, 0);
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;
	while (1) {
		BeginBatchDraw();//���壬��ֹһֱˢ��
		putimage(0, 0, &imgBg);
		if (flag == 1) {
			putimagePNG(474, 75, &imgMenu2);
		}
		else {
			putimagePNG(474, 75, &imgMenu1);
		}

		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN
				&& msg.x > 474 && msg.x < 474 + 300
				&& msg.y >75 && msg.y < 75 + 140) {
				flag = 1;
			}
			else if (msg.message == WM_MOUSEMOVE && flag) {
				mouseX = msg.x;
				mouseY = msg.y;
			}
			else if (msg.message = WM_LBUTTONUP && flag) {
				if (msg.x > 474 && msg.x < 474 + 300
					&& msg.y > 75 && msg.y < 75 + 140) {
					mciSendString("close res/bg.mp3", 0, 0, 0);
				}
				EndBatchDraw();
				break;
			}
		}
		EndBatchDraw();
	}
}

void viewScence() {
	PlaySound("res/Look up at the Sky.wav", NULL, SND_FILENAME | SND_ASYNC);
	int xMin = WIN_WIDTH - imgBg.getwidth();
	vector2 points[9] = {
		{550,80},{530,160},{630,170},
		{530,200},{515,270},{565,370},
		{605,340},{705,280},{690,340}
	};
	int index[9];
	for (int i = 0; i < 9; i++) {
		index[i] = rand() % 11;
	}
	int count = 0;
	for (int x = 0; x >= xMin; x -= 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		count++;
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZombieStand[index[k]]);

			if (count >= 50) {
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10)count = 0;
		EndBatchDraw();
		Sleep(10);
	}
	//ͣ��2��
	for (int i = 0; i < 100; i++) {
		BeginBatchDraw();
		putimage(xMin, 0, &imgBg);
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x, points[k].y, &imgZombieStand[index[k]]);
			index[k] = (index[k] + 1) % 11;
		}

		EndBatchDraw();
		Sleep(30);
	}
	for (int x = xMin; x <= -112; x += 2) {
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		count++;
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZombieStand[index[k]]);

			if (count >= 50) {
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10)count = 0;
		EndBatchDraw();
		Sleep(10);
	}

}

void barsDown() {
	mciSendString("play res/audio/Ultimate_Battle.mp3 repeat", 0, 0, 0);
	int height = imgbar.getheight();
	for (int k = -height; k <= 0; k++) {
		BeginBatchDraw();


		putimage(-112, 0, &imgBg);
		putimagePNG(138, k, &imgbar);
		putimagePNG(750, k, &imgshovelSlot);
		putimagePNG(750, k, &imgshovel);

		for (int i = 0; i < PLANTE_COUNT; i++) {
			int x = 338 - 112 + i * 66;
			int y = 6 + k;
			putimage(x, y, &imgplantCard[i]);
		}

		EndBatchDraw();
		Sleep(10);
	}
}

bool checkover() {
	int xMin = WIN_WIDTH - imgBg.getwidth();
	int count = 0;
	int ret = false;
	if (gameStatus == WIN) {
		Sleep(2000);
		loadimage(0, "res/win2.png");
		mciSendString("play res/win.mp3", 0, 0, 0);
		ret = true;
	}
	else if (gameStatus == FAIL) {
		Sleep(2000);
		loadimage(0, "res/fail.png");
		for (int x = -112; x <= 0; x += 2) {
			BeginBatchDraw();
			putimage(x, 0, &imgBg);
			count++;
			if (count >= 10)count = 0;
			EndBatchDraw();
			Sleep(10);
		}
		PlaySound("res/lose.wav", NULL, SND_FILENAME | SND_SYNC);
		PlaySound("res/scream.wav", NULL, SND_FILENAME | SND_ASYNC);
		loadimage(0, "res/ZombiesWon.png");
		ret = true;
	}
	return ret;
}


void begin() {
	PlaySound("res/readysetplant.wav", NULL, SND_FILENAME | SND_ASYNC);
	for (int i = 0; i < 3; i++) {
	IMAGE* img = &imgbegin[i];
	putimagePNG(323, 252, img);
	Sleep(400);
	if (i < 2) {
		IMAGE* prev_img = &imgBg;
		putimagePNG(-112, 0, prev_img);
	}
	}
	Sleep(500);
}

void beagin2() {

}

void beagin3() {

}

int main(void) {
	HWND hwnd = GetConsoleWindow();
	if (hwnd != NULL) {
		ShowWindow(hwnd, SW_HIDE); // ���ؿ���̨����
	}

	gameInit();//��ʼ����Ϸ

	startUI();//��ʼ�˵�

	viewScence();
	begin();
	barsDown();
	int timer1 = 0;
	bool flag1 = true;
	while (1) {
		userClick();
		if (isresume) {
			gameInit();
			viewScence();
			begin();
			barsDown();
			isresume = false;
			isPaused = false;
		}
		if (isPaused) {
			showSettingsMenu();
		}
		if (isPaused == false) {
		timer1 += getDelay();
		updateWindow();//������Ϸ����,��Ⱦ����	
		if (timer1 > 15) {
			flag1 = true;
			timer1 = 0;
		}
		if (flag1) {
			flag1 = false;
			updateGame();
			if (checkover())exit(0);
		}
		
		}
		if (isexits) exit(0);
		
		
	}

	system("pause");
	return 0;
}