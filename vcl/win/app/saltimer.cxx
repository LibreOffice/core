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

// in order to prevent concurrent execution of ImplSalStartTimer and double
// deletion of timer (which is extremely likely, given that
// INVALID_HANDLE_VALUE waits for the callback to run on the main thread),
// this must run on the main thread too
void ImplSalStopTimer()
{
    SalData *const pSalData = GetSalData();
    HANDLE hTimer = pSalData->mnTimerId;
    if (hTimer)
    {
        pSalData->mnTimerId = 0; // reset so it doesn't restart
        DeleteTimerQueueTimer(NULL, hTimer, INVALID_HANDLE_VALUE);
        pSalData->mnNextTimerTime = 0;
    }
    MSG aMsg;
    // this needs to run on the main thread
    while (PeekMessageW(&aMsg, 0, SAL_MSG_TIMER_CALLBACK, SAL_MSG_TIMER_CALLBACK, PM_REMOVE))
    {
        // just remove all the SAL_MSG_TIMER_CALLBACKs
        // when the application end, this SAL_MSG_TIMER_CALLBACK start the timer again
        // and then crashed in "SalTimerProc" when the object "SalData" was deleted
    }
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

    // cannot change a one-shot timer, so delete it and create new one
    if (pSalData->mnTimerId)
    {
        DeleteTimerQueueTimer(NULL, pSalData->mnTimerId, INVALID_HANDLE_VALUE);
        pSalData->mnTimerId = 0;
    }
    CreateTimerQueueTimer(&pSalData->mnTimerId, NULL, SalTimerProc, NULL, nMS, 0, WT_EXECUTEINTIMERTHREAD);

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
        {
            BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS);
            SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
        }
        else
            SendMessageW( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE );
}

void WinSalTimer::Stop()
{
    SalData* pSalData = GetSalData();

    assert(pSalData->mpFirstInstance);
    SendMessageW(pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STOPTIMER, 0, 0);
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

        // always post message when the timer fires, we will remove the ones
        // that happened during execution of the callback later directly from
        // the message queue
        BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd, SAL_MSG_TIMER_CALLBACK, 0, 0);
#if OSL_DEBUG_LEVEL > 0
        if (0 == ret) // SEH prevents using SAL_WARN here?
            fputs("ERROR: PostMessage() failed!", stderr);
#endif

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
*/
void EmitTimerCallback()
{
    SalData* pSalData = GetSalData();
    ImplSVData* pSVData = ImplGetSVData();

    // Test for MouseLeave
    SalTestMouseLeave();

    // Try to acquire the mutex. If we don't get the mutex then we
    // try this a short time later again.
    if (pSVData->mpSalTimer && ImplSalYieldMutexTryToAcquire())
    {
        pSVData->mpSalTimer->CallCallback();
        ImplSalYieldMutexRelease();

        // Run the timer again if it was started before, and also
        // Run the timer in the correct time, if we started this
        // with a small timeout, because we didn't get the mutex
        // - but not if mnTimerId is 0, which is set by ImplSalStopTimer()
        if (pSalData->mnTimerId)
            ImplSalStartTimer(pSalData->mnTimerOrgMS, false);
    }
    else
    {
        ImplSalStartTimer(10, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
