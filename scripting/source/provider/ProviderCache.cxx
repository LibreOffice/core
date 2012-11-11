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

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <tools/diagnose_ex.h>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include "ProviderCache.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;

namespace func_provider
{

ProviderCache::ProviderCache( const Reference< XComponentContext >& xContext, const Sequence< Any >& scriptContext )
    throw ( RuntimeException ) : m_Sctx( scriptContext ), m_xContext( xContext )
{
    // initialise m_hProviderDetailsCache with details of ScriptProviders
    // will use createContentEnumeration

    m_xMgr = m_xContext->getServiceManager();
    ENSURE_OR_THROW( m_xMgr.is(), "ProviderCache::ProviderCache() failed to obtain ServiceManager" );
    populateCache();
}


ProviderCache::ProviderCache( const Reference< XComponentContext >& xContext, const Sequence< Any >& scriptContext, const Sequence< ::rtl::OUString >& blackList )
    throw ( RuntimeException ) : m_sBlackList( blackList ), m_Sctx( scriptContext ), m_xContext( xContext )

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
ProviderCache::getProvider( const ::rtl::OUString& providerName )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
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
ProviderCache::getAllProviders() throw ( RuntimeException )
{
    Sequence < Reference< provider::XScriptProvider > > providers (  m_hProviderDetailsCache.size() );
    // need to create providers that haven't been created already
    // so check what providers exist and what ones don't

    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    ProviderDetails_hash::iterator h_itEnd =  m_hProviderDetailsCache.end();
    ProviderDetails_hash::iterator h_it = m_hProviderDetailsCache.begin();
    // should assert if size !>  0
    if (  m_hProviderDetailsCache.size() )
    {
        sal_Int32 providerIndex = 0;
    sal_Int32 index = 0;
        for ( index = 0; h_it !=  h_itEnd; ++h_it, index++ )
        {
            Reference< provider::XScriptProvider > xScriptProvider  = h_it->second.provider;
            if ( xScriptProvider.is() )
            {
                providers[ providerIndex++ ] = xScriptProvider;
            }
            else
            {
                // create provider
                try
                {
                    xScriptProvider  = createProvider( h_it->second );
                    providers[ providerIndex++ ] = xScriptProvider;
                }
                catch ( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }

        if ( providerIndex < index )
        {
            providers.realloc( providerIndex );
        }

    }
    else
    {
        OSL_TRACE("no available providers, something very wrong!!!");
    }
    return providers;
}

void
ProviderCache::populateCache() throw ( RuntimeException )
{
    // wrong name in services.rdb
    ::rtl::OUString serviceName;
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    try
    {
        ::rtl::OUString languageProviderName( "com.sun.star.script.provider.LanguageScriptProvider"  );

        Reference< container::XContentEnumerationAccess > xEnumAccess = Reference< container::XContentEnumerationAccess >( m_xMgr, UNO_QUERY_THROW );
        Reference< container::XEnumeration > xEnum = xEnumAccess->createContentEnumeration ( languageProviderName );

        while ( xEnum->hasMoreElements() )
        {

            Reference< lang::XSingleComponentFactory > factory( xEnum->nextElement(), UNO_QUERY_THROW );
            Reference< lang::XServiceInfo > xServiceInfo( factory, UNO_QUERY_THROW );

            Sequence< ::rtl::OUString > serviceNames = xServiceInfo->getSupportedServiceNames();

            if ( serviceNames.getLength() > 0 )
            {
                ::rtl::OUString searchString( "com.sun.star.script.provider.ScriptProviderFor"  );

                for ( sal_Int32 index = 0; index < serviceNames.getLength(); index++ )
                {
                    if ( serviceNames[ index ].indexOf( searchString ) == 0 && !isInBlackList(  serviceNames[ index ] ) )
                    {
                        serviceName = serviceNames[ index ];
                        ProviderDetails details;
                        details.factory = factory;
                        m_hProviderDetailsCache[ serviceName ] = details;
                        break;
                    }
                }
            }
        }
    }
    catch ( const Exception &e )
    {
        ::rtl::OUString temp = OUSTR(
            "ProviderCache::populateCache: couldn't obtain XSingleComponentFactory for " );
        temp.concat( serviceName );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
}

Reference< provider::XScriptProvider >
ProviderCache::createProvider( ProviderDetails& details ) throw ( RuntimeException )
{
    try
    {
        details.provider.set(
            details.factory->createInstanceWithArgumentsAndContext( m_Sctx, m_xContext ), UNO_QUERY_THROW );
    }
    catch ( const Exception& e )
    {
        ::rtl::OUString temp("ProviderCache::createProvider() Error creating provider from factory!!!\n");
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }

    return details.provider;
}
} //end namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
