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
                , public css::lang::XTypeProvider
                , public css::lang::XServiceInfo
                , public css::ucb::XFetchProvider
                , public css::ucb::XFetchProviderForContentAccess
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
        css::uno::Any& rRowContent,
        css::uno::Reference< css::sdbc::XRow > xRow )
        throw ( css::sdbc::SQLException
              , css::uno::RuntimeException );

    sal_Int32 SAL_CALL
    impl_getColumnCount();

    static void SAL_CALL
    impl_getCurrentContentIdentifierString(
            css::uno::Any& rAny
            , css::uno::Reference< css::ucb::XContentAccess > xContentAccess )
            throw ( css::uno::RuntimeException );

    static void SAL_CALL
    impl_getCurrentContentIdentifier(
            css::uno::Any& rAny
            , css::uno::Reference< css::ucb::XContentAccess > xContentAccess )
            throw ( css::uno::RuntimeException );

    static void SAL_CALL
    impl_getCurrentContent(
            css::uno::Any& rAny
            , css::uno::Reference< css::ucb::XContentAccess > xContentAccess )
            throw ( css::uno::RuntimeException );

    void SAL_CALL
    impl_propagateFetchSizeAndDirection( sal_Int32 nFetchSize, bool bFetchDirection )
        throw ( css::uno::RuntimeException );

public:
    CachedContentResultSetStub( css::uno::Reference< css::sdbc::XResultSet > xOrigin );

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
    impl_propertyChange( const css::beans::PropertyChangeEvent& evt )
        throw( css::uno::RuntimeException ) override;

    virtual void SAL_CALL
    impl_vetoableChange( const css::beans::PropertyChangeEvent& aEvent )
        throw( css::beans::PropertyVetoException,
               css::uno::RuntimeException ) override;

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


    virtual css::ucb::FetchResult SAL_CALL
    fetch( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( css::uno::RuntimeException, std::exception ) override;


    // XFetchProviderForContentAccess

    virtual css::ucb::FetchResult SAL_CALL
         fetchContentIdentifierStrings( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::ucb::FetchResult SAL_CALL
         fetchContentIdentifiers( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::ucb::FetchResult SAL_CALL
         fetchContents( sal_Int32 nRowStartPosition
        , sal_Int32 nRowCount, sal_Bool bDirection )
        throw( css::uno::RuntimeException, std::exception ) override;
};



class CachedContentResultSetStubFactory
                : public cppu::OWeakObject
                , public css::lang::XTypeProvider
                , public css::lang::XServiceInfo
                , public css::ucb::XCachedContentResultSetStubFactory
{
protected:
    css::uno::Reference< css::lang::XMultiServiceFactory >    m_xSMgr;

public:

    CachedContentResultSetStubFactory(
        const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr);

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

    // XCachedContentResultSetStubFactory

    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
    createCachedContentResultSetStub(
                const css::uno::Reference< css::sdbc::XResultSet > & xSource )
            throw( css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
