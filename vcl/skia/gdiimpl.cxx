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
#include <vcl/gradient.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <skia/utils.hxx>
#include <skia/zone.hxx>

#include <SkCanvas.h>
#include <SkGradientShader.h>
#include <SkPath.h>
#include <SkRegion.h>
#include <SkDashPathEffect.h>
#include <GrBackendSurface.h>
#include <SkTextBlob.h>
#include <SkRSXform.h>

#include <numeric>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <o3tl/sorted_vector.hxx>
#include <rtl/math.hxx>

using namespace SkiaHelper;

namespace
{
// Create Skia Path from B2DPolygon
// Note that polygons generally have the complication that when used
// for area (fill) operations they usually miss the right-most and
// bottom-most line of pixels of the bounding rectangle (see
// https://lists.freedesktop.org/archives/libreoffice/2019-November/083709.html).
// So be careful with rectangle->polygon conversions (generally avoid them).
void addPolygonToPath(const basegfx::B2DPolygon& rPolygon, SkPath& rPath, sal_uInt32 nFirstIndex,
                      sal_uInt32 nLastIndex, const sal_uInt32 nPointCount, const bool bClosePath,
                      const bool bHasCurves, bool* hasOnlyOrthogonal = nullptr)
{
    assert(nFirstIndex < nPointCount);
    assert(nLastIndex <= nPointCount);

    if (nPointCount <= 1)
        return;

    bool bFirst = true;
    sal_uInt32 nPreviousIndex = nFirstIndex == 0 ? nPointCount - 1 : nFirstIndex - 1;
    basegfx::B2DPoint aPreviousPoint = rPolygon.getB2DPoint(nPreviousIndex);

    for (sal_uInt32 nIndex = nFirstIndex; nIndex <= nLastIndex; nIndex++)
    {
        if (nIndex == nPointCount && !bClosePath)
            continue;

        // Make sure we loop the last point to first point
        sal_uInt32 nCurrentIndex = nIndex % nPointCount;
        basegfx::B2DPoint aCurrentPoint = rPolygon.getB2DPoint(nCurrentIndex);

        if (bFirst)
        {
            rPath.moveTo(aCurrentPoint.getX(), aCurrentPoint.getY());
            bFirst = false;
        }
        else if (!bHasCurves)
        {
            rPath.lineTo(aCurrentPoint.getX(), aCurrentPoint.getY());
            // If asked for, check whether the polygon has a line that is not
            // strictly horizontal or vertical.
            if (hasOnlyOrthogonal != nullptr && aCurrentPoint.getX() != aPreviousPoint.getX()
                && aCurrentPoint.getY() != aPreviousPoint.getY())
                *hasOnlyOrthogonal = false;
        }
        else
        {
            basegfx::B2DPoint aPreviousControlPoint = rPolygon.getNextControlPoint(nPreviousIndex);
            basegfx::B2DPoint aCurrentControlPoint = rPolygon.getPrevControlPoint(nCurrentIndex);

            if (aPreviousControlPoint.equal(aPreviousPoint)
                && aCurrentControlPoint.equal(aCurrentPoint))
            {
                rPath.lineTo(aCurrentPoint.getX(), aCurrentPoint.getY()); // a straight line
                if (hasOnlyOrthogonal != nullptr && aCurrentPoint.getX() != aPreviousPoint.getX()
                    && aCurrentPoint.getY() != aPreviousPoint.getY())
                    *hasOnlyOrthogonal = false;
            }
            else
            {
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
                if (hasOnlyOrthogonal != nullptr)
                    *hasOnlyOrthogonal = false;
            }
        }
        aPreviousPoint = aCurrentPoint;
        nPreviousIndex = nCurrentIndex;
    }
    if (bClosePath && nFirstIndex == 0 && nLastIndex == nPointCount)
    {
        rPath.close();
    }
}

void addPolygonToPath(const basegfx::B2DPolygon& rPolygon, SkPath& rPath,
                      bool* hasOnlyOrthogonal = nullptr)
{
    addPolygonToPath(rPolygon, rPath, 0, rPolygon.count(), rPolygon.count(), rPolygon.isClosed(),
                     rPolygon.areControlPointsUsed(), hasOnlyOrthogonal);
}

void addPolyPolygonToPath(const basegfx::B2DPolyPolygon& rPolyPolygon, SkPath& rPath,
                          bool* hasOnlyOrthogonal = nullptr)
{
    const sal_uInt32 nPolygonCount(rPolyPolygon.count());

    if (nPolygonCount == 0)
        return;

    sal_uInt32 nPointCount = 0;
    for (const auto& rPolygon : rPolyPolygon)
        nPointCount += rPolygon.count() * 3; // because cubicTo is 3 elements
    rPath.incReserve(nPointCount);

    for (const auto& rPolygon : rPolyPolygon)
    {
        addPolygonToPath(rPolygon, rPath, hasOnlyOrthogonal);
    }
}

// Check if the given polygon contains a straight line. If not, it consists
// solely of curves.
bool polygonContainsLine(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    if (!rPolyPolygon.areControlPointsUsed())
        return true; // no curves at all
    for (const auto& rPolygon : rPolyPolygon)
    {
        const sal_uInt32 nPointCount(rPolygon.count());
        bool bFirst = true;

        const bool bClosePath(rPolygon.isClosed());

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
            if (bFirst)
                bFirst = false;
            else
            {
                basegfx::B2DPoint aPreviousControlPoint
                    = rPolygon.getNextControlPoint(nPreviousIndex);
                basegfx::B2DPoint aCurrentControlPoint
                    = rPolygon.getPrevControlPoint(nCurrentIndex);

                if (aPreviousControlPoint.equal(aPreviousPoint)
                    && aCurrentControlPoint.equal(aCurrentPoint))
                {
                    return true; // found a straight line
                }
            }
            aPreviousPoint = aCurrentPoint;
            nPreviousIndex = nCurrentIndex;
        }
    }
    return false; // no straight line found
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
#ifndef NDEBUG
    char* debugname;
#endif

public:
    explicit SkiaFlushIdle(SkiaSalGraphicsImpl* pGraphics)
        : Idle(get_debug_name(pGraphics))
        , mpGraphics(pGraphics)
    {
        // We don't want to be swapping before we've painted.
        SetPriority(TaskPriority::POST_PAINT);
    }
#ifndef NDEBUG
    virtual ~SkiaFlushIdle() { free(debugname); }
#endif
    const char* get_debug_name(SkiaSalGraphicsImpl* pGraphics)
    {
#ifndef NDEBUG
        // Idle keeps just a pointer, so we need to store the string
        debugname = strdup(
            OString("skia idle 0x" + OString::number(reinterpret_cast<sal_uIntPtr>(pGraphics), 16))
                .getStr());
        return debugname;
#else
        (void)pGraphics;
        return "skia idle";
#endif
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
    , mXorMode(XorMode::None)
    , mFlush(new SkiaFlushIdle(this))
    , mPendingOperationsToFlush(0)
    , mScaling(1)
{
}

SkiaSalGraphicsImpl::~SkiaSalGraphicsImpl()
{
    assert(!mSurface);
    assert(!mWindowContext);
}

void SkiaSalGraphicsImpl::Init() {}

void SkiaSalGraphicsImpl::createSurface()
{
    SkiaZone zone;
    if (isOffscreen())
        createOffscreenSurface();
    else
        createWindowSurface();
    mClipRegion = vcl::Region(tools::Rectangle(0, 0, GetWidth(), GetHeight()));
    mDirtyRect = SkIRect::MakeWH(GetWidth(), GetHeight());
    setCanvasScalingAndClipping();

    // We don't want to be swapping before we've painted.
    mFlush->Stop();
    mFlush->SetPriority(TaskPriority::POST_PAINT);
}

void SkiaSalGraphicsImpl::createWindowSurface(bool forceRaster)
{
    SkiaZone zone;
    assert(!isOffscreen());
    assert(!mSurface);
    createWindowSurfaceInternal(forceRaster);
    if (!mSurface)
    {
        switch (renderMethodToUse())
        {
            case RenderVulkan:
                SAL_WARN("vcl.skia",
                         "cannot create Vulkan GPU window surface, falling back to Raster");
                destroySurface(); // destroys also WindowContext
                return createWindowSurface(true); // try again
            case RenderMetal:
                SAL_WARN("vcl.skia",
                         "cannot create Metal GPU window surface, falling back to Raster");
                destroySurface(); // destroys also WindowContext
                return createWindowSurface(true); // try again
            case RenderRaster:
                abort(); // This should not really happen, do not even try to cope with it.
        }
    }
    mIsGPU = mSurface->getCanvas()->recordingContext() != nullptr;
#ifdef DBG_UTIL
    prefillSurface(mSurface);
#endif
}

bool SkiaSalGraphicsImpl::isOffscreen() const
{
    if (mProvider == nullptr || mProvider->IsOffScreen())
        return true;
    // HACK: Sometimes (tdf#131939, tdf#138022, tdf#140288) VCL passes us a zero-sized window,
    // and zero size is invalid for Skia, so force offscreen surface, where we handle this.
    if (GetWidth() <= 0 || GetHeight() <= 0)
        return true;
    return false;
}

void SkiaSalGraphicsImpl::createOffscreenSurface()
{
    SkiaZone zone;
    assert(isOffscreen());
    assert(!mSurface);
    // HACK: See isOffscreen().
    int width = std::max(1, GetWidth());
    int height = std::max(1, GetHeight());
    // We need to use window scaling even for offscreen surfaces, because the common usage is rendering something
    // into an offscreen surface and then copy it to a window, so without scaling here the result would be originally
    // drawn without scaling and only upscaled when drawing to a window.
    mScaling = getWindowScaling();
    mSurface = createSkSurface(width * mScaling, height * mScaling);
    assert(mSurface);
    mIsGPU = mSurface->getCanvas()->recordingContext() != nullptr;
}

void SkiaSalGraphicsImpl::destroySurface()
{
    SkiaZone zone;
    if (mSurface)
    {
        // check setClipRegion() invariant
        assert(mSurface->getCanvas()->getSaveCount() == 3);
        // if this fails, something forgot to use SkAutoCanvasRestore
        assert(mSurface->getCanvas()->getTotalMatrix() == SkMatrix::Scale(mScaling, mScaling));
    }
    // If we use e.g. Vulkan, we must destroy the surface before the context,
    // otherwise destroying the surface will reference the context. This is
    // handled by calling destroySurface() before destroying the context.
    // However we also need to flush the surface before destroying it,
    // otherwise when destroying the context later there still could be queued
    // commands referring to the surface data. This is probably a Skia bug,
    // but work around it here.
    if (mSurface)
        mSurface->flushAndSubmit();
    mSurface.reset();
    mWindowContext.reset();
    mIsGPU = false;
    mScaling = 1;
}

void SkiaSalGraphicsImpl::performFlush()
{
    SkiaZone zone;
    flushDrawing();
    if (mSurface)
    {
        if (mDirtyRect.intersect(SkIRect::MakeWH(GetWidth(), GetHeight())))
            flushSurfaceToWindowContext();
        mDirtyRect.setEmpty();
    }
}

void SkiaSalGraphicsImpl::flushSurfaceToWindowContext()
{
    sk_sp<SkSurface> screenSurface = mWindowContext->getBackbufferSurface();
    if (screenSurface != mSurface)
    {
        // GPU-based window contexts require calling getBackbufferSurface()
        // for every swapBuffers(), for this reason mSurface is an offscreen surface
        // where we keep the contents (LO does not do full redraws).
        // So here blit the surface to the window context surface and then swap it.
        assert(isGPU()); // Raster should always draw directly to backbuffer to save copying
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // copy as is
        // We ignore mDirtyRect here, and mSurface already is in screenSurface coordinates,
        // so no transformation needed.
        screenSurface->getCanvas()->drawImage(makeCheckedImageSnapshot(mSurface), 0, 0,
                                              SkSamplingOptions(), &paint);
        screenSurface->flushAndSubmit(); // Otherwise the window is not drawn sometimes.
        mWindowContext->swapBuffers(nullptr); // Must swap the entire surface.
    }
    else
    {
        // For raster mode use directly the backbuffer surface, it's just a bitmap
        // surface anyway, and for those there's no real requirement to call
        // getBackbufferSurface() repeatedly. Using our own surface would duplicate
        // memory and cost time copying pixels around.
        assert(!isGPU());
        SkIRect dirtyRect = mDirtyRect;
        if (mScaling != 1) // Adjust to mSurface coordinates if needed.
            dirtyRect = scaleRect(dirtyRect, mScaling);
        mWindowContext->swapBuffers(&dirtyRect);
    }
}

void SkiaSalGraphicsImpl::DeInit() { destroySurface(); }

void SkiaSalGraphicsImpl::preDraw()
{
    assert(comphelper::SolarMutex::get()->IsCurrentThread());
    SkiaZone::enter(); // matched in postDraw()
    checkSurface();
    checkPendingDrawing();
}

void SkiaSalGraphicsImpl::postDraw()
{
    scheduleFlush();
    // Skia (at least when using Vulkan) queues drawing commands and executes them only later.
    // But tdf#136369 leads to creating and queueing many tiny bitmaps, which makes
    // Skia slow, and may make it even run out of memory. So force a flush if such
    // a problematic operation has been performed too many times without a flush.
    if (mPendingOperationsToFlush > 1000)
    {
        mSurface->flushAndSubmit();
        mPendingOperationsToFlush = 0;
    }
    SkiaZone::leave(); // matched in preDraw()
    // If there's a problem with the GPU context, abort.
    if (GrDirectContext* context = GrAsDirectContext(mSurface->getCanvas()->recordingContext()))
    {
        // Running out of memory on the GPU technically could be possibly recoverable,
        // but we don't know the exact status of the surface (and what has or has not been drawn to it),
        // so in practice this is unrecoverable without possible data loss.
        if (context->oomed())
        {
            SAL_WARN("vcl.skia", "GPU context has run out of memory, aborting.");
            abort();
        }
        // Unrecoverable problem.
        if (context->abandoned())
        {
            SAL_WARN("vcl.skia", "GPU context has been abandoned, aborting.");
            abort();
        }
    }
}

void SkiaSalGraphicsImpl::scheduleFlush()
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
        createSurface();
        SAL_INFO("vcl.skia.trace",
                 "create(" << this << "): " << Size(mSurface->width(), mSurface->height()));
    }
    else if (GetWidth() * mScaling != mSurface->width()
             || GetHeight() * mScaling != mSurface->height())
    {
        if (!avoidRecreateByResize())
        {
            Size oldSize(mSurface->width(), mSurface->height());
            // Recreating a surface means that the old SkSurface contents will be lost.
            // But if a window has been resized the windowing system may send repaint events
            // only for changed parts and VCL would not repaint the whole area, assuming
            // that some parts have not changed (this is what seems to cause tdf#131952).
            // So carry over the old contents for windows, even though generally everything
            // will be usually repainted anyway.
            sk_sp<SkImage> snapshot;
            if (!isOffscreen())
            {
                flushDrawing();
                snapshot = makeCheckedImageSnapshot(mSurface);
            }

            destroySurface();
            createSurface();

            if (snapshot)
            {
                SkPaint paint;
                paint.setBlendMode(SkBlendMode::kSrc); // copy as is
                // Scaling by current mScaling is active, undo that. We assume that the scaling
                // does not change.
                resetCanvasScalingAndClipping();
                mSurface->getCanvas()->drawImage(snapshot, 0, 0, SkSamplingOptions(), &paint);
                setCanvasScalingAndClipping();
            }
            SAL_INFO("vcl.skia.trace", "recreate(" << this << "): old " << oldSize << " new "
                                                   << Size(mSurface->width(), mSurface->height())
                                                   << " requested "
                                                   << Size(GetWidth(), GetHeight()));
        }
    }
}

bool SkiaSalGraphicsImpl::avoidRecreateByResize() const
{
    // Keep the old surface if VCL sends us a broken size (see isOffscreen()).
    if (GetWidth() == 0 || GetHeight() == 0)
        return true;
    return false;
}

void SkiaSalGraphicsImpl::flushDrawing()
{
    if (!mSurface)
        return;
    checkPendingDrawing();
    mSurface->flushAndSubmit();
    mPendingOperationsToFlush = 0;
}

void SkiaSalGraphicsImpl::setCanvasScalingAndClipping()
{
    SkCanvas* canvas = mSurface->getCanvas();
    assert(canvas->getSaveCount() == 1);
    // If HiDPI scaling is active, simply set a scaling matrix for the canvas. This means
    // that all painting can use VCL coordinates and they'll be automatically translated to mSurface
    // scaled coordinates. If that is not wanted, the scale() state needs to be temporarily unset.
    // State such as mDirtyRect is not scaled, the scaling matrix applies to clipping too,
    // and the rest needs to be handled explicitly.
    // When reading mSurface contents there's no automatic scaling and it needs to be handled explicitly.
    canvas->save(); // keep the original state without any scaling
    canvas->scale(mScaling, mScaling);

    // SkCanvas::clipRegion() can only further reduce the clip region,
    // but we need to set the given region, which may extend it.
    // So handle that by always having the full clip region saved on the stack
    // and always go back to that. SkCanvas::restore() only affects the clip
    // and the matrix.
    canvas->save(); // keep scaled state without clipping
    setCanvasClipRegion(canvas, mClipRegion);
}

void SkiaSalGraphicsImpl::resetCanvasScalingAndClipping()
{
    SkCanvas* canvas = mSurface->getCanvas();
    assert(canvas->getSaveCount() == 3);
    canvas->restore(); // undo clipping
    canvas->restore(); // undo scaling
}

bool SkiaSalGraphicsImpl::setClipRegion(const vcl::Region& region)
{
    if (mClipRegion == region)
        return true;
    SkiaZone zone;
    checkPendingDrawing();
    checkSurface();
    mClipRegion = region;
    SAL_INFO("vcl.skia.trace", "setclipregion(" << this << "): " << region);
    SkCanvas* canvas = mSurface->getCanvas();
    assert(canvas->getSaveCount() == 3);
    canvas->restore(); // undo previous clip state, see setCanvasScalingAndClipping()
    canvas->save();
    setCanvasClipRegion(canvas, region);
    return true;
}

void SkiaSalGraphicsImpl::setCanvasClipRegion(SkCanvas* canvas, const vcl::Region& region)
{
    SkiaZone zone;
    SkPath path;
    // Always use region rectangles, regardless of what the region uses internally.
    // That's what other VCL backends do, and trying to use addPolyPolygonToPath()
    // in case a polygon is used leads to off-by-one errors such as tdf#133208.
    RectangleVector rectangles;
    region.GetRegionRectangles(rectangles);
    path.incReserve(rectangles.size() + 1);
    for (const tools::Rectangle& rectangle : rectangles)
        path.addRect(SkRect::MakeXYWH(rectangle.getX(), rectangle.getY(), rectangle.GetWidth(),
                                      rectangle.GetHeight()));
    path.setFillType(SkPathFillType::kEvenOdd);
    canvas->clipPath(path);
}

void SkiaSalGraphicsImpl::ResetClipRegion()
{
    setClipRegion(vcl::Region(tools::Rectangle(0, 0, GetWidth(), GetHeight())));
}

const vcl::Region& SkiaSalGraphicsImpl::getClipRegion() const { return mClipRegion; }

sal_uInt16 SkiaSalGraphicsImpl::GetBitCount() const { return 32; }

tools::Long SkiaSalGraphicsImpl::GetGraphicsWidth() const { return GetWidth(); }

void SkiaSalGraphicsImpl::SetLineColor()
{
    checkPendingDrawing();
    mLineColor = SALCOLOR_NONE;
}

void SkiaSalGraphicsImpl::SetLineColor(Color nColor)
{
    checkPendingDrawing();
    mLineColor = nColor;
}

void SkiaSalGraphicsImpl::SetFillColor()
{
    checkPendingDrawing();
    mFillColor = SALCOLOR_NONE;
}

void SkiaSalGraphicsImpl::SetFillColor(Color nColor)
{
    checkPendingDrawing();
    mFillColor = nColor;
}

void SkiaSalGraphicsImpl::SetXORMode(bool set, bool invert)
{
    XorMode newMode = set ? (invert ? XorMode::Invert : XorMode::Xor) : XorMode::None;
    if (newMode == mXorMode)
        return;
    checkPendingDrawing();
    SAL_INFO("vcl.skia.trace", "setxormode(" << this << "): " << set << "/" << invert);
    mXorMode = newMode;
}

void SkiaSalGraphicsImpl::SetROPLineColor(SalROPColor nROPColor)
{
    checkPendingDrawing();
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
    checkPendingDrawing();
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

void SkiaSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY)
{
    drawPixel(nX, nY, mLineColor);
}

void SkiaSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY, Color nColor)
{
    if (nColor == SALCOLOR_NONE)
        return;
    preDraw();
    SAL_INFO("vcl.skia.trace", "drawpixel(" << this << "): " << Point(nX, nY) << ":" << nColor);
    addUpdateRegion(SkRect::MakeXYWH(nX, nY, 1, 1));
    SkPaint paint = makePixelPaint(nColor);
    // Apparently drawPixel() is actually expected to set the pixel and not draw it.
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    if (mScaling != 1 && isUnitTestRunning())
    {
        // On HiDPI displays, draw a square on the entire non-hidpi "pixel" when running unittests,
        // since tests often require precise pixel drawing.
        paint.setStrokeWidth(1); // this will be scaled by mScaling
        paint.setStrokeCap(SkPaint::kSquare_Cap);
    }
    getDrawCanvas()->drawPoint(toSkX(nX), toSkY(nY), paint);
    postDraw();
}

void SkiaSalGraphicsImpl::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                                   tools::Long nY2)
{
    if (mLineColor == SALCOLOR_NONE)
        return;
    preDraw();
    SAL_INFO("vcl.skia.trace", "drawline(" << this << "): " << Point(nX1, nY1) << "->"
                                           << Point(nX2, nY2) << ":" << mLineColor);
    addUpdateRegion(SkRect::MakeLTRB(nX1, nY1, nX2, nY2).makeSorted());
    SkPaint paint = makeLinePaint();
    paint.setAntiAlias(mParent.getAntiAlias());
    if (mScaling != 1 && isUnitTestRunning())
    {
        // On HiDPI displays, do not draw hairlines, draw 1-pixel wide lines in order to avoid
        // smoothing that would confuse unittests.
        paint.setStrokeWidth(1); // this will be scaled by mScaling
        paint.setStrokeCap(SkPaint::kSquare_Cap);
    }
    getDrawCanvas()->drawLine(toSkX(nX1), toSkY(nY1), toSkX(nX2), toSkY(nY2), paint);
    postDraw();
}

void SkiaSalGraphicsImpl::privateDrawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                               tools::Long nHeight, double fTransparency,
                                               bool blockAA)
{
    preDraw();
    SAL_INFO("vcl.skia.trace",
             "privatedrawrect(" << this << "): " << SkIRect::MakeXYWH(nX, nY, nWidth, nHeight)
                                << ":" << mLineColor << ":" << mFillColor << ":" << fTransparency);
    addUpdateRegion(SkRect::MakeXYWH(nX, nY, nWidth, nHeight));
    SkCanvas* canvas = getDrawCanvas();
    if (mFillColor != SALCOLOR_NONE)
    {
        SkPaint paint = makeFillPaint(fTransparency);
        paint.setAntiAlias(!blockAA && mParent.getAntiAlias());
        // HACK: If the polygon is just a line, it still should be drawn. But when filling
        // Skia doesn't draw empty polygons, so in that case ensure the line is drawn.
        if (mLineColor == SALCOLOR_NONE && SkSize::Make(nWidth, nHeight).isEmpty())
            paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawIRect(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight), paint);
    }
    if (mLineColor != SALCOLOR_NONE && mLineColor != mFillColor) // otherwise handled by fill
    {
        SkPaint paint = makeLinePaint(fTransparency);
        paint.setAntiAlias(!blockAA && mParent.getAntiAlias());
        if (mScaling != 1 && isUnitTestRunning())
        {
            // On HiDPI displays, do not draw just a hairline but instead a full-width "pixel" when running unittests,
            // since tests often require precise pixel drawing.
            paint.setStrokeWidth(1); // this will be scaled by mScaling
            paint.setStrokeCap(SkPaint::kSquare_Cap);
        }
        // The obnoxious "-1 DrawRect()" hack that I don't understand the purpose of (and I'm not sure
        // if anybody does), but without it some cases do not work. The max() is needed because Skia
        // will not draw anything if width or height is 0.
        canvas->drawRect(SkRect::MakeXYWH(toSkX(nX), toSkY(nY),
                                          std::max(tools::Long(1), nWidth - 1),
                                          std::max(tools::Long(1), nHeight - 1)),
                         paint);
    }
    postDraw();
}

void SkiaSalGraphicsImpl::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                   tools::Long nHeight)
{
    privateDrawAlphaRect(nX, nY, nWidth, nHeight, 0.0, true);
}

void SkiaSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPolygon.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));
    aPolygon.setClosed(false);

    drawPolyLine(basegfx::B2DHomMatrix(), aPolygon, 0.0, 1.0, nullptr, basegfx::B2DLineJoin::Miter,
                 css::drawing::LineCap_BUTT, basegfx::deg2rad(15.0) /*default*/, false);
}

void SkiaSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPolygon.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));

    drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aPolygon), 0.0);
}

void SkiaSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                          const Point** pPtAry)
{
    basegfx::B2DPolyPolygon aPolyPolygon;
    for (sal_uInt32 nPolygon = 0; nPolygon < nPoly; ++nPolygon)
    {
        sal_uInt32 nPoints = pPoints[nPolygon];
        if (nPoints)
        {
            const Point* pSubPoints = pPtAry[nPolygon];
            basegfx::B2DPolygon aPolygon;
            aPolygon.append(basegfx::B2DPoint(pSubPoints->getX(), pSubPoints->getY()), nPoints);
            for (sal_uInt32 i = 1; i < nPoints; ++i)
                aPolygon.setB2DPoint(i,
                                     basegfx::B2DPoint(pSubPoints[i].getX(), pSubPoints[i].getY()));

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

    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);

    SAL_INFO("vcl.skia.trace", "drawpolypolygon(" << this << "): " << aPolyPolygon << ":"
                                                  << mLineColor << ":" << mFillColor);

    if (delayDrawPolyPolygon(aPolyPolygon, fTransparency))
    {
        scheduleFlush();
        return true;
    }

    performDrawPolyPolygon(aPolyPolygon, fTransparency, mParent.getAntiAlias());
    return true;
}

void SkiaSalGraphicsImpl::performDrawPolyPolygon(const basegfx::B2DPolyPolygon& aPolyPolygon,
                                                 double fTransparency, bool useAA)
{
    preDraw();

    SkPath polygonPath;
    bool hasOnlyOrthogonal = true;
    addPolyPolygonToPath(aPolyPolygon, polygonPath, &hasOnlyOrthogonal);
    polygonPath.setFillType(SkPathFillType::kEvenOdd);
    addUpdateRegion(polygonPath.getBounds());

    // For lines we use toSkX()/toSkY() in order to pass centers of pixels to Skia,
    // as that leads to better results with floating-point coordinates
    // (e.g. https://bugs.chromium.org/p/skia/issues/detail?id=9611).
    // But that means that we generally need to use it also for areas, so that they
    // line up properly if used together (tdf#134346).
    // On the other hand, with AA enabled and rectangular areas, this leads to fuzzy
    // edges (tdf#137329). But since rectangular areas line up perfectly to pixels
    // everywhere, it shouldn't be necessary to do this for them.
    // So if AA is enabled, avoid this fixup for rectangular areas.
    if (!useAA || !hasOnlyOrthogonal)
    {
        // We normally use pixel at their center positions, but slightly off (see toSkX/Y()).
        // With AA lines that "slightly off" causes tiny changes of color, making some tests
        // fail. Since moving AA-ed line slightly to a side doesn't cause any real visual
        // difference, just place exactly at the center. tdf#134346
        const SkScalar posFix = useAA ? toSkXYFix : 0;
        polygonPath.offset(toSkX(0) + posFix, toSkY(0) + posFix, nullptr);
    }
    if (mFillColor != SALCOLOR_NONE)
    {
        SkPaint aPaint = makeFillPaint(fTransparency);
        aPaint.setAntiAlias(useAA);
        // HACK: If the polygon is just a line, it still should be drawn. But when filling
        // Skia doesn't draw empty polygons, so in that case ensure the line is drawn.
        if (mLineColor == SALCOLOR_NONE && polygonPath.getBounds().isEmpty())
            aPaint.setStyle(SkPaint::kStroke_Style);
        getDrawCanvas()->drawPath(polygonPath, aPaint);
    }
    if (mLineColor != SALCOLOR_NONE && mLineColor != mFillColor) // otherwise handled by fill
    {
        SkPaint aPaint = makeLinePaint(fTransparency);
        aPaint.setAntiAlias(useAA);
        getDrawCanvas()->drawPath(polygonPath, aPaint);
    }
    postDraw();
#if defined LINUX
    // WORKAROUND: The logo in the about dialog has drawing errors. This seems to happen
    // only on Linux (not Windows on the same machine), with both AMDGPU and Mesa,
    // and only when antialiasing is enabled. Flushing seems to avoid the problem.
    if (useAA && getVendor() == DriverBlocklist::VendorAMD)
        mSurface->flushAndSubmit();
#endif
}

namespace
{
struct LessThan
{
    bool operator()(const basegfx::B2DPoint& point1, const basegfx::B2DPoint& point2) const
    {
        if (basegfx::fTools::equal(point1.getX(), point2.getX()))
            return basegfx::fTools::less(point1.getY(), point2.getY());
        return basegfx::fTools::less(point1.getX(), point2.getX());
    }
};
} // namespace

bool SkiaSalGraphicsImpl::delayDrawPolyPolygon(const basegfx::B2DPolyPolygon& aPolyPolygon,
                                               double fTransparency)
{
    // There is some code that needlessly subdivides areas into adjacent rectangles,
    // but Skia doesn't line them up perfectly if AA is enabled (e.g. Cairo, Qt5 do,
    // but Skia devs claim it's working as intended
    // https://groups.google.com/d/msg/skia-discuss/NlKpD2X_5uc/Vuwd-kyYBwAJ).
    // An example is tdf#133016, which triggers SvgStyleAttributes::add_stroke()
    // implementing a line stroke as a bunch of polygons instead of just one, and
    // SvgLinearAtomPrimitive2D::create2DDecomposition() creates a gradient
    // as a series of polygons of gradually changing color. Those places should be
    // changed, but try to merge those split polygons back into the original one,
    // where the needlessly created edges causing problems will not exist.
    // This means drawing of such polygons needs to be delayed, so that they can
    // be possibly merged with the next one.
    // Merge only polygons of the same properties (color, etc.), so the gradient problem
    // actually isn't handled here.

    // Only AA polygons need merging, because they do not line up well because of the AA of the edges.
    if (!mParent.getAntiAlias())
        return false;
    // Only filled polygons without an outline are problematic.
    if (mFillColor == SALCOLOR_NONE || mLineColor != SALCOLOR_NONE)
        return false;
    // Merge only simple polygons, real polypolygons most likely aren't needlessly split,
    // so they do not need joining.
    if (aPolyPolygon.count() != 1)
        return false;
    // If the polygon is not closed, it doesn't mark an area to be filled.
    if (!aPolyPolygon.isClosed())
        return false;
    // If a polygon does not contain a straight line, i.e. it's all curves, then do not merge.
    // First of all that's even more expensive, and second it's very unlikely that it's a polygon
    // split into more polygons.
    if (!polygonContainsLine(aPolyPolygon))
        return false;

    if (mLastPolyPolygonInfo.polygons.size() != 0
        && (mLastPolyPolygonInfo.transparency != fTransparency
            || !mLastPolyPolygonInfo.bounds.overlaps(aPolyPolygon.getB2DRange())))
    {
        checkPendingDrawing(); // Cannot be parts of the same larger polygon, draw the last and reset.
    }
    if (!mLastPolyPolygonInfo.polygons.empty())
    {
        assert(aPolyPolygon.count() == 1);
        assert(mLastPolyPolygonInfo.polygons.back().count() == 1);
        // Check if the new and the previous polygon share at least one point. If not, then they
        // cannot be adjacent polygons, so there's no point in trying to merge them.
        bool sharePoint = false;
        const basegfx::B2DPolygon& poly1 = aPolyPolygon.getB2DPolygon(0);
        const basegfx::B2DPolygon& poly2 = mLastPolyPolygonInfo.polygons.back().getB2DPolygon(0);
        o3tl::sorted_vector<basegfx::B2DPoint, LessThan> poly1Points; // for O(n log n)
        poly1Points.reserve(poly1.count());
        for (sal_uInt32 i = 0; i < poly1.count(); ++i)
            poly1Points.insert(poly1.getB2DPoint(i));
        for (sal_uInt32 i = 0; i < poly2.count(); ++i)
            if (poly1Points.find(poly2.getB2DPoint(i)) != poly1Points.end())
            {
                sharePoint = true;
                break;
            }
        if (!sharePoint)
            checkPendingDrawing(); // Draw the previous one and reset.
    }
    // Collect the polygons that can be possibly merged. Do the merging only once at the end,
    // because it's not a cheap operation.
    mLastPolyPolygonInfo.polygons.push_back(aPolyPolygon);
    mLastPolyPolygonInfo.bounds.expand(aPolyPolygon.getB2DRange());
    mLastPolyPolygonInfo.transparency = fTransparency;
    return true;
}

// Tdf#140848 - basegfx::utils::mergeToSinglePolyPolygon() seems to have rounding
// errors that sometimes cause it to merge incorrectly.
static void roundPolygonPoints(basegfx::B2DPolyPolygon& polyPolygon)
{
    for (basegfx::B2DPolygon& polygon : polyPolygon)
    {
        polygon.makeUnique();
        for (sal_uInt32 i = 0; i < polygon.count(); ++i)
            polygon.setB2DPoint(i, basegfx::B2DPoint(basegfx::fround(polygon.getB2DPoint(i))));
        // Control points are saved as vectors relative to points, so hopefully
        // there's no need to round those.
    }
}

void SkiaSalGraphicsImpl::checkPendingDrawing()
{
    if (mLastPolyPolygonInfo.polygons.size() != 0)
    { // Flush any pending polygon drawing.
        basegfx::B2DPolyPolygonVector polygons;
        std::swap(polygons, mLastPolyPolygonInfo.polygons);
        double transparency = mLastPolyPolygonInfo.transparency;
        mLastPolyPolygonInfo.bounds.reset();
        if (polygons.size() == 1)
            performDrawPolyPolygon(polygons.front(), transparency, true);
        else
        {
            for (basegfx::B2DPolyPolygon& p : polygons)
                roundPolygonPoints(p);
            performDrawPolyPolygon(basegfx::utils::mergeToSinglePolyPolygon(polygons), transparency,
                                   true);
        }
    }
}

bool SkiaSalGraphicsImpl::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                       const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                                       double fLineWidth, const std::vector<double>* pStroke,
                                       basegfx::B2DLineJoin eLineJoin,
                                       css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                                       bool bPixelSnapHairline)
{
    if (!rPolyLine.count() || fTransparency < 0.0 || fTransparency > 1.0
        || mLineColor == SALCOLOR_NONE)
    {
        return true;
    }

    preDraw();
    SAL_INFO("vcl.skia.trace", "drawpolyline(" << this << "): " << rPolyLine << ":" << mLineColor);

    // Adjust line width for object-to-device scale.
    fLineWidth = (rObjectToDevice * basegfx::B2DVector(fLineWidth, 0)).getLength();
    // On HiDPI displays, do not draw hairlines, draw 1-pixel wide lines in order to avoid
    // smoothing that would confuse unittests.
    if (fLineWidth == 0 && mScaling != 1 && isUnitTestRunning())
        fLineWidth = 1; // this will be scaled by mScaling

    // Transform to DeviceCoordinates, get DeviceLineWidth, execute PixelSnapHairline
    basegfx::B2DPolygon aPolyLine(rPolyLine);
    aPolyLine.transform(rObjectToDevice);
    if (bPixelSnapHairline)
    {
        aPolyLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyLine);
    }

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

    SkPaint aPaint = makeLinePaint(fTransparency);
    aPaint.setStrokeCap(eSkLineCap);
    aPaint.setStrokeJoin(eSkLineJoin);
    aPaint.setStrokeMiter(fMiterLimit);
    aPaint.setStrokeWidth(fLineWidth);
    aPaint.setAntiAlias(mParent.getAntiAlias());
    // See the tdf#134346 comment above.
    const SkScalar posFix = mParent.getAntiAlias() ? toSkXYFix : 0;

    if (pStroke && std::accumulate(pStroke->begin(), pStroke->end(), 0.0) != 0)
    {
        std::vector<SkScalar> intervals;
        // Transform size by the matrix.
        for (double stroke : *pStroke)
            intervals.push_back((rObjectToDevice * basegfx::B2DVector(stroke, 0)).getLength());
        aPaint.setPathEffect(SkDashPathEffect::Make(intervals.data(), intervals.size(), 0));
    }

    // Skia does not support basegfx::B2DLineJoin::NONE, so in that case batch only if lines
    // are not wider than a pixel.
    if (eLineJoin != basegfx::B2DLineJoin::NONE || fLineWidth <= 1.0)
    {
        SkPath aPath;
        aPath.incReserve(aPolyLine.count() * 3); // because cubicTo is 3 elements
        addPolygonToPath(aPolyLine, aPath);
        aPath.offset(toSkX(0) + posFix, toSkY(0) + posFix, nullptr);
        addUpdateRegion(aPath.getBounds());
        getDrawCanvas()->drawPath(aPath, aPaint);
    }
    else
    {
        sal_uInt32 nPoints = aPolyLine.count();
        bool bClosed = aPolyLine.isClosed();
        bool bHasCurves = aPolyLine.areControlPointsUsed();
        for (sal_uInt32 j = 0; j < nPoints; ++j)
        {
            SkPath aPath;
            aPath.incReserve(2 * 3); // because cubicTo is 3 elements
            addPolygonToPath(aPolyLine, aPath, j, j + 1, nPoints, bClosed, bHasCurves);
            aPath.offset(toSkX(0) + posFix, toSkY(0) + posFix, nullptr);
            addUpdateRegion(aPath.getBounds());
            getDrawCanvas()->drawPath(aPath, aPaint);
        }
    }

    postDraw();

    return true;
}

bool SkiaSalGraphicsImpl::drawPolyLineBezier(sal_uInt32, const Point*, const PolyFlags*)
{
    return false;
}

bool SkiaSalGraphicsImpl::drawPolygonBezier(sal_uInt32, const Point*, const PolyFlags*)
{
    return false;
}

bool SkiaSalGraphicsImpl::drawPolyPolygonBezier(sal_uInt32, const sal_uInt32*, const Point* const*,
                                                const PolyFlags* const*)
{
    return false;
}

void SkiaSalGraphicsImpl::copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX,
                                   tools::Long nSrcY, tools::Long nSrcWidth, tools::Long nSrcHeight,
                                   bool /*bWindowInvalidate*/)
{
    if (nDestX == nSrcX && nDestY == nSrcY)
        return;
    preDraw();
    SAL_INFO("vcl.skia.trace", "copyarea("
                                   << this << "): " << Point(nSrcX, nSrcY) << "->"
                                   << SkIRect::MakeXYWH(nDestX, nDestY, nSrcWidth, nSrcHeight));
    // Using SkSurface::draw() should be more efficient, but it's too buggy.
    SalTwoRect rPosAry(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);
    privateCopyBits(rPosAry, this);
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
        src->flushDrawing();
    }
    else
    {
        src = this;
        assert(mXorMode == XorMode::None);
    }
    auto srcDebug = [&]() -> std::string {
        if (src == this)
            return "(self)";
        else
        {
            std::ostringstream stream;
            stream << "(" << src << ")";
            return stream.str();
        }
    };
    SAL_INFO("vcl.skia.trace", "copybits(" << this << "): " << srcDebug() << ": " << rPosAry);
    privateCopyBits(rPosAry, src);
    postDraw();
}

void SkiaSalGraphicsImpl::privateCopyBits(const SalTwoRect& rPosAry, SkiaSalGraphicsImpl* src)
{
    assert(mXorMode == XorMode::None);
    addUpdateRegion(SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                                     rPosAry.mnDestHeight));
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
    SkIRect srcRect = SkIRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth,
                                        rPosAry.mnSrcHeight);
    SkRect destRect = SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                                       rPosAry.mnDestHeight);

    if (!SkIRect::Intersects(srcRect, SkIRect::MakeWH(src->GetWidth(), src->GetHeight()))
        || !SkRect::Intersects(destRect, SkRect::MakeWH(GetWidth(), GetHeight())))
        return;

    if (src == this)
    {
        // Copy-to-self means that we'd take a snapshot, which would refcount the data,
        // and then drawing would result in copy in write, copying the entire surface.
        // Try to copy less by making a snapshot of only what is needed.
        // A complication here is that drawImageRect() can handle coordinates outside
        // of surface fine, but makeImageSnapshot() will crop to the surface area,
        // so do that manually here in order to adjust also destination rectangle.
        if (srcRect.x() < 0 || srcRect.y() < 0)
        {
            destRect.fLeft += -srcRect.x();
            destRect.fTop += -srcRect.y();
            srcRect.adjust(-srcRect.x(), -srcRect.y(), 0, 0);
        }
        // Note that right() and bottom() are not inclusive (are outside of the rect).
        if (srcRect.right() - 1 > GetWidth() || srcRect.bottom() - 1 > GetHeight())
        {
            destRect.fRight += GetWidth() - srcRect.right();
            destRect.fBottom += GetHeight() - srcRect.bottom();
            srcRect.adjust(0, 0, GetWidth() - srcRect.right(), GetHeight() - srcRect.bottom());
        }
        // Scaling for source coordinates must be done manually.
        if (src->mScaling != 1)
            srcRect = scaleRect(srcRect, src->mScaling);
        sk_sp<SkImage> image = makeCheckedImageSnapshot(src->mSurface, srcRect);
        srcRect.offset(-srcRect.x(), -srcRect.y());
        getDrawCanvas()->drawImageRect(image, SkRect::Make(srcRect), destRect,
                                       makeSamplingOptions(rPosAry, mScaling, src->mScaling),
                                       &paint, SkCanvas::kFast_SrcRectConstraint);
    }
    else
    {
        // Scaling for source coordinates must be done manually.
        if (src->mScaling != 1)
            srcRect = scaleRect(srcRect, src->mScaling);
        // Do not use makeImageSnapshot(rect), as that one may make a needless data copy.
        getDrawCanvas()->drawImageRect(makeCheckedImageSnapshot(src->mSurface),
                                       SkRect::Make(srcRect), destRect,
                                       makeSamplingOptions(rPosAry, mScaling, src->mScaling),
                                       &paint, SkCanvas::kFast_SrcRectConstraint);
    }
}

bool SkiaSalGraphicsImpl::blendBitmap(const SalTwoRect& rPosAry, const SalBitmap& rBitmap)
{
    if (checkInvalidSourceOrDestination(rPosAry))
        return false;

    assert(dynamic_cast<const SkiaSalBitmap*>(&rBitmap));
    const SkiaSalBitmap& rSkiaBitmap = static_cast<const SkiaSalBitmap&>(rBitmap);
    // This is used by VirtualDevice in the alpha mode for the "alpha" layer which
    // is actually one-minus-alpha (opacity). Therefore white=0xff=transparent,
    // black=0x00=opaque. So the result is transparent only if both the inputs
    // are transparent. Since for blending operations white=1.0 and black=0.0,
    // kMultiply should handle exactly that (transparent*transparent=transparent,
    // opaque*transparent=opaque). And guessing from the "floor" in TYPE_BLEND in opengl's
    // combinedTextureFragmentShader.glsl, the layer is not even alpha values but
    // simply yes-or-no mask.
    // See also blendAlphaBitmap().
    if (rSkiaBitmap.IsFullyOpaqueAsAlpha())
    {
        // Optimization. If the bitmap means fully opaque, it's all zero's. In CPU
        // mode it should be faster to just copy instead of SkBlendMode::kMultiply.
        drawBitmap(rPosAry, rSkiaBitmap);
    }
    else
        drawBitmap(rPosAry, rSkiaBitmap, SkBlendMode::kMultiply);
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
    const SkiaSalBitmap& rSkiaSourceBitmap = static_cast<const SkiaSalBitmap&>(rSourceBitmap);
    const SkiaSalBitmap& rSkiaMaskBitmap = static_cast<const SkiaSalBitmap&>(rMaskBitmap);
    const SkiaSalBitmap& rSkiaAlphaBitmap = static_cast<const SkiaSalBitmap&>(rAlphaBitmap);

    if (rSkiaMaskBitmap.IsFullyOpaqueAsAlpha())
    {
        // Optimization. If the mask of the bitmap to be blended means it's actually opaque,
        // just draw the bitmap directly (that's what the math below will result in).
        drawBitmap(rPosAry, rSkiaSourceBitmap);
        return true;
    }
    // This was originally implemented for the OpenGL drawing method and it is poorly documented.
    // The source and mask bitmaps are the usual data and alpha bitmaps, and 'alpha'
    // is the "alpha" layer of the VirtualDevice (the alpha in VirtualDevice is also stored
    // as a separate bitmap). Now if I understand it correctly these two alpha masks first need
    // to be combined into the actual alpha mask to be used. The formula for TYPE_BLEND
    // in opengl's combinedTextureFragmentShader.glsl is
    // "result_alpha = 1.0 - (1.0 - floor(alpha)) * mask".
    // See also blendBitmap().

    SkSamplingOptions samplingOptions = makeSamplingOptions(rPosAry, mScaling);
    // First do the "( 1 - alpha ) * mask"
    // (no idea how to do "floor", but hopefully not needed in practice).
    sk_sp<SkShader> shaderAlpha
        = SkShaders::Blend(SkBlendMode::kDstOut, rSkiaMaskBitmap.GetAlphaSkShader(samplingOptions),
                           rSkiaAlphaBitmap.GetAlphaSkShader(samplingOptions));
    // And now draw the bitmap with "1 - x", where x is the "( 1 - alpha ) * mask".
    sk_sp<SkShader> shader = SkShaders::Blend(SkBlendMode::kSrcOut, shaderAlpha,
                                              rSkiaSourceBitmap.GetSkShader(samplingOptions));
    drawShader(rPosAry, shader);
    return true;
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (checkInvalidSourceOrDestination(rPosAry))
        return;

    assert(dynamic_cast<const SkiaSalBitmap*>(&rSalBitmap));
    const SkiaSalBitmap& rSkiaSourceBitmap = static_cast<const SkiaSalBitmap&>(rSalBitmap);

    drawBitmap(rPosAry, rSkiaSourceBitmap);
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
    const SkiaSalBitmap& skiaBitmap = static_cast<const SkiaSalBitmap&>(rSalBitmap);
    drawShader(
        rPosAry,
        SkShaders::Blend(SkBlendMode::kDstOut, // VCL alpha is one-minus-alpha.
                         SkShaders::Color(toSkColor(nMaskColor)),
                         skiaBitmap.GetAlphaSkShader(makeSamplingOptions(rPosAry, mScaling))));
}

std::shared_ptr<SalBitmap> SkiaSalGraphicsImpl::getBitmap(tools::Long nX, tools::Long nY,
                                                          tools::Long nWidth, tools::Long nHeight)
{
    SkiaZone zone;
    checkSurface();
    SAL_INFO("vcl.skia.trace",
             "getbitmap(" << this << "): " << SkIRect::MakeXYWH(nX, nY, nWidth, nHeight));
    flushDrawing();
    // TODO makeImageSnapshot(rect) may copy the data, which may be a waste if this is used
    // e.g. for VirtualDevice's lame alpha blending, in which case the image will eventually end up
    // in blendAlphaBitmap(), where we could simply use the proper rect of the image.
    sk_sp<SkImage> image = makeCheckedImageSnapshot(
        mSurface, scaleRect(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight), mScaling));
    std::shared_ptr<SkiaSalBitmap> bitmap = std::make_shared<SkiaSalBitmap>(image);
    // If the surface is scaled for HiDPI, the bitmap needs to be scaled down, otherwise
    // it would have incorrect size from the API point of view. The DirectImage::Yes handling
    // in mergeCacheBitmaps() should access the original unscaled bitmap data to avoid
    // pointless scaling back and forth.
    if (mScaling != 1)
    {
        if (!isUnitTestRunning())
            bitmap->Scale(1.0 / mScaling, 1.0 / mScaling, goodScalingQuality());
        else
        {
            // Some tests require exact pixel values and would be confused by smooth-scaling.
            // And some draw something smooth and not smooth-scaling there would break the checks.
            if (isUnitTestRunning("BackendTest__testDrawHaflEllipseAAWithPolyLineB2D_")
                || isUnitTestRunning("BackendTest__testDrawRectAAWithLine_"))
            {
                bitmap->Scale(1.0 / mScaling, 1.0 / mScaling, goodScalingQuality());
            }
            else
                bitmap->Scale(1.0 / mScaling, 1.0 / mScaling, BmpScaleFlag::NearestNeighbor);
        }
    }
    return bitmap;
}

Color SkiaSalGraphicsImpl::getPixel(tools::Long nX, tools::Long nY)
{
    SkiaZone zone;
    checkSurface();
    SAL_INFO("vcl.skia.trace", "getpixel(" << this << "): " << Point(nX, nY));
    flushDrawing();
    // This is presumably slow, but getPixel() should be generally used only by unit tests.
    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(mSurface->width(), mSurface->height()))
        abort();
    if (!mSurface->readPixels(bitmap, 0, 0))
        abort();
    return fromSkColor(bitmap.getColor(nX * mScaling, nY * mScaling));
}

void SkiaSalGraphicsImpl::invert(basegfx::B2DPolygon const& rPoly, SalInvert eFlags)
{
    preDraw();
    SAL_INFO("vcl.skia.trace", "invert(" << this << "): " << rPoly << ":" << int(eFlags));
    assert(mXorMode == XorMode::None);
    SkPath aPath;
    aPath.incReserve(rPoly.count());
    addPolygonToPath(rPoly, aPath);
    aPath.setFillType(SkPathFillType::kEvenOdd);
    addUpdateRegion(aPath.getBounds());
    SkAutoCanvasRestore autoRestore(getDrawCanvas(), true);
    SkPaint aPaint;
    // There's no blend mode for inverting as such, but kExclusion is 's + d - 2*s*d',
    // so with d = 1.0 (all channels) it becomes effectively '1 - s', i.e. inverted color.
    aPaint.setBlendMode(SkBlendMode::kExclusion);
    aPaint.setColor(SkColorSetARGB(255, 255, 255, 255));
    // TrackFrame just inverts a dashed path around the polygon
    if (eFlags == SalInvert::TrackFrame)
    {
        // TrackFrame is not supposed to paint outside of the polygon (usually rectangle),
        // but wider stroke width usually results in that, so ensure the requirement
        // by clipping.
        getDrawCanvas()->clipRect(aPath.getBounds(), SkClipOp::kIntersect, false);
        aPaint.setStrokeWidth(2);
        constexpr float intervals[] = { 4.0f, 4.0f };
        aPaint.setStyle(SkPaint::kStroke_Style);
        aPaint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
    }
    else
    {
        aPaint.setStyle(SkPaint::kFill_Style);

        // N50 inverts in checker pattern
        if (eFlags == SalInvert::N50)
        {
            // This creates 2x2 checker pattern bitmap
            // TODO Use createSkSurface() and cache the image
            SkBitmap aBitmap;
            aBitmap.allocN32Pixels(2, 2);
            const SkPMColor white = SkPreMultiplyARGB(0xFF, 0xFF, 0xFF, 0xFF);
            const SkPMColor black = SkPreMultiplyARGB(0xFF, 0x00, 0x00, 0x00);
            SkPMColor* scanline;
            scanline = aBitmap.getAddr32(0, 0);
            *scanline++ = white;
            *scanline++ = black;
            scanline = aBitmap.getAddr32(0, 1);
            *scanline++ = black;
            *scanline++ = white;
            aBitmap.setImmutable();
            // The bitmap is repeated in both directions the checker pattern is as big
            // as the polygon (usually rectangle)
            aPaint.setShader(
                aBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions()));
        }
    }
    getDrawCanvas()->drawPath(aPath, aPaint);
    postDraw();
}

void SkiaSalGraphicsImpl::invert(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                 tools::Long nHeight, SalInvert eFlags)
{
    basegfx::B2DRectangle aRectangle(nX, nY, nX + nWidth, nY + nHeight);
    auto aRect = basegfx::utils::createPolygonFromRect(aRectangle);
    invert(aRect, eFlags);
}

void SkiaSalGraphicsImpl::invert(sal_uInt32 nPoints, const Point* pPointArray, SalInvert eFlags)
{
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(pPointArray[0].getX(), pPointArray[0].getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
    {
        aPolygon.setB2DPoint(i, basegfx::B2DPoint(pPointArray[i].getX(), pPointArray[i].getY()));
    }
    aPolygon.setClosed(true);

    invert(aPolygon, eFlags);
}

bool SkiaSalGraphicsImpl::drawEPS(tools::Long, tools::Long, tools::Long, tools::Long, void*,
                                  sal_uInt32)
{
    return false;
}

// Create SkImage from a bitmap and possibly an alpha mask (the usual VCL one-minus-alpha),
// with the given target size. Result will be possibly cached, unless disabled.
// Especially in raster mode scaling and alpha blending may be expensive if done repeatedly.
sk_sp<SkImage> SkiaSalGraphicsImpl::mergeCacheBitmaps(const SkiaSalBitmap& bitmap,
                                                      const SkiaSalBitmap* alphaBitmap,
                                                      const Size& targetSize)
{
    if (alphaBitmap)
        assert(bitmap.GetSize() == alphaBitmap->GetSize());

    if (targetSize.IsEmpty())
        return {};
    if (alphaBitmap && alphaBitmap->IsFullyOpaqueAsAlpha())
        alphaBitmap = nullptr; // the alpha can be ignored
    if (bitmap.PreferSkShader() && (!alphaBitmap || alphaBitmap->PreferSkShader()))
        return {};

    // If the bitmap has SkImage that matches the required size, try to use it, even
    // if it doesn't match bitmap.GetSize(). This can happen with delayed scaling.
    // This will catch cases such as some code pre-scaling the bitmap, which would make GetSkImage()
    // scale, changing GetImageKey() in the process so we'd have to re-cache, and then we'd need
    // to scale again in this function.
    bool bitmapReady = false;
    bool alphaBitmapReady = false;
    if (const sk_sp<SkImage>& image = bitmap.GetSkImage(DirectImage::Yes))
    {
        assert(!bitmap.PreferSkShader());
        if (imageSize(image) == targetSize)
            bitmapReady = true;
    }
    // If the image usable and there's no alpha, then it matches exactly what's wanted.
    if (bitmapReady && !alphaBitmap)
        return bitmap.GetSkImage(DirectImage::Yes);
    if (alphaBitmap)
    {
        if (!alphaBitmap->GetAlphaSkImage(DirectImage::Yes)
            && alphaBitmap->GetSkImage(DirectImage::Yes)
            && imageSize(alphaBitmap->GetSkImage(DirectImage::Yes)) == targetSize)
        {
            // There's a usable non-alpha image, try to convert it to alpha.
            assert(!alphaBitmap->PreferSkShader());
            const_cast<SkiaSalBitmap*>(alphaBitmap)->TryDirectConvertToAlphaNoScaling();
        }
        if (const sk_sp<SkImage>& image = alphaBitmap->GetAlphaSkImage(DirectImage::Yes))
        {
            assert(!alphaBitmap->PreferSkShader());
            if (imageSize(image) == targetSize)
                alphaBitmapReady = true;
        }
    }

    if (bitmapReady && (!alphaBitmap || alphaBitmapReady))
    {
        // Try to find a cached image based on the already existing images.
        OString key = makeCachedImageKey(bitmap, alphaBitmap, targetSize, DirectImage::Yes,
                                         DirectImage::Yes);
        if (sk_sp<SkImage> image = findCachedImage(key))
        {
            assert(imageSize(image) == targetSize);
            return image;
        }
    }

    // Probably not much point in caching of just doing a copy.
    if (alphaBitmap == nullptr && targetSize == bitmap.GetSize())
        return {};
    // Image too small to be worth caching if not scaling.
    if (targetSize == bitmap.GetSize() && targetSize.Width() < 100 && targetSize.Height() < 100)
        return {};
    // GPU-accelerated drawing with SkShader should be fast enough to not need caching.
    if (isGPU())
    {
        // tdf#140925: But if this is such an extensive downscaling that caching the result
        // would noticeably reduce amount of data processed by the GPU on repeated usage, do it.
        int reduceRatio = bitmap.GetSize().Width() * bitmap.GetSize().Height() / targetSize.Width()
                          / targetSize.Height();
        if (reduceRatio < 10)
            return {};
    }
    // Do not cache the result if it would take most of the cache and thus get evicted soon.
    if (targetSize.Width() * targetSize.Height() * 4 > maxImageCacheSize() * 0.7)
        return {};

    // Use ready direct image if they are both available, now even the size doesn't matter
    // (we'll scale as necessary and it's better to scale from the original). Require only
    // that they are the same size, or that one prefers a shader or doesn't exist
    // (i.e. avoid two images of different size).
    bitmapReady = bitmap.GetSkImage(DirectImage::Yes) != nullptr;
    alphaBitmapReady = alphaBitmap && alphaBitmap->GetAlphaSkImage(DirectImage::Yes) != nullptr;
    if (bitmapReady && alphaBitmap && !alphaBitmapReady && !alphaBitmap->PreferSkShader())
        bitmapReady = false;
    if (alphaBitmapReady && !bitmapReady && bitmap.PreferSkShader())
        alphaBitmapReady = false;

    DirectImage bitmapType = bitmapReady ? DirectImage::Yes : DirectImage::No;
    DirectImage alphaBitmapType = alphaBitmapReady ? DirectImage::Yes : DirectImage::No;

    // Try to find a cached result, this time after possible delayed scaling.
    OString key = makeCachedImageKey(bitmap, alphaBitmap, targetSize, bitmapType, alphaBitmapType);
    if (sk_sp<SkImage> image = findCachedImage(key))
    {
        assert(imageSize(image) == targetSize);
        return image;
    }

    // In some cases (tdf#134237) the target size may be very large. In that case it's
    // better to rely on Skia to clip and draw only the necessary, rather than prepare
    // a very large image only to not use most of it. Do this only after checking whether
    // the image is already cached, since it might have been already cached in a previous
    // call that had the draw area large enough to be seen as worth caching.
    const Size drawAreaSize = mClipRegion.GetBoundRect().GetSize() * mScaling;
    if (targetSize.Width() > drawAreaSize.Width() || targetSize.Height() > drawAreaSize.Height())
    {
        // This is a bit tricky. The condition above just checks that at least a part of the resulting
        // image will not be used (it's larger then our drawing area). But this may often happen
        // when just scrolling a document with a large image, where the caching may very well be worth it.
        // Since the problem is mainly the cost of upscaling and then the size of the resulting bitmap,
        // compute a ratio of how much this is going to be scaled up, how much this is larger than
        // the drawing area, and then refuse to cache if it's too much.
        const double upscaleRatio
            = std::max(1.0, 1.0 * targetSize.Width() / bitmap.GetSize().Width()
                                * targetSize.Height() / bitmap.GetSize().Height());
        const double oversizeRatio = 1.0 * targetSize.Width() / drawAreaSize.Width()
                                     * targetSize.Height() / drawAreaSize.Height();
        const double ratio = upscaleRatio * oversizeRatio;
        if (ratio > 4)
        {
            SAL_INFO("vcl.skia.trace", "mergecachebitmaps("
                                           << this << "): not caching, ratio:" << ratio << ", "
                                           << bitmap.GetSize() << "->" << targetSize << " in "
                                           << drawAreaSize);
            return {};
        }
    }

    Size sourceSize;
    if (bitmapReady)
        sourceSize = imageSize(bitmap.GetSkImage(DirectImage::Yes));
    else if (alphaBitmapReady)
        sourceSize = imageSize(alphaBitmap->GetAlphaSkImage(DirectImage::Yes));
    else
        sourceSize = bitmap.GetSize();

    // Generate a new result and cache it.
    sk_sp<SkSurface> tmpSurface
        = createSkSurface(targetSize, alphaBitmap ? kPremul_SkAlphaType : bitmap.alphaType());
    if (!tmpSurface)
        return nullptr;
    SkCanvas* canvas = tmpSurface->getCanvas();
    SkAutoCanvasRestore autoRestore(canvas, true);
    SkPaint paint;
    SkSamplingOptions samplingOptions;
    if (targetSize != sourceSize)
    {
        SkMatrix matrix;
        matrix.set(SkMatrix::kMScaleX, 1.0 * targetSize.Width() / sourceSize.Width());
        matrix.set(SkMatrix::kMScaleY, 1.0 * targetSize.Height() / sourceSize.Height());
        canvas->concat(matrix);
        if (!isUnitTestRunning()) // unittests want exact pixel values
            samplingOptions = makeSamplingOptions(matrix, 1);
    }
    if (alphaBitmap != nullptr)
    {
        canvas->clear(SK_ColorTRANSPARENT);
        paint.setShader(
            SkShaders::Blend(SkBlendMode::kDstOut, bitmap.GetSkShader(samplingOptions, bitmapType),
                             alphaBitmap->GetAlphaSkShader(samplingOptions, alphaBitmapType)));
        canvas->drawPaint(paint);
    }
    else if (bitmap.PreferSkShader())
    {
        paint.setShader(bitmap.GetSkShader(samplingOptions, bitmapType));
        canvas->drawPaint(paint);
    }
    else
        canvas->drawImage(bitmap.GetSkImage(bitmapType), 0, 0, samplingOptions, &paint);
    if (isGPU())
        SAL_INFO("vcl.skia.trace", "mergecachebitmaps(" << this << "): caching GPU downscaling:"
                                                        << bitmap.GetSize() << "->" << targetSize);
    sk_sp<SkImage> image = makeCheckedImageSnapshot(tmpSurface);
    addCachedImage(key, image);
    return image;
}

OString SkiaSalGraphicsImpl::makeCachedImageKey(const SkiaSalBitmap& bitmap,
                                                const SkiaSalBitmap* alphaBitmap,
                                                const Size& targetSize, DirectImage bitmapType,
                                                DirectImage alphaBitmapType)
{
    OString key = OString::number(targetSize.Width()) + "x" + OString::number(targetSize.Height())
                  + "_" + bitmap.GetImageKey(bitmapType);
    if (alphaBitmap)
        key += "_" + alphaBitmap->GetAlphaImageKey(alphaBitmapType);
    return key;
}

bool SkiaSalGraphicsImpl::drawAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                          const SalBitmap& rAlphaBitmap)
{
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSourceBitmap));
    assert(dynamic_cast<const SkiaSalBitmap*>(&rAlphaBitmap));
    const SkiaSalBitmap& rSkiaSourceBitmap = static_cast<const SkiaSalBitmap&>(rSourceBitmap);
    const SkiaSalBitmap& rSkiaAlphaBitmap = static_cast<const SkiaSalBitmap&>(rAlphaBitmap);
    // Use mergeCacheBitmaps(), which may decide to cache the result, avoiding repeated
    // alpha blending or scaling.
    SalTwoRect imagePosAry(rPosAry);
    Size imageSize = rSourceBitmap.GetSize();
    // If the bitmap will be scaled, prefer to do it in mergeCacheBitmaps(), if possible.
    if ((rPosAry.mnSrcWidth != rPosAry.mnDestWidth || rPosAry.mnSrcHeight != rPosAry.mnDestHeight)
        && rPosAry.mnSrcX == 0 && rPosAry.mnSrcY == 0
        && rPosAry.mnSrcWidth == rSourceBitmap.GetSize().Width()
        && rPosAry.mnSrcHeight == rSourceBitmap.GetSize().Height())
    {
        imagePosAry.mnSrcWidth = imagePosAry.mnDestWidth;
        imagePosAry.mnSrcHeight = imagePosAry.mnDestHeight;
        imageSize = Size(imagePosAry.mnSrcWidth, imagePosAry.mnSrcHeight);
    }
    sk_sp<SkImage> image
        = mergeCacheBitmaps(rSkiaSourceBitmap, &rSkiaAlphaBitmap, imageSize * mScaling);
    if (image)
        drawImage(imagePosAry, image, mScaling);
    else if (rSkiaAlphaBitmap.IsFullyOpaqueAsAlpha()
             && !rSkiaSourceBitmap.PreferSkShader()) // alpha can be ignored
        drawBitmap(rPosAry, rSkiaSourceBitmap);
    else
        drawShader(rPosAry,
                   SkShaders::Blend(
                       SkBlendMode::kDstOut, // VCL alpha is one-minus-alpha.
                       rSkiaSourceBitmap.GetSkShader(makeSamplingOptions(rPosAry, mScaling)),
                       rSkiaAlphaBitmap.GetAlphaSkShader(makeSamplingOptions(rPosAry, mScaling))));
    return true;
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SkiaSalBitmap& bitmap,
                                     SkBlendMode blendMode)
{
    // Use mergeCacheBitmaps(), which may decide to cache the result, avoiding repeated
    // scaling.
    SalTwoRect imagePosAry(rPosAry);
    Size imageSize = bitmap.GetSize();
    // If the bitmap will be scaled, prefer to do it in mergeCacheBitmaps(), if possible.
    if ((rPosAry.mnSrcWidth != rPosAry.mnDestWidth || rPosAry.mnSrcHeight != rPosAry.mnDestHeight)
        && rPosAry.mnSrcX == 0 && rPosAry.mnSrcY == 0
        && rPosAry.mnSrcWidth == bitmap.GetSize().Width()
        && rPosAry.mnSrcHeight == bitmap.GetSize().Height())
    {
        imagePosAry.mnSrcWidth = imagePosAry.mnDestWidth;
        imagePosAry.mnSrcHeight = imagePosAry.mnDestHeight;
        imageSize = Size(imagePosAry.mnSrcWidth, imagePosAry.mnSrcHeight);
    }
    sk_sp<SkImage> image = mergeCacheBitmaps(bitmap, nullptr, imageSize * mScaling);
    if (image)
        drawImage(imagePosAry, image, mScaling, blendMode);
    else if (bitmap.PreferSkShader())
        drawShader(rPosAry, bitmap.GetSkShader(makeSamplingOptions(rPosAry, mScaling)), blendMode);
    else
        drawImage(rPosAry, bitmap.GetSkImage(), 1, blendMode);
}

void SkiaSalGraphicsImpl::drawImage(const SalTwoRect& rPosAry, const sk_sp<SkImage>& aImage,
                                    int srcScaling, SkBlendMode eBlendMode)
{
    SkRect aSourceRect
        = SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    if (srcScaling != 1)
        aSourceRect = scaleRect(aSourceRect, srcScaling);
    SkRect aDestinationRect = SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY,
                                               rPosAry.mnDestWidth, rPosAry.mnDestHeight);

    SkPaint aPaint = makeBitmapPaint();
    aPaint.setBlendMode(eBlendMode);

    preDraw();
    SAL_INFO("vcl.skia.trace",
             "drawimage(" << this << "): " << rPosAry << ":" << SkBlendMode_Name(eBlendMode));
    addUpdateRegion(aDestinationRect);
    getDrawCanvas()->drawImageRect(aImage, aSourceRect, aDestinationRect,
                                   makeSamplingOptions(rPosAry, mScaling, srcScaling), &aPaint,
                                   SkCanvas::kFast_SrcRectConstraint);
    ++mPendingOperationsToFlush; // tdf#136369
    postDraw();
}

// SkShader can be used to merge multiple bitmaps with appropriate blend modes (e.g. when
// merging a bitmap with its alpha mask).
void SkiaSalGraphicsImpl::drawShader(const SalTwoRect& rPosAry, const sk_sp<SkShader>& shader,
                                     SkBlendMode blendMode)
{
    preDraw();
    SAL_INFO("vcl.skia.trace", "drawshader(" << this << "): " << rPosAry);
    SkRect destinationRect = SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth,
                                              rPosAry.mnDestHeight);
    addUpdateRegion(destinationRect);
    SkPaint paint = makeBitmapPaint();
    paint.setBlendMode(blendMode);
    paint.setShader(shader);
    SkCanvas* canvas = getDrawCanvas();
    // Scaling needs to be done explicitly using a matrix.
    SkAutoCanvasRestore autoRestore(canvas, true);
    SkMatrix matrix = SkMatrix::Translate(rPosAry.mnDestX, rPosAry.mnDestY)
                      * SkMatrix::Scale(1.0 * rPosAry.mnDestWidth / rPosAry.mnSrcWidth,
                                        1.0 * rPosAry.mnDestHeight / rPosAry.mnSrcHeight)
                      * SkMatrix::Translate(-rPosAry.mnSrcX, -rPosAry.mnSrcY);
#ifndef NDEBUG
    // Handle floating point imprecisions, round p1 to 2 decimal places.
    auto compareRounded = [](const SkPoint& p1, const SkPoint& p2) {
        return rtl::math::round(p1.x(), 2) == p2.x() && rtl::math::round(p1.y(), 2) == p2.y();
    };
#endif
    assert(compareRounded(matrix.mapXY(rPosAry.mnSrcX, rPosAry.mnSrcY),
                          SkPoint::Make(rPosAry.mnDestX, rPosAry.mnDestY)));
    assert(compareRounded(
        matrix.mapXY(rPosAry.mnSrcX + rPosAry.mnSrcWidth, rPosAry.mnSrcY + rPosAry.mnSrcHeight),
        SkPoint::Make(rPosAry.mnDestX + rPosAry.mnDestWidth,
                      rPosAry.mnDestY + rPosAry.mnDestHeight)));
    canvas->concat(matrix);
    SkRect sourceRect
        = SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    canvas->drawRect(sourceRect, paint);
    postDraw();
}

bool SkiaSalGraphicsImpl::hasFastDrawTransformedBitmap() const
{
    // Return true even in raster mode, even that way Skia is faster than e.g. GraphicObject
    // trying to handle stuff manually.
    return true;
}

// Whether applying matrix needs image smoothing for the transformation.
static bool matrixNeedsHighQuality(const SkMatrix& matrix)
{
    if (matrix.isIdentity())
        return false;
    if (matrix.isScaleTranslate())
    {
        if (abs(matrix.getScaleX()) == 1 && abs(matrix.getScaleY()) == 1)
            return false; // Only at most flipping and keeping the size.
        return true;
    }
    assert(!matrix.hasPerspective()); // we do not use this
    if (matrix.getScaleX() == 0 && matrix.getScaleY() == 0)
    {
        // Rotating 90 or 270 degrees while keeping the size.
        if ((matrix.getSkewX() == 1 && matrix.getSkewY() == -1)
            || (matrix.getSkewX() == -1 && matrix.getSkewY() == 1))
            return false;
    }
    return true;
}

namespace SkiaTests
{
bool matrixNeedsHighQuality(const SkMatrix& matrix) { return ::matrixNeedsHighQuality(matrix); }
}

bool SkiaSalGraphicsImpl::drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                                const basegfx::B2DPoint& rX,
                                                const basegfx::B2DPoint& rY,
                                                const SalBitmap& rSourceBitmap,
                                                const SalBitmap* pAlphaBitmap, double fAlpha)
{
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSourceBitmap));
    assert(!pAlphaBitmap || dynamic_cast<const SkiaSalBitmap*>(pAlphaBitmap));

    const SkiaSalBitmap& rSkiaBitmap = static_cast<const SkiaSalBitmap&>(rSourceBitmap);
    const SkiaSalBitmap* pSkiaAlphaBitmap = static_cast<const SkiaSalBitmap*>(pAlphaBitmap);

    if (pSkiaAlphaBitmap && pSkiaAlphaBitmap->IsFullyOpaqueAsAlpha())
        pSkiaAlphaBitmap = nullptr; // the alpha can be ignored

    // Setup the image transformation,
    // using the rNull, rX, rY points as destinations for the (0,0), (Width,0), (0,Height) source points.
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;

    preDraw();
    SAL_INFO("vcl.skia.trace", "drawtransformedbitmap(" << this << "): " << rSourceBitmap.GetSize()
                                                        << " " << rNull << ":" << rX << ":" << rY);

    addUpdateRegion(SkRect::MakeWH(GetWidth(), GetHeight())); // can't tell, use whole area
    // Use mergeCacheBitmaps(), which may decide to cache the result, avoiding repeated
    // alpha blending or scaling.
    // The extra fAlpha blending is not cached, with the assumption that it usually gradually changes
    // for each invocation.
    // Pass size * mScaling to mergeCacheBitmaps() so that it prepares the size that will be needed
    // after the mScaling-scaling matrix, but otherwise calculate everything else using the VCL coordinates.
    Size imageSize(round(aXRel.getLength()), round(aYRel.getLength()));
    sk_sp<SkImage> imageToDraw
        = mergeCacheBitmaps(rSkiaBitmap, pSkiaAlphaBitmap, imageSize * mScaling);
    if (imageToDraw)
    {
        SkMatrix matrix;
        // Round sizes for scaling, so that sub-pixel differences don't
        // trigger unnecessary scaling. Image has already been scaled
        // by mergeCacheBitmaps() and we shouldn't scale here again
        // unless the drawing is also skewed.
        matrix.set(SkMatrix::kMScaleX, round(aXRel.getX()) / imageSize.Width());
        matrix.set(SkMatrix::kMScaleY, round(aYRel.getY()) / imageSize.Height());
        matrix.set(SkMatrix::kMSkewY, aXRel.getY() / imageSize.Width());
        matrix.set(SkMatrix::kMSkewX, aYRel.getX() / imageSize.Height());
        matrix.set(SkMatrix::kMTransX, rNull.getX());
        matrix.set(SkMatrix::kMTransY, rNull.getY());
        SkCanvas* canvas = getDrawCanvas();
        SkAutoCanvasRestore autoRestore(canvas, true);
        canvas->concat(matrix);
        SkSamplingOptions samplingOptions;
        // If the matrix changes geometry, we need to smooth-scale. If there's mScaling,
        // that's already been handled by mergeCacheBitmaps().
        if (matrixNeedsHighQuality(matrix))
            samplingOptions = makeSamplingOptions(matrix, 1);
        if (fAlpha == 1.0)
        {
            // Specify sizes to scale the image size back if needed (because of mScaling).
            SkRect dstRect = SkRect::MakeWH(imageSize.Width(), imageSize.Height());
            SkRect srcRect = SkRect::MakeWH(imageToDraw->width(), imageToDraw->height());
            SkPaint paint = makeBitmapPaint();
            canvas->drawImageRect(imageToDraw, srcRect, dstRect, samplingOptions, &paint,
                                  SkCanvas::kFast_SrcRectConstraint);
        }
        else
        {
            SkPaint paint = makeBitmapPaint();
            // Scale the image size back if needed.
            SkMatrix scale = SkMatrix::Scale(1.0 / mScaling, 1.0 / mScaling);
            paint.setShader(SkShaders::Blend(
                SkBlendMode::kDstIn, imageToDraw->makeShader(samplingOptions, &scale),
                SkShaders::Color(SkColorSetARGB(fAlpha * 255, 0, 0, 0))));
            canvas->drawRect(SkRect::MakeWH(imageSize.Width(), imageSize.Height()), paint);
        }
    }
    else
    {
        SkMatrix matrix;
        const Size aSize = rSourceBitmap.GetSize();
        matrix.set(SkMatrix::kMScaleX, aXRel.getX() / aSize.Width());
        matrix.set(SkMatrix::kMScaleY, aYRel.getY() / aSize.Height());
        matrix.set(SkMatrix::kMSkewY, aXRel.getY() / aSize.Width());
        matrix.set(SkMatrix::kMSkewX, aYRel.getX() / aSize.Height());
        matrix.set(SkMatrix::kMTransX, rNull.getX());
        matrix.set(SkMatrix::kMTransY, rNull.getY());
        SkCanvas* canvas = getDrawCanvas();
        SkAutoCanvasRestore autoRestore(canvas, true);
        canvas->concat(matrix);
        SkSamplingOptions samplingOptions;
        if (matrixNeedsHighQuality(matrix) || (mScaling != 1 && !isUnitTestRunning()))
            samplingOptions = makeSamplingOptions(matrix, mScaling);
        if (pSkiaAlphaBitmap)
        {
            SkPaint paint = makeBitmapPaint();
            paint.setShader(SkShaders::Blend(SkBlendMode::kDstOut, // VCL alpha is one-minus-alpha.
                                             rSkiaBitmap.GetSkShader(samplingOptions),
                                             pSkiaAlphaBitmap->GetAlphaSkShader(samplingOptions)));
            if (fAlpha != 1.0)
                paint.setShader(
                    SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(),
                                     SkShaders::Color(SkColorSetARGB(fAlpha * 255, 0, 0, 0))));
            canvas->drawRect(SkRect::MakeWH(aSize.Width(), aSize.Height()), paint);
        }
        else if (rSkiaBitmap.PreferSkShader() || fAlpha != 1.0)
        {
            SkPaint paint = makeBitmapPaint();
            paint.setShader(rSkiaBitmap.GetSkShader(samplingOptions));
            if (fAlpha != 1.0)
                paint.setShader(
                    SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(),
                                     SkShaders::Color(SkColorSetARGB(fAlpha * 255, 0, 0, 0))));
            canvas->drawRect(SkRect::MakeWH(aSize.Width(), aSize.Height()), paint);
        }
        else
        {
            SkPaint paint = makeBitmapPaint();
            canvas->drawImage(rSkiaBitmap.GetSkImage(), 0, 0, samplingOptions, &paint);
        }
    }
    postDraw();
    return true;
}

bool SkiaSalGraphicsImpl::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                        tools::Long nHeight, sal_uInt8 nTransparency)
{
    privateDrawAlphaRect(nX, nY, nWidth, nHeight, nTransparency / 100.0);
    return true;
}

bool SkiaSalGraphicsImpl::drawGradient(const tools::PolyPolygon& rPolyPolygon,
                                       const Gradient& rGradient)
{
    if (rGradient.GetStyle() != GradientStyle::Linear
        && rGradient.GetStyle() != GradientStyle::Axial
        && rGradient.GetStyle() != GradientStyle::Radial)
        return false; // unsupported
    if (rGradient.GetSteps() != 0)
        return false; // We can't tell Skia how many colors to use in the gradient.
    preDraw();
    SAL_INFO("vcl.skia.trace", "drawgradient(" << this << "): " << rPolyPolygon.getB2DPolyPolygon()
                                               << ":" << static_cast<int>(rGradient.GetStyle()));
    tools::Rectangle boundRect(rPolyPolygon.GetBoundRect());
    if (boundRect.IsEmpty())
        return true;
    SkPath path;
    if (rPolyPolygon.IsRect())
    {
        // Rect->Polygon conversion loses the right and bottom edge, fix that.
        path.addRect(SkRect::MakeXYWH(boundRect.getX(), boundRect.getY(), boundRect.GetWidth(),
                                      boundRect.GetHeight()));
        boundRect.AdjustRight(1);
        boundRect.AdjustBottom(1);
    }
    else
        addPolyPolygonToPath(rPolyPolygon.getB2DPolyPolygon(), path);
    path.setFillType(SkPathFillType::kEvenOdd);
    addUpdateRegion(path.getBounds());

    Gradient aGradient(rGradient);
    tools::Rectangle aBoundRect;
    Point aCenter;
    aGradient.SetAngle(aGradient.GetAngle() + 2700_deg10);
    aGradient.GetBoundRect(boundRect, aBoundRect, aCenter);

    SkColor startColor
        = toSkColorWithIntensity(rGradient.GetStartColor(), rGradient.GetStartIntensity());
    SkColor endColor = toSkColorWithIntensity(rGradient.GetEndColor(), rGradient.GetEndIntensity());

    sk_sp<SkShader> shader;
    if (rGradient.GetStyle() == GradientStyle::Linear)
    {
        tools::Polygon aPoly(aBoundRect);
        aPoly.Rotate(aCenter, aGradient.GetAngle() % 3600_deg10);
        SkPoint points[2] = { SkPoint::Make(toSkX(aPoly[0].X()), toSkY(aPoly[0].Y())),
                              SkPoint::Make(toSkX(aPoly[1].X()), toSkY(aPoly[1].Y())) };
        SkColor colors[2] = { startColor, endColor };
        SkScalar pos[2] = { SkDoubleToScalar(aGradient.GetBorder() / 100.0), 1.0 };
        shader = SkGradientShader::MakeLinear(points, colors, pos, 2, SkTileMode::kClamp);
    }
    else if (rGradient.GetStyle() == GradientStyle::Axial)
    {
        tools::Polygon aPoly(aBoundRect);
        aPoly.Rotate(aCenter, aGradient.GetAngle() % 3600_deg10);
        SkPoint points[2] = { SkPoint::Make(toSkX(aPoly[0].X()), toSkY(aPoly[0].Y())),
                              SkPoint::Make(toSkX(aPoly[1].X()), toSkY(aPoly[1].Y())) };
        SkColor colors[3] = { endColor, startColor, endColor };
        SkScalar border = SkDoubleToScalar(aGradient.GetBorder() / 100.0);
        SkScalar pos[3]
            = { std::min<SkScalar>(border, 0.5), 0.5, std::max<SkScalar>(1 - border, 0.5) };
        shader = SkGradientShader::MakeLinear(points, colors, pos, 3, SkTileMode::kClamp);
    }
    else
    {
        // Move the center by (-1,-1) (the default VCL algorithm is a bit off-center that way,
        // Skia is the opposite way).
        SkPoint center = SkPoint::Make(toSkX(aCenter.X()) - 1, toSkY(aCenter.Y()) - 1);
        SkScalar radius = std::max(aBoundRect.GetWidth() / 2.0, aBoundRect.GetHeight() / 2.0);
        SkColor colors[2] = { endColor, startColor };
        SkScalar pos[2] = { SkDoubleToScalar(aGradient.GetBorder() / 100.0), 1.0 };
        shader = SkGradientShader::MakeRadial(center, radius, colors, pos, 2, SkTileMode::kClamp);
    }

    SkPaint paint = makeGradientPaint();
    paint.setAntiAlias(mParent.getAntiAlias());
    paint.setShader(shader);
    getDrawCanvas()->drawPath(path, paint);
    postDraw();
    return true;
}

bool SkiaSalGraphicsImpl::implDrawGradient(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                           const SalGradient& rGradient)
{
    preDraw();
    SAL_INFO("vcl.skia.trace",
             "impldrawgradient(" << this << "): " << rPolyPolygon << ":" << rGradient.maPoint1
                                 << "->" << rGradient.maPoint2 << ":" << rGradient.maStops.size());

    SkPath path;
    addPolyPolygonToPath(rPolyPolygon, path);
    path.setFillType(SkPathFillType::kEvenOdd);
    addUpdateRegion(path.getBounds());

    SkPoint points[2]
        = { SkPoint::Make(toSkX(rGradient.maPoint1.getX()), toSkY(rGradient.maPoint1.getY())),
            SkPoint::Make(toSkX(rGradient.maPoint2.getX()), toSkY(rGradient.maPoint2.getY())) };
    std::vector<SkColor> colors;
    std::vector<SkScalar> pos;
    for (const SalGradientStop& stop : rGradient.maStops)
    {
        colors.emplace_back(toSkColor(stop.maColor));
        pos.emplace_back(stop.mfOffset);
    }
    sk_sp<SkShader> shader = SkGradientShader::MakeLinear(points, colors.data(), pos.data(),
                                                          colors.size(), SkTileMode::kDecal);
    SkPaint paint = makeGradientPaint();
    paint.setAntiAlias(mParent.getAntiAlias());
    paint.setShader(shader);
    getDrawCanvas()->drawPath(path, paint);
    postDraw();
    return true;
}

static double toRadian(Degree10 degree10th) { return toRadians(3600_deg10 - degree10th); }
static double toCos(Degree10 degree10th) { return SkScalarCos(toRadian(degree10th)); }
static double toSin(Degree10 degree10th) { return SkScalarSin(toRadian(degree10th)); }

void SkiaSalGraphicsImpl::drawGenericLayout(const GenericSalLayout& layout, Color textColor,
                                            const SkFont& font, const SkFont& verticalFont)
{
    SkiaZone zone;
    std::vector<SkGlyphID> glyphIds;
    std::vector<SkRSXform> glyphForms;
    std::vector<bool> verticals;
    glyphIds.reserve(256);
    glyphForms.reserve(256);
    verticals.reserve(256);
    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (layout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        glyphIds.push_back(pGlyph->glyphId());
        Degree10 angle = layout.GetOrientation();
        if (pGlyph->IsVertical())
            angle += 900_deg10;
        SkRSXform form = SkRSXform::Make(toCos(angle), toSin(angle), aPos.X(), aPos.Y());
        glyphForms.emplace_back(std::move(form));
        verticals.emplace_back(pGlyph->IsVertical());
    }
    if (glyphIds.empty())
        return;

    preDraw();
    auto getBoundRect = [&layout]() {
        tools::Rectangle rect;
        layout.GetBoundRect(rect);
        return rect;
    };
    SAL_INFO("vcl.skia.trace", "drawtextblob(" << this << "): " << getBoundRect() << ", "
                                               << glyphIds.size() << " glyphs, " << textColor);

    // Vertical glyphs need a different font, so split drawing into runs that each
    // draw only consecutive horizontal or vertical glyphs.
    std::vector<bool>::const_iterator pos = verticals.cbegin();
    std::vector<bool>::const_iterator end = verticals.cend();
    while (pos != end)
    {
        bool verticalRun = *pos;
        std::vector<bool>::const_iterator rangeEnd = std::find(pos + 1, end, !verticalRun);
        size_t index = pos - verticals.cbegin();
        size_t count = rangeEnd - pos;
        sk_sp<SkTextBlob> textBlob = SkTextBlob::MakeFromRSXform(
            glyphIds.data() + index, count * sizeof(SkGlyphID), glyphForms.data() + index,
            verticalRun ? verticalFont : font, SkTextEncoding::kGlyphID);
        addUpdateRegion(textBlob->bounds());
        SkPaint paint = makeTextPaint(textColor);
        getDrawCanvas()->drawTextBlob(textBlob, 0, 0, paint);
        pos = rangeEnd;
    }
    postDraw();
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

static int getScaling()
{
    // It makes sense to support the debugging flag on all platforms
    // for unittests purpose, even if the actual windows cannot do it.
    if (const char* env = getenv("SAL_FORCE_HIDPI_SCALING"))
        return atoi(env);
    return 1;
}

int SkiaSalGraphicsImpl::getWindowScaling() const
{
    static const int scaling = getScaling();
    return scaling;
}

void SkiaSalGraphicsImpl::dump(const char* file) const
{
    assert(mSurface.get());
    SkiaHelper::dump(mSurface, file);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
