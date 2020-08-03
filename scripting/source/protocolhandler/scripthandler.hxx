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

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/script/provider/XScriptProvider.hpp>


namespace com::sun::star {

    namespace document {
        class XScriptInvocationContext;
    }
    namespace uno {
        class Any;
        class XComponentContext;
    }
    namespace lang {
        class XMultiServiceFactory;
        class XSingleServiceFactory;
    }
    namespace frame {
        class XFrame;
        class XDispatch;
        class XNotifyingDispatch;
        class XDispatchResultListener;
        struct DispatchDescriptor;
    }
    namespace beans {
        struct PropertyValue;
    }
    namespace util {
        struct URL;
    }
}

namespace scripting_protocolhandler
{

class ScriptProtocolHandler :
public ::cppu::WeakImplHelper< css::frame::XDispatchProvider,
    css::frame::XNotifyingDispatch, css::lang::XServiceInfo, css::lang::XInitialization >
{
private:
    bool m_bInitialised;
    css::uno::Reference < css::uno::XComponentContext >             m_xContext;
    css::uno::Reference < css::frame::XFrame >                      m_xFrame;
    css::uno::Reference < css::script::provider::XScriptProvider >  m_xScriptProvider;
    css::uno::Reference< css::document::XScriptInvocationContext >  m_xScriptInvocation;

    void createScriptProvider();
    bool getScriptInvocation();

public:
    explicit ScriptProtocolHandler( const css::uno::Reference < css::uno::XComponentContext >& xContext );
    virtual ~ScriptProtocolHandler() override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
    virtual css::uno::Sequence < OUString > SAL_CALL getSupportedServiceNames() override;

    /* Implementation for XDispatchProvider */
    virtual css::uno::Reference < css::frame::XDispatch > SAL_CALL
    queryDispatch( const css::util::URL& aURL, const OUString& sTargetFrameName,
                   sal_Int32 eSearchFlags ) override ;
    virtual css::uno::Sequence< css::uno::Reference < css::frame::XDispatch > > SAL_CALL
    queryDispatches(
        const css::uno::Sequence < css::frame::DispatchDescriptor >& seqDescriptor ) override;

    /* Implementation for X(Notifying)Dispatch */
    virtual void SAL_CALL dispatchWithNotification(
        const css::util::URL& aURL,
        const css::uno::Sequence< css::beans::PropertyValue >& lArgs,
        const css::uno::Reference< css::frame::XDispatchResultListener >& Listener ) override;
    virtual void SAL_CALL dispatch(
        const css::util::URL& aURL,
        const css::uno::Sequence< css::beans::PropertyValue >& lArgs ) override;
    virtual void SAL_CALL addStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& xControl,
        const css::util::URL& aURL ) override;
    virtual void SAL_CALL removeStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& xControl,
        const css::util::URL& aURL ) override;

    /* Implementation for XInitialization */
    virtual void SAL_CALL initialize(
        const css::uno::Sequence < css::uno::Any >& aArguments ) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
