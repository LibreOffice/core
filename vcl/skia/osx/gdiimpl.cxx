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

#include <quartz/CoreTextFont.hxx>
#include <quartz/SystemFontList.hxx>
#include <skia/quartz/cgutils.h>

#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkFont.h>
#include <SkFontMgr_mac_ct.h>
#include <SkTypeface_mac.h>

using namespace SkiaHelper;

namespace
{
struct SnapshotImageData
{
    sk_sp<SkImage> image;
    SkPixmap pixmap;
};
}

static void SnapshotImageDataCallback(void* pInfo, const void* pData, size_t nSize)
{
    (void)pData;
    (void)nSize;

    if (pInfo)
        delete static_cast<SnapshotImageData*>(pInfo);
}

AquaSkiaSalGraphicsImpl::AquaSkiaSalGraphicsImpl(AquaSalGraphics& rParent,
                                                 AquaSharedAttributes& rShared)
    : SkiaSalGraphicsImpl(rParent, rShared.mpFrame)
    , AquaGraphicsBackendBase(rShared, this)
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

void AquaSkiaSalGraphicsImpl::WindowBackingPropertiesChanged() { windowBackingPropertiesChanged(); }

void AquaSkiaSalGraphicsImpl::flushSurfaceToWindowContext()
{
    if (!isGPU())
    {
        // tdf159175 mark dirty area in NSWindow for redrawing
        // This will cause -[SalFrameView drawRect:] to be called. That,
        // in turn, will draw a CGImageRef of the surface fetched from
        // AquaSkiaSalGraphicsImpl::createCGImageFromRasterSurface().
        mrShared.refreshRect(mDirtyRect.x(), mDirtyRect.y(), mDirtyRect.width(),
                             mDirtyRect.height());
    }
    else
    {
        SkiaSalGraphicsImpl::flushSurfaceToWindowContext();
    }
}

// For Raster we use our own screen blitting (see above).
CGImageRef AquaSkiaSalGraphicsImpl::createCGImageFromRasterSurface(const NSRect& rDirtyRect,
                                                                   CGPoint& rImageOrigin,
                                                                   bool& rImageFlipped)
{
    if (isGPU() || !mSurface)
        return nullptr;

    // Based on AquaGraphicsBackend::drawBitmap().
    if (!mrShared.checkContext())
        return nullptr;

    NSRect aIntegralRect = NSIntegralRect(rDirtyRect);
    if (NSIsEmptyRect(aIntegralRect))
        return nullptr;

    // Do not use sub-rect, it creates copies of the data.
    SnapshotImageData* pInfo = new SnapshotImageData;
    pInfo->image = makeCheckedImageSnapshot(mSurface);
    if (!pInfo->image->peekPixels(&pInfo->pixmap))
        abort();

    SkIRect aDirtyRect = SkIRect::MakeXYWH(
        aIntegralRect.origin.x * mScaling, aIntegralRect.origin.y * mScaling,
        aIntegralRect.size.width * mScaling, aIntegralRect.size.height * mScaling);
    if (mrShared.isFlipped())
        aDirtyRect = SkIRect::MakeXYWH(
            aDirtyRect.x(), pInfo->pixmap.bounds().height() - aDirtyRect.y() - aDirtyRect.height(),
            aDirtyRect.width(), aDirtyRect.height());
    if (!aDirtyRect.intersect(pInfo->pixmap.bounds()))
    {
        delete pInfo;
        return nullptr;
    }

    // If window scaling, then aDirtyRect is in scaled VCL coordinates and mSurface has
    // screen size (=points,HiDPI).
    // This creates the bitmap context from the cropped part, writable_addr32() will get
    // the first pixel of aDirtyRect.topLeft(), and using pixmap.rowBytes() ensures the following
    // pixel lines will be read from correct positions.
    if (pInfo->pixmap.bounds() != aDirtyRect
        && pInfo->pixmap.bounds().bottom() == aDirtyRect.bottom())
    {
        // HACK for tdf#145843: If aDirtyRect includes the last line but not the first pixel of it,
        // then the rowBytes() trick would lead to the CG* functions thinking that even pixels after
        // the pixmap data belong to the area (since the shifted x()+rowBytes() points there) and
        // at least on Intel Mac they would actually read those data, even though I see no good reason
        // to do that, as that's beyond the x()+width() for the last line. That could be handled
        // by creating a subset SkImage (which as is said above copies data), or set the x coordinate
        // to 0, which will then make rowBytes() match the actual data.
        aDirtyRect.fLeft = 0;
        // Related tdf#156630 pixmaps can be wider than the dirty rectangle
        // This seems to most commonly occur when SAL_FORCE_HIDPI_SCALING=1
        // and the native window scale is 2.
        assert(aDirtyRect.width() <= pInfo->pixmap.bounds().width());
    }

    // tdf#145843 Do not use CGBitmapContextCreate() to create a bitmap context
    // As described in the comment in the above code, CGBitmapContextCreate()
    // and CGBitmapContextCreateWithData() will try to access pixels up to
    // aDirtyRect.x() + pixmap.bounds.width() for each row. When reading the
    // last line in the SkPixmap, the buffer allocated for the SkPixmap ends at
    // aDirtyRect.x() + aDirtyRect.width() and aDirtyRect.width() is clamped to
    // pixmap.bounds.width() - aDirtyRect.x().
    // This behavior looks like an optimization within CGBitmapContextCreate()
    // to draw with a single memcpy() so fix this bug by chaining the
    // CGDataProvider(), CGImageCreate(), and CGImageCreateWithImageInRect()
    // functions to create the screen image.
    CGDataProviderRef dataProvider
        = CGDataProviderCreateWithData(pInfo, pInfo->pixmap.writable_addr32(0, 0),
                                       pInfo->pixmap.computeByteSize(), SnapshotImageDataCallback);
    if (!dataProvider)
    {
        delete pInfo;
        SAL_WARN("vcl.skia", "flushSurfaceToScreenGC(): Failed to allocate data provider");
        return nullptr;
    }

    CGImageRef fullImage
        = CGImageCreate(pInfo->pixmap.bounds().width(), pInfo->pixmap.bounds().height(), 8,
                        8 * pInfo->image->imageInfo().bytesPerPixel(), pInfo->pixmap.rowBytes(),
                        GetSalData()->mxRGBSpace,
                        SkiaToCGBitmapType(pInfo->image->colorType(), pInfo->image->alphaType()),
                        dataProvider, nullptr, false, kCGRenderingIntentDefault);
    if (!fullImage)
    {
        CGDataProviderRelease(dataProvider);
        SAL_WARN("vcl.skia", "flushSurfaceToScreenGC(): Failed to allocate full image");
        return nullptr;
    }

    CGImageRef screenImage = CGImageCreateWithImageInRect(
        fullImage,
        CGRectMake(aDirtyRect.x(), aDirtyRect.y(), aDirtyRect.width(), aDirtyRect.height()));
    if (!screenImage)
    {
        CGImageRelease(fullImage);
        CGDataProviderRelease(dataProvider);
        SAL_WARN("vcl.skia", "createCGImageFromRasterSurface(): Failed to allocate screen image");
        return nullptr;
    }

    rImageOrigin = CGPointMake(aDirtyRect.x(), aDirtyRect.y());
    rImageFlipped = mrShared.isFlipped();

    CGImageRelease(fullImage);
    CGDataProviderRelease(dataProvider);

    return screenImage;
}

bool AquaSkiaSalGraphicsImpl::drawNativeControl(ControlType nType, ControlPart nPart,
                                                const tools::Rectangle& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue)
{
    // tdf#157613 make sure surface is not a nullptr
    checkSurface();
    if (!mSurface)
        return false;

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
    sal_uInt8* data = new sal_uInt8[bytes];
    memset(data, 0, bytes);
    CGContextRef context = CGBitmapContextCreate(
        data, width, height, 8, width * 4, GetSalData()->mxRGBSpace,
        SkiaToCGBitmapType(mSurface->imageInfo().colorType(), kPremul_SkAlphaType));
    if (!context)
    {
        SAL_WARN("vcl.skia", "drawNativeControl(): Failed to allocate bitmap context");
        delete[] data;
        return false;
    }
    // Setup context state for drawing (performDrawNativeControl() e.g. fills background in some cases).
    CGContextSetFillColorSpace(context, GetSalData()->mxRGBSpace);
    CGContextSetStrokeColorSpace(context, GetSalData()->mxRGBSpace);
    if (moLineColor)
    {
        RGBAColor lineColor(*moLineColor);
        CGContextSetRGBStrokeColor(context, lineColor.GetRed(), lineColor.GetGreen(),
                                   lineColor.GetBlue(), lineColor.GetAlpha());
    }
    if (moFillColor)
    {
        RGBAColor fillColor(*moFillColor);
        CGContextSetRGBFillColor(context, fillColor.GetRed(), fillColor.GetGreen(),
                                 fillColor.GetBlue(), fillColor.GetAlpha());
    }
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
        // Let SkBitmap determine when it is safe to delete the pixel buffer
        SkBitmap bitmap;
        if (!bitmap.installPixels(SkImageInfo::Make(width, height,
                                                    mSurface->imageInfo().colorType(),
                                                    kPremul_SkAlphaType),
                                  data, width * 4, nullptr, nullptr))
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
        // Related: tdf#156881 flush the canvas after drawing the pixel buffer
        getDrawCanvas()->flush();
        ++pendingOperationsToFlush; // tdf#136369
        postDraw();
    }
    // Related: tdf#159529 eliminate possible memory leak
    // Despite confirming that the release function passed to
    // SkBitmap.bitmap.installPixels() does get called for every
    // data array that has been allocated, Apple's Instruments
    //  indicates that the data is leaking. While it is likely a
    // false positive, it makes leak analysis difficult so leave
    // the bitmap mutable. That causes SkBitmap.asImage() to make
    // a copy of the data and the data can be safely deleted here.
    delete[] data;
    return bOK;
}

void AquaSkiaSalGraphicsImpl::drawTextLayout(const GenericSalLayout& rLayout)
{
    const bool bSubpixelPositioning = rLayout.GetSubpixelPositioning();
    const CoreTextFont& rFont = *static_cast<const CoreTextFont*>(&rLayout.GetFont());
    const vcl::font::FontSelectPattern& rFontSelect = rFont.GetFontSelectPattern();
    int nHeight = rFontSelect.mnHeight;
    int nWidth = rFontSelect.mnWidth ? rFontSelect.mnWidth : nHeight;
    if (nWidth == 0 || nHeight == 0)
    {
        SAL_WARN("vcl.skia", "DrawTextLayout(): rFontSelect.mnHeight is zero!?");
        return;
    }

    if (!fontManager)
    {
        std::unique_ptr<SystemFontList> fontList = GetCoretextFontList();
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

    sk_sp<SkTypeface> typeface = SkMakeTypefaceFromCTFont(rFont.GetCTFont());
    SkFont font(typeface);
    font.setSize(nHeight);
    //    font.setScaleX(rFont.mfFontStretch); TODO
    if (rFont.NeedsArtificialBold())
        font.setEmbolden(true);

    SkFont::Edging ePreferredAliasing
        = bSubpixelPositioning ? SkFont::Edging::kSubpixelAntiAlias : SkFont::Edging::kAntiAlias;
    if (bSubpixelPositioning)
    {
        // note that SkFont defaults to a BaselineSnap of true, so I think really only
        // subpixel in text direction
        font.setSubpixel(true);
    }
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
