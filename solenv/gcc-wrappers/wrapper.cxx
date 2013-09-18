/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "wrapper.hxx"

#define WIN32_LEAN_AND_MEAN

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
    const char *const pDebugRuntime(getenv("MSVC_USE_DEBUG_RUNTIME"));
    if (pDebugRuntime && !strcmp(pDebugRuntime, "TRUE"))
        args.append(" -MDd");
    else
        args.append(" -MD");
    args.append(" -Gy");
    args.append(" -Zc:wchar_t-");
    args.append(" -Ob1 -Oxs -Oy-");

    // apparently these must be at the end
    // otherwise configure tests may fail
    string linkargs(" -link");

    for(vector<string>::iterator i = rawargs.begin(); i != rawargs.end(); ++i) {
        args.append(" ");
        if(i->find("/") == 0) {
            cerr << "Error: absolute unix path passed that looks like an option: '" << *i << "'";
            args.append(*i);
        }
        else if(*i == "-o") {
            // TODO: handle more than just exe output
            ++i;
            size_t dot=(*i).find_last_of(".");
            if(!(*i).compare(dot+1,3,"obj") || !(*i).compare(dot+1,1,"o"))
            {
                args.append("-Fo");
                args.append(*i);
            }
            else if(!(*i).compare(dot+1,3,"exe"))
            {
                args.append("-Fe");
                args.append(*i);
            }
            else if(!(*i).compare(dot+1,3,"dll"))
            {   // apparently cl.exe has no flag for dll?
                linkargs.append(" -dll -out:");
                linkargs.append(*i);
            }
            else
            {
                cerr << "unknonwn -o argument - please adapt gcc-wrapper for \""
                     << (*i) << "\"";
                exit(1);
            }
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
        else if(!(*i).compare(0,2,"-L")) {
            linkargs.append(" -LIBPATH:"+(*i).substr(2));
        }
        else if(!(*i).compare(0,2,"-l")) {
            linkargs.append(" "+(*i).substr(2)+".lib");
        }
        else if(!(*i).compare(0,5,"-def:") || !(*i).compare(0,5,"/def:")) {
            // why are we invoked with /def:? cl.exe should handle plain
            // "foo.def" by itself
            linkargs.append(" " + *i);
        }
        else if(!(*i).compare(0,12,"-fvisibility")) {
            //TODO: drop other gcc-specific options
        }
        else if(!(*i).compare(0,4,"-Wl,")) {
            //TODO: drop other gcc-specific options
        }
        else if(*i == "-Werror")
            args.append("-WX");
        else
            args.append(*i);
    }
    args.append(linkargs);
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
        cerr << "Error: could not create stdout pipe" << endl;
        exit(1);
    }

    si.cb=sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput=childout_write;
    si.hStdError=childout_write;

    // support ccache
    size_t pos=command.find("ccache ");
    if(pos != string::npos) {
        args.insert(0,"cl.exe");
        command=command.substr(0,pos+strlen("ccache"))+".exe";
    }

    //cerr << "CMD= " << command << " " << args << endl;

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
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &ret);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return int(ret);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
