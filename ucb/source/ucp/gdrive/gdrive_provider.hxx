/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <ucbhelper/providerhelper.hxx>
#include <memory>
#include <map>
#include <rtl/ref.hxx>

namespace ucp {
namespace gdrive {

class GoogleDriveApiClient;

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
inline constexpr OUString GDRIVE_CONTENT_PROVIDER_SERVICE_NAME = u"com.sun.star.ucb.GoogleDriveContentProvider"_ustr;

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider according to this scheme.
#define GDRIVE_URL_SCHEME u"gdrive"

// Content types
inline constexpr OUString GDRIVE_FILE_TYPE = u"application/gdrive-file"_ustr;
inline constexpr OUString GDRIVE_FOLDER_TYPE = u"application/gdrive-folder"_ustr;

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
private:
    // Cache of GoogleDriveApiClient instances per user/session
    std::map<OUString, std::shared_ptr<GoogleDriveApiClient>> m_aClientCache;

public:
    explicit ContentProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XContentProvider
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;

    // Helper methods
    std::shared_ptr<GoogleDriveApiClient> getApiClient( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );
    void registerApiClient( const OUString& sUserId, std::shared_ptr<GoogleDriveApiClient> pClient );

    static bool isGDriveURL( const OUString& rURL );
    static OUString getFileIdFromURL( const OUString& rURL );

    // Factory methods
    static css::uno::Reference< css::uno::XInterface > SAL_CALL CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr );
    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
};

} // namespace gdrive
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
