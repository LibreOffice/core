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

#include <prex.h>
#include <postx.h>
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

class ImplFontMetricData;
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
class PspSalPrinter;
class PspSalInfoPrinter;
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
    void                            Init( X11SalVirtualDevice *pVirtualDevice, SalColormap* pColormap = NULL, bool bDeleteColormap = false );
    void                            Init( X11OpenGLSalVirtualDevice *pVirtualDevice );
    void                            Init( class ImplSalPrinterData *pPrinter );
    void                            DeInit();

    virtual SalGraphicsImpl*        GetImpl() const SAL_OVERRIDE;
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

    SalX11Screen                    GetScreenNumber() const { return m_nXScreen; }

    // override all pure virtual methods
    virtual void                    GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) SAL_OVERRIDE;
    virtual sal_uInt16              GetBitCount() const SAL_OVERRIDE;
    virtual long                    GetGraphicsWidth() const SAL_OVERRIDE;

    virtual void                    ResetClipRegion() SAL_OVERRIDE;
    virtual bool                    setClipRegion( const vcl::Region& ) SAL_OVERRIDE;

    virtual void                    SetLineColor() SAL_OVERRIDE;
    virtual void                    SetLineColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual void                    SetFillColor() SAL_OVERRIDE;

    virtual void                    SetFillColor( SalColor nSalColor ) SAL_OVERRIDE;

    virtual void                    SetXORMode( bool bSet, bool ) SAL_OVERRIDE;

    virtual void                    SetROPLineColor( SalROPColor nROPColor ) SAL_OVERRIDE;
    virtual void                    SetROPFillColor( SalROPColor nROPColor ) SAL_OVERRIDE;

    virtual void                    SetTextColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual sal_uInt16              SetFont( FontSelectPattern*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void                    GetFontMetric( ImplFontMetricData*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual const FontCharMapPtr    GetFontCharMap() const SAL_OVERRIDE;
    virtual bool                    GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const SAL_OVERRIDE;
    virtual void                    GetDevFontList( PhysicalFontCollection* ) SAL_OVERRIDE;
    virtual void                    ClearDevFontCache() SAL_OVERRIDE;
    virtual bool                    AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) SAL_OVERRIDE;

    virtual bool                    CreateFontSubset(
                                        const OUString& rToFile,
                                        const PhysicalFontFace*,
                                        const sal_GlyphId* pGlyphIDs,
                                        const sal_uInt8* pEncoding,
                                        sal_Int32* pWidths,
                                        int nGlyphs,
                                        FontSubsetInfo& rInfo ) SAL_OVERRIDE;

    virtual const Ucs2SIntMap*      GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const**) SAL_OVERRIDE;

    virtual const void*             GetEmbedFontData(
                                        const PhysicalFontFace*,
                                        const sal_Ucs* pUnicodes,
                                        sal_Int32* pWidths,
                                        size_t nLen,
                                        FontSubsetInfo& rInfo,
                                        long* pDataLen ) SAL_OVERRIDE;

    virtual void                    FreeEmbedFontData( const void* pData, long nDataLen ) SAL_OVERRIDE;

    virtual void                    GetGlyphWidths(
                                        const PhysicalFontFace*,
                                        bool bVertical,
                                        Int32Vector& rWidths,
                                        Ucs2UIntMap& rUnicodeEnc ) SAL_OVERRIDE;

    virtual bool                    GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) SAL_OVERRIDE;
    virtual bool                    GetGlyphOutline( sal_GlyphId nIndex, ::basegfx::B2DPolyPolygon& ) SAL_OVERRIDE;
    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void                    DrawServerFontLayout( const ServerFontLayout& ) SAL_OVERRIDE;

    virtual bool                    supportsOperation( OutDevSupportType ) const SAL_OVERRIDE;
    virtual void                    drawPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void                    drawPixel( long nX, long nY, SalColor nSalColor ) SAL_OVERRIDE;
    virtual void                    drawLine( long nX1, long nY1, long nX2, long nY2 ) SAL_OVERRIDE;
    virtual void                    drawRect( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                    drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
    virtual void                    drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;

    virtual void                    drawPolyPolygon(
                                        sal_uInt32 nPoly,
                                        const sal_uInt32* pPoints,
                                        PCONSTSALPOINT* pPtAry ) SAL_OVERRIDE;

    virtual bool                    drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency ) SAL_OVERRIDE;

    virtual bool                    drawPolyLine(
                                        const ::basegfx::B2DPolygon&,
                                        double fTransparency,
                                        const ::basegfx::B2DVector& rLineWidth,
                                        basegfx::B2DLineJoin,
                                        com::sun::star::drawing::LineCap ) SAL_OVERRIDE;

    virtual bool                    drawGradient( const tools::PolyPolygon&, const Gradient& ) SAL_OVERRIDE;

#if 1 // TODO: remove these obselete methods
    virtual bool                    drawPolyLineBezier(
                                        sal_uInt32 nPoints,
                                        const SalPoint* pPtAry,
                                        const sal_uInt8* pFlgAry ) SAL_OVERRIDE;

    virtual bool                    drawPolygonBezier(
                                        sal_uInt32 nPoints,
                                        const SalPoint* pPtAry,
                                        const sal_uInt8* pFlgAry ) SAL_OVERRIDE;

    virtual bool                    drawPolyPolygonBezier(
                                        sal_uInt32 nPoly,
                                        const sal_uInt32* pPoints,
                                        const SalPoint* const* pPtAry,
                                        const sal_uInt8* const* pFlgAry ) SAL_OVERRIDE;
#endif

    virtual void                    copyArea(
                                        long nDestX,
                                        long nDestY,
                                        long nSrcX,
                                        long nSrcY,
                                        long nSrcWidth,
                                        long nSrcHeight,
                                        sal_uInt16 nFlags ) SAL_OVERRIDE;

    virtual void                    copyBits(
                                        const SalTwoRect& rPosAry,
                                        SalGraphics* pSrcGraphics ) SAL_OVERRIDE;

    virtual void                    drawBitmap(
                                        const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap ) SAL_OVERRIDE;

    virtual void                    drawBitmap(
                                        const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor ) SAL_OVERRIDE;

    virtual void                    drawBitmap(
                                        const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rMaskBitmap ) SAL_OVERRIDE;

    virtual void                    drawMask(
                                        const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nMaskColor ) SAL_OVERRIDE;

    virtual SalBitmap*              getBitmap( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual SalColor                getPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void                    invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) SAL_OVERRIDE;
    virtual void                    invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) SAL_OVERRIDE;

    virtual bool                    drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize ) SAL_OVERRIDE;

    virtual bool                    blendBitmap(
                                        const SalTwoRect&,
                                        const SalBitmap& rBitmap ) SAL_OVERRIDE;

    virtual bool                    blendAlphaBitmap(
                                        const SalTwoRect&,
                                        const SalBitmap& rSrcBitmap,
                                        const SalBitmap& rMaskBitmap,
                                        const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;

    virtual bool                    drawAlphaBitmap(
                                        const SalTwoRect&,
                                        const SalBitmap& rSourceBitmap,
                                        const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;

    virtual bool                    drawTransformedBitmap(
                                        const basegfx::B2DPoint& rNull,
                                        const basegfx::B2DPoint& rX,
                                        const basegfx::B2DPoint& rY,
                                        const SalBitmap& rSourceBitmap,
                                        const SalBitmap* pAlphaBitmap) SAL_OVERRIDE;

    virtual bool                    drawAlphaRect(
                                        long nX, long nY, long nWidth,
                                        long nHeight, sal_uInt8 nTransparency ) SAL_OVERRIDE;

    virtual SystemGraphicsData      GetGraphicsData() const SAL_OVERRIDE;
    virtual bool                    SupportsCairo() const SAL_OVERRIDE;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const SAL_OVERRIDE;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const SAL_OVERRIDE;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const SAL_OVERRIDE;
    virtual css::uno::Any           GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const SAL_OVERRIDE;
    virtual SystemFontData          GetSysFontData( int nFallbackLevel ) const SAL_OVERRIDE;

    virtual OpenGLContext          *BeginPaint() SAL_OVERRIDE;

    bool TryRenderCachedNativeControl(ControlCacheKey& aControlCacheKey,
                                      int nX, int nY);

    bool RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                     ControlCacheKey& aControlCacheKey);

    // fill a pixmap from a screen region
    bool                            FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY );

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
    void                            SetClipRegion( GC pGC, Region pXReg = NULL ) const;
    bool                            GetDitherPixmap ( SalColor nSalColor );

    using SalGraphics::DrawBitmap;
    void                            DrawBitmap(
                                        const SalTwoRect& rPosAry,
                                        SalGraphics      *pThis,
                                        const SalBitmap  &rSalBitmap,
                                        const SalBitmap  &rTransparentBitmap,
                                        SalColor          nTransparentColor );

    GC                              GetFontGC();
    void                            DrawPrinterString( const SalLayout& );
    void                            DrawServerFontString( const ServerFontLayout& );

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

    GC                              pFontGC_;       // Font attributes
    Pixel                           nTextPixel_;

    Pixmap                          hBrush_;        // Dither

    bool                            bWindow_ : 1;       // is Window
    bool                            bPrinter_ : 1;      // is Printer
    bool                            bVirDev_ : 1;       // is VirDev
    bool                            bFontGC_ : 1;       // is Font GC valid

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

#ifdef DBG_UTIL
#define stderr0( s )            fprintf( stderr, s )
#else
#define stderr0( s )            ;
#endif

#endif // INCLUDED_VCL_INC_UNX_SALGDI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
