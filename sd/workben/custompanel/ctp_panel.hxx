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

#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
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
            const css::uno::Reference< css::uno::XComponentContext >& i_rContext,
            const css::uno::Reference< css::drawing::framework::XConfigurationController >& i_rConfigController,
            const css::uno::Reference< css::drawing::framework::XResourceId >& i_rResourceId
        );

        // XToolPanel
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getWindow(  ) throw (css::uno::RuntimeException);
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL createAccessible( const css::uno::Reference< css::accessibility::XAccessible >& ParentAccessible ) throw (css::uno::RuntimeException);

        // XView
        // (no methods)

        // XResource
        virtual css::uno::Reference< css::drawing::framework::XResourceId > SAL_CALL getResourceId(  ) throw (css::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAnchorOnly(  ) throw (css::uno::RuntimeException);

        // XPaintListener
        virtual void SAL_CALL windowPaint( const css::awt::PaintEvent& e ) throw (css::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException);

        // XComponent equivalents
        virtual void SAL_CALL disposing();

    protected:
        ~SingleColorPanel();

    private:
        css::uno::Reference< css::uno::XComponentContext >            m_xContext;
        css::uno::Reference< css::drawing::framework::XResourceId >   m_xResourceId;
        css::uno::Reference< css::awt::XWindow >                      m_xWindow;
    };

} } // namespace sd::colortoolpanel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
