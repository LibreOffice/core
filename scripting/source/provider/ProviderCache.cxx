/*************************************************************************
 *
 *  $RCSfile: ProviderCache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: npower $ $Date: 2003-09-15 14:32:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include "ProviderCache.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace drafts::com::sun::star::script::framework;

namespace func_provider
{
    ::rtl::OUString languageProviderName =
        ::rtl::OUString::createFromAscii("drafts.com.sun.star.script.framework.provider.LanguageScriptProvider" );
    ::rtl::OUString providerKey =
        ::rtl::OUString::createFromAscii("drafts.com.sun.star.script.framework.provider.ScriptProviderFor" );

ProviderCache::ProviderCache( const Reference< XComponentContext >& xContext, const Sequence< Any >& scriptContext )
    throw ( RuntimeException ) : m_Sctx( scriptContext ), m_xContext( xContext )
{
    OSL_TRACE("ProviderCache::ProviderCache() - ctor");
    // initialise m_hProviderDetailsCache with details of ScriptProviders
    // will use createContentEnumeration

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr, "ProviderCache::ProviderCache() failed to obtain ServiceManager" );
    OSL_TRACE("ProviderCache::ProviderCache() about to populateCache");
    populateCache();
}

ProviderCache::~ProviderCache()
{
    OSL_TRACE("ProviderCache::ProviderCache() - dtor");
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
            OSL_TRACE("ProviderCache::getProvider(), service for %s already in cache",
                ::rtl::OUStringToOString( h_it->first,
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            provider = h_it->second.provider;
        }
    else
    {
            OSL_TRACE("ProviderCache::getProvider(), %s not in cache, try to create.",
                ::rtl::OUStringToOString( h_it->first,
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        // need to create provider and insert into hash
            provider = createProvider( h_it->second );
    }
    }
    return provider;
}

Sequence < Reference< provider::XScriptProvider > >
ProviderCache::getAllProviders() throw ( RuntimeException )
{
    OSL_TRACE("ProviderCache::getAllProviders()");
    Sequence < Reference< provider::XScriptProvider > > providers (  m_hProviderDetailsCache.size() );
    // need to create providers that haven't been created already
    // so check what providers exist and what ones don't

    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    ProviderDetails_hash::iterator h_itEnd =  m_hProviderDetailsCache.end();
    ProviderDetails_hash::iterator h_it = m_hProviderDetailsCache.begin();
    // should assert if size !>  0
    if (  m_hProviderDetailsCache.size() )
    {
        for ( sal_Int32 index  = 0; h_it !=  h_itEnd; ++h_it, index++ )
        {
            Reference< provider::XScriptProvider > xScriptProvider  = h_it->second.provider;
            if ( xScriptProvider.is() )
            {
                OSL_TRACE("ProviderCache::getAllProviders(), service for %s already in cache",
                    ::rtl::OUStringToOString( h_it->first,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                providers[ index ] = xScriptProvider;
            }
            else
            {
                OSL_TRACE("ProviderCache::getAllProviders(), no service in cache for %s",
                    ::rtl::OUStringToOString( h_it->first,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                // create provider
                try
                {
                    xScriptProvider  = createProvider( h_it->second );
                    providers[ index ] = xScriptProvider;
                }
                catch ( RuntimeException& e )
                {
                    ::rtl::OUString temp = OUSTR( "ProviderCache::getAllProviders: failed to create provider, " );
                    temp.concat( e.Message );
                    throw RuntimeException( temp.concat( e.Message ),
                        Reference< XInterface >() );
                }
            }
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
    OSL_TRACE("ProviderCache::populateCache()");
    // wrong name in services.rdb
    ::rtl::OUString serviceName;
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    try
    {
        Reference< container::XContentEnumerationAccess > xEnumAccess = Reference< container::XContentEnumerationAccess >( m_xMgr, UNO_QUERY_THROW );
        Reference< container::XEnumeration > xEnum = xEnumAccess->createContentEnumeration ( languageProviderName );

        while ( xEnum->hasMoreElements() )
        {

            Reference< lang::XSingleComponentFactory > factory;
            if ( sal_False == ( xEnum->nextElement() >>= factory ) )
            {
                OSL_TRACE("ProviderCache::populateCache() failed to extract factory from any");
                throw new RuntimeException( ::rtl::OUString::createFromAscii( "  error extracting XSingleComponentFactory from Content enumeration. " ), Reference< XInterface >() );
            }
            validateXRef( factory, "ProviderCache::populateCache() invalid factory" );
            Reference< lang::XServiceInfo > xServiceInfo( factory, UNO_QUERY_THROW );
            validateXRef( xServiceInfo, "ProviderCache::populateCache() failed to get XServiceInfo from factory" );
            OSL_TRACE("ProviderCache::populateCache() processing element for implementation name %s",
                ::rtl::OUStringToOString( xServiceInfo->getImplementationName(),
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );

            Sequence< ::rtl::OUString > serviceNames = xServiceInfo->getSupportedServiceNames();

            if ( serviceNames.getLength() > 0 )
            {
                for ( sal_Int32 index = 0; index < serviceNames.getLength(); index++ )
                {
                    if ( serviceNames[ index ].indexOf( providerKey ) == 0 )
                    {
                        serviceName = serviceNames[ index ];
                        OSL_TRACE("ProviderCache::populateCache(), creating entry with factory for service %s",
                            ::rtl::OUStringToOString( serviceName,
                                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
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
   OSL_TRACE("ProviderCache::createProvider()");
    try
    {
        details.provider  = Reference< provider::XScriptProvider >( details.factory->createInstanceWithArgumentsAndContext( m_Sctx, m_xContext ), UNO_QUERY_THROW );
        validateXRef( details.provider, "ProviderCache::createProvider, failed to create provider");
    }
    catch ( RuntimeException& e )
    {
        OSL_TRACE("Error creating provider from factory!!!");
        ::rtl::OUString temp = ::rtl::OUString::createFromAscii("ProviderCache::createProvider() Error creating provider from factory!!!");
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }

    return details.provider;
}
} //end namespace
