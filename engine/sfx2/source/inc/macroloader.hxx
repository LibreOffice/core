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

#ifndef INCLUDED_SFX2_SOURCE_INC_MACROLOADER_HXX
#define INCLUDED_SFX2_SOURCE_INC_MACROLOADER_HXX

#include <sal/config.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/util/URL.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <comphelper/errcode.hxx>

class SfxObjectShell;

class SfxMacroLoader final : public cppu::WeakImplHelper<
                                       css::frame::XDispatchProvider,
                                       css::frame::XNotifyingDispatch,
                                       css::frame::XSynchronousDispatch,
                                       css::lang::XServiceInfo>
{
    cpo::uno::WeakReference < css::frame::XFrame > m_xFrame;
    SfxObjectShell* GetObjectShell_Impl();

public:
    /// @throws cpo::uno::Exception
    /// @throws cpo::uno::RuntimeException
    SfxMacroLoader(const cpo::uno::Sequence< cpo::uno::Any >& aArguments);

    virtual OUString getImplementationName() override;

    virtual bool supportsService(OUString const & ServiceName) override;

    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    /// @throws cpo::uno::RuntimeException
    /// @throws css::ucb::ContentCreationException
    static ErrCode loadMacro( const OUString& aURL, cpo::uno::Any& rRetval, SfxObjectShell* pDoc );

    virtual css::uno::Reference < css::frame::XDispatch > queryDispatch(
            const css::util::URL& aURL, const OUString& sTargetFrameName,
            sal_Int32 eSearchFlags ) override;

    virtual cpo::uno::Sequence< css::uno::Reference < css::frame::XDispatch > > queryDispatches(
            const cpo::uno::Sequence < css::frame::DispatchDescriptor >& seqDescriptor ) override;

    virtual void dispatchWithNotification( const css::util::URL& aURL, const cpo::uno::Sequence< css::beans::PropertyValue >& lArgs, const css::uno::Reference< css::frame::XDispatchResultListener >& Listener ) override;

    virtual void dispatch( const css::util::URL& aURL, const cpo::uno::Sequence< css::beans::PropertyValue >& lArgs ) override;

    virtual cpo::uno::Any dispatchWithReturnValue( const css::util::URL& aURL, const cpo::uno::Sequence< css::beans::PropertyValue >& lArgs ) override;

    virtual void addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) override;

    virtual void removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) override;

    static SfxObjectShell* GetObjectShell(const css::uno::Reference<css::frame::XFrame>& xFrame);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
