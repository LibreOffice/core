/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef SDEXT_PRESENTER_PRESENTER_BUTTON_HXX
#define SDEXT_PRESENTER_PRESENTER_BUTTON_HXX

#include "PresenterBitmapContainer.hxx"
#include "PresenterTheme.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <boost/noncopyable.hpp>
#include <rtl/ref.hxx>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterController;

namespace {
    typedef ::cppu::WeakComponentImplHelper4 <
        css::awt::XWindowListener,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener
    > PresenterButtonInterfaceBase;
}

/** Button for the presenter screen.  It displays a text surrounded by a
    frame.
*/
class PresenterButton
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterButtonInterfaceBase
{
public:
    static ::rtl::Reference<PresenterButton> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const ::boost::shared_ptr<PresenterTheme>& rpTheme,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const css::uno::Reference<css::rendering::XCanvas>& rxParentCanvas,
        const ::rtl::OUString& rsConfigurationName);
    ~PresenterButton (void);

    virtual void SAL_CALL disposing (void);

    void SetCenter (const css::geometry::RealPoint2D& rLocation);
    void SetCanvas (
        const css::uno::Reference<css::rendering::XCanvas>& rxParentCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow);
    css::geometry::IntegerSize2D GetSize (void);

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);


    // XMouseMotionListener

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);


    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

private:
    ::rtl::Reference<PresenterController> mpPresenterController;
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    const ::rtl::OUString msText;
    const PresenterTheme::SharedFontDescriptor mpFont;
    const PresenterTheme::SharedFontDescriptor mpMouseOverFont;
    const ::rtl::OUString msAction;
    css::geometry::RealPoint2D maCenter;
    css::geometry::IntegerSize2D maButtonSize;
    PresenterBitmapDescriptor::Mode meState;
    css::uno::Reference<css::rendering::XBitmap> mxNormalBitmap;
    css::uno::Reference<css::rendering::XBitmap> mxMouseOverBitmap;

    PresenterButton (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const ::boost::shared_ptr<PresenterTheme>& rpTheme,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const PresenterTheme::SharedFontDescriptor& rFont,
        const PresenterTheme::SharedFontDescriptor& rMouseOverFont,
        const ::rtl::OUString& rxText,
        const ::rtl::OUString& rxAction);
    void RenderButton (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::geometry::IntegerSize2D& rSize,
        const PresenterTheme::SharedFontDescriptor& rFont,
        const PresenterBitmapDescriptor::Mode eMode,
        const SharedBitmapDescriptor& rpLeft,
        const SharedBitmapDescriptor& rpCenter,
        const SharedBitmapDescriptor& rpRight);
    css::geometry::IntegerSize2D CalculateButtonSize (void);
    void Invalidate (void);
    css::uno::Reference<css::rendering::XBitmap> GetBitmap (
        const SharedBitmapDescriptor& mpIcon,
        const PresenterBitmapDescriptor::Mode eMode);
    void SetupButtonBitmaps (void);
    static css::uno::Reference<css::beans::XPropertySet> GetConfigurationProperties (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const ::rtl::OUString& rsConfgurationName);

    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
