/*************************************************************************
 *
 *  $RCSfile: compiler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gt $ $Date: 2000-09-22 13:42:36 $
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

#ifndef SC_COMPILER_HXX
#define SC_COMPILER_HXX

#include <string.h>     // wg. memcpy() und bitte nicht memory.h (WKC)

#ifndef _INC_CDATA
#define _INC_CDATA

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"                       // fuer UpdateRefMode
#endif
#ifndef SC_COMPILER_HRC
#include "compiler.hrc"                     // fuer OpCodes
#endif

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#define _NEW_COMPILER

//-----------------------------------------------

// Konstanten und Datentypen auch fuer externe Module (Interpreter etc.):

#define MAXCODE                 512         // Maximale Anzahl Tokens
#define MAXSTRLEN               256         // Maximale Laenge fuer Eingabestring
                                            // (auch Formeln), "Verlaengerung" durch
                                            // Namensvergabe moeglich
#define MAXJUMPCOUNT            32
#define MAXMATSIZE              100
#define SC_DOUBLE_MAXVALUE  1.7e307

#define errIllegalChar          501
#define errIllegalArgument      502
#define errIllegalFPOperation   503
#define errIllegalParameter     504
#define errIllegalJump          505
#define errSeparator            506
#define errPair                 507
#define errPairExpected         508
#define errOperatorExpected     509
#define errVariableExpected     510
#define errParameterExpected    511
#define errCodeOverflow         512
#define errStringOverflow       513
#define errStackOverflow        514
#define errUnknownState         515
#define errUnknownVariable      516
#define errUnknownOpCode        517
#define errUnknownStackVariable 518
#define errNoValue              519
#define errUnknownToken         520
#define errNoCode               521
#define errCircularReference    522
#define errNoConvergence        523
#define errNoRef                524
#define errNoName               525
#define errDoubleRef            526
#define errInterpOverflow       527
// wird nicht angezeigt, temporaer fuer TrackFormulas
// Zelle haengt von einer ab, die errCircularReference ist
#define errTrackFromCircRef     528

#define NOVALUE                 0x7fff


// flag values of CharTable
#define SC_COMPILER_C_ILLEGAL       0x0000
#define SC_COMPILER_C_CHAR          0x0001
#define SC_COMPILER_C_CHAR_BOOL     0x0002
#define SC_COMPILER_C_CHAR_WORD     0x0004
#define SC_COMPILER_C_CHAR_VALUE    0x0008
#define SC_COMPILER_C_CHAR_STRING   0x0010
#define SC_COMPILER_C_CHAR_DONTCARE 0x0020
#define SC_COMPILER_C_BOOL          0x0040
#define SC_COMPILER_C_WORD          0x0080      // auch in spstring.cxx dekl.
#define SC_COMPILER_C_WORD_SEP      0x0100      // und rangenam.cxx (WORD und CHAR_WORD)
#define SC_COMPILER_C_VALUE         0x0200
#define SC_COMPILER_C_VALUE_SEP     0x0400
#define SC_COMPILER_C_VALUE_EXP     0x0800
#define SC_COMPILER_C_VALUE_SIGN    0x1000
#define SC_COMPILER_C_VALUE_VALUE   0x2000
#define SC_COMPILER_C_STRING_SEP    0x4000
#define SC_COMPILER_C_NAME_SEP      0x8000      // es kann nur einen geben! '\''

#define SC_COMPILER_FILE_TAB_SEP    '#'         // 'Doc'#Tab  auch in global2.cxx


enum OpCode
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

enum StackVar
    {
        svByte,
        svDouble,
        svString,
        svSingleRef,
        svDoubleRef,
        svMatrix,
        svIndex,
        svJump,
        svExternal,                         // Byte + String

        svMissing = 0x70,                   // liefert 0 oder ""
        svErr                               // unbekannter StackType
    };

#endif

class ScDocument;
class ScMatrix;
class ScRangeData;

// Konstanten und Datentypen compilerintern:


// Ref-Flags fuer alte (bis Release 3.1) Dokumente

struct OldSingleRefBools
{
    BYTE    bRelCol;    // Flagwerte (s.u.), jeweils 2 Bits im Dateiformat
    BYTE    bRelRow;
    BYTE    bRelTab;
    BYTE    bOldFlag3D; // zwei BOOL Flags (s.u.)
};

#define SR_ABSOLUTE 0       // Absolutwert
#define SR_RELABS   1       // Relativwert als Absolutwert  (bis Release 3.1)
#define SR_RELATIVE 2       // Relativwert als Deltawert    (nach Release 3.1)
#define SR_DELETED  3       // Gel”schte Zeile/Spalte/Tabelle

#define SRF_3D      0x01    // 3D Ref, war frueher (vor 304a) der BOOL
#define SRF_RELNAME 0x02    // Ref ist aus RangeName mit relativen Angaben entstanden
#define SRF_BITS    0x03    // Maske der moeglichen Bits


struct SingleRefData        // Referenzen auf die Tabelle
{
    INT16   nCol;       // absolute Angaben
    INT16   nRow;
    INT16   nTab;
    INT16   nRelCol;    // Angaben relativ zur Position
    INT16   nRelRow;
    INT16   nRelTab;

    union
    {
        BOOL bFlags;
        struct
        {
            BOOL    bColRel     :1;
            BOOL    bColDeleted :1;
            BOOL    bRowRel     :1;
            BOOL    bRowDeleted :1;
            BOOL    bTabRel     :1;
            BOOL    bTabDeleted :1;
            BOOL    bFlag3D     :1;     // 3D-Ref
            BOOL    bRelName    :1;     // Ref aus RangeName mit relativen Angaben
        }Flags;
    };

    // kein default cTor, da in ScToken union verwendet, InitFlags setzen!
    inline  void InitFlags() { bFlags = 0; }    // alles FALSE
    // InitAddress: InitFlags und setzt Adresse
    inline  void InitAddress( const ScAddress& rAdr );
    // InitAddressRel: InitFlags und setzt Adresse, alles relativ zu rPos
    inline  void InitAddressRel( const ScAddress& rAdr, const ScAddress& rPos );
    inline  void SetColRel( BOOL bVal ) { Flags.bColRel = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsColRel() const       { return Flags.bColRel; }
    inline  void SetRowRel( BOOL bVal ) { Flags.bRowRel = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsRowRel() const       { return Flags.bRowRel; }
    inline  void SetTabRel( BOOL bVal ) { Flags.bTabRel = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsTabRel() const       { return Flags.bTabRel; }

    inline  void SetColDeleted( BOOL bVal ) { Flags.bColDeleted = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsColDeleted() const       { return Flags.bColDeleted; }
    inline  void SetRowDeleted( BOOL bVal ) { Flags.bRowDeleted = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsRowDeleted() const       { return Flags.bRowDeleted; }
    inline  void SetTabDeleted( BOOL bVal ) { Flags.bTabDeleted = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsTabDeleted() const       { return Flags.bTabDeleted; }

    inline  void SetFlag3D( BOOL bVal ) { Flags.bFlag3D = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsFlag3D() const       { return Flags.bFlag3D; }
    inline  void SetRelName( BOOL bVal )    { Flags.bRelName = (bVal ? TRUE : FALSE ); }
    inline  BOOL IsRelName() const          { return Flags.bRelName; }

    inline  BOOL Valid() const;

            void SmartRelAbs( const ScAddress& rPos );
            void CalcRelFromAbs( const ScAddress& rPos );
            void CalcAbsIfRel( const ScAddress& rPos );
            void OldBoolsToNewFlags( const OldSingleRefBools& );
            BYTE CreateStoreByteFromFlags() const;
            void CreateFlagsFromLoadByte( BYTE );
};

inline void SingleRefData::InitAddress( const ScAddress& rAdr )
{
    InitFlags();
    nCol = rAdr.Col();
    nRow = rAdr.Row();
    nTab = rAdr.Tab();
}

inline void SingleRefData::InitAddressRel( const ScAddress& rAdr,
                                            const ScAddress& rPos )
{
    InitFlags();
    nCol = rAdr.Col();
    nRow = rAdr.Row();
    nTab = rAdr.Tab();
    SetColRel( TRUE );
    SetRowRel( TRUE );
    SetTabRel( TRUE );
    CalcRelFromAbs( rPos );
}

inline BOOL SingleRefData::Valid() const
{
    return  nCol >= 0 && nCol <= MAXCOL &&
            nRow >= 0 && nRow <= MAXROW &&
            nTab >= 0 && nTab <= MAXTAB;
}


struct ComplRefData         // Komplette Referenz
{
    SingleRefData Ref1;
    SingleRefData Ref2;

    inline  void InitFlags()
        { Ref1.InitFlags(); Ref2.InitFlags(); }
    inline  void InitRange( const ScRange& rRange )
        {
            Ref1.InitAddress( rRange.aStart );
            Ref2.InitAddress( rRange.aEnd );
        }
    inline  void InitRangeRel( const ScRange& rRange, const ScAddress& rPos )
        {
            Ref1.InitAddressRel( rRange.aStart, rPos );
            Ref2.InitAddressRel( rRange.aEnd, rPos );
        }
    inline  void SmartRelAbs( const ScAddress& rPos )
        { Ref1.SmartRelAbs( rPos ); Ref2.SmartRelAbs( rPos ); }
    inline  void CalcRelFromAbs( const ScAddress& rPos )
        { Ref1.CalcRelFromAbs( rPos ); Ref2.CalcRelFromAbs( rPos ); }
    inline  void CalcAbsIfRel( const ScAddress& rPos )
        { Ref1.CalcAbsIfRel( rPos ); Ref2.CalcAbsIfRel( rPos ); }
    inline  BOOL Valid() const
        { return Ref1.Valid() && Ref2.Valid(); }
            void PutInOrder();
};


#if 0
/*
    OpCode   eOp;           // Op-Code
    StackVar eType;         // Art der Daten
    USHORT   nRefCnt;       // Ref-Count
    BOOL     bRaw;          // nicht per Clone auf passende Groesse getrimmt
 */
#endif

#ifndef PRODUCT
#define SC_TOKEN_FIX_MEMBERS    \
    OpCode   eOp;               \
    StackVar eType;             \
    USHORT   nRefCnt;           \
    BOOL     bRaw;
#else
#define SC_TOKEN_FIX_MEMBERS    \
    USHORT   eOp;               \
    USHORT   eType;             \
    USHORT   nRefCnt;           \
    BOOL     bRaw;
#endif

struct ScDoubleToken
{
private:
    SC_TOKEN_FIX_MEMBERS
public:
    union
    {   // union nur um gleiches Alignment wie ScToken sicherzustellen
        double      nValue;
        BYTE        cByte;
    };
                DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleToken );
};

struct ScToken
{
    friend class ScCompiler;
    // Friends, die temporaere Raw-Tokens auf dem Stack benutzen (und deswegen
    // den private dtor brauchen) und wissen, was sie tun..
    friend class ScTokenArray;
    friend class ScInterpreter;
private:
    SC_TOKEN_FIX_MEMBERS
public:
    union {
        double       nValue;
        ComplRefData aRef;
        BYTE         cByte;
        ScMatrix*    pMat;
        USHORT       nIndex;                // Index fuer Name-Tabelle
        sal_Unicode  cStr[ MAXSTRLEN+1 ];   // String (bis 255 Zeichen + 0)
        short        nJump[MAXJUMPCOUNT+1]; // If/Chose-Token
    };

                ScToken() : bRaw( TRUE ) {}
private:
                ~ScToken() {}               //! nur via Delete() loeschen
public:
                DECL_FIXEDMEMPOOL_NEWDEL( ScToken );        // Raw-ScToken
    StackVar    GetType()   const       { return (StackVar) eType; }
    OpCode      GetOpCode() const       { return (OpCode)   eOp;   }
    BOOL        IsFunction() const;     // reine Funktionen, keine Operatoren
    BOOL        IsMatrixFunction() const;   // ob Funktion _immer_ eine Matrix returnt
    BYTE        GetParamCount() const;
    void        NewOpCode( OpCode e )   { eOp = e; }
    void        IncRef()                { nRefCnt++;       }
    void        DecRef()                { if( !--nRefCnt ) Delete(); }
    USHORT      GetRef() const          { return nRefCnt; }
    void        Delete();

    // Diese Methoden nur auf Tokens, die nicht im TokenArray stehen,
    // anwenden, da der RefCount auf 0 gesetzt wird!
    void SetOpCode( OpCode eCode );
    void SetString( const sal_Unicode* pStr );
    void SetSingleReference( const SingleRefData& rRef );
    void SetDoubleReference( const ComplRefData& rRef );
    void SetByte( BYTE c );
    void SetDouble( double fVal );
    void SetInt( int nVal );
    void SetName( USHORT n );
    void SetMatrix( ScMatrix* p );
    void SetExternal(const sal_Unicode* pStr);
    // Diese Methoden sind hingegen OK.
    ComplRefData& GetReference();
    void SetReference( ComplRefData& rRef );

    ScToken* Clone() const; // echte Kopie!
    void Load30( SvStream& );
    void Load( SvStream&, USHORT nVer );
    void Store( SvStream& ) const;

    BOOL operator== (const ScToken& rToken) const;
    BOOL TextEqual(const ScToken& rToken) const;

    static xub_StrLen GetStrLen( const sal_Unicode* pStr ); // as long as a "string" is an array
    static size_t GetStrLenBytes( xub_StrLen nLen )
        { return nLen * sizeof(sal_Unicode); }
    static size_t GetStrLenBytes( const sal_Unicode* pStr )
        { return GetStrLenBytes( GetStrLen( pStr ) ); }
};


class ScTokenRef
{
    ScToken* p;
public:
    inline ScTokenRef() { p = NULL; }
    inline ScTokenRef( const ScTokenRef& r ) { if( ( p = r.p ) != NULL ) p->IncRef(); }
    inline ScTokenRef( ScToken *t )          { if( ( p = t ) != NULL ) t->IncRef(); }
    inline void Clear()                      { if( p ) p->DecRef(); }
    inline ~ScTokenRef()                     { if( p ) p->DecRef(); }
    inline ScTokenRef& operator=( const ScTokenRef& r ) { return *this = r.p; }
    inline ScTokenRef& operator=( ScToken* t )
    { if( t ) t->IncRef(); if( p ) p->DecRef(); p = t; return *this; }
    inline BOOL Is() const                  { return p != NULL; }
    inline BOOL operator ! () const         { return p == NULL; }
    inline ScToken* operator&() const       { return p; }
    inline ScToken* operator->() const      { return p; }
    inline ScToken& operator*() const       { return *p; }
    inline operator ScToken*() const        { return p; }
};


enum ScRecalcMode40
{                           // alter RecalcMode bis einschliesslich 4.0
    RC_NORMAL,
    RC_ALWAYS,
    RC_ONLOAD,              // immer nach dem Laden
    RC_ONLOAD_ONCE,         // einmal nach dem Laden
    RC_FORCED,              // auch wenn Zelle nicht sichtbar
    RC_ONREFMOVE            // wenn Referenz bewegt wurde (ab SP3, 05.11.97)
};

// neuer (ab 5.0, 14.01.98) RecalcMode Zugriff nur noch noch via TokenArray
// SetRecalcMode / IsRecalcMode...

typedef BYTE ScRecalcMode;
// von den exklusiven Bits kann nur eins gesetzt sein
// wird von TokenArray SetRecalcMode... Methoden gehandelt
#define RECALCMODE_NORMAL       0x01    // exklusiv
#define RECALCMODE_ALWAYS       0x02    // exklusiv, immer
#define RECALCMODE_ONLOAD       0x04    // exklusiv, immer nach dem Laden
#define RECALCMODE_ONLOAD_ONCE  0x08    // exklusiv, einmal nach dem Laden
#define RECALCMODE_FORCED       0x10    // kombiniert, auch wenn Zelle nicht sichtbar
#define RECALCMODE_ONREFMOVE    0x20    // kombiniert, wenn Referenz bewegt wurde
#define RECALCMODE_EMASK        0x0F    // Maske der exklusiven Bits
// werden neue Bits definiert, muessen ExportRecalcMode40 und AddRecalcMode
// angepasst werden!

class ScTokenArray
{
    friend class ScCompiler;
    friend class ScTokenIterator;

    ScToken** pCode;        // Token->Array
    ScToken** pRPN;         // RPN-Array
    USHORT nLen;            // Laenge des TokenArrays
    USHORT nRPN;            // Laenge des RPN-Arrays
    USHORT nIndex;          // aktueller Step-Index
    USHORT nError;          // Fehlercode
    short  nRefs;           // Anzahl Referenzen
    ScRecalcMode nMode;     // wann muss berechnet werden?

    void                    Assign( const ScTokenArray& );

    ScToken*                Add( ScToken* );
    void                    ImportRecalcMode40( ScRecalcMode40 );
    ScRecalcMode40          ExportRecalcMode40() const;
    inline  void            SetCombinedBitsRecalcMode( ScRecalcMode nBits )
                            { nMode |= (nBits & ~RECALCMODE_EMASK); }
    inline  ScRecalcMode    GetCombinedBitsRecalcMode() const
                            { return nMode & ~RECALCMODE_EMASK; }
    inline  void            SetMaskedRecalcMode( ScRecalcMode nBits )
                            { nMode = GetCombinedBitsRecalcMode() | nBits; }
                            // in nMode gesetzte exklusive Bits werden genullt,
                            // nVal kann auch kombinierte Bits enthalten,
                            // aber nur ein exklusives Bit darf gesetzt sein!

public:
    ScTokenArray();
    ScTokenArray( const ScTokenArray& );    // Zuweisung mit Referenzen auf Tokens
   ~ScTokenArray();
    ScTokenArray* Clone() const;    // echte Kopie!
    void Clear();
    void DelRPN();
    ScToken* First() { nIndex = 0; return Next(); }
    ScToken* Next();
    ScToken* GetNextName();
    ScToken* GetNextDBArea();
    ScToken* GetNextReference();
    ScToken* GetNextReferenceRPN();
    ScToken* GetNextReferenceOrName();
    ScToken* GetNextColRowName();
    ScToken* GetNextOpCodeRPN( OpCode );
    ScToken* PeekNext();
    ScToken* PeekPrevNoSpaces();    // nur nach First/Next/Last/Prev !
    ScToken* PeekNextNoSpaces();    // nur nach First/Next/Last/Prev !
    ScToken* FirstRPN() { nIndex = 0; return NextRPN(); }
    ScToken* NextRPN();
    ScToken* LastRPN() { nIndex = nRPN; return PrevRPN(); }
    ScToken* PrevRPN();

    BOOL    HasOpCodeRPN( OpCode ) const;
    BOOL    HasName() const;        // token of type svIndex
    BOOL    IsReference( ScRange& rRange ) const;       // exactly and only one range

    ScToken** GetArray() const  { return pCode; }
    ScToken** GetCode()  const  { return pRPN; }
    USHORT    GetLen() const     { return nLen; }
    USHORT    GetCodeLen() const { return nRPN; }
    void      Reset()            { nIndex = 0; }
    USHORT    GetError() const { return nError; }
    void      SetError( USHORT n ) { nError = n; }
    short     GetRefs()  const { return nRefs;  }

    inline  ScRecalcMode    GetRecalcMode() const { return nMode; }
            void            AddRecalcMode( ScRecalcMode nBits );
                                // Bits werden nicht direkt gesetzt sondern
                                // geprueft und evtl. nach Prio behandelt,
                                // wenn mehrere exklusive gesetzt sind!

    inline  void            ClearRecalcMode() { nMode = RECALCMODE_NORMAL; }
    inline  void            SetRecalcModeNormal()
                                { SetMaskedRecalcMode( RECALCMODE_NORMAL ); }
    inline  void            SetRecalcModeAlways()
                                { SetMaskedRecalcMode( RECALCMODE_ALWAYS ); }
    inline  void            SetRecalcModeOnLoad()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD ); }
    inline  void            SetRecalcModeOnLoadOnce()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD_ONCE ); }
    inline  void            SetRecalcModeForced()
                                { nMode |= RECALCMODE_FORCED; }
    inline  void            ClearRecalcModeForced()
                                { nMode &= ~RECALCMODE_FORCED; }
    inline  void            SetRecalcModeOnRefMove()
                                { nMode |= RECALCMODE_ONREFMOVE; }
    inline  void            ClearRecalcModeOnRefMove()
                                { nMode &= ~RECALCMODE_ONREFMOVE; }
    inline  BOOL            IsRecalcModeNormal() const
                                { return (nMode & RECALCMODE_NORMAL) != 0; }
    inline  BOOL            IsRecalcModeAlways() const
                                { return (nMode & RECALCMODE_ALWAYS) != 0; }
    inline  BOOL            IsRecalcModeOnLoad() const
                                { return (nMode & RECALCMODE_ONLOAD) != 0; }
    inline  BOOL            IsRecalcModeOnLoadOnce() const
                                { return (nMode & RECALCMODE_ONLOAD_ONCE) != 0; }
    inline  BOOL            IsRecalcModeForced() const
                                { return (nMode & RECALCMODE_FORCED) != 0; }
    inline  BOOL            IsRecalcModeOnRefMove() const
                                { return (nMode & RECALCMODE_ONREFMOVE) != 0; }

    inline OpCode   GetOuterFuncOpCode();       // OpCode der aeusseren Funktion
                // ermittelt Ausdehnung direkt angrenzender Refs
                // nur auf echte Funcs wie z.B. GetOuterFuncOpCode() == ocSum anwenden!
    BOOL        GetAdjacentExtendOfOuterFuncRefs( USHORT& nExtend,
                    const ScAddress& rPos, ScDirection );

                            // Operatoren +,-,*,/,^,&,=,<>,<,>,<=,>=
                            // mit DoubleRef in Formel?
    BOOL                    HasMatrixDoubleRefOps();

    void Load30( SvStream&, const ScAddress& );
    void Load( SvStream&, USHORT, const ScAddress& );
    void Store( SvStream&, const ScAddress& ) const;

    ScToken* AddToken( const ScToken& );
    ScToken* AddOpCode( OpCode eCode );
    ScToken* AddString( const sal_Unicode* pStr );
    ScToken* AddDouble( double fVal );
    ScToken* AddSingleReference( const SingleRefData& rRef );
    ScToken* AddDoubleReference( const ComplRefData& rRef );
    ScToken* AddName( USHORT n );
    ScToken* AddMatrix( ScMatrix* p );
    ScToken* AddExternal( const sal_Unicode* pStr );
    ScToken* AddColRowName( const SingleRefData& rRef );
    ScToken* AddBad( const sal_Unicode* pStr );     // ocBad mit String

    // Zuweisung mit Referenzen auf Tokens
    ScTokenArray& operator=( const ScTokenArray& );
};

inline OpCode ScTokenArray::GetOuterFuncOpCode()
{
    if ( pRPN && nRPN )
        return pRPN[nRPN-1]->GetOpCode();
    return (OpCode) SC_OPCODE_NONE;
}

struct ImpTokenIterator;

class ScTokenIterator
{
    friend class ScInterpreter;     // fuer Jump()
    ImpTokenIterator* pCur;
    void Jump( short nStart, short nNext );
    void Push( ScTokenArray* );
    void Pop();
public:
    ScTokenIterator( const ScTokenArray& );
   ~ScTokenIterator();
    void  Reset();
    const ScToken* First();
    const ScToken* Next();
};

struct ScArrayStack;

class ScCompiler
{
public:
    static String*  pSymbolTableNative;             // Liste der Symbole: String[]
    static String*  pSymbolTableEnglish;            // Liste der Symbole English
    static USHORT   nAnzStrings;                    // Anzahl der Symbole
private:
    static USHORT*  pCharTable;                     // char[];
    ScDocument* pDoc;
    ScAddress   aPos;
    String      cFormula;                           // String -> TokenArray
    String      aCorrectedFormula;                  // autocorrected Formula
    String      aCorrectedSymbol;                   // autocorrected Symbol
    sal_Unicode cSymbol[MAXSTRLEN];                 // current Symbol
    String      aFormula;                           // formula source code
    xub_StrLen  nSrcPos;                            // tokenizer position (source code)
    ScTokenArray* pArr;
    ScTokenRef  pToken;
    OpCode      eLastOp;
    ScToken**   pCode;
    ScArrayStack* pStack;
    String*     pSymbolTable;               // welche SymbolTable benutzt wird
    USHORT      pc;
    short       nNumFmt;                    // bei CompileTokenArray() gesetzt
    short       nMaxTab;                    // letzte Tabelle im Doc
    short       nRecursion;                 // GetToken Rekursionen
    BOOL        glSubTotal;                 // Merkvariablen fuer Zelle
    BOOL        bAutoCorrect;               // AutoCorrection ausfuehren
    BOOL        bCorrected;                 // AutoCorrection ausgefuehrt
    BOOL        bCompileForFAP;             //! kein echtes RPN sondern Namen
                                            // werden nicht aufgeloest
    BOOL        bIgnoreErrors;              // bei AutoCorrect und CompileForFAP
                                            // trotz Fehler RPN erzeugen
    BOOL        bCompileXML;
    BOOL        bImportXML;

    BOOL   GetToken();
    BOOL   NextNewToken();
    OpCode NextToken();
    ScToken* PersistToken( ScToken* );
    void PutCode( ScToken* );
    void Factor();
    void UnionCutLine();
    void Unary();
    void PowLine();
    void MulDivLine();
    void AddSubLine();
    void CompareLine();
    void NotLine();
    OpCode Expression();

    String MakeColStr( USHORT nCol );
    String MakeRowStr( USHORT nRow );
    String MakeTabStr( USHORT nTab, String& aDoc );
    String MakeRefStr( ComplRefData& rRefData, BOOL bSingleRef );

    void SetError(USHORT nError);
    xub_StrLen NextSymbol();
    BOOL IsValue( const String& );
    BOOL IsOpCode( const String& );
    BOOL IsOpCode2( const String& );
    BOOL IsString();
    BOOL IsReference( const String& );
    BOOL IsMacro( const String& );
    BOOL IsNamedRange( const String& );
    BOOL IsDBRange( const String& );
    BOOL IsColRowName( const String& );
    void AutoCorrectParsedSymbol();
    void AdjustReference( SingleRefData& r );
    void PushTokenArray( ScTokenArray*, BOOL = FALSE );
    void PopTokenArray();
    void SetRelNameReference();
public:
    ScCompiler(ScDocument* pDocument, const ScAddress& );

    ScCompiler( ScDocument* pDocument, const ScAddress&,
                const ScTokenArray& rArr );

    static void Init();
    static void DeInit();
    static void CheckTabQuotes( String& );  // fuer ScAddress::Format()

    // schliesst String in Quotes ein, wenn nicht-alphanumerische Zeichen
    // enthalten sind, bestehende Quotes werden escaped
    static BOOL EnQuote( String& );
    // entfernt Quotes, escaped Quotes werden unescaped
    static BOOL DeQuote( String& );

    //! _entweder_ CompileForFAP _oder_ AutoCorrection, _nicht_ beides
    void            SetCompileForFAP( BOOL bVal )
                        { bCompileForFAP = bVal; bIgnoreErrors = bVal; }
    void            SetAutoCorrection( BOOL bVal )
                        { bAutoCorrect = bVal; bIgnoreErrors = bVal; }
    void            SetCompileEnglish( BOOL bVal );     // use English SymbolTable
    void            SetCompileXML( BOOL bVal )
                        { bCompileXML = bVal; }
    void            SetImportXML( BOOL bVal )
                        { bImportXML = bVal; }
    BOOL            IsCorrected() { return bCorrected; }
    const String&   GetCorrectedFormula() { return aCorrectedFormula; }
    ScTokenArray* CompileString( const String& rFormula );
    BOOL  CompileTokenArray();
    short GetNumFormatType() { return nNumFmt; }

    ScToken* CreateStringFromToken( String& rFormula, ScToken* pToken,
                                    BOOL bAllowArrAdvance = FALSE );
    void CreateStringFromTokenArray( String& rFormula );

    void MoveRelWrap();
    static void MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc,
                             const ScAddress& rPos );

    BOOL UpdateNameReference( UpdateRefMode eUpdateRefMode,
                              const ScRange&,
                              short nDx, short nDy, short nDz,
                              BOOL& rChanged);

    ScRangeData* UpdateReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  short nDx, short nDy, short nDz,
                                  BOOL& rChanged);

    /// Only once for converted shared formulas,
    /// token array has to be compiled afterwards.
    void UpdateSharedFormulaReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  short nDx, short nDy, short nDz );

    ScRangeData* UpdateInsertTab(USHORT nTable, BOOL bIsName );
    ScRangeData* UpdateDeleteTab(USHORT nTable, BOOL bIsMove, BOOL bIsName, BOOL& bCompile);
    ScRangeData* UpdateMoveTab(USHORT nOldPos, USHORT nNewPos, BOOL bIsName );

    BOOL HasModifiedRange();

    /// Is the CharTable initialized? If not call Init() yourself!
    static inline BOOL HasCharTable() { return pCharTable != NULL; }

    /// Access the CharTable flags
    static inline USHORT GetCharTableFlags( sal_Unicode c )
        { return c < 128 ? pCharTable[ UINT8(c) ] : 0; }

    /// If the character is allowed as first character in sheet names or references
    static inline BOOL IsCharWordChar( sal_Unicode c )
        {
            return c < 128 ?
                ((pCharTable[ UINT8(c) ] & SC_COMPILER_C_CHAR_WORD) == SC_COMPILER_C_CHAR_WORD) :
                ScGlobal::pCharClass->isLetterNumeric( c );
        }

    /// If the character is allowed in sheet names or references
    static inline BOOL IsWordChar( sal_Unicode c )
        {
            return c < 128 ?
                ((pCharTable[ UINT8(c) ] & SC_COMPILER_C_WORD) == SC_COMPILER_C_WORD) :
                ScGlobal::pCharClass->isLetterNumeric( c );
        }
};

#endif
