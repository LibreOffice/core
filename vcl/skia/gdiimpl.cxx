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
#include <SkDashPathEffect.h>

#include <basegfx/polygon/b2dpolygontools.hxx>

#ifdef DBG_UTIL
#include <fstream>
#endif

namespace
{
// Create Skia Path from B2DPolygon
// TODO - take bezier curves into account
// TODO - use this for all Polygon / PolyPolygon needs
void lclPolygonToPath(const basegfx::B2DPolygon& rPolygon, SkPath& rPath)
{
    const sal_uInt32 nPointCount(rPolygon.count());

    if (nPointCount == 0)
        return;

    const bool bClosePath(rPolygon.isClosed());

    bool bFirst = true;

    for (sal_uInt32 nPointIndex = 0; nPointIndex < nPointCount; nPointIndex++)
    {
        auto const& rPoint = rPolygon.getB2DPoint(nPointIndex);
        if (bFirst)
        {
            rPath.moveTo(rPoint.getX(), rPoint.getY());
            bFirst = false;
        }
        else
        {
            rPath.lineTo(rPoint.getX(), rPoint.getY());
        }
    }
    if (bClosePath)
        rPath.close();
}

SkColor toSkColor(Color color)
{
    return SkColorSetARGB(255 - color.GetTransparency(), color.GetRed(), color.GetGreen(),
                          color.GetBlue());
}

Color fromSkColor(SkColor color)
{
    return Color(255 - SkColorGetA(color), SkColorGetR(color), SkColorGetG(color),
                 SkColorGetB(color));
}
} // end anonymous namespace

// Class that triggers flushing the backing buffer when idle.
class SkiaFlushIdle : public Idle
{
    SkiaSalGraphicsImpl* mpGraphics;

public:
    explicit SkiaFlushIdle(SkiaSalGraphicsImpl* pGraphics)
        : Idle("skia idle swap")
        , mpGraphics(pGraphics)
    {
        // We don't want to be swapping before we've painted.
        SetPriority(TaskPriority::POST_PAINT);
    }

    virtual void Invoke() override
    {
        mpGraphics->performFlush();
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

void SkiaSalGraphicsImpl::Init() {}

void SkiaSalGraphicsImpl::resetSurface()
{
    createSurface();
    mSurface->getCanvas()->save(); // see SetClipRegion()
    mClipRegion = vcl::Region(tools::Rectangle(0, 0, GetWidth(), GetHeight()));

    // We don't want to be swapping before we've painted.
    mFlush->Stop();
    mFlush->SetPriority(TaskPriority::POST_PAINT);
}

void SkiaSalGraphicsImpl::createSurface()
{
    // Create surface for offscreen graphics. Subclasses will create GPU-backed
    // surfaces as appropriate.
    mSurface = SkSurface::MakeRasterN32Premul(GetWidth(), GetHeight());
}

void SkiaSalGraphicsImpl::DeInit() { mSurface.reset(); }

void SkiaSalGraphicsImpl::preDraw() { checkSurface(); }

void SkiaSalGraphicsImpl::postDraw()
{
    if (!isOffscreen())
    {
        if (!Application::IsInExecute())
            performFlush(); // otherwise nothing would trigger idle rendering
        else if (!mFlush->IsActive())
            mFlush->Start();
    }
}

// VCL can sometimes resize us without telling us, update the surface if needed.
// Also create the surface on demand if it has not been created yet (it is a waste
// to create it in Init() if it gets recreated later anyway).
void SkiaSalGraphicsImpl::checkSurface()
{
    if (!mSurface || GetWidth() != mSurface->width() || GetHeight() != mSurface->height())
        resetSurface();
}

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
    checkSurface();
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
    preDraw();
    SkCanvas* canvas = mSurface->getCanvas();
    canvas->drawPoint(nX, nY, SkPaint());
    postDraw();
}

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY, Color nColor)
{
    if (nColor == SALCOLOR_NONE)
        return;
    preDraw();
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
    paint.setColor(toSkColor(nColor));
    // Apparently drawPixel() is actually expected to set the pixel and not draw it.
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    canvas->drawPoint(nX, nY, paint);
    postDraw();
}

void SkiaSalGraphicsImpl::drawLine(long nX1, long nY1, long nX2, long nY2)
{
    if (mLineColor == SALCOLOR_NONE)
        return;
    preDraw();
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
    paint.setColor(toSkColor(mLineColor));
    canvas->drawLine(nX1, nY1, nX2, nY2, paint);
    postDraw();
}

void SkiaSalGraphicsImpl::drawRect(long nX, long nY, long nWidth, long nHeight)
{
    preDraw();
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
    postDraw();
}

void SkiaSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    if (mLineColor == SALCOLOR_NONE)
        return;
    preDraw();
    std::vector<SkPoint> pointVector;
    pointVector.reserve(nPoints);
    for (sal_uInt32 i = 0; i < nPoints; ++i)
        pointVector.emplace_back(SkPoint::Make(pPtAry[i].mnX, pPtAry[i].mnY));
    SkPaint paint;
    paint.setColor(toSkColor(mLineColor));
    mSurface->getCanvas()->drawPoints(SkCanvas::kLines_PointMode, nPoints, pointVector.data(),
                                      paint);
    postDraw();
}

void SkiaSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    if (mLineColor == SALCOLOR_NONE && mFillColor == SALCOLOR_NONE)
        return;
    preDraw();
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
    postDraw();
}

void SkiaSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                          PCONSTSALPOINT* pPtAry)
{
    if (mLineColor == SALCOLOR_NONE && mFillColor == SALCOLOR_NONE)
        return;
    preDraw();
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
    postDraw();
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
    preDraw();
    sk_sp<SkImage> image
        = mSurface->makeImageSnapshot(SkIRect::MakeXYWH(nSrcX, nSrcY, nSrcWidth, nSrcHeight));
    // TODO makeNonTextureImage() ?
    mSurface->getCanvas()->drawImage(image, nDestX, nDestY);
    postDraw();
}

void SkiaSalGraphicsImpl::copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics)
{
    preDraw();
    SkiaSalGraphicsImpl* src;
    if (pSrcGraphics)
    {
        assert(dynamic_cast<SkiaSalGraphicsImpl*>(pSrcGraphics->GetImpl()));
        src = static_cast<SkiaSalGraphicsImpl*>(pSrcGraphics->GetImpl());
    }
    else
        src = this;
    src->checkSurface();
    sk_sp<SkImage> image = src->mSurface->makeImageSnapshot(
        SkIRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight));
    // TODO makeNonTextureImage() ?
    mSurface->getCanvas()->drawImageRect(image,
                                         SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY,
                                                          rPosAry.mnDestWidth,
                                                          rPosAry.mnDestHeight),
                                         nullptr);
    postDraw();
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
    preDraw();
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSalBitmap));
    mSurface->getCanvas()->drawBitmapRect(
        static_cast<const SkiaSalBitmap&>(rSalBitmap).GetSkBitmap(),
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
    postDraw();
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
    drawMask(rPosAry, static_cast<const SkiaSalBitmap&>(rSalBitmap).GetSkBitmap(), nMaskColor);
}

void SkiaSalGraphicsImpl::drawMask(const SalTwoRect& rPosAry, const SkBitmap& rBitmap,
                                   Color nMaskColor)
{
    preDraw();
    SkBitmap tmpBitmap;
    if (!tmpBitmap.tryAllocN32Pixels(rBitmap.width(), rBitmap.height()))
        abort();
    SkCanvas canvas(tmpBitmap);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas.drawBitmap(rBitmap, 0, 0, &paint);
    // TODO what is this function supposed to do exactly?
    // Text drawing on Windows doesn't work if this is uncommented.
    //    tmpBitmap.eraseColor(toSkColor(nMaskColor));
    (void)nMaskColor;
    mSurface->getCanvas()->drawBitmapRect(
        tmpBitmap,
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
    postDraw();
}

std::shared_ptr<SalBitmap> SkiaSalGraphicsImpl::getBitmap(long nX, long nY, long nWidth,
                                                          long nHeight)
{
    checkSurface();
    mSurface->getCanvas()->flush();
    sk_sp<SkImage> image = mSurface->makeImageSnapshot(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight));
    return std::make_shared<SkiaSalBitmap>(*image);
}

Color SkiaSalGraphicsImpl::getPixel(long nX, long nY)
{
    checkSurface();
    mSurface->getCanvas()->flush();
    // TODO this is presumably slow, and possibly won't work with GPU surfaces
    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(GetWidth(), GetHeight()))
        abort();
    if (!mSurface->readPixels(bitmap, 0, 0))
        abort();
    return fromSkColor(bitmap.getColor(nX, nY));
}

void SkiaSalGraphicsImpl::invert(basegfx::B2DPolygon const& rPoly, SalInvert eFlags)
{
    preDraw();
    // TrackFrame just inverts a dashed path around the polygon
    if (eFlags == SalInvert::TrackFrame)
    {
        SkPath aPath;
        lclPolygonToPath(rPoly, aPath);
        SkPaint aPaint;
        aPaint.setStrokeWidth(2);
        float intervals[] = { 4.0f, 4.0f };
        aPaint.setStyle(SkPaint::kStroke_Style);
        aPaint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
        aPaint.setColor(SkColorSetARGB(255, 255, 255, 255));
        aPaint.setBlendMode(SkBlendMode::kDifference);

        mSurface->getCanvas()->drawPath(aPath, aPaint);
    }
    else
    {
        SkPath aPath;
        lclPolygonToPath(rPoly, aPath);
        SkPaint aPaint;
        aPaint.setColor(SkColorSetARGB(255, 255, 255, 255));
        aPaint.setStyle(SkPaint::kFill_Style);
        aPaint.setBlendMode(SkBlendMode::kDifference);

        // N50 inverts in 4x4 checker pattern
        if (eFlags == SalInvert::N50)
        {
            // This creates 4x4 checker pattern bitmap
            // TODO Cache the bitmap
            SkBitmap aBitmap;
            aBitmap.allocN32Pixels(4, 4);
            SkPMColor* scanline;
            scanline = aBitmap.getAddr32(0, 0);
            *scanline++ = 0xFFFFFFFF;
            *scanline++ = 0xFFFFFFFF;
            *scanline++ = 0xFF000000;
            *scanline++ = 0xFF000000;
            scanline = aBitmap.getAddr32(0, 1);
            *scanline++ = 0xFFFFFFFF;
            *scanline++ = 0xFFFFFFFF;
            *scanline++ = 0xFF000000;
            *scanline++ = 0xFF000000;
            scanline = aBitmap.getAddr32(0, 2);
            *scanline++ = 0xFF000000;
            *scanline++ = 0xFF000000;
            *scanline++ = 0xFFFFFFFF;
            *scanline++ = 0xFFFFFFFF;
            scanline = aBitmap.getAddr32(0, 3);
            *scanline++ = 0xFF000000;
            *scanline++ = 0xFF000000;
            *scanline++ = 0xFFFFFFFF;
            *scanline++ = 0xFFFFFFFF;
            // The bitmap is repeated in both directions the checker pattern is as big
            // as the polygon (usually rectangle)
            aPaint.setShader(aBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
        }

        mSurface->getCanvas()->drawPath(aPath, aPaint);
    }
    postDraw();
}

void SkiaSalGraphicsImpl::invert(long nX, long nY, long nWidth, long nHeight, SalInvert eFlags)
{
    basegfx::B2DRectangle aRectangle(nX, nY, nX + nWidth, nY + nHeight);
    auto aRect = basegfx::utils::createPolygonFromRect(aRectangle);
    invert(aRect, eFlags);
}

void SkiaSalGraphicsImpl::invert(sal_uInt32 nPoints, const SalPoint* pPointArray, SalInvert eFlags)
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(pPointArray[0].mnX, pPointArray[0].mnY), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
    {
        aPolygon.setB2DPoint(i, basegfx::B2DPoint(pPointArray[i].mnX, pPointArray[i].mnY));
    }
    aPolygon.setClosed(true);

    invert(aPolygon, eFlags);
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
    preDraw();
    mSurface->getCanvas()->drawBitmapRect(
        bitmap,
        SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
        SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                         rPosAry.mnDestHeight),
        nullptr);
    postDraw();
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
    assert(mSurface.get());
    mSurface->getCanvas()->flush();
    sk_sp<SkImage> image = mSurface->makeImageSnapshot();
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}

void SkiaSalGraphicsImpl::dump(const SkBitmap& bitmap, const char* file)
{
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
