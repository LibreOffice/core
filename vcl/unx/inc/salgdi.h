/*************************************************************************
 *
 *  $RCSfile: salgdi.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pl $ $Date: 2001-02-16 12:12:23 $
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
// *=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//
//                                                                            //
// (C) 1997 Star Division GmbH, Hamburg, Germany                              //
//                                                                            //
// $Revision: 1.6 $  $Author: pl $  $Date: 2001-02-16 12:12:23 $   //
//                                                                            //
// $Workfile:   salgdi.h  $                                                   //
//  $Modtime:   10 Sep 1997 11:55:36  $                                       //
//                                                                            //
// *=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//

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
class   SalVirtualDevice;
class   SalPolyLine;
class   SalPrinter;
class   SalInfoPrinter;

#ifdef USE_PSPRINT
namespace psp { struct JobData; class PrinterGfx; }
#endif

#ifndef _SV_SALDISP_HXX
typedef SalColormap        *SalColormapRef;
#endif

// -=-= SalGraphicsData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class SalGraphicsData
{
    friend  class           SalGraphics;
    friend  class           SalPrinter;
    friend  class           SalInfoPrinter;

                            STDAPI( SalGraphicsData )

#ifdef USE_PSPRINT
            ::psp::JobData*         m_pJobData;
            ::psp::PrinterGfx*      m_pPrinterGfx;
            String*                 m_pPhoneNr;
#endif

            SalColormapRef  xColormap_;
            Drawable        hDrawable_;         // use

            XLIB_Region     pPaintRegion_;
            XLIB_Region     pClipRegion_;

            GC              pPenGC_;            // Pen attributes
            SalColor        nPenColor_;
            Pixel           nPenPixel_;

            GC              pFontGC_;           // Font attributes
            ExtendedFontStructRef       xFont_;
            class ServerFont* mpServerSideFont;
            Fraction        aScale_;
            SalColor        nTextColor_;
            Pixel           nTextPixel_;
            short           nFontOrientation_;
            BOOL            bFontVertical_;

            GC              pBrushGC_;          // Brush attributes
            SalColor        nBrushColor_;
            Pixel           nBrushPixel_;
            Pixmap          hBrush_;            // Dither

             GC             pMonoGC_;
            GC              pCopyGC_;
             GC             pMaskGC_;
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


#if defined(_SV_SALGDI_CXX) || defined (_SV_SALGDI2_CXX)
            GC              GetTrackingGC();
            GC              GetInvertGC();
            GC              GetInvert50GC();
            GC              CreateGC( Drawable      hDrawable,
                                      unsigned long nMask = GCGraphicsExposures );
#endif

#if defined _SV_SALGDI_CXX
            GC              SelectPen();
            GC              SelectBrush();
            void            DrawLines( ULONG              nPoints,
                                       const SalPolyLine &rPoints,
                                       GC                 pGC );
            BOOL            GetDitherPixmap ( SalColor nSalColor );
#endif

#if defined _SV_SALGDI2_CXX

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
#endif

#if defined _SV_SALGDI3_CXX
            GC              SelectFont();

            void            SetFont( const ImplFontSelectData* pEntry );
            void            DrawText( long          nX,
                                      long          nY,
                                      const xub_Unicode* pStr,
                                      USHORT        nLen,
                                      const long*   pDXAry );
            void            DrawText( long nX,
                                      long nY,
                                      const xub_Unicode* pStr,
                                      USHORT nLen );
#endif
public:
                            SalGraphicsData();
                            ~SalGraphicsData();

            void            Init( SalFrame         *pFrame );
            void            Init( SalVirtualDevice *pVirtualDevice,
                                  SalGraphics      *pSalGraphics );
            void            Init( class ImplSalPrinterData  *pPrinter );
            void            DeInit();

    inline  SalDisplay     *GetDisplay() const;
    inline  Display        *GetXDisplay() const;
    inline  Drawable        GetDrawable() const { return hDrawable_; }
    inline  void            SetDrawable( Drawable d ) { hDrawable_ = d; }
    inline  SalColormap    &GetColormap() const { return *xColormap_; }
    inline  BOOL            IsCompatible( USHORT       nDepth,
                                          SalColormap *pMap ) const;
    inline  Pixel           GetPixel( SalColor nSalColor ) const;

            void            FaxPhoneComment( const sal_Unicode* pStr, USHORT nLen ) const;
};

#ifdef _SV_SALDATA_HXX

inline SalDisplay *SalGraphicsData::GetDisplay() const
{ return GetColormap().GetDisplay(); }

inline Display *SalGraphicsData::GetXDisplay() const
{ return GetColormap().GetXDisplay(); }

inline BOOL SalGraphicsData::IsCompatible( USHORT       nDepth,
                                           SalColormap *pMap ) const
{
    return (GetDisplay()->GetImageDepths() & (1 << (nDepth-1))) != 0
           && &xColormap_ == pMap;
}

inline Pixel SalGraphicsData::GetPixel( SalColor nSalColor ) const
{ return GetColormap().GetPixel( nSalColor ); }

#endif

// -=-= Shortcuts =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#if defined _SV_SALGDI_CXX || defined _SV_SALGDI2_CXX || defined _SV_SALGDI3_CXX
#define _GetXDisplay()      maGraphicsData.GetXDisplay()
#define _GetDisplay()       maGraphicsData.GetDisplay()
#define _GetVisual()        maGraphicsData.GetDisplay()->GetVisual()
#define _GetDrawable()      maGraphicsData.hDrawable_
#define _GetColormap()      maGraphicsData.GetColormap()
#define _GetClipRegion()    maGraphicsData.pClipRegion_
#define _GetPenPixel()      maGraphicsData.nPenPixel_
#define _GetTextPixel()     maGraphicsData.nTextPixel_
#define _GetBrushPixel()    maGraphicsData.nBrushPixel_
#define _GetPenColor()      maGraphicsData.nPenColor_
#define _GetTextColor()     maGraphicsData.nTextColor_
#define _GetBrushColor()    maGraphicsData.nBrushColor_
#define _GetPixel(n)        maGraphicsData.GetPixel( n )
#define _GetColor(n)        maGraphicsData.GetColormap().GetColor( n )
#define _IsPenGC()          maGraphicsData.bPenGC_
#define _IsFontGC()         maGraphicsData.bFontGC_
#define _IsBrushGC()        maGraphicsData.bBrushGC_
#define _IsMonoGC()         maGraphicsData.bMonoGC_
#define _IsCopyGC()         maGraphicsData.bCopyGC_
#define _IsInvertGC()       maGraphicsData.bInvertGC_
#define _IsInvert50GC()     maGraphicsData.bInvert50GC_
#define _IsStippleGC()      maGraphicsData.bStippleGC_
#define _IsTrackingGC()     maGraphicsData.bTrackingGC_
#define _IsXORMode()        maGraphicsData.bXORMode_
#define _IsWindow()         maGraphicsData.bWindow_
#define _IsPrinter()        maGraphicsData.bPrinter_
#define _IsVirtualDevice()  maGraphicsData.bVirDev_
#define _IsDitherBrush()    maGraphicsData.bDitherBrush_
#define _SelectPen()        maGraphicsData.SelectPen()
#define _SelectBrush()      maGraphicsData.SelectBrush()
#define _GetTrackingGC()    maGraphicsData.GetTrackingGC()
#endif

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
#define stderr0( s )            nop
#define stderr1( s, a )         nop
#define stderr2( s, a, b )      nop
#define stderr3( s, a, b, c )   nop
#define stdass0( b )            nop
#endif
#endif // _SV_SALGDI_H

