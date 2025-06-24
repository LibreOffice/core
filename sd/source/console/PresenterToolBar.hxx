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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERTOOLBAR_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERTOOLBAR_HXX

#include "PresenterController.hxx"
#include "PresenterViewFactory.hxx"

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <framework/AbstractView.hxx>
#include <ResourceId.hxx>
#include <com/sun/star/frame/XController.hpp>

#include <functional>

namespace sdext::presenter {

typedef cppu::WeakComponentImplHelper<
    css::awt::XWindowListener,
    css::awt::XPaintListener,
    css::awt::XMouseListener,
    css::awt::XMouseMotionListener,
    css::drawing::XDrawView
    > PresenterToolBarInterfaceBase;

typedef cppu::ImplInheritanceHelper<
    sd::framework::AbstractView,
    css::awt::XPaintListener,
    css::drawing::XDrawView
    > PresenterToolBarViewInterfaceBase;

/** A simple tool bar that can display bitmapped buttons and labels.  At the
    moment there are buttons for moving to the next and previous slide and
    to the next effect.  A label displays the index of the current slide
    and the total number of slides.
*/
class PresenterToolBar
    : private ::cppu::BaseMutex,
      public PresenterToolBarInterfaceBase,
      public CachablePresenterView
{
public:
    typedef ::std::function<void ()> Action;

    enum Anchor { Left, Center };

    PresenterToolBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        css::uno::Reference<css::awt::XWindow> xWindow,
        css::uno::Reference<css::rendering::XCanvas> xCanvas,
        ::rtl::Reference<PresenterController> pPresenterController,
        const Anchor eAnchor);
    virtual ~PresenterToolBar() override;
    PresenterToolBar(const PresenterToolBar&) = delete;
    PresenterToolBar& operator=(const PresenterToolBar&) = delete;

    void Initialize (
        const OUString& rsConfigurationPath);

    virtual void SAL_CALL disposing() override;

    void InvalidateArea (
        const css::awt::Rectangle& rRepaintBox,
        const bool bSynchronous);

    void RequestLayout();
    css::geometry::RealSize2D const & GetMinimalSize();
    const ::rtl::Reference<PresenterController>& GetPresenterController() const;
    const css::uno::Reference<css::uno::XComponentContext>& GetComponentContext() const;

    // lang::XEventListener

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

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide) override;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage() override;
    class Element;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;

    using ElementContainerPart = std::vector<rtl::Reference<Element>>;
    typedef std::shared_ptr<ElementContainerPart> SharedElementContainerPart;
    typedef ::std::vector<SharedElementContainerPart> ElementContainer;
    ElementContainer maElementContainer;
    SharedElementContainerPart mpCurrentContainerPart;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;
    ::rtl::Reference<PresenterController> mpPresenterController;
    bool mbIsLayoutPending;
    const Anchor meAnchor;
    /** The minimal size that is necessary to display all elements without
        overlap and with minimal gaps between them.
    */
    css::geometry::RealSize2D maMinimalSize;

    void CreateControls (
        const OUString& rsConfigurationPath);
    void Layout();
    css::geometry::RealSize2D CalculatePartSize(
        const SharedElementContainerPart& rpPart,
        const bool bIsHorizontal);
    void LayoutPart(
        const SharedElementContainerPart& rpPart,
        const css::geometry::RealRectangle2D& rBoundingBox,
        const css::geometry::RealSize2D& rPartSize,
        const bool bIsHorizontal);
    void Paint (
        const css::awt::Rectangle& rUpdateBox,
        const css::rendering::ViewState& rViewState);

    void UpdateSlideNumber();

    void CheckMouseOver (
        const css::awt::MouseEvent& rEvent,
        const bool bOverWindow,
        const bool bMouseDown=false);

    void ProcessEntry(const css::uno::Reference<css::beans::XPropertySet>& rProperties);

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed() const;
};

/** View for the PresenterToolBar.
*/
class PresenterToolBarView
    : public PresenterToolBarViewInterfaceBase
{
public:
    explicit PresenterToolBarView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const rtl::Reference<sd::framework::ResourceId>& rxViewId,
        const ::rtl::Reference<::sd::DrawController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterToolBarView() override;
    PresenterToolBarView(const PresenterToolBarView&) = delete;
    PresenterToolBarView& operator=(const PresenterToolBarView&) = delete;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    const ::rtl::Reference<PresenterToolBar>& GetPresenterToolBar() const;

    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent) override;

    // lang::XEventListener

    using WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject) override;

    // AbstractResource

    virtual rtl::Reference<sd::framework::ResourceId> getResourceId() override;

    virtual bool isAnchorOnly() override;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide) override;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage() override;

private:
    //    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    rtl::Reference<sd::framework::AbstractPane> mxPane;
    rtl::Reference<sd::framework::ResourceId> mxViewId;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    ::rtl::Reference<PresenterController> mpPresenterController;
    ::rtl::Reference<PresenterToolBar> mpToolBar;

};

} // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
