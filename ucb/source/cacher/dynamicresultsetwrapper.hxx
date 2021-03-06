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

#pragma once

#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>

#include <memory>

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
    std::unique_ptr<comphelper::OInterfaceContainerHelper2>
                            m_pDisposeEventListeners;
protected:
    rtl::Reference<DynamicResultSetWrapperListener>
                            m_xMyListenerImpl;

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
    void impl_init();
    void impl_deinit();
    /// @throws css::lang::DisposedException
    /// @throws css::uno::RuntimeException
    void
    impl_EnsureNotDisposed();

    virtual void
    impl_InitResultSetOne( const css::uno::Reference<
                css::sdbc::XResultSet >& xResultSet );
    virtual void
    impl_InitResultSetTwo( const css::uno::Reference<
                css::sdbc::XResultSet >& xResultSet );

public:

    DynamicResultSetWrapper(
        css::uno::Reference< css::ucb::XDynamicResultSet > const & xOrigin
        , const css::uno::Reference< css::uno::XComponentContext > & rxContext );

    virtual ~DynamicResultSetWrapper() override;


    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type & rType ) override;


    // XDynamicResultSet
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
    getStaticResultSet() override;

    virtual void SAL_CALL
    setListener( const css::uno::Reference< css::ucb::XDynamicResultSetListener > & Listener ) override;

    virtual void SAL_CALL
    connectToCache( const css::uno::Reference< css::ucb::XDynamicResultSet > & xCache ) override;

    virtual sal_Int16 SAL_CALL
    getCapabilities() override;


    // XComponent ( base of XDynamicResultSet )
    virtual void SAL_CALL
    dispose() override;

    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) override;


    // XSourceInitialization

    virtual void SAL_CALL
    setSource(  const css::uno::Reference< css::uno::XInterface > & Source ) override;


    // own methods:
    /// @throws css::uno::RuntimeException
    virtual void
    impl_disposing( const css::lang::EventObject& Source );

    /// @throws css::uno::RuntimeException
    void
    impl_notify( const css::ucb::ListEvent& Changes );
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

    virtual ~DynamicResultSetWrapperListener() override;


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XEventListener ( base of XDynamicResultSetListener )

    virtual void SAL_CALL
    disposing( const css::lang::EventObject& Source ) override;

    // XDynamicResultSetListener
    virtual void SAL_CALL
    notify( const css::ucb::ListEvent& Changes ) override;


    // own methods:
    void impl_OwnerDies();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
