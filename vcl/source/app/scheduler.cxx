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
    if (mbDelete || mbInScheduler )
        return;

    // prepare Scheduler Object for deletion after handling
    mpScheduler->SetDeletionFlags();

    // tdf#92036 Reset the period to avoid re-firing immediately.
    mpScheduler->mpSchedulerData->mnUpdateTime = tools::Time::GetSystemTicks();

    // invoke it
    mbInScheduler = true;
    mpScheduler->Invoke();
    mbInScheduler = false;
}

ImplSchedulerData *ImplSchedulerData::GetMostImportantTask( bool bTimerOnly )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplSchedulerData *pMostUrgent = nullptr;

    for ( ImplSchedulerData *pSchedulerData = pSVData->mpFirstSchedulerData; pSchedulerData; pSchedulerData = pSchedulerData->mpNext )
    {
        if ( !pSchedulerData->mpScheduler || pSchedulerData->mbDelete ||
             !pSchedulerData->mpScheduler->ReadyForSchedule( bTimerOnly ) ||
             !pSchedulerData->mpScheduler->IsActive())
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
    mpSchedulerData->mbDelete = true;
    mbActive = false;
}

void Scheduler::ImplDeInitScheduler()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplSchedulerData*  pSchedulerData = pSVData->mpFirstSchedulerData;
    if (pSVData->mpSalTimer)
    {
        pSVData->mpSalTimer->Stop();
    }

    if ( pSchedulerData )
    {
        do
        {
            ImplSchedulerData* pTempSchedulerData = pSchedulerData;
            if ( pSchedulerData->mpScheduler )
            {
                pSchedulerData->mpScheduler->mbActive = false;
                pSchedulerData->mpScheduler->mpSchedulerData = nullptr;
            }
            pSchedulerData = pSchedulerData->mpNext;
            delete pTempSchedulerData;
        }
        while ( pSchedulerData );

        pSVData->mpFirstSchedulerData = nullptr;
        pSVData->mnTimerPeriod = 0;
    }

    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer = nullptr;
}

/**
 * Start a new timer if we need to for @nMS duration.
 *
 * if this is longer than the existing duration we're
 * waiting for, do nothing - unless @bForce - which means
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

/**
* Initialize the platform specific timer on which all the
* platform independent timers are built
*/
void Scheduler::InitSystemTimer(ImplSVData* pSVData)
{
    assert(pSVData != nullptr);
    if (!pSVData->mpSalTimer)
    {
        pSVData->mnTimerPeriod = MaximumTimeoutMs;
        pSVData->mpSalTimer = pSVData->mpDefInst->CreateSalTimer();
        pSVData->mpSalTimer->SetCallback(CallbackTaskScheduling);
    }
}

void Scheduler::CallbackTaskScheduling(bool ignore)
{
    // this function is for the saltimer callback
    (void)ignore;
    Scheduler::ProcessTaskScheduling( true );
}

void Scheduler::ProcessTaskScheduling( bool bTimer )
{
    // process all pending Tasks
    // if bTimer True, only handle timer
    ImplSchedulerData* pSchedulerData = nullptr;
    ImplSchedulerData* pPrevSchedulerData = nullptr;
    ImplSVData*        pSVData = ImplGetSVData();
    sal_uInt64         nTime = tools::Time::GetSystemTicks();
    sal_uInt64         nMinPeriod = MaximumTimeoutMs;

    // tdf#91727 - NB. bTimer is ultimately not used
    if ((pSchedulerData = ImplSchedulerData::GetMostImportantTask(bTimer)))
    {
        pSchedulerData->mnUpdateTime = nTime;
        pSchedulerData->Invoke();
    }

    pSchedulerData = pSVData->mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        if( pSchedulerData->mbInScheduler )
        {
            pPrevSchedulerData = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
        }
        // Should Task be released from scheduling?
        else if ( pSchedulerData->mbDelete )
        {
            if ( pPrevSchedulerData )
                pPrevSchedulerData->mpNext = pSchedulerData->mpNext;
            else
                pSVData->mpFirstSchedulerData = pSchedulerData->mpNext;
            if ( pSchedulerData->mpScheduler )
                pSchedulerData->mpScheduler->mpSchedulerData = nullptr;
            ImplSchedulerData* pTempSchedulerData = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
            delete pTempSchedulerData;
        }
        else
        {
            nMinPeriod = pSchedulerData->mpScheduler->UpdateMinPeriod( nMinPeriod, nTime );
            pPrevSchedulerData = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
        }
    }

    // delete clock if no more timers available
    if ( !pSVData->mpFirstSchedulerData )
    {
        if ( pSVData->mpSalTimer )
            pSVData->mpSalTimer->Stop();
        pSVData->mnTimerPeriod = MaximumTimeoutMs;
    }
    else
    {
        Scheduler::ImplStartTimer(nMinPeriod, true);
    }
}

void Scheduler::Start()
{
    ImplSVData *const pSVData = ImplGetSVData();
    if (pSVData->mbDeInit)
    {
        return;
    }

    // Mark timer active
    mbActive = true;

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
    mpSchedulerData->mbDelete      = false;
    mpSchedulerData->mnUpdateTime  = tools::Time::GetSystemTicks();
}

void Scheduler::Stop()
{
    mbActive = false;

    if ( mpSchedulerData )
        mpSchedulerData->mbDelete = true;
}

Scheduler& Scheduler::operator=( const Scheduler& rScheduler )
{
    if ( IsActive() )
        Stop();

    mbActive = false;
    mePriority = rScheduler.mePriority;

    if ( rScheduler.IsActive() )
        Start();

    return *this;
}

Scheduler::Scheduler(const sal_Char *pDebugName):
    mpSchedulerData(nullptr),
    mpDebugName(pDebugName),
    mePriority(SchedulerPriority::HIGH),
    mbActive(false)
{
}

Scheduler::Scheduler( const Scheduler& rScheduler ):
    mpSchedulerData(nullptr),
    mpDebugName(rScheduler.mpDebugName),
    mePriority(rScheduler.mePriority),
    mbActive(false)
{
    if ( rScheduler.IsActive() )
        Start();
}

Scheduler::~Scheduler()
{
    if ( mpSchedulerData )
    {
        mpSchedulerData->mbDelete = true;
        mpSchedulerData->mpScheduler = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
