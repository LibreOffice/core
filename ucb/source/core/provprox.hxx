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

#ifndef INCLUDED_UCB_SOURCE_CORE_PROVPROX_HXX
#define INCLUDED_UCB_SOURCE_CORE_PROVPROX_HXX

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
#include <cppuhelper/implbase.hxx>




// class UcbContentProviderProxyFactory.



class UcbContentProviderProxyFactory : public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::ucb::XContentProviderFactory >
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;

public:
    explicit UcbContentProviderProxyFactory(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr );
    virtual ~UcbContentProviderProxyFactory();

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

    // XContentProviderFactory
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    createContentProvider( const OUString& Service )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};



// class UcbContentProviderProxy.



class UcbContentProviderProxy :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XContentProviderSupplier,
                public com::sun::star::ucb::XContentProvider,
                public com::sun::star::ucb::XParameterizedContentProvider
{
    ::osl::Mutex    m_aMutex;
    OUString m_aService;
    OUString m_aTemplate;
    OUString m_aArguments;
    bool        m_bReplace;
    bool        m_bRegister;

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
            const OUString& Service );
    virtual ~UcbContentProviderProxy();

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

    // XContentProviderSupplier
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    getContentProvider()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL
    compareContentIds( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& Id1,
                       const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XContentIdentifier >& Id2 )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XParameterizedContentProvider
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    registerInstance( const OUString& Template,
                      const OUString& Arguments,
                      sal_Bool ReplaceExisting )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    deregisterInstance( const OUString& Template,
                        const OUString& Arguments )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif // INCLUDED_UCB_SOURCE_CORE_PROVPROX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
