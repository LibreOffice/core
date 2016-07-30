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

namespace {
const sal_uInt64 MaximumTimeoutMs = 1000 * 60; // 1 minute
void InitSystemTimer(ImplSVData* pSVData);
}

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

ImplSchedulerData *ImplSchedulerData::GetMostImportantTask( bool bTimerOnly )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplSchedulerData *pMostUrgent = nullptr;

    sal_uInt64 nTimeNow = tools::Time::GetSystemTicks();
    for ( ImplSchedulerData *pSchedulerData = pSVData->mpFirstSchedulerData; pSchedulerData; pSchedulerData = pSchedulerData->mpNext )
    {
        if ( !pSchedulerData->mpScheduler || !pSchedulerData->mpScheduler->ReadyForSchedule( bTimerOnly, nTimeNow ) )
            continue;
        if (!pMostUrgent)
            pMostUrgent = pSchedulerData;
        else
        {
            // Find the highest priority.
            // If the priority of the current task is higher (numerical value is lower) than
            // the priority of the most urgent, the current task gets the new most urgent.
            if ( pSchedulerData->mpScheduler->GetPriority() < pMostUrgent->mpScheduler->GetPriority() )
                pMostUrgent = pSchedulerData;
        }
    }

    return pMostUrgent;
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
 * waiting for, do nothing - unless bForce - which means
 * to reset the minimum period; used by the scheduled itself.
 */
void Scheduler::ImplStartTimer(sal_uInt64 nMS, bool bForce)
{
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mbDeInit)
    {
        // do not start new timers during shutdown - if that happens after
        // ImplSalStopTimer() on WNT the timer queue is restarted and never ends
        return;
    }
    InitSystemTimer(pSVData);

    if ( !nMS )
        nMS = 1;

    // Only if smaller timeout, to avoid skipping.
    if (bForce || nMS < pSVData->mnTimerPeriod)
    {
        pSVData->mnTimerPeriod = nMS;
        pSVData->mpSalTimer->Start(nMS);
    }
}

namespace {

/**
* Initialize the platform specific timer on which all the
* platform independent timers are built
*/
void InitSystemTimer(ImplSVData* pSVData)
{
    assert(pSVData != nullptr);
    if (!pSVData->mpSalTimer)
    {
        pSVData->mnTimerPeriod = MaximumTimeoutMs;
        pSVData->mpSalTimer = pSVData->mpDefInst->CreateSalTimer();
        pSVData->mpSalTimer->SetCallback(Scheduler::CallbackTaskScheduling);
    }
}

}

void Scheduler::CallbackTaskScheduling(bool)
{
    // this function is for the saltimer callback
    Scheduler::ProcessTaskScheduling( false );
}

bool Scheduler::ProcessTaskScheduling( bool bTimerOnly )
{
    ImplSchedulerData* pSchedulerData;

    if ((pSchedulerData = ImplSchedulerData::GetMostImportantTask(bTimerOnly)))
    {
        SAL_INFO("vcl.schedule", "Invoke task " << pSchedulerData->GetDebugName());

        pSchedulerData->mnUpdateTime = tools::Time::GetSystemTicks();
        pSchedulerData->Invoke();
        return true;
    }
    else
        return false;
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

sal_uInt64 Scheduler::CalculateMinimumTimeout( bool &bHasActiveIdles )
{
    // process all pending Tasks
    // if bTimer True, only handle timer
    ImplSchedulerData* pSchedulerData = nullptr;
    ImplSchedulerData* pPrevSchedulerData = nullptr;
    ImplSVData*        pSVData = ImplGetSVData();
    sal_uInt64         nTime = tools::Time::GetSystemTicks();
    sal_uInt64         nMinPeriod = MaximumTimeoutMs;

    SAL_INFO("vcl.schedule", "Calculating minimum timeout:");
    pSchedulerData = pSVData->mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        ImplSchedulerData *pNext = pSchedulerData->mpNext;

        // Should Task be released from scheduling?
        if ( !pSchedulerData->mbInScheduler &&
             !pSchedulerData->mpScheduler )
        {
            if ( pPrevSchedulerData )
                pPrevSchedulerData->mpNext = pSchedulerData->mpNext;
            else
                pSVData->mpFirstSchedulerData = pSchedulerData->mpNext;
            if ( pSchedulerData->mpScheduler )
                pSchedulerData->mpScheduler->mpSchedulerData = nullptr;
            pNext = pSchedulerData->mpNext;
            delete pSchedulerData;
        }
        else
        {
            if (!pSchedulerData->mbInScheduler)
            {
                if ( !pSchedulerData->mpScheduler->IsIdle() )
                {
                    sal_uInt64 nOldMinPeriod = nMinPeriod;
                    nMinPeriod = pSchedulerData->mpScheduler->UpdateMinPeriod(
                                                                nOldMinPeriod, nTime );
                    SAL_INFO("vcl.schedule", "Have active timer " <<
                             pSchedulerData->GetDebugName() <<
                             "update min period from " << nOldMinPeriod <<
                             " to " << nMinPeriod);
                }
                else
                {
                    SAL_INFO("vcl.schedule", "Have active idle " <<
                             pSchedulerData->GetDebugName());
                    bHasActiveIdles = true;
                }
            }
            pPrevSchedulerData = pSchedulerData;
        }
        pSchedulerData = pNext;
    }

    // delete clock if no more timers available,
    if ( !pSVData->mpFirstSchedulerData )
    {
        if ( pSVData->mpSalTimer )
            pSVData->mpSalTimer->Stop();
        nMinPeriod = MaximumTimeoutMs;
        pSVData->mnTimerPeriod = nMinPeriod;
        SAL_INFO("vcl.schedule", "Unusual - no more timers available - stop timer");
    }
    else
    {
        Scheduler::ImplStartTimer(nMinPeriod, true);
        SAL_INFO("vcl.schedule", "Calculated minimum timeout as " << nMinPeriod << " and " <<
                 (bHasActiveIdles ? "has active idles" : "no idles"));
    }

    return nMinPeriod;
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
