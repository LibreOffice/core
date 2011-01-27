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

#include "PresenterSpritePane.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== PresenterSpritePane =========================================================

PresenterSpritePane::PresenterSpritePane (const Reference<XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterPaneBase(rxContext, rpPresenterController),
      mxParentWindow(),
      mxParentCanvas(),
      mpSprite(new PresenterSprite())
{
    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    mxPresenterHelper = Reference<drawing::XPresenterHelper>(
        xFactory->createInstanceWithContext(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.PresenterHelper")),
            mxComponentContext),
        UNO_QUERY_THROW);
}




PresenterSpritePane::~PresenterSpritePane (void)
{
}




void PresenterSpritePane::disposing (void)
{
    mpSprite->SetFactory(NULL);
    mxParentWindow = NULL;
    mxParentCanvas = NULL;
    PresenterPaneBase::disposing();
}




//----- XPane -----------------------------------------------------------------

Reference<awt::XWindow> SAL_CALL PresenterSpritePane::getWindow (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxContentWindow;
}




Reference<rendering::XCanvas> SAL_CALL PresenterSpritePane::getCanvas (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if ( ! mxContentCanvas.is())
        UpdateCanvases();

    return mxContentCanvas;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSpritePane::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    PresenterPaneBase::windowResized(rEvent);

    mpSprite->Resize(geometry::RealSize2D(rEvent.Width, rEvent.Height));
    LayoutContextWindow();
    UpdateCanvases();
}





void SAL_CALL PresenterSpritePane::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    PresenterPaneBase::windowMoved(rEvent);

    awt::Rectangle aBox (
        mxPresenterHelper->getWindowExtentsRelative(mxBorderWindow, mxParentWindow));
    mpSprite->MoveTo(geometry::RealPoint2D(aBox.X, aBox.Y));
    mpSprite->Update();
}




void SAL_CALL PresenterSpritePane::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    PresenterPaneBase::windowShown(rEvent);

    mpSprite->Show();
    ToTop();

    if (mxContentWindow.is())
    {
        LayoutContextWindow();
        mxContentWindow->setVisible(sal_True);
    }
}




void SAL_CALL PresenterSpritePane::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    PresenterPaneBase::windowHidden(rEvent);

    mpSprite->Hide();
    if (mxContentWindow.is())
        mxContentWindow->setVisible(sal_False);
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSpritePane::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    /*
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxParentCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
    */
}




//-----------------------------------------------------------------------------


::boost::shared_ptr<PresenterSprite> PresenterSpritePane::GetSprite (void)
{
    return mpSprite;
}




void PresenterSpritePane::ShowTransparentBorder (void)
{
}




void PresenterSpritePane::UpdateCanvases (void)
{
    Reference<XComponent> xContentCanvasComponent (mxContentCanvas, UNO_QUERY);
    if (xContentCanvasComponent.is())
    {
        if (xContentCanvasComponent.is())
            xContentCanvasComponent->dispose();
    }

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
    const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
    const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas)
{
    OSL_ASSERT(!mxParentWindow.is() || mxParentWindow==rxParentWindow);
    OSL_ASSERT(!mxParentCanvas.is() || mxParentCanvas==rxParentCanvas);
    mxParentWindow = rxParentWindow;
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
