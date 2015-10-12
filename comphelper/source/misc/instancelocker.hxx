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

#ifndef INCLUDED_COMPHELPER_SOURCE_MISC_INSTANCELOCKER_HXX
#define INCLUDED_COMPHELPER_SOURCE_MISC_INSTANCELOCKER_HXX

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
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>


class OLockListener;

// the service is implemented as a wrapper to be able to die by refcount
// the disposing mechanics is required for java related scenarios
class OInstanceLocker : public ::cppu::WeakImplHelper< css::lang::XComponent,
                                                       css::lang::XInitialization,
                                                       css::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    css::uno::Reference< css::uno::XInterface > m_xLockListener;
    OLockListener* m_pLockListener;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners

    bool m_bDisposed;
    bool m_bInitialized;

public:
    explicit OInstanceLocker( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~OInstanceLocker();

    static css::uno::Sequence< OUString > SAL_CALL
            getSupportedServiceNames_static();

    static OUString SAL_CALL getImplementationName_static();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        Create(const css::uno::Reference< css::uno::XComponentContext >& rxContext );

// XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

// XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

// XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

};


class OLockListener : public ::cppu::WeakImplHelper< css::util::XCloseListener,
                                                     css::frame::XTerminateListener >
{
    ::osl::Mutex m_aMutex;
    css::uno::Reference< css::uno::XInterface > m_xInstance;
    css::uno::Reference< css::embed::XActionsApproval > m_xApproval;

    css::uno::WeakReference< css::lang::XComponent > m_xWrapper;

    bool m_bDisposed;
    bool m_bInitialized;

    sal_Int32 m_nMode;

public:
    OLockListener(  const css::uno::WeakReference< css::lang::XComponent >& xWrapper,
                    const css::uno::Reference< css::uno::XInterface >& xInstance,
                    sal_Int32 nMode,
                    const css::uno::Reference< css::embed::XActionsApproval >& rApproval );

    virtual ~OLockListener();

    bool Init();
    void Dispose();

// XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

// XCloseListener
    virtual void SAL_CALL queryClosing( const css::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (css::util::CloseVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyClosing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

// XTerminateListener
    virtual void SAL_CALL queryTermination( const css::lang::EventObject& Event ) throw (css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyTermination( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
