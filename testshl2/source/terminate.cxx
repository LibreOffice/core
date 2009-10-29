/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: terminate.cxx,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef SOLARIS
#include <sys/time.h>
#include <sys/types.h>
#endif

#include <signal.h>

#include <iostream>
#include <string>

#include "testshl/getopt.hxx"

#if (defined UNX) || (defined OS2)
#include <unistd.h> /* usleep */
#include <sys/types.h>
#include <signal.h>
#endif

#ifdef WNT
# include "testshl/winstuff.hxx"
#endif

using namespace std;

// -----------------------------------------------------------------------------
class ProcessHandler
{
    std::string m_sProcessIDFilename;
    int m_nPID;

    int  getPID();
    int readPIDFromFile();
    void sendSignal(int _nPID);
    void write(int);
public:
    ProcessHandler();
    void setName(std::string const& _sFilename);

    ~ProcessHandler();

    void waitForPIDFile(int);
    void waitForTimeout(int);
};

void my_sleep(int sec)
{
#ifdef WNT
            WinSleep(sec * 1000);
#else
            usleep(sec * 1000000); // 10 ms
#endif
}

// ------------------------------- ProcessHelper -------------------------------
ProcessHandler::ProcessHandler():m_nPID(0) {}

void ProcessHandler::setName(std::string const& _sPIDFilename)
{
    m_sProcessIDFilename = _sPIDFilename;
}

int ProcessHandler::getPID()
{
    return m_nPID;
}

int ProcessHandler::readPIDFromFile()
{
    // get own PID
    int nPID = 0;
    if (m_sProcessIDFilename.size() > 0)
    {
        FILE* in;
        in = fopen(m_sProcessIDFilename.c_str(), "r");
        if (!in)
        {
            // fprintf(stderr, "warning: (testshl.cxx) can't read own pid.\n");
            return 0;
            // exit(0);
        }
        // if file exist, wait short, maybe the other tool writes it down.
        if (fscanf(in, "%d", &nPID) != 1)
        {
            // fprintf(stderr, "warning: (testshl.cxx) can't read own pid.\n");
            return 0;
            // exit(0);
        }
        fclose(in);
    }
    else
    {
        fprintf(stderr, "error: (terminate.cxx) PID Filename empty, must set.\n");
        exit(0);
    }
    return nPID;
}

ProcessHandler::~ProcessHandler()
{
}

void ProcessHandler::sendSignal(int _nPID)
{
    if (_nPID != 0)
    {
#ifdef WNT
        WinTerminateApp(_nPID, 100);
#else
        kill(_nPID, SIGKILL);
#endif
    }
}

// -----------------------------------------------------------------------------
void ProcessHandler::waitForPIDFile(int _nTimeout)
{
    int nWaitforTimeout = _nTimeout;
    while (getPID() == 0 && nWaitforTimeout > 0)
    {
        int nPID = readPIDFromFile();
        if (nPID != 0)
        {
            m_nPID = nPID;
            break;
        }

        my_sleep(1);
        fprintf(stderr, "wait for pid file\n");
        nWaitforTimeout--;
    }

    if (nWaitforTimeout <= 0)
    {
        fprintf(stderr, "No PID found, time runs out\n");
        exit(1);
    }
}
// -----------------------------------------------------------------------------
void ProcessHandler::waitForTimeout(int _nTimeout)
{
    int nTimeout = _nTimeout;
    while (nTimeout > 0)
    {
        my_sleep(1);
        fprintf(stderr, "%d   \r", nTimeout);

        int nNewPID = readPIDFromFile();
        if ( nNewPID != getPID() )
        {
            fprintf(stderr, "PID has changed.\n");
            if ( nNewPID != 0)
            {
                fprintf(stderr, "new PID is not 0, maybe forgotten to delete old PID file, restart timeout.\n");
                m_nPID = nNewPID;
                nTimeout = _nTimeout;
            }
            else
            {
                break;
            }
        }
        nTimeout --;
    }
    if (nTimeout <= 0)
    {
        fprintf(stderr, "PID: %d\n", getPID());
        sendSignal(getPID());
        write(0);
    }
}

void ProcessHandler::write(int _nPID)
{
    // get own PID

    if (m_sProcessIDFilename.size() > 0)
    {
        FILE* out;
        out = fopen(m_sProcessIDFilename.c_str(), "w");
        if (!out)
        {
            fprintf(stderr, "warning: (testshl.cxx) can't write own pid.\n");
            return;
            // exit(0);
        }
        fprintf(out, "%d", _nPID);
        fclose(out);
    }
    else
    {
        fprintf(stderr, "warning: (testshl.cxx) PID Filename empty, must set.\n");
    }
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int, char* argv[] )
#else
int _cdecl main( int, char* argv[] )
#endif
{
    static char const * optionSet[] = {
        "-version,  shows current program version and exit.",
        "-pid=s,    write current process id to file",
        "-time=s,   timeout [default is 10 sec]",
        "-h:s,      display help or help on option",
        "-help:s,   see -h",
        NULL
    };

    ProcessHandler aCurrentProcess;

    GetOpt opt( argv, optionSet );
    if ( opt.hasOpt("-pid") )
    {
        aCurrentProcess.setName(opt.getOpt("-pid").getStr());
    }

    int nTimeout = 10;
    if ( opt.hasOpt("-time"))
    {
        //
        nTimeout = opt.getOpt("-time").toInt32();
        if (nTimeout == 0)
        {
            nTimeout = 10;
        }
    }

    if ( opt.hasOpt("-version") )
    {
        fprintf(stderr, "testshl2_timeout $Revision: 1.9 $\n");
        exit(0);
    }

    // someone indicates that he needs help
    if ( opt.hasOpt( "-h" ) || opt.hasOpt( "-help" ) )
    {
        opt.showUsage();
        exit(0);
    }

    // wait until pid file exist ==============================

    aCurrentProcess.waitForPIDFile(10);

    printf("Found PID file, wait for timeout %d sec.\n", nTimeout);

    // timeout ==================================================
    aCurrentProcess.waitForTimeout(nTimeout);

    return 0;
}

