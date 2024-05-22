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
#include "vbaaddins.hxx"
#include "vbaaddin.hxx"
#include <unotools/pathoptions.hxx>
#include <sal/log.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static uno::Reference< container::XIndexAccess > lcl_getAddinCollection( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext )
{
    XNamedObjectCollectionHelper< word::XAddin >::XNamedVec aAddins;

    // first get the autoload addins in the directory STARTUP
    uno::Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager(), uno::UNO_SET_THROW );
    uno::Reference<ucb::XSimpleFileAccess3> xSFA(ucb::SimpleFileAccess::create(xContext));
    SvtPathOptions aPathOpt;
    // FIXME: temporary the STARTUP path is located in $OO/basic3.1/program/addin
    const OUString& aAddinPath = aPathOpt.GetAddinPath();
    SAL_INFO("sw.vba", "lcl_getAddinCollection: " << aAddinPath );
    if( xSFA->isFolder( aAddinPath ) )
    {
        const uno::Sequence< OUString > sEntries = xSFA->getFolderContents( aAddinPath, false );
        for( const OUString& sUrl : sEntries )
        {
            if( !xSFA->isFolder( sUrl ) && sUrl.endsWithIgnoreAsciiCase( ".dot" ) )
            {
                aAddins.push_back( uno::Reference< word::XAddin >( new SwVbaAddin( xParent, xContext, sUrl ) ) );
            }
        }
    }

    // TODO: second get the customize addins in the org.openoffice.Office.Writer/GlobalTemplateList

    uno::Reference< container::XIndexAccess > xAddinsAccess( new XNamedObjectCollectionHelper< word::XAddin >( std::move(aAddins) ) );
    return xAddinsAccess;
}

SwVbaAddins::SwVbaAddins( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext ): SwVbaAddins_BASE( xParent, xContext, lcl_getAddinCollection( xParent,xContext ) )
{
}
// XEnumerationAccess
uno::Type
SwVbaAddins::getElementType()
{
    return cppu::UnoType<word::XAddin>::get();
}
uno::Reference< container::XEnumeration >
SwVbaAddins::createEnumeration()
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
    return u"SwVbaAddins"_ustr;
}

css::uno::Sequence<OUString>
SwVbaAddins::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Addins"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
