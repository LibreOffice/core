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

#include <AbstractView.hxx>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <memory>

namespace sd::colortoolpanel
{

    typedef ::cppu::WeakComponentImplHelper    <   css::drawing::framework::XView
                                                ,   css::ui::XToolPanel
                                                ,   css::awt::XPaintListener
                                                >   SingleColorPanel_Base;
    class SingleColorPanel  :public ::cppu::BaseMutex
                            ,public SingleColorPanel_Base
    {
    public:
        SingleColorPanel(
            const cpo::uno::Reference< cpo::uno::XComponentContext >& i_rContext,
            const rtl::Reference< ConfigurationController >& i_rConfigController,
            const cpo::uno::Reference< css::drawing::framework::XResourceId >& i_rResourceId
        );

        // XToolPanel
        virtual cpo::uno::Reference< css::awt::XWindow > getWindow(  ) throw (cpo::uno::RuntimeException);
        virtual cpo::uno::Reference< css::accessibility::XAccessible > createAccessible( const cpo::uno::Reference< css::accessibility::XAccessible >& ParentAccessible ) throw (cpo::uno::RuntimeException);

        // XView
        // (no methods)

        // XResource
        virtual cpo::uno::Reference< css::drawing::framework::XResourceId > getResourceId(  ) throw (cpo::uno::RuntimeException);
        virtual bool isAnchorOnly(  ) throw (cpo::uno::RuntimeException);

        // XPaintListener
        virtual void windowPaint( const css::awt::PaintEvent& e ) throw (cpo::uno::RuntimeException);

        // XEventListener
        virtual void disposing( const css::lang::EventObject& Source ) throw (cpo::uno::RuntimeException);

        // XComponent equivalents
        virtual void disposing();

    protected:
        ~SingleColorPanel();

    private:
        cpo::uno::Reference< cpo::uno::XComponentContext >            m_xContext;
        cpo::uno::Reference< css::drawing::framework::XResourceId >   m_xResourceId;
        cpo::uno::Reference< css::awt::XWindow >                      m_xWindow;
    };

} } // namespace sd::colortoolpanel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
