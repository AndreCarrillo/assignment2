// assignment2.cpp : Defines the entry point for the console application.

//#include "stdafx.h"
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <iterator>  
#include <queue>
#include <stack>
#include <sstream> 
#include <locale>
#include <math.h>
#include <sstream>
#include <string>
#include <semaphore.h>
#include <fcntl.h>

using namespace std;
class ExpressionParser
{
public:

	ExpressionParser(const  string& input);
	bool MatchingParetheses();

	bool Evaluate(const  vector< string>& rpn, string& result);

	bool InfixToRPN(vector< string>& inputs);

private:
	void ReplaceAll(string& str,
		const  string& from,
		const  string& to);

	void Tokenize(list< string>& tokens,
		const  string& delimiter);

private:
	string m_strInput;
};




const  string charSet[] = { "(", ")", "%", "+", "-", "*", "/", "^", "," };

const double pi = 3.1415927;
const double e = 2.71828182846;

int Modulo(int num, int div)
{
	int mod = num % div;

	return (num >= 0 || mod == 0) ? mod : div + mod;
}


unsigned int OpArgCount(const  string& s)
{
	unsigned int val = 1;

	if (s == "*" || s == "/" || s == "%" ||
		s == "+" || s == "-" || s == "=" ||
		s == "^" || s == "POW")
	{
		val = 2;
	}
	else if (s == "!")
	{
		val = 1;
	}

	return val;
}


int OpPrecedence(const  string& s)
{
	int precedence = 1;

	if (s == "!")
	{
		precedence = 4;
	}
	else if (s == "*" || s == "/" || s == "%")
	{
		precedence = 3;
	}
	else if (s == "+" || s == "-")
	{
		precedence = 2;
	}
	else if (s == "=")
	{
		precedence = 1;
	}

	return precedence;
}

// Return true if left associative; false otherwise
bool OpLeftAssoc(const  string& s)
{
	bool opLeftAssoc = false;

	// left to right
	if (s == "*" || s == "/" || s == "%" || s == "+" || s == "-")
	{
		opLeftAssoc = true;
	}
	// right to left
	else if (s == "=" || s == "!")
	{
		opLeftAssoc = false;
	}

	return opLeftAssoc;
}

// Is token an operator
bool IsOperator(const  string& s)
{
	return s == "+" || s == "-" || s == "/" ||
		s == "*" || s == "!" || s == "%" ||
		s == "=";
}

// Is token a function argument separator eg comma
bool IsComma(const  string& s)
{
	return s == ",";
}

// Convert string into all uppercase
string UpperCase(string input)
{
	for (string::iterator it = input.begin();
		it != input.end();
		++it)
	{
		*it = toupper(*it);
	}

	return input;
}

// Is token PI
bool IsPi(const  string& s)
{
	bool isPi = false;

	if (UpperCase(s) == "PI")
	{
		isPi = true;
	}

	return isPi;
}

// Is token Euler's constant
bool IsE(const  string& s)
{
	bool isE = false;

	if (UpperCase(s) == "E")
	{
		isE = true;
	}

	return isE;
}

// Is the token a function
bool IsFunction(const string& s)
{
	string str = UpperCase(s);

	bool isFunction = false;

	if (str.find("^") != string::npos ||
		str.find("SIN") != string::npos ||
		str.find("COS") != string::npos ||
		str.find("TAN") != string::npos ||
		str.find("LN") != string::npos ||
		str.find("LOG") != string::npos ||
		str.find("EXP") != string::npos ||
		str.find("POW") != string::npos ||
		str.find("SQRT") != string::npos)
	{
		isFunction = true;
	}

	return isFunction;
}

// Is the number a float
bool IsFloat(const string& s)
{
	istringstream iss(s);
	float f;
	iss >> noskipws >> f;
	return iss.eof() && !iss.fail();
}

// Is the string a number
bool IsNumber(const string& s)
{
	string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it, locale()))
	{
		++it;
	}

	return !s.empty() && it == s.end();
}

ExpressionParser::ExpressionParser(const string& input)
{
	m_strInput = input;
}

// Determine if matching number of left and right parentheses
bool ExpressionParser::MatchingParetheses()
{
	vector< string > inputs;

	// Check left and right parentheses are equal
	const size_t nLeft = count(m_strInput.begin(), m_strInput.end(), '(');
	const size_t nRight = count(m_strInput.begin(), m_strInput.end(), ')');

	return nLeft == nRight && !m_strInput.empty();
}

// Split selected text into delimited vector array of strings
void ExpressionParser::Tokenize(list<string>& tokens,
	const string& delimiter)
{
	// Insert whitepaces before and after each special characters
	size_t size = sizeof(charSet) / sizeof(string);

	for (int i = 0; i < static_cast<int>(size); i++)
	{
		string s = charSet[i];
		ReplaceAll(m_strInput, s, " " + s + " ");
	}

	size_t next_pos = 0;
	size_t init_pos = m_strInput.find_first_not_of(delimiter, next_pos);

	while (next_pos != string::npos &&
		init_pos != string::npos)
	{
		// Get next delimiter position    
		next_pos = m_strInput.find(delimiter, init_pos);

		string token = m_strInput.substr(init_pos, next_pos - init_pos);
		tokens.push_back(token);

		init_pos = m_strInput.find_first_not_of(delimiter, next_pos);
	}

	// Deal with start token being a minus sign
	string firstToken = tokens.front();
	if (firstToken == "-")
	{
		list<string>::iterator it = tokens.begin();
		it++;

		if (it == tokens.end())
		{
			return;
		}

		string nextToken = *(it);

		if (IsNumber(nextToken) || IsFloat(nextToken))
		{
			tokens.pop_front();
			tokens.front() = firstToken + nextToken;
		}
		else if (nextToken == "(" || IsFunction(nextToken))
		{
			tokens.front() = firstToken + "1";
			tokens.insert(it, "*");
		}
		// minus minus is a plus
		else if (nextToken == "-" && firstToken == "-")
		{
			list<string>::iterator nit = it;
			advance(nit, -1);
			tokens.erase(it);
			tokens.erase(nit);
		}
	}

	// Deal with minus sign after opening parenthesis or operator
	typedef list<string>::iterator t_iter;
	string prevToken = "";
	for (t_iter it = tokens.begin(); it != prev(tokens.end()); it++)
	{
		string token = *it;

		list<string>::iterator nit = it;
		advance(nit, 1);

		if (nit == tokens.end())
		{
			break;
		}

		string ntoken = *nit;

		if (token == "-" && prevToken == "(")
		{
			if (IsNumber(ntoken) || IsFloat(ntoken))
			{
				tokens.erase(nit);
				*it = "-" + ntoken;
				token = *it;
			}
		}

		else if (token == "-" &&
			(IsOperator(prevToken) || prevToken == "^" || prevToken == "%"))
		{
			// Minus minus becomes a plus
			if (token == "-" && prevToken == "-")
			{
				list<string>::iterator nit = it;
				list<string>::iterator nnit = nit;
				nnit++;
				advance(nit, -1);
				tokens.erase(it);
				*nit = "+";

				list<string>::iterator pnit = nit;
				advance(pnit, -1);

				if (IsOperator(*pnit) || *pnit == "(")
				{
					tokens.erase(nit);
				}

				token = *nnit;
				it = nnit;

				if (it == prev(tokens.end()))
				{
					break;
				}
			}
			else if (IsNumber(ntoken) || IsFloat(ntoken) || IsFunction(ntoken))
			{
				bool exit = false;
				if (nit == prev(tokens.end()))
				{
					exit = true;
				}

				tokens.erase(nit);
				*it = "-" + ntoken;
				token = *it;

				if (exit) break;
			}
			else if (ntoken == "(")
			{
				*it = "-1";
				token = *it;
				tokens.insert(nit, "*");
			}
		}

		prevToken = token;
	}

	// Deal with minus sign before opening parenthesis
	prevToken = "";
	t_iter prevIt;

	for (t_iter it = tokens.begin(); it != tokens.end(); it++)
	{
		string token = *it;

		if (token == "(" && prevToken == "-")
		{
			tokens.insert(it, "1");
			tokens.insert(it, "*");
		}

		prevToken = token;
		prevIt = it;
	}

}


// Replace all instances of selected string with replacement string
void ExpressionParser::ReplaceAll(string& str,
	const string& from,
	const string& to)
{
	size_t start_pos = 0;

	while ((start_pos = str.find(from, start_pos)) != string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // ...
	}
}


bool ExpressionParser::Evaluate(const vector<string>& rpn, string& result)
{
	typedef vector<string>::const_iterator rpn_iter;
	stack<string> stack;

	// While there are input tokens left
	for (rpn_iter it = rpn.begin(); it != rpn.end(); it++)
	{
		// Read the next token from input.
		string token = *it;

		// If the token is a value push it onto the stack.
		if (IsNumber(token) ||
			IsFloat(token) ||
			IsPi(token) ||
			IsE(token))
		{
			if (IsPi(token))
			{
				stringstream s;
				s << pi;
				token = s.str();
			}
			else if (IsE(token))
			{
				stringstream s;
				s << e;
				token = s.str();
			}
			stack.push(token);
		}

		// Otherwise, the token is an operator or a function
		else if (IsOperator(token) || IsFunction(token))
		{
			double result = 0.0;

			// It is known a priori that the operator takes n arguments.
			unsigned int nargs = OpArgCount(UpperCase(token));

			bool isUnary = false;
			unsigned int stackArgs = stack.size();
			vector<double> args;

			if (stackArgs < nargs)
			{
				// For dealing with unary '-' or unary '+'
				if (stackArgs == 1 && nargs == 2 && (token == "+" || token == "-"))
				{
					string value = stack.top();
					result = strtod(value.c_str(), NULL);
					stack.pop();
					isUnary = true;
				}
				else
				{
					// (Error) The user has not input sufficient values in the expression.
					return false;
				}
			}
			else
			{
				// Else, Pop the top n values from the stack.				
				while (nargs > 0)
				{
					string value = stack.top();
					double d = strtod(value.c_str(), NULL);
					args.push_back(d);
					stack.pop();
					nargs--;
				}
			}

			// Evaluate the operator, with the values as arguments.                       
			if (IsOperator(token) && !isUnary)
			{
				// Token is an operator: pop top two entries                          
				double d2 = args[0];
				double d1 = args[1];

				//Get the result  
				if (token == "+")
				{
					result = d1 + d2;
				}
				else if (token == "-")
				{
					result = d1 - d2;
				}
				else if (token == "*")
				{
					result = d1 * d2;
				}
				else if (token == "/")
				{
					result = d1 / d2;
				}
				else if (token == "%")
				{
					int i2 = (int)args[0];
					int i1 = (int)args[1];
					double iresult = Modulo(i1, i2);
					result = iresult;
				}
			}
			else if (IsFunction(token))
			{
				double d0 = args[0];

				string capToken = UpperCase(token);

				// If say -SIN( x ) then multiply result of SIN by -1.0
				double mult =
					token.find("-") != string::npos ? -1 : 1;

				if (capToken.find("SIN") != string::npos)
				{
					result = sin(d0);
				}
				else if (capToken.find("COS") != string::npos)
				{
					result = cos(d0);
				}
				else if (capToken.find("TAN") != string::npos)
				{
					result = tan(d0);
				}
				else if (capToken.find("LN") != string::npos)
				{
					result = log(d0);
				}
				else if (capToken.find("LOG") != string::npos)
				{
					result = log10(d0);
				}
				else if (capToken.find("EXP") != string::npos)
				{
					result = exp(d0);
				}
				else if (capToken.find("^") != string::npos)
				{
					double d2 = args[0];
					double d1 = args[1];

					result = pow((double)d1, d2);
				}
				else if (capToken.find("POW") != string::npos)
				{
					double d2 = args[0];
					double d1 = args[1];
					result = pow(d1, d2);
				}
				else if (capToken.find("SQRT") != string::npos)
				{
					result = sqrt(d0);
				}

				result *= mult;
			}

			// Push the returned results, if any, back onto the stack
			// Push result onto stack   
			if (result == (long)result)
			{
				result = long(result);
			}
			stringstream s;
			s << result;
			stack.push(s.str());
		}
	}

	// If there is only one value in the stack then
	// that value is the result of the calculation.
	if (stack.size() == 1)
	{
		result = stack.top();

		// Output the result

		double res;
		istringstream in(result.c_str()); //line is the string which contains the number, ex: 3.30144800e+03
		in >> res;


		//double res = strtod( result.c_str(), NULL );  
		if (res == (long)res)
		{
			long lres = (long)res;
			stringstream s;
			s << lres;
			result = s.str();
		}

		return true;
	}

	// If there are more values in the stack
	// (Error) The user input has too many values.
	return false;
}

// Convert infix expression format into reverse Polish notation    
bool ExpressionParser::InfixToRPN(vector< string>& inputs)
{
	list< string> infix;
	Tokenize(infix, " ");

	typedef  list< string>::const_iterator tok_iter;
	stack< string> stack;
	queue< string> outputQueue;

	bool success = true;

	// For each token
	for (tok_iter it = infix.begin(); it != infix.end(); it++)
	{
		// Read a token.
		string token = *it;

		// If the token is a number, then add it to the output queue.
		if (IsNumber(token) || IsFloat(token) || IsPi(token) || IsE(token))
		{
			outputQueue.push(token);
		}
		// If the token is a function token, then push it onto the stack.
		else if (IsFunction(token))
		{
			stack.push(token);
		}
		else if (IsComma(token))
		{
			// Until token at the top of stack is left parenthesis, pop operators 
			// off the stack onto the output queue.             
			string stackToken = stack.top();

			while (stackToken != "(")
			{
				outputQueue.push(stackToken);
				stack.pop();
				stackToken = stack.top();
			}

			if (stackToken == "(")
			{
				success = true;
			}
			else
			{
				success = false;
			}
		}
		// If the token (o1) is an operator, then:
		else if (IsOperator(token))
		{
			// While there is operator token, o2, at the top of the stack, and:
			// either o1 is left-associative and its precedence is equal to that of o2,
			// or o1 has precedence less than that of o2
			while (!stack.empty() && IsOperator(stack.top()) &&
				((OpLeftAssoc(token) && OpPrecedence(token) == OpPrecedence(stack.top())) ||
				(OpPrecedence(token) < OpPrecedence(stack.top()))))
			{
				// Pop o2 off the stack, onto the output queue
				string stackToken = stack.top();
				stack.pop();
				outputQueue.push(stackToken);
			}

			// Push o1 onto the stack
			stack.push(token);

		}
		// If the token is a left parenthesis, then push it onto the stack.
		else if (token == "(")
		{
			// Push token to top of the stack  
			stack.push(token);
		}
		// If the token is a right parenthesis:
		else if (token == ")")
		{
			// Until the token at the top of the stack is a left parenthesis, pop operators 
			// off the stack onto the output queue.
			while (!stack.empty() && stack.top() != "(")
			{
				// Add to end of list  
				outputQueue.push(stack.top());
				stack.pop();
			}

			// If stack runs out without finding a left parenthesis, 
			// there are mismatched parentheses
			if (!stack.empty())
			{
				string stackToken = stack.top();

				// Mismatched parentheses
				if (stackToken != "(")
				{
					success = false;
				}
			}
			else
			{
				return false;
			}

			// Pop left parenthesis from the stack, but not onto output queue.
			stack.pop();

			// If token at top of stack is function token, pop it onto output queue
			if (!stack.empty())
			{
				string stackToken = stack.top();
				if (IsFunction(stackToken))
				{
					outputQueue.push(stackToken);
					stack.pop();
				}
			}
		}
	}

	// While there are still operator tokens in the stack:
	while (!stack.empty())
	{
		// Pop the operator onto the output queue
		outputQueue.push(stack.top());
		stack.pop();
	}

	while (!outputQueue.empty())
	{
		string token = outputQueue.front();
		inputs.push_back(token);
		outputQueue.pop();
	}

	return success;
}


int string_to_double(const  string& s)
{
	istringstream i(s);
	int x;
	if (!(i >> x))
		return 0;
	return x;
}

int calculate(string s)
{



	ExpressionParser parser(s);

	vector< string> RPN;

	parser.InfixToRPN(RPN);


	string str_result;
	parser.Evaluate(RPN, str_result);

	int result = string_to_double(str_result);
	return result;

}


map<string, int> data_map;
vector<string> res;

/*STRING SPLIT FUCTION*/

vector<string> strsplit(string str) {
	string sstr;
	int start = 0;
	for (int i = 0; i<str.size(); i++) {
		if (str.at(i) == ';') return res;
		while (i<str.size() && str.at(i) != ',') i++;
		sstr = str.substr(start, i - start);

		int j = 0;
		while (j<sstr.size() - 1 && sstr.at(j) == ' ') j++;
		sstr = sstr.substr(j);

		j = sstr.size() - 1;
		while (j >= 0 && sstr.at(j) == ' ') j--;
		sstr = sstr.substr(0, j + 1);
		res.push_back(sstr);

		start = i + 1;
	}
	size_t pos = sstr.find(";");

	string str3 = sstr.substr(0, pos);

	res[res.size() - 1] = str3;

	return res;
}

ifstream myFile2;
string line;
vector<string> input;
char comma = ',';
string a;
vector<string> input_var;
vector<string> internal_var;
ifstream myFile1;
string inputvalues;
vector<string> inputvaluevector;
string process;
vector<string> itos;
int index;
int *shm, *s[10];
int beginidx;
string string1;
string string2;

int p_count = 0;
void assignment2() {

	while (getline(myFile1, line)) {

		istringstream iss(line);
		input.push_back(line);

	}

	for (int i = 1; i < input.size(); i++) {

		if (input[i].find("input_var") != string::npos) {

			int counter = 0;
			istringstream iss(input[i]);
			getline(iss, line, '\t');

			getline(iss, line, ' ');

			getline(iss, line);

			strsplit(line);
			//stored in res vector
			input_var = res;

			res.clear();

		}

		else if (input[i].find("internal_var") != string::npos) {

			istringstream iss(input[i]);
			getline(iss, line, '\t');
			getline(iss, line, ' ');
			getline(iss, line);
			strsplit(line);
			internal_var = res;

		}

		else if (input[i].find("read") != string::npos) {
			string temp;
			while (getline(myFile2, temp)) {
				//cout << inputvalues << endl;
				istringstream iss(temp);

				if (temp.find(",") != string::npos) {
					while (getline(iss, inputvalues, ',')) {
						istringstream iss(inputvalues);
						inputvaluevector.push_back(inputvalues);
					}

				}
				if (temp.find(" ") != string::npos) {
					while (getline(iss, inputvalues, ' ')) {
						istringstream iss(inputvalues);
						inputvaluevector.push_back(inputvalues);
					}

				}
				

			}

		}

		else if (input[i].find("cobegin") != string::npos) {
			i++;
			string str;
			beginidx = i;

			while (input[i].find("p") != string::npos) {
				p_count++;
				istringstream count(input[i]);
				count >> str;

				for (int j = 0; j < input_var.size(); j++) {
					if (input[i].find(input_var[j]) != string::npos) {

						size_t posA = input[i].find(input_var[j]);

						input[i].replace(posA, input_var[j].length(), inputvaluevector[j]);
						if (input[i].find('=') != string::npos) {
							size_t posB = input[i].find('=');
							size_t posC = input[i].find(';');
							input[i].replace(posB, 1, "(");
							input[i].replace(posC, 1, ")");
							input[i] = input[i].substr(posB, posC - posB + 1);

						}

					}

				}
				//	cout << input[i] << endl;
				//data_map[str] = calculate(input[i]);
				//cout << data_map[str] << endl;
				i++;
				index = i + 1;
			}


		}
		else if (input[i - 1].find("coend") != string::npos) {



		}



	}

	/*	int shmid;
	key_t key;
	int *shm, *s[10];
	key = 5678;

	shmid = shmget(key, 27, IPC_CREAT | 0666);
	shm = (int*)shmat(shmid, NULL, 0);
	for (int i = 0; i < p_count; i++) {

	s[i] = shm + i;
	}
	for (int i = 0; i < p_count; i++) {

	*s[i] = data_map[internal_var[i]];
	cout << *s[i] << endl;
	}*/
}



string st;


int initializeshm() {
	int shmid;
	key_t key;

	key = 5678;
	shmid = shmget(key, 27, IPC_CREAT | 0666);
	shm = (int*)shmat(shmid, NULL, 0);
	for (int i = 0; i < p_count; i++) {

		s[i] = shm + i;

	}
	return 0;

}
int main(int argc, char* argv[3]) {
	string1 = argv[1];
	string2 = argv[2];
	myFile1.open(string1.c_str());
	myFile2.open(string2.c_str());
	assignment2();
	initializeshm();
	int pid[p_count];
	sem_t *sem[10];
	for (int i = 0; i < p_count; i++) {
		string str = "sem-" + i;
		sem[i] = sem_open(str.c_str(), O_CREAT, 0644, 0);
	}
	for (int i = 0; i < p_count; i++) {
		pid[i] = fork();
		if (pid[i] == 0) {
			for (int n = 0; n < p_count; n++) {

				if (input[beginidx].find(internal_var[n]) != string::npos) {
					sem_wait(sem[n]);
				}
				if (input[i + beginidx].find(internal_var[n]) != string::npos) {
					size_t posB = input[i + beginidx].find(internal_var[n]);
					input[i + beginidx].replace(posB, internal_var[n].length(), to_string(*s[n]));
				}
				//cout << input[i + 6] << endl;
				*s[i] = calculate(input[i + 6]);
				sem_post(sem[1]);


				exit(0);
			}



		}
	}
		for (int i = 0; i < p_count; i++) {

			itos.push_back(to_string(*s[i]));

		}
		while (input[index].find("write") == string::npos) {
			istringstream cnt(input[index]);
			cnt >> st;



			for (int j = 0; j < internal_var.size(); j++) {
				if (input[index].find(internal_var[j]) != string::npos) {

					size_t posA = input[index].find(internal_var[j]);

					input[index].replace(posA, internal_var[j].length(), itos[j]);
					if (input[index].find('=') != string::npos) {
						size_t posB = input[index].find('=');
						size_t posC = input[index].find(';');
						input[index].replace(posB, 1, "(");
						input[index].replace(posC, 1, ")");
						input[index] = input[index].substr(posB, posC - posB + 1);

					}

				}

			}
			//cout << input[index] << endl;
			data_map[st] = calculate(input[index]);
			itos.push_back(to_string(data_map[st]));
			//cout << data_map[st] << endl;
			index++;
		}





		if (input[index].find("write") != string::npos) {

			for (int j = 0; j < input_var.size(); j++) {
				if (input[index].find(input_var[j]) != string::npos) {
					cout << "variable " + input_var[j] + " = " + inputvaluevector[j] << endl;


				}

			}
			for (int j = 0; j < internal_var.size(); j++) {
				if (input[index].find(internal_var[j]) != string::npos) {
					cout << "process " + internal_var[j] + " = " + itos[j] << endl;


				}

			}

		}

	}







