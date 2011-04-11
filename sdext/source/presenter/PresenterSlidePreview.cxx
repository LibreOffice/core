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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterSlidePreview.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
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
      mnSlideAspectRatio(28.0 / 21.0),
      mxWindow(),
      mxCanvas()
{
    if ( ! rxContext.is()
        || ! rxViewId.is()
        || ! rxAnchorPane.is()
        || ! rpPresenterController.is())
    {
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterSlidePreview can not be constructed due to empty argument")),
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

    if (mpPresenterController.get() != NULL)
        mnSlideAspectRatio = mpPresenterController->GetSlideAspectRatio();

    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager(), UNO_QUERY);
    if (xFactory.is())
        mxPreviewRenderer = Reference<drawing::XSlideRenderer>(
            xFactory->createInstanceWithContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.SlideRenderer")),
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
                OUString(RTL_CONSTASCII_USTRINGPARAM("Width"))) >>= aSlideSize.Width;
            xPropertySet->getPropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Height"))) >>= aSlideSize.Height;
        }
        catch (beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }
    }

    // The preview is not transparent, therefore only this window, not its
    // parent, has to be invalidated.
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
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

    // Determine the bounding box of the preview.
    awt::Rectangle aPreviewBox;
    if (mxPreview.is())
    {
        const geometry::IntegerSize2D aPreviewSize (mxPreview->getSize());
        aPreviewBox = awt::Rectangle(
            (aWindowBox.Width - aPreviewSize.Width)/2,
            (aWindowBox.Height - aPreviewSize.Height)/2,
            aPreviewSize.Width,
            aPreviewSize.Height);
    }
    else
    {
        if (mnSlideAspectRatio > 0)
        {
            const awt::Size aPreviewSize (mxPreviewRenderer->calculatePreviewSize(
                mnSlideAspectRatio,awt::Size(aWindowBox.Width, aWindowBox.Height)));
            aPreviewBox = awt::Rectangle(
                (aWindowBox.Width - aPreviewSize.Width)/2,
                (aWindowBox.Height - aPreviewSize.Height)/2,
                aPreviewSize.Width,
                aPreviewSize.Height);
        }
    }

    // Paint the background.
    mpPresenterController->GetCanvasHelper()->Paint(
        mpPresenterController->GetViewBackground(mxViewId->getResourceURL()),
        mxCanvas,
        rBoundingBox,
        awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height),
        aPreviewBox);

    // Paint the preview.
    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);

    Sequence<double> aBackgroundColor(4);
    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1, 0, aPreviewBox.X, 0, 1, aPreviewBox.Y),
        NULL,
        aBackgroundColor,
        rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, 0x00000000);
    if (mxPreview.is())
    {
        mxCanvas->drawBitmap(mxPreview, aViewState, aRenderState);
    }
    else
    {
        if (mnSlideAspectRatio > 0)
        {
            Reference<rendering::XPolyPolygon2D> xPolygon (
                PresenterGeometryHelper::CreatePolygon(aPreviewBox, mxCanvas->getDevice()));
            if (xPolygon.is())
                mxCanvas->fillPolyPolygon(xPolygon, aViewState, aRenderState);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
