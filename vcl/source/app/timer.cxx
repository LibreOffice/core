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

#include <tools/time.hxx>
#include <vcl/timer.hxx>
#include <saltimer.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

#define MAX_TIMER_PERIOD   SAL_MAX_UINT64

void Timer::ImplStartTimer( ImplSVData* pSVData, sal_uInt64 nMS )
{
    InitSystemTimer();

    if ( !nMS )
        nMS = 1;

    // Assume underlying timers are recurring timers, if same period - just wait.
    if ( nMS != pSVData->mnTimerPeriod )
    {
        pSVData->mnTimerPeriod = nMS;
        pSVData->mpSalTimer->Start( nMS );
    }
}

void Timer::SetDeletionFlags()
{
        // if no AutoTimer than stop
        if ( !mbAuto )
        {
            mpSchedulerData->mbDelete = true;
            mbActive = false;
        }
}

bool Timer::ReadyForSchedule( bool bTimer )
{
    (void)bTimer;
    return (mpSchedulerData->mnUpdateTime + mnTimeout) <= tools::Time::GetSystemTicks();
}

sal_uInt64 Timer::UpdateMinPeriod( sal_uInt64 nMinPeriod, sal_uInt64 nTime )
{
    sal_uInt64 nNewTime = tools::Time::GetSystemTicks();
    sal_uInt64 nDeltaTime;
    //determine smallest time slot
    if( mpSchedulerData->mnUpdateTime == nTime )
    {
       nDeltaTime = mnTimeout;
       if( nDeltaTime < nMinPeriod )
           nMinPeriod = nDeltaTime;
    }
    else
    {
        nDeltaTime = mpSchedulerData->mnUpdateTime + mnTimeout;
        if( nDeltaTime < nNewTime )
            nMinPeriod = 1;
        else
        {
            nDeltaTime -= nNewTime;
            if( nDeltaTime < nMinPeriod )
                nMinPeriod = nDeltaTime;
        }
    }

    return nMinPeriod;
}

/**
 * Initialize the platform specific timer on which all the
 * platform independent timers are built
 */
void Timer::InitSystemTimer()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSalTimer )
    {
        pSVData->mnTimerPeriod = MAX_TIMER_PERIOD;
        pSVData->mpSalTimer = pSVData->mpDefInst->CreateSalTimer();
        pSVData->mpSalTimer->SetCallback( CallbackTaskScheduling );
    }
}

Timer::Timer(const sal_Char *pDebugName) : Scheduler(pDebugName)
{
    mnTimeout = 1;
    mbAuto = false;
    mePriority = SchedulerPriority::HIGHEST;
}

Timer::Timer( const Timer& rTimer ) : Scheduler(rTimer)
{
    mnTimeout = rTimer.mnTimeout;
    mbAuto = rTimer.mbAuto;
    maTimeoutHdl = rTimer.maTimeoutHdl;
}

void Timer::Invoke()
{
    maTimeoutHdl.Call( this );
}

void Timer::Start()
{
    Scheduler::Start();

    ImplSVData* pSVData = ImplGetSVData();
    if ( mnTimeout < pSVData->mnTimerPeriod )
        Timer::ImplStartTimer( pSVData, mnTimeout );
}

void Timer::SetTimeout( sal_uInt64 nNewTimeout )
{
    mnTimeout = nNewTimeout;
    // if timer is active then renew clock
    if ( mbActive )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( !pSVData->mnUpdateStack && (mnTimeout < pSVData->mnTimerPeriod) )
            Timer::ImplStartTimer( pSVData, mnTimeout );
    }
}

Timer& Timer::operator=( const Timer& rTimer )
{
    Scheduler::operator=(rTimer);
    maTimeoutHdl = rTimer.maTimeoutHdl;
    mnTimeout = rTimer.mnTimeout;
    mbAuto = rTimer.mbAuto;
    return *this;
}

AutoTimer::AutoTimer()
{
    mbAuto = true;
}

AutoTimer::AutoTimer( const AutoTimer& rTimer ) : Timer( rTimer )
{
    mbAuto = true;
}

AutoTimer& AutoTimer::operator=( const AutoTimer& rTimer )
{
    Timer::operator=( rTimer );
    return *this;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
