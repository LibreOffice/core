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

#include <sal/config.h>

#include <cmath>

#include <tools/toolsdllapi.h>
#include <com/sun/star/util/Time.hpp>

#include <compare>

namespace com::sun::star::util { struct DateTime; }

/**
 @WARNING: This class can serve both as wall clock time and time duration, and
           the mixing of these concepts leads to problems such as there being
           25 hours or 10 minus 20 seconds being (non-negative) 10 seconds.
*/

namespace tools {

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Time
{
private:
    sal_Int64       nTime;
    explicit Time(sal_Int64 _nTime) { nTime = _nTime; }
    static sal_Int64 assemble(sal_uInt32 h, sal_uInt32 m, sal_uInt32 s, sal_uInt64 ns);
    short GetSign() const { return (nTime >= 0) ? +1 : -1; }

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

    static const sal_Int64 milliSecPerSec = 1000;
    static const sal_Int64 milliSecPerMinute = milliSecPerSec * secondPerMinute;
    static const sal_Int64 milliSecPerHour = milliSecPerMinute * minutePerHour;
    static const sal_Int64 milliSecPerDay = milliSecPerHour * hourPerDay;


                    explicit Time( TimeInitEmpty )
                        { nTime = 0; }
                    explicit Time( TimeInitSystem );
                    Time( const tools::Time& rTime ) = default;
                    explicit Time( const css::util::Time& rTime );
                    explicit Time( const css::util::DateTime& rDateTime );
                    Time( sal_uInt32 nHour, sal_uInt32 nMin,
                          sal_uInt32 nSec = 0, sal_uInt64 nNanoSec = 0 );

    // The argument is not nanoseconds, it's what nTime must contain!
    static Time fromEncodedTime(sal_Int64 _nTime) { return Time(_nTime); }

    void            SetTime( sal_Int64 nNewTime ) { nTime = nNewTime; }
    sal_Int64       GetTime() const { return nTime; }
    css::util::Time GetUNOTime() const { return css::util::Time(GetNanoSec(),GetSec(),GetMin(),GetHour(),false); }

    void            SetHour( sal_uInt16 nNewHour );
    void            SetMin( sal_uInt16 nNewMin );
    void            SetSec( sal_uInt16 nNewSec );
    void            SetNanoSec( sal_uInt32 nNewNanoSec );
    sal_uInt16      GetHour() const { return std::abs(nTime) / SAL_CONST_UINT64(10000000000000); }
    sal_uInt16      GetMin() const { return (std::abs(nTime) / SAL_CONST_UINT64(100000000000)) % 100; }
    sal_uInt16      GetSec() const { return (std::abs(nTime) / SAL_CONST_UINT64(1000000000)) % 100; }
    sal_uInt32      GetNanoSec() const { return std::abs(nTime) % SAL_CONST_UINT64(1000000000); }

    // TODO: consider removing GetMSFromTime and MakeTimeFromMS?
    sal_Int32       GetMSFromTime() const;
    void            MakeTimeFromMS( sal_Int32 nMS );
    sal_Int64       GetNSFromTime() const;
    void            MakeTimeFromNS( sal_Int64 nNS );

                    /// 12 hours == 0.5 days
    double          GetTimeInDays() const;

    /** Get the wall clock time particles for a (date+)time value.

        Does the necessary rounding and truncating to obtain hour, minute,
        second and fraction of second from a double time value (time in days,
        0.5 == 12h) such that individual values are not rounded up, i.e.
        x:59:59.999 does not yield x+1:0:0.00

        A potential date component (fTimeInDays >= 1.0) is discarded.

        @param  nFractionDecimals
                If > 0 fFractionOfSecond is truncated to that amount of
                decimals.
                Else fFractionOfSecond returns the full remainder of the
                fractional second.
     */
    static void     GetClock( double fTimeInDays,
                              sal_uInt16& nHour, sal_uInt16& nMinute, sal_uInt16& nSecond,
                              double& fFractionOfSecond, int nFractionDecimals );

    bool            IsEqualIgnoreNanoSec( const tools::Time& rTime ) const;

    auto            operator <=> ( const Time& rTime ) const = default;

    static Time     GetUTCOffset();

    /**
     * Elapsed time in milliseconds (1e-3) since some unspecified starting point
     *
     * Convenience function, which just calls GetMonotonicTicks() / 1000.
     */
    static sal_uInt64 GetSystemTicks();

    /**
     * Elapsed time in microseconds (1e-6) since some unspecified starting point
     *
     * Uses the high-precision, monotonic time sources provided by the OS, if
     * available. Don't try to relate it to the system time, and also it's long
     * time accuracy is not the best.
     *
     * Currently used to measure the runtime of OpenCL shaders and to set a
     * message creation timestamp to allow filtering of invalid timer messages.
     *
     * @return current system ticks in microseconds (1e-6s)
     */
    static sal_uInt64 GetMonotonicTicks();

    tools::Time&           operator =( const tools::Time& rTime ) = default;
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
