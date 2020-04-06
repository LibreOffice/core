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
#include <skia/zone.hxx>
#include <win/winlayout.hxx>

#include <SkCanvas.h>
#include <SkColorFilter.h>
#include <SkPaint.h>
#include <SkPixelRef.h>
#include <SkTypeface_win.h>
#include <SkFont.h>
#include <tools/sk_app/win/WindowContextFactory_win.h>
#include <tools/sk_app/WindowContext.h>

#include <windows.h>

WinSkiaSalGraphicsImpl::WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics,
                                               SalGeometryProvider* mpProvider)
    : SkiaSalGraphicsImpl(rGraphics, mpProvider)
    , mWinParent(rGraphics)
{
}

void WinSkiaSalGraphicsImpl::createWindowContext()
{
    SkiaZone zone;
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
    SkiaZone zone;
    SkiaSalGraphicsImpl::DeInit();
    mWindowContext.reset();
}

void WinSkiaSalGraphicsImpl::freeResources() {}

void WinSkiaSalGraphicsImpl::performFlush()
{
    SkiaZone zone;
    mPendingPixelsToFlush = 0;
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

    sk_sp<SkImage> image = static_cast<SkiaCompatibleDC&>(rBlack).getAsImageDiff(
        static_cast<SkiaCompatibleDC&>(rWhite));
    preDraw();
    mSurface->getCanvas()->drawImage(image, nX, nY);
    postDraw();

    if (!aControlCacheKey.canCacheControl())
        return true;
    SkiaControlCachePair pair(aControlCacheKey, std::move(image));
    SkiaControlsCache::get().insert(std::move(pair));
    return true;
}

bool WinSkiaSalGraphicsImpl::DrawTextLayout(const GenericSalLayout& rLayout)
{
    const WinFontInstance& rWinFont = static_cast<const WinFontInstance&>(rLayout.GetFont());
    float fHScale = rWinFont.getHScale();

    assert(dynamic_cast<const WinFontInstance*>(&rLayout.GetFont()));
    const WinFontInstance* pWinFont = static_cast<const WinFontInstance*>(&rLayout.GetFont());
    const HFONT hLayoutFont = pWinFont->GetHFONT();
    LOGFONTW logFont;
    if (GetObjectW(hLayoutFont, sizeof(logFont), &logFont) == 0)
    {
        assert(false);
        return false;
    }
    sk_sp<SkTypeface> typeface(SkCreateTypefaceFromLOGFONT(logFont));
    // lfHeight actually depends on DPI, so it's not really font height as such,
    // but for LOGFONT-based typefaces Skia simply sets lfHeight back to this value
    // directly.
    double fontHeight = logFont.lfHeight;
    if (fontHeight < 0)
        fontHeight = -fontHeight;
    SkFont font(typeface, fontHeight, fHScale, 0);
    // Skia needs to be explicitly told what kind of antialiasing should be used,
    // get it from system settings. This does not actually matter for the text
    // rendering itself, since Skia has been patched to simply use the setting
    // from the LOGFONT, which gets set by VCL's ImplGetLogFontFromFontSelect()
    // and that one normally uses DEFAULT_QUALITY, so Windows will select
    // the appropriate AA setting. But Skia internally chooses the format to which
    // the glyphs will be rendered based on this setting (subpixel AA requires colors,
    // others do not).
    BOOL set;
    if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &set, 0) && set)
    {
        UINT set2;
        if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &set2, 0)
            && set2 == FE_FONTSMOOTHINGCLEARTYPE)
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        else
            font.setEdging(SkFont::Edging::kAntiAlias);
    }
    assert(dynamic_cast<SkiaSalGraphicsImpl*>(mWinParent.GetImpl()));
    SkiaSalGraphicsImpl* impl = static_cast<SkiaSalGraphicsImpl*>(mWinParent.GetImpl());
    COLORREF color = ::GetTextColor(mWinParent.getHDC());
    Color salColor(GetRValue(color), GetGValue(color), GetBValue(color));
    // The font already is set up to have glyphs rotated as needed.
    impl->drawGenericLayout(rLayout, salColor, font, SkiaSalGraphicsImpl::GlyphOrientation::Ignore);
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
    assert(dynamic_cast<const SkiaCompatibleDC::PackedTexture*>(pTexture));
    const SkiaCompatibleDC::PackedTexture* texture
        = static_cast<const SkiaCompatibleDC::PackedTexture*>(pTexture);
    preDraw();
    SkPaint paint;
    // The glyph is painted as white, modulate it to be of the appropriate color.
    // SkiaCompatibleDC::wantsTextColorWhite() ensures the glyph is white.
    // TODO maybe other black/white in WinFontInstance::CacheGlyphToAtlas() should be swapped.
    paint.setColorFilter(SkColorFilters::Blend(toSkColor(aMaskColor), SkBlendMode::kModulate));
    // We use SkiaPackedSurface, so use also the appropriate rectangle in the source SkSurface.
    const tools::Rectangle& rect = texture->packedSurface.mRect;
    // The source in SalTwoRect is actually just the size.
    assert(rPosAry.mnSrcX == 0 && rPosAry.mnSrcY == 0);
    assert(rPosAry.mnSrcWidth == rect.GetWidth());
    assert(rPosAry.mnSrcHeight == rect.GetHeight());
    mSurface->getCanvas()->drawImageRect(
        texture->packedSurface.mSurface->makeImageSnapshot(),
        SkRect::MakeXYWH(rect.getX(), rect.getY(), rect.GetWidth(), rect.GetHeight()),
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

std::unique_ptr<CompatibleDC::Texture> SkiaCompatibleDC::getAsMaskTexture() const
{
    auto ret = std::make_unique<SkiaCompatibleDC::Texture>();
    ret->image = getAsMaskImage();
    return ret;
}

sk_sp<SkImage> SkiaCompatibleDC::getAsMaskImage() const
{
    SkiaZone zone;
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

sk_sp<SkImage> SkiaCompatibleDC::getAsImage() const
{
    SkiaZone zone;
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

sk_sp<SkImage> SkiaCompatibleDC::getAsImageDiff(const SkiaCompatibleDC& white) const
{
    SkiaZone zone;
    assert(maRects.mnSrcWidth == white.maRects.mnSrcWidth
           || maRects.mnSrcHeight == white.maRects.mnSrcHeight);
    SkBitmap tmpBitmap;
    if (!tmpBitmap.tryAllocPixels(SkImageInfo::Make(maRects.mnSrcWidth, maRects.mnSrcHeight,
                                                    kBGRA_8888_SkColorType, kPremul_SkAlphaType),
                                  maRects.mnSrcWidth * 4))
        abort();
    // Native widgets are drawn twice on black/white background to synthetize alpha
    // (commit c6b66646870cb2bffaa73565affcf80bf74e0b5c). The problem is that
    // most widgets when drawn on transparent background are drawn properly (and the result
    // is in premultiplied alpha format), some such as "Edit" (used by ControlType::Editbox)
    // keep the alpha channel as transparent. Therefore the alpha is actually computed
    // from the difference in the premultiplied red channels when drawn one black and on white.
    // Alpha is computed as "alpha = 1.0 - abs(black.red - white.red)".
    // TODO I doubt this can be done using Skia, so do it manually here. Fortunately
    // the bitmaps should be fairly small and are cached.
    uint32_t* dest = tmpBitmap.getAddr32(0, 0);
    assert(dest == tmpBitmap.getPixels());
    const sal_uInt32* src = mpData;
    const sal_uInt32* whiteSrc = white.mpData;
    uint32_t* end = dest + tmpBitmap.width() * tmpBitmap.height();
    while (dest < end)
    {
        uint32_t alpha = 255 - abs(int(*src & 0xff) - int(*whiteSrc & 0xff));
        *dest = (*src & 0x00ffffff) | (alpha << 24);
        ++dest;
        ++src;
        ++whiteSrc;
    }
    tmpBitmap.notifyPixelsChanged();
    tmpBitmap.setImmutable();
    sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(tmpBitmap.width(), tmpBitmap.height());
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    SkCanvas* canvas = surface->getCanvas();
    canvas->save();
    // The data we got is upside-down.
    SkMatrix matrix;
    matrix.preTranslate(0, tmpBitmap.height());
    matrix.setConcat(matrix, SkMatrix::MakeScale(1, -1));
    canvas->concat(matrix);
    canvas->drawBitmap(tmpBitmap, 0, 0, &paint);
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

namespace
{
std::unique_ptr<sk_app::WindowContext> createVulkanWindowContext(bool /*temporary*/)
{
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    return sk_app::window_context_factory::MakeVulkanForWin(nullptr, displayParams);
}
}

void WinSkiaSalGraphicsImpl::prepareSkia() { SkiaHelper::prepareSkia(createVulkanWindowContext); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
