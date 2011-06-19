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
