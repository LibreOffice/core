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

#include "formulaparser.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/ExternalReference.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/NameToken.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <osl/diagnose.h>
#include <oox/core/filterbase.hxx>
#include <oox/token/properties.hxx>
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "defnamesbuffer.hxx"
#include "externallinkbuffer.hxx"
#include "tablebuffer.hxx"
#include "worksheethelper.hxx"

namespace oox {
namespace xls {

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::sheet::ReferenceFlags;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

// formula finalizer ==========================================================

FormulaFinalizer::FormulaFinalizer( const OpCodeProvider& rOpCodeProv ) :
    OpCodeProvider( rOpCodeProv ),
    ApiOpCodes( getOpCodes() )
{
    maTokens.reserve( 0x2000 );
}

ApiTokenSequence FormulaFinalizer::finalizeTokenArray( const ApiTokenSequence& rTokens )
{
    maTokens.clear();
    if( rTokens.hasElements() )
    {
        const ApiToken* pToken = rTokens.getConstArray();
        processTokens( pToken, pToken + rTokens.getLength() );
    }
    return maTokens.toSequence();
}

const FunctionInfo* FormulaFinalizer::resolveBadFuncName( const OUString& ) const
{
    return nullptr;
}

OUString FormulaFinalizer::resolveDefinedName( sal_Int32 ) const
{
    return OUString();
}

const FunctionInfo* FormulaFinalizer::getFunctionInfo( ApiToken& orFuncToken )
{
    // first, try to find a regular function info from token op-code
    if( const FunctionInfo* pRegFuncInfo = getFuncInfoFromApiToken( orFuncToken ) )
        return pRegFuncInfo;

    // try to recognize a function from an external library
    if( (orFuncToken.OpCode == OPCODE_BAD) && orFuncToken.Data.has< OUString >() )
    {
        // virtual call to resolveBadFuncName()
        if( const FunctionInfo* pLibFuncInfo = resolveBadFuncName( orFuncToken.Data.get< OUString >() ) )
        {
            // write function op-code to the OPCODE_BAD token
            orFuncToken.OpCode = pLibFuncInfo->mnApiOpCode;
            // if it is an external function, insert programmatic function name
            if( (orFuncToken.OpCode == OPCODE_EXTERNAL) && (!pLibFuncInfo->maExtProgName.isEmpty()) )
                orFuncToken.Data <<= pLibFuncInfo->maExtProgName;
            else
                orFuncToken.Data.clear();   // clear string from OPCODE_BAD
            return pLibFuncInfo;
        }
    }

    // no success - return null
    return nullptr;
}

const FunctionInfo* FormulaFinalizer::getExternCallInfo( ApiToken& orFuncToken, const ApiToken& rECToken )
{
    // try to resolve the passed token to a supported sheet function
    if( const FunctionInfo* pFuncInfo = getFuncInfoFromApiToken( rECToken ) )
    {
        orFuncToken.OpCode = pFuncInfo->mnApiOpCode;
        // programmatic add-in function name
        if( (pFuncInfo->mnApiOpCode == OPCODE_EXTERNAL) && !pFuncInfo->maExtProgName.isEmpty() )
            orFuncToken.Data <<= pFuncInfo->maExtProgName;
        // name of unsupported function, convert to OPCODE_BAD to preserve the name
        else if( (pFuncInfo->mnApiOpCode == OPCODE_BAD) && !pFuncInfo->maOoxFuncName.isEmpty() )
            orFuncToken.Data <<= pFuncInfo->maOoxFuncName;
        return pFuncInfo;
    }

    // macro call or unknown function name, move data to function token
    if( (rECToken.OpCode == OPCODE_MACRO) || (rECToken.OpCode == OPCODE_BAD) )
        orFuncToken = rECToken;

    // defined name used as function call, convert to OPCODE_BAD to preserve the name
    if( (rECToken.OpCode == OPCODE_NAME) && rECToken.Data.has< sal_Int32 >() )
    {
        OUString aDefName = resolveDefinedName( rECToken.Data.get< sal_Int32 >() );
        if( !aDefName.isEmpty() )
        {
            orFuncToken.OpCode = OPCODE_BAD;
            orFuncToken.Data <<= aDefName;
        }
    }

    return nullptr;
}

void FormulaFinalizer::processTokens( const ApiToken* pToken, const ApiToken* pTokenEnd )
{
    while( pToken < pTokenEnd )
    {
        // push the current token into the vector
        bool bValid = appendFinalToken( *pToken );
        // try to process a function
        if( const FunctionInfo* pFuncInfo = bValid ? getFunctionInfo( maTokens.back() ) : nullptr )
            pToken = processParameters( *pFuncInfo, pToken + 1, pTokenEnd );
        // otherwise, go to next token
        else
            ++pToken;
    }
}

const ApiToken* FormulaFinalizer::processParameters(
        const FunctionInfo& rFuncInfo, const ApiToken* pToken, const ApiToken* pTokenEnd )
{
    // remember position of the token containing the function op-code
    size_t nFuncNameIdx = maTokens.size() - 1;

    // process a function, if an OPCODE_OPEN token is following
    OSL_ENSURE( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_OPEN), "FormulaFinalizer::processParameters - OPCODE_OPEN expected" );
    if( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_OPEN) )
    {
        // append the OPCODE_OPEN token to the vector
        maTokens.append( OPCODE_OPEN );

        // store positions of OPCODE_OPEN, parameter separators, and OPCODE_CLOSE
        ParameterPosVector aParams;
        pToken = findParameters( aParams, pToken, pTokenEnd );
        OSL_ENSURE( aParams.size() >= 2, "FormulaFinalizer::processParameters - missing tokens" );
        size_t nParamCount = aParams.size() - 1;

        if( (nParamCount == 1) && isEmptyParameter( aParams[ 0 ] + 1, aParams[ 1 ] ) )
        {
            /*  Empty pair of parentheses -> function call without parameters,
                process parameter, there might be spaces between parentheses. */
            processTokens( aParams[ 0 ] + 1, aParams[ 1 ] );
        }
        else
        {
            const FunctionInfo* pRealFuncInfo = &rFuncInfo;
            ParameterPosVector::const_iterator aPosIt = aParams.begin();

            /*  Preprocess EXTERN.CALL functions. The actual function name is
                contained as reference to a defined name in the first (hidden)
                parameter. */
            if( rFuncInfo.mnBiffFuncId == BIFF_FUNC_EXTERNCALL )
            {
                ApiToken& rFuncToken = maTokens[ nFuncNameIdx ];
                rFuncToken.OpCode = OPCODE_NONAME;

                // try to initialize function token from first parameter
                if( const ApiToken* pECToken = getSingleToken( *aPosIt + 1, *(aPosIt + 1) ) )
                    if( const FunctionInfo* pECFuncInfo = getExternCallInfo( rFuncToken, *pECToken ) )
                        pRealFuncInfo = pECFuncInfo;

                /*  On success (something has been inserted into rFuncToken),
                    skip the first parameter. */
                if( rFuncToken.OpCode != OPCODE_NONAME )
                {
                    --nParamCount;
                    ++aPosIt;
                }
            }

            // process all parameters
            FunctionParamInfoIterator aParamInfoIt( *pRealFuncInfo );
            size_t nLastValidSize = maTokens.size();
            size_t nLastValidCount = 0;
            for( size_t nParam = 0; nParam < nParamCount; ++nParam, ++aPosIt, ++aParamInfoIt )
            {
                // add embedded Calc-only parameters
                if( aParamInfoIt.isCalcOnlyParam() )
                {
                    appendCalcOnlyParameter( *pRealFuncInfo, nParam, nParamCount );
                    while( aParamInfoIt.isCalcOnlyParam() ) ++aParamInfoIt;
                }

                const ApiToken* pParamBegin = *aPosIt + 1;
                const ApiToken* pParamEnd = *(aPosIt + 1);
                bool bIsEmpty = isEmptyParameter( pParamBegin, pParamEnd );

                if( !aParamInfoIt.isExcelOnlyParam() )
                {
                    // handle empty parameters
                    if( bIsEmpty )
                    {
                        // append leading space tokens from original token array
                        while( (pParamBegin < pParamEnd) && (pParamBegin->OpCode == OPCODE_SPACES) )
                            maTokens.push_back( *pParamBegin++ );
                        // add default values for some empty parameters, or the OPCODE_MISSING token
                        appendEmptyParameter( *pRealFuncInfo, nParam );
                        // reset bIsEmpty flag, if something has been appended in appendEmptyParameter()
                        bIsEmpty = maTokens.back().OpCode == OPCODE_MISSING;
                        // skip OPCODE_MISSING token in the original token array
                        OSL_ENSURE( (pParamBegin == pParamEnd) || (pParamBegin->OpCode == OPCODE_MISSING), "FormulaFinalizer::processParameters - OPCODE_MISSING expected" );
                        if( pParamBegin < pParamEnd ) ++pParamBegin;
                        // append trailing space tokens from original token array
                        while( (pParamBegin < pParamEnd) && (pParamBegin->OpCode == OPCODE_SPACES) )
                            maTokens.push_back( *pParamBegin++ );
                    }
                    else
                    {
                        // if parameter is not empty, process all tokens of the parameter
                        processTokens( pParamBegin, pParamEnd );
                    }

                    // append parameter separator token
                    maTokens.append( OPCODE_SEP );
                }

                /*  #84453# Update size of new token sequence with valid parameters
                    to be able to remove trailing optional empty parameters. */
                if( !bIsEmpty || (nParam < pRealFuncInfo->mnMinParamCount) )
                {
                    nLastValidSize = maTokens.size();
                    nLastValidCount = nParam + 1;
                }
            }

            // #84453# remove trailing optional empty parameters
            maTokens.resize( nLastValidSize );

            // add trailing Calc-only parameters
            if( aParamInfoIt.isCalcOnlyParam() )
                appendCalcOnlyParameter( *pRealFuncInfo, nLastValidCount, nParamCount );

            // add optional parameters that are required in Calc
            appendRequiredParameters( *pRealFuncInfo, nLastValidCount );

            // remove last parameter separator token
            if( maTokens.back().OpCode == OPCODE_SEP )
                maTokens.pop_back();
        }

        /*  Append the OPCODE_CLOSE token to the vector, but only if there is
            no OPCODE_BAD token at the end, this token already contains the
            trailing closing parentheses. */
        if( (pTokenEnd - 1)->OpCode != OPCODE_BAD )
            maTokens.append( OPCODE_CLOSE );
    }

    /*  Replace OPCODE_EXTERNAL with OPCODE_NONAME to get #NAME! error in cell,
        if no matching add-in function was found. */
    ApiToken& rFuncNameToken = maTokens[ nFuncNameIdx ];
    if( (rFuncNameToken.OpCode == OPCODE_EXTERNAL) && !rFuncNameToken.Data.hasValue() )
        rFuncNameToken.OpCode = OPCODE_NONAME;

    return pToken;
}

bool FormulaFinalizer::isEmptyParameter( const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    while( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_SPACES) ) ++pToken;
    if( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_MISSING) ) ++pToken;
    while( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_SPACES) ) ++pToken;
    return pToken == pTokenEnd;
}

const ApiToken* FormulaFinalizer::getSingleToken( const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    const ApiToken* pSingleToken = nullptr;
    // skip leading whitespace tokens
    while( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_SPACES) ) ++pToken;
    // remember first non-whitespace token
    if( pToken < pTokenEnd ) pSingleToken = pToken++;
    // skip trailing whitespace tokens
    while( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_SPACES) ) ++pToken;
    // return null, if other non-whitespace tokens follow
    return (pToken == pTokenEnd) ? pSingleToken : nullptr;
}

const ApiToken* FormulaFinalizer::skipParentheses( const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    // skip tokens between OPCODE_OPEN and OPCODE_CLOSE
    OSL_ENSURE( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_OPEN), "skipParentheses - OPCODE_OPEN expected" );
    ++pToken;
    while( (pToken < pTokenEnd) && (pToken->OpCode != OPCODE_CLOSE) )
    {
        if( pToken->OpCode == OPCODE_OPEN )
            pToken = skipParentheses( pToken, pTokenEnd );
        else
            ++pToken;
    }
    // skip the OPCODE_CLOSE token
    OSL_ENSURE( ((pToken < pTokenEnd) && (pToken->OpCode == OPCODE_CLOSE)) || ((pTokenEnd - 1)->OpCode == OPCODE_BAD), "skipParentheses - OPCODE_CLOSE expected" );
    return (pToken < pTokenEnd) ? (pToken + 1) : pTokenEnd;
}

const ApiToken* FormulaFinalizer::findParameters( ParameterPosVector& rParams,
        const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    // push position of OPCODE_OPEN
    OSL_ENSURE( (pToken < pTokenEnd) && (pToken->OpCode == OPCODE_OPEN), "FormulaFinalizer::findParameters - OPCODE_OPEN expected" );
    rParams.push_back( pToken++ );

    // find positions of parameter separators
    while( (pToken < pTokenEnd) && (pToken->OpCode != OPCODE_CLOSE) )
    {
        if( pToken->OpCode == OPCODE_OPEN )
            pToken = skipParentheses( pToken, pTokenEnd );
        else if( pToken->OpCode == OPCODE_SEP )
            rParams.push_back( pToken++ );
        else
            ++pToken;
    }

    // push position of OPCODE_CLOSE
    OSL_ENSURE( ((pToken < pTokenEnd) && (pToken->OpCode == OPCODE_CLOSE)) || ((pTokenEnd - 1)->OpCode == OPCODE_BAD), "FormulaFinalizer::findParameters - OPCODE_CLOSE expected" );
    rParams.push_back( pToken );
    return (pToken < pTokenEnd) ? (pToken + 1) : pTokenEnd;
}

void FormulaFinalizer::appendEmptyParameter( const FunctionInfo& rFuncInfo, size_t nParam )
{
    // remember old size of the token array
    size_t nTokenArraySize = maTokens.size();

    switch( rFuncInfo.mnBiff12FuncId )
    {
        case BIFF_FUNC_IF:
            if( (nParam == 1) || (nParam == 2) )
                maTokens.append< double >( OPCODE_PUSH, 0.0 );
        break;
        default:;
    }

    // if no token has been added, append a OPCODE_MISSING token
    if( nTokenArraySize == maTokens.size() )
        maTokens.append( OPCODE_MISSING );
}

void FormulaFinalizer::appendCalcOnlyParameter( const FunctionInfo& rFuncInfo, size_t nParam, size_t nParamCount )
{
    switch( rFuncInfo.mnBiff12FuncId )
    {
        case BIFF_FUNC_FLOOR:
        case BIFF_FUNC_CEILING:
            if (nParam == 2 && nParamCount < 3)
            {
                maTokens.append< double >( OPCODE_PUSH, 1.0 );
                maTokens.append( OPCODE_SEP );
            }
        break;
    }
}

void FormulaFinalizer::appendRequiredParameters( const FunctionInfo& rFuncInfo, size_t nParamCount )
{
    switch( rFuncInfo.mnBiff12FuncId )
    {
        case BIFF_FUNC_WEEKNUM:
            if( nParamCount == 1 )
            {
                maTokens.append< double >( OPCODE_PUSH, 1.0 );
                maTokens.append( OPCODE_SEP );
            }
        break;
    }
}

bool FormulaFinalizer::appendFinalToken( const ApiToken& rToken )
{
    // replace OPCODE_MACRO without macro name with #NAME? error code
    bool bValid = (rToken.OpCode != OPCODE_MACRO) || rToken.Data.hasValue();
    if( bValid )
    {
        maTokens.push_back( rToken );
    }
    else
    {
        maTokens.append( OPCODE_ARRAY_OPEN );
        maTokens.append( OPCODE_PUSH, BiffHelper::calcDoubleFromError( BIFF_ERR_NAME ) );
        maTokens.append( OPCODE_ARRAY_CLOSE );
    }
    return bValid;
}

// parser implementation base =================================================

class FormulaParserImpl : public FormulaFinalizer, public WorkbookHelper
{
public:
    explicit            FormulaParserImpl( const FormulaParser& rParent );

    /** Converts an OOXML formula string. */
    virtual ApiTokenSequence importOoxFormula(
                            const CellAddress& rBaseAddress,
                            const OUString& rFormulaString );

    /** Imports and converts a BIFF12 token array from the passed stream. */
    virtual ApiTokenSequence importBiff12Formula(
                            const CellAddress& rBaseAddress,
                            FormulaType eType,
                            SequenceInputStream& rStrm );

    /** Tries to resolve the passed ref-id to an OLE target URL. */
    OUString            resolveOleTarget( sal_Int32 nRefId, bool bUseRefSheets ) const;

protected:
    typedef ::std::pair< sal_Int32, bool >  WhiteSpace;
    typedef ::std::vector< WhiteSpace >     WhiteSpaceVec;

    /** Initializes the formula parser before importing a formula. */
    void                initializeImport( const CellAddress& rBaseAddress, FormulaType eType );
    /** Finalizes the internal token storage after import. */
    ApiTokenSequence    finalizeImport();

    // token array ------------------------------------------------------------

    bool                resetSpaces();
    static void         appendSpaces( WhiteSpaceVec& orSpaces, sal_Int32 nCount, bool bLineFeed );
    void                appendLeadingSpaces( sal_Int32 nCount, bool bLineFeed );
    void                appendOpeningSpaces( sal_Int32 nCount, bool bLineFeed );
    void                appendClosingSpaces( sal_Int32 nCount, bool bLineFeed );

    size_t              getFormulaSize() const;
    Any&                appendRawToken( sal_Int32 nOpCode );
    Any&                insertRawToken( sal_Int32 nOpCode, size_t nIndexFromEnd );
    size_t              appendWhiteSpaceTokens( const WhiteSpaceVec* pSpaces );
    size_t              insertWhiteSpaceTokens( const WhiteSpaceVec* pSpaces, size_t nIndexFromEnd );

    size_t              getOperandSize( size_t nOpCountFromEnd, size_t nOpIndex ) const;
    void                pushOperandSize( size_t nSize );
    size_t              popOperandSize();

    ApiToken&           getOperandToken( size_t nOpCountFromEnd, size_t nOpIndex, size_t nTokenIndex );

    bool                pushOperandToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces = nullptr );
    bool                pushAnyOperandToken( const Any& rAny, sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces = nullptr );
    template< typename Type >
    bool                pushValueOperandToken( const Type& rValue, sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces = nullptr );
    template< typename Type >
    inline bool         pushValueOperandToken( const Type& rValue, const WhiteSpaceVec* pSpaces = nullptr )
                            { return pushValueOperandToken( rValue, OPCODE_PUSH, pSpaces ); }
    bool                pushParenthesesOperandToken( const WhiteSpaceVec* pOpeningSpaces = nullptr, const WhiteSpaceVec* pClosingSpaces = nullptr );
    bool                pushUnaryPreOperatorToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces = nullptr );
    bool                pushUnaryPostOperatorToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces = nullptr );
    bool                pushBinaryOperatorToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces = nullptr );
    bool                pushParenthesesOperatorToken( const WhiteSpaceVec* pOpeningSpaces = nullptr, const WhiteSpaceVec* pClosingSpaces = nullptr );
    bool                pushFunctionOperatorToken( sal_Int32 nOpCode, size_t nParamCount, const WhiteSpaceVec* pLeadingSpaces = nullptr, const WhiteSpaceVec* pClosingSpaces = nullptr );
    bool                pushFunctionOperatorToken( const FunctionInfo& rFuncInfo, size_t nParamCount, const WhiteSpaceVec* pLeadingSpaces = nullptr, const WhiteSpaceVec* pClosingSpaces = nullptr );

    bool                pushOperand( sal_Int32 nOpCode );
    bool                pushAnyOperand( const Any& rAny, sal_Int32 nOpCode );
    template< typename Type >
    bool                pushValueOperand( const Type& rValue, sal_Int32 nOpCode );
    template< typename Type >
    inline bool         pushValueOperand( const Type& rValue )
                            { return pushValueOperand( rValue, OPCODE_PUSH ); }
    bool                pushBoolOperand( bool bValue );
    bool                pushErrorOperand( double fEncodedError );
    bool                pushBiffBoolOperand( sal_uInt8 nValue );
    bool                pushBiffErrorOperand( sal_uInt8 nErrorCode );
    bool                pushReferenceOperand( const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushReferenceOperand( const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    template< typename Type >
    bool                pushReferenceOperand( const LinkSheetRange& rSheetRange, const Type& rApiRef );
    bool                pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushNlrOperand( const BinSingleRef2d& rRef );
    bool                pushEmbeddedRefOperand( const DefinedNameBase& rName, bool bPushBadToken );
    bool                pushDefinedNameOperand( const DefinedNameRef& rxDefName );
    bool                pushExternalFuncOperand( const FunctionInfo& rFuncInfo );
    bool                pushDdeLinkOperand( const OUString& rDdeServer, const OUString& rDdeTopic, const OUString& rDdeItem );
    bool                pushExternalNameOperand( const ExternalNameRef& rxExtName, const ExternalLink& rExtLink );
    bool                pushSpecialTokenOperand( const BinAddress& rBaseAddr, bool bTable );

    bool                pushUnaryPreOperator( sal_Int32 nOpCode );
    bool                pushUnaryPostOperator( sal_Int32 nOpCode );
    bool                pushBinaryOperator( sal_Int32 nOpCode );
    bool                pushParenthesesOperator();
    bool                pushFunctionOperator( sal_Int32 nOpCode, size_t nParamCount );
    bool                pushFunctionOperator( const FunctionInfo& rFuncInfo, size_t nParamCount );

private:
    // reference conversion ---------------------------------------------------

    void                initReference2d( SingleReference& orApiRef ) const;
    static void         initReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bSameSheet );
    void                convertReference( SingleReference& orApiRef, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference( ComplexReference& orApiRef, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference2d( SingleReference& orApiRef, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference2d( ComplexReference& orApiRef, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bSameSheet, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference3d( ComplexReference& orApiRef, const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const;

private:
    // finalize token sequence ------------------------------------------------

    virtual const FunctionInfo* resolveBadFuncName( const OUString& rTokenData ) const override;
    virtual OUString resolveDefinedName( sal_Int32 nTokenIndex ) const override;

protected:
    const sal_Int32     mnMaxApiCol;                /// Maximum column index in own document.
    const sal_Int32     mnMaxApiRow;                /// Maximum row index in own document.
    const sal_Int32     mnMaxXlsCol;                /// Maximum column index in imported document.
    const sal_Int32     mnMaxXlsRow;                /// Maximum row index in imported document.

    CellAddress         maBaseAddr;                 /// Base address for relative references.
    bool                mbRelativeAsOffset;         /// True = relative row/column index is (signed) offset, false = explicit index.
    bool                mb2dRefsAs3dRefs;           /// True = convert all 2D references to 3D references in sheet specified by base address.
    bool                mbSpecialTokens;            /// True = special handling for tExp and tTbl tokens, false = exit with error.
    bool                mbAllowNulChars;            /// True = keep NUL characters in string tokens.

private:
    typedef ::std::vector< size_t > SizeTypeVector;

    ApiTokenVector      maTokenStorage;             /// Raw unordered token storage.
    SizeTypeVector      maTokenIndexes;             /// Indexes into maTokenStorage.
    SizeTypeVector      maOperandSizeStack;         /// Stack with token sizes per operand.
    WhiteSpaceVec       maLeadingSpaces;            /// List of whitespaces before next token.
    WhiteSpaceVec       maOpeningSpaces;            /// List of whitespaces before opening parenthesis.
    WhiteSpaceVec       maClosingSpaces;            /// List of whitespaces before closing parenthesis.
};

FormulaParserImpl::FormulaParserImpl( const FormulaParser& rParent ) :
    FormulaFinalizer( rParent ),
    WorkbookHelper( rParent ),
    mnMaxApiCol( rParent.getAddressConverter().getMaxApiAddress().Column ),
    mnMaxApiRow( rParent.getAddressConverter().getMaxApiAddress().Row ),
    mnMaxXlsCol( rParent.getAddressConverter().getMaxXlsAddress().Column ),
    mnMaxXlsRow( rParent.getAddressConverter().getMaxXlsAddress().Row ),
    mbRelativeAsOffset( false ),
    mb2dRefsAs3dRefs( false ),
    mbSpecialTokens( false ),
    mbAllowNulChars( false )
{
    // reserve enough space to make resize(), push_back() etc. cheap
    maTokenStorage.reserve( 0x2000 );
    maTokenIndexes.reserve( 0x2000 );
    maOperandSizeStack.reserve( 256 );
    maLeadingSpaces.reserve( 256 );
    maOpeningSpaces.reserve( 256 );
    maClosingSpaces.reserve( 256 );
}

ApiTokenSequence FormulaParserImpl::importOoxFormula( const CellAddress&, const OUString& )
{
    OSL_FAIL( "FormulaParserImpl::importOoxFormula - not implemented" );
    return ApiTokenSequence();
}

ApiTokenSequence FormulaParserImpl::importBiff12Formula( const CellAddress&, FormulaType, SequenceInputStream& )
{
    SAL_WARN("sc", "FormulaParserImpl::importBiff12Formula - not implemented" );
    return ApiTokenSequence();
}

OUString FormulaParserImpl::resolveOleTarget( sal_Int32 nRefId, bool bUseRefSheets ) const
{
    const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId, bUseRefSheets ).get();
    OSL_ENSURE( pExtLink && (pExtLink->getLinkType() == LINKTYPE_OLE), "FormulaParserImpl::resolveOleTarget - missing or wrong link" );
    if( pExtLink && (pExtLink->getLinkType() == LINKTYPE_OLE) )
         return getBaseFilter().getAbsoluteUrl( pExtLink->getTargetUrl() );
    return OUString();
}

void FormulaParserImpl::initializeImport( const CellAddress& rBaseAddr, FormulaType eType )
{
    maBaseAddr = rBaseAddr;
    mbRelativeAsOffset = mb2dRefsAs3dRefs = mbSpecialTokens = mbAllowNulChars = false;
    switch( eType )
    {
        case FORMULATYPE_CELL:
            mbSpecialTokens = true;
        break;
        case FORMULATYPE_ARRAY:
        break;
        case FORMULATYPE_SHAREDFORMULA:
            mbRelativeAsOffset = true;
        break;
        case FORMULATYPE_CONDFORMAT:
            mbRelativeAsOffset = true;
        break;
        case FORMULATYPE_VALIDATION:
            mbRelativeAsOffset = true;
            // enable NUL characters in BIFF import, string list is single tStr token with NUL separators
            mbAllowNulChars = getFilterType() == FILTER_BIFF;
        break;
        case FORMULATYPE_DEFINEDNAME:
            mbRelativeAsOffset = true;
            // BIFF2-BIFF4: convert 2D references to absolute 3D references
            mb2dRefsAs3dRefs = (getFilterType() == FILTER_BIFF) && (getBiff() <= BIFF4);
        break;
    }

    maTokenStorage.clear();
    maTokenIndexes.clear();
    maOperandSizeStack.clear();
}

ApiTokenSequence FormulaParserImpl::finalizeImport()
{
    ApiTokenSequence aTokens( static_cast< sal_Int32 >( maTokenIndexes.size() ) );
    if( aTokens.hasElements() )
    {
        ApiToken* pToken = aTokens.getArray();
        for( SizeTypeVector::const_iterator aIt = maTokenIndexes.begin(), aEnd = maTokenIndexes.end(); aIt != aEnd; ++aIt, ++pToken )
            *pToken = maTokenStorage[ *aIt ];
    }
    return finalizeTokenArray( aTokens );
}

// token array ----------------------------------------------------------------

bool FormulaParserImpl::resetSpaces()
{
    maLeadingSpaces.clear();
    maOpeningSpaces.clear();
    maClosingSpaces.clear();
    return true;
}

void FormulaParserImpl::appendSpaces( WhiteSpaceVec& orSpaces, sal_Int32 nCount, bool bLineFeed )
{
    OSL_ENSURE( nCount >= 0, "FormulaParserImpl::appendSpaces - negative count" );
    if( nCount > 0 )
        orSpaces.push_back( WhiteSpace( nCount, bLineFeed ) );
}

void FormulaParserImpl::appendLeadingSpaces( sal_Int32 nCount, bool bLineFeed )
{
    appendSpaces( maLeadingSpaces, nCount, bLineFeed );
}

void FormulaParserImpl::appendOpeningSpaces( sal_Int32 nCount, bool bLineFeed )
{
    appendSpaces( maOpeningSpaces, nCount, bLineFeed );
}

void FormulaParserImpl::appendClosingSpaces( sal_Int32 nCount, bool bLineFeed )
{
    appendSpaces( maClosingSpaces, nCount, bLineFeed );
}

size_t FormulaParserImpl::getFormulaSize() const
{
    return maTokenIndexes.size();
}

Any& FormulaParserImpl::appendRawToken( sal_Int32 nOpCode )
{
    maTokenIndexes.push_back( maTokenStorage.size() );
    return maTokenStorage.append( nOpCode );
}

Any& FormulaParserImpl::insertRawToken( sal_Int32 nOpCode, size_t nIndexFromEnd )
{
    maTokenIndexes.insert( maTokenIndexes.end() - nIndexFromEnd, maTokenStorage.size() );
    return maTokenStorage.append( nOpCode );
}

size_t FormulaParserImpl::appendWhiteSpaceTokens( const WhiteSpaceVec* pSpaces )
{
    if( pSpaces && !pSpaces->empty() )
        for( WhiteSpaceVec::const_iterator aIt = pSpaces->begin(), aEnd = pSpaces->end(); aIt != aEnd; ++aIt )
            appendRawToken( OPCODE_SPACES ) <<= aIt->first;
    return pSpaces ? pSpaces->size() : 0;
}

size_t FormulaParserImpl::insertWhiteSpaceTokens( const WhiteSpaceVec* pSpaces, size_t nIndexFromEnd )
{
    if( pSpaces && !pSpaces->empty() )
        for( WhiteSpaceVec::const_iterator aIt = pSpaces->begin(), aEnd = pSpaces->end(); aIt != aEnd; ++aIt )
            insertRawToken( OPCODE_SPACES, nIndexFromEnd ) <<= aIt->first;
    return pSpaces ? pSpaces->size() : 0;
}

size_t FormulaParserImpl::getOperandSize( size_t nOpCountFromEnd, size_t nOpIndex ) const
{
    OSL_ENSURE( (nOpIndex < nOpCountFromEnd) && (nOpCountFromEnd <= maOperandSizeStack.size()),
        "FormulaParserImpl::getOperandSize - invalid parameters" );
    return maOperandSizeStack[ maOperandSizeStack.size() - nOpCountFromEnd + nOpIndex ];
}

void FormulaParserImpl::pushOperandSize( size_t nSize )
{
    maOperandSizeStack.push_back( nSize );
}

size_t FormulaParserImpl::popOperandSize()
{
    OSL_ENSURE( !maOperandSizeStack.empty(), "FormulaParserImpl::popOperandSize - invalid call" );
    size_t nOpSize = maOperandSizeStack.back();
    maOperandSizeStack.pop_back();
    return nOpSize;
}

ApiToken& FormulaParserImpl::getOperandToken( size_t nOpCountFromEnd, size_t nOpIndex, size_t nTokenIndex )
{
    SAL_WARN_IF(
        getOperandSize( nOpCountFromEnd, nOpIndex ) <= nTokenIndex, "sc.filter",
        "FormulaParserImpl::getOperandToken - invalid parameters" );
    SizeTypeVector::const_iterator aIndexIt = maTokenIndexes.end();
    for( SizeTypeVector::const_iterator aEnd = maOperandSizeStack.end(), aIt = aEnd - nOpCountFromEnd + nOpIndex; aIt != aEnd; ++aIt )
        aIndexIt -= *aIt;
    return maTokenStorage[ *(aIndexIt + nTokenIndex) ];
}

bool FormulaParserImpl::pushOperandToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces )
{
    size_t nSpacesSize = appendWhiteSpaceTokens( pSpaces );
    appendRawToken( nOpCode );
    pushOperandSize( nSpacesSize + 1 );
    return true;
}

bool FormulaParserImpl::pushAnyOperandToken( const Any& rAny, sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces )
{
    size_t nSpacesSize = appendWhiteSpaceTokens( pSpaces );
    appendRawToken( nOpCode ) = rAny;
    pushOperandSize( nSpacesSize + 1 );
    return true;
}

template< typename Type >
bool FormulaParserImpl::pushValueOperandToken( const Type& rValue, sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces )
{
    size_t nSpacesSize = appendWhiteSpaceTokens( pSpaces );
    appendRawToken( nOpCode ) <<= rValue;
    pushOperandSize( nSpacesSize + 1 );
    return true;
}

bool FormulaParserImpl::pushParenthesesOperandToken( const WhiteSpaceVec* pOpeningSpaces, const WhiteSpaceVec* pClosingSpaces )
{
    size_t nSpacesSize = appendWhiteSpaceTokens( pOpeningSpaces );
    appendRawToken( OPCODE_OPEN );
    nSpacesSize += appendWhiteSpaceTokens( pClosingSpaces );
    appendRawToken( OPCODE_CLOSE );
    pushOperandSize( nSpacesSize + 2 );
    return true;
}

bool FormulaParserImpl::pushUnaryPreOperatorToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 1;
    if( bOk )
    {
        size_t nOpSize = popOperandSize();
        size_t nSpacesSize = insertWhiteSpaceTokens( pSpaces, nOpSize );
        insertRawToken( nOpCode, nOpSize );
        pushOperandSize( nOpSize + nSpacesSize + 1 );
    }
    return bOk;
}

bool FormulaParserImpl::pushUnaryPostOperatorToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 1;
    if( bOk )
    {
        size_t nOpSize = popOperandSize();
        size_t nSpacesSize = appendWhiteSpaceTokens( pSpaces );
        appendRawToken( nOpCode );
        pushOperandSize( nOpSize + nSpacesSize + 1 );
    }
    return bOk;
}

bool FormulaParserImpl::pushBinaryOperatorToken( sal_Int32 nOpCode, const WhiteSpaceVec* pSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 2;
    if( bOk )
    {
        size_t nOp2Size = popOperandSize();
        size_t nOp1Size = popOperandSize();
        size_t nSpacesSize = insertWhiteSpaceTokens( pSpaces, nOp2Size );
        insertRawToken( nOpCode, nOp2Size );
        pushOperandSize( nOp1Size + nSpacesSize + 1 + nOp2Size );
    }
    return bOk;
}

bool FormulaParserImpl::pushParenthesesOperatorToken( const WhiteSpaceVec* pOpeningSpaces, const WhiteSpaceVec* pClosingSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 1;
    if( bOk )
    {
        size_t nOpSize = popOperandSize();
        size_t nSpacesSize = insertWhiteSpaceTokens( pOpeningSpaces, nOpSize );
        insertRawToken( OPCODE_OPEN, nOpSize );
        nSpacesSize += appendWhiteSpaceTokens( pClosingSpaces );
        appendRawToken( OPCODE_CLOSE );
        pushOperandSize( nOpSize + nSpacesSize + 2 );
    }
    return bOk;
}

bool FormulaParserImpl::pushFunctionOperatorToken( sal_Int32 nOpCode, size_t nParamCount, const WhiteSpaceVec* pLeadingSpaces, const WhiteSpaceVec* pClosingSpaces )
{
    /*  #i70925# if there are not enough tokens available on token stack, do
        not exit with error, but reduce parameter count. */
    nParamCount = ::std::min( maOperandSizeStack.size(), nParamCount );

    // convert all parameters on stack to a single operand separated with OPCODE_SEP
    bool bOk = true;
    for( size_t nParam = 1; bOk && (nParam < nParamCount); ++nParam )
        bOk = pushBinaryOperatorToken( OPCODE_SEP );

    // add function parentheses and function name
    return bOk &&
        ((nParamCount > 0) ? pushParenthesesOperatorToken( nullptr, pClosingSpaces ) : pushParenthesesOperandToken( nullptr, pClosingSpaces )) &&
        pushUnaryPreOperatorToken( nOpCode, pLeadingSpaces );
}

bool FormulaParserImpl::pushFunctionOperatorToken( const FunctionInfo& rFuncInfo, size_t nParamCount, const WhiteSpaceVec* pLeadingSpaces, const WhiteSpaceVec* pClosingSpaces )
{
    bool bOk = pushFunctionOperatorToken( rFuncInfo.mnApiOpCode, nParamCount, pLeadingSpaces, pClosingSpaces );
    if( bOk )
    {
       // create an external add-in call for the passed built-in function
        if( (rFuncInfo.mnApiOpCode == OPCODE_EXTERNAL) && !rFuncInfo.maExtProgName.isEmpty() )
            getOperandToken( 1, 0, 0 ).Data <<= rFuncInfo.maExtProgName;
        // create a bad token with unsupported function name
        else if( (rFuncInfo.mnApiOpCode == OPCODE_BAD) && !rFuncInfo.maOoxFuncName.isEmpty() )
            getOperandToken( 1, 0, 0 ).Data <<= rFuncInfo.maOoxFuncName;
    }
    return bOk;
}

bool FormulaParserImpl::pushOperand( sal_Int32 nOpCode )
{
    return pushOperandToken( nOpCode, &maLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushAnyOperand( const Any& rAny, sal_Int32 nOpCode )
{
    return pushAnyOperandToken( rAny, nOpCode, &maLeadingSpaces ) && resetSpaces();
}

template< typename Type >
bool FormulaParserImpl::pushValueOperand( const Type& rValue, sal_Int32 nOpCode )
{
    return pushValueOperandToken( rValue, nOpCode, &maLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushBoolOperand( bool bValue )
{
    if( const FunctionInfo* pFuncInfo = getFuncInfoFromBiff12FuncId( bValue ? BIFF_FUNC_TRUE : BIFF_FUNC_FALSE ) )
        return pushFunctionOperator( pFuncInfo->mnApiOpCode, 0 );
    return pushValueOperand< double >( bValue ? 1.0 : 0.0 );
}

bool FormulaParserImpl::pushErrorOperand( double fEncodedError )
{
    // HACK: enclose all error codes into an 1x1 matrix
    // start token array with opening brace and leading spaces
    pushOperand( OPCODE_ARRAY_OPEN );
    size_t nOpSize = popOperandSize();
    size_t nOldArraySize = maTokenIndexes.size();
    // push a double containing the Calc error code
    appendRawToken( OPCODE_PUSH ) <<= fEncodedError;
    // close token array and set resulting operand size
    appendRawToken( OPCODE_ARRAY_CLOSE );
    pushOperandSize( nOpSize + maTokenIndexes.size() - nOldArraySize );
    return true;
}

bool FormulaParserImpl::pushBiffBoolOperand( sal_uInt8 nValue )
{
    return pushBoolOperand( nValue != BIFF_TOK_BOOL_FALSE );
}

bool FormulaParserImpl::pushBiffErrorOperand( sal_uInt8 nErrorCode )
{
    return pushErrorOperand( BiffHelper::calcDoubleFromError( nErrorCode ) );
}

bool FormulaParserImpl::pushReferenceOperand( const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    SingleReference aApiRef;
    convertReference2d( aApiRef, rRef, bDeleted, bRelativeAsOffset );
    return pushValueOperand( aApiRef );
}

bool FormulaParserImpl::pushReferenceOperand( const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    ComplexReference aApiRef;
    convertReference2d( aApiRef, rRef.maRef1, rRef.maRef2, bDeleted, bRelativeAsOffset );
    return pushValueOperand( aApiRef );
}

template< typename Type >
bool FormulaParserImpl::pushReferenceOperand( const LinkSheetRange& rSheetRange, const Type& rApiRef )
{
    if( rSheetRange.isExternal() )
    {
        ExternalReference aApiExtRef;
        aApiExtRef.Index = rSheetRange.getDocLinkIndex();
        aApiExtRef.Reference <<= rApiRef;
        return pushValueOperand( aApiExtRef );
    }
    return pushValueOperand( rApiRef );
}

bool FormulaParserImpl::pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    if( rSheetRange.is3dRange() )
    {
        // single-cell-range over several sheets, needs to create a ComplexReference
        ComplexReference aApiRef;
        convertReference3d( aApiRef, rSheetRange, rRef, rRef, bDeleted, bRelativeAsOffset );
        return pushReferenceOperand( rSheetRange, aApiRef );
    }
    SingleReference aApiRef;
    convertReference3d( aApiRef, rSheetRange.getFirstSheet(), rSheetRange.isSameSheet(), rRef, bDeleted, bRelativeAsOffset );
    return pushReferenceOperand( rSheetRange, aApiRef );
}

bool FormulaParserImpl::pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    ComplexReference aApiRef;
    convertReference3d( aApiRef, rSheetRange, rRef.maRef1, rRef.maRef2, bDeleted, bRelativeAsOffset );
    return pushReferenceOperand( rSheetRange, aApiRef );
}

bool FormulaParserImpl::pushNlrOperand( const BinSingleRef2d& rRef )
{
    SingleReference aApiRef;
    convertReference2d( aApiRef, rRef, false, false );
    return pushValueOperand( aApiRef, OPCODE_NLR );
}

bool FormulaParserImpl::pushEmbeddedRefOperand( const DefinedNameBase& rName, bool bPushBadToken )
{
    Any aRefAny = rName.getReference( maBaseAddr );
    if( aRefAny.hasValue() )
        return pushAnyOperand( aRefAny, OPCODE_PUSH );
    if( bPushBadToken && !rName.getModelName().isEmpty() && (rName.getModelName()[ 0 ] >= ' ') )
        return pushValueOperand( rName.getModelName(), OPCODE_BAD );
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool FormulaParserImpl::pushDefinedNameOperand( const DefinedNameRef& rxDefName )
{
    if( !rxDefName || rxDefName->getModelName().isEmpty() )
        return pushBiffErrorOperand( BIFF_ERR_NAME );
    if( rxDefName->isMacroFunction() )
        return pushValueOperand( rxDefName->getModelName(), OPCODE_MACRO );
    if( rxDefName->getTokenIndex() >= 0 )
        return pushValueOperand( rxDefName->getTokenIndex(), OPCODE_NAME );
    return pushEmbeddedRefOperand( *rxDefName, true );
}

bool FormulaParserImpl::pushExternalFuncOperand( const FunctionInfo& rFuncInfo )
{
    return (rFuncInfo.mnApiOpCode == OPCODE_EXTERNAL) ?
        pushValueOperand( rFuncInfo.maExtProgName, OPCODE_EXTERNAL ) :
        pushOperand( rFuncInfo.mnApiOpCode );
}

bool FormulaParserImpl::pushDdeLinkOperand( const OUString& rDdeServer, const OUString& rDdeTopic, const OUString& rDdeItem )
{
    // create the function call DDE("server";"topic";"item")
    return
        pushValueOperandToken( rDdeServer ) &&
        pushValueOperandToken( rDdeTopic ) &&
        pushValueOperandToken( rDdeItem ) &&
        pushFunctionOperator( OPCODE_DDE, 3 );
}

bool FormulaParserImpl::pushExternalNameOperand( const ExternalNameRef& rxExtName, const ExternalLink& rExtLink )
{
    if( rxExtName.get() ) switch( rExtLink.getLinkType() )
    {
        case LINKTYPE_INTERNAL:
        case LINKTYPE_EXTERNAL:
            return pushEmbeddedRefOperand( *rxExtName, false );

        case LINKTYPE_ANALYSIS:
            // TODO: need support for localized addin function names
            if( const FunctionInfo* pFuncInfo = getFuncInfoFromOoxFuncName( rxExtName->getUpcaseModelName() ) )
                return pushExternalFuncOperand( *pFuncInfo );
        break;

        case LINKTYPE_LIBRARY:
            if( const FunctionInfo* pFuncInfo = getFuncInfoFromOoxFuncName( rxExtName->getUpcaseModelName() ) )
                if( (pFuncInfo->meFuncLibType != FUNCLIB_UNKNOWN) && (pFuncInfo->meFuncLibType == rExtLink.getFuncLibraryType()) )
                    return pushExternalFuncOperand( *pFuncInfo );
        break;

        case LINKTYPE_DDE:
        {
            OUString aDdeServer, aDdeTopic, aDdeItem;
            if( rxExtName->getDdeLinkData( aDdeServer, aDdeTopic, aDdeItem ) )
                return pushDdeLinkOperand( aDdeServer, aDdeTopic, aDdeItem );
        }
        break;

        default:
            OSL_ENSURE( rExtLink.getLinkType() != LINKTYPE_SELF, "FormulaParserImpl::pushExternalNameOperand - invalid call" );
    }
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool FormulaParserImpl::pushSpecialTokenOperand( const BinAddress& rBaseAddr, bool bTable )
{
    CellAddress aBaseAddr( maBaseAddr.Sheet, rBaseAddr.mnCol, rBaseAddr.mnRow );
    ApiSpecialTokenInfo aTokenInfo( aBaseAddr, bTable );
    return mbSpecialTokens && (getFormulaSize() == 0) && pushValueOperand( aTokenInfo, OPCODE_BAD );
}

bool FormulaParserImpl::pushUnaryPreOperator( sal_Int32 nOpCode )
{
    return pushUnaryPreOperatorToken( nOpCode, &maLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushUnaryPostOperator( sal_Int32 nOpCode )
{
    return pushUnaryPostOperatorToken( nOpCode, &maLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushBinaryOperator( sal_Int32 nOpCode )
{
    return pushBinaryOperatorToken( nOpCode, &maLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushParenthesesOperator()
{
    return pushParenthesesOperatorToken( &maOpeningSpaces, &maClosingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushFunctionOperator( sal_Int32 nOpCode, size_t nParamCount )
{
    return pushFunctionOperatorToken( nOpCode, nParamCount, &maLeadingSpaces, &maClosingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushFunctionOperator( const FunctionInfo& rFuncInfo, size_t nParamCount )
{
    return pushFunctionOperatorToken( rFuncInfo, nParamCount, &maLeadingSpaces, &maClosingSpaces ) && resetSpaces();
}

// reference conversion -------------------------------------------------------

void FormulaParserImpl::initReference2d( SingleReference& orApiRef ) const
{
    if( mb2dRefsAs3dRefs )
    {
        initReference3d( orApiRef, maBaseAddr.Sheet, false );
    }
    else
    {
        orApiRef.Flags = SHEET_RELATIVE;
        // #i10184# absolute sheet index needed for relative references in shared formulas
        orApiRef.Sheet = maBaseAddr.Sheet;
        orApiRef.RelativeSheet = 0;
    }
}

void FormulaParserImpl::initReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bSameSheet )
{
    orApiRef.Flags = SHEET_3D;
    if( nSheet < 0 )
    {
        orApiRef.Sheet = 0;
        orApiRef.Flags |= SHEET_DELETED;
    }
    else if( bSameSheet )
    {
        OSL_ENSURE( nSheet == 0, "FormulaParserImpl::initReference3d - invalid sheet index" );
        orApiRef.Flags |= SHEET_RELATIVE;
        orApiRef.RelativeSheet = 0;
    }
    else
    {
        orApiRef.Sheet = nSheet;
    }
}

void FormulaParserImpl::convertReference( SingleReference& orApiRef, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const
{
    if( bDeleted )
    {
        orApiRef.Column = 0;
        orApiRef.Row = 0;
        // no explicit information about whether row or column is deleted
        orApiRef.Flags |= COLUMN_DELETED | ROW_DELETED;
    }
    else
    {
        // column/row indexes and flags
        setFlag( orApiRef.Flags, COLUMN_RELATIVE, rRef.mbColRel );
        setFlag( orApiRef.Flags, ROW_RELATIVE, rRef.mbRowRel );
        (rRef.mbColRel ? orApiRef.RelativeColumn : orApiRef.Column) = rRef.mnCol;
        (rRef.mbRowRel ? orApiRef.RelativeRow : orApiRef.Row) = rRef.mnRow;
        // convert absolute indexes to relative offsets used in API
        if( !bRelativeAsOffset )
        {
            if( rRef.mbColRel )
                orApiRef.RelativeColumn -= maBaseAddr.Column;
            if( rRef.mbRowRel )
                orApiRef.RelativeRow -= maBaseAddr.Row;
        }
    }
}

void FormulaParserImpl::convertReference( ComplexReference& orApiRef, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const
{
    convertReference( orApiRef.Reference1, rRef1, bDeleted, bRelativeAsOffset );
    convertReference( orApiRef.Reference2, rRef2, bDeleted, bRelativeAsOffset );
    /*  Handle references to complete rows or columns (e.g. $1:$2 or C:D),
        need to expand or shrink to limits of own document. */
    if( !bDeleted && !rRef1.mbColRel && !rRef2.mbColRel && (orApiRef.Reference1.Column == 0) && (orApiRef.Reference2.Column == mnMaxXlsCol) )
        orApiRef.Reference2.Column = mnMaxApiCol;
    if( !bDeleted && !rRef1.mbRowRel && !rRef2.mbRowRel && (orApiRef.Reference1.Row == 0) && (orApiRef.Reference2.Row == mnMaxXlsRow) )
        orApiRef.Reference2.Row = mnMaxApiRow;
}

void FormulaParserImpl::convertReference2d( SingleReference& orApiRef, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const
{
    initReference2d( orApiRef );
    convertReference( orApiRef, rRef, bDeleted, bRelativeAsOffset );
}

void FormulaParserImpl::convertReference2d( ComplexReference& orApiRef, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const
{
    initReference2d( orApiRef.Reference1 );
    initReference2d( orApiRef.Reference2 );
    convertReference( orApiRef, rRef1, rRef2, bDeleted, bRelativeAsOffset );
    // remove sheet name from second part of reference
    setFlag( orApiRef.Reference2.Flags, SHEET_3D, false );
}

void FormulaParserImpl::convertReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bSameSheet, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const
{
    initReference3d( orApiRef, nSheet, bSameSheet );
    convertReference( orApiRef, rRef, bDeleted, bRelativeAsOffset );
}

void FormulaParserImpl::convertReference3d( ComplexReference& orApiRef, const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const
{
    bool bSameSheet = rSheetRange.isSameSheet();
    initReference3d( orApiRef.Reference1, rSheetRange.getFirstSheet(), bSameSheet );
    initReference3d( orApiRef.Reference2, rSheetRange.getLastSheet(), bSameSheet );
    convertReference( orApiRef, rRef1, rRef2, bDeleted, bRelativeAsOffset );
    // remove sheet name from second part of reference
    setFlag( orApiRef.Reference2.Flags, SHEET_3D, rSheetRange.is3dRange() );
}

// finalize token sequence ----------------------------------------------------

const FunctionInfo* FormulaParserImpl::resolveBadFuncName( const OUString& rTokenData ) const
{
    /*  Try to parse calls to library functions. The format of such a function
        call is "[n]!funcname", n>0 being the link identifier of the function
        library spreadsheet file. */
    sal_Int32 nBracketOpen = rTokenData.indexOf( '[' );
    sal_Int32 nBracketClose = rTokenData.indexOf( ']' );
    sal_Int32 nExclamation = rTokenData.indexOf( '!' );
    if( (0 == nBracketOpen) && (nBracketOpen + 1 < nBracketClose) && (nBracketClose + 1 == nExclamation) && (nExclamation + 1 < rTokenData.getLength()) )
    {
        sal_Int32 nRefId = rTokenData.copy( nBracketOpen + 1, nBracketClose - nBracketOpen - 1 ).toInt32();
        const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId ).get();
        if( pExtLink && (pExtLink->getLinkType() == LINKTYPE_LIBRARY) )
        {
            OUString aFuncName = rTokenData.copy( nExclamation + 1 ).toAsciiUpperCase();
            if( const FunctionInfo* pFuncInfo = getFuncInfoFromOoxFuncName( aFuncName ) )
                if( (pFuncInfo->meFuncLibType != FUNCLIB_UNKNOWN) && (pFuncInfo->meFuncLibType == pExtLink->getFuncLibraryType()) )
                    return pFuncInfo;
        }
    }
    return nullptr;
}

OUString FormulaParserImpl::resolveDefinedName( sal_Int32 nTokenIndex ) const
{
    if( const DefinedName* pDefName = getDefinedNames().getByTokenIndex( nTokenIndex ).get() )
        return pDefName->getCalcName();
    return OUString();
}

// OOXML/BIFF12 parser implementation =========================================

class OoxFormulaParserImpl : public FormulaParserImpl
{
public:
    explicit            OoxFormulaParserImpl( const FormulaParser& rParent );

    virtual ApiTokenSequence importOoxFormula(
                            const CellAddress& rBaseAddr,
                            const OUString& rFormulaString ) override;

    virtual ApiTokenSequence importBiff12Formula(
                            const CellAddress& rBaseAddr,
                            FormulaType eType,
                            SequenceInputStream& rStrm ) override;

private:
    // import token contents and create API formula token ---------------------

    bool                importAttrToken( SequenceInputStream& rStrm );
    bool                importSpaceToken( SequenceInputStream& rStrm );
    bool                importTableToken( SequenceInputStream& rStrm );
    bool                importArrayToken( SequenceInputStream& rStrm );
    bool                importRefToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importAreaToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRef3dToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importArea3dToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importMemAreaToken( SequenceInputStream& rStrm, bool bAddData );
    bool                importMemFuncToken( SequenceInputStream& rStrm );
    bool                importNameToken( SequenceInputStream& rStrm );
    bool                importNameXToken( SequenceInputStream& rStrm );
    bool                importFuncToken( SequenceInputStream& rStrm );
    bool                importFuncVarToken( SequenceInputStream& rStrm );
    bool                importExpToken( SequenceInputStream& rStrm );

    LinkSheetRange      readSheetRange( SequenceInputStream& rStrm );

    void                swapStreamPosition( SequenceInputStream& rStrm );
    void                skipMemAreaAddData( SequenceInputStream& rStrm );

    // convert BIN token and push API operand or operator ---------------------

    bool                pushBiff12Name( sal_Int32 nNameId );
    bool                pushBiff12ExtName( sal_Int32 nRefId, sal_Int32 nNameId );
    bool                pushBiff12Function( sal_uInt16 nFuncId );
    bool                pushBiff12Function( sal_uInt16 nFuncId, sal_uInt8 nParamCount );

private:
    ApiParserWrapper    maApiParser;        /// Wrapper for the API formula parser object.
    sal_Int64           mnAddDataPos;       /// Current stream position for additional data (tExp, tArray, tMemArea).
    bool                mbNeedExtRefs;      /// True = parser needs initialization of external reference info.
};

OoxFormulaParserImpl::OoxFormulaParserImpl( const FormulaParser& rParent ) :
    FormulaParserImpl( rParent ),
    maApiParser( rParent.getBaseFilter().getModelFactory(), rParent ),
    mnAddDataPos( 0 ),
    mbNeedExtRefs( true )
{
}

ApiTokenSequence OoxFormulaParserImpl::importOoxFormula( const CellAddress& rBaseAddr, const OUString& rFormulaString )
{
    if( mbNeedExtRefs )
    {
        maApiParser.getParserProperties().setProperty( PROP_ExternalLinks, getExternalLinks().getLinkInfos() );
        mbNeedExtRefs = false;
    }
    return finalizeTokenArray( maApiParser.parseFormula( rFormulaString, rBaseAddr ) );
}

ApiTokenSequence OoxFormulaParserImpl::importBiff12Formula( const CellAddress& rBaseAddr, FormulaType eType, SequenceInputStream& rStrm )
{
    initializeImport( rBaseAddr, eType );

    sal_Int32 nFmlaSize = rStrm.readInt32();
    sal_Int64 nFmlaPos = rStrm.tell();
    sal_Int64 nFmlaEndPos = nFmlaPos + nFmlaSize;

    rStrm.seek( nFmlaEndPos );
    sal_Int32 nAddDataSize = rStrm.readInt32();
    mnAddDataPos = rStrm.tell();
    sal_Int64 nAddDataEndPos = mnAddDataPos + nAddDataSize;
    rStrm.seek( nFmlaPos );

    bool bOk = (nFmlaSize >= 0) && (nAddDataSize >= 0);
    bool bRelativeAsOffset = mbRelativeAsOffset;

    while( bOk && !rStrm.isEof() && (rStrm.tell() < nFmlaEndPos) )
    {
        sal_uInt8 nTokenId;
        nTokenId = rStrm.readuChar();
        sal_uInt8 nTokenClass = nTokenId & BIFF_TOKCLASS_MASK;
        sal_uInt8 nBaseId = nTokenId & BIFF_TOKID_MASK;

        if( nTokenClass == BIFF_TOKCLASS_NONE )
        {
            // base tokens
            switch( nBaseId )
            {
                case BIFF_TOKID_EXP:        bOk = importExpToken( rStrm );                                      break;
                case BIFF_TOKID_ADD:        bOk = pushBinaryOperator( OPCODE_ADD );                             break;
                case BIFF_TOKID_SUB:        bOk = pushBinaryOperator( OPCODE_SUB );                             break;
                case BIFF_TOKID_MUL:        bOk = pushBinaryOperator( OPCODE_MULT );                            break;
                case BIFF_TOKID_DIV:        bOk = pushBinaryOperator( OPCODE_DIV );                             break;
                case BIFF_TOKID_POWER:      bOk = pushBinaryOperator( OPCODE_POWER );                           break;
                case BIFF_TOKID_CONCAT:     bOk = pushBinaryOperator( OPCODE_CONCAT );                          break;
                case BIFF_TOKID_LT:         bOk = pushBinaryOperator( OPCODE_LESS );                            break;
                case BIFF_TOKID_LE:         bOk = pushBinaryOperator( OPCODE_LESS_EQUAL );                      break;
                case BIFF_TOKID_EQ:         bOk = pushBinaryOperator( OPCODE_EQUAL );                           break;
                case BIFF_TOKID_GE:         bOk = pushBinaryOperator( OPCODE_GREATER_EQUAL );                   break;
                case BIFF_TOKID_GT:         bOk = pushBinaryOperator( OPCODE_GREATER );                         break;
                case BIFF_TOKID_NE:         bOk = pushBinaryOperator( OPCODE_NOT_EQUAL );                       break;
                case BIFF_TOKID_ISECT:      bOk = pushBinaryOperator( OPCODE_INTERSECT );                       break;
                case BIFF_TOKID_LIST:       bOk = pushBinaryOperator( OPCODE_LIST );                            break;
                case BIFF_TOKID_RANGE:      bOk = pushBinaryOperator( OPCODE_RANGE );                           break;
                case BIFF_TOKID_UPLUS:      bOk = pushUnaryPreOperator( OPCODE_PLUS_SIGN );                     break;
                case BIFF_TOKID_UMINUS:     bOk = pushUnaryPreOperator( OPCODE_MINUS_SIGN );                    break;
                case BIFF_TOKID_PERCENT:    bOk = pushUnaryPostOperator( OPCODE_PERCENT );                      break;
                case BIFF_TOKID_PAREN:      bOk = pushParenthesesOperator();                                    break;
                case BIFF_TOKID_MISSARG:    bOk = pushOperand( OPCODE_MISSING );                                break;
                case BIFF_TOKID_STR:        bOk = pushValueOperand( BiffHelper::readString( rStrm, false ) );   break;
                case BIFF_TOKID_NLR:        bOk = importTableToken( rStrm );                                    break;
                case BIFF_TOKID_ATTR:       bOk = importAttrToken( rStrm );                                     break;
                case BIFF_TOKID_ERR:        bOk = pushBiffErrorOperand( rStrm.readuInt8() );                    break;
                case BIFF_TOKID_BOOL:       bOk = pushBiffBoolOperand( rStrm.readuInt8() );                     break;
                case BIFF_TOKID_INT:        bOk = pushValueOperand< double >( rStrm.readuInt16() );             break;
                case BIFF_TOKID_NUM:        bOk = pushValueOperand( rStrm.readDouble() );                       break;
                default:                    bOk = false;
            }
        }
        else
        {
            // classified tokens
            switch( nBaseId )
            {
                case BIFF_TOKID_ARRAY:      bOk = importArrayToken( rStrm );                            break;
                case BIFF_TOKID_FUNC:       bOk = importFuncToken( rStrm );                             break;
                case BIFF_TOKID_FUNCVAR:    bOk = importFuncVarToken( rStrm );                          break;
                case BIFF_TOKID_NAME:       bOk = importNameToken( rStrm );                             break;
                case BIFF_TOKID_REF:        bOk = importRefToken( rStrm, false, false );                break;
                case BIFF_TOKID_AREA:       bOk = importAreaToken( rStrm, false, false );               break;
                case BIFF_TOKID_MEMAREA:    bOk = importMemAreaToken( rStrm, true );                    break;
                case BIFF_TOKID_MEMERR:     bOk = importMemAreaToken( rStrm, false );                   break;
                case BIFF_TOKID_MEMNOMEM:   bOk = importMemAreaToken( rStrm, false );                   break;
                case BIFF_TOKID_MEMFUNC:    bOk = importMemFuncToken( rStrm );                          break;
                case BIFF_TOKID_REFERR:     bOk = importRefToken( rStrm, true, false );                 break;
                case BIFF_TOKID_AREAERR:    bOk = importAreaToken( rStrm, true, false );                break;
                case BIFF_TOKID_REFN:       bOk = importRefToken( rStrm, false, true );                 break;
                case BIFF_TOKID_AREAN:      bOk = importAreaToken( rStrm, false, true );                break;
                case BIFF_TOKID_MEMAREAN:   bOk = importMemFuncToken( rStrm );                          break;
                case BIFF_TOKID_MEMNOMEMN:  bOk = importMemFuncToken( rStrm );                          break;
                case BIFF_TOKID_NAMEX:      bOk = importNameXToken( rStrm );                            break;
                case BIFF_TOKID_REF3D:      bOk = importRef3dToken( rStrm, false, bRelativeAsOffset );  break;
                case BIFF_TOKID_AREA3D:     bOk = importArea3dToken( rStrm, false, bRelativeAsOffset ); break;
                case BIFF_TOKID_REFERR3D:   bOk = importRef3dToken( rStrm, true, bRelativeAsOffset );   break;
                case BIFF_TOKID_AREAERR3D:  bOk = importArea3dToken( rStrm, true, bRelativeAsOffset );  break;
                default:                    bOk = false;
            }
        }
    }

    // build and finalize the token sequence
    ApiTokenSequence aFinalTokens;
    if( bOk && (rStrm.tell() == nFmlaEndPos) && (mnAddDataPos == nAddDataEndPos) )
        aFinalTokens = finalizeImport();

    // seek behind token array
    if( (nFmlaSize >= 0) && (nAddDataSize >= 0) )
        rStrm.seek( nAddDataEndPos );

    // return the final token sequence
    return aFinalTokens;
}

// import token contents and create API formula token -------------------------

bool OoxFormulaParserImpl::importAttrToken( SequenceInputStream& rStrm )
{
    bool bOk = true;
    sal_uInt8 nType;
    nType = rStrm.readuChar();
    // equal flags in all BIFFs
    switch( nType )
    {
        case 0:     // sometimes, tAttrSkip tokens miss the type flag
        case BIFF_TOK_ATTR_VOLATILE:
        case BIFF_TOK_ATTR_IF:
        case BIFF_TOK_ATTR_SKIP:
        case BIFF_TOK_ATTR_ASSIGN:
        case BIFF_TOK_ATTR_IFERROR:
            rStrm.skip( 2 );
        break;
        case BIFF_TOK_ATTR_CHOOSE:
            rStrm.skip( 2 * rStrm.readuInt16() + 2 );
        break;
        case BIFF_TOK_ATTR_SUM:
            rStrm.skip( 2 );
            bOk = pushBiff12Function( BIFF_FUNC_SUM, 1 );
        break;
        case BIFF_TOK_ATTR_SPACE:
        case BIFF_TOK_ATTR_SPACE_VOLATILE:
            bOk = importSpaceToken( rStrm );
        break;
        default:
            bOk = false;
    }
    return bOk;
}

bool OoxFormulaParserImpl::importSpaceToken( SequenceInputStream& rStrm )
{
    // equal constants in BIFF and OOX
    sal_uInt8 nType, nCount;
    nType = rStrm.readuChar();
    nCount = rStrm.readuChar();
    switch( nType )
    {
        case BIFF_TOK_ATTR_SPACE_SP:
            appendLeadingSpaces( nCount, false );
        break;
        case BIFF_TOK_ATTR_SPACE_BR:
            appendLeadingSpaces( nCount, true );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_OPEN:
            appendOpeningSpaces( nCount, false );
        break;
        case BIFF_TOK_ATTR_SPACE_BR_OPEN:
            appendOpeningSpaces( nCount, true );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_CLOSE:
            appendClosingSpaces( nCount, false );
        break;
        case BIFF_TOK_ATTR_SPACE_BR_CLOSE:
            appendClosingSpaces( nCount, true );
        break;
    }
    return true;
}

bool OoxFormulaParserImpl::importTableToken( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags, nTableId, nCol1, nCol2;
    rStrm.skip( 3 );
    nFlags = rStrm.readuInt16();
    nTableId = rStrm.readuInt16();
    rStrm.skip( 2 );
    nCol1 = rStrm.readuInt16();
    nCol2 = rStrm.readuInt16();
    TableRef xTable = getTables().getTable( nTableId );
    sal_Int32 nTokenIndex = xTable.get() ? xTable->getTokenIndex() : -1;
    if( nTokenIndex >= 0 )
    {
        sal_Int32 nWidth = xTable->getWidth();
        sal_Int32 nHeight = xTable->getHeight();
        sal_Int32 nStartCol = 0;
        sal_Int32 nEndCol = nWidth - 1;
        sal_Int32 nStartRow = 0;
        sal_Int32 nEndRow = nHeight - 1;
        bool bFixedStartRow = true;
        bool bFixedHeight = false;

        bool bSingleCol = getFlag( nFlags, BIFF12_TOK_TABLE_COLUMN );
        bool bColRange = getFlag( nFlags, BIFF12_TOK_TABLE_COLRANGE );
        bool bValidRef = !bSingleCol || !bColRange;
        OSL_ENSURE( bValidRef, "OoxFormulaParserImpl::importTableToken - illegal combination of single column and column range" );
        if( bValidRef )
        {
            if( bSingleCol )
                nStartCol = nEndCol = nCol1;
            else if( bColRange )
                { nStartCol = nCol1; nEndCol = nCol2; }
            bValidRef = (nStartCol <= nEndCol) && (nEndCol < nWidth);
            OSL_ENSURE( bValidRef, "OoxFormulaParserImpl::importTableToken - invalid column range" );
        }

        if( bValidRef )
        {
            bool bAllRows    = getFlag( nFlags, BIFF12_TOK_TABLE_ALL );
            bool bHeaderRows = getFlag( nFlags, BIFF12_TOK_TABLE_HEADERS );
            bool bDataRows   = getFlag( nFlags, BIFF12_TOK_TABLE_DATA );
            bool bTotalsRows = getFlag( nFlags, BIFF12_TOK_TABLE_TOTALS );
            bool bThisRow    = getFlag( nFlags, BIFF12_TOK_TABLE_THISROW );

            sal_Int32 nStartDataRow = xTable->getHeaderRows();
            sal_Int32 nEndDataRow = nEndRow - xTable->getTotalsRows();
            bValidRef = (nStartRow <= nStartDataRow) && (nStartDataRow <= nEndDataRow) && (nEndDataRow <= nEndRow);
            OSL_ENSURE( bValidRef, "OoxFormulaParserImpl::importTableToken - invalid data row range" );
            if( bValidRef )
            {
                if( bAllRows )
                {
                    bValidRef = !bHeaderRows && !bDataRows && !bTotalsRows && !bThisRow;
                    OSL_ENSURE( bValidRef, "OoxFormulaParserImpl::importTableToken - unexpected flags in [#All] table token" );
                }
                else if( bHeaderRows )
                {
                    bValidRef = !bTotalsRows && !bThisRow;
                    OSL_ENSURE( bValidRef, "OoxFormulaParserImpl::importTableToken - unexpected flags in [#Headers] table token" );
                    nEndRow = bDataRows ? nEndDataRow : (nStartDataRow - 1);
                    bFixedHeight = !bDataRows;
                }
                else if( bDataRows )
                {
                    bValidRef = !bThisRow;
                    OSL_ENSURE( bValidRef, "OoxFormulaParserImpl::importTableToken - unexpected flags in [#Data] table token" );
                    nStartRow = nStartDataRow;
                    if( !bTotalsRows ) nEndRow = nEndDataRow;
                }
                else if( bTotalsRows )
                {
                    bValidRef = !bThisRow;
                    OSL_ENSURE( bValidRef, "OoxFormulaParserImpl::importTableToken - unexpected flags in [#Totals] table token" );
                    nStartRow = nEndDataRow + 1;
                    bFixedStartRow = false;
                    bFixedHeight = !bDataRows;
                }
                else if( bThisRow )
                {
                    nStartRow = nEndRow = maBaseAddr.Row - xTable->getRange().StartRow;
                    bFixedHeight = true;
                }
                else
                {
                    // nothing is the same as [#Data]
                    nStartRow = nStartDataRow;
                    nEndRow = nEndDataRow;
                }
            }
            if( bValidRef )
                bValidRef = (0 <= nStartRow) && (nStartRow <= nEndRow) && (nEndRow < nHeight);
        }
        if( bValidRef )
        {
            // push single database area token, if table token refers to entire table
            if( (nStartCol == 0) && (nEndCol + 1 == nWidth) && (nStartRow == 0) && (nEndRow + 1 == nHeight) )
                return pushValueOperand( nTokenIndex, OPCODE_DBAREA );
            // create an OFFSET function call to refer to a subrange of the table
            const FunctionInfo* pRowsInfo = getFuncInfoFromBiff12FuncId( BIFF_FUNC_ROWS );
            const FunctionInfo* pColumnsInfo = getFuncInfoFromBiff12FuncId( BIFF_FUNC_COLUMNS );
            return
                pRowsInfo && pColumnsInfo &&
                pushValueOperandToken( nTokenIndex, OPCODE_DBAREA ) &&
                (bFixedStartRow ?
                    pushValueOperandToken< double >( nStartRow ) :
                    (pushValueOperandToken( nTokenIndex, OPCODE_DBAREA ) &&
                     pushFunctionOperatorToken( *pRowsInfo, 1 ) &&
                     pushValueOperandToken< double >( nHeight - nStartRow ) &&
                     pushBinaryOperatorToken( OPCODE_SUB ))) &&
                pushValueOperandToken< double >( nStartCol ) &&
                (bFixedHeight ?
                    pushValueOperandToken< double >( nEndRow - nStartRow + 1 ) :
                    (pushValueOperandToken( nTokenIndex, OPCODE_DBAREA ) &&
                     pushFunctionOperatorToken( *pRowsInfo, 1 ) &&
                     (((nStartRow == 0) && (nEndRow + 1 == nHeight)) ||
                      (pushValueOperandToken< double >( nHeight - (nEndRow - nStartRow + 1) ) &&
                       pushBinaryOperatorToken( OPCODE_SUB ))))) &&
                (((nStartCol == 0) && (nEndCol + 1 == nWidth)) ?
                    (pushValueOperandToken( nTokenIndex, OPCODE_DBAREA ) &&
                     pushFunctionOperatorToken( *pColumnsInfo, 1 )) :
                    pushValueOperandToken< double >( nEndCol - nStartCol + 1 )) &&
                pushBiff12Function( BIFF_FUNC_OFFSET, 5 );
        }
    }
    return pushBiffErrorOperand( BIFF_ERR_REF );
}

bool OoxFormulaParserImpl::importArrayToken( SequenceInputStream& rStrm )
{
    rStrm.skip( 14 );

    // start token array with opening brace and leading spaces
    pushOperand( OPCODE_ARRAY_OPEN );
    size_t nOpSize = popOperandSize();
    size_t nOldArraySize = getFormulaSize();

    // read array size
    swapStreamPosition( rStrm );
    sal_Int32 nRows = rStrm.readInt32();
    sal_Int32 nCols = rStrm.readInt32();
    OSL_ENSURE( (nCols > 0) && (nRows > 0), "OoxFormulaParserImpl::importArrayToken - empty array" );

    // read array values and build token array
    for( sal_Int32 nRow = 0; !rStrm.isEof() && (nRow < nRows); ++nRow )
    {
        if( nRow > 0 )
            appendRawToken( OPCODE_ARRAY_ROWSEP );
        for( sal_Int32 nCol = 0; !rStrm.isEof() && (nCol < nCols); ++nCol )
        {
            if( nCol > 0 )
                appendRawToken( OPCODE_ARRAY_COLSEP );
            switch( rStrm.readuInt8() )
            {
                case BIFF_TOK_ARRAY_DOUBLE:
                    appendRawToken( OPCODE_PUSH ) <<= rStrm.readDouble();
                break;
                case BIFF_TOK_ARRAY_STRING:
                    appendRawToken( OPCODE_PUSH ) <<= BiffHelper::readString( rStrm, false );
                break;
                case BIFF_TOK_ARRAY_BOOL:
                    appendRawToken( OPCODE_PUSH ) <<= (static_cast< double >( (rStrm.readuInt8() == BIFF_TOK_BOOL_FALSE) ? 0.0 : 1.0 ));
                break;
                case BIFF_TOK_ARRAY_ERROR:
                    appendRawToken( OPCODE_PUSH ) <<= BiffHelper::calcDoubleFromError( rStrm.readuInt8() );
                    rStrm.skip( 3 );
                break;
                default:
                    OSL_FAIL( "OoxFormulaParserImpl::importArrayToken - unknown data type" );
                    appendRawToken( OPCODE_PUSH ) <<= BiffHelper::calcDoubleFromError( BIFF_ERR_NA );
            }
        }
    }
    swapStreamPosition( rStrm );

    // close token array and set resulting operand size
    appendRawToken( OPCODE_ARRAY_CLOSE );
    pushOperandSize( nOpSize + getFormulaSize() - nOldArraySize );
    return true;
}

bool OoxFormulaParserImpl::importRefToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinSingleRef2d aRef;
    aRef.readBiff12Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importAreaToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinComplexRef2d aRef;
    aRef.readBiff12Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importRef3dToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange( rStrm );
    BinSingleRef2d aRef;
    aRef.readBiff12Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importArea3dToken( SequenceInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange( rStrm );
    BinComplexRef2d aRef;
    aRef.readBiff12Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importMemAreaToken( SequenceInputStream& rStrm, bool bAddData )
{
    rStrm.skip( 6 );
    if( bAddData )
        skipMemAreaAddData( rStrm );
    return true;
}

bool OoxFormulaParserImpl::importMemFuncToken( SequenceInputStream& rStrm )
{
    rStrm.skip( 2 );
    return true;
}

bool OoxFormulaParserImpl::importNameToken( SequenceInputStream& rStrm )
{
    return pushBiff12Name( rStrm.readInt32() );
}

bool OoxFormulaParserImpl::importNameXToken( SequenceInputStream& rStrm )
{
    sal_Int32 nRefId = rStrm.readInt16();
    sal_Int32 nNameId = rStrm.readInt32();
    return pushBiff12ExtName( nRefId, nNameId );
}

bool OoxFormulaParserImpl::importFuncToken( SequenceInputStream& rStrm )
{
    sal_uInt16 nFuncId;
    nFuncId = rStrm.readuInt16();
    return pushBiff12Function( nFuncId );
}

bool OoxFormulaParserImpl::importFuncVarToken( SequenceInputStream& rStrm )
{
    sal_uInt8 nParamCount;
    sal_uInt16 nFuncId;
    nParamCount = rStrm.readuChar();
    nFuncId = rStrm.readuInt16();
    return pushBiff12Function( nFuncId, nParamCount );
}

bool OoxFormulaParserImpl::importExpToken( SequenceInputStream& rStrm )
{
    BinAddress aBaseAddr;
    aBaseAddr.mnRow = rStrm.readInt32();
    swapStreamPosition( rStrm );
    aBaseAddr.mnCol = rStrm.readInt32();
    swapStreamPosition( rStrm );
    return pushSpecialTokenOperand( aBaseAddr, false );
}

LinkSheetRange OoxFormulaParserImpl::readSheetRange( SequenceInputStream& rStrm )
{
    return getExternalLinks().getSheetRange( rStrm.readInt16() );
}

void OoxFormulaParserImpl::swapStreamPosition( SequenceInputStream& rStrm )
{
    sal_Int64 nRecPos = rStrm.tell();
    rStrm.seek( mnAddDataPos );
    mnAddDataPos = nRecPos;
}

void OoxFormulaParserImpl::skipMemAreaAddData( SequenceInputStream& rStrm )
{
    swapStreamPosition( rStrm );
    rStrm.skip( 16 * rStrm.readInt32() );
    swapStreamPosition( rStrm );
}

// convert BIN token and push API operand or operator -------------------------

bool OoxFormulaParserImpl::pushBiff12Name( sal_Int32 nNameId )
{
    // one-based in BIFF12 formulas
    return pushDefinedNameOperand( getDefinedNames().getByIndex( nNameId - 1 ) );
}

bool OoxFormulaParserImpl::pushBiff12ExtName( sal_Int32 nRefId, sal_Int32 nNameId )
{
    if( const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId ).get() )
    {
        if( pExtLink->getLinkType() == LINKTYPE_SELF )
            return pushBiff12Name( nNameId );
        // external name indexes are one-based in BIFF12
        ExternalNameRef xExtName = pExtLink->getNameByIndex( nNameId - 1 );
        return pushExternalNameOperand( xExtName, *pExtLink );
    }
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool OoxFormulaParserImpl::pushBiff12Function( sal_uInt16 nFuncId )
{
    if( const FunctionInfo* pFuncInfo = getFuncInfoFromBiff12FuncId( nFuncId ) )
        if( pFuncInfo->mnMinParamCount == pFuncInfo->mnMaxParamCount )
            return pushFunctionOperator( *pFuncInfo, pFuncInfo->mnMinParamCount );
    return pushFunctionOperator( OPCODE_NONAME, 0 );
}

bool OoxFormulaParserImpl::pushBiff12Function( sal_uInt16 nFuncId, sal_uInt8 nParamCount )
{
    if( getFlag( nFuncId, BIFF_TOK_FUNCVAR_CMD ) )
        nParamCount &= BIFF_TOK_FUNCVAR_COUNTMASK;
    if( const FunctionInfo* pFuncInfo = getFuncInfoFromBiff12FuncId( nFuncId ) )
        return pushFunctionOperator( *pFuncInfo, nParamCount );
    return pushFunctionOperator( OPCODE_NONAME, nParamCount );
}

// BIFF parser implementation =================================================

namespace {

/** A natural language reference struct with relative flag. */
struct BiffNlr
{
    sal_Int32           mnCol;              /// Column index.
    sal_Int32           mnRow;              /// Row index.
    bool                mbRel;              /// True = relative column/row reference.

    explicit            BiffNlr();

    void                readBiff8Data( BiffInputStream& rStrm );
};

BiffNlr::BiffNlr() :
    mnCol( 0 ),
    mnRow( 0 ),
    mbRel( false )
{
}

void BiffNlr::readBiff8Data( BiffInputStream& rStrm )
{
    sal_uInt16 nRow, nCol;
    rStrm >> nRow >> nCol;
    mnCol = nCol & BIFF_TOK_NLR_MASK;
    mnRow = nRow;
    mbRel = getFlag( nCol, BIFF_TOK_NLR_REL );
}

bool lclIsValidNlrStack( const BinAddress& rAddr1, const BinAddress& rAddr2, bool bRow )
{
    return bRow ?
        ((rAddr1.mnRow == rAddr2.mnRow) && (rAddr1.mnCol + 1 == rAddr2.mnCol)) :
        ((rAddr1.mnCol == rAddr2.mnCol) && (rAddr1.mnRow + 1 == rAddr2.mnRow));
}

bool lclIsValidNlrRange( const BiffNlr& rNlr, const BinRange& rRange, bool bRow )
{
    return bRow ?
        ((rNlr.mnRow == rRange.maFirst.mnRow) && (rNlr.mnCol + 1 == rRange.maFirst.mnCol) && (rRange.maFirst.mnRow == rRange.maLast.mnRow)) :
        ((rNlr.mnCol == rRange.maFirst.mnCol) && (rNlr.mnRow + 1 == rRange.maFirst.mnRow) && (rRange.maFirst.mnCol == rRange.maLast.mnCol));
}

} // namespace

class BiffFormulaParserImpl : public FormulaParserImpl
{
public:
    explicit            BiffFormulaParserImpl( const FormulaParser& rParent );

private:
    // import token contents and create API formula token ---------------------

    bool                importTokenNotAvailable( BiffInputStream& rStrm );
    bool                importRefTokenNotAvailable( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importStrToken2( BiffInputStream& rStrm );
    bool                importStrToken8( BiffInputStream& rStrm );
    bool                importSpaceToken3( BiffInputStream& rStrm );
    bool                importSpaceToken4( BiffInputStream& rStrm );
    bool                importSheetToken2( BiffInputStream& rStrm );
    bool                importSheetToken3( BiffInputStream& rStrm );
    bool                importEndSheetToken2( BiffInputStream& rStrm );
    bool                importEndSheetToken3( BiffInputStream& rStrm );
    bool                importNlrToken( BiffInputStream& rStrm );
    bool                importRefToken2( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRefToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importAreaToken2( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importAreaToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRef3dToken5( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRef3dToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importArea3dToken5( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importArea3dToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importNameXToken( BiffInputStream& rStrm );
    bool                importFuncToken2( BiffInputStream& rStrm );
    bool                importFuncToken4( BiffInputStream& rStrm );
    bool                importFuncVarToken2( BiffInputStream& rStrm );
    bool                importFuncVarToken4( BiffInputStream& rStrm );
    bool                importFuncCEToken( BiffInputStream& rStrm );

    bool                importNlrAddrToken( BiffInputStream& rStrm, bool bRow );
    bool                importNlrRangeToken( BiffInputStream& rStrm );
    bool                importNlrSAddrToken( BiffInputStream& rStrm, bool bRow );
    bool                importNlrSRangeToken( BiffInputStream& rStrm );
    bool                importNlrErrToken( BiffInputStream& rStrm, sal_uInt16 nSkip );

    sal_Int32           readRefId( BiffInputStream& rStrm );
    sal_uInt16          readNameId( BiffInputStream& rStrm );
    LinkSheetRange      readSheetRange5( BiffInputStream& rStrm );
    LinkSheetRange      readSheetRange8( BiffInputStream& rStrm );

    void                swapStreamPosition( BiffInputStream& rStrm );
    bool                readNlrSAddrAddData( BiffNlr& orNlr, BiffInputStream& rStrm, bool bRow );
    bool                readNlrSRangeAddData( BiffNlr& orNlr, bool& orbIsRow, BiffInputStream& rStrm );

    // convert BIFF token and push API operand or operator --------------------

    bool                pushBiffReference( const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushBiffReference( const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushBiffNlrAddr( const BiffNlr& rNlr, bool bRow );
    bool                pushBiffNlrRange( const BiffNlr& rNlr, const BinRange& rRange );
    bool                pushBiffNlrSAddr( const BiffNlr& rNlr, bool bRow );
    bool                pushBiffNlrSRange( const BiffNlr& rNlr, const BinRange& rRange, bool bRow );
    bool                pushBiffName( sal_uInt16 nNameId );
    bool                pushBiffExtName( sal_Int32 nRefId, sal_uInt16 nNameId );
    bool                pushBiffFunction( sal_uInt16 nFuncId );
    bool                pushBiffFunction( sal_uInt16 nFuncId, sal_uInt8 nParamCount );

private:
    typedef bool (BiffFormulaParserImpl::*ImportTokenFunc)( BiffInputStream& );
    typedef bool (BiffFormulaParserImpl::*ImportRefTokenFunc)( BiffInputStream&, bool, bool );

    ImportTokenFunc     mpImportStrToken;           /// Pointer to tStr import function (string constant).
    ImportTokenFunc     mpImportSpaceToken;         /// Pointer to tAttrSpace import function (spaces/line breaks).
    ImportTokenFunc     mpImportSheetToken;         /// Pointer to tSheet import function (external reference).
    ImportTokenFunc     mpImportEndSheetToken;      /// Pointer to tEndSheet import function (end of external reference).
    ImportTokenFunc     mpImportNlrToken;           /// Pointer to tNlr import function (natural language reference).
    ImportRefTokenFunc  mpImportRefToken;           /// Pointer to tRef import function (2d cell reference).
    ImportRefTokenFunc  mpImportAreaToken;          /// Pointer to tArea import function (2d area reference).
    ImportRefTokenFunc  mpImportRef3dToken;         /// Pointer to tRef3d import function (3d cell reference).
    ImportRefTokenFunc  mpImportArea3dToken;        /// Pointer to tArea3d import function (3d area reference).
    ImportTokenFunc     mpImportNameXToken;         /// Pointer to tNameX import function (external name).
    ImportTokenFunc     mpImportFuncToken;          /// Pointer to tFunc import function (function with fixed parameter count).
    ImportTokenFunc     mpImportFuncVarToken;       /// Pointer to tFuncVar import function (function with variable parameter count).
    ImportTokenFunc     mpImportFuncCEToken;        /// Pointer to tFuncCE import function (command macro call).
    sal_Int64           mnAddDataPos;               /// Current stream position for additional data (tArray, tMemArea, tNlr).
    sal_Int32           mnCurrRefId;                /// Current ref-id from tSheet token (BIFF2-BIFF4 only).
    sal_uInt16          mnAttrDataSize;             /// Size of one tAttr data element.
    sal_uInt16          mnArraySize;                /// Size of tArray data.
    sal_uInt16          mnNameSize;                 /// Size of tName data.
    sal_uInt16          mnMemAreaSize;              /// Size of tMemArea data.
    sal_uInt16          mnMemFuncSize;              /// Size of tMemFunc data.
    sal_uInt16          mnRefIdSize;                /// Size of unused data following a reference identifier.
};

BiffFormulaParserImpl::BiffFormulaParserImpl( const FormulaParser& rParent ) :
    FormulaParserImpl( rParent ),
    mnAddDataPos( 0 ),
    mnCurrRefId( 0 )
{
    switch( getBiff() )
    {
        case BIFF2:
            mpImportStrToken = &BiffFormulaParserImpl::importStrToken2;
            mpImportSpaceToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportSheetToken = &BiffFormulaParserImpl::importSheetToken2;
            mpImportEndSheetToken = &BiffFormulaParserImpl::importEndSheetToken2;
            mpImportNlrToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportRefToken = &BiffFormulaParserImpl::importRefToken2;
            mpImportAreaToken = &BiffFormulaParserImpl::importAreaToken2;
            mpImportRef3dToken = &BiffFormulaParserImpl::importRefTokenNotAvailable;
            mpImportArea3dToken = &BiffFormulaParserImpl::importRefTokenNotAvailable;
            mpImportNameXToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportFuncToken = &BiffFormulaParserImpl::importFuncToken2;
            mpImportFuncVarToken = &BiffFormulaParserImpl::importFuncVarToken2;
            mpImportFuncCEToken = &BiffFormulaParserImpl::importFuncCEToken;
            mnAttrDataSize = 1;
            mnArraySize = 6;
            mnNameSize = 5;
            mnMemAreaSize = 4;
            mnMemFuncSize = 1;
            mnRefIdSize = 1;
        break;
        case BIFF3:
            mpImportStrToken = &BiffFormulaParserImpl::importStrToken2;
            mpImportSpaceToken = &BiffFormulaParserImpl::importSpaceToken3;
            mpImportSheetToken = &BiffFormulaParserImpl::importSheetToken3;
            mpImportEndSheetToken = &BiffFormulaParserImpl::importEndSheetToken3;
            mpImportNlrToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportRefToken = &BiffFormulaParserImpl::importRefToken2;
            mpImportAreaToken = &BiffFormulaParserImpl::importAreaToken2;
            mpImportRef3dToken = &BiffFormulaParserImpl::importRefTokenNotAvailable;
            mpImportArea3dToken = &BiffFormulaParserImpl::importRefTokenNotAvailable;
            mpImportNameXToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportFuncToken = &BiffFormulaParserImpl::importFuncToken2;
            mpImportFuncVarToken = &BiffFormulaParserImpl::importFuncVarToken2;
            mpImportFuncCEToken = &BiffFormulaParserImpl::importFuncCEToken;
            mnAttrDataSize = 2;
            mnArraySize = 7;
            mnNameSize = 8;
            mnMemAreaSize = 6;
            mnMemFuncSize = 2;
            mnRefIdSize = 2;
        break;
        case BIFF4:
            mpImportStrToken = &BiffFormulaParserImpl::importStrToken2;
            mpImportSpaceToken = &BiffFormulaParserImpl::importSpaceToken4;
            mpImportSheetToken = &BiffFormulaParserImpl::importSheetToken3;
            mpImportEndSheetToken = &BiffFormulaParserImpl::importEndSheetToken3;
            mpImportNlrToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportRefToken = &BiffFormulaParserImpl::importRefToken2;
            mpImportAreaToken = &BiffFormulaParserImpl::importAreaToken2;
            mpImportRef3dToken = &BiffFormulaParserImpl::importRefTokenNotAvailable;
            mpImportArea3dToken = &BiffFormulaParserImpl::importRefTokenNotAvailable;
            mpImportNameXToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportFuncToken = &BiffFormulaParserImpl::importFuncToken4;
            mpImportFuncVarToken = &BiffFormulaParserImpl::importFuncVarToken4;
            mpImportFuncCEToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mnAttrDataSize = 2;
            mnArraySize = 7;
            mnNameSize = 8;
            mnMemAreaSize = 6;
            mnMemFuncSize = 2;
            mnRefIdSize = 2;
        break;
        case BIFF5:
            mpImportStrToken = &BiffFormulaParserImpl::importStrToken2;
            mpImportSpaceToken = &BiffFormulaParserImpl::importSpaceToken4;
            mpImportSheetToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportEndSheetToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportNlrToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportRefToken = &BiffFormulaParserImpl::importRefToken2;
            mpImportAreaToken = &BiffFormulaParserImpl::importAreaToken2;
            mpImportRef3dToken = &BiffFormulaParserImpl::importRef3dToken5;
            mpImportArea3dToken = &BiffFormulaParserImpl::importArea3dToken5;
            mpImportNameXToken = &BiffFormulaParserImpl::importNameXToken;
            mpImportFuncToken = &BiffFormulaParserImpl::importFuncToken4;
            mpImportFuncVarToken = &BiffFormulaParserImpl::importFuncVarToken4;
            mpImportFuncCEToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mnAttrDataSize = 2;
            mnArraySize = 7;
            mnNameSize = 12;
            mnMemAreaSize = 6;
            mnMemFuncSize = 2;
            mnRefIdSize = 8;
        break;
        case BIFF8:
            mpImportStrToken = &BiffFormulaParserImpl::importStrToken8;
            mpImportSpaceToken = &BiffFormulaParserImpl::importSpaceToken4;
            mpImportSheetToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportEndSheetToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mpImportNlrToken = &BiffFormulaParserImpl::importNlrToken;
            mpImportRefToken = &BiffFormulaParserImpl::importRefToken8;
            mpImportAreaToken = &BiffFormulaParserImpl::importAreaToken8;
            mpImportRef3dToken = &BiffFormulaParserImpl::importRef3dToken8;
            mpImportArea3dToken = &BiffFormulaParserImpl::importArea3dToken8;
            mpImportNameXToken = &BiffFormulaParserImpl::importNameXToken;
            mpImportFuncToken = &BiffFormulaParserImpl::importFuncToken4;
            mpImportFuncVarToken = &BiffFormulaParserImpl::importFuncVarToken4;
            mpImportFuncCEToken = &BiffFormulaParserImpl::importTokenNotAvailable;
            mnAttrDataSize = 2;
            mnArraySize = 7;
            mnNameSize = 2;
            mnMemAreaSize = 6;
            mnMemFuncSize = 2;
            mnRefIdSize = 0;
        break;
        case BIFF_UNKNOWN:
            mpImportStrToken = nullptr;
            mpImportSpaceToken = nullptr;
            mpImportSheetToken = nullptr;
            mpImportEndSheetToken = nullptr;
            mpImportNlrToken = nullptr;
            mpImportRefToken = nullptr;
            mpImportAreaToken = nullptr;
            mpImportRef3dToken = nullptr;
            mpImportArea3dToken = nullptr;
            mpImportNameXToken = nullptr;
            mpImportFuncToken = nullptr;
            mpImportFuncVarToken = nullptr;
            mpImportFuncCEToken = nullptr;
            mnAttrDataSize = 0;
            mnArraySize = 0;
            mnNameSize = 0;
            mnMemAreaSize = 0;
            mnMemFuncSize = 0;
            mnRefIdSize = 0;
        break;
    }
}

// import token contents and create API formula token -------------------------

bool BiffFormulaParserImpl::importTokenNotAvailable( BiffInputStream& )
{
    // dummy function for pointer-to-member-function
    return false;
}

bool BiffFormulaParserImpl::importRefTokenNotAvailable( BiffInputStream&, bool, bool )
{
    // dummy function for pointer-to-member-function
    return false;
}

bool BiffFormulaParserImpl::importStrToken2( BiffInputStream& rStrm )
{
    return pushValueOperand( rStrm.readByteStringUC( false, getTextEncoding(), mbAllowNulChars ) );
}

bool BiffFormulaParserImpl::importStrToken8( BiffInputStream& rStrm )
{
    // read flags field for empty strings also
    return pushValueOperand( rStrm.readUniStringBody( rStrm.readuInt8(), mbAllowNulChars ) );
}

bool BiffFormulaParserImpl::importSpaceToken3( BiffInputStream& rStrm )
{
    rStrm.skip( 2 );
    return true;
}

bool BiffFormulaParserImpl::importSpaceToken4( BiffInputStream& rStrm )
{
    sal_uInt8 nType, nCount;
    rStrm >> nType >> nCount;
    switch( nType )
    {
        case BIFF_TOK_ATTR_SPACE_SP:
            appendLeadingSpaces( nCount, false );
        break;
        case BIFF_TOK_ATTR_SPACE_BR:
            appendLeadingSpaces( nCount, true );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_OPEN:
            appendOpeningSpaces( nCount, false );
        break;
        case BIFF_TOK_ATTR_SPACE_BR_OPEN:
            appendOpeningSpaces( nCount, true );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_CLOSE:
            appendClosingSpaces( nCount, false );
        break;
        case BIFF_TOK_ATTR_SPACE_BR_CLOSE:
            appendClosingSpaces( nCount, true );
        break;
    }
    return true;
}

bool BiffFormulaParserImpl::importSheetToken2( BiffInputStream& rStrm )
{
    rStrm.skip( 4 );
    mnCurrRefId = readRefId( rStrm );
    return true;
}

bool BiffFormulaParserImpl::importSheetToken3( BiffInputStream& rStrm )
{
    rStrm.skip( 6 );
    mnCurrRefId = readRefId( rStrm );
    return true;
}

bool BiffFormulaParserImpl::importEndSheetToken2( BiffInputStream& rStrm )
{
    rStrm.skip( 3 );
    mnCurrRefId = 0;
    return true;
}

bool BiffFormulaParserImpl::importEndSheetToken3( BiffInputStream& rStrm )
{
    rStrm.skip( 4 );
    mnCurrRefId = 0;
    return true;
}

bool BiffFormulaParserImpl::importNlrToken( BiffInputStream& rStrm )
{
    bool bOk = true;
    sal_uInt8 nNlrType;
    rStrm >> nNlrType;
    switch( nNlrType )
    {
        case BIFF_TOK_NLR_ERR:      bOk = importNlrErrToken( rStrm, 4 );        break;
        case BIFF_TOK_NLR_ROWR:     bOk = importNlrAddrToken( rStrm, true );    break;
        case BIFF_TOK_NLR_COLR:     bOk = importNlrAddrToken( rStrm, false );   break;
        case BIFF_TOK_NLR_ROWV:     bOk = importNlrAddrToken( rStrm, true );    break;
        case BIFF_TOK_NLR_COLV:     bOk = importNlrAddrToken( rStrm, false );   break;
        case BIFF_TOK_NLR_RANGE:    bOk = importNlrRangeToken( rStrm );         break;
        case BIFF_TOK_NLR_SRANGE:   bOk = importNlrSRangeToken( rStrm );        break;
        case BIFF_TOK_NLR_SROWR:    bOk = importNlrSAddrToken( rStrm, true );   break;
        case BIFF_TOK_NLR_SCOLR:    bOk = importNlrSAddrToken( rStrm, false );  break;
        case BIFF_TOK_NLR_SROWV:    bOk = importNlrSAddrToken( rStrm, true );   break;
        case BIFF_TOK_NLR_SCOLV:    bOk = importNlrSAddrToken( rStrm, false );  break;
        case BIFF_TOK_NLR_RANGEERR: bOk = importNlrErrToken( rStrm, 13 );       break;
        case BIFF_TOK_NLR_SXNAME:   bOk = importNlrErrToken( rStrm, 4 );        break;
        default:                    bOk = false;
    }
    return bOk;
}

bool BiffFormulaParserImpl::importRefToken2( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinSingleRef2d aRef;
    aRef.readBiff2Data( rStrm, bRelativeAsOffset );
    return pushBiffReference( aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importRefToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinSingleRef2d aRef;
    aRef.readBiff8Data( rStrm, bRelativeAsOffset );
    return pushBiffReference( aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importAreaToken2( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinComplexRef2d aRef;
    aRef.readBiff2Data( rStrm, bRelativeAsOffset );
    return pushBiffReference( aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importAreaToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinComplexRef2d aRef;
    aRef.readBiff8Data( rStrm, bRelativeAsOffset );
    return pushBiffReference( aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importRef3dToken5( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange5( rStrm );
    BinSingleRef2d aRef;
    aRef.readBiff2Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importRef3dToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange8( rStrm );
    BinSingleRef2d aRef;
    aRef.readBiff8Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importArea3dToken5( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange5( rStrm );
    BinComplexRef2d aRef;
    aRef.readBiff2Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importArea3dToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange8( rStrm );
    BinComplexRef2d aRef;
    aRef.readBiff8Data( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::importNameXToken( BiffInputStream& rStrm )
{
    sal_Int32 nRefId = readRefId( rStrm );
    sal_uInt16 nNameId = readNameId( rStrm );
    return pushBiffExtName( nRefId, nNameId );
}

bool BiffFormulaParserImpl::importFuncToken2( BiffInputStream& rStrm )
{
    sal_uInt8 nFuncId;
    rStrm >> nFuncId;
    return pushBiffFunction( nFuncId );
}

bool BiffFormulaParserImpl::importFuncToken4( BiffInputStream& rStrm )
{
    sal_uInt16 nFuncId;
    rStrm >> nFuncId;
    return pushBiffFunction( nFuncId );
}

bool BiffFormulaParserImpl::importFuncVarToken2( BiffInputStream& rStrm )
{
    sal_uInt8 nParamCount, nFuncId;
    rStrm >> nParamCount >> nFuncId;
    return pushBiffFunction( nFuncId, nParamCount );
}

bool BiffFormulaParserImpl::importFuncVarToken4( BiffInputStream& rStrm )
{
    sal_uInt8 nParamCount;
    sal_uInt16 nFuncId;
    rStrm >> nParamCount >> nFuncId;
    return pushBiffFunction( nFuncId, nParamCount & BIFF_TOK_FUNCVAR_COUNTMASK );
}

bool BiffFormulaParserImpl::importFuncCEToken( BiffInputStream& rStrm )
{
    sal_uInt8 nParamCount, nFuncId;
    rStrm >> nParamCount >> nFuncId;
    sal_uInt16 nCmdId = nFuncId;
    setFlag( nCmdId, BIFF_TOK_FUNCVAR_CMD );
    return pushBiffFunction( nCmdId, nParamCount );
}

bool BiffFormulaParserImpl::importNlrAddrToken( BiffInputStream& rStrm, bool bRow )
{
    BiffNlr aNlr;
    aNlr.readBiff8Data( rStrm );
    return pushBiffNlrAddr( aNlr, bRow );
}

bool BiffFormulaParserImpl::importNlrRangeToken( BiffInputStream& rStrm )
{
    BiffNlr aNlr;
    aNlr.readBiff8Data( rStrm );
    rStrm.skip( 1 );
    BinRange aRange;
    rStrm >> aRange;
    return pushBiffNlrRange( aNlr, aRange );
}

bool BiffFormulaParserImpl::importNlrSAddrToken( BiffInputStream& rStrm, bool bRow )
{
    rStrm.skip( 4 );
    BiffNlr aNlr;
    return readNlrSAddrAddData( aNlr, rStrm, bRow ) ? pushBiffNlrSAddr( aNlr, bRow ) : pushBiffErrorOperand( BIFF_ERR_REF );
}

bool BiffFormulaParserImpl::importNlrSRangeToken( BiffInputStream& rStrm )
{
    rStrm.skip( 5 );
    BinRange aRange;
    rStrm >> aRange;
    BiffNlr aNlr;
    bool bRow;
    return readNlrSRangeAddData( aNlr, bRow, rStrm ) ? pushBiffNlrSRange( aNlr, aRange, bRow ) : pushBiffErrorOperand( BIFF_ERR_REF );
}

bool BiffFormulaParserImpl::importNlrErrToken( BiffInputStream& rStrm, sal_uInt16 nIgnore )
{
    rStrm.skip( nIgnore );
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

sal_Int32 BiffFormulaParserImpl::readRefId( BiffInputStream& rStrm )
{
    sal_Int16 nRefId;
    rStrm >> nRefId;
    rStrm.skip( mnRefIdSize );
    return nRefId;
}

sal_uInt16 BiffFormulaParserImpl::readNameId( BiffInputStream& rStrm )
{
    sal_uInt16 nNameId;
    rStrm >> nNameId;
    rStrm.skip( mnNameSize );
    return nNameId;
}

LinkSheetRange BiffFormulaParserImpl::readSheetRange5( BiffInputStream& rStrm )
{
    sal_Int32 nRefId = readRefId( rStrm );
    sal_Int16 nTab1, nTab2;
    rStrm >> nTab1 >> nTab2;
    return getExternalLinks().getSheetRange( nRefId, nTab1, nTab2 );
}

LinkSheetRange BiffFormulaParserImpl::readSheetRange8( BiffInputStream& rStrm )
{
    return getExternalLinks().getSheetRange( readRefId( rStrm ) );
}

void BiffFormulaParserImpl::swapStreamPosition( BiffInputStream& rStrm )
{
    sal_Int64 nRecPos = rStrm.tell();
    rStrm.seek( mnAddDataPos );
    mnAddDataPos = nRecPos;
}

bool BiffFormulaParserImpl::readNlrSAddrAddData( BiffNlr& orNlr, BiffInputStream& rStrm, bool bRow )
{
    bool bIsRow = false;
    return readNlrSRangeAddData( orNlr, bIsRow, rStrm ) && (bIsRow == bRow);
}

bool BiffFormulaParserImpl::readNlrSRangeAddData( BiffNlr& orNlr, bool& orbIsRow, BiffInputStream& rStrm )
{
    orbIsRow = false;
    swapStreamPosition( rStrm );
    // read number of cell addresses and relative flag
    sal_uInt32 nCount;
    rStrm >> nCount;
    bool bRel = getFlag( nCount, BIFF_TOK_NLR_ADDREL );
    nCount &= BIFF_TOK_NLR_ADDMASK;
    sal_Int64 nEndPos = rStrm.tell() + 4 * nCount;
    // read list of cell addresses
    bool bValid = false;
    if( nCount >= 2 )
    {
        // detect column/row orientation
        BinAddress aAddr1, aAddr2;
        rStrm >> aAddr1 >> aAddr2;
        orbIsRow = aAddr1.mnRow == aAddr2.mnRow;
        bValid = lclIsValidNlrStack( aAddr1, aAddr2, orbIsRow );
        // read and verify additional cell positions
        for( sal_uInt32 nIndex = 2; bValid && (nIndex < nCount); ++nIndex )
        {
            aAddr1 = aAddr2;
            rStrm >> aAddr2;
            bValid = !rStrm.isEof() && lclIsValidNlrStack( aAddr1, aAddr2, orbIsRow );
        }
        // check that last imported position (aAddr2) is not at the end of the sheet
        bValid = bValid && (orbIsRow ? (aAddr2.mnCol < mnMaxApiCol) : (aAddr2.mnRow < mnMaxApiRow));
        // fill the NLR struct with the last imported position
        if( bValid )
        {
            orNlr.mnCol = aAddr2.mnCol;
            orNlr.mnRow = aAddr2.mnRow;
            orNlr.mbRel = bRel;
        }
    }
    // seek to end of additional data for this token
    rStrm.seek( nEndPos );
    swapStreamPosition( rStrm );

    return bValid;
}

// convert BIFF token and push API operand or operator ------------------------

bool BiffFormulaParserImpl::pushBiffReference( const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    return (mnCurrRefId > 0) ?
        pushReferenceOperand( getExternalLinks().getSheetRange( mnCurrRefId, 0, 0 ), rRef, bDeleted, bRelativeAsOffset ) :
        pushReferenceOperand( rRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::pushBiffReference( const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    return (mnCurrRefId > 0) ?
        pushReferenceOperand( getExternalLinks().getSheetRange( mnCurrRefId, 0, 0 ), rRef, bDeleted, bRelativeAsOffset ) :
        pushReferenceOperand( rRef, bDeleted, bRelativeAsOffset );
}

bool BiffFormulaParserImpl::pushBiffNlrAddr( const BiffNlr& rNlr, bool bRow )
{
    BinSingleRef2d aRef;
    aRef.mnCol = rNlr.mnCol;
    aRef.mnRow = rNlr.mnRow;
    aRef.mbColRel = !bRow;
    aRef.mbRowRel = bRow;
    return pushNlrOperand( aRef );
}

bool BiffFormulaParserImpl::pushBiffNlrRange( const BiffNlr& rNlr, const BinRange& rRange )
{
    bool bRow = rNlr.mnRow == rRange.maFirst.mnRow;
    return lclIsValidNlrRange( rNlr, rRange, bRow ) ?
        pushBiffNlrAddr( rNlr, bRow ) : pushBiffErrorOperand( BIFF_ERR_REF );
}

bool BiffFormulaParserImpl::pushBiffNlrSAddr( const BiffNlr& rNlr, bool bRow )
{
    BinRange aRange;
    aRange.maFirst.mnCol = rNlr.mnCol + (bRow ? 1 : 0);
    aRange.maFirst.mnRow = rNlr.mnRow + (bRow ? 0 : 1);
    aRange.maLast.mnCol = bRow ? mnMaxApiCol : rNlr.mnCol;
    aRange.maLast.mnRow = bRow ? rNlr.mnRow : mnMaxApiRow;
    return pushBiffNlrSRange( rNlr, aRange, bRow );
}

bool BiffFormulaParserImpl::pushBiffNlrSRange( const BiffNlr& rNlr, const BinRange& rRange, bool bRow )
{
    if( lclIsValidNlrRange( rNlr, rRange, bRow ) )
    {
        BinComplexRef2d aRef;
        aRef.maRef1.mnCol = rRange.maFirst.mnCol;
        aRef.maRef1.mnRow = rRange.maFirst.mnRow;
        aRef.maRef2.mnCol = rRange.maLast.mnCol;
        aRef.maRef2.mnRow = rRange.maLast.mnRow;
        aRef.maRef1.mbColRel = aRef.maRef2.mbColRel = !bRow && rNlr.mbRel;
        aRef.maRef1.mbRowRel = aRef.maRef2.mbRowRel = bRow && rNlr.mbRel;
        return pushReferenceOperand( aRef, false, false );
    }
    return pushBiffErrorOperand( BIFF_ERR_REF );
}

bool BiffFormulaParserImpl::pushBiffName( sal_uInt16 nNameId )
{
    // one-based in BIFF formulas
    return pushDefinedNameOperand( getDefinedNames().getByIndex( static_cast< sal_Int32 >( nNameId ) - 1 ) );
}

bool BiffFormulaParserImpl::pushBiffExtName( sal_Int32 nRefId, sal_uInt16 nNameId )
{
    if( const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId ).get() )
    {
        if( pExtLink->getLinkType() == LINKTYPE_SELF )
            return pushBiffName( nNameId );
        // external name indexes are one-based in BIFF
        ExternalNameRef xExtName = pExtLink->getNameByIndex( static_cast< sal_Int32 >( nNameId ) - 1 );
        return pushExternalNameOperand( xExtName, *pExtLink );
    }
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool BiffFormulaParserImpl::pushBiffFunction( sal_uInt16 nFuncId )
{
    if( const FunctionInfo* pFuncInfo = getFuncInfoFromBiffFuncId( nFuncId ) )
        if( pFuncInfo->mnMinParamCount == pFuncInfo->mnMaxParamCount )
            return pushFunctionOperator( *pFuncInfo, pFuncInfo->mnMinParamCount );
    return pushFunctionOperator( OPCODE_NONAME, 0 );
}

bool BiffFormulaParserImpl::pushBiffFunction( sal_uInt16 nFuncId, sal_uInt8 nParamCount )
{
    if( getFlag( nFuncId, BIFF_TOK_FUNCVAR_CMD ) )
        nParamCount &= BIFF_TOK_FUNCVAR_COUNTMASK;
    if( const FunctionInfo* pFuncInfo = getFuncInfoFromBiffFuncId( nFuncId ) )
        return pushFunctionOperator( *pFuncInfo, nParamCount );
    return pushFunctionOperator( OPCODE_NONAME, nParamCount );
}

namespace {

/** Extracts the reference identifier and the remaining data from a formula in
    the format '[RefID]Remaining'. */
bool lclExtractRefId( sal_Int32& rnRefId, OUString& rRemainder, const OUString& rFormulaString )
{
    if( (rFormulaString.getLength() >= 4) && (rFormulaString[ 0 ] == '[') )
    {
        sal_Int32 nBracketClose = rFormulaString.indexOf( ']', 1 );
        if( nBracketClose >= 2 )
        {
            rnRefId = rFormulaString.copy( 1, nBracketClose - 1 ).toInt32();
            rRemainder = rFormulaString.copy( nBracketClose + 1 );
            return !rRemainder.isEmpty();
        }
    }
    return false;
}

}

FormulaParser::FormulaParser( const WorkbookHelper& rHelper ) :
    FormulaProcessorBase( rHelper )
{
    switch( getFilterType() )
    {
        case FILTER_OOXML:  mxImpl.reset( new OoxFormulaParserImpl( *this ) );  break;
        case FILTER_BIFF:   mxImpl.reset( new BiffFormulaParserImpl( *this ) ); break;
        case FILTER_UNKNOWN: break;
    }
}

FormulaParser::~FormulaParser()
{
}

ApiTokenSequence FormulaParser::importFormula( const CellAddress& rBaseAddress, const OUString& rFormulaString ) const
{
    return mxImpl->importOoxFormula( rBaseAddress, rFormulaString );
}

ApiTokenSequence FormulaParser::importFormula( const CellAddress& rBaseAddress, FormulaType eType, SequenceInputStream& rStrm ) const
{
    return mxImpl->importBiff12Formula( rBaseAddress, eType, rStrm );
}

OUString FormulaParser::importOleTargetLink( const OUString& rFormulaString )
{
    sal_Int32 nRefId = -1;
    OUString aRemainder;
    if( lclExtractRefId( nRefId, aRemainder, rFormulaString ) && (aRemainder.getLength() >= 3) &&
            (aRemainder[ 0 ] == '!') && (aRemainder[ 1 ] == '\'') && (aRemainder[ aRemainder.getLength() - 1 ] == '\'') )
        return mxImpl->resolveOleTarget( nRefId, false );
    return OUString();
}

OUString FormulaParser::importOleTargetLink( SequenceInputStream& rStrm )
{
    OUString aTargetLink;
    sal_Int32 nFmlaSize = rStrm.readInt32();
    sal_Int64 nFmlaEndPos = rStrm.tell() + ::std::max< sal_Int32 >( nFmlaSize, 0 );
    if( (nFmlaSize == 7) && (rStrm.getRemaining() >= 7) )
    {
        sal_uInt8 nToken = rStrm.readuChar();
        sal_Int16 nRefId = rStrm.readInt16();
        rStrm.skip(4); //nNameId
        if( nToken == (BIFF_TOKCLASS_VAL|BIFF_TOKID_NAMEX) )
            aTargetLink = mxImpl->resolveOleTarget( nRefId, true );
    }
    rStrm.seek( nFmlaEndPos );
    return aTargetLink;
}

OUString FormulaParser::importMacroName( const OUString& rFormulaString )
{
    /*  Valid macros are either sheet macros or VBA macros. OOXML and all BIFF
        documents store defined names for sheet macros, but OOXML documents do
        not store any defined name for VBA macros (while BIFF documents do).
        Sheet macros may be defined locally to a sheet, or globally to the
        document. As a result, all of the following macro specifiers are valid:

        1) Macros located in the own document:
            [0]!MySheetMacro    (global sheet macro 'MySheetMacro')
            Macro1!MyMacro      (sheet-local sheet macro 'MyMacro')
            [0]!MyVBAProc       (VBA macro 'MyVBAProc')
            [0]!Mod1.MyVBAProc  (VBA macro 'MyVBAProc' from code module 'Mod1')

        2) Macros from an external document:
            [2]!MySheetMacro    (global external sheet macro 'MySheetMacro')
            [2]Macro1!MyMacro   (sheet-local external sheet macro 'MyMacro')
            [2]!MyVBAProc       (external VBA macro 'MyVBAProc')
            [2]!Mod1.MyVBAProc  (external VBA macro from code module 'Mod1')

        This implementation is only interested in VBA macros from the own
        document, ignoring the valid syntax 'Macro1!MyMacro' for sheet-local
        sheet macros.
     */
    sal_Int32 nRefId = -1;
    OUString aRemainder;
    if( lclExtractRefId( nRefId, aRemainder, rFormulaString ) && (aRemainder.getLength() > 1) && (aRemainder[ 0 ] == '!') )
    {
        /*  In BIFF12 documents, the reference identifier is always the
            one-based index of the external link as it is in OOXML documents
            (it is not an index into the list of reference sheets as used in
            cell formulas). Index 0 is an implicit placeholder for the own
            document. In BIFF12 documents, the reference to the own document is
            stored explicitly, mostly at the top of the list, so index 1 may
            resolve to the own document too.
            Passing 'false' to getExternalLink() specifies to ignore the
            reference sheets list (if existing) and to access the list of
            external links directly. */
        const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId, false ).get();
        OSL_ENSURE( pExtLink, "FormulaParser::importMacroName - missing link" );
        // do not accept macros in external documents (not supported)
        if( pExtLink && (pExtLink->getLinkType() == LINKTYPE_SELF) )
        {
            // ignore sheet macros (defined name for VBA macros may not exist, see above)
            OUString aMacroName = aRemainder.copy( 1 );
            const DefinedName* pDefName = getDefinedNames().getByModelName( aMacroName ).get();
            if( !pDefName || pDefName->isVBName() )
                return aMacroName;
        }
    }
    return OUString();
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
