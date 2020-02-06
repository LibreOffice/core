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

#include <skia/utils.hxx>
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
    assert(mX11Parent.GetDrawable() != None);
    mWindowContext = createWindowContext(mX11Parent.GetXDisplay(), mX11Parent.GetDrawable(),
                                         &mX11Parent.GetVisual(), GetWidth(), GetHeight(),
                                         SkiaHelper::renderMethodToUse());
    if (mWindowContext && SkiaHelper::renderMethodToUse() == SkiaHelper::RenderVulkan)
        mIsGPU = true;
    else
        mIsGPU = false;
}

std::unique_ptr<sk_app::WindowContext>
X11SkiaSalGraphicsImpl::createWindowContext(Display* display, Drawable drawable,
                                            const SalVisual* visual, int width, int height,
                                            SkiaHelper::RenderMethod renderMethod)
{
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    displayParams.fColorType = kN32_SkColorType;
    sk_app::window_context_factory::XlibWindowInfo winInfo;
    assert(display);
    winInfo.fDisplay = display;
    winInfo.fWindow = drawable;
    winInfo.fFBConfig = nullptr; // not used
    winInfo.fVisualInfo = const_cast<SalVisual*>(visual);
    winInfo.fWidth = width;
    winInfo.fHeight = height;
#ifdef DBG_UTIL
    // Our patched Skia has VulkanWindowContext that shares GrContext, which requires
    // that the X11 visual is always the same. Ensure it is so.
    static VisualID checkVisualID = -1U;
    assert(checkVisualID == -1U || winInfo.fVisualInfo->visualid == checkVisualID);
    checkVisualID = winInfo.fVisualInfo->visualid;
#endif
    switch (renderMethod)
    {
        case SkiaHelper::RenderRaster:
            // TODO The Skia Xlib code actually requires the non-native color type to work properly.
            displayParams.fColorType
                = (displayParams.fColorType == kBGRA_8888_SkColorType ? kRGBA_8888_SkColorType
                                                                      : kBGRA_8888_SkColorType);
            return sk_app::window_context_factory::MakeRasterForXlib(winInfo, displayParams);
        case SkiaHelper::RenderVulkan:
            return sk_app::window_context_factory::MakeVulkanForXlib(winInfo, displayParams);
    }
    abort();
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

std::unique_ptr<sk_app::WindowContext> createVulkanWindowContext()
{
    SalDisplay* salDisplay = vcl_sal::getSalDisplay(GetGenericUnixSalData());
    return X11SkiaSalGraphicsImpl::createWindowContext(
        salDisplay->GetDisplay(), None, &salDisplay->GetVisual(salDisplay->GetDefaultXScreen()), 1,
        1, SkiaHelper::RenderVulkan);
}

namespace
{
struct SetFunction
{
    SetFunction() { SkiaHelper::setCreateVulkanWindowContext(createVulkanWindowContext); }
};
SetFunction setFunction;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
