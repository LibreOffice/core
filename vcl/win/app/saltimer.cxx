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
    assert( !pSalData->mpFirstInstance || pSalData->mnAppThreadId == GetCurrentThreadId() );

    HANDLE hTimer = pSalData->mnTimerId;
    if (hTimer)
    {
        pSalData->mnTimerId = nullptr;
        DeleteTimerQueueTimer(nullptr, hTimer, INVALID_HANDLE_VALUE);
    }

    // remove all pending SAL_MSG_TIMER_CALLBACK messages
    // we always have to do this, since ImplSalStartTimer with 0ms just queues
    // a new SAL_MSG_TIMER_CALLBACK message
    MSG aMsg;
    int nMsgCount = 0;
    while ( PeekMessageW(&aMsg, nullptr, SAL_MSG_TIMER_CALLBACK,
                         SAL_MSG_TIMER_CALLBACK, PM_REMOVE) )
        nMsgCount++;
    assert( nMsgCount <= 1 );
}

void ImplSalStartTimer( sal_uLong nMS )
{
    SalData* pSalData = GetSalData();
    assert( !pSalData->mpFirstInstance || pSalData->mnAppThreadId == GetCurrentThreadId() );

    // DueTime parameter is a DWORD, which is always an unsigned 32bit
    if (nMS > SAL_MAX_UINT32)
        nMS = SAL_MAX_UINT32;

    // cannot change a one-shot timer, so delete it and create a new one
    ImplSalStopTimer();

    // directly post a timer callback message for instant timers / idles
    if ( 0 == nMS )
    {
        BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd,
                                      SAL_MSG_TIMER_CALLBACK, 0, 0);
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
    else
    {
        // probably WT_EXECUTEONLYONCE is not needed, but it enforces Period
        // to be 0 and should not hurt; also see
        // https://www.microsoft.com/msj/0499/pooling/pooling.aspx
        CreateTimerQueueTimer(&pSalData->mnTimerId, nullptr, SalTimerProc, nullptr,
                              nMS, 0, WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);
    }
}

WinSalTimer::~WinSalTimer()
{
    Stop();
}

void WinSalTimer::Start( sal_uLong nMS )
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance && pSalData->mnAppThreadId != GetCurrentThreadId() )
    {
        BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd,
            SAL_MSG_STARTTIMER, 0, (LPARAM)GetTickCount() + nMS);
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
    else
        ImplSalStartTimer( nMS );
}

void WinSalTimer::Stop()
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance && pSalData->mnAppThreadId != GetCurrentThreadId() )
    {
        BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd,
            SAL_MSG_STOPTIMER, 0, 0);
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
    else
        ImplSalStopTimer();
}

/** This gets invoked from a Timer Queue thread.

Don't acquire the SolarMutex to avoid deadlocks, just wake up the main thread
at better resolution than 10ms.
*/
void CALLBACK SalTimerProc(PVOID, BOOLEAN)
{
    __try
    {
        SalData* pSalData = GetSalData();

        // always post message when the timer fires, we will remove the ones
        // that happened during execution of the callback later directly from
        // the message queue
        BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd,
                                      SAL_MSG_TIMER_CALLBACK, 0, 0);
#if OSL_DEBUG_LEVEL > 0
        if (0 == ret) // SEH prevents using SAL_WARN here?
            fputs("ERROR: PostMessage() failed!", stderr);
#endif
    }
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
}

/** Called in the main thread.

We assured that by posting the message from the SalTimeProc only, the real
call then happens when the main thread gets SAL_MSG_TIMER_CALLBACK.
*/
void EmitTimerCallback()
{
    // Test for MouseLeave
    SalTestMouseLeave();

    ImplSVData *pSVData = ImplGetSVData();
    if ( ! pSVData->maSchedCtx.mpSalTimer )
        return;

    // Try to acquire the mutex. If we don't get the mutex then we
    // try this a short time later again.
    if (ImplSalYieldMutexTryToAcquire())
    {
        pSVData->maSchedCtx.mpSalTimer->CallCallback();

        ImplSalYieldMutexRelease();
    }
    else
        ImplSalStartTimer( 10 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
