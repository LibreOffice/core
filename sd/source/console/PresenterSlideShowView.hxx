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
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

namespace sdext::presenter {

typedef cppu::WeakComponentImplHelper<
    css::presentation::XSlideShowView,
    css::awt::XPaintListener,
    css::awt::XMouseListener,
    css::awt::XMouseMotionListener,
    css::awt::XWindowListener,
    css::drawing::framework::XView,
    css::drawing::XDrawView
    > PresenterSlideShowViewInterfaceBase;

/** Life view in a secondary window of a full screen slide show.
*/
class PresenterSlideShowView
    : protected ::cppu::BaseMutex,
      public PresenterSlideShowViewInterfaceBase,
      public CachablePresenterView
{
public:
    PresenterSlideShowView (
        css::uno::Reference<css::uno::XComponentContext> xContext,
        css::uno::Reference<css::drawing::framework::XResourceId> xViewId,
        const rtl::Reference<::sd::DrawController>& rxController,
        ::rtl::Reference<PresenterController> xPresenterController);
    virtual ~PresenterSlideShowView() override;
    PresenterSlideShowView(const PresenterSlideShowView&) = delete;
    PresenterSlideShowView& operator=(const PresenterSlideShowView&) = delete;

    void LateInit();
    virtual void SAL_CALL disposing() override;

    // CachablePresenterView

    virtual void ReleaseView() override;

    // XSlideShowView

    virtual css::uno::Reference<
        css::rendering::XSpriteCanvas > SAL_CALL getCanvas() override;

    virtual void SAL_CALL clear() override;

    virtual css::geometry::AffineMatrix2D SAL_CALL getTransformation() override;

    virtual css::geometry::IntegerSize2D SAL_CALL getTranslationOffset() override;

    virtual void SAL_CALL addTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener) override;

    virtual void SAL_CALL removeTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener) override;

    virtual void SAL_CALL addPaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener) override;

    virtual void SAL_CALL removePaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener) override;

    virtual void SAL_CALL addMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener) override;

    virtual void SAL_CALL removeMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener) override;

    virtual void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener) override;

    virtual void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener) override;

    virtual void SAL_CALL setMouseCursor(::sal_Int16 nPointerShape) override;

    virtual css::awt::Rectangle SAL_CALL getCanvasArea(  ) override;

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

    // XPaintListener
    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent) override;

    // XMouseListener
    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent) override;

    // XMouseMotionListener

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent) override;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent) override;

    // XView

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL
        getResourceId() override;

    virtual sal_Bool SAL_CALL isAnchorOnly() override;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide) override;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage() override;

    // CachablePresenterView

    virtual void ActivatePresenterView() override;

    virtual void DeactivatePresenterView() override;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    rtl::Reference<::sd::DrawController> mxController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::presentation::XSlideShow> mxSlideShow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::rendering::XCanvas> mxViewCanvas;
    css::uno::Reference<css::awt::XPointer> mxPointer;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::awt::XWindow> mxViewWindow;
    css::uno::Reference<css::drawing::framework::XPane> mxTopPane;
    rtl::Reference<sd::presenter::PresenterHelper> mxPresenterHelper;
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

    void CreateBackgroundPolygons();

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed();

    void impl_addAndConfigureView();
};

} // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
