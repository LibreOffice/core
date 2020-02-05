/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Some of this code is based on Skia source code, covered by the following
 * license notice (see readlicense_oo for the full license):
 *
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <skia/x11/gdiimpl.hxx>

#include <tools/sk_app/unix/WindowContextFactory_unix.h>
#include <tools/sk_app/WindowContext.h>

#include <vcl/skia/SkiaHelper.hxx>
#include <skia/zone.hxx>

X11SkiaSalGraphicsImpl::X11SkiaSalGraphicsImpl(X11SalGraphics& rParent)
    : SkiaSalGraphicsImpl(rParent, rParent.GetGeometryProvider())
    , mX11Parent(rParent)
{
}

X11SkiaSalGraphicsImpl::~X11SkiaSalGraphicsImpl() {}

void X11SkiaSalGraphicsImpl::Init()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    setProvider(mX11Parent.GetGeometryProvider());
    SkiaSalGraphicsImpl::Init();
}

void X11SkiaSalGraphicsImpl::createWindowContext()
{
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    displayParams.fColorType = kN32_SkColorType;
    sk_app::window_context_factory::XlibWindowInfo winInfo;
    winInfo.fDisplay = mX11Parent.GetXDisplay();
    winInfo.fWindow = mX11Parent.GetDrawable();
    assert(winInfo.fDisplay);
    // Allow window being None if offscreen, this is used to temporarily create GrContext
    // for an offscreen surface.
    assert(winInfo.fWindow != None || isOffscreen());
    winInfo.fFBConfig = nullptr; // not used
    winInfo.fVisualInfo = const_cast<SalVisual*>(&mX11Parent.GetVisual());
#ifdef DBG_UTIL
    // Our patched Skia has VulkanWindowContext that shares GrContext, which requires
    // that the X11 visual is always the same. Ensure it is so.
    static VisualID checkVisualID = -1U;
    assert(checkVisualID == -1U || winInfo.fVisualInfo->visualid == checkVisualID);
    checkVisualID = winInfo.fVisualInfo->visualid;
#endif
    winInfo.fWidth = GetWidth();
    winInfo.fHeight = GetHeight();
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderRaster:
            // TODO The Skia Xlib code actually requires the non-native color type to work properly.
            displayParams.fColorType
                = (displayParams.fColorType == kBGRA_8888_SkColorType ? kRGBA_8888_SkColorType
                                                                      : kBGRA_8888_SkColorType);
            mWindowContext
                = sk_app::window_context_factory::MakeRasterForXlib(winInfo, displayParams);
            mIsGPU = false;
            break;
        case SkiaHelper::RenderVulkan:
            mWindowContext
                = sk_app::window_context_factory::MakeVulkanForXlib(winInfo, displayParams);
            mIsGPU = true;
            break;
    }
}

bool X11SkiaSalGraphicsImpl::avoidRecreateByResize() const
{
    if (!mSurface || isOffscreen())
        return false;
    // Skia's WindowContext uses actual dimensions of the X window, which due to X11 being
    // asynchronous may be temporarily different from what VCL thinks are the dimensions.
    // That can lead to us repeatedly calling recreateSurface() because of "incorrect"
    // size, and we otherwise need to check for size changes, because VCL does not inform us.
    // Avoid the problem here by checking the size of the X window and bail out if Skia
    // would just return the same size as it is now.
    Window r;
    int x, y;
    unsigned int w, h, border, depth;
    XGetGeometry(mX11Parent.GetXDisplay(), mX11Parent.GetDrawable(), &r, &x, &y, &w, &h, &border,
                 &depth);
    return mSurface->width() == int(w) && mSurface->height() == int(h);
}

void X11SkiaSalGraphicsImpl::DeInit()
{
    SkiaZone zone;
    SkiaSalGraphicsImpl::DeInit();
    mWindowContext.reset();
}

void X11SkiaSalGraphicsImpl::freeResources() {}

void X11SkiaSalGraphicsImpl::performFlush()
{
    SkiaZone zone;
    mPendingPixelsToFlush = 0;
    // TODO XPutImage() is somewhat inefficient, XShmPutImage() should be preferred.
    mWindowContext->swapBuffers();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
