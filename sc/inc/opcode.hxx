/*************************************************************************
 *
 *  $RCSfile: opcode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-28 16:36:52 $
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

#ifndef SC_OPCODE_HXX
#define SC_OPCODE_HXX

#ifndef SC_COMPILER_HRC
#include "compiler.hrc"                     // OpCodes
#endif

enum OpCodeEnum
{
    // Spezielle Kommandos
        ocPush              = SC_OPCODE_PUSH,
        ocJump              = SC_OPCODE_JUMP,
        ocStop              = SC_OPCODE_STOP,
        ocExternal          = SC_OPCODE_EXTERNAL,
        ocName              = SC_OPCODE_NAME,
    // Sprung Kommandos
        ocIf                = SC_OPCODE_IF,
        ocChose             = SC_OPCODE_CHOSE,
    // Klammern und Seps
        ocOpen              = SC_OPCODE_OPEN,
        ocClose             = SC_OPCODE_CLOSE,
        ocSep               = SC_OPCODE_SEP,
    // Spezial-Opcodes
        ocMissing           = SC_OPCODE_MISSING,
        ocBad               = SC_OPCODE_BAD,
        ocSpaces            = SC_OPCODE_SPACES,
        ocMatRef            = SC_OPCODE_MAT_REF,
    // weitere Zugriffs-Operanden
        ocDBArea            = SC_OPCODE_DB_AREA,
        ocMacro             = SC_OPCODE_MACRO,
        ocColRowName        = SC_OPCODE_COL_ROW_NAME,
        ocColRowNameAuto    = SC_OPCODE_COL_ROW_NAME_AUTO,
        ocEndDiv            = SC_OPCODE_END_DIV,
    // Binaere Operatoren
        ocAdd               = SC_OPCODE_ADD,
        ocSub               = SC_OPCODE_SUB,
        ocMul               = SC_OPCODE_MUL,
        ocDiv               = SC_OPCODE_DIV,
        ocAmpersand         = SC_OPCODE_AMPERSAND,
        ocPow               = SC_OPCODE_POW,
        ocEqual             = SC_OPCODE_EQUAL,
        ocNotEqual          = SC_OPCODE_NOT_EQUAL,
        ocLess              = SC_OPCODE_LESS,
        ocGreater           = SC_OPCODE_GREATER,
        ocLessEqual         = SC_OPCODE_LESS_EQUAL,
        ocGreaterEqual      = SC_OPCODE_GREATER_EQUAL,
        ocAnd               = SC_OPCODE_AND,
        ocOr                = SC_OPCODE_OR,
        ocIntersect         = SC_OPCODE_INTERSECT,
        ocUnion             = SC_OPCODE_UNION,
        ocRange             = SC_OPCODE_RANGE,
        ocEndBinOp          = SC_OPCODE_END_BIN_OP,
    // Unaere Operatoren
        ocNot               = SC_OPCODE_NOT,
        ocNeg               = SC_OPCODE_NEG,
        ocNegSub            = SC_OPCODE_NEG_SUB,
        ocEndUnOp           = SC_OPCODE_END_UN_OP,
    // Funktionen mit keinem Parameter
        ocPi                = SC_OPCODE_PI,
        ocRandom            = SC_OPCODE_RANDOM,
        ocTrue              = SC_OPCODE_TRUE,
        ocFalse             = SC_OPCODE_FALSE,
        ocGetActDate        = SC_OPCODE_GET_ACT_DATE,
        ocGetActTime        = SC_OPCODE_GET_ACT_TIME,
        ocNoValue           = SC_OPCODE_NO_VALUE,
        ocCurrent           = SC_OPCODE_CURRENT,
        ocEndNoPar          = SC_OPCODE_END_NO_PAR,
    // Funktionen mit einem Parameter
        ocDeg               = SC_OPCODE_DEG,
        ocRad               = SC_OPCODE_RAD,
        ocSin               = SC_OPCODE_SIN,
        ocCos               = SC_OPCODE_COS,
        ocTan               = SC_OPCODE_TAN,
        ocCot               = SC_OPCODE_COT,
        ocArcSin            = SC_OPCODE_ARC_SIN,
        ocArcCos            = SC_OPCODE_ARC_COS,
        ocArcTan            = SC_OPCODE_ARC_TAN,
        ocArcCot            = SC_OPCODE_ARC_COT,
        ocSinHyp            = SC_OPCODE_SIN_HYP,
        ocCosHyp            = SC_OPCODE_COS_HYP,
        ocTanHyp            = SC_OPCODE_TAN_HYP,
        ocCotHyp            = SC_OPCODE_COT_HYP,
        ocArcSinHyp         = SC_OPCODE_ARC_SIN_HYP,
        ocArcCosHyp         = SC_OPCODE_ARC_COS_HYP,
        ocArcTanHyp         = SC_OPCODE_ARC_TAN_HYP,
        ocArcCotHyp         = SC_OPCODE_ARC_COT_HYP,
        ocExp               = SC_OPCODE_EXP,
        ocLn                = SC_OPCODE_LN,
        ocSqrt              = SC_OPCODE_SQRT,
        ocFact              = SC_OPCODE_FACT,
        ocGetYear           = SC_OPCODE_GET_YEAR,
        ocGetMonth          = SC_OPCODE_GET_MONTH,
        ocGetDay            = SC_OPCODE_GET_DAY,
        ocGetHour           = SC_OPCODE_GET_HOUR,
        ocGetMin            = SC_OPCODE_GET_MIN,
        ocGetSec            = SC_OPCODE_GET_SEC,
        ocPlusMinus         = SC_OPCODE_PLUS_MINUS,
        ocAbs               = SC_OPCODE_ABS,
        ocInt               = SC_OPCODE_INT,
        ocPhi               = SC_OPCODE_PHI,
        ocGauss             = SC_OPCODE_GAUSS,
        ocIsEmpty           = SC_OPCODE_IS_EMPTY,
        ocIsString          = SC_OPCODE_IS_STRING,
        ocIsNonString       = SC_OPCODE_IS_NON_STRING,
        ocIsLogical         = SC_OPCODE_IS_LOGICAL,
        ocType              = SC_OPCODE_TYPE,
        ocIsRef             = SC_OPCODE_IS_REF,
        ocIsValue           = SC_OPCODE_IS_VALUE,
        ocIsFormula         = SC_OPCODE_IS_FORMULA,
        ocIsNV              = SC_OPCODE_IS_NV,
        ocIsErr             = SC_OPCODE_IS_ERR,
        ocIsError           = SC_OPCODE_IS_ERROR,
        ocIsEven            = SC_OPCODE_IS_EVEN,
        ocIsOdd             = SC_OPCODE_IS_ODD,
        ocN                 = SC_OPCODE_N,
    // String-Funktionen
        ocGetDateValue      = SC_OPCODE_GET_DATE_VALUE,
        ocGetTimeValue      = SC_OPCODE_GET_TIME_VALUE,
        ocCode              = SC_OPCODE_CODE,
        ocTrim              = SC_OPCODE_TRIM,
        ocUpper             = SC_OPCODE_UPPER,
        ocPropper           = SC_OPCODE_PROPPER,
        ocLower             = SC_OPCODE_LOWER,
        ocLen               = SC_OPCODE_LEN,
        ocT                 = SC_OPCODE_T,
        ocValue             = SC_OPCODE_VALUE,
        ocClean             = SC_OPCODE_CLEAN,
        ocChar              = SC_OPCODE_CHAR,
        ocLog10             = SC_OPCODE_LOG10,
        ocEven              = SC_OPCODE_EVEN,
        ocOdd               = SC_OPCODE_ODD,
        ocStdNormDist       = SC_OPCODE_STD_NORM_DIST,
        ocFisher            = SC_OPCODE_FISHER,
        ocFisherInv         = SC_OPCODE_FISHER_INV,
        ocSNormInv          = SC_OPCODE_S_NORM_INV,
        ocGammaLn           = SC_OPCODE_GAMMA_LN,
        ocErrorType         = SC_OPCODE_ERROR_TYPE,
        ocErrCell           = SC_OPCODE_ERR_CELL,
        ocFormula           = SC_OPCODE_FORMULA,
        ocEnd1Par           = SC_OPCODE_END_1_PAR,
    // Funktionen mit mehreren Parametern
        ocArcTan2           = SC_OPCODE_ARC_TAN_2,
        ocCeil              = SC_OPCODE_CEIL,
        ocFloor             = SC_OPCODE_FLOOR,
        ocRound             = SC_OPCODE_ROUND,
        ocRoundUp           = SC_OPCODE_ROUND_UP,
        ocRoundDown         = SC_OPCODE_ROUND_DOWN,
        ocTrunc             = SC_OPCODE_TRUNC,
        ocLog               = SC_OPCODE_LOG,
        ocPower             = SC_OPCODE_POWER,
        ocGGT               = SC_OPCODE_GGT,
        ocKGV               = SC_OPCODE_KGV,
        ocMod               = SC_OPCODE_MOD,
        ocSumProduct        = SC_OPCODE_SUM_PRODUCT,
        ocSumSQ             = SC_OPCODE_SUM_SQ,
        ocSumX2MY2          = SC_OPCODE_SUM_X2MY2,
        ocSumX2DY2          = SC_OPCODE_SUM_X2DY2,
        ocSumXMY2           = SC_OPCODE_SUM_XMY2,
        ocGetDate           = SC_OPCODE_GET_DATE,
        ocGetTime           = SC_OPCODE_GET_TIME,
        ocGetDiffDate       = SC_OPCODE_GET_DIFF_DATE,
        ocGetDiffDate360    = SC_OPCODE_GET_DIFF_DATE_360,
        ocMin               = SC_OPCODE_MIN,
        ocMax               = SC_OPCODE_MAX,
        ocSum               = SC_OPCODE_SUM,
        ocProduct           = SC_OPCODE_PRODUCT,
        ocAverage           = SC_OPCODE_AVERAGE,
        ocCount             = SC_OPCODE_COUNT,
        ocCount2            = SC_OPCODE_COUNT_2,
        ocNBW               = SC_OPCODE_NBW,
        ocIKV               = SC_OPCODE_IKV,
        ocVar               = SC_OPCODE_VAR,
        ocVarP              = SC_OPCODE_VAR_P,
        ocStDev             = SC_OPCODE_ST_DEV,
        ocStDevP            = SC_OPCODE_ST_DEV_P,
        ocB                 = SC_OPCODE_B,
        ocNormDist          = SC_OPCODE_NORM_DIST,
        ocExpDist           = SC_OPCODE_EXP_DIST,
        ocBinomDist         = SC_OPCODE_BINOM_DIST,
        ocPoissonDist       = SC_OPCODE_POISSON_DIST,
        ocKombin            = SC_OPCODE_KOMBIN,
        ocKombin2           = SC_OPCODE_KOMBIN_2,
        ocVariationen       = SC_OPCODE_VARIATIONEN,
        ocVariationen2      = SC_OPCODE_VARIATIONEN_2,
        ocBW                = SC_OPCODE_BW,
        ocDIA               = SC_OPCODE_DIA,
        ocGDA               = SC_OPCODE_GDA,
        ocGDA2              = SC_OPCODE_GDA_2,
        ocVBD               = SC_OPCODE_VBD,
        ocLaufz             = SC_OPCODE_LAUFZ,
        ocLIA               = SC_OPCODE_LIA,
        ocRMZ               = SC_OPCODE_RMZ,
        ocColumns           = SC_OPCODE_COLUMNS,
        ocRows              = SC_OPCODE_ROWS,
        ocColumn            = SC_OPCODE_COLUMN,
        ocRow               = SC_OPCODE_ROW,
        ocZGZ               = SC_OPCODE_ZGZ,
        ocZW                = SC_OPCODE_ZW,
        ocZZR               = SC_OPCODE_ZZR,
        ocZins              = SC_OPCODE_ZINS,
        ocZinsZ             = SC_OPCODE_ZINS_Z,
        ocKapz              = SC_OPCODE_KAPZ,
        ocKumZinsZ          = SC_OPCODE_KUM_ZINS_Z,
        ocKumKapZ           = SC_OPCODE_KUM_KAP_Z,
        ocEffektiv          = SC_OPCODE_EFFEKTIV,
        ocNominal           = SC_OPCODE_NOMINAL,
    // Spezialfunktion für Teilergebnisse
        ocSubTotal          = SC_OPCODE_SUB_TOTAL,
    // Datenbankfunktionen
        ocDBSum             = SC_OPCODE_DB_SUM,
        ocDBCount           = SC_OPCODE_DB_COUNT,
        ocDBCount2          = SC_OPCODE_DB_COUNT_2,
        ocDBAverage         = SC_OPCODE_DB_AVERAGE,
        ocDBGet             = SC_OPCODE_DB_GET,
        ocDBMax             = SC_OPCODE_DB_MAX,
        ocDBMin             = SC_OPCODE_DB_MIN,
        ocDBProduct         = SC_OPCODE_DB_PRODUCT,
        ocDBStdDev          = SC_OPCODE_DB_STD_DEV,
        ocDBStdDevP         = SC_OPCODE_DB_STD_DEV_P,
        ocDBVar             = SC_OPCODE_DB_VAR,
        ocDBVarP            = SC_OPCODE_DB_VAR_P,
    // Verwaltungsfunktionen
        ocIndirect          = SC_OPCODE_INDIRECT,
        ocAdress            = SC_OPCODE_ADRESS,
        ocMatch             = SC_OPCODE_MATCH,
        ocCountEmptyCells   = SC_OPCODE_COUNT_EMPTY_CELLS,
        ocCountIf           = SC_OPCODE_COUNT_IF,
        ocSumIf             = SC_OPCODE_SUM_IF,
        ocLookup            = SC_OPCODE_LOOKUP,
        ocVLookup           = SC_OPCODE_V_LOOKUP,
        ocHLookup           = SC_OPCODE_H_LOOKUP,
        ocMultiArea         = SC_OPCODE_MULTI_AREA,
        ocOffset            = SC_OPCODE_OFFSET,
        ocIndex             = SC_OPCODE_INDEX,
        ocAreas             = SC_OPCODE_AREAS,
    // String-Funktionen
        ocCurrency          = SC_OPCODE_CURRENCY,
        ocReplace           = SC_OPCODE_REPLACE,
        ocFixed             = SC_OPCODE_FIXED,
        ocFind              = SC_OPCODE_FIND,
        ocExact             = SC_OPCODE_EXACT,
        ocLeft              = SC_OPCODE_LEFT,
        ocRight             = SC_OPCODE_RIGHT,
        ocSearch            = SC_OPCODE_SEARCH,
        ocMid               = SC_OPCODE_MID,
        ocText              = SC_OPCODE_TEXT,
        ocSubstitute        = SC_OPCODE_SUBSTITUTE,
        ocRept              = SC_OPCODE_REPT,
        ocConcat            = SC_OPCODE_CONCAT,
    // Matrix-Funktionen
        ocMatValue          = SC_OPCODE_MAT_VALUE,
        ocMatDet            = SC_OPCODE_MAT_DET,
        ocMatInv            = SC_OPCODE_MAT_INV,
        ocMatMult           = SC_OPCODE_MAT_MULT,
        ocMatTrans          = SC_OPCODE_MAT_TRANS,
        ocMatrixUnit        = SC_OPCODE_MATRIX_UNIT,
    // BackSolver
        ocBackSolver        = SC_OPCODE_BACK_SOLVER,
    // Statistik-Funktionen
        ocHypGeomDist       = SC_OPCODE_HYP_GEOM_DIST,
        ocLogNormDist       = SC_OPCODE_LOG_NORM_DIST,
        ocTDist             = SC_OPCODE_T_DIST,
        ocFDist             = SC_OPCODE_F_DIST,
        ocChiDist           = SC_OPCODE_CHI_DIST,
        ocWeibull           = SC_OPCODE_WEIBULL,
        ocNegBinomVert      = SC_OPCODE_NEG_BINOM_VERT,
        ocKritBinom         = SC_OPCODE_KRIT_BINOM,
        ocKurt              = SC_OPCODE_KURT,
        ocHarMean           = SC_OPCODE_HAR_MEAN,
        ocGeoMean           = SC_OPCODE_GEO_MEAN,
        ocStandard          = SC_OPCODE_STANDARD,
        ocAveDev            = SC_OPCODE_AVE_DEV,
        ocSchiefe           = SC_OPCODE_SCHIEFE,
        ocDevSq             = SC_OPCODE_DEV_SQ,
        ocMedian            = SC_OPCODE_MEDIAN,
        ocModalValue        = SC_OPCODE_MODAL_VALUE,
        ocZTest             = SC_OPCODE_Z_TEST,
        ocTTest             = SC_OPCODE_T_TEST,
        ocRank              = SC_OPCODE_RANK,
        ocPercentile        = SC_OPCODE_PERCENTILE,
        ocPercentrank       = SC_OPCODE_PERCENT_RANK,
        ocLarge             = SC_OPCODE_LARGE,
        ocSmall             = SC_OPCODE_SMALL,
        ocFrequency         = SC_OPCODE_FREQUENCY,
        ocQuartile          = SC_OPCODE_QUARTILE,
        ocNormInv           = SC_OPCODE_NORM_INV,
        ocConfidence        = SC_OPCODE_CONFIDENCE,
        ocFTest             = SC_OPCODE_F_TEST,
        ocTrimMean          = SC_OPCODE_TRIM_MEAN,
        ocProb              = SC_OPCODE_PROB,
        ocCorrel            = SC_OPCODE_CORREL,
        ocCovar             = SC_OPCODE_COVAR,
        ocPearson           = SC_OPCODE_PEARSON,
        ocRSQ               = SC_OPCODE_RSQ,
        ocSTEYX             = SC_OPCODE_STEYX,
        ocSlope             = SC_OPCODE_SLOPE,
        ocIntercept         = SC_OPCODE_INTERCEPT,
        ocTrend             = SC_OPCODE_TREND,
        ocGrowth            = SC_OPCODE_GROWTH,
        ocRGP               = SC_OPCODE_RGP,
        ocRKP               = SC_OPCODE_RKP,
        ocForecast          = SC_OPCODE_FORECAST,
        ocChiInv            = SC_OPCODE_CHI_INV,
        ocGammaDist         = SC_OPCODE_GAMMA_DIST,
        ocGammaInv          = SC_OPCODE_GAMMA_INV,
        ocTInv              = SC_OPCODE_T_INV,
        ocFInv              = SC_OPCODE_F_INV,
        ocChiTest           = SC_OPCODE_CHI_TEST,
        ocLogInv            = SC_OPCODE_LOG_INV,
        ocTableOp           = SC_OPCODE_TABLE_OP,
        ocBetaDist          = SC_OPCODE_BETA_DIST,
        ocBetaInv           = SC_OPCODE_BETA_INV,
    // sonstige
        ocWeek              = SC_OPCODE_WEEK,
        ocGetDayOfWeek      = SC_OPCODE_GET_DAY_OF_WEEK,
        ocNoName            = SC_OPCODE_NO_NAME,
        ocStyle             = SC_OPCODE_STYLE,
        ocDde               = SC_OPCODE_DDE,
        ocBase              = SC_OPCODE_BASE,
        ocTable             = SC_OPCODE_TABLE,
        ocTables            = SC_OPCODE_TABLES,
        ocMinA              = SC_OPCODE_MIN_A,
        ocMaxA              = SC_OPCODE_MAX_A,
        ocAverageA          = SC_OPCODE_AVERAGE_A,
        ocStDevA            = SC_OPCODE_ST_DEV_A,
        ocStDevPA           = SC_OPCODE_ST_DEV_P_A,
        ocVarA              = SC_OPCODE_VAR_A,
        ocVarPA             = SC_OPCODE_VAR_P_A,
        ocEasterSunday      = SC_OPCODE_EASTERSUNDAY,
        ocDecimal           = SC_OPCODE_DECIMAL,
        ocConvert           = SC_OPCODE_CONVERT,
        ocRoman             = SC_OPCODE_ROMAN,
        ocEnd2Par           = SC_OPCODE_END_2_PAR,
    // internes Allerlei
        ocInternalBegin     = SC_OPCODE_INTERNAL_BEGIN,
        ocGame              = SC_OPCODE_GAME,
        ocSpew              = SC_OPCODE_SPEW,
        ocTTT               = SC_OPCODE_TTT,
        ocTeam              = SC_OPCODE_TEAM,
        ocAnswer            = SC_OPCODE_ANSWER,
        ocInternalEnd       = SC_OPCODE_INTERNAL_END,
    // ab hier sind Extradaten drin
        ocDataToken1        = SC_OPCODE_DATA_TOKEN_1,
    // kein OpCode
        ocNone              = SC_OPCODE_NONE
};

#ifdef PRODUCT
// save memory since compilers tend to int an enum
typedef USHORT OpCode;
#else
// have enum names in debugger
typedef OpCodeEnum OpCode;
#endif

#endif
