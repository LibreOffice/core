/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <skia/win/gdiimpl.hxx>

#include <win/saldata.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <skia/utils.hxx>

#include <SkColorFilter.h>
#include <SkPixelRef.h>
#include <tools/sk_app/win/WindowContextFactory_win.h>
#include <tools/sk_app/WindowContext.h>

WinSkiaSalGraphicsImpl::WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics,
                                               SalGeometryProvider* mpProvider)
    : SkiaSalGraphicsImpl(rGraphics, mpProvider)
    , mWinParent(rGraphics)
{
}

#if 0 // TODO
void WinSkiaSalGraphicsImpl::Init()
{
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
}
#endif

void WinSkiaSalGraphicsImpl::createWindowContext()
{
    // When created, Init() gets called with size (0,0), which is invalid size
    // for Skia. Creating the actual surface is delayed, so the size should be always
    // valid here, but better check.
    assert((GetWidth() != 0 && GetHeight() != 0) || isOffscreen());
    sk_app::DisplayParams displayParams;
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderRaster:
            mWindowContext = sk_app::window_context_factory::MakeRasterForWin(mWinParent.gethWnd(),
                                                                              displayParams);
            mIsGPU = false;
            break;
        case SkiaHelper::RenderVulkan:
            mWindowContext = sk_app::window_context_factory::MakeVulkanForWin(mWinParent.gethWnd(),
                                                                              displayParams);
            mIsGPU = true;
            break;
    }
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
    mSurface->getCanvas()->drawImage(iterator->second, nX, nY);
    postDraw();
    return true;
}

bool WinSkiaSalGraphicsImpl::RenderAndCacheNativeControl(CompatibleDC& rWhite, CompatibleDC& rBlack,
                                                         int nX, int nY,
                                                         ControlCacheKey& aControlCacheKey)
{
    assert(dynamic_cast<SkiaCompatibleDC*>(&rWhite));
    assert(dynamic_cast<SkiaCompatibleDC*>(&rBlack));

    sk_sp<SkImage> image = static_cast<SkiaCompatibleDC&>(rWhite).getAsImage();
    preDraw();
    mSurface->getCanvas()->drawImage(image, nX, nY);
    postDraw();
    // TODO what is the point of the second texture?
    (void)rBlack;

    if (!aControlCacheKey.canCacheControl())
        return true;
    SkiaControlCachePair pair(aControlCacheKey, std::move(image));
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
    mSurface->getCanvas()->drawImageRect(
        static_cast<const SkiaCompatibleDC::Texture*>(pTexture)->image,
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
    drawMask(rPosAry, static_cast<const SkiaCompatibleDC::Texture*>(pTexture)->image, nMaskColor);
}

SkiaCompatibleDC::SkiaCompatibleDC(SalGraphics& rGraphics, int x, int y, int width, int height)
    : CompatibleDC(rGraphics, x, y, width, height, false)
{
}

std::unique_ptr<CompatibleDC::Texture> SkiaCompatibleDC::getAsMaskTexture()
{
    auto ret = std::make_unique<SkiaCompatibleDC::Texture>();
    ret->image = getAsMaskImage();
    return ret;
}

sk_sp<SkImage> SkiaCompatibleDC::getAsMaskImage()
{
    // mpData is in the BGRA format, with A unused (and set to 0), and RGB are grey,
    // so convert it to Skia format, then to 8bit and finally use as alpha mask
    SkBitmap tmpBitmap;
    if (!tmpBitmap.installPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                   kBGRA_8888_SkColorType, kOpaque_SkAlphaType),
                                 mpData, maRects.mnSrcWidth * 4))
        abort();
    tmpBitmap.setImmutable();
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
    bitmap8.setImmutable();
    // use the 8bit data as an alpha channel
    SkBitmap alpha;
    alpha.setInfo(bitmap8.info().makeColorType(kAlpha_8_SkColorType), bitmap8.rowBytes());
    alpha.setPixelRef(sk_ref_sp(bitmap8.pixelRef()), bitmap8.pixelRefOrigin().x(),
                      bitmap8.pixelRefOrigin().y());
    alpha.setImmutable();
    sk_sp<SkSurface> surface
        = SkiaHelper::createSkSurface(alpha.width(), alpha.height(), kAlpha_8_SkColorType);
    // https://bugs.chromium.org/p/skia/issues/detail?id=9692
    // Raster kAlpha_8_SkColorType surfaces need empty contents for SkBlendMode::kSrc.
    if (!surface->getCanvas()->getGrContext())
        surface->getCanvas()->clear(SkColorSetARGB(0x00, 0x00, 0x00, 0x00));
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    surface->getCanvas()->drawBitmap(alpha, 0, 0, &paint);
    return surface->makeImageSnapshot();
}

sk_sp<SkImage> SkiaCompatibleDC::getAsImage()
{
    SkBitmap tmpBitmap;
    if (!tmpBitmap.installPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                   kBGRA_8888_SkColorType, kUnpremul_SkAlphaType),
                                 mpData, maRects.mnSrcWidth * 4))
        abort();
    tmpBitmap.setImmutable();
    sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(tmpBitmap.width(), tmpBitmap.height());
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    SkCanvas* canvas = surface->getCanvas();
    canvas->save();
    // The data we got is upside-down.
    SkMatrix matrix;
    matrix.preTranslate(0, maRects.mnSrcHeight);
    matrix.setConcat(matrix, SkMatrix::MakeScale(1, -1));
    canvas->concat(matrix);
    canvas->drawBitmapRect(tmpBitmap,
                           SkRect::MakeXYWH(0, 0, maRects.mnSrcWidth, maRects.mnSrcHeight),
                           SkRect::MakeXYWH(0, 0, maRects.mnSrcWidth, maRects.mnSrcHeight), &paint);
    canvas->restore();
    return surface->makeImageSnapshot();
}

SkiaControlsCache::SkiaControlsCache()
    : cache(200)
{
}

SkiaControlCacheType& SkiaControlsCache::get()
{
    SalData* data = GetSalData();
    if (!data->m_pSkiaControlsCache)
        data->m_pSkiaControlsCache.reset(new SkiaControlsCache);
    return data->m_pSkiaControlsCache->cache;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
