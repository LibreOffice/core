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
#if 1

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/embed/XActionsApproval.hpp>
#include <com/sun/star/embed/Actions.hpp>
#include <cppuhelper/weakref.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/interfacecontainer.h>


class OLockListener;

// the service is implemented as a wrapper to be able to die by refcount
// the disposing mechanics is required for java related scenarios
class OInstanceLocker : public ::cppu::WeakImplHelper3< ::com::sun::star::lang::XComponent,
                                                        ::com::sun::star::lang::XInitialization,
                                                        ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xLockListener;
    OLockListener* m_pLockListener;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners

    sal_Bool m_bDisposed;
    sal_Bool m_bInitialized;

public:
    OInstanceLocker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
    ~OInstanceLocker();

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
            getSupportedServiceNames_static();

    static ::rtl::OUString SAL_CALL getImplementationName_static();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

// XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

// XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};


class OLockListener : public ::cppu::WeakImplHelper2< ::com::sun::star::util::XCloseListener,
                                                    ::com::sun::star::frame::XTerminateListener >
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xInstance;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XActionsApproval > m_xApproval;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XComponent > m_xWrapper;

    sal_Bool m_bDisposed;
    sal_Bool m_bInitialized;

    sal_Int32 m_nMode;

public:
    OLockListener(  const ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XComponent >& xWrapper,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInstance,
                    sal_Int32 nMode,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XActionsApproval > xApproval );

    ~OLockListener();

    sal_Bool Init();
    void Dispose();

// XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

// XCloseListener
    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

// XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
