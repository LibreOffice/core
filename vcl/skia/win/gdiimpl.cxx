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
#include <comphelper/windowserrorstring.hxx>

#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkPixelRef.h>
#include <SkTypeface_win.h>
#include <SkFont.h>
#include <SkFontMgr.h>
#include <tools/sk_app/win/WindowContextFactory_win.h>
#include <tools/sk_app/WindowContext.h>

#include <windows.h>

WinSkiaSalGraphicsImpl::WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics,
                                               SalGeometryProvider* mpProvider)
    : SkiaSalGraphicsImpl(rGraphics, mpProvider)
    , mWinParent(rGraphics)
{
}

void WinSkiaSalGraphicsImpl::createWindowContext(bool forceRaster)
{
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    assert(GetWidth() > 0 && GetHeight() > 0);
    displayParams.fSurfaceProps = *SkiaHelper::surfaceProps();
    switch (forceRaster ? SkiaHelper::RenderRaster : SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderRaster:
            mWindowContext = sk_app::window_context_factory::MakeRasterForWin(mWinParent.gethWnd(),
                                                                              displayParams);
            break;
        case SkiaHelper::RenderVulkan:
            mWindowContext = sk_app::window_context_factory::MakeVulkanForWin(mWinParent.gethWnd(),
                                                                              displayParams);
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

void WinSkiaSalGraphicsImpl::Flush() { performFlush(); }

void WinSkiaSalGraphicsImpl::performFlush()
{
    SkiaZone zone;
    flushDrawing();
    if (mWindowContext)
    {
        if (mDirtyRect.intersect(SkIRect::MakeWH(GetWidth(), GetHeight())))
            mWindowContext->swapBuffers(&mDirtyRect);
        mDirtyRect.setEmpty();
    }
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
    SAL_INFO("vcl.skia.trace", "tryrendercachednativecontrol("
                                   << this << "): "
                                   << SkIRect::MakeXYWH(nX, nY, iterator->second->width(),
                                                        iterator->second->height()));
    addUpdateRegion(
        SkRect::MakeXYWH(nX, nY, iterator->second->width(), iterator->second->height()));
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
    SAL_INFO("vcl.skia.trace",
             "renderandcachednativecontrol("
                 << this << "): " << SkIRect::MakeXYWH(nX, nY, image->width(), image->height()));
    addUpdateRegion(SkRect::MakeXYWH(nX, nY, image->width(), image->height()));
    mSurface->getCanvas()->drawImage(image, nX, nY);
    postDraw();

    if (!aControlCacheKey.canCacheControl())
        return true;
    SkiaControlCachePair pair(aControlCacheKey, std::move(image));
    SkiaControlsCache::get().insert(std::move(pair));
    return true;
}

#ifdef SAL_LOG_INFO
static HRESULT checkResult(HRESULT hr, const char* file, size_t line)
{
    if (FAILED(hr))
    {
        OUString sLocationString
            = OUString::createFromAscii(file) + ":" + OUString::number(line) + " ";
        SAL_DETAIL_LOG_STREAM(SAL_DETAIL_ENABLE_LOG_INFO, ::SAL_DETAIL_LOG_LEVEL_INFO, "vcl.skia",
                              sLocationString.toUtf8().getStr(),
                              "HRESULT failed with: 0x" << OUString::number(hr, 16) << ": "
                                                        << WindowsErrorStringFromHRESULT(hr));
    }
    return hr;
}

#define CHECKHR(funct) checkResult(funct, __FILE__, __LINE__)
#else
#define CHECKHR(funct) (funct)
#endif

sk_sp<SkTypeface> WinSkiaSalGraphicsImpl::createDirectWriteTypeface(HDC hdc, HFONT hfont)
{
    if (!dwriteDone)
    {
        if (SUCCEEDED(
                CHECKHR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                            reinterpret_cast<IUnknown**>(&dwriteFactory)))))
        {
            if (SUCCEEDED(CHECKHR(dwriteFactory->GetGdiInterop(&dwriteGdiInterop))))
                dwriteFontMgr = SkFontMgr_New_DirectWrite(dwriteFactory.get());
            else
                dwriteFactory.clear();
        }
        dwriteDone = true;
    }
    if (!dwriteFontMgr)
        return nullptr;

    // tdf#137122: We need to get the exact same font as HFONT refers to,
    // since VCL core computes things like glyph ids based on that, and getting
    // a different font could lead to mismatches (e.g. if there's a slightly
    // different version of the same font installed system-wide).
    // For that CreateFromFaceFromHdc() is necessary. The simpler
    // CreateFontFromLOGFONT() seems to search for the best matching font,
    // which may not be the exact font. Our private fonts are installed
    // using AddFontResourceExW( FR_PRIVATE ) and that apparently does
    // not make them available to DirectWrite (at least, they are not
    // included the DWrite system font collection). For such cases, we'll
    // need to fall back to Skia's GDI-based font rendering.
    HFONT oldFont = SelectFont(hdc, hfont);
    auto restoreFont = [hdc, oldFont]() { SelectFont(hdc, oldFont); };
    sal::systools::COMReference<IDWriteFontFace> fontFace;
    if (FAILED(CHECKHR(dwriteGdiInterop->CreateFontFaceFromHdc(hdc, &fontFace))))
        return nullptr;
    sal::systools::COMReference<IDWriteFontCollection> collection;
    if (FAILED(CHECKHR(dwriteFactory->GetSystemFontCollection(&collection))))
        return nullptr;
    sal::systools::COMReference<IDWriteFont> font;
    // Do not use CHECKHR() here, as said above, this fails for our fonts.
    if (FAILED(collection->GetFontFromFontFace(fontFace.get(), &font)))
        return nullptr;
    sal::systools::COMReference<IDWriteFontFamily> fontFamily;
    if (FAILED(CHECKHR(font->GetFontFamily(&fontFamily))))
        return nullptr;
    return sk_sp<SkTypeface>(
        SkCreateTypefaceDirectWrite(dwriteFontMgr, fontFace.get(), font.get(), fontFamily.get()));
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
    sk_sp<SkTypeface> typeface = createDirectWriteTypeface(mWinParent.getHDC(), hLayoutFont);
    GlyphOrientation glyphOrientation = GlyphOrientation::Apply;
    if (!typeface) // fall back to GDI text rendering
    {
        typeface.reset(SkCreateTypefaceFromLOGFONT(logFont));
        glyphOrientation = GlyphOrientation::Ignore;
    }
    // lfHeight actually depends on DPI, so it's not really font height as such,
    // but for LOGFONT-based typefaces Skia simply sets lfHeight back to this value
    // directly.
    double fontHeight = logFont.lfHeight;
    if (fontHeight < 0)
        fontHeight = -fontHeight;
    SkFont font(typeface, fontHeight, fHScale, 0);
    font.setEdging(fontEdging);
    assert(dynamic_cast<SkiaSalGraphicsImpl*>(mWinParent.GetImpl()));
    SkiaSalGraphicsImpl* impl = static_cast<SkiaSalGraphicsImpl*>(mWinParent.GetImpl());
    COLORREF color = ::GetTextColor(mWinParent.getHDC());
    Color salColor(GetRValue(color), GetGValue(color), GetBValue(color));
    // The font already is set up to have glyphs rotated as needed.
    impl->drawGenericLayout(rLayout, salColor, font, glyphOrientation);
    return true;
}

SkFont::Edging WinSkiaSalGraphicsImpl::fontEdging;

void WinSkiaSalGraphicsImpl::initFontInfo()
{
    // Skia needs to be explicitly told what kind of antialiasing should be used,
    // get it from system settings. This does not actually matter for the text
    // rendering itself, since Skia has been patched to simply use the setting
    // from the LOGFONT, which gets set by VCL's ImplGetLogFontFromFontSelect()
    // and that one normally uses DEFAULT_QUALITY, so Windows will select
    // the appropriate AA setting. But Skia internally chooses the format to which
    // the glyphs will be rendered based on this setting (subpixel AA requires colors,
    // others do not).
    fontEdging = SkFont::Edging::kAlias;
    SkPixelGeometry pixelGeometry = kUnknown_SkPixelGeometry;
    BOOL set;
    if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &set, 0) && set)
    {
        UINT set2;
        if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &set2, 0)
            && set2 == FE_FONTSMOOTHINGCLEARTYPE)
        {
            fontEdging = SkFont::Edging::kSubpixelAntiAlias;
            if (SystemParametersInfo(SPI_GETFONTSMOOTHINGORIENTATION, 0, &set2, 0)
                && set2 == FE_FONTSMOOTHINGORIENTATIONBGR)
                // No idea how to tell if it's horizontal or vertical.
                pixelGeometry = kBGR_H_SkPixelGeometry;
            else
                pixelGeometry = kRGB_H_SkPixelGeometry; // default
        }
        else
            fontEdging = SkFont::Edging::kAntiAlias;
    }
    SkiaHelper::setPixelGeometry(pixelGeometry);
}

void WinSkiaSalGraphicsImpl::ClearDevFontCache()
{
    dwriteFontMgr.reset();
    dwriteFactory.clear();
    dwriteGdiInterop.clear();
    dwriteDone = false;
    initFontInfo(); // get font info again, just in case
}

SkiaCompatibleDC::SkiaCompatibleDC(SalGraphics& rGraphics, int x, int y, int width, int height)
    : CompatibleDC(rGraphics, x, y, width, height, false)
{
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
    // I doubt this can be done using Skia, so do it manually here. Fortunately
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
    matrix.setConcat(matrix, SkMatrix::Scale(1, -1));
    canvas->concat(matrix);
    canvas->drawImage(tmpBitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
    canvas->restore();
    return SkiaHelper::makeCheckedImageSnapshot(surface);
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

void WinSkiaSalGraphicsImpl::prepareSkia()
{
    initFontInfo();
    SkiaHelper::prepareSkia(createVulkanWindowContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
