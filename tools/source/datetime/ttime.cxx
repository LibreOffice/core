/*************************************************************************
 *
 *  $RCSfile: ttime.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:06 $
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

#define _TOOLS_TIME_CXX

#if defined( OS2 )
#define INCL_DOSMISC
#include <svpm.h>
#elif defined( WNT )
#include <svwin.h>
#elif defined( WIN )
#include <svwin.h>
#include <dos.h>
#elif defined( DOS )
#include <dos.h>
#elif defined UNX
#include <limits.h>
#ifdef IRIX
#include <unistd.h>
#endif
#include <sys/times.h>
#elif defined( MAC )
#include "mac_start.h"
#ifndef __OSUTILS__
#include "OSUtils.h"
#include <MAC_Timer.h>
#include "mac_end.h"
#endif
#endif

#include <time.h>
#include <time.hxx>

#ifdef UNX
#include <math.h>
#endif

#ifndef WNT
#ifndef localtime_r
extern "C" {
struct tm *localtime_r(const time_t *timep, struct tm *buffer);
}
#endif

#ifndef gmtime_r
extern "C" {
struct tm *gmtime_r(const time_t *timep, struct tm *buffer);
}
#endif
#endif

// =======================================================================

static long TimeToSec100( const Time& rTime )
{
    short  nSign   = (rTime.GetTime() >= 0) ? +1 : -1;
    long   nHour   = rTime.GetHour();
    long   nMin    = rTime.GetMin();
    long   nSec    = rTime.GetSec();
    long   n100Sec = rTime.Get100Sec();

//  Wegen Interal Compiler Error bei MSC, etwas komplizierter
//  return (n100Sec + (nSec*100) + (nMin*60*100) + (nHour*60*60*100) * nSign);

    long nRet = n100Sec;
    nRet     += nSec*100;
    nRet     += nMin*60*100;
    nRet     += nHour*60*60*100;

    return (nRet * nSign);
}

// -----------------------------------------------------------------------

static Time Sec100ToTime( long nSec100 )
{
    short nSign;
    if ( nSec100 < 0 )
    {
        nSec100 *= -1;
        nSign = -1;
    }
    else
        nSign = 1;

    Time aTime( 0, 0, 0, nSec100 );
    aTime.SetTime( aTime.GetTime() * nSign );
    return aTime;
}

// =======================================================================

Time::Time()
{
#if defined( OS2 )
    DATETIME aDateTime;
    DosGetDateTime( &aDateTime );

    // Zeit zusammenbauen
    nTime = (((long)aDateTime.hours)*1000000) +
            (((long)aDateTime.minutes)*10000) +
            (((long)aDateTime.seconds)*100) +
            ((long)aDateTime.hundredths);
#elif defined( WNT )
    SYSTEMTIME aDateTime;
    GetLocalTime( &aDateTime );

    // Zeit zusammenbauen
    nTime = (((long)aDateTime.wHour)*1000000) +
            (((long)aDateTime.wMinute)*10000) +
            (((long)aDateTime.wSecond)*100) +
            ((long)aDateTime.wMilliseconds/10);
#elif ( defined( WIN ) || defined( DOS ) ) && !defined ( BLC )
    _dostime_t aTime;
    _dos_gettime( &aTime );

    // Zeit zusammenbauen
    nTime = (((long)aTime.hour)*1000000) +
            (((long)aTime.minute)*10000) +
            (((long)aTime.second)*100) +
            ((long)aTime.hsecond);
#elif ( defined( WIN ) || defined( DOS ) ) && defined ( BLC )
    dostime_t aTime;
    _dos_gettime( &aTime );

    // Zeit zusammenbauen
    nTime = (((long)aTime.hour)*1000000) +
            (((long)aTime.minute)*10000) +
            (((long)aTime.second)*100) +
            ((long)aTime.hsecond);
#elif defined( MAC )
    DateTimeRec dt;
    ::GetTime(&dt);
    nTime = (((long)dt.hour)*1000000) +
            (((long)dt.minute)*10000) +
            (((long)dt.second)*100);
#else
    time_t     nTmpTime;
    struct tm aTime;

    // Zeit ermitteln
    nTmpTime = time( 0 );

    // Zeit zusammenbauen
    if ( localtime_r( &nTmpTime, &aTime ) )
    {
        nTime = (((long)aTime.tm_hour)*1000000) +
                (((long)aTime.tm_min)*10000) +
                (((long)aTime.tm_sec)*100);
    }
    else
        nTime = 0;
#endif
}

// -----------------------------------------------------------------------

Time::Time( ULONG nHour, ULONG nMin, ULONG nSec, ULONG n100Sec )
{
    // Zeit normalisieren
    nSec    += n100Sec / 100;
    n100Sec  = n100Sec % 100;
    nMin    += nSec / 60;
    nSec     = nSec % 60;
    nHour   += nMin / 60;
    nMin     = nMin % 60;

    // Zeit zusammenbauen
    nTime = (long)(n100Sec + (nSec*100) + (nMin*10000) + (nHour*1000000));
}

// -----------------------------------------------------------------------

void Time::SetHour( USHORT nNewHour )
{
    short  nSign      = (nTime >= 0) ? +1 : -1;
    long   nMin       = GetMin();
    long   nSec       = GetSec();
    long   n100Sec    = Get100Sec();

    nTime = (n100Sec + (nSec*100) + (nMin*10000) +
            (((long)nNewHour)*1000000)) * nSign;
}

// -----------------------------------------------------------------------

void Time::SetMin( USHORT nNewMin )
{
    short  nSign      = (nTime >= 0) ? +1 : -1;
    long   nHour      = GetHour();
    long   nSec       = GetSec();
    long   n100Sec    = Get100Sec();

    // kein Ueberlauf
    nNewMin = nNewMin % 60;

    nTime = (n100Sec + (nSec*100) + (((long)nNewMin)*10000) +
            (nHour*1000000)) * nSign;
}

// -----------------------------------------------------------------------

void Time::SetSec( USHORT nNewSec )
{
    short  nSign      = (nTime >= 0) ? +1 : -1;
    long   nHour      = GetHour();
    long   nMin       = GetMin();
    long   n100Sec    = Get100Sec();

    // kein Ueberlauf
    nNewSec = nNewSec % 60;

    nTime = (n100Sec + (((long)nNewSec)*100) + (nMin*10000) +
            (nHour*1000000)) * nSign;
}

// -----------------------------------------------------------------------

void Time::Set100Sec( USHORT nNew100Sec )
{
    short  nSign      = (nTime >= 0) ? +1 : -1;
    long   nHour      = GetHour();
    long   nMin       = GetMin();
    long   nSec       = GetSec();

    // kein Ueberlauf
    nNew100Sec = nNew100Sec % 100;

    nTime = (((long)nNew100Sec) + (nSec*100) + (nMin*10000) +
            (nHour*1000000)) * nSign;
}

// -----------------------------------------------------------------------

long Time::GetMSFromTime() const
{
    short  nSign      = (nTime >= 0) ? +1 : -1;
    long   nHour      = GetHour();
    long   nMin       = GetMin();
    long   nSec       = GetSec();
    long   n100Sec    = Get100Sec();

    return (((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10))*nSign);
}

// -----------------------------------------------------------------------

void Time::MakeTimeFromMS( long nMS )
{
    short nSign;
    if ( nMS < 0 )
    {
        nMS *= -1;
        nSign = -1;
    }
    else
        nSign = 1;

    Time aTime( 0, 0, 0, nMS/10 );
    SetTime( aTime.GetTime() * nSign );
}

// -----------------------------------------------------------------------

Time& Time::operator +=( const Time& rTime )
{
    nTime = Sec100ToTime( TimeToSec100( *this ) +
                          TimeToSec100( rTime ) ).GetTime();
    return *this;
}

// -----------------------------------------------------------------------

Time& Time::operator -=( const Time& rTime )
{
    nTime = Sec100ToTime( TimeToSec100( *this ) -
                          TimeToSec100( rTime ) ).GetTime();
    return *this;
}

// -----------------------------------------------------------------------

Time operator +( const Time& rTime1, const Time& rTime2 )
{
    return Sec100ToTime( TimeToSec100( rTime1 ) +
                         TimeToSec100( rTime2 ) );
}

// -----------------------------------------------------------------------

Time operator -( const Time& rTime1, const Time& rTime2 )
{
    return Sec100ToTime( TimeToSec100( rTime1 ) -
                         TimeToSec100( rTime2 ) );
}

// -----------------------------------------------------------------------

Time Time::GetUTCOffset()
{
#if defined( OS2 )
#undef timezone
    DATETIME aDateTime;
    DosGetDateTime( &aDateTime );

    // Zeit zusammenbauen
    if ( aDateTime.timezone != -1  )
    {
        short nTempTime = (short)Abs( aDateTime.timezone );
        Time aTime( 0, (USHORT)nTempTime );
        if ( aDateTime.timezone > 0 )
            aTime = -aTime;
        return aTime;
    }
    else
        return Time( 0 );
#elif defined( WNT )
    TIME_ZONE_INFORMATION   aTimeZone;
    aTimeZone.Bias = 0;
    DWORD nTimeZoneRet = GetTimeZoneInformation( &aTimeZone );
    long nTempTime = aTimeZone.Bias;
    if ( nTimeZoneRet == TIME_ZONE_ID_STANDARD )
        nTempTime += aTimeZone.StandardBias;
    else if ( nTimeZoneRet == TIME_ZONE_ID_DAYLIGHT )
        nTempTime += aTimeZone.DaylightBias;
    Time aTime( 0, (USHORT)Abs( nTempTime ) );
    if ( nTempTime > 0 )
        aTime = -aTime;
    return aTime;
#elif ( defined( WIN ) || defined( DOS ) ) && defined ( BLC )
    static ULONG    nCacheTicks = 0;
    static long     nCacheSecOffset = -1;
    ULONG           nTicks = Time::GetSystemTicks();
    time_t          nTime;
    tm              aTM;
    long            nLocalTime;
    long            nUTC;
    short           nTempTime;

    // Evt. Wert neu ermitteln
    if ( (nCacheSecOffset == -1) || ((nTicks - nCacheTicks) > 360000) )
    {
        nTime = time( 0 );
        tm aTMTmp;
        aTM = *localtime_r( &nTime, &aTMTmp);
        nLocalTime = mktime( &aTM );
        aTM = *gmtime_r( &nTime, &aTMTmp);
        nUTC = mktime( &aTM );
        nCacheTicks = nTicks;
        nCacheSecOffset = (nLocalTime-nUTC) / 60;
    }

    nTempTime = (short)Abs( nCacheSecOffset );
    Time aTime( 0, (USHORT)nTempTime );
    if ( nCacheSecOffset < 0 )
        aTime = -aTime;
    return aTime;
#else
    static ULONG    nCacheTicks = 0;
    static long     nCacheSecOffset = -1;
    ULONG           nTicks = Time::GetSystemTicks();
    time_t          nTime;
    tm              aTM;
    long            nLocalTime;
    long            nUTC;
    short           nTempTime;

    // Evt. Wert neu ermitteln
    if ( (nCacheSecOffset == -1)            ||
         ((nTicks - nCacheTicks) > 360000)  ||
         ( nTicks < nCacheTicks ) // handle overflow
         )
    {
        nTime = time( 0 );
        localtime_r( &nTime, &aTM );
        nLocalTime = mktime( &aTM );
#if defined( SOLARIS )
        // Solaris gmtime_r() seems not to handle daylight saving time
        // flags correctly
        nUTC = nLocalTime + ( aTM.tm_isdst == 0 ? timezone : altzone );
#elif defined( LINUX )
        // Linux mktime() seems not to handle tm_isdst correctly
        nUTC = nLocalTime - aTM.tm_gmtoff;
#else
         gmtime_r( &nTime, &aTM );
         nUTC = mktime( &aTM );
#endif
         nCacheTicks = nTicks;
         nCacheSecOffset = (nLocalTime-nUTC) / 60;
    }

    nTempTime = (short)Abs( nCacheSecOffset );
    Time aTime( 0, (USHORT)nTempTime );
    if ( nCacheSecOffset < 0 )
        aTime = -aTime;
    return aTime;
#endif
}


// -----------------------------------------------------------------------

ULONG Time::GetSystemTicks()
{
#if defined( WIN ) || defined( WNT )
    return (ULONG)GetTickCount();
#elif defined( OS2 )
    PM_ULONG nClock;
    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &nClock, sizeof( nClock ) );
    return (ULONG)nClock;
#elif defined( MAC )
    long long millisec;
    Microseconds((UnsignedWide *)&millisec);
    millisec = ( millisec + 500L ) / 1000L;
    return (ULONG)millisec;
#else
    static ULONG    nImplTicksPerSecond = 0;
    static double   dImplTicksPerSecond;
    static double   dImplTicksULONGMAX;
    struct tms      aTms;
    ULONG           nTicks = (ULONG)times( &aTms );

    if ( !nImplTicksPerSecond )
    {
        nImplTicksPerSecond = CLK_TCK;
        dImplTicksPerSecond = nImplTicksPerSecond;
        dImplTicksULONGMAX  = (double)(ULONG)ULONG_MAX;
    }

    double fTicks = nTicks;
    fTicks *= 1000;
    fTicks /= dImplTicksPerSecond;
    fTicks = fmod (fTicks, dImplTicksULONGMAX);

    return (ULONG)fTicks;
#endif
}

// -----------------------------------------------------------------------

ULONG Time::GetProcessTicks()
{
#if defined( WIN ) || defined( WNT )
    return (ULONG)GetTickCount();
#elif defined( OS2 )
    PM_ULONG nClock;
    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &nClock, sizeof( nClock ) );
    return (ULONG)nClock;
#elif defined( MAC )
    long long millisec;
    Microseconds((UnsignedWide *)&millisec);
    millisec = ( millisec + 500L ) / 1000L;
    return (ULONG)millisec;
#else
    static ULONG    nImplTicksPerSecond = 0;
    static double   dImplTicksPerSecond;
    static double   dImplTicksULONGMAX;
    ULONG           nTicks = (ULONG)clock();

    if ( !nImplTicksPerSecond )
    {
        nImplTicksPerSecond = CLOCKS_PER_SEC;
        dImplTicksPerSecond = nImplTicksPerSecond;
        dImplTicksULONGMAX  = (double)(ULONG)ULONG_MAX;
    }

    double fTicks = nTicks;
    fTicks *= 1000;
    fTicks /= dImplTicksPerSecond;
    fTicks = fmod (fTicks, dImplTicksULONGMAX);
    return (ULONG)fTicks;
#endif
}
