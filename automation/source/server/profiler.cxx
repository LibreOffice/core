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
#include "precompiled_automation.hxx"


#include <tools/time.hxx>
#include <tools/string.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>
#include <basic/ttstrhlp.hxx>


#include "profiler.hxx"


TTProfiler::TTProfiler()
: mpStart( NULL )
, mpEnd( NULL )
, bIsProfileIntervalStarted( FALSE )
, bIsProfilingPerCommand( FALSE )
, bIsPartitioning( FALSE )
, bIsAutoProfiling( FALSE )
, pSysDepStatic( NULL )
{
    InitSysdepProfiler();
    mpStart = new ProfileSnapshot;
    mpStart->pSysdepProfileSnapshot = NewSysdepSnapshotData();
    mpEnd = new ProfileSnapshot;
    mpEnd->pSysdepProfileSnapshot = NewSysdepSnapshotData();
    StartProfileInterval();
}

TTProfiler::~TTProfiler()
{
    if ( IsAutoProfiling() )
        StopAutoProfiling();
    if ( mpStart )
    {
        if ( mpStart->pSysdepProfileSnapshot )
            DeleteSysdepSnapshotData( mpStart->pSysdepProfileSnapshot );
        delete mpStart;
        mpStart = NULL;
    }
    if ( mpEnd )
    {
        if ( mpEnd->pSysdepProfileSnapshot )
            DeleteSysdepSnapshotData( mpEnd->pSysdepProfileSnapshot );
        delete mpEnd;
        mpEnd = NULL;
    }
    DeinitSysdepProfiler();
}


String TTProfiler::GetProfileHeader()
{
    UniString aReturn;
    aReturn += '\n';
    if ( !IsAutoProfiling() )
        aReturn.AppendAscii("Befehl").Append(TabString(36));

    aReturn.AppendAscii("   Zeitdauer");
    aReturn.AppendAscii("  Ticks in %");
    aReturn.Append( GetSysdepProfileHeader() );
    aReturn.AppendAscii("\n");
    return aReturn;
}


void TTProfiler::StartProfileInterval( BOOL bReadAnyway )
{
    if ( !bIsProfileIntervalStarted || bReadAnyway )
    {
        GetProfileSnapshot( mpStart );
        GetSysdepProfileSnapshot( mpStart->pSysdepProfileSnapshot, PROFILE_START );
        bIsProfileIntervalStarted = TRUE;
    }
}

String TTProfiler::GetProfileLine( ProfileSnapshot *pStart, ProfileSnapshot *pEnd )
{
    String aProfileString;

    aProfileString += Pad(GetpApp()->GetAppLocaleDataWrapper().getDuration( DIFF( pStart, pEnd, aTime) , TRUE, TRUE ), 12);

    ULONG nProcessTicks = DIFF( pStart, pEnd, nProcessTicks );
    ULONG nSystemTicks = DIFF( pStart, pEnd, nSystemTicks );
    if ( nSystemTicks )
    {
        aProfileString += Pad(UniString::CreateFromInt32( (100 * nProcessTicks) / nSystemTicks ), 11);
        aProfileString += '%';
    }
    else
        aProfileString += Pad(CUniString("??  "), 12);

    return aProfileString;
}


String TTProfiler::GetProfileLine( String &aPrefix )
{
    String aProfileString;
    if ( IsProfilingPerCommand() || IsAutoProfiling() )
    {
        aProfileString = aPrefix;
        aProfileString += TabString(35);


        aProfileString += GetProfileLine( mpStart, mpEnd );
        aProfileString += GetSysdepProfileLine( mpStart->pSysdepProfileSnapshot, mpEnd->pSysdepProfileSnapshot );
        aProfileString += '\n';
    }

    return aProfileString;
}


void TTProfiler::EndProfileInterval()
{
    GetProfileSnapshot( mpEnd );
    GetSysdepProfileSnapshot( mpEnd->pSysdepProfileSnapshot, PROFILE_END );
    bIsProfileIntervalStarted = FALSE;
}


void TTProfiler::GetProfileSnapshot( ProfileSnapshot *pProfileSnapshot )
{
    pProfileSnapshot->aTime = Time();
    pProfileSnapshot->nProcessTicks = Time::GetProcessTicks();
    pProfileSnapshot->nSystemTicks = Time::GetSystemTicks();
}


void TTProfiler::StartProfilingPerCommand()     // Jeden Befehl mitschneiden
{
    bIsProfilingPerCommand = TRUE;
}

void TTProfiler::StopProfilingPerCommand()
{
    bIsProfilingPerCommand = FALSE;
}

void TTProfiler::StartPartitioning()
{
    bIsPartitioning = TRUE;
}

void TTProfiler::StopPartitioning()
{
    bIsPartitioning = TRUE;
}

ULONG TTProfiler::GetPartitioningTime()
{
    return DIFF( mpStart, mpEnd, nSystemTicks );
}



void TTProfiler::StartAutoProfiling( ULONG nMSec )
{
    if ( !bIsAutoProfiling )
    {
        pAutoStart = new ProfileSnapshot;
        pAutoStart->pSysdepProfileSnapshot = NewSysdepSnapshotData();
        pAutoEnd = new ProfileSnapshot;
        pAutoEnd->pSysdepProfileSnapshot = NewSysdepSnapshotData();
        GetProfileSnapshot( pAutoStart );
        GetSysdepProfileSnapshot( pAutoStart->pSysdepProfileSnapshot, PROFILE_START );
        SetTimeout( nMSec );
        bIsAutoProfiling = TRUE;
        Start();
    }

}

void TTProfiler::Timeout()
{
    GetProfileSnapshot( pAutoEnd );
    GetSysdepProfileSnapshot( pAutoEnd->pSysdepProfileSnapshot, PROFILE_END );
    String aLine;

    aLine += GetProfileLine( pAutoStart, pAutoEnd );
    aLine += GetSysdepProfileLine( pAutoStart->pSysdepProfileSnapshot, pAutoEnd->pSysdepProfileSnapshot );
    aLine += '\n';

    aAutoProfileBuffer += aLine;

    ProfileSnapshot *pTemp = pAutoStart;        // Tauschen, so daﬂ jetziges Ende n‰chsten Start wird
    pAutoStart = pAutoEnd;
    pAutoEnd = pTemp;

    Start();    // Timer neu starten
}

String TTProfiler::GetAutoProfiling()
{
    String aTemp(aAutoProfileBuffer);
    aAutoProfileBuffer.Erase();
    return aTemp;
}

void TTProfiler::StopAutoProfiling()
{
    if ( bIsAutoProfiling )
    {
        Stop();
        bIsAutoProfiling = FALSE;
    }
}



//String TTProfiler::Hex( ULONG nNr )
String TTProfiler::Dec( ULONG nNr )
{
    String aRet(UniString::CreateFromInt32(nNr));
    if ( nNr < 100 )
    {
        aRet = Pad( aRet, 3);
        aRet.SearchAndReplaceAll(' ','0');
    }
    aRet.Insert( ',', aRet.Len() - 2 );
    return aRet;
}

String TTProfiler::Pad( const String &aS, xub_StrLen nLen )
{
    if ( nLen > aS.Len() )
        return UniString().Fill( nLen - aS.Len() ).Append( aS );
    else
        return CUniString(" ").Append( aS );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
