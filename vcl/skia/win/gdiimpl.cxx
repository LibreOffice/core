/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <skia/win/gdiimpl.hxx>

#include <tools/sk_app/win/WindowContextFactory_win.h>
#include <tools/sk_app/WindowContext.h>

WinSkiaSalGraphicsImpl::WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics,
                                               SalGeometryProvider* mpProvider)
    : SkiaSalGraphicsImpl(rGraphics, mpProvider)
    , mWinParent(rGraphics)
{
}

void WinSkiaSalGraphicsImpl::copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics)
{
    (void)rPosAry;
    (void)pSrcGraphics;
    // TODO
}

void WinSkiaSalGraphicsImpl::Init()
{
#if 0 // TODO
    if (!IsOffscreen() && mpContext.is() && mpContext->isInitialized())
    {
        const GLWinWindow& rGLWindow = static_cast<const GLWinWindow&>(mpContext->getOpenGLWindow());
        if (rGLWindow.hWnd != mrWinParent.mhWnd || rGLWindow.hDC == mrWinParent.mhLocalDC)
        {
            // This can legitimately happen, SalFrame keeps 2x
            // SalGraphics which share the same hWnd and hDC.
            // The shape 'Area' dialog does reparenting to trigger this.
            SAL_WARN("vcl.opengl", "Unusual: Windows handle / DC changed without DeInit");
            DeInit();
        }
    }
#endif
    SkiaSalGraphicsImpl::Init();
}

void WinSkiaSalGraphicsImpl::createSurface()
{
    if (isOffscreen())
        return SkiaSalGraphicsImpl::createSurface();
    if (GetWidth() == 0 || GetHeight() == 0)
    {
        // When created, Init() gets called with size (0,0), which is invalid size
        // for Skia. So fake a surface, Init() will get called later again with the correct size.
        mSurface = SkSurface::MakeRasterN32Premul(1, 1);
        return;
    }
    sk_app::DisplayParams displayParams;
    mWindowContext.reset(
        sk_app::window_context_factory::NewRasterForWin(mWinParent.gethWnd(), displayParams));
    assert(SkToBool(mWindowContext)); // TODO
    mSurface = mWindowContext->getBackbufferSurface();
    assert(mSurface.get());
}

void WinSkiaSalGraphicsImpl::DeInit()
{
    mWindowContext.reset();
    SkiaSalGraphicsImpl::DeInit();
}

void WinSkiaSalGraphicsImpl::freeResources() {}

void WinSkiaSalGraphicsImpl::performFlush()
{
    if (mWindowContext)
        mWindowContext->swapBuffers();
}

bool WinSkiaSalGraphicsImpl::TryRenderCachedNativeControl(ControlCacheKey const& rControlCacheKey,
                                                          int nX, int nY)
{
    (void)rControlCacheKey;
    (void)nX;
    (void)nY;
    return false; // TODO
}

bool WinSkiaSalGraphicsImpl::RenderAndCacheNativeControl(CompatibleDC& rWhite, CompatibleDC& rBlack,
                                                         int nX, int nY,
                                                         ControlCacheKey& aControlCacheKey)
{
    (void)rWhite;
    (void)rBlack;
    (void)nX;
    (void)nY;
    (void)aControlCacheKey;
    return false; // TODO
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
