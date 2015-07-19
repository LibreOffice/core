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
#include <vcl/timer.hxx>
#include <saltimer.hxx>

#define MAX_TIMER_PERIOD    SAL_MAX_UINT64

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

ImplSchedulerData *ImplSchedulerData::GetMostImportantTask( bool bTimer )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplSchedulerData *pMostUrgent = NULL;

    for ( ImplSchedulerData *pSchedulerData = pSVData->mpFirstSchedulerData; pSchedulerData; pSchedulerData = pSchedulerData->mpNext )
    {
        if ( !pSchedulerData->mpScheduler || pSchedulerData->mbDelete || pSchedulerData->mnUpdateStack >= pSVData->mnUpdateStack
            || !pSchedulerData->mpScheduler->ReadyForSchedule( bTimer ) || !pSchedulerData->mpScheduler->IsActive())
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
                pSchedulerData->mpScheduler->mpSchedulerData = NULL;
            }
            pSchedulerData = pSchedulerData->mpNext;
            delete pTempSchedulerData;
        }
        while ( pSchedulerData );

        pSVData->mpFirstSchedulerData   = NULL;
        pSVData->mnTimerPeriod      = 0;
    }

    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer = 0;
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
    ImplSchedulerData* pSchedulerData = NULL;
    ImplSchedulerData* pPrevSchedulerData = NULL;
    ImplSVData*        pSVData = ImplGetSVData();
    sal_uInt64         nTime = tools::Time::GetSystemTicks();
    sal_uInt64         nMinPeriod = MAX_TIMER_PERIOD;
    pSVData->mnUpdateStack++;

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
                pSchedulerData->mpScheduler->mpSchedulerData = NULL;
            ImplSchedulerData* pTempSchedulerData = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
            delete pTempSchedulerData;
        }
        else
        {
            pSchedulerData->mnUpdateStack = 0;
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
        pSVData->mnTimerPeriod = MAX_TIMER_PERIOD;
    }
    else
    {
        Timer::ImplStartTimer( pSVData, nMinPeriod );
    }
    pSVData->mnUpdateStack--;
}

void Scheduler::SetPriority( SchedulerPriority ePriority )
{
    mePriority = ePriority;
}

void Scheduler::Start()
{
    // Mark timer active
    mbActive = true;

    ImplSVData* pSVData = ImplGetSVData();
    if ( !mpSchedulerData )
    {
        // insert Scheduler
        mpSchedulerData                = new ImplSchedulerData;
        mpSchedulerData->mpScheduler   = this;
        mpSchedulerData->mbInScheduler = false;

        // insert last due to SFX!
        ImplSchedulerData* pPrev = NULL;
        ImplSchedulerData* pData = pSVData->mpFirstSchedulerData;
        while ( pData )
        {
            pPrev = pData;
            pData = pData->mpNext;
        }
        mpSchedulerData->mpNext = NULL;
        if ( pPrev )
            pPrev->mpNext = mpSchedulerData;
        else
            pSVData->mpFirstSchedulerData = mpSchedulerData;
    }
    mpSchedulerData->mbDelete      = false;
    mpSchedulerData->mnUpdateTime  = tools::Time::GetSystemTicks();
    mpSchedulerData->mnUpdateStack = pSVData->mnUpdateStack;
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

    mbActive          = false;
    mePriority = rScheduler.mePriority;

    if ( rScheduler.IsActive() )
        Start();

    return *this;
}

Scheduler::Scheduler(const sal_Char *pDebugName):
    mpSchedulerData(NULL),
    mpDebugName(pDebugName),
    mePriority(SchedulerPriority::HIGH),
    mbActive(false)
{
}

Scheduler::Scheduler( const Scheduler& rScheduler ):
    mpSchedulerData(NULL),
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
        mpSchedulerData->mpScheduler = NULL;
    }
}

