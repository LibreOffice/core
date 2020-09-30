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

            if (aPreviousControlPoint.equal(aPreviousPoint)
                && aCurrentControlPoint.equal(aCurrentPoint))
                rPath.lineTo(aCurrentPoint.getX(), aCurrentPoint.getY()); // a straight line
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
            }
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

SkColor toSkColor(Color color)
{
    return SkColorSetARGB(255 - color.GetTransparency(), color.GetRed(), color.GetGreen(),
                          color.GetBlue());
}

SkColor toSkColorWithTransparency(Color aColor, double fTransparency)
{
    return SkColorSetA(toSkColor(aColor), 255 * (1.0 - fTransparency));
}

SkColor toSkColorWithIntensity(Color color, int intensity)
{
    return SkColorSetARGB(255 - color.GetTransparency(), color.GetRed() * intensity / 100,
                          color.GetGreen() * intensity / 100, color.GetBlue() * intensity / 100);
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
    , mXorMode(false)
    , mFlush(new SkiaFlushIdle(this))
    , mPendingOperationsToFlush(0)
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
    SkiaZone zone;
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

void SkiaSalGraphicsImpl::createWindowSurface(bool forceRaster)
{
    SkiaZone zone;
    assert(!isOffscreen());
    assert(!mSurface);
    assert(!mWindowContext);
    createWindowContext(forceRaster);
    if (mWindowContext)
        mSurface = mWindowContext->getBackbufferSurface();
    if (!mSurface)
    {
        switch (SkiaHelper::renderMethodToUse())
        {
            case SkiaHelper::RenderVulkan:
                SAL_WARN("vcl.skia",
                         "cannot create Vulkan GPU window surface, falling back to Raster");
                destroySurface(); // destroys also WindowContext
                return createWindowSurface(true); // try again
            case SkiaHelper::RenderRaster:
                abort(); // This should not really happen, do not even try to cope with it.
        }
    }
    mIsGPU = mSurface->getCanvas()->getGrContext() != nullptr;
#ifdef DBG_UTIL
    SkiaHelper::prefillSurface(mSurface);
#endif
}

void SkiaSalGraphicsImpl::createOffscreenSurface()
{
    SkiaZone zone;
    assert(isOffscreen());
    assert(!mSurface);
    assert(!mWindowContext);
    // When created (especially on Windows), Init() gets called with size (0,0), which is invalid size
    // for Skia. May happen also in rare cases such as shutting down (tdf#131939).
    int width = std::max(1, GetWidth());
    int height = std::max(1, GetHeight());
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderVulkan:
        {
            if (SkiaHelper::getSharedGrDirectContext())
            {
                mSurface = SkiaHelper::createSkSurface(width, height);
                if (mSurface)
                {
                    mIsGPU = mSurface->getCanvas()->getGrContext() != nullptr;
                    return;
                }
            }
            break;
        }
        default:
            break;
    }
    // Create raster surface as a fallback.
    mSurface = SkiaHelper::createSkSurface(width, height);
    assert(mSurface);
    assert(!mSurface->getCanvas()->getGrContext()); // is not GPU-backed
    mIsGPU = false;
}

void SkiaSalGraphicsImpl::destroySurface()
{
    SkiaZone zone;
    if (mSurface)
    {
        // check setClipRegion() invariant
        assert(mSurface->getCanvas()->getSaveCount() == 2);
        // if this fails, something forgot to use SkAutoCanvasRestore
        assert(mSurface->getCanvas()->getTotalMatrix().isIdentity());
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
    if (GrContext* context = mSurface->getCanvas()->getGrContext())
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
    else if (GetWidth() != mSurface->width() || GetHeight() != mSurface->height())
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
                snapshot = SkiaHelper::makeCheckedImageSnapshot(mSurface);
            }
            recreateSurface();
            if (snapshot)
            {
                SkPaint paint;
                paint.setBlendMode(SkBlendMode::kSrc); // copy as is
                mSurface->getCanvas()->drawImage(snapshot, 0, 0, &paint);
            }
            SAL_INFO("vcl.skia.trace", "recreate(" << this << "): old " << oldSize << " new "
                                                   << Size(mSurface->width(), mSurface->height())
                                                   << " requested "
                                                   << Size(GetWidth(), GetHeight()));
        }
    }
}

void SkiaSalGraphicsImpl::flushDrawing()
{
    if (!mSurface)
        return;
    checkPendingDrawing();
    if (mXorMode)
        applyXor();
    mSurface->flushAndSubmit();
    mPendingOperationsToFlush = 0;
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
    // SkCanvas::clipRegion() can only further reduce the clip region,
    // but we need to set the given region, which may extend it.
    // So handle that by always having the full clip region saved on the stack
    // and always go back to that. SkCanvas::restore() only affects the clip
    // and the matrix.
    assert(canvas->getSaveCount() == 2); // = there is just one save()
    canvas->restore();
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

long SkiaSalGraphicsImpl::GetGraphicsWidth() const { return GetWidth(); }

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

void SkiaSalGraphicsImpl::SetXORMode(bool set, bool)
{
    if (mXorMode == set)
        return;
    checkPendingDrawing();
    SAL_INFO("vcl.skia.trace", "setxormode(" << this << "): " << set);
    if (set)
        mXorRegion.setEmpty();
    else
        applyXor();
    mXorMode = set;
}

SkCanvas* SkiaSalGraphicsImpl::getXorCanvas()
{
    SkiaZone zone;
    assert(mXorMode);
    // Skia does not implement xor drawing, so we need to handle it manually by redirecting
    // to a temporary SkBitmap and then doing the xor operation on the data ourselves.
    // There's no point in using SkSurface for GPU, we'd immediately need to get the pixels back.
    if (!mXorCanvas)
    {
        // Use unpremultiplied alpha (see xor applying in applyXor()).
        if (!mXorBitmap.tryAllocPixels(mSurface->imageInfo().makeAlphaType(kUnpremul_SkAlphaType)))
            abort();
        mXorBitmap.eraseARGB(0, 0, 0, 0);
        mXorCanvas = std::make_unique<SkCanvas>(mXorBitmap);
        setCanvasClipRegion(mXorCanvas.get(), mClipRegion);
    }
    return mXorCanvas.get();
}

void SkiaSalGraphicsImpl::applyXor()
{
    // Apply the result from the temporary bitmap manually. This is indeed
    // slow, but it doesn't seem to be needed often and is optimized
    // in each operation by extending mXorRegion with the area that should be
    // updated.
    assert(mXorMode);
    if (!mSurface || !mXorCanvas
        || !mXorRegion.op(SkIRect::MakeXYWH(0, 0, mSurface->width(), mSurface->height()),
                          SkRegion::kIntersect_Op))
    {
        mXorRegion.setEmpty();
        return;
    }
    SAL_INFO("vcl.skia.trace", "applyxor(" << this << "): " << mXorRegion);
    // Copy the surface contents to another pixmap.
    SkBitmap surfaceBitmap;
    // Use unpremultiplied alpha format, so that we do not have to do the conversions to get
    // the RGB and back (Skia will do it when converting, but it'll be presumably faster at it).
    if (!surfaceBitmap.tryAllocPixels(mSurface->imageInfo().makeAlphaType(kUnpremul_SkAlphaType)))
        abort();
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is
    SkCanvas canvas(surfaceBitmap);
    canvas.drawImageRect(SkiaHelper::makeCheckedImageSnapshot(mSurface), mXorRegion.getBounds(),
                         SkRect::Make(mXorRegion.getBounds()), &paint);
    // xor to surfaceBitmap
    assert(surfaceBitmap.info().alphaType() == kUnpremul_SkAlphaType);
    assert(mXorBitmap.info().alphaType() == kUnpremul_SkAlphaType);
    assert(surfaceBitmap.bytesPerPixel() == 4);
    assert(mXorBitmap.bytesPerPixel() == 4);
    for (SkRegion::Iterator it(mXorRegion); !it.done(); it.next())
    {
        for (int y = it.rect().top(); y < it.rect().bottom(); ++y)
        {
            uint8_t* data = static_cast<uint8_t*>(surfaceBitmap.getAddr(it.rect().x(), y));
            const uint8_t* xordata = static_cast<uint8_t*>(mXorBitmap.getAddr(it.rect().x(), y));
            for (int x = 0; x < it.rect().width(); ++x)
            {
                *data++ ^= *xordata++;
                *data++ ^= *xordata++;
                *data++ ^= *xordata++;
                // alpha is not xor-ed
                data++;
                xordata++;
            }
        }
    }
    surfaceBitmap.notifyPixelsChanged();
    mSurface->getCanvas()->drawBitmapRect(surfaceBitmap, mXorRegion.getBounds(),
                                          SkRect::Make(mXorRegion.getBounds()), &paint);
    mXorCanvas.reset();
    mXorBitmap.reset();
    mXorRegion.setEmpty();
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

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY) { drawPixel(nX, nY, mLineColor); }

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY, Color nColor)
{
    if (nColor == SALCOLOR_NONE)
        return;
    preDraw();
    SAL_INFO("vcl.skia.trace", "drawpixel(" << this << "): " << Point(nX, nY) << ":" << nColor);
    addXorRegion(SkRect::MakeXYWH(nX, nY, 1, 1));
    SkPaint paint;
    paint.setColor(toSkColor(nColor));
    // Apparently drawPixel() is actually expected to set the pixel and not draw it.
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    getDrawCanvas()->drawPoint(toSkX(nX), toSkY(nY), paint);
    postDraw();
}

void SkiaSalGraphicsImpl::drawLine(long nX1, long nY1, long nX2, long nY2)
{
    if (mLineColor == SALCOLOR_NONE)
        return;
    preDraw();
    SAL_INFO("vcl.skia.trace", "drawline(" << this << "): " << Point(nX1, nY1) << "->"
                                           << Point(nX2, nY2) << ":" << mLineColor);
    addXorRegion(SkRect::MakeLTRB(nX1, nY1, nX2, nY2).makeSorted());
    SkPaint paint;
    paint.setColor(toSkColor(mLineColor));
    paint.setAntiAlias(mParent.getAntiAlias());
    getDrawCanvas()->drawLine(toSkX(nX1), toSkY(nY1), toSkX(nX2), toSkY(nY2), paint);
    postDraw();
}

void SkiaSalGraphicsImpl::privateDrawAlphaRect(long nX, long nY, long nWidth, long nHeight,
                                               double fTransparency, bool blockAA)
{
    preDraw();
    SAL_INFO("vcl.skia.trace",
             "privatedrawrect(" << this << "): " << SkIRect::MakeXYWH(nX, nY, nWidth, nHeight)
                                << ":" << mLineColor << ":" << mFillColor << ":" << fTransparency);
    addXorRegion(SkRect::MakeXYWH(nX, nY, nWidth, nHeight));
    SkCanvas* canvas = getDrawCanvas();
    SkPaint paint;
    paint.setAntiAlias(!blockAA && mParent.getAntiAlias());
    if (mFillColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColorWithTransparency(mFillColor, fTransparency));
        paint.setStyle(SkPaint::kFill_Style);
        // HACK: If the polygon is just a line, it still should be drawn. But when filling
        // Skia doesn't draw empty polygons, so in that case ensure the line is drawn.
        if (mLineColor == SALCOLOR_NONE && SkSize::Make(nWidth, nHeight).isEmpty())
            paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawIRect(SkIRect::MakeXYWH(nX, nY, nWidth, nHeight), paint);
    }
    if (mLineColor != SALCOLOR_NONE)
    {
        paint.setColor(toSkColorWithTransparency(mLineColor, fTransparency));
        paint.setStyle(SkPaint::kStroke_Style);
        // The obnoxious "-1 DrawRect()" hack that I don't understand the purpose of (and I'm not sure
        // if anybody does), but without it some cases do not work. The max() is needed because Skia
        // will not draw anything if width or height is 0.
        canvas->drawIRect(
            SkIRect::MakeXYWH(nX, nY, std::max(1L, nWidth - 1), std::max(1L, nHeight - 1)), paint);
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

    drawPolyLine(basegfx::B2DHomMatrix(), aPolygon, 0.0, 1.0, nullptr, basegfx::B2DLineJoin::Miter,
                 css::drawing::LineCap_BUTT, basegfx::deg2rad(15.0) /*default*/, false);
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
    addPolyPolygonToPath(aPolyPolygon, polygonPath);
    polygonPath.setFillType(SkPathFillType::kEvenOdd);
    addXorRegion(polygonPath.getBounds());

    SkPaint aPaint;
    aPaint.setAntiAlias(useAA);
    // We normally use pixel at their center positions, but slightly off (see toSkX/Y()).
    // With AA lines that "slightly off" causes tiny changes of color, making some tests
    // fail. Since moving AA-ed line slightly to a side doesn't cause any real visual
    // difference, just place exactly at the center. tdf#134346
    const SkScalar posFix = useAA ? toSkXYFix : 0;
    if (mFillColor != SALCOLOR_NONE)
    {
        SkPath path;
        polygonPath.offset(toSkX(0) + posFix, toSkY(0) + posFix, &path);
        aPaint.setColor(toSkColorWithTransparency(mFillColor, fTransparency));
        aPaint.setStyle(SkPaint::kFill_Style);
        // HACK: If the polygon is just a line, it still should be drawn. But when filling
        // Skia doesn't draw empty polygons, so in that case ensure the line is drawn.
        if (mLineColor == SALCOLOR_NONE && path.getBounds().isEmpty())
            aPaint.setStyle(SkPaint::kStroke_Style);
        getDrawCanvas()->drawPath(path, aPaint);
    }
    if (mLineColor != SALCOLOR_NONE)
    {
        SkPath path;
        polygonPath.offset(toSkX(0) + posFix, toSkY(0) + posFix, &path);
        aPaint.setColor(toSkColorWithTransparency(mLineColor, fTransparency));
        aPaint.setStyle(SkPaint::kStroke_Style);
        getDrawCanvas()->drawPath(path, aPaint);
    }
    postDraw();
#if defined LINUX
    // WORKAROUND: The logo in the about dialog has drawing errors. This seems to happen
    // only on Linux (not Windows on the same machine), with both AMDGPU and Mesa,
    // and only when antialiasing is enabled. Flushing seems to avoid the problem.
    if (useAA && SkiaHelper::getVendor() == DriverBlocklist::VendorAMD)
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
            // TODO: tdf#136222 shows that basegfx::utils::mergeToSinglePolyPolygon() is unreliable
            // in corner cases, possibly either a bug or rounding errors somewhere.
            performDrawPolyPolygon(basegfx::utils::mergeToSinglePolyPolygon(polygons), transparency,
                                   true);
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

    // tdf#124848 get correct LineWidth in discrete coordinates,
    if (fLineWidth == 0) // hairline
        fLineWidth = 1.0;
    else // Adjust line width for object-to-device scale.
        fLineWidth = (rObjectToDevice * basegfx::B2DVector(fLineWidth, 0)).getLength();

    // Transform to DeviceCoordinates, get DeviceLineWidth, execute PixelSnapHairline
    basegfx::B2DPolyPolygon aPolyPolygonLine;
    aPolyPolygonLine.append(rPolyLine);
    aPolyPolygonLine.transform(rObjectToDevice);
    if (bPixelSnapHairline)
    {
        aPolyPolygonLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyPolygonLine);
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

    SkPaint aPaint;
    aPaint.setStyle(SkPaint::kStroke_Style);
    aPaint.setStrokeCap(eSkLineCap);
    aPaint.setStrokeJoin(eSkLineJoin);
    aPaint.setColor(toSkColorWithTransparency(mLineColor, fTransparency));
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
        aPath.setFillType(SkPathFillType::kEvenOdd);
        for (sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
            addPolygonToPath(aPolyPolygonLine.getB2DPolygon(a), aPath);
        aPath.offset(toSkX(0) + posFix, toSkY(0) + posFix, nullptr);
        addXorRegion(aPath.getBounds());
        getDrawCanvas()->drawPath(aPath, aPaint);
    }
    else
    {
        for (sal_uInt32 i = 0; i < aPolyPolygonLine.count(); ++i)
        {
            const basegfx::B2DPolygon& rPolygon = aPolyPolygonLine.getB2DPolygon(i);
            sal_uInt32 nPoints = rPolygon.count();
            bool bClosed = rPolygon.isClosed();
            for (sal_uInt32 j = 0; j < (bClosed ? nPoints : nPoints - 1); ++j)
            {
                sal_uInt32 index1 = (j + 0) % nPoints;
                sal_uInt32 index2 = (j + 1) % nPoints;
                SkPath aPath;
                aPath.moveTo(rPolygon.getB2DPoint(index1).getX(),
                             rPolygon.getB2DPoint(index1).getY());
                aPath.lineTo(rPolygon.getB2DPoint(index2).getX(),
                             rPolygon.getB2DPoint(index2).getY());

                aPath.offset(toSkX(0) + posFix, toSkY(0) + posFix, nullptr);
                addXorRegion(aPath.getBounds());
                getDrawCanvas()->drawPath(aPath, aPaint);
            }
        }
    }

    postDraw();

    return true;
}

bool SkiaSalGraphicsImpl::drawPolyLineBezier(sal_uInt32, const SalPoint*, const PolyFlags*)
{
    return false;
}

bool SkiaSalGraphicsImpl::drawPolygonBezier(sal_uInt32, const SalPoint*, const PolyFlags*)
{
    return false;
}

bool SkiaSalGraphicsImpl::drawPolyPolygonBezier(sal_uInt32, const sal_uInt32*,
                                                const SalPoint* const*, const PolyFlags* const*)
{
    return false;
}

static void copyArea(SkCanvas* canvas, sk_sp<SkSurface> surface, long nDestX, long nDestY,
                     long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, bool srcIsRaster,
                     bool destIsRaster)
{
    // Using SkSurface::draw() should be more efficient than SkSurface::makeImageSnapshot(),
    // because it may detect copying to itself and avoid some needless copies.
    // But it has problems with drawing to itself
    // (https://groups.google.com/forum/#!topic/skia-discuss/6yiuw24jv0I) and also
    // raster surfaces do not avoid a copy of the source
    // (https://groups.google.com/forum/#!topic/skia-discuss/S3FMpCi82k0).
    // Finally, there's not much point if one of them is raster and the other is not (chrome/m86 even crashes).
    if (canvas == surface->getCanvas() || srcIsRaster || (srcIsRaster != destIsRaster))
    {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
        canvas->drawImageRect(SkiaHelper::makeCheckedImageSnapshot(surface),
                              SkIRect::MakeXYWH(nSrcX, nSrcY, nSrcWidth, nSrcHeight),
                              SkRect::MakeXYWH(nDestX, nDestY, nSrcWidth, nSrcHeight), &paint);
        return;
    }
    // SkCanvas::draw() cannot do a subrectangle, so clip.
    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH(nDestX, nDestY, nSrcWidth, nSrcHeight));
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
    surface->draw(canvas, nDestX - nSrcX, nDestY - nSrcY, &paint);
    canvas->restore();
}

void SkiaSalGraphicsImpl::copyArea(long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                                   long nSrcHeight, bool /*bWindowInvalidate*/)
{
    if (nDestX == nSrcX && nDestY == nSrcY)
        return;
    preDraw();
    SAL_INFO("vcl.skia.trace", "copyarea("
                                   << this << "): " << Point(nSrcX, nSrcY) << "->"
                                   << SkIRect::MakeXYWH(nDestX, nDestY, nSrcWidth, nSrcHeight));
    assert(!mXorMode);
    ::copyArea(getDrawCanvas(), mSurface, nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight,
               !isGPU(), !isGPU());
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
        assert(!mXorMode);
    }
    if (rPosAry.mnSrcWidth == rPosAry.mnDestWidth && rPosAry.mnSrcHeight == rPosAry.mnDestHeight)
    {
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
        SAL_INFO("vcl.skia.trace",
                 "copybits(" << this << "): " << srcDebug() << " copy area: " << rPosAry);
        ::copyArea(getDrawCanvas(), src->mSurface, rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnSrcX,
                   rPosAry.mnSrcY, rPosAry.mnDestWidth, rPosAry.mnDestHeight, !src->isGPU(),
                   !isGPU());
    }
    else
    {
        SAL_INFO("vcl.skia.trace", "copybits(" << this << "): (" << src << "): " << rPosAry);
        // Do not use makeImageSnapshot(rect), as that one may make a needless data copy.
        sk_sp<SkImage> image = SkiaHelper::makeCheckedImageSnapshot(src->mSurface);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // copy as is, including alpha
        if (rPosAry.mnSrcWidth != rPosAry.mnDestWidth
            || rPosAry.mnSrcHeight != rPosAry.mnDestHeight)
            paint.setFilterQuality(kHigh_SkFilterQuality);
        getDrawCanvas()->drawImageRect(image,
                                       SkIRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                         rPosAry.mnSrcWidth, rPosAry.mnSrcHeight),
                                       SkRect::MakeXYWH(rPosAry.mnDestX, rPosAry.mnDestY,
                                                        rPosAry.mnDestWidth, rPosAry.mnDestHeight),
                                       &paint);
    }
    assert(!mXorMode);
    postDraw();
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

    // First do the "( 1 - alpha ) * mask"
    // (no idea how to do "floor", but hopefully not needed in practice).
    sk_sp<SkShader> shaderAlpha
        = SkShaders::Blend(SkBlendMode::kDstOut, rSkiaMaskBitmap.GetAlphaSkShader(),
                           rSkiaAlphaBitmap.GetAlphaSkShader());
    // And now draw the bitmap with "1 - x", where x is the "( 1 - alpha ) * mask".
    sk_sp<SkShader> shader
        = SkShaders::Blend(SkBlendMode::kSrcOut, shaderAlpha, rSkiaSourceBitmap.GetSkShader());
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
    drawShader(rPosAry,
               SkShaders::Blend(SkBlendMode::kDstOut, // VCL alpha is one-minus-alpha.
                                SkShaders::Color(toSkColor(nMaskColor)),
                                skiaBitmap.GetAlphaSkShader()));
}

std::shared_ptr<SalBitmap> SkiaSalGraphicsImpl::getBitmap(long nX, long nY, long nWidth,
                                                          long nHeight)
{
    SkiaZone zone;
    checkSurface();
    SAL_INFO("vcl.skia.trace",
             "getbitmap(" << this << "): " << SkIRect::MakeXYWH(nX, nY, nWidth, nHeight));
    flushDrawing();
    // TODO makeImageSnapshot(rect) may copy the data, which may be a waste if this is used
    // e.g. for VirtualDevice's lame alpha blending, in which case the image will eventually end up
    // in blendAlphaBitmap(), where we could simply use the proper rect of the image.
    sk_sp<SkImage> image = SkiaHelper::makeCheckedImageSnapshot(
        mSurface, SkIRect::MakeXYWH(nX, nY, nWidth, nHeight));
    return std::make_shared<SkiaSalBitmap>(image);
}

Color SkiaSalGraphicsImpl::getPixel(long nX, long nY)
{
    SkiaZone zone;
    checkSurface();
    SAL_INFO("vcl.skia.trace", "getpixel(" << this << "): " << Point(nX, nY));
    flushDrawing();
    // This is presumably slow, but getPixel() should be generally used only by unit tests.
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
    SAL_INFO("vcl.skia.trace", "invert(" << this << "): " << rPoly << ":" << int(eFlags));
    assert(!mXorMode);
    // Intel Vulkan drivers (up to current 0.401.3889) have a problem
    // with SkBlendMode::kDifference(?) and surfaces wider than 1024 pixels, resulting
    // in drawing errors. Work that around by fetching the relevant part of the surface
    // and drawing using CPU.
    bool intelHack
        = (isGPU() && SkiaHelper::getVendor() == DriverBlocklist::VendorIntel && !mXorMode);
    // TrackFrame just inverts a dashed path around the polygon
    if (eFlags == SalInvert::TrackFrame)
    {
        SkPath aPath;
        addPolygonToPath(rPoly, aPath);
        aPath.setFillType(SkPathFillType::kEvenOdd);
        // TrackFrame is not supposed to paint outside of the polygon (usually rectangle),
        // but wider stroke width usually results in that, so ensure the requirement
        // by clipping.
        SkAutoCanvasRestore autoRestore(getDrawCanvas(), true);
        getDrawCanvas()->clipRect(aPath.getBounds(), SkClipOp::kIntersect, false);
        SkPaint aPaint;
        aPaint.setStrokeWidth(2);
        float intervals[] = { 4.0f, 4.0f };
        aPaint.setStyle(SkPaint::kStroke_Style);
        aPaint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
        aPaint.setColor(SkColorSetARGB(255, 255, 255, 255));
        aPaint.setBlendMode(SkBlendMode::kDifference);
        if (!intelHack)
            getDrawCanvas()->drawPath(aPath, aPaint);
        else
        {
            SkRect area;
            aPath.getBounds().roundOut(&area);
            SkRect size = SkRect::MakeWH(area.width(), area.height());
            sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(area.width(), area.height());
            SkPaint copy;
            copy.setBlendMode(SkBlendMode::kSrc);
            flushDrawing();
            surface->getCanvas()->drawImageRect(SkiaHelper::makeCheckedImageSnapshot(mSurface),
                                                area, size, &copy);
            aPath.offset(-area.x(), -area.y());
            surface->getCanvas()->drawPath(aPath, aPaint);
            getDrawCanvas()->drawImageRect(SkiaHelper::makeCheckedImageSnapshot(surface), size,
                                           area, &copy);
        }
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

        // N50 inverts in checker pattern
        if (eFlags == SalInvert::N50)
        {
            // This creates 2x2 checker pattern bitmap
            // TODO Use SkiaHelper::createSkSurface() and cache the image
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
            aPaint.setShader(aBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
        }
        if (!intelHack)
            getDrawCanvas()->drawPath(aPath, aPaint);
        else
        {
            SkRect area;
            aPath.getBounds().roundOut(&area);
            SkRect size = SkRect::MakeWH(area.width(), area.height());
            sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(area.width(), area.height());
            SkPaint copy;
            copy.setBlendMode(SkBlendMode::kSrc);
            flushDrawing();
            surface->getCanvas()->drawImageRect(SkiaHelper::makeCheckedImageSnapshot(mSurface),
                                                area, size, &copy);
            aPath.offset(-area.x(), -area.y());
            surface->getCanvas()->drawPath(aPath, aPaint);
            getDrawCanvas()->drawImageRect(SkiaHelper::makeCheckedImageSnapshot(surface), size,
                                           area, &copy);
        }
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

bool SkiaSalGraphicsImpl::drawEPS(long, long, long, long, void*, sal_uInt32) { return false; }

// Create SkImage from a bitmap and possibly an alpha mask (the usual VCL one-minus-alpha),
// with the given target size. Result will be possibly cached, unless disabled.
sk_sp<SkImage> SkiaSalGraphicsImpl::mergeCacheBitmaps(const SkiaSalBitmap& bitmap,
                                                      const SkiaSalBitmap* alphaBitmap,
                                                      const Size targetSize)
{
    sk_sp<SkImage> image;
    // GPU-accelerated drawing with SkShader should be fast enough to not need caching.
    if (isGPU())
        return image;
    if (targetSize.IsEmpty())
        return image;
    if (alphaBitmap && alphaBitmap->IsFullyOpaqueAsAlpha())
        alphaBitmap = nullptr; // the alpha can be ignored
    // Probably not much point in caching of just doing a copy.
    if (alphaBitmap == nullptr && targetSize == bitmap.GetSize())
        return image;
    // Image too small to be worth caching.
    if (bitmap.GetSize().Width() < 100 && bitmap.GetSize().Height() < 100
        && targetSize.Width() < 100 && targetSize.Height() < 100)
        return image;
    // In some cases (tdf#134237) the target size may be very large. In that case it's
    // better to rely on Skia to clip and draw only the necessary, rather than prepare
    // a very large image only to not use most of it.
    const Size drawAreaSize = mClipRegion.GetBoundRect().GetSize();
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
            return image;
        }
    }
    // Do not cache the result if it would take most of the cache and thus get evicted soon.
    if (targetSize.Width() * targetSize.Height() * 4 > SkiaHelper::MAX_CACHE_SIZE * 0.7)
        return image;
    OString key;
    OStringBuffer keyBuf;
    keyBuf.append(targetSize.Width())
        .append("x")
        .append(targetSize.Height())
        .append("_")
        .append(bitmap.GetImageKey());
    if (alphaBitmap)
        keyBuf.append("_").append(alphaBitmap->GetAlphaImageKey());
    key = keyBuf.makeStringAndClear();
    image = SkiaHelper::findCachedImage(key);
    if (image)
    {
        assert(image->width() == targetSize.Width() && image->height() == targetSize.Height());
        return image;
    }
    sk_sp<SkSurface> tmpSurface = SkiaHelper::createSkSurface(targetSize);
    if (!tmpSurface)
        return nullptr;
    SkCanvas* canvas = tmpSurface->getCanvas();
    SkAutoCanvasRestore autoRestore(canvas, true);
    SkPaint paint;
    if (targetSize != bitmap.GetSize())
    {
        SkMatrix matrix;
        matrix.set(SkMatrix::kMScaleX, 1.0 * targetSize.Width() / bitmap.GetSize().Width());
        matrix.set(SkMatrix::kMScaleY, 1.0 * targetSize.Height() / bitmap.GetSize().Height());
        canvas->concat(matrix);
        paint.setFilterQuality(kHigh_SkFilterQuality);
    }
    if (alphaBitmap != nullptr)
    {
        canvas->clear(SK_ColorTRANSPARENT);
        paint.setShader(SkShaders::Blend(SkBlendMode::kDstOut, bitmap.GetSkShader(),
                                         alphaBitmap->GetAlphaSkShader()));
        canvas->drawPaint(paint);
    }
    else if (bitmap.PreferSkShader())
    {
        paint.setShader(bitmap.GetSkShader());
        canvas->drawPaint(paint);
    }
    else
        canvas->drawImage(bitmap.GetSkImage(), 0, 0, &paint);
    image = SkiaHelper::makeCheckedImageSnapshot(tmpSurface);
    SkiaHelper::addCachedImage(key, image);
    return image;
}

bool SkiaSalGraphicsImpl::drawAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                          const SalBitmap& rAlphaBitmap)
{
    assert(dynamic_cast<const SkiaSalBitmap*>(&rSourceBitmap));
    assert(dynamic_cast<const SkiaSalBitmap*>(&rAlphaBitmap));
    const SkiaSalBitmap& rSkiaSourceBitmap = static_cast<const SkiaSalBitmap&>(rSourceBitmap);
    const SkiaSalBitmap& rSkiaAlphaBitmap = static_cast<const SkiaSalBitmap&>(rAlphaBitmap);
    // In raster mode use mergeCacheBitmaps(), which will cache the result, avoiding repeated
    // alpha blending or scaling. In GPU mode it is simpler to just use SkShader.
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
    sk_sp<SkImage> image = mergeCacheBitmaps(rSkiaSourceBitmap, &rSkiaAlphaBitmap, imageSize);
    if (image)
        drawImage(imagePosAry, image);
    else if (rSkiaAlphaBitmap.IsFullyOpaqueAsAlpha()) // alpha can be ignored
        drawBitmap(rPosAry, rSkiaSourceBitmap);
    else
        drawShader(rPosAry,
                   SkShaders::Blend(SkBlendMode::kDstOut, // VCL alpha is one-minus-alpha.
                                    rSkiaSourceBitmap.GetSkShader(),
                                    rSkiaAlphaBitmap.GetAlphaSkShader()));
    return true;
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SkiaSalBitmap& bitmap,
                                     SkBlendMode blendMode)
{
    if (bitmap.PreferSkShader())
        drawShader(rPosAry, bitmap.GetSkShader(), blendMode);
    else
        drawImage(rPosAry, bitmap.GetSkImage(), blendMode);
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
    if (rPosAry.mnSrcWidth != rPosAry.mnDestWidth || rPosAry.mnSrcHeight != rPosAry.mnDestHeight)
        aPaint.setFilterQuality(kHigh_SkFilterQuality);

    preDraw();
    SAL_INFO("vcl.skia.trace",
             "drawimage(" << this << "): " << rPosAry << ":" << SkBlendMode_Name(eBlendMode));
    addXorRegion(aDestinationRect);
    getDrawCanvas()->drawImageRect(aImage, aSourceRect, aDestinationRect, &aPaint);
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
    addXorRegion(destinationRect);
    SkPaint paint;
    paint.setBlendMode(blendMode);
    paint.setShader(shader);
    if (rPosAry.mnSrcWidth != rPosAry.mnDestWidth || rPosAry.mnSrcHeight != rPosAry.mnDestHeight)
        paint.setFilterQuality(kHigh_SkFilterQuality);
    SkCanvas* canvas = getDrawCanvas();
    // Scaling needs to be done explicitly using a matrix.
    SkAutoCanvasRestore autoRestore(canvas, true);
    SkMatrix matrix = SkMatrix::Translate(rPosAry.mnDestX, rPosAry.mnDestY)
                      * SkMatrix::Scale(1.0 * rPosAry.mnDestWidth / rPosAry.mnSrcWidth,
                                        1.0 * rPosAry.mnDestHeight / rPosAry.mnSrcHeight)
                      * SkMatrix::Translate(-rPosAry.mnSrcX, -rPosAry.mnSrcY);
    assert(matrix.mapXY(rPosAry.mnSrcX, rPosAry.mnSrcY)
           == SkPoint::Make(rPosAry.mnDestX, rPosAry.mnDestY));
    assert(matrix.mapXY(rPosAry.mnSrcX + rPosAry.mnSrcWidth, rPosAry.mnSrcY + rPosAry.mnSrcHeight)
           == SkPoint::Make(rPosAry.mnDestX + rPosAry.mnDestWidth,
                            rPosAry.mnDestY + rPosAry.mnDestHeight));
    canvas->concat(matrix);
    SkRect sourceRect
        = SkRect::MakeXYWH(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    canvas->drawRect(sourceRect, paint);
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

    if (pSkiaAlphaBitmap && pSkiaAlphaBitmap->IsFullyOpaqueAsAlpha())
        pSkiaAlphaBitmap = nullptr; // the alpha can be ignored

    // Setup the image transformation,
    // using the rNull, rX, rY points as destinations for the (0,0), (Width,0), (0,Height) source points.
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;

    preDraw();
    SAL_INFO("vcl.skia.trace", "drawtransformedbitmap(" << this << "): " << rSourceBitmap.GetSize()
                                                        << " " << rNull << ":" << rX << ":" << rY);

    addXorRegion(SkRect::MakeWH(GetWidth(), GetHeight())); // can't tell, use whole area
    // In raster mode scaling and alpha blending is still somewhat expensive if done repeatedly,
    // so use mergeCacheBitmaps(), which will cache the result if useful.
    // It is better to use SkShader if in GPU mode, if the operation is simple or if the temporary
    // image would be very large.
    sk_sp<SkImage> imageToDraw = mergeCacheBitmaps(
        rSkiaBitmap, pSkiaAlphaBitmap, Size(round(aXRel.getLength()), round(aYRel.getLength())));
    if (imageToDraw)
    {
        SkMatrix matrix;
        // Round sizes for scaling, so that sub-pixel differences don't
        // trigger unnecessary scaling. Image has already been scaled
        // by mergeCacheBitmaps() and we shouldn't scale here again
        // unless the drawing is also skewed.
        matrix.set(SkMatrix::kMScaleX, round(aXRel.getX()) / imageToDraw->width());
        matrix.set(SkMatrix::kMScaleY, round(aYRel.getY()) / imageToDraw->height());
        matrix.set(SkMatrix::kMSkewY, aXRel.getY() / imageToDraw->width());
        matrix.set(SkMatrix::kMSkewX, aYRel.getX() / imageToDraw->height());
        matrix.set(SkMatrix::kMTransX, rNull.getX());
        matrix.set(SkMatrix::kMTransY, rNull.getY());
        SkCanvas* canvas = getDrawCanvas();
        SkAutoCanvasRestore autoRestore(canvas, true);
        canvas->concat(matrix);
        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality);
        canvas->drawImage(imageToDraw, 0, 0, &paint);
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
        SkPaint paint;
        paint.setFilterQuality(kHigh_SkFilterQuality);
        if (pSkiaAlphaBitmap)
        {
            paint.setShader(SkShaders::Blend(SkBlendMode::kDstOut, // VCL alpha is one-minus-alpha.
                                             rSkiaBitmap.GetSkShader(),
                                             pSkiaAlphaBitmap->GetAlphaSkShader()));
            canvas->drawRect(SkRect::MakeWH(aSize.Width(), aSize.Height()), paint);
        }
        else if (rSkiaBitmap.PreferSkShader())
        {
            paint.setShader(rSkiaBitmap.GetSkShader());
            canvas->drawRect(SkRect::MakeWH(aSize.Width(), aSize.Height()), paint);
        }
        else
        {
            canvas->drawImage(rSkiaBitmap.GetSkImage(), 0, 0, &paint);
        }
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

bool SkiaSalGraphicsImpl::drawGradient(const tools::PolyPolygon& rPolyPolygon,
                                       const Gradient& rGradient)
{
    if (rGradient.GetStyle() != GradientStyle::Linear
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
    addXorRegion(path.getBounds());

    Gradient aGradient(rGradient);
    tools::Rectangle aBoundRect;
    Point aCenter;
    aGradient.SetAngle(aGradient.GetAngle() + 2700);
    aGradient.GetBoundRect(boundRect, aBoundRect, aCenter);

    SkColor startColor
        = toSkColorWithIntensity(rGradient.GetStartColor(), rGradient.GetStartIntensity());
    SkColor endColor = toSkColorWithIntensity(rGradient.GetEndColor(), rGradient.GetEndIntensity());

    sk_sp<SkShader> shader;
    if (rGradient.GetStyle() == GradientStyle::Linear)
    {
        tools::Polygon aPoly(aBoundRect);
        aPoly.Rotate(aCenter, aGradient.GetAngle() % 3600);
        SkPoint points[2] = { SkPoint::Make(toSkX(aPoly[0].X()), toSkY(aPoly[0].Y())),
                              SkPoint::Make(toSkX(aPoly[1].X()), toSkY(aPoly[1].Y())) };
        SkColor colors[2] = { startColor, endColor };
        SkScalar pos[2] = { SkDoubleToScalar(aGradient.GetBorder() / 100.0), 1.0 };
        shader = SkGradientShader::MakeLinear(points, colors, pos, 2, SkTileMode::kClamp);
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

    SkPaint paint;
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
    SkPaint paint;
    paint.setAntiAlias(mParent.getAntiAlias());
    paint.setShader(shader);
    getDrawCanvas()->drawPath(path, paint);
    addXorRegion(path.getBounds());
    postDraw();
    return true;
}

static double toRadian(int degree10th) { return (3600 - degree10th) * M_PI / 1800.0; }
static double toCos(int degree10th) { return SkScalarCos(toRadian(degree10th)); }
static double toSin(int degree10th) { return SkScalarSin(toRadian(degree10th)); }

void SkiaSalGraphicsImpl::drawGenericLayout(const GenericSalLayout& layout, Color textColor,
                                            const SkFont& font, GlyphOrientation glyphOrientation)
{
    SkiaZone zone;
    std::vector<SkGlyphID> glyphIds;
    std::vector<SkRSXform> glyphForms;
    glyphIds.reserve(256);
    glyphForms.reserve(256);
    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (layout.GetNextGlyph(&pGlyph, aPos, nStart))
    {
        glyphIds.push_back(pGlyph->glyphId());
        int angle = 0; // 10th of degree
        if (glyphOrientation == GlyphOrientation::Apply)
        {
            angle = layout.GetOrientation();
            if (pGlyph->IsVertical())
                angle += 900; // 90 degree
        }
        SkRSXform form = SkRSXform::Make(toCos(angle), toSin(angle), aPos.X(), aPos.Y());
        glyphForms.emplace_back(std::move(form));
    }
    if (glyphIds.empty())
        return;
    sk_sp<SkTextBlob> textBlob
        = SkTextBlob::MakeFromRSXform(glyphIds.data(), glyphIds.size() * sizeof(SkGlyphID),
                                      glyphForms.data(), font, SkTextEncoding::kGlyphID);
    preDraw();
    SAL_INFO("vcl.skia.trace",
             "drawtextblob(" << this << "): " << textBlob->bounds() << ":" << textColor);
    addXorRegion(textBlob->bounds());
    SkPaint paint;
    paint.setColor(toSkColor(textColor));
    getDrawCanvas()->drawTextBlob(textBlob, 0, 0, paint);
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

#ifdef DBG_UTIL
void SkiaSalGraphicsImpl::dump(const char* file) const
{
    assert(mSurface.get());
    SkiaHelper::dump(mSurface, file);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
