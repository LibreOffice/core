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

#include "Kf5Graphics.hxx"

bool Kf5Graphics::setClipRegion( const vcl::Region& rRegion )
{
    return false;
}

void Kf5Graphics::ResetClipRegion()
{
}

void Kf5Graphics::drawPixel( long nX, long nY )
{
}

void Kf5Graphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
}

void Kf5Graphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
}

void Kf5Graphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
}

void Kf5Graphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
}

void Kf5Graphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
}

void Kf5Graphics::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
}

bool Kf5Graphics::drawPolyPolygon( const basegfx::B2DPolyPolygon&, double fTransparency )
{
    return false;
}

bool Kf5Graphics::drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    return false;
}

bool Kf5Graphics::drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    return false;
}

bool Kf5Graphics::drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                         const SalPoint* const* pPtAry, const PolyFlags* const* pFlgAry )
{
    return false;
}

bool Kf5Graphics::drawPolyLine( const basegfx::B2DPolygon&,
                                double fTransparency,
                                const basegfx::B2DVector& rLineWidths,
                                basegfx::B2DLineJoin,
                                css::drawing::LineCap eLineCap,
                                double fMiterMinimumAngle )
{
    return false;
}

bool Kf5Graphics::drawGradient( const tools::PolyPolygon&, const Gradient& )
{
    return false;
}

void Kf5Graphics::copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                            long nSrcHeight, bool bWindowInvalidate )
{
}

void Kf5Graphics::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
}

void Kf5Graphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
}

void Kf5Graphics::drawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap )
{
}

void Kf5Graphics::drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor )
{
}

SalBitmap* Kf5Graphics::getBitmap( long nX, long nY, long nWidth, long nHeight )
{
    return nullptr;
}

SalColor Kf5Graphics::getPixel( long nX, long nY )
{
    return 0;
}

void Kf5Graphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags)
{
}

void Kf5Graphics::invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
}

bool Kf5Graphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
    return false;
}

bool Kf5Graphics::blendBitmap( const SalTwoRect&,
                                         const SalBitmap& rBitmap )
{
    return false;
}

bool Kf5Graphics::blendAlphaBitmap( const SalTwoRect&,
                                              const SalBitmap& rSrcBitmap,
                                              const SalBitmap& rMaskBitmap,
                                              const SalBitmap& rAlphaBitmap )
{
    return false;
}

bool Kf5Graphics::drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap )
{
    return false;
}

bool Kf5Graphics::drawTransformedBitmap(
                                            const basegfx::B2DPoint& rNull,
                                            const basegfx::B2DPoint& rX,
                                            const basegfx::B2DPoint& rY,
                                            const SalBitmap& rSourceBitmap,
                                            const SalBitmap* pAlphaBitmap)
{
    return false;
}

bool Kf5Graphics::drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency )
{
    return false;
}

void Kf5Graphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
}

sal_uInt16 Kf5Graphics::GetBitCount() const
{
    return 0;
}

long Kf5Graphics::GetGraphicsWidth() const
{
    return 0;
}

void Kf5Graphics::SetLineColor()
{
}

void Kf5Graphics::SetLineColor( SalColor nSalColor )
{
}

void Kf5Graphics::SetFillColor()
{
}

void Kf5Graphics::SetFillColor( SalColor nSalColor )
{
}

void Kf5Graphics::SetXORMode( bool bSet )
{
}

void Kf5Graphics::SetROPLineColor( SalROPColor nROPColor )
{
}

void Kf5Graphics::SetROPFillColor( SalROPColor nROPColor )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
