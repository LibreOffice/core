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



#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_WEBDAVPROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_WEBDAVPROVIDER_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/beans/Property.hpp>
#include "DAVSessionFactory.hxx"
#include <ucbhelper/providerhelper.hxx>
#include "PropertyMap.hxx"

namespace http_dav_ucp {



// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
#define WEBDAV_CONTENT_PROVIDER_SERVICE_NAME "com.sun.star.ucb.WebDAVContentProvider"

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.
#define WEBDAV_URL_SCHEME       "vnd.sun.star.webdav"
#define WEBDAV_URL_SCHEME_LENGTH    19

#define WEBDAVS_URL_SCHEME "vnd.sun.star.webdavs"
#define WEBDAVS_URL_SCHEME_LENGTH 20

#define HTTP_URL_SCHEME         "http"

#define HTTPS_URL_SCHEME        "https"

#define DAV_URL_SCHEME          "dav"
#define DAV_URL_SCHEME_LENGTH   3

#define DAVS_URL_SCHEME         "davs"
#define DAVS_URL_SCHEME_LENGTH  4

#define HTTP_CONTENT_TYPE      "application/" HTTP_URL_SCHEME "-content"

#define WEBDAV_CONTENT_TYPE    HTTP_CONTENT_TYPE
#define WEBDAV_COLLECTION_TYPE "application/" WEBDAV_URL_SCHEME "-collection"



class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    rtl::Reference< DAVSessionFactory > m_xDAVSessionFactory;
    PropertyMap * m_pProps;

public:
    explicit ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext >& rContext );
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
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException ) override;


    // Non-interface methods.

    bool getProperty( const OUString & rPropName,
                      ::com::sun::star::beans::Property & rProp,
                      bool bStrict = false );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
