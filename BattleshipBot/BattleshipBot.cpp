/*
* Author: Brendan Baker
* Created: 05/03/2019
* Revised: 04/04/2019
* Description: Defines the entry point for the console application.
* User advice: none
*
*TODO:
*  - work out weakest ship based on health and distance
*  - work out closest ship and target it
*  - work out avoiding ships based on their health
*  - change flag every 10 seconds so it doesnt get detected
*  - group up with the team
*  - test out the types
*/

#include "stdafx.h"
#include <winsock2.h>
#include <math.h>
#include <time.h>

#pragma comment(lib, "wsock32.lib")

#define SHIPTYPE_BATTLESHIP	"0"
#define SHIPTYPE_FRIGATE	"1"
#define SHIPTYPE_SUBMARINE	"2"

#define STUDENT_NUMBER		" (: > :( "
#define STUDENT_FIRSTNAME	"BRENDAN"
#define STUDENT_FAMILYNAME	"BAKER"
#define MY_SHIP	SHIPTYPE_BATTLESHIP

//#define IP_ADDRESS_SERVER	"127.0.0.1"
#define IP_ADDRESS_SERVER "164.11.76.216"

#define PORT_SEND	 1924 // We define a port that we are going to use.
#define PORT_RECEIVE 1925 // We define a port that we are going to use.

#define MAX_BUFFER_SIZE	500
#define MAX_SHIPS		200

#define FIRING_RANGE	100

#define MOVE_LEFT		-1
#define MOVE_RIGHT		 1
#define MOVE_UP			 1
#define MOVE_DOWN		-1
#define MOVE_FAST		 2
#define MOVE_SLOW		 1


SOCKADDR_IN sendto_addr;
SOCKADDR_IN receive_addr;

SOCKET sock_send;  // This is our socket, it is the handle to the IO address to read/write packets
SOCKET sock_recv;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer[MAX_BUFFER_SIZE];

// VARIABLES
int myX;
int myY;
int myHealth;
int myFlag;
int myType;

int closestShipDistance;
int closestShip;
int lowestEnemyHealth;
int weakestShip;
bool firedAt = false;
bool attacking = false;

int number_of_ships; //Number of ships in range
int shipX[MAX_SHIPS]; //Array of all ship's X coordinates on the server
int shipY[MAX_SHIPS]; //Array of all ship's Y coordinates on the server
int shipHealth[MAX_SHIPS]; //Array of all ship's health on the server
int shipFlag[MAX_SHIPS];
int shipType[MAX_SHIPS];

bool message = false;
char MsgBuffer[MAX_BUFFER_SIZE];

//Used in fire_at_ship()
bool fire = false;
int fireX;
int fireY;

//Used in move_in_direction;
bool moveShip = false;
int moveX;
int moveY;

bool setFlag = true;
int new_flag = 0;

void send_message(char* dest, char* source, char* msg);
void fire_at_ship(int X, int Y);
void move_in_direction(int left_right, int up_down);
void set_new_flag(int newFlag);

/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/

int up_down = MOVE_LEFT * MOVE_SLOW;
int left_right = MOVE_UP * MOVE_FAST;

int shipDistance[MAX_SHIPS];

int number_of_friends;
int friendX[MAX_SHIPS];
int friendY[MAX_SHIPS];
int friendHealth[MAX_SHIPS];
int friendFlag[MAX_SHIPS];
int friendDistance[MAX_SHIPS];
int friendType[MAX_SHIPS];

int number_of_enemies;
int enemyX[MAX_SHIPS];
int enemyY[MAX_SHIPS];
int enemyHealth[MAX_SHIPS];
int enemyFlag[MAX_SHIPS];
int enemyDistance[MAX_SHIPS];
int enemyType[MAX_SHIPS];

/*
* Function: grouped
* Description: checks whether ship ia friend and got towards them
* Parameters: index (int) - the index of the ship you want to check
* Returns: boolean
* Warnings: Currently only returns false as don't want any friends
*/
bool grouped(int f) {

	bool group = false;

	for (int i = 0; i < number_of_friends; i++) {
		if (i == 2 && friendDistance[i] < 5) {
			group = true;
		}
		else {
			group = false;
		}
	}

	return group;
}
/*
* Function: isaFriend
* Description: checks whether ship is a friendly
* Parameters: index (int) - the index of the ship you want to check
* Returns: boolean
* Warnings: Currently only returns false as don't want any friends
*/
bool IsaFriend(int index)
{
	bool rc;

	rc = false;

	if (shipFlag[index] == myFlag)
	{
		rc = true;
	}

	return rc;
}


/*
* Function: stats
* Description: prints me and my friends stats
* Parameters: printf print stats
* Returns: none (int)
* Warnings: none
*/
void stats() {

	for (int i = 0; i < 10; i++) {
		printf("\n\n\n\n\n\n\n\n\n");
	}

	printf("Bot\tX co\tY co\tHealth\tFlag\tClosest Ship Distance\tFriends\tEnemies\n");
	printf("-----------------------------------------------------------------------------------------------------\n");
	printf("Me\t");
	printf("%d\t", myX);
	printf("%d\t", myY);
	printf("%d\t", myHealth);
	printf("%d\t", myFlag);
	printf("\t\t\t");
	printf("%d\t", number_of_friends);
	printf("%d\n", number_of_enemies);
	//printf("%d\n", lowestEnemyHealth);
	printf("-----------------------------------------------------------------------------------------------------\n");

	for (int i = 0; i < number_of_enemies; i++) {
		printf("E %d\t", i);
		printf("%d\t", enemyX[i]);
		printf("%d\t", enemyY[i]);
		printf("%d\t", enemyHealth[i]);
		printf("%d\t\t", enemyFlag[i]);
		printf("%d\n", enemyDistance[i]);
		//printf("%d\n", lowestEnemyHealth);
		printf("-----------------------------------------------------------------------------------------------------\n");
	}
}

/*
* Function: followAndShoot
* Description: Moves the ship towards coordinates and Shoots them
* Parameters: x (int), y (int) - x and y coordinates you want to move towards
* Returns: none (void)
* Warnings: none
*/
//follow ships and shoot at them - WORKS
void followAndShoot(int targetX, int targetY) {

	int dirX;
	int dirY;

	if (myX > targetX) {
		dirX = MOVE_LEFT * MOVE_FAST;
	}
	else {
		dirX = MOVE_RIGHT * MOVE_FAST;
	}

	if (myY > targetY) {
		dirY = MOVE_DOWN * MOVE_FAST;
	}
	else {
		dirY = MOVE_UP * MOVE_FAST;
	}

	move_in_direction(dirX, dirY);
	fire_at_ship(targetX, targetY);

	//TESTING - Add onc array is filtered to move back if their distance gets smaller
	/*if (myHealth > enemyHealth[weakestShip] * 2 && shipDistance[weakestShip] < 90){
	move_in_direction(0,0);
	}
	else if (myHealth > enemyHealth[weakestShip] * 1.5 && shipDistance[weakestShip] < 60){
	move_in_direction(0,0);
	}
	else {
	move_in_direction(dirX, dirY);
	}*/
}

/*
* Function:getWeakestShip
* Description: returns the index of the most damaged enemy
* Parameters: none
* Returns: most_damaged (int) the index of the most damaged enemy
* Warnings: none
*/
void getWeakestShip() {

	lowestEnemyHealth = 10;

	for (int i = 0; i < number_of_enemies; i++) {
		if (enemyHealth[i] < lowestEnemyHealth) {

			lowestEnemyHealth = enemyHealth[i];
			//weakestShip = i;

		}
	}
}
/*
* Function: cal_distance
* Description: Calculates distance between two coordinates
* Parameters: x1 (int), y1 (int) - first set of coordinates
* Parameters: x2 (int), y2 (int) - second set of coordinates
* Returns: output (int) - the distance between the two sets of coordinates
* Warnings: none
*/
int cal_distance(int i) {
	int x, y, z;
	x = myX - shipX[i];
	y = myY - shipY[i];
	z = pow(x, 2) + pow(y, 2);
	return (int)sqrt(z);
}
/*
* Function: getClosestEnemy
* Description: returns the index of the nearest enemy
* Parameters: none
* Returns: closestShipDistance (int) - the index of the closest enemy
* Warnings: none
*/
void getClosestEnemy(int i) {

	closestShipDistance = 200;

	for (int i = 0; i < number_of_enemies; i++) {

		if (enemyDistance[i] < closestShipDistance) {

			closestShipDistance = enemyDistance[i];
			//closestShip = i;
		}

		//if the distance to enemy is less than 100, destroy that bitch
		if (enemyDistance[i] < 100) {
			followAndShoot(enemyX[i], enemyY[i]);
		}
	}
}

/*
* Function:followFriends
* Description: checks whether ship ia friend and got towards them
* Parameters: index (int) - the index of the ship you want to check
* Returns: int
* Warnings: Currently only returns false as don't want any friends
*/
void followFriends() {

	int dirX, dirY;
	int i;

	for (i = 0; i < number_of_friends; i++) {
		if (friendDistance[i] > 2) {
			if (myX > friendX[i]) {
				dirX = MOVE_LEFT * MOVE_FAST;
			}
			else {
				dirX = MOVE_RIGHT * MOVE_FAST;
			}

			if (myY > friendY[i] > 2) {
				dirY = MOVE_DOWN * MOVE_FAST;
			}
			else {
				dirY = MOVE_UP * MOVE_FAST;
			}
		}
	}
}

/*
void avoidShips(int h[], int nopeX, int nopeY) {

int i;

for (i = 0; i < number_of_enemies; i++) {

if (myHealth < h[i]) {
//move away from them and towards a ship with less health that is close to me
if (myX > enemyX[i]) {
MOVE_RIGHT*MOVE_FAST;
}
else {

}
}
else {
//continue with other tactics
}
}

//CHECK FOR MULTIPLE ENEMIES AND AVOID THEM
}
*/

/*
void changeFlag(int fl) {

int i;

for (i = 0; i < 10; i++) {

if (i == 10) {
fl = fl * 3;
i = 0;
}
else if (i == 10 && fl > 200) {
fl = fl / 3;
}
}

//set time at start
//10 seconds after change flag
//change flag every 10 seconds based on the time from the previous change
//so what I need to do is basically save the time at which the first changed occured and store it in a variable then take that variable and set it 0 when the second change occurs
}
*/

/*
* Function: rendezvous
* Description: Moves to the centre of the grip everytime
* Parameters: x (int), y (int) - x and y coordinates you want to move towards
* Returns: none (void)
* Warnings: none
*/
void rendezvous() {

	int dirX, dirY;

	if (!grouped(number_of_friends)) {
		if (myX > 500) {
			dirX = MOVE_LEFT * MOVE_FAST;
		}
		else {
			dirX = MOVE_RIGHT * MOVE_FAST;
		}

		if (myY > 500) {
			dirY = MOVE_DOWN * MOVE_FAST;
		}
		else {
			dirY = MOVE_UP * MOVE_FAST;
		}

		move_in_direction(dirX, dirY);
	}
}
/*
* Function: tactics
* Description: Where most of the tactic code resides
* Parameters: none
* Returns: none (void)
* Warnings: none
*/
void tactics()
{
	int i;
	myFlag = 50;

	if (myY > 900)
	{
		up_down = MOVE_DOWN * MOVE_FAST;
	}

	if (myX < 200)
	{
		left_right = MOVE_RIGHT * MOVE_FAST;
	}

	if (myY < 100)
	{
		up_down = MOVE_UP * MOVE_FAST;
	}

	if (myX > 800)
	{
		left_right = MOVE_LEFT * MOVE_FAST;
	}

	//get the distance to enemy ships
	for (i = 0; i<number_of_ships; i++)
	{
		//shipDistance[i] = (int)sqrt((double)((shipX[i] - shipX[0])*(shipX[i] - shipX[0]) + (shipY[i] - shipY[0])*(shipY[i] - shipY[0])));
		shipDistance[i] = cal_distance(i);
	}

	number_of_friends = 0;
	number_of_enemies = 0;

	if (number_of_ships > 1)
	{
		for (i = 1; i<number_of_ships; i++)
		{
			if (IsaFriend(i))
			{
				friendX[number_of_friends] = shipX[i];
				friendY[number_of_friends] = shipY[i];
				friendHealth[number_of_friends] = shipHealth[i];
				friendFlag[number_of_friends] = shipFlag[i];
				friendDistance[number_of_friends] = shipDistance[i];
				friendType[number_of_friends] = shipType[i];
				number_of_friends++;

			}
			else
			{
				enemyX[number_of_enemies] = shipX[i];
				enemyY[number_of_enemies] = shipY[i];
				enemyHealth[number_of_enemies] = shipHealth[i];
				enemyFlag[number_of_enemies] = shipFlag[i];
				enemyDistance[number_of_enemies] = shipDistance[i];
				enemyType[number_of_enemies] = shipType[i];
				number_of_enemies++;
			}
		}

		if (number_of_enemies > 0)
		{
			fire_at_ship(enemyX[0], enemyY[0]);
		}
	}

	//char msg[100];
	//sprintf_s(msg, "Im at %d %d", myX, myY);
	//send_message("12345678", "23456789", msg);  // send my co-ordinates to myself 

	if (!grouped(number_of_friends)) {

		rendezvous();
		stats();

		for (i = 0; i < number_of_enemies; i++) {

			getClosestEnemy(i);
			//followAndShoot(enemyX[i], enemyY[i]);

		}
	}
	else {

		for (i = 0; i < number_of_enemies; i++) {

			getClosestEnemy(i);
			followAndShoot(enemyX[i], enemyY[i]);

		}

		followFriends();
		getWeakestShip();
		stats();
	}

	//move_in_direction(left_right, up_down);
}

/*
* Function: messageReceived
* Description: Checks if the message recieved is matching the required criteria
* Parameters: msg (char*)
* Returns: none (void)
* Warnings: none
*/
void messageReceived(char* msg)
{
	int X;
	int Y;

	printf("%s\n", msg);

	if (sscanf_s(msg, "Message 12345678, 23456789, Im at %d %d", &X, &Y) == 2)
	{
		printf("My friend is at %d %d\n", X, Y);
	}
}


/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/


void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	char chr;
	bool finished;
	int  i;
	int  j;
	int  rc;
	char* p;

	sprintf_s(buffer, "Register  %s,%s,%s,%s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME, MY_SHIP);
	sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));

	while (true)
	{
		if (recvfrom(sock_recv, buffer, sizeof(buffer) - 1, 0, (SOCKADDR *)&receive_addr, &len) != SOCKET_ERROR)
		{
			p = ::inet_ntoa(receive_addr.sin_addr);

			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				if (buffer[0] == 'M')
				{
					messageReceived(buffer);
				}
				else
				{
					i = 0;
					j = 0;
					finished = false;
					number_of_ships = 0;

					while ((!finished) && (i<4096))
					{
						chr = buffer[i];

						switch (chr)
						{
						case '|':
							InputBuffer[j] = '\0';
							j = 0;
							sscanf_s(InputBuffer, "%d,%d,%d,%d", &shipX[number_of_ships], &shipY[number_of_ships], &shipHealth[number_of_ships], &shipFlag[number_of_ships], &shipType[number_of_ships]);
							number_of_ships++;
							break;

						case '\0':
							InputBuffer[j] = '\0';
							sscanf_s(InputBuffer, "%d,%d,%d,%d", &shipX[number_of_ships], &shipY[number_of_ships], &shipHealth[number_of_ships], &shipFlag[number_of_ships], &shipType[number_of_ships]);
							number_of_ships++;
							finished = true;
							break;

						default:
							InputBuffer[j] = chr;
							j++;
							break;
						}
						i++;
					}

					myX = shipX[0];
					myY = shipY[0];
					myHealth = shipHealth[0];
					myFlag = shipFlag[0];
					myType = shipType[0];
				}

				tactics();

				if (message)
				{
					sendto(sock_send, MsgBuffer, strlen(MsgBuffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					message = false;
				}

				if (fire)
				{
					sprintf_s(buffer, "Fire %s,%d,%d", STUDENT_NUMBER, fireX, fireY);
					sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					fire = false;
				}

				if (moveShip)
				{
					sprintf_s(buffer, "Move %s,%d,%d", STUDENT_NUMBER, moveX, moveY);
					rc = sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					moveShip = false;
				}

				if (setFlag)
				{
					sprintf_s(buffer, "Flag %s,%d", STUDENT_NUMBER, new_flag);
					sendto(sock_send, buffer, strlen(buffer), 0, (SOCKADDR *)&sendto_addr, sizeof(SOCKADDR));
					setFlag = false;
				}

			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}

void send_message(char* dest, char* source, char* msg)
{
	message = true;
	sprintf_s(MsgBuffer, "Message %s,%s,%s,%s", STUDENT_NUMBER, dest, source, msg);
}

void fire_at_ship(int X, int Y)
{
	fire = true;
	fireX = X;
	fireY = Y;
}

void move_in_direction(int X, int Y)
{
	if (X < -2) X = -2;
	if (X >  2) X = 2;
	if (Y < -2) Y = -2;
	if (Y >  2) Y = 2;

	moveShip = true;
	moveX = X;
	moveY = Y;
}

void set_new_flag(int newFlag)
{
	setFlag = true;
	new_flag = newFlag;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char chr = '\0';

	printf("\n");
	printf("Battleship Bots\n");
	printf("UWE Computer and Network Systems Assignment 2 (2016-17)\n");
	printf("\n");

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	//sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	//if (!sock)
	//{	
	//	printf("Socket creation failed!\n"); 
	//}

	sock_send = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock_send)
	{
		printf("Socket creation failed!\n");
	}

	sock_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock_recv)
	{
		printf("Socket creation failed!\n");
	}

	memset(&sendto_addr, 0, sizeof(SOCKADDR_IN));
	sendto_addr.sin_family = AF_INET;
	sendto_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	sendto_addr.sin_port = htons(PORT_SEND);

	memset(&receive_addr, 0, sizeof(SOCKADDR_IN));
	receive_addr.sin_family = AF_INET;
	//	receive_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	receive_addr.sin_addr.s_addr = INADDR_ANY;
	receive_addr.sin_port = htons(PORT_RECEIVE);

	int ret = bind(sock_recv, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	//	int ret = bind(sock_send, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	if (ret)
	{
		printf("Bind failed! %d\n", WSAGetLastError());
	}

	communicate_with_server();

	closesocket(sock_send);
	closesocket(sock_recv);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}