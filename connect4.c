#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
/* Domain of Coordinates
 *
 *Those two constants specify the 
 *maximum value that can be applied 
 *to this game.
 *
 *The domain is:
 *Any point at (0,0) to (6,5).
 *
 *The Coordinate System is shown below.
*/
#define MaxX 6
#define MaxY 5
//Player Flag
typedef enum
{
	PLAYER_A = 1, // For User
	PLAYER_B = 2  // For Computer
}PLAYER;
/* RoundState -- Basic Construction of the Game
 *
 *Data Members:
 * Scene    - The whole board. It will be constantly updated 
 *            when the game is undergoing.
 *
 * NextMove - Because of the rules, positions that are available 
 *            for the next move are restricted: consider the board 
 *            as a top-open box, the only thing you can do is grabbing 
 *            a ball and let it free drop to the bottom.
 *
 * CurrentPlayer - Specify which player is currently playing.
 *
 * Moves    - Count the moves that have been made.
*/
/* Thorough Explanation of Scene
 *
 *A (MaxY+1) * (MaxX+1) two-dimensional board.
 *For each block: -1 means forbidden (in next move)
 *                0  means available and no chess in it
 *                1  means occupied by player A (User)
 *                2  means occupied by player B (Computer)
 *
 *  0  1  2  3  4  5  6
 * +--+--+--+--+--+--+--+
 *0|  |  |  |  |  |  |  |
 * +--+--+--+--+--+--+--+
 *1|  |  |  |  |  |  |  |
 * +--+--+--+--+--+--+--+
 *2|  |  |  |  |  |  |  |
 * +--+--+--+--+--+--+--+
 *3|  |  |  |  |  |  |  |
 * +--+--+--+--+--+--+--+
 *4|  |  |  |  |  |  |  |
 * +--+--+--+--+--+--+--+
 *5|  |  |  |  |  |  |  |
 * +--+--+--+--+--+--+--+
 *
 *Initially, the bottom line should be set by 0, the other should be set by -1.
*/
typedef struct
{
	int Scene[MaxY+1][MaxX+1];
	int NextMove[MaxX+1][2];
	PLAYER CurrentPlayer;
	int Moves;
}RoundState;
/*Rating System -- Primary Evaluation
 *
 *Minimax Algorithm will rate moves as the 
 *reference of the primary evaluation. More 
 *meaningful values are WinPosition and 
 *LosePosition.
 *
 *Because the algorithm evaluates on the view 
 *of computer, WinPosition would be the best; 
 *since NeutralPosition is relatively 
 *meaningless, so it is set to the lowest.
*/
#define WinPosition     1000
#define LosePosition    0
#define NeutralPosition -1000
/* MaxDepth
*/
#define MaxDepth 8
/* Rating System -- Secondary Evaluation
 *
 *It will be used only when the Primary Evaluation 
 *NeutralPosition. The thorough explanation of 
 *it is stated at DetermineBestMove().
*/
int VictoryProbability[MaxX+1] = {0};
/* CalculateCoordinateY()
 *
 *This function is a helper function of DetermineBestMove().
 *
 *DetermineBestMove() only returns the x-coordinate of next 
 *move. So CalculateCoordinateY() can scan NextMove Matrix 
 *to find the y-coordinate. Because in each turn, the 
 *x-coordinate of next move is unique.
*/
int CalculateCoordinateY(RoundState state, int x)
{
	int i;
	
	for(i=0;i<=MaxX;i++)
	{
		if(state.NextMove[i][0] == x)
		{
			return state.NextMove[i][1];
		}
	}
	
	return -1;
}
/* DisplayScene()
 *
 *Display the board on the screen.
*/
void DisplayScene(RoundState state)
{
	int i,j;
	int k,code;
	
	for(k=0;k<=MaxX;k++)
	{
		printf("   %d",k);
	}
	
	printf("\n");
	
	for(i=0;i<=MaxY;i++)
	{
		printf(" ");
		for(k=0;k<=MaxX;k++)
		{
			printf("+---");
		}
		printf("+\n");
		printf("%d",i);
		
		for(j=0;j<=MaxX;j++)
		{
			code = state.Scene[i][j];
			
			if(code == -1)
			{
				printf("|   ");
			}
			else
			{
				printf("| %d ", state.Scene[i][j]);
			}
		}
		printf("|\n");
	}
	
	printf(" ");
	for(k=0;k<=MaxX;k++)
	{
		printf("+---");
	}
	printf("+\n");
	for(k=0;k<=MaxX;k++)
	{
		printf("   %d",k);
	}
	
	printf("\n\n");
}
/* Direction Matrix
 *
 *Used by FindWinner().
 *
 *Each coordinate represents a direction vector. The whole matrix 
 *states 8 directions relative to a center point.
*/
const int Direction[8][2] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
/* GameInit()
 *
 *Preparations before the game starts.
*/
void GameInit(RoundState *state, PLAYER player)
{
	int i,j;
	
	// Initialize Scene Matrix
	for(i=0;i<=MaxY;i++)
	{
		for(j=0;j<=MaxX;j++)
		{
			state->Scene[i][j] = -1;
		}
	}
	// Modify the bottom line
	for(j=0;j<=MaxX;j++)
	{
		state->Scene[MaxY][j] = 0;
	}
	
	// Initialize NextMove Matrix
	for(i=0;i<=MaxX;i++)
	{
		state->NextMove[i][0] = i;
		state->NextMove[i][1] = MaxY;
	}
	
	state->CurrentPlayer = player;
	
	// Reset the global counter
	state->Moves = 0;
	
	// Initialize VictoryProbability Matrix
	for(i=0;i<=MaxX;i++)
	{
		VictoryProbability[i] = 0;
	}
	
	// Initialize the seed
	srand((unsigned int)time(NULL));
}
/* Opponent()
 *
 *UPDATE: Find the opponent of the player who is currently playing.
*/
PLAYER Opponent(PLAYER CurrentPlayer)
{
	switch(CurrentPlayer)
	{
		case PLAYER_A:
			return PLAYER_B;
			
		case PLAYER_B:
			return PLAYER_A;
	}
}
/* CheckNextStep()
 *
 *UPDATE: For the consideration of efficiency, this function now 
 *only works for users in the main loop.
*/
bool CheckNextStep(RoundState state, int x, int y)
{
	int i = 0;
	
	if(y == -1)
		return false;
	
	for(i=0;i<=MaxX;i++)
	{
		if(state.NextMove[i][0] == -1)
		{
			continue;
		}
		else if(state.NextMove[i][0] == x && state.NextMove[i][1] == y)
		{
			return true;
		}
	}
	
	return false;
}
/* MakeMove()
 *Old SetNewChess()
 *
 *UPDATE: In order to consider the effeciency, MakeMove() 
 *will no longer check if the coordinate is in the correct range.
*/
void MakeMove(RoundState *state, int x, int y)
{
	// Mark that position
	state->Scene[y][x] = state->CurrentPlayer;
	
	// Update the NextMove Matrix
	if(y > 0)
	{
		state->Scene[y-1][x] = 0;
		state->NextMove[x][0] = x;
		state->NextMove[x][1] = y-1;
	}
	// If we reach the top, disable this column
	else if(y == 0)
	{
		state->NextMove[x][0] = -1;
		state->NextMove[x][1] = -1;
	}
	
	// Reverse the player
	state->CurrentPlayer = Opponent(state->CurrentPlayer);
	
	// Finish this turn
	state->Moves++;
}
/* RetractMove()
 *
 *UPDATE: Retract one move, actually does the same job as MakeMove().
 *
 *UPDATE: In order to consider the effeciency, RetractMove() 
 *will also not check if the coordinate is in the correct range. 
 *So if it will be used by users, adding a function CheckRetraction() 
 *is necessary to make sure if this position is the top number(1 or 2) 
 *of this column.
*/
void RetractMove(RoundState *state, int x, int y)
{
	// Reset this position
	state->Scene[y][x] = 0;
	// If there is any meaningless zero, reset it
	if(y > 0)
	{
		state->Scene[y-1][x] = -1;
	}
	
	// Update the NextMove Matrix
	// We do not need to check the y-coordinate now, 
	//because you cannot make a move outside anyway.
	state->NextMove[x][0] = x;
	state->NextMove[x][1] = y;
	
	// Reverse the player
	state->CurrentPlayer = Opponent(state->CurrentPlayer);
	
	// Roll back this turn
	state->Moves--;
}
/* FindWinner()
 *
 *UPDATE: new update of CheckWinner(), it is faster.
*/
int FindWinner(RoundState state)
{
	int x,y,m,n;
	int NextX,NextY;
	int id;
	bool pass;
	
	if(state.Moves < 7)
	{
		return -1;
	}
	
	for(x=0;x<=MaxX;x++)
	{
		for(y=MaxY;y>=0;y--)
		{
			id = state.Scene[y][x];
			
			if(id == 0 || id == -1)
			{
				//Instead of 'continue', we search the Scene vertically, 
				//based on the rule, it would be meaningless if we linear 
				//search a winner strand based on a reference of 0 or -1.
				//Surprisingly, the values above 0 would always be -1.
				break;
			}
			
			for(m=0;m<8;m++) // Direction
			{
				pass = true;
				NextX = x;
				NextY = y;
				
				for(n=0;n<3;n++) // Number of points scanned forward
				{
					NextX += Direction[m][0];
					NextY += Direction[m][1];
					
					if((NextX<0 || NextX>MaxX) || (NextY<0 || NextY>MaxY))
					{
						//This strand is a dead strand, so it is not 
						//necessary to step forward along this direction.
						pass = false;
						break;
					}
					
					if(state.Scene[NextY][NextX] != id)
					{
						//This strand is a dead strand, so it is not 
						//necessary to step forward along this direction.
						pass = false;
						break;
					}
				}
				
				if(pass) // All three points are passed
				{
					// Winner strand is detected, no need to continue on.
					return id;
				}
			}
		}
	}
	
	return -1;
}
/* Minimax Algorithm
 *
 *DetermineBestMove() and EvaluatePosition()
 *
 *The core is a recursion involved two functions. These 2 functions 
 *call each other to evaluate 'current' Scene.
 *
 *DetermineBestMove() will simulate what the Scene would look like 
 *next step as many as possible. How many scenario it can evaluate 
 *depends on the 'depth' value. Bsed on the experimental data, when 
 *depth = 10, it is goning to take more than a minute to think.
 *
 *Since it is a game, the winning and losing scenes might occur at 
 *any round, instead after all blocks are filled. So the number of 
 *all possiblities would be extremely hard to determine accurately.
*/
int EvaluatePosition(RoundState, int);
/* EvaluateBestMove()
 *
 *Entry point of the evaluation.
 *
 *Find a best move among those simulated scenes. how many final-round 
 *situations can be simulated depends on depth value.
 *
 *The concept is easy: EvaluateBestMove(), assisted with EvaluatePosition(), 
 *continuously plays(simulates) this game. When a result occurs(win/lose/draw), 
 *EvaluateBestMove() will catch a rating of this simulation, if the rating is 
 *benefit for computer/not benefit for user, and better than previous one, this 
 *simulation(move) will be reserved.
*/
int EvaluateBestMove(RoundState state, int *MoveRating, int depth)
{
	int i;
	int Move[2], BestMoveX;
	int MaxRating = NeutralPosition - 1; // in order to be replaced at the first time
	int Rating;
	
	for(i=0; i<=MaxX; i++)
	{
		if(state.NextMove[i][0] == -1)
		{
			continue;
		}
		
		Move[0] = state.NextMove[i][0];
		Move[1] = state.NextMove[i][1];
		
		// Tactical Prediction Stage
		
		// Virtually make a move
		MakeMove(&state, Move[0], Move[1]);
		
		// Evaluate this move
		Rating = EvaluatePosition(state, depth + 1);
		
		// Secondary Rating Mechanism
		if(Rating == WinPosition)
		{
			VictoryProbability[i]++;
		}
		
		// Primary Rating Mechanism
		if (Rating > MaxRating)
		{
			BestMoveX = Move[0];
			MaxRating = Rating;
        }
        
		// Retract this move, that is why we call it 'virtual'
		RetractMove(&state, Move[0], Move[1]);
	}
	
	// Rating of the CURRENT Move
	//This is tricky: Each simulated step is actually made by different 
	//players. Since the rating is used for computer (PLAYER_B), so a 
	//good move for PLAYER_B is a bad move for PLAYER_A. Therefore when 
	//the recursion is rolling back, we need to reverse the evaluation 
	//result if the current player is switched to PLAYER_A.
	//Which means that this algorithm is actually evaluating for both sides, 
	//but only the best move for computer (PLAYER_B) should be preserved and 
	//the rest should be eliminated properly.
	*MoveRating = (state.CurrentPlayer == PLAYER_B)?(MaxRating):(-MaxRating);
	
	/*
	if(*MoveRating == NeutralPosition)
	{
		BestMoveX = FindMaxVP();
	}
	*/
	
	//For this game only, we do not need to return a coordinate. 
	//Because at each turn, the x-coordinate is unique in NextMove, 
	//y-coordinate, however, is not.
	return BestMoveX;
}
/* EvaluatePosition()
 *
 *This function is easier to understand: check if the simulation is 
 *over(hence the game is over). If so, grade this simulation; if not, 
 *come back to DetermineBestMove() to proceed the current simulation.
*/
int EvaluatePosition(RoundState state, int depth)
{
	int rate;
	int code = FindWinner(state);
	
	if(code != -1 || depth >= MaxDepth)
	{
		//PossibleScenes++;
		
		if(code == PLAYER_B)
		{
			return WinPosition;
		}
		else if(code == PLAYER_A)
		{
			return LosePosition;
		}
		else
		{
			return NeutralPosition;
		}
	}
	
	// Game is undergoing, proceed the simulation
	EvaluateBestMove(state, &rate, depth);
	return rate;
}
/* DetermineBestMove()
 *
 *It is an external packer function to make a final 
 *decision through considering ratings from primary 
 *and secondary rating system.
 *
 *The concept is: because of the limitation of depth 
 *value, many scenes might not reveal a result. This 
 *time the primary evaluation will always return 
 *NeutralPosition. Then we need the secondary evaluation. 
 *
 *VictoryProbability stores the total number of scenes of 
 *WinPosition for each point where you can make the move.
 *A higher number stands for a higher chance you are going 
 *to win finally.
*/
int DetermineBestMove(RoundState state, int *MoveRating)
{
	int BestX;
	int i, max = -1;
	
	BestX = EvaluateBestMove(state, MoveRating, 0);
	
	if(*MoveRating == NeutralPosition)
	{
		// Find the maximum probability
		for(i=0;i<=MaxX;i++)
		{
			if(VictoryProbability[i] > max)
			{
				max = VictoryProbability[i];
				BestX = i;
			}
		}
	}
	
	// Clean up VictoryProbability for the next round.
	for(i=0;i<=MaxX;i++)
	{
		VictoryProbability[i] = 0;
	}
	
	return BestX;
}
/* RandCreate()
 *
 *Create a random number within the domain [low, high).
*/
int RandCreate(int low, int high) 
{
	int val;
	val = ((int)(rand()%(high - low)) + low);
	return(val);
}
/* DummyPlayer()
 *
 *A dummy procedure to play against players.
 *
 *Dummy player uses a randomly picked coordinate to make a move.
*/
int DummyPlayer(RoundState *state)
{
	int choice;
	
	while(1)
	{
		// Why MaxX+1? See comments of RandCreate() and RoundState
		choice = RandCreate(0, MaxX+1);
		if(state->NextMove[choice][0] != -1)
		{
			break;
		}
	}
	
	return choice;
	
	//MakeMove(state, state->NextMove[choice][0], state->NextMove[choice][1]);
}
const char* Instruction1 = 
"\nWelcome to Connect 4\n\n"
"The following instructions will teach you how to play this game.\n\n"
"First, There is a %d * %d board.\n";
const char* Instruction2 = 
"You can see numbers along edges, when you make a move, you only "
"need to choose one number from those shown on the top or the bottom.\n"
"In this case, your input should be 0~%d.\n"
"Because where you make the next move is restricted. Those blocks that "
"contains '0' are open for the next move.\n"
"Give it a try, make a move.\n\n";
const char* Instruction3 = 
"Nicely done! Now you can play, but before you get started. You have to "
"know how to win in this game.\n"
"Just like what the name tells you, you need to connect at least 4 chess "
"of yours along one of eight directions to win.\n"
"Now, try to connect 4 chess.\n\n";
const char* Instruction4 = 
"Congratulations, you win!\n"
"This is just a demo procedure, of course. When you start a new game, you and "
"another player or computer take turns to make moves.\n"
"For the record, When you play with computer, '1' stands for you, '2' stands for "
"the computer.\n\n"
"Good luck and enjoy this game :)\n\n";
/* WaitForSpace and WaitForYesNo
 *
 *Get a restricted input from user for pauses.
*/
void WaitForSpace(char* hint)
{
	char ch;
	
	printf(hint);
	
	while(1)
	{
		ch = getch();
		
		if(ch == ' ')
		{
			system("cls");
			break;
		}
	}
}
bool WaitForYesNo(char* hint)
{
	char ch;
	bool bRet;
	
	printf(hint);
	
	while(1)
	{
		ch = getch();
		
		if(ch == 'y' || ch == 'Y')
		{
			bRet = true;
			break;
		}
		
		if(ch == 'n' || ch == 'N')
		{
			bRet = false;
			break;
		}
	}
	
	return bRet;
}
/* DemoHelper
 *
 *A special move-making procedure, for simulating 
 *a real game scenario.
 *
 *In order to use this function properly, the member 
 *Moves in RoundState must be adjusted to 7 (or a value 
 *that is slightly higher that 7) to ensure FindWinner 
 *runs properly (for more references, see EnterInstruction).
 *
 *The reason is that in the demo procedure, '2 players' is 
 *not meaningful. So for each turn, the player will be 
 *locked at the same one (PLAYER_A or PLAYER_B). For more 
 *references, see FindWinner and MakeMove.
*/
void DemoHelper(RoundState *game)
{
	int x,y;
	
	while(1)
	{
		printf("Your move: ");
		scanf("%d",&x);
		y = CalculateCoordinateY(*game, x);
		if(CheckNextStep(*game, x, y))
		{
			break;
		}
		printf("Illegal Input, try again.\n");
	}
	MakeMove(game, x, y);
	// Lock the player, it is just a demo.
	game->CurrentPlayer = PLAYER_A;
}
/* EnterInstruction()
 *
 *Guidence of instructions.
*/
void EnterInstruction(RoundState *game)
{
	system("cls");
	
	printf(Instruction1, MaxY+1, MaxX+1);
	
	DisplayScene(*game);
	
	WaitForSpace("\n\nPress SPACE to proceed.");
	
	printf(Instruction2, MaxX);
	
	DisplayScene(*game);
	
	// Hack the move counter, because FindWinner() works only after the 7th move.
	game->Moves = 7;
	
	DemoHelper(game);
	
	printf(Instruction3);
	
	DisplayScene(*game);
	
	while(FindWinner(*game) == -1)
	{
		DemoHelper(game);
		DisplayScene(*game);
	}
	
	printf(Instruction4);
	
	WaitForSpace("\n\nPress SPACE to finish.");
}
/* ModeHelper()
 *
 *Guidence of game modes.
 *
 *Easy Mode
 *	Computer randomly makes a move.
 *
 *Hard Mode
 *	Computer carefully makes a move.
 *
 *Hell Mode
 *	Computer carefully makes a move, meanwhile, 
 *users will not see any board update on the screen, 
 *but coordinates.
 *
 *2-player mode
 *	2 users play the game.
*/
int ModeHelper()
{
	char ch;
	
	printf("\n\nChoose a mode to play:\n"
	       "1 -> Easy mode\n"
	       "2 -> Hard mode\n"
	       "3 -> Hell mode\n"
	       "4 -> 2-player mode\n"
	       "Enter your choice(1|2|3|4):");
	
	while(1)
	{
		ch = getch();
		
		switch(ch)
		{
			case '1':
				return 1;
			
			case '2':
				return 2;
			
			case '3':
				return 3;
				
			case '4':
				return 4;
			
			default:
				break;
		}
	}
}
/* Guidance()
 *
 *A run-once function that satrts when this program just starts.
*/
void Guidance()
{
	RoundState game;
	bool bRules;
	
	system("cls");
	
	printf("Welcome to Connect 4\n\n");
	
	bRules = WaitForYesNo("Would you like to know more about the rules?(y/n)");
	
	if(bRules)
	{
		GameInit(&game,PLAYER_A);
		EnterInstruction(&game);
	}
}
/* GameMain_EasyMode()
 *
 *The game loop for Easy Mode.
*/
void GameMain_EasyMode(RoundState *state)
{
	int x,y,rating;
	
	DisplayScene(*state);
	
	while((FindWinner(*state) == -1) && (state->Moves <= (MaxX+1)*(MaxY+1)))
	{
		switch(state->CurrentPlayer)
		{
			case PLAYER_A:
			{
				while(1)
				{
					printf("Your move: ");
					//scanf("%d %d",&x,&y);
					scanf("%d",&x);
					y = CalculateCoordinateY(*state, x);
					if(CheckNextStep(*state, x, y))
					{
						break;
					}
					printf("Illegal Input, try again.\n");
				}
				system("cls");
				break;
			}
			
			case PLAYER_B:
			{
				x = DummyPlayer(state);
				y = CalculateCoordinateY(*state, x);
				system("cls");
				printf("Computer makes a move (%d,%d).\n",x,y);
				break;
			}
		}
		
		MakeMove(state, x, y);
		
		DisplayScene(*state);
	}
	
	return;
}
/* GameMain_HardMode()
 *
 *The game loop for Hard Mode.
*/
void GameMain_HardMode(RoundState *state)
{
	int x,y,rating;
	
	DisplayScene(*state);
	
	while((FindWinner(*state) == -1) && (state->Moves <= (MaxX+1)*(MaxY+1)))
	{
		switch(state->CurrentPlayer)
		{
			case PLAYER_A:
			{
				while(1)
				{
					printf("Your move: ");
					//scanf("%d %d",&x,&y);
					scanf("%d",&x);
					y = CalculateCoordinateY(*state, x);
					if(CheckNextStep(*state, x, y))
					{
						break;
					}
					printf("Illegal Input, try again.\n");
				}
				system("cls");
				break;
			}
			
			case PLAYER_B:
				printf("Computer is thinking...");
				x = DetermineBestMove(*state, &rating);
				y = CalculateCoordinateY(*state, x);
				system("cls");
				printf("\nIt makes the move (%d, %d) (%d)\n",x,y,rating);
				break;
		}
		
		MakeMove(state, x, y);
		
		DisplayScene(*state);
	}
	
	return;
}
/* GameMain_HellMode()
 *
 *The game loop for Hell Mode.
*/
void GameMain_HellMode(RoundState *state)
{
	int x,y,rating;
	
	//DisplayScene(*state);
	
	while((FindWinner(*state) == -1) && (state->Moves <= (MaxX+1)*(MaxY+1)))
	{
		switch(state->CurrentPlayer)
		{
			case PLAYER_A:
			{
				while(1)
				{
					printf("Your move: ");
					//scanf("%d %d",&x,&y);
					scanf("%d",&x);
					y = CalculateCoordinateY(*state, x);
					if(CheckNextStep(*state, x, y))
					{
						break;
					}
					printf("Illegal Input, try again.\n");
				}
				//system("cls");
				break;
			}
			
			case PLAYER_B:
				printf("Computer is thinking...");
				x = DetermineBestMove(*state, &rating);
				y = CalculateCoordinateY(*state, x);
				//system("cls");
				printf("\nIt makes the move (%d, %d) (%d)\n",x,y,rating);
				break;
		}
		
		MakeMove(state, x, y);
		
		//DisplayScene(*state);
	}
	
	return;
}
/* GameMain_TwoPlayerMode()
 *
 *The game loop for 2-player Mode.
*/
void GameMain_TwoPlayerMode(RoundState *state)
{
	int x,y,rating;
	
	DisplayScene(*state);
	
	while((FindWinner(*state) == -1) && (state->Moves <= (MaxX+1)*(MaxY+1)))
	{
		printf("%s makes a move: ", state->CurrentPlayer==PLAYER_A?"Player A":"Player B");
		
		while(1)
		{
			scanf("%d",&x);
			y = CalculateCoordinateY(*state, x);
			if(CheckNextStep(*state, x, y))
			{
				break;
			}
			printf("Illegal Input, try again.\n");
		}
		
		system("cls");
		printf("%s makes a move (%d,%d).",
		       state->CurrentPlayer==PLAYER_A?"Player A":"Player B",
			   x,y);
		
		MakeMove(state, x, y);
		
		DisplayScene(*state);
	}
	
	return;
}
/* main()
 *
 *The entry point of the game.
*/
int main()
{
	RoundState game;
	int choice;
	
	Guidance();
	
	// The main loop of the game.
	
	while(true)
	{
		choice = ModeHelper();
		GameInit(&game, PLAYER_B);
		
		printf("\n\n");
		
		switch(choice)
		{
			case 1:
				GameMain_EasyMode(&game);
				break;
				
			case 2:
				GameMain_HardMode(&game);
				break;
				
			case 3:
				GameMain_HellMode(&game);
				break;
				
			case 4:
				GameMain_TwoPlayerMode(&game);
				break;
		}
		
		switch(FindWinner(game))
		{
			case -1:
				printf("Tie.");
				break;
				
			case 1:
				printf("You win.");
				break;
				
			case 2:
				printf("You lose.");
				break;
		}
		
		if(WaitForYesNo("\n\nWould you like to play again?(y/n)") == false)
			break;
	}
	
	return 0;
}
