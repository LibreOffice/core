/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <skia/win/gdiimpl.hxx>

#include <win/saldata.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <skia/utils.hxx>
#include <skia/zone.hxx>
#include <skia/win/font.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/windowserrorstring.hxx>
#include <sal/log.hxx>

#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkPixelRef.h>
#include <SkTypeface_win.h>
#include <SkFont.h>
#include <SkFontMgr.h>
#include <tools/sk_app/win/WindowContextFactory_win.h>
#include <tools/sk_app/WindowContext.h>

#include <windows.h>

using namespace SkiaHelper;

WinSkiaSalGraphicsImpl::WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics,
                                               SalGeometryProvider* mpProvider)
    : SkiaSalGraphicsImpl(rGraphics, mpProvider)
    , mWinParent(rGraphics)
{
}

void WinSkiaSalGraphicsImpl::createWindowSurfaceInternal(bool forceRaster)
{
    assert(!mWindowContext);
    assert(!mSurface);
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    assert(GetWidth() > 0 && GetHeight() > 0);
    displayParams.fSurfaceProps = *surfaceProps();
    switch (forceRaster ? RenderRaster : renderMethodToUse())
    {
        case RenderRaster:
            mWindowContext = sk_app::window_context_factory::MakeRasterForWin(mWinParent.gethWnd(),
                                                                              displayParams);
            if (mWindowContext)
                mSurface = mWindowContext->getBackbufferSurface();
            break;
        case RenderVulkan:
            mWindowContext = sk_app::window_context_factory::MakeVulkanForWin(mWinParent.gethWnd(),
                                                                              displayParams);
            // See flushSurfaceToWindowContext().
            if (mWindowContext)
                mSurface = createSkSurface(GetWidth(), GetHeight());
            break;
        case RenderMetal:
            abort();
            break;
    }
}

void WinSkiaSalGraphicsImpl::freeResources() {}

void WinSkiaSalGraphicsImpl::Flush() { performFlush(); }

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

sk_sp<SkTypeface>
WinSkiaSalGraphicsImpl::createDirectWriteTypeface(const WinFontInstance* pWinFont) try
{
    using sal::systools::ThrowIfFailed;
    IDWriteFactory* dwriteFactory;
    WinSalGraphics::getDWriteFactory(&dwriteFactory);
    if (!dwriteDone)
    {
        dwriteFontMgr = SkFontMgr_New_DirectWrite(dwriteFactory);
        dwriteDone = true;
    }
    if (!dwriteFontMgr)
        return nullptr;

    IDWriteFontFace* fontFace = pWinFont->GetDWFontFace();
    if (!fontFace)
        return nullptr;

    sal::systools::COMReference<IDWriteFontCollection> collection;
    ThrowIfFailed(dwriteFactory->GetSystemFontCollection(&collection), SAL_WHERE);
    sal::systools::COMReference<IDWriteFont> font;
    // As said above, this fails for our fonts.
    if (FAILED(collection->GetFontFromFontFace(fontFace, &font)))
    {
        // If not found in system collection, try our private font collection.
        // If that's not possible we'll fall back to Skia's GDI-based font rendering.
        if (!dwritePrivateCollection
            || FAILED(dwritePrivateCollection->GetFontFromFontFace(fontFace, &font)))
        {
            // Our private fonts are installed using AddFontResourceExW( FR_PRIVATE )
            // and that does not make them available to the DWrite system font
            // collection. For such cases attempt to update a collection of
            // private fonts with this newly used font.

            sal::systools::COMReference<IDWriteFactory3> dwriteFactory3;
            ThrowIfFailed(dwriteFactory->QueryInterface(&dwriteFactory3), SAL_WHERE);

            if (!dwriteFontSetBuilder)
                ThrowIfFailed(dwriteFactory3->CreateFontSetBuilder(&dwriteFontSetBuilder),
                              SAL_WHERE);

            UINT32 numberOfFiles;
            ThrowIfFailed(fontFace->GetFiles(&numberOfFiles, nullptr), SAL_WHERE);
            if (numberOfFiles != 1)
                return nullptr;

            sal::systools::COMReference<IDWriteFontFile> fontFile;
            ThrowIfFailed(fontFace->GetFiles(&numberOfFiles, &fontFile), SAL_WHERE);

            BOOL isSupported;
            DWRITE_FONT_FILE_TYPE fileType;
            UINT32 numberOfFonts;
            ThrowIfFailed(fontFile->Analyze(&isSupported, &fileType, nullptr, &numberOfFonts),
                          SAL_WHERE);
            if (!isSupported)
                return nullptr;

            // For each font within the font file, get a font face reference and add to the builder.
            for (UINT32 fontIndex = 0; fontIndex < numberOfFonts; ++fontIndex)
            {
                sal::systools::COMReference<IDWriteFontFaceReference> fontFaceReference;
                if (FAILED(dwriteFactory3->CreateFontFaceReference(fontFile.get(), fontIndex,
                                                                   DWRITE_FONT_SIMULATIONS_NONE,
                                                                   &fontFaceReference)))
                    continue;

                // Leave it to DirectWrite to read properties directly out of the font files
                dwriteFontSetBuilder->AddFontFaceReference(fontFaceReference.get());
            }

            sal::systools::COMReference<IDWriteFontSet> fontSet;
            ThrowIfFailed(dwriteFontSetBuilder->CreateFontSet(&fontSet), SAL_WHERE);
            ThrowIfFailed(dwriteFactory3->CreateFontCollectionFromFontSet(fontSet.get(),
                                                                          &dwritePrivateCollection),
                          SAL_WHERE);
            ThrowIfFailed(dwritePrivateCollection->GetFontFromFontFace(fontFace, &font), SAL_WHERE);
        }
    }
    sal::systools::COMReference<IDWriteFontFamily> fontFamily;
    ThrowIfFailed(font->GetFontFamily(&fontFamily), SAL_WHERE);
    return sk_sp<SkTypeface>(
        SkCreateTypefaceDirectWrite(dwriteFontMgr, fontFace, font.get(), fontFamily.get()));
}
catch (const sal::systools::ComError& e)
{
    SAL_DETAIL_LOG_STREAM(SAL_DETAIL_ENABLE_LOG_INFO, ::SAL_DETAIL_LOG_LEVEL_INFO, "vcl.skia",
                          e.what(),
                          "HRESULT 0x" << OUString::number(e.GetHresult(), 16) << ": "
                                       << WindowsErrorStringFromHRESULT(e.GetHresult()));
    return nullptr;
}

bool WinSkiaSalGraphicsImpl::DrawTextLayout(const GenericSalLayout& rLayout)
{
    assert(dynamic_cast<SkiaWinFontInstance*>(&rLayout.GetFont()));
    SkiaWinFontInstance& rWinFont = static_cast<SkiaWinFontInstance&>(rLayout.GetFont());
    const vcl::font::FontSelectPattern& rFSD = rWinFont.GetFontSelectPattern();
    if (rFSD.mnHeight == 0)
        return false;
    const HFONT hLayoutFont = rWinFont.GetHFONT();
    LOGFONTW logFont;
    if (GetObjectW(hLayoutFont, sizeof(logFont), &logFont) == 0)
    {
        assert(false);
        return false;
    }
    sk_sp<SkTypeface> typeface = rWinFont.GetSkiaTypeface();
    if (!typeface)
    {
        typeface = createDirectWriteTypeface(&rWinFont);
        bool dwrite = true;
        if (!typeface) // fall back to GDI text rendering
        {
            // If lfWidth is kept, then with hScale != 1 characters get too wide, presumably
            // because the horizontal scaling gets applied twice if GDI is used for drawing (tdf#141715).
            // Using lfWidth /= hScale gives slightly incorrect sizes, for a reason I don't understand.
            // LOGFONT docs say that 0 means GDI will find out the right value on its own somehow,
            // and it apparently works.
            logFont.lfWidth = 0;
            // Reset LOGFONT orientation, the proper orientation is applied by drawGenericLayout(),
            // and keeping this would make it get applied once more when doing the actual GDI drawing.
            // Resetting it here does not seem to cause any problem.
            logFont.lfOrientation = 0;
            logFont.lfEscapement = 0;
            typeface = SkCreateTypefaceFromLOGFONT(logFont);
            dwrite = false;
            if (!typeface)
                return false;
        }
        // Cache the typeface.
        rWinFont.SetSkiaTypeface(typeface, dwrite);
    }

    SkFont font(typeface);

    bool bSubpixelPositioning = rLayout.GetSubpixelPositioning();
    SkFont::Edging ePreferredAliasing
        = bSubpixelPositioning ? SkFont::Edging::kSubpixelAntiAlias : fontEdging;
    if (bSubpixelPositioning)
    {
        // note that SkFont defaults to a BaselineSnap of true, so I think really only
        // subpixel in text direction
        font.setSubpixel(true);
    }
    font.setEdging(logFont.lfQuality == NONANTIALIASED_QUALITY ? SkFont::Edging::kAlias
                                                               : ePreferredAliasing);

    double nHeight = rFSD.mnHeight;
    double nWidth = rFSD.mnWidth ? rFSD.mnWidth * rWinFont.GetAverageWidthFactor() : nHeight;
    font.setSize(nHeight);
    font.setScaleX(nWidth / nHeight);

    SkFont verticalFont(font);
    verticalFont.setSize(nWidth);
    verticalFont.setScaleX(nHeight / nWidth);

    assert(dynamic_cast<SkiaSalGraphicsImpl*>(mWinParent.GetImpl()));
    SkiaSalGraphicsImpl* impl = static_cast<SkiaSalGraphicsImpl*>(mWinParent.GetImpl());
    COLORREF color = ::GetTextColor(mWinParent.getHDC());
    Color salColor(GetRValue(color), GetGValue(color), GetBValue(color));
    impl->drawGenericLayout(rLayout, salColor, font, verticalFont);
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
    setPixelGeometry(pixelGeometry);
}

void WinSkiaSalGraphicsImpl::ClearDevFontCache()
{
    dwriteFontMgr.reset();
    dwriteFontSetBuilder.clear();
    dwritePrivateCollection.clear();
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
    sk_sp<SkSurface> surface = createSkSurface(tmpBitmap.width(), tmpBitmap.height());
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
    return makeCheckedImageSnapshot(surface);
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

void WinSkiaSalGraphicsImpl::ClearNativeControlCache()
{
    SalData* data = GetSalData();
    data->m_pSkiaControlsCache.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
