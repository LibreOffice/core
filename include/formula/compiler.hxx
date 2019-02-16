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

#ifndef FORMULA_COMPILER_HXX
#define FORMULA_COMPILER_HXX

/* Central definition of OpCodes for spreadsheet functions */

/*** Special commands ***/
#define SC_OPCODE_PUSH                0     /* internal commands */
#define SC_OPCODE_CALL                1
#define SC_OPCODE_STOP                2
#define SC_OPCODE_EXTERNAL            3
#define SC_OPCODE_NAME                4
#define SC_OPCODE_EXTERNAL_REF        5
#define SC_OPCODE_IF                  6     /* jump commands */
#define SC_OPCODE_IF_ERROR            7
#define SC_OPCODE_IF_NA               8
#define SC_OPCODE_CHOOSE              9
#define SC_OPCODE_OPEN               10     /* parentheses and separators */
#define SC_OPCODE_CLOSE              11
#define SC_OPCODE_SEP                12
#define SC_OPCODE_MISSING            13     /* special OpCodes */
#define SC_OPCODE_BAD                14
#define SC_OPCODE_STRINGXML          15
#define SC_OPCODE_SPACES             16
#define SC_OPCODE_MAT_REF            17
#define SC_OPCODE_DB_AREA            18     /* additional access operators */
#define SC_OPCODE_TABLE_REF          19
#define SC_OPCODE_MACRO              20
#define SC_OPCODE_COL_ROW_NAME       21
#define SC_OPCODE_COL_ROW_NAME_AUTO  22
#define SC_OPCODE_PERCENT_SIGN       23     /* operator _follows_ value */
#define SC_OPCODE_ARRAY_OPEN         24
#define SC_OPCODE_ARRAY_CLOSE        25
#define SC_OPCODE_ARRAY_ROW_SEP      26
#define SC_OPCODE_ARRAY_COL_SEP      27     /* some convs use sep != col_sep */
#define SC_OPCODE_TABLE_REF_OPEN     28
#define SC_OPCODE_TABLE_REF_CLOSE    29
#define SC_OPCODE_TABLE_REF_ITEM_ALL      30
#define SC_OPCODE_TABLE_REF_ITEM_HEADERS  31
#define SC_OPCODE_TABLE_REF_ITEM_DATA     32
#define SC_OPCODE_TABLE_REF_ITEM_TOTALS   33
#define SC_OPCODE_TABLE_REF_ITEM_THIS_ROW 34
#define SC_OPCODE_STOP_DIV           35
#define SC_OPCODE_SKIP               36     /* used to skip raw tokens during string compilation */

/*** error constants #... ***/
#define SC_OPCODE_START_ERRORS       40
#define SC_OPCODE_ERROR_NULL         40
#define SC_OPCODE_ERROR_DIVZERO      41
#define SC_OPCODE_ERROR_VALUE        42
#define SC_OPCODE_ERROR_REF          43
#define SC_OPCODE_ERROR_NAME         44
#define SC_OPCODE_ERROR_NUM          45
#define SC_OPCODE_ERROR_NA           46
#define SC_OPCODE_STOP_ERRORS        47

/*** Binary operators ***/
#define SC_OPCODE_START_BIN_OP       50
#define SC_OPCODE_ADD                50
#define SC_OPCODE_SUB                51
#define SC_OPCODE_MUL                52
#define SC_OPCODE_DIV                53
#define SC_OPCODE_AMPERSAND          54
#define SC_OPCODE_POW                55
#define SC_OPCODE_EQUAL              56
#define SC_OPCODE_NOT_EQUAL          57
#define SC_OPCODE_LESS               58
#define SC_OPCODE_GREATER            59
#define SC_OPCODE_LESS_EQUAL         60
#define SC_OPCODE_GREATER_EQUAL      61
#define SC_OPCODE_AND                62
#define SC_OPCODE_OR                 63
#define SC_OPCODE_INTERSECT          64
#define SC_OPCODE_UNION              65
#define SC_OPCODE_RANGE              66
#define SC_OPCODE_STOP_BIN_OP        67

/* NOTE: binary and unary operators must be in sequence for compiler! */

/*** Unary operators ***/
#define SC_OPCODE_START_UN_OP        70
#define SC_OPCODE_NEG_SUB            70
#define SC_OPCODE_STOP_UN_OP         71

#define SC_OPCODE_START_FUNCTION     75

/*** Functions without parameters ***/
#define SC_OPCODE_START_NO_PAR       75
#define SC_OPCODE_PI                 75
#define SC_OPCODE_RANDOM             76
#define SC_OPCODE_TRUE               77
#define SC_OPCODE_FALSE              78
#define SC_OPCODE_GET_ACT_DATE       79
#define SC_OPCODE_GET_ACT_TIME       80
#define SC_OPCODE_NO_VALUE           81
#define SC_OPCODE_CURRENT            82
#define SC_OPCODE_STOP_NO_PAR        83

/*** Functions with one parameter ***/
#define SC_OPCODE_START_1_PAR        90
#define SC_OPCODE_DEG                90     /* trigonometric */
#define SC_OPCODE_RAD                91
#define SC_OPCODE_SIN                92
#define SC_OPCODE_COS                93
#define SC_OPCODE_TAN                94
#define SC_OPCODE_COT                95
#define SC_OPCODE_ARC_SIN            96
#define SC_OPCODE_ARC_COS            97
#define SC_OPCODE_ARC_TAN            98
#define SC_OPCODE_ARC_COT            99
#define SC_OPCODE_SIN_HYP           100
#define SC_OPCODE_COS_HYP           101
#define SC_OPCODE_TAN_HYP           102
#define SC_OPCODE_COT_HYP           103
#define SC_OPCODE_ARC_SIN_HYP       104     /* transcendent */
#define SC_OPCODE_ARC_COS_HYP       105
#define SC_OPCODE_ARC_TAN_HYP       106
#define SC_OPCODE_ARC_COT_HYP       107
#define SC_OPCODE_COSECANT          108
#define SC_OPCODE_SECANT            109
#define SC_OPCODE_COSECANT_HYP      110
#define SC_OPCODE_SECANT_HYP        111
#define SC_OPCODE_EXP               112
#define SC_OPCODE_LN                113
#define SC_OPCODE_SQRT              114
#define SC_OPCODE_FACT              115
#define SC_OPCODE_GET_YEAR          116     /* date and time */
#define SC_OPCODE_GET_MONTH         117
#define SC_OPCODE_GET_DAY           118
#define SC_OPCODE_GET_HOUR          119
#define SC_OPCODE_GET_MIN           120
#define SC_OPCODE_GET_SEC           121
#define SC_OPCODE_PLUS_MINUS        122     /* miscellaneous */
#define SC_OPCODE_ABS               123
#define SC_OPCODE_INT               124
#define SC_OPCODE_PHI               125
#define SC_OPCODE_GAUSS             126
#define SC_OPCODE_IS_EMPTY          127     /* obtain type */
#define SC_OPCODE_IS_STRING         128
#define SC_OPCODE_IS_NON_STRING     129
#define SC_OPCODE_IS_LOGICAL        130
#define SC_OPCODE_TYPE              131
#define SC_OPCODE_IS_REF            132
#define SC_OPCODE_IS_VALUE          133
#define SC_OPCODE_IS_FORMULA        134
#define SC_OPCODE_IS_NV             135
#define SC_OPCODE_IS_ERR            136
#define SC_OPCODE_IS_ERROR          137
#define SC_OPCODE_IS_EVEN           138
#define SC_OPCODE_IS_ODD            139
#define SC_OPCODE_N                 140
#define SC_OPCODE_GET_DATE_VALUE    141     /* string functions */
#define SC_OPCODE_GET_TIME_VALUE    142
#define SC_OPCODE_CODE              143
#define SC_OPCODE_TRIM              144
#define SC_OPCODE_UPPER             145
#define SC_OPCODE_PROPER            146
#define SC_OPCODE_LOWER             147
#define SC_OPCODE_LEN               148
#define SC_OPCODE_T                 149     /* miscellaneous, part 21 */
#define SC_OPCODE_VALUE             150
#define SC_OPCODE_CLEAN             151
#define SC_OPCODE_CHAR              152
#define SC_OPCODE_LOG10             153
#define SC_OPCODE_EVEN              154
#define SC_OPCODE_ODD               155
#define SC_OPCODE_STD_NORM_DIST     156
#define SC_OPCODE_FISHER            157
#define SC_OPCODE_FISHER_INV        158
#define SC_OPCODE_S_NORM_INV        159
#define SC_OPCODE_GAMMA_LN          160
#define SC_OPCODE_ERROR_TYPE        161
#define SC_OPCODE_FORMULA           163
#define SC_OPCODE_ARABIC            164
#define SC_OPCODE_INFO              165
#define SC_OPCODE_BAHTTEXT          166
#define SC_OPCODE_JIS               167
#define SC_OPCODE_ASC               168
#define SC_OPCODE_UNICODE           169
#define SC_OPCODE_UNICHAR           170
#define SC_OPCODE_GAMMA             171
#define SC_OPCODE_GAMMA_LN_MS       172
#define SC_OPCODE_ERF_MS            173
#define SC_OPCODE_ERFC_MS           174
#define SC_OPCODE_ERROR_TYPE_ODF    175
#define SC_OPCODE_ENCODEURL         176
#define SC_OPCODE_ISOWEEKNUM        177
#define SC_OPCODE_NOT               178
#define SC_OPCODE_NEG               179
#define SC_OPCODE_STOP_1_PAR        180

/*** Functions with more than one parameters ***/
#define SC_OPCODE_START_2_PAR       201
#define SC_OPCODE_ARC_TAN_2         201
#define SC_OPCODE_CEIL              202
#define SC_OPCODE_FLOOR             203
#define SC_OPCODE_ROUND             204
#define SC_OPCODE_ROUND_UP          205
#define SC_OPCODE_ROUND_DOWN        206
#define SC_OPCODE_TRUNC             207
#define SC_OPCODE_LOG               208
#define SC_OPCODE_POWER             209
#define SC_OPCODE_GCD               210
#define SC_OPCODE_LCM               211
#define SC_OPCODE_MOD               212
#define SC_OPCODE_SUM_PRODUCT       213
#define SC_OPCODE_SUM_SQ            214
#define SC_OPCODE_SUM_X2MY2         215
#define SC_OPCODE_SUM_X2DY2         216
#define SC_OPCODE_SUM_XMY2          217
#define SC_OPCODE_GET_DATE          218
#define SC_OPCODE_GET_TIME          219
#define SC_OPCODE_GET_DIFF_DATE     220
#define SC_OPCODE_GET_DIFF_DATE_360 221
#define SC_OPCODE_MIN               222
#define SC_OPCODE_MAX               223
#define SC_OPCODE_SUM               224
#define SC_OPCODE_PRODUCT           225
#define SC_OPCODE_AVERAGE           226
#define SC_OPCODE_COUNT             227
#define SC_OPCODE_COUNT_2           228
#define SC_OPCODE_NPV               229
#define SC_OPCODE_IRR               230
#define SC_OPCODE_VAR               231
#define SC_OPCODE_VAR_P             232
#define SC_OPCODE_ST_DEV            233
#define SC_OPCODE_ST_DEV_P          234
#define SC_OPCODE_B                 235
#define SC_OPCODE_NORM_DIST         236
#define SC_OPCODE_EXP_DIST          237
#define SC_OPCODE_BINOM_DIST        238
#define SC_OPCODE_POISSON_DIST      239
#define SC_OPCODE_COMBIN            240
#define SC_OPCODE_COMBIN_A          241
#define SC_OPCODE_PERMUT            242
#define SC_OPCODE_PERMUTATION_A     243
#define SC_OPCODE_PV                244
#define SC_OPCODE_SYD               245
#define SC_OPCODE_DDB               246
#define SC_OPCODE_DB                247
#define SC_OPCODE_VBD               248
#define SC_OPCODE_PDURATION         249
#define SC_OPCODE_SLN               250
#define SC_OPCODE_PMT               251
#define SC_OPCODE_COLUMNS           252
#define SC_OPCODE_ROWS              253
#define SC_OPCODE_COLUMN            254
#define SC_OPCODE_ROW               255
#define SC_OPCODE_RRI               256
#define SC_OPCODE_FV                257
#define SC_OPCODE_NPER              258
#define SC_OPCODE_RATE              259
#define SC_OPCODE_IPMT              260
#define SC_OPCODE_PPMT              261
#define SC_OPCODE_CUM_IPMT          262
#define SC_OPCODE_CUM_PRINC         263
#define SC_OPCODE_EFFECT            264
#define SC_OPCODE_NOMINAL           265
#define SC_OPCODE_SUB_TOTAL         266
#define SC_OPCODE_DB_SUM            267     /* database functions */
#define SC_OPCODE_DB_COUNT          268
#define SC_OPCODE_DB_COUNT_2        269
#define SC_OPCODE_DB_AVERAGE        270
#define SC_OPCODE_DB_GET            271
#define SC_OPCODE_DB_MAX            272
#define SC_OPCODE_DB_MIN            273
#define SC_OPCODE_DB_PRODUCT        274
#define SC_OPCODE_DB_STD_DEV        275
#define SC_OPCODE_DB_STD_DEV_P      276
#define SC_OPCODE_DB_VAR            277
#define SC_OPCODE_DB_VAR_P          278
#define SC_OPCODE_INDIRECT          279     /* management functions */
#define SC_OPCODE_ADDRESS           280
#define SC_OPCODE_MATCH             281
#define SC_OPCODE_COUNT_EMPTY_CELLS 282
#define SC_OPCODE_COUNT_IF          283
#define SC_OPCODE_SUM_IF            284
#define SC_OPCODE_LOOKUP            285
#define SC_OPCODE_V_LOOKUP          286
#define SC_OPCODE_H_LOOKUP          287
#define SC_OPCODE_MULTI_AREA        288
#define SC_OPCODE_OFFSET            289
#define SC_OPCODE_INDEX             290
#define SC_OPCODE_AREAS             291
#define SC_OPCODE_CURRENCY          292     /* string functions */
#define SC_OPCODE_REPLACE           293
#define SC_OPCODE_FIXED             294
#define SC_OPCODE_FIND              295
#define SC_OPCODE_EXACT             296
#define SC_OPCODE_LEFT              297
#define SC_OPCODE_RIGHT             298
#define SC_OPCODE_SEARCH            299
#define SC_OPCODE_MID               300
#define SC_OPCODE_TEXT              301
#define SC_OPCODE_SUBSTITUTE        302
#define SC_OPCODE_REPT              303
#define SC_OPCODE_CONCAT            304
#define SC_OPCODE_MAT_VALUE         305     /* matrix functions */
#define SC_OPCODE_MAT_DET           306
#define SC_OPCODE_MAT_INV           307
#define SC_OPCODE_MAT_MULT          308
#define SC_OPCODE_MAT_TRANS         309
#define SC_OPCODE_MATRIX_UNIT       310
#define SC_OPCODE_BACK_SOLVER       311     /* BackSolver */
#define SC_OPCODE_HYP_GEOM_DIST     312     /* statistical functions */
#define SC_OPCODE_LOG_NORM_DIST     313
#define SC_OPCODE_T_DIST            314
#define SC_OPCODE_F_DIST            315
#define SC_OPCODE_CHI_DIST          316
#define SC_OPCODE_WEIBULL           317
#define SC_OPCODE_NEG_BINOM_VERT    318
#define SC_OPCODE_CRIT_BINOM        319
#define SC_OPCODE_KURT              320
#define SC_OPCODE_HAR_MEAN          321
#define SC_OPCODE_GEO_MEAN          322
#define SC_OPCODE_STANDARD          323
#define SC_OPCODE_AVE_DEV           324
#define SC_OPCODE_SKEW              325
#define SC_OPCODE_DEV_SQ            326
#define SC_OPCODE_MEDIAN            327
#define SC_OPCODE_MODAL_VALUE       328
#define SC_OPCODE_Z_TEST            329
#define SC_OPCODE_T_TEST            330
#define SC_OPCODE_RANK              331
#define SC_OPCODE_PERCENTILE        332
#define SC_OPCODE_PERCENT_RANK      333
#define SC_OPCODE_LARGE             334
#define SC_OPCODE_SMALL             335
#define SC_OPCODE_FREQUENCY         336
#define SC_OPCODE_QUARTILE          337
#define SC_OPCODE_NORM_INV          338
#define SC_OPCODE_CONFIDENCE        339
#define SC_OPCODE_F_TEST            340
#define SC_OPCODE_TRIM_MEAN         341
#define SC_OPCODE_PROB              342
#define SC_OPCODE_CORREL            343
#define SC_OPCODE_COVAR             344
#define SC_OPCODE_PEARSON           345
#define SC_OPCODE_RSQ               346
#define SC_OPCODE_STEYX             347
#define SC_OPCODE_SLOPE             348
#define SC_OPCODE_INTERCEPT         349
#define SC_OPCODE_TREND             350
#define SC_OPCODE_GROWTH            351
#define SC_OPCODE_LINEST            352
#define SC_OPCODE_LOGEST            353
#define SC_OPCODE_FORECAST          354
#define SC_OPCODE_CHI_INV           355
#define SC_OPCODE_GAMMA_DIST        356
#define SC_OPCODE_GAMMA_INV         357
#define SC_OPCODE_T_INV             358
#define SC_OPCODE_F_INV             359
#define SC_OPCODE_CHI_TEST          360
#define SC_OPCODE_LOG_INV           361
#define SC_OPCODE_TABLE_OP          362
#define SC_OPCODE_BETA_DIST         363
#define SC_OPCODE_BETA_INV          364
#define SC_OPCODE_WEEK              365     /* miscellaneous */
#define SC_OPCODE_GET_DAY_OF_WEEK   366
#define SC_OPCODE_NO_NAME           367
#define SC_OPCODE_STYLE             368
#define SC_OPCODE_DDE               369
#define SC_OPCODE_BASE              370
#define SC_OPCODE_SHEET             371
#define SC_OPCODE_SHEETS            372
#define SC_OPCODE_MIN_A             373
#define SC_OPCODE_MAX_A             374
#define SC_OPCODE_AVERAGE_A         375
#define SC_OPCODE_ST_DEV_A          376
#define SC_OPCODE_ST_DEV_P_A        377
#define SC_OPCODE_VAR_A             378
#define SC_OPCODE_VAR_P_A           379
#define SC_OPCODE_EASTERSUNDAY      380
#define SC_OPCODE_DECIMAL           381
#define SC_OPCODE_CONVERT_OOO       382
#define SC_OPCODE_ROMAN             383
#define SC_OPCODE_MIRR              384
#define SC_OPCODE_CELL              385
#define SC_OPCODE_ISPMT             386
#define SC_OPCODE_HYPERLINK         387
// free: 388
// free: 389
#define SC_OPCODE_GET_PIVOT_DATA    390
#define SC_OPCODE_EUROCONVERT       391
#define SC_OPCODE_NUMBERVALUE       392
#define SC_OPCODE_CHISQ_DIST        393
#define SC_OPCODE_CHISQ_INV         394
#define SC_OPCODE_BITAND            395
#define SC_OPCODE_BITOR             396
#define SC_OPCODE_BITXOR            397
#define SC_OPCODE_BITRSHIFT         398
#define SC_OPCODE_BITLSHIFT         399
#define SC_OPCODE_GET_DATEDIF       400
#define SC_OPCODE_XOR               401
#define SC_OPCODE_AVERAGE_IF        402
#define SC_OPCODE_SUM_IFS           403
#define SC_OPCODE_AVERAGE_IFS       404
#define SC_OPCODE_COUNT_IFS         405
#define SC_OPCODE_SKEWP             406
#define SC_OPCODE_LENB              407
#define SC_OPCODE_RIGHTB            408
#define SC_OPCODE_LEFTB             409
#define SC_OPCODE_MIDB              410
#define SC_OPCODE_FILTERXML         411
#define SC_OPCODE_WEBSERVICE        412
#define SC_OPCODE_COVARIANCE_S      413
#define SC_OPCODE_COVARIANCE_P      414
#define SC_OPCODE_ST_DEV_P_MS       415
#define SC_OPCODE_ST_DEV_S          416
#define SC_OPCODE_VAR_P_MS          417
#define SC_OPCODE_VAR_S             418
#define SC_OPCODE_BETA_DIST_MS      419
#define SC_OPCODE_BETA_INV_MS       420
#define SC_OPCODE_BINOM_DIST_MS     421
#define SC_OPCODE_BINOM_INV         422
#define SC_OPCODE_CHI_DIST_MS       423
#define SC_OPCODE_CHI_INV_MS        424
#define SC_OPCODE_CHI_TEST_MS       425
#define SC_OPCODE_CHISQ_DIST_MS     426
#define SC_OPCODE_CHISQ_INV_MS      427
#define SC_OPCODE_CONFIDENCE_N      428
#define SC_OPCODE_CONFIDENCE_T      429
#define SC_OPCODE_F_DIST_LT         430
#define SC_OPCODE_F_DIST_RT         431
#define SC_OPCODE_F_INV_LT          432
#define SC_OPCODE_F_INV_RT          433
#define SC_OPCODE_F_TEST_MS         434
#define SC_OPCODE_EXP_DIST_MS       435
#define SC_OPCODE_HYP_GEOM_DIST_MS  436
#define SC_OPCODE_POISSON_DIST_MS   437
#define SC_OPCODE_WEIBULL_MS        438
#define SC_OPCODE_GAMMA_DIST_MS     439
#define SC_OPCODE_GAMMA_INV_MS      440
#define SC_OPCODE_LOG_NORM_DIST_MS  441
#define SC_OPCODE_LOG_INV_MS        442
#define SC_OPCODE_NORM_DIST_MS      443
#define SC_OPCODE_NORM_INV_MS       444
#define SC_OPCODE_STD_NORM_DIST_MS  445
#define SC_OPCODE_S_NORM_INV_MS     446
#define SC_OPCODE_T_DIST_MS         447
#define SC_OPCODE_T_DIST_RT         448
#define SC_OPCODE_T_DIST_2T         449
#define SC_OPCODE_T_INV_2T          450
#define SC_OPCODE_T_INV_MS          451
#define SC_OPCODE_T_TEST_MS         452
#define SC_OPCODE_PERCENTILE_INC    453
#define SC_OPCODE_PERCENT_RANK_INC  454
#define SC_OPCODE_QUARTILE_INC      455
#define SC_OPCODE_RANK_EQ           456
#define SC_OPCODE_PERCENTILE_EXC    457
#define SC_OPCODE_PERCENT_RANK_EXC  458
#define SC_OPCODE_QUARTILE_EXC      459
#define SC_OPCODE_RANK_AVG          460
#define SC_OPCODE_MODAL_VALUE_MS    461
#define SC_OPCODE_MODAL_VALUE_MULTI 462
#define SC_OPCODE_NEG_BINOM_DIST_MS 463
#define SC_OPCODE_Z_TEST_MS         464
#define SC_OPCODE_CEIL_MS           465
#define SC_OPCODE_CEIL_ISO          466
#define SC_OPCODE_FLOOR_MS          467
#define SC_OPCODE_NETWORKDAYS_MS    468
#define SC_OPCODE_WORKDAY_MS        469
#define SC_OPCODE_AGGREGATE         470
#define SC_OPCODE_COLOR             471
#define SC_OPCODE_CEIL_MATH         472
#define SC_OPCODE_CEIL_PRECISE      473
#define SC_OPCODE_NETWORKDAYS       474
#define SC_OPCODE_FLOOR_MATH        475
#define SC_OPCODE_FLOOR_PRECISE     476
#define SC_OPCODE_RAWSUBTRACT       477
#define SC_OPCODE_WEEKNUM_OOO       478
#define SC_OPCODE_FORECAST_ETS_ADD  479
#define SC_OPCODE_FORECAST_ETS_SEA  480
#define SC_OPCODE_FORECAST_ETS_MUL  481
#define SC_OPCODE_FORECAST_ETS_PIA  482
#define SC_OPCODE_FORECAST_ETS_PIM  483
#define SC_OPCODE_FORECAST_ETS_STA  484
#define SC_OPCODE_FORECAST_ETS_STM  485
#define SC_OPCODE_FORECAST_LIN      486
#define SC_OPCODE_CONCAT_MS         487
#define SC_OPCODE_TEXTJOIN_MS       488
#define SC_OPCODE_IFS_MS            489
#define SC_OPCODE_SWITCH_MS         490
#define SC_OPCODE_MINIFS_MS         491
#define SC_OPCODE_MAXIFS_MS         492
#define SC_OPCODE_ROUNDSIG          493
#define SC_OPCODE_REPLACEB          494
#define SC_OPCODE_FINDB             495
#define SC_OPCODE_SEARCHB           496
#define SC_OPCODE_REGEX             497
#define SC_OPCODE_FOURIER           498
#define SC_OPCODE_STOP_2_PAR        499     /* last function with two or more parameters' OpCode + 1 */

#define SC_OPCODE_STOP_FUNCTION     SC_OPCODE_STOP_2_PAR            /* last function's OpCode + 1 */
#define SC_OPCODE_LAST_OPCODE_ID    (SC_OPCODE_STOP_FUNCTION - 1)   /* last OpCode */

/*** Internal ***/
#define SC_OPCODE_INTERNAL_BEGIN   9999
#define SC_OPCODE_TTT              9999
#define SC_OPCODE_DEBUG_VAR       10000
#define SC_OPCODE_INTERNAL_END    10000

/*** from here on ExtraData contained ***/
#define SC_OPCODE_DATA_TOKEN_1    10001

#define SC_OPCODE_NONE           0xFFFF

#endif /* FORMULA_COMPILER_HRC */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
