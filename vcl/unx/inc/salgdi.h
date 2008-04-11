/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salgdi.h,v $
 * $Revision: 1.45 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H


// -=-= exports -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class   SalFontCacheItem;

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "salstd.hxx"
#include "vcl/salgdi.hxx"
#include "vcl/salgtype.hxx"
#include "tools/fract.hxx"
#include "vcl/dllapi.h"
#include <deque>
#include "xfont.hxx"

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
struct  ImplFontMetricData;
struct  ImplFontSelectData;
class   SalBitmap;
class   SalColormap;
class   SalDisplay;
class   SalFrame;
class   X11SalVirtualDevice;
class   SalPolyLine;
class   PspSalPrinter;
class   PspSalInfoPrinter;
class   ServerFont;
class   ImplLayoutArgs;
class   X11FontLayout;
class   ServerFontLayout;

// -=-= SalGraphicsData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class CairoFontsCache
{
private:
    static int mnRefCount;
    typedef std::deque< std::pair<void *, void*> > LRUFonts;
    static LRUFonts maLRUFonts;
public:
    CairoFontsCache();
    static void  CacheFont(void *pFont, void *pId);
    static void* FindCachedFont(void *pId);
    ~CairoFontsCache();
};

class VCL_DLLPUBLIC X11SalGraphics : public SalGraphics
{
    friend class            X11FontLayout;
    friend class            ServerFontLayout;
protected:
    SalFrame*               m_pFrame; // the SalFrame which created this Graphics or NULL
    X11SalVirtualDevice*    m_pVDev;  // the SalVirtualDevice which created this Graphics or NULL

    const SalColormap*      m_pColormap;
    SalColormap    *m_pDeleteColormap;
    Drawable        hDrawable_;     // use
    int             m_nScreen;
    void*           pRenderFormat_;
    CairoFontsCache m_aCairoFontsCache;

    XLIB_Region     pPaintRegion_;
    XLIB_Region     pClipRegion_;

    GC              pPenGC_;        // Pen attributes
    SalColor        nPenColor_;
    Pixel           nPenPixel_;

    GC              pFontGC_;       // Font attributes
    ExtendedFontStructRef   mXFont[ MAX_FALLBACK ];
    ServerFont*             mpServerFont[ MAX_FALLBACK ];

    SalColor        nTextColor_;
    Pixel           nTextPixel_;
    BOOL            bFontVertical_;

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

    BOOL            bWindow_ : 1;       // is Window
    BOOL            bPrinter_ : 1;      // is Printer
    BOOL            bVirDev_ : 1;       // is VirDev
    BOOL            bPenGC_ : 1;        // is Pen GC valid
    BOOL            bFontGC_ : 1;       // is Font GC valid
    BOOL            bBrushGC_ : 1;      // is Brush GC valid
    BOOL            bMonoGC_ : 1;       // is Mono GC valid
    BOOL            bCopyGC_ : 1;       // is Copy GC valid
    BOOL            bInvertGC_ : 1;     // is Invert GC valid
    BOOL            bInvert50GC_ : 1;   // is Invert50 GC valid
    BOOL            bStippleGC_ : 1;    // is Stipple GC valid
    BOOL            bTrackingGC_ : 1;   // is Tracking GC valid
    BOOL            bXORMode_ : 1;      // is ROP XOR Mode set
    BOOL            bDitherBrush_ : 1;  // is solid or tile

    void            SetClipRegion( GC          pGC,
                                   XLIB_Region pXReg = NULL ) const;

    GC              GetTrackingGC();
    GC              GetInvertGC();
    GC              GetInvert50GC();
    GC              CreateGC( Drawable      hDrawable,
                              unsigned long nMask = GCGraphicsExposures );
    GC              SelectPen();
    GC              SelectBrush();
    void            DrawLines( ULONG              nPoints,
                               const SalPolyLine &rPoints,
                               GC                 pGC );
    BOOL            GetDitherPixmap ( SalColor nSalColor );

    inline  GC              GetMonoGC( Pixmap hPixmap );
    inline  GC              GetCopyGC();
    inline  GC              GetStippleGC();

    int             Clip      ( XLIB_Region   pRegion,
                                int          &nX,
                                int          &nY,
                                unsigned int &nDX,
                                unsigned int &nDY,
                                int          &nSrcX,
                                int          &nSrcY ) const;
    int             Clip      ( int          &nX,
                                int          &nY,
                                unsigned int &nDX,
                                unsigned int &nDY,
                                int          &nSrcX,
                                int          &nSrcY ) const;
    GC              SetMask   ( int          &nX,
                                int          &nY,
                                unsigned int &nDX,
                                unsigned int &nDY,
                                int          &nSrcX,
                                int          &nSrcY,
                                Pixmap        hClipMask );
    using SalGraphics::DrawBitmap;
    void            DrawBitmap( const SalTwoRect *pPosAry,
                                SalGraphics      *pThis,
                                const SalBitmap  &rSalBitmap,
                                const SalBitmap  &rTransparentBitmap,
                                SalColor          nTransparentColor );

    GC                      SelectFont();
    bool                    setFont( const ImplFontSelectData* pEntry, int nFallbackLevel );

    void                    drawMaskedBitmap( const SalTwoRect* pPosAry,
                                              const SalBitmap& rSalBitmap,
                                              const SalBitmap& rTransparentBitmap );

protected:
    void                    DrawStringUCS2MB( ExtendedFontStruct& rFont, const Point&,
                                const sal_Unicode* pStr, int nLength );

    void                    DrawPrinterString( const SalLayout& );

    void                    DrawServerFontString( const ServerFontLayout& );
    void                    DrawServerSimpleFontString( const ServerFontLayout& );
    void                    DrawServerAAFontString( const ServerFontLayout& );
    bool                    DrawServerAAForcedString( const ServerFontLayout& );
    void                    DrawCairoAAFontString( const ServerFontLayout& );

    void freeResources();
public:
                            X11SalGraphics();
    virtual             ~X11SalGraphics();

            void            Init( SalFrame *pFrame, Drawable aDrawable, int nScreen );
            void            Init( X11SalVirtualDevice *pVirtualDevice, SalColormap* pColormap = NULL, bool bDeleteColormap = false );
            void            Init( class ImplSalPrinterData *pPrinter );
            void            DeInit();

    inline  const SalDisplay*   GetDisplay() const;
    inline  Display*            GetXDisplay() const;
    inline  const SalVisual&    GetVisual() const;
    inline  Drawable        GetDrawable() const { return hDrawable_; }
    void                    SetDrawable( Drawable d, int nScreen );
    inline  void*           GetXRenderFormat() { return pRenderFormat_; }
    inline  void            SetXRenderFormat( void* pRenderFormat ) { pRenderFormat_ = pRenderFormat; }
    inline  const SalColormap&    GetColormap() const { return *m_pColormap; }
    using SalGraphics::GetPixel;
    inline  Pixel           GetPixel( SalColor nSalColor ) const;

    int                     GetScreenNumber() const
    { return m_nScreen; }

    // overload all pure virtual methods
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual void            GetScreenFontResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual USHORT          GetBitCount();
    virtual long            GetGraphicsWidth() const;

    virtual void            ResetClipRegion();
    virtual void            BeginSetClipRegion( ULONG nCount );
    virtual BOOL            unionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void            EndSetClipRegion();

    virtual void            SetLineColor();
    virtual void            SetLineColor( SalColor nSalColor );
    virtual void            SetFillColor();

    virtual void            SetFillColor( SalColor nSalColor );

    virtual void            SetXORMode( BOOL bSet );

    virtual void            SetROPLineColor( SalROPColor nROPColor );
    virtual void            SetROPFillColor( SalROPColor nROPColor );

    virtual void            SetTextColor( SalColor nSalColor );
    virtual USHORT         SetFont( ImplFontSelectData*, int nFallbackLevel );
    virtual void            GetFontMetric( ImplFontMetricData* );
    virtual ULONG           GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
    virtual ImplFontCharMap* GetImplFontCharMap() const;
    virtual void            GetDevFontList( ImplDevFontList* );
    virtual void            GetDevFontSubstList( OutputDevice* );
    virtual bool            AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
    virtual BOOL            CreateFontSubset( const rtl::OUString& rToFile,
                                              const ImplFontData*,
                                              sal_Int32* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              );
    virtual const Ucs2SIntMap* GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );
    virtual const void* GetEmbedFontData( const ImplFontData*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );
    virtual void            GetGlyphWidths( const ImplFontData*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );
    virtual BOOL            GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual BOOL            GetGlyphOutline( long nIndex, ::basegfx::B2DPolyPolygon& );
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void            DrawServerFontLayout( const ServerFontLayout& );
    virtual bool            supportsOperation( OutDevSupportType ) const;
    virtual void            drawPixel( long nX, long nY );
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void            drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual sal_Bool        drawPolyLineBezier( ULONG nPoints,
                                                const SalPoint* pPtAry,
                                                const BYTE* pFlgAry );
    virtual sal_Bool        drawPolygonBezier( ULONG nPoints,
                                               const SalPoint* pPtAry,
                                               const BYTE* pFlgAry );
    virtual sal_Bool        drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const BYTE* const* pFlgAry );
    virtual void            copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      USHORT nFlags );
    virtual void            copyBits( const SalTwoRect* pPosAry,
                                      SalGraphics* pSrcGraphics );
    virtual void            drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap );
    virtual void            drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor );
    virtual void            drawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rMaskBitmap );
    virtual void            drawMask( const SalTwoRect* pPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor );
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor        getPixel( long nX, long nY );
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
    virtual void            invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual BOOL            drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize );

    virtual bool            drawAlphaBitmap( const SalTwoRect&,
                                             const SalBitmap& rSourceBitmap,
                                             const SalBitmap& rAlphaBitmap );

    virtual bool            drawAlphaRect( long nX, long nY, long nWidth,
                                           long nHeight, sal_uInt8 nTransparency );

    /*  use to handle GraphicsExpose/NoExpose after XCopyArea & friends
     *  if pFrame is not NULL, corresponding Paint events are generated
     *  and dispatched to pFrame
     *
     *  it is imperative to eat up graphics exposes even in case you don't need
     *  them because the next one using XCopyArea can depend on them
     */
    static void YieldGraphicsExpose( Display* pDisplay, SalFrame* pFrame, Drawable aDrawable );

    // do XCopyArea or XGet/PutImage depending on screen numbers
    // signature is like XCopyArea with screen numbers added
    static void CopyScreenArea( Display* pDisplay,
                                Drawable aSrc, int nScreenSrc, int nSrcDepth,
                                Drawable aDest, int nScreenDest, int nDestDepth,
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


// -=-= Shortcuts =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifdef DBG_UTIL
#define stderr0( s )            fprintf( stderr, s )
#define stderr1( s, a )         fprintf( stderr, s, a )
#define stderr2( s, a, b )      fprintf( stderr, s, a, b )
#define stderr3( s, a, b, c )   fprintf( stderr, s, a, b, c )
#define stdass0( b )            (void)( !(b) \
                                        ? fprintf( stderr, "\"%s\" (%s line %d)\n", \
                                                    #b, __FILE__, __LINE__ ) \
                                        : 0 )
#else
#define stderr0( s )            ;
#define stderr1( s, a )     ;
#define stderr2( s, a, b )  ;
#define stderr3( s, a, b, c )   ;
#define stdass0( b )            ;
#endif

#endif // _SV_SALGDI_H

