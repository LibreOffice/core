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

#include <xlformula.hxx>

#include <refdata.hxx>
#include <tokenarray.hxx>
#include <xestream.hxx>
#include <xistream.hxx>
#include <xlroot.hxx>

#include <comphelper/string.hxx>
#include <svl/sharedstringpool.hxx>

using namespace ::formula;

// Function data ==============================================================

OUString XclFunctionInfo::GetMacroFuncName() const
{
    if( IsMacroFunc() )
        return OUString( mpcMacroName, strlen(mpcMacroName), RTL_TEXTENCODING_UTF8 );
    return OUString();
}

OUString XclFunctionInfo::GetAddInEquivalentFuncName() const
{
    if( IsAddInEquivalent() )
        return OUString( mpcMacroName, strlen(mpcMacroName), RTL_TEXTENCODING_UTF8 );
    return OUString();
}

// abbreviations for function return token class
const sal_uInt8 R = EXC_TOKCLASS_REF;
const sal_uInt8 V = EXC_TOKCLASS_VAL;
const sal_uInt8 A = EXC_TOKCLASS_ARR;

// abbreviations for parameter infos
#define RO   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_ORG, false }
#define RA   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_ARR, false }
#define RR   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_RPT, false }
#define RX   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_RPX, false }
#define VO   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_ORG, true  }
#define VV   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_VAL, true  }
#define VA   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_ARR, true  }
#define VR   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_RPT, true  }
#define VX   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_RPX, true  }
#define RO_E { EXC_PARAM_EXCELONLY, EXC_PARAMCONV_ORG, false }
#define VR_E { EXC_PARAM_EXCELONLY, EXC_PARAMCONV_RPT, true  }
#define C    { EXC_PARAM_CALCONLY,  EXC_PARAMCONV_ORG, false }

const sal_uInt16 NOID = SAL_MAX_UINT16;     /// No BIFF/OOBIN function identifier available.
const sal_uInt8 MX    = 30;                 /// Maximum parameter count.

#define EXC_FUNCNAME( ascii )       "_xlfn." ascii
#define EXC_FUNCNAME_ODF( ascii )   "_xlfnodf." ascii
#define EXC_FUNCNAME_ADDIN( ascii )   "com.sun.star.sheet.addin." ascii

/** Functions new in BIFF2. */
static const XclFunctionInfo saFuncTable_2[] =
{
    { ocCount,              0,      0,  MX, V, { RX }, 0, nullptr },
    { ocIf,                 1,      2,  3,  R, { VO, RO }, 0, nullptr },
    { ocIsNA,               2,      1,  1,  V, { VR }, 0, nullptr },
    { ocIsError,            3,      1,  1,  V, { VR }, 0, nullptr },
    { ocSum,                4,      0,  MX, V, { RX }, 0, nullptr },
    { ocAverage,            5,      1,  MX, V, { RX }, 0, nullptr },
    { ocMin,                6,      1,  MX, V, { RX }, 0, nullptr },
    { ocMax,                7,      1,  MX, V, { RX }, 0, nullptr },
    { ocRow,                8,      0,  1,  V, { RO }, 0, nullptr },
    { ocColumn,             9,      0,  1,  V, { RO }, 0, nullptr },
    { ocNotAvail,           10,     0,  0,  V, {}, 0, nullptr },
    { ocNPV,                11,     2,  MX, V, { VR, RX }, 0, nullptr },
    { ocStDev,              12,     1,  MX, V, { RX }, 0, nullptr },
    { ocCurrency,           13,     1,  2,  V, { VR }, 0, nullptr },
    { ocFixed,              14,     1,  2,  V, { VR, VR, C }, 0, nullptr },
    { ocSin,                15,     1,  1,  V, { VR }, 0, nullptr },
    { ocCosecant,           15,     1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocCos,                16,     1,  1,  V, { VR }, 0, nullptr },
    { ocSecant,             16,     1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocTan,                17,     1,  1,  V, { VR }, 0, nullptr },
    { ocCot,                17,     1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocArcTan,             18,     1,  1,  V, { VR }, 0, nullptr },
    { ocArcCot,             18,     1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocPi,                 19,     0,  0,  V, {}, 0, nullptr },
    { ocSqrt,               20,     1,  1,  V, { VR }, 0, nullptr },
    { ocExp,                21,     1,  1,  V, { VR }, 0, nullptr },
    { ocLn,                 22,     1,  1,  V, { VR }, 0, nullptr },
    { ocLog10,              23,     1,  1,  V, { VR }, 0, nullptr },
    { ocAbs,                24,     1,  1,  V, { VR }, 0, nullptr },
    { ocInt,                25,     1,  1,  V, { VR }, 0, nullptr },
    { ocPlusMinus,          26,     1,  1,  V, { VR }, 0, nullptr },
    { ocRound,              27,     2,  2,  V, { VR }, 0, nullptr },
    { ocLookup,             28,     2,  3,  V, { VR, RA }, 0, nullptr },
    { ocIndex,              29,     2,  4,  R, { RA, VV }, 0, nullptr },
    { ocRept,               30,     2,  2,  V, { VR }, 0, nullptr },
    { ocMid,                31,     3,  3,  V, { VR }, 0, nullptr },
    { ocLen,                32,     1,  1,  V, { VR }, 0, nullptr },
    { ocValue,              33,     1,  1,  V, { VR }, 0, nullptr },
    { ocTrue,               34,     0,  0,  V, {}, 0, nullptr },
    { ocFalse,              35,     0,  0,  V, {}, 0, nullptr },
    { ocAnd,                36,     1,  MX, V, { RX }, 0, nullptr },
    { ocOr,                 37,     1,  MX, V, { RX }, 0, nullptr },
    { ocNot,                38,     1,  1,  V, { VR }, 0, nullptr },
    { ocMod,                39,     2,  2,  V, { VR }, 0, nullptr },
    { ocDBCount,            40,     3,  3,  V, { RO, RR }, 0, nullptr },
    { ocDBSum,              41,     3,  3,  V, { RO, RR }, 0, nullptr },
    { ocDBAverage,          42,     3,  3,  V, { RO, RR }, 0, nullptr },
    { ocDBMin,              43,     3,  3,  V, { RO, RR }, 0, nullptr },
    { ocDBMax,              44,     3,  3,  V, { RO, RR }, 0, nullptr },
    { ocDBStdDev,           45,     3,  3,  V, { RO, RR }, 0, nullptr },
    { ocVar,                46,     1,  MX, V, { RX }, 0, nullptr },
    { ocDBVar,              47,     3,  3,  V, { RO, RR }, 0, nullptr },
    { ocText,               48,     2,  2,  V, { VR }, 0, nullptr },
    { ocLinest,             49,     1,  2,  A, { RA, RA, C, C }, 0, nullptr },
    { ocTrend,              50,     1,  3,  A, { RA, RA, RA, C }, 0, nullptr },
    { ocLogest,             51,     1,  2,  A, { RA, RA, C, C }, 0, nullptr },
    { ocGrowth,             52,     1,  3,  A, { RA, RA, RA, C }, 0, nullptr },
    { ocPV,                 56,     3,  5,  V, { VR }, 0, nullptr },
    { ocFV,                 57,     3,  5,  V, { VR }, 0, nullptr },
    { ocNper,               58,     3,  5,  V, { VR }, 0, nullptr },
    { ocPMT,                59,     3,  5,  V, { VR }, 0, nullptr },
    { ocRate,               60,     3,  6,  V, { VR }, 0, nullptr },
    { ocMIRR,               61,     3,  3,  V, { RA, VR }, 0, nullptr },
    { ocIRR,                62,     1,  2,  V, { RA, VR }, 0, nullptr },
    { ocRandom,             63,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, nullptr },
    { ocMatch,              64,     2,  3,  V, { VR, RX, RR }, 0, nullptr },
    { ocGetDate,            65,     3,  3,  V, { VR }, 0, nullptr },
    { ocGetTime,            66,     3,  3,  V, { VR }, 0, nullptr },
    { ocGetDay,             67,     1,  1,  V, { VR }, 0, nullptr },
    { ocGetMonth,           68,     1,  1,  V, { VR }, 0, nullptr },
    { ocGetYear,            69,     1,  1,  V, { VR }, 0, nullptr },
    { ocGetDayOfWeek,       70,     1,  1,  V, { VR, C }, 0, nullptr },
    { ocGetHour,            71,     1,  1,  V, { VR }, 0, nullptr },
    { ocGetMin,             72,     1,  1,  V, { VR }, 0, nullptr },
    { ocGetSec,             73,     1,  1,  V, { VR }, 0, nullptr },
    { ocGetActTime,         74,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, nullptr },
    { ocAreas,              75,     1,  1,  V, { RO }, 0, nullptr },
    { ocRows,               76,     1,  1,  V, { RO }, 0, nullptr },
    { ocColumns,            77,     1,  1,  V, { RO }, 0, nullptr },
    { ocOffset,             78,     3,  5,  R, { RO, VR }, EXC_FUNCFLAG_VOLATILE, nullptr },
    { ocSearch,             82,     2,  3,  V, { VR }, 0, nullptr },
    { ocMatTrans,           83,     1,  1,  A, { VO }, 0, nullptr },
    { ocType,               86,     1,  1,  V, { VX }, 0, nullptr },
    { ocArcTan2,            97,     2,  2,  V, { VR }, 0, nullptr },
    { ocArcSin,             98,     1,  1,  V, { VR }, 0, nullptr },
    { ocArcCos,             99,     1,  1,  V, { VR }, 0, nullptr },
    { ocChoose,             100,    2,  MX, R, { VO, RO }, 0, nullptr },
    { ocHLookup,            101,    3,  3,  V, { VV, RO, RO, C }, 0, nullptr },
    { ocVLookup,            102,    3,  3,  V, { VV, RO, RO, C }, 0, nullptr },
    { ocIsRef,              105,    1,  1,  V, { RX }, 0, nullptr },
    { ocLog,                109,    1,  2,  V, { VR }, 0, nullptr },
    { ocChar,               111,    1,  1,  V, { VR }, 0, nullptr },
    { ocLower,              112,    1,  1,  V, { VR }, 0, nullptr },
    { ocUpper,              113,    1,  1,  V, { VR }, 0, nullptr },
    { ocProper,             114,    1,  1,  V, { VR }, 0, nullptr },
    { ocLeft,               115,    1,  2,  V, { VR }, 0, nullptr },
    { ocRight,              116,    1,  2,  V, { VR }, 0, nullptr },
    { ocExact,              117,    2,  2,  V, { VR }, 0, nullptr },
    { ocTrim,               118,    1,  1,  V, { VR }, 0, nullptr },
    { ocReplace,            119,    4,  4,  V, { VR }, 0, nullptr },
    { ocSubstitute,         120,    3,  4,  V, { VR }, 0, nullptr },
    { ocCode,               121,    1,  1,  V, { VR }, 0, nullptr },
    { ocFind,               124,    2,  3,  V, { VR }, 0, nullptr },
    { ocCell,               125,    1,  2,  V, { VV, RO }, EXC_FUNCFLAG_VOLATILE, nullptr },
    { ocIsErr,              126,    1,  1,  V, { VR }, 0, nullptr },
    { ocIsString,           127,    1,  1,  V, { VR }, 0, nullptr },
    { ocIsValue,            128,    1,  1,  V, { VR }, 0, nullptr },
    { ocIsEmpty,            129,    1,  1,  V, { VR }, 0, nullptr },
    { ocT,                  130,    1,  1,  V, { RO }, 0, nullptr },
    { ocN,                  131,    1,  1,  V, { RO }, 0, nullptr },
    { ocGetDateValue,       140,    1,  1,  V, { VR }, 0, nullptr },
    { ocGetTimeValue,       141,    1,  1,  V, { VR }, 0, nullptr },
    { ocSLN,                142,    3,  3,  V, { VR }, 0, nullptr },
    { ocSYD,                143,    4,  4,  V, { VR }, 0, nullptr },
    { ocDDB,                144,    4,  5,  V, { VR }, 0, nullptr },
    { ocIndirect,           148,    1,  2,  R, { VR }, EXC_FUNCFLAG_VOLATILE, nullptr },
    { ocClean,              162,    1,  1,  V, { VR }, 0, nullptr },
    { ocMatDet,             163,    1,  1,  V, { VA }, 0, nullptr },
    { ocMatInv,             164,    1,  1,  A, { VA }, 0, nullptr },
    { ocMatMult,            165,    2,  2,  A, { VA }, 0, nullptr },
    { ocIpmt,               167,    4,  6,  V, { VR }, 0, nullptr },
    { ocPpmt,               168,    4,  6,  V, { VR }, 0, nullptr },
    { ocCount2,             169,    0,  MX, V, { RX }, 0, nullptr },
    { ocProduct,            183,    0,  MX, V, { RX }, 0, nullptr },
    { ocFact,               184,    1,  1,  V, { VR }, 0, nullptr },
    { ocDBProduct,          189,    3,  3,  V, { RO, RR }, 0, nullptr },
    { ocIsNonString,        190,    1,  1,  V, { VR }, 0, nullptr },
    { ocStDevP,             193,    1,  MX, V, { RX }, 0, nullptr },
    { ocVarP,               194,    1,  MX, V, { RX }, 0, nullptr },
    { ocDBStdDevP,          195,    3,  3,  V, { RO, RR }, 0, nullptr },
    { ocDBVarP,             196,    3,  3,  V, { RO, RR }, 0, nullptr },
    { ocTrunc,              197,    1,  1,  V, { VR, C }, 0, nullptr },
    { ocIsLogical,          198,    1,  1,  V, { VR }, 0, nullptr },
    { ocDBCount2,           199,    3,  3,  V, { RO, RR }, 0, nullptr },
    { ocCurrency,           204,    1,  2,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, nullptr },
    { ocFindB,              205,    2,  3,  V, { VR }, 0, nullptr },
    { ocSearchB,            206,    2,  3,  V, { VR }, 0, nullptr },
    { ocReplaceB,           207,    4,  4,  V, { VR }, 0, nullptr },
    { ocLeftB,              208,    1,  2,  V, { VR }, 0, nullptr },
    { ocRightB,             209,    1,  2,  V, { VR }, 0, nullptr },
    { ocMidB,               210,    3,  3,  V, { VR }, 0, nullptr },
    { ocLenB,               211,    1,  1,  V, { VR }, 0, nullptr },
    { ocRoundUp,            212,    2,  2,  V, { VR }, 0, nullptr },
    { ocRoundDown,          213,    2,  2,  V, { VR }, 0, nullptr },
    { ocExternal,           255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_IMPORTONLY, nullptr }
};

/** Functions new in BIFF3. */
static const XclFunctionInfo saFuncTable_3[] =
{
    { ocLinest,             49,     1,  4,  A, { RA, RA, VV }, 0, nullptr },          // BIFF2: 1-2, BIFF3: 1-4
    { ocTrend,              50,     1,  4,  A, { RA, RA, RA, VV }, 0, nullptr },      // BIFF2: 1-3, BIFF3: 1-4
    { ocLogest,             51,     1,  4,  A, { RA, RA, VV }, 0, nullptr },          // BIFF2: 1-2, BIFF3: 1-4
    { ocGrowth,             52,     1,  4,  A, { RA, RA, RA, VV }, 0, nullptr },      // BIFF2: 1-3, BIFF3: 1-4
    { ocTrunc,              197,    1,  2,  V, { VR }, 0, nullptr },                  // BIFF2: 1,   BIFF3: 1-2
    { ocAddress,            219,    2,  5,  V, { VR }, 0, nullptr },
    { ocGetDiffDate360,     220,    2,  2,  V, { VR, VR, C }, 0, nullptr },
    { ocGetActDate,         221,    0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, nullptr },
    { ocVBD,                222,    5,  7,  V, { VR }, 0, nullptr },
    { ocMedian,             227,    1,  MX, V, { RX }, 0, nullptr },
    { ocSumProduct,         228,    1,  MX, V, { VA }, 0, nullptr },
    { ocSinHyp,             229,    1,  1,  V, { VR }, 0, nullptr },
    { ocCosecantHyp,        229,    1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocCosHyp,             230,    1,  1,  V, { VR }, 0, nullptr },
    { ocSecantHyp,          230,    1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocTanHyp,             231,    1,  1,  V, { VR }, 0, nullptr },
    { ocCotHyp,             231,    1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocArcSinHyp,          232,    1,  1,  V, { VR }, 0, nullptr },
    { ocArcCosHyp,          233,    1,  1,  V, { VR }, 0, nullptr },
    { ocArcTanHyp,          234,    1,  1,  V, { VR }, 0, nullptr },
    { ocArcCotHyp,          234,    1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocDBGet,              235,    3,  3,  V, { RO, RR }, 0, nullptr },
    { ocInfo,               244,    1,  1,  V, { VR }, EXC_FUNCFLAG_VOLATILE, nullptr }
};

/** Functions new in BIFF4. */
static const XclFunctionInfo saFuncTable_4[] =
{
    { ocFixed,              14,     1,  3,  V, { VR }, 0, nullptr },                  // BIFF2-3: 1-2, BIFF4: 1-3
    { ocAsc,                214,    1,  1,  V, { VR }, 0, nullptr },
    { ocJis,                215,    1,  1,  V, { VR }, 0, nullptr },
    { ocRank,               216,    2,  3,  V, { VR, RO, VR }, 0, nullptr },
    { ocDB,                 247,    4,  5,  V, { VR }, 0, nullptr },
    { ocFrequency,          252,    2,  2,  A, { RA }, 0, nullptr },
    { ocErrorType_ODF,      261,    1,  1,  V, { VR }, 0, nullptr },
    { ocAveDev,             269,    1,  MX, V, { RX }, 0, nullptr },
    { ocBetaDist,           270,    3,  5,  V, { VR }, 0, nullptr },
    { ocGammaLn,            271,    1,  1,  V, { VR }, 0, nullptr },
    { ocBetaInv,            272,    3,  5,  V, { VR }, 0, nullptr },
    { ocBinomDist,          273,    4,  4,  V, { VR }, 0, nullptr },
    { ocChiDist,            274,    2,  2,  V, { VR }, 0, nullptr },
    { ocChiInv,             275,    2,  2,  V, { VR }, 0, nullptr },
    { ocCombin,             276,    2,  2,  V, { VR }, 0, nullptr },
    { ocConfidence,         277,    3,  3,  V, { VR }, 0, nullptr },
    { ocCritBinom,          278,    3,  3,  V, { VR }, 0, nullptr },
    { ocEven,               279,    1,  1,  V, { VR }, 0, nullptr },
    { ocExpDist,            280,    3,  3,  V, { VR }, 0, nullptr },
    { ocFDist,              281,    3,  3,  V, { VR }, 0, nullptr },
    { ocFInv,               282,    3,  3,  V, { VR }, 0, nullptr },
    { ocFisher,             283,    1,  1,  V, { VR }, 0, nullptr },
    { ocFisherInv,          284,    1,  1,  V, { VR }, 0, nullptr },
    { ocFloor_MS,           285,    2,  2,  V, { VR }, 0, nullptr },
    { ocGammaDist,          286,    4,  4,  V, { VR }, 0, nullptr },
    { ocGammaInv,           287,    3,  3,  V, { VR }, 0, nullptr },
    { ocCeil_MS,            288,    2,  2,  V, { VR }, 0, nullptr },
    { ocHypGeomDist,        289,    4,  4,  V, { VR }, 0, nullptr },
    { ocLogNormDist,        290,    3,  3,  V, { VR }, 0, nullptr },
    { ocLogInv,             291,    3,  3,  V, { VR }, 0, nullptr },
    { ocNegBinomVert,       292,    3,  3,  V, { VR }, 0, nullptr },
    { ocNormDist,           293,    4,  4,  V, { VR }, 0, nullptr },
    { ocStdNormDist,        294,    1,  1,  V, { VR }, 0, nullptr },
    { ocNormInv,            295,    3,  3,  V, { VR }, 0, nullptr },
    { ocSNormInv,           296,    1,  1,  V, { VR }, 0, nullptr },
    { ocStandard,           297,    3,  3,  V, { VR }, 0, nullptr },
    { ocOdd,                298,    1,  1,  V, { VR }, 0, nullptr },
    { ocPermut,             299,    2,  2,  V, { VR }, 0, nullptr },
    { ocPoissonDist,        300,    3,  3,  V, { VR }, 0, nullptr },
    { ocTDist,              301,    3,  3,  V, { VR }, 0, nullptr },
    { ocWeibull,            302,    4,  4,  V, { VR }, 0, nullptr },
    { ocSumXMY2,            303,    2,  2,  V, { VA }, 0, nullptr },
    { ocSumX2MY2,           304,    2,  2,  V, { VA }, 0, nullptr },
    { ocSumX2DY2,           305,    2,  2,  V, { VA }, 0, nullptr },
    { ocChiTest,            306,    2,  2,  V, { VA }, 0, nullptr },
    { ocCorrel,             307,    2,  2,  V, { VA }, 0, nullptr },
    { ocCovar,              308,    2,  2,  V, { VA }, 0, nullptr },
    { ocForecast,           309,    3,  3,  V, { VR, VA }, 0, nullptr },
    { ocFTest,              310,    2,  2,  V, { VA }, 0, nullptr },
    { ocIntercept,          311,    2,  2,  V, { VA }, 0, nullptr },
    { ocPearson,            312,    2,  2,  V, { VA }, 0, nullptr },
    { ocRSQ,                313,    2,  2,  V, { VA }, 0, nullptr },
    { ocSTEYX,              314,    2,  2,  V, { VA }, 0, nullptr },
    { ocSlope,              315,    2,  2,  V, { VA }, 0, nullptr },
    { ocTTest,              316,    4,  4,  V, { VA, VA, VR }, 0, nullptr },
    { ocProb,               317,    3,  4,  V, { VA, VA, VR }, 0, nullptr },
    { ocDevSq,              318,    1,  MX, V, { RX }, 0, nullptr },
    { ocGeoMean,            319,    1,  MX, V, { RX }, 0, nullptr },
    { ocHarMean,            320,    1,  MX, V, { RX }, 0, nullptr },
    { ocSumSQ,              321,    0,  MX, V, { RX }, 0, nullptr },
    { ocKurt,               322,    1,  MX, V, { RX }, 0, nullptr },
    { ocSkew,               323,    1,  MX, V, { RX }, 0, nullptr },
    { ocZTest,              324,    2,  3,  V, { RX, VR }, 0, nullptr },
    { ocLarge,              325,    2,  2,  V, { RX, VR }, 0, nullptr },
    { ocSmall,              326,    2,  2,  V, { RX, VR }, 0, nullptr },
    { ocQuartile,           327,    2,  2,  V, { RX, VR }, 0, nullptr },
    { ocPercentile,         328,    2,  2,  V, { RX, VR }, 0, nullptr },
    { ocPercentrank,        329,    2,  3,  V, { RX, VR, VR_E }, 0, nullptr },
    { ocModalValue,         330,    1,  MX, V, { VA }, 0, nullptr },
    { ocTrimMean,           331,    2,  2,  V, { RX, VR }, 0, nullptr },
    { ocTInv,               332,    2,  2,  V, { VR }, 0, nullptr },
    // Functions equivalent to add-in functions, use same parameters as
    // ocExternal but add programmatical function name (here without
    // "com.sun.star.sheet.addin.") so it can be looked up and stored as
    // add-in, as older Excel versions only know them as add-in.
    { ocIsEven,             255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getIseven" ) },
    { ocIsOdd,              255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getIsodd" ) },
    { ocGCD,                255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getGcd" ) },
    { ocLCM,                255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getLcm" ) },
    { ocEffect,             255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getEffect" ) },
    { ocCumPrinc,           255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getCumprinc" ) },
    { ocCumIpmt,            255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getCumipmt" ) },
    { ocNominal,            255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getNominal" ) },
    { ocNetWorkdays,        255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY | EXC_FUNCFLAG_ADDINEQUIV, EXC_FUNCNAME_ADDIN( "Analysis.getNetworkdays" ) }
};

/** Functions new in BIFF5/BIFF7. Unsupported functions: DATESTRING, NUMBERSTRING. */
static const XclFunctionInfo saFuncTable_5[] =
{
    { ocGetDayOfWeek,       70,     1,  2,  V, { VR }, 0, nullptr },                  // BIFF2-4: 1, BIFF5: 1-2
    { ocHLookup,            101,    3,  4,  V, { VV, RO, RO, VV }, 0, nullptr },      // BIFF2-4: 3, BIFF5: 3-4
    { ocVLookup,            102,    3,  4,  V, { VV, RO, RO, VV }, 0, nullptr },      // BIFF2-4: 3, BIFF5: 3-4
    { ocGetDiffDate360,     220,    2,  3,  V, { VR }, 0, nullptr },                  // BIFF3-4: 2, BIFF5: 2-3
    { ocMacro,              255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocExternal,           255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, nullptr },
    { ocConcat,             336,    0,  MX, V, { VR }, 0, nullptr },
    { ocPower,              337,    2,  2,  V, { VR }, 0, nullptr },
    { ocRad,                342,    1,  1,  V, { VR }, 0, nullptr },
    { ocDeg,                343,    1,  1,  V, { VR }, 0, nullptr },
    { ocSubTotal,           344,    2,  MX, V, { VR, RO }, 0, nullptr },
    { ocSumIf,              345,    2,  3,  V, { RO, VR, RO }, 0, nullptr },
    { ocCountIf,            346,    2,  2,  V, { RO, VR }, 0, nullptr },
    { ocCountEmptyCells,    347,    1,  1,  V, { RO }, 0, nullptr },
    { ocISPMT,              350,    4,  4,  V, { VR }, 0, nullptr },
    { ocGetDateDif,         351,    3,  3,  V, { VR }, 0, nullptr },
    { ocNoName,             352,    1,  1,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, nullptr },    // DATESTRING
    { ocNoName,             353,    2,  2,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, nullptr },    // NUMBERSTRING
    { ocRoman,              354,    1,  2,  V, { VR }, 0, nullptr }
};

/** Functions new in BIFF8. Unsupported functions: PHONETIC. */
static const XclFunctionInfo saFuncTable_8[] =
{
    { ocGetPivotData,       358,    2,  MX, V, { RR, RR, VR }, 0, nullptr },
    { ocHyperLink,          359,    1,  2,  V, { VV, VO }, 0, nullptr },
    { ocNoName,             360,    1,  1,  V, { RO }, EXC_FUNCFLAG_IMPORTONLY, nullptr },    // PHONETIC
    { ocAverageA,           361,    1,  MX, V, { RX }, 0, nullptr },
    { ocMaxA,               362,    1,  MX, V, { RX }, 0, nullptr },
    { ocMinA,               363,    1,  MX, V, { RX }, 0, nullptr },
    { ocStDevPA,            364,    1,  MX, V, { RX }, 0, nullptr },
    { ocVarPA,              365,    1,  MX, V, { RX }, 0, nullptr },
    { ocStDevA,             366,    1,  MX, V, { RX }, 0, nullptr },
    { ocVarA,               367,    1,  MX, V, { RX }, 0, nullptr },
    { ocBahtText,           368,    1,  1,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME( "BAHTTEXT" ) },
    { ocBahtText,           255,    2,  2,  V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME( "BAHTTEXT" ) },
    { ocEuroConvert,        255,    4,  6,  V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, "EUROCONVERT" }
};

#define EXC_FUNCENTRY_V_VR( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { VR },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }, \
    { opcode,  255, (minparam)+1, (maxparam)+1, V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

/** Functions new in OOXML. */
static const XclFunctionInfo saFuncTable_Oox[] =
{
    { ocCountIfs,           NOID,   2,  MX, V, { RO, VR }, EXC_FUNCFLAG_IMPORTONLY|EXC_FUNCFLAG_PARAMPAIRS, EXC_FUNCNAME( "COUNTIFS" ) },
    { ocCountIfs,           255,    3,  MX, V, { RO_E, RO, VR }, EXC_FUNCFLAG_EXPORTONLY|EXC_FUNCFLAG_PARAMPAIRS, EXC_FUNCNAME( "COUNTIFS" ) },
    { ocSumIfs,             NOID,   3,  MX, V, { RO, RO, VR }, EXC_FUNCFLAG_IMPORTONLY|EXC_FUNCFLAG_PARAMPAIRS, EXC_FUNCNAME( "SUMIFS" ) },
    { ocSumIfs,             255,    4,  MX, V, { RO_E, RO, RO, VR }, EXC_FUNCFLAG_EXPORTONLY|EXC_FUNCFLAG_PARAMPAIRS, EXC_FUNCNAME( "SUMIFS" ) },
    { ocAverageIf,          NOID,   2,  3,  V, { RO, VR, RO }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME( "AVERAGEIF" ) },
    { ocAverageIf,          255,    3,  4,  V, { RO_E, RO, VR, RO }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME( "AVERAGEIF" ) },
    { ocAverageIfs,         NOID,   3,  MX, V, { RO, RO, VR }, EXC_FUNCFLAG_IMPORTONLY|EXC_FUNCFLAG_PARAMPAIRS, EXC_FUNCNAME( "AVERAGEIFS" ) },
    { ocAverageIfs,         255,    4,  MX, V, { RO_E, RO, RO, VR }, EXC_FUNCFLAG_EXPORTONLY|EXC_FUNCFLAG_PARAMPAIRS, EXC_FUNCNAME( "AVERAGEIFS" ) },
    { ocIfError,            NOID,   2,  2,  V, { VO, RO }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME( "IFERROR" ) },
    { ocIfError,            255,    3,  3,  V, { RO_E, VO, RO }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME( "IFERROR" ) },
    { ocNetWorkdays_MS,     NOID,   2,  4,  V, { VR, VR, RO, RO }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME( "NETWORKDAYS.INTL" ) },
    { ocNetWorkdays_MS,     255,    3,  5,  V, { RO_E, VR, VR, RO, RO }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME( "NETWORKDAYS.INTL" ) },
    { ocWorkday_MS,         NOID,   2,  4,  V, { VR, VR, VR, RO }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME( "WORKDAY.INTL" ) },
    { ocWorkday_MS,         255,    3,  5,  V, { RO_E, VR, VR, VR, RO }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME( "WORKDAY.INTL" ) },
    EXC_FUNCENTRY_V_VR( ocCeil_ISO,         1,  2,  0,  "ISO.CEILING" )
};

#define EXC_FUNCENTRY_V_VR_IMPORT( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { VR },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

#define EXC_FUNCENTRY_V_RO_EXPORT( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode,  255, (minparam)+1, (maxparam)+1, V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

#define EXC_FUNCENTRY_A_VR( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     A, { VR },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }, \
    { opcode,  255, (minparam)+1, (maxparam)+1, A, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

#define EXC_FUNCENTRY_V_RO( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { RO },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }, \
    { opcode,  255, (minparam)+1, (maxparam)+1, V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

// implicit maxparam=MX
#define EXC_FUNCENTRY_V_RX( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     MX,           V, { RX },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }, \
    { opcode,  255, (minparam)+1, MX,           V, { RO_E, RX }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

#define EXC_FUNCENTRY_V_VA( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { VA },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }, \
    { opcode,  255, (minparam)+1, (maxparam)+1, V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

/** Functions new in Excel 2010.

    See http://office.microsoft.com/en-us/excel-help/what-s-new-changes-made-to-excel-functions-HA010355760.aspx
    A lot of statistical functions have been renamed (the 'old' function names still exist).

    @See sc/source/filter/oox/formulabase.cxx saFuncTable2010 for V,VR,RO,...
 */
static const XclFunctionInfo saFuncTable_2010[] =
{
    EXC_FUNCENTRY_V_VA( ocCovarianceP,      2,  2,  0,  "COVARIANCE.P" ),
    EXC_FUNCENTRY_V_VA( ocCovarianceS,      2,  2,  0,  "COVARIANCE.S" ),
    EXC_FUNCENTRY_V_RX( ocStDevP_MS,        1, MX,  0,  "STDEV.P" ),
    EXC_FUNCENTRY_V_RX( ocStDevS,           1, MX,  0,  "STDEV.S" ),
    EXC_FUNCENTRY_V_RX( ocVarP_MS,          1, MX,  0,  "VAR.P" ),
    EXC_FUNCENTRY_V_RX( ocVarS,             1, MX,  0,  "VAR.S" ),
    EXC_FUNCENTRY_V_VR( ocBetaDist_MS,      4,  6,  0,  "BETA.DIST" ),
    EXC_FUNCENTRY_V_VR( ocBetaInv_MS,       3,  5,  0,  "BETA.INV" ),
    EXC_FUNCENTRY_V_VR( ocBinomDist_MS,     4,  4,  0,  "BINOM.DIST" ),
    EXC_FUNCENTRY_V_VR( ocBinomInv,         3,  3,  0,  "BINOM.INV" ),
    EXC_FUNCENTRY_V_VR( ocChiSqDist_MS,     3,  3,  0,  "CHISQ.DIST" ),
    EXC_FUNCENTRY_V_VR( ocChiSqInv_MS,      2,  2,  0,  "CHISQ.INV" ),
    EXC_FUNCENTRY_V_VR( ocChiDist_MS,       2,  2,  0,  "CHISQ.DIST.RT" ),
    EXC_FUNCENTRY_V_VR( ocChiInv_MS,        2,  2,  0,  "CHISQ.INV.RT" ),
    EXC_FUNCENTRY_V_VR( ocChiTest_MS,       2,  2,  0,  "CHISQ.TEST" ),
    EXC_FUNCENTRY_V_VR( ocConfidence_N,     3,  3,  0,  "CONFIDENCE.NORM" ),
    EXC_FUNCENTRY_V_VR( ocConfidence_T,     3,  3,  0,  "CONFIDENCE.T" ),
    EXC_FUNCENTRY_V_VR( ocFDist_LT,         4,  4,  0,  "F.DIST" ),
    EXC_FUNCENTRY_V_VR( ocFDist_RT,         3,  3,  0,  "F.DIST.RT" ),
    EXC_FUNCENTRY_V_VR( ocFInv_LT,          3,  3,  0,  "F.INV" ),
    EXC_FUNCENTRY_V_VR( ocFInv_RT,          3,  3,  0,  "F.INV.RT" ),
    EXC_FUNCENTRY_V_VR( ocFTest_MS,         2,  2,  0,  "F.TEST" ),
    EXC_FUNCENTRY_V_VR( ocExpDist_MS,       3,  3,  0,  "EXPON.DIST" ),
    EXC_FUNCENTRY_V_VR( ocHypGeomDist_MS,   5,  5,  0,  "HYPGEOM.DIST" ),
    EXC_FUNCENTRY_V_VR( ocPoissonDist_MS,   3,  3,  0,  "POISSON.DIST" ),
    EXC_FUNCENTRY_V_VR( ocWeibull_MS,       4,  4,  0,  "WEIBULL.DIST" ),
    EXC_FUNCENTRY_V_VR( ocGammaDist_MS,     4,  4,  0,  "GAMMA.DIST" ),
    EXC_FUNCENTRY_V_VR( ocGammaInv_MS,      3,  3,  0,  "GAMMA.INV" ),
    EXC_FUNCENTRY_V_VR( ocGammaLn_MS,       1,  1,  0,  "GAMMALN.PRECISE" ),
    EXC_FUNCENTRY_V_VR( ocLogNormDist_MS,   4,  4,  0,  "LOGNORM.DIST" ),
    EXC_FUNCENTRY_V_VR( ocLogInv_MS,        3,  3,  0,  "LOGNORM.INV" ),
    EXC_FUNCENTRY_V_VR( ocNormDist_MS,      4,  4,  0,  "NORM.DIST" ),
    EXC_FUNCENTRY_V_VR( ocNormInv_MS,       3,  3,  0,  "NORM.INV" ),
    EXC_FUNCENTRY_V_VR( ocStdNormDist_MS,   2,  2,  0,  "NORM.S.DIST" ),
    EXC_FUNCENTRY_V_VR( ocSNormInv_MS,      1,  1,  0,  "NORM.S.INV" ),
    EXC_FUNCENTRY_V_VR( ocTDist_2T,         2,  2,  0,  "T.DIST.2T" ),
    EXC_FUNCENTRY_V_VR( ocTDist_MS,         3,  3,  0,  "T.DIST" ),
    EXC_FUNCENTRY_V_VR( ocTDist_RT,         2,  2,  0,  "T.DIST.RT" ),
    EXC_FUNCENTRY_V_VR( ocTInv_2T,          2,  2,  0,  "T.INV.2T" ),
    EXC_FUNCENTRY_V_VR( ocTInv_MS,          2,  2,  0,  "T.INV" ),
    EXC_FUNCENTRY_V_VR( ocTTest_MS,         4,  4,  0,  "T.TEST" ),
    EXC_FUNCENTRY_V_VR( ocPercentile_Inc,   2,  2,  0,  "PERCENTILE.INC" ),
    EXC_FUNCENTRY_V_VR( ocPercentrank_Inc,  2,  3,  0,  "PERCENTRANK.INC" ),
    EXC_FUNCENTRY_V_VR( ocQuartile_Inc,     2,  2,  0,  "QUARTILE.INC" ),
    EXC_FUNCENTRY_V_VR( ocRank_Eq,          2,  3,  0,  "RANK.EQ" ),
    EXC_FUNCENTRY_V_VR( ocPercentile_Exc,   2,  2,  0,  "PERCENTILE.EXC" ),
    EXC_FUNCENTRY_V_VR( ocPercentrank_Exc,  2,  3,  0,  "PERCENTRANK.EXC" ),
    EXC_FUNCENTRY_V_VR( ocQuartile_Exc,     2,  2,  0,  "QUARTILE.EXC" ),
    EXC_FUNCENTRY_V_VR( ocRank_Avg,         2,  3,  0,  "RANK.AVG" ),
    EXC_FUNCENTRY_V_RX( ocModalValue_MS,    1, MX,  0,  "MODE.SNGL" ),
    EXC_FUNCENTRY_V_RX( ocModalValue_Multi, 1, MX,  0,  "MODE.MULT" ),
    EXC_FUNCENTRY_V_VR( ocNegBinomDist_MS,  4,  4,  0,  "NEGBINOM.DIST" ),
    EXC_FUNCENTRY_V_VR( ocZTest_MS,         2,  3,  0,  "Z.TEST" ),
    EXC_FUNCENTRY_V_VR( ocCeil_Precise,     1,  2,  0,  "CEILING.PRECISE" ),
    EXC_FUNCENTRY_V_VR( ocFloor_Precise,    1,  2,  0,  "FLOOR.PRECISE" ),
    EXC_FUNCENTRY_V_VR( ocErf_MS,           1,  1,  0,  "ERF.PRECISE" ),
    EXC_FUNCENTRY_V_VR( ocErfc_MS,          1,  1,  0,  "ERFC.PRECISE" ),
    EXC_FUNCENTRY_V_RX( ocAggregate,        3, MX,  0,  "AGGREGATE" ),
};

/** Functions new in Excel 2013.

    See http://office.microsoft.com/en-us/excel-help/new-functions-in-excel-2013-HA103980604.aspx
    Most functions apparently were added for ODF1.2 ODFF / OpenFormula
    compatibility.

    Functions with EXC_FUNCENTRY_V_VR_IMPORT are rewritten in
    sc/source/filter/excel/xeformula.cxx during export for BIFF, OOXML export
    uses a different mapping but still uses this mapping here to determine the
    feature set.

    FIXME: either have the exporter determine the feature set from the active
    mapping, preferred, or enhance this mapping here such that for OOXML the
    rewrite can be overridden.

    @See sc/source/filter/oox/formulabase.cxx saFuncTable2013 for V,VR,RO,...
 */
static const XclFunctionInfo saFuncTable_2013[] =
{
    EXC_FUNCENTRY_V_VR_IMPORT(  ocArcCot,        1,  1,  0,  "ACOT" ),
    EXC_FUNCENTRY_V_VR_IMPORT(  ocArcCotHyp,     1,  1,  0,  "ACOTH" ),
    EXC_FUNCENTRY_V_VR(         ocArabic,        1,  1,  0,  "ARABIC" ),
    EXC_FUNCENTRY_V_VR(         ocBase,          2,  3,  0,  "BASE" ),
    EXC_FUNCENTRY_V_VR(         ocB,             3,  4,  0,  "BINOM.DIST.RANGE" ),
    EXC_FUNCENTRY_V_VR(         ocBitAnd,        2,  2,  0,  "BITAND" ),
    EXC_FUNCENTRY_V_VR(         ocBitLshift,     2,  2,  0,  "BITLSHIFT" ),
    EXC_FUNCENTRY_V_VR(         ocBitOr,         2,  2,  0,  "BITOR" ),
    EXC_FUNCENTRY_V_VR(         ocBitRshift,     2,  2,  0,  "BITRSHIFT" ),
    EXC_FUNCENTRY_V_VR(         ocBitXor,        2,  2,  0,  "BITXOR" ),
    EXC_FUNCENTRY_V_VR(         ocCeil_Math,     1,  3,  0,  "CEILING.MATH" ),
    EXC_FUNCENTRY_V_RO_EXPORT(  ocCeil,          1,  3,  0,  "CEILING.MATH" ),
    EXC_FUNCENTRY_V_VR(         ocCombinA,       2,  2,  0,  "COMBINA" ),
    EXC_FUNCENTRY_V_VR_IMPORT(  ocCot,           1,  1,  0,  "COT" ),
    EXC_FUNCENTRY_V_VR_IMPORT(  ocCotHyp,        1,  1,  0,  "COTH" ),
    EXC_FUNCENTRY_V_VR_IMPORT(  ocCosecant,      1,  1,  0,  "CSC" ),
    EXC_FUNCENTRY_V_VR_IMPORT(  ocCosecantHyp,   1,  1,  0,  "CSCH" ),
    EXC_FUNCENTRY_V_VR(         ocGetDiffDate,   2,  2,  0,  "DAYS" ),
    EXC_FUNCENTRY_V_VR(         ocDecimal,       2,  2,  0,  "DECIMAL" ),
    EXC_FUNCENTRY_V_VR(         ocEncodeURL,     1,  1,  0,  "ENCODEURL" ),
    // NOTE: this FDIST is not our LEGACY.FDIST
    EXC_FUNCENTRY_V_VR(         ocNoName,        3,  4,  0,  "FDIST" ),
    // NOTE: this FINV is not our LEGACY.FINV
    EXC_FUNCENTRY_V_VR(         ocNoName,        3,  3,  0,  "FINV" ),
    EXC_FUNCENTRY_V_VR(         ocFilterXML,     2,  2,  0,  "FILTERXML" ),
    EXC_FUNCENTRY_V_VR(         ocFloor_Math,    1,  3,  0,  "FLOOR.MATH" ),
    EXC_FUNCENTRY_V_RO_EXPORT(  ocFloor,         1,  3,  0,  "FLOOR.MATH" ),
    EXC_FUNCENTRY_V_RO(         ocFormula,       1,  1,  0,  "FORMULATEXT" ),
    EXC_FUNCENTRY_V_VR(         ocGamma,         1,  1,  0,  "GAMMA" ),
    EXC_FUNCENTRY_V_VR(         ocGauss,         1,  1,  0,  "GAUSS" ),
    {                           ocIfNA,       NOID,  2,  2,  V, { VO, RO }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME( "IFNA" ) },
    {                           ocIfNA,        255,  3,  3,  V, { RO_E, VO, RO }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME( "IFNA" ) },
    // IMCOSH, IMCOT, IMCSC, IMCSCH, IMSEC, IMSECH, IMSINH and IMTAN are
    // implemented in the Analysis Add-In.
    EXC_FUNCENTRY_V_RO(         ocIsFormula,     1,  1,  0,  "ISFORMULA" ),
    EXC_FUNCENTRY_V_VR(         ocWeek,          1,  2,  0,  "WEEKNUM" ),
    EXC_FUNCENTRY_V_VR(         ocIsoWeeknum,    1,  1,  0,  "ISOWEEKNUM" ),
    EXC_FUNCENTRY_A_VR(         ocMatrixUnit,    1,  1,  0,  "MUNIT" ),
    EXC_FUNCENTRY_V_VR(         ocNumberValue,   1,  3,  0,  "NUMBERVALUE" ),
    EXC_FUNCENTRY_V_VR(         ocPDuration,     3,  3,  0,  "PDURATION" ),
    EXC_FUNCENTRY_V_VR(         ocPermutationA,  2,  2,  0,  "PERMUTATIONA" ),
    EXC_FUNCENTRY_V_VR(         ocPhi,           1,  1,  0,  "PHI" ),
    EXC_FUNCENTRY_V_VR(         ocRRI,           3,  3,  0,  "RRI" ),
    EXC_FUNCENTRY_V_VR_IMPORT(  ocSecant,        1,  1,  0,  "SEC" ),
    EXC_FUNCENTRY_V_VR_IMPORT(  ocSecantHyp,     1,  1,  0,  "SECH" ),
    EXC_FUNCENTRY_V_RO(         ocSheet,         0,  1,  0,  "SHEET" ),
    EXC_FUNCENTRY_V_RO(         ocSheets,        0,  1,  0,  "SHEETS" ),
    EXC_FUNCENTRY_V_RX(         ocSkewp,         1,  MX, 0,  "SKEW.P" ),
    EXC_FUNCENTRY_V_VR(         ocUnichar,       1,  1,  0,  "UNICHAR" ),
    EXC_FUNCENTRY_V_VR(         ocUnicode,       1,  1,  0,  "UNICODE" ),
    EXC_FUNCENTRY_V_VR(         ocWebservice,    1,  1,  0,  "WEBSERVICE" ),
    EXC_FUNCENTRY_V_RX(         ocXor,           1,  MX, 0,  "XOR" ),
    EXC_FUNCENTRY_V_VR(         ocErrorType_ODF, 1,  1,  0,  "ERROR.TYPE" )
};

/** Functions new in Excel 2016.

    See https://support.office.com/en-us/article/Forecasting-functions-897a2fe9-6595-4680-a0b0-93e0308d5f6e?ui=en-US&rs=en-US&ad=US#_forecast.ets
    and  https://support.office.com/en-us/article/What-s-New-and-Improved-in-Office-2016-for-Office-365-95c8d81d-08ba-42c1-914f-bca4603e1426?ui=en-US&rs=en-US&ad=US

    @See sc/source/filter/oox/formulabase.cxx saFuncTable2016 for V,VR,RO,...
 */
static const XclFunctionInfo saFuncTable_2016[] =
{
    EXC_FUNCENTRY_V_VR(  ocForecast_ETS_ADD,    3,  6,  0,  "FORECAST.ETS" ),
    EXC_FUNCENTRY_V_VR(  ocForecast_ETS_PIA,    3,  7,  0,  "FORECAST.ETS.CONFINT" ),
    EXC_FUNCENTRY_V_VR(  ocForecast_ETS_SEA,    2,  4,  0,  "FORECAST.ETS.SEASONALITY" ),
    EXC_FUNCENTRY_V_VR(  ocForecast_ETS_STA,    3,  6,  0,  "FORECAST.ETS.STAT" ),
    EXC_FUNCENTRY_V_VR(  ocForecast_LIN,        3,  3,  0,  "FORECAST.LINEAR" ),
    EXC_FUNCENTRY_V_VR(  ocConcat_MS,           1,  MX, 0,  "CONCAT" ),
    EXC_FUNCENTRY_V_VR(  ocTextJoin_MS,         3,  MX, 0,  "TEXTJOIN" ),
    EXC_FUNCENTRY_V_VR(  ocIfs_MS,              2,  MX, 0,  "IFS" ),
    EXC_FUNCENTRY_V_VR(  ocSwitch_MS,           3,  MX, 0,  "SWITCH" ),
    EXC_FUNCENTRY_V_VR(  ocMinIfs_MS,           3,  MX, 0,  "MINIFS" ),
    EXC_FUNCENTRY_V_VR(  ocMaxIfs_MS,           3,  MX, 0,  "MAXIFS" )
};

#define EXC_FUNCENTRY_ODF( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { VR },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME_ODF( asciiname ) }, \
    { opcode,  255, (minparam)+1, (maxparam)+1, V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME_ODF( asciiname ) }

/** Functions defined by OpenFormula, but not supported by Calc (ocNoName) or by Excel (defined op-code). */
static const XclFunctionInfo saFuncTable_Odf[] =
{
    EXC_FUNCENTRY_ODF( ocChiSqDist,     2,  3,  0,  "CHISQDIST" ),
    EXC_FUNCENTRY_ODF( ocChiSqInv,      2,  2,  0,  "CHISQINV" )
};

#undef EXC_FUNCENTRY_ODF

#define EXC_FUNCENTRY_OOO( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { VR },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }, \
    { opcode,  255, (minparam)+1, (maxparam)+1, V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME( asciiname ) }

// Import Broken Raw ... even without leading _xlfn.
#define EXC_FUNCENTRY_OOO_IBR( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { VR },       EXC_FUNCFLAG_IMPORTONLY|(flags), asciiname }

/** Functions defined by Calc, but not in OpenFormula nor supported by Excel. */
static const XclFunctionInfo saFuncTable_OOoLO[] =
{
    EXC_FUNCENTRY_OOO( ocErrorType,     1,  1,  0,  "ORG.OPENOFFICE.ERRORTYPE" ),
    EXC_FUNCENTRY_OOO_IBR( ocErrorType, 1,  1,  0,  "ERRORTYPE" ),      // was written wrongly, read it
    EXC_FUNCENTRY_OOO( ocMultiArea,     1, MX,  0,  "ORG.OPENOFFICE.MULTIRANGE" ),
    EXC_FUNCENTRY_OOO_IBR( ocMultiArea, 1, MX,  0,  "MULTIRANGE" ),     // was written wrongly, read it
    EXC_FUNCENTRY_OOO( ocBackSolver,    3,  3,  0,  "ORG.OPENOFFICE.GOALSEEK" ),
    EXC_FUNCENTRY_OOO_IBR( ocBackSolver,3,  3,  0,  "GOALSEEK" ),       // was written wrongly, read it
    EXC_FUNCENTRY_OOO( ocEasterSunday,  1,  1,  0,  "ORG.OPENOFFICE.EASTERSUNDAY" ),
    EXC_FUNCENTRY_OOO_IBR( ocEasterSunday,1,1,  0,  "EASTERSUNDAY" ),   // was written wrongly, read it
    EXC_FUNCENTRY_OOO( ocCurrent,       0,  0,  0,  "ORG.OPENOFFICE.CURRENT" ),
    EXC_FUNCENTRY_OOO_IBR( ocCurrent,   0,  0,  0,  "CURRENT" ),        // was written wrongly, read it
    EXC_FUNCENTRY_OOO( ocStyle,         1,  3,  0,  "ORG.OPENOFFICE.STYLE" ),
    EXC_FUNCENTRY_OOO_IBR( ocStyle,     1,  3,  0,  "STYLE" ),          // was written wrongly, read it
    EXC_FUNCENTRY_OOO( ocConvertOOo,    3,  3,  0,  "ORG.OPENOFFICE.CONVERT" ),
    EXC_FUNCENTRY_OOO( ocColor,         3,  4,  0,  "ORG.LIBREOFFICE.COLOR" ),
    EXC_FUNCENTRY_OOO( ocRawSubtract,   2, MX,  0,  "ORG.LIBREOFFICE.RAWSUBTRACT" ),
    EXC_FUNCENTRY_OOO( ocWeeknumOOo,    2,  2,  0,  "ORG.LIBREOFFICE.WEEKNUM_OOO" ),
    EXC_FUNCENTRY_OOO( ocForecast_ETS_MUL, 3,  6,  0,  "ORG.LIBREOFFICE.FORECAST.ETS.MULT" ),
    EXC_FUNCENTRY_OOO( ocForecast_ETS_PIM, 3,  7,  0,  "ORG.LIBREOFFICE.FORECAST.ETS.PI.MULT" ),
    EXC_FUNCENTRY_OOO( ocForecast_ETS_STM, 3,  6,  0,  "ORG.LIBREOFFICE.FORECAST.ETS.STAT.MULT" ),
    EXC_FUNCENTRY_OOO( ocRoundSig,      2,  2,  0,  "ORG.LIBREOFFICE.ROUNDSIG" ),
    EXC_FUNCENTRY_OOO( ocRegex,         2,  4,  0,  "ORG.LIBREOFFICE.REGEX" ),
    EXC_FUNCENTRY_OOO( ocFourier,       2,  4,  0,  "ORG.LIBREOFFICE.FOURIER" )
};

#undef EXC_FUNCENTRY_OOO_IBR
#undef EXC_FUNCENTRY_OOO

XclFunctionProvider::XclFunctionProvider( const XclRoot& rRoot )
{
    void (XclFunctionProvider::*pFillFunc)( const XclFunctionInfo*, const XclFunctionInfo* ) =
        rRoot.IsImport() ? &XclFunctionProvider::FillXclFuncMap : &XclFunctionProvider::FillScFuncMap;

    /*  Only read/write functions supported in the current BIFF version.
        Function tables from later BIFF versions may overwrite single functions
        from earlier tables. */
    XclBiff eBiff = rRoot.GetBiff();
    if( eBiff >= EXC_BIFF2 )
        (this->*pFillFunc)(saFuncTable_2, saFuncTable_2 + SAL_N_ELEMENTS(saFuncTable_2));
    if( eBiff >= EXC_BIFF3 )
        (this->*pFillFunc)(saFuncTable_3, saFuncTable_3 + SAL_N_ELEMENTS(saFuncTable_3));
    if( eBiff >= EXC_BIFF4 )
        (this->*pFillFunc)(saFuncTable_4, saFuncTable_4 + SAL_N_ELEMENTS(saFuncTable_4));
    if( eBiff >= EXC_BIFF5 )
        (this->*pFillFunc)(saFuncTable_5, saFuncTable_5 + SAL_N_ELEMENTS(saFuncTable_5));
    if( eBiff >= EXC_BIFF8 )
        (this->*pFillFunc)(saFuncTable_8, saFuncTable_8 + SAL_N_ELEMENTS(saFuncTable_8));
    (this->*pFillFunc)(saFuncTable_Oox, saFuncTable_Oox + SAL_N_ELEMENTS(saFuncTable_Oox));
    (this->*pFillFunc)(saFuncTable_2010, saFuncTable_2010 + SAL_N_ELEMENTS(saFuncTable_2010));
    (this->*pFillFunc)(saFuncTable_2013, saFuncTable_2013 + SAL_N_ELEMENTS(saFuncTable_2013));
    (this->*pFillFunc)(saFuncTable_2016, saFuncTable_2016 + SAL_N_ELEMENTS(saFuncTable_2016));
    (this->*pFillFunc)(saFuncTable_Odf, saFuncTable_Odf + SAL_N_ELEMENTS(saFuncTable_Odf));
    (this->*pFillFunc)(saFuncTable_OOoLO, saFuncTable_OOoLO + SAL_N_ELEMENTS(saFuncTable_OOoLO));
}

const XclFunctionInfo* XclFunctionProvider::GetFuncInfoFromXclFunc( sal_uInt16 nXclFunc ) const
{
    // only in import filter allowed
    OSL_ENSURE( !maXclFuncMap.empty(), "XclFunctionProvider::GetFuncInfoFromXclFunc - wrong filter" );
    XclFuncMap::const_iterator aIt = maXclFuncMap.find( nXclFunc );
    return (aIt == maXclFuncMap.end()) ? nullptr : aIt->second;
}

const XclFunctionInfo* XclFunctionProvider::GetFuncInfoFromXclMacroName( const OUString& rXclMacroName ) const
{
    // only in import filter allowed, but do not test maXclMacroNameMap, it may be empty for old BIFF versions
    OSL_ENSURE( !maXclFuncMap.empty(), "XclFunctionProvider::GetFuncInfoFromXclMacroName - wrong filter" );
    XclMacroNameMap::const_iterator aIt = maXclMacroNameMap.find( rXclMacroName );
    return (aIt == maXclMacroNameMap.end()) ? nullptr : aIt->second;
}

const XclFunctionInfo* XclFunctionProvider::GetFuncInfoFromOpCode( OpCode eOpCode ) const
{
    // only in export filter allowed
    OSL_ENSURE( !maScFuncMap.empty(), "XclFunctionProvider::GetFuncInfoFromOpCode - wrong filter" );
    ScFuncMap::const_iterator aIt = maScFuncMap.find( eOpCode );
    return (aIt == maScFuncMap.end()) ? nullptr : aIt->second;
}

void XclFunctionProvider::FillXclFuncMap( const XclFunctionInfo* pBeg, const XclFunctionInfo* pEnd )
{
    for( const XclFunctionInfo* pIt = pBeg; pIt != pEnd; ++pIt )
    {
        if( !::get_flag( pIt->mnFlags, EXC_FUNCFLAG_EXPORTONLY ) )
        {
            if( pIt->mnXclFunc != NOID )
                maXclFuncMap[ pIt->mnXclFunc ] = pIt;
            if( pIt->IsMacroFunc() )
                maXclMacroNameMap[ pIt->GetMacroFuncName() ] = pIt;
        }
    }
}

void XclFunctionProvider::FillScFuncMap( const XclFunctionInfo* pBeg, const XclFunctionInfo* pEnd )
{
    for( const XclFunctionInfo* pIt = pBeg; pIt != pEnd; ++pIt )
        if( !::get_flag( pIt->mnFlags, EXC_FUNCFLAG_IMPORTONLY ) )
            maScFuncMap[ pIt->meOpCode ] = pIt;
}

// Token array ================================================================

XclTokenArray::XclTokenArray( bool bVolatile ) :
    mbVolatile( bVolatile )
{
}

XclTokenArray::XclTokenArray( ScfUInt8Vec& rTokVec, ScfUInt8Vec& rExtDataVec, bool bVolatile ) :
    mbVolatile( bVolatile )
{
    maTokVec.swap( rTokVec );
    maExtDataVec.swap( rExtDataVec );
}

sal_uInt16 XclTokenArray::GetSize() const
{
    OSL_ENSURE( maTokVec.size() <= 0xFFFF, "XclTokenArray::GetSize - array too long" );
    return limit_cast< sal_uInt16 >( maTokVec.size() );
}

void XclTokenArray::ReadSize( XclImpStream& rStrm )
{
    sal_uInt16 nSize;
    nSize = rStrm.ReaduInt16();
    maTokVec.resize( nSize );
}

void XclTokenArray::ReadArray( XclImpStream& rStrm )
{
    if( !maTokVec.empty() )
        rStrm.Read(maTokVec.data(), GetSize());
}

void XclTokenArray::Read( XclImpStream& rStrm )
{
    ReadSize( rStrm );
    ReadArray( rStrm );
}

void XclTokenArray::WriteSize( XclExpStream& rStrm ) const
{
    rStrm << GetSize();
}

void XclTokenArray::WriteArray( XclExpStream& rStrm ) const
{
    if( !maTokVec.empty() )
        rStrm.Write(maTokVec.data(), GetSize());
    if( !maExtDataVec.empty() )
        rStrm.Write(maExtDataVec.data(), maExtDataVec.size());
}

void XclTokenArray::Write( XclExpStream& rStrm ) const
{
    WriteSize( rStrm );
    WriteArray( rStrm );
}

bool XclTokenArray::operator==( const XclTokenArray& rTokArr ) const
{
    return (mbVolatile == rTokArr.mbVolatile) && (maTokVec == rTokArr.maTokVec) && (maExtDataVec == rTokArr.maExtDataVec);
}

XclImpStream& operator>>( XclImpStream& rStrm, XclTokenArray& rTokArr )
{
    rTokArr.Read( rStrm );
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclTokenArray& rTokArr )
{
    rTokArr.Write( rStrm );
    return rStrm;
}

XclExpStream& operator<<( XclExpStream& rStrm, const XclTokenArrayRef& rxTokArr )
{
    if( rxTokArr )
        rxTokArr->Write( rStrm );
    else
        rStrm << sal_uInt16( 0 );
    return rStrm;
}

XclTokenArrayIterator::XclTokenArrayIterator() :
    mppScTokenBeg( nullptr ),
    mppScTokenEnd( nullptr ),
    mppScToken( nullptr ),
    mbSkipSpaces( false )
{
}

XclTokenArrayIterator::XclTokenArrayIterator( const ScTokenArray& rScTokArr, bool bSkipSpaces )
{
    Init( rScTokArr, bSkipSpaces );
}

XclTokenArrayIterator::XclTokenArrayIterator( const XclTokenArrayIterator& rTokArrIt, bool bSkipSpaces ) :
    mppScTokenBeg( rTokArrIt.mppScTokenBeg ),
    mppScTokenEnd( rTokArrIt.mppScTokenEnd ),
    mppScToken( rTokArrIt.mppScToken ),
    mbSkipSpaces( bSkipSpaces )
{
    SkipSpaces();
}

void XclTokenArrayIterator::Init( const ScTokenArray& rScTokArr, bool bSkipSpaces )
{
    sal_uInt16 nTokArrLen = rScTokArr.GetLen();
    mppScTokenBeg = static_cast< const FormulaToken* const* >( nTokArrLen ? rScTokArr.GetArray() : nullptr );
    mppScTokenEnd = mppScTokenBeg ? (mppScTokenBeg + nTokArrLen) : nullptr;
    mppScToken = (mppScTokenBeg != mppScTokenEnd) ? mppScTokenBeg : nullptr;
    mbSkipSpaces = bSkipSpaces;
    SkipSpaces();
}

XclTokenArrayIterator& XclTokenArrayIterator::operator++()
{
    NextRawToken();
    SkipSpaces();
    return *this;
}

void XclTokenArrayIterator::NextRawToken()
{
    if( mppScToken )
        if( (++mppScToken == mppScTokenEnd) || !*mppScToken )
            mppScToken = nullptr;
}

void XclTokenArrayIterator::SkipSpaces()
{
    if( mbSkipSpaces )
        while( Is() && ((*this)->GetOpCode() == ocSpaces) )
            NextRawToken();
}

// strings and string lists ---------------------------------------------------

bool XclTokenArrayHelper::GetTokenString( OUString& rString, const FormulaToken& rScToken )
{
    bool bIsStr = (rScToken.GetType() == svString) && (rScToken.GetOpCode() == ocPush);
    if( bIsStr ) rString = rScToken.GetString().getString();
    return bIsStr;
}

bool XclTokenArrayHelper::GetString( OUString& rString, const ScTokenArray& rScTokArr )
{
    XclTokenArrayIterator aIt( rScTokArr, true );
    // something is following the string token -> error
    return aIt.Is() && GetTokenString( rString, *aIt ) && !++aIt;
}

bool XclTokenArrayHelper::GetStringList( OUString& rStringList, const ScTokenArray& rScTokArr, sal_Unicode cSep )
{
    bool bRet = true;
    XclTokenArrayIterator aIt( rScTokArr, true );
    enum { STATE_START, STATE_STR, STATE_SEP, STATE_END } eState = STATE_START;
    while( eState != STATE_END ) switch( eState )
    {
        case STATE_START:
            eState = aIt.Is() ? STATE_STR : STATE_END;
        break;
        case STATE_STR:
        {
            OUString aString;
            bRet = GetTokenString( aString, *aIt );
            if( bRet ) rStringList += aString ;
            eState = (bRet && (++aIt).Is()) ? STATE_SEP : STATE_END;
            break;
        }
        case STATE_SEP:
            bRet = aIt->GetOpCode() == ocSep;
            if( bRet ) rStringList += OUStringLiteral1(cSep);
            eState = (bRet && (++aIt).Is()) ? STATE_STR : STATE_END;
        break;
        default:;
    }
    return bRet;
}

void XclTokenArrayHelper::ConvertStringToList(
    ScTokenArray& rScTokArr, svl::SharedStringPool& rSPool, sal_Unicode cStringSep )
{
    OUString aString;
    if( GetString( aString, rScTokArr ) )
    {
        rScTokArr.Clear();
        if (aString.isEmpty())
            return;
        sal_Int32 nStringIx = 0;
        for (;;)
        {
            OUString aToken( aString.getToken( 0, cStringSep, nStringIx ) );
            rScTokArr.AddString(rSPool.intern(comphelper::string::stripStart(aToken, ' ')));
            if (nStringIx<0)
                break;
            rScTokArr.AddOpCode( ocSep );
        }
    }
}

// multiple operations --------------------------------------------------------

namespace {

bool lclGetAddress( ScAddress& rAddress, const FormulaToken& rToken, const ScAddress& rPos )
{
    OpCode eOpCode = rToken.GetOpCode();
    bool bIsSingleRef = (eOpCode == ocPush) && (rToken.GetType() == svSingleRef);
    if( bIsSingleRef )
    {
        const ScSingleRefData& rRef = *rToken.GetSingleRef();
        rAddress = rRef.toAbs(rPos);
        bIsSingleRef = !rRef.IsDeleted();
    }
    return bIsSingleRef;
}

} // namespace

bool XclTokenArrayHelper::GetMultipleOpRefs(
    XclMultipleOpRefs& rRefs, const ScTokenArray& rScTokArr, const ScAddress& rScPos )
{
    rRefs.mbDblRefMode = false;
    enum
    {
        stBegin, stTableOp, stOpen, stFormula, stFormulaSep,
        stColFirst, stColFirstSep, stColRel, stColRelSep,
        stRowFirst, stRowFirstSep, stRowRel, stClose, stError
    } eState = stBegin;     // last read token
    for( XclTokenArrayIterator aIt( rScTokArr, true ); aIt.Is() && (eState != stError); ++aIt )
    {
        OpCode eOpCode = aIt->GetOpCode();
        bool bIsSep = eOpCode == ocSep;
        switch( eState )
        {
            case stBegin:
                eState = (eOpCode == ocTableOp) ? stTableOp : stError;
            break;
            case stTableOp:
                eState = (eOpCode == ocOpen) ? stOpen : stError;
            break;
            case stOpen:
                eState = lclGetAddress(rRefs.maFmlaScPos, *aIt, rScPos) ? stFormula : stError;
            break;
            case stFormula:
                eState = bIsSep ? stFormulaSep : stError;
            break;
            case stFormulaSep:
                eState = lclGetAddress(rRefs.maColFirstScPos, *aIt, rScPos) ? stColFirst : stError;
            break;
            case stColFirst:
                eState = bIsSep ? stColFirstSep : stError;
            break;
            case stColFirstSep:
                eState = lclGetAddress(rRefs.maColRelScPos, *aIt, rScPos) ? stColRel : stError;
            break;
            case stColRel:
                eState = bIsSep ? stColRelSep : ((eOpCode == ocClose) ? stClose : stError);
            break;
            case stColRelSep:
                eState = lclGetAddress(rRefs.maRowFirstScPos, *aIt, rScPos) ? stRowFirst : stError;
                rRefs.mbDblRefMode = true;
            break;
            case stRowFirst:
                eState = bIsSep ? stRowFirstSep : stError;
            break;
            case stRowFirstSep:
                eState = lclGetAddress(rRefs.maRowRelScPos, *aIt, rScPos) ? stRowRel : stError;
            break;
            case stRowRel:
                eState = (eOpCode == ocClose) ? stClose : stError;
            break;
            default:
                eState = stError;
        }
    }
    return eState == stClose;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
