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
#include <vcl/lazydelete.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <skia/utils.hxx>

#include <SkCanvas.h>
#include <SkPath.h>
#include <SkRegion.h>
#include <SkDashPathEffect.h>
#include <GrBackendSurface.h>

#include <basegfx/polygon/b2dpolygontools.hxx>

namespace
{
// Create Skia Path from B2DPolygon
// Note that polygons generally have the complication that when used
// for area (fill) operations they usually miss the right-most and
// bottom-most line of pixels of the bounding rectangle (see
// https://lists.freedesktop.org/archives/libreoffice/2019-November/083709.html).
// So be careful with rectangle->polygon conversions (generally avoid them).
void addPolygonToPath(const basegfx::B2DPolygon& rPolygon, SkPath& rPath)
{
    const sal_uInt32 nPointCount(rPolygon.count());

    if (nPointCount <= 1)
        return;

    const bool bClosePath(rPolygon.isClosed());
    const bool bHasCurves(rPolygon.areControlPointsUsed());

    bool bFirst = true;

    sal_uInt32 nCurrentIndex = 0;
    sal_uInt32 nPreviousIndex = nPointCount - 1;

    basegfx::B2DPoint aCurrentPoint;
    basegfx::B2DPoint aPreviousPoint;

    for (sal_uInt32 nIndex = 0; nIndex <= nPointCount; nIndex++)
    {
        if (nIndex == nPointCount && !bClosePath)
            continue;

        // Make sure we loop the last point to first point
        nCurrentIndex = nIndex % nPointCount;
        aCurrentPoint = rPolygon.getB2DPoint(nCurrentIndex);

        if (bFirst)
        {
            rPath.moveTo(aCurrentPoint.getX(), aCurrentPoint.getY());
            bFirst = false;
        }
        else if (!bHasCurves)
        {
            rPath.lineTo(aCurrentPoint.getX(), aCurrentPoint.getY());
        }
        else
        {
            basegfx::B2DPoint aPreviousControlPoint = rPolygon.getNextControlPoint(nPreviousIndex);
            basegfx::B2DPoint aCurrentControlPoint = rPolygon.getPrevControlPoint(nCurrentIndex);

            if (aPreviousControlPoint.equal(aPreviousPoint))
            {
                aPreviousControlPoint
                    = aPreviousPoint + ((aPreviousControlPoint - aCurrentPoint) * 0.0005);
            }

            if (aCurrentControlPoint.equal(aCurrentPoint))
            {
                aCurrentControlPoint
                    = aCurrentPoint + ((aCurrentControlPoint - aPreviousPoint) * 0.0005);
            }
            rPath.cubicTo(aPreviousControlPoint.getX(), aPreviousControlPoint.getY(),
                          aCurrentControlPoint.getX(), aCurrentControlPoint.getY(),
                          aCurrentPoint.getX(), aCurrentPoint.getY());
        }
        aPreviousPoint = aCurrentPoint;
        nPreviousIndex = nCurrentIndex;
    }
    if (bClosePath)
    {
        rPath.close();
    }
}

void addPolyPolygonToPath(const basegfx::B2DPolyPolygon& rPolyPolygon, SkPath& rPath)
{
    const sal_uInt32 nPolygonCount(rPolyPolygon.count());

    if (nPolygonCount == 0)
        return;

    for (const auto& rPolygon : rPolyPolygon)
    {
        addPolygonToPath(rPolygon, rPath);
    }
}

SkColor toSkColor(Color color)
{
    return SkColorSetARGB(255 - color.GetTransparency(), color.GetRed(), color.GetGreen(),
                          color.GetBlue());
}

SkColor toSkColorWithTransparency(Color aColor, double fTransparency)
{
    return SkColorSetA(toSkColor(aColor), 255 * (1.0 - fTransparency));
}

Color fromSkColor(SkColor color)
{
    return Color(255 - SkColorGetA(color), SkColorGetR(color), SkColorGetG(color),
                 SkColorGetB(color));
}

// returns true if the source or destination rectangles are invalid
bool checkInvalidSourceOrDestination(SalTwoRect const& rPosAry)
{
    return rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
           || rPosAry.mnDestHeight <= 0;
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
    , mIsGPU(false)
    , mLineColor(SALCOLOR_NONE)
    , mFillColor(SALCOLOR_NONE)
    , mFlush(new SkiaFlushIdle(this))
{
}

SkiaSalGraphicsImpl::~SkiaSalGraphicsImpl()
{
    assert(!mSurface);
    assert(!mWindowContext);
}

void SkiaSalGraphicsImpl::Init() {}

void SkiaSalGraphicsImpl::recreateSurface()
{
    destroySurface();
    createSurface();
}

void SkiaSalGraphicsImpl::createSurface()
{
    if (isOffscreen())
        createOffscreenSurface();
    else
        createWindowSurface();
    mSurface->getCanvas()->save(); // see SetClipRegion()
    mClipRegion = vcl::Region(tools::Rectangle(0, 0, GetWidth(), GetHeight()));

    // We don't want to be swapping before we've painted.
    mFlush->Stop();
    mFlush->SetPriority(TaskPriority::POST_PAINT);
}

void SkiaSalGraphicsImpl::createWindowSurface()
{
    assert(!isOffscreen());
    assert(!mSurface);
    assert(!mWindowContext);
    createWindowContext();
    if (mWindowContext)
        mSurface = mWindowContext->getBackbufferSurface();
    if (!mSurface)
    {
        switch (SkiaHelper::renderMethodToUse())
        {
            case SkiaHelper::RenderVulkan:
                SAL_WARN("vcl.skia", "cannot create Vulkan GPU window surface, disabling Vulkan");
                // fall back to raster
                SkiaHelper::disableRenderMethod(SkiaHelper::RenderVulkan);
                destroySurface(); // destroys also WindowContext
                return createWindowSurface(); // try again
            case SkiaHelper::RenderRaster:
                abort(); // this should not really happen
        }
    }
    assert((mSurface->getCanvas()->getGrContext() != nullptr) == mIsGPU);
#ifdef DBG_UTIL
    SkiaHelper::prefillSurface(mSurface);
#endif
}

void SkiaSalGraphicsImpl::createOffscreenSurface()
{
    assert(isOffscreen());
    assert(!mSurface);
    assert(!mWindowContext);
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderVulkan:
        {
            GrContext* grContext = SkiaHelper::getSharedGrContext();
            // We may not get a GrContext if called before any onscreen window is created.
            if (!grContext)
            {
                SAL_INFO("vcl.skia",
                         "creating Vulkan offscreen GPU surface before any window exists");
                // Create temporary WindowContext with no window. That will fail,
                // but it will initialize the shared GrContext.
                createWindowContext();
                // This will use the temporarily created context.
                grContext = SkiaHelper::getSharedGrContext();
                // Destroy the temporary WindowContext.
                destroySurface();
            }
            if (grContext)
            {
                mSurface = SkiaHelper::createSkSurface(GetWidth(), GetHeight());
                assert(mSurface);
                assert(mSurface->getCanvas()->getGrContext()); // is GPU-backed
                mIsGPU = true;
                return;
            }
            SAL_WARN("vcl.skia", "cannot create Vulkan offscreen GPU surface, disabling Vulkan");
            SkiaHelper::disableRenderMethod(SkiaHelper::RenderVulkan);
            break;
        }
        default:
            break;
    }
    // Create raster surface as a fallback.
    mSurface = SkiaHelper::createSkSurface(GetWidth(), GetHeight());
    assert(mSurface);
    assert(!mSurface->getCanvas()->getGrContext()); // is not GPU-backed
    mIsGPU = false;
}

void SkiaSalGraphicsImpl::destroySurface()
{
    if (mSurface)
    {
        // check setClipRegion() invariant
        assert(mSurface->getCanvas()->getSaveCount() == 2);
        // if this fails, something forgot to use SkAutoCanvasRestore
        assert(mSurface->getCanvas()->getTotalMatrix().isIdentity());
    }
    // TODO Is this still needed?
    // If we use e.g. Vulkan, we must destroy the surface before the context,
    // otherwise destroying the surface will reference the context. This is
    // handled by calling destroySurface() before destroying the context.
    // However we also need to flush the surface before destroying it,
    // otherwise when destroying the context later there still could be queued
    // commands referring to the surface data. This is probably a Skia bug,
    // but work around it here.
    if (mSurface)
        mSurface->flush();
    mSurface.reset();
    mWindowContext.reset();
    mIsGPU = false;
}

void SkiaSalGraphicsImpl::DeInit() { destroySurface(); }

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
    if (!mSurface)
    {
        recreateSurface();
        SAL_INFO("vcl.skia",
                 "create(" << this << "): " << Size(mSurface->width(), mSurface->height()));
    }
    else if (GetWidth() != mSurface->width() || GetHeight() != mSurface->height())
    {
        if (!avoidRecreateByResize())
        {
            Size oldSize(mSurface->width(), mSurface->height());
            recreateSurface();
            SAL_INFO("vcl.skia", "recreate(" << this << "): old " << oldSize << " new "
                                             << Size(mSurface->width(), mSurface->height())
                                             << " requested " << Size(GetWidth(), GetHeight()));
        }
    }
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
    // Prefer rectangles to polygons (simpler and also see the addPolygonToPath() comment).
    if (region.getRegionBand())
    {
        SkRegion skRegion;
        RectangleVector rectangles;
        region.GetRegionRectangles(rectangles);
        for (const tools::Rectangle& rect : rectangles)
            skRegion.op(toSkIRect(rect), SkRegion::kUnion_Op);
        return skRegion;
    }
    else
    {
        assert(region.HasPolyPolygonOrB2DPolyPolygon());
        SkPath path;
        addPolyPolygonToPath(region.GetAsB2DPolyPolygon(), path);
        path.setFillType(SkPathFillType::kEvenOdd);
        SkRegion skRegion;
        skRegion.setPath(path, SkRegion(path.getBounds().roundOut()));
        return skRegion;
    }
}

bool SkiaSalGraphicsImpl::setClipRegion(const vcl::Region& region)
{
    if (mClipRegion == region)
        return true;
    mClipRegion = region;
    checkSurface();
    SAL_INFO("vcl.skia", "setclipregion(" << this << "): " << region);
    SkCanvas* canvas = mSurface->getCanvas();
    // SkCanvas::clipRegion() can only further reduce the clip region,
    // but we need to set the given region, which may extend it.
    // So handle that by always having the full clip region saved on the stack
    // and always go back to that. SkCanvas::restore() only affects the clip
    // and the matrix.
    assert(canvas->getSaveCount() == 2); // = there is just one save()
    canvas->restore();
    canvas->save();
#if 1
    // TODO
    // SkCanvas::clipRegion() is buggy with Vulkan, use SkCanvas::clipPath().
    // https://bugs.chromium.org/p/skia/issues/detail?id=9580
    // This is further complicated by rectangle->polygon area conversions
    // being problematic (see addPolygonToPath() comment), so handle rectangles
    // first before resorting to polygons.
    if (region.getRegionBand())
    {
        RectangleVector rectangles;
        region.GetRegionRectangles(rectangles);
        SkPath path;
        for (const tools::Rectangle& rectangle : rectangles)
            path.addRect(SkRect::MakeXYWH(rectangle.getX(), rectangle.getY(), rectangle.GetWidth(),
                                          rectangle.GetHeight()));
        path.setFillType(SkPathFillType::kEvenOdd);
        path.dump();
        canvas->clipPath(path);
    }
    else if (!region.IsEmpty() && !region.IsRectangle())
    {
        SkPath path;
        addPolyPolygonToPath(region.GetAsB2DPolyPolygon(), path);
        path.setFillType(SkPathFillType::kEvenOdd);
        canvas->clipPath(path);
    }
    else
#endif
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

void SkiaSalGraphicsImpl::SetXORMode(bool, bool)
{
    // TODO
}

void SkiaSalGraphicsImpl::SetROPLineColor(SalROPColor nROPColor)
{
    switch (nROPColor)
    {
        case SalROPColor::N0:
            mLineColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
            mLineColor = Color(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            mLineColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void SkiaSalGraphicsImpl::SetROPFillColor(SalROPColor nROPColor)
{
    switch (nROPColor)
    {
        case SalROPColor::N0:
            mFillColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
            mFillColor = Color(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            mFillColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY) { drawPixel(nX, nY, mLineColor); }

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY, Color nColor)
{
    if (nColor == SALCOLOR_NONE)
        return;
    preDraw();
    SAL_INFO("vcl.skia", "drawpixel(" << this << "): " << Point(nX, nY) << ":" << nColor);
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
    paint.setColor(toSkColor(nColor));
    // Apparently drawPixel() is actually expected to set the pixel and not draw it.
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    canvas->drawPoint(toSkX(nX), toSkY(nY), paint);
    postDraw();
}

void SkiaSalGraphicsImpl::drawLine(long nX1, long nY1, long nX2, long nY2)
{
    if (mLineColor == SALCOLOR_NONE)
        return;
    preDraw();
    SAL_INFO("vcl.skia", "drawline(" << this << "): " << Point(nX1, nY1) << "->" << Point(nX2, nY2)
                                     << ":" << mLineColor);
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
    paint.setColor(toSkColor(mLineColor));
    paint.setAntiAlias(mParent.getAntiAliasB2DDraw());
    canvas->drawLine(toSkX(nX1), toSkY(nY1), toSkX(nX2), toSkY(nY2), paint);
    postDraw();
}

void SkiaSalGraphicsImpl::privateDrawAlphaRect(long nX, long nY, long nWidth, long nHeight,
                                               double fTransparency, bool blockAA)
{
    preDraw();
    SAL_INFO("vcl.skia", "privatedrawrect(" << this << "): " << Point(nX, nY) << "/"
                                            << Size(nWidth, nHeight) << ":" << mLineColor << ":"
                                            << mFillColor << ":" << fTransparency);
    SkCanvas* canvas = mSurface->getCanvas();
    SkPaint paint;
    paint.setAntiAlias(!blockAA && mParent.getAntiAliasB2DDraw());
    if (mFillColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColorWithTransparency(mFillColor, fTransparency));
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawIRect(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight), paint);
    }
    if (mLineColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColorWithTransparency(mLineColor, fTransparency));
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawIRect(SkIRect::MakeXYWH(nX, nY, nWidth - 1, nHeight - 1), paint);
    }
    postDraw();
}

void SkiaSalGraphicsImpl::drawRect(long nX, long nY, long nWidth, long nHeight)
{
    privateDrawAlphaRect(nX, nY, nWidth, nHeight, 0.0, true);
}

void SkiaSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(pPtAry->mnX, pPtAry->mnY), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPolygon.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].mnX, pPtAry[i].mnY));
    aPolygon.setClosed(false);

    drawPolyLine(basegfx::B2DHomMatrix(), aPolygon, 0.0, basegfx::B2DVector(1.0, 1.0),
                 basegfx::B2DLineJoin::Miter, css::drawing::LineCap_BUTT,
                 basegfx::deg2rad(15.0) /*default*/, false);
}

void SkiaSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(pPtAry->mnX, pPtAry->mnY), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPolygon.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].mnX, pPtAry[i].mnY));

    drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aPolygon), 0.0);
}

void SkiaSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                          PCONSTSALPOINT* pPtAry)
{
    basegfx::B2DPolyPolygon aPolyPolygon;
    for (sal_uInt32 nPolygon = 0; nPolygon < nPoly; ++nPolygon)
    {
        sal_uInt32 nPoints = pPoints[nPolygon];
        if (nPoints)
        {
            PCONSTSALPOINT pSalPoints = pPtAry[nPolygon];
            basegfx::B2DPolygon aPolygon;
            aPolygon.append(basegfx::B2DPoint(pSalPoints->mnX, pSalPoints->mnY), nPoints);
            for (sal_uInt32 i = 1; i < nPoints; ++i)
                aPolygon.setB2DPoint(i, basegfx::B2DPoint(pSalPoints[i].mnX, pSalPoints[i].mnY));

            aPolyPolygon.append(aPolygon);
        }
    }

    drawPolyPolygon(basegfx::B2DHomMatrix(), aPolyPolygon, 0.0);
}

bool SkiaSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                          const basegfx::B2DPolyPolygon& rPolyPolygon,
                                          double fTransparency)
{
    const bool bHasFill(mFillColor != SALCOLOR_NONE);
    const bool bHasLine(mLineColor != SALCOLOR_NONE);

    if (rPolyPolygon.count() == 0 || !(bHasFill || bHasLine) || fTransparency < 0.0
        || fTransparency >= 1.0)
        return true;

    preDraw();

    SkPath aPath;
    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);
    SAL_INFO("vcl.skia", "drawpolypolygon(" << this << "): " << aPolyPolygon << ":" << mLineColor
                                            << ":" << mFillColor);
    addPolyPolygonToPath(aPolyPolygon, aPath);
    aPath.setFillType(SkPathFillType::kEvenOdd);

    SkPaint aPaint;
    aPaint.setAntiAlias(mParent.getAntiAliasB2DDraw());
    if (mFillColor != SALCOLOR_NONE)
    {
        aPaint.setColor(toSkColorWithTransparency(mFillColor, fTransparency));
        aPaint.setStyle(SkPaint::kFill_Style);
        mSurface->getCanvas()->drawPath(aPath, aPaint);
    }
    if (mLineColor != SALCOLOR_NONE)
    {
        if (isGPU()) // Apply the same adjustment as toSkX()/toSkY() do.
            aPath.offset(0.5, 0.5, nullptr);
        aPaint.setColor(toSkColorWithTransparency(mLineColor, fTransparency));
        aPaint.setStyle(SkPaint::kStroke_Style);
        mSurface->getCanvas()->drawPath(aPath, aPaint);
    }
    postDraw();
    return true;
}

bool SkiaSalGraphicsImpl::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                       const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                                       const basegfx::B2DVector& rLineWidths,
                                       basegfx::B2DLineJoin eLineJoin,
                                       css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                                       bool bPixelSnapHairline)
{
    if (rPolyLine.count() == 0 || fTransparency < 0.0 || fTransparency >= 1.0
        || mLineColor == SALCOLOR_NONE)
        return true;

    preDraw();
    SAL_INFO("vcl.skia", "drawpolyline(" << this << "): " << rPolyLine << ":" << mLineColor);

    // need to check/handle LineWidth when ObjectToDevice transformation is used
    const basegfx::B2DVector aDeviceLineWidths(rObjectToDevice * rLineWidths);
    const bool bCorrectLineWidth(aDeviceLineWidths.getX() < 1.0 && rLineWidths.getX() >= 1.0);
    const basegfx::B2DVector aLineWidths(bCorrectLineWidth ? rLineWidths : aDeviceLineWidths);

    // Skia does not support B2DLineJoin::NONE; return false to use
    // the fallback (own geometry preparation),
    // linejoin-mode and thus the above only applies to "fat" lines.
    if ((basegfx::B2DLineJoin::NONE == eLineJoin) && (aLineWidths.getX() > 1.3))
        return false;

    // Transform to DeviceCoordinates, get DeviceLineWidth, execute PixelSnapHairline
    basegfx::B2DPolygon aPolyLine(rPolyLine);
    aPolyLine.transform(rObjectToDevice);
    if (bPixelSnapHairline)
        aPolyLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyLine);

    // Setup Line Join
    SkPaint::Join eSkLineJoin = SkPaint::kMiter_Join;
    switch (eLineJoin)
    {
        case basegfx::B2DLineJoin::Bevel:
            eSkLineJoin = SkPaint::kBevel_Join;
            break;
        case basegfx::B2DLineJoin::Round:
            eSkLineJoin = SkPaint::kRound_Join;
            break;
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
            eSkLineJoin = SkPaint::kMiter_Join;
            break;
    }

    // convert miter minimum angle to miter limit
    double fMiterLimit = 1.0 / std::sin(fMiterMinimumAngle / 2.0);

    // Setup Line Cap
    SkPaint::Cap eSkLineCap(SkPaint::kButt_Cap);

    switch (eLineCap)
    {
        case css::drawing::LineCap_ROUND:
            eSkLineCap = SkPaint::kRound_Cap;
            break;
        case css::drawing::LineCap_SQUARE:
            eSkLineCap = SkPaint::kSquare_Cap;
            break;
        default: // css::drawing::LineCap_BUTT:
            eSkLineCap = SkPaint::kButt_Cap;
            break;
    }

    SkPaint aPaint;
    aPaint.setStyle(SkPaint::kStroke_Style);
    aPaint.setStrokeCap(eSkLineCap);
    aPaint.setStrokeJoin(eSkLineJoin);
    aPaint.setColor(toSkColorWithTransparency(mLineColor, fTransparency));
    aPaint.setStrokeMiter(fMiterLimit);
    aPaint.setStrokeWidth(aLineWidths.getX());
    aPaint.setAntiAlias(mParent.getAntiAliasB2DDraw());

    SkPath aPath;
    addPolygonToPath(aPolyLine, aPath);
    aPath.setFillType(SkPathFillType::kEvenOdd);
    // Apply the same adjustment as toSkX()/toSkY() do. Do it here even in the non-GPU
    // case as it seems to produce better results.
    aPath.offset(0.5, 0.5, nullptr);
    mSurface->getCanvas()->drawPath(aPath, aPaint);
    postDraw();

    return true;
}

bool SkiaSalGraphicsImpl::drawPolyLineBezier(sal_uInt32, const SalPoint*, const PolyFlags*)
{
    // TODO?
    return false;
}

bool SkiaSalGraphicsImpl::drawPolygonBezier(sal_uInt32, const SalPoint*, const PolyFlags*)
{
    // TODO?
    return false;
}

bool SkiaSalGraphicsImpl::drawPolyPolygonBezier(sal_uInt32, const sal_uInt32*,
                                                const SalPoint* const*, const PolyFlags* const*)
{
    // TODO?
    return false;
}

void SkiaSalGraphicsImpl::copyArea(long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                   long nSrcHeight, bool /*bWindowInvalidate*/)
{
    if (nDestX == nSrcX && nDestY == nSrcY)
        return;
    preDraw();
    SAL_INFO("vcl.skia", "copyarea(" << this << "): " << Point(nSrcX, nSrcY) << "->"
                                     << Point(nDestX, nDestY) << "/"
                                     << Size(nSrcWidth, nSrcHeight));
    sk_sp<SkImage> image
        = mSurface->makeImageSnapshot(SkIRect::MakeXYWH(nSrcX, nSrcY, nSrcWidth, nSrcHeight));
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
    mSurface->getCanvas()->drawImage(image, nDestX, nDestY, &paint);
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
        src->checkSurface();
    }
    else
        src = this;
    SAL_INFO("vcl.skia", "copybits(" << this << "): (" << src << "):" << rPosAry);
    sk_sp<SkImage> image = src->mSurface->makeImageSnapshot(
        SkIRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight));
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
    mSurface->getCanvas()->drawImageRect(image,
                                         SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY,
                                                          rPosAry.mnDestWidth,
                                                          rPosAry.mnDestHeight),
                                         &paint);
    postDraw();
}

bool SkiaSalGraphicsImpl::blendBitmap(const SalTwoRect& rPosAry, const SalBitmap& rBitmap)
{
    if (checkInvalidSourceOrDestination(rPosAry))
        return false;

    assert(dynamic_cast<const SkiaSalBitmap*>(&rBitmap));
    const SkiaSalBitmap& rSkiaBitmap = static_cast<const SkiaSalBitmap&>(rBitmap);
    drawImage(rPosAry, rSkiaBitmap.GetSkImage(), SkBlendMode::kMultiply);
    return true;
}

bool SkiaSalGraphicsImpl::blendAlphaBitmap(const SalTwoRect& rPosAry,
                                           const SalBitmap& rSourceBitmap,
                                           const SalBitmap& rMaskBitmap,
                                           const SalBitmap& rAlphaBitmap)
{
    if (checkInvalidSourceOrDestination(rPosAry))
        return false;

    assert(dynamic_cast<const SkiaSalBitmap*>(&rSourceBitmap));
    assert(dynamic_cast<const SkiaSalBitmap*>(&rMaskBitmap));
    assert(dynamic_cast<const SkiaSalBitmap*>(&rAlphaBitmap));

    sk_sp<SkSurface> tmpSurface = SkiaHelper::createSkSurface(rSourceBitmap.GetSize());
    if (!tmpSurface)
        return false;

    const SkiaSalBitmap& rSkiaSourceBitmap = static_cast<const SkiaSalBitmap&>(rSourceBitmap);
    const SkiaSalBitmap& rSkiaMaskBitmap = static_cast<const SkiaSalBitmap&>(rMaskBitmap);
    const SkiaSalBitmap& rSkiaAlphaBitmap = static_cast<const SkiaSalBitmap&>(rAlphaBitmap);

    SkCanvas* aCanvas = tmpSurface->getCanvas();
    SkPaint aPaint;

    aPaint.setBlendMode(SkBlendMode::kSrc);
    aCanvas->drawImage(rSkiaSourceBitmap.GetSkImage(), 0, 0, &aPaint);

    // Apply cumulatively both the bitmap alpha and the device alpha.
    aPaint.setBlendMode(SkBlendMode::kDstOut); // VCL alpha is one-minus-alpha
    aCanvas->drawImage(rSkiaMaskBitmap.GetAlphaSkImage(), 0, 0, &aPaint);
    aPaint.setBlendMode(SkBlendMode::kDstOut); // VCL alpha is one-minus-alpha
    aCanvas->drawImage(rSkiaAlphaBitmap.GetAlphaSkImage(), 0, 0, &aPaint);

    drawImage(rPosAry, tmpSurface->makeImageSnapshot());
    return true;
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (checkInvalidSourceOrDestination(rPosAry))
        return;

    assert(dynamic_cast<const SkiaSalBitmap*>(&rSalBitmap));
    const SkiaSalBitmap& rSkiaSourceBitmap = static_cast<const SkiaSalBitmap&>(rSalBitmap);

    drawImage(rPosAry, rSkiaSourceBitmap.GetSkImage());
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                     const SalBitmap& rMaskBitmap)
{
    drawAlphaBitmap(rPosAry, rSalBitmap, rMaskBitmap);
}

void SkiaSalGraphicsImpl::drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                   Color nMaskColor)
{
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSalBitmap));
    drawMask(rPosAry, static_cast<const SkiaSalBitmap&>(rSalBitmap).GetAlphaSkImage(), nMaskColor);
}

void SkiaSalGraphicsImpl::drawMask(const SalTwoRect& rPosAry, const sk_sp<SkImage>& rImage,
                                   Color nMaskColor)
{
    SAL_INFO("vcl.skia", "drawmask(" << this << "): " << rPosAry << ":" << nMaskColor);
    sk_sp<SkSurface> tmpSurface = SkiaHelper::createSkSurface(rImage->width(), rImage->height());
    assert(tmpSurface);
    SkCanvas* canvas = tmpSurface->getCanvas();
    canvas->clear(toSkColor(nMaskColor));
    SkPaint paint;
    // Draw the color with the given mask.
    // TODO figure out the right blend mode to avoid the temporary surface
    paint.setBlendMode(SkBlendMode::kDstOut);
    canvas->drawImage(rImage, 0, 0, &paint);

    drawImage(rPosAry, tmpSurface->makeImageSnapshot());
}

std::shared_ptr<SalBitmap> SkiaSalGraphicsImpl::getBitmap(long nX, long nY, long nWidth,
                                                          long nHeight)
{
    checkSurface();
    SAL_INFO("vcl.skia",
             "getbitmap(" << this << "): " << Point(nX, nY) << "/" << Size(nWidth, nHeight));
    mSurface->getCanvas()->flush();
    sk_sp<SkImage> image = mSurface->makeImageSnapshot(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight));
    return std::make_shared<SkiaSalBitmap>(image);
}

Color SkiaSalGraphicsImpl::getPixel(long nX, long nY)
{
    checkSurface();
    SAL_INFO("vcl.skia", "getpixel(" << this << "): " << Point(nX, nY));
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
    SAL_INFO("vcl.skia", "invert(" << this << "): " << rPoly << ":" << int(eFlags));
    // TrackFrame just inverts a dashed path around the polygon
    if (eFlags == SalInvert::TrackFrame)
    {
        SkPath aPath;
        addPolygonToPath(rPoly, aPath);
        aPath.setFillType(SkPathFillType::kEvenOdd);
        // TrackFrame is not supposed to paint outside of the polygon (usually rectangle),
        // but wider stroke width usually results in that, so ensure the requirement
        // by clipping.
        SkAutoCanvasRestore autoRestore(mSurface->getCanvas(), true);
        mSurface->getCanvas()->clipRect(aPath.getBounds(), SkClipOp::kIntersect, false);
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
        addPolygonToPath(rPoly, aPath);
        aPath.setFillType(SkPathFillType::kEvenOdd);
        SkPaint aPaint;
        aPaint.setColor(SkColorSetARGB(255, 255, 255, 255));
        aPaint.setStyle(SkPaint::kFill_Style);
        aPaint.setBlendMode(SkBlendMode::kDifference);

        // N50 inverts in 4x4 checker pattern
        if (eFlags == SalInvert::N50)
        {
            // This creates 4x4 checker pattern bitmap
            // TODO Use SkiaHelper::createSkSurface() and cache the image
            SkBitmap aBitmap;
            aBitmap.allocN32Pixels(4, 4);
            const SkPMColor white = SkPreMultiplyARGB(0xFF, 0xFF, 0xFF, 0xFF);
            const SkPMColor black = SkPreMultiplyARGB(0xFF, 0x00, 0x00, 0x00);
            SkPMColor* scanline;
            scanline = aBitmap.getAddr32(0, 0);
            *scanline++ = white;
            *scanline++ = white;
            *scanline++ = black;
            *scanline++ = black;
            scanline = aBitmap.getAddr32(0, 1);
            *scanline++ = white;
            *scanline++ = white;
            *scanline++ = black;
            *scanline++ = black;
            scanline = aBitmap.getAddr32(0, 2);
            *scanline++ = black;
            *scanline++ = black;
            *scanline++ = white;
            *scanline++ = white;
            scanline = aBitmap.getAddr32(0, 3);
            *scanline++ = black;
            *scanline++ = black;
            *scanline++ = white;
            *scanline++ = white;
            aBitmap.setImmutable();
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

bool SkiaSalGraphicsImpl::drawEPS(long, long, long, long, void*, sal_uInt32)
{
    // TODO?
    return false;
}

bool SkiaSalGraphicsImpl::drawAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                          const SalBitmap& rAlphaBitmap)
{
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSourceBitmap));
    assert(dynamic_cast<const SkiaSalBitmap*>(&rAlphaBitmap));
    sk_sp<SkSurface> tmpSurface = SkiaHelper::createSkSurface(rSourceBitmap.GetSize());
    if (!tmpSurface)
        return false;
    SkCanvas* canvas = tmpSurface->getCanvas();
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
    canvas->drawImage(static_cast<const SkiaSalBitmap&>(rSourceBitmap).GetSkImage(), 0, 0, &paint);
    paint.setBlendMode(SkBlendMode::kDstOut); // VCL alpha is one-minus-alpha
    canvas->drawImage(static_cast<const SkiaSalBitmap&>(rAlphaBitmap).GetAlphaSkImage(), 0, 0,
                      &paint);
    drawImage(rPosAry, tmpSurface->makeImageSnapshot());
    return true;
}

void SkiaSalGraphicsImpl::drawImage(const SalTwoRect& rPosAry, const sk_sp<SkImage>& aImage,
                                    SkBlendMode eBlendMode)
{
    SkRect aSourceRect
        = SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    SkRect aDestinationRect = SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY,
                                               rPosAry.mnDestWidth, rPosAry.mnDestHeight);

    SkPaint aPaint;
    aPaint.setBlendMode(eBlendMode);

    preDraw();
    SAL_INFO("vcl.skia", "drawimage(" << this << "): " << rPosAry << ":" << int(eBlendMode));
    mSurface->getCanvas()->drawImageRect(aImage, aSourceRect, aDestinationRect, &aPaint);
    postDraw();
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SkBitmap& aBitmap,
                                     SkBlendMode eBlendMode)
{
    SkRect aSourceRect
        = SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    SkRect aDestinationRect = SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY,
                                               rPosAry.mnDestWidth, rPosAry.mnDestHeight);

    SkPaint aPaint;
    aPaint.setBlendMode(eBlendMode);

    preDraw();
    SAL_INFO("vcl.skia", "drawbitmap(" << this << "): " << rPosAry << ":" << int(eBlendMode));
    mSurface->getCanvas()->drawBitmapRect(aBitmap, aSourceRect, aDestinationRect, &aPaint);
    postDraw();
}

bool SkiaSalGraphicsImpl::drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                                const basegfx::B2DPoint& rX,
                                                const basegfx::B2DPoint& rY,
                                                const SalBitmap& rSourceBitmap,
                                                const SalBitmap* pAlphaBitmap)
{
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSourceBitmap));
    assert(!pAlphaBitmap || dynamic_cast<const SkiaSalBitmap*>(pAlphaBitmap));

    const SkiaSalBitmap& rSkiaBitmap = static_cast<const SkiaSalBitmap&>(rSourceBitmap);
    const SkiaSalBitmap* pSkiaAlphaBitmap = static_cast<const SkiaSalBitmap*>(pAlphaBitmap);

    sk_sp<SkSurface> tmpSurface = SkiaHelper::createSkSurface(rSourceBitmap.GetSize());
    if (!tmpSurface)
        return false;

    // Combine bitmap + alpha bitmap into one temporary bitmap with alpha
    SkCanvas* aCanvas = tmpSurface->getCanvas();
    SkPaint aPaint;
    aPaint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
    aCanvas->drawImage(rSkiaBitmap.GetSkImage(), 0, 0, &aPaint);
    if (pSkiaAlphaBitmap != nullptr)
    {
        aPaint.setBlendMode(SkBlendMode::kDstOut); // VCL alpha is one-minus-alpha
        aCanvas->drawImage(pSkiaAlphaBitmap->GetAlphaSkImage(), 0, 0, &aPaint);
    }
    // setup the image transformation
    // using the rNull, rX, rY points as destinations for the (0,0), (0,Width), (Height,0) source points
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;

    const Size aSize = rSourceBitmap.GetSize();

    SkMatrix aMatrix;
    aMatrix.set(SkMatrix::kMScaleX, aXRel.getX() / aSize.Width());
    aMatrix.set(SkMatrix::kMSkewY, aXRel.getY() / aSize.Width());
    aMatrix.set(SkMatrix::kMSkewX, aYRel.getX() / aSize.Height());
    aMatrix.set(SkMatrix::kMScaleY, aYRel.getY() / aSize.Height());
    aMatrix.set(SkMatrix::kMTransX, rNull.getX());
    aMatrix.set(SkMatrix::kMTransY, rNull.getY());

    preDraw();
    SAL_INFO("vcl.skia",
             "drawtransformedbitmap(" << this << "): " << rNull << ":" << rX << ":" << rY);
    {
        SkAutoCanvasRestore autoRestore(mSurface->getCanvas(), true);
        mSurface->getCanvas()->concat(aMatrix);
        mSurface->getCanvas()->drawImage(tmpSurface->makeImageSnapshot(), 0, 0);
    }
    postDraw();

    return true;
}

bool SkiaSalGraphicsImpl::drawAlphaRect(long nX, long nY, long nWidth, long nHeight,
                                        sal_uInt8 nTransparency)
{
    privateDrawAlphaRect(nX, nY, nWidth, nHeight, nTransparency / 100.0);
    return true;
}

bool SkiaSalGraphicsImpl::drawGradient(const tools::PolyPolygon&, const Gradient&)
{
    // TODO?
    return false;
}

bool SkiaSalGraphicsImpl::supportsOperation(OutDevSupportType eType) const
{
    switch (eType)
    {
        case OutDevSupportType::B2DDraw:
        case OutDevSupportType::TransparentRect:
            return true;
        default:
            return false;
    }
}

#ifdef DBG_UTIL
void SkiaSalGraphicsImpl::dump(const char* file) const
{
    assert(mSurface.get());
    SkiaHelper::dump(mSurface, file);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
