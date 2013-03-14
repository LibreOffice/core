/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "wrapper.hxx"

#include <windows.h>

#define BUFLEN 2048

string getexe(string exename) {
    char* cmdbuf;
    size_t cmdlen;
    _dupenv_s(&cmdbuf,&cmdlen,exename.c_str());
    if(!cmdbuf) {
        cout << "Error " << exename << " not defined. Did you forget to source the enviroment?" << endl;
        exit(1);
    }
    string command(cmdbuf);
    free(cmdbuf);
    return command;
}

void setupccenv() {
    // Set-up library path
    string libpath="LIB=";
    char* libbuf;
    size_t liblen;
    _dupenv_s(&libbuf,&liblen,"ILIB");
    libpath.append(libbuf);
    free(libbuf);
    if(_putenv(libpath.c_str())<0) {
        cerr << "Error: could not export LIB" << endl;
        exit(1);
    }

    // Set-up include path
    string includepath="INCLUDE=.;";
    char* incbuf;
    size_t inclen;
    _dupenv_s(&incbuf,&inclen,"SOLARINC");
    string inctmp(incbuf);
    free(incbuf);

    // 3 = strlen(" -I")
    for(size_t pos=0; pos != string::npos;) {
        size_t endpos=inctmp.find(" -I",pos+3);
        size_t len=endpos-pos-3;
        if(endpos==string::npos)
            includepath.append(inctmp,pos+3,endpos);
        else if(len>0) {
            includepath.append(inctmp,pos+3,len);
            includepath.append(";");
        }
        pos=inctmp.find(" -I",pos+len);
    }
    if(_putenv(includepath.c_str())<0) {
        cerr << "Error: could not export INCLUDE" << endl;
        exit(1);
    }
}

string processccargs(vector<string> rawargs) {
    // suppress the msvc banner
    string args=" -nologo";
    // TODO: should these options be enabled globally?
    args.append(" -EHsc");
    args.append(" -MD");

    for(vector<string>::iterator i = rawargs.begin(); i != rawargs.end(); ++i) {
        args.append(" ");
        if(*i == "-o") {
            // TODO: handle more than just exe output
            args.append("-Fe");
            ++i;
            args.append(*i);
        }
        else if(*i == "-g")
            args.append("-Zi");
        else if(!(*i).compare(0,2,"-D")) {
            // need to re-escape strings for preprocessor
            for(size_t pos=(*i).find("\"",0); pos!=string::npos; pos=(*i).find("\"",pos)) {
                (*i).replace(pos,0,"\\");
                pos+=2;
            }
            args.append(*i);
        }
        else
            args.append(*i);
    }
    return args;
}

int startprocess(string command, string args) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;

    HANDLE childout_read;
    HANDLE childout_write;

    memset(&sa,0,sizeof(sa));
    memset(&si,0,sizeof(si));
    memset(&pi,0,sizeof(pi));

    sa.nLength=sizeof(sa);
    sa.bInheritHandle=TRUE;

    if(!CreatePipe(&childout_read,&childout_write,&sa,0)) {
        cerr << "Error: could not create sdtout pipe" << endl;
        exit(1);
    }

    si.cb=sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput=childout_write;
    si.hStdError=childout_write;

    // Commandline may be modified by CreateProcess
    char* cmdline=_strdup(args.c_str());

    if(!CreateProcess(command.c_str(), // Process Name
        cmdline, // Command Line
        NULL, // Process Handle not Inheritable
        NULL, // Thread Handle not Inheritable
        TRUE, // Handles are Inherited
        0, // No creation flags
        NULL, // Enviroment for process
        NULL, // Use same starting directory
        &si, // Startup Info
        &pi) // Process Information
        ) {
            cerr << "Error: could not create process" << endl;
            exit(1);
    }

    // if you don't close this the process will hang
    CloseHandle(childout_write);

    // Get Process output
    char buffer[BUFLEN];
    DWORD readlen, writelen, ret;
    HANDLE stdout_handle=GetStdHandle(STD_OUTPUT_HANDLE);
    while(true) {
        int success=ReadFile(childout_read,buffer,BUFLEN,&readlen,NULL);
        // check if the child process has exited
        if(GetLastError()==ERROR_BROKEN_PIPE)
            break;
        if(!success) {
            cerr << "Error: could not read from subprocess stdout" << endl;
            exit(1);
        }
        if(readlen!=0) {
                WriteFile(stdout_handle,buffer,readlen,&writelen,NULL);
        }
    }
    GetExitCodeProcess(pi.hProcess, &ret);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return int(ret);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
