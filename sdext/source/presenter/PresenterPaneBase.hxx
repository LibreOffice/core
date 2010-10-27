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

#ifndef SD_PRESENTER_PRESENTER_PANE_BASE_HXX
#define SD_PRESENTER_PRESENTER_PANE_BASE_HXX

#include "PresenterTheme.hxx"
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XPaneBorderPainter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <rtl/ref.hxx>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace sdext { namespace presenter {

class PresenterController;
class PresenterTextView;

namespace {
    typedef ::cppu::WeakComponentImplHelper4 <
        css::drawing::framework::XPane,
        css::lang::XInitialization,
        css::awt::XWindowListener,
        css::awt::XPaintListener
    > PresenterPaneBaseInterfaceBase;
}


/** Base class of the panes used by the presenter screen.  Pane objects are
    stored in the PresenterPaneContainer.  Sizes and positions are
    controlled by the PresenterWindowManager.  Interactive positioning and
    resizing is managed by the PresenterPaneBorderManager.  Borders around
    panes are painted by the PresenterPaneBorderPainter.
*/
class PresenterPaneBase
    : protected ::cppu::BaseMutex,
      private ::boost::noncopyable,
      public PresenterPaneBaseInterfaceBase
{
public:
    PresenterPaneBase (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterPaneBase (void);

    virtual void SAL_CALL disposing (void);

    css::uno::Reference<css::awt::XWindow> GetBorderWindow (void) const;
    void SetBackground (const SharedBitmapDescriptor& rpBackground);
    void SetTitle (const ::rtl::OUString& rsTitle);
    ::rtl::OUString GetTitle (void) const;
    css::uno::Reference<css::drawing::framework::XPaneBorderPainter> GetPaneBorderPainter (void) const;
    void SetCalloutAnchor (const css::awt::Point& rAnchorPosition);
    css::awt::Point GetCalloutAnchor (void) const;

    ::boost::shared_ptr<PresenterTextView> GetTextViewForTitle (void);

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


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


    // lang::XEventListener

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


protected:
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::awt::XWindow> mxBorderWindow;
    css::uno::Reference<css::rendering::XCanvas> mxBorderCanvas;
    css::uno::Reference<css::awt::XWindow> mxContentWindow;
    css::uno::Reference<css::rendering::XCanvas> mxContentCanvas;
    css::uno::Reference<css::drawing::framework::XResourceId> mxPaneId;
    css::uno::Reference<css::drawing::framework::XPaneBorderPainter> mxBorderPainter;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    ::rtl::OUString msTitle;
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    SharedBitmapDescriptor mpViewBackground;
    bool mbHasCallout;
    css::awt::Point maCalloutAnchor;

    virtual void CreateCanvases (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas) = 0;

    void CreateWindows (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const bool bIsWindowVisibleOnCreation);
    void PaintBorderBackground (
        const css::awt::Rectangle& rCenterBox,
        const css::awt::Rectangle& rUpdateBox);
    void PaintBorder (const css::awt::Rectangle& rUpdateRectangle);
    void ToTop (void);
    void LayoutContextWindow (void);
    bool IsVisible (void) const;

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
