/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterSlidePreview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:04:19 $
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

#include "PresenterSlidePreview.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace
{
    // Use a super sample factor greater than 1 to achive a poor mans
    // antialiasing effect for slide previews.
    const sal_Int16 gnSuperSampleFactor = 2;
}

namespace sdext { namespace presenter {

//===== PresenterSlidePreview =================================================

PresenterSlidePreview::PresenterSlidePreview (
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<XPane>& rxAnchorPane,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterSlidePreviewInterfaceBase(m_aMutex),
      mpPresenterController(rpPresenterController),
      mxPane(rxAnchorPane),
      mxViewId(rxViewId),
      mxPreviewRenderer(),
      mxPreview(),
      mxCurrentSlide(),
      mxWindow(),
      mxCanvas()
{
    if ( ! rxContext.is()
        || ! rxViewId.is()
        || ! rxAnchorPane.is()
        || ! rpPresenterController.is())
    {
        throw RuntimeException(
            OUString::createFromAscii(
                "PresenterSlidePreview can not be constructed due to empty argument"),
            static_cast<XWeak*>(this));
    }

    mxWindow = rxAnchorPane->getWindow();
    mxCanvas = rxAnchorPane->getCanvas();

    if (mxWindow.is())
    {
        mxWindow->addWindowListener(this);
        mxWindow->addPaintListener(this);

        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->setBackground(util::Color(0xff000000));

        mxWindow->setVisible(sal_True);
    }


    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager(), UNO_QUERY);
    if (xFactory.is())
        mxPreviewRenderer = Reference<drawing::XSlideRenderer>(
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.drawing.SlideRenderer"),
                rxContext),
            UNO_QUERY);

    Resize();
}




PresenterSlidePreview::~PresenterSlidePreview (void)
{
}




void SAL_CALL PresenterSlidePreview::disposing (void)
{
    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
        mxWindow = NULL;
        mxCanvas = NULL;
    }

    Reference<lang::XComponent> xComponent (mxPreviewRenderer, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}




//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterSlidePreview::getResourceId (void)
    throw (RuntimeException)
{
    return mxViewId;
}




sal_Bool SAL_CALL PresenterSlidePreview::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSlidePreview::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    Resize();
}





void SAL_CALL PresenterSlidePreview::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterSlidePreview::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    Resize();
}




void SAL_CALL PresenterSlidePreview::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSlidePreview::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    if (mxWindow.is())
        Paint(awt::Rectangle(
            rEvent.UpdateRect.X,
            rEvent.UpdateRect.Y,
            rEvent.UpdateRect.Width,
            rEvent.UpdateRect.Height));
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterSlidePreview::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxWindow)
    {
        mxWindow = NULL;
        mxCanvas = NULL;
        mxPreview = NULL;
    }
}




//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterSlidePreview::setCurrentPage (const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    SetSlide(rxSlide);
}




Reference<drawing::XDrawPage> SAL_CALL PresenterSlidePreview::getCurrentPage (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return NULL;
}




//-----------------------------------------------------------------------------

void PresenterSlidePreview::SetSlide (const Reference<drawing::XDrawPage>& rxPage)
{
    mxCurrentSlide = rxPage;
    mxPreview = NULL;

    Reference<beans::XPropertySet> xPropertySet (mxCurrentSlide, UNO_QUERY);
    if (xPropertySet.is())
    {
        awt::Size aSlideSize;
        try
        {
            xPropertySet->getPropertyValue(
                OUString::createFromAscii("Width")) >>= aSlideSize.Width;
            xPropertySet->getPropertyValue(
                OUString::createFromAscii("Height")) >>= aSlideSize.Height;
        }
        catch (beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }
        if (aSlideSize.Height > 0)
            mnSlideAspectRatio = double(aSlideSize.Width) / double(aSlideSize.Height);
    }

    Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
    if (xPeer.is())
        xPeer->invalidate(awt::InvalidateStyle::NOERASE);
}




void PresenterSlidePreview::Paint (const awt::Rectangle& rBoundingBox)
{
    (void)rBoundingBox;
    if ( ! mxWindow.is())
        return;
    if ( ! mxCanvas.is())
        return;
    if ( ! mxPreviewRenderer.is())
        return;

    // Make sure that a preview in the correct size exists.
    awt::Rectangle aWindowBox (mxWindow->getPosSize());

    if ( ! mxPreview.is() && mxCurrentSlide.is())
    {
        // Create a new preview bitmap.
        mxPreview = mxPreviewRenderer->createPreviewForCanvas(
            mxCurrentSlide,
            awt::Size(aWindowBox.Width, aWindowBox.Height),
            gnSuperSampleFactor,
            mxCanvas);
    }

    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);

    // Paint the background
    {
        util::Color aColor (
            mpPresenterController->GetViewBackgroundColor(mxViewId->getResourceURL()));
        Sequence<double> aBackgroundColor(3);
        aBackgroundColor[0] = ((aColor >> 16) & 0x0ff) / 255.0;
        aBackgroundColor[1] = ((aColor >> 8) & 0x0ff) / 255.0;
        aBackgroundColor[2] = ((aColor >> 0) & 0x0ff) / 255.0;
        const rendering::RenderState aRenderState (
            geometry::AffineMatrix2D(1, 0, 0, 0, 1, 0),
            NULL,
            aBackgroundColor,
            rendering::CompositeOperation::SOURCE);

        Sequence<Sequence<geometry::RealPoint2D> > aBox(1);
        aBox[0] = Sequence<geometry::RealPoint2D>(4);
        aBox[0][0] = geometry::RealPoint2D(0,0);
        aBox[0][1] = geometry::RealPoint2D(aWindowBox.Width, 0);
        aBox[0][2] = geometry::RealPoint2D(aWindowBox.Width, aWindowBox.Height);
        aBox[0][3] = geometry::RealPoint2D(0, aWindowBox.Height);
        Reference<rendering::XPolyPolygon2D> xPolygon (
            mxCanvas->getDevice()->createCompatibleLinePolyPolygon(aBox),
            UNO_QUERY);
        if (xPolygon.is())
        {
            xPolygon->setClosed(0, sal_True);
            mxCanvas->fillPolyPolygon(xPolygon, aViewState, aRenderState);
        }
    }

    // Paint the preview.
    Sequence<double> aBackgroundColor(3);
    aBackgroundColor[0] = 0;
    aBackgroundColor[1] = 0;
    aBackgroundColor[2] = 0;
    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1, 0, 0, 0, 1, 0),
        NULL,
        aBackgroundColor,
        rendering::CompositeOperation::SOURCE);
    if (mxPreview.is())
    {
        const geometry::IntegerSize2D aPreviewSize (mxPreview->getSize());
        aRenderState.AffineTransform.m02 = (aWindowBox.Width - aPreviewSize.Width)/2;
        aRenderState.AffineTransform.m12 = (aWindowBox.Height - aPreviewSize.Height)/2;
        mxCanvas->drawBitmap(mxPreview, aViewState, aRenderState);
    }
    else
    {
        if (mnSlideAspectRatio > 0)
        {
            const awt::Size aPreviewSize (mxPreviewRenderer->calculatePreviewSize(
                mnSlideAspectRatio,awt::Size(aWindowBox.Width, aWindowBox.Height)));
            aRenderState.AffineTransform.m02 = (aWindowBox.Width - aPreviewSize.Width)/2;
            aRenderState.AffineTransform.m12 = (aWindowBox.Height - aPreviewSize.Height)/2;
            Sequence<Sequence<geometry::RealPoint2D> > aBox(1);
            aBox[0] = Sequence<geometry::RealPoint2D>(4);
            aBox[0][0] = geometry::RealPoint2D(0,0);
            aBox[0][1] = geometry::RealPoint2D(aPreviewSize.Width, 0);
            aBox[0][2] = geometry::RealPoint2D(aPreviewSize.Width, aPreviewSize.Height);
            aBox[0][3] = geometry::RealPoint2D(0, aPreviewSize.Height);
            Reference<rendering::XPolyPolygon2D> xPolygon (
                mxCanvas->getDevice()->createCompatibleLinePolyPolygon(aBox),
                UNO_QUERY);
            if (xPolygon.is())
            {
                xPolygon->setClosed(0, sal_False);
                mxCanvas->fillPolyPolygon(xPolygon, aViewState, aRenderState);
            }
        }
    }

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}




void PresenterSlidePreview::Resize (void)
{
    if (mxPreviewRenderer.is() && mxPreview.is())
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        const awt::Size aNewPreviewSize (mxPreviewRenderer->calculatePreviewSize(
            mnSlideAspectRatio,
                awt::Size(aWindowBox.Width, aWindowBox.Height)));
        const geometry::IntegerSize2D aPreviewSize (mxPreview->getSize());
        if (aNewPreviewSize.Width==aPreviewSize.Width
            && aNewPreviewSize.Height==aPreviewSize.Height)
        {
            // The size of the window may have changed but the preview would
            // be painted in the same size (but not necessarily at the same
            // position.)
            return;
        }
    }
    SetSlide(mxCurrentSlide);
}




void PresenterSlidePreview::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (PresenterSlidePreviewInterfaceBase::rBHelper.bDisposed || PresenterSlidePreviewInterfaceBase::rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterSlidePreview object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}


} } // end of namespace ::sd::presenter

