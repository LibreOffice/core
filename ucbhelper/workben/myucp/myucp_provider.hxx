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

// @@@ Adjust multi-include-protection-ifdef.
#ifndef _MYUCP_PROVIDER_HXX
#define _MYUCP_PROVIDER_HXX

#include "ucbhelper/providerhelper.hxx"

// @@@ Adjust namespace name.
namespace myucp {

//=========================================================================

// @@@ Adjust defines.

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider. Prefix with reversed company domain name.
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.MyContentProvider"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH  34

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.
#define MYUCP_URL_SCHEME \
                "vnd.sun.star.myucp"
#define MYUCP_URL_SCHEME_LENGTH 18

// UCB Content Type.
#define MYUCP_CONTENT_TYPE \
                "application/" MYUCP_URL_SCHEME "-content"

//=========================================================================

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
public:
    ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );
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
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
