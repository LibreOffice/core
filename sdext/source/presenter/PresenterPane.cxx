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
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.PresenterHelper")),
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
