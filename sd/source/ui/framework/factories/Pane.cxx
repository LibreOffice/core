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

#include <framework/Pane.hxx>
#include <framework/ResourceFactory.hxx>

#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd::framework {

Pane::Pane (
    const Reference<XResourceId>& rxPaneId,
    vcl::Window* pWindow)
    noexcept
    : mxPaneId(rxPaneId),
      mpWindow(pWindow),
      mxWindow(VCLUnoHelper::GetInterface(pWindow))
{
}

Pane::~Pane()
{
}

void Pane::disposing(std::unique_lock<std::mutex>&)
{
    mxWindow = nullptr;
    mpWindow = nullptr;
}

vcl::Window* Pane::GetWindow()
{
    if (mxWindow.is())
        return mpWindow;
    else
        return nullptr;
}

//----- AbstractPane -----------------------------------------------------------------

Reference<awt::XWindow> Pane::getWindow()
{
    std::unique_lock aGuard (m_aMutex);
    throwIfDisposed(aGuard);

    return mxWindow;
}

Reference<rendering::XCanvas> Pane::getCanvas()
{
    std::unique_lock aGuard (m_aMutex);
    throwIfDisposed(aGuard);

    if ( ! mxCanvas.is())
        mxCanvas = CreateCanvas();

    return mxCanvas;
}

bool Pane::isVisible()
{
    {
        std::unique_lock aGuard (m_aMutex);
        throwIfDisposed(aGuard);
    }

    const vcl::Window* pWindow = GetWindow();
    if (pWindow != nullptr)
        return pWindow->IsVisible();
    else
        return false;
}

void Pane::setVisible (bool bIsVisible)
{
    {
        std::unique_lock aGuard (m_aMutex);
        throwIfDisposed(aGuard);
    }

    vcl::Window* pWindow = GetWindow();
    if (pWindow != nullptr)
        pWindow->Show(bIsVisible);
}

//----- XResource -------------------------------------------------------------

Reference<XResourceId> SAL_CALL Pane::getResourceId()
{
    {
        std::unique_lock aGuard (m_aMutex);
        throwIfDisposed(aGuard);
    }

    return mxPaneId;
}

sal_Bool SAL_CALL Pane::isAnchorOnly()
{
    return true;
}

Reference<rendering::XCanvas> Pane::CreateCanvas()
{
    Reference<rendering::XCanvas> xCanvas;

    if (mpWindow != nullptr)
    {
        ::cppcanvas::SpriteCanvasSharedPtr pCanvas (
            cppcanvas::VCLFactory::createSpriteCanvas(*mpWindow));
        if (pCanvas)
            xCanvas.set(pCanvas->getUNOSpriteCanvas());
    }

    return xCanvas;
}

ResourceFactory::~ResourceFactory() {}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
