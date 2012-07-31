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

#include <com/sun/star/util/XNumberFormatTypes.hpp>

#include <string.h>
#include <stdio.h>
#include <tools/resary.hxx>
#include <rtl/math.hxx>
#include <sal/macros.h>
#include "analysishelper.hxx"
#include "analysis.hrc"

using namespace                 ::rtl;
using namespace                 ::com::sun::star;



#define UNIQUE              sal_False   // function name does not exist in Calc
#define DOUBLE              sal_True    // function name exists in Calc

#define STDPAR              sal_False   // all parameters are described
#define INTPAR              sal_True    // first parameter is internal

#define FUNCDATA( FUNCNAME, DBL, OPT, NUMOFPAR, CAT ) \
    { "get" #FUNCNAME, ANALYSIS_FUNCNAME_##FUNCNAME, ANALYSIS_##FUNCNAME, DBL, OPT, ANALYSIS_DEFFUNCNAME_##FUNCNAME, NUMOFPAR, CAT }

const FuncDataBase pFuncDatas[] =
{
    //                          UNIQUE or   INTPAR or
    //         function name     DOUBLE      STDPAR     # of param  category
    FUNCDATA( Workday,          UNIQUE,     INTPAR,     3,          FDCat_DateTime ),
    FUNCDATA( Yearfrac,         UNIQUE,     INTPAR,     3,          FDCat_DateTime ),
    FUNCDATA( Edate,            UNIQUE,     INTPAR,     2,          FDCat_DateTime ),
    FUNCDATA( Weeknum,          DOUBLE,     INTPAR,     2,          FDCat_DateTime ),
    FUNCDATA( Eomonth,          UNIQUE,     INTPAR,     2,          FDCat_DateTime ),
    FUNCDATA( Networkdays,      UNIQUE,     INTPAR,     3,          FDCat_DateTime ),
    FUNCDATA( Iseven,           DOUBLE,     STDPAR,     1,          FDCat_Inf ),
    FUNCDATA( Isodd,            DOUBLE,     STDPAR,     1,          FDCat_Inf ),
    FUNCDATA( Multinomial,      UNIQUE,     STDPAR,     1,          FDCat_Math ),
    FUNCDATA( Seriessum,        UNIQUE,     STDPAR,     4,          FDCat_Math ),
    FUNCDATA( Quotient,         UNIQUE,     STDPAR,     2,          FDCat_Math ),
    FUNCDATA( Mround,           UNIQUE,     STDPAR,     2,          FDCat_Math ),
    FUNCDATA( Sqrtpi,           UNIQUE,     STDPAR,     1,          FDCat_Math ),
    FUNCDATA( Randbetween,      UNIQUE,     STDPAR,     2,          FDCat_Math ),
    FUNCDATA( Gcd,              DOUBLE,     INTPAR,     1,          FDCat_Math ),
    FUNCDATA( Lcm,              DOUBLE,     INTPAR,     1,          FDCat_Math ),
    FUNCDATA( Besseli,          UNIQUE,     STDPAR,     2,          FDCat_Tech ),
    FUNCDATA( Besselj,          UNIQUE,     STDPAR,     2,          FDCat_Tech ),
    FUNCDATA( Besselk,          UNIQUE,     STDPAR,     2,          FDCat_Tech ),
    FUNCDATA( Bessely,          UNIQUE,     STDPAR,     2,          FDCat_Tech ),
    FUNCDATA( Bin2Oct,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Bin2Dec,          UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Bin2Hex,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Oct2Bin,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Oct2Dec,          UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Oct2Hex,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Dec2Bin,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Dec2Hex,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Dec2Oct,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Hex2Bin,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Hex2Dec,          UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Hex2Oct,          UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Delta,            UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Erf,              UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Erfc,             UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Gestep,           UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Factdouble,       UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imabs,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imaginary,        UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Impower,          UNIQUE,     STDPAR,     2,          FDCat_Tech ),
    FUNCDATA( Imargument,       UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imcos,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imdiv,            UNIQUE,     STDPAR,     2,          FDCat_Tech ),
    FUNCDATA( Imexp,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imconjugate,      UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imln,             UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imlog10,          UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imlog2,           UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Improduct,        UNIQUE,     INTPAR,     2,          FDCat_Tech ),
    FUNCDATA( Imreal,           UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imsin,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imsub,            UNIQUE,     STDPAR,     2,          FDCat_Tech ),
    FUNCDATA( Imsqrt,           UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imsum,            UNIQUE,     INTPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imtan,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imsec,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imcsc,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imcot,            UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imsinh,           UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imcosh,           UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imsech,           UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Imcsch,           UNIQUE,     STDPAR,     1,          FDCat_Tech ),
    FUNCDATA( Complex,          UNIQUE,     STDPAR,     3,          FDCat_Tech ),
    FUNCDATA( Convert,          DOUBLE,     STDPAR,     3,          FDCat_Tech ),
    FUNCDATA( Amordegrc,        UNIQUE,     INTPAR,     7,          FDCat_Finance ),
    FUNCDATA( Amorlinc,         UNIQUE,     INTPAR,     7,          FDCat_Finance ),
    FUNCDATA( Accrint,          UNIQUE,     INTPAR,     7,          FDCat_Finance ),
    FUNCDATA( Accrintm,         UNIQUE,     INTPAR,     5,          FDCat_Finance ),
    FUNCDATA( Received,         UNIQUE,     INTPAR,     5,          FDCat_Finance ),
    FUNCDATA( Disc,             UNIQUE,     INTPAR,     5,          FDCat_Finance ),
    FUNCDATA( Duration,         DOUBLE,     INTPAR,     6,          FDCat_Finance ),
    FUNCDATA( Effect,           DOUBLE,     STDPAR,     2,          FDCat_Finance ),
    FUNCDATA( Cumprinc,         DOUBLE,     STDPAR,     6,          FDCat_Finance ),
    FUNCDATA( Cumipmt,          DOUBLE,     STDPAR,     6,          FDCat_Finance ),
    FUNCDATA( Price,            UNIQUE,     INTPAR,     7,          FDCat_Finance ),
    FUNCDATA( Pricedisc,        UNIQUE,     INTPAR,     5,          FDCat_Finance ),
    FUNCDATA( Pricemat,         UNIQUE,     INTPAR,     6,          FDCat_Finance ),
    FUNCDATA( Mduration,        UNIQUE,     INTPAR,     6,          FDCat_Finance ),
    FUNCDATA( Nominal,          DOUBLE,     STDPAR,     2,          FDCat_Finance ),
    FUNCDATA( Dollarfr,         UNIQUE,     STDPAR,     2,          FDCat_Finance ),
    FUNCDATA( Dollarde,         UNIQUE,     STDPAR,     2,          FDCat_Finance ),
    FUNCDATA( Yield,            UNIQUE,     INTPAR,     7,          FDCat_Finance ),
    FUNCDATA( Yielddisc,        UNIQUE,     INTPAR,     5,          FDCat_Finance ),
    FUNCDATA( Yieldmat,         UNIQUE,     INTPAR,     6,          FDCat_Finance ),
    FUNCDATA( Tbilleq,          UNIQUE,     INTPAR,     3,          FDCat_Finance ),
    FUNCDATA( Tbillprice,       UNIQUE,     INTPAR,     3,          FDCat_Finance ),
    FUNCDATA( Tbillyield,       UNIQUE,     INTPAR,     3,          FDCat_Finance ),
    FUNCDATA( Oddfprice,        UNIQUE,     INTPAR,     9,          FDCat_Finance ),
    FUNCDATA( Oddfyield,        UNIQUE,     INTPAR,     9,          FDCat_Finance ),
    FUNCDATA( Oddlprice,        UNIQUE,     INTPAR,     8,          FDCat_Finance ),
    FUNCDATA( Oddlyield,        UNIQUE,     INTPAR,     8,          FDCat_Finance ),
    FUNCDATA( Xirr,             UNIQUE,     INTPAR,     3,          FDCat_Finance ),
    FUNCDATA( Xnpv,             UNIQUE,     STDPAR,     3,          FDCat_Finance ),
    FUNCDATA( Intrate,          UNIQUE,     INTPAR,     5,          FDCat_Finance ),
    FUNCDATA( Coupncd,          UNIQUE,     INTPAR,     4,          FDCat_Finance ),
    FUNCDATA( Coupdays,         UNIQUE,     INTPAR,     4,          FDCat_Finance ),
    FUNCDATA( Coupdaysnc,       UNIQUE,     INTPAR,     4,          FDCat_Finance ),
    FUNCDATA( Coupdaybs,        UNIQUE,     INTPAR,     4,          FDCat_Finance ),
    FUNCDATA( Couppcd,          UNIQUE,     INTPAR,     4,          FDCat_Finance ),
    FUNCDATA( Coupnum,          UNIQUE,     INTPAR,     4,          FDCat_Finance ),
    FUNCDATA( Fvschedule,       UNIQUE,     STDPAR,     2,          FDCat_Finance )
};
#undef FUNCDATA


sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{
    if( (nMonth == 2) && IsLeapYear( nYear ) )
        return 29;
    static const sal_uInt16 aDaysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return aDaysInMonth[ nMonth ];
}


/**
 * Convert a date to a count of days starting from 01/01/0001
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts a Day , Month, Year representation
 * to this internal Date value.
 *
 */

sal_Int32 DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
{
    sal_Int32 nDays = ((sal_Int32)nYear-1) * 365;
    nDays += ((nYear-1) / 4) - ((nYear-1) / 100) + ((nYear-1) / 400);

    for( sal_uInt16 i = 1; i < nMonth; i++ )
        nDays += DaysInMonth(i,nYear);
    nDays += nDay;

    return nDays;
}


/**
 * Convert a count of days starting from 01/01/0001 to a date
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts this internal Date value
 * to a Day , Month, Year representation of a Date.
 *
 */

void DaysToDate( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear )
    throw( lang::IllegalArgumentException )
{
    if( nDays < 0 )
        throw lang::IllegalArgumentException();

    sal_Int32   nTempDays;
    sal_Int32   i = 0;
    sal_Bool    bCalc;

    do
    {
        nTempDays = nDays;
        rYear = (sal_uInt16)((nTempDays / 365) - i);
        nTempDays -= ((sal_Int32) rYear -1) * 365;
        nTempDays -= (( rYear -1) / 4) - (( rYear -1) / 100) + ((rYear -1) / 400);
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
                if ( (nTempDays != 366) || !IsLeapYear( rYear ) )
                {
                    i--;
                    bCalc = sal_True;
                }
            }
        }
    }
    while ( bCalc );

    rMonth = 1;
    while ( (sal_Int32)nTempDays > DaysInMonth( rMonth, rYear ) )
    {
        nTempDays -= DaysInMonth( rMonth, rYear );
        rMonth++;
    }
    rDay = (sal_uInt16)nTempDays;
}


/**
 * Get the null date used by the spreadsheet document
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function returns this internal Date value for the document null date
 *
 */

sal_Int32 GetNullDate( constREFXPS& xOpt ) THROWDEF_RTE
{
    if( xOpt.is() )
    {
        try
        {
            ANY aAny = xOpt->getPropertyValue( STRFROMASCII( "NullDate" ) );
            util::Date  aDate;
            if( aAny >>= aDate )
                return DateToDays( aDate.Day, aDate.Month, aDate.Year );
        }
        catch( uno::Exception& )
        {
        }
    }

    // no null date available -> no calculations possible
    throw uno::RuntimeException();
}


sal_Int32 GetDiffDate360(
                sal_uInt16 nDay1, sal_uInt16 nMonth1, sal_uInt16 nYear1, sal_Bool bLeapYear1,
                sal_uInt16 nDay2, sal_uInt16 nMonth2, sal_uInt16 nYear2,
                sal_Bool bUSAMethod )
{
    if( nDay1 == 31 )
        nDay1--;
    else if( bUSAMethod && ( nMonth1 == 2 && ( nDay1 == 29 || ( nDay1 == 28 && !bLeapYear1 ) ) ) )
            nDay1 = 30;

    if( nDay2 == 31 )
    {
        if( bUSAMethod && nDay1 != 30 )
        {
            nDay2 = 1;
            if( nMonth2 == 12 )
            {
                nYear2++;
                nMonth2 = 1;
            }
            else
                nMonth2++;
        }
        else
            nDay2 = 30;
    }

    return nDay2 + nMonth2 * 30 + nYear2 * 360 - nDay1 - nMonth1 * 30 - nYear1 * 360;
}


sal_Int32 GetDiffDate360( sal_Int32 nNullDate, sal_Int32 nDate1, sal_Int32 nDate2, sal_Bool bUSAMethod )
{
    nDate1 += nNullDate;
    nDate2 += nNullDate;

    sal_uInt16 nDay1, nMonth1, nYear1, nDay2, nMonth2, nYear2;

    DaysToDate( nDate1, nDay1, nMonth1, nYear1 );
    DaysToDate( nDate2, nDay2, nMonth2, nYear2 );

    return GetDiffDate360( nDay1, nMonth1, nYear1, IsLeapYear( nYear1 ), nDay2, nMonth2, nYear2, bUSAMethod );
}


sal_Int32 GetDaysInYears( sal_uInt16 nYear1, sal_uInt16 nYear2 )
{
    sal_uInt16  nLeaps = 0;
    for( sal_uInt16 n = nYear1 ; n <= nYear2 ; n++ )
    {
        if( IsLeapYear( n ) )
            nLeaps++;
    }

    sal_uInt32  nSum = 1;
    nSum += nYear2;
    nSum -= nYear1;
    nSum *= 365;
    nSum += nLeaps;

    return nSum;
}


void GetDiffParam( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode,
    sal_uInt16& rYears, sal_Int32& rDayDiffPart, sal_Int32& rDaysInYear ) THROWDEF_RTE_IAE
{
    if( nStartDate > nEndDate )
    {
        sal_Int32   n = nEndDate;
        nEndDate = nStartDate;
        nStartDate = n;
    }

    sal_Int32   nDate1 = nStartDate + nNullDate;
    sal_Int32   nDate2 = nEndDate + nNullDate;

    sal_uInt16  nDay1, nDay2;
    sal_uInt16  nMonth1, nMonth2;
    sal_uInt16  nYear1, nYear2;

    DaysToDate( nDate1, nDay1, nMonth1, nYear1 );
    DaysToDate( nDate2, nDay2, nMonth2, nYear2 );

    sal_uInt16  nYears;

    sal_Int32   nDayDiff, nDaysInYear;

    switch( nMode )
    {
        case 0:         // 0=USA (NASD) 30/360
        case 4:         // 4=Europe 30/360
            nDaysInYear = 360;
            nYears = nYear2 - nYear1;
            nDayDiff = GetDiffDate360( nDay1, nMonth1, nYear1, IsLeapYear( nYear1 ),
                                        nDay2, nMonth2, nYear2, nMode == 0 ) - nYears * nDaysInYear;
            break;
        case 1:         // 1=exact/exact
            nYears = nYear2 - nYear1;

            nDaysInYear = IsLeapYear( nYear1 )? 366 : 365;

            if( nYears && ( nMonth1 > nMonth2 || ( nMonth1 == nMonth2 && nDay1 > nDay2 ) ) )
                nYears--;

            if( nYears )
                nDayDiff = nDate2 - DateToDays( nDay1, nMonth1, nYear2 );
            else
                nDayDiff = nDate2 - nDate1;

            if( nDayDiff < 0 )
                nDayDiff += nDaysInYear;

            break;
        case 2:         // 2=exact/360
            nDaysInYear = 360;
            nYears = sal_uInt16( ( nDate2 - nDate1 ) / nDaysInYear );
            nDayDiff = nDate2 - nDate1;
            nDayDiff %= nDaysInYear;
            break;
        case 3:         //3=exact/365
            nDaysInYear = 365;
            nYears = sal_uInt16( ( nDate2 - nDate1 ) / nDaysInYear );
            nDayDiff = nDate2 - nDate1;
            nDayDiff %= nDaysInYear;
            break;
        default:
            THROW_IAE;
    }

    rYears = nYears;
    rDayDiffPart = nDayDiff;
    rDaysInYear = nDaysInYear;
}


sal_Int32 GetDiffDate( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode,
    sal_Int32* pOptDaysIn1stYear ) THROWDEF_RTE_IAE
{
    sal_Bool    bNeg = nStartDate > nEndDate;

    if( bNeg )
    {
        sal_Int32   n = nEndDate;
        nEndDate = nStartDate;
        nStartDate = n;
    }

    sal_Int32       nRet;

    switch( nMode )
    {
        case 0:         // 0=USA (NASD) 30/360
        case 4:         // 4=Europe 30/360
            {
            sal_uInt16      nD1, nM1, nY1, nD2, nM2, nY2;

            nStartDate += nNullDate;
            nEndDate += nNullDate;

            DaysToDate( nStartDate, nD1, nM1, nY1 );
            DaysToDate( nEndDate, nD2, nM2, nY2 );

            sal_Bool        bLeap = IsLeapYear( nY1 );
            sal_Int32       nDays, nMonths;

            nMonths = nM2 - nM1;
            nDays = nD2 - nD1;

            nMonths += ( nY2 - nY1 ) * 12;

            nRet = nMonths * 30 + nDays;
            if( nMode == 0 && nM1 == 2 && nM2 != 2 && nY1 == nY2 )
                nRet -= bLeap? 1 : 2;

            if( pOptDaysIn1stYear )
                *pOptDaysIn1stYear = 360;
            }
            break;
        case 1:         // 1=exact/exact
            if( pOptDaysIn1stYear )
            {
                sal_uInt16      nD, nM, nY;

                DaysToDate( nStartDate + nNullDate, nD, nM, nY );

                *pOptDaysIn1stYear = IsLeapYear( nY )? 366 : 365;
            }
            nRet = nEndDate - nStartDate;
            break;
        case 2:         // 2=exact/360
            nRet = nEndDate - nStartDate;
            if( pOptDaysIn1stYear )
                *pOptDaysIn1stYear = 360;
            break;
        case 3:         //3=exact/365
            nRet = nEndDate - nStartDate;
            if( pOptDaysIn1stYear )
                *pOptDaysIn1stYear = 365;
            break;
        default:
            THROW_IAE;
    }

    return bNeg? -nRet : nRet;
}


double GetYearDiff( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode ) THROWDEF_RTE_IAE
{
    sal_Int32   nDays1stYear;
    sal_Int32   nTotalDays = GetDiffDate( nNullDate, nStartDate, nEndDate, nMode, &nDays1stYear );

    return double( nTotalDays ) / double( nDays1stYear );
}


sal_Int32 GetDaysInYear( sal_Int32 nNullDate, sal_Int32 nDate, sal_Int32 nMode ) THROWDEF_RTE_IAE
{
    switch( nMode )
    {
        case 0:         // 0=USA (NASD) 30/360
        case 2:         // 2=exact/360
        case 4:         // 4=Europe 30/360
            return 360;
        case 1:         // 1=exact/exact
            {
            sal_uInt16  nD, nM, nY;
            nDate += nNullDate;
            DaysToDate( nDate, nD, nM, nY );
            return IsLeapYear( nY )? 366 : 365;
            }
        case 3:         //3=exact/365
            return 365;
        default:
            THROW_IAE;
    }
}


double GetYearFrac( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode ) THROWDEF_RTE_IAE
{
    if( nStartDate == nEndDate )
        return 0.0;     // nothing to do...

    sal_uInt16  nYears;
    sal_Int32   nDayDiff, nDaysInYear;

    GetDiffParam( nNullDate, nStartDate, nEndDate, nMode, nYears, nDayDiff, nDaysInYear );

    return double( nYears ) + double( nDayDiff ) / double( nDaysInYear );
}

double BinomialCoefficient( double n, double k )
{
    // This method is a copy of BinomKoeff()
    // found in sc/source/core/tool/interpr3.cxx

    double nVal = 0.0;
    k = ::rtl::math::approxFloor(k);
    if (n < k)
        nVal = 0.0;
    else if (k == 0.0)
        nVal = 1.0;
    else
    {
        nVal = n/k;
        n--;
        k--;
        while (k > 0.0)
        {
            nVal *= n/k;
            k--;
            n--;
        }
    }
    return nVal;
}

double GetGcd( double f1, double f2 )
{
    double  f = fmod( f1, f2 );
    while( f > 0.0 )
    {
        f1 = f2;
        f2 = f;
        f = fmod( f1, f2 );
    }

    return f2;
}


double ConvertToDec( const STRING& aStr, sal_uInt16 nBase, sal_uInt16 nCharLim ) THROWDEF_RTE_IAE
{
    if ( nBase < 2 || nBase > 36 )
        THROW_IAE;

    sal_uInt32      nStrLen = aStr.getLength();
    if( nStrLen > nCharLim )
        THROW_IAE;
    else if( !nStrLen )
        return 0.0;

    double          fVal = 0.0;

    register const sal_Unicode* p = aStr.getStr();

    sal_uInt16          nFirstDig = 0;
    sal_Bool            bFirstDig = sal_True;
    double              fBase = nBase;

    while ( *p )
    {
        sal_uInt16      n;

        if( '0' <= *p && *p <= '9' )
            n = *p - '0';
        else if( 'A' <= *p && *p <= 'Z' )
            n = 10 + ( *p - 'A' );
        else if ( 'a' <= *p && *p <= 'z' )
            n = 10 + ( *p - 'a' );
        else
            n = nBase;

        if( n < nBase )
        {
            if( bFirstDig )
            {
                bFirstDig = sal_False;
                nFirstDig = n;
            }
            fVal = fVal * fBase + double( n );
        }
        else
            // illegal char!
            THROW_IAE;

        p++;

    }

    if( nStrLen == nCharLim && !bFirstDig && (nFirstDig >= nBase / 2) )
    {   // handling negativ values
        fVal = ( pow( double( nBase ), double( nCharLim ) ) - fVal );   // complement
        fVal *= -1.0;
    }

    return fVal;
}


static inline sal_Char GetMaxChar( sal_uInt16 nBase )
{
    const sal_Char* c = "--123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return c[ nBase ];
}


STRING ConvertFromDec( double fNum, double fMin, double fMax, sal_uInt16 nBase,
    sal_Int32 nPlaces, sal_Int32 nMaxPlaces, sal_Bool bUsePlaces ) THROWDEF_RTE_IAE
{
    fNum = ::rtl::math::approxFloor( fNum );
    fMin = ::rtl::math::approxFloor( fMin );
    fMax = ::rtl::math::approxFloor( fMax );

    if( fNum < fMin || fNum > fMax || ( bUsePlaces && ( nPlaces <= 0 || nPlaces > nMaxPlaces ) ) )
        THROW_IAE;

    sal_Int64 nNum = static_cast< sal_Int64 >( fNum );
    sal_Bool        bNeg = nNum < 0;
    if( bNeg )
        nNum = sal_Int64( pow( double( nBase ), double( nMaxPlaces ) ) ) + nNum;

    STRING          aRet( STRING::valueOf( nNum, nBase ).toAsciiUpperCase() );


    if( bUsePlaces )
    {
        sal_Int32 nLen = aRet.getLength();
        if( !bNeg && nLen > nPlaces )
        {
            THROW_IAE;
        }
        else if( ( bNeg && nLen < nMaxPlaces ) || ( !bNeg && nLen < nPlaces ) )
        {
            sal_Int32   nLeft = nPlaces - nLen;
            sal_Char*   p = new sal_Char[ nLeft + 1 ];
            memset( p, bNeg? GetMaxChar( nBase ) : '0', nLeft );
            p[ nLeft ] = 0x00;
            STRING  aTmp( p, nLeft, RTL_TEXTENCODING_MS_1252 );
            aTmp += aRet;
            aRet = aTmp;

            delete[] p;
        }
    }

    return aRet;
}

// implementation moved to module sal, see #i97091#
double Erf( double x )
{
    return ::rtl::math::erf(x);
}

// implementation moved to module sal, see #i97091#
double Erfc( double x )
{
    return ::rtl::math::erfc(x);
}

inline sal_Bool IsNum( sal_Unicode c )
{
    return c >= '0' && c <= '9';
}


inline sal_Bool IsComma( sal_Unicode c )
{
    return c == '.' || c == ',';
}


inline sal_Bool IsExpStart( sal_Unicode c )
{
    return c == 'e' || c == 'E';
}


inline sal_Bool IsImagUnit( sal_Unicode c )
{
    return c == 'i' || c == 'j';
}


inline sal_uInt16 GetVal( sal_Unicode c )
{
    return sal_uInt16( c - '0' );
}


sal_Bool ParseDouble( const sal_Unicode*& rp, double& rRet )
{
    double              fInt = 0.0;
    double              fFrac = 0.0;
    double              fMult = 0.1;    // multiplier to multiply digits with, when adding fractional ones
    sal_Int32           nExp = 0;
    sal_Int32           nMaxExp = 307;
    sal_uInt16          nDigCnt = 18;   // max. number of digits to read in, rest doesn't matter

    enum State  { S_End = 0, S_Sign, S_IntStart, S_Int, S_IgnoreIntDigs, S_Frac, S_IgnoreFracDigs, S_ExpSign, S_Exp };

    State           eS = S_Sign;

    sal_Bool            bNegNum = sal_False;
    sal_Bool            bNegExp = sal_False;

    const sal_Unicode*  p = rp;
    sal_Unicode         c;

    while( eS )
    {
        c = *p;
        switch( eS )
        {
            case S_Sign:
                if( IsNum( c ) )
                {
                    fInt = GetVal( c );
                    nDigCnt--;
                    eS = S_Int;
                }
                else if( c == '-' )
                {
                    bNegNum = sal_True;
                    eS = S_IntStart;
                }
                else if( c == '+' )
                    eS = S_IntStart;
                else if( IsComma( c ) )
                    eS = S_Frac;
                else
                    return sal_False;
                break;
            case S_IntStart:
                if( IsNum( c ) )
                {
                    fInt = GetVal( c );
                    nDigCnt--;
                    eS = S_Int;
                }
                else if( IsComma( c ) )
                    eS = S_Frac;
                else if( IsImagUnit( c ) )
                {
                    rRet = 0.0;
                    return sal_True;
                }
                else
                    return sal_False;
                break;
            case S_Int:
                if( IsNum( c ) )
                {
                    fInt *= 10.0;
                    fInt += double( GetVal( c ) );
                    nDigCnt--;
                    if( !nDigCnt )
                        eS = S_IgnoreIntDigs;
                }
                else if( IsComma( c ) )
                    eS = S_Frac;
                else if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else
                    eS = S_End;
                break;
            case S_IgnoreIntDigs:
                if( IsNum( c ) )
                    nExp++;         // just multiply num with 10... ;-)
                else if( IsComma( c ) )
                    eS = S_Frac;
                else if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else
                    eS = S_End;
                break;
            case S_Frac:
                if( IsNum( c ) )
                {
                    fFrac += double( GetVal( c ) ) * fMult;
                    nDigCnt--;
                    if( nDigCnt )
                        fMult *= 0.1;
                    else
                        eS = S_IgnoreFracDigs;
                }
                else if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else
                    eS = S_End;
                break;
            case S_IgnoreFracDigs:
                if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else if( !IsNum( c ) )
                    eS = S_End;
                break;
            case S_ExpSign:
                if( IsNum( c ) )
                {
                    nExp = GetVal( c );
                    eS = S_Exp;
                }
                else if( c == '-' )
                {
                    bNegExp = sal_True;
                    eS = S_Exp;
                }
                else if( c != '+' )
                    eS = S_End;
                break;
            case S_Exp:
                if( IsNum( c ) )
                {
                    nExp *= 10;
                    nExp += GetVal( c );
                    if( nExp > nMaxExp )
                        return sal_False;
                }
                else
                    eS = S_End;
                break;
            case S_End:     // to avoid compiler warning
                break;      // loop exits anyway
        }

        p++;
    }

    p--;        // set pointer back to last
    rp = p;

    fInt += fFrac;
    sal_Int32   nLog10 = sal_Int32( log10( fInt ) );

    if( bNegExp )
        nExp = -nExp;

    if( nLog10 + nExp > nMaxExp )
        return sal_False;

    fInt = ::rtl::math::pow10Exp( fInt, nExp );

    if( bNegNum )
        fInt = -fInt;

    rRet = fInt;

    return sal_True;
}


STRING GetString( double f, sal_Bool bLeadingSign, sal_uInt16 nMaxDig )
{
    const int       nBuff = 256;
    sal_Char        aBuff[ nBuff + 1 ];
    const char*     pFormStr = bLeadingSign? "%+.*g" : "%.*g";
    int             nLen = snprintf( aBuff, nBuff, pFormStr, int( nMaxDig ), f );
                    // you never know which underlying implementation you get ...
                    aBuff[nBuff] = 0;
                    if ( nLen < 0 || nLen > nBuff )
                        nLen = strlen( aBuff );

    STRING          aRet( aBuff, nLen, RTL_TEXTENCODING_MS_1252 );

    return aRet;
}


double GetAmordegrc( sal_Int32 nNullDate, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer,
    double fRestVal, double fPer, double fRate, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    if( nBase == 2 )
        THROW_IAE;

    sal_uInt32  nPer = sal_uInt32( fPer );
    double      fUsePer = 1.0 / fRate;
    double      fAmorCoeff;

    if( fUsePer < 3.0 )
        fAmorCoeff = 1.0;
    else if( fUsePer < 5.0 )
        fAmorCoeff = 1.5;
    else if( fUsePer <= 6.0 )
        fAmorCoeff = 2.0;
    else
        fAmorCoeff = 2.5;

    fRate *= fAmorCoeff;
    double      fNRate = ::rtl::math::round( GetYearFrac( nNullDate, nDate, nFirstPer, nBase ) * fRate * fCost, 0 );
    fCost -= fNRate;
    double      fRest = fCost - fRestVal;   // Anschaffungskosten - Restwert - Summe aller Abschreibungen

    for( sal_uInt32 n = 0 ; n < nPer ; n++ )
    {
        fNRate = ::rtl::math::round( fRate * fCost, 0 );
        fRest -= fNRate;

        if( fRest < 0.0 )
        {
            switch( nPer - n )
            {
                case 0:
                case 1:
                    return ::rtl::math::round( fCost * 0.5, 0 );
                default:
                    return 0.0;
            }
        }

        fCost -= fNRate;
    }

    return fNRate;
}


double GetAmorlinc( sal_Int32 nNullDate, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer,
    double fRestVal, double fPer, double fRate, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    if( nBase == 2 )
        THROW_IAE;

    sal_uInt32  nPer = sal_uInt32( fPer );
    double      fOneRate = fCost * fRate;
    double      fCostDelta = fCost - fRestVal;
    double      f0Rate = GetYearFrac( nNullDate, nDate, nFirstPer, nBase ) * fRate * fCost;
    sal_uInt32  nNumOfFullPeriods = sal_uInt32( ( fCost - fRestVal - f0Rate) / fOneRate );

    if( nPer == 0 )
        return f0Rate;
    else if( nPer <= nNumOfFullPeriods )
        return fOneRate;
    else if( nPer == nNumOfFullPeriods + 1 )
        return fCostDelta - fOneRate * nNumOfFullPeriods - f0Rate;
    else
        return 0.0;
}


double GetDuration( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fCoup,
    double fYield, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    double          fYearfrac = GetYearFrac( nNullDate, nSettle, nMat, nBase );
    double          fNumOfCoups = GetCoupnum( nNullDate, nSettle, nMat, nFreq, nBase );
    double          fDur = 0.0;
    const double    f100 = 100.0;
    fCoup *= f100 / double( nFreq );    // fCoup is used as cash flow
    fYield /= nFreq;
    fYield += 1.0;

    double nDiff = fYearfrac * nFreq - fNumOfCoups;

    double          t;

    for( t = 1.0 ; t < fNumOfCoups ; t++ )
        fDur += ( t + nDiff ) * ( fCoup ) / pow( fYield, t + nDiff );

    fDur += ( fNumOfCoups + nDiff ) * ( fCoup + f100 ) / pow( fYield, fNumOfCoups + nDiff );

    double          p = 0.0;
    for( t = 1.0 ; t < fNumOfCoups ; t++ )
        p += fCoup / pow( fYield, t + nDiff );

    p += ( fCoup + f100 ) / pow( fYield, fNumOfCoups + nDiff );

    fDur /= p;
    fDur /= double( nFreq );

    return fDur;
}


double GetYieldmat( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
    double fRate, double fPrice, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    double      fIssMat = GetYearFrac( nNullDate, nIssue, nMat, nBase );
    double      fIssSet = GetYearFrac( nNullDate, nIssue, nSettle, nBase );
    double      fSetMat = GetYearFrac( nNullDate, nSettle, nMat, nBase );

    double      y = 1.0 + fIssMat * fRate;
    y /= fPrice / 100.0 + fIssSet * fRate;
    y--;
    y /= fSetMat;

    return y;
}


double GetOddfprice( sal_Int32 /*nNullDate*/, sal_Int32 /*nSettle*/, sal_Int32 /*nMat*/, sal_Int32 /*nIssue*/,
    sal_Int32 /*nFirstCoup*/, double /*fRate*/, double /*fYield*/, double /*fRedemp*/, sal_Int32 /*nFreq*/,
    sal_Int32 /*nBase*/ ) THROWDEF_RTE_IAE
{
    THROW_RTE;  // #87380#
}


double getYield_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice,
                    double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    double      fRate = fCoup;
    double      fPriceN = 0.0;
    double      fYield1 = 0.0;
    double      fYield2 = 1.0;
    double      fPrice1 = getPrice_( nNullDate, nSettle, nMat, fRate, fYield1, fRedemp, nFreq, nBase );
    double      fPrice2 = getPrice_( nNullDate, nSettle, nMat, fRate, fYield2, fRedemp, nFreq, nBase );
    double      fYieldN = ( fYield2 - fYield1 ) * 0.5;

    for( sal_uInt32 nIter = 0 ; nIter < 100 && fPriceN != fPrice ; nIter++ )
    {
        fPriceN = getPrice_( nNullDate, nSettle, nMat, fRate, fYieldN, fRedemp, nFreq, nBase );

        if( fPrice == fPrice1 )
            return fYield1;
        else if( fPrice == fPrice2 )
            return fYield2;
        else if( fPrice == fPriceN )
            return fYieldN;
        else if( fPrice < fPrice2 )
        {
            fYield2 *= 2.0;
            fPrice2 = getPrice_( nNullDate, nSettle, nMat, fRate, fYield2, fRedemp, nFreq, nBase );

            fYieldN = ( fYield2 - fYield1 ) * 0.5;
        }
        else
        {
            if( fPrice < fPriceN )
            {
                fYield1 = fYieldN;
                fPrice1 = fPriceN;
            }
            else
            {
                fYield2 = fYieldN;
                fPrice2 = fPriceN;
            }

            fYieldN = fYield2 - ( fYield2 - fYield1 ) * ( ( fPrice - fPrice2 ) / ( fPrice1 - fPrice2 ) );
        }
    }

    if( fabs( fPrice - fPriceN ) > fPrice / 100.0 )
        THROW_IAE;      // result not precise enough

    return fYieldN;
}


double getPrice_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield,
                    double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    double      fFreq = nFreq;

    double      fE = GetCoupdays( nNullDate, nSettle, nMat, nFreq, nBase );
    double      fDSC_E = GetCoupdaysnc( nNullDate, nSettle, nMat, nFreq, nBase ) / fE;
    double      fN = GetCoupnum( nNullDate, nSettle, nMat, nFreq, nBase );
    double      fA = GetCoupdaybs( nNullDate, nSettle, nMat, nFreq, nBase );

    double      fRet = fRedemp / ( pow( 1.0 + fYield / fFreq, fN - 1.0 + fDSC_E ) );
    fRet -= 100.0 * fRate / fFreq * fA / fE;

    double      fT1 = 100.0 * fRate / fFreq;
    double      fT2 = 1.0 + fYield / fFreq;

    for( double fK = 0.0 ; fK < fN ; fK++ )
        fRet += fT1 / pow( fT2, fK + fDSC_E );

    return fRet;
}


double GetOddfyield( sal_Int32 /*nNullDate*/, sal_Int32 /*nSettle*/, sal_Int32 /*nMat*/, sal_Int32 /*nIssue*/,
    sal_Int32 /*nFirstCoup*/, double /*fRate*/, double /*fPrice*/, double /*fRedemp*/, sal_Int32 /*nFreq*/,
    sal_Int32 /*nBase*/ ) THROWDEF_RTE_IAE
{
    THROW_RTE;  // #87380#
}


double GetOddlprice( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastCoup,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    double      fFreq = double( nFreq );
    double      fDCi = GetYearFrac( nNullDate, nLastCoup, nMat, nBase ) * fFreq;
    double      fDSCi = GetYearFrac( nNullDate, nSettle, nMat, nBase ) * fFreq;
    double      fAi = GetYearFrac( nNullDate, nLastCoup, nSettle, nBase ) * fFreq;

    double      p = fRedemp + fDCi * 100.0 * fRate / fFreq;
    p /= fDSCi * fYield / fFreq + 1.0;
    p -= fAi * 100.0 * fRate / fFreq;

    return p;
}


double GetOddlyield( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastCoup,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    double      fFreq = double( nFreq );
    double      fDCi = GetYearFrac( nNullDate, nLastCoup, nMat, nBase ) * fFreq;
    double      fDSCi = GetYearFrac( nNullDate, nSettle, nMat, nBase ) * fFreq;
    double      fAi = GetYearFrac( nNullDate, nLastCoup, nSettle, nBase ) * fFreq;

    double      y = fRedemp + fDCi * 100.0 * fRate / fFreq;
    y /= fPrice + fAi * 100.0 * fRate / fFreq;
    y--;
    y *= fFreq / fDSCi;

    return y;
}


double GetRmz( double fZins, double fZzr, double fBw, double fZw, sal_Int32 nF )
{
    double      fRmz;
    if( fZins == 0.0 )
        fRmz = ( fBw + fZw ) / fZzr;
    else
    {
        double  fTerm = pow( 1.0 + fZins, fZzr );
        if( nF > 0 )
            fRmz = ( fZw * fZins / ( fTerm - 1.0 ) + fBw * fZins / ( 1.0 - 1.0 / fTerm ) ) / ( 1.0 + fZins );
        else
            fRmz = fZw * fZins / ( fTerm - 1.0 ) + fBw * fZins / ( 1.0 - 1.0 / fTerm );
    }

    return -fRmz;
}


double GetZw( double fZins, double fZzr, double fRmz, double fBw, sal_Int32 nF )
{
    double      fZw;
    if( fZins == 0.0 )
        fZw = fBw + fRmz * fZzr;
    else
    {
        double  fTerm = pow( 1.0 + fZins, fZzr );
        if( nF > 0 )
            fZw = fBw * fTerm + fRmz * ( 1.0 + fZins ) * ( fTerm - 1.0 ) / fZins;
        else
            fZw = fBw * fTerm + fRmz * ( fTerm - 1.0 ) / fZins;
    }

    return -fZw;
}


//-----------------------------------------------------------------------------
// financial functions COUP***


//-------
// COUPPCD: find last coupon date before settlement (can be equal to settlement)
void lcl_GetCouppcd( ScaDate& rDate, const ScaDate& rSettle, const ScaDate& rMat, sal_Int32 nFreq )
    throw( lang::IllegalArgumentException )
{
    rDate = rMat;
    rDate.setYear( rSettle.getYear() );
    if( rDate < rSettle )
        rDate.addYears( 1 );
    while( rDate > rSettle )
        rDate.addMonths( -12 / nFreq );
}

double GetCouppcd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    ScaDate aDate;
    lcl_GetCouppcd( aDate, ScaDate( nNullDate, nSettle, nBase ), ScaDate( nNullDate, nMat, nBase ), nFreq );
    return aDate.getDate( nNullDate );
}


//-------
// COUPNCD: find first coupon date after settlement (is never equal to settlement)
void lcl_GetCoupncd( ScaDate& rDate, const ScaDate& rSettle, const ScaDate& rMat, sal_Int32 nFreq )
    throw( lang::IllegalArgumentException )
{
    rDate = rMat;
    rDate.setYear( rSettle.getYear() );
    if( rDate > rSettle )
        rDate.addYears( -1 );
    while( rDate <= rSettle )
        rDate.addMonths( 12 / nFreq );
}

double GetCoupncd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    ScaDate aDate;
    lcl_GetCoupncd( aDate, ScaDate( nNullDate, nSettle, nBase ), ScaDate( nNullDate, nMat, nBase ), nFreq );
    return aDate.getDate( nNullDate );
}


//-------
// COUPDAYBS: get day count: coupon date before settlement <-> settlement
double GetCoupdaybs( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    ScaDate aSettle( nNullDate, nSettle, nBase );
    ScaDate aDate;
    lcl_GetCouppcd( aDate, aSettle, ScaDate( nNullDate, nMat, nBase ), nFreq );
    return ScaDate::getDiff( aDate, aSettle );
}


//-------
// COUPDAYSNC: get day count: settlement <-> coupon date after settlement
double GetCoupdaysnc( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    if( (nBase != 0) && (nBase != 4) )
    {
        ScaDate aSettle( nNullDate, nSettle, nBase );
        ScaDate aDate;
        lcl_GetCoupncd( aDate, aSettle, ScaDate( nNullDate, nMat, nBase ), nFreq );
        return ScaDate::getDiff( aSettle, aDate );
    }
    return GetCoupdays( nNullDate, nSettle, nMat, nFreq, nBase ) - GetCoupdaybs( nNullDate, nSettle, nMat, nFreq, nBase );
}


//-------
// COUPDAYS: get day count: coupon date before settlement <-> coupon date after settlement
double GetCoupdays( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    if( nBase == 1 )
    {
        ScaDate aDate;
        lcl_GetCouppcd( aDate, ScaDate( nNullDate, nSettle, nBase ), ScaDate( nNullDate, nMat, nBase ), nFreq );
        ScaDate aNextDate( aDate );
        aNextDate.addMonths( 12 / nFreq );
        return ScaDate::getDiff( aDate, aNextDate );
    }
    return static_cast< double >( GetDaysInYear( 0, 0, nBase ) ) / nFreq;
}


//-------
// COUPNUM: get count of coupon dates
double GetCoupnum( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    ScaDate aMat( nNullDate, nMat, nBase );
    ScaDate aDate;
    lcl_GetCouppcd( aDate, ScaDate( nNullDate, nSettle, nBase ), aMat, nFreq );
    sal_uInt16 nMonths = (aMat.getYear() - aDate.getYear()) * 12 + aMat.getMonth() - aDate.getMonth();
    return static_cast< double >( nMonths * nFreq / 12 );
}







const sal_uInt32 MyList::nStartSize = 16;
const sal_uInt32 MyList::nIncrSize = 16;


void MyList::_Grow( void )
{
    nSize += nIncrSize;

    void**          pNewData = new void*[ nSize ];
    memcpy( pNewData, pData, nNew * sizeof( void* ) );

    delete[] pData;
    pData = pNewData;
}


MyList::MyList( void )
{
    nSize = nStartSize;
    pData = new void*[ nSize ];
    nNew = nAct = 0;
}


MyList::~MyList()
{
    delete[] pData;
}


void MyList::Insert( void* p, sal_uInt32 n )
{
    if( n >= nNew )
        Append( p );
    else
    {
        Grow();

        void**      pIns = pData + n;
        memmove( pIns + 1, pIns, ( nNew - n ) * sizeof( void* ) );

        *pIns = p;

        nNew++;
    }
}




StringList::~StringList()
{
    for( STRING* p = ( STRING* ) First() ; p ; p = ( STRING* ) Next() )
        delete p;
}


class AnalysisRscStrArrLoader : public Resource
{
private:
    ResStringArray          aStrArray;
public:
                            AnalysisRscStrArrLoader( sal_uInt16 nRsc, sal_uInt16 nArrayId, ResMgr& rResMgr ) :
                                Resource( AnalysisResId( nRsc, rResMgr ) ),
                                aStrArray( AnalysisResId( nArrayId, rResMgr ) )
                            {
                                FreeResource();
                            }

    const ResStringArray&   GetStringArray() const { return aStrArray; }
};




FuncData::FuncData( const FuncDataBase& r, ResMgr& rResMgr ) :
    aIntName( OUString::createFromAscii( r.pIntName ) ),
    nUINameID( r.nUINameID ),
    nDescrID( r.nDescrID ),
    bDouble( r.bDouble ),
    bWithOpt( r.bWithOpt ),
    nParam( r.nNumOfParams ),
    nCompID( r.nCompListID ),
    eCat( r.eCat )
{
    AnalysisRscStrArrLoader aArrLoader( RID_ANALYSIS_DEFFUNCTION_NAMES, nCompID, rResMgr );
    const ResStringArray&   rArr = aArrLoader.GetStringArray();

    sal_uInt16              nCount = sal::static_int_cast<sal_uInt16>( rArr.Count() );
    sal_uInt16              n;

    for( n = 0 ; n < nCount ; n++ )
        aCompList.Append( rArr.GetString( n ) );
}


FuncData::~FuncData()
{
}


sal_uInt16 FuncData::GetStrIndex( sal_uInt16 nParamNum ) const
{
    if( !bWithOpt )
        nParamNum++;

    if( nParamNum > nParam )
        return nParam * 2;
    else
        return nParamNum * 2;
}




FuncDataList::FuncDataList( ResMgr& rResMgr )
{
    for( sal_uInt16 n = 0 ; n < SAL_N_ELEMENTS(pFuncDatas) ; n++ )
        Append( new FuncData( pFuncDatas[ n ], rResMgr ) );
}


FuncDataList::~FuncDataList()
{
    for( FuncData* p = ( FuncData* ) First() ; p ; p = ( FuncData* ) Next() )
        delete p;
}


const FuncData* FuncDataList::Get(  const OUString& aProgrammaticName ) const
{
    if( aLastName == aProgrammaticName )
        return Get( nLast );

    ( ( FuncDataList* ) this )->aLastName = aProgrammaticName;

    sal_uInt32  nE = Count();
    for( sal_uInt32 n = 0 ; n < nE ; n++ )
    {
        const FuncData* p = Get( n );
        if( p->Is( aProgrammaticName ) )
        {
            ( ( FuncDataList* ) this )->nLast = n;
            return p;
        }
    }

    ( ( FuncDataList* ) this )->nLast = 0xFFFFFFFF;
    return NULL;
}


AnalysisResId::AnalysisResId( sal_uInt16 nId, ResMgr& rResMgr ) : ResId( nId, rResMgr )
{
}




SortedIndividualInt32List::SortedIndividualInt32List()
{
}


SortedIndividualInt32List::~SortedIndividualInt32List()
{
}


void SortedIndividualInt32List::Insert( sal_Int32 nDay )
{
    sal_uInt32 nIndex = Count();
    while( nIndex )
    {
        nIndex--;
        sal_Int32 nRef = Get( nIndex );
        if( nDay == nRef )
            return;
        else if( nDay > nRef )
        {
            MyList::Insert( (void*)(sal_IntPtr)nDay, nIndex + 1 );
            return;
        }
    }
    MyList::Insert( (void*)(sal_IntPtr)nDay, 0UL );
}


void SortedIndividualInt32List::Insert( sal_Int32 nDay, sal_Int32 nNullDate, sal_Bool bInsertOnWeekend )
{
    if( !nDay )
        return;

    nDay += nNullDate;
    if( bInsertOnWeekend || (GetDayOfWeek( nDay ) < 5) )
        Insert( nDay );
}


void SortedIndividualInt32List::Insert(
        double fDay, sal_Int32 nNullDate, sal_Bool bInsertOnWeekend ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( (fDay < -2147483648.0) || (fDay > 2147483649.0) )
        throw lang::IllegalArgumentException();
    Insert( static_cast< sal_Int32 >( fDay ), nNullDate, bInsertOnWeekend );
}


sal_Bool SortedIndividualInt32List::Find( sal_Int32 nVal ) const
{
    sal_uInt32  nE = Count();

    if( !nE || nVal < Get( 0 ) || nVal > Get( nE - 1 ) )
        return sal_False;

    // linear search

    for( sal_uInt32 n = 0 ; n < nE ; n++ )
    {
        sal_Int32   nRef = Get( n );

        if( nRef == nVal )
            return sal_True;
        else if( nRef > nVal )
            return sal_False;
    }
    return sal_False;
}


void SortedIndividualInt32List::InsertHolidayList(
        const ScaAnyConverter& rAnyConv,
        const uno::Any& rHolAny,
        sal_Int32 nNullDate,
        sal_Bool bInsertOnWeekend ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    double fDay;
    if( rAnyConv.getDouble( fDay, rHolAny ) )
        Insert( fDay, nNullDate, bInsertOnWeekend );
}


void SortedIndividualInt32List::InsertHolidayList(
        ScaAnyConverter& rAnyConv,
        const uno::Reference< beans::XPropertySet >& xOptions,
        const uno::Any& rHolAny,
        sal_Int32 nNullDate,
        sal_Bool bInsertOnWeekend ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    rAnyConv.init( xOptions );
    if( rHolAny.getValueTypeClass() == uno::TypeClass_SEQUENCE )
    {
        uno::Sequence< uno::Sequence< uno::Any > > aAnySeq;
        if( rHolAny >>= aAnySeq )
        {
            const uno::Sequence< uno::Any >* pSeqArray = aAnySeq.getConstArray();
            for( sal_Int32 nIndex1 = 0; nIndex1 < aAnySeq.getLength(); nIndex1++ )
            {
                const uno::Sequence< uno::Any >& rSubSeq = pSeqArray[ nIndex1 ];
                const uno::Any* pAnyArray = rSubSeq.getConstArray();

                for( sal_Int32 nIndex2 = 0; nIndex2 < rSubSeq.getLength(); nIndex2++ )
                    InsertHolidayList( rAnyConv, pAnyArray[ nIndex2 ], nNullDate, bInsertOnWeekend );
            }
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        InsertHolidayList( rAnyConv, rHolAny, nNullDate, bInsertOnWeekend );
}



//-----------------------------------------------------------------------------

ScaDoubleList::~ScaDoubleList()
{
    for( double* pDbl = const_cast< double* >( First() ); pDbl; pDbl = const_cast< double* >( Next() ) )
        delete pDbl;
}


void ScaDoubleList::Append(
        const uno::Sequence< uno::Sequence< double > >& rValueSeq ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    const uno::Sequence< double >* pSeqArray = rValueSeq.getConstArray();
    for( sal_Int32 nIndex1 = 0; nIndex1 < rValueSeq.getLength(); nIndex1++ )
    {
        const uno::Sequence< double >& rSubSeq = pSeqArray[ nIndex1 ];
        const double* pArray = rSubSeq.getConstArray();
        for( sal_Int32 nIndex2 = 0; nIndex2 < rSubSeq.getLength(); nIndex2++ )
            Append( pArray[ nIndex2 ] );
    }
}


void ScaDoubleList::Append(
        const uno::Sequence< uno::Sequence< sal_Int32 > >& rValueSeq ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    const uno::Sequence< sal_Int32 >* pSeqArray = rValueSeq.getConstArray();
    for( sal_Int32 nIndex1 = 0; nIndex1 < rValueSeq.getLength(); nIndex1++ )
    {
        const uno::Sequence< sal_Int32 >& rSubSeq = pSeqArray[ nIndex1 ];
        const sal_Int32* pArray = rSubSeq.getConstArray();
        for( sal_Int32 nIndex2 = 0; nIndex2 < rSubSeq.getLength(); nIndex2++ )
            Append( pArray[ nIndex2 ] );
    }
}



void ScaDoubleList::Append(
        const ScaAnyConverter& rAnyConv,
        const uno::Any& rAny,
        sal_Bool bIgnoreEmpty ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( rAny.getValueTypeClass() == uno::TypeClass_SEQUENCE )
        Append( rAnyConv, *static_cast< const uno::Sequence< uno::Sequence< uno::Any > >* >( rAny.getValue() ), bIgnoreEmpty );
    else
    {
        double fValue;
        if( rAnyConv.getDouble( fValue, rAny ) )
            Append( fValue );
        else if( !bIgnoreEmpty )
            Append( 0.0 );
    }
}


void ScaDoubleList::Append(
        const ScaAnyConverter& rAnyConv,
        const uno::Sequence< uno::Any >& rAnySeq,
        sal_Bool bIgnoreEmpty ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    const uno::Any* pArray = rAnySeq.getConstArray();
    for( sal_Int32 nIndex = 0; nIndex < rAnySeq.getLength(); nIndex++ )
        Append( rAnyConv, pArray[ nIndex ], bIgnoreEmpty );
}


void ScaDoubleList::Append(
        const ScaAnyConverter& rAnyConv,
        const uno::Sequence< uno::Sequence< uno::Any > >& rAnySeq,
        sal_Bool bIgnoreEmpty ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    const uno::Sequence< uno::Any >* pArray = rAnySeq.getConstArray();
    for( sal_Int32 nIndex = 0; nIndex < rAnySeq.getLength(); nIndex++ )
        Append( rAnyConv, pArray[ nIndex ], bIgnoreEmpty );
}



void ScaDoubleList::Append(
        ScaAnyConverter& rAnyConv,
        const uno::Reference< beans::XPropertySet >& xOpt,
        const uno::Sequence< uno::Any >& rAnySeq,
        sal_Bool bIgnoreEmpty ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    rAnyConv.init( xOpt );
    Append( rAnyConv, rAnySeq, bIgnoreEmpty );
}


sal_Bool ScaDoubleList::CheckInsert( double ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    return sal_True;
}



//-----------------------------------------------------------------------------

sal_Bool ScaDoubleListGT0::CheckInsert( double fValue ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( fValue < 0.0 )
        throw lang::IllegalArgumentException();
    return fValue > 0.0;
}



//-----------------------------------------------------------------------------

sal_Bool ScaDoubleListGE0::CheckInsert( double fValue ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( fValue < 0.0 )
        throw lang::IllegalArgumentException();
    return sal_True;
}



//-----------------------------------------------------------------------------

Complex::Complex( const STRING& rStr ) THROWDEF_RTE_IAE
{
    if( !ParseString( rStr, *this ) )
        THROW_IAE;
}


inline sal_Bool Complex::IsImagUnit( sal_Unicode c )
{
    return c == 'i' || c == 'j';
}

sal_Bool Complex::ParseString( const STRING& rStr, Complex& rCompl )
{
    rCompl.c = '\0';    // do not force a symbol, if only real part present

    const sal_Unicode* pStr = rStr.getStr();

    if( IsImagUnit( *pStr ) && rStr.getLength() == 1)
    {
        rCompl.r = 0.0;
        rCompl.i = 1.0;
        rCompl.c = *pStr;
        return sal_True;
    }

    double                  f;

    if( !ParseDouble( pStr, f ) )
        return sal_False;

    switch( *pStr )
    {
        case '-':   // imag part follows
        case '+':
            {
            double      r = f;
            if( IsImagUnit( pStr[ 1 ] ) )
            {
                rCompl.c = pStr[ 1 ];
                if( pStr[ 2 ] == 0 )
                {
                    rCompl.r = f;
                    rCompl.i = ( *pStr == '+' )? 1.0 : -1.0;
                    return sal_True;
                }
            }
            else if( ParseDouble( pStr, f ) && IsImagUnit( *pStr ) )
            {
                rCompl.c = *pStr;
                pStr++;
                if( *pStr == 0 )
                {
                    rCompl.r = r;
                    rCompl.i = f;
                    return sal_True;
                }
            }
            }
            break;
        case 'j':
        case 'i':
            rCompl.c = *pStr;
            pStr++;
            if( *pStr == 0 )
            {
                rCompl.i = f;
                rCompl.r = 0.0;
                return sal_True;
            }
            break;
        case 0:     // only real-part
            rCompl.r = f;
            rCompl.i = 0.0;
            return sal_True;
    }

    return sal_False;
}


STRING Complex::GetString() const THROWDEF_RTE_IAE
{
    CHK_FINITE(r);
    CHK_FINITE(i);
    rtl::OUStringBuffer aRet;

    bool bHasImag = i != 0.0;
    bool bHasReal = !bHasImag || (r != 0.0);

    if( bHasReal )
        aRet.append(::GetString( r ));
    if( bHasImag )
    {
        if( i == 1.0 )
        {
            if( bHasReal )
                aRet.append('+');
        }
        else if( i == -1.0 )
            aRet.append('-');
        else
            aRet.append(::GetString( i, bHasReal ));
        aRet.append((c != 'j') ? 'i' : 'j');
    }

    return aRet.makeStringAndClear();
}


double Complex::Arg( void ) const THROWDEF_RTE_IAE
{
    if( r == 0.0 && i == 0.0 )
        THROW_IAE;

    double  phi = acos( r / Abs() );

    if( i < 0.0 )
        phi = -phi;

    return phi;
}


void Complex::Power( double fPower ) THROWDEF_RTE_IAE
{
    if( r == 0.0 && i == 0.0 )
    {
        if( fPower > 0 )
        {
            r = i = 0.0;
            return;
        }
        else
            THROW_IAE;
    }

    double      p, phi;

    p = Abs();

    phi = acos( r / p );
    if( i < 0.0 )
        phi = -phi;

    p = pow( p, fPower );
    phi *= fPower;

    r = cos( phi ) * p;
    i = sin( phi ) * p;
}


void Complex::Sqrt( void )
{
    static const double fMultConst = 0.7071067811865475;    // ...2440084436210485 = 1/sqrt(2)
    double  p = Abs();
    double  i_ = sqrt( p - r ) * fMultConst;

    r = sqrt( p + r ) * fMultConst;
    i = ( i < 0.0 )? -i_ : i_;
}


void Complex::Sin( void ) THROWDEF_RTE_IAE
{
    if( !::rtl::math::isValidArcArg( r ) )
        THROW_IAE;

    if( i )
    {
        double  r_;

        r_ = sin( r ) * cosh( i );
        i = cos( r ) * sinh( i );
        r = r_;
    }
    else
        r = sin( r );
}


void Complex::Cos( void ) THROWDEF_RTE_IAE
{
	if( !::rtl::math::isValidArcArg( r ) )
        THROW_IAE;

    if( i )
    {
        double      r_;

        r_ = cos( r ) * cosh( i );
        i = -( sin( r ) * sinh( i ) );
        r = r_;
    }
    else
        r = cos( r );
}


void Complex::Div( const Complex& z ) THROWDEF_RTE_IAE
{
    if( z.r == 0 && z.i == 0 )
        THROW_IAE;

    double  a1 = r;
    double  a2 = z.r;
    double  b1 = i;
    double  b2 = z.i;

    double  f = 1.0 / ( a2 * a2 + b2 * b2 );

    r = ( a1 * a2 + b1 * b2 ) * f;
    i = ( a2 * b1 - a1 * b2 ) * f;

    if( !c ) c = z.c;
}


void Complex::Exp( void )
{
    double  fE = exp( r );
    r = fE * cos( i );
    i = fE * sin( i );
}


void Complex::Ln( void ) THROWDEF_RTE_IAE
{
    if( r == 0.0 && i == 0.0 )
        THROW_IAE;

    double      fAbs = Abs();
    sal_Bool    bNegi = i < 0.0;

    i = acos( r / fAbs );

    if( bNegi )
        i = -i;

    r = log( fAbs );
}


void Complex::Log10( void ) THROWDEF_RTE_IAE
{
    Ln();
    Mult( 0.434294481903251828 );   // * log10( e )
}


void Complex::Log2( void ) THROWDEF_RTE_IAE
{
    Ln();
    Mult( 1.442695040888963407 );   // * log2( e )
}


void Complex::Tan(void) THROWDEF_RTE_IAE
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            THROW_IAE;
        double fScale =1.0 / ( cos( 2.0 * r ) + cosh( 2.0 * i ));
        r = sin( 2.0 * r ) * fScale;
        i = sinh( 2.0 * i ) * fScale;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            THROW_IAE;
        r = tan( r );
    }
}


void Complex::Sec( void ) THROWDEF_RTE_IAE
{
    if( i )
    {
        if( !::rtl::math::isValidArcArg( 2 * r ) )
            THROW_IAE;
        double fScale = 1.0 / (cosh( 2.0 * i) + cos ( 2.0 * r));
        double  r_;
        r_ = 2.0 * cos( r ) * cosh( i ) * fScale;
        i = 2.0 * sin( r ) * sinh( i ) * fScale;
        r = r_;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            THROW_IAE;
        r = 1.0 / cos( r );
    }
}


void Complex::Csc( void ) THROWDEF_RTE_IAE
{
    if( i )
    {
        if( !::rtl::math::isValidArcArg( 2 * r ) )
            THROW_IAE;
        double fScale = 1.0 / (cosh( 2.0 * i) - cos ( 2.0 * r));
        double  r_;
        r_ = 2.0 * sin( r ) * cosh( i ) * fScale;
        i = -2.0 * cos( r ) * sinh( i ) * fScale;
        r = r_;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            THROW_IAE;
        r = 1.0 / sin( r );
    }
}


void Complex::Cot(void) THROWDEF_RTE_IAE
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            THROW_IAE;
        double fScale =1.0 / ( cosh( 2.0 * i ) - cos( 2.0 * r ) );
        r = sin( 2.0 * r ) * fScale;
        i = - ( sinh( 2.0 * i ) * fScale );
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            THROW_IAE;
        r = 1.0 / tan( r );
    }
}


void Complex::Sinh( void ) THROWDEF_RTE_IAE
{
    if( !::rtl::math::isValidArcArg( r ) )
        THROW_IAE;

    if( i )
    {
        double	r_;
        r_ = sinh( r ) * cos( i );
		i = cosh( r ) * sin( i );
		r = r_;
	}
	else
		r = sinh( r );
}


void Complex::Cosh( void ) THROWDEF_RTE_IAE
{
    if( !::rtl::math::isValidArcArg( r ) )
        THROW_IAE;

    if( i )
    {
        double	r_;
        r_ = cosh( r ) * cos( i );
		i = sinh( r ) * sin( i );
		r = r_;
	}
	else
		r = cosh( r );
}


void Complex::Sech(void) THROWDEF_RTE_IAE
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            THROW_IAE;
        double fScale =1.0 / ( cosh( 2.0 * r ) + cos( 2.0 * i ));
        double r_;
        r_ = 2.0 * cosh( 2.0 * r ) * cos( i ) * fScale;
        i = - (2.0 * sinh( 2.0 * r ) * sin( i ) * fScale );
        r = r_ ;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            THROW_IAE;
        r = 1.0 / cosh( r );
    }
}


void Complex::Csch(void) THROWDEF_RTE_IAE
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            THROW_IAE;
        double fScale =1.0 / ( cosh( 2.0 * r ) - cos( 2.0 * i ));
        double r_;
        r_ = 2.0 * sinh( 2.0 * r ) * cos( i ) * fScale;
        i = - ( 2.0 * cosh( 2.0 * r ) * sin( i ) * fScale );
        r = r_ ;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            THROW_IAE;
        r = 1.0 / sinh( r );
    }
}


ComplexList::~ComplexList()
{
    for( Complex* p = ( Complex* ) First() ; p ; p = ( Complex* ) Next() )
        delete p;
}


void ComplexList::Append( const SEQSEQ( STRING )& r, ComplListAppendHandl eAH ) THROWDEF_RTE_IAE
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = r.getLength();
    sal_Int32   nE2;
    sal_Bool    bEmpty0 = eAH == AH_EmpyAs0;
    sal_Bool    bErrOnEmpty = eAH == AH_EmptyAsErr;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( STRING )&    rList = r[ n1 ];
        nE2 = rList.getLength();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
        {
            const STRING&   rStr = rList[ n2 ];

            if( !rStr.isEmpty() )
                Append( new Complex( rStr ) );
            else if( bEmpty0 )
                Append( new Complex( 0.0 ) );
            else if( bErrOnEmpty )
                THROW_IAE;
        }
    }
}


void ComplexList::Append( const SEQ( ANY )& aMultPars, ComplListAppendHandl eAH ) THROWDEF_RTE_IAE
{
    sal_Int32       nEle = aMultPars.getLength();
    sal_Bool        bEmpty0 = eAH == AH_EmpyAs0;
    sal_Bool        bErrOnEmpty = eAH == AH_EmptyAsErr;

    for( sal_Int32 i = 0 ; i < nEle ; i++ )
    {
        const ANY&  r = aMultPars[ i ];
        switch( r.getValueTypeClass() )
        {
            case uno::TypeClass_VOID:       break;
            case uno::TypeClass_STRING:
                {
                const STRING*       pStr = ( const STRING* ) r.getValue();

                if( !pStr->isEmpty() )
                    Append( new Complex( *( STRING* ) r.getValue() ) );
                else if( bEmpty0 )
                    Append( new Complex( 0.0 ) );
                else if( bErrOnEmpty )
                    THROW_IAE;
                }
                break;
            case uno::TypeClass_DOUBLE:
                Append( new Complex( *( double* ) r.getValue(), 0.0 ) );
                break;
            case uno::TypeClass_SEQUENCE:
                {
                SEQSEQ( ANY )           aValArr;
                if( r >>= aValArr )
                {
                    sal_Int32           nE = aValArr.getLength();
                    const SEQ( ANY )*   pArr = aValArr.getConstArray();
                    for( sal_Int32 n = 0 ; n < nE ; n++ )
                        Append( pArr[ n ], eAH );
                }
                else
                    THROW_IAE;
                }
                break;
            default:
                THROW_IAE;
        }
    }
}




ConvertData::ConvertData( const sal_Char p[], double fC, ConvertDataClass e, sal_Bool bPrefSupport ) : aName( p, strlen( p ), RTL_TEXTENCODING_MS_1252 )
{
    fConst = fC;
    eClass = e;
    bPrefixSupport = bPrefSupport;
}

ConvertData::~ConvertData()
{
}


sal_Int16 ConvertData::GetMatchingLevel( const STRING& rRef ) const
{
    STRING aStr = rRef;
    sal_Int32 nLen = rRef.getLength();
    sal_Int32 nIndex = rRef.lastIndexOf( '^' );
    if( nIndex > 0 && nIndex  == ( nLen - 2 ) )
    {
        const sal_Unicode*  p = aStr.getStr();
        aStr = STRING( p, nLen - 2 );
        aStr += STRING( p[ nLen - 1 ] );
    }
    if( aName.equals( aStr ) )
        return 0;
    else
    {
        const sal_Unicode*  p = aStr.getStr();

        nLen = aStr.getLength();
        bool bPref = IsPrefixSupport();
        bool bOneChar = (bPref && nLen > 1 && (aName == p + 1));
        if (bOneChar || (bPref && nLen > 2 && (aName == p + 2) &&
                    *p == 'd' && *(p+1) == 'a'))
        {
            sal_Int16       n;
            switch( *p )
            {
                case 'y':   n = -24;    break;      // yocto
                case 'z':   n = -21;    break;      // zepto
                case 'a':   n = -18;    break;
                case 'f':   n = -15;    break;
                case 'p':   n = -12;    break;
                case 'n':   n = -9;     break;
                case 'u':   n = -6;     break;
                case 'm':   n = -3;     break;
                case 'c':   n = -2;     break;
                case 'd':
                    {
                        if ( bOneChar )
                            n = -1;                 // deci
                        else
                            n = 1;                  // deca
                    }
                    break;
                case 'e':   n = 1;      break;
                case 'h':   n = 2;      break;
                case 'k':   n = 3;      break;
                case 'M':   n = 6;      break;
                case 'G':   n = 9;      break;
                case 'T':   n = 12;     break;
                case 'P':   n = 15;     break;
                case 'E':   n = 18;     break;
                case 'Z':   n = 21;     break;      // zetta
                case 'Y':   n = 24;     break;      // yotta
                default:
                            n = INV_MATCHLEV;
            }

// We could weed some nonsense out, ODFF doesn't say so though.
#if 0
            if (n < 0 && Class() == CDC_Information)
                n = INV_MATCHLEV;   // milli-bits doesn't make sense
#endif

//! <HACK> "cm3" is not 10^-2 m^3 but 10^-6 m^3 !!! ------------------
            if( n != INV_MATCHLEV )
            {
                sal_Unicode cLast = p[ aStr.getLength() - 1 ];
                if( cLast == '2' )
                    n *= 2;
                else if( cLast == '3' )
                    n *= 3;
            }
//! </HACK> -------------------------------------------------------------------

            return n;
        }
        else if ( nLen > 2 && ( aName == p + 2 ) && ( Class() == CDC_Information ) )
        {
            const sal_Unicode*  pStr = aStr.getStr();
            if ( *(pStr + 1) != 'i')
                return INV_MATCHLEV;
            sal_Int16 n;
            switch( *pStr )
            {
                case 'k':   n = 10;      break;
                case 'M':   n = 20;      break;
                case 'G':   n = 30;      break;
                case 'T':   n = 40;      break;
                case 'P':   n = 50;      break;
                case 'E':   n = 60;      break;
                case 'Z':   n = 70;      break;
                case 'Y':   n = 80;      break;
                default:
                            n = INV_MATCHLEV;
            }
            return n;
        }
        else
            return INV_MATCHLEV;
    }
}


double ConvertData::Convert(
    double f, const ConvertData& r, sal_Int16 nLevFrom, sal_Int16 nLevTo ) const THROWDEF_RTE_IAE
{
    if( Class() != r.Class() )
        THROW_IAE;

    sal_Bool bBinFromLev = ( nLevFrom > 0 && ( nLevFrom % 10 ) == 0 );
    sal_Bool bBinToLev   = ( nLevTo > 0 && ( nLevTo % 10 ) == 0 );

    if ( Class() == CDC_Information && ( bBinFromLev || bBinToLev ) )
    {
        if ( bBinFromLev && bBinToLev )
        {
            nLevFrom = sal::static_int_cast<sal_Int16>( nLevFrom - nLevTo );
            f *= r.fConst / fConst;
            if( nLevFrom )
                f *= pow( 2.0, nLevFrom );
        }
        else if ( bBinFromLev )
            f *= ( r.fConst / fConst ) * ( pow( 2.0, nLevFrom ) / pow( 10.0, nLevTo ) );
        else
            f *= ( r.fConst / fConst ) * ( pow( 10.0, nLevFrom ) / pow( 2.0, nLevTo ) );
        return f;
    }

    nLevFrom = sal::static_int_cast<sal_Int16>( nLevFrom - nLevTo );    // effective level

    f *= r.fConst / fConst;

    if( nLevFrom )
        f = ::rtl::math::pow10Exp( f, nLevFrom );

    return f;
}


double ConvertData::ConvertToBase( double f, sal_Int16 n ) const
{
    return ::rtl::math::pow10Exp( f / fConst, n );
}


double ConvertData::ConvertFromBase( double f, sal_Int16 n ) const
{
    return ::rtl::math::pow10Exp( f * fConst, -n );
}



ConvertDataLinear::~ConvertDataLinear()
{
}

double ConvertDataLinear::Convert(
    double f, const ConvertData& r, sal_Int16 nLevFrom, sal_Int16 nLevTo ) const THROWDEF_RTE_IAE
{
    if( Class() != r.Class() )
        THROW_IAE;
    return r.ConvertFromBase( ConvertToBase( f, nLevFrom ), nLevTo );
}


double ConvertDataLinear::ConvertToBase( double f, sal_Int16 n ) const
{
    if( n )
        f = ::rtl::math::pow10Exp( f, n );

    f /= fConst;
    f -= fOffs;

    return f;
}


double ConvertDataLinear::ConvertFromBase( double f, sal_Int16 n ) const
{
    f += fOffs;
    f *= fConst;

    if( n )
        f = ::rtl::math::pow10Exp( f, -n );

    return f;
}




ConvertDataList::ConvertDataList( void )
{
#define NEWD(str,unit,cl)   Append(new ConvertData(str,unit,cl))
#define NEWDP(str,unit,cl)  Append(new ConvertData(str,unit,cl,sal_True))
#define NEWL(str,unit,offs,cl)  Append(new ConvertDataLinear(str,unit,offs,cl))
#define NEWLP(str,unit,offs,cl) Append(new ConvertDataLinear(str,unit,offs,cl,sal_True))

    // *** are extra and not standard Excel Analysis Addin!

    // MASS: 1 Gram is...
    NEWDP( "g",         1.0000000000000000E00,  CDC_Mass ); // Gram
    NEWD( "sg",         6.8522050005347800E-05, CDC_Mass ); // Pieces
    NEWD( "lbm",        2.2046229146913400E-03, CDC_Mass ); // Pound (commercial weight)
    NEWDP( "u",         6.0221370000000000E23,  CDC_Mass ); // U (atomic mass)
    NEWD( "ozm",        3.5273971800362700E-02, CDC_Mass ); // Ounce (commercial weight)
    NEWD( "stone",      1.574730e-04,           CDC_Mass ); // *** Stone
    NEWD( "ton",        1.102311e-06,           CDC_Mass ); // *** Ton
    NEWD( "grain",      1.543236E01,            CDC_Mass ); // *** Grain
    NEWD( "pweight",    7.054792E-01,           CDC_Mass ); // *** Pennyweight
    NEWD( "hweight",    1.968413E-05,           CDC_Mass ); // *** Hundredweight
    NEWD( "shweight",   2.204623E-05,           CDC_Mass ); // *** Shorthundredweight
    NEWD( "brton",      9.842065E-07,           CDC_Mass ); // *** Gross Registered Ton
    NEWD( "cwt",        2.2046226218487758E-05, CDC_Mass ); // U.S. (short) hundredweight
    NEWD( "shweight",   2.2046226218487758E-05, CDC_Mass ); // U.S. (short) hundredweight also
    NEWD( "uk_cwt",     1.9684130552221213E-05, CDC_Mass ); // Imperial hundredweight
    NEWD( "lcwt",       1.9684130552221213E-05, CDC_Mass ); // Imperial hundredweight also
    NEWD( "hweight",    1.9684130552221213E-05, CDC_Mass ); // Imperial hundredweight also
    NEWD( "uk_ton",     9.8420652761106063E-07, CDC_Mass ); // Imperial ton
    NEWD( "LTON",       9.8420652761106063E-07, CDC_Mass ); // Imperial ton also

    // LENGTH: 1 Meter is...
    NEWDP( "m",         1.0000000000000000E00,  CDC_Length ); // Meter
    NEWD( "mi",         6.2137119223733397E-04, CDC_Length ); // Britsh Mile        6,21371192237333969617434184363e-4
    NEWD( "Nmi",        5.3995680345572354E-04, CDC_Length ); // Nautical Mile      5,39956803455723542116630669546e-4
    NEWD( "in",         3.9370078740157480E01,  CDC_Length ); // Inch               39,37007874015748031496062992126
    NEWD( "ft",         3.2808398950131234E00,  CDC_Length ); // Foot               3,2808398950131233595800524934383
    NEWD( "yd",         1.0936132983377078E00,  CDC_Length ); // Yard               1,0936132983377077865266841644794
    NEWDP( "ang",       1.0000000000000000E10,  CDC_Length ); // Angstroem
    NEWD( "Pica",       2.8346456692913386E03,  CDC_Length ); // Pica (1/72 Inch)   2834,6456692913385826771653543307
    NEWD( "ell",        8.748906E-01,           CDC_Length ); // *** Ell
    NEWDP( "parsec",    3.240779E-17,           CDC_Length ); // *** Parsec
    NEWDP( "pc",        3.240779E-17,           CDC_Length ); // *** Parsec also
    NEWDP( "lightyear", 1.0570234557732930E-16, CDC_Length ); // *** Light Year
    NEWDP( "ly",        1.0570234557732930E-16, CDC_Length ); // *** Light Year also
    NEWD( "survey_mi",  6.2136994949494949E-04, CDC_Length ); // U.S. survey mile

    // TIME: 1 Second is...
    NEWD( "yr",     3.1688087814028950E-08, CDC_Time ); // Year
    NEWD( "day",    1.1574074074074074E-05, CDC_Time ); // Day
    NEWD( "d",      1.1574074074074074E-05, CDC_Time ); // Day also
    NEWD( "hr",     2.7777777777777778E-04, CDC_Time ); // Hour
    NEWD( "mn",     1.6666666666666667E-02, CDC_Time ); // Minute
    NEWD( "min",    1.6666666666666667E-02, CDC_Time ); // Minute also
    NEWDP( "sec",   1.0000000000000000E00,  CDC_Time ); // Second
    NEWDP( "s",     1.0000000000000000E00,  CDC_Time ); // Second also

    // PRESSURE: 1 Pascal is...
    NEWDP( "Pa",    1.0000000000000000E00,  CDC_Pressure ); // Pascal
    NEWDP( "atm",   9.8692329999819300E-06, CDC_Pressure ); // Atmosphere
    NEWDP( "at",    9.8692329999819300E-06, CDC_Pressure ); // Atmosphere also
    NEWDP( "mmHg",  7.5006170799862700E-03, CDC_Pressure ); // mm Hg (Mercury)
    NEWD( "Torr",   7.5006380000000000E-03, CDC_Pressure ); // *** Torr
    NEWD( "psi",    1.4503770000000000E-04, CDC_Pressure ); // *** Psi

    // FORCE: 1 Newton is...
    NEWDP( "N",     1.0000000000000000E00,  CDC_Force ); // Newton
    NEWDP( "dyn",   1.0000000000000000E05,  CDC_Force ); // Dyn
    NEWDP( "dy",    1.0000000000000000E05,  CDC_Force ); // Dyn also
    NEWD( "lbf",    2.24808923655339E-01,   CDC_Force ); // Pound-Force
    NEWDP( "pond",  1.019716E02,            CDC_Force ); // *** Pond

    // ENERGY: 1 Joule is...
    NEWDP( "J",     1.0000000000000000E00,  CDC_Energy ); // Joule
    NEWDP( "e",     1.0000000000000000E07,  CDC_Energy ); // Erg  -> http://www.chemie.fu-berlin.de/chemistry/general/si.html
    NEWDP( "c",     2.3900624947346700E-01, CDC_Energy ); // Thermodynamical Calorie
    NEWDP( "cal",   2.3884619064201700E-01, CDC_Energy ); // Calorie
    NEWDP( "eV",    6.2414570000000000E18,  CDC_Energy ); // Electronvolt
    NEWDP( "ev",    6.2414570000000000E18,  CDC_Energy ); // Electronvolt also
    NEWD( "HPh",    3.7250611111111111E-07, CDC_Energy ); // Horsepower Hours
    NEWD( "hh",     3.7250611111111111E-07, CDC_Energy ); // Horsepower Hours also
    NEWDP( "Wh",    2.7777777777777778E-04, CDC_Energy ); // Watt Hours
    NEWDP( "wh",    2.7777777777777778E-04, CDC_Energy ); // Watt Hours also
    NEWD( "flb",    2.37304222192651E01,    CDC_Energy ); // Foot Pound
    NEWD( "BTU",    9.4781506734901500E-04, CDC_Energy ); // British Thermal Unit
    NEWD( "btu",    9.4781506734901500E-04, CDC_Energy ); // British Thermal Unit also

    // POWER: 1 Watt is...
    NEWDP( "W",     1.0000000000000000E00,  CDC_Power ); // Watt
    NEWDP( "w",     1.0000000000000000E00,  CDC_Power ); // Watt also
    NEWD( "HP",     1.341022E-03,           CDC_Power ); // Horsepower
    NEWD( "h",      1.341022E-03,           CDC_Power ); // Horsepower also
    NEWD( "PS",     1.359622E-03,           CDC_Power ); // *** German Pferdestaerke

    // MAGNETISM: 1 Tesla is...
    NEWDP( "T",     1.0000000000000000E00,  CDC_Magnetism ); // Tesla
    NEWDP( "ga",    1.0000000000000000E04,  CDC_Magnetism ); // Gauss

    // TEMERATURE: 1 Kelvin is...
    NEWL( "C",      1.0000000000000000E00,  -2.7315000000000000E02, CDC_Temperature ); // Celsius
    NEWL( "cel",    1.0000000000000000E00,  -2.7315000000000000E02, CDC_Temperature ); // Celsius also
    NEWL( "F",      1.8000000000000000E00,  -2.5537222222222222E02, CDC_Temperature ); // Fahrenheit
    NEWL( "fah",    1.8000000000000000E00,  -2.5537222222222222E02, CDC_Temperature ); // Fahrenheit also
    NEWLP( "K",     1.0000000000000000E00,  +0.0000000000000000E00, CDC_Temperature ); // Kelvin
    NEWLP( "kel",   1.0000000000000000E00,  +0.0000000000000000E00, CDC_Temperature ); // Kelvin also
    NEWL( "Reau",   8.0000000000000000E-01, -2.7315000000000000E02, CDC_Temperature ); // *** Reaumur
    NEWL( "Rank",   1.8000000000000000E00,  +0.0000000000000000E00, CDC_Temperature ); // *** Rankine

    // VOLUMNE: 1 Liter is...
    NEWD( "tsp",        2.0288413621105798E02,  CDC_Volume ); // US teaspoon            1/768 gallon
    NEWD( "tbs",        6.7628045403685994E01,  CDC_Volume ); // US tablespoon          1/256 gallon
    NEWD( "oz",         3.3814022701842997E01,  CDC_Volume ); // Ounce Liquid           1/128 gallon
    NEWD( "cup",        4.2267528377303746E00,  CDC_Volume ); // Cup                    1/16 gallon
    NEWD( "pt",         2.1133764188651873E00,  CDC_Volume ); // US Pint                1/8 gallon
    NEWD( "us_pt",      2.1133764188651873E00,  CDC_Volume ); // US Pint also
    NEWD( "uk_pt",      1.7597539863927023E00,  CDC_Volume ); // UK Pint                1/8 imperial gallon
    NEWD( "qt",         1.0566882094325937E00,  CDC_Volume ); // Quart                  1/4 gallon
    NEWD( "gal",        2.6417205235814842E-01, CDC_Volume ); // Gallon                 1/3.785411784
    NEWDP( "l",         1.0000000000000000E00,  CDC_Volume ); // Liter
    NEWDP( "L",         1.0000000000000000E00,  CDC_Volume ); // Liter also
    NEWDP( "lt",        1.0000000000000000E00,  CDC_Volume ); // Liter also
    NEWDP( "m3",        1.0000000000000000E-03, CDC_Volume ); // *** Cubic Meter
    NEWD( "mi3",        2.3991275857892772E-13, CDC_Volume ); // *** Cubic Britsh Mile
    NEWD( "Nmi3",       1.5742621468581148E-13, CDC_Volume ); // *** Cubic Nautical Mile
    NEWD( "in3",        6.1023744094732284E01,  CDC_Volume ); // *** Cubic Inch
    NEWD( "ft3",        3.5314666721488590E-02, CDC_Volume ); // *** Cubic Foot
    NEWD( "yd3",        1.3079506193143922E-03, CDC_Volume ); // *** Cubic Yard
    NEWDP( "ang3",      1.0000000000000000E27,  CDC_Volume ); // *** Cubic Angstroem
    NEWD( "Pica3",      2.2776990435870636E07,  CDC_Volume ); // *** Cubic Pica
    NEWD( "barrel",     6.2898107704321051E-03, CDC_Volume ); // *** Barrel (=42gal)
    NEWD( "bushel",     2.837759E-02,           CDC_Volume ); // *** Bushel
    NEWD( "regton",     3.531467E-04,           CDC_Volume ); // *** Register ton
    NEWD( "GRT",        3.531467E-04,           CDC_Volume ); // *** Register ton also
    NEWD( "Schooner",   2.3529411764705882E00,  CDC_Volume ); // *** austr. Schooner
    NEWD( "Middy",      3.5087719298245614E00,  CDC_Volume ); // *** austr. Middy
    NEWD( "Glass",      5.0000000000000000E00,  CDC_Volume ); // *** austr. Glass
    NEWD( "Sixpack",    0.5,                    CDC_Volume ); // ***
    NEWD( "Humpen",     2.0,                    CDC_Volume ); // ***
    NEWD( "ly3",        1.1810108125623799E-51, CDC_Volume ); // *** Cubic light-year
    NEWD( "MTON",       1.4125866688595436E00,  CDC_Volume ); // *** Measurement ton
    NEWD( "tspm",       5.0000000000000000E02,  CDC_Volume ); // *** Modern teaspoon
    NEWD( "uk_gal",     2.1996924829908779E-01,  CDC_Volume ); // U.K. / Imperial gallon        1/4.54609
    NEWD( "uk_qt",      8.7987699319635115E-01,  CDC_Volume ); // U.K. / Imperial quart         1/4 imperial gallon

    // 1 Square Meter is...
    NEWDP( "m2",        1.0000000000000000E00,  CDC_Area ); // *** Square Meter
    NEWD( "mi2",        3.8610215854244585E-07, CDC_Area ); // *** Square Britsh Mile
    NEWD( "Nmi2",       2.9155334959812286E-07, CDC_Area ); // *** Square Nautical Mile
    NEWD( "in2",        1.5500031000062000E03,  CDC_Area ); // *** Square Inch
    NEWD( "ft2",        1.0763910416709722E01,  CDC_Area ); // *** Square Foot
    NEWD( "yd2",        1.1959900463010803E00,  CDC_Area ); // *** Square Yard
    NEWDP( "ang2",      1.0000000000000000E20,  CDC_Area ); // *** Square Angstroem
    NEWD( "Pica2",      8.0352160704321409E06,  CDC_Area ); // *** Square Pica
    NEWD( "Morgen",     4.0000000000000000E-04, CDC_Area ); // *** Morgen
    NEWDP( "ar",        1.000000E-02,           CDC_Area ); // *** Ar
    NEWD( "acre",       2.471053815E-04,        CDC_Area ); // *** Acre
    NEWD( "uk_acre",    2.4710538146716534E-04, CDC_Area ); // *** International acre
    NEWD( "us_acre",    2.4710439304662790E-04, CDC_Area ); // *** U.S. survey/statute acre
    NEWD( "ly2",        1.1172985860549147E-32, CDC_Area ); // *** Square Light-year
    NEWD( "ha",         1.000000E-04,           CDC_Area ); // *** Hectare
    NEWD( "Quadratlatschen",5.6689342403628117914,CDC_Area ); // ***

    // SPEED: 1 Meter per Second is...
    NEWDP( "m/s",   1.0000000000000000E00,  CDC_Speed ); // *** Meters per Second
    NEWDP( "m/sec", 1.0000000000000000E00,  CDC_Speed ); // *** Meters per Second also
    NEWDP( "m/h",   3.6000000000000000E03,  CDC_Speed ); // *** Meters per Hour
    NEWDP( "m/hr",  3.6000000000000000E03,  CDC_Speed ); // *** Meters per Hour also
    NEWD( "mph",    2.2369362920544023E00,  CDC_Speed ); // *** Britsh Miles per Hour
    NEWD( "kn",     1.9438444924406048E00,  CDC_Speed ); // *** Knot = Nautical Miles per Hour
    NEWD( "admkn",  1.9438446603753486E00,  CDC_Speed ); // *** Admiralty Knot
    NEWD( "wahnsinnige Geschwindigkeit", 2.0494886343432328E-14, CDC_Speed ); // ***
    NEWD( "ludicrous speed", 2.0494886343432328E-14, CDC_Speed ); // ***
    NEWD( "laecherliche Geschwindigkeit", 4.0156958471424288E-06, CDC_Speed); // ***
    NEWD( "ridiculous speed", 4.0156958471424288E-06, CDC_Speed); // ***

    // INFORMATION: 1 Bit is...
    NEWDP( "bit",   1.00E00,  CDC_Information); // *** Bit
    NEWDP( "byte",  1.25E-01, CDC_Information); // *** Byte
}


ConvertDataList::~ConvertDataList()
{
    for( ConvertData* p = First() ; p ; p = Next() )
        delete p;
}


double ConvertDataList::Convert( double fVal, const STRING& rFrom, const STRING& rTo ) THROWDEF_RTE_IAE
{
    ConvertData*    pFrom = NULL;
    ConvertData*    pTo = NULL;
    sal_Bool        bSearchFrom = sal_True;
    sal_Bool        bSearchTo = sal_True;
    sal_Int16       nLevelFrom = 0;
    sal_Int16       nLevelTo = 0;

    ConvertData*    p = First();
    while( p && ( bSearchFrom || bSearchTo ) )
    {
        if( bSearchFrom )
        {
            sal_Int16   n = p->GetMatchingLevel( rFrom );
            if( n != INV_MATCHLEV )
            {
                if( n )
                {   // only first match for partial equality rulz a little bit more
                    pFrom = p;
                    nLevelFrom = n;
                }
                else
                {   // ... but exact match rulz most
                    pFrom = p;
                    bSearchFrom = sal_False;
                    nLevelFrom = n;
                }
            }
        }

        if( bSearchTo )
        {
            sal_Int16   n = p->GetMatchingLevel( rTo );
            if( n != INV_MATCHLEV )
            {
                if( n )
                {   // only first match for partial equality rulz a little bit more
                    pTo = p;
                    nLevelTo = n;
                }
                else
                {   // ... but exact match rulz most
                    pTo = p;
                    bSearchTo = sal_False;
                    nLevelTo = n;
                }
            }
        }

        p = Next();
    }

    if( pFrom && pTo )
        return pFrom->Convert( fVal, *pTo, nLevelFrom, nLevelTo );
    else
        THROW_IAE;
}



//-----------------------------------------------------------------------------

ScaDate::ScaDate() :
    nOrigDay( 1 ),
    nDay( 1 ),
    nMonth( 1 ),
    nYear( 1900 ),
    bLastDayMode( sal_True ),
    bLastDay( sal_False ),
    b30Days( sal_False ),
    bUSMode( sal_False )
{
}

ScaDate::ScaDate( sal_Int32 nNullDate, sal_Int32 nDate, sal_Int32 nBase )
{
    DaysToDate( nNullDate + nDate, nOrigDay, nMonth, nYear );
    bLastDayMode = (nBase != 5);
    bLastDay = (nOrigDay >= ::DaysInMonth( nMonth, nYear ));
    b30Days = (nBase == 0) || (nBase == 4);
    bUSMode = (nBase == 0);
    setDay();
}

ScaDate::ScaDate( const ScaDate& rCopy ) :
    nOrigDay( rCopy.nOrigDay ),
    nDay( rCopy.nDay ),
    nMonth( rCopy.nMonth ),
    nYear( rCopy.nYear ),
    bLastDayMode( rCopy.bLastDayMode ),
    bLastDay( rCopy.bLastDay ),
    b30Days( rCopy.b30Days ),
    bUSMode( rCopy.bUSMode )
{
}

ScaDate& ScaDate::operator=( const ScaDate& rCopy )
{
    if( this != &rCopy )
    {
        nOrigDay = rCopy.nOrigDay;
        nDay = rCopy.nDay;
        nMonth = rCopy.nMonth;
        nYear = rCopy.nYear;
        bLastDayMode = rCopy.bLastDayMode;
        bLastDay = rCopy.bLastDay;
        b30Days = rCopy.b30Days;
        bUSMode = rCopy.bUSMode;
    }
    return *this;
}

void ScaDate::setDay()
{
    if( b30Days )
    {
        // 30-days-mode: set nDay to 30 if original was last day in month
        nDay = Min( nOrigDay, static_cast< sal_uInt16 >( 30 ) );
        if( bLastDay || (nDay >= ::DaysInMonth( nMonth, nYear )) )
            nDay = 30;
    }
    else
    {
        // set nDay to last day in this month if original was last day
        sal_uInt16 nLastDay = ::DaysInMonth( nMonth, nYear );
        nDay = bLastDay ? nLastDay : Min( nOrigDay, nLastDay );
    }
}

sal_Int32 ScaDate::getDaysInMonthRange( sal_uInt16 nFrom, sal_uInt16 nTo ) const
{
    if( nFrom > nTo )
        return 0;

    sal_Int32 nRet = 0;
    if( b30Days )
        nRet = (nTo - nFrom + 1) * 30;
    else
    {
        for( sal_uInt16 nMonthIx = nFrom; nMonthIx <= nTo; ++nMonthIx )
            nRet += getDaysInMonth( nMonthIx );
    }
    return nRet;
}

sal_Int32 ScaDate::getDaysInYearRange( sal_uInt16 nFrom, sal_uInt16 nTo ) const
{
    if( nFrom > nTo )
        return 0;

    return b30Days ? ((nTo - nFrom + 1) * 360) : ::GetDaysInYears( nFrom, nTo );
}

void ScaDate::doAddYears( sal_Int32 nYearCount ) throw( lang::IllegalArgumentException )
{
    sal_Int32 nNewYear = nYearCount + nYear;
    if( (nNewYear < 0) || (nNewYear > 0x7FFF) )
        throw lang::IllegalArgumentException();
    nYear = static_cast< sal_uInt16 >( nNewYear );
}

void ScaDate::addMonths( sal_Int32 nMonthCount ) throw( lang::IllegalArgumentException )
{
    sal_Int32 nNewMonth = nMonthCount + nMonth;
    if( nNewMonth > 12 )
    {
        --nNewMonth;
        doAddYears( nNewMonth / 12 );
        nMonth = static_cast< sal_uInt16 >( nNewMonth % 12 ) + 1;
    }
    else if( nNewMonth < 1 )
    {
        doAddYears( nNewMonth / 12 - 1 );
        nMonth = static_cast< sal_uInt16 >( nNewMonth % 12 + 12 );
    }
    else
        nMonth = static_cast< sal_uInt16 >( nNewMonth );
    setDay();
}

sal_Int32 ScaDate::getDate( sal_Int32 nNullDate ) const
{
    sal_uInt16 nLastDay = ::DaysInMonth( nMonth, nYear );
    sal_uInt16 nRealDay = (bLastDayMode && bLastDay) ? nLastDay : Min( nLastDay, nOrigDay );
    return ::DateToDays( nRealDay, nMonth, nYear ) - nNullDate;
}

sal_Int32 ScaDate::getDiff( const ScaDate& rFrom, const ScaDate& rTo ) throw( lang::IllegalArgumentException )
{
    if( rFrom > rTo )
        return getDiff( rTo, rFrom );

    sal_Int32 nDiff = 0;
    ScaDate aFrom( rFrom );
    ScaDate aTo( rTo );

    if( rTo.b30Days )
    {
        // corrections for base 0 (US NASD)
        if( rTo.bUSMode )
        {
            if( ((rFrom.nMonth == 2) || (rFrom.nDay < 30)) && (aTo.nOrigDay == 31) )
                aTo.nDay = 31;
            else if( (aTo.nMonth == 2) && aTo.bLastDay )
                aTo.nDay = ::DaysInMonth( 2, aTo.nYear );
        }
        // corrections for base 4 (Europe)
        else
        {
            if( (aFrom.nMonth == 2) && (aFrom.nDay == 30) )
                aFrom.nDay = ::DaysInMonth( 2, aFrom.nYear );
            if( (aTo.nMonth == 2) && (aTo.nDay == 30) )
                aTo.nDay = ::DaysInMonth( 2, aTo.nYear );
        }
    }

    if( (aFrom.nYear < aTo.nYear) || ((aFrom.nYear == aTo.nYear) && (aFrom.nMonth < aTo.nMonth)) )
    {
        // move aFrom to 1st day of next month
        nDiff = aFrom.getDaysInMonth() - aFrom.nDay + 1;
        aFrom.nOrigDay = aFrom.nDay = 1;
        aFrom.bLastDay = sal_False;
        aFrom.addMonths( 1 );

        if( aFrom.nYear < aTo.nYear )
        {
            // move aFrom to 1st day of next year
            nDiff += aFrom.getDaysInMonthRange( aFrom.nMonth, 12 );
            aFrom.addMonths( 13 - aFrom.nMonth );

            // move aFrom to 1st day of this year
            nDiff += aFrom.getDaysInYearRange( aFrom.nYear, aTo.nYear - 1 );
            aFrom.addYears( aTo.nYear - aFrom.nYear );
        }

        // move aFrom to 1st day of this month
        nDiff += aFrom.getDaysInMonthRange( aFrom.nMonth, aTo.nMonth - 1 );
        aFrom.addMonths( aTo.nMonth - aFrom.nMonth );
    }
    // finally add remaining days in this month
    nDiff += aTo.nDay - aFrom.nDay;
    return nDiff > 0 ? nDiff : 0;
}

sal_Bool ScaDate::operator<( const ScaDate& rCmp ) const
{
    if( nYear != rCmp.nYear )
        return nYear < rCmp.nYear;
    if( nMonth != rCmp.nMonth )
        return nMonth < rCmp.nMonth;
    if( nDay != rCmp.nDay )
        return nDay < rCmp.nDay;
    if( bLastDay || rCmp.bLastDay )
        return !bLastDay && rCmp.bLastDay;
    return nOrigDay < rCmp.nOrigDay;
}



//-----------------------------------------------------------------------------

ScaAnyConverter::ScaAnyConverter( const uno::Reference< lang::XMultiServiceFactory >& xServiceFact ) :
    bHasValidFormat( sal_False )
{
    if( xServiceFact.is() )
    {
        uno::Reference< uno::XInterface > xInstance = xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.NumberFormatter" )) );
        xFormatter = uno::Reference< util::XNumberFormatter >( xInstance, uno::UNO_QUERY );
    }
}

ScaAnyConverter::~ScaAnyConverter()
{
}

void ScaAnyConverter::init( const uno::Reference< beans::XPropertySet >& xPropSet ) throw( uno::RuntimeException )
{
    // try to get default number format
    bHasValidFormat = sal_False;
    if( xFormatter.is() )
    {
        // get XFormatsSupplier from outer XPropertySet
        uno::Reference< util::XNumberFormatsSupplier > xFormatsSupp( xPropSet, uno::UNO_QUERY );
        if( xFormatsSupp.is() )
        {
            // get XNumberFormatTypes from XNumberFormatsSupplier to get standard index
            uno::Reference< util::XNumberFormats > xFormats( xFormatsSupp->getNumberFormats() );
            uno::Reference< util::XNumberFormatTypes > xFormatTypes( xFormats, uno::UNO_QUERY );
            if( xFormatTypes.is() )
            {
                lang::Locale eLocale;
                nDefaultFormat = xFormatTypes->getStandardIndex( eLocale );
                xFormatter->attachNumberFormatsSupplier( xFormatsSupp );
                bHasValidFormat = sal_True;
            }
        }
    }
}

double ScaAnyConverter::convertToDouble( const OUString& rString ) const throw( lang::IllegalArgumentException )
{
    double fValue = 0.0;
    if( bHasValidFormat )
    {
        try
        {
            fValue = xFormatter->convertStringToNumber( nDefaultFormat, rString );
        }
        catch( uno::Exception& )
        {
            throw lang::IllegalArgumentException();
        }
    }
    else
    {
        rtl_math_ConversionStatus eStatus;
        sal_Int32 nEnd;
        fValue = ::rtl::math::stringToDouble( rString, '.', ',', &eStatus, &nEnd );
        if( (eStatus != rtl_math_ConversionStatus_Ok) || (nEnd < rString.getLength()) )
            throw lang::IllegalArgumentException();
    }
    return fValue;
}

sal_Bool ScaAnyConverter::getDouble(
        double& rfResult,
        const uno::Any& rAny ) const throw( lang::IllegalArgumentException )
{
    rfResult = 0.0;
    sal_Bool bContainsVal = sal_True;
    switch( rAny.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:
            bContainsVal = sal_False;
        break;
        case uno::TypeClass_DOUBLE:
            rAny >>= rfResult;
        break;
        case uno::TypeClass_STRING:
        {
            const OUString* pString = static_cast< const OUString* >( rAny.getValue() );
            if( !pString->isEmpty() )
                rfResult = convertToDouble( *pString );
            else
                bContainsVal = sal_False;
        }
        break;
        default:
            throw lang::IllegalArgumentException();
    }
    return bContainsVal;
}

sal_Bool ScaAnyConverter::getDouble(
        double& rfResult,
        const uno::Reference< beans::XPropertySet >& xPropSet,
        const uno::Any& rAny ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    init( xPropSet );
    return getDouble( rfResult, rAny );
}

double ScaAnyConverter::getDouble(
        const uno::Reference< beans::XPropertySet >& xPropSet,
        const uno::Any& rAny,
        double fDefault ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    double fResult;
    if( !getDouble( fResult, xPropSet, rAny ) )
        fResult = fDefault;
    return fResult;
}

sal_Bool ScaAnyConverter::getInt32(
        sal_Int32& rnResult,
        const uno::Reference< beans::XPropertySet >& xPropSet,
        const uno::Any& rAny ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    double fResult;
    sal_Bool bContainsVal = getDouble( fResult, xPropSet, rAny );
    if( (fResult <= -2147483649.0) || (fResult >= 2147483648.0) )
        throw lang::IllegalArgumentException();

    rnResult = static_cast< sal_Int32 >( fResult );
    return bContainsVal;
}

sal_Int32 ScaAnyConverter::getInt32(
        const uno::Reference< beans::XPropertySet >& xPropSet,
        const uno::Any& rAny,
        sal_Int32 nDefault ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nResult;
    if( !getInt32( nResult, xPropSet, rAny ) )
        nResult = nDefault;
    return nResult;
}



//-----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
