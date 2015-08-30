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

#ifndef INCLUDED_SD_WORKBEN_CUSTOMPANEL_CTP_PANEL_HXX
#define INCLUDED_SD_WORKBEN_CUSTOMPANEL_CTP_PANEL_HXX

#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <boost/scoped_ptr.hpp>

namespace sd { namespace colortoolpanel
{

    // class SingleColorPanel
    typedef ::cppu::WeakComponentImplHelper    <   ::com::sun::star::drawing::framework::XView
                                                ,   ::com::sun::star::ui::XToolPanel
                                                ,   ::com::sun::star::awt::XPaintListener
                                                >   SingleColorPanel_Base;
    class SingleColorPanel  :public ::cppu::BaseMutex
                            ,public SingleColorPanel_Base
    {
    public:
        SingleColorPanel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XConfigurationController >& i_rConfigController,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& i_rResourceId
        );

        // XToolPanel
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getWindow(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL createAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& ParentAccessible ) throw (::com::sun::star::uno::RuntimeException);

        // XView
        // (no methods)

        // XResource
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId > SAL_CALL getResourceId(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAnchorOnly(  ) throw (::com::sun::star::uno::RuntimeException);

        // XPaintListener
        virtual void SAL_CALL windowPaint( const ::com::sun::star::awt::PaintEvent& e ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent equivalents
        virtual void SAL_CALL disposing();

    protected:
        ~SingleColorPanel();

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >   m_xResourceId;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                      m_xWindow;
    };

} } // namespace sd::colortoolpanel

#endif // INCLUDED_SD_WORKBEN_CUSTOMPANEL_CTP_PANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
