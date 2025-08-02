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

#include <skia/osx/bitmap.hxx>

#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>

#include <skia/salbmp.hxx>
#include <osx/saldata.hxx>

#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkShader.h>

using namespace SkiaHelper;

namespace SkiaHelper
{
CGImageRef createCGImage(const Image& rImage)
{
    Bitmap bitmap(rImage.GetBitmap());

    if (bitmap.IsEmpty() || !bitmap.ImplGetSalBitmap())
        return nullptr;

    assert(dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get()) != nullptr);
    SkiaSalBitmap* skiaBitmap = static_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());

    SkBitmap targetBitmap;
    if (!targetBitmap.tryAllocPixels(
            SkImageInfo::Make(bitmap.GetSizePixel().getWidth(), bitmap.GetSizePixel().getHeight(),
                              kRGBA_8888_SkColorType, kPremul_SkAlphaType)))
        return nullptr;
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is
    SkMatrix matrix; // The image is needed upside-down.
    matrix.preTranslate(0, targetBitmap.height());
    matrix.setConcat(matrix, SkMatrix::Scale(1, -1));

    if (!bitmap.HasAlpha())
    {
        SkCanvas canvas(targetBitmap);
        canvas.concat(matrix);
        canvas.drawImage(skiaBitmap->GetSkImage(), 0, 0, SkSamplingOptions(), &paint);
    }
    else
    {
        BitmapEx bitmapEx(bitmap);
        AlphaMask alpha(bitmapEx.GetAlphaMask());
        // tdf#156854 invert alpha mask for macOS native menu item images
        // At the time of this change, only the AquaSalMenu class calls this
        // function so it should be safe to invert the alpha mask here.
        alpha.Invert();
        Bitmap alphaBitmap(alpha.GetBitmap());
        assert(dynamic_cast<SkiaSalBitmap*>(alphaBitmap.ImplGetSalBitmap().get()) != nullptr);
        SkiaSalBitmap* skiaAlpha
            = static_cast<SkiaSalBitmap*>(alphaBitmap.ImplGetSalBitmap().get());
#if 0
        // Drawing to a bitmap using a shader from a GPU-backed image fails silently.
        // https://bugs.chromium.org/p/skia/issues/detail?id=12685
        paint.setShader(SkShaders::Blend(SkBlendMode::kDstOut,
                                         skiaBitmap->GetSkShader(SkSamplingOptions()),
                                         skiaAlpha->GetAlphaSkShader(SkSamplingOptions())));
#else
        sk_sp<SkImage> imB = skiaBitmap->GetSkImage()->makeNonTextureImage();
        sk_sp<SkImage> imA = skiaAlpha->GetAlphaSkImage()->makeNonTextureImage();
        paint.setShader(SkShaders::Blend(SkBlendMode::kDstOut, imB->makeShader(SkSamplingOptions()),
                                         imA->makeShader(SkSamplingOptions())));
#endif
        SkCanvas canvas(targetBitmap);
        canvas.concat(matrix);
        canvas.drawPaint(paint);
    }

    CGContextRef context = CGBitmapContextCreate(
        targetBitmap.getAddr32(0, 0), targetBitmap.width(), targetBitmap.height(), 8,
        targetBitmap.rowBytes(), GetSalData()->mxRGBSpace, kCGImageAlphaPremultipliedLast);
    if (!context)
        return nullptr;
    CGImageRef screenImage = CGBitmapContextCreateImage(context);
    CFRelease(context);
    return screenImage;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
