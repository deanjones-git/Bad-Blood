// Dean Jones
// 005-299-127

// vampires.cpp

// Portions you are to complete are marked with a TODO: comment.
// We've provided some incorrect return statements (so indicated) just
// to allow this skeleton program to compile and run, albeit incorrectly.
// The first thing you probably want to do is implement the utterly trivial
// functions (marked TRIVIAL).  Then get Arena::display going.  That gives
// you more flexibility in the order you tackle the rest of the functionality.
// As you finish implementing each TODO: item, remove its TODO: comment.

#include <iostream>
#include <string>
#include <random>
#include <utility>
#include <cstdlib>
#include <cctype>
#include <type_traits>
#include <cassert>
using namespace std;

///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 20;                // max number of rows in the arena
const int MAXCOLS = 20;                // max number of columns in the arena
const int MAXVAMPIRES = 100;           // max number of vampires allowed

const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;
const int NUMDIRS = 4;

const int EMPTY = 0;
const int HAS_POISON = 1;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////

class Arena;  // This is needed to let the compiler know that Arena is a
			  // type name, since it's mentioned in the Vampire declaration.

class Vampire
{
public:
	// Constructor
	Vampire(Arena* ap, int r, int c);

	// Accessors
	int  row() const;
	int  col() const;
	bool isDead() const;

	// Mutators
	void move();

private:
	Arena* m_arena;
	int    m_row;
	int    m_col;
	int    m_poisoned; // tracks the number of times the vampire has been poisoned
	bool   m_justMoved; // tracks whether the vampire just moved for poisoned vampire
						// movement frequency
};

class Player
{
public:
	// Constructor
	Player(Arena* ap, int r, int c);

	// Accessors
	int  row() const;
	int  col() const;
	bool isDead() const;

	// Mutators
	string dropPoisonVial();
	string move(int dir);
	void   setDead();

private:
	Arena* m_arena;
	int    m_row;
	int    m_col;
	bool   m_dead;
};

class Arena
{
public:
	// Constructor/destructor
	Arena(int nRows, int nCols);
	~Arena();

	// Accessors
	int     rows() const;
	int     cols() const;
	Player* player() const;
	int     vampireCount() const;
	int     getCellStatus(int r, int c) const;
	int     numberOfVampiresAt(int r, int c) const;
	void    display(string msg) const;

	// Mutators
	void setCellStatus(int r, int c, int status);
	bool addVampire(int r, int c);
	bool addPlayer(int r, int c);
	void moveVampires();

private:
	int      m_grid[MAXROWS][MAXCOLS];
	int      m_rows;
	int      m_cols;
	Player* m_player;
	Vampire* m_vampires[MAXVAMPIRES];
	int      m_nVampires;
	int      m_turns;

	// Helper functions
	void checkPos(int r, int c, string functionName) const;
	bool isPosInBounds(int r, int c) const;
};

class Game
{
public:
	// Constructor/destructor
	Game(int rows, int cols, int nVampires);
	~Game();

	// Mutators
	void play();

private:
	Arena* m_arena;

	// Helper functions
	string takePlayerTurn();
};

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////

int randInt(int lowest, int highest);
bool decodeDirection(char ch, int& dir);
bool attemptMove(const Arena& a, int dir, int& r, int& c);
bool recommendMove(const Arena& a, int r, int c, int& bestDir);
void clearScreen();

///////////////////////////////////////////////////////////////////////////
//  Vampire implementation
///////////////////////////////////////////////////////////////////////////

Vampire::Vampire(Arena* ap, int r, int c)
{
	if (ap == nullptr)
	{
		cout << "***** A vampire must be created in some Arena!" << endl;
		exit(1);
	}
	if (r < 1 || r > ap->rows() || c < 1 || c > ap->cols())
	{
		cout << "***** Vampire created with invalid coordinates (" << r << ","
			<< c << ")!" << endl;
		exit(1);
	}
	m_arena = ap;
	m_row = r;
	m_col = c;
	m_poisoned = 0;
	m_justMoved = false;
}

int Vampire::row() const
{
	return m_row;
}

int Vampire::col() const
{
	return m_col;
}

bool Vampire::isDead() const
{
	// Return whether the Vampire is dead (has been poisoned twice)
	if (m_poisoned == 2)
		return true;
	return false;
}

void Vampire::move()
{
	//   Return without moving if the vampire has drunk one vial of
	//   poisoned blood (so is supposed to move only every other turn) and
	//   this is a turn it does not move.

	// set m_justMoved to false so the next turn the poisoned vampire can
	// attempt to move
	if (m_poisoned == 1 && m_justMoved) {
		m_justMoved = false;
		return;
	}
	//   Otherwise, attempt to move in a random direction; if can't
	//   move, don't move.  If it lands on a poisoned blood vial, drink all
	//   the blood in the vial and remove it from the game (so it is no
	//   longer on that grid point).

	//   Pick the random direction and attempt the move. Vampire will move if it's valid
	//   and not move if it isn't. m_justMoved should be set to true.
	int dir = randInt(0, NUMDIRS-1);
	attemptMove(*m_arena, dir, m_row, m_col);
	m_justMoved = true; 

	// If the cell was poisoned, increased the vampire's m_poisoned and remove the vial from the game
	if (m_arena->getCellStatus(m_row, m_col) == HAS_POISON) {
		m_poisoned++;
		m_arena->setCellStatus(m_row, m_col, EMPTY);
	}
}

///////////////////////////////////////////////////////////////////////////
//  Player implementation
///////////////////////////////////////////////////////////////////////////

Player::Player(Arena* ap, int r, int c)
{
	if (ap == nullptr)
	{
		cout << "***** The player must be created in some Arena!" << endl;
		exit(1);
	}
	if (r < 1 || r > ap->rows() || c < 1 || c > ap->cols())
	{
		cout << "**** Player created with invalid coordinates (" << r
			<< "," << c << ")!" << endl;
		exit(1);
	}
	m_arena = ap;
	m_row = r;
	m_col = c;
	m_dead = false;
}

int Player::row() const
{
	return m_row;
}

int Player::col() const
{
	return m_col;
}

string Player::dropPoisonVial()
{
	if (m_arena->getCellStatus(m_row, m_col) == HAS_POISON)
		return "There's already a poisoned blood vial at this spot.";
	m_arena->setCellStatus(m_row, m_col, HAS_POISON);
	return "A poisoned blood vial has been dropped.";
}

string Player::move(int dir)
{
	// Attempt to move the player one step in the indicated
	// direction.  If this fails,
	// return "Player couldn't move; player stands."
	if (!attemptMove(*m_arena, dir, m_row, m_col))
		return "Player couldn't move; player stands.";
	// A player who moves onto a vampire dies, and this
	// returns "Player walked into a vampire and died."
	if (m_arena->numberOfVampiresAt(m_row, m_col) > 0) {
		setDead();
		return "Player walked into a vampire and died.";
	}
	// Otherwise, return one of "Player moved north.",
	// "Player moved east.", "Player moved south.", or
	// "Player moved west."
	switch (dir) {
		case NORTH:
			return "Player moved north.";
			break;
		case EAST:
			return "Player moved east.";
			break;
		case SOUTH:
			return "Player moved south.";
			break;
		case WEST:
			return "Player moved west.";
			break;
		}
	// This should never be returned
	return "";
}

bool Player::isDead() const
{
	// Return whether the Player is dead
	return m_dead;
}

void Player::setDead()
{
	m_dead = true;
}

///////////////////////////////////////////////////////////////////////////
//  Arena implementation
///////////////////////////////////////////////////////////////////////////

Arena::Arena(int nRows, int nCols)
{
	if (nRows <= 0 || nCols <= 0 || nRows > MAXROWS || nCols > MAXCOLS)
	{
		cout << "***** Arena created with invalid size " << nRows << " by "
			<< nCols << "!" << endl;
		exit(1);
	}
	m_rows = nRows;
	m_cols = nCols;
	m_player = nullptr;
	m_nVampires = 0;
	m_turns = 0;
	for (int r = 1; r <= m_rows; r++)
		for (int c = 1; c <= m_cols; c++)
			setCellStatus(r, c, EMPTY);
}

Arena::~Arena()
{
	// Deallocate the player
	delete m_player;
	// and all remaining dynamically allocated vampires.
	for (int vampire = 0; vampire < vampireCount(); vampire++)
		delete m_vampires[vampire];
}

int Arena::rows() const
{
	return m_rows;
}

int Arena::cols() const
{
	return m_cols;
}

Player* Arena::player() const
{
	return m_player;
}

int Arena::vampireCount() const
{
	return m_nVampires;
}

int Arena::getCellStatus(int r, int c) const
{
	checkPos(r, c, "Arena::getCellStatus");
	return m_grid[r - 1][c - 1];
}

int Arena::numberOfVampiresAt(int r, int c) const
{
	int vampires = 0;
	for (int vampire = 0; vampire < vampireCount(); vampire++) {
		if (m_vampires[vampire]->row() == r && m_vampires[vampire]->col() == c)
			vampires++;
	}
	return vampires;
}

void Arena::display(string msg) const
{
	char displayGrid[MAXROWS][MAXCOLS];
	int r, c;

	// Fill displayGrid with dots (empty) and stars (poisoned blood vials)
	for (r = 1; r <= rows(); r++)
		for (c = 1; c <= cols(); c++)
			displayGrid[r - 1][c - 1] = (getCellStatus(r, c) == EMPTY ? '.' : '*');

	// Indicate each vampire's position
	for (int vampire = 0; vampire < vampireCount(); vampire++) {
		// Get number of vampires at each vampire's position
		int vRow = m_vampires[vampire]->row();
		int vCol = m_vampires[vampire]->col();
		int numVampires = numberOfVampiresAt(vRow, vCol);
		// If one vampire is at some grid point, set the displayGrid char to 'V'.
		if (numVampires == 1) {
			displayGrid[vRow - 1][vCol - 1] = 'V';
		}
		// If it's 2 though 8, set it to '2' through '8'.
		// For 9 or more, set it to '9'.
		else {
			displayGrid[vRow - 1][vCol - 1] = (numVampires < 10 ? '0' + numVampires : '9');
		}
	}

	// Indicate player's position
	if (m_player != nullptr)
		displayGrid[m_player->row() - 1][m_player->col() - 1] = (m_player->isDead() ? 'X' : '@');

	// Draw the grid
	clearScreen();

	for (r = 1; r <= rows(); r++)
	{
		for (c = 1; c <= cols(); c++)
			cout << displayGrid[r - 1][c - 1];
		cout << endl;
	}
	cout << endl;

	// Write message, vampire, and player info
	if (msg != "")
		cout << msg << endl;
	if (vampireCount() == 1)
		cout << "There is 1 vampire remaining." << endl;
	else
		cout << "There are " << vampireCount() << " vampires remaining." << endl;
	if (m_player == nullptr)
		cout << "There is no player!" << endl;
	else if (m_player->isDead())
		cout << "The player is dead." << endl;
	if (m_turns == 1)
		cout << "1 turn has been taken." << endl;
	else
		cout << m_turns << " turns have been taken." << endl;
}

void Arena::setCellStatus(int r, int c, int status)
{
	checkPos(r, c, "Arena::setCellStatus");
	m_grid[r - 1][c - 1] = status;
}

bool Arena::addVampire(int r, int c)
{
	if (!isPosInBounds(r, c))
		return false;

	// Don't add a vampire on a spot with a poisoned blood vial
	if (getCellStatus(r, c) != EMPTY)
		return false;

	// Don't add a vampire on a spot with a player
	if (m_player != nullptr && m_player->row() == r && m_player->col() == c)
		return false;

	// If there are MAXVAMPIRES existing vampires, return false.  Otherwise,
	// dynamically allocate a new vampire at coordinates (r,c).  Save the
	// pointer to newly allocated vampire and return true.
	if (vampireCount() == MAXVAMPIRES)
		return false;

	// If the position passes all the tests (not out of bounds, on a poisoned
	// blood vial, or on the player), add it to the array, increase the vampire
	// count, and return true
	m_vampires[vampireCount()] = new Vampire(this, r, c);
	m_nVampires++;
	return true;
}

bool Arena::addPlayer(int r, int c)
{
	if (!isPosInBounds(r, c))
		return false;

	// Don't add a player if one already exists
	if (m_player != nullptr)
		return false;

	// Don't add a player on a spot with a vampire
	if (numberOfVampiresAt(r, c) > 0)
		return false;

	m_player = new Player(this, r, c);
	return true;
}

void Arena::moveVampires()
{
	// Move all vampires
	// Move each vampire.  Mark the player as dead if necessary.
	// Deallocate any dead dynamically allocated vampire.
	int deadVampires = 0;
	// Iterate through vampire array
	for (int vampire = 0; vampire < vampireCount(); vampire++) {
		// Move this vampire
		m_vampires[vampire]->move();
		// If the vampire moves to the player's position, the player is dead
		if (m_player->row() == m_vampires[vampire]->row() && m_player->col() == m_vampires[vampire]->col())
			m_player->setDead();
		// If the vampire moves to the position of a blood vial and was
		// already poisoned, the vampire is dead. Increase dead vampire
		// count, deallocate dead vampire object, and go to the next iteration
		if (m_vampires[vampire]->isDead()) {
			deadVampires++;
			delete(m_vampires[vampire]);
			continue;
		}
		// When vampires start to die, alive vampires should be copied
		// to the position the dead vampire used to occupy
		m_vampires[vampire - deadVampires] = m_vampires[vampire];
	}
	// Decrease dead vampire count from number of vampires in arena
	m_nVampires -= deadVampires;
	// Another turn has been taken
	m_turns++;
}

bool Arena::isPosInBounds(int r, int c) const
{
	return (r >= 1 && r <= m_rows && c >= 1 && c <= m_cols);
}

void Arena::checkPos(int r, int c, string functionName) const
{
	if (r < 1 || r > m_rows || c < 1 || c > m_cols)
	{
		cout << "***** " << "Invalid arena position (" << r << ","
			<< c << ") in call to " << functionName << endl;
		exit(1);
	}
}

///////////////////////////////////////////////////////////////////////////
//  Game implementation
///////////////////////////////////////////////////////////////////////////

Game::Game(int rows, int cols, int nVampires)
{
	if (nVampires < 0)
	{
		cout << "***** Cannot create Game with negative number of vampires!" << endl;
		exit(1);
	}
	if (nVampires > MAXVAMPIRES)
	{
		cout << "***** Trying to create Game with " << nVampires
			<< " vampires; only " << MAXVAMPIRES << " are allowed!" << endl;
		exit(1);
	}
	int nEmpty = rows * cols - nVampires - 1;  // 1 for Player
	if (nEmpty < 0)
	{
		cout << "***** Game created with a " << rows << " by "
			<< cols << " arena, which is too small too hold a player and "
			<< nVampires << " vampires!" << endl;
		exit(1);
	}

	// Create arena
	m_arena = new Arena(rows, cols);

	// Add player
	int rPlayer;
	int cPlayer;
	do
	{
		rPlayer = randInt(1, rows);
		cPlayer = randInt(1, cols);
	} while (m_arena->getCellStatus(rPlayer, cPlayer) != EMPTY);
	m_arena->addPlayer(rPlayer, cPlayer);

	// Populate with vampires
	while (nVampires > 0)
	{
		int r = randInt(1, rows);
		int c = randInt(1, cols);
		if (r == rPlayer && c == cPlayer)
			continue;
		m_arena->addVampire(r, c);
		nVampires--;
	}
}

Game::~Game()
{
	delete m_arena;
}

string Game::takePlayerTurn()
{
	for (;;)
	{
		cout << "Your move (n/e/s/w/x or nothing): ";
		string playerMove;
		getline(cin, playerMove);

		Player* player = m_arena->player();
		int dir;

		if (playerMove.size() == 0)
		{
			if (recommendMove(*m_arena, player->row(), player->col(), dir))
				return player->move(dir);
			else
				return player->dropPoisonVial();
		}
		else if (playerMove.size() == 1)
		{
			if (tolower(playerMove[0]) == 'x')
				return player->dropPoisonVial();
			else if (decodeDirection(playerMove[0], dir))
				return player->move(dir);
		}
		cout << "Player move must be nothing, or 1 character n/e/s/w/x." << endl;
	}
}

void Game::play()
{
	m_arena->display("");
	Player* player = m_arena->player();
	if (player == nullptr)
		return;
	while (!player->isDead() && m_arena->vampireCount() > 0)
	{
		string msg = takePlayerTurn();
		m_arena->display(msg);
		if (player->isDead())
			break;
		m_arena->moveVampires();
		m_arena->display(msg);
	}
	if (player->isDead())
		cout << "You lose." << endl;
	else
		cout << "You win." << endl;
}

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function implementation
///////////////////////////////////////////////////////////////////////////

  // Return a uniformly distributed random int from lowest to highest, inclusive
int randInt(int lowest, int highest)
{
	if (highest < lowest)
		swap(highest, lowest);
	static random_device rd;
	static default_random_engine generator(rd());
	uniform_int_distribution<> distro(lowest, highest);
	return distro(generator);
}

bool decodeDirection(char ch, int& dir)
{
	switch (tolower(ch))
	{
	default:  return false;
	case 'n': dir = NORTH; break;
	case 'e': dir = EAST;  break;
	case 's': dir = SOUTH; break;
	case 'w': dir = WEST;  break;
	}
	return true;
}

// Return false without changing anything if moving one step from (r,c)
// in the indicated direction would run off the edge of the arena.
// Otherwise, update r and c to the position resulting from the move and
// return true.
bool attemptMove(const Arena& a, int dir, int& r, int& c)
{
	// Handle different directions with switch statement
	switch (dir) {
	case NORTH:
		// If the row is 1, the player/vampire is at the top of
		// the arena and shouldn't move. Else, move north.
		if (r != 1) {
			r--;
			return true;
		}
		break;
	case EAST:
		// If the col equals the arena's column number, the player/
		// vampire is at the right of the arena and shouldn't move.
		// Else, move east.
		if (c != a.cols()) {
			c++;
			return true;
		}
		break;
	case SOUTH:
		// If the row equals the arena's row number, the player/
		// vampire is at the bottom of the arena and shouldn't move.
		// Else, move south.
		if (r != a.rows()) {
			r++;
			return true;
		}
		break;
	case WEST:
		// If the column is 1, the player/vampire is at the left
		// of the arena and shouldn't move. Else, move west.
		if (c != 1) {
			c--;
			return true;
		}
		break;
	}
	// If the move wasn't possible, return false
	return false;
}

// Recommend a move for a player at (r,c):  A false return means the
// recommendation is that the player should drop a poisoned blood vial and
// not move; otherwise, this function sets bestDir to the recommended
// direction to move and returns true.

// Your replacement implementation should do something intelligent.
// You don't have to be any smarter than the following, although
// you can if you want to be:  If staying put runs the risk of a
// vampire possibly moving onto the player's location when the vampires
// move, yet moving in a particular direction puts the player in a
// position that is safe when the vampires move, then the chosen
// action is to move to a safer location.  Similarly, if staying put
// is safe, but moving in certain directions puts the player in
// danger of dying when the vampires move, then the chosen action should
// not be to move in one of the dangerous directions; instead, the player
// should stay put or move to another safe position.  In general, a
// position that may be moved to by many vampires is more dangerous than
// one that may be moved to by few.
//
// Unless you want to, you do not have to take into account that a
// vampire might be poisoned and thus sometimes less dangerous than one
// that is not.  That requires a more sophisticated analysis that
// we're not asking you to do.
bool recommendMove(const Arena& a, int r, int c, int& bestDir)
{
	// moveVampires will score the danger of moving N, E, S, or W
	// stayVampires will score the danger of staying put
	double moveVampires[NUMDIRS] = { 0, 0, 0, 0 };
	double stayVampires = 0;
	// In cases where the same number of dangerous vampires exist for a move
	// and standing still, the best move is to be in position to drop a
	// poisoned blood vial. This is possible if the cell is empty, so
	// stayVampires should be decreased a bit.
	if (a.getCellStatus(r, c) == EMPTY) {
		stayVampires -= 0.5;
	}
	// Check dangerous vampires in each direction
	for (int firstDir = 0; firstDir < NUMDIRS; firstDir++) {
		int firstRow = r;
		int firstCol = c;
		// If moving in that direction is out of bounds or causes the player to land on the vampire,
		// that move is the worst and should be set to maximum danger. Continue to check the remaining
		// directions.
		if (!attemptMove(a, firstDir, firstRow, firstCol) || a.numberOfVampiresAt(firstRow, firstCol) > 0) {
			if (a.numberOfVampiresAt(firstRow, firstCol) > 0)
				stayVampires += a.numberOfVampiresAt(firstRow, firstCol);
			moveVampires[firstDir] = double(MAXVAMPIRES)+1;
			continue;
		}
		// Like checking the position the player is at, check the position the player could move to 
		// if it's empty. In comparing moving in multiple "safe" directions, it would be best to move
		// to an empty cell and have the chance to drop a poisoned blood vial.
		if (a.getCellStatus(firstRow, firstCol) == EMPTY) {
			moveVampires[firstDir] -= 0.5;
		}
		// dirVampires used to count dangerous vampires that may move from N, E, S, W directions 
		int dirVampires = 0;
		for (int secondDir = 0; secondDir < NUMDIRS; secondDir++) {
			int secondRow = firstRow;
			int secondCol = firstCol;
			// If the move is inbounds, add the number of vampires at that position
			if (attemptMove(a, secondDir, secondRow, secondCol))
				dirVampires += double(a.numberOfVampiresAt(secondRow, secondCol));
		}
		// Add the dangerous vampires to the correct position of the array
		moveVampires[firstDir] += dirVampires;
	}
	// Determine the best direction from the minimum danger score 
	// The direction with the least vampires will always be chosen,
	// with the additional caveat that moving to an empty cell would be 
	// better than moving to a poisoned cell if the number of threatening
	// vampires is the same.
	int moveDir = -1;
	double minVampires = double(MAXVAMPIRES)+1;
	for (int dir = 0; dir < NUMDIRS; dir++) {
		if (moveVampires[dir] < minVampires) {
			minVampires = moveVampires[dir];
			moveDir = dir;
		}
	}
	// When deciding between moving in the most safe direction and standing still,
	// moving should only happen when:
	//	(1) there are less threatening vampires in that direction
	//  (2) the same number of vampires threaten whether standing still or not,
	//		but moving allows the player to move from a poisoned cell to an empty
	//		one and have the chance to drop another vial
	if (minVampires < stayVampires) {
		// Return true if a move should be taken and update bestDir
		bestDir = moveDir;
		return true;
	}
	// Else, return false to indicate staying still is best
	return false;
}

///////////////////////////////////////////////////////////////////////////
// main()
///////////////////////////////////////////////////////////////////////////

int main()
{
	// Create a game
	// Use this instead to create a mini-game:   Game g(3, 5, 2);
	Game g(10, 12, 40);

	// Play the game
	g.play();
}

///////////////////////////////////////////////////////////////////////////
//  clearScreen implementation
///////////////////////////////////////////////////////////////////////////

// DO NOT MODIFY OR REMOVE ANY CODE BETWEEN HERE AND THE END OF THE FILE!!!
// THE CODE IS SUITABLE FOR VISUAL C++, XCODE, AND g++/g31 UNDER LINUX.

// Note to Xcode users:  clearScreen() will just write a newline instead
// of clearing the window if you launch your program from within Xcode.
// That's acceptable.  (The Xcode output window doesn't have the capability
// of being cleared.)

#ifdef _MSC_VER  //  Microsoft Visual C++

#pragma warning(disable : 4005)
#include <windows.h>

void clearScreen()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	COORD upperLeft = { 0, 0 };
	DWORD dwCharsWritten;
	FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
		&dwCharsWritten);
	SetConsoleCursorPosition(hConsole, upperLeft);
}

#else  // not Microsoft Visual C++, so assume UNIX interface

#include <iostream>
#include <cstring>
#include <cstdlib>

void clearScreen()  // will just write a newline in an Xcode output window
{
	static const char* term = getenv("TERM");
	if (term == nullptr || strcmp(term, "dumb") == 0)
		cout << endl;
	else
	{
		static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal esc seq:  ESC [
		cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
	}
}

#endif
