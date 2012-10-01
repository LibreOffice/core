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

#include "PresenterPane.hxx"
#include "PresenterController.hxx"
#include "PresenterPaintManager.hxx"
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/drawing/CanvasFeature.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== PresenterPane =========================================================

PresenterPane::PresenterPane (
    const Reference<XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterPaneBase(rxContext, rpPresenterController),
      maBoundingBox()
{
    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    mxPresenterHelper = Reference<drawing::XPresenterHelper>(
        xFactory->createInstanceWithContext(
            OUString("com.sun.star.comp.Draw.PresenterHelper"),
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
    PresenterPaneBase::windowResized(rEvent);

    Invalidate(maBoundingBox);

    LayoutContextWindow();
    ToTop();

    UpdateBoundingBox();
    Invalidate(maBoundingBox);
}

void SAL_CALL PresenterPane::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    PresenterPaneBase::windowMoved(rEvent);

    Invalidate(maBoundingBox);

    ToTop();

    UpdateBoundingBox();
    Invalidate(maBoundingBox);
}

void SAL_CALL PresenterPane::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    PresenterPaneBase::windowShown(rEvent);

    ToTop();

    if (mxContentWindow.is())
    {
        LayoutContextWindow();
        mxContentWindow->setVisible(sal_True);
    }

    UpdateBoundingBox();
    Invalidate(maBoundingBox);
}

void SAL_CALL PresenterPane::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    PresenterPaneBase::windowHidden(rEvent);

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

void PresenterPane::Invalidate (const css::awt::Rectangle& rRepaintBox)
{
    // Invalidate the parent window to be able to invalidate an area outside
    // the current window area.
    mpPresenterController->GetPaintManager()->Invalidate(mxParentWindow, rRepaintBox);
}

void PresenterPane::UpdateBoundingBox (void)
{
    if (mxBorderWindow.is() && IsVisible())
        maBoundingBox = mxBorderWindow->getPosSize();
    else
        maBoundingBox = awt::Rectangle();
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
