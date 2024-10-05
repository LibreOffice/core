/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <skia/salbmp.hxx>
#include <skia/zone.hxx>
#include <SkBitmap.h>
#include <SkCanvas.h>
#include <SkPaint.h>

#include <quartz/cgutils.h>
#ifdef MACOSX 
#include <osx/saldata.hxx>
#else
#include <svdata.hxx>
#endif
#include <skia/quartz/cgutils.h>

CGImageRef SkiaSalBitmap::CreateWithMask(const SalBitmap& rMask, int nX, int nY, int nWidth,
                                         int nHeight) const
{
    return CreateWithSalBitmapAndMask( *this, rMask, nX, nY, nWidth, nHeight );
}

/** creates an image from the given rectangle, replacing all black pixels
    with nMaskColor and make all other full transparent */
CGImageRef SkiaSalBitmap::CreateColorMask(int nX, int nY, int nWidth, int nHeight,
                                          Color nMaskColor) const
{
    SkiaZone zone;
    SkBitmap targetBitmap;
    if (!targetBitmap.tryAllocPixels(SkImageInfo::Make(nWidth, nHeight, kN32_SkColorType, kPremul_SkAlphaType)))
        return nullptr;

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is
    SkMatrix matrix; // The image is needed upside-down.
    matrix.preTranslate(0, targetBitmap.height());
    matrix.setConcat(matrix, SkMatrix::Scale(1, -1));

    SkCanvas canvas(targetBitmap);
    canvas.concat(matrix);
    canvas.drawImageRect(GetSkImage(), SkRect::MakeXYWH(nX, nY, nWidth, nHeight), SkRect::MakeXYWH(0, 0, nWidth, nHeight), SkSamplingOptions(), &paint, SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint);
    if (auto dContext = GrAsDirectContext(canvas.recordingContext()))
        dContext->flushAndSubmit();

    const SkColor maskColor = SkiaHelper::toSkColor(nMaskColor);
    for (int y = 0; y < targetBitmap.height(); y++)
    {
        SkColor *pPixels = targetBitmap.getAddr32(0, y);
        for (int x = 0; x < targetBitmap.width(); x++)
            *pPixels++ = (*pPixels == SK_ColorBLACK ? maskColor : SK_ColorTRANSPARENT);
    }

    targetBitmap.setImmutable();

    CGContextRef xContext = CGBitmapContextCreate(targetBitmap.getAddr32(0, 0), targetBitmap.width(), targetBitmap.height(), 8, targetBitmap.rowBytes(), GetSalData()->mxRGBSpace, SkiaToCGBitmapType(targetBitmap.colorType(), targetBitmap.alphaType()));
    if (!xContext)
        return nullptr;

    CGImageRef xCroppedImage = CGBitmapContextCreateImage(xContext);
    CGContextRelease(xContext);
    return xCroppedImage;
}


CGImageRef SkiaSalBitmap::CreateCroppedImage(int nX, int nY, int nWidth, int nHeight) const
{
    SkiaZone zone;
    SkBitmap targetBitmap;
    if (!targetBitmap.tryAllocPixels(SkImageInfo::Make(nWidth, nHeight, kN32_SkColorType, kPremul_SkAlphaType)))
        return nullptr;

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is
    SkMatrix matrix; // The image is needed upside-down.
    matrix.preTranslate(0, targetBitmap.height());
    matrix.setConcat(matrix, SkMatrix::Scale(1, -1));

    SkCanvas canvas(targetBitmap);
    canvas.concat(matrix);
    canvas.drawImageRect(GetSkImage(), SkRect::MakeXYWH(nX, nY, nWidth, nHeight), SkRect::MakeXYWH(0, 0, nWidth, nHeight), SkSamplingOptions(), &paint, SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint);
    if (auto dContext = GrAsDirectContext(canvas.recordingContext()))
        dContext->flushAndSubmit();
    targetBitmap.setImmutable();

    CGContextRef xContext = CGBitmapContextCreate(targetBitmap.getAddr32(0, 0), targetBitmap.width(), targetBitmap.height(), 8, targetBitmap.rowBytes(), GetSalData()->mxRGBSpace, SkiaToCGBitmapType(targetBitmap.colorType(), targetBitmap.alphaType()));
    if (!xContext)
        return nullptr;

    CGImageRef xCroppedImage = CGBitmapContextCreateImage(xContext);
    CGContextRelease(xContext);
    return xCroppedImage;
}
