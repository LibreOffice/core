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

#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>
#include <vcl/timer.hxx>

#include <svdata.hxx>
#include <salinst.hxx>

struct ImplIdleData
{
    ImplIdleData*   mpNext;      // Pointer to the next element in list
    Idle*           mpIdle;      // Pointer to VCL Idle instance
    bool            mbDelete;    // Destroy this idle?
    bool            mbInIdle;    // Idle handler currently processed?

    void Invoke()
    {
        if (mbDelete || mbInIdle )
            return;

        mpIdle->SetSchedulingPriority(static_cast<sal_Int32>(mpIdle->GetDefaultPriority()));
        mbDelete = true;
        mpIdle->mbActive = false;

        // invoke it
        mbInIdle = true;
        mpIdle->DoIdle();
        mbInIdle = false;
    }

    static ImplIdleData *GetFirstIdle()
    {
        ImplSVData*     pSVData = ImplGetSVData();
        ImplIdleData *pMostUrgent = NULL;

        for ( ImplIdleData *p = pSVData->mpFirstIdleData; p; p = p->mpNext )
        {
            if ( !p->mpIdle || p->mbDelete )
                continue;
            if (!pMostUrgent)
                pMostUrgent = p;
            else
            {
                // Find the highest priority.
                // If the priority of the current idle is higher (numerical value is lower) than
                // the priority of the most urgent, the priority of most urgent is increased and
                // the current is the new most urgent. So starving is impossible.
                if ( p->mpIdle->GetPriority() < pMostUrgent->mpIdle->GetPriority() )
                {
                    pMostUrgent->mpIdle->SetSchedulingPriority( pMostUrgent->mpIdle->GetPriority() - 1);
                    pMostUrgent = p;
                }
                else
                    p->mpIdle->SetSchedulingPriority( p->mpIdle->GetPriority() - 1);
            }
        }

        return pMostUrgent;
    }
};

void Idle::ImplDeInitIdle()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplIdleData*  pIdleData = pSVData->mpFirstIdleData;

    if ( pIdleData )
    {
        do
        {
            ImplIdleData* pTempIdleData = pIdleData;
            if ( pIdleData->mpIdle )
            {
                pIdleData->mpIdle->mbActive = false;
                pIdleData->mpIdle->mpIdleData = NULL;
            }
            pIdleData = pIdleData->mpNext;
            delete pTempIdleData;
        }
        while ( pIdleData );

        pSVData->mpFirstIdleData   = NULL;
    }
}

void Idle::ProcessAllIdleHandlers()
{
    // process all pending idle
    ImplIdleData* pIdleData = NULL;
    ImplIdleData* pPrevIdleData = NULL;
    ImplSVData*     pSVData = ImplGetSVData();
    // timer can interrupt idle
    while (!Timer::TimerReady() && (pIdleData = ImplIdleData::GetFirstIdle()))
    {
        pIdleData->Invoke();
    }

    pIdleData = pSVData->mpFirstIdleData;
    while ( pIdleData )
    {
        // Should idle be released from scheduling?
        if ( pIdleData->mbDelete )
        {
            if ( pPrevIdleData )
                pPrevIdleData->mpNext = pIdleData->mpNext;
            else
                pSVData->mpFirstIdleData = pIdleData->mpNext;
            if ( pIdleData->mpIdle )
                pIdleData->mpIdle->mpIdleData = NULL;
            ImplIdleData* pTempIdleData = pIdleData;
            pIdleData = pIdleData->mpNext;
            delete pTempIdleData;
        }
        else
        {
            pPrevIdleData = pIdleData;
            pIdleData = pIdleData->mpNext;
        }
    }
}

void Idle::SetPriority( IdlePriority ePriority )
{
    meDefaultPriority = ePriority;
}

void Idle::SetSchedulingPriority( sal_Int32 iPriority )
{
    miPriority = iPriority;
}

void Idle::DoIdle()
{
    maIdleHdl.Call( this );
}

void Idle::Start()
{
    // Mark timer active
    mbActive = true;

    ImplSVData* pSVData = ImplGetSVData();
    if ( !mpIdleData )
    {
        // insert Idle
        mpIdleData              = new ImplIdleData;
        mpIdleData->mpIdle      = this;
        mpIdleData->mbInIdle    = false;

        // insert last due to SFX!
        ImplIdleData* pPrev = NULL;
        ImplIdleData* pData = pSVData->mpFirstIdleData;
        while ( pData )
        {
            pPrev = pData;
            pData = pData->mpNext;
        }
        mpIdleData->mpNext = NULL;
        if ( pPrev )
            pPrev->mpNext = mpIdleData;
        else
            pSVData->mpFirstIdleData = mpIdleData;
    }
    mpIdleData->mbDelete        = false;
}

void Idle::Stop()
{
    mbActive = false;

    if ( mpIdleData )
        mpIdleData->mbDelete = true;
}

Idle& Idle::operator=( const Idle& rIdle )
{
    if ( IsActive() )
        Stop();

    mbActive          = false;
    miPriority        = rIdle.miPriority;
    meDefaultPriority = rIdle.meDefaultPriority;
    maIdleHdl         = rIdle.maIdleHdl;

    if ( rIdle.IsActive() )
        Start();

    return *this;
}

Idle::Idle():
    mpIdleData(NULL),
    miPriority(static_cast<sal_Int32>(IdlePriority::HIGH)),
    meDefaultPriority(IdlePriority::HIGH),
    mbActive(false)
{
}

Idle::Idle( const Idle& rIdle ):
    mpIdleData(NULL),
    miPriority(rIdle.miPriority),
    meDefaultPriority(rIdle.meDefaultPriority),
    mbActive(false),
    maIdleHdl(rIdle.maIdleHdl)
{
    if ( rIdle.IsActive() )
        Start();
}

Idle::~Idle()
{
    if ( mpIdleData )
    {
        mpIdleData->mbDelete = true;
        mpIdleData->mpIdle = NULL;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
