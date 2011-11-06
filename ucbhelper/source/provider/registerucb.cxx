/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
#include <ucbhelper/registerucb.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
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
    rtl::OUString const & rName,
    rtl::OUString const & rArguments,
    rtl::OUString const & rTemplate,
    ContentProviderRegistrationInfo * pInfo)
    throw (uno::RuntimeException)
{
    OSL_ENSURE(rServiceFactory.is(),
               "ucb::registerAtUcb(): No service factory");

    bool bNoProxy
        = rArguments.compareToAscii(RTL_CONSTASCII_STRINGPARAM("{noproxy}"))
              == 0;
    rtl::OUString
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
            xProxyFactory
                = uno::Reference< ucb::XContentProviderFactory >(
                      rServiceFactory->
                          createInstance(
                              rtl::OUString(
                                  RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.ucb.ContentProviderProxyFactory"))),
                      uno::UNO_QUERY);
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

    if (bSuccess && pInfo)
    {
        pInfo->m_xProvider = xOriginalProvider;
        pInfo->m_aArguments = aProviderArguments;
        pInfo->m_aTemplate = rTemplate;
    }
    return bSuccess;
}

}
