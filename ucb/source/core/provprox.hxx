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

#ifndef _PROVPROX_HXX
#define _PROVPROX_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderSupplier.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>

//=========================================================================

#define PROVIDER_FACTORY_SERVICE_NAME \
                            "com.sun.star.ucb.ContentProviderProxyFactory"
#define PROVIDER_PROXY_SERVICE_NAME \
                            "com.sun.star.ucb.ContentProviderProxy"

//============================================================================
//
// class UcbContentProviderProxyFactory.
//
//============================================================================

class UcbContentProviderProxyFactory :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XContentProviderFactory
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;

public:
    UcbContentProviderProxyFactory(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr );
    virtual ~UcbContentProviderProxyFactory();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProviderFactory
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    createContentProvider( const ::rtl::OUString& Service )
        throw( ::com::sun::star::uno::RuntimeException );
};

//============================================================================
//
// class UcbContentProviderProxy.
//
//============================================================================

class UcbContentProviderProxy :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XContentProviderSupplier,
                public com::sun::star::ucb::XContentProvider,
                public com::sun::star::ucb::XParameterizedContentProvider
{
    ::osl::Mutex    m_aMutex;
    ::rtl::OUString m_aService;
    ::rtl::OUString m_aTemplate;
    ::rtl::OUString m_aArguments;
    sal_Bool        m_bReplace;
    sal_Bool        m_bRegister;

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
                                m_xProvider;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
                                m_xTargetProvider;

public:
    UcbContentProviderProxy(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const ::rtl::OUString& Service );
    virtual ~UcbContentProviderProxy();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XContentProviderSupplier
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    getContentProvider()
        throw( ::com::sun::star::uno::RuntimeException );

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    compareContentIds( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& Id1,
                       const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XContentIdentifier >& Id2 )
        throw( ::com::sun::star::uno::RuntimeException );

    // XParameterizedContentProvider
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    registerInstance( const ::rtl::OUString& Template,
                      const ::rtl::OUString& Arguments,
                      sal_Bool ReplaceExisting )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    deregisterInstance( const ::rtl::OUString& Template,
                        const ::rtl::OUString& Arguments )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );
};

#endif /* !_PROVPROX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
