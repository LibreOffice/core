/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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
// #include "precompiled_cppunit.hxx"

#ifdef WIN32
#include "testshl/winstuff.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <MAPIWin.h>

void WinDebugBreak()
{
    // forward to WinAPI. Note that the whole story is simply to
    // disentangle OOo headers from Windows headers (which are largely
    // incompatible, due to name clashes and macro mess-ups)
    DebugBreak();
}

void WinSleep( sal_uInt32 sec )
{
    Sleep(sec * 1000);
}

sal_uInt32 WinGetCurrentProcessId()
{
    return GetCurrentProcessId();
}

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
void WinTerminateApp( sal_uInt32 dwPID, sal_uInt32 dwTimeout )
{
    HANDLE   hProc;
    DWORD   dwRet;

    // If we can't open the process with PROCESS_TERMINATE rights,
    // then we give up immediately.
    hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, false,
                        dwPID);

    if(hProc == NULL)
        return;

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
#else

// this file is only need within Windows.
// due to the fact that empty files don't create libraries
// we store one function in it.
int WinStuffDummy_this_function_will_never_need()
{
    return 42; // we need some code
}

#endif /* WIN32 */

