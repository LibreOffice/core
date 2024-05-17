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

#include <ooxformulaparser.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/supportsservice.hxx>
#include <formulaparser.hxx>

namespace com::sun::star::uno { class XComponentContext; }

namespace oox::xls {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

class OOXMLFormulaParserImpl : private FormulaFinalizer
{
public:
    explicit            OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxModelFactory );

    Sequence< FormulaToken > parseFormula( const OUString& rFormula, const ScAddress& rReferencePos );

protected:
    virtual const FunctionInfo* resolveBadFuncName( const OUString& rTokenData ) const override;

private:
    ApiParserWrapper    maApiParser;
};

OOXMLFormulaParserImpl::OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    FormulaFinalizer( OpCodeProvider( rxModelFactory, true ) ),
    maApiParser( rxModelFactory, *this )
{
}

Sequence< FormulaToken > OOXMLFormulaParserImpl::parseFormula( const OUString& rFormula, const ScAddress& rReferencePos )
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
                if( (nDirSep > 0) && rTokenData.matchIgnoreAsciiCase( "\\LIBRARY\\", nDirSep ) )
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
    return nullptr;
}

OOXMLFormulaParser::OOXMLFormulaParser()
{
}

OOXMLFormulaParser::~OOXMLFormulaParser()
{
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------
OUString SAL_CALL OOXMLFormulaParser::getImplementationName()
{
    return u"com.sun.star.comp.oox.xls.FormulaParser"_ustr;
}

sal_Bool SAL_CALL OOXMLFormulaParser::supportsService( const OUString& rService )
{
    return cppu::supportsService(this, rService);
}

Sequence< OUString > SAL_CALL OOXMLFormulaParser::getSupportedServiceNames()
{
    return { u"com.sun.star.sheet.FilterFormulaParser"_ustr };
}

// com.sun.star.lang.XInitialization interface --------------------------------

void SAL_CALL OOXMLFormulaParser::initialize( const Sequence< Any >& rArgs )
{
    OSL_ENSURE( rArgs.hasElements(), "OOXMLFormulaParser::initialize - missing arguments" );
    if( !rArgs.hasElements() )
        throw RuntimeException();
    mxComponent.set( rArgs[ 0 ], UNO_QUERY_THROW );
}

// com.sun.star.sheet.XFilterFormulaParser interface --------------------------

OUString SAL_CALL OOXMLFormulaParser::getSupportedNamespace()
{
    return u"http://schemas.microsoft.com/office/excel/formula"_ustr;
}

// com.sun.star.sheet.XFormulaParser interface --------------------------------

Sequence< FormulaToken > SAL_CALL OOXMLFormulaParser::parseFormula(
        const OUString& rFormula, const CellAddress& rReferencePos )
{
    if( !mxParserImpl )
    {
        Reference< XMultiServiceFactory > xModelFactory( mxComponent, UNO_QUERY_THROW );
        mxParserImpl = std::make_shared<OOXMLFormulaParserImpl>( xModelFactory );
    }
    return mxParserImpl->parseFormula( rFormula,
                                       ScAddress(rReferencePos.Column, rReferencePos.Row, rReferencePos.Sheet) );
}

OUString SAL_CALL OOXMLFormulaParser::printFormula(
        const Sequence< FormulaToken >& /*rTokens*/, const CellAddress& /*rReferencePos*/ )
{
    // not implemented
    throw RuntimeException();
}

} // namespace oox::xls


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_oox_xls_FormulaParser_get_implementation(css::uno::XComponentContext*,
                                                           css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new oox::xls::OOXMLFormulaParser());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
