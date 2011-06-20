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
#include "precompiled_sc.hxx"

#include "compiler.hxx"
#include <sal/macros.h>

// ODFF, English, Programmatical, ODF_11
ScCompiler::AddInMap ScCompiler::maAddInMap[] =
{
    { "ORG.OPENOFFICE.WEEKS", "WEEKS", false, "com.sun.star.sheet.addin.DateFunctions.getDiffWeeks", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFWEEKS" },
    { "ORG.OPENOFFICE.MONTHS", "MONTHS", false, "com.sun.star.sheet.addin.DateFunctions.getDiffMonths", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFMONTHS" },
    { "ORG.OPENOFFICE.YEARS", "YEARS", false, "com.sun.star.sheet.addin.DateFunctions.getDiffYears", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFYEARS" },
    { "ORG.OPENOFFICE.ISLEAPYEAR", "ISLEAPYEAR", false, "com.sun.star.sheet.addin.DateFunctions.getIsLeapYear", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETISLEAPYEAR" },
    { "ORG.OPENOFFICE.DAYSINMONTH", "DAYSINMONTH", false, "com.sun.star.sheet.addin.DateFunctions.getDaysInMonth", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINMONTH" },
    { "ORG.OPENOFFICE.DAYSINYEAR", "DAYSINYEAR", false, "com.sun.star.sheet.addin.DateFunctions.getDaysInYear", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINYEAR" },
    { "ORG.OPENOFFICE.WEEKSINYEAR", "WEEKSINYEAR", false, "com.sun.star.sheet.addin.DateFunctions.getWeeksInYear", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETWEEKSINYEAR" },
    { "ORG.OPENOFFICE.ROT13", "ROT13", false, "com.sun.star.sheet.addin.DateFunctions.getRot13", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETROT13" },
    { "WORKDAY", "WORKDAY", false, "com.sun.star.sheet.addin.Analysis.getWorkday", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWORKDAY" },
    { "YEARFRAC", "YEARFRAC", false, "com.sun.star.sheet.addin.Analysis.getYearfrac", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYEARFRAC" },
    { "EDATE", "EDATE", false, "com.sun.star.sheet.addin.Analysis.getEdate", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEDATE" },
    { "WEEKNUM", "WEEKNUM_ADD", false, "com.sun.star.sheet.addin.Analysis.getWeeknum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWEEKNUM" },
    { "EOMONTH", "EOMONTH", false, "com.sun.star.sheet.addin.Analysis.getEomonth", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEOMONTH" },
    { "NETWORKDAYS", "NETWORKDAYS", false, "com.sun.star.sheet.addin.Analysis.getNetworkdays", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETNETWORKDAYS" },
    { "ISEVEN", "ISEVEN_ADD", true, "com.sun.star.sheet.addin.Analysis.getIseven", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISEVEN" },
    { "ISODD", "ISODD_ADD", true, "com.sun.star.sheet.addin.Analysis.getIsodd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISODD" },
    { "MULTINOMIAL", "MULTINOMIAL", false, "com.sun.star.sheet.addin.Analysis.getMultinomial", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMULTINOMIAL" },
    { "SERIESSUM", "SERIESSUM", false, "com.sun.star.sheet.addin.Analysis.getSeriessum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETSERIESSUM" },
    { "QUOTIENT", "QUOTIENT", false, "com.sun.star.sheet.addin.Analysis.getQuotient", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETQUOTIENT" },
    { "MROUND", "MROUND", false, "com.sun.star.sheet.addin.Analysis.getMround", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMROUND" },
    { "SQRTPI", "SQRTPI", false, "com.sun.star.sheet.addin.Analysis.getSqrtpi", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETSQRTPI" },
    { "RANDBETWEEN", "RANDBETWEEN", false, "com.sun.star.sheet.addin.Analysis.getRandbetween", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETRANDBETWEEN" },
    { "GCD", "GCD_ADD", true, "com.sun.star.sheet.addin.Analysis.getGcd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETGCD" },
    { "LCM", "LCM_ADD", true, "com.sun.star.sheet.addin.Analysis.getLcm", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETLCM" },
    { "BESSELI", "BESSELI", false, "com.sun.star.sheet.addin.Analysis.getBesseli", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELI" },
    { "BESSELJ", "BESSELJ", false, "com.sun.star.sheet.addin.Analysis.getBesselj", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELJ" },
    { "BESSELK", "BESSELK", false, "com.sun.star.sheet.addin.Analysis.getBesselk", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELK" },
    { "BESSELY", "BESSELY", false, "com.sun.star.sheet.addin.Analysis.getBessely", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBESSELY" },
    { "BIN2OCT", "BIN2OCT", false, "com.sun.star.sheet.addin.Analysis.getBin2Oct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBIN2OCT" },
    { "BIN2DEC", "BIN2DEC", false, "com.sun.star.sheet.addin.Analysis.getBin2Dec", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBIN2DEC" },
    { "BIN2HEX", "BIN2HEX", false, "com.sun.star.sheet.addin.Analysis.getBin2Hex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBIN2HEX" },
    { "OCT2BIN", "OCT2BIN", false, "com.sun.star.sheet.addin.Analysis.getOct2Bin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETOCT2BIN" },
    { "OCT2DEC", "OCT2DEC", false, "com.sun.star.sheet.addin.Analysis.getOct2Dec", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETOCT2DEC" },
    { "OCT2HEX", "OCT2HEX", false, "com.sun.star.sheet.addin.Analysis.getOct2Hex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETOCT2HEX" },
    { "DEC2BIN", "DEC2BIN", false, "com.sun.star.sheet.addin.Analysis.getDec2Bin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDEC2BIN" },
    { "DEC2OCT", "DEC2OCT", false, "com.sun.star.sheet.addin.Analysis.getDec2Oct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDEC2OCT" },
    { "DEC2HEX", "DEC2HEX", false, "com.sun.star.sheet.addin.Analysis.getDec2Hex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDEC2HEX" },
    { "HEX2BIN", "HEX2BIN", false, "com.sun.star.sheet.addin.Analysis.getHex2Bin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETHEX2BIN" },
    { "HEX2DEC", "HEX2DEC", false, "com.sun.star.sheet.addin.Analysis.getHex2Dec", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETHEX2DEC" },
    { "HEX2OCT", "HEX2OCT", false, "com.sun.star.sheet.addin.Analysis.getHex2Oct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETHEX2OCT" },
    { "DELTA", "DELTA", false, "com.sun.star.sheet.addin.Analysis.getDelta", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDELTA" },
    { "ERF", "ERF", false, "com.sun.star.sheet.addin.Analysis.getErf", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETERF" },
    { "ERFC", "ERFC", false, "com.sun.star.sheet.addin.Analysis.getErfc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETERFC" },
    { "GESTEP", "GESTEP", false, "com.sun.star.sheet.addin.Analysis.getGestep", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETGESTEP" },
    { "FACTDOUBLE", "FACTDOUBLE", false, "com.sun.star.sheet.addin.Analysis.getFactdouble", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETFACTDOUBLE" },
    { "IMABS", "IMABS", false, "com.sun.star.sheet.addin.Analysis.getImabs", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMABS" },
    { "IMAGINARY", "IMAGINARY", false, "com.sun.star.sheet.addin.Analysis.getImaginary", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMAGINARY" },
    { "IMPOWER", "IMPOWER", false, "com.sun.star.sheet.addin.Analysis.getImpower", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMPOWER" },
    { "IMARGUMENT", "IMARGUMENT", false, "com.sun.star.sheet.addin.Analysis.getImargument", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMARGUMENT" },
    { "IMCOS", "IMCOS", false, "com.sun.star.sheet.addin.Analysis.getImcos", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCOS" },
    { "IMDIV", "IMDIV", false, "com.sun.star.sheet.addin.Analysis.getImdiv", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMDIV" },
    { "IMEXP", "IMEXP", false, "com.sun.star.sheet.addin.Analysis.getImexp", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMEXP" },
    { "IMCONJUGATE", "IMCONJUGATE", false, "com.sun.star.sheet.addin.Analysis.getImconjugate", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMCONJUGATE" },
    { "IMLN", "IMLN", false, "com.sun.star.sheet.addin.Analysis.getImln", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMLN" },
    { "IMLOG10", "IMLOG10", false, "com.sun.star.sheet.addin.Analysis.getImlog10", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMLOG10" },
    { "IMLOG2", "IMLOG2", false, "com.sun.star.sheet.addin.Analysis.getImlog2", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMLOG2" },
    { "IMPRODUCT", "IMPRODUCT", false, "com.sun.star.sheet.addin.Analysis.getImproduct", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMPRODUCT" },
    { "IMREAL", "IMREAL", false, "com.sun.star.sheet.addin.Analysis.getImreal", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMREAL" },
    { "IMSIN", "IMSIN", false, "com.sun.star.sheet.addin.Analysis.getImsin", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSIN" },
    { "IMSUB", "IMSUB", false, "com.sun.star.sheet.addin.Analysis.getImsub", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSUB" },
    { "IMSUM", "IMSUM", false, "com.sun.star.sheet.addin.Analysis.getImsum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSUM" },
    { "IMSQRT", "IMSQRT", false, "com.sun.star.sheet.addin.Analysis.getImsqrt", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETIMSQRT" },
    { "COMPLEX", "COMPLEX", false, "com.sun.star.sheet.addin.Analysis.getComplex", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOMPLEX" },
    { "CONVERT", "CONVERT_ADD", false, "com.sun.star.sheet.addin.Analysis.getConvert", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCONVERT" },
    { "AMORDEGRC", "AMORDEGRC", false, "com.sun.star.sheet.addin.Analysis.getAmordegrc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETAMORDEGRC" },
    { "AMORLINC", "AMORLINC", false, "com.sun.star.sheet.addin.Analysis.getAmorlinc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETAMORLINC" },
    { "ACCRINT", "ACCRINT", false, "com.sun.star.sheet.addin.Analysis.getAccrint", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETACCRINT" },
    { "ACCRINTM", "ACCRINTM", false, "com.sun.star.sheet.addin.Analysis.getAccrintm", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETACCRINTM" },
    { "RECEIVED", "RECEIVED", false, "com.sun.star.sheet.addin.Analysis.getReceived", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETRECEIVED" },
    { "DISC", "DISC", false, "com.sun.star.sheet.addin.Analysis.getDisc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDISC" },
    { "DURATION", "DURATION_ADD", false, "com.sun.star.sheet.addin.Analysis.getDuration", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDURATION" },
    { "EFFECT", "EFFECT_ADD", true, "com.sun.star.sheet.addin.Analysis.getEffect", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEFFECT" },
    { "CUMPRINC", "CUMPRINC_ADD", true, "com.sun.star.sheet.addin.Analysis.getCumprinc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCUMPRINC" },
    { "CUMIPMT", "CUMIPMT_ADD", true, "com.sun.star.sheet.addin.Analysis.getCumipmt", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCUMIPMT" },
    { "PRICE", "PRICE", false, "com.sun.star.sheet.addin.Analysis.getPrice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICE" },
    { "PRICEDISC", "PRICEDISC", false, "com.sun.star.sheet.addin.Analysis.getPricedisc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICEDISC" },
    { "PRICEMAT", "PRICEMAT", false, "com.sun.star.sheet.addin.Analysis.getPricemat", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICEMAT" },
    { "MDURATION", "MDURATION", false, "com.sun.star.sheet.addin.Analysis.getMduration", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMDURATION" },
    { "NOMINAL", "NOMINAL_ADD", true, "com.sun.star.sheet.addin.Analysis.getNominal", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETNOMINAL" },
    { "DOLLARFR", "DOLLARFR", false, "com.sun.star.sheet.addin.Analysis.getDollarfr", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDOLLARFR" },
    { "DOLLARDE", "DOLLARDE", false, "com.sun.star.sheet.addin.Analysis.getDollarde", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDOLLARDE" },
    { "YIELD", "YIELD", false, "com.sun.star.sheet.addin.Analysis.getYield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELD" },
    { "YIELDDISC", "YIELDDISC", false, "com.sun.star.sheet.addin.Analysis.getYielddisc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELDDISC" },
    { "YIELDMAT", "YIELDMAT", false, "com.sun.star.sheet.addin.Analysis.getYieldmat", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELDMAT" },
    { "TBILLEQ", "TBILLEQ", false, "com.sun.star.sheet.addin.Analysis.getTbilleq", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLEQ" },
    { "TBILLPRICE", "TBILLPRICE", false, "com.sun.star.sheet.addin.Analysis.getTbillprice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLPRICE" },
    { "TBILLYIELD", "TBILLYIELD", false, "com.sun.star.sheet.addin.Analysis.getTbillyield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLYIELD" },
    { "ODDFPRICE", "ODDFPRICE", false, "com.sun.star.sheet.addin.Analysis.getOddfprice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDFPRICE" },
    { "ODDFYIELD", "ODDFYIELD", false, "com.sun.star.sheet.addin.Analysis.getOddfyield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDFYIELD" },
    { "ODDLPRICE", "ODDLPRICE", false, "com.sun.star.sheet.addin.Analysis.getOddlprice", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDLPRICE" },
    { "ODDLYIELD", "ODDLYIELD", false, "com.sun.star.sheet.addin.Analysis.getOddlyield", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDLYIELD" },
    { "XIRR", "XIRR", false, "com.sun.star.sheet.addin.Analysis.getXirr", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETXIRR" },
    { "XNPV", "XNPV", false, "com.sun.star.sheet.addin.Analysis.getXnpv", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETXNPV" },
    { "INTRATE", "INTRATE", false, "com.sun.star.sheet.addin.Analysis.getIntrate", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETINTRATE" },
    { "COUPNCD", "COUPNCD", false, "com.sun.star.sheet.addin.Analysis.getCoupncd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPNCD" },
    { "COUPDAYS", "COUPDAYS", false, "com.sun.star.sheet.addin.Analysis.getCoupdays", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYS" },
    { "COUPDAYSNC", "COUPDAYSNC", false, "com.sun.star.sheet.addin.Analysis.getCoupdaysnc", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYSNC" },
    { "COUPDAYBS", "COUPDAYBS", false, "com.sun.star.sheet.addin.Analysis.getCoupdaybs", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYBS" },
    { "COUPPCD", "COUPPCD", false, "com.sun.star.sheet.addin.Analysis.getCouppcd", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPPCD" },
    { "COUPNUM", "COUPNUM", false, "com.sun.star.sheet.addin.Analysis.getCoupnum", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPNUM" },
    { "FVSCHEDULE", "FVSCHEDULE", false, "com.sun.star.sheet.addin.Analysis.getFvschedule", "COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETFVSCHEDULE" },
};

const ScCompiler::AddInMap* ScCompiler::GetAddInMap()
{
    return maAddInMap;
}

size_t ScCompiler::GetAddInMapCount()
{
    return SAL_N_ELEMENTS(maAddInMap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
