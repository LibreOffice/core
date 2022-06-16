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

#include <sal/config.h>

#include <skia/osx/gdiimpl.hxx>

#include <skia/utils.hxx>
#include <skia/zone.hxx>

#include <tools/sk_app/mac/WindowContextFactory_mac.h>

#include <quartz/ctfonts.hxx>

#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkFont.h>
#include <SkFontMgr_mac_ct.h>
#include <SkTypeface_mac.h>

using namespace SkiaHelper;

AquaSkiaSalGraphicsImpl::AquaSkiaSalGraphicsImpl(AquaSalGraphics& rParent,
                                                 AquaSharedAttributes& rShared)
    : SkiaSalGraphicsImpl(rParent, rShared.mpFrame)
    , AquaGraphicsBackendBase(rShared)
{
    Init(); // mac code doesn't call Init()
}

AquaSkiaSalGraphicsImpl::~AquaSkiaSalGraphicsImpl()
{
    DeInit(); // mac code doesn't call DeInit()
}

void AquaSkiaSalGraphicsImpl::freeResources() {}

void AquaSkiaSalGraphicsImpl::createWindowSurfaceInternal(bool forceRaster)
{
    assert(!mWindowContext);
    assert(!mSurface);
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    displayParams.fColorType = kN32_SkColorType;
    sk_app::window_context_factory::MacWindowInfo macWindow;
    macWindow.fMainView = mrShared.mpFrame->mpNSView;
    mScaling = getWindowScaling();
    RenderMethod renderMethod = forceRaster ? RenderRaster : renderMethodToUse();
    switch (renderMethod)
    {
        case RenderRaster:
            // RasterWindowContext_mac uses OpenGL internally, which we don't want,
            // so use our own surface and do blitting to the screen ourselves.
            mSurface = createSkSurface(GetWidth() * mScaling, GetHeight() * mScaling);
            break;
        case RenderMetal:
            mWindowContext
                = sk_app::window_context_factory::MakeMetalForMac(macWindow, displayParams);
            // Like with other GPU contexts, create a proxy offscreen surface (see
            // flushSurfaceToWindowContext()). Here it's additionally needed because
            // it appears that Metal surfaces cannot be read from, which would break things
            // like copyArea().
            if (mWindowContext)
                mSurface = createSkSurface(GetWidth() * mScaling, GetHeight() * mScaling);
            break;
        case RenderVulkan:
            abort();
            break;
    }
}

int AquaSkiaSalGraphicsImpl::getWindowScaling() const
{
    // The system function returns float, but only integer multiples realistically make sense.
    return sal::aqua::getWindowScaling();
}

void AquaSkiaSalGraphicsImpl::Flush() { performFlush(); }

void AquaSkiaSalGraphicsImpl::Flush(const tools::Rectangle&) { performFlush(); }

void AquaSkiaSalGraphicsImpl::flushSurfaceToWindowContext()
{
    if (!isGPU())
        flushSurfaceToScreenCG();
    else
        SkiaSalGraphicsImpl::flushSurfaceToWindowContext();
}

constexpr static uint32_t toCGBitmapType(SkColorType color, SkAlphaType alpha)
{
    if (alpha == kPremul_SkAlphaType)
    {
        return color == kBGRA_8888_SkColorType
                   ? (uint32_t(kCGImageAlphaPremultipliedFirst)
                      | uint32_t(kCGBitmapByteOrder32Little))
                   : (uint32_t(kCGImageAlphaPremultipliedLast) | uint32_t(kCGBitmapByteOrder32Big));
    }
    else
    {
        assert(alpha == kOpaque_SkAlphaType);
        return color == kBGRA_8888_SkColorType
                   ? (uint32_t(kCGImageAlphaNoneSkipFirst) | uint32_t(kCGBitmapByteOrder32Little))
                   : (uint32_t(kCGImageAlphaNoneSkipLast) | uint32_t(kCGBitmapByteOrder32Big));
    }
}

// For Raster we use our own screen blitting (see above).
void AquaSkiaSalGraphicsImpl::flushSurfaceToScreenCG()
{
    // Based on AquaGraphicsBackend::drawBitmap().
    if (!mrShared.checkContext())
        return;

    assert(mSurface.get());
    // Do not use sub-rect, it creates copies of the data.
    sk_sp<SkImage> image = makeCheckedImageSnapshot(mSurface);
    SkPixmap pixmap;
    if (!image->peekPixels(&pixmap))
        abort();
    // If window scaling, then mDirtyRect is in VCL coordinates, mSurface has screen size (=points,HiDPI),
    // maContextHolder has screen size but a scale matrix set so its inputs are in VCL coordinates (see
    // its setup in AquaSharedAttributes::checkContext()).
    // This creates the bitmap context from the cropped part, writable_addr32() will get
    // the first pixel of mDirtyRect.topLeft(), and using pixmap.rowBytes() ensures the following
    // pixel lines will be read from correct positions.
    if (pixmap.bounds() != mDirtyRect && pixmap.bounds().bottom() == mDirtyRect.bottom())
    {
        // HACK for tdf#145843: If mDirtyRect includes the last line but not the first pixel of it,
        // then the rowBytes() trick would lead to the GC* functions thinking that even pixels after
        // the pixmap data belong to the area (since the shifted x()+rowBytes() points there) and
        // at least on Intel Mac they would actually read those data, even though I see no good reason
        // to do that, as that's beyond the x()+width() for the last line. That could be handled
        // by creating a subset SkImage (which as is said above copies data), or set the x coordinate
        // to 0, which will then make rowBytes() match the actual data.
        mDirtyRect.fLeft = 0;
        assert(mDirtyRect.width() == pixmap.bounds().width());
    }
    CGContextRef context = CGBitmapContextCreate(
        pixmap.writable_addr32(mDirtyRect.x() * mScaling, mDirtyRect.y() * mScaling),
        mDirtyRect.width() * mScaling, mDirtyRect.height() * mScaling, 8, pixmap.rowBytes(),
        GetSalData()->mxRGBSpace, toCGBitmapType(image->colorType(), image->alphaType()));
    if (!context)
    {
        SAL_WARN("vcl.skia", "flushSurfaceToScreenGC(): Failed to allocate bitmap context");
        return;
    }
    CGImageRef screenImage = CGBitmapContextCreateImage(context);
    if (!screenImage)
    {
        CGContextRelease(context);
        SAL_WARN("vcl.skia", "flushSurfaceToScreenGC(): Failed to allocate screen image");
        return;
    }
    mrShared.maContextHolder.saveState();
    // Drawing to the actual window has scaling active, so use unscaled coordinates, the scaling matrix will scale them
    // to the proper screen coordinates. Unless the scaling is fake for debugging, in which case scale them to draw
    // at the scaled size.
    int windowScaling = 1;
    static const char* env = getenv("SAL_FORCE_HIDPI_SCALING");
    if (env != nullptr)
        windowScaling = atoi(env);
    CGRect drawRect
        = CGRectMake(mDirtyRect.x() * windowScaling, mDirtyRect.y() * windowScaling,
                     mDirtyRect.width() * windowScaling, mDirtyRect.height() * windowScaling);
    if (mrShared.isFlipped())
    {
        // I don't understand why, but apparently it's needed to explicitly to flip the drawing, even though maContextHelper
        // has this set up, so this unsets the flipping.
        CGFloat invertedY = drawRect.origin.y + drawRect.size.height;
        CGContextTranslateCTM(mrShared.maContextHolder.get(), 0, invertedY);
        CGContextScaleCTM(mrShared.maContextHolder.get(), 1, -1);
        drawRect.origin.y = 0;
    }
    CGContextDrawImage(mrShared.maContextHolder.get(), drawRect, screenImage);
    mrShared.maContextHolder.restoreState();

    CGImageRelease(screenImage);
    CGContextRelease(context);
    // This is also in VCL coordinates.
    mrShared.refreshRect(mDirtyRect.x(), mDirtyRect.y(), mDirtyRect.width(), mDirtyRect.height());
}

bool AquaSkiaSalGraphicsImpl::drawNativeControl(ControlType nType, ControlPart nPart,
                                                const tools::Rectangle& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue)
{
    // rControlRegion is not the whole area that the control should be painted to (e.g. highlight
    // around focused lineedit is outside of it). Since we draw to a temporary bitmap, we need tofind out
    // the real size. Using getNativeControlRegion() might seem like the function to call, but we need
    // the other direction - what is called rControlRegion here is rNativeContentRegion in that function
    // what's called rControlRegion there is what we need here. Moreover getNativeControlRegion()
    // in some cases returns a fixed size that does not depend on its input, so we have no way to
    // actually find out what the original size was (or maybe the function is kind of broken, I don't know).
    // So, add a generous margin and hope it's enough.
    tools::Rectangle boundingRegion(rControlRegion);
    boundingRegion.expand(50 * mScaling);
    // Do a scaled bitmap in HiDPI in order not to lose precision.
    const tools::Long width = boundingRegion.GetWidth() * mScaling;
    const tools::Long height = boundingRegion.GetHeight() * mScaling;
    const size_t bytes = width * height * 4;
    std::unique_ptr<sal_uInt8[]> data(new sal_uInt8[bytes]);
    memset(data.get(), 0, bytes);
    CGContextRef context = CGBitmapContextCreate(
        data.get(), width, height, 8, width * 4, GetSalData()->mxRGBSpace,
        toCGBitmapType(mSurface->imageInfo().colorType(), kPremul_SkAlphaType));
    if (!context)
    {
        SAL_WARN("vcl.skia", "drawNativeControl(): Failed to allocate bitmap context");
        return false;
    }
    // Setup context state for drawing (performDrawNativeControl() e.g. fills background in some cases).
    CGContextSetFillColorSpace(context, GetSalData()->mxRGBSpace);
    CGContextSetStrokeColorSpace(context, GetSalData()->mxRGBSpace);
    RGBAColor lineColor(mLineColor);
    CGContextSetRGBStrokeColor(context, lineColor.GetRed(), lineColor.GetGreen(),
                               lineColor.GetBlue(), lineColor.GetAlpha());
    RGBAColor fillColor(mFillColor);
    CGContextSetRGBFillColor(context, fillColor.GetRed(), fillColor.GetGreen(), fillColor.GetBlue(),
                             fillColor.GetAlpha());
    // Adjust for our drawn-to coordinates in the bitmap.
    tools::Rectangle movedRegion(Point(rControlRegion.getX() - boundingRegion.getX(),
                                       rControlRegion.getY() - boundingRegion.getY()),
                                 rControlRegion.GetSize());
    // Flip drawing upside down.
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1, -1);
    // And possibly scale the native drawing.
    CGContextScaleCTM(context, mScaling, mScaling);
    bool bOK = performDrawNativeControl(nType, nPart, movedRegion, nState, aValue, context,
                                        mrShared.mpFrame);
    CGContextRelease(context);
    if (bOK)
    {
        SkBitmap bitmap;
        if (!bitmap.installPixels(SkImageInfo::Make(width, height,
                                                    mSurface->imageInfo().colorType(),
                                                    kPremul_SkAlphaType),
                                  data.get(), width * 4))
            abort();

        preDraw();
        SAL_INFO("vcl.skia.trace", "drawnativecontrol(" << this << "): " << rControlRegion << ":"
                                                        << int(nType) << "/" << int(nPart));
        tools::Rectangle updateRect = boundingRegion;
        // For background update only part that is not clipped, the same
        // as in AquaGraphicsBackend::drawNativeControl().
        if (nType == ControlType::WindowBackground)
            updateRect.Intersection(mClipRegion.GetBoundRect());
        addUpdateRegion(SkRect::MakeXYWH(updateRect.getX(), updateRect.getY(),
                                         updateRect.GetWidth(), updateRect.GetHeight()));
        SkRect drawRect = SkRect::MakeXYWH(boundingRegion.getX(), boundingRegion.getY(),
                                           boundingRegion.GetWidth(), boundingRegion.GetHeight());
        assert(drawRect.width() * mScaling == bitmap.width()); // no scaling should be needed
        getDrawCanvas()->drawImageRect(bitmap.asImage(), drawRect, SkSamplingOptions());
        ++pendingOperationsToFlush; // tdf#136369
        postDraw();
    }
    return bOK;
}

void AquaSkiaSalGraphicsImpl::drawTextLayout(const GenericSalLayout& rLayout,
                                             bool bSubpixelPositioning)
{
    const CoreTextStyle& rStyle = *static_cast<const CoreTextStyle*>(&rLayout.GetFont());
    const vcl::font::FontSelectPattern& rFontSelect = rStyle.GetFontSelectPattern();
    int nHeight = rFontSelect.mnHeight;
    int nWidth = rFontSelect.mnWidth ? rFontSelect.mnWidth : nHeight;
    if (nWidth == 0 || nHeight == 0)
    {
        SAL_WARN("vcl.skia", "DrawTextLayout(): rFontSelect.mnHeight is zero!?");
        return;
    }

    if (!fontManager)
    {
        SystemFontList* fontList = GetCoretextFontList();
        if (fontList == nullptr)
        {
            SAL_WARN("vcl.skia", "DrawTextLayout(): No coretext font list");
            fontManager = SkFontMgr_New_CoreText(nullptr);
        }
        else
        {
            fontManager = SkFontMgr_New_CoreText(fontList->fontCollection());
        }
    }

    CTFontRef pFont
        = static_cast<CTFontRef>(CFDictionaryGetValue(rStyle.GetStyleDict(), kCTFontAttributeName));
    sk_sp<SkTypeface> typeface = SkMakeTypefaceFromCTFont(pFont);
    SkFont font(typeface);
    font.setSize(nHeight);
    //    font.setScaleX(rStyle.mfFontStretch); TODO
    if (rStyle.mbFauxBold)
        font.setEmbolden(true);

    SkFont::Edging ePreferredAliasing
        = bSubpixelPositioning ? SkFont::Edging::kSubpixelAntiAlias : SkFont::Edging::kAntiAlias;
    if (bSubpixelPositioning)
        font.setSubpixel(true);
    font.setEdging(mrShared.mbNonAntialiasedText ? SkFont::Edging::kAlias : ePreferredAliasing);

    // Vertical font, use width as "height".
    SkFont verticalFont(font);
    verticalFont.setSize(nHeight);
    //    verticalFont.setSize(nWidth); TODO
    //    verticalFont.setScaleX(1.0 * nHeight / nWidth);

    drawGenericLayout(rLayout, mrShared.maTextColor, font, verticalFont);
}

namespace
{
std::unique_ptr<sk_app::WindowContext> createMetalWindowContext(bool /*temporary*/)
{
    sk_app::DisplayParams displayParams;
    sk_app::window_context_factory::MacWindowInfo macWindow;
    macWindow.fMainView = nullptr;
    return sk_app::window_context_factory::MakeMetalForMac(macWindow, displayParams);
}
}

void AquaSkiaSalGraphicsImpl::prepareSkia() { SkiaHelper::prepareSkia(createMetalWindowContext); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
