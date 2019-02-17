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

#include <formulabase.hxx>
#include <rangelst.hxx>
#include <addressconverter.hxx>

#include <map>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/token/properties.hxx>
#include <o3tl/typed_flags_set.hxx>

enum class FuncFlags : sal_uInt16 {
    NONE              = 0x0000,
    VOLATILE          = 0x0001,   /// Result is volatile (e.g. NOW() function).
    IMPORTONLY        = 0x0002,   /// Only used in import filter.
    EXPORTONLY        = 0x0004,   /// Only used in export filter.
    MACROCALL         = 0x0008,   /// Function is stored as macro call in BIFF Excel (_xlfn. prefix). OOXML name MUST exist.
    MACROCALLODF      = 0x0010,   /// ODF-only function stored as macro call in BIFF Excel (_xlfnodf. prefix). ODF name MUST exist.
    EXTERNAL          = 0x0020,   /// Function is external in Calc.
    MACROFUNC         = 0x0040,   /// Function is a macro-sheet function.
    MACROCMD          = 0x0080,   /// Function is a macro-sheet command.
    ALWAYSVAR         = 0x0100,   /// Function is always represented by a tFuncVar token.
    PARAMPAIRS        = 0x0200,   /// Optional parameters are expected to appear in pairs.
    MACROCALL_FN      = 0x0400,   /** Function is stored as macro call in Excel (_xlfn. prefix)
                                      for OOXML. OOXML name MUST exist. Do not use without FuncFlags::MACROCALL. */
    MACROCALL_NEW     = MACROCALL | MACROCALL_FN, /** New Excel functions not
                                                      defined in OOXML, _xlfn. prefix in all formats. OOXML name
                                                      must exist. */
    BIFFEXPORTONLY    = 0x1000,   /// Only used in BIFF binary export filter.
    INTERNAL          = 0x2000,   /// Function is internal in Calc.
    EUROTOOL          = 0x4000,   /// function of euro tool lib, FUNCLIB_EUROTOOL
};
namespace o3tl {
    template<> struct typed_flags<FuncFlags> : is_typed_flags<FuncFlags, 0x77ff> {};
}

namespace oox {
namespace xls {

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

void BinSingleRef2d::readBiff12Data( SequenceInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_Int32 nRow;
    sal_uInt16 nCol;
    nRow = rStrm.readInt32();
    nCol = rStrm.readuInt16();
    setBiff12Data( nCol, nRow, bRelativeAsOffset );
}

void BinComplexRef2d::readBiff12Data( SequenceInputStream& rStrm, bool bRelativeAsOffset )
{
    sal_Int32 nRow1, nRow2;
    sal_uInt16 nCol1, nCol2;
    nRow1 = rStrm.readInt32();
    nRow2 = rStrm.readInt32();
    nCol1 = rStrm.readuInt16();
    nCol2 = rStrm.readuInt16();
    maRef1.setBiff12Data( nCol1, nRow1, bRelativeAsOffset );
    maRef2.setBiff12Data( nCol2, nRow2, bRelativeAsOffset );
}

// token vector, sequence =====================================================

ApiTokenVector::ApiTokenVector()
    : mvTokens()
{
}

Any& ApiTokenVector::append( sal_Int32 nOpCode )
{
    mvTokens.emplace_back();
    mvTokens.back().OpCode = nOpCode;
    return mvTokens.back().Data;
}

ApiTokenSequence ApiTokenVector::toSequence() const
{
    return ContainerHelper::vectorToSequence( mvTokens );
}

// token sequence iterator ====================================================

ApiTokenIterator::ApiTokenIterator( const ApiTokenSequence& rTokens, sal_Int32 nSpacesOpCode ) :
    mpToken( rTokens.getConstArray() ),
    mpTokenEnd( rTokens.getConstArray() + rTokens.getLength() ),
    mnSpacesOpCode( nSpacesOpCode )
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
    while( is() && (mpToken->OpCode == mnSpacesOpCode) )
        ++mpToken;
}

// function data ==============================================================

namespace {

const size_t FUNCINFO_PARAMINFOCOUNT        = 5;        /// Number of parameter type entries.

typedef std::shared_ptr< FunctionInfo > FunctionInfoRef;

struct FunctionData
{
    const sal_Char*     mpcOdfFuncName;     /// ODF function name.
    const sal_Char*     mpcOoxFuncName;     /// OOXML function name.
    sal_uInt16 const          mnBiff12FuncId;     /// BIFF12 function identifier.
    sal_uInt16 const          mnBiffFuncId;       /// BIFF2-BIFF8 function identifier.
    sal_uInt8 const           mnMinParamCount;    /// Minimum number of parameters.
    sal_uInt8 const           mnMaxParamCount;    /// Maximum number of parameters.
    sal_uInt8 const           mnRetClass;         /// BIFF token class of the return value.
    FunctionParamInfo const   mpParamInfos[ FUNCINFO_PARAMINFOCOUNT ]; /// Information about all parameters.
    FuncFlags const           mnFlags;            /// Additional flags.

    bool         isSupported(bool bImportFilter) const;
};

bool FunctionData::isSupported(bool bImportFilter) const
{
    /*  For import filters: the FuncFlags::EXPORTONLY, FuncFlags::BIFFEXPORTONLY
                            must not be set.
        For export filters: the FuncFlags::IMPORTONLY, FuncFlags::BIFFEXPORTONLY
                            must not be set. */
    if (bImportFilter)
        return !(mnFlags & ( FuncFlags::EXPORTONLY | FuncFlags::BIFFEXPORTONLY));
    else
        return !(mnFlags & ( FuncFlags::IMPORTONLY | FuncFlags::BIFFEXPORTONLY));
}

const sal_uInt16 NOID = SAL_MAX_UINT16;     /// No BIFF function identifier available.
const sal_uInt8 MX    = SAL_MAX_UINT8;      /// Maximum parameter count.

// abbreviations for function return token class
const sal_uInt8 R = BIFF_TOKCLASS_REF;
const sal_uInt8 V = BIFF_TOKCLASS_VAL;
const sal_uInt8 A = BIFF_TOKCLASS_ARR;

// abbreviations for parameter infos
#define RO   { FuncParamValidity::Regular }
#define RA   { FuncParamValidity::Regular }
#define RR   { FuncParamValidity::Regular }
#define RX   { FuncParamValidity::Regular }
#define VO   { FuncParamValidity::Regular  }
#define VV   { FuncParamValidity::Regular  }
#define VA   { FuncParamValidity::Regular  }
#define VR   { FuncParamValidity::Regular  }
#define VX   { FuncParamValidity::Regular  }
#define RO_E { FuncParamValidity::ExcelOnly }
#define VR_E { FuncParamValidity::ExcelOnly  }
#define C    { FuncParamValidity::CalcOnly }

// Note: parameter types of all macro sheet functions (FuncFlags::MACROFUNC/FuncFlags::MACROCMD) untested!

/** Functions new in BIFF2. */
static const FunctionData saFuncTableBiff2[] =
{
    { "COUNT",                  "COUNT",                0,      0,      0,  MX, V, { RX }, FuncFlags::NONE },
    { "IF",                     "IF",                   1,      1,      2,  3,  R, { VO, RO }, FuncFlags::NONE },
    { "ISNA",                   "ISNA",                 2,      2,      1,  1,  V, { VR }, FuncFlags::NONE },
    { "ISERROR",                "ISERROR",              3,      3,      1,  1,  V, { VR }, FuncFlags::NONE },
    { "SUM",                    "SUM",                  4,      4,      0,  MX, V, { RX }, FuncFlags::NONE },
    { "AVERAGE",                "AVERAGE",              5,      5,      1,  MX, V, { RX }, FuncFlags::NONE },
    { "MIN",                    "MIN",                  6,      6,      1,  MX, V, { RX }, FuncFlags::NONE },
    { "MAX",                    "MAX",                  7,      7,      1,  MX, V, { RX }, FuncFlags::NONE },
    { "ROW",                    "ROW",                  8,      8,      0,  1,  V, { RO }, FuncFlags::NONE },
    { "COLUMN",                 "COLUMN",               9,      9,      0,  1,  V, { RO }, FuncFlags::NONE },
    { "NA",                     "NA",                   10,     10,     0,  0,  V, {}, FuncFlags::NONE },
    { "NPV",                    "NPV",                  11,     11,     2,  MX, V, { VR, RX }, FuncFlags::NONE },
    { "STDEV",                  "STDEV",                12,     12,     1,  MX, V, { RX }, FuncFlags::NONE },
    { "DOLLAR",                 "DOLLAR",               13,     13,     1,  2,  V, { VR }, FuncFlags::NONE },
    { "FIXED",                  "FIXED",                14,     14,     1,  2,  V, { VR, VR, C }, FuncFlags::NONE },
    { "SIN",                    "SIN",                  15,     15,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "CSC",                    "SIN",                  15,     15,     1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "COS",                    "COS",                  16,     16,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "SEC",                    "COS",                  16,     16,     1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "TAN",                    "TAN",                  17,     17,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "COT",                    "TAN",                  17,     17,     1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "ATAN",                   "ATAN",                 18,     18,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "ACOT",                   "ATAN",                 18,     18,     1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "PI",                     "PI",                   19,     19,     0,  0,  V, {}, FuncFlags::NONE },
    { "SQRT",                   "SQRT",                 20,     20,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "EXP",                    "EXP",                  21,     21,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "LN",                     "LN",                   22,     22,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "LOG10",                  "LOG10",                23,     23,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "ABS",                    "ABS",                  24,     24,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "INT",                    "INT",                  25,     25,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "SIGN",                   "SIGN",                 26,     26,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "ROUND",                  "ROUND",                27,     27,     2,  2,  V, { VR }, FuncFlags::NONE },
    { "LOOKUP",                 "LOOKUP",               28,     28,     2,  3,  V, { VR, RA }, FuncFlags::NONE },
    { "INDEX",                  "INDEX",                29,     29,     2,  4,  R, { RA, VV }, FuncFlags::NONE },
    { "REPT",                   "REPT",                 30,     30,     2,  2,  V, { VR }, FuncFlags::NONE },
    { "MID",                    "MID",                  31,     31,     3,  3,  V, { VR }, FuncFlags::NONE },
    { "LEN",                    "LEN",                  32,     32,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "VALUE",                  "VALUE",                33,     33,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "TRUE",                   "TRUE",                 34,     34,     0,  0,  V, {}, FuncFlags::NONE },
    { "FALSE",                  "FALSE",                35,     35,     0,  0,  V, {}, FuncFlags::NONE },
    { "AND",                    "AND",                  36,     36,     1,  MX, V, { RX }, FuncFlags::NONE },
    { "OR",                     "OR",                   37,     37,     1,  MX, V, { RX }, FuncFlags::NONE },
    { "NOT",                    "NOT",                  38,     38,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "MOD",                    "MOD",                  39,     39,     2,  2,  V, { VR }, FuncFlags::NONE },
    { "DCOUNT",                 "DCOUNT",               40,     40,     3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "DSUM",                   "DSUM",                 41,     41,     3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "DAVERAGE",               "DAVERAGE",             42,     42,     3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "DMIN",                   "DMIN",                 43,     43,     3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "DMAX",                   "DMAX",                 44,     44,     3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "DSTDEV",                 "DSTDEV",               45,     45,     3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "VAR",                    "VAR",                  46,     46,     1,  MX, V, { RX }, FuncFlags::NONE },
    { "DVAR",                   "DVAR",                 47,     47,     3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "TEXT",                   "TEXT",                 48,     48,     2,  2,  V, { VR }, FuncFlags::NONE },
    { "LINEST",                 "LINEST",               49,     49,     1,  2,  A, { RA, RA, C, C }, FuncFlags::NONE },
    { "TREND",                  "TREND",                50,     50,     1,  3,  A, { RA, RA, RA, C }, FuncFlags::NONE },
    { "LOGEST",                 "LOGEST",               51,     51,     1,  2,  A, { RA, RA, C, C }, FuncFlags::NONE },
    { "GROWTH",                 "GROWTH",               52,     52,     1,  3,  A, { RA, RA, RA, C }, FuncFlags::NONE },
    { "PV",                     "PV",                   56,     56,     3,  5,  V, { VR }, FuncFlags::NONE },
    { "FV",                     "FV",                   57,     57,     3,  5,  V, { VR }, FuncFlags::NONE },
    { "NPER",                   "NPER",                 58,     58,     3,  5,  V, { VR }, FuncFlags::NONE },
    { "PMT",                    "PMT",                  59,     59,     3,  5,  V, { VR }, FuncFlags::NONE },
    { "RATE",                   "RATE",                 60,     60,     3,  6,  V, { VR }, FuncFlags::NONE },
    { "MIRR",                   "MIRR",                 61,     61,     3,  3,  V, { RA, VR }, FuncFlags::NONE },
    { "IRR",                    "IRR",                  62,     62,     1,  2,  V, { RA, VR }, FuncFlags::NONE },
    { "RAND",                   "RAND",                 63,     63,     0,  0,  V, {}, FuncFlags::VOLATILE },
    { "MATCH",                  "MATCH",                64,     64,     2,  3,  V, { VR, RX, RR }, FuncFlags::NONE },
    { "DATE",                   "DATE",                 65,     65,     3,  3,  V, { VR }, FuncFlags::NONE },
    { "TIME",                   "TIME",                 66,     66,     3,  3,  V, { VR }, FuncFlags::NONE },
    { "DAY",                    "DAY",                  67,     67,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "MONTH",                  "MONTH",                68,     68,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "YEAR",                   "YEAR",                 69,     69,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "WEEKDAY",                "WEEKDAY",              70,     70,     1,  1,  V, { VR, C }, FuncFlags::NONE },
    { "HOUR",                   "HOUR",                 71,     71,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "MINUTE",                 "MINUTE",               72,     72,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "SECOND",                 "SECOND",               73,     73,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "NOW",                    "NOW",                  74,     74,     0,  0,  V, {}, FuncFlags::VOLATILE },
    { "AREAS",                  "AREAS",                75,     75,     1,  1,  V, { RO }, FuncFlags::NONE },
    { "ROWS",                   "ROWS",                 76,     76,     1,  1,  V, { RO }, FuncFlags::NONE },
    { "COLUMNS",                "COLUMNS",              77,     77,     1,  1,  V, { RO }, FuncFlags::NONE },
    { "OFFSET",                 "OFFSET",               78,     78,     3,  5,  R, { RO, VR }, FuncFlags::VOLATILE },
    { "SEARCH",                 "SEARCH",               82,     82,     2,  3,  V, { VR }, FuncFlags::NONE },
    { "TRANSPOSE",              "TRANSPOSE",            83,     83,     1,  1,  A, { VO }, FuncFlags::NONE },
    { "TYPE",                   "TYPE",                 86,     86,     1,  1,  V, { VX }, FuncFlags::NONE },
    { "ATAN2",                  "ATAN2",                97,     97,     2,  2,  V, { VR }, FuncFlags::NONE },
    { "ASIN",                   "ASIN",                 98,     98,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "ACOS",                   "ACOS",                 99,     99,     1,  1,  V, { VR }, FuncFlags::NONE },
    { "CHOOSE",                 "CHOOSE",               100,    100,    2,  MX, R, { VO, RO }, FuncFlags::NONE },
    { "HLOOKUP",                "HLOOKUP",              101,    101,    3,  3,  V, { VV, RO, RO, C }, FuncFlags::NONE },
    { "VLOOKUP",                "VLOOKUP",              102,    102,    3,  3,  V, { VV, RO, RO, C }, FuncFlags::NONE },
    { "ISREF",                  "ISREF",                105,    105,    1,  1,  V, { RX }, FuncFlags::NONE },
    { "LOG",                    "LOG",                  109,    109,    1,  2,  V, { VR }, FuncFlags::NONE },
    { "CHAR",                   "CHAR",                 111,    111,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "LOWER",                  "LOWER",                112,    112,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "UPPER",                  "UPPER",                113,    113,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "PROPER",                 "PROPER",               114,    114,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "LEFT",                   "LEFT",                 115,    115,    1,  2,  V, { VR }, FuncFlags::NONE },
    { "RIGHT",                  "RIGHT",                116,    116,    1,  2,  V, { VR }, FuncFlags::NONE },
    { "EXACT",                  "EXACT",                117,    117,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "TRIM",                   "TRIM",                 118,    118,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "REPLACE",                "REPLACE",              119,    119,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "SUBSTITUTE",             "SUBSTITUTE",           120,    120,    3,  4,  V, { VR }, FuncFlags::NONE },
    { "CODE",                   "CODE",                 121,    121,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "FIND",                   "FIND",                 124,    124,    2,  3,  V, { VR }, FuncFlags::NONE },
    { "CELL",                   "CELL",                 125,    125,    1,  2,  V, { VV, RO }, FuncFlags::VOLATILE },
    { "ISERR",                  "ISERR",                126,    126,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ISTEXT",                 "ISTEXT",               127,    127,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ISNUMBER",               "ISNUMBER",             128,    128,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ISBLANK",                "ISBLANK",              129,    129,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "T",                      "T",                    130,    130,    1,  1,  V, { RO }, FuncFlags::NONE },
    { "N",                      "N",                    131,    131,    1,  1,  V, { RO }, FuncFlags::NONE },
    { "DATEVALUE",              "DATEVALUE",            140,    140,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "TIMEVALUE",              "TIMEVALUE",            141,    141,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "SLN",                    "SLN",                  142,    142,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "SYD",                    "SYD",                  143,    143,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "DDB",                    "DDB",                  144,    144,    4,  5,  V, { VR }, FuncFlags::NONE },
    { "INDIRECT",               "INDIRECT",             148,    148,    1,  2,  R, { VR }, FuncFlags::VOLATILE },
    { "CLEAN",                  "CLEAN",                162,    162,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "MDETERM",                "MDETERM",              163,    163,    1,  1,  V, { VA }, FuncFlags::NONE },
    { "MINVERSE",               "MINVERSE",             164,    164,    1,  1,  A, { VA }, FuncFlags::NONE },
    { "MMULT",                  "MMULT",                165,    165,    2,  2,  A, { VA }, FuncFlags::NONE },
    { "IPMT",                   "IPMT",                 167,    167,    4,  6,  V, { VR }, FuncFlags::NONE },
    { "PPMT",                   "PPMT",                 168,    168,    4,  6,  V, { VR }, FuncFlags::NONE },
    { "COUNTA",                 "COUNTA",               169,    169,    0,  MX, V, { RX }, FuncFlags::NONE },
    { "PRODUCT",                "PRODUCT",              183,    183,    0,  MX, V, { RX }, FuncFlags::NONE },
    { "FACT",                   "FACT",                 184,    184,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "DPRODUCT",               "DPRODUCT",             189,    189,    3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "ISNONTEXT",              "ISNONTEXT",            190,    190,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "STDEVP",                 "STDEVP",               193,    193,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "VARP",                   "VARP",                 194,    194,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "DSTDEVP",                "DSTDEVP",              195,    195,    3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "DVARP",                  "DVARP",                196,    196,    3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "TRUNC",                  "TRUNC",                197,    197,    1,  1,  V, { VR, C }, FuncFlags::NONE },
    { "ISLOGICAL",              "ISLOGICAL",            198,    198,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "DCOUNTA",                "DCOUNTA",              199,    199,    3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { nullptr,                        "EXTERN.CALL",          255,    255,    1,  MX, R, { RO_E, RO }, FuncFlags::IMPORTONLY },

    // *** macro sheet commands ***

    { nullptr,                        "A1.R1C1",              30,     30,     0,  1,  V, { VR }, FuncFlags::MACROCMD },
    { nullptr,                        "RETURN",               55,     55,     0,  1,  R, { RO }, FuncFlags::MACROFUNC },
    { nullptr,                        "ABSREF",               79,     79,     2,  2,  R, { VR, RO }, FuncFlags::MACROFUNC },
    { nullptr,                        "ADD.ARROW",            81,     81,     0,  0,  V, {}, FuncFlags::MACROCMD },
    { nullptr,                        "ACTIVE.CELL",          94,     94,     0,  0,  R, {}, FuncFlags::MACROFUNC },
    { nullptr,                        "ACTIVATE",             103,    103,    0,  2,  V, { VR }, FuncFlags::MACROCMD },
    { nullptr,                        "ACTIVATE.NEXT",        104,    104,    0,  0,  V, {}, FuncFlags::MACROCMD },
    { nullptr,                        "ACTIVATE.PREV",        105,    105,    0,  0,  V, {}, FuncFlags::MACROCMD },
    { nullptr,                        "ADD.BAR",              151,    151,    0,  0,  V, {}, FuncFlags::MACROFUNC | FuncFlags::ALWAYSVAR },
    { nullptr,                        "ADD.MENU",             152,    152,    2,  2,  V, { VR, RO }, FuncFlags::MACROFUNC | FuncFlags::ALWAYSVAR },
    { nullptr,                        "ADD.COMMAND",          153,    153,    3,  3,  V, { VR, RO }, FuncFlags::MACROFUNC | FuncFlags::ALWAYSVAR }
};

/** Functions new in BIFF3. */
static const FunctionData saFuncTableBiff3[] =
{
    { "LINEST",                 "LINEST",               49,     49,     1,  4,  A, { RA, RA, VV }, FuncFlags::NONE },             // BIFF2: 1-2, BIFF3: 1-4
    { "TREND",                  "TREND",                50,     50,     1,  4,  A, { RA, RA, RA, VV }, FuncFlags::NONE },             // BIFF2: 1-3, BIFF3: 1-4
    { "LOGEST",                 "LOGEST",               51,     51,     1,  4,  A, { RA, RA, VV }, FuncFlags::NONE },             // BIFF2: 1-2, BIFF3: 1-4
    { "GROWTH",                 "GROWTH",               52,     52,     1,  4,  A, { RA, RA, RA, VV }, FuncFlags::NONE },             // BIFF2: 1-3, BIFF3: 1-4
    { "TRUNC",                  "TRUNC",                197,    197,    1,  2,  V, { VR }, FuncFlags::NONE },                      // BIFF2: 1,   BIFF3: 1-2
    { "DOLLAR",                 "USDOLLAR",             204,    204,    1,  2,  V, { VR }, FuncFlags::IMPORTONLY },
    { "FINDB",                  "FINDB",                205,    205,    2,  3,  V, { VR }, FuncFlags::NONE },
    { "SEARCHB",                "SEARCHB",              206,    206,    2,  3,  V, { VR }, FuncFlags::NONE },
    { "REPLACEB",               "REPLACEB",             207,    207,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "LEFTB",                  "LEFTB",                208,    208,    1,  2,  V, { VR }, FuncFlags::NONE },
    { "RIGHTB",                 "RIGHTB",               209,    209,    1,  2,  V, { VR }, FuncFlags::NONE },
    { "MIDB",                   "MIDB",                 210,    210,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "LENB",                   "LENB",                 211,    211,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ROUNDUP",                "ROUNDUP",              212,    212,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "ROUNDDOWN",              "ROUNDDOWN",            213,    213,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "ASC",                    "ASC",                  214,    214,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "JIS",                    "DBCS",                 215,    215,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ADDRESS",                "ADDRESS",              219,    219,    2,  5,  V, { VR }, FuncFlags::NONE },
    { "DAYS360",                "DAYS360",              220,    220,    2,  2,  V, { VR, VR, C }, FuncFlags::NONE },
    { "TODAY",                  "TODAY",                221,    221,    0,  0,  V, {}, FuncFlags::VOLATILE },
    { "VDB",                    "VDB",                  222,    222,    5,  7,  V, { VR }, FuncFlags::NONE },
    { "MEDIAN",                 "MEDIAN",               227,    227,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "SUMPRODUCT",             "SUMPRODUCT",           228,    228,    1,  MX, V, { VA }, FuncFlags::NONE },
    { "SINH",                   "SINH",                 229,    229,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "CSCH",                   "SINH",                 229,    229,    1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "COSH",                   "COSH",                 230,    230,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "SECH",                   "COSH",                 230,    230,    1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "TANH",                   "TANH",                 231,    231,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "COTH",                   "TANH",                 231,    231,    1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "ASINH",                  "ASINH",                232,    232,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ACOSH",                  "ACOSH",                233,    233,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ATANH",                  "ATANH",                234,    234,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "ACOTH",                  "ATANH",                234,    234,    1,  1,  V, { VR }, FuncFlags::BIFFEXPORTONLY },
    { "DGET",                   "DGET",                 235,    235,    3,  3,  V, { RO, RR }, FuncFlags::NONE },
    { "INFO",                   "INFO",                 244,    244,    1,  1,  V, { VR }, FuncFlags::VOLATILE },

    // *** macro sheet commands ***

    { nullptr,                        "ADD.BAR",              151,    151,    0,  1,  V, { VR }, FuncFlags::MACROFUNC },    // BIFF2: 0,   BIFF3: 0-1
    { nullptr,                        "ADD.MENU",             152,    152,    2,  3,  V, { VR, RO }, FuncFlags::MACROFUNC },  // BIFF2: 2,   BIFF3: 2-3
    { nullptr,                        "ADD.COMMAND",          153,    153,    3,  4,  V, { VR, RO }, FuncFlags::MACROFUNC }   // BIFF2: 3,   BIFF3: 3-4
};

/** Functions new in BIFF4. */
static const FunctionData saFuncTableBiff4[] =
{
    { "FIXED",                  "FIXED",                14,     14,     1,  3,  V, { VR }, FuncFlags::NONE },       // BIFF2-3: 1-2, BIFF4: 1-3
    { "RANK",                   "RANK",                 216,    216,    2,  3,  V, { VR, RO, VR }, FuncFlags::NONE },
    { "DB",                     "DB",                   247,    247,    4,  5,  V, { VR }, FuncFlags::NONE },
    { "FREQUENCY",              "FREQUENCY",            252,    252,    2,  2,  A, { RA }, FuncFlags::NONE },
    { "ERROR.TYPE",             "ERROR.TYPE",           261,    261,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "AVEDEV",                 "AVEDEV",               269,    269,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "BETADIST",               "BETADIST",             270,    270,    3,  5,  V, { VR }, FuncFlags::NONE },
    { "GAMMALN",                "GAMMALN",              271,    271,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "BETAINV",                "BETAINV",              272,    272,    3,  5,  V, { VR }, FuncFlags::NONE },
    { "BINOMDIST",              "BINOMDIST",            273,    273,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "LEGACY.CHIDIST",         "CHIDIST",              274,    274,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "LEGACY.CHIINV",          "CHIINV",               275,    275,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "COMBIN",                 "COMBIN",               276,    276,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "CONFIDENCE",             "CONFIDENCE",           277,    277,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "CRITBINOM",              "CRITBINOM",            278,    278,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "EVEN",                   "EVEN",                 279,    279,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "EXPONDIST",              "EXPONDIST",            280,    280,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "LEGACY.FDIST",           "FDIST",                281,    281,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "LEGACY.FINV",            "FINV",                 282,    282,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "FISHER",                 "FISHER",               283,    283,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "FISHERINV",              "FISHERINV",            284,    284,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "COM.MICROSOFT.FLOOR",    "FLOOR",                285,    285,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "GAMMADIST",              "GAMMADIST",            286,    286,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "GAMMAINV",               "GAMMAINV",             287,    287,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "COM.MICROSOFT.CEILING",  "CEILING",              288,    288,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "HYPGEOMDIST",            "HYPGEOMDIST",          289,    289,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "LOGNORMDIST",            "LOGNORMDIST",          290,    290,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "LOGINV",                 "LOGINV",               291,    291,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "NEGBINOMDIST",           "NEGBINOMDIST",         292,    292,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "NORMDIST",               "NORMDIST",             293,    293,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "LEGACY.NORMSDIST",       "NORMSDIST",            294,    294,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "NORMINV",                "NORMINV",              295,    295,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "LEGACY.NORMSINV",        "NORMSINV",             296,    296,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "STANDARDIZE",            "STANDARDIZE",          297,    297,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "ODD",                    "ODD",                  298,    298,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "PERMUT",                 "PERMUT",               299,    299,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "POISSON",                "POISSON",              300,    300,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "LEGACY.TDIST",           "TDIST",                301,    301,    3,  3,  V, { VR }, FuncFlags::NONE },
    { "WEIBULL",                "WEIBULL",              302,    302,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "SUMXMY2",                "SUMXMY2",              303,    303,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "SUMX2MY2",               "SUMX2MY2",             304,    304,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "SUMX2PY2",               "SUMX2PY2",             305,    305,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "LEGACY.CHITEST",         "CHITEST",              306,    306,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "CORREL",                 "CORREL",               307,    307,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "COVAR",                  "COVAR",                308,    308,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "FORECAST",               "FORECAST",             309,    309,    3,  3,  V, { VR, VA }, FuncFlags::NONE },
    { "FTEST",                  "FTEST",                310,    310,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "INTERCEPT",              "INTERCEPT",            311,    311,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "PEARSON",                "PEARSON",              312,    312,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "RSQ",                    "RSQ",                  313,    313,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "STEYX",                  "STEYX",                314,    314,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "SLOPE",                  "SLOPE",                315,    315,    2,  2,  V, { VA }, FuncFlags::NONE },
    { "TTEST",                  "TTEST",                316,    316,    4,  4,  V, { VA, VA, VR }, FuncFlags::NONE },
    { "PROB",                   "PROB",                 317,    317,    3,  4,  V, { VA, VA, VR }, FuncFlags::NONE },
    { "DEVSQ",                  "DEVSQ",                318,    318,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "GEOMEAN",                "GEOMEAN",              319,    319,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "HARMEAN",                "HARMEAN",              320,    320,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "SUMSQ",                  "SUMSQ",                321,    321,    0,  MX, V, { RX }, FuncFlags::NONE },
    { "KURT",                   "KURT",                 322,    322,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "SKEW",                   "SKEW",                 323,    323,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "ZTEST",                  "ZTEST",                324,    324,    2,  3,  V, { RX, VR }, FuncFlags::NONE },
    { "LARGE",                  "LARGE",                325,    325,    2,  2,  V, { RX, VR }, FuncFlags::NONE },
    { "SMALL",                  "SMALL",                326,    326,    2,  2,  V, { RX, VR }, FuncFlags::NONE },
    { "QUARTILE",               "QUARTILE",             327,    327,    2,  2,  V, { RX, VR }, FuncFlags::NONE },
    { "PERCENTILE",             "PERCENTILE",           328,    328,    2,  2,  V, { RX, VR }, FuncFlags::NONE },
    { "PERCENTRANK",            "PERCENTRANK",          329,    329,    2,  3,  V, { RX, VR, VR_E }, FuncFlags::NONE },
    { "MODE",                   "MODE",                 330,    330,    1,  MX, V, { VA }, FuncFlags::NONE },
    { "TRIMMEAN",               "TRIMMEAN",             331,    331,    2,  2,  V, { RX, VR }, FuncFlags::NONE },
    { "TINV",                   "TINV",                 332,    332,    2,  2,  V, { VR }, FuncFlags::NONE },

    // *** Analysis add-in ***

    { "HEX2BIN",                "HEX2BIN",              384,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "HEX2DEC",                "HEX2DEC",              385,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "HEX2OCT",                "HEX2OCT",              386,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "DEC2BIN",                "DEC2BIN",              387,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "DEC2HEX",                "DEC2HEX",              388,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "DEC2OCT",                "DEC2OCT",              389,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "OCT2BIN",                "OCT2BIN",              390,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "OCT2HEX",                "OCT2HEX",              391,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "OCT2DEC",                "OCT2DEC",              392,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "BIN2DEC",                "BIN2DEC",              393,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "BIN2OCT",                "BIN2OCT",              394,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "BIN2HEX",                "BIN2HEX",              395,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMSUB",                  "IMSUB",                396,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMDIV",                  "IMDIV",                397,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMPOWER",                "IMPOWER",              398,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMABS",                  "IMABS",                399,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMSQRT",                 "IMSQRT",               400,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMLN",                   "IMLN",                 401,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMLOG2",                 "IMLOG2",               402,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMLOG10",                "IMLOG10",              403,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMSIN",                  "IMSIN",                404,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMCOS",                  "IMCOS",                405,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMEXP",                  "IMEXP",                406,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMARGUMENT",             "IMARGUMENT",           407,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMCONJUGATE",            "IMCONJUGATE",          408,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMAGINARY",              "IMAGINARY",            409,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMREAL",                 "IMREAL",               410,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "COMPLEX",                "COMPLEX",              411,    NOID,   2,  3,  V, { RR }, FuncFlags::EXTERNAL },
    { "IMSUM",                  "IMSUM",                412,    NOID,   1,  MX, V, { RX }, FuncFlags::EXTERNAL },
    { "IMPRODUCT",              "IMPRODUCT",            413,    NOID,   1,  MX, V, { RX }, FuncFlags::EXTERNAL },
    { "SERIESSUM",              "SERIESSUM",            414,    NOID,   4,  4,  V, { RR, RR, RR, RX }, FuncFlags::EXTERNAL },
    { "FACTDOUBLE",             "FACTDOUBLE",           415,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "SQRTPI",                 "SQRTPI",               416,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "QUOTIENT",               "QUOTIENT",             417,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "DELTA",                  "DELTA",                418,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "GESTEP",                 "GESTEP",               419,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "ISEVEN",                 "ISEVEN",               420,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "ISODD",                  "ISODD",                421,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "MROUND",                 "MROUND",               422,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "ERF",                    "ERF",                  423,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "ERFC",                   "ERFC",                 424,    NOID,   1,  1,  V, { RR }, FuncFlags::EXTERNAL },
    { "BESSELJ",                "BESSELJ",              425,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "BESSELK",                "BESSELK",              426,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "BESSELY",                "BESSELY",              427,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "BESSELI",                "BESSELI",              428,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "XIRR",                   "XIRR",                 429,    NOID,   2,  3,  V, { RX, RX, RR }, FuncFlags::EXTERNAL },
    { "XNPV",                   "XNPV",                 430,    NOID,   3,  3,  V, { RR, RX, RX }, FuncFlags::EXTERNAL },
    { "PRICEMAT",               "PRICEMAT",             431,    NOID,   5,  6,  V, { RR }, FuncFlags::EXTERNAL },
    { "YIELDMAT",               "YIELDMAT",             432,    NOID,   5,  6,  V, { RR }, FuncFlags::EXTERNAL },
    { "INTRATE",                "INTRATE",              433,    NOID,   4,  5,  V, { RR }, FuncFlags::EXTERNAL },
    { "RECEIVED",               "RECEIVED",             434,    NOID,   4,  5,  V, { RR }, FuncFlags::EXTERNAL },
    { "DISC",                   "DISC",                 435,    NOID,   4,  5,  V, { RR }, FuncFlags::EXTERNAL },
    { "PRICEDISC",              "PRICEDISC",            436,    NOID,   4,  5,  V, { RR }, FuncFlags::EXTERNAL },
    { "YIELDDISC",              "YIELDDISC",            437,    NOID,   4,  5,  V, { RR }, FuncFlags::EXTERNAL },
    { "TBILLEQ",                "TBILLEQ",              438,    NOID,   3,  3,  V, { RR }, FuncFlags::EXTERNAL },
    { "TBILLPRICE",             "TBILLPRICE",           439,    NOID,   3,  3,  V, { RR }, FuncFlags::EXTERNAL },
    { "TBILLYIELD",             "TBILLYIELD",           440,    NOID,   3,  3,  V, { RR }, FuncFlags::EXTERNAL },
    { "PRICE",                  "PRICE",                441,    NOID,   6,  7,  V, { RR }, FuncFlags::EXTERNAL },
    { "YIELD",                  "YIELD",                442,    NOID,   6,  7,  V, { RR }, FuncFlags::EXTERNAL },
    { "DOLLARDE",               "DOLLARDE",             443,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "DOLLARFR",               "DOLLARFR",             444,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "NOMINAL",                "NOMINAL",              445,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "EFFECT",                 "EFFECT",               446,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "CUMPRINC",               "CUMPRINC",             447,    NOID,   6,  6,  V, { RR }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "CUMIPMT",                "CUMIPMT",              448,    NOID,   6,  6,  V, { RR }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "EDATE",                  "EDATE",                449,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "EOMONTH",                "EOMONTH",              450,    NOID,   2,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "YEARFRAC",               "YEARFRAC",             451,    NOID,   2,  3,  V, { RR }, FuncFlags::EXTERNAL },
    { "COUPDAYBS",              "COUPDAYBS",            452,    NOID,   3,  4,  V, { RR }, FuncFlags::EXTERNAL },
    { "COUPDAYS",               "COUPDAYS",             453,    NOID,   3,  4,  V, { RR }, FuncFlags::EXTERNAL },
    { "COUPDAYSNC",             "COUPDAYSNC",           454,    NOID,   3,  4,  V, { RR }, FuncFlags::EXTERNAL },
    { "COUPNCD",                "COUPNCD",              455,    NOID,   3,  4,  V, { RR }, FuncFlags::EXTERNAL },
    { "COUPNUM",                "COUPNUM",              456,    NOID,   3,  4,  V, { RR }, FuncFlags::EXTERNAL },
    { "COUPPCD",                "COUPPCD",              457,    NOID,   3,  4,  V, { RR }, FuncFlags::EXTERNAL },
    { "DURATION",               "DURATION",             458,    NOID,   5,  6,  V, { RR }, FuncFlags::EXTERNAL }, // Calc: builtin and add-in (but different!)
    { "MDURATION",              "MDURATION",            459,    NOID,   5,  6,  V, { RR }, FuncFlags::EXTERNAL },
    { "ODDLPRICE",              "ODDLPRICE",            460,    NOID,   7,  8,  V, { RR }, FuncFlags::EXTERNAL },
    { "ODDLYIELD",              "ODDLYIELD",            461,    NOID,   8,  9,  V, { RR }, FuncFlags::EXTERNAL },
    { "ODDFPRICE",              "ODDFPRICE",            462,    NOID,   8,  9,  V, { RR }, FuncFlags::EXTERNAL },
    { "ODDFYIELD",              "ODDFYIELD",            463,    NOID,   8,  9,  V, { RR }, FuncFlags::EXTERNAL },
    { "RANDBETWEEN",            "RANDBETWEEN",          464,    NOID,   2,  2,  V, { RR }, FuncFlags::VOLATILE | FuncFlags::EXTERNAL },
    { "WEEKNUM",                "WEEKNUM",              465,    NOID,   1,  2,  V, { RR }, FuncFlags::EXTERNAL },
    { "AMORDEGRC",              "AMORDEGRC",            466,    NOID,   6,  7,  V, { RR }, FuncFlags::EXTERNAL },
    { "AMORLINC",               "AMORLINC",             467,    NOID,   6,  7,  V, { RR }, FuncFlags::EXTERNAL },
    { "CONVERT",                "CONVERT",              468,    NOID,   3,  3,  V, { RR }, FuncFlags::EXTERNAL },       // Calc: builtin and add-in (but different!)
    { "ACCRINT",                "ACCRINT",              469,    NOID,   6,  7,  V, { RR }, FuncFlags::EXTERNAL },
    { "ACCRINTM",               "ACCRINTM",             470,    NOID,   4,  5,  V, { RR }, FuncFlags::EXTERNAL },
    { "WORKDAY",                "WORKDAY",              471,    NOID,   2,  3,  V, { RR, RR, RX, C }, FuncFlags::EXTERNAL },
    { "NETWORKDAYS",            "NETWORKDAYS",          472,    NOID,   2,  3,  V, { RR, RR, RX, C }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "GCD",                    "GCD",                  473,    NOID,   1,  MX, V, { RX }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "MULTINOMIAL",            "MULTINOMIAL",          474,    NOID,   1,  MX, V, { RX }, FuncFlags::EXTERNAL },
    { "LCM",                    "LCM",                  475,    NOID,   1,  MX, V, { RX }, FuncFlags::EXTERNAL | FuncFlags::INTERNAL }, // Calc: builtin and add-in
    { "FVSCHEDULE",             "FVSCHEDULE",           476,    NOID,   2,  2,  V, { RR, RX }, FuncFlags::EXTERNAL },

    // *** macro sheet commands ***

    { nullptr,                        "ACTIVATE.NEXT",        104,    104,    0,  1,  V, { VR }, FuncFlags::MACROCMD },      // BIFF2-3: 0, BIFF4: 0-1
    { nullptr,                        "ACTIVATE.PREV",        105,    105,    0,  1,  V, { VR }, FuncFlags::MACROCMD }       // BIFF2-3: 0, BIFF4: 0-1
};

/** Functions new in BIFF5/BIFF7. */
static const FunctionData saFuncTableBiff5[] =
{
    { "WEEKDAY",                "WEEKDAY",              70,     70,     1,  2,  V, { VR }, FuncFlags::NONE },                              // BIFF2-4: 1,   BIFF5: 1-2
    { "HLOOKUP",                "HLOOKUP",              101,    101,    3,  4,  V, { VV, RO, RO, VV }, FuncFlags::NONE },                     // BIFF2-4: 3,   BIFF5: 3-4
    { "VLOOKUP",                "VLOOKUP",              102,    102,    3,  4,  V, { VV, RO, RO, VV }, FuncFlags::NONE },                     // BIFF2-4: 3,   BIFF5: 3-4
    { "DAYS360",                "DAYS360",              220,    220,    2,  3,  V, { VR }, FuncFlags::NONE },                              // BIFF3-4: 2,   BIFF5: 2-3
    { nullptr,                        "EXTERN.CALL",          255,    255,    1,  MX, R, { RO_E, RO }, FuncFlags::EXPORTONLY },        // MACRO or EXTERNAL
    { "CONCATENATE",            "CONCATENATE",          336,    336,    0,  MX, V, { VR }, FuncFlags::NONE },
    { "POWER",                  "POWER",                337,    337,    2,  2,  V, { VR }, FuncFlags::NONE },
    { "RADIANS",                "RADIANS",              342,    342,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "DEGREES",                "DEGREES",              343,    343,    1,  1,  V, { VR }, FuncFlags::NONE },
    { "SUBTOTAL",               "SUBTOTAL",             344,    344,    2,  MX, V, { VR, RO }, FuncFlags::NONE },
    { "SUMIF",                  "SUMIF",                345,    345,    2,  3,  V, { RO, VR, RO }, FuncFlags::NONE },
    { "COUNTIF",                "COUNTIF",              346,    346,    2,  2,  V, { RO, VR }, FuncFlags::NONE },
    { "COUNTBLANK",             "COUNTBLANK",           347,    347,    1,  1,  V, { RO }, FuncFlags::NONE },
    { "ISPMT",                  "ISPMT",                350,    350,    4,  4,  V, { VR }, FuncFlags::NONE },
    { "DATEDIF",                "DATEDIF",              351,    351,    3,  3,  V, { VR }, FuncFlags::NONE },
    { nullptr,                        "DATESTRING",           352,    352,    1,  1,  V, { VR }, FuncFlags::IMPORTONLY },   // not supported in Calc, missing in OOXML spec
    { nullptr,                        "NUMBERSTRING",         353,    353,    2,  2,  V, { VR }, FuncFlags::IMPORTONLY },   // not supported in Calc, missing in OOXML spec
    { "ROMAN",                  "ROMAN",                354,    354,    1,  2,  V, { VR }, FuncFlags::NONE },

    // *** EuroTool add-in ***
    { "EUROCONVERT",            "EUROCONVERT",          NOID,   NOID,   3,  5,  V, { VR }, FuncFlags::EUROTOOL },

    // *** macro sheet commands ***

    { nullptr,                        "ADD.MENU",             152,    152,    2,  4,  V, { VR, RO, RO, VR }, FuncFlags::MACROFUNC },    // BIFF3-4: 2-3, BIFF5: 2-4
    { nullptr,                        "ADD.COMMAND",          153,    153,    3,  5,  V, { VR, RO, RO, RO, VR }, FuncFlags::MACROFUNC }, // BIFF3-4: 3-4, BIFF5: 3-5
    { nullptr,                        "ADD.CHART.AUTOFORMAT", 390,    390,    0,  2,  V, { VR }, FuncFlags::MACROCMD },
    { nullptr,                        "ADD.LIST.ITEM",        451,    451,    0,  2,  V, { VR }, FuncFlags::MACROCMD },
    { nullptr,                        "ACTIVE.CELL.FONT",     476,    476,    0,  14, V, { VR }, FuncFlags::MACROCMD }
};

/** Functions new in BIFF8. */
static const FunctionData saFuncTableBiff8[] =
{
    { "GETPIVOTDATA",           "GETPIVOTDATA",         358,    358,    2,  MX, V, { RR, RR, VR, VR }, FuncFlags::IMPORTONLY | FuncFlags::PARAMPAIRS },
    { "HYPERLINK",              "HYPERLINK",            359,    359,    1,  2,  V, { VV, VO }, FuncFlags::NONE },
    { nullptr,                        "PHONETIC",             360,    360,    1,  1,  V, { RO }, FuncFlags::IMPORTONLY },
    { "AVERAGEA",               "AVERAGEA",             361,    361,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "MAXA",                   "MAXA",                 362,    362,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "MINA",                   "MINA",                 363,    363,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "STDEVPA",                "STDEVPA",              364,    364,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "VARPA",                  "VARPA",                365,    365,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "STDEVA",                 "STDEVA",               366,    366,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "VARA",                   "VARA",                 367,    367,    1,  MX, V, { RX }, FuncFlags::NONE },
    { "COM.MICROSOFT.BAHTTEXT", "BAHTTEXT",             368,    368,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "THAIDAYOFWEEK",        369,    369,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "THAIDIGIT",            370,    370,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "THAIMONTHOFYEAR",      371,    371,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "THAINUMSOUND",         372,    372,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "THAINUMSTRING",        373,    373,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "THAISTRINGLENGTH",     374,    374,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "ISTHAIDIGIT",          375,    375,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "ROUNDBAHTDOWN",        376,    376,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "ROUNDBAHTUP",          377,    377,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "THAIYEAR",             378,    378,    1,  1,  V, { VR }, FuncFlags::MACROCALL },
    { nullptr,                        "RTD",                  379,    379,    3,  3,  A, { VR, VR, RO }, FuncFlags::NONE }
};

/** Functions new in OOXML. */
static const FunctionData saFuncTableOox[] =
{
    { nullptr,                        "CUBEVALUE",            380,    NOID,   1,  MX, V, { VR, RX }, FuncFlags::NONE },
    { nullptr,                        "CUBEMEMBER",           381,    NOID,   2,  3,  V, { VR, RX, VR }, FuncFlags::NONE },
    { nullptr,                        "CUBEMEMBERPROPERTY",   382,    NOID,   3,  3,  V, { VR }, FuncFlags::NONE },
    { nullptr,                        "CUBERANKEDMEMBER",     383,    NOID,   3,  4,  V, { VR }, FuncFlags::NONE },
    { nullptr,                        "CUBEKPIMEMBER",        477,    NOID,   3,  4,  V, { VR }, FuncFlags::NONE },
    { nullptr,                        "CUBESET",              478,    NOID,   2,  5,  V, { VR, RX, VR }, FuncFlags::NONE },
    { nullptr,                        "CUBESETCOUNT",         479,    NOID,   1,  1,  V, { VR }, FuncFlags::NONE },
    { "IFERROR",                "IFERROR",              480,    NOID,   2,  2,  V, { VO, RO }, FuncFlags::MACROCALL },
    { "COUNTIFS",               "COUNTIFS",             481,    NOID,   2,  MX, V, { RO, VR }, FuncFlags::MACROCALL | FuncFlags::PARAMPAIRS },
    { "SUMIFS",                 "SUMIFS",               482,    NOID,   3,  MX, V, { RO, RO, VR }, FuncFlags::MACROCALL | FuncFlags::PARAMPAIRS },
    { "AVERAGEIF",              "AVERAGEIF",            483,    NOID,   2,  3,  V, { RO, VR, RO }, FuncFlags::MACROCALL },
    { "AVERAGEIFS",             "AVERAGEIFS",           484,    NOID,   3,  MX, V, { RO, RO, VR }, FuncFlags::MACROCALL | FuncFlags::PARAMPAIRS },
    { "COM.MICROSOFT.ISO.CEILING",  "ISO.CEILING",     NOID,    NOID,   1,  2,  V, { VR }, FuncFlags::MACROCALL },
    { "COM.MICROSOFT.NETWORKDAYS.INTL", "NETWORKDAYS.INTL", NOID, NOID, 2,  4,  V, { VR, VR, VR, RX }, FuncFlags::MACROCALL },
    { "COM.MICROSOFT.WORKDAY.INTL",     "WORKDAY.INTL",     NOID, NOID, 2,  4,  V, { VR, VR, VR, RX }, FuncFlags::MACROCALL }
};

/** Functions new in Excel 2010.

    A lot of statistical functions have been renamed (although the 'old' function name still is valid).
    See http://office.microsoft.com/en-us/excel-help/what-s-new-changes-made-to-excel-functions-HA010355760.aspx

    Functions with FuncFlags::IMPORTONLY are rewritten in
    sc/source/filter/excel/xeformula.cxx during export for
    BIFF, OOXML export uses this different mapping here but still uses the
    mapping there to determine the feature set.

    FIXME: either have the exporter determine the feature set from the active
    mapping, preferred, or enhance that mapping there such that for OOXML the
    rewrite can be overridden.

    @See sc/source/filter/excel/xlformula.cxx saFuncTable_2010
 */
/* FIXME: BIFF12 function identifiers available? Where to obtain? */
static const FunctionData saFuncTable2010[] =
{
    { "COM.MICROSOFT.COVARIANCE.P",           "COVARIANCE.P",        NOID,    NOID,   2,  2,  V, { VA }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.COVARIANCE.S",           "COVARIANCE.S",        NOID,    NOID,   2,  2,  V, { VA }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.STDEV.P",                "STDEV.P",             NOID,    NOID,   1, MX,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.STDEV.S",                "STDEV.S",             NOID,    NOID,   1, MX,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.VAR.P",                  "VAR.P"  ,             NOID,    NOID,   1, MX,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.VAR.S",                  "VAR.S",               NOID,    NOID,   1, MX,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.BETA.DIST",              "BETA.DIST"  ,         NOID,    NOID,   4,  6,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.BETA.INV",               "BETA.INV",            NOID,    NOID,   3,  5,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.BINOM.DIST",             "BINOM.DIST",          NOID,    NOID,   4,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.BINOM.INV",              "BINOM.INV",           NOID,    NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CHISQ.DIST",             "CHISQ.DIST",          NOID,    NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CHISQ.INV",              "CHISQ.INV",           NOID,    NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CHISQ.DIST.RT",          "CHISQ.DIST.RT",       NOID,    NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CHISQ.INV.RT",           "CHISQ.INV.RT",        NOID,    NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CHISQ.TEST",             "CHISQ.TEST",          NOID,    NOID,   2,  2,  V, { VA }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CONFIDENCE.NORM",        "CONFIDENCE.NORM",     NOID,    NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CONFIDENCE.T",           "CONFIDENCE.T",        NOID,    NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "FDIST",                                "F.DIST",              NOID,   NOID,    4,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.F.DIST.RT",              "F.DIST.RT",           NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "FINV",                                 "F.INV",               NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.F.INV.RT",               "F.INV.RT",            NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.F.TEST",                 "F.TEST",              NOID,   NOID,    2,  2,  V, { VA }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.EXPON.DIST",             "EXPON.DIST",          NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.HYPGEOM.DIST",           "HYPGEOM.DIST",        NOID,   NOID,    5,  5,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.POISSON.DIST",           "POISSON.DIST",        NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.WEIBULL.DIST",           "WEIBULL.DIST",        NOID,   NOID,    4,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.GAMMA.DIST",             "GAMMA.DIST",          NOID,   NOID,    4,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.GAMMA.INV",              "GAMMA.INV",           NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.GAMMALN.PRECISE",        "GAMMALN.PRECISE",     NOID,   NOID,    1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.LOGNORM.DIST",           "LOGNORM.DIST",        NOID,   NOID,    4,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.LOGNORM.INV",            "LOGNORM.INV",         NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.NORM.DIST",              "NORM.DIST",           NOID,   NOID,    4,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.NORM.INV",               "NORM.INV",            NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.NORM.S.DIST",            "NORM.S.DIST",         NOID,   NOID,    2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.NORM.S.INV",             "NORM.S.INV",          NOID,   NOID,    1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.T.DIST",                 "T.DIST",              NOID,   NOID,    3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.T.DIST.2T",              "T.DIST.2T",           NOID,   NOID,    2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.T.DIST.RT",              "T.DIST.RT",           NOID,   NOID,    2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.T.INV",                  "T.INV",               NOID,   NOID,    2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.T.INV.2T",               "T.INV.2T",            NOID,   NOID,    2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.T.TEST",                 "T.TEST",              NOID,   NOID,    4,  4,  V, { VA, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.PERCENTILE.INC",         "PERCENTILE.INC",      NOID,   NOID,    2,  2,  V, { RX, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.PERCENTRANK.INC",        "PERCENTRANK.INC",     NOID,   NOID,    2,  3,  V, { RX, VR, VR_E }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.QUARTILE.INC",           "QUARTILE.INC",        NOID,   NOID,    2,  2,  V, { RX, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.RANK.EQ",                "RANK.EQ",             NOID,   NOID,    2,  3,  V, { VR, RO, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.PERCENTILE.EXC",         "PERCENTILE.EXC",      NOID,   NOID,    2,  2,  V, { RX, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.PERCENTRANK.EXC",        "PERCENTRANK.EXC",     NOID,   NOID,    2,  3,  V, { RX, VR, VR_E }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.QUARTILE.EXC",           "QUARTILE.EXC",        NOID,   NOID,    2,  2,  V, { RX, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.RANK.AVG",               "RANK.AVG",            NOID,   NOID,    2,  3,  V, { VR, RO, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.MODE.SNGL",              "MODE.SNGL",           NOID,   NOID,    1,  MX, V, { VA }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.MODE.MULT",              "MODE.MULT",           NOID,   NOID,    1,  MX, V, { VA }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.NEGBINOM.DIST",          "NEGBINOM.DIST",       NOID,   NOID,    4,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.Z.TEST",                 "Z.TEST",              NOID,   NOID,    2,  3,  V, { RX, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CEILING.PRECISE",        "CEILING.PRECISE",     NOID,   NOID,    1,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.FLOOR.PRECISE",          "FLOOR.PRECISE",       NOID,   NOID,    1,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.ERF.PRECISE",            "ERF.PRECISE",         NOID,   NOID,    1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.ERFC.PRECISE",           "ERFC.PRECISE",        NOID,   NOID,    1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.AGGREGATE",              "AGGREGATE",           NOID,   NOID,    3,  MX, V, { VR, RO }, FuncFlags::MACROCALL_NEW }
};

/** Functions new in Excel 2013.

    See http://office.microsoft.com/en-us/excel-help/new-functions-in-excel-2013-HA103980604.aspx
    Most functions apparently were added for ODF1.2 ODFF / OpenFormula
    compatibility.

    Functions with FuncFlags::IMPORTONLY are rewritten in
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
    { "ACOT",                   "ACOT",                 NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "ACOTH",                  "ACOTH",                NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "ARABIC",                 "ARABIC",               NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "BASE",                   "BASE",                 NOID,   NOID,   2,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "BINOM.DIST.RANGE",       "BINOM.DIST.RANGE",     NOID,   NOID,   3,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "BITAND",                 "BITAND",               NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "BITLSHIFT",              "BITLSHIFT",            NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "BITOR",                  "BITOR",                NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "BITRSHIFT",              "BITRSHIFT",            NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "BITXOR",                 "BITXOR",               NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CEILING.MATH", "CEILING.MATH",     NOID,   NOID,   1,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "CEILING",                "CEILING.MATH",         NOID,   NOID,   1,  3,  V, { VR }, FuncFlags::EXPORTONLY | FuncFlags::MACROCALL_NEW },
    { "COMBINA",                "COMBINA",              NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COT",                    "COT",                  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COTH",                   "COTH",                 NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "CSC",                    "CSC",                  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "CSCH",                   "CSCH",                 NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "DAYS",                   "DAYS",                 NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "DECIMAL",                "DECIMAL",              NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.ENCODEURL","ENCODEURL",            NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.FILTERXML","FILTERXML",            NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.FLOOR.MATH", "FLOOR.MATH",         NOID,   NOID,   1,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "FLOOR",                  "FLOOR.MATH",           NOID,   NOID,   1,  3,  V, { VR }, FuncFlags::EXPORTONLY | FuncFlags::MACROCALL_NEW },
    // NOTE: this FDIST is not our LEGACY.FDIST
    { nullptr/*"FDIST"*/,             "FDIST",                NOID,   NOID,   3,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    // NOTE: this FINV is not our LEGACY.FINV
    { nullptr/*"FINV"*/,              "FINV",                 NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "FORMULA",                "FORMULATEXT",          NOID,   NOID,   1,  1,  V, { RO }, FuncFlags::MACROCALL_NEW },
    { "GAMMA",                  "GAMMA",                NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "GAUSS",                  "GAUSS",                NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "IFNA",                   "IFNA",                 NOID,   NOID,   2,  2,  V, { VO, RO }, FuncFlags::MACROCALL_NEW },
    { "IMCOSH",                 "IMCOSH",               NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "IMCOT",                  "IMCOT",                NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "IMCSC",                  "IMCSC",                NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "IMCSCH",                 "IMCSCH",               NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "IMSEC",                  "IMSEC",                NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "IMSECH",                 "IMSECH",               NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "IMSINH",                 "IMSINH",               NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "IMTAN",                  "IMTAN",                NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ISFORMULA",              "ISFORMULA",            NOID,   NOID,   1,  1,  V, { RO }, FuncFlags::MACROCALL_NEW },
    { "ISOWEEKNUM",             "ISOWEEKNUM",           NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "MUNIT",                  "MUNIT",                NOID,   NOID,   1,  1,  A, { VR }, FuncFlags::MACROCALL_NEW },
    { "NUMBERVALUE",            "NUMBERVALUE",          NOID,   NOID,   1,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "PDURATION",              "PDURATION",            NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "PERMUTATIONA",           "PERMUTATIONA",         NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "PHI",                    "PHI",                  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "RRI",                    "RRI",                  NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "SEC",                    "SEC",                  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "SECH",                   "SECH",                 NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "SHEET",                  "SHEET",                NOID,   NOID,   0,  1,  V, { RO }, FuncFlags::MACROCALL_NEW },
    { "SHEETS",                 "SHEETS",               NOID,   NOID,   0,  1,  V, { RO }, FuncFlags::MACROCALL_NEW },
    { "SKEWP",                  "SKEW.P",               NOID,   NOID,   1,  MX, V, { RX }, FuncFlags::MACROCALL_NEW },
    { "UNICHAR",                "UNICHAR",              NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "UNICODE",                "UNICODE",              NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.WEBSERVICE","WEBSERVICE",          NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "XOR",                    "XOR",                  NOID,   NOID,   1,  MX, V, { RX }, FuncFlags::MACROCALL_NEW }
};

/** Functions new in Excel 2016.

    See https://support.office.com/en-us/article/Forecasting-functions-897a2fe9-6595-4680-a0b0-93e0308d5f6e?ui=en-US&rs=en-US&ad=US#_forecast.ets
    and  https://support.office.com/en-us/article/What-s-New-and-Improved-in-Office-2016-for-Office-365-95c8d81d-08ba-42c1-914f-bca4603e1426?ui=en-US&rs=en-US&ad=US

    @See sc/source/filter/excel/xlformula.cxx saFuncTable_2016
 */
/* FIXME: BIFF12 function identifiers available? Where to obtain? */
static const FunctionData saFuncTable2016[] =
{
    { "COM.MICROSOFT.FORECAST.ETS",             "FORECAST.ETS",             NOID,   NOID,   3,  6,  V, { VR, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.FORECAST.ETS.CONFINT",     "FORECAST.ETS.CONFINT",     NOID,   NOID,   4,  7,  V, { VR, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.FORECAST.ETS.SEASONALITY", "FORECAST.ETS.SEASONALITY", NOID,   NOID,   2,  4,  V, { VR, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.FORECAST.ETS.STAT",        "FORECAST.ETS.STAT",        NOID,   NOID,   3,  6,  V, { VR, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.FORECAST.LINEAR",          "FORECAST.LINEAR",          NOID,   NOID,   3,  3,  V, { VR, VA }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.CONCAT",                   "CONCAT",                   NOID,   NOID,   1,  MX, V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.TEXTJOIN",                 "TEXTJOIN",                 NOID,   NOID,   3,  MX, V, { VR }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.IFS",                      "IFS",                      NOID,   NOID,   2,  MX, R, { VO, RO }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.SWITCH",                   "SWITCH",                   NOID,   NOID,   3,  MX, R, { VO, RO }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.MINIFS",                   "MINIFS",                   NOID,   NOID,   3,  MX, R, { VO, RO }, FuncFlags::MACROCALL_NEW },
    { "COM.MICROSOFT.MAXIFS",                   "MAXIFS",                   NOID,   NOID,   3,  MX, R, { VO, RO }, FuncFlags::MACROCALL_NEW }
};


/** Functions defined by OpenFormula, but not supported by Calc or by Excel. */
static const FunctionData saFuncTableOdf[] =
{
    { "CHISQDIST",              nullptr,                      NOID,   NOID,   2,  3,  V, { VR }, FuncFlags::MACROCALLODF },
    { "CHISQINV",               nullptr,                      NOID,   NOID,   2,  2,  V, { VR }, FuncFlags::MACROCALLODF }
};

/** Functions defined by Calc, but not in OpenFormula nor supported by Excel. */
static const FunctionData saFuncTableOOoLO[] =
{
    { "ORG.OPENOFFICE.WEEKS",       "ORG.OPENOFFICE.WEEKS",       NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.MONTHS",      "ORG.OPENOFFICE.MONTHS",      NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.YEARS",       "ORG.OPENOFFICE.YEARS",       NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.ISLEAPYEAR",  "ORG.OPENOFFICE.ISLEAPYEAR",  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.DAYSINMONTH", "ORG.OPENOFFICE.DAYSINMONTH", NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.DAYSINYEAR",  "ORG.OPENOFFICE.DAYSINYEAR",  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.WEEKSINYEAR", "ORG.OPENOFFICE.WEEKSINYEAR", NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.ROT13",       "ORG.OPENOFFICE.ROT13",       NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW | FuncFlags::EXTERNAL },
    /* Next 8 lines are for importing from .xlsx files saved by Calc before
     * fdo#59727 was patched with the entries above. */
    { "ORG.OPENOFFICE.WEEKS",       "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFWEEKS",   NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.MONTHS",      "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFMONTHS",  NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.YEARS",       "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFYEARS",   NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.ISLEAPYEAR",  "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETISLEAPYEAR",  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.DAYSINMONTH", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINMONTH", NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.DAYSINYEAR",  "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINYEAR",  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.WEEKSINYEAR", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETWEEKSINYEAR", NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    { "ORG.OPENOFFICE.ROT13",       "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETROT13",       NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::IMPORTONLY | FuncFlags::EXTERNAL },
    // More functions written wrongly in the past.
    { "ORG.OPENOFFICE.ERRORTYPE",   "ORG.OPENOFFICE.ERRORTYPE",     NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW  },
    { "ORG.OPENOFFICE.MULTIRANGE",  "ORG.OPENOFFICE.MULTIRANGE",    NOID,   NOID,   1, MX,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "ORG.OPENOFFICE.GOALSEEK",    "ORG.OPENOFFICE.GOALSEEK",      NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.OPENOFFICE.EASTERSUNDAY","ORG.OPENOFFICE.EASTERSUNDAY",  NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.OPENOFFICE.CURRENT",     "ORG.OPENOFFICE.CURRENT",       NOID,   NOID,   0,  0,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.OPENOFFICE.STYLE",       "ORG.OPENOFFICE.STYLE",         NOID,   NOID,   1,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    // And the import for the wrongly written functions even without _xlfn.
    { "ORG.OPENOFFICE.ERRORTYPE",   "ERRORTYPE",    NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::IMPORTONLY },
    { "ORG.OPENOFFICE.MULTIRANGE",  "MULTIRANGE",   NOID,   NOID,   1, MX,  V, { RX }, FuncFlags::IMPORTONLY },
    { "ORG.OPENOFFICE.GOALSEEK",    "GOALSEEK",     NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::IMPORTONLY },
    { "ORG.OPENOFFICE.EASTERSUNDAY","EASTERSUNDAY", NOID,   NOID,   1,  1,  V, { VR }, FuncFlags::IMPORTONLY },
    { "ORG.OPENOFFICE.CURRENT",     "CURRENT",      NOID,   NOID,   0,  0,  V, { VR }, FuncFlags::IMPORTONLY },
    { "ORG.OPENOFFICE.STYLE",       "STYLE",        NOID,   NOID,   1,  3,  V, { VR }, FuncFlags::IMPORTONLY },
    // Other functions.
    { "ORG.OPENOFFICE.CONVERT",     "ORG.OPENOFFICE.CONVERT",   NOID,   NOID,   3,  3,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.COLOR",      "ORG.LIBREOFFICE.COLOR",    NOID,   NOID,   3,  4,  V, { VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.RAWSUBTRACT","ORG.LIBREOFFICE.RAWSUBTRACT",NOID, NOID,   1, MX,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.FORECAST.ETS.MULT",      "ORG.LIBREOFFICE.FORECAST.ETS.MULT",      NOID,   NOID,   3,  6,  V, { VR, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.FORECAST.ETS.PI.MULT",   "ORG.LIBREOFFICE.FORECAST.ETS.PI.MULT",   NOID,   NOID,   4,  7,  V, { VR, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.FORECAST.ETS.STAT.MULT", "ORG.LIBREOFFICE.FORECAST.ETS.STAT.MULT", NOID,   NOID,   3,  6,  V, { VR, VA, VR }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.ROUNDSIG",   "ORG.LIBREOFFICE.ROUNDSIG", NOID, NOID,  2,  2,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.REGEX",      "ORG.LIBREOFFICE.REGEX", NOID, NOID,  2,  4,  V, { RX }, FuncFlags::MACROCALL_NEW },
    { "ORG.LIBREOFFICE.FOURIER",    "ORG.LIBREOFFICE.FOURIER", NOID, NOID,  2,  4,  A, { RX }, FuncFlags::MACROCALL_NEW }

};

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
    return mpParamInfo && (mpParamInfo->meValid == FuncParamValidity::CalcOnly);
}

bool FunctionParamInfoIterator::isExcelOnlyParam() const
{
    return mpParamInfo && (mpParamInfo->meValid == FuncParamValidity::ExcelOnly);
}

FunctionParamInfoIterator& FunctionParamInfoIterator::operator++()
{
    if( mpParamInfo )
    {
        // move pointer to next entry, if something explicit follows
        if( mpParamInfo + 1 < mpParamInfoEnd )
            ++mpParamInfo;
        // if last parameter type is 'Excel-only' or 'Calc-only', do not repeat it
        else if( isExcelOnlyParam() || isCalcOnlyParam() )
            mpParamInfo = nullptr;
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

    explicit            FunctionProviderImpl(bool bImportFilter);

private:
    /** Creates and inserts a function info struct from the passed function data. */
    void                initFunc(const FunctionData& rFuncData);

    /** Initializes the members from the passed function data list. */
    void                initFuncs(const FunctionData* pBeg, const FunctionData* pEnd, bool bImportFilter);
};

FunctionProviderImpl::FunctionProviderImpl( bool bImportFilter )
{
    /*  Add functions supported in the current BIFF version only. Function
        tables from later BIFF versions may overwrite single functions from
        earlier tables. */
    initFuncs(saFuncTableBiff2, saFuncTableBiff2 + SAL_N_ELEMENTS(saFuncTableBiff2), bImportFilter);
    initFuncs(saFuncTableBiff3, saFuncTableBiff3 + SAL_N_ELEMENTS(saFuncTableBiff3), bImportFilter);
    initFuncs(saFuncTableBiff4, saFuncTableBiff4 + SAL_N_ELEMENTS(saFuncTableBiff4), bImportFilter);
    initFuncs(saFuncTableBiff5, saFuncTableBiff5 + SAL_N_ELEMENTS(saFuncTableBiff5), bImportFilter);
    initFuncs(saFuncTableBiff8, saFuncTableBiff8 + SAL_N_ELEMENTS(saFuncTableBiff8), bImportFilter);
    initFuncs(saFuncTableOox  , saFuncTableOox   + SAL_N_ELEMENTS(saFuncTableOox  ), bImportFilter);
    initFuncs(saFuncTable2010 , saFuncTable2010  + SAL_N_ELEMENTS(saFuncTable2010 ), bImportFilter);
    initFuncs(saFuncTable2013 , saFuncTable2013  + SAL_N_ELEMENTS(saFuncTable2013 ), bImportFilter);
    initFuncs(saFuncTable2016 , saFuncTable2016  + SAL_N_ELEMENTS(saFuncTable2016 ), bImportFilter);
    initFuncs(saFuncTableOdf  , saFuncTableOdf   + SAL_N_ELEMENTS(saFuncTableOdf  ), bImportFilter);
    initFuncs(saFuncTableOOoLO, saFuncTableOOoLO + SAL_N_ELEMENTS(saFuncTableOOoLO), bImportFilter);
}

void FunctionProviderImpl::initFunc(const FunctionData& rFuncData)
{
    // create a function info object
    FunctionInfoRef xFuncInfo( new FunctionInfo );
    if( rFuncData.mpcOdfFuncName )
        xFuncInfo->maOdfFuncName = OUString::createFromAscii( rFuncData.mpcOdfFuncName );
    if( rFuncData.mpcOoxFuncName )
        xFuncInfo->maOoxFuncName = OUString::createFromAscii( rFuncData.mpcOoxFuncName );

    if( rFuncData.mnFlags & FuncFlags::MACROCALL )
    {
        OSL_ENSURE( !xFuncInfo->maOoxFuncName.isEmpty(), "FunctionProviderImpl::initFunc - missing OOXML function name" );
        OSL_ENSURE( !(rFuncData.mnFlags & FuncFlags::MACROCALLODF ), "FunctionProviderImpl::initFunc - unexpected flag FuncFlags::MACROCALLODF" );
        xFuncInfo->maBiffMacroName = "_xlfn." + xFuncInfo->maOoxFuncName;
        if( rFuncData.mnFlags & FuncFlags::MACROCALL_FN )
        {
            xFuncInfo->maOoxFuncName = "_xlfn." + xFuncInfo->maOoxFuncName;
            //! From here on maOoxFuncName contains the _xlfn. prefix!
        }
    }
    else if( rFuncData.mnFlags & FuncFlags::MACROCALLODF )
    {
        OSL_ENSURE( !xFuncInfo->maOdfFuncName.isEmpty(), "FunctionProviderImpl::initFunc - missing ODF function name" );
        xFuncInfo->maBiffMacroName = "_xlfnodf." + xFuncInfo->maOdfFuncName;
    }
    xFuncInfo->meFuncLibType = (rFuncData.mnFlags & FuncFlags::EUROTOOL) ? FUNCLIB_EUROTOOL : FUNCLIB_UNKNOWN;
    xFuncInfo->mnApiOpCode = -1;
    xFuncInfo->mnBiff12FuncId = rFuncData.mnBiff12FuncId;
    xFuncInfo->mnBiffFuncId = rFuncData.mnBiffFuncId;
    xFuncInfo->mnMinParamCount = rFuncData.mnMinParamCount;
    xFuncInfo->mnMaxParamCount = (rFuncData.mnMaxParamCount == MX) ? OOX_MAX_PARAMCOUNT : rFuncData.mnMaxParamCount;
    xFuncInfo->mnRetClass = rFuncData.mnRetClass;
    xFuncInfo->mpParamInfos = rFuncData.mpParamInfos;
    xFuncInfo->mbParamPairs = bool(rFuncData.mnFlags & FuncFlags::PARAMPAIRS);
    xFuncInfo->mbVolatile = bool(rFuncData.mnFlags & FuncFlags::VOLATILE);
    xFuncInfo->mbExternal = bool(rFuncData.mnFlags & FuncFlags::EXTERNAL);
    xFuncInfo->mbInternal = !xFuncInfo->mbExternal || ( rFuncData.mnFlags & FuncFlags::INTERNAL );
    bool bMacroCmd(rFuncData.mnFlags & FuncFlags::MACROCMD);
    xFuncInfo->mbMacroFunc = bMacroCmd || ( rFuncData.mnFlags & FuncFlags::MACROFUNC );
    xFuncInfo->mbVarParam = bMacroCmd || (rFuncData.mnMinParamCount != rFuncData.mnMaxParamCount) || ( rFuncData.mnFlags & FuncFlags::ALWAYSVAR );

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

void FunctionProviderImpl::initFuncs(const FunctionData* pBeg, const FunctionData* pEnd, bool bImportFilter)
{
    for( const FunctionData* pIt = pBeg; pIt != pEnd; ++pIt )
        if( pIt->isSupported(bImportFilter) )
            initFunc(*pIt);
}

FunctionProvider::FunctionProvider(  bool bImportFilter ) :
    mxFuncImpl( new FunctionProviderImpl( bImportFilter ) )
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

const FunctionInfo* FunctionProvider::getFuncInfoFromMacroName( const OUString& rFuncName ) const
{
    return mxFuncImpl->maMacroFuncs.get( rFuncName ).get();
}

FunctionLibraryType FunctionProvider::getFuncLibTypeFromLibraryName( const OUString& rLibraryName )
{
    // the EUROTOOL add-in containing the EUROCONVERT function
    if(   rLibraryName.equalsIgnoreAsciiCase("EUROTOOL.XLA")
       || rLibraryName.equalsIgnoreAsciiCase("EUROTOOL.XLAM"))
        return FUNCLIB_EUROTOOL;

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
            initOpCode( OPCODE_DDE,           aTokenMap, "DDE", nullptr );

        OSL_ENSURE( bIsValid, "OpCodeProviderImpl::OpCodeProviderImpl - opcodes not initialized" );

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
        orEntrySeq = rxMapper->getAvailableMappings( css::sheet::FormulaLanguage::ODFF, nMapGroup );
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
    if( fillEntrySeq( orEntrySeq, rxMapper, css::sheet::FormulaMapGroup::FUNCTIONS ) )
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
    for( const FunctionInfoRef& xFuncInfo : rFuncInfos )
    {
        // set API opcode from ODF function name
        if (xFuncInfo->mbExternal)
            bIsValid &= initFuncOpCode( *xFuncInfo, rExtFuncTokenMap );
        if (xFuncInfo->mbInternal)
            bIsValid &= initFuncOpCode( *xFuncInfo, rIntFuncTokenMap );
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

OpCodeProvider::OpCodeProvider( const Reference< XMultiServiceFactory >& rxModelFactory,
         bool bImportFilter ) :
    FunctionProvider( bImportFilter ),
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
    const FunctionInfo* pFuncInfo = nullptr;
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
    maParserProps.setProperty( PROP_FormulaConvention, css::sheet::AddressConvention::XL_OOX );
    maParserProps.setProperty( PROP_IgnoreLeadingSpaces, false );
    maParserProps.setProperty( PROP_OpCodeMap, getOoxParserMap() );
}

ApiTokenSequence ApiParserWrapper::parseFormula( const OUString& rFormula, const ScAddress& rRefPos )
{
    ApiTokenSequence aTokenSeq;
    if( mxParser.is() ) try
    {
        aTokenSeq = mxParser->parseFormula( rFormula,
                                            CellAddress(rRefPos.Tab(), rRefPos.Col(), rRefPos.Row()) );
    }
    catch( Exception& )
    {
    }
    return aTokenSeq;
}

// formula parser/printer base class for filters ==============================

namespace {

bool lclConvertToCellAddress( ScAddress& orAddress, const SingleReference& rSingleRef, sal_Int32 nForbiddenFlags, sal_Int32 nFilterBySheet )
{
    orAddress = ScAddress( rSingleRef.Column, rSingleRef.Row, rSingleRef.Sheet );
    return
        !getFlag( rSingleRef.Flags, nForbiddenFlags ) &&
        ((nFilterBySheet < 0) || (nFilterBySheet == rSingleRef.Sheet));
}

bool lclConvertToCellRange( ScRange& orRange, const ComplexReference& rComplexRef, sal_Int32 nForbiddenFlags, sal_Int32 nFilterBySheet )
{
    orRange = ScRange( rComplexRef.Reference1.Column, rComplexRef.Reference1.Row, rComplexRef.Reference1.Sheet,
                       rComplexRef.Reference2.Column, rComplexRef.Reference2.Row, rComplexRef.Reference2.Sheet );
    return
        !getFlag( rComplexRef.Reference1.Flags, nForbiddenFlags ) &&
        !getFlag( rComplexRef.Reference2.Flags, nForbiddenFlags ) &&
        (rComplexRef.Reference1.Sheet == rComplexRef.Reference2.Sheet) &&
        ((nFilterBySheet < 0) || (nFilterBySheet == rComplexRef.Reference1.Sheet));
}

enum TokenToRangeListState { STATE_REF, STATE_SEP, STATE_OPEN, STATE_CLOSE, STATE_ERROR };

TokenToRangeListState lclProcessRef( ScRangeList& orRanges, const Any& rData, sal_Int32 nFilterBySheet )
{
    using namespace ::com::sun::star::sheet::ReferenceFlags;
    const sal_Int32 FORBIDDEN_FLAGS_REL = COLUMN_DELETED | ROW_DELETED | SHEET_DELETED |
                                          COLUMN_RELATIVE | ROW_RELATIVE | SHEET_RELATIVE | RELATIVE_NAME;

    sal_Int32 nForbiddenFlags = FORBIDDEN_FLAGS_REL;
    SingleReference aSingleRef;
    if( rData >>= aSingleRef )
    {
        ScAddress aAddress;
        // ignore invalid addresses (with #REF! errors), but do not stop parsing
        if( lclConvertToCellAddress( aAddress, aSingleRef, nForbiddenFlags, nFilterBySheet ) )
            orRanges.push_back( ScRange(aAddress, aAddress) );
        return STATE_REF;
    }
    ComplexReference aComplexRef;
    if( rData >>= aComplexRef )
    {
        ScRange aRange;
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

FormulaProcessorBase::FormulaProcessorBase( const WorkbookHelper& rHelper ) :
    OpCodeProvider( rHelper.getBaseFilter().getModelFactory(), rHelper.getBaseFilter().isImportFilter() ),
    ApiOpCodes( getOpCodes() ),
    WorkbookHelper( rHelper )
{
}

OUString FormulaProcessorBase::generateAddress2dString( const ScAddress& rAddress, bool bAbsolute )
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
        aBuffer.insert( 0, '$' );
    // row
    if( bAbsolute )
        aBuffer.append( '$' );
    aBuffer.append( static_cast< sal_Int32 >( rAddress.mnRow + 1 ) );
    return aBuffer.makeStringAndClear();
}

OUString FormulaProcessorBase::generateApiString( const OUString& rString )
{
    OUString aRetString = rString;
    sal_Int32 nQuotePos = aRetString.getLength();
    while( (nQuotePos = aRetString.lastIndexOf( '"', nQuotePos )) >= 0 )
        aRetString = aRetString.replaceAt( nQuotePos, 1, "\"\"" );
    return "\"" + aRetString + "\"";
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
                aBuffer.append( "\"\"" );
        }
    }
    aBuffer.append( API_TOKEN_ARRAY_CLOSE );
    return aBuffer.makeStringAndClear();
}

Any FormulaProcessorBase::extractReference( const ApiTokenSequence& rTokens ) const
{
    ApiTokenIterator aTokenIt( rTokens, OPCODE_SPACES );
    if( aTokenIt.is() && (aTokenIt->OpCode == OPCODE_PUSH) )
    {
        Any aRefAny = aTokenIt->Data;
        if( !(++aTokenIt).is() && (aRefAny.has< SingleReference >() || aRefAny.has< ComplexReference >()) )
            return aRefAny;
    }
    return Any();
}

bool FormulaProcessorBase::extractCellRange( ScRange& orRange,
        const ApiTokenSequence& rTokens ) const
{
    ScRangeList aRanges;
    lclProcessRef( aRanges, extractReference( rTokens ), -1 );
    if( !aRanges.empty() )
    {
        orRange = aRanges.front();
        return true;
    }
    return false;
}

void FormulaProcessorBase::extractCellRangeList( ScRangeList& orRanges,
        const ApiTokenSequence& rTokens, sal_Int32 nFilterBySheet ) const
{
    orRanges.RemoveAll();
    TokenToRangeListState eState = STATE_OPEN;
    sal_Int32 nParenLevel = 0;
    for( ApiTokenIterator aIt( rTokens, OPCODE_SPACES ); aIt.is() && (eState != STATE_ERROR); ++aIt )
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
                     if( nOpCode == OPCODE_PUSH )  eState = lclProcessRef( orRanges, aIt->Data, nFilterBySheet );
                else if( nOpCode == OPCODE_SEP )   eState = STATE_SEP;
                else if( nOpCode == OPCODE_LIST )  eState = STATE_SEP;
                else if( nOpCode == OPCODE_OPEN )  eState = lclProcessOpen( nParenLevel );
                else if( nOpCode == OPCODE_CLOSE ) eState = lclProcessClose( nParenLevel );
                else                               eState = STATE_ERROR;
            break;
            case STATE_OPEN:
                     if( nOpCode == OPCODE_PUSH )  eState = lclProcessRef( orRanges, aIt->Data, nFilterBySheet );
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
        orRanges.RemoveAll();
    else
        getAddressConverter().validateCellRangeList( orRanges, false );
}

bool FormulaProcessorBase::extractString( OUString& orString, const ApiTokenSequence& rTokens ) const
{
    ApiTokenIterator aTokenIt( rTokens, OPCODE_SPACES );
    return aTokenIt.is() && (aTokenIt->OpCode == OPCODE_PUSH) && (aTokenIt->Data >>= orString) && !(++aTokenIt).is();
}

bool FormulaProcessorBase::extractSpecialTokenInfo( ApiSpecialTokenInfo& orTokenInfo, const ApiTokenSequence& rTokens ) const
{
    ApiTokenIterator aTokenIt( rTokens, OPCODE_SPACES );
    return aTokenIt.is() && (aTokenIt->OpCode == OPCODE_BAD) && (aTokenIt->Data >>= orTokenInfo);
}

void FormulaProcessorBase::convertStringToStringList(
        ApiTokenSequence& orTokens, sal_Unicode cStringSep, bool bTrimLeadingSpaces ) const
{
    OUString aString;
    if( extractString( aString, orTokens ) && !aString.isEmpty() )
    {
        ::std::vector< ApiToken > aNewTokens;
        for( sal_Int32 nPos{ 0 }; nPos>=0; )
        {
            OUString aEntry = aString.getToken( 0, cStringSep, nPos );
            if( bTrimLeadingSpaces )
            {
                sal_Int32 nStart = 0;
                while( (nStart < aEntry.getLength()) && (aEntry[ nStart ] == ' ') ) ++nStart;
                aEntry = aEntry.copy( nStart );
            }
            if( !aNewTokens.empty() )
                aNewTokens.emplace_back( OPCODE_SEP, Any() );
            aNewTokens.emplace_back( OPCODE_PUSH, Any( aEntry ) );
        }
        orTokens = ContainerHelper::vectorToSequence( aNewTokens );
    }
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
