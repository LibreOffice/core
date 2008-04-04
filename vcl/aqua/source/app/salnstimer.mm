/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salnstimer.mm,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:57:32 $
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
    ImplSVData* pSVData = ImplGetSVData();
    if( AquaSalTimer::bDispatchTimer )
    {
        if( pSVData->mpSalTimer )
        {
            YIELD_GUARD;
            pSVData->mpSalTimer->CallCallback();
            
            // NSTimer does not end nextEventMatchingMask of NSApplication
            // so we need to wakeup a waiting Yield to inform it something happened
            GetSalData()->mpFirstInstance->wakeupYield();
        }
    }
}
@end

