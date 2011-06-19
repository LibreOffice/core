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

#ifndef SDEXT_PRESENTER_PRESENTER_SLIDE_SORTER_HXX
#define SDEXT_PRESENTER_PRESENTER_SLIDE_SORTER_HXX

#include "PresenterController.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterViewFactory.hxx"
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase8.hxx>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XSlidePreviewCache.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/rendering/XSprite.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>

namespace css = ::com::sun::star;

namespace {
    typedef cppu::WeakComponentImplHelper8<
        css::drawing::framework::XView,
        css::awt::XWindowListener,
        css::awt::XPaintListener,
        css::beans::XPropertyChangeListener,
        css::drawing::XSlidePreviewCacheListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener,
        css::drawing::XDrawView
        > PresenterSlideSorterInterfaceBase;
}

namespace sdext { namespace presenter {

class PresenterButton;
class PresenterScrollBar;

/** A simple slide sorter for the presenter screen.  It uses a preview cache
    to create the slide previews.  Painting is done via a canvas.
*/
class PresenterSlideSorter
    : private ::cppu::BaseMutex,
      public PresenterSlideSorterInterfaceBase,
      public CachablePresenterView
{
public:
    PresenterSlideSorter (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterSlideSorter (void);

    virtual void SAL_CALL disposing (void);

    void SetActiveState (const bool bIsActive);


    // lang::XEventListener

    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject)
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


    // XResourceId

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId (void)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);


    // XPropertyChangeListener

    virtual void SAL_CALL propertyChange (
        const css::beans::PropertyChangeEvent& rEvent)
        throw(css::uno::RuntimeException);


    // XSlidePreviewCacheListener

    virtual void SAL_CALL notifyPreviewCreation (
        sal_Int32 nSlideIndex)
        throw(css::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage (void)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::drawing::framework::XPane> mxPane;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::drawing::XSlidePreviewCache> mxPreviewCache;
    bool mbIsPaintPending;
    bool mbIsLayoutPending;
    class Layout;
    ::boost::shared_ptr<Layout> mpLayout;
    ::rtl::Reference<PresenterScrollBar> mpHorizontalScrollBar;
    ::rtl::Reference<PresenterScrollBar> mpVerticalScrollBar;
    ::rtl::Reference<PresenterButton> mpCloseButton;
    class MouseOverManager;
    ::boost::scoped_ptr<MouseOverManager> mpMouseOverManager;
    sal_Int32 mnSlideIndexMousePressed;
    sal_Int32 mnCurrentSlideIndex;
    sal_Int32 mnSeparatorY;
    css::util::Color maSeparatorColor;
    css::awt::Point maCloseButtonCenter;
    css::awt::Rectangle maCurrentSlideFrameBoundingBox;
    class CurrentSlideFrameRenderer;
    ::boost::shared_ptr<CurrentSlideFrameRenderer> mpCurrentSlideFrameRenderer;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxPreviewFrame;

    void UpdateLayout (void);
    css::geometry::RealRectangle2D PlaceScrollBars (
        const css::geometry::RealRectangle2D& rUpperBox);
    void PlaceCloseButton (
        const PresenterPaneContainer::SharedPaneDescriptor& rpPane,
        const css::awt::Rectangle& rCenterBox,
    const sal_Int32 nLeftFrameWidth);
    void ClearBackground (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rRedrawArea);
    double GetSlideAspectRatio (void) const;
    css::uno::Reference<css::rendering::XBitmap> GetPreview (const sal_Int32 nSlideIndex);
    void PaintPreview (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rUpdateBox,
        const sal_Int32 nSlideIndex);
    void Paint (const css::awt::Rectangle& rUpdateBox);
    void SetHorizontalOffset (const double nXOffset);
    void SetVerticalOffset (const double nYOffset);
    void GotoSlide (const sal_Int32 nSlideIndex);
    bool ProvideCanvas (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
