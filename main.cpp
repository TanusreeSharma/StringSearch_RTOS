//Assignment 2 RTS Spring 2019
//Implementation of different String Matching algorithm and adding vxWorks hooks for task execution time measurement
//cited:
// https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
//https://www.geeksforgeeks.org/
//https://www.ee.ryerson.ca/~courses/ee8205/Data-Sheets/Tornado-VxWorks/vxworks/ref/
// Tanusree Sharma
// PSID: 1802020

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <taskLib.h>
#include <taskHookLib.h>
#include <sysLib.h>
#include <stdint.h>
#include <msgQLib.h>
#include <logLib.h>
#include <vxWorks.h>
#include <tickLib.h>
#include <taskVarLib.h>
#include <kernelLib.h>
#include "text.h"

using namespace std;

string input;//string to record data read from file
MSG_Q_ID msgid; // unique id for Message Queue
struct message
{
    string name;
    // char name;
    //char task_name[20];
    double waittime;
    double executiontime;
    double tasktime;
};

/*string readFile()
{
    string ifile = "text.h";
    string content;
    //ifstream fin(argv[1].c_str());
    ifstream in(ifile.c_str());
    char temp;
    while (in >> temp)
    {
        if ((temp >= 'a'&&temp <= 'z') || (temp >= 'A'&&temp <= 'Z'))
        {
            content += temp;
        }
    }
    
}*/

string readFile()//read in contents in text.h
{
    string rawString = srcString, content = "";
    char temp;
    for(int i=0;i<rawString.size();i++)
    {
        temp = rawString.at(i);
        if ((temp >= 'a'&&temp <= 'z') || (temp >= 'A'&&temp <= 'Z'))
        {
            content += temp;
            
        }
    }
    return content;
}

//Sorting algorithm are from https://www.geeksforgeeks.org/
// https://en.wikipedia.org/wiki/Rabin%E2%80%93Karp_algorithm
//https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm

//--------------------------------------------------------Naive Search------------------------------------------------------------------------------
int naive_search(char* srcString, char* patternString)
{
    int cnt,m,n,i,j;
    n = strlen(srcString);
    m = strlen(patternString);
    
    cnt=0;
    //loop to slide pattern one by one
    for (i=0; i <= n-m; i++)
    {
        // Finding pattern match for current index
        for (j = 0; j < m; j++)
        {
            if (srcString[i+j] != patternString[j])
                break;
        }
        
        if (j == m) //match is found
            cnt++;
        
        if(i==n/2)
            taskPrioritySet(taskIdSelf(),110);
    }
    return cnt;
}

void WD_Naive(WIND_TCB *ptcb)
{
    cout<<"\nNaive Search Count = "<< naive_search(srcString2,patternString);;
}

//-------------------------------------------------------RABIN-KARP ALGORITHM------------------------------------------------------
int rabinKarp_search(char* srcString, char* patternString)
{
    int cnt,m,n,i,j;
    n = strlen(srcString);
    m = strlen(patternString);
    
    cnt=0;
    int q = 101; // prime number
    int p = 0; // hash value for pattern
    int t = 0; // hash value for txt
    int h = 1;
    int d = 256; // number of characters in the input alphabet
    
    // The value of h would be "pow(d, m-1)%q"
    for (i = 0; i < m-1; i++)
        h = (h*d)%q;
    
    // Calculate the hash value of pattern and first
    // window of text
    for (i = 0; i < m; i++)
    {
        p = (d*p + patternString[i])%q;
        t = (d*t + srcString[i])%q;
    }
    
    // Slide the pattern over text one by one
    for (i = 0; i <= n-m; i++)
    {
        
        // Check the hash values of current window of text
        // and pattern. If the hash values match then only
        // check for characters on by one
        if ( p == t )
        {
            /* Check for characters one by one */
            for (j = 0; j < m; j++)
            {
                if (srcString[i+j] != patternString[j])
                    break;
            }
            
            // if p == t and ptrnStr[0...M-1] = txt[i, i+1, ...i+M-1]
            if (j == m)
                cnt++;
        }
        
        // Calculate hash value for next window of text: Remove
        // leading digit, add trailing digit
        if ( i < n-m )
        {
            t = (d*(t - srcString[i]*h) + srcString[i+m])%q;
            
            // We might get negative value of t, converting it
            // to positive
            if (t < 0)
                t = (t + q);
        }
        if(i==n/2)
            taskPrioritySet(taskIdSelf(),130);
    }
    return cnt;
}
void WD_rabinKarp(WIND_TCB *ptcb)
{
    cout<<"\nRabin-Karp Count = "<< rabinKarp_search(srcString2,patternString);
    
}

//-------------------------------------------------------------- KNUTH-MORRIS-PRAT search ------------------------------------------------------------------------------
void computeLPSArray(char* pat, int M, int* lps)
{
    // length of the previous longest prefix suffix
    int len = 0;
    
    lps[0] = 0; // lps[0] is always 0
    
    // the loop calculates lps[i] for i = 1 to M-1
    int i = 1;
    while (i < M)
    {
        if (pat[i] == pat[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        else // (pat[i] != pat[len])
        {
            // This is tricky. Consider the example.
            // AAACAAAA and i = 7. The idea is similar
            // to search step.
            if (len != 0)
            {
                len = lps[len - 1];
                
                // Also, note that we do not increment
                // i here
            }
            else // if (len == 0)
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

int kmp_search(char* srcString, char* patternString)
{
    int cnt,n,m;
    n = strlen(srcString);
    m = strlen(patternString);
    
    cnt = 0;
    // create lps[] that will hold the longest prefix suffix
    // values for pattern
    int lps[50];
    
    // Preprocess the pattern (calculate lps[] array)
    computeLPSArray(patternString, m, lps);
    
    int i = 0; // index for txt[]
    int j = 0; // index for pat[]
    while (i < n)
    {
        if (patternString[j] == srcString[i])
        {
            j++;
            i++;
        }
        if (j == m)
        {
            cnt++;
            j = lps[j - 1];
        }
        
        // mismatch after j matches
        else if (i < n && patternString[j] != srcString[i])
        {
            // Do not match lps[0..lps[j-1]] characters,
            // they will match anyway
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
        if(i == n/2)
            taskPrioritySet(taskIdSelf(),170);
    }
    return cnt;
}
void WD_kmp(WIND_TCB *ptcb)
{
    cout<<"\nKMP Search Count = "<< kmp_search(srcString2,patternString);
    //taskprioritySet(taskIdSelf(), 140);
}
//------------------------------------------------------------------------ End of PatternSearch --------------------------------

//int print string
void print_string()
{
    int print_times = 10;
    for(int i=0;i<print_times; i++)
        cout<<"input"<<endl;
}
//create tasks using taskSpawn
void createtasks(void)
{
    
    input = readFile();
    int id1,id2,id3,id4;
    id1 = (int)taskSpawn((char*)"naive_search",40,0,5000,(FUNCPTR)naive_search,0,0,0,0,0,0,0,0,0,0);
    id2 = (int)taskSpawn((char*)"rabinKarp_search",50,0,5000,(FUNCPTR)rabinKarp_search,0,0,0,0,0,0,0,0,0,0);
    id3 = (int)taskSpawn((char*)"kmp_search",60,0,5000,(FUNCPTR)kmp_search,0,0,0,0,0,0,0,0,0,0);
    //id1 = (int)taskSpawn((char*)"print_string",80,0,4096,(FUNCPTR)print_string,0,0,0,0,0,0,0,0,0,0);
    
    cout<<id1<<endl;
    cout<<id2<<endl;
    cout<<id2<<endl;
    cout<<id2<<endl;
}
//comparing task
int compareTargetTask(string name)
{
    if(name == "naive_search" || name == "rabinKarp_search"
       || name == "kmp_search" )
        return 1;
    return 0;
}


//-----------------------------Initializing Tasks-------------------------------------------------------------------------------
void createFunction(WIND_TCB *iniTask)
{
    iniTask->spare1 = sysTimestamp();    //storing start time
    iniTask->spare2 = 0;                    //storing total wait time
    iniTask->spare3 = iniTask->spare1;        //storing last stop time
    
}
//---------------------------------------------------------- functiion to call when deleting tasks------------------------------
void deleteFunction(WIND_TCB *deletedTask)
{
    uint64_t timeNow = sysTimestamp();
    struct message mg;
    
    if(compareTargetTask(taskName((int)deletedTask)))
    {
        //Calculating task time, waittime and execution time
        double tasktime = (timeNow - deletedTask->spare1);
        double waittime = deletedTask->spare2;
        double executiontime = tasktime - waittime;
        
        mg.name = taskName((int)deletedTask);
        
        mg.tasktime = 1.0 * tasktime * 1000 / sysTimestampFreq();//milliseconds
        mg.executiontime = executiontime * 1000 / sysTimestampFreq();
        mg.waittime= 1.0* waittime * 1000 / sysTimestampFreq();
        
        msgQSend(msgid,(char *)&mg, sizeof(struct message), WAIT_FOREVER, MSG_PRI_NORMAL);
    }
}

//--------------------------------------------Function to call when switching task when preemption happens ----------------------------------------------
//void switchFunction(WIND_TCB *pt1, WIND_TCB *pt2)
//{
//uint64_t cur_time = sysTimestamp();

//pt1->spare3 = cur_time;
//pt2->spare2 += cur_time - pt2->spare3;
// called when a  preemption occurs, record info needed
void switchFunction(WIND_TCB *pt1, WIND_TCB *pt2)
{
    uint64_t time = sysTimestamp();
    //Recoring the targetted one
    if(compareTargetTask(taskName((int)pt1)) == 1)
    {
        pt1->spare3 = time;
    }
    if(compareTargetTask(taskName((int)pt2)) == 1)
    {
        pt2->spare2 += time - pt2->spare3;
    }
}
//---------------------------------------------------Obtaining the number of message for queue and getting task information------------------------------------
int receiveMsgInfo(message msg[])
{
    int nummsg = msgQNumMsgs(msgid);
    
    for(int i=0; i<nummsg; i++)
    {
        // if(msgQReceive(msgid,(char *)&msg[i],sizeof(struct message),WAIT_FOREVER) == ERROR)
        if(msgQReceive(msgid,(char *)&msg[i],sizeof(message),-1)==ERROR)
            return (ERROR);
    }
    return nummsg;
}//--------------------------------------------------------------Displaying task information from the message queue-----------------------------------
void displayInfo(struct message* msg, int msg_number)
{
    
    for(int i=0; i<msg_number; i++)
    {
        
        cout<<"\nTask Name = "<< "msg[i].name";
        cout<<"\nTask Execution Time = "<< msg[i].executiontime << "ms";
        cout<<"\nTask Wait Time = "<< msg[i].waittime << "ms";
        cout<<"\nTotal Time Taken = "<< msg[i].tasktime << "ms";
    }
}

int main( void )
{
    message msg[1000];
    
    int msgNum = 0;
    sysTimestampEnable();
    if((msgid=msgQCreate(1000,sizeof(struct message),0))== NULL)
        return(ERROR);
    
    if(taskCreateHookAdd((FUNCPTR)createFunction)== ERROR)
        return(ERROR);
    if(taskDeleteHookAdd((FUNCPTR)deleteFunction)== ERROR)
        return(ERROR);
    if(taskSwitchHookAdd((FUNCPTR)switchFunction)== ERROR)
        return(ERROR);
    if(taskSpawn("createtasks",98,0,5000,(FUNCPTR)createtasks,0,0,0,0,0,0,0,0,0,0)==ERROR)
          return(ERROR);
    if(taskSpawn("WD_Naive", 40, 0, 5000, (FUNCPTR)WD_Naive,0,0,0,0,0,0,0,0,0,0)== ERROR)
        return(ERROR);
    
    if(taskSpawn("WD_RabinKarp", 50, 0, 5000, (FUNCPTR)WD_RabinKarp,0,0,0,0,0,0,0,0,0,0)== ERROR)
        return(ERROR);
    
    if(taskSpawn("WD_kmp", 60, 0, 5000, (FUNCPTR)WD_kmp,0,0,0,0,0,0,0,0,0,0) == ERROR)
        return(ERROR);
    
    cout<<"Tasks have been set"<<endl;
    
    if((msgNum=receiveMsgInfo(msg))== ERROR)
        return(ERROR);
    
    if(msgQDelete(msgid)==ERROR)
        return(ERROR);
    
    displayInfo(msg, msgNum);
    
    return 0;
}

