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

#ifndef _CACHED_DYNAMIC_RESULTSET_STUB_HXX
#define _CACHED_DYNAMIC_RESULTSET_STUB_HXX

#include <dynamicresultsetwrapper.hxx>
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>

#define CACHED_DRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSetStub"
#define CACHED_DRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetStubFactory"



class CachedDynamicResultSetStub
                : public DynamicResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
{
protected:
    virtual void SAL_CALL
    impl_InitResultSetOne( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );
    virtual void SAL_CALL
    impl_InitResultSetTwo( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );

public:
    CachedDynamicResultSetStub( com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > xOrigin
        , const com::sun::star::uno::Reference<
                        com::sun::star::uno::XComponentContext > & rxContext );

    virtual ~CachedDynamicResultSetStub();



    // XInterface inherited

    XINTERFACE_DECL()

    // XTypeProvider

    XTYPEPROVIDER_DECL()

    // XServiceInfo

    XSERVICEINFO_NOFACTORY_DECL()
};



class CachedDynamicResultSetStubFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedDynamicResultSetStubFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >    m_xContext;

public:

    CachedDynamicResultSetStubFactory(
        const com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext > & rxContext);

    virtual ~CachedDynamicResultSetStubFactory();


    // XInterface
    XINTERFACE_DECL()


    // XTypeProvider
    XTYPEPROVIDER_DECL()


    // XServiceInfo
    XSERVICEINFO_DECL()


    // XCachedDynamicResultSetStubFactory

    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSetStub(
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSet > & Source )
                throw( com::sun::star::uno::RuntimeException, std::exception );


    virtual void SAL_CALL connectToCache(
                  const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > & Source
                , const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > & TargetCache
                , const com::sun::star::uno::Sequence<
                        com::sun::star::ucb::NumberedSortingInfo > & SortingInfo
                , const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XAnyCompareFactory > & CompareFactory
                )
                throw (
                  com::sun::star::ucb::ListenerAlreadySetException
                , com::sun::star::ucb::AlreadyInitializedException
                , com::sun::star::uno::RuntimeException, std::exception
                 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
