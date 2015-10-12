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

typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XStatusListener > XStatusListenerRef;
typedef std::vector<XStatusListenerRef> XStatusListenerArr_Impl;

class ScDispatchProviderInterceptor : public cppu::WeakImplHelper<
                                        com::sun::star::frame::XDispatchProviderInterceptor,
                                        com::sun::star::lang::XEventListener>,
                                    public SfxListener
{
    ScTabViewShell*     pViewShell;

    /// the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProviderInterception> m_xIntercepted;

    /// chaining
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider> m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider> m_xMasterDispatcher;

    /// own dispatch
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch> m_xMyDispatch;

public:

                            ScDispatchProviderInterceptor(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatchProviderInterceptor();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL
                            queryDispatch( const ::com::sun::star::util::URL& aURL,
                                        const OUString& aTargetFrameName,
                                        sal_Int32 nSearchFlags )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
                                        ::com::sun::star::frame::XDispatch > > SAL_CALL
                            queryDispatches( const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::frame::DispatchDescriptor >& aDescripts )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            /// XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getSlaveDispatchProvider() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setSlaveDispatchProvider( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getMasterDispatchProvider() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setMasterDispatchProvider( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XDispatchProvider >& xNewSupplier )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            /// XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

class ScDispatch : public cppu::WeakImplHelper<
                                    com::sun::star::frame::XDispatch,
                                    com::sun::star::view::XSelectionChangeListener >,
                                public SfxListener
{
    ScTabViewShell*         pViewShell;
    XStatusListenerArr_Impl aDataSourceListeners;
    ScImportParam           aLastImport;
    bool                    bListeningToView;

public:

                            ScDispatch(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatch();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            /// XDispatch
    virtual void SAL_CALL   dispatch( const ::com::sun::star::util::URL& aURL,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aArgs )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addStatusListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::frame::XStatusListener >& xControl,
                                const ::com::sun::star::util::URL& aURL )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   removeStatusListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::frame::XStatusListener >& xControl,
                                const ::com::sun::star::util::URL& aURL )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            /// XSelectionChangeListener
    virtual void SAL_CALL   selectionChanged( const ::com::sun::star::lang::EventObject& aEvent )
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) override;

                            /// XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
