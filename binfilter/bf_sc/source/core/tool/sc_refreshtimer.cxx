/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "refreshtimer.hxx"
#include <tools/debug.hxx>//STRIP001 
namespace binfilter {

/*N*/ ScRefreshTimerProtector::ScRefreshTimerProtector( ScRefreshTimerControl * const * pp )
/*N*/ 		:
/*N*/ 		ppControl( pp )
/*N*/ {
/*N*/ 	if ( ppControl && *ppControl )
/*N*/ 	{
/*N*/ 		(*ppControl)->SetAllowRefresh( FALSE );
/*N*/ 		// wait for any running refresh in another thread to finnish
/*N*/ 		::vos::OGuard aGuard( (*ppControl)->GetMutex() );
/*N*/ 	}
/*N*/ }


/*N*/ ScRefreshTimer::~ScRefreshTimer()
/*N*/ {
/*N*/ 	if ( IsActive() )
/*?*/ 		Stop();
/*N*/ 	RemoveFromControl();
/*N*/ }


/*N*/ void ScRefreshTimer::SetRefreshDelay( ULONG nSeconds )
/*N*/ {
/*N*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 BOOL bActive = IsActive();
/*N*/ }



}
