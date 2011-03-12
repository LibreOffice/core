/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/

#ifndef SD_WORKBENCH_CTP_PANEL_HXX
#define SD_WORKBENCH_CTP_PANEL_HXX

/** === begin UNO includes === **/
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
/** === end UNO includes === **/

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= SingleColorPanel
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::ui::XToolPanel
                                                ,   ::com::sun::star::awt::XPaintListener
                                                >   SingleColorPanel_Base;
    class SingleColorPanel  :public ::cppu::BaseMutex
                            ,public SingleColorPanel_Base
    {
    public:
        SingleColorPanel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& i_rParentWindow,
            const ::sal_Int32 i_nPanelColor
        );

        // XToolPanel
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL createAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible ) throw (::com::sun::star::uno::RuntimeException);

        // XPaintListener
        virtual void SAL_CALL windowPaint( const ::com::sun::star::awt::PaintEvent& e ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent equivalents
        virtual void SAL_CALL disposing();

    protected:
        ~SingleColorPanel();

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  m_xWindow;
        const sal_Int32                                                     m_nPanelColor;
    };

    //==================================================================================================================
    //= PanelUIElement
    //==================================================================================================================
    typedef ::cppu::WeakComponentImplHelper1    <   ::com::sun::star::ui::XUIElement
                                                >   PanelUIElement_Base;
    class PanelUIElement    :public ::cppu::BaseMutex
                            ,public PanelUIElement_Base
    {
    public:
        PanelUIElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& i_rParentWindow,
            const ::rtl::OUString& i_rResourceURL,
            const ::sal_Int32 i_nPanelColor
        );

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getResourceURL() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getType() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface(  ) throw (::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing();

    protected:
        ~PanelUIElement();

    private:
        const ::rtl::OUString   m_sResourceURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >
                                m_xToolPanel;
    };

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................

#endif // SD_WORKBENCH_CTP_PANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
