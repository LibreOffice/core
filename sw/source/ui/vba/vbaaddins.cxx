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
#include "vbaaddins.hxx"
#include "vbaaddin.hxx"
#include <cppuhelper/implbase3.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static uno::Reference< container::XIndexAccess > lcl_getAddinCollection( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext )
{
    XNamedObjectCollectionHelper< word::XAddin >::XNamedVec maAddins;

    
    uno::Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference<ucb::XSimpleFileAccess3> xSFA(ucb::SimpleFileAccess::create(xContext));
    SvtPathOptions aPathOpt;
    
    OUString aAddinPath = aPathOpt.GetAddinPath();
    OSL_TRACE("lcl_getAddinCollection: %s", OUStringToOString( aAddinPath, RTL_TEXTENCODING_UTF8 ).getStr() );
    if( xSFA->isFolder( aAddinPath ) )
    {
        uno::Sequence< OUString > sEntries = xSFA->getFolderContents( aAddinPath, sal_False );
        sal_Int32 nEntry = sEntries.getLength();
        for( sal_Int32 index = 0; index < nEntry; ++index )
        {
            OUString sUrl = sEntries[ index ];
            if( !xSFA->isFolder( sUrl ) && sUrl.endsWithIgnoreAsciiCaseAsciiL( ".dot", 4 ) )
            {
                maAddins.push_back( uno::Reference< word::XAddin >( new SwVbaAddin( xParent, xContext, sUrl, sal_True ) ) );
            }
        }
    }

    

    uno::Reference< container::XIndexAccess > xAddinsAccess( new XNamedObjectCollectionHelper< word::XAddin >( maAddins ) );
    return xAddinsAccess;
}

SwVbaAddins::SwVbaAddins( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext ) throw (uno::RuntimeException): SwVbaAddins_BASE( xParent, xContext, lcl_getAddinCollection( xParent,xContext ) )
{
}

uno::Type
SwVbaAddins::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<word::XAddin>::get();
}
uno::Reference< container::XEnumeration >
SwVbaAddins::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumerationAccess->createEnumeration();
}

uno::Any
SwVbaAddins::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaAddins::getServiceImplName()
{
    return OUString("SwVbaAddins");
}

css::uno::Sequence<OUString>
SwVbaAddins::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.Addins";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
