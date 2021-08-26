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

#include <skia/osx/gdiimpl.hxx>

#include <skia/utils.hxx>
#include <skia/zone.hxx>

#include <tools/sk_app/mac/WindowContextFactory_mac.h>

#include <quartz/ctfonts.hxx>

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
    RenderMethod renderMethod = forceRaster ? RenderRaster : renderMethodToUse();
    switch (renderMethod)
    {
        case RenderRaster:
            // RasterWindowContext_mac uses OpenGL internally, which we don't want,
            // so use our own surface and do blitting to the screen ourselves.
            mSurface = createSkSurface(GetWidth(), GetHeight());
            break;
        case RenderMetal:
            mWindowContext
                = sk_app::window_context_factory::MakeMetalForMac(macWindow, displayParams);
            // Like with other GPU contexts, create a proxy offscreen surface (see
            // flushSurfaceToWindowContext()). Here it's additionally needed because
            // it appears that Metal surfaces cannot be read from, which would break things
            // like copyArea().
            if (mWindowContext)
                mSurface = createSkSurface(GetWidth(), GetHeight());
            break;
        case RenderVulkan:
            abort();
            break;
    }
}

void AquaSkiaSalGraphicsImpl::Flush() { performFlush(); }

void AquaSkiaSalGraphicsImpl::Flush(const tools::Rectangle&) { performFlush(); }

void AquaSkiaSalGraphicsImpl::performFlush()
{
    SkiaZone zone;
    flushDrawing();
    if (mSurface)
    {
        if (mDirtyRect.intersect(SkIRect::MakeWH(GetWidth(), GetHeight())))
        {
            if (!isGPU())
                flushSurfaceToScreenCG(mDirtyRect);
            else
                flushSurfaceToWindowContext(mDirtyRect);
        }
        mDirtyRect.setEmpty();
    }
}

constexpr static uint32_t toCGBitmapType(SkColorType color, SkAlphaType alpha)
{
    if (alpha == kPremul_SkAlphaType)
    {
        return color == kBGRA_8888_SkColorType
                   ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little)
                   : (kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    }
    else
    {
        assert(alpha == kOpaque_SkAlphaType);
        return color == kBGRA_8888_SkColorType
                   ? (kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little)
                   : (kCGImageAlphaNoneSkipLast | kCGBitmapByteOrder32Big);
    }
}

// For Raster we use our own screen blitting (see above).
void AquaSkiaSalGraphicsImpl::flushSurfaceToScreenCG(const SkIRect& rect)
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
    // This creates the bitmap context from the cropped part, writable_addr32() will get
    // the first pixel of rect.topLeft(), and using pixmap.rowBytes() ensures the following
    // pixel lines will be read from correct positions.
    CGContextRef context
        = CGBitmapContextCreate(pixmap.writable_addr32(rect.x(), rect.y()), rect.width(),
                                rect.height(), 8, pixmap.rowBytes(), GetSalData()->mxRGBSpace,
                                toCGBitmapType(image->colorType(), image->alphaType()));
    assert(context); // TODO
    CGImageRef screenImage = CGBitmapContextCreateImage(context);
    assert(screenImage); // TODO
    if (mrShared.isFlipped())
    {
        const CGRect screenRect = CGRectMake(rect.x(), GetHeight() - rect.y() - rect.height(),
                                             rect.width(), rect.height());
        mrShared.maContextHolder.saveState();
        CGContextTranslateCTM(mrShared.maContextHolder.get(), 0, pixmap.height());
        CGContextScaleCTM(mrShared.maContextHolder.get(), 1, -1);
        CGContextDrawImage(mrShared.maContextHolder.get(), screenRect, screenImage);
        mrShared.maContextHolder.restoreState();
    }
    else
    {
        const CGRect screenRect = CGRectMake(rect.x(), rect.y(), rect.width(), rect.height());
        CGContextDrawImage(mrShared.maContextHolder.get(), screenRect, screenImage);
    }

    CGImageRelease(screenImage);
    CGContextRelease(context);
    mrShared.refreshRect(rect.x(), rect.y(), rect.width(), rect.height());
}

bool AquaSkiaSalGraphicsImpl::drawNativeControl(ControlType nType, ControlPart nPart,
                                                const tools::Rectangle& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue)
{
    const tools::Long width = rControlRegion.GetWidth();
    const tools::Long height = rControlRegion.GetHeight();
    const size_t bytes = width * height * 4;
    std::unique_ptr<sal_uInt8[]> data(new sal_uInt8[bytes]);
    memset(data.get(), 0, bytes);
    CGContextRef context = CGBitmapContextCreate(
        data.get(), width, height, 8, width * 4, GetSalData()->mxRGBSpace,
        toCGBitmapType(mSurface->imageInfo().colorType(), kPremul_SkAlphaType));
    assert(context); // TODO
    // Flip upside down.
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1, -1);
    // Adjust for our drawn-to coordinates in the bitmap.
    tools::Rectangle movedRegion = rControlRegion;
    movedRegion.SetPos(Point(0, 0));
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
        tools::Rectangle updateRect = rControlRegion;
        // For background update only part that is not clipped, the same
        // as in AquaGraphicsBackend::drawNativeControl().
        if (nType == ControlType::WindowBackground)
            updateRect.Intersection(mClipRegion.GetBoundRect());
        addUpdateRegion(SkRect::MakeXYWH(updateRect.getX(), updateRect.getY(),
                                         updateRect.GetWidth(), updateRect.GetHeight()));
        getDrawCanvas()->drawImage(bitmap.asImage(), rControlRegion.getX(), rControlRegion.getY());
        ++mPendingOperationsToFlush; // tdf#136369
        postDraw();
    }
    return bOK;
}

void AquaSkiaSalGraphicsImpl::drawTextLayout(const GenericSalLayout& rLayout)
{
    const CoreTextStyle& rStyle = *static_cast<const CoreTextStyle*>(&rLayout.GetFont());
    const FontSelectPattern& rFontSelect = rStyle.GetFontSelectPattern();
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
    font.setEdging(!mrShared.mbNonAntialiasedText ? SkFont::Edging::kAntiAlias
                                                  : SkFont::Edging::kAlias);

    // Vertical font, use width as "height".
    SkFont verticalFont(font);
    verticalFont.setSize(nHeight);
    //    verticalFont.setSize(nWidth); TODO
    //    verticalFont.setScaleX(1.0 * nHeight / nWidth);

    drawGenericLayout(rLayout, mrShared.maTextColor, font, verticalFont);
}

std::unique_ptr<sk_app::WindowContext> createMetalWindowContext(bool /*temporary*/)
{
    sk_app::DisplayParams displayParams;
    sk_app::window_context_factory::MacWindowInfo macWindow;
    macWindow.fMainView = nullptr;
    return sk_app::window_context_factory::MakeMetalForMac(macWindow, displayParams);
}

void AquaSkiaSalGraphicsImpl::prepareSkia() { SkiaHelper::prepareSkia(createMetalWindowContext); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
