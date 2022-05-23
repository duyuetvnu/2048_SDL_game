#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int BUTTON_WIDTH = 300;
const int BUTTON_HEIGHT = 117;
int SIZE = 4;
int WIN = 2048;
enum LButtonSprite {
	BUTTON_SPRITE_MOUSE_OUT = 0,
	BUTTON_SPRITE_MOUSE_OVER_MOTION = 1,
	BUTTON_SPRITE_MOUSE_DOWN = 2,
	BUTTON_SPRITE_MOUSE_UP = 3,
	BUTTON_SPRITE_TOTAL = 4
};
enum MyButton {
	START_BUTTON,
	EXIT1_BUTTON,
	EXIT2_BUTTON,
	PLAY_AGAIN_BUTTON,
	//SCORE_BUTTON,
	TOTAL_BUTTONS
};
class LTexture
{
public:
	LTexture();
	~LTexture();
	bool loadFromFile(string path);
	bool loadFromRenderedText(string textureText, SDL_Color textColor);
	bool loadFromTopScoreText(string textureText, SDL_Color textColor);
	void free();
	void setColor(Uint8 red, Uint8 green, Uint8 blue);
	void setBlendMode(SDL_BlendMode blending);
	void setAlpha(Uint8 alpha);
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
	int getWidth();
	int getHeight();
private:
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
};

LTexture::LTexture()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	free();
}
class LButton
{
public:
	LButton();
	~LButton();
	void setPosition(int x, int y);
	int handleEvent(SDL_Event* e, int i, bool& getStart, bool& quit);
	void render(int i);
private:
	SDL_Point mPosition;
	LButtonSprite mCurrentSprite;
};

LButton::LButton()
{
	mPosition.x = 0;
	mPosition.y = 0;
	mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
}

LButton::~LButton()
{
}
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
TTF_Font* gTopScoreFont = NULL;
bool init();
bool loadMedia();
void close();
LTexture gBoard;
LTexture g2Tile, g4Tile, g8Tile, g16Tile, g32Tile, g64Tile, g128Tile, g256Tile, g512Tile, g1024Tile, g2048Tile, gBlankTile, gWelcome, gWin, gLose, gBackground;
LTexture gButtonSpriteSheetTexture[TOTAL_BUTTONS];
LTexture gScoreTexture;
LTexture gHighScoreTexture;
LTexture gScoreBoardTexture;
LTexture gTopScoreTexture[5];
LButton gButton[TOTAL_BUTTONS];
LButton gSpeakerButton;
LTexture gSpeakerSpriteSheet;
SDL_Rect gSpriteClips[BUTTON_SPRITE_TOTAL];
Mix_Music* gBackgroundMusic = NULL;
Mix_Chunk* gMove = NULL;
Mix_Chunk* gError = NULL;
Mix_Chunk* gGameOver = NULL;
Mix_Chunk* gWinSound = NULL;
int** createBoard();
bool checkWin(int**& board);
bool checkPut(int**& board);
bool checkLose(int**& board);
void randomOnBoard(int**& board);
void rushTop(int**& board);
void moveTop(int**& board, int& score);
void rushBot(int**& board);
void moveBot(int**& board, int& score);
void rushRight(int**& board);
void moveRight(int**& board, int& score);
void rushLeft(int**& board);
void moveLeft(int**& board, int& score);
void copyBoard(int**& board1, int**& board2);
void print(int**& board, int& score);
bool checkToRandom(int**& board, int**& backBoard);
void playGame(SDL_Event* e, bool& quit);

int main(int argc, char* argv[]) {
	srand(time(NULL));
	if (!init()) {
		cout << "Failed to initialize!\n";
	}
	else {
		if (!loadMedia()) {
			cout << "Failed to load media!\n";
		}
		else {
			bool quit = false;
			SDL_Event e;
			playGame(&e, quit);
		}
	}
	close();
	return 0;
}

bool LTexture::loadFromFile(string path) {
	free();
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSuface = IMG_Load(path.c_str());
	if (loadedSuface == NULL) {
		cout << "Unable to load image!\n";
	}
	else {
		SDL_SetColorKey(loadedSuface, SDL_TRUE, SDL_MapRGB(loadedSuface->format, 255, 255, 255));
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSuface);
		if (newTexture == NULL) {
			cout << "Unable to create texture!\n";
		}
		else {
			mWidth = loadedSuface->w;
			mHeight = loadedSuface->h;
		}
		SDL_FreeSurface(loadedSuface);
	}
	mTexture = newTexture;
	return mTexture != NULL;
}
bool LTexture::loadFromRenderedText(string textureText, SDL_Color textColor) {
	free();
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface == NULL) {
		cout << "Unable to render surface!\n";
	}
	else {
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL) {
			cout << "Unable to create texture from rendered text!\n";
		}
		else {
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}
		SDL_FreeSurface(textSurface);
	}
	return mTexture != NULL;
}
void LTexture::free() {
	if (mTexture != NULL) {
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}
void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
	SDL_Rect renderQuad = { x,y,mWidth,mHeight };
	if (clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}
void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}
void LTexture::setBlendMode(SDL_BlendMode blending) {
	SDL_SetTextureBlendMode(mTexture, blending);
}
void LTexture::setAlpha(Uint8 alpha) {
	SDL_SetTextureAlphaMod(mTexture, alpha);
}
int LTexture::getWidth() {
	return mWidth;
}
int LTexture::getHeight() {
	return mHeight;
}
bool init() {
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		cout << "SDL could not initilize!\n";
		success = false;
	}
	else {
		gWindow = SDL_CreateWindow("2048 by Duy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL) {
			cout << "Window could not be created!\n";
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL) {
				cout << "Renderer could not be created!\n";
				success = false;
			}
			else {
				SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags)) {
					cout << "SDL_image could not be created!\n";
					success = false;
				}
				if (TTF_Init() == -1) {
					cout << "SDL_ttf could not initialize!\n";
					success = false;
				}
				if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
					cout << "SDL_mixer could not initialize!\n";
					success = false;
				}
			}
		}
	}
	return success;
}
bool loadMedia() {
	bool success = true;
	if (!gBoard.loadFromFile("media/image/board.png")) {
		cout << "Failed to load board!\n";
		success = false;
	}
	if (!g2Tile.loadFromFile("media/image/2.png")) {
		cout << "Failed to load 2 tile!\n";
		success = false;
	}
	if (!g4Tile.loadFromFile("media/image/4.png")) {
		cout << "Failed to load 4 tile!\n";
		success = false;
	}
	if (!g8Tile.loadFromFile("media/image/8.png")) {
		cout << "Failed to load 8 tile!\n";
		success = false;
	}
	if (!g16Tile.loadFromFile("media/image/16.png")) {
		cout << "Failed to load 16 tile!\n";
		success = false;
	}
	if (!g32Tile.loadFromFile("media/image/32.png")) {
		cout << "Failed to load 32 tile!\n";
		success = false;
	}
	if (!g64Tile.loadFromFile("media/image/64.png")) {
		cout << "Failed to load 64 tile!\n";
		success = false;
	}
	if (!g128Tile.loadFromFile("media/image/128.png")) {
		cout << "Failed to load 128 tile!\n";
		success = false;
	}
	if (!g256Tile.loadFromFile("media/image/256.png")) {
		cout << "Failed to load 256 tile!\n";
		success = false;
	}
	if (!g512Tile.loadFromFile("media/image/512.png")) {
		cout << "Failed to load 512 tile!\n";
		success = false;
	}
	if (!g1024Tile.loadFromFile("media/image/1024.png")) {
		cout << "Failed to load 1024 tile!\n";
		success = false;
	}
	if (!g2048Tile.loadFromFile("media/image/2048.png")) {
		cout << "Failed to load 2048 tile!\n";
		success = false;
	}
	if (!gBlankTile.loadFromFile("media/image/blank.png")) {
		cout << "Failed to load 0 tile!\n";
		success = false;
	}
	if (!gWelcome.loadFromFile("media/image/Welcome.png")) {
		cout << "Failed to load welcome background!\n";
		success = false;
	}
	if (!gLose.loadFromFile("media/image/lose.png")) {
		cout << "Failed to load losing background!\n";
		success = false;
	}
	if (!gWin.loadFromFile("media/image/win.png")) {
		cout << "Failed to load winning background!\n";
		success = false;
	}
	if (!gBackground.loadFromFile("media/image/background.png")) {
		cout << "Failed to load background!\n";
		success = false;
	}
	gFont = TTF_OpenFont("media/font/SVN-DHF Dexsar Brush.ttf", 28);
	if (gFont == NULL) {
		cout << "Failed to load font!\n";
		success = false;
	}
	gTopScoreFont = TTF_OpenFont("media/font/SVN-DHF Dexsar Brush.ttf", 40);
	if (gTopScoreFont == NULL) {
		cout << "Failed to load font!\n";
		success = false;
	}
	if (!gButtonSpriteSheetTexture[START_BUTTON].loadFromFile("media/image/start.png")) {
		cout << "Failed to load button sprite texture!\n";
		success = false;
	}
	else {
		for (int i = 0; i < BUTTON_SPRITE_TOTAL; ++i) {
			gSpriteClips[i].x = 0;
			gSpriteClips[i].y = i * BUTTON_HEIGHT;
			gSpriteClips[i].w = BUTTON_WIDTH;
			gSpriteClips[i].h = BUTTON_HEIGHT;
		}
		gButton[START_BUTTON].setPosition((SCREEN_WIDTH - BUTTON_WIDTH) / 8, (SCREEN_HEIGHT - BUTTON_HEIGHT) / 2 + 50);
	}
	if (!gButtonSpriteSheetTexture[EXIT1_BUTTON].loadFromFile("media/image/exit.png")) {
		cout << "Failed to load button sprite texture!\n";
		success = false;
	}
	else {
		for (int i = 0; i < BUTTON_SPRITE_TOTAL; ++i) {
			gSpriteClips[i].x = 0;
			gSpriteClips[i].y = i * BUTTON_HEIGHT;
			gSpriteClips[i].w = BUTTON_WIDTH;
			gSpriteClips[i].h = BUTTON_HEIGHT;
		}
		gButton[EXIT1_BUTTON].setPosition((SCREEN_WIDTH - BUTTON_WIDTH) / 8, (SCREEN_HEIGHT - BUTTON_HEIGHT) / 2 + 50 + BUTTON_HEIGHT + 50);
	}
	if (!gButtonSpriteSheetTexture[EXIT2_BUTTON].loadFromFile("media/image/exit.png")) {
		cout << "Failed to load button sprite texture!\n";
		success = false;
	}
	else {
		for (int i = 0; i < BUTTON_SPRITE_TOTAL; ++i) {
			gSpriteClips[i].x = 0;
			gSpriteClips[i].y = i * BUTTON_HEIGHT;
			gSpriteClips[i].w = BUTTON_WIDTH;
			gSpriteClips[i].h = BUTTON_HEIGHT;
		}
		gButton[EXIT2_BUTTON].setPosition((SCREEN_WIDTH - BUTTON_WIDTH) / 2, (SCREEN_HEIGHT - BUTTON_HEIGHT) / 2 + 50 + BUTTON_HEIGHT + 50);
	}
	if (!gButtonSpriteSheetTexture[PLAY_AGAIN_BUTTON].loadFromFile("media/image/menu.png")) {
		cout << "Failed to load button sprite texture!\n";
		success = false;
	}
	else {
		for (int i = 0; i < BUTTON_SPRITE_TOTAL; ++i) {
			gSpriteClips[i].x = 0;
			gSpriteClips[i].y = i * BUTTON_HEIGHT;
			gSpriteClips[i].w = BUTTON_WIDTH;
			gSpriteClips[i].h = BUTTON_HEIGHT;
		}
		gButton[PLAY_AGAIN_BUTTON].setPosition((SCREEN_WIDTH - BUTTON_WIDTH) / 2, (SCREEN_HEIGHT - BUTTON_HEIGHT) / 2 + 70);
	}
	gBackgroundMusic = Mix_LoadMUS("media/music/background.wav");
	if (gBackgroundMusic == NULL) {
		cout << "Failed to load background music!\n";
		success = false;
	}
	gMove = Mix_LoadWAV("media/music/move.wav");
	if (gMove == NULL) {
		cout << "Failed to load move sound!\n";
		cout << Mix_GetError() << "\n";
		success = false;
	}
	gError = Mix_LoadWAV("media/music/error.wav");
	if (gError == NULL) {
		cout << "Failed to load error sound!\n";
		success = false;
	}
	gGameOver = Mix_LoadWAV("media/music/lose.wav");
	if (gGameOver == NULL) {
		cout << "Failed to load game over sound!\n";
		success = false;
	}
	gWinSound = Mix_LoadWAV("media/music/win.wav");
	if (gWinSound == NULL) {
		cout << "Failed to load winning sound!\n";
		success = false;
	}
	return success;

}
void close() {
	gBoard.free();
	g2Tile.free();
	g4Tile.free();
	g8Tile.free();
	g16Tile.free();
	g32Tile.free();
	g64Tile.free();
	g128Tile.free();
	g256Tile.free();
	g512Tile.free();
	g1024Tile.free();
	g2048Tile.free();
	gBlankTile.free();
	gBackground.free();
	for (int i = 0; i < 5; ++i) {
		gTopScoreTexture[i].free();
	}
	gScoreTexture.free();
	gHighScoreTexture.free();
	gScoreBoardTexture.free();
	for (int i = START_BUTTON; i < TOTAL_BUTTONS; ++i) {
		gButtonSpriteSheetTexture[i].free();
	}
	Mix_FreeMusic(gBackgroundMusic);
	gBackgroundMusic = NULL;
	Mix_FreeChunk(gMove);
	Mix_FreeChunk(gError);
	Mix_FreeChunk(gWinSound);
	Mix_FreeChunk(gGameOver);
	gWinSound = NULL;
	gGameOver = NULL;
	gMove = NULL;
	gError = NULL;
	TTF_CloseFont(gFont);
	TTF_CloseFont(gTopScoreFont);
	gFont = NULL;
	gTopScoreFont = NULL;
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gRenderer = NULL;
	gWindow = NULL;
	IMG_Quit();
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
}
int** createBoard() {
	int** board;
	srand(time(NULL));
	board = new int* [SIZE];
	for (int i = 0; i < SIZE; ++i) {
		board[i] = new int[SIZE];
	}
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			board[i][j] = 0;
		}
	}
	int x = rand() % SIZE;
	int y = rand() % SIZE;
	board[x][y] = 2;
	x = rand() % SIZE;
	y = rand() % SIZE;
	board[x][y] = 2;
	return board;
}
void rushTop(int**& board) {
	for (int i = 1; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			if (board[i][j] != 0) {
				for (int k = i; k > 0; --k) {
					if (board[k - 1][j] == 0) {
						board[k - 1][j] = board[k][j];
						board[k][j] = 0;
					}
				}
			}
		}
	}
}
void moveTop(int**& board, int& score) {
	rushTop(board);
	for (int i = 1; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			if (board[i][j] != 0) {
				if (board[i - 1][j] == board[i][j]) {
					board[i - 1][j] += board[i][j];
					score += board[i - 1][j];
					board[i][j] = 0;
				}
			}
		}
	}
	rushTop(board);
}
void rushBot(int**& board) {
	for (int i = 0; i < SIZE; ++i) {
		for (int j = SIZE - 2; j >= 0; --j) {
			if (board[j][i] != 0) {
				for (int k = j; k < SIZE - 1; ++k) {
					if (board[k + 1][i] == 0) {
						board[k + 1][i] = board[k][i];
						board[k][i] = 0;
					}
				}
			}
		}
	}
}
void moveBot(int**& board, int& score) {
	rushBot(board);
	for (int i = 0; i < SIZE; ++i) {
		for (int j = SIZE - 2; j >= 0; --j) {
			if (board[j][i] != 0) {
				if (board[j][i] == board[j + 1][i]) {
					board[j + 1][i] += board[j][i];
					score += board[j + 1][i];
					board[j][i] = 0;
				}
			}
		}
	}
	rushBot(board);
}
void rushRight(int**& board) {
	for (int i = 0; i < SIZE; ++i) {
		for (int j = SIZE - 2; j >= 0; --j) {
			if (board[i][j] != 0) {
				for (int k = j; k < SIZE - 1; ++k) {
					if (board[i][k + 1] == 0) {
						board[i][k + 1] = board[i][k];
						board[i][k] = 0;
					}
				}
			}
		}
	}
}
void moveRight(int**& board, int& score) {
	rushRight(board);
	for (int i = 0; i < SIZE; ++i) {
		for (int j = SIZE - 2; j >= 0; --j) {
			if (board[i][j] != 0) {
				if (board[i][j] == board[i][j + 1]) {
					board[i][j + 1] += board[i][j];
					score += board[i][j + 1];
					board[i][j] = 0;
				}
			}
		}
	}
	rushRight(board);
}
void rushLeft(int**& board) {
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 1; j < SIZE; ++j) {
			if (board[i][j] != 0) {
				for (int k = j; k > 0; --k) {
					if (board[i][k - 1] == 0) {
						board[i][k - 1] = board[i][k];
						board[i][k] = 0;
					}
				}
			}
		}
	}
}
void moveLeft(int**& board, int& score) {
	rushLeft(board);
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 1; j < SIZE; ++j) {
			if (board[i][j] != 0) {
				if (board[i][j] == board[i][j - 1]) {
					board[i][j - 1] += board[i][j];
					score += board[i][j - 1];
					board[i][j] = 0;
				}
			}
		}
	}
	rushLeft(board);
}
bool checkWin(int**& board) {
	bool isWin = false;
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			if (board[i][j] == WIN) {
				isWin = true;
				break;
			}
		}
	}
	return isWin;
}
bool checkPut(int**& board) {
	bool canPut = false;
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			if (board[i][j] == 0) {
				canPut = true;
				break;
			}
		}
	}
	return canPut;
}
bool checkLose(int**& board) {
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE - 1; ++j) {
			if (board[i][j] == board[i][j + 1]) {
				return false;
			}
		}
	}
	for (int i = 0; i < SIZE - 1; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			if (board[i][j] == board[i + 1][j]) {
				return false;
			}
		}
	}
	if (checkPut(board)) {
		return false;
	}
	return true;
}
bool checkToRandom(int**& board, int**& backBoard) {
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			if (board[i][j] != backBoard[i][j]) {
				return true;
			}
		}
	}
	return false;
}
void randomOnBoard(int**& board) {
	if (checkPut(board)) {
		int values[10] = { 2,2,2,2,2,2,2,2,4,2 };
		int value = values[rand() % 10];
		int x = rand() % SIZE;
		int y = rand() % SIZE;
		if (board[x][y] == 0) {
			board[x][y] = 2;
		}
		else {
			while (board[x][y] != 0) {
				x = rand() % SIZE;
				y = rand() % SIZE;
			}
			board[x][y] = value;
		}
	}
}
void copyBoard(int**& board1, int**& board2) {
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			board1[i][j] = board2[i][j];
		}
	}
}
void print(int**& board, int& score) {
	cout << "Score: " << score << "\n";
	for (int i = 0; i < SIZE; ++i) {
		for (int j = 0; j < SIZE; ++j) {
			cout << board[i][j] << " ";
		}
		cout << "\n";
	}
}
void LButton::setPosition(int x, int y) {
	mPosition.x = x;
	mPosition.y = y;
}
int LButton::handleEvent(SDL_Event* e, int i, bool& getStart, bool& quit) {
	//If mouse event happened
	if (i == START_BUTTON) {
		if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
			//Get mouse position
			int x, y;
			SDL_GetMouseState(&x, &y);
			bool inside = true;
			if (x < mPosition.x) {
				inside = false;
			}
			else if (x > mPosition.x + BUTTON_WIDTH) {
				inside = false;
			}
			else if (y < mPosition.y) {
				inside = false;
			}
			else if (y > mPosition.y + BUTTON_HEIGHT) {
				inside = false;
			}
			if (!inside) {
				mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
			}
			else {
				switch (e->type)
				{
				case SDL_MOUSEMOTION:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
					break;
				case SDL_MOUSEBUTTONDOWN:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
					cout << "You've pressed Start!\n";
					getStart = true;
					return START_BUTTON;
					break;
				case SDL_MOUSEBUTTONUP:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
					break;
				default:
					break;
				}
			}
		}
	}
	if (i == PLAY_AGAIN_BUTTON) {
		if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
			//Get mouse position
			int x, y;
			SDL_GetMouseState(&x, &y);
			bool inside = true;
			if (x < mPosition.x) {
				inside = false;
			}
			else if (x > mPosition.x + BUTTON_WIDTH) {
				inside = false;
			}
			else if (y < mPosition.y) {
				inside = false;
			}
			else if (y > mPosition.y + BUTTON_HEIGHT) {
				inside = false;
			}
			if (!inside) {
				mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
			}
			else {
				switch (e->type)
				{
				case SDL_MOUSEMOTION:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
					break;
				case SDL_MOUSEBUTTONDOWN:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
					cout << "You've pressed play again!\n";
					return PLAY_AGAIN_BUTTON;
					break;
				case SDL_MOUSEBUTTONUP:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
					break;
				default:
					break;
				}
			}
		}
	}
	if (i == EXIT1_BUTTON || i == EXIT2_BUTTON) {
		if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
			//Get mouse position
			int x, y;
			SDL_GetMouseState(&x, &y);
			bool inside = true;
			if (x < mPosition.x) {
				inside = false;
			}
			else if (x > mPosition.x + BUTTON_WIDTH) {
				inside = false;
			}
			else if (y < mPosition.y) {
				inside = false;
			}
			else if (y > mPosition.y + BUTTON_HEIGHT) {
				inside = false;
			}
			if (!inside) {
				mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
			}
			else {
				switch (e->type)
				{
				case SDL_MOUSEMOTION:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
					break;
				case SDL_MOUSEBUTTONDOWN:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
					cout << "You've pressed Exit!\n";
					break;
				case SDL_MOUSEBUTTONUP:
					mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
					quit = true;
					return i;
					break;
				default:
					break;
				}
			}
		}
	}
}
void LButton::render(int i) {
	//Show current button sprite
	gButtonSpriteSheetTexture[i].render(mPosition.x, mPosition.y, &gSpriteClips[mCurrentSprite]);
}
void playGame(SDL_Event* e, bool& quit) {
	int score = 0;
	int** board = createBoard();
	int** backBoard = createBoard();
	int backScore = 0;
	bool getStart = false;
	bool getPlayAgain = false;
	bool backMenu = false;
	vector<int> scoreArr;
	ifstream input("score.txt");
	fstream output;
	output.open("score.txt", ios::out | ios::app);
	while (!input.eof()) {
		int number;
		if (input >> number) { scoreArr.push_back(number); }
	}
	sort(scoreArr.begin(), scoreArr.end(), greater<int>());
	int highScore = scoreArr[0];
	while (!getStart && !quit) {
		while (SDL_PollEvent(e) != 0) {
			if (e->type == SDL_QUIT) {
				quit = true;
			}
			else {
				SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
				SDL_RenderClear(gRenderer);
				gBackground.render(0, 0);
				SDL_Color textColor = { 0,0,0 };
				gScoreBoardTexture.loadFromTopScoreText("High score board:", textColor);
				gScoreBoardTexture.render((SCREEN_WIDTH - BUTTON_WIDTH) / 8 + BUTTON_WIDTH + 40, SCREEN_HEIGHT / 2 - 80);
				for (int i = 0; i < 5; ++i) {
					gTopScoreTexture[i].loadFromTopScoreText(to_string(i + 1) + ", " + to_string(scoreArr[i]) + "\n", textColor);
					gTopScoreTexture[i].render((SCREEN_WIDTH - BUTTON_WIDTH) / 8 + BUTTON_WIDTH + 125, SCREEN_HEIGHT / 2 - 10 + 10 * i + gTopScoreTexture[i].getHeight() * i);
				}
				if (e->key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
				}
				for (int i = 0; i < TOTAL_BUTTONS; ++i) {
					if (i != PLAY_AGAIN_BUTTON && i != EXIT2_BUTTON) 
						gButton[i].handleEvent(e, i, getStart, quit);
				}
			}
		}
		for (int i = 0; i < TOTAL_BUTTONS; ++i) {
			if (i != PLAY_AGAIN_BUTTON && i != EXIT2_BUTTON)
				gButton[i].render(i);
		}
		SDL_RenderPresent(gRenderer);
	}
	SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
	SDL_RenderClear(gRenderer);
	if (Mix_PlayingMusic() != 1) {
		Mix_PlayMusic(gBackgroundMusic, -1);
	}
	else if (Mix_PausedMusic() == 1) {
		Mix_ResumeMusic();
	}
	while (!quit) {
		if (getStart && !checkLose(board) && !checkWin(board)) {
			while (SDL_PollEvent(e) != 0) {
				if (e->type == SDL_QUIT) {
					quit = true;
				}
				else {
					if (e->type == SDL_MOUSEBUTTONUP || e->type==SDL_KEYDOWN) {
						print(board, score);
						SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
						SDL_RenderClear(gRenderer);
						gBoard.render((SCREEN_WIDTH - gBoard.getWidth()) / 2, (SCREEN_HEIGHT - gBoard.getHeight()) / 2);
						copyBoard(backBoard, board);
						if (backMenu) playGame(e, quit);
						if (e->type == SDL_KEYDOWN) {
							switch (e->key.keysym.sym) {
							case SDLK_UP:
								moveTop(board, score);
								break;
							case SDLK_DOWN:
								moveBot(board, score);
								break;
							case SDLK_RIGHT:
								moveRight(board, score);
								break;
							case SDLK_LEFT:
								moveLeft(board, score);
								break;
							case SDLK_ESCAPE:
								quit = true;
								break;
							default:
								break;
							}
							if (checkToRandom(board, backBoard)) {
								randomOnBoard(board);
								Mix_PlayChannel(-1, gMove, 0);
							}
							else {
								Mix_PlayChannel(-1, gError, 0);
							}
						}
						for (int i = 0; i < SIZE; ++i) {
							for (int j = 0; j < SIZE; ++j) {
								if (board[i][j] == 0) {
									gBlankTile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + gBlankTile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + gBlankTile.getWidth() * i);
								}
								if (board[i][j] == 2) {
									g2Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g2Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g2Tile.getWidth() * i);
								}
								if (board[i][j] == 4) {
									g4Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g4Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g4Tile.getWidth() * i);
								}
								if (board[i][j] == 8) {
									g8Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g8Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g8Tile.getWidth() * i);
								}
								if (board[i][j] == 16) {
									g16Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g16Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g16Tile.getWidth() * i);
								}
								if (board[i][j] == 32) {
									g32Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g32Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g32Tile.getWidth() * i);
								}
								if (board[i][j] == 64) {
									g64Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g64Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g64Tile.getWidth() * i);
								}
								if (board[i][j] == 128) {
									g128Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g128Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g128Tile.getWidth() * i);
								}
								if (board[i][j] == 256) {
									g256Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g256Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g256Tile.getWidth() * i);
								}
								if (board[i][j] == 512) {
									g512Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g512Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g512Tile.getWidth() * i);
								}
								if (board[i][j] == 1024) {
									g1024Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g1024Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g1024Tile.getWidth() * i);
								}
								if (board[i][j] == 2048) {
									g2048Tile.render(19 * (j + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g2048Tile.getWidth() * j, 19 * (i + 1) + (SCREEN_WIDTH - gBoard.getWidth()) / 2 + g2048Tile.getWidth() * i);
								}
							}
						}
						if (highScore < score) highScore = score;
						SDL_Color textColor = { 0,0,0 };
						string highScoreString = to_string(highScore);
						gHighScoreTexture.loadFromRenderedText("best score: " + highScoreString, textColor);
						gHighScoreTexture.render(SCREEN_WIDTH * 1 / 5, SCREEN_HEIGHT * 1 / 18);
						textColor = { 0,0,0 };
						string scoreString = to_string(score);
						gScoreTexture.loadFromRenderedText("score: " + scoreString, textColor);
						gScoreTexture.render(SCREEN_WIDTH * 3 / 5, SCREEN_HEIGHT * 1 / 18);
						SDL_RenderPresent(gRenderer);
					}
				}
			}
		}
		else if (checkWin(board)) {
			scoreArr.push_back(score);
			if (Mix_PlayingMusic() == 1) {
				Mix_PauseMusic();
			}
			Mix_PlayChannel(-1, gWinSound, 0);
			while (!quit) {
				while (SDL_PollEvent(e)) {
					SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
					SDL_RenderClear(gRenderer);
					gWin.render((SCREEN_WIDTH - gWin.getWidth()) / 2, (SCREEN_HEIGHT - gWin.getHeight()) / 2);
					if (gButton[PLAY_AGAIN_BUTTON].handleEvent(e, PLAY_AGAIN_BUTTON, getStart, quit) == PLAY_AGAIN_BUTTON) {
						getPlayAgain = true;
					}
					gButton[EXIT2_BUTTON].handleEvent(e, EXIT2_BUTTON, getStart, quit);
					gButton[EXIT2_BUTTON].render(EXIT2_BUTTON);
					gButton[PLAY_AGAIN_BUTTON].render(PLAY_AGAIN_BUTTON);
					SDL_RenderPresent(gRenderer);
					if (getPlayAgain) playGame(e, quit);
					switch (e->type) {
					case SDL_QUIT:
						quit = true;
						break;
					case SDL_KEYDOWN:
						if (e->key.keysym.sym == SDLK_ESCAPE) quit = true;
					}
				}
			}

		}
		else if (checkLose(board)) {
			scoreArr.push_back(score);
			if (Mix_PlayingMusic() == 1) {
				Mix_PauseMusic();
			}
			Mix_PlayChannel(-1, gGameOver, 0);
			while (!quit) {
				while (SDL_PollEvent(e)) {
					SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
					SDL_RenderClear(gRenderer);
					gLose.render((SCREEN_WIDTH - gLose.getWidth()) / 2, (SCREEN_HEIGHT - gLose.getHeight()) / 2);
					if (gButton[PLAY_AGAIN_BUTTON].handleEvent(e, PLAY_AGAIN_BUTTON, getStart, quit) == PLAY_AGAIN_BUTTON) {
						getPlayAgain = true;
					}
					gButton[EXIT2_BUTTON].handleEvent(e, EXIT2_BUTTON, getStart, quit);
					gButton[EXIT2_BUTTON].render(EXIT2_BUTTON);
					gButton[PLAY_AGAIN_BUTTON].render(PLAY_AGAIN_BUTTON);
					SDL_RenderPresent(gRenderer);
					if (getPlayAgain) playGame(e, quit);
					switch (e->type) {
					case SDL_QUIT:
						quit = true;
						break;
					case SDL_KEYDOWN:
						if (e->key.keysym.sym == SDLK_ESCAPE) quit = true;
					}
				}
			}
		}
	}
	for (int i = 0; i < SIZE; ++i) {
		delete[] board[i];
	}
	delete[] board;
	output << score << "\n";
	input.close();
	output.close();
}
bool LTexture::loadFromTopScoreText(string textureText, SDL_Color textColor) {
	free();
	SDL_Surface* textSurface = TTF_RenderText_Solid(gTopScoreFont, textureText.c_str(), textColor);
	if (textSurface == NULL) {
		cout << "Unable to render surface!\n";
	}
	else {
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL) {
			cout << "Unable to create texture from rendered text!\n";
		}
		else {
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}
		SDL_FreeSurface(textSurface);
	}
	return mTexture != NULL;
}