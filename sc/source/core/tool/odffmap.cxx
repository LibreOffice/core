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

#include "compiler.hxx"
#include <sal/macros.h>

// ODFF, English, MapDupToInternal when writing ODFF, Programmatical, ODF_11
// functions duplicated to internal when writing ODFF are listed in static const XclFunctionInfo saFuncTable_4[]
ScCompiler::AddInMap ScCompiler::maAddInMap[] =
{
    { "ORG.OPENOFFICE.WEEKS", "WEEKS", "com.sun.star.sheet.addin.DateFunctions.getDiffWeeks", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFWEEKS" },
    { "ORG.OPENOFFICE.MONTHS", "MONTHS", "com.sun.star.sheet.addin.DateFunctions.getDiffMonths", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFMONTHS" },
    { "ORG.OPENOFFICE.YEARS", "YEARS", "com.sun.star.sheet.addin.DateFunctions.getDiffYears", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFYEARS" },
    { "ORG.OPENOFFICE.ISLEAPYEAR", "ISLEAPYEAR", "com.sun.star.sheet.addin.DateFunctions.getIsLeapYear", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETISLEAPYEAR" },
    { "ORG.OPENOFFICE.DAYSINMONTH", "DAYSINMONTH", "com.sun.star.sheet.addin.DateFunctions.getDaysInMonth", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINMONTH" },
    { "ORG.OPENOFFICE.DAYSINYEAR", "DAYSINYEAR", "com.sun.star.sheet.addin.DateFunctions.getDaysInYear", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINYEAR" },
    { "ORG.OPENOFFICE.WEEKSINYEAR", "WEEKSINYEAR", "com.sun.star.sheet.addin.DateFunctions.getWeeksInYear", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETWEEKSINYEAR" },
    { "ORG.OPENOFFICE.ROT13", "ROT13", "com.sun.star.sheet.addin.DateFunctions.getRot13", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETROT13" },
    { "WORKDAY", "WORKDAY", "com.sun.star.sheet.addin.Analysis.getWorkday", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWORKDAY" },
    { "YEARFRAC", "YEARFRAC", "com.sun.star.sheet.addin.Analysis.getYearfrac", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYEARFRAC" },
    { "EDATE", "EDATE", "com.sun.star.sheet.addin.Analysis.getEdate", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEDATE" },
    { "WEEKNUM", "WEEKNUM_EXCEL2003", "com.sun.star.sheet.addin.Analysis.getWeeknum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWEEKNUM" },
    { "EOMONTH", "EOMONTH", "com.sun.star.sheet.addin.Analysis.getEomonth", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEOMONTH" },
    { "NETWORKDAYS", "NETWORKDAYS_XCL", "com.sun.star.sheet.addin.Analysis.getNetworkdays", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETNETWORKDAYS" },
    { "ISEVEN", "ISEVEN_ADD", "com.sun.star.sheet.addin.Analysis.getIseven", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISEVEN" },
    { "ISODD", "ISODD_ADD", "com.sun.star.sheet.addin.Analysis.getIsodd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISODD" },
    { "MULTINOMIAL", "MULTINOMIAL", "com.sun.star.sheet.addin.Analysis.getMultinomial", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMULTINOMIAL" },
    { "SERIESSUM", "SERIESSUM", "com.sun.star.sheet.addin.Analysis.getSeriessum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETSERIESSUM" },
    { "QUOTIENT", "QUOTIENT", "com.sun.star.sheet.addin.Analysis.getQuotient", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETQUOTIENT" },
    { "MROUND", "MROUND", "com.sun.star.sheet.addin.Analysis.getMround", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMROUND" },
    { "SQRTPI", "SQRTPI", "com.sun.star.sheet.addin.Analysis.getSqrtpi", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETSQRTPI" },
    { "RANDBETWEEN", "RANDBETWEEN", "com.sun.star.sheet.addin.Analysis.getRandbetween", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETRANDBETWEEN" },
    { "GCD", "GCD_ADD", "com.sun.star.sheet.addin.Analysis.getGcd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETGCD" },
    { "LCM", "LCM_ADD", "com.sun.star.sheet.addin.Analysis.getLcm", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETLCM" },
    { "BESSELI", "BESSELI", "com.sun.star.sheet.addin.Analysis.getBesseli", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELI" },
    { "BESSELJ", "BESSELJ", "com.sun.star.sheet.addin.Analysis.getBesselj", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELJ" },
    { "BESSELK", "BESSELK", "com.sun.star.sheet.addin.Analysis.getBesselk", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELK" },
    { "BESSELY", "BESSELY", "com.sun.star.sheet.addin.Analysis.getBessely", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELY" },
    { "BIN2OCT", "BIN2OCT", "com.sun.star.sheet.addin.Analysis.getBin2Oct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBIN2OCT" },
    { "BIN2DEC", "BIN2DEC", "com.sun.star.sheet.addin.Analysis.getBin2Dec", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBIN2DEC" },
    { "BIN2HEX", "BIN2HEX", "com.sun.star.sheet.addin.Analysis.getBin2Hex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBIN2HEX" },
    { "OCT2BIN", "OCT2BIN", "com.sun.star.sheet.addin.Analysis.getOct2Bin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETOCT2BIN" },
    { "OCT2DEC", "OCT2DEC", "com.sun.star.sheet.addin.Analysis.getOct2Dec", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETOCT2DEC" },
    { "OCT2HEX", "OCT2HEX", "com.sun.star.sheet.addin.Analysis.getOct2Hex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETOCT2HEX" },
    { "DEC2BIN", "DEC2BIN", "com.sun.star.sheet.addin.Analysis.getDec2Bin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDEC2BIN" },
    { "DEC2OCT", "DEC2OCT", "com.sun.star.sheet.addin.Analysis.getDec2Oct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDEC2OCT" },
    { "DEC2HEX", "DEC2HEX", "com.sun.star.sheet.addin.Analysis.getDec2Hex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDEC2HEX" },
    { "HEX2BIN", "HEX2BIN", "com.sun.star.sheet.addin.Analysis.getHex2Bin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETHEX2BIN" },
    { "HEX2DEC", "HEX2DEC", "com.sun.star.sheet.addin.Analysis.getHex2Dec", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETHEX2DEC" },
    { "HEX2OCT", "HEX2OCT", "com.sun.star.sheet.addin.Analysis.getHex2Oct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETHEX2OCT" },
    { "DELTA", "DELTA", "com.sun.star.sheet.addin.Analysis.getDelta", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDELTA" },
    { "ERF", "ERF", "com.sun.star.sheet.addin.Analysis.getErf", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETERF" },
    { "ERFC", "ERFC", "com.sun.star.sheet.addin.Analysis.getErfc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETERFC" },
    { "GESTEP", "GESTEP", "com.sun.star.sheet.addin.Analysis.getGestep", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETGESTEP" },
    { "FACTDOUBLE", "FACTDOUBLE", "com.sun.star.sheet.addin.Analysis.getFactdouble", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETFACTDOUBLE" },
    { "IMABS", "IMABS", "com.sun.star.sheet.addin.Analysis.getImabs", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMABS" },
    { "IMAGINARY", "IMAGINARY", "com.sun.star.sheet.addin.Analysis.getImaginary", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMAGINARY" },
    { "IMPOWER", "IMPOWER", "com.sun.star.sheet.addin.Analysis.getImpower", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMPOWER" },
    { "IMARGUMENT", "IMARGUMENT", "com.sun.star.sheet.addin.Analysis.getImargument", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMARGUMENT" },
    { "IMCOS", "IMCOS", "com.sun.star.sheet.addin.Analysis.getImcos", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCOS" },
    { "IMDIV", "IMDIV", "com.sun.star.sheet.addin.Analysis.getImdiv", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMDIV" },
    { "IMEXP", "IMEXP", "com.sun.star.sheet.addin.Analysis.getImexp", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMEXP" },
    { "IMCONJUGATE", "IMCONJUGATE", "com.sun.star.sheet.addin.Analysis.getImconjugate", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCONJUGATE" },
    { "IMLN", "IMLN", "com.sun.star.sheet.addin.Analysis.getImln", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMLN" },
    { "IMLOG10", "IMLOG10", "com.sun.star.sheet.addin.Analysis.getImlog10", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMLOG10" },
    { "IMLOG2", "IMLOG2", "com.sun.star.sheet.addin.Analysis.getImlog2", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMLOG2" },
    { "IMPRODUCT", "IMPRODUCT", "com.sun.star.sheet.addin.Analysis.getImproduct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMPRODUCT" },
    { "IMREAL", "IMREAL", "com.sun.star.sheet.addin.Analysis.getImreal", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMREAL" },
    { "IMSIN", "IMSIN", "com.sun.star.sheet.addin.Analysis.getImsin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSIN" },
    { "IMSUB", "IMSUB", "com.sun.star.sheet.addin.Analysis.getImsub", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSUB" },
    { "IMSUM", "IMSUM", "com.sun.star.sheet.addin.Analysis.getImsum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSUM" },
    { "IMSQRT", "IMSQRT", "com.sun.star.sheet.addin.Analysis.getImsqrt", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSQRT" },
    { "IMTAN", "IMTAN", "com.sun.star.sheet.addin.Analysis.getImtan", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMTAN" },
    { "IMSEC", "IMSEC", "com.sun.star.sheet.addin.Analysis.getImsec", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSEC" },
    { "IMCSC", "IMCSC", "com.sun.star.sheet.addin.Analysis.getImcsc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCSC" },
    { "IMCOT", "IMCOT", "com.sun.star.sheet.addin.Analysis.getImcot", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCOT" },
    { "IMSINH", "IMSINH", "com.sun.star.sheet.addin.Analysis.getImsinh", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSINH" },
    { "IMCOSH", "IMCOSH", "com.sun.star.sheet.addin.Analysis.getImcosh", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCOSH" },
    { "IMSECH", "IMSECH", "com.sun.star.sheet.addin.Analysis.getImsech", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSECH" },
    { "IMCSCH", "IMCSCH", "com.sun.star.sheet.addin.Analysis.getImcsch", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCSCH" },
    { "COMPLEX", "COMPLEX", "com.sun.star.sheet.addin.Analysis.getComplex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOMPLEX" },
    { "CONVERT", "CONVERT_ADD", "com.sun.star.sheet.addin.Analysis.getConvert", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCONVERT" },
    { "AMORDEGRC", "AMORDEGRC", "com.sun.star.sheet.addin.Analysis.getAmordegrc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETAMORDEGRC" },
    { "AMORLINC", "AMORLINC", "com.sun.star.sheet.addin.Analysis.getAmorlinc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETAMORLINC" },
    { "ACCRINT", "ACCRINT", "com.sun.star.sheet.addin.Analysis.getAccrint", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETACCRINT" },
    { "ACCRINTM", "ACCRINTM", "com.sun.star.sheet.addin.Analysis.getAccrintm", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETACCRINTM" },
    { "RECEIVED", "RECEIVED", "com.sun.star.sheet.addin.Analysis.getReceived", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETRECEIVED" },
    { "DISC", "DISC", "com.sun.star.sheet.addin.Analysis.getDisc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDISC" },
    { "DURATION", "DURATION_ADD", "com.sun.star.sheet.addin.Analysis.getDuration", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDURATION" },
    { "EFFECT", "EFFECT_ADD", "com.sun.star.sheet.addin.Analysis.getEffect", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEFFECT" },
    { "CUMPRINC", "CUMPRINC_ADD", "com.sun.star.sheet.addin.Analysis.getCumprinc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCUMPRINC" },
    { "CUMIPMT", "CUMIPMT_ADD", "com.sun.star.sheet.addin.Analysis.getCumipmt", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCUMIPMT" },
    { "PRICE", "PRICE", "com.sun.star.sheet.addin.Analysis.getPrice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICE" },
    { "PRICEDISC", "PRICEDISC", "com.sun.star.sheet.addin.Analysis.getPricedisc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICEDISC" },
    { "PRICEMAT", "PRICEMAT", "com.sun.star.sheet.addin.Analysis.getPricemat", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICEMAT" },
    { "MDURATION", "MDURATION", "com.sun.star.sheet.addin.Analysis.getMduration", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMDURATION" },
    { "NOMINAL", "NOMINAL_ADD", "com.sun.star.sheet.addin.Analysis.getNominal", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETNOMINAL" },
    { "DOLLARFR", "DOLLARFR", "com.sun.star.sheet.addin.Analysis.getDollarfr", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDOLLARFR" },
    { "DOLLARDE", "DOLLARDE", "com.sun.star.sheet.addin.Analysis.getDollarde", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDOLLARDE" },
    { "YIELD", "YIELD", "com.sun.star.sheet.addin.Analysis.getYield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELD" },
    { "YIELDDISC", "YIELDDISC", "com.sun.star.sheet.addin.Analysis.getYielddisc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELDDISC" },
    { "YIELDMAT", "YIELDMAT", "com.sun.star.sheet.addin.Analysis.getYieldmat", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELDMAT" },
    { "TBILLEQ", "TBILLEQ", "com.sun.star.sheet.addin.Analysis.getTbilleq", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLEQ" },
    { "TBILLPRICE", "TBILLPRICE", "com.sun.star.sheet.addin.Analysis.getTbillprice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLPRICE" },
    { "TBILLYIELD", "TBILLYIELD", "com.sun.star.sheet.addin.Analysis.getTbillyield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLYIELD" },
    { "ODDFPRICE", "ODDFPRICE", "com.sun.star.sheet.addin.Analysis.getOddfprice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDFPRICE" },
    { "ODDFYIELD", "ODDFYIELD", "com.sun.star.sheet.addin.Analysis.getOddfyield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDFYIELD" },
    { "ODDLPRICE", "ODDLPRICE", "com.sun.star.sheet.addin.Analysis.getOddlprice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDLPRICE" },
    { "ODDLYIELD", "ODDLYIELD", "com.sun.star.sheet.addin.Analysis.getOddlyield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDLYIELD" },
    { "XIRR", "XIRR", "com.sun.star.sheet.addin.Analysis.getXirr", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETXIRR" },
    { "XNPV", "XNPV", "com.sun.star.sheet.addin.Analysis.getXnpv", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETXNPV" },
    { "INTRATE", "INTRATE", "com.sun.star.sheet.addin.Analysis.getIntrate", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETINTRATE" },
    { "COUPNCD", "COUPNCD", "com.sun.star.sheet.addin.Analysis.getCoupncd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPNCD" },
    { "COUPDAYS", "COUPDAYS", "com.sun.star.sheet.addin.Analysis.getCoupdays", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYS" },
    { "COUPDAYSNC", "COUPDAYSNC", "com.sun.star.sheet.addin.Analysis.getCoupdaysnc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYSNC" },
    { "COUPDAYBS", "COUPDAYBS", "com.sun.star.sheet.addin.Analysis.getCoupdaybs", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYBS" },
    { "COUPPCD", "COUPPCD", "com.sun.star.sheet.addin.Analysis.getCouppcd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPPCD" },
    { "COUPNUM", "COUPNUM", "com.sun.star.sheet.addin.Analysis.getCoupnum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPNUM" },
    { "FVSCHEDULE", "FVSCHEDULE", "com.sun.star.sheet.addin.Analysis.getFvschedule", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETFVSCHEDULE" },
};

const ScCompiler::AddInMap* ScCompiler::GetAddInMap()
{
    return maAddInMap;
}

size_t ScCompiler::GetAddInMapCount()
{
    return sizeof(maAddInMap)/sizeof(maAddInMap[0]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
