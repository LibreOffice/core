/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <memory>
#include <ucbhelper/providerhelper.hxx>
#include "pkguri.hxx"

namespace com::sun::star::container {
    class XHierarchicalNameAccess;
}

namespace package_ucp {


// UCB Content Type.
#define PACKAGE_FOLDER_CONTENT_TYPE \
                "application/" PACKAGE_URL_SCHEME "-folder"
#define PACKAGE_STREAM_CONTENT_TYPE \
                "application/" PACKAGE_URL_SCHEME "-stream"
#define PACKAGE_ZIP_FOLDER_CONTENT_TYPE \
                "application/" PACKAGE_ZIP_URL_SCHEME "-folder"
#define PACKAGE_ZIP_STREAM_CONTENT_TYPE \
                "application/" PACKAGE_ZIP_URL_SCHEME "-stream"


class Package;

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    std::unordered_map<OUString, Package*> m_aPackages;

public:
    explicit ContentProvider( const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider() override;

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
    virtual void acquire()
        noexcept override;
    virtual void release()
        noexcept override;

    // XTypeProvider
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XContentProvider
    virtual cpo::uno::Reference< css::ucb::XContent >
    queryContent( const cpo::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;


    // Non-interface methods.


    cpo::uno::Reference< css::container::XHierarchicalNameAccess >
    createPackage( const PackageUri & rParam );
    void
    removePackage( const OUString & rName );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
