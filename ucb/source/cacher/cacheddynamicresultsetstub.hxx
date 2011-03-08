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

#ifndef _CACHED_DYNAMIC_RESULTSET_STUB_HXX
#define _CACHED_DYNAMIC_RESULTSET_STUB_HXX

#include <dynamicresultsetwrapper.hxx>
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>

#define CACHED_DRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSetStub"
#define CACHED_DRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetStubFactory"

//=========================================================================

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
                        com::sun::star::lang::XMultiServiceFactory > & xSMgr );

    virtual ~CachedDynamicResultSetStub();


    //-----------------------------------------------------------------
    // XInterface inherited
    //-----------------------------------------------------------------
    XINTERFACE_DECL()
    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()
    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_NOFACTORY_DECL()
};

//=========================================================================

class CachedDynamicResultSetStubFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedDynamicResultSetStubFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >    m_xSMgr;

public:

    CachedDynamicResultSetStubFactory(
        const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & rSMgr);

    virtual ~CachedDynamicResultSetStubFactory();

    //-----------------------------------------------------------------
    // XInterface
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XTypeProvider
    XTYPEPROVIDER_DECL()

    //-----------------------------------------------------------------
    // XServiceInfo
    XSERVICEINFO_DECL()

    //-----------------------------------------------------------------
    // XCachedDynamicResultSetStubFactory

    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSetStub(
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSet > & Source )
                throw( com::sun::star::uno::RuntimeException );


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
                , com::sun::star::uno::RuntimeException
                 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
