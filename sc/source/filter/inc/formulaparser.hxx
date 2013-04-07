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

#ifndef OOX_XLS_FORMULAPARSER_HXX
#define OOX_XLS_FORMULAPARSER_HXX

#include "formulabase.hxx"

namespace oox {
namespace xls {

// formula finalizer ==========================================================

/** A generic formula token array finalizer.

    After building a formula token array from alien binary file formats, or
    parsing an XML formula string using the com.sun.star.sheet.FormulaParser
    service, the token array is still not ready to be put into the spreadsheet
    document. There may be functions with a wrong number of parameters (missing
    but required parameters, or unsupported parameters) or intermediate tokens
    used to encode references to macro functions or add-in functions. This
    helper processes a passed token array and builds a new compatible token
    array.

    Derived classes may add more functionality by overwriting the virtual
    functions.
 */
class FormulaFinalizer : public OpCodeProvider, protected ApiOpCodes
{
public:
    explicit            FormulaFinalizer( const OpCodeProvider& rOpCodeProv );

    /** Finalizes and returns the passed token array. */
    ApiTokenSequence    finalizeTokenArray( const ApiTokenSequence& rTokens );

protected:
    /** Derived classed may try to find a function info struct from the passed
        string extracted from an OPCODE_BAD token.

        @param rTokenData  The string that has been found in an OPCODE_BAD
            token preceding the function parentheses.
     */
    virtual const FunctionInfo* resolveBadFuncName( const OUString& rTokenData ) const;

    /** Derived classed may try to find the name of a defined name with the
        passed index extracted from an OPCODE_NAME token.

        @param nTokenIndex  The index of the defined name that has been found
            in an OPCODE_NAME token preceding the function parentheses.
     */
    virtual OUString resolveDefinedName( sal_Int32 nTokenIndex ) const;

private:
    typedef ::std::vector< const ApiToken* > ParameterPosVector;

    const FunctionInfo* getFunctionInfo( ApiToken& orFuncToken );
    const FunctionInfo* getExternCallInfo( ApiToken& orFuncToken, const ApiToken& rECToken );

    void                processTokens( const ApiToken* pToken, const ApiToken* pTokenEnd );
    const ApiToken*     processParameters( const FunctionInfo& rFuncInfo, const ApiToken* pToken, const ApiToken* pTokenEnd );

    bool                isEmptyParameter( const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    const ApiToken*     getSingleToken( const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    const ApiToken*     skipParentheses( const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    const ApiToken*     findParameters( ParameterPosVector& rParams, const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    void                appendEmptyParameter( const FunctionInfo& rFuncInfo, size_t nParam );
    void                appendCalcOnlyParameter( const FunctionInfo& rFuncInfo, size_t nParam );
    void                appendRequiredParameters( const FunctionInfo& rFuncInfo, size_t nParamCount );

    bool                appendFinalToken( const ApiToken& rToken );

private:
    ApiTokenVector      maTokens;
};

// ============================================================================

class FormulaParserImpl;

/** Import formula parser for OOXML and BIFF filters.

    This class implements formula import for the OOXML and BIFF filter. One
    instance is contained in the global filter data to prevent construction and
    destruction of internal buffers for every imported formula.
 */
class FormulaParser : public FormulaProcessorBase
{
public:
    explicit            FormulaParser( const WorkbookHelper& rHelper );
    virtual             ~FormulaParser();

    /** Converts an OOXML formula string. */
    ApiTokenSequence    importFormula(
                            const ::com::sun::star::table::CellAddress& rBaseAddr,
                            const OUString& rFormulaString ) const;

    /** Imports and converts a BIFF12 token array from the passed stream. */
    ApiTokenSequence    importFormula(
                            const ::com::sun::star::table::CellAddress& rBaseAddr,
                            FormulaType eType,
                            SequenceInputStream& rStrm ) const;

    /** Imports and converts a BIFF2-BIFF8 token array from the passed stream.
        @param pnFmlaSize  Size of the token array. If null is passed, reads
        it from stream (1 byte in BIFF2, 2 bytes otherwise) first. */
    ApiTokenSequence    importFormula(
                            const ::com::sun::star::table::CellAddress& rBaseAddr,
                            FormulaType eType,
                            BiffInputStream& rStrm,
                            const sal_uInt16* pnFmlaSize = 0 ) const;

    /** Converts the passed Boolean value to a similar formula. */
    ApiTokenSequence    convertBoolToFormula( bool bValue ) const;

    /** Converts the passed BIFF error code to a similar formula. */
    ApiTokenSequence    convertErrorToFormula( sal_uInt8 nErrorCode ) const;

    /** Converts the passed token index of a defined name to a formula calling that name. */
    ApiTokenSequence    convertNameToFormula( sal_Int32 nTokenIndex ) const;

    /** Converts the passed XML formula to an OLE link target. */
    OUString     importOleTargetLink( const OUString& rFormulaString );

    /** Imports and converts an OLE link target from the passed stream. */
    OUString     importOleTargetLink( SequenceInputStream& rStrm );

    /** Converts the passed formula to a macro name for a drawing shape. */
    OUString     importMacroName( const OUString& rFormulaString );

private:
    ::std::auto_ptr< FormulaParserImpl > mxImpl;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
