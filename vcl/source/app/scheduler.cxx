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
#include <algorithm>
#include <saltimer.hxx>

void Scheduler::ImplInvoke(sal_uInt64 nTime)
{
    mnUpdateTime = nTime;

    if (mpSchedulerData->mbDelete || mbInScheduler )
        return;

    // prepare Scheduler Object for deletion after handling
    SetDeletionFlags();

    // invoke it
    mbInScheduler = true;
    Invoke();
    mbInScheduler = false;
}

Scheduler* Scheduler::ImplGetHighestPrioTask( bool bTimer )
{
    ImplSVData*     pSVData     = ImplGetSVData();
    Scheduler *     pMostUrgent = NULL;

    std::for_each(pSVData->maSchedulers->begin(), pSVData->maSchedulers->end(),
        [&pSVData, bTimer, &pMostUrgent] (ImplSchedulerData *rScheduler)
    {
        if ( rScheduler->mpScheduler &&
            rScheduler->mpScheduler->ImplIsScheduleReady(pSVData->mnUpdateStack) &&
            rScheduler->mpScheduler->ReadyForSchedule( bTimer ) &&
            rScheduler->mpScheduler->IsActive() )
        {
            if (!pMostUrgent)
                pMostUrgent = rScheduler->mpScheduler;
            else
            {
                // Find the highest priority.
                // If the priority of the current task is higher (numerical value is lower) than
                // the priority of the most urgent, the current task gets the new most urgent.
                if ( rScheduler->mpScheduler->GetPriority() < pMostUrgent->GetPriority() )
                    pMostUrgent = rScheduler->mpScheduler;
            }
        }
    });

    return pMostUrgent;
}

void Scheduler::SetDeletionFlags()
{
    Stop();
}

void Scheduler::ImplDeInitScheduler(bool bAll /*=true*/)
{
    ImplSVData* pSVData = ImplGetSVData();

    if (pSVData->mpSalTimer)
    {
        pSVData->mpSalTimer->Stop();
    }

    pSVData->maSchedulers->remove_if( [] (ImplSchedulerData *rSchedulerData)
    {
        if(rSchedulerData->mpScheduler != NULL)
        {
            rSchedulerData->mpScheduler->ImplDispose();
        }
        else
            delete rSchedulerData;

        return true;
    });

    if(bAll)
    {
        delete pSVData->maSchedulers;
        pSVData->maSchedulers = NULL;
    }

    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer  = NULL;
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
    Scheduler*  pScheduler = NULL;
    ImplSVData* pSVData = ImplGetSVData();
    sal_uInt64  nTime = tools::Time::GetSystemTicks();
    sal_uInt64  nMinPeriod = MAX_TIMER_PERIOD;

    pSVData->mnUpdateStack++;

    // tdf#91727 - NB. bTimer is ultimately not used
    if ((pScheduler = Scheduler::ImplGetHighestPrioTask(bTimer)) != NULL)
        pScheduler->ImplInvoke(nTime);

    pSVData->maSchedulers->remove_if( [&nMinPeriod, nTime, pSVData] (ImplSchedulerData *rSchedulerData)
    {
        if (rSchedulerData->mpScheduler)
            return rSchedulerData->mpScheduler->ImplHandleTaskScheduling(nMinPeriod, nTime);
        else
        {
            delete rSchedulerData;
            return true;
        }
    });

    // delete clock if no more timers available
    if ( pSVData->maSchedulers->empty() )
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
    ImplSVData* pSVData = ImplGetSVData();
    // Mark timer active
    mbActive = true;

    if ( !mpSchedulerData )
    {
        mpSchedulerData = new ImplSchedulerData;
        mpSchedulerData->mpScheduler = this;
        // insert Scheduler
        mbInScheduler   = false;
        pSVData->maSchedulers->push_back(mpSchedulerData);
    }

    mpSchedulerData->mbDelete = false;
    mnUpdateTime              = tools::Time::GetSystemTicks();
    mnUpdateStack             = pSVData->mnUpdateStack;
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

    mbActive   = false;
    mePriority = rScheduler.mePriority;

    if ( rScheduler.IsActive() )
        Start();

    return *this;
}

Scheduler::Scheduler(const sal_Char *pDebugName):
    mpSchedulerData(NULL),
    mpDebugName(pDebugName),
    mePriority(SchedulerPriority::HIGH),
    mbActive(false),
    mnUpdateTime(0)
{
}

Scheduler::Scheduler( const Scheduler& rScheduler ):
    mpSchedulerData(NULL),
    mpDebugName(rScheduler.mpDebugName),
    mePriority(SchedulerPriority::HIGH),
    mbActive(false),
    mnUpdateTime(0)
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

bool Scheduler::ImplIsScheduleReady(sal_uInt32 nUpdateStack)
{
    return !mpSchedulerData->mbDelete && (mnUpdateStack <= nUpdateStack);
}

void Scheduler::ImplDispose()
{
    mpSchedulerData->mpScheduler = NULL;
    delete mpSchedulerData;
    mpSchedulerData = NULL;
}

void Scheduler::ImplInitScheduler()
{
    ImplSVData* pSVData = ImplGetSVData();

    if(pSVData->maSchedulers == NULL)
        pSVData->maSchedulers = new ImplScheduler_t;
}


bool  Scheduler::ImplHandleTaskScheduling(sal_uInt64 &nMinPeriod, sal_uInt64 nTime)
{
    // process all pending Tasks
    if( !mbInScheduler )
    {
        // Should Task be released from scheduling?
        if ( !mpSchedulerData->mbDelete )
        {
            mnUpdateStack = 0;
            nMinPeriod    = UpdateMinPeriod( nMinPeriod, nTime );
        }
        else
        {
            ImplDispose();
            return true;
        }
    }

    return false;
}
