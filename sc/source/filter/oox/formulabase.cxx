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

#include "formulabase.hxx"

#include <map>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/AddressConvention.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <com/sun/star/sheet/FormulaMapGroup.hpp>
#include <com/sun/star/sheet/FormulaMapGroupSpecialOffset.hpp>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include "oox/core/filterbase.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/token/properties.hxx"
#include "biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;


// reference helpers ==========================================================

BinSingleRef2d::BinSingleRef2d() :
    mnCol( 0 ),
    mnRow( 0 ),
    mbColRel( false ),
    mbRowRel( false )
{
}

void BinSingleRef2d::setBiff12Data( sal_uInt16 nCol, sal_Int32 nRow, bool bRelativeAsOffset )
{
    mnCol = nCol & BIFF12_TOK_REF_COLMASK;
    mnRow = nRow & BIFF12_TOK_REF_ROWMASK;
    mbColRel = getFlag( nCol, BIFF12_TOK_REF_COLREL );
    mbRowRel = getFlag( nCol, BIFF12_TOK_REF_ROWREL );
    if( bRelativeAsOffset && mbColRel && (mnCol > (BIFF12_TOK_REF_COLMASK >> 1)) )
        mnCol -= (BIFF12_TOK_REF_COLMASK + 1);
    if( bRelativeAsOffset && mbRowRel && (mnRow > (BIFF12_TOK_REF_ROWMASK >> 1)) )
        mnRow -= (BIFF12_TOK_REF_ROWMASK + 1);
}

void BinSingleRef2d::setBiff2Data( sal_uInt8 nCol, sal_uInt16 nRow, bool bRelativeAsOffset )
{
    mnCol = nCol;
    mnRow = nRow & BIFF_TOK_REF_ROWMASK;
    mbColRel = getFlag( nRow, BIFF_TOK_REF_COLREL );
    mbRowRel = getFlag( nRow, BIFF_TOK_REF_ROWREL );
    if( bRelativeAsOffset && mbColRel && (mnCol >= 0x80) )
        mnCol -= 0x100;
    if( bRelativeAsOffset && mbRowRel && (mnRow > (BIFF_TOK_REF_ROWMASK >> 1)) )
        mnRow -= (BIFF_TOK_REF_ROWMASK + 1);
}

void BinSingleRef2d::setBiff8Data( sal_uInt16 nCol, sal_uInt16 nRow, bool bRelativeAsOffset )
{
    mnCol = nCol & BIFF_TOK_REF_COLMASK;
    mnRow = nRow;
    mbColRel = getFlag( nCol, BIFF_TOK_REF_COLREL );
    mbRowRel = getFlag( nCol, BIFF_TOK_REF_ROWREL );
    if( bRelativeAsOffset && mbColRel && (mnCol > (BIFF_TOK_REF_COLMASK >> 1)) )
        mnCol -= (BIFF_TOK_REF_COLMASK + 1);
    if( bRelativeAsOffset && mbRowRel && (mnRow >= 0x8000) )
        mnRow -= 0x10000;
}

void BinSingleRef2d::readBiff12Data( SequenceInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_Int32 nRow;
    sal_uInt16 nCol;
    rStrm >> nRow >> nCol;
    setBiff12Data( nCol, nRow, bRelativeAsOffset );
}

void BinSingleRef2d::readBiff2Data( BiffInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_uInt16 nRow;
    sal_uInt8 nCol;
    rStrm >> nRow >> nCol;
    setBiff2Data( nCol, nRow, bRelativeAsOffset );
}

void BinSingleRef2d::readBiff8Data( BiffInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_uInt16 nRow, nCol;
    rStrm >> nRow >> nCol;
    setBiff8Data( nCol, nRow, bRelativeAsOffset );
}

// ----------------------------------------------------------------------------

void BinComplexRef2d::readBiff12Data( SequenceInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_Int32 nRow1, nRow2;
    sal_uInt16 nCol1, nCol2;
    rStrm >> nRow1 >> nRow2 >> nCol1 >> nCol2;
    maRef1.setBiff12Data( nCol1, nRow1, bRelativeAsOffset );
    maRef2.setBiff12Data( nCol2, nRow2, bRelativeAsOffset );
}

void BinComplexRef2d::readBiff2Data( BiffInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_uInt16 nRow1, nRow2;
    sal_uInt8 nCol1, nCol2;
    rStrm >> nRow1 >> nRow2 >> nCol1 >> nCol2;
    maRef1.setBiff2Data( nCol1, nRow1, bRelativeAsOffset );
    maRef2.setBiff2Data( nCol2, nRow2, bRelativeAsOffset );
}

void BinComplexRef2d::readBiff8Data( BiffInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_uInt16 nRow1, nRow2, nCol1, nCol2;
    rStrm >> nRow1 >> nRow2 >> nCol1 >> nCol2;
    maRef1.setBiff8Data( nCol1, nRow1, bRelativeAsOffset );
    maRef2.setBiff8Data( nCol2, nRow2, bRelativeAsOffset );
}

// token vector, sequence =====================================================

ApiTokenVector::ApiTokenVector()
{
}

Any& ApiTokenVector::append( sal_Int32 nOpCode )
{
    resize( size() + 1 );
    back().OpCode = nOpCode;
    return back().Data;
}

// token sequence iterator ====================================================

ApiTokenIterator::ApiTokenIterator( const ApiTokenSequence& rTokens, sal_Int32 nSpacesOpCode, bool bSkipSpaces ) :
    mpToken( rTokens.getConstArray() ),
    mpTokenEnd( rTokens.getConstArray() + rTokens.getLength() ),
    mnSpacesOpCode( nSpacesOpCode ),
    mbSkipSpaces( bSkipSpaces )
{
    skipSpaces();
}

ApiTokenIterator& ApiTokenIterator::operator++()
{
    if( is() )
    {
        ++mpToken;
        skipSpaces();
    }
    return *this;
}

void ApiTokenIterator::skipSpaces()
{
    if( mbSkipSpaces )
        while( is() && (mpToken->OpCode == mnSpacesOpCode) )
            ++mpToken;
}

// function data ==============================================================

namespace {

const size_t FUNCINFO_PARAMINFOCOUNT        = 5;        /// Number of parameter type entries.

const sal_uInt16 FUNCFLAG_VOLATILE          = 0x0001;   /// Result is volatile (e.g. NOW() function).
const sal_uInt16 FUNCFLAG_IMPORTONLY        = 0x0002;   /// Only used in import filter.
const sal_uInt16 FUNCFLAG_EXPORTONLY        = 0x0004;   /// Only used in export filter.
const sal_uInt16 FUNCFLAG_MACROCALL         = 0x0008;   /// Function is stored as macro call in Excel (_xlfn. prefix). OOXML name MUST exist.
const sal_uInt16 FUNCFLAG_MACROCALLODF      = 0x0010;   /// ODF-only function stored as macro call in Excel (_xlfnodf. prefix). ODF name MUST exist.
const sal_uInt16 FUNCFLAG_EXTERNAL          = 0x0020;   /// Function is external in Calc.
const sal_uInt16 FUNCFLAG_MACROFUNC         = 0x0040;   /// Function is a macro-sheet function.
const sal_uInt16 FUNCFLAG_MACROCMD          = 0x0080;   /// Function is a macro-sheet command.
const sal_uInt16 FUNCFLAG_ALWAYSVAR         = 0x0100;   /// Function is always represented by a tFuncVar token.
const sal_uInt16 FUNCFLAG_PARAMPAIRS        = 0x0200;   /// Optional parameters are expected to appear in pairs.
const sal_uInt16 FUNCFLAG_MACROCALL_FN      = 0x0400;   /** Function is stored as macro call in Excel (_xlfn. prefix)
                                                            for OOXML. OOXML name MUST exist. Do not use without
                                                            FUNCFLAG_MACROCALL. */
const sal_uInt16 FUNCFLAG_MACROCALL_NEW     = FUNCFLAG_MACROCALL | FUNCFLAG_MACROCALL_FN;   /** New Excel functions not
                                                            defined in OOXML, _xlfn. prefix in all formats. OOXML name
                                                            must exist. */
const sal_uInt16 FUNCFLAG_BIFFIMPORTONLY    = 0x0800;   /// Only used in BIFF binary import filter.
const sal_uInt16 FUNCFLAG_BIFFEXPORTONLY    = 0x1000;   /// Only used in BIFF binary export filter.

/// Converts a function library index (value of enum FunctionLibraryType) to function flags.
#define FUNCLIB_TO_FUNCFLAGS( funclib_index ) static_cast< sal_uInt16 >( static_cast< sal_uInt8 >( funclib_index ) << 12 )
/// Extracts a function library index (value of enum FunctionLibraryType) from function flags.
#define FUNCFLAGS_TO_FUNCLIB( func_flags ) extractValue< FunctionLibraryType >( func_flags, 12, 4 )

typedef ::boost::shared_ptr< FunctionInfo > FunctionInfoRef;

struct FunctionData
{
    const sal_Char*     mpcOdfFuncName;     /// ODF function name.
    const sal_Char*     mpcOoxFuncName;     /// OOXML function name.
    sal_uInt16          mnBiff12FuncId;     /// BIFF12 function identifier.
    sal_uInt16          mnBiffFuncId;       /// BIFF2-BIFF8 function identifier.
    sal_uInt8           mnMinParamCount;    /// Minimum number of parameters.
    sal_uInt8           mnMaxParamCount;    /// Maximum number of parameters.
    sal_uInt8           mnRetClass;         /// BIFF token class of the return value.
    FunctionParamInfo   mpParamInfos[ FUNCINFO_PARAMINFOCOUNT ]; /// Information about all parameters.
    sal_uInt16          mnFlags;            /// Additional flags.

    inline bool         isSupported( bool bImportFilter, FilterType eFilter ) const;
};

inline bool FunctionData::isSupported( bool bImportFilter, FilterType eFilter ) const
{
    /*  For import filters: the FUNCFLAG_EXPORTONLY and FUNCFLAG_BIFFEXPORTONLY flag must not be set.
        For OOXML import:   the FUNCFLAG_BIFFIMPORTONLY flag must not be set.
        For export filters: the FUNCFLAG_IMPORTONLY and FUNCFLAG_BIFFIMPORTONLY flag must not be set.
        For OOXML export:   the FUNCFLAG_BIFFEXPORTONLY flag must not be set. */
    bool bSupported = !getFlag( mnFlags, static_cast<sal_uInt16>(bImportFilter ?
                (FUNCFLAG_EXPORTONLY | FUNCFLAG_BIFFEXPORTONLY) :
                (FUNCFLAG_IMPORTONLY | FUNCFLAG_BIFFIMPORTONLY)));
    if (bSupported && eFilter == FILTER_OOXML)
        bSupported = !getFlag( mnFlags, bImportFilter ? FUNCFLAG_BIFFIMPORTONLY : FUNCFLAG_BIFFEXPORTONLY );
    return bSupported;
}

const sal_uInt16 NOID = SAL_MAX_UINT16;     /// No BIFF function identifier available.
const sal_uInt8 MX    = SAL_MAX_UINT8;      /// Maximum parameter count.

// abbreviations for function return token class
const sal_uInt8 R = BIFF_TOKCLASS_REF;
const sal_uInt8 V = BIFF_TOKCLASS_VAL;
const sal_uInt8 A = BIFF_TOKCLASS_ARR;

// abbreviations for parameter infos
#define RO   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_ORG, false }
#define RA   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_ARR, false }
#define RR   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_RPT, false }
#define RX   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_RPX, false }
#define VO   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_ORG, true  }
#define VV   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_VAL, true  }
#define VA   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_ARR, true  }
#define VR   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_RPT, true  }
#define VX   { FUNC_PARAM_REGULAR,   FUNC_PARAMCONV_RPX, true  }
#define RO_E { FUNC_PARAM_EXCELONLY, FUNC_PARAMCONV_ORG, false }
#define VR_E { FUNC_PARAM_EXCELONLY, FUNC_PARAMCONV_RPT, true  }
#define C    { FUNC_PARAM_CALCONLY,  FUNC_PARAMCONV_ORG, false }

// Note: parameter types of all macro sheet functions (FUNCFLAG_MACROFUNC/FUNCFLAG_MACROCMD) untested!

/** Functions new in BIFF2. */
static const FunctionData saFuncTableBiff2[] =
{
    { "COUNT",                  "COUNT",                0,      0,      0,  MX, V, { RX }, 0 },
    { "IF",                     "IF",                   1,      1,      2,  3,  R, { VO, RO }, 0 },
    { "ISNA",                   "ISNA",                 2,      2,      1,  1,  V, { VR }, 0 },
    { "ISERROR",                "ISERROR",              3,      3,      1,  1,  V, { VR }, 0 },
    { "SUM",                    "SUM",                  4,      4,      0,  MX, V, { RX }, 0 },
    { "AVERAGE",                "AVERAGE",              5,      5,      1,  MX, V, { RX }, 0 },
    { "MIN",                    "MIN",                  6,      6,      1,  MX, V, { RX }, 0 },
    { "MAX",                    "MAX",                  7,      7,      1,  MX, V, { RX }, 0 },
    { "ROW",                    "ROW",                  8,      8,      0,  1,  V, { RO }, 0 },
    { "COLUMN",                 "COLUMN",               9,      9,      0,  1,  V, { RO }, 0 },
    { "NA",                     "NA",                   10,     10,     0,  0,  V, {}, 0 },
    { "NPV",                    "NPV",                  11,     11,     2,  MX, V, { VR, RX }, 0 },
    { "STDEV",                  "STDEV",                12,     12,     1,  MX, V, { RX }, 0 },
    { "DOLLAR",                 "DOLLAR",               13,     13,     1,  2,  V, { VR }, 0 },
    { "FIXED",                  "FIXED",                14,     14,     1,  2,  V, { VR, VR, C }, 0 },
    { "SIN",                    "SIN",                  15,     15,     1,  1,  V, { VR }, 0 },
    { "CSC",                    "SIN",                  15,     15,     1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "COS",                    "COS",                  16,     16,     1,  1,  V, { VR }, 0 },
    { "SEC",                    "COS",                  16,     16,     1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "TAN",                    "TAN",                  17,     17,     1,  1,  V, { VR }, 0 },
    { "COT",                    "TAN",                  17,     17,     1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "ATAN",                   "ATAN",                 18,     18,     1,  1,  V, { VR }, 0 },
    { "ACOT",                   "ATAN",                 18,     18,     1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "PI",                     "PI",                   19,     19,     0,  0,  V, {}, 0 },
    { "SQRT",                   "SQRT",                 20,     20,     1,  1,  V, { VR }, 0 },
    { "EXP",                    "EXP",                  21,     21,     1,  1,  V, { VR }, 0 },
    { "LN",                     "LN",                   22,     22,     1,  1,  V, { VR }, 0 },
    { "LOG10",                  "LOG10",                23,     23,     1,  1,  V, { VR }, 0 },
    { "ABS",                    "ABS",                  24,     24,     1,  1,  V, { VR }, 0 },
    { "INT",                    "INT",                  25,     25,     1,  1,  V, { VR }, 0 },
    { "SIGN",                   "SIGN",                 26,     26,     1,  1,  V, { VR }, 0 },
    { "ROUND",                  "ROUND",                27,     27,     2,  2,  V, { VR }, 0 },
    { "LOOKUP",                 "LOOKUP",               28,     28,     2,  3,  V, { VR, RA }, 0 },
    { "INDEX",                  "INDEX",                29,     29,     2,  4,  R, { RA, VV }, 0 },
    { "REPT",                   "REPT",                 30,     30,     2,  2,  V, { VR }, 0 },
    { "MID",                    "MID",                  31,     31,     3,  3,  V, { VR }, 0 },
    { "LEN",                    "LEN",                  32,     32,     1,  1,  V, { VR }, 0 },
    { "VALUE",                  "VALUE",                33,     33,     1,  1,  V, { VR }, 0 },
    { "TRUE",                   "TRUE",                 34,     34,     0,  0,  V, {}, 0 },
    { "FALSE",                  "FALSE",                35,     35,     0,  0,  V, {}, 0 },
    { "AND",                    "AND",                  36,     36,     1,  MX, V, { RX }, 0 },
    { "OR",                     "OR",                   37,     37,     1,  MX, V, { RX }, 0 },
    { "NOT",                    "NOT",                  38,     38,     1,  1,  V, { VR }, 0 },
    { "MOD",                    "MOD",                  39,     39,     2,  2,  V, { VR }, 0 },
    { "DCOUNT",                 "DCOUNT",               40,     40,     3,  3,  V, { RO, RR }, 0 },
    { "DSUM",                   "DSUM",                 41,     41,     3,  3,  V, { RO, RR }, 0 },
    { "DAVERAGE",               "DAVERAGE",             42,     42,     3,  3,  V, { RO, RR }, 0 },
    { "DMIN",                   "DMIN",                 43,     43,     3,  3,  V, { RO, RR }, 0 },
    { "DMAX",                   "DMAX",                 44,     44,     3,  3,  V, { RO, RR }, 0 },
    { "DSTDEV",                 "DSTDEV",               45,     45,     3,  3,  V, { RO, RR }, 0 },
    { "VAR",                    "VAR",                  46,     46,     1,  MX, V, { RX }, 0 },
    { "DVAR",                   "DVAR",                 47,     47,     3,  3,  V, { RO, RR }, 0 },
    { "TEXT",                   "TEXT",                 48,     48,     2,  2,  V, { VR }, 0 },
    { "LINEST",                 "LINEST",               49,     49,     1,  2,  A, { RA, RA, C, C }, 0 },
    { "TREND",                  "TREND",                50,     50,     1,  3,  A, { RA, RA, RA, C }, 0 },
    { "LOGEST",                 "LOGEST",               51,     51,     1,  2,  A, { RA, RA, C, C }, 0 },
    { "GROWTH",                 "GROWTH",               52,     52,     1,  3,  A, { RA, RA, RA, C }, 0 },
    { "PV",                     "PV",                   56,     56,     3,  5,  V, { VR }, 0 },
    { "FV",                     "FV",                   57,     57,     3,  5,  V, { VR }, 0 },
    { "NPER",                   "NPER",                 58,     58,     3,  5,  V, { VR }, 0 },
    { "PMT",                    "PMT",                  59,     59,     3,  5,  V, { VR }, 0 },
    { "RATE",                   "RATE",                 60,     60,     3,  6,  V, { VR }, 0 },
    { "MIRR",                   "MIRR",                 61,     61,     3,  3,  V, { RA, VR }, 0 },
    { "IRR",                    "IRR",                  62,     62,     1,  2,  V, { RA, VR }, 0 },
    { "RAND",                   "RAND",                 63,     63,     0,  0,  V, {}, FUNCFLAG_VOLATILE },
    { "MATCH",                  "MATCH",                64,     64,     2,  3,  V, { VR, RX, RR }, 0 },
    { "DATE",                   "DATE",                 65,     65,     3,  3,  V, { VR }, 0 },
    { "TIME",                   "TIME",                 66,     66,     3,  3,  V, { VR }, 0 },
    { "DAY",                    "DAY",                  67,     67,     1,  1,  V, { VR }, 0 },
    { "MONTH",                  "MONTH",                68,     68,     1,  1,  V, { VR }, 0 },
    { "YEAR",                   "YEAR",                 69,     69,     1,  1,  V, { VR }, 0 },
    { "WEEKDAY",                "WEEKDAY",              70,     70,     1,  1,  V, { VR, C }, 0 },
    { "HOUR",                   "HOUR",                 71,     71,     1,  1,  V, { VR }, 0 },
    { "MINUTE",                 "MINUTE",               72,     72,     1,  1,  V, { VR }, 0 },
    { "SECOND",                 "SECOND",               73,     73,     1,  1,  V, { VR }, 0 },
    { "NOW",                    "NOW",                  74,     74,     0,  0,  V, {}, FUNCFLAG_VOLATILE },
    { "AREAS",                  "AREAS",                75,     75,     1,  1,  V, { RO }, 0 },
    { "ROWS",                   "ROWS",                 76,     76,     1,  1,  V, { RO }, 0 },
    { "COLUMNS",                "COLUMNS",              77,     77,     1,  1,  V, { RO }, 0 },
    { "OFFSET",                 "OFFSET",               78,     78,     3,  5,  R, { RO, VR }, FUNCFLAG_VOLATILE },
    { "SEARCH",                 "SEARCH",               82,     82,     2,  3,  V, { VR }, 0 },
    { "TRANSPOSE",              "TRANSPOSE",            83,     83,     1,  1,  A, { VO }, 0 },
    { "TYPE",                   "TYPE",                 86,     86,     1,  1,  V, { VX }, 0 },
    { "ATAN2",                  "ATAN2",                97,     97,     2,  2,  V, { VR }, 0 },
    { "ASIN",                   "ASIN",                 98,     98,     1,  1,  V, { VR }, 0 },
    { "ACOS",                   "ACOS",                 99,     99,     1,  1,  V, { VR }, 0 },
    { "CHOOSE",                 "CHOOSE",               100,    100,    2,  MX, R, { VO, RO }, 0 },
    { "HLOOKUP",                "HLOOKUP",              101,    101,    3,  3,  V, { VV, RO, RO, C }, 0 },
    { "VLOOKUP",                "VLOOKUP",              102,    102,    3,  3,  V, { VV, RO, RO, C }, 0 },
    { "ISREF",                  "ISREF",                105,    105,    1,  1,  V, { RX }, 0 },
    { "LOG",                    "LOG",                  109,    109,    1,  2,  V, { VR }, 0 },
    { "CHAR",                   "CHAR",                 111,    111,    1,  1,  V, { VR }, 0 },
    { "LOWER",                  "LOWER",                112,    112,    1,  1,  V, { VR }, 0 },
    { "UPPER",                  "UPPER",                113,    113,    1,  1,  V, { VR }, 0 },
    { "PROPER",                 "PROPER",               114,    114,    1,  1,  V, { VR }, 0 },
    { "LEFT",                   "LEFT",                 115,    115,    1,  2,  V, { VR }, 0 },
    { "RIGHT",                  "RIGHT",                116,    116,    1,  2,  V, { VR }, 0 },
    { "EXACT",                  "EXACT",                117,    117,    2,  2,  V, { VR }, 0 },
    { "TRIM",                   "TRIM",                 118,    118,    1,  1,  V, { VR }, 0 },
    { "REPLACE",                "REPLACE",              119,    119,    4,  4,  V, { VR }, 0 },
    { "SUBSTITUTE",             "SUBSTITUTE",           120,    120,    3,  4,  V, { VR }, 0 },
    { "CODE",                   "CODE",                 121,    121,    1,  1,  V, { VR }, 0 },
    { "FIND",                   "FIND",                 124,    124,    2,  3,  V, { VR }, 0 },
    { "CELL",                   "CELL",                 125,    125,    1,  2,  V, { VV, RO }, FUNCFLAG_VOLATILE },
    { "ISERR",                  "ISERR",                126,    126,    1,  1,  V, { VR }, 0 },
    { "ISTEXT",                 "ISTEXT",               127,    127,    1,  1,  V, { VR }, 0 },
    { "ISNUMBER",               "ISNUMBER",             128,    128,    1,  1,  V, { VR }, 0 },
    { "ISBLANK",                "ISBLANK",              129,    129,    1,  1,  V, { VR }, 0 },
    { "T",                      "T",                    130,    130,    1,  1,  V, { RO }, 0 },
    { "N",                      "N",                    131,    131,    1,  1,  V, { RO }, 0 },
    { "DATEVALUE",              "DATEVALUE",            140,    140,    1,  1,  V, { VR }, 0 },
    { "TIMEVALUE",              "TIMEVALUE",            141,    141,    1,  1,  V, { VR }, 0 },
    { "SLN",                    "SLN",                  142,    142,    3,  3,  V, { VR }, 0 },
    { "SYD",                    "SYD",                  143,    143,    4,  4,  V, { VR }, 0 },
    { "DDB",                    "DDB",                  144,    144,    4,  5,  V, { VR }, 0 },
    { "INDIRECT",               "INDIRECT",             148,    148,    1,  2,  R, { VR }, FUNCFLAG_VOLATILE },
    { "CLEAN",                  "CLEAN",                162,    162,    1,  1,  V, { VR }, 0 },
    { "MDETERM",                "MDETERM",              163,    163,    1,  1,  V, { VA }, 0 },
    { "MINVERSE",               "MINVERSE",             164,    164,    1,  1,  A, { VA }, 0 },
    { "MMULT",                  "MMULT",                165,    165,    2,  2,  A, { VA }, 0 },
    { "IPMT",                   "IPMT",                 167,    167,    4,  6,  V, { VR }, 0 },
    { "PPMT",                   "PPMT",                 168,    168,    4,  6,  V, { VR }, 0 },
    { "COUNTA",                 "COUNTA",               169,    169,    0,  MX, V, { RX }, 0 },
    { "PRODUCT",                "PRODUCT",              183,    183,    0,  MX, V, { RX }, 0 },
    { "FACT",                   "FACT",                 184,    184,    1,  1,  V, { VR }, 0 },
    { "DPRODUCT",               "DPRODUCT",             189,    189,    3,  3,  V, { RO, RR }, 0 },
    { "ISNONTEXT",              "ISNONTEXT",            190,    190,    1,  1,  V, { VR }, 0 },
    { "STDEVP",                 "STDEVP",               193,    193,    1,  MX, V, { RX }, 0 },
    { "VARP",                   "VARP",                 194,    194,    1,  MX, V, { RX }, 0 },
    { "DSTDEVP",                "DSTDEVP",              195,    195,    3,  3,  V, { RO, RR }, 0 },
    { "DVARP",                  "DVARP",                196,    196,    3,  3,  V, { RO, RR }, 0 },
    { "TRUNC",                  "TRUNC",                197,    197,    1,  1,  V, { VR, C }, 0 },
    { "ISLOGICAL",              "ISLOGICAL",            198,    198,    1,  1,  V, { VR }, 0 },
    { "DCOUNTA",                "DCOUNTA",              199,    199,    3,  3,  V, { RO, RR }, 0 },
    { 0,                        "EXTERN.CALL",          255,    255,    1,  MX, R, { RO_E, RO }, FUNCFLAG_IMPORTONLY },

    // *** macro sheet commands ***

    { 0,                        "A1.R1C1",              30,     30,     0,  1,  V, { VR }, FUNCFLAG_MACROCMD },
    { 0,                        "RETURN",               55,     55,     0,  1,  R, { RO }, FUNCFLAG_MACROFUNC },
    { 0,                        "ABSREF",               79,     79,     2,  2,  R, { VR, RO }, FUNCFLAG_MACROFUNC },
    { 0,                        "ADD.ARROW",            81,     81,     0,  0,  V, {}, FUNCFLAG_MACROCMD },
    { 0,                        "ACTIVE.CELL",          94,     94,     0,  0,  R, {}, FUNCFLAG_MACROFUNC },
    { 0,                        "ACTIVATE",             103,    103,    0,  2,  V, { VR }, FUNCFLAG_MACROCMD },
    { 0,                        "ACTIVATE.NEXT",        104,    104,    0,  0,  V, {}, FUNCFLAG_MACROCMD },
    { 0,                        "ACTIVATE.PREV",        105,    105,    0,  0,  V, {}, FUNCFLAG_MACROCMD },
    { 0,                        "ADD.BAR",              151,    151,    0,  0,  V, {}, FUNCFLAG_MACROFUNC | FUNCFLAG_ALWAYSVAR },
    { 0,                        "ADD.MENU",             152,    152,    2,  2,  V, { VR, RO }, FUNCFLAG_MACROFUNC | FUNCFLAG_ALWAYSVAR },
    { 0,                        "ADD.COMMAND",          153,    153,    3,  3,  V, { VR, RO }, FUNCFLAG_MACROFUNC | FUNCFLAG_ALWAYSVAR }
};

/** Functions new in BIFF3. */
static const FunctionData saFuncTableBiff3[] =
{
    { "LINEST",                 "LINEST",               49,     49,     1,  4,  A, { RA, RA, VV }, 0 },             // BIFF2: 1-2, BIFF3: 1-4
    { "TREND",                  "TREND",                50,     50,     1,  4,  A, { RA, RA, RA, VV }, 0 },             // BIFF2: 1-3, BIFF3: 1-4
    { "LOGEST",                 "LOGEST",               51,     51,     1,  4,  A, { RA, RA, VV }, 0 },             // BIFF2: 1-2, BIFF3: 1-4
    { "GROWTH",                 "GROWTH",               52,     52,     1,  4,  A, { RA, RA, RA, VV }, 0 },             // BIFF2: 1-3, BIFF3: 1-4
    { "TRUNC",                  "TRUNC",                197,    197,    1,  2,  V, { VR }, 0 },                      // BIFF2: 1,   BIFF3: 1-2
    { "DOLLAR",                 "USDOLLAR",             204,    204,    1,  2,  V, { VR }, FUNCFLAG_IMPORTONLY },
    { 0/*"FIND"*/,              "FINDB",                205,    205,    2,  3,  V, { VR }, 0 },
    { 0/*"SEARCH"*/,            "SEARCHB",              206,    206,    2,  3,  V, { VR }, 0 },
    { 0/*"REPLACE"*/,           "REPLACEB",             207,    207,    4,  4,  V, { VR }, 0 },
    { "LEFTB",                  "LEFTB",                208,    208,    1,  2,  V, { VR }, 0 },
    { "RIGHTB",                 "RIGHTB",               209,    209,    1,  2,  V, { VR }, 0 },
    { "MIDB",                   "MIDB",                 210,    210,    3,  3,  V, { VR }, 0 },
    { "LENB",                   "LENB",                 211,    211,    1,  1,  V, { VR }, 0 },
    { "ROUNDUP",                "ROUNDUP",              212,    212,    2,  2,  V, { VR }, 0 },
    { "ROUNDDOWN",              "ROUNDDOWN",            213,    213,    2,  2,  V, { VR }, 0 },
    { "ASC",                    "ASC",                  214,    214,    1,  1,  V, { VR }, 0 },
    { "JIS",                    "DBCS",                 215,    215,    1,  1,  V, { VR }, 0 },
    { "ADDRESS",                "ADDRESS",              219,    219,    2,  5,  V, { VR }, 0 },
    { "DAYS360",                "DAYS360",              220,    220,    2,  2,  V, { VR, VR, C }, 0 },
    { "TODAY",                  "TODAY",                221,    221,    0,  0,  V, {}, FUNCFLAG_VOLATILE },
    { "VDB",                    "VDB",                  222,    222,    5,  7,  V, { VR }, 0 },
    { "MEDIAN",                 "MEDIAN",               227,    227,    1,  MX, V, { RX }, 0 },
    { "SUMPRODUCT",             "SUMPRODUCT",           228,    228,    1,  MX, V, { VA }, 0 },
    { "SINH",                   "SINH",                 229,    229,    1,  1,  V, { VR }, 0 },
    { "CSCH",                   "SINH",                 229,    229,    1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "COSH",                   "COSH",                 230,    230,    1,  1,  V, { VR }, 0 },
    { "SECH",                   "COSH",                 230,    230,    1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "TANH",                   "TANH",                 231,    231,    1,  1,  V, { VR }, 0 },
    { "COTH",                   "TANH",                 231,    231,    1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "ASINH",                  "ASINH",                232,    232,    1,  1,  V, { VR }, 0 },
    { "ACOSH",                  "ACOSH",                233,    233,    1,  1,  V, { VR }, 0 },
    { "ATANH",                  "ATANH",                234,    234,    1,  1,  V, { VR }, 0 },
    { "ACOTH",                  "ATANH",                234,    234,    1,  1,  V, { VR }, FUNCFLAG_BIFFEXPORTONLY },
    { "DGET",                   "DGET",                 235,    235,    3,  3,  V, { RO, RR }, 0 },
    { "INFO",                   "INFO",                 244,    244,    1,  1,  V, { VR }, FUNCFLAG_VOLATILE },

    // *** macro sheet commands ***

    { 0,                        "ADD.BAR",              151,    151,    0,  1,  V, { VR }, FUNCFLAG_MACROFUNC },    // BIFF2: 0,   BIFF3: 0-1
    { 0,                        "ADD.MENU",             152,    152,    2,  3,  V, { VR, RO }, FUNCFLAG_MACROFUNC },  // BIFF2: 2,   BIFF3: 2-3
    { 0,                        "ADD.COMMAND",          153,    153,    3,  4,  V, { VR, RO }, FUNCFLAG_MACROFUNC }   // BIFF2: 3,   BIFF3: 3-4
};

/** Functions new in BIFF4. */
static const FunctionData saFuncTableBiff4[] =
{
    { "FIXED",                  "FIXED",                14,     14,     1,  3,  V, { VR }, 0 },       // BIFF2-3: 1-2, BIFF4: 1-3
    { "RANK",                   "RANK",                 216,    216,    2,  3,  V, { VR, RO, VR }, 0 },
    { "DB",                     "DB",                   247,    247,    4,  5,  V, { VR }, 0 },
    { "FREQUENCY",              "FREQUENCY",            252,    252,    2,  2,  A, { RA }, 0 },
    { "ORG.OPENOFFICE.ERRORTYPE","ERROR.TYPE",          261,    261,    1,  1,  V, { VR }, 0 },
    { "AVEDEV",                 "AVEDEV",               269,    269,    1,  MX, V, { RX }, 0 },
    { "BETADIST",               "BETADIST",             270,    270,    3,  5,  V, { VR }, 0 },
    { "GAMMALN",                "GAMMALN",              271,    271,    1,  1,  V, { VR }, 0 },
    { "BETAINV",                "BETAINV",              272,    272,    3,  5,  V, { VR }, 0 },
    { "BINOMDIST",              "BINOMDIST",            273,    273,    4,  4,  V, { VR }, 0 },
    { "LEGACY.CHIDIST",         "CHIDIST",              274,    274,    2,  2,  V, { VR }, 0 },
    { "LEGACY.CHIINV",          "CHIINV",               275,    275,    2,  2,  V, { VR }, 0 },
    { "COMBIN",                 "COMBIN",               276,    276,    2,  2,  V, { VR }, 0 },
    { "CONFIDENCE",             "CONFIDENCE",           277,    277,    3,  3,  V, { VR }, 0 },
    { "CRITBINOM",              "CRITBINOM",            278,    278,    3,  3,  V, { VR }, 0 },
    { "EVEN",                   "EVEN",                 279,    279,    1,  1,  V, { VR }, 0 },
    { "EXPONDIST",              "EXPONDIST",            280,    280,    3,  3,  V, { VR }, 0 },
    { "LEGACY.FDIST",           "FDIST",                281,    281,    3,  3,  V, { VR }, 0 },
    { "LEGACY.FINV",            "FINV",                 282,    282,    3,  3,  V, { VR }, 0 },
    { "FISHER",                 "FISHER",               283,    283,    1,  1,  V, { VR }, 0 },
    { "FISHERINV",              "FISHERINV",            284,    284,    1,  1,  V, { VR }, 0 },
    { "FLOOR",                  "FLOOR",                285,    285,    2,  2,  V, { VR, VR, C }, 0 },
    { "GAMMADIST",              "GAMMADIST",            286,    286,    4,  4,  V, { VR }, 0 },
    { "GAMMAINV",               "GAMMAINV",             287,    287,    3,  3,  V, { VR }, 0 },
    { "CEILING",                "CEILING",              288,    288,    2,  2,  V, { VR, VR, C }, 0 },
    { "HYPGEOMDIST",            "HYPGEOMDIST",          289,    289,    4,  4,  V, { VR }, 0 },
    { "LOGNORMDIST",            "LOGNORMDIST",          290,    290,    3,  3,  V, { VR }, 0 },
    { "LOGINV",                 "LOGINV",               291,    291,    3,  3,  V, { VR }, 0 },
    { "NEGBINOMDIST",           "NEGBINOMDIST",         292,    292,    3,  3,  V, { VR }, 0 },
    { "NORMDIST",               "NORMDIST",             293,    293,    4,  4,  V, { VR }, 0 },
    { "LEGACY.NORMSDIST",       "NORMSDIST",            294,    294,    1,  1,  V, { VR }, 0 },
    { "NORMINV",                "NORMINV",              295,    295,    3,  3,  V, { VR }, 0 },
    { "LEGACY.NORMSINV",        "NORMSINV",             296,    296,    1,  1,  V, { VR }, 0 },
    { "STANDARDIZE",            "STANDARDIZE",          297,    297,    3,  3,  V, { VR }, 0 },
    { "ODD",                    "ODD",                  298,    298,    1,  1,  V, { VR }, 0 },
    { "PERMUT",                 "PERMUT",               299,    299,    2,  2,  V, { VR }, 0 },
    { "POISSON",                "POISSON",              300,    300,    3,  3,  V, { VR }, 0 },
    { "LEGACY.TDIST",           "TDIST",                301,    301,    3,  3,  V, { VR }, 0 },
    { "WEIBULL",                "WEIBULL",              302,    302,    4,  4,  V, { VR }, 0 },
    { "SUMXMY2",                "SUMXMY2",              303,    303,    2,  2,  V, { VA }, 0 },
    { "SUMX2MY2",               "SUMX2MY2",             304,    304,    2,  2,  V, { VA }, 0 },
    { "SUMX2PY2",               "SUMX2PY2",             305,    305,    2,  2,  V, { VA }, 0 },
    { "LEGACY.CHITEST",         "CHITEST",              306,    306,    2,  2,  V, { VA }, 0 },
    { "CORREL",                 "CORREL",               307,    307,    2,  2,  V, { VA }, 0 },
    { "COVAR",                  "COVAR",                308,    308,    2,  2,  V, { VA }, 0 },
    { "FORECAST",               "FORECAST",             309,    309,    3,  3,  V, { VR, VA }, 0 },
    { "FTEST",                  "FTEST",                310,    310,    2,  2,  V, { VA }, 0 },
    { "INTERCEPT",              "INTERCEPT",            311,    311,    2,  2,  V, { VA }, 0 },
    { "PEARSON",                "PEARSON",              312,    312,    2,  2,  V, { VA }, 0 },
    { "RSQ",                    "RSQ",                  313,    313,    2,  2,  V, { VA }, 0 },
    { "STEYX",                  "STEYX",                314,    314,    2,  2,  V, { VA }, 0 },
    { "SLOPE",                  "SLOPE",                315,    315,    2,  2,  V, { VA }, 0 },
    { "TTEST",                  "TTEST",                316,    316,    4,  4,  V, { VA, VA, VR }, 0 },
    { "PROB",                   "PROB",                 317,    317,    3,  4,  V, { VA, VA, VR }, 0 },
    { "DEVSQ",                  "DEVSQ",                318,    318,    1,  MX, V, { RX }, 0 },
    { "GEOMEAN",                "GEOMEAN",              319,    319,    1,  MX, V, { RX }, 0 },
    { "HARMEAN",                "HARMEAN",              320,    320,    1,  MX, V, { RX }, 0 },
    { "SUMSQ",                  "SUMSQ",                321,    321,    0,  MX, V, { RX }, 0 },
    { "KURT",                   "KURT",                 322,    322,    1,  MX, V, { RX }, 0 },
    { "SKEW",                   "SKEW",                 323,    323,    1,  MX, V, { RX }, 0 },
    { "ZTEST",                  "ZTEST",                324,    324,    2,  3,  V, { RX, VR }, 0 },
    { "LARGE",                  "LARGE",                325,    325,    2,  2,  V, { RX, VR }, 0 },
    { "SMALL",                  "SMALL",                326,    326,    2,  2,  V, { RX, VR }, 0 },
    { "QUARTILE",               "QUARTILE",             327,    327,    2,  2,  V, { RX, VR }, 0 },
    { "PERCENTILE",             "PERCENTILE",           328,    328,    2,  2,  V, { RX, VR }, 0 },
    { "PERCENTRANK",            "PERCENTRANK",          329,    329,    2,  3,  V, { RX, VR, VR_E }, 0 },
    { "MODE",                   "MODE",                 330,    330,    1,  MX, V, { VA }, 0 },
    { "TRIMMEAN",               "TRIMMEAN",             331,    331,    2,  2,  V, { RX, VR }, 0 },
    { "TINV",                   "TINV",                 332,    332,    2,  2,  V, { VR }, 0 },

    // *** Analysis add-in ***

    { "HEX2BIN",                "HEX2BIN",              384,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "HEX2DEC",                "HEX2DEC",              385,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "HEX2OCT",                "HEX2OCT",              386,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DEC2BIN",                "DEC2BIN",              387,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DEC2HEX",                "DEC2HEX",              388,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DEC2OCT",                "DEC2OCT",              389,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "OCT2BIN",                "OCT2BIN",              390,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "OCT2HEX",                "OCT2HEX",              391,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "OCT2DEC",                "OCT2DEC",              392,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "BIN2DEC",                "BIN2DEC",              393,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "BIN2OCT",                "BIN2OCT",              394,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "BIN2HEX",                "BIN2HEX",              395,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMSUB",                  "IMSUB",                396,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMDIV",                  "IMDIV",                397,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMPOWER",                "IMPOWER",              398,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMABS",                  "IMABS",                399,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMSQRT",                 "IMSQRT",               400,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMLN",                   "IMLN",                 401,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMLOG2",                 "IMLOG2",               402,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMLOG10",                "IMLOG10",              403,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMSIN",                  "IMSIN",                404,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMCOS",                  "IMCOS",                405,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMEXP",                  "IMEXP",                406,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMARGUMENT",             "IMARGUMENT",           407,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMCONJUGATE",            "IMCONJUGATE",          408,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMAGINARY",              "IMAGINARY",            409,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMREAL",                 "IMREAL",               410,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "COMPLEX",                "COMPLEX",              411,    NOID,   2,  3,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "IMSUM",                  "IMSUM",                412,    NOID,   1,  MX, V, { RX }, FUNCFLAG_EXTERNAL },
    { "IMPRODUCT",              "IMPRODUCT",            413,    NOID,   1,  MX, V, { RX }, FUNCFLAG_EXTERNAL },
    { "SERIESSUM",              "SERIESSUM",            414,    NOID,   4,  4,  V, { RR, RR, RR, RX }, FUNCFLAG_EXTERNAL },
    { "FACTDOUBLE",             "FACTDOUBLE",           415,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "SQRTPI",                 "SQRTPI",               416,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "QUOTIENT",               "QUOTIENT",             417,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DELTA",                  "DELTA",                418,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "GESTEP",                 "GESTEP",               419,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ISEVEN",                 "ISEVEN",               420,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "ISODD",                  "ISODD",                421,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "MROUND",                 "MROUND",               422,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ERF",                    "ERF",                  423,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ERFC",                   "ERFC",                 424,    NOID,   1,  1,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "BESSELJ",                "BESSELJ",              425,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "BESSELK",                "BESSELK",              426,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "BESSELY",                "BESSELY",              427,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "BESSELI",                "BESSELI",              428,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "XIRR",                   "XIRR",                 429,    NOID,   2,  3,  V, { RX, RX, RR }, FUNCFLAG_EXTERNAL },
    { "XNPV",                   "XNPV",                 430,    NOID,   3,  3,  V, { RR, RX, RX }, FUNCFLAG_EXTERNAL },
    { "PRICEMAT",               "PRICEMAT",             431,    NOID,   5,  6,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "YIELDMAT",               "YIELDMAT",             432,    NOID,   5,  6,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "INTRATE",                "INTRATE",              433,    NOID,   4,  5,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "RECEIVED",               "RECEIVED",             434,    NOID,   4,  5,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DISC",                   "DISC",                 435,    NOID,   4,  5,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "PRICEDISC",              "PRICEDISC",            436,    NOID,   4,  5,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "YIELDDISC",              "YIELDDISC",            437,    NOID,   4,  5,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "TBILLEQ",                "TBILLEQ",              438,    NOID,   3,  3,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "TBILLPRICE",             "TBILLPRICE",           439,    NOID,   3,  3,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "TBILLYIELD",             "TBILLYIELD",           440,    NOID,   3,  3,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "PRICE",                  "PRICE",                441,    NOID,   6,  7,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "YIELD",                  "YIELD",                442,    NOID,   6,  7,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DOLLARDE",               "DOLLARDE",             443,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DOLLARFR",               "DOLLARFR",             444,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "NOMINAL",                "NOMINAL",              445,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "EFFECT",                 "EFFECT",               446,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "CUMPRINC",               "CUMPRINC",             447,    NOID,   6,  6,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "CUMIPMT",                "CUMIPMT",              448,    NOID,   6,  6,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "EDATE",                  "EDATE",                449,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "EOMONTH",                "EOMONTH",              450,    NOID,   2,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "YEARFRAC",               "YEARFRAC",             451,    NOID,   2,  3,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "COUPDAYBS",              "COUPDAYBS",            452,    NOID,   3,  4,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "COUPDAYS",               "COUPDAYS",             453,    NOID,   3,  4,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "COUPDAYSNC",             "COUPDAYSNC",           454,    NOID,   3,  4,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "COUPNCD",                "COUPNCD",              455,    NOID,   3,  4,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "COUPNUM",                "COUPNUM",              456,    NOID,   3,  4,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "COUPPCD",                "COUPPCD",              457,    NOID,   3,  4,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "DURATION",               "DURATION",             458,    NOID,   5,  6,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "MDURATION",              "MDURATION",            459,    NOID,   5,  6,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ODDLPRICE",              "ODDLPRICE",            460,    NOID,   7,  8,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ODDLYIELD",              "ODDLYIELD",            461,    NOID,   8,  9,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ODDFPRICE",              "ODDFPRICE",            462,    NOID,   8,  9,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ODDFYIELD",              "ODDFYIELD",            463,    NOID,   8,  9,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "RANDBETWEEN",            "RANDBETWEEN",          464,    NOID,   2,  2,  V, { RR }, FUNCFLAG_VOLATILE | FUNCFLAG_EXTERNAL },
    { "WEEKNUM",                "WEEKNUM",              465,    NOID,   1,  2,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "AMORDEGRC",              "AMORDEGRC",            466,    NOID,   6,  7,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "AMORLINC",               "AMORLINC",             467,    NOID,   6,  7,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "CONVERT",                "CONVERT",              468,    NOID,   3,  3,  V, { RR }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "ACCRINT",                "ACCRINT",              469,    NOID,   6,  7,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "ACCRINTM",               "ACCRINTM",             470,    NOID,   4,  5,  V, { RR }, FUNCFLAG_EXTERNAL },
    { "WORKDAY",                "WORKDAY",              471,    NOID,   2,  3,  V, { RR, RR, RX, C }, FUNCFLAG_EXTERNAL },
    { "NETWORKDAYS",            "NETWORKDAYS",          472,    NOID,   2,  3,  V, { RR, RR, RX, C }, FUNCFLAG_EXTERNAL },
    { "GCD",                    "GCD",                  473,    NOID,   1,  MX, V, { RX }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "MULTINOMIAL",            "MULTINOMIAL",          474,    NOID,   1,  MX, V, { RX }, FUNCFLAG_EXTERNAL },
    { "LCM",                    "LCM",                  475,    NOID,   1,  MX, V, { RX }, FUNCFLAG_EXTERNAL },       // Calc: builtin and add-in
    { "FVSCHEDULE",             "FVSCHEDULE",           476,    NOID,   2,  2,  V, { RR, RX }, FUNCFLAG_EXTERNAL },

    // *** macro sheet commands ***

    { 0,                        "ACTIVATE.NEXT",        104,    104,    0,  1,  V, { VR }, FUNCFLAG_MACROCMD },      // BIFF2-3: 0, BIFF4: 0-1
    { 0,                        "ACTIVATE.PREV",        105,    105,    0,  1,  V, { VR }, FUNCFLAG_MACROCMD }       // BIFF2-3: 0, BIFF4: 0-1
};

/** Functions new in BIFF5/BIFF7. */
static const FunctionData saFuncTableBiff5[] =
{
    { "WEEKDAY",                "WEEKDAY",              70,     70,     1,  2,  V, { VR }, 0 },                              // BIFF2-4: 1,   BIFF5: 1-2
    { "HLOOKUP",                "HLOOKUP",              101,    101,    3,  4,  V, { VV, RO, RO, VV }, 0 },                     // BIFF2-4: 3,   BIFF5: 3-4
    { "VLOOKUP",                "VLOOKUP",              102,    102,    3,  4,  V, { VV, RO, RO, VV }, 0 },                     // BIFF2-4: 3,   BIFF5: 3-4
    { "DAYS360",                "DAYS360",              220,    220,    2,  3,  V, { VR }, 0 },                              // BIFF3-4: 2,   BIFF5: 2-3
    { 0,                        "EXTERN.CALL",          255,    255,    1,  MX, R, { RO_E, RO }, FUNCFLAG_EXPORTONLY },        // MACRO or EXTERNAL
    { "CONCATENATE",            "CONCATENATE",          336,    336,    0,  MX, V, { VR }, 0 },
    { "POWER",                  "POWER",                337,    337,    2,  2,  V, { VR }, 0 },
    { "RADIANS",                "RADIANS",              342,    342,    1,  1,  V, { VR }, 0 },
    { "DEGREES",                "DEGREES",              343,    343,    1,  1,  V, { VR }, 0 },
    { "SUBTOTAL",               "SUBTOTAL",             344,    344,    2,  MX, V, { VR, RO }, 0 },
    { "SUMIF",                  "SUMIF",                345,    345,    2,  3,  V, { RO, VR, RO }, 0 },
    { "COUNTIF",                "COUNTIF",              346,    346,    2,  2,  V, { RO, VR }, 0 },
    { "COUNTBLANK",             "COUNTBLANK",           347,    347,    1,  1,  V, { RO }, 0 },
    { "ISPMT",                  "ISPMT",                350,    350,    4,  4,  V, { VR }, 0 },
    { "DATEDIF",                "DATEDIF",              351,    351,    3,  3,  V, { VR }, 0 },
    { 0,                        "DATESTRING",           352,    352,    1,  1,  V, { VR }, FUNCFLAG_IMPORTONLY },   // not supported in Calc, missing in OOXML spec
    { 0,                        "NUMBERSTRING",         353,    353,    2,  2,  V, { VR }, FUNCFLAG_IMPORTONLY },   // not supported in Calc, missing in OOXML spec
    { "ROMAN",                  "ROMAN",                354,    354,    1,  2,  V, { VR }, 0 },

    // *** EuroTool add-in ***

    { "EUROCONVERT",            "EUROCONVERT",          NOID,   NOID,   3,  5,  V, { VR }, FUNCLIB_TO_FUNCFLAGS( FUNCLIB_EUROTOOL ) },

    // *** macro sheet commands ***

    { 0,                        "ADD.MENU",             152,    152,    2,  4,  V, { VR, RO, RO, VR }, FUNCFLAG_MACROFUNC },    // BIFF3-4: 2-3, BIFF5: 2-4
    { 0,                        "ADD.COMMAND",          153,    153,    3,  5,  V, { VR, RO, RO, RO, VR }, FUNCFLAG_MACROFUNC }, // BIFF3-4: 3-4, BIFF5: 3-5
    { 0,                        "ADD.CHART.AUTOFORMAT", 390,    390,    0,  2,  V, { VR }, FUNCFLAG_MACROCMD },
    { 0,                        "ADD.LIST.ITEM",        451,    451,    0,  2,  V, { VR }, FUNCFLAG_MACROCMD },
    { 0,                        "ACTIVE.CELL.FONT",     476,    476,    0,  14, V, { VR }, FUNCFLAG_MACROCMD }
};

/** Functions new in BIFF8. */
static const FunctionData saFuncTableBiff8[] =
{
    { "GETPIVOTDATA",           "GETPIVOTDATA",         358,    358,    2,  MX, V, { RR, RR, VR, VR }, FUNCFLAG_IMPORTONLY | FUNCFLAG_PARAMPAIRS },
    { "HYPERLINK",              "HYPERLINK",            359,    359,    1,  2,  V, { VV, VO }, 0 },
    { 0,                        "PHONETIC",             360,    360,    1,  1,  V, { RO }, FUNCFLAG_IMPORTONLY },
    { "AVERAGEA",               "AVERAGEA",             361,    361,    1,  MX, V, { RX }, 0 },
    { "MAXA",                   "MAXA",                 362,    362,    1,  MX, V, { RX }, 0 },
    { "MINA",                   "MINA",                 363,    363,    1,  MX, V, { RX }, 0 },
    { "STDEVPA",                "STDEVPA",              364,    364,    1,  MX, V, { RX }, 0 },
    { "VARPA",                  "VARPA",                365,    365,    1,  MX, V, { RX }, 0 },
    { "STDEVA",                 "STDEVA",               366,    366,    1,  MX, V, { RX }, 0 },
    { "VARA",                   "VARA",                 367,    367,    1,  MX, V, { RX }, 0 },
    { "COM.MICROSOFT.BAHTTEXT", "BAHTTEXT",             368,    368,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "THAIDAYOFWEEK",        369,    369,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "THAIDIGIT",            370,    370,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "THAIMONTHOFYEAR",      371,    371,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "THAINUMSOUND",         372,    372,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "THAINUMSTRING",        373,    373,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "THAISTRINGLENGTH",     374,    374,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "ISTHAIDIGIT",          375,    375,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "ROUNDBAHTDOWN",        376,    376,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "ROUNDBAHTUP",          377,    377,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "THAIYEAR",             378,    378,    1,  1,  V, { VR }, FUNCFLAG_MACROCALL },
    { 0,                        "RTD",                  379,    379,    3,  3,  A, { VR, VR, RO }, 0 }
};

/** Functions new in OOXML. */
static const FunctionData saFuncTableOox[] =
{
    { 0,                        "CUBEVALUE",            380,    NOID,   1,  MX, V, { VR, RX }, 0 },
    { 0,                        "CUBEMEMBER",           381,    NOID,   2,  3,  V, { VR, RX, VR }, 0 },
    { 0,                        "CUBEMEMBERPROPERTY",   382,    NOID,   3,  3,  V, { VR }, 0 },
    { 0,                        "CUBERANKEDMEMBER",     383,    NOID,   3,  4,  V, { VR }, 0 },
    { 0,                        "CUBEKPIMEMBER",        477,    NOID,   3,  4,  V, { VR }, 0 },
    { 0,                        "CUBESET",              478,    NOID,   2,  5,  V, { VR, RX, VR }, 0 },
    { 0,                        "CUBESETCOUNT",         479,    NOID,   1,  1,  V, { VR }, 0 },
    { "IFERROR",                "IFERROR",              480,    NOID,   2,  2,  V, { VO, RO }, FUNCFLAG_MACROCALL },
    { "COUNTIFS",               "COUNTIFS",             481,    NOID,   2,  MX, V, { RO, VR }, FUNCFLAG_MACROCALL | FUNCFLAG_PARAMPAIRS },
    { "SUMIFS",                 "SUMIFS",               482,    NOID,   3,  MX, V, { RO, RO, VR }, FUNCFLAG_MACROCALL | FUNCFLAG_PARAMPAIRS },
    { "AVERAGEIF",              "AVERAGEIF",            483,    NOID,   2,  3,  V, { RO, VR, RO }, FUNCFLAG_MACROCALL },
    { "AVERAGEIFS",             "AVERAGEIFS",           484,    NOID,   3,  MX, V, { RO, RO, VR }, FUNCFLAG_MACROCALL | FUNCFLAG_PARAMPAIRS }
};

/** Functions new in Excel 2013.

    See http://office.microsoft.com/en-us/excel-help/new-functions-in-excel-2013-HA103980604.aspx
    Most functions apparently were added for ODF1.2 ODFF / OpenFormula
    compatibility.

    Functions with FUNCFLAG_IMPORTONLY are rewritten in
    sc/source/filter/excel/xeformula.cxx during export for
    BIFF, OOXML export uses this different mapping here but still uses the
    mapping there to determine the feature set.

    FIXME: either have the exporter determine the feature set from the active
    mapping, preferred, or enhance that mapping there such that for OOXML the
    rewrite can be overridden.

    @See sc/source/filter/excel/xlformula.cxx saFuncTable_2013
 */
/* FIXME: BIFF12 function identifiers available? Where to obtain? */
static const FunctionData saFuncTable2013[] =
{
    { "ACOT",                   "ACOT",                 NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "ACOTH",                  "ACOTH",                NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "ARABIC",                 "ARABIC",               NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "BASE",                   "BASE",                 NOID,   NOID,   2,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "BINOM.DIST.RANGE",       "BINOM.DIST.RANGE",     NOID,   NOID,   3,  4,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "BITAND",                 "BITAND",               NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "BITLSHIFT",              "BITLSHIFT",            NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "BITOR",                  "BITOR",                NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "BITRSHIFT",              "BITRSHIFT",            NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "BITXOR",                 "BITXOR",               NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    /* FIXME: CEILING.MATH is our/ODFF CEILING, but we have special handling
     * for the weird Excel CEILING behavior, check that and unify or diversify.
     * */
    { 0/*"CEILING"*/,           "CEILING.MATH",         NOID,   NOID,   1,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "COMBINA",                "COMBINA",              NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "COT",                    "COT",                  NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "COTH",                   "COTH",                 NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "CSC",                    "CSC",                  NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "CSCH",                   "CSCH",                 NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "DAYS",                   "DAYS",                 NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "DECIMAL",                "DECIMAL",              NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { 0,                        "ENCODEURL",            NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { 0,                        "FILTERXML",            NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    /* FIXME: FLOOR.MATH is our/ODFF FLOOR, but we have special handling for
     * the weird Excel FLOOR behavior, check that and unify or diversify. */
    { 0/*"FLOOR"*/,             "FLOOR.MATH",           NOID,   NOID,   1,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    // NOTE: this FDIST is not our LEGACY.FDIST
    { 0/*"FDIST"*/,             "FDIST",                NOID,   NOID,   3,  4,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    // NOTE: this FINV is not our LEGACY.FINV
    { 0/*"FINV"*/,              "FINV",                 NOID,   NOID,   3,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "FORMULA",                "FORMULATEXT",          NOID,   NOID,   1,  1,  V, { RO }, FUNCFLAG_MACROCALL_NEW },
    { "GAMMA",                  "GAMMA",                NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "GAUSS",                  "GAUSS",                NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "IFNA",                   "IFNA",                 NOID,   NOID,   2,  2,  V, { VO, RO }, FUNCFLAG_MACROCALL_NEW },
    { "IMCOSH",                 "IMCOSH",               NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "IMCOT",                  "IMCOT",                NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "IMCSC",                  "IMCSC",                NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "IMCSCH",                 "IMCSCH",               NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "IMSEC",                  "IMSEC",                NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "IMSECH",                 "IMSECH",               NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "IMSINH",                 "IMSINH",               NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "IMTAN",                  "IMTAN",                NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ISFORMULA",              "ISFORMULA",            NOID,   NOID,   1,  1,  V, { RO }, FUNCFLAG_MACROCALL_NEW },
    /* FIXME: ISOWEEKNUM vs. WEEKNUM mess needs to be sorted out before we can
     * import. */
    { 0/*"ISOWEEKNUM"*/,        "ISOWEEKNUM",           NOID,   NOID,   1,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "MUNIT",                  "MUNIT",                NOID,   NOID,   1,  1,  A, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "NUMBERVALUE",            "NUMBERVALUE",          NOID,   NOID,   1,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "PDURATION",              "PDURATION",            NOID,   NOID,   3,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "PERMUTATIONA",           "PERMUTATIONA",         NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "PHI",                    "PHI",                  NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "RRI",                    "RRI",                  NOID,   NOID,   3,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "SEC",                    "SEC",                  NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "SECH",                   "SECH",                 NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "SHEET",                  "SHEET",                NOID,   NOID,   0,  1,  V, { RO }, FUNCFLAG_MACROCALL_NEW },
    { "SHEETS",                 "SHEETS",               NOID,   NOID,   0,  1,  V, { RO }, FUNCFLAG_MACROCALL_NEW },
    { "SKEWP",                  "SKEW.P",               NOID,   NOID,   1,  MX, V, { RX }, FUNCFLAG_MACROCALL_NEW },
    { "UNICHAR",                "UNICHAR",              NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "UNICODE",                "UNICODE",              NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { 0,                        "WEBSERVICE",           NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW },
    { "XOR",                    "XOR",                  NOID,   NOID,   1,  MX, V, { RX }, FUNCFLAG_MACROCALL_NEW }
};

/** Functions defined by OpenFormula, but not supported by Calc or by Excel. */
static const FunctionData saFuncTableOdf[] =
{
    { "CHISQDIST",              0,                      NOID,   NOID,   2,  3,  V, { VR }, FUNCFLAG_MACROCALLODF },
    { "CHISQINV",               0,                      NOID,   NOID,   2,  2,  V, { VR }, FUNCFLAG_MACROCALLODF }
};

/** Functions defined by calc, but not in OpenFormula nor supported by Excel. */
static const FunctionData saFuncTableOOoLO[] =
{
    { "ORG.OPENOFFICE.WEEKS",       "ORG.OPENOFFICE.WEEKS",       NOID,   NOID,   3,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ORG.OPENOFFICE.MONTHS",      "ORG.OPENOFFICE.MONTHS",      NOID,   NOID,   3,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ORG.OPENOFFICE.YEARS",       "ORG.OPENOFFICE.YEARS",       NOID,   NOID,   3,  3,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ORG.OPENOFFICE.ISLEAPYEAR",  "ORG.OPENOFFICE.ISLEAPYEAR",  NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ORG.OPENOFFICE.DAYSINMONTH", "ORG.OPENOFFICE.DAYSINMONTH", NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ORG.OPENOFFICE.DAYSINYEAR",  "ORG.OPENOFFICE.DAYSINYEAR",  NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ORG.OPENOFFICE.WEEKSINYEAR", "ORG.OPENOFFICE.WEEKSINYEAR", NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL },
    { "ORG.OPENOFFICE.ROT13",       "ORG.OPENOFFICE.ROT13",       NOID,   NOID,   1,  1,  V, { VR }, FUNCFLAG_MACROCALL_NEW | FUNCFLAG_EXTERNAL }
};
// ----------------------------------------------------------------------------

const sal_Unicode API_TOKEN_OPEN            = '(';
const sal_Unicode API_TOKEN_CLOSE           = ')';
const sal_Unicode API_TOKEN_SEP             = ';';

const sal_Unicode API_TOKEN_ARRAY_OPEN      = '{';
const sal_Unicode API_TOKEN_ARRAY_CLOSE     = '}';
const sal_Unicode API_TOKEN_ARRAY_ROWSEP    = '|';
const sal_Unicode API_TOKEN_ARRAY_COLSEP    = ';';

} // namespace

// function info parameter class iterator =====================================

FunctionParamInfoIterator::FunctionParamInfoIterator( const FunctionInfo& rFuncInfo ) :
    mpParamInfo( rFuncInfo.mpParamInfos ),
    mpParamInfoEnd( rFuncInfo.mpParamInfos + FUNCINFO_PARAMINFOCOUNT ),
    mbParamPairs( rFuncInfo.mbParamPairs )
{
}

bool FunctionParamInfoIterator::isCalcOnlyParam() const
{
    return mpParamInfo && (mpParamInfo->meValid == FUNC_PARAM_CALCONLY);
}

bool FunctionParamInfoIterator::isExcelOnlyParam() const
{
    return mpParamInfo && (mpParamInfo->meValid == FUNC_PARAM_EXCELONLY);
}

FunctionParamInfoIterator& FunctionParamInfoIterator::operator++()
{
    if( mpParamInfo )
    {
        // move pointer to next entry, if something explicit follows
        if( (mpParamInfo + 1 < mpParamInfoEnd) && (mpParamInfo[ 1 ].meValid != FUNC_PARAM_NONE) )
            ++mpParamInfo;
        // if last parameter type is 'Excel-only' or 'Calc-only', do not repeat it
        else if( isExcelOnlyParam() || isCalcOnlyParam() )
            mpParamInfo = 0;
        // points to last info, but parameter pairs expected, move to previous info
        else if( mbParamPairs )
            --mpParamInfo;
        // otherwise: repeat last parameter class
    }
    return *this;
}

// function provider ==========================================================

struct FunctionProviderImpl
{
    typedef RefMap< OUString, FunctionInfo >    FuncNameMap;
    typedef RefMap< sal_uInt16, FunctionInfo >  FuncIdMap;

    FunctionInfoVector  maFuncs;            /// All function infos in one list.
    FuncNameMap         maOoxFuncs;         /// Maps OOXML function names to function data.
    FuncIdMap           maBiff12Funcs;      /// Maps BIFF12 function indexes to function data.
    FuncIdMap           maBiffFuncs;        /// Maps BIFF2-BIFF8 function indexes to function data.
    FuncNameMap         maMacroFuncs;       /// Maps macro function names to function data.

    explicit            FunctionProviderImpl( FilterType eFilter, BiffType eBiff, bool bImportFilter,
                                              bool bCallerKnowsAboutMacroExport );

private:
    /** Creates and inserts a function info struct from the passed function data. */
    void                initFunc( const FunctionData& rFuncData, sal_uInt8 nMaxParam );

    /** Initializes the members from the passed function data list. */
    void                initFuncs(
                            const FunctionData* pBeg, const FunctionData* pEnd,
                            sal_uInt8 nMaxParam, bool bImportFilter, FilterType eFilter );
};

// ----------------------------------------------------------------------------

FunctionProviderImpl::FunctionProviderImpl( FilterType eFilter, BiffType eBiff, bool bImportFilter,
        bool bCallerKnowsAboutMacroExport )
{
    // NOTE: this warning is only applicable if called for not yet existing
    // external export filter, not the Calc internal conversion from binary to
    // OOXML that also uses this mapping. Which is the only reason for
    // bCallerKnowsAboutMacroExport, to suppress this warning then.
    OSL_ENSURE( bImportFilter || bCallerKnowsAboutMacroExport,
            "FunctionProviderImpl::FunctionProviderImpl - need special handling for macro call functions" );
    (void)bCallerKnowsAboutMacroExport;
    sal_uInt8 nMaxParam = 0;
    switch( eFilter )
    {
        case FILTER_OOXML:
            nMaxParam = OOX_MAX_PARAMCOUNT;
            eBiff = BIFF8;  // insert all BIFF function tables, then the OOXML table
        break;
        case FILTER_BIFF:
            nMaxParam = BIFF_MAX_PARAMCOUNT;
        break;
        case FILTER_UNKNOWN:
            OSL_FAIL( "FunctionProviderImpl::FunctionProviderImpl - invalid filter type" );
        break;
    }
    OSL_ENSURE( eBiff != BIFF_UNKNOWN, "FunctionProviderImpl::FunctionProviderImpl - invalid BIFF type" );

    /*  Add functions supported in the current BIFF version only. Function
        tables from later BIFF versions may overwrite single functions from
        earlier tables. */
    if( eBiff >= BIFF2 )
        initFuncs( saFuncTableBiff2, STATIC_ARRAY_END( saFuncTableBiff2 ), nMaxParam, bImportFilter, eFilter );
    if( eBiff >= BIFF3 )
        initFuncs( saFuncTableBiff3, STATIC_ARRAY_END( saFuncTableBiff3 ), nMaxParam, bImportFilter, eFilter );
    if( eBiff >= BIFF4 )
        initFuncs( saFuncTableBiff4, STATIC_ARRAY_END( saFuncTableBiff4 ), nMaxParam, bImportFilter, eFilter );
    if( eBiff >= BIFF5 )
        initFuncs( saFuncTableBiff5, STATIC_ARRAY_END( saFuncTableBiff5 ), nMaxParam, bImportFilter, eFilter );
    if( eBiff >= BIFF8 )
        initFuncs( saFuncTableBiff8, STATIC_ARRAY_END( saFuncTableBiff8 ), nMaxParam, bImportFilter, eFilter );
    initFuncs( saFuncTableOox, STATIC_ARRAY_END( saFuncTableOox ), nMaxParam, bImportFilter, eFilter );
    initFuncs( saFuncTable2013, STATIC_ARRAY_END( saFuncTable2013 ), nMaxParam, bImportFilter, eFilter );
    initFuncs( saFuncTableOdf, STATIC_ARRAY_END( saFuncTableOdf ), nMaxParam, bImportFilter, eFilter );
    initFuncs( saFuncTableOOoLO, STATIC_ARRAY_END( saFuncTableOOoLO ), nMaxParam, bImportFilter, eFilter );
}

void FunctionProviderImpl::initFunc( const FunctionData& rFuncData, sal_uInt8 nMaxParam )
{
    // create a function info object
    FunctionInfoRef xFuncInfo( new FunctionInfo );
    if( rFuncData.mpcOdfFuncName )
        xFuncInfo->maOdfFuncName = OUString::createFromAscii( rFuncData.mpcOdfFuncName );
    if( rFuncData.mpcOoxFuncName )
        xFuncInfo->maOoxFuncName = OUString::createFromAscii( rFuncData.mpcOoxFuncName );

    if( getFlag( rFuncData.mnFlags, FUNCFLAG_MACROCALL ) )
    {
        OSL_ENSURE( !xFuncInfo->maOoxFuncName.isEmpty(), "FunctionProviderImpl::initFunc - missing OOXML function name" );
        OSL_ENSURE( !getFlag( rFuncData.mnFlags, FUNCFLAG_MACROCALLODF ), "FunctionProviderImpl::initFunc - unexpected flag FUNCFLAG_MACROCALLODF" );
        xFuncInfo->maBiffMacroName = "_xlfn." + xFuncInfo->maOoxFuncName;
        if( getFlag( rFuncData.mnFlags, FUNCFLAG_MACROCALL_FN ) )
        {
            xFuncInfo->maOoxFuncName = "_xlfn." + xFuncInfo->maOoxFuncName;
            //! From here on maOoxFuncName contains the _xlfn. prefix!
        }
    }
    else if( getFlag( rFuncData.mnFlags, FUNCFLAG_MACROCALLODF ) )
    {
        OSL_ENSURE( !xFuncInfo->maOdfFuncName.isEmpty(), "FunctionProviderImpl::initFunc - missing ODF function name" );
        xFuncInfo->maBiffMacroName = "_xlfnodf." + xFuncInfo->maOdfFuncName;
    }

    xFuncInfo->meFuncLibType = FUNCFLAGS_TO_FUNCLIB( rFuncData.mnFlags );
    xFuncInfo->mnApiOpCode = -1;
    xFuncInfo->mnBiff12FuncId = rFuncData.mnBiff12FuncId;
    xFuncInfo->mnBiffFuncId = rFuncData.mnBiffFuncId;
    xFuncInfo->mnMinParamCount = rFuncData.mnMinParamCount;
    xFuncInfo->mnMaxParamCount = (rFuncData.mnMaxParamCount == MX) ? nMaxParam : rFuncData.mnMaxParamCount;
    xFuncInfo->mnRetClass = rFuncData.mnRetClass;
    xFuncInfo->mpParamInfos = rFuncData.mpParamInfos;
    xFuncInfo->mbParamPairs = getFlag( rFuncData.mnFlags, FUNCFLAG_PARAMPAIRS );
    xFuncInfo->mbVolatile = getFlag( rFuncData.mnFlags, FUNCFLAG_VOLATILE );
    xFuncInfo->mbExternal = getFlag( rFuncData.mnFlags, FUNCFLAG_EXTERNAL );
    bool bMacroCmd = getFlag( rFuncData.mnFlags, FUNCFLAG_MACROCMD );
    xFuncInfo->mbMacroFunc = bMacroCmd || getFlag( rFuncData.mnFlags, FUNCFLAG_MACROFUNC );
    xFuncInfo->mbVarParam = bMacroCmd || (rFuncData.mnMinParamCount != rFuncData.mnMaxParamCount) || getFlag( rFuncData.mnFlags, FUNCFLAG_ALWAYSVAR );

    setFlag( xFuncInfo->mnBiff12FuncId, BIFF_TOK_FUNCVAR_CMD, bMacroCmd );
    setFlag( xFuncInfo->mnBiffFuncId, BIFF_TOK_FUNCVAR_CMD, bMacroCmd );

    // insert the function info into the member maps
    maFuncs.push_back( xFuncInfo );
    if( !xFuncInfo->maOoxFuncName.isEmpty() )
        maOoxFuncs[ xFuncInfo->maOoxFuncName ] = xFuncInfo;
    if( xFuncInfo->mnBiff12FuncId != NOID )
        maBiff12Funcs[ xFuncInfo->mnBiff12FuncId ] = xFuncInfo;
    if( xFuncInfo->mnBiffFuncId != NOID )
        maBiffFuncs[ xFuncInfo->mnBiffFuncId ] = xFuncInfo;
    if( !xFuncInfo->maBiffMacroName.isEmpty() )
        maMacroFuncs[ xFuncInfo->maBiffMacroName ] = xFuncInfo;
}

void FunctionProviderImpl::initFuncs( const FunctionData* pBeg, const FunctionData* pEnd, sal_uInt8 nMaxParam,
        bool bImportFilter, FilterType eFilter )
{
    for( const FunctionData* pIt = pBeg; pIt != pEnd; ++pIt )
        if( pIt->isSupported( bImportFilter, eFilter ) )
            initFunc( *pIt, nMaxParam );
}

// ----------------------------------------------------------------------------

FunctionProvider::FunctionProvider( FilterType eFilter, BiffType eBiff, bool bImportFilter,
        bool bCallerKnowsAboutMacroExport ) :
    mxFuncImpl( new FunctionProviderImpl( eFilter, eBiff, bImportFilter, bCallerKnowsAboutMacroExport ) )
{
}

FunctionProvider::~FunctionProvider()
{
}

const FunctionInfo* FunctionProvider::getFuncInfoFromOoxFuncName( const OUString& rFuncName ) const
{
    return mxFuncImpl->maOoxFuncs.get( rFuncName ).get();
}

const FunctionInfo* FunctionProvider::getFuncInfoFromBiff12FuncId( sal_uInt16 nFuncId ) const
{
    return mxFuncImpl->maBiff12Funcs.get( nFuncId ).get();
}

const FunctionInfo* FunctionProvider::getFuncInfoFromBiffFuncId( sal_uInt16 nFuncId ) const
{
    return mxFuncImpl->maBiffFuncs.get( nFuncId ).get();
}

const FunctionInfo* FunctionProvider::getFuncInfoFromMacroName( const OUString& rFuncName ) const
{
    return mxFuncImpl->maMacroFuncs.get( rFuncName ).get();
}

FunctionLibraryType FunctionProvider::getFuncLibTypeFromLibraryName( const OUString& rLibraryName ) const
{
#define OOX_XLS_IS_LIBNAME( libname, basename ) (libname.equalsIgnoreAsciiCase( basename ".XLA" ) || libname.equalsIgnoreAsciiCase( basename ".XLAM" ))

    // the EUROTOOL add-in containing the EUROCONVERT function
    if( OOX_XLS_IS_LIBNAME( rLibraryName, "EUROTOOL" ) )
        return FUNCLIB_EUROTOOL;

#undef OOX_XLS_IS_LIBNAME

    // default: unknown library
    return FUNCLIB_UNKNOWN;
}

const FunctionInfoVector& FunctionProvider::getFuncs() const
{
    return mxFuncImpl->maFuncs;
}

// op-code and function provider ==============================================

struct OpCodeProviderImpl : public ApiOpCodes
{
    typedef RefMap< sal_Int32, FunctionInfo >       OpCodeFuncMap;
    typedef RefMap< OUString, FunctionInfo >        FuncNameMap;
    typedef ::std::vector< FormulaOpCodeMapEntry >  OpCodeEntryVector;

    OpCodeFuncMap       maOpCodeFuncs;      /// Maps API function op-codes to function data.
    FuncNameMap         maExtProgFuncs;     /// Maps programmatical API function names to function data.
    OpCodeEntryVector   maParserMap;        /// OOXML token mapping for formula parser service.

    explicit            OpCodeProviderImpl(
                            const FunctionInfoVector& rFuncInfos,
                            const Reference< XMultiServiceFactory >& rxModelFactory );

private:
    typedef ::std::map< OUString, ApiToken >    ApiTokenMap;
    typedef Sequence< FormulaOpCodeMapEntry >   OpCodeEntrySequence;

    static bool         fillEntrySeq( OpCodeEntrySequence& orEntrySeq, const Reference< XFormulaOpCodeMapper >& rxMapper, sal_Int32 nMapGroup );
    static bool         fillTokenMap( ApiTokenMap& orTokenMap, OpCodeEntrySequence& orEntrySeq, const Reference< XFormulaOpCodeMapper >& rxMapper, sal_Int32 nMapGroup );
    bool                fillFuncTokenMaps( ApiTokenMap& orIntFuncTokenMap, ApiTokenMap& orExtFuncTokenMap, OpCodeEntrySequence& orEntrySeq, const Reference< XFormulaOpCodeMapper >& rxMapper ) const;

    static bool         initOpCode( sal_Int32& ornOpCode, const OpCodeEntrySequence& rEntrySeq, sal_Int32 nSpecialId );
    bool                initOpCode( sal_Int32& ornOpCode, const ApiTokenMap& rTokenMap, const OUString& rOdfName, const OUString& rOoxName );
    bool                initOpCode( sal_Int32& ornOpCode, const ApiTokenMap& rTokenMap, const sal_Char* pcOdfName, const sal_Char* pcOoxName );
    bool                initOpCode( sal_Int32& ornOpCode, const ApiTokenMap& rTokenMap, sal_Unicode cOdfName, sal_Unicode cOoxName );

    bool                initFuncOpCode( FunctionInfo& orFuncInfo, const ApiTokenMap& rFuncTokenMap );
    bool                initFuncOpCodes( const ApiTokenMap& rIntFuncTokenMap, const ApiTokenMap& rExtFuncTokenMap, const FunctionInfoVector& rFuncInfos );
};

// ----------------------------------------------------------------------------

OpCodeProviderImpl::OpCodeProviderImpl( const FunctionInfoVector& rFuncInfos,
        const Reference< XMultiServiceFactory >& rxModelFactory )
{
    if( rxModelFactory.is() ) try
    {
        Reference< XFormulaOpCodeMapper > xMapper( rxModelFactory->createInstance(
            "com.sun.star.sheet.FormulaOpCodeMapper" ), UNO_QUERY_THROW );

        // op-codes provided as attributes
        OPCODE_UNKNOWN = xMapper->getOpCodeUnknown();
        OPCODE_EXTERNAL = xMapper->getOpCodeExternal();

        using namespace ::com::sun::star::sheet::FormulaMapGroup;
        using namespace ::com::sun::star::sheet::FormulaMapGroupSpecialOffset;

        OpCodeEntrySequence aEntrySeq;
        ApiTokenMap aTokenMap, aExtFuncTokenMap;
        bool bIsValid =
            // special
            fillEntrySeq( aEntrySeq, xMapper, SPECIAL ) &&
            initOpCode( OPCODE_PUSH,          aEntrySeq, PUSH ) &&
            initOpCode( OPCODE_MISSING,       aEntrySeq, MISSING ) &&
            initOpCode( OPCODE_SPACES,        aEntrySeq, SPACES ) &&
            initOpCode( OPCODE_NAME,          aEntrySeq, NAME ) &&
            initOpCode( OPCODE_DBAREA,        aEntrySeq, DB_AREA ) &&
            initOpCode( OPCODE_NLR,           aEntrySeq, COL_ROW_NAME ) &&
            initOpCode( OPCODE_MACRO,         aEntrySeq, MACRO ) &&
            initOpCode( OPCODE_BAD,           aEntrySeq, BAD ) &&
            initOpCode( OPCODE_NONAME,        aEntrySeq, NO_NAME ) &&
            // separators
            fillTokenMap( aTokenMap, aEntrySeq, xMapper, SEPARATORS ) &&
            initOpCode( OPCODE_OPEN,          aTokenMap, API_TOKEN_OPEN,  '('  ) &&
            initOpCode( OPCODE_CLOSE,         aTokenMap, API_TOKEN_CLOSE, ')'  ) &&
            initOpCode( OPCODE_SEP,           aTokenMap, API_TOKEN_SEP,   ','  ) &&
            // array separators
            fillTokenMap( aTokenMap, aEntrySeq, xMapper, ARRAY_SEPARATORS ) &&
            initOpCode( OPCODE_ARRAY_OPEN,    aTokenMap, API_TOKEN_ARRAY_OPEN,   '{'  ) &&
            initOpCode( OPCODE_ARRAY_CLOSE,   aTokenMap, API_TOKEN_ARRAY_CLOSE,  '}'  ) &&
            initOpCode( OPCODE_ARRAY_ROWSEP,  aTokenMap, API_TOKEN_ARRAY_ROWSEP, ';'  ) &&
            initOpCode( OPCODE_ARRAY_COLSEP,  aTokenMap, API_TOKEN_ARRAY_COLSEP, ','  ) &&
            // unary operators
            fillTokenMap( aTokenMap, aEntrySeq, xMapper, UNARY_OPERATORS ) &&
            initOpCode( OPCODE_PLUS_SIGN,     aTokenMap, '+',  '\0' ) && // same op-code as OPCODE_ADD
            initOpCode( OPCODE_MINUS_SIGN,    aTokenMap, '-',  '-'  ) &&
            initOpCode( OPCODE_PERCENT,       aTokenMap, '%',  '%'  ) &&
            // binary operators
            fillTokenMap( aTokenMap, aEntrySeq, xMapper, BINARY_OPERATORS ) &&
            initOpCode( OPCODE_ADD,           aTokenMap, '+',  '+'  ) &&
            initOpCode( OPCODE_SUB,           aTokenMap, '-',  '-'  ) &&
            initOpCode( OPCODE_MULT,          aTokenMap, '*',  '*'  ) &&
            initOpCode( OPCODE_DIV,           aTokenMap, '/',  '/'  ) &&
            initOpCode( OPCODE_POWER,         aTokenMap, '^',  '^'  ) &&
            initOpCode( OPCODE_CONCAT,        aTokenMap, '&',  '&'  ) &&
            initOpCode( OPCODE_EQUAL,         aTokenMap, '=',  '='  ) &&
            initOpCode( OPCODE_NOT_EQUAL,     aTokenMap, "<>", "<>" ) &&
            initOpCode( OPCODE_LESS,          aTokenMap, '<',  '<'  ) &&
            initOpCode( OPCODE_LESS_EQUAL,    aTokenMap, "<=", "<=" ) &&
            initOpCode( OPCODE_GREATER,       aTokenMap, '>',  '>'  ) &&
            initOpCode( OPCODE_GREATER_EQUAL, aTokenMap, ">=", ">=" ) &&
            initOpCode( OPCODE_INTERSECT,     aTokenMap, '!',  ' '  ) &&
            initOpCode( OPCODE_LIST,          aTokenMap, '~',  ','  ) &&
            initOpCode( OPCODE_RANGE,         aTokenMap, ':',  ':'  ) &&
            // functions
            fillFuncTokenMaps( aTokenMap, aExtFuncTokenMap, aEntrySeq, xMapper ) &&
            initFuncOpCodes( aTokenMap, aExtFuncTokenMap, rFuncInfos ) &&
            initOpCode( OPCODE_DDE,           aTokenMap, "DDE", 0 );

        OSL_ENSURE( bIsValid, "OpCodeProviderImpl::OpCodeProviderImpl - opcodes not initialized" );
        (void)bIsValid;

        // OPCODE_PLUS_SIGN and OPCODE_ADD should be equal, otherwise "+" has to be passed above
        OSL_ENSURE( OPCODE_PLUS_SIGN == OPCODE_ADD, "OpCodeProviderImpl::OpCodeProviderImpl - need opcode mapping for OPCODE_PLUS_SIGN" );
    }
    catch( Exception& )
    {
        OSL_FAIL( "OpCodeProviderImpl::OpCodeProviderImpl - cannot receive formula opcode mapper" );
    }
}

bool OpCodeProviderImpl::fillEntrySeq( OpCodeEntrySequence& orEntrySeq,
        const Reference< XFormulaOpCodeMapper >& rxMapper, sal_Int32 nMapGroup )
{
    try
    {
        orEntrySeq = rxMapper->getAvailableMappings( ::com::sun::star::sheet::FormulaLanguage::ODFF, nMapGroup );
        return orEntrySeq.hasElements();
    }
    catch( Exception& )
    {
    }
    return false;
}

bool OpCodeProviderImpl::fillTokenMap( ApiTokenMap& orTokenMap, OpCodeEntrySequence& orEntrySeq,
        const Reference< XFormulaOpCodeMapper >& rxMapper, sal_Int32 nMapGroup )
{
    orTokenMap.clear();
    if( fillEntrySeq( orEntrySeq, rxMapper, nMapGroup ) )
    {
        const FormulaOpCodeMapEntry* pEntry = orEntrySeq.getConstArray();
        const FormulaOpCodeMapEntry* pEntryEnd = pEntry + orEntrySeq.getLength();
        for( ; pEntry != pEntryEnd; ++pEntry )
            orTokenMap[ pEntry->Name ] = pEntry->Token;
    }
    return orEntrySeq.hasElements();
}

bool OpCodeProviderImpl::fillFuncTokenMaps( ApiTokenMap& orIntFuncTokenMap, ApiTokenMap& orExtFuncTokenMap, OpCodeEntrySequence& orEntrySeq, const Reference< XFormulaOpCodeMapper >& rxMapper ) const
{
    orIntFuncTokenMap.clear();
    orExtFuncTokenMap.clear();
    if( fillEntrySeq( orEntrySeq, rxMapper, ::com::sun::star::sheet::FormulaMapGroup::FUNCTIONS ) )
    {
        const FormulaOpCodeMapEntry* pEntry = orEntrySeq.getConstArray();
        const FormulaOpCodeMapEntry* pEntryEnd = pEntry + orEntrySeq.getLength();
        for( ; pEntry != pEntryEnd; ++pEntry )
            ((pEntry->Token.OpCode == OPCODE_EXTERNAL) ? orExtFuncTokenMap : orIntFuncTokenMap)[ pEntry->Name ] = pEntry->Token;
    }
    return orEntrySeq.hasElements();
}

bool OpCodeProviderImpl::initOpCode( sal_Int32& ornOpCode, const OpCodeEntrySequence& rEntrySeq, sal_Int32 nSpecialId )
{
    if( (0 <= nSpecialId) && (nSpecialId < rEntrySeq.getLength()) )
    {
        ornOpCode = rEntrySeq[ nSpecialId ].Token.OpCode;
        return true;
    }
    OSL_FAIL( OStringBuffer( "OpCodeProviderImpl::initOpCode - opcode for special offset " ).
        append( nSpecialId ).append( " not found" ).getStr() );
    return false;
}

bool OpCodeProviderImpl::initOpCode( sal_Int32& ornOpCode, const ApiTokenMap& rTokenMap, const OUString& rOdfName, const OUString& rOoxName )
{
    ApiTokenMap::const_iterator aIt = rTokenMap.find( rOdfName );
    if( aIt != rTokenMap.end() )
    {
        ornOpCode = aIt->second.OpCode;
        if( !rOoxName.isEmpty() )
        {
            FormulaOpCodeMapEntry aEntry;
            aEntry.Name = rOoxName;
            aEntry.Token.OpCode = ornOpCode;
            maParserMap.push_back( aEntry );
        }
        return true;
    }
    OSL_FAIL( OStringBuffer( "OpCodeProviderImpl::initOpCode - opcode for \"" ).
        append( OUStringToOString( rOdfName, RTL_TEXTENCODING_ASCII_US ) ).
        append( "\" not found" ).getStr() );
    return false;
}

bool OpCodeProviderImpl::initOpCode( sal_Int32& ornOpCode, const ApiTokenMap& rTokenMap, const sal_Char* pcOdfName, const sal_Char* pcOoxName )
{
    OUString aOoxName;
    if( pcOoxName ) aOoxName = OUString::createFromAscii( pcOoxName );
    return initOpCode( ornOpCode, rTokenMap, OUString::createFromAscii( pcOdfName ), aOoxName );
}

bool OpCodeProviderImpl::initOpCode( sal_Int32& ornOpCode, const ApiTokenMap& rTokenMap, sal_Unicode cOdfName, sal_Unicode cOoxName )
{
    OUString aOoxName;
    if( cOoxName ) aOoxName = OUString( cOoxName );
    return initOpCode( ornOpCode, rTokenMap, OUString( cOdfName ), aOoxName );
}

bool OpCodeProviderImpl::initFuncOpCode( FunctionInfo& orFuncInfo, const ApiTokenMap& rFuncTokenMap )
{
    bool bIsValid = false;
    if( !orFuncInfo.maOdfFuncName.isEmpty() )
    {
        ApiTokenMap::const_iterator aIt = rFuncTokenMap.find( orFuncInfo.maOdfFuncName );
        if( aIt != rFuncTokenMap.end() )
        {
            orFuncInfo.mnApiOpCode = aIt->second.OpCode;
            bIsValid =
                (orFuncInfo.mnApiOpCode >= 0) &&
                (orFuncInfo.mnApiOpCode != OPCODE_UNKNOWN) &&
                (orFuncInfo.mnApiOpCode != OPCODE_NONAME);
            OSL_ENSURE( bIsValid,
                OStringBuffer( "OpCodeProviderImpl::initFuncOpCode - no valid opcode for ODF function \"" ).
                append( OUStringToOString( orFuncInfo.maOdfFuncName, RTL_TEXTENCODING_ASCII_US ) ).
                append( '"' ).getStr() );

            if( bIsValid && (orFuncInfo.mnApiOpCode == OPCODE_EXTERNAL) )
            {
                bIsValid = (aIt->second.Data >>= orFuncInfo.maExtProgName) && !orFuncInfo.maExtProgName.isEmpty();
                OSL_ENSURE( bIsValid,
                    OStringBuffer( "OpCodeProviderImpl::initFuncOpCode - no programmatical name for external function \"" ).
                    append( OUStringToOString( orFuncInfo.maOdfFuncName, RTL_TEXTENCODING_ASCII_US ) ).
                    append( '"' ).getStr() );
            }

            // add to parser map, if OOXML function name exists
            if( bIsValid && !orFuncInfo.maOoxFuncName.isEmpty() )
            {
                // create the parser map entry
                FormulaOpCodeMapEntry aEntry;
                aEntry.Name = orFuncInfo.maOoxFuncName;
                aEntry.Token = aIt->second;
                maParserMap.push_back( aEntry );
            }
        }
        else
        {
            // ignore entries for functions unknown by Calc *and* by Excel
            bIsValid = orFuncInfo.maOoxFuncName.isEmpty();
            SAL_WARN_IF( !bIsValid, "sc",
                    "OpCodeProviderImpl::initFuncOpCode - no opcode mapping for function ODF '" <<
                    orFuncInfo.maOdfFuncName << "' <-> OOXML '" << orFuncInfo.maOoxFuncName << "'");
        }
    }
    else if( orFuncInfo.mnBiffFuncId == BIFF_FUNC_EXTERNCALL )
    {
        orFuncInfo.mnApiOpCode = OPCODE_EXTERNAL;
        bIsValid = true;
    }
    else if( !orFuncInfo.maOoxFuncName.isEmpty() )
    {
        orFuncInfo.mnApiOpCode = OPCODE_BAD;
        bIsValid = true;
    }

    if( !bIsValid || (orFuncInfo.mnApiOpCode == OPCODE_UNKNOWN) || (orFuncInfo.mnApiOpCode < 0) )
        orFuncInfo.mnApiOpCode = OPCODE_NONAME;
    return bIsValid;
}

bool OpCodeProviderImpl::initFuncOpCodes( const ApiTokenMap& rIntFuncTokenMap, const ApiTokenMap& rExtFuncTokenMap, const FunctionInfoVector& rFuncInfos )
{
    bool bIsValid = true;
    for( FunctionInfoVector::const_iterator aIt = rFuncInfos.begin(), aEnd = rFuncInfos.end(); aIt != aEnd; ++aIt )
    {
        FunctionInfoRef xFuncInfo = *aIt;
        // set API opcode from ODF function name
        bIsValid &= initFuncOpCode( *xFuncInfo, xFuncInfo->mbExternal ? rExtFuncTokenMap : rIntFuncTokenMap );
        // insert the function info into the maps
        if( (xFuncInfo->mnApiOpCode != OPCODE_NONAME) && (xFuncInfo->mnApiOpCode != OPCODE_BAD) )
        {
            if( (xFuncInfo->mnApiOpCode == OPCODE_EXTERNAL) && !xFuncInfo->maExtProgName.isEmpty() )
                maExtProgFuncs[ xFuncInfo->maExtProgName ] = xFuncInfo;
            else
                maOpCodeFuncs[ xFuncInfo->mnApiOpCode ] = xFuncInfo;
        }
    }
    return bIsValid;
}

// ----------------------------------------------------------------------------

OpCodeProvider::OpCodeProvider( const Reference< XMultiServiceFactory >& rxModelFactory,
        FilterType eFilter, BiffType eBiff, bool bImportFilter, bool bCallerKnowsAboutMacroExport ) :
    FunctionProvider( eFilter, eBiff, bImportFilter, bCallerKnowsAboutMacroExport ),
    mxOpCodeImpl( new OpCodeProviderImpl( getFuncs(), rxModelFactory ) )
{
}

OpCodeProvider::~OpCodeProvider()
{
}

const ApiOpCodes& OpCodeProvider::getOpCodes() const
{
    return *mxOpCodeImpl;
}

const FunctionInfo* OpCodeProvider::getFuncInfoFromApiToken( const ApiToken& rToken ) const
{
    const FunctionInfo* pFuncInfo = 0;
    if( (rToken.OpCode == mxOpCodeImpl->OPCODE_EXTERNAL) && rToken.Data.has< OUString >() )
        pFuncInfo = mxOpCodeImpl->maExtProgFuncs.get( rToken.Data.get< OUString >() ).get();
    else if( (rToken.OpCode == mxOpCodeImpl->OPCODE_MACRO) && rToken.Data.has< OUString >() )
        pFuncInfo = getFuncInfoFromMacroName( rToken.Data.get< OUString >() );
    else if( (rToken.OpCode == mxOpCodeImpl->OPCODE_BAD) && rToken.Data.has< OUString >() )
        pFuncInfo = getFuncInfoFromOoxFuncName( rToken.Data.get< OUString >() );
    else
        pFuncInfo = mxOpCodeImpl->maOpCodeFuncs.get( rToken.OpCode ).get();
    return pFuncInfo;
}

Sequence< FormulaOpCodeMapEntry > OpCodeProvider::getOoxParserMap() const
{
    return ContainerHelper::vectorToSequence( mxOpCodeImpl->maParserMap );
}

// API formula parser wrapper =================================================

ApiParserWrapper::ApiParserWrapper(
        const Reference< XMultiServiceFactory >& rxModelFactory, const OpCodeProvider& rOpCodeProv ) :
    OpCodeProvider( rOpCodeProv )
{
    if( rxModelFactory.is() ) try
    {
        mxParser.set( rxModelFactory->createInstance( "com.sun.star.sheet.FormulaParser" ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxParser.is(), "ApiParserWrapper::ApiParserWrapper - cannot create API formula parser object" );
    maParserProps.set( mxParser );
    maParserProps.setProperty( PROP_CompileEnglish, true );
    maParserProps.setProperty( PROP_FormulaConvention, ::com::sun::star::sheet::AddressConvention::XL_OOX );
    maParserProps.setProperty( PROP_IgnoreLeadingSpaces, false );
    maParserProps.setProperty( PROP_OpCodeMap, getOoxParserMap() );
}

ApiTokenSequence ApiParserWrapper::parseFormula( const OUString& rFormula, const CellAddress& rRefPos )
{
    ApiTokenSequence aTokenSeq;
    if( mxParser.is() ) try
    {
        aTokenSeq = mxParser->parseFormula( rFormula, rRefPos );
    }
    catch( Exception& )
    {
    }
    return aTokenSeq;
}

// formula parser/printer base class for filters ==============================

namespace {

bool lclConvertToCellAddress( CellAddress& orAddress, const SingleReference& rSingleRef, sal_Int32 nForbiddenFlags, sal_Int32 nFilterBySheet )
{
    orAddress = CellAddress( static_cast< sal_Int16 >( rSingleRef.Sheet ),
        rSingleRef.Column, rSingleRef.Row );
    return
        !getFlag( rSingleRef.Flags, nForbiddenFlags ) &&
        ((nFilterBySheet < 0) || (nFilterBySheet == rSingleRef.Sheet));
}

bool lclConvertToCellRange( CellRangeAddress& orRange, const ComplexReference& rComplexRef, sal_Int32 nForbiddenFlags, sal_Int32 nFilterBySheet )
{
    orRange = CellRangeAddress( static_cast< sal_Int16 >( rComplexRef.Reference1.Sheet ),
        rComplexRef.Reference1.Column, rComplexRef.Reference1.Row,
        rComplexRef.Reference2.Column, rComplexRef.Reference2.Row );
    return
        !getFlag( rComplexRef.Reference1.Flags, nForbiddenFlags ) &&
        !getFlag( rComplexRef.Reference2.Flags, nForbiddenFlags ) &&
        (rComplexRef.Reference1.Sheet == rComplexRef.Reference2.Sheet) &&
        ((nFilterBySheet < 0) || (nFilterBySheet == rComplexRef.Reference1.Sheet));
}

enum TokenToRangeListState { STATE_REF, STATE_SEP, STATE_OPEN, STATE_CLOSE, STATE_ERROR };

TokenToRangeListState lclProcessRef( ApiCellRangeList& orRanges, const Any& rData, bool bAllowRelative, sal_Int32 nFilterBySheet )
{
    using namespace ::com::sun::star::sheet::ReferenceFlags;
    const sal_Int32 FORBIDDEN_FLAGS_DEL = COLUMN_DELETED | ROW_DELETED | SHEET_DELETED;
    const sal_Int32 FORBIDDEN_FLAGS_REL = FORBIDDEN_FLAGS_DEL | COLUMN_RELATIVE | ROW_RELATIVE | SHEET_RELATIVE | RELATIVE_NAME;

    sal_Int32 nForbiddenFlags = bAllowRelative ? FORBIDDEN_FLAGS_DEL : FORBIDDEN_FLAGS_REL;
    SingleReference aSingleRef;
    if( rData >>= aSingleRef )
    {
        CellAddress aAddress;
        // ignore invalid addresses (with #REF! errors), but do not stop parsing
        if( lclConvertToCellAddress( aAddress, aSingleRef, nForbiddenFlags, nFilterBySheet ) )
            orRanges.push_back( CellRangeAddress( aAddress.Sheet, aAddress.Column, aAddress.Row, aAddress.Column, aAddress.Row ) );
        return STATE_REF;
    }
    ComplexReference aComplexRef;
    if( rData >>= aComplexRef )
    {
        CellRangeAddress aRange;
        // ignore invalid ranges (with #REF! errors), but do not stop parsing
        if( lclConvertToCellRange( aRange, aComplexRef, nForbiddenFlags, nFilterBySheet ) )
            orRanges.push_back( aRange );
        return STATE_REF;
    }
    return STATE_ERROR;
}

TokenToRangeListState lclProcessOpen( sal_Int32& ornParenLevel )
{
    ++ornParenLevel;
    return STATE_OPEN;
}

TokenToRangeListState lclProcessClose( sal_Int32& ornParenLevel )
{
    --ornParenLevel;
    return (ornParenLevel >= 0) ? STATE_CLOSE : STATE_ERROR;
}

} // namespace

// ----------------------------------------------------------------------------

FormulaProcessorBase::FormulaProcessorBase( const WorkbookHelper& rHelper ) :
    OpCodeProvider( rHelper.getBaseFilter().getModelFactory(), rHelper.getFilterType(), rHelper.getBiff(), rHelper.getBaseFilter().isImportFilter() ),
    ApiOpCodes( getOpCodes() ),
    WorkbookHelper( rHelper )
{
}

// ----------------------------------------------------------------------------

OUString FormulaProcessorBase::generateAddress2dString( const CellAddress& rAddress, bool bAbsolute )
{
    return generateAddress2dString( BinAddress( rAddress ), bAbsolute );
}

OUString FormulaProcessorBase::generateAddress2dString( const BinAddress& rAddress, bool bAbsolute )
{
    OUStringBuffer aBuffer;
    // column
    for( sal_Int32 nTemp = rAddress.mnCol; nTemp >= 0; (nTemp /= 26) -= 1 )
        aBuffer.insert( 0, sal_Unicode( 'A' + (nTemp % 26) ) );
    if( bAbsolute )
        aBuffer.insert( 0, sal_Unicode( '$' ) );
    // row
    if( bAbsolute )
        aBuffer.append( sal_Unicode( '$' ) );
    aBuffer.append( static_cast< sal_Int32 >( rAddress.mnRow + 1 ) );
    return aBuffer.makeStringAndClear();
}

// ----------------------------------------------------------------------------

OUString FormulaProcessorBase::generateApiString( const OUString& rString )
{
    OUString aRetString = rString;
    sal_Int32 nQuotePos = aRetString.getLength();
    while( (nQuotePos = aRetString.lastIndexOf( '"', nQuotePos )) >= 0 )
        aRetString = aRetString.replaceAt( nQuotePos, 1, OUString( "\"\"" ) );
    return OUStringBuffer().append( sal_Unicode( '"' ) ).append( aRetString ).append( sal_Unicode( '"' ) ).makeStringAndClear();
}

OUString FormulaProcessorBase::generateApiArray( const Matrix< Any >& rMatrix )
{
    OSL_ENSURE( !rMatrix.empty(), "FormulaProcessorBase::generateApiArray - missing matrix values" );
    OUStringBuffer aBuffer;
    aBuffer.append( API_TOKEN_ARRAY_OPEN );
    for( size_t nRow = 0, nHeight = rMatrix.height(); nRow < nHeight; ++nRow )
    {
        if( nRow > 0 )
            aBuffer.append( API_TOKEN_ARRAY_ROWSEP );
        for( Matrix< Any >::const_iterator aBeg = rMatrix.row_begin( nRow ), aIt = aBeg, aEnd = rMatrix.row_end( nRow ); aIt != aEnd; ++aIt )
        {
            double fValue = 0.0;
            OUString aString;
            if( aIt != aBeg )
                aBuffer.append( API_TOKEN_ARRAY_COLSEP );
            if( *aIt >>= fValue )
                aBuffer.append( fValue );
            else if( *aIt >>= aString )
                aBuffer.append( generateApiString( aString ) );
            else
                aBuffer.appendAscii( "\"\"" );
        }
    }
    aBuffer.append( API_TOKEN_ARRAY_CLOSE );
    return aBuffer.makeStringAndClear();
}

// ----------------------------------------------------------------------------

Any FormulaProcessorBase::extractReference( const ApiTokenSequence& rTokens ) const
{
    ApiTokenIterator aTokenIt( rTokens, OPCODE_SPACES, true );
    if( aTokenIt.is() && (aTokenIt->OpCode == OPCODE_PUSH) )
    {
        Any aRefAny = aTokenIt->Data;
        if( !(++aTokenIt).is() && (aRefAny.has< SingleReference >() || aRefAny.has< ComplexReference >()) )
            return aRefAny;
    }
    return Any();
}

bool FormulaProcessorBase::extractCellRange( CellRangeAddress& orRange,
        const ApiTokenSequence& rTokens, bool bAllowRelative ) const
{
    ApiCellRangeList aRanges;
    lclProcessRef( aRanges, extractReference( rTokens ), bAllowRelative, -1 );
    if( !aRanges.empty() )
    {
        orRange = aRanges.front();
        return true;
    }
    return false;
}

void FormulaProcessorBase::extractCellRangeList( ApiCellRangeList& orRanges,
        const ApiTokenSequence& rTokens, bool bAllowRelative, sal_Int32 nFilterBySheet ) const
{
    orRanges.clear();
    TokenToRangeListState eState = STATE_OPEN;
    sal_Int32 nParenLevel = 0;
    for( ApiTokenIterator aIt( rTokens, OPCODE_SPACES, true ); aIt.is() && (eState != STATE_ERROR); ++aIt )
    {
        sal_Int32 nOpCode = aIt->OpCode;
        switch( eState )
        {
            // #i107275# accept OPCODE_SEP and OPCODE_LIST as separator token
            case STATE_REF:
                     if( nOpCode == OPCODE_SEP )   eState = STATE_SEP;
                else if( nOpCode == OPCODE_LIST )  eState = STATE_SEP;
                else if( nOpCode == OPCODE_CLOSE ) eState = lclProcessClose( nParenLevel );
                else                               eState = STATE_ERROR;
            break;
            case STATE_SEP:
                     if( nOpCode == OPCODE_PUSH )  eState = lclProcessRef( orRanges, aIt->Data, bAllowRelative, nFilterBySheet );
                else if( nOpCode == OPCODE_SEP )   eState = STATE_SEP;
                else if( nOpCode == OPCODE_LIST )  eState = STATE_SEP;
                else if( nOpCode == OPCODE_OPEN )  eState = lclProcessOpen( nParenLevel );
                else if( nOpCode == OPCODE_CLOSE ) eState = lclProcessClose( nParenLevel );
                else                               eState = STATE_ERROR;
            break;
            case STATE_OPEN:
                     if( nOpCode == OPCODE_PUSH )  eState = lclProcessRef( orRanges, aIt->Data, bAllowRelative, nFilterBySheet );
                else if( nOpCode == OPCODE_SEP )   eState = STATE_SEP;
                else if( nOpCode == OPCODE_LIST )  eState = STATE_SEP;
                else if( nOpCode == OPCODE_OPEN )  eState = lclProcessOpen( nParenLevel );
                else if( nOpCode == OPCODE_CLOSE ) eState = lclProcessClose( nParenLevel );
                else                               eState = STATE_ERROR;
            break;
            case STATE_CLOSE:
                     if( nOpCode == OPCODE_SEP )   eState = STATE_SEP;
                else if( nOpCode == OPCODE_LIST )  eState = STATE_SEP;
                else if( nOpCode == OPCODE_CLOSE ) eState = lclProcessClose( nParenLevel );
                else                               eState = STATE_ERROR;
            break;
            default:;
        }
    }

    if( eState == STATE_ERROR )
        orRanges.clear();
    else
        getAddressConverter().validateCellRangeList( orRanges, false );
}

bool FormulaProcessorBase::extractString( OUString& orString, const ApiTokenSequence& rTokens ) const
{
    ApiTokenIterator aTokenIt( rTokens, OPCODE_SPACES, true );
    return aTokenIt.is() && (aTokenIt->OpCode == OPCODE_PUSH) && (aTokenIt->Data >>= orString) && !(++aTokenIt).is();
}

bool FormulaProcessorBase::extractSpecialTokenInfo( ApiSpecialTokenInfo& orTokenInfo, const ApiTokenSequence& rTokens ) const
{
    ApiTokenIterator aTokenIt( rTokens, OPCODE_SPACES, true );
    return aTokenIt.is() && (aTokenIt->OpCode == OPCODE_BAD) && (aTokenIt->Data >>= orTokenInfo);
}

void FormulaProcessorBase::convertStringToStringList(
        ApiTokenSequence& orTokens, sal_Unicode cStringSep, bool bTrimLeadingSpaces ) const
{
    OUString aString;
    if( extractString( aString, orTokens ) && !aString.isEmpty() )
    {
        ::std::vector< ApiToken > aNewTokens;
        sal_Int32 nPos = 0;
        sal_Int32 nLen = aString.getLength();
        while( (0 <= nPos) && (nPos < nLen) )
        {
            OUString aEntry = aString.getToken( 0, cStringSep, nPos );
            if( bTrimLeadingSpaces )
            {
                sal_Int32 nStart = 0;
                while( (nStart < aEntry.getLength()) && (aEntry[ nStart ] == ' ') ) ++nStart;
                aEntry = aEntry.copy( nStart );
            }
            if( !aNewTokens.empty() )
                aNewTokens.push_back( ApiToken( OPCODE_SEP, Any() ) );
            aNewTokens.push_back( ApiToken( OPCODE_PUSH, Any( aEntry ) ) );
        }
        orTokens = ContainerHelper::vectorToSequence( aNewTokens );
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
