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

#include "X11CairoSalGraphicsImpl.hxx"

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

X11CairoSalGraphicsImpl::X11CairoSalGraphicsImpl(X11SalGraphics& rParent, CairoCommon& rCairoCommon)
    : X11SalGraphicsImpl(rParent)
    , mrCairoCommon(rCairoCommon)
{
}

void X11CairoSalGraphicsImpl::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                       tools::Long nHeight)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(true, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    CairoCommon::drawRect(cr, &extents, mrCairoCommon.m_oLineColor, mrCairoCommon.m_oFillColor,
                          getAntiAlias(), nX, nY, nWidth, nHeight);

    mrCairoCommon.releaseCairoContext(cr, true, extents);
}

void X11CairoSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(true, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    CairoCommon::drawPolygon(cr, &extents, mrCairoCommon.m_oLineColor, mrCairoCommon.m_oFillColor,
                             getAntiAlias(), nPoints, pPtAry);

    mrCairoCommon.releaseCairoContext(cr, true, extents);
}

void X11CairoSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPointCounts,
                                              const Point** pPtAry)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(true, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    CairoCommon::drawPolyPolygon(cr, &extents, mrCairoCommon.m_oLineColor,
                                 mrCairoCommon.m_oFillColor, getAntiAlias(), nPoly, pPointCounts,
                                 pPtAry);

    mrCairoCommon.releaseCairoContext(cr, true, extents);
}

bool X11CairoSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                              const basegfx::B2DPolyPolygon& rPolyPolygon,
                                              double fTransparency)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(true, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    bool bRetVal(CairoCommon::drawPolyPolygon(cr, &extents, mrCairoCommon.m_oLineColor,
                                              mrCairoCommon.m_oFillColor, getAntiAlias(),
                                              rObjectToDevice, rPolyPolygon, fTransparency));

    mrCairoCommon.releaseCairoContext(cr, true, extents);

    return bRetVal;
}

void X11CairoSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY)
{
    drawPixel(nX, nY, *mrCairoCommon.m_oLineColor);
}

void X11CairoSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY, Color aColor)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(true, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    CairoCommon::drawPixel(cr, &extents, aColor, nX, nY);

    mrCairoCommon.releaseCairoContext(cr, true, extents);
}

Color X11CairoSalGraphicsImpl::getPixel(tools::Long nX, tools::Long nY)
{
    return CairoCommon::getPixel(mrCairoCommon.m_pSurface, nX, nY);
}

void X11CairoSalGraphicsImpl::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                                       tools::Long nY2)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(false, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    CairoCommon::drawLine(cr, &extents, *mrCairoCommon.m_oLineColor, getAntiAlias(), nX1, nY1, nX2,
                          nY2);

    mrCairoCommon.releaseCairoContext(cr, false, extents);
}

void X11CairoSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(false, getAntiAlias());
    basegfx::B2DRange aExtents;
    mrCairoCommon.clipRegion(cr);

    CairoCommon::drawPolyLine(cr, &aExtents, *mrCairoCommon.m_oLineColor, getAntiAlias(), nPoints,
                              pPtAry);

    mrCairoCommon.releaseCairoContext(cr, false, aExtents);
}

bool X11CairoSalGraphicsImpl::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                           const basegfx::B2DPolygon& rPolyLine,
                                           double fTransparency, double fLineWidth,
                                           const std::vector<double>* pStroke,
                                           basegfx::B2DLineJoin eLineJoin,
                                           css::drawing::LineCap eLineCap,
                                           double fMiterMinimumAngle, bool bPixelSnapHairline)
{
    // short circuit if there is nothing to do
    if (0 == rPolyLine.count() || fTransparency < 0.0 || fTransparency >= 1.0)
        return true;

    cairo_t* cr = mrCairoCommon.getCairoContext(false, getAntiAlias());
    basegfx::B2DRange aExtents;
    mrCairoCommon.clipRegion(cr);

    // Use the now available static drawPolyLine from the Cairo-Headless-Fallback
    // that will take care of all needed stuff
    bool bRetval(CairoCommon::drawPolyLine(cr, &aExtents, *mrCairoCommon.m_oLineColor,
                                           getAntiAlias(), rObjectToDevice, rPolyLine,
                                           fTransparency, fLineWidth, pStroke, eLineJoin, eLineCap,
                                           fMiterMinimumAngle, bPixelSnapHairline));

    mrCairoCommon.releaseCairoContext(cr, false, aExtents);

    return bRetval;
}

bool X11CairoSalGraphicsImpl::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                            tools::Long nHeight, sal_uInt8 nTransparency)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(false, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    const bool bRetval(CairoCommon::drawAlphaRect(cr, &extents, *mrCairoCommon.m_oLineColor,
                                                  *mrCairoCommon.m_oFillColor, nX, nY, nWidth,
                                                  nHeight, nTransparency));
    mrCairoCommon.releaseCairoContext(cr, false, extents);

    return bRetval;
}

bool X11CairoSalGraphicsImpl::drawGradient(const tools::PolyPolygon& rPolyPolygon,
                                           const Gradient& rGradient)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(true, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    const bool bRetval(
        CairoCommon::drawGradient(cr, &extents, getAntiAlias(), rPolyPolygon, rGradient));

    mrCairoCommon.releaseCairoContext(cr, true, extents);

    return bRetval;
}

bool X11CairoSalGraphicsImpl::implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                                               SalGradient const& rGradient)
{
    cairo_t* cr = mrCairoCommon.getCairoContext(true, getAntiAlias());
    basegfx::B2DRange extents;
    mrCairoCommon.clipRegion(cr);

    bool bRetVal(
        CairoCommon::implDrawGradient(cr, &extents, getAntiAlias(), rPolyPolygon, rGradient));

    mrCairoCommon.releaseCairoContext(cr, true, extents);

    return bRetVal;
}

bool X11CairoSalGraphicsImpl::hasFastDrawTransformedBitmap() const
{
    return CairoCommon::hasFastDrawTransformedBitmap();
}

bool X11CairoSalGraphicsImpl::supportsOperation(OutDevSupportType eType) const
{
    return CairoCommon::supportsOperation(eType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
