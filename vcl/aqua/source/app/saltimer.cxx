/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saltimer.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 10:06:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_SALTIMER_H
#include <vcl/saltimer.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif

// =======================================================================


#define USEMAINTHREAD 1

void ImplSalStartTimer ( ULONG nMS, BOOL bMutex)
{
    AquaLog( "ImplSalStartTimer\n");

    SalData* pSalData = GetSalData();

    // Store the new timeout
    pSalData->mnTimerMS = nMS;

    if ( !bMutex )
        pSalData->mnTimerOrgMS = nMS;

    // Cancel current timer
    if( pSalData->mbTimerInstalled )
        RemoveEventLoopTimer( pSalData->mrTimerRef );

    // Install the timer task
    if( InstallEventLoopTimer( GetMainEventLoop(),
                               kEventDurationMillisecond * nMS,
                               kEventDurationForever,
                               pSalData->mrTimerUPP,
                               NULL,
                               &pSalData->mrTimerRef) == noErr )
    {
        pSalData->mbTimerInstalled = TRUE;
    }
    else
    {
        AquaLog( "Could not install timer task!\n");
        pSalData->mbTimerInstalled = FALSE;
    }
}

void AquaSalTimerProc ( EventLoopTimerRef theTimer, void * /* userData */)
{
    AquaLog( "...AquaSalTimerProc...\n");

    SalData* pSalData = GetSalData();
     ImplSVData* pSVData = ImplGetSVData();
    AquaSalTimer *pSalTimer = (AquaSalTimer*) pSVData->mpSalTimer;

    if( pSalTimer && !pSalData->mbInTimerProc )
     {
        #ifdef USEMAINTHREAD
        // Send event to the main thread
        if( ! pSalData->maFrames.empty() )
            pSalData->maFrames.front()->PostTimerEvent( pSalTimer );

        // FIXME?
        // fire again using current timeout as this is a single shot timer
        ImplSalStartTimer( pSalData->mnTimerOrgMS, FALSE );
        #else
        // call back directly from timer thread
         if( ImplSalYieldMutexTryToAcquire() )
         {
            pSalData->mbInTimerProc = TRUE;
             pSalTimer->CallCallback();
            pSalData->mbInTimerProc = FALSE;
             ImplSalYieldMutexRelease();

             // fire again using current timeout as this is a single shot timer
            ImplSalStartTimer( pSalData->mnTimerOrgMS, FALSE );
         }
         else
         {
             // could not acquire solar mutex, so
             // fire again with a short delay (10ms)
            AquaLog( "SHOULD NOT HAPPEN! TIMER: solar mutex not free\n");
            ImplSalStartTimer( 10, TRUE );
         }
        #endif
    }
}


AquaSalTimer::AquaSalTimer( )
{
    AquaLog( "AquaSalTimer::AquaSalTimer\n");
    SalData* pSalData = GetSalData();

    pSalData->mbTimerInstalled = FALSE;
    pSalData->mnTimerMS = 0;
    pSalData->mnTimerOrgMS = 0;
    pSalData->mrTimerUPP = NewEventLoopTimerUPP( AquaSalTimerProc );
}

AquaSalTimer::~AquaSalTimer()
{
    AquaLog( "AquaSalTimer::~AquaSalTimer\n");

    SalData* pSalData = GetSalData();
    if( pSalData->mbTimerInstalled )
        RemoveEventLoopTimer( pSalData->mrTimerRef );

    DisposeEventLoopTimerUPP( pSalData->mrTimerUPP );
}

void AquaSalTimer::Start( ULONG nMS )
{
    ImplSalStartTimer(nMS, FALSE);
}

void AquaSalTimer::Stop()
{
    AquaLog( "AquaSalTimer::Stop\n");

    SalData* pSalData = GetSalData();
    if( pSalData->mbTimerInstalled )
     {
        RemoveEventLoopTimer( pSalData->mrTimerRef );
        pSalData->mbTimerInstalled = FALSE;
     }
}


