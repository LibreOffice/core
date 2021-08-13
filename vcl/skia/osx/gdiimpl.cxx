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

#include <skia/osx/rastercontext.hxx>

#include <SkCanvas.h>

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

void AquaSkiaSalGraphicsImpl::DeInit()
{
    SkiaZone zone;
    SkiaSalGraphicsImpl::DeInit();
    mWindowContext.reset();
}

void AquaSkiaSalGraphicsImpl::freeResources() {}

void AquaSkiaSalGraphicsImpl::createWindowContext(bool forceRaster)
{
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    displayParams.fColorType = kN32_SkColorType;
    forceRaster = true; // TODO
    RenderMethod renderMethod = forceRaster ? RenderRaster : renderMethodToUse();
    switch (renderMethod)
    {
        case RenderRaster:
            displayParams.fColorType = kRGBA_8888_SkColorType; // TODO
            mWindowContext.reset(
                new AquaSkiaWindowContextRaster(GetWidth(), GetHeight(), displayParams));
            break;
        case RenderVulkan:
            abort();
            break;
    }
}

//void AquaSkiaSalGraphicsImpl::Flush() { performFlush(); }

void AquaSkiaSalGraphicsImpl::performFlush()
{
    SkiaZone zone;
    flushDrawing();
    if (mWindowContext)
    {
        if (mDirtyRect.intersect(SkIRect::MakeWH(GetWidth(), GetHeight())))
            flushToScreen(mDirtyRect);
        mDirtyRect.setEmpty();
    }
}

void AquaSkiaSalGraphicsImpl::flushToScreen(const SkIRect& rect)
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
        = CGBitmapContextCreate(pixmap.writable_addr32(rect.left(), rect.top()), rect.width(),
                                rect.height(), 8, pixmap.rowBytes(), // TODO
                                GetSalData()->mxRGBSpace, kCGImageAlphaNoneSkipLast); // TODO
    assert(context); // TODO
    CGImageRef screenImage = CGBitmapContextCreateImage(context);
    assert(screenImage); // TODO
    if (mrShared.isFlipped())
    {
        const CGRect screenRect = CGRectMake(rect.left(), GetHeight() - rect.top() - rect.height(),
                                             rect.width(), rect.height());
        mrShared.maContextHolder.saveState();
        CGContextTranslateCTM(mrShared.maContextHolder.get(), 0, pixmap.height());
        CGContextScaleCTM(mrShared.maContextHolder.get(), 1, -1);
        CGContextDrawImage(mrShared.maContextHolder.get(), screenRect, screenImage);
        mrShared.maContextHolder.restoreState();
    }
    else
    {
        const CGRect screenRect = CGRectMake(rect.left(), rect.top(), rect.width(), rect.height());
        CGContextDrawImage(mrShared.maContextHolder.get(), screenRect, screenImage);
    }

    CGImageRelease(screenImage);
    CGContextRelease(context);
    mrShared.refreshRect(rect.left(), rect.top(), rect.width(), rect.height());
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
    CGContextRef context
        = CGBitmapContextCreate(data.get(), width, height, 8, width * 4, // TODO
                                GetSalData()->mxRGBSpace, kCGImageAlphaPremultipliedLast); // TODO
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
        if (!bitmap.installPixels(
                SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
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
        addUpdateRegion(SkRect::MakeXYWH(updateRect.Left(), updateRect.Top(), updateRect.GetWidth(),
                                         updateRect.GetHeight()));
        getDrawCanvas()->drawImage(bitmap.asImage(), rControlRegion.getX(), rControlRegion.getY());
        ++mPendingOperationsToFlush; // tdf#136369
        postDraw();
    }
    return bOK;
}

std::unique_ptr<sk_app::WindowContext> createVulkanWindowContext(bool /*temporary*/)
{
    return nullptr;
}

void AquaSkiaSalGraphicsImpl::prepareSkia() { SkiaHelper::prepareSkia(createVulkanWindowContext); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
