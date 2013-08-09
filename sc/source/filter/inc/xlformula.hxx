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

#ifndef SC_XLFORMULA_HXX
#define SC_XLFORMULA_HXX

#include <map>
#include <formula/opcode.hxx>
#include "address.hxx"
#include "ftools.hxx"
#include <boost/shared_ptr.hpp>

// Constants ==================================================================

const size_t EXC_TOKARR_MAXLEN              = 4096;     /// Maximum size of a token array.

// Token class flags ----------------------------------------------------------

const sal_uInt8 EXC_TOKCLASS_MASK           = 0x60;
const sal_uInt8 EXC_TOKCLASS_NONE           = 0x00;     /// 00-1F: Base tokens.
const sal_uInt8 EXC_TOKCLASS_REF            = 0x20;     /// 20-3F: Reference class tokens.
const sal_uInt8 EXC_TOKCLASS_VAL            = 0x40;     /// 40-5F: Value class tokens.
const sal_uInt8 EXC_TOKCLASS_ARR            = 0x60;     /// 60-7F: Array class tokens.

// Base tokens ----------------------------------------------------------------

const sal_uInt8 EXC_TOKID_MASK              = 0x1F;

const sal_uInt8 EXC_TOKID_NONE              = 0x00;     /// Placeholder for invalid token id.
const sal_uInt8 EXC_TOKID_EXP               = 0x01;     /// Array or shared formula reference.
const sal_uInt8 EXC_TOKID_TBL               = 0x02;     /// Multiple operation reference.
const sal_uInt8 EXC_TOKID_ADD               = 0x03;     /// Addition operator.
const sal_uInt8 EXC_TOKID_SUB               = 0x04;     /// Subtraction operator.
const sal_uInt8 EXC_TOKID_MUL               = 0x05;     /// Multiplication operator.
const sal_uInt8 EXC_TOKID_DIV               = 0x06;     /// Division operator.
const sal_uInt8 EXC_TOKID_POWER             = 0x07;     /// Power operator.
const sal_uInt8 EXC_TOKID_CONCAT            = 0x08;     /// String concatenation operator.
const sal_uInt8 EXC_TOKID_LT                = 0x09;     /// Less than operator.
const sal_uInt8 EXC_TOKID_LE                = 0x0A;     /// Less than or equal operator.
const sal_uInt8 EXC_TOKID_EQ                = 0x0B;     /// Equal operator.
const sal_uInt8 EXC_TOKID_GE                = 0x0C;     /// Greater than or equal operator.
const sal_uInt8 EXC_TOKID_GT                = 0x0D;     /// Greater than operator.
const sal_uInt8 EXC_TOKID_NE                = 0x0E;     /// Not equal operator.
const sal_uInt8 EXC_TOKID_ISECT             = 0x0F;     /// Intersection operator.
const sal_uInt8 EXC_TOKID_LIST              = 0x10;     /// List operator.
const sal_uInt8 EXC_TOKID_RANGE             = 0x11;     /// Range operator.
const sal_uInt8 EXC_TOKID_UPLUS             = 0x12;     /// Unary plus.
const sal_uInt8 EXC_TOKID_UMINUS            = 0x13;     /// Unary minus.
const sal_uInt8 EXC_TOKID_PERCENT           = 0x14;     /// Percent sign.
const sal_uInt8 EXC_TOKID_PAREN             = 0x15;     /// Parentheses.
const sal_uInt8 EXC_TOKID_MISSARG           = 0x16;     /// Missing argument.
const sal_uInt8 EXC_TOKID_STR               = 0x17;     /// String constant.
const sal_uInt8 EXC_TOKID_NLR               = 0x18;     /// Natural language reference (NLR).
const sal_uInt8 EXC_TOKID_ATTR              = 0x19;     /// Special attribute.
const sal_uInt8 EXC_TOKID_SHEET             = 0x1A;     /// Start of a sheet reference (BIFF2-BIFF4).
const sal_uInt8 EXC_TOKID_ENDSHEET          = 0x1B;     /// End of a sheet reference (BIFF2-BIFF4).
const sal_uInt8 EXC_TOKID_ERR               = 0x1C;     /// Error constant.
const sal_uInt8 EXC_TOKID_BOOL              = 0x1D;     /// Boolean constant.
const sal_uInt8 EXC_TOKID_INT               = 0x1E;     /// Integer constant.
const sal_uInt8 EXC_TOKID_NUM               = 0x1F;     /// Floating-point constant.

// Base IDs of classified tokens ----------------------------------------------

const sal_uInt8 EXC_TOKID_ARRAY             = 0x00;     /// Array constant.
const sal_uInt8 EXC_TOKID_FUNC              = 0x01;     /// Function, fixed number of arguments.
const sal_uInt8 EXC_TOKID_FUNCVAR           = 0x02;     /// Function, variable number of arguments.
const sal_uInt8 EXC_TOKID_NAME              = 0x03;     /// Defined name.
const sal_uInt8 EXC_TOKID_REF               = 0x04;     /// 2D cell reference.
const sal_uInt8 EXC_TOKID_AREA              = 0x05;     /// 2D area reference.
const sal_uInt8 EXC_TOKID_MEMAREA           = 0x06;     /// Constant reference subexpression.
const sal_uInt8 EXC_TOKID_MEMERR            = 0x07;     /// Deleted reference subexpression.
const sal_uInt8 EXC_TOKID_MEMNOMEM          = 0x08;     /// Constant reference subexpression without result.
const sal_uInt8 EXC_TOKID_MEMFUNC           = 0x09;     /// Variable reference subexpression.
const sal_uInt8 EXC_TOKID_REFERR            = 0x0A;     /// Deleted 2D cell reference.
const sal_uInt8 EXC_TOKID_AREAERR           = 0x0B;     /// Deleted 2D area reference.
const sal_uInt8 EXC_TOKID_REFN              = 0x0C;     /// Relative 2D cell reference (in names).
const sal_uInt8 EXC_TOKID_AREAN             = 0x0D;     /// Relative 2D area reference (in names).
const sal_uInt8 EXC_TOKID_MEMAREAN          = 0x0E;     /// Reference subexpression (in names).
const sal_uInt8 EXC_TOKID_MEMNOMEMN         = 0x0F;     /// Reference subexpression (in names) without result.
const sal_uInt8 EXC_TOKID_FUNCCE            = 0x18;
const sal_uInt8 EXC_TOKID_NAMEX             = 0x19;     /// External reference.
const sal_uInt8 EXC_TOKID_REF3D             = 0x1A;     /// 3D cell reference.
const sal_uInt8 EXC_TOKID_AREA3D            = 0x1B;     /// 3D area reference.
const sal_uInt8 EXC_TOKID_REFERR3D          = 0x1C;     /// Deleted 3D cell reference.
const sal_uInt8 EXC_TOKID_AREAERR3D         = 0x1D;     /// Deleted 3D area reference

// specific token constants ---------------------------------------------------

const sal_uInt16 EXC_TOK_STR_MAXLEN         = 255;      /// Maximum string length of a tStr token.

const sal_uInt8 EXC_TOK_BOOL_FALSE          = 0;        /// sal_False value of a tBool token.
const sal_uInt8 EXC_TOK_BOOL_TRUE           = 1;        /// sal_True value of a tBool token.

const sal_uInt8 EXC_TOK_ATTR_VOLATILE       = 0x01;     /// Volatile function.
const sal_uInt8 EXC_TOK_ATTR_IF             = 0x02;     /// Start of true condition in IF function.
const sal_uInt8 EXC_TOK_ATTR_CHOOSE         = 0x04;     /// Jump array of CHOOSE function.
const sal_uInt8 EXC_TOK_ATTR_GOTO           = 0x08;     /// Jump to token.
const sal_uInt8 EXC_TOK_ATTR_SUM            = 0x10;     /// SUM function with one parameter.
const sal_uInt8 EXC_TOK_ATTR_ASSIGN         = 0x20;     /// BASIC style assignment.
const sal_uInt8 EXC_TOK_ATTR_SPACE          = 0x40;     /// Spaces in formula representation.

const sal_uInt8 EXC_TOK_ATTR_SPACE_SP       = 0x00;     /// Spaces before next token.
const sal_uInt8 EXC_TOK_ATTR_SPACE_BR       = 0x01;     /// Line breaks before next token.
const sal_uInt8 EXC_TOK_ATTR_SPACE_SP_OPEN  = 0x02;     /// Spaces before opening parenthesis.
const sal_uInt8 EXC_TOK_ATTR_SPACE_BR_OPEN  = 0x03;     /// Line breaks before opening parenthesis.
const sal_uInt8 EXC_TOK_ATTR_SPACE_SP_CLOSE = 0x04;     /// Spaces before closing parenthesis.
const sal_uInt8 EXC_TOK_ATTR_SPACE_BR_CLOSE = 0x05;     /// Line breaks before closing parenthesis.
const sal_uInt8 EXC_TOK_ATTR_SPACE_SP_PRE   = 0x06;     /// Spaces before formula (BIFF3).

const sal_uInt16 EXC_TOK_FUNCVAR_CMD        = 0x8000;   /// Macro command.
const sal_uInt16 EXC_TOK_FUNCVAR_INDEXMASK  = 0x7FFF;   /// Mask for function/command index.
const sal_uInt8 EXC_TOK_FUNCVAR_PROMPT      = 0x80;     /// User prompt for macro commands.
const sal_uInt8 EXC_TOK_FUNCVAR_COUNTMASK   = 0x7F;     /// Mask for parameter count.

const sal_uInt16 EXC_TOK_REF_COLREL         = 0x4000;   /// True = Column is relative.
const sal_uInt16 EXC_TOK_REF_ROWREL         = 0x8000;   /// True = Row is relative.

const sal_uInt8 EXC_TOK_NLR_ERR             = 0x01;     /// NLR: Invalid/deleted.
const sal_uInt8 EXC_TOK_NLR_ROWR            = 0x02;     /// NLR: Row index.
const sal_uInt8 EXC_TOK_NLR_COLR            = 0x03;     /// NLR: Column index.
const sal_uInt8 EXC_TOK_NLR_ROWV            = 0x06;     /// NLR: Value in row.
const sal_uInt8 EXC_TOK_NLR_COLV            = 0x07;     /// NLR: Value in column.
const sal_uInt8 EXC_TOK_NLR_RANGE           = 0x0A;     /// NLR: Range.
const sal_uInt8 EXC_TOK_NLR_SRANGE          = 0x0B;     /// Stacked NLR: Range.
const sal_uInt8 EXC_TOK_NLR_SROWR           = 0x0C;     /// Stacked NLR: Row index.
const sal_uInt8 EXC_TOK_NLR_SCOLR           = 0x0D;     /// Stacked NLR: Column index.
const sal_uInt8 EXC_TOK_NLR_SROWV           = 0x0E;     /// Stacked NLR: Value in row.
const sal_uInt8 EXC_TOK_NLR_SCOLV           = 0x0F;     /// Stacked NLR: Value in column.
const sal_uInt8 EXC_TOK_NLR_RANGEERR        = 0x10;     /// NLR: Invalid/deleted range.
const sal_uInt8 EXC_TOK_NLR_SXNAME          = 0x1D;     /// NLR: Pivot table name.
const sal_uInt16 EXC_TOK_NLR_REL            = 0x8000;   /// True = Natural language ref is relative.

const sal_uInt32 EXC_TOK_NLR_ADDREL         = 0x80000000;   /// NLR relative (in appended data).
const sal_uInt32 EXC_TOK_NLR_ADDMASK        = 0x3FFFFFFF;   /// Mask for number of appended ranges.

// ----------------------------------------------------------------------------

/** Type of a formula. */
enum XclFormulaType
{
    EXC_FMLATYPE_CELL,          /// Simple cell formula, also used in change tracking.
    EXC_FMLATYPE_MATRIX,        /// Matrix (array) formula.
    EXC_FMLATYPE_SHARED,        /// Shared formula.
    EXC_FMLATYPE_CONDFMT,       /// Conditional format.
    EXC_FMLATYPE_DATAVAL,       /// Data validation.
    EXC_FMLATYPE_NAME,          /// Defined name.
    EXC_FMLATYPE_CHART,         /// Chart source ranges.
    EXC_FMLATYPE_CONTROL,       /// Spreadsheet links in form controls.
    EXC_FMLATYPE_WQUERY,        /// Web query source range.
    EXC_FMLATYPE_LISTVAL        /// List (cell range) validation.
};

// Function parameter info ====================================================

/** Enumerates validity modes for a function parameter. */
enum XclFuncParamValidity
{
    EXC_PARAM_NONE = 0,         /// Default for an unspecified entry in a C-array.
    EXC_PARAM_REGULAR,          /// Parameter supported by Calc and Excel.
    EXC_PARAM_CALCONLY,         /// Parameter supported by Calc only.
    EXC_PARAM_EXCELONLY         /// Parameter supported by Excel only.
};

/** Enumerates different types of token class conversion in function parameters. */
enum XclFuncParamConv
{
    EXC_PARAMCONV_ORG,          /// Use original class of current token.
    EXC_PARAMCONV_VAL,          /// Convert tokens to VAL class.
    EXC_PARAMCONV_ARR,          /// Convert tokens to ARR class.
    EXC_PARAMCONV_RPT,          /// Repeat parent conversion in VALTYPE parameters.
    EXC_PARAMCONV_RPX,          /// Repeat parent conversion in REFTYPE parameters.
    EXC_PARAMCONV_RPO           /// Repeat parent conversion in operands of operators.
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
struct XclFuncParamInfo
{
    XclFuncParamValidity meValid;       /// Parameter validity.
    XclFuncParamConv    meConv;         /// Token class conversion type.
    bool                mbValType;      /// Data type (false = REFTYPE, true = VALTYPE).
};

// Function data ==============================================================

const sal_uInt8 EXC_FUNC_MAXPARAM           = 30;       /// Maximum parameter count.

const size_t EXC_FUNCINFO_PARAMINFO_COUNT   = 5;        /// Number of parameter info entries.

const sal_uInt8 EXC_FUNCFLAG_VOLATILE       = 0x01;     /// Result is volatile (e.g. NOW() function).
const sal_uInt8 EXC_FUNCFLAG_IMPORTONLY     = 0x02;     /// Only used in import filter.
const sal_uInt8 EXC_FUNCFLAG_EXPORTONLY     = 0x04;     /// Only used in export filter.
const sal_uInt8 EXC_FUNCFLAG_PARAMPAIRS     = 0x08;     /// Optional parameters are expected to appear in pairs.

// selected function IDs
const sal_uInt16 EXC_FUNCID_IF              = 1;
const sal_uInt16 EXC_FUNCID_SUM             = 4;
const sal_uInt16 EXC_FUNCID_AND             = 36;
const sal_uInt16 EXC_FUNCID_OR              = 37;
const sal_uInt16 EXC_FUNCID_CHOOSE          = 100;
const sal_uInt16 EXC_FUNCID_EXTERNCALL      = 255;

/** Represents information for a spreadsheet function for import and export.

    The member mpParamInfos points to an array of type information structures
    for all parameters of the function. The last initialized structure
    describing a regular parameter (member meValid == EXC_PARAMVALID_ALWAYS) in
    this array is used repeatedly for all following parameters supported by a
    function.
 */
struct XclFunctionInfo
{
    OpCode              meOpCode;           /// Calc function opcode.
    sal_uInt16          mnXclFunc;          /// Excel function index.
    sal_uInt8           mnMinParamCount;    /// Minimum number of parameters.
    sal_uInt8           mnMaxParamCount;    /// Maximum number of parameters.
    sal_uInt8           mnRetClass;         /// Token class of the return value.
    XclFuncParamInfo    mpParamInfos[ EXC_FUNCINFO_PARAMINFO_COUNT ]; /// Information for all parameters.
    sal_uInt8           mnFlags;            /// Additional flags (EXC_FUNCFLAG_* constants).
    const sal_Char*     mpcMacroName;       /// Function name, if simulated by a macro call (UTF-8).

    /** Returns true, if the function is volatile. */
    inline bool         IsVolatile() const { return ::get_flag( mnFlags, EXC_FUNCFLAG_VOLATILE ); }
    /** Returns true, if optional parameters are expected to appear in pairs. */
    inline bool         IsParamPairs() const { return ::get_flag( mnFlags, EXC_FUNCFLAG_PARAMPAIRS ); }
    /** Returns true, if the function parameter count is fixed. */
    inline bool         IsFixedParamCount() const { return (mnXclFunc != EXC_FUNCID_EXTERNCALL) && (mnMinParamCount == mnMaxParamCount); }
    /** Returns true, if the function is simulated by a macro call. */
    inline bool         IsMacroFunc() const { return mpcMacroName != 0; }
    /** Returns the name of the external function as string. */
    String              GetMacroFuncName() const;
};

// ----------------------------------------------------------------------------

class XclRoot;

/** Provides access to function info structs for all available functions. */
class XclFunctionProvider
{
public:
    explicit            XclFunctionProvider( const XclRoot& rRoot );

    /** Returns the function data for an Excel function index, or 0 on error. */
    const XclFunctionInfo* GetFuncInfoFromXclFunc( sal_uInt16 nXclFunc ) const;
    /** Returns the function data for an Excel function simulated by a macro call, or 0 on error. */
    const XclFunctionInfo* GetFuncInfoFromXclMacroName( const String& rXclMacroName ) const;
    /** Returns the function data for a Calc opcode, or 0 on error. */
    const XclFunctionInfo* GetFuncInfoFromOpCode( OpCode eOpCode ) const;

private:
    void                FillXclFuncMap( const XclFunctionInfo* pBeg, const XclFunctionInfo* pEnd );
    void                FillScFuncMap( const XclFunctionInfo* pBeg, const XclFunctionInfo* pEnd );

private:
    typedef ::std::map< sal_uInt16, const XclFunctionInfo* >    XclFuncMap;
    typedef ::std::map< String, const XclFunctionInfo* >        XclMacroNameMap;
    typedef ::std::map< OpCode, const XclFunctionInfo* >        ScFuncMap;

    XclFuncMap          maXclFuncMap;       /// Maps Excel function indexes to function data.
    XclMacroNameMap     maXclMacroNameMap;  /// Maps macro function names to function data.
    ScFuncMap           maScFuncMap;        /// Maps Calc opcodes to function data.
};

// Token array ================================================================

class XclImpStream;
class XclExpStream;

/** Binary representation of an Excel token array. */
class XclTokenArray
{
public:
    /** Creates an empty token array. */
    explicit            XclTokenArray( bool bVolatile = false );
    /** Creates a token array, swaps passed token vectors into own data. */
    explicit            XclTokenArray( ScfUInt8Vec& rTokVec, ScfUInt8Vec& rExtDataVec, bool bVolatile = false );

    /** Returns true, if the token array is empty. */
    inline bool         Empty() const { return maTokVec.empty(); }
    /** Returns the size of the token array in bytes. */
    sal_uInt16          GetSize() const;
    /** Returns read-only access to the byte vector storing token data. */
    inline const sal_uInt8* GetData() const { return maTokVec.empty() ? 0 : &maTokVec.front(); }
    /** Returns true, if the formula contains a volatile function. */
    inline bool         IsVolatile() const { return mbVolatile; }

    /** Reads the size field of the token array. */
    void                ReadSize( XclImpStream& rStrm );
    /** Reads the tokens of the token array (without size field). */
    void                ReadArray( XclImpStream& rStrm );
    /** Reads size field and the tokens. */
    void                Read( XclImpStream& rStrm );

    /** Writes the size field of the token array. */
    void                WriteSize( XclExpStream& rStrm ) const;
    /** Writes the tokens of the token array (without size field). */
    void                WriteArray( XclExpStream& rStrm ) const;
    /** Writes size field and the tokens. */
    void                Write( XclExpStream& rStrm ) const;

    /** Compares this token array with the passed. */
    bool                operator==( const XclTokenArray& rTokArr ) const;

private:
    ScfUInt8Vec         maTokVec;       /// Byte vector containing token data.
    ScfUInt8Vec         maExtDataVec;   /// Byte vector containing extended data (arrays, stacked NLRs).
    bool                mbVolatile;     /// True = Formula contains volatile function.
};

typedef boost::shared_ptr< XclTokenArray > XclTokenArrayRef;

/** Calls the Read() function at the passed token array. */
XclImpStream& operator>>( XclImpStream& rStrm, XclTokenArray& rTokArr );
/** Calls the Read() function at the passed token array. */
XclImpStream& operator>>( XclImpStream& rStrm, XclTokenArrayRef& rxTokArr );
/** Calls the Write() function at the passed token array. */
XclExpStream& operator<<( XclExpStream& rStrm, const XclTokenArray& rTokArr );
/** Calls the Write() function at the passed token array. */
XclExpStream& operator<<( XclExpStream& rStrm, const XclTokenArrayRef& rxTokArr );

// ----------------------------------------------------------------------------

namespace formula { class FormulaToken; }
class ScTokenArray;

/** Special token array iterator for the Excel filters.

    Iterates over a Calc token array without modifying it (therefore the
    iterator can be used with constant token arrays).

    Usage: Construct a new iterator object and pass a Calc token array, or use
    the Init() function to assign another Calc token array. As long as the Is()
    function returns true, the accessor functions can be used to get the
    current Calc token.
 */
class XclTokenArrayIterator
{
public:
    explicit            XclTokenArrayIterator();
    explicit            XclTokenArrayIterator( const ScTokenArray& rScTokArr, bool bSkipSpaces );
    /** Copy constructor that allows to change the skip-spaces mode. */
    explicit            XclTokenArrayIterator( const XclTokenArrayIterator& rTokArrIt, bool bSkipSpaces );

    void                Init( const ScTokenArray& rScTokArr, bool bSkipSpaces );

    inline bool         Is() const { return mppScToken != 0; }
    inline bool         operator!() const { return !Is(); }
    inline const ::formula::FormulaToken* Get() const { return mppScToken ? *mppScToken : 0; }
    inline const ::formula::FormulaToken* operator->() const { return Get(); }
    inline const ::formula::FormulaToken& operator*() const { return *Get(); }

    XclTokenArrayIterator& operator++();

private:
    void                NextRawToken();
    void                SkipSpaces();

private:
    const ::formula::FormulaToken*const* mppScTokenBeg;     /// Pointer to first token pointer of token array.
    const ::formula::FormulaToken*const* mppScTokenEnd;     /// Pointer behind last token pointer of token array.
    const ::formula::FormulaToken*const* mppScToken;        /// Pointer to current token pointer of token array.
    bool                mbSkipSpaces;       /// true = Skip whitespace tokens.
};

// ----------------------------------------------------------------------------

/** Contains all cell references that can be extracted from a multiple operations formula. */
struct XclMultipleOpRefs
{
    ScAddress           maFmlaScPos;        /// Position of the (first) formula cell.
    ScAddress           maColFirstScPos;
    ScAddress           maColRelScPos;
    ScAddress           maRowFirstScPos;
    ScAddress           maRowRelScPos;
    bool                mbDblRefMode;       /// true = One formula with row and column values.
};

// ----------------------------------------------------------------------------

/** A helper with Excel specific token array functions.

    The purpose to not add these functions to ScTokenArray is to prevent code
    changes in low-level Calc headers and to keep the Excel specific source
    code in the filter directory. Deriving from ScTokenArray is not viable
    because that would need expensive copy-constructions of the token arrays.
 */
class XclTokenArrayHelper
{
public:
    // token identifiers ------------------------------------------------------

    /** Returns the base token ID of the passed (classified) token ID. */
    inline static sal_uInt8 GetBaseTokenId( sal_uInt8 nTokenId ) { return nTokenId & EXC_TOKID_MASK; }
    /** Returns the classified token ID from a base ID and the token class. */
    inline static sal_uInt8 GetTokenId( sal_uInt8 nBaseId, sal_uInt8 nTokenClass );

    /** Returns the token class of the passed token ID. */
    inline static sal_uInt8 GetTokenClass( sal_uInt8 nTokenId ) { return nTokenId & EXC_TOKCLASS_MASK; }
    /** Changes the token class in the passed classified token ID. */
    inline static void  ChangeTokenClass( sal_uInt8& rnTokenId, sal_uInt8 nTokenClass );

    // strings and string lists -----------------------------------------------

    /** Tries to extract a string from the passed token.
        @param rString  (out-parameter) The string contained in the token.
        @return  true = Passed token is a string token, rString parameter is valid. */
    static bool         GetTokenString( String& rString, const ::formula::FormulaToken& rScToken );

    /** Parses the passed formula and tries to find a single string token, i.e. "abc".
        @param rString  (out-parameter) The string contained in the formula.
        @return  true = String token found, rString parameter is valid. */
    static bool         GetString( String& rString, const ScTokenArray& rScTokArr );

    /** Parses the passed formula and tries to find a string token list, i.e. "abc";"def";"ghi".
        @descr  Returns the unquoted (!) strings in a single string, separated with the
        passed character. If a comma is specified, the function will return abc,def,ghi from
        the example above.
        @param rStringList  (out-parameter) All strings contained in the formula as list.
        @param cSep  List separator character.
        @return  true = String token list found, rString parameter is valid. */
    static bool         GetStringList( String& rStringList, const ScTokenArray& rScTokArr, sal_Unicode cSep );

    /** Tries to convert a formula that consists of a single string token to a list of strings.
        @descr  Example: The formula ="abc\ndef\nghi" will be converted to the formula
        ="abc";"def";"ghi", if the LF character is specified as separator.
        @param rScTokArr  (in/out-parameter) The token array to modify.
        @param cStringSep  The separator in the source string.
        @param bTrimLeadingSpaces  true = remove leading spaces from each token. */
    static void         ConvertStringToList( ScTokenArray& rScTokArr, sal_Unicode cStringSep, bool bTrimLeadingSpaces );

    // multiple operations ----------------------------------------------------

    /** Parses the passed formula and tries to extract references of a multiple operation.
        @descr  Requires that the formula contains a single MULTIPLE.OPERATION function call.
        Spaces in the formula are silently ignored.
        @return  true = Multiple operation found, and all references successfully extracted. */
    static bool GetMultipleOpRefs( XclMultipleOpRefs& rRefs, const ScTokenArray& rScTokArr, const ScAddress& rScPos );
};

// ----------------------------------------------------------------------------

inline sal_uInt8 XclTokenArrayHelper::GetTokenId( sal_uInt8 nBaseId, sal_uInt8 nTokenClass )
{
    OSL_ENSURE( !::get_flag( nBaseId, static_cast< sal_uInt8 >( ~EXC_TOKID_MASK ) ), "XclTokenArrayHelper::GetTokenId - invalid token ID" );
    OSL_ENSURE( !::get_flag( nTokenClass, static_cast< sal_uInt8 >( ~EXC_TOKCLASS_MASK ) ), "XclTokenArrayHelper::GetTokenId - invalid token class" );
    return nBaseId | nTokenClass;
}

inline void XclTokenArrayHelper::ChangeTokenClass( sal_uInt8& rnTokenId, sal_uInt8 nTokenClass )
{
    OSL_ENSURE( !::get_flag( nTokenClass, static_cast< sal_uInt8 >( ~EXC_TOKCLASS_MASK ) ), "XclTokenArrayHelper::ChangeTokenClass - invalid token class" );
    ::set_flag( rnTokenId, EXC_TOKCLASS_MASK, false );
    ::set_flag( rnTokenId, nTokenClass );
}

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
