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

#ifndef INCLUDED_VCL_OPENGLGDIIMPL_HXX
#define INCLUDED_VCL_OPENGLGDIIMPL_HXX

#include "salgdiimpl.hxx"
#include <vcl/dllapi.h>

#include "opengl/texture.hxx"

#include <tools/poly.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

class SalFrame;
class SalVirtualDevice;

class VCL_PLUGIN_PUBLIC OpenGLSalGraphicsImpl : public SalGraphicsImpl
{
protected:

    OpenGLContext maContext;
    SalFrame* mpFrame;
    int mnPainting;

    // clipping
    bool mbUseScissor;
    bool mbUseStencil;

    bool mbOffscreen;
    GLuint mnFramebufferId;
    OpenGLTexture maOffscreenTex;

    SalColor mnLineColor;
    SalColor mnFillColor;

    GLuint mnSolidProgram;
    GLuint mnColorUniform;

    GLuint mnTextureProgram;
    GLuint mnSamplerUniform;

    GLuint mnMaskedTextureProgram;
    GLuint mnMaskedSamplerUniform;
    GLuint mnMaskSamplerUniform;

    GLuint mnMaskProgram;
    GLuint mnMaskUniform;
    GLuint mnMaskColorUniform;

    GLuint mnLinearGradientProgram;
    GLuint mnLinearGradientStartColorUniform;
    GLuint mnLinearGradientEndColorUniform;

    GLuint mnRadialGradientProgram;
    GLuint mnRadialGradientStartColorUniform;
    GLuint mnRadialGradientEndColorUniform;
    GLuint mnRadialGradientCenterUniform;

    void ImplSetClipBit( const vcl::Region& rClip, GLuint nMask );

    bool CreateSolidProgram( void );
    bool CreateTextureProgram( void );
    bool CreateMaskedTextureProgram( void );
    bool CreateMaskProgram( void );
    bool CreateLinearGradientProgram( void );
    bool CreateRadialGradientProgram( void );

    void BeginSolid( SalColor nColor, sal_uInt8 nTransparency );
    void BeginSolid( SalColor nColor, double fTransparency );
    void BeginSolid( SalColor nColor );
    void EndSolid( void );
    void BeginInvert( void );
    void EndInvert( void );

    void DrawPoint( long nX, long nY );
    void DrawLine( long nX1, long nY1, long nX2, long nY2 );
    void DrawLines( sal_uInt32 nPoints, const SalPoint* pPtAry, bool bClose );
    void DrawConvexPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry );
    void DrawConvexPolygon( const Polygon& rPolygon );
    void DrawRect( long nX, long nY, long nWidth, long nHeight );
    void DrawRect( const Rectangle& rRect );
    void DrawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry );
    void DrawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void DrawTextureRect( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTexture( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTextureWithMask( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& rPosAry );
    void DrawMask( OpenGLTexture& rTexture, SalColor nMaskColor, const SalTwoRect& rPosAry );
    void DrawLinearGradient( const Gradient& rGradient, const Rectangle& rRect );
    void DrawRadialGradient( const Gradient& rGradient, const Rectangle& rRect );

protected:
    // get the width of the device
    virtual GLfloat GetWidth() const = 0;

    // get the height of the device
    virtual GLfloat GetHeight() const = 0;

    // operations to do before painting
    virtual void PreDraw();

    // operations to do after painting
    virtual void PostDraw();

    // enable/disable offscreen rendering
    virtual void SetOffscreen( bool bOffscreen );


public:
    OpenGLSalGraphicsImpl();
    virtual ~OpenGLSalGraphicsImpl ();

    OpenGLContext& GetOpenGLContext() { return maContext; }

    virtual void freeResources() SAL_OVERRIDE;

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

    /** Render 32-bits bitmap with alpha channel */
    virtual bool drawAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rBitmap ) SAL_OVERRIDE;

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

    virtual void beginPaint() SAL_OVERRIDE;
    virtual void endPaint() SAL_OVERRIDE;
private:
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
