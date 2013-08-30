/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

#include <stdio.h>
#include <sal/macros.h>

#ifdef UNOPKG

DWORD passOutputToConsole(HANDLE readPipe, HANDLE console)
{
    BYTE aBuffer[1024];
    DWORD dwRead = 0;
    HANDLE hReadPipe = readPipe;
    DWORD dwWritten;

    //Indicates that we read an odd number of bytes. That is, we only read half of the last
    //wchar_t
    bool bIncompleteWchar = false;
    //fprintf, fwprintf will both send char data without the terminating zero.
    //fwprintf converts the unicode string first.
    //We expect here to receive unicode without the terminating zero.
    //unopkg and the extension manager code MUST
    //use dp_misc::writeConsole instead of using fprintf, etc.

    DWORD dwToRead = sizeof(aBuffer);
    BYTE * pBuffer = aBuffer;
    while ( ReadFile( hReadPipe, pBuffer, dwToRead, &dwRead, NULL ) )
    {
        //If the previous ReadFile call read an odd number of bytes, then the last one was
        //put at the front of the buffer. We increase the number of read bytes by one to reflect
        //that one byte.
        if (bIncompleteWchar)
            dwRead++;
        //We must make sure that only complete wchar_t|s are written. WriteConsolse takes
        //the number of wchar_t|s as argument. ReadFile, however, reads bytes.
        bIncompleteWchar = (dwRead % 2) ? true : false;
        if (bIncompleteWchar)
        {
            //To test this case, give aBuffer a small odd size, e.g. aBuffer[3]
            //The last byte, which is the incomplete wchar_t (half of it), will not be written.
            (void) WriteConsoleW( console, aBuffer,
                (dwRead - 1) / 2, &dwWritten, NULL );

            //Move the last byte to the front of the buffer, so that it is the start of the
            //next string
            aBuffer[0] = aBuffer[dwRead - 1];

            //Make sure that ReadFile does not overwrite the first byte the next time
            dwToRead = sizeof(aBuffer) - 1;
            pBuffer = aBuffer + 1;

        }
        else
        {   //We have read an even number of bytes. Therefore, we do not put the last incomplete
            //wchar_t at the front of the buffer. We will use the complete buffer the next time
            //when ReadFile is called.
            dwToRead = sizeof(aBuffer);
            pBuffer = aBuffer;
            (void) WriteConsoleW( console,
                aBuffer, dwRead / 2, &dwWritten, NULL );
        }
    }

    return 0;
}

#endif

#ifdef UNOPKG
DWORD WINAPI OutputThread( LPVOID pParam )
{
    return passOutputToConsole((HANDLE)pParam, GetStdHandle( STD_OUTPUT_HANDLE ));
}

#else
DWORD WINAPI OutputThread( LPVOID pParam )
{
    BYTE    aBuffer[256];
    DWORD   dwRead = 0;
    HANDLE  hReadPipe = (HANDLE)pParam;
    while ( ReadFile( hReadPipe, &aBuffer, sizeof(aBuffer), &dwRead, NULL ) )
    {
        DWORD   dwWritten;

        (void) WriteFile( GetStdHandle( STD_OUTPUT_HANDLE ), aBuffer, dwRead, &dwWritten, NULL );
    }

    return 0;
}
#endif
//---------------------------------------------------------------------------
// Thread that reads from child process standard error pipe
//---------------------------------------------------------------------------

#ifdef UNOPKG
DWORD WINAPI ErrorThread( LPVOID pParam )
{
    return passOutputToConsole((HANDLE)pParam, GetStdHandle( STD_ERROR_HANDLE ));
}

#else
DWORD WINAPI ErrorThread( LPVOID pParam )
{
    BYTE    aBuffer[256];
    DWORD   dwRead = 0;
    HANDLE  hReadPipe = (HANDLE)pParam;

    while ( ReadFile( hReadPipe, &aBuffer, sizeof(aBuffer), &dwRead, NULL ) )
    {
        DWORD   dwWritten;

        (void) WriteFile( GetStdHandle( STD_ERROR_HANDLE ), aBuffer, dwRead, &dwWritten, NULL );
    }

    return 0;
}
#endif
//---------------------------------------------------------------------------
// Thread that writes to child process standard input pipe
//---------------------------------------------------------------------------
#ifdef UNOPKG

DWORD WINAPI InputThread( LPVOID pParam )
{
    DWORD   dwRead = 0;
    HANDLE  hWritePipe = (HANDLE)pParam;

    //We need to read in the complete input until we encounter a new line before
    //converting to Unicode. This is necessary because the input string can use
    //characters of one, two, and more bytes. If the last character is not
    //complete, then it will not be converted properly.

    //Find out how a new line (0xd 0xa) looks like with the used code page.
    //Characters may have one or multiple bytes and different byte ordering
    //can be used (little and big endian);
    int cNewLine = WideCharToMultiByte(
        GetConsoleCP(), 0, L"\r\n", 2, NULL, 0, NULL, NULL);
    char * mbBuff = new char[cNewLine];
    WideCharToMultiByte(
        GetConsoleCP(), 0, L"\r\n", 2, mbBuff, cNewLine, NULL, NULL);

    const DWORD dwBufferSize = 256;
    char* readBuf = (char*) malloc(dwBufferSize);
    int readAll = 0;
    DWORD curBufSize = dwBufferSize;

    while ( ReadFile( GetStdHandle( STD_INPUT_HANDLE ),
                      readBuf + readAll,
                      curBufSize - readAll, &dwRead, NULL ) )
    {
        readAll += dwRead;
        int lastBufSize = curBufSize;
        //Grow the buffer if necessary
        if (readAll > curBufSize * 0.7)
        {
            curBufSize *= 2;
            readBuf = (char *) realloc(readBuf, curBufSize);
        }

        //If the buffer was filled completely then
        //there could be more input coming. But if we read from the console
        //and the console input fits exactly in the buffer, then the next
        //ReadFile would block until the users presses return, etc.
        //Therefor we check if last character is a new line.
        //To test this, set dwBufferSize to 4 and enter "no". This should produce
        //4 bytes with most code pages.
        if ( readAll == lastBufSize
             && memcmp(readBuf + lastBufSize - cNewLine, mbBuff, cNewLine) != 0)
        {
            //The buffer was completely filled and the last byte(s) are no
            //new line, so there is more to come.
            continue;
        }
        //Obtain the size of the buffer for the converted string.
        int sizeWBuf = MultiByteToWideChar(
            GetConsoleCP(), MB_PRECOMPOSED, readBuf, readAll, NULL, 0);

        wchar_t * wideBuf = new wchar_t[sizeWBuf];

        //Do the conversion.
        MultiByteToWideChar(
            GetConsoleCP(), MB_PRECOMPOSED, readBuf, readAll, wideBuf, sizeWBuf);

        DWORD   dwWritten;
        (void)WriteFile( hWritePipe, wideBuf, sizeWBuf * 2, &dwWritten, NULL );

        delete[] wideBuf;
        readAll = 0;
    }
    delete[] mbBuff;
    free(readBuf);
    return 0;
}
#else
DWORD WINAPI InputThread( LPVOID pParam )
{
    BYTE    aBuffer[256];
    DWORD   dwRead = 0;
    HANDLE  hWritePipe = (HANDLE)pParam;

    while ( ReadFile( GetStdHandle( STD_INPUT_HANDLE ), &aBuffer, sizeof(aBuffer), &dwRead, NULL ) )
    {
        DWORD dwWritten;
        (void) WriteFile( hWritePipe, aBuffer, dwRead, &dwWritten, NULL );
    }

    return 0;
}
#endif

//---------------------------------------------------------------------------
// Thread that waits until child process reached input idle
//---------------------------------------------------------------------------

DWORD WINAPI WaitForUIThread( LPVOID pParam )
{
#ifndef UNOPKG
    HANDLE  hProcess = (HANDLE)pParam;

    if ( !_tgetenv( TEXT("UNOPKG") ) )
        WaitForInputIdle( hProcess, INFINITE );
#else
    (void) pParam;
#endif

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
        _tcsncpy( szTargetFileName + len, _T(".EXE"), SAL_N_ELEMENTS(szTargetFileName) - len );
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

        HANDLE  hObjects[] =
            {
                hTargetProcess,
                hWaitForUIThread,
                hOutputThread,
                hErrorThread
            };

 #ifdef UNOPKG
        WaitForMultipleObjects( SAL_N_ELEMENTS(hObjects), hObjects, TRUE, INFINITE );
 #else
        bool    bDetach = false;
        int     nOpenPipes = 2;
        do
        {
            DWORD dwWaitResult = WaitForMultipleObjects( SAL_N_ELEMENTS(hObjects), hObjects, FALSE, INFINITE );

            switch ( dwWaitResult )
            {
            case WAIT_OBJECT_0: // The child process has terminated
            case WAIT_OBJECT_0 + 1: // The child process entered input idle
                bDetach = true;
                break;
            case WAIT_OBJECT_0 + 2: // The remote end of stdout pipe was closed
            case WAIT_OBJECT_0 + 3: // The remote end of stderr pipe was closed
                bDetach = --nOpenPipes <= 0;
                break;
            default: // Something went wrong
                bDetach = true;
                break;
            }
        } while( !bDetach );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
