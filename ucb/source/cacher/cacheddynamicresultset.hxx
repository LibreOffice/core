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

#include <dynamicresultsetwrapper.hxx>
#include <com/sun/star/ucb/XContentIdentifierMapping.hpp>
#include <com/sun/star/ucb/XCachedDynamicResultSetFactory.hpp>

#define CACHED_DRS_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSet"
#define CACHED_DRS_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetFactory"



class CachedDynamicResultSet
                : public DynamicResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
{
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifierMapping >
                            m_xContentIdentifierMapping;

protected:
    virtual void SAL_CALL
    impl_InitResultSetOne( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet ) override;
    virtual void SAL_CALL
    impl_InitResultSetTwo( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet ) override;

public:
    CachedDynamicResultSet( com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > xOrigin
        , const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifierMapping > & xContentMapping
        , const com::sun::star::uno::Reference<
                        com::sun::star::uno::XComponentContext > & xContext );

    virtual ~CachedDynamicResultSet();


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


    // own methods ( inherited )

    virtual void SAL_CALL
    impl_disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException ) override;
};



class CachedDynamicResultSetFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedDynamicResultSetFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >    m_xContext;

public:

    CachedDynamicResultSetFactory(
        const com::sun::star::uno::Reference<
           com::sun::star::uno::XComponentContext > & xContext);

    virtual ~CachedDynamicResultSetFactory();


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
    virtual css::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
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

    // XCachedDynamicResultSetFactory

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSet(
                  const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSet > &
                        SourceStub
                , const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifierMapping > &
                        ContentIdentifierMapping
                )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
