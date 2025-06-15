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
#include "PresenterController.hxx"
#include "PresenterPaintManager.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <utility>

using namespace css;
using namespace css::uno;
using namespace css::drawing::framework;

namespace sdext::presenter {

//===== PresenterPaneBase =====================================================

PresenterPaneBase::PresenterPaneBase (
    const Reference<XComponentContext>& rxContext,
    ::rtl::Reference<PresenterController> xPresenterController)
    : PresenterPaneBaseInterfaceBase(m_aMutex),
      mpPresenterController(std::move(xPresenterController)),
      mxComponentContext(rxContext)
{
}

PresenterPaneBase::~PresenterPaneBase()
{
}

void PresenterPaneBase::disposing()
{
    if (mxBorderWindow.is())
    {
        mxBorderWindow->removeWindowListener(this);
        mxBorderWindow->removePaintListener(this);
    }

    {
        Reference<XComponent> xComponent (mxContentCanvas, UNO_QUERY);
        mxContentCanvas = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent = mxContentWindow;
        mxContentWindow = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent (mxBorderCanvas, UNO_QUERY);
        mxBorderCanvas = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent  = mxBorderWindow;
        mxBorderWindow = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    mxComponentContext = nullptr;
}

void PresenterPaneBase::SetTitle (const OUString& rsTitle)
{
    msTitle = rsTitle;

    assert(mpPresenterController);
    assert(mpPresenterController->GetPaintManager());

    mpPresenterController->GetPaintManager()->Invalidate(mxBorderWindow);
}

const rtl::Reference<PresenterPaneBorderPainter>&
    PresenterPaneBase::GetPaneBorderPainter() const
{
    return mxBorderPainter;
}

void PresenterPaneBase::initialize(
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
    const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
    const css::uno::Reference<css::rendering::XCanvas>& rxParentCanvas,
    const rtl::Reference<PresenterPaneBorderPainter>& rxBorderPainter,
    bool bIsWindowVisibleOnCreation)
{
    ThrowIfDisposed();

    if ( ! mxComponentContext.is())
    {
        throw RuntimeException(
            u"PresenterSpritePane: missing component context"_ustr,
            static_cast<XWeak*>(this));
    }

    mxPaneId = rxPaneId;
    mxParentWindow = rxParentWindow;

    Reference<rendering::XSpriteCanvas> xParentCanvas(rxParentCanvas, UNO_QUERY_THROW);
    mxBorderPainter = rxBorderPainter;

    CreateWindows(bIsWindowVisibleOnCreation);

    if (mxBorderWindow.is())
    {
        mxBorderWindow->addWindowListener(this);
        mxBorderWindow->addPaintListener(this);
    }

    CreateCanvases(xParentCanvas);

    // Raise new windows.
    ToTop();
}

//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterPaneBase::getResourceId()
{
    ThrowIfDisposed();
    return mxPaneId;
}

sal_Bool SAL_CALL PresenterPaneBase::isAnchorOnly()
{
    return true;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterPaneBase::windowResized (const awt::WindowEvent&)
{
    ThrowIfDisposed();
}

void SAL_CALL PresenterPaneBase::windowMoved (const awt::WindowEvent&)
{
    ThrowIfDisposed();
}

void SAL_CALL PresenterPaneBase::windowShown (const lang::EventObject&)
{
    ThrowIfDisposed();
}

void SAL_CALL PresenterPaneBase::windowHidden (const lang::EventObject&)
{
    ThrowIfDisposed();
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterPaneBase::disposing (const lang::EventObject& rEvent)
{
    if (rEvent.Source == mxBorderWindow)
    {
        mxBorderWindow = nullptr;
    }
}


void PresenterPaneBase::CreateWindows (
    const bool bIsWindowVisibleOnCreation)
{
    if (!mxParentWindow.is())
        return;

    mxBorderWindow = sd::presenter::PresenterHelper::createWindow(
        mxParentWindow, bIsWindowVisibleOnCreation);
    mxContentWindow = sd::presenter::PresenterHelper::createWindow(
        mxBorderWindow, bIsWindowVisibleOnCreation);
}

const Reference<awt::XWindow>& PresenterPaneBase::GetBorderWindow() const
{
    return mxBorderWindow;
}

void PresenterPaneBase::ToTop()
{
    sd::presenter::PresenterHelper::toTop(mxContentWindow);
}

void PresenterPaneBase::PaintBorder (const awt::Rectangle& rUpdateBox)
{
    OSL_ASSERT(mxPaneId.is());

    if (!(mxBorderPainter.is() && mxBorderWindow.is() && mxBorderCanvas.is()))
        return;

    awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
    awt::Rectangle aLocalBorderBox (0,0, aBorderBox.Width, aBorderBox.Height);

    //TODO: paint border background?

    mxBorderPainter->paintBorder(
            mxPaneId->getResourceURL(),
            mxBorderCanvas,
            aLocalBorderBox,
            rUpdateBox,
            msTitle);
}

void PresenterPaneBase::LayoutContextWindow()
{
    OSL_ASSERT(mxPaneId.is());
    OSL_ASSERT(mxBorderWindow.is());
    OSL_ASSERT(mxContentWindow.is());
    if (!(mxBorderPainter.is() && mxPaneId.is() && mxBorderWindow.is() && mxContentWindow.is()))
        return;

    const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
    const awt::Rectangle aInnerBox (mxBorderPainter->removeBorder(
        mxPaneId->getResourceURL(),
        aBorderBox,
        sdext::presenter::BorderType::TOTAL));
    mxContentWindow->setPosSize(
        aInnerBox.X - aBorderBox.X,
        aInnerBox.Y - aBorderBox.Y,
        aInnerBox.Width,
        aInnerBox.Height,
        awt::PosSize::POSSIZE);
}

void PresenterPaneBase::ThrowIfDisposed()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            u"PresenterPane object has already been disposed"_ustr,
            static_cast<uno::XWeak*>(this));
    }
}

} // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
