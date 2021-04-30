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

#pragma once

#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <unordered_map>

namespace com::sun::star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace util {
        class XOfficeInstallationDirectories;
    }
}

namespace hierarchy_ucp {


#define HIERARCHY_URL_SCHEME \
                "vnd.sun.star.hier"
#define HIERARCHY_URL_SCHEME_LENGTH 17

#define HIERARCHY_FOLDER_CONTENT_TYPE \
                "application/" HIERARCHY_URL_SCHEME "-folder"
#define HIERARCHY_LINK_CONTENT_TYPE \
                "application/" HIERARCHY_URL_SCHEME "-link"

struct ConfigProviderMapEntry
{
    css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider;
    css::uno::Reference< css::container::XHierarchicalNameAccess > xRootReadAccess;
    bool bTriedToGetRootReadAccess;

    ConfigProviderMapEntry() : bTriedToGetRootReadAccess( false ) {}
};

typedef std::unordered_map
<
    OUString,  // service specifier
    ConfigProviderMapEntry
>
ConfigProviderMap;

class HierarchyContentProvider : public ::ucbhelper::ContentProviderImplHelper,
                                 public css::lang::XInitialization
{
    ConfigProviderMap   m_aConfigProviderMap;
    css::uno::Reference< css::util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

public:
    explicit HierarchyContentProvider(
                const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~HierarchyContentProvider() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        noexcept override;
    virtual void SAL_CALL release()
        noexcept override;

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

    // XInitialization
    virtual void SAL_CALL
    initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // Non-Interface methods
    css::uno::Reference< css::lang::XMultiServiceFactory >
    getConfigProvider( const OUString & rServiceSpecifier );
    css::uno::Reference< css::container::XHierarchicalNameAccess >
    getRootConfigReadNameAccess( const OUString & rServiceSpecifier );

    // Note: may return an empty reference.
    css::uno::Reference< css::util::XOfficeInstallationDirectories >
    getOfficeInstallationDirectories();
};

} // namespace hierarchy_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
