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

#pragma once

#include <memory>
#include <config_lgpl.h>
#include <rtl/ref.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
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
#define VNDSUNSTARWEBDAV_URL_SCHEME  "vnd.sun.star.webdav"
#define VNDSUNSTARWEBDAVS_URL_SCHEME u"vnd.sun.star.webdavs"
#define HTTP_URL_SCHEME              "http"
#define HTTPS_URL_SCHEME             "https"
#define DAV_URL_SCHEME               u"dav"
#define DAVS_URL_SCHEME              u"davs"
#define WEBDAV_URL_SCHEME            u"webdav"
#define WEBDAVS_URL_SCHEME           u"webdavs"

#define FTP_URL_SCHEME "ftp"

#define HTTP_CONTENT_TYPE      "application/" HTTP_URL_SCHEME "-content"

#define WEBDAV_CONTENT_TYPE    HTTP_CONTENT_TYPE
#define WEBDAV_COLLECTION_TYPE "application/" VNDSUNSTARWEBDAV_URL_SCHEME "-collection"


class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    rtl::Reference< DAVSessionFactory > m_xDAVSessionFactory;
    std::unique_ptr<PropertyMap> m_pProps;

public:
    explicit ContentProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

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


    // Non-interface methods.

    void getProperty( const OUString & rPropName,
                      css::beans::Property & rProp );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
