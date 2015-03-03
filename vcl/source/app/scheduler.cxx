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

void ImplSchedulerData::Invoke()
{
    if (mbDelete || mbInScheduler )
        return;

    mpScheduler->SetSchedulingPriority(static_cast<sal_Int32>(mpScheduler->GetDefaultPriority()));
    mpScheduler->SetDeletionFlags();

    // invoke it
    mbInScheduler = true;
    mpScheduler->Invoke();
    mbInScheduler = false;
}

ImplSchedulerData *ImplSchedulerData::GetMostImportantTask( bool bTimer )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplSchedulerData *pMostUrgent = NULL;

    for ( ImplSchedulerData *p = pSVData->mpFirstSchedulerData; p; p = p->mpNext )
    {
        if ( !p->mpScheduler || p->mbDelete || p->mnUpdateStack >= pSVData->mnTimerUpdate || !p->mpScheduler->ReadyForSchedule( bTimer ) )
            continue;
        if (!pMostUrgent)
            pMostUrgent = p;
        else
        {
            // Find the highest priority.
            // If the priority of the current idle is higher (numerical value is lower) than
            // the priority of the most urgent, the priority of most urgent is increased and
            // the current is the new most urgent. So starving is impossible.
            if ( p->mpScheduler->GetPriority() < pMostUrgent->mpScheduler->GetPriority() )
                pMostUrgent = p;
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
    }
}

void Scheduler::CallbackTaskScheduling()
{
    Scheduler::ProcessTaskScheduling( true );
}

void Scheduler::ProcessTaskScheduling( bool bTimer )
{
    // process all pending Tasks
    ImplSchedulerData* pSchedulerData = NULL;
    ImplSchedulerData* pPrevSchedulerData = NULL;
    ImplSVData*        pSVData = ImplGetSVData();
    pSVData->mnTimerUpdate++;

    if ((pSchedulerData = ImplSchedulerData::GetMostImportantTask(bTimer)))
    {
        pSchedulerData->mnUpdateTime = tools::Time::GetSystemTicks();
        pSchedulerData->Invoke();
    }

    pSchedulerData = pSVData->mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        // Should Task be released from scheduling?
        if ( pSchedulerData->mbDelete )
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
            if( !pSchedulerData->mbInScheduler )
                pSchedulerData->mnUpdateStack = 0;
            pPrevSchedulerData = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
        }
    }
    pSVData->mnTimerUpdate--;
}

void Scheduler::SetPriority( SchedulerPriority ePriority )
{
    meDefaultPriority = ePriority;
}

void Scheduler::SetSchedulingPriority( sal_Int32 iPriority )
{
    miPriority = iPriority;
}

void Scheduler::Start()
{
    // Mark timer active
    mbActive = true;

    ImplSVData* pSVData = ImplGetSVData();
    if ( !mpSchedulerData )
    {
        // insert Idle
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
    mpSchedulerData->mnUpdateStack = pSVData->mnTimerUpdate;
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
    miPriority        = rScheduler.miPriority;
    meDefaultPriority = rScheduler.meDefaultPriority;

    if ( rScheduler.IsActive() )
        Start();

    return *this;
}

Scheduler::Scheduler():
    mpSchedulerData(NULL),
    miPriority(static_cast<sal_Int32>(SchedulerPriority::HIGH)),
    meDefaultPriority(SchedulerPriority::HIGH),
    mbActive(false)
{
}

Scheduler::Scheduler( const Scheduler& rScheduler ):
    mpSchedulerData(NULL),
    miPriority(rScheduler.miPriority),
    meDefaultPriority(rScheduler.meDefaultPriority),
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

