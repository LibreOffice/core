/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gdrive_provider.hxx"
#include "gdrive_content.hxx"
#include "GoogleDriveApiClient.hxx"

#include <sal/log.hxx>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/macros.hxx>

using namespace com::sun::star;
using namespace ucp::gdrive;

ContentProvider::ContentProvider( const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext )
{
}

ContentProvider::~ContentProvider()
{
}

// XInterface methods
uno::Any SAL_CALL ContentProvider::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          static_cast< lang::XTypeProvider* >(this),
                                          static_cast< lang::XServiceInfo* >(this),
                                          static_cast< ucb::XContentProvider* >(this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

void SAL_CALL ContentProvider::acquire() noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release() noexcept
{
    OWeakObject::release();
}

// XTypeProvider methods
uno::Sequence< sal_Int8 > SAL_CALL ContentProvider::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Sequence< uno::Type > SAL_CALL ContentProvider::getTypes()
{
    static cppu::OTypeCollection s_aCollection(
        CPPU_TYPE_REF( lang::XTypeProvider ),
        CPPU_TYPE_REF( lang::XServiceInfo ),
        CPPU_TYPE_REF( ucb::XContentProvider ) );

    return s_aCollection.getTypes();
}

// XServiceInfo methods
OUString SAL_CALL ContentProvider::getImplementationName()
{
    return u"com.sun.star.comp.ucb.GoogleDriveContentProvider"_ustr;
}

sal_Bool SAL_CALL ContentProvider::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL ContentProvider::getSupportedServiceNames()
{
    return { GDRIVE_CONTENT_PROVIDER_SERVICE_NAME };
}

// XContentProvider methods
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent( const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    if ( !Identifier.is() )
    {
        SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent - No identifier provided");
        return uno::Reference< ucb::XContent >();
    }

    OUString sURL = Identifier->getContentIdentifier();
    SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent called for URL: " + sURL);

    // Check if this is a gdrive URL
    if ( !isGDriveURL( sURL ) )
    {
        SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent - Not a gdrive URL: " + sURL);
        throw ucb::IllegalIdentifierException(
            u"Invalid Google Drive URL: "_ustr + sURL,
            static_cast< cppu::OWeakObject * >( this ) );
    }

    // Extract file ID for debugging
    OUString sFileId = getFileIdFromURL( sURL );
    SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent - Extracted file ID: " + sFileId);

    // Create content object
    try
    {
        uno::Reference< ucb::XContentIdentifier > xCanonicalId =
            new ::ucbhelper::ContentIdentifier( sURL );

        SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent - Creating Content object");
        uno::Reference< ucb::XContent > xContent = new Content( m_xContext, this, xCanonicalId );
        SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent - Content object created successfully");
        return xContent;
    }
    catch ( const ucb::ContentCreationException& e )
    {
        SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent - ContentCreationException: " + e.Message);
        throw ucb::IllegalIdentifierException(
            u"Cannot create content for: "_ustr + sURL,
            static_cast< cppu::OWeakObject * >( this ) );
    }
    catch ( const uno::Exception& e )
    {
        SAL_WARN("ucb.ucp.gdrive", "ContentProvider::queryContent - Exception: " + e.Message);
        throw ucb::IllegalIdentifierException(
            u"Cannot create content for: "_ustr + sURL,
            static_cast< cppu::OWeakObject * >( this ) );
    }
}

// Helper methods
std::shared_ptr<GoogleDriveApiClient> ContentProvider::getApiClient(
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    // For now, use a single shared client
    // TODO: Implement per-user caching based on authentication
    OUString sUserId = u"default"_ustr;

    auto it = m_aClientCache.find( sUserId );
    if ( it != m_aClientCache.end() && xEnv.is() )
    {
        // If we have a command environment and there's a cached client,
        // we might need to recreate it with the proper environment for authentication
        // For now, always create a new client when we have an environment
        auto pClient = std::make_shared<GoogleDriveApiClient>( xEnv );
        m_aClientCache[sUserId] = pClient;
        return pClient;
    }
    else if ( it != m_aClientCache.end() )
    {
        return it->second;
    }

    // Create new client
    auto pClient = std::make_shared<GoogleDriveApiClient>( xEnv );
    m_aClientCache[sUserId] = pClient;

    return pClient;
}

void ContentProvider::registerApiClient( const OUString& sUserId,
                                        std::shared_ptr<GoogleDriveApiClient> pClient )
{
    m_aClientCache[sUserId] = pClient;
}

bool ContentProvider::isGDriveURL( const OUString& rURL )
{
    return rURL.startsWithIgnoreAsciiCase( u"gdrive://"_ustr );
}

OUString ContentProvider::getFileIdFromURL( const OUString& rURL )
{
    // Parse gdrive://file_id or gdrive://folder_id from URL
    // Format: gdrive://root or gdrive://1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms

    if ( !isGDriveURL( rURL ) )
        return OUString();

    OUString sPath = rURL.copy( 9 ); // Remove "gdrive://"

    // Remove leading slash if present
    if ( sPath.startsWith( u"/"_ustr ) )
        sPath = sPath.copy( 1 );

    // Extract file ID (everything up to next slash or end)
    sal_Int32 nSlash = sPath.indexOf( '/' );
    if ( nSlash != -1 )
        sPath = sPath.copy( 0, nSlash );

    return sPath.isEmpty() ? u"root"_ustr : sPath;
}

// Factory methods
uno::Reference< uno::XInterface > SAL_CALL ContentProvider::CreateInstance(
    const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
{
    lang::XServiceInfo* pX = static_cast<lang::XServiceInfo*>(
        new ContentProvider( comphelper::getComponentContext( rSMgr ) ) );
    return uno::Reference< uno::XInterface >::query( pX );
}

OUString ContentProvider::getImplementationName_Static()
{
    return u"com.sun.star.comp.ucb.GoogleDriveContentProvider"_ustr;
}

uno::Sequence< OUString > ContentProvider::getSupportedServiceNames_Static()
{
    return { GDRIVE_CONTENT_PROVIDER_SERVICE_NAME };
}

// Component registration functions
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_gdrive_ContentProvider_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ContentProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */