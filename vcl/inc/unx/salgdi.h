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

#ifndef INCLUDED_VCL_INC_UNX_SALGDI_H
#define INCLUDED_VCL_INC_UNX_SALGDI_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>

#include <tools/fract.hxx>

#include <vcl/salgtype.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/metric.hxx>

#include "salgdi.hxx"
#include "salgeom.hxx"
#include "sallayout.hxx"
#include "vclpluginapi.h"

#include <deque>
#include <memory>

/* From <X11/Intrinsic.h> */
typedef unsigned long Pixel;

class FontAttributes;
class FontSelectPattern;
class SalBitmap;
class SalColormap;
class SalDisplay;
class SalFrame;
class X11Pixmap;
class X11SalVirtualDevice;
class X11SalGraphicsImpl;
class X11OpenGLSalGraphicsImpl;
class X11OpenGLSalVirtualDevice;
class ServerFont;
class ImplLayoutArgs;
class ServerFontLayout;
class PhysicalFontCollection;
class PhysicalFontFace;
class SalGraphicsImpl;
class TextRenderImpl;

namespace basegfx {
    class B2DTrapezoid;
}

class VCLPLUG_GEN_PUBLIC X11SalGraphics : public SalGraphics
{
    friend class ServerFontLayout;
    friend class X11SalGraphicsImpl;
    friend class X11OpenGLSalGraphicsImpl;
    friend class X11CairoTextRender;

public:
                                    X11SalGraphics();
    virtual                         ~X11SalGraphics();

    void                            Init( SalFrame *pFrame, Drawable aDrawable, SalX11Screen nXScreen );
    void                            Init( X11SalVirtualDevice *pVirtualDevice, SalColormap* pColormap = nullptr, bool bDeleteColormap = false );
    void                            Init( X11OpenGLSalVirtualDevice *pVirtualDevice );
    void                            DeInit();

    virtual SalGraphicsImpl*        GetImpl() const override;
    inline  const SalDisplay*       GetDisplay() const;
    inline  Display*                GetXDisplay() const;
    inline  const SalVisual&        GetVisual() const;
    SalGeometryProvider*            GetGeometryProvider() const;
    inline  Drawable                GetDrawable() const { return hDrawable_; }
    void                            SetDrawable( Drawable d, SalX11Screen nXScreen );
    XRenderPictFormat*              GetXRenderFormat() const;
    inline  void                    SetXRenderFormat( XRenderPictFormat* pXRenderFormat ) { m_pXRenderFormat = pXRenderFormat; }
    inline  const SalColormap&      GetColormap() const { return *m_pColormap; }

    using SalGraphics::GetPixel;
    inline  Pixel                   GetPixel( SalColor nSalColor ) const;

    const SalX11Screen&             GetScreenNumber() const { return m_nXScreen; }

    // override all pure virtual methods
    virtual void                    GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
    virtual sal_uInt16              GetBitCount() const override;
    virtual long                    GetGraphicsWidth() const override;

    virtual void                    ResetClipRegion() override;
    virtual bool                    setClipRegion( const vcl::Region& ) override;

    virtual void                    SetLineColor() override;
    virtual void                    SetLineColor( SalColor nSalColor ) override;
    virtual void                    SetFillColor() override;

    virtual void                    SetFillColor( SalColor nSalColor ) override;

    virtual void                    SetXORMode( bool bSet ) override;

    virtual void                    SetROPLineColor( SalROPColor nROPColor ) override;
    virtual void                    SetROPFillColor( SalROPColor nROPColor ) override;

    virtual void                    SetTextColor( SalColor nSalColor ) override;
    virtual void                    SetFont( FontSelectPattern*, int nFallbackLevel ) override;
    virtual void                    GetFontMetric( ImplFontMetricDataPtr&, int nFallbackLevel ) override;
    virtual const FontCharMapPtr    GetFontCharMap() const override;
    virtual bool                    GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void                    GetDevFontList( PhysicalFontCollection* ) override;
    virtual void                    ClearDevFontCache() override;
    virtual bool                    AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;

    virtual bool                    CreateFontSubset(
                                        const OUString& rToFile,
                                        const PhysicalFontFace*,
                                        const sal_GlyphId* pGlyphIDs,
                                        const sal_uInt8* pEncoding,
                                        sal_Int32* pWidths,
                                        int nGlyphs,
                                        FontSubsetInfo& rInfo ) override;

    virtual const Ucs2SIntMap*      GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const**) override;

    virtual const void*             GetEmbedFontData(
                                        const PhysicalFontFace*,
                                        const sal_Ucs* pUnicodes,
                                        sal_Int32* pWidths,
                                        size_t nLen,
                                        FontSubsetInfo& rInfo,
                                        long* pDataLen ) override;

    virtual void                    FreeEmbedFontData( const void* pData, long nDataLen ) override;

    virtual void                    GetGlyphWidths(
                                        const PhysicalFontFace*,
                                        bool bVertical,
                                        std::vector< sal_Int32 >& rWidths,
                                        Ucs2UIntMap& rUnicodeEnc ) override;

    virtual bool                    GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) override;
    virtual bool                    GetGlyphOutline( sal_GlyphId nIndex, basegfx::B2DPolyPolygon& ) override;
    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) override;
    virtual void                    DrawServerFontLayout( const ServerFontLayout& ) override;

    virtual bool                    supportsOperation( OutDevSupportType ) const override;
    virtual void                    drawPixel( long nX, long nY ) override;
    virtual void                    drawPixel( long nX, long nY, SalColor nSalColor ) override;
    virtual void                    drawLine( long nX1, long nY1, long nX2, long nY2 ) override;
    virtual void                    drawRect( long nX, long nY, long nWidth, long nHeight ) override;
    virtual void                    drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
    virtual void                    drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;

    virtual void                    drawPolyPolygon(
                                        sal_uInt32 nPoly,
                                        const sal_uInt32* pPoints,
                                        PCONSTSALPOINT* pPtAry ) override;

    virtual bool                    drawPolyPolygon( const basegfx::B2DPolyPolygon&, double fTransparency ) override;

    virtual bool                    drawPolyLine(
                                        const basegfx::B2DPolygon&,
                                        double fTransparency,
                                        const basegfx::B2DVector& rLineWidth,
                                        basegfx::B2DLineJoin,
                                        css::drawing::LineCap,
                                        double fMiterMinimumAngle) override;

    virtual bool                    drawGradient( const tools::PolyPolygon&, const Gradient& ) override;

#if 1 // TODO: remove these obsolete methods
    virtual bool                    drawPolyLineBezier(
                                        sal_uInt32 nPoints,
                                        const SalPoint* pPtAry,
                                        const sal_uInt8* pFlgAry ) override;

    virtual bool                    drawPolygonBezier(
                                        sal_uInt32 nPoints,
                                        const SalPoint* pPtAry,
                                        const sal_uInt8* pFlgAry ) override;

    virtual bool                    drawPolyPolygonBezier(
                                        sal_uInt32 nPoly,
                                        const sal_uInt32* pPoints,
                                        const SalPoint* const* pPtAry,
                                        const sal_uInt8* const* pFlgAry ) override;
#endif

    virtual void                    copyArea(
                                        long nDestX,
                                        long nDestY,
                                        long nSrcX,
                                        long nSrcY,
                                        long nSrcWidth,
                                        long nSrcHeight,
                                        bool bWindowInvalidate ) override;

    virtual void                    copyBits(
                                        const SalTwoRect& rPosAry,
                                        SalGraphics* pSrcGraphics ) override;

    virtual void                    drawBitmap(
                                        const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap ) override;

    virtual void                    drawBitmap(
                                        const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rMaskBitmap ) override;

    virtual void                    drawMask(
                                        const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nMaskColor ) override;

    virtual SalBitmap*              getBitmap( long nX, long nY, long nWidth, long nHeight ) override;
    virtual SalColor                getPixel( long nX, long nY ) override;
    virtual void                    invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) override;
    virtual void                    invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) override;

    virtual bool                    drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize ) override;

    virtual bool                    blendBitmap(
                                        const SalTwoRect&,
                                        const SalBitmap& rBitmap ) override;

    virtual bool                    blendAlphaBitmap(
                                        const SalTwoRect&,
                                        const SalBitmap& rSrcBitmap,
                                        const SalBitmap& rMaskBitmap,
                                        const SalBitmap& rAlphaBitmap ) override;

    virtual bool                    drawAlphaBitmap(
                                        const SalTwoRect&,
                                        const SalBitmap& rSourceBitmap,
                                        const SalBitmap& rAlphaBitmap ) override;

    virtual bool                    drawTransformedBitmap(
                                        const basegfx::B2DPoint& rNull,
                                        const basegfx::B2DPoint& rX,
                                        const basegfx::B2DPoint& rY,
                                        const SalBitmap& rSourceBitmap,
                                        const SalBitmap* pAlphaBitmap) override;

    virtual bool                    drawAlphaRect(
                                        long nX, long nY, long nWidth,
                                        long nHeight, sal_uInt8 nTransparency ) override;

    virtual SystemGraphicsData      GetGraphicsData() const override;
    virtual bool                    SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any           GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;
    virtual SystemFontData          GetSysFontData( int nFallbackLevel ) const override;

#if ENABLE_CAIRO_CANVAS
    void clipRegion(cairo_t* cr);
#endif // ENABLE_CAIRO_CANVAS

    bool TryRenderCachedNativeControl(ControlCacheKey& aControlCacheKey,
                                      int nX, int nY);

    bool RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                     ControlCacheKey& aControlCacheKey);

    // fill a pixmap from a screen region
    void                            FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY );

    // render a pixmap to the screen
    bool                            RenderPixmapToScreen( X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY );


    /*  use to handle GraphicsExpose/NoExpose after XCopyArea & friends
     *  if pFrame is not NULL, corresponding Paint events are generated
     *  and dispatched to pFrame
     *
     *  it is imperative to eat up graphics exposes even in case you don't need
     *  them because the next one using XCopyArea can depend on them
     */
    void                            YieldGraphicsExpose();

    cairo_t* getCairoContext();
    static void releaseCairoContext(cairo_t* cr);


    // do XCopyArea or XGet/PutImage depending on screen numbers
    // signature is like XCopyArea with screen numbers added
    static void                     CopyScreenArea(
                                        Display* pDisplay,
                                        Drawable aSrc, SalX11Screen nXScreenSrc, int nSrcDepth,
                                        Drawable aDest, SalX11Screen nXScreenDest, int nDestDepth,
                                        GC aDestGC,
                                        int src_x, int src_y,
                                        unsigned int w, unsigned int h,
                                        int dest_x, int dest_y );

    static void                     releaseGlyphPeer();

protected:
    using SalGraphics::SetClipRegion;
    void                            SetClipRegion( GC pGC, Region pXReg = nullptr ) const;
    bool                            GetDitherPixmap ( SalColor nSalColor );

    using SalGraphics::DrawBitmap;

    GC                              GetFontGC();

    void                            freeResources();

protected:
    SalFrame*                       m_pFrame; // the SalFrame which created this Graphics or NULL
    SalVirtualDevice*               m_pVDev;  // the SalVirtualDevice which created this Graphics or NULL

    const SalColormap*              m_pColormap;
    SalColormap*                    m_pDeleteColormap;
    Drawable                        hDrawable_;     // use
    SalX11Screen                    m_nXScreen;
    mutable XRenderPictFormat*      m_pXRenderFormat;
    XID                             m_aXRenderPicture;

    Region                          pPaintRegion_;
    Region                          mpClipRegion;
#if ENABLE_CAIRO_CANVAS
    vcl::Region                     maClipRegion;
    SalColor                        mnPenColor;
    SalColor                        mnFillColor;
#endif // ENABLE_CAIRO_CANVAS

    GC                              pFontGC_;       // Font attributes
    Pixel                           nTextPixel_;

    Pixmap                          hBrush_;        // Dither

    bool                            bWindow_ : 1;       // is Window
    bool                            bPrinter_ : 1;      // is Printer
    bool                            bVirDev_ : 1;       // is VirDev
    bool                            bFontGC_ : 1;       // is Font GC valid
    bool                            m_bOpenGL : 1;

private:
    std::unique_ptr<SalGraphicsImpl> mxImpl;
    std::unique_ptr<TextRenderImpl> mxTextRenderImpl;

};

inline const SalDisplay *X11SalGraphics::GetDisplay() const
{ return GetColormap().GetDisplay(); }

inline const SalVisual& X11SalGraphics::GetVisual() const
{ return GetColormap().GetVisual(); }

inline Display *X11SalGraphics::GetXDisplay() const
{ return GetColormap().GetXDisplay(); }

inline Pixel X11SalGraphics::GetPixel( SalColor nSalColor ) const
{ return GetColormap().GetPixel( nSalColor ); }

#endif // INCLUDED_VCL_INC_UNX_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
