#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <string>
#include <iostream>
#include <vector>

using namespace std;

bool LoadCSVFile(string csvFile,string &csvString){
    FILE* fp = fopen(csvFile.c_str(), "rb");
    if(fp == 0) return false;

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    char *buffer = new char[len+1];
    fseek(fp, 0, SEEK_SET);
    fread(buffer, 1, len, fp);
    fclose(fp);
    buffer[len] = '\0';
    csvString.assign(buffer, len);

    delete[] buffer;
    return true;
}

enum ParseState{
    PrepareNewField,    // prepare to start new field   
    NormalField,        // (normal char received) normal-field started
    QuotesField,        // (quotes received) quotes-field started
    SecondQuotesField , // quotes-field started,and have received second quotes
    CRReceived,         // (in normalField) have received CR
    Error,              // final state, parse error 
    Success,            // final state, parse success 
};

// Parse csv string to two-dimensional array
// use FSM(finite-state machine) to achieve parse process
// state types: ParseState
// action types: CR, LF, '"'(quotes), ','(comma), '/', other chars, end-signal 
bool ParseCSV(vector<vector<string>> &result, const string &csv){
    ParseState state = PrepareNewField;
    int commasEachLine = 0;
    int commasCurLine = 0;
    bool isFirstLine = true;
    int curLine = 0;
    vector<string> line0;
    result.push_back(line0);
    string curContent = "";

    // parse each characters 
    for(int i = 0;i < csv.size();i++){
        char ch = csv[i];
        switch(state){
        case PrepareNewField:
            {
                if(ch == '\r'){
                    state = CRReceived;
                }
                else if(ch == '\n' || ch == '/'){
                    state = Error;
                }
                else if(ch == '"'){
                    state = QuotesField;
                }
                else if(ch == ','){
                    commasCurLine ++; 
                    result[curLine].push_back("");
                    curContent = "";
                    // state unchange
                }
                else{
                    curContent += ch;
                    state = NormalField;
                }
                break;
            }
        case NormalField:
            {
                if(ch == '\r'){
                    state = CRReceived;
                }
                else if(ch == '\n' || ch == '/' || ch == '"'){
                    state = Error;
                }
                else if(ch == ','){
                    commasCurLine ++; 
                    result[curLine].push_back(curContent);
                    curContent = "";
                    state = PrepareNewField;
                }
                else{
                    curContent += ch;
                    // state unchange
                }
                break;
            }
        case QuotesField:
            {
                if(ch == '"'){
                    state = SecondQuotesField;
                }
                else{
                    curContent += ch;
                    // state unchange
                }
                break;
            }
        case SecondQuotesField:
            {
                if(ch == '\r'){
                    state = CRReceived;
                }
                else if(ch == '\n' || ch == '/'){
                    state = Error;
                }
                else if(ch == '"'){
                    curContent += '"';
                    state = QuotesField;
                }
                else if(ch == ','){
                    commasCurLine ++; 
                    result[curLine].push_back(curContent);
                    curContent = "";
                    state = PrepareNewField;
                }
                else{
                    state = Error;
                }
                break;
            }
        case CRReceived:
            {
                if(ch == '\n'){
                    if(isFirstLine || commasCurLine == commasEachLine){
                        result[curLine].push_back(curContent);
                        curContent = "";
                        vector<string> newline;
                        result.push_back(newline);
                        curLine++;
                        state = PrepareNewField;

                        if(isFirstLine){
                            commasEachLine = commasCurLine;
                            isFirstLine = false;
                        }
                        commasCurLine = 0;
                    }
                    else if(!isFirstLine){
                        state = Error;
                    }
                }
                else{
                    state = Error;
                }
                break;
            }
        case Error:
            {
                break;
            }
        default:
            assert(false);
        }

        if(state == Error) break;
    }

    // parse "end" action 
    if(state != Error){
        switch(state){
        case PrepareNewField:
        case NormalField:
        case SecondQuotesField:
            if(result[result.size()-1].size() == 0){
                // end file with CRLF
                result.erase(result.end()-1);
                state = Success;
            }
            else if(isFirstLine || commasCurLine == commasEachLine){
                // end file without CRLF
                result[curLine].push_back(curContent);
                state = Success;
            }
            else
                state = Error;
            break;
        case QuotesField:
        case CRReceived:
            state = Error;
            break;
        default:
            assert(false);
        }
    }

    if(state == Success){
        return true;
    }
    else{
        result.clear();
        return false;
    }
}

int main(int argc,char** argv)
{
    string csvString;
    bool ret = LoadCSVFile(".\\data\\case0-7.csv",csvString);
    if(ret){
        vector<vector<string>> res;
        if(ParseCSV(res,csvString))
            cout<<"parse success!\n"<<endl;
        else
            cout<<"parse failed!\n"<<endl;
    }

    return 0;
}
