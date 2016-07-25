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

#include <vcl/dllapi.h>
#include <vcl/opengl/OpenGLContext.hxx>

#include "regionband.hxx"
#include "salgeom.hxx"
#include "salgdiimpl.hxx"
#include "opengl/framebuffer.hxx"
#include "opengl/program.hxx"
#include "opengl/texture.hxx"
#include "opengl/RenderList.hxx"

#include <memory>

class SalFrame;
class SalVirtualDevice;
class OpenGLTests;

namespace basegfx
{
class B2DTrapezoid;
};

namespace tools
{
    class Polygon;
    class PolyPolygon;
}

struct TextureCombo
{
    std::unique_ptr<OpenGLTexture> mpTexture;
    std::unique_ptr<OpenGLTexture> mpMask;
};

class OpenGLFlushIdle;

class VCL_DLLPUBLIC OpenGLSalGraphicsImpl : public SalGraphicsImpl
{
    friend class OpenGLTests;
protected:

    /// This context is solely for blitting maOffscreenTex
    rtl::Reference<OpenGLContext> mpWindowContext;

    /// This context is whatever is most convenient to render
    /// to maOffscreenTex with.
    rtl::Reference<OpenGLContext> mpContext;

    SalGraphics& mrParent;
    /// Pointer to the SalFrame or SalVirtualDevice
    SalGeometryProvider* mpProvider;
    OpenGLProgram* mpProgram;

    /// This idle handler is used to swap buffers after rendering.
    OpenGLFlushIdle *mpFlush;

    // clipping
    vcl::Region maClipRegion;
    bool mbUseScissor;
    bool mbUseStencil;

    bool mbXORMode;

    /**
     * All rendering happens to this off-screen texture. For
     * non-virtual devices, ie. windows - we will blit it and
     * swapBuffers later.
     */
    OpenGLTexture maOffscreenTex;

    SalColor mnLineColor;
    SalColor mnFillColor;
#ifdef DBG_UTIL
    bool mProgramIsSolidColor;
#endif
    sal_uInt32 mnDrawCount;
    sal_uInt32 mnDrawCountAtFlush;
    SalColor mProgramSolidColor;
    double mProgramSolidTransparency;

    std::unique_ptr<RenderList> mpRenderList;

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
    bool UseSolid();
    bool UseLine(SalColor nColor, double fTransparency, GLfloat fLineWidth, bool bUseAA);
    bool UseLine(GLfloat fLineWidth, bool bUseAA);
    bool UseInvert50();
    bool UseInvert(SalInvert nFlags);

    void DrawConvexPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry, bool blockAA = false );
    void DrawConvexPolygon( const tools::Polygon& rPolygon, bool blockAA = false );
    void DrawTrapezoid( const basegfx::B2DTrapezoid& trapezoid, bool blockAA );
    void DrawRect( long nX, long nY, long nWidth, long nHeight );
    void DrawRect( const Rectangle& rRect );
    void DrawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry );
    void DrawLineSegment(float x1, float y1, float x2, float y2);
    void DrawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, bool blockAA = false );
    void DrawRegionBand( const RegionBand& rRegion );
    void DrawTextureRect( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTexture( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTransformedTexture( OpenGLTexture& rTexture, OpenGLTexture& rMask, const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX, const basegfx::B2DPoint& rY );
    void DrawAlphaTexture( OpenGLTexture& rTexture, const SalTwoRect& rPosAry, bool bInverted, bool pPremultiplied );
    void DrawTextureDiff( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& rPosAry, bool bInverted = false );
    void DrawTextureWithMask( OpenGLTexture& rTexture, OpenGLTexture& rMask, const SalTwoRect& rPosAry );
    void DrawBlendedTexture( OpenGLTexture& rTexture, OpenGLTexture& rMask, OpenGLTexture& rAlpha, const SalTwoRect& rPosAry );
    void DrawMask( OpenGLTexture& rTexture, SalColor nMaskColor, const SalTwoRect& rPosAry );
    void DrawLinearGradient( const Gradient& rGradient, const Rectangle& rRect );
    void DrawAxialGradient( const Gradient& rGradient, const Rectangle& rRect );
    void DrawRadialGradient( const Gradient& rGradient, const Rectangle& rRect );
    void DeferredTextDraw(OpenGLTexture& rTexture, const SalColor nMaskColor, const SalTwoRect& rPosAry);

    void FlushDeferredDrawing();
    bool FlushLinesOrTriangles(DrawShaderType eType, RenderParameters& rParameters);

public:
    // get the width of the device
    GLfloat GetWidth() const { return mpProvider ? mpProvider->GetWidth() : 1; }

    // get the height of the device
    GLfloat GetHeight() const { return mpProvider ? mpProvider->GetHeight() : 1; }

    /**
     * check whether this instance is used for offscreen (Virtual Device)
     * rendering ie. does it need its own context.
     */
    bool IsOffscreen() const { return mpProvider == nullptr || mpProvider->IsOffScreen(); }

    /// Oddly not all operations obey the XOR option.
    enum XOROption { IGNORE_XOR, IMPLEMENT_XOR };

    // initialize pre-draw state
    void InitializePreDrawState(XOROption eOpt = IGNORE_XOR);

    // operations to do before painting
    void PreDraw(XOROption eOpt = IGNORE_XOR);

    // operations to do after painting
    void PostDraw();

    void PostBatchDraw();

protected:
    bool AcquireContext(bool bForceCreate = false);
    bool ReleaseContext();

    /// create a new context for rendering to the underlying window
    virtual rtl::Reference<OpenGLContext> CreateWinContext() = 0;

    /// check whether the given context can be used for off-screen rendering
    static bool UseContext( const rtl::Reference<OpenGLContext> &pContext )
    {
        return pContext->isInitialized() &&  // not released by the OS etc.
               pContext->isVCLOnly();
    }

public:
    OpenGLSalGraphicsImpl(SalGraphics& pParent, SalGeometryProvider *pProvider);
    virtual ~OpenGLSalGraphicsImpl ();

    rtl::Reference<OpenGLContext> GetOpenGLContext();

    virtual void Init() override;

    virtual void DeInit() override;

    virtual void freeResources() override;

    const vcl::Region& getClipRegion() const;
    virtual bool setClipRegion( const vcl::Region& ) override;

    //
    // get the depth of the device
    virtual sal_uInt16 GetBitCount() const override;

    // get the width of the device
    virtual long GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void ResetClipRegion() override;

    // set the line color to transparent (= don't draw lines)

    virtual void SetLineColor() override;

    // set the line color to a specific color
    virtual void SetLineColor( SalColor nSalColor ) override;

    // set the fill color to transparent (= don't fill)
    virtual void SetFillColor() override;

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void SetFillColor( SalColor nSalColor ) override;

    // enable/disable XOR drawing
    virtual void SetXORMode( bool bSet ) override;

    // set line color for raster operations
    virtual void SetROPLineColor( SalROPColor nROPColor ) override;

    // set fill color for raster operations
    virtual void SetROPFillColor( SalROPColor nROPColor ) override;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void drawPixel( long nX, long nY ) override;
    virtual void drawPixel( long nX, long nY, SalColor nSalColor ) override;

    virtual void drawLine( long nX1, long nY1, long nX2, long nY2 ) override;

    virtual void drawRect( long nX, long nY, long nWidth, long nHeight ) override;

    virtual void drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;

    virtual void drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;

    virtual void drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) override;
    virtual bool drawPolyPolygon( const basegfx::B2DPolyPolygon&, double fTransparency ) override;

    virtual bool drawPolyLine(
                const basegfx::B2DPolygon&,
                double fTransparency,
                const basegfx::B2DVector& rLineWidths,
                basegfx::B2DLineJoin,
                css::drawing::LineCap,
                double fMiterMinimumAngle) override;

    virtual bool drawPolyLineBezier(
                sal_uInt32 nPoints,
                const SalPoint* pPtAry,
                const sal_uInt8* pFlgAry ) override;

    virtual bool drawPolygonBezier(
                sal_uInt32 nPoints,
                const SalPoint* pPtAry,
                const sal_uInt8* pFlgAry ) override;

    virtual bool drawPolyPolygonBezier(
                sal_uInt32 nPoly,
                const sal_uInt32* pPoints,
                const SalPoint* const* pPtAry,
                const sal_uInt8* const* pFlgAry ) override;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void copyArea(
                long nDestX, long nDestY,
                long nSrcX, long nSrcY,
                long nSrcWidth, long nSrcHeight,
                bool bWindowInvalidate ) override;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    void DoCopyBits(const SalTwoRect& rPosAry, OpenGLSalGraphicsImpl &rSrcImpl);

    virtual bool blendBitmap(
                const SalTwoRect&,
                const SalBitmap& rBitmap ) override;

    virtual bool blendAlphaBitmap(
                const SalTwoRect&,
                const SalBitmap& rSrcBitmap,
                const SalBitmap& rMaskBitmap,
                const SalBitmap& rAlphaBitmap ) override;

    virtual void drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) override;

    virtual void drawBitmap(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                const SalBitmap& rMaskBitmap ) override;

    virtual void drawMask(
                const SalTwoRect& rPosAry,
                const SalBitmap& rSalBitmap,
                SalColor nMaskColor ) override;

    virtual SalBitmap* getBitmap( long nX, long nY, long nWidth, long nHeight ) override;

    virtual SalColor getPixel( long nX, long nY ) override;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void invert(
                long nX, long nY,
                long nWidth, long nHeight,
                SalInvert nFlags) override;

    virtual void invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) override;

    virtual bool drawEPS(
                long nX, long nY,
                long nWidth, long nHeight,
                void* pPtr,
                sal_uLong nSize ) override;

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
                const SalBitmap* pAlphaBitmap) override;

    /** Render solid rectangle with given transparency

      @param nX             Top left coordinate of rectangle

      @param nY             Bottom right coordinate of rectangle

      @param nWidth         Width of rectangle

      @param nHeight        Height of rectangle

      @param nTransparency  Transparency value (0-255) to use. 0 blits and opaque, 255 a
                            fully transparent rectangle

      @returns true if successfully drawn, false if not able to draw rectangle
     */
    virtual bool drawAlphaRect(
                    long nX, long nY,
                    long nWidth, long nHeight,
                    sal_uInt8 nTransparency ) override;

    virtual bool drawGradient(const tools::PolyPolygon& rPolygon, const Gradient& rGradient) override;

    /// queue an idle flush of contents of the back-buffer to the screen
    void flush();

public:
    /// do flush of contents of the back-buffer to the screen & swap.
    void doFlush();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
