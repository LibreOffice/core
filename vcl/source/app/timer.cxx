/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    ImplTimerData*  mpNext;         
    Timer*          mpTimer;        
    sal_uLong       mnUpdateTime;   
    sal_uLong       mnTimerUpdate;  
    bool            mbDelete;       
    bool            mbInTimeout;    
};

void Timer::ImplDeInitTimer()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplTimerData*  pTimerData = pSVData->mpFirstTimerData;

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
        delete pSVData->mpSalTimer;
        pSVData->mpSalTimer = NULL;
    }
}

static void ImplStartTimer( ImplSVData* pSVData, sal_uLong nMS )
{
    if ( !nMS )
        nMS = 1;

    if ( nMS != pSVData->mnTimerPeriod )
    {
        pSVData->mnTimerPeriod = nMS;
        pSVData->mpSalTimer->Start( nMS );
    }
}

void Timer::ImplTimerCallbackProc()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplTimerData*  pTimerData;
    ImplTimerData*  pPrevTimerData;
    sal_uLong       nMinPeriod = MAX_TIMER_PERIOD;
    sal_uLong       nDeltaTime;
    sal_uLong       nTime = Time::GetSystemTicks();

    if ( pSVData->mbNoCallTimer )
        return;

    pSVData->mnTimerUpdate++;
    pSVData->mbNotAllTimerCalled = true;

    
    pTimerData = pSVData->mpFirstTimerData;
    while ( pTimerData )
    {
        
        
        if ( (pTimerData->mnTimerUpdate < pSVData->mnTimerUpdate) &&
             !pTimerData->mbDelete && !pTimerData->mbInTimeout )
        {
            
            if ( (pTimerData->mnUpdateTime+pTimerData->mpTimer->mnTimeout) <= nTime )
            {
                
                pTimerData->mnUpdateTime = nTime;

                
                if ( !pTimerData->mpTimer->mbAuto )
                {
                    pTimerData->mpTimer->mbActive = false;
                    pTimerData->mbDelete = true;
                }

                
                pTimerData->mbInTimeout = true;
                pTimerData->mpTimer->Timeout();
                pTimerData->mbInTimeout = false;
            }
        }

        pTimerData = pTimerData->mpNext;
    }

    
    sal_uLong nNewTime = Time::GetSystemTicks();
    pPrevTimerData = NULL;
    pTimerData = pSVData->mpFirstTimerData;
    while ( pTimerData )
    {
        
        if ( pTimerData->mbInTimeout )
        {
            pPrevTimerData = pTimerData;
            pTimerData = pTimerData->mpNext;
        }
        
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

Timer::Timer():
    mpTimerData(NULL),
    mnTimeout(1),
    mbActive(false),
    mbAuto(false)
{
}

Timer::Timer( const Timer& rTimer ):
    mpTimerData(NULL),
    mnTimeout(rTimer.mnTimeout),
    mbActive(false),
    mbAuto(false),
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

        
        mpTimerData = new ImplTimerData;
        mpTimerData->mpTimer        = this;
        mpTimerData->mnUpdateTime   = Time::GetSystemTicks();
        mpTimerData->mnTimerUpdate  = pSVData->mnTimerUpdate;
        mpTimerData->mbDelete       = false;
        mpTimerData->mbInTimeout    = false;

        
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
    else if( !mpTimerData->mpTimer ) 
    {
        OSL_FAIL( "Timer::Start() on a destroyed Timer!" );
    }
    else
    {
        mpTimerData->mnUpdateTime    = Time::GetSystemTicks();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
