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

#ifndef INCLUDED_FORMULA_OPCODE_HXX
#define INCLUDED_FORMULA_OPCODE_HXX

#include <sstream>

#include <formula/compiler.hxx>
#include <sal/types.h>

enum OpCode : sal_uInt16
{
    // Special commands
        ocPush              = SC_OPCODE_PUSH,
        ocCall              = SC_OPCODE_CALL,
        ocStop              = SC_OPCODE_STOP,
        ocExternal          = SC_OPCODE_EXTERNAL,
        ocName              = SC_OPCODE_NAME,
    // Jump commands
        ocIf                = SC_OPCODE_IF,
        ocIfError           = SC_OPCODE_IF_ERROR,
        ocIfNA              = SC_OPCODE_IF_NA,
        ocChoose            = SC_OPCODE_CHOOSE,
    // Parentheses and separators
        ocOpen              = SC_OPCODE_OPEN,
        ocClose             = SC_OPCODE_CLOSE,
        ocTableRefOpen      = SC_OPCODE_TABLE_REF_OPEN,
        ocTableRefClose     = SC_OPCODE_TABLE_REF_CLOSE,
        ocSep               = SC_OPCODE_SEP,
        ocArrayOpen         = SC_OPCODE_ARRAY_OPEN,
        ocArrayClose        = SC_OPCODE_ARRAY_CLOSE,
        ocArrayRowSep       = SC_OPCODE_ARRAY_ROW_SEP,
        ocArrayColSep       = SC_OPCODE_ARRAY_COL_SEP,
    // Special OpCodes
        ocMissing           = SC_OPCODE_MISSING,
        ocBad               = SC_OPCODE_BAD,
        ocStringXML         = SC_OPCODE_STRINGXML,
        ocSpaces            = SC_OPCODE_SPACES,
        ocMatRef            = SC_OPCODE_MAT_REF,
        ocTableRefItemAll     = SC_OPCODE_TABLE_REF_ITEM_ALL,
        ocTableRefItemHeaders = SC_OPCODE_TABLE_REF_ITEM_HEADERS,
        ocTableRefItemData    = SC_OPCODE_TABLE_REF_ITEM_DATA,
        ocTableRefItemTotals  = SC_OPCODE_TABLE_REF_ITEM_TOTALS,
        ocTableRefItemThisRow = SC_OPCODE_TABLE_REF_ITEM_THIS_ROW,
        ocSkip              = SC_OPCODE_SKIP,
    // Access commands
        ocDBArea            = SC_OPCODE_DB_AREA,
        ocTableRef          = SC_OPCODE_TABLE_REF,
        ocMacro             = SC_OPCODE_MACRO,
        ocColRowName        = SC_OPCODE_COL_ROW_NAME,
        ocColRowNameAuto    = SC_OPCODE_COL_ROW_NAME_AUTO,
    // Percent operator _follows_ value
        ocPercentSign       = SC_OPCODE_PERCENT_SIGN,
    // Error constants
        ocErrNull           = SC_OPCODE_ERROR_NULL,
        ocErrDivZero        = SC_OPCODE_ERROR_DIVZERO,
        ocErrValue          = SC_OPCODE_ERROR_VALUE,
        ocErrRef            = SC_OPCODE_ERROR_REF,
        ocErrName           = SC_OPCODE_ERROR_NAME,
        ocErrNum            = SC_OPCODE_ERROR_NUM,
        ocErrNA             = SC_OPCODE_ERROR_NA,
    // Binary operators
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
        ocXor               = SC_OPCODE_XOR,
        ocIntersect         = SC_OPCODE_INTERSECT,
        ocUnion             = SC_OPCODE_UNION,
        ocRange             = SC_OPCODE_RANGE,
    // Unary operators
        ocNegSub            = SC_OPCODE_NEG_SUB,
    // Functions with no parameters
        ocPi                = SC_OPCODE_PI,
        ocRandom            = SC_OPCODE_RANDOM,
        ocTrue              = SC_OPCODE_TRUE,
        ocFalse             = SC_OPCODE_FALSE,
        ocGetActDate        = SC_OPCODE_GET_ACT_DATE,
        ocGetActTime        = SC_OPCODE_GET_ACT_TIME,
        ocNotAvail          = SC_OPCODE_NO_VALUE,
        ocCurrent           = SC_OPCODE_CURRENT,
    // Functions with one parameter
        ocNot               = SC_OPCODE_NOT,
        ocNeg               = SC_OPCODE_NEG,
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
        ocCosecant          = SC_OPCODE_COSECANT,
        ocSecant            = SC_OPCODE_SECANT,
        ocCosecantHyp       = SC_OPCODE_COSECANT_HYP,
        ocSecantHyp         = SC_OPCODE_SECANT_HYP,
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
        ocCell              = SC_OPCODE_CELL,
        ocIsRef             = SC_OPCODE_IS_REF,
        ocIsValue           = SC_OPCODE_IS_VALUE,
        ocIsFormula         = SC_OPCODE_IS_FORMULA,
        ocIsNA              = SC_OPCODE_IS_NV,
        ocIsErr             = SC_OPCODE_IS_ERR,
        ocIsError           = SC_OPCODE_IS_ERROR,
        ocIsEven            = SC_OPCODE_IS_EVEN,
        ocIsOdd             = SC_OPCODE_IS_ODD,
        ocN                 = SC_OPCODE_N,
    // String functions
        ocGetDateValue      = SC_OPCODE_GET_DATE_VALUE,
        ocGetTimeValue      = SC_OPCODE_GET_TIME_VALUE,
        ocCode              = SC_OPCODE_CODE,
        ocTrim              = SC_OPCODE_TRIM,
        ocUpper             = SC_OPCODE_UPPER,
        ocProper            = SC_OPCODE_PROPER,
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
        ocStdNormDist_MS    = SC_OPCODE_STD_NORM_DIST_MS,
        ocFisher            = SC_OPCODE_FISHER,
        ocFisherInv         = SC_OPCODE_FISHER_INV,
        ocSNormInv          = SC_OPCODE_S_NORM_INV,
        ocSNormInv_MS       = SC_OPCODE_S_NORM_INV_MS,
        ocGammaLn           = SC_OPCODE_GAMMA_LN,
        ocGammaLn_MS        = SC_OPCODE_GAMMA_LN_MS,
        ocGamma             = SC_OPCODE_GAMMA,
        ocErrorType         = SC_OPCODE_ERROR_TYPE,
        ocErrorType_ODF     = SC_OPCODE_ERROR_TYPE_ODF,
        ocFormula           = SC_OPCODE_FORMULA,
        ocArabic            = SC_OPCODE_ARABIC,
        ocInfo              = SC_OPCODE_INFO,
        ocBahtText          = SC_OPCODE_BAHTTEXT,
        ocJis               = SC_OPCODE_JIS,
        ocAsc               = SC_OPCODE_ASC,
        ocUnicode           = SC_OPCODE_UNICODE,
        ocUnichar           = SC_OPCODE_UNICHAR,
    // Functions with more than one parameters
        ocArcTan2           = SC_OPCODE_ARC_TAN_2,
        ocCeil              = SC_OPCODE_CEIL,
        ocCeil_MS           = SC_OPCODE_CEIL_MS,
        ocCeil_Precise      = SC_OPCODE_CEIL_PRECISE,
        ocCeil_ISO          = SC_OPCODE_CEIL_ISO,
        ocCeil_Math         = SC_OPCODE_CEIL_MATH,
        ocFloor             = SC_OPCODE_FLOOR,
        ocFloor_MS          = SC_OPCODE_FLOOR_MS,
        ocFloor_Math        = SC_OPCODE_FLOOR_MATH,
        ocFloor_Precise     = SC_OPCODE_FLOOR_PRECISE,
        ocRound             = SC_OPCODE_ROUND,
        ocRoundUp           = SC_OPCODE_ROUND_UP,
        ocRoundDown         = SC_OPCODE_ROUND_DOWN,
        ocTrunc             = SC_OPCODE_TRUNC,
        ocLog               = SC_OPCODE_LOG,
        ocPower             = SC_OPCODE_POWER,
        ocGCD               = SC_OPCODE_GCD,
        ocLCM               = SC_OPCODE_LCM,
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
        ocGetDateDif        = SC_OPCODE_GET_DATEDIF,
        ocMin               = SC_OPCODE_MIN,
        ocMax               = SC_OPCODE_MAX,
        ocSum               = SC_OPCODE_SUM,
        ocProduct           = SC_OPCODE_PRODUCT,
        ocAverage           = SC_OPCODE_AVERAGE,
        ocCount             = SC_OPCODE_COUNT,
        ocCount2            = SC_OPCODE_COUNT_2,
        ocNPV               = SC_OPCODE_NPV,
        ocIRR               = SC_OPCODE_IRR,
        ocMIRR              = SC_OPCODE_MIRR,
        ocISPMT             = SC_OPCODE_ISPMT,
        ocVar               = SC_OPCODE_VAR,
        ocVarP              = SC_OPCODE_VAR_P,
        ocVarP_MS           = SC_OPCODE_VAR_P_MS,
        ocVarS              = SC_OPCODE_VAR_S,
        ocStDev             = SC_OPCODE_ST_DEV,
        ocStDevP            = SC_OPCODE_ST_DEV_P,
        ocStDevP_MS         = SC_OPCODE_ST_DEV_P_MS,
        ocStDevS            = SC_OPCODE_ST_DEV_S,
        ocB                 = SC_OPCODE_B,
        ocNormDist          = SC_OPCODE_NORM_DIST,
        ocNormDist_MS       = SC_OPCODE_NORM_DIST_MS,
        ocExpDist           = SC_OPCODE_EXP_DIST,
        ocExpDist_MS        = SC_OPCODE_EXP_DIST_MS,
        ocBinomDist         = SC_OPCODE_BINOM_DIST,
        ocBinomDist_MS      = SC_OPCODE_BINOM_DIST_MS,
        ocBinomInv          = SC_OPCODE_BINOM_INV,
        ocPoissonDist       = SC_OPCODE_POISSON_DIST,
        ocPoissonDist_MS    = SC_OPCODE_POISSON_DIST_MS,
        ocCombin            = SC_OPCODE_COMBIN,
        ocCombinA           = SC_OPCODE_COMBIN_A,
        ocPermut            = SC_OPCODE_PERMUT,
        ocPermutationA      = SC_OPCODE_PERMUTATION_A,
        ocPV                = SC_OPCODE_PV,
        ocSYD               = SC_OPCODE_SYD,
        ocDDB               = SC_OPCODE_DDB,
        ocDB                = SC_OPCODE_DB,
        ocVBD               = SC_OPCODE_VBD,
        ocPDuration         = SC_OPCODE_PDURATION,
        ocSLN               = SC_OPCODE_SLN,
        ocPMT               = SC_OPCODE_PMT,
        ocColumns           = SC_OPCODE_COLUMNS,
        ocRows              = SC_OPCODE_ROWS,
        ocColumn            = SC_OPCODE_COLUMN,
        ocRow               = SC_OPCODE_ROW,
        ocRRI               = SC_OPCODE_RRI,
        ocFV                = SC_OPCODE_FV,
        ocNper              = SC_OPCODE_NPER,
        ocRate              = SC_OPCODE_RATE,
        ocIpmt              = SC_OPCODE_IPMT,
        ocPpmt              = SC_OPCODE_PPMT,
        ocCumIpmt           = SC_OPCODE_CUM_IPMT,
        ocCumPrinc          = SC_OPCODE_CUM_PRINC,
        ocEffect            = SC_OPCODE_EFFECT,
        ocNominal           = SC_OPCODE_NOMINAL,
        ocSubTotal          = SC_OPCODE_SUB_TOTAL,
        ocRawSubtract       = SC_OPCODE_RAWSUBTRACT,
        ocIfs_MS            = SC_OPCODE_IFS_MS,
        ocSwitch_MS         = SC_OPCODE_SWITCH_MS,
        ocMinIfs_MS         = SC_OPCODE_MINIFS_MS,
        ocMaxIfs_MS         = SC_OPCODE_MAXIFS_MS,
        ocRoundSig          = SC_OPCODE_ROUNDSIG,
    // Database functions
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
    // Management functions
        ocIndirect          = SC_OPCODE_INDIRECT,
        ocAddress           = SC_OPCODE_ADDRESS,
        ocMatch             = SC_OPCODE_MATCH,
        ocCountEmptyCells   = SC_OPCODE_COUNT_EMPTY_CELLS,
        ocCountIf           = SC_OPCODE_COUNT_IF,
        ocSumIf             = SC_OPCODE_SUM_IF,
        ocAverageIf         = SC_OPCODE_AVERAGE_IF,
        ocSumIfs            = SC_OPCODE_SUM_IFS,
        ocAverageIfs        = SC_OPCODE_AVERAGE_IFS,
        ocCountIfs          = SC_OPCODE_COUNT_IFS,
        ocLookup            = SC_OPCODE_LOOKUP,
        ocVLookup           = SC_OPCODE_V_LOOKUP,
        ocHLookup           = SC_OPCODE_H_LOOKUP,
        ocMultiArea         = SC_OPCODE_MULTI_AREA,
        ocOffset            = SC_OPCODE_OFFSET,
        ocIndex             = SC_OPCODE_INDEX,
        ocAreas             = SC_OPCODE_AREAS,
    // String functions
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
        ocConcat_MS         = SC_OPCODE_CONCAT_MS,
        ocTextJoin_MS       = SC_OPCODE_TEXTJOIN_MS,
        ocLenB              = SC_OPCODE_LENB,
        ocRightB            = SC_OPCODE_RIGHTB,
        ocLeftB             = SC_OPCODE_LEFTB,
        ocMidB              = SC_OPCODE_MIDB,
        ocReplaceB          = SC_OPCODE_REPLACEB,
        ocFindB             = SC_OPCODE_FINDB,
        ocSearchB           = SC_OPCODE_SEARCHB,
        ocNumberValue       = SC_OPCODE_NUMBERVALUE,
        ocRegex             = SC_OPCODE_REGEX,
    // Matrix functions
        ocMatValue          = SC_OPCODE_MAT_VALUE,
        ocMatDet            = SC_OPCODE_MAT_DET,
        ocMatInv            = SC_OPCODE_MAT_INV,
        ocMatMult           = SC_OPCODE_MAT_MULT,
        ocMatTrans          = SC_OPCODE_MAT_TRANS,
        ocMatrixUnit        = SC_OPCODE_MATRIX_UNIT,
    // BackSolver
        ocBackSolver        = SC_OPCODE_BACK_SOLVER,
    // Statistical functions
        ocHypGeomDist       = SC_OPCODE_HYP_GEOM_DIST,
        ocHypGeomDist_MS    = SC_OPCODE_HYP_GEOM_DIST_MS,
        ocLogNormDist       = SC_OPCODE_LOG_NORM_DIST,
        ocLogNormDist_MS    = SC_OPCODE_LOG_NORM_DIST_MS,
        ocTDist             = SC_OPCODE_T_DIST,
        ocTDist_MS          = SC_OPCODE_T_DIST_MS,
        ocTDist_RT          = SC_OPCODE_T_DIST_RT,
        ocTDist_2T          = SC_OPCODE_T_DIST_2T,
        ocFDist             = SC_OPCODE_F_DIST,
        ocFDist_LT          = SC_OPCODE_F_DIST_LT,
        ocFDist_RT          = SC_OPCODE_F_DIST_RT,
        ocChiDist           = SC_OPCODE_CHI_DIST,
        ocChiDist_MS        = SC_OPCODE_CHI_DIST_MS,
        ocChiSqDist         = SC_OPCODE_CHISQ_DIST,
        ocChiSqDist_MS      = SC_OPCODE_CHISQ_DIST_MS,
        ocChiSqInv          = SC_OPCODE_CHISQ_INV,
        ocChiSqInv_MS       = SC_OPCODE_CHISQ_INV_MS,
        ocWeibull           = SC_OPCODE_WEIBULL,
        ocWeibull_MS        = SC_OPCODE_WEIBULL_MS,
        ocNegBinomVert      = SC_OPCODE_NEG_BINOM_VERT,
        ocNegBinomDist_MS   = SC_OPCODE_NEG_BINOM_DIST_MS,
        ocCritBinom         = SC_OPCODE_CRIT_BINOM,
        ocKurt              = SC_OPCODE_KURT,
        ocHarMean           = SC_OPCODE_HAR_MEAN,
        ocGeoMean           = SC_OPCODE_GEO_MEAN,
        ocStandard          = SC_OPCODE_STANDARD,
        ocAveDev            = SC_OPCODE_AVE_DEV,
        ocSkew              = SC_OPCODE_SKEW,
        ocSkewp             = SC_OPCODE_SKEWP,
        ocDevSq             = SC_OPCODE_DEV_SQ,
        ocMedian            = SC_OPCODE_MEDIAN,
        ocModalValue        = SC_OPCODE_MODAL_VALUE,
        ocModalValue_MS     = SC_OPCODE_MODAL_VALUE_MS,
        ocModalValue_Multi  = SC_OPCODE_MODAL_VALUE_MULTI,
        ocZTest             = SC_OPCODE_Z_TEST,
        ocZTest_MS          = SC_OPCODE_Z_TEST_MS,
        ocAggregate         = SC_OPCODE_AGGREGATE,
        ocTTest             = SC_OPCODE_T_TEST,
        ocTTest_MS          = SC_OPCODE_T_TEST_MS,
        ocRank              = SC_OPCODE_RANK,
        ocPercentile        = SC_OPCODE_PERCENTILE,
        ocPercentrank       = SC_OPCODE_PERCENT_RANK,
        ocPercentile_Inc    = SC_OPCODE_PERCENTILE_INC,
        ocPercentrank_Inc   = SC_OPCODE_PERCENT_RANK_INC,
        ocQuartile_Inc      = SC_OPCODE_QUARTILE_INC,
        ocRank_Eq           = SC_OPCODE_RANK_EQ,
        ocPercentile_Exc    = SC_OPCODE_PERCENTILE_EXC,
        ocPercentrank_Exc   = SC_OPCODE_PERCENT_RANK_EXC,
        ocQuartile_Exc      = SC_OPCODE_QUARTILE_EXC,
        ocRank_Avg          = SC_OPCODE_RANK_AVG,
        ocLarge             = SC_OPCODE_LARGE,
        ocSmall             = SC_OPCODE_SMALL,
        ocFrequency         = SC_OPCODE_FREQUENCY,
        ocQuartile          = SC_OPCODE_QUARTILE,
        ocNormInv           = SC_OPCODE_NORM_INV,
        ocNormInv_MS        = SC_OPCODE_NORM_INV_MS,
        ocConfidence        = SC_OPCODE_CONFIDENCE,
        ocConfidence_N      = SC_OPCODE_CONFIDENCE_N,
        ocConfidence_T      = SC_OPCODE_CONFIDENCE_T,
        ocFTest             = SC_OPCODE_F_TEST,
        ocFTest_MS          = SC_OPCODE_F_TEST_MS,
        ocTrimMean          = SC_OPCODE_TRIM_MEAN,
        ocProb              = SC_OPCODE_PROB,
        ocCorrel            = SC_OPCODE_CORREL,
        ocCovar             = SC_OPCODE_COVAR,
        ocCovarianceP       = SC_OPCODE_COVARIANCE_P,
        ocCovarianceS       = SC_OPCODE_COVARIANCE_S,
        ocPearson           = SC_OPCODE_PEARSON,
        ocRSQ               = SC_OPCODE_RSQ,
        ocSTEYX             = SC_OPCODE_STEYX,
        ocSlope             = SC_OPCODE_SLOPE,
        ocIntercept         = SC_OPCODE_INTERCEPT,
        ocTrend             = SC_OPCODE_TREND,
        ocGrowth            = SC_OPCODE_GROWTH,
        ocLinest            = SC_OPCODE_LINEST,
        ocLogest            = SC_OPCODE_LOGEST,
        ocForecast          = SC_OPCODE_FORECAST,
        ocForecast_ETS_ADD  = SC_OPCODE_FORECAST_ETS_ADD,
        ocForecast_ETS_SEA  = SC_OPCODE_FORECAST_ETS_SEA,
        ocForecast_ETS_MUL  = SC_OPCODE_FORECAST_ETS_MUL,
        ocForecast_ETS_PIA  = SC_OPCODE_FORECAST_ETS_PIA,
        ocForecast_ETS_PIM  = SC_OPCODE_FORECAST_ETS_PIM,
        ocForecast_ETS_STA  = SC_OPCODE_FORECAST_ETS_STA,
        ocForecast_ETS_STM  = SC_OPCODE_FORECAST_ETS_STM,
        ocForecast_LIN      = SC_OPCODE_FORECAST_LIN,
        ocChiInv            = SC_OPCODE_CHI_INV,
        ocChiInv_MS         = SC_OPCODE_CHI_INV_MS,
        ocGammaDist         = SC_OPCODE_GAMMA_DIST,
        ocGammaDist_MS      = SC_OPCODE_GAMMA_DIST_MS,
        ocGammaInv          = SC_OPCODE_GAMMA_INV,
        ocGammaInv_MS       = SC_OPCODE_GAMMA_INV_MS,
        ocTInv              = SC_OPCODE_T_INV,
        ocTInv_2T           = SC_OPCODE_T_INV_2T,
        ocTInv_MS           = SC_OPCODE_T_INV_MS,
        ocFInv              = SC_OPCODE_F_INV,
        ocFInv_LT           = SC_OPCODE_F_INV_LT,
        ocFInv_RT           = SC_OPCODE_F_INV_RT,
        ocChiTest           = SC_OPCODE_CHI_TEST,
        ocChiTest_MS        = SC_OPCODE_CHI_TEST_MS,
        ocLogInv            = SC_OPCODE_LOG_INV,
        ocLogInv_MS         = SC_OPCODE_LOG_INV_MS,
        ocTableOp           = SC_OPCODE_TABLE_OP,
        ocBetaDist          = SC_OPCODE_BETA_DIST,
        ocBetaInv           = SC_OPCODE_BETA_INV,
        ocBetaDist_MS       = SC_OPCODE_BETA_DIST_MS,
        ocBetaInv_MS        = SC_OPCODE_BETA_INV_MS,
    // Bit functions
        ocBitAnd            = SC_OPCODE_BITAND,
        ocBitOr             = SC_OPCODE_BITOR,
        ocBitXor            = SC_OPCODE_BITXOR,
        ocBitRshift         = SC_OPCODE_BITRSHIFT,
        ocBitLshift         = SC_OPCODE_BITLSHIFT,
    // miscellaneous
        ocWeek              = SC_OPCODE_WEEK,
        ocIsoWeeknum        = SC_OPCODE_ISOWEEKNUM,
        ocWeeknumOOo        = SC_OPCODE_WEEKNUM_OOO,
        ocGetDayOfWeek      = SC_OPCODE_GET_DAY_OF_WEEK,
        ocNetWorkdays       = SC_OPCODE_NETWORKDAYS,
        ocNetWorkdays_MS    = SC_OPCODE_NETWORKDAYS_MS,
        ocWorkday_MS        = SC_OPCODE_WORKDAY_MS,
        ocNoName            = SC_OPCODE_NO_NAME,
        ocStyle             = SC_OPCODE_STYLE,
        ocDde               = SC_OPCODE_DDE,
        ocBase              = SC_OPCODE_BASE,
        ocSheet             = SC_OPCODE_SHEET,
        ocSheets            = SC_OPCODE_SHEETS,
        ocMinA              = SC_OPCODE_MIN_A,
        ocMaxA              = SC_OPCODE_MAX_A,
        ocAverageA          = SC_OPCODE_AVERAGE_A,
        ocStDevA            = SC_OPCODE_ST_DEV_A,
        ocStDevPA           = SC_OPCODE_ST_DEV_P_A,
        ocVarA              = SC_OPCODE_VAR_A,
        ocVarPA             = SC_OPCODE_VAR_P_A,
        ocEasterSunday      = SC_OPCODE_EASTERSUNDAY,
        ocDecimal           = SC_OPCODE_DECIMAL,
        ocConvertOOo        = SC_OPCODE_CONVERT_OOO,
        ocRoman             = SC_OPCODE_ROMAN,
        ocHyperLink         = SC_OPCODE_HYPERLINK,
        ocGetPivotData      = SC_OPCODE_GET_PIVOT_DATA,
        ocEuroConvert       = SC_OPCODE_EUROCONVERT,
        ocFilterXML         = SC_OPCODE_FILTERXML,
        ocWebservice        = SC_OPCODE_WEBSERVICE,
        ocColor             = SC_OPCODE_COLOR,
        ocErf_MS            = SC_OPCODE_ERF_MS,
        ocErfc_MS           = SC_OPCODE_ERFC_MS,
        ocEncodeURL         = SC_OPCODE_ENCODEURL,
        ocFourier           = SC_OPCODE_FOURIER,
    // internal stuff
        ocInternalBegin     = SC_OPCODE_INTERNAL_BEGIN,
        ocTTT               = SC_OPCODE_TTT,
        ocDebugVar          = SC_OPCODE_DEBUG_VAR,
        ocInternalEnd       = SC_OPCODE_INTERNAL_END,
    // from here on ExtraData
        ocDataToken1        = SC_OPCODE_DATA_TOKEN_1,
    // no OpCode
        ocNone              = SC_OPCODE_NONE
};

// Only to be used for debugging output. No guarantee of stability of the
// return value.

// Turn this into an operator<< when OpCode becomes a scoped enum

inline std::string OpCodeEnumToString(OpCode eCode)
{
    switch (eCode)
    {
    case ocPush: return "Push";
    case ocCall: return "Call";
    case ocStop: return "Stop";
    case ocExternal: return "External";
    case ocName: return "Name";
    case ocIf: return "If";
    case ocIfError: return "IfError";
    case ocIfNA: return "IfNA";
    case ocChoose: return "Choose";
    case ocOpen: return "Open";
    case ocClose: return "Close";
    case ocTableRefOpen: return "TableRefOpen";
    case ocTableRefClose: return "TableRefClose";
    case ocSep: return "Sep";
    case ocArrayOpen: return "ArrayOpen";
    case ocArrayClose: return "ArrayClose";
    case ocArrayRowSep: return "ArrayRowSep";
    case ocArrayColSep: return "ArrayColSep";
    case ocMissing: return "Missing";
    case ocBad: return "Bad";
    case ocStringXML: return "StringXML";
    case ocSpaces: return "Spaces";
    case ocMatRef: return "MatRef";
    case ocTableRefItemAll: return "TableRefItemAll";
    case ocTableRefItemHeaders: return "TableRefItemHeaders";
    case ocTableRefItemData: return "TableRefItemData";
    case ocTableRefItemTotals: return "TableRefItemTotals";
    case ocTableRefItemThisRow: return "TableRefItemThisRow";
    case ocSkip: return "Skip";
    case ocDBArea: return "DBArea";
    case ocTableRef: return "TableRef";
    case ocMacro: return "Macro";
    case ocColRowName: return "ColRowName";
    case ocColRowNameAuto: return "ColRowNameAuto";
    case ocPercentSign: return "PercentSign";
    case ocErrNull: return "ErrNull";
    case ocErrDivZero: return "ErrDivZero";
    case ocErrValue: return "ErrValue";
    case ocErrRef: return "ErrRef";
    case ocErrName: return "ErrName";
    case ocErrNum: return "ErrNum";
    case ocErrNA: return "ErrNA";
    case ocAdd: return "Add";
    case ocSub: return "Sub";
    case ocMul: return "Mul";
    case ocDiv: return "Div";
    case ocAmpersand: return "Ampersand";
    case ocPow: return "Pow";
    case ocEqual: return "Equal";
    case ocNotEqual: return "NotEqual";
    case ocLess: return "Less";
    case ocGreater: return "Greater";
    case ocLessEqual: return "LessEqual";
    case ocGreaterEqual: return "GreaterEqual";
    case ocAnd: return "And";
    case ocOr: return "Or";
    case ocXor: return "Xor";
    case ocIntersect: return "Intersect";
    case ocUnion: return "Union";
    case ocRange: return "Range";
    case ocNot: return "Not";
    case ocNeg: return "Neg";
    case ocNegSub: return "NegSub";
    case ocPi: return "Pi";
    case ocRandom: return "Random";
    case ocTrue: return "True";
    case ocFalse: return "False";
    case ocGetActDate: return "GetActDate";
    case ocGetActTime: return "GetActTime";
    case ocNotAvail: return "NotAvail";
    case ocCurrent: return "Current";
    case ocDeg: return "Deg";
    case ocRad: return "Rad";
    case ocSin: return "Sin";
    case ocCos: return "Cos";
    case ocTan: return "Tan";
    case ocCot: return "Cot";
    case ocArcSin: return "ArcSin";
    case ocArcCos: return "ArcCos";
    case ocArcTan: return "ArcTan";
    case ocArcCot: return "ArcCot";
    case ocSinHyp: return "SinHyp";
    case ocCosHyp: return "CosHyp";
    case ocTanHyp: return "TanHyp";
    case ocCotHyp: return "CotHyp";
    case ocArcSinHyp: return "ArcSinHyp";
    case ocArcCosHyp: return "ArcCosHyp";
    case ocArcTanHyp: return "ArcTanHyp";
    case ocArcCotHyp: return "ArcCotHyp";
    case ocCosecant: return "Cosecant";
    case ocSecant: return "Secant";
    case ocCosecantHyp: return "CosecantHyp";
    case ocSecantHyp: return "SecantHyp";
    case ocExp: return "Exp";
    case ocLn: return "Ln";
    case ocSqrt: return "Sqrt";
    case ocFact: return "Fact";
    case ocGetYear: return "GetYear";
    case ocGetMonth: return "GetMonth";
    case ocGetDay: return "GetDay";
    case ocGetHour: return "GetHour";
    case ocGetMin: return "GetMin";
    case ocGetSec: return "GetSec";
    case ocPlusMinus: return "PlusMinus";
    case ocAbs: return "Abs";
    case ocInt: return "Int";
    case ocPhi: return "Phi";
    case ocGauss: return "Gauss";
    case ocIsEmpty: return "IsEmpty";
    case ocIsString: return "IsString";
    case ocIsNonString: return "IsNonString";
    case ocIsLogical: return "IsLogical";
    case ocType: return "Type";
    case ocCell: return "Cell";
    case ocIsRef: return "IsRef";
    case ocIsValue: return "IsValue";
    case ocIsFormula: return "IsFormula";
    case ocIsNA: return "IsNA";
    case ocIsErr: return "IsErr";
    case ocIsError: return "IsError";
    case ocIsEven: return "IsEven";
    case ocIsOdd: return "IsOdd";
    case ocN: return "N";
    case ocGetDateValue: return "GetDateValue";
    case ocGetTimeValue: return "GetTimeValue";
    case ocCode: return "Code";
    case ocTrim: return "Trim";
    case ocUpper: return "Upper";
    case ocProper: return "Proper";
    case ocLower: return "Lower";
    case ocLen: return "Len";
    case ocT: return "T";
    case ocValue: return "Value";
    case ocClean: return "Clean";
    case ocChar: return "Char";
    case ocLog10: return "Log10";
    case ocEven: return "Even";
    case ocOdd: return "Odd";
    case ocStdNormDist: return "StdNormDist";
    case ocStdNormDist_MS: return "StdNormDist_MS";
    case ocFisher: return "Fisher";
    case ocFisherInv: return "FisherInv";
    case ocSNormInv: return "SNormInv";
    case ocSNormInv_MS: return "SNormInv_MS";
    case ocGammaLn: return "GammaLn";
    case ocGammaLn_MS: return "GammaLn_MS";
    case ocGamma: return "Gamma";
    case ocErrorType: return "ErrorType";
    case ocErrorType_ODF: return "ErrorType_ODF";
    case ocFormula: return "Formula";
    case ocArabic: return "Arabic";
    case ocInfo: return "Info";
    case ocBahtText: return "BahtText";
    case ocJis: return "Jis";
    case ocAsc: return "Asc";
    case ocUnicode: return "Unicode";
    case ocUnichar: return "Unichar";
    case ocArcTan2: return "ArcTan2";
    case ocCeil: return "Ceil";
    case ocCeil_MS: return "Ceil_MS";
    case ocCeil_Precise: return "Ceil_Precise";
    case ocCeil_ISO: return "Ceil_ISO";
    case ocCeil_Math: return "Ceil_Math";
    case ocFloor: return "Floor";
    case ocFloor_MS: return "Floor_MS";
    case ocFloor_Math: return "Floor_Math";
    case ocFloor_Precise: return "Floor_Precise";
    case ocRound: return "Round";
    case ocRoundUp: return "RoundUp";
    case ocRoundDown: return "RoundDown";
    case ocTrunc: return "Trunc";
    case ocLog: return "Log";
    case ocPower: return "Power";
    case ocGCD: return "GCD";
    case ocLCM: return "LCM";
    case ocMod: return "Mod";
    case ocSumProduct: return "SumProduct";
    case ocSumSQ: return "SumSQ";
    case ocSumX2MY2: return "SumX2MY2";
    case ocSumX2DY2: return "SumX2DY2";
    case ocSumXMY2: return "SumXMY2";
    case ocGetDate: return "GetDate";
    case ocGetTime: return "GetTime";
    case ocGetDiffDate: return "GetDiffDate";
    case ocGetDiffDate360: return "GetDiffDate360";
    case ocGetDateDif: return "GetDateDif";
    case ocMin: return "Min";
    case ocMax: return "Max";
    case ocSum: return "Sum";
    case ocProduct: return "Product";
    case ocAverage: return "Average";
    case ocCount: return "Count";
    case ocCount2: return "Count2";
    case ocNPV: return "NPV";
    case ocIRR: return "IRR";
    case ocMIRR: return "MIRR";
    case ocISPMT: return "ISPMT";
    case ocVar: return "Var";
    case ocVarP: return "VarP";
    case ocVarP_MS: return "VarP_MS";
    case ocVarS: return "VarS";
    case ocStDev: return "StDev";
    case ocStDevP: return "StDevP";
    case ocStDevP_MS: return "StDevP_MS";
    case ocStDevS: return "StDevS";
    case ocB: return "B";
    case ocNormDist: return "NormDist";
    case ocNormDist_MS: return "NormDist_MS";
    case ocExpDist: return "ExpDist";
    case ocExpDist_MS: return "ExpDist_MS";
    case ocBinomDist: return "BinomDist";
    case ocBinomDist_MS: return "BinomDist_MS";
    case ocBinomInv: return "BinomInv";
    case ocPoissonDist: return "PoissonDist";
    case ocPoissonDist_MS: return "PoissonDist_MS";
    case ocCombin: return "Combin";
    case ocCombinA: return "CombinA";
    case ocPermut: return "Permut";
    case ocPermutationA: return "PermutationA";
    case ocPV: return "PV";
    case ocSYD: return "SYD";
    case ocDDB: return "DDB";
    case ocDB: return "DB";
    case ocVBD: return "VBD";
    case ocPDuration: return "PDuration";
    case ocSLN: return "SLN";
    case ocPMT: return "PMT";
    case ocColumns: return "Columns";
    case ocRows: return "Rows";
    case ocColumn: return "Column";
    case ocRow: return "Row";
    case ocRRI: return "RRI";
    case ocFV: return "FV";
    case ocNper: return "Nper";
    case ocRate: return "Rate";
    case ocIpmt: return "Ipmt";
    case ocPpmt: return "Ppmt";
    case ocCumIpmt: return "CumIpmt";
    case ocCumPrinc: return "CumPrinc";
    case ocEffect: return "Effect";
    case ocNominal: return "Nominal";
    case ocSubTotal: return "SubTotal";
    case ocRawSubtract: return "RawSubtract";
    case ocIfs_MS: return "Ifs_MS";
    case ocSwitch_MS: return "Switch_MS";
    case ocMinIfs_MS: return "MinIfs_MS";
    case ocMaxIfs_MS: return "MaxIfs_MS";
    case ocRoundSig: return "RoundSig";
    case ocDBSum: return "DBSum";
    case ocDBCount: return "DBCount";
    case ocDBCount2: return "DBCount2";
    case ocDBAverage: return "DBAverage";
    case ocDBGet: return "DBGet";
    case ocDBMax: return "DBMax";
    case ocDBMin: return "DBMin";
    case ocDBProduct: return "DBProduct";
    case ocDBStdDev: return "DBStdDev";
    case ocDBStdDevP: return "DBStdDevP";
    case ocDBVar: return "DBVar";
    case ocDBVarP: return "DBVarP";
    case ocIndirect: return "Indirect";
    case ocAddress: return "Address";
    case ocMatch: return "Match";
    case ocCountEmptyCells: return "CountEmptyCells";
    case ocCountIf: return "CountIf";
    case ocSumIf: return "SumIf";
    case ocAverageIf: return "AverageIf";
    case ocSumIfs: return "SumIfs";
    case ocAverageIfs: return "AverageIfs";
    case ocCountIfs: return "CountIfs";
    case ocLookup: return "Lookup";
    case ocVLookup: return "VLookup";
    case ocHLookup: return "HLookup";
    case ocMultiArea: return "MultiArea";
    case ocOffset: return "Offset";
    case ocIndex: return "Index";
    case ocAreas: return "Areas";
    case ocCurrency: return "Currency";
    case ocReplace: return "Replace";
    case ocFixed: return "Fixed";
    case ocFind: return "Find";
    case ocExact: return "Exact";
    case ocLeft: return "Left";
    case ocRight: return "Right";
    case ocSearch: return "Search";
    case ocMid: return "Mid";
    case ocText: return "Text";
    case ocSubstitute: return "Substitute";
    case ocRept: return "Rept";
    case ocRegex: return "Regex";
    case ocConcat: return "Concat";
    case ocConcat_MS: return "Concat_MS";
    case ocTextJoin_MS: return "TextJoin_MS";
    case ocLenB: return "LenB";
    case ocRightB: return "RightB";
    case ocLeftB: return "LeftB";
    case ocMidB: return "MidB";
    case ocReplaceB: return "ReplaceB";
    case ocFindB: return "FindB";
    case ocSearchB: return "SearchB";
    case ocNumberValue: return "NumberValue";
    case ocMatValue: return "MatValue";
    case ocMatDet: return "MatDet";
    case ocMatInv: return "MatInv";
    case ocMatMult: return "MatMult";
    case ocMatTrans: return "MatTrans";
    case ocMatrixUnit: return "MatrixUnit";
    case ocBackSolver: return "BackSolver";
    case ocHypGeomDist: return "HypGeomDist";
    case ocHypGeomDist_MS: return "HypGeomDist_MS";
    case ocLogNormDist: return "LogNormDist";
    case ocLogNormDist_MS: return "LogNormDist_MS";
    case ocTDist: return "TDist";
    case ocTDist_MS: return "TDist_MS";
    case ocTDist_RT: return "TDist_RT";
    case ocTDist_2T: return "TDist_2T";
    case ocFDist: return "FDist";
    case ocFDist_LT: return "FDist_LT";
    case ocFDist_RT: return "FDist_RT";
    case ocChiDist: return "ChiDist";
    case ocChiDist_MS: return "ChiDist_MS";
    case ocChiSqDist: return "ChiSqDist";
    case ocChiSqDist_MS: return "ChiSqDist_MS";
    case ocChiSqInv: return "ChiSqInv";
    case ocChiSqInv_MS: return "ChiSqInv_MS";
    case ocWeibull: return "Weibull";
    case ocWeibull_MS: return "Weibull_MS";
    case ocNegBinomVert: return "NegBinomVert";
    case ocNegBinomDist_MS: return "NegBinomDist_MS";
    case ocCritBinom: return "CritBinom";
    case ocKurt: return "Kurt";
    case ocHarMean: return "HarMean";
    case ocGeoMean: return "GeoMean";
    case ocStandard: return "Standard";
    case ocAveDev: return "AveDev";
    case ocSkew: return "Skew";
    case ocSkewp: return "Skewp";
    case ocDevSq: return "DevSq";
    case ocMedian: return "Median";
    case ocModalValue: return "ModalValue";
    case ocModalValue_MS: return "ModalValue_MS";
    case ocModalValue_Multi: return "ModalValue_Multi";
    case ocZTest: return "ZTest";
    case ocZTest_MS: return "ZTest_MS";
    case ocAggregate: return "Aggregate";
    case ocTTest: return "TTest";
    case ocTTest_MS: return "TTest_MS";
    case ocRank: return "Rank";
    case ocPercentile: return "Percentile";
    case ocPercentrank: return "Percentrank";
    case ocPercentile_Inc: return "Percentile_Inc";
    case ocPercentrank_Inc: return "Percentrank_Inc";
    case ocQuartile_Inc: return "Quartile_Inc";
    case ocRank_Eq: return "Rank_Eq";
    case ocPercentile_Exc: return "Percentile_Exc";
    case ocPercentrank_Exc: return "Percentrank_Exc";
    case ocQuartile_Exc: return "Quartile_Exc";
    case ocRank_Avg: return "Rank_Avg";
    case ocLarge: return "Large";
    case ocSmall: return "Small";
    case ocFrequency: return "Frequency";
    case ocQuartile: return "Quartile";
    case ocNormInv: return "NormInv";
    case ocNormInv_MS: return "NormInv_MS";
    case ocConfidence: return "Confidence";
    case ocConfidence_N: return "Confidence_N";
    case ocConfidence_T: return "Confidence_T";
    case ocFTest: return "FTest";
    case ocFTest_MS: return "FTest_MS";
    case ocTrimMean: return "TrimMean";
    case ocProb: return "Prob";
    case ocCorrel: return "Correl";
    case ocCovar: return "Covar";
    case ocCovarianceP: return "CovarianceP";
    case ocCovarianceS: return "CovarianceS";
    case ocPearson: return "Pearson";
    case ocRSQ: return "RSQ";
    case ocSTEYX: return "STEYX";
    case ocSlope: return "Slope";
    case ocIntercept: return "Intercept";
    case ocTrend: return "Trend";
    case ocGrowth: return "Growth";
    case ocLinest: return "Linest";
    case ocLogest: return "Logest";
    case ocForecast: return "Forecast";
    case ocForecast_ETS_ADD: return "Forecast_ETS_ADD";
    case ocForecast_ETS_SEA: return "Forecast_ETS_SEA";
    case ocForecast_ETS_MUL: return "Forecast_ETS_MUL";
    case ocForecast_ETS_PIA: return "Forecast_ETS_PIA";
    case ocForecast_ETS_PIM: return "Forecast_ETS_PIM";
    case ocForecast_ETS_STA: return "Forecast_ETS_STA";
    case ocForecast_ETS_STM: return "Forecast_ETS_STM";
    case ocForecast_LIN: return "Forecast_LIN";
    case ocChiInv: return "ChiInv";
    case ocChiInv_MS: return "ChiInv_MS";
    case ocGammaDist: return "GammaDist";
    case ocGammaDist_MS: return "GammaDist_MS";
    case ocGammaInv: return "GammaInv";
    case ocGammaInv_MS: return "GammaInv_MS";
    case ocTInv: return "TInv";
    case ocTInv_2T: return "TInv_2T";
    case ocTInv_MS: return "TInv_MS";
    case ocFInv: return "FInv";
    case ocFInv_LT: return "FInv_LT";
    case ocFInv_RT: return "FInv_RT";
    case ocChiTest: return "ChiTest";
    case ocChiTest_MS: return "ChiTest_MS";
    case ocLogInv: return "LogInv";
    case ocLogInv_MS: return "LogInv_MS";
    case ocTableOp: return "TableOp";
    case ocBetaDist: return "BetaDist";
    case ocBetaInv: return "BetaInv";
    case ocBetaDist_MS: return "BetaDist_MS";
    case ocBetaInv_MS: return "BetaInv_MS";
    case ocBitAnd: return "BitAnd";
    case ocBitOr: return "BitOr";
    case ocBitXor: return "BitXor";
    case ocBitRshift: return "BitRshift";
    case ocBitLshift: return "BitLshift";
    case ocWeek: return "Week";
    case ocIsoWeeknum: return "IsoWeeknum";
    case ocWeeknumOOo: return "WeeknumOOo";
    case ocGetDayOfWeek: return "GetDayOfWeek";
    case ocNetWorkdays: return "NetWorkdays";
    case ocNetWorkdays_MS: return "NetWorkdays_MS";
    case ocWorkday_MS: return "Workday_MS";
    case ocNoName: return "NoName";
    case ocStyle: return "Style";
    case ocDde: return "Dde";
    case ocBase: return "Base";
    case ocSheet: return "Sheet";
    case ocSheets: return "Sheets";
    case ocMinA: return "MinA";
    case ocMaxA: return "MaxA";
    case ocAverageA: return "AverageA";
    case ocStDevA: return "StDevA";
    case ocStDevPA: return "StDevPA";
    case ocVarA: return "VarA";
    case ocVarPA: return "VarPA";
    case ocEasterSunday: return "EasterSunday";
    case ocDecimal: return "Decimal";
    case ocConvertOOo: return "ConvertOOo";
    case ocRoman: return "Roman";
    case ocHyperLink: return "HyperLink";
    case ocGetPivotData: return "GetPivotData";
    case ocEuroConvert: return "EuroConvert";
    case ocFilterXML: return "FilterXML";
    case ocWebservice: return "Webservice";
    case ocColor: return "Color";
    case ocErf_MS: return "Erf_MS";
    case ocErfc_MS: return "Erfc_MS";
    case ocEncodeURL: return "EncodeURL";
    case ocFourier: return "Fourier";
    case ocTTT: return "TTT";
    case ocDebugVar: return "DebugVar";
    case ocDataToken1: return "DataToken1";
    case ocNone: return "None";
    }
    std::ostringstream os;
    os << static_cast<int>(eCode);
    return os.str();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
