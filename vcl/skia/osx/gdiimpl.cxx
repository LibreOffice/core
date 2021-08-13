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
    RenderMethod renderMethod = forceRaster ? RenderRaster : renderMethodToUse();
    switch (renderMethod)
    {
        case RenderRaster:
            displayParams.fColorType = kRGBA_8888_SkColorType; // TODO
            mWindowContext.reset(
                new AquaSkiaWindowContextRaster(GetWidth(), GetHeight(), displayParams, this));
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

std::unique_ptr<sk_app::WindowContext> createVulkanWindowContext(bool /*temporary*/)
{
    return nullptr;
}

void AquaSkiaSalGraphicsImpl::prepareSkia() { SkiaHelper::prepareSkia(createVulkanWindowContext); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
