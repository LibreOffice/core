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

// maximum period
#define MAX_SYSPERIOD     65533

void CALLBACK SalTimerProc(PVOID pParameter, BOOLEAN bTimerOrWaitFired);

// See http://msdn.microsoft.com/en-us/library/windows/desktop/ms687003%28v=vs.85%29.aspx
// (and related pages) for details about the Timer Queues.

void ImplSalStopTimer(SalData* pSalData)
{
    HANDLE hTimer = pSalData->mnTimerId;
    pSalData->mnTimerId = 0;
    DeleteTimerQueueTimer(NULL, hTimer, 0);
}

void ImplSalStartTimer( sal_uLong nMS, bool bMutex )
{
    SalData* pSalData = GetSalData();

    // Remember the time of the timer
    pSalData->mnTimerMS = nMS;
    if (!bMutex)
        pSalData->mnTimerOrgMS = nMS;

    // duration has to fit into Window's sal_uInt16
    if (nMS > MAX_SYSPERIOD)
        nMS = MAX_SYSPERIOD;

    // change if it exists, create if not
    if (pSalData->mnTimerId)
        ChangeTimerQueueTimer(NULL, pSalData->mnTimerId, nMS, nMS);
    else
        CreateTimerQueueTimer(&pSalData->mnTimerId, NULL, SalTimerProc, NULL, nMS, nMS, WT_EXECUTEDEFAULT);

    pSalData->mnNextTimerTime = pSalData->mnLastEventTime + nMS;
}

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
        ImplSalStopTimer(pSalData);
        pSalData->mnNextTimerTime = 0;
    }
}

/** This gets invoked from a Timer Queue thread.

Don't acquire the SolarMutex to avoid deadlocks, just wake up the main thread
at better resolution than 10ms.
*/
void CALLBACK SalTimerProc(PVOID, BOOLEAN)
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

        // don't allow recursive calls (mbInTimerProc is set when the callback
        // is being processed)
        if (pSVData->mpSalTimer && !pSalData->mbInTimerProc)
        {
            ImplPostMessage(pSalData->mpFirstInstance->mhComWnd, SAL_MSG_TIMER_CALLBACK, 0, 0);
        }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    }
    han.Reset();
#else
    }
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif
}

/** Called in the main thread.

We assured that by posting the message from the SalTimeProc only, the real
call then happens when the main thread gets SAL_MSG_TIMER_CALLBACK.

@param bAllowRecursive allows to skip the check that assures that two timeouts
do not overlap.
*/
void EmitTimerCallback(bool bAllowRecursive)
{
    SalData* pSalData = GetSalData();
    ImplSVData* pSVData = ImplGetSVData();

    // Test for MouseLeave
    SalTestMouseLeave();

    // Try to acquire the mutex. If we don't get the mutex then we
    // try this a short time later again.
    if (ImplSalYieldMutexTryToAcquire())
    {
        if (pSVData->mpSalTimer && (!pSalData->mbInTimerProc || bAllowRecursive))
        {
            pSalData->mbInTimerProc = true;
            pSVData->mpSalTimer->CallCallback();
            pSalData->mbInTimerProc = false;
            ImplSalYieldMutexRelease();

            // Run the timer in the correct time, if we start this
            // with a small timeout, because we don't get the mutex
            if (pSalData->mnTimerId && (pSalData->mnTimerMS != pSalData->mnTimerOrgMS))
                ImplSalStartTimer(pSalData->mnTimerOrgMS, false);
        }
    }
    else
        ImplSalStartTimer(10, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
