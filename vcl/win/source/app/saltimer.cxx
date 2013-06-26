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

#include <svsys.h>
#include <win/saldata.hxx>
#include <win/saltimer.h>
#include <win/salinst.h>

#if defined ( __MINGW32__ )
#include <sehandler.hxx>
#endif


// =======================================================================

// maximum period
#define MAX_SYSPERIOD     65533

// =======================================================================

void ImplSalStartTimer( sal_uLong nMS, sal_Bool bMutex )
{
    SalData* pSalData = GetSalData();

    // Remember the time of the timer
    pSalData->mnTimerMS = nMS;
    if ( !bMutex )
        pSalData->mnTimerOrgMS = nMS;

    // duration has to fit into Window's sal_uInt16
    if ( nMS > MAX_SYSPERIOD )
        nMS = MAX_SYSPERIOD;

    // kill timer if it exists
    if ( pSalData->mnTimerId )
        KillTimer( 0, pSalData->mnTimerId );

    // Make a new timer with new period
    pSalData->mnTimerId = SetTimer( 0, 0, (UINT)nMS, SalTimerProc );
    pSalData->mnNextTimerTime = pSalData->mnLastEventTime + nMS;
}

// -----------------------------------------------------------------------

WinSalTimer::~WinSalTimer()
{
}

void WinSalTimer::Start( sal_uLong nMS )
{
    // switch to main thread
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
            ImplPostMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
        else
            ImplSendMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE );
}

void WinSalTimer::Stop()
{
    SalData* pSalData = GetSalData();

    // If we have a timer, than
    if ( pSalData->mnTimerId )
    {
        KillTimer( 0, pSalData->mnTimerId );
        pSalData->mnTimerId = 0;
        pSalData->mnNextTimerTime = 0;
    }
}

// -----------------------------------------------------------------------

void CALLBACK SalTimerProc( HWND, UINT, UINT_PTR nId, DWORD )
{
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        SalData* pSalData = GetSalData();
        ImplSVData* pSVData = ImplGetSVData();

        // Test for MouseLeave
        SalTestMouseLeave();

        bool bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
        if ( pSVData->mpSalTimer && ! bRecursive )
        {
            // Try to aquire the mutex. If we don't get the mutex then we
            // try this a short time later again.
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
                if ( pSVData->mpSalTimer && ! bRecursive )
                {
                    pSalData->mbInTimerProc = TRUE;
                    pSVData->mpSalTimer->CallCallback();
                    pSalData->mbInTimerProc = FALSE;
                    ImplSalYieldMutexRelease();

                    // Run the timer in the correct time, if we start this
                    // with a small timeout, because we don't get the mutex
                    if ( pSalData->mnTimerId &&
                        (pSalData->mnTimerMS != pSalData->mnTimerOrgMS) )
                        ImplSalStartTimer( pSalData->mnTimerOrgMS, FALSE );
                }
            }
            else
                ImplSalStartTimer( 10, TRUE );
        }
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#else
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
