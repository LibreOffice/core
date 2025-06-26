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

#include <vcl/gradient.hxx>
#include <vcl/svapp.hxx>
#include <headless/BitmapHelper.hxx>
#include <headless/CairoCommon.hxx>
#include <vcl/cairo.hxx>
#include <vcl/CairoFormats.hxx>
#include <vcl/BitmapTools.hxx>
#include <SalGradient.hxx>
#include <tools/helpers.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2irange.hxx>
#include <comphelper/configuration.hxx>
#include <sal/log.hxx>
#include <osl/module.h>

#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 12, 0)
#error "require at least cairo 1.12.0"
#endif

void dl_cairo_surface_set_device_scale(cairo_surface_t* surface, double x_scale, double y_scale)
{
#if !HAVE_DLAPI || !defined(SYSTEM_CAIRO)
    cairo_surface_set_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void (*)(cairo_surface_t*, double, double)>(
        osl_getAsciiFunctionSymbol(nullptr, "cairo_surface_set_device_scale"));
    if (func)
        func(surface, x_scale, y_scale);
#endif
}

void dl_cairo_surface_get_device_scale(cairo_surface_t* surface, double* x_scale, double* y_scale)
{
#if !HAVE_DLAPI || !defined(SYSTEM_CAIRO)
    cairo_surface_get_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void (*)(cairo_surface_t*, double*, double*)>(
        osl_getAsciiFunctionSymbol(nullptr, "cairo_surface_get_device_scale"));
    if (func)
        func(surface, x_scale, y_scale);
    else
    {
        if (x_scale)
            *x_scale = 1.0;
        if (y_scale)
            *y_scale = 1.0;
    }
#endif
}

basegfx::B2DRange getFillDamage(cairo_t* cr)
{
    double x1, y1, x2, y2;

    // this is faster than cairo_fill_extents, at the cost of some overdraw
    cairo_path_extents(cr, &x1, &y1, &x2, &y2);

    // support B2DRange::isEmpty()
    if (0.0 != x1 || 0.0 != y1 || 0.0 != x2 || 0.0 != y2)
    {
        return basegfx::B2DRange(x1, y1, x2, y2);
    }

    return basegfx::B2DRange();
}

basegfx::B2DRange getClipBox(cairo_t* cr)
{
    double x1, y1, x2, y2;

    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

    // support B2DRange::isEmpty()
    if (0.0 != x1 || 0.0 != y1 || 0.0 != x2 || 0.0 != y2)
    {
        return basegfx::B2DRange(x1, y1, x2, y2);
    }

    return basegfx::B2DRange();
}

basegfx::B2DRange getClippedFillDamage(cairo_t* cr)
{
    basegfx::B2DRange aDamageRect(getFillDamage(cr));
    aDamageRect.intersect(getClipBox(cr));
    return aDamageRect;
}

basegfx::B2DRange getStrokeDamage(cairo_t* cr)
{
    double x1, y1, x2, y2;

    // less accurate, but much faster
    cairo_path_extents(cr, &x1, &y1, &x2, &y2);

    // support B2DRange::isEmpty()
    if (0.0 != x1 || 0.0 != y1 || 0.0 != x2 || 0.0 != y2)
    {
        return basegfx::B2DRange(x1, y1, x2, y2);
    }

    return basegfx::B2DRange();
}

basegfx::B2DRange getClippedStrokeDamage(cairo_t* cr)
{
    basegfx::B2DRange aDamageRect(getStrokeDamage(cr));
    aDamageRect.intersect(getClipBox(cr));
    return aDamageRect;
}

// Remove bClosePath: Checked that the already used mechanism for Win using
// Gdiplus already relies on rPolygon.isClosed(), so should be safe to replace
// this.
// For PixelSnap we need the ObjectToDevice transformation here now. This is a
// special case relative to the also executed LineDraw-Offset of (0.5, 0.5) in
// DeviceCoordinates: The LineDraw-Offset is applied *after* the snap, so we
// need the ObjectToDevice transformation *without* that offset here to do the
// same. The LineDraw-Offset will be applied by the callers using a linear
// transformation for Cairo now
// For support of PixelSnapHairline we also need the ObjectToDevice transformation
// and a method (same as in gdiimpl.cxx for Win and Gdiplus). This is needed e.g.
// for Chart-content visualization. CAUTION: It's not the same as PixelSnap (!)
// tdf#129845 add reply value to allow counting a point/byte/size measurement to
// be included
static size_t AddPolygonToPath(cairo_t* cr, const basegfx::B2DPolygon& rPolygon,
                               const basegfx::B2DHomMatrix& rObjectToDevice, bool bPixelSnap,
                               bool bPixelSnapHairline)
{
    // short circuit if there is nothing to do
    const sal_uInt32 nPointCount(rPolygon.count());
    size_t nSizeMeasure(0);

    if (0 == nPointCount)
    {
        return nSizeMeasure;
    }

    const bool bHasCurves(rPolygon.areControlPointsUsed());
    const bool bClosePath(rPolygon.isClosed());
    const bool bObjectToDeviceUsed(!rObjectToDevice.isIdentity());
    basegfx::B2DHomMatrix aObjectToDeviceInv;
    basegfx::B2DPoint aLast;
    PixelSnapper aSnapper;

    for (sal_uInt32 nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++)
    {
        int nClosedIdx = nPointIdx;
        if (nPointIdx >= nPointCount)
        {
            // prepare to close last curve segment if needed
            if (bClosePath && (nPointIdx == nPointCount))
            {
                nClosedIdx = 0;
            }
            else
            {
                break;
            }
        }

        basegfx::B2DPoint aPoint(rPolygon.getB2DPoint(nClosedIdx));

        if (bPixelSnap)
        {
            // snap device coordinates to full pixels
            if (bObjectToDeviceUsed)
            {
                // go to DeviceCoordinates
                aPoint *= rObjectToDevice;
            }

            // snap by rounding
            aPoint.setX(basegfx::fround(aPoint.getX()));
            aPoint.setY(basegfx::fround(aPoint.getY()));

            if (bObjectToDeviceUsed)
            {
                if (aObjectToDeviceInv.isIdentity())
                {
                    aObjectToDeviceInv = rObjectToDevice;
                    aObjectToDeviceInv.invert();
                }

                // go back to ObjectCoordinates
                aPoint *= aObjectToDeviceInv;
            }
        }

        if (bPixelSnapHairline)
        {
            // snap horizontal and vertical lines (mainly used in Chart for
            // 'nicer' AAing)
            aPoint = aSnapper.snap(rPolygon, rObjectToDevice, aObjectToDeviceInv, nClosedIdx);
        }

        if (!nPointIdx)
        {
            // first point => just move there
            cairo_move_to(cr, aPoint.getX(), aPoint.getY());
            aLast = aPoint;
            continue;
        }

        bool bPendingCurve(false);

        if (bHasCurves)
        {
            bPendingCurve = rPolygon.isNextControlPointUsed(nPrevIdx);
            bPendingCurve |= rPolygon.isPrevControlPointUsed(nClosedIdx);
        }

        if (!bPendingCurve) // line segment
        {
            cairo_line_to(cr, aPoint.getX(), aPoint.getY());
            nSizeMeasure++;
        }
        else // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint(nPrevIdx);
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint(nClosedIdx);

            // tdf#99165 if the control points are 'empty', create the mathematical
            // correct replacement ones to avoid problems with the graphical sub-system
            // tdf#101026 The 1st attempt to create a mathematically correct replacement control
            // vector was wrong. Best alternative is one as close as possible which means short.
            if (aCP1.equal(aLast))
            {
                aCP1 = aLast + ((aCP2 - aLast) * 0.0005);
            }

            if (aCP2.equal(aPoint))
            {
                aCP2 = aPoint + ((aCP1 - aPoint) * 0.0005);
            }

            cairo_curve_to(cr, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(), aPoint.getX(),
                           aPoint.getY());
            // take some bigger measure for curve segments - too expensive to subdivide
            // here and that precision not needed, but four (2 points, 2 control-points)
            // would be a too low weight
            nSizeMeasure += 10;
        }

        aLast = aPoint;
    }

    if (bClosePath)
    {
        cairo_close_path(cr);
    }

    return nSizeMeasure;
}

basegfx::B2DPoint PixelSnapper::snap(const basegfx::B2DPolygon& rPolygon,
                                     const basegfx::B2DHomMatrix& rObjectToDevice,
                                     basegfx::B2DHomMatrix& rObjectToDeviceInv, sal_uInt32 nIndex)
{
    const sal_uInt32 nCount(rPolygon.count());

    // get the data
    if (nIndex == 0)
    {
        // if it's the first time, we need to calculate everything
        maPrevPoint = rObjectToDevice * rPolygon.getB2DPoint((nIndex + nCount - 1) % nCount);
        maCurrPoint = rObjectToDevice * rPolygon.getB2DPoint(nIndex);
        maPrevTuple = basegfx::fround(maPrevPoint);
        maCurrTuple = basegfx::fround(maCurrPoint);
    }
    else
    {
        // but for all other times, we can re-use the previous iteration computations
        maPrevPoint = maCurrPoint;
        maPrevTuple = maCurrTuple;
        maCurrPoint = maNextPoint;
        maCurrTuple = maNextTuple;
    }
    maNextPoint = rObjectToDevice * rPolygon.getB2DPoint((nIndex + 1) % nCount);
    maNextTuple = basegfx::fround(maNextPoint);

    // get the states
    const bool bPrevVertical(maPrevTuple.getX() == maCurrTuple.getX());
    const bool bNextVertical(maNextTuple.getX() == maCurrTuple.getX());
    const bool bPrevHorizontal(maPrevTuple.getY() == maCurrTuple.getY());
    const bool bNextHorizontal(maNextTuple.getY() == maCurrTuple.getY());
    const bool bSnapX(bPrevVertical || bNextVertical);
    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

    if (bSnapX || bSnapY)
    {
        basegfx::B2DPoint aSnappedPoint(bSnapX ? maCurrTuple.getX() : maCurrPoint.getX(),
                                        bSnapY ? maCurrTuple.getY() : maCurrPoint.getY());

        if (rObjectToDeviceInv.isIdentity())
        {
            rObjectToDeviceInv = rObjectToDevice;
            rObjectToDeviceInv.invert();
        }

        aSnappedPoint *= rObjectToDeviceInv;

        return aSnappedPoint;
    }

    return rPolygon.getB2DPoint(nIndex);
}

SystemDependentData_CairoPath::SystemDependentData_CairoPath(size_t nSizeMeasure, cairo_t* cr,
                                                             bool bNoJoin, bool bAntiAlias,
                                                             const std::vector<double>* pStroke)
    : basegfx::SystemDependentData(Application::GetSystemDependentDataManager(),
                                   basegfx::SDD_Type::SDDType_CairoPath)
    , mpCairoPath(nullptr)
    , mbNoJoin(bNoJoin)
    , mbAntiAlias(bAntiAlias)
{
    static const bool bFuzzing = comphelper::IsFuzzing();

    // tdf#129845 only create a copy of the path when nSizeMeasure is
    // bigger than some decent threshold
    if (!bFuzzing && nSizeMeasure > 50)
    {
        mpCairoPath = cairo_copy_path(cr);

        if (nullptr != pStroke)
        {
            maStroke = *pStroke;
        }
    }
}

SystemDependentData_CairoPath::~SystemDependentData_CairoPath()
{
    if (nullptr != mpCairoPath)
    {
        cairo_path_destroy(mpCairoPath);
        mpCairoPath = nullptr;
    }
}

sal_Int64 SystemDependentData_CairoPath::estimateUsageInBytes() const
{
    // tdf#129845 by using the default return value of zero when no path
    // was created, SystemDependentData::calculateCombinedHoldCyclesInSeconds
    // will do the right thing and not buffer this entry at all
    sal_Int64 nRetval(0);

    if (nullptr != mpCairoPath)
    {
        // per node
        // - num_data incarnations of
        // - sizeof(cairo_path_data_t) which is a union of defines and point data
        //   thus may 2 x sizeof(double)
        nRetval = mpCairoPath->num_data * sizeof(cairo_path_data_t);
    }

    return nRetval;
}

void add_polygon_path(cairo_t* cr, const basegfx::B2DPolyPolygon& rPolyPolygon,
                      const basegfx::B2DHomMatrix& rObjectToDevice, bool bPixelSnap)
{
    // try to access buffered data
    std::shared_ptr<SystemDependentData_CairoPath> pSystemDependentData_CairoPath(
        rPolyPolygon.getSystemDependentData<SystemDependentData_CairoPath>(
            basegfx::SDD_Type::SDDType_CairoPath));

    if (pSystemDependentData_CairoPath)
    {
        // re-use data
        cairo_append_path(cr, pSystemDependentData_CairoPath->getCairoPath());
    }
    else
    {
        // create data
        size_t nSizeMeasure(0);

        for (const auto& rPoly : rPolyPolygon)
        {
            // PixelOffset used: Was dependent of 'm_aLineColor != SALCOLOR_NONE'
            // Adapt setupPolyPolygon-users to set a linear transformation to achieve PixelOffset
            nSizeMeasure += AddPolygonToPath(cr, rPoly, rObjectToDevice, bPixelSnap, false);
        }

        // copy and add to buffering mechanism
        // for decisions how/what to buffer, see Note in WinSalGraphicsImpl::drawPolyPolygon
        pSystemDependentData_CairoPath
            = rPolyPolygon.addOrReplaceSystemDependentData<SystemDependentData_CairoPath>(
                nSizeMeasure, cr, false, false, nullptr);
    }
}

cairo_user_data_key_t* CairoCommon::getDamageKey()
{
    static cairo_user_data_key_t aDamageKey;
    return &aDamageKey;
}

sal_uInt16 CairoCommon::GetBitCount() const
{
    if (cairo_surface_get_content(m_pSurface) == CAIRO_CONTENT_ALPHA)
        return 1;
    return 32;
}

cairo_t* CairoCommon::getCairoContext(bool bXorModeAllowed, bool bAntiAlias) const
{
    cairo_t* cr;
    if (m_ePaintMode == PaintMode::Xor && bXorModeAllowed)
        cr = createTmpCompatibleCairoContext();
    else
        cr = cairo_create(m_pSurface);
    cairo_set_line_width(cr, 1);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_antialias(cr, bAntiAlias ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // ensure no linear transformation and no PathInfo in local cairo_path_t
    cairo_identity_matrix(cr);
    cairo_new_path(cr);

    return cr;
}

void CairoCommon::releaseCairoContext(cairo_t* cr, bool bXorModeAllowed,
                                      const basegfx::B2DRange& rExtents) const
{
    const bool bXoring = (m_ePaintMode == PaintMode::Xor && bXorModeAllowed);

    if (rExtents.isEmpty())
    {
        //nothing changed, return early
        if (bXoring)
        {
            cairo_surface_t* surface = cairo_get_target(cr);
            cairo_surface_destroy(surface);
        }
        cairo_destroy(cr);
        return;
    }

    basegfx::B2IRange aIntExtents(basegfx::unotools::b2ISurroundingRangeFromB2DRange(rExtents));
    sal_Int32 nExtentsLeft(aIntExtents.getMinX()), nExtentsTop(aIntExtents.getMinY());
    sal_Int32 nExtentsRight(aIntExtents.getMaxX()), nExtentsBottom(aIntExtents.getMaxY());
    sal_Int32 nWidth = m_aFrameSize.getX();
    sal_Int32 nHeight = m_aFrameSize.getY();
    nExtentsLeft = std::max<sal_Int32>(nExtentsLeft, 0);
    nExtentsTop = std::max<sal_Int32>(nExtentsTop, 0);
    nExtentsRight = std::min<sal_Int32>(nExtentsRight, nWidth);
    nExtentsBottom = std::min<sal_Int32>(nExtentsBottom, nHeight);

    cairo_surface_t* surface = cairo_get_target(cr);
    cairo_surface_flush(surface);

    //For the most part we avoid the use of XOR these days, but there
    //are some edge cases where legacy stuff still supports it, so
    //emulate it (slowly) here.
    if (bXoring)
        doXorOnRelease(nExtentsLeft, nExtentsTop, nExtentsRight, nExtentsBottom, surface, nWidth);

    cairo_destroy(cr); // unref

    DamageHandler* pDamage
        = static_cast<DamageHandler*>(cairo_surface_get_user_data(m_pSurface, getDamageKey()));

    if (pDamage)
    {
        pDamage->damaged(pDamage->handle, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                         nExtentsBottom - nExtentsTop);
    }
}

void CairoCommon::applyFullDamage() const
{
    if (nullptr == m_pSurface)
        return;
    DamageHandler* pDamage
        = static_cast<DamageHandler*>(cairo_surface_get_user_data(m_pSurface, getDamageKey()));
    if (nullptr == pDamage)
        return;
    pDamage->damaged(pDamage->handle, 0, 0, m_aFrameSize.getX(), m_aFrameSize.getY());
}

void CairoCommon::doXorOnRelease(sal_Int32 nExtentsLeft, sal_Int32 nExtentsTop,
                                 sal_Int32 nExtentsRight, sal_Int32 nExtentsBottom,
                                 cairo_surface_t* const surface, sal_Int32 nWidth) const
{
    //For the most part we avoid the use of XOR these days, but there
    //are some edge cases where legacy stuff still supports it, so
    //emulate it (slowly) here.
    cairo_surface_t* target_surface = m_pSurface;
    if (cairo_surface_get_type(target_surface) != CAIRO_SURFACE_TYPE_IMAGE)
    {
        //in the unlikely case we can't use m_pSurface directly, copy contents
        //to another temp image surface
        if (cairo_surface_get_content(m_pSurface) == CAIRO_CONTENT_COLOR_ALPHA)
            target_surface = cairo_surface_map_to_image(target_surface, nullptr);
        else
        {
            // for gen, which is CAIRO_FORMAT_RGB24/CAIRO_CONTENT_COLOR I'm getting
            // visual corruption in vcldemo with cairo_surface_map_to_image
            cairo_t* copycr = createTmpCompatibleCairoContext();
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                            nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, m_pSurface, 0, 0);
            cairo_fill(copycr);
            target_surface = cairo_get_target(copycr);
            cairo_destroy(copycr);
        }
    }

    cairo_surface_flush(target_surface);
    unsigned char* target_surface_data = cairo_image_surface_get_data(target_surface);
    unsigned char* xor_surface_data = cairo_image_surface_get_data(surface);

    cairo_format_t nFormat = cairo_image_surface_get_format(target_surface);
    assert(nFormat == CAIRO_FORMAT_ARGB32 && "need to implement CAIRO_FORMAT_A1 after all here");
    sal_Int32 nStride = cairo_format_stride_for_width(nFormat, nWidth * m_fScale);
    sal_Int32 nUnscaledExtentsLeft = nExtentsLeft * m_fScale;
    sal_Int32 nUnscaledExtentsRight = nExtentsRight * m_fScale;
    sal_Int32 nUnscaledExtentsTop = nExtentsTop * m_fScale;
    sal_Int32 nUnscaledExtentsBottom = nExtentsBottom * m_fScale;

    // Handle headless size forced to (1,1) by SvpSalFrame::GetSurfaceFrameSize().
    int target_surface_width = cairo_image_surface_get_width(target_surface);
    if (nUnscaledExtentsLeft > target_surface_width)
        nUnscaledExtentsLeft = target_surface_width;
    if (nUnscaledExtentsRight > target_surface_width)
        nUnscaledExtentsRight = target_surface_width;
    int target_surface_height = cairo_image_surface_get_height(target_surface);
    if (nUnscaledExtentsTop > target_surface_height)
        nUnscaledExtentsTop = target_surface_height;
    if (nUnscaledExtentsBottom > target_surface_height)
        nUnscaledExtentsBottom = target_surface_height;

#if !ENABLE_WASM_STRIP_PREMULTIPLY
    vcl::bitmap::lookup_table const& unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
    vcl::bitmap::lookup_table const& premultiply_table = vcl::bitmap::get_premultiply_table();
#endif
    for (sal_Int32 y = nUnscaledExtentsTop; y < nUnscaledExtentsBottom; ++y)
    {
        unsigned char* true_row = target_surface_data + (nStride * y);
        unsigned char* xor_row = xor_surface_data + (nStride * y);
        unsigned char* true_data = true_row + (nUnscaledExtentsLeft * 4);
        unsigned char* xor_data = xor_row + (nUnscaledExtentsLeft * 4);
        for (sal_Int32 x = nUnscaledExtentsLeft; x < nUnscaledExtentsRight; ++x)
        {
            sal_uInt8 a = true_data[SVP_CAIRO_ALPHA];
            sal_uInt8 xor_a = xor_data[SVP_CAIRO_ALPHA];
#if ENABLE_WASM_STRIP_PREMULTIPLY
            sal_uInt8 b = vcl::bitmap::unpremultiply(true_data[SVP_CAIRO_BLUE], a)
                          ^ vcl::bitmap::unpremultiply(xor_data[SVP_CAIRO_BLUE], xor_a);
            sal_uInt8 g = vcl::bitmap::unpremultiply(true_data[SVP_CAIRO_GREEN], a)
                          ^ vcl::bitmap::unpremultiply(xor_data[SVP_CAIRO_GREEN], xor_a);
            sal_uInt8 r = vcl::bitmap::unpremultiply(true_data[SVP_CAIRO_RED], a)
                          ^ vcl::bitmap::unpremultiply(xor_data[SVP_CAIRO_RED], xor_a);
            true_data[SVP_CAIRO_BLUE] = vcl::bitmap::premultiply(b, a);
            true_data[SVP_CAIRO_GREEN] = vcl::bitmap::premultiply(g, a);
            true_data[SVP_CAIRO_RED] = vcl::bitmap::premultiply(r, a);
#else
            sal_uInt8 b = unpremultiply_table[a][true_data[SVP_CAIRO_BLUE]]
                          ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_BLUE]];
            sal_uInt8 g = unpremultiply_table[a][true_data[SVP_CAIRO_GREEN]]
                          ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_GREEN]];
            sal_uInt8 r = unpremultiply_table[a][true_data[SVP_CAIRO_RED]]
                          ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_RED]];
            true_data[SVP_CAIRO_BLUE] = premultiply_table[a][b];
            true_data[SVP_CAIRO_GREEN] = premultiply_table[a][g];
            true_data[SVP_CAIRO_RED] = premultiply_table[a][r];
#endif
            true_data += 4;
            xor_data += 4;
        }
    }
    cairo_surface_mark_dirty(target_surface);

    if (target_surface != m_pSurface)
    {
        if (cairo_surface_get_content(m_pSurface) == CAIRO_CONTENT_COLOR_ALPHA)
            cairo_surface_unmap_image(m_pSurface, target_surface);
        else
        {
            cairo_t* copycr = cairo_create(m_pSurface);
            //copy contents back from image surface
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                            nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, target_surface, 0, 0);
            cairo_fill(copycr);
            cairo_destroy(copycr);
            cairo_surface_destroy(target_surface);
        }
    }

    cairo_surface_destroy(surface);
}

cairo_t* CairoCommon::createTmpCompatibleCairoContext() const
{
    cairo_surface_t* target = cairo_surface_create_similar_image(m_pSurface, CAIRO_FORMAT_ARGB32,
                                                                 m_aFrameSize.getX() * m_fScale,
                                                                 m_aFrameSize.getY() * m_fScale);

    dl_cairo_surface_set_device_scale(target, m_fScale, m_fScale);

    return cairo_create(target);
}

void CairoCommon::applyColor(cairo_t* cr, Color aColor, double fTransparency)
{
    if (cairo_surface_get_content(cairo_get_target(cr)) != CAIRO_CONTENT_ALPHA)
    {
        cairo_set_source_rgba(cr, aColor.GetRed() / 255.0, aColor.GetGreen() / 255.0,
                              aColor.GetBlue() / 255.0, 1.0 - fTransparency);
    }
    else
    {
        double fSet = aColor == COL_BLACK ? 1.0 : 0.0;
        cairo_set_source_rgba(cr, 1, 1, 1, fSet);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    }
}

void CairoCommon::clipRegion(cairo_t* cr, const vcl::Region& rClipRegion)
{
    RectangleVector aRectangles;
    if (!rClipRegion.IsEmpty())
    {
        rClipRegion.GetRegionRectangles(aRectangles);
    }
    if (!aRectangles.empty())
    {
        bool bEmpty = true;
        for (auto const& rectangle : aRectangles)
        {
            if (rectangle.GetWidth() <= 0 || rectangle.GetHeight() <= 0)
            {
                SAL_WARN("vcl.gdi", "bad clip rect of: " << rectangle);
                continue;
            }
            cairo_rectangle(cr, rectangle.Left(), rectangle.Top(), rectangle.GetWidth(),
                            rectangle.GetHeight());
            bEmpty = false;
        }
        if (!bEmpty)
            cairo_clip(cr);
    }
}

void CairoCommon::clipRegion(cairo_t* cr) { CairoCommon::clipRegion(cr, m_aClipRegion); }

void CairoCommon::SetXORMode(bool bSet, bool /*bInvertOnly*/)
{
    m_ePaintMode = bSet ? PaintMode::Xor : PaintMode::Over;
}

void CairoCommon::SetROPLineColor(SalROPColor nROPColor)
{
    switch (nROPColor)
    {
        case SalROPColor::N0:
            m_oLineColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
        case SalROPColor::Invert:
            m_oLineColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void CairoCommon::SetROPFillColor(SalROPColor nROPColor)
{
    switch (nROPColor)
    {
        case SalROPColor::N0:
            m_oFillColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
        case SalROPColor::Invert:
            m_oFillColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void CairoCommon::drawPixel(const std::optional<Color>& rLineColor, tools::Long nX, tools::Long nY,
                            bool bAntiAlias)
{
    if (!rLineColor)
        return;

    cairo_t* cr = getCairoContext(true, bAntiAlias);
    clipRegion(cr);

    cairo_rectangle(cr, nX, nY, 1, 1);
    CairoCommon::applyColor(cr, *rLineColor, 0.0);
    cairo_fill(cr);

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    releaseCairoContext(cr, true, extents);
}

Color CairoCommon::getPixel(cairo_surface_t* pSurface, tools::Long nX, tools::Long nY)
{
    cairo_surface_t* target
        = cairo_surface_create_similar_image(pSurface, CAIRO_FORMAT_ARGB32, 1, 1);

    cairo_t* cr = cairo_create(target);

    cairo_rectangle(cr, 0, 0, 1, 1);
    cairo_set_source_surface(cr, pSurface, -nX, -nY);
    cairo_paint(cr);
    cairo_destroy(cr);

    cairo_surface_flush(target);
#if !ENABLE_WASM_STRIP_PREMULTIPLY
    vcl::bitmap::lookup_table const& unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
#endif
    unsigned char* data = cairo_image_surface_get_data(target);
    sal_uInt8 a = data[SVP_CAIRO_ALPHA];
#if ENABLE_WASM_STRIP_PREMULTIPLY
    sal_uInt8 b = vcl::bitmap::unpremultiply(data[SVP_CAIRO_BLUE], a);
    sal_uInt8 g = vcl::bitmap::unpremultiply(data[SVP_CAIRO_GREEN], a);
    sal_uInt8 r = vcl::bitmap::unpremultiply(data[SVP_CAIRO_RED], a);
#else
    sal_uInt8 b = unpremultiply_table[a][data[SVP_CAIRO_BLUE]];
    sal_uInt8 g = unpremultiply_table[a][data[SVP_CAIRO_GREEN]];
    sal_uInt8 r = unpremultiply_table[a][data[SVP_CAIRO_RED]];
#endif
    Color aColor(ColorAlpha, a, r, g, b);
    cairo_surface_destroy(target);

    return aColor;
}

void CairoCommon::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2,
                           bool bAntiAlias)
{
    cairo_t* cr = getCairoContext(false, bAntiAlias);
    clipRegion(cr);

    basegfx::B2DPolygon aPoly;

    // PixelOffset used: To not mix with possible PixelSnap, cannot do
    // directly on coordinates as tried before - despite being already 'snapped'
    // due to being integer. If it would be directly added here, it would be
    // 'snapped' again when !getAntiAlias(), losing the (0.5, 0.5) offset
    aPoly.append(basegfx::B2DPoint(nX1, nY1));
    aPoly.append(basegfx::B2DPoint(nX2, nY2));

    // PixelOffset used: Set PixelOffset as linear transformation
    cairo_matrix_t aMatrix;
    cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
    cairo_set_matrix(cr, &aMatrix);

    AddPolygonToPath(cr, aPoly, basegfx::B2DHomMatrix(), !bAntiAlias, false);

    CairoCommon::applyColor(cr, *m_oLineColor);

    basegfx::B2DRange extents = getClippedStrokeDamage(cr);
    extents.translate(0.5, 0.5);

    cairo_stroke(cr);

    releaseCairoContext(cr, false, extents);
}

// true if we have a fill color and the line color is the same or non-existent
static bool onlyFillRect(const std::optional<Color>& rFillColor,
                         const std::optional<Color>& rLineColor)
{
    if (!rFillColor)
        return false;
    if (!rLineColor)
        return true;
    return *rFillColor == *rLineColor;
}

void CairoCommon::drawRect(double nX, double nY, double nWidth, double nHeight, bool bAntiAlias)
{
    // fast path for the common case of simply creating a solid block of color
    if (onlyFillRect(m_oFillColor, m_oLineColor))
    {
        double fTransparency = 0;
        // don't bother trying to draw stuff which is effectively invisible
        if (nWidth < 0.1 || nHeight < 0.1)
            return;

        cairo_t* cr = getCairoContext(true, bAntiAlias);
        clipRegion(cr);

        bool bPixelSnap = !bAntiAlias;
        if (bPixelSnap)
        {
            // snap by rounding
            nX = basegfx::fround(nX);
            nY = basegfx::fround(nY);
            nWidth = basegfx::fround(nWidth);
            nHeight = basegfx::fround(nHeight);
        }
        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        CairoCommon::applyColor(cr, *m_oFillColor, fTransparency);
        // Get FillDamage
        basegfx::B2DRange extents = getClippedFillDamage(cr);

        cairo_fill(cr);

        releaseCairoContext(cr, true, extents);

        return;
    }
    // because of the -1 hack we have to do fill and draw separately
    std::optional<Color> aOrigFillColor = m_oFillColor;
    std::optional<Color> aOrigLineColor = m_oLineColor;
    m_oFillColor = std::nullopt;
    m_oLineColor = std::nullopt;

    if (aOrigFillColor)
    {
        basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(
            basegfx::B2DRectangle(nX, nY, nX + nWidth, nY + nHeight));

        m_oFillColor = aOrigFillColor;
        drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aRect), 0.0, bAntiAlias);
        m_oFillColor = std::nullopt;
    }

    if (aOrigLineColor)
    {
        // need -1 hack to exclude the bottom and right edges to act like wingdi "Rectangle"
        // function which is what was probably the ultimate origin of this behavior
        basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(
            basegfx::B2DRectangle(nX, nY, nX + nWidth - 1, nY + nHeight - 1));

        m_oLineColor = aOrigLineColor;
        drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aRect), 0.0, bAntiAlias);
        m_oLineColor = std::nullopt;
    }

    m_oFillColor = aOrigFillColor;
    m_oLineColor = aOrigLineColor;
}

void CairoCommon::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry, bool bAntiAlias)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));

    drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aPoly), 0.0, bAntiAlias);
}

void CairoCommon::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPointCounts,
                                  const Point** pPtAry, bool bAntiAlias)
{
    basegfx::B2DPolyPolygon aPolyPoly;
    for (sal_uInt32 nPolygon = 0; nPolygon < nPoly; ++nPolygon)
    {
        sal_uInt32 nPoints = pPointCounts[nPolygon];
        if (nPoints)
        {
            const Point* pPoints = pPtAry[nPolygon];
            basegfx::B2DPolygon aPoly;
            aPoly.append(basegfx::B2DPoint(pPoints->getX(), pPoints->getY()), nPoints);
            for (sal_uInt32 i = 1; i < nPoints; ++i)
                aPoly.setB2DPoint(i, basegfx::B2DPoint(pPoints[i].getX(), pPoints[i].getY()));

            aPolyPoly.append(aPoly);
        }
    }

    drawPolyPolygon(basegfx::B2DHomMatrix(), aPolyPoly, 0.0, bAntiAlias);
}

void CairoCommon::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                  const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency,
                                  bool bAntiAlias)
{
    const bool bHasFill(m_oFillColor.has_value());
    const bool bHasLine(m_oLineColor.has_value());

    if (0 == rPolyPolygon.count() || !(bHasFill || bHasLine) || fTransparency < 0.0
        || fTransparency >= 1.0)
    {
        return;
    }

    if (!bHasLine)
    {
        // don't bother trying to draw stuff which is effectively invisible, speeds up
        // drawing some complex drawings. This optimisation is not valid when we do
        // the pixel offset thing (i.e. bHasLine)
        basegfx::B2DRange aPolygonRange = rPolyPolygon.getB2DRange();
        aPolygonRange.transform(rObjectToDevice);
        if (aPolygonRange.getWidth() < 0.1 || aPolygonRange.getHeight() < 0.1)
            return;
    }

    cairo_t* cr = getCairoContext(true, bAntiAlias);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
    {
        SAL_WARN("vcl.gdi",
                 "cannot render to surface: " << cairo_status_to_string(cairo_status(cr)));
        releaseCairoContext(cr, true, basegfx::B2DRange());
        return;
    }
    clipRegion(cr);

    // Set full (Object-to-Device) transformation - if used
    if (!rObjectToDevice.isIdentity())
    {
        cairo_matrix_t aMatrix;

        cairo_matrix_init(&aMatrix, rObjectToDevice.get(0, 0), rObjectToDevice.get(1, 0),
                          rObjectToDevice.get(0, 1), rObjectToDevice.get(1, 1),
                          rObjectToDevice.get(0, 2), rObjectToDevice.get(1, 2));
        cairo_set_matrix(cr, &aMatrix);
    }

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    if (bHasFill)
    {
        add_polygon_path(cr, rPolyPolygon, rObjectToDevice, !bAntiAlias);

        CairoCommon::applyColor(cr, *m_oFillColor, fTransparency);
        // Get FillDamage (will be extended for LineDamage below)
        extents = getClippedFillDamage(cr);

        cairo_fill(cr);
    }

    if (bHasLine)
    {
        // PixelOffset used: Set PixelOffset as linear transformation
        cairo_matrix_t aMatrix;
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
        cairo_set_matrix(cr, &aMatrix);

        add_polygon_path(cr, rPolyPolygon, rObjectToDevice, !bAntiAlias);

        CairoCommon::applyColor(cr, *m_oLineColor, fTransparency);

        // expand with possible StrokeDamage
        basegfx::B2DRange stroke_extents = getClippedStrokeDamage(cr);
        stroke_extents.translate(0.5, 0.5);
        extents.expand(stroke_extents);

        cairo_stroke(cr);
    }

    // if transformation has been applied, transform also extents (ranges)
    // of damage so they can be correctly redrawn
    extents.transform(rObjectToDevice);
    releaseCairoContext(cr, true, extents);
}

void CairoCommon::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry, bool bAntiAlias)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));
    aPoly.setClosed(false);

    drawPolyLine(basegfx::B2DHomMatrix(), aPoly, 0.0, 1.0, nullptr, basegfx::B2DLineJoin::Miter,
                 css::drawing::LineCap_BUTT, basegfx::deg2rad(15.0) /*default*/, false, bAntiAlias);
}

bool CairoCommon::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                               const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                               double fLineWidth, const std::vector<double>* pStroke,
                               basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap,
                               double fMiterMinimumAngle, bool bPixelSnapHairline, bool bAntiAlias)
{
    // short circuit if there is nothing to do
    if (0 == rPolyLine.count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return true;
    }

    static const bool bFuzzing = comphelper::IsFuzzing();
    if (bFuzzing)
    {
        const basegfx::B2DRange aRange(basegfx::utils::getRange(rPolyLine));
        if (aRange.getMaxX() - aRange.getMinX() > 0x10000000
            || aRange.getMaxY() - aRange.getMinY() > 0x10000000)
        {
            SAL_WARN("vcl.gdi", "drawPolyLine, skipping suspicious range of: "
                                    << aRange << " for fuzzing performance");
            return true;
        }
    }

    cairo_t* cr = getCairoContext(false, bAntiAlias);
    clipRegion(cr);

    // need to check/handle LineWidth when ObjectToDevice transformation is used
    const bool bObjectToDeviceIsIdentity(rObjectToDevice.isIdentity());

    // tdf#124848 calculate-back logical LineWidth for a hairline
    // since this implementation hands over the transformation to
    // the graphic sub-system
    if (fLineWidth == 0)
    {
        fLineWidth = 1.0;

        if (!bObjectToDeviceIsIdentity)
        {
            basegfx::B2DHomMatrix aObjectToDeviceInv(rObjectToDevice);
            aObjectToDeviceInv.invert();
            fLineWidth = (aObjectToDeviceInv * basegfx::B2DVector(fLineWidth, 0)).getLength();
        }
    }

    // PixelOffset used: Need to reflect in linear transformation
    cairo_matrix_t aMatrix;
    basegfx::B2DHomMatrix aDamageMatrix(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));

    if (bObjectToDeviceIsIdentity)
    {
        // Set PixelOffset as requested
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
    }
    else
    {
        // Prepare ObjectToDevice transformation. Take PixelOffset for Lines into
        // account: Multiply from left to act in DeviceCoordinates
        aDamageMatrix = aDamageMatrix * rObjectToDevice;
        cairo_matrix_init(&aMatrix, aDamageMatrix.get(0, 0), aDamageMatrix.get(1, 0),
                          aDamageMatrix.get(0, 1), aDamageMatrix.get(1, 1), aDamageMatrix.get(0, 2),
                          aDamageMatrix.get(1, 2));
    }

    // set linear transformation
    cairo_set_matrix(cr, &aMatrix);

    // setup line attributes
    cairo_line_join_t eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
    switch (eLineJoin)
    {
        case basegfx::B2DLineJoin::Bevel:
            eCairoLineJoin = CAIRO_LINE_JOIN_BEVEL;
            break;
        case basegfx::B2DLineJoin::Round:
            eCairoLineJoin = CAIRO_LINE_JOIN_ROUND;
            break;
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
            eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
            break;
    }

    // convert miter minimum angle to miter limit
    double fMiterLimit = 1.0 / sin(std::max(fMiterMinimumAngle, 0.01 * M_PI) / 2.0);

    // setup cap attribute
    cairo_line_cap_t eCairoLineCap(CAIRO_LINE_CAP_BUTT);

    switch (eLineCap)
    {
        default: // css::drawing::LineCap_BUTT:
        {
            eCairoLineCap = CAIRO_LINE_CAP_BUTT;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            eCairoLineCap = CAIRO_LINE_CAP_ROUND;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            eCairoLineCap = CAIRO_LINE_CAP_SQUARE;
            break;
        }
    }

    cairo_set_source_rgba(cr, m_oLineColor->GetRed() / 255.0, m_oLineColor->GetGreen() / 255.0,
                          m_oLineColor->GetBlue() / 255.0, 1.0 - fTransparency);

    cairo_set_line_join(cr, eCairoLineJoin);
    cairo_set_line_cap(cr, eCairoLineCap);

    constexpr int MaxNormalLineWidthPx = 64;
    if (fLineWidth > MaxNormalLineWidthPx)
    {
        const double fLineWidthPixel
            = bObjectToDeviceIsIdentity
                  ? fLineWidth
                  : (rObjectToDevice * basegfx::B2DVector(fLineWidth, 0)).getLength();
        constexpr double MaxLineWidth = 0x20000000;
        // if the width is pixels is excessive, or if the actual number is huge, then
        // when fuzzing drop it to something small
        if (fLineWidthPixel > MaxNormalLineWidthPx || fLineWidth > MaxLineWidth)
        {
            SAL_WARN("vcl.gdi", "drawPolyLine, suspicious input line width of: "
                                    << fLineWidth << ", will be " << fLineWidthPixel
                                    << " pixels thick");
            if (bFuzzing)
            {
                basegfx::B2DHomMatrix aObjectToDeviceInv(rObjectToDevice);
                aObjectToDeviceInv.invert();
                fLineWidth = (aObjectToDeviceInv * basegfx::B2DVector(MaxNormalLineWidthPx, 0))
                                 .getLength();
                fLineWidth = std::min(fLineWidth, 2048.0);
            }
        }
    }
    cairo_set_line_width(cr, fLineWidth);
    cairo_set_miter_limit(cr, fMiterLimit);

    // try to access buffered data
    std::shared_ptr<SystemDependentData_CairoPath> pSystemDependentData_CairoPath(
        rPolyLine.getSystemDependentData<SystemDependentData_CairoPath>(
            basegfx::SDD_Type::SDDType_CairoPath));

    // MM01 need to do line dashing as fallback stuff here now
    const double fDotDashLength(
        nullptr != pStroke ? std::accumulate(pStroke->begin(), pStroke->end(), 0.0) : 0.0);
    const bool bStrokeUsed(0.0 != fDotDashLength);
    assert(!bStrokeUsed || (bStrokeUsed && pStroke));

    // MM01 activate to stroke directly
    if (bStrokeUsed)
    {
        cairo_set_dash(cr, pStroke->data(), pStroke->size(), 0.0);
    }

    // check for basegfx::B2DLineJoin::NONE to react accordingly
    const bool bNoJoin(basegfx::B2DLineJoin::NONE == eLineJoin && fLineWidth > 0.0
                       && !basegfx::fTools::equalZero(fLineWidth));

    if (pSystemDependentData_CairoPath)
    {
        auto strokeEquals
            = [](const std::vector<double>& rStroke1, const std::vector<double>* pStroke2) -> bool {
            if (!pStroke2)
                return rStroke1.empty();
            return rStroke1 == *pStroke2;
        };
        // check data validity
        if (nullptr == pSystemDependentData_CairoPath->getCairoPath()
            || pSystemDependentData_CairoPath->getNoJoin() != bNoJoin
            || pSystemDependentData_CairoPath->getAntiAlias() != bAntiAlias
            || !strokeEquals(pSystemDependentData_CairoPath->getStroke(), pStroke)
            || bPixelSnapHairline /*tdf#124700*/)
        {
            // data invalid, forget
            pSystemDependentData_CairoPath.reset();
        }
    }

    if (pSystemDependentData_CairoPath)
    {
        // re-use data
        cairo_append_path(cr, pSystemDependentData_CairoPath->getCairoPath());
    }
    else
    {
        // create data
        size_t nSizeMeasure(0);

        // MM01 need to do line dashing as fallback stuff here now
        basegfx::B2DPolyPolygon aPolyPolygonLine;

        // no line dashing or direct stroke, just copy
        aPolyPolygonLine.append(rPolyLine);

        // MM01 checked/verified for Cairo
        for (sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
        {
            const basegfx::B2DPolygon& aPolyLine(aPolyPolygonLine.getB2DPolygon(a));

            if (!bNoJoin)
            {
                // PixelOffset now reflected in linear transformation used
                nSizeMeasure
                    += AddPolygonToPath(cr, aPolyLine,
                                        rObjectToDevice, // ObjectToDevice *without* LineDraw-Offset
                                        !bAntiAlias, bPixelSnapHairline);
            }
            else
            {
                const sal_uInt32 nPointCount(aPolyLine.count());
                const sal_uInt32 nEdgeCount(aPolyLine.isClosed() ? nPointCount : nPointCount - 1);
                basegfx::B2DPolygon aEdge;

                aEdge.append(aPolyLine.getB2DPoint(0));
                aEdge.append(basegfx::B2DPoint(0.0, 0.0));

                for (sal_uInt32 i(0); i < nEdgeCount; i++)
                {
                    const sal_uInt32 nNextIndex((i + 1) % nPointCount);
                    aEdge.setB2DPoint(1, aPolyLine.getB2DPoint(nNextIndex));
                    aEdge.setNextControlPoint(0, aPolyLine.getNextControlPoint(i));
                    aEdge.setPrevControlPoint(1, aPolyLine.getPrevControlPoint(nNextIndex));

                    // PixelOffset now reflected in linear transformation used
                    nSizeMeasure += AddPolygonToPath(
                        cr, aEdge,
                        rObjectToDevice, // ObjectToDevice *without* LineDraw-Offset
                        !bAntiAlias, bPixelSnapHairline);

                    // prepare next step
                    aEdge.setB2DPoint(0, aEdge.getB2DPoint(1));
                }
            }
        }

        // copy and add to buffering mechanism
        if (!bPixelSnapHairline /*tdf#124700*/)
        {
            pSystemDependentData_CairoPath
                = rPolyLine.addOrReplaceSystemDependentData<SystemDependentData_CairoPath>(
                    nSizeMeasure, cr, bNoJoin, bAntiAlias, pStroke);
        }
    }

    // extract extents
    basegfx::B2DRange extents = getClippedStrokeDamage(cr);
    // transform also extents (ranges) of damage so they can be correctly redrawn
    extents.transform(aDamageMatrix);

    // draw and consume
    cairo_stroke(cr);

    releaseCairoContext(cr, false, extents);

    return true;
}

bool CairoCommon::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                tools::Long nHeight, sal_uInt8 nTransparency, bool bAntiAlias)
{
    const bool bHasFill(m_oFillColor.has_value());
    const bool bHasLine(m_oLineColor.has_value());

    if (!bHasFill && !bHasLine)
        return true;

    cairo_t* cr = getCairoContext(false, bAntiAlias);
    clipRegion(cr);

    const double fTransparency = nTransparency * (1.0 / 100);

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    if (bHasFill)
    {
        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        applyColor(cr, *m_oFillColor, fTransparency);

        // set FillDamage
        extents = getClippedFillDamage(cr);

        cairo_fill(cr);
    }

    if (bHasLine)
    {
        // PixelOffset used: Set PixelOffset as linear transformation
        // Note: Was missing here - probably not by purpose (?)
        cairo_matrix_t aMatrix;
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
        cairo_set_matrix(cr, &aMatrix);

        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        applyColor(cr, *m_oLineColor, fTransparency);

        // expand with possible StrokeDamage
        basegfx::B2DRange stroke_extents = getClippedStrokeDamage(cr);
        stroke_extents.translate(0.5, 0.5);
        extents.expand(stroke_extents);

        cairo_stroke(cr);
    }

    releaseCairoContext(cr, false, extents);

    return true;
}

bool CairoCommon::drawGradient(const tools::PolyPolygon& rPolyPolygon, const Gradient& rGradient,
                               bool bAntiAlias)
{
    if (rGradient.GetStyle() != css::awt::GradientStyle_LINEAR
        && rGradient.GetStyle() != css::awt::GradientStyle_RADIAL)
        return false; // unsupported
    if (rGradient.GetSteps() != 0)
        return false; // We can't tell cairo how many colors to use in the gradient.

    cairo_t* cr = getCairoContext(true, bAntiAlias);
    clipRegion(cr);

    tools::Rectangle aInputRect(rPolyPolygon.GetBoundRect());
    if (rPolyPolygon.IsRect())
    {
        // Rect->Polygon conversion loses the right and bottom edge, fix that.
        aInputRect.AdjustRight(1);
        aInputRect.AdjustBottom(1);
        basegfx::B2DHomMatrix rObjectToDevice;
        AddPolygonToPath(cr, tools::Polygon(aInputRect).getB2DPolygon(), rObjectToDevice,
                         !bAntiAlias, false);
    }
    else
    {
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPolygon.getB2DPolyPolygon());
        for (auto const& rPolygon : std::as_const(aB2DPolyPolygon))
        {
            basegfx::B2DHomMatrix rObjectToDevice;
            AddPolygonToPath(cr, rPolygon, rObjectToDevice, !bAntiAlias, false);
        }
    }

    Gradient aGradient(rGradient);

    tools::Rectangle aBoundRect;
    Point aCenter;

    aGradient.SetAngle(aGradient.GetAngle() + 2700_deg10);
    aGradient.GetBoundRect(aInputRect, aBoundRect, aCenter);
    if (aBoundRect.IsEmpty())
        SAL_WARN("vcl.gdi", "empty gradient bounding rectangle");
    else
    {
        Color aStartColor = aGradient.GetStartColor();
        Color aEndColor = aGradient.GetEndColor();

        cairo_pattern_t* pattern;
        if (rGradient.GetStyle() == css::awt::GradientStyle_LINEAR)
        {
            tools::Polygon aPoly(aBoundRect);
            aPoly.Rotate(aCenter, aGradient.GetAngle() % 3600_deg10);
            pattern = cairo_pattern_create_linear(aPoly[0].X(), aPoly[0].Y(), aPoly[1].X(),
                                                  aPoly[1].Y());
        }
        else
        {
            double radius = std::max(aBoundRect.GetWidth() / 2.0, aBoundRect.GetHeight() / 2.0);
            // Move the center a bit to the top-left (the default VCL algorithm is a bit off-center that way,
            // cairo is the opposite way).
            pattern = cairo_pattern_create_radial(aCenter.X() - 0.5, aCenter.Y() - 0.5, 0,
                                                  aCenter.X() - 0.5, aCenter.Y() - 0.5, radius);
            std::swap(aStartColor, aEndColor);
        }

        cairo_pattern_add_color_stop_rgba(
            pattern, aGradient.GetBorder() / 100.0,
            aStartColor.GetRed() * aGradient.GetStartIntensity() / 25500.0,
            aStartColor.GetGreen() * aGradient.GetStartIntensity() / 25500.0,
            aStartColor.GetBlue() * aGradient.GetStartIntensity() / 25500.0, 1.0);

        cairo_pattern_add_color_stop_rgba(
            pattern, 1.0, aEndColor.GetRed() * aGradient.GetEndIntensity() / 25500.0,
            aEndColor.GetGreen() * aGradient.GetEndIntensity() / 25500.0,
            aEndColor.GetBlue() * aGradient.GetEndIntensity() / 25500.0, 1.0);

        cairo_set_source(cr, pattern);
        cairo_pattern_destroy(pattern);
    }

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_fill_preserve(cr);

    releaseCairoContext(cr, true, extents);

    return true;
}

bool CairoCommon::implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                                   SalGradient const& rGradient, bool bAntiAlias)
{
    cairo_t* cr = getCairoContext(true, bAntiAlias);

    basegfx::B2DHomMatrix rObjectToDevice;

    for (auto const& rPolygon : rPolyPolygon)
        AddPolygonToPath(cr, rPolygon, rObjectToDevice, !bAntiAlias, false);

    cairo_pattern_t* pattern
        = cairo_pattern_create_linear(rGradient.maPoint1.getX(), rGradient.maPoint1.getY(),
                                      rGradient.maPoint2.getX(), rGradient.maPoint2.getY());

    for (SalGradientStop const& rStop : rGradient.maStops)
    {
        double r = rStop.maColor.GetRed() / 255.0;
        double g = rStop.maColor.GetGreen() / 255.0;
        double b = rStop.maColor.GetBlue() / 255.0;
        double a = rStop.maColor.GetAlpha() / 255.0;
        double offset = rStop.mfOffset;

        cairo_pattern_add_color_stop_rgba(pattern, offset, r, g, b, a);
    }
    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_fill_preserve(cr);

    releaseCairoContext(cr, true, extents);

    return true;
}

namespace
{
basegfx::B2DRange renderWithOperator(cairo_t* cr, const SalTwoRect& rTR, cairo_surface_t* source,
                                     cairo_operator_t eOperator = CAIRO_OPERATOR_SOURCE)
{
    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    if (rTR.mnSrcWidth != 0 && rTR.mnSrcHeight != 0)
    {
        double fXScale = static_cast<double>(rTR.mnDestWidth) / rTR.mnSrcWidth;
        double fYScale = static_cast<double>(rTR.mnDestHeight) / rTR.mnSrcHeight;
        cairo_scale(cr, fXScale, fYScale);
    }

    cairo_save(cr);
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);

    if (cairo_status(cr) == CAIRO_STATUS_SUCCESS)
    {
        //tdf#133716 borders of upscaled images should not be blurred
        cairo_pattern_t* sourcepattern = cairo_get_source(cr);
        cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_PAD);
    }

    cairo_set_operator(cr, eOperator);
    cairo_paint(cr);
    cairo_restore(cr);

    return extents;
}

} // end anonymous ns

basegfx::B2DRange CairoCommon::renderSource(cairo_t* cr, const SalTwoRect& rTR,
                                            cairo_surface_t* source)
{
    return renderWithOperator(cr, rTR, source, CAIRO_OPERATOR_SOURCE);
}

void CairoCommon::copyWithOperator(const SalTwoRect& rTR, cairo_surface_t* source,
                                   cairo_operator_t eOp, bool bAntiAlias)
{
    cairo_t* cr = getCairoContext(false, bAntiAlias);
    clipRegion(cr);

    basegfx::B2DRange extents = renderWithOperator(cr, rTR, source, eOp);

    releaseCairoContext(cr, false, extents);
}

void CairoCommon::copySource(const SalTwoRect& rTR, cairo_surface_t* source, bool bAntiAlias)
{
    copyWithOperator(rTR, source, CAIRO_OPERATOR_SOURCE, bAntiAlias);
}

void CairoCommon::copyBitsCairo(const SalTwoRect& rTR, cairo_surface_t* pSourceSurface,
                                bool bAntiAlias)
{
    SalTwoRect aTR(rTR);

    cairo_surface_t* pCopy = nullptr;

    if (pSourceSurface == getSurface())
    {
        //self copy is a problem, so dup source in that case
        pCopy
            = cairo_surface_create_similar(pSourceSurface, cairo_surface_get_content(getSurface()),
                                           aTR.mnSrcWidth * m_fScale, aTR.mnSrcHeight * m_fScale);
        dl_cairo_surface_set_device_scale(pCopy, m_fScale, m_fScale);
        cairo_t* cr = cairo_create(pCopy);
        cairo_set_source_surface(cr, pSourceSurface, -aTR.mnSrcX, -aTR.mnSrcY);
        cairo_rectangle(cr, 0, 0, aTR.mnSrcWidth, aTR.mnSrcHeight);
        cairo_fill(cr);
        cairo_destroy(cr);

        pSourceSurface = pCopy;

        aTR.mnSrcX = 0;
        aTR.mnSrcY = 0;
    }

    copySource(aTR, pSourceSurface, bAntiAlias);

    if (pCopy)
        cairo_surface_destroy(pCopy);
}

namespace
{
cairo_pattern_t* create_stipple()
{
    static unsigned char data[16] = { 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
                                      0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF };
    cairo_surface_t* surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_A8, 4, 4, 4);
    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surface);
    cairo_surface_destroy(surface);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
    cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);
    return pattern;
}
} // end anonymous ns

void CairoCommon::invert(const basegfx::B2DPolygon& rPoly, SalInvert nFlags, bool bAntiAlias)
{
    cairo_t* cr = getCairoContext(false, bAntiAlias);
    clipRegion(cr);

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    AddPolygonToPath(cr, rPoly, basegfx::B2DHomMatrix(), !bAntiAlias, false);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);

    if (nFlags & SalInvert::TrackFrame)
    {
        cairo_set_line_width(cr, 2.0);
        const double dashLengths[2] = { 4.0, 4.0 };
        cairo_set_dash(cr, dashLengths, 2, 0);

        extents = getClippedStrokeDamage(cr);
        //see tdf#106577 under wayland, some pixel droppings seen, maybe we're
        //out by one somewhere, or cairo_stroke_extents is confused by
        //dashes/line width
        if (!extents.isEmpty())
        {
            extents.grow(1);
        }

        cairo_stroke(cr);
    }
    else
    {
        extents = getClippedFillDamage(cr);

        cairo_clip(cr);

        if (nFlags & SalInvert::N50)
        {
            cairo_pattern_t* pattern = create_stipple();
            cairo_surface_t* surface = cairo_surface_create_similar(
                m_pSurface, cairo_surface_get_content(m_pSurface), extents.getWidth() * m_fScale,
                extents.getHeight() * m_fScale);

            dl_cairo_surface_set_device_scale(surface, m_fScale, m_fScale);
            cairo_t* stipple_cr = cairo_create(surface);
            cairo_set_source_rgb(stipple_cr, 1.0, 1.0, 1.0);
            cairo_mask(stipple_cr, pattern);
            cairo_pattern_destroy(pattern);
            cairo_destroy(stipple_cr);
            cairo_mask_surface(cr, surface, extents.getMinX(), extents.getMinY());
            cairo_surface_destroy(surface);
        }
        else
        {
            cairo_paint(cr);
        }
    }

    releaseCairoContext(cr, false, extents);
}

void CairoCommon::invert(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                         SalInvert nFlags, bool bAntiAlias)
{
    basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(
        basegfx::B2DRectangle(nX, nY, nX + nWidth, nY + nHeight));

    invert(aRect, nFlags, bAntiAlias);
}

void CairoCommon::invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags, bool bAntiAlias)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));
    aPoly.setClosed(true);

    invert(aPoly, nFlags, bAntiAlias);
}

void CairoCommon::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                             bool bAntiAlias)
{
    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSalBitmap, aSurface);
    cairo_surface_t* source = aSurface->getSurface(rPosAry.mnDestWidth, rPosAry.mnDestHeight);

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return;
    }

#if 0 // LO code is not yet bitmap32-ready.
    // if m_bSupportsBitmap32 becomes true for Svp revisit this
    copyWithOperator(rPosAry, source, CAIRO_OPERATOR_OVER, bAntiAlias);
#else
    copyWithOperator(rPosAry, source, CAIRO_OPERATOR_SOURCE, bAntiAlias);
#endif
}

bool CairoCommon::drawAlphaBitmap(const SalTwoRect& rTR, const SalBitmap& rSourceBitmap,
                                  const SalBitmap& rAlphaBitmap, bool bAntiAlias)
{
    if (rAlphaBitmap.GetBitCount() != 8 && rAlphaBitmap.GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap alpha depth case: "
                                << rAlphaBitmap.GetBitCount());
        return false;
    }

    if (!rTR.mnSrcWidth || !rTR.mnSrcHeight)
    {
        SAL_WARN("vcl.gdi", "not possible to stretch nothing");
        return true;
    }

    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSourceBitmap, aSurface);
    cairo_surface_t* source = aSurface->getSurface(rTR.mnDestWidth, rTR.mnDestHeight);

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    // MM02 try to access buffered MaskHelper
    std::shared_ptr<MaskHelper> aMask;
    tryToUseMaskBuffer(rAlphaBitmap, aMask);
    cairo_surface_t* mask = aMask->getSurface(rTR.mnDestWidth, rTR.mnDestHeight);

    if (!mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    cairo_t* cr = getCairoContext(false, bAntiAlias);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
    {
        SAL_WARN("vcl.gdi",
                 "cannot render to surface: " << cairo_status_to_string(cairo_status(cr)));
        releaseCairoContext(cr, false, basegfx::B2DRange());
        return true;
    }

    clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_pattern_t* maskpattern = cairo_pattern_create_for_surface(mask);
    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    double fXScale = static_cast<double>(rTR.mnDestWidth) / rTR.mnSrcWidth;
    double fYScale = static_cast<double>(rTR.mnDestHeight) / rTR.mnSrcHeight;
    cairo_scale(cr, fXScale, fYScale);
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);

    cairo_pattern_t* sourcepattern = cairo_get_source(cr);

    //tdf#133716 borders of upscaled images should not be blurred
    //tdf#114117 when stretching a single or multi pixel width/height source to fit an area
    //the image will be extended into that size.
    cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_PAD);
    cairo_pattern_set_extend(maskpattern, CAIRO_EXTEND_PAD);

    //this block is just "cairo_mask_surface", but we have to make it explicit
    //because of the cairo_pattern_set_filter etc we may want applied
    cairo_matrix_t matrix;
    cairo_matrix_init_translate(&matrix, rTR.mnSrcX, rTR.mnSrcY);
    cairo_pattern_set_matrix(maskpattern, &matrix);
    cairo_mask(cr, maskpattern);

    cairo_pattern_destroy(maskpattern);

    releaseCairoContext(cr, false, extents);

    return true;
}

bool CairoCommon::drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                                        const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                                        const SalBitmap* pAlphaBitmap, double fAlpha,
                                        bool bAntiAlias)
{
    if (pAlphaBitmap && pAlphaBitmap->GetBitCount() != 8 && pAlphaBitmap->GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap alpha depth case: "
                                << pAlphaBitmap->GetBitCount());
        return false;
    }

    if (fAlpha != 1.0)
        return false;

    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSourceBitmap, aSurface);
    const tools::Long nDestWidth(basegfx::fround(basegfx::B2DVector(rX - rNull).getLength()));
    const tools::Long nDestHeight(basegfx::fround(basegfx::B2DVector(rY - rNull).getLength()));
    cairo_surface_t* source(aSurface->getSurface(nDestWidth, nDestHeight));

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
        return false;
    }

    // MM02 try to access buffered MaskHelper
    std::shared_ptr<MaskHelper> aMask;
    if (nullptr != pAlphaBitmap)
    {
        tryToUseMaskBuffer(*pAlphaBitmap, aMask);
    }

    // access cairo_surface_t from MaskHelper
    cairo_surface_t* mask(nullptr);
    if (aMask)
    {
        mask = aMask->getSurface(nDestWidth, nDestHeight);
    }

    if (nullptr != pAlphaBitmap && nullptr == mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
        return false;
    }

    const Size aSize = rSourceBitmap.GetSize();
    cairo_t* cr = getCairoContext(false, bAntiAlias);
    clipRegion(cr);

    // setup the image transformation
    // using the rNull,rX,rY points as destinations for the (0,0),(0,Width),(Height,0) source points
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, aXRel.getX() / aSize.Width(), aXRel.getY() / aSize.Width(),
                      aYRel.getX() / aSize.Height(), aYRel.getY() / aSize.Height(), rNull.getX(),
                      rNull.getY());

    cairo_transform(cr, &matrix);

    cairo_rectangle(cr, 0, 0, aSize.Width(), aSize.Height());
    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_clip(cr);

    cairo_set_source_surface(cr, source, 0, 0);
    if (mask)
        cairo_mask_surface(cr, mask, 0, 0);
    else
        cairo_paint(cr);

    releaseCairoContext(cr, false, extents);

    return true;
}

void CairoCommon::drawMask(const SalTwoRect& rTR, const SalBitmap& rSalBitmap, Color nMaskColor,
                           bool bAntiAlias)
{
    /** creates an image from the given rectangle, replacing all black pixels
     *  with nMaskColor and make all other full transparent */
    // MM02 here decided *against* using buffered BitmapHelper
    // because the data gets somehow 'unmuliplied'. This may also be
    // done just once, but I am not sure if this is safe to do.
    // So for now dispense re-using data here.
    BitmapHelper aSurface(rSalBitmap, true); // The mask is argb32
    if (!aSurface.getSurface())
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawMask case");
        return;
    }
    sal_Int32 nStride;
    unsigned char* mask_data = aSurface.getBits(nStride);
    for (tools::Long y = rTR.mnSrcY; y < rTR.mnSrcY + rTR.mnSrcHeight; ++y)
    {
        unsigned char* row = mask_data + (nStride * y);
        unsigned char* data = row + (rTR.mnSrcX * 4);
        for (tools::Long x = rTR.mnSrcX; x < rTR.mnSrcX + rTR.mnSrcWidth; ++x)
        {
            // Don't need to unpremultiply the data here, because we are checking for zero,
            // and that is the same multiplied or un-multiplied.
            if (data[SVP_CAIRO_RED] == 0 && data[SVP_CAIRO_GREEN] == 0 && data[SVP_CAIRO_BLUE] == 0)
            {
                data[0] = nMaskColor.GetBlue();
                data[1] = nMaskColor.GetGreen();
                data[2] = nMaskColor.GetRed();
                data[3] = 0xff;
            }
            else
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
                data[3] = 0;
            }
            data += 4;
        }
    }
    aSurface.mark_dirty();

    cairo_t* cr = getCairoContext(false, bAntiAlias);
    clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    double fXScale = static_cast<double>(rTR.mnDestWidth) / rTR.mnSrcWidth;
    double fYScale = static_cast<double>(rTR.mnDestHeight) / rTR.mnSrcHeight;
    cairo_scale(cr, fXScale, fYScale);
    cairo_set_source_surface(cr, aSurface.getSurface(), -rTR.mnSrcX, -rTR.mnSrcY);

    if (cairo_status(cr) == CAIRO_STATUS_SUCCESS)
    {
        //tdf#133716 borders of upscaled images should not be blurred
        cairo_pattern_t* sourcepattern = cairo_get_source(cr);
        cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_PAD);
    }

    cairo_paint(cr);

    releaseCairoContext(cr, false, extents);
}

std::shared_ptr<SalBitmap> CairoCommon::getBitmap(tools::Long nX, tools::Long nY,
                                                  tools::Long nWidth, tools::Long nHeight)
{
    std::shared_ptr<SvpSalBitmap> pBitmap = std::make_shared<SvpSalBitmap>();
    BitmapPalette aPal;
    assert(GetBitCount() != 1 && "not supported anymore");
    vcl::PixelFormat ePixelFormat = vcl::PixelFormat::N32_BPP;

    if (!pBitmap->ImplCreate(Size(nWidth, nHeight), ePixelFormat, aPal, false))
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create bitmap");
        return nullptr;
    }

    cairo_surface_t* target = CairoCommon::createCairoSurface(pBitmap->GetBuffer());
    if (!target)
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create cairo surface");
        return nullptr;
    }
    cairo_t* cr = cairo_create(target);

    SalTwoRect aTR(nX, nY, nWidth, nHeight, 0, 0, nWidth, nHeight);
    CairoCommon::renderSource(cr, aTR, m_pSurface);

    cairo_destroy(cr);
    cairo_surface_destroy(target);

    return pBitmap;
}

cairo_format_t getCairoFormat(const BitmapBuffer& rBuffer)
{
    cairo_format_t nFormat;
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
    assert(rBuffer.mnBitCount == 32 || rBuffer.mnBitCount == 24 || rBuffer.mnBitCount == 1);
#else
    assert(rBuffer.mnBitCount == 32 || rBuffer.mnBitCount == 1);
#endif

    if (rBuffer.mnBitCount == 32)
        nFormat = CAIRO_FORMAT_ARGB32;
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
    else if (rBuffer.mnBitCount == 24)
        nFormat = CAIRO_FORMAT_RGB24_888;
#endif
    else
        nFormat = CAIRO_FORMAT_A1;
    return nFormat;
}

namespace
{
bool isCairoCompatible(const BitmapBuffer* pBuffer)
{
    if (!pBuffer)
        return false;

        // We use Cairo that supports 24-bit RGB.
#ifdef HAVE_CAIRO_FORMAT_RGB24_888
    if (pBuffer->mnBitCount != 32 && pBuffer->mnBitCount != 24 && pBuffer->mnBitCount != 1)
#else
    if (pBuffer->mnBitCount != 32 && pBuffer->mnBitCount != 1)
#endif
        return false;

    cairo_format_t nFormat = getCairoFormat(*pBuffer);
    return (cairo_format_stride_for_width(nFormat, pBuffer->mnWidth) == pBuffer->mnScanlineSize);
}
}

cairo_surface_t* CairoCommon::createCairoSurface(const BitmapBuffer* pBuffer)
{
    if (!isCairoCompatible(pBuffer))
        return nullptr;

    cairo_format_t nFormat = getCairoFormat(*pBuffer);
    cairo_surface_t* target = cairo_image_surface_create_for_data(
        pBuffer->mpBits, nFormat, pBuffer->mnWidth, pBuffer->mnHeight, pBuffer->mnScanlineSize);
    if (cairo_surface_status(target) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(target);
        return nullptr;
    }
    return target;
}

bool CairoCommon::supportsOperation(OutDevSupportType eType)
{
    switch (eType)
    {
        case OutDevSupportType::TransparentText:
            return true;
    }
    return false;
}

std::optional<BitmapBuffer> FastConvert24BitRgbTo32BitCairo(const BitmapBuffer* pSrc)
{
    if (pSrc == nullptr)
        return std::nullopt;

    assert(pSrc->meFormat == SVP_24BIT_FORMAT);
    const tools::Long nWidth = pSrc->mnWidth;
    const tools::Long nHeight = pSrc->mnHeight;
    std::optional<BitmapBuffer> pDst(std::in_place);
    pDst->meFormat = ScanlineFormat::N32BitTcArgb;
    pDst->meDirection = ScanlineDirection::TopDown;
    pDst->mnWidth = nWidth;
    pDst->mnHeight = nHeight;
    pDst->mnBitCount = 32;
    pDst->maPalette = pSrc->maPalette;

    tools::Long nScanlineBase;
    const bool bFail = o3tl::checked_multiply<tools::Long>(pDst->mnBitCount, nWidth, nScanlineBase);
    if (bFail)
    {
        SAL_WARN("vcl.gdi", "checked multiply failed");
        pDst->mpBits = nullptr;
        return std::nullopt;
    }

    pDst->mnScanlineSize = AlignedWidth4Bytes(nScanlineBase);
    if (pDst->mnScanlineSize < nScanlineBase / 8)
    {
        SAL_WARN("vcl.gdi", "scanline calculation wraparound");
        pDst->mpBits = nullptr;
        return std::nullopt;
    }

    try
    {
        pDst->mpBits = new sal_uInt8[pDst->mnScanlineSize * nHeight];
    }
    catch (const std::bad_alloc&)
    {
        // memory exception, clean up
        pDst->mpBits = nullptr;
        return std::nullopt;
    }

    for (tools::Long y = 0; y < nHeight; ++y)
    {
        sal_uInt8* pS = pSrc->mpBits + y * pSrc->mnScanlineSize;
        sal_uInt8* pD = pDst->mpBits + y * pDst->mnScanlineSize;
        for (tools::Long x = 0; x < nWidth; ++x)
        {
#if ENABLE_CAIRO_RGBA
            static_assert(SVP_CAIRO_FORMAT == ScanlineFormat::N32BitTcRgbx,
                          "Expected SVP_CAIRO_FORMAT set to N32BitTcRgbx");
            static_assert(SVP_24BIT_FORMAT == ScanlineFormat::N24BitTcRgb,
                          "Expected SVP_24BIT_FORMAT set to N24BitTcRgb");
            pD[0] = pS[0];
            pD[1] = pS[1];
            pD[2] = pS[2];
            pD[3] = 0xff; // Alpha
#elif defined OSL_BIGENDIAN
            static_assert(SVP_CAIRO_FORMAT == ScanlineFormat::N32BitTcXrgb,
                          "Expected SVP_CAIRO_FORMAT set to N32BitTcXrgb");
            static_assert(SVP_24BIT_FORMAT == ScanlineFormat::N24BitTcRgb,
                          "Expected SVP_24BIT_FORMAT set to N24BitTcRgb");
            pD[0] = 0xff; // Alpha
            pD[1] = pS[0];
            pD[2] = pS[1];
            pD[3] = pS[2];
#else
            static_assert(SVP_CAIRO_FORMAT == ScanlineFormat::N32BitTcBgrx,
                          "Expected SVP_CAIRO_FORMAT set to N32BitTcBgrx");
            static_assert(SVP_24BIT_FORMAT == ScanlineFormat::N24BitTcBgr,
                          "Expected SVP_24BIT_FORMAT set to N24BitTcBgr");
            pD[0] = pS[0];
            pD[1] = pS[1];
            pD[2] = pS[2];
            pD[3] = 0xff; // Alpha
#endif

            pS += 3;
            pD += 4;
        }
    }

    return pDst;
}

namespace
{
// check for env var that decides for using downscale pattern
const char* pDisableDownScale(getenv("SAL_DISABLE_CAIRO_DOWNSCALE"));
bool bDisableDownScale(nullptr != pDisableDownScale);
}

cairo_surface_t* SurfaceHelper::implCreateOrReuseDownscale(unsigned long nTargetWidth,
                                                           unsigned long nTargetHeight)
{
    const unsigned long nSourceWidth(cairo_image_surface_get_width(pSurface));
    const unsigned long nSourceHeight(cairo_image_surface_get_height(pSurface));

    // zoomed in, need to stretch at paint, no pre-scale useful
    if (nTargetWidth >= nSourceWidth || nTargetHeight >= nSourceHeight)
    {
        return pSurface;
    }

    // calculate downscale factor
    unsigned long nWFactor(1);
    unsigned long nW((nSourceWidth + 1) / 2);
    unsigned long nHFactor(1);
    unsigned long nH((nSourceHeight + 1) / 2);

    while (nW > nTargetWidth && nW > 1)
    {
        nW = (nW + 1) / 2;
        nWFactor *= 2;
    }

    while (nH > nTargetHeight && nH > 1)
    {
        nH = (nH + 1) / 2;
        nHFactor *= 2;
    }

    if (1 == nWFactor && 1 == nHFactor)
    {
        // original size *is* best binary size, use it
        return pSurface;
    }

    // go up one scale again - look for no change
    nW = (1 == nWFactor) ? nTargetWidth : nW * 2;
    nH = (1 == nHFactor) ? nTargetHeight : nH * 2;

    // check if we have a downscaled version of required size
    // bail out if the multiplication for the key would overflow
    if (nW >= SAL_MAX_UINT32 || nH >= SAL_MAX_UINT32)
        return pSurface;
    const sal_uInt64 key((nW * static_cast<sal_uInt64>(SAL_MAX_UINT32)) + nH);
    auto isHit(maDownscaled.find(key));

    if (isHit != maDownscaled.end())
    {
        return isHit->second;
    }

    // create new surface in the targeted size
    cairo_surface_t* pSurfaceTarget
        = cairo_surface_create_similar(pSurface, cairo_surface_get_content(pSurface), nW, nH);

    // made a version to scale self first that worked well, but would've
    // been hard to support CAIRO_FORMAT_A1 including bit shifting, so
    // I decided to go with cairo itself - use CAIRO_FILTER_FAST or
    // CAIRO_FILTER_GOOD though. Please modify as needed for
    // performance/quality
    cairo_t* cr = cairo_create(pSurfaceTarget);
    const double fScaleX(static_cast<double>(nW) / static_cast<double>(nSourceWidth));
    const double fScaleY(static_cast<double>(nH) / static_cast<double>(nSourceHeight));
    cairo_scale(cr, fScaleX, fScaleY);
    cairo_set_source_surface(cr, pSurface, 0.0, 0.0);
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_GOOD);
    cairo_paint(cr);
    cairo_destroy(cr);

    // need to set device_scale for downscale surfaces to get
    // them handled correctly
    cairo_surface_set_device_scale(pSurfaceTarget, fScaleX, fScaleY);

    // add entry to cached entries
    maDownscaled[key] = pSurfaceTarget;

    return pSurfaceTarget;
}

bool SurfaceHelper::isTrivial() const
{
    constexpr unsigned long nMinimalSquareSizeToBuffer(64 * 64);
    const unsigned long nSourceWidth(cairo_image_surface_get_width(pSurface));
    const unsigned long nSourceHeight(cairo_image_surface_get_height(pSurface));

    return nSourceWidth * nSourceHeight < nMinimalSquareSizeToBuffer;
}

SurfaceHelper::SurfaceHelper()
    : pSurface(nullptr)
{
}

SurfaceHelper::~SurfaceHelper()
{
    cairo_surface_destroy(pSurface);
    for (auto& candidate : maDownscaled)
    {
        cairo_surface_destroy(candidate.second);
    }
}

cairo_surface_t* SurfaceHelper::getSurface(unsigned long nTargetWidth,
                                           unsigned long nTargetHeight) const
{
    if (bDisableDownScale || 0 == nTargetWidth || 0 == nTargetHeight || !pSurface || isTrivial())
    {
        // caller asks for original or disabled or trivial (smaller then a minimal square size)
        // also excludes zero cases for width/height after this point if need to prescale
        return pSurface;
    }

    return const_cast<SurfaceHelper*>(this)->implCreateOrReuseDownscale(nTargetWidth,
                                                                        nTargetHeight);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
