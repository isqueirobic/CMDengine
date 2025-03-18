#include"Screen.h"
MainScreen::MainScreen(int width, int hight, int PixelWidth, int PixelHight) { // initat here if yes
	// variables that holds sizes and buffers
	Width = width;
	Height = hight;
	AbsoluteSize = width * hight;
	ScreenAspectRatio = width / hight;
	Screen = new char[AbsoluteSize];
	Zbuffer = new float[AbsoluteSize];
	lenght = AbsoluteSize;
	// font and screen size
	font.cbSize = sizeof(font);
	font.dwFontSize.X = PixelWidth;  // Largura do font
	font.dwFontSize.Y = PixelHight;  // Altura do font
	SetCurrentConsoleFontEx(buff, FALSE, &font);

	COORD s = { Width, Height };
	SetConsoleScreenBufferSize(buff, s);

	SMALL_RECT windowSize;
	windowSize.Left = 0;
	windowSize.Top = 0;
	windowSize.Right = width - 1;
	windowSize.Bottom = Height - 1;
	SetConsoleWindowInfo(buff, TRUE, &windowSize);
}
void MainScreen::CleanBuffer() { // clear our screen and zbuffer
	GetConsoleScreenBufferInfo(buff, &bfinfo);
	int wid = bfinfo.srWindow.Right - bfinfo.srWindow.Left + 1;
	int hei = bfinfo.srWindow.Bottom - bfinfo.srWindow.Top + 1;
	wid = std::abs(wid);
	hei = std::abs(hei);
	if (wid != Width || hei != Height) { // rip and tear (recize our console and buffer)
		Width = wid;
		Height = hei;
		AbsoluteSize = Width * Height;
		ScreenAspectRatio = Width / Height;
		lenght = AbsoluteSize;
		COORD s = { Width, Height };
		SetConsoleScreenBufferSize(buff, s);
	}

	delete[] Screen;
	Screen = new char[AbsoluteSize + 1];
	delete[]Zbuffer;
	Zbuffer = new float[AbsoluteSize + 1];

	for (int i = 0;i < AbsoluteSize;i++) {
		Screen[i] = ' ';
		Zbuffer[i] = 0.0f;
	}


	foregroundWindow = GetForegroundWindow();
	if (ConsoleW == foregroundWindow) {
		IsTargetWindow = false;
	}
	else {
		IsTargetWindow = true;
	}
	
	last = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> deltatimeC = now - last;
	deltatime = deltatimeC.count();
}
void MainScreen::DrawBuffer() { // write onto our buffer
	DWORD charswriten;
	now = std::chrono::high_resolution_clock::now();
	WriteConsoleOutputCharacterA(buff, Screen, lenght, cood, &charswriten);
}

Position MainScreen::ToScreen(Position p) {
	p.x = (p.x + 1.f) * 0.5 * Width;
	p.y = (1.f-p.y) * 0.5 * Height;
	p.z *= ScreenAspectRatio;
	//printf("x: %d, y: %d \n", Width, Height);
	return p;
}

//--------------------------------------------------------------------------------------
// TEXTURES
//--------------------------------------------------------------------------------------

Texture::Texture(Position position, Position size, MainScreen& Screen) {
	p = position;
	this->size = size;
	screen = &Screen;
}

Texture::Texture() {}

void Texture::Set(MainScreen& Screen) {
	screen = &Screen;
}

void Texture::LoadTexture(const char* FileName, int FileszX, int FileszY) {
	TextureIsLoading = true; // maybe mult thread so keep all save
	std::ifstream file(FileName); // load
	TextureRealSize.x = FileszX;
	TextureRealSize.y = FileszY;
	TextureRealSize.z = FileszX * FileszY; // set real sizes
	char* text = new char[TextureRealSize.z]; // set our string that contains our texture
	file.read(text, TextureRealSize.z); // copy to the string
	file.close(); // close 
	Texture_ = text;
	delete[] text;// memory leacks no more
	TextureIsLoading = false;
	TextureIsSet = true; // just cuz
}

void Texture::LoadTexture(std::string text, int TextX, int TextY) { // it's the same thing but we jsut copy straight to our texture string
	TextureIsLoading = true;
	TextureRealSize.x = TextX;
	TextureRealSize.y = TextY;
	TextureRealSize.z = TextX * TextY;
	Texture_ = text;
	TextureIsLoading = false;
	TextureIsSet = true;
}

void Texture::RenderTexture() {
	Position size = this->size, p = this->p; // copy
	Prep(p, size); // translate things to screen space
	Position Tz; Tz.x = size.x / p.z; Tz.y = size.y / p.z;Tz.z = p.z; // aply the Z buffer to encrease or decrease the texture size on the screen
	Position Tps; Tps.x = TextureRealSize.x / Tz.x; Tps.y = TextureRealSize.y / Tz.y; // Ratio from the size to the screen to the size of the texture
	Position  Rps{ p.x - Tz.x / 2, p.y - Tz.y / 2 }; // let's put the texture in the midle of the position it is
	for (int y1 = 0; y1 < Tz.y; y1++) {
		for (int x1 = 0; x1 < Tz.x; x1++) {

			Position p2 = { x1, y1, p.z };
			Position fin = RotateVec(p2, ToRadians(Angles));
			
			float x2 =  Rps.x + fin.x; // this one goes da screen
			int y2 =  Rps.y + fin.y; // this one goes to the screen
			if (x2 < screen->Width && y2 < screen->Height && x2 > 0 && y2 > 0) { // is in screen or out

				float x = x1;
				int y = y1;
				x *= Tps.x; //aply ratio
				y *= Tps.y;

				int indexscreen = y2 * screen->Width + x2; // index that goes to the screen
				int indextexture = y * TextureRealSize.x + x;//and this one goes to the texture

				indextexture = indextexture > 0 && indextexture < TextureRealSize.z ? indextexture : 0; // is in bounds ? yes alr, no, 0

				if (screen->Zbuffer[indexscreen] > p.z || screen->Zbuffer[indexscreen] < 0.000001f) // it only worcks if we put this smaal float number there
				{
					if (Texture_[indextexture] != ' ' && Texture_[indextexture] != '\n') // fun fact that if the space is nothing we just skip, same with \n
					{
						screen->Screen[indexscreen] = Texture_[indextexture];
						screen->Zbuffer[indexscreen] = p.z;
					}
				}
			}
		}
	}
}

void Texture::RenderTexture(Position p1) { // same thing but we cant set the position by calling it
	Position size = this->size, p = p1;
	Prep(p, size);
	Position Tz; Tz.x = size.x / p.z; Tz.y = size.y / p.z;Tz.z = p.z;
	Position Tps; Tps.x = TextureRealSize.x / Tz.x; Tps.y = TextureRealSize.y / Tz.y;
	Position Rps = { p.x - Tz.x / 2, p.y - Tz.y / 2 };
	if (p.z > NearZ) {
		for (int y1 = 0; y1 < Tz.y; y1++) {
			for (int x1 = 0; x1 < Tz.x; x1++) {

				Position p2 = { x1, y1, p.z };
				Position fin = RotateVec(p2, ToRadians(Angles));

				float x2 = fin.x + Rps.x;
				int y2 = fin.y + Rps.y;
				if (x2 < screen->Width && y2 < screen->Height && x2 > 0 && y2 > 0) {

					float x = x1;
					int y = y1;
					x *= Tps.x;
					y *= Tps.y;

					int indexscreen = y2 * screen->Width + x2;
					int indextexture = y * TextureRealSize.x + x;

					indextexture = indextexture > 0 && indextexture < TextureRealSize.z ? indextexture : 0;

					//std::cout << screen->Zbuffer[indexscreen] << ' ' << p.z << '\n';
					if (screen->Zbuffer[indexscreen] > p.z || screen->Zbuffer[indexscreen] < 0.000001f)
					{
						if (Texture_[indextexture] != ' ' && Texture_[indextexture] != '\n')
						{
							screen->Screen[indexscreen] = Texture_[indextexture];
							screen->Zbuffer[indexscreen] = p.z;
						}
					}
				}
			}
		}
	}
}

void Texture::Draw() { // same thing but just draw a square on the screen
	Position size = this->size, p = this->p;
	Prep(p, size);
	Position Tz; Tz.x = size.x / p.z; Tz.y = size.y / p.z;Tz.z = p.z;
	Position  Rps{ p.x - Tz.x / 2, p.y - Tz.y / 2 };
	for (int y1 = 0; y1 < Tz.y; y1++) {
		for (int x1 = 0; x1 < Tz.x; x1++) {

			Position p2 = { x1, y1, p.z };
			Position fin;
			fin = RotateVec(p2, ToRadians(Angles));
			float x2 = fin.x + Rps.x;
			int y2 = fin.y + Rps.y;
			if (x2 < screen->Width && y2 < screen->Height && x2 > 0 && y2 > 0) {

				int indexscreen = y2 * screen->Width + x2;

				if (screen->Zbuffer[indexscreen] > p.z || screen->Zbuffer[indexscreen] < 0.000001f)
				{
					screen->Screen[indexscreen] = DefaultText;
					screen->Zbuffer[indexscreen] = p.z;
				}
			}
		}
	}

}

void Texture::Draw(Position p1) {
	Position size = this->size, p=p1;
	Prep(p, size);
	Position Tz; Tz.x = size.x / p.z; Tz.y = size.y / p.z;Tz.z = p.z;
	Position Rps{ p.x - Tz.x / 2, p.y - Tz.y / 2 };
	for (int y1 = 0; y1 < Tz.y; y1++) {
		for (int x1 = 0; x1 < Tz.x; x1++) {

			Position p2 = { x1, y1, p.z };
			Position fin;
			fin = RotateVec(p2, ToRadians(Angles));
			float x2 = fin.x + Rps.x;
			int y2 = fin.y + Rps.y;
			if (x2 < screen->Width && y2 < screen->Height && x2 > 0 && y2 > 0) {

				int indexscreen = y2 * screen->Width + x2;

				if (screen->Zbuffer[indexscreen] > p.z || screen->Zbuffer[indexscreen] < 0.000001f)
				{
					screen->Screen[indexscreen] = DefaultText;
					screen->Zbuffer[indexscreen] = p.z;
				}
			}
		}
	}

}



void Texture::Update() {
	if (CanDraw) {
		if (TextureIsSet)
		{
			RenderTexture();
		}
		else
		{
			Draw();
		}
	}
	
}

void Texture::Update(Position p) {
	if (CanDraw) {
		if (TextureIsSet)
			RenderTexture(p);
		else
			Draw(p);
	}

}

void Texture::Prep(Position& p, Position& size) {
	p = screen->ToScreen(p);
	size.x *= screen->Width;
	size.y *= screen->Height;
}

//--------------------------------------------------------------------------------------
// KEYBOARD
//--------------------------------------------------------------------------------------

KeyBoard::KeyBoard(MainScreen& sc) { // YOU CAN CALL THE KEY BY JUST PUTTING THE UPER CASE KEY per example Keys['A'] if pressed is true if not false
	Screen = &sc;
	memset(Keys, false, 256);
}

void KeyBoard::UpdateKeys() {
	DoneOnce = false;
	for (int i = 8; i <= 255; i++) { // just check all keys and set true or false
		if (GetAsyncKeyState(i) & 0x8000) { Keys[i] = true; }
		else { Keys[i] = false; }
	}

}

//--------------------------------------------------------------------------------------
// LIGHT
//--------------------------------------------------------------------------------------

Light::Light(MainScreen& sc, float _magnetude, Position pos) { // neds to be revised
	intenct = 1.3;
	Siz = { _magnetude, _magnetude , _magnetude };
	p = pos;
	screen = &sc;
	CharLightLenght = sizeof(LightTbl) + 0.1;
}

void Light::ApplyLightOn(Texture& _Rect) {
	rect = &_Rect;
	float SizeM = Magnitude(Siz);
	float TrueSize[2] = { rect->size.x / rect->p.z, rect->p.y / rect->p.z };
	for (int y = rect->p.y - TrueSize[1]; y < rect->p.y + TrueSize[1]; y++) {
		for (int x = rect->p.x - TrueSize[0]; x < rect->p.x + TrueSize[0]; x++) {
			if (x < screen->Width && y < screen->Height && x > 0 && y > 0) {
				int Index = y * screen->Width + x;
				Position po = { x , y ,  rect->p.z };
				Position P1 = p;
				double Indexlight = DistanciaReal(P1, po); // got the magnitude of the 2 vectors
				Indexlight *= CharLightLenght; // gotta make shure that this shit dont be > then the sizer of the list
				Indexlight /= SizeM; // our light cant be infite so we divide by his oun size

				Indexlight = Indexlight > CharLightLenght ? CharLightLenght : Indexlight;

				if (rect->p.z > screen->Zbuffer[Index]) {  // z buff
					screen->Screen[Index] = LightTbl[(int)Indexlight]; // the rest we know
					screen->Zbuffer[Index] = rect->p.z;
				}
			}
		}
	}



}


//--------------------------------------------------------------------------------------
// WORDS
//--------------------------------------------------------------------------------------

Words::Words(const char* FontPath, Position FontXY, MainScreen& sc, int Yabsolute) { // the first one is the path to the font, WE ONLY ACCEPT FONTS ON TXT FILES LITERELY DRAWED and it has to be verticaly
	screen = &sc;
	std::ifstream FontFile(FontPath);
	TotalSize = (FontXY.x * Yabsolute);
	FontTexture = new char[TotalSize];
	TrueFontSize.x = FontXY.x;
	TrueFontSize.y = Yabsolute;
	size = FontXY;
	TrueFontSize.z = 1;
	CharSpacing.x = FontXY.x;
	CharSpacing.y = FontXY.y;
	p.z = 1;
	FontFile.read(FontTexture, TotalSize);
	FontFile.close();
}


void Words::SetString(char* m) {
	NumLaters = strlen(m);
	delete[] message;
	delete[] NumLatersPos;
	message = new char[NumLaters];
	NumLatersPos = new int[NumLaters];
	message = m;

	for (int i = 0; i < NumLaters + 1;i++) { // the +1 it's for displaying the last letter because it wasant displaying it
		int index = NumLaters;
		index = IsIn(message[i], Suported, NumSuported);
		NumLatersPos[i] = index; // set index to display like 0 = a and 24 = z
	}

}

void Words::SetString(std::string m) { // same but with strings
	NumLaters = m.size() - 1;
	delete[] message;
	delete[] NumLatersPos;
	message = new char[m.size()];
	NumLatersPos = new int[m.size()];
	m.copy(message, m.size());

	for (int i = 0; i < NumLaters + 1;i++) {
		int index = NumLaters;
		index = IsIn(message[i], Suported, NumSuported);
		NumLatersPos[i] = index;
		oi[i] = index;
	}

}

void Words::RenderWords(int CharPosIndex, Position offset) {
	Position ToScrenn = { size.x / p.z,size.y / p.z, 1 }; // this part is the same as the setup with textures
	Position Ratio = { CharSpacing.x / ToScrenn.x, CharSpacing.y / ToScrenn.y };
	for (int y = 0;y < ToScrenn.y;y++) {
		for (int x = 0;x < ToScrenn.x;x++) {
			int X = x + p.x + (ToScrenn.x * offset.x); // this one just set the offset of the word
			int Y = y + p.y + (ToScrenn.y * offset.y);// same here
			if (X > 0 && X < screen->Width && Y>0 && Y < screen->Height) { // is in bounds
				int x1 = x * Ratio.x;
				int y1 = y * Ratio.y;

				int indexscreen = Y * screen->Width + X;
				int atY = CharSpacing.y * CharPosIndex + y1;// it's because the pallet horientation is UP
				int indexchar = (atY) * (TrueFontSize.x) + x1;
				if (indexchar > TotalSize - 1 || indexchar < 1) // I alredy forgot what this does dont change it
					indexchar = 0;
				if (FontTexture[indexchar] == '\n')
					indexchar = 0;
					screen->Screen[indexscreen] = FontTexture[indexchar];
					screen->Zbuffer[indexscreen] = p.z;
			}
		}
	}
}


void Words::WriteWords() {
	for (int i = 0;i < NumLaters + 1 - 1;i++) {
		Position p = { i,0,0 };
		RenderWords(NumLatersPos[i], p);

	}
}

//--------------------------------------------------------------------------------------
// Twennumber
//--------------------------------------------------------------------------------------

Twennumber::Twennumber(float& deltaT) {
	deltat = &deltaT;
}

Twennumber::Twennumber() {
}

void Twennumber::Setdeltatime(float& deltaT) {
	deltat = &deltaT;
}

void Twennumber::CreateTwen(float& num, float end, float time) {
	timePoll = 0;
	beguin = &num;
	fin = end;
	this->time = time;
	speed = *beguin - end;
	speed /= time;

}

void Twennumber::twen() {
	playing = true;
	if (timePoll <= time)
	{
		float det = *deltat;
		timePoll += det;
		if ((speed > 0 && *beguin >= fin) || (speed < 0 && *beguin <= fin)) { // this wierd thing it presents the values to goes to the space
			*beguin -= speed * det;
			//*beguin = fin; // Garante que o valor final seja exatamente atingido
		}

	}

}

void Twennumber::Update() {
	if (update)
		twen();
	else
		playing = false;
}

void Twennumber::Play() { update = true; }
void Twennumber::Stop() { update = false; }

Twennumber::~Twennumber()
{
	delete beguin;
	delete deltat;
}


//--------------------------------------------------------------------------------------
// Twenposition
//--------------------------------------------------------------------------------------


Twenposition::Twenposition(float& deltatime) {
	deltaT = &deltatime;
	for (int i = 0;i < 3;i++) {
		tw[i].Setdeltatime(*deltaT);
	}
}


void Twenposition::CreateTwen(Position& Posbeguin, Position Posend, float time) {
	beguin = &Posbeguin;
	for (int i = 0;i < 3;i++) {
		numbers[i] = Posbeguin[i];
		tw[i].CreateTwen(numbers[i], Posend[i], time);
	}
}

void Twenposition::Play() {
	isplaying = true;
	tw[0].Play();
	tw[1].Play();
	tw[2].Play();
}

void Twenposition::Stop() {
	isplaying = false;
	tw[0].Stop();
	tw[1].Stop();
	tw[2].Stop();
}

void Twenposition::Update() {
	for (int i = 0;i < 3;i++) {
		tw[i].Update();
	}
	beguin->x = numbers[0];
	beguin->y = numbers[1];
	beguin->z = numbers[2];
}

Twenposition::~Twenposition() {
		delete beguin;
		delete deltaT;
}

//-----------------------------------------------------
// TICKS
//-----------------------------------------------------

Ticks::Ticks(float* deltaT, float maxT, float tickduration) { // delta time, max count of ticks, how much time is a tick
	t = deltaT;
	TickPass = tickduration;
	max = maxT;
}

void Ticks::Update() {
	if (trigger) trigger = false;
	if (timepool >= TickPass)
	{
		if (ticks >= max) {
			ticks = 0;
			trigger = true;
		}
		else {
			ticks++;
		}
		
		timepool = 0;
		
	}
	else {
		timepool += *t; // increase the time pool with the delta time
	}
}
//---------------------------------------------------------------
// CAMERA
//---------------------------------------------------------------

void Camera::keys() {
	
	if (k->Keys['A']) {
		Cmp.x += 0.01;
	}

	if (k->Keys['D']) {
		Cmp.x -= 0.01;
	}

	if (k->Keys['W']) {
		Cmp.y -= 0.01;
	}

	if (k->Keys['S']) {
		Cmp.y += 0.01;
	}

	if (k->Keys['E']) {
		Cmp.z -= 0.01;
	}

	if (k->Keys['Q']) {
		Cmp.z += 0.01;
	}

	// bruh

	if (k->Keys['F']) {
		Cmfront.x += 0.01;
	}

	if (k->Keys['H']) {
		Cmfront.x -= 0.01;
	}

	if (k->Keys['T']) {
		Cmfront.y -= 0.01;
	}

	if (k->Keys['G']) {
		Cmfront.y += 0.01;
	}
}

Camera::Camera() {}

Camera::Camera(KeyBoard& k) { this->k = &k; keyboardaffect = true; }

Position Camera::camerapov3D(Position p) { 
	lookat(Cmfront, Cmp, CmUp, matView);
	perspective(fov, 1.3f, near1, far1, matPers);
	multplMat(matView, p);
	//printf("%f %f\n", p.z, p2.z);
	multplMat(matPers, p);
	return p;
}

Position Camera::camerapov2D(const Position p) {
	return Position{ p.x - Cmp.x, p.y - Cmp.y, p.z };
}

void Camera::update() {
	if (keyboardaffect)
		keys();
}

//--------------------------------------------------
// COLLIDER
//--------------------------------------------------

Collider::Collider(std::string Mytipo, void* thing) { // the ID must be the index of the manager of the thing
	MyType = Mytipo;
	pointertothething = thing;
}

Position Collider::midle(Position p, Position sz) {
	return Position{ p.x - size.x / 2, p.y - size.y / 2 };
}

void Collider::collide(Collider &othercollider) {
	if (CanColllide)
	{
		float pm = Magnitude(p - othercollider.p);
		float szm = Magnitude(size + othercollider.size);
		if (szm > pm) { // it's that close ? if yes let's see if not go away
		bool can = true;
		for (std::string s : excludeTipes) { // let's see if there is sothing we dont like 1
			if (othercollider.MyType == s) {
				can = false;
			}
		}
		for (std::string s : ExcludeBySerial) { // let's see if there is sothing we dont like 2
			if (othercollider.serial == s) {
				can = false;
			}
		}
		if (can) { //can
				Position p = midle(this->p, this->size); // let's set our quare to the midle of the position
				Position p1 = midle(othercollider.p, othercollider.size);// and our frend's suqare to
				Position sz1 = size / 2; // we do that cuz if we dont the size actualy it's 2x bigger
				Position sz2 = othercollider.size / 2;
				bool cx = p.x < p1.x + sz2.x && p.x + sz1.x > p1.x; // see if it's in bounds
				bool cy = p.y < p1.y + sz2.y && p.y + sz1.y > p1.y;
				if (cx && cy) // it's in bounds ?
				{
					collied = true; // yes
					types.push_back({othercollider.MyType, othercollider});
					colisors.push_back(othercollider.pointertothething);

					if (functset) {
						WhenCO(*this,othercollider); // run sum code that you wwant
					}
				}



			}
		}

	}
}

void Collider::Update() {
	if (Set) {
		types.clear();
		colisors.clear();
		Set = false;
		collied = false;
	}
	if (collied) {
		Set = true; // collided can be true for a entire cicle, i think it's better this way
	}

}

void Collider::SetWhenCollideFunction(std::function<void(Collider &me,Collider& other)> other) {
	WhenCO = other;
	functset = true;
}

//--------------------------------------------------
// REGION GENERATOR
//--------------------------------------------------



RegionGenerator::RegionGenerator(MainScreen& screen, Position MapRatio, std::string TextureNames) { // screen the ratio of the map, and the texture names and it's only one char it works like a index per name
	sc = &screen;
	mr = MapRatio;
	for (auto h : TextureNames) {
		txtnames.push_back(h);
	}
}

void RegionGenerator::AddRegion(char* re, Position size) {
	Regions.push_back(re);
	RegionsSize.push_back(size);
}

char RegionGenerator::getnum(int index) {
	return Regions[index][pIndx.y * RegionsSize[index].x + pIndx.x];
}

void RegionGenerator::Generate(Manager<Texture>& mn, Manager<Collider>& mc, std::vector<Texture>& txs) { // the manager of texture, collider, and the a vector of textures that you want to be your word
	for (int i = 0;i <= RegionsSize[Reindex].y+1;i++) {

		for (int x = 0;x < RegionsSize[Reindex].x;x++) {
			char TextType = getnum(Reindex); // se what texture we have to do
			if (TextType != '0') // if not nothing do sothing
			{
				int txtindex = IsIn(TextType, txtnames);// get the index of the texture
				Position size = { 1.f / mr.x, 1.f / mr.y, 1.f };
				Position p = { size.x * (x-(RegionsSize[Reindex].x/2)), size.y * WhereIleft.y, 1.f }; // getting the position of the texture 
				size /= 2;
				Texture t(p, size, *sc);
				Collider* c = mc.Create();
				c->p = p;
				c->size = size;
				c->MyType = "Terrain";
				t.LoadTexture(txs[txtindex].Texture_, txs[txtindex].size.x, txs[txtindex].size.y);
				mn.thingV.push_back(t);
			}
			pIndx.x++;// encrease to get the X in the list of the map
		}
		pIndx.x = 0;

		if (pIndx.y < RegionsSize[Reindex].y)
			pIndx.y++; // the map can go infnitly up
		else
			pIndexDone.y = 1;

		if (pIndexDone.y == 1) {
			changeRegio();
		}

		WhereIleft.y++;
	}
}

void RegionGenerator::Generate(Manager<Texture>& mn, Manager<Collider>& mc, std::vector<Texture>& txs, int index) {
	int Reindex = index;
	for (int i = 0;i <= RegionsSize[Reindex].y + 1;i++) {

		for (int x = 0;x < RegionsSize[Reindex].x;x++) {
			char TextType = getnum(Reindex);
			if (TextType != '0')
			{
				int txtindex = IsIn(TextType, txtnames);
				Position size = { 1.f / mr.x, 1.f / mr.y, 1.f };
				Position p = { (size.x * 1) * (x - (RegionsSize[Reindex].x / 2)), (size.y * 1) * WhereIleft.y, 1.f };
				size /= 2;
				Texture t(p, size, *sc);
				Collider* c = mc.Create();
				c->p = p;
				c->size = size;
				c->MyType = "Terrain";
				t.LoadTexture(txs[txtindex].Texture_, txs[txtindex].size.x, txs[txtindex].size.y);
				mn.thingV.push_back(t);
			}
			pIndx.x++;
		}
		pIndx.x = 0;

		if (pIndx.y < RegionsSize[Reindex].y)
			pIndx.y++;
		else
			pIndexDone.y = 1;

		if (pIndexDone.y == 1) {
			changeRegio();
		}

		WhereIleft.y++;
	}
}

void RegionGenerator::DeleteRegio(int index) {
	Regions.erase(Regions.begin() + index);
	RegionsSize.erase(RegionsSize.begin() + index);
}

void RegionGenerator::changeRegio() {
	Reindex = rand() % Regions.size();
	pIndx.x = 0;
	pIndx.y = 0;
	pIndexDone.y = 0;
}

//
// PATH FIND
//

double  PathFind::DisEuclid(Position n1, Position n2) {
	return std::sqrtl((n1.x - n2.x) * (n1.x - n2.x) + (n1.y - n2.y) * (n1.y - n2.y));
}

void  PathFind::UpdateCO(Collider& c) {
	c.Update();
	for (Collider co : mc->thingV)
	{
		c.collide(co);
	}

}

void  PathFind::Getneighbors(Position current) {
	for (int i = 0;i < 4;i++) {
		c[i]->p = current + moves[i];
		UpdateCO(*c[i]);
	}
}

void  PathFind::GetneighborsPerFrame(Position current) {
	for (int i = 0;i < 4;i++) {
		c[i]->p = current + moves[i];
	}
}

int  PathFind::IsIn(Node* shearshfor, std::vector<Node*> thingV) {
	int i = 0;
	for (const auto c : thingV) {
		if (shearshfor->p == c->p) { return i; }
		i++;
	}
	return -1;
}

void  PathFind::RedoPath(Node* agora, std::vector<Position>& path) {
	Node back;
	Node now = *agora;
	while (true)
	{
		if (now.Parent != nullptr)
		{
			//printf("p: %f %f %f\n", now.p.x, now.p.y, now.p.z);
			path.push_back(now.p);
			now = *now.Parent;
		}
		else
		{
			break;
		}
	}
}

bool  PathFind::InRange(Position p1, Position p2, double threshold) {
	double dis = DisEuclid(p1, p2);
	return dis <= threshold;
}

PathFind::PathFind(Manager<Collider>& MC, Position pass, double threshold) {
	mc = &MC;
	this->threshold = threshold;
	for (int i = 0; i < 4;i++) {
		c.push_back(MC.Create());
		c[i]->MyType = "Move";
		c[i]->size = pass;
		c[i]->excludeTipes.push_back("Move");
		//c[i]->excludeTipes.push_back("Npc");
		moves[i] *= pass;


	}
}

std::vector<Position> PathFind::Find(Position Start, Position End) {
	interations = 0;
	std::vector<Node*> aberto;
	std::vector<Node*> fechado;
	std::vector<Node*> Fechar;
	Node t = { Start, 0.f, nullptr };
	aberto.push_back(&t);
	while (!aberto.empty())
	{
		std::sort(aberto.begin(), aberto.end(), sor);
		Node* agora = aberto.back();
		aberto.pop_back();

		//printf("p: %f %f %f disgoal: %f score: %f\n", agora->p.x, agora->p.y, agora->p.z, DisEuclid(agora->p, End),agora->Score);
		if (InRange(agora->p, End, threshold) || interations == MaxInterations) {
			std::vector<Position> path;
			RedoPath(agora, path);
			for (auto c : Fechar) delete c;
			std::reverse(path.begin(), path.end());
			return path;
		}


		Getneighbors(agora->p);
		for (Collider* co : c) {
			if (!co->collied) {
				double dis = DisEuclid(co->p, End) + agora->tile + 1;
				Node* h = new Node{ co->p, dis, agora, agora->tile + 1 };
				Fechar.push_back(h);
				if (IsIn(h, fechado) == -1)
				{
					aberto.push_back(h);
				}
			}

		}

		fechado.push_back(agora);
		interations++;
	}

	return {};
}

Position PathFind::FindPerFrame(Position Start, Position End) {
	interations = 0;
	if (!once) {
		Node t = { Start, 0.f, nullptr };
		aberto.push_back(&t);
		once = true;
	}
	if (!aberto.empty())
	{
		std::sort(aberto.begin(), aberto.end(), sor);
		Node* agora = aberto.back();
		aberto.pop_back();

		//printf("p: %f %f %f disgoal: %f score: %f\n", agora->p.x, agora->p.y, agora->p.z, DisEuclid(agora->p, End),agora->Score);
		if (InRange(agora->p, End, threshold) || interations == MaxInterations) {
			for (auto c : Fechar) 
			{
				h
				//delete c;
			}
			return agora->p;
		}


		Getneighbors(agora->p);
		for (Collider* co : c) {
			if (!co->collied) {
				double dis = DisEuclid(co->p, End) + agora->tile + 1;
				Node* h = new Node{ co->p, dis, agora, agora->tile + 1 };
				Fechar.push_back(h);
				if (IsIn(h, fechado) == -1)
				{
					aberto.push_back(h);
				}
			}

		}

		fechado.push_back(agora);
		interations++;

		return agora->p;
	}
}
