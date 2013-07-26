/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>

#include "cmis_content.hxx"
#include "cmis_provider.hxx"
#include "cmis_repo_content.hxx"

using namespace com::sun::star;

namespace cmis
{
uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
    throw( com::sun::star::ucb::IllegalIdentifierException,
           uno::RuntimeException )
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
    catch ( com::sun::star::ucb::ContentCreationException const & )
    {
        throw com::sun::star::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw com::sun::star::ucb::IllegalIdentifierException();

    return xContent;
}

libcmis::Session* ContentProvider::getSession( const OUString& sBindingUrl )
{
    libcmis::Session* pSession = NULL;
    std::map< OUString, libcmis::Session* >::iterator it = m_aSessionCache.find( sBindingUrl );
    if ( it != m_aSessionCache.end( ) )
    {
        pSession = it->second;
    }
    return pSession;
}

void ContentProvider::registerSession( const OUString& sBindingUrl, libcmis::Session* pSession )
{
    m_aSessionCache.insert( std::pair< OUString, libcmis::Session* >( sBindingUrl, pSession ) );
}

ContentProvider::ContentProvider(
    const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext )
{
}

ContentProvider::~ContentProvider()
{
}

XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   com::sun::star::ucb::XContentProvider );

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      com::sun::star::ucb::XContentProvider );

XSERVICEINFO_IMPL_1_CTX( ContentProvider,
                     OUString("com.sun.star.comp.CmisContentProvider"),
                     OUString("com.sun.star.ucb.CmisContentProvider") );

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpcmis1_component_getFactory( const sal_Char *pImplName,
    void *pServiceManager, void * )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr
        (reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( !::cmis::ContentProvider::getImplementationName_Static().compareToAscii( pImplName ) )
        xFactory = ::cmis::ContentProvider::createServiceFactory( xSMgr );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
