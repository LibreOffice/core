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
#include <sal/log.hxx>

#include <tools/time.hxx>

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
void WinSalTimer::ImplStop()
{
    SalData *const pSalData = GetSalData();
    const WinSalInstance *pInst = pSalData->mpInstance;
    assert( !pInst || pSalData->mnAppThreadId == GetCurrentThreadId() );

    if ( m_bSetTimerRunning )
    {
        m_bSetTimerRunning = false;
        KillTimer( GetSalData()->mpInstance->mhComWnd, m_aWmTimerId );
    }
    m_bDirectTimeout = false;

    const HANDLE hTimer = m_nTimerId;
    if ( nullptr == hTimer )
        return;

    m_nTimerId = nullptr;
    DeleteTimerQueueTimer( nullptr, hTimer, INVALID_HANDLE_VALUE );
    // Keep InvalidateEvent after DeleteTimerQueueTimer, because the event id
    // is set in SalTimerProc, which DeleteTimerQueueTimer will finish or cancel.
    InvalidateEvent();
}

void WinSalTimer::ImplStart( sal_uInt64 nMS )
{
    SalData* pSalData = GetSalData();
    assert( !pSalData->mpInstance || pSalData->mnAppThreadId == GetCurrentThreadId() );

    // DueTime parameter is a DWORD, which is always an unsigned 32bit
    if (nMS > SAL_MAX_UINT32)
        nMS = SAL_MAX_UINT32;

    // cannot change a one-shot timer, so delete it and create a new one
    ImplStop();

    // directly indicate an elapsed timer
    m_bDirectTimeout = ( 0 == nMS );
    // probably WT_EXECUTEONLYONCE is not needed, but it enforces Period
    // to be 0 and should not hurt; also see
    // https://www.microsoft.com/msj/0499/pooling/pooling.aspx
    if ( !m_bDirectTimeout )
        CreateTimerQueueTimer(&m_nTimerId, nullptr, SalTimerProc, this,
                              nMS, 0, WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);
    else if ( m_bForceRealTimer )
    {
        // so we don't block the nested message queue in move and resize
        // with posted 0ms SAL_MSG_TIMER_CALLBACK messages
        SetTimer( GetSalData()->mpInstance->mhComWnd, m_aWmTimerId,
                  USER_TIMER_MINIMUM, nullptr );
        m_bSetTimerRunning = true;
    }
    // we don't need any wakeup message, as this code can just run in the
    // main thread!
}

WinSalTimer::WinSalTimer()
    : m_nTimerId( nullptr )
    , m_bDirectTimeout( false )
    , m_bForceRealTimer( false )
    , m_bSetTimerRunning( false )
{
}

WinSalTimer::~WinSalTimer()
{
    Stop();
}

void WinSalTimer::Start( sal_uInt64 nMS )
{
    WinSalInstance *pInst = GetSalData()->mpInstance;
    if ( pInst && !pInst->IsMainThread() )
    {
        BOOL const ret = PostMessageW(pInst->mhComWnd,
            SAL_MSG_STARTTIMER, 0, static_cast<LPARAM>(tools::Time::GetSystemTicks()) + nMS);
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
    else
        ImplStart( nMS );
}

void WinSalTimer::Stop()
{
    WinSalInstance *pInst = GetSalData()->mpInstance;
    if ( pInst && !pInst->IsMainThread() )
    {
        BOOL const ret = PostMessageW(pInst->mhComWnd,
            SAL_MSG_STOPTIMER, 0, 0);
        SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
    }
    else
        ImplStop();
}

/**
 * This gets invoked from a Timer Queue thread.
 * Don't acquire the SolarMutex to avoid deadlocks.
 */
void CALLBACK SalTimerProc(PVOID data, BOOLEAN)
{
    __try
    {
        WinSalTimer *pTimer = static_cast<WinSalTimer*>( data );
        BOOL const ret = PostMessageW(
            GetSalData()->mpInstance->mhComWnd, SAL_MSG_TIMER_CALLBACK,
            static_cast<WPARAM>(pTimer->GetNextEventVersion()), 0 );
#if OSL_DEBUG_LEVEL > 0
        if (0 == ret) // SEH prevents using SAL_WARN here?
            fputs("ERROR: PostMessage() failed!\n", stderr);
#endif
    }
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
}

void WinSalTimer::ImplHandleElapsedTimer()
{
    // Test for MouseLeave
    SalTestMouseLeave();

    m_bDirectTimeout = false;
    ImplSalYieldMutexAcquireWithWait();
    CallCallback();
    ImplSalYieldMutexRelease();
}

void WinSalTimer::ImplHandleTimerEvent( const WPARAM aWPARAM )
{
    assert( aWPARAM <= SAL_MAX_INT32 );
    if ( !IsValidEventVersion( static_cast<sal_Int32>( aWPARAM ) ) )
        return;

    ImplHandleElapsedTimer();
}

void WinSalTimer::SetForceRealTimer( const bool bVal )
{
    if ( m_bForceRealTimer == bVal )
        return;

    m_bForceRealTimer = bVal;

    // we need a real timer, as m_bDirectTimeout won't be processed
    if ( bVal && m_bDirectTimeout )
        Start( 0 );
}

void WinSalTimer::ImplHandle_WM_TIMER( const WPARAM aWPARAM )
{
    assert( m_aWmTimerId == aWPARAM );
    if ( !(m_aWmTimerId == aWPARAM && m_bSetTimerRunning) )
        return;

    m_bSetTimerRunning = false;
    KillTimer( GetSalData()->mpInstance->mhComWnd, m_aWmTimerId );
    ImplHandleElapsedTimer();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
