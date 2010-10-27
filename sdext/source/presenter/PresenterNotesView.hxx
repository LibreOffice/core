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

#ifndef SDEXT_PRESENTER_NOTES_VIEW2_HXX
#define SDEXT_PRESENTER_NOTES_VIEW2_HXX

#include "PresenterController.hxx"
#include "PresenterToolBar.hxx"
#include "PresenterViewFactory.hxx"
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase5.hxx>
#include <com/sun/star/awt/ActionEvent.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <rtl/ref.hxx>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace {
    typedef cppu::WeakComponentImplHelper5<
        css::awt::XWindowListener,
        css::awt::XPaintListener,
        css::drawing::framework::XView,
        css::drawing::XDrawView,
        css::awt::XKeyListener
        > PresenterNotesViewInterfaceBase;
}

namespace sdext { namespace presenter {

class PresenterButton;
class PresenterScrollBar;
class PresenterTextView;


/** A drawing framework view of the notes of a slide.  At the moment this is
    a simple text view that does not show the original formatting of the
    notes text.
*/
class PresenterNotesView
    : private ::cppu::BaseMutex,
      public PresenterNotesViewInterfaceBase,
      public CachablePresenterView
{
public:
    explicit PresenterNotesView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterNotesView (void);

    virtual void SAL_CALL disposing (void);

    /** Typically called from setCurrentSlide() with the notes page that is
        associed with the slide given to setCurrentSlide().

        Iterates over all text shapes on the given notes page and diplays
        the concatenated text of these.
    */
    void SetSlide (
        const css::uno::Reference<css::drawing::XDrawPage>& rxNotesPage);

    void ChangeFontSize (const sal_Int32 nSizeChange);

    ::boost::shared_ptr<PresenterTextView> GetTextView (void) const;


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


    // XKeyListener

    virtual void SAL_CALL keyPressed (const css::awt::KeyEvent& rEvent)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL keyReleased (const css::awt::KeyEvent& rEvent)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentNotesPage;
    ::rtl::Reference<PresenterScrollBar> mpScrollBar;
    css::uno::Reference<css::awt::XWindow> mxToolBarWindow;
    css::uno::Reference<css::rendering::XCanvas> mxToolBarCanvas;
    ::rtl::Reference<PresenterToolBar> mpToolBar;
    ::rtl::Reference<PresenterButton> mpCloseButton;
    css::util::Color maSeparatorColor;
    sal_Int32 mnSeparatorYLocation;
    css::geometry::RealRectangle2D maTextBoundingBox;
    SharedBitmapDescriptor mpBackground;
    double mnTop;
    PresenterTheme::SharedFontDescriptor mpFont;
    ::boost::shared_ptr<PresenterTextView> mpTextView;

    void CreateToolBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    void Layout (void);
    void Paint (const css::awt::Rectangle& rUpdateBox);
    void PaintToolBar (const css::awt::Rectangle& rUpdateBox);
    void PaintText (const css::awt::Rectangle& rUpdateBox);
    void Invalidate (void);
    void Scroll (const double nDistance);
    void SetTop (const double nTop);
    void UpdateScrollBar (void);
    void MoveCaret (const sal_Int32 nDistance);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
