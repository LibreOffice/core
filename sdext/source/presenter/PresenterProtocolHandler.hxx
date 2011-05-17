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

#ifndef SDEXT_PRESENTER_PRESENTER_PROTOCOL_HANDLER_HXX
#define SDEXT_PRESENTER_PRESENTER_PROTOCOL_HANDLER_HXX

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <boost/unordered_map.hpp>
#include <rtl/ref.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {


namespace {
    typedef ::cppu::WeakComponentImplHelper2 <
        css::lang::XInitialization,
        css::frame::XDispatchProvider
    > PresenterProtocolHandlerInterfaceBase;
}

class PresenterController;

class PresenterProtocolHandler
    : protected ::cppu::BaseMutex,
      public PresenterProtocolHandlerInterfaceBase
{
public:
    PresenterProtocolHandler (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterProtocolHandler (void);

    void SAL_CALL disposing (void);

    static ::rtl::OUString getImplementationName_static (void);
    static css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static (void);
    static css::uno::Reference<css::uno::XInterface> Create(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        SAL_THROW((css::uno::Exception));


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XDispatchProvider

    virtual css::uno::Reference<css::frame::XDispatch > SAL_CALL
        queryDispatch (
            const css::util::URL& aURL,
            const rtl::OUString& aTargetFrameName,
            sal_Int32 nSearchFlags )
        throw(css::uno::RuntimeException);

    virtual css::uno::Sequence<css::uno::Reference<css::frame::XDispatch> > SAL_CALL
        queryDispatches(
            const css::uno::Sequence< css::frame::DispatchDescriptor>& rDescriptors)
        throw(css::uno::RuntimeException);


private:
    class Dispatch;
    ::rtl::Reference<PresenterController> mpPresenterController;

    void ThrowIfDisposed (void) const throw (css::lang::DisposedException);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
