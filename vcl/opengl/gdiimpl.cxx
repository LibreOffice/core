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

#include "openglgdiimpl.hxx"

OpenGLSalGraphicsImpl::~OpenGLSalGraphicsImpl()
{
}

void OpenGLSalGraphicsImpl::freeResources()
{
}

bool OpenGLSalGraphicsImpl::setClipRegion( const vcl::Region& )
{
    return false;
}

// get the depth of the device
sal_uInt16 OpenGLSalGraphicsImpl::GetBitCount() const
{
    return 32;
}

// get the width of the device
long OpenGLSalGraphicsImpl::GetGraphicsWidth() const
{
    return 0;
}

// set the clip region to empty
void OpenGLSalGraphicsImpl::ResetClipRegion()
{
}

// set the line color to transparent (= don't draw lines)

void OpenGLSalGraphicsImpl::SetLineColor()
{
}

// set the line color to a specific color
void OpenGLSalGraphicsImpl::SetLineColor( SalColor /*nSalColor*/ )
{
}

// set the fill color to transparent (= don't fill)
void OpenGLSalGraphicsImpl::SetFillColor()
{
}

// set the fill color to a specific color, shapes will be
// filled accordingly
void OpenGLSalGraphicsImpl::SetFillColor( SalColor /*nSalColor*/ )
{
}

// enable/disable XOR drawing
void OpenGLSalGraphicsImpl::SetXORMode( bool /*bSet*/, bool /*bInvertOnly*/ )
{
}

// set line color for raster operations
void OpenGLSalGraphicsImpl::SetROPLineColor( SalROPColor /*nROPColor*/ )
{
}

// set fill color for raster operations
void OpenGLSalGraphicsImpl::SetROPFillColor( SalROPColor /*nROPColor*/ )
{
}

// draw --> LineColor and FillColor and RasterOp and ClipRegion
void OpenGLSalGraphicsImpl::drawPixel( long /*nX*/, long /*nY*/ )
{
}

void OpenGLSalGraphicsImpl::drawPixel( long /*nX*/, long /*nY*/, SalColor /*nSalColor*/ )
{
}

void OpenGLSalGraphicsImpl::drawLine( long /*nX1*/, long /*nY1*/, long /*nX2*/, long /*nY2*/ )
{
}

void OpenGLSalGraphicsImpl::drawRect( long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/ )
{
}

void OpenGLSalGraphicsImpl::drawPolyLine( sal_uInt32 /*nPoints*/, const SalPoint* /*pPtAry*/ )
{
}

void OpenGLSalGraphicsImpl::drawPolygon( sal_uInt32 /*nPoints*/, const SalPoint* /*pPtAry*/ )
{
}

void OpenGLSalGraphicsImpl::drawPolyPolygon( sal_uInt32 /*nPoly*/, const sal_uInt32* /*pPoints*/, PCONSTSALPOINT* /*pPtAry*/ )
{
}
bool OpenGLSalGraphicsImpl::drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double /*fTransparency*/ )
{
    return false;
}

bool OpenGLSalGraphicsImpl::drawPolyLine(
            const ::basegfx::B2DPolygon&,
            double /*fTransparency*/,
            const ::basegfx::B2DVector& /*rLineWidths*/,
            basegfx::B2DLineJoin,
            com::sun::star::drawing::LineCap)
{
    return false;
}

bool OpenGLSalGraphicsImpl::drawPolyLineBezier(
            sal_uInt32 /*nPoints*/,
            const SalPoint* /*pPtAry*/,
            const sal_uInt8* /*pFlgAry*/ )
{
    return false;
}

bool OpenGLSalGraphicsImpl::drawPolygonBezier(
            sal_uInt32 /*nPoints*/,
            const SalPoint* /*pPtAry*/,
            const sal_uInt8* /*pFlgAry*/ )
{
    return false;
}

bool OpenGLSalGraphicsImpl::drawPolyPolygonBezier(
            sal_uInt32 /*nPoly*/,
            const sal_uInt32* /*pPoints*/,
            const SalPoint* const* /*pPtAry*/,
            const sal_uInt8* const* /*pFlgAry*/ )
{
    return NULL;
}

// CopyArea --> No RasterOp, but ClipRegion
void OpenGLSalGraphicsImpl::copyArea(
            long /*nDestX*/, long /*nDestY*/,
            long /*nSrcX*/, long /*nSrcY*/,
            long /*nSrcWidth*/, long /*nSrcHeight*/,
            sal_uInt16 /*nFlags*/ )
{
}

// CopyBits and DrawBitmap --> RasterOp and ClipRegion
// CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
void OpenGLSalGraphicsImpl::copyBits( const SalTwoRect& /*rPosAry*/, SalGraphics* /*pSrcGraphics*/ )
{
}

void OpenGLSalGraphicsImpl::drawBitmap( const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rSalBitmap*/ )
{
}

void OpenGLSalGraphicsImpl::drawBitmap(
            const SalTwoRect& /*rPosAry*/,
            const SalBitmap& /*rSalBitmap*/,
            SalColor /*nTransparentColor*/ )
{
}

void OpenGLSalGraphicsImpl::drawBitmap(
            const SalTwoRect& /*rPosAry*/,
            const SalBitmap& /*rSalBitmap*/,
            const SalBitmap& /*rMaskBitmap*/ )
{
}

void OpenGLSalGraphicsImpl::drawMask(
            const SalTwoRect& /*rPosAry*/,
            const SalBitmap& /*rSalBitmap*/,
            SalColor /*nMaskColor*/ )
{
}

SalBitmap* OpenGLSalGraphicsImpl::getBitmap( long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/ )
{
    return NULL;
}

SalColor OpenGLSalGraphicsImpl::getPixel( long /*nX*/, long /*nY*/ )
{
    return 0;
}

// invert --> ClipRegion (only Windows or VirDevs)
void OpenGLSalGraphicsImpl::invert(
            long /*nX*/, long /*nY*/,
            long /*nWidth*/, long /*nHeight*/,
            SalInvert /*nFlags*/)
{
}

void OpenGLSalGraphicsImpl::invert( sal_uInt32 /*nPoints*/, const SalPoint* /*pPtAry*/, SalInvert /*nFlags*/ )
{
}

bool OpenGLSalGraphicsImpl::drawEPS(
            long /*nX*/, long /*nY*/,
            long /*nWidth*/, long /*nHeight*/,
            void* /*pPtr*/,
            sal_uLong /*nSize*/ )
{
    return false;
}

/** Render bitmap with alpha channel

    @param rSourceBitmap
    Source bitmap to blit

    @param rAlphaBitmap
    Alpha channel to use for blitting

    @return true, if the operation succeeded, and false
    otherwise. In this case, clients should try to emulate alpha
    compositing themselves
 */
bool OpenGLSalGraphicsImpl::drawAlphaBitmap(
            const SalTwoRect&,
            const SalBitmap& /*rSourceBitmap*/,
            const SalBitmap& /*rAlphaBitmap*/ )
{
    return false;
}

/** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
bool OpenGLSalGraphicsImpl::drawTransformedBitmap(
            const basegfx::B2DPoint& /*rNull*/,
            const basegfx::B2DPoint& /*rX*/,
            const basegfx::B2DPoint& /*rY*/,
            const SalBitmap& /*rSourceBitmap*/,
            const SalBitmap* /*pAlphaBitmap*/)
{
    return false;
}

/** Render solid rectangle with given transparency

    @param nTransparency
    Transparency value (0-255) to use. 0 blits and opaque, 255 a
    fully transparent rectangle
 */
bool OpenGLSalGraphicsImpl::drawAlphaRect(
                long /*nX*/, long /*nY*/,
                long /*nWidth*/, long /*nHeight*/,
                sal_uInt8 /*nTransparency*/ )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
