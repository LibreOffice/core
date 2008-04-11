/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: guistdio.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_desktop.hxx"

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define _UNICODE
#include <tchar.h>

#include <string.h>
#include <stdlib.h>
#include <systools/win32/uwinapi.h>

//---------------------------------------------------------------------------
// Thread that reads from child process standard output pipe
//---------------------------------------------------------------------------

DWORD WINAPI OutputThread( LPVOID pParam )
{
    BYTE    aBuffer[256];
    DWORD   dwRead = 0;
    HANDLE  hReadPipe = (HANDLE)pParam;

    while ( ReadFile( hReadPipe, &aBuffer, sizeof(aBuffer), &dwRead, NULL ) )
    {
        BOOL    fSuccess;
        DWORD   dwWritten;

        fSuccess = WriteFile( GetStdHandle( STD_OUTPUT_HANDLE ), aBuffer, dwRead, &dwWritten, NULL );
    }

    return 0;
}

//---------------------------------------------------------------------------
// Thread that reads from child process standard error pipe
//---------------------------------------------------------------------------

DWORD WINAPI ErrorThread( LPVOID pParam )
{
    BYTE    aBuffer[256];
    DWORD   dwRead = 0;
    HANDLE  hReadPipe = (HANDLE)pParam;

    while ( ReadFile( hReadPipe, &aBuffer, sizeof(aBuffer), &dwRead, NULL ) )
    {
        BOOL    fSuccess;
        DWORD   dwWritten;

        fSuccess = WriteFile( GetStdHandle( STD_ERROR_HANDLE ), aBuffer, dwRead, &dwWritten, NULL );
    }

    return 0;
}

//---------------------------------------------------------------------------
// Thread that writes to child process standard input pipe
//---------------------------------------------------------------------------

DWORD WINAPI InputThread( LPVOID pParam )
{
    BYTE    aBuffer[256];
    DWORD   dwRead = 0;
    HANDLE  hWritePipe = (HANDLE)pParam;

    while ( ReadFile( GetStdHandle( STD_INPUT_HANDLE ), &aBuffer, sizeof(aBuffer), &dwRead, NULL ) )
    {
        BOOL    fSuccess;
        DWORD   dwWritten;

        fSuccess = WriteFile( hWritePipe, aBuffer, dwRead, &dwWritten, NULL );
    }

    return 0;
}

//---------------------------------------------------------------------------
// Thread that waits until child process reached input idle
//---------------------------------------------------------------------------

DWORD WINAPI WaitForUIThread( LPVOID pParam )
{
    HANDLE  hProcess = (HANDLE)pParam;

#ifndef GUISTDIO_KEEPRUNNING
    if ( !_tgetenv( TEXT("GUISTDIO_KEEPRUNNING") ) )
        WaitForInputIdle( hProcess, INFINITE );
    else
#endif
        WaitForSingleObject( hProcess, INFINITE );

    return 0;
}


//---------------------------------------------------------------------------
// Ctrl-Break handler that terminates the child process if Ctrl-C was pressed
//---------------------------------------------------------------------------

HANDLE  hTargetProcess = INVALID_HANDLE_VALUE;

BOOL WINAPI CtrlBreakHandler(
  DWORD   //  control signal type
)
{
    TerminateProcess( hTargetProcess, 255 );
    return TRUE;
}


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#ifdef __MINGW32__
int main( int, char ** )
#else
int _tmain( int, _TCHAR ** )
#endif
{
    TCHAR               szTargetFileName[MAX_PATH] = TEXT("");
    STARTUPINFO         aStartupInfo;
    PROCESS_INFORMATION aProcessInfo;

    ZeroMemory( &aStartupInfo, sizeof(aStartupInfo) );
    aStartupInfo.cb = sizeof(aStartupInfo);
    aStartupInfo.dwFlags = STARTF_USESTDHANDLES;

    // Create an output pipe where the write end is inheritable

    HANDLE  hOutputRead, hOutputWrite;

    if ( CreatePipe( &hOutputRead, &hOutputWrite, NULL, 0 ) )
    {
        HANDLE  hTemp;

        DuplicateHandle( GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hTemp, 0, TRUE, DUPLICATE_SAME_ACCESS );
        CloseHandle( hOutputWrite );
        hOutputWrite = hTemp;

        aStartupInfo.hStdOutput = hOutputWrite;
    }

    // Create an error pipe where the write end is inheritable

    HANDLE  hErrorRead, hErrorWrite;

    if ( CreatePipe( &hErrorRead, &hErrorWrite, NULL, 0 ) )
    {
        HANDLE  hTemp;

        DuplicateHandle( GetCurrentProcess(), hErrorWrite, GetCurrentProcess(), &hTemp, 0, TRUE, DUPLICATE_SAME_ACCESS );
        CloseHandle( hErrorWrite );
        hErrorWrite = hTemp;

        aStartupInfo.hStdError = hErrorWrite;
    }

    // Create an input pipe where the read end is inheritable

    HANDLE  hInputRead, hInputWrite;

    if ( CreatePipe( &hInputRead, &hInputWrite, NULL, 0 ) )
    {
        HANDLE  hTemp;

        DuplicateHandle( GetCurrentProcess(), hInputRead, GetCurrentProcess(), &hTemp, 0, TRUE, DUPLICATE_SAME_ACCESS );
        CloseHandle( hInputRead );
        hInputRead = hTemp;

        aStartupInfo.hStdInput = hInputRead;
    }

    // Get image path with same name but with .exe extension

    TCHAR               szModuleFileName[MAX_PATH];

    GetModuleFileName( NULL, szModuleFileName, MAX_PATH );
    _TCHAR  *lpLastDot = _tcsrchr( szModuleFileName, '.' );
    if ( lpLastDot && 0 == _tcsicmp( lpLastDot, _T(".COM") ) )
    {
        size_t len = lpLastDot - szModuleFileName;
        _tcsncpy( szTargetFileName, szModuleFileName, len );
        _tcsncpy( szTargetFileName + len, _T(".EXE"), sizeof(szTargetFileName)/sizeof(szTargetFileName[0]) - len );
    }

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes

    BOOL    fSuccess = CreateProcess(
        szTargetFileName,
        GetCommandLine(),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &aStartupInfo,
        &aProcessInfo );

    if ( fSuccess )
    {
        // These pipe ends are inherited by the child process and no longer used
        CloseHandle( hOutputWrite );
        CloseHandle( hErrorWrite );
        CloseHandle( hInputRead );

        // Set the Ctrl-Break handler
        hTargetProcess = aProcessInfo.hProcess;
        SetConsoleCtrlHandler( CtrlBreakHandler, TRUE );

        // Create threads that redirect remote pipe io to current process's console stdio

        DWORD   dwOutputThreadId, dwErrorThreadId, dwInputThreadId;

        HANDLE  hOutputThread = CreateThread( NULL, 0, OutputThread, (LPVOID)hOutputRead, 0, &dwOutputThreadId );
        HANDLE  hErrorThread = CreateThread( NULL, 0, OutputThread, (LPVOID)hErrorRead, 0, &dwErrorThreadId );
        HANDLE  hInputThread = CreateThread( NULL, 0, InputThread, (LPVOID)hInputWrite, 0, &dwInputThreadId );

        // Create thread that wait until child process entered input idle

        DWORD   dwWaitForUIThreadId;
        HANDLE  hWaitForUIThread = CreateThread( NULL, 0, WaitForUIThread, (LPVOID)aProcessInfo.hProcess, 0, &dwWaitForUIThreadId );

        DWORD   dwWaitResult;
        bool    bDetach = false;
        int     nOpenPipes = 3;
        HANDLE  hObjects[] =
            {
                hTargetProcess,
                hWaitForUIThread,
                hInputThread,
                hOutputThread,
                hErrorThread
            };

        do
        {
            dwWaitResult = WaitForMultipleObjects( elementsof(hObjects), hObjects, FALSE, INFINITE );

            switch ( dwWaitResult )
            {
            case WAIT_OBJECT_0: // The child process has terminated
            case WAIT_OBJECT_0 + 1: // The child process entered input idle
                bDetach = true;
                break;
            case WAIT_OBJECT_0 + 2: // The remote end of stdin pipe was closed
            case WAIT_OBJECT_0 + 3: // The remote end of stdout pipe was closed
            case WAIT_OBJECT_0 + 4: // The remote end of stderr pipe was closed
                bDetach = --nOpenPipes <= 0;
                break;
            default: // Something went wrong
                bDetach = true;
                break;
            }
        } while( !bDetach );

        //Even if the child process terminates it is not garanteed that all three pipe threads terminate
        //as tests have proven. The loop above will be typically terminate because the process has
        //terminated. Then the pipe threads may not have read all data from the pipes yet. When we close
        //the threads then data may be lost. For example running unopkg without arguments shall print out
        //the help text. Without this workaround some text would be missing.
        //ifdef only for unopkg
#ifdef GUISTDIO_KEEPRUNNING
        Sleep(1000);
#endif
        CloseHandle( hOutputThread );
        CloseHandle( hErrorThread );
        CloseHandle( hInputThread );
        CloseHandle( hWaitForUIThread );

        DWORD   dwExitCode = 0;
        GetExitCodeProcess( aProcessInfo.hProcess, &dwExitCode );
        CloseHandle( aProcessInfo.hProcess );
        CloseHandle( aProcessInfo.hThread );

        return dwExitCode;
    }

    return -1;
}
