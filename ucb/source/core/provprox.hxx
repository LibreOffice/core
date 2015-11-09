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
    css::uno::Reference< css::lang::XMultiServiceFactory >
                                m_xSMgr;

public:
    explicit UcbContentProviderProxyFactory(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& rxSMgr );
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
    virtual css::uno::Reference< css::ucb::XContentProvider > SAL_CALL
    createContentProvider( const OUString& Service )
        throw( css::uno::RuntimeException, std::exception ) override;
};



// class UcbContentProviderProxy.



class UcbContentProviderProxy :
                public cppu::OWeakObject,
                public css::lang::XTypeProvider,
                public css::lang::XServiceInfo,
                public css::ucb::XContentProviderSupplier,
                public css::ucb::XContentProvider,
                public css::ucb::XParameterizedContentProvider
{
    ::osl::Mutex    m_aMutex;
    OUString m_aService;
    OUString m_aTemplate;
    OUString m_aArguments;
    bool        m_bReplace;
    bool        m_bRegister;

    css::uno::Reference< css::lang::XMultiServiceFactory >
                                m_xSMgr;
    css::uno::Reference< css::ucb::XContentProvider >
                                m_xProvider;
    css::uno::Reference< css::ucb::XContentProvider >
                                m_xTargetProvider;

public:
    UcbContentProviderProxy(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& rxSMgr,
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
    virtual css::uno::Reference<
        css::ucb::XContentProvider > SAL_CALL
    getContentProvider()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XContentProvider
    virtual css::uno::Reference<
        css::ucb::XContent > SAL_CALL
    queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
        throw( css::ucb::IllegalIdentifierException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL
    compareContentIds( const css::uno::Reference< css::ucb::XContentIdentifier >& Id1,
                       const css::uno::Reference< css::ucb::XContentIdentifier >& Id2 )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XParameterizedContentProvider
    virtual css::uno::Reference< css::ucb::XContentProvider > SAL_CALL
    registerInstance( const OUString& Template,
                      const OUString& Arguments,
                      sal_Bool ReplaceExisting )
        throw( css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::ucb::XContentProvider > SAL_CALL
    deregisterInstance( const OUString& Template,
                        const OUString& Arguments )
        throw( css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception ) override;
};

#endif // INCLUDED_UCB_SOURCE_CORE_PROVPROX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
