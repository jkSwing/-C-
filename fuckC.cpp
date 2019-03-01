#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <vector>
#include <fstream>

using namespace std;

const int typenum = 7;

enum {
    ID, NUMBER, DELIMITER, STRINGLIT, RELOP, OP, RESERVED
};

const char* const typenames[typenum] = {
        "标识符",
        "数字常量",
        "界符",
        "字符串常量",
        "关系运算符",
        "运算符",
        "保留字"
};

set<string> reserved;
string word;
int cur[typenum];
function<bool(int c)> DFAforward[typenum];

vector<map<int, int>> DFAs[typenum];
set<int> accepted[typenum];

bool goId(int c) {
    if (cur[ID] == 0) {
        if (isalpha(c) || c == '_') {
            cur[ID] = 1;
            return true;
        }
        else return false;
    }
    else {
        if (isalnum(c) || c == '_') return true;
        else return false;
    }
}

bool goNumber(int c) {
    auto &node = DFAs[NUMBER][cur[NUMBER]];
    auto it = node.find(c);
    if (it != node.end()) {
        cur[NUMBER] = it->second;
        return true;
    }
    return false;
}

bool goDelimiter(int c) {
    if (cur[DELIMITER] == 0) {
        if (c == '{' || c == '}' || c == '(' || c == ')' ||
            c == '[' || c == ']' || c == ';' || c == ',') {
            cur[DELIMITER] = 1;
            return true;
        }
        return false;
    }
    else {
        return false;
    }
}

bool goStringlit(int c) {
    if (cur[STRINGLIT] == 0) {
        if (c == '"') {
            cur[STRINGLIT] = 1;
            return true;
        }
        return false;
    }
    else if (cur[STRINGLIT] == 1) {
        if (c == '"') {
            cur[STRINGLIT] = 2;
            return true;
        }
        if (c == '\n') {
            cur[STRINGLIT] = 3;
            return false;
        }
        return true;
    }
    else return false;
}

bool goRelop(int c) {
    auto &node = DFAs[RELOP][cur[RELOP]];
    auto it = node.find(c);
    if (it != node.end()) {
        cur[RELOP] = it->second;
        return true;
    }
    return false;
}

bool goOp(int c) {
    auto &node = DFAs[OP][cur[OP]];
    auto it = node.find(c);
    if (it != node.end()) {
        cur[OP] = it->second;
        return true;
    }
    return false;
}

bool foo(int c) {
    return false;
}

void initNumber() {
    vector<map<int, int>> &dfa = DFAs[NUMBER];
    dfa.resize(7);
    for (char i = '0'; i <= '9'; ++i) dfa[0].insert({i, 1});

    for (char i = '0'; i <= '9'; ++i) dfa[1].insert({i, 1});
    dfa[1].insert({'.', 2});
    dfa[1].insert({'E', 4});
    dfa[1].insert({'e', 4});

    for (char i = '0'; i <= '9'; ++i) dfa[2].insert({i, 3});

    for (char i = '0'; i <= '9'; ++i) dfa[3].insert({i, 3});
    dfa[3].insert({'E', 4});
    dfa[3].insert({'e', 4});

    for (char i = '0'; i <= '9'; ++i) dfa[4].insert({i, 6});
    dfa[4].insert({'+', 5});
    dfa[4].insert({'-', 5});

    for (char i = '0'; i <= '9'; ++i) dfa[5].insert({i, 6});

    for (char i = '0'; i <= '9'; ++i) dfa[6].insert({i, 6});
}

void initRelop() {
    vector<map<int, int>> &dfa = DFAs[RELOP];
    dfa.resize(4);

    dfa[0].insert({'<', 1});
    dfa[0].insert({'=', 3});
    dfa[0].insert({'>', 1});
    dfa[0].insert({'!', 3});

    dfa[1].insert({'=', 2});

    dfa[3].insert({'=', 2});
}

void initOp() {
    ifstream ifs("operatorDFA.txt");
    string s;
    int n, m;
    ifs >> n >> m;
    auto &dfa = DFAs[OP];
    dfa.resize(n);
    for (int i = 0; i < m; ++i) {
        int a, b;
        char e[3];
        ifs >> a >> e >> b;
        dfa[a].insert({e[0], b});
    }

    int ac;
    while (ifs >> ac) accepted[OP].insert(ac);
}

void initAccepted() {
    accepted[ID].insert(1);

    accepted[NUMBER].insert(1);
    accepted[NUMBER].insert(3);
    accepted[NUMBER].insert(6);

    accepted[DELIMITER].insert(1);

    accepted[STRINGLIT].insert(2);

    accepted[RELOP].insert(1);
    accepted[RELOP].insert(2);
}

void init() {
    ifstream ifs("reserved.txt");
    string s;
    while (ifs >> s) reserved.insert(s);

    DFAforward[ID] = goId;
    DFAforward[NUMBER] = goNumber;
    DFAforward[DELIMITER] = goDelimiter;
    DFAforward[STRINGLIT] = goStringlit;
    DFAforward[RELOP] = goRelop;
    DFAforward[OP] = goOp;
    DFAforward[RESERVED] = foo;

    initAccepted();
    initNumber();
    initRelop();
    initOp();
}

int goDFA() {
    memset(cur, 0, sizeof(cur));

    vector<int> prev;
    for (int i = 0; i < typenum; ++i) {
        prev.push_back(i);
    }
    do {
        vector<int> valid;
        int nxtc = cin.peek();
        for (auto i : prev) {
            if (DFAforward[i](nxtc)) valid.push_back(i);
        }
        if (valid.empty()) break;
        else {
            word.push_back(static_cast<char>(cin.get()));
            swap(prev, valid);
        }
    } while (true);

    if (prev.size() == typenum) return -1; // unrecognized character or space, ignore it
    for (auto i : prev) {
        if (accepted[i].count(cur[i])) return i;
    }

    // invalid word, error
    return -2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
#ifdef LOCAL
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
#endif

   cout << "Please enter a C source file name:" << endl;
   string sourcefile;
   cin >> sourcefile;
   if (freopen(sourcefile.c_str(), "r", stdin) == NULL) {
       cout << "file doesnt exist" << endl;
       return 0;
   }
    init();
    while(cin) {
        word.clear();
        int type = goDFA();
        if (type == -1) {
            cin.get();
            continue;
        }
        if (type == -2) {
            cout << word << ' ' << "ERROR" << endl;
            continue;
        }
        if (type == ID && reserved.count(word)) type = RESERVED;
        cout << word << "  " << typenames[type] << endl;
    }
    return 0;
}