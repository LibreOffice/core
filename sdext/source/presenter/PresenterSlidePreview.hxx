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

#ifndef SDEXT_PRESENTER_SLIDE_PREVIEW_HXX
#define SDEXT_PRESENTER_SLIDE_PREVIEW_HXX

#include "PresenterController.hxx"

#include <boost/noncopyable.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XDisplayBitmap.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XSlideRenderer.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <rtl/ref.hxx>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper4 <
        css::drawing::framework::XView,
        css::drawing::XDrawView,
        css::awt::XPaintListener,
        css::awt::XWindowListener
    > PresenterSlidePreviewInterfaceBase;
}


/** Static preview of a slide.  Typically used for the preview of the next
    slide.
    This implementation shows a preview of the slide given to the
    setCurrentSlide.  For showing the next slide the PresenterViewFactory
    uses a derived class that overloads the setCurrentSlide() method.
*/
class PresenterSlidePreview
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterSlidePreviewInterfaceBase
{
public:
    PresenterSlidePreview (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxAnchorPane,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterSlidePreview (void);
    virtual void SAL_CALL disposing (void);


    // XResourceId

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId (void)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);

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


    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage (void)
        throw (css::uno::RuntimeException);

protected:
    ::rtl::Reference<PresenterController> mpPresenterController;

private:
    css::uno::Reference<css::drawing::framework::XPane> mxPane;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::drawing::XSlideRenderer> mxPreviewRenderer;

    /** This Image holds the preview of the current slide.  After resize
        requests the image may be empty.  This results eventually in a call
        to ProvideSlide() in order to created a preview in the correct new
        size.
    */
    css::uno::Reference<css::rendering::XBitmap> mxPreview;

    /**  The current slide for which a preview is displayed.  This may or
        may not be the same as the current slide of the PresenterView.
    */
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;
    double mnSlideAspectRatio;

    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;

    /** Set the given slide as the current slide of the called PresenterSlidePreview
        object.
    */
    void SetSlide (const css::uno::Reference<css::drawing::XDrawPage>& rxPage);

    /** Paint the preview of the current slide centered in the window of the
        anchor pane.
    */
    void Paint (const css::awt::Rectangle& rBoundingBox);

    /** React to a resize of the anchor pane.
    */
    void Resize (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
