/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>
#include "global.hxx"

namespace com::sun::star::frame { class XDispatchProviderInterception; }
class ScTabViewShell;
class ScDispatch;

class ScDispatchProviderInterceptor final : public cppu::WeakImplHelper<
                                        css::frame::XDispatchProviderInterceptor,
                                        css::lang::XEventListener>,
                                    public SfxListener
{
    ScTabViewShell*     pViewShell;

    /// the component which's dispatches we're intercepting
    cpo::uno::Reference< css::frame::XDispatchProviderInterception> m_xIntercepted;

    /// chaining
    cpo::uno::Reference< css::frame::XDispatchProvider> m_xSlaveDispatcher;
    cpo::uno::Reference< css::frame::XDispatchProvider> m_xMasterDispatcher;

    /// own dispatch
    rtl::Reference<ScDispatch> m_xMyDispatch;

public:

                            ScDispatchProviderInterceptor(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatchProviderInterceptor() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDispatchProvider
    virtual cpo::uno::Reference< css::frame::XDispatch >
                            queryDispatch( const css::util::URL& aURL,
                                        const OUString& aTargetFrameName,
                                        sal_Int32 nSearchFlags ) override;
    virtual cpo::uno::Sequence< cpo::uno::Reference< css::frame::XDispatch > >
                            queryDispatches( const cpo::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts ) override;

                            /// XDispatchProviderInterceptor
    virtual cpo::uno::Reference< css::frame::XDispatchProvider >
                            getSlaveDispatchProvider() override;
    virtual void   setSlaveDispatchProvider( const cpo::uno::Reference<
                                css::frame::XDispatchProvider >& xNewDispatchProvider ) override;
    virtual cpo::uno::Reference< css::frame::XDispatchProvider >
                            getMasterDispatchProvider() override;
    virtual void   setMasterDispatchProvider( const cpo::uno::Reference<
                                css::frame::XDispatchProvider >& xNewSupplier ) override;

                            /// XEventListener
    virtual void   disposing( const css::lang::EventObject& Source ) override;
};

class ScDispatch final : public cppu::WeakImplHelper<
                                    css::frame::XDispatch,
                                    css::view::XSelectionChangeListener >,
                                public SfxListener
{
    ScTabViewShell*         pViewShell;
    std::vector< cpo::uno::Reference< css::frame::XStatusListener > >
                            aDataSourceListeners;
    ScImportParam           aLastImport;
    bool                    bListeningToView;

public:

                            ScDispatch(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatch() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDispatch
    virtual void   dispatch( const css::util::URL& aURL,
                                const cpo::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual void   addStatusListener( const cpo::uno::Reference< css::frame::XStatusListener >& xControl,
                                const css::util::URL& aURL ) override;
    virtual void   removeStatusListener( const cpo::uno::Reference< css::frame::XStatusListener >& xControl,
                                const css::util::URL& aURL ) override;

                            /// XSelectionChangeListener
    virtual void   selectionChanged( const css::lang::EventObject& aEvent ) override;

                            /// XEventListener
    virtual void   disposing( const css::lang::EventObject& Source ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
