/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_TOOLS_TIME_HXX
#define INCLUDED_TOOLS_TIME_HXX

#include <tools/toolsdllapi.h>
#include <tools/solar.h>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

/**
 @WARNING: This class can serve both as wall clock time and time duration, and
           the mixing of these concepts leads to problems such as there being
           25 hours or 10 minus 20 seconds being (non-negative) 10 seconds.
*/

namespace tools {

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED Time
{
private:
    sal_Int64       nTime;
    void            init( sal_uInt32 nHour, sal_uInt32 nMin,
                          sal_uInt32 nSec, sal_uInt64 nNanoSec);

public:
    enum TimeInitSystem
    {
        SYSTEM
    };

    // temporary until all uses are inspected and resolved
    enum TimeInitEmpty
    {
        EMPTY
    };
    static const sal_Int64 hourPerDay = 24;
    static const sal_Int64 minutePerHour = 60;
    static const sal_Int64 secondPerMinute = 60;
    static const sal_Int64 nanoSecPerSec = 1000000000;
    static const sal_Int64 nanoSecPerMinute = nanoSecPerSec * secondPerMinute;
    static const sal_Int64 nanoSecPerHour = nanoSecPerSec * secondPerMinute * minutePerHour;
    static const sal_Int64 nanoSecPerDay = nanoSecPerSec * secondPerMinute * minutePerHour * hourPerDay;
    static const sal_Int64 secondPerHour = secondPerMinute * minutePerHour;
    static const sal_Int64 secondPerDay  = secondPerMinute * minutePerHour * hourPerDay;
    static const sal_Int64 minutePerDay  = minutePerHour * hourPerDay;
    static const sal_Int64 nanoPerMicro  = 1000;
    static const sal_Int64 nanoPerMilli  = 1000000;
    static const sal_Int64 nanoPerCenti  = 10000000;

                    Time( TimeInitEmpty )
                        { nTime = 0; }
                    Time( TimeInitSystem );
                    Time( sal_Int64 _nTime ) { Time::nTime = _nTime; }
                    Time( const tools::Time& rTime );
                    Time( const css::util::Time& rTime );
                    Time( const css::util::DateTime& rDateTime );
                    Time( sal_uInt32 nHour, sal_uInt32 nMin,
                          sal_uInt32 nSec = 0, sal_uInt64 nNanoSec = 0 );

    void            SetTime( sal_Int64 nNewTime ) { nTime = nNewTime; }
    sal_Int64       GetTime() const { return nTime; }
    css::util::Time GetUNOTime() const { return css::util::Time(GetNanoSec(),GetSec(),GetMin(),GetHour(),false); }

    void            SetHour( sal_uInt16 nNewHour );
    void            SetMin( sal_uInt16 nNewMin );
    void            SetSec( sal_uInt16 nNewSec );
    void            SetNanoSec( sal_uInt32 nNewNanoSec );
    sal_uInt16      GetHour() const
                    { sal_uInt64 nTempTime = (nTime >= 0) ? nTime : -nTime;
                      return static_cast<sal_uInt16>(nTempTime / SAL_CONST_UINT64(10000000000000)); }
    sal_uInt16      GetMin() const
                    { sal_uInt64 nTempTime = (nTime >= 0) ? nTime : -nTime;
                      return static_cast<sal_uInt16>((nTempTime / SAL_CONST_UINT64(100000000000)) % 100); }
    sal_uInt16      GetSec() const
                    { sal_uInt64 nTempTime = (nTime >= 0) ? nTime : -nTime;
                      return static_cast<sal_uInt16>((nTempTime / SAL_CONST_UINT64(1000000000)) % 100); }
    sal_uInt32      GetNanoSec() const
                    { sal_uInt64 nTempTime = (nTime >= 0) ? nTime : -nTime;
                      return static_cast<sal_uInt32>( nTempTime % SAL_CONST_UINT64(1000000000)); }

    // TODO: consider removing GetMSFromTime and MakeTimeFromMS?
    sal_Int32       GetMSFromTime() const;
    void            MakeTimeFromMS( sal_Int32 nMS );
    sal_Int64       GetNSFromTime() const;
    void            MakeTimeFromNS( sal_Int64 nNS );

                    /// 12 hours == 0.5 days
    double          GetTimeInDays() const;

    bool            IsEqualIgnoreNanoSec( const tools::Time& rTime ) const;

    bool            operator ==( const tools::Time& rTime ) const
                    { return (nTime == rTime.nTime); }
    bool            operator !=( const tools::Time& rTime ) const
                    { return (nTime != rTime.nTime); }
    bool            operator  >( const tools::Time& rTime ) const
                    { return (nTime > rTime.nTime); }
    bool            operator  <( const tools::Time& rTime ) const
                    { return (nTime < rTime.nTime); }
    bool            operator >=( const tools::Time& rTime ) const
                    { return (nTime >= rTime.nTime); }
    bool            operator <=( const tools::Time& rTime ) const
                    { return (nTime <= rTime.nTime); }

    static Time     GetUTCOffset();

    /// Elapsed time since epoch in milliseconds
    static sal_uInt64 GetSystemTicks();

    tools::Time&           operator =( const tools::Time& rTime );
    Time            operator -() const
                        { return Time( -nTime ); }
    tools::Time&           operator +=( const tools::Time& rTime );
    tools::Time&           operator -=( const tools::Time& rTime );
    TOOLS_DLLPUBLIC friend Time     operator +( const tools::Time& rTime1, const tools::Time& rTime2 );
    TOOLS_DLLPUBLIC friend Time     operator -( const tools::Time& rTime1, const tools::Time& rTime2 );
};

} /* namespace tools */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
