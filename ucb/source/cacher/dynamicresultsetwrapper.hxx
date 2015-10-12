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
                , public com::sun::star::ucb::XDynamicResultSet
                , public com::sun::star::ucb::XSourceInitialization
{
private:
    //management of listeners
    bool                m_bDisposed; ///Dispose call ready.
    bool                m_bInDispose;///In dispose call
    osl::Mutex              m_aContainerMutex;
    cppu::OInterfaceContainerHelper*
                            m_pDisposeEventListeners;
protected:
    com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSetListener >
                            m_xMyListenerImpl;
    DynamicResultSetWrapperListener*
                            m_pMyListenerImpl;

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
                            m_xContext;

    osl::Mutex              m_aMutex;
    bool                m_bStatic;
    bool                m_bGotWelcome;

    //different Interfaces from Origin:
    com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSet >
                            m_xSource;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xSourceResultOne;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xSourceResultTwo;
    //com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
    //                      m_xSourceResultCurrent;
    //sal_Bool              m_bUseOne;

    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xMyResultOne;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xMyResultTwo;

    com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSetListener >
                            m_xListener;

    osl::Condition          m_aSourceSet;
    osl::Condition          m_aListenerSet;

protected:
    void SAL_CALL impl_init();
    void SAL_CALL impl_deinit();
    void SAL_CALL
    impl_EnsureNotDisposed()
        throw( com::sun::star::lang::DisposedException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_InitResultSetOne( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );
    virtual void SAL_CALL
    impl_InitResultSetTwo( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );

public:

    DynamicResultSetWrapper(
        com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > xOrigin
        , const com::sun::star::uno::Reference<
                        com::sun::star::uno::XComponentContext > & rxContext );

    virtual ~DynamicResultSetWrapper();


    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    // XDynamicResultSet
    virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet > SAL_CALL
    getStaticResultSet()
        throw( com::sun::star::ucb::ListenerAlreadySetException
        , com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    setListener( const com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSetListener > & Listener )
        throw( com::sun::star::ucb::ListenerAlreadySetException
            , com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    connectToCache( const com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSet > & xCache )
        throw( com::sun::star::ucb::ListenerAlreadySetException
            , com::sun::star::ucb::AlreadyInitializedException
            , com::sun::star::ucb::ServiceNotFoundException
            , com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
    getCapabilities() throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    // XComponent ( base of XDynamicResultSet )
    virtual void SAL_CALL
    dispose() throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
        com::sun::star::lang::XEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
        com::sun::star::lang::XEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    // XSourceInitialization

    virtual void SAL_CALL
    setSource(  const com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > & Source )
         throw( com::sun::star::ucb::AlreadyInitializedException
                , com::sun::star::uno::RuntimeException, std::exception ) override;


    // own methods:
    virtual void SAL_CALL
    impl_disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    impl_notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( com::sun::star::uno::RuntimeException );
};



class DynamicResultSetWrapperListener
                : public cppu::OWeakObject
                , public com::sun::star::ucb::XDynamicResultSetListener
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
    disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XDynamicResultSetListener
    virtual void SAL_CALL
    notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;


    // own methods:
    void SAL_CALL impl_OwnerDies();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
