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

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/weakref.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <memory>

namespace com::sun::star::embed { class XActionsApproval; }
namespace comphelper { class OInterfaceContainerHelper2; }


class OLockListener;

// the service is implemented as a wrapper to be able to die by refcount
// the disposing mechanics is required for java related scenarios
class OInstanceLocker : public ::cppu::WeakImplHelper< css::lang::XComponent,
                                                       css::lang::XInitialization,
                                                       css::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;

    rtl::Reference< OLockListener > m_xLockListener;

    std::unique_ptr<::comphelper::OInterfaceContainerHelper2> m_pListenersContainer; // list of listeners

    bool m_bDisposed;
    bool m_bInitialized;

public:
    explicit OInstanceLocker();
    virtual ~OInstanceLocker() override;

// XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

// XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

// XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

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

    virtual ~OLockListener() override;

    void Init();
    void Dispose();

// XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

// XCloseListener
    virtual void SAL_CALL queryClosing( const css::lang::EventObject& Source, sal_Bool GetsOwnership ) override;
    virtual void SAL_CALL notifyClosing( const css::lang::EventObject& Source ) override;

// XTerminateListener
    virtual void SAL_CALL queryTermination( const css::lang::EventObject& Event ) override;
    virtual void SAL_CALL notifyTermination( const css::lang::EventObject& Event ) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
