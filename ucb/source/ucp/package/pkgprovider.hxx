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

#ifndef _PKGPROVIDER_HXX
#define _PKGPROVIDER_HXX

#include <ucbhelper/providerhelper.hxx>
#include "pkguri.hxx"

namespace com { namespace sun { namespace star { namespace container {
    class XHierarchicalNameAccess;
} } } }

namespace package_ucp {

//=========================================================================

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
#define PACKAGE_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.PackageContentProvider"
#define PACKAGE_CONTENT_PROVIDER_SERVICE_NAME_LENGTH    39

// UCB Content Type.
#define PACKAGE_FOLDER_CONTENT_TYPE \
                "application/" PACKAGE_URL_SCHEME "-folder"
#define PACKAGE_STREAM_CONTENT_TYPE \
                "application/" PACKAGE_URL_SCHEME "-stream"
#define PACKAGE_ZIP_FOLDER_CONTENT_TYPE \
                "application/" PACKAGE_ZIP_URL_SCHEME "-folder"
#define PACKAGE_ZIP_STREAM_CONTENT_TYPE \
                "application/" PACKAGE_ZIP_URL_SCHEME "-stream"

//=========================================================================

class Packages;

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    Packages* m_pPackages;

public:
    ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess >
    createPackage( const OUString & rName, const OUString & rParam );
    sal_Bool
    removePackage( const OUString & rName );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
