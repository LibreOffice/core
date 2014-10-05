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

#include "vcl/salgtype.hxx"
#include "vcl/vclenum.hxx"

#include "salgdi.hxx"
#include "sallayout.hxx"
#include "vclpluginapi.h"

#include <deque>

class ImplFontMetricData;
class FontSelectPattern;
class SalBitmap;
class SalColormap;
class SalDisplay;
class SalFrame;
class X11SalVirtualDevice;
class SalPolyLine;
class PspSalPrinter;
class PspSalInfoPrinter;
class ServerFont;
class ImplLayoutArgs;
class ServerFontLayout;
class PhysicalFontCollection;
class PhysicalFontFace;

namespace basegfx {
    class B2DTrapezoid;
}

typedef struct FT_FaceRec_* FT_Face;

class CairoFontsCache
{
public:
    struct CacheId
    {
        FT_Face maFace;
        const void *mpOptions;
        bool mbEmbolden;
        bool mbVerticalMetrics;
        bool operator ==(const CacheId& rOther) const
        {
            return maFace == rOther.maFace &&
                mpOptions == rOther.mpOptions &&
                mbEmbolden == rOther.mbEmbolden &&
                mbVerticalMetrics == rOther.mbVerticalMetrics;
        }
    };
private:
    static int mnRefCount;
    typedef std::deque< std::pair<void *, CacheId> > LRUFonts;
    static LRUFonts maLRUFonts;
public:
    CairoFontsCache();
    static void  CacheFont(void *pFont, const CacheId &rId);
    static void* FindCachedFont(const CacheId &rId);
    ~CairoFontsCache();
};

class VCLPLUG_GEN_PUBLIC X11SalGraphics : public SalGraphics
{
    friend class            ServerFontLayout;
protected:
    SalFrame*               m_pFrame; // the SalFrame which created this Graphics or NULL
    X11SalVirtualDevice*    m_pVDev;  // the SalVirtualDevice which created this Graphics or NULL

    const SalColormap*      m_pColormap;
    SalColormap    *m_pDeleteColormap;
    Drawable        hDrawable_;     // use
    SalX11Screen    m_nXScreen;
    mutable XRenderPictFormat* m_pXRenderFormat;
    XID             m_aXRenderPicture;
    CairoFontsCache m_aCairoFontsCache;

    Region          pPaintRegion_;
    Region          mpClipRegion;

    GC              pPenGC_;        // Pen attributes
    SalColor        nPenColor_;
    Pixel           nPenPixel_;

    GC              pFontGC_;       // Font attributes
    ServerFont*             mpServerFont[ MAX_FALLBACK ];

    SalColor        nTextColor_;
    Pixel           nTextPixel_;

    bool            bDisableGraphite_;

    GC              pBrushGC_;      // Brush attributes
    SalColor        nBrushColor_;
    Pixel           nBrushPixel_;
    Pixmap          hBrush_;        // Dither

    GC              pMonoGC_;
    GC              pCopyGC_;
    GC              pMaskGC_;
    GC              pInvertGC_;
    GC              pInvert50GC_;
    GC              pStippleGC_;
    GC              pTrackingGC_;

    bool            bWindow_ : 1;       // is Window
    bool            bPrinter_ : 1;      // is Printer
    bool            bVirDev_ : 1;       // is VirDev
    bool            bPenGC_ : 1;        // is Pen GC valid
    bool            bFontGC_ : 1;       // is Font GC valid
    bool            bBrushGC_ : 1;      // is Brush GC valid
    bool            bMonoGC_ : 1;       // is Mono GC valid
    bool            bCopyGC_ : 1;       // is Copy GC valid
    bool            bInvertGC_ : 1;     // is Invert GC valid
    bool            bInvert50GC_ : 1;   // is Invert50 GC valid
    bool            bStippleGC_ : 1;    // is Stipple GC valid
    bool            bTrackingGC_ : 1;   // is Tracking GC valid
    bool            bXORMode_ : 1;      // is ROP XOR Mode set
    bool            bDitherBrush_ : 1;  // is solid or tile

    using SalGraphics::SetClipRegion;
    void            SetClipRegion( GC          pGC,
                                   Region      pXReg = NULL ) const;

    GC              GetTrackingGC();
    GC              GetInvertGC();
    GC              GetInvert50GC();
    GC              CreateGC( Drawable      hDrawable,
                              unsigned long nMask = GCGraphicsExposures );
    GC              SelectPen();
    GC              SelectBrush();
    void            DrawLines( sal_uIntPtr              nPoints,
                               const SalPolyLine &rPoints,
                               GC                 pGC,
                               bool bClose
                               );
    bool            GetDitherPixmap ( SalColor nSalColor );

    inline  GC              GetMonoGC( Pixmap hPixmap );
    inline  GC              GetCopyGC();
    inline  GC              GetStippleGC();

    using SalGraphics::DrawBitmap;
    void            DrawBitmap( const SalTwoRect& rPosAry,
                                SalGraphics      *pThis,
                                const SalBitmap  &rSalBitmap,
                                const SalBitmap  &rTransparentBitmap,
                                SalColor          nTransparentColor );

    GC                      GetFontGC();
    bool                    setFont( const FontSelectPattern* pEntry, int nFallbackLevel );

    void                    drawMaskedBitmap( const SalTwoRect& rPosAry,
                                              const SalBitmap& rSalBitmap,
                                              const SalBitmap& rTransparentBitmap );

protected:
    void                    DrawPrinterString( const SalLayout& );

    void                    DrawServerFontString( const ServerFontLayout& );

    void freeResources();
public:
                            X11SalGraphics();
    virtual             ~X11SalGraphics();

            void            Init( SalFrame *pFrame, Drawable aDrawable, SalX11Screen nXScreen );
            void            Init( X11SalVirtualDevice *pVirtualDevice, SalColormap* pColormap = NULL, bool bDeleteColormap = false );
            void            Init( class ImplSalPrinterData *pPrinter );
            void            DeInit();

    inline  const SalDisplay*   GetDisplay() const;
    inline  Display*            GetXDisplay() const;
    inline  const SalVisual&    GetVisual() const;
    inline  Drawable        GetDrawable() const { return hDrawable_; }
    void                    SetDrawable( Drawable d, SalX11Screen nXScreen );
    XID                     GetXRenderPicture();
    XRenderPictFormat*      GetXRenderFormat() const;
    inline  void            SetXRenderFormat( XRenderPictFormat* pXRenderFormat ) { m_pXRenderFormat = pXRenderFormat; }
    inline  const SalColormap&    GetColormap() const { return *m_pColormap; }
    using SalGraphics::GetPixel;
    inline  Pixel           GetPixel( SalColor nSalColor ) const;

    SalX11Screen            GetScreenNumber() const { return m_nXScreen; }

    // overload all pure virtual methods
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) SAL_OVERRIDE;
    virtual sal_uInt16          GetBitCount() const SAL_OVERRIDE;
    virtual long            GetGraphicsWidth() const SAL_OVERRIDE;
    virtual long            GetGraphicsHeight() const;

    virtual void            ResetClipRegion() SAL_OVERRIDE;
    virtual bool            setClipRegion( const vcl::Region& ) SAL_OVERRIDE;

    virtual void            SetLineColor() SAL_OVERRIDE;
    virtual void            SetLineColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual void            SetFillColor() SAL_OVERRIDE;

    virtual void            SetFillColor( SalColor nSalColor ) SAL_OVERRIDE;

    virtual void            SetXORMode( bool bSet, bool ) SAL_OVERRIDE;

    virtual void            SetROPLineColor( SalROPColor nROPColor ) SAL_OVERRIDE;
    virtual void            SetROPFillColor( SalROPColor nROPColor ) SAL_OVERRIDE;

    virtual void            SetTextColor( SalColor nSalColor ) SAL_OVERRIDE;
    virtual sal_uInt16          SetFont( FontSelectPattern*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void            GetFontMetric( ImplFontMetricData*, int nFallbackLevel ) SAL_OVERRIDE;
    virtual const ImplFontCharMapPtr GetImplFontCharMap() const SAL_OVERRIDE;
    virtual bool GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const SAL_OVERRIDE;
    virtual void            GetDevFontList( PhysicalFontCollection* ) SAL_OVERRIDE;
    virtual void ClearDevFontCache() SAL_OVERRIDE;
    virtual bool            AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) SAL_OVERRIDE;
    virtual bool            CreateFontSubset( const OUString& rToFile,
                                              const PhysicalFontFace*,
                                              sal_GlyphId* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              ) SAL_OVERRIDE;
    virtual const Ucs2SIntMap* GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded ) SAL_OVERRIDE;
    virtual const void* GetEmbedFontData( const PhysicalFontFace*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen ) SAL_OVERRIDE;
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen ) SAL_OVERRIDE;
    virtual void            GetGlyphWidths( const PhysicalFontFace*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) SAL_OVERRIDE;
    virtual bool            GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) SAL_OVERRIDE;
    virtual bool            GetGlyphOutline( sal_GlyphId nIndex, ::basegfx::B2DPolyPolygon& ) SAL_OVERRIDE;
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) SAL_OVERRIDE;
    virtual void            DrawServerFontLayout( const ServerFontLayout& ) SAL_OVERRIDE;
    virtual bool            supportsOperation( OutDevSupportType ) const SAL_OVERRIDE;
    virtual void            drawPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor ) SAL_OVERRIDE;
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 ) SAL_OVERRIDE;
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    void                    drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry, bool bClose );
    virtual void            drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
    virtual void            drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry ) SAL_OVERRIDE;
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency ) SAL_OVERRIDE;
    virtual bool            drawPolyLine(
        const ::basegfx::B2DPolygon&,
        double fTransparency,
        const ::basegfx::B2DVector& rLineWidth,
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap) SAL_OVERRIDE;
    virtual bool            drawFilledTrapezoids( const ::basegfx::B2DTrapezoid*, int nTrapCount, double fTransparency );

#if 1 // TODO: remove these obselete methods
    virtual bool        drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
    virtual bool        drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
    virtual bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry ) SAL_OVERRIDE;
#endif

    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void            copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor ) SAL_OVERRIDE;
    virtual void            drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rMaskBitmap ) SAL_OVERRIDE;
    virtual void            drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor ) SAL_OVERRIDE;
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual SalColor        getPixel( long nX, long nY ) SAL_OVERRIDE;
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) SAL_OVERRIDE;
    virtual void            invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) SAL_OVERRIDE;

    virtual bool            drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uIntPtr nSize ) SAL_OVERRIDE;

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;
    virtual bool drawTransformedBitmap(
        const basegfx::B2DPoint& rNull,
        const basegfx::B2DPoint& rX,
        const basegfx::B2DPoint& rY,
        const SalBitmap& rSourceBitmap,
        const SalBitmap* pAlphaBitmap) SAL_OVERRIDE;

    virtual bool            drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency ) SAL_OVERRIDE;

    virtual SystemGraphicsData GetGraphicsData() const SAL_OVERRIDE;
    virtual SystemFontData     GetSysFontData( int nFallbacklevel ) const SAL_OVERRIDE;

    /*  use to handle GraphicsExpose/NoExpose after XCopyArea & friends
     *  if pFrame is not NULL, corresponding Paint events are generated
     *  and dispatched to pFrame
     *
     *  it is imperative to eat up graphics exposes even in case you don't need
     *  them because the next one using XCopyArea can depend on them
     */
    void YieldGraphicsExpose();

    // do XCopyArea or XGet/PutImage depending on screen numbers
    // signature is like XCopyArea with screen numbers added
    static void CopyScreenArea( Display* pDisplay,
                                Drawable aSrc, SalX11Screen nXScreenSrc, int nSrcDepth,
                                Drawable aDest, SalX11Screen nXScreenDest, int nDestDepth,
                                GC aDestGC,
                                int src_x, int src_y,
                                unsigned int w, unsigned int h,
                                int dest_x, int dest_y );
    static void releaseGlyphPeer();
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
