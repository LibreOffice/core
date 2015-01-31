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

#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>

#include <saltimer.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

#define MAX_TIMER_PERIOD    ((sal_uLong)0xFFFFFFFF)

struct ImplTimerData
{
    ImplTimerData*  mpNext;         // Pointer to the next Instance
    Timer*          mpTimer;        // Pointer to VCL Timer instance
    sal_uLong       mnUpdateTime;   // Last Update Time
    sal_uLong       mnTimerUpdate;  // TimerCallbackProcs on stack
    bool            mbDelete;       // Was timer deleted during Update()?
    bool            mbInTimeout;    // Are we in a timeout handler?

    void Invoke()
    {
        if (mbDelete || mbInTimeout )
            return;

        // if no AutoTimer than stop
        if ( !mpTimer->mbAuto )
        {
            mbDelete = true;
            mpTimer->mbActive = false;
        }

        // invoke it
        mbInTimeout = true;
        mpTimer->Timeout();
        mbInTimeout = false;
    }

    sal_uLong GetDeadline()
    {
        return mnUpdateTime + mpTimer->mnTimeout;
    }
    static ImplTimerData *GetFirstIdle()
    {
        ImplSVData*     pSVData = ImplGetSVData();
        ImplTimerData *pMostUrgent = NULL;

        for ( ImplTimerData *p = pSVData->mpFirstTimerData; p; p = p->mpNext )
        {
            if ( !p->mpTimer || p->mbDelete || !p->mpTimer->mbIdle )
                continue;
            if (!pMostUrgent)
                pMostUrgent = p;
            else
            {
                // Find the highest priority one somehow.
                if ( p->GetDeadline() < pMostUrgent->GetDeadline() )
                    pMostUrgent = p;
            }
        }

        return pMostUrgent;
    }
};

void Timer::ImplDeInitTimer()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplTimerData*  pTimerData = pSVData->mpFirstTimerData;

    // on WNT the timer queue thread needs killing
    if (pSVData->mpSalTimer)
    {
        pSVData->mpSalTimer->Stop();
    }

    if ( pTimerData )
    {
        do
        {
            ImplTimerData* pTempTimerData = pTimerData;
            if ( pTimerData->mpTimer )
            {
                pTimerData->mpTimer->mbActive = false;
                pTimerData->mpTimer->mpTimerData = NULL;
            }
            pTimerData = pTimerData->mpNext;
            delete pTempTimerData;
        }
        while ( pTimerData );

        pSVData->mpFirstTimerData   = NULL;
        pSVData->mnTimerPeriod      = 0;
    }

    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer = 0;
}

static void ImplStartTimer( ImplSVData* pSVData, sal_uLong nMS )
{
    if ( !nMS )
        nMS = 1;

    // Assume underlying timers are recurring timers, if same period - just wait.
    if ( nMS != pSVData->mnTimerPeriod )
    {
        pSVData->mnTimerPeriod = nMS;
        pSVData->mpSalTimer->Start( nMS );
    }
}

void Timer::ImplTimerCallbackProc( bool idle )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplTimerData*  pTimerData;
    ImplTimerData*  pPrevTimerData;
    sal_uLong       nMinPeriod = MAX_TIMER_PERIOD;
    sal_uLong       nDeltaTime;
    sal_uLong       nTime = tools::Time::GetSystemTicks();

    if ( pSVData->mbNoCallTimer )
        return;

    pSVData->mnTimerUpdate++;
    pSVData->mbNotAllTimerCalled = true;

    // find timer where the timer handler needs to be called
    pTimerData = pSVData->mpFirstTimerData;
    while ( pTimerData )
    {
        // If the timer is not new, was not deleted, and if it is not in the timeout handler, then
        // call the handler as soon as the time is up.
        if ( (pTimerData->mnTimerUpdate < pSVData->mnTimerUpdate) &&
             !pTimerData->mbDelete && !pTimerData->mbInTimeout && (!pTimerData->mpTimer->mbIdle || idle) )
        {
            // time has expired
            if ( pTimerData->GetDeadline() <= nTime )
            {
                // set new update time
                pTimerData->mnUpdateTime = nTime;

                pTimerData->Invoke();
            }
        }

        pTimerData = pTimerData->mpNext;
    }

    // determine new time
    sal_uLong nNewTime = tools::Time::GetSystemTicks();
    pPrevTimerData = NULL;
    pTimerData = pSVData->mpFirstTimerData;
    while ( pTimerData )
    {
        // ignore if timer is still in timeout handler
        if ( pTimerData->mbInTimeout )
        {
            pPrevTimerData = pTimerData;
            pTimerData = pTimerData->mpNext;
        }
        // Was timer destroyed in the meantime?
        else if ( pTimerData->mbDelete )
        {
            if ( pPrevTimerData )
                pPrevTimerData->mpNext = pTimerData->mpNext;
            else
                pSVData->mpFirstTimerData = pTimerData->mpNext;
            if ( pTimerData->mpTimer )
                pTimerData->mpTimer->mpTimerData = NULL;
            ImplTimerData* pTempTimerData = pTimerData;
            pTimerData = pTimerData->mpNext;
            delete pTempTimerData;
        }
        else
        {
            pTimerData->mnTimerUpdate = 0;
            // determine smallest time slot
            if ( pTimerData->mnUpdateTime == nTime )
            {
                nDeltaTime = pTimerData->mpTimer->mnTimeout;
                if ( nDeltaTime < nMinPeriod )
                    nMinPeriod = nDeltaTime;
            }
            else
            {
                nDeltaTime = pTimerData->mnUpdateTime + pTimerData->mpTimer->mnTimeout;
                if ( nDeltaTime < nNewTime )
                    nMinPeriod = 1;
                else
                {
                    nDeltaTime -= nNewTime;
                    if ( nDeltaTime < nMinPeriod )
                        nMinPeriod = nDeltaTime;
                }
            }
            pPrevTimerData = pTimerData;
            pTimerData = pTimerData->mpNext;
        }
    }

    // delete clock if no more timers available
    if ( !pSVData->mpFirstTimerData )
    {
        pSVData->mpSalTimer->Stop();
        pSVData->mnTimerPeriod = MAX_TIMER_PERIOD;
    }
    else
        ImplStartTimer( pSVData, nMinPeriod );

    pSVData->mnTimerUpdate--;
    pSVData->mbNotAllTimerCalled = false;
}

void Timer::ProcessAllIdleHandlers()
{
    // process all pending Idle timers
    while (ImplTimerData* pTimerData =
                ImplTimerData::GetFirstIdle())
    {
        pTimerData->Invoke();
    }
}

Timer::Timer():
    mpTimerData(NULL),
    mnTimeout(1),
    mbActive(false),
    mbAuto(false),
    mbIdle(false)
{
}

Timer::Timer( const Timer& rTimer ):
    mpTimerData(NULL),
    mnTimeout(rTimer.mnTimeout),
    mbActive(false),
    mbAuto(false),
    mbIdle(false),
    maTimeoutHdl(rTimer.maTimeoutHdl)
{
    if ( rTimer.IsActive() )
        Start();
}

Timer::~Timer()
{
    if ( mpTimerData )
    {
        mpTimerData->mbDelete = true;
        mpTimerData->mpTimer = NULL;
     }
}

void Timer::Timeout()
{
    maTimeoutHdl.Call( this );
}

void Timer::SetTimeout( sal_uLong nNewTimeout )
{
    mnTimeout = nNewTimeout;

    // if timer is active then renew clock
    if ( mbActive )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( !pSVData->mnTimerUpdate && (mnTimeout < pSVData->mnTimerPeriod) )
            ImplStartTimer( pSVData, mnTimeout );
    }
}

void Timer::Start()
{
    mbActive = true;

    ImplSVData* pSVData = ImplGetSVData();
    if ( !mpTimerData )
    {
        if ( !pSVData->mpFirstTimerData )
        {
            pSVData->mnTimerPeriod = MAX_TIMER_PERIOD;
            if( ! pSVData->mpSalTimer )
            {
                pSVData->mpSalTimer = pSVData->mpDefInst->CreateSalTimer();
                pSVData->mpSalTimer->SetCallback( ImplTimerCallbackProc );
            }
        }

        // insert timer and start
        mpTimerData = new ImplTimerData;
        mpTimerData->mpTimer        = this;
        mpTimerData->mnUpdateTime   = tools::Time::GetSystemTicks();
        mpTimerData->mnTimerUpdate  = pSVData->mnTimerUpdate;
        mpTimerData->mbDelete       = false;
        mpTimerData->mbInTimeout    = false;

        // insert last due to SFX!
        ImplTimerData* pPrev = NULL;
        ImplTimerData* pData = pSVData->mpFirstTimerData;
        while ( pData )
        {
            pPrev = pData;
            pData = pData->mpNext;
        }
        mpTimerData->mpNext = NULL;
        if ( pPrev )
            pPrev->mpNext = mpTimerData;
        else
            pSVData->mpFirstTimerData = mpTimerData;

        if ( mnTimeout < pSVData->mnTimerPeriod )
            ImplStartTimer( pSVData, mnTimeout );
    }
    else if( !mpTimerData->mpTimer ) // TODO: remove when guilty found
    {
        OSL_FAIL( "Timer::Start() on a destroyed Timer!" );
    }
    else
    {
        mpTimerData->mnUpdateTime    = tools::Time::GetSystemTicks();
        mpTimerData->mnTimerUpdate   = pSVData->mnTimerUpdate;
        mpTimerData->mbDelete        = false;
    }
}

void Timer::Stop()
{
    mbActive = false;

    if ( mpTimerData )
        mpTimerData->mbDelete = true;
}

Timer& Timer::operator=( const Timer& rTimer )
{
    if ( IsActive() )
        Stop();

    mbActive        = false;
    mnTimeout       = rTimer.mnTimeout;
    maTimeoutHdl    = rTimer.maTimeoutHdl;

    if ( rTimer.IsActive() )
        Start();

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

Idle::Idle()
    : Timer()
{
    mbIdle = true;
    SetPriority(VCL_IDLE_PRIORITY_LOWEST);
}

Idle::Idle( IdlePriority ePriority )
    : Timer()
{
    mbIdle = true;
    SetPriority( ePriority );
}

void Idle::SetPriority( IdlePriority ePriority )
{
    sal_uLong nTimeoutMS = 0;

    // Ultimately this will just be a sort key in a work queue.
    switch (ePriority) {
    case VCL_IDLE_PRIORITY_HIGHEST:
        nTimeoutMS = 0;
        break;
    case VCL_IDLE_PRIORITY_HIGH:
        nTimeoutMS = 1;
        break;
    case VCL_IDLE_PRIORITY_REPAINT:
        nTimeoutMS = 30;
        break;
    case VCL_IDLE_PRIORITY_RESIZE:
        nTimeoutMS = 50;
        break;
    case VCL_IDLE_PRIORITY_MEDIUM:
        nTimeoutMS = 50;
        break;
    case VCL_IDLE_PRIORITY_LOW:
        nTimeoutMS = 100;
        break;
    case VCL_IDLE_PRIORITY_LOWER:
        nTimeoutMS = 200;
        break;
    case VCL_IDLE_PRIORITY_LOWEST:
    default:
        nTimeoutMS = 400;
        break;
    }
    SetTimeout( nTimeoutMS );
}

void Idle::DoIdle()
{
    maTimeoutHdl.Call( this );
}


Idle::~Idle()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
