/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: time.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:19:52 $
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
#ifndef _TOOLS_TIME_HXX
#define _TOOLS_TIME_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class ResId;

// --------
// - Time -
// --------

class TOOLS_DLLPUBLIC Time
{
private:
    sal_Int32           nTime;

public:
                    Time();
                    Time( const ResId & rResId );
                    Time( sal_Int32 _nTime ) { Time::nTime = _nTime; }
                    Time( const Time& rTime );
                    Time( ULONG nHour, ULONG nMin,
                          ULONG nSec = 0, ULONG n100Sec = 0 );

    void            SetTime( sal_Int32 nNewTime ) { nTime = nNewTime; }
    sal_Int32       GetTime() const { return nTime; }

    void            SetHour( USHORT nNewHour );
    void            SetMin( USHORT nNewMin );
    void            SetSec( USHORT nNewSec );
    void            Set100Sec( USHORT nNew100Sec );
    USHORT          GetHour() const
                        { ULONG nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (USHORT)(nTempTime / 1000000); }
    USHORT          GetMin() const
                        { ULONG nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (USHORT)((nTempTime / 10000) % 100); }
    USHORT          GetSec() const
                        { ULONG nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (USHORT)((nTempTime / 100) % 100); }
    USHORT          Get100Sec() const
                        { ULONG nTempTime = (nTime >= 0) ? nTime : nTime*-1;
                          return (USHORT)(nTempTime % 100); }

    sal_Int32       GetMSFromTime() const;
    void            MakeTimeFromMS( sal_Int32 nMS );

                    /// 12 hours == 0.5 days
    double          GetTimeInDays() const;

    BOOL            IsBetween( const Time& rFrom, const Time& rTo ) const
                        { return ((nTime >= rFrom.nTime) && (nTime <= rTo.nTime)); }

    BOOL            IsEqualIgnore100Sec( const Time& rTime ) const;

    BOOL            operator ==( const Time& rTime ) const
                        { return (nTime == rTime.nTime); }
    BOOL            operator !=( const Time& rTime ) const
                        { return (nTime != rTime.nTime); }
    BOOL            operator  >( const Time& rTime ) const
                        { return (nTime > rTime.nTime); }
    BOOL            operator  <( const Time& rTime ) const
                        { return (nTime < rTime.nTime); }
    BOOL            operator >=( const Time& rTime ) const
                        { return (nTime >= rTime.nTime); }
    BOOL            operator <=( const Time& rTime ) const
                        { return (nTime <= rTime.nTime); }

    static Time     GetUTCOffset();
    static ULONG    GetSystemTicks();       // Elapsed time
    static ULONG    GetProcessTicks();      // CPU time

    void            ConvertToUTC()       { *this -= Time::GetUTCOffset(); }
    void            ConvertToLocalTime() { *this += Time::GetUTCOffset(); }

    Time&           operator =( const Time& rTime );
    Time            operator -() const
                        { return Time( nTime * -1 ); }
    Time&           operator +=( const Time& rTime );
    Time&           operator -=( const Time& rTime );
    TOOLS_DLLPUBLIC friend Time     operator +( const Time& rTime1, const Time& rTime2 );
    TOOLS_DLLPUBLIC friend Time     operator -( const Time& rTime1, const Time& rTime2 );
};

#endif // _TOOLS_TIME_HXX
