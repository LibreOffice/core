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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_FORMULABASE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_FORMULABASE_HXX

#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/helper/refvector.hxx>
#include "addressconverter.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace sheet { class XFormulaParser; }
} } }

namespace oox { template< typename Type > class Matrix; }

namespace oox {
namespace xls {

// Constants ==================================================================

const size_t BIFF_TOKARR_MAXLEN                 = 4096;     /// Maximum size of a token array.

// token class flags ----------------------------------------------------------

const sal_uInt8 BIFF_TOKCLASS_MASK              = 0x60;
const sal_uInt8 BIFF_TOKCLASS_NONE              = 0x00;     /// 00-1F: Base tokens.
const sal_uInt8 BIFF_TOKCLASS_REF               = 0x20;     /// 20-3F: Reference class tokens.
const sal_uInt8 BIFF_TOKCLASS_VAL               = 0x40;     /// 40-5F: Value class tokens.
const sal_uInt8 BIFF_TOKCLASS_ARR               = 0x60;     /// 60-7F: Array class tokens.

const sal_uInt8 BIFF_TOKFLAG_INVALID            = 0x80;     /// This bit must be null for a valid token identifier.

// base token identifiers -----------------------------------------------------

const sal_uInt8 BIFF_TOKID_MASK                 = 0x1F;

const sal_uInt8 BIFF_TOKID_NONE                 = 0x00;     /// Placeholder for invalid token id.
const sal_uInt8 BIFF_TOKID_EXP                  = 0x01;     /// Array or shared formula reference.
const sal_uInt8 BIFF_TOKID_TBL                  = 0x02;     /// Multiple operation reference.
const sal_uInt8 BIFF_TOKID_ADD                  = 0x03;     /// Addition operator.
const sal_uInt8 BIFF_TOKID_SUB                  = 0x04;     /// Subtraction operator.
const sal_uInt8 BIFF_TOKID_MUL                  = 0x05;     /// Multiplication operator.
const sal_uInt8 BIFF_TOKID_DIV                  = 0x06;     /// Division operator.
const sal_uInt8 BIFF_TOKID_POWER                = 0x07;     /// Power operator.
const sal_uInt8 BIFF_TOKID_CONCAT               = 0x08;     /// String concatenation operator.
const sal_uInt8 BIFF_TOKID_LT                   = 0x09;     /// Less than operator.
const sal_uInt8 BIFF_TOKID_LE                   = 0x0A;     /// Less than or equal operator.
const sal_uInt8 BIFF_TOKID_EQ                   = 0x0B;     /// Equal operator.
const sal_uInt8 BIFF_TOKID_GE                   = 0x0C;     /// Greater than or equal operator.
const sal_uInt8 BIFF_TOKID_GT                   = 0x0D;     /// Greater than operator.
const sal_uInt8 BIFF_TOKID_NE                   = 0x0E;     /// Not equal operator.
const sal_uInt8 BIFF_TOKID_ISECT                = 0x0F;     /// Intersection operator.
const sal_uInt8 BIFF_TOKID_LIST                 = 0x10;     /// List operator.
const sal_uInt8 BIFF_TOKID_RANGE                = 0x11;     /// Range operator.
const sal_uInt8 BIFF_TOKID_UPLUS                = 0x12;     /// Unary plus.
const sal_uInt8 BIFF_TOKID_UMINUS               = 0x13;     /// Unary minus.
const sal_uInt8 BIFF_TOKID_PERCENT              = 0x14;     /// Percent sign.
const sal_uInt8 BIFF_TOKID_PAREN                = 0x15;     /// Parentheses.
const sal_uInt8 BIFF_TOKID_MISSARG              = 0x16;     /// Missing argument.
const sal_uInt8 BIFF_TOKID_STR                  = 0x17;     /// String constant.
const sal_uInt8 BIFF_TOKID_NLR                  = 0x18;     /// Natural language reference (NLR).
const sal_uInt8 BIFF_TOKID_ATTR                 = 0x19;     /// Special attribute.
const sal_uInt8 BIFF_TOKID_SHEET                = 0x1A;     /// Start of a sheet reference (BIFF2-BIFF4).
const sal_uInt8 BIFF_TOKID_ENDSHEET             = 0x1B;     /// End of a sheet reference (BIFF2-BIFF4).
const sal_uInt8 BIFF_TOKID_ERR                  = 0x1C;     /// Error constant.
const sal_uInt8 BIFF_TOKID_BOOL                 = 0x1D;     /// Boolean constant.
const sal_uInt8 BIFF_TOKID_INT                  = 0x1E;     /// Integer constant.
const sal_uInt8 BIFF_TOKID_NUM                  = 0x1F;     /// Floating-point constant.

// base identifiers of classified tokens --------------------------------------

const sal_uInt8 BIFF_TOKID_ARRAY                = 0x00;     /// Array constant.
const sal_uInt8 BIFF_TOKID_FUNC                 = 0x01;     /// Function, fixed number of arguments.
const sal_uInt8 BIFF_TOKID_FUNCVAR              = 0x02;     /// Function, variable number of arguments.
const sal_uInt8 BIFF_TOKID_NAME                 = 0x03;     /// Defined name.
const sal_uInt8 BIFF_TOKID_REF                  = 0x04;     /// 2D cell reference.
const sal_uInt8 BIFF_TOKID_AREA                 = 0x05;     /// 2D area reference.
const sal_uInt8 BIFF_TOKID_MEMAREA              = 0x06;     /// Constant reference subexpression.
const sal_uInt8 BIFF_TOKID_MEMERR               = 0x07;     /// Deleted reference subexpression.
const sal_uInt8 BIFF_TOKID_MEMNOMEM             = 0x08;     /// Constant reference subexpression without result.
const sal_uInt8 BIFF_TOKID_MEMFUNC              = 0x09;     /// Variable reference subexpression.
const sal_uInt8 BIFF_TOKID_REFERR               = 0x0A;     /// Deleted 2D cell reference.
const sal_uInt8 BIFF_TOKID_AREAERR              = 0x0B;     /// Deleted 2D area reference.
const sal_uInt8 BIFF_TOKID_REFN                 = 0x0C;     /// Relative 2D cell reference (in names).
const sal_uInt8 BIFF_TOKID_AREAN                = 0x0D;     /// Relative 2D area reference (in names).
const sal_uInt8 BIFF_TOKID_MEMAREAN             = 0x0E;     /// Reference subexpression (in names).
const sal_uInt8 BIFF_TOKID_MEMNOMEMN            = 0x0F;     /// Reference subexpression (in names) without result.
const sal_uInt8 BIFF_TOKID_FUNCCE               = 0x18;
const sal_uInt8 BIFF_TOKID_NAMEX                = 0x19;     /// External reference.
const sal_uInt8 BIFF_TOKID_REF3D                = 0x1A;     /// 3D cell reference.
const sal_uInt8 BIFF_TOKID_AREA3D               = 0x1B;     /// 3D area reference.
const sal_uInt8 BIFF_TOKID_REFERR3D             = 0x1C;     /// Deleted 3D cell reference.
const sal_uInt8 BIFF_TOKID_AREAERR3D            = 0x1D;     /// Deleted 3D area reference

// specific token constants ---------------------------------------------------

const sal_uInt8 BIFF_TOK_ARRAY_DOUBLE           = 0;        /// Double value in an array.
const sal_uInt8 BIFF_TOK_ARRAY_STRING           = 1;        /// String value in an array.
const sal_uInt8 BIFF_TOK_ARRAY_BOOL             = 2;        /// Boolean value in an array.
const sal_uInt8 BIFF_TOK_ARRAY_ERROR            = 4;        /// Error code in an array.

const sal_uInt8 BIFF_TOK_BOOL_FALSE             = 0;        /// FALSE value of a tBool token.
const sal_uInt8 BIFF_TOK_BOOL_TRUE              = 1;        /// TRUE value of a tBool token.

const sal_uInt8 BIFF_TOK_ATTR_VOLATILE          = 0x01;     /// Volatile function.
const sal_uInt8 BIFF_TOK_ATTR_IF                = 0x02;     /// Start of true condition in IF function.
const sal_uInt8 BIFF_TOK_ATTR_CHOOSE            = 0x04;     /// Jump array of CHOOSE function.
const sal_uInt8 BIFF_TOK_ATTR_SKIP              = 0x08;     /// Skip tokens.
const sal_uInt8 BIFF_TOK_ATTR_SUM               = 0x10;     /// SUM function with one parameter.
const sal_uInt8 BIFF_TOK_ATTR_ASSIGN            = 0x20;     /// BASIC style assignment.
const sal_uInt8 BIFF_TOK_ATTR_SPACE             = 0x40;     /// Spaces in formula representation.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_VOLATILE    = 0x41;     /// Leading spaces and volatile formula.
const sal_uInt8 BIFF_TOK_ATTR_IFERROR           = 0x80;     /// Start of condition in IFERROR function (BIFF12 only).

const sal_uInt8 BIFF_TOK_ATTR_SPACE_SP          = 0x00;     /// Spaces before next token.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_BR          = 0x01;     /// Line breaks before next token.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_SP_OPEN     = 0x02;     /// Spaces before opening parenthesis.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_BR_OPEN     = 0x03;     /// Line breaks before opening parenthesis.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_SP_CLOSE    = 0x04;     /// Spaces before closing parenthesis.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_BR_CLOSE    = 0x05;     /// Line breaks before closing parenthesis.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_SP_PRE      = 0x06;     /// Spaces before formula (BIFF3).

const sal_uInt16 BIFF_TOK_FUNCVAR_CMD           = 0x8000;   /// Macro command.
const sal_uInt16 BIFF_TOK_FUNCVAR_FUNCIDMASK    = 0x7FFF;   /// Mask for function/command index.
const sal_uInt8 BIFF_TOK_FUNCVAR_CMDPROMPT      = 0x80;     /// User prompt for macro commands.
const sal_uInt8 BIFF_TOK_FUNCVAR_COUNTMASK      = 0x7F;     /// Mask for parameter count.

const sal_uInt16 BIFF12_TOK_REF_COLMASK         = 0x3FFF;   /// Mask to extract column from reference (BIFF12).
const sal_Int32 BIFF12_TOK_REF_ROWMASK          = 0xFFFFF;  /// Mask to extract row from reference (BIFF12).
const sal_uInt16 BIFF12_TOK_REF_COLREL          = 0x4000;   /// True = column is relative (BIFF12).
const sal_uInt16 BIFF12_TOK_REF_ROWREL          = 0x8000;   /// True = row is relative (BIFF12).

const sal_uInt16 BIFF_TOK_REF_COLMASK           = 0x00FF;   /// Mask to extract BIFF8 column from reference.
const sal_uInt16 BIFF_TOK_REF_ROWMASK           = 0x3FFF;   /// Mask to extract BIFF2-BIFF5 row from reference.
const sal_uInt16 BIFF_TOK_REF_COLREL            = 0x4000;   /// True = column is relative.
const sal_uInt16 BIFF_TOK_REF_ROWREL            = 0x8000;   /// True = row is relative.

const sal_uInt16 BIFF12_TOK_TABLE_COLUMN         = 0x0001;   /// Table reference: Single column.
const sal_uInt16 BIFF12_TOK_TABLE_COLRANGE       = 0x0002;   /// Table reference: Range of columns.
const sal_uInt16 BIFF12_TOK_TABLE_ALL            = 0x0004;   /// Table reference: Special [#All] range.
const sal_uInt16 BIFF12_TOK_TABLE_HEADERS        = 0x0008;   /// Table reference: Special [#Headers] range.
const sal_uInt16 BIFF12_TOK_TABLE_DATA           = 0x0010;   /// Table reference: Special [#Data] range.
const sal_uInt16 BIFF12_TOK_TABLE_TOTALS         = 0x0020;   /// Table reference: Special [#Totals] range.
const sal_uInt16 BIFF12_TOK_TABLE_THISROW        = 0x0040;   /// Table reference: Special [#This Row] range.
const sal_uInt16 BIFF12_TOK_TABLE_SP_BRACKETS    = 0x0080;   /// Table reference: Spaces in outer brackets.
const sal_uInt16 BIFF12_TOK_TABLE_SP_SEP         = 0x0100;   /// Table reference: Spaces after separators.
const sal_uInt16 BIFF12_TOK_TABLE_ROW            = 0x0200;   /// Table reference: Single row.
const sal_uInt16 BIFF12_TOK_TABLE_CELL           = 0x0400;   /// Table reference: Single cell.

const sal_uInt8 BIFF_TOK_NLR_ERR                = 0x01;     /// NLR: Invalid/deleted.
const sal_uInt8 BIFF_TOK_NLR_ROWR               = 0x02;     /// NLR: Row index.
const sal_uInt8 BIFF_TOK_NLR_COLR               = 0x03;     /// NLR: Column index.
const sal_uInt8 BIFF_TOK_NLR_ROWV               = 0x06;     /// NLR: Value in row.
const sal_uInt8 BIFF_TOK_NLR_COLV               = 0x07;     /// NLR: Value in column.
const sal_uInt8 BIFF_TOK_NLR_RANGE              = 0x0A;     /// NLR: Range.
const sal_uInt8 BIFF_TOK_NLR_SRANGE             = 0x0B;     /// Stacked NLR: Range.
const sal_uInt8 BIFF_TOK_NLR_SROWR              = 0x0C;     /// Stacked NLR: Row index.
const sal_uInt8 BIFF_TOK_NLR_SCOLR              = 0x0D;     /// Stacked NLR: Column index.
const sal_uInt8 BIFF_TOK_NLR_SROWV              = 0x0E;     /// Stacked NLR: Value in row.
const sal_uInt8 BIFF_TOK_NLR_SCOLV              = 0x0F;     /// Stacked NLR: Value in column.
const sal_uInt8 BIFF_TOK_NLR_RANGEERR           = 0x10;     /// NLR: Invalid/deleted range.
const sal_uInt8 BIFF_TOK_NLR_SXNAME             = 0x1D;     /// NLR: Pivot table name.
const sal_uInt16 BIFF_TOK_NLR_REL               = 0x8000;   /// True = NLR is relative.
const sal_uInt16 BIFF_TOK_NLR_MASK              = 0x3FFF;   /// Mask to extract BIFF8 column from NLR.

const sal_uInt32 BIFF_TOK_NLR_ADDREL            = 0x80000000;   /// NLR relative (in appended data).
const sal_uInt32 BIFF_TOK_NLR_ADDMASK           = 0x3FFFFFFF;   /// Mask for number of appended ranges.

// function constants ---------------------------------------------------------

const sal_uInt8 OOX_MAX_PARAMCOUNT              = 255;      /// Maximum parameter count for OOXML/BIFF12 files.
const sal_uInt8 BIFF_MAX_PARAMCOUNT             = 30;       /// Maximum parameter count for BIFF2-BIFF8 files.

const sal_uInt16 BIFF_FUNC_IF                   = 1;        /// Function identifier of the IF function.
const sal_uInt16 BIFF_FUNC_SUM                  = 4;        /// Function identifier of the SUM function.
const sal_uInt16 BIFF_FUNC_TRUE                 = 34;       /// Function identifier of the TRUE function.
const sal_uInt16 BIFF_FUNC_FALSE                = 35;       /// Function identifier of the FALSE function.
const sal_uInt16 BIFF_FUNC_ROWS                 = 76;       /// Function identifier of the ROWS function.
const sal_uInt16 BIFF_FUNC_COLUMNS              = 77;       /// Function identifier of the COLUMNS function.
const sal_uInt16 BIFF_FUNC_OFFSET               = 78;       /// Function identifier of the OFFSET function.
const sal_uInt16 BIFF_FUNC_EXTERNCALL           = 255;      /// BIFF function id of the EXTERN.CALL function.
const sal_uInt16 BIFF_FUNC_FLOOR                = 285;      /// Function identifier of the FLOOR function.
const sal_uInt16 BIFF_FUNC_CEILING              = 288;      /// Function identifier of the CEILING function.
const sal_uInt16 BIFF_FUNC_HYPERLINK            = 359;      /// Function identifier of the HYPERLINK function.
const sal_uInt16 BIFF_FUNC_WEEKNUM              = 465;      /// Function identifier of the WEEKNUM function.

// Formula type ===============================================================

/** Enumerates all possible types of a formula. */
enum class FormulaType
{
    Cell,           /// Simple cell formula, or reference to a shared formula name.
    Array,          /// Array (matrix) formula.
    SharedFormula,  /// Shared formula definition.
    CondFormat,     /// Condition of a conditional format rule.
    Validation      /// Condition of a data validation.
};

// Reference helpers ==========================================================

/** A 2D formula cell reference struct with relative flags. */
struct BinSingleRef2d
{
    sal_Int32           mnCol;              /// Column index.
    sal_Int32           mnRow;              /// Row index.
    bool                mbColRel;           /// True = relative column reference.
    bool                mbRowRel;           /// True = relative row reference.

    explicit            BinSingleRef2d();

    void                setBiff12Data( sal_uInt16 nCol, sal_Int32 nRow, bool bRelativeAsOffset );

    void                readBiff12Data( SequenceInputStream& rStrm, bool bRelativeAsOffset );
};

/** A 2D formula cell range reference struct with relative flags. */
struct BinComplexRef2d
{
    BinSingleRef2d      maRef1;             /// Start (top-left) cell address.
    BinSingleRef2d      maRef2;             /// End (bottom-right) cell address.

    void                readBiff12Data( SequenceInputStream& rStrm, bool bRelativeAsOffset );
};

// Token vector, token sequence ===============================================

typedef css::sheet::FormulaToken       ApiToken;
typedef css::uno::Sequence< ApiToken > ApiTokenSequence;

/** Contains the base address and type of a special token representing an array
    formula or a shared formula (sal_False), or a table operation (sal_True). */
typedef css::beans::Pair< css::table::CellAddress, sal_Bool > ApiSpecialTokenInfo;

/** A vector of formula tokens with additional convenience functions. */
class ApiTokenVector
{
public:
    explicit            ApiTokenVector();

    ApiToken& operator[]( size_t i ) { return mvTokens[i]; }

    size_t size() const { return mvTokens.size(); }

    ApiToken& back() { return mvTokens.back(); }
    const ApiToken& back() const { return mvTokens.back(); }

    void clear() { mvTokens.clear(); }

    void pop_back() { mvTokens.pop_back(); }

    void push_back( const ApiToken& rToken ) { mvTokens.push_back( rToken ); }

    void reserve( size_t n ) { mvTokens.reserve( n ); }

    void resize( size_t n ) { mvTokens.resize( n ); }

    /** Appends a new token with the passed op-code, returns its data field. */
    css::uno::Any&      append( sal_Int32 nOpCode );

    /** Appends a new token with the passed op-code and data. */
    template< typename Type >
    void         append( sal_Int32 nOpCode, const Type& rData ) { append( nOpCode ) <<= rData; }

    /** Converts to a sequence. */
    ApiTokenSequence toSequence() const;

private:
    ::std::vector< ApiToken > mvTokens;
};

// Token sequence iterator ====================================================

/** Token sequence iterator that is able to skip space tokens. */
class ApiTokenIterator
{
public:
    explicit            ApiTokenIterator( const ApiTokenSequence& rTokens, sal_Int32 nSpacesOpCode, bool bSkipSpaces );
    bool         is() const { return mpToken != mpTokenEnd; }
    const ApiToken* operator->() const { return mpToken; }

    ApiTokenIterator&   operator++();

private:
    void                skipSpaces();

private:
    const ApiToken*     mpToken;            /// Pointer to current token of the token sequence.
    const ApiToken*     mpTokenEnd;         /// Pointer behind last token of the token sequence.
    const sal_Int32     mnSpacesOpCode;     /// Op-code for whitespace tokens.
    const bool          mbSkipSpaces;       /// true = Skip whitespace tokens.
};

// List of API op-codes =======================================================

/** Contains all API op-codes needed to build formulas with tokens. */
struct ApiOpCodes
{
    // special
    sal_Int32           OPCODE_UNKNOWN;         /// Internal: function name unknown to mapper.
    sal_Int32           OPCODE_EXTERNAL;        /// External function call (e.g. add-ins).
    // formula structure
    sal_Int32           OPCODE_PUSH;            /// Op-code for common value operands.
    sal_Int32           OPCODE_MISSING;         /// Placeholder for a missing function parameter.
    sal_Int32           OPCODE_SPACES;          /// Spaces between other formula tokens.
    sal_Int32           OPCODE_NAME;            /// Index of a defined name.
    sal_Int32           OPCODE_DBAREA;          /// Index of a database area.
    sal_Int32           OPCODE_NLR;             /// Natural language reference.
    sal_Int32           OPCODE_DDE;             /// DDE link function.
    sal_Int32           OPCODE_MACRO;           /// Macro function call.
    sal_Int32           OPCODE_BAD;             /// Bad token (unknown name, formula error).
    sal_Int32           OPCODE_NONAME;          /// Function style #NAME? error.
    // separators
    sal_Int32           OPCODE_OPEN;            /// Opening parenthesis.
    sal_Int32           OPCODE_CLOSE;           /// Closing parenthesis.
    sal_Int32           OPCODE_SEP;             /// Function parameter separator.
    // array separators
    sal_Int32           OPCODE_ARRAY_OPEN;      /// Opening brace for constant arrays.
    sal_Int32           OPCODE_ARRAY_CLOSE;     /// Closing brace for constant arrays.
    sal_Int32           OPCODE_ARRAY_ROWSEP;    /// Row separator in constant arrays.
    sal_Int32           OPCODE_ARRAY_COLSEP;    /// Column separator in constant arrays.
    // unary operators
    sal_Int32           OPCODE_PLUS_SIGN;       /// Unary plus sign.
    sal_Int32           OPCODE_MINUS_SIGN;      /// Unary minus sign.
    sal_Int32           OPCODE_PERCENT;         /// Percent sign.
    // binary operators
    sal_Int32           OPCODE_ADD;             /// Addition operator.
    sal_Int32           OPCODE_SUB;             /// Subtraction operator.
    sal_Int32           OPCODE_MULT;            /// Multiplication operator.
    sal_Int32           OPCODE_DIV;             /// Division operator.
    sal_Int32           OPCODE_POWER;           /// Power operator.
    sal_Int32           OPCODE_CONCAT;          /// String concatenation operator.
    sal_Int32           OPCODE_EQUAL;           /// Compare equal operator.
    sal_Int32           OPCODE_NOT_EQUAL;       /// Compare not equal operator.
    sal_Int32           OPCODE_LESS;            /// Compare less operator.
    sal_Int32           OPCODE_LESS_EQUAL;      /// Compare less or equal operator.
    sal_Int32           OPCODE_GREATER;         /// Compare greater operator.
    sal_Int32           OPCODE_GREATER_EQUAL;   /// Compare greater or equal operator.
    sal_Int32           OPCODE_INTERSECT;       /// Range intersection operator.
    sal_Int32           OPCODE_LIST;            /// Range list operator.
    sal_Int32           OPCODE_RANGE;           /// Range operator.
};

// Function parameter info ====================================================

/** Enumerates validity modes for a function parameter. */
enum class FuncParamValidity
{
    Regular,         /// Parameter supported by Calc and Excel.
    CalcOnly,        /// Parameter supported by Calc only.
    ExcelOnly        /// Parameter supported by Excel only.
};

/** Structure that contains all needed information for a parameter in a
    function.

    The member meValid specifies which application supports the parameter. If
    set to CALCONLY, import filters have to insert a default value for this
    parameter, and export filters have to skip the parameter. If set to
    EXCELONLY, import filters have to skip the parameter, and export filters
    have to insert a default value for this parameter.

    The member mbValType specifies whether the parameter requires tokens to be
    of value type (VAL or ARR class).

        If set to false, the parameter is called to be REFTYPE. Tokens with REF
        default class can be inserted for the parameter (e.g. tAreaR tokens).

        If set to true, the parameter is called to be VALTYPE. Tokens with REF
        class need to be converted to VAL tokens first (e.g. tAreaR will be
        converted to tAreaV), and further conversion is done according to this
        new token class.

    The member meConv specifies how to convert the current token class of the
    token inserted for the parameter. If the token class is still REF this
    means that the token has default REF class and the parameter is REFTYPE
    (see member mbValType), the token will not be converted at all and remains
    in REF class. Otherwise, token class conversion is depending on the actual
    token class of the return value of the function containing this parameter.
    The function may return REF class (tFuncR, tFuncVarR, tFuncCER), or it may
    return VAL or ARR class (tFuncV, tFuncA, tFuncVarV, tFuncVarA, tFuncCEV,
    tFuncCEA). Even if the function is able to return REF class, it may return
    VAL or ARR class instead due to the VALTYPE data type of the parent
    function parameter that calls the own function. Example: The INDIRECT
    function returns REF class by default. But if called from a VALTYPE
    function parameter, e.g. in the formula =ABS(INDIRECT("A1")), it returns
    VAL or ARR class instead. Additionally, the repeating conversion types RPT
    and RPX rely on the conversion executed for the function token class.

        1) ORG:
        Use the original class of the token (VAL or ARR), regardless of any
        conversion done for the function return class.

        2) VAL:
        Convert ARR tokens to VAL class, regardless of any conversion done for
        the function return class.

        3) ARR:
        Convert VAL tokens to ARR class, regardless of any conversion done for
        the function return class.

        4) RPT:
        If the own function returns REF class (thus it is called from a REFTYPE
        parameter, see above), and the parent conversion type (for the function
        return class) was ORG, VAL, or ARR, ignore that conversion and always
        use VAL conversion for the own token instead. If the parent conversion
        type was RPT or RPX, repeat the conversion that would have been used if
        the function would return value type.
        If the own function returns value type (VAL or ARR class, see above),
        and the parent conversion type (for the function return class) was ORG,
        VAL, ARR, or RPT, repeat this conversion for the own token. If the
        parent conversion type was RPX, always use ORG conversion type for the
        own token instead.

        5) RPX:
        This type of conversion only occurs in functions returning VAL class by
        default. If the own token is value type, and the VAL return class of
        the own function has been changed to ARR class (due to direct ARR
        conversion, or due to ARR conversion repeated by RPT or RPX), set the
        own token to ARR type. Otherwise use the original token type (VAL
        conversion from parent parameter will not be repeated at all). If
        nested functions have RPT or value-type RPX parameters, they will not
        repeat this conversion type, but will use ORG conversion instead (see
        description of RPT above).

        6) RPO:
        This type of conversion is only used for the operands of all operators
        (unary and binary arithmetic operators, comparison operators, and range
        operators). It is not used for function parameters. On conversion, it
        will be replaced by the last conversion type that was not the RPO
        conversion. This leads to a slightly different behaviour than the RPT
        conversion for operands in conjunction with a parent RPX conversion.
 */
struct FunctionParamInfo
{
    FuncParamValidity   meValid;        /// Parameter validity.
};

// Function data ==============================================================

/** This enumeration contains constants for all known external libraries
    containing supported sheet functions. */
enum FunctionLibraryType
{
    FUNCLIB_UNKNOWN = 0,        /// Unknown library (must be zero).
    FUNCLIB_EUROTOOL            /// EuroTool add-in with EUROCONVERT function.
};

/** Represents information for a spreadsheet function.

    The member mpParamInfos points to a C-array of type information structures
    for all parameters of the function. The last initialized structure
    describing a regular parameter (member meValid == FuncParamValidity::Regular) in
    this array is used repeatedly for all following parameters supported by a
    function.
 */
struct FunctionInfo
{
    OUString     maOdfFuncName;      /// ODF function name.
    OUString     maOoxFuncName;      /// OOXML function name.
    OUString     maBiffMacroName;    /// Expected macro name in EXTERN.CALL function.
    OUString     maExtProgName;      /// Programmatic function name for external functions.
    FunctionLibraryType meFuncLibType;      /// The external library this function is part of.
    sal_Int32           mnApiOpCode;        /// API function opcode.
    sal_uInt16          mnBiff12FuncId;     /// BIFF12 function identifier.
    sal_uInt16          mnBiffFuncId;       /// BIFF2-BIFF8 function identifier.
    sal_uInt8           mnMinParamCount;    /// Minimum number of parameters.
    sal_uInt8           mnMaxParamCount;    /// Maximum number of parameters.
    sal_uInt8           mnRetClass;         /// BIFF token class of the return value.
    const FunctionParamInfo* mpParamInfos;  /// Information about all parameters.
    bool                mbParamPairs;       /// True = optional parameters are expected to appear in pairs.
    bool                mbVolatile;         /// True = volatile function.
    bool                mbExternal;         /// True = external function in Calc.
    bool                mbInternal;         /// True = internal function in Calc. (Both can be true!)
    bool                mbMacroFunc;        /// True = macro sheet function or command.
    bool                mbVarParam;         /// True = use a tFuncVar token, also if min/max are equal.
};

typedef RefVector< FunctionInfo > FunctionInfoVector;

// Function info parameter class iterator =====================================

/** Iterator working on the mpParamInfos member of the FunctionInfo struct.

    This iterator can be used to iterate through the array containing the
    token class conversion information of function parameters. This iterator
    repeats the last valid structure in the array - it stops automatically
    before the first empty array entry or before the end of the array, even for
    repeated calls to the increment operator.
 */
class FunctionParamInfoIterator
{
public:
    explicit            FunctionParamInfoIterator( const FunctionInfo& rFuncInfo );

    bool                isCalcOnlyParam() const;
    bool                isExcelOnlyParam() const;
    FunctionParamInfoIterator& operator++();

private:
    const FunctionParamInfo* mpParamInfo;
    const FunctionParamInfo* mpParamInfoEnd;
    bool                mbParamPairs;
};

// Base function provider =====================================================

struct FunctionProviderImpl;

/** Provides access to function info structs for all available sheet functions.
 */
class FunctionProvider  // not derived from WorkbookHelper to make it usable in file dumpers
{
public:
    explicit            FunctionProvider(bool bImportFilter);
    virtual             ~FunctionProvider();

    FunctionProvider(FunctionProvider const &) = default;
    FunctionProvider(FunctionProvider &&) = default;
    FunctionProvider & operator =(FunctionProvider const &) = default;
    FunctionProvider & operator =(FunctionProvider &&) = default;

    /** Returns the function info for an OOXML function name, or 0 on error. */
    const FunctionInfo* getFuncInfoFromOoxFuncName( const OUString& rFuncName ) const;

    /** Returns the function info for a BIFF12 function index, or 0 on error. */
    const FunctionInfo* getFuncInfoFromBiff12FuncId( sal_uInt16 nFuncId ) const;

    /** Returns the function info for a macro function referred by the
        EXTERN.CALL function, or 0 on error. */
    const FunctionInfo* getFuncInfoFromMacroName( const OUString& rFuncName ) const;

    /** Returns the library type associated with the passed URL of a function
        library (function add-in). */
    static FunctionLibraryType getFuncLibTypeFromLibraryName( const OUString& rLibraryName );

protected:
    /** Returns the list of all function infos. */
    const FunctionInfoVector& getFuncs() const;

private:
    typedef std::shared_ptr< FunctionProviderImpl > FunctionProviderImplRef;
    FunctionProviderImplRef mxFuncImpl;     /// Shared implementation between all copies of the provider.
};

// Op-code and function provider ==============================================

struct OpCodeProviderImpl;

/** Provides access to API op-codes for all available formula tokens and to
    function info structs for all available sheet functions.
 */
class OpCodeProvider : public FunctionProvider // not derived from WorkbookHelper to make it usable as UNO service
{
public:
    explicit            OpCodeProvider(const css::uno::Reference<css::lang::XMultiServiceFactory>& rxModelFactory,
                                       bool bImportFilter);
    virtual             ~OpCodeProvider() override;

    OpCodeProvider(OpCodeProvider const &) = default;
    OpCodeProvider(OpCodeProvider &&) = default;
    OpCodeProvider & operator =(OpCodeProvider const &) = default;
    OpCodeProvider & operator =(OpCodeProvider &&) = default;

    /** Returns the structure containing all token op-codes for operators and
        special tokens used by the Calc document and its formula parser. */
    const ApiOpCodes&   getOpCodes() const;

    /** Returns the function info for an API token, or 0 on error. */
    const FunctionInfo* getFuncInfoFromApiToken( const ApiToken& rToken ) const;

    /** Returns the op-code map that is used by the OOXML formula parser. */
    css::uno::Sequence< css::sheet::FormulaOpCodeMapEntry >
                        getOoxParserMap() const;

private:
    typedef std::shared_ptr< OpCodeProviderImpl > OpCodeProviderImplRef;
    OpCodeProviderImplRef mxOpCodeImpl;     /// Shared implementation between all copies of the provider.
};

// API formula parser wrapper =================================================

/** A wrapper around the FormulaParser service provided by the Calc document. */
class ApiParserWrapper : public OpCodeProvider
{
public:
    explicit            ApiParserWrapper(
                            const css::uno::Reference< css::lang::XMultiServiceFactory >& rxModelFactory,
                            const OpCodeProvider& rOpCodeProv );

    /** Returns read/write access to the formula parser property set. */
    PropertySet& getParserProperties() { return maParserProps; }

    /** Calls the XFormulaParser::parseFormula() function of the API parser. */
    ApiTokenSequence    parseFormula(
                            const OUString& rFormula,
                            const ScAddress& rRefPos );

private:
    css::uno::Reference< css::sheet::XFormulaParser >
                        mxParser;
    PropertySet         maParserProps;
};

// Formula parser/printer base class for filters ==============================

/** Base class for import formula parsers and export formula compilers. */
class FormulaProcessorBase : public OpCodeProvider, protected ApiOpCodes, public WorkbookHelper
{
public:
    explicit            FormulaProcessorBase( const WorkbookHelper& rHelper );

    /** Generates a cell address string in A1 notation from the passed cell
        address.

        @param rAddress  The cell address containing column and row index.
        @param bAbsolute  True = adds dollar signs before column and row.
     */
    static OUString generateAddress2dString(
                            const ScAddress& rAddress,
                            bool bAbsolute );

    /** Generates a cell address string in A1 notation from the passed binary
        cell address.

        @param rAddress  The cell address containing column and row index.
        @param bAbsolute  True = adds dollar signs before column and row.
     */
    static OUString generateAddress2dString(
                            const BinAddress& rAddress,
                            bool bAbsolute );

    /** Generates a string in Calc formula notation from the passed string.

        @param rString  The string value.

        @return  The string enclosed in double quotes, where all contained
            quote characters are doubled.
     */
    static OUString generateApiString( const OUString& rString );

    /** Generates an array string in Calc formula notation from the passed
        matrix with Any's containing double values or strings.

        @param rMatrix  The matrix containing double values or strings.
     */
    static OUString generateApiArray( const Matrix< css::uno::Any >& rMatrix );

    /** Tries to extract a single cell reference from a formula token sequence.

        @param rTokens  The token sequence to be parsed. Should contain exactly
            one address token or cell range address token. The token sequence
            may contain whitespace tokens.

        @return  If the token sequence is valid, this function returns an Any
            containing a com.sun.star.sheet.SingleReference object, or a
            com.sun.star.sheet.ComplexReference object. If the token sequence
            contains too many, or unexpected tokens, an empty Any is returned.
     */
    css::uno::Any
                        extractReference( const ApiTokenSequence& rTokens ) const;

    /** Tries to extract a cell range address from a formula token sequence.
        Only real absolute references will be accepted.

        @param orAddress  (output parameter) If the token sequence is valid,
            this parameter will contain the extracted cell range address. If
            the token sequence contains unexpected tokens, nothing meaningful
            is inserted, and the function returns false.

        @param rTokens  The token sequence to be parsed. Should contain exactly
            one cell range address token. The token sequence may contain
            whitespace tokens.

        @return  True, if the token sequence contains a valid cell range
            address which has been extracted to orRange, false otherwise.
     */
    bool                extractCellRange(
                            ScRange& orRange,
                            const ApiTokenSequence& rTokens ) const;

    /** Tries to extract a cell range list from a formula token sequence.
        Only real absolute references will be accepted.

        @param orRanges  (output parameter) If the token sequence is valid,
            this parameter will contain the extracted cell range list. Deleted
            cells or cell ranges (shown as #REF! error in a formula) will be
            skipped. If the token sequence contains unexpected tokens, an empty
            list is returned here.

        @param rTokens  The token sequence to be parsed. Should contain cell
            address tokens or cell range address tokens, separated by the
            standard function parameter separator token. The token sequence may
            contain parentheses and whitespace tokens.

        @param nFilterBySheet  If non-negative, this function returns only cell
            ranges located in the specified sheet, otherwise returns all cell
            ranges contained in the token sequence.
     */
    void                extractCellRangeList(
                            ScRangeList& orRanges,
                            const ApiTokenSequence& rTokens,
                            sal_Int32 nFilterBySheet ) const;

    /** Tries to extract a string from a formula token sequence.

        @param orString  (output parameter) The extracted string.

        @param rTokens  The token sequence to be parsed. Should contain exactly
            one string token, may contain whitespace tokens.

        @return  True = token sequence is valid, output parameter orString
            contains the string extracted from the token sequence.
     */
    bool                extractString(
                            OUString& orString,
                            const ApiTokenSequence& rTokens ) const;

    /** Tries to extract information about a special token used for array
        formulas, shared formulas, or table operations.

        @param orTokenInfo  (output parameter) The extracted information about
            the token. Contains the base address and the token type (sal_False
            for array or shared formulas, sal_True for table operations).

        @param rTokens  The token sequence to be parsed. If it contains exactly
            one OPCODE_BAD token with special token information, this
            information will be extracted.

        @return  True = token sequence is valid, output parameter orTokenInfo
            contains the token information extracted from the token sequence.
     */
    bool                extractSpecialTokenInfo(
                            ApiSpecialTokenInfo& orTokenInfo,
                            const ApiTokenSequence& rTokens ) const;

    /** Converts a single string with separators in the passed formula token
        sequence to a list of string tokens.

        @param orTokens  (input/output parameter) Expects a single string token
            in this token sequence (whitespace tokens are allowed). The string
            is split into substrings. A list of string tokens separated with
            parameter separator tokens is returned in this parameter.

        @param cStringSep  The separator character used to split the input
            string.

        @param bTrimLeadingSpaces  True = removes leading whitespace from all
            substrings inserted into the formula token sequence.
     */
    void                convertStringToStringList(
                            ApiTokenSequence& orTokens,
                            sal_Unicode cStringSep,
                            bool bTrimLeadingSpaces ) const;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
