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
#include <boost/math/special_functions/log1p.hpp>

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
#include "stlsheet.hxx"
#include "dpcache.hxx"

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include <limits>
#include <string.h>
#include <math.h>

using ::std::vector;
using namespace com::sun::star;
using namespace formula;

#define SCdEpsilon                1.0E-7

// Date and Time

double ScInterpreter::GetDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay,
        bool bStrict )
{
    if ( nYear < 100 && !bStrict )
        nYear = pFormatter->ExpandTwoDigitYear( nYear );
    // Do not use a default Date ctor here because it asks system time with a
    // performance penalty.
    sal_Int16 nY, nM, nD;
    if (bStrict)
    {
        nY = nYear;
        nM = nMonth;
        nD = nDay;
    }
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
    if (aDate.IsValidAndGregorian())
        return (double) (aDate - *(pFormatter->GetNullDate()));
    else
    {
        SetError(errNoValue);
        return 0;
    }
}

void ScInterpreter::ScGetActDate()
{
    nFuncFmtType = css::util::NumberFormat::DATE;
    Date aActDate( Date::SYSTEM );
    long nDiff = aActDate - *(pFormatter->GetNullDate());
    PushDouble((double) nDiff);
}

void ScInterpreter::ScGetActTime()
{
    nFuncFmtType = css::util::NumberFormat::DATETIME;
    Date aActDate( Date::SYSTEM );
    long nDiff = aActDate - *(pFormatter->GetNullDate());
    tools::Time aActTime( tools::Time::SYSTEM );
    double nTime = aActTime.GetHour()    / static_cast<double>(::tools::Time::hourPerDay)   +
                   aActTime.GetMin()     / static_cast<double>(::tools::Time::minutePerDay) +
                   aActTime.GetSec()     / static_cast<double>(::tools::Time::secondPerDay) +
                   aActTime.GetNanoSec() / static_cast<double>(::tools::Time::nanoSecPerDay);
    PushDouble( (double) nDiff + nTime );
}

void ScInterpreter::ScGetYear()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) GetInt32();
    PushDouble( (double) aDate.GetYear() );
}

void ScInterpreter::ScGetMonth()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) GetInt32();
    PushDouble( (double) aDate.GetMonth() );
}

void ScInterpreter::ScGetDay()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) GetInt32();
    PushDouble((double) aDate.GetDay());
}

void ScInterpreter::ScGetMin()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*DATE_TIME_FACTOR+0.5) % ::tools::Time::secondPerHour;
    PushDouble( (double) (nVal / ::tools::Time::secondPerMinute) );
}

void ScInterpreter::ScGetSec()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*DATE_TIME_FACTOR+0.5) % ::tools::Time::secondPerMinute;
    PushDouble( (double) nVal );
}

void ScInterpreter::ScGetHour()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*DATE_TIME_FACTOR+0.5) / ::tools::Time::secondPerHour;
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
        if (eType == css::util::NumberFormat::DATE || eType == css::util::NumberFormat::DATETIME)
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
        sal_Int16 nFlag;
        if (nParamCount == 2)
            nFlag = GetInt16();
        else
            nFlag = 1;

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long) GetInt32();
        int nVal = (int) aDate.GetDayOfWeek();  // MONDAY = 0
        switch (nFlag)
        {
            case 1:     // Sunday = 1
                if (nVal == 6)
                    nVal = 1;
                else
                    nVal += 2;
            break;
            case 2:     // Monday = 1
                nVal += 1;
            break;
            case 3:     // Monday = 0
                ;   // nothing
            break;
            case 11:    // Monday = 1
            case 12:    // Tuesday = 1
            case 13:    // Wednesday = 1
            case 14:    // Thursday = 1
            case 15:    // Friday = 1
            case 16:    // Saturday = 1
            case 17:    // Sunday = 1
                if (nVal < nFlag - 11)      // x = nFlag - 11 = 0,1,2,3,4,5,6
                    nVal += 19 - nFlag;     // nVal += (8 - (nFlag - 11) = 8 - x = 8,7,6,5,4,3,2)
                else
                    nVal -= nFlag - 12;     // nVal -= ((nFlag - 11) - 1 = x - 1 = -1,0,1,2,3,4,5)
            break;
            default:
                SetError( errIllegalArgument);
        }
        PushInt( nVal );
    }
}

void ScInterpreter::ScWeeknumOOo()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        sal_Int16 nFlag = GetInt16();

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long) GetInt32();
        PushInt( (int) aDate.GetWeekOfYear( nFlag == 1 ? SUNDAY : MONDAY ));
    }
}

void ScInterpreter::ScGetWeekOfYear()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        sal_Int16 nFlag;
        if (nParamCount == 1)
            nFlag = 1;
        else
            nFlag = GetInt16();

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long) GetInt32();

        sal_Int32 nMinimumNumberOfDaysInWeek;
        DayOfWeek eFirstDayOfWeek;
        switch ( nFlag )
        {
            case   1 :
                eFirstDayOfWeek = SUNDAY;
                nMinimumNumberOfDaysInWeek = 1;
                break;
            case   2 :
                eFirstDayOfWeek = MONDAY;
                nMinimumNumberOfDaysInWeek = 1;
                break;
            case  11 :
            case  12 :
            case  13 :
            case  14 :
            case  15 :
            case  16 :
            case  17 :
                eFirstDayOfWeek = static_cast<DayOfWeek>( nFlag - 11 ); // MONDAY := 0
                nMinimumNumberOfDaysInWeek = 1; //the week containing January 1 is week 1
                break;
            case  21 :
            case 150 :
                // ISO 8601
                eFirstDayOfWeek = MONDAY;
                nMinimumNumberOfDaysInWeek = 4;
                break;
            default :
                PushIllegalArgument();
                return;
        }
        PushInt( (int) aDate.GetWeekOfYear( eFirstDayOfWeek, nMinimumNumberOfDaysInWeek ) );
    }
}

void ScInterpreter::ScGetIsoWeekOfYear()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long) GetInt32();
        PushInt( (int) aDate.GetWeekOfYear() );
    }
}

void ScInterpreter::ScEasterSunday()
{
    nFuncFmtType = css::util::NumberFormat::DATE;
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        sal_Int16 nDay, nMonth, nYear;
        nYear = GetInt16();
        if (nGlobalError)
        {
            PushError( nGlobalError);
            return;
        }
        if ( nYear < 100 )
            nYear = pFormatter->ExpandTwoDigitYear( nYear );
        if (nYear < 1583 || nYear > 9956)
        {
            // Valid Gregorian and maximum year constraints not met.
            PushIllegalArgument();
            return;
        }
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
        PushDouble( GetDateSerial( nYear, nMonth, nDay, true ) );
    }
}

sal_uInt16 ScInterpreter::GetWeekendAndHolidayMasks(
    const sal_uInt8 nParamCount, const sal_uInt32 nNullDate, vector< double >& rSortArray,
    bool bWeekendMask[ 7 ] )
{
    if ( nParamCount == 4 )
    {
        vector< double > nWeekendDays;
        GetNumberSequenceArray( 1, nWeekendDays, false );
        if ( nGlobalError )
            return nGlobalError;
        else
        {
            if ( nWeekendDays.size() != 7 )
                return  errIllegalArgument;

            // Weekend days defined by string, Sunday...Saturday
            for ( int i = 0; i < 7; i++ )
                bWeekendMask[ i ] = ( bool ) nWeekendDays[ ( i == 6 ? 0 : i + 1 ) ];
        }
    }
    else
    {
        for ( int i = 0; i < 7; i++ )
            bWeekendMask[ i] = false;

        bWeekendMask[ SATURDAY ] = true;
        bWeekendMask[ SUNDAY ]   = true;
    }

    if ( nParamCount >= 3 )
    {
        GetSortArray( 1, rSortArray, nullptr, true, true );
        size_t nMax = rSortArray.size();
        for ( size_t i = 0; i < nMax; i++ )
            rSortArray.at( i ) = ::rtl::math::approxFloor( rSortArray.at( i ) ) + nNullDate;
    }

    return nGlobalError;
}

sal_uInt16 ScInterpreter::GetWeekendAndHolidayMasks_MS(
    const sal_uInt8 nParamCount, const sal_uInt32 nNullDate, vector< double >& rSortArray,
    bool bWeekendMask[ 7 ], bool bWorkdayFunction )
{
    sal_uInt16 nErr = 0;
    OUString aWeekendDays;
    if ( nParamCount == 4 )
    {
        GetSortArray( 1, rSortArray, nullptr, true, true );
        size_t nMax = rSortArray.size();
        for ( size_t i = 0; i < nMax; i++ )
            rSortArray.at( i ) = ::rtl::math::approxFloor( rSortArray.at( i ) ) + nNullDate;
    }

    if ( nParamCount >= 3 )
    {
        if ( IsMissing() )
            Pop();
        else
        {
            switch ( GetStackType() )
            {
                case svDoubleRef :
                case svExternalDoubleRef :
                    return errNoValue;
                    break;

                default :
                    {
                        double fDouble;
                        svl::SharedString aSharedString;
                        bool bDouble = GetDoubleOrString( fDouble, aSharedString);
                        if ( bDouble )
                        {
                            if ( fDouble >= 1.0 && fDouble <= 17 )
                                aWeekendDays = OUString::number( fDouble );
                            else
                                return errNoValue;
                        }
                        else
                        {
                            if ( aSharedString.isEmpty() || aSharedString.getLength() != 7 ||
                                 ( bWorkdayFunction && aSharedString.getString() == "1111111" ) )
                                return errNoValue;
                            else
                                aWeekendDays = aSharedString.getString();
                        }
                    }
                    break;
            }
        }
    }

    for ( int i = 0; i < 7; i++ )
        bWeekendMask[ i] = false;

    if ( aWeekendDays.isEmpty() )
    {
        bWeekendMask[ SATURDAY ] = true;
        bWeekendMask[ SUNDAY ]   = true;
    }
    else
    {
        switch ( aWeekendDays.getLength() )
        {
            case 1 :
                // Weekend days defined by code
                switch ( aWeekendDays[ 0 ] )
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
                if ( aWeekendDays[ 0 ] == '1' )
                {
                    switch ( aWeekendDays[ 1 ] )
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
                    switch ( aWeekendDays[ i ] )
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

void ScInterpreter::ScNetWorkdays( bool bOOXML_Version )
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 4 ) )
    {
        vector<double> nSortArray;
        bool bWeekendMask[ 7 ];
        Date aNullDate = *( pFormatter->GetNullDate() );
        sal_uInt32 nNullDate = Date::DateToDays( aNullDate.GetDay(), aNullDate.GetMonth(), aNullDate.GetYear() );
        sal_uInt16 nErr;
        if ( bOOXML_Version )
        {
            nErr = GetWeekendAndHolidayMasks_MS( nParamCount, nNullDate,
                            nSortArray, bWeekendMask, false );
        }
        else
        {
            nErr = GetWeekendAndHolidayMasks( nParamCount, nNullDate,
                            nSortArray, bWeekendMask );
        }
        if ( nErr )
            PushError( nErr );
        else
        {
            sal_uInt32 nDate2 = GetUInt32();
            sal_uInt32 nDate1 = GetUInt32();
            if (nGlobalError || (nDate1 > SAL_MAX_UINT32 - nNullDate) || nDate2 > (SAL_MAX_UINT32 - nNullDate))
            {
                PushIllegalArgument();
                return;
            }
            nDate2 += nNullDate;
            nDate1 += nNullDate;

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
        nFuncFmtType = css::util::NumberFormat::DATE;
        vector<double> nSortArray;
        bool bWeekendMask[ 7 ];
        Date aNullDate = *( pFormatter->GetNullDate() );
        sal_uInt32 nNullDate = Date::DateToDays( aNullDate.GetDay(), aNullDate.GetMonth(), aNullDate.GetYear() );
        sal_uInt16 nErr = GetWeekendAndHolidayMasks_MS( nParamCount, nNullDate,
                            nSortArray, bWeekendMask, true );
        if ( nErr )
            PushError( nErr );
        else
        {
            sal_Int32 nDays = GetInt32();
            sal_uInt32 nDate = GetUInt32();
            if (nGlobalError || (nDate > SAL_MAX_UINT32 - nNullDate))
            {
                PushIllegalArgument();
                return;
            }
            nDate += nNullDate;

            if ( !nDays )
                PushDouble( ( double ) ( nDate - nNullDate ) );
            else
            {
                size_t nMax = nSortArray.size();
                if ( nDays > 0 )
                {
                    size_t nRef = 0;
                    //skip holidays before/on start date
                    while ( nRef < nMax && nSortArray.at( nRef ) <= nDate )
                        nRef++;

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
                    //skip holidays after/on start date
                    while ( nRef >= 0 && nSortArray.at( nRef ) >= nDate )
                        nRef--;

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
    nFuncFmtType = css::util::NumberFormat::DATE;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        sal_Int16 nDay   = GetInt16();
        sal_Int16 nMonth = GetInt16();
        if (IsMissing())
            SetError( errParameterExpected);    // Year must be given.
        sal_Int16 nYear  = GetInt16();
        if (nGlobalError || nYear < 0)
            PushIllegalArgument();
        else
        {
            PushDouble(GetDateSerial(nYear, nMonth, nDay, false));
        }
    }
}

void ScInterpreter::ScGetTime()
{
    nFuncFmtType = css::util::NumberFormat::TIME;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nSec = GetDouble();
        double nMin = GetDouble();
        double nHour = GetDouble();
        double fTime = fmod( (nHour * ::tools::Time::secondPerHour) + (nMin * ::tools::Time::secondPerMinute) + nSec, DATE_TIME_FACTOR) / DATE_TIME_FACTOR;
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
        sal_uInt16 d1, m1, d2, m2;
        sal_Int16 y1, y2;
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

        // Close the year 0 gap to calculate year difference.
        if (y1 < 0 && y2 > 0)
            ++y1;
        else if (y1 > 0 && y2 < 0)
            ++y2;

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
                    aDate1.SetYear( y2 == 1 ? -1 : y2 - 1 );
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
        if (eType == css::util::NumberFormat::TIME || eType == css::util::NumberFormat::DATETIME)
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
            sal_Int16 nDec = GetInt16();
            if ( nGlobalError || nDec < -20 || nDec > 20 )
                PushIllegalArgument();
            else
                fVal = ::rtl::math::round( GetDouble(), nDec, eMode );
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

/** tdf69552 ODFF1.2 function CEILING and Excel function CEILING.MATH
    In essence, the difference between the two is that ODFF-CEILING needs to
    have arguments value and significance of the same sign and with
    CEILING.MATH the sign of argument significance is irrevelevant.
    This is why ODFF-CEILING is exported to Excel as CEILING.MATH and
    CEILING.MATH is imported in Calc as CEILING.MATH
 */
void ScInterpreter::ScCeil( bool bODFF )
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 3 ) )
    {
        bool bAbs = nParamCount == 3 && GetBool();
        double fDec, fVal;
        if ( nParamCount == 1 )
        {
            fVal = GetDouble();
            fDec = ( fVal < 0 ? -1 : 1 );
        }
        else
        {
            bool bArgumentMissing = IsMissing();
            fDec = GetDouble();
            fVal = GetDouble();
            if ( bArgumentMissing )
                fDec = ( fVal < 0 ? -1 : 1 );
        }
        if ( fVal == 0 || fDec == 0.0 )
            PushInt( 0 );
        else
        {
            if ( bODFF && fVal * fDec < 0 )
                PushIllegalArgument();
            else
            {
                if ( fVal * fDec < 0.0 )
                    fDec = -fDec;

                if ( !bAbs && fVal < 0.0 )
                    PushDouble(::rtl::math::approxFloor( fVal / fDec ) * fDec );
                else
                    PushDouble(::rtl::math::approxCeil( fVal / fDec ) * fDec );
            }
        }
    }
}

void ScInterpreter::ScCeil_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2 ) )
    {
        double fDec = GetDouble();
        double fVal = GetDouble();
        if ( fVal == 0 || fDec == 0.0 )
            PushInt(0);
        else if ( fVal * fDec > 0 )
            PushDouble(::rtl::math::approxCeil( fVal / fDec ) * fDec );
        else if ( fVal < 0.0 )
            PushDouble(::rtl::math::approxFloor( fVal / -fDec ) * -fDec );
        else
            PushIllegalArgument();
    }
}

void ScInterpreter::ScCeil_Precise()
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

/** tdf69552 ODFF1.2 function FLOOR and Excel function FLOOR.MATH
    In essence, the difference between the two is that ODFF-FLOOR needs to
    have arguments value and significance of the same sign and with
    FLOOR.MATH the sign of argument significance is irrevelevant.
    This is why ODFF-FLOOR is exported to Excel as FLOOR.MATH and
    FLOOR.MATH is imported in Calc as FLOOR.MATH
 */
void ScInterpreter::ScFloor( bool bODFF )
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 3 ) )
    {
        bool bAbs = ( nParamCount == 3 && GetBool() );
        double fDec, fVal;
        if ( nParamCount == 1 )
        {
            fVal = GetDouble();
            fDec = ( fVal < 0 ? -1 : 1 );
        }
        else
        {
            bool bArgumentMissing = IsMissing();
            fDec = GetDouble();
            fVal = GetDouble();
            if ( bArgumentMissing )
                fDec = ( fVal < 0 ? -1 : 1 );
        }
        if ( fDec == 0.0 || fVal == 0.0 )
            PushInt( 0 );
        else
        {
            if ( bODFF && ( fVal * fDec < 0.0 ) )
                PushIllegalArgument();
            else
            {
                if ( fVal * fDec < 0.0 )
                    fDec = -fDec;

                if ( !bAbs && fVal < 0.0 )
                    PushDouble(::rtl::math::approxCeil( fVal / fDec ) * fDec );
                else
                    PushDouble(::rtl::math::approxFloor( fVal / fDec ) * fDec );
            }
        }
    }
}

void ScInterpreter::ScFloor_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2 ) )
    {
        double fDec = GetDouble();
        double fVal = GetDouble();

        if ( fVal == 0 )
            PushInt( 0 );
        else if ( fVal * fDec > 0 )
            PushDouble(::rtl::math::approxFloor( fVal / fDec ) * fDec );
        else if ( fDec == 0 )
            PushIllegalArgument();
        else if ( fVal < 0.0 )
            PushDouble(::rtl::math::approxCeil( fVal / -fDec ) * -fDec );
        else
            PushIllegalArgument();
    }
}

void ScInterpreter::ScFloor_Precise()
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
            fDec = fabs( GetDoubleWithDefault( 1.0 ) );
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
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    short nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 31 ) )
    {
        double nVal = 0.0;
        //We turn the stack upside down!
        FormulaToken* pTemp[ 31 ];
        for( short i = 0; i < nParamCount; i++ )
            pTemp[ i ] = pStack[ sp - i - 1 ];
        memcpy( &pStack[ sp - nParamCount ], pTemp, nParamCount * sizeof( FormulaToken* ) );
        if (nGlobalError == 0)
        {
            double  nCount = 1.0;
            double  nInterest = GetDouble();
            --nParamCount;
            size_t nRefInList = 0;
            ScRange aRange;
            while (nParamCount-- > 0)
            {
                switch (GetStackType())
                {
                    case svDouble :
                    {
                        nVal += (GetDouble() / pow(1.0 + nInterest, (double)nCount));
                        nCount++;
                    }
                    break;
                    case svSingleRef :
                    {
                        ScAddress aAdr;
                        PopSingleRef( aAdr );
                        ScRefCellValue aCell(*pDok, aAdr);
                        if (!aCell.hasEmptyValue() && aCell.hasNumeric())
                        {
                            double nCellVal = GetCellValue(aAdr, aCell);
                            nVal += (nCellVal / pow(1.0 + nInterest, (double)nCount));
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
                        ScHorizontalValueIterator aValIter( pDok, aRange );
                        while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                        {
                            nVal += (nCellVal / pow(1.0 + nInterest, (double)nCount));
                            nCount++;
                        }
                        if ( nErr != 0 )
                            SetError(nErr);
                    }
                    break;
                    case svMatrix :
                    case svExternalSingleRef:
                    case svExternalDoubleRef:
                    {
                        ScMatrixRef pMat = GetMatrix();
                        if (pMat)
                        {
                            SCSIZE nC, nR;
                            pMat->GetDimensions(nC, nR);
                            if (nC == 0 || nR == 0)
                            {
                                PushIllegalArgument();
                                return;
                            }
                            else
                            {
                                double fx;
                                for ( SCSIZE j = 0; j < nC; j++ )
                                {
                                    for (SCSIZE k = 0; k < nR; ++k)
                                    {
                                        if (!pMat->IsValue(j,k))
                                        {
                                            PushIllegalArgument();
                                            return;
                                        }
                                        fx = pMat->GetDouble(j,k);
                                        nVal += (fx / pow(1.0 + nInterest, nCount));
                                        nCount++;
                                    }
                                }
                            }
                        }
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
    double fEstimated;
    nFuncFmtType = css::util::NumberFormat::PERCENT;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;
    if (nParamCount == 2)
        fEstimated = GetDouble();
    else
        fEstimated = 0.1;
    sal_uInt16 sPos = sp;                  // memorize the position of the stack
    double fEps = 1.0;
    double x, fValue;
    if (fEstimated == -1.0)
        x = 0.1;                           // default result for division by zero
    else
        x = fEstimated;                    // startvalue
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
    {                                       // Newtons method:
        sp = sPos;                          // reset stack
        double fNom = 0.0;
        double fDenom = 0.0;
        sal_uInt16 nErr = 0;
        PopDoubleRef( aRange );
        ScValueIterator aValIter(pDok, aRange, mnSubTotalFlags);
        if (aValIter.GetFirst(fValue, nErr))
        {
            double nCount = 0.0;
            fNom +=           fValue / pow(1.0+x,(double)nCount);
            fDenom  += -nCount * fValue / pow(1.0+x,nCount+1.0);
            nCount++;
            while ((nErr == 0) && aValIter.GetNext(fValue, nErr))
            {
                fNom +=           fValue / pow(1.0+x,(double)nCount);
                fDenom  += -nCount * fValue / pow(1.0+x,nCount+1.0);
                nCount++;
            }
            SetError(nErr);
        }
        double xNew = x - fNom / fDenom;  // x(i+1) = x(i)-f(x(i))/f'(x(i))
        nItCount++;
        fEps = fabs(xNew - x);
        x = xNew;
    }
    if (fEstimated == 0.0 && fabs(x) < SCdEpsilon)
        x = 0.0;                        // adjust to zero
    if (fEps < SCdEpsilon)
        PushDouble(x);
    else
        PushError( errNoConvergence);
}

void ScInterpreter::ScMIRR()
{   // range_of_values ; rate_invest ; rate_reinvest
    nFuncFmtType = css::util::NumberFormat::PERCENT;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double fRate1_reinvest = GetDouble() + 1;
        double fRate1_invest = GetDouble() + 1;

        ScRange aRange;
        ScMatrixRef pMat;
        SCSIZE nC = 0;
        SCSIZE nR = 0;
        bool bIsMatrix = false;
        switch ( GetStackType() )
        {
            case svDoubleRef :
                PopDoubleRef( aRange );
                break;
            case svMatrix :
            case svExternalSingleRef:
            case svExternalDoubleRef:
                {
                    pMat = GetMatrix();
                    if ( pMat )
                    {
                        pMat->GetDimensions( nC, nR );
                        if ( nC == 0 || nR == 0 )
                            SetError( errIllegalArgument );
                        bIsMatrix = true;
                    }
                    else
                        SetError( errIllegalArgument );
                }
                break;
            default :
                SetError( errIllegalParameter );
                break;
        }

        if ( nGlobalError )
            PushError( nGlobalError );
        else
        {
            double fNPV_reinvest = 0.0;
            double fPow_reinvest = 1.0;
            double fNPV_invest = 0.0;
            double fPow_invest = 1.0;
            sal_uLong nCount = 0;
            bool bHasPosValue = false;
            bool bHasNegValue = false;

            if ( bIsMatrix )
            {
                double fX;
                for ( SCSIZE j = 0; j < nC; j++ )
                {
                    for ( SCSIZE k = 0; k < nR; ++k )
                    {
                        if ( !pMat->IsValue( j, k ) )
                            continue;
                        fX = pMat->GetDouble( j, k );
                        if ( nGlobalError )
                            break;

                        if ( fX > 0.0 )
                        {    // reinvestments
                            bHasPosValue = true;
                            fNPV_reinvest += fX * fPow_reinvest;
                        }
                        else if ( fX < 0.0 )
                        {   // investments
                            bHasNegValue = true;
                            fNPV_invest += fX * fPow_invest;
                        }
                        fPow_reinvest /= fRate1_reinvest;
                        fPow_invest /= fRate1_invest;
                        nCount++;
                    }
                }
            }
            else
            {
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                double fCellValue;
                sal_uInt16 nIterError = 0;

                bool bLoop = aValIter.GetFirst( fCellValue, nIterError );
                while( bLoop )
                {
                    if( fCellValue > 0.0 )          // reinvestments
                    {    // reinvestments
                        bHasPosValue = true;
                        fNPV_reinvest += fCellValue * fPow_reinvest;
                    }
                    else if( fCellValue < 0.0 )     // investments
                    {   // investments
                        bHasNegValue = true;
                        fNPV_invest += fCellValue * fPow_invest;
                    }
                    fPow_reinvest /= fRate1_reinvest;
                    fPow_invest /= fRate1_invest;
                    nCount++;

                    bLoop = aValIter.GetNext( fCellValue, nIterError );
                }

                if ( nIterError )
                    SetError( nIterError );
            }
            if ( !( bHasPosValue && bHasNegValue ) )
                SetError( errIllegalArgument );

            if ( nGlobalError )
                PushError( nGlobalError );
            else
            {
                double fResult = -fNPV_reinvest / fNPV_invest;
                fResult *= pow( fRate1_reinvest, (double)( nCount - 1 ) );
                fResult = pow( fResult, div( 1.0, (nCount - 1)) );
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
double ScInterpreter::ScGetBw(double fInterest, double fZzr, double fRmz,
                              double fZw, bool bPayInAdvance)
{
    double fBw;
    if (fInterest == 0.0)
        fBw = fZw + fRmz * fZzr;
    else
    {
        if (bPayInAdvance)
            fBw = (fZw * pow(1.0 + fInterest, -fZzr))
                    + (fRmz * (1.0 - pow(1.0 + fInterest, -fZzr + 1.0)) / fInterest)
                    + fRmz;
        else
            fBw = (fZw * pow(1.0 + fInterest, -fZzr))
                    + (fRmz * (1.0 - pow(1.0 + fInterest, -fZzr)) / fInterest);
    }
    return -fBw;
}

void ScInterpreter::ScPV()
{
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    double nRmz, nZzr, nInterest, nZw = 0;
    bool bPayInAdvance = false;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        bPayInAdvance = GetBool();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nRmz  = GetDouble();
    nZzr  = GetDouble();
    nInterest = GetDouble();
    PushDouble(ScGetBw(nInterest, nZzr, nRmz, nZw, bPayInAdvance));
}

void ScInterpreter::ScSYD()
{
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double nZr = GetDouble();
        double nTimeLength = GetDouble();
        double nRest = GetDouble();
        double nValue = GetDouble();
        double nDia = ((nValue - nRest) * (nTimeLength - nZr + 1.0)) /
                      ((nTimeLength * (nTimeLength + 1.0)) / 2.0);
        PushDouble(nDia);
    }
}

double ScInterpreter::ScGetGDA(double fValue, double fRest, double fTimeLength,
                double fPeriod, double fFactor)
{
    double fGda, fInterest, fOldValue, fNewValue;
    fInterest = fFactor / fTimeLength;
    if (fInterest >= 1.0)
    {
        fInterest = 1.0;
        if (fPeriod == 1.0)
            fOldValue = fValue;
        else
            fOldValue = 0.0;
    }
    else
        fOldValue = fValue * pow(1.0 - fInterest, fPeriod - 1.0);
    fNewValue = fValue * pow(1.0 - fInterest, fPeriod);

    if (fNewValue < fRest)
        fGda = fOldValue - fRest;
    else
        fGda = fOldValue - fNewValue;
    if (fGda < 0.0)
        fGda = 0.0;
    return fGda;
}

void ScInterpreter::ScDDB()
{
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 4, 5 ) )
    {
        double nFactor;
        if (nParamCount == 5)
            nFactor = GetDouble();
        else
            nFactor = 2.0;
        double nPeriod = GetDouble();
        double nTimeLength   = GetDouble();
        double nRest    = GetDouble();
        double nValue    = GetDouble();
        if (nValue < 0.0 || nRest < 0.0 || nFactor <= 0.0 || nRest > nValue
                        || nPeriod < 1.0 || nPeriod > nTimeLength)
            PushIllegalArgument();
        else
            PushDouble(ScGetGDA(nValue, nRest, nTimeLength, nPeriod, nFactor));
    }
}

void ScInterpreter::ScDB()
{
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 5 ) )
        return ;
    double nMonths;
    if (nParamCount == 4)
        nMonths = 12.0;
    else
        nMonths = ::rtl::math::approxFloor(GetDouble());
    double nPeriod = GetDouble();
    double nTimeLength = GetDouble();
    double nRest = GetDouble();
    double nValue = GetDouble();
    if (nMonths < 1.0 || nMonths > 12.0 || nTimeLength > 1200.0 || nRest < 0.0 ||
        nPeriod > (nTimeLength + 1.0) || nRest > nValue || nValue <= 0.0 ||
        nTimeLength <= 0 || nPeriod <= 0 )
    {
        PushIllegalArgument();
        return;
    }
    double nOffRate = 1.0 - pow(nRest / nValue, 1.0 / nTimeLength);
    nOffRate = ::rtl::math::approxFloor((nOffRate * 1000.0) + 0.5) / 1000.0;
    double nFirstOffRate = nValue * nOffRate * nMonths / 12.0;
    double nGda2 = 0.0;
    if (::rtl::math::approxFloor(nPeriod) == 1)
        nGda2 = nFirstOffRate;
    else
    {
        double nSumOffRate = nFirstOffRate;
        double nMin = nTimeLength;
        if (nMin > nPeriod) nMin = nPeriod;
        sal_uInt16 iMax = (sal_uInt16)::rtl::math::approxFloor(nMin);
        for (sal_uInt16 i = 2; i <= iMax; i++)
        {
            nGda2 = (nValue - nSumOffRate) * nOffRate;
            nSumOffRate += nGda2;
        }
        if (nPeriod > nTimeLength)
            nGda2 = ((nValue - nSumOffRate) * nOffRate * (12.0 - nMonths)) / 12.0;
    }
    PushDouble(nGda2);
}

double ScInterpreter::ScInterVDB(double fValue,double fRest,double fTimeLength,
                             double fTimeLength1,double fPeriod,double fFactor)
{
    double fVdb=0;
    double fIntEnd   = ::rtl::math::approxCeil(fPeriod);
    sal_uLong nLoopEnd   = (sal_uLong) fIntEnd;

    double fTerm, fLia;
    double fSalvageValue = fValue - fRest;
    bool bNowLia = false;

    double fGda;
    sal_uLong i;
    fLia=0;
    for ( i = 1; i <= nLoopEnd; i++)
    {
        if(!bNowLia)
        {
            fGda = ScGetGDA(fValue, fRest, fTimeLength, (double) i, fFactor);
            fLia = fSalvageValue/ (fTimeLength1 - (double) (i-1));

            if (fLia > fGda)
            {
                fTerm = fLia;
                bNowLia = true;
            }
            else
            {
                fTerm = fGda;
                fSalvageValue -= fGda;
            }
        }
        else
        {
            fTerm = fLia;
        }

        if ( i == nLoopEnd)
            fTerm *= ( fPeriod + 1.0 - fIntEnd );

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
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 5, 7 ) )
    {
        double fValue, fRest, fTimeLength, fStart, fEnd, fFactor, fVdb = 0.0;
        bool bFlag;
        if (nParamCount == 7)
            bFlag = GetBool();
        else
            bFlag = false;
        if (nParamCount >= 6)
            fFactor = GetDouble();
        else
            fFactor = 2.0;
        fEnd   = GetDouble();
        fStart = GetDouble();
        fTimeLength  = GetDouble();
        fRest   = GetDouble();
        fValue   = GetDouble();
        if (fStart < 0.0 || fEnd < fStart || fEnd > fTimeLength || fValue < 0.0
                          || fRest > fValue || fFactor <= 0.0)
            PushIllegalArgument();
        else
        {
            double fIntStart = ::rtl::math::approxFloor(fStart);
            double fIntEnd   = ::rtl::math::approxCeil(fEnd);
            sal_uLong nLoopStart = (sal_uLong) fIntStart;
            sal_uLong nLoopEnd   = (sal_uLong) fIntEnd;

            fVdb = 0.0;
            if (bFlag)
            {
                for (sal_uLong i = nLoopStart + 1; i <= nLoopEnd; i++)
                {
                    double fTerm = ScGetGDA(fValue, fRest, fTimeLength, (double) i, fFactor);

                    //respect partial period in the Beginning/ End:
                    if ( i == nLoopStart+1 )
                        fTerm *= ( DblMin( fEnd, fIntStart + 1.0 ) - fStart );
                    else if ( i == nLoopEnd )
                        fTerm *= ( fEnd + 1.0 - fIntEnd );

                    fVdb += fTerm;
                }
            }
            else
            {

                double fTimeLength1=fTimeLength;

                //@ The question of all questions: 'Is this right'
                if(!::rtl::math::approxEqual(fStart,::rtl::math::approxFloor(fStart)))
                {
                    if(fFactor>1)
                    {
                        if(fStart>fTimeLength/2 || ::rtl::math::approxEqual(fStart,fTimeLength/2))
                        {
                            double fPart=fStart-fTimeLength/2;
                            fStart=fTimeLength/2;
                            fEnd-=fPart;
                            fTimeLength1+=1;
                        }
                    }
                }

                fValue-=ScInterVDB(fValue,fRest,fTimeLength,fTimeLength1,fStart,fFactor);
                fVdb=ScInterVDB(fValue,fRest,fTimeLength,fTimeLength-fStart,fEnd-fStart,fFactor);
            }
        }
        PushDouble(fVdb);
    }
}

void ScInterpreter::ScDuration()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nFuture = GetDouble();
        double nPresent = GetDouble();
        double nInterest = GetDouble();
        PushDouble(log(nFuture / nPresent) / boost::math::log1p(nInterest));
    }
}

void ScInterpreter::ScSLN()
{
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nTimeLength = GetDouble();
        double nRest = GetDouble();
        double nValue = GetDouble();
        PushDouble((nValue - nRest) / nTimeLength);
    }
}

double ScInterpreter::ScGetRmz(double fRate, double fNper, double fPv,
                       double fFv, bool bPayInAdvance)
{
    double fPayment;
    if (fRate == 0.0)
        fPayment = (fPv + fFv) / fNper;
    else
    {
        if (bPayInAdvance) // payment in advance
            fPayment = div((fFv + fPv * exp( fNper * ::rtl::math::log1p(fRate) ) ) * fRate,
                (::rtl::math::expm1( (fNper + 1) * ::rtl::math::log1p(fRate) ) - fRate));
        else  // payment in arrear
            fPayment = div((fFv + fPv * exp(fNper * ::rtl::math::log1p(fRate) ) ) * fRate,
                ::rtl::math::expm1( fNper * ::rtl::math::log1p(fRate) ));
    }
    return -fPayment;
}

void ScInterpreter::ScPMT()
{
    double nInterest, nZzr, nBw, nZw = 0;
    bool bFlag = false;
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        bFlag = GetBool();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nInterest = GetDouble();
    PushDouble(ScGetRmz(nInterest, nZzr, nBw, nZw, bFlag));
}

void ScInterpreter::ScRRI()
{
    nFuncFmtType = css::util::NumberFormat::PERCENT;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nValueInFuture = GetDouble();
        double nValueNow = GetDouble();
        double nSpaceOfTime = GetDouble();
        PushDouble(pow(nValueInFuture / nValueNow, 1.0 / nSpaceOfTime) - 1.0);
    }
}

double ScInterpreter::ScGetZw(double fInterest, double fZzr, double fRmz,
                              double fBw, bool bFlag)
{
    double fZw;
    if (fInterest == 0.0)
        fZw = fBw + fRmz * fZzr;
    else
    {
        double fTerm = pow(1.0 + fInterest, fZzr);
        if (bFlag)
            fZw = fBw * fTerm + fRmz*(1.0 + fInterest)*(fTerm - 1.0)/fInterest;
        else
            fZw = fBw * fTerm + fRmz*(fTerm - 1.0)/fInterest;
    }
    return -fZw;
}

void ScInterpreter::ScFV()
{
    double nInterest, nZzr, nRmz, nBw = 0;
    bool bFlag = false;
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        bFlag = GetBool();
    if (nParamCount >= 4)
        nBw   = GetDouble();
    nRmz  = GetDouble();
    nZzr  = GetDouble();
    nInterest = GetDouble();
    PushDouble(ScGetZw(nInterest, nZzr, nRmz, nBw, bFlag));
}

void ScInterpreter::ScNper()
{
    double nInterest, nRmz, nBw, nZw = 0;
    bool bPayInAdvance = false;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        bPayInAdvance = GetBool();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nRmz  = GetDouble();
    nInterest = GetDouble();
    if (nInterest == 0.0)
        PushDouble(-(nBw + nZw)/nRmz);
    else if (bPayInAdvance)
        PushDouble(log(-(nInterest*nZw-nRmz*(1.0+nInterest))/(nInterest*nBw+nRmz*(1.0+nInterest)))
                  /boost::math::log1p(nInterest));
    else
        PushDouble(log(-(nInterest*nZw-nRmz)/(nInterest*nBw+nRmz))/boost::math::log1p(nInterest));
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
    if (fNper == ::rtl::math::round( fNper ))
    { // Nper is an integer value
        fX = fGuess;
        while (!bFound && nCount < nIterationsMax)
        {
            double fPowN, fPowNminus1;  // for (1.0+fX)^Nper and (1.0+fX)^(Nper-1)
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
void ScInterpreter::ScRate()
{
    double fPv, fPayment, fNper;
    // defaults for missing arguments, see ODFF spec
    double fFv = 0, fPayType = 0, fGuess = 0.1, fOrigGuess = 0.1;
    bool bValid = true;
    bool bDefaultGuess = true;
    nFuncFmtType = css::util::NumberFormat::PERCENT;
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

double ScInterpreter::ScGetCompoundInterest(double fInterest, double fZr, double fZzr, double fBw,
                                 double fZw, bool bPayInAdvance, double& fRmz)
{
    fRmz = ScGetRmz(fInterest, fZzr, fBw, fZw, bPayInAdvance);     // fuer kapz auch bei fZr == 1
    double fCompoundInterest;
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    if (fZr == 1.0)
    {
        if (bPayInAdvance)
            fCompoundInterest = 0.0;
        else
            fCompoundInterest = -fBw;
    }
    else
    {
        if (bPayInAdvance)
            fCompoundInterest = ScGetZw(fInterest, fZr-2.0, fRmz, fBw, true) - fRmz;
        else
            fCompoundInterest = ScGetZw(fInterest, fZr-1.0, fRmz, fBw, false);
    }
    return fCompoundInterest * fInterest;
}

void ScInterpreter::ScIpmt()
{
    double nInterest, nZr, nZzr, nBw, nZw = 0;
    bool bPayInAdvance = false;
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 6 ) )
        return;
    if (nParamCount == 6)
        bPayInAdvance = GetBool();
    if (nParamCount >= 5)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZr   = GetDouble();
    nInterest = GetDouble();
    if (nZr < 1.0 || nZr > nZzr)
        PushIllegalArgument();
    else
    {
        double nRmz;
        PushDouble(ScGetCompoundInterest(nInterest, nZr, nZzr, nBw, nZw, bPayInAdvance, nRmz));
    }
}

void ScInterpreter::ScPpmt()
{
    double nInterest, nZr, nZzr, nBw, nZw = 0;
    bool bPayInAdvance = false;
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 6 ) )
        return;
    if (nParamCount == 6)
        bPayInAdvance = GetBool();
    if (nParamCount >= 5)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZr   = GetDouble();
    nInterest = GetDouble();
    if (nZr < 1.0 || nZr > nZzr)
        PushIllegalArgument();
    else
    {
        double nRmz;
        double nInterestz = ScGetCompoundInterest(nInterest, nZr, nZzr, nBw, nZw, bPayInAdvance, nRmz);
        PushDouble(nRmz - nInterestz);
    }
}

void ScInterpreter::ScCumIpmt()
{
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    if ( MustHaveParamCount( GetByte(), 6 ) )
    {
        double fInterest, fZzr, fBw, fStart, fEnd;
        double fFlag = GetDoubleWithDefault( -1.0 );
        fEnd   = ::rtl::math::approxFloor(GetDouble());
        fStart = ::rtl::math::approxFloor(GetDouble());
        fBw     = GetDouble();
        fZzr    = GetDouble();
        fInterest   = GetDouble();
        if (fStart < 1.0 || fEnd < fStart || fInterest <= 0.0 ||
            fEnd > fZzr  || fZzr <= 0.0 || fBw <= 0.0 ||
            ( fFlag != 0.0 && fFlag != 1.0 ))
            PushIllegalArgument();
        else
        {
            bool bPayInAdvance = ( bool ) fFlag;
            sal_uLong nStart = (sal_uLong) fStart;
            sal_uLong nEnd = (sal_uLong) fEnd ;
            double fRmz = ScGetRmz(fInterest, fZzr, fBw, 0.0, bPayInAdvance);
            double fCompoundInterest = 0.0;
            if (nStart == 1)
            {
                if (!bPayInAdvance)
                    fCompoundInterest = -fBw;
                nStart++;
            }
            for (sal_uLong i = nStart; i <= nEnd; i++)
            {
                if (bPayInAdvance)
                    fCompoundInterest += ScGetZw(fInterest, (double)(i-2), fRmz, fBw, true) - fRmz;
                else
                    fCompoundInterest += ScGetZw(fInterest, (double)(i-1), fRmz, fBw, false);
            }
            fCompoundInterest *= fInterest;
            PushDouble(fCompoundInterest);
        }
    }
}

void ScInterpreter::ScCumPrinc()
{
    nFuncFmtType = css::util::NumberFormat::CURRENCY;
    if ( MustHaveParamCount( GetByte(), 6 ) )
    {
        double fInterest, fZzr, fBw, fStart, fEnd;
        double fFlag = GetDoubleWithDefault( -1.0 );
        fEnd   = ::rtl::math::approxFloor(GetDouble());
        fStart = ::rtl::math::approxFloor(GetDouble());
        fBw     = GetDouble();
        fZzr    = GetDouble();
        fInterest   = GetDouble();
        if (fStart < 1.0 || fEnd < fStart || fInterest <= 0.0 ||
            fEnd > fZzr  || fZzr <= 0.0 || fBw <= 0.0 ||
            ( fFlag != 0.0 && fFlag != 1.0 ))
            PushIllegalArgument();
        else
        {
            bool bPayInAdvance = ( bool ) fFlag;
            double fRmz = ScGetRmz(fInterest, fZzr, fBw, 0.0, bPayInAdvance);
            double fKapZ = 0.0;
            sal_uLong nStart = (sal_uLong) fStart;
            sal_uLong nEnd = (sal_uLong) fEnd;
            if (nStart == 1)
            {
                if (bPayInAdvance)
                    fKapZ = fRmz;
                else
                    fKapZ = fRmz + fBw * fInterest;
                nStart++;
            }
            for (sal_uLong i = nStart; i <= nEnd; i++)
            {
                if (bPayInAdvance)
                    fKapZ += fRmz - (ScGetZw(fInterest, (double)(i-2), fRmz, fBw, true) - fRmz) * fInterest;
                else
                    fKapZ += fRmz - ScGetZw(fInterest, (double)(i-1), fRmz, fBw, false) * fInterest;
            }
            PushDouble(fKapZ);
        }
    }
}

void ScInterpreter::ScEffect()
{
    nFuncFmtType = css::util::NumberFormat::PERCENT;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fPeriods = GetDouble();
        double fNominal = GetDouble();
        if (fPeriods < 1.0 || fNominal <= 0.0)
            PushIllegalArgument();
        else
        {
            fPeriods = ::rtl::math::approxFloor(fPeriods);
            PushDouble(pow(1.0 + fNominal/fPeriods, fPeriods) - 1.0);
        }
    }
}

void ScInterpreter::ScNominal()
{
    nFuncFmtType = css::util::NumberFormat::PERCENT;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fPeriods = GetDouble();
        double fEffective = GetDouble();
        if (fPeriods < 1.0 || fEffective <= 0.0)
            PushIllegalArgument();
        else
        {
            fPeriods = ::rtl::math::approxFloor(fPeriods);
            PushDouble( (pow(fEffective + 1.0, 1.0 / fPeriods) - 1.0) * fPeriods );
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

    formula::FormulaToken* x1 = p1st.get();
    formula::FormulaToken* x2 = p2nd.get();
    if (sv1 == svRefList || sv2 == svRefList)
    {
        // Now this is a bit nasty but it simplifies things, and having
        // intersections with lists isn't too common, if at all..
        // Convert a reference to list.
        formula::FormulaToken* xt[2] = { x1, x2 };
        StackVar sv[2] = { sv1, sv2 };
        for (size_t i=0; i<2; ++i)
        {
            if (sv[i] == svSingleRef)
            {
                ScComplexRefData aRef;
                aRef.Ref1 = aRef.Ref2 = *xt[i]->GetSingleRef();
                xt[i] = new ScRefListToken;
                xt[i]->GetRefList()->push_back( aRef);
            }
            else if (sv[i] == svDoubleRef)
            {
                ScComplexRefData aRef = *xt[i]->GetDoubleRef();
                xt[i] = new ScRefListToken;
                xt[i]->GetRefList()->push_back( aRef);
            }
        }
        x1 = xt[0];
        x2 = xt[1];

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
        formula::FormulaToken* pt[2] = { x1, x2 };
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
                        const ScAddress& r = pt[i]->GetSingleRef()->toAbs(aPos);
                        nC1[i] = r.Col();
                        nR1[i] = r.Row();
                        nT1[i] = r.Tab();
                    }
                    if (sv[i] == svDoubleRef)
                    {
                        const ScAddress& r = pt[i]->GetSingleRef2()->toAbs(aPos);
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
    FormulaTokenRef xRes = extendRangeReference( *x1, *x2, aPos, false);
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

    formula::FormulaToken* x1 = p1st.get();
    formula::FormulaToken* x2 = p2nd.get();

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
    formula::FormulaToken* pt[2] = { x1, x2 };
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
                    aRef.Ref1 = aRef.Ref2 = *pt[i]->GetSingleRef();
                    pRes->push_back( aRef);
                }
                break;
            case svDoubleRef:
                pRes->push_back( *pt[i]->GetDoubleRef());
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
        OUString aStyle2;                           // Template after timer
        if (nParamCount >= 3)
            aStyle2 = GetString().getString();
        long nTimeOut = 0;                          // timeout
        if (nParamCount >= 2)
            nTimeOut = (long)(GetDouble()*1000.0);
        OUString aStyle1 = GetString().getString(); // Template for immediate

        if (nTimeOut < 0)
            nTimeOut = 0;

        // Execute request to apply template
        if ( !pDok->IsClipOrUndo() )
        {
            SfxObjectShell* pShell = pDok->GetDocumentShell();
            if (pShell)
            {
                // notify object shell directly!
                bool bNotify = true;
                if (aStyle2.isEmpty())
                {
                    const ScStyleSheet* pStyle = pDok->GetStyle(aPos.Col(), aPos.Row(), aPos.Tab());

                    if (pStyle && pStyle->GetName() == aStyle1)
                        bNotify = false;
                }

                if (bNotify)
                {
                    ScRange aRange(aPos);
                    ScAutoStyleHint aHint( aRange, aStyle1, nTimeOut, aStyle2 );
                    pShell->Broadcast( aHint );
                }
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
        ::sfx2::SvBaseLink* pBase = pLinkMgr->GetLinks()[i].get();
        if (ScDdeLink* pLink = dynamic_cast<ScDdeLink*>(pBase))
        {
            if ( pLink->GetAppl() == rA &&
                 pLink->GetTopic() == rT &&
                 pLink->GetItem() == rI &&
                 pLink->GetMode() == nM )
                return pLink;
        }
    }

    return nullptr;
}

void ScInterpreter::ScDde()
{
    //  application, file, scope
    //  application, Topic, Item

    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        sal_uInt8 nMode = SC_DDE_DEFAULT;
        if (nParamCount == 4)
        {
            sal_uInt32 nTmp = GetUInt32();
            if (nGlobalError || nTmp > SAL_MAX_UINT8)
            {
                PushIllegalArgument();
                return;
            }
            nMode = (sal_uInt8) nTmp;
        }
        OUString aItem  = GetString().getString();
        OUString aTopic = GetString().getString();
        OUString aAppl  = GetString().getString();

        if (nMode > SC_DDE_TEXT)
            nMode = SC_DDE_DEFAULT;

        //  temporary documents (ScFunctionAccess) have no DocShell
        //  and no LinkManager -> abort

        //sfx2::LinkManager* pLinkMgr = pDok->GetLinkManager();
        if (!mpLinkManager)
        {
            PushNoValue();
            return;
        }

            // Need to reinterpret after loading (build links)

        if ( rArr.IsRecalcModeNormal() )
            rArr.SetExclusiveRecalcModeOnLoad();

            //  while the link ist not evaluated idle must be disabled (to avoid circular references)

        bool bOldEnabled = pDok->IsIdleEnabled();
        pDok->EnableIdle(false);

            // Get/ Create link object

        ScDdeLink* pLink = lcl_GetDdeLink( mpLinkManager, aAppl, aTopic, aItem, nMode );

        //TODO: Save Dde-links (in addition) more efficient at document !!!!!
        //      ScDdeLink* pLink = pDok->GetDdeLink( aAppl, aTopic, aItem );

        bool bWasError = ( pMyFormulaCell && pMyFormulaCell->GetRawError() != 0 );

        if (!pLink)
        {
            pLink = new ScDdeLink( pDok, aAppl, aTopic, aItem, nMode );
            mpLinkManager->InsertDDELink( pLink, aAppl, aTopic, aItem );
            if ( mpLinkManager->GetLinks().size() == 1 )                    // erster ?
            {
                SfxBindings* pBindings = pDok->GetViewBindings();
                if (pBindings)
                    pBindings->Invalidate( SID_LINKS );             // Link-Manager enablen
            }

                                    //TODO: evaluate asynchron ???
            pLink->TryUpdate();     //  TryUpdate doesn't call Update multiple times

            if (pMyFormulaCell)
            {
                // StartListening after the Update to avoid circular references
                pMyFormulaCell->StartListening( *pLink );
            }
        }
        else
        {
            if (pMyFormulaCell)
                pMyFormulaCell->StartListening( *pLink );
        }

        //  If an new Error from Reschedule appears when the link is executed then reset the errorflag


        if ( pMyFormulaCell && pMyFormulaCell->GetRawError() && !bWasError )
            pMyFormulaCell->SetErrCode(0);

            //  check the value

        const ScMatrix* pLinkMat = pLink->GetResult();
        if (pLinkMat)
        {
            SCSIZE nC, nR;
            pLinkMat->GetDimensions(nC, nR);
            ScMatrixRef pNewMat = GetNewMat( nC, nR);
            if (pNewMat)
            {
                pLinkMat->MatCopy(*pNewMat);        // copy
                PushMatrix( pNewMat );
            }
            else
                PushIllegalArgument();
        }
        else
            PushNA();

        pDok->EnableIdle(bOldEnabled);
        mpLinkManager->CloseCachedComps();
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
        static const int nDigits = SAL_N_ELEMENTS(pDigits) - 1;
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
//TODO: roundoff error starting with numbers greater than 2**48
//                  double fDig = ::rtl::math::approxFloor( fmod( fVal, fBase ) );
// a little bit better:
                    double fInt = ::rtl::math::approxFloor( fVal / fBase );
                    double fMult = fInt * fBase;
#if 0
                    // =BASIS(1e308;36) => GPF with
                    // nDig = (size_t) ::rtl::math::approxFloor( fVal - fMult );
                    // in spite off previous test if fVal >= fMult
                    double fDebug1 = fVal - fMult;
                    // fVal    := 7,5975311883090e+290
                    // fMult   := 7,5975311883090e+290
                    // fDebug1 := 1,3848924157003e+275  <- RoundOff-Error
                    // fVal != fMult, aber: ::rtl::math::approxEqual( fVal, fMult ) == TRUE
                    double fDebug2 = ::rtl::math::approxSub( fVal, fMult );
                    // und ::rtl::math::approxSub( fVal, fMult ) == 0
                    double fDebug3 = ( fInt ? fVal / fInt : 0.0 );

                    // Actual after strange fDebug1 and fVal < fMult is fDebug2 == fBase, but
                    // anyway it can't be compared, then bDirt is executed an everything is good...

                    // prevent compiler warnings
                    (void)fDebug1; (void)fDebug2; (void)fDebug3;
#endif
                    size_t nDig;
                    if ( fVal < fMult )
                    {   // something is wrong there
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
                        ;       // 101b and F00Dh are ok
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
        {
            // first of all search for the given order; if it can't be found then search for the inverse
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
            static const sal_uInt16 nMaxIndex = (sal_uInt16)(SAL_N_ELEMENTS(pValues) - 1);

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

                    ScRefCellValue aCell(*pDok, aAdr);
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

    for (const auto & i : aConvertTable)
        if ( aSearchUnit.equalsIgnoreAsciiCaseAscii( i.pCurrText ) )
        {
            rfRate = i.fRate;
            rnDec  = i.nDec;
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
            /* TODO: also, in case of numeric the entire filter match should
             * not be on a (even if locale independent) formatted string down
             * below in pDPObj->GetPivotData(). */

            bool bEvaluateFormatIndex;
            switch (GetRawStackType())
            {
                case svSingleRef:
                case svDoubleRef:
                    bEvaluateFormatIndex = true;
                break;
                default:
                    bEvaluateFormatIndex = false;
            }

            double fDouble;
            svl::SharedString aSharedString;
            bool bDouble = GetDoubleOrString( fDouble, aSharedString);
            if (nGlobalError)
            {
                PushError( nGlobalError);
                return;
            }

            if (bDouble)
            {
                sal_uInt32 nNumFormat;
                if (bEvaluateFormatIndex && nCurFmtIndex)
                    nNumFormat = nCurFmtIndex;
                else
                {
                    if (nCurFmtType == css::util::NumberFormat::UNDEFINED)
                        nNumFormat = 0;
                    else
                        nNumFormat = pFormatter->GetStandardFormat( nCurFmtType, ScGlobal::eLnge);
                }
                Color* pColor;
                pFormatter->GetOutputString( fDouble, nNumFormat, aFilters[i].MatchValueName, &pColor);
                aFilters[i].MatchValue = ScDPCache::GetLocaleIndependentFormattedString(
                        fDouble, *pFormatter, nNumFormat);
            }
            else
            {
                aFilters[i].MatchValueName = aSharedString.getString();

                // Parse possible number from MatchValueName and format
                // locale independent as MatchValue.
                sal_uInt32 nNumFormat = 0;
                double fValue;
                if (pFormatter->IsNumberFormat( aFilters[i].MatchValueName, nNumFormat, fValue))
                    aFilters[i].MatchValue = ScDPCache::GetLocaleIndependentFormattedString(
                            fValue, *pFormatter, nNumFormat);
                else
                    aFilters[i].MatchValue = aFilters[i].MatchValueName;
            }

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

    // Early bail-out, don't grind through data pilot cache and all.
    if (nGlobalError)
    {
        PushError( nGlobalError);
        return;
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
