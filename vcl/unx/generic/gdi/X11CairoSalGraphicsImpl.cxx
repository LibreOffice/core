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
#include <salframe.hxx>

X11CairoSalGraphicsImpl::X11CairoSalGraphicsImpl(X11SalGraphics& rParent, CairoCommon& rCairoCommon)
    : mrParent(rParent)
    , mrCairoCommon(rCairoCommon)
{
}

tools::Long X11CairoSalGraphicsImpl::GetGraphicsWidth() const
{
    if (mrParent.m_pFrame)
        return mrParent.m_pFrame->GetWidth();
    return mrCairoCommon.m_pSurface ? mrCairoCommon.m_aFrameSize.getX() : 0;
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

void X11CairoSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                              const basegfx::B2DPolyPolygon& rPolyPolygon,
                                              double fTransparency)
{
    mrCairoCommon.drawPolyPolygon(rObjectToDevice, rPolyPolygon, fTransparency, getAntiAlias());
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

void X11CairoSalGraphicsImpl::invert(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                     tools::Long nHeight, SalInvert nFlags)
{
    mrCairoCommon.invert(nX, nY, nWidth, nHeight, nFlags, getAntiAlias());
}

void X11CairoSalGraphicsImpl::invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags)
{
    mrCairoCommon.invert(nPoints, pPtAry, nFlags, getAntiAlias());
}

bool X11CairoSalGraphicsImpl::supportsOperation(OutDevSupportType eType) const
{
    return CairoCommon::supportsOperation(eType);
}

void X11CairoSalGraphicsImpl::copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX,
                                       tools::Long nSrcY, tools::Long nSrcWidth,
                                       tools::Long nSrcHeight, bool /*bWindowInvalidate*/)
{
    SalTwoRect aTR(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);

    cairo_surface_t* source = mrCairoCommon.m_pSurface;
    mrCairoCommon.copyBitsCairo(aTR, source, getAntiAlias());
}

void X11CairoSalGraphicsImpl::copyBits(const SalTwoRect& rTR, SalGraphics* pSrcGraphics)
{
    cairo_surface_t* source = nullptr;

    if (pSrcGraphics)
    {
        X11CairoSalGraphicsImpl* pSrc
            = static_cast<X11CairoSalGraphicsImpl*>(pSrcGraphics->GetImpl());
        source = pSrc->mrCairoCommon.m_pSurface;
    }
    else
    {
        source = mrCairoCommon.m_pSurface;
    }

    mrCairoCommon.copyBitsCairo(rTR, source, getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    mrCairoCommon.drawBitmap(rPosAry, rSalBitmap, getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                         const SalBitmap& rTransparentBitmap)
{
    drawAlphaBitmap(rPosAry, rSalBitmap, rTransparentBitmap);
}

bool X11CairoSalGraphicsImpl::drawAlphaBitmap(const SalTwoRect& rTR, const SalBitmap& rSrcBitmap,
                                              const SalBitmap& rAlphaBmp)
{
    return mrCairoCommon.drawAlphaBitmap(rTR, rSrcBitmap, rAlphaBmp, getAntiAlias());
}

void X11CairoSalGraphicsImpl::drawMask(const SalTwoRect& rTR, const SalBitmap& rSalBitmap,
                                       Color nMaskColor)
{
    mrCairoCommon.drawMask(rTR, rSalBitmap, nMaskColor, getAntiAlias());
}

std::shared_ptr<SalBitmap> X11CairoSalGraphicsImpl::getBitmap(tools::Long nX, tools::Long nY,
                                                              tools::Long nWidth,
                                                              tools::Long nHeight,
                                                              bool bWithoutAlpha)
{
    return mrCairoCommon.getBitmap(nX, nY, nWidth, nHeight, bWithoutAlpha);
}

bool X11CairoSalGraphicsImpl::drawPolyLineBezier(sal_uInt32, const Point*, const PolyFlags*)
{
    return false;
}

bool X11CairoSalGraphicsImpl::drawPolygonBezier(sal_uInt32, const Point*, const PolyFlags*)
{
    return false;
}

bool X11CairoSalGraphicsImpl::drawPolyPolygonBezier(sal_uInt32, const sal_uInt32*,
                                                    const Point* const*, const PolyFlags* const*)
{
    return false;
}

bool X11CairoSalGraphicsImpl::drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                                    const basegfx::B2DPoint& rX,
                                                    const basegfx::B2DPoint& rY,
                                                    const SalBitmap& rSourceBitmap,
                                                    const SalBitmap* pAlphaBitmap, double fAlpha)
{
    return mrCairoCommon.drawTransformedBitmap(rNull, rX, rY, rSourceBitmap, pAlphaBitmap, fAlpha,
                                               getAntiAlias());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
