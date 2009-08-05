/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ooxformulaparser.cxx,v $
 * $Revision: 1.1 $
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

#include "oox/xls/ooxformulaparser.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include "oox/xls/formulaparser.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::sheet::FormulaToken;

namespace oox {
namespace xls {

// ============================================================================

class OOXMLFormulaParserImpl : private FormulaFinalizer
{
public:
    explicit            OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxFactory );

    Sequence< FormulaToken > parseFormula( const OUString& rFormula, const CellAddress& rReferencePos );

protected:
    virtual const FunctionInfo* resolveBadFuncName( const OUString& rTokenData ) const;

private:
    ApiParserWrapper    maApiParser;
};

// ----------------------------------------------------------------------------

OOXMLFormulaParserImpl::OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxFactory ) :
    FormulaFinalizer( OpCodeProvider( rxFactory, FILTER_OOX, BIFF_UNKNOWN, true ) ),
    maApiParser( rxFactory, *this )
{
}

Sequence< FormulaToken > OOXMLFormulaParserImpl::parseFormula( const OUString& rFormula, const CellAddress& rReferencePos )
{
    return finalizeTokenArray( maApiParser.parseFormula( rFormula, rReferencePos ) );
}

const FunctionInfo* OOXMLFormulaParserImpl::resolveBadFuncName( const OUString& rTokenData ) const
{
    /*  Try to parse calls to library functions. The format of such a function
        call is assumed to be
            "'<path-to-office-install>\Library\<libname>'!<funcname>". */

    // the string has to start with an apostroph (followed by the library URL)
    if( (rTokenData.getLength() >= 6) && (rTokenData[ 0 ] == '\'') )
    {
        // library URL and function name are separated by an exclamation mark
        sal_Int32 nExclamPos = rTokenData.lastIndexOf( '!' );
        if( (1 < nExclamPos) && (nExclamPos + 1 < rTokenData.getLength()) && (rTokenData[ nExclamPos - 1 ] == '\'') )
        {
            // find the last backslash that separates library path and name
            sal_Int32 nFileSep = rTokenData.lastIndexOf( '\\', nExclamPos - 2 );
            if( nFileSep > 1 )
            {
                // find preceding backslash that separates the last directory name
                sal_Int32 nDirSep = rTokenData.lastIndexOf( '\\', nFileSep - 1 );
                // function library is located in a directory called 'library'
                if( (nDirSep > 0) && rTokenData.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "\\LIBRARY\\" ), nDirSep ) )
                {
                    // try to find a function info for the function name
                    OUString aFuncName = rTokenData.copy( nExclamPos + 1 ).toAsciiUpperCase();
                    const FunctionInfo* pFuncInfo = getFuncInfoFromOoxFuncName( aFuncName );
                    if( pFuncInfo && (pFuncInfo->meFuncLibType != FUNCLIB_UNKNOWN) )
                    {
                        // check that the name of the library matches
                        OUString aLibName = rTokenData.copy( nFileSep + 1, nExclamPos - nFileSep - 2 );
                        if( pFuncInfo->meFuncLibType == getFuncLibTypeFromLibraryName( aLibName ) )
                            return pFuncInfo;
                    }
                }
            }
        }
    }
    return 0;
}

// ============================================================================

class OOXMLFormulaPrinterImpl : public OpCodeProvider
{
public:
    explicit            OOXMLFormulaPrinterImpl( const Reference< XMultiServiceFactory >& rxFactory );

private:
    ApiParserWrapper    maApiParser;
};

// ----------------------------------------------------------------------------

OOXMLFormulaPrinterImpl::OOXMLFormulaPrinterImpl( const Reference< XMultiServiceFactory >& rxFactory ) :
    OpCodeProvider( rxFactory, FILTER_OOX, BIFF_UNKNOWN, false ),
    maApiParser( rxFactory, *this )
{
}

// ============================================================================

Sequence< OUString > OOXMLFormulaParser_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = CREATE_OUSTRING( "com.sun.star.sheet.FilterFormulaParser" );
    return aServiceNames;
}

OUString OOXMLFormulaParser_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.OOXMLFormulaParser" );
}

Reference< XInterface > SAL_CALL OOXMLFormulaParser_createInstance( const Reference< XComponentContext >& ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new OOXMLFormulaParser );
}

// ============================================================================

OOXMLFormulaParser::OOXMLFormulaParser()
{
}

OOXMLFormulaParser::~OOXMLFormulaParser()
{
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

OUString SAL_CALL OOXMLFormulaParser::getImplementationName() throw( RuntimeException )
{
    return OOXMLFormulaParser_getImplementationName();
}

sal_Bool SAL_CALL OOXMLFormulaParser::supportsService( const OUString& rService ) throw( RuntimeException )
{
    const Sequence< OUString > aServices( OOXMLFormulaParser_getSupportedServiceNames() );
    const OUString* pArray = aServices.getConstArray();
    const OUString* pArrayEnd = pArray + aServices.getLength();
    return ::std::find( pArray, pArrayEnd, rService ) != pArrayEnd;
}

Sequence< OUString > SAL_CALL OOXMLFormulaParser::getSupportedServiceNames() throw( RuntimeException )
{
    return OOXMLFormulaParser_getSupportedServiceNames();
}

// com.sun.star.lang.XInitialization interface --------------------------------

void SAL_CALL OOXMLFormulaParser::initialize( const Sequence< Any >& rArgs ) throw( Exception, RuntimeException )
{
    OSL_ENSURE( rArgs.hasElements(), "OOXMLFormulaParser::initialize - missing arguments" );
    if( !rArgs.hasElements() )
        throw RuntimeException();
    mxComponent.set( rArgs[ 0 ], UNO_QUERY_THROW );
}

// com.sun.star.sheet.XFilterFormulaParser interface --------------------------

OUString SAL_CALL OOXMLFormulaParser::getSupportedNamespace() throw( RuntimeException )
{
    return CREATE_OUSTRING( "http://schemas.microsoft.com/office/excel/formula" );
}

// com.sun.star.sheet.XFormulaParser interface --------------------------------

Sequence< FormulaToken > SAL_CALL OOXMLFormulaParser::parseFormula(
        const OUString& rFormula, const CellAddress& rReferencePos ) throw( RuntimeException )
{
    if( !mxParserImpl )
    {
        Reference< XMultiServiceFactory > xFactory( mxComponent, UNO_QUERY_THROW );
        mxParserImpl.reset( new OOXMLFormulaParserImpl( xFactory ) );
    }
    return mxParserImpl->parseFormula( rFormula, rReferencePos );
}

OUString SAL_CALL OOXMLFormulaParser::printFormula(
        const Sequence< FormulaToken >& /*rTokens*/, const CellAddress& /*rReferencePos*/ ) throw( RuntimeException )
{
    // not implemented
    throw RuntimeException();
}

// ============================================================================

} // namespace xls
} // namespace oox

