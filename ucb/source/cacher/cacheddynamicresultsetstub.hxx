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

#ifndef INCLUDED_UCB_SOURCE_CACHER_CACHEDDYNAMICRESULTSETSTUB_HXX
#define INCLUDED_UCB_SOURCE_CACHER_CACHEDDYNAMICRESULTSETSTUB_HXX

#include <dynamicresultsetwrapper.hxx>
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>

#define CACHED_DRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSetStub"
#define CACHED_DRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetStubFactory"



class CachedDynamicResultSetStub
                : public DynamicResultSetWrapper
                , public css::lang::XTypeProvider
                , public css::lang::XServiceInfo
{
protected:
    virtual void SAL_CALL
    impl_InitResultSetOne( const css::uno::Reference< css::sdbc::XResultSet >& xResultSet ) override;
    virtual void SAL_CALL
    impl_InitResultSetTwo( const css::uno::Reference< css::sdbc::XResultSet >& xResultSet ) override;

public:
    CachedDynamicResultSetStub( css::uno::Reference< css::ucb::XDynamicResultSet > xOrigin
        , const css::uno::Reference< css::uno::XComponentContext > & rxContext );

    virtual ~CachedDynamicResultSetStub();


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

};



class CachedDynamicResultSetStubFactory
                : public cppu::OWeakObject
                , public css::lang::XTypeProvider
                , public css::lang::XServiceInfo
                , public css::ucb::XCachedDynamicResultSetStubFactory
{
protected:
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;

public:

    CachedDynamicResultSetStubFactory(
        const css::uno::Reference< css::uno::XComponentContext > & rxContext);

    virtual ~CachedDynamicResultSetStubFactory();

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

    // XCachedDynamicResultSetStubFactory

    virtual css::uno::Reference< css::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSetStub(
                const css::uno::Reference< css::ucb::XDynamicResultSet > & Source )
                throw( css::uno::RuntimeException, std::exception ) override;


    virtual void SAL_CALL connectToCache(
                  const css::uno::Reference< css::ucb::XDynamicResultSet > & Source
                , const css::uno::Reference< css::ucb::XDynamicResultSet > & TargetCache
                , const css::uno::Sequence< css::ucb::NumberedSortingInfo > & SortingInfo
                , const css::uno::Reference< css::ucb::XAnyCompareFactory > & CompareFactory
                )
                throw (
                  css::ucb::ListenerAlreadySetException
                , css::ucb::AlreadyInitializedException
                , css::uno::RuntimeException, std::exception
                 ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
