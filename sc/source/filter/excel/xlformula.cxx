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
#include "xlformula.hxx"

#include "compiler.hxx"
#include "rangenam.hxx"
#include "token.hxx"
#include "tokenarray.hxx"
#include "xestream.hxx"
#include "xistream.hxx"
#include "xlroot.hxx"

using namespace ::formula;

// Function data ==============================================================

String XclFunctionInfo::GetMacroFuncName() const
{
    if( IsMacroFunc() )
        return String( mpcMacroName, RTL_TEXTENCODING_UTF8 );
    return EMPTY_STRING;
}

// abbreviations for function return token class
const sal_uInt8 R = EXC_TOKCLASS_REF;
const sal_uInt8 V = EXC_TOKCLASS_VAL;
const sal_uInt8 A = EXC_TOKCLASS_ARR;

// abbreviations for parameter infos
#define RO   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_ORG, false }
#define RV   { EXC_PARAM_REGULAR,   EXC_PARAMCONV_VAL, false }
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

/** Functions new in BIFF2. */
static const XclFunctionInfo saFuncTable_2[] =
{
    { ocCount,              0,      0,  MX, V, { RX }, 0, 0 },
    { ocIf,                 1,      2,  3,  R, { VO, RO }, 0, 0 },
    { ocIsNA,               2,      1,  1,  V, { VR }, 0, 0 },
    { ocIsError,            3,      1,  1,  V, { VR }, 0, 0 },
    { ocSum,                4,      0,  MX, V, { RX }, 0, 0 },
    { ocAverage,            5,      1,  MX, V, { RX }, 0, 0 },
    { ocMin,                6,      1,  MX, V, { RX }, 0, 0 },
    { ocMax,                7,      1,  MX, V, { RX }, 0, 0 },
    { ocRow,                8,      0,  1,  V, { RO }, 0, 0 },
    { ocColumn,             9,      0,  1,  V, { RO }, 0, 0 },
    { ocNotAvail,           10,     0,  0,  V, {}, 0, 0 },
    { ocNPV,                11,     2,  MX, V, { VR, RX }, 0, 0 },
    { ocStDev,              12,     1,  MX, V, { RX }, 0, 0 },
    { ocCurrency,           13,     1,  2,  V, { VR }, 0, 0 },
    { ocFixed,              14,     1,  2,  V, { VR, VR, C }, 0, 0 },
    { ocSin,                15,     1,  1,  V, { VR }, 0, 0 },
    { ocCos,                16,     1,  1,  V, { VR }, 0, 0 },
    { ocTan,                17,     1,  1,  V, { VR }, 0, 0 },
    { ocCot,                17,     1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocArcTan,             18,     1,  1,  V, { VR }, 0, 0 },
    { ocArcCot,             18,     1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocPi,                 19,     0,  0,  V, {}, 0, 0 },
    { ocSqrt,               20,     1,  1,  V, { VR }, 0, 0 },
    { ocExp,                21,     1,  1,  V, { VR }, 0, 0 },
    { ocLn,                 22,     1,  1,  V, { VR }, 0, 0 },
    { ocLog10,              23,     1,  1,  V, { VR }, 0, 0 },
    { ocAbs,                24,     1,  1,  V, { VR }, 0, 0 },
    { ocInt,                25,     1,  1,  V, { VR }, 0, 0 },
    { ocPlusMinus,          26,     1,  1,  V, { VR }, 0, 0 },
    { ocRound,              27,     2,  2,  V, { VR }, 0, 0 },
    { ocLookup,             28,     2,  3,  V, { VR, RA }, 0, 0 },
    { ocIndex,              29,     2,  4,  R, { RA, VV }, 0, 0 },
    { ocRept,               30,     2,  2,  V, { VR }, 0, 0 },
    { ocMid,                31,     3,  3,  V, { VR }, 0, 0 },
    { ocLen,                32,     1,  1,  V, { VR }, 0, 0 },
    { ocValue,              33,     1,  1,  V, { VR }, 0, 0 },
    { ocTrue,               34,     0,  0,  V, {}, 0, 0 },
    { ocFalse,              35,     0,  0,  V, {}, 0, 0 },
    { ocAnd,                36,     1,  MX, V, { RX }, 0, 0 },
    { ocOr,                 37,     1,  MX, V, { RX }, 0, 0 },
    { ocNot,                38,     1,  1,  V, { VR }, 0, 0 },
    { ocMod,                39,     2,  2,  V, { VR }, 0, 0 },
    { ocDBCount,            40,     3,  3,  V, { RO, RR }, 0, 0 },
    { ocDBSum,              41,     3,  3,  V, { RO, RR }, 0, 0 },
    { ocDBAverage,          42,     3,  3,  V, { RO, RR }, 0, 0 },
    { ocDBMin,              43,     3,  3,  V, { RO, RR }, 0, 0 },
    { ocDBMax,              44,     3,  3,  V, { RO, RR }, 0, 0 },
    { ocDBStdDev,           45,     3,  3,  V, { RO, RR }, 0, 0 },
    { ocVar,                46,     1,  MX, V, { RX }, 0, 0 },
    { ocDBVar,              47,     3,  3,  V, { RO, RR }, 0, 0 },
    { ocText,               48,     2,  2,  V, { VR }, 0, 0 },
    { ocRGP,                49,     1,  2,  A, { RA, RA, C, C }, 0, 0 },
    { ocTrend,              50,     1,  3,  A, { RA, RA, RA, C }, 0, 0 },
    { ocRKP,                51,     1,  2,  A, { RA, RA, C, C }, 0, 0 },
    { ocGrowth,             52,     1,  3,  A, { RA, RA, RA, C }, 0, 0 },
    { ocBW,                 56,     3,  5,  V, { VR }, 0, 0 },
    { ocZW,                 57,     3,  5,  V, { VR }, 0, 0 },
    { ocZZR,                58,     3,  5,  V, { VR }, 0, 0 },
    { ocRMZ,                59,     3,  5,  V, { VR }, 0, 0 },
    { ocZins,               60,     3,  6,  V, { VR }, 0, 0 },
    { ocMIRR,               61,     3,  3,  V, { RA, VR }, 0, 0 },
    { ocIRR,                62,     1,  2,  V, { RA, VR }, 0, 0 },
    { ocRandom,             63,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocMatch,              64,     2,  3,  V, { VR, RX, RR }, 0, 0 },
    { ocGetDate,            65,     3,  3,  V, { VR }, 0, 0 },
    { ocGetTime,            66,     3,  3,  V, { VR }, 0, 0 },
    { ocGetDay,             67,     1,  1,  V, { VR }, 0, 0 },
    { ocGetMonth,           68,     1,  1,  V, { VR }, 0, 0 },
    { ocGetYear,            69,     1,  1,  V, { VR }, 0, 0 },
    { ocGetDayOfWeek,       70,     1,  1,  V, { VR, C }, 0, 0 },
    { ocGetHour,            71,     1,  1,  V, { VR }, 0, 0 },
    { ocGetMin,             72,     1,  1,  V, { VR }, 0, 0 },
    { ocGetSec,             73,     1,  1,  V, { VR }, 0, 0 },
    { ocGetActTime,         74,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocAreas,              75,     1,  1,  V, { RO }, 0, 0 },
    { ocRows,               76,     1,  1,  V, { RO }, 0, 0 },
    { ocColumns,            77,     1,  1,  V, { RO }, 0, 0 },
    { ocOffset,             78,     3,  5,  R, { RO, VR }, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocSearch,             82,     2,  3,  V, { VR }, 0, 0 },
    { ocMatTrans,           83,     1,  1,  A, { VO }, 0, 0 },
    { ocType,               86,     1,  1,  V, { VX }, 0, 0 },
    { ocArcTan2,            97,     2,  2,  V, { VR }, 0, 0 },
    { ocArcSin,             98,     1,  1,  V, { VR }, 0, 0 },
    { ocArcCos,             99,     1,  1,  V, { VR }, 0, 0 },
    { ocChose,              100,    2,  MX, R, { VO, RO }, 0, 0 },
    { ocHLookup,            101,    3,  3,  V, { VV, RO, RO, C }, 0, 0 },
    { ocVLookup,            102,    3,  3,  V, { VV, RO, RO, C }, 0, 0 },
    { ocIsRef,              105,    1,  1,  V, { RX }, 0, 0 },
    { ocLog,                109,    1,  2,  V, { VR }, 0, 0 },
    { ocChar,               111,    1,  1,  V, { VR }, 0, 0 },
    { ocLower,              112,    1,  1,  V, { VR }, 0, 0 },
    { ocUpper,              113,    1,  1,  V, { VR }, 0, 0 },
    { ocPropper,            114,    1,  1,  V, { VR }, 0, 0 },
    { ocLeft,               115,    1,  2,  V, { VR }, 0, 0 },
    { ocRight,              116,    1,  2,  V, { VR }, 0, 0 },
    { ocExact,              117,    2,  2,  V, { VR }, 0, 0 },
    { ocTrim,               118,    1,  1,  V, { VR }, 0, 0 },
    { ocReplace,            119,    4,  4,  V, { VR }, 0, 0 },
    { ocSubstitute,         120,    3,  4,  V, { VR }, 0, 0 },
    { ocCode,               121,    1,  1,  V, { VR }, 0, 0 },
    { ocFind,               124,    2,  3,  V, { VR }, 0, 0 },
    { ocCell,               125,    1,  2,  V, { VV, RO }, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocIsErr,              126,    1,  1,  V, { VR }, 0, 0 },
    { ocIsString,           127,    1,  1,  V, { VR }, 0, 0 },
    { ocIsValue,            128,    1,  1,  V, { VR }, 0, 0 },
    { ocIsEmpty,            129,    1,  1,  V, { VR }, 0, 0 },
    { ocT,                  130,    1,  1,  V, { RO }, 0, 0 },
    { ocN,                  131,    1,  1,  V, { RO }, 0, 0 },
    { ocGetDateValue,       140,    1,  1,  V, { VR }, 0, 0 },
    { ocGetTimeValue,       141,    1,  1,  V, { VR }, 0, 0 },
    { ocLIA,                142,    3,  3,  V, { VR }, 0, 0 },
    { ocDIA,                143,    4,  4,  V, { VR }, 0, 0 },
    { ocGDA,                144,    4,  5,  V, { VR }, 0, 0 },
    { ocIndirect,           148,    1,  2,  R, { VR }, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocClean,              162,    1,  1,  V, { VR }, 0, 0 },
    { ocMatDet,             163,    1,  1,  V, { VA }, 0, 0 },
    { ocMatInv,             164,    1,  1,  A, { VA }, 0, 0 },
    { ocMatMult,            165,    2,  2,  A, { VA }, 0, 0 },
    { ocZinsZ,              167,    4,  6,  V, { VR }, 0, 0 },
    { ocKapz,               168,    4,  6,  V, { VR }, 0, 0 },
    { ocCount2,             169,    0,  MX, V, { RX }, 0, 0 },
    { ocProduct,            183,    0,  MX, V, { RX }, 0, 0 },
    { ocFact,               184,    1,  1,  V, { VR }, 0, 0 },
    { ocDBProduct,          189,    3,  3,  V, { RO, RR }, 0, 0 },
    { ocIsNonString,        190,    1,  1,  V, { VR }, 0, 0 },
    { ocStDevP,             193,    1,  MX, V, { RX }, 0, 0 },
    { ocVarP,               194,    1,  MX, V, { RX }, 0, 0 },
    { ocDBStdDevP,          195,    3,  3,  V, { RO, RR }, 0, 0 },
    { ocDBVarP,             196,    3,  3,  V, { RO, RR }, 0, 0 },
    { ocTrunc,              197,    1,  1,  V, { VR, C }, 0, 0 },
    { ocIsLogical,          198,    1,  1,  V, { VR }, 0, 0 },
    { ocDBCount2,           199,    3,  3,  V, { RO, RR }, 0, 0 },
    { ocCurrency,           204,    1,  2,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, 0 },
    { ocRoundUp,            212,    2,  2,  V, { VR }, 0, 0 },
    { ocRoundDown,          213,    2,  2,  V, { VR }, 0, 0 },
    { ocExternal,           255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_IMPORTONLY, 0 }
};

/** Functions new in BIFF3. */
static const XclFunctionInfo saFuncTable_3[] =
{
    { ocRGP,                49,     1,  4,  A, { RA, RA, VV }, 0, 0 },          // BIFF2: 1-2, BIFF3: 1-4
    { ocTrend,              50,     1,  4,  A, { RA, RA, RA, VV }, 0, 0 },      // BIFF2: 1-3, BIFF3: 1-4
    { ocRKP,                51,     1,  4,  A, { RA, RA, VV }, 0, 0 },          // BIFF2: 1-2, BIFF3: 1-4
    { ocGrowth,             52,     1,  4,  A, { RA, RA, RA, VV }, 0, 0 },      // BIFF2: 1-3, BIFF3: 1-4
    { ocTrunc,              197,    1,  2,  V, { VR }, 0, 0 },                  // BIFF2: 1,   BIFF3: 1-2
    { ocAddress,            219,    2,  5,  V, { VR }, 0, 0 },
    { ocGetDiffDate360,     220,    2,  2,  V, { VR, VR, C }, 0, 0 },
    { ocGetActDate,         221,    0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocVBD,                222,    5,  7,  V, { VR }, 0, 0 },
    { ocMedian,             227,    1,  MX, V, { RX }, 0, 0 },
    { ocSumProduct,         228,    1,  MX, V, { VA }, 0, 0 },
    { ocSinHyp,             229,    1,  1,  V, { VR }, 0, 0 },
    { ocCosHyp,             230,    1,  1,  V, { VR }, 0, 0 },
    { ocTanHyp,             231,    1,  1,  V, { VR }, 0, 0 },
    { ocCotHyp,             231,    1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocArcSinHyp,          232,    1,  1,  V, { VR }, 0, 0 },
    { ocArcCosHyp,          233,    1,  1,  V, { VR }, 0, 0 },
    { ocArcTanHyp,          234,    1,  1,  V, { VR }, 0, 0 },
    { ocArcCotHyp,          234,    1,  1,  V, { VR }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocDBGet,              235,    3,  3,  V, { RO, RR }, 0, 0 },
    { ocInfo,               244,    1,  1,  V, { VR }, EXC_FUNCFLAG_VOLATILE, 0 }
};

/** Functions new in BIFF4. */
static const XclFunctionInfo saFuncTable_4[] =
{
    { ocFixed,              14,     1,  3,  V, { VR }, 0, 0 },                  // BIFF2-3: 1-2, BIFF4: 1-3
    { ocAsc,                214,    1,  1,  V, { VR }, 0, 0 },
    { ocJis,                215,    1,  1,  V, { VR }, 0, 0 },
    { ocRank,               216,    2,  3,  V, { VR, RO, VR }, 0, 0 },
    { ocGDA2,               247,    4,  5,  V, { VR }, 0, 0 },
    { ocFrequency,          252,    2,  2,  A, { RA }, 0, 0 },
    { ocErrorType,          261,    1,  1,  V, { VR }, 0, 0 },
    { ocAveDev,             269,    1,  MX, V, { RX }, 0, 0 },
    { ocBetaDist,           270,    3,  5,  V, { VR }, 0, 0 },
    { ocGammaLn,            271,    1,  1,  V, { VR }, 0, 0 },
    { ocBetaInv,            272,    3,  5,  V, { VR }, 0, 0 },
    { ocBinomDist,          273,    4,  4,  V, { VR }, 0, 0 },
    { ocChiDist,            274,    2,  2,  V, { VR }, 0, 0 },
    { ocChiInv,             275,    2,  2,  V, { VR }, 0, 0 },
    { ocKombin,             276,    2,  2,  V, { VR }, 0, 0 },
    { ocConfidence,         277,    3,  3,  V, { VR }, 0, 0 },
    { ocKritBinom,          278,    3,  3,  V, { VR }, 0, 0 },
    { ocEven,               279,    1,  1,  V, { VR }, 0, 0 },
    { ocExpDist,            280,    3,  3,  V, { VR }, 0, 0 },
    { ocFDist,              281,    3,  3,  V, { VR }, 0, 0 },
    { ocFInv,               282,    3,  3,  V, { VR }, 0, 0 },
    { ocFisher,             283,    1,  1,  V, { VR }, 0, 0 },
    { ocFisherInv,          284,    1,  1,  V, { VR }, 0, 0 },
    { ocFloor,              285,    2,  2,  V, { VR, VR, C }, 0, 0 },
    { ocGammaDist,          286,    4,  4,  V, { VR }, 0, 0 },
    { ocGammaInv,           287,    3,  3,  V, { VR }, 0, 0 },
    { ocCeil,               288,    2,  2,  V, { VR, VR, C }, 0, 0 },
    { ocHypGeomDist,        289,    4,  4,  V, { VR }, 0, 0 },
    { ocLogNormDist,        290,    3,  3,  V, { VR }, 0, 0 },
    { ocLogInv,             291,    3,  3,  V, { VR }, 0, 0 },
    { ocNegBinomVert,       292,    3,  3,  V, { VR }, 0, 0 },
    { ocNormDist,           293,    4,  4,  V, { VR }, 0, 0 },
    { ocStdNormDist,        294,    1,  1,  V, { VR }, 0, 0 },
    { ocNormInv,            295,    3,  3,  V, { VR }, 0, 0 },
    { ocSNormInv,           296,    1,  1,  V, { VR }, 0, 0 },
    { ocStandard,           297,    3,  3,  V, { VR }, 0, 0 },
    { ocOdd,                298,    1,  1,  V, { VR }, 0, 0 },
    { ocVariationen,        299,    2,  2,  V, { VR }, 0, 0 },
    { ocPoissonDist,        300,    3,  3,  V, { VR }, 0, 0 },
    { ocTDist,              301,    3,  3,  V, { VR }, 0, 0 },
    { ocWeibull,            302,    4,  4,  V, { VR }, 0, 0 },
    { ocSumXMY2,            303,    2,  2,  V, { VA }, 0, 0 },
    { ocSumX2MY2,           304,    2,  2,  V, { VA }, 0, 0 },
    { ocSumX2DY2,           305,    2,  2,  V, { VA }, 0, 0 },
    { ocChiTest,            306,    2,  2,  V, { VA }, 0, 0 },
    { ocCorrel,             307,    2,  2,  V, { VA }, 0, 0 },
    { ocCovar,              308,    2,  2,  V, { VA }, 0, 0 },
    { ocForecast,           309,    3,  3,  V, { VR, VA }, 0, 0 },
    { ocFTest,              310,    2,  2,  V, { VA }, 0, 0 },
    { ocIntercept,          311,    2,  2,  V, { VA }, 0, 0 },
    { ocPearson,            312,    2,  2,  V, { VA }, 0, 0 },
    { ocRSQ,                313,    2,  2,  V, { VA }, 0, 0 },
    { ocSTEYX,              314,    2,  2,  V, { VA }, 0, 0 },
    { ocSlope,              315,    2,  2,  V, { VA }, 0, 0 },
    { ocTTest,              316,    4,  4,  V, { VA, VA, VR }, 0, 0 },
    { ocProb,               317,    3,  4,  V, { VA, VA, VR }, 0, 0 },
    { ocDevSq,              318,    1,  MX, V, { RX }, 0, 0 },
    { ocGeoMean,            319,    1,  MX, V, { RX }, 0, 0 },
    { ocHarMean,            320,    1,  MX, V, { RX }, 0, 0 },
    { ocSumSQ,              321,    0,  MX, V, { RX }, 0, 0 },
    { ocKurt,               322,    1,  MX, V, { RX }, 0, 0 },
    { ocSchiefe,            323,    1,  MX, V, { RX }, 0, 0 },
    { ocZTest,              324,    2,  3,  V, { RX, VR }, 0, 0 },
    { ocLarge,              325,    2,  2,  V, { RX, VR }, 0, 0 },
    { ocSmall,              326,    2,  2,  V, { RX, VR }, 0, 0 },
    { ocQuartile,           327,    2,  2,  V, { RX, VR }, 0, 0 },
    { ocPercentile,         328,    2,  2,  V, { RX, VR }, 0, 0 },
    { ocPercentrank,        329,    2,  3,  V, { RX, VR, VR_E }, 0, 0 },
    { ocModalValue,         330,    1,  MX, V, { VA }, 0, 0 },
    { ocTrimMean,           331,    2,  2,  V, { RX, VR }, 0, 0 },
    { ocTInv,               332,    2,  2,  V, { VR }, 0, 0 }
};

/** Functions new in BIFF5/BIFF7. Unsupported functions: DATEDIF, DATESTRING, NUMBERSTRING. */
static const XclFunctionInfo saFuncTable_5[] =
{
    { ocGetDayOfWeek,       70,     1,  2,  V, { VR }, 0, 0 },                  // BIFF2-4: 1, BIFF5: 1-2
    { ocHLookup,            101,    3,  4,  V, { VV, RO, RO, VV }, 0, 0 },      // BIFF2-4: 3, BIFF5: 3-4
    { ocVLookup,            102,    3,  4,  V, { VV, RO, RO, VV }, 0, 0 },      // BIFF2-4: 3, BIFF5: 3-4
    { ocGetDiffDate360,     220,    2,  3,  V, { VR }, 0, 0 },                  // BIFF3-4: 2, BIFF5: 2-3
    { ocMacro,              255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocExternal,           255,    1,  MX, R, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocConcat,             336,    0,  MX, V, { VR }, 0, 0 },
    { ocPower,              337,    2,  2,  V, { VR }, 0, 0 },
    { ocRad,                342,    1,  1,  V, { VR }, 0, 0 },
    { ocDeg,                343,    1,  1,  V, { VR }, 0, 0 },
    { ocSubTotal,           344,    2,  MX, V, { VR, RO }, 0, 0 },
    { ocSumIf,              345,    2,  3,  V, { RO, VR, RO }, 0, 0 },
    { ocCountIf,            346,    2,  2,  V, { RO, VR }, 0, 0 },
    { ocCountEmptyCells,    347,    1,  1,  V, { RO }, 0, 0 },
    { ocISPMT,              350,    4,  4,  V, { VR }, 0, 0 },
    { ocNoName,             351,    3,  3,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // DATEDIF
    { ocNoName,             352,    1,  1,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // DATESTRING
    { ocNoName,             353,    2,  2,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // NUMBERSTRING
    { ocRoman,              354,    1,  2,  V, { VR }, 0, 0 }
};

/** Functions new in BIFF8. Unsupported functions: PHONETIC. */
static const XclFunctionInfo saFuncTable_8[] =
{
    { ocGetPivotData,       358,    2,  MX, V, { RR, RR, VR }, 0, 0 },
    { ocHyperLink,          359,    1,  2,  V, { VV, VO }, 0, 0 },
    { ocNoName,             360,    1,  1,  V, { RO }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // PHONETIC
    { ocAverageA,           361,    1,  MX, V, { RX }, 0, 0 },
    { ocMaxA,               362,    1,  MX, V, { RX }, 0, 0 },
    { ocMinA,               363,    1,  MX, V, { RX }, 0, 0 },
    { ocStDevPA,            364,    1,  MX, V, { RX }, 0, 0 },
    { ocVarPA,              365,    1,  MX, V, { RX }, 0, 0 },
    { ocStDevA,             366,    1,  MX, V, { RX }, 0, 0 },
    { ocVarA,               367,    1,  MX, V, { RX }, 0, 0 },
    { ocBahtText,           368,    1,  1,  V, { VR }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME( "BAHTTEXT" ) },
    { ocBahtText,           255,    2,  2,  V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME( "BAHTTEXT" ) },
    { ocEuroConvert,        255,    4,  6,  V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY, "EUROCONVERT" }
};

#define EXC_FUNCENTRY_ODF( opcode, minparam, maxparam, flags, asciiname ) \
    { opcode, NOID, minparam,     maxparam,     V, { VR },       EXC_FUNCFLAG_IMPORTONLY|(flags), EXC_FUNCNAME_ODF( asciiname ) }, \
    { opcode,  255, (minparam)+1, (maxparam)+1, V, { RO_E, RO }, EXC_FUNCFLAG_EXPORTONLY|(flags), EXC_FUNCNAME_ODF( asciiname ) }

/** Functions defined by OpenFormula, but not supported by Calc (ocNoName) or by Excel (defined op-code). */
static const XclFunctionInfo saFuncTable_Odf[] =
{
    EXC_FUNCENTRY_ODF( ocArabic,        1,  1,  0,  "ARABIC" ),
    EXC_FUNCENTRY_ODF( ocB,             3,  4,  0,  "B" ),
    EXC_FUNCENTRY_ODF( ocBase,          2,  3,  0,  "BASE" ),
    EXC_FUNCENTRY_ODF( ocNoName,        2,  2,  0,  "BITAND" ),
    EXC_FUNCENTRY_ODF( ocNoName,        2,  2,  0,  "BITLSHIFT" ),
    EXC_FUNCENTRY_ODF( ocNoName,        2,  2,  0,  "BITOR" ),
    EXC_FUNCENTRY_ODF( ocNoName,        2,  2,  0,  "BITRSHIFT" ),
    EXC_FUNCENTRY_ODF( ocNoName,        2,  2,  0,  "BITXOR" ),
    EXC_FUNCENTRY_ODF( ocChiSqDist,     2,  3,  0,  "CHISQDIST" ),
    EXC_FUNCENTRY_ODF( ocChiSqInv,      2,  2,  0,  "CHISQINV" ),
    EXC_FUNCENTRY_ODF( ocKombin2,       2,  2,  0,  "COMBINA" ),
    EXC_FUNCENTRY_ODF( ocGetDiffDate,   2,  2,  0,  "DAYS" ),
    EXC_FUNCENTRY_ODF( ocDecimal,       2,  2,  0,  "DECIMAL" ),
    EXC_FUNCENTRY_ODF( ocFDist,         3,  4,  0,  "FDIST" ),
    EXC_FUNCENTRY_ODF( ocFInv,          3,  3,  0,  "FINV" ),
    EXC_FUNCENTRY_ODF( ocFormula,       1,  1,  0,  "FORMULA" ),
    EXC_FUNCENTRY_ODF( ocGamma,         1,  1,  0,  "GAMMA" ),
    EXC_FUNCENTRY_ODF( ocGauss,         1,  1,  0,  "GAUSS" ),
    EXC_FUNCENTRY_ODF( ocNoName,        2,  2,  0,  "IFNA" ),
    EXC_FUNCENTRY_ODF( ocIsFormula,     1,  1,  0,  "ISFORMULA" ),
    EXC_FUNCENTRY_ODF( ocWeek,          1,  2,  0,  "ISOWEEKNUM" ),
    EXC_FUNCENTRY_ODF( ocMatrixUnit,    1,  1,  0,  "MUNIT" ),
    EXC_FUNCENTRY_ODF( ocNumberValue,   2,  2,  0,  "NUMBERVALUE" ),
    EXC_FUNCENTRY_ODF( ocLaufz,         3,  3,  0,  "PDURATION" ),
    EXC_FUNCENTRY_ODF( ocVariationen2,  2,  2,  0,  "PERMUTATIONA" ),
    EXC_FUNCENTRY_ODF( ocPhi,           1,  1,  0,  "PHI" ),
    EXC_FUNCENTRY_ODF( ocZGZ,           3,  3,  0,  "RRI" ),
    EXC_FUNCENTRY_ODF( ocTable,         1,  1,  0,  "SHEET" ),
    EXC_FUNCENTRY_ODF( ocTables,        0,  1,  0,  "SHEETS" ),
    EXC_FUNCENTRY_ODF( ocNoName,        1,  MX, 0,  "SKEWP" ),
    EXC_FUNCENTRY_ODF( ocUnichar,       1,  1,  0,  "UNICHAR" ),
    EXC_FUNCENTRY_ODF( ocUnicode,       1,  1,  0,  "UNICODE" ),
    EXC_FUNCENTRY_ODF( ocNoName,        1,  MX, 0,  "XOR" )
};

#undef EXC_FUNCENTRY_ODF

// ----------------------------------------------------------------------------

XclFunctionProvider::XclFunctionProvider( const XclRoot& rRoot )
{
    void (XclFunctionProvider::*pFillFunc)( const XclFunctionInfo*, const XclFunctionInfo* ) =
        rRoot.IsImport() ? &XclFunctionProvider::FillXclFuncMap : &XclFunctionProvider::FillScFuncMap;

    /*  Only read/write functions supported in the current BIFF version.
        Function tables from later BIFF versions may overwrite single functions
        from earlier tables. */
    XclBiff eBiff = rRoot.GetBiff();
    if( eBiff >= EXC_BIFF2 )
        (this->*pFillFunc)( saFuncTable_2, STATIC_TABLE_END( saFuncTable_2 ) );
    if( eBiff >= EXC_BIFF3 )
        (this->*pFillFunc)( saFuncTable_3, STATIC_TABLE_END( saFuncTable_3 ) );
    if( eBiff >= EXC_BIFF4 )
        (this->*pFillFunc)( saFuncTable_4, STATIC_TABLE_END( saFuncTable_4 ) );
    if( eBiff >= EXC_BIFF5 )
        (this->*pFillFunc)( saFuncTable_5, STATIC_TABLE_END( saFuncTable_5 ) );
    if( eBiff >= EXC_BIFF8 )
        (this->*pFillFunc)( saFuncTable_8, STATIC_TABLE_END( saFuncTable_8 ) );
    (this->*pFillFunc)( saFuncTable_Odf, STATIC_TABLE_END( saFuncTable_Odf ) );
}

const XclFunctionInfo* XclFunctionProvider::GetFuncInfoFromXclFunc( sal_uInt16 nXclFunc ) const
{
    // only in import filter allowed
    DBG_ASSERT( !maXclFuncMap.empty(), "XclFunctionProvider::GetFuncInfoFromXclFunc - wrong filter" );
    XclFuncMap::const_iterator aIt = maXclFuncMap.find( nXclFunc );
    return (aIt == maXclFuncMap.end()) ? 0 : aIt->second;
}

const XclFunctionInfo* XclFunctionProvider::GetFuncInfoFromXclMacroName( const String& rXclMacroName ) const
{
    // only in import filter allowed, but do not test maXclMacroNameMap, it may be empty for old BIFF versions
    DBG_ASSERT( !maXclFuncMap.empty(), "XclFunctionProvider::GetFuncInfoFromXclMacroName - wrong filter" );
    XclMacroNameMap::const_iterator aIt = maXclMacroNameMap.find( rXclMacroName );
    return (aIt == maXclMacroNameMap.end()) ? 0 : aIt->second;
}

const XclFunctionInfo* XclFunctionProvider::GetFuncInfoFromOpCode( OpCode eOpCode ) const
{
    // only in export filter allowed
    DBG_ASSERT( !maScFuncMap.empty(), "XclFunctionProvider::GetFuncInfoFromOpCode - wrong filter" );
    ScFuncMap::const_iterator aIt = maScFuncMap.find( eOpCode );
    return (aIt == maScFuncMap.end()) ? 0 : aIt->second;
}

void XclFunctionProvider::FillXclFuncMap( const XclFunctionInfo* pBeg, const XclFunctionInfo* pEnd )
{
    for( const XclFunctionInfo* pIt = pBeg; pIt != pEnd; ++pIt )
    {
        if( !::get_flag( pIt->mnFlags, EXC_FUNCFLAG_EXPORTONLY ) )
        {
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

XclTokenArray::XclTokenArray( ScfUInt8Vec& rTokVec, bool bVolatile ) :
    mbVolatile( bVolatile )
{
    maTokVec.swap( rTokVec );
}

XclTokenArray::XclTokenArray( ScfUInt8Vec& rTokVec, ScfUInt8Vec& rExtDataVec, bool bVolatile ) :
    mbVolatile( bVolatile )
{
    maTokVec.swap( rTokVec );
    maExtDataVec.swap( rExtDataVec );
}

sal_uInt16 XclTokenArray::GetSize() const
{
    DBG_ASSERT( maTokVec.size() <= 0xFFFF, "XclTokenArray::GetSize - array too long" );
    return limit_cast< sal_uInt16 >( maTokVec.size() );
}

void XclTokenArray::ReadSize( XclImpStream& rStrm )
{
    sal_uInt16 nSize;
    rStrm >> nSize;
    maTokVec.resize( nSize );
}

void XclTokenArray::ReadArray( XclImpStream& rStrm )
{
    if( !maTokVec.empty() )
        rStrm.Read( &maTokVec.front(), GetSize() );
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
        rStrm.Write( &maTokVec.front(), GetSize() );
    if( !maExtDataVec.empty() )
        rStrm.Write( &maExtDataVec.front(), maExtDataVec.size() );
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

XclImpStream& operator>>( XclImpStream& rStrm, XclTokenArrayRef& rxTokArr )
{
    if( !rxTokArr )
        rxTokArr.reset( new XclTokenArray );
    rxTokArr->Read( rStrm );
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

// ----------------------------------------------------------------------------

XclTokenArrayIterator::XclTokenArrayIterator() :
    mppScTokenBeg( 0 ),
    mppScTokenEnd( 0 ),
    mppScToken( 0 ),
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

void XclTokenArrayIterator::Init()
{
    mppScTokenBeg = mppScTokenEnd = mppScToken = 0;
}

void XclTokenArrayIterator::Init( const ScTokenArray& rScTokArr, bool bSkipSpaces )
{
    USHORT nTokArrLen = rScTokArr.GetLen();
    mppScTokenBeg = static_cast< const FormulaToken* const* >( nTokArrLen ? rScTokArr.GetArray() : 0 );
    mppScTokenEnd = mppScTokenBeg ? (mppScTokenBeg + nTokArrLen) : 0;
    mppScToken = (mppScTokenBeg != mppScTokenEnd) ? mppScTokenBeg : 0;
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
            mppScToken = 0;
}

void XclTokenArrayIterator::SkipSpaces()
{
    if( mbSkipSpaces )
        while( Is() && ((*this)->GetOpCode() == ocSpaces) )
            NextRawToken();
}

// strings and string lists ---------------------------------------------------

bool XclTokenArrayHelper::GetTokenString( String& rString, const FormulaToken& rScToken )
{
    bool bIsStr = (rScToken.GetType() == svString) && (rScToken.GetOpCode() == ocPush);
    if( bIsStr ) rString = rScToken.GetString();
    return bIsStr;
}

bool XclTokenArrayHelper::GetString( String& rString, const ScTokenArray& rScTokArr )
{
    XclTokenArrayIterator aIt( rScTokArr, true );
    // something is following the string token -> error
    return aIt.Is() && GetTokenString( rString, *aIt ) && !++aIt;
}

bool XclTokenArrayHelper::GetStringList( String& rStringList, const ScTokenArray& rScTokArr, sal_Unicode cSep )
{
    bool bRet = true;
    String aString;
    XclTokenArrayIterator aIt( rScTokArr, true );
    enum { STATE_START, STATE_STR, STATE_SEP, STATE_END } eState = STATE_START;
    while( eState != STATE_END ) switch( eState )
    {
        case STATE_START:
            eState = aIt.Is() ? STATE_STR : STATE_END;
        break;
        case STATE_STR:
            bRet = GetTokenString( aString, *aIt );
            if( bRet ) rStringList.Append( aString );
            eState = (bRet && (++aIt).Is()) ? STATE_SEP : STATE_END;
        break;
        case STATE_SEP:
            bRet = aIt->GetOpCode() == ocSep;
            if( bRet ) rStringList.Append( cSep );
            eState = (bRet && (++aIt).Is()) ? STATE_STR : STATE_END;
        break;
        default:;
    }
    return bRet;
}

void XclTokenArrayHelper::ConvertStringToList( ScTokenArray& rScTokArr, sal_Unicode cStringSep, bool bTrimLeadingSpaces )
{
    String aString;
    if( GetString( aString, rScTokArr ) )
    {
        rScTokArr.Clear();
        xub_StrLen nTokenCnt = aString.GetTokenCount( cStringSep );
        xub_StrLen nStringIx = 0;
        for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
        {
            String aToken( aString.GetToken( 0, cStringSep, nStringIx ) );
            if( bTrimLeadingSpaces )
                aToken.EraseLeadingChars( ' ' );
            if( nToken > 0 )
                rScTokArr.AddOpCode( ocSep );
            rScTokArr.AddString( aToken );
        }
    }
}

// shared formulas ------------------------------------------------------------

const ScTokenArray* XclTokenArrayHelper::GetSharedFormula( const XclRoot& rRoot, const ScTokenArray& rScTokArr )
{
    if( rScTokArr.GetLen() == 1 )
        if( const FormulaToken* pScToken = rScTokArr.GetArray()[ 0 ] )
            if( pScToken->GetOpCode() == ocName )
                if( ScRangeData* pData = rRoot.GetNamedRanges().findByIndex( pScToken->GetIndex() ) )
                    if( pData->HasType( RT_SHARED ) )
                        return pData->GetCode();
    return 0;
}

// multiple operations --------------------------------------------------------

namespace {

inline bool lclGetAddress( ScAddress& rAddress, const FormulaToken& rToken )
{
    OpCode eOpCode = rToken.GetOpCode();
    bool bIsSingleRef = (eOpCode == ocPush) && (rToken.GetType() == svSingleRef);
    if( bIsSingleRef )
    {
        const ScSingleRefData& rRef = static_cast<const ScToken&>(rToken).GetSingleRef();
        rAddress.Set( rRef.nCol, rRef.nRow, rRef.nTab );
        bIsSingleRef = !rRef.IsDeleted();
    }
    return bIsSingleRef;
}

} // namespace

bool XclTokenArrayHelper::GetMultipleOpRefs( XclMultipleOpRefs& rRefs, const ScTokenArray& rScTokArr )
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
                eState = lclGetAddress( rRefs.maFmlaScPos, *aIt ) ? stFormula : stError;
            break;
            case stFormula:
                eState = bIsSep ? stFormulaSep : stError;
            break;
            case stFormulaSep:
                eState = lclGetAddress( rRefs.maColFirstScPos, *aIt ) ? stColFirst : stError;
            break;
            case stColFirst:
                eState = bIsSep ? stColFirstSep : stError;
            break;
            case stColFirstSep:
                eState = lclGetAddress( rRefs.maColRelScPos, *aIt ) ? stColRel : stError;
            break;
            case stColRel:
                eState = bIsSep ? stColRelSep : ((eOpCode == ocClose) ? stClose : stError);
            break;
            case stColRelSep:
                eState = lclGetAddress( rRefs.maRowFirstScPos, *aIt ) ? stRowFirst : stError;
                rRefs.mbDblRefMode = true;
            break;
            case stRowFirst:
                eState = bIsSep ? stRowFirstSep : stError;
            break;
            case stRowFirstSep:
                eState = lclGetAddress( rRefs.maRowRelScPos, *aIt ) ? stRowRel : stError;
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

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
