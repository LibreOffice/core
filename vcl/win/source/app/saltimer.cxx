/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/svwin.h>
#ifdef __MINGW32__
#include <excpt.h>
#endif
#include <saldata.hxx>
#include <saltimer.h>
#include <salinst.h>

// =======================================================================

// Maximale Periode
#define MAX_SYSPERIOD     65533

// =======================================================================

void ImplSalStartTimer( sal_uLong nMS, sal_Bool bMutex )
{
    SalData* pSalData = GetSalData();

    // Remenber the time of the timer
    pSalData->mnTimerMS = nMS;
    if ( !bMutex )
        pSalData->mnTimerOrgMS = nMS;

    // Periode darf nicht zu gross sein, da Windows mit sal_uInt16 arbeitet
    if ( nMS > MAX_SYSPERIOD )
        nMS = MAX_SYSPERIOD;

    // Gibt es einen Timer, dann zerstoren
    if ( pSalData->mnTimerId )
        KillTimer( 0, pSalData->mnTimerId );

    // Make a new timer with new period
    pSalData->mnTimerId = SetTimer( 0, 0, (UINT)nMS, SalTimerProc );
    pSalData->mnNextTimerTime = pSalData->mnLastEventTime + nMS;
}

// -----------------------------------------------------------------------

WinSalTimer::~WinSalTimer()
{
}

void WinSalTimer::Start( sal_uLong nMS )
{
    // switch to main thread
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
            ImplPostMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
        else
            ImplSendMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE );
}

void WinSalTimer::Stop()
{
    SalData* pSalData = GetSalData();

    // If we have a timer, than
    if ( pSalData->mnTimerId )
    {
        KillTimer( 0, pSalData->mnTimerId );
        pSalData->mnTimerId = 0;
        pSalData->mnNextTimerTime = 0;
    }
}

// -----------------------------------------------------------------------

void CALLBACK SalTimerProc( HWND, UINT, UINT_PTR nId, DWORD )
{
#ifdef __MINGW32__
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
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
#ifdef __MINGW32__
    han.Reset();
#else
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
