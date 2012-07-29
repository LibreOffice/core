/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_DISPUNO_HXX
#define SC_DISPUNO_HXX

#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/implbase2.hxx>
#include <svl/lstner.hxx>
#include "global.hxx"       // ScImportParam
#include <boost/ptr_container/ptr_vector.hpp>


namespace com { namespace sun { namespace star { namespace frame {
    class XDispatchProviderInterception;
} } } }

class ScTabViewShell;


typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XStatusListener > XStatusListenerRef;
typedef boost::ptr_vector<XStatusListenerRef> XStatusListenerArr_Impl;


class ScDispatchProviderInterceptor : public cppu::WeakImplHelper2<
                                        com::sun::star::frame::XDispatchProviderInterceptor,
                                        com::sun::star::lang::XEventListener>,
                                    public SfxListener
{
    ScTabViewShell*     pViewShell;

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProviderInterception> m_xIntercepted;

    // chaining
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider> m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider> m_xMasterDispatcher;

    // own dispatch
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatch> m_xMyDispatch;

public:

                            ScDispatchProviderInterceptor(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatchProviderInterceptor();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL
                            queryDispatch( const ::com::sun::star::util::URL& aURL,
                                        const ::rtl::OUString& aTargetFrameName,
                                        sal_Int32 nSearchFlags )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
                                        ::com::sun::star::frame::XDispatch > > SAL_CALL
                            queryDispatches( const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::frame::DispatchDescriptor >& aDescripts )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getSlaveDispatchProvider() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSlaveDispatchProvider( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getMasterDispatchProvider() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setMasterDispatchProvider( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XDispatchProvider >& xNewSupplier )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScDispatch : public cppu::WeakImplHelper2<
                                    com::sun::star::frame::XDispatch,
                                    com::sun::star::view::XSelectionChangeListener >,
                                public SfxListener
{
    ScTabViewShell*         pViewShell;
    XStatusListenerArr_Impl aDataSourceListeners;
    ScImportParam           aLastImport;
    sal_Bool                bListeningToView;

public:

                            ScDispatch(ScTabViewShell* pViewSh);
    virtual                 ~ScDispatch();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XDispatch
    virtual void SAL_CALL   dispatch( const ::com::sun::star::util::URL& aURL,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aArgs )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addStatusListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::frame::XStatusListener >& xControl,
                                const ::com::sun::star::util::URL& aURL )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeStatusListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::frame::XStatusListener >& xControl,
                                const ::com::sun::star::util::URL& aURL )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSelectionChangeListener
    virtual void SAL_CALL   selectionChanged( const ::com::sun::star::lang::EventObject& aEvent )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source )
                                throw (::com::sun::star::uno::RuntimeException);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
