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

#include <skia/gdiimpl.hxx>

#include <skia/salbmp.hxx>

#include <SkCanvas.h>

#ifdef DBG_UTIL
#include <fstream>
#endif

SkiaSalGraphicsImpl::SkiaSalGraphicsImpl(SalGraphics& rParent, SalGeometryProvider* pProvider)
    : mParent(rParent)
    , mProvider(pProvider)
    , mLineColor(SALCOLOR_NONE)
    , mFillColor(SALCOLOR_NONE)
{
}

SkiaSalGraphicsImpl::~SkiaSalGraphicsImpl() {}

void SkiaSalGraphicsImpl::Init()
{
    // TODO
    mSurface = SkSurface::MakeRasterN32Premul(GetWidth(), GetHeight());
}

void SkiaSalGraphicsImpl::DeInit() { mSurface.reset(); }

void SkiaSalGraphicsImpl::freeResources() {}

const vcl::Region& SkiaSalGraphicsImpl::getClipRegion() const { return mClipRegion; }

bool SkiaSalGraphicsImpl::setClipRegion(const vcl::Region& region)
{
    mClipRegion = region;
    return true;
}

sal_uInt16 SkiaSalGraphicsImpl::GetBitCount() const { return 32; }

long SkiaSalGraphicsImpl::GetGraphicsWidth() const { return GetWidth(); }

void SkiaSalGraphicsImpl::ResetClipRegion() { mClipRegion.SetEmpty(); }

void SkiaSalGraphicsImpl::SetLineColor() { mLineColor = SALCOLOR_NONE; }

void SkiaSalGraphicsImpl::SetLineColor(Color nColor) { mLineColor = nColor; }

void SkiaSalGraphicsImpl::SetFillColor() { mFillColor = SALCOLOR_NONE; }

void SkiaSalGraphicsImpl::SetFillColor(Color nColor) { mFillColor = nColor; }

void SkiaSalGraphicsImpl::SetXORMode(bool bSet, bool bInvertOnly)
{
    (void)bSet;
    (void)bInvertOnly;
}

void SkiaSalGraphicsImpl::SetROPLineColor(SalROPColor nROPColor) { (void)nROPColor; }

void SkiaSalGraphicsImpl::SetROPFillColor(SalROPColor nROPColor) { (void)nROPColor; }

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY)
{
    SkCanvas* canvas = mSurface->getCanvas();
    canvas->drawPoint(nX, nY, mPaint);
}

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY, Color nColor)
{
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint(mPaint);
    paint.setColor(toSkColor(nColor));
    canvas->drawPoint(nX, nY, paint);
}

void SkiaSalGraphicsImpl::drawLine(long nX1, long nY1, long nX2, long nY2)
{
    SkCanvas* canvas = mSurface->getCanvas();
    canvas->drawLine(nX1, nY1, nX2, nY2, mPaint);
}

void SkiaSalGraphicsImpl::drawRect(long nX, long nY, long nWidth, long nHeight)
{
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint(mPaint);
    paint.setStrokeWidth(0); // smallest possible
    if (mFillColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColor(mFillColor));
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawIRect(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight), paint);
    }
    if (mLineColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColor(mLineColor));
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawIRect(SkIRect::MakeXYWH(nX, nY, nWidth - 1, nHeight - 1), paint);
    }
}

void SkiaSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    (void)nPoints;
    (void)pPtAry;
    abort();
}

void SkiaSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    (void)nPoints;
    (void)pPtAry;
    abort();
}

void SkiaSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                          PCONSTSALPOINT* pPtAry)
{
    (void)nPoly;
    (void)pPoints;
    (void)pPtAry;
    abort();
}

bool SkiaSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                          const basegfx::B2DPolyPolygon&, double fTransparency)
{
    (void)rObjectToDevice;
    (void)fTransparency;
    return false;
}

bool SkiaSalGraphicsImpl::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                       const basegfx::B2DPolygon&, double fTransparency,
                                       const basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin,
                                       css::drawing::LineCap, double fMiterMinimumAngle,
                                       bool bPixelSnapHairline)
{
    (void)rObjectToDevice;
    (void)fTransparency;
    (void)rLineWidths;
    (void)fMiterMinimumAngle;
    (void)bPixelSnapHairline;
    return false;
}

bool SkiaSalGraphicsImpl::drawPolyLineBezier(sal_uInt32 nPoints, const SalPoint* pPtAry,
                                             const PolyFlags* pFlgAry)
{
    (void)nPoints;
    (void)pPtAry;
    (void)pFlgAry;
    return false;
}

bool SkiaSalGraphicsImpl::drawPolygonBezier(sal_uInt32 nPoints, const SalPoint* pPtAry,
                                            const PolyFlags* pFlgAry)
{
    (void)nPoints;
    (void)pPtAry;
    (void)pFlgAry;
    return false;
}

bool SkiaSalGraphicsImpl::drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                                const SalPoint* const* pPtAry,
                                                const PolyFlags* const* pFlgAry)
{
    (void)nPoly;
    (void)pPoints;
    (void)pPtAry;
    (void)pFlgAry;
    return false;
}

void SkiaSalGraphicsImpl::copyArea(long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                   long nSrcHeight, bool bWindowInvalidate)
{
    (void)nDestX;
    (void)nDestY;
    (void)nSrcX;
    (void)nSrcY;
    (void)nSrcWidth;
    (void)nSrcHeight;
    (void)bWindowInvalidate;
    abort();
}

bool SkiaSalGraphicsImpl::blendBitmap(const SalTwoRect&, const SalBitmap& rBitmap)
{
    (void)rBitmap;
    return false;
}

bool SkiaSalGraphicsImpl::blendAlphaBitmap(const SalTwoRect&, const SalBitmap& rSrcBitmap,
                                           const SalBitmap& rMaskBitmap,
                                           const SalBitmap& rAlphaBitmap)
{
    (void)rSrcBitmap;
    (void)rMaskBitmap;
    (void)rAlphaBitmap;
    return false;
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSalBitmap));
    mSurface->getCanvas()->drawBitmapRect(
        static_cast<const SkiaSalBitmap&>(rSalBitmap).mBitmap,
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                     const SalBitmap& rMaskBitmap)
{
    (void)rPosAry;
    (void)rSalBitmap;
    (void)rMaskBitmap;
    abort();
}

void SkiaSalGraphicsImpl::drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                   Color nMaskColor)
{
    (void)rPosAry;
    (void)rSalBitmap;
    (void)nMaskColor;
    abort();
}

std::shared_ptr<SalBitmap> SkiaSalGraphicsImpl::getBitmap(long nX, long nY, long nWidth,
                                                          long nHeight)
{
    sk_sp<SkImage> image = mSurface->makeImageSnapshot(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight));
    return std::make_shared<SkiaSalBitmap>(*image);
}

Color SkiaSalGraphicsImpl::getPixel(long nX, long nY)
{
    // TODO this is presumably slow, and possibly won't work with GPU surfaces
    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(GetWidth(), GetHeight()))
        abort();
    if (!mSurface->readPixels(bitmap, 0, 0))
        abort();
    return fromSkColor(bitmap.getColor(nX, nY));
}

void SkiaSalGraphicsImpl::invert(long nX, long nY, long nWidth, long nHeight, SalInvert nFlags)
{
    (void)nX;
    (void)nY;
    (void)nWidth;
    (void)nHeight;
    (void)nFlags;
    abort();
}

void SkiaSalGraphicsImpl::invert(sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags)
{
    (void)nPoints;
    (void)pPtAry;
    (void)nFlags;
    abort();
}

bool SkiaSalGraphicsImpl::drawEPS(long nX, long nY, long nWidth, long nHeight, void* pPtr,
                                  sal_uInt32 nSize)
{
    (void)nX;
    (void)nY;
    (void)nWidth;
    (void)nHeight;
    (void)pPtr;
    (void)nSize;
    return false;
}

bool SkiaSalGraphicsImpl::drawAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                          const SalBitmap& rAlphaBitmap)
{
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSourceBitmap));
    assert(dynamic_cast<const SkiaSalBitmap*>(&rAlphaBitmap));
    SkBitmap tmpBitmap;
    if (!tmpBitmap.tryAllocPixels(SkImageInfo::Make(rSourceBitmap.GetSize().Width(),
                                                    rSourceBitmap.GetSize().Height(),
                                                    kN32_SkColorType, kUnpremul_SkAlphaType)))
        return false;
    SkCanvas canvas(tmpBitmap);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kDst);
    canvas.drawBitmap(static_cast<const SkiaSalBitmap&>(rSourceBitmap).mBitmap, 0, 0,
                      &paint); // TODO bpp < 8?
    paint.setBlendMode(SkBlendMode::kSrcIn);
    canvas.drawBitmap(static_cast<const SkiaSalBitmap&>(rAlphaBitmap).mBitmap, 0, 0,
                      &paint); // TODO bpp < 8?
    mSurface->getCanvas()->drawBitmapRect(
        tmpBitmap,
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
    return true;
}

bool SkiaSalGraphicsImpl::drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                                const basegfx::B2DPoint& rX,
                                                const basegfx::B2DPoint& rY,
                                                const SalBitmap& rSourceBitmap,
                                                const SalBitmap* pAlphaBitmap)
{
    (void)rNull;
    (void)rX;
    (void)rY;
    (void)rSourceBitmap;
    (void)pAlphaBitmap;
    return false;
}

bool SkiaSalGraphicsImpl::drawAlphaRect(long nX, long nY, long nWidth, long nHeight,
                                        sal_uInt8 nTransparency)
{
    (void)nX;
    (void)nY;
    (void)nWidth;
    (void)nHeight;
    (void)nTransparency;
    return false;
}

bool SkiaSalGraphicsImpl::drawGradient(const tools::PolyPolygon& rPolygon,
                                       const Gradient& rGradient)
{
    (void)rPolygon;
    (void)rGradient;
    return false;
}

#ifdef DBG_UTIL
void SkiaSalGraphicsImpl::dump(const char* file) const
{
    sk_sp<SkImage> image = mSurface->makeImageSnapshot();
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
