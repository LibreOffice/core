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
    ImplIdleData*   mpNext;         // Pointer to the next Instance
    Idle*           mpIdle;        // Pointer to VCL Idle instance
    bool            mbDelete;       // Was Idle deleted during Update()?
    bool            mbInIdle;    // Are we in a idle handler?

    void Invoke()
    {
        if (mbDelete || mbInIdle )
            return;

        mpIdle->SetPriority(mpIdle->GetDefaultPriority());
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
                if ( p->mpIdle->GetPriority() < pMostUrgent->mpIdle->GetPriority() )
                {
                    IncreasePriority(pMostUrgent->mpIdle);
                    pMostUrgent = p;
                }
                else
                    IncreasePriority(p->mpIdle);
            }
        }

        return pMostUrgent;
    }

    static void IncreasePriority( Idle *pIdle )
    {
        switch(pIdle->GetPriority())
        {
            // Increase priority based on their current priority
            case IdlePriority::VCL_IDLE_PRIORITY_STARVATIONPROTECTION:
                break;
            // If already highest priority -> extra state for starving tasks
            case IdlePriority::VCL_IDLE_PRIORITY_HIGHEST:
                pIdle->SetPriority(IdlePriority::VCL_IDLE_PRIORITY_STARVATIONPROTECTION);
                break;
            case IdlePriority::VCL_IDLE_PRIORITY_HIGH:
                pIdle->SetPriority(IdlePriority::VCL_IDLE_PRIORITY_HIGHEST);
                break;
            case IdlePriority::VCL_IDLE_PRIORITY_REPAINT:
                pIdle->SetPriority(IdlePriority::VCL_IDLE_PRIORITY_HIGH);
                break;
            case IdlePriority::VCL_IDLE_PRIORITY_MEDIUM:
                pIdle->SetPriority(IdlePriority::VCL_IDLE_PRIORITY_REPAINT);
                break;
            case IdlePriority::VCL_IDLE_PRIORITY_LOW:
                pIdle->SetPriority(IdlePriority::VCL_IDLE_PRIORITY_MEDIUM);
                break;
            case IdlePriority::VCL_IDLE_PRIORITY_LOWER:
                pIdle->SetPriority(IdlePriority::VCL_IDLE_PRIORITY_LOW);
                break;
            case IdlePriority::VCL_IDLE_PRIORITY_LOWEST:
                pIdle->SetPriority(IdlePriority::VCL_IDLE_PRIORITY_LOWER);
                break;
        }
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
    // process all pending Idle
    ImplIdleData* pIdleData = NULL;
    ImplIdleData* pPrevIdleData = NULL;
    ImplSVData*     pSVData = ImplGetSVData();
    while (!Timer::TimerReady() && (pIdleData = ImplIdleData::GetFirstIdle()))
    {
        pIdleData->Invoke();
    }

    pIdleData = pSVData->mpFirstIdleData;
    while ( pIdleData )
    {
        // Was Idle destroyed in the meantime?
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
    mePriority = ePriority;
    // Was a new priority set before excecution?
    // Then take it as default priority
    if( !mbActive && meDefaultPriority == IdlePriority::VCL_IDLE_PRIORITY_DEFAULT )
        meDefaultPriority = mePriority;
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
        mpIdleData = new ImplIdleData;
        mpIdleData->mpIdle        = this;
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

    mbActive        = false;
    mePriority       = rIdle.mePriority;
    meDefaultPriority = rIdle.meDefaultPriority;
    maIdleHdl    = rIdle.maIdleHdl;

    if ( rIdle.IsActive() )
        Start();

    return *this;
}

Idle::Idle():
    mpIdleData(NULL),
    mePriority(IdlePriority::VCL_IDLE_PRIORITY_DEFAULT),
    meDefaultPriority(IdlePriority::VCL_IDLE_PRIORITY_DEFAULT),
    mbActive(false)
{
}

Idle::Idle( const Idle& rIdle ):
    mpIdleData(NULL),
    mePriority(rIdle.mePriority),
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
