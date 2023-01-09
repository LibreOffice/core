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
    mrCairoCommon.drawRect(nX, nY, nWidth, nHeight, getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    mrCairoCommon.drawPolygon(nPoints, pPtAry, getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPointCounts,
                                              const Point** pPtAry)
{
    mrCairoCommon.drawPolyPolygon(nPoly, pPointCounts, pPtAry, getAntiAlias());
}

bool X11CairoSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                              const basegfx::B2DPolyPolygon& rPolyPolygon,
                                              double fTransparency)
{
    return mrCairoCommon.drawPolyPolygon(rObjectToDevice, rPolyPolygon, fTransparency,
                                         getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY)
{
    mrCairoCommon.drawPixel(mrCairoCommon.m_oLineColor, nX, nY, getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawPixel(tools::Long nX, tools::Long nY, Color aColor)
{
    mrCairoCommon.drawPixel(aColor, nX, nY, getAntiAlias());
}

Color X11CairoSalGraphicsImpl::getPixel(tools::Long nX, tools::Long nY)
{
    return CairoCommon::getPixel(mrCairoCommon.m_pSurface, nX, nY);
}

void X11CairoSalGraphicsImpl::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                                       tools::Long nY2)
{
    mrCairoCommon.drawLine(nX1, nY1, nX2, nY2, getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    mrCairoCommon.drawPolyLine(nPoints, pPtAry, getAntiAlias());
}

bool X11CairoSalGraphicsImpl::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                           const basegfx::B2DPolygon& rPolyLine,
                                           double fTransparency, double fLineWidth,
                                           const std::vector<double>* pStroke,
                                           basegfx::B2DLineJoin eLineJoin,
                                           css::drawing::LineCap eLineCap,
                                           double fMiterMinimumAngle, bool bPixelSnapHairline)
{
    return mrCairoCommon.drawPolyLine(rObjectToDevice, rPolyLine, fTransparency, fLineWidth,
                                      pStroke, eLineJoin, eLineCap, fMiterMinimumAngle,
                                      bPixelSnapHairline, getAntiAlias());
}

bool X11CairoSalGraphicsImpl::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                            tools::Long nHeight, sal_uInt8 nTransparency)
{
    return mrCairoCommon.drawAlphaRect(nX, nY, nWidth, nHeight, nTransparency, getAntiAlias());
}

bool X11CairoSalGraphicsImpl::drawGradient(const tools::PolyPolygon& rPolyPolygon,
                                           const Gradient& rGradient)
{
    return mrCairoCommon.drawGradient(rPolyPolygon, rGradient, getAntiAlias());
}

bool X11CairoSalGraphicsImpl::implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                                               SalGradient const& rGradient)
{
    return mrCairoCommon.implDrawGradient(rPolyPolygon, rGradient, getAntiAlias());
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
