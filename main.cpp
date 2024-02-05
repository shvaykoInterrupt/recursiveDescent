#include <stdlib.h>
#include <stdio.h>

typedef enum
{
	TOKEN_STAR = 0,
	TOKEN_SLASH,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_INT,
	TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
	TOKEN_EOF
}TokenType;

typedef struct 
{
	TokenType type;
	char* lexeme;
	size_t length;
	int value;
}Token;
	
typedef struct 
{
	int size;
	int capacity;
	Token* buffer;
}Tokens;

Tokens tokens;
int currentTokenIndex;
Token currentToken;

#define NEW_CAPACITY(oldCapacity) ((oldCapacity == 0) ? 8 : oldCapacity*2)
#define ALLOCATE_MEMORY(buffer, newSize, type) ((type*)allocateMemory(buffer, newSize*sizeof(type)))
#define FREE_MEMORY(buffer) (allocateMemory(buffer,0))

void initTokensArray(Tokens* arr)
{
	arr->size = 0;
	arr->capacity = 0;
	arr->buffer = 0;
}


void* allocateMemory(void *buffer, size_t newSize)
{

	if (newSize == 0)
	{
		free(buffer);
		return 0;
	}

	buffer = realloc(buffer, newSize);


	return buffer;
}

void freeTokensArray(Tokens* arr)
{
	FREE_MEMORY(arr->buffer);
	initTokensArray(arr);
}

void writeTokensArray(Tokens* arr, Token token)
{
	if ((arr->size + 1) >= arr->capacity)
	{
		int oldCapacity = arr->capacity;
		arr->capacity = NEW_CAPACITY(oldCapacity);
		arr->buffer = ALLOCATE_MEMORY(arr->buffer, arr->capacity, Token);
	}
	arr->buffer[arr->size++] = token;
}


typedef struct
{
	char* start;
	char* current;

	int line;
}Scanner;

typedef enum
{
	NODE_ADDITION,
	NODE_SUBTRACTION,
	NODE_MULTIPLY,
	NODE_DIVIDE,
	NODE_INT
}NodeType;

typedef struct ASTNode
{
	NodeType type;

	ASTNode* left;
	ASTNode* right;

	int val;
}ASTNode;

ASTNode* makeASTNode(ASTNode *left, ASTNode *right,NodeType type, int value)
{
	ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
	node->type  = type;
	node->left  = left;
	node->right = right;
	node->val = value;
	return node;
}

ASTNode* makeASTLeaf(NodeType type, int value)
{
	return makeASTNode(0, 0,type,value);
}

ASTNode* makeASTUnary(ASTNode *right, NodeType type, int value)
{
	return makeASTNode(0, right, type, value);
}

NodeType arithop()
{
	switch (currentToken.type)
	{
		case TOKEN_PLUS: return NODE_ADDITION;
		case TOKEN_MINUS: return NODE_SUBTRACTION;
		case TOKEN_STAR: return NODE_MULTIPLY;
		case TOKEN_SLASH: return NODE_DIVIDE;
		default:
		{
			// unreachable
		}
	}
}

static Scanner scanner;

void initScanner(char *src)
{
	scanner.start = src;
	scanner.current = src;
	int line = 1;
}

void makeToken(Tokens *tokens, TokenType type, int value)
{
	Token token;
	token.lexeme = scanner.start;
	token.length = (int)(scanner.current - scanner.start);
	token.type = type;
	token.value = value;
	writeTokensArray(tokens, token);
}

bool isDigit(char c)
{
	return (c >= '0') && (c <= '9');
}

char peek()
{
	return scanner.current[0];
}

char advance()
{
	scanner.current++;
	return scanner.current[-1];
}


void skipWhitespace()
{
	for (;;)
	{
		char c = peek();
		switch (c)
		{
			case ' ': 
			case '\t':
			{
				advance();
			}break;
			default:
				return;
		}
	}
}

bool lex(Tokens *tokens)
{
	skipWhitespace();
	scanner.start = scanner.current;
	char ch = advance();
	switch(ch)
	{
		case '+':
		{
			makeToken(tokens, TOKEN_PLUS, 0);
		}break;
		case '-':
		{
			makeToken(tokens, TOKEN_MINUS, 0);
		}break;
		case '*':
		{
			makeToken(tokens, TOKEN_STAR, 0);
		}break;
		case '/':
		{
			makeToken(tokens, TOKEN_SLASH, 0);
		}break;
		case '(':
		{
			makeToken(tokens, TOKEN_LEFT_PAREN, 0);
		}break;
		case ')':
		{
			makeToken(tokens, TOKEN_RIGHT_PAREN, 0);
		}break;
		case '\0':
		{
			makeToken(tokens, TOKEN_EOF, 0);
			return false;
		}break;
		default:
		{
			if(isDigit(ch))
			{
				while (isDigit(peek())) advance();

				int val = (int)strtod(scanner.start, &scanner.current);
				makeToken(tokens, TOKEN_INT,val);
			}
		}break;
	}
	return true;
}

void printTokens(Tokens *tokens)
{
	for(int i = 0; i < tokens->size; i++)
	{
		Token token = tokens->buffer[i];
		if (token.type != TOKEN_EOF && token.type != TOKEN_INT)
		{
			printf("Token '%.*s'\n", token.length, token.lexeme);
		}
		else if (token.type == TOKEN_INT)
		{
			printf("Token '%.*s with value: %d\n", token.length, token.lexeme, token.value);
		}
	}
}

void nextToken()
{
	currentTokenIndex++;
	currentToken = tokens.buffer[currentTokenIndex - 1];
}


bool checkToken(TokenType type)
{
	if (currentToken.type == type)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool matchToken(TokenType type)
{
	if (checkToken(type))
	{
		nextToken();
		return true;
	}

	return false;
}

void consume(TokenType type,const char *message)
{
	if (matchToken(type))
	{
		return;
	}
	else
	{
		printf(message);
		puts("\n");
	}
}

ASTNode* expression();



ASTNode* primary()
{
	if (checkToken(TOKEN_INT))
	{
		ASTNode *leaf = makeASTLeaf(NODE_INT, currentToken.value);
		nextToken();
		return leaf;
	}
	else if (checkToken(TOKEN_LEFT_PAREN))
	{
		nextToken();
		ASTNode *leaf = expression();
		consume(TOKEN_RIGHT_PAREN, "Expect ')' ");
		return  leaf;
	}
}

ASTNode* unary()
{
	if (checkToken(TOKEN_MINUS))
	{
		NodeType operatorType = arithop();
		nextToken();
		ASTNode* right = unary();
		return makeASTUnary(right, operatorType, 0);
	}

	return primary();
}

ASTNode* factor()
{
	ASTNode* left = unary();

	while (checkToken(TOKEN_STAR) || checkToken(TOKEN_SLASH))
	{
		NodeType operatorType = arithop();
		nextToken();
		ASTNode* right = unary();
		left = makeASTNode(left, right, operatorType, 0);
	}
	return left;
}

ASTNode* term()
{
	ASTNode* left = factor();
	while(checkToken(TOKEN_PLUS) || checkToken(TOKEN_MINUS))
	{
		NodeType operatorType = arithop();
		nextToken();
		ASTNode *right = factor();
		left = makeASTNode(left, right, operatorType, 0);
	}
	return left;
}

ASTNode* expression()
{
	return term();
}

int interpretAST(ASTNode *node)
{
	if (!node) return 0;

	int right = 0;
	int left = 0;
	if (node->left)
	{
		left = interpretAST(node->left);
	}
	if (node->right)
	{
		right = interpretAST(node->right);
	}
	
	switch (node->type)
	{
		case NODE_INT:
		{
			return node->val;
		}break;
		case NODE_ADDITION:
		{
			return left + right;
		}break;
		case NODE_SUBTRACTION:
		{
			return left - right;
		}break;
		case NODE_DIVIDE:
		{
			return left / right;
		}break;
		case NODE_MULTIPLY:
		{
			return left * right;
		}break;
		default:
		{
			// unknown
		};
	}
}

int main(int argc, char *argv[])
{
	initTokensArray(&tokens);
	char line[1024];
	fgets(line, 1024, stdin);
	initScanner(line);
	while (lex(&tokens)) {};

	nextToken();
	ASTNode* root = expression();
	int result = interpretAST(root);

	printf("Interpretation result: %d\n", result);
	//printTokens(&tokens);
	freeTokensArray(&tokens);
	return 0;
}