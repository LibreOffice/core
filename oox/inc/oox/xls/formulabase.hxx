/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formulabase.hxx,v $
 * $Revision: 1.5.20.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_XLS_FORMULABASE_HXX
#define OOX_XLS_FORMULABASE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/addressconverter.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XFormulaParser; }
    namespace sheet { class XFormulaTokens; }
    namespace sheet { class XFormulaOpCodeMapper; }
} } }

namespace oox { template< typename Type > class Matrix; }

namespace oox {
namespace xls {

// Constants ==================================================================

const size_t BIFF_TOKARR_MAXLEN                 = 4096;     /// Maximum size of a token array.

// token class flags ----------------------------------------------------------

const sal_uInt8 BIFF_TOKCLASS_MASK              = 0x60;
const sal_uInt8 BIFF_TOKID_MASK                 = 0x1F;

const sal_uInt8 BIFF_TOKCLASS_NONE              = 0x00;     /// 00-1F: Base tokens.
const sal_uInt8 BIFF_TOKCLASS_REF               = 0x20;     /// 20-3F: Reference class tokens.
const sal_uInt8 BIFF_TOKCLASS_VAL               = 0x40;     /// 40-5F: Value class tokens.
const sal_uInt8 BIFF_TOKCLASS_ARR               = 0x60;     /// 60-7F: Array class tokens.

const sal_uInt8 BIFF_TOKFLAG_INVALID            = 0x80;     /// This bit must be null for a valid token identifier.

// base token identifiers -----------------------------------------------------

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

const sal_uInt8 OOBIN_TOK_ARRAY_DOUBLE          = 0;
const sal_uInt8 OOBIN_TOK_ARRAY_STRING          = 1;
const sal_uInt8 OOBIN_TOK_ARRAY_BOOL            = 2;
const sal_uInt8 OOBIN_TOK_ARRAY_ERROR           = 4;

const sal_uInt8 BIFF_TOK_BOOL_FALSE             = 0;        /// FALSE value of a tBool token.
const sal_uInt8 BIFF_TOK_BOOL_TRUE              = 1;        /// TRUE value of a tBool token.

const sal_uInt8 OOBIN_TOK_ATTR_VOLATILE         = 0x01;     /// Volatile function.
const sal_uInt8 OOBIN_TOK_ATTR_IF               = 0x02;     /// Start of true condition in IF function.
const sal_uInt8 OOBIN_TOK_ATTR_CHOOSE           = 0x04;     /// Jump array of CHOOSE function.
const sal_uInt8 OOBIN_TOK_ATTR_SKIP             = 0x08;     /// Skip tokens.
const sal_uInt8 OOBIN_TOK_ATTR_SUM              = 0x10;     /// SUM function with one parameter.
const sal_uInt8 OOBIN_TOK_ATTR_ASSIGN           = 0x20;     /// BASIC style assignment.
const sal_uInt8 OOBIN_TOK_ATTR_SPACE            = 0x40;     /// Spaces in formula representation.
const sal_uInt8 OOBIN_TOK_ATTR_SPACE_VOLATILE   = 0x41;     /// Leading spaces and volatile formula.
const sal_uInt8 OOBIN_TOK_ATTR_IFERROR          = 0x80;     /// Start of condition in IFERROR function.

const sal_uInt8 BIFF_TOK_ATTR_VOLATILE          = 0x01;     /// Volatile function.
const sal_uInt8 BIFF_TOK_ATTR_IF                = 0x02;     /// Start of true condition in IF function.
const sal_uInt8 BIFF_TOK_ATTR_CHOOSE            = 0x04;     /// Jump array of CHOOSE function.
const sal_uInt8 BIFF_TOK_ATTR_SKIP              = 0x08;     /// Skip tokens.
const sal_uInt8 BIFF_TOK_ATTR_SUM               = 0x10;     /// SUM function with one parameter.
const sal_uInt8 BIFF_TOK_ATTR_ASSIGN            = 0x20;     /// BASIC style assignment.
const sal_uInt8 BIFF_TOK_ATTR_SPACE             = 0x40;     /// Spaces in formula representation.
const sal_uInt8 BIFF_TOK_ATTR_SPACE_VOLATILE    = 0x41;     /// Leading spaces and volatile formula.

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

const sal_uInt16 OOBIN_TOK_REF_COLMASK          = 0x3FFF;   /// Mask to extract column from reference.
const sal_Int32 OOBIN_TOK_REF_ROWMASK           = 0xFFFFF;  /// Mask to extract row from reference.
const sal_uInt16 OOBIN_TOK_REF_COLREL           = 0x4000;   /// True = column is relative.
const sal_uInt16 OOBIN_TOK_REF_ROWREL           = 0x8000;   /// True = row is relative.

const sal_uInt16 BIFF_TOK_REF_COLMASK           = 0x00FF;   /// Mask to extract BIFF8 column from reference.
const sal_uInt16 BIFF_TOK_REF_ROWMASK           = 0x3FFF;   /// Mask to extract BIFF2-BIFF5 row from reference.
const sal_uInt16 BIFF_TOK_REF_COLREL            = 0x4000;   /// True = column is relative.
const sal_uInt16 BIFF_TOK_REF_ROWREL            = 0x8000;   /// True = row is relative.

const sal_uInt16 OOBIN_TOK_TABLE_COLUMN         = 0x0001;   /// Table reference: Single column.
const sal_uInt16 OOBIN_TOK_TABLE_COLRANGE       = 0x0002;   /// Table reference: Range of columns.
const sal_uInt16 OOBIN_TOK_TABLE_ALL            = 0x0004;   /// Table reference: Special [#All] range.
const sal_uInt16 OOBIN_TOK_TABLE_HEADERS        = 0x0008;   /// Table reference: Special [#Headers] range.
const sal_uInt16 OOBIN_TOK_TABLE_DATA           = 0x0010;   /// Table reference: Special [#Data] range.
const sal_uInt16 OOBIN_TOK_TABLE_TOTALS         = 0x0020;   /// Table reference: Special [#Totals] range.
const sal_uInt16 OOBIN_TOK_TABLE_THISROW        = 0x0040;   /// Table reference: Special [#This Row] range.
const sal_uInt16 OOBIN_TOK_TABLE_SP_BRACKETS    = 0x0080;   /// Table reference: Spaces in outer brackets.
const sal_uInt16 OOBIN_TOK_TABLE_SP_SEP         = 0x0100;   /// Table reference: Spaces after separators.
const sal_uInt16 OOBIN_TOK_TABLE_ROW            = 0x0200;   /// Table reference: Single row.
const sal_uInt16 OOBIN_TOK_TABLE_CELL           = 0x0400;   /// Table reference: Single cell.

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

const sal_uInt8 OOX_MAX_PARAMCOUNT              = 255;      /// Maximum parameter count for OOXML/OOBIN files.
const sal_uInt8 BIFF_MAX_PARAMCOUNT             = 30;       /// Maximum parameter count for BIFF files.

const sal_uInt16 OOBIN_FUNC_IF                  = 1;        /// OOBIN function id of the IF function.
const sal_uInt16 OOBIN_FUNC_SUM                 = 4;        /// OOBIN function id of the SUM function.
const sal_uInt16 OOBIN_FUNC_TRUE                = 34;       /// OOBIN function id of the TRUE function.
const sal_uInt16 OOBIN_FUNC_FALSE               = 35;       /// OOBIN function id of the FALSE function.
const sal_uInt16 OOBIN_FUNC_ROWS                = 76;       /// OOBIN function id of the ROWS function.
const sal_uInt16 OOBIN_FUNC_COLUMNS             = 77;       /// OOBIN function id of the COLUMNS function.
const sal_uInt16 OOBIN_FUNC_OFFSET              = 78;       /// OOBIN function id of the OFFSET function.
const sal_uInt16 OOBIN_FUNC_FLOOR               = 285;      /// OOBIN function id of the FLOOR function.
const sal_uInt16 OOBIN_FUNC_CEILING             = 288;      /// OOBIN function id of the CEILING function.
const sal_uInt16 OOBIN_FUNC_HYPERLINK           = 359;      /// OOBIN function id of the HYPERLINK function.
const sal_uInt16 OOBIN_FUNC_WEEKNUM             = 465;      /// OOBIN function id of the WEEKNUM function.

const sal_uInt16 BIFF_FUNC_SUM                  = 4;        /// BIFF function id of the SUM function.
const sal_uInt16 BIFF_FUNC_EXTERNCALL           = 255;      /// BIFF function id of the EXTERN.CALL function.

// reference helpers ==========================================================

/** A 2D formula cell reference struct with relative flags. */
struct BinSingleRef2d
{
    sal_Int32           mnCol;              /// Column index.
    sal_Int32           mnRow;              /// Row index.
    bool                mbColRel;           /// True = relative column reference.
    bool                mbRowRel;           /// True = relative row reference.

    explicit            BinSingleRef2d();

    void                setOobData( sal_uInt16 nCol, sal_Int32 nRow, bool bRelativeAsOffset );
    void                setBiff2Data( sal_uInt8 nCol, sal_uInt16 nRow, bool bRelativeAsOffset );
    void                setBiff8Data( sal_uInt16 nCol, sal_uInt16 nRow, bool bRelativeAsOffset );

    void                readOobData( RecordInputStream& rStrm, bool bRelativeAsOffset );
    void                readBiff2Data( BiffInputStream& rStrm, bool bRelativeAsOffset );
    void                readBiff8Data( BiffInputStream& rStrm, bool bRelativeAsOffset );
};

// ----------------------------------------------------------------------------

/** A 2D formula cell range reference struct with relative flags. */
struct BinComplexRef2d
{
    BinSingleRef2d      maRef1;             /// Start (top-left) cell address.
    BinSingleRef2d      maRef2;             /// End (bottom-right) cell address.

    void                readOobData( RecordInputStream& rStrm, bool bRelativeAsOffset );
    void                readBiff2Data( BiffInputStream& rStrm, bool bRelativeAsOffset );
    void                readBiff8Data( BiffInputStream& rStrm, bool bRelativeAsOffset );
};

// token vector, sequence =====================================================

typedef ::com::sun::star::sheet::FormulaToken       ApiToken;
typedef ::com::sun::star::uno::Sequence< ApiToken > ApiTokenSequence;

/** A vector of formula tokens with additional convenience functions. */
class ApiTokenVector : public ::std::vector< ApiToken >
{
public:
    explicit            ApiTokenVector();

    /** Appends a new token with the passed op-code, returns its data field. */
    ::com::sun::star::uno::Any&
                        append( sal_Int32 nOpCode );

    /** Appends a new token with the passed op-code and data. */
    template< typename Type >
    inline void         append( sal_Int32 nOpCode, const Type& rData ) { append( nOpCode ) <<= rData; }
};

// token sequence iterator ====================================================

/** Token sequence iterator that is able to skip space tokens. */
class ApiTokenIterator
{
public:
    explicit            ApiTokenIterator( const ApiTokenSequence& rTokens, sal_Int32 nSpacesOpCode, bool bSkipSpaces );
    /** Copy constructor that allows to change the skip spaces mode. */
    explicit            ApiTokenIterator( const ApiTokenIterator& rIter, bool bSkipSpaces );

    inline bool         is() const { return mpToken != mpTokenEnd; }
    inline const ApiToken* get() const { return mpToken; }
    inline const ApiToken* operator->() const { return mpToken; }
    inline const ApiToken& operator*() const { return *mpToken; }

    ApiTokenIterator&   operator++();

private:
    void                skipSpaces();

private:
    const ApiToken*     mpToken;            /// Pointer to current token of the token sequence.
    const ApiToken*     mpTokenEnd;         /// Pointer behind last token of the token sequence.
    const sal_Int32     mnSpacesOpCode;     /// Op-code for whitespace tokens.
    const bool          mbSkipSpaces;       /// true = Skip whitespace tokens.
};

// list of API op-codes =======================================================

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

// function data ==============================================================

/** This enumeration contains constants for all known external libraries
    containing supported sheet functions. */
enum FunctionLibraryType
{
    FUNCLIB_EUROTOOL,           /// EuroTool add-in with EUROCONVERT function.
    FUNCLIB_UNKNOWN             /// Unknown library.
};

// ----------------------------------------------------------------------------

const sal_uInt8 FUNCINFO_MAXPARAM           = 30;       /// Maximum parameter count.

const sal_uInt8 FUNCINFO_PARAM_EXCELONLY    = 0x01;     /// Flag for a parameter existing in Excel, but not in Calc.
const sal_uInt8 FUNCINFO_PARAM_CALCONLY     = 0x02;     /// Flag for a parameter existing in Calc, but not in Excel.
const sal_uInt8 FUNCINFO_PARAM_INVALID      = 0x04;     /// Flag for an invalid token class.

/** Represents information for a spreadsheet function.

    The member mpnParamClass contains an array of BIFF token classes for each
    parameter of the function. The last existing (non-null) value in this array
    is used for all following parameters used in a function. Additionally to
    the three actual token classes, this array may contain the special values
    FUNCINFO_PARAM_CALCONLY, FUNCINFO_PARAM_EXCELONLY, and
    FUNCINFO_PARAM_INVALID. The former two specify parameters only existing in
    one of the applications. FUNCINFO_PARAM_INVALID is simply a terminator for
    the array to prevent repetition of the last token class or special value
    for additional parameters.
 */
struct FunctionInfo
{
    ::rtl::OUString     maOdfFuncName;      /// ODF function name.
    ::rtl::OUString     maOoxFuncName;      /// OOXML function name.
    ::rtl::OUString     maBiffMacroName;    /// Expected macro name in EXTERN.CALL function.
    ::rtl::OUString     maExtProgName;      /// Programmatic function name for external functions.
    FunctionLibraryType meFuncLibType;      /// The external library this function is part of.
    sal_Int32           mnApiOpCode;        /// API function opcode.
    sal_uInt16          mnOobFuncId;        /// OOBIN function identifier.
    sal_uInt16          mnBiffFuncId;       /// BIFF function identifier.
    sal_uInt8           mnMinParamCount;    /// Minimum number of parameters.
    sal_uInt8           mnMaxParamCount;    /// Maximum number of parameters.
    sal_uInt8           mnRetClass;         /// BIFF token class of the return value.
    const sal_uInt8*    mpnParamClass;      /// Expected BIFF token classes of parameters.
    bool                mbVolatile;         /// True = volatile function.
    bool                mbExternal;         /// True = external function in Calc.
    bool                mbMacroFunc;        /// True = macro sheet function or command.
    bool                mbVarParam;         /// True = use a tFuncVar token, also if min/max are equal.
};

typedef RefVector< FunctionInfo > FunctionInfoVector;

// function info parameter class iterator =====================================

/** Iterator working on the mpnParamClass member of the FunctionInfo struct.

    This iterator can be used to iterate through the array containing the
    expected token classes of function parameters. This iterator repeats the
    last valid token class in the array - it stops automatically before the
    first empty array entry or before the end of the array, even for repeated
    calls to the increment operator.
 */
class FuncInfoParamClassIterator
{
public:
    explicit            FuncInfoParamClassIterator( const FunctionInfo& rFuncInfo );

    FuncInfoParamClassIterator& operator++();

    inline sal_uInt8    getParamClass() const { return *mpnParamClass; }
    inline sal_uInt8    isExcelOnlyParam() const { return getFlag( *mpnParamClass, FUNCINFO_PARAM_EXCELONLY ); }
    inline sal_uInt8    isCalcOnlyParam() const { return getFlag( *mpnParamClass, FUNCINFO_PARAM_CALCONLY ); }

private:
    const sal_uInt8*    mpnParamClass;
    const sal_uInt8*    mpnParamClassEnd;
};

// base function provider =====================================================

struct FunctionProviderImpl;

/** Provides access to function info structs for all available sheet functions.
 */
class FunctionProvider  // not derived from WorkbookHelper to make it usable in file dumpers
{
public:
    explicit            FunctionProvider( FilterType eFilter, BiffType eBiff, bool bImportFilter );
    virtual             ~FunctionProvider();

    /** Returns the function info for an ODF function name, or 0 on error. */
    const FunctionInfo* getFuncInfoFromOdfFuncName( const ::rtl::OUString& rFuncName ) const;

    /** Returns the function info for an OOX function name, or 0 on error. */
    const FunctionInfo* getFuncInfoFromOoxFuncName( const ::rtl::OUString& rFuncName ) const;

    /** Returns the function info for an OOBIN function index, or 0 on error. */
    const FunctionInfo* getFuncInfoFromOobFuncId( sal_uInt16 nFuncId ) const;

    /** Returns the function info for a BIFF function index, or 0 on error. */
    const FunctionInfo* getFuncInfoFromBiffFuncId( sal_uInt16 nFuncId ) const;

    /** Returns the function info for a macro function referred by the
        EXTERN.CALL function, or 0 on error. */
    const FunctionInfo* getFuncInfoFromMacroName( const ::rtl::OUString& rFuncName ) const;

    /** Returns the library type associated with the passed URL of a function
        library (function add-in). */
    FunctionLibraryType getFuncLibTypeFromLibraryName( const ::rtl::OUString& rLibraryName ) const;

protected:
    /** Returns the list of all function infos. */
    const FunctionInfoVector& getFuncs() const;

private:
    typedef ::boost::shared_ptr< FunctionProviderImpl > FunctionProviderImplRef;
    FunctionProviderImplRef mxFuncImpl;     /// Shared implementation between all copies of the provider.
};

// op-code and function provider ==============================================

struct OpCodeProviderImpl;

/** Provides access to API op-codes for all available formula tokens and to
    function info structs for all available sheet functions.
 */
class OpCodeProvider : public FunctionProvider // not derived from WorkbookHelper to make it usable as UNO service
{
public:
    explicit            OpCodeProvider(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            FilterType eFilter, BiffType eBiff, bool bImportFilter );
    virtual             ~OpCodeProvider();

    /** Returns the structure containing all token op-codes for operators and
        special tokens used by the Calc document and its formula parser. */
    const ApiOpCodes&   getOpCodes() const;

    /** Returns the function info for an API token, or 0 on error. */
    const FunctionInfo* getFuncInfoFromApiToken( const ApiToken& rToken ) const;

    /** Returns the op-code map that is used by the OOX formula parser. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaOpCodeMapEntry >
                        getOoxParserMap() const;

private:
    typedef ::boost::shared_ptr< OpCodeProviderImpl > OpCodeProviderImplRef;
    OpCodeProviderImplRef mxOpCodeImpl;     /// Shared implementation between all copies of the provider.
};

// API formula parser wrapper =================================================

/** A wrapper around the FormulaParser service provided by the Calc document. */
class ApiParserWrapper : public OpCodeProvider
{
public:
    explicit            ApiParserWrapper(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            const OpCodeProvider& rOpCodeProv );

    /** Returns read/write access to the formula parser property set. */
    inline PropertySet& getParserProperties() { return maParserProps; }

    /** Calls the XFormulaParser::parseFormula() function of the API parser. */
    ApiTokenSequence    parseFormula(
                            const ::rtl::OUString& rFormula,
                            const ::com::sun::star::table::CellAddress& rRefPos );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser >
                        mxParser;
    PropertySet         maParserProps;
};

// formula contexts ===========================================================

class FormulaContext
{
public:
    inline void         setBaseAddress( const ::com::sun::star::table::CellAddress& rBaseAddress )
                            { maBaseAddress = rBaseAddress; }

    inline const ::com::sun::star::table::CellAddress& getBaseAddress() const { return maBaseAddress; }
    inline bool         isRelativeAsOffset() const { return mbRelativeAsOffset; }
    inline bool         is2dRefsAs3dRefs() const { return mb2dRefsAs3dRefs; }
    inline bool         isNulCharsAllowed() const { return mbAllowNulChars; }

    virtual void        setTokens( const ApiTokenSequence& rTokens ) = 0;
    virtual void        setSharedFormula( const ::com::sun::star::table::CellAddress& rBaseAddr );

protected:
    explicit            FormulaContext(
                            bool bRelativeAsOffset,
                            bool b2dRefsAs3dRefs,
                            bool bAllowNulChars = false );
    virtual             ~FormulaContext();

private:
    ::com::sun::star::table::CellAddress maBaseAddress;
    bool                mbRelativeAsOffset;
    bool                mb2dRefsAs3dRefs;
    bool                mbAllowNulChars;
};

// ----------------------------------------------------------------------------

/** Stores the converted formula token sequence in a class member. */
class TokensFormulaContext : public FormulaContext
{
public:
    explicit            TokensFormulaContext(
                            bool bRelativeAsOffset,
                            bool b2dRefsAs3dRefs,
                            bool bAllowNulChars = false );

    inline const ApiTokenSequence& getTokens() const { return maTokens; }

    virtual void        setTokens( const ApiTokenSequence& rTokens );

private:
    ApiTokenSequence    maTokens;
};

// ----------------------------------------------------------------------------

/** Uses the com.sun.star.sheet.XFormulaTokens interface to set a token sequence. */
class SimpleFormulaContext : public FormulaContext
{
public:
    explicit            SimpleFormulaContext(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaTokens >& rxTokens,
                            bool bRelativeAsOffset,
                            bool b2dRefsAs3dRefs,
                            bool bAllowNulChars = false );

    virtual void        setTokens( const ApiTokenSequence& rTokens );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaTokens > mxTokens;
};

// formula parser/printer base class for filters ==============================

/** Base class for import formula parsers and export formula compilers. */
class FormulaProcessorBase : public OpCodeProvider, protected ApiOpCodes, public WorkbookHelper
{
public:
    explicit            FormulaProcessorBase( const WorkbookHelper& rHelper );

    // ------------------------------------------------------------------------

    /** Generates a cell address string in A1 notation from the passed cell
        address.

        @param rAddress  The cell address containing column and row index.
        @param bAbsolute  True = adds dollar signs before column and row.
     */
    static ::rtl::OUString generateAddress2dString(
                            const ::com::sun::star::table::CellAddress& rAddress,
                            bool bAbsolute );

    /** Generates a cell address string in A1 notation from the passed binary
        cell address.

        @param rAddress  The cell address containing column and row index.
        @param bAbsolute  True = adds dollar signs before column and row.
     */
    static ::rtl::OUString generateAddress2dString(
                            const BinAddress& rAddress,
                            bool bAbsolute );

    /** Generates a cell range string in A1:A1 notation from the passed cell
        range address.

        @param rRange  The cell range address containing column and row indexes.
        @param bAbsolute  True = adds dollar signs before columns and rows.
     */
    static ::rtl::OUString generateRange2dString(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            bool bAbsolute );

    /** Generates a cell range string in A1:A1 notation from the passed binary
        cell range address.

        @param rRange  The cell range address containing column and row indexes.
        @param bAbsolute  True = adds dollar signs before columns and rows.
     */
    static ::rtl::OUString generateRange2dString(
                            const BinRange& rRange,
                            bool bAbsolute );

    /** Generates a cell range list string in A1:A1 notation from the passed
        cell range addresses. May enclose multiple ranges into parentheses.

        @param rRanges  The list of cell range addresses.
        @param bAbsolute  True = adds dollar signs before columns and rows.
        @param cSeparator  Separator character between ranges.
        @param bEncloseMultiple  True = enclose multiple ranges in parentheses.
     */
    static ::rtl::OUString generateRangeList2dString(
                            const ApiCellRangeList& rRanges,
                            bool bAbsolute,
                            sal_Unicode cSeparator,
                            bool bEncloseMultiple );

    // ------------------------------------------------------------------------

    /** Generates a cell address string in Calc's absolute $Sheet.$A$1 notation
        from the passed cell address.

        @param rAddress  The cell address to be converted to a string.
     */
    ::rtl::OUString     generateApiAddressString(
                            const ::com::sun::star::table::CellAddress& rAddress ) const;

    /** Generates a cell range string in Calc's absolute $Sheet.$A$1:$A$
        notation from the passed cell range address.

        @param rRange  The cell range address to be converted to a string.
     */
    ::rtl::OUString     generateApiRangeString(
                            const ::com::sun::star::table::CellRangeAddress& rRange ) const;

    /** Generates a cell range list string in Calc's absolute $Sheet.$A$1:$A$1
        notation from the passed cell range addresses.

        @param rRanges  The list of cell ranges to be converted to a string.
     */
    ::rtl::OUString     generateApiRangeListString( const ApiCellRangeList& rRanges ) const;

    /** Generates a string in Calc formula notation from the passed string.

        @param rString  The string value.

        @return  The string enclosed in double quotes, where all contained
            quote characters are doubled.
     */
    static ::rtl::OUString generateApiString( const ::rtl::OUString& rString );

    /** Generates an array string in Calc formula notation from the passed
        matrix with Any's containing double values or strings.

        @param rMatrix  The matrix containing double values or strings.
     */
    static ::rtl::OUString generateApiArray( const Matrix< ::com::sun::star::uno::Any >& rMatrix );

    // ------------------------------------------------------------------------

    /** Tries to extract a single cell reference from a formula token sequence.

        @param rTokens  The token sequence to be parsed. Should contain exactly
            one address token or cell range address token. The token sequence
            may contain whitespace tokens.

        @return  If the token sequence is valid, this function returns an Any
            containing a com.sun.star.sheet.SingleReference object, or a
            com.sun.star.sheet.ComplexReference object. If the token sequence
            contains too many, or unexpected tokens, an empty Any is returned.
     */
    ::com::sun::star::uno::Any
                        extractReference( const ApiTokenSequence& rTokens ) const;

    /** Tries to extract a single cell address from a formula token sequence.

        @param orAddress  (output parameter) If the token sequence is valid,
            this parameter will contain the extracted cell address. If the
            token sequence contains unexpected tokens, nothing meaningful is
            inserted, and the function returns false.

        @param rTokens  The token sequence to be parsed. Should contain exactly
            one cell address token. The token sequence may contain whitespace
            tokens.

        @param bAllowRelative  True = it is allowed that rTokens contains
            relative references (based on cell A1 of the current sheet).
            False = only real absolute references will be accepted.

        @return  True, if the token sequence contains a valid cell address
            which has been extracted to orAddress, false otherwise.
     */
    bool                extractCellAddress(
                            ::com::sun::star::table::CellAddress& orAddress,
                            const ApiTokenSequence& rTokens,
                            bool bAllowRelative ) const;

    /** Tries to extract a cell range address from a formula token sequence.

        @param orAddress  (output parameter) If the token sequence is valid,
            this parameter will contain the extracted cell range address. If
            the token sequence contains unexpected tokens, nothing meaningful
            is inserted, and the function returns false.

        @param rTokens  The token sequence to be parsed. Should contain exactly
            one cell range address token. The token sequence may contain
            whitespace tokens.

        @param bAllowRelative  True = it is allowed that rTokens contains
            relative references (based on cell A1 of the current sheet).
            False = only real absolute references will be accepted.

        @return  True, if the token sequence contains a valid cell range
            address which has been extracted to orRange, false otherwise.
     */
    bool                extractCellRange(
                            ::com::sun::star::table::CellRangeAddress& orRange,
                            const ApiTokenSequence& rTokens,
                            bool bAllowRelative ) const;

    /** Tries to extract a cell range list from a formula token sequence.

        @param orRanges  (output parameter) If the token sequence is valid,
            this parameter will contain the extracted cell range list. Deleted
            cells or cell ranges (shown as #REF! error in a formula) will be
            skipped. If the token sequence contains unexpected tokens, an empty
            list is returned here.

        @param rTokens  The token sequence to be parsed. Should contain cell
            address tokens or cell range address tokens, separated by the
            standard function parameter separator token. The token sequence may
            contain parentheses and whitespace tokens.

        @param bAllowRelative  True = it is allowed that rTokens contains
            relative references (based on cell A1 of the current sheet).
            False = only real absolute references will be accepted.

        @param nFilterBySheet  If non-negative, this function returns only cell
            ranges located in the specified sheet, otherwise returns all cell
            ranges contained in the token sequence.
     */
    void                extractCellRangeList(
                            ApiCellRangeList& orRanges,
                            const ApiTokenSequence& rTokens,
                            bool bAllowRelative,
                            sal_Int32 nFilterBySheet = -1 ) const;

    /** Tries to extract a string from a formula token sequence.

        @param orString  (output parameter) The extracted string.

        @param rTokens  The token sequence to be parsed. Should contain exactly
            one string token, may contain whitespace tokens.

        @return  True = token sequence is valid, output parameter orString
            contains the string extracted from the token sequence.
     */
    bool                extractString(
                            ::rtl::OUString& orString,
                            const ApiTokenSequence& rTokens ) const;

    /** Converts a single string with separators in the passed formula token
        sequence to a list of string tokens.

        @param orTokens  (input/output parameter) Expects a single string token
            in this token sequence (whitespace tokens are allowed). The string
            is split into substrings. A list of string tokens separated with
            parameter separator tokens is returned in this psrameter.

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

// ============================================================================

} // namespace xls
} // namespace oox

#endif

