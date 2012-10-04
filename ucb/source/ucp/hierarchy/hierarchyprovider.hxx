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

#ifndef _HIERARCHYPROVIDER_HXX
#define _HIERARCHYPROVIDER_HXX

#include <boost/unordered_map.hpp>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/lang/XInitialization.hpp>

namespace com { namespace sun { namespace star {
    namespace container {
        class XHierarchicalNameAccess;
    }
    namespace util {
        class XOfficeInstallationDirectories;
    }
} } }

namespace hierarchy_ucp {

//=========================================================================

#define HIERARCHY_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.HierarchyContentProvider"
#define HIERARCHY_CONTENT_PROVIDER_SERVICE_NAME_LENGTH  41

#define HIERARCHY_URL_SCHEME \
                "vnd.sun.star.hier"
#define HIERARCHY_URL_SCHEME_LENGTH 17

#define HIERARCHY_FOLDER_CONTENT_TYPE \
                "application/" HIERARCHY_URL_SCHEME "-folder"
#define HIERARCHY_LINK_CONTENT_TYPE \
                "application/" HIERARCHY_URL_SCHEME "-link"

//=========================================================================

struct ConfigProviderMapEntry
{
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > xConfigProvider;
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess > xRootReadAccess;
    bool bTriedToGetRootReadAccess;  // #82494#

    ConfigProviderMapEntry() : bTriedToGetRootReadAccess( false ) {}
};

struct equalString
{
    bool operator()(
        const rtl::OUString& rKey1, const rtl::OUString& rKey2 ) const
    {
        return !!( rKey1 == rKey2 );
    }
};

struct hashString
{
    size_t operator()( const rtl::OUString & rName ) const
    {
        return rName.hashCode();
    }
};

typedef boost::unordered_map
<
    rtl::OUString,  // servcie specifier
    ConfigProviderMapEntry,
    hashString,
    equalString
>
ConfigProviderMap;

//=========================================================================

class HierarchyContentProvider : public ::ucbhelper::ContentProviderImplHelper,
                                 public com::sun::star::lang::XInitialization
{
    ConfigProviderMap   m_aConfigProviderMap;
    com::sun::star::uno::Reference<
        com::sun::star::util::XOfficeInstallationDirectories > m_xOfficeInstDirs;

public:
    HierarchyContentProvider(
                const com::sun::star::uno::Reference<
                    com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~HierarchyContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( com::sun::star::ucb::IllegalIdentifierException,
               com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL
    initialize( const ::com::sun::star::uno::Sequence<
                        ::com::sun::star::uno::Any >& aArguments )
        throw( ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException );

    // Non-Interface methods
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >
    getConfigProvider( const rtl::OUString & rServiceSpecifier );
    com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    getRootConfigReadNameAccess( const rtl::OUString & rServiceSpecifier );

    // Note: may retrun an empty reference.
    com::sun::star::uno::Reference<
        com::sun::star::util::XOfficeInstallationDirectories >
    getOfficeInstallationDirectories();
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYPROVIDER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
