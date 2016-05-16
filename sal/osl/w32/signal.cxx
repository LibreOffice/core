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

#include <sal/config.h>
#include <config_features.h>

#include <signalshared.hxx>

/* system headers */
#include "system.h"
#include <tchar.h>

#include "file_url.h"
#include "path_helper.hxx"

#include <osl/diagnose.h>
#include <osl/signal.h>
#ifndef __MINGW32__
#include <DbgHelp.h>
#endif
#include <errorrep.h>
#include <systools/win32/uwinapi.h>
#include <sal/macros.h>
#include <eh.h>
#include <stdexcept>

namespace
{
long WINAPI signalHandlerFunction(LPEXCEPTION_POINTERS lpEP);

LPTOP_LEVEL_EXCEPTION_FILTER pPreviousHandler = nullptr;
}

bool onInitSignal()
{
    pPreviousHandler = SetUnhandledExceptionFilter(signalHandlerFunction);

    HMODULE hFaultRep = LoadLibrary( "faultrep.dll" );
    if ( hFaultRep )
    {
        pfn_ADDEREXCLUDEDAPPLICATIONW pfn = (pfn_ADDEREXCLUDEDAPPLICATIONW)GetProcAddress( hFaultRep, "AddERExcludedApplicationW" );
        if ( pfn )
            pfn( L"SOFFICE.EXE" );
        FreeLibrary( hFaultRep );
    }

    return true;
}

bool onDeInitSignal()
{
    SetUnhandledExceptionFilter(pPreviousHandler);

    return false;
}

namespace
{
/* magic Microsoft C++ compiler exception constant */
#define EXCEPTION_MSC_CPP_EXCEPTION 0xe06d7363

long WINAPI signalHandlerFunction(LPEXCEPTION_POINTERS lpEP)
{
#if HAVE_FEATURE_BREAKPAD
    // we should make sure to call the breakpad handler as
    // first step when we hit a problem
    if (pPreviousHandler)
        pPreviousHandler(lpEP);
#endif

    static bool bNested = false;

    oslSignalInfo info;

    info.UserSignal = lpEP->ExceptionRecord->ExceptionCode;
    info.UserData   = nullptr;

    switch (lpEP->ExceptionRecord->ExceptionCode)
    {
        /* Transform unhandled exceptions into access violations.
           Microsoft C++ compiler (add more for other compilers if necessary).
         */
        case EXCEPTION_MSC_CPP_EXCEPTION:
        case EXCEPTION_ACCESS_VIOLATION:
            info.Signal = osl_Signal_AccessViolation;
            break;

        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            info.Signal = osl_Signal_IntegerDivideByZero;
            break;

        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            info.Signal = osl_Signal_FloatDivideByZero;
            break;

        case EXCEPTION_BREAKPOINT:
            info.Signal = osl_Signal_DebugBreak;
            break;

        default:
            info.Signal = osl_Signal_System;
            break;
    }

    oslSignalAction action;

    if ( !bNested )
    {
        bNested = true;
        action = callSignalHandler(&info);
    }
    else
        action = osl_Signal_ActKillApp;

    switch ( action )
    {
        case osl_Signal_ActCallNextHdl:
            return EXCEPTION_CONTINUE_SEARCH;

        case osl_Signal_ActAbortApp:
            return EXCEPTION_EXECUTE_HANDLER;

        case osl_Signal_ActKillApp:
            SetErrorMode(SEM_NOGPFAULTERRORBOX);
            exit(255);
            break;
        default:
            break;
    }

    return EXCEPTION_CONTINUE_EXECUTION;
}

void win_seh_translator( unsigned nSEHCode, _EXCEPTION_POINTERS* /* pExcPtrs */)
{
    const char* pSEHName = nullptr;
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
        case EXCEPTION_PRIV_INSTRUCTION:         pSEHName = "SEH Exception: PRIVILEGED INSTRUCTION"; break;
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

}

void onErrorReportingChanged(bool bEnable)
{
#if defined _MSC_VER
    if( !bEnable) // if the crash reporter is disabled
    {
        // fall back to handle Window's SEH events as C++ exceptions
        _set_se_translator( win_seh_translator);
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
