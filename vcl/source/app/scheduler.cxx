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

    // Free active tasks
    ImplSchedulerData *pSchedulerData = pSVData->mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        if ( pSchedulerData->mpScheduler )
            pSchedulerData->mpScheduler->mpSchedulerData = nullptr;
        ImplSchedulerData* pNextSchedulerData = pSchedulerData->mpNext;
        delete pSchedulerData;
        pSchedulerData = pNextSchedulerData;
    }

    // Free "deleted" tasks
    pSchedulerData = pSVData->mpFreeSchedulerData;
    while ( pSchedulerData )
    {
        assert( !pSchedulerData->mpScheduler );
        ImplSchedulerData* pNextSchedulerData = pSchedulerData->mpNext;
        delete pSchedulerData;
        pSchedulerData = pNextSchedulerData;
    }

    pSVData->mpFirstSchedulerData = nullptr;
    pSVData->mpFreeSchedulerData  = nullptr;
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

    // Only if smaller timeout, to avoid skipping.
    if (nMS < pSVData->mnTimerPeriod || (bForce && nMS != pSVData->mnTimerPeriod) )
    {
        pSVData->mnTimerPeriod = nMS;
        pSVData->mpSalTimer->Start(nMS);
    }
}

void Scheduler::CallbackTaskScheduling()
{
    // this function is for the saltimer callback
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mbNeedsReschedule = true;
    Scheduler::ProcessTaskScheduling();
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

inline bool Scheduler::HasPendingEvents( const ImplSVData* pSVData, const sal_uInt64 nTime )
{
   return ( pSVData->mbNeedsReschedule || ((pSVData->mnTimerPeriod != InfiniteTimeoutMs)
       && (nTime >= pSVData->mnLastUpdate + pSVData->mnTimerPeriod)) );
}

bool Scheduler::HasPendingEvents()
{
    ImplSVData*  pSVData = ImplGetSVData();
    sal_uInt64   nTime = tools::Time::GetSystemTicks();
    return HasPendingEvents( pSVData, nTime );
}

inline void Scheduler::UpdateMinPeriod( ImplSchedulerData *pSchedulerData,
                                        const sal_uInt64 nTime, sal_uInt64 &nMinPeriod )
{
    if ( nMinPeriod > ImmediateTimeoutMs )
        pSchedulerData->mpScheduler->UpdateMinPeriod( nTime, nMinPeriod );
}

bool Scheduler::ProcessTaskScheduling( IdleRunPolicy eIdleRunPolicy )
{
    ImplSVData*        pSVData = ImplGetSVData();
    sal_uInt64         nTime = tools::Time::GetSystemTicks();
    if ( !HasPendingEvents( pSVData, nTime ) )
        return false;
    pSVData->mbNeedsReschedule = false;

    ImplSchedulerData* pSchedulerData = pSVData->mpFirstSchedulerData;
    ImplSchedulerData* pPrevSchedulerData = nullptr;
    ImplSchedulerData *pPrevMostUrgent = nullptr;
    ImplSchedulerData *pMostUrgent = nullptr;
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
            pSchedulerData->mpNext = pSVData->mpFreeSchedulerData;
            pSVData->mpFreeSchedulerData = pSchedulerData;
            pSchedulerData = pNextSchedulerData;
            continue;
        }

        assert( pSchedulerData->mpScheduler );
        if ( !pSchedulerData->mpScheduler->ReadyForSchedule( nTime ) )
            goto evaluate_entry;

        // if the priority of the current task is higher (numerical value is lower) than
        // the priority of the most urgent, the current task becomes the new most urgent
        if ( !pMostUrgent )
        {
            pPrevMostUrgent = pPrevSchedulerData;
            pMostUrgent = pSchedulerData;
            goto next_entry;
        }
        else if ( pSchedulerData->mpScheduler->GetPriority() < pMostUrgent->mpScheduler->GetPriority() )
        {
            UpdateMinPeriod( pMostUrgent, nTime, nMinPeriod );
            pPrevMostUrgent = pPrevSchedulerData;
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
        assert( pPrevMostUrgent != pMostUrgent );
        assert( !pPrevMostUrgent || (pPrevMostUrgent->mpNext == pMostUrgent) );

        pMostUrgent->mnUpdateTime = nTime;
        UpdateMinPeriod( pMostUrgent, nTime, nMinPeriod );

        Scheduler *pTempScheduler = pMostUrgent->mpScheduler;
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " " << pMostUrgent << "  invoke     "
                   << (int) pTempScheduler->mePriority << " " << pTempScheduler->mpDebugName );
        pMostUrgent->Invoke();
        SAL_INFO_IF( !pMostUrgent->mpScheduler, "vcl.schedule", tools::Time::GetSystemTicks()
                     << " " << pMostUrgent <<  "  tag-rm     " );

        // do some simple round-robin scheduling
        // nothing to do, if we're already the last element
        if ( pMostUrgent->mpScheduler && pMostUrgent->mpNext )
        {
            if ( pPrevMostUrgent )
                pPrevMostUrgent->mpNext = pMostUrgent->mpNext;
            else
                pSVData->mpFirstSchedulerData = pMostUrgent->mpNext;
            // Invoke might have added tasks to the end of the list
            while ( pPrevSchedulerData->mpNext )
                pPrevSchedulerData = pPrevSchedulerData->mpNext;
            pPrevSchedulerData->mpNext = pMostUrgent;
            pMostUrgent->mpNext = nullptr;
        }
    }

    if ( nMinPeriod != InfiniteTimeoutMs
            && ((eIdleRunPolicy == IdleRunPolicy::IDLE_VIA_TIMER)
                || (nMinPeriod > ImmediateTimeoutMs)) )
    {
        SAL_INFO( "vcl.schedule", "Scheduler sleep timeout: " << nMinPeriod );
        ImplStartTimer( nMinPeriod, true );
    }
    else if ( pSVData->mpSalTimer )
        pSVData->mpSalTimer->Stop();

    pSVData->mnTimerPeriod = nMinPeriod;
    pSVData->mnLastUpdate = nTime;

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
        if ( pSVData->mpFreeSchedulerData )
        {
            mpSchedulerData = pSVData->mpFreeSchedulerData;
            pSVData->mpFreeSchedulerData = mpSchedulerData->mpNext;
        }
        else
            mpSchedulerData = new ImplSchedulerData;
        mpSchedulerData->mpScheduler   = this;
        mpSchedulerData->mbInScheduler = false;
        mpSchedulerData->mpNext = nullptr;

        // insert last due to SFX!
        ImplSchedulerData* pData = pSVData->mpFirstSchedulerData;
        if ( pData )
        {
            while ( pData->mpNext )
                pData = pData->mpNext;
            pData->mpNext = mpSchedulerData;
        }
        else
            pSVData->mpFirstSchedulerData = mpSchedulerData;
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " " << mpSchedulerData
             <<  "  added      " << (int) mePriority << " " << mpDebugName );
    }
    else
    {
        if ( mpSchedulerData->mpNext )
        {
            ImplSchedulerData* pData = pSVData->mpFirstSchedulerData;
            while ( pData->mpNext != mpSchedulerData )
                pData = pData->mpNext;
            pData->mpNext = mpSchedulerData->mpNext;
            mpSchedulerData->mpNext = nullptr;
            while ( pData->mpNext )
                pData = pData->mpNext;
            pData->mpNext = mpSchedulerData;
            SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " " << mpSchedulerData
                 <<  "  restarted  " << (int) mePriority << " " << mpDebugName );
        }
        else
            SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " " << mpSchedulerData
                 <<  "  kept       " << (int) mePriority << " " << mpDebugName );
    }

    assert( mpSchedulerData->mpScheduler == this );
    mpSchedulerData->mnUpdateTime = tools::Time::GetSystemTicks();
    pSVData->mbNeedsReschedule = true;
}

void Scheduler::Stop()
{
    if ( !mpSchedulerData )
        return;
    ImplSchedulerData *pData = mpSchedulerData;
    Scheduler::SetDeletionFlags();
    SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " " << pData
          << "  stopped    " << (int) mePriority << " " << mpDebugName );
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
    mePriority(SchedulerPriority::DEFAULT)
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
