/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saltimer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 10:05:49 $
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

#include <svpm.h>

#define _SV_SALTIMER_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALTIMER_H
#include <saltimer.h>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif

// =======================================================================

// Maximale Periode
#define MAX_SYSPERIOD     65533
#define IDTIMER             10

// =======================================================================

void ImplSalStartTimer( ULONG nMS, BOOL bMutex )
{
    SalData*    pSalData = GetSalData();

    // Periode darf nicht zu gross sein, da OS2 2.11 mit USHORT arbeitet
    // Remenber the time of the timer
    pSalData->mnTimerMS = nMS;
    if ( !bMutex )
        pSalData->mnTimerOrgMS = nMS;

    // Periode darf nicht zu gross sein, da Windows mit USHORT arbeitet
    if ( nMS > MAX_SYSPERIOD )
        nMS = MAX_SYSPERIOD;

    // Gibt es einen Timer, dann zerstoren
    if ( pSalData->mnTimerId )
        WinStopTimer( pSalData->mhAB, pSalData->mpFirstInstance->mhComWnd, pSalData->mnTimerId );

    // Make a new timer with new period
    pSalData->mnTimerId = WinStartTimer( pSalData->mhAB, pSalData->mpFirstInstance->mhComWnd, IDTIMER, nMS );
    pSalData->mnNextTimerTime = pSalData->mnLastEventTime + nMS;
}

// -----------------------------------------------------------------------

Os2SalTimer::~Os2SalTimer()
{
}

// -----------------------------------------------------------------------

void Os2SalTimer::Start( ULONG nMS )
{
    // Um auf Main-Thread umzuschalten
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
            WinPostMsg( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (MPARAM)nMS );
        else
            WinSendMsg( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (MPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE);
}

// -----------------------------------------------------------------------

void Os2SalTimer::Stop()
{
    SalData*    pSalData = GetSalData();

    // Exitstiert ein Timer, dann diesen zerstoeren
    if ( pSalData->mnTimerId ) {
        WinStopTimer( pSalData->mhAB, pSalData->mpFirstInstance->mhComWnd, pSalData->mnTimerId );
        pSalData->mnTimerId = 0;
        pSalData->mnNextTimerTime = 0;
    }
}

// -----------------------------------------------------------------------

void SalTimerProc( HWND, UINT, UINT nId, ULONG )
{
    SalData* pSalData = GetSalData();
    ImplSVData* pSVData = ImplGetSVData();

    // Test for MouseLeave
    SalTestMouseLeave();

    bool bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
    if ( pSVData->mpSalTimer && ! bRecursive )
    {
        // Try to aquire the mutex. If we don't get the mutex then we
        // try this a short time later again.
        if ( ImplSalYieldMutexTryToAcquire() )
        {
            bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
            if ( pSVData->mpSalTimer && ! bRecursive )
            {
                pSalData->mbInTimerProc = TRUE;
                pSVData->mpSalTimer->CallCallback();
                pSalData->mbInTimerProc = FALSE;
                ImplSalYieldMutexRelease();

                // Run the timer in the correct time, if we start this
                // with a small timeout, because we don't get the mutex
                if ( pSalData->mnTimerId &&
                    (pSalData->mnTimerMS != pSalData->mnTimerOrgMS) )
                    ImplSalStartTimer( pSalData->mnTimerOrgMS, FALSE );
            }
        }
        else
            ImplSalStartTimer( 10, TRUE );
    }

}

