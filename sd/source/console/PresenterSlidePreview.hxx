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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSLIDEPREVIEW_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSLIDEPREVIEW_HXX

#include "PresenterController.hxx"

#include <SlideRenderer.hxx>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <framework/AbstractPane.hxx>
#include <framework/AbstractView.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

namespace sdext::presenter {

typedef ::cppu::ImplInheritanceHelper <
    sd::framework::AbstractView,
    css::drawing::XDrawView,
    css::awt::XPaintListener,
    css::awt::XWindowListener
> PresenterSlidePreviewInterfaceBase;

/** Static preview of a slide.  Typically used for the preview of the next
    slide.
    This implementation shows a preview of the slide given to the
    setCurrentSlide.  For showing the next slide the PresenterViewFactory
    uses a derived class that overrides the setCurrentSlide() method.
*/
class PresenterSlidePreview
    : public PresenterSlidePreviewInterfaceBase
{
public:
    PresenterSlidePreview (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const rtl::Reference<sd::framework::AbstractPane>& rxAnchorPane,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterSlidePreview() override;

    PresenterSlidePreview(const PresenterSlidePreview&) = delete;
    PresenterSlidePreview& operator=(const PresenterSlidePreview&) = delete;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // XResourceId

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId() override;

    virtual sal_Bool SAL_CALL isAnchorOnly() override;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent) override;

    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent) override;

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide) override;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage() override;

protected:
    ::rtl::Reference<PresenterController> mpPresenterController;

private:
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    rtl::Reference<sd::presenter::SlideRenderer> mxPreviewRenderer;

    /** This Image holds the preview of the current slide.  After resize
        requests the image may be empty.  This results eventually in a call
        to ProvideSlide() in order to created a preview in the correct new
        size.
    */
    css::uno::Reference<css::rendering::XBitmap> mxPreview;
    std::shared_ptr<PresenterBitmapContainer> mpBitmaps;

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
    void Resize();
};

} // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
