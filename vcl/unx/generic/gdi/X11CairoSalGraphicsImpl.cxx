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

X11CairoSalGraphicsImpl::X11CairoSalGraphicsImpl(X11SalGraphics& rParent, X11Common& rX11Common)
    : X11SalGraphicsImpl(rParent)
    , mrX11Common(rX11Common)
    , moPenColor(std::nullopt)
    , moFillColor(std::nullopt)
{
}

void X11CairoSalGraphicsImpl::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                       tools::Long nHeight)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    CairoCommon::drawRect(cr, nullptr, moPenColor, moFillColor, getAntiAlias(), nX, nY, nWidth,
                          nHeight);

    X11Common::releaseCairoContext(cr);
}

void X11CairoSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    CairoCommon::drawPolygon(cr, nullptr, moPenColor, moFillColor, getAntiAlias(), nPoints, pPtAry);

    X11Common::releaseCairoContext(cr);
}

void X11CairoSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                              const Point** pPtAry)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    CairoCommon::drawPolyPolygon(cr, nullptr, moPenColor, moFillColor, getAntiAlias(), nPoly,
                                 pPoints, pPtAry);

    X11Common::releaseCairoContext(cr);
}

bool X11CairoSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                              const basegfx::B2DPolyPolygon& rPolyPolygon,
                                              double fTransparency)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    bool bRetVal(CairoCommon::drawPolyPolygon(cr, nullptr, moPenColor, moFillColor, getAntiAlias(),
                                              rObjectToDevice, rPolyPolygon, fTransparency));

    X11Common::releaseCairoContext(cr);
    return bRetVal;
}

void X11CairoSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY)
{
    drawPixel(nX, nY, *moPenColor);
}

void X11CairoSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY, Color nColor)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    CairoCommon::drawPixel(cr, nullptr, nColor, nX, nY);

    X11Common::releaseCairoContext(cr);
}

Color X11CairoSalGraphicsImpl::getPixel(tools::Long nX, tools::Long nY)
{
    cairo_t* cr = mrX11Common.getCairoContext();

    Color aRet = CairoCommon::getPixel(cairo_get_target(cr), nX, nY);

    X11Common::releaseCairoContext(cr);

    return aRet;
}

void X11CairoSalGraphicsImpl::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                                       tools::Long nY2)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    CairoCommon::drawLine(cr, nullptr, *moPenColor, getAntiAlias(), nX1, nY1, nX2, nY2);

    X11Common::releaseCairoContext(cr);
}

void X11CairoSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    CairoCommon::drawPolyLine(cr, nullptr, *moPenColor, getAntiAlias(), nPoints, pPtAry);

    X11Common::releaseCairoContext(cr);
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

    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    // Use the now available static drawPolyLine from the Cairo-Headless-Fallback
    // that will take care of all needed stuff
    const bool bRetval(CairoCommon::drawPolyLine(
        cr, nullptr, *moPenColor, getAntiAlias(), rObjectToDevice, rPolyLine, fTransparency,
        fLineWidth, pStroke, eLineJoin, eLineCap, fMiterMinimumAngle, bPixelSnapHairline));

    X11Common::releaseCairoContext(cr);

    return bRetval;
}

bool X11CairoSalGraphicsImpl::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                            tools::Long nHeight, sal_uInt8 nTransparency)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    const bool bRetval(CairoCommon::drawAlphaRect(cr, nullptr, *moPenColor, *moFillColor, nX, nY,
                                                  nWidth, nHeight, nTransparency));

    X11Common::releaseCairoContext(cr);

    return bRetval;
}

bool X11CairoSalGraphicsImpl::drawGradient(const tools::PolyPolygon& rPolyPolygon,
                                           const Gradient& rGradient)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    const bool bRetval(
        CairoCommon::drawGradient(cr, nullptr, getAntiAlias(), rPolyPolygon, rGradient));

    X11Common::releaseCairoContext(cr);

    return bRetval;
}

bool X11CairoSalGraphicsImpl::implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                                               SalGradient const& rGradient)
{
    cairo_t* cr = mrX11Common.getCairoContext();
    clipRegion(cr);

    const bool bRetval(
        CairoCommon::implDrawGradient(cr, nullptr, getAntiAlias(), rPolyPolygon, rGradient));

    X11Common::releaseCairoContext(cr);

    return bRetval;
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
