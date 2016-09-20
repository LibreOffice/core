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

#include <svdata.hxx>
#include <tools/time.hxx>
#include <vcl/scheduler.hxx>
#include <saltimer.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

void ImplSchedulerData::Invoke()
{
    assert( mpScheduler && !mbInScheduler );
    if ( !mpScheduler || mbInScheduler )
        return;

    // tdf#92036 Reset the period to avoid re-firing immediately.
    mpScheduler->mpSchedulerData->mnUpdateTime = tools::Time::GetSystemTicks();

    Scheduler *sched = mpScheduler;

    // prepare Scheduler Object for deletion after handling
    mpScheduler->SetDeletionFlags();

    // invoke it
    mbInScheduler = true;
    sched->Invoke();
    mbInScheduler = false;
}

void Scheduler::SetDeletionFlags()
{
    assert( mpSchedulerData );
    mpSchedulerData->mpScheduler = nullptr;
    mpSchedulerData = nullptr;
}

void Scheduler::ImplDeInitScheduler()
{
    ImplSVData*     pSVData = ImplGetSVData();
    if (pSVData->mpSalTimer)
    {
        pSVData->mpSalTimer->Stop();
    }

    ImplSchedulerData *pSchedulerData = pSVData->mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        if ( pSchedulerData->mpScheduler )
            pSchedulerData->mpScheduler->mpSchedulerData = nullptr;
        ImplSchedulerData* pNextSchedulerData = pSchedulerData->mpNext;
        delete pSchedulerData;
        pSchedulerData = pNextSchedulerData;
    }

    pSVData->mpFirstSchedulerData = nullptr;
    pSVData->mnTimerPeriod        = 0;

    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer = nullptr;
}

/**
 * Start a new timer if we need to for nMS duration.
 *
 * if this is longer than the existing duration we're
 * waiting for, do nothing.
 */
void Scheduler::ImplStartTimer( sal_uInt64 nMS, bool bForce )
{
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mbDeInit)
    {
        // do not start new timers during shutdown - if that happens after
        // ImplSalStopTimer() on WNT the timer queue is restarted and never ends
        return;
    }

    /**
    * Initialize the platform specific timer on which all the
    * platform independent timers are built
    */
    if (!pSVData->mpSalTimer)
    {
        pSVData->mnTimerPeriod = InfiniteTimeoutMs;
        pSVData->mpSalTimer = pSVData->mpDefInst->CreateSalTimer();
        pSVData->mpSalTimer->SetCallback(Scheduler::CallbackTaskScheduling);
    }

    if ( !nMS )
        nMS = 1;

    // Only if smaller timeout, to avoid skipping.
    if (nMS < pSVData->mnTimerPeriod || (bForce && nMS != pSVData->mnTimerPeriod) )
    {
        pSVData->mnTimerPeriod = nMS;
        pSVData->mpSalTimer->Start(nMS);
    }
}

void Scheduler::CallbackTaskScheduling( bool bIdle )
{
    // this function is for the saltimer callback
    Scheduler::ProcessTaskScheduling( bIdle );
}

static bool g_bDeterministicMode = false;

void Scheduler::SetDeterministicMode(bool bDeterministic)
{
    g_bDeterministicMode = bDeterministic;
}

bool Scheduler::GetDeterministicMode()
{
    return g_bDeterministicMode;
}

inline void Scheduler::UpdateMinPeriod( ImplSchedulerData *pSchedulerData,
                                        const sal_uInt64 nTime, sal_uInt64 &nMinPeriod )
{
    if ( nMinPeriod > ImmediateTimeoutMs )
        pSchedulerData->mpScheduler->UpdateMinPeriod( nTime, nMinPeriod );
    assert( nMinPeriod >= ImmediateTimeoutMs );
}

bool Scheduler::ProcessTaskScheduling( bool bIdle )
{
    // if bIdle is false, only handle timer
    ImplSVData*        pSVData = ImplGetSVData();
    ImplSchedulerData* pSchedulerData = pSVData->mpFirstSchedulerData;
    ImplSchedulerData* pPrevSchedulerData = nullptr;
    ImplSchedulerData *pMostUrgent = nullptr;

    sal_uInt64         nTime = tools::Time::GetSystemTicks();
    sal_uInt64         nMinPeriod = InfiniteTimeoutMs;

    while ( pSchedulerData )
    {
        // Skip invoked task
        if ( pSchedulerData->mbInScheduler )
            goto next_entry;

        // Should Task be released from scheduling?
        if ( !pSchedulerData->mpScheduler )
        {
            ImplSchedulerData* pNextSchedulerData = pSchedulerData->mpNext;
            if ( pPrevSchedulerData )
                pPrevSchedulerData->mpNext = pNextSchedulerData;
            else
                pSVData->mpFirstSchedulerData = pNextSchedulerData;
            ImplSchedulerData* pTempSchedulerData = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
            delete pTempSchedulerData;
            continue;
        }

        assert( pSchedulerData->mpScheduler );
        if ( !pSchedulerData->mpScheduler->ReadyForSchedule( nTime, bIdle ) )
            goto evaluate_entry;

        // if the priority of the current task is higher (numerical value is lower) than
        // the priority of the most urgent, the current task becomes the new most urgent
        if ( !pMostUrgent )
            pMostUrgent = pSchedulerData;
            goto next_entry;
        }
        else if ( pSchedulerData->mpScheduler->GetPriority() < pMostUrgent->mpScheduler->GetPriority() )
        {
            UpdateMinPeriod( pMostUrgent, nTime, nMinPeriod );
            pMostUrgent = pSchedulerData;
            goto next_entry;
        }

evaluate_entry:
        UpdateMinPeriod( pSchedulerData, nTime, nMinPeriod );

next_entry:
        pPrevSchedulerData = pSchedulerData;
        pSchedulerData = pSchedulerData->mpNext;
    }

    assert( !pSchedulerData );

    if ( pMostUrgent )
    {
        pMostUrgent->mnUpdateTime = nTime;
        UpdateMinPeriod( pMostUrgent, nTime, nMinPeriod );

        pMostUrgent->Invoke();
    }

    if ( nMinPeriod != InfiniteTimeoutMs )
        ImplStartTimer( nMinPeriod, true );
    else if ( pSVData->mpSalTimer )
        pSVData->mpSalTimer->Stop();

    pSVData->mnTimerPeriod = nMinPeriod;

    return pMostUrgent != nullptr;
}

void Scheduler::Start()
{
    ImplSVData *const pSVData = ImplGetSVData();
    if (pSVData->mbDeInit)
    {
        return;
    }

    if ( !mpSchedulerData )
    {
        // insert Scheduler
        mpSchedulerData                = new ImplSchedulerData;
        mpSchedulerData->mpScheduler   = this;
        mpSchedulerData->mbInScheduler = false;

        // insert last due to SFX!
        ImplSchedulerData* pPrev = nullptr;
        ImplSchedulerData* pData = pSVData->mpFirstSchedulerData;
        while ( pData )
        {
            pPrev = pData;
            pData = pData->mpNext;
        }
        mpSchedulerData->mpNext = nullptr;
        if ( pPrev )
            pPrev->mpNext = mpSchedulerData;
        else
            pSVData->mpFirstSchedulerData = mpSchedulerData;
    }
    mpSchedulerData->mnUpdateTime  = tools::Time::GetSystemTicks();
}

void Scheduler::Stop()
{
    if ( !mpSchedulerData )
        return;
    Scheduler::SetDeletionFlags();
    assert( !mpSchedulerData );
}

Scheduler& Scheduler::operator=( const Scheduler& rScheduler )
{
    if ( IsActive() )
        Stop();

    mePriority = rScheduler.mePriority;

    if ( rScheduler.IsActive() )
        Start();

    return *this;
}

Scheduler::Scheduler(const sal_Char *pDebugName):
    mpSchedulerData(nullptr),
    mpDebugName(pDebugName),
    mePriority(SchedulerPriority::HIGH)
{
}

Scheduler::Scheduler( const Scheduler& rScheduler ):
    mpSchedulerData(nullptr),
    mpDebugName(rScheduler.mpDebugName),
    mePriority(rScheduler.mePriority)
{
    if ( rScheduler.IsActive() )
        Start();
}

Scheduler::~Scheduler()
{
    if ( mpSchedulerData )
        mpSchedulerData->mpScheduler = nullptr;
}

const char *ImplSchedulerData::GetDebugName() const
{
    return mpScheduler && mpScheduler->GetDebugName() ?
        mpScheduler->GetDebugName() : "unknown";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
