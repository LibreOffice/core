/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refreshtimer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:50:03 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "refreshtimer.hxx"


ScRefreshTimerProtector::ScRefreshTimerProtector( ScRefreshTimerControl * const * pp )
        :
        ppControl( pp )
{
    if ( ppControl && *ppControl )
    {
        (*ppControl)->SetAllowRefresh( FALSE );
        // wait for any running refresh in another thread to finnish
        ::vos::OGuard aGuard( (*ppControl)->GetMutex() );
    }
}


ScRefreshTimer::~ScRefreshTimer()
{
    if ( IsActive() )
        Stop();
    RemoveFromControl();
}


void ScRefreshTimer::SetRefreshDelay( ULONG nSeconds )
{
    BOOL bActive = IsActive();
    if ( bActive && !nSeconds )
        Stop();
    SetTimeout( nSeconds * 1000 );
    if ( !bActive && nSeconds )
        Start();
}


void ScRefreshTimer::Timeout()
{
    if ( ppControl && *ppControl && (*ppControl)->IsRefreshAllowed() )
    {
        // now we COULD make the call in another thread ...
        ::vos::OGuard aGuard( (*ppControl)->GetMutex() );
        maTimeoutHdl.Call( this );
        // restart from now on, don't execute immediately again if timed out
        // a second time during refresh
        if ( IsActive() )
            Start();
    }
}

