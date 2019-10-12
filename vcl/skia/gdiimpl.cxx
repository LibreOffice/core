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

SkiaSalGraphicsImpl::SkiaSalGraphicsImpl(SalGraphics& rParent, SalGeometryProvider* pProvider)
{
    (void)rParent;
    (void)pProvider;
}

SkiaSalGraphicsImpl::~SkiaSalGraphicsImpl() {}

void SkiaSalGraphicsImpl::Init() {}

void SkiaSalGraphicsImpl::DeInit() {}

void SkiaSalGraphicsImpl::freeResources() {}

const vcl::Region& SkiaSalGraphicsImpl::getClipRegion() const
{
    static const vcl::Region reg;
    return reg;
}

bool SkiaSalGraphicsImpl::setClipRegion(const vcl::Region&) { return false; }

sal_uInt16 SkiaSalGraphicsImpl::GetBitCount() const { return 0; }

long SkiaSalGraphicsImpl::GetGraphicsWidth() const { return 0; }

void SkiaSalGraphicsImpl::ResetClipRegion() {}

void SkiaSalGraphicsImpl::SetLineColor() {}

void SkiaSalGraphicsImpl::SetLineColor(Color nColor) { (void)nColor; }

void SkiaSalGraphicsImpl::SetFillColor() {}

void SkiaSalGraphicsImpl::SetFillColor(Color nColor) { (void)nColor; }

void SkiaSalGraphicsImpl::SetXORMode(bool bSet, bool bInvertOnly)
{
    (void)bSet;
    (void)bInvertOnly;
}

void SkiaSalGraphicsImpl::SetROPLineColor(SalROPColor nROPColor) { (void)nROPColor; }

void SkiaSalGraphicsImpl::SetROPFillColor(SalROPColor nROPColor) { (void)nROPColor; }

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY)
{
    (void)nX;
    (void)nY;
}

void SkiaSalGraphicsImpl::drawPixel(long nX, long nY, Color nColor)
{
    (void)nX;
    (void)nY;
    (void)nColor;
}

void SkiaSalGraphicsImpl::drawLine(long nX1, long nY1, long nX2, long nY2)
{
    (void)nX1;
    (void)nY1;
    (void)nX2;
    (void)nY2;
}

void SkiaSalGraphicsImpl::drawRect(long nX, long nY, long nWidth, long nHeight)
{
    (void)nX;
    (void)nY;
    (void)nWidth;
    (void)nHeight;
}

void SkiaSalGraphicsImpl::drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    (void)nPoints;
    (void)pPtAry;
}

void SkiaSalGraphicsImpl::drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    (void)nPoints;
    (void)pPtAry;
}

void SkiaSalGraphicsImpl::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                          PCONSTSALPOINT* pPtAry)
{
    (void)nPoly;
    (void)pPoints;
    (void)pPtAry;
}

bool SkiaSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                          const basegfx::B2DPolyPolygon&, double fTransparency)
{
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
                                   long nSrcHeight, bool bWindowInvalidate)
{
    (void)nDestX;
    (void)nDestY;
    (void)nSrcX;
    (void)nSrcY;
    (void)nSrcWidth;
    (void)nSrcHeight;
    (void)bWindowInvalidate;
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
    (void)rPosAry;
    (void)rSalBitmap;
}

void SkiaSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                     const SalBitmap& rMaskBitmap)
{
    (void)rPosAry;
    (void)rSalBitmap;
    (void)rMaskBitmap;
}

void SkiaSalGraphicsImpl::drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                   Color nMaskColor)
{
    (void)rPosAry;
    (void)rSalBitmap;
    (void)nMaskColor;
}

std::shared_ptr<SalBitmap> SkiaSalGraphicsImpl::getBitmap(long nX, long nY, long nWidth,
                                                          long nHeight)
{
    (void)nX;
    (void)nY;
    (void)nWidth;
    (void)nHeight;
    return nullptr;
}

Color SkiaSalGraphicsImpl::getPixel(long nX, long nY)
{
    (void)nX;
    (void)nY;
    return COL_BLACK;
}

void SkiaSalGraphicsImpl::invert(long nX, long nY, long nWidth, long nHeight, SalInvert nFlags)
{
    (void)nX;
    (void)nY;
    (void)nWidth;
    (void)nHeight;
    (void)nFlags;
}

void SkiaSalGraphicsImpl::invert(sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags)
{
    (void)nPoints;
    (void)pPtAry;
    (void)nFlags;
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

bool SkiaSalGraphicsImpl::drawAlphaBitmap(const SalTwoRect&, const SalBitmap& rSourceBitmap,
                                          const SalBitmap& rAlphaBitmap)
{
    (void)rSourceBitmap;
    (void)rAlphaBitmap;
    return false;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
