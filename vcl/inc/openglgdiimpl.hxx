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

#include "salgeom.hxx"
#include "salgdiimpl.hxx"
#include <vcl/dllapi.h>

#include "opengl/framebuffer.hxx"
#include "opengl/program.hxx"
#include "opengl/texture.hxx"
#include "regionband.hxx"

#include <tools/poly.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

class SalFrame;
class SalVirtualDevice;
class OpenGLTests;

namespace basegfx
{
class B2DTrapezoid;
};

struct TextureCombo
{
    std::unique_ptr<OpenGLTexture> mpTexture;
    std::unique_ptr<OpenGLTexture> mpMask;
};

class VCL_DLLPUBLIC OpenGLSalGraphicsImpl : public SalGraphicsImpl
{
    friend class OpenGLTests;
protected:

    rtl::Reference<OpenGLContext> mpContext;
    SalGraphics& mrParent;
    /// Pointer to the SalFrame or SalVirtualDevice
    SalGeometryProvider* mpProvider;
    OpenGLFramebuffer* mpFramebuffer;
    OpenGLProgram* mpProgram;

    // clipping
    vcl::Region maClipRegion;
    bool mbUseScissor;
    bool mbUseStencil;

    bool mbOffscreen;
    OpenGLTexture maOffscreenTex;

    SalColor mnLineColor;
    SalColor mnFillColor;
#ifdef DBG_UTIL
    bool mProgramIsSolidColor;
#endif
    SalColor mProgramSolidColor;
    double mProgramSolidTransparency;

    void ImplInitClipRegion();
    void ImplSetClipBit( const vcl::Region& rClip, GLuint nMask );
    void ImplDrawLineAA( double nX1, double nY1, double nX2, double nY2, bool edge = false );
    bool CheckOffscreenTexture();

    void ApplyProgramMatrices(float fPixelOffset = 0.0);

public:
    bool UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );
    bool UseSolid( SalColor nColor, sal_uInt8 nTransparency );
    bool UseSolid( SalColor nColor, double fTransparency );
    bool UseSolid( SalColor nColor );
    bool UseSolidAA( SalColor nColor, double fTransparency );
    bool UseSolidAA( SalColor nColor );
    bool UseInvert();

    void DrawPoint( long nX, long nY );
    void DrawLine( double nX1, double nY1, double nX2, double nY2 );
    void DrawLines( sal_uInt32 nPoints, const SalPoint* pPtAry, bool bClose );
    void DrawLineAA( double nX1, double nY1, double nX2, double nY2 );
    void DrawLinesAA( sal_uInt32 nPoints, const SalPoint* pPtAry, bool bClose );
    void DrawEdgeAA( double nX1, double nY1, double nX2, double nY2 );
    void DrawConvexPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry, bool blockAA = false );
    void DrawConvexPolygon( const Polygon& rPolygon, bool blockAA = false );
    void DrawTrapezoid( const basegfx::B2DTrapezoid& trapezoid, bool blockAA = false );
    void DrawRect( long nX, long nY, long nWidth, long nHeight );
    void DrawRect( const Rectangle& rRect );
    void DrawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry );
    void DrawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, bool blockAA = false );
    void DrawRegionBand( const RegionBand& rRegion );
    void DrawTextureRect( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTexture( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTransformedTexture( OpenGLTexture& rTexture, OpenGLTexture& rMask, const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX, const basegfx::B2DPoint& rY );
    void DrawAlphaTexture( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted = false, bool pPremultiplied = false );
    void DrawTextureDiff( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTextureWithMask( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& rPosAry );
    void DrawBlendedTexture( OpenGLTexture& rTexture, OpenGLTexture& rMask, OpenGLTexture& rAlpha, const SalTwoRect& rPosAry );
    void DrawMask( OpenGLTexture& rTexture, SalColor nMaskColor, const SalTwoRect& rPosAry );
    void DrawLinearGradient( const Gradient& rGradient, const Rectangle& rRect );
    void DrawAxialGradient( const Gradient& rGradient, const Rectangle& rRect );
    void DrawRadialGradient( const Gradient& rGradient, const Rectangle& rRect );

public:
    // get the width of the device
    GLfloat GetWidth() const { return mpProvider ? mpProvider->GetWidth() : 1; }

    // get the height of the device
    GLfloat GetHeight() const { return mpProvider ? mpProvider->GetHeight() : 1; }

    // check whether this instance is used for offscreen rendering
    bool IsOffscreen() const { return mpProvider == nullptr || mpProvider->IsOffScreen(); }

    // operations to do before painting
    void PreDraw();

    // operations to do after painting
    void PostDraw();

protected:
    bool AcquireContext();
    bool ReleaseContext();

    // retrieve the default context for offscreen rendering
    static rtl::Reference<OpenGLContext> GetDefaultContext();

    // create a new context for window rendering
    virtual rtl::Reference<OpenGLContext> CreateWinContext() = 0;

    // check whether the given context can be used by this instance
    virtual bool UseContext( const rtl::Reference<OpenGLContext> &pContext ) = 0;

public:
    OpenGLSalGraphicsImpl(SalGraphics& pParent, SalGeometryProvider *pProvider);
    virtual ~OpenGLSalGraphicsImpl ();

    rtl::Reference<OpenGLContext> GetOpenGLContext();

    virtual void Init() SAL_OVERRIDE;

    virtual void DeInit() SAL_OVERRIDE;

    virtual void freeResources() SAL_OVERRIDE;

    const vcl::Region& getClipRegion() const;
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
    void DoCopyBits(const SalTwoRect& rPosAry, OpenGLSalGraphicsImpl &rSrcImpl);

    virtual bool blendBitmap(
                const SalTwoRect&,
                const SalBitmap& rBitmap ) SAL_OVERRIDE;

    virtual bool blendAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rSrcBitmap,
                const SalBitmap& rMaskBitmap,
                const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;

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

    virtual OpenGLContext *beginPaint() SAL_OVERRIDE;
private:
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
