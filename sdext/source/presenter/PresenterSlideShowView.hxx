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

#ifndef SDEXT_PRESENTER_SLIDE_SHOW_VIEW_HXX
#define SDEXT_PRESENTER_SLIDE_SHOW_VIEW_HXX

#include "PresenterViewFactory.hxx"
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XPointer.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <cppuhelper/compbase7.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

namespace {
    typedef cppu::WeakComponentImplHelper7<
        css::presentation::XSlideShowView,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener,
        css::awt::XWindowListener,
        css::drawing::framework::XView,
        css::drawing::XDrawView
        > PresenterSlideShowViewInterfaceBase;
}

/** Life view in a secondary windo of a full screen slide show.
*/
class PresenterSlideShowView
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public PresenterSlideShowViewInterfaceBase,
      public CachablePresenterView
{
public:
    PresenterSlideShowView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterSlideShowView (void);

    void LateInit (void);
    virtual void SAL_CALL disposing (void);


    // CachablePresenterView

    virtual void ReleaseView (void);


    // XSlideShowView

    virtual css::uno::Reference<
        css::rendering::XSpriteCanvas > SAL_CALL getCanvas (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL clear (void)
        throw (css::uno::RuntimeException);

    virtual css::geometry::AffineMatrix2D SAL_CALL getTransformation (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addPaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removePaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setMouseCursor(::sal_Int16 nPointerShape)
        throw (css::uno::RuntimeException);

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCanvasArea(  )
    throw (::com::sun::star::uno::RuntimeException);

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XPaintListener
    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseListener
    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseMotionListener

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XView

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL
        getResourceId (void)
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage (void)
        throw (css::uno::RuntimeException);


    // CachablePresenterView

    virtual void ActivatePresenterView (void);

    virtual void DeactivatePresenterView (void);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::presentation::XSlideShow> mxSlideShow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::rendering::XCanvas> mxViewCanvas;
    css::uno::Reference<css::awt::XPointer> mxPointer;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::awt::XWindow> mxViewWindow;
    css::uno::Reference<css::drawing::framework::XPane> mxTopPane;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxBackgroundPolygon1;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxBackgroundPolygon2;
    bool mbIsViewAdded;

    /** Aspect ratio of the current slide.
    */
    double mnPageAspectRatio;

    /** This broadcast helper is used to notify listeners registed to a
        SlideShowView object.
    */
    ::cppu::OBroadcastHelper maBroadcaster;

    SharedBitmapDescriptor mpBackground;

    bool mbIsInModifyNotification;
    bool mbIsForcedPaintPending;
    bool mbIsPaintPending;
    ::rtl::OUString msClickToExitPresentationText;
    ::rtl::OUString msClickToExitPresentationTitle;
    ::rtl::OUString msTitleTemplate;
    bool mbIsEndSlideVisible;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;

    /** Create the window into which the slide show will render its
        content.  This window has the correct aspect ratio and is displayed centered
        and as large as possible in its parent window.
    */
    css::uno::Reference<css::awt::XWindow> CreateViewWindow (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow) const;
    css::uno::Reference<css::rendering::XCanvas> CreateViewCanvas (
        const css::uno::Reference<css::awt::XWindow>& rxWindow) const;

    void Resize (void);

    void PaintOuterWindow (const css::awt::Rectangle& rRepaintBox);
    void PaintInnerWindow (const css::awt::PaintEvent& rEvent);
    void PaintEndSlide (const css::awt::Rectangle& rRepaintBox);

    /** The slide show relies on the back buffer of the canvas not being
        modified.  With a shared canvas there are times when that can not be
        guaranteed.
        Call this method when the back buffer may have changed its content,
        like when the window has been moved but not resized.
    */
    void ForceRepaint (void);

    void CreateBackgroundPolygons (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
