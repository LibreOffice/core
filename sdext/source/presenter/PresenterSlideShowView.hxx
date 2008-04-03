/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterSlideShowView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:04:51 $
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

#ifndef SDEXT_PRESENTER_SLIDE_SHOW_VIEW_HXX
#define SDEXT_PRESENTER_SLIDE_SHOW_VIEW_HXX

#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XPointer.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <cppuhelper/compbase7.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

namespace {
    typedef cppu::WeakComponentImplHelper7<
        css::presentation::XSlideShowView,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener,
        css::awt::XWindowListener,
        css::drawing::framework::XView,
        css::lang::XInitialization
        > PresenterSlideShowViewInterfaceBase;
}

/** Life view in a secondary windo of a full screen slide show.
*/
class PresenterSlideShowView
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public PresenterSlideShowViewInterfaceBase
{
public:
    PresenterSlideShowView (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterSlideShowView (void);

    virtual void SAL_CALL disposing (void);

    // Service.

    static ::rtl::OUString getImplementationName_static (void);
    static css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static (void);
    static css::uno::Reference<css::uno::XInterface> Create(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        SAL_THROW((css::uno::Exception));


    // XSlideShowView

    virtual css::uno::Reference<
        css::rendering::XSpriteCanvas > SAL_CALL getCanvas (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL clear (void)
        throw (css::uno::RuntimeException);

    virtual css::geometry::AffineMatrix2D SAL_CALL getTransformation (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeTransformationChangedListener(
        const css::uno::Reference<
            css::util::XModifyListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addPaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removePaintListener(
        const css::uno::Reference<
            css::awt::XPaintListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeMouseListener(
        const css::uno::Reference<
            css::awt::XMouseListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<
            css::awt::XMouseMotionListener >& xListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setMouseCursor(::sal_Int16 nPointerShape)
        throw (css::uno::RuntimeException);


    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
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

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
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


    // XView

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL
        getResourceId (void)
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);


    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::presentation::XSlideShow> mxSlideShow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::rendering::XCanvas> mxViewCanvas;
    css::uno::Reference<css::awt::XPointer> mxPointer;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::awt::XWindow> mxViewWindow;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxBackgroundPolyPolygon;

    /** Aspect ratio of the current slide.
    */
    double mnPageAspectRatio;

    /** This broadcast helper is used to notify listeners registed to a
        SlideShowView object.
    */
    ::cppu::OBroadcastHelper maBroadcaster;

    css::util::Color maBackgroundColor;

    bool mbIsInModifyNotification;
    bool mbIsForcedPaintPending;
    bool mbIsPaintPending;

    /** Create the window into which the slide show will render its
        content.  This window has the correct aspect ratio and is displayed centered
        and as large as possible in its parent window.
    */
    css::uno::Reference<css::awt::XWindow> CreateViewWindow (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow);

    void Resize (void);

    /** The slide show relies on the back buffer of the canvas not being
        modified.  With a shared canvas there are times when that can not be
        guaranteed.
        Call this method when the back buffer may have changed its content,
        like when the window has been moved but not resized.
    */
    void ForceRepaint (void);

    css::uno::Reference<css::rendering::XPolyPolygon2D> CreateBackgroundPolyPolygon (void) const;

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif
