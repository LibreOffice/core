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

#ifndef INCLUDED_UCB_SOURCE_CACHER_DYNAMICRESULTSETWRAPPER_HXX
#define INCLUDED_UCB_SOURCE_CACHER_DYNAMICRESULTSETWRAPPER_HXX

#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>



class DynamicResultSetWrapperListener;
class DynamicResultSetWrapper
                : public cppu::OWeakObject
                , public css::ucb::XDynamicResultSet
                , public css::ucb::XSourceInitialization
{
private:
    //management of listeners
    bool                    m_bDisposed; ///Dispose call ready.
    bool                    m_bInDispose;///In dispose call
    osl::Mutex              m_aContainerMutex;
    cppu::OInterfaceContainerHelper*
                            m_pDisposeEventListeners;
protected:
    css::uno::Reference< css::ucb::XDynamicResultSetListener >
                            m_xMyListenerImpl;
    DynamicResultSetWrapperListener*
                            m_pMyListenerImpl;

    css::uno::Reference< css::uno::XComponentContext >
                            m_xContext;

    osl::Mutex              m_aMutex;
    bool                    m_bStatic;
    bool                    m_bGotWelcome;

    //different Interfaces from Origin:
    css::uno::Reference< css::ucb::XDynamicResultSet >
                            m_xSource;
    css::uno::Reference< css::sdbc::XResultSet >
                            m_xSourceResultOne;
    css::uno::Reference< css::sdbc::XResultSet >
                            m_xSourceResultTwo;

    css::uno::Reference< css::sdbc::XResultSet >
                            m_xMyResultOne;
    css::uno::Reference< css::sdbc::XResultSet >
                            m_xMyResultTwo;

    css::uno::Reference< css::ucb::XDynamicResultSetListener >
                            m_xListener;

    osl::Condition          m_aSourceSet;
    osl::Condition          m_aListenerSet;

protected:
    void SAL_CALL impl_init();
    void SAL_CALL impl_deinit();
    void SAL_CALL
    impl_EnsureNotDisposed()
        throw( css::lang::DisposedException,
               css::uno::RuntimeException );

    virtual void SAL_CALL
    impl_InitResultSetOne( const css::uno::Reference<
                css::sdbc::XResultSet >& xResultSet );
    virtual void SAL_CALL
    impl_InitResultSetTwo( const css::uno::Reference<
                css::sdbc::XResultSet >& xResultSet );

public:

    DynamicResultSetWrapper(
        css::uno::Reference< css::ucb::XDynamicResultSet > xOrigin
        , const css::uno::Reference< css::uno::XComponentContext > & rxContext );

    virtual ~DynamicResultSetWrapper();


    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;


    // XDynamicResultSet
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
    getStaticResultSet()
        throw( css::ucb::ListenerAlreadySetException
        , css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    setListener( const css::uno::Reference< css::ucb::XDynamicResultSetListener > & Listener )
        throw( css::ucb::ListenerAlreadySetException
            , css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    connectToCache( const css::uno::Reference< css::ucb::XDynamicResultSet > & xCache )
        throw( css::ucb::ListenerAlreadySetException
            , css::ucb::AlreadyInitializedException
            , css::ucb::ServiceNotFoundException
            , css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
    getCapabilities() throw( css::uno::RuntimeException, std::exception ) override;


    // XComponent ( base of XDynamicResultSet )
    virtual void SAL_CALL
    dispose() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
            throw( css::uno::RuntimeException, std::exception ) override;


    // XSourceInitialization

    virtual void SAL_CALL
    setSource(  const css::uno::Reference< css::uno::XInterface > & Source )
         throw( css::ucb::AlreadyInitializedException
                , css::uno::RuntimeException, std::exception ) override;


    // own methods:
    virtual void SAL_CALL
    impl_disposing( const css::lang::EventObject& Source )
        throw( css::uno::RuntimeException );

    void SAL_CALL
    impl_notify( const css::ucb::ListEvent& Changes )
        throw( css::uno::RuntimeException );
};



class DynamicResultSetWrapperListener
                : public cppu::OWeakObject
                , public css::ucb::XDynamicResultSetListener
{
protected:
    DynamicResultSetWrapper*    m_pOwner;
    osl::Mutex                  m_aMutex;

public:
    DynamicResultSetWrapperListener( DynamicResultSetWrapper* pOwner );

    virtual ~DynamicResultSetWrapperListener();


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XEventListener ( base of XDynamicResultSetListener )

    virtual void SAL_CALL
    disposing( const css::lang::EventObject& Source )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XDynamicResultSetListener
    virtual void SAL_CALL
    notify( const css::ucb::ListEvent& Changes )
        throw( css::uno::RuntimeException, std::exception ) override;


    // own methods:
    void SAL_CALL impl_OwnerDies();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
