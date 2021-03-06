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

#pragma once

#include <memory>
#include "formulabase.hxx"

namespace oox::xls {

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
    void                appendCalcOnlyParameter( const FunctionInfo& rFuncInfo, size_t nParam, size_t nParamCount );
    void                appendRequiredParameters( const FunctionInfo& rFuncInfo, size_t nParamCount );

    bool                appendFinalToken( const ApiToken& rToken );

private:
    ApiTokenVector      maTokens;
};

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
    virtual             ~FormulaParser() override;

    /** Converts an OOXML formula string. */
    ApiTokenSequence    importFormula(
                            const ScAddress& rBaseAddr,
                            const OUString& rFormulaString ) const;

    /** Imports and converts a BIFF12 token array from the passed stream. */
    ApiTokenSequence    importFormula(
                            const ScAddress& rBaseAddr,
                            FormulaType eType,
                            SequenceInputStream& rStrm ) const;

    /** Converts the passed XML formula to an OLE link target. */
    OUString     importOleTargetLink( const OUString& rFormulaString );

    /** Imports and converts an OLE link target from the passed stream. */
    OUString     importOleTargetLink( SequenceInputStream& rStrm );

    /** Converts the passed formula to a macro name for a drawing shape. */
    OUString     importMacroName( const OUString& rFormulaString );

private:
    ::std::unique_ptr< FormulaParserImpl > mxImpl;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
