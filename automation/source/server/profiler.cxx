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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"


#include <tools/time.hxx>
#include <tools/string.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif


#include "profiler.hxx"


TTProfiler::TTProfiler()
: mpStart( NULL )
, mpEnd( NULL )
, bIsProfileIntervalStarted( sal_False )
, bIsProfilingPerCommand( sal_False )
, bIsPartitioning( sal_False )
, bIsAutoProfiling( sal_False )
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


void TTProfiler::StartProfileInterval( sal_Bool bReadAnyway )
{
    if ( !bIsProfileIntervalStarted || bReadAnyway )
    {
        GetProfileSnapshot( mpStart );
        GetSysdepProfileSnapshot( mpStart->pSysdepProfileSnapshot, PROFILE_START );
        bIsProfileIntervalStarted = sal_True;
    }
}

String TTProfiler::GetProfileLine( ProfileSnapshot *pStart, ProfileSnapshot *pEnd )
{
    String aProfileString;

    aProfileString += Pad(GetpApp()->GetAppLocaleDataWrapper().getDuration( DIFF( pStart, pEnd, aTime) , sal_True, sal_True ), 12);

    sal_uLong nProcessTicks = DIFF( pStart, pEnd, nProcessTicks );
    sal_uLong nSystemTicks = DIFF( pStart, pEnd, nSystemTicks );
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
    bIsProfileIntervalStarted = sal_False;
}


void TTProfiler::GetProfileSnapshot( ProfileSnapshot *pProfileSnapshot )
{
    pProfileSnapshot->aTime = Time();
    pProfileSnapshot->nProcessTicks = Time::GetProcessTicks();
    pProfileSnapshot->nSystemTicks = Time::GetSystemTicks();
}


void TTProfiler::StartProfilingPerCommand()     // Jeden Befehl mitschneiden
{
    bIsProfilingPerCommand = sal_True;
}

void TTProfiler::StopProfilingPerCommand()
{
    bIsProfilingPerCommand = sal_False;
}

void TTProfiler::StartPartitioning()
{
    bIsPartitioning = sal_True;
}

void TTProfiler::StopPartitioning()
{
    bIsPartitioning = sal_True;
}

sal_uLong TTProfiler::GetPartitioningTime()
{
    return DIFF( mpStart, mpEnd, nSystemTicks );
}



void TTProfiler::StartAutoProfiling( sal_uLong nMSec )
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
        bIsAutoProfiling = sal_True;
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
        bIsAutoProfiling = sal_False;
    }
}



//String TTProfiler::Hex( sal_uLong nNr )
String TTProfiler::Dec( sal_uLong nNr )
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

String TTProfiler::Pad( const String aS, xub_StrLen nLen )
{
    if ( nLen > aS.Len() )
        return UniString().Fill( nLen - aS.Len() ).Append( aS );
    else
        return CUniString(" ").Append( aS );
}


