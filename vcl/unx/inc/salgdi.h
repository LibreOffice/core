/*************************************************************************
 *
 *  $RCSfile: salgdi.h,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:34:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H


// -=-= exports -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class   SalFontCacheItem;
class   SalGraphicsContext;
class   SalGraphicsData;

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _FRACT_HXX
#include <tools/fract.hxx>
#endif

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

class X11SalGraphics : public SalGraphics
{
    friend class            X11FontLayout;
    friend class            ServerFontLayout;
protected:
    SalFrame*               m_pFrame; // the SalFrame which created this Graphics or NULL
    X11SalVirtualDevice*    m_pVDev;  // the SalVirtualDevice which created this Graphics or NULL

    SalColormap    *m_pColormap;
    SalColormap    *m_pDeleteColormap;
    Drawable        hDrawable_;     // use

    XLIB_Region     pPaintRegion_;
    XLIB_Region     pClipRegion_;

    int             numClipRects_;
    Rectangle       boundingClipRect_;

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
    void            DrawBitmap( const SalTwoRect *pPosAry,
                                SalGraphics      *pThis,
                                const SalBitmap  &rSalBitmap,
                                const SalBitmap  &rTransparentBitmap,
                                SalColor          nTransparentColor );

    GC                      SelectFont();
    bool                    setFont( const ImplFontSelectData* pEntry, int nFallbackLevel );

protected:
    void                    DrawStringUCS2MB( ExtendedFontStruct& rFont, const Point&,
                                const sal_Unicode* pStr, int nLength );

    void                    DrawPrinterString( const SalLayout& );

    void                    DrawServerFontString( const ServerFontLayout& );
    void                    DrawServerSimpleFontString( const ServerFontLayout& );
    void                    DrawServerAAFontString( const ServerFontLayout& );
    bool                    DrawServerAAForcedString( const ServerFontLayout& );
public:
                            X11SalGraphics();
    virtual             ~X11SalGraphics();

            void            Init( SalFrame *pFrame, Drawable aDrawable );
            void            Init( X11SalVirtualDevice *pVirtualDevice );
            void            Init( class ImplSalPrinterData *pPrinter );
            void            DeInit();

    inline  SalDisplay     *GetDisplay() const;
    inline  Display        *GetXDisplay() const;
    inline  SalVisual       *GetVisual() const;
    inline  Drawable        GetDrawable() const { return hDrawable_; }
    inline  void            SetDrawable( Drawable d ) { hDrawable_ = d; }
    inline  SalColormap    &GetColormap() const { return *m_pColormap; }
    inline  Pixel           GetPixel( SalColor nSalColor ) const;

    inline  int            GetNumCliprects() const { return numClipRects_; }
    inline  const Rectangle& GetBoundingCliprect() const { return boundingClipRect_; }


    // overload all pure virtual methods
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual void            GetScreenFontResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual USHORT          GetBitCount();
    virtual long            GetGraphicsWidth();

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
                                              ImplFontData* pFont,
                                              sal_Int32* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              );
    virtual const std::map< sal_Unicode, sal_Int32 >* GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** ppNonEncoded );
    virtual const void* GetEmbedFontData( ImplFontData* pFont,
                                          const sal_Unicode* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    virtual void            FreeEmbedFontData( const void* pData, long nDataLen );
    virtual BOOL            GetGlyphBoundRect( long nIndex, Rectangle& );
    virtual BOOL            GetGlyphOutline( long nIndex, PolyPolygon& );
    virtual SalLayout*      GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void            DrawServerFontLayout( const ServerFontLayout& );
    virtual void            drawPixel( long nX, long nY );
    virtual void            drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void            drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void            drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual void            drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
    virtual void            drawPolyPolygon( ULONG nPoly,
                                             const ULONG* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual sal_Bool        drawPolyLineBezier( ULONG nPoints,
                                                const SalPoint* pPtAry,
                                                const BYTE* pFlgAry );
    virtual sal_Bool        drawPolygonBezier( ULONG nPoints,
                                               const SalPoint* pPtAry,
                                               const BYTE* pFlgAry );
    virtual sal_Bool        drawPolyPolygonBezier( ULONG nPoly,
                                                   const ULONG* pPoints,
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
                                        const SalBitmap& rTransparentBitmap );
    virtual void            drawMask( const SalTwoRect* pPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor );
    virtual SalBitmap*      getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor        getPixel( long nX, long nY );
    virtual void            invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
    virtual void            invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual BOOL            drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize );

    /*  use to handle GraphicsExpose/NoExpose after XCopyArea & friends
     *  if pFrame is not NULL, corresponding Paint events are generated
     *  and dispatched to pFrame
     *
     *  it is imperative to eat up graphics exposes even in case you don't need
     *  them because the next one using XCopyArea can depend on them
     */
    static void YieldGraphicsExpose( Display* pDisplay, SalFrame* pFrame, Drawable aDrawable );
};


inline SalDisplay *X11SalGraphics::GetDisplay() const
{ return GetColormap().GetDisplay(); }

inline SalVisual *X11SalGraphics::GetVisual() const
{ return GetColormap().GetDisplay()->GetVisual(); }

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

