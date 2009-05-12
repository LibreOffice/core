/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: timer.cxx,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/saltimer.hxx>
#include <tools/time.hxx>
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salinst.hxx>
#include <tools/debug.hxx>
#include <vcl/timer.hxx>



// =======================================================================

#define MAX_TIMER_PERIOD    ((ULONG)0xFFFFFFFF)

// ---------------------
// - TimeManager-Types -
// ---------------------

struct ImplTimerData
{
    ImplTimerData*  mpNext;         // Pointer to the next Instance
    Timer*          mpSVTimer;      // Pointer to SV Timer instance
    ULONG           mnUpdateTime;   // Last Update Time
    ULONG           mnTimerUpdate;  // TimerCallbackProcs on stack
    BOOL            mbDelete;       // Wurde Timer waehren Update() geloescht
    BOOL            mbInTimeout;    // Befinden wir uns im Timeout-Handler
};

// =======================================================================

void Timer::ImplDeInitTimer()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplTimerData*  pTimerData = pSVData->mpFirstTimerData;

    if ( pTimerData )
    {
        do
        {
            ImplTimerData* pTempTimerData = pTimerData;
            if ( pTimerData->mpSVTimer )
            {
                pTimerData->mpSVTimer->mbActive = FALSE;
                pTimerData->mpSVTimer->mpTimerData = NULL;
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

// -----------------------------------------------------------------------

static void ImplStartTimer( ImplSVData* pSVData, ULONG nMS )
{
    if ( !nMS )
        nMS = 1;

    if ( nMS != pSVData->mnTimerPeriod )
    {
        pSVData->mnTimerPeriod = nMS;
        pSVData->mpSalTimer->Start( nMS );
    }
}

// -----------------------------------------------------------------------

void Timer::ImplTimerCallbackProc()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplTimerData*  pTimerData;
    ImplTimerData*  pPrevTimerData;
    ULONG           nMinPeriod = MAX_TIMER_PERIOD;
    ULONG           nDeltaTime;
    ULONG           nTime = Time::GetSystemTicks();

    if ( pSVData->mbNoCallTimer )
        return;

    pSVData->mnTimerUpdate++;
    pSVData->mbNotAllTimerCalled = TRUE;

    // Suche Timer raus, wo der Timeout-Handler gerufen werden muss
    pTimerData = pSVData->mpFirstTimerData;
    while ( pTimerData )
    {
        // Wenn Timer noch nicht neu ist und noch nicht geloescht wurde
        // und er sich nicht im Timeout-Handler befindet,
        // dann den Handler rufen, wenn die Zeit abgelaufen ist
        if ( (pTimerData->mnTimerUpdate < pSVData->mnTimerUpdate) &&
             !pTimerData->mbDelete && !pTimerData->mbInTimeout )
        {
            // Zeit abgelaufen
            if ( (pTimerData->mnUpdateTime+pTimerData->mpSVTimer->mnTimeout) <= nTime )
            {
                // Neue Updatezeit setzen
                pTimerData->mnUpdateTime = nTime;

                // kein AutoTimer, dann anhalten
                if ( !pTimerData->mpSVTimer->mbAuto )
                {
                    pTimerData->mpSVTimer->mbActive = FALSE;
                    pTimerData->mbDelete = TRUE;
                }

                // call Timeout
                pTimerData->mbInTimeout = TRUE;
                pTimerData->mpSVTimer->Timeout();
                pTimerData->mbInTimeout = FALSE;
            }
        }

        pTimerData = pTimerData->mpNext;
    }

    // Neue Zeit ermitteln
    ULONG nNewTime = Time::GetSystemTicks();
    pPrevTimerData = NULL;
    pTimerData = pSVData->mpFirstTimerData;
    while ( pTimerData )
    {
        // Befindet sich Timer noch im Timeout-Handler, dann ignorieren
        if ( pTimerData->mbInTimeout )
        {
            pPrevTimerData = pTimerData;
            pTimerData = pTimerData->mpNext;
        }
        // Wurde Timer zwischenzeitlich zerstoert ?
        else if ( pTimerData->mbDelete )
        {
            if ( pPrevTimerData )
                pPrevTimerData->mpNext = pTimerData->mpNext;
            else
                pSVData->mpFirstTimerData = pTimerData->mpNext;
            if ( pTimerData->mpSVTimer )
                pTimerData->mpSVTimer->mpTimerData = NULL;
            ImplTimerData* pTempTimerData = pTimerData;
            pTimerData = pTimerData->mpNext;
            delete pTempTimerData;
        }
        else
        {
            pTimerData->mnTimerUpdate = 0;
            // kleinste Zeitspanne ermitteln
            if ( pTimerData->mnUpdateTime == nTime )
            {
                nDeltaTime = pTimerData->mpSVTimer->mnTimeout;
                if ( nDeltaTime < nMinPeriod )
                    nMinPeriod = nDeltaTime;
            }
            else
            {
                nDeltaTime = pTimerData->mnUpdateTime + pTimerData->mpSVTimer->mnTimeout;
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

    // Wenn keine Timer mehr existieren, dann Clock loeschen
    if ( !pSVData->mpFirstTimerData )
    {
        pSVData->mpSalTimer->Stop();
        pSVData->mnTimerPeriod = MAX_TIMER_PERIOD;
    }
    else
        ImplStartTimer( pSVData, nMinPeriod );

    pSVData->mnTimerUpdate--;
    pSVData->mbNotAllTimerCalled = FALSE;
}

// =======================================================================

Timer::Timer()
{
    mpTimerData     = NULL;
    mnTimeout       = 1;
    mbAuto          = FALSE;
    mbActive        = FALSE;
}

// -----------------------------------------------------------------------

Timer::Timer( const Timer& rTimer )
{
    mpTimerData     = NULL;
    mnTimeout       = rTimer.mnTimeout;
    mbAuto          = FALSE;
    mbActive        = FALSE;
    maTimeoutHdl    = rTimer.maTimeoutHdl;

    if ( rTimer.IsActive() )
        Start();
}

// -----------------------------------------------------------------------

Timer::~Timer()
{
    if ( mpTimerData )
    {
        mpTimerData->mbDelete = TRUE;
        mpTimerData->mpSVTimer = NULL;
    }
}

// -----------------------------------------------------------------------

void Timer::Timeout()
{
    maTimeoutHdl.Call( this );
}

// -----------------------------------------------------------------------

void Timer::SetTimeout( ULONG nNewTimeout )
{
    mnTimeout = nNewTimeout;

    // Wenn Timer aktiv, dann Clock erneuern
    if ( mbActive )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( !pSVData->mnTimerUpdate && (mnTimeout < pSVData->mnTimerPeriod) )
            ImplStartTimer( pSVData, mnTimeout );
    }
}

// -----------------------------------------------------------------------

void Timer::Start()
{
    mbActive = TRUE;

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

        // insert timer and start
        mpTimerData                 = new ImplTimerData;
        mpTimerData->mpSVTimer      = this;
        mpTimerData->mnUpdateTime   = Time::GetSystemTicks();
        mpTimerData->mnTimerUpdate  = pSVData->mnTimerUpdate;
        mpTimerData->mbDelete       = FALSE;
        mpTimerData->mbInTimeout    = FALSE;

        // !!!!! Wegen SFX hinten einordnen !!!!!
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
    else if( !mpTimerData->mpSVTimer ) // TODO: remove when guilty found
    {
        DBG_ERROR( "Timer::Start() on a destroyed Timer!" );
    }
    else
    {
        mpTimerData->mnUpdateTime    = Time::GetSystemTicks();
        mpTimerData->mnTimerUpdate   = pSVData->mnTimerUpdate;
        mpTimerData->mbDelete        = FALSE;
    }
}

// -----------------------------------------------------------------------

void Timer::Stop()
{
    mbActive = FALSE;

    if ( mpTimerData )
        mpTimerData->mbDelete = TRUE;
}

// -----------------------------------------------------------------------

Timer& Timer::operator=( const Timer& rTimer )
{
    if ( IsActive() )
        Stop();

    mbActive        = FALSE;
    mnTimeout       = rTimer.mnTimeout;
    maTimeoutHdl    = rTimer.maTimeoutHdl;

    if ( rTimer.IsActive() )
        Start();

    return *this;
}

// =======================================================================

AutoTimer::AutoTimer()
{
    mbAuto = TRUE;
}

// -----------------------------------------------------------------------

AutoTimer::AutoTimer( const AutoTimer& rTimer ) : Timer( rTimer )
{
    mbAuto = TRUE;
}

// -----------------------------------------------------------------------

AutoTimer& AutoTimer::operator=( const AutoTimer& rTimer )
{
    Timer::operator=( rTimer );
    return *this;
}
