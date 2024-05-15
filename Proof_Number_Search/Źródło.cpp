#include <iostream>
#include <cmath>
#pragma warning(disable : 4996)
#define AND 1
#define OR 0
#define DISPROVEN -10
#define PROVEN 10
#define UNKNOWN 0

using namespace std;

struct node;

struct vector {
	node** table;
	int size;
	int capacity;

	vector() {
		this->size = 0;
		this->capacity = 1;
		this->table = (node**)malloc(this->capacity * sizeof(node*));
	}
	vector(vector& old) {
		this->size = 0;
		this->capacity = 1;
		this->table = (node**)malloc(this->capacity * sizeof(node*));
	}

	void reallocate(double multi) {
		this->capacity *= multi;
		node** newPtr = (node**)malloc(this->capacity * sizeof(node*));
		memcpy(newPtr, table, this->size * sizeof(node*));
		free(table);
		this->table = newPtr;
	}
	void push_back(node& val) {
		if (this->size == this->capacity)
			reallocate(2);
		this->table[this->size] = &val;
		this->size++;
	}
	void pop_back() {
		this->size--;
		node* retv = this->table[this->size];
		if (4 * this->size <= this->capacity && this->capacity != 1)
			reallocate(0.5);
	}

	void clear() {
		free(this->table);
		this->size = 0;
		this->capacity = 1;
		this->table = (node**)malloc(this->capacity * sizeof(node*));
	}

	~vector();

	node* operator[](int i) {
		return table[i];
	}
};

typedef struct board_S {
	unsigned short* table;
	int X = 0;
	int Y = 0;
	int size = 0;
	int sequence = 0;
	unsigned short active_player = 1;
	int free_space = 0;
	board_S(int Y, int X, unsigned short active_player, int sequence) : X(X), Y(Y), active_player(active_player), sequence(sequence) {
		table = (unsigned short*)calloc(X * Y, X * Y * sizeof(*table));
		size = X * Y;
	}
	board_S(board_S& old) : X(old.X), Y(old.Y), active_player(old.active_player), sequence(old.sequence), size(old.size), free_space(old.free_space) {
		table = (unsigned short*)calloc(X * Y, X * Y * sizeof(*table));
		memcpy(table, old.table, old.size * sizeof(unsigned short));
	}
	~board_S() {
		free(table);
	}


	void print() {
		for (int j = 0; j < Y; j++) {
			for (int i = 0; i < X; i++) {
				printf("%i ", (*this)(j, i));
			}
			printf("\n");
		}
	}
	unsigned short getOponent() {
		if (active_player == 1) {
			return 2;
		}
		else {
			return 1;
		}
	}

	unsigned short& operator()(int m) {
		return table[m];
	}
	unsigned short operator()(int m) const {
		return table[m];
	}
	unsigned short& operator()(int y, int x) {
		return table[y * X + x];
	}
	unsigned short operator()(int y, int x) const {
		return table[y * X + x];
	}
};

struct ret {
	int b;
	int a;
	bool control;

	ret add(const ret& r, int sequence, bool player) const {
		ret temp = { 0,0,false };
		bool t1 = r.control || control;

		temp.b = b + r.b + 1;
		temp.a = a + r.a + t1;

		if ((temp.b + r.a + r.control >= sequence) != (temp.b + a + control >= sequence)) {
			temp.control = true;
		}
		if (player && temp.b + t1 == sequence) {
			temp.control = t1;
		}
		return temp;
	}
};

struct node {
	board_S board;
	bool type = 0;
	int proof = 0, disproof = 0;
	int value = 0;
	node* parent = NULL;
	vector children;

	node(board_S& old, node* parent): board(old), parent(parent	), type(!parent->type), children() {
		
	}
	node(int Y, int X, unsigned short active_player, int sequence, bool type) : board(Y, X, active_player, sequence), type(type), children() {
		
	}
};

vector::~vector() {

	if (size > 0) {
		for (int i = 0; i < size; i++) {
			delete table[i];
		}
	}
	free(this->table);
}

int checkSequence(board_S& board, int y, int x, int dy, int dx, unsigned short active, int number = 0) {
	if (x + dx >= 0 && x + dx < board.X && y + dy >= 0 && y + dy < board.Y) {
		if (board(y + dy, x + dx) == active) {
			return checkSequence(board, y + dy, x + dx, dy, dx, active, number + 1);
		}
	}
	return number;
}

ret checkSequencev2(board_S& board, int y, int x, int dy, int dx, unsigned short active, int a = 0, int b = 0, bool skip = false) {
	if (x + dx >= 0 && x + dx < board.X && y + dy >= 0 && y + dy < board.Y) {
		bool test = board(y + dy, x + dx) == active;
		if (test || (!skip && board(y + dy, x + dx) == 0)) {
			if (!test) skip = true;
			if (!skip) {
				b++;
			}
			else if (test) {
				a++;
			}
			return checkSequencev2(board, y + dy, x + dx, dy, dx, active, a, b, skip);
		}
	}

	return { b, a, skip };
}

unsigned short checkPoint(board_S& board, int y, int x) {
	int res;
	unsigned short num = board(y, x);
	if (num == 0)return 0;
	res = checkSequence(board, y, x, -1, -1, num) + checkSequence(board, y, x, 1, 1, num) + 1;
	if (res >= board.sequence) return num;
	res = checkSequence(board, y, x, 1, -1, num) + checkSequence(board, y, x, -1, 1, num) + 1;
	if (res >= board.sequence) return num;
	res = checkSequence(board, y, x, -1, 0, num) + checkSequence(board, y, x, 1, 0, num) + 1;
	if (res >= board.sequence) return num;
	res = checkSequence(board, y, x, 0, -1, num) + checkSequence(board, y, x, 0, 1, num) + 1;
	if (res >= board.sequence) return num;
	return 0;
}

ret checkPointv2(board_S& board, int y, int x, unsigned short active = 0) {
	ret res;
	unsigned short num = board(y, x);
	int test = 0;
	if (num == 0)return { 0,0,false };

	res = checkSequencev2(board, y, x, -1, -1, num).add(checkSequencev2(board, y, x, 1, 1, num), board.sequence, active == num);
	if (res.control) {
		test++;
	}
	else {
		if (res.a + res.b >= board.sequence) {
			return { num, 0, false };
		}
	}
	res = checkSequencev2(board, y, x, 1, -1, num).add(checkSequencev2(board, y, x, -1, 1, num), board.sequence, active == num);
	if (res.control) {
		test++;
	}
	else {
		if (res.a + res.b >= board.sequence) {
			return { num, 0, false };
		}
	}
	res = checkSequencev2(board, y, x, -1, 0, num).add(checkSequencev2(board, y, x, 1, 0, num), board.sequence, active == num);
	if (res.control) {
		test++;
	}
	else {
		if (res.a + res.b >= board.sequence) {
			return { num, 0, false };
		}
	}

	res = checkSequencev2(board, y, x, 0, -1, num).add(checkSequencev2(board, y, x, 0, 1, num), board.sequence, active == num);
	if (res.control) {
		test++;
	}
	else {
		if (res.a + res.b >= board.sequence) {
			return { num, 0, false };
		}
	}

	if (test >= 1) {
		if (test >= 2 || num == active) {
			return { num, 0, false };
		}
		return { 0, 0, true };
	}
	else {
		return { 0, 0, false };
	}
}

unsigned short allBoardCheckWin(board_S& board) {
	unsigned short test = 0;
	for (int j = 0; j < board.Y && !test; j++) {
		for (int i = 0; i < board.X && !test; i++) {
			test = checkPoint(board, j, i);
		}
	}
	return test;
}

unsigned short allBoardCheckWinv2(board_S& board, unsigned short active) {
	unsigned short bes = 0;
	unsigned short temp = 0;
	unsigned short oponent = 0;
	unsigned short ans = allBoardCheckWin(board);
	if (!ans) {
		for (int j = 0; j < board.Y && bes != active; j++) {
			for (int i = 0; i < board.X && bes != active; i++) {
				if (board(j, i) == 0) {
					board(j, i) = board.active_player;
					temp = checkPoint(board, j, i);
					if (temp != 0)bes = temp;
					board(j, i) = board.getOponent();
					temp = checkPoint(board, j, i);
					if (temp != 0)oponent++;
					board(j, i) = 0;
				}
			}
		}
		if (bes == 0 && oponent >= 2) bes = board.getOponent();
		return bes;
	}
	else {
		return ans;
	}
}

ret GEN_ALL_POS_MOV_CUT_IF_GAME_OVER(board_S& board) {
	bool control = false;

	for (int j = 0; j < board.Y; j++) {
		for (int i = 0; i < board.X; i++) {
			if (board(j, i) == 0) {
				board(j, i) = board.active_player;
				control = checkPoint(board, j, i);
				board(j, i) = 0;
				if (control)return { j,i,true };
			}
		}
	}
	return { 0, 0, false };
}
//----------------------------------

int decode(node& n) {
	if (n.disproof == INT_MAX) {
		return PROVEN;
	}
	else if (n.proof == INT_MAX) {
		return DISPROVEN;
	}
	else {
		return UNKNOWN;
	}
}

void generateChildren(node& n) {
	for (int j = 0; j < n.board.Y; j++) {
		for (int i = 0; i < n.board.X; i++) {
			if (n.board(j, i) == 0) {
				n.board(j, i) = n.board.active_player;
				n.board.active_player = n.board.getOponent();
				node* temp = new node(n.board, &n);
				temp->board.free_space--;
				n.children.push_back(*temp);
				n.board.active_player = n.board.getOponent();
				n.board(j, i) = 0;
			}
		}
	}
}

void evaluate(node& n, unsigned short question) {
	unsigned short res = allBoardCheckWinv2(n.board, n.board.active_player);
	if (res == question) {
		n.value = PROVEN;
	}
	else if (res == 0 && n.board.free_space != 0) {
		n.value = UNKNOWN;
	}
	else {
		n.value = DISPROVEN;
	}
}

void setProofAndDisproofNumbers(node& n) {
	if (n.children.size) {
		if (n.type == AND) {
			n.proof = 0; n.disproof = INT_MAX;
			for (int i = 0; i < n.children.size; i++) {
				node* c = n.children[i];
				if (c->proof == INT_MAX) {
					n.proof = INT_MAX;
				}
				else if(n.proof != INT_MAX){
					n.proof += c->proof;
				}
				n.disproof = fmin(n.disproof, c->disproof);
			}
		}
		else {
			n.disproof = 0; n.proof = INT_MAX;
			for (int i = 0; i < n.children.size; i++) {
				node* c = n.children[i];
				if (c->disproof == INT_MAX) {
					n.disproof = INT_MAX;
				}
				else if(n.disproof != INT_MAX){
					n.disproof += c->disproof;
				}
				n.proof = fmin(n.proof, c->proof);
			}
		}
	}
	else {
		switch (n.value) {
			case DISPROVEN: n.proof = INT_MAX; n.disproof = 0; break;
			case PROVEN:    n.proof = 0; n.disproof = INT_MAX; break;
			case UNKNOWN:   n.proof = 1; n.disproof = 1; break;
		}
	}
}

node* selectMostProvingNode(node* n)	 {
	while (n->children.size) {
		int value = INT_MAX;
		node* best = NULL;
		if (n->type == AND) {
			for (int i = 0; i < n->children.size; i++) {
				node* c = n->children[i];
				if (value > c->disproof) {
					best = c;
					value = c->disproof;
				}
			}
		}
		else {
			for (int i = 0; i < n->children.size; i++) {
				node* c = n->children[i];
				if (value > c->proof) {
					best = c;
					value = c->proof;
				}
			}
		}
		n = best;
	}
	return n;
}

void expandNode(node& n, unsigned short question) {
	generateChildren(n);
	//printf("===========================\n");
	//n.board.print();
	//printf("---------------------------");
	for (int i = 0; i < n.children.size; i++) {
		node* c = n.children[i];
		evaluate(*c, question);
		setProofAndDisproofNumbers(*c);
		//printf("\n(%d,%d)\n",c->proof, c->disproof);
		//c->board.print();
		if (n.type == AND) {
			if (c->disproof == 0)break;
		}
		else {
			if (c->proof == 0)break;
		}
	}
	//printf("===========================\n");
}

node* updateAncestors(node* n, node& root) {
	while (n != &root) {
		int oldProof = n->proof;
		int oldDisproof = n->disproof;
		setProofAndDisproofNumbers(*n);
		if (n->proof == oldProof && n->disproof == oldDisproof)
			return n;
		n = n->parent;
	}
	setProofAndDisproofNumbers(root);
	return &root;
}

int PNS(node root, unsigned short question) {
	evaluate(root, question);
	setProofAndDisproofNumbers(root);
	node* current = &root;
	while (root.proof != 0 && root.disproof != 0) {
		node* mostProving = selectMostProvingNode(current);
		expandNode(*mostProving, question);
		current = updateAncestors(mostProving, root);
	}
	//printf("\n\n\n\n\n\n\n");
	return decode(root);
}

int main() {
	char buffer[256];
	char buffer2[256];
	setvbuf(stdout, buffer, _IOFBF, sizeof(buffer));
	setvbuf(stdin, buffer2, _IOFBF, sizeof(buffer2));
	char input[64], t;
	int x, y, xx, sequence;
	unsigned short player;

	while (true) {
		x = 0;
		y = 0;
		sequence = 0;
		player = 0;
		xx = 0;
		scanf("%100s %d %d %d %hu\n", &input, &y, &x, &sequence, &player);
		if (feof(stdin) != 0)break;
		node root(y, x, player, sequence, OR);
		while (xx < x * y && !feof(stdin)) {
			t = (char)getchar();
			switch (t) {
			case '\n':
			case EOF:
			case ' ':
				xx++;
				break;
			default:
				root.board(xx) = t - '0';
				if (t == '0')root.board.free_space++;
				break;
			}
		}


		if (strcmp(input, "SOLVE_GAME_STATE") == 0) {
			int res = PNS(root, player);
			if (res == PROVEN) {
				if(player == 1)
					printf("FIRST_PLAYER_WINS\n");
				else
					printf("SECOND_PLAYER_WINS\n");
			}
			else{
				//printf("--------------\n");
				root.type = AND;
				res = PNS(root, root.board.getOponent());
				if (res == PROVEN) {
					if (player == 1)
						printf("SECOND_PLAYER_WINS\n");
					else
						printf("FIRST_PLAYER_WINS\n");
				}
				else if (res == DISPROVEN) {
					printf("BOTH_PLAYERS_TIE\n");
				}
			}
		}
	}
	fflush(stdout);
	return 0;
}