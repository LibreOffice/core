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

#ifndef INCLUDED_UCB_SOURCE_CACHER_CACHEDCONTENTRESULTSETSTUB_HXX
#define INCLUDED_UCB_SOURCE_CACHER_CACHEDCONTENTRESULTSETSTUB_HXX

#include <contentresultsetwrapper.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XFetchProvider.hpp>
#include <com/sun/star/ucb/XFetchProviderForContentAccess.hpp>
#include <com/sun/star/ucb/XCachedContentResultSetStubFactory.hpp>

#define CACHED_CRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedContentResultSetStub"
#define CACHED_CRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedContentResultSetStubFactory"



class CachedContentResultSetStub
                : public ContentResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XFetchProvider
                , public com::sun::star::ucb::XFetchProviderForContentAccess
{
private:
    sal_Int32       m_nColumnCount;
    bool        m_bColumnCountCached;

    //members to propagate fetchsize and direction:
    bool        m_bNeedToPropagateFetchSize;
    bool        m_bFirstFetchSizePropagationDone;
    sal_Int32       m_nLastFetchSize;
    bool        m_bLastFetchDirection;
    const OUString     m_aPropertyNameForFetchSize;
    const OUString     m_aPropertyNameForFetchDirection;


    void SAL_CALL
    impl_getCurrentRowContent(
        com::sun::star::uno::Any& rRowContent,
        com::sun::star::uno::Reference<
            com::sun::star::sdbc::XRow > xRow )
        throw ( com::sun::star::sdbc::SQLException
              , com::sun::star::uno::RuntimeException );

    sal_Int32 SAL_CALL
    impl_getColumnCount();

    static void SAL_CALL
    impl_getCurrentContentIdentifierString(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    static void SAL_CALL
    impl_getCurrentContentIdentifier(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    static void SAL_CALL
    impl_getCurrentContent(
            com::sun::star::uno::Any& rAny
            , com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentAccess > xContentAccess )
            throw ( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_propagateFetchSizeAndDirection( sal_Int32 nFetchSize, bool bFetchDirection )
        throw ( com::sun::star::uno::RuntimeException );

public:
    CachedContentResultSetStub( com::sun::star::uno::Reference<
                        com::sun::star::sdbc::XResultSet > xOrigin );

    virtual ~CachedContentResultSetStub();



    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // own inherited

    virtual void SAL_CALL
    impl_propertyChange( const com::sun::star::beans::PropertyChangeEvent& evt )
        throw( com::sun::star::uno::RuntimeException ) override;

    virtual void SAL_CALL
    impl_vetoableChange( const com::sun::star::beans::PropertyChangeEvent& aEvent )
        throw( com::sun::star::beans::PropertyVetoException,
               com::sun::star::uno::RuntimeException ) override;

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

    // XFetchProvider


    virtual com::sun::star::ucb::FetchResult SAL_CALL
    fetch( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    // XFetchProviderForContentAccess

    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContentIdentifierStrings( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContentIdentifiers( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual com::sun::star::ucb::FetchResult SAL_CALL
         fetchContents( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
};



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

    // XCachedContentResultSetStubFactory

    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet > SAL_CALL
    createCachedContentResultSetStub(
                const com::sun::star::uno::Reference<
                    com::sun::star::sdbc::XResultSet > & xSource )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
