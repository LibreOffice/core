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

#ifndef _CACHED_CONTENT_RESULTSET_STUB_HXX
#define _CACHED_CONTENT_RESULTSET_STUB_HXX

#include <contentresultsetwrapper.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XFetchProvider.hpp>
#include <com/sun/star/ucb/XFetchProviderForContentAccess.hpp>
#include <com/sun/star/ucb/XCachedContentResultSetStubFactory.hpp>

#define CACHED_CRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedContentResultSetStub"
#define CACHED_CRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedContentResultSetStubFactory"

//=========================================================================

class CachedContentResultSetStub
                : public ContentResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XFetchProvider
                , public com::sun::star::ucb::XFetchProviderForContentAccess
{
private:
    sal_Int32       m_nColumnCount;
    sal_Bool        m_bColumnCountCached;

    //members to propagate fetchsize and direction:
    sal_Bool        m_bNeedToPropagateFetchSize;
    sal_Bool        m_bFirstFetchSizePropagationDone;
    sal_Int32       m_nLastFetchSize;
    sal_Bool        m_bLastFetchDirection;
    const rtl::OUString     m_aPropertyNameForFetchSize;
    const rtl::OUString     m_aPropertyNameForFetchDirection;


    void SAL_CALL
    impl_getCurrentRowContent(
        com::sun::star::uno::Any& rRowContent,
        com::sun::star::uno::Reference<
            com::sun::star::sdbc::XRow > xRow )
        throw ( com::sun::star::sdbc::SQLException
              , com::sun::star::uno::RuntimeException );

    sal_Int32 SAL_CALL
    impl_getColumnCount();

    void SAL_CALL
    impl_getCurrentContentIdentifierString(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_getCurrentContentIdentifier(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_getCurrentContent(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_propagateFetchSizeAndDirection( sal_Int32 nFetchSize, sal_Bool bFetchDirection )
        throw ( com::sun::star::uno::RuntimeException );

public:
    CachedContentResultSetStub( com::sun::star::uno::Reference<
                        com::sun::star::sdbc::XResultSet > xOrigin );

    virtual ~CachedContentResultSetStub();


    //-----------------------------------------------------------------
    // XInterface inherited
    //-----------------------------------------------------------------
    XINTERFACE_DECL()
    //-----------------------------------------------------------------
    // own inherited
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    impl_propertyChange( const com::sun::star::beans::PropertyChangeEvent& evt )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_vetoableChange( const com::sun::star::beans::PropertyChangeEvent& aEvent )
        throw( com::sun::star::beans::PropertyVetoException,
               com::sun::star::uno::RuntimeException );
    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()
    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_NOFACTORY_DECL()

    //-----------------------------------------------------------------
    // XFetchProvider
    //-----------------------------------------------------------------

    virtual com::sun::star::ucb::FetchResult SAL_CALL
    fetch( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XFetchProviderForContentAccess
    //-----------------------------------------------------------------
    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContentIdentifierStrings( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContentIdentifiers( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContents( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException );
};

//=========================================================================

class CachedContentResultSetStubFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedContentResultSetStubFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >    m_xSMgr;

public:

    CachedContentResultSetStubFactory(
        const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & rSMgr);

    virtual ~CachedContentResultSetStubFactory();

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
    // XCachedContentResultSetStubFactory

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet > SAL_CALL
    createCachedContentResultSetStub(
                const com::sun::star::uno::Reference<
                    com::sun::star::sdbc::XResultSet > & xSource )
            throw( com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
