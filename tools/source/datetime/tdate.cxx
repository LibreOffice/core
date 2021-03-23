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

const sal_uInt16 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                             31, 31, 30, 31, 30, 31 };

// Once upon a time the number of days we internally handled was limited to
// MAX_DAYS 3636532. That changed with a full 16-bit year.
// Assuming the first valid positive date in a proleptic Gregorian calendar is
// 0001-01-01, this resulted in an end date of 9957-06-26.
// Hence we documented that years up to and including 9956 are handled.
/* XXX: it is unclear history why this value was chosen, the representable
 * 9999-12-31 would be 3652060 days from 0001-01-01. Even 9998-12-31 to
 * distinguish from a maximum possible date would be 3651695.
 * There is connectivity/source/commontools/dbconversion.cxx that still has the
 * same value to calculate with css::util::Date */
/* XXX can that dbconversion cope with years > 9999 or negative years at all?
 * Database fields may be limited to positive 4 digits. */

const sal_Int32 MIN_DAYS = -11968265;     // -32768-01-01
const sal_Int32 MAX_DAYS =  11967900;     //  32767-12-31

namespace
{

const sal_Int16 kYearMax = SAL_MAX_INT16;
const sal_Int16 kYearMin = SAL_MIN_INT16;

// Days until start of year from zero, so month and day of month can be added.
// year 1 => 0 days, year 2 => 365 days, ...
// year -1 => -366 days, year -2 => -731 days, ...
sal_Int32 ImpYearToDays( sal_Int16 nYear )
{
    assert( nYear != 0 );
    sal_Int32 nOffset;
    sal_Int32 nYr;
    if (nYear < 0)
    {
        nOffset = -366;
        nYr = nYear + 1;
    }
    else
    {
        nOffset = 0;
        nYr = nYear - 1;
    }
    return nOffset + nYr*365 + nYr/4 - nYr/100 + nYr/400;
}

bool ImpIsLeapYear( sal_Int16 nYear )
{
    // Leap years BCE are -1, -5, -9, ...
    // See
    // https://en.wikipedia.org/wiki/Proleptic_Gregorian_calendar#Usage
    // https://en.wikipedia.org/wiki/0_(year)#History_of_astronomical_usage
    assert( nYear != 0 );
    if (nYear < 0)
        nYear = -nYear - 1;
    return ( ( ((nYear % 4) == 0) && ((nYear % 100) != 0) ) ||
             ( (nYear % 400) == 0 ) );
}

// All callers must have sanitized or normalized month and year values!
sal_uInt16 ImplDaysInMonth( sal_uInt16 nMonth, sal_Int16 nYear )
{
    if ( nMonth != 2 )
        return aDaysInMonth[nMonth-1];
    else
    {
        if (ImpIsLeapYear(nYear))
            return aDaysInMonth[nMonth-1] + 1;
        else
            return aDaysInMonth[nMonth-1];
    }
}

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
    return ImplDaysInMonth( nMonth, nYear);
}

sal_Int32 Date::GetAsNormalizedDays() const
{
    // This is a very common datum we often calculate from.
    if (mnDate == 18991230) // 1899-12-30
    {
        assert(DateToDays( GetDay(), GetMonth(), GetYear() ) == 693594);
        return 693594;
    }
    return DateToDays( GetDay(), GetMonth(), GetYear() );
}

sal_Int32 Date::DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear )
{
    Normalize( nDay, nMonth, nYear);

    sal_Int32 nDays = ImpYearToDays(nYear);
    for( sal_uInt16 i = 1; i < nMonth; i++ )
        nDays += ImplDaysInMonth(i,nYear);
    nDays += nDay;
    return nDays;
}

static Date lcl_DaysToDate( sal_Int32 nDays )
{
    if ( nDays <= MIN_DAYS )
        return Date( 1, 1, kYearMin );
    if ( nDays >= MAX_DAYS )
        return Date( 31, 12, kYearMax );

    // Day 0 is -0001-12-31, day 1 is 0001-01-01
    const sal_Int16 nSign = (nDays <= 0 ? -1 : 1);
    sal_Int32 nTempDays;
    sal_Int32 i = 0;
    bool bCalc;

    sal_Int16 nYear;
    do
    {
        nYear = static_cast<sal_Int16>((nDays / 365) - (i * nSign));
        if (nYear == 0)
            nYear = nSign;
        nTempDays = nDays - ImpYearToDays(nYear);
        bCalc = false;
        if ( nTempDays < 1 )
        {
            i += nSign;
            bCalc = true;
        }
        else
        {
            if ( nTempDays > 365 )
            {
                if ( (nTempDays != 366) || !ImpIsLeapYear( nYear ) )
                {
                    i -= nSign;
                    bCalc = true;
                }
            }
        }
    }
    while ( bCalc );

    sal_uInt16 nMonth = 1;
    while ( nTempDays > ImplDaysInMonth( nMonth, nYear ) )
    {
        nTempDays -= ImplDaysInMonth( nMonth, nYear );
        ++nMonth;
    }

    return Date( static_cast<sal_uInt16>(nTempDays), nMonth, nYear );
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
    if (GetMonth() == 2 && GetDay() == 29 && !ImpIsLeapYear( nYear))
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
         nDay += ::ImplDaysInMonth( i, nYear );
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

    return ImplDaysInMonth( nMonth, nYear );
}

bool Date::IsLeapYear() const
{
    sal_Int16 nYear = GetYear();
    return ImpIsLeapYear( nYear );
}

bool Date::IsValidAndGregorian() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_Int16  nYear  = GetYear();

    if ( !nMonth || (nMonth > 12) )
        return false;
    if ( !nDay || (nDay > ImplDaysInMonth( nMonth, nYear )) )
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
    return IsValidDate( GetDay(), GetMonth(), GetYear());
}

//static
bool Date::IsValidDate( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear )
{
    if (nYear == 0)
        return false;
    if ( !nMonth || (nMonth > 12) )
        return false;
    if ( !nDay || (nDay > ImplDaysInMonth( nMonth, nYear )) )
        return false;
    return true;
}

bool Date::IsEndOfMonth() const
{
    return IsEndOfMonth(GetDay(), GetMonth(), GetYear());
}

//static
bool Date::IsEndOfMonth(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear)
{
    return IsValidDate(nDay, nMonth, nYear) && ImplDaysInMonth(nMonth, nYear) == nDay;
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
    if (IsValidDate( rDay, rMonth, rYear))
        return false;

    if (rDay == 0 && rMonth == 0 && rYear == 0)
        return false;   // empty date

    if (rDay == 0)
    {
        if (rMonth == 0)
            ;   // nothing, handled below
        else
            --rMonth;
        // Last day of month is determined at the end.
    }

    if (rMonth > 12)
    {
        rYear += rMonth / 12;
        rMonth = rMonth % 12;
        if (rYear == 0)
            rYear = 1;
    }
    if (rMonth == 0)
    {
        --rYear;
        if (rYear == 0)
            rYear = -1;
        rMonth = 12;
    }

    if (rYear < 0)
    {
        sal_uInt16 nDays;
        while (rDay > (nDays = ImplDaysInMonth( rMonth, rYear)))
        {
            rDay -= nDays;
            if (rMonth > 1)
                --rMonth;
            else
            {
                if (rYear == kYearMin)
                {
                    rDay = 1;
                    rMonth = 1;
                    return true;
                }
                --rYear;
                rMonth = 12;
            }
        }
    }
    else
    {
        sal_uInt16 nDays;
        while (rDay > (nDays = ImplDaysInMonth( rMonth, rYear)))
        {
            rDay -= nDays;
            if (rMonth < 12)
                ++rMonth;
            else
            {
                if (rYear == kYearMax)
                {
                    rDay = 31;
                    rMonth = 12;
                    return true;
                }
                ++rYear;
                rMonth = 1;
            }
        }
    }

    if (rDay == 0)
        rDay = ImplDaysInMonth( rMonth, rYear);

    return true;
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
