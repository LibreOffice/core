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

#pragma once

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;

namespace framework
{

class ButtonToolbarController final : public css::frame::XStatusListener,
                                public css::frame::XToolbarController,
                                public css::lang::XInitialization,
                                public css::util::XUpdatable,
                                public css::lang::XComponent,
                                public ::cppu::BaseMutex,
                                public ::cppu::OWeakObject
{
    public:
        ButtonToolbarController( css::uno::Reference< css::uno::XComponentContext > xContext,
                                 ToolBox* pToolBar,
                                 OUString aCommand );
        virtual ~ButtonToolbarController() override;

        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

        // XInitialization
        virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

        // XUpdatable
        virtual void update() override;

        // XComponent
        virtual void dispose() override;
        virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
        virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

        // XEventListener
        virtual void disposing( const css::lang::EventObject& Source ) override;

        // XStatusListener
        virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        // XToolbarController
        virtual void execute( sal_Int16 KeyModifier ) override;
        virtual void click() override;
        virtual void doubleClick() override;
        virtual css::uno::Reference< css::awt::XWindow > createPopupWindow() override;
        virtual css::uno::Reference< css::awt::XWindow > createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

    private:
        bool                                                   m_bInitialized : 1,
                                                               m_bDisposed : 1;
        OUString                                               m_aCommandURL;
        css::uno::Reference< css::frame::XFrame >              m_xFrame;
        css::uno::Reference< css::uno::XComponentContext >     m_xContext;
        css::uno::Reference< css::util::XURLTransformer >      m_xURLTransformer;
        VclPtr<ToolBox>                                        m_pToolbar;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
