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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>

class BasicManager;


namespace basprov
{




    typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::script::provider::XScriptProvider,
        css::script::browse::XBrowseNode > BasicProviderImpl_BASE;


    class BasicProviderImpl : public BasicProviderImpl_BASE, public SfxListener
    {
    private:
        BasicManager*   m_pAppBasicManager;
        BasicManager*   m_pDocBasicManager;
        css::uno::Reference< css::script::XLibraryContainer >             m_xLibContainerApp;
        css::uno::Reference< css::script::XLibraryContainer >             m_xLibContainerDoc;
        css::uno::Reference< css::uno::XComponentContext >                m_xContext;
        css::uno::Reference< css::document::XScriptInvocationContext >    m_xInvocationContext;
        OUString  m_sScriptingContext;
        bool m_bIsAppScriptCtx;
        bool m_bIsUserCtx;

        bool isLibraryShared(
            const css::uno::Reference< css::script::XLibraryContainer >& rxLibContainer,
            const OUString& rLibName );

    public:
        explicit BasicProviderImpl(
            const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~BasicProviderImpl() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XScriptProvider
        virtual css::uno::Reference < css::script::provider::XScript > SAL_CALL getScript(
            const OUString& scriptURI ) override;

        // XBrowseNode
        virtual OUString SAL_CALL getName(  ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::script::browse::XBrowseNode > > SAL_CALL getChildNodes(  ) override;
        virtual sal_Bool SAL_CALL hasChildNodes(  ) override;
        virtual sal_Int16 SAL_CALL getType(  ) override;

    protected:
        // SfxListener
        virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
    };


}   // namespace basprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
