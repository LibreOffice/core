/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlformula.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: ihi $ $Date: 2006-10-18 12:24:52 $
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
    { ocCount,              0,      0,  30, V, { R } },
    { ocIf,                 1,      2,  3,  R, { V, R } },
    { ocIsNV,               2,      1,  1,  V, { V } },
    { ocIsError,            3,      1,  1,  V, { V } },
    { ocSum,                4,      0,  30, V, { R } },
    { ocAverage,            5,      1,  30, V, { R } },
    { ocMin,                6,      1,  30, V, { R } },
    { ocMax,                7,      1,  30, V, { R } },
    { ocRow,                8,      0,  1,  V, { R } },
    { ocColumn,             9,      0,  1,  V, { R } },
    { ocNoValue,            10,     0,  0,  V },
    { ocNBW,                11,     2,  30, V, { V, R } },
    { ocStDev,              12,     1,  30, V, { R } },
    { ocCurrency,           13,     1,  2,  V, { V } },
    { ocFixed,              14,     2,  2,  V, { V, V, C, I } },
    { ocSin,                15,     1,  1,  V, { V } },
    { ocCos,                16,     1,  1,  V, { V } },
    { ocTan,                17,     1,  1,  V, { V } },
    { ocArcTan,             18,     1,  1,  V, { V } },
    { ocPi,                 19,     0,  0,  V },
    { ocSqrt,               20,     1,  1,  V, { V } },
    { ocExp,                21,     1,  1,  V, { V } },
    { ocLn,                 22,     1,  1,  V, { V } },
    { ocLog10,              23,     1,  1,  V, { V } },
    { ocAbs,                24,     1,  1,  V, { V } },
    { ocInt,                25,     1,  1,  V, { V } },
    { ocPlusMinus,          26,     1,  1,  V, { V } },
    { ocRound,              27,     2,  2,  V, { V } },
    { ocLookup,             28,     2,  3,  V, { V, R } },
    { ocIndex,              29,     2,  4,  R, { R, V } },
    { ocRept,               30,     2,  2,  V, { V } },
    { ocMid,                31,     3,  3,  V, { V } },
    { ocLen,                32,     1,  1,  V, { V } },
    { ocValue,              33,     1,  1,  V, { V } },
    { ocTrue,               34,     0,  0,  V },
    { ocFalse,              35,     0,  0,  V },
    { ocAnd,                36,     1,  30, V, { R } },
    { ocOr,                 37,     1,  30, V, { R } },
    { ocNot,                38,     1,  1,  V, { V } },
    { ocMod,                39,     2,  2,  V, { V } },
    { ocDBCount,            40,     3,  3,  V, { R } },
    { ocDBSum,              41,     3,  3,  V, { R } },
    { ocDBAverage,          42,     3,  3,  V, { R } },
    { ocDBMin,              43,     3,  3,  V, { R } },
    { ocDBMax,              44,     3,  3,  V, { R } },
    { ocDBStdDev,           45,     3,  3,  V, { R } },
    { ocVar,                46,     1,  30, V, { R } },
    { ocDBVar,              47,     3,  3,  V, { R } },
    { ocText,               48,     2,  2,  V, { V } },
    { ocRGP,                49,     1,  2,  A, { R, R, C, C, I } },
    { ocTrend,              50,     1,  3,  A, { R, R, R, C, I } },
    { ocRKP,                51,     1,  2,  A, { R, R, C, C, I } },
    { ocGrowth,             52,     1,  3,  A, { R, R, R, C, I } },
    { ocBW,                 56,     3,  5,  V, { V } },
    { ocZW,                 57,     3,  5,  V, { V } },
    { ocZZR,                58,     3,  5,  V, { V } },
    { ocRMZ,                59,     3,  5,  V, { V } },
    { ocZins,               60,     3,  6,  V, { V } },
    { ocMIRR,               61,     3,  3,  V, { R, V } },
    { ocIKV,                62,     1,  2,  V, { R, V } },
    { ocRandom,             63,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE },
    { ocMatch,              64,     2,  3,  V, { V, R } },
    { ocGetDate,            65,     3,  3,  V, { V } },
    { ocGetTime,            66,     3,  3,  V, { V } },
    { ocGetDay,             67,     1,  1,  V, { V } },
    { ocGetMonth,           68,     1,  1,  V, { V } },
    { ocGetYear,            69,     1,  1,  V, { V } },
    { ocGetDayOfWeek,       70,     1,  1,  V, { V, C, I } },
    { ocGetHour,            71,     1,  1,  V, { V } },
    { ocGetMin,             72,     1,  1,  V, { V } },
    { ocGetSec,             73,     1,  1,  V, { V } },
    { ocGetActTime,         74,     0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE },
    { ocAreas,              75,     1,  1,  V, { R } },
    { ocRows,               76,     1,  1,  V, { R } },
    { ocColumns,            77,     1,  1,  V, { R } },
    { ocOffset,             78,     3,  5,  R, { R, V }, EXC_FUNCFLAG_VOLATILE },
    { ocSearch,             82,     2,  3,  V, { V } },
    { ocMatTrans,           83,     1,  1,  A, { A } },
    { ocType,               86,     1,  1,  V, { V } },
    { ocArcTan2,            97,     2,  2,  V, { V } },
    { ocArcSin,             98,     1,  1,  V, { V } },
    { ocArcCos,             99,     1,  1,  V, { V } },
    { ocChose,              100,    2,  30, R, { V, R } },
    { ocHLookup,            101,    3,  3,  V, { V, R, R, C, I } },
    { ocVLookup,            102,    3,  3,  V, { V, R, R, C, I } },
    { ocIsRef,              105,    1,  1,  V, { R } },
    { ocLog,                109,    1,  2,  V, { V } },
    { ocChar,               111,    1,  1,  V, { V } },
    { ocLower,              112,    1,  1,  V, { V } },
    { ocUpper,              113,    1,  1,  V, { V } },
    { ocPropper,            114,    1,  1,  V, { V } },
    { ocLeft,               115,    1,  2,  V, { V } },
    { ocRight,              116,    1,  2,  V, { V } },
    { ocExact,              117,    2,  2,  V, { V } },
    { ocTrim,               118,    1,  1,  V, { V } },
    { ocReplace,            119,    4,  4,  V, { V } },
    { ocSubstitute,         120,    3,  4,  V, { V } },
    { ocCode,               121,    1,  1,  V, { V } },
    { ocFind,               124,    2,  3,  V, { V } },
    { ocCell,               125,    1,  2,  V, { V, R }, EXC_FUNCFLAG_VOLATILE },
    { ocIsErr,              126,    1,  1,  V, { V } },
    { ocIsString,           127,    1,  1,  V, { V } },
    { ocIsValue,            128,    1,  1,  V, { V } },
    { ocIsEmpty,            129,    1,  1,  V, { V } },
    { ocT,                  130,    1,  1,  V, { R } },
    { ocN,                  131,    1,  1,  V, { R } },
    { ocGetDateValue,       140,    1,  1,  V, { V } },
    { ocGetTimeValue,       141,    1,  1,  V, { V } },
    { ocLIA,                142,    3,  3,  V, { V } },
    { ocDIA,                143,    4,  4,  V, { V } },
    { ocGDA,                144,    4,  5,  V, { V } },
    { ocIndirect,           148,    1,  2,  R, { V }, EXC_FUNCFLAG_VOLATILE },
    { ocClean,              162,    1,  1,  V, { V } },
    { ocMatDet,             163,    1,  1,  V, { A } },
    { ocMatInv,             164,    1,  1,  A, { A } },
    { ocMatMult,            165,    2,  2,  A, { A } },
    { ocZinsZ,              167,    4,  6,  V, { V } },
    { ocKapz,               168,    4,  6,  V, { V } },
    { ocCount2,             169,    0,  30, V, { R } },
    { ocProduct,            183,    0,  30, V, { R } },
    { ocFact,               184,    1,  1,  V, { V } },
    { ocDBProduct,          189,    3,  3,  V, { R } },
    { ocIsNonString,        190,    1,  1,  V, { V } },
    { ocStDevP,             193,    1,  30, V, { R } },
    { ocVarP,               194,    1,  30, V, { R } },
    { ocDBStdDevP,          195,    3,  3,  V, { R } },
    { ocDBVarP,             196,    3,  3,  V, { R } },
    { ocTrunc,              197,    1,  1,  V, { V, C, I } },
    { ocIsLogical,          198,    1,  1,  V, { V } },
    { ocDBCount2,           199,    3,  3,  V, { R } },
    { ocRoundUp,            212,    2,  2,  V, { V } },
    { ocRoundDown,          213,    2,  2,  V, { V } },
    { ocExternal,           255,    1,  30, R, { E, R }, EXC_FUNCFLAG_IMPORTONLY }
};

/** Functions new in BIFF3. */
static const XclFunctionInfo saFuncTable_3[] =
{
    { ocRGP,                49,     1,  4,  A, { R, R, V, V } },    // BIFF2: 1-2, BIFF3: 1-4
    { ocTrend,              50,     1,  4,  A, { R, R, R, V } },    // BIFF2: 1-3, BIFF3: 1-4
    { ocRKP,                51,     1,  4,  A, { R, R, V, V } },    // BIFF2: 1-2, BIFF3: 1-4
    { ocGrowth,             52,     1,  4,  A, { R, R, R, V } },    // BIFF2: 1-3, BIFF3: 1-4
    { ocTrunc,              197,    1,  2,  V, { V } },             // BIFF2: 1,   BIFF3: 1-2
    { ocAddress,            219,    2,  5,  V, { V, V, V, E, V } },
    { ocGetDiffDate360,     220,    2,  2,  V, { V, V, C, I } },
    { ocGetActDate,         221,    0,  0,  V, {}, EXC_FUNCFLAG_VOLATILE },
    { ocVBD,                222,    5,  7,  V, { V } },
    { ocMedian,             227,    1,  30, V, { R } },
    { ocSumProduct,         228,    1,  30, V, { A } },
    { ocSinHyp,             229,    1,  1,  V, { V } },
    { ocCosHyp,             230,    1,  1,  V, { V } },
    { ocTanHyp,             231,    1,  1,  V, { V } },
    { ocArcSinHyp,          232,    1,  1,  V, { V } },
    { ocArcCosHyp,          233,    1,  1,  V, { V } },
    { ocArcTanHyp,          234,    1,  1,  V, { V } },
    { ocDBGet,              235,    3,  3,  V, { R } },
    { ocInfo,               244,    1,  1,  V, { V }, EXC_FUNCFLAG_VOLATILE }
};

/** Functions new in BIFF4. Unsupported functions: ASC, DBCS. */
static const XclFunctionInfo saFuncTable_4[] =
{
    { ocFixed,              14,     2,  3,  V, { V } },     // BIFF2-3: 2, BIFF4: 2-3
    { ocNoName,             214,    1,  1,  V, { V }, EXC_FUNCFLAG_IMPORTONLY },    // ASC
    { ocNoName,             215,    1,  1,  V, { V }, EXC_FUNCFLAG_IMPORTONLY },    // DBCS
    { ocRank,               216,    2,  3,  V, { V, R, V } },
    { ocGDA2,               247,    4,  5,  V, { V } },
    { ocFrequency,          252,    2,  2,  A, { R } },
    { ocErrorType,          261,    1,  1,  V, { V } },
    { ocAveDev,             269,    1,  30, V, { R } },
    { ocBetaDist,           270,    3,  5,  V, { V } },
    { ocGammaLn,            271,    1,  1,  V, { V } },
    { ocBetaInv,            272,    3,  5,  V, { V } },
    { ocBinomDist,          273,    4,  4,  V, { V } },
    { ocChiDist,            274,    2,  2,  V, { V } },
    { ocChiInv,             275,    2,  2,  V, { V } },
    { ocKombin,             276,    2,  2,  V, { V } },
    { ocConfidence,         277,    3,  3,  V, { V } },
    { ocKritBinom,          278,    3,  3,  V, { V } },
    { ocEven,               279,    1,  1,  V, { V } },
    { ocExpDist,            280,    3,  3,  V, { V } },
    { ocFDist,              281,    3,  3,  V, { V } },
    { ocFInv,               282,    3,  3,  V, { V } },
    { ocFisher,             283,    1,  1,  V, { V } },
    { ocFisherInv,          284,    1,  1,  V, { V } },
    { ocFloor,              285,    2,  2,  V, { V, V, C, I } },
    { ocGammaDist,          286,    4,  4,  V, { V } },
    { ocGammaInv,           287,    3,  3,  V, { V } },
    { ocCeil,               288,    2,  2,  V, { V, V, C, I } },
    { ocHypGeomDist,        289,    4,  4,  V, { V } },
    { ocLogNormDist,        290,    3,  3,  V, { V } },
    { ocLogInv,             291,    3,  3,  V, { V } },
    { ocNegBinomVert,       292,    3,  3,  V, { V } },
    { ocNormDist,           293,    4,  4,  V, { V } },
    { ocStdNormDist,        294,    1,  1,  V, { V } },
    { ocNormInv,            295,    3,  3,  V, { V } },
    { ocSNormInv,           296,    1,  1,  V, { V } },
    { ocStandard,           297,    3,  3,  V, { V } },
    { ocOdd,                298,    1,  1,  V, { V } },
    { ocVariationen,        299,    2,  2,  V, { V } },
    { ocPoissonDist,        300,    3,  3,  V, { V } },
    { ocTDist,              301,    3,  3,  V, { V } },
    { ocWeibull,            302,    4,  4,  V, { V } },
    { ocSumXMY2,            303,    2,  2,  V, { A } },
    { ocSumX2MY2,           304,    2,  2,  V, { A } },
    { ocSumX2DY2,           305,    2,  2,  V, { A } },
    { ocChiTest,            306,    2,  2,  V, { A } },
    { ocCorrel,             307,    2,  2,  V, { A } },
    { ocCovar,              308,    2,  2,  V, { A } },
    { ocForecast,           309,    3,  3,  V, { V, A } },
    { ocFTest,              310,    2,  2,  V, { A } },
    { ocIntercept,          311,    2,  2,  V, { A } },
    { ocPearson,            312,    2,  2,  V, { A } },
    { ocRSQ,                313,    2,  2,  V, { A } },
    { ocSTEYX,              314,    2,  2,  V, { A } },
    { ocSlope,              315,    2,  2,  V, { A } },
    { ocTTest,              316,    4,  4,  V, { A, A, V } },
    { ocProb,               317,    3,  4,  V, { A, A, V } },
    { ocDevSq,              318,    1,  30, V, { R } },
    { ocGeoMean,            319,    1,  30, V, { R } },
    { ocHarMean,            320,    1,  30, V, { R } },
    { ocSumSQ,              321,    0,  30, V, { R } },
    { ocKurt,               322,    1,  30, V, { R } },
    { ocSchiefe,            323,    1,  30, V, { R } },
    { ocZTest,              324,    2,  3,  V, { R, V } },
    { ocLarge,              325,    2,  2,  V, { R, V } },
    { ocSmall,              326,    2,  2,  V, { R, V } },
    { ocQuartile,           327,    2,  2,  V, { R, V } },
    { ocPercentile,         328,    2,  2,  V, { R, V } },
    { ocPercentrank,        329,    2,  3,  V, { R, V } },
    { ocModalValue,         330,    1,  30, V, { A } },
    { ocTrimMean,           331,    2,  2,  V, { R, V } },
    { ocTInv,               332,    2,  2,  V, { V } }
};

/** Functions new in BIFF5/BIFF7. Unsupported functions: DATEDIF, DATESTRING, NUMBERSTRING. */
static const XclFunctionInfo saFuncTable_5[] =
{
    { ocGetDayOfWeek,       70,     1,  2,  V, { V } },             // BIFF2-4: 1, BIFF5: 1-2
    { ocHLookup,            101,    3,  4,  V, { V, R, R, V } },    // BIFF2-4: 3, BIFF5: 3-4
    { ocVLookup,            102,    3,  4,  V, { V, R, R, V } },    // BIFF2-4: 3, BIFF5: 3-4
    { ocGetDiffDate360,     220,    2,  3,  V, { V } },             // BIFF3-4: 2, BIFF5: 2-3
    { ocMacro,              255,    1,  30, R, { E, R }, EXC_FUNCFLAG_EXPORTONLY },
    { ocExternal,           255,    1,  30, R, { E, R }, EXC_FUNCFLAG_EXPORTONLY },
    { ocConcat,             336,    0,  30, V, { V } },
    { ocPower,              337,    2,  2,  V, { V } },
    { ocRad,                342,    1,  1,  V, { V } },
    { ocDeg,                343,    1,  1,  V, { V } },
    { ocSubTotal,           344,    2,  30, V, { V, R } },
    { ocSumIf,              345,    2,  3,  V, { R, V, R } },
    { ocCountIf,            346,    2,  2,  V, { R, V } },
    { ocCountEmptyCells,    347,    1,  1,  V, { R } },
    { ocISPMT,              350,    4,  4,  V, { V } },
    { ocNoName,             351,    3,  3,  V, { V }, EXC_FUNCFLAG_IMPORTONLY },    // DATEDIF
    { ocNoName,             352,    1,  1,  V, { V }, EXC_FUNCFLAG_IMPORTONLY },    // DATESTRING
    { ocNoName,             353,    2,  2,  V, { V }, EXC_FUNCFLAG_IMPORTONLY },    // NUMBERSTRING
    { ocRoman,              354,    1,  2,  V, { V } }
};

#define EXC_FUNCNAME_PREFIX "_xlfn."

const sal_Char* const EXC_FUNCNAME_BAHTTEXT = EXC_FUNCNAME_PREFIX "BAHTTEXT";

/** Functions new in BIFF8. Unsupported functions: PHONETIC. */
static const XclFunctionInfo saFuncTable_8[] =
{
    { ocHyperLink,          359,    1,  2,  V, { V } },
    { ocNoName,             360,    1,  1,  V, { R }, EXC_FUNCFLAG_IMPORTONLY },    // PHONETIC
    { ocAverageA,           361,    1,  30, V, { R } },
    { ocMaxA,               362,    1,  30, V, { R } },
    { ocMinA,               363,    1,  30, V, { R } },
    { ocStDevPA,            364,    1,  30, V, { R } },
    { ocVarPA,              365,    1,  30, V, { R } },
    { ocStDevA,             366,    1,  30, V, { R } },
    { ocVarA,               367,    1,  30, V, { R } },
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

XclTokenArray::XclTokenArray( ScfUInt8Vec& rTokVec, bool bVolatile ) :
    mbVolatile( bVolatile )
{
    maTokVec.swap( rTokVec );
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

XclExpStream& operator<<( XclExpStream& rStrm, const XclTokenArray& rTokArr )
{
    rTokArr.Write( rStrm );
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
    }
    return bRet;
}

void XclTokenArrayHelper::ConvertStringToList( ScTokenArray& rScTokArr, sal_Unicode cStringSep )
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
    bool bRet = false;
    enum
    {
        stBegin, stTableOp, stOpen, stFormula, stFormulaSep,
        stColFirst, stColFirstSep, stColRel, stColRelSep,
        stRowFirst, stRowFirstSep, stRowRel, stClose, stError
    } eState = stBegin;     // last read token
    for( XclTokenArrayIterator aIt( rScTokArr, true ); aIt.Is() && (eState != stError); ++aIt )
    {
        OpCode eOpCode = aIt->GetOpCode();
        bool bIsSingleRef = (eOpCode == ocPush) && (aIt->GetType() == svSingleRef);
        bool bIsSep = (eOpCode == ocSep);
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

