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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSLIDESHOWVIEW_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSLIDESHOWVIEW_HXX

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
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <boost/noncopyable.hpp>

namespace sdext { namespace presenter {

namespace {
    typedef cppu::WeakComponentImplHelper<
        css::presentation::XSlideShowView,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener,
        css::awt::XWindowListener,
        css::drawing::framework::XView,
        css::drawing::XDrawView
        > PresenterSlideShowViewInterfaceBase;
}

/** Life view in a secondary window of a full screen slide show.
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
    virtual ~PresenterSlideShowView();

    void LateInit();
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // CachablePresenterView

    virtual void ReleaseView() SAL_OVERRIDE;

    // XSlideShowView

    virtual css::uno::Reference<
        css::rendering::XSpriteCanvas > SAL_CALL getCanvas()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL clear()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::geometry::AffineMatrix2D SAL_CALL getTransformation()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::geometry::IntegerSize2D SAL_CALL getTranslationOffset()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setMouseCursor(::sal_Int16 nPointerShape)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::awt::Rectangle SAL_CALL getCanvasArea(  )
    throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPaintListener
    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMouseListener
    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMouseMotionListener

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XView

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL
        getResourceId()
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // CachablePresenterView

    virtual void ActivatePresenterView() SAL_OVERRIDE;

    virtual void DeactivatePresenterView() SAL_OVERRIDE;

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

    /** This broadcast helper is used to notify listeners registered to a
        SlideShowView object.
    */
    ::cppu::OBroadcastHelper maBroadcaster;

    SharedBitmapDescriptor mpBackground;

    bool mbIsInModifyNotification;
    bool mbIsForcedPaintPending;
    bool mbIsPaintPending;
    OUString msClickToExitPresentationText;
    OUString msClickToExitPresentationTitle;
    OUString msTitleTemplate;
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

    void Resize();

    void PaintOuterWindow (const css::awt::Rectangle& rRepaintBox);
    void PaintInnerWindow (const css::awt::PaintEvent& rEvent);
    void PaintEndSlide (const css::awt::Rectangle& rRepaintBox);

    /** The slide show relies on the back buffer of the canvas not being
        modified.  With a shared canvas there are times when that can not be
        guaranteed.
        Call this method when the back buffer may have changed its content,
        like when the window has been moved but not resized.
    */
    void ForceRepaint();

    void CreateBackgroundPolygons();

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed()
        throw (css::lang::DisposedException);

    void impl_addAndConfigureView();
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
