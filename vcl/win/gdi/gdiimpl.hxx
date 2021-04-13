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

#pragma once

#include <salgdiimpl.hxx>
#include <tools/long.hxx>
#include <win/salgdi.h>
#include <win/wingdiimpl.hxx>

#include <vcl/gradient.hxx>

#include <svsys.h>
#include <ControlCacheKey.hxx>

class WinSalGraphics;

class WinSalGraphicsImpl : public SalGraphicsImpl, public WinSalGraphicsImplBase
{
private:

    WinSalGraphics& mrParent;
    bool mbXORMode : 1; // _every_ output with RasterOp XOR
    bool mbPen : 1; // is Pen (FALSE == NULL_PEN)
    HPEN mhPen; // Pen
    bool mbStockPen : 1; // is Pen a stockpen
    bool mbBrush : 1; // is Brush (FALSE == NULL_BRUSH)
    bool mbStockBrush : 1; // is Brush a stockbrush
    HBRUSH mhBrush; // Brush
    COLORREF mnPenColor; // PenColor
    COLORREF mnBrushColor; // BrushColor

    // remember RGB values for SetLineColor/SetFillColor
    Color                maLineColor;
    Color                maFillColor;

    bool TryDrawBitmapGDIPlus(const SalTwoRect& rTR, const SalBitmap& rSrcBitmap);
    void DrawPixelImpl(tools::Long nX, tools::Long nY, COLORREF crColor);

    HPEN SearchStockPen(COLORREF nPenColor);
    HPEN MakePen(Color nColor);
    void ResetPen(HPEN hNewPen);

    HBRUSH SearchStockBrush(COLORREF nBrushColor);
    HBRUSH MakeBrush(Color nColor);
    void ResetBrush(HBRUSH hNewBrush);
public:

    explicit WinSalGraphicsImpl(WinSalGraphics& rParent);

    virtual ~WinSalGraphicsImpl() override;

    virtual void Init() override;

    virtual void freeResources() override;

    virtual OUString getRenderBackendName() const override { return "gdi"; }

    virtual bool setClipRegion( const vcl::Region& ) override;
    //
    // get the depth of the device
    virtual sal_uInt16 GetBitCount() const override;

    // get the width of the device
    virtual tools::Long GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void ResetClipRegion() override;

    // set the line color to transparent (= don't draw lines)

    virtual void SetLineColor() override;

    // set the line color to a specific color
    virtual void SetLineColor( Color nColor ) override;

    // set the fill color to transparent (= don't fill)
    virtual void SetFillColor() override;

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void SetFillColor( Color nColor ) override;

    // enable/disable XOR drawing
    virtual void SetXORMode( bool bSet, bool bInvertOnly ) override;

    // set line color for raster operations
    virtual void SetROPLineColor( SalROPColor nROPColor ) override;

    // set fill color for raster operations
    virtual void SetROPFillColor( SalROPColor nROPColor ) override;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void drawPixel( tools::Long nX, tools::Long nY ) override;
    virtual void drawPixel( tools::Long nX, tools::Long nY, Color nColor ) override;

    virtual void drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 ) override;

    virtual void drawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;

    virtual void drawPolyLine( sal_uInt32 nPoints, const Point* pPtAry ) override;

    virtual void drawPolygon( sal_uInt32 nPoints, const Point* pPtAry ) override;

    virtual void drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point** pPtAry ) override;

    virtual bool drawPolyPolygon(
                const basegfx::B2DHomMatrix& rObjectToDevice,
                const basegfx::B2DPolyPolygon&,
                double fTransparency) override;

    virtual bool drawPolyLine(
                const basegfx::B2DHomMatrix& rObjectToDevice,
                const basegfx::B2DPolygon&,
                double fTransparency,
                double fLineWidth,
                const std::vector< double >* pStroke, // MM01
                basegfx::B2DLineJoin,
                css::drawing::LineCap,
                double fMiterMinimumAngle,
                bool bPixelSnapHairline) override;

    virtual bool drawPolyLineBezier(
                sal_uInt32 nPoints,
                const Point* pPtAry,
                const PolyFlags* pFlgAry ) override;

    virtual bool drawPolygonBezier(
                sal_uInt32 nPoints,
                const Point* pPtAry,
                const PolyFlags* pFlgAry ) override;

    virtual bool drawPolyPolygonBezier(
                sal_uInt32 nPoly,
                const sal_uInt32* pPoints,
                const Point* const* pPtAry,
                const PolyFlags* const* pFlgAry ) override;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void copyArea(
                tools::Long nDestX, tools::Long nDestY,
                tools::Long nSrcX, tools::Long nSrcY,
                tools::Long nSrcWidth, tools::Long nSrcHeight, bool bWindowInvalidate ) override;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) override;

    virtual void drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) override;

    virtual void drawBitmap(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                const SalBitmap& rMaskBitmap ) override;

    virtual void drawMask(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                Color nMaskColor ) override;

    virtual std::shared_ptr<SalBitmap> getBitmap( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;

    virtual Color getPixel( tools::Long nX, tools::Long nY ) override;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void invert(
                tools::Long nX, tools::Long nY,
                tools::Long nWidth, tools::Long nHeight,
                SalInvert nFlags) override;

    virtual void invert( sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags ) override;

    virtual bool drawEPS(
                tools::Long nX, tools::Long nY,
                tools::Long nWidth, tools::Long nHeight,
                void* pPtr,
                sal_uInt32 nSize ) override;

    virtual bool blendBitmap(
                const SalTwoRect&,
                const SalBitmap& rBitmap ) override;

    virtual bool blendAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rSrcBitmap,
                const SalBitmap& rMaskBitmap,
                const SalBitmap& rAlphaBitmap ) override;

    /** Render bitmap with alpha channel

        @param rSourceBitmap
        Source bitmap to blit

        @param rAlphaBitmap
        Alpha channel to use for blitting

        @return true, if the operation succeeded, and false
        otherwise. In this case, clients should try to emulate alpha
        compositing themselves
     */
    virtual bool drawAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rSourceBitmap,
                const SalBitmap& rAlphaBitmap ) override;

    /** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
    virtual bool drawTransformedBitmap(
                const basegfx::B2DPoint& rNull,
                const basegfx::B2DPoint& rX,
                const basegfx::B2DPoint& rY,
                const SalBitmap& rSourceBitmap,
                const SalBitmap* pAlphaBitmap,
                double fAlpha) override;

    virtual bool hasFastDrawTransformedBitmap() const override;

    /** Render solid rectangle with given transparency

        @param nTransparency
        Transparency value (0-255) to use. 0 blits and opaque, 255 a
        fully transparent rectangle
     */
    virtual bool drawAlphaRect(
                    tools::Long nX, tools::Long nY,
                    tools::Long nWidth, tools::Long nHeight,
                    sal_uInt8 nTransparency ) override;


    virtual bool drawGradient(const tools::PolyPolygon& rPolygon,
            const Gradient& rGradient) override;
    virtual bool implDrawGradient(basegfx::B2DPolyPolygon const & rPolyPolygon,
                SalGradient const & rGradient) override;

    virtual bool supportsOperation(OutDevSupportType eType) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
