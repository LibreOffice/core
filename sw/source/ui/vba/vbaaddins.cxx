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
#include "vbaaddins.hxx"
#include "vbaaddin.hxx"
#include <cppuhelper/implbase3.hxx>
#include <unotools/pathoptions.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static uno::Reference< container::XIndexAccess > lcl_getAddinCollection( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext )
{
    XNamedObjectCollectionHelper< word::XAddin >::XNamedVec maAddins;

    // first get the autoload addins in the directory STARTUP
    uno::Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference<ucb::XSimpleFileAccess2> xSFA(ucb::SimpleFileAccess::create(xContext));
    SvtPathOptions aPathOpt;
    // FIXME: temporary the STARTUP path is located in $OO/basic3.1/program/addin
    String aAddinPath = aPathOpt.GetAddinPath();
    OSL_TRACE("lcl_getAddinCollection: %s", rtl::OUStringToOString( aAddinPath, RTL_TEXTENCODING_UTF8 ).getStr() );
    if( xSFA->isFolder( aAddinPath ) )
    {
        uno::Sequence< rtl::OUString > sEntries = xSFA->getFolderContents( aAddinPath, sal_False );
        sal_Int32 nEntry = sEntries.getLength();
        for( sal_Int32 index = 0; index < nEntry; ++index )
        {
            rtl::OUString sUrl = sEntries[ index ];
            if( !xSFA->isFolder( sUrl ) && sUrl.endsWithIgnoreAsciiCaseAsciiL( ".dot", 4 ) )
            {
                maAddins.push_back( uno::Reference< word::XAddin >( new SwVbaAddin( xParent, xContext, sUrl, sal_True ) ) );
            }
        }
    }

    // TODO: second get the customize addins in the org.openoffice.Office.Writer/GlobalTemplateList

    uno::Reference< container::XIndexAccess > xAddinsAccess( new XNamedObjectCollectionHelper< word::XAddin >( maAddins ) );
    return xAddinsAccess;
}

SwVbaAddins::SwVbaAddins( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext ) throw (uno::RuntimeException): SwVbaAddins_BASE( xParent, xContext, lcl_getAddinCollection( xParent,xContext ) )
{
}
// XEnumerationAccess
uno::Type
SwVbaAddins::getElementType() throw (uno::RuntimeException)
{
    return word::XAddin::static_type(0);
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

rtl::OUString
SwVbaAddins::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SwVbaAddins"));
}

css::uno::Sequence<rtl::OUString>
SwVbaAddins::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Addins") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
