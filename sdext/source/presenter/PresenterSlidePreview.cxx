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

#include "PresenterSlidePreview.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterScrollBar.hxx"
#include "PresenterBitmapContainer.hxx"
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

namespace
{
    // Use a super sample factor greater than 1 to achieve a poor mans
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
            "PresenterSlidePreview can not be constructed due to empty argument",
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

        mxWindow->setVisible(true);
    }

    if (mpPresenterController.get() != nullptr)
        mnSlideAspectRatio = mpPresenterController->GetSlideAspectRatio();

    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager(), UNO_QUERY);
    if (xFactory.is())
        mxPreviewRenderer.set(
            xFactory->createInstanceWithContext(
                "com.sun.star.drawing.SlideRenderer",
                rxContext),
            UNO_QUERY);
    mpBitmaps.reset(new PresenterBitmapContainer(
            OUString("PresenterScreenSettings/ScrollBar/Bitmaps"),
            std::shared_ptr<PresenterBitmapContainer>(),
            rxContext,
            mxCanvas));
    Resize();
}

PresenterSlidePreview::~PresenterSlidePreview()
{
}

void SAL_CALL PresenterSlidePreview::disposing()
{
    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
        mxWindow = nullptr;
        mxCanvas = nullptr;
    }

    Reference<lang::XComponent> xComponent (mxPreviewRenderer, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterSlidePreview::getResourceId()
{
    return mxViewId;
}

sal_Bool SAL_CALL PresenterSlidePreview::isAnchorOnly()
{
    return false;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSlidePreview::windowResized (const awt::WindowEvent&)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    Resize();
}

void SAL_CALL PresenterSlidePreview::windowMoved (const awt::WindowEvent&) {}

void SAL_CALL PresenterSlidePreview::windowShown (const lang::EventObject&)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    Resize();
}

void SAL_CALL PresenterSlidePreview::windowHidden (const lang::EventObject&) {}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSlidePreview::windowPaint (const awt::PaintEvent& rEvent)
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
{
    if (rEvent.Source == mxWindow)
    {
        mxWindow = nullptr;
        mxCanvas = nullptr;
        mxPreview = nullptr;
    }
}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterSlidePreview::setCurrentPage (const Reference<drawing::XDrawPage>& rxSlide)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
    SetSlide(rxSlide);
}

Reference<drawing::XDrawPage> SAL_CALL PresenterSlidePreview::getCurrentPage()
{
    ThrowIfDisposed();
    return nullptr;
}


void PresenterSlidePreview::SetSlide (const Reference<drawing::XDrawPage>& rxPage)
{
    mxCurrentSlide = rxPage;
    mxPreview = nullptr;

    // The preview is not transparent, therefore only this window, not its
    // parent, has to be invalidated.
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
}

void PresenterSlidePreview::Paint (const awt::Rectangle& rBoundingBox)
{
    if ( ! mxWindow.is())
        return;
    if ( ! mxCanvas.is())
        return;
    if ( ! mxPreviewRenderer.is())
        return;

    // Make sure that a preview in the correct size exists.
    awt::Rectangle aWindowBox (mxWindow->getPosSize());

    bool bCustomAnimation = false;
    bool bTransition = false;
    if( mxCurrentSlide.is() )
    {
        bCustomAnimation = PresenterController::HasCustomAnimation(mxCurrentSlide);
        bTransition = PresenterController::HasTransition(mxCurrentSlide);
    }

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
        nullptr);

    Sequence<double> aBackgroundColor(4);
    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1, 0, aPreviewBox.X, 0, 1, aPreviewBox.Y),
        nullptr,
        aBackgroundColor,
        rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, 0x00000000);
    if (mxPreview.is())
    {
        mxCanvas->drawBitmap(mxPreview, aViewState, aRenderState);
        if( bTransition )
        {
            const awt::Rectangle aTransitionPreviewBox(5, aWindowBox.Height-20, 0, 0);
            SharedBitmapDescriptor aTransitionDescriptor = mpBitmaps->GetBitmap("Transition");
            Reference<rendering::XBitmap> xTransitionIcon (aTransitionDescriptor->GetNormalBitmap());
            rendering::RenderState aTransitionRenderState (
                geometry::AffineMatrix2D(1, 0, aTransitionPreviewBox.X, 0, 1, aTransitionPreviewBox.Y),
                nullptr,
                aBackgroundColor,
                rendering::CompositeOperation::SOURCE);
            mxCanvas->drawBitmap(xTransitionIcon, aViewState, aTransitionRenderState);
        }
        if( bCustomAnimation )
        {
            const awt::Rectangle aAnimationPreviewBox(5, aWindowBox.Height-40, 0, 0);
            SharedBitmapDescriptor aAnimationDescriptor = mpBitmaps->GetBitmap("Animation");
            Reference<rendering::XBitmap> xAnimationIcon (aAnimationDescriptor->GetNormalBitmap());
            rendering::RenderState aAnimationRenderState (
                geometry::AffineMatrix2D(1, 0, aAnimationPreviewBox.X, 0, 1, aAnimationPreviewBox.Y),
                nullptr,
                aBackgroundColor,
                rendering::CompositeOperation::SOURCE);
            mxCanvas->drawBitmap(xAnimationIcon, aViewState, aAnimationRenderState);
        }
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
        xSpriteCanvas->updateScreen(false);
}

void PresenterSlidePreview::Resize()
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

void PresenterSlidePreview::ThrowIfDisposed()
{
    if (PresenterSlidePreviewInterfaceBase::rBHelper.bDisposed || PresenterSlidePreviewInterfaceBase::rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterSlidePreview object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
