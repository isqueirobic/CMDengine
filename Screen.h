#pragma once
#include <iostream>
#include <Windows.h>
#include <fstream>
#include "Positions.h"
#include <vector>
#include <list>
#include <chrono>
#include <utility>
#include <utility>
#include <queue>
#include <unordered_map>
#include <functional>

using namespace position;

class MainScreen {
private:
	std::chrono::steady_clock::time_point last = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();
public:
	int Width, Height, AbsoluteSize, ScreenAspectRatio;
	char* Screen;
	float* Zbuffer;
	HANDLE buff = GetStdHandle(STD_OUTPUT_HANDLE);
	HWND ConsoleW = GetConsoleWindow();
	COORD cood = { 0,0 };
	DWORD charswriten;
	DWORD lenght;
	RECT r;
	bool IsTargetWindow = false;
	HWND foregroundWindow;
	CONSOLE_SCREEN_BUFFER_INFO bfinfo;
	CONSOLE_FONT_INFOEX font;
	float deltatime = 0.f;
	MainScreen(int width, int hight, int PixelWidth, int PixelHeight);
	void CleanBuffer();
	void DrawBuffer();
	Position ToScreen(Position tscp); // it normals the position to the screen
};

template <typename thing>
int IsIn(thing shearshfor, std::vector<thing> thingV) {
	int i = 0;
	for (auto c : thingV) {
		if (shearshfor == c) { return i; }
		i++;
	}
}

class Texture {
	Position TextureRealSize;
	void Prep(Position& p, Position& size);
public:
	MainScreen* screen;
	std::string Texture_;
	Position p;
	Position size;
	Position lookV = { 1.00000000000001f,0.000000000000001f,0.00000000001f };
	Position RightV = { 0.0000000000000001f,0.0000000000000000001,1.00000000001f };
	Position UpV = { 0.0000000000001f,1.000000000000001f,0.0000000000000001f };
	Position Angles = { 1.000000000000001f,90.000000000000001f,1.00000000000001f };
	bool TextureIsSet = false;
	bool TextureIsLoading = false;
	bool CanDraw = true;
	std::string Myname = "Texture";
	float NearZ = 0.1f;
	char DefaultText = '@';

	void LoadTexture(const char* FileName, int FileszX, int FileszY);

	void LoadTexture(std::string text, int textX, int textY);

	void RenderTexture();

	void RenderTexture(Position p);

	Texture();

	Texture(Position position, Position size, MainScreen& Screen);

	void Set(MainScreen& Screen);

	void Draw();

	void Draw(Position p);

	void Update();

	void Update(Position p);

	bool operator==(const Texture& another) const {
		return another.Myname.compare(Myname);
	}

};

class Words {
public:
	Position p;
	Position size;
	Position TrueFontSize;
	Position CharSpacing;
	char* FontTexture;
	char* message;
	int TotalSize;
	int  NumLaters;
	int* NumLatersPos;
	char Suported[38] = "abcdefghijklmnopqrstuvwxyz1234567890";
	int NumSuported = sizeof(Suported);
	int oi[100];
	MainScreen* screen;
	Words(const char* FontPath, Position FontXY, MainScreen& sc, int Yabsolute);

	void SetString(char* m);
	void SetString(std::string m);
	void RenderWords(int CharPosIndex, Position offset);


	void WriteWords();

};

class Light {

	char LightTbl[70] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,¨^`' ";
	int CharLightLenght = 0;

	Texture* rect;
	MainScreen* screen;
	float light = 1;
public:
	int intenct = 0.2;
	Position p;
	Position Siz;
	Light(MainScreen& sc, float _magnetude, Position pos);

	void ApplyLightOn(Texture& _Rect);
};

class KeyBoard {
	MainScreen* Screen;
	bool DoneOnce = false;
public:
	bool Keys[256];
	KeyBoard(MainScreen& sc);
	void UpdateKeys();

};

class Twennumber {
	float fin;
	float* beguin;
	float time;
	float timePoll;
	float* deltat;
	float speed;
	bool sing = false;
	bool update = false;
public:
	bool playing = false;
	Twennumber(float& deltaT);

	Twennumber();

	void Setdeltatime(float& deltaT);

	void CreateTwen(float& num, float end, float time);

	void twen();

	void Update();

	void Play();
	void Stop();

	~Twennumber();
};

class Twenposition {
	Position* beguin;
	Position end;
	float* deltaT;
	Twennumber tw[3];
	float numbers[3] = { 0.f,0.f,0.f };
public:
	bool isplaying = false;
	Twenposition(float& deltatime);

	void CreateTwen(Position& Posbeguin, Position Posend, float time);

	void Play();

	void Stop();

	void Update();

	~Twenposition();
};

template <typename Thing>
class Manager {
public:
	std::list<Thing> thingV;
	int size = 0;
	Thing(*fun)();
	std::chrono::steady_clock::time_point last = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();
	float deltatime=0;

	Thing* Create() {
		size++;
		thingV.push_back(fun());
		return &thingV.back();
	}

	template <typename... Args>
	inline Thing* Create(Thing(*f)(Args...), Args... args) {
		size++;
		thingV.push_back(f(args...));
		return &thingV.back();
	}

	int FindObj(Thing& tg) {
		auto n = std::find(thingV.begin(), thingV.end(), tg);
		if (n != thingV.end())
			return std::distance(thingV.begin(), n);
		else
			return -1;
	}

	void Erease(Thing& tg) {
		size--;
		auto i = FindObj(tg);
		if (i != thingV.end())
			thingV.erase(i);
	}

	template <typename... Args>
	void Update(void(*f)(Thing&, Args...), Args...args) {
		last = std::chrono::high_resolution_clock::now();
		for (Thing& t : thingV) {
			f(t, args...);
		}
		now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> deltatimeC = now - last;
		deltatime = deltatimeC.count();

	}

	void Update() {
		for (Thing& t : thingV) {
			t.Update();
		}
	}

	~Manager() {
		thingV.clear();
	}

	Thing* operator[](int i) {
		if (i < 0 || i >= thingV.size()) {
			return nullptr;
		}
		auto it = thingV.begin();
		std::advance(it, i);
		return &(*it);
	}
};

class Ticks {
	float TickPass = 0;
	float timepool = 0;
	float max = 0;
	bool set = false;

public:
	float* t;
	bool trigger = false;
	int ticks = 0;

	Ticks(float* deltaT, float maxT, float tickduration);

	void Update();
};

class Camera {
private:
	void keys();

public:
	Position Cmp;
	Position Cmfront = { 0.001f, 1.f, 0.001f };
	Position CmUp = { 0.001f, 1.f, 0.001f };
	float fov = ToRadians(60.f);
	float near1 = 0.1f;
	float far1 = 100.0f;
	bool keyboardaffect = false;
	KeyBoard* k;
	mat4 matView;
	mat4 matPers;

	Camera();

	Camera(KeyBoard& k) ;

	Position camerapov3D(Position p);

	Position camerapov2D(const Position p);

	void update();

};

class Collider {
	bool Set = false;
	bool functset = false;
public:
	
	std::function<void(Collider &me,Collider &other)> WhenCO;

	std::string serial = "serial";
	std::string MyType;

	bool collied = false;
	bool CanColllide = true;
	int MyId = 0;

	std::vector<void*> colisors;
	std::vector<std::pair<std::string, Collider>> types;
	std::vector<std::string> excludeTipes;
	std::vector<std::string> ExcludeBySerial;

	Position p;
	Position size;

	void* pointertothething;


	Collider(std::string Mytipo, void* thing);

	Position midle(Position p, Position sz);

	void collide(Collider &othercollider);

	void Update();

	void SetWhenCollideFunction(std::function<void(Collider &me,Collider& other)>);

	bool operator ==(const Collider other)const {
		if (serial == other.serial) {
			return true;
		}
	}

};
//---------------------------------------------------
// REGIUON GENERATOR
//---------------------------------------------------

class RegionGenerator {
	std::vector<std::string> Regions;
	std::vector<Position> RegionsSize;

	std::vector<Texture>* txs;
	std::vector<char> txtnames;
	int Reindex = 0;
	MainScreen* sc;
	Position pIndx = { 1,1,1 };
	Position mr;
public:
	Position WhereIleft = { 0.f,0.f,0.f };
	Position pIndexDone;

	RegionGenerator(MainScreen& screen, Position MapRatio, std::string TextureNames);

	void AddRegion(char* re, Position size);

	char getnum(int index);

	void Generate(Manager<Texture>& mn, Manager<Collider>& mc, std::vector<Texture>& txs);

	void Generate(Manager<Texture>& mn, Manager<Collider>& mc, std::vector<Texture>& txs, int index);

	void DeleteRegio(int index);

	void changeRegio();
};

//---------------------------
// PATHFIND
//-----------------------------

struct Node {

	Position p;
	double Score;
	Node* Parent;
	int tile = 0;
};

inline bool sor(const Node* a, const Node* b) {
	return a->Score > b->Score;
}

class PathFind {
	Position moves[4] = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {-1.f, 0.f, 0.f}, {0.f, -1.f, 0.f} };
	std::vector<Collider*> c;
	Manager<Collider>* mc;
	double threshold = 0.05f;
	int interations = 0;
	int MaxInterations = 500;

	double DisEuclid(Position n1, Position n2);

	void UpdateCO(Collider& c);

	void Getneighbors(Position current);

	void GetneighborsPerFrame(Position current);

	int IsIn(Node* shearshfor, std::vector<Node*> thingV);

	void RedoPath(Node* agora, std::vector<Position>& path);

	bool InRange(Position p1, Position p2, double threshold);


public:
	PathFind(Manager<Collider>& MC, Position pass, double threshold);

	std::vector<Position> Find(Position Start, Position End);

	std::vector<Node*> aberto;
	std::vector<Node*> fechado;
	std::vector<Node*> Fechar;
	bool once = false;
	Position FindPerFrame(Position Start, Position End);
};