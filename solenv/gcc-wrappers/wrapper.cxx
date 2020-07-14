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

string getexe(string exename, bool maybeempty) {
    char* cmdbuf;
    size_t cmdlen;
    _dupenv_s(&cmdbuf,&cmdlen,exename.c_str());
    if(!cmdbuf) {
        if (maybeempty) {
            return string();
        }
        cout << "Error " << exename << " not defined. Did you forget to source the environment?" << endl;
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
    if (libbuf == nullptr) {
        std::cerr << "No environment variable ILIB" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    libpath.append(libbuf);
    free(libbuf);
    if(_putenv(libpath.c_str())<0) {
        cerr << "Error: could not export LIB" << endl;
        exit(1);
    }

    // Set-up include path
    string includepath="INCLUDE=.";
    char* incbuf;
    size_t inclen;
    _dupenv_s(&incbuf,&inclen,"SOLARINC");
    if (incbuf == nullptr) {
        std::cerr << "No environment variable SOLARINC" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    string inctmp(incbuf);
    free(incbuf);

    // 3 = strlen(" -I")
    for(size_t pos=0,len=0;pos<inctmp.length();) {
        size_t endpos=inctmp.find(" -I",pos+1);
        if(endpos==string::npos)
            endpos=inctmp.length();
        len=endpos-pos;

        while(len>0&&inctmp[pos+len-1]==' ')
            --len;

        if(len>3) {
            includepath.append(";");
            includepath.append(inctmp,pos+3,len-3);
        }
        pos=endpos;
    }
    if(_putenv(includepath.c_str())<0) {
        cerr << "Error: could not export INCLUDE" << endl;
        exit(1);
    }
}

string processccargs(vector<string> rawargs, string &env_prefix, bool &verbose)
{
    // default env var prefix
    env_prefix = "REAL_";
    verbose = false;
    bool env_prefix_next_arg = false;

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
    args.append(" -Ob1 -Oxs -Oy-");

    // apparently these must be at the end
    // otherwise configure tests may fail
    // note: always use -debug so a PDB file is created
    string linkargs(" -link -debug");

    for(vector<string>::iterator i = rawargs.begin(); i != rawargs.end(); ++i) {
        if (env_prefix_next_arg)
        {
            env_prefix = *i;
            env_prefix_next_arg = false;
            continue;
        }

        args.append(" ");
        if(*i == "-o") {
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
            else if (dot == string::npos)
            {
                args.append("-Fe");
                args.append(*i + ".exe");
            }
            else
            {
                cerr << "unknown -o argument - please adapt gcc-wrapper for \""
                     << (*i) << "\"" << endl;
                exit(1);
            }
        }
        else if(*i == "-g" || !(*i).compare(0,5,"-ggdb")) {
            args.append("-Zi");
            args.append(" -FS");
        }
        else if(!(*i).compare(0,2,"-D")) {
            // need to re-escape strings for preprocessor
            for(size_t pos=(*i).find("\""); pos!=string::npos; pos=(*i).find("\"",pos)) {
                (*i).replace(pos,0,"\\");
                pos+=2;
            }
            args.append(*i);
        }
        else if(!(*i).compare(0,2,"-L")) {
            linkargs.append(" -LIBPATH:"+(*i).substr(2));
        }
        else if(!(*i).compare(0,2,"-l") && (*i).compare(0,5,"-link")) {
            linkargs.append(" "+(*i).substr(2)+".lib");
        }
        else if(!(*i).compare(0,5,"-def:") || !(*i).compare(0,5,"/def:")) {
            // why are we invoked with /def:? cl.exe should handle plain
            // "foo.def" by itself
            linkargs.append(" " + *i);
        }
        else if(!(*i).compare(0,12,"-fvisibility") || *i == "-fPIC") {
            //TODO: drop other gcc-specific options
        }
        else if(!(*i).compare(0,4,"-Wl,")) {
            //TODO: drop other gcc-specific options
        }
        else if(*i == "-Werror")
            args.append("-WX");
        else if (*i == "--wrapper-print-cmdline")
            verbose = true;
        else
        {
            size_t pos = i->find("=");
            if (0 == i->compare(0, pos, "--wrapper-env-prefix"))
            {
                if (pos == string::npos)
                    env_prefix_next_arg = true;
                else if (pos + 1 == i->length())
                {
                    // bailout - missing arg
                    env_prefix_next_arg = true;
                    break;
                }
                else
                    env_prefix = i->substr(pos + 1);
            }
            else
                args.append(*i);
        }
    }

    if (env_prefix_next_arg)
    {
        cerr << "wrapper-env-prefix needs an argument!" << endl;
        exit(1);
    }

    args.append(linkargs);
    return args;
}

int startprocess(string command, string args, bool verbose)
{
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

    auto cmdline = "\"" + command + "\" " + args;

    if (verbose)
        cerr << "CMD= " << command << " " << args << endl;

    // Commandline may be modified by CreateProcess
    char* cmdlineBuf=_strdup(cmdline.c_str());

    if(!CreateProcess(nullptr, // Process Name
        cmdlineBuf, // Command Line
        nullptr, // Process Handle not Inheritable
        nullptr, // Thread Handle not Inheritable
        TRUE, // Handles are Inherited
        0, // No creation flags
        nullptr, // Environment for process
        nullptr, // Use same starting directory
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
        int success=ReadFile(childout_read,buffer,BUFLEN,&readlen,nullptr);
        // check if the child process has exited
        if(GetLastError()==ERROR_BROKEN_PIPE)
            break;
        if(!success) {
            cerr << "Error: could not read from subprocess stdout" << endl;
            exit(1);
        }
        if(readlen!=0) {
            WriteFile(stdout_handle,buffer,readlen,&writelen,nullptr);
        }
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &ret);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return int(ret);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
