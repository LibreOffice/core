/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contenthelper.hxx>
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
    uno::Reference< ucb::XContent > xContent = queryExistingContent( Identifier ).get();
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
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< lang::XServiceInfo* >(this)),
                                               (static_cast< css::ucb::XContentProvider* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      css::ucb::XContentProvider );

XSERVICEINFO_COMMOM_IMPL( ContentProvider,
                          OUString("com.sun.star.comp.CmisContentProvider") )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
ContentProvider_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX =
        static_cast<css::lang::XServiceInfo*>(new ContentProvider( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString >
ContentProvider::getSupportedServiceNames_Static()
{
    css::uno::Sequence< OUString > aSNS { "com.sun.star.ucb.CmisContentProvider" };
    return aSNS;
}

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpcmis1_component_getFactory( const sal_Char *pImplName,
    void *pServiceManager, void * )
{
    void * pRet = nullptr;

    uno::Reference< lang::XMultiServiceFactory > xSMgr
        (static_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( ::cmis::ContentProvider::getImplementationName_Static().equalsAscii( pImplName ) )
        xFactory = ::cmis::ContentProvider::createServiceFactory( xSMgr );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
