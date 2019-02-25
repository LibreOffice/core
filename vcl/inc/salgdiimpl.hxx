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

#ifndef INCLUDED_VCL_INC_SALGDIIMPL_HXX
#define INCLUDED_VCL_INC_SALGDIIMPL_HXX

#include <vcl/dllapi.h>

#include <rtl/ustring.hxx>

#include <tools/color.hxx>
#include <tools/poly.hxx>
#include <tools/solar.h>

#include <vcl/salgtype.hxx>
#include <vcl/region.hxx>
#include <vcl/salnativewidgets.hxx>

#include <com/sun/star/drawing/LineCap.hpp>

class SalGraphics;
class SalBitmap;
class SalFrame;
class Gradient;
class OpenGLContext;
class SalVirtualDevice;

class VCL_PLUGIN_PUBLIC SalGraphicsImpl
{
public:

    virtual ~SalGraphicsImpl();

    virtual void Init() = 0;

    virtual void DeInit() {}

    virtual void freeResources() = 0;

    virtual bool setClipRegion( const vcl::Region& ) = 0;
    //
    // get the depth of the device
    virtual sal_uInt16 GetBitCount() const = 0;

    // get the width of the device
    virtual long GetGraphicsWidth() const = 0;

    // set the clip region to empty
    virtual void ResetClipRegion() = 0;

    // set the line color to transparent (= don't draw lines)

    virtual void SetLineColor() = 0;

    // set the line color to a specific color
    virtual void SetLineColor( Color nColor ) = 0;

    // set the fill color to transparent (= don't fill)
    virtual void SetFillColor() = 0;

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void SetFillColor( Color nColor ) = 0;

    // enable/disable XOR drawing
    virtual void SetXORMode( bool bSet, bool bInvertOnly ) = 0;

    // set line color for raster operations
    virtual void SetROPLineColor( SalROPColor nROPColor ) = 0;

    // set fill color for raster operations
    virtual void SetROPFillColor( SalROPColor nROPColor ) = 0;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void drawPixel( long nX, long nY ) = 0;
    virtual void drawPixel( long nX, long nY, Color nColor ) = 0;

    virtual void drawLine( long nX1, long nY1, long nX2, long nY2 ) = 0;

    virtual void drawRect( long nX, long nY, long nWidth, long nHeight ) = 0;

    virtual void drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) = 0;

    virtual void drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) = 0;

    virtual void drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) = 0;

    virtual bool drawPolyPolygon(
        const basegfx::B2DHomMatrix& rObjectToDevice,
        const basegfx::B2DPolyPolygon&,
        double fTransparency) = 0;

    virtual bool drawPolyLine(
                const basegfx::B2DHomMatrix& rObjectToDevice,
                const basegfx::B2DPolygon&,
                double fTransparency,
                const basegfx::B2DVector& rLineWidths,
                basegfx::B2DLineJoin,
                css::drawing::LineCap,
                double fMiterMinimumAngle,
                bool bPixelSnapHairline) = 0;

    virtual bool drawPolyLineBezier(
                sal_uInt32 nPoints,
                const SalPoint* pPtAry,
                const PolyFlags* pFlgAry ) = 0;

    virtual bool drawPolygonBezier(
                sal_uInt32 nPoints,
                const SalPoint* pPtAry,
                const PolyFlags* pFlgAry ) = 0;

    virtual bool drawPolyPolygonBezier(
                sal_uInt32 nPoly,
                const sal_uInt32* pPoints,
                const SalPoint* const* pPtAry,
                const PolyFlags* const* pFlgAry ) = 0;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void copyArea(
                long nDestX, long nDestY,
                long nSrcX, long nSrcY,
                long nSrcWidth, long nSrcHeight,
                bool bWindowInvalidate ) = 0;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) = 0;

    virtual void drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) = 0;

    virtual void drawBitmap(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                const SalBitmap& rMaskBitmap ) = 0;

    virtual void drawMask(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                Color nMaskColor ) = 0;

    virtual std::shared_ptr<SalBitmap> getBitmap( long nX, long nY, long nWidth, long nHeight ) = 0;

    virtual Color getPixel( long nX, long nY ) = 0;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void invert(
                long nX, long nY,
                long nWidth, long nHeight,
                SalInvert nFlags) = 0;

    virtual void invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) = 0;

    virtual bool drawEPS(
                long nX, long nY,
                long nWidth, long nHeight,
                void* pPtr,
                sal_uInt32 nSize ) = 0;

    virtual bool blendBitmap(
                const SalTwoRect&,
                const SalBitmap& rBitmap ) = 0;

    virtual bool blendAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rSrcBitmap,
                const SalBitmap& rMaskBitmap,
                const SalBitmap& rAlphaBitmap ) = 0;

    virtual bool drawAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rSourceBitmap,
                const SalBitmap& rAlphaBitmap ) = 0;

    /** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
    virtual bool drawTransformedBitmap(
                const basegfx::B2DPoint& rNull,
                const basegfx::B2DPoint& rX,
                const basegfx::B2DPoint& rY,
                const SalBitmap& rSourceBitmap,
                const SalBitmap* pAlphaBitmap) = 0;

    virtual bool drawAlphaRect(
                    long nX, long nY,
                    long nWidth, long nHeight,
                    sal_uInt8 nTransparency ) = 0;

    virtual bool drawGradient(const tools::PolyPolygon& rPolygon, const Gradient& rGradient) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
