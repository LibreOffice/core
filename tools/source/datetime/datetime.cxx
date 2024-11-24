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
#include <tools/datetime.hxx>
#include <tools/duration.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <compare>

#include <systemdatetime.hxx>

DateTime::DateTime(DateTimeInitSystem)
    : Date( Date::EMPTY )
    , Time( Time::EMPTY )
{
    sal_Int32 nD = 0;
    sal_Int64 nT = 0;
    if ( GetSystemDateTime( &nD, &nT ) )
    {
        Date::operator=( Date( nD ) );
        SetTime( nT );
    }
    else
        Date::operator=( Date( 1, 1, 1900 ) ); // Time::nTime is already 0
}

DateTime::DateTime( const css::util::DateTime& rDateTime )
  : Date( rDateTime.Day, rDateTime.Month, rDateTime.Year ),
    Time( rDateTime.Hours, rDateTime.Minutes, rDateTime.Seconds, rDateTime.NanoSeconds )
{
}

DateTime& DateTime::operator =( const css::util::DateTime& rUDateTime )
{
    Date::operator=( Date( rUDateTime.Day, rUDateTime.Month, rUDateTime.Year));
    Time::operator=( Time( rUDateTime));
    return *this;
}

bool DateTime::IsBetween( const DateTime& rFrom, const DateTime& rTo ) const
{
    return (*this >= rFrom) && (*this <= rTo);
}

sal_Int64 DateTime::GetSecFromDateTime( const Date& rDate ) const
{
    if (*this < rDate)
        return 0;
    else
    {
        sal_Int64 nSec = Date( *this ) - rDate;
        nSec *= 24UL*60*60;
        sal_Int64 nHour = GetHour();
        sal_Int64 nMin  = GetMin();
        nSec += (nHour*3600)+(nMin*60)+GetSec();
        return nSec;
    }
}

void DateTime::NormalizeTimeRemainderAndApply( tools::Time& rTime )
{
    sal_uInt16 nHours = rTime.GetHour();
    if ( rTime.GetTime() > 0 )
    {
        if (nHours >= 24)
        {
            AddDays( nHours / 24 );
            nHours %= 24;
            rTime.SetHour( nHours );
        }
    }
    else if ( rTime.GetTime() != 0 )
    {
        if (nHours >= 24)
        {
            AddDays( -static_cast<sal_Int32>(nHours) / 24 );
            nHours %= 24;
            rTime.SetHour( nHours );
        }
        Date::operator--();
        rTime = Time( 24, 0, 0 ) + rTime;
    }
    tools::Time::operator=( rTime );
}

DateTime& DateTime::operator +=( const tools::Time& rTime )
{
    tools::Time aTime = *this;
    aTime += rTime;
    NormalizeTimeRemainderAndApply(aTime);
    return *this;
}

DateTime& DateTime::operator -=( const tools::Time& rTime )
{
    tools::Time aTime = *this;
    aTime -= rTime;
    NormalizeTimeRemainderAndApply(aTime);
    return *this;
}

DateTime& DateTime::operator +=( const tools::Duration& rDuration )
{
    AddDays(rDuration.GetDays());
    operator+=(rDuration.GetTime());
    return *this;
}

DateTime operator +( const DateTime& rDateTime, sal_Int32 nDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime.AddDays( nDays );
    return aDateTime;
}

DateTime operator -( const DateTime& rDateTime, sal_Int32 nDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime.AddDays( -nDays );
    return aDateTime;
}

DateTime operator +( const DateTime& rDateTime, const tools::Time& rTime )
{
    DateTime aDateTime( rDateTime );
    aDateTime += rTime;
    return aDateTime;
}

DateTime operator -( const DateTime& rDateTime, const tools::Time& rTime )
{
    DateTime aDateTime( rDateTime );
    aDateTime -= rTime;
    return aDateTime;
}

DateTime operator +( const DateTime& rDateTime, const tools::Duration& rDuration )
{
    DateTime aDateTime(rDateTime);
    aDateTime.AddDays( rDuration.GetDays());
    aDateTime += rDuration.GetTime();
    return aDateTime;
}

void DateTime::AddTime( double fTimeInDays )
{
    // Use Duration to diminish floating point accuracy errors.
    tools::Duration aDuration(fTimeInDays);
    operator+=(aDuration);
}

DateTime operator +( const DateTime& rDateTime, double fTimeInDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime.AddTime( fTimeInDays );
    return aDateTime;
}

tools::Duration operator -( const DateTime& rDateTime1, const DateTime& rDateTime2 )
{
    return tools::Duration( rDateTime2, rDateTime1);
}

// static
double DateTime::Sub( const DateTime& rDateTime1, const DateTime& rDateTime2 )
{
    if (static_cast<const tools::Time&>(rDateTime1) != static_cast<const tools::Time&>(rDateTime2))
    {
        // Use Duration to diminish floating point accuracy errors.
        const tools::Duration aDuration( rDateTime2, rDateTime1);
        return aDuration.GetInDays();
    }
    return static_cast<const Date&>(rDateTime1) - static_cast<const Date&>(rDateTime2);
}

void DateTime::GetWin32FileDateTime( sal_uInt32 & rLower, sal_uInt32 & rUpper ) const
{
    const sal_Int64 a100nPerSecond = SAL_CONST_INT64( 10000000 );
    const sal_Int64 a100nPerDay = a100nPerSecond * sal_Int64( 60 * 60 * 24 );

    // FILETIME is indirectly documented as uint64, see
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284.aspx
    // mentioning the ULARGE_INTEGER structure.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724280.aspx
    // mentions that if FILETIME is not less than 0x8000000000000000 then the
    // FileTimeToSystemTime function fails, which is another indicator.
    // Unless there's evidence that FILETIME can represent a signed offset from
    // 1601-01-01 truncate at 0. (reading part below in
    // CreateFromWin32FileDateTime() would had to be adapted to signed as
    // well).
    sal_Int16 nYear = GetYear();
    SAL_WARN_IF( nYear < 1601, "tools.datetime", "DateTime::GetWin32FileDateTime - year < 1601: " << nYear);

    sal_Int64 aTime = (nYear < 1601 ? 0 : (
        a100nPerDay * (*this - Date(1,1,1601)) +
        GetNSFromTime()/100));

    rLower = sal_uInt32( aTime % SAL_CONST_UINT64( 0x100000000 ) );
    rUpper = sal_uInt32( aTime / SAL_CONST_UINT64( 0x100000000 ) );
}

DateTime DateTime::CreateFromWin32FileDateTime( sal_uInt32 rLower, sal_uInt32 rUpper )
{
    // (rUpper|rLower) = 100-nanosecond intervals since 1601-01-01 00:00
    const sal_uInt64 a100nPerSecond = SAL_CONST_UINT64( 10000000 );
    const sal_uInt64 a100nPerDay = a100nPerSecond * sal_uInt64( 60 * 60 * 24 );

    sal_uInt64 aTime =
            sal_uInt64( rUpper ) * SAL_CONST_UINT64( 0x100000000 ) +
            sal_uInt64( rLower );

    SAL_WARN_IF( static_cast<sal_Int64>(aTime) < 0, "tools.datetime",
            "DateTime::CreateFromWin32FileDateTime - absurdly high value expected?");

    sal_uInt64 nDays = aTime / a100nPerDay;

    Date aDate(1,1,1601);
    // (0xffffffffffffffff / a100nPerDay = 21350398) fits into sal_Int32
    // (0x7fffffff = 2147483647)
    aDate.AddDays(nDays);

    SAL_WARN_IF( aDate - Date(1,1,1601) != static_cast<sal_Int32>(nDays), "tools.datetime",
            "DateTime::CreateFromWin32FileDateTime - date truncated to max");

    sal_uInt64 nNanos = (aTime - (nDays * a100nPerDay)) * 100;
    return DateTime( aDate, tools::Time(
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerHour)   % sal_uInt64( 24 )),
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerMinute) % sal_uInt64( 60 )),
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerSec)    % sal_uInt64( 60 )),
                static_cast<sal_uInt64>( nNanos % tools::Time::nanoSecPerSec)));
}

DateTime DateTime::CreateFromUnixTime(const double fSecondsSinceEpoch)
{
    double fValue = fSecondsSinceEpoch / Time::secondPerDay;
    const sal_Int32 nDays = static_cast <sal_Int32>(::rtl::math::approxFloor(fValue));

    Date aDate (1, 1, 1970);
    aDate.AddDays(nDays);
    SAL_WARN_IF(aDate - Date(1, 1, 1970) != nDays, "tools.datetime",
                "DateTime::CreateFromUnixTime - date truncated to max");

    fValue -= nDays;

    const sal_uInt64 nNanos = fValue * tools::Time::nanoSecPerDay;
    return DateTime( aDate, tools::Time(
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerHour)   % sal_uInt64( 24 )),
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerMinute) % sal_uInt64( 60 )),
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerSec)    % sal_uInt64( 60 )),
                static_cast<sal_uInt64>( nNanos % tools::Time::nanoSecPerSec)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
