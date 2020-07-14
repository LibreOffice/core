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

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XDialogProvider2.hpp>
#include <com/sun/star/awt/XContainerWindowProvider.hpp>
#include <com/sun/star/awt/XUnoControlDialog.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <memory>


namespace dlgprov
{


    // mutex


    ::osl::Mutex& getMutex();



    css::uno::Reference< css::container::XNameContainer > lcl_createControlModel(const css::uno::Reference< css::uno::XComponentContext >& i_xContext);
    css::uno::Reference< css::resource::XStringResourceManager > lcl_getStringResourceManager(const css::uno::Reference< css::uno::XComponentContext >& i_xContext,const OUString& i_sURL);
    /// @throws css::uno::Exception
    css::uno::Reference< css::container::XNameContainer > lcl_createDialogModel(
                const css::uno::Reference< css::uno::XComponentContext >& i_xContext,
                const css::uno::Reference< css::io::XInputStream >& xInput,
                const css::uno::Reference< css::frame::XModel >& xModel,
                const css::uno::Reference< css::resource::XStringResourceManager >& xStringResourceManager,
                const css::uno::Any &aDialogSourceURL);

    typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::awt::XDialogProvider2,
        css::awt::XContainerWindowProvider > DialogProviderImpl_BASE;

    class DialogProviderImpl : public DialogProviderImpl_BASE
    {
    private:
        struct BasicRTLParams
        {
             css::uno::Reference< css::io::XInputStream >          mxInput;
             css::uno::Reference< css::container::XNameContainer > mxDlgLib;
             css::uno::Reference< css::script::XScriptListener >   mxBasicRTLListener;
        };
        std::unique_ptr< BasicRTLParams > m_BasicInfo;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::frame::XModel >                 m_xModel;

    OUString msDialogLibName;
        css::uno::Reference< css::awt::XControlModel > createDialogModel( const OUString& sURL );

        css::uno::Reference< css::awt::XUnoControlDialog > createDialogControl(
            const css::uno::Reference< css::awt::XControlModel >& rxDialogModel,
            const css::uno::Reference< css::awt::XWindowPeer >& xParent );

        void attachControlEvents( const css::uno::Reference< css::awt::XControl >& rxControlContainer,
            const css::uno::Reference< css::uno::XInterface >& rxHandler,
            const css::uno::Reference< css::beans::XIntrospectionAccess >& rxIntrospectionAccess,
            bool bDialogProviderMode );
        css::uno::Reference< css::beans::XIntrospectionAccess > inspectHandler(
            const css::uno::Reference< css::uno::XInterface >& rxHandler );
    // helper methods
            /// @throws css::uno::Exception
            css::uno::Reference< css::container::XNameContainer > createDialogModel(
                const css::uno::Reference< css::io::XInputStream >& xInput,
                const css::uno::Reference< css::resource::XStringResourceManager >& xStringResourceManager,
                const css::uno::Any &aDialogSourceURL);
            /// @throws css::uno::Exception
            css::uno::Reference< css::awt::XControlModel > createDialogModelForBasic();

        // XDialogProvider / XDialogProvider2 impl method
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::RuntimeException
        css::uno::Reference < css::awt::XControl > createDialogImpl(
            const OUString& URL,
            const css::uno::Reference< css::uno::XInterface >& xHandler,
            const css::uno::Reference< css::awt::XWindowPeer >& xParent,
            bool bDialogProviderMode );

    public:
        explicit DialogProviderImpl(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~DialogProviderImpl() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XDialogProvider
        virtual css::uno::Reference < css::awt::XDialog > SAL_CALL createDialog(
            const OUString& URL ) override;

        // XDialogProvider2
        virtual css::uno::Reference < css::awt::XDialog > SAL_CALL createDialogWithHandler(
            const OUString& URL,
            const css::uno::Reference< css::uno::XInterface >& xHandler ) override;

        virtual css::uno::Reference < css::awt::XDialog > SAL_CALL createDialogWithArguments(
            const OUString& URL,
            const css::uno::Sequence< css::beans::NamedValue >& Arguments ) override;

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createContainerWindow(
            const OUString& URL, const OUString& WindowType,
            const css::uno::Reference< css::awt::XWindowPeer >& xParent,
            const css::uno::Reference< css::uno::XInterface >& xHandler ) override;
     };


}   // namespace dlgprov

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
