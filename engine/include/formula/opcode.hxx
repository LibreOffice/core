/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cassert>
#include <sstream>

#include <sal/types.h>

/* Central definition of OpCodes for spreadsheet functions */

enum OpCode : sal_uInt16
{
    // Special commands
        ocPush              = 0,
        // 1 used to be SC_OPCODE_CALL
        ocStop              = 2,
        ocExternal          = 3,
        ocName              = 4,
        // 5 used to be SC_OPCODE_EXTERNAL_REF
    // Jump commands
        ocIf                = 6,
        ocIfError           = 7,
        ocIfNA              = 8,
        ocChoose            = 9,
    // Parentheses and separators
        ocOpen              = 10,
        ocClose             = 11,
        ocSep               = 12,
    // Special OpCodes
        ocMissing           = 13,
        ocBad               = 14,
        ocStringXML         = 15,
        ocSpaces            = 16,
        ocWhitespace        = 17,
        ocMatRef            = 18,
    // Access commands
    /* additional access operators */
        ocDBArea            = 19,
        ocTableRef          = 20,
        ocMacro             = 21,
        ocColRowName        = 22,
        ocColRowNameAuto    = 23,
    // Percent operator _follows_ value
        ocPercentSign       = 24,
        ocArrayOpen         = 25,
        ocArrayClose        = 26,
        ocArrayRowSep       = 27,
        ocArrayColSep       = 28, /* some convs use sep != col_sep */
        ocTableRefOpen      = 29,
        ocTableRefClose     = 30,
        ocTableRefItemAll     = 31,
        ocTableRefItemHeaders = 32,
        ocTableRefItemData    = 33,
        ocTableRefItemTotals  = 34,
        ocTableRefItemThisRow = 35,
        ocStopDiv           = 36,
        ocSkip              = 37, /* used to skip raw tokens during string compilation */
        ocStringName        = 38, /* special OpCode for lambda function names */
        ocLet               = 39,
        ocDPFieldName       = 40,
    // Error constants
        ocStartErrors       = 41,
        ocErrNull           = 41,
        ocErrDivZero        = 42,
        ocErrValue          = 43,
        ocErrRef            = 44,
        ocErrName           = 45,
        ocErrNum            = 46,
        ocErrNA             = 47,
        ocErrSpill          = 48,
        ocStopErrors        = 49,
    // Binary operators
        ocStartBinaryOperators = 50,
        ocAdd               = 50,
        ocSub               = 51,
        ocMul               = 52,
        ocDiv               = 53,
        ocAmpersand         = 54,
        ocPow               = 55,
        ocEqual             = 56,
        ocNotEqual          = 57,
        ocLess              = 58,
        ocGreater           = 59,
        ocLessEqual         = 60,
        ocGreaterEqual      = 61,
        ocAnd               = 62,
        ocOr                = 63,
        ocIntersect         = 64,
        ocUnion             = 65,
        ocRange             = 66,
        ocStopBinaryOperators = 67,

    /* NOTE: binary and unary operators must be in sequence for compiler! */

    // Unary operators
        ocStartUnaryOperators = 70,
        ocNegSub            = 70,
        ocStopUnaryOperators = 71,

        ocStartFunction     = 75,

    // Functions with no parameters
        ocStartNoParameters = 75,
        ocPi                = 75,
        ocRandom            = 76,
        ocTrue              = 77,
        ocFalse             = 78,
        ocGetActDate        = 79,
        ocGetActTime        = 80,
        ocNotAvail          = 81,
        ocCurrent           = 82,
        ocRandomNV          = 83,
        ocStopNoParameters  = 84,
    // Functions with one parameter
        ocStartOneParameter = 90,
        ocDeg               = 90,  /* trigonometric */
        ocRad               = 91,
        ocSin               = 92,
        ocCos               = 93,
        ocTan               = 94,
        ocCot               = 95,
        ocArcSin            = 96,
        ocArcCos            = 97,
        ocArcTan            = 98,
        ocArcCot            = 99,
        ocSinHyp            = 100,
        ocCosHyp            = 101,
        ocTanHyp            = 102,
        ocCotHyp            = 103,
        ocArcSinHyp         = 104, /* transcendent */
        ocArcCosHyp         = 105,
        ocArcTanHyp         = 106,
        ocArcCotHyp         = 107,
        ocCosecant          = 108,
        ocSecant            = 109,
        ocCosecantHyp       = 110,
        ocSecantHyp         = 111,
        ocExp               = 112,
        ocLn                = 113,
        ocSqrt              = 114,
        ocFact              = 115,
        ocGetYear           = 116, /* date and time */
        ocGetMonth          = 117,
        ocGetDay            = 118,
        ocGetHour           = 119,
        ocGetMin            = 120,
        ocGetSec            = 121,
        ocPlusMinus         = 122, /* miscellaneous */
        ocAbs               = 123,
        ocInt               = 124,
        ocPhi               = 125,
        ocGauss             = 126,
        ocIsEmpty           = 127, /* obtain type */
        ocIsString          = 128,
        ocIsNonString       = 129,
        ocIsLogical         = 130,
        ocType              = 131,
        ocIsRef             = 132,
        ocIsValue           = 133,
        ocIsFormula         = 134,
        ocIsNA              = 135,
        ocIsErr             = 136,
        ocIsError           = 137,
        ocIsEven            = 138,
        ocIsOdd             = 139,
        ocN                 = 140,
    // String functions
        ocGetDateValue      = 141,
        ocGetTimeValue      = 142,
        ocCode              = 143,
        ocTrim              = 144,
        ocUpper             = 145,
        ocProper            = 146,
        ocLower             = 147,
        ocLen               = 148,
        ocT                 = 149, /* miscellaneous, part 21 */
        ocValue             = 150,
        ocClean             = 151,
        ocChar              = 152,
        ocLog10             = 153,
        ocEven              = 154,
        ocOdd               = 155,
        ocStdNormDist       = 156,
        ocFisher            = 157,
        ocFisherInv         = 158,
        ocSNormInv          = 159,
        ocGammaLn           = 160,
        ocErrorType         = 161,
        // 162 was ???
        ocFormula           = 163,
        ocArabic            = 164,
        ocInfo              = 165,
        ocBahtText          = 166,
        ocJis               = 167,
        ocAsc               = 168,
        ocUnicode           = 169,
        ocUnichar           = 170,
        ocGamma             = 171,
        ocGammaLn_MS        = 172,
        ocErf_MS            = 173,
        ocErfc_MS           = 174,
        ocErrorType_ODF     = 175,
        ocEncodeURL         = 176,
        ocIsoWeeknum        = 177,
        ocNot               = 178,
        ocNeg               = 179,
        ocStopOneParameter  = 180,

    // Functions with more than one parameters
        ocStartTwoParameters = 201,
        ocArcTan2           = 201,
        ocCeil              = 202,
        ocFloor             = 203,
        ocRound             = 204,
        ocRoundUp           = 205,
        ocRoundDown         = 206,
        ocTrunc             = 207,
        ocLog               = 208,
        ocPower             = 209,
        ocGCD               = 210,
        ocLCM               = 211,
        ocMod               = 212,
        ocSumProduct        = 213,
        ocSumSQ             = 214,
        ocSumX2MY2          = 215,
        ocSumX2DY2          = 216,
        ocSumXMY2           = 217,
        ocGetDate           = 218,
        ocGetTime           = 219,
        ocGetDiffDate       = 220,
        ocGetDiffDate360    = 221,
        ocMin               = 222,
        ocMax               = 223,
        ocSum               = 224,
        ocProduct           = 225,
        ocAverage           = 226,
        ocCount             = 227,
        ocCount2            = 228,
        ocNPV               = 229,
        ocIRR               = 230,
        ocVar               = 231,
        ocVarP              = 232,
        ocStDev             = 233,
        ocStDevP            = 234,
        ocB                 = 235,
        ocNormDist          = 236,
        ocExpDist           = 237,
        ocBinomDist         = 238,
        ocPoissonDist       = 239,
        ocCombin            = 240,
        ocCombinA           = 241,
        ocPermut            = 242,
        ocPermutationA      = 243,
        ocPV                = 244,
        ocSYD               = 245,
        ocDDB               = 246,
        ocDB                = 247,
        ocVBD               = 248,
        ocPDuration         = 249,
        ocSLN               = 250,
        ocPMT               = 251,
        ocColumns           = 252,
        ocRows              = 253,
        ocColumn            = 254,
        ocRow               = 255,
        ocRRI               = 256,
        ocFV                = 257,
        ocNper              = 258,
        ocRate              = 259,
        ocIpmt              = 260,
        ocPpmt              = 261,
        ocCumIpmt           = 262,
        ocCumPrinc          = 263,
        ocEffect            = 264,
        ocNominal           = 265,
        ocSubTotal          = 266,
    // Database functions
        ocDBSum             = 267,
        ocDBCount           = 268,
        ocDBCount2          = 269,
        ocDBAverage         = 270,
        ocDBGet             = 271,
        ocDBMax             = 272,
        ocDBMin             = 273,
        ocDBProduct         = 274,
        ocDBStdDev          = 275,
        ocDBStdDevP         = 276,
        ocDBVar             = 277,
        ocDBVarP            = 278,
    // Management functions
        ocIndirect          = 279,
        ocAddress           = 280,
        ocMatch             = 281,
        ocCountEmptyCells   = 282,
        ocCountIf           = 283,
        ocSumIf             = 284,
        ocLookup            = 285,
        ocVLookup           = 286,
        ocHLookup           = 287,
        ocMultiArea         = 288,
        ocOffset            = 289,
        ocIndex             = 290,
        ocAreas             = 291,
    // String functions
        ocCurrency          = 292,
        ocReplace           = 293,
        ocFixed             = 294,
        ocFind              = 295,
        ocExact             = 296,
        ocLeft              = 297,
        ocRight             = 298,
        ocSearch            = 299,
        ocMid               = 300,
        ocText              = 301,
        ocSubstitute        = 302,
        ocRept              = 303,
        ocConcat            = 304,
    // Matrix functions
        ocMatValue          = 305,
        ocMatDet            = 306,
        ocMatInv            = 307,
        ocMatMult           = 308,
        ocMatTrans          = 309,
        ocMatrixUnit        = 310,
    // BackSolver
        ocBackSolver        = 311,
    // Statistical functions
        ocHypGeomDist       = 312,
        ocLogNormDist       = 313,
        ocTDist             = 314,
        ocFDist             = 315,
        ocChiDist           = 316,
        ocWeibull           = 317,
        ocNegBinomVert      = 318,
        ocCritBinom         = 319,
        ocKurt              = 320,
        ocHarMean           = 321,
        ocGeoMean           = 322,
        ocStandard          = 323,
        ocAveDev            = 324,
        ocSkew              = 325,
        ocDevSq             = 326,
        ocMedian            = 327,
        ocModalValue        = 328,
        ocZTest             = 329,
        ocTTest             = 330,
        ocRank              = 331,
        ocPercentile        = 332,
        ocPercentrank       = 333,
        ocLarge             = 334,
        ocSmall             = 335,
        ocFrequency         = 336,
        ocQuartile          = 337,
        ocNormInv           = 338,
        ocConfidence        = 339,
        ocFTest             = 340,
        ocTrimMean          = 341,
        ocProb              = 342,
        ocCorrel            = 343,
        ocCovar             = 344,
        ocPearson           = 345,
        ocRSQ               = 346,
        ocSTEYX             = 347,
        ocSlope             = 348,
        ocIntercept         = 349,
        ocTrend             = 350,
        ocGrowth            = 351,
        ocLinest            = 352,
        ocLogest            = 353,
        ocForecast          = 354,
        ocChiInv            = 355,
        ocGammaDist         = 356,
        ocGammaInv          = 357,
        ocTInv              = 358,
        ocFInv              = 359,
        ocChiTest           = 360,
        ocLogInv            = 361,
        ocTableOp           = 362,
        ocBetaDist          = 363,
        ocBetaInv           = 364,
    // miscellaneous
        ocWeek              = 365,
        ocGetDayOfWeek      = 366,
        ocNoName            = 367,
        ocStyle             = 368,
        ocDde               = 369,
        ocBase              = 370,
        ocSheet             = 371,
        ocSheets            = 372,
        ocMinA              = 373,
        ocMaxA              = 374,
        ocAverageA          = 375,
        ocStDevA            = 376,
        ocStDevPA           = 377,
        ocVarA              = 378,
        ocVarPA             = 379,
        ocEasterSunday      = 380,
        ocDecimal           = 381,
        ocConvertOOo        = 382,
        ocRoman             = 383,
        ocMIRR              = 384,
        ocCell              = 385,
        ocISPMT             = 386,
        ocHyperLink         = 387,
        ocXLookup           = 388,
        ocXMatch            = 389,
        ocGetPivotData      = 390,
        ocEuroConvert       = 391,
        ocNumberValue       = 392,
        ocChiSqDist         = 393,
        ocChiSqInv          = 394,
    // Bit functions
        ocBitAnd            = 395,
        ocBitOr             = 396,
        ocBitXor            = 397,
        ocBitRshift         = 398,
        ocBitLshift         = 399,
        ocGetDateDif        = 400,
        ocXor               = 401,
        ocAverageIf         = 402,
        ocSumIfs            = 403,
        ocAverageIfs        = 404,
        ocCountIfs          = 405,
        ocSkewp             = 406,
        ocLenB              = 407,
        ocRightB            = 408,
        ocLeftB             = 409,
        ocMidB              = 410,
        ocFilterXML         = 411,
        ocWebservice        = 412,
        ocCovarianceS       = 413,
        ocCovarianceP       = 414,
        ocStDevP_MS         = 415,
        ocStDevS            = 416,
        ocVarP_MS           = 417,
        ocVarS              = 418,
        ocBetaDist_MS       = 419,
        ocBetaInv_MS        = 420,
        ocBinomDist_MS      = 421,
        ocBinomInv          = 422,
        ocChiDist_MS        = 423,
        ocChiInv_MS         = 424,
        ocChiTest_MS        = 425,
        ocChiSqDist_MS      = 426,
        ocChiSqInv_MS       = 427,
        ocConfidence_N      = 428,
        ocConfidence_T      = 429,
        ocFDist_LT          = 430,
        ocFDist_RT          = 431,
        ocFInv_LT           = 432,
        ocFInv_RT           = 433,
        ocFTest_MS          = 434,
        ocExpDist_MS        = 435,
        ocHypGeomDist_MS    = 436,
        ocPoissonDist_MS    = 437,
        ocWeibull_MS        = 438,
        ocGammaDist_MS      = 439,
        ocGammaInv_MS       = 440,
        ocLogNormDist_MS    = 441,
        ocLogInv_MS         = 442,
        ocNormDist_MS       = 443,
        ocNormInv_MS        = 444,
        ocStdNormDist_MS    = 445,
        ocSNormInv_MS       = 446,
        ocTDist_MS          = 447,
        ocTDist_RT          = 448,
        ocTDist_2T          = 449,
        ocTInv_2T           = 450,
        ocTInv_MS           = 451,
        ocTTest_MS          = 452,
        ocPercentile_Inc    = 453,
        ocPercentrank_Inc   = 454,
        ocQuartile_Inc      = 455,
        ocRank_Eq           = 456,
        ocPercentile_Exc    = 457,
        ocPercentrank_Exc   = 458,
        ocQuartile_Exc      = 459,
        ocRank_Avg          = 460,
        ocModalValue_MS     = 461,
        ocModalValue_Multi  = 462,
        ocNegBinomDist_MS   = 463,
        ocZTest_MS          = 464,
        ocCeil_MS           = 465,
        ocCeil_ISO          = 466,
        ocFloor_MS          = 467,
        ocNetWorkdays_MS    = 468,
        ocWorkday_MS        = 469,
        ocAggregate         = 470,
        ocColor             = 471,
        ocCeil_Math         = 472,
        ocCeil_Precise      = 473,
        ocNetWorkdays       = 474,
        ocFloor_Math        = 475,
        ocFloor_Precise     = 476,
        ocRawSubtract       = 477,
        ocWeeknumOOo        = 478,
        ocForecast_ETS_ADD  = 479,
        ocForecast_ETS_SEA  = 480,
        ocForecast_ETS_MUL  = 481,
        ocForecast_ETS_PIA  = 482,
        ocForecast_ETS_PIM  = 483,
        ocForecast_ETS_STA  = 484,
        ocForecast_ETS_STM  = 485,
        ocForecast_LIN      = 486,
        ocConcat_MS         = 487,
        ocTextJoin_MS       = 488,
        ocIfs_MS            = 489,
        ocSwitch_MS         = 490,
        ocMinIfs_MS         = 491,
        ocMaxIfs_MS         = 492,
        ocRoundSig          = 493,
        ocReplaceB          = 494,
        ocFindB             = 495,
        ocSearchB           = 496,
        ocRegex             = 497,
        ocFourier           = 498,
        ocRandbetweenNV     = 499,
        ocFilter            = 500,
        ocSort              = 501,
        ocSortBy            = 502,
        ocMatSequence       = 503,
        ocRandArray         = 504,
        ocChooseCols        = 505,
        ocChooseRows        = 506,
        ocDrop              = 507,
        ocExpand            = 508,
        ocHStack            = 509,
        ocVStack            = 510,
        ocTake              = 511,
        ocTextAfter         = 512,
        ocTextBefore        = 513,
        ocTextSplit         = 514,
        ocToCol             = 515,
        ocToRow             = 516,
        ocUnique            = 517,
        ocWrapCols          = 518,
        ocWrapRows          = 519,
        ocUDExternal        = 520, /* User-defined external function */
        ocStopTwoParameters = 521, /* last function with two or more parameters' OpCode + 1 */

        ocStopFunction      = ocStopTwoParameters,  /* last function's OpCode + 1 */
        ocLastOpcodeId      = ocStopFunction - 1, /* last OpCode */

    // internal stuff
        ocInternalBegin     = 9999,
        ocTTT               = 9999,
        ocDebugVar          = 10000,
        ocInternalEnd       = 10000,
    // from here on ExtraData
        ocDataToken1        = 10001,
    // no OpCode
        ocNone              = 0xFFFF
};

// Only to be used for debugging output. No guarantee of stability of the
// return value.

// Turn this into an operator<< when OpCode becomes a scoped enum

inline std::string OpCodeEnumToString(OpCode eCode)
{
    switch (eCode)
    {
    case ocPush: return "Push";
    case ocStop: return "Stop";
    case ocExternal: return "External";
    case ocUDExternal: return "UDExternal";
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
    case ocStringName: return "StringName";
    case ocSpaces: return "Spaces";
    case ocWhitespace: return "Whitespace";
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
    case ocDPFieldName: return "DPFieldName";
    case ocPercentSign: return "PercentSign";
    case ocErrNull: return "ErrNull";
    case ocErrDivZero: return "ErrDivZero";
    case ocErrValue: return "ErrValue";
    case ocErrRef: return "ErrRef";
    case ocErrName: return "ErrName";
    case ocErrNum: return "ErrNum";
    case ocErrNA: return "ErrNA";
    case ocErrSpill: return "ErrSpill";
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
    case ocRandomNV: return "RandomNV";
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
    case ocXMatch: return "XMatch";
    case ocCountEmptyCells: return "CountEmptyCells";
    case ocCountIf: return "CountIf";
    case ocSumIf: return "SumIf";
    case ocAverageIf: return "AverageIf";
    case ocSumIfs: return "SumIfs";
    case ocAverageIfs: return "AverageIfs";
    case ocCountIfs: return "CountIfs";
    case ocLookup: return "Lookup";
    case ocVLookup: return "VLookup";
    case ocXLookup: return "XLookup";
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
    case ocMatSequence: return "MatSequence";
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
    case ocRandbetweenNV: return "RandbetweenNV";
    case ocRandArray: return "RandArray";
    case ocFilter: return "Filter";
    case ocSort: return "Sort";
    case ocSortBy: return "SortBy";
    case ocChooseCols: return "ChooseCols";
    case ocChooseRows: return "ChooseRows";
    case ocDrop: return "Drop";
    case ocExpand: return "Expand";
    case ocHStack: return "HStack";
    case ocVStack: return "VStack";
    case ocTake: return "Take";
    case ocTextAfter: return "TextAfter";
    case ocTextBefore: return "TextBefore";
    case ocTextSplit: return "TextSplit";
    case ocToCol: return "ToCol";
    case ocToRow: return "ToRow";
    case ocUnique: return "Unique";
    case ocLet: return "Let";
    case ocWrapCols: return "WrapCols";
    case ocWrapRows: return "WrapRows";
    case ocTTT: return "TTT";
    case ocDebugVar: return "DebugVar";
    case ocDataToken1: return "DataToken1";
    case ocNone: return "None";
    default: assert(false); break; // should never happen
    }
    std::ostringstream os;
    os << static_cast<int>(eCode);
    return os.str();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
