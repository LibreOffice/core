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

#include <tools/window/unix/RasterWindowContext_unix.h>
#include <tools/window/unix/GaneshVulkanWindowContext_unix.h>
#include <tools/window/unix/XlibWindowInfo.h>

#include <skia/utils.hxx>
#include <skia/zone.hxx>

#include <X11/Xutil.h>

using namespace SkiaHelper;

X11SkiaSalGraphicsImpl::X11SkiaSalGraphicsImpl(X11SalGraphics& rParent)
    : SkiaSalGraphicsImpl(rParent, rParent.GetGeometryProvider())
    , mX11Parent(rParent)
{
}

void X11SkiaSalGraphicsImpl::UpdateX11GeometryProvider()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    setProvider(mX11Parent.GetGeometryProvider());
}

void X11SkiaSalGraphicsImpl::createWindowSurfaceInternal(bool forceRaster)
{
    assert(!mWindowContext);
    assert(!mSurface);
    assert(mX11Parent.GetDrawable() != None);
    RenderMethod renderMethod = forceRaster ? RenderRaster : renderMethodToUse();
    mScaling = getWindowScaling();
    mWindowContext = createWindowContext(mX11Parent.GetXDisplay(), mX11Parent.GetDrawable(),
                                         &mX11Parent.GetVisual(), GetWidth() * mScaling,
                                         GetHeight() * mScaling, renderMethod, false);
    if (mWindowContext)
    {
        // See flushSurfaceToWindowContext().
        if (renderMethod == RenderRaster)
            mSurface = mWindowContext->getBackbufferSurface();
        else
            mSurface = createSkSurface(GetWidth(), GetHeight());
    }
}

std::unique_ptr<skwindow::WindowContext>
X11SkiaSalGraphicsImpl::createWindowContext(Display* display, Drawable drawable,
                                            const XVisualInfo* visual, int width, int height,
                                            RenderMethod renderMethod, bool temporary)
{
    SkiaZone zone;
    skwindow::DisplayParamsBuilder displayParamsBuilder;
    displayParamsBuilder.colorType(kN32_SkColorType);
#if defined LINUX
    // WORKAROUND: VSync causes freezes that can even temporarily freeze the entire desktop.
    // This happens even with the latest 450.66 drivers despite them claiming a fix for vsync.
    // https://forums.developer.nvidia.com/t/hangs-freezes-when-vulkan-v-sync-vk-present-mode-fifo-khr-is-enabled/67751
    if (getVendor() == DriverBlocklist::VendorNVIDIA)
        displayParamsBuilder.disableVsync(true);
#endif
    skwindow::XlibWindowInfo winInfo;
    assert(display);
    winInfo.fDisplay = display;
    winInfo.fWindow = drawable;
    winInfo.fFBConfig = nullptr; // not used
    winInfo.fVisualInfo = const_cast<XVisualInfo*>(visual);
    assert(winInfo.fVisualInfo->visual != nullptr); // make sure it's not an uninitialized SalVisual
    winInfo.fWidth = width;
    winInfo.fHeight = height;
#if defined DBG_UTIL && !defined NDEBUG
    // Our patched Skia has VulkanWindowContext that shares grDirectContext, which requires
    // that the X11 visual is always the same. Ensure it is so.
    static VisualID checkVisualID = -1U;
    // Exception is for the temporary case during startup, when SkiaHelper's
    // checkDeviceDenylisted() needs a WindowContext and may be called before SalVisual
    // is ready.
    if (!temporary)
    {
        assert(checkVisualID == -1U || winInfo.fVisualInfo->visualid == checkVisualID);
        checkVisualID = winInfo.fVisualInfo->visualid;
    }
#else
    (void)temporary;
#endif
    switch (renderMethod)
    {
        case RenderRaster:
        {
            // Make sure we ask for color type that matches the X11 visual. If red mask
            // is larger value than blue mask, then on little endian this means blue is first.
            // This should also preferably match SK_R32_SHIFT set in config_skia.h, as that
            // improves performance, the common setup seems to be BGRA (possibly because of
            // choosing OpenGL-capable visual).
            displayParamsBuilder.colorType(visual->red_mask > visual->blue_mask
                                               ? kBGRA_8888_SkColorType
                                               : kRGBA_8888_SkColorType);
            return skwindow::MakeRasterForXlib(winInfo, displayParamsBuilder.build());
        }
        case RenderVulkan:
            return skwindow::MakeGaneshVulkanForXlib(winInfo, displayParamsBuilder.build());
        case RenderMetal:
            abort();
            break;
    }
    abort();
}

bool X11SkiaSalGraphicsImpl::avoidRecreateByResize() const
{
    if (SkiaSalGraphicsImpl::avoidRecreateByResize())
        return true;
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

void X11SkiaSalGraphicsImpl::Flush() { performFlush(); }

std::unique_ptr<skwindow::WindowContext> createVulkanWindowContext(bool temporary)
{
    SalDisplay* salDisplay = vcl_sal::getSalDisplay(GetGenericUnixSalData());
    const XVisualInfo* visual;
    XVisualInfo* visuals = nullptr;
    if (!temporary)
        visual = &salDisplay->GetVisual(salDisplay->GetDefaultXScreen());
    else
    {
        // SalVisual from salDisplay may not be setup yet at this point, get
        // info for the default visual.
        XVisualInfo search;
        search.visualid = XVisualIDFromVisual(
            DefaultVisual(salDisplay->GetDisplay(), salDisplay->GetDefaultXScreen().getXScreen()));
        int count;
        visuals = XGetVisualInfo(salDisplay->GetDisplay(), VisualIDMask, &search, &count);
        assert(count == 1);
        visual = visuals;
    }
    std::unique_ptr<skwindow::WindowContext> ret = X11SkiaSalGraphicsImpl::createWindowContext(
        salDisplay->GetDisplay(), None, visual, 1, 1, RenderVulkan, temporary);
    if (temporary)
        XFree(visuals);
    return ret;
}

void X11SkiaSalGraphicsImpl::prepareSkia() { SkiaHelper::prepareSkia(createVulkanWindowContext); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
