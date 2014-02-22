/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ooxformulaparser.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include "formulaparser.hxx"

namespace oox {
namespace xls {



using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;



class OOXMLFormulaParserImpl : private FormulaFinalizer
{
public:
    explicit            OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxModelFactory );

    Sequence< FormulaToken > parseFormula( const OUString& rFormula, const CellAddress& rReferencePos );

protected:
    virtual const FunctionInfo* resolveBadFuncName( const OUString& rTokenData ) const;

private:
    ApiParserWrapper    maApiParser;
};



OOXMLFormulaParserImpl::OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    FormulaFinalizer( OpCodeProvider( rxModelFactory, FILTER_OOXML, BIFF_UNKNOWN, true ) ),
    maApiParser( rxModelFactory, *this )
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

    
    if( (rTokenData.getLength() >= 6) && (rTokenData[ 0 ] == '\'') )
    {
        
        sal_Int32 nExclamPos = rTokenData.lastIndexOf( '!' );
        if( (1 < nExclamPos) && (nExclamPos + 1 < rTokenData.getLength()) && (rTokenData[ nExclamPos - 1 ] == '\'') )
        {
            
            sal_Int32 nFileSep = rTokenData.lastIndexOf( '\\', nExclamPos - 2 );
            if( nFileSep > 1 )
            {
                
                sal_Int32 nDirSep = rTokenData.lastIndexOf( '\\', nFileSep - 1 );
                
                if( (nDirSep > 0) && rTokenData.matchIgnoreAsciiCase( "\\LIBRARY\\", nDirSep ) )
                {
                    
                    OUString aFuncName = rTokenData.copy( nExclamPos + 1 ).toAsciiUpperCase();
                    const FunctionInfo* pFuncInfo = getFuncInfoFromOoxFuncName( aFuncName );
                    if( pFuncInfo && (pFuncInfo->meFuncLibType != FUNCLIB_UNKNOWN) )
                    {
                        
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



Sequence< OUString > OOXMLFormulaParser_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] =  "com.sun.star.sheet.FilterFormulaParser";
    return aServiceNames;
}

OUString OOXMLFormulaParser_getImplementationName()
{
    return OUString( "com.sun.star.comp.oox.xls.FormulaParser");
}

Reference< XInterface > OOXMLFormulaParser_create( const Reference< XComponentContext >& )
{
    return static_cast< ::cppu::OWeakObject* >( new OOXMLFormulaParser );
}



OOXMLFormulaParser::OOXMLFormulaParser()
{
}

OOXMLFormulaParser::~OOXMLFormulaParser()
{
}


OUString SAL_CALL OOXMLFormulaParser::getImplementationName() throw( RuntimeException )
{
    return OOXMLFormulaParser_getImplementationName();
}

sal_Bool SAL_CALL OOXMLFormulaParser::supportsService( const OUString& rService ) throw( RuntimeException )
{
    return cppu::supportsService(this, rService);
}

Sequence< OUString > SAL_CALL OOXMLFormulaParser::getSupportedServiceNames() throw( RuntimeException )
{
    return OOXMLFormulaParser_getSupportedServiceNames();
}



void SAL_CALL OOXMLFormulaParser::initialize( const Sequence< Any >& rArgs ) throw( Exception, RuntimeException )
{
    OSL_ENSURE( rArgs.hasElements(), "OOXMLFormulaParser::initialize - missing arguments" );
    if( !rArgs.hasElements() )
        throw RuntimeException();
    mxComponent.set( rArgs[ 0 ], UNO_QUERY_THROW );
}



OUString SAL_CALL OOXMLFormulaParser::getSupportedNamespace() throw( RuntimeException )
{
    return OUString( "http:
}



Sequence< FormulaToken > SAL_CALL OOXMLFormulaParser::parseFormula(
        const OUString& rFormula, const CellAddress& rReferencePos ) throw( RuntimeException )
{
    if( !mxParserImpl )
    {
        Reference< XMultiServiceFactory > xModelFactory( mxComponent, UNO_QUERY_THROW );
        mxParserImpl.reset( new OOXMLFormulaParserImpl( xModelFactory ) );
    }
    return mxParserImpl->parseFormula( rFormula, rReferencePos );
}

OUString SAL_CALL OOXMLFormulaParser::printFormula(
        const Sequence< FormulaToken >& /*rTokens*/, const CellAddress& /*rReferencePos*/ ) throw( RuntimeException )
{
    
    throw RuntimeException();
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
