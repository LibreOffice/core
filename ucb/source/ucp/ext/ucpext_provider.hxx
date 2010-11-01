/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef UCB_UCPEXT_CONTENT_PROVIDER_HXX
#define UCB_UCPEXT_CONTENT_PROVIDER_HXX

#include <ucbhelper/providerhelper.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    //==================================================================================================================
    //= ContentProvider
    //==================================================================================================================
    typedef ::ucbhelper::ContentProviderImplHelper  ContentProvider_Base;
    class ContentProvider : public ContentProvider_Base
    {
    public:
        ContentProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_rServiceManager );
        virtual ~ContentProvider();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static() throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext );

        // XContentProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > SAL_CALL queryContent( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier >& Identifier ) throw (::com::sun::star::ucb::IllegalIdentifierException, ::com::sun::star::uno::RuntimeException);

    public:
        static ::rtl::OUString getRootURL();
        static ::rtl::OUString getArtificialNodeContentType();
    };

//......................................................................................................................
} } }   // namespace ucb::ucp::ext
//......................................................................................................................

#endif  // UCB_UCPEXT_CONTENT_PROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
