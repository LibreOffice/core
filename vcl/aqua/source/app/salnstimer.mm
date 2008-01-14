/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salnstimer.mm,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 16:15:46 $
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
#include "salinst.h"
#include "saldata.hxx"

#include "vcl/svdata.hxx"

@implementation TimerCallbackCaller
-(void)timerElapsed:(NSTimer*)pTimer
{
    if( AquaSalTimer::bDispatchTimer && ! AquaSalTimer::bTimerInDispatch )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if( pSVData->mpSalTimer )
        {
            YIELD_GUARD;
            AquaSalTimer::bTimerInDispatch = true;
            pSVData->mpSalTimer->CallCallback();
            AquaSalTimer::bTimerInDispatch = false;
            
            // NSTimer does not end nextEventMatchingMask of NSApplication
            // so we need to wakeup a waiting Yield to inform it something happened
            GetSalData()->mpFirstInstance->wakeupYield();
        }
    }
}
@end

