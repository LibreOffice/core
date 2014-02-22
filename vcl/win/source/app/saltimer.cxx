/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svsys.h>
#include <win/saldata.hxx>
#include <win/saltimer.h>
#include <win/salinst.h>

#if defined ( __MINGW32__ )
#include <sehandler.hxx>
#endif





#define MAX_SYSPERIOD     65533



void ImplSalStartTimer( sal_uLong nMS, sal_Bool bMutex )
{
    SalData* pSalData = GetSalData();

    
    pSalData->mnTimerMS = nMS;
    if ( !bMutex )
        pSalData->mnTimerOrgMS = nMS;

    
    if ( nMS > MAX_SYSPERIOD )
        nMS = MAX_SYSPERIOD;

    
    if ( pSalData->mnTimerId )
        KillTimer( 0, pSalData->mnTimerId );

    
    pSalData->mnTimerId = SetTimer( 0, 0, (UINT)nMS, SalTimerProc );
    pSalData->mnNextTimerTime = pSalData->mnLastEventTime + nMS;
}



WinSalTimer::~WinSalTimer()
{
}

void WinSalTimer::Start( sal_uLong nMS )
{
    
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

    
    if ( pSalData->mnTimerId )
    {
        KillTimer( 0, pSalData->mnTimerId );
        pSalData->mnTimerId = 0;
        pSalData->mnNextTimerTime = 0;
    }
}



void CALLBACK SalTimerProc( HWND, UINT, UINT_PTR nId, DWORD )
{
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
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

        
        SalTestMouseLeave();

        bool bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
        if ( pSVData->mpSalTimer && ! bRecursive )
        {
            
            
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
                if ( pSVData->mpSalTimer && ! bRecursive )
                {
                    pSalData->mbInTimerProc = TRUE;
                    pSVData->mpSalTimer->CallCallback();
                    pSalData->mbInTimerProc = FALSE;
                    ImplSalYieldMutexRelease();

                    
                    
                    if ( pSalData->mnTimerId &&
                        (pSalData->mnTimerMS != pSalData->mnTimerOrgMS) )
                        ImplSalStartTimer( pSalData->mnTimerOrgMS, FALSE );
                }
            }
            else
                ImplSalStartTimer( 10, TRUE );
        }
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#else
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
