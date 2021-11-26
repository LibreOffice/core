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

#include <headless/CairoCommon.hxx>
#include <dlfcn.h>
#include <vcl/BitmapTools.hxx>
#include <basegfx/utils/canvastools.hxx>

void dl_cairo_surface_set_device_scale(cairo_surface_t* surface, double x_scale, double y_scale)
{
#ifdef ANDROID
    cairo_surface_set_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void (*)(cairo_surface_t*, double, double)>(
        dlsym(nullptr, "cairo_surface_set_device_scale"));
    if (func)
        func(surface, x_scale, y_scale);
#endif
}

void dl_cairo_surface_get_device_scale(cairo_surface_t* surface, double* x_scale, double* y_scale)
{
#ifdef ANDROID
    cairo_surface_get_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void (*)(cairo_surface_t*, double*, double*)>(
        dlsym(nullptr, "cairo_surface_get_device_scale"));
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
size_t AddPolygonToPath(cairo_t* cr, const basegfx::B2DPolygon& rPolygon,
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
            aPoint = impPixelSnap(rPolygon, rObjectToDevice, aObjectToDeviceInv, nClosedIdx);
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

basegfx::B2DPoint impPixelSnap(const basegfx::B2DPolygon& rPolygon,
                               const basegfx::B2DHomMatrix& rObjectToDevice,
                               basegfx::B2DHomMatrix& rObjectToDeviceInv, sal_uInt32 nIndex)
{
    const sal_uInt32 nCount(rPolygon.count());

    // get the data
    const basegfx::B2ITuple aPrevTuple(
        basegfx::fround(rObjectToDevice * rPolygon.getB2DPoint((nIndex + nCount - 1) % nCount)));
    const basegfx::B2DPoint aCurrPoint(rObjectToDevice * rPolygon.getB2DPoint(nIndex));
    const basegfx::B2ITuple aCurrTuple(basegfx::fround(aCurrPoint));
    const basegfx::B2ITuple aNextTuple(
        basegfx::fround(rObjectToDevice * rPolygon.getB2DPoint((nIndex + 1) % nCount)));

    // get the states
    const bool bPrevVertical(aPrevTuple.getX() == aCurrTuple.getX());
    const bool bNextVertical(aNextTuple.getX() == aCurrTuple.getX());
    const bool bPrevHorizontal(aPrevTuple.getY() == aCurrTuple.getY());
    const bool bNextHorizontal(aNextTuple.getY() == aCurrTuple.getY());
    const bool bSnapX(bPrevVertical || bNextVertical);
    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

    if (bSnapX || bSnapY)
    {
        basegfx::B2DPoint aSnappedPoint(bSnapX ? aCurrTuple.getX() : aCurrPoint.getX(),
                                        bSnapY ? aCurrTuple.getY() : aCurrPoint.getY());

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

cairo_user_data_key_t* CairoCommon::getDamageKey()
{
    static cairo_user_data_key_t aDamageKey;
    return &aDamageKey;
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
    {
        cairo_surface_t* target_surface = m_pSurface;
        if (cairo_surface_get_type(target_surface) != CAIRO_SURFACE_TYPE_IMAGE)
        {
            //in the unlikely case we can't use m_pSurface directly, copy contents
            //to another temp image surface
            cairo_t* copycr = createTmpCompatibleCairoContext();
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                            nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, m_pSurface, 0, 0);
            cairo_paint(copycr);
            target_surface = cairo_get_target(copycr);
            cairo_destroy(copycr);
        }

        cairo_surface_flush(target_surface);
        unsigned char* target_surface_data = cairo_image_surface_get_data(target_surface);
        unsigned char* xor_surface_data = cairo_image_surface_get_data(surface);

        cairo_format_t nFormat = cairo_image_surface_get_format(target_surface);
        assert(nFormat == CAIRO_FORMAT_ARGB32
               && "need to implement CAIRO_FORMAT_A1 after all here");
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

        vcl::bitmap::lookup_table const& unpremultiply_table
            = vcl::bitmap::get_unpremultiply_table();
        vcl::bitmap::lookup_table const& premultiply_table = vcl::bitmap::get_premultiply_table();
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
                sal_uInt8 b = unpremultiply_table[a][true_data[SVP_CAIRO_BLUE]]
                              ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_BLUE]];
                sal_uInt8 g = unpremultiply_table[a][true_data[SVP_CAIRO_GREEN]]
                              ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_GREEN]];
                sal_uInt8 r = unpremultiply_table[a][true_data[SVP_CAIRO_RED]]
                              ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_RED]];
                true_data[SVP_CAIRO_BLUE] = premultiply_table[a][b];
                true_data[SVP_CAIRO_GREEN] = premultiply_table[a][g];
                true_data[SVP_CAIRO_RED] = premultiply_table[a][r];
                true_data += 4;
                xor_data += 4;
            }
        }
        cairo_surface_mark_dirty(target_surface);

        if (target_surface != m_pSurface)
        {
            cairo_t* copycr = cairo_create(m_pSurface);
            //unlikely case we couldn't use m_pSurface directly, copy contents
            //back from image surface
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                            nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, target_surface, 0, 0);
            cairo_paint(copycr);
            cairo_destroy(copycr);
            cairo_surface_destroy(target_surface);
        }

        cairo_surface_destroy(surface);
    }

    cairo_destroy(cr); // unref

    DamageHandler* pDamage
        = static_cast<DamageHandler*>(cairo_surface_get_user_data(m_pSurface, getDamageKey()));

    if (pDamage)
    {
        pDamage->damaged(pDamage->handle, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                         nExtentsBottom - nExtentsTop);
    }
}

cairo_t* CairoCommon::createTmpCompatibleCairoContext() const
{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    cairo_surface_t* target = cairo_surface_create_similar_image(
        m_pSurface,
#else
    cairo_surface_t* target = cairo_image_surface_create(
#endif
        CAIRO_FORMAT_ARGB32, m_aFrameSize.getX() * m_fScale, m_aFrameSize.getY() * m_fScale);

    dl_cairo_surface_set_device_scale(target, m_fScale, m_fScale);

    return cairo_create(target);
}

void CairoCommon::applyColor(cairo_t* cr, Color aColor, double fTransparency)
{
    if (cairo_surface_get_content(m_pSurface) == CAIRO_CONTENT_COLOR_ALPHA)
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
        for (auto const& rectangle : aRectangles)
        {
            cairo_rectangle(cr, rectangle.Left(), rectangle.Top(), rectangle.GetWidth(),
                            rectangle.GetHeight());
        }
        cairo_clip(cr);
    }
}

void CairoCommon::clipRegion(cairo_t* cr) { CairoCommon::clipRegion(cr, m_aClipRegion); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
