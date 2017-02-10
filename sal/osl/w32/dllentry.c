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

#ifdef _MSC_VER
#pragma warning(push,1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif
#include <tlhelp32.h>
#include <systools/win32/uwinapi.h>
#include <winsock.h>
#include <osl/diagnose.h>
#include <sal/types.h>
#include <float.h>

#include <osl/mutex.h>
#include <osl/thread.h>

#include "file_url.h"
#include "rtllifecycle.h"

#include <thread.h>

// externals

extern CRITICAL_SECTION g_ThreadKeyListCS;
extern oslMutex         g_Mutex;

/*
This is needed because DllMain is called after static constructors. A DLL's
startup and shutdown sequence looks like this:

_pRawDllMain()
_CRT_INIT()
DllMain()
....
DllMain()
_CRT_INIT()
_pRawDllMain()

*/

static BOOL WINAPI RawDllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved );
BOOL (WINAPI *_pRawDllMain)(HINSTANCE, DWORD, LPVOID) = RawDllMain;

static BOOL WINAPI RawDllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    (void)hinstDLL; /* avoid warnings */
    (void)lpvReserved; /* avoid warnings */

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            {
#ifdef _DEBUG
                WCHAR buf[64];
                DWORD const res = GetEnvironmentVariableW(L"SAL_NO_ASSERT_DIALOGS", buf, sizeof(buf));
                if (res && res < sizeof(buf))
                {
                    // disable the dialog on abort()
                    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
                    _CrtSetReportMode(_CRT_ERROR, (_CRTDBG_MODE_DEBUG|_CRTDBG_MODE_FILE));
                    // not sure which assertions this affects
                    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
                    _CrtSetReportMode(_CRT_ASSERT, (_CRTDBG_MODE_DEBUG|_CRTDBG_MODE_FILE));
                    // disable the dialog on assert(false)
                    _set_error_mode(_OUT_TO_STDERR);
                }
#endif

#if OSL_DEBUG_LEVEL < 2
                /* Suppress file error messages from system like "Floppy A: not inserted" */
                SetErrorMode( SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS );
#endif

                /* initialize global mutex */
                g_Mutex = osl_createMutex();

                /* initialize "current directory" mutex */
                g_CurrentDirectoryMutex = osl_createMutex();

                g_dwTLSTextEncodingIndex = TlsAlloc();
                InitializeCriticalSection( &g_ThreadKeyListCS );

                //We disable floating point exceptions. This is the usual state at program startup
                //but on Windows 98 and ME this is not always the case.
                _control87(_MCW_EM, _MCW_EM);
                break;
            }

        case DLL_PROCESS_DETACH:
            WSACleanup( );

            TlsFree( g_dwTLSTextEncodingIndex );
            DeleteCriticalSection( &g_ThreadKeyListCS );

            osl_destroyMutex( g_Mutex );

            osl_destroyMutex( g_CurrentDirectoryMutex );

            /*

            On a product build memory management finalization might
            cause a crash without assertion (assertions off) if heap is
            corrupted. But a crash report won't help here because at
            this point all other threads have been terminated and only
            ntdll is on the stack. No chance to find the reason for the
            corrupted heap if so.

            So annoying the user with a crash report is completely useless.

            */

#ifndef DBG_UTIL
            __try
#endif
            {
                /* cleanup locale hashtable */
                rtl_locale_fini();

                /* finalize memory management */
                rtl_memory_fini();
                rtl_cache_fini();
                rtl_arena_fini();
            }
#ifndef DBG_UTIL
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
            }
#endif
            break;
    }

    return TRUE;
}

static DWORD GetParentProcessId()
{
    DWORD   dwParentProcessId = 0;
    HANDLE  hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if ( IsValidHandle( hSnapshot ) )
    {
        PROCESSENTRY32  pe;
        BOOL            fSuccess;

        ZeroMemory( &pe, sizeof(pe) );
        pe.dwSize = sizeof(pe);
        fSuccess = Process32First( hSnapshot, &pe );

        while( fSuccess )
        {
            if ( GetCurrentProcessId() == pe.th32ProcessID )
            {
                dwParentProcessId = pe.th32ParentProcessID;
                break;
            }

            fSuccess = Process32Next( hSnapshot, &pe );
        }

        CloseHandle( hSnapshot );
    }

    return dwParentProcessId;
}

static DWORD WINAPI ParentMonitorThreadProc( LPVOID lpParam )
{
    DWORD_PTR dwParentProcessId = (DWORD_PTR)lpParam;

    HANDLE  hParentProcess = OpenProcess( SYNCHRONIZE, FALSE, dwParentProcessId );

    osl_setThreadName("headless ParentMonitorThread");

    if ( IsValidHandle( hParentProcess ) )
    {
        if ( WAIT_OBJECT_0 == WaitForSingleObject( hParentProcess, INFINITE ) )
        {
            TerminateProcess( GetCurrentProcess(), 0 );
        }
        CloseHandle( hParentProcess );
    }
    return 0;
}

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    (void)hinstDLL; /* avoid warning */
    (void)lpvReserved; /* avoid warning */
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            TCHAR   szBuffer[64];

            // This code will attach the process to its parent process
            // if the parent process had set the environment variable.
            // The corresponding code (setting the environment variable)
            // is desktop/win32/source/officeloader.cxx

            DWORD   dwResult = GetEnvironmentVariable( "ATTACHED_PARENT_PROCESSID", szBuffer, sizeof(szBuffer) );

            if ( dwResult && dwResult < sizeof(szBuffer) )
            {
                DWORD   dwThreadId = 0;

                DWORD_PTR   dwParentProcessId = (DWORD_PTR)atol( szBuffer );

                if ( dwParentProcessId && GetParentProcessId() == dwParentProcessId )
                {
                    // No error check, it works or it does not
                    // Thread should only be started for headless mode, see desktop/win32/source/officeloader.cxx
                    CreateThread( NULL, 0, ParentMonitorThreadProc, (LPVOID)dwParentProcessId, 0, &dwThreadId );
                    // Note: calling CreateThread in DllMain is discouraged
                    // but this is only done in the headless mode and in
                    // that case no other threads should be running at startup
                    // when sal3.dll is loaded; also there is no
                    // synchronization with the spawned thread, so there
                    // does not appear to be a real risk of deadlock here
                }
            }

            return TRUE;
        }

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            osl_callThreadKeyCallbackOnThreadDetach( );
            break;
    }

    return TRUE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
