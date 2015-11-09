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

#ifndef INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYPROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYPROVIDER_HXX

#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <unordered_map>

namespace com { namespace sun { namespace star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace util {
        class XOfficeInstallationDirectories;
    }
} } }

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
    OUString,  // servcie specifier
    ConfigProviderMapEntry,
    OUStringHash
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
    virtual ~HierarchyContentProvider();

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

    // XInitialization
    virtual void SAL_CALL
    initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw( css::uno::Exception,
               css::uno::RuntimeException, std::exception ) override;

    // Non-Interface methods
    css::uno::Reference< css::lang::XMultiServiceFactory >
    getConfigProvider( const OUString & rServiceSpecifier );
    css::uno::Reference< css::container::XHierarchicalNameAccess >
    getRootConfigReadNameAccess( const OUString & rServiceSpecifier );

    // Note: may retrun an empty reference.
    css::uno::Reference< css::util::XOfficeInstallationDirectories >
    getOfficeInstallationDirectories();
};

} // namespace hierarchy_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
