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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERNOTESVIEW_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERNOTESVIEW_HXX

#include "PresenterController.hxx"
#include "PresenterToolBar.hxx"
#include "PresenterViewFactory.hxx"
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
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
#include <memory>

namespace {
    typedef cppu::WeakComponentImplHelper<
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
    virtual ~PresenterNotesView();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    /** Typically called from setCurrentSlide() with the notes page that is
        associeted with the slide given to setCurrentSlide().

        Iterates over all text shapes on the given notes page and diplays
        the concatenated text of these.
    */
    void SetSlide (
        const css::uno::Reference<css::drawing::XDrawPage>& rxNotesPage);

    void ChangeFontSize (const sal_Int32 nSizeChange);

    std::shared_ptr<PresenterTextView> GetTextView() const;

    // lang::XEventListener

    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject)
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

    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XResourceId

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XDrawView

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XKeyListener

    virtual void SAL_CALL keyPressed (const css::awt::KeyEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL keyReleased (const css::awt::KeyEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
    std::shared_ptr<PresenterTextView> mpTextView;

    void CreateToolBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    void Layout();
    void Paint (const css::awt::Rectangle& rUpdateBox);
    void PaintToolBar (const css::awt::Rectangle& rUpdateBox);
    void PaintText (const css::awt::Rectangle& rUpdateBox);
    void Invalidate();
    void Scroll (const double nDistance);
    void SetTop (const double nTop);
    void UpdateScrollBar();

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed()
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
