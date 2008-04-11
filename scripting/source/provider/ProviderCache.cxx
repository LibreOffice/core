/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ProviderCache.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include "ProviderCache.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace ::scripting_util;

namespace func_provider
{

ProviderCache::ProviderCache( const Reference< XComponentContext >& xContext, const Sequence< Any >& scriptContext )
    throw ( RuntimeException ) : m_Sctx( scriptContext ), m_xContext( xContext )
{
    // initialise m_hProviderDetailsCache with details of ScriptProviders
    // will use createContentEnumeration

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr, "ProviderCache::ProviderCache() failed to obtain ServiceManager" );
    populateCache();
}


ProviderCache::ProviderCache( const Reference< XComponentContext >& xContext, const Sequence< Any >& scriptContext, const Sequence< ::rtl::OUString >& blackList )
    throw ( RuntimeException ) : m_sBlackList( blackList ), m_Sctx( scriptContext ), m_xContext( xContext )

{
    // initialise m_hProviderDetailsCache with details of ScriptProviders
    // will use createContentEnumeration

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr, "ProviderCache::ProviderCache() failed to obtain ServiceManager" );
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
                catch ( Exception& e )
                {
                    ::rtl::OUString temp = OUSTR( "ProviderCache::getAllProviders: failed to create provider, " );
                    temp.concat( e.Message );
                    //throw RuntimeException( temp.concat( e.Message ),
                    //    Reference< XInterface >() );
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
        ::rtl::OUString languageProviderName( RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.script.provider.LanguageScriptProvider" ) );

        Reference< container::XContentEnumerationAccess > xEnumAccess = Reference< container::XContentEnumerationAccess >( m_xMgr, UNO_QUERY_THROW );
        Reference< container::XEnumeration > xEnum = xEnumAccess->createContentEnumeration ( languageProviderName );

        while ( xEnum->hasMoreElements() )
        {

            Reference< lang::XSingleComponentFactory > factory;
            if ( sal_False == ( xEnum->nextElement() >>= factory ) )
            {
                throw new RuntimeException( ::rtl::OUString::createFromAscii( "  error extracting XSingleComponentFactory from Content enumeration. " ), Reference< XInterface >() );
            }
            validateXRef( factory, "ProviderCache::populateCache() invalid factory" );
            Reference< lang::XServiceInfo > xServiceInfo( factory, UNO_QUERY_THROW );
            validateXRef( xServiceInfo, "ProviderCache::populateCache() failed to get XServiceInfo from factory" );

            Sequence< ::rtl::OUString > serviceNames = xServiceInfo->getSupportedServiceNames();

            if ( serviceNames.getLength() > 0 )
            {
                ::rtl::OUString searchString( RTL_CONSTASCII_USTRINGPARAM (
                    "com.sun.star.script.provider.ScriptProviderFor" ) );

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
    catch ( Exception e )
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
        details.provider  = Reference< provider::XScriptProvider >(
            details.factory->createInstanceWithArgumentsAndContext( m_Sctx, m_xContext ), UNO_QUERY_THROW );
        validateXRef( details.provider, "ProviderCache::createProvider, failed to create provider");
    }
    catch ( RuntimeException& e )
    {
        ::rtl::OUString temp = ::rtl::OUString::createFromAscii("ProviderCache::createProvider() Error creating provider from factory!!!");
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }

    return details.provider;
}
} //end namespace
