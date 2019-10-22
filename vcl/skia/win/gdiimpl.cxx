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
    // When created, Init() gets called with size (0,0), which is invalid size
    // for Skia. Creating the actual surface is delayed, so the size should be always
    // valid here, but better check.
    assert(GetWidth() != 0 && GetHeight() != 0);
    sk_app::DisplayParams displayParams;
    switch (renderMethodToUse())
    {
        case RenderRaster:
            mWindowContext = sk_app::window_context_factory::MakeRasterForWin(mWinParent.gethWnd(),
                                                                              displayParams);
            break;
        case RenderVulkan:
            mWindowContext = sk_app::window_context_factory::MakeVulkanForWin(mWinParent.gethWnd(),
                                                                              displayParams);
            break;
    }
    assert(SkToBool(mWindowContext)); // TODO
    mSurface = mWindowContext->getBackbufferSurface();
    assert(mSurface.get());
}

void WinSkiaSalGraphicsImpl::DeInit()
{
    SkiaSalGraphicsImpl::DeInit();
    mWindowContext.reset();
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

void WinSkiaSalGraphicsImpl::PreDrawText() { preDraw(); }

void WinSkiaSalGraphicsImpl::PostDrawText() { postDraw(); }

void WinSkiaSalGraphicsImpl::DeferredTextDraw(const CompatibleDC::Texture* pTexture,
                                              Color aMaskColor, const SalTwoRect& rPosAry)
{
    assert(dynamic_cast<const SkiaCompatibleDC::Texture*>(pTexture));
    drawMask(rPosAry, static_cast<const SkiaCompatibleDC::Texture*>(pTexture)->bitmap, aMaskColor);
}

void WinSkiaSalGraphicsImpl::DrawMask(CompatibleDC::Texture* pTexture, Color nMaskColor,
                                      const SalTwoRect& rPosAry)
{
    assert(dynamic_cast<SkiaCompatibleDC::Texture*>(pTexture));
    drawMask(rPosAry, static_cast<const SkiaCompatibleDC::Texture*>(pTexture)->bitmap, nMaskColor);
}

SkiaCompatibleDC::SkiaCompatibleDC(SalGraphics& rGraphics, int x, int y, int width, int height)
    : CompatibleDC(rGraphics, x, y, width, height, false)
{
}

std::unique_ptr<CompatibleDC::Texture> SkiaCompatibleDC::getTexture()
{
    auto ret = std::make_unique<SkiaCompatibleDC::Texture>();
    // TODO is this correct?
    // TODO make copy of data?
    if (!ret->bitmap.installPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                     kBGRA_8888_SkColorType, kUnpremul_SkAlphaType),
                                   mpData, maRects.mnSrcWidth * 4))
        abort();
    return ret;
}

bool SkiaCompatibleDC::copyToTexture(CompatibleDC::Texture& aTexture)
{
    assert(mpImpl);
    assert(dynamic_cast<SkiaCompatibleDC::Texture*>(&aTexture));
    SkBitmap tmpBitmap;
    if (!tmpBitmap.installPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                   kBGRA_8888_SkColorType, kUnpremul_SkAlphaType),
                                 mpData, maRects.mnSrcWidth * 4))
        abort();
    SkBitmap& bitmap = static_cast<SkiaCompatibleDC::Texture&>(aTexture).bitmap;
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    SkCanvas canvas(bitmap);
    // The data we got is upside-down.
    SkMatrix matrix;
    matrix.preTranslate(0, maRects.mnSrcHeight);
    matrix.setConcat(matrix, SkMatrix::MakeScale(1, -1));
    canvas.concat(matrix);
    canvas.drawBitmapRect(tmpBitmap,
                          SkRect::MakeXYWH(0, 0, maRects.mnSrcWidth, maRects.mnSrcHeight),
                          SkRect::MakeXYWH(0, 0, maRects.mnSrcWidth, maRects.mnSrcHeight), &paint);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
