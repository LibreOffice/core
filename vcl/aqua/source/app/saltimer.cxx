/*************************************************************************
 *
 *  $RCSfile: saltimer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-01 03:12:44 $
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

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALTIMER_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
#endif

// =======================================================================

// Maximale Periode
#define MAX_SYSPERIOD     65533

// =======================================================================

void ImplSalStartTimer( ULONG nMS, BOOL bMutex )
{
    SalData* pSalData = GetSalData();

    // Remenber the time of the timer
    pSalData->mnTimerMS = nMS;
    if ( !bMutex )
        pSalData->mnTimerOrgMS = nMS;

    // Periode darf nicht zu gross sein, da Windows mit USHORT arbeitet
    if ( nMS > MAX_SYSPERIOD )
        nMS = MAX_SYSPERIOD;

#ifdef WIN
    // Gibt es einen Timer, dann zerstoren
    if ( pSalData->mnTimerId )
        KillTimer( 0, pSalData->mnTimerId );

    // Make a new timer with new period
    pSalData->mnTimerId = SetTimer( 0, 0, (UINT)nMS, SalTimerProc );
#endif
}

// -----------------------------------------------------------------------

void SalTimer::Stop()
{
    SalData* pSalData = GetSalData();

#ifdef WIN
    // If we have a timer, than
    if ( pSalData->mnTimerId )
    {
        KillTimer( 0, pSalData->mnTimerId );
        pSalData->mnTimerId = 0;
    }
#endif
}

// -----------------------------------------------------------------------

void SalTimer::SetCallback( SALTIMERPROC pProc )
{
    SalData* pSalData = GetSalData();
    pSalData->mpTimerProc = pProc;
}

// -----------------------------------------------------------------------

void CALLBACK SalTimerProc( VCLWINDOW, UINT, UINT, DWORD )
{
    SalData* pSalData = GetSalData();

    // Test for MouseLeave
    SalTestMouseLeave();

    if ( pSalData->mpTimerProc )
    {
        // Try to aquire the mutex. If we don't get the mutex then we
        // try this a short time later again.
        if ( ImplSalYieldMutexTryToAcquire() )
        {
            pSalData->mpTimerProc();
            ImplSalYieldMutexRelease();

            // Run the timer in the correct time, if we start this
            // with a small timeout, because we don't get the mutex
            if ( pSalData->mnTimerId &&
                 (pSalData->mnTimerMS != pSalData->mnTimerOrgMS) )
                ImplSalStartTimer( pSalData->mnTimerOrgMS, FALSE );
        }
        else
            ImplSalStartTimer( 10, TRUE );
    }
}
