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

#ifndef INCLUDED_SC_INC_DISPUNO_HXX
#define INCLUDED_SC_INC_DISPUNO_HXX

#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>
#include "global.hxx"

namespace com { namespace sun { namespace star { namespace frame {
    class XDispatchProviderInterception;
} } } }

class ScTabViewShell;

class ScDispatchProviderInterceptor : public cppu::WeakImplHelper<
                                        css::frame::XDispatchProviderInterceptor,
                                        css::lang::XEventListener>,
                                    public SfxListener
{
    ScTabViewShell*     pViewShell;

    /// the component which's dispatches we're intercepting
    css::uno::Reference< css::frame::XDispatchProviderInterception> m_xIntercepted;

    /// chaining
    css::uno::Reference< css::frame::XDispatchProvider> m_xSlaveDispatcher;
    css::uno::Reference< css::frame::XDispatchProvider> m_xMasterDispatcher;

    /// own dispatch
    css::uno::Reference< css::frame::XDispatch> m_xMyDispatch;

public:

                            ScDispatchProviderInterceptor(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatchProviderInterceptor();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL
                            queryDispatch( const css::util::URL& aURL,
                                        const OUString& aTargetFrameName,
                                        sal_Int32 nSearchFlags )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
                            queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            /// XDispatchProviderInterceptor
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
                            getSlaveDispatchProvider() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setSlaveDispatchProvider( const css::uno::Reference<
                                css::frame::XDispatchProvider >& xNewDispatchProvider )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
                            getMasterDispatchProvider() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setMasterDispatchProvider( const css::uno::Reference<
                                css::frame::XDispatchProvider >& xNewSupplier )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XEventListener
    virtual void SAL_CALL   disposing( const css::lang::EventObject& Source )
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScDispatch : public cppu::WeakImplHelper<
                                    css::frame::XDispatch,
                                    css::view::XSelectionChangeListener >,
                                public SfxListener
{
    ScTabViewShell*         pViewShell;
    std::vector< css::uno::Reference< css::frame::XStatusListener > >
                            aDataSourceListeners;
    ScImportParam           aLastImport;
    bool                    bListeningToView;

public:

                            ScDispatch(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatch();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDispatch
    virtual void SAL_CALL   dispatch( const css::util::URL& aURL,
                                const css::uno::Sequence< css::beans::PropertyValue >& aArgs )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl,
                                const css::util::URL& aURL )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl,
                                const css::util::URL& aURL )
                                throw(css::uno::RuntimeException, std::exception) override;

                            /// XSelectionChangeListener
    virtual void SAL_CALL   selectionChanged( const css::lang::EventObject& aEvent )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;

                            /// XEventListener
    virtual void SAL_CALL   disposing( const css::lang::EventObject& Source )
                                throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
