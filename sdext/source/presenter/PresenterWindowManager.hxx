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

#ifndef SDEXT_PRESENTER_PRESENTER_WINDOW_MANAGER_HXX
#define SDEXT_PRESENTER_PRESENTER_WINDOW_MANAGER_HXX

#include "PresenterPaneContainer.hxx"
#include "PresenterTheme.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XSprite.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <rtl/ref.hxx>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterController;
class PresenterPaneBorderPainter;
class PresenterTheme;

namespace {
    typedef ::cppu::WeakComponentImplHelper4<
        css::awt::XWindowListener,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XFocusListener
    > PresenterWindowManagerInterfaceBase;
}

/** A simple manager of the positions of the panes of the presenter screen.
    Uses relative coordinates of the four sides of each pane.  Allows panes
    to be moved or resized with the mouse.
*/
class PresenterWindowManager
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public PresenterWindowManagerInterfaceBase
{
public:
    PresenterWindowManager (
        const ::css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterWindowManager (void);

    void SAL_CALL disposing (void);

    void SetParentPane (const css::uno::Reference<css::drawing::framework::XPane>& rxPane);
    void SetTheme (const ::boost::shared_ptr<PresenterTheme>& rpTheme);
    void NotifyViewCreation (const css::uno::Reference<css::drawing::framework::XView>& rxView);
    void SetPanePosSizeAbsolute (
        const ::rtl::OUString& rsPaneURL,
        const double nX,
        const double nY,
        const double nWidth,
        const double nHeight);
    void SetPaneBorderPainter (const ::rtl::Reference<PresenterPaneBorderPainter>& rPainter);
    void Update (void);
    void Layout (void);

    void SetSlideSorterState (bool bIsActive);
    void SetHelpViewState (bool bIsActive);

    enum LayoutMode { LM_Standard, LM_Notes, LM_Generic };
private:
    void SetLayoutMode (const LayoutMode eMode);

public:
    enum ViewMode { VM_Standard, VM_Notes, VM_SlideOverview, VM_Help };
    /** The high-level method to switch the view mode.  Use this instead of
        SetLayoutMode and Set(Help|SlideSorter)State when possible.
    */
    void SetViewMode (const ViewMode eMode);

    ViewMode GetViewMode (void) const;

    /** Restore the layout mode (or slide sorter state) from the
        configuration.
    */
    void RestoreViewMode (void);

    void AddLayoutListener (
        const css::uno::Reference<css::document::XEventListener>& rxListener);
    void RemoveLayoutListener (
        const css::uno::Reference<css::document::XEventListener>& rxListener);

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

    // XFocusListener

    virtual void SAL_CALL focusGained (const css::awt::FocusEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL focusLost (const css::awt::FocusEvent& rEvent)
        throw (css::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::rendering::XCanvas> mxParentCanvas;
    css::uno::Reference<css::uno::XInterface> mxPaneBorderManager;
    ::rtl::Reference<PresenterPaneBorderPainter> mpPaneBorderPainter;
    ::rtl::Reference<PresenterPaneContainer> mpPaneContainer;
    bool mbIsLayoutPending;
    /** This flag is set to <TRUE/> while the Layout() method is being
        executed.  Prevents windowMoved() and windowResized() from changing
        the window sizes.
    */
    bool mbIsLayouting;
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    SharedBitmapDescriptor mpBackgroundBitmap;
    css::uno::Reference<css::rendering::XBitmap> mxScaledBackgroundBitmap;
    css::util::Color maPaneBackgroundColor;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxClipPolygon;
    LayoutMode meLayoutMode;
    bool mbIsSlideSorterActive;
    bool mbIsHelpViewActive;
    typedef ::std::vector<css::uno::Reference<css::document::XEventListener> >
        LayoutListenerContainer;
    LayoutListenerContainer maLayoutListeners;
    bool mbIsMouseClickPending;

    bool PaintChildren (const css::awt::PaintEvent& rEvent) const;
    void UpdateWindowSize (const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);
    void PaintBackground (const css::awt::Rectangle& rUpdateBox);
    void ProvideBackgroundBitmap (void);
    css::uno::Reference<css::rendering::XPolyPolygon2D> CreateClipPolyPolygon (void) const;
    void ToTop ();

    void UpdateWindowList (void);
    bool ChildNeedsRepaint (
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane) const;

    void Invalidate (void);

    void StoreViewMode (const ViewMode eViewMode);

    void LayoutStandardMode (void);
    void LayoutNotesMode (void);
    void LayoutSlideSorterMode (void);
    void LayoutHelpMode (void);

    /** Layout the tool bar and return its outer bounding box.
    */
    css::geometry::RealRectangle2D LayoutToolBar (void);

    css::awt::Size CalculatePaneSize (
        const double nOuterWidth,
        const ::rtl::OUString& rsPaneURL);

    /** Notify changes of the layout mode and of the slide sorter state.
    */
    void NotifyLayoutModeChange (void);

    void NotifyDisposing (void);

    void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
