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
#include <win/saldata.hxx>

#include <SkColorFilter.h>
#include <SkPixelRef.h>

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
    destroySurface();
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
#ifdef DBG_UTIL
    prefillSurface();
#endif
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
    static bool gbCacheEnabled = !getenv("SAL_WITHOUT_WIDGET_CACHE");
    if (!gbCacheEnabled)
        return false;

    auto& controlsCache = SkiaControlsCache::get();
    SkiaControlCacheType::const_iterator iterator = controlsCache.find(rControlCacheKey);
    if (iterator == controlsCache.end())
        return false;

    preDraw();
    mSurface->getCanvas()->drawBitmap(iterator->second, nX, nY);
    postDraw();
    return true;
}

bool WinSkiaSalGraphicsImpl::RenderAndCacheNativeControl(CompatibleDC& rWhite, CompatibleDC& rBlack,
                                                         int nX, int nY,
                                                         ControlCacheKey& aControlCacheKey)
{
    assert(dynamic_cast<SkiaCompatibleDC*>(&rWhite));
    assert(dynamic_cast<SkiaCompatibleDC*>(&rBlack));

    SkBitmap bitmap = static_cast<SkiaCompatibleDC&>(rWhite).getAsBitmap();
    preDraw();
    mSurface->getCanvas()->drawBitmap(bitmap, nX, nY);
    postDraw();
    // TODO what is the point of the second texture?
    (void)rBlack;

    if (!aControlCacheKey.canCacheControl())
        return true;
    SkiaControlCachePair pair(aControlCacheKey, std::move(bitmap));
    SkiaControlsCache::get().insert(std::move(pair));
    return true;
}

void WinSkiaSalGraphicsImpl::PreDrawText() { preDraw(); }

void WinSkiaSalGraphicsImpl::PostDrawText() { postDraw(); }

static SkColor toSkColor(Color color)
{
    return SkColorSetARGB(255 - color.GetTransparency(), color.GetRed(), color.GetGreen(),
                          color.GetBlue());
}

void WinSkiaSalGraphicsImpl::DeferredTextDraw(const CompatibleDC::Texture* pTexture,
                                              Color aMaskColor, const SalTwoRect& rPosAry)
{
    assert(dynamic_cast<const SkiaCompatibleDC::Texture*>(pTexture));
    preDraw();
    SkPaint paint;
    // The glyph is painted as white, modulate it to be of the appropriate color.
    // SkiaCompatibleDC::wantsTextColorWhite() ensures the glyph is white.
    // TODO maybe other black/white in WinFontInstance::CacheGlyphToAtlas() should be swapped.
    paint.setColorFilter(SkColorFilters::Blend(toSkColor(aMaskColor), SkBlendMode::kModulate));
    mSurface->getCanvas()->drawBitmapRect(
        static_cast<const SkiaCompatibleDC::Texture*>(pTexture)->bitmap,
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        &paint);
    postDraw();
}

void WinSkiaSalGraphicsImpl::DrawTextMask(CompatibleDC::Texture* pTexture, Color nMaskColor,
                                          const SalTwoRect& rPosAry)
{
    assert(dynamic_cast<SkiaCompatibleDC::Texture*>(pTexture));
    drawMask(rPosAry, static_cast<const SkiaCompatibleDC::Texture*>(pTexture)->bitmap, nMaskColor);
}

SkiaCompatibleDC::SkiaCompatibleDC(SalGraphics& rGraphics, int x, int y, int width, int height)
    : CompatibleDC(rGraphics, x, y, width, height, false)
{
}

std::unique_ptr<CompatibleDC::Texture> SkiaCompatibleDC::getAsMaskTexture()
{
    auto ret = std::make_unique<SkiaCompatibleDC::Texture>();
    ret->bitmap = getAsMaskBitmap();
    return ret;
}

SkBitmap SkiaCompatibleDC::getAsMaskBitmap()
{
    // mpData is in the BGRA format, with A unused (and set to 0), and RGB are grey,
    // so convert it to Skia format, then to 8bit and finally use as alpha mask
    SkBitmap tmpBitmap;
    if (!tmpBitmap.installPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                   kBGRA_8888_SkColorType, kOpaque_SkAlphaType),
                                 mpData, maRects.mnSrcWidth * 4))
        abort();
    SkBitmap bitmap8;
    if (!bitmap8.tryAllocPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                  kGray_8_SkColorType, kOpaque_SkAlphaType)))
        abort();
    SkCanvas canvas8(bitmap8);
    SkPaint paint8;
    paint8.setBlendMode(SkBlendMode::kSrc); // copy and convert depth
    // The data we got is upside-down.
    SkMatrix matrix;
    matrix.preTranslate(0, maRects.mnSrcHeight);
    matrix.setConcat(matrix, SkMatrix::MakeScale(1, -1));
    canvas8.concat(matrix);
    canvas8.drawBitmap(tmpBitmap, 0, 0, &paint8);
    // use the 8bit data as an alpha channel
    SkBitmap alpha;
    alpha.setInfo(bitmap8.info().makeColorType(kAlpha_8_SkColorType), bitmap8.rowBytes());
    alpha.setPixelRef(sk_ref_sp(bitmap8.pixelRef()), bitmap8.pixelRefOrigin().x(),
                      bitmap8.pixelRefOrigin().y());
    return alpha;
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

SkBitmap SkiaCompatibleDC::getAsBitmap()
{
    SkBitmap tmpBitmap;
    if (!tmpBitmap.installPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                   kBGRA_8888_SkColorType, kUnpremul_SkAlphaType),
                                 mpData, maRects.mnSrcWidth * 4))
        abort();
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(tmpBitmap.info()))
        abort();
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
    return bitmap;
}

SkiaControlsCache::SkiaControlsCache()
    : cache(200)
{
}

SkiaControlCacheType& SkiaControlsCache::get()
{
    SalData* data = GetSalData();
    if (!data->m_pSkiaControlsCache)
    {
        data->m_pSkiaControlsCache.reset(new SkiaControlsCache);
    }
    return data->m_pSkiaControlsCache->cache;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
