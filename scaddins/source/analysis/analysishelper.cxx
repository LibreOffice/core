/*************************************************************************
 *
 *  $RCSfile: analysishelper.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: gt $ $Date: 2001-08-13 10:10:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <string.h>
#include <stdio.h>
#include <vcl/resary.hxx>
#include "analysishelper.hxx"
#include "analysis.hrc"



#define unique              sal_False
#define DOUBLE              sal_True
#define STDPAR              sal_False
#define INTPAR              sal_True

#define FD( FNCNM, FNCDESC, DBL, OPT, NUMOFPAR, CAT )       { "get" #FNCNM, ANALYSIS_FUNCNAME_##FNCNM, ANALYSIS_##FNCDESC, DBL, OPT, ANALYSIS_DEFFUNCNAME_##FNCNM, NUMOFPAR, CAT }


const FuncDataBase pFuncDatas[] =
{
    FD( Workday,        Workday,        unique, INTPAR, 3, FDCat_DateTime ),
    FD( Yearfrac,       Yearfrac,       unique, INTPAR, 3, FDCat_DateTime ),
    FD( Edate,          Edate,          unique, INTPAR, 2, FDCat_DateTime ),
    FD( Weeknum,        Weeknum_add,    DOUBLE, INTPAR, 2, FDCat_DateTime ),
    FD( Eomonth,        EoMonth,        unique, INTPAR, 2, FDCat_DateTime ),
    FD( Networkdays,    Networkdays,    unique, INTPAR, 3, FDCat_DateTime ),
    FD( Iseven,         Iseven_add,     DOUBLE, STDPAR, 1, FDCat_Inf ),
    FD( Isodd,          Isodd_add,      DOUBLE, STDPAR, 1, FDCat_Inf ),
    FD( Multinomial,    Multinomial,    unique, STDPAR, 1, FDCat_Math ),
    FD( Seriessum,      Seriessum,      unique, STDPAR, 4, FDCat_Math ),
    FD( Quotient,       Quotient,       unique, STDPAR, 2, FDCat_Math ),
    FD( Mround,         Mround,         unique, STDPAR, 2, FDCat_Math ),
    FD( Sqrtpi,         SqrtPI,         unique, STDPAR, 1, FDCat_Math ),
    FD( Randbetween,    Randbetween,    unique, STDPAR, 2, FDCat_Math ),
    FD( Gcd,            Gcd_add,        DOUBLE, STDPAR, 1, FDCat_Math ),
    FD( Lcm,            Lcm_add,        DOUBLE, STDPAR, 1, FDCat_Math ),
    FD( Besseli,        BesselI,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Besselj,        BesselJ,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Besselk,        BesselK,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Bessely,        BesselY,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Bin2Oct,        Bin2Oct,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Bin2Dec,        Bin2Dec,        unique, STDPAR, 1, FDCat_Tech ),
    FD( Bin2Hex,        Bin2Hex,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Oct2Bin,        Oct2Bin,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Oct2Dec,        Oct2Dec,        unique, STDPAR, 1, FDCat_Tech ),
    FD( Oct2Hex,        Oct2Hex,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Dec2Bin,        Dec2Bin,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Dec2Hex,        Dec2Hex,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Dec2Oct,        Dec2Oct,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Hex2Bin,        Hex2Bin,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Hex2Dec,        Hex2Dec,        unique, STDPAR, 1, FDCat_Tech ),
    FD( Hex2Oct,        Hex2Oct,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Delta,          Delta,          unique, STDPAR, 2, FDCat_Tech ),
    FD( Erf,            Erf,            unique, STDPAR, 2, FDCat_Tech ),
    FD( Erfc,           Erfc,           unique, STDPAR, 1, FDCat_Tech ),
    FD( Gestep,         GeStep,         unique, STDPAR, 2, FDCat_Tech ),
    FD( Factdouble,     Factdouble,     unique, STDPAR, 1, FDCat_Tech ),
    FD( Imabs,          Imabs,          unique, STDPAR, 1, FDCat_Tech ),
    FD( Imaginary,      Imaginary,      unique, STDPAR, 1, FDCat_Tech ),
    FD( Impower,        Impower,        unique, STDPAR, 2, FDCat_Tech ),
    FD( Imargument,     Imargument,     unique, STDPAR, 1, FDCat_Tech ),
    FD( Imcos,          Imcos,          unique, STDPAR, 1, FDCat_Tech ),
    FD( Imdiv,          Imdiv,          unique, STDPAR, 2, FDCat_Tech ),
    FD( Imexp,          Imexp,          unique, STDPAR, 1, FDCat_Tech ),
    FD( Imconjugate,    Imconjugate,    unique, STDPAR, 1, FDCat_Tech ),
    FD( Imln,           Imln,           unique, STDPAR, 1, FDCat_Tech ),
    FD( Imlog10,        Imlog10,        unique, STDPAR, 1, FDCat_Tech ),
    FD( Imlog2,         Imlog2,         unique, STDPAR, 1, FDCat_Tech ),
    FD( Improduct,      Improduct,      unique, STDPAR, 2, FDCat_Tech ),
    FD( Imreal,         Imreal,         unique, STDPAR, 1, FDCat_Tech ),
    FD( Imsin,          Imsin,          unique, STDPAR, 1, FDCat_Tech ),
    FD( Imsub,          Imsub,          unique, STDPAR, 2, FDCat_Tech ),
    FD( Imsqrt,         Imsqrt,         unique, STDPAR, 1, FDCat_Tech ),
    FD( Imsum,          Imsum,          unique, STDPAR, 1, FDCat_Tech ),
    FD( Complex,        Complex,        unique, STDPAR, 3, FDCat_Tech ),
    FD( Convert,        Convert_add,    DOUBLE, STDPAR, 3, FDCat_Tech ),
    FD( Amordegrc,      Amordegrc,      unique, INTPAR, 7, FDCat_Finance ),
    FD( Amorlinc,       Amorlinc,       unique, INTPAR, 7, FDCat_Finance ),
    FD( Accrint,        Accrint,        unique, INTPAR, 7, FDCat_Finance ),
    FD( Accrintm,       Accrintm,       unique, INTPAR, 5, FDCat_Finance ),
    FD( Received,       Received,       unique, INTPAR, 5, FDCat_Finance ),
    FD( Disc,           Disc,           unique, INTPAR, 5, FDCat_Finance ),
    FD( Duration,       Duration,       DOUBLE, INTPAR, 6, FDCat_Finance ),
    FD( Effect,         Effect_add,     DOUBLE, STDPAR, 2, FDCat_Finance ),
    FD( Cumprinc,       Cumprinc_add,   DOUBLE, STDPAR, 6, FDCat_Finance ),
    FD( Cumipmt,        Cumipmt_add,    DOUBLE, STDPAR, 6, FDCat_Finance ),
    FD( Price,          Price,          unique, INTPAR, 7, FDCat_Finance ),
    FD( Pricedisc,      Pricedisc,      unique, INTPAR, 5, FDCat_Finance ),
    FD( Pricemat,       Pricemat,       unique, INTPAR, 6, FDCat_Finance ),
    FD( Mduration,      Mduration,      unique, INTPAR, 6, FDCat_Finance ),
    FD( Nominal,        Nomial_add,     DOUBLE, STDPAR, 2, FDCat_Finance ),
    FD( Dollarfr,       Dollarfr,       unique, STDPAR, 2, FDCat_Finance ),
    FD( Dollarde,       Dollarde,       unique, STDPAR, 2, FDCat_Finance ),
    FD( Yield,          Yield,          unique, INTPAR, 7, FDCat_Finance ),
    FD( Yielddisc,      Yielddisc,      unique, INTPAR, 5, FDCat_Finance ),
    FD( Yieldmat,       Yieldmat,       unique, INTPAR, 6, FDCat_Finance ),
    FD( Tbilleq,        Tbilleq,        unique, INTPAR, 3, FDCat_Finance ),
    FD( Tbillprice,     Tbillprice,     unique, INTPAR, 3, FDCat_Finance ),
    FD( Tbillyield,     Tbillyield,     unique, INTPAR, 3, FDCat_Finance ),
    FD( Oddfprice,      Oddfprice,      unique, INTPAR, 9, FDCat_Finance ),
    FD( Oddfyield,      Oddfyield,      unique, INTPAR, 9, FDCat_Finance ),
    FD( Oddlprice,      Oddlprice,      unique, INTPAR, 8, FDCat_Finance ),
    FD( Oddlyield,      Oddlyield,      unique, INTPAR, 8, FDCat_Finance ),
    FD( Xirr,           Xirr,           unique, STDPAR, 3, FDCat_Finance ),
    FD( Xnpv,           Xnpv,           unique, STDPAR, 3, FDCat_Finance ),
    FD( Intrate,        Intrate,        unique, INTPAR, 5, FDCat_Finance ),
    FD( Coupncd,        Coupncd,        unique, INTPAR, 4, FDCat_Finance ),
    FD( Coupdays,       Coupdays,       unique, INTPAR, 4, FDCat_Finance ),
    FD( Coupdaysnc,     Coupdaysnc,     unique, INTPAR, 4, FDCat_Finance ),
    FD( Coupdaybs,      Coupdaybs,      unique, INTPAR, 4, FDCat_Finance ),
    FD( Couppcd,        Couppcd,        unique, INTPAR, 4, FDCat_Finance ),
    FD( Coupnum,        Coupnum,        unique, INTPAR, 4, FDCat_Finance ),
    FD( Fvschedule,     Fvschedule,     unique, STDPAR, 2, FDCat_Finance )
};


static const double nKorrVal[] = {
    0, 9e-1, 9e-2, 9e-3, 9e-4, 9e-5, 9e-6, 9e-7, 9e-8,
    9e-9, 9e-10, 9e-11, 9e-12, 9e-13, 9e-14, 9e-15
};

static const double f_Ret = 0.0;


double Round( double fVal, short nDec )
{
    if ( fVal == 0.0  )
        return fVal;

    // sign adjustment
    sal_Bool bSign = fVal < 0.0;
    if ( bSign )
        fVal = -fVal;

    double fFac;
    if ( nDec != 0 )
    {
        // max 20 decimals, we don't have unlimited precision
        // #38810# and no overflow on fVal*=fFac
        if ( nDec < -20 || 20 < nDec || fVal > (1.7976931348623158e+308 / 1e20) )
            return bSign ? -fVal : fVal;

        fFac = pow( 10.0, nDec );
        fVal *= fFac;
    }
    //else  //! uninitialized fFac, not needed

            int nExp;       // exponent for correction
            if ( fVal > 0.0 )
                nExp = (int) floor( log10( fVal ) );
            else
                nExp = 0;
            int nIndex = 15 - nExp;
            if ( nIndex > 15 )
                nIndex = 15;
            else if ( nIndex <= 1 )
                nIndex = 0;
            fVal = floor( fVal + 0.5 + nKorrVal[nIndex] );

    if ( nDec != 0 )
        fVal /= fFac;

    return bSign ? -fVal : fVal;
}


/*double _Test( sal_Int32 nMode, double f1, double f2, double f3 )
{
    double      f = -1.0;
    switch( nMode )
    {
        case 0:     f = GammaN( f1, sal_uInt32( f2 ) );     break;
        case 1:     f = Gamma( f1 );                        break;
    }

    return f;
}*/


static sal_uInt16 aDaysInMonth[ 13 ] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{

    if( nMonth != 2 )
        return aDaysInMonth[ nMonth ];
    else
    {
        if( IsLeapYear( nYear ) )
            return aDaysInMonth[ nMonth ] + 1;
        else
            return aDaysInMonth[ nMonth ];
    }
}


sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear, sal_Bool bLeapYear )
{

    if( nMonth != 2 )
        return aDaysInMonth[ nMonth ];
    else
    {
        if( bLeapYear )
            return aDaysInMonth[ nMonth ] + 1;
        else
            return aDaysInMonth[ nMonth ];
    }
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
{
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
            //aDate2 += 1;      -> 1.xx.yyyy
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
            sal_Int32       nDays, nMonths/*, nYears*/;

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


void AddDate( sal_uInt16& rD, sal_uInt16& rM, sal_uInt16& rY, sal_Int32 nDD, sal_Int32 nDM, sal_Int32 nDY )
{
    sal_Int32   nD = rD;
    sal_Int32   nM = rM;
    sal_Int32   nY = rY;

    sal_Bool    bMod = sal_False;

    if( nDM )
    {
        bMod = sal_True;

        nY += nDM / 12;
        nM += nDM % 12;
        if( nM <= 0 )
        {
            nY--;
            nM += 12;
        }
        else if( nM > 12 )
        {
            nY++;
            nM -= 12;
        }
    }

    if( bMod )
    {
        rD = sal_uInt16( nD );
        rM = sal_uInt16( nM );
        rY = sal_uInt16( nY );

        AlignDate( rD, sal_uInt16( nM ), sal_uInt16( nY ) );
    }
}


void AddDate( sal_Int32 nND, sal_Int32& rD, sal_Int32 nDD, sal_Int32 nDM, sal_Int32 nDY )
{
    sal_uInt16  nD, nM, nY;
    DaysToDate( rD + nND, nD, nM, nY );

    AddDate( nD, nM, nY, nDD, nDM, nDY );

    rD = DateToDays( nD, nM, nY ) - nND;
}


// if nRefDate is last day in month and day of nDate is >= day of nRefDate,
// adjust nDate to last day in month and return
sal_Int32 AdjustLastDayInMonth( sal_Int32 nNullDate, sal_Int32 nRefDate, sal_Int32 nDate )
{
    sal_uInt16 nRefDay, nRefMonth, nRefYear;
    DaysToDate( nNullDate + nRefDate, nRefDay, nRefMonth, nRefYear );
    if( nRefDay == DaysInMonth( nRefMonth, nRefYear ) )
    {
        sal_uInt16 nDay, nMonth, nYear;
        DaysToDate( nNullDate + nDate, nDay, nMonth, nYear );
        if( nDay >= nRefDay )
        {
            nDate += DaysInMonth( nMonth, nYear );
            nDate -= nDay;
        }
    }
    return nDate;
}


double Fak( sal_Int32 n )
{
    if( n > 0 )
    {
        double  fRet = n;
        double  f = n - 1;

        while( f >= 2.0 )
        {
            fRet *= f;
            f--;
        }

        return fRet;
    }
    else if( !n )
        return 1.0;
    else
        return 0.0;
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


double GammaHelp( double& x, sal_Bool& bReflect )
{
    double c[6] = {76.18009173, -86.50532033, 24.01409822,
                   -1.231739516, 0.120858003E-2, -0.536382E-5};
    if (x >= 1.0)
    {
        bReflect = sal_False;
        x -= 1.0;
    }
    else
    {
        bReflect = sal_True;
        x = 1.0 - x;
    }
    double s, anum;
    s = 1.0;
    anum = x;
    for (sal_uInt16 i = 0; i < 6; i++)
    {
        anum += 1.0;
        s += c[i]/anum;
    }
    s *= 2.506628275;                   // sqrt(2*PI)
    return s;
}


double Gamma( double x )
{
    sal_Bool bReflect;
    double G = GammaHelp(x, bReflect);
    G = pow(x+5.5,x+0.5)*G/exp(x+5.5);
    if (bReflect)
        G = PI*x/(G*sin(PI*x));
    return G;
}


double GammaN( double x, sal_uInt32 nIter )
{
    double  n = nIter;
    double  f = pow( n, x ) / x;
    double  c = 1.0;

    while( nIter )
    {
        x++;

        f /= x;
        f *= c;

        nIter--;
        c++;
    }

    return f;
}


double Bessel( double fNum, sal_Int32 nOrder, sal_Bool bModfied ) THROWDEF_RTE_IAE
{
    if( nOrder < 0 )
        THROW_IAE;

    double      fZ, fZm, fN1, fN2, fn1, fn2, fAct, fOld;
    sal_Int16   nIterMax = 100;

    fZ = fNum * 0.5;                            // x/2
    fZm = fZ * fZ;                              // (x/2)^2
    fZ = pow( fZ, double( nOrder ) );           // (x/2)^n

    fN1 = Fak( nOrder );                        // n!
    fn1 = 0.0;
    fN2 = 1.0;
    fn2 = double( nOrder );

    fAct = fZ / fN1;
    fOld = fAct * 0.9;

    if( bModfied )
    {
        while( fAct != fOld && nIterMax )
        {
            fZ *= fZm;

            fn1++;
            fN1 *= fn1;

            fn2++;
            fN2 *= fn2;

            fOld = fAct;

            fAct += fZ / fN1 / fN2;

            nIterMax--;
        }
    }
    else
    {
        sal_Bool    bAdd = sal_False;   // start with second term, so subtraction is first in loop

        while( fAct != fOld && nIterMax )
        {
            fZ *= fZm;

            fn1++;
            fN1 *= fn1;

            fn2++;
            fN2 *= fn2;

            fOld = fAct;

            if( bAdd )
                fAct += fZ / fN1 / fN2;
            else
                fAct -= fZ / fN1 / fN2;

            nIterMax--;
            bAdd = !bAdd;
        }
    }

    return fAct;
}


double Besselk0( double fNum )
{
    double  fRet;

    if( fNum <= 2.0 )
    {
        double  fNum2 = fNum * 0.5;
        double  y = fNum2 * fNum2;

        fRet = -log( fNum2 ) * Bessel( fNum, 0, sal_True ) +
                ( -0.57721566 + y * ( 0.42278420 + y * ( 0.23069756 + y * ( 0.3488590e-1 +
                    y * ( 0.262698e-2 + y * ( 0.10750e-3 + y * 0.74e-5 ) ) ) ) ) );
    }
    else
    {
        double  y = 2.0 / fNum;

        fRet = exp( -fNum ) / sqrt( fNum ) * ( 1.25331414 + y * ( -0.7832358e-1 +
                y * ( 0.2189568e-1 + y * ( -0.1062446e-1 + y * ( 0.587872e-2 +
                y * ( -0.251540e-2 + y * 0.53208e-3 ) ) ) ) ) );
    }

    return fRet;
}


double Besselk1( double fNum )
{
    double  fRet;

    if( fNum <= 2.0 )
    {
        double  fNum2 = fNum * 0.5;
        double  y = fNum2 * fNum2;

        fRet = log( fNum2 ) * Bessel( fNum, 1, sal_True ) +
                ( 1.0 + y * ( 0.15443144 + y * ( -0.67278579 + y * ( -0.18156897 + y * ( -0.1919402e-1 +
                    y * ( -0.110404e-2 + y * ( -0.4686e-4 ) ) ) ) ) ) )
                / fNum;
    }
    else
    {
        double  y = 2.0 / fNum;

        fRet = exp( -fNum ) / sqrt( fNum ) * ( 1.25331414 + y * ( 0.23498619 +
                y * ( -0.3655620e-1 + y * ( 0.1504268e-1 + y * ( -0.780353e-2 +
                y * ( 0.325614e-2 + y * ( -0.68245e-3 ) ) ) ) ) ) );
    }

    return fRet;
}


double Besselk( double fNum, sal_Int32 nOrder )
{
    switch( nOrder )
    {
        case 0:     return Besselk0( fNum );        break;
        case 1:     return Besselk1( fNum );        break;
        default:
        {
            double      fBkp;

            double      fTox = 2.0 / fNum;
            double      fBkm = Besselk0( fNum );
            double      fBk = Besselk1( fNum );

            for( sal_Int32 n = 1 ; n < nOrder ; n++ )
            {
                fBkp = fBkm + double( n ) * fTox * fBk;
                fBkm = fBk;
                fBk = fBkp;
            }

            return fBk;
        }
    }
}


double Bessely0( double fNum )
{
    double  fRet;

    if( fNum < 8.0 )
    {
        double  y = fNum * fNum;

        double  f1 = -2957821389.0 + y * ( 7062834065.0 + y * ( -512359803.6 +
                    y * ( 10879881.29 + y * ( -86327.92757 + y * 228.4622733 ) ) ) );

        double  f2 = 40076544269.0 + y * ( 745249964.8 + y * ( 7189466.438 +
                    y * ( 47447.26470 + y * ( 226.1030244 + y ) ) ) );

        fRet = f1 / f2 + 0.636619772 * Bessel( fNum, 0, sal_False ) * log( fNum );
    }
    else
    {
        double  z = 8.0 / fNum;
        double  y = z * z;
        double  xx = fNum - 0.785398164;

        double  f1 = 1.0 + y * ( -0.1098628627e-2 + y * ( 0.2734510407e-4 +
                        y * ( -0.2073370639e-5 + y * 0.2093887211e-6 ) ) );

        double  f2 = -0.1562499995e-1 + y * ( 0.1430488765e-3 +
                        y * ( -0.6911147651e-5 + y * ( 0.7621095161e-6 +
                        y * ( -0.934945152e-7 ) ) ) );

        fRet = sqrt( 0.636619772 / fNum ) * ( sin( xx ) * f1 + z * cos( xx ) * f2 );
    }

    return fRet;
}


double Bessely1( double fNum )
{
    double  fRet;

    if( fNum < 8.0 )
    {
        double  y = fNum * fNum;

        double  f1 = fNum * ( -0.4900604943e13 + y * ( 0.1275274390e13 +
                        y * ( -0.5153438139e11 + y * ( 0.7349264551e9 +
                        y * ( -0.4237922726e7 + y * 0.8511937935e4 ) ) ) ) );

        double  f2 = 0.2499580570e14 + y * ( 0.4244419664e12 +
                        y * ( 0.3733650367e10 + y * ( 0.2245904002e8 +
                        y * ( 0.1020426050e6 + y * ( 0.3549632885e3 + y ) ) ) ) );

        fRet = f1 / f2 + 0.636619772 * ( Bessel( fNum, 1, sal_False ) * log( fNum ) - 1.0 / fNum );
    }
    else
    {
        double  z = 8.0 / fNum;
        double  y = z * z;
        double  xx = fNum - 2.356194491;

        double  f1 = 1.0 + y * ( 0.183105e-2 + y * ( -0.3516396496e-4 +
                        y * ( 0.2457520174e-5 + y * ( -0.240337019e6 ) ) ) );

        double  f2 = 0.04687499995 + y * ( -0.2002690873e-3 +
                        y * ( 0.8449199096e-5 + y * ( -0.88228987e-6 +
                        y * 0.105787412e-6 ) ) );

        fRet = sqrt( 0.636619772 / fNum ) * ( sin( xx ) * f1 + z * cos( xx ) * f2 );
    }

    return fRet;
}


double Bessely( double fNum, sal_Int32 nOrder )
{
    switch( nOrder )
    {
        case 0:     return Bessely0( fNum );        break;
        case 1:     return Bessely1( fNum );        break;
        default:
        {
            double      fByp;

            double      fTox = 2.0 / fNum;
            double      fBym = Bessely0( fNum );
            double      fBy = Bessely1( fNum );

            for( sal_Int32 n = 1 ; n < nOrder ; n++ )
            {
                fByp = double( n ) * fTox * fBy - fBym;
                fBym = fBy;
                fBy = fByp;
            }

            return fBy;
        }
    }
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

    sal_uInt16          nFirstDig;
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

    if( nStrLen == nCharLim && !bFirstDig && nFirstDig == nBase - 1 )
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


STRING ConvertFromDec( sal_Int64 nNum, sal_Int64 nMin, sal_Int64 nMax, sal_uInt16 nBase,
    sal_Int32 nPlaces, sal_Int32 nMaxPlaces ) THROWDEF_RTE_IAE
{
    sal_Bool        bUsePlaces = nPlaces != 0x80000000;
    if( nNum < nMin || nNum > nMax || ( bUsePlaces && ( nPlaces <= 0 || nPlaces > nMaxPlaces ) ) )
        THROW_IAE;

    sal_Bool        bNeg = nNum < 0;
    if( bNeg )
        nNum = sal_Int64( pow( double( nBase ), double( nMaxPlaces ) ) ) + nNum;

    STRING          aRet( STRING::valueOf( nNum, nBase ).toAsciiUpperCase() );

    sal_Int32       nLen = aRet.getLength();

    if( bUsePlaces )
    {
        if( !bNeg && nLen > nPlaces )
            THROW_IAE;
        else if( ( bNeg && nLen < nMaxPlaces ) || ( !bNeg && nLen < nPlaces ) )
        {
            sal_uInt32  nLeft = nPlaces - nLen;
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


double Erf( double x )
{
    if( x == 0.0 )
        return 0.0;

    double      f, fZm, fN2, fn1, fOld, fT;
    sal_Bool    bAdd = sal_True;
    sal_Int32   nMaxIter = 1000;

    fZm = x;
    fZm *= x;   // x^2

    fn1 = 2.0;
    fN2 = 3.0;

    fT = x * fZm;

    f = x - fT / fN2;

    fOld = f * 0.9;

    while( f != fOld && nMaxIter )
    {
        fOld = f;

        fN2 += 2.0;

        fT /= fn1;
        fT *= fZm;

        fn1++;

        if( bAdd )
            f += fT / fN2;
        else
            f -= fT / fN2;

        bAdd = !bAdd;

        nMaxIter--;
    }

    return f * 1.128379167095512573896; // * 2/sqrt(PI)
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
                    rRet = bNegNum? -1.0 : 1.0;
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

    fInt *= pow( 10.0, nExp );

    if( bNegNum )
        fInt = -fInt;

    rRet = fInt;

    return sal_True;
}


STRING GetString( double f, sal_Bool bLeadingSign, sal_uInt16 nMaxDig )
{
    const int       nBuff = 256;
    sal_Char*       pBuff = new sal_Char[ nBuff + 1 ];
    const char*     pFormStr = bLeadingSign? "%+.*g" : "%.*g";
    int             nLen = sprintf( pBuff, pFormStr, int( nMaxDig ), f );

    STRING          aRet( pBuff, nLen, RTL_TEXTENCODING_MS_1252 );

    delete pBuff;

    return aRet;
}


inline double Exp10( sal_Int16 n )
{
    return pow( 10.0, double( n ) );
}


sal_Int32 GetOpt( const ANY& rAny, sal_Int32 nDefault ) THROWDEF_RTE_IAE
{
    switch( rAny.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:
            return nDefault;
            break;
        case uno::TypeClass_DOUBLE:
            double  fTmp = *( double* ) rAny.getValue();
            if( fTmp >= -2147483648.0 && fTmp < 2147483648 )
                return sal_Int32( fTmp );
            break;
    }

    THROW_IAE;
}


double GetOpt( const ANY& rAny, double fDefault ) THROWDEF_RTE_IAE
{
    switch( rAny.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:       return fDefault;                        break;
        case uno::TypeClass_DOUBLE:     return *( double* ) rAny.getValue();    break;
    }

    THROW_IAE;
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
    double      fNRate = Round( GetYearFrac( nNullDate, nDate, nFirstPer, nBase ) * fRate * fCost, 0 );
    fCost -= fNRate;
    double      fRest = fCost - fRestVal;   // Anschaffungskosten - Restwert - Summe aller Abschreibungen

    for( sal_uInt32 n = 0 ; n < nPer ; n++ )
    {
        fNRate = Round( fRate * fCost, 0 );
        fRest -= fNRate;

        if( fRest < 0.0 )
        {
            switch( nPer - n )
            {
                case 0:
                case 1:
                    return Round( fCost * 0.5, 0 );
                    break;
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

    double          t;

    for( t = 1.0 ; t < fNumOfCoups ; t++ )
        fDur += t * ( fCoup ) / pow( fYield, t );

    fDur += fNumOfCoups * ( fCoup + f100 ) / pow( fYield, fNumOfCoups );

    double          p = 0.0;
    for( t = 1.0 ; t < fNumOfCoups ; t++ )
        p += fCoup / pow( fYield, t );

    p += ( fCoup + f100 ) / pow( fYield, fNumOfCoups );

    fDur /= p;
    fDur /= double( nFreq );

    return fDur;
}


double GetYieldmat( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
    double fRate, double fPrice, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    return f_Ret;
}


double GetOddfyield( sal_Int32 nNullDate,  sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
    sal_Int32 nFirstCoup, double fRate, double fPrice, double fRedemp, sal_Int32 nFreq,
    sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    return f_Ret;
}


double GetOddlprice( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    THROW_IAE;
    return 0.0;
}


double GetOddlyield( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    THROW_IAE;
    return 0.0;
/*  double      f100ZH = 100.0 * fRate / double( nFreq );
    double      s;
    sal_uInt32  i;
    sal_uInt32  nc = 42;

#define _DC( i )        1
#define _NL( i )        1
#define _A( i )         1
#define _DSC( i )       1

    double      fZ1 = fRedemp;
    s = 0.0;
    for( i = 1 ; i <= nc ; i++ )
        s += _DC( i ) / _NL( i );
    s*= f100ZH;
    fZ1 += s;

    double      fN1 = fPrice;
    s = 0.0;
    for( i = 1 ; i <= nc ; i++ )
        s += _A( i ) / _NL( i );
    s*= f100ZH;
    fN1 += s;

    s = 0.0;
    for( i = 1 ; i <= nc ; i++ )
        s += _DSC( i ) / _NL( i );
    double      fN2 = s;

    return ( fZ1 / fN1 - 1.0 ) * double( nFreq ) / fN2;

#undef _DC
#undef _NL
#undef _A
#undef _DSC
*/
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


/*double TBillYield( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice ) THROWDEF_RTE_IAE
{
    sal_Int32   nDiff = GetDiffDate360( xOpt, nSettle, nMat, sal_True );

    if( fPrice <= 0.0 || nSettle >= nMat || nDiff > 360 )
        THROW_IAE;

    double      fRet = 100.0;
    fRet /= fPrice;
    fRet--;
    fRet *= double( nDiff );
    fRet /= 360.0;

    return fRet;
}*/


double GetCoupnum( sal_Int32 nND, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

//    return ceil( GetYearFrac( nND, nSettle, nMat, nBase ) * nFreq );

    sal_Int32 nRet = 0;
    sal_uInt16 nSetDay, nSetMonth, nSetYear;
    nSettle += nND;
    DaysToDate( nSettle, nSetDay, nSetMonth, nSetYear );
    sal_Bool bSetLastDay = (DaysInMonth( nSetMonth, nSetYear ) == nSetDay);

    sal_uInt16 nMatDay, nMatMonth, nMatYear;
    nMat += nND;
    DaysToDate( nMat, nMatDay, nMatMonth, nMatYear );
    sal_Bool bMatLastDay = (DaysInMonth( nMatMonth, nMatYear ) == nMatDay);

    sal_uInt16 nFreqMonths = 12 / nFreq;
    nRet = (nMatYear - nSetYear) * nFreq;
    nRet += (nMatMonth - nSetMonth + nFreqMonths + 11) / nFreqMonths - nFreq;
    nSetMonth %= nFreqMonths;
    nMatMonth %= nFreqMonths;
    if( (nSetMonth == nMatMonth) && !bSetLastDay && (bMatLastDay || (nMatDay > nSetDay)) )
        nRet += 1;

    return (double) nRet;
}


double GetCouppcd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    sal_Int32   nRet = nMat;
    sal_Int32   nPeriodsDelta = sal_Int32( GetCoupnum( nNullDate, nSettle, nRet, nFreq, nBase ) );

    AddDate( nNullDate, nRet, 0, -( nPeriodsDelta * 12 / nFreq ), 0 );
    nRet = AdjustLastDayInMonth( nNullDate, nMat, nRet );

    return nRet;
}


double GetCoupdays( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    double nRet;
    if( nBase == 1 )
    {
        sal_Int32   nStart = sal_Int32( GetCouppcd( nNullDate, nSettle, nMat, nFreq, nBase ) );
        sal_Int32   nEnd = nStart;
        AddDate( nNullDate, nEnd, 0, 12 / nFreq, 0 );
        nStart = AdjustLastDayInMonth( nNullDate, nEnd, nStart );
        nRet = nEnd - nStart;
    }
    else
    {
        // get constant default day count for bases 0, 2, 3, 4
        nRet = GetDaysInYear( 0, 0, nBase );
        nRet /= nFreq;
    }
    return nRet;
}


double GetCoupdaysnc( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    sal_Int32   nDate = sal_Int32( GetCouppcd( nNullDate, nSettle, nMat, nFreq, nBase ) );
    AddDate( nNullDate, nDate, 0, 12 / nFreq, 0 );

    return nDate - nSettle;
}


double GetCoupncd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, sal_Int32 nBase )
    THROWDEF_RTE_IAE
{
    if( nSettle >= nMat || CHK_Freq )
        THROW_IAE;

    sal_Int32   nPeriodsDelta = sal_Int32( GetCoupnum( nNullDate, nSettle, nMat, nFreq, nBase ) );

    AddDate( nNullDate, nMat, 0, -( ( nPeriodsDelta - 1 ) * 12 / nFreq ), 0 );

    return nMat;
}




const sal_uInt32 MyList::nStartSize = 16;
const sal_uInt32 MyList::nIncrSize = 16;


void MyList::_Grow( void )
{
    nSize += nIncrSize;

    void**          pNewData = new void*[ nSize ];
    memcpy( pNewData, pData, nNew * sizeof( void* ) );

    delete pData;
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
    delete pData;
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


sal_Bool StringList::Contains( const STRING& r ) const
{
    sal_uInt32      n = 0;
    const STRING*   p = Get( n );

    while( p )
    {
        if( *p == r )
            return sal_True;

        n++;
        p = Get( n );
    }

    return sal_False;
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
//  ResStringArray      aDefFuncNameArray( AnalysisResId( nCompID, rResMgr ) );
    const ResStringArray&   rArr = aArrLoader.GetStringArray();

    sal_uInt16              nCount = rArr.Count();
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
    const sal_uInt32    nNum = sizeof( pFuncDatas ) / sizeof( FuncDataBase );

    for( sal_uInt16 n = 0 ; n < nNum ; n++ )
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


AnalysisResId::AnalysisResId( sal_uInt16 nId, ResMgr& rResMgr ) : ResId( nId, &rResMgr )
{
}




SortedIndividualInt32List::SortedIndividualInt32List()
{
}


SortedIndividualInt32List::~SortedIndividualInt32List()
{
}


void SortedIndividualInt32List::Insert( sal_Int32 nVal )
{
    sal_Int32   n = Count();

    while( n )
    {
        n--;
        sal_Int32   nRef = Get( n );
        if( nVal == nRef )
            return;
        else if( nVal > nRef )
        {
            MyList::Insert( ( void* ) nVal, n + 1 );
            return;
        }
    }

    // smalest (or first) element
    MyList::Insert( ( void* ) nVal, sal_uInt32( 0 ) );
}


sal_Int32 SortedIndividualInt32List::CountCondition( sal_Int32 nMinVal, sal_Int32 nMaxVal ) const
{
    sal_uInt32  nE = Count();

    if( !nE || nMaxVal < Get( 0 ) || nMinVal > Get( nE - 1 ) )
        // don't care if list is empty
        // ~ if nMaxVal is smaler than smalest value in list
        // ~ if nMinVal is bigger than biggest value in list
        return 0;

    sal_uInt32  n;
    sal_Int32   nCnt = 0;

    for( n = 0 ; n < nE && Get( n ) < nMinVal ; n++);   // skip values less than nMinVal

    for( ; n < nE ; n++ )
    {
        if( Get( n ) <= nMaxVal )
            nCnt++;
        else
            break;
    }

    return nCnt;
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


void SortedIndividualInt32List::InsertHolidayList( const SEQSEQ( sal_Int32 )& aHD, sal_Int32 nND, sal_Bool bInsOnWE )
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = aHD.getLength();
    sal_Int32   nE2;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( sal_Int32 )& rList = aHD[ n1 ];
        nE2 = rList.getLength();
        const sal_Int32*        pList = rList.getConstArray();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
        {
            sal_Int32   n = pList[ n2 ];

            if( n )
            {
                n += nND;
                if( bInsOnWE || GetDayOfWeek( n ) < 5 )
                    Insert( n );
            }
        }
    }
}


void SortedIndividualInt32List::InsertHolidayList( const SEQ( double )& aHD, sal_Int32 nND, sal_Bool bInsOnWE )
    THROWDEF_RTE_IAE
{
    sal_Int32   n1;
    sal_Int32   nE = aHD.getLength();

    nE = aHD.getLength();
    const double*   pList = aHD.getConstArray();

    for( n1 = 0 ; n1 < nE ; n1++ )
    {
        double      f = pList[ n1 ];
        if( f < -2147483648.0 || f >= 2147483648.0 )
            THROW_IAE;

        sal_Int32   n = sal_Int32( f );

        if( n )
        {
            n += nND;
            if( bInsOnWE || GetDayOfWeek( n ) < 5 )
                Insert( n );
        }
    }
}


void SortedIndividualInt32List::InsertHolidayList(
    const ANY& aHDay, sal_Int32 nNullDate, sal_Bool bInsOnWE ) THROWDEF_RTE_IAE
{
    switch( aHDay.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:       break;
        case uno::TypeClass_STRING:
            if( ( ( const STRING* ) aHDay.getValue() )->getLength() )
                THROW_IAE;
            break;
        case uno::TypeClass_DOUBLE:
            {
            double          f = *( double* ) aHDay.getValue();
            if( f < -2147483648.0 || f >= 2147483648.0 )
                THROW_IAE;

            sal_Int32       n = sal_Int32( f );

            if( n )
            {
                n += nNullDate;
                if( bInsOnWE || GetDayOfWeek( n ) < 5 )
                    Insert( n );
            }
            }
            break;
        case uno::TypeClass_SEQUENCE:
            {
            SEQSEQ( ANY )   aValArr;
            if( aHDay >>= aValArr )
            {
                sal_Int32           nE = aValArr.getLength();
                const SEQ( ANY )*   pArr = aValArr.getConstArray();
                for( sal_Int32 n = 0 ; n < nE ; n++ )
                {
                    sal_Int32       nF = pArr[ n ].getLength();
                    const ANY*      pAny = pArr[ n ].getConstArray();

                    for( sal_Int32 m = 0 ; m < nF ; m++ )
                        InsertHolidayList( pAny[ m ], nNullDate, bInsOnWE );
                }
            }
            else
                THROW_IAE;
            }
            break;
        default:
            THROW_IAE;
    }

}




inline void DoubleList::AppendVoid( sal_Bool bErrOnEmpty ) THROWDEF_RTE_IAE
{
    if( bErrOnEmpty )
        THROW_IAE;
}


inline void DoubleList::AppendDouble( double f ) THROWDEF_RTE_IAE
{
    if( IsFaulty( f ) )
        THROW_IAE;
    if( IsProper( f ) )
        _Append( f );
}


void DoubleList::AppendString( const ANY& r, sal_Bool bEmptyAs0 ) THROWDEF_RTE_IAE
{
    if( bEmptyAs0 || ( ( const STRING* ) r.getValue() )->getLength() )
        AppendDouble( 0.0 );
    else
        THROW_IAE;
}


void DoubleList::AppendDouble( const ANY& r ) THROWDEF_RTE_IAE
{
    double  f = *( double* ) r.getValue();
    if( IsFaulty( f ) )
        THROW_IAE;
    if( IsProper( f ) )
        _Append( f );
}


inline void DoubleList::AppendAnyArray2( const ANY& r ) THROWDEF_RTE_IAE
{
    Append( *( const SEQSEQ( ANY )* ) r.getValue() );
}


DoubleList::~DoubleList()
{
    for( double* p = ( double* ) MyList::First(); p ; p = ( double* ) MyList::Next() )
        delete p;
}


void DoubleList::Append( const SEQSEQ( double )& aVLst ) THROWDEF_RTE_IAE
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = aVLst.getLength();
    sal_Int32   nE2;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( double )&    rList = aVLst[ n1 ];
        nE2 = rList.getLength();
        const double*           pList = rList.getConstArray();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
            AppendDouble( pList[ n2 ] );
    }
}


void DoubleList::Append( const SEQSEQ( sal_Int32 )& aVLst ) THROWDEF_RTE_IAE
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = aVLst.getLength();
    sal_Int32   nE2;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( sal_Int32 )& rList = aVLst[ n1 ];
        nE2 = rList.getLength();
        const sal_Int32*        pList = rList.getConstArray();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
            AppendDouble( pList[ n2 ] );
    }
}


void DoubleList::Append( const SEQ( ANY )& aVList, sal_Bool bEmptyAs0, sal_Bool bErrOnEmpty ) THROWDEF_RTE_IAE
{
    sal_Int32           nE = aVList.getLength();
    const ANY*      pList = aVList.getConstArray();

    for( sal_Int32 n = 0 ; n < nE ; n++ )
    {
        const ANY&  r = pList[ n ];

        switch( r.getValueTypeClass() )
        {
            case uno::TypeClass_VOID:       AppendVoid( bErrOnEmpty );      break;
            case uno::TypeClass_STRING:     AppendString( r, bEmptyAs0 );   break;
            case uno::TypeClass_DOUBLE:     AppendDouble( r );              break;
            case uno::TypeClass_SEQUENCE:   AppendAnyArray2( r );           break;
            default:
                THROW_IAE;
        }
    }
}


void DoubleList::Append( const SEQSEQ( ANY )& aVArr, sal_Bool bEmptyAs0, sal_Bool bErrOnEmpty ) THROWDEF_RTE_IAE
{
    sal_Int32               nE1 = aVArr.getLength();

    for( sal_Int32 n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( ANY )&   rVList = aVArr[ n1 ];
        sal_Int32           nE = rVList.getLength();
        const ANY*          pList = rVList.getConstArray();

        for( sal_Int32 n = 0 ; n < nE ; n++ )
        {
            const ANY&  r = pList[ n ];

            switch( r.getValueTypeClass() )
            {
                case uno::TypeClass_VOID:       AppendVoid( bErrOnEmpty );      break;
                case uno::TypeClass_STRING:     AppendString( r, bEmptyAs0 );   break;
                case uno::TypeClass_DOUBLE:     AppendDouble( r );              break;
                default:
                    THROW_IAE;
            }
        }
    }
}


sal_Bool DoubleList::IsProper( double ) const
{
    return sal_True;
}


sal_Bool DoubleList::IsFaulty( double ) const
{
    return sal_False;
}




sal_Bool ChkDoubleList1::IsProper( double f ) const
{
    return f > 0.0;
}


sal_Bool ChkDoubleList1::IsFaulty( double f ) const
{
    return f < 0.0;
}




sal_Bool ChkDoubleList2::IsProper( double f ) const
{
    return f >= 0.0;
}


sal_Bool ChkDoubleList2::IsFaulty( double f ) const
{
    return f < 0.0;
}




Complex::Complex( const STRING& r ) THROWDEF_RTE_IAE
{
    if( !ParseString( r, *this ) )
        THROW_IAE;
}


inline sal_Bool Complex::IsImagUnit( sal_Unicode c )
{
    return c == 'i' || c == 'j';
}


sal_Bool Complex::ParseString( const STRING& rStr, Complex& rCompl )
{
    const sal_Unicode*      pStr = ( const sal_Unicode * ) rStr;


    if( IsImagUnit( *pStr ) && rStr.getLength() == 1 )
    {
        rCompl.r = 0.0;
        rCompl.i = 1.0;
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
                if( pStr[ 2 ] == 0 )
                {
                    rCompl.r = f;
                    rCompl.i = ( *pStr == '+' )? 1.0 : -1.0;
                    return sal_True;
                }
            }
            else if( ParseDouble( pStr, f ) && IsImagUnit( *pStr ) )
            {
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


STRING Complex::GetString( sal_Bool bi ) const
{
    static const STRING aI( "i", 1, RTL_TEXTENCODING_MS_1252 );
    static const STRING aJ( "j", 1, RTL_TEXTENCODING_MS_1252 );
    static const STRING aPlus( "+", 1, RTL_TEXTENCODING_MS_1252 );
    static const STRING aMinus( "-", 1, RTL_TEXTENCODING_MS_1252 );

    STRING              aRet( ::GetString( r ) );

    if( i == 1.0 )
        aRet += aPlus;
    else if( i == -1.0 )
        aRet += aMinus;
    else
        aRet += ::GetString( i, sal_True );

    aRet += bi? aI : aJ;

    return aRet;
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


inline BOOL SinOverflow( double f )
{
    return fabs( f ) >= 134217728;
}


void Complex::Sin( void ) THROWDEF_RTE_IAE
{
    if( SinOverflow( r ) )
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
    if( SinOverflow( r ) )
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

            if( rStr.getLength() )
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
    sal_Int32       nE = aMultPars.getLength();
    sal_Bool        bEmpty0 = eAH == AH_EmpyAs0;
    sal_Bool        bErrOnEmpty = eAH == AH_EmptyAsErr;

    for( sal_Int32 n = 0 ; n < nE ; n++ )
    {
        const ANY&  r = aMultPars[ n ];
        switch( r.getValueTypeClass() )
        {
            case uno::TypeClass_VOID:       break;
            case uno::TypeClass_STRING:
                {
                const STRING*       pStr = ( const STRING* ) r.getValue();

                if( pStr->getLength() )
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




ConvertData::ConvertData( const sal_Char p[], double fC, ConvertDataClass e ) : aName( p, strlen( p ), RTL_TEXTENCODING_MS_1252 )
{
    fConst = fC;
    eClass = e;
}


sal_Int16 ConvertData::GetMatchingLevel( const STRING& rRef ) const
{
    if( aName.equals( rRef ) )
        return 0;
    else
    {
        const sal_Unicode*  p = rRef.getStr();

        if ( aName == p + 1 )
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
                case 'd':   n = -1;     break;
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

    nLevFrom -= nLevTo;                         // effectiv level

    f *= r.fConst / fConst;

    if( nLevFrom )
        f *= Exp10( nLevFrom );

    return f;
}


double ConvertData::ConvertToBase( double f, sal_Int16 n ) const
{
    return f / fConst * Exp10( n );
}


double ConvertData::ConvertFromBase( double f, sal_Int16 n ) const
{
    return f * fConst * Exp10( -n );
}




double ConvertDataLinear::Convert(
    double f, const ConvertData& r, sal_Int16 nLevFrom, sal_Int16 nLevTo ) const THROWDEF_RTE_IAE
{
    if( Class() != r.Class() )
        THROW_IAE;

//  return Round( r.ConvertFromBase( ConvertToBase( f, nLevFrom ), nLevTo ), 13 );
    return r.ConvertFromBase( ConvertToBase( f, nLevFrom ), nLevTo );
}


double ConvertDataLinear::ConvertToBase( double f, sal_Int16 n ) const
{
    if( n )
        f *= Exp10( n );

    f /= fConst;
    f -= fOffs;

    return f;
}


double ConvertDataLinear::ConvertFromBase( double f, sal_Int16 n ) const
{
    f += fOffs;
    f *= fConst;

    if( n )
        f *= Exp10( -n );

    return f;
}




ConvertDataList::ConvertDataList( void )
{
#define NEWD(str,unit,cl)   Append(new ConvertData(str,unit,cl))
#define NEWL(str,unit,offs,cl)  Append(new ConvertDataLinear(str,unit,offs,cl))

    // *** are extra and not standard Excel Analysis Addin!

                                                                // Gramm ->
    NEWD( "g",      1.0000000000000000E00,  CDC_Mass        );      // Gramm
    NEWD( "sg",     6.8522050005347800E-05, CDC_Mass        );      // Stueck
    NEWD( "lbm",    2.2046229146913400E-03, CDC_Mass        );      // Pfund (Handelsgewicht)
//  NEWD( "u",      6.0221700000000000E23,  CDC_Mass        );      // U (atomare Masseinheit)
    NEWD( "u",      6.0221370000000000E23,  CDC_Mass        );      // U (atomare Masseinheit)
    NEWD( "ozm",    3.5273971800362700E-02, CDC_Mass        );      // Unze (Handelsgewicht)
    NEWD( "stone",  1.574730e-04,           CDC_Mass        );      // Stone                                    ***
    NEWD( "ton",    1.102311e-06,           CDC_Mass        );      // ton                                      ***
    NEWD( "grain",  1.543236E01,            CDC_Mass        );      // grain                                    ***
    NEWD( "pweight",    7.054792E-01,       CDC_Mass        );      // Pennyweight                              ***
    NEWD( "hweight",    1.968413E-05,       CDC_Mass        );      // Hundredweight                            ***
    NEWD( "shweight",   2.204623E-05,       CDC_Mass        );      // Shorthundredweight                       ***
    NEWD( "brton",  9.842065E-07,           CDC_Mass        );      // BRT                                      ***

                                                                // Meter ->
    NEWD( "m",      1.0000000000000000E00,  CDC_Length      );      // Meter
    NEWD( "mi",     6.2137119223733397E-04, CDC_Length      );      // Britsche Meile       6,21371192237333969617434184363e-4
    NEWD( "Nmi",    5.3995680345572354E-04, CDC_Length      );      // Nautische Meile      5,39956803455723542116630669546e-4
    NEWD( "in",     3.9370078740157480E01,  CDC_Length      );      // Zoll                 39,37007874015748031496062992126
    NEWD( "ft",     3.2808398950131234E00,  CDC_Length      );      // Fuss                 3,2808398950131233595800524934383
    NEWD( "yd",     1.0936132983377078E00,  CDC_Length      );      // Yard                 1,0936132983377077865266841644794
    NEWD( "ang",    1.0000000000000000E10,  CDC_Length      );      // Angstroem
    NEWD( "Pica",   2.8346456692913386E03,  CDC_Length      );      // Pica (1/72 Zoll)     2834,6456692913385826771653543307
    NEWD( "ell",    8.748906E-01,           CDC_Length      );      // Elle?                                    ***
    NEWD( "parsec", 3.240779E-17,           CDC_Length      );      // parsec                                   ***
    NEWD( "lightyear",1.0570234557732930E-16,CDC_Length     );      // Lichtjahr                                ***

                                                                // Sekunde ->
    NEWD( "yr",     3.1688087814028950E-08, CDC_Time        );      // Jahr
    NEWD( "day",    1.1574074074074074E-05, CDC_Time        );      // Tag
    NEWD( "hr",     2.7777777777777778E-04, CDC_Time        );      // Stunde
    NEWD( "mn",     1.6666666666666667E-02, CDC_Time        );      // Minute
    NEWD( "sec",    1.0000000000000000E00,  CDC_Time        );      // Sekunde

                                                                // Pascal ->
    NEWD( "Pa",     1.0000000000000000E00,  CDC_Pressure    );      // Pascal
    NEWD( "atm",    9.8692329999819300E-06, CDC_Pressure    );      // Atmoshpaere
    NEWD( "mmHg",   7.5006170799862700E-03, CDC_Pressure    );      // mm Quecksilbersaeule
    NEWD( "Torr",   7.5006380000000000E-03, CDC_Pressure    );      // Torr                                     ***
    NEWD( "psi",    1.4503770000000000E-04, CDC_Pressure    );      // psi                                      ***

                                                                // Newton ->
    NEWD( "N",      1.0000000000000000E00,  CDC_Force       );      // Newton
    NEWD( "dyn",    1.0000000000000000E05,  CDC_Force       );      // Dyn
    NEWD( "pond",   1.019716E02,            CDC_Force       );      // Pond

                                                                // Joule ->
    NEWD( "J",      1.0000000000000000E00,  CDC_Energy      );      // Joule
//  NEWD( "e",      9.99999519343231E06,    CDC_Energy      );      // Erg
    NEWD( "e",      1.0000000000000000E07,  CDC_Energy      );      // Erg  -> http://www.chemie.fu-berlin.de/chemistry/general/si.html
    NEWD( "c",      2.3900624947346700E-01, CDC_Energy      );      // Thermodynamische Kalorie
    NEWD( "cal",    2.3884619064201700E-01, CDC_Energy      );      // Kalorie
    NEWD( "eV",     6.2414570000000000E18,  CDC_Energy      );      // Elektronenvolt
//  NEWD( "HPh",    3.72506430801000E-07,   CDC_Energy      );      // Pferdestaerke mal Stunde
    NEWD( "HPh",    3.7250611111111111E-07, CDC_Energy      );      // Horsepower mal Stunde
    NEWD( "Wh",     2.7777777777777778E-04, CDC_Energy      );      // Watt mal Stunde
    NEWD( "BTU",    9.4781506734901500E-04, CDC_Energy      );      // BTU

                                                                // Watt ->
    NEWD( "W",      1.0000000000000000E00,  CDC_Power       );      // Watt
    NEWD( "HP",     1.341022E-03,           CDC_Power       );      // Horsepower
    NEWD( "PS",     1.359622E-03,           CDC_Power       );      // Pferdestaerke                            ***
//EWD( "HP",        1.4102006031908E-03,    CDC_Power       );      // PS   ->Excel seams to be a little bit wrong...
                                                                    //  either this doesn't fit to J -> HPh....

                                                                // Tesla ->
    NEWD( "T",      1.0000000000000000E00,  CDC_Magnetism   );      // Tesla
    NEWD( "ga",     1.0000000000000000E04,  CDC_Magnetism   );      // Gauss

                                                                // Grad Kelvin ->
    NEWL( "C",      1.0000000000000000E00,  -2.7315000000000000E02, CDC_Temperature );      // Grad Celsius
    NEWL( "F",      1.8000000000000000E00,  -2.5537222222222222E02, CDC_Temperature );      // Grad Fahrenheit
    NEWL( "K",      1.0000000000000000E00,  +0.0000000000000000E00, CDC_Temperature );      // Grad Kelvin
    NEWL( "Reau",   8.0000000000000000E-01, -2.7315000000000000E02, CDC_Temperature );      // Grad Reaumur     ***
    NEWL( "Rank",   1.8000000000000000E00,  +0.0000000000000000E00, CDC_Temperature );      // Grad Rankine     ***

                                                                // Liter ->
    NEWD( "tsp",    2.0284000000000000E02,  CDC_Volume      );      // Teeloeffel (Teaspoon)
    NEWD( "tbs",    6.7613333333333333E01,  CDC_Volume      );      // Essloeffel (Tablespoon)
    NEWD( "oz",     3.3806666666666667E01,  CDC_Volume      );      // Unze (Fluessigkeit)
    NEWD( "cup",    4.2258333333333333E00,  CDC_Volume      );      // Tasse (Cup)
    NEWD( "pt",     2.1129166666666667E00,  CDC_Volume      );      // Pint
    NEWD( "qt",     1.0564583333333333E00,  CDC_Volume      );      // Quart
    NEWD( "gal",    2.6411458333333333E-01, CDC_Volume      );      // Gallone
    NEWD( "l",      1.0000000000000000E00,  CDC_Volume      );      // Liter
    NEWD( "m3",     1.0000000000000000E03,  CDC_Volume      );      // Meter^3                                  ***
    NEWD( "mi3",    2.3991275857892772E-10, CDC_Volume      );      // Britsche Meile^3                         ***
    NEWD( "Nmi3",   1.5742621468581148E-10, CDC_Volume      );      // Nautische Meile^3                        ***
    NEWD( "in3",    6.1023744094732284E04,  CDC_Volume      );      // Zoll^3                                   ***
    NEWD( "ft3",    3.5314666721488590E01,  CDC_Volume      );      // Fuss^3                                   ***
    NEWD( "yd3",    1.3079506193143922E00,  CDC_Volume      );      // Yard^3                                   ***
    NEWD( "ang3",   1.0000000000000000E30,  CDC_Volume      );      // Angstroem^3                              ***
    NEWD( "Pica3",  2.2776990435870636E10,  CDC_Volume      );      // Pica^3                                   ***
    NEWD( "barrel", 6.289811E-03,           CDC_Volume      );      // Barrel (=42gal?)                         ***
    NEWD( "bushel", 2.837759E-02,           CDC_Volume      );      // Bushel                                   ***
    NEWD( "regton", 3.531467E-04,           CDC_Volume      );      // Register ton                             ***
    NEWD( "Schooner",2.3529411764705882E00, CDC_Volume      );      // austr. Schooner                          ***
    NEWD( "Middy",  3.5087719298245614E00,  CDC_Volume      );      // austr. Middy                             ***
    NEWD( "Glass",  5.0000000000000000E00,  CDC_Volume      );      // austr. Glass                             ***
    NEWD( "Sixpack",0.5,                    CDC_Volume      );      //                                          ***
    NEWD( "Humpen", 2.0,                    CDC_Volume      );      //                                          ***

                                                                // Meter^2 ->
    NEWD( "m2",     1.0000000000000000E00,  CDC_Area        );      // Meter^2                                  ***
    NEWD( "mi2",    3.8610215854244585E-07, CDC_Area        );      // Britsche Meile^2                         ***
    NEWD( "Nmi2",   2.9155334959812286E-07, CDC_Area        );      // Nautische Meile^2                        ***
    NEWD( "in2",    1.5500031000062000E03,  CDC_Area        );      // Zoll^2                                   ***
    NEWD( "ft2",    1.0763910416709722E01,  CDC_Area        );      // Fuss^2                                   ***
    NEWD( "yd2",    1.0936132983377078E00,  CDC_Area        );      // Yard^2                                   ***
    NEWD( "ang2",   1.0000000000000000E20,  CDC_Area        );      // Angstroem^2                              ***
    NEWD( "Pica2",  8.0352160704321409E06,  CDC_Area        );      // Pica^2                                   ***
    NEWD( "Morgen", 2.553167E03,            CDC_Area        );      // Morgen                                   ***
    NEWD( "ar",     1.000000E02,            CDC_Area        );      // Ar                                       ***
    NEWD( "acre",   4.046856E03,            CDC_Area        );      // acre (oder auch Acker)                   ***
    NEWD( "ha",     1.000000E04,            CDC_Area        );      // Hektar                                   ***
    NEWD( "Quadratlatschen",5.6689342403628117914,CDC_Area  );      //                                          ***

                                                                // Meter/Sekunde ->
    NEWD( "m/s",    1.0000000000000000E00,  CDC_Speed       );      // Meter/Sekunde                            ***
    NEWD( "m/h",    3.6000000000000000E03,  CDC_Speed       );      // Meter/Stunde -> km/h                     ***
    NEWD( "mph",    2.2369362920544023E00,  CDC_Speed       );      // (Britsche) Meilen/Stunde                 ***
    NEWD( "kn",     1.9438444924406048E00,  CDC_Speed       );      // Knoten = (Nautische) Meilen/Stunde       ***
    NEWD( "admkn",  1.9438446603753486E00,  CDC_Speed       );      // Admiraltyknot                            ***
    NEWD( "wahnsinnige Geschwindigkeit",2.0494886343432328E-14,CDC_Speed);//                                    ***
    NEWD( "ludicrous speed",2.0494886343432328E-14,CDC_Speed);      //                                          ***
    NEWD( "laecherliche Geschwindigkeit",4.0156958471424288E-06,CDC_Speed);//                                   ***
    NEWD( "ridiculous speed",4.0156958471424288E-06,CDC_Speed);     //                                          ***

                                                                //
//  Append( new ConvertData( "",        ,   CDC_Length  ) );        //
//  NEWD( "",   ,       );      //
}


ConvertDataList::~ConvertDataList()
{
    for( ConvertData* p = First() ; p ; p = Next() )
        delete p;
}


double ConvertDataList::Convert( double fVal, const STRING& rFrom, const STRING& rTo ) THROWDEF_RTE_IAE
{
    if( rFrom == rTo )
        return fVal;

    ConvertData*    pFrom = NULL;
    ConvertData*    pTo = NULL;
    sal_Bool        bSearchFrom = sal_True;
    sal_Bool        bSearchTo = sal_True;
    sal_Int16       nLevelFrom = 0;
    sal_Int16       nLevelTo = 0;

    ConvertData*    p = First();
    while( p && ( bSearchFrom || bSearchTo ) )
    {
#define CHECKIT(part)

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

