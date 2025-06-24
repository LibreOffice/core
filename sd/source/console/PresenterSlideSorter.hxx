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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSLIDESORTER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSLIDESORTER_HXX

#include <memory>
#include "PresenterController.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterViewFactory.hxx"
#include <PresenterPreviewCache.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <framework/AbstractView.hxx>
#include <ResourceId.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

namespace sdext::presenter {

class PresenterButton;
class PresenterScrollBar;

typedef cppu::ImplInheritanceHelper<
    sd::framework::AbstractView,
    css::awt::XWindowListener,
    css::awt::XPaintListener,
    css::beans::XPropertyChangeListener,
    css::drawing::XSlidePreviewCacheListener,
    css::awt::XMouseListener,
    css::awt::XMouseMotionListener,
    css::drawing::XDrawView
    > PresenterSlideSorterInterfaceBase;

/** A simple slide sorter for the presenter screen.  It uses a preview cache
    to create the slide previews.  Painting is done via a canvas.
*/
class PresenterSlideSorter
    : public PresenterSlideSorterInterfaceBase,
      public CachablePresenterView
{
public:
    PresenterSlideSorter (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const rtl::Reference<sd::framework::ResourceId>& rxViewId,
        const rtl::Reference<::sd::DrawController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterSlideSorter() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // lang::XEventListener

    using WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject) override;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent) override;

    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent) override;

    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent) override;

    // XMouseMotionListener

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent) override;

    // AbstractResource

    virtual rtl::Reference<sd::framework::ResourceId> getResourceId() override;

    virtual bool isAnchorOnly() override;

    // XPropertyChangeListener

    virtual void SAL_CALL propertyChange (
        const css::beans::PropertyChangeEvent& rEvent) override;

    // XSlidePreviewCacheListener

    virtual void SAL_CALL notifyPreviewCreation (
        sal_Int32 nSlideIndex) override;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide) override;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage() override;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    rtl::Reference<sd::framework::ResourceId> mxViewId;
    rtl::Reference<sd::framework::AbstractPane> mxPane;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    rtl::Reference<sd::presenter::PresenterPreviewCache> mxPreviewCache;
    bool mbIsLayoutPending;
    class Layout;
    std::shared_ptr<Layout> mpLayout;
    ::rtl::Reference<PresenterScrollBar> mpVerticalScrollBar;
    ::rtl::Reference<PresenterButton> mpCloseButton;
    class MouseOverManager;
    std::unique_ptr<MouseOverManager> mpMouseOverManager;
    sal_Int32 mnSlideIndexMousePressed;
    sal_Int32 mnCurrentSlideIndex;
    sal_Int32 mnSeparatorY;
    css::util::Color maSeparatorColor;
    css::awt::Rectangle maCurrentSlideFrameBoundingBox;
    class CurrentSlideFrameRenderer;
    std::shared_ptr<CurrentSlideFrameRenderer> mpCurrentSlideFrameRenderer;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxPreviewFrame;

    void UpdateLayout();
    css::geometry::RealRectangle2D PlaceScrollBars (
        const css::geometry::RealRectangle2D& rUpperBox);
    void PlaceCloseButton (
        const PresenterPaneContainer::SharedPaneDescriptor& rpPane,
        const css::awt::Rectangle& rCenterBox,
    const sal_Int32 nLeftFrameWidth);
    void ClearBackground (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rRedrawArea);
    double GetSlideAspectRatio() const;
    css::uno::Reference<css::rendering::XBitmap> GetPreview (const sal_Int32 nSlideIndex);
    void PaintPreview (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rUpdateBox,
        const sal_Int32 nSlideIndex);
    void Paint (const css::awt::Rectangle& rUpdateBox);
    void SetHorizontalOffset (const double nXOffset);
    void SetVerticalOffset (const double nYOffset);
    void GotoSlide (const sal_Int32 nSlideIndex);
    void ScrollSlideIntoView(sal_Int32 nSlideIndex);
    bool ProvideCanvas();
};

} // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
