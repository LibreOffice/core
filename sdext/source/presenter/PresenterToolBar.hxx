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

#ifndef SDEXT_PRESENTER_TOOL_BAR_HXX
#define SDEXT_PRESENTER_TOOL_BAR_HXX

#include "PresenterController.hxx"
#include "PresenterViewFactory.hxx"

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/compbase5.hxx>
#include <com/sun/star/awt/ActionEvent.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <map>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace {
    typedef cppu::WeakComponentImplHelper5<
        css::awt::XWindowListener,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener,
        css::drawing::XDrawView
        > PresenterToolBarInterfaceBase;

    typedef cppu::WeakComponentImplHelper3<
        css::awt::XPaintListener,
        css::drawing::framework::XView,
        css::drawing::XDrawView
        > PresenterToolBarViewInterfaceBase;
}

namespace sdext { namespace presenter {

/** A simple tool bar that can display bitmapped buttons and labels.  At the
    moment there are buttons for moving to the next and previous slide and
    to the next effect.  A label displayes the index of the current slide
    and the total number of slides.
*/
class PresenterToolBar
    : private ::cppu::BaseMutex,
      private ::boost::noncopyable,
      public PresenterToolBarInterfaceBase,
      public CachablePresenterView
{
public:
    typedef ::boost::function<void(void)> Action;

    enum Anchor { Left, Center, Right };

    PresenterToolBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const Anchor eAnchor);
    virtual ~PresenterToolBar (void);

    void Initialize (
        const ::rtl::OUString& rsConfigurationPath);

    virtual void SAL_CALL disposing (void);

    void InvalidateArea (
        const css::awt::Rectangle& rRepaintBox,
        const bool bSynchronous);
    sal_Int32 GetSlideCount (void);
    sal_Int32 GetCurrentSlideIndex (void);
    void RequestLayout (void);
    css::geometry::RealSize2D GetSize (void);
    css::geometry::RealSize2D GetMinimalSize (void);
    ::rtl::Reference<PresenterController> GetPresenterController (void) const;
    css::uno::Reference<css::awt::XWindow> GetWindow (void) const;
    css::uno::Reference<css::uno::XComponentContext> GetComponentContext (void) const;

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
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseMotionListener

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage (void)
        throw (css::uno::RuntimeException);

    class Context;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;

    class ElementContainerPart;
    typedef ::boost::shared_ptr<ElementContainerPart> SharedElementContainerPart;
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
    css::geometry::RealRectangle2D maBoundingBox;
    /** The minimal size that is necessary to display all elements without
        overlap and with minimal gaps between them.
    */
    css::geometry::RealSize2D maMinimalSize;

    void CreateControls (
        const ::rtl::OUString& rsConfigurationPath);
    void Layout (const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);
    css::geometry::RealSize2D CalculatePartSize (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const SharedElementContainerPart& rpPart,
        const bool bIsHorizontal);
    void LayoutPart (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const SharedElementContainerPart& rpPart,
        const css::geometry::RealRectangle2D& rBoundingBox,
        const css::geometry::RealSize2D& rPartSize,
        const bool bIsHorizontal);
    void Clear (
        const css::awt::Rectangle& rUpdateBox,
        const css::rendering::ViewState& rViewState);
    void Paint (
        const css::awt::Rectangle& rUpdateBox,
        const css::rendering::ViewState& rViewState);

    void UpdateSlideNumber (void);

    void CheckMouseOver (
        const css::awt::MouseEvent& rEvent,
        const bool bOverWindow,
        const bool bMouseDown=false);

    void ProcessEntry (
        const ::css::uno::Reference<css::beans::XPropertySet>& rProperties,
        Context& rContext);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};




/** View for the PresenterToolBar.
*/
class PresenterToolBarView
    : private ::cppu::BaseMutex,
      private ::boost::noncopyable,
      public PresenterToolBarViewInterfaceBase
{
public:
    explicit PresenterToolBarView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterToolBarView (void);

    virtual void SAL_CALL disposing (void);

    ::rtl::Reference<PresenterToolBar> GetPresenterToolBar (void) const;


    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException);


    // lang::XEventListener

    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException);


    // XResourceId

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId (void)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage (void)
        throw (css::uno::RuntimeException);

private:
    //    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::drawing::framework::XPane> mxPane;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    ::rtl::Reference<PresenterToolBar> mpToolBar;

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
