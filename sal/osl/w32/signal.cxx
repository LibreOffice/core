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

/* system headers */
#include "system.h"
#include <tchar.h>

#include "file_url.h"
#include "path_helper.hxx"

#include <osl/diagnose.h>
#include <osl/mutex.h>
#include <osl/signal.h>
#ifndef __MINGW32__
#include <DbgHelp.h>
#endif
#include <errorrep.h>
#include <systools/win32/uwinapi.h>
#include <sal/macros.h>
#include <eh.h>
#include <stdexcept>

typedef struct _oslSignalHandlerImpl
{
    oslSignalHandlerFunction      Handler;
    void*                         pData;
    struct _oslSignalHandlerImpl* pNext;
} oslSignalHandlerImpl;

static sal_Bool               bErrorReportingEnabled = sal_True;
static sal_Bool               bInitSignal = sal_False;
static oslMutex               SignalListMutex;
static oslSignalHandlerImpl*  SignalList;

static long WINAPI SignalHandlerFunction(LPEXCEPTION_POINTERS lpEP);

static sal_Bool InitSignal(void)
{
    HMODULE hFaultRep;

    SignalListMutex = osl_createMutex();

    SetUnhandledExceptionFilter(SignalHandlerFunction);

    hFaultRep = LoadLibrary( "faultrep.dll" );
    if ( hFaultRep )
    {
        pfn_ADDEREXCLUDEDAPPLICATIONW       pfn = (pfn_ADDEREXCLUDEDAPPLICATIONW)GetProcAddress( hFaultRep, "AddERExcludedApplicationW" );
        if ( pfn )
            pfn( L"SOFFICE.EXE" );
        FreeLibrary( hFaultRep );
    }

    return sal_True;
}

static sal_Bool DeInitSignal(void)
{
    SetUnhandledExceptionFilter(NULL);

    osl_destroyMutex(SignalListMutex);

    return sal_False;
}

static oslSignalAction CallSignalHandler(oslSignalInfo *pInfo)
{
    oslSignalHandlerImpl* pHandler = SignalList;
    oslSignalAction Action = osl_Signal_ActCallNextHdl;

    while (pHandler != NULL)
    {
        if ((Action = pHandler->Handler(pHandler->pData, pInfo)) != osl_Signal_ActCallNextHdl)
            break;

        pHandler = pHandler->pNext;
    }

    return Action;
}

/*****************************************************************************/
/* SignalHandlerFunction    */
/*****************************************************************************/

#define REPORTENV_PARAM     "-crashreportenv:"
#define REPORTENV_PARAM2    "/crashreportenv:"

static BOOL ReportCrash( LPEXCEPTION_POINTERS lpEP )
{
    BOOL    fSuccess = FALSE;
    BOOL    fAutoReport = FALSE;
    TCHAR   szBuffer[1024];
    ::osl::LongPathBuffer< sal_Char > aPath( MAX_LONG_PATH );
    LPTSTR  lpFilePart;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO StartupInfo;
    int     argi;

    if ( !bErrorReportingEnabled )
        return FALSE;

    /* Check if crash reporter was disabled by command line */

    for ( argi = 1; argi < __argc; argi++ )
    {
        if (
            0 == stricmp( __argv[argi], "--nocrashreport" ) ||
            0 == stricmp( __argv[argi], "/nocrashreport" )
            )
            return FALSE;
        else if (
            0 == stricmp( __argv[argi], "--autocrashreport" ) ||
            0 == stricmp( __argv[argi], "/autocrashreport" )
            )
            fAutoReport = TRUE;
        else if (
            0 == strnicmp( __argv[argi], REPORTENV_PARAM, strlen(REPORTENV_PARAM) ) ||
            0 == strnicmp( __argv[argi], REPORTENV_PARAM2, strlen(REPORTENV_PARAM2) )
            )
        {
            const char *envparam = __argv[argi] + strlen(REPORTENV_PARAM);
            const char *delim = strchr(envparam, '=' );

            if ( delim )
            {
                CHAR    *lpVariable;
                CHAR    *lpValue;
                const char *variable = envparam;
                size_t variable_len = delim - envparam;
                const char *value = delim + 1;
                size_t value_len = strlen(envparam) - variable_len - 1;

                if ( '\"' == *value )
                {
                    const char *quote;

                    value++;
                    value_len--;

                    quote = strchr( value, '\"' );
                    if ( quote )
                        value_len = quote - value;
                }

                lpVariable = reinterpret_cast< CHAR* >( _alloca( variable_len + 1 ) );
                memcpy( lpVariable, variable, variable_len );
                lpVariable[variable_len] = 0;

                lpValue = reinterpret_cast< CHAR* >( _alloca( value_len + 1) );
                memcpy( lpValue, value, value_len );
                lpValue[value_len] = 0;

                SetEnvironmentVariable( lpVariable, lpValue );
            }
        }
    }

    if ( SearchPath( NULL, TEXT( "crashrep.exe" ), NULL, aPath.getBufSizeInSymbols(), aPath, &lpFilePart ) )
    {
        ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
        StartupInfo.cb = sizeof(StartupInfo.cb);

        sntprintf( szBuffer, SAL_N_ELEMENTS(szBuffer),
            _T("%s -p %lu -excp 0x%p -t %lu%s"),
            static_cast<sal_Char*>( aPath ),
            GetCurrentProcessId(),
            lpEP,
            GetCurrentThreadId(),
            fAutoReport ? _T(" -noui -send") : _T(" -noui") );

        if (
            CreateProcess(
                NULL,
                szBuffer,
                NULL,
                NULL,
                FALSE,
#ifdef UNICODE
                CREATE_UNICODE_ENVIRONMENT,
#else
                0,
#endif
                NULL, NULL, &StartupInfo, &ProcessInfo )
            )
        {
            DWORD   dwExitCode;

            WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
            if ( GetExitCodeProcess( ProcessInfo.hProcess, &dwExitCode ) && 0 == dwExitCode )

            fSuccess = TRUE;

        }
    }

    return fSuccess;
}

/*****************************************************************************/
/* SignalHandlerFunction    */
/*****************************************************************************/

/* magic Microsoft C++ compiler exception constant */
#define EXCEPTION_MSC_CPP_EXCEPTION 0xe06d7363

static long WINAPI SignalHandlerFunction(LPEXCEPTION_POINTERS lpEP)
{
    static sal_Bool     bNested = sal_False;
    sal_Bool        bRaiseCrashReporter = sal_False;
    oslSignalInfo   Info;
    oslSignalAction Action;

    Info.UserSignal = lpEP->ExceptionRecord->ExceptionCode;
    Info.UserData   = NULL;

    switch (lpEP->ExceptionRecord->ExceptionCode)
    {
        /* Transform unhandled exceptions into access violations.
           Microsoft C++ compiler (add more for other compilers if necessary).
         */
        case EXCEPTION_MSC_CPP_EXCEPTION:
        case EXCEPTION_ACCESS_VIOLATION:
            Info.Signal = osl_Signal_AccessViolation;
            bRaiseCrashReporter = sal_True;
            break;

        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            Info.Signal = osl_Signal_IntegerDivideByZero;
            bRaiseCrashReporter = sal_True;
            break;

        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            Info.Signal = osl_Signal_FloatDivideByZero;
            bRaiseCrashReporter = sal_True;
            break;

        case EXCEPTION_BREAKPOINT:
            Info.Signal = osl_Signal_DebugBreak;
            break;

        default:
            Info.Signal = osl_Signal_System;
            bRaiseCrashReporter = sal_True;
            break;
    }

    if ( !bNested )
    {
        bNested = sal_True;

        if ( bRaiseCrashReporter && ReportCrash( lpEP ) )
        {
            CallSignalHandler(&Info);
            Action = osl_Signal_ActKillApp;
        }
        else
            Action = CallSignalHandler(&Info);
    }
    else
        Action = osl_Signal_ActKillApp;

    switch ( Action )
    {
        case osl_Signal_ActCallNextHdl:
            return (EXCEPTION_CONTINUE_SEARCH);

        case osl_Signal_ActAbortApp:
            return (EXCEPTION_EXECUTE_HANDLER);

        case osl_Signal_ActKillApp:
            SetErrorMode(SEM_NOGPFAULTERRORBOX);
            exit(255);
            break;
        default:
            break;
    }

    return (EXCEPTION_CONTINUE_EXECUTION);
}

/*****************************************************************************/
/* osl_addSignalHandler */
/*****************************************************************************/
oslSignalHandler SAL_CALL osl_addSignalHandler(oslSignalHandlerFunction Handler, void* pData)
{
    oslSignalHandlerImpl* pHandler;

    OSL_ASSERT(Handler != NULL);

    if (! bInitSignal)
        bInitSignal = InitSignal();

    pHandler = reinterpret_cast< oslSignalHandlerImpl* >( calloc( 1, sizeof(oslSignalHandlerImpl) ) );

    if (pHandler != NULL)
    {
        pHandler->Handler = Handler;
        pHandler->pData   = pData;

        osl_acquireMutex(SignalListMutex);

        pHandler->pNext = SignalList;
        SignalList      = pHandler;

        osl_releaseMutex(SignalListMutex);

        return (pHandler);
    }

    return (NULL);
}

/*****************************************************************************/
/* osl_removeSignalHandler */
/*****************************************************************************/
sal_Bool SAL_CALL osl_removeSignalHandler(oslSignalHandler Handler)
{
    oslSignalHandlerImpl *pHandler, *pPrevious = NULL;

    OSL_ASSERT(Handler != NULL);

    if (! bInitSignal)
        bInitSignal = InitSignal();

    osl_acquireMutex(SignalListMutex);

    pHandler = SignalList;

    while (pHandler != NULL)
    {
        if (pHandler == Handler)
        {
            if (pPrevious)
                pPrevious->pNext = pHandler->pNext;
            else
                SignalList = pHandler->pNext;

            osl_releaseMutex(SignalListMutex);

            if (SignalList == NULL)
                bInitSignal = DeInitSignal();

            free(pHandler);

            return (sal_True);
        }

        pPrevious = pHandler;
        pHandler  = pHandler->pNext;
    }

    osl_releaseMutex(SignalListMutex);

    return (sal_False);
}

/*****************************************************************************/
/* osl_raiseSignal */
/*****************************************************************************/
oslSignalAction SAL_CALL osl_raiseSignal(sal_Int32 UserSignal, void* UserData)
{
    oslSignalInfo   Info;
    oslSignalAction Action;

    if (! bInitSignal)
        bInitSignal = InitSignal();

    osl_acquireMutex(SignalListMutex);

    Info.Signal     = osl_Signal_User;
    Info.UserSignal = UserSignal;
    Info.UserData   = UserData;

    Action = CallSignalHandler(&Info);

    osl_releaseMutex(SignalListMutex);

    return (Action);
}

/*****************************************************************************/
/* osl_setErrorReporting */
/*****************************************************************************/

void win_seh_translator( unsigned nSEHCode, _EXCEPTION_POINTERS* /* pExcPtrs */)
{
    const char* pSEHName = NULL;
    switch( nSEHCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:         pSEHName = "SEH Exception: ACCESS VIOLATION"; break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:    pSEHName = "SEH Exception: DATATYPE MISALIGNMENT"; break;
        case EXCEPTION_BREAKPOINT:               /*pSEHName = "SEH Exception: BREAKPOINT";*/ break;
        case EXCEPTION_SINGLE_STEP:              /*pSEHName = "SEH Exception: SINGLE STEP";*/ break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    pSEHName = "SEH Exception: ARRAY BOUNDS EXCEEDED"; break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:     pSEHName = "SEH Exception: DENORMAL FLOAT OPERAND"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       pSEHName = "SEH Exception: FLOAT DIVIDE_BY_ZERO"; break;
        case EXCEPTION_FLT_INEXACT_RESULT:       pSEHName = "SEH Exception: FLOAT INEXACT RESULT"; break;
        case EXCEPTION_FLT_INVALID_OPERATION:    pSEHName = "SEH Exception: INVALID FLOAT OPERATION"; break;
        case EXCEPTION_FLT_OVERFLOW:             pSEHName = "SEH Exception: FLOAT OVERFLOW"; break;
        case EXCEPTION_FLT_STACK_CHECK:          pSEHName = "SEH Exception: FLOAT STACK_CHECK"; break;
        case EXCEPTION_FLT_UNDERFLOW:            pSEHName = "SEH Exception: FLOAT UNDERFLOW"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       pSEHName = "SEH Exception: INTEGER DIVIDE_BY_ZERO"; break;
        case EXCEPTION_INT_OVERFLOW:             pSEHName = "SEH Exception: INTEGER OVERFLOW"; break;
        case EXCEPTION_PRIV_INSTRUCTION:         pSEHName = "SEH Exception: PRIVILEDGED INSTRUCTION"; break;
        case EXCEPTION_IN_PAGE_ERROR:            pSEHName = "SEH Exception: IN_PAGE_ERROR"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:      pSEHName = "SEH Exception: ILLEGAL INSTRUCTION"; break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: pSEHName = "SEH Exception: NONCONTINUABLE EXCEPTION"; break;
        case EXCEPTION_STACK_OVERFLOW:           pSEHName = "SEH Exception: STACK OVERFLOW"; break;
        case EXCEPTION_INVALID_DISPOSITION:      pSEHName = "SEH Exception: INVALID DISPOSITION"; break;
        case EXCEPTION_GUARD_PAGE:               pSEHName = "SEH Exception: GUARD PAGE"; break;
        case EXCEPTION_INVALID_HANDLE:           pSEHName = "SEH Exception: INVALID HANDLE"; break;
//      case EXCEPTION_POSSIBLE_DEADLOCK:        pSEHName = "SEH Exception: POSSIBLE DEADLOCK"; break;
        default:                                 pSEHName = "Unknown SEH Exception"; break;
    }

    if( pSEHName)
        throw std::runtime_error( pSEHName);
}

sal_Bool SAL_CALL osl_setErrorReporting( sal_Bool bEnable )
{
    sal_Bool bOld = bErrorReportingEnabled;
    bErrorReportingEnabled = bEnable;

    if( !bEnable) // if the crash reporter is disabled
    {
        // fall back to handle Window's SEH events as C++ exceptions
        _set_se_translator( win_seh_translator);
    }

    return bOld;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
