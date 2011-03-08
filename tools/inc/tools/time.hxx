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
#ifndef _TOOLS_TIME_HXX
#define _TOOLS_TIME_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
