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

#include <salgdi.hxx>
#include <skia/salbmp.hxx>
#include <vcl/idle.hxx>
#include <vcl/svapp.hxx>

#include <SkCanvas.h>
#include <SkPath.h>
#include <SkRegion.h>

#ifdef DBG_UTIL
#include <fstream>
#endif

// Class that triggers flushing the backing buffer when idle.
class SkiaFlushIdle : public Idle
{
    SkiaSalGraphicsImpl* graphics;

public:
    explicit SkiaFlushIdle(SkiaSalGraphicsImpl* graphics)
        : Idle("skia idle swap")
        , graphics(graphics)
    {
        // We don't want to be swapping before we've painted.
        SetPriority(TaskPriority::POST_PAINT);
    }

    virtual void Invoke() override
    {
        graphics->performFlush();
        Stop();
        SetPriority(TaskPriority::HIGHEST);
    }
};

SkiaSalGraphicsImpl::SkiaSalGraphicsImpl(SalGraphics& rParent, SalGeometryProvider* pProvider)
    : mParent(rParent)
    , mProvider(pProvider)
    , mLineColor(SALCOLOR_NONE)
    , mFillColor(SALCOLOR_NONE)
    , mFlush(new SkiaFlushIdle(this))
{
}

SkiaSalGraphicsImpl::~SkiaSalGraphicsImpl() {}

void SkiaSalGraphicsImpl::Init()
{
    // TODO
    mSurface = SkSurface::MakeRasterN32Premul(GetWidth(), GetHeight());
    mSurface->getCanvas()->save(); // see SetClipRegion()
    mClipRegion = vcl::Region(tools::Rectangle(0, 0, GetWidth(), GetHeight()));

    // We don't want to be swapping before we've painted.
    mFlush->SetPriority(TaskPriority::POST_PAINT);
}

void SkiaSalGraphicsImpl::DeInit() { mSurface.reset(); }

static SkIRect toSkIRect(const tools::Rectangle& rectangle)
{
    return SkIRect::MakeXYWH(rectangle.Left(), rectangle.Top(), rectangle.GetWidth(),
                             rectangle.GetHeight());
}

static SkRegion toSkRegion(const vcl::Region& region)
{
    if (region.IsEmpty())
        return SkRegion();
    if (region.IsRectangle())
        return SkRegion(toSkIRect(region.GetBoundRect()));
    if (!region.HasPolyPolygonOrB2DPolyPolygon())
    {
        SkRegion skRegion;
        RectangleVector rectangles;
        region.GetRegionRectangles(rectangles);
        for (const tools::Rectangle& rect : rectangles)
            skRegion.op(toSkIRect(rect), SkRegion::kUnion_Op);
        return skRegion;
    }
    abort();
}

bool SkiaSalGraphicsImpl::setClipRegion(const vcl::Region& region)
{
    if (mClipRegion == region)
        return true;
    mClipRegion = region;
    SkCanvas* canvas = mSurface->getCanvas();
    // SkCanvas::clipRegion() can only further reduce the clip region,
    // but we need to set the given region, which may extend it.
    // So handle that by always having the full clip region saved on the stack
    // and always go back to that. SkCanvas::restore() only affects the clip
    // and the matrix.
    assert(canvas->getSaveCount() == 2); // = there is just one save()
    canvas->restore();
    canvas->save();
    canvas->clipRegion(toSkRegion(region));
    return true;
}

void SkiaSalGraphicsImpl::ResetClipRegion()
{
    setClipRegion(vcl::Region(tools::Rectangle(0, 0, GetWidth(), GetHeight())));
}

const vcl::Region& SkiaSalGraphicsImpl::getClipRegion() const { return mClipRegion; }

sal_uInt16 SkiaSalGraphicsImpl::GetBitCount() const { return 32; }

long SkiaSalGraphicsImpl::GetGraphicsWidth() const { return GetWidth(); }

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
    if (mLineColor == SALCOLOR_NONE)
        return;
    SkCanvas* canvas = mSurface->getCanvas();
    canvas->drawPoint(nX, nY, SkPaint());
    scheduleFlush();
}

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY, Color nColor)
{
    if (nColor == SALCOLOR_NONE)
        return;
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
    paint.setColor(toSkColor(nColor));
    // Apparently drawPixel() is actually expected to set the pixel and not draw it.
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    canvas->drawPoint(nX, nY, paint);
    scheduleFlush();
}

void SkiaSalGraphicsImpl::drawLine(long nX1, long nY1, long nX2, long nY2)
{
    if (mLineColor == SALCOLOR_NONE)
        return;
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
    paint.setColor(toSkColor(mLineColor));
    canvas->drawLine(nX1, nY1, nX2, nY2, paint);
    scheduleFlush();
}

void SkiaSalGraphicsImpl::drawRect(long nX, long nY, long nWidth, long nHeight)
{
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
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
    scheduleFlush();
}

void SkiaSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    if (mLineColor == SALCOLOR_NONE)
        return;
    std::vector<SkPoint> pointVector;
    pointVector.reserve(nPoints);
    for (sal_uInt32 i = 0; i < nPoints; ++i)
        pointVector.emplace_back(SkPoint::Make(pPtAry[i].mnX, pPtAry[i].mnY));
    SkPaint paint;
    paint.setColor(toSkColor(mLineColor));
    mSurface->getCanvas()->drawPoints(SkCanvas::kLines_PointMode, nPoints, pointVector.data(),
                                      paint);
    scheduleFlush();
}

void SkiaSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    if (mLineColor == SALCOLOR_NONE && mFillColor == SALCOLOR_NONE)
        return;
    std::vector<SkPoint> pointVector;
    pointVector.reserve(nPoints);
    for (sal_uInt32 i = 0; i < nPoints; ++i)
        pointVector.emplace_back(SkPoint::Make(pPtAry[i].mnX, pPtAry[i].mnY));
    SkPath path;
    path.addPoly(pointVector.data(), nPoints, false);
    SkPaint paint;
    if (mFillColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColor(mFillColor));
        paint.setStyle(SkPaint::kFill_Style);
        mSurface->getCanvas()->drawPath(path, paint);
    }
    if (mLineColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColor(mLineColor));
        paint.setStyle(SkPaint::kStroke_Style);
        mSurface->getCanvas()->drawPath(path, paint);
    }
    scheduleFlush();
}

void SkiaSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                          PCONSTSALPOINT* pPtAry)
{
    if (mLineColor == SALCOLOR_NONE && mFillColor == SALCOLOR_NONE)
        return;
    std::vector<SkPoint> pointVector;
    SkPath path;
    for (sal_uInt32 poly = 0; poly < nPoly; ++poly)
    {
        const sal_uInt32 points = pPoints[poly];
        if (points > 1)
        {
            pointVector.reserve(points);
            const SalPoint* p = pPtAry[poly];
            for (sal_uInt32 i = 0; i < points; ++i)
                pointVector.emplace_back(SkPoint::Make(p->mnX, p->mnY));
            path.addPoly(pointVector.data(), points, true);
        }
    }
    SkPaint paint;
    if (mFillColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColor(mFillColor));
        paint.setStyle(SkPaint::kFill_Style);
        mSurface->getCanvas()->drawPath(path, paint);
    }
    if (mLineColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColor(mLineColor));
        paint.setStyle(SkPaint::kStroke_Style);
        mSurface->getCanvas()->drawPath(path, paint);
    }
    scheduleFlush();
}

bool SkiaSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                          const basegfx::B2DPolyPolygon&, double fTransparency)
{
    if (mLineColor == SALCOLOR_NONE && mFillColor == SALCOLOR_NONE)
        return true;
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
                                   long nSrcHeight, bool /*bWindowInvalidate*/)
{
    if (nDestX == nSrcX && nDestY == nSrcY)
        return;
    sk_sp<SkImage> image
        = mSurface->makeImageSnapshot(SkIRect::MakeXYWH(nSrcX, nSrcY, nSrcWidth, nSrcHeight));
    // TODO makeNonTextureImage() ?
    mSurface->getCanvas()->drawImage(image, nDestX, nDestY);
    scheduleFlush();
}

void SkiaSalGraphicsImpl::copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics)
{
    SkiaSalGraphicsImpl* src;
    if (pSrcGraphics)
    {
        assert(dynamic_cast<SkiaSalGraphicsImpl*>(pSrcGraphics->GetImpl()));
        src = static_cast<SkiaSalGraphicsImpl*>(pSrcGraphics->GetImpl());
    }
    else
        src = this;
    sk_sp<SkImage> image = src->mSurface->makeImageSnapshot(
        SkIRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight));
    // TODO makeNonTextureImage() ?
    mSurface->getCanvas()->drawImageRect(image,
                                         SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY,
                                                          rPosAry.mnDestWidth,
                                                          rPosAry.mnDestHeight),
                                         nullptr);
    scheduleFlush();
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
        static_cast<const SkiaSalBitmap&>(rSalBitmap).GetSkBitmap(),
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
    scheduleFlush();
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
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSalBitmap));
    SkBitmap tmpBitmap;
    if (!tmpBitmap.tryAllocN32Pixels(rSalBitmap.GetSize().Width(), rSalBitmap.GetSize().Height()))
        abort();
    SkCanvas canvas(tmpBitmap);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas.drawBitmap(static_cast<const SkiaSalBitmap&>(rSalBitmap).GetSkBitmap(), 0, 0, &paint);
    tmpBitmap.eraseColor(toSkColor(nMaskColor));
    mSurface->getCanvas()->drawBitmapRect(
        tmpBitmap,
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
    scheduleFlush();
}

std::shared_ptr<SalBitmap> SkiaSalGraphicsImpl::getBitmap(long nX, long nY, long nWidth,
                                                          long nHeight)
{
    mSurface->getCanvas()->flush();
    sk_sp<SkImage> image = mSurface->makeImageSnapshot(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight));
    return std::make_shared<SkiaSalBitmap>(*image);
}

Color SkiaSalGraphicsImpl::getPixel(long nX, long nY)
{
    mSurface->getCanvas()->flush();
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
    if (!tmpBitmap.tryAllocN32Pixels(rSourceBitmap.GetSize().Width(),
                                     rSourceBitmap.GetSize().Height()))
        return false;
    SkCanvas canvas(tmpBitmap);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
    canvas.drawBitmap(static_cast<const SkiaSalBitmap&>(rAlphaBitmap).GetAlphaSkBitmap(), 0, 0,
                      &paint);
    paint.setBlendMode(SkBlendMode::kSrcIn);
    canvas.drawBitmap(static_cast<const SkiaSalBitmap&>(rSourceBitmap).GetSkBitmap(), 0, 0, &paint);
    drawBitmap(rPosAry, tmpBitmap);
    return true;
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SkBitmap& bitmap)
{
    mSurface->getCanvas()->drawBitmapRect(
        bitmap,
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
    scheduleFlush();
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

void SkiaSalGraphicsImpl::scheduleFlush()
{
    if (isOffscreen())
        return;
    if (!Application::IsInExecute())
        performFlush(); // otherwise nothing would trigger idle rendering
    else if (!mFlush->IsActive())
        mFlush->Start();
}

#ifdef DBG_UTIL
void SkiaSalGraphicsImpl::dump(const char* file) const
{
    mSurface->getCanvas()->flush();
    sk_sp<SkImage> image = mSurface->makeImageSnapshot();
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}

void SkiaSalGraphicsImpl::dump(const SkBitmap& bitmap, const char* file) const
{
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
