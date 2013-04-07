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

#include <ucbhelper/registerucb.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#include <com/sun/star/ucb/ContentProviderProxyFactory.hpp>
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include "osl/diagnose.h"

using namespace com::sun::star;

namespace ucbhelper {

//============================================================================
//
//  registerAtUcb
//
//============================================================================

bool
registerAtUcb(
    uno::Reference< ucb::XContentProviderManager > const & rManager,
    uno::Reference< lang::XMultiServiceFactory > const & rServiceFactory,
    OUString const & rName,
    OUString const & rArguments,
    OUString const & rTemplate,
    ContentProviderRegistrationInfo * pInfo)
    throw (uno::RuntimeException)
{
    OSL_ENSURE(rServiceFactory.is(),
               "ucb::registerAtUcb(): No service factory");

    bool bNoProxy = rArguments.startsWith("{noproxy}");
    OUString
        aProviderArguments(bNoProxy ?
                               rArguments.
                                   copy(RTL_CONSTASCII_LENGTH("{noproxy}")) :
                               rArguments);

    // First, try to instantiate proxy for provider:
    uno::Reference< ucb::XContentProvider > xProvider;
    if (!bNoProxy)
    {
        uno::Reference< ucb::XContentProviderFactory > xProxyFactory;
        try
        {
            uno::Reference< beans::XPropertySet > xFactoryProperties( rServiceFactory, uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xContext = uno::Reference< uno::XComponentContext >(
                xFactoryProperties->getPropertyValue( OUString( "DefaultContext" ) ),
                uno::UNO_QUERY );
            xProxyFactory
                = uno::Reference< ucb::XContentProviderFactory >(
                      ucb::ContentProviderProxyFactory::create( xContext ) );
        }
        catch (uno::Exception const &) {}
        OSL_ENSURE(xProxyFactory.is(), "No ContentProviderProxyFactory");
        if (xProxyFactory.is())
            xProvider = xProxyFactory->createContentProvider(rName);
    }

    // Then, try to instantiate provider directly:
    if (!xProvider.is())
        try
        {
            xProvider = uno::Reference< ucb::XContentProvider >(
                            rServiceFactory->createInstance(rName),
                            uno::UNO_QUERY);
        }
        catch (uno::RuntimeException const &) { throw; }
        catch (uno::Exception const &) {}

    uno::Reference< ucb::XContentProvider >
        xOriginalProvider(xProvider);
    uno::Reference< ucb::XParameterizedContentProvider >
        xParameterized(xProvider, uno::UNO_QUERY);
    if (xParameterized.is())
    {
        uno::Reference< ucb::XContentProvider > xInstance;
        try
        {
            xInstance = xParameterized->registerInstance(rTemplate,
                                                         aProviderArguments,
                                                         true);
                //@@@ if this call replaces an old instance, the commit-or-
                // rollback code below will not work
        }
        catch (lang::IllegalArgumentException const &) {}

        if (xInstance.is())
            xProvider = xInstance;
    }

    bool bSuccess = false;
    if (rManager.is() && xProvider.is())
    {
        try
        {
            rManager->registerContentProvider(xProvider, rTemplate, true);
            bSuccess = true;
        }
        catch (ucb::DuplicateProviderException const &)
        {
            if (xParameterized.is())
                try
                {
                    xParameterized->deregisterInstance(rTemplate,
                                                       aProviderArguments);
                }
                catch (lang::IllegalArgumentException const &) {}
        }
        catch (...)
        {
            if (xParameterized.is())
                try
                {
                    xParameterized->deregisterInstance(rTemplate,
                                                       aProviderArguments);
                }
                catch (lang::IllegalArgumentException const &) {}
                catch (uno::RuntimeException const &) {}
            throw;
        }
    }
    if (bSuccess && pInfo)
    {
        pInfo->m_xProvider = xOriginalProvider;
        pInfo->m_aArguments = aProviderArguments;
        pInfo->m_aTemplate = rTemplate;
    }
    return bSuccess;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
