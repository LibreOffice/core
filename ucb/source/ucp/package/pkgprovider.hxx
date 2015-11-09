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

#ifndef INCLUDED_UCB_SOURCE_UCP_PACKAGE_PKGPROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_PACKAGE_PKGPROVIDER_HXX

#include <ucbhelper/providerhelper.hxx>
#include "pkguri.hxx"

namespace com { namespace sun { namespace star { namespace container {
    class XHierarchicalNameAccess;
} } } }

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



class Packages;

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    Packages* m_pPackages;

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


    // Additional interfaces



    // Non-interface methods.


    css::uno::Reference< css::container::XHierarchicalNameAccess >
    createPackage( const PackageUri & rParam );
    bool
    removePackage( const OUString & rName );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
