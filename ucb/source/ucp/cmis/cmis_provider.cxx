/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>

#include "cmis_content.hxx"
#include "cmis_provider.hxx"
#include "cmis_repo_content.hxx"

using namespace com::sun::star;

namespace cmis
{
uno::Reference< css::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference< css::ucb::XContentIdentifier >& Identifier )
{
    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent = queryExistingContent( Identifier );
    if ( xContent.is() )
        return xContent;

    try
    {
        URL aUrl( Identifier->getContentIdentifier( ) );
        if ( aUrl.getRepositoryId( ).isEmpty( ) )
        {
            xContent = new RepoContent( m_xContext, this, Identifier );
            registerNewContent( xContent );
        }
        else
        {
            xContent = new Content( m_xContext, this, Identifier );
            registerNewContent( xContent );
        }
    }
    catch ( css::ucb::ContentCreationException const & )
    {
        throw css::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw css::ucb::IllegalIdentifierException();

    return xContent;
}

libcmis::Session* ContentProvider::getSession( const OUString& sBindingUrl, const OUString& sUsername )
{
    libcmis::Session* pSession = nullptr;
    std::map< std::pair< OUString, OUString >, libcmis::Session* >::iterator it
            = m_aSessionCache.find( std::pair< OUString, OUString >( sBindingUrl, sUsername ) );
    if ( it != m_aSessionCache.end( ) )
    {
        pSession = it->second;
    }
    return pSession;
}

void ContentProvider::registerSession( const OUString& sBindingUrl, const OUString& sUsername, libcmis::Session* pSession )
{
    m_aSessionCache.insert( std::pair< std::pair< OUString, OUString >, libcmis::Session* >
                            (
                                std::pair< OUString, OUString >( sBindingUrl, sUsername ),
                                pSession
                            ) );
}

ContentProvider::ContentProvider(
    const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext )
{
}

ContentProvider::~ContentProvider()
{
}

//XInterface
void SAL_CALL ContentProvider::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProvider::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< lang::XTypeProvider* >(this),
                                               static_cast< lang::XServiceInfo* >(this),
                                               static_cast< css::ucb::XContentProvider* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      css::ucb::XContentProvider );

sal_Bool ContentProvider::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}
OUString ContentProvider::getImplementationName()
{
    return "com.sun.star.comp.CmisContentProvider";
}
css::uno::Sequence< OUString > ContentProvider::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.CmisContentProvider" };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_cmis_ContentProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new cmis::ContentProvider(context)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
