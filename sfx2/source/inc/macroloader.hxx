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
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/util/URL.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <sfx2/sfxuno.hxx>
#include <vcl/errcode.hxx>

class SfxObjectShell;

class SfxMacroLoader : public cppu::WeakImplHelper<
                                       css::frame::XDispatchProvider,
                                       css::frame::XNotifyingDispatch,
                                       css::frame::XSynchronousDispatch,
                                       css::lang::XServiceInfo>
{
    css::uno::WeakReference < css::frame::XFrame > m_xFrame;
    SfxObjectShell* GetObjectShell_Impl();

public:
    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    SfxMacroLoader(const css::uno::Sequence< css::uno::Any >& aArguments);

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    /// @throws css::uno::RuntimeException
    /// @throws css::ucb::ContentCreationException
    static ErrCode loadMacro( const OUString& aURL, css::uno::Any& rRetval, SfxObjectShell* pDoc );

    virtual css::uno::Reference < css::frame::XDispatch > SAL_CALL queryDispatch(
            const css::util::URL& aURL, const OUString& sTargetFrameName,
            sal_Int32 eSearchFlags ) override;

    virtual css::uno::Sequence< css::uno::Reference < css::frame::XDispatch > > SAL_CALL queryDispatches(
            const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescriptor ) override;

    virtual void SAL_CALL dispatchWithNotification( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArgs, const css::uno::Reference< css::frame::XDispatchResultListener >& Listener ) override;

    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArgs ) override;

    virtual css::uno::Any SAL_CALL dispatchWithReturnValue( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArgs ) override;

    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) override;

    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
