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

#ifndef INCLUDED_VCL_GENERIC_GDI_GDIIMPL_HXX
#define INCLUDED_VCL_GENERIC_GDI_GDIIMPL_HXX

#include <prex.h>
#include <postx.h>

#include "unx/saltype.h"
#include "unx/x11/x11gdiimpl.h"

#include "salgdiimpl.hxx"

#include <basegfx/polygon/b2dtrapezoid.hxx>

class SalGraphics;
class SalBitmap;
class SalPolyLine;
class X11SalGraphics;
class Gradient;

class X11SalGraphicsImpl : public SalGraphicsImpl, public X11GraphicsImpl
{
private:
    X11SalGraphics& mrParent;

    SalColor mnBrushColor;
    GC mpBrushGC;      // Brush attributes
    Pixel mnBrushPixel;

    bool mbPenGC : 1;        // is Pen GC valid
    bool mbBrushGC : 1;      // is Brush GC valid
    bool mbMonoGC : 1;       // is Mono GC valid
    bool mbCopyGC : 1;       // is Copy GC valid
    bool mbInvertGC : 1;     // is Invert GC valid
    bool mbInvert50GC : 1;   // is Invert50 GC valid
    bool mbStippleGC : 1;    // is Stipple GC valid
    bool mbTrackingGC : 1;   // is Tracking GC valid
    bool mbDitherBrush : 1;  // is solid or tile

    bool mbXORMode : 1;      // is ROP XOR Mode set

    GC mpPenGC;        // Pen attributes
    SalColor mnPenColor;
    Pixel mnPenPixel;


    GC mpMonoGC;
    GC mpCopyGC;
    GC mpMaskGC;
    GC mpInvertGC;
    GC mpInvert50GC;
    GC mpStippleGC;
    GC mpTrackingGC;

    GC CreateGC( Drawable      hDrawable,
            unsigned long nMask = GCGraphicsExposures );

    GC SelectBrush();
    GC SelectPen();
    inline GC GetCopyGC();
    inline GC GetStippleGC();
    GC GetTrackingGC();
    GC GetInvertGC();
    GC GetInvert50GC();
    inline GC GetMonoGC( Pixmap hPixmap );

    void DrawLines( sal_uIntPtr              nPoints,
                               const SalPolyLine &rPoints,
                               GC                 pGC,
                               bool bClose
                               );

    XID GetXRenderPicture();
    bool drawFilledTrapezoids( const ::basegfx::B2DTrapezoid*, int nTrapCount, double fTransparency );

    long GetGraphicsHeight() const;

    void drawMaskedBitmap( const SalTwoRect& rPosAry,
                                              const SalBitmap& rSalBitmap,
                                              const SalBitmap& rTransparentBitmap );

    void drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry, bool bClose );

public:

    X11SalGraphicsImpl(X11SalGraphics& rParent);

    virtual void freeResources() SAL_OVERRIDE;

    virtual ~X11SalGraphicsImpl();

    virtual bool setClipRegion( const vcl::Region& ) SAL_OVERRIDE;
    //
    // get the depth of the device
    virtual sal_uInt16 GetBitCount() const SAL_OVERRIDE;

    // get the width of the device
    virtual long GetGraphicsWidth() const SAL_OVERRIDE;

    // set the clip region to empty
    virtual void ResetClipRegion() SAL_OVERRIDE;

    // set the line color to transparent (= don't draw lines)

    virtual void SetLineColor() SAL_OVERRIDE;

    // set the line color to a specific color
    virtual void SetLineColor( SalColor nSalColor ) SAL_OVERRIDE;

    // set the fill color to transparent (= don't fill)
    virtual void SetFillColor() SAL_OVERRIDE;

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void SetFillColor( SalColor nSalColor ) SAL_OVERRIDE;

    // enable/disable XOR drawing
    virtual void SetXORMode( bool bSet, bool bInvertOnly ) SAL_OVERRIDE;

    // set line color for raster operations
    virtual void SetROPLineColor( SalROPColor nROPColor ) SAL_OVERRIDE;

    // set fill color for raster operations
    virtual void SetROPFillColor( SalROPColor nROPColor ) SAL_OVERRIDE;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void drawPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void drawPixel( long nX, long nY, SalColor nSalColor ) SAL_OVERRIDE;

    virtual void drawLine( long nX1, long nY1, long nX2, long nY2 ) SAL_OVERRIDE;

    virtual void drawRect( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;

    virtual void drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;

    virtual void drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;

    virtual void drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) SAL_OVERRIDE;
    virtual bool drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency ) SAL_OVERRIDE;

    virtual bool drawPolyLine(
                const ::basegfx::B2DPolygon&,
                double fTransparency,
                const ::basegfx::B2DVector& rLineWidths,
                basegfx::B2DLineJoin,
                com::sun::star::drawing::LineCap) SAL_OVERRIDE;

    virtual bool drawPolyLineBezier(
                sal_uInt32 nPoints,
                const SalPoint* pPtAry,
                const sal_uInt8* pFlgAry ) SAL_OVERRIDE;

    virtual bool drawPolygonBezier(
                sal_uInt32 nPoints,
                const SalPoint* pPtAry,
                const sal_uInt8* pFlgAry ) SAL_OVERRIDE;

    virtual bool drawPolyPolygonBezier(
                sal_uInt32 nPoly,
                const sal_uInt32* pPoints,
                const SalPoint* const* pPtAry,
                const sal_uInt8* const* pFlgAry ) SAL_OVERRIDE;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void copyArea(
                long nDestX, long nDestY,
                long nSrcX, long nSrcY,
                long nSrcWidth, long nSrcHeight,
                sal_uInt16 nFlags ) SAL_OVERRIDE;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) SAL_OVERRIDE;

    virtual void drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) SAL_OVERRIDE;

    virtual void drawBitmap(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                SalColor nTransparentColor ) SAL_OVERRIDE;

    virtual void drawBitmap(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                const SalBitmap& rMaskBitmap ) SAL_OVERRIDE;

    virtual void drawMask(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                SalColor nMaskColor ) SAL_OVERRIDE;

    virtual SalBitmap* getBitmap( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;

    virtual SalColor getPixel( long nX, long nY ) SAL_OVERRIDE;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void invert(
                long nX, long nY,
                long nWidth, long nHeight,
                SalInvert nFlags) SAL_OVERRIDE;

    virtual void invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) SAL_OVERRIDE;

    virtual bool drawEPS(
                long nX, long nY,
                long nWidth, long nHeight,
                void* pPtr,
                sal_uLong nSize ) SAL_OVERRIDE;

    /** Blend bitmap with color channels */
    virtual bool blendBitmap(
                const SalTwoRect&,
                const SalBitmap& rBitmap ) SAL_OVERRIDE;

    /** Render bitmap by blending using the mask and alpha channel */
    virtual bool blendAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rSrcBitmap,
                const SalBitmap& rMaskBitmap,
                const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;

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
                const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;

    /** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
    virtual bool drawTransformedBitmap(
                const basegfx::B2DPoint& rNull,
                const basegfx::B2DPoint& rX,
                const basegfx::B2DPoint& rY,
                const SalBitmap& rSourceBitmap,
                const SalBitmap* pAlphaBitmap) SAL_OVERRIDE;

    /** Render solid rectangle with given transparency

        @param nTransparency
        Transparency value (0-255) to use. 0 blits and opaque, 255 a
        fully transparent rectangle
     */
    virtual bool drawAlphaRect(
                    long nX, long nY,
                    long nWidth, long nHeight,
                    sal_uInt8 nTransparency ) SAL_OVERRIDE;

    virtual bool drawGradient(const tools::PolyPolygon& rPolygon, const Gradient& rGradient) SAL_OVERRIDE;

public:
    // implementation of X11GraphicsImpl

    void Init() SAL_OVERRIDE;
    bool FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY ) SAL_OVERRIDE;
    bool RenderPixmapToScreen( X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY ) SAL_OVERRIDE;

    virtual bool TryRenderCachedNativeControl(ControlCacheKey& rControlCacheKey,
                                              int nX, int nY) SAL_OVERRIDE;
    virtual bool RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                             ControlCacheKey& aControlCacheKey) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
