/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: profiler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:24:22 $
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


#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif


#include "profiler.hxx"


TTProfiler::TTProfiler()
: pStart( NULL )
, pEnd( NULL )
, bIsProfileIntervalStarted( FALSE )
, bIsProfilingPerCommand( FALSE )
, bIsPartitioning( FALSE )
, bIsAutoProfiling( FALSE )
, pSysDepStatic( NULL )
{
    InitSysdepProfiler();
    pStart = new ProfileSnapshot;
    pStart->pSysdepProfileSnapshot = NewSysdepSnapshotData();
    pEnd = new ProfileSnapshot;
    pEnd->pSysdepProfileSnapshot = NewSysdepSnapshotData();
    StartProfileInterval();
}

TTProfiler::~TTProfiler()
{
    if ( IsAutoProfiling() )
        StopAutoProfiling();
    if ( pStart )
    {
        if ( pStart->pSysdepProfileSnapshot )
            DeleteSysdepSnapshotData( pStart->pSysdepProfileSnapshot );
        delete pStart;
        pStart = NULL;
    }
    if ( pEnd )
    {
        if ( pEnd->pSysdepProfileSnapshot )
            DeleteSysdepSnapshotData( pEnd->pSysdepProfileSnapshot );
        delete pEnd;
        pEnd = NULL;
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
        GetProfileSnapshot( pStart );
        GetSysdepProfileSnapshot( pStart->pSysdepProfileSnapshot, PROFILE_START );
        bIsProfileIntervalStarted = TRUE;
    }
}

String TTProfiler::GetProfileLine( ProfileSnapshot *pStart, ProfileSnapshot *pEnd )
{
    String aProfileString;

    aProfileString += Pad(GetpApp()->GetAppInternational().GetDuration( DIFF( pStart, pEnd, aTime) , TRUE, TRUE ), 12);

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


        aProfileString += GetProfileLine( pStart, pEnd );
        aProfileString += GetSysdepProfileLine( pStart->pSysdepProfileSnapshot, pEnd->pSysdepProfileSnapshot );
        aProfileString += '\n';
    }

    return aProfileString;
}


void TTProfiler::EndProfileInterval()
{
    GetProfileSnapshot( pEnd );
    GetSysdepProfileSnapshot( pEnd->pSysdepProfileSnapshot, PROFILE_END );
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
    return DIFF( pStart, pEnd, nSystemTicks );
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

String TTProfiler::Pad( const String aS, xub_StrLen nLen )
{
    if ( nLen > aS.Len() )
        return UniString().Fill( nLen - aS.Len() ).Append( aS );
    else
        return CUniString(" ").Append( aS );
}


