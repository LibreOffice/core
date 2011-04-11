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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#if defined( OS2 )
#define INCL_DOSDATETIME
#include <svpm.h>
#elif defined( WNT )
#include <windows.h>
#else
#include <time.h>
#endif

#include <tools/debug.hxx>
#include <tools/date.hxx>
#ifdef  MACOSX
extern "C" {
struct tm *localtime_r(const time_t *timep, struct tm *buffer);
}
#endif

// =======================================================================

static sal_uInt16 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };

#define MAX_DAYS    3636532

// =======================================================================

inline sal_Bool ImpIsLeapYear( sal_uInt16 nYear )
{
    return (
                 ( ((nYear % 4) == 0) && ((nYear % 100) != 0) ) ||
                 ( (nYear % 400) == 0 )
               );
}

// -----------------------------------------------------------------------

inline sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
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

// -----------------------------------------------------------------------

long Date::DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
{
    long nDays;

    nDays = ((sal_uIntPtr)nYear-1) * 365;
    nDays += ((nYear-1) / 4) - ((nYear-1) / 100) + ((nYear-1) / 400);
    for( sal_uInt16 i = 1; i < nMonth; i++ )
        nDays += DaysInMonth(i,nYear);
    nDays += nDay;
    return nDays;
}

// -----------------------------------------------------------------------

static void DaysToDate( long nDays,
                        sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear )
{
    long    nTempDays;
    long    i = 0;
    sal_Bool    bCalc;

    do
    {
        nTempDays = (long)nDays;
        rYear = (sal_uInt16)((nTempDays / 365) - i);
        nTempDays -= ((sal_uIntPtr)rYear-1) * 365;
        nTempDays -= ((rYear-1) / 4) - ((rYear-1) / 100) + ((rYear-1) / 400);
        bCalc = sal_False;
        if ( nTempDays < 1 )
        {
            i++;
            bCalc = sal_True;
        }
        else
        {
            if ( nTempDays > 365 )
            {
                if ( (nTempDays != 366) || !ImpIsLeapYear( rYear ) )
                {
                    i--;
                    bCalc = sal_True;
                }
            }
        }
    }
    while ( bCalc );

    rMonth = 1;
    while ( (sal_uIntPtr)nTempDays > DaysInMonth( rMonth, rYear ) )
    {
        nTempDays -= DaysInMonth( rMonth, rYear );
        rMonth++;
    }
    rDay = (sal_uInt16)nTempDays;
}

// =======================================================================

Date::Date()
{
#if defined( OS2 )
    DATETIME aDateTime;
    DosGetDateTime( &aDateTime );

    // Datum zusammenbauen
    nDate = ((sal_uIntPtr)aDateTime.day) +
            (((sal_uIntPtr)aDateTime.month)*100) +
            (((sal_uIntPtr)aDateTime.year)*10000);
#elif defined WNT
    SYSTEMTIME aDateTime;
    GetLocalTime( &aDateTime );

    // Datum zusammenbauen
    nDate = ((sal_uIntPtr)aDateTime.wDay) +
            (((sal_uIntPtr)aDateTime.wMonth)*100) +
            (((sal_uIntPtr)aDateTime.wYear)*10000);
#else
    time_t     nTmpTime;
    struct tm aTime;

    // Zeit ermitteln
    nTmpTime = time( 0 );

    // Datum zusammenbauen
    if ( localtime_r( &nTmpTime, &aTime ) )
    {
        nDate = ((sal_uIntPtr)aTime.tm_mday) +
                (((sal_uIntPtr)(aTime.tm_mon+1))*100) +
                (((sal_uIntPtr)(aTime.tm_year+1900))*10000);
    }
    else
        nDate = 1 + 100 + (((sal_uIntPtr)1900)*10000);
#endif
}

// -----------------------------------------------------------------------

void Date::SetDay( sal_uInt16 nNewDay )
{
    sal_uIntPtr  nMonth  = GetMonth();
    sal_uIntPtr  nYear   = GetYear();

    nDate = ((sal_uIntPtr)(nNewDay%100)) + (nMonth*100) + (nYear*10000);
}

// -----------------------------------------------------------------------

void Date::SetMonth( sal_uInt16 nNewMonth )
{
    sal_uIntPtr  nDay    = GetDay();
    sal_uIntPtr  nYear   = GetYear();

    nDate = nDay + (((sal_uIntPtr)(nNewMonth%100))*100) + (nYear*10000);
}

// -----------------------------------------------------------------------

void Date::SetYear( sal_uInt16 nNewYear )
{
    sal_uIntPtr  nDay   = GetDay();
    sal_uIntPtr  nMonth = GetMonth();

    nDate = nDay + (nMonth*100) + (((sal_uIntPtr)(nNewYear%10000))*10000);
}

// -----------------------------------------------------------------------

DayOfWeek Date::GetDayOfWeek() const
{
    return (DayOfWeek)((sal_uIntPtr)(DateToDays( GetDay(), GetMonth(), GetYear() )-1) % 7);
}

// -----------------------------------------------------------------------

sal_uInt16 Date::GetDayOfYear() const
{
    sal_uInt16 nDay = GetDay();
    for( sal_uInt16 i = 1; i < GetMonth(); i++ )
         nDay = nDay + ::DaysInMonth( i, GetYear() );   // += yields a warning on MSVC, so don't use it
    return nDay;
}

// -----------------------------------------------------------------------

sal_uInt16 Date::GetWeekOfYear( DayOfWeek eStartDay,
                            sal_Int16 nMinimumNumberOfDaysInWeek ) const
{
    short nWeek;
    short n1WDay = (short)Date( 1, 1, GetYear() ).GetDayOfWeek();
    short nDayOfYear = (short)GetDayOfYear();

    // Wochentage beginnen bei 0, deshalb einen abziehen
    nDayOfYear--;
    // StartDay beruecksichtigen
    n1WDay = (n1WDay+(7-(short)eStartDay)) % 7;

    if (nMinimumNumberOfDaysInWeek < 1 || 7 < nMinimumNumberOfDaysInWeek)
    {
        DBG_ERRORFILE("Date::GetWeekOfYear: invalid nMinimumNumberOfDaysInWeek");
        nMinimumNumberOfDaysInWeek = 4;
    }

    if ( nMinimumNumberOfDaysInWeek == 1 )
    {
        nWeek = ((n1WDay+nDayOfYear)/7) + 1;
        // 53te-Woche nur dann, wenn wir nicht schon in der ersten
        // Woche des neuen Jahres liegen
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
        // Erste Woche eines Jahres entspricht der letzen Woche des
        // vorherigen Jahres
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
        // friday
        else if ( n1WDay == nMinimumNumberOfDaysInWeek )
            nWeek = 53;
        // saturday
        else if ( n1WDay == nMinimumNumberOfDaysInWeek + 1 )
        {
            // Jahr nach Schaltjahr
            if ( Date( 1, 1, GetYear()-1 ).IsLeapYear() )
                nWeek = 53;
            else
                nWeek = 52;
        }
        // sunday
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
                // naechster x_Sonntag == erster x_Sonntag im neuen Jahr
                //                     == noch gleiche Woche
                long nTempDays = DateToDays( GetDay(), GetMonth(), GetYear() );
                nTempDays +=  6 - (GetDayOfWeek()+(7-(short)eStartDay)) % 7;
                sal_uInt16  nDay;
                sal_uInt16  nMonth;
                sal_uInt16  nYear;
                DaysToDate( nTempDays, nDay, nMonth, nYear );
                nWeek = Date( nDay, nMonth, nYear ).GetWeekOfYear( eStartDay, nMinimumNumberOfDaysInWeek );
            }
        }
    }

    return (sal_uInt16)nWeek;
}

// -----------------------------------------------------------------------

sal_uInt16 Date::GetDaysInMonth() const
{
    return DaysInMonth( GetMonth(), GetYear() );
}

// -----------------------------------------------------------------------

sal_Bool Date::IsLeapYear() const
{
    sal_uInt16 nYear = GetYear();
    return ImpIsLeapYear( nYear );
}

// -----------------------------------------------------------------------

sal_Bool Date::IsValid() const
{
    sal_uInt16 nDay   = GetDay();
    sal_uInt16 nMonth = GetMonth();
    sal_uInt16 nYear  = GetYear();

    if ( !nMonth || (nMonth > 12) )
        return sal_False;
    if ( !nDay || (nDay > DaysInMonth( nMonth, nYear )) )
        return sal_False;
    else if ( nYear <= 1582 )
    {
        if ( nYear < 1582 )
            return sal_False;
        else if ( nMonth < 10 )
            return sal_False;
        else if ( (nMonth == 10) && (nDay < 15) )
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

Date& Date::operator +=( long nDays )
{
    sal_uInt16  nDay;
    sal_uInt16  nMonth;
    sal_uInt16  nYear;
    long    nTempDays = DateToDays( GetDay(), GetMonth(), GetYear() );

    nTempDays += nDays;
    if ( nTempDays > MAX_DAYS )
        nDate = 31 + (12*100) + (((sal_uIntPtr)9999)*10000);
    else if ( nTempDays <= 0 )
        nDate = 1 + 100;
    else
    {
        DaysToDate( nTempDays, nDay, nMonth, nYear );
        nDate = ((sal_uIntPtr)nDay) + (((sal_uIntPtr)nMonth)*100) + (((sal_uIntPtr)nYear)*10000);
    }

    return *this;
}

// -----------------------------------------------------------------------

Date& Date::operator -=( long nDays )
{
    sal_uInt16  nDay;
    sal_uInt16  nMonth;
    sal_uInt16  nYear;
    long    nTempDays = DateToDays( GetDay(), GetMonth(), GetYear() );

    nTempDays -= nDays;
    if ( nTempDays > MAX_DAYS )
        nDate = 31 + (12*100) + (((sal_uIntPtr)9999)*10000);
    else if ( nTempDays <= 0 )
        nDate = 1 + 100;
    else
    {
        DaysToDate( nTempDays, nDay, nMonth, nYear );
        nDate = ((sal_uIntPtr)nDay) + (((sal_uIntPtr)nMonth)*100) + (((sal_uIntPtr)nYear)*10000);
    }

    return *this;
}

// -----------------------------------------------------------------------

Date& Date::operator ++()
{
    sal_uInt16  nDay;
    sal_uInt16  nMonth;
    sal_uInt16  nYear;
    long    nTempDays = DateToDays( GetDay(), GetMonth(), GetYear() );

    if ( nTempDays < MAX_DAYS )
    {
        nTempDays++;
        DaysToDate( nTempDays, nDay, nMonth, nYear );
        nDate = ((sal_uIntPtr)nDay) + (((sal_uIntPtr)nMonth)*100) + (((sal_uIntPtr)nYear)*10000);
    }

    return *this;
}

// -----------------------------------------------------------------------

Date& Date::operator --()
{
    sal_uInt16  nDay;
    sal_uInt16  nMonth;
    sal_uInt16  nYear;
    long    nTempDays = DateToDays( GetDay(), GetMonth(), GetYear() );

    if ( nTempDays > 1 )
    {
        nTempDays--;
        DaysToDate( nTempDays, nDay, nMonth, nYear );
        nDate = ((sal_uIntPtr)nDay) + (((sal_uIntPtr)nMonth)*100) + (((sal_uIntPtr)nYear)*10000);
    }
    return *this;
}

#ifndef MPW33

// -----------------------------------------------------------------------

Date Date::operator ++( int )
{
    Date aOldDate = *this;
    Date::operator++();
    return aOldDate;
}

// -----------------------------------------------------------------------

Date Date::operator --( int )
{
    Date aOldDate = *this;
    Date::operator--();
    return aOldDate;
}

#endif

// -----------------------------------------------------------------------

Date operator +( const Date& rDate, long nDays )
{
    Date aDate( rDate );
    aDate += nDays;
    return aDate;
}

// -----------------------------------------------------------------------

Date operator -( const Date& rDate, long nDays )
{
    Date aDate( rDate );
    aDate -= nDays;
    return aDate;
}

// -----------------------------------------------------------------------

long operator -( const Date& rDate1, const Date& rDate2 )
{
    sal_uIntPtr  nTempDays1 = Date::DateToDays( rDate1.GetDay(), rDate1.GetMonth(),
                                    rDate1.GetYear() );
    sal_uIntPtr  nTempDays2 = Date::DateToDays( rDate2.GetDay(), rDate2.GetMonth(),
                                    rDate2.GetYear() );
    return nTempDays1 - nTempDays2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
