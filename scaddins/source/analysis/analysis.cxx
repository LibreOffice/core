/*************************************************************************
 *
 *  $RCSfile: analysis.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: gt $ $Date: 2001-05-30 11:27:47 $
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

#include "analysis.hxx"
#include "analysishelper.hxx"

#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <tools/solmath.hxx>
#include <string.h>


#define ADDIN_SERVICE       "com.sun.star.sheet.AddIn"
#define MY_SERVICE          "com.sun.star.sheet.addin.Analysis"
#define MY_IMPLNAME         "com.sun.star.sheet.addin.AnalysisImpl"


//------------------------------------------------------------------
//
//  entry points for service registration / instantiation
//
//------------------------------------------------------------------

extern "C" {


void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, registry::XRegistryKey* pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            STRING                          aImpl = STRFROMASCII( "/" );
            aImpl += AnalysisAddIn::getImplementationName_Static();
            aImpl += STRFROMASCII( "/UNO/SERVICES" );

            REF( registry::XRegistryKey )   xNewKey(
                reinterpret_cast< registry::XRegistryKey* >( pRegistryKey )->createKey( aImpl ) );

            SEQ( STRING )                   aSequ = AnalysisAddIn::getSupportedServiceNames_Static();
            const STRING*                   pArray = aSequ.getConstArray();

            for( sal_Int32 i = 0 ; i < aSequ.getLength() ; i++ )
                xNewKey->createKey( pArray[ i ] );

            return sal_True;
        }
        catch( registry::InvalidRegistryException& )
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}


void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    void*                                   pRet = 0;

    if( pServiceManager && STRING::createFromAscii( pImplName ) == AnalysisAddIn::getImplementationName_Static() )
    {
        REF( lang::XSingleServiceFactory )  xFactory( cppu::createOneInstanceFactory(
                reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                AnalysisAddIn::getImplementationName_Static(),
                AnalysisAddIn_CreateInstance,
                AnalysisAddIn::getSupportedServiceNames_Static() ) );

        if( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}


}   // extern C




//------------------------------------------------------------------------
//
//  "normal" service implementation
//
//------------------------------------------------------------------------

#define INTERNPARAM     FuncData::pInternParam

const sal_Char* pFuncDatas[] =
{
//  "iget_Test", "1_test", "2_test",
//      "d4 testing only",
//      "pMode", "pparameter 1", "pparameter 2", "pparameter 3",
//      "PMode", "Pparameter 1", "Pparameter 2", "Pparameter 3",
//      EOE,
    "igetWorkday", "1Arbeitstag", "2Workday",
        "dReturns the serial number of the date before or after a specified number of workdays",
        INTERNPARAM, "pstart date", "pdays", "pholidays",
        INTERNPARAM, "PThe start date for calculating the work day", "PThe number of working days for calculating the work day", "PList of days work off",
        EOE,
    "igetYearfrac", "1Brteiljahre", "2Yearfrac",
        "dReturns the year fraction representing the number of whole days between start_date and end_date",
        INTERNPARAM, "pstart date", "pend date", "pbasis",
        INTERNPARAM, "PThe start date for calculating the fraction of year", "PThe end date for calculating the fraction of year", "PBase for counting days: 0=USA (NASD) 30/360, 1=exact/exact, 2=exact/360, 3=exact/365, 4=Europe 30/360",
        EOE,
    "igetEdate", "1Edatum", "2Edate",
        "dReturns the serial number of the date that is the indicated number of months before or after the start date",
        INTERNPARAM, "pstart date", "pmonths",
        INTERNPARAM, "PThe start date for calculating the edate", "PThe number of months to be added or subtracted",
        EOE,
    "igetWeeknum", "1Kalenderwoche_add", "2Weeknum_add",
        "dReturns the week number in year",
        INTERNPARAM, "pdate", "preturn type",
        INTERNPARAM, "PDate as calculation base for weeknum", "PType of the return value: 1=week starts on sunday, 2=week starts on monday",
        EOE,
    "igetEomonth", "1Monatsende", "2EoMonth",
        "dReturns the serial number of the last day of the month before or after a specified number of months",
        INTERNPARAM, "pstart date", "pmonths",
        INTERNPARAM, "PThe start date for calculating the end of month", "PThe number of months before or after",
        EOE,
    "igetNetworkdays", "1Nettoarbeitstage", "2Networkdays",
        "dReturns the number of whole workdays between to dates",
        INTERNPARAM, "pstart date", "pend date", "pholidays",
        INTERNPARAM, "Pstart date", "Pend date", "Pholidays",
        EOE,
    "igetIseven", "1Istgerade_add", "2Iseven_add",
        "dReturns true if the number is even",
        "pnumber",
        "Pnumber",
        EOE,
    "igetIsodd", "1Istungerade_add", "2Isodd_add",
        "dReturns true if the number is odd",
        "pnumber",
        "Pnumber",
        EOE,
    "igetMultinomial", "1Polynomial", "2Multinomial",
        "dReturns the multinomial of a set of numbers",
        "pnumbers",
        "Pnumbers",
        EOE,
    "igetSeriessum", "1Potenzreihe", "2Seriessum",
        "dReturns the sum of a power series based on the formula",
        "pX", "pN", "pM", "pCoefficients",
        "PX is the value of the independent variable of the serie", "PN is the starting power of X", "PM is the increment from N from one element to the next", "PIs a group of coefficients",
        EOE,
    "igetQuotient", "1Quotient", "2Quotient",
        "dReturns the integer portion of a division",
        "pnumerator", "pdenominator",
        "Pnumerator", "Pdenominator",
        EOE,
    "igetMround", "1Vrunden", "2Mround",
        "dReturns a number rounded to the desire multiple",
        "pnumber", "pmultiple",
        "Pnumber", "Pmultiple",
        EOE,
    "igetSqrtpi", "1Wurzelpi", "2SqrtPI",
        "dReturns the square root of ( number * pi )",
        "pnumber",
        "Pnumber",
        EOE,
    "igetRandbetween", "1Zufallsbereich", "2Randbetween",
        "dReturns a random integer number between the numbers you specify",
        "pbottom", "ptop",
        "Pbottom", "Ptop",
        EOE,
    "igetGcd", "1Ggt_add", "2Gcd_add",
        "dReturns the greatest common divisor",
        "pnumbers", "onumbers",
        "PList of numbers", "OList of numbers",
        EOE,
    "igetLcm", "1Kgv_add", "2Lcm_add",
        "dReturns the least common multiple",
        "pnumbers", "onumbers",
        "PList of numbers", "OList of numbers",
        EOE,
    "igetBesseli", "1Besseli", "2BesselI",
        "dReturns the modified Bessel function In(x)",
        "px", "pn",
        "Px", "Porder",
        EOE,
    "igetBesselj", "1Besselj", "2BesselJ",
        "dReturns the Bessel function Jn(x)",
        "px", "pn",
        "Px", "Porder",
        EOE,
    "igetBesselk", "1Besselk", "2BesselK",
        "dReturns the Bessel function Kn(x)",
        "px", "pn",
        "Px", "Porder",
        EOE,
    "igetBessely", "1Bessely", "2BesselY",
        "dReturns the Bessel function Yn(x)",
        "px", "pn",
        "Px", "Porder",
        EOE,
    "igetBin2oct", "1Bininokt", "2Bin2Oct",
        "dConverts a binary number to octal",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetBin2dec", "1Binindez", "2Bin2Dec",
        "dConverts a binary number to decimal",
        "pnumber",
        "Pnumber",
        EOE,
    "igetBin2hex", "1Bininhex", "2Bin2Hex",
        "dConverts a binary number to hexadecimal",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetOct2bin", "1Oktinbin", "2Oct2Bin",
        "dConverts a octal number to binary",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetOct2dec", "1Oktindez", "2Oct2Dec",
        "dConverts a octal number to decimal",
        "pnumber",
        "Pnumber",
        EOE,
    "igetOct2hex", "1Oktinhex", "2Oct2Hex",
        "dConverts a octal number to hexadecimal",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetDec2bin", "1Dezinbin", "2Dec2Bin",
        "dConverts a decimal number to binary",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetDec2oct", "1Dezinokt", "2Dec2Oct",
        "dConverts a decimal number to octal",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetDec2hex", "1Dezinhex", "2Dec2Hex",
        "dConverts a decimal number to hexadecimal",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetHex2bin", "1Hexinbin", "2Hex2Bin",
        "dConverts a hexadecimal number to binary",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetHex2dec", "1Hexindez", "2Hex2Dec",
        "dConverts a hexadecimal number to decimal",
        "pnumber",
        "Pnumber",
        EOE,
    "igetHex2oct", "1Hexinokt", "2Hex2Oct",
        "dConverts a hexadecimal number to octal",
        "pnumber", "pplaces",
        "Pnumber", "Pplaces",
        EOE,
    "igetDelta", "1Delta", "2Delta",
        "dTests wether two numbers are equal",
        "pnumber 1", "pnumber 2",
        "Pnumber 1", "Pnumber 2",
        EOE,
    "igetErf", "1Gaussfehler", "2Erf",
        "dReturns the error function",
        "plower limit", "pupper limit",
        "Plower limit", "Pupper limit",
        EOE,
    "igetErfc", "1Gaussfkompl", "2Erfc",
        "dReturns the complementary error function",
        "plower limit",
        "Plower limit",
        EOE,
    "igetGestep", "1Gganzzahl", "2GeStep",
        "dTests wether a number is greater than a threshold value",
        "pnumber", "pstep",
        "Pnumber", "Pstep",
        EOE,
    "igetFactdouble", "1Zweifakultät", "2Factdouble",
        "dReturns the double factorial of a number",
        "pnumber",
        "Pnumber",
        EOE,
    "igetImabs", "1Imabs", "2Imabs",
        "dReturns the absolute value (modulus) of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImaginary", "1Imaginärteil", "2Imaginary",
        "dReturns the imaginary coefficient of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImpower", "1Imapotenz", "2Impower",
        "dReturns a complex number raised by a real number",
        "pinumber", "pnumber",
        "Pinumber", "Pnumber",
        EOE,
    "igetImargument", "1Imargument", "2Imargument",
        "dReturns the argument q, an angle expressed in radians",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImcos", "1Imcos", "2Imcos",
        "dReturns the cosine of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImdiv", "1Imdiv", "2Imdiv",
        "dReturns the quotient of two complex numbers",
        "pinumber 1", "pinumber 2",
        "Pinumber 1", "Pinumber 2",
        EOE,
    "igetImexp", "1Imexp", "2Imexp",
        "dReturns the exponential of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImconjugate", "1Imkonjugierte", "2Imconjugate",
        "dReturns the comlex conjugate of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImln", "1Imln", "2Imln",
        "dReturns the natural logarithm of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImlog10", "1Imlog10", "2Imlog10",
        "dReturns the base-10 logarithm of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImlog2", "1Imlog2", "2Imlog2",
        "dReturns the base-2 logarithm of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImproduct", "1Improdukt", "2Improduct",
        "dReturns the product of complex numbers",
        "pinumber 1", "oinumber",
        "Pinumber 1", "Pinumber",
        EOE,
    "igetImreal", "1Imrealteil", "2Imreal",
        "dReturns the real coefficient of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImsin", "1Imsin", "2Imsin",
        "dReturns the sine of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImsub", "1Imsub", "2Imsub",
        "dReturns the difference of two complex numbers",
        "pinumber 1", "pinumber 2",
        "Pinumber 1", "Pinumber 2",
        EOE,
    "igetImsqrt", "1Imwurzel", "2Imsqrt",
        "dReturns the square root of a complex number",
        "pinumber",
        "Pinumber",
        EOE,
    "igetImsum", "1Imsumme", "2Imsum",
        "dReturns the sum of complex numbers",
        "pinumber 1", "oinumber",
        "Pinumber 1", "Pinumber",
        EOE,
    "igetComplex", "1Komplexe", "2Complex",
        "dConverts real and imaginary coefficients into a complex number",
        "preal_num", "pi_num", "psuffix",
        "Preal_num", "Pi_num", "Psuffix",
        EOE,
    "igetConvert", "1Umwandeln_add", "2Convert_add",
        "dConverts a number from one measurement system to another",
        "pnumber", "pfrom_unit", "pto_unit",
        "Pnumber", "Pfrom_unit", "Pto_unit",
        EOE,
// ----------------------------------------------------------------------------
    "igetAmordegrc", "1Amordegrk", "2Amordegrc",
        "dReturns the prorated linear depreciation of an asset for each accounting period",
        INTERNPARAM, "pCost", "pDate purchased", "pFirst period", "pSalvage", "pPeriod", "pRate", "pYear base",
        INTERNPARAM, "PCost", "PDate purchased", "PFirst period", "PSalvage", "PPeriod", "PRate", "PYear base",
        EOE,
    "igetAmorlinc", "1Amorlineark", "2Amorlinc",
        "dReturns the prorated linear depreciation of an asset for each accounting period",
        INTERNPARAM, "pCost", "pDate purchased", "pFirst period", "pSalvage", "pPeriod", "pRate", "pYear base",
        INTERNPARAM, "PCost", "PDate purchased", "PFirst period", "PSalvage", "PPeriod", "PRate", "PYear base",
        EOE,
    "igetAccrint", "1Aufgeldzins", "2Accrint",
        "dReturn the accrued interest for a security that pays periodic interest",
        INTERNPARAM, "pIssue", "pFirst interest", "pSettlement", "pRate", "pPar", "pFrequency", "pBase",
        INTERNPARAM, "PIssue", "PFirst interest", "PSettlement", "PRate", "PPar", "PFrequency", "PBase",
        EOE,
    "igetAccrintm", "1Aufgeldzinsf", "2Accrintm",
        "dReturn the accrued interest for a security that pays interest at maturity",
        INTERNPARAM, "pIssue", "pSettlement", "pRate", "pPar", "pBase",
        INTERNPARAM, "PIssue", "PSettlement", "PRate", "PPar", "PBase",
        EOE,
    "igetReceived", "1Auszahlung", "2Received",
        "dReturns the amount received at maturity for a fully invested security",
        INTERNPARAM, "pSettlement", "pMaturity", "pInvestment", "pDiscount", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PInvestment", "PDiscount", "PBase",
        EOE,
    "igetDisc", "1Disagio", "2Disc",
        "dReturns the discount rate for a security",
        INTERNPARAM, "pSettlement", "pMaturity", "pPrice", "pRedemption", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PPrice", "PRedemption", "PBase",
        EOE,
    "igetDuration", "1Duration_add", "2Duration_add",
        "dReturns the annual duration of a security with periodic interest payments",
        INTERNPARAM, "pSettlement", "pMaturity", "pCoupon", "pYield", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PCoupon", "PYield", "PFrequency", "PBase",
        EOE,
    "igetEffect", "1Effektiv_add", "2Effect_add",
        "dReturns the effectiv annual interest rate",
        "pNominal rate", "pNum periods",
        "PNominal rate", "PNum periods",
        EOE,
    "igetCumprinc", "1Kumkapital_add", "2Cumprinc_add",
        "dReturns the cumulative principal paid a loan between two periods",
        "pRate", "pNum periods", "pValue", "pStart period", "pEnd period", "pType payment",
        "PRate", "PNum periods", "PValue", "PStart period", "PEnd period", "PType payment",
        EOE,
    "igetCumipmt", "1Kumzins_add", "2Cumipmt_add",
        "dReturns the cumulative interest paid between two periods",
        "pRate", "pNum periods", "pValue", "pStart period", "pEnd period", "pType payment",
        "PRate", "PNum periods", "PValue", "PStart period", "PEnd period", "PType payment",
        EOE,
    "igetPrice", "1Kurs", "2Price",
        "dReturns the price per $100 face value of a security that pays periodic interest",
        INTERNPARAM, "pSettlement", "pMaturity", "pRate", "pYield", "pRedemption", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PRate", "PYield", "PRedemption", "PFrequency", "PBase",
        EOE,
    "igetPricedisc", "1Kursdisagio", "2Pricedisc",
        "dReturns the price per $100 face value of a discounted security",
        INTERNPARAM, "pSettlement", "pMaturity", "pDiscount", "pRedemption", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PDiscount", "PRedemption", "PBase",
        EOE,
    "igetPricemat", "1Kursfällig", "2Pricemat",
        "dReturns the price per $100 face value of a security that pays interest at maturity",
        INTERNPARAM, "pSettlement", "pMaturity", "pIssue", "pRate", "pYield", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PIssue", "PRate", "PYield", "PBase",
        EOE,
    "igetMduration", "1Mduration", "2Mduration",
        "dReturns the Macauley modified duration for a security with an assumed par ",
        INTERNPARAM, "pSettlement", "pMaturity", "pCoupon", "pYield", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PCoupon", "PYield", "PFrequency", "PBase",
        EOE,
    "igetNominal", "1Nominal_add", "2Nominal_add",
        "dReturns the annual nominal interest rate",
        "pEffective rate", "pNum periods",
        "PEffective rate", "PNum periods",
        EOE,
    "igetDollarfr", "1Notierungbru", "2Dollarfr",
        "dConverts a dollar price, expressed as a decimal number, into a dollar price, expressed as a fraction",
        "pDecimal dollar", "pFraction",
        "PDecimal dollar", "PFraction",
        EOE,
    "igetDollarde", "1Notierungdez", "2Dollarde",
        "dConverts a dollar price, expressed as a fraction, into a dollar price, expressed as a decimal number",
        "pFractional dollar", "pFraction",
        "PFractional dollar", "PFraction",
        EOE,
    "igetYield", "1Rendite", "2Yield",
        "dReturns the yield on a security that pays periodic interest",
        INTERNPARAM, "pSettlement", "pMaturity", "pRate", "pPar", "pRedemption", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PRate", "PPar", "PRedemption", "PFrequency", "PBase",
        EOE,
    "igetYielddisc", "1Renditedis", "2Yielddisc",
        "dReturns the annual yield for a discounted security (e.g. treasury bill)",
        INTERNPARAM, "pSettlement", "pMaturity", "pPrice", "pRedemption", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PPrice", "PRedemption", "PBase",
        EOE,
    "igetYieldmat", "1Renditefäll", "2Yieldmat",
        "dReturns the annual yield of a security that pays interest at maturity",
        INTERNPARAM, "pSettlement", "pMaturity", "pIssue", "pRate", "pPrice", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PIssue", "PRate", "PPrice", "PBase",
        EOE,
    "igetTbilleq", "1Tbilläquiv", "2Tbilleq",
        "dReturns the bond-equivalent yield for a treasury bill",
        INTERNPARAM, "pSettlement", "pMaturity", "pDiscount",
        INTERNPARAM, "PSettlement", "PMaturity", "PDiscount",
        EOE,
    "igetTbillprice", "1Tbillkurs", "2Tbillprice",
        "dReturns the price of $100 face value for a treasury bill",
        INTERNPARAM, "pSettlement", "pMaturity", "pDiscount",
        INTERNPARAM, "PSettlement", "PMaturity", "PDiscount",
        EOE,
    "igetTbillyield", "1Tbillrendite", "2Tbillyield",
        "dReturns the yield for a treasury bill",
        INTERNPARAM, "pSettlement", "pMaturity", "pPrice",
        INTERNPARAM, "PSettlement", "PMaturity", "PPrice",
        EOE,
    "igetOddfprice", "1Unreger_kurs", "2Oddfprice",
        "dReturns the price of $100 face value of a security with an odd first period",
        INTERNPARAM, "pSettlement", "pMaturity", "pIssue", "pFirst coupon", "pRate", "pYield", "pRedemption", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PIssue", "PFirst coupon", "PRate", "PYield", "PRedemption", "PFrequency", "PBase",
        EOE,
    "igetOddfyield", "1Unreger_rend", "2Oddfyield",
        "dReturns the yield of a security with an odd first period",
        INTERNPARAM, "pSettlement", "pMaturity", "pIssue", "pFirst coupon", "pRate", "pPrice", "pRedemption", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PIssue", "PFirst coupon", "PRate", "PPrice", "PRedemption", "PFrequency", "PBase",
        EOE,
    "igetOddlprice", "1Unregle_kurs", "2Oddlprice",
        "dReturns the price of $100 face value of a security with an odd last period",
        INTERNPARAM, "pSettlement", "pMaturity", "pLast interst", "pRate", "pYield", "pRedemption", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PLast interst", "PRate", "PYield", "PRedemption", "PFrequency", "PBase",
        EOE,
    "igetOddlyield", "1Unregle_rend", "2Oddlyield",
        "dReturns the yield of a security with an odd last period",
        INTERNPARAM, "pSettlement", "pMaturity", "pLast interest", "pRate", "pPrice", "pRedemption", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PLast interest", "PRate", "PPrice", "PRedemption", "PFrequency", "PBase",
        EOE,
    "igetXirr", "1Xintzinsfuss", "2Xirr",
        "dReturns the internal rate of return for a schedule of cash flows",
        "pValues", "pDates", "pGuess",
        "PValues", "PDates", "PGuess",
        EOE,
    "igetXnpv", "1Xkapitalwert", "2Xnpv",
        "dReturns the net present value for a schedule of cash flows",
        "pRate", "pValues", "pDates",
        "PRate", "PValues", "PDates",
        EOE,
    "igetIntrate", "1Zinssatz", "2Intrate",
        "dReturns the interest rate for a fully invested security",
        INTERNPARAM, "pSettlement", "pMaturity", "pInvestment", "pRedemption", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PInvestment", "PRedemption", "PBase",
        EOE,
    "igetCoupncd", "1Zinstermnz", "2Coupncd",
        "dReturns the next coupon date after the settlement date",
        INTERNPARAM, "pSettlement", "pMaturity", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PFrequency", "PBase",
        EOE,
    "igetCoupdays", "1Zinstermtage", "2Coupdays",
        "dReturns the number of days in the coupon period that contains the settlement date",
        INTERNPARAM, "pSettlement", "pMaturity", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PFrequency", "PBase",
        EOE,
    "igetCoupdaysnc", "1Zinstermtagnz", "2Coupdaysnc",
        "dReturns the number of days from the settlement date to the next coupon date",
        INTERNPARAM, "pSettlement", "pMaturity", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PFrequency", "PBase",
        EOE,
    "igetCoupdaybs", "1Zinstermtagva", "2Coupdaybs",
        "dReturns the number of days from the beginning of the coupon period to the settlement date",
        INTERNPARAM, "pSettlement", "pMaturity", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PFrequency", "PBase",
        EOE,
    "igetCouppcd", "1Zinstermvz", "2Couppcd",
        "dReturns the previous coupon date before the settlement date",
        INTERNPARAM, "pSettlement", "pMaturity", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PFrequency", "PBase",
        EOE,
    "igetCoupnum", "1Zinstermzahl", "2Coupnum",
        "dReturns the number of coupons payable between the settlement date and maturity date",
        INTERNPARAM, "pSettlement", "pMaturity", "pFrequency", "pBase",
        INTERNPARAM, "PSettlement", "PMaturity", "PFrequency", "PBase",
        EOE,
    "igetFvschedule", "1Zw2", "2Fvschedule",
        "dReturns the future value of an initial principal after applying a series of compound interest rates",
        "pPrincipal", "pSchedule",
        "PPrincipal", "PSchedule",
        EOE,
    EOL
};


AnalysisAddIn::AnalysisAddIn()
{
    pFD = new FuncDataList( pFuncDatas );
    pFactDoubles = NULL;
    pCDL = NULL;
}


AnalysisAddIn::~AnalysisAddIn()
{
    delete pFD;

    if( pFactDoubles )
        delete pFactDoubles;

    if( pCDL )
        delete pCDL;
}


#define MAXFACTDOUBLE   300

double AnalysisAddIn::FactDouble( sal_Int32 nNum ) THROWDEF_RTE_IAE
{
    if( nNum < 0 || nNum > MAXFACTDOUBLE )
        THROW_IAE;

    if( !pFactDoubles )
    {
        pFactDoubles = new double[ MAXFACTDOUBLE + 1 ];

        pFactDoubles[ 0 ] = 1.0;    // by default

        double      fOdd = 1.0;
        double      fEven = 2.0;

        pFactDoubles[ 1 ] = fOdd;
        pFactDoubles[ 2 ] = fEven;

        sal_Bool    bOdd = sal_True;

        for( sal_uInt16 nCnt = 3 ; nCnt <= MAXFACTDOUBLE ; nCnt++ )
        {
            if( bOdd )
            {
                fOdd *= nCnt;
                pFactDoubles[ nCnt ] = fOdd;
            }
            else
            {
                fEven *= nCnt;
                pFactDoubles[ nCnt ] = fEven;
            }

            bOdd = !bOdd;

        }
    }

    return pFactDoubles[ nNum ];
}


STRING AnalysisAddIn::getImplementationName_Static()
{
    return STRFROMASCII( MY_IMPLNAME );
}


SEQ( STRING ) AnalysisAddIn::getSupportedServiceNames_Static()
{
    SEQ( STRING )   aRet(2);
    STRING*         pArray = aRet.getArray();
    pArray[0] = STRFROMASCII( ADDIN_SERVICE );
    pArray[1] = STRFROMASCII( MY_SERVICE );
    return aRet;
}


REF( uno::XInterface ) SAL_CALL AnalysisAddIn_CreateInstance( const REF( lang::XMultiServiceFactory )& )
{
    static REF( uno::XInterface )   xInst = ( cppu::OWeakObject* ) new AnalysisAddIn();
    return xInst;
}


// XServiceName

STRING SAL_CALL AnalysisAddIn::getServiceName() THROWDEF_RTE
{
    // name of specific AddIn service
    return STRFROMASCII( MY_SERVICE );
}


// XServiceInfo

STRING SAL_CALL AnalysisAddIn::getImplementationName() THROWDEF_RTE
{
    return getImplementationName_Static();
}


sal_Bool SAL_CALL AnalysisAddIn::supportsService( const STRING& aName ) THROWDEF_RTE
{
    return aName.compareToAscii( ADDIN_SERVICE ) == 0 || aName.compareToAscii( MY_SERVICE ) == 0;
}


SEQ( STRING ) SAL_CALL AnalysisAddIn::getSupportedServiceNames() THROWDEF_RTE
{
    return getSupportedServiceNames_Static();
}


// XLocalizable

void SAL_CALL AnalysisAddIn::setLocale( const lang::Locale& eLocale ) THROWDEF_RTE
{
    aFuncLoc = eLocale;
}

lang::Locale SAL_CALL AnalysisAddIn::getLocale() THROWDEF_RTE
{
    return aFuncLoc;
}


// XAddIn

STRING SAL_CALL AnalysisAddIn::getProgrammaticFuntionName( const STRING& aDisplayName ) THROWDEF_RTE
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)

    return STRING();
}


STRING SAL_CALL AnalysisAddIn::getDisplayFunctionName( const STRING& aProgrammaticName ) THROWDEF_RTE
{
    //! allow different languages
    sal_Bool        bGerman = ( aFuncLoc.Language.equalsIgnoreAsciiCase( STRFROMASCII( "DE" ) ) );

    STRING          aRet;

    const FuncData* p = pFD->Get( aProgrammaticName );
    if( p )
        aRet = STRFROMANSI( bGerman ? p->pGerman : p->pEnglish );

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getFunctionDescription( const STRING& aProgrammaticName ) THROWDEF_RTE
{
    //! return translated strings

    STRING          aRet;

    const FuncData* p = pFD->Get( aProgrammaticName );
    if( p )
        aRet = STRFROMANSI( p->pDescr );

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getDisplayArgumentName( const STRING& aName, sal_Int32 nArg ) THROWDEF_RTE
{
    //! return translated strings
    STRING          aRet;

    const FuncData* p = pFD->Get( aName );
    if( p )
        aRet = STRFROMANSI( p->GetParam( nArg ) );

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getArgumentDescription( const STRING& aName, sal_Int32 nArg ) THROWDEF_RTE
{
    //! return translated strings
    STRING          aRet;
    const FuncData* p = pFD->Get( aName );
    if( p )
        aRet = STRFROMANSI( p->GetParamDescr( nArg ) );

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getProgrammaticCategoryName( const STRING& aName ) THROWDEF_RTE
{
    //  return non-translated strings
    return STRFROMASCII( "Add-In" );
}


STRING SAL_CALL AnalysisAddIn::getDisplayCategoryName( const STRING& aProgrammaticFunctionName ) THROWDEF_RTE
{
    //  return translated strings, not used for predefined categories
    return STRFROMASCII( "Add-In" );
}


// XAnalysis

/*double SAL_CALL AnalysisAddIn::get_Test( constREFXPS&,
    sal_Int32 nMode, double f1, double f2, double f3 ) THROWDEF_RTE
{
    return _Test( nMode, f1, f2, f3 );
}*/


/**
 * Workday
 */

sal_Int32 SAL_CALL AnalysisAddIn::getWorkday( constREFXPS& xOptions,
    sal_Int32 nDate, sal_Int32 nDays, const ANY& aHDay ) THROWDEF_RTE
{
    if( !nDays )
        return nDate;

    sal_Int32                   nNullDate = GetNullDate( xOptions );

    SortedIndividualInt32List   aSrtLst;

    aSrtLst.InsertHolidayList( aHDay, nNullDate, sal_False );

    sal_Int32                   nActDate = nDate + nNullDate;

    if( nDays > 0 )
    {
        while( nDays )
        {
            nActDate++;

            if( GetDayOfWeek( nActDate ) < 5 )
            {
                if( !aSrtLst.Find( nActDate ) )
                    nDays--;
            }
            else
                nActDate++;     // jump over weekend
        }
    }
    else
    {
        while( nDays )
        {
            nActDate--;

            if( GetDayOfWeek( nActDate ) < 5 )
            {
                if( !aSrtLst.Find( nActDate ) )
                    nDays--;
            }
            else
                nActDate--;     // jump over weekend
        }
    }

    return nActDate - nNullDate;
}


/**
 * Yearfrac
 */

double SAL_CALL AnalysisAddIn::getYearfrac( constREFXPS& xOpt,
    sal_Int32 nStartDate, sal_Int32 nEndDate, const ANY& rMode ) THROWDEF_RTE_IAE
{
    return GetYearFrac( xOpt, nStartDate, nEndDate, GetOptBase( rMode ) );
}


sal_Int32 SAL_CALL AnalysisAddIn::getEdate( constREFXPS& xOpt, sal_Int32 nStartDate, sal_Int32 nMonths ) THROWDEF_RTE
{
    sal_Int32   nNullDate = GetNullDate( xOpt );
    sal_Int32   nDate = nStartDate + nNullDate;
    sal_uInt16  nDay, nMonth, nYear;
    DaysToDate( nDate, nDay, nMonth, nYear );

    nYear += nMonths / 12;
    nMonth += nMonths % 12;

    sal_uInt16  nDaysInMonth = DaysInMonth( nMonth, nYear );
    if( nDay > nDaysInMonth )
        nDay = nDaysInMonth;

    return DateToDays( nDay, nMonth, nYear ) - nNullDate;
}


sal_Int32 SAL_CALL AnalysisAddIn::getWeeknum( constREFXPS& xOpt, sal_Int32 nDate, sal_Int32 nMode ) THROWDEF_RTE
{
    nDate += GetNullDate( xOpt );

    sal_uInt16  nDay, nMonth, nYear;
    DaysToDate( nDate, nDay, nMonth, nYear );

    sal_Int32   nFirstInYear = DateToDays( 1, 1, nYear );
    sal_uInt16  nFirstDayInYear = GetDayOfWeek( nFirstInYear );

    return ( nDate - nFirstInYear + ( ( nMode == 1 )? ( nFirstDayInYear + 1 ) % 7 : nFirstDayInYear ) ) / 7 + 1;
}


sal_Int32 SAL_CALL AnalysisAddIn::getEomonth( constREFXPS& xOpt, sal_Int32 nDate, sal_Int32 nMonths ) THROWDEF_RTE
{
    sal_Int32   nNullDate = GetNullDate( xOpt );
    nDate += nNullDate;
    sal_uInt16  nDay, nMonth, nYear;
    DaysToDate( nDate, nDay, nMonth, nYear );

    sal_Int32   nNewMonth = nMonth + nMonths;

    if( nNewMonth > 12 )
    {
        nYear += sal_uInt16( nNewMonth / 12 );
        nNewMonth %= 12;
    }
    else if( nNewMonth < 1 )
    {
        nNewMonth = -nNewMonth;
        nYear += sal_uInt16( nNewMonth / 12 );
        nYear--;
        nNewMonth %= 12;
        nNewMonth = 12 - nNewMonth;
    }

    return DateToDays( DaysInMonth( sal_uInt16( nNewMonth ), nYear ), sal_uInt16( nNewMonth ), nYear ) - nNullDate;
}


sal_Int32 SAL_CALL AnalysisAddIn::getNetworkdays( constREFXPS& xOpt,
        sal_Int32 nStartDate, sal_Int32 nEndDate, const ANY& aHDay ) THROWDEF_RTE
{
    sal_Int32                   nNullDate = GetNullDate( xOpt );

    SortedIndividualInt32List   aSrtLst;

    aSrtLst.InsertHolidayList( aHDay, nNullDate, sal_False );

    sal_Int32                   nActDate = nStartDate + nNullDate;
    sal_Int32                   nStopDate = nEndDate + nNullDate;
    sal_Int32                   nCnt = 0;

    if( nActDate <= nStopDate )
    {
        while( nActDate <= nStopDate )
        {
            if( GetDayOfWeek( nActDate ) < 5 && !aSrtLst.Find( nActDate ) )
                nCnt++;

            nActDate++;
        }
    }
    else
    {
        while( nActDate >= nStopDate )
        {
            if( GetDayOfWeek( nActDate ) < 5 && !aSrtLst.Find( nActDate ) )
                nCnt--;

            nActDate--;
        }
    }

    return nCnt;
}


sal_Int32 SAL_CALL AnalysisAddIn::getIseven( sal_Int32 nVal ) THROWDEF_RTE
{
    return ( nVal & 0x00000001 )? 0 : 1;
}


sal_Int32 SAL_CALL AnalysisAddIn::getIsodd( sal_Int32 nVal ) THROWDEF_RTE
{
    return ( nVal & 0x00000001 )? 1 : 0;
}


double SAL_CALL AnalysisAddIn::getMultinomial( const SEQSEQ( sal_Int32 )& aV ) THROWDEF_RTE_IAE
{
    sal_Int32       n1, n2;
    sal_Int32       nE1 = aV.getLength();
    sal_Int32       nE2;
    sal_Int32       nZ = 0;
    double          fN = 1.0;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( sal_Int32 )& rList = aV[ n1 ];
        nE2 = rList.getLength();
        const sal_Int32*    pList = rList.getConstArray();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
        {
            sal_Int32       n = pList[ n2 ];

            if( n < 0 || n > 170 )
                THROW_IAE;
            else if( n > 0 )
            {
                nZ += n;
                fN *= Fak( n );
            }
        }
    }

    if( nZ <= 170 )
        return Fak( nZ ) / fN;
    else
        THROW_IAE;
}


double SAL_CALL AnalysisAddIn::getSeriessum( double fX, double fN, double fM, const SEQSEQ( double )& aCoeffList ) THROWDEF_RTE
{
    double                          fRet = 0.0;

    if( fX != 0.0 )
    {
        sal_Int32       n1, n2;
        sal_Int32       nE1 = aCoeffList.getLength();
        sal_Int32       nE2;
        sal_Int32       nZ = 0;

        for( n1 = 0 ; n1 < nE1 ; n1++ )
        {
            const SEQ( double )&    rList = aCoeffList[ n1 ];
            nE2 = rList.getLength();
            const double*           pList = rList.getConstArray();

            for( n2 = 0 ; n2 < nE2 ; n2++ )
            {
                fRet += pList[ n2 ] * pow( fX, fN );

                fN += fM;
            }
        }
    }

    return fRet;
}


double SAL_CALL AnalysisAddIn::getQuotient( double fNum, double fDenum ) THROWDEF_RTE
{
    return SolarMath::ApproxFloor( fNum / fDenum );
}


double SAL_CALL AnalysisAddIn::getMround( double fNum, double fMult ) THROWDEF_RTE
{
    if( fMult == 0.0 )
        return fMult;

    return fMult * SolarMath::Round( fNum / fMult );
}


double SAL_CALL AnalysisAddIn::getSqrtpi( double fNum ) THROWDEF_RTE
{
    return sqrt( fNum * PI );
}


double SAL_CALL AnalysisAddIn::getRandbetween( double fMin, double fMax ) THROWDEF_RTE_IAE
{
    const SolarMathRoundingMode     eRM = SolarMathRoundUp;
    fMin = SolarMath::Round( fMin, 0, eRM );
    fMax = SolarMath::Round( fMax, 0, eRM );

    if( fMin > fMax )
        THROW_IAE;

    // fMax -> range
    fMax -= fMin;
    fMax /= double( RAND_MAX );

    return SolarMath::Round( fMin + fMax * double( rand() ) );
}


double SAL_CALL AnalysisAddIn::getGcd( const SEQSEQ( double )& aVLst, const SEQ( uno::Any )& aOptVLst ) THROWDEF_RTE_IAE
{
    ChkDoubleList1  aValList;

    aValList.Append( aVLst );
    aValList.Append( aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    const double*   p = aValList.First();
    double          f = *p;

    p = aValList.Next();

    while( p )
    {
        f = GetGcd( *p, f );
        p = aValList.Next();
    }

    return f;
}


double SAL_CALL AnalysisAddIn::getLcm( const SEQSEQ( ANY )& aVLst, const SEQ( uno::Any )& aOptVLst ) THROWDEF_RTE_IAE
{
    ChkDoubleList2  aValList;

    aValList.Append( aVLst );
    aValList.Append( aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    const double*   p = aValList.First();
    double          f = *p;

    if( f == 0.0 )
        return f;

    p = aValList.Next();

    while( p )
    {
        double      fTmp = *p;
        if( f == 0.0 )
            return f;
        else
            f = fTmp * f / GetGcd( fTmp, f );
        p = aValList.Next();
    }

    return f;
}


double SAL_CALL AnalysisAddIn::getBesseli( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE
{
    return Bessel( fNum, nOrder, sal_True );
}


double SAL_CALL AnalysisAddIn::getBesselj( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE
{
    return Bessel( fNum, nOrder, sal_False );
}


double SAL_CALL AnalysisAddIn::getBesselk( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE
{
    THROW_IAE;

    if( nOrder < 0 )
        THROW_IAE;

    return Besselk( fNum, nOrder );
}


double SAL_CALL AnalysisAddIn::getBessely( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE
{
    THROW_IAE;

    if( nOrder < 0 )
        THROW_IAE;
    return 0.0; // temporary solution ;-)
}


#define _P      10                  // max. number of places
#define _MIN2   -512                // min. val for binary numbers
#define _MAX2   511                 // min. val for binary numbers
#define _MIN8   -536870912          // min. val for octal numbers
#define _MAX8   536870911           // max. val for octal numbers
#define _MIN16  -1099511627776      // min. val for hexadecimal numbers
#define _MAX16  1099511627775       // max. val for hexadecimal numbers
#define GETPLACES()                 GetOpt(rPlaces,sal_Int32(-(2^31)))
#define DOUBLECONV(from,to)         ConvertFromDec(sal_Int64(ConvertToDec(aNum,from,_P)),_MIN##to,_MAX##to,to,GETPLACES(),_P)


STRING SAL_CALL AnalysisAddIn::getBin2oct( const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return DOUBLECONV( 2, 8 );
}


double SAL_CALL AnalysisAddIn::getBin2dec( const STRING& aNum ) THROWDEF_RTE_IAE
{
    return ConvertToDec( aNum, 2, _P );
}


STRING SAL_CALL AnalysisAddIn::getBin2hex( const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return DOUBLECONV( 2, 16 );
}


STRING SAL_CALL AnalysisAddIn::getOct2bin( const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return DOUBLECONV( 8, 2 );
}


double SAL_CALL AnalysisAddIn::getOct2dec( const STRING& aNum ) THROWDEF_RTE_IAE
{
    return ConvertToDec( aNum, 8, _P );
}


STRING SAL_CALL AnalysisAddIn::getOct2hex( const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return DOUBLECONV( 8, 16 );
}


STRING SAL_CALL AnalysisAddIn::getDec2bin( sal_Int32 nNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return ConvertFromDec( nNum, _MIN2, _MAX2, 2, GETPLACES(), _P );
}


STRING SAL_CALL AnalysisAddIn::getDec2oct( sal_Int32 nNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return ConvertFromDec( nNum, _MIN8, _MAX8, 8, GETPLACES(), _P );
}


STRING SAL_CALL AnalysisAddIn::getDec2hex( double fNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return ConvertFromDec( sal_Int64( fNum ), _MIN16, _MAX16, 16, GETPLACES(), _P );
}


STRING SAL_CALL AnalysisAddIn::getHex2bin( const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return DOUBLECONV( 16, 2 );
}


double SAL_CALL AnalysisAddIn::getHex2dec( const STRING& aNum ) THROWDEF_RTE_IAE
{
    return ConvertToDec( aNum, 16, _P );
}


STRING SAL_CALL AnalysisAddIn::getHex2oct( const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    return DOUBLECONV( 16, 8 );
}


sal_Int32 SAL_CALL AnalysisAddIn::getDelta( double fNum1, const ANY& rNum2 ) THROWDEF_RTE
{
#ifdef DEBUG
    return ( fNum1 == GetOpt( rNum2, 0.0 ) )? 1 : 0;
#else
    return fNum1 == GetOpt( rNum2, 0.0 );
#endif
}


double SAL_CALL AnalysisAddIn::getErf( double fLL, const ANY& rUL ) THROWDEF_RTE
{
    switch( rUL.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:
            return Erf( fLL );
            break;
        case uno::TypeClass_DOUBLE:
            double  fUL = *( double* ) rUL.getValue();
            return Erf( fUL ) - Erf( fLL );
            break;
    }

    THROW_IAE;

    return 0.0;
}


double SAL_CALL AnalysisAddIn::getErfc( double f ) THROWDEF_RTE
{
    return 1.0 - Erf( f );
}


sal_Int32 SAL_CALL AnalysisAddIn::getGestep( double fNum, const ANY& rStep ) THROWDEF_RTE
{
#ifdef DEBUG
    return ( fNum >= GetOpt( rStep, 0.0 ) )? 1 : 0;
#else
    return fNum >= GetOpt( rStep, 0.0 );
#endif
}


double SAL_CALL AnalysisAddIn::getFactdouble( sal_Int32 nNum ) THROWDEF_RTE_IAE
{
    return FactDouble( nNum );
}


double SAL_CALL AnalysisAddIn::getImabs( const STRING& aNum ) THROWDEF_RTE_IAE
{
    return Complex( aNum ).Abs();
}


double SAL_CALL AnalysisAddIn::getImaginary( const STRING& aNum ) THROWDEF_RTE_IAE
{
    return Complex( aNum ).Imag();
}


STRING SAL_CALL AnalysisAddIn::getImpower( const STRING& aNum, double f ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Power( f );

    return z.GetString();
}


double SAL_CALL AnalysisAddIn::getImargument( const STRING& aNum ) THROWDEF_RTE_IAE
{
    return Complex( aNum ).Arg();
}


STRING SAL_CALL AnalysisAddIn::getImcos( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Cos();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImdiv( const STRING& aDivid, const STRING& aDivis ) THROWDEF_RTE_IAE
{
    Complex     z( aDivid );

    z.Div( Complex( aDivis ) );

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImexp( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Exp();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImconjugate( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Conjugate();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImln( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Ln();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImlog10( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Log10();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImlog2( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Log2();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImproduct( const STRING& aNum1, const SEQ( uno::Any )& aNL ) THROWDEF_RTE_IAE
{
    Complex         z( aNum1 );

    ComplexList     aNumList;

    aNumList.Append( aNL );

    for( const Complex* p = aNumList.First(); p ; p = aNumList.Next() )
        z.Mult( *p );

    return z.GetString();
}


double SAL_CALL AnalysisAddIn::getImreal( const STRING& aNum ) THROWDEF_RTE_IAE
{
    return Complex( aNum ).Real();
}


STRING SAL_CALL AnalysisAddIn::getImsin( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Sin();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsub( const STRING& aNum1, const STRING& aNum2 ) THROWDEF_RTE_IAE
{
    Complex     z( aNum1 );

    z.Sub( Complex( aNum2 ) );

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsum( const STRING& aNum1, const SEQ( CSS::uno::Any )& aFollowingPars ) THROWDEF_RTE_IAE
{
    Complex         z( aNum1 );

    ComplexList     z_list;

    z_list.Append( aFollowingPars );

    for( const Complex* p = z_list.First() ; p ; p = z_list.Next() )
        z.Add( *p );

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsqrt( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Power( 0.5 );

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getComplex( double fR, double fI, const ANY& rSuff ) THROWDEF_RTE_IAE
{
    sal_Bool    bi;

    switch( rSuff.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:
            bi = sal_True;
            break;
        case uno::TypeClass_STRING:
            {
            const STRING*   pSuff = ( const STRING* ) rSuff.getValue();
            bi = pSuff->compareToAscii( "i" ) == 0 || pSuff->getLength() == 0;
            if( !bi && pSuff->compareToAscii( "j" ) != 0 )
                THROW_IAE;
            }
            break;
        default:
            THROW_IAE;
    }

    return Complex( fR,fI ).GetString( bi );
}


double SAL_CALL AnalysisAddIn::getConvert( double f, const STRING& aFU, const STRING& aTU ) THROWDEF_RTE_IAE
{
    if( !pCDL )
        pCDL = new ConvertDataList();

    return pCDL->Convert( f, aFU, aTU );
}


