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

#if defined( WNT )
#include <windows.h>
#else
#include <time.h>
#endif

#include <tools/date.hxx>
#include <sal/log.hxx>

static const sal_uInt16 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                             31, 31, 30, 31, 30, 31 };

#define MAX_DAYS    3636532

namespace
{

inline long ImpYearToDays( sal_uInt16 nYear )
{
    const long nYr(static_cast<long>(nYear) - 1);
    return nYr*365 + nYr/4 - nYr/100 + nYr/400;
}

inline bool ImpIsLeapYear( sal_uInt16 nYear )
{
    return ( ( ((nYear % 4) == 0) && ((nYear % 100) != 0) ) ||
             ( (nYear % 400) == 0 ) );
}

// All callers must have sanitized or normalized month and year values!
inline sal_uInt16 ImplDaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
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

// static
sal_uInt16 Date::GetDaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{
    SAL_WARN_IF( nMonth < 1 || 12 < nMonth, "tools", "Date::GetDaysInMonth - nMonth out of bounds " << nMonth);
    if (nMonth < 1)
        nMonth = 1;
    else if (12 < nMonth)
        nMonth = 12;
    return ImplDaysInMonth( nMonth, nYear);
}

long Date::GetAsNormalizedDays() const
{
    // This is a very common datum we often calculate from.
    if (nDate == 18991230) // 1899-12-30
    {
        assert(DateToDays( GetDay(), GetMonth(), GetYear() ) == 693594);
        return 693594;
    }
    return DateToDays( GetDay(), GetMonth(), GetYear() );
}

long Date::DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
{
    Normalize( nDay, nMonth, nYear);

    long nDays = ImpYearToDays(nYear);
    for( sal_uInt16 i = 1; i < nMonth; i++ )
        nDays += ImplDaysInMonth(i,nYear);
    nDays += nDay;
    return nDays;
}

static Date lcl_DaysToDate( long nDays )
{
    if ( nDays >= MAX_DAYS )
        return Date( 31, 12, 9999 );

    if ( nDays <= 0 )
        return Date( 1, 0, 0 );

    long    nTempDays;
    long    i = 0;
    bool    bCalc;

    sal_uInt16 nYear;
    do
    {
        nYear = (sal_uInt16)((nDays / 365) - i);
        nTempDays = nDays - ImpYearToDays(nYear);
        bCalc = false;
        if ( nTempDays < 1 )
        {
            i++;
            bCalc = true;
        }
        else
        {
            if ( nTempDays > 365 )
            {
                if ( (nTempDays != 366) || !ImpIsLeapYear( nYear ) )
                {
                    i--;
                    bCalc = true;
                }
            }
        }
    }
    while ( bCalc );

    sal_uInt16 nMonth = 1;
    while ( nTempDays > static_cast<long>(ImplDaysInMonth( nMonth, nYear )) )
    {
        nTempDays -= ImplDaysInMonth( nMonth, nYear );
        nMonth++;
    }

    return Date( static_cast<sal_uInt16>(nTempDays), nMonth, nYear );
}

Date::Date( DateInitSystem )
{
#if defined WNT
    SYSTEMTIME aDateTime;
    GetLocalTime( &aDateTime );

    // Combine to date
    setDateFromDMY( aDateTime.wDay, aDateTime.wMonth, aDateTime.wYear );
#else
    time_t     nTmpTime;
    struct tm aTime;

    // get current time
    nTmpTime = time( 0 );

    // compute date
    if ( localtime_r( &nTmpTime, &aTime ) )
    {
        setDateFromDMY( static_cast<sal_uInt16>(aTime.tm_mday),
            static_cast<sal_uInt16>(aTime.tm_mon+1),
            static_cast<sal_uInt16>(aTime.tm_year+1900) );
    }
    else
        setDateFromDMY( 1, 100, 1900 );
#endif
}

Date::Date( const ::com::sun::star::util::DateTime& rDateTime )
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

void Date::SetYear( sal_uInt16 nNewYear )
{
    setDateFromDMY( GetDay(), GetMonth(), nNewYear );
}

DayOfWeek Date::GetDayOfWeek() const
{
    return static_cast<DayOfWeek>((GetAsNormalizedDays()-1) % 7);
}

sal_uInt16 Date::GetDayOfYear() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_uInt16 nYear  = GetYear();
    Normalize( nDay, nMonth, nYear);

    for( sal_uInt16 i = 1; i < nMonth; i++ )
         nDay = nDay + ::ImplDaysInMonth( i, nYear );   // += yields a warning on MSVC, so don't use it
    return nDay;
}

sal_uInt16 Date::GetWeekOfYear( DayOfWeek eStartDay,
                                sal_Int16 nMinimumNumberOfDaysInWeek ) const
{
    short nWeek;
    short n1WDay = (short)Date( 1, 1, GetYear() ).GetDayOfWeek();
    short nDayOfYear = (short)GetDayOfYear();

    // weekdays start at 0, thus decrement one
    nDayOfYear--;
    // account for StartDay
    n1WDay = (n1WDay+(7-(short)eStartDay)) % 7;

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
            short nDaysInYear = (short)GetDaysInYear();
            short nDaysNextYear = (short)Date( 1, 1, GetYear()+1 ).GetDayOfWeek();
            nDaysNextYear = (nDaysNextYear+(7-(short)eStartDay)) % 7;
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
            Date aLastDatePrevYear( 31, 12, GetYear()-1 );
            nWeek = aLastDatePrevYear.GetWeekOfYear( eStartDay, nMinimumNumberOfDaysInWeek );
        }
    }
    else // ( nMinimumNumberOfDaysInWeek == somehing_else, commentary examples for 4 )
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
            // Year after leapyear
            if ( Date( 1, 1, GetYear()-1 ).IsLeapYear() )
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
                long nTempDays = GetAsNormalizedDays();

                nTempDays +=  6 - (GetDayOfWeek()+(7-(short)eStartDay)) % 7;
                nWeek = lcl_DaysToDate( nTempDays ).GetWeekOfYear( eStartDay, nMinimumNumberOfDaysInWeek );
            }
        }
    }

    return (sal_uInt16)nWeek;
}

sal_uInt16 Date::GetDaysInMonth() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_uInt16 nYear  = GetYear();
    Normalize( nDay, nMonth, nYear);

    return ImplDaysInMonth( nMonth, nYear );
}

bool Date::IsLeapYear() const
{
    sal_uInt16 nYear = GetYear();
    return ImpIsLeapYear( nYear );
}

bool Date::IsValidAndGregorian() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_uInt16 nYear  = GetYear();

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
bool Date::IsValidDate( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
{
    if ( !nMonth || (nMonth > 12) )
        return false;
    if ( !nDay || (nDay > ImplDaysInMonth( nMonth, nYear )) )
        return false;
    return true;
}

bool Date::Normalize()
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_uInt16 nYear  = GetYear();

    if (!Normalize( nDay, nMonth, nYear))
        return false;

    setDateFromDMY( nDay, nMonth, nYear );

    return true;
}

//static
bool Date::Normalize( sal_uInt16 & rDay, sal_uInt16 & rMonth, sal_uInt16 & rYear )
{
    if (IsValidDate( rDay, rMonth, rYear))
        return false;

    if (rMonth > 12)
    {
        rYear += rMonth / 12;
        rMonth = rMonth % 12;
    }
    if (!rMonth)
    {
        if (!rYear)
        {
            rYear = 0;
            rMonth = 1;
            if (rDay > 31)
                rDay -= 31;
            else
                rDay = 1;
        }
        else
        {
            --rYear;
            rMonth = 12;
        }
    }
    sal_uInt16 nDays;
    while (rDay > (nDays = ImplDaysInMonth( rMonth, rYear)))
    {
        rDay -= nDays;
        if (rMonth < 12)
            ++rMonth;
        else
        {
            ++rYear;
            rMonth = 1;
        }
    }
    if (rYear > 9999)
    {
        rDay = 31;
        rMonth = 12;
        rYear = 9999;
    }
    return true;
}

Date& Date::operator +=( long nDays )
{
    if (nDays != 0)
        *this = lcl_DaysToDate( GetAsNormalizedDays() + nDays );

    return *this;
}

Date& Date::operator -=( long nDays )
{
    if (nDays != 0)
        *this = lcl_DaysToDate( GetAsNormalizedDays() - nDays );

    return *this;
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

Date Date::operator ++( int )
{
    Date aOldDate = *this;
    Date::operator++();
    return aOldDate;
}

Date Date::operator --( int )
{
    Date aOldDate = *this;
    Date::operator--();
    return aOldDate;
}

Date operator +( const Date& rDate, long nDays )
{
    Date aDate( rDate );
    aDate += nDays;
    return aDate;
}

Date operator -( const Date& rDate, long nDays )
{
    Date aDate( rDate );
    aDate -= nDays;
    return aDate;
}

long operator -( const Date& rDate1, const Date& rDate2 )
{
    return rDate1.GetAsNormalizedDays() - rDate2.GetAsNormalizedDays();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
