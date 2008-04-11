/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterToolBar.hxx,v $
 *
 * $Revision: 1.3 $
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

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase6.hxx>
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
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace {
    typedef cppu::WeakComponentImplHelper6<
        css::awt::XWindowListener,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener,
        css::drawing::framework::XView,
        css::drawing::XDrawView
        > PresenterToolBarInterfaceBase;
}

namespace sdext { namespace presenter {

class PresenterBitmapContainer;

/** A simple tool bar that can display bitmapped buttons and labels.  At the
    moment there are buttons for moving to the next and previous slide and
    to the next effect.  A label displayes the index of the current slide
    and the total number of slides.

    In the future this set of controls may be extended.  Declaration of the
    control set could be made more flexible and more easy (e.g. via the
    configuration to avoid the need to recompile this class.)
*/
class PresenterToolBar
    : private ::cppu::BaseMutex,
      public PresenterToolBarInterfaceBase
{
public:
    explicit PresenterToolBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterToolBar (void);

    virtual void SAL_CALL disposing (void);

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

    class Element;

private:

    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::drawing::framework::XPane> mxPane;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;

    typedef ::std::vector<boost::shared_ptr<Element> > ElementContainer;
    ElementContainer maElementContainer;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::awt::XWindow> mxControl;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    ::rtl::Reference<PresenterController> mpPresenterController;
    ::rtl::OUString msPreviousButtonBitmapURL;
    ::rtl::OUString msNextButtonBitmapURL;
    ::boost::scoped_ptr<PresenterBitmapContainer> mpIconContainer;

    void CreateControls (void);
    void Resize (void);
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

    void GotoPreviousSlide (void);
    void GotoNextSlide (void);
    void GotoNextEffect (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif
