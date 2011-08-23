/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef OOX_XLS_FORMULAPARSER_HXX
#define OOX_XLS_FORMULAPARSER_HXX

#include "oox/xls/formulabase.hxx"

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
    virtual const FunctionInfo* resolveBadFuncName( const ::rtl::OUString& rTokenData ) const;

    /** Derived classed may try to find the name of a defined name with the
        passed index extracted from an OPCODE_NAME token.

        @param nTokenIndex  The index of the defined name that has been found
            in an OPCODE_NAME token preceding the function parentheses.
     */
    virtual ::rtl::OUString resolveDefinedName( sal_Int32 nTokenIndex ) const;

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
    void                appendCalcOnlyParameter( const FunctionInfo& rFuncInfo, size_t nParam );
    void                appendRequiredParameters( const FunctionInfo& rFuncInfo, size_t nParamCount );

    bool                appendFinalToken( const ApiToken& rToken );

private:
    ApiTokenVector      maTokens;
};

// ============================================================================

class FormulaParserImpl;

/** Import formula parser for OOX and BIFF filters.

    This class implements formula import for the OOX and BIFF filter. One
    instance is contained in the global filter data to prevent construction and
    destruction of internal buffers for every imported formula.
 */
class FormulaParser : public FormulaProcessorBase
{
public:
    explicit            FormulaParser( const WorkbookHelper& rHelper );
    virtual             ~FormulaParser();

    /** Converts an XML formula string. */
    void                importFormula(
                            FormulaContext& rContext,
                            const ::rtl::OUString& rFormulaString ) const;

    /** Imports and converts a OOBIN token array from the passed stream. */
    void                importFormula(
                            FormulaContext& rContext,
                            RecordInputStream& rStrm ) const;

    /** Imports and converts a BIFF token array from the passed stream.
        @param pnFmlaSize  Size of the token array. If null is passed, reads
        it from stream (1 byte in BIFF2, 2 bytes otherwise) first. */
    void                importFormula(
                            FormulaContext& rContext,
                            BiffInputStream& rStrm,
                            const sal_uInt16* pnFmlaSize = 0 ) const;

    /** Converts the passed BIFF error code to a similar formula. */
    void                convertErrorToFormula(
                            FormulaContext& rContext,
                            sal_uInt8 nErrorCode ) const;

    /** Converts the passed token index of a defined name to a formula calling that name. */
    void                convertNameToFormula(
                            FormulaContext& rContext,
                            sal_Int32 nTokenIndex ) const;

    /** Converts the passed number into a HYPERLINK formula with the passed URL. */
    void                convertNumberToHyperlink(
                            FormulaContext& rContext,
                            const ::rtl::OUString& rUrl,
                            double fValue ) const;

    /** Converts the passed XML formula to an OLE link target. */
    ::rtl::OUString     importOleTargetLink( const ::rtl::OUString& rFormulaString );

    /** Imports and converts an OLE link target from the passed stream. */
    ::rtl::OUString     importOleTargetLink( RecordInputStream& rStrm );

    /** Imports and converts an OLE link target from the passed stream. */
    ::rtl::OUString     importOleTargetLink(
                            BiffInputStream& rStrm,
                            const sal_uInt16* pnFmlaSize = 0 ) const;

private:
    ::std::auto_ptr< FormulaParserImpl > mxImpl;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
