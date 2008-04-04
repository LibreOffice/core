/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saltimer.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:57:45 $
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

#include "saltimer.h"
#include "salnstimer.h"
#include "saldata.hxx"
#include "salframe.h"
#include "salinst.h"

// =======================================================================

NSTimer* AquaSalTimer::pRunningTimer = nil;
bool AquaSalTimer::bDispatchTimer = false;


void ImplSalStartTimer( ULONG nMS )
{
    SalData* pSalData = GetSalData();
    if( pSalData->mpFirstInstance->isNSAppThread() )
    {
        AquaSalTimer::bDispatchTimer = true;
        NSTimeInterval aTI = double(nMS)/1000.0;
        if( AquaSalTimer::pRunningTimer != nil )
        {
            if( [AquaSalTimer::pRunningTimer timeInterval] == aTI )
                // set new fire date
                [AquaSalTimer::pRunningTimer setFireDate: [NSDate dateWithTimeIntervalSinceNow: aTI]];
            else
            {
                [AquaSalTimer::pRunningTimer invalidate];
                AquaSalTimer::pRunningTimer = nil;
            }
        }
        if( AquaSalTimer::pRunningTimer == nil )
        {
            AquaSalTimer::pRunningTimer = [NSTimer scheduledTimerWithTimeInterval: aTI
                                                   target: [[[TimerCallbackCaller alloc] init] autorelease]
                                                   selector: @selector(timerElapsed:)
                                                   userInfo: nil
                                                   repeats: YES];
            /* #i84055# add timer to tracking run loop mode,
               so they also elapse while e.g. life resize
            */
            [[NSRunLoop currentRunLoop] addTimer: AquaSalTimer::pRunningTimer forMode: NSEventTrackingRunLoopMode];
        }
    }
    else
    {
        // post an event so we can get into the main thread
        NSPoint aPt = { 0, 0 };
        NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
                                   location: aPt
                                   modifierFlags: 0
                                   timestamp: [NSDate timeIntervalSinceReferenceDate]
                                   windowNumber: 0
                                   context: nil
                                   subtype: AquaSalInstance::AppStartTimerEvent
                                   data1: (int)nMS
                                   data2: 0 ];
        if( pEvent )
            [NSApp postEvent: pEvent atStart: YES];
    }
}

void ImplSalStopTimer()
{
    AquaSalTimer::bDispatchTimer = false;
}

void AquaSalTimer::handleStartTimerEvent( NSEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpSalTimer )
    {
        NSTimeInterval posted = [pEvent timestamp] + NSTimeInterval([pEvent data1])/1000.0;
        NSTimeInterval current = [NSDate timeIntervalSinceReferenceDate];
        if( current - posted <= 0.0 )
            // timer already elapsed since event posted
            pSVData->mpSalTimer->CallCallback();
        else
            ImplSalStartTimer( ULONG( (posted - current) * 1000.0 ) );
    }

}

AquaSalTimer::AquaSalTimer( )
{
}

AquaSalTimer::~AquaSalTimer()
{
    ImplSalStopTimer();
}

void AquaSalTimer::Start( ULONG nMS )
{
    ImplSalStartTimer( nMS );
}

void AquaSalTimer::Stop()
{
    ImplSalStopTimer();
}


