/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_WEBDAVPROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_WEBDAVPROVIDER_HXX

#include <config_lgpl.h>
#include <rtl/ref.hxx>
#include <com/sun/star/beans/Property.hpp>
#include "DAVSessionFactory.hxx"
#include <ucbhelper/providerhelper.hxx>
#include "PropertyMap.hxx"

namespace webdav_ucp {



// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
#define WEBDAV_CONTENT_PROVIDER_SERVICE_NAME "com.sun.star.ucb.WebDAVContentProvider"

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.
#define WEBDAV_URL_SCHEME           "vnd.sun.star.webdav"
#define WEBDAV_URL_SCHEME_LENGTH    19

#define WEBDAVS_URL_SCHEME "vnd.sun.star.webdavs"
#define WEBDAVS_URL_SCHEME_LENGTH 20

#define HTTP_URL_SCHEME         "http"

#define HTTPS_URL_SCHEME        "https"

#define DAV_URL_SCHEME          "dav"
#define DAV_URL_SCHEME_LENGTH   3

#define DAVS_URL_SCHEME         "davs"
#define DAVS_URL_SCHEME_LENGTH  4

#define FTP_URL_SCHEME "ftp"

#define HTTP_CONTENT_TYPE      "application/" HTTP_URL_SCHEME "-content"

#define WEBDAV_CONTENT_TYPE    HTTP_CONTENT_TYPE
#define WEBDAV_COLLECTION_TYPE "application/" WEBDAV_URL_SCHEME "-collection"



class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    rtl::Reference< DAVSessionFactory > m_xDAVSessionFactory;
    PropertyMap * m_pProps;

public:
    explicit ContentProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference<
                          css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XContentProvider
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
        throw( css::ucb::IllegalIdentifierException,
               css::uno::RuntimeException, std::exception ) override;


    // Non-interface methods.

    bool getProperty( const OUString & rPropName,
                      css::beans::Property & rProp,
                      bool bStrict = false );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
