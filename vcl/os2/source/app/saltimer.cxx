/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <svpm.h>

#define _SV_SALTIMER_CXX
#include <os2/saldata.hxx>
#include <os2/saltimer.h>
#include <os2/salinst.h>

// =======================================================================

// Maximale Periode
#define MAX_SYSPERIOD     65533
#define IDTIMER             10

// =======================================================================

void ImplSalStartTimer( ULONG nMS, sal_Bool bMutex )
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
        // Try to acquire the mutex. If we don't get the mutex then we
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

