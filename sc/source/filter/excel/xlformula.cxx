/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlformula.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:48:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif
#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif

// Function data ==============================================================

String XclFunctionInfo::GetMacroFuncName() const
{
    if( IsMacroFunc() )
        return String( mpcMacroName, RTL_TEXTENCODING_UTF8 );
    return EMPTY_STRING;
}

const sal_uInt8 R = EXC_TOKCLASS_REF;
const sal_uInt8 V = EXC_TOKCLASS_VAL;
const sal_uInt8 A = EXC_TOKCLASS_ARR;
const sal_uInt8 C = EXC_FUNC_PAR_CALCONLY;
const sal_uInt8 E = EXC_FUNC_PAR_EXCELONLY;
const sal_uInt8 I = EXC_FUNC_PAR_INVALID;

/** Functions new in BIFF2. */
static const XclFunctionInfo saFuncTable_2[] =
{
    { ocCount,              0,      0,  30, V, { R }, 0, 0 },
    { ocIf,                 1,      2,  3,  R, { V, R }, 0, 0 },
    { ocIsNA,               2,      1,  1,  V, { V }, 0, 0 },
    { ocIsError,            3,      1,  1,  V, { V }, 0, 0 },
    { ocSum,                4,      0,  30, V, { R }, 0, 0 },
    { ocAverage,            5,      1,  30, V, { R }, 0, 0 },
    { ocMin,                6,      1,  30, V, { R }, 0, 0 },
    { ocMax,                7,      1,  30, V, { R }, 0, 0 },
    { ocRow,                8,      0,  1,  V, { R }, 0, 0 },
    { ocColumn,             9,      0,  1,  V, { R }, 0, 0 },
    { ocNotAvail,           10,     0,  0,  V, {}, 0, 0 },
    { ocNPV,                11,     2,  30, V, { V, R }, 0, 0 },
    { ocStDev,              12,     1,  30, V, { R }, 0, 0 },
    { ocCurrency,           13,     1,  2,  V, { V }, 0, 0 },
    { ocFixed,              14,     1,  2,  V, { V, V, C, I }, 0, 0 },
    { ocSin,                15,     1,  1,  V, { V }, 0, 0 },
    { ocCos,                16,     1,  1,  V, { V }, 0, 0 },
    { ocTan,                17,     1,  1,  V, { V }, 0, 0 },
    { ocCot,                17,     1,  1,  V, { V }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocArcTan,             18,     1,  1,  V, { V }, 0, 0 },
    { ocArcCot,             18,     1,  1,  V, { V }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocPi,                 19,     0,  0,  V, {}, 0, 0 },
    { ocSqrt,               20,     1,  1,  V, { V }, 0, 0 },
    { ocExp,                21,     1,  1,  V, { V }, 0, 0 },
    { ocLn,                 22,     1,  1,  V, { V }, 0, 0 },
    { ocLog10,              23,     1,  1,  V, { V }, 0, 0 },
    { ocAbs,                24,     1,  1,  V, { V }, 0, 0 },
    { ocInt,                25,     1,  1,  V, { V }, 0, 0 },
    { ocPlusMinus,          26,     1,  1,  V, { V }, 0, 0 },
    { ocRound,              27,     2,  2,  V, { V }, 0, 0 },
    { ocLookup,             28,     2,  3,  V, { V, R }, 0, 0 },
    { ocIndex,              29,     2,  4,  R, { R, V }, 0, 0 },
    { ocRept,               30,     2,  2,  V, { V }, 0, 0 },
    { ocMid,                31,     3,  3,  V, { V }, 0, 0 },
    { ocLen,                32,     1,  1,  V, { V }, 0, 0 },
    { ocValue,              33,     1,  1,  V, { V }, 0, 0 },
    { ocTrue,               34,     0,  0,  V, {}, 0, 0 },
    { ocFalse,              35,     0,  0,  V, {}, 0, 0 },
    { ocAnd,                36,     1,  30, V, { R }, 0, 0 },
    { ocOr,                 37,     1,  30, V, { R }, 0, 0 },
    { ocNot,                38,     1,  1,  V, { V }, 0, 0 },
    { ocMod,                39,     2,  2,  V, { V }, 0, 0 },
    { ocDBCount,            40,     3,  3,  V, { R }, 0, 0 },
    { ocDBSum,              41,     3,  3,  V, { R }, 0, 0 },
    { ocDBAverage,          42,     3,  3,  V, { R }, 0, 0 },
    { ocDBMin,              43,     3,  3,  V, { R }, 0, 0 },
    { ocDBMax,              44,     3,  3,  V, { R }, 0, 0 },
    { ocDBStdDev,           45,     3,  3,  V, { R }, 0, 0 },
    { ocVar,                46,     1,  30, V, { R }, 0, 0 },
    { ocDBVar,              47,     3,  3,  V, { R }, 0, 0 },
    { ocText,               48,     2,  2,  V, { V }, 0, 0 },
    { ocRGP,                49,     1,  2,  A, { R, R, C, C, I }, 0, 0 },
    { ocTrend,              50,     1,  3,  A, { R, R, R, C, I }, 0, 0 },
    { ocRKP,                51,     1,  2,  A, { R, R, C, C, I }, 0, 0 },
    { ocGrowth,             52,     1,  3,  A, { R, R, R, C, I }, 0, 0 },
    { ocBW,                 56,     3,  5,  V, { V }, 0, 0 },
    { ocZW,                 57,     3,  5,  V, { V }, 0, 0 },
    { ocZZR,                58,     3,  5,  V, { V }, 0, 0 },
    { ocRMZ,                59,     3,  5,  V, { V }, 0, 0 },
    { ocZins,               60,     3,  6,  V, { V }, 0, 0 },
    { ocMIRR,               61,     3,  3,  V, { R, V }, 0, 0 },
    { ocIRR,                62,     1,  2,  V, { R, V }, 0, 0 },
    { ocRandom,             63,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocMatch,              64,     2,  3,  V, { V, R }, 0, 0 },
    { ocGetDate,            65,     3,  3,  V, { V }, 0, 0 },
    { ocGetTime,            66,     3,  3,  V, { V }, 0, 0 },
    { ocGetDay,             67,     1,  1,  V, { V }, 0, 0 },
    { ocGetMonth,           68,     1,  1,  V, { V }, 0, 0 },
    { ocGetYear,            69,     1,  1,  V, { V }, 0, 0 },
    { ocGetDayOfWeek,       70,     1,  1,  V, { V, C, I }, 0, 0 },
    { ocGetHour,            71,     1,  1,  V, { V }, 0, 0 },
    { ocGetMin,             72,     1,  1,  V, { V }, 0, 0 },
    { ocGetSec,             73,     1,  1,  V, { V }, 0, 0 },
    { ocGetActTime,         74,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocAreas,              75,     1,  1,  V, { R }, 0, 0 },
    { ocRows,               76,     1,  1,  V, { R }, 0, 0 },
    { ocColumns,            77,     1,  1,  V, { R }, 0, 0 },
    { ocOffset,             78,     3,  5,  R, { R, V }, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocSearch,             82,     2,  3,  V, { V }, 0, 0 },
    { ocMatTrans,           83,     1,  1,  A, { A }, 0, 0 },
    { ocType,               86,     1,  1,  V, { V }, 0, 0 },
    { ocArcTan2,            97,     2,  2,  V, { V }, 0, 0 },
    { ocArcSin,             98,     1,  1,  V, { V }, 0, 0 },
    { ocArcCos,             99,     1,  1,  V, { V }, 0, 0 },
    { ocChose,              100,    2,  30, R, { V, R }, 0, 0 },
    { ocHLookup,            101,    3,  3,  V, { V, R, R, C, I }, 0, 0 },
    { ocVLookup,            102,    3,  3,  V, { V, R, R, C, I }, 0, 0 },
    { ocIsRef,              105,    1,  1,  V, { R }, 0, 0 },
    { ocLog,                109,    1,  2,  V, { V }, 0, 0 },
    { ocChar,               111,    1,  1,  V, { V }, 0, 0 },
    { ocLower,              112,    1,  1,  V, { V }, 0, 0 },
    { ocUpper,              113,    1,  1,  V, { V }, 0, 0 },
    { ocPropper,            114,    1,  1,  V, { V }, 0, 0 },
    { ocLeft,               115,    1,  2,  V, { V }, 0, 0 },
    { ocRight,              116,    1,  2,  V, { V }, 0, 0 },
    { ocExact,              117,    2,  2,  V, { V }, 0, 0 },
    { ocTrim,               118,    1,  1,  V, { V }, 0, 0 },
    { ocReplace,            119,    4,  4,  V, { V }, 0, 0 },
    { ocSubstitute,         120,    3,  4,  V, { V }, 0, 0 },
    { ocCode,               121,    1,  1,  V, { V }, 0, 0 },
    { ocFind,               124,    2,  3,  V, { V }, 0, 0 },
    { ocCell,               125,    1,  2,  V, { V, R }, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocIsErr,              126,    1,  1,  V, { V }, 0, 0 },
    { ocIsString,           127,    1,  1,  V, { V }, 0, 0 },
    { ocIsValue,            128,    1,  1,  V, { V }, 0, 0 },
    { ocIsEmpty,            129,    1,  1,  V, { V }, 0, 0 },
    { ocT,                  130,    1,  1,  V, { R }, 0, 0 },
    { ocN,                  131,    1,  1,  V, { R }, 0, 0 },
    { ocGetDateValue,       140,    1,  1,  V, { V }, 0, 0 },
    { ocGetTimeValue,       141,    1,  1,  V, { V }, 0, 0 },
    { ocLIA,                142,    3,  3,  V, { V }, 0, 0 },
    { ocDIA,                143,    4,  4,  V, { V }, 0, 0 },
    { ocGDA,                144,    4,  5,  V, { V }, 0, 0 },
    { ocIndirect,           148,    1,  2,  R, { V }, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocClean,              162,    1,  1,  V, { V }, 0, 0 },
    { ocMatDet,             163,    1,  1,  V, { A }, 0, 0 },
    { ocMatInv,             164,    1,  1,  A, { A }, 0, 0 },
    { ocMatMult,            165,    2,  2,  A, { A }, 0, 0 },
    { ocZinsZ,              167,    4,  6,  V, { V }, 0, 0 },
    { ocKapz,               168,    4,  6,  V, { V }, 0, 0 },
    { ocCount2,             169,    0,  30, V, { R }, 0, 0 },
    { ocProduct,            183,    0,  30, V, { R }, 0, 0 },
    { ocFact,               184,    1,  1,  V, { V }, 0, 0 },
    { ocDBProduct,          189,    3,  3,  V, { R }, 0, 0 },
    { ocIsNonString,        190,    1,  1,  V, { V }, 0, 0 },
    { ocStDevP,             193,    1,  30, V, { R }, 0, 0 },
    { ocVarP,               194,    1,  30, V, { R }, 0, 0 },
    { ocDBStdDevP,          195,    3,  3,  V, { R }, 0, 0 },
    { ocDBVarP,             196,    3,  3,  V, { R }, 0, 0 },
    { ocTrunc,              197,    1,  1,  V, { V, C, I }, 0, 0 },
    { ocIsLogical,          198,    1,  1,  V, { V }, 0, 0 },
    { ocDBCount2,           199,    3,  3,  V, { R }, 0, 0 },
    { ocCurrency,           204,    1,  2,  V, { V }, EXC_FUNCFLAG_IMPORTONLY, 0 },
    { ocRoundUp,            212,    2,  2,  V, { V }, 0, 0 },
    { ocRoundDown,          213,    2,  2,  V, { V }, 0, 0 },
    { ocExternal,           255,    1,  30, R, { E, R }, EXC_FUNCFLAG_IMPORTONLY, 0 }
};

/** Functions new in BIFF3. */
static const XclFunctionInfo saFuncTable_3[] =
{
    { ocRGP,                49,     1,  4,  A, { R, R, V, V }, 0, 0 },    // BIFF2: 1-2, BIFF3: 1-4
    { ocTrend,              50,     1,  4,  A, { R, R, R, V }, 0, 0 },    // BIFF2: 1-3, BIFF3: 1-4
    { ocRKP,                51,     1,  4,  A, { R, R, V, V }, 0, 0 },    // BIFF2: 1-2, BIFF3: 1-4
    { ocGrowth,             52,     1,  4,  A, { R, R, R, V }, 0, 0 },    // BIFF2: 1-3, BIFF3: 1-4
    { ocTrunc,              197,    1,  2,  V, { V }, 0, 0 },             // BIFF2: 1,   BIFF3: 1-2
    { ocAddress,            219,    2,  5,  V, { V, V, V, E, V }, 0, 0 },
    { ocGetDiffDate360,     220,    2,  2,  V, { V, V, C, I }, 0, 0 },
    { ocGetActDate,         221,    0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE, 0 },
    { ocVBD,                222,    5,  7,  V, { V }, 0, 0 },
    { ocMedian,             227,    1,  30, V, { R }, 0, 0 },
    { ocSumProduct,         228,    1,  30, V, { A }, 0, 0 },
    { ocSinHyp,             229,    1,  1,  V, { V }, 0, 0 },
    { ocCosHyp,             230,    1,  1,  V, { V }, 0, 0 },
    { ocTanHyp,             231,    1,  1,  V, { V }, 0, 0 },
    { ocCotHyp,             231,    1,  1,  V, { V }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocArcSinHyp,          232,    1,  1,  V, { V }, 0, 0 },
    { ocArcCosHyp,          233,    1,  1,  V, { V }, 0, 0 },
    { ocArcTanHyp,          234,    1,  1,  V, { V }, 0, 0 },
    { ocArcCotHyp,          234,    1,  1,  V, { V }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocDBGet,              235,    3,  3,  V, { R }, 0, 0 },
    { ocInfo,               244,    1,  1,  V, { V }, EXC_FUNCFLAG_VOLATILE, 0 }
};

/** Functions new in BIFF4. */
static const XclFunctionInfo saFuncTable_4[] =
{
    { ocFixed,              14,     1,  3,  V, { V }, 0, 0 },     // BIFF2-3: 1-2, BIFF4: 1-3
    { ocAsc,                214,    1,  1,  V, { V }, 0, 0 },
    { ocJis,                215,    1,  1,  V, { V }, 0, 0 },
    { ocRank,               216,    2,  3,  V, { V, R, V }, 0, 0 },
    { ocGDA2,               247,    4,  5,  V, { V }, 0, 0 },
    { ocFrequency,          252,    2,  2,  A, { R }, 0, 0 },
    { ocErrorType,          261,    1,  1,  V, { V }, 0, 0 },
    { ocAveDev,             269,    1,  30, V, { R }, 0, 0 },
    { ocBetaDist,           270,    3,  5,  V, { V }, 0, 0 },
    { ocGammaLn,            271,    1,  1,  V, { V }, 0, 0 },
    { ocBetaInv,            272,    3,  5,  V, { V }, 0, 0 },
    { ocBinomDist,          273,    4,  4,  V, { V }, 0, 0 },
    { ocChiDist,            274,    2,  2,  V, { V }, 0, 0 },
    { ocChiInv,             275,    2,  2,  V, { V }, 0, 0 },
    { ocKombin,             276,    2,  2,  V, { V }, 0, 0 },
    { ocConfidence,         277,    3,  3,  V, { V }, 0, 0 },
    { ocKritBinom,          278,    3,  3,  V, { V }, 0, 0 },
    { ocEven,               279,    1,  1,  V, { V }, 0, 0 },
    { ocExpDist,            280,    3,  3,  V, { V }, 0, 0 },
    { ocFDist,              281,    3,  3,  V, { V }, 0, 0 },
    { ocFInv,               282,    3,  3,  V, { V }, 0, 0 },
    { ocFisher,             283,    1,  1,  V, { V }, 0, 0 },
    { ocFisherInv,          284,    1,  1,  V, { V }, 0, 0 },
    { ocFloor,              285,    2,  2,  V, { V, V, C, I }, 0, 0 },
    { ocGammaDist,          286,    4,  4,  V, { V }, 0, 0 },
    { ocGammaInv,           287,    3,  3,  V, { V }, 0, 0 },
    { ocCeil,               288,    2,  2,  V, { V, V, C, I }, 0, 0 },
    { ocHypGeomDist,        289,    4,  4,  V, { V }, 0, 0 },
    { ocLogNormDist,        290,    3,  3,  V, { V }, 0, 0 },
    { ocLogInv,             291,    3,  3,  V, { V }, 0, 0 },
    { ocNegBinomVert,       292,    3,  3,  V, { V }, 0, 0 },
    { ocNormDist,           293,    4,  4,  V, { V }, 0, 0 },
    { ocStdNormDist,        294,    1,  1,  V, { V }, 0, 0 },
    { ocNormInv,            295,    3,  3,  V, { V }, 0, 0 },
    { ocSNormInv,           296,    1,  1,  V, { V }, 0, 0 },
    { ocStandard,           297,    3,  3,  V, { V }, 0, 0 },
    { ocOdd,                298,    1,  1,  V, { V }, 0, 0 },
    { ocVariationen,        299,    2,  2,  V, { V }, 0, 0 },
    { ocPoissonDist,        300,    3,  3,  V, { V }, 0, 0 },
    { ocTDist,              301,    3,  3,  V, { V }, 0, 0 },
    { ocWeibull,            302,    4,  4,  V, { V }, 0, 0 },
    { ocSumXMY2,            303,    2,  2,  V, { A }, 0, 0 },
    { ocSumX2MY2,           304,    2,  2,  V, { A }, 0, 0 },
    { ocSumX2DY2,           305,    2,  2,  V, { A }, 0, 0 },
    { ocChiTest,            306,    2,  2,  V, { A }, 0, 0 },
    { ocCorrel,             307,    2,  2,  V, { A }, 0, 0 },
    { ocCovar,              308,    2,  2,  V, { A }, 0, 0 },
    { ocForecast,           309,    3,  3,  V, { V, A }, 0, 0 },
    { ocFTest,              310,    2,  2,  V, { A }, 0, 0 },
    { ocIntercept,          311,    2,  2,  V, { A }, 0, 0 },
    { ocPearson,            312,    2,  2,  V, { A }, 0, 0 },
    { ocRSQ,                313,    2,  2,  V, { A }, 0, 0 },
    { ocSTEYX,              314,    2,  2,  V, { A }, 0, 0 },
    { ocSlope,              315,    2,  2,  V, { A }, 0, 0 },
    { ocTTest,              316,    4,  4,  V, { A, A, V }, 0, 0 },
    { ocProb,               317,    3,  4,  V, { A, A, V }, 0, 0 },
    { ocDevSq,              318,    1,  30, V, { R }, 0, 0 },
    { ocGeoMean,            319,    1,  30, V, { R }, 0, 0 },
    { ocHarMean,            320,    1,  30, V, { R }, 0, 0 },
    { ocSumSQ,              321,    0,  30, V, { R }, 0, 0 },
    { ocKurt,               322,    1,  30, V, { R }, 0, 0 },
    { ocSchiefe,            323,    1,  30, V, { R }, 0, 0 },
    { ocZTest,              324,    2,  3,  V, { R, V }, 0, 0 },
    { ocLarge,              325,    2,  2,  V, { R, V }, 0, 0 },
    { ocSmall,              326,    2,  2,  V, { R, V }, 0, 0 },
    { ocQuartile,           327,    2,  2,  V, { R, V }, 0, 0 },
    { ocPercentile,         328,    2,  2,  V, { R, V }, 0, 0 },
    { ocPercentrank,        329,    2,  3,  V, { R, V }, 0, 0 },
    { ocModalValue,         330,    1,  30, V, { A }, 0, 0 },
    { ocTrimMean,           331,    2,  2,  V, { R, V }, 0, 0 },
    { ocTInv,               332,    2,  2,  V, { V }, 0, 0 }
};

/** Functions new in BIFF5/BIFF7. Unsupported functions: DATEDIF, DATESTRING, NUMBERSTRING. */
static const XclFunctionInfo saFuncTable_5[] =
{
    { ocGetDayOfWeek,       70,     1,  2,  V, { V }, 0, 0 },             // BIFF2-4: 1, BIFF5: 1-2
    { ocHLookup,            101,    3,  4,  V, { V, R, R, V }, 0, 0 },    // BIFF2-4: 3, BIFF5: 3-4
    { ocVLookup,            102,    3,  4,  V, { V, R, R, V }, 0, 0 },    // BIFF2-4: 3, BIFF5: 3-4
    { ocGetDiffDate360,     220,    2,  3,  V, { V }, 0, 0 },             // BIFF3-4: 2, BIFF5: 2-3
    { ocMacro,              255,    1,  30, R, { E, R }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocExternal,           255,    1,  30, R, { E, R }, EXC_FUNCFLAG_EXPORTONLY, 0 },
    { ocConcat,             336,    0,  30, V, { V }, 0, 0 },
    { ocPower,              337,    2,  2,  V, { V }, 0, 0 },
    { ocRad,                342,    1,  1,  V, { V }, 0, 0 },
    { ocDeg,                343,    1,  1,  V, { V }, 0, 0 },
    { ocSubTotal,           344,    2,  30, V, { V, R }, 0, 0 },
    { ocSumIf,              345,    2,  3,  V, { R, V, R }, 0, 0 },
    { ocCountIf,            346,    2,  2,  V, { R, V }, 0, 0 },
    { ocCountEmptyCells,    347,    1,  1,  V, { R }, 0, 0 },
    { ocISPMT,              350,    4,  4,  V, { V }, 0, 0 },
    { ocNoName,             351,    3,  3,  V, { V }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // DATEDIF
    { ocNoName,             352,    1,  1,  V, { V }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // DATESTRING
    { ocNoName,             353,    2,  2,  V, { V }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // NUMBERSTRING
    { ocRoman,              354,    1,  2,  V, { V }, 0, 0 }
};

#define EXC_FUNCNAME_PREFIX "_xlfn."

const sal_Char* const EXC_FUNCNAME_BAHTTEXT = EXC_FUNCNAME_PREFIX "BAHTTEXT";

/** Functions new in BIFF8. Unsupported functions: PHONETIC. */
static const XclFunctionInfo saFuncTable_8[] =
{
    { ocGetPivotData,       358,    2,  30, V, { V, R, V }, 0, 0 },
    { ocHyperLink,          359,    1,  2,  V, { V }, 0, 0 },
    { ocNoName,             360,    1,  1,  V, { R }, EXC_FUNCFLAG_IMPORTONLY, 0 },    // PHONETIC
    { ocAverageA,           361,    1,  30, V, { R }, 0, 0 },
    { ocMaxA,               362,    1,  30, V, { R }, 0, 0 },
    { ocMinA,               363,    1,  30, V, { R }, 0, 0 },
    { ocStDevPA,            364,    1,  30, V, { R }, 0, 0 },
    { ocVarPA,              365,    1,  30, V, { R }, 0, 0 },
    { ocStDevA,             366,    1,  30, V, { R }, 0, 0 },
    { ocVarA,               367,    1,  30, V, { R }, 0, 0 },
    { ocBahtText,           368,    1,  1,  V, { V }, EXC_FUNCFLAG_IMPORTONLY, EXC_FUNCNAME_BAHTTEXT },
    { ocBahtText,           255,    2,  2,  V, { E, V }, EXC_FUNCFLAG_EXPORTONLY, EXC_FUNCNAME_BAHTTEXT }
};

// ----------------------------------------------------------------------------

XclFunctionProvider::XclFunctionProvider( const XclRoot& rRoot )
{
    XclBiff eBiff = rRoot.GetBiff();

    if( rRoot.IsImport() )
    {
        /*  Import: only read functions supported in the current BIFF version.
            Function tables from later BIFF versions may overwrite single
            functions from earlier tables. */
        if( eBiff >= EXC_BIFF2 )
            FillXclFuncMap( saFuncTable_2, STATIC_TABLE_END( saFuncTable_2 ) );
        if( eBiff >= EXC_BIFF3 )
            FillXclFuncMap( saFuncTable_3, STATIC_TABLE_END( saFuncTable_3 ) );
        if( eBiff >= EXC_BIFF4 )
            FillXclFuncMap( saFuncTable_4, STATIC_TABLE_END( saFuncTable_4 ) );
        if( eBiff >= EXC_BIFF5 )
            FillXclFuncMap( saFuncTable_5, STATIC_TABLE_END( saFuncTable_5 ) );
        if( eBiff >= EXC_BIFF8 )
            FillXclFuncMap( saFuncTable_8, STATIC_TABLE_END( saFuncTable_8 ) );
    }
    else
    {
        /*  Export: only write functions supported in the current BIFF version.
            Function tables from later BIFF versions may overwrite single
            functions from earlier tables. */
        if( eBiff >= EXC_BIFF2 )
            FillScFuncMap( saFuncTable_2, STATIC_TABLE_END( saFuncTable_2 ) );
        if( eBiff >= EXC_BIFF3 )
            FillScFuncMap( saFuncTable_3, STATIC_TABLE_END( saFuncTable_3 ) );
        if( eBiff >= EXC_BIFF4 )
            FillScFuncMap( saFuncTable_4, STATIC_TABLE_END( saFuncTable_4 ) );
        if( eBiff >= EXC_BIFF5 )
            FillScFuncMap( saFuncTable_5, STATIC_TABLE_END( saFuncTable_5 ) );
        if( eBiff >= EXC_BIFF8 )
            FillScFuncMap( saFuncTable_8, STATIC_TABLE_END( saFuncTable_8 ) );
    }
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

XclTokenArray::XclTokenArray( ScfUInt8Vec& rTokVec, bool bVolatile,
                              ScfUInt8Vec* pExtensionTokens) :
    mbVolatile( bVolatile )
{
    maTokVec.swap( rTokVec );
    if( NULL != pExtensionTokens)
    {
        DBG_ASSERT( maTokVec.size() <= 0xFFFF, "XclTokenArray::XclTokenArray - extension array too long" );
        maExtensions.swap( *pExtensionTokens );
    }
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
    if( !maExtensions.empty() )
        rStrm.Write( &maExtensions.front(), limit_cast< sal_uInt16 >(maExtensions.size() ) );
}

void XclTokenArray::Write( XclExpStream& rStrm ) const
{
    WriteSize( rStrm );
    WriteArray( rStrm );
}

bool XclTokenArray::operator==( const XclTokenArray& rTokArr ) const
{
    return (mbVolatile == rTokArr.mbVolatile) && (maTokVec == rTokArr.maTokVec);
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
    if( rxTokArr.is() )
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
    mppScTokenBeg = static_cast< const ScToken*const* >( nTokArrLen ? rScTokArr.GetArray() : 0 );
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

bool XclTokenArrayHelper::GetTokenString( String& rString, const ScToken& rScToken )
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
        if( const ScToken* pScToken = rScTokArr.GetArray()[ 0 ] )
            if( pScToken->GetOpCode() == ocName )
                if( ScRangeData* pData = rRoot.GetNamedRanges().FindIndex( pScToken->GetIndex() ) )
                    if( pData->HasType( RT_SHARED ) )
                        return pData->GetCode();
    return 0;
}

// multiple operations --------------------------------------------------------

namespace {

inline bool lclGetAddress( ScAddress& rAddress, const ScToken& rToken )
{
    OpCode eOpCode = rToken.GetOpCode();
    bool bIsSingleRef = (eOpCode == ocPush) && (rToken.GetType() == svSingleRef);
    if( bIsSingleRef )
    {
        const SingleRefData& rRef = rToken.GetSingleRef();
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

