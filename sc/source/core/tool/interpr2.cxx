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

#include "interpre.hxx"

#include <comphelper/string.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
#include <sal/macros.h>

#include "attrib.hxx"
#include "sc.hrc"
#include "ddelink.hxx"
#include "scmatrix.hxx"
#include "compiler.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
#include "unitconv.hxx"
#include "globstr.hrc"
#include "hints.hxx"
#include "dpobject.hxx"
#include "postit.hxx"
#include "tokenarray.hxx"
#include "globalnames.hxx"

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include <string.h>
#include <math.h>

using ::std::vector;
using namespace com::sun::star;
using namespace formula;

#define SCdEpsilon                1.0E-7

// Datum und Zeit

double ScInterpreter::GetDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay,
        bool bStrict, bool bCheckGregorian )
{
    if ( nYear < 100 && !bStrict )
        nYear = pFormatter->ExpandTwoDigitYear( nYear );
    // Do not use a default Date ctor here because it asks system time with a
    // performance penalty.
    sal_Int16 nY, nM, nD;
    if (bStrict)
        nY = nYear, nM = nMonth, nD = nDay;
    else
    {
        if (nMonth > 0)
        {
            nY = nYear + (nMonth-1) / 12;
            nM = ((nMonth-1) % 12) + 1;
        }
        else
        {
            nY = nYear + (nMonth-12) / 12;
            nM = 12 - (-nMonth) % 12;
        }
        nD = 1;
    }
    Date aDate( nD, nM, nY);
    if (!bStrict)
        aDate += nDay - 1;
    if ((!bCheckGregorian && aDate.IsValidDate()) || (bCheckGregorian && aDate.IsValidAndGregorian()))
        return (double) (aDate - *(pFormatter->GetNullDate()));
    else
    {
        SetError(errNoValue);
        return 0;
    }
}

// Funktionen

void ScInterpreter::ScGetActDate()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    Date aActDate( Date::SYSTEM );
    long nDiff = aActDate - *(pFormatter->GetNullDate());
    PushDouble((double) nDiff);
}

void ScInterpreter::ScGetActTime()
{
    nFuncFmtType = NUMBERFORMAT_DATETIME;
    Date aActDate( Date::SYSTEM );
    long nDiff = aActDate - *(pFormatter->GetNullDate());
    Time aActTime( Time::SYSTEM );
    double nTime = aActTime.GetHour()    / static_cast<double>(::Time::hourPerDay)   +
                   aActTime.GetMin()     / static_cast<double>(::Time::minutePerDay) +
                   aActTime.GetSec()     / static_cast<double>(::Time::secondPerDay) +
                   aActTime.GetNanoSec() / static_cast<double>(::Time::nanoSecPerDay);
    PushDouble( (double) nDiff + nTime );
}

void ScInterpreter::ScGetYear()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) ::rtl::math::approxFloor(GetDouble());
    PushDouble( (double) aDate.GetYear() );
}

void ScInterpreter::ScGetMonth()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) ::rtl::math::approxFloor(GetDouble());
    PushDouble( (double) aDate.GetMonth() );
}

void ScInterpreter::ScGetDay()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long)::rtl::math::approxFloor(GetDouble());
    PushDouble((double) aDate.GetDay());
}

void ScInterpreter::ScGetMin()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*DATE_TIME_FACTOR+0.5) % ::Time::secondPerHour;
    PushDouble( (double) (nVal / ::Time::secondPerMinute) );
}

void ScInterpreter::ScGetSec()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*DATE_TIME_FACTOR+0.5) % ::Time::secondPerMinute;
    PushDouble( (double) nVal );
}

void ScInterpreter::ScGetHour()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*DATE_TIME_FACTOR+0.5) / ::Time::secondPerHour;
    PushDouble((double) nVal);
}

void ScInterpreter::ScGetDateValue()
{
    OUString aInputString = GetString().getString();
    sal_uInt32 nFIndex = 0;                 // damit default Land/Spr.
    double fVal;
    if (pFormatter->IsNumberFormat(aInputString, nFIndex, fVal))
    {
        short eType = pFormatter->GetType(nFIndex);
        if (eType == NUMBERFORMAT_DATE || eType == NUMBERFORMAT_DATETIME)
            PushDouble(::rtl::math::approxFloor(fVal));
        else
            PushIllegalArgument();
    }
    else
        PushIllegalArgument();
}

void ScInterpreter::ScGetDayOfWeek()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        short nFlag;
        if (nParamCount == 2)
            nFlag = (short) ::rtl::math::approxFloor(GetDouble());
        else
            nFlag = 1;

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)::rtl::math::approxFloor(GetDouble());
        int nVal = (int) aDate.GetDayOfWeek();
        if (nFlag == 1)
        {
            if (nVal == 6)
                nVal = 1;
            else
                nVal += 2;
        }
        else if (nFlag == 2)
            nVal += 1;
        PushInt( nVal );
    }
}

void ScInterpreter::ScGetWeekOfYear()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        short nFlag = (short) ::rtl::math::approxFloor(GetDouble());

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)::rtl::math::approxFloor(GetDouble());
        PushInt( (int) aDate.GetWeekOfYear( nFlag == 1 ? SUNDAY : MONDAY ));
    }
}

void ScInterpreter::ScEasterSunday()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        sal_Int16 nDay, nMonth, nYear;
        nYear = (sal_Int16) ::rtl::math::approxFloor( GetDouble() );
        if ( nYear < 100 )
            nYear = pFormatter->ExpandTwoDigitYear( nYear );
        // don't worry, be happy :)
        int B,C,D,E,F,G,H,I,K,L,M,N,O;
        N = nYear % 19;
        B = int(nYear / 100);
        C = nYear % 100;
        D = int(B / 4);
        E = B % 4;
        F = int((B + 8) / 25);
        G = int((B - F + 1) / 3);
        H = (19 * N + B - D - G + 15) % 30;
        I = int(C / 4);
        K = C % 4;
        L = (32 + 2 * E + 2 * I - H - K) % 7;
        M = int((N + 11 * H + 22 * L) / 451);
        O = H + L - 7 * M + 114;
        nDay = sal::static_int_cast<sal_Int16>( O % 31 + 1 );
        nMonth = sal::static_int_cast<sal_Int16>( int(O / 31) );
        PushDouble( GetDateSerial( nYear, nMonth, nDay, true, true ) );
    }
}

sal_uInt16 ScInterpreter::GetWeekendAndHolidayMasks(
    const sal_uInt8 nParamCount, const sal_uInt32 nNullDate, vector< double >& rSortArray,
    OUString& rWeekendDays, bool bWeekendMask[ 7 ] )
{
    sal_uInt16 nErr = 0;
    if ( nParamCount == 4 )
    {
        GetSortArray( 1, rSortArray );
        size_t nMax = rSortArray.size();
        for ( size_t i = 0; i < nMax; i++ )
            rSortArray.at( i ) = ::rtl::math::approxFloor( rSortArray.at( i ) ) + nNullDate;
    }

    if ( nParamCount >= 3 )
        rWeekendDays = GetString().getString();

    for ( int i = 0; i < 7; i++ )
        bWeekendMask[ i] = false;

    if ( rWeekendDays.isEmpty() )
    {
        bWeekendMask[ SATURDAY ] = true;
        bWeekendMask[ SUNDAY ]   = true;
    }
    else
    {
        switch ( rWeekendDays.getLength() )
        {
            case 1 :
                // Weekend days defined by code
                switch ( rWeekendDays[ 0 ] )
                {
                    case '1' : bWeekendMask[ SATURDAY ]  = true; bWeekendMask[ SUNDAY ]    = true; break;
                    case '2' : bWeekendMask[ SUNDAY ]    = true; bWeekendMask[ MONDAY ]    = true; break;
                    case '3' : bWeekendMask[ MONDAY ]    = true; bWeekendMask[ TUESDAY ]   = true; break;
                    case '4' : bWeekendMask[ TUESDAY ]   = true; bWeekendMask[ WEDNESDAY ] = true; break;
                    case '5' : bWeekendMask[ WEDNESDAY ] = true; bWeekendMask[ THURSDAY ]  = true; break;
                    case '6' : bWeekendMask[ THURSDAY ]  = true; bWeekendMask[ FRIDAY ]    = true; break;
                    case '7' : bWeekendMask[ FRIDAY ]    = true; bWeekendMask[ SATURDAY ]  = true; break;
                    default  : nErr = errIllegalArgument;                                          break;
                }
                break;
            case 2 :
                // Weekend day defined by code
                if ( rWeekendDays[ 0 ] == '1' )
                {
                    switch ( rWeekendDays[ 1 ] )
                    {
                        case '1' : bWeekendMask[ SUNDAY ]    = true; break;
                        case '2' : bWeekendMask[ MONDAY ]    = true; break;
                        case '3' : bWeekendMask[ TUESDAY ]   = true; break;
                        case '4' : bWeekendMask[ WEDNESDAY ] = true; break;
                        case '5' : bWeekendMask[ THURSDAY ]  = true; break;
                        case '6' : bWeekendMask[ FRIDAY ]    = true; break;
                        case '7' : bWeekendMask[ SATURDAY ]  = true; break;
                        default  : nErr = errIllegalArgument;        break;
                    }
                }
                else
                    nErr = errIllegalArgument;
                break;
            case 7 :
                // Weekend days defined by string
                for ( int i = 0; i < 7 && !nErr; i++ )
                {
                    switch ( rWeekendDays[ i ] )
                    {
                        case '0' : bWeekendMask[ i ] = false; break;
                        case '1' : bWeekendMask[ i ] = true;  break;
                        default  : nErr = errIllegalArgument; break;
                    }
                }
                break;
            default :
                nErr = errIllegalArgument;
                break;
        }
    }
    return nErr;
}

void ScInterpreter::ScNetWorkdays_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 4 ) )
    {
        vector<double> nSortArray;
        bool bWeekendMask[ 7 ];
        OUString aWeekendDays;
        Date aNullDate = *( pFormatter->GetNullDate() );
        sal_uInt32 nNullDate = Date::DateToDays( aNullDate.GetDay(), aNullDate.GetMonth(), aNullDate.GetYear() );
        sal_uInt16 nErr = GetWeekendAndHolidayMasks( nParamCount, nNullDate,
                            nSortArray , aWeekendDays, bWeekendMask );
        if ( nErr )
            PushError( nErr );
        else
        {
            sal_uInt32 nDate2 = ( sal_uInt32 )::rtl::math::approxFloor( GetDouble() ) + nNullDate;
            sal_uInt32 nDate1 = ( sal_uInt32 )::rtl::math::approxFloor( GetDouble() ) + nNullDate;

            sal_Int32 nCnt = 0;
            size_t nRef = 0;
            bool bReverse = ( nDate1 > nDate2 );
            if ( bReverse )
            {
                sal_uInt32 nTemp = nDate1;
                nDate1 = nDate2;
                nDate2 = nTemp;
            }
            size_t nMax = nSortArray.size();
            while ( nDate1 <= nDate2 )
            {
                if ( !bWeekendMask[ GetDayOfWeek( nDate1 ) ] )
                {
                    while ( nRef < nMax && nSortArray.at( nRef ) < nDate1 )
                        nRef++;
                    if ( !( nRef < nMax && nSortArray.at( nRef ) == nDate1 ) )
                        nCnt++;
                }
                ++nDate1;
            }
            PushDouble( ( double ) ( bReverse ? -nCnt : nCnt ) );
        }
    }
}

void ScInterpreter::ScWorkday_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 4 ) )
    {
        nFuncFmtType = NUMBERFORMAT_DATE;
        vector<double> nSortArray;
        bool bWeekendMask[ 7 ];
        OUString aWeekendDays;
        Date aNullDate = *( pFormatter->GetNullDate() );
        sal_uInt32 nNullDate = Date::DateToDays( aNullDate.GetDay(), aNullDate.GetMonth(), aNullDate.GetYear() );
        sal_uInt16 nErr = GetWeekendAndHolidayMasks( nParamCount, nNullDate,
                            nSortArray , aWeekendDays, bWeekendMask );
        if ( nErr )
            PushError( nErr );
        else
        {
            sal_Int32 nDays = ::rtl::math::approxFloor( GetDouble() );
            sal_uInt32 nDate = ( sal_uInt32 )::rtl::math::approxFloor( GetDouble() ) + nNullDate;

            if ( !nDays )
                PushDouble( ( double ) ( nDate - nNullDate ) );
            else
            {
                size_t nMax = nSortArray.size();
                if ( nDays > 0 )
                {
                    size_t nRef = 0;
                    while ( nDays )
                    {
                        while ( nRef < nMax && nSortArray.at( nRef ) < nDate )
                            nRef++;
                        if ( !( nRef < nMax && nSortArray.at( nRef ) == nDate ) || nRef >= nMax )
                             nDays--;

                        do
                            ++nDate;
                        while ( bWeekendMask[ GetDayOfWeek( nDate ) ] ); //jump over weekend day(s)
                    }
                }
                else
                {
                    sal_Int16 nRef = nMax - 1;
                    while ( nDays )
                    {
                        while ( nRef >= 0 && nSortArray.at( nRef ) > nDate )
                            nRef--;
                        if ( !( nRef >= 0 && nSortArray.at( nRef ) == nDate ) || nRef < 0 )
                             nDays++;

                        do
                          --nDate;
                        while ( bWeekendMask[ GetDayOfWeek( nDate ) ] ); //jump over weekend day(s)
                    }
                }
                PushDouble( ( double ) ( nDate - nNullDate ) );
            }
        }
    }
}

void ScInterpreter::ScGetDate()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        sal_Int16 nDay   = (sal_Int16) ::rtl::math::approxFloor(GetDouble());
        sal_Int16 nMonth = (sal_Int16) ::rtl::math::approxFloor(GetDouble());
        sal_Int16 nYear  = (sal_Int16) ::rtl::math::approxFloor(GetDouble());
        if (nYear < 0)
            PushIllegalArgument();
        else
        {
            PushDouble(GetDateSerial(nYear, nMonth, nDay, false, true));
        }
    }
}

void ScInterpreter::ScGetTime()
{
    nFuncFmtType = NUMBERFORMAT_TIME;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nSec = GetDouble();
        double nMin = GetDouble();
        double nHour = GetDouble();
        double fTime = fmod( (nHour * ::Time::secondPerHour) + (nMin * ::Time::secondPerMinute) + nSec, DATE_TIME_FACTOR) / DATE_TIME_FACTOR;
        if (fTime < 0)
            PushIllegalArgument();
        else
            PushDouble( fTime);
    }
}

void ScInterpreter::ScGetDiffDate()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double nDate2 = GetDouble();
        double nDate1 = GetDouble();
        PushDouble(nDate1 - nDate2);
    }
}

void ScInterpreter::ScGetDiffDate360()
{
    /* Implementation follows
     * http://www.bondmarkets.com/eCommerce/SMD_Fields_030802.pdf
     * Appendix B: Day-Count Bases, there are 7 different ways to calculate the
     * 30-days count. That document also claims that Excel implements the "PSA
     * 30" or "NASD 30" method (funny enough they also state that Excel is the
     * only tool that does so).
     *
     * Note that the definition given in
     * http://msdn.microsoft.com/library/en-us/office97/html/SEB7C.asp
     * is _not_ the way how it is actually calculated by Excel (that would not
     * even match any of the 7 methods mentioned above) and would result in the
     * following test cases producing wrong results according to that appendix B:
     *
     * 28-Feb-95  31-Aug-95  181 instead of 180
     * 29-Feb-96  31-Aug-96  181 instead of 180
     * 30-Jan-96  31-Mar-96   61 instead of  60
     * 31-Jan-96  31-Mar-96   61 instead of  60
     *
     * Still, there is a difference between OOoCalc and Excel:
     * In Excel:
     * 02-Feb-99 31-Mar-00 results in  419
     * 31-Mar-00 02-Feb-99 results in -418
     * In Calc the result is 419 respectively -419. I consider the -418 a bug in Excel.
     */

    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        bool bFlag;
        if (nParamCount == 3)
            bFlag = GetBool();
        else
            bFlag = false;
        double nDate2 = GetDouble();
        double nDate1 = GetDouble();
        if (nGlobalError)
            PushError( nGlobalError);
        else
        {
            double fSign;
            // #i84934# only for non-US European algorithm swap dates. Else
            // follow Excel's meaningless extrapolation for "interoperability".
            if (bFlag && (nDate2 < nDate1))
            {
                fSign = nDate1;
                nDate1 = nDate2;
                nDate2 = fSign;
                fSign = -1.0;
            }
            else
                fSign = 1.0;
            Date aDate1 = *(pFormatter->GetNullDate());
            aDate1 += (long) ::rtl::math::approxFloor(nDate1);
            Date aDate2 = *(pFormatter->GetNullDate());
            aDate2 += (long) ::rtl::math::approxFloor(nDate2);
            if (aDate1.GetDay() == 31)
                aDate1 -= (sal_uLong) 1;
            else if (!bFlag)
            {
                if (aDate1.GetMonth() == 2)
                {
                    switch ( aDate1.GetDay() )
                    {
                        case 28 :
                            if ( !aDate1.IsLeapYear() )
                                aDate1.SetDay(30);
                        break;
                        case 29 :
                            aDate1.SetDay(30);
                        break;
                    }
                }
            }
            if (aDate2.GetDay() == 31)
            {
                if (!bFlag )
                {
                    if (aDate1.GetDay() == 30)
                        aDate2 -= (sal_uLong) 1;
                }
                else
                    aDate2.SetDay(30);
            }
            PushDouble( fSign * (double)
                (  (double) aDate2.GetDay() + (double) aDate2.GetMonth() * 30.0 +
                   (double) aDate2.GetYear() * 360.0
                 - (double) aDate1.GetDay() - (double) aDate1.GetMonth() * 30.0
                 - (double)aDate1.GetYear() * 360.0) );
        }
    }
}

// fdo#44456 function DATEDIF as defined in ODF1.2 (Par. 6.10.3)
void ScInterpreter::ScGetDateDif()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        OUString aInterval = GetString().getString();
        double nDate2    = GetDouble();
        double nDate1    = GetDouble();

        if (nGlobalError)
        {
            PushError( nGlobalError);
            return;
        }

        // Excel doesn't swap dates or return negative numbers, so don't we.
        if (nDate1 > nDate2)
        {
            PushIllegalArgument();
            return;
        }

        long dd = nDate2 - nDate1;
        // Zero difference or number of days can be returned immediately.
        if (dd == 0 || aInterval.equalsIgnoreAsciiCase( "d" ))
        {
            PushDouble( dd );
            return;
        }

        // split dates in day, month, year for use with formats other than "d"
        sal_uInt16 d1, m1, y1, d2, m2, y2;
        Date aDate1( *( pFormatter->GetNullDate()));
        aDate1 += (long) ::rtl::math::approxFloor( nDate1 );
        y1 = aDate1.GetYear();
        m1 = aDate1.GetMonth();
        d1 = aDate1.GetDay();
        Date aDate2( *( pFormatter->GetNullDate()));
        aDate2 += (long) ::rtl::math::approxFloor( nDate2 );
        y2 = aDate2.GetYear();
        m2 = aDate2.GetMonth();
        d2 = aDate2.GetDay();

        if (  aInterval.equalsIgnoreAsciiCase( "m" ) )
        {
            // Return number of months.
            int md = m2 - m1 + 12 * (y2 - y1);
            if (d1 > d2)
                --md;
            PushInt( md );
        }
        else if ( aInterval.equalsIgnoreAsciiCase( "y" ) )
        {
            // Return number of years.
            int yd;
            if ( y2 > y1 )
            {
                if (m2 > m1 || (m2 == m1 && d2 >= d1))
                    yd = y2 - y1;       // complete years between dates
                else
                    yd = y2 - y1 - 1;   // one incomplete year
            }
            else
            {
                // Year is equal as we don't allow reversed arguments, no
                // complete year between dates.
                yd = 0;
            }
            PushInt( yd );
        }
        else if ( aInterval.equalsIgnoreAsciiCase( "md" ) )
        {
            // Return number of days, excluding months and years.
            // This is actually the remainder of days when subtracting years
            // and months from the difference of dates. Birthday-like 23 years
            // and 10 months and 19 days.

            // Algorithm's roll-over behavior extracted from Excel by try and
            // error..
            // If day1 <= day2 then simply day2 - day1.
            // If day1 > day2 then set month1 to month2-1 and year1 to
            // year2(-1) and subtract dates, e.g. for 2012-01-28,2012-03-01 set
            // 2012-02-28 and then (2012-03-01)-(2012-02-28) => 2 days (leap
            // year).
            // For 2011-01-29,2011-03-01 the non-existent 2011-02-29 rolls over
            // to 2011-03-01 so the result is 0. Same for day 31 in months with
            // only 30 days.

            long nd;
            if (d1 <= d2)
                nd = d2 - d1;
            else
            {
                if (m2 == 1)
                {
                    aDate1.SetYear( y2 - 1 );
                    aDate1.SetMonth( 12 );
                }
                else
                {
                    aDate1.SetYear( y2 );
                    aDate1.SetMonth( m2 - 1 );
                }
                aDate1.Normalize();
                nd = aDate2 - aDate1;
            }
            PushDouble( nd );
        }
        else if ( aInterval.equalsIgnoreAsciiCase( "ym" ) )
        {
            // Return number of months, excluding years.
            int md = m2 - m1 + 12 * (y2 - y1);
            if (d1 > d2)
                --md;
            md %= 12;
            PushInt( md );
        }
        else if ( aInterval.equalsIgnoreAsciiCase( "yd" ) )
        {
            // Return number of days, excluding years.

            /* TODO: check what Excel really does, though this seems to be
             * reasonable */

            // Condition corresponds with "y".
            if (m2 > m1 || (m2 == m1 && d2 >= d1))
                aDate1.SetYear( y2 );
            else
                aDate1.SetYear( y2 - 1 );
                // XXX NOTE: Excel for the case 1988-06-22,2012-05-11 returns
                // 323, whereas the result here is 324. Don't they use the leap
                // year of 2012?
                // http://www.cpearson.com/excel/datedif.aspx "DATEDIF And Leap
                // Years" is not correct and Excel 2010 correctly returns 0 in
                // both cases mentioned there. Also using year1 as mentioned
                // produces incorrect results in other cases and different from
                // Excel 2010. Apparently they fixed some calculations.
            aDate1.Normalize();
            double nd = aDate2 - aDate1;
            PushDouble( nd );
        }
        else
            PushIllegalArgument();               // unsupported format
    }
}

void ScInterpreter::ScGetTimeValue()
{
    OUString aInputString = GetString().getString();
    sal_uInt32 nFIndex = 0;                 // damit default Land/Spr.
    double fVal;
    if (pFormatter->IsNumberFormat(aInputString, nFIndex, fVal))
    {
        short eType = pFormatter->GetType(nFIndex);
        if (eType == NUMBERFORMAT_TIME || eType == NUMBERFORMAT_DATETIME)
        {
            double fDateVal = rtl::math::approxFloor(fVal);
            double fTimeVal = fVal - fDateVal;
            PushDouble(fTimeVal);
        }
        else
            PushIllegalArgument();
    }
    else
        PushIllegalArgument();
}

void ScInterpreter::ScPlusMinus()
{
    double nVal = GetDouble();
    short n = 0;
    if (nVal < 0.0)
        n = -1;
    else if (nVal > 0.0)
        n = 1;
    PushInt( n );
}

void ScInterpreter::ScAbs()
{
    PushDouble(fabs(GetDouble()));
}

void ScInterpreter::ScInt()
{
    PushDouble(::rtl::math::approxFloor(GetDouble()));
}

void ScInterpreter::RoundNumber( rtl_math_RoundingMode eMode )
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fVal = 0.0;
        if (nParamCount == 1)
            fVal = ::rtl::math::round( GetDouble(), 0, eMode );
        else
        {
            sal_Int32 nDec = (sal_Int32) ::rtl::math::approxFloor(GetDouble());
            if( nDec < -20 || nDec > 20 )
                PushIllegalArgument();
            else
                fVal = ::rtl::math::round( GetDouble(), (short)nDec, eMode );
        }
        PushDouble(fVal);
    }
}

void ScInterpreter::ScRound()
{
    RoundNumber( rtl_math_RoundingMode_Corrected );
}

void ScInterpreter::ScRoundDown()
{
    RoundNumber( rtl_math_RoundingMode_Down );
}

void ScInterpreter::ScRoundUp()
{
    RoundNumber( rtl_math_RoundingMode_Up );
}

void ScInterpreter::ScCeil()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        bool bAbs = nParamCount == 3 && GetBool();
        double fDec = GetDouble();
        double fVal = GetDouble();
        if ( fDec == 0.0 )
            PushInt(0);
        else if (fVal*fDec < 0.0)
            PushIllegalArgument();
        else
        {
            if ( !bAbs && fVal < 0.0 )
                PushDouble(::rtl::math::approxFloor(fVal/fDec) * fDec);
            else
                PushDouble(::rtl::math::approxCeil(fVal/fDec) * fDec);
        }
    }
}

void ScInterpreter::ScCeil_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fDec, fVal;
        if ( nParamCount == 1 )
        {
            fVal = GetDouble();
            fDec = 1.0;
        }
        else
        {
            fDec = fabs( GetDoubleWithDefault( 1.0 ));
            fVal = GetDouble();
        }
        if ( fDec == 0.0 || fVal == 0.0 )
            PushInt( 0 );
        else
            PushDouble(::rtl::math::approxCeil( fVal / fDec ) * fDec );
    }
}

void ScInterpreter::ScFloor()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        bool bAbs = nParamCount == 3 && GetBool();
        double fDec = GetDouble();
        double fVal = GetDouble();
        if ( fDec == 0.0 )
            PushInt(0);
        else if (fVal*fDec < 0.0)
            PushIllegalArgument();
        else
        {
            if ( !bAbs && fVal < 0.0 )
                PushDouble(::rtl::math::approxCeil(fVal/fDec) * fDec);
            else
                PushDouble(::rtl::math::approxFloor(fVal/fDec) * fDec);
        }
    }
}

void ScInterpreter::ScFloor_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fDec, fVal;
        if ( nParamCount == 1 )
        {
            fVal = GetDouble();
            fDec = 1.0;
        }
        else
        {
            fDec = fabs( GetDoubleWithDefault( 1.0 ));
            fVal = GetDouble();
        }
        if ( fDec == 0.0 || fVal == 0.0 )
            PushInt( 0 );
        else
            PushDouble(::rtl::math::approxFloor( fVal / fDec ) * fDec );
    }
}

void ScInterpreter::ScEven()
{
    double fVal = GetDouble();
    if (fVal < 0.0)
        PushDouble(::rtl::math::approxFloor(fVal/2.0) * 2.0);
    else
        PushDouble(::rtl::math::approxCeil(fVal/2.0) * 2.0);
}

void ScInterpreter::ScOdd()
{
    double fVal = GetDouble();
    if (fVal >= 0.0)
    {
        fVal = ::rtl::math::approxCeil(fVal);
        if (fmod(fVal, 2.0) == 0.0)
            fVal += 1.0;
    }
    else
    {
        fVal = ::rtl::math::approxFloor(fVal);
        if (fmod(fVal, 2.0) == 0.0)
            fVal -= 1.0;
    }
    PushDouble(fVal);
}

void ScInterpreter::ScArcTan2()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double nVal2 = GetDouble();
        double nVal1 = GetDouble();
        PushDouble(atan2(nVal2, nVal1));
    }
}

void ScInterpreter::ScLog()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double nBase;
        if (nParamCount == 2)
            nBase = GetDouble();
        else
            nBase = 10.0;
        double nVal = GetDouble();
        if (nVal > 0.0 && nBase > 0.0 && nBase != 1.0)
            PushDouble(log(nVal) / log(nBase));
        else
            PushIllegalArgument();
    }
}

void ScInterpreter::ScLn()
{
    double fVal = GetDouble();
    if (fVal > 0.0)
        PushDouble(log(fVal));
    else
        PushIllegalArgument();
}

void ScInterpreter::ScLog10()
{
    double fVal = GetDouble();
    if (fVal > 0.0)
        PushDouble(log10(fVal));
    else
        PushIllegalArgument();
}

void ScInterpreter::ScNPV()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    short nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 31 ) )
    {
        double nVal = 0.0;
        // Wir drehen den Stack um!!
        FormulaToken* pTemp[ 31 ];
        for( short i = 0; i < nParamCount; i++ )
            pTemp[ i ] = pStack[ sp - i - 1 ];
        memcpy( &pStack[ sp - nParamCount ], pTemp, nParamCount * sizeof( FormulaToken* ) );
        if (nGlobalError == 0)
        {
            double  nCount = 1.0;
            double  nZins = GetDouble();
            --nParamCount;
            size_t nRefInList = 0;
            ScRange aRange;
            while (nParamCount-- > 0)
            {
                switch (GetStackType())
                {
                    case svDouble :
                    {
                        nVal += (GetDouble() / pow(1.0 + nZins, (double)nCount));
                        nCount++;
                    }
                    break;
                    case svSingleRef :
                    {
                        ScAddress aAdr;
                        PopSingleRef( aAdr );
                        ScRefCellValue aCell;
                        aCell.assign(*pDok, aAdr);
                        if (!aCell.hasEmptyValue() && aCell.hasNumeric())
                        {
                            double nCellVal = GetCellValue(aAdr, aCell);
                            nVal += (nCellVal / pow(1.0 + nZins, (double)nCount));
                            nCount++;
                        }
                    }
                    break;
                    case svDoubleRef :
                    case svRefList :
                    {
                        sal_uInt16 nErr = 0;
                        double nCellVal;
                        PopDoubleRef( aRange, nParamCount, nRefInList);
                        ScHorizontalValueIterator aValIter( pDok, aRange, glSubTotal);
                        while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                        {
                            nVal += (nCellVal / pow(1.0 + nZins, (double)nCount));
                            nCount++;
                        }
                        if ( nErr != 0 )
                            SetError(nErr);
                    }
                    break;
                    default : SetError(errIllegalParameter); break;
                }
            }
        }
        PushDouble(nVal);
    }
}

void ScInterpreter::ScIRR()
{
    double fSchaetzwert;
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;
    if (nParamCount == 2)
        fSchaetzwert = GetDouble();
    else
        fSchaetzwert = 0.1;
    sal_uInt16 sPos = sp;                       // Stack-Position merken
    double fEps = 1.0;
    double x, xNeu, fWert, fZaehler, fNenner, nCount;
    if (fSchaetzwert == -1.0)
        x = 0.1;                            // default gegen Nulldivisionen
    else
        x = fSchaetzwert;                   // Startwert
    switch (GetStackType())
    {
        case svDoubleRef :
        break;
        default:
        {
            PushIllegalParameter();
            return;
        }
    }
    const sal_uInt16 nIterationsMax = 20;
    sal_uInt16 nItCount = 0;
    ScRange aRange;
    while (fEps > SCdEpsilon && nItCount < nIterationsMax)
    {                                       // Newton-Verfahren:
        sp = sPos;                          // Stack zuruecksetzen
        nCount = 0.0;
        fZaehler = 0.0;
        fNenner = 0.0;
        sal_uInt16 nErr = 0;
        PopDoubleRef( aRange );
        ScValueIterator aValIter(pDok, aRange, glSubTotal);
        if (aValIter.GetFirst(fWert, nErr))
        {
            fZaehler +=           fWert / pow(1.0+x,(double)nCount);
            fNenner  += -nCount * fWert / pow(1.0+x,nCount+1.0);
            nCount++;
            while ((nErr == 0) && aValIter.GetNext(fWert, nErr))
            {
                fZaehler +=           fWert / pow(1.0+x,(double)nCount);
                fNenner  += -nCount * fWert / pow(1.0+x,nCount+1.0);
                nCount++;
            }
            SetError(nErr);
        }
        xNeu = x - fZaehler / fNenner;  // x(i+1) = x(i)-f(x(i))/f'(x(i))
        nItCount++;
        fEps = fabs(xNeu - x);
        x = xNeu;
    }
    if (fSchaetzwert == 0.0 && fabs(x) < SCdEpsilon)
        x = 0.0;                        // auf Null normieren
    if (fEps < SCdEpsilon)
        PushDouble(x);
    else
        PushError( errNoConvergence);
}

void ScInterpreter::ScMIRR()
{   // range_of_values ; rate_invest ; rate_reinvest
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if( MustHaveParamCount( GetByte(), 3 ) )
    {
        double fRate1_reinvest = GetDouble() + 1;
        double fRate1_invest = GetDouble() + 1;

        ScRange aRange;
        PopDoubleRef( aRange );

        if( nGlobalError )
            PushError( nGlobalError);
        else
        {
            double fNPV_reinvest = 0.0;
            double fPow_reinvest = 1.0;
            double fNPV_invest = 0.0;
            double fPow_invest = 1.0;
            ScValueIterator aValIter( pDok, aRange, glSubTotal );
            double fCellValue;
            sal_uLong nCount = 0;
            sal_uInt16 nIterError = 0;

            bool bLoop = aValIter.GetFirst( fCellValue, nIterError );
            while( bLoop )
            {
                if( fCellValue > 0.0 )          // reinvestments
                    fNPV_reinvest += fCellValue * fPow_reinvest;
                else if( fCellValue < 0.0 )     // investments
                    fNPV_invest += fCellValue * fPow_invest;
                fPow_reinvest /= fRate1_reinvest;
                fPow_invest /= fRate1_invest;
                nCount++;

                bLoop = aValIter.GetNext( fCellValue, nIterError );
            }
            if( nIterError )
                PushError( nIterError );
            else
            {
                double fResult = -fNPV_reinvest / fNPV_invest;
                fResult *= pow( fRate1_reinvest, (double) nCount - 1 );
                fResult = pow( fResult, 1.0 / (nCount - 1) );
                PushDouble( fResult - 1.0 );
            }
        }
    }
}

void ScInterpreter::ScISPMT()
{   // rate ; period ; total_periods ; invest
    if( MustHaveParamCount( GetByte(), 4 ) )
    {
        double fInvest = GetDouble();
        double fTotal = GetDouble();
        double fPeriod = GetDouble();
        double fRate = GetDouble();

        if( nGlobalError )
            PushError( nGlobalError);
        else
            PushDouble( fInvest * fRate * (fPeriod / fTotal - 1.0) );
    }
}

// Finanzfunktionen
double ScInterpreter::ScGetBw(double fZins, double fZzr, double fRmz,
                              double fZw, double fF)
{
    double fBw;
    if (fZins == 0.0)
        fBw = fZw + fRmz * fZzr;
    else if (fF > 0.0)
        fBw = (fZw * pow(1.0 + fZins, -fZzr))
                + (fRmz * (1.0 - pow(1.0 + fZins, -fZzr + 1.0)) / fZins)
                + fRmz;
    else
        fBw = (fZw * pow(1.0 + fZins, -fZzr))
                + (fRmz * (1.0 - pow(1.0 + fZins, -fZzr)) / fZins);
    return -fBw;
}

void ScInterpreter::ScBW()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    double nRmz, nZzr, nZins, nZw = 0, nFlag = 0;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nRmz  = GetDouble();
    nZzr  = GetDouble();
    nZins = GetDouble();
    PushDouble(ScGetBw(nZins, nZzr, nRmz, nZw, nFlag));
}

void ScInterpreter::ScDIA()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double nZr = GetDouble();
        double nDauer = GetDouble();
        double nRest = GetDouble();
        double nWert = GetDouble();
        double nDia = ((nWert - nRest) * (nDauer - nZr + 1.0)) /
                      ((nDauer * (nDauer + 1.0)) / 2.0);
        PushDouble(nDia);
    }
}

double ScInterpreter::ScGetGDA(double fWert, double fRest, double fDauer,
                double fPeriode, double fFactor)
{
    double fGda, fZins, fAlterWert, fNeuerWert;
    fZins = fFactor / fDauer;
    if (fZins >= 1.0)
    {
        fZins = 1.0;
        if (fPeriode == 1.0)
            fAlterWert = fWert;
        else
            fAlterWert = 0.0;
    }
    else
        fAlterWert = fWert * pow(1.0 - fZins, fPeriode - 1.0);
    fNeuerWert = fWert * pow(1.0 - fZins, fPeriode);

    if (fNeuerWert < fRest)
        fGda = fAlterWert - fRest;
    else
        fGda = fAlterWert - fNeuerWert;
    if (fGda < 0.0)
        fGda = 0.0;
    return fGda;
}

void ScInterpreter::ScGDA()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 4, 5 ) )
    {
        double nFactor;
        if (nParamCount == 5)
            nFactor = GetDouble();
        else
            nFactor = 2.0;
        double nPeriode = GetDouble();
        double nDauer   = GetDouble();
        double nRest    = GetDouble();
        double nWert    = GetDouble();
        if (nWert < 0.0 || nRest < 0.0 || nFactor <= 0.0 || nRest > nWert
                        || nPeriode < 1.0 || nPeriode > nDauer)
            PushIllegalArgument();
        else
            PushDouble(ScGetGDA(nWert, nRest, nDauer, nPeriode, nFactor));
    }
}

void ScInterpreter::ScGDA2()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 5 ) )
        return ;
    double nMonate;
    if (nParamCount == 4)
        nMonate = 12.0;
    else
        nMonate = ::rtl::math::approxFloor(GetDouble());
    double nPeriode = GetDouble();
    double nDauer = GetDouble();
    double nRest = GetDouble();
    double nWert = GetDouble();
    if (nMonate < 1.0 || nMonate > 12.0 || nDauer > 1200.0 || nRest < 0.0 ||
        nPeriode > (nDauer + 1.0) || nRest > nWert || nWert < 0.0)
    {
        PushIllegalArgument();
        return;
    }
    double nAbRate = 1.0 - pow(nRest / nWert, 1.0 / nDauer);
    nAbRate = ::rtl::math::approxFloor((nAbRate * 1000.0) + 0.5) / 1000.0;
    double nErsteAbRate = nWert * nAbRate * nMonate / 12.0;
    double nGda2 = 0.0;
    if (::rtl::math::approxFloor(nPeriode) == 1)
        nGda2 = nErsteAbRate;
    else
    {
        double nSummAbRate = nErsteAbRate;
        double nMin = nDauer;
        if (nMin > nPeriode) nMin = nPeriode;
        sal_uInt16 iMax = (sal_uInt16)::rtl::math::approxFloor(nMin);
        for (sal_uInt16 i = 2; i <= iMax; i++)
        {
            nGda2 = (nWert - nSummAbRate) * nAbRate;
            nSummAbRate += nGda2;
        }
        if (nPeriode > nDauer)
            nGda2 = ((nWert - nSummAbRate) * nAbRate * (12.0 - nMonate)) / 12.0;
    }
    PushDouble(nGda2);
}

double ScInterpreter::ScInterVDB(double fWert,double fRest,double fDauer,
                             double fDauer1,double fPeriode,double fFactor)
{
    double fVdb=0;
    double fIntEnd   = ::rtl::math::approxCeil(fPeriode);
    sal_uLong nLoopEnd   = (sal_uLong) fIntEnd;

    double fTerm, fLia;
    double fRestwert = fWert - fRest;
    bool bNowLia = false;

    double fGda;
    sal_uLong i;
    fLia=0;
    for ( i = 1; i <= nLoopEnd; i++)
    {
        if(!bNowLia)
        {
            fGda = ScGetGDA(fWert, fRest, fDauer, (double) i, fFactor);
            fLia = fRestwert/ (fDauer1 - (double) (i-1));

            if (fLia > fGda)
            {
                fTerm = fLia;
                bNowLia = true;
            }
            else
            {
                fTerm = fGda;
                fRestwert -= fGda;
            }
        }
        else
        {
            fTerm = fLia;
        }

        if ( i == nLoopEnd)
            fTerm *= ( fPeriode + 1.0 - fIntEnd );

        fVdb += fTerm;
    }
    return fVdb;
}

inline double DblMin( double a, double b )
{
    return (a < b) ? a : b;
}

void ScInterpreter::ScVDB()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 5, 7 ) )
    {
        double fWert, fRest, fDauer, fAnfang, fEnde, fFactor, fVdb = 0.0;
        bool bFlag;
        if (nParamCount == 7)
            bFlag = GetBool();
        else
            bFlag = false;
        if (nParamCount >= 6)
            fFactor = GetDouble();
        else
            fFactor = 2.0;
        fEnde   = GetDouble();
        fAnfang = GetDouble();
        fDauer  = GetDouble();
        fRest   = GetDouble();
        fWert   = GetDouble();
        if (fAnfang < 0.0 || fEnde < fAnfang || fEnde > fDauer || fWert < 0.0
                          || fRest > fWert || fFactor <= 0.0)
            PushIllegalArgument();
        else
        {
            double fIntStart = ::rtl::math::approxFloor(fAnfang);
            double fIntEnd   = ::rtl::math::approxCeil(fEnde);
            sal_uLong nLoopStart = (sal_uLong) fIntStart;
            sal_uLong nLoopEnd   = (sal_uLong) fIntEnd;

            fVdb = 0.0;
            if (bFlag)
            {
                for (sal_uLong i = nLoopStart + 1; i <= nLoopEnd; i++)
                {
                    double fTerm = ScGetGDA(fWert, fRest, fDauer, (double) i, fFactor);

                    //  Teilperioden am Anfang / Ende beruecksichtigen:
                    if ( i == nLoopStart+1 )
                        fTerm *= ( DblMin( fEnde, fIntStart + 1.0 ) - fAnfang );
                    else if ( i == nLoopEnd )
                        fTerm *= ( fEnde + 1.0 - fIntEnd );

                    fVdb += fTerm;
                }
            }
            else
            {

                double fDauer1=fDauer;

                //@Die Frage aller Fragen: "Ist das hier richtig"
                if(!::rtl::math::approxEqual(fAnfang,::rtl::math::approxFloor(fAnfang)))
                {
                    if(fFactor>1)
                    {
                        if(fAnfang>fDauer/2 || ::rtl::math::approxEqual(fAnfang,fDauer/2))
                        {
                            double fPart=fAnfang-fDauer/2;
                            fAnfang=fDauer/2;
                            fEnde-=fPart;
                            fDauer1+=1;
                        }
                    }
                }

                fWert-=ScInterVDB(fWert,fRest,fDauer,fDauer1,fAnfang,fFactor);
                fVdb=ScInterVDB(fWert,fRest,fDauer,fDauer-fAnfang,fEnde-fAnfang,fFactor);
            }
        }
        PushDouble(fVdb);
    }
}

void ScInterpreter::ScLaufz()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nZukunft = GetDouble();
        double nGegenwart = GetDouble();
        double nZins = GetDouble();
        PushDouble(log(nZukunft / nGegenwart) / log(1.0 + nZins));
    }
}

void ScInterpreter::ScLIA()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nDauer = GetDouble();
        double nRest = GetDouble();
        double nWert = GetDouble();
        PushDouble((nWert - nRest) / nDauer);
    }
}

double ScInterpreter::ScGetRmz(double fRate, double fNper, double fPv,
                       double fFv, double fPaytype)
{
    double fPayment;
    if (fRate == 0.0)
        fPayment = (fPv + fFv) / fNper;
    else
    {
        if (fPaytype > 0.0) // payment in advance
            fPayment = (fFv + fPv * exp( fNper * ::rtl::math::log1p(fRate) ) ) * fRate /
                (::rtl::math::expm1( (fNper + 1) * ::rtl::math::log1p(fRate) ) - fRate);
        else  // payment in arrear
            fPayment = (fFv + fPv * exp(fNper * ::rtl::math::log1p(fRate) ) ) * fRate /
                ::rtl::math::expm1( fNper * ::rtl::math::log1p(fRate) );
    }
    return -fPayment;
}

void ScInterpreter::ScRMZ()
{
    double nZins, nZzr, nBw, nZw = 0, nFlag = 0;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZins = GetDouble();
    PushDouble(ScGetRmz(nZins, nZzr, nBw, nZw, nFlag));
}

void ScInterpreter::ScZGZ()
{
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nZukunftswert = GetDouble();
        double nGegenwartswert = GetDouble();
        double nZeitraum = GetDouble();
        PushDouble(pow(nZukunftswert / nGegenwartswert, 1.0 / nZeitraum) - 1.0);
    }
}

double ScInterpreter::ScGetZw(double fZins, double fZzr, double fRmz,
                              double fBw, double fF)
{
    double fZw;
    if (fZins == 0.0)
        fZw = fBw + fRmz * fZzr;
    else
    {
        double fTerm = pow(1.0 + fZins, fZzr);
        if (fF > 0.0)
            fZw = fBw * fTerm + fRmz*(1.0 + fZins)*(fTerm - 1.0)/fZins;
        else
            fZw = fBw * fTerm + fRmz*(fTerm - 1.0)/fZins;
    }
    return -fZw;
}

void ScInterpreter::ScZW()
{
    double nZins, nZzr, nRmz, nBw = 0, nFlag = 0;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nBw   = GetDouble();
    nRmz  = GetDouble();
    nZzr  = GetDouble();
    nZins = GetDouble();
    PushDouble(ScGetZw(nZins, nZzr, nRmz, nBw, nFlag));
}

void ScInterpreter::ScZZR()
{
    double nZins, nRmz, nBw, nZw = 0, nFlag = 0;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nRmz  = GetDouble();
    nZins = GetDouble();
    if (nZins == 0.0)
        PushDouble(-(nBw + nZw)/nRmz);
    else if (nFlag > 0.0)
        PushDouble(log(-(nZins*nZw-nRmz*(1.0+nZins))/(nZins*nBw+nRmz*(1.0+nZins)))
                  /log(1.0+nZins));
    else
        PushDouble(log(-(nZins*nZw-nRmz)/(nZins*nBw+nRmz))/log(1.0+nZins));
}

bool ScInterpreter::RateIteration( double fNper, double fPayment, double fPv,
                                   double fFv, double fPayType, double & fGuess )
{
    // See also #i15090#
    // Newton-Raphson method: x(i+1) = x(i) - f(x(i)) / f'(x(i))
    // This solution handles integer and non-integer values of Nper different.
    // If ODFF will constraint Nper to integer, the distinction of cases can be
    // removed; only the integer-part is needed then.
    bool bValid = true, bFound = false;
    double fX, fXnew, fTerm, fTermDerivation;
    double fGeoSeries, fGeoSeriesDerivation;
    const sal_uInt16 nIterationsMax = 150;
    sal_uInt16 nCount = 0;
    const double fEpsilonSmall = 1.0E-14;
    // convert any fPayType situation to fPayType == zero situation
    fFv = fFv - fPayment * fPayType;
    fPv = fPv + fPayment * fPayType;
    if (fNper == ::rtl::math::round( fNper, 0, rtl_math_RoundingMode_Corrected ))
    { // Nper is an integer value
        fX = fGuess;
        double fPowN, fPowNminus1;  // for (1.0+fX)^Nper and (1.0+fX)^(Nper-1)
        while (!bFound && nCount < nIterationsMax)
        {
            fPowNminus1 = pow( 1.0+fX, fNper-1.0);
            fPowN = fPowNminus1 * (1.0+fX);
            if (rtl::math::approxEqual( fabs(fX), 0.0))
            {
                fGeoSeries = fNper;
                fGeoSeriesDerivation = fNper * (fNper-1.0)/2.0;
            }
            else
            {
                fGeoSeries = (fPowN-1.0)/fX;
                fGeoSeriesDerivation = fNper * fPowNminus1 / fX - fGeoSeries / fX;
            }
            fTerm = fFv + fPv *fPowN+ fPayment * fGeoSeries;
            fTermDerivation = fPv * fNper * fPowNminus1 + fPayment * fGeoSeriesDerivation;
            if (fabs(fTerm) < fEpsilonSmall)
                bFound = true;  // will catch root which is at an extreme
            else
            {
                if (rtl::math::approxEqual( fabs(fTermDerivation), 0.0))
                    fXnew = fX + 1.1 * SCdEpsilon;  // move away from zero slope
                else
                    fXnew = fX - fTerm / fTermDerivation;
                nCount++;
                // more accuracy not possible in oscillating cases
                bFound = (fabs(fXnew - fX) < SCdEpsilon);
                fX = fXnew;
            }
        }
        // Gnumeric returns roots < -1, Excel gives an error in that cases,
        // ODFF says nothing about it. Enable the statement, if you want Excel's
        // behavior.
        //bValid =(fX >=-1.0);
        // Update 2013-06-17: Gnumeric (v1.12.2) doesn't return roots <= -1
        // anymore.
        bValid = (fX > -1.0);
    }
    else
    { // Nper is not an integer value.
        fX = (fGuess < -1.0) ? -1.0 : fGuess;   // start with a valid fX
        while (bValid && !bFound && nCount < nIterationsMax)
        {
            if (rtl::math::approxEqual( fabs(fX), 0.0))
            {
                fGeoSeries = fNper;
                fGeoSeriesDerivation = fNper * (fNper-1.0)/2.0;
            }
            else
            {
                fGeoSeries = (pow( 1.0+fX, fNper) - 1.0) / fX;
                fGeoSeriesDerivation = fNper * pow( 1.0+fX, fNper-1.0) / fX - fGeoSeries / fX;
            }
            fTerm = fFv + fPv *pow(1.0 + fX,fNper)+ fPayment * fGeoSeries;
            fTermDerivation = fPv * fNper * pow( 1.0+fX, fNper-1.0) + fPayment * fGeoSeriesDerivation;
            if (fabs(fTerm) < fEpsilonSmall)
                bFound = true;  // will catch root which is at an extreme
            else
            {
                if (rtl::math::approxEqual( fabs(fTermDerivation), 0.0))
                    fXnew = fX + 1.1 * SCdEpsilon;  // move away from zero slope
                else
                    fXnew = fX - fTerm / fTermDerivation;
                nCount++;
                // more accuracy not possible in oscillating cases
                bFound = (fabs(fXnew - fX) < SCdEpsilon);
                fX = fXnew;
                bValid = (fX >= -1.0);  // otherwise pow(1.0+fX,fNper) will fail
            }
        }
    }
    fGuess = fX;    // return approximate root
    return bValid && bFound;
}

// In Calc UI it is the function RATE(Nper;Pmt;Pv;Fv;Type;Guess)
void ScInterpreter::ScZins()
{
    double fPv, fPayment, fNper;
    // defaults for missing arguments, see ODFF spec
    double fFv = 0, fPayType = 0, fGuess = 0.1, fOrigGuess = 0.1;
    bool bValid = true;
    bool bDefaultGuess = true;
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 6 ) )
        return;
    if (nParamCount == 6)
    {
        fOrigGuess = fGuess = GetDouble();
        bDefaultGuess = false;
    }
    if (nParamCount >= 5)
        fPayType = GetDouble();
    if (nParamCount >= 4)
        fFv = GetDouble();
    fPv = GetDouble();
    fPayment = GetDouble();
    fNper = GetDouble();
    if (fNper <= 0.0) // constraint from ODFF spec
    {
        PushIllegalArgument();
        return;
    }
    // other values for fPayType might be meaningful,
    // ODFF spec is not clear yet, enable statement if you want only 0 and 1
    //if (fPayType != 0.0) fPayType = 1.0;
    bValid = RateIteration(fNper, fPayment, fPv, fFv, fPayType, fGuess);
    if (!bValid)
    {
        /* TODO: try also for specified guess values, not only default? As is,
         * a specified 0.1 guess may be error result but a default 0.1 guess
         * may succeed. On the other hand, using a different guess value than
         * the specified one may not be desired, even if that didn't match. */
        if (bDefaultGuess)
        {
            /* TODO: this is rather ugly, instead of looping over different
             * guess values and doing a Newton goal seek for each we could
             * first insert the values into the RATE equation to obtain a set
             * of y values and then do a bisecting goal seek, possibly using
             * different algorithms. */
            double fX = fOrigGuess;
            for (int nStep = 2; nStep <= 10 && !bValid; ++nStep)
            {
                fGuess = fX * nStep;
                bValid = RateIteration( fNper, fPayment, fPv, fFv, fPayType, fGuess);
                if (!bValid)
                {
                    fGuess = fX / nStep;
                    bValid = RateIteration( fNper, fPayment, fPv, fFv, fPayType, fGuess);
                }
            }
        }
        if (!bValid)
            SetError(errNoConvergence);
    }
    PushDouble(fGuess);
}

double ScInterpreter::ScGetZinsZ(double fZins, double fZr, double fZzr, double fBw,
                                 double fZw, double fF, double& fRmz)
{
    fRmz = ScGetRmz(fZins, fZzr, fBw, fZw, fF);     // fuer kapz auch bei fZr == 1
    double fZinsZ;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if (fZr == 1.0)
    {
        if (fF > 0.0)
            fZinsZ = 0.0;
        else
            fZinsZ = -fBw;
    }
    else
    {
        if (fF > 0.0)
            fZinsZ = ScGetZw(fZins, fZr-2.0, fRmz, fBw, 1.0) - fRmz;
        else
            fZinsZ = ScGetZw(fZins, fZr-1.0, fRmz, fBw, 0.0);
    }
    return fZinsZ * fZins;
}

void ScInterpreter::ScZinsZ()
{
    double nZins, nZr, nZzr, nBw, nZw = 0, nFlag = 0;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 6 ) )
        return;
    if (nParamCount == 6)
        nFlag = GetDouble();
    if (nParamCount >= 5)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZr   = GetDouble();
    nZins = GetDouble();
    if (nZr < 1.0 || nZr > nZzr)
        PushIllegalArgument();
    else
    {
        double nRmz;
        PushDouble(ScGetZinsZ(nZins, nZr, nZzr, nBw, nZw, nFlag, nRmz));
    }
}

void ScInterpreter::ScKapz()
{
    double nZins, nZr, nZzr, nBw, nZw = 0, nFlag = 0;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 6 ) )
        return;
    if (nParamCount == 6)
        nFlag = GetDouble();
    if (nParamCount >= 5)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZr   = GetDouble();
    nZins = GetDouble();
    if (nZr < 1.0 || nZr > nZzr)
        PushIllegalArgument();
    else
    {
        double nRmz;
        double nZinsz = ScGetZinsZ(nZins, nZr, nZzr, nBw, nZw, nFlag, nRmz);
        PushDouble(nRmz - nZinsz);
    }
}

void ScInterpreter::ScKumZinsZ()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 6 ) )
    {
        double fZins, fZzr, fBw, fAnfang, fEnde, fF;
        fF      = GetDouble();
        fEnde   = ::rtl::math::approxFloor(GetDouble());
        fAnfang = ::rtl::math::approxFloor(GetDouble());
        fBw     = GetDouble();
        fZzr    = GetDouble();
        fZins   = GetDouble();
        if (fAnfang < 1.0 || fEnde < fAnfang || fZins <= 0.0 ||
            fEnde > fZzr  || fZzr <= 0.0 || fBw <= 0.0)
            PushIllegalArgument();
        else
        {
            sal_uLong nAnfang = (sal_uLong) fAnfang;
            sal_uLong nEnde = (sal_uLong) fEnde ;
            double fRmz = ScGetRmz(fZins, fZzr, fBw, 0.0, fF);
            double fZinsZ = 0.0;
            if (nAnfang == 1)
            {
                if (fF <= 0.0)
                    fZinsZ = -fBw;
                nAnfang++;
            }
            for (sal_uLong i = nAnfang; i <= nEnde; i++)
            {
                if (fF > 0.0)
                    fZinsZ += ScGetZw(fZins, (double)(i-2), fRmz, fBw, 1.0) - fRmz;
                else
                    fZinsZ += ScGetZw(fZins, (double)(i-1), fRmz, fBw, 0.0);
            }
            fZinsZ *= fZins;
            PushDouble(fZinsZ);
        }
    }
}

void ScInterpreter::ScKumKapZ()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 6 ) )
    {
        double fZins, fZzr, fBw, fAnfang, fEnde, fF;
        fF      = GetDouble();
        fEnde   = ::rtl::math::approxFloor(GetDouble());
        fAnfang = ::rtl::math::approxFloor(GetDouble());
        fBw     = GetDouble();
        fZzr    = GetDouble();
        fZins   = GetDouble();
        if (fAnfang < 1.0 || fEnde < fAnfang || fZins <= 0.0 ||
            fEnde > fZzr  || fZzr <= 0.0 || fBw <= 0.0)
            PushIllegalArgument();
        else
        {
            double fRmz = ScGetRmz(fZins, fZzr, fBw, 0.0, fF);
            double fKapZ = 0.0;
            sal_uLong nAnfang = (sal_uLong) fAnfang;
            sal_uLong nEnde = (sal_uLong) fEnde;
            if (nAnfang == 1)
            {
                if (fF <= 0.0)
                    fKapZ = fRmz + fBw * fZins;
                else
                    fKapZ = fRmz;
                nAnfang++;
            }
            for (sal_uLong i = nAnfang; i <= nEnde; i++)
            {
                if (fF > 0.0)
                    fKapZ += fRmz - (ScGetZw(fZins, (double)(i-2), fRmz, fBw, 1.0) - fRmz) * fZins;
                else
                    fKapZ += fRmz - ScGetZw(fZins, (double)(i-1), fRmz, fBw, 0.0) * fZins;
            }
            PushDouble(fKapZ);
        }
    }
}

void ScInterpreter::ScEffektiv()
{
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fPerioden = GetDouble();
        double fNominal = GetDouble();
        if (fPerioden < 1.0 || fNominal <= 0.0)
            PushIllegalArgument();
        else
        {
            fPerioden = ::rtl::math::approxFloor(fPerioden);
            PushDouble(pow(1.0 + fNominal/fPerioden, fPerioden) - 1.0);
        }
    }
}

void ScInterpreter::ScNominal()
{
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fPerioden = GetDouble();
        double fEffektiv = GetDouble();
        if (fPerioden < 1.0 || fEffektiv <= 0.0)
            PushIllegalArgument();
        else
        {
            fPerioden = ::rtl::math::approxFloor(fPerioden);
            PushDouble( (pow(fEffektiv + 1.0, 1.0 / fPerioden) - 1.0) * fPerioden );
        }
    }
}

void ScInterpreter::ScMod()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fVal2 = GetDouble(); // Denominator
        double fVal1 = GetDouble(); // Numerator
        if (fVal2 == floor(fVal2))  // a pure integral number stored in double
        {
            double fResult = fmod(fVal1,fVal2);
            if ( (fResult != 0.0) &&
                ((fVal1 > 0.0 && fVal2 < 0.0) || (fVal1 < 0.0 && fVal2 > 0.0)))
                fResult += fVal2 ;
            PushDouble( fResult );
        }
        else
        {
            PushDouble( ::rtl::math::approxSub( fVal1,
                    ::rtl::math::approxFloor(fVal1 / fVal2) * fVal2));
        }
    }
}

void ScInterpreter::ScIntersect()
{
    formula::FormulaTokenRef p2nd = PopToken();
    formula::FormulaTokenRef p1st = PopToken();

    if (nGlobalError || !p2nd || !p1st)
    {
        PushIllegalArgument();
        return;
    }

    StackVar sv1 = p1st->GetType();
    StackVar sv2 = p2nd->GetType();
    if ((sv1 != svSingleRef && sv1 != svDoubleRef && sv1 != svRefList) ||
        (sv2 != svSingleRef && sv2 != svDoubleRef && sv2 != svRefList))
    {
        PushIllegalArgument();
        return;
    }

    ScToken* x1 = static_cast<ScToken*>(p1st.get());
    ScToken* x2 = static_cast<ScToken*>(p2nd.get());
    if (sv1 == svRefList || sv2 == svRefList)
    {
        // Now this is a bit nasty but it simplifies things, and having
        // intersections with lists isn't too common, if at all..
        // Convert a reference to list.
        ScToken* xt[2] = { x1, x2 };
        StackVar sv[2] = { sv1, sv2 };
        for (size_t i=0; i<2; ++i)
        {
            if (sv[i] == svSingleRef)
            {
                ScComplexRefData aRef;
                aRef.Ref1 = aRef.Ref2 = xt[i]->GetSingleRef();
                xt[i] = new ScRefListToken;
                xt[i]->GetRefList()->push_back( aRef);
            }
            else if (sv[i] == svDoubleRef)
            {
                ScComplexRefData aRef = xt[i]->GetDoubleRef();
                xt[i] = new ScRefListToken;
                xt[i]->GetRefList()->push_back( aRef);
            }
        }
        x1 = xt[0], x2 = xt[1];

        ScTokenRef xRes = new ScRefListToken;
        ScRefList* pRefList = xRes->GetRefList();
        ScRefList::const_iterator end1( x1->GetRefList()->end());
        ScRefList::const_iterator end2( x2->GetRefList()->end());
        for (ScRefList::const_iterator it1( x1->GetRefList()->begin());
                it1 != end1; ++it1)
        {
            const ScAddress& r11 = (*it1).Ref1.toAbs(aPos);
            const ScAddress& r12 = (*it1).Ref2.toAbs(aPos);
            for (ScRefList::const_iterator it2( x2->GetRefList()->begin());
                    it2 != end2; ++it2)
            {
                const ScAddress& r21 = (*it2).Ref1.toAbs(aPos);
                const ScAddress& r22 = (*it2).Ref2.toAbs(aPos);
                SCCOL nCol1 = ::std::max( r11.Col(), r21.Col());
                SCROW nRow1 = ::std::max( r11.Row(), r21.Row());
                SCTAB nTab1 = ::std::max( r11.Tab(), r21.Tab());
                SCCOL nCol2 = ::std::min( r12.Col(), r22.Col());
                SCROW nRow2 = ::std::min( r12.Row(), r22.Row());
                SCTAB nTab2 = ::std::min( r12.Tab(), r22.Tab());
                if (nCol2 < nCol1 || nRow2 < nRow1 || nTab2 < nTab1)
                    ;   // nothing
                else
                {
                    ScComplexRefData aRef;
                    aRef.InitRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                    pRefList->push_back( aRef);
                }
            }
        }
        size_t n = pRefList->size();
        if (!n)
            PushError( errNoRef);
        else if (n == 1)
        {
            const ScComplexRefData& rRef = (*pRefList)[0];
            if (rRef.Ref1 == rRef.Ref2)
                PushTempToken( new ScSingleRefToken( rRef.Ref1));
            else
                PushTempToken( new ScDoubleRefToken( rRef));
        }
        else
            PushTempToken( xRes.get());
    }
    else
    {
        ScToken* pt[2] = { x1, x2 };
        StackVar sv[2] = { sv1, sv2 };
        SCCOL nC1[2], nC2[2];
        SCROW nR1[2], nR2[2];
        SCTAB nT1[2], nT2[2];
        for (size_t i=0; i<2; ++i)
        {
            switch (sv[i])
            {
                case svSingleRef:
                case svDoubleRef:
                {
                    {
                        const ScAddress& r = pt[i]->GetSingleRef().toAbs(aPos);
                        nC1[i] = r.Col();
                        nR1[i] = r.Row();
                        nT1[i] = r.Tab();
                    }
                    if (sv[i] == svDoubleRef)
                    {
                        const ScAddress& r = pt[i]->GetSingleRef2().toAbs(aPos);
                        nC2[i] = r.Col();
                        nR2[i] = r.Row();
                        nT2[i] = r.Tab();
                    }
                    else
                    {
                        nC2[i] = nC1[i];
                        nR2[i] = nR1[i];
                        nT2[i] = nT1[i];
                    }
                }
                break;
                default:
                    ;   // nothing, prevent compiler warning
            }
        }
        SCCOL nCol1 = ::std::max( nC1[0], nC1[1]);
        SCROW nRow1 = ::std::max( nR1[0], nR1[1]);
        SCTAB nTab1 = ::std::max( nT1[0], nT1[1]);
        SCCOL nCol2 = ::std::min( nC2[0], nC2[1]);
        SCROW nRow2 = ::std::min( nR2[0], nR2[1]);
        SCTAB nTab2 = ::std::min( nT2[0], nT2[1]);
        if (nCol2 < nCol1 || nRow2 < nRow1 || nTab2 < nTab1)
            PushError( errNoRef);
        else if (nCol2 == nCol1 && nRow2 == nRow1 && nTab2 == nTab1)
            PushSingleRef( nCol1, nRow1, nTab1);
        else
            PushDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
    }
}

void ScInterpreter::ScRangeFunc()
{
    formula::FormulaTokenRef x2 = PopToken();
    formula::FormulaTokenRef x1 = PopToken();

    if (nGlobalError || !x2 || !x1)
    {
        PushIllegalArgument();
        return;
    }
    FormulaTokenRef xRes = ScToken::ExtendRangeReference( *x1, *x2, aPos, false);
    if (!xRes)
        PushIllegalArgument();
    else
        PushTempToken( xRes.get());
}

void ScInterpreter::ScUnionFunc()
{
    formula::FormulaTokenRef p2nd = PopToken();
    formula::FormulaTokenRef p1st = PopToken();

    if (nGlobalError || !p2nd || !p1st)
    {
        PushIllegalArgument();
        return;
    }

    StackVar sv1 = p1st->GetType();
    StackVar sv2 = p2nd->GetType();
    if ((sv1 != svSingleRef && sv1 != svDoubleRef && sv1 != svRefList) ||
        (sv2 != svSingleRef && sv2 != svDoubleRef && sv2 != svRefList))
    {
        PushIllegalArgument();
        return;
    }

    ScToken* x1 = static_cast<ScToken*>(p1st.get());
    ScToken* x2 = static_cast<ScToken*>(p2nd.get());

    ScTokenRef xRes;
    // Append to an existing RefList if there is one.
    if (sv1 == svRefList)
    {
        xRes = x1;
        sv1 = svUnknown;    // mark as handled
    }
    else if (sv2 == svRefList)
    {
        xRes = x2;
        sv2 = svUnknown;    // mark as handled
    }
    else
        xRes = new ScRefListToken;
    ScRefList* pRes = xRes->GetRefList();
    ScToken* pt[2] = { x1, x2 };
    StackVar sv[2] = { sv1, sv2 };
    for (size_t i=0; i<2; ++i)
    {
        if (pt[i] == xRes)
            continue;
        switch (sv[i])
        {
            case svSingleRef:
                {
                    ScComplexRefData aRef;
                    aRef.Ref1 = aRef.Ref2 = pt[i]->GetSingleRef();
                    pRes->push_back( aRef);
                }
                break;
            case svDoubleRef:
                pRes->push_back( pt[i]->GetDoubleRef());
                break;
            case svRefList:
                {
                    const ScRefList* p = pt[i]->GetRefList();
                    ScRefList::const_iterator it( p->begin());
                    ScRefList::const_iterator end( p->end());
                    for ( ; it != end; ++it)
                    {
                        pRes->push_back( *it);
                    }
                }
                break;
            default:
                ;   // nothing, prevent compiler warning
        }
    }
    ValidateRef( *pRes);    // set #REF! if needed
    PushTempToken( xRes.get());
}

void ScInterpreter::ScCurrent()
{
    FormulaTokenRef xTok( PopToken());
    if (xTok)
    {
        PushTempToken( xTok.get());
        PushTempToken( xTok.get());
    }
    else
        PushError( errUnknownStackVariable);
}

void ScInterpreter::ScStyle()
{
    sal_uInt8 nParamCount = GetByte();
    if (nParamCount >= 1 && nParamCount <= 3)
    {
        OUString aStyle2;                             // Vorlage nach Timer
        if (nParamCount >= 3)
            aStyle2 = GetString().getString();
        long nTimeOut = 0;                          // Timeout
        if (nParamCount >= 2)
            nTimeOut = (long)(GetDouble()*1000.0);
        OUString aStyle1 = GetString().getString();               // Vorlage fuer sofort

        if (nTimeOut < 0)
            nTimeOut = 0;


        //  Request ausfuehren, um Vorlage anzuwenden


        if ( !pDok->IsClipOrUndo() )
        {
            SfxObjectShell* pShell = pDok->GetDocumentShell();
            if (pShell)
            {
                //! notify object shell directly

                ScRange aRange(aPos);
                ScAutoStyleHint aHint( aRange, aStyle1, nTimeOut, aStyle2 );
                pShell->Broadcast( aHint );
            }
        }

        PushDouble(0.0);
    }
    else
        PushIllegalParameter();
}

static ScDdeLink* lcl_GetDdeLink( sfx2::LinkManager* pLinkMgr,
                                const OUString& rA, const OUString& rT, const OUString& rI, sal_uInt8 nM )
{
    size_t nCount = pLinkMgr->GetLinks().size();
    for (size_t i=0; i<nCount; i++ )
    {
        ::sfx2::SvBaseLink* pBase = *pLinkMgr->GetLinks()[i];
        if (pBase->ISA(ScDdeLink))
        {
            ScDdeLink* pLink = static_cast<ScDdeLink*>(pBase);
            if ( pLink->GetAppl() == rA &&
                 pLink->GetTopic() == rT &&
                 pLink->GetItem() == rI &&
                 pLink->GetMode() == nM )
                return pLink;
        }
    }

    return NULL;
}

void ScInterpreter::ScDde()
{
    //  Applikation, Datei, Bereich
    //  Application, Topic, Item

    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        sal_uInt8 nMode = SC_DDE_DEFAULT;
        if (nParamCount == 4)
            nMode = (sal_uInt8) ::rtl::math::approxFloor(GetDouble());
        OUString aItem  = GetString().getString();
        OUString aTopic = GetString().getString();
        OUString aAppl  = GetString().getString();

        if (nMode > SC_DDE_TEXT)
            nMode = SC_DDE_DEFAULT;

        //  temporary documents (ScFunctionAccess) have no DocShell
        //  and no LinkManager -> abort

        sfx2::LinkManager* pLinkMgr = pDok->GetLinkManager();
        if (!pLinkMgr)
        {
            PushNoValue();
            return;
        }

            //  Nach dem Laden muss neu interpretiert werden (Verknuepfungen aufbauen)

        if ( rArr.IsRecalcModeNormal() )
            rArr.SetExclusiveRecalcModeOnLoad();

            //  solange der Link nicht ausgewertet ist, Idle abklemmen
            //  (um zirkulaere Referenzen zu vermeiden)

        bool bOldEnabled = pDok->IsIdleEnabled();
        pDok->EnableIdle(false);

            //  Link-Objekt holen / anlegen

        ScDdeLink* pLink = lcl_GetDdeLink( pLinkMgr, aAppl, aTopic, aItem, nMode );

        //! Dde-Links (zusaetzlich) effizienter am Dokument speichern !!!!!
        //      ScDdeLink* pLink = pDok->GetDdeLink( aAppl, aTopic, aItem );

        bool bWasError = ( pMyFormulaCell && pMyFormulaCell->GetRawError() != 0 );

        if (!pLink)
        {
            pLink = new ScDdeLink( pDok, aAppl, aTopic, aItem, nMode );
            pLinkMgr->InsertDDELink( pLink, aAppl, aTopic, aItem );
            if ( pLinkMgr->GetLinks().size() == 1 )                    // erster ?
            {
                SfxBindings* pBindings = pDok->GetViewBindings();
                if (pBindings)
                    pBindings->Invalidate( SID_LINKS );             // Link-Manager enablen
            }

                                    //! asynchron auswerten ???
            pLink->TryUpdate();     //  TryUpdate ruft Update nicht mehrfach auf

            if (pMyFormulaCell)
            {
                // StartListening erst nach dem Update, sonst circular reference
                pMyFormulaCell->StartListening( *pLink );
            }
        }
        else
        {
            if (pMyFormulaCell)
                pMyFormulaCell->StartListening( *pLink );
        }

        //  Wenn aus dem Reschedule beim Ausfuehren des Links ein Fehler
        //  (z.B. zirkulaere Referenz) entstanden ist, der vorher nicht da war,
        //  das Fehler-Flag zuruecksetzen:

        if ( pMyFormulaCell && pMyFormulaCell->GetRawError() && !bWasError )
            pMyFormulaCell->SetErrCode(0);

            //  Wert abfragen

        const ScMatrix* pLinkMat = pLink->GetResult();
        if (pLinkMat)
        {
            SCSIZE nC, nR;
            pLinkMat->GetDimensions(nC, nR);
            ScMatrixRef pNewMat = GetNewMat( nC, nR);
            if (pNewMat)
            {
                pLinkMat->MatCopy(*pNewMat);        // kopieren
                PushMatrix( pNewMat );
            }
            else
                PushIllegalArgument();
        }
        else
            PushNA();

        pDok->EnableIdle(bOldEnabled);
        pLinkMgr->CloseCachedComps();
    }
}

void ScInterpreter::ScBase()
{   // Value, Base [, MinLen]
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        static const sal_Unicode pDigits[] = {
            '0','1','2','3','4','5','6','7','8','9',
            'A','B','C','D','E','F','G','H','I','J','K','L','M',
            'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
            0
        };
        static const int nDigits = (sizeof (pDigits)/sizeof(pDigits[0]))-1;
        sal_Int32 nMinLen;
        if ( nParamCount == 3 )
        {
            double fLen = ::rtl::math::approxFloor( GetDouble() );
            if ( 1.0 <= fLen && fLen < SAL_MAX_UINT16 )
                nMinLen = (sal_Int32) fLen;
            else if ( fLen == 0.0 )
                nMinLen = 1;
            else
                nMinLen = 0;    // Error
        }
        else
            nMinLen = 1;
        double fBase = ::rtl::math::approxFloor( GetDouble() );
        double fVal = ::rtl::math::approxFloor( GetDouble() );
        double fChars = ((fVal > 0.0 && fBase > 0.0) ?
            (ceil( log( fVal ) / log( fBase ) ) + 2.0) :
            2.0);
        if ( fChars >= SAL_MAX_UINT16 )
            nMinLen = 0;    // Error

        if ( !nGlobalError && nMinLen && 2 <= fBase && fBase <= nDigits && 0 <= fVal )
        {
            const sal_Int32 nConstBuf = 128;
            sal_Unicode aBuf[nConstBuf];
            sal_Int32 nBuf = std::max<sal_Int32>( fChars, nMinLen + 1 );
            sal_Unicode* pBuf = (nBuf <= nConstBuf ? aBuf : new sal_Unicode[nBuf]);
            for ( sal_Int32 j = 0; j < nBuf; ++j )
            {
                pBuf[j] = '0';
            }
            sal_Unicode* p = pBuf + nBuf - 1;
            *p = 0;
            if ( fVal <= (sal_uLong)(~0) )
            {
                sal_uLong nVal = (sal_uLong) fVal;
                sal_uLong nBase = (sal_uLong) fBase;
                while ( nVal && p > pBuf )
                {
                    *--p = pDigits[ nVal % nBase ];
                    nVal /= nBase;
                }
                fVal = (double) nVal;
            }
            else
            {
                bool bDirt = false;
                while ( fVal && p > pBuf )
                {
//! mit fmod Rundungsfehler ab 2**48
//                  double fDig = ::rtl::math::approxFloor( fmod( fVal, fBase ) );
// so ist es etwas besser
                    double fInt = ::rtl::math::approxFloor( fVal / fBase );
                    double fMult = fInt * fBase;
#if OSL_DEBUG_LEVEL > 1
                    // =BASIS(1e308;36) => GPF mit
                    // nDig = (size_t) ::rtl::math::approxFloor( fVal - fMult );
                    // trotz vorheriger Pruefung ob fVal >= fMult
                    double fDebug1 = fVal - fMult;
                    // fVal    := 7,5975311883090e+290
                    // fMult   := 7,5975311883090e+290
                    // fDebug1 := 1,3848924157003e+275  <- RoundOff-Error
                    // fVal != fMult, aber: ::rtl::math::approxEqual( fVal, fMult ) == TRUE
                    double fDebug2 = ::rtl::math::approxSub( fVal, fMult );
                    // und ::rtl::math::approxSub( fVal, fMult ) == 0
                    double fDebug3 = ( fInt ? fVal / fInt : 0.0 );
                    // Nach dem strange fDebug1 und fVal < fMult  ist eigentlich
                    // fDebug2 == fBase, trotzdem wird das mit einem Vergleich
                    // nicht erkannt, dann schlaegt bDirt zu und alles wird wieder gut..

                    // prevent compiler warnings
                    (void)fDebug1; (void)fDebug2; (void)fDebug3;
#endif
                    size_t nDig;
                    if ( fVal < fMult )
                    {   // da ist was gekippt
                        bDirt = true;
                        nDig = 0;
                    }
                    else
                    {
                        double fDig = ::rtl::math::approxFloor( ::rtl::math::approxSub( fVal, fMult ) );
                        if ( bDirt )
                        {
                            bDirt = false;
                            --fDig;
                        }
                        if ( fDig <= 0.0 )
                            nDig = 0;
                        else if ( fDig >= fBase )
                            nDig = ((size_t) fBase) - 1;
                        else
                            nDig = (size_t) fDig;
                    }
                    *--p = pDigits[ nDig ];
                    fVal = fInt;
                }
            }
            if ( fVal )
                PushError( errStringOverflow );
            else
            {
                if ( nBuf - (p - pBuf) <= nMinLen )
                    p = pBuf + nBuf - 1 - nMinLen;
                PushStringBuffer( p );
            }
            if ( pBuf != aBuf )
                delete [] pBuf;
        }
        else
            PushIllegalArgument();
    }
}

void ScInterpreter::ScDecimal()
{   // Text, Base
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fBase = ::rtl::math::approxFloor( GetDouble() );
        OUString aStr = GetString().getString();
        if ( !nGlobalError && 2 <= fBase && fBase <= 36 )
        {
            double fVal = 0.0;
            int nBase = (int) fBase;
            const sal_Unicode* p = aStr.getStr();
            while ( *p == ' ' || *p == '\t' )
                p++;        // strip leading white space
            if ( nBase == 16 )
            {   // evtl. hex-prefix strippen
                if ( *p == 'x' || *p == 'X' )
                    p++;
                else if ( *p == '0' && (*(p+1) == 'x' || *(p+1) == 'X') )
                    p += 2;
            }
            while ( *p )
            {
                int n;
                if ( '0' <= *p && *p <= '9' )
                    n = *p - '0';
                else if ( 'A' <= *p && *p <= 'Z' )
                    n = 10 + (*p - 'A');
                else if ( 'a' <= *p && *p <= 'z' )
                    n = 10 + (*p - 'a');
                else
                    n = nBase;
                if ( nBase <= n )
                {
                    if ( *(p+1) == 0 &&
                            ( (nBase ==  2 && (*p == 'b' || *p == 'B'))
                            ||(nBase == 16 && (*p == 'h' || *p == 'H')) )
                        )
                        ;       // 101b und F00Dh sind ok
                    else
                    {
                        PushIllegalArgument();
                        return ;
                    }
                }
                else
                    fVal = fVal * fBase + n;
                p++;

            }
            PushDouble( fVal );
        }
        else
            PushIllegalArgument();
    }
}

void ScInterpreter::ScConvert()
{   // Value, FromUnit, ToUnit
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        OUString aToUnit = GetString().getString();
        OUString aFromUnit = GetString().getString();
        double fVal = GetDouble();
        if ( nGlobalError )
            PushError( nGlobalError);
        else
        {   // erst die angegebene Reihenfolge suchen, wenn nicht gefunden den Kehrwert
            double fConv;
            if ( ScGlobal::GetUnitConverter()->GetValue( fConv, aFromUnit, aToUnit ) )
                PushDouble( fVal * fConv );
            else if ( ScGlobal::GetUnitConverter()->GetValue( fConv, aToUnit, aFromUnit ) )
                PushDouble( fVal / fConv );
            else
                PushNA();
        }
    }
}

void ScInterpreter::ScRoman()
{   // Value [Mode]
    sal_uInt8 nParamCount = GetByte();
    if( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fMode = (nParamCount == 2) ? ::rtl::math::approxFloor( GetDouble() ) : 0.0;
        double fVal = ::rtl::math::approxFloor( GetDouble() );
        if( nGlobalError )
            PushError( nGlobalError);
        else if( (fMode >= 0.0) && (fMode < 5.0) && (fVal >= 0.0) && (fVal < 4000.0) )
        {
            static const sal_Unicode pChars[] = { 'M', 'D', 'C', 'L', 'X', 'V', 'I' };
            static const sal_uInt16 pValues[] = { 1000, 500, 100, 50, 10, 5, 1 };
            static const sal_uInt16 nMaxIndex = (sal_uInt16)((sizeof(pValues)/sizeof(pValues[0])) - 1);

            OUString aRoman;
            sal_uInt16 nVal = (sal_uInt16) fVal;
            sal_uInt16 nMode = (sal_uInt16) fMode;

            for( sal_uInt16 i = 0; i <= nMaxIndex / 2; i++ )
            {
                sal_uInt16 nIndex = 2 * i;
                sal_uInt16 nDigit = nVal / pValues[ nIndex ];

                if( (nDigit % 5) == 4 )
                {
                    // assert can't happen with nVal<4000 precondition
                    assert( ((nDigit == 4) ? (nIndex >= 1) : (nIndex >= 2)));

                    sal_uInt16 nIndex2 = (nDigit == 4) ? nIndex - 1 : nIndex - 2;
                    sal_uInt16 nSteps = 0;
                    while( (nSteps < nMode) && (nIndex < nMaxIndex) )
                    {
                        nSteps++;
                        if( pValues[ nIndex2 ] - pValues[ nIndex + 1 ] <= nVal )
                            nIndex++;
                        else
                            nSteps = nMode;
                    }
                    aRoman += OUString( pChars[ nIndex ] );
                    aRoman += OUString( pChars[ nIndex2 ] );
                    nVal = sal::static_int_cast<sal_uInt16>( nVal + pValues[ nIndex ] );
                    nVal = sal::static_int_cast<sal_uInt16>( nVal - pValues[ nIndex2 ] );
                }
                else
                {
                    if( nDigit > 4 )
                    {
                        // assert can't happen with nVal<4000 precondition
                        assert( nIndex >= 1 );
                        aRoman += OUString( pChars[ nIndex - 1 ] );
                    }
                    sal_Int32 nPad = nDigit % 5;
                    if (nPad)
                    {
                        OUStringBuffer aBuf(aRoman);
                        comphelper::string::padToLength(aBuf, aBuf.getLength() + nPad,
                            pChars[nIndex]);
                        aRoman = aBuf.makeStringAndClear();
                    }
                    nVal %= pValues[ nIndex ];
                }
            }

            PushString( aRoman );
        }
        else
            PushIllegalArgument();
    }
}

static bool lcl_GetArabicValue( sal_Unicode cChar, sal_uInt16& rnValue, bool& rbIsDec )
{
    switch( cChar )
    {
        case 'M':   rnValue = 1000; rbIsDec = true;     break;
        case 'D':   rnValue = 500;  rbIsDec = false;    break;
        case 'C':   rnValue = 100;  rbIsDec = true;     break;
        case 'L':   rnValue = 50;   rbIsDec = false;    break;
        case 'X':   rnValue = 10;   rbIsDec = true;     break;
        case 'V':   rnValue = 5;    rbIsDec = false;    break;
        case 'I':   rnValue = 1;    rbIsDec = true;     break;
        default:    return false;
    }
    return true;
}

void ScInterpreter::ScArabic()
{
    OUString aRoman = GetString().getString();
    if( nGlobalError )
        PushError( nGlobalError);
    else
    {
        aRoman = aRoman.toAsciiUpperCase();

        sal_uInt16 nValue = 0;
        sal_uInt16 nValidRest = 3999;
        sal_Int32 nCharIndex = 0;
        sal_Int32 nCharCount = aRoman.getLength();
        bool bValid = true;

        while( bValid && (nCharIndex < nCharCount) )
        {
            sal_uInt16 nDigit1 = 0;
            sal_uInt16 nDigit2 = 0;
            bool bIsDec1 = false;
            bValid = lcl_GetArabicValue( aRoman[nCharIndex], nDigit1, bIsDec1 );
            if( bValid && (nCharIndex + 1 < nCharCount) )
            {
                bool bIsDec2 = false;
                bValid = lcl_GetArabicValue( aRoman[nCharIndex + 1], nDigit2, bIsDec2 );
            }
            if( bValid )
            {
                if( nDigit1 >= nDigit2 )
                {
                    nValue = sal::static_int_cast<sal_uInt16>( nValue + nDigit1 );
                    nValidRest %= (nDigit1 * (bIsDec1 ? 5 : 2));
                    bValid = (nValidRest >= nDigit1);
                    if( bValid )
                        nValidRest = sal::static_int_cast<sal_uInt16>( nValidRest - nDigit1 );
                    nCharIndex++;
                }
                else if( nDigit1 * 2 != nDigit2 )
                {
                    sal_uInt16 nDiff = nDigit2 - nDigit1;
                    nValue = sal::static_int_cast<sal_uInt16>( nValue + nDiff );
                    bValid = (nValidRest >= nDiff);
                    if( bValid )
                        nValidRest = nDigit1 - 1;
                    nCharIndex += 2;
                }
                else
                    bValid = false;
            }
        }
        if( bValid )
            PushInt( nValue );
        else
            PushIllegalArgument();
    }
}

void ScInterpreter::ScHyperLink()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fVal = 0.0;
        svl::SharedString aStr;
        ScMatValType nResultType = SC_MATVAL_STRING;

        if ( nParamCount == 2 )
        {
            switch ( GetStackType() )
            {
                case svDouble:
                    fVal = GetDouble();
                    nResultType = SC_MATVAL_VALUE;
                break;
                case svString:
                    aStr = GetString();
                break;
                case svSingleRef:
                case svDoubleRef:
                {
                    ScAddress aAdr;
                    if ( !PopDoubleRefOrSingleRef( aAdr ) )
                        break;

                    ScRefCellValue aCell;
                    aCell.assign(*pDok, aAdr);
                    if (aCell.hasEmptyValue())
                        nResultType = SC_MATVAL_EMPTY;
                    else
                    {
                        sal_uInt16 nErr = GetCellErrCode(aCell);
                        if (nErr)
                            SetError( nErr);
                        else if (aCell.hasNumeric())
                        {
                            fVal = GetCellValue(aAdr, aCell);
                            nResultType = SC_MATVAL_VALUE;
                        }
                        else
                            GetCellString(aStr, aCell);
                    }
                }
                break;
                case svMatrix:
                    nResultType = GetDoubleOrStringFromMatrix( fVal, aStr);
                break;
                case svMissing:
                case svEmptyCell:
                    Pop();
                    // mimic xcl
                    fVal = 0.0;
                    nResultType = SC_MATVAL_VALUE;
                break;
                default:
                    PopError();
                    SetError( errIllegalArgument);
            }
        }
        svl::SharedString aUrl = GetString();
        ScMatrixRef pResMat = GetNewMat( 1, 2);
        if (nGlobalError)
        {
            fVal = CreateDoubleError( nGlobalError);
            nResultType = SC_MATVAL_VALUE;
        }
        if (nParamCount == 2 || nGlobalError)
        {
            if (ScMatrix::IsValueType( nResultType))
                pResMat->PutDouble( fVal, 0);
            else if (ScMatrix::IsRealStringType( nResultType))
                pResMat->PutString(aStr, 0);
            else    // EmptyType, EmptyPathType, mimic xcl
                pResMat->PutDouble( 0.0, 0 );
        }
        else
            pResMat->PutString(aUrl, 0);
        pResMat->PutString(aUrl, 1);
        bMatrixFormula = true;
        PushMatrix(pResMat);
    }
}

/** Resources at the website of the European Commission:
    http://ec.europa.eu/economy_finance/euro/adoption/conversion/
    http://ec.europa.eu/economy_finance/euro/countries/
 */
static bool lclConvertMoney( const OUString& aSearchUnit, double& rfRate, int& rnDec )
{
    struct ConvertInfo
    {
        const sal_Char* pCurrText;
        double          fRate;
        int             nDec;
    };
    static const ConvertInfo aConvertTable[] = {
        { "EUR", 1.0,      2 },
        { "ATS", 13.7603,  2 },
        { "BEF", 40.3399,  0 },
        { "DEM", 1.95583,  2 },
        { "ESP", 166.386,  0 },
        { "FIM", 5.94573,  2 },
        { "FRF", 6.55957,  2 },
        { "IEP", 0.787564, 2 },
        { "ITL", 1936.27,  0 },
        { "LUF", 40.3399,  0 },
        { "NLG", 2.20371,  2 },
        { "PTE", 200.482,  2 },
        { "GRD", 340.750,  2 },
        { "SIT", 239.640,  2 },
        { "MTL", 0.429300, 2 },
        { "CYP", 0.585274, 2 },
        { "SKK", 30.1260,  2 },
        { "EEK", 15.6466,  2 },
        { "LVL", 0.702804, 2 },
        { "LTL", 3.45280,  2 }
    };

    static const size_t nConversionCount = sizeof( aConvertTable ) / sizeof( aConvertTable[0] );
    for ( size_t i = 0; i < nConversionCount; ++i )
        if ( aSearchUnit.equalsIgnoreAsciiCaseAscii( aConvertTable[i].pCurrText ) )
        {
            rfRate = aConvertTable[i].fRate;
            rnDec  = aConvertTable[i].nDec;
            return true;
        }
    return false;
}

void ScInterpreter::ScEuroConvert()
{   //Value, FromUnit, ToUnit[, FullPrecision, [TriangulationPrecision]]
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 5 ) )
    {
        double nPrecision = 0.0;
        if ( nParamCount == 5 )
        {
            nPrecision = ::rtl::math::approxFloor(GetDouble());
            if ( nPrecision < 3 )
            {
                PushIllegalArgument();
                return;
            }
        }
        bool bFullPrecision = false;
        if ( nParamCount >= 4 )
            bFullPrecision = GetBool();
        OUString aToUnit = GetString().getString();
        OUString aFromUnit = GetString().getString();
        double fVal = GetDouble();
        if ( nGlobalError )
            PushError( nGlobalError);
        else
        {
            double fFromRate;
            double fToRate;
            int    nFromDec;
            int    nToDec;
            OUString aEur( "EUR");
            if ( lclConvertMoney( aFromUnit, fFromRate, nFromDec )
                && lclConvertMoney( aToUnit, fToRate, nToDec ) )
            {
                double fRes;
                if ( aFromUnit.equalsIgnoreAsciiCase( aToUnit ) )
                    fRes = fVal;
                else
                {
                    if ( aFromUnit.equalsIgnoreAsciiCase( aEur ) )
                       fRes = fVal * fToRate;
                    else
                    {
                        double fIntermediate = fVal / fFromRate;
                        if ( nPrecision )
                            fIntermediate = ::rtl::math::round( fIntermediate,
                                                            (int) nPrecision );
                        fRes = fIntermediate * fToRate;
                    }
                    if ( !bFullPrecision )
                        fRes = ::rtl::math::round( fRes, nToDec );
                }
                PushDouble( fRes );
            }
            else
                PushIllegalArgument();
        }
    }
}

// BAHTTEXT
#define UTF8_TH_0       "\340\270\250\340\270\271\340\270\231\340\270\242\340\271\214"
#define UTF8_TH_1       "\340\270\253\340\270\231\340\270\266\340\271\210\340\270\207"
#define UTF8_TH_2       "\340\270\252\340\270\255\340\270\207"
#define UTF8_TH_3       "\340\270\252\340\270\262\340\270\241"
#define UTF8_TH_4       "\340\270\252\340\270\265\340\271\210"
#define UTF8_TH_5       "\340\270\253\340\271\211\340\270\262"
#define UTF8_TH_6       "\340\270\253\340\270\201"
#define UTF8_TH_7       "\340\271\200\340\270\210\340\271\207\340\270\224"
#define UTF8_TH_8       "\340\271\201\340\270\233\340\270\224"
#define UTF8_TH_9       "\340\271\200\340\270\201\340\271\211\340\270\262"
#define UTF8_TH_10      "\340\270\252\340\270\264\340\270\232"
#define UTF8_TH_11      "\340\271\200\340\270\255\340\271\207\340\270\224"
#define UTF8_TH_20      "\340\270\242\340\270\265\340\271\210"
#define UTF8_TH_1E2     "\340\270\243\340\271\211\340\270\255\340\270\242"
#define UTF8_TH_1E3     "\340\270\236\340\270\261\340\270\231"
#define UTF8_TH_1E4     "\340\270\253\340\270\241\340\270\267\340\271\210\340\270\231"
#define UTF8_TH_1E5     "\340\271\201\340\270\252\340\270\231"
#define UTF8_TH_1E6     "\340\270\245\340\271\211\340\270\262\340\270\231"
#define UTF8_TH_DOT0    "\340\270\226\340\271\211\340\270\247\340\270\231"
#define UTF8_TH_BAHT    "\340\270\232\340\270\262\340\270\227"
#define UTF8_TH_SATANG  "\340\270\252\340\270\225\340\270\262\340\270\207\340\270\204\340\271\214"
#define UTF8_TH_MINUS   "\340\270\245\340\270\232"

// local functions
namespace {

inline void lclSplitBlock( double& rfInt, sal_Int32& rnBlock, double fValue, double fSize )
{
    rnBlock = static_cast< sal_Int32 >( modf( (fValue + 0.1) / fSize, &rfInt ) * fSize + 0.1 );
}

/** Appends a digit (0 to 9) to the passed string. */
void lclAppendDigit( OStringBuffer& rText, sal_Int32 nDigit )
{
    switch( nDigit )
    {
        case 0: rText.append( UTF8_TH_0 ); break;
        case 1: rText.append( UTF8_TH_1 ); break;
        case 2: rText.append( UTF8_TH_2 ); break;
        case 3: rText.append( UTF8_TH_3 ); break;
        case 4: rText.append( UTF8_TH_4 ); break;
        case 5: rText.append( UTF8_TH_5 ); break;
        case 6: rText.append( UTF8_TH_6 ); break;
        case 7: rText.append( UTF8_TH_7 ); break;
        case 8: rText.append( UTF8_TH_8 ); break;
        case 9: rText.append( UTF8_TH_9 ); break;
        default:    OSL_FAIL( "lclAppendDigit - illegal digit" );
    }
}

/** Appends a value raised to a power of 10: nDigit*10^nPow10.
    @param nDigit  A digit in the range from 1 to 9.
    @param nPow10  A value in the range from 2 to 5.
 */
void lclAppendPow10( OStringBuffer& rText, sal_Int32 nDigit, sal_Int32 nPow10 )
{
    OSL_ENSURE( (1 <= nDigit) && (nDigit <= 9), "lclAppendPow10 - illegal digit" );
    lclAppendDigit( rText, nDigit );
    switch( nPow10 )
    {
        case 2: rText.append( UTF8_TH_1E2 );   break;
        case 3: rText.append( UTF8_TH_1E3 );   break;
        case 4: rText.append( UTF8_TH_1E4 );   break;
        case 5: rText.append( UTF8_TH_1E5 );   break;
        default:    OSL_FAIL( "lclAppendPow10 - illegal power" );
    }
}

/** Appends a block of 6 digits (value from 1 to 999,999) to the passed string. */
void lclAppendBlock( OStringBuffer& rText, sal_Int32 nValue )
{
    OSL_ENSURE( (1 <= nValue) && (nValue <= 999999), "lclAppendBlock - illegal value" );
    if( nValue >= 100000 )
    {
        lclAppendPow10( rText, nValue / 100000, 5 );
        nValue %= 100000;
    }
    if( nValue >= 10000 )
    {
        lclAppendPow10( rText, nValue / 10000, 4 );
        nValue %= 10000;
    }
    if( nValue >= 1000 )
    {
        lclAppendPow10( rText, nValue / 1000, 3 );
        nValue %= 1000;
    }
    if( nValue >= 100 )
    {
        lclAppendPow10( rText, nValue / 100, 2 );
        nValue %= 100;
    }
    if( nValue > 0 )
    {
        sal_Int32 nTen = nValue / 10;
        sal_Int32 nOne = nValue % 10;
        if( nTen >= 1 )
        {
            if( nTen >= 3 )
                lclAppendDigit( rText, nTen );
            else if( nTen == 2 )
                rText.append( UTF8_TH_20 );
            rText.append( UTF8_TH_10 );
        }
        if( (nTen > 0) && (nOne == 1) )
            rText.append( UTF8_TH_11 );
        else if( nOne > 0 )
            lclAppendDigit( rText, nOne );
    }
}

} // namespace

void ScInterpreter::ScBahtText()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1 ) )
    {
        double fValue = GetDouble();
        if( nGlobalError )
        {
            PushError( nGlobalError);
            return;
        }

        // sign
        bool bMinus = fValue < 0.0;
        fValue = fabs( fValue );

        // round to 2 digits after decimal point, fValue contains Satang as integer
        fValue = ::rtl::math::approxFloor( fValue * 100.0 + 0.5 );

        // split Baht and Satang
        double fBaht = 0.0;
        sal_Int32 nSatang = 0;
        lclSplitBlock( fBaht, nSatang, fValue, 100.0 );

        OStringBuffer aText;

        // generate text for Baht value
        if( fBaht == 0.0 )
        {
            if( nSatang == 0 )
                aText.append( UTF8_TH_0 );
        }
        else while( fBaht > 0.0 )
        {
            OStringBuffer aBlock;
            sal_Int32 nBlock = 0;
            lclSplitBlock( fBaht, nBlock, fBaht, 1.0e6 );
            if( nBlock > 0 )
                lclAppendBlock( aBlock, nBlock );
            // add leading "million", if there will come more blocks
            if( fBaht > 0.0 )
                aBlock.insert( 0, OString(UTF8_TH_1E6 ) );

            aText.insert(0, aBlock.makeStringAndClear());
        }
        if (!aText.isEmpty())
            aText.append( UTF8_TH_BAHT );

        // generate text for Satang value
        if( nSatang == 0 )
        {
            aText.append( UTF8_TH_DOT0 );
        }
        else
        {
            lclAppendBlock( aText, nSatang );
            aText.append( UTF8_TH_SATANG );
        }

        // add the minus sign
        if( bMinus )
            aText.insert( 0, OString( UTF8_TH_MINUS ) );

        PushString( OStringToOUString(aText.makeStringAndClear(), RTL_TEXTENCODING_UTF8) );
    }
}

void ScInterpreter::ScGetPivotData()
{
    sal_uInt8 nParamCount = GetByte();

    if (!MustHaveParamCount(nParamCount, 2, 30) || (nParamCount % 2) == 1)
    {
        PushError(errNoRef);
        return;
    }

    bool bOldSyntax = false;
    if (nParamCount == 2)
    {
        // if the first parameter is a ref, assume old syntax
        StackVar eFirstType = GetStackType(2);
        if (eFirstType == svSingleRef || eFirstType == svDoubleRef)
            bOldSyntax = true;
    }

    std::vector<sheet::DataPilotFieldFilter> aFilters;
    OUString aDataFieldName;
    ScRange aBlock;

    if (bOldSyntax)
    {
        aDataFieldName = GetString().getString();

        switch (GetStackType())
        {
            case svDoubleRef :
                PopDoubleRef(aBlock);
            break;
            case svSingleRef :
            {
                ScAddress aAddr;
                PopSingleRef(aAddr);
                aBlock = aAddr;
            }
            break;
            default:
                PushError(errNoRef);
                return;
        }
    }
    else
    {
        // Standard syntax: separate name/value pairs

        sal_uInt16 nFilterCount = nParamCount / 2 - 1;
        aFilters.resize(nFilterCount);

        sal_uInt16 i = nFilterCount;
        while (i-- > 0)
        {
            //! should allow numeric constraint values
            aFilters[i].MatchValue = GetString().getString();
            aFilters[i].FieldName = GetString().getString();
        }

        switch (GetStackType())
        {
            case svDoubleRef :
                PopDoubleRef(aBlock);
            break;
            case svSingleRef :
            {
                ScAddress aAddr;
                PopSingleRef(aAddr);
                aBlock = aAddr;
            }
            break;
            default:
                PushError(errNoRef);
                return;
        }

        aDataFieldName = GetString().getString(); // First parameter is data field name.
    }

    // NOTE : MS Excel docs claim to use the 'most recent' which is not
    // exactly the same as what we do in ScDocument::GetDPAtBlock
    // However we do need to use GetDPABlock
    ScDPObject* pDPObj = pDok->GetDPAtBlock(aBlock);
    if (!pDPObj)
    {
        PushError(errNoRef);
        return;
    }

    if (bOldSyntax)
    {
        OUString aFilterStr = aDataFieldName;
        std::vector<sheet::GeneralFunction> aFilterFuncs;
        if (!pDPObj->ParseFilters(aDataFieldName, aFilters, aFilterFuncs, aFilterStr))
        {
            PushError(errNoRef);
            return;
        }

        // TODO : For now, we ignore filter functions since we couldn't find a
        // live example of how they are supposed to be used. We'll support
        // this again once we come across a real-world example.
    }

    double fVal = pDPObj->GetPivotData(aDataFieldName, aFilters);
    if (rtl::math::isNan(fVal))
    {
        PushError(errNoRef);
        return;
    }
    PushDouble(fVal);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
