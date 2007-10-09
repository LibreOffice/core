/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saltimer.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:13:56 $
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
#include "saldata.hxx"
#include "salframe.h"
#include "salinst.h"

// =======================================================================

static NSTimer* pRunningTimer = nil;
static bool bDispatchTimer = false;
static bool bTimerInDispatch = false;

@interface TimerCallbackCaller : NSObject
{
}
-(void)timerElapsed:(NSTimer*)pTimer;
@end

@implementation TimerCallbackCaller
-(void)timerElapsed:(NSTimer*)pTimer
{
    if( bDispatchTimer && ! bTimerInDispatch )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if( pSVData->mpSalTimer )
        {
            YIELD_GUARD;
            bTimerInDispatch = true;
            pSVData->mpSalTimer->CallCallback();
            bTimerInDispatch = false;
        }
    }
}
@end

void ImplSalStartTimer( ULONG nMS )
{
    SalData* pSalData = GetSalData();
    if( pSalData->mpFirstInstance->isNSAppThread() )
    {
        bDispatchTimer = true;
        NSTimeInterval aTI = double(nMS)/1000.0;
        if( pRunningTimer != nil )
        {
            if( [pRunningTimer timeInterval] == aTI )
                // set new fire date
                [pRunningTimer setFireDate: [NSDate dateWithTimeIntervalSinceNow: aTI]];
            else
            {
                [pRunningTimer invalidate];
                pRunningTimer = nil;
            }
        }
        if( pRunningTimer == nil )
        {
            pRunningTimer = [NSTimer scheduledTimerWithTimeInterval: aTI
                                     target: [[[TimerCallbackCaller alloc] init] autorelease]
                                     selector: @selector(timerElapsed:)
                                     userInfo: nil
                                     repeats: YES];
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
    bDispatchTimer = false;
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


