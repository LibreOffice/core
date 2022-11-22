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
#include <tools/date.hxx>
#include <sal/log.hxx>
#include <com/sun/star/util/DateTime.hpp>

#include <systemdatetime.hxx>
#include <comphelper/date.hxx>

namespace
{

const sal_Int16 kYearMax = SAL_MAX_INT16;
const sal_Int16 kYearMin = SAL_MIN_INT16;

}

void Date::setDateFromDMY( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear )
{
    // don't warn about 0/0/0, commonly used as a default-value/no-value
    SAL_WARN_IF( nYear == 0 && !(nYear == 0 && nMonth == 0 && nDay == 0),
        "tools.datetime", "Date::setDateFromDMY - sure about 0 year? It's not in the calendar.");
    assert( nMonth < 100 && "nMonth % 100 not representable" );
    assert(   nDay < 100 && "nDay % 100 not representable" );
    if (nYear < 0)
        mnDate =
            (static_cast<sal_Int32>( nYear        ) * 10000) -
            (static_cast<sal_Int32>( nMonth % 100 ) * 100) -
             static_cast<sal_Int32>( nDay   % 100 );
    else
        mnDate =
            (static_cast<sal_Int32>( nYear        ) * 10000) +
            (static_cast<sal_Int32>( nMonth % 100 ) * 100) +
             static_cast<sal_Int32>( nDay   % 100 );
}

void Date::SetDate( sal_Int32 nNewDate )
{
    assert( ((nNewDate / 10000) != 0) && "you don't want to set a 0 year, do you?" );
    mnDate = nNewDate;
}

// static
sal_uInt16 Date::GetDaysInMonth( sal_uInt16 nMonth, sal_Int16 nYear )
{
    SAL_WARN_IF( nMonth < 1 || 12 < nMonth, "tools.datetime", "Date::GetDaysInMonth - nMonth out of bounds " << nMonth);
    if (nMonth < 1)
        nMonth = 1;
    else if (12 < nMonth)
        nMonth = 12;
    return comphelper::date::getDaysInMonth( nMonth, nYear);
}

sal_Int32 Date::GetAsNormalizedDays() const
{
    // This is a very common datum we often calculate from.
    if (mnDate == 18991230) // 1899-12-30
    {
#ifndef NDEBUG
        static sal_Int32 nDays = DateToDays( GetDay(), GetMonth(), GetYear());
        assert(nDays == 693594);
#endif
        return 693594;
    }
    // Not calling comphelper::date::convertDateToDaysNormalizing() here just
    // avoids a second check on null-date handling like above.
    sal_uInt16 nDay = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_Int16 nYear = GetYear();
    comphelper::date::normalize( nDay, nMonth, nYear);
    return comphelper::date::convertDateToDays( nDay, nMonth, nYear);
}

sal_Int32 Date::DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear )
{
    return comphelper::date::convertDateToDaysNormalizing( nDay, nMonth, nYear);
}

static Date lcl_DaysToDate( sal_Int32 nDays )
{
    sal_uInt16 nDay;
    sal_uInt16 nMonth;
    sal_Int16 nYear;
    comphelper::date::convertDaysToDate( nDays, nDay, nMonth, nYear);
    return Date( nDay, nMonth, nYear);
}

Date::Date( DateInitSystem )
{
    if ( !GetSystemDateTime( &mnDate, nullptr ) )
        setDateFromDMY( 1, 1, 1900 );
}

Date::Date( const css::util::DateTime& rDateTime )
{
    setDateFromDMY( rDateTime.Day, rDateTime.Month, rDateTime.Year );
}

void Date::SetDay( sal_uInt16 nNewDay )
{
    setDateFromDMY( nNewDay, GetMonth(), GetYear() );
}

void Date::SetMonth( sal_uInt16 nNewMonth )
{
    setDateFromDMY( GetDay(), nNewMonth, GetYear() );
}

void Date::SetYear( sal_Int16 nNewYear )
{
    assert( nNewYear != 0 );
    setDateFromDMY( GetDay(), GetMonth(), nNewYear );
}

void Date::AddYears( sal_Int16 nAddYears )
{
    sal_Int16 nYear = GetYear();
    if (nYear < 0)
    {
        if (nAddYears < 0)
        {
            if (nYear < kYearMin - nAddYears)
                nYear = kYearMin;
            else
                nYear += nAddYears;
        }
        else
        {
            nYear += nAddYears;
            if (nYear == 0)
                nYear = 1;
        }
    }
    else
    {
        if (nAddYears > 0)
        {
            if (kYearMax - nAddYears < nYear)
                nYear = kYearMax;
            else
                nYear += nAddYears;
        }
        else
        {
            nYear += nAddYears;
            if (nYear == 0)
                nYear = -1;
        }
    }

    SetYear( nYear );
    if (GetMonth() == 2 && GetDay() == 29 && !comphelper::date::isLeapYear( nYear))
        SetDay(28);
}

void Date::AddMonths( sal_Int32 nAddMonths )
{
    sal_Int32 nMonths = GetMonth() + nAddMonths;
    sal_Int32 nNewMonth = nMonths % 12;
    sal_Int32 nYear = GetYear() + nMonths / 12;
    if( nMonths <= 0 || nNewMonth == 0 )
        --nYear;
    if( nNewMonth <= 0 )
        nNewMonth += 12;
    if (nYear == 0)
        nYear = (nAddMonths < 0 ? -1 : 1);
    else if (nYear < kYearMin)
        nYear = kYearMin;
    else if (nYear > kYearMax)
        nYear = kYearMax;
    SetMonth( static_cast<sal_uInt16>(nNewMonth) );
    SetYear( static_cast<sal_Int16>(nYear) );
    Normalize();
}

DayOfWeek Date::GetDayOfWeek() const
{
    return static_cast<DayOfWeek>((GetAsNormalizedDays()-1) % 7);
}

sal_uInt16 Date::GetDayOfYear() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_Int16  nYear  = GetYear();
    Normalize( nDay, nMonth, nYear);

    for( sal_uInt16 i = 1; i < nMonth; i++ )
         nDay += comphelper::date::getDaysInMonth( i, nYear );
    return nDay;
}

sal_uInt16 Date::GetWeekOfYear( DayOfWeek eStartDay,
                                sal_Int16 nMinimumNumberOfDaysInWeek ) const
{
    short nWeek;
    short n1WDay = static_cast<short>(Date( 1, 1, GetYear() ).GetDayOfWeek());
    short nDayOfYear = static_cast<short>(GetDayOfYear());

    // weekdays start at 0, thus decrement one
    nDayOfYear--;
    // account for StartDay
    n1WDay = (n1WDay+(7-static_cast<short>(eStartDay))) % 7;

    if (nMinimumNumberOfDaysInWeek < 1 || 7 < nMinimumNumberOfDaysInWeek)
    {
        SAL_WARN( "tools.datetime", "Date::GetWeekOfYear: invalid nMinimumNumberOfDaysInWeek" );
        nMinimumNumberOfDaysInWeek = 4;
    }

    if ( nMinimumNumberOfDaysInWeek == 1 )
    {
        nWeek = ((n1WDay+nDayOfYear)/7) + 1;
        // Set to 53rd week only if we're not in the
        // first week of the new year
        if ( nWeek == 54 )
            nWeek = 1;
        else if ( nWeek == 53 )
        {
            short nDaysInYear = static_cast<short>(GetDaysInYear());
            short nDaysNextYear = static_cast<short>(Date( 1, 1, GetNextYear() ).GetDayOfWeek());
            nDaysNextYear = (nDaysNextYear+(7-static_cast<short>(eStartDay))) % 7;
            if ( nDayOfYear > (nDaysInYear-nDaysNextYear-1) )
                nWeek = 1;
        }
    }
    else if ( nMinimumNumberOfDaysInWeek == 7 )
    {
        nWeek = ((n1WDay+nDayOfYear)/7);
        // First week of a year is equal to the last week of the previous year
        if ( nWeek == 0 )
        {
            Date aLastDatePrevYear( 31, 12, GetPrevYear() );
            nWeek = aLastDatePrevYear.GetWeekOfYear( eStartDay, nMinimumNumberOfDaysInWeek );
        }
    }
    else // ( nMinimumNumberOfDaysInWeek == something_else, commentary examples for 4 )
    {
        // x_monday - thursday
        if ( n1WDay < nMinimumNumberOfDaysInWeek )
            nWeek = 1;
        // Friday
        else if ( n1WDay == nMinimumNumberOfDaysInWeek )
            nWeek = 53;
        // Saturday
        else if ( n1WDay == nMinimumNumberOfDaysInWeek + 1 )
        {
            // Year after leap year
            if ( Date( 1, 1, GetPrevYear() ).IsLeapYear() )
                nWeek = 53;
            else
                nWeek = 52;
        }
        // Sunday
        else
            nWeek = 52;

        if ( (nWeek == 1) || (nDayOfYear + n1WDay > 6) )
        {
            if ( nWeek == 1 )
                nWeek += (nDayOfYear + n1WDay) / 7;
            else
                nWeek = (nDayOfYear + n1WDay) / 7;
            if ( nWeek == 53 )
            {
                // next x_Sunday == first x_Sunday in the new year
                // == still the same week!
                sal_Int32 nTempDays = GetAsNormalizedDays();

                nTempDays +=  6 - (GetDayOfWeek()+(7-static_cast<short>(eStartDay))) % 7;
                nWeek = lcl_DaysToDate( nTempDays ).GetWeekOfYear( eStartDay, nMinimumNumberOfDaysInWeek );
            }
        }
    }

    return static_cast<sal_uInt16>(nWeek);
}

sal_uInt16 Date::GetDaysInMonth() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_Int16  nYear  = GetYear();
    Normalize( nDay, nMonth, nYear);

    return comphelper::date::getDaysInMonth( nMonth, nYear );
}

bool Date::IsLeapYear() const
{
    sal_Int16 nYear = GetYear();
    return comphelper::date::isLeapYear( nYear );
}

bool Date::IsValidAndGregorian() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_Int16  nYear  = GetYear();

    if ( !nMonth || (nMonth > 12) )
        return false;
    if ( !nDay || (nDay > comphelper::date::getDaysInMonth( nMonth, nYear )) )
        return false;
    else if ( nYear <= 1582 )
    {
        if ( nYear < 1582 )
            return false;
        else if ( nMonth < 10 )
            return false;
        else if ( (nMonth == 10) && (nDay < 15) )
            return false;
    }

    return true;
}

bool Date::IsValidDate() const
{
    return comphelper::date::isValidDate( GetDay(), GetMonth(), GetYear());
}

//static
bool Date::IsValidDate( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear )
{
    return comphelper::date::isValidDate( nDay, nMonth, nYear);
}

bool Date::IsEndOfMonth() const
{
    return IsEndOfMonth(GetDay(), GetMonth(), GetYear());
}

//static
bool Date::IsEndOfMonth(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear)
{
    return comphelper::date::isValidDate(nDay, nMonth, nYear)
        && comphelper::date::getDaysInMonth(nMonth, nYear) == nDay;
}

void Date::Normalize()
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_Int16  nYear  = GetYear();

    if (!Normalize( nDay, nMonth, nYear))
        return;

    setDateFromDMY( nDay, nMonth, nYear );
}

//static
bool Date::Normalize( sal_uInt16 & rDay, sal_uInt16 & rMonth, sal_Int16 & rYear )
{
    return comphelper::date::normalize( rDay, rMonth, rYear);
}

void Date::AddDays( sal_Int32 nDays )
{
    if (nDays != 0)
        *this = lcl_DaysToDate( GetAsNormalizedDays() + nDays );
}

Date& Date::operator ++()
{
    *this = lcl_DaysToDate( GetAsNormalizedDays() + 1 );
    return *this;
}

Date& Date::operator --()
{
    *this = lcl_DaysToDate( GetAsNormalizedDays() - 1 );
    return *this;
}

Date operator +( const Date& rDate, sal_Int32 nDays )
{
    Date aDate( rDate );
    aDate.AddDays( nDays );
    return aDate;
}

Date operator -( const Date& rDate, sal_Int32 nDays )
{
    Date aDate( rDate );
    aDate.AddDays( -nDays );
    return aDate;
}

sal_Int32 operator -( const Date& rDate1, const Date& rDate2 )
{
    return rDate1.GetAsNormalizedDays() - rDate2.GetAsNormalizedDays();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
