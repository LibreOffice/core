/*************************************************************************
 *
 *  $RCSfile: timer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_TIMER_CXX

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
#endif
#else
#include <rmevents.hxx>
#endif

#ifndef _TIME_HXX
#include <tools/time.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

#define protected public
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#undef protected

#pragma hdrstop

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

void ImplDeInitTimer()
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
#ifndef REMOTE_APPSERVER
        SalTimer::Stop();
#else
        if ( pSVData->mpOTimer )
            pSVData->mpOTimer->StopTimer();
#endif
    }
}

// -----------------------------------------------------------------------

static void ImplStartTimer( ImplSVData* pSVData, ULONG nMS )
{
    if ( !nMS )
        nMS = 1;

#ifndef REMOTE_APPSERVER
    if ( nMS != pSVData->mnTimerPeriod )
    {
        pSVData->mnTimerPeriod = nMS;
        SalTimer::Start( nMS );
    }
#else
    pSVData->mnTimerPeriod = nMS;
    if ( !pSVData->mpOTimer )
    {
        pSVData->mpOTimer = new VclOTimer;
        pSVData->mpOTimer->acquire();
    }
    else
        pSVData->mpOTimer->StopTimer();
    pSVData->mpOTimer->setRemainingTime( nMS, 0 );
    pSVData->mpOTimer->StartTimer();
#endif
}

// -----------------------------------------------------------------------

void ImplTimerCallbackProc()
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
#ifndef REMOTE_APPSERVER
        SalTimer::Stop();
#else
        if ( pSVData->mpOTimer )
            pSVData->mpOTimer->StopTimer();
#endif
        pSVData->mnTimerPeriod = MAX_TIMER_PERIOD;
    }
    else
        ImplStartTimer( pSVData, nMinPeriod );

    pSVData->mnTimerUpdate--;
    pSVData->mbNotAllTimerCalled = FALSE;
}

// =======================================================================

#ifdef REMOTE_APPSERVER

VclOTimer::VclOTimer() :
    maTimeoutHdl( LINK( this, VclOTimer, Timeout ) )
{
    mbSend = FALSE;
}

void SAL_CALL VclOTimer::onShot()
{
    stop(); // start wird durch ImplTimerCallbackProc() gerufen
    if ( !mbSend )
    {
        mbSend = TRUE;
        if ( !Application::PostUserEvent( maTimeoutHdl ) )
        {
            mbSend = FALSE;
            start();
        }
    }
}

IMPL_LINK( VclOTimer, Timeout, void*, EMPTYARG )
{
    mbSend = FALSE;
    ImplTimerCallbackProc();
    return 0;
}

void VclOTimer::StartTimer()
{
    start();
}

void VclOTimer::StopTimer()
{
    stop();
}

#endif

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
#ifndef REMOTE_APPSERVER
            SalTimer::SetCallback( ImplTimerCallbackProc );
#endif
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
