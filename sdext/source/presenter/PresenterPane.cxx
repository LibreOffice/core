/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterPane.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:00:01 $
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

#include "PresenterPane.hxx"

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/drawing/CanvasFeature.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//----- Service ---------------------------------------------------------------

OUString PresenterPane::getImplementationName_static (void)
{
    return OUString::createFromAscii("com.sun.star.comp.Draw.PresenterPane");
}




Sequence<OUString> PresenterPane::getSupportedServiceNames_static (void)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.PresenterPane"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




Reference<XInterface> PresenterPane::Create (const Reference<uno::XComponentContext>& rxContext)
    SAL_THROW((css::uno::Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterPane(rxContext)));
}




//===== PresenterPane =========================================================

PresenterPane::PresenterPane (const Reference<XComponentContext>& rxContext)
    : PresenterPaneBase(rxContext)
{
    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    mxPresenterHelper = Reference<drawing::XPresenterHelper>(
        xFactory->createInstanceWithContext(
            OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper"),
            mxComponentContext),
        UNO_QUERY_THROW);
}




PresenterPane::~PresenterPane (void)
{
}




//----- XPane -----------------------------------------------------------------

Reference<awt::XWindow> SAL_CALL PresenterPane::getWindow (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxContentWindow;
}




Reference<rendering::XCanvas> SAL_CALL PresenterPane::getCanvas (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxContentCanvas;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterPane::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    LayoutContextWindow();
    ToTop();
}





void SAL_CALL PresenterPane::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    ToTop();
}




void SAL_CALL PresenterPane::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    ToTop();

    if (mxContentWindow.is())
    {
        LayoutContextWindow();
        mxContentWindow->setVisible(sal_True);
    }
}




void SAL_CALL PresenterPane::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    if (mxContentWindow.is())
        mxContentWindow->setVisible(sal_False);
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterPane::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    PaintBorder(rEvent.UpdateRect);
}




//-----------------------------------------------------------------------------


void PresenterPane::CreateCanvases (
    const Reference<awt::XWindow>& rxParentWindow,
    const Reference<rendering::XSpriteCanvas>& rxParentCanvas)
{
    if ( ! mxPresenterHelper.is())
        return;
    if ( ! rxParentWindow.is())
        return;
    if ( ! rxParentCanvas.is())
        return;

    mxBorderCanvas = mxPresenterHelper->createSharedCanvas(
        rxParentCanvas,
        rxParentWindow,
        Reference<rendering::XCanvas>(rxParentCanvas, UNO_QUERY),
        rxParentWindow,
        mxBorderWindow);
    mxContentCanvas = mxPresenterHelper->createSharedCanvas(
        rxParentCanvas,
        rxParentWindow,
        Reference<rendering::XCanvas>(rxParentCanvas, UNO_QUERY),
        rxParentWindow,
        mxContentWindow);

    PaintBorder(mxBorderWindow->getPosSize());
}




} } // end of namespace ::sd::presenter
