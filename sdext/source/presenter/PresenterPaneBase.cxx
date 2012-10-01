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

#include "PresenterPaneBase.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterController.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterTextView.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/drawing/CanvasFeature.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== PresenterPaneBase =====================================================

PresenterPaneBase::PresenterPaneBase (
    const Reference<XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterPaneBaseInterfaceBase(m_aMutex),
      mpPresenterController(rpPresenterController),
      mxParentWindow(),
      mxBorderWindow(),
      mxBorderCanvas(),
      mxContentWindow(),
      mxContentCanvas(),
      mxPaneId(),
      mxBorderPainter(),
      mxPresenterHelper(),
      msTitle(),
      mxComponentContext(rxContext),
      mpViewBackground(),
      mbHasCallout(false),
      maCalloutAnchor()
{
    if (mpPresenterController.get() != NULL)
        mxPresenterHelper = mpPresenterController->GetPresenterHelper();
}

PresenterPaneBase::~PresenterPaneBase (void)
{
}

void PresenterPaneBase::disposing (void)
{
    if (mxBorderWindow.is())
    {
        mxBorderWindow->removeWindowListener(this);
        mxBorderWindow->removePaintListener(this);
    }

    {
        Reference<XComponent> xComponent (mxContentCanvas, UNO_QUERY);
        mxContentCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent (mxContentWindow, UNO_QUERY);
        mxContentWindow = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent (mxBorderCanvas, UNO_QUERY);
        mxBorderCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent (mxBorderWindow, UNO_QUERY);
        mxBorderWindow = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    mxComponentContext = NULL;
}

void PresenterPaneBase::SetTitle (const OUString& rsTitle)
{
    msTitle = rsTitle;

    OSL_ASSERT(mpPresenterController.get()!=NULL);
    OSL_ASSERT(mpPresenterController->GetPaintManager().get()!=NULL);

    mpPresenterController->GetPaintManager()->Invalidate(mxBorderWindow);
}

::rtl::OUString PresenterPaneBase::GetTitle (void) const
{
    return msTitle;
}

Reference<drawing::framework::XPaneBorderPainter>
    PresenterPaneBase::GetPaneBorderPainter (void) const
{
    return mxBorderPainter;
}

void PresenterPaneBase::SetCalloutAnchor (const css::awt::Point& rCalloutAnchor)
{
    mbHasCallout = true;
    // Anchor is given in the coorindate system of the parent window.
    // Transform it into the local coordinate system.
    maCalloutAnchor = rCalloutAnchor;
    const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
    maCalloutAnchor.X -= aBorderBox.X;
    maCalloutAnchor.Y -= aBorderBox.Y;

    // Move the bottom of the border window so that it goes through the
    // callout anchor (special case for bottom callout).
    sal_Int32 nHeight (rCalloutAnchor.Y - aBorderBox.Y);
    if (mxBorderPainter.is() && mxPaneId.is())
        nHeight += mxBorderPainter->getCalloutOffset(mxPaneId->getResourceURL()).Y;

    if (nHeight != aBorderBox.Height)
    {
        mxBorderWindow->setPosSize(
            aBorderBox.X,
            aBorderBox.Y,
            aBorderBox.Width,
            nHeight,
            awt::PosSize::HEIGHT);
    }

    mpPresenterController->GetPaintManager()->Invalidate(mxBorderWindow);
}

awt::Point PresenterPaneBase::GetCalloutAnchor (void) const
{
    return maCalloutAnchor;
}

//----- XInitialization -------------------------------------------------------

void SAL_CALL PresenterPaneBase::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();

    if ( ! mxComponentContext.is())
    {
        throw RuntimeException(
            OUString("PresenterSpritePane: missing component context"),
            static_cast<XWeak*>(this));
    }

    if (rArguments.getLength() == 5 || rArguments.getLength() == 6)
    {
        try
        {
            // Get the resource id from the first argument.
            if ( ! (rArguments[0] >>= mxPaneId))
            {
                throw lang::IllegalArgumentException(
                    OUString("PresenterPane: invalid pane id"),
                    static_cast<XWeak*>(this),
                    0);
            }

            if ( ! (rArguments[1] >>= mxParentWindow))
            {
                throw lang::IllegalArgumentException(
                    OUString("PresenterPane: invalid parent window"),
                    static_cast<XWeak*>(this),
                    1);
            }

            Reference<rendering::XSpriteCanvas> xParentCanvas;
            if ( ! (rArguments[2] >>= xParentCanvas))
            {
                throw lang::IllegalArgumentException(
                    OUString("PresenterPane: invalid parent canvas"),
                    static_cast<XWeak*>(this),
                    2);
            }

            if ( ! (rArguments[3] >>= msTitle))
            {
                throw lang::IllegalArgumentException(
                    OUString("PresenterPane: invalid title"),
                    static_cast<XWeak*>(this),
                    3);
            }

            if ( ! (rArguments[4] >>= mxBorderPainter))
            {
                throw lang::IllegalArgumentException(
                    OUString("PresenterPane: invalid border painter"),
                    static_cast<XWeak*>(this),
                    4);
            }

            bool bIsWindowVisibleOnCreation (true);
            if (rArguments.getLength()>5 && ! (rArguments[5] >>= bIsWindowVisibleOnCreation))
            {
                throw lang::IllegalArgumentException(
                    OUString("PresenterPane: invalid window visibility flag"),
                    static_cast<XWeak*>(this),
                    5);
            }

            CreateWindows(mxParentWindow, bIsWindowVisibleOnCreation);

            if (mxBorderWindow.is())
            {
                mxBorderWindow->addWindowListener(this);
                mxBorderWindow->addPaintListener(this);
            }

            CreateCanvases(mxParentWindow, xParentCanvas);

            // Raise new windows.
            ToTop();
        }
        catch (Exception&)
        {
            mxContentWindow = NULL;
            mxComponentContext = NULL;
            throw;
        }
    }
    else
    {
        throw RuntimeException(
            OUString("PresenterSpritePane: invalid number of arguments"),
                static_cast<XWeak*>(this));
    }
}

//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterPaneBase::getResourceId (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxPaneId;
}

sal_Bool SAL_CALL PresenterPaneBase::isAnchorOnly (void)
    throw (RuntimeException)
{
    return true;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterPaneBase::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

void SAL_CALL PresenterPaneBase::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

void SAL_CALL PresenterPaneBase::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

void SAL_CALL PresenterPaneBase::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterPaneBase::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxBorderWindow)
    {
        mxBorderWindow = NULL;
    }
}

//-----------------------------------------------------------------------------

void PresenterPaneBase::CreateWindows (
    const Reference<awt::XWindow>& rxParentWindow,
    const bool bIsWindowVisibleOnCreation)
{
    if (mxPresenterHelper.is() && rxParentWindow.is())
    {

        mxBorderWindow = mxPresenterHelper->createWindow(
            rxParentWindow,
            sal_False,
            bIsWindowVisibleOnCreation,
            sal_False,
            sal_False);
        mxContentWindow = mxPresenterHelper->createWindow(
            mxBorderWindow,
            sal_False,
            bIsWindowVisibleOnCreation,
            sal_False,
            sal_False);
    }
}

Reference<awt::XWindow> PresenterPaneBase::GetBorderWindow (void) const
{
    return mxBorderWindow;
}

void PresenterPaneBase::ToTop (void)
{
    if (mxPresenterHelper.is())
        mxPresenterHelper->toTop(mxContentWindow);
}

void PresenterPaneBase::SetBackground (const SharedBitmapDescriptor& rpBackground)
{
    mpViewBackground = rpBackground;
}

void PresenterPaneBase::PaintBorderBackground (
    const awt::Rectangle& rBorderBox,
    const awt::Rectangle& rUpdateBox)
{
    (void)rBorderBox;
    (void)rUpdateBox;
    /*
    // The outer box of the border is given.  We need the center and inner
    // box as well.
    awt::Rectangle aCenterBox (
        mxBorderPainter->removeBorder(
            mxPaneId->getResourceURL(),
            rBorderBox,
            drawing::framework::BorderType_OUTER_BORDER));
    awt::Rectangle aInnerBox (
        mxBorderPainter->removeBorder(
            mxPaneId->getResourceURL(),
            rBorderBox,
            drawing::framework::BorderType_TOTAL_BORDER));
    mpPresenterController->GetCanvasHelper()->Paint(
        mpViewBackground,
        mxBorderCanvas,
        rUpdateBox,
        aCenterBox,
        aInnerBox);
    */
}

void PresenterPaneBase::PaintBorder (const awt::Rectangle& rUpdateBox)
{
    OSL_ASSERT(mxPaneId.is());

    if (mxBorderPainter.is() && mxBorderWindow.is() && mxBorderCanvas.is())
    {
        awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
        awt::Rectangle aLocalBorderBox (0,0, aBorderBox.Width, aBorderBox.Height);

        PaintBorderBackground(aLocalBorderBox, rUpdateBox);

        if (mbHasCallout)
            mxBorderPainter->paintBorderWithCallout(
                mxPaneId->getResourceURL(),
                mxBorderCanvas,
                aLocalBorderBox,
                rUpdateBox,
                msTitle,
                maCalloutAnchor);
        else
            mxBorderPainter->paintBorder(
                mxPaneId->getResourceURL(),
                mxBorderCanvas,
                aLocalBorderBox,
                rUpdateBox,
                msTitle);
    }
}

void PresenterPaneBase::LayoutContextWindow (void)
{
    OSL_ASSERT(mxPaneId.is());
    OSL_ASSERT(mxBorderWindow.is());
    OSL_ASSERT(mxContentWindow.is());
    if (mxBorderPainter.is() && mxPaneId.is() && mxBorderWindow.is() && mxContentWindow.is())
    {
        const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
        const awt::Rectangle aInnerBox (mxBorderPainter->removeBorder(
            mxPaneId->getResourceURL(),
            aBorderBox,
            drawing::framework::BorderType_TOTAL_BORDER));
        mxContentWindow->setPosSize(
            aInnerBox.X - aBorderBox.X,
            aInnerBox.Y - aBorderBox.Y,
            aInnerBox.Width,
            aInnerBox.Height,
            awt::PosSize::POSSIZE);
    }
}

bool PresenterPaneBase::IsVisible (void) const
{
    Reference<awt::XWindow2> xWindow2 (mxBorderPainter, UNO_QUERY);
    if (xWindow2.is())
        return xWindow2->isVisible();

    return false;
}

void PresenterPaneBase::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString( "PresenterPane object has already been disposed"),
            static_cast<uno::XWeak*>(this));
    }
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
