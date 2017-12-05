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

#ifndef INCLUDED_UCB_SOURCE_CACHER_CACHEDDYNAMICRESULTSET_HXX
#define INCLUDED_UCB_SOURCE_CACHER_CACHEDDYNAMICRESULTSET_HXX

#include "dynamicresultsetwrapper.hxx"
#include <com/sun/star/ucb/XContentIdentifierMapping.hpp>
#include <com/sun/star/ucb/XCachedDynamicResultSetFactory.hpp>

#define CACHED_DRS_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSet"
#define CACHED_DRS_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetFactory"


class CachedDynamicResultSet
                : public DynamicResultSetWrapper
                , public css::lang::XTypeProvider
                , public css::lang::XServiceInfo
{
    css::uno::Reference< css::ucb::XContentIdentifierMapping >
                            m_xContentIdentifierMapping;

protected:
    virtual void
    impl_InitResultSetOne( const css::uno::Reference< css::sdbc::XResultSet >& xResultSet ) override;
    virtual void
    impl_InitResultSetTwo( const css::uno::Reference< css::sdbc::XResultSet >& xResultSet ) override;

public:
    CachedDynamicResultSet( css::uno::Reference< css::ucb::XDynamicResultSet > const & xOrigin
        , const css::uno::Reference< css::ucb::XContentIdentifierMapping > & xContentMapping
        , const css::uno::Reference< css::uno::XComponentContext > & xContext );

    virtual ~CachedDynamicResultSet() override;


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // own methods ( inherited )

    virtual void
    impl_disposing( const css::lang::EventObject& Source ) override;
};


class CachedDynamicResultSetFactory final
                : public cppu::OWeakObject
                , public css::lang::XTypeProvider
                , public css::lang::XServiceInfo
                , public css::ucb::XCachedDynamicResultSetFactory
{
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;

public:

    CachedDynamicResultSetFactory(
        const css::uno::Reference< css::uno::XComponentContext > & xContext);

    virtual ~CachedDynamicResultSetFactory() override;


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;


    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference<
                          css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XCachedDynamicResultSetFactory

    virtual css::uno::Reference< css::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSet(
                  const css::uno::Reference< css::ucb::XDynamicResultSet > &          SourceStub
                , const css::uno::Reference< css::ucb::XContentIdentifierMapping > &  ContentIdentifierMapping
                ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
