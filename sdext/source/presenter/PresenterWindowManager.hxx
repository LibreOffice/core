/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterWindowManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:07:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SDEXT_PRESENTER_PRESENTER_WINDOW_MANAGER_HXX
#define SDEXT_PRESENTER_PRESENTER_WINDOW_MANAGER_HXX

#include "PresenterPaneContainer.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XSprite.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <tools/gen.hxx>
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
    void NotifyPaneCreation (const PresenterPaneContainer::SharedPaneDescriptor& rpDescriptor);
    void NotifyViewCreation (const css::uno::Reference<css::drawing::framework::XView>& rxView);
    void SetPanePosSizeRelative (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const double nRelativeX,
        const double nRelativeY,
        const double nRelativeWidth,
        const double nRelativeHeight);
    void SetPanePosSizeAbsolute (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const double nX,
        const double nY,
        const double nWidth,
        const double nHeight);
    void SetPaneBorderPainter (const ::rtl::Reference<PresenterPaneBorderPainter>& rPainter);
    css::uno::Reference<css::awt::XWindow> GetParentWindow (void) const;
    css::uno::Reference<css::rendering::XCanvas> GetParentCanvas (void) const;
    void Update (void);
    void Layout (void);

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
    css::uno::Reference<css::rendering::XBitmap> mxBackgroundBitmap;
    css::util::Color maBackgroundColor;
    css::util::Color maPaneBackgroundColor;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxClipPolygon;

    void PaintChildren (const css::awt::PaintEvent& rEvent) const;
    void UpdateWindowSize (const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);
    void PaintBackground (const css::awt::Rectangle& rUpdateBox);
    css::uno::Reference<css::rendering::XPolyPolygon2D> CreateClipPolyPolygon (void) const;
    void ToTop ();

    void UpdateWindowList (void);
    bool ChildNeedsRepaint (
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane) const;

    void Invalidate (void);

    void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
};

} }

#endif
