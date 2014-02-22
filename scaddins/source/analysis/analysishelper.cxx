/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>

#include <string.h>
#include <stdio.h>
#include <tools/resary.hxx>
#include <rtl/math.hxx>
#include <sal/macros.h>
#include "analysishelper.hxx"
#include "analysis.hrc"

using namespace                 ::com::sun::star;



#define UNIQUE              sal_False   
#define DOUBLE              sal_True    

#define STDPAR              sal_False   
#define INTPAR              sal_True    

#define FUNCDATA( FUNCNAME, DBL, OPT, NUMOFPAR, CAT ) \
    { "get" #FUNCNAME, ANALYSIS_FUNCNAME_##FUNCNAME, ANALYSIS_##FUNCNAME, DBL, OPT, ANALYSIS_DEFFUNCNAME_##FUNCNAME, NUMOFPAR, CAT }

const FuncDataBase pFuncDatas[] =
{
    
    
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

sal_Int32 GetNullDate( const uno::Reference< beans::XPropertySet >& xOpt ) throw( uno::RuntimeException )
{
    if( xOpt.is() )
    {
        try
        {
            uno::Any aAny = xOpt->getPropertyValue( "NullDate" );
            util::Date  aDate;
            if( aAny >>= aDate )
                return DateToDays( aDate.Day, aDate.Month, aDate.Year );
        }
        catch( uno::Exception& )
        {
        }
    }

    
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


sal_Int32 GetDiffDate( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode,
    sal_Int32* pOptDaysIn1stYear ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
        case 0:         
        case 4:         
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
        case 1:         
            if( pOptDaysIn1stYear )
            {
                sal_uInt16      nD, nM, nY;

                DaysToDate( nStartDate + nNullDate, nD, nM, nY );

                *pOptDaysIn1stYear = IsLeapYear( nY )? 366 : 365;
            }
            nRet = nEndDate - nStartDate;
            break;
        case 2:         
            nRet = nEndDate - nStartDate;
            if( pOptDaysIn1stYear )
                *pOptDaysIn1stYear = 360;
            break;
        case 3:         
            nRet = nEndDate - nStartDate;
            if( pOptDaysIn1stYear )
                *pOptDaysIn1stYear = 365;
            break;
        default:
            throw lang::IllegalArgumentException();
    }

    return bNeg? -nRet : nRet;
}


double GetYearDiff( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32   nDays1stYear;
    sal_Int32   nTotalDays = GetDiffDate( nNullDate, nStartDate, nEndDate, nMode, &nDays1stYear );

    return double( nTotalDays ) / double( nDays1stYear );
}


sal_Int32 GetDaysInYear( sal_Int32 nNullDate, sal_Int32 nDate, sal_Int32 nMode ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    switch( nMode )
    {
        case 0:         
        case 2:         
        case 4:         
            return 360;
        case 1:         
            {
            sal_uInt16  nD, nM, nY;
            nDate += nNullDate;
            DaysToDate( nDate, nD, nM, nY );
            return IsLeapYear( nY )? 366 : 365;
            }
        case 3:         
            return 365;
        default:
            throw lang::IllegalArgumentException();
    }
}




/**
 * Function GetYearFrac implements YEARFRAC as defined in:
 *   Open Document Format for Office Applications version 1.2 Part 2, par. 6.10.24
 *   The calculations are defined in:
 *   Open Document Format for Office Applications version 1.2 Part 2, par. 4.11.7
 */
double GetYearFrac( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nStartDate == nEndDate )
        return 0.0;     

    if( nStartDate > nEndDate )
    {
        sal_Int32   n = nEndDate;
        nEndDate = nStartDate;
        nStartDate = n;
    }

    sal_Int32 nDate1 = nStartDate + nNullDate;
    sal_Int32 nDate2 = nEndDate + nNullDate;

    sal_uInt16  nDay1, nDay2;
    sal_uInt16  nMonth1, nMonth2;
    sal_uInt16  nYear1, nYear2;

    DaysToDate( nDate1, nDay1, nMonth1, nYear1 );
    DaysToDate( nDate2, nDay2, nMonth2, nYear2 );

    
    sal_Int32 nDayDiff;
    switch( nMode )
    {
        case 0:         
            if ( nDay1 == 31 )
            {
                nDay1--;
            }
            if ( nDay1 == 30 && nDay2 == 31 )
            {
                nDay2--;
            }
            else
            {
                if ( nMonth1 == 2 && nDay1 == ( IsLeapYear( nYear1 ) ? 29 : 28 ) )
                {
                    nDay1 = 30;
                    if ( nMonth2 == 2 && nDay2 == ( IsLeapYear( nYear2 ) ? 29 : 28 ) )
                    {
                        nDay2 = 30;
                    }
                }
            }
            nDayDiff = ( nYear2 - nYear1 ) * 360 + ( nMonth2 - nMonth1 ) * 30 + ( nDay2 - nDay1 );
            break;
        case 1:         
        case 2:         
        case 3:         
            nDayDiff = nDate2 - nDate1;
            break;
        case 4:         
            if ( nDay1 == 31 )
            {
                nDay1--;
            }
            if ( nDay2 == 31 )
            {
                nDay2--;
            }
            nDayDiff = ( nYear2 - nYear1 ) * 360 + ( nMonth2 - nMonth1 ) * 30 + ( nDay2 - nDay1 );
            break;
        default:
            throw lang::IllegalArgumentException();
    }

    
    double nDaysInYear;
    switch( nMode )
    {
        case 0:         
        case 2:         
        case 4:         
            nDaysInYear = 360;
            break;
        case 1:         
            {
                const bool isYearDifferent = ( nYear1 != nYear2 );
                
                if ( isYearDifferent &&
                     ( ( nYear2 != nYear1 + 1 ) ||
                       ( nMonth1 < nMonth2 ) ||
                       ( nMonth1 == nMonth2 && nDay1 < nDay2 ) ) )
                {
                    
                    sal_Int32 nDayCount = 0;
                    for ( sal_Int16 i = nYear1; i <= nYear2; i++ )
                        nDayCount += ( IsLeapYear( i ) ? 366 : 365 );

                    nDaysInYear = ( double ) nDayCount / ( double ) ( nYear2 - nYear1 + 1 );
                }
                
                
                
                else if ( ( isYearDifferent && IsLeapYear( nYear1 ) ) ||
                          ( nMonth2 == 2 && nDay2 == 29) )
                {
                    nDaysInYear = 366;
                }
                else
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    

                    assert(nYear1 == nYear2 || nYear1 + 1 == nYear2);
                    
                    
                    

                    
                    
                    assert( ! (nMonth2 == 2 && nDay2 == 29));

                    if( IsLeapYear( nYear1 ) )
                       assert(nYear1 == nYear2);

                    
                    
                    
                    if( IsLeapYear( nYear1 ) &&
                        ( nMonth1 == 1 || ( nMonth1 == 2 && nDay1 <= 28 )) &&
                        nMonth2 > 2 )
                    {
                        nDaysInYear = 366;
                    }
                    
                    
                    
                    else if(isYearDifferent && nMonth2 > 2 && IsLeapYear( nYear2 ))
                    {
                        nDaysInYear = 366;
                    }
                    else
                    {
                        assert( !( IsLeapYear( nYear2 ) &&
                                   nYear1 == nYear2 &&
                                   (nMonth1 == 1 || (nMonth1==2 && nDay1 <= 28)) &&
                                   nMonth2 > 2));
                        nDaysInYear = 365;
                    }
                }
            }
            break;
        case 3:         
            nDaysInYear = 365;
            break;
        default:
            throw lang::IllegalArgumentException();
    }

    return double( nDayDiff ) / nDaysInYear;
}

double BinomialCoefficient( double n, double k )
{
    
    

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


double ConvertToDec( const OUString& aStr, sal_uInt16 nBase, sal_uInt16 nCharLim ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if ( nBase < 2 || nBase > 36 )
        throw lang::IllegalArgumentException();

    sal_uInt32      nStrLen = aStr.getLength();
    if( nStrLen > nCharLim )
        throw lang::IllegalArgumentException();
    else if( !nStrLen )
        return 0.0;

    double          fVal = 0.0;

    const sal_Unicode* p = aStr.getStr();

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
            
            throw lang::IllegalArgumentException();

        p++;

    }

    if( nStrLen == nCharLim && !bFirstDig && (nFirstDig >= nBase / 2) )
    {   
        fVal = ( pow( double( nBase ), double( nCharLim ) ) - fVal );   
        fVal *= -1.0;
    }

    return fVal;
}


static inline sal_Char GetMaxChar( sal_uInt16 nBase )
{
    const sal_Char* c = "--123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return c[ nBase ];
}


OUString ConvertFromDec( double fNum, double fMin, double fMax, sal_uInt16 nBase,
    sal_Int32 nPlaces, sal_Int32 nMaxPlaces, sal_Bool bUsePlaces ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    fNum = ::rtl::math::approxFloor( fNum );
    fMin = ::rtl::math::approxFloor( fMin );
    fMax = ::rtl::math::approxFloor( fMax );

    if( fNum < fMin || fNum > fMax || ( bUsePlaces && ( nPlaces <= 0 || nPlaces > nMaxPlaces ) ) )
        throw lang::IllegalArgumentException();

    sal_Int64 nNum = static_cast< sal_Int64 >( fNum );
    sal_Bool        bNeg = nNum < 0;
    if( bNeg )
        nNum = sal_Int64( pow( double( nBase ), double( nMaxPlaces ) ) ) + nNum;

    OUString          aRet( OUString::number( nNum, nBase ).toAsciiUpperCase() );


    if( bUsePlaces )
    {
        sal_Int32 nLen = aRet.getLength();
        if( !bNeg && nLen > nPlaces )
        {
            throw lang::IllegalArgumentException();
        }
        else if( ( bNeg && nLen < nMaxPlaces ) || ( !bNeg && nLen < nPlaces ) )
        {
            sal_Int32   nLeft = nPlaces - nLen;
            sal_Char*   p = new sal_Char[ nLeft + 1 ];
            memset( p, bNeg? GetMaxChar( nBase ) : '0', nLeft );
            p[ nLeft ] = 0x00;
            OUString  aTmp( p, nLeft, RTL_TEXTENCODING_MS_1252 );
            aTmp += aRet;
            aRet = aTmp;

            delete[] p;
        }
    }

    return aRet;
}


double Erf( double x )
{
    return ::rtl::math::erf(x);
}


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
    double              fMult = 0.1;    
    sal_Int32           nExp = 0;
    sal_Int32           nMaxExp = 307;
    sal_uInt16          nDigCnt = 18;   

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
                    nExp++;         
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
            case S_End:     
                break;      
        }

        p++;
    }

    p--;        
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


OUString GetString( double f, sal_Bool bLeadingSign, sal_uInt16 nMaxDig )
{
    const int       nBuff = 256;
    sal_Char        aBuff[ nBuff + 1 ];
    const char*     pFormStr = bLeadingSign? "%+.*g" : "%.*g";
    int             nLen = snprintf( aBuff, nBuff, pFormStr, int( nMaxDig ), f );
                    
                    aBuff[nBuff] = 0;
                    if ( nLen < 0 || nLen > nBuff )
                        nLen = strlen( aBuff );

    OUString          aRet( aBuff, nLen, RTL_TEXTENCODING_MS_1252 );

    return aRet;
}


double GetAmordegrc( sal_Int32 nNullDate, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer,
    double fRestVal, double fPer, double fRate, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nBase == 2 )
        throw lang::IllegalArgumentException();

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
    double      fRest = fCost - fRestVal;   

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
    double fRestVal, double fPer, double fRate, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nBase == 2 )
        throw lang::IllegalArgumentException();

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
    double fYield, sal_Int32 nFreq, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    double          fYearfrac = GetYearFrac( nNullDate, nSettle, nMat, nBase );
    double          fNumOfCoups = GetCoupnum( nNullDate, nSettle, nMat, nFreq, nBase );
    double          fDur = 0.0;
    const double    f100 = 100.0;
    fCoup *= f100 / double( nFreq );    
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
    double fRate, double fPrice, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
    sal_Int32 /*nBase*/ ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    
    
    
    throw uno::RuntimeException();  
}


double getYield_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice,
                    double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
        throw lang::IllegalArgumentException();      

    return fYieldN;
}


double getPrice_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield,
                    double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
    sal_Int32 /*nBase*/ ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    
    
    
    throw uno::RuntimeException();  
}


double GetOddlprice( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastCoup,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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








static void lcl_GetCouppcd( ScaDate& rDate, const ScaDate& rSettle, const ScaDate& rMat, sal_Int32 nFreq )
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
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nSettle >= nMat || CHK_Freq )
        throw lang::IllegalArgumentException();

    ScaDate aDate;
    lcl_GetCouppcd( aDate, ScaDate( nNullDate, nSettle, nBase ), ScaDate( nNullDate, nMat, nBase ), nFreq );
    return aDate.getDate( nNullDate );
}




static void lcl_GetCoupncd( ScaDate& rDate, const ScaDate& rSettle, const ScaDate& rMat, sal_Int32 nFreq )
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
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nSettle >= nMat || CHK_Freq )
        throw lang::IllegalArgumentException();

    ScaDate aDate;
    lcl_GetCoupncd( aDate, ScaDate( nNullDate, nSettle, nBase ), ScaDate( nNullDate, nMat, nBase ), nFreq );
    return aDate.getDate( nNullDate );
}




double GetCoupdaybs( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nSettle >= nMat || CHK_Freq )
        throw lang::IllegalArgumentException();

    ScaDate aSettle( nNullDate, nSettle, nBase );
    ScaDate aDate;
    lcl_GetCouppcd( aDate, aSettle, ScaDate( nNullDate, nMat, nBase ), nFreq );
    return ScaDate::getDiff( aDate, aSettle );
}




double GetCoupdaysnc( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nSettle >= nMat || CHK_Freq )
        throw lang::IllegalArgumentException();

    if( (nBase != 0) && (nBase != 4) )
    {
        ScaDate aSettle( nNullDate, nSettle, nBase );
        ScaDate aDate;
        lcl_GetCoupncd( aDate, aSettle, ScaDate( nNullDate, nMat, nBase ), nFreq );
        return ScaDate::getDiff( aSettle, aDate );
    }
    return GetCoupdays( nNullDate, nSettle, nMat, nFreq, nBase ) - GetCoupdaybs( nNullDate, nSettle, nMat, nFreq, nBase );
}




double GetCoupdays( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nSettle >= nMat || CHK_Freq )
        throw lang::IllegalArgumentException();

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




double GetCoupnum( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( nSettle >= nMat || CHK_Freq )
        throw lang::IllegalArgumentException();

    ScaDate aMat( nNullDate, nMat, nBase );
    ScaDate aDate;
    lcl_GetCouppcd( aDate, ScaDate( nNullDate, nSettle, nBase ), aMat, nFreq );
    sal_uInt16 nMonths = (aMat.getYear() - aDate.getYear()) * 12 + aMat.getMonth() - aDate.getMonth();
    return static_cast< double >( nMonths * nFreq / 12 );
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

    aCompList.resize( nCount );
    for( sal_uInt16 n = 0 ; n < nCount ; n++ )
        aCompList[n] = rArr.GetString( n );
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
    for( std::vector<FuncData*>::iterator it = maVector.begin(); it != maVector.end(); ++it )
        delete *it;
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
            maVector.insert( maVector.begin() + nIndex + 1, nDay );
            return;
        }
    }
    maVector.insert( maVector.begin(), nDay );
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





sal_Bool ScaDoubleListGT0::CheckInsert( double fValue ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( fValue < 0.0 )
        throw lang::IllegalArgumentException();
    return fValue > 0.0;
}





sal_Bool ScaDoubleListGE0::CheckInsert( double fValue ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( fValue < 0.0 )
        throw lang::IllegalArgumentException();
    return sal_True;
}





Complex::Complex( const OUString& rStr ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( !ParseString( rStr, *this ) )
        throw lang::IllegalArgumentException();
}


inline sal_Bool Complex::IsImagUnit( sal_Unicode c )
{
    return c == 'i' || c == 'j';
}

sal_Bool Complex::ParseString( const OUString& rStr, Complex& rCompl )
{
    rCompl.c = '\0';    

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
        case '-':   
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
        case 0:     
            rCompl.r = f;
            rCompl.i = 0.0;
            return sal_True;
    }

    return sal_False;
}


OUString Complex::GetString() const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    CHK_FINITE(r);
    CHK_FINITE(i);
    OUStringBuffer aRet;

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


double Complex::Arg( void ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( r == 0.0 && i == 0.0 )
        throw lang::IllegalArgumentException();

    double  phi = acos( r / Abs() );

    if( i < 0.0 )
        phi = -phi;

    return phi;
}


void Complex::Power( double fPower ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( r == 0.0 && i == 0.0 )
    {
        if( fPower > 0 )
        {
            r = i = 0.0;
            return;
        }
        else
            throw lang::IllegalArgumentException();
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
    static const double fMultConst = 0.7071067811865475;    
    double  p = Abs();
    double  i_ = sqrt( p - r ) * fMultConst;

    r = sqrt( p + r ) * fMultConst;
    i = ( i < 0.0 )? -i_ : i_;
}


void Complex::Sin( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( !::rtl::math::isValidArcArg( r ) )
        throw lang::IllegalArgumentException();

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


void Complex::Cos( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
	if( !::rtl::math::isValidArcArg( r ) )
        throw lang::IllegalArgumentException();

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


void Complex::Div( const Complex& z ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( z.r == 0 && z.i == 0 )
        throw lang::IllegalArgumentException();

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


void Complex::Ln( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( r == 0.0 && i == 0.0 )
        throw lang::IllegalArgumentException();

    double      fAbs = Abs();
    sal_Bool    bNegi = i < 0.0;

    i = acos( r / fAbs );

    if( bNegi )
        i = -i;

    r = log( fAbs );
}


void Complex::Log10( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    Ln();
    Mult( 0.434294481903251828 );   
}


void Complex::Log2( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    Ln();
    Mult( 1.442695040888963407 );   
}


void Complex::Tan(void) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            throw lang::IllegalArgumentException();
        double fScale =1.0 / ( cos( 2.0 * r ) + cosh( 2.0 * i ));
        r = sin( 2.0 * r ) * fScale;
        i = sinh( 2.0 * i ) * fScale;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            throw lang::IllegalArgumentException();
        r = tan( r );
    }
}


void Complex::Sec( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( i )
    {
        if( !::rtl::math::isValidArcArg( 2 * r ) )
            throw lang::IllegalArgumentException();
        double fScale = 1.0 / (cosh( 2.0 * i) + cos ( 2.0 * r));
        double  r_;
        r_ = 2.0 * cos( r ) * cosh( i ) * fScale;
        i = 2.0 * sin( r ) * sinh( i ) * fScale;
        r = r_;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            throw lang::IllegalArgumentException();
        r = 1.0 / cos( r );
    }
}


void Complex::Csc( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( i )
    {
        if( !::rtl::math::isValidArcArg( 2 * r ) )
            throw lang::IllegalArgumentException();
        double fScale = 1.0 / (cosh( 2.0 * i) - cos ( 2.0 * r));
        double  r_;
        r_ = 2.0 * sin( r ) * cosh( i ) * fScale;
        i = -2.0 * cos( r ) * sinh( i ) * fScale;
        r = r_;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            throw lang::IllegalArgumentException();
        r = 1.0 / sin( r );
    }
}


void Complex::Cot(void) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            throw lang::IllegalArgumentException();
        double fScale =1.0 / ( cosh( 2.0 * i ) - cos( 2.0 * r ) );
        r = sin( 2.0 * r ) * fScale;
        i = - ( sinh( 2.0 * i ) * fScale );
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            throw lang::IllegalArgumentException();
        r = 1.0 / tan( r );
    }
}


void Complex::Sinh( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( !::rtl::math::isValidArcArg( r ) )
        throw lang::IllegalArgumentException();

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


void Complex::Cosh( void ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( !::rtl::math::isValidArcArg( r ) )
        throw lang::IllegalArgumentException();

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


void Complex::Sech(void) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            throw lang::IllegalArgumentException();
        double fScale =1.0 / ( cosh( 2.0 * r ) + cos( 2.0 * i ));
        double r_;
        r_ = 2.0 * cosh( r ) * cos( i ) * fScale;
        i = - (2.0 * sinh( r ) * sin( i ) * fScale );
        r = r_ ;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            throw lang::IllegalArgumentException();
        r = 1.0 / cosh( r );
    }
}


void Complex::Csch(void) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if ( i )
    {
        if( !::rtl::math::isValidArcArg( 2.0 * r ) )
            throw lang::IllegalArgumentException();
        double fScale =1.0 / ( cosh( 2.0 * r ) - cos( 2.0 * i ));
        double r_;
        r_ = 2.0 * sinh( r ) * cos( i ) * fScale;
        i = - ( 2.0 * cosh( r ) * sin( i ) * fScale );
        r = r_ ;
    }
    else
    {
        if( !::rtl::math::isValidArcArg( r ) )
            throw lang::IllegalArgumentException();
        r = 1.0 / sinh( r );
    }
}


ComplexList::~ComplexList()
{
    for( sal_uInt32 i = 0; i < maVector.size(); ++i )
        delete maVector[i];
}


void ComplexList::Append( const uno::Sequence< uno::Sequence< OUString > >& r, ComplListAppendHandl eAH ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = r.getLength();
    sal_Int32   nE2;
    sal_Bool    bEmpty0 = eAH == AH_EmpyAs0;
    sal_Bool    bErrOnEmpty = eAH == AH_EmptyAsErr;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const uno::Sequence< OUString >&    rList = r[ n1 ];
        nE2 = rList.getLength();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
        {
            const OUString&   rStr = rList[ n2 ];

            if( !rStr.isEmpty() )
                Append( new Complex( rStr ) );
            else if( bEmpty0 )
                Append( new Complex( 0.0 ) );
            else if( bErrOnEmpty )
                throw lang::IllegalArgumentException();
        }
    }
}


void ComplexList::Append( const uno::Sequence< uno::Any >& aMultPars, ComplListAppendHandl eAH ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32       nEle = aMultPars.getLength();
    sal_Bool        bEmpty0 = eAH == AH_EmpyAs0;
    sal_Bool        bErrOnEmpty = eAH == AH_EmptyAsErr;

    for( sal_Int32 i = 0 ; i < nEle ; i++ )
    {
        const uno::Any&  r = aMultPars[ i ];
        switch( r.getValueTypeClass() )
        {
            case uno::TypeClass_VOID:       break;
            case uno::TypeClass_STRING:
                {
                const OUString*       pStr = ( const OUString* ) r.getValue();

                if( !pStr->isEmpty() )
                    Append( new Complex( *( OUString* ) r.getValue() ) );
                else if( bEmpty0 )
                    Append( new Complex( 0.0 ) );
                else if( bErrOnEmpty )
                    throw lang::IllegalArgumentException();
                }
                break;
            case uno::TypeClass_DOUBLE:
                Append( new Complex( *( double* ) r.getValue(), 0.0 ) );
                break;
            case uno::TypeClass_SEQUENCE:
                {
                uno::Sequence< uno::Sequence< uno::Any > >           aValArr;
                if( r >>= aValArr )
                {
                    sal_Int32           nE = aValArr.getLength();
                    const uno::Sequence< uno::Any >*   pArr = aValArr.getConstArray();
                    for( sal_Int32 n = 0 ; n < nE ; n++ )
                        Append( pArr[ n ], eAH );
                }
                else
                    throw lang::IllegalArgumentException();
                }
                break;
            default:
                throw lang::IllegalArgumentException();
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


sal_Int16 ConvertData::GetMatchingLevel( const OUString& rRef ) const
{
    OUString aStr = rRef;
    sal_Int32 nLen = rRef.getLength();
    sal_Int32 nIndex = rRef.lastIndexOf( '^' );
    if( nIndex > 0 && nIndex  == ( nLen - 2 ) )
    {
        const sal_Unicode*  p = aStr.getStr();
        aStr = OUString( p, nLen - 2 );
        aStr += OUString( p[ nLen - 1 ] );
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
                case 'y':   n = -24;    break;      
                case 'z':   n = -21;    break;      
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
                            n = -1;                 
                        else
                            n = 1;                  
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
                case 'Z':   n = 21;     break;      
                case 'Y':   n = 24;     break;      
                default:
                            n = INV_MATCHLEV;
            }


#if 0
            if (n < 0 && Class() == CDC_Information)
                n = INV_MATCHLEV;   
#endif


            if( n != INV_MATCHLEV )
            {
                sal_Unicode cLast = p[ aStr.getLength() - 1 ];
                if( cLast == '2' )
                    n *= 2;
                else if( cLast == '3' )
                    n *= 3;
            }


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
    double f, const ConvertData& r, sal_Int16 nLevFrom, sal_Int16 nLevTo ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( Class() != r.Class() )
        throw lang::IllegalArgumentException();

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

    nLevFrom = sal::static_int_cast<sal_Int16>( nLevFrom - nLevTo );    

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
    double f, const ConvertData& r, sal_Int16 nLevFrom, sal_Int16 nLevTo ) const throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if( Class() != r.Class() )
        throw lang::IllegalArgumentException();
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
#define NEWD(str,unit,cl)   maVector.push_back(new ConvertData(str,unit,cl))
#define NEWDP(str,unit,cl)  maVector.push_back(new ConvertData(str,unit,cl,sal_True))
#define NEWL(str,unit,offs,cl)  maVector.push_back(new ConvertDataLinear(str,unit,offs,cl))
#define NEWLP(str,unit,offs,cl) maVector.push_back(new ConvertDataLinear(str,unit,offs,cl,sal_True))

    

    
    NEWDP( "g",         1.0000000000000000E00,  CDC_Mass ); 
    NEWD( "sg",         6.8522050005347800E-05, CDC_Mass ); 
    NEWD( "lbm",        2.2046229146913400E-03, CDC_Mass ); 
    NEWDP( "u",         6.0221370000000000E23,  CDC_Mass ); 
    NEWD( "ozm",        3.5273971800362700E-02, CDC_Mass ); 
    NEWD( "stone",      1.574730e-04,           CDC_Mass ); 
    NEWD( "ton",        1.102311e-06,           CDC_Mass ); 
    NEWD( "grain",      1.543236E01,            CDC_Mass ); 
    NEWD( "pweight",    7.054792E-01,           CDC_Mass ); 
    NEWD( "hweight",    1.968413E-05,           CDC_Mass ); 
    NEWD( "shweight",   2.204623E-05,           CDC_Mass ); 
    NEWD( "brton",      9.842065E-07,           CDC_Mass ); 
    NEWD( "cwt",        2.2046226218487758E-05, CDC_Mass ); 
    NEWD( "shweight",   2.2046226218487758E-05, CDC_Mass ); 
    NEWD( "uk_cwt",     1.9684130552221213E-05, CDC_Mass ); 
    NEWD( "lcwt",       1.9684130552221213E-05, CDC_Mass ); 
    NEWD( "hweight",    1.9684130552221213E-05, CDC_Mass ); 
    NEWD( "uk_ton",     9.8420652761106063E-07, CDC_Mass ); 
    NEWD( "LTON",       9.8420652761106063E-07, CDC_Mass ); 

    
    NEWDP( "m",         1.0000000000000000E00,  CDC_Length ); 
    NEWD( "mi",         6.2137119223733397E-04, CDC_Length ); 
    NEWD( "Nmi",        5.3995680345572354E-04, CDC_Length ); 
    NEWD( "in",         3.9370078740157480E01,  CDC_Length ); 
    NEWD( "ft",         3.2808398950131234E00,  CDC_Length ); 
    NEWD( "yd",         1.0936132983377078E00,  CDC_Length ); 
    NEWDP( "ang",       1.0000000000000000E10,  CDC_Length ); 
    NEWD( "Pica",       2.8346456692913386E03,  CDC_Length ); 
    NEWD( "ell",        8.748906E-01,           CDC_Length ); 
    NEWDP( "parsec",    3.240779E-17,           CDC_Length ); 
    NEWDP( "pc",        3.240779E-17,           CDC_Length ); 
    NEWDP( "lightyear", 1.0570234557732930E-16, CDC_Length ); 
    NEWDP( "ly",        1.0570234557732930E-16, CDC_Length ); 
    NEWD( "survey_mi",  6.2136994949494949E-04, CDC_Length ); 

    
    NEWD( "yr",     3.1688087814028950E-08, CDC_Time ); 
    NEWD( "day",    1.1574074074074074E-05, CDC_Time ); 
    NEWD( "d",      1.1574074074074074E-05, CDC_Time ); 
    NEWD( "hr",     2.7777777777777778E-04, CDC_Time ); 
    NEWD( "mn",     1.6666666666666667E-02, CDC_Time ); 
    NEWD( "min",    1.6666666666666667E-02, CDC_Time ); 
    NEWDP( "sec",   1.0000000000000000E00,  CDC_Time ); 
    NEWDP( "s",     1.0000000000000000E00,  CDC_Time ); 

    
    NEWDP( "Pa",    1.0000000000000000E00,  CDC_Pressure ); 
    NEWDP( "atm",   9.8692329999819300E-06, CDC_Pressure ); 
    NEWDP( "at",    9.8692329999819300E-06, CDC_Pressure ); 
    NEWDP( "mmHg",  7.5006170799862700E-03, CDC_Pressure ); 
    NEWD( "Torr",   7.5006380000000000E-03, CDC_Pressure ); 
    NEWD( "psi",    1.4503770000000000E-04, CDC_Pressure ); 

    
    NEWDP( "N",     1.0000000000000000E00,  CDC_Force ); 
    NEWDP( "dyn",   1.0000000000000000E05,  CDC_Force ); 
    NEWDP( "dy",    1.0000000000000000E05,  CDC_Force ); 
    NEWD( "lbf",    2.24808923655339E-01,   CDC_Force ); 
    NEWDP( "pond",  1.019716E02,            CDC_Force ); 

    
    NEWDP( "J",     1.0000000000000000E00,  CDC_Energy ); 
    NEWDP( "e",     1.0000000000000000E07,  CDC_Energy ); 
    NEWDP( "c",     2.3900624947346700E-01, CDC_Energy ); 
    NEWDP( "cal",   2.3884619064201700E-01, CDC_Energy ); 
    NEWDP( "eV",    6.2414570000000000E18,  CDC_Energy ); 
    NEWDP( "ev",    6.2414570000000000E18,  CDC_Energy ); 
    NEWD( "HPh",    3.7250611111111111E-07, CDC_Energy ); 
    NEWD( "hh",     3.7250611111111111E-07, CDC_Energy ); 
    NEWDP( "Wh",    2.7777777777777778E-04, CDC_Energy ); 
    NEWDP( "wh",    2.7777777777777778E-04, CDC_Energy ); 
    NEWD( "flb",    2.37304222192651E01,    CDC_Energy ); 
    NEWD( "BTU",    9.4781506734901500E-04, CDC_Energy ); 
    NEWD( "btu",    9.4781506734901500E-04, CDC_Energy ); 

    
    NEWDP( "W",     1.0000000000000000E00,  CDC_Power ); 
    NEWDP( "w",     1.0000000000000000E00,  CDC_Power ); 
    NEWD( "HP",     1.341022E-03,           CDC_Power ); 
    NEWD( "h",      1.341022E-03,           CDC_Power ); 
    NEWD( "PS",     1.359622E-03,           CDC_Power ); 

    
    NEWDP( "T",     1.0000000000000000E00,  CDC_Magnetism ); 
    NEWDP( "ga",    1.0000000000000000E04,  CDC_Magnetism ); 

    
    NEWL( "C",      1.0000000000000000E00,  -2.7315000000000000E02, CDC_Temperature ); 
    NEWL( "cel",    1.0000000000000000E00,  -2.7315000000000000E02, CDC_Temperature ); 
    NEWL( "F",      1.8000000000000000E00,  -2.5537222222222222E02, CDC_Temperature ); 
    NEWL( "fah",    1.8000000000000000E00,  -2.5537222222222222E02, CDC_Temperature ); 
    NEWLP( "K",     1.0000000000000000E00,  +0.0000000000000000E00, CDC_Temperature ); 
    NEWLP( "kel",   1.0000000000000000E00,  +0.0000000000000000E00, CDC_Temperature ); 
    NEWL( "Reau",   8.0000000000000000E-01, -2.7315000000000000E02, CDC_Temperature ); 
    NEWL( "Rank",   1.8000000000000000E00,  +0.0000000000000000E00, CDC_Temperature ); 

    
    NEWD( "tsp",        2.0288413621105798E02,  CDC_Volume ); 
    NEWD( "tbs",        6.7628045403685994E01,  CDC_Volume ); 
    NEWD( "oz",         3.3814022701842997E01,  CDC_Volume ); 
    NEWD( "cup",        4.2267528377303746E00,  CDC_Volume ); 
    NEWD( "pt",         2.1133764188651873E00,  CDC_Volume ); 
    NEWD( "us_pt",      2.1133764188651873E00,  CDC_Volume ); 
    NEWD( "uk_pt",      1.7597539863927023E00,  CDC_Volume ); 
    NEWD( "qt",         1.0566882094325937E00,  CDC_Volume ); 
    NEWD( "gal",        2.6417205235814842E-01, CDC_Volume ); 
    NEWDP( "l",         1.0000000000000000E00,  CDC_Volume ); 
    NEWDP( "L",         1.0000000000000000E00,  CDC_Volume ); 
    NEWDP( "lt",        1.0000000000000000E00,  CDC_Volume ); 
    NEWDP( "m3",        1.0000000000000000E-03, CDC_Volume ); 
    NEWD( "mi3",        2.3991275857892772E-13, CDC_Volume ); 
    NEWD( "Nmi3",       1.5742621468581148E-13, CDC_Volume ); 
    NEWD( "in3",        6.1023744094732284E01,  CDC_Volume ); 
    NEWD( "ft3",        3.5314666721488590E-02, CDC_Volume ); 
    NEWD( "yd3",        1.3079506193143922E-03, CDC_Volume ); 
    NEWDP( "ang3",      1.0000000000000000E27,  CDC_Volume ); 
    NEWD( "Pica3",      2.2776990435870636E07,  CDC_Volume ); 
    NEWD( "barrel",     6.2898107704321051E-03, CDC_Volume ); 
    NEWD( "bushel",     2.837759E-02,           CDC_Volume ); 
    NEWD( "regton",     3.531467E-04,           CDC_Volume ); 
    NEWD( "GRT",        3.531467E-04,           CDC_Volume ); 
    NEWD( "Schooner",   2.3529411764705882E00,  CDC_Volume ); 
    NEWD( "Middy",      3.5087719298245614E00,  CDC_Volume ); 
    NEWD( "Glass",      5.0000000000000000E00,  CDC_Volume ); 
    NEWD( "Sixpack",    0.5,                    CDC_Volume ); 
    NEWD( "Humpen",     2.0,                    CDC_Volume ); 
    NEWD( "ly3",        1.1810108125623799E-51, CDC_Volume ); 
    NEWD( "MTON",       1.4125866688595436E00,  CDC_Volume ); 
    NEWD( "tspm",       2.0000000000000000E02,  CDC_Volume ); 
    NEWD( "uk_gal",     2.1996924829908779E-01,  CDC_Volume ); 
    NEWD( "uk_qt",      8.7987699319635115E-01,  CDC_Volume ); 

    
    NEWDP( "m2",        1.0000000000000000E00,  CDC_Area ); 
    NEWD( "mi2",        3.8610215854244585E-07, CDC_Area ); 
    NEWD( "Nmi2",       2.9155334959812286E-07, CDC_Area ); 
    NEWD( "in2",        1.5500031000062000E03,  CDC_Area ); 
    NEWD( "ft2",        1.0763910416709722E01,  CDC_Area ); 
    NEWD( "yd2",        1.1959900463010803E00,  CDC_Area ); 
    NEWDP( "ang2",      1.0000000000000000E20,  CDC_Area ); 
    NEWD( "Pica2",      8.0352160704321409E06,  CDC_Area ); 
    NEWD( "Morgen",     4.0000000000000000E-04, CDC_Area ); 
    NEWDP( "ar",        1.000000E-02,           CDC_Area ); 
    NEWD( "acre",       2.471053815E-04,        CDC_Area ); 
    NEWD( "uk_acre",    2.4710538146716534E-04, CDC_Area ); 
    NEWD( "us_acre",    2.4710439304662790E-04, CDC_Area ); 
    NEWD( "ly2",        1.1172985860549147E-32, CDC_Area ); 
    NEWD( "ha",         1.000000E-04,           CDC_Area ); 
    NEWD( "Quadratlatschen",5.6689342403628117914,CDC_Area ); 

    
    NEWDP( "m/s",   1.0000000000000000E00,  CDC_Speed ); 
    NEWDP( "m/sec", 1.0000000000000000E00,  CDC_Speed ); 
    NEWDP( "m/h",   3.6000000000000000E03,  CDC_Speed ); 
    NEWDP( "m/hr",  3.6000000000000000E03,  CDC_Speed ); 
    NEWD( "mph",    2.2369362920544023E00,  CDC_Speed ); 
    NEWD( "kn",     1.9438444924406048E00,  CDC_Speed ); 
    NEWD( "admkn",  1.9438446603753486E00,  CDC_Speed ); 
    NEWD( "wahnsinnige Geschwindigkeit", 2.0494886343432328E-14, CDC_Speed ); 
    NEWD( "ludicrous speed", 2.0494886343432328E-14, CDC_Speed ); 
    NEWD( "laecherliche Geschwindigkeit", 4.0156958471424288E-06, CDC_Speed); 
    NEWD( "ridiculous speed", 4.0156958471424288E-06, CDC_Speed); 

    
    NEWDP( "bit",   1.00E00,  CDC_Information); 
    NEWDP( "byte",  1.25E-01, CDC_Information); 
}


ConvertDataList::~ConvertDataList()
{
    for( std::vector<ConvertData*>::const_iterator it = maVector.begin(); it != maVector.end(); ++it )
        delete *it;
}


double ConvertDataList::Convert( double fVal, const OUString& rFrom, const OUString& rTo ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    ConvertData*    pFrom = NULL;
    ConvertData*    pTo = NULL;
    sal_Bool        bSearchFrom = sal_True;
    sal_Bool        bSearchTo = sal_True;
    sal_Int16       nLevelFrom = 0;
    sal_Int16       nLevelTo = 0;

    std::vector<ConvertData*>::iterator it = maVector.begin();
    while( it != maVector.end() && ( bSearchFrom || bSearchTo ) )
    {
        ConvertData*    p = *it;
        if( bSearchFrom )
        {
            sal_Int16   n = p->GetMatchingLevel( rFrom );
            if( n != INV_MATCHLEV )
            {
                if( n )
                {   
                    pFrom = p;
                    nLevelFrom = n;
                }
                else
                {   
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
                {   
                    pTo = p;
                    nLevelTo = n;
                }
                else
                {   
                    pTo = p;
                    bSearchTo = sal_False;
                    nLevelTo = n;
                }
            }
        }

        ++it;
    }

    if( pFrom && pTo )
        return pFrom->Convert( fVal, *pTo, nLevelFrom, nLevelTo );
    else
        throw lang::IllegalArgumentException();
}





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
        
        nDay = std::min( nOrigDay, static_cast< sal_uInt16 >( 30 ) );
        if( bLastDay || (nDay >= ::DaysInMonth( nMonth, nYear )) )
            nDay = 30;
    }
    else
    {
        
        sal_uInt16 nLastDay = ::DaysInMonth( nMonth, nYear );
        nDay = bLastDay ? nLastDay : std::min( nOrigDay, nLastDay );
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
    sal_uInt16 nRealDay = (bLastDayMode && bLastDay) ? nLastDay : std::min( nLastDay, nOrigDay );
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
        
        if( rTo.bUSMode )
        {
            if( ((rFrom.nMonth == 2) || (rFrom.nDay < 30)) && (aTo.nOrigDay == 31) )
                aTo.nDay = 31;
            else if( (aTo.nMonth == 2) && aTo.bLastDay )
                aTo.nDay = ::DaysInMonth( 2, aTo.nYear );
        }
        
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
        
        nDiff = aFrom.getDaysInMonth() - aFrom.nDay + 1;
        aFrom.nOrigDay = aFrom.nDay = 1;
        aFrom.bLastDay = sal_False;
        aFrom.addMonths( 1 );

        if( aFrom.nYear < aTo.nYear )
        {
            
            nDiff += aFrom.getDaysInMonthRange( aFrom.nMonth, 12 );
            aFrom.addMonths( 13 - aFrom.nMonth );

            
            nDiff += aFrom.getDaysInYearRange( aFrom.nYear, aTo.nYear - 1 );
            aFrom.addYears( aTo.nYear - aFrom.nYear );
        }

        
        nDiff += aFrom.getDaysInMonthRange( aFrom.nMonth, aTo.nMonth - 1 );
        aFrom.addMonths( aTo.nMonth - aFrom.nMonth );
    }
    
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





ScaAnyConverter::ScaAnyConverter( const uno::Reference< uno::XComponentContext >& xContext ) :
    bHasValidFormat( sal_False )
{
    xFormatter = util::NumberFormatter::create(xContext);
}

ScaAnyConverter::~ScaAnyConverter()
{
}

void ScaAnyConverter::init( const uno::Reference< beans::XPropertySet >& xPropSet ) throw( uno::RuntimeException )
{
    
    bHasValidFormat = sal_False;
    if( xFormatter.is() )
    {
        
        uno::Reference< util::XNumberFormatsSupplier > xFormatsSupp( xPropSet, uno::UNO_QUERY );
        if( xFormatsSupp.is() )
        {
            
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






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
