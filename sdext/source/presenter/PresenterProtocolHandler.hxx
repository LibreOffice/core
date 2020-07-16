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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPROTOCOLHANDLER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPROTOCOLHANDLER_HXX

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>

namespace sdext::presenter {

typedef ::cppu::WeakComponentImplHelper <
    css::lang::XInitialization,
    css::lang::XServiceInfo,
    css::frame::XDispatchProvider
> PresenterProtocolHandlerInterfaceBase;

class PresenterController;

class PresenterProtocolHandler
    : protected ::cppu::BaseMutex,
      public PresenterProtocolHandlerInterfaceBase
{
public:
    PresenterProtocolHandler ();
    virtual ~PresenterProtocolHandler() override;

    void SAL_CALL disposing() override;

    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XDispatchProvider

    virtual css::uno::Reference<css::frame::XDispatch > SAL_CALL
        queryDispatch (
            const css::util::URL& aURL,
            const OUString& aTargetFrameName,
            sal_Int32 nSearchFlags ) override;

    virtual css::uno::Sequence<css::uno::Reference<css::frame::XDispatch> > SAL_CALL
        queryDispatches(
            const css::uno::Sequence< css::frame::DispatchDescriptor>& rDescriptors) override;

private:
    class Dispatch;
    ::rtl::Reference<PresenterController> mpPresenterController;

    /// @throws css::lang::DisposedException
    void ThrowIfDisposed() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
