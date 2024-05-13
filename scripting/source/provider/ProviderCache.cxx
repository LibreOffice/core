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

#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "ProviderCache.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;

namespace func_provider
{

ProviderCache::ProviderCache( const Reference< XComponentContext >& xContext, const Sequence< Any >& scriptContext ) : m_Sctx( scriptContext ), m_xContext( xContext )
{
    // initialise m_hProviderDetailsCache with details of ScriptProviders
    // will use createContentEnumeration

    m_xMgr = m_xContext->getServiceManager();
    ENSURE_OR_THROW( m_xMgr.is(), "ProviderCache::ProviderCache() failed to obtain ServiceManager" );
    populateCache();
}


ProviderCache::ProviderCache( const Reference< XComponentContext >& xContext, const Sequence< Any >& scriptContext, const Sequence< OUString >& denyList ) : m_sDenyList( denyList ), m_Sctx( scriptContext ), m_xContext( xContext )

{
    // initialise m_hProviderDetailsCache with details of ScriptProviders
    // will use createContentEnumeration

    m_xMgr = m_xContext->getServiceManager();
    ENSURE_OR_THROW( m_xMgr.is(), "ProviderCache::ProviderCache() failed to obtain ServiceManager" );
    populateCache();
}

ProviderCache::~ProviderCache()
{
}

Reference< provider::XScriptProvider >
ProviderCache::getProvider( const OUString& providerName )
{
    std::scoped_lock aGuard( m_mutex );
    Reference< provider::XScriptProvider > provider;
    ProviderDetails_hash::iterator h_it = m_hProviderDetailsCache.find( providerName );
    if ( h_it != m_hProviderDetailsCache.end() )
    {
        if (  h_it->second.provider.is() )
        {
            provider = h_it->second.provider;
        }
        else
        {
            // need to create provider and insert into hash
            provider = createProvider( h_it->second );
        }
    }
    return provider;
}

Sequence < Reference< provider::XScriptProvider > >
ProviderCache::getAllProviders()
{
    // need to create providers that haven't been created already
    // so check what providers exist and what ones don't

    std::scoped_lock aGuard( m_mutex );
    Sequence < Reference< provider::XScriptProvider > > providers (  m_hProviderDetailsCache.size() );
    // should assert if size !>  0
    if (  !m_hProviderDetailsCache.empty() )
    {
        auto pproviders = providers.getArray();
        sal_Int32 providerIndex = 0;
        for (auto& rDetail : m_hProviderDetailsCache)
        {
            Reference<provider::XScriptProvider> xScriptProvider = rDetail.second.provider;
            if ( xScriptProvider.is() )
            {
                pproviders[ providerIndex++ ] = xScriptProvider;
            }
            else
            {
                // create provider
                try
                {
                    xScriptProvider = createProvider(rDetail.second);
                    pproviders[ providerIndex++ ] = xScriptProvider;
                }
                catch ( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("scripting");
                }
            }
        }

        if (providerIndex < providers.getLength())
        {
            providers.realloc( providerIndex );
        }

    }
    else
    {
        SAL_WARN("scripting", "no available providers, something very wrong!!!");
    }
    return providers;
}

void
ProviderCache::populateCache()
{
    // wrong name in services.rdb
    OUString serviceName;
    std::scoped_lock aGuard( m_mutex );
    try
    {
        Reference< container::XContentEnumerationAccess > xEnumAccess( m_xMgr, UNO_QUERY_THROW );
        Reference< container::XEnumeration > xEnum = xEnumAccess->createContentEnumeration ( u"com.sun.star.script.provider.LanguageScriptProvider"_ustr );

        while ( xEnum->hasMoreElements() )
        {

            Reference< lang::XSingleComponentFactory > factory( xEnum->nextElement(), UNO_QUERY_THROW );
            Reference< lang::XServiceInfo > xServiceInfo( factory, UNO_QUERY_THROW );

            const Sequence< OUString > serviceNames = xServiceInfo->getSupportedServiceNames();

            if ( serviceNames.hasElements() )
            {
                auto pName = std::find_if(serviceNames.begin(), serviceNames.end(),
                    [this](const OUString& rName) {
                        return rName.startsWith("com.sun.star.script.provider.ScriptProviderFor")
                            && !isInDenyList(rName);
                    });
                if (pName != serviceNames.end())
                {
                    serviceName = *pName;
                    ProviderDetails details;
                    details.factory = factory;
                    m_hProviderDetailsCache[ serviceName ] = details;
                }
            }
        }
    }
    catch ( const Exception &e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
                "ProviderCache::populateCache: couldn't obtain XSingleComponentFactory for " + serviceName
                + " " + e.Message,
                nullptr, anyEx );
    }
}

Reference< provider::XScriptProvider >
ProviderCache::createProvider( ProviderDetails& details )
{
    try
    {
        details.provider.set(
            details.factory->createInstanceWithArgumentsAndContext( m_Sctx, m_xContext ), UNO_QUERY_THROW );
    }
    catch ( const Exception& e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
                "ProviderCache::createProvider() Error creating provider from factory. " + e.Message,
                nullptr, anyEx );
    }

    return details.provider;
}

bool
ProviderCache::isInDenyList( const OUString& serviceName ) const
{
    return comphelper::findValue(m_sDenyList, serviceName) != -1;
}
} //end namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
