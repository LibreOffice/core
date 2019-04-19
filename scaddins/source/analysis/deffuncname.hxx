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

#pragma once

static const char* ANALYSIS_DEFFUNCNAME_Workday[2] =
{
    reinterpret_cast<char const *>(u8"ARBEITSTAG"),
    reinterpret_cast<char const *>(u8"WORKDAY")
};

static const char* ANALYSIS_DEFFUNCNAME_Yearfrac[2] =
{
    reinterpret_cast<char const *>(u8"BRTEILJAHRE"),
    reinterpret_cast<char const *>(u8"YEARFRAC")
};

static const char* ANALYSIS_DEFFUNCNAME_Edate[2] =
{
    reinterpret_cast<char const *>(u8"EDATUM"),
    reinterpret_cast<char const *>(u8"EDATE")
};

static const char* ANALYSIS_DEFFUNCNAME_Weeknum[2] =
{
    reinterpret_cast<char const *>(u8"KALENDERWOCHE"),
    reinterpret_cast<char const *>(u8"WEEKNUM")
};

static const char* ANALYSIS_DEFFUNCNAME_Eomonth[2] =
{
    reinterpret_cast<char const *>(u8"MONATSENDE"),
    reinterpret_cast<char const *>(u8"EOMONTH")
};

static const char* ANALYSIS_DEFFUNCNAME_Networkdays[2] =
{
    reinterpret_cast<char const *>(u8"NETTOARBEITSTAGE"),
    reinterpret_cast<char const *>(u8"NETWORKDAYS")
};

static const char* ANALYSIS_DEFFUNCNAME_Amordegrc[2] =
{
    reinterpret_cast<char const *>(u8"AMORDEGRK"),
    reinterpret_cast<char const *>(u8"AMORDEGRC")
};

static const char* ANALYSIS_DEFFUNCNAME_Amorlinc[2] =
{
    reinterpret_cast<char const *>(u8"AMORLINEARK"),
    reinterpret_cast<char const *>(u8"AMORLINC")
};

static const char* ANALYSIS_DEFFUNCNAME_Accrint[2] =
{
    reinterpret_cast<char const *>(u8"AUFGELZINS"),
    reinterpret_cast<char const *>(u8"ACCRINT")
};

static const char* ANALYSIS_DEFFUNCNAME_Accrintm[2] =
{
    reinterpret_cast<char const *>(u8"AUFGELZINSF"),
    reinterpret_cast<char const *>(u8"ACCRINTM")
};

static const char* ANALYSIS_DEFFUNCNAME_Received[2] =
{
    reinterpret_cast<char const *>(u8"AUSZAHLUNG"),
    reinterpret_cast<char const *>(u8"RECEIVED")
};

static const char* ANALYSIS_DEFFUNCNAME_Disc[2] =
{
    reinterpret_cast<char const *>(u8"DISAGIO"),
    reinterpret_cast<char const *>(u8"DISC")
};

static const char* ANALYSIS_DEFFUNCNAME_Duration[2] =
{
    reinterpret_cast<char const *>(u8"DURATION"),
    reinterpret_cast<char const *>(u8"DURATION")
};

static const char* ANALYSIS_DEFFUNCNAME_Effect[2] =
{
    reinterpret_cast<char const *>(u8"EFFEKTIV"),
    reinterpret_cast<char const *>(u8"EFFECT")
};

static const char* ANALYSIS_DEFFUNCNAME_Cumprinc[2] =
{
    reinterpret_cast<char const *>(u8"KUMKAPITAL"),
    reinterpret_cast<char const *>(u8"CUMPRINC")
};

static const char* ANALYSIS_DEFFUNCNAME_Cumipmt[2] =
{
    reinterpret_cast<char const *>(u8"KUMZINSZ"),
    reinterpret_cast<char const *>(u8"CUMIPMT")
};

static const char* ANALYSIS_DEFFUNCNAME_Price[2] =
{
    reinterpret_cast<char const *>(u8"KURS"),
    reinterpret_cast<char const *>(u8"PRICE")
};

static const char* ANALYSIS_DEFFUNCNAME_Pricedisc[2] =
{
    reinterpret_cast<char const *>(u8"KURSDISAGIO"),
    reinterpret_cast<char const *>(u8"PRICEDISC")
};

static const char* ANALYSIS_DEFFUNCNAME_Pricemat[2] =
{
    reinterpret_cast<char const *>(u8"KURSFÄLLIG"),
    reinterpret_cast<char const *>(u8"PRICEMAT")
};

static const char* ANALYSIS_DEFFUNCNAME_Mduration[2] =
{
    reinterpret_cast<char const *>(u8"MDURATION"),
    reinterpret_cast<char const *>(u8"MDURATION")
};

static const char* ANALYSIS_DEFFUNCNAME_Nominal[2] =
{
    reinterpret_cast<char const *>(u8"NOMINAL"),
    reinterpret_cast<char const *>(u8"NOMINAL")
};

static const char* ANALYSIS_DEFFUNCNAME_Dollarfr[2] =
{
    reinterpret_cast<char const *>(u8"NOTIERUNGBRU"),
    reinterpret_cast<char const *>(u8"DOLLARFR")
};

static const char* ANALYSIS_DEFFUNCNAME_Dollarde[2] =
{
    reinterpret_cast<char const *>(u8"NOTIERUNGDEZ"),
    reinterpret_cast<char const *>(u8"DOLLARDE")
};

static const char* ANALYSIS_DEFFUNCNAME_Yield[2] =
{
    reinterpret_cast<char const *>(u8"RENDITE"),
    reinterpret_cast<char const *>(u8"YIELD")
};

static const char* ANALYSIS_DEFFUNCNAME_Yielddisc[2] =
{
    reinterpret_cast<char const *>(u8"RENDITEDIS"),
    reinterpret_cast<char const *>(u8"YIELDDISC")
};

static const char* ANALYSIS_DEFFUNCNAME_Yieldmat[2] =
{
    reinterpret_cast<char const *>(u8"RENDITEFÄLL"),
    reinterpret_cast<char const *>(u8"YIELDMAT")
};

static const char* ANALYSIS_DEFFUNCNAME_Tbilleq[2] =
{
    reinterpret_cast<char const *>(u8"TBILLÄQUIV"),
    reinterpret_cast<char const *>(u8"TBILLEQ")
};

static const char* ANALYSIS_DEFFUNCNAME_Tbillprice[2] =
{
    reinterpret_cast<char const *>(u8"TBILLKURS"),
    reinterpret_cast<char const *>(u8"TBILLPRICE")
};

static const char* ANALYSIS_DEFFUNCNAME_Tbillyield[2] =
{
    reinterpret_cast<char const *>(u8"TBILLRENDITE"),
    reinterpret_cast<char const *>(u8"TBILLYIELD")
};

static const char* ANALYSIS_DEFFUNCNAME_Oddfprice[2] =
{
    reinterpret_cast<char const *>(u8"UNREGER.KURS"),
    reinterpret_cast<char const *>(u8"ODDFPRICE")
};

static const char* ANALYSIS_DEFFUNCNAME_Oddfyield[2] =
{
    reinterpret_cast<char const *>(u8"UNREGER.REND"),
    reinterpret_cast<char const *>(u8"ODDFYIELD")
};

static const char* ANALYSIS_DEFFUNCNAME_Oddlprice[2] =
{
    reinterpret_cast<char const *>(u8"UNREGLE.KURS"),
    reinterpret_cast<char const *>(u8"ODDLPRICE")
};

static const char* ANALYSIS_DEFFUNCNAME_Oddlyield[2] =
{
    reinterpret_cast<char const *>(u8"UNREGLE.REND"),
    reinterpret_cast<char const *>(u8"ODDLYIELD")
};

static const char* ANALYSIS_DEFFUNCNAME_Xirr[2] =
{
    reinterpret_cast<char const *>(u8"XINTZINSFUSS"),
    reinterpret_cast<char const *>(u8"XIRR")
};

static const char* ANALYSIS_DEFFUNCNAME_Xnpv[2] =
{
    reinterpret_cast<char const *>(u8"XKAPITALWERT"),
    reinterpret_cast<char const *>(u8"XNPV")
};

static const char* ANALYSIS_DEFFUNCNAME_Intrate[2] =
{
    reinterpret_cast<char const *>(u8"ZINSSATZ"),
    reinterpret_cast<char const *>(u8"INTRATE")
};

static const char* ANALYSIS_DEFFUNCNAME_Coupncd[2] =
{
    reinterpret_cast<char const *>(u8"ZINSTERMNZ"),
    reinterpret_cast<char const *>(u8"COUPNCD")
};

static const char* ANALYSIS_DEFFUNCNAME_Coupdays[2] =
{
    reinterpret_cast<char const *>(u8"ZINSTERMTAGE"),
    reinterpret_cast<char const *>(u8"COUPDAYS")
};

static const char* ANALYSIS_DEFFUNCNAME_Coupdaysnc[2] =
{
    reinterpret_cast<char const *>(u8"ZINSTERMTAGNZ"),
    reinterpret_cast<char const *>(u8"COUPDAYSNC")
};

static const char* ANALYSIS_DEFFUNCNAME_Coupdaybs[2] =
{
    reinterpret_cast<char const *>(u8"ZINSTERMTAGVA"),
    reinterpret_cast<char const *>(u8"COUPDAYBS")
};

static const char* ANALYSIS_DEFFUNCNAME_Couppcd[2] =
{
    reinterpret_cast<char const *>(u8"ZINSTERMVZ"),
    reinterpret_cast<char const *>(u8"COUPPCD")
};

static const char* ANALYSIS_DEFFUNCNAME_Coupnum[2] =
{
    reinterpret_cast<char const *>(u8"ZINSTERMZAHL"),
    reinterpret_cast<char const *>(u8"COUPNUM")
};

static const char* ANALYSIS_DEFFUNCNAME_Fvschedule[2] =
{
    reinterpret_cast<char const *>(u8"ZW2"),
    reinterpret_cast<char const *>(u8"FVSCHEDULE")
};

static const char* ANALYSIS_DEFFUNCNAME_Iseven[2] =
{
    reinterpret_cast<char const *>(u8"ISTGERADE"),
    reinterpret_cast<char const *>(u8"ISEVEN")
};

static const char* ANALYSIS_DEFFUNCNAME_Isodd[2] =
{
    reinterpret_cast<char const *>(u8"ISTUNGERADE"),
    reinterpret_cast<char const *>(u8"ISODD")
};

static const char* ANALYSIS_DEFFUNCNAME_Gcd[2] =
{
    reinterpret_cast<char const *>(u8"GGT"),
    reinterpret_cast<char const *>(u8"GCD")
};

static const char* ANALYSIS_DEFFUNCNAME_Lcm[2] =
{
    reinterpret_cast<char const *>(u8"KGV"),
    reinterpret_cast<char const *>(u8"LCM")
};

static const char* ANALYSIS_DEFFUNCNAME_Multinomial[2] =
{
    reinterpret_cast<char const *>(u8"POLYNOMIAL"),
    reinterpret_cast<char const *>(u8"MULTINOMIAL")
};

static const char* ANALYSIS_DEFFUNCNAME_Seriessum[2] =
{
    reinterpret_cast<char const *>(u8"POTENZREIHE"),
    reinterpret_cast<char const *>(u8"SERIESSUM")
};

static const char* ANALYSIS_DEFFUNCNAME_Quotient[2] =
{
    reinterpret_cast<char const *>(u8"QUOTIENT"),
    reinterpret_cast<char const *>(u8"QUOTIENT")
};

static const char* ANALYSIS_DEFFUNCNAME_Mround[2] =
{
    reinterpret_cast<char const *>(u8"VRUNDEN"),
    reinterpret_cast<char const *>(u8"MROUND")
};

static const char* ANALYSIS_DEFFUNCNAME_Sqrtpi[2] =
{
    reinterpret_cast<char const *>(u8"WURZELPI"),
    reinterpret_cast<char const *>(u8"SQRTPI")
};

static const char* ANALYSIS_DEFFUNCNAME_Randbetween[2] =
{
    reinterpret_cast<char const *>(u8"ZUFALLSBEREICH"),
    reinterpret_cast<char const *>(u8"RANDBETWEEN")
};

static const char* ANALYSIS_DEFFUNCNAME_Besseli[2] =
{
    reinterpret_cast<char const *>(u8"BESSELI"),
    reinterpret_cast<char const *>(u8"BESSELI")
};

static const char* ANALYSIS_DEFFUNCNAME_Besselj[2] =
{
    reinterpret_cast<char const *>(u8"BESSELJ"),
    reinterpret_cast<char const *>(u8"BESSELJ")
};

static const char* ANALYSIS_DEFFUNCNAME_Besselk[2] =
{
    reinterpret_cast<char const *>(u8"BESSELK"),
    reinterpret_cast<char const *>(u8"BESSELK")
};

static const char* ANALYSIS_DEFFUNCNAME_Bessely[2] =
{
    reinterpret_cast<char const *>(u8"BESSELY"),
    reinterpret_cast<char const *>(u8"BESSELY")
};

static const char* ANALYSIS_DEFFUNCNAME_Bin2Dec[2] =
{
    reinterpret_cast<char const *>(u8"BININDEZ"),
    reinterpret_cast<char const *>(u8"BIN2DEC")
};

static const char* ANALYSIS_DEFFUNCNAME_Bin2Hex[2] =
{
    reinterpret_cast<char const *>(u8"BININHEX"),
    reinterpret_cast<char const *>(u8"BIN2HEX")
};

static const char* ANALYSIS_DEFFUNCNAME_Bin2Oct[2] =
{
    reinterpret_cast<char const *>(u8"BININOKT"),
    reinterpret_cast<char const *>(u8"BIN2OCT")
};

static const char* ANALYSIS_DEFFUNCNAME_Delta[2] =
{
    reinterpret_cast<char const *>(u8"DELTA"),
    reinterpret_cast<char const *>(u8"DELTA")
};

static const char* ANALYSIS_DEFFUNCNAME_Dec2Bin[2] =
{
    reinterpret_cast<char const *>(u8"DEZINBIN"),
    reinterpret_cast<char const *>(u8"DEC2BIN")
};

static const char* ANALYSIS_DEFFUNCNAME_Dec2Hex[2] =
{
    reinterpret_cast<char const *>(u8"DEZINHEX"),
    reinterpret_cast<char const *>(u8"DEC2HEX")
};

static const char* ANALYSIS_DEFFUNCNAME_Dec2Oct[2] =
{
    reinterpret_cast<char const *>(u8"DEZINOKT"),
    reinterpret_cast<char const *>(u8"DEC2OCT")
};

static const char* ANALYSIS_DEFFUNCNAME_Erf[2] =
{
    reinterpret_cast<char const *>(u8"GAUSSFEHLER"),
    reinterpret_cast<char const *>(u8"ERF")
};

static const char* ANALYSIS_DEFFUNCNAME_Erfc[2] =
{
    reinterpret_cast<char const *>(u8"GAUSSFKOMPL"),
    reinterpret_cast<char const *>(u8"ERFC")
};

static const char* ANALYSIS_DEFFUNCNAME_Gestep[2] =
{
    reinterpret_cast<char const *>(u8"GGANZZAHL"),
    reinterpret_cast<char const *>(u8"GESTEP")
};

static const char* ANALYSIS_DEFFUNCNAME_Hex2Bin[2] =
{
    reinterpret_cast<char const *>(u8"HEXINBIN"),
    reinterpret_cast<char const *>(u8"HEX2BIN")
};

static const char* ANALYSIS_DEFFUNCNAME_Hex2Dec[2] =
{
    reinterpret_cast<char const *>(u8"HEXINDEZ"),
    reinterpret_cast<char const *>(u8"HEX2DEC")
};

static const char* ANALYSIS_DEFFUNCNAME_Hex2Oct[2] =
{
    reinterpret_cast<char const *>(u8"HEXINOKT"),
    reinterpret_cast<char const *>(u8"HEX2OCT")
};

static const char* ANALYSIS_DEFFUNCNAME_Imabs[2] =
{
    reinterpret_cast<char const *>(u8"IMABS"),
    reinterpret_cast<char const *>(u8"IMABS")
};

static const char* ANALYSIS_DEFFUNCNAME_Imaginary[2] =
{
    reinterpret_cast<char const *>(u8"IMAGINÄRTEIL"),
    reinterpret_cast<char const *>(u8"IMAGINARY")
};

static const char* ANALYSIS_DEFFUNCNAME_Impower[2] =
{
    reinterpret_cast<char const *>(u8"IMAPOTENZ"),
    reinterpret_cast<char const *>(u8"IMPOWER")
};

static const char* ANALYSIS_DEFFUNCNAME_Imargument[2] =
{
    reinterpret_cast<char const *>(u8"IMARGUMENT"),
    reinterpret_cast<char const *>(u8"IMARGUMENT")
};

static const char* ANALYSIS_DEFFUNCNAME_Imcos[2] =
{
    reinterpret_cast<char const *>(u8"IMCOS"),
    reinterpret_cast<char const *>(u8"IMCOS")
};

static const char* ANALYSIS_DEFFUNCNAME_Imdiv[2] =
{
    reinterpret_cast<char const *>(u8"IMDIV"),
    reinterpret_cast<char const *>(u8"IMDIV")
};

static const char* ANALYSIS_DEFFUNCNAME_Imexp[2] =
{
    reinterpret_cast<char const *>(u8"IMEXP"),
    reinterpret_cast<char const *>(u8"IMEXP")
};

static const char* ANALYSIS_DEFFUNCNAME_Imconjugate[2] =
{
    reinterpret_cast<char const *>(u8"IMKONJUGIERTE"),
    reinterpret_cast<char const *>(u8"IMCONJUGATE")
};

static const char* ANALYSIS_DEFFUNCNAME_Imln[2] =
{
    reinterpret_cast<char const *>(u8"IMLN"),
    reinterpret_cast<char const *>(u8"IMLN")
};

static const char* ANALYSIS_DEFFUNCNAME_Imlog10[2] =
{
    reinterpret_cast<char const *>(u8"IMLOG10"),
    reinterpret_cast<char const *>(u8"IMLOG10")
};

static const char* ANALYSIS_DEFFUNCNAME_Imlog2[2] =
{
    reinterpret_cast<char const *>(u8"IMLOG2"),
    reinterpret_cast<char const *>(u8"IMLOG2")
};

static const char* ANALYSIS_DEFFUNCNAME_Improduct[2] =
{
    reinterpret_cast<char const *>(u8"IMPRODUKT"),
    reinterpret_cast<char const *>(u8"IMPRODUCT")
};

static const char* ANALYSIS_DEFFUNCNAME_Imreal[2] =
{
    reinterpret_cast<char const *>(u8"IMREALTEIL"),
    reinterpret_cast<char const *>(u8"IMREAL")
};

static const char* ANALYSIS_DEFFUNCNAME_Imsin[2] =
{
    reinterpret_cast<char const *>(u8"IMSIN"),
    reinterpret_cast<char const *>(u8"IMSIN")
};

static const char* ANALYSIS_DEFFUNCNAME_Imsub[2] =
{
    reinterpret_cast<char const *>(u8"IMSUB"),
    reinterpret_cast<char const *>(u8"IMSUB")
};

static const char* ANALYSIS_DEFFUNCNAME_Imsum[2] =
{
    reinterpret_cast<char const *>(u8"IMSUMME"),
    reinterpret_cast<char const *>(u8"IMSUM")
};

static const char* ANALYSIS_DEFFUNCNAME_Imsqrt[2] =
{
    reinterpret_cast<char const *>(u8"IMWURZEL"),
    reinterpret_cast<char const *>(u8"IMSQRT")
};

static const char* ANALYSIS_DEFFUNCNAME_Imtan[2] =
{
    reinterpret_cast<char const *>(u8"IMTAN"),
    reinterpret_cast<char const *>(u8"IMTAN")
};

static const char* ANALYSIS_DEFFUNCNAME_Imsec[2] =
{
    reinterpret_cast<char const *>(u8"IMSEC"),
    reinterpret_cast<char const *>(u8"IMSEC")
};

static const char* ANALYSIS_DEFFUNCNAME_Imcsc[2] =
{
    reinterpret_cast<char const *>(u8"IMCSC"),
    reinterpret_cast<char const *>(u8"IMCSC")
};

static const char* ANALYSIS_DEFFUNCNAME_Imcot[2] =
{
    reinterpret_cast<char const *>(u8"IMCOT"),
    reinterpret_cast<char const *>(u8"IMCOT")
};

static const char* ANALYSIS_DEFFUNCNAME_Imsinh[2] =
{
    reinterpret_cast<char const *>(u8"IMSINH"),
    reinterpret_cast<char const *>(u8"IMSINH")
};

static const char* ANALYSIS_DEFFUNCNAME_Imcosh[2] =
{
    reinterpret_cast<char const *>(u8"IMCOSH"),
    reinterpret_cast<char const *>(u8"IMCOSH")
};

static const char* ANALYSIS_DEFFUNCNAME_Imsech[2] =
{
    reinterpret_cast<char const *>(u8"IMSECH"),
    reinterpret_cast<char const *>(u8"IMSECH")
};

static const char* ANALYSIS_DEFFUNCNAME_Imcsch[2] =
{
    reinterpret_cast<char const *>(u8"IMCSCH"),
    reinterpret_cast<char const *>(u8"IMCSCH")
};

static const char* ANALYSIS_DEFFUNCNAME_Complex[2] =
{
    reinterpret_cast<char const *>(u8"KOMPLEXE"),
    reinterpret_cast<char const *>(u8"COMPLEX")
};

static const char* ANALYSIS_DEFFUNCNAME_Oct2Bin[2] =
{
    reinterpret_cast<char const *>(u8"OKTINBIN"),
    reinterpret_cast<char const *>(u8"OCT2BIN")
};

static const char* ANALYSIS_DEFFUNCNAME_Oct2Dec[2] =
{
    reinterpret_cast<char const *>(u8"OKTINDEZ"),
    reinterpret_cast<char const *>(u8"OCT2DEC")
};

static const char* ANALYSIS_DEFFUNCNAME_Oct2Hex[2] =
{
    reinterpret_cast<char const *>(u8"OKTINHEX"),
    reinterpret_cast<char const *>(u8"OCT2HEX")
};

static const char* ANALYSIS_DEFFUNCNAME_Convert[2] =
{
    reinterpret_cast<char const *>(u8"UMWANDELN"),
    reinterpret_cast<char const *>(u8"CONVERT")
};

static const char* ANALYSIS_DEFFUNCNAME_Factdouble[2] =
{
    reinterpret_cast<char const *>(u8"ZWEIFAKULTÄT"),
    reinterpret_cast<char const *>(u8"FACTDOUBLE")
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
