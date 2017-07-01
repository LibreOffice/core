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
#include <rtl/math.hxx>

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
    if ( (*this >= rFrom) && (*this <= rTo) )
        return true;
    else
        return false;
}

bool DateTime::operator >( const DateTime& rDateTime ) const
{
    if ( (Date::operator>( rDateTime )) ||
         (Date::operator==( rDateTime ) && tools::Time::operator>( rDateTime )) )
        return true;
    else
        return false;
}

bool DateTime::operator <( const DateTime& rDateTime ) const
{
    if ( (Date::operator<( rDateTime )) ||
         (Date::operator==( rDateTime ) && tools::Time::operator<( rDateTime )) )
        return true;
    else
        return false;
}

bool DateTime::operator >=( const DateTime& rDateTime ) const
{
    if ( (Date::operator>( rDateTime )) ||
         (Date::operator==( rDateTime ) && tools::Time::operator>=( rDateTime )) )
        return true;
    else
        return false;
}

bool DateTime::operator <=( const DateTime& rDateTime ) const
{
    if ( (Date::operator<( rDateTime )) ||
         (Date::operator==( rDateTime ) && tools::Time::operator<=( rDateTime )) )
        return true;
    else
        return false;
}

sal_Int64 DateTime::GetSecFromDateTime( const Date& rDate ) const
{
    if ( Date::operator<( rDate ) )
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

DateTime& DateTime::operator +=( const tools::Time& rTime )
{
    tools::Time aTime = *this;
    aTime += rTime;
    sal_uInt16 nHours = aTime.GetHour();
    if ( aTime.GetTime() > 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator++();
            nHours -= 24;
        }
        aTime.SetHour( nHours );
    }
    else if ( aTime.GetTime() != 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator--();
            nHours -= 24;
        }
        Date::operator--();
        aTime = Time( 24, 0, 0 )+aTime;
    }
    tools::Time::operator=( aTime );

    return *this;
}

DateTime& DateTime::operator -=( const tools::Time& rTime )
{
    tools::Time aTime = *this;
    aTime -= rTime;
    sal_uInt16 nHours = aTime.GetHour();
    if ( aTime.GetTime() > 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator++();
            nHours -= 24;
        }
        aTime.SetHour( nHours );
    }
    else if ( aTime.GetTime() != 0 )
    {
        while ( nHours >= 24 )
        {
            Date::operator--();
            nHours -= 24;
        }
        Date::operator--();
        aTime = Time( 24, 0, 0 )+aTime;
    }
    tools::Time::operator=( aTime );

    return *this;
}

DateTime operator +( const DateTime& rDateTime, sal_Int32 nDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime += nDays;
    return aDateTime;
}

DateTime operator -( const DateTime& rDateTime, sal_Int32 nDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime -= nDays;
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

DateTime& DateTime::operator +=( double fTimeInDays )
{
    double fInt, fFrac;
    if ( fTimeInDays < 0.0 )
    {
        fInt = ::rtl::math::approxCeil( fTimeInDays );
        fFrac = fInt <= fTimeInDays ? 0.0 : fTimeInDays - fInt;
    }
    else
    {
        fInt = ::rtl::math::approxFloor( fTimeInDays );
        fFrac = fInt >= fTimeInDays ? 0.0 : fTimeInDays - fInt;
    }
    Date::operator+=( sal_Int32(fInt) );     // full days
    if ( fFrac )
    {
        tools::Time aTime(0);  // default ctor calls system time, we don't need that
        fFrac *= ::tools::Time::nanoSecPerDay;   // time expressed in nanoseconds
        aTime.MakeTimeFromNS( static_cast<sal_Int64>(fFrac) );    // method handles negative ns
        operator+=( aTime );
    }
    return *this;
}

DateTime operator +( const DateTime& rDateTime, double fTimeInDays )
{
    DateTime aDateTime( rDateTime );
    aDateTime += fTimeInDays;
    return aDateTime;
}

double operator -( const DateTime& rDateTime1, const DateTime& rDateTime2 )
{
    sal_Int32 nDays = static_cast<const Date&>(rDateTime1)
        - static_cast<const Date&>(rDateTime2);
    sal_Int64 nTime = rDateTime1.GetNSFromTime() - rDateTime2.GetNSFromTime();
    if ( nTime )
    {
        double fTime = double(nTime);
        fTime /= ::tools::Time::nanoSecPerDay; // convert from nanoseconds to fraction
        if ( nDays < 0 && fTime > 0.0 )
            fTime = 1.0 - fTime;
        return double(nDays) + fTime;
    }
    return double(nDays);
}

void DateTime::GetWin32FileDateTime( sal_uInt32 & rLower, sal_uInt32 & rUpper )
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
        a100nPerDay * static_cast<sal_Int64>(*this - Date(1,1,1601)) +
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
    aDate += static_cast<sal_Int32>(nDays);

    SAL_WARN_IF( aDate - Date(1,1,1601) != static_cast<sal_Int32>(nDays), "tools.datetime",
            "DateTime::CreateFromWin32FileDateTime - date truncated to max");

    sal_uInt64 nNanos = (aTime - (nDays * a100nPerDay)) * 100;
    return DateTime( aDate, tools::Time(
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerHour)   % sal_uInt64( 24 )),
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerMinute) % sal_uInt64( 60 )),
                static_cast<sal_uInt32>((nNanos / tools::Time::nanoSecPerSec)    % sal_uInt64( 60 )),
                static_cast<sal_uInt64>( nNanos % tools::Time::nanoSecPerSec)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
