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

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <boost/unordered_map.hpp>
#include <rtl/ref.hxx>
#include <boost/scoped_ptr.hpp>

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
