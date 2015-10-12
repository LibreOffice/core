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

#ifndef INCLUDED_UCB_SOURCE_SORTER_SORTDYNRES_HXX
#define INCLUDED_UCB_SOURCE_SORTER_SORTDYNRES_HXX

#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include "sortresult.hxx"


namespace cppu {
    class OInterfaceContainerHelper;
}

#define DYNAMIC_RESULTSET_SERVICE_NAME  "com.sun.star.ucb.SortedDynamicResultSet"
#define DYNAMIC_RESULTSET_FACTORY_NAME  "com.sun.star.ucb.SortedDynamicResultSetFactory"

class SortedDynamicResultSetListener;

class SortedDynamicResultSet: public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::ucb::XDynamicResultSet >
{
    cppu::OInterfaceContainerHelper *mpDisposeEventListeners;

    css::uno::Reference < css::ucb::XDynamicResultSetListener > mxListener;
    css::uno::Reference < css::ucb::XDynamicResultSetListener > mxOwnListener;

    css::uno::Reference < css::sdbc::XResultSet >            mxOne;
    css::uno::Reference < css::sdbc::XResultSet >            mxTwo;
    css::uno::Reference < css::ucb::XDynamicResultSet >      mxOriginal;
    css::uno::Sequence  < css::ucb::NumberedSortingInfo >    maOptions;
    css::uno::Reference < css::ucb::XAnyCompareFactory >     mxCompFac;
    css::uno::Reference < css::uno::XComponentContext >      m_xContext;

    SortedResultSet*                    mpOne;
    SortedResultSet*                    mpTwo;
    SortedDynamicResultSetListener*     mpOwnListener;

    EventList                           maActions;
    osl::Mutex                          maMutex;
    bool                            mbGotWelcome:1;
    bool                            mbUseOne:1;
    bool                            mbStatic:1;

private:
    void                SendNotify();

public:
    SortedDynamicResultSet( const css::uno::Reference < css::ucb::XDynamicResultSet >    &xOriginal,
                            const css::uno::Sequence  < css::ucb::NumberedSortingInfo >  &aOptions,
                            const css::uno::Reference < css::ucb::XAnyCompareFactory >   &xCompFac,
                            const css::uno::Reference < css::uno::XComponentContext >    &rxContext );

    virtual ~SortedDynamicResultSet();


    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;
    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XComponent

    virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;


    // XDynamicResultSet

    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getStaticResultSet()
        throw( css::ucb::ListenerAlreadySetException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    setListener( const css::uno::Reference< css::ucb::XDynamicResultSetListener >& Listener )
        throw( css::ucb::ListenerAlreadySetException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    connectToCache( const css::uno::Reference< css::ucb::XDynamicResultSet > & xCache )
        throw( css::ucb::ListenerAlreadySetException,
               css::ucb::AlreadyInitializedException,
               css::ucb::ServiceNotFoundException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
    getCapabilities()
        throw( css::uno::RuntimeException, std::exception ) override;


    // own methods:

    void impl_disposing( const css::lang::EventObject& Source )
        throw( css::uno::RuntimeException );

    void impl_notify( const css::ucb::ListEvent& Changes )
        throw( css::uno::RuntimeException );
};

class SortedDynamicResultSetListener: public cppu::WeakImplHelper <
    css::ucb::XDynamicResultSetListener >
{
    SortedDynamicResultSet  *mpOwner;
    osl::Mutex              maMutex;

public:
     SortedDynamicResultSetListener( SortedDynamicResultSet *mOwner );
    virtual ~SortedDynamicResultSetListener();

    // XEventListener ( base of XDynamicResultSetListener )

    virtual void SAL_CALL
    disposing( const css::lang::EventObject& Source )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XDynamicResultSetListener

    virtual void SAL_CALL notify( const css::ucb::ListEvent& Changes )
        throw( css::uno::RuntimeException, std::exception ) override;

    // own methods:

    void SAL_CALL impl_OwnerDies();
};



class SortedDynamicResultSetFactory: public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::ucb::XSortedDynamicResultSetFactory >
{

    css::uno::Reference< css::uno::XComponentContext >   m_xContext;

public:
    SortedDynamicResultSetFactory(
        const css::uno::Reference< css::uno::XComponentContext > & rxContext);

    virtual ~SortedDynamicResultSetFactory();


    // XServiceInfo

    static css::uno::Reference< css::lang::XSingleServiceFactory > createServiceFactory(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XSortedDynamicResultSetFactory

    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;
    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    virtual css::uno::Reference< css::ucb::XDynamicResultSet > SAL_CALL
    createSortedDynamicResultSet(
                const css::uno::Reference< css::ucb::XDynamicResultSet > & Source,
                const css::uno::Sequence< css::ucb::NumberedSortingInfo > & Info,
                const css::uno::Reference< css::ucb::XAnyCompareFactory > & CompareFactory )
        throw( css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
