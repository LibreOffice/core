/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formulaparser.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:02:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/formulaparser.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/ReferenceFlags.hpp>
#include <com/sun/star/sheet/SingleReference.hpp>
#include <com/sun/star/sheet/ComplexReference.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/defnamesbuffer.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/tablebuffer.hxx"
#include "oox/xls/worksheethelper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::SingleReference;
using ::com::sun::star::sheet::ComplexReference;
using ::com::sun::star::sheet::XFormulaParser;
using namespace ::com::sun::star::sheet::ReferenceFlags;

namespace oox {
namespace xls {

// parser implementation base =================================================

class FormulaParserImpl : public WorkbookHelper
{
public:
    explicit            FormulaParserImpl(
                            const WorkbookHelper& rHelper,
                            const FunctionProvider& rFuncProv );

    /** Converts an XML formula string. */
    virtual void        importOoxFormula(
                            FormulaContext& rContext,
                            const OUString& rFormulaString );

    /** Imports and converts a OOBIN token array from the passed stream. */
    virtual void        importOobFormula(
                            FormulaContext& rContext,
                            RecordInputStream& rStrm );

    /** Imports and converts a BIFF token array from the passed stream. */
    virtual void        importBiffFormula(
                            FormulaContext& rContext,
                            BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize );

    /** Finalizes the passed token array after import (e.g. adjusts function
        parameters) and sets the formula using the passed context. */
    void                setFormula(
                            FormulaContext& rContext,
                            const ApiTokenSequence& rTokens );

protected:
    /** Sets the current formula context used for import. */
    inline FormulaContext& getFormulaContext() const { return *mpContext; }

    /** Sets the current formula context used for import. */
    void                initializeImport( FormulaContext& rContext );
    /** Finalizes the passed token array after import. */
    void                finalizeImport( const ApiTokenSequence& rTokens );
    /** Finalizes the internal token storage after import. */
    void                finalizeImport();

    /** Inserts a shared formula using the current formula context and passed base address. */
    void                setSharedFormula( const BinAddress& rBaseAddr );

    // token array ------------------------------------------------------------

    bool                resetSpaces();
    inline void         incLeadingSpaces( sal_Int32 nSpaces ) { mnLeadingSpaces += nSpaces; }
    inline void         incOpeningSpaces( sal_Int32 nSpaces ) { mnOpeningSpaces += nSpaces; }
    inline void         incClosingSpaces( sal_Int32 nSpaces ) { mnClosingSpaces += nSpaces; }

    size_t              getFormulaSize() const;
    Any&                appendRawToken( sal_Int32 nOpCode );
    Any&                insertRawToken( sal_Int32 nOpCode, size_t nIndexFromEnd );
    size_t              appendSpacesToken( sal_Int32 nSpaces );
    size_t              insertSpacesToken( sal_Int32 nSpaces, size_t nIndexFromEnd );

    size_t              getOperandSize( size_t nOpCountFromEnd, size_t nOpIndex ) const;
    void                pushOperandSize( size_t nSize );
    size_t              popOperandSize();

    ApiToken&           getOperandToken( size_t nOpCountFromEnd, size_t nOpIndex, size_t nTokenIndex );
    void                removeOperand( size_t nOpCountFromEnd, size_t nOpIndex );
    void                removeLastOperands( size_t nOpCountFromEnd );

    bool                pushOperandToken( sal_Int32 nOpCode, sal_Int32 nSpaces );
    bool                pushAnyOperandToken( const Any& rAny, sal_Int32 nOpCode, sal_Int32 nSpaces );
    template< typename Type >
    bool                pushValueOperandToken( const Type& rValue, sal_Int32 nOpCode, sal_Int32 nSpaces );
    template< typename Type >
    inline bool         pushValueOperandToken( const Type& rValue, sal_Int32 nSpaces )
                            { return pushValueOperandToken( rValue, mrFuncProv.OPCODE_PUSH, nSpaces ); }
    bool                pushParenthesesOperandToken( sal_Int32 nOpeningSpaces, sal_Int32 nClosingSpaces );
    bool                pushUnaryPreOperatorToken( sal_Int32 nOpCode, sal_Int32 nSpaces );
    bool                pushUnaryPostOperatorToken( sal_Int32 nOpCode, sal_Int32 nSpaces );
    bool                pushBinaryOperatorToken( sal_Int32 nOpCode, sal_Int32 nSpaces );
    bool                pushParenthesesOperatorToken( sal_Int32 nOpeningSpaces, sal_Int32 nClosingSpaces );
    bool                pushFunctionOperatorToken( sal_Int32 nOpCode, size_t nParamCount, sal_Int32 nLeadingSpaces, sal_Int32 nClosingSpaces );
    bool                pushFunctionOperatorToken( const FunctionInfo& rFuncInfo, size_t nParamCount, sal_Int32 nLeadingSpaces, sal_Int32 nClosingSpaces );

    bool                pushOperand( sal_Int32 nOpCode );
    bool                pushAnyOperand( const Any& rAny, sal_Int32 nOpCode );
    template< typename Type >
    bool                pushValueOperand( const Type& rValue, sal_Int32 nOpCode );
    template< typename Type >
    inline bool         pushValueOperand( const Type& rValue )
                            { return pushValueOperand( rValue, mrFuncProv.OPCODE_PUSH ); }
    bool                pushBoolOperand( bool bValue );
    bool                pushErrorOperand( double fEncodedError );
    bool                pushBiffErrorOperand( sal_uInt8 nErrorCode );
    bool                pushParenthesesOperand();
    bool                pushReferenceOperand( const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushReferenceOperand( const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset );
    bool                pushNlrOperand( const BinSingleRef2d& rRef );
    bool                pushEmbeddedRefOperand( const DefinedNameBase& rName );
    bool                pushDefinedNameOperand( const DefinedNameRef& rxDefName );
    bool                pushDdeLinkOperand( const OUString& rDdeServer, const OUString& rDdeTopic, const OUString& rDdeItem );
    bool                pushExternalNameOperand( const ExternalNameRef& rxExtName, ExternalLinkType eLinkType );

    bool                pushUnaryPreOperator( sal_Int32 nOpCode );
    bool                pushUnaryPostOperator( sal_Int32 nOpCode );
    bool                pushBinaryOperator( sal_Int32 nOpCode );
    bool                pushParenthesesOperator();
    bool                pushFunctionOperator( sal_Int32 nOpCode, size_t nParamCount );
    bool                pushFunctionOperator( const FunctionInfo& rFuncInfo, size_t nParamCount );

private:
    // reference conversion ---------------------------------------------------

    void                initReference2d( SingleReference& orApiRef ) const;
    void                initReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bRelSheet ) const;
    void                convertColRow( SingleReference& orApiRef, const BinSingleRef2d& rRef, bool bRelativeAsOffset ) const;
    void                convertReference( SingleReference& orApiRef, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference( ComplexReference& orApiRef, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference2d( SingleReference& orApiRef, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference2d( ComplexReference& orApiRef, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bRelSheet, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const;
    void                convertReference3d( ComplexReference& orApiRef, const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const;

    // finalize token sequence ------------------------------------------------

    typedef ::std::vector< const ApiToken* > ParameterPosVector;

    void                processTokens( const ApiToken* pToken, const ApiToken* pTokenEnd );
    const ApiToken*     processParameters( const FunctionInfo& rFuncInfo, const ApiToken* pToken, const ApiToken* pTokenEnd );

    bool                isEmptyParameter( const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    OUString            getExternCallParameter( const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    const ApiToken*     skipParentheses( const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    const ApiToken*     findParameters( ParameterPosVector& rParams, const ApiToken* pToken, const ApiToken* pTokenEnd ) const;
    void                appendCalcOnlyParameter( const FunctionInfo& rFuncInfo, size_t nParam );
    void                appendRequiredParameters( const FunctionInfo& rFuncInfo, size_t nParamCount );

    void                appendFinalToken( const ApiToken& rToken );
    Any&                appendFinalToken( sal_Int32 nOpCode );

protected:
    const FunctionProvider& mrFuncProv;             /// Function info provider.
    const sal_Int32     mnMaxApiCol;                /// Maximum column index in own document.
    const sal_Int32     mnMaxApiRow;                /// Maximum row index in own document.
    const sal_Int32     mnMaxXlsCol;                /// Maximum column index in imported document.
    const sal_Int32     mnMaxXlsRow;                /// Maximum row index in imported document.

private:
    typedef ::std::vector< ApiToken >   ApiTokenVector;
    typedef ::std::vector< size_t >     SizeTypeVector;

    ApiTokenVector      maTokenStorage;             /// Raw unordered token storage.
    SizeTypeVector      maTokenIndexes;             /// Indexes into maTokenStorage.
    SizeTypeVector      maOperandSizeStack;         /// Stack with token sizes per operand.
    FormulaContext*     mpContext;                  /// Current formula context.
    sal_Int32           mnLeadingSpaces;            /// Current number of spaces before next token.
    sal_Int32           mnOpeningSpaces;            /// Current number of spaces before opening parenthesis.
    sal_Int32           mnClosingSpaces;            /// Current number of spaces before closing parenthesis.
};

// ----------------------------------------------------------------------------

FormulaParserImpl::FormulaParserImpl( const WorkbookHelper& rHelper, const FunctionProvider& rFuncProv ) :
    WorkbookHelper( rHelper ),
    mrFuncProv( rFuncProv ),
    mnMaxApiCol( rHelper.getAddressConverter().getMaxApiAddress().Column ),
    mnMaxApiRow( rHelper.getAddressConverter().getMaxApiAddress().Row ),
    mnMaxXlsCol( rHelper.getAddressConverter().getMaxXlsAddress().Column ),
    mnMaxXlsRow( rHelper.getAddressConverter().getMaxXlsAddress().Row ),
    mpContext( 0 )
{
    maTokenStorage.reserve( 0x2000 );
    maTokenIndexes.reserve( 0x2000 );
    maOperandSizeStack.reserve( 256 );
    resetSpaces();
}

void FormulaParserImpl::importOoxFormula( FormulaContext&, const OUString& )
{
    OSL_ENSURE( false, "FormulaParserImpl::importOoxFormula - not implemented" );
}

void FormulaParserImpl::importOobFormula( FormulaContext&, RecordInputStream& )
{
    OSL_ENSURE( false, "FormulaParserImpl::importOobFormula - not implemented" );
}

void FormulaParserImpl::importBiffFormula( FormulaContext&, BiffInputStream&, const sal_uInt16* )
{
    OSL_ENSURE( false, "FormulaParserImpl::importBiffFormula - not implemented" );
}

void FormulaParserImpl::setFormula( FormulaContext& rContext, const ApiTokenSequence& rTokens )
{
    initializeImport( rContext );
    finalizeImport( rTokens );
}

void FormulaParserImpl::initializeImport( FormulaContext& rContext )
{
    maTokenStorage.clear();
    maTokenIndexes.clear();
    maOperandSizeStack.clear();
    mpContext = &rContext;
}

void FormulaParserImpl::finalizeImport( const ApiTokenSequence& rTokens )
{
    maTokenStorage.clear();
    const ApiToken* pToken = rTokens.getConstArray();
    processTokens( pToken, pToken + rTokens.getLength() );
    if( !maTokenStorage.empty() )
        mpContext->setTokens( ContainerHelper::vectorToSequence( maTokenStorage ) );
}

void FormulaParserImpl::finalizeImport()
{
    ApiTokenSequence aTokens( static_cast< sal_Int32 >( maTokenIndexes.size() ) );
    if( aTokens.hasElements() )
    {
        ApiToken* pToken = aTokens.getArray();
        for( SizeTypeVector::const_iterator aIt = maTokenIndexes.begin(), aEnd = maTokenIndexes.end(); aIt != aEnd; ++aIt, ++pToken )
            *pToken = maTokenStorage[ *aIt ];
    }
    finalizeImport( aTokens );
}

void FormulaParserImpl::setSharedFormula( const BinAddress& rBaseAddr )
{
    CellAddress aApiBaseAddr;
    if( getAddressConverter().convertToCellAddress( aApiBaseAddr, rBaseAddr, mpContext->getBaseAddress().Sheet, false ) )
        mpContext->setSharedFormula( aApiBaseAddr );
}

// token array ----------------------------------------------------------------

bool FormulaParserImpl::resetSpaces()
{
    mnLeadingSpaces = mnOpeningSpaces = mnClosingSpaces = 0;
    return true;
}

size_t FormulaParserImpl::getFormulaSize() const
{
    return maTokenIndexes.size();
}

Any& FormulaParserImpl::appendRawToken( sal_Int32 nOpCode )
{
    size_t nTokenIndex = maTokenStorage.size();
    maTokenStorage.resize( nTokenIndex + 1 );
    maTokenStorage.back().OpCode = nOpCode;
    maTokenIndexes.push_back( nTokenIndex );
    return maTokenStorage.back().Data;
}

Any& FormulaParserImpl::insertRawToken( sal_Int32 nOpCode, size_t nIndexFromEnd )
{
    size_t nTokenIndex = maTokenStorage.size();
    maTokenStorage.resize( nTokenIndex + 1 );
    maTokenStorage.back().OpCode = nOpCode;
    maTokenIndexes.insert( maTokenIndexes.end() - nIndexFromEnd, nTokenIndex );
    return maTokenStorage.back().Data;
}

size_t FormulaParserImpl::appendSpacesToken( sal_Int32 nSpaces )
{
    if( nSpaces > 0 )
    {
        appendRawToken( mrFuncProv.OPCODE_SPACES ) <<= nSpaces;
        return 1;
    }
    return 0;
}

size_t FormulaParserImpl::insertSpacesToken( sal_Int32 nSpaces, size_t nIndexFromEnd )
{
    if( nSpaces > 0 )
    {
        insertRawToken( mrFuncProv.OPCODE_SPACES, nIndexFromEnd ) <<= nSpaces;
        return 1;
    }
    return 0;
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
    OSL_ENSURE( getOperandSize( nOpCountFromEnd, nOpIndex ) > nTokenIndex,
        "FormulaParserImpl::getOperandToken - invalid parameters" );
    SizeTypeVector::const_iterator aIndexIt = maTokenIndexes.end();
    for( SizeTypeVector::const_iterator aEnd = maOperandSizeStack.end(), aIt = aEnd - nOpCountFromEnd + nOpIndex; aIt != aEnd; ++aIt )
        aIndexIt -= *aIt;
    return maTokenStorage[ *(aIndexIt + nTokenIndex) ];
}

void FormulaParserImpl::removeOperand( size_t nOpCountFromEnd, size_t nOpIndex )
{
    OSL_ENSURE( (nOpIndex < nOpCountFromEnd) && (nOpCountFromEnd <= maOperandSizeStack.size()),
        "FormulaParserImpl::removeOperand - invalid parameters" );
    // remove indexes into token storage, but do not touch storage itself
    SizeTypeVector::iterator aSizeEnd = maOperandSizeStack.end();
    SizeTypeVector::iterator aSizeIt = aSizeEnd - nOpCountFromEnd + nOpIndex;
    size_t nRemainingSize = 0;
    for( SizeTypeVector::iterator aIt = aSizeIt + 1; aIt != aSizeEnd; ++aIt )
        nRemainingSize += *aIt;
    maTokenIndexes.erase( maTokenIndexes.end() - nRemainingSize - *aSizeIt, maTokenIndexes.end() - nRemainingSize );
    maOperandSizeStack.erase( aSizeIt );
}

void FormulaParserImpl::removeLastOperands( size_t nOpCountFromEnd )
{
    for( size_t nOpIndex = 0; nOpIndex < nOpCountFromEnd; ++nOpIndex )
        removeOperand( 1, 0 );
}

bool FormulaParserImpl::pushOperandToken( sal_Int32 nOpCode, sal_Int32 nSpaces )
{
    size_t nSpacesSize = appendSpacesToken( nSpaces );
    appendRawToken( nOpCode );
    pushOperandSize( nSpacesSize + 1 );
    return true;
}

bool FormulaParserImpl::pushAnyOperandToken( const Any& rAny, sal_Int32 nOpCode, sal_Int32 nSpaces )
{
    size_t nSpacesSize = appendSpacesToken( nSpaces );
    appendRawToken( nOpCode ) = rAny;
    pushOperandSize( nSpacesSize + 1 );
    return true;
}

template< typename Type >
bool FormulaParserImpl::pushValueOperandToken( const Type& rValue, sal_Int32 nOpCode, sal_Int32 nSpaces )
{
    size_t nSpacesSize = appendSpacesToken( nSpaces );
    appendRawToken( nOpCode ) <<= rValue;
    pushOperandSize( nSpacesSize + 1 );
    return true;
}

bool FormulaParserImpl::pushParenthesesOperandToken( sal_Int32 nOpeningSpaces, sal_Int32 nClosingSpaces )
{
    size_t nSpacesSize = appendSpacesToken( nOpeningSpaces );
    appendRawToken( mrFuncProv.OPCODE_OPEN );
    nSpacesSize += appendSpacesToken( nClosingSpaces );
    appendRawToken( mrFuncProv.OPCODE_CLOSE );
    pushOperandSize( nSpacesSize + 2 );
    return true;
}

bool FormulaParserImpl::pushUnaryPreOperatorToken( sal_Int32 nOpCode, sal_Int32 nSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 1;
    if( bOk )
    {
        size_t nOpSize = popOperandSize();
        size_t nSpacesSize = insertSpacesToken( nSpaces, nOpSize );
        insertRawToken( nOpCode, nOpSize );
        pushOperandSize( nOpSize + nSpacesSize + 1 );
    }
    return bOk;
}

bool FormulaParserImpl::pushUnaryPostOperatorToken( sal_Int32 nOpCode, sal_Int32 nSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 1;
    if( bOk )
    {
        size_t nOpSize = popOperandSize();
        size_t nSpacesSize = appendSpacesToken( nSpaces );
        appendRawToken( nOpCode );
        pushOperandSize( nOpSize + nSpacesSize + 1 );
    }
    return bOk;
}

bool FormulaParserImpl::pushBinaryOperatorToken( sal_Int32 nOpCode, sal_Int32 nSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 2;
    if( bOk )
    {
        size_t nOp2Size = popOperandSize();
        size_t nOp1Size = popOperandSize();
        size_t nSpacesSize = insertSpacesToken( nSpaces, nOp2Size );
        insertRawToken( nOpCode, nOp2Size );
        pushOperandSize( nOp1Size + nSpacesSize + 1 + nOp2Size );
    }
    return bOk;
}

bool FormulaParserImpl::pushParenthesesOperatorToken( sal_Int32 nOpeningSpaces, sal_Int32 nClosingSpaces )
{
    bool bOk = maOperandSizeStack.size() >= 1;
    if( bOk )
    {
        size_t nOpSize = popOperandSize();
        size_t nSpacesSize = insertSpacesToken( nOpeningSpaces, nOpSize );
        insertRawToken( mrFuncProv.OPCODE_OPEN, nOpSize );
        nSpacesSize += appendSpacesToken( nClosingSpaces );
        appendRawToken( mrFuncProv.OPCODE_CLOSE );
        pushOperandSize( nOpSize + nSpacesSize + 2 );
    }
    return bOk;
}

bool FormulaParserImpl::pushFunctionOperatorToken( sal_Int32 nOpCode, size_t nParamCount, sal_Int32 nLeadingSpaces, sal_Int32 nClosingSpaces )
{
    /*  #i70925# if there are not enough tokens available on token stack, do
        not exit with error, but reduce parameter count. */
    nParamCount = ::std::min( maOperandSizeStack.size(), nParamCount );

    // convert all parameters on stack to a single operand separated with OPCODE_SEP
    bool bOk = true;
    for( size_t nParam = 1; bOk && (nParam < nParamCount); ++nParam )
        bOk = pushBinaryOperatorToken( mrFuncProv.OPCODE_SEP, 0 );

    // add function parentheses and function name
    return bOk &&
        ((nParamCount > 0) ? pushParenthesesOperatorToken( 0, nClosingSpaces ) : pushParenthesesOperandToken( 0, nClosingSpaces )) &&
        pushUnaryPreOperatorToken( nOpCode, nLeadingSpaces );
}

bool FormulaParserImpl::pushFunctionOperatorToken( const FunctionInfo& rFuncInfo, size_t nParamCount, sal_Int32 nLeadingSpaces, sal_Int32 nClosingSpaces )
{
    bool bOk = pushFunctionOperatorToken( rFuncInfo.mnApiOpCode, nParamCount, nLeadingSpaces, nClosingSpaces );
    // try to create an external add-in call for the passed built-in function
    if( bOk && (rFuncInfo.maExtProgName.getLength() > 0) )
        getOperandToken( 1, 0, 0 ).Data <<= rFuncInfo.maExtProgName;
    return bOk;
}

bool FormulaParserImpl::pushOperand( sal_Int32 nOpCode )
{
    return pushOperandToken( nOpCode, mnLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushAnyOperand( const Any& rAny, sal_Int32 nOpCode )
{
    return pushAnyOperandToken( rAny, nOpCode, mnLeadingSpaces ) && resetSpaces();
}

template< typename Type >
bool FormulaParserImpl::pushValueOperand( const Type& rValue, sal_Int32 nOpCode )
{
    return pushValueOperandToken( rValue, nOpCode, mnLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushBoolOperand( bool bValue )
{
    if( const FunctionInfo* pFuncInfo = mrFuncProv.getFuncInfoFromOobFuncId( bValue ? OOBIN_FUNC_TRUE : OOBIN_FUNC_FALSE ) )
        return pushFunctionOperator( pFuncInfo->mnApiOpCode, 0 );
    return pushValueOperand< double >( bValue ? 1.0 : 0.0 );
}

bool FormulaParserImpl::pushErrorOperand( double fEncodedError )
{
    // HACK: enclose all error codes into an 1x1 matrix
    // start token array with opening brace and leading spaces
    pushOperand( mrFuncProv.OPCODE_ARRAY_OPEN );
    size_t nOpSize = popOperandSize();
    size_t nOldArraySize = maTokenIndexes.size();
    // push a double containing the Calc error code
    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= fEncodedError;
    // close token array and set resulting operand size
    appendRawToken( mrFuncProv.OPCODE_ARRAY_CLOSE );
    pushOperandSize( nOpSize + maTokenIndexes.size() - nOldArraySize );
    return true;
}

bool FormulaParserImpl::pushBiffErrorOperand( sal_uInt8 nErrorCode )
{
    return pushErrorOperand( BiffHelper::calcDoubleFromError( nErrorCode ) );
}

bool FormulaParserImpl::pushParenthesesOperand()
{
    return pushParenthesesOperandToken( mnOpeningSpaces, mnClosingSpaces ) && resetSpaces();
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

bool FormulaParserImpl::pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    if( rSheetRange.is3dRange() )
    {
        // single-cell-range over several sheets, needs to create a ComplexReference
        ComplexReference aApiRef;
        convertReference3d( aApiRef, rSheetRange, rRef, rRef, bDeleted, bRelativeAsOffset );
        return pushValueOperand( aApiRef );
    }
    SingleReference aApiRef;
    convertReference3d( aApiRef, rSheetRange.mnFirst, rSheetRange.mbRel, rRef, bDeleted, bRelativeAsOffset );
    return pushValueOperand( aApiRef );
}

bool FormulaParserImpl::pushReferenceOperand( const LinkSheetRange& rSheetRange, const BinComplexRef2d& rRef, bool bDeleted, bool bRelativeAsOffset )
{
    ComplexReference aApiRef;
    convertReference3d( aApiRef, rSheetRange, rRef.maRef1, rRef.maRef2, bDeleted, bRelativeAsOffset );
    return pushValueOperand( aApiRef );
}

bool FormulaParserImpl::pushNlrOperand( const BinSingleRef2d& rRef )
{
    SingleReference aApiRef;
    convertReference2d( aApiRef, rRef, false, false );
    return pushValueOperand( aApiRef, mrFuncProv.OPCODE_NLR );
}

bool FormulaParserImpl::pushEmbeddedRefOperand( const DefinedNameBase& rName )
{
    Any aRefAny = rName.getReference( mpContext->getBaseAddress() );
    return aRefAny.hasValue() ? pushAnyOperand( aRefAny, mrFuncProv.OPCODE_PUSH ) : pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool FormulaParserImpl::pushDefinedNameOperand( const DefinedNameRef& rxDefName )
{
    if( !rxDefName )
        return pushBiffErrorOperand( BIFF_ERR_NAME );
    if( rxDefName->isMacroFunc( false ) )
        return pushValueOperand( rxDefName->getOoxName(), mrFuncProv.OPCODE_MACRO );
    if( rxDefName->getTokenIndex() >= 0 )
        return pushValueOperand( rxDefName->getTokenIndex(), mrFuncProv.OPCODE_NAME );
    return pushEmbeddedRefOperand( *rxDefName );
}

bool FormulaParserImpl::pushDdeLinkOperand( const OUString& rDdeServer, const OUString& rDdeTopic, const OUString& rDdeItem )
{
    // create the function call DDE("server";"topic";"item")
    return
        pushValueOperandToken( rDdeServer, 0 ) &&
        pushValueOperandToken( rDdeTopic, 0 ) &&
        pushValueOperandToken( rDdeItem, 0 ) &&
        pushFunctionOperator( mrFuncProv.OPCODE_DDE, 3 );
}

bool FormulaParserImpl::pushExternalNameOperand( const ExternalNameRef& rxExtName, ExternalLinkType eLinkType )
{
    if( rxExtName.get() ) switch( eLinkType )
    {
        case LINKTYPE_INTERNAL:
        case LINKTYPE_EXTERNAL:
            return pushEmbeddedRefOperand( *rxExtName );

        case LINKTYPE_ANALYSIS:
            if( const FunctionInfo* pFuncInfo = mrFuncProv.getFuncInfoFromOoxFuncName( rxExtName->getOoxName() ) )
                if( pFuncInfo->maExternCallName.getLength() > 0 )
                    return pushValueOperand( pFuncInfo->maExternCallName, mrFuncProv.OPCODE_MACRO );
        break;

        case LINKTYPE_DDE:
        {
            OUString aDdeServer, aDdeTopic, aDdeItem;
            if( rxExtName->getDdeLinkData( aDdeServer, aDdeTopic, aDdeItem ) )
                return pushDdeLinkOperand( aDdeServer, aDdeTopic, aDdeItem );
        }
        break;

        default:
            OSL_ENSURE( eLinkType != LINKTYPE_SELF, "FormulaParserImpl::pushExternalNameOperand - invalid call" );
    }
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool FormulaParserImpl::pushUnaryPreOperator( sal_Int32 nOpCode )
{
    return pushUnaryPreOperatorToken( nOpCode, mnLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushUnaryPostOperator( sal_Int32 nOpCode )
{
    return pushUnaryPostOperatorToken( nOpCode, mnLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushBinaryOperator( sal_Int32 nOpCode )
{
    return pushBinaryOperatorToken( nOpCode, mnLeadingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushParenthesesOperator()
{
    return pushParenthesesOperatorToken( mnOpeningSpaces, mnClosingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushFunctionOperator( sal_Int32 nOpCode, size_t nParamCount )
{
    return pushFunctionOperatorToken( nOpCode, nParamCount, mnLeadingSpaces, mnClosingSpaces ) && resetSpaces();
}

bool FormulaParserImpl::pushFunctionOperator( const FunctionInfo& rFuncInfo, size_t nParamCount )
{
    return pushFunctionOperatorToken( rFuncInfo, nParamCount, mnLeadingSpaces, mnClosingSpaces ) && resetSpaces();
}

// reference conversion -------------------------------------------------------

void FormulaParserImpl::initReference2d( SingleReference& orApiRef ) const
{
    if( mpContext->is2dRefsAs3dRefs() )
    {
        initReference3d( orApiRef, mpContext->getBaseAddress().Sheet, false );
    }
    else
    {
        orApiRef.Flags = SHEET_RELATIVE;
        // #i10184# absolute sheet index needed for relative references in shared formulas
        orApiRef.Sheet = mpContext->getBaseAddress().Sheet;
        orApiRef.RelativeSheet = 0;
    }
}

void FormulaParserImpl::initReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bRelSheet ) const
{
    orApiRef.Flags = SHEET_3D;
    if( nSheet < 0 )
    {
        orApiRef.Sheet = 0;
        orApiRef.Flags |= SHEET_DELETED;
    }
    else if( bRelSheet )
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
                orApiRef.RelativeColumn -= mpContext->getBaseAddress().Column;
            if( rRef.mbRowRel )
                orApiRef.RelativeRow -= mpContext->getBaseAddress().Row;
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

void FormulaParserImpl::convertReference3d( SingleReference& orApiRef, sal_Int32 nSheet, bool bRelSheet, const BinSingleRef2d& rRef, bool bDeleted, bool bRelativeAsOffset ) const
{
    initReference3d( orApiRef, nSheet, bRelSheet );
    convertReference( orApiRef, rRef, bDeleted, bRelativeAsOffset );
}

void FormulaParserImpl::convertReference3d( ComplexReference& orApiRef, const LinkSheetRange& rSheetRange, const BinSingleRef2d& rRef1, const BinSingleRef2d& rRef2, bool bDeleted, bool bRelativeAsOffset ) const
{
    initReference3d( orApiRef.Reference1, rSheetRange.mnFirst, rSheetRange.mbRel );
    initReference3d( orApiRef.Reference2, rSheetRange.mnLast, rSheetRange.mbRel );
    convertReference( orApiRef, rRef1, rRef2, bDeleted, bRelativeAsOffset );
    // remove sheet name from second part of reference
    setFlag( orApiRef.Reference2.Flags, SHEET_3D, rSheetRange.is3dRange() );
}

// finalize token sequence ----------------------------------------------------

void FormulaParserImpl::processTokens( const ApiToken* pToken, const ApiToken* pTokenEnd )
{
    while( pToken < pTokenEnd )
    {
        // push the current token into the vector
        appendFinalToken( *pToken );
        // try to process a function, otherwise go to next token
        if( const FunctionInfo* pFuncInfo = mrFuncProv.getFuncInfoFromApiToken( *pToken ) )
            pToken = processParameters( *pFuncInfo, pToken + 1, pTokenEnd );
        else
            ++pToken;
    }
}

const ApiToken* FormulaParserImpl::processParameters(
        const FunctionInfo& rFuncInfo, const ApiToken* pToken, const ApiToken* pTokenEnd )
{
    // remember position of the token containing the function op-code
    size_t nFuncNameIdx = maTokenStorage.size() - 1;

    // process a function, if an OPCODE_OPEN token is following
    OSL_ENSURE( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_OPEN), "FormulaParserImpl::processParameters - OPCODE_OPEN expected" );
    if( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_OPEN) )
    {
        // append the OPCODE_OPEN token to the vector
        appendFinalToken( mrFuncProv.OPCODE_OPEN );

        // store positions of OPCODE_OPEN, parameter separators, and OPCODE_CLOSE
        ParameterPosVector aParams;
        pToken = findParameters( aParams, pToken, pTokenEnd );
        OSL_ENSURE( aParams.size() >= 2, "FormulaParserImpl::processParameters - missing tokens" );
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

            // preprocess add-ins, first parameter is reference to function name
            if( rFuncInfo.mnOobFuncId == OOBIN_FUNC_EXTERNCALL )
            {
                OUString aName = getExternCallParameter( *aPosIt + 1, *(aPosIt + 1) );
                if( const FunctionInfo* pExtFuncInfo = mrFuncProv.getFuncInfoFromExternCallName( aName ) )
                {
                    maTokenStorage[ nFuncNameIdx ].OpCode = pExtFuncInfo->mnApiOpCode;
                    // insert programmatic add-in function name
                    if( pExtFuncInfo->mnApiOpCode == mrFuncProv.OPCODE_EXTERNAL )
                        maTokenStorage[ nFuncNameIdx ].Data <<= pExtFuncInfo->maExtProgName;
                    // prepare for following parameters
                    pRealFuncInfo = pExtFuncInfo;
                    --nParamCount;
                    ++aPosIt;
                }
            }

            // process all parameters
            FuncInfoParamClassIterator aClassIt( *pRealFuncInfo );
            size_t nLastValidSize = maTokenStorage.size();
            size_t nLastValidCount = 0;
            for( size_t nParam = 0; nParam < nParamCount; ++nParam, ++aPosIt, ++aClassIt )
            {
                // add embedded Calc-only parameters
                if( aClassIt.isCalcOnlyParam() )
                {
                    appendCalcOnlyParameter( *pRealFuncInfo, nParam );
                    while( aClassIt.isCalcOnlyParam() ) ++aClassIt;
                }

                const ApiToken* pParamBegin = *aPosIt + 1;
                const ApiToken* pParamEnd = *(aPosIt + 1);
                bool bIsEmpty = isEmptyParameter( pParamBegin, pParamEnd );

                if( !aClassIt.isExcelOnlyParam() )
                {
                    // replace empty second and third parameter in IF function with zeros
                    if( (pRealFuncInfo->mnOobFuncId == OOBIN_FUNC_IF) && ((nParam == 1) || (nParam == 2)) && bIsEmpty )
                    {
                        appendFinalToken( mrFuncProv.OPCODE_PUSH ) <<= static_cast< double >( 0.0 );
                        bIsEmpty = false;
                    }
                    else
                    {
                        // process all tokens of the parameter
                        processTokens( pParamBegin, pParamEnd );
                    }
                    // append parameter separator token
                    appendFinalToken( mrFuncProv.OPCODE_SEP );
                }

                /*  #84453# Update size of new token sequence with valid parameters
                    to be able to remove trailing optional empty parameters. */
                if( !bIsEmpty || (nParam < pRealFuncInfo->mnMinParamCount) )
                {
                    nLastValidSize = maTokenStorage.size();
                    nLastValidCount = nParam + 1;
                }
            }

            // #84453# remove trailing optional empty parameters
            maTokenStorage.resize( nLastValidSize );

            // add trailing Calc-only parameters
            if( aClassIt.isCalcOnlyParam() )
                appendCalcOnlyParameter( *pRealFuncInfo, nLastValidCount );

            // add optional parameters that are required in Calc
            appendRequiredParameters( *pRealFuncInfo, nLastValidCount );

            // remove last parameter separator token
            if( maTokenStorage.back().OpCode == mrFuncProv.OPCODE_SEP )
                maTokenStorage.pop_back();
        }

        /*  Append the OPCODE_CLOSE token to the vector, but only if there is
            no OPCODE_BAD token at the end, this token already contains the
            trailing closing parentheses. */
        if( (pTokenEnd - 1)->OpCode != mrFuncProv.OPCODE_BAD )
            appendFinalToken( mrFuncProv.OPCODE_CLOSE );
    }

    /*  Replace OPCODE_EXTERNAL with OPCODE_NONAME to get #NAME! error in cell,
        if no matching add-in function was found. */
    ApiToken& rFuncNameToken = maTokenStorage[ nFuncNameIdx ];
    if( (rFuncNameToken.OpCode == mrFuncProv.OPCODE_EXTERNAL) && !rFuncNameToken.Data.hasValue() )
        rFuncNameToken.OpCode = mrFuncProv.OPCODE_NONAME;

    return pToken;
}

bool FormulaParserImpl::isEmptyParameter( const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    while( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_SPACES) ) ++pToken;
    if( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_MISSING) ) ++pToken;
    while( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_SPACES) ) ++pToken;
    return pToken == pTokenEnd;
}

OUString FormulaParserImpl::getExternCallParameter( const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    OUString aExtCallName;
    while( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_SPACES) ) ++pToken;
    if( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_MACRO) ) (pToken++)->Data >>= aExtCallName;
    while( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_SPACES) ) ++pToken;
    return (pToken == pTokenEnd) ? aExtCallName : OUString();
}

const ApiToken* FormulaParserImpl::skipParentheses( const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    // skip tokens between OPCODE_OPEN and OPCODE_CLOSE
    OSL_ENSURE( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_OPEN), "skipParentheses - OPCODE_OPEN expected" );
    ++pToken;
    while( (pToken < pTokenEnd) && (pToken->OpCode != mrFuncProv.OPCODE_CLOSE) )
    {
        if( pToken->OpCode == mrFuncProv.OPCODE_OPEN )
            pToken = skipParentheses( pToken, pTokenEnd );
        else
            ++pToken;
    }
    // skip the OPCODE_CLOSE token
    OSL_ENSURE( ((pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_CLOSE)) || ((pTokenEnd - 1)->OpCode == mrFuncProv.OPCODE_BAD), "skipParentheses - OPCODE_CLOSE expected" );
    return (pToken < pTokenEnd) ? (pToken + 1) : pTokenEnd;
}

const ApiToken* FormulaParserImpl::findParameters( ParameterPosVector& rParams,
        const ApiToken* pToken, const ApiToken* pTokenEnd ) const
{
    // push position of OPCODE_OPEN
    OSL_ENSURE( (pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_OPEN), "FormulaParserImpl::findParameters - OPCODE_OPEN expected" );
    rParams.push_back( pToken++ );

    // find positions of parameter separators
    while( (pToken < pTokenEnd) && (pToken->OpCode != mrFuncProv.OPCODE_CLOSE) )
    {
        if( pToken->OpCode == mrFuncProv.OPCODE_OPEN )
            pToken = skipParentheses( pToken, pTokenEnd );
        else if( pToken->OpCode == mrFuncProv.OPCODE_SEP )
            rParams.push_back( pToken++ );
        else
            ++pToken;
    }

    // push position of OPCODE_CLOSE
    OSL_ENSURE( ((pToken < pTokenEnd) && (pToken->OpCode == mrFuncProv.OPCODE_CLOSE)) || ((pTokenEnd - 1)->OpCode == mrFuncProv.OPCODE_BAD), "FormulaParserImpl::findParameters - OPCODE_CLOSE expected" );
    rParams.push_back( pToken );
    return (pToken < pTokenEnd) ? (pToken + 1) : pTokenEnd;
}

void FormulaParserImpl::appendCalcOnlyParameter( const FunctionInfo& rFuncInfo, size_t nParam )
{
    (void)nParam;   // prevent 'unused' warning
    switch( rFuncInfo.mnOobFuncId )
    {
        case OOBIN_FUNC_FLOOR:
        case OOBIN_FUNC_CEILING:
            OSL_ENSURE( nParam == 2, "FormulaParserImpl::appendCalcOnlyParameter - unexpected parameter index" );
            appendFinalToken( mrFuncProv.OPCODE_PUSH ) <<= static_cast< double >( 1.0 );
            appendFinalToken( mrFuncProv.OPCODE_SEP );
        break;
    }
}

void FormulaParserImpl::appendRequiredParameters( const FunctionInfo& rFuncInfo, size_t nParamCount )
{
    switch( rFuncInfo.mnOobFuncId )
    {
        case OOBIN_FUNC_WEEKNUM:
            if( nParamCount == 1 )
            {
                appendFinalToken( mrFuncProv.OPCODE_PUSH ) <<= static_cast< double >( 1.0 );
                appendFinalToken( mrFuncProv.OPCODE_SEP );
            }
        break;
    }
}

void FormulaParserImpl::appendFinalToken( const ApiToken& rToken )
{
    if( (rToken.OpCode == mrFuncProv.OPCODE_MACRO) && !rToken.Data.hasValue() )
    {
        appendFinalToken( mrFuncProv.OPCODE_ARRAY_OPEN );
        appendFinalToken( mrFuncProv.OPCODE_PUSH ) <<= BiffHelper::calcDoubleFromError( BIFF_ERR_NAME );
        appendFinalToken( mrFuncProv.OPCODE_ARRAY_CLOSE );
    }
    else
        maTokenStorage.push_back( rToken );
}

Any& FormulaParserImpl::appendFinalToken( sal_Int32 nOpCode )
{
    maTokenStorage.resize( maTokenStorage.size() + 1 );
    maTokenStorage.back().OpCode = nOpCode;
    return maTokenStorage.back().Data;
}

// OOX parser implementation ==================================================

class OoxFormulaParserImpl : public FormulaParserImpl
{
public:
    explicit            OoxFormulaParserImpl(
                            const WorkbookHelper& rHelper,
                            const FunctionProvider& rFuncProv );

    virtual void        importOoxFormula(
                            FormulaContext& rContext,
                            const OUString& rFormulaString );

    virtual void        importOobFormula(
                            FormulaContext& rContext,
                            RecordInputStream& rStrm );

private:
    // import token contents and create API formula token ---------------------

    bool                importAttrToken( RecordInputStream& rStrm );
    bool                importSpaceToken( RecordInputStream& rStrm );
    bool                importTableToken( RecordInputStream& rStrm );
    bool                importArrayToken( RecordInputStream& rStrm );
    bool                importRefToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importAreaToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRef3dToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importArea3dToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importMemAreaToken( RecordInputStream& rStrm, bool bAddData );
    bool                importMemFuncToken( RecordInputStream& rStrm );
    bool                importNameToken( RecordInputStream& rStrm );
    bool                importNameXToken( RecordInputStream& rStrm );
    bool                importFuncToken( RecordInputStream& rStrm );
    bool                importFuncVarToken( RecordInputStream& rStrm );
    bool                importExpToken( RecordInputStream& rStrm );

    LinkSheetRange      readSheetRange( RecordInputStream& rStrm );

    void                swapStreamPosition( RecordInputStream& rStrm );
    void                skipMemAreaAddData( RecordInputStream& rStrm );

    // convert BIN token and push API operand or operator ---------------------

    bool                pushOobName( sal_Int32 nNameId );
    bool                pushOobExtName( sal_Int32 nRefId, sal_Int32 nNameId );
    bool                pushOobFunction( sal_uInt16 nFuncId );
    bool                pushOobFunction( sal_uInt16 nFuncId, sal_uInt8 nParamCount );

private:
    Reference< XFormulaParser > mxParser;
    PropertySet         maParserProps;
    const OUString      maRefPosProp;
    sal_Int32           mnAddDataPos;               /// Current stream position for additional data (tExp, tArray, tMemArea).
};

// ----------------------------------------------------------------------------

OoxFormulaParserImpl::OoxFormulaParserImpl( const WorkbookHelper& rHelper, const FunctionProvider& rFuncProv ) :
    FormulaParserImpl( rHelper, rFuncProv ),
    maRefPosProp( CREATE_OUSTRING( "ReferencePosition" ) ),
    mnAddDataPos( 0 )
{
    try
    {
        Reference< XMultiServiceFactory > xFactory( getDocument(), UNO_QUERY_THROW );
        mxParser.set( xFactory->createInstance( CREATE_OUSTRING( "com.sun.star.sheet.FormulaParser" ) ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxParser.is(), "OoxFormulaParserImpl::OoxFormulaParserImpl - cannot create formula parser" );
    maParserProps.set( mxParser );
    maParserProps.setProperty( CREATE_OUSTRING( "CompileEnglish" ), true );
    maParserProps.setProperty( CREATE_OUSTRING( "R1C1Notation" ), false );
    maParserProps.setProperty( CREATE_OUSTRING( "Compatibility3DNotation" ), true );
    maParserProps.setProperty( CREATE_OUSTRING( "IgnoreLeadingSpaces" ), false );
    maParserProps.setProperty( CREATE_OUSTRING( "OpCodeMap" ), mrFuncProv.getOoxParserMap() );
}

void OoxFormulaParserImpl::importOoxFormula(
        FormulaContext& rContext, const OUString& rFormulaString )
{
    if( mxParser.is() )
    {
        initializeImport( rContext );
        maParserProps.setProperty( maRefPosProp, getFormulaContext().getBaseAddress() );
        finalizeImport( mxParser->parseFormula( rFormulaString ) );
    }
}

void OoxFormulaParserImpl::importOobFormula( FormulaContext& rContext, RecordInputStream& rStrm )
{
    initializeImport( rContext );

    sal_Int32 nFmlaSize = rStrm.readInt32();
    sal_Int32 nFmlaPos = rStrm.getRecPos();
    sal_Int32 nFmlaEndPos = nFmlaPos + nFmlaSize;

    rStrm.seek( nFmlaEndPos );
    sal_Int32 nAddDataSize = rStrm.readInt32();
    mnAddDataPos = rStrm.getRecPos();
    sal_Int32 nAddDataEndPos = mnAddDataPos + nAddDataSize;
    rStrm.seek( nFmlaPos );

    bool bOk = (nFmlaSize >= 0) && (nAddDataSize >= 0);
    bool bRelativeAsOffset = getFormulaContext().isRelativeAsOffset();

    while( bOk && rStrm.isValid() && (rStrm.getRecPos() < nFmlaEndPos) )
    {
        sal_uInt8 nTokenId;
        rStrm >> nTokenId;
        sal_uInt8 nTokenClass = nTokenId & BIFF_TOKCLASS_MASK;
        sal_uInt8 nBaseId = nTokenId & BIFF_TOKID_MASK;

        if( nTokenClass == BIFF_TOKCLASS_NONE )
        {
            // base tokens
            switch( nBaseId )
            {
                case BIFF_TOKID_EXP:        bOk = importExpToken( rStrm );                                      break;
                case BIFF_TOKID_ADD:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_ADD );                  break;
                case BIFF_TOKID_SUB:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_SUB );                  break;
                case BIFF_TOKID_MUL:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_MULT );                 break;
                case BIFF_TOKID_DIV:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_DIV );                  break;
                case BIFF_TOKID_POWER:      bOk = pushBinaryOperator( mrFuncProv.OPCODE_POWER );                break;
                case BIFF_TOKID_CONCAT:     bOk = pushBinaryOperator( mrFuncProv.OPCODE_CONCAT );               break;
                case BIFF_TOKID_LT:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_LESS );                 break;
                case BIFF_TOKID_LE:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_LESS_EQUAL );           break;
                case BIFF_TOKID_EQ:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_EQUAL );                break;
                case BIFF_TOKID_GE:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_GREATER_EQUAL );        break;
                case BIFF_TOKID_GT:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_GREATER );              break;
                case BIFF_TOKID_NE:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_NOT_EQUAL );            break;
                case BIFF_TOKID_ISECT:      bOk = pushBinaryOperator( mrFuncProv.OPCODE_INTERSECT );            break;
                case BIFF_TOKID_LIST:       bOk = pushBinaryOperator( mrFuncProv.OPCODE_LIST );                 break;
                case BIFF_TOKID_RANGE:      bOk = pushBinaryOperator( mrFuncProv.OPCODE_RANGE );                break;
                case BIFF_TOKID_UPLUS:      bOk = pushUnaryPreOperator( mrFuncProv.OPCODE_PLUS_SIGN );          break;
                case BIFF_TOKID_UMINUS:     bOk = pushUnaryPreOperator( mrFuncProv.OPCODE_MINUS_SIGN );         break;
                case BIFF_TOKID_PERCENT:    bOk = pushUnaryPostOperator( mrFuncProv.OPCODE_PERCENT );           break;
                case BIFF_TOKID_PAREN:      bOk = pushParenthesesOperator();                                    break;
                case BIFF_TOKID_MISSARG:    bOk = pushOperand( mrFuncProv.OPCODE_MISSING );                     break;
                case BIFF_TOKID_STR:        bOk = pushValueOperand( rStrm.readString( false ) );                break;
                case BIFF_TOKID_NLR:        bOk = importTableToken( rStrm );                                    break;
                case BIFF_TOKID_ATTR:       bOk = importAttrToken( rStrm );                                     break;
                case BIFF_TOKID_ERR:        bOk = pushBiffErrorOperand( rStrm.readuInt8() );                    break;
                case BIFF_TOKID_BOOL:       bOk = pushBoolOperand( rStrm.readuInt8() != BIFF_TOK_BOOL_FALSE );  break;
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
    if( bOk && (rStrm.getRecPos() == nFmlaEndPos) && (mnAddDataPos == nAddDataEndPos) )
        finalizeImport();

    // seek behind token array
    if( (nFmlaSize >= 0) && (nAddDataSize >= 0) )
        rStrm.seek( nAddDataEndPos );
}

// import token contents and create API formula token -------------------------

bool OoxFormulaParserImpl::importAttrToken( RecordInputStream& rStrm )
{
    bool bOk = true;
    sal_uInt8 nType;
    rStrm >> nType;
    // equal flags in BIFF and OOBIN
    switch( nType )
    {
        case OOBIN_TOK_ATTR_VOLATILE:
        case OOBIN_TOK_ATTR_IF:
        case OOBIN_TOK_ATTR_SKIP:
        case OOBIN_TOK_ATTR_ASSIGN:
        case OOBIN_TOK_ATTR_IFERROR:
            rStrm.skip( 2 );
        break;
        case OOBIN_TOK_ATTR_CHOOSE:
            rStrm.skip( 2 * rStrm.readuInt16() + 2 );
        break;
        case OOBIN_TOK_ATTR_SUM:
            rStrm.skip( 2 );
            bOk = pushOobFunction( OOBIN_FUNC_SUM, 1 );
        break;
        case OOBIN_TOK_ATTR_SPACE:
        case OOBIN_TOK_ATTR_SPACE_VOLATILE:
            bOk = importSpaceToken( rStrm );
        break;
        default:
            bOk = false;
    }
    return bOk;
}

bool OoxFormulaParserImpl::importSpaceToken( RecordInputStream& rStrm )
{
    // equal constants in BIFF and OOX
    sal_uInt8 nType, nCount;
    rStrm >> nType >> nCount;
    switch( nType )
    {
        case BIFF_TOK_ATTR_SPACE_SP:
        case BIFF_TOK_ATTR_SPACE_BR:
            incLeadingSpaces( nCount );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_OPEN:
        case BIFF_TOK_ATTR_SPACE_BR_OPEN:
            incOpeningSpaces( nCount );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_CLOSE:
        case BIFF_TOK_ATTR_SPACE_BR_CLOSE:
            incClosingSpaces( nCount );
        break;
    }
    return true;
}

bool OoxFormulaParserImpl::importTableToken( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags, nTableId, nCol1, nCol2;
    rStrm.skip( 3 );
    rStrm >> nFlags >> nTableId;
    rStrm.skip( 2 );
    rStrm >> nCol1 >> nCol2;
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

        bool bSingleCol = getFlag( nFlags, OOBIN_TOK_TABLE_COLUMN );
        bool bColRange = getFlag( nFlags, OOBIN_TOK_TABLE_COLRANGE );
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
            bool bAllRows    = getFlag( nFlags, OOBIN_TOK_TABLE_ALL );
            bool bHeaderRows = getFlag( nFlags, OOBIN_TOK_TABLE_HEADERS );
            bool bDataRows   = getFlag( nFlags, OOBIN_TOK_TABLE_DATA );
            bool bTotalsRows = getFlag( nFlags, OOBIN_TOK_TABLE_TOTALS );
            bool bThisRow    = getFlag( nFlags, OOBIN_TOK_TABLE_THISROW );

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
                    nStartRow = nEndRow = getFormulaContext().getBaseAddress().Row - xTable->getRange().StartRow;
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
                return pushValueOperand( nTokenIndex, mrFuncProv.OPCODE_DBAREA );
            // create an OFFSET function call to refer to a subrange of the table
            const FunctionInfo* pRowsInfo = mrFuncProv.getFuncInfoFromOobFuncId( OOBIN_FUNC_ROWS );
            const FunctionInfo* pColumnsInfo = mrFuncProv.getFuncInfoFromOobFuncId( OOBIN_FUNC_COLUMNS );
            return
                pRowsInfo && pColumnsInfo &&
                pushValueOperandToken( nTokenIndex, mrFuncProv.OPCODE_DBAREA, 0 ) &&
                (bFixedStartRow ?
                    pushValueOperandToken< double >( nStartRow, 0 ) :
                    (pushValueOperandToken( nTokenIndex, mrFuncProv.OPCODE_DBAREA, 0 ) &&
                     pushFunctionOperatorToken( *pRowsInfo, 1, 0, 0 ) &&
                     pushValueOperandToken< double >( nHeight - nStartRow, 0 ) &&
                     pushBinaryOperatorToken( mrFuncProv.OPCODE_SUB, 0 ))) &&
                pushValueOperandToken< double >( nStartCol, 0 ) &&
                (bFixedHeight ?
                    pushValueOperandToken< double >( nEndRow - nStartRow + 1, 0 ) :
                    (pushValueOperandToken( nTokenIndex, mrFuncProv.OPCODE_DBAREA, 0 ) &&
                     pushFunctionOperatorToken( *pRowsInfo, 1, 0, 0 ) &&
                     (((nStartRow == 0) && (nEndRow + 1 == nHeight)) ||
                      (pushValueOperandToken< double >( nHeight - (nEndRow - nStartRow + 1), 0 ) &&
                       pushBinaryOperatorToken( mrFuncProv.OPCODE_SUB, 0 ))))) &&
                (((nStartCol == 0) && (nEndCol + 1 == nWidth)) ?
                    (pushValueOperandToken( nTokenIndex, mrFuncProv.OPCODE_DBAREA, 0 ) &&
                     pushFunctionOperatorToken( *pColumnsInfo, 1, 0, 0 )) :
                    pushValueOperandToken< double >( nEndCol - nStartCol + 1, 0 )) &&
                pushOobFunction( OOBIN_FUNC_OFFSET, 5 );
        }
    }
    return pushBiffErrorOperand( BIFF_ERR_REF );
}

bool OoxFormulaParserImpl::importArrayToken( RecordInputStream& rStrm )
{
    rStrm.skip( 14 );

    // start token array with opening brace and leading spaces
    pushOperand( mrFuncProv.OPCODE_ARRAY_OPEN );
    size_t nOpSize = popOperandSize();
    size_t nOldArraySize = getFormulaSize();

    // read array size
    swapStreamPosition( rStrm );
    sal_Int32 nRows = rStrm.readInt32();
    sal_Int32 nCols = rStrm.readInt32();
    OSL_ENSURE( (nCols > 0) && (nRows > 0), "OoxFormulaParserImpl::importArrayToken - empty array" );

    // read array values and build token array
    for( sal_Int32 nRow = 0; rStrm.isValid() && (nRow < nRows); ++nRow )
    {
        if( nRow > 0 )
            appendRawToken( mrFuncProv.OPCODE_ARRAY_ROWSEP );
        for( sal_Int32 nCol = 0; rStrm.isValid() && (nCol < nCols); ++nCol )
        {
            if( nCol > 0 )
                appendRawToken( mrFuncProv.OPCODE_ARRAY_COLSEP );
            switch( rStrm.readuInt8() )
            {
                case OOBIN_TOK_ARRAY_DOUBLE:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= rStrm.readDouble();
                break;
                case OOBIN_TOK_ARRAY_STRING:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= rStrm.readString( false );
                break;
                case OOBIN_TOK_ARRAY_BOOL:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= static_cast< double >( (rStrm.readuInt8() == 0) ? 0.0 : 1.0 );
                break;
                case OOBIN_TOK_ARRAY_ERROR:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= BiffHelper::calcDoubleFromError( rStrm.readuInt8() );
                    rStrm.skip( 3 );
                break;
                default:
                    OSL_ENSURE( false, "OoxFormulaParserImpl::importArrayToken - unknown data type" );
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= BiffHelper::calcDoubleFromError( BIFF_ERR_NA );
            }
        }
    }
    swapStreamPosition( rStrm );

    // close token array and set resulting operand size
    appendRawToken( mrFuncProv.OPCODE_ARRAY_CLOSE );
    pushOperandSize( nOpSize + getFormulaSize() - nOldArraySize );
    return true;
}

bool OoxFormulaParserImpl::importRefToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinSingleRef2d aRef;
    aRef.readOobData( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importAreaToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    BinComplexRef2d aRef;
    aRef.readOobData( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importRef3dToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange( rStrm );
    BinSingleRef2d aRef;
    aRef.readOobData( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importArea3dToken( RecordInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset )
{
    LinkSheetRange aSheetRange = readSheetRange( rStrm );
    BinComplexRef2d aRef;
    aRef.readOobData( rStrm, bRelativeAsOffset );
    return pushReferenceOperand( aSheetRange, aRef, bDeleted, bRelativeAsOffset );
}

bool OoxFormulaParserImpl::importMemAreaToken( RecordInputStream& rStrm, bool bAddData )
{
    rStrm.skip( 6 );
    if( bAddData )
        skipMemAreaAddData( rStrm );
    return true;
}

bool OoxFormulaParserImpl::importMemFuncToken( RecordInputStream& rStrm )
{
    rStrm.skip( 2 );
    return true;
}

bool OoxFormulaParserImpl::importNameToken( RecordInputStream& rStrm )
{
    return pushOobName( rStrm.readInt32() );
}

bool OoxFormulaParserImpl::importNameXToken( RecordInputStream& rStrm )
{
    sal_Int32 nRefId = rStrm.readInt16();
    sal_Int32 nNameId = rStrm.readInt32();
    return pushOobExtName( nRefId, nNameId );
}

bool OoxFormulaParserImpl::importFuncToken( RecordInputStream& rStrm )
{
    sal_uInt16 nFuncId;
    rStrm >> nFuncId;
    return pushOobFunction( nFuncId );
}

bool OoxFormulaParserImpl::importFuncVarToken( RecordInputStream& rStrm )
{
    sal_uInt8 nParamCount;
    sal_uInt16 nFuncId;
    rStrm >> nParamCount >> nFuncId;
    return pushOobFunction( nFuncId, nParamCount );
}

bool OoxFormulaParserImpl::importExpToken( RecordInputStream& rStrm )
{
    BinAddress aBaseAddr;
    rStrm >> aBaseAddr.mnRow;
    swapStreamPosition( rStrm );
    rStrm >> aBaseAddr.mnCol;
    swapStreamPosition( rStrm );
    setSharedFormula( aBaseAddr );
    // formula has been set, exit parser by returning false
    return false;
}

LinkSheetRange OoxFormulaParserImpl::readSheetRange( RecordInputStream& rStrm )
{
    return getExternalLinks().getSheetRange( rStrm.readInt16() );
}

void OoxFormulaParserImpl::swapStreamPosition( RecordInputStream& rStrm )
{
    sal_Int32 nRecPos = rStrm.getRecPos();
    rStrm.seek( mnAddDataPos );
    mnAddDataPos = nRecPos;
}

void OoxFormulaParserImpl::skipMemAreaAddData( RecordInputStream& rStrm )
{
    swapStreamPosition( rStrm );
    rStrm.skip( 16 * rStrm.readInt32() );
    swapStreamPosition( rStrm );
}

// convert BIN token and push API operand or operator -------------------------

bool OoxFormulaParserImpl::pushOobName( sal_Int32 nNameId )
{
    // one-based in OOBIN formulas
    return pushDefinedNameOperand( getDefinedNames().getByIndex( nNameId - 1 ) );
}

bool OoxFormulaParserImpl::pushOobExtName( sal_Int32 nRefId, sal_Int32 nNameId )
{
    if( const ExternalLink* pExtLink = getExternalLinks().getExternalLink( nRefId ).get() )
    {
        if( pExtLink->getLinkType() == LINKTYPE_SELF )
            return pushOobName( nNameId );
            // external name indexes are one-based in OOBIN
        ExternalNameRef xExtName = pExtLink->getNameByIndex( nNameId - 1 );
        return pushExternalNameOperand( xExtName, pExtLink->getLinkType() );
    }
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool OoxFormulaParserImpl::pushOobFunction( sal_uInt16 nFuncId )
{
    if( const FunctionInfo* pFuncInfo = mrFuncProv.getFuncInfoFromOobFuncId( nFuncId ) )
        if( pFuncInfo->mnMinParamCount == pFuncInfo->mnMaxParamCount )
            return pushFunctionOperator( *pFuncInfo, pFuncInfo->mnMinParamCount );
    return pushFunctionOperator( mrFuncProv.OPCODE_NONAME, 0 );
}

bool OoxFormulaParserImpl::pushOobFunction( sal_uInt16 nFuncId, sal_uInt8 nParamCount )
{
    if( getFlag( nFuncId, BIFF_TOK_FUNCVAR_CMD ) )
        nParamCount &= BIFF_TOK_FUNCVAR_COUNTMASK;
    if( const FunctionInfo* pFuncInfo = mrFuncProv.getFuncInfoFromOobFuncId( nFuncId ) )
        return pushFunctionOperator( *pFuncInfo, nParamCount );
    return pushFunctionOperator( mrFuncProv.OPCODE_NONAME, nParamCount );
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

// ----------------------------------------------------------------------------

class BiffFormulaParserImpl : public FormulaParserImpl
{
public:
    explicit            BiffFormulaParserImpl(
                            const WorkbookHelper& rHelper,
                            const FunctionProvider& rFuncProv );

    virtual void        importBiffFormula(
                            FormulaContext& rContext,
                            BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize );

private:
    // import token contents and create API formula token ---------------------

    bool                importTokenNotAvailable( BiffInputStream& rStrm );
    bool                importRefTokenNotAvailable( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importStrToken2( BiffInputStream& rStrm );
    bool                importStrToken8( BiffInputStream& rStrm );
    bool                importAttrToken( BiffInputStream& rStrm );
    bool                importSpaceToken3( BiffInputStream& rStrm );
    bool                importSpaceToken4( BiffInputStream& rStrm );
    bool                importSheetToken2( BiffInputStream& rStrm );
    bool                importSheetToken3( BiffInputStream& rStrm );
    bool                importEndSheetToken2( BiffInputStream& rStrm );
    bool                importEndSheetToken3( BiffInputStream& rStrm );
    bool                importNlrToken( BiffInputStream& rStrm );
    bool                importArrayToken( BiffInputStream& rStrm );
    bool                importRefToken2( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRefToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importAreaToken2( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importAreaToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRef3dToken5( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importRef3dToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importArea3dToken5( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importArea3dToken8( BiffInputStream& rStrm, bool bDeleted, bool bRelativeAsOffset );
    bool                importMemAreaToken( BiffInputStream& rStrm, bool bAddData );
    bool                importMemFuncToken( BiffInputStream& rStrm );
    bool                importNameToken( BiffInputStream& rStrm );
    bool                importNameXToken( BiffInputStream& rStrm );
    bool                importFuncToken2( BiffInputStream& rStrm );
    bool                importFuncToken4( BiffInputStream& rStrm );
    bool                importFuncVarToken2( BiffInputStream& rStrm );
    bool                importFuncVarToken4( BiffInputStream& rStrm );
    bool                importFuncCEToken( BiffInputStream& rStrm );
    bool                importExpToken5( BiffInputStream& rStrm );

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
    void                skipMemAreaAddData( BiffInputStream& rStrm );
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

    // ------------------------------------------------------------------------
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
    ImportTokenFunc     mpImportExpToken;           /// Pointer to tExp import function (array/shared formula).
    sal_uInt32          mnAddDataPos;               /// Current stream position for additional data (tArray, tMemArea, tNlr).
    sal_Int32           mnCurrRefId;                /// Current ref-id from tSheet token (BIFF2-BIFF4 only).
    sal_uInt16          mnAttrDataSize;             /// Size of one tAttr data element.
    sal_uInt16          mnArraySize;                /// Size of tArray data.
    sal_uInt16          mnNameSize;                 /// Size of tName data.
    sal_uInt16          mnMemAreaSize;              /// Size of tMemArea data.
    sal_uInt16          mnMemFuncSize;              /// Size of tMemFunc data.
    sal_uInt16          mnRefIdSize;                /// Size of unused data following a reference identifier.
};

// ----------------------------------------------------------------------------

BiffFormulaParserImpl::BiffFormulaParserImpl( const WorkbookHelper& rHelper, const FunctionProvider& rFuncProv ) :
    FormulaParserImpl( rHelper, rFuncProv ),
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
            mpImportExpToken = &BiffFormulaParserImpl::importTokenNotAvailable;
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
            mpImportExpToken = &BiffFormulaParserImpl::importTokenNotAvailable;
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
            mpImportExpToken = &BiffFormulaParserImpl::importTokenNotAvailable;
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
            mpImportExpToken = &BiffFormulaParserImpl::importExpToken5;
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
            mpImportExpToken = &BiffFormulaParserImpl::importExpToken5;
            mnAttrDataSize = 2;
            mnArraySize = 7;
            mnNameSize = 2;
            mnMemAreaSize = 6;
            mnMemFuncSize = 2;
            mnRefIdSize = 0;
        break;
        case BIFF_UNKNOWN: break;
    }
}

void BiffFormulaParserImpl::importBiffFormula( FormulaContext& rContext,
        BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize )
{
    initializeImport( rContext );
    mnCurrRefId = 0;

    sal_uInt16 nFmlaSize = pnFmlaSize ? *pnFmlaSize : ((getBiff() == BIFF2) ? rStrm.readuInt8() : rStrm.readuInt16());
    sal_uInt32 nEndPos = mnAddDataPos = rStrm.getRecPos() + nFmlaSize;
    bool bRelativeAsOffset = getFormulaContext().isRelativeAsOffset();

    bool bOk = true;
    while( bOk && rStrm.isValid() && (rStrm.getRecPos() < nEndPos) )
    {
        sal_uInt8 nTokenId;
        rStrm >> nTokenId;
        sal_uInt8 nTokenClass = nTokenId & BIFF_TOKCLASS_MASK;
        sal_uInt8 nBaseId = nTokenId & BIFF_TOKID_MASK;

        if( nTokenClass == BIFF_TOKCLASS_NONE )
        {
            // base tokens
            switch( nBaseId )
            {
                case BIFF_TOKID_EXP:        bOk = (this->*mpImportExpToken)( rStrm );                           break;
                case BIFF_TOKID_TBL:        bOk = false; /* multiple op. will be set externally */              break;
                case BIFF_TOKID_ADD:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_ADD );                  break;
                case BIFF_TOKID_SUB:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_SUB );                  break;
                case BIFF_TOKID_MUL:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_MULT );                 break;
                case BIFF_TOKID_DIV:        bOk = pushBinaryOperator( mrFuncProv.OPCODE_DIV );                  break;
                case BIFF_TOKID_POWER:      bOk = pushBinaryOperator( mrFuncProv.OPCODE_POWER );                break;
                case BIFF_TOKID_CONCAT:     bOk = pushBinaryOperator( mrFuncProv.OPCODE_CONCAT );               break;
                case BIFF_TOKID_LT:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_LESS );                 break;
                case BIFF_TOKID_LE:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_LESS_EQUAL );           break;
                case BIFF_TOKID_EQ:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_EQUAL );                break;
                case BIFF_TOKID_GE:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_GREATER_EQUAL );        break;
                case BIFF_TOKID_GT:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_GREATER );              break;
                case BIFF_TOKID_NE:         bOk = pushBinaryOperator( mrFuncProv.OPCODE_NOT_EQUAL );            break;
                case BIFF_TOKID_ISECT:      bOk = pushBinaryOperator( mrFuncProv.OPCODE_INTERSECT );            break;
                case BIFF_TOKID_LIST:       bOk = pushBinaryOperator( mrFuncProv.OPCODE_LIST );                 break;
                case BIFF_TOKID_RANGE:      bOk = pushBinaryOperator( mrFuncProv.OPCODE_RANGE );                break;
                case BIFF_TOKID_UPLUS:      bOk = pushUnaryPreOperator( mrFuncProv.OPCODE_PLUS_SIGN );          break;
                case BIFF_TOKID_UMINUS:     bOk = pushUnaryPreOperator( mrFuncProv.OPCODE_MINUS_SIGN );         break;
                case BIFF_TOKID_PERCENT:    bOk = pushUnaryPostOperator( mrFuncProv.OPCODE_PERCENT );           break;
                case BIFF_TOKID_PAREN:      bOk = pushParenthesesOperator();                                    break;
                case BIFF_TOKID_MISSARG:    bOk = pushOperand( mrFuncProv.OPCODE_MISSING );                     break;
                case BIFF_TOKID_STR:        bOk = (this->*mpImportStrToken)( rStrm );                           break;
                case BIFF_TOKID_NLR:        bOk = (this->*mpImportNlrToken)( rStrm );                           break;
                case BIFF_TOKID_ATTR:       bOk = importAttrToken( rStrm );                                     break;
                case BIFF_TOKID_SHEET:      bOk = (this->*mpImportSheetToken)( rStrm );                         break;
                case BIFF_TOKID_ENDSHEET:   bOk = (this->*mpImportEndSheetToken)( rStrm );                      break;
                case BIFF_TOKID_ERR:        bOk = pushBiffErrorOperand( rStrm.readuInt8() );                    break;
                case BIFF_TOKID_BOOL:       bOk = pushBoolOperand( rStrm.readuInt8() != BIFF_TOK_BOOL_FALSE );  break;
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
                case BIFF_TOKID_ARRAY:      bOk = importArrayToken( rStrm );                                        break;
                case BIFF_TOKID_FUNC:       bOk = (this->*mpImportFuncToken)( rStrm );                              break;
                case BIFF_TOKID_FUNCVAR:    bOk = (this->*mpImportFuncVarToken)( rStrm );                           break;
                case BIFF_TOKID_NAME:       bOk = importNameToken( rStrm );                                         break;
                case BIFF_TOKID_REF:        bOk = (this->*mpImportRefToken)( rStrm, false, false );                 break;
                case BIFF_TOKID_AREA:       bOk = (this->*mpImportAreaToken)( rStrm, false, false );                break;
                case BIFF_TOKID_MEMAREA:    bOk = importMemAreaToken( rStrm, true );                                break;
                case BIFF_TOKID_MEMERR:     bOk = importMemAreaToken( rStrm, false );                               break;
                case BIFF_TOKID_MEMNOMEM:   bOk = importMemAreaToken( rStrm, false );                               break;
                case BIFF_TOKID_MEMFUNC:    bOk = importMemFuncToken( rStrm );                                      break;
                case BIFF_TOKID_REFERR:     bOk = (this->*mpImportRefToken)( rStrm, true, false );                  break;
                case BIFF_TOKID_AREAERR:    bOk = (this->*mpImportAreaToken)( rStrm, true, false );                 break;
                case BIFF_TOKID_REFN:       bOk = (this->*mpImportRefToken)( rStrm, false, true );                  break;
                case BIFF_TOKID_AREAN:      bOk = (this->*mpImportAreaToken)( rStrm, false, true );                 break;
                case BIFF_TOKID_MEMAREAN:   bOk = importMemFuncToken( rStrm );                                      break;
                case BIFF_TOKID_MEMNOMEMN:  bOk = importMemFuncToken( rStrm );                                      break;
                case BIFF_TOKID_FUNCCE:     bOk = (this->*mpImportFuncCEToken)( rStrm );                            break;
                case BIFF_TOKID_NAMEX:      bOk = (this->*mpImportNameXToken)( rStrm );                             break;
                case BIFF_TOKID_REF3D:      bOk = (this->*mpImportRef3dToken)( rStrm, false, bRelativeAsOffset );   break;
                case BIFF_TOKID_AREA3D:     bOk = (this->*mpImportArea3dToken)( rStrm, false, bRelativeAsOffset );  break;
                case BIFF_TOKID_REFERR3D:   bOk = (this->*mpImportRef3dToken)( rStrm, true, bRelativeAsOffset );    break;
                case BIFF_TOKID_AREAERR3D:  bOk = (this->*mpImportArea3dToken)( rStrm, true, bRelativeAsOffset );   break;
                default:                    bOk = false;
            }
        }
    }

    // build and finalize the token sequence
    if( bOk && (rStrm.getRecPos() == nEndPos) )
        finalizeImport();

    // seek behind additional token data of tArray, tMemArea, tNlr tokens
    rStrm.seek( mnAddDataPos );
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
    return pushValueOperand( rStrm.readByteString( false, getTextEncoding() ) );
}

bool BiffFormulaParserImpl::importStrToken8( BiffInputStream& rStrm )
{
    // read flags field for empty strings also
    return pushValueOperand( rStrm.readUniString( rStrm.readuInt8() ) );
}

bool BiffFormulaParserImpl::importAttrToken( BiffInputStream& rStrm )
{
    bool bOk = true;
    sal_uInt8 nType;
    rStrm >> nType;
    switch( nType )
    {
        case BIFF_TOK_ATTR_VOLATILE:
        case BIFF_TOK_ATTR_IF:
        case BIFF_TOK_ATTR_SKIP:
        case BIFF_TOK_ATTR_ASSIGN:
            rStrm.skip( mnAttrDataSize );
        break;
        case BIFF_TOK_ATTR_CHOOSE:
            rStrm.skip( mnAttrDataSize * (1 + ((getBiff() == BIFF2) ? rStrm.readuInt8() : rStrm.readuInt16())) );
        break;
        case BIFF_TOK_ATTR_SUM:
            rStrm.skip( mnAttrDataSize );
            bOk = pushBiffFunction( BIFF_FUNC_SUM, 1 );
        break;
        case BIFF_TOK_ATTR_SPACE:
        case BIFF_TOK_ATTR_SPACE_VOLATILE:
            bOk = (this->*mpImportSpaceToken)( rStrm );
        break;
        default:
            bOk = false;
    }
    return bOk;
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
        case BIFF_TOK_ATTR_SPACE_BR:
            incLeadingSpaces( nCount );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_OPEN:
        case BIFF_TOK_ATTR_SPACE_BR_OPEN:
            incOpeningSpaces( nCount );
        break;
        case BIFF_TOK_ATTR_SPACE_SP_CLOSE:
        case BIFF_TOK_ATTR_SPACE_BR_CLOSE:
            incClosingSpaces( nCount );
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

bool BiffFormulaParserImpl::importArrayToken( BiffInputStream& rStrm )
{
    rStrm.skip( mnArraySize );

    // start token array with opening brace and leading spaces
    pushOperand( mrFuncProv.OPCODE_ARRAY_OPEN );
    size_t nOpSize = popOperandSize();
    size_t nOldArraySize = getFormulaSize();
    bool bBiff8 = getBiff() == BIFF8;

    // read array size
    swapStreamPosition( rStrm );
    sal_uInt16 nCols = rStrm.readuInt8();
    sal_uInt16 nRows = rStrm.readuInt16();
    if( bBiff8 ) { ++nCols; ++nRows; } else if( nCols == 0 ) nCols = 256;
    OSL_ENSURE( (nCols > 0) && (nRows > 0), "BiffFormulaParserImpl::importArrayToken - empty array" );

    // read array values and build token array
    for( sal_uInt16 nRow = 0; rStrm.isValid() && (nRow < nRows); ++nRow )
    {
        if( nRow > 0 )
            appendRawToken( mrFuncProv.OPCODE_ARRAY_ROWSEP );
        for( sal_uInt16 nCol = 0; rStrm.isValid() && (nCol < nCols); ++nCol )
        {
            if( nCol > 0 )
                appendRawToken( mrFuncProv.OPCODE_ARRAY_COLSEP );
            switch( rStrm.readuInt8() )
            {
                case BIFF_DATATYPE_EMPTY:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= OUString();
                    rStrm.skip( 8 );
                break;
                case BIFF_DATATYPE_DOUBLE:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= rStrm.readDouble();
                break;
                case BIFF_DATATYPE_STRING:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= bBiff8 ?
                        rStrm.readUniString() :
                        rStrm.readByteString( false, getTextEncoding() );
                break;
                case BIFF_DATATYPE_BOOL:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= static_cast< double >( (rStrm.readuInt8() == 0) ? 0.0 : 1.0 );
                    rStrm.skip( 7 );
                break;
                case BIFF_DATATYPE_ERROR:
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= BiffHelper::calcDoubleFromError( rStrm.readuInt8() );
                    rStrm.skip( 7 );
                break;
                default:
                    OSL_ENSURE( false, "BiffFormulaParserImpl::importArrayToken - unknown data type" );
                    appendRawToken( mrFuncProv.OPCODE_PUSH ) <<= BiffHelper::calcDoubleFromError( BIFF_ERR_NA );
            }
        }
    }
    swapStreamPosition( rStrm );

    // close token array and set resulting operand size
    appendRawToken( mrFuncProv.OPCODE_ARRAY_CLOSE );
    pushOperandSize( nOpSize + getFormulaSize() - nOldArraySize );
    return true;
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

bool BiffFormulaParserImpl::importMemAreaToken( BiffInputStream& rStrm, bool bAddData )
{
    rStrm.skip( mnMemAreaSize );
    if( bAddData )
        skipMemAreaAddData( rStrm );
    return true;
}

bool BiffFormulaParserImpl::importMemFuncToken( BiffInputStream& rStrm )
{
    rStrm.skip( mnMemFuncSize );
    return true;
}

bool BiffFormulaParserImpl::importNameToken( BiffInputStream& rStrm )
{
    sal_uInt16 nNameId = readNameId( rStrm );
    return (mnCurrRefId > 0) ? pushBiffExtName( mnCurrRefId, nNameId ) : pushBiffName( nNameId );
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

bool BiffFormulaParserImpl::importExpToken5( BiffInputStream& rStrm )
{
    BinAddress aBaseAddr;
    aBaseAddr.read( rStrm );
    setSharedFormula( aBaseAddr );
    // formula has been set, exit parser by returning false
    return false;
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
    sal_uInt32 nRecPos = rStrm.getRecPos();
    rStrm.seek( mnAddDataPos );
    mnAddDataPos = nRecPos;
}

void BiffFormulaParserImpl::skipMemAreaAddData( BiffInputStream& rStrm )
{
    swapStreamPosition( rStrm );
    sal_uInt32 nCount = rStrm.readuInt16();
    rStrm.skip( ((getBiff() == BIFF8) ? 8 : 6) * nCount );
    swapStreamPosition( rStrm );
}

bool BiffFormulaParserImpl::readNlrSAddrAddData( BiffNlr& orNlr, BiffInputStream& rStrm, bool bRow )
{
    bool bIsRow;
    return readNlrSRangeAddData( orNlr, bIsRow, rStrm ) && (bIsRow == bRow);
}

bool BiffFormulaParserImpl::readNlrSRangeAddData( BiffNlr& orNlr, bool& orbIsRow, BiffInputStream& rStrm )
{
    swapStreamPosition( rStrm );
    // read number of cell addresses and relative flag
    sal_uInt32 nCount;
    rStrm >> nCount;
    bool bRel = getFlag( nCount, BIFF_TOK_NLR_ADDREL );
    nCount &= BIFF_TOK_NLR_ADDMASK;
    sal_uInt32 nEndPos = rStrm.getRecPos() + 4 * nCount;
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
            bValid = rStrm.isValid() && lclIsValidNlrStack( aAddr1, aAddr2, orbIsRow );
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
        return pushExternalNameOperand( xExtName, pExtLink->getLinkType() );
    }
    return pushBiffErrorOperand( BIFF_ERR_NAME );
}

bool BiffFormulaParserImpl::pushBiffFunction( sal_uInt16 nFuncId )
{
    if( const FunctionInfo* pFuncInfo = mrFuncProv.getFuncInfoFromBiffFuncId( nFuncId ) )
        if( pFuncInfo->mnMinParamCount == pFuncInfo->mnMaxParamCount )
            return pushFunctionOperator( *pFuncInfo, pFuncInfo->mnMinParamCount );
    return pushFunctionOperator( mrFuncProv.OPCODE_NONAME, 0 );
}

bool BiffFormulaParserImpl::pushBiffFunction( sal_uInt16 nFuncId, sal_uInt8 nParamCount )
{
    if( getFlag( nFuncId, BIFF_TOK_FUNCVAR_CMD ) )
        nParamCount &= BIFF_TOK_FUNCVAR_COUNTMASK;
    if( const FunctionInfo* pFuncInfo = mrFuncProv.getFuncInfoFromBiffFuncId( nFuncId ) )
        return pushFunctionOperator( *pFuncInfo, nParamCount );
    return pushFunctionOperator( mrFuncProv.OPCODE_NONAME, nParamCount );
}

// ============================================================================

FormulaParser::FormulaParser( const WorkbookHelper& rHelper ) :
    FormulaProcessorBase( rHelper )
{
    switch( getFilterType() )
    {
        case FILTER_OOX:    mxImpl.reset( new OoxFormulaParserImpl( rHelper, maFuncProv ) );    break;
        case FILTER_BIFF:   mxImpl.reset( new BiffFormulaParserImpl( rHelper, maFuncProv ) );   break;
        case FILTER_UNKNOWN: break;
    }
}

FormulaParser::~FormulaParser()
{
}

void FormulaParser::importFormula( FormulaContext& rContext, const OUString& rFormulaString ) const
{
    mxImpl->importOoxFormula( rContext, rFormulaString );
}

void FormulaParser::importFormula( FormulaContext& rContext, RecordInputStream& rStrm ) const
{
    mxImpl->importOobFormula( rContext, rStrm );
}

void FormulaParser::importFormula( FormulaContext& rContext, BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize ) const
{
    mxImpl->importBiffFormula( rContext, rStrm, pnFmlaSize );
}

void FormulaParser::convertErrorToFormula( FormulaContext& rContext, sal_uInt8 nErrorCode ) const
{
    ApiTokenSequence aTokens( 3 );
    // HACK: enclose all error codes into an 1x1 matrix
    aTokens[ 0 ].OpCode = maFuncProv.OPCODE_ARRAY_OPEN;
    aTokens[ 1 ].OpCode = maFuncProv.OPCODE_PUSH;
    aTokens[ 1 ].Data <<= BiffHelper::calcDoubleFromError( nErrorCode );
    aTokens[ 2 ].OpCode = maFuncProv.OPCODE_ARRAY_CLOSE;
    mxImpl->setFormula( rContext, aTokens );
}

void FormulaParser::convertNameToFormula( FormulaContext& rContext, sal_Int32 nTokenIndex ) const
{
    if( nTokenIndex >= 0 )
    {
        ApiTokenSequence aTokens( 1 );
        aTokens[ 0 ].OpCode = maFuncProv.OPCODE_NAME;
        aTokens[ 0 ].Data <<= nTokenIndex;
        mxImpl->setFormula( rContext, aTokens );
    }
    else
        convertErrorToFormula( rContext, BIFF_ERR_REF );
}

// ============================================================================

} // namespace xls
} // namespace oox

