/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sal.hxx"

#include <tchar.h>

#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef SIGNULL
#define SIGNULL 0
#endif

#ifndef SIGKILL
#define SIGKILL 9
#endif

#include <signal.h>

#define MAX_MODULES 1024

/////////////////////////////////////////////////////////////////////////////
// Determines if a returned handle value is valid
/////////////////////////////////////////////////////////////////////////////

static inline bool IsValidHandle( HANDLE handle )
{
    return INVALID_HANDLE_VALUE != handle && NULL != handle;
}


#define elementsof( a ) (sizeof(a) / sizeof( (a)[0] ))

/////////////////////////////////////////////////////////////////////////////
// Retrieves function adress in another process
/////////////////////////////////////////////////////////////////////////////

#if 1
#define GetProcAddressEx( hProcess, hModule, lpProcName ) GetProcAddress( hModule, lpProcName )
#else
FARPROC WINAPI GetProcAddressEx( HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName )
{
    FARPROC lpfnProcAddress = GetProcAddress( hModule, lpProcName );

    if ( lpfnProcAddress )
    {
        DWORD   dwProcessId = GetProcessId( hProcess );

        if ( GetCurrentProcessId() != dwProcessId )
        {
            FARPROC lpfnRemoteProcAddress = NULL;
            TCHAR   szBaseName[MAX_PATH];

            if ( GetModuleBaseName( GetCurrentProcess(), hModule, szBaseName, elementsof(szBaseName) ) )
            {
                HMODULE ahModules[MAX_MODULES];
                DWORD   cbNeeded = 0;

                if ( EnumProcessModules( hProcess, ahModules, sizeof(ahModules), &cbNeeded ) )
                {
                    ULONG   nModules = cbNeeded / sizeof(ahModules[0]);

                    for ( ULONG n = 0; n < nModules; n++ )
                    {
                        TCHAR   szRemoteBaseName[MAX_PATH];

                        if ( GetModuleBaseName(
                            hProcess, ahModules[n], szRemoteBaseName, elementsof(szRemoteBaseName) ) &&
                            0 == lstrcmpi( szRemoteBaseName, szBaseName )
                            )
                        {
                            lpfnRemoteProcAddress = lpfnProcAddress;

                            if ( ahModules[n] != hModule )
                                *(LPBYTE*)&lpfnRemoteProcAddress += (LPBYTE)ahModules[n] - (LPBYTE)hModule;
                            break;
                        }
                    }
                }
            }

            lpfnProcAddress = lpfnRemoteProcAddress;
        }
    }

    return lpfnProcAddress;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Raises a signal in an other process
/////////////////////////////////////////////////////////////////////////////

static DWORD SignalToExceptionCode( int signal )
{
    switch ( signal )
    {
    case SIGSEGV:
        return EXCEPTION_ACCESS_VIOLATION;
    case SIGFPE:
        return EXCEPTION_FLT_INVALID_OPERATION;
    case SIGILL:
        return EXCEPTION_ILLEGAL_INSTRUCTION;
    case SIGINT:
        return CONTROL_C_EXIT;
    case SIGBREAK:
        return CONTROL_C_EXIT;
    default:
        return 0;
    }
}

static BOOL RaiseSignalEx( HANDLE hProcess, int sig )
{
    DWORD   dwProcessId = GetProcessId( hProcess );

    HANDLE  hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
    HANDLE  hThread = 0;
    BOOL fSuccess = FALSE;

    if ( IsValidHandle(hSnapshot) )
    {
        THREADENTRY32   te;

        te.dwSize = sizeof(te);
        fSuccess = Thread32First( hSnapshot, &te );
        while ( fSuccess )
        {
            if ( te.th32OwnerProcessID == dwProcessId )
            {
                hThread = OpenThread(
                    THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION |
                    THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,
                    FALSE, te.th32ThreadID );
                if ( IsValidHandle(hThread) )
                    break;
            }

            fSuccess = Thread32Next( hSnapshot, &te );
        }

        CloseHandle( hSnapshot );
    }

    if ( fSuccess )
    {
        CONTEXT aContext;

        if ( SuspendThread( hThread ) != (DWORD)-1 )
        {
            ZeroMemory( &aContext, sizeof(aContext) );
            aContext.ContextFlags = CONTEXT_FULL;

            fSuccess = GetThreadContext( hThread, &aContext );

            if ( fSuccess )
            {
                if ( sig )
                {
                    DWORD   dwStackBuffer[] =
                    {
                        aContext.Eip,
                        SignalToExceptionCode( sig ),
                        EXCEPTION_NONCONTINUABLE,
                        0,
                        0
                    };

                    aContext.Esp -= sizeof(dwStackBuffer);
                    WriteProcessMemory( hProcess, (LPVOID)aContext.Esp, dwStackBuffer, sizeof(dwStackBuffer), NULL );
                    aContext.Eip = (DWORD)GetProcAddressEx( hProcess, GetModuleHandleA("KERNEL32"), "RaiseException" );
                }
                else
                {
                    aContext.Ecx = aContext.Eax = aContext.Ebx = aContext.Edx = aContext.Esi = aContext.Edi = 0;
                }

                fSuccess = SetThreadContext( hThread, &aContext );
            }

            fSuccess = ResumeThread( hThread ) && fSuccess;

            DWORD   dwLastError = GetLastError();
            CloseHandle( hThread );
            SetLastError( dwLastError );

            return fSuccess;
        }
    }

    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// Command line parameter parsing
/////////////////////////////////////////////////////////////////////////////

static void ParseCommandArgs( LPDWORD lpProcesses, LPDWORD lpdwNumProcesses, int *pSig )
{
    typedef struct _SignalEntry
    {
        LPCTSTR lpSignalName;
        int iSignalValue;
    } SignalEntry;

    #define SIG_ENTRY( signal ) { TEXT(#signal), SIG##signal }

    static SignalEntry SupportedSignals[] =
    {
        SIG_ENTRY( NULL ),
        SIG_ENTRY( SEGV ),
        SIG_ENTRY( ILL ),
        SIG_ENTRY( FPE ),
        SIG_ENTRY( INT ),
        SIG_ENTRY( BREAK ),
        SIG_ENTRY( TERM ),
        SIG_ENTRY( ABRT ),
        SIG_ENTRY( KILL )
    };

    const int NumSupportedSignals = elementsof(SupportedSignals);

    DWORD   dwMaxProcesses = *lpdwNumProcesses;
    int     argc = __argc;
    TCHAR   **argv = __targv;

    *lpdwNumProcesses = 0;

    for ( int argn = 1; argn < argc; argn++ )
    {
        if ( 0 == lstrcmpi( argv[argn], TEXT("-l") ) ||
             0 == lstrcmpi( argv[argn], TEXT("/l") ) )

        {
            for ( int n = 0; n < NumSupportedSignals; n++ )
            {
                _tprintf( _T("%s "), SupportedSignals[n] );
            }
            _tprintf( _T("\n") );
            ExitProcess( 0 );
        }
        else if ( 0 == lstrcmpi( argv[argn], TEXT("-?") ) ||
                  0 == lstrcmpi( argv[argn], TEXT("/?") ) ||
                  0 == lstrcmpi( argv[argn], TEXT("-h") ) ||
                  0 == lstrcmpi( argv[argn], TEXT("/h") ) ||
                  0 == lstrcmpi( argv[argn], TEXT("--help") ) )
        {
            _tprintf(
                _T("Terminates a process by sending a signal.\n\n")
                _T("Usage: kill [ -l ] [ -signal ] pid ...\n\n")
                _T("-l        Lists supported signals\n")
                _T("-signal   Sends the specified signal to the given processes.\n")
                _T("          signal can be a numeric value specifying the signal number\n")
                _T("          or a string listed by the -l parameter. If no signal is\n")
                _T("          given SIGTERM (-TERM) is used.\n")
                _T("pid       Process id(s) or executables names(s) of processes to \n")
                _T("          signal or terminate.\n\n")
                );
            ExitProcess( 0 );
        }
        else if ( argv[argn] && ( *argv[argn] == '-' || *argv[argn] == '/' ) )
        {
            LPCTSTR argsig = CharNext( argv[argn] );

            int n;
            for ( n = 0; n < NumSupportedSignals; n++ )
            {
                _TCHAR *endptr = NULL;

                if ( 0 == lstrcmpi( SupportedSignals[n].lpSignalName, argsig ) ||
                     _tcstoul( argsig, &endptr, 0 ) == static_cast< unsigned >(SupportedSignals[n].iSignalValue) && (!endptr || !*endptr) )
                {
                    *pSig = SupportedSignals[n].iSignalValue;
                    break;
                }
            }

            if ( n >= NumSupportedSignals )
            {
                _ftprintf( stderr,
                    _T("kill: Illegal argument %s\n")
                    _T("Type 'kill --help' to show allowed syntax.\n")
                    _T("Type 'kill -l' to show supported signals.\n"),
                    argv[argn] );
                ExitProcess( 0 );
            }
        }
        else
        {
            unsigned long value = 0;
            _TCHAR  *endptr = NULL;

            value = _tcstoul( argv[argn], &endptr, 0 );

            if ( !endptr || !*endptr )
            {
                if ( *lpdwNumProcesses < dwMaxProcesses )
                {
                    *(lpProcesses++) = value;
                    (*lpdwNumProcesses)++;
                }
            }
            else
            {
                HANDLE  hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

                if ( IsValidHandle( hSnapshot ) )
                {
                    PROCESSENTRY32  pe;

                    pe.dwSize = sizeof(pe);
                    BOOL fSuccess = Process32First( hSnapshot, &pe );

                    while ( fSuccess )
                    {
                        if ( 0 == lstrcmpi( argv[argn], pe.szExeFile ) )
                        {
                            if ( *lpdwNumProcesses < dwMaxProcesses )
                            {
                                *(lpProcesses++) = pe.th32ProcessID;
                                (*lpdwNumProcesses)++;
                            }
                        }
                        fSuccess = Process32Next( hSnapshot, &pe );
                    }

                    CloseHandle( hSnapshot );
                }
            }
        }
    }

    if ( !*lpdwNumProcesses )
    {
        _ftprintf( stderr,
            _T("kill: No process specified.\n")
            _T("Use kill --help to show allowed syntax.\n")
            );
        ExitProcess( 0 );
    }

}

void OutputSystemMessage( DWORD dwErrorCode )
{
    LPVOID lpMsgBuf;
    FormatMessageA(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dwErrorCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (LPSTR)&lpMsgBuf,
                    0,
                    NULL
                );

    printf( (LPSTR)lpMsgBuf );
    LocalFree( lpMsgBuf );
}

int _tmain()
{
    DWORD   dwProcessIds[1024];
    DWORD   nProcesses = elementsof(dwProcessIds);
    int     sig = SIGTERM;


    ParseCommandArgs( dwProcessIds, &nProcesses, &sig );

    for ( ULONG n = 0; n < nProcesses; n++ )
    {
        HANDLE  hProcess;

        _tprintf( _T("Sending signal to process id %d..."), dwProcessIds[n] );
        hProcess = OpenProcess( PROCESS_TERMINATE | PROCESS_CREATE_THREAD | SYNCHRONIZE |
            PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
            FALSE, dwProcessIds[n] );

        if ( IsValidHandle( hProcess ) )
        {
            if ( SIGKILL == sig )
                TerminateProcess( hProcess, 255 );
            else
            {
                if ( RaiseSignalEx( hProcess, sig ) )
                    _tprintf( _T("OK\n") );
                else
                {
                    OutputSystemMessage( GetLastError() );
                }
            }

            CloseHandle( hProcess );
        }
        else
        {
            OutputSystemMessage( GetLastError() );
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
