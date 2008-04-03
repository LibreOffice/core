/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterNotesView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:59:50 $
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

#ifndef SDEXT_PRESENTER_NOTES_VIEW2_HXX
#define SDEXT_PRESENTER_NOTES_VIEW2_HXX

#include "PresenterController.hxx"

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
#include <boost/scoped_ptr.hpp>

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

class PresenterScrollBar;

/** A drawing framework view of the notes of a slide.  At the moment this is
    a simple text view that does not show the original formatting of the
    notes text.
*/
class PresenterNotesView
    : private ::cppu::BaseMutex,
      public PresenterNotesViewInterfaceBase
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

    css::uno::Reference<css::awt::XWindow> GetWindow (void) const;

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
    css::uno::Reference<css::beans::XPropertySet> mxTextView;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::rendering::XBitmap> mxBitmap;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentNotesPage;
    css::awt::FontDescriptor maFontDescriptor;
    ::rtl::Reference<PresenterScrollBar> mpScrollBar;

    void Resize (void);
    void Paint (void);
    void Invalidate (void);
    void Scroll (const ::rtl::OUString& rsDistance);
    void SetTop (const double nTop);
    void UpdateScrollBar (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif
