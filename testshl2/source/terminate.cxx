/*************************************************************************
 *
 *  $RCSfile: terminate.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:36:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef SOLARIS
#include <sys/time.h>
#include <sys/types.h>
#endif

#include <signal.h>

#ifdef WNT
#define UNDER_WINDOWS_DEBUGGING
// Nice feature, to debug under windows, install msdev locally and use DebugBreak() to stop a new process at a point you want.
#ifdef UNDER_WINDOWS_DEBUGGING
#include <tools/presys.h>
#include <windows.h>
#include <MAPIWin.h>
#include <tools/postsys.h>

#define VCL_NEED_BASETSD

#endif /* UNDER_WINDOWS_DEBUGGING */
#endif /* WNT */

#include <iostream>
#include <string>

#include "getopt.hxx"

#ifdef UNX
#include <unistd.h> /* usleep */
#include <sys/types.h>
#include <signal.h>
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
            Sleep(sec * 1000);
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
        fscanf(in, "%d", &nPID);
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

#ifdef WNT
#define TA_FAILED 0
#define TA_SUCCESS_CLEAN 1
#define TA_SUCCESS_KILL 2
#define TA_SUCCESS_16 3

   // Declare Callback Enum Functions.
bool CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam );

/*----------------------------------------------------------------
   DWORD WINAPI TerminateApp( DWORD dwPID, DWORD dwTimeout )

   Purpose:
      Shut down a 32-Bit Process (or 16-bit process under Windows 95)

   Parameters:
      dwPID
         Process ID of the process to shut down.

      dwTimeout
         Wait time in milliseconds before shutting down the process.

   Return Value:
      TA_FAILED - If the shutdown failed.
      TA_SUCCESS_CLEAN - If the process was shutdown using WM_CLOSE.
      TA_SUCCESS_KILL - if the process was shut down with
         TerminateProcess().
      NOTE:  See header for these defines.
   ----------------------------------------------------------------*/
DWORD WINAPI TerminateApp( DWORD dwPID, DWORD dwTimeout )
{
    HANDLE   hProc;
    DWORD   dwRet;

    // If we can't open the process with PROCESS_TERMINATE rights,
    // then we give up immediately.
    hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, false,
                        dwPID);

    if(hProc == NULL)
    {
        return TA_FAILED;
    }

    // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
    // matches your process's.
    EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM) dwPID);

    // Wait on the handle. If it signals, great. If it times out,
    // then you kill it.
    if (WaitForSingleObject(hProc, dwTimeout) != WAIT_OBJECT_0)
        dwRet= (TerminateProcess(hProc,0) ? TA_SUCCESS_KILL : TA_FAILED);
    else
        dwRet = TA_SUCCESS_CLEAN;

    CloseHandle(hProc);

    return dwRet;
}

bool CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam )
{
    DWORD dwID;

    GetWindowThreadProcessId(hwnd, &dwID);

    if(dwID == (DWORD)lParam)
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }

    return true;
}
#endif

void ProcessHandler::sendSignal(int _nPID)
{
    if (_nPID != 0)
    {
#ifdef WNT
        TerminateApp(_nPID, 100);
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
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    static char* optionSet[] = {
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
        fprintf(stderr, "testshl2_timeout $Revision: 1.2 $\n");
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

