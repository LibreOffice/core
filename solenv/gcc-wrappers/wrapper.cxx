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

std::string getexe(std::string exename, bool maybeempty) {
    char* cmdbuf;
    size_t cmdlen;
    _dupenv_s(&cmdbuf,&cmdlen,exename.c_str());
    if(!cmdbuf) {
        if (maybeempty) {
            return std::string();
        }
        std::cout << "Error " << exename << " not defined. Did you forget to source the environment?" << std::endl;
        exit(1);
    }
    std::string command(cmdbuf);
    free(cmdbuf);
    return command;
}

void setupccenv() {
    // Set-up library path
    std::string libpath="LIB=";
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
        std::cerr << "Error: could not export LIB" << std::endl;
        exit(1);
    }

    // Set-up include path
    std::string includepath="INCLUDE=.";
    char* incbuf;
    size_t inclen;
    _dupenv_s(&incbuf,&inclen,"SOLARINC");
    if (incbuf == nullptr) {
        std::cerr << "No environment variable SOLARINC" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string inctmp(incbuf);
    free(incbuf);

    // 3 = strlen(" -I")
    for(size_t pos=0,len=0;pos<inctmp.length();) {
        size_t endpos=inctmp.find(" -I",pos+1);
        if(endpos==std::string::npos)
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
        std::cerr << "Error: could not export INCLUDE" << std::endl;
        exit(1);
    }
}

std::string processccargs(const std::vector<std::string>& rawargs, std::string &env_prefix, bool &verbose)
{
    // default env var prefix
    env_prefix = "REAL_";
    verbose = false;
    bool env_prefix_next_arg = false;

    // suppress the msvc banner
    std::string args=" -nologo";
    // TODO: should these options be enabled globally?
    args.append(" -EHsc");
    const char *const pDebugRuntime(getenv("MSVC_USE_DEBUG_RUNTIME"));
    if (pDebugRuntime && !strcmp(pDebugRuntime, "TRUE"))
        args.append(" -MDd");
    else
        args.append(" -MD");
    args.append(" -Gy");
    args.append(" -Ob1 -Oxs -Oy-");

    std::string linkargs;
    bool block_linkargs = false;

    // instead of using synced PDB access (-FS), use individual PDB files based on output
    const char *const pEnvIndividualPDBs(getenv("MSVC_USE_INDIVIDUAL_PDBS"));
    const bool bIndividualPDBs = (pEnvIndividualPDBs && !strcmp(pEnvIndividualPDBs, "TRUE"));
    const char *const pEnvEnableZ7Debug(getenv("ENABLE_Z7_DEBUG"));
    const bool bEnableZ7Debug = (pEnvEnableZ7Debug && !strcmp(pEnvEnableZ7Debug, "TRUE"));

    for(std::vector<std::string>::const_iterator i = rawargs.begin(); i != rawargs.end(); ++i) {
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
            else if (dot == std::string::npos)
            {
                args.append("-Fe");
                args.append(*i + ".exe");
            }
            else
            {
                std::cerr << "unknown -o argument - please adapt gcc-wrapper for \""
                     << (*i) << "\"" << std::endl;
                exit(1);
            }

            if (bIndividualPDBs && !bEnableZ7Debug)
            {
                if (dot == std::string::npos)
                    args.append(" -Fd" + *i + ".pdb");
                else
                    args.append(" -Fd" + (*i).substr(0, dot) + ".pdb");
            }
        }
        else if(*i == "-g" || !(*i).compare(0,5,"-ggdb")) {
            if(!bEnableZ7Debug)
            {
                args.append("-Zi");
                if (!bIndividualPDBs)
                    args.append(" -FS");
            }
            else
            {
                // ccache doesn't work with -Zi, the -link -debug for linking will create a final PDB
                args.append("-Z7");
            }
        }
        else if(!(*i).compare(0,2,"-D")) {
            // need to re-escape strings for preprocessor
            std::string str = *i;
            for(size_t pos=str.find("\""); pos!=std::string::npos; pos=str.find("\"",pos)) {
                str.replace(pos,0,"\\");
                pos+=2;
            }
            args.append(str);
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
        else if(*i == "-c") {
            args.append("-c");
            // If -c is specified, there will be no linking anyway,
            // and passing -link with -c stops ccache from caching.
            block_linkargs = true;
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
                if (pos == std::string::npos)
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
        std::cerr << "wrapper-env-prefix needs an argument!" << std::endl;
        exit(1);
    }

    if(!block_linkargs) {
        // apparently these must be at the end
        // otherwise configure tests may fail
        // note: always use -debug so a PDB file is created
        args.append(" -link -debug ");
        args.append(linkargs);
    }

    return args;
}

int startprocess(std::string command, std::string args, bool verbose)
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
        std::cerr << "Error: could not create stdout pipe" << std::endl;
        exit(1);
    }

    si.cb=sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput=childout_write;
    si.hStdError=childout_write;

    // support ccache
    size_t pos=command.find("ccache ");
    size_t len = strlen("ccache ");
    if(pos == std::string::npos) {
        pos=command.find("ccache.exe ");
        len = strlen("ccache.exe ");
    }
    if(pos != std::string::npos) {
        args.insert(0,command.substr(pos+len));
        command=command.substr(0,pos+len-1);
    }

    auto cmdline = "\"" + command + "\" " + args;

    if (verbose)
        std::cerr << "CMD= " << command << " " << args << std::endl;

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
            auto const e = GetLastError();
            std::cerr << "Error: could not create process \"" << cmdlineBuf << "\": " << e << std::endl;
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
            std::cerr << "Error: could not read from subprocess stdout" << std::endl;
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
