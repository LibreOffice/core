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

#include "PresenterSpritePane.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext { namespace presenter {

//===== PresenterSpritePane =========================================================

PresenterSpritePane::PresenterSpritePane (const Reference<XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterPaneBase(rxContext, rpPresenterController),
      mxParentCanvas(),
      mpSprite(new PresenterSprite())
{
    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    mxPresenterHelper.set(
        xFactory->createInstanceWithContext(
            "com.sun.star.comp.Draw.PresenterHelper",
            mxComponentContext),
        UNO_QUERY_THROW);
}

PresenterSpritePane::~PresenterSpritePane()
{
}

void PresenterSpritePane::disposing()
{
    mpSprite->SetFactory(nullptr);
    mxParentCanvas = nullptr;
    PresenterPaneBase::disposing();
}

//----- XPane -----------------------------------------------------------------

Reference<awt::XWindow> SAL_CALL PresenterSpritePane::getWindow()
{
    ThrowIfDisposed();
    return mxContentWindow;
}

Reference<rendering::XCanvas> SAL_CALL PresenterSpritePane::getCanvas()
{
    ThrowIfDisposed();

    if ( ! mxContentCanvas.is())
        UpdateCanvases();

    return mxContentCanvas;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSpritePane::windowResized (const awt::WindowEvent& rEvent)
{
    PresenterPaneBase::windowResized(rEvent);

    mpSprite->Resize(geometry::RealSize2D(rEvent.Width, rEvent.Height));
    LayoutContextWindow();
    UpdateCanvases();
}

void SAL_CALL PresenterSpritePane::windowMoved (const awt::WindowEvent& rEvent)
{
    PresenterPaneBase::windowMoved(rEvent);

    awt::Rectangle aBox (
        mxPresenterHelper->getWindowExtentsRelative(mxBorderWindow, mxParentWindow));
    mpSprite->MoveTo(geometry::RealPoint2D(aBox.X, aBox.Y));
    mpSprite->Update();
}

void SAL_CALL PresenterSpritePane::windowShown (const lang::EventObject& rEvent)
{
    PresenterPaneBase::windowShown(rEvent);

    mpSprite->Show();
    ToTop();

    if (mxContentWindow.is())
    {
        LayoutContextWindow();
        mxContentWindow->setVisible(true);
    }
}

void SAL_CALL PresenterSpritePane::windowHidden (const lang::EventObject& rEvent)
{
    PresenterPaneBase::windowHidden(rEvent);

    mpSprite->Hide();
    if (mxContentWindow.is())
        mxContentWindow->setVisible(false);
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSpritePane::windowPaint (const awt::PaintEvent&)
{
    ThrowIfDisposed();

    /*
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxParentCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
    */
}


void PresenterSpritePane::UpdateCanvases()
{
    Reference<XComponent> xContentCanvasComponent (mxContentCanvas, UNO_QUERY);
    if (xContentCanvasComponent.is())
        xContentCanvasComponent->dispose();

    // The border canvas is the content canvas of the sprite.
    mxBorderCanvas = mpSprite->GetCanvas();

    // The content canvas is a wrapper of the border canvas.
    if (mxBorderCanvas.is())
        mxContentCanvas = mxPresenterHelper->createSharedCanvas(
            mxParentCanvas,
            mxParentWindow,
            mxBorderCanvas,
            mxBorderWindow,
            mxContentWindow);

    const awt::Rectangle aWindowBox (mxBorderWindow->getPosSize());
    PaintBorder(awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height));
}

void PresenterSpritePane::CreateCanvases (
    const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas)
{
    OSL_ASSERT(!mxParentCanvas.is() || mxParentCanvas==rxParentCanvas);
    mxParentCanvas = rxParentCanvas;

    mpSprite->SetFactory(mxParentCanvas);
    if (mxBorderWindow.is())
    {
        const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
        mpSprite->Resize(geometry::RealSize2D(aBorderBox.Width, aBorderBox.Height));
    }

    UpdateCanvases();
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
