/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gh $ $Date: 2000-10-12 13:59:12 $
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

#define _SV_SALGDI_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <stdio.h>
#include <stdlib.h>

#include <prex.h>
#include <X11/Xproto.h>
#include <postx.h>

#include <salunx.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif

#include <tools/debug.hxx>

#ifndef PRINTER_DUMMY
#define Font XLIB_Font
#define Region XLIB_Region
#include <xprinter/xp.h>
#undef Font
#undef Region
#endif

// -=-= SalPolyLine =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define STATIC_POINTS 64

class SalPolyLine
{
            XPoint              Points_[STATIC_POINTS];
            XPoint             *pFirst_;
public:
    inline                      SalPolyLine( ULONG nPoints );
    inline                      SalPolyLine( ULONG nPoints, const SalPoint *p );
    inline                      ~SalPolyLine();
    inline  XPoint             &operator [] ( ULONG n ) const
                                { return pFirst_[n]; }
};

inline SalPolyLine::SalPolyLine( ULONG nPoints )
    : pFirst_( nPoints+1 > STATIC_POINTS ? new XPoint[nPoints+1] : Points_ )
{}

inline SalPolyLine::SalPolyLine( ULONG nPoints, const SalPoint *p )
    : pFirst_( nPoints+1 > STATIC_POINTS ? new XPoint[nPoints+1] : Points_ )
{
    for( ULONG i = 0; i < nPoints; i++ )
    {
        pFirst_[i].x = (short)p[i].mnX;
        pFirst_[i].y = (short)p[i].mnY;
    }
    pFirst_[nPoints] = pFirst_[0]; // close polyline
}

inline SalPolyLine::~SalPolyLine()
{ if( pFirst_ != Points_ ) delete pFirst_; }

#undef STATIC_POINTS
// -=-= SalGraphicsData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final SalGraphicsData::SalGraphicsData()
{
    hDrawable_          = None;

    pClipRegion_        = NULL;
    pPaintRegion_       = NULL;

    pPenGC_             = NULL;
    nPenPixel_          = 0;
    nPenColor_          = MAKE_SALCOLOR( 0x00, 0x00, 0x00 ); // Black

    pFontGC_            = NULL;
//  xFont_              = NULL;
    aScale_             = Fraction( 1, 1 );
    nTextPixel_         = 0;
    nTextColor_         = MAKE_SALCOLOR( 0x00, 0x00, 0x00 ); // Black

    pBrushGC_           = NULL;
    nBrushPixel_        = 0;
    nBrushColor_        = MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF ); // White
    hBrush_             = None;

    pMonoGC_            = NULL;
    pCopyGC_            = NULL;
    pMaskGC_            = NULL;
    pInvertGC_          = NULL;
    pInvert50GC_        = NULL;
    pStippleGC_         = NULL;
    pTrackingGC_        = NULL;

    bWindow_            = FALSE;
    bPrinter_           = FALSE;
    bVirDev_            = FALSE;
    bPenGC_             = FALSE;
    bFontGC_            = FALSE;
    bBrushGC_           = FALSE;
    bMonoGC_            = FALSE;
    bCopyGC_            = FALSE;
    bInvertGC_          = FALSE;
    bInvert50GC_        = FALSE;
    bStippleGC_         = FALSE;
    bTrackingGC_        = FALSE;
    bXORMode_           = FALSE;
    bDitherBrush_       = FALSE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final SalGraphicsData::~SalGraphicsData()
{
    Display *pDisplay = GetXDisplay();

    DBG_ASSERT( !pPaintRegion_, "pPaintRegion_" )
    if( pClipRegion_ ) XDestroyRegion( pClipRegion_ );

    if( hBrush_ )       XFreePixmap( pDisplay, hBrush_ );
    if( pPenGC_ )       XFreeGC( pDisplay, pPenGC_ );
    if( pFontGC_ )      XFreeGC( pDisplay, pFontGC_ );
    if( pBrushGC_ )     XFreeGC( pDisplay, pBrushGC_ );
    if( pMonoGC_ )      XFreeGC( pDisplay, pMonoGC_ );
    if( pCopyGC_ )      XFreeGC( pDisplay, pCopyGC_ );
    if( pMaskGC_ )      XFreeGC( pDisplay, pMaskGC_ );
    if( pInvertGC_ )    XFreeGC( pDisplay, pInvertGC_ );
    if( pInvert50GC_ )  XFreeGC( pDisplay, pInvert50GC_ );
    if( pStippleGC_ )   XFreeGC( pDisplay, pStippleGC_ );
    if( pTrackingGC_ )  XFreeGC( pDisplay, pTrackingGC_ );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
beta  void SalGraphicsData::DeInit()
{
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalGraphicsData::SetClipRegion( GC pGC, XLIB_Region pXReg ) const
{
    Display *pDisplay = GetXDisplay();

    int n = 0;
    XLIB_Region Regions[3];

    if( pClipRegion_ /* && !XEmptyRegion( pClipRegion_ ) */ )
        Regions[n++] = pClipRegion_;
//  if( pPaintRegion_ /* && !XEmptyRegion( pPaintRegion_ ) */ )
//      Regions[n++] = pPaintRegion_;
    if( pXReg && !XEmptyRegion( pXReg ) )
        Regions[n++] = pXReg;

    if( 0 == n )
        XSetClipMask( pDisplay, pGC, None );
    else if( 1 == n )
        XSetRegion( pDisplay, pGC, Regions[0] );
    else
    {
        XLIB_Region pTmpRegion = XCreateRegion();
        XIntersectRegion( Regions[0], Regions[1], pTmpRegion );
//      if( 3 == n )
//          XIntersectRegion( Regions[2], pTmpRegion, pTmpRegion );
        XSetRegion( pDisplay, pGC, pTmpRegion );
        XDestroyRegion( pTmpRegion );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final GC SalGraphicsData::SelectPen()
{
    Display *pDisplay = GetXDisplay();

    DBG_ASSERT( nPenColor_ != 0xFFFFFFFF, "Pen Transparent" );

    if( !pPenGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = IncludeInferiors;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = True;

        pPenGC_ = XCreateGC( pDisplay, hDrawable_,
                             GCSubwindowMode | GCFillRule | GCGraphicsExposures,
                             &values );
    }

    if( !bPenGC_ )
    {
        XSetForeground( pDisplay, pPenGC_, nPenPixel_ );
        XSetFunction  ( pDisplay, pPenGC_, bXORMode_ ? GXxor : GXcopy );
        SetClipRegion( pPenGC_ );
        bPenGC_ = TRUE;
    }

    return pPenGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final GC SalGraphicsData::SelectBrush()
{
    Display *pDisplay = GetXDisplay();

    DBG_ASSERT( nBrushColor_ != 0xFFFFFFFF, "Brush Transparent" );

    if( !pBrushGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = IncludeInferiors;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = True;

        pBrushGC_ = XCreateGC( pDisplay, hDrawable_,
                               GCSubwindowMode | GCFillRule | GCGraphicsExposures,
                               &values );
    }

    if( !bBrushGC_ )
    {
        if( !bDitherBrush_ )
        {
            XSetFillStyle ( pDisplay, pBrushGC_, FillSolid );
            XSetForeground( pDisplay, pBrushGC_, nBrushPixel_ );
            if( bPrinter_ )
                XSetTile( pDisplay, pBrushGC_, None );
        }
        else
        {
            // Bug in Sun Solaris 2.5.1, XFillPolygon doesn't allways reflect
            // changes of the tile. PROPERTY_BUG_Tile doesn't fix this !
            if (GetDisplay()->GetProperties() & PROPERTY_BUG_FillPolygon_Tile)
                XSetFillStyle ( pDisplay, pBrushGC_, FillSolid );

            XSetFillStyle ( pDisplay, pBrushGC_, FillTiled );
            XSetTile      ( pDisplay, pBrushGC_, hBrush_ );
        }
        XSetFunction  ( pDisplay, pBrushGC_, bXORMode_ ? GXxor : GXcopy );
        SetClipRegion( pBrushGC_ );
        bBrushGC_ = TRUE;
    }

    return pBrushGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final GC SalGraphicsData::GetTrackingGC()
{
    const char    dash_list[2] = {2, 2};

    if( !pTrackingGC_ )
    {
        XGCValues     values;

        values.graphics_exposures   = True;
        values.foreground           = xColormap_->GetBlackPixel()
                                      ^ xColormap_->GetWhitePixel();
        values.function             = GXxor;
        values.line_width           = 1;
        values.line_style           = LineOnOffDash;

        pTrackingGC_ = XCreateGC( GetXDisplay(), GetDrawable(),
                                  GCGraphicsExposures | GCForeground | GCFunction
                                  | GCLineWidth | GCLineStyle,
                                  &values );
        XSetDashes( GetXDisplay(), pTrackingGC_, 0, dash_list, 2 );
    }

    if( !bTrackingGC_ )
    {
        SetClipRegion( pTrackingGC_ );
        bTrackingGC_ = TRUE;
    }

    return pTrackingGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphicsData::DrawLines( ULONG              nPoints,
                                       const SalPolyLine &rPoints,
                                       GC                 pGC )
{
    // errechne wie viele Linien XWindow auf einmal zeichnen kann
    ULONG nMaxLines = (GetDisplay()->GetMaxRequestSize() - sizeof(xPolyPointReq))
                      / sizeof(xPoint);
    if( nMaxLines > nPoints ) nMaxLines = nPoints;

    // gebe alle Linien aus, die XWindows zeichnen kann.
    ULONG n;
    for( n = 0; nPoints - n > nMaxLines; n += nMaxLines - 1 )
        XDrawLines( GetXDisplay(),
                    GetDrawable(),
                    pGC,
                    &rPoints[n],
                    nMaxLines,
                    CoordModeOrigin );

    if( n < nPoints )
        XDrawLines( GetXDisplay(),
                    GetDrawable(),
                    pGC,
                    &rPoints[n],
                    nPoints - n,
                    CoordModeOrigin );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Dithern: Calculate a dither-pixmap and make a brush of it
#define P_DELTA         51
#define DMAP( v, m )    ((v % P_DELTA) > m ? (v / P_DELTA) + 1 : (v / P_DELTA))

final BOOL SalGraphicsData::GetDitherPixmap( SalColor nSalColor )
{
    static const short nOrdDither8Bit[ 8 ][ 8 ] =
    {
         0, 38,  9, 48,  2, 40, 12, 50,
        25, 12, 35, 22, 28, 15, 37, 24,
         6, 44,  3, 41,  8, 47,  5, 44,
        32, 19, 28, 16, 34, 21, 31, 18,
         1, 40, 11, 49,  0, 39, 10, 48,
        27, 14, 36, 24, 26, 13, 36, 23,
         8, 46,  4, 43,  7, 45,  4, 42,
        33, 20, 30, 17, 32, 20, 29, 16
    };

    // test for correct depth (8bit)
    if( GetColormap().GetVisual()->GetDepth() != 8 )
        return FALSE;

    char    pBits[64];
    char   *pBitsPtr = pBits;

    // Set the pallette-entries for the dithering tile
    UINT8 nSalColorRed   = SALCOLOR_RED   ( nSalColor );
    UINT8 nSalColorGreen = SALCOLOR_GREEN ( nSalColor );
    UINT8 nSalColorBlue  = SALCOLOR_BLUE  ( nSalColor );

    for( int nY = 0; nY < 8; nY++ )
    {
        for( int nX = 0; nX < 8; nX++ )
        {
            short nMagic = nOrdDither8Bit[nY][nX];
            UINT8 nR   = P_DELTA * DMAP( nSalColorRed,   nMagic );
            UINT8 nG   = P_DELTA * DMAP( nSalColorGreen, nMagic );
            UINT8 nB   = P_DELTA * DMAP( nSalColorBlue,  nMagic );

            *pBitsPtr++ = GetColormap().GetPixel( MAKE_SALCOLOR( nR, nG, nB ) );
        }
    }

    // create the tile as ximage and an according pixmap -> caching
    XImage *pImage = XCreateImage( GetXDisplay(),
                                   GetColormap().GetXVisual(),
                                   8,
                                   ZPixmap,
                                   0,               // offset
                                   pBits,           // data
                                   8, 8,            // width & height
                                   8,               // bitmap_pad
                                   0 );             // (default) bytes_per_line

    if ( GetDisplay()->GetProperties() & PROPERTY_BUG_Tile )
    {
        if (hBrush_)
            XFreePixmap (GetXDisplay(), hBrush_);
        hBrush_ = XCreatePixmap( GetXDisplay(), GetDrawable(), 8, 8, 8 );
    }
    else
    if( !hBrush_ )
        hBrush_ = XCreatePixmap( GetXDisplay(), GetDrawable(), 8, 8, 8 );

    // put the ximage to the pixmap
    XPutImage( GetXDisplay(),
               hBrush_,
               GetDisplay()->GetCopyGC(),
               pImage,
               0, 0,                        // Source
               0, 0,                        // Destination
               8, 8 );                      // width & height

    // destroy image-frame but not palette-data
    pImage->data = NULL;
    XDestroyImage( pImage );

    return TRUE;
}

// -=-= SalGraphics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final SalGraphics::SalGraphics()
{ }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final SalGraphics::~SalGraphics()
{ }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::GetResolution( long &rDPIX, long &rDPIY ) // const
{
    SalDisplay *pDisplay = _GetDisplay();

    rDPIX = pDisplay->GetResolution().A();
    rDPIY = pDisplay->GetResolution().B();
    if ( rDPIY < 96 )
    {
        rDPIX = Divide( rDPIX * 96, rDPIY );
        rDPIY = 96;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::GetScreenFontResolution( long &rDPIX, long &rDPIY ) // const
{
    GetResolution ( rDPIX, rDPIY );

    if( rDPIY < 108 )
    {
        rDPIX = Divide( rDPIX * 108, rDPIY );
        rDPIY = 108;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final USHORT SalGraphics::GetBitCount() // const
{ return _GetVisual()->GetDepth(); }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::ResetClipRegion()
{
    if( _GetClipRegion() )
    {
        _IsPenGC()          = FALSE;
        _IsFontGC()         = FALSE;
        _IsBrushGC()        = FALSE;
        _IsMonoGC()         = FALSE;
        _IsCopyGC()         = FALSE;
        _IsInvertGC()       = FALSE;
        _IsInvert50GC()     = FALSE;
        _IsStippleGC()      = FALSE;
        _IsTrackingGC()     = FALSE;

        XDestroyRegion( _GetClipRegion() );
        _GetClipRegion()    = NULL;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::BeginSetClipRegion( ULONG )
{
    if( _GetClipRegion() )
        XDestroyRegion( _GetClipRegion() );
    _GetClipRegion() = XCreateRegion();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nDX, long nDY )
{
#ifdef DEBUG
    if( nDX <= 0 || nX + nDX > 32767 )
        fprintf( stderr, "CombineClipRegion %ld %ld\n", nX, nDX );
    if( nDY <= 0 || nX + nDY > 32767 )
        fprintf( stderr, "CombineClipRegion %ld %ld\n", nY, nDY );
#endif
    if (!nDX || !nDY)
        return TRUE;

    XRectangle aRect;
    aRect.x         = (short)nX;
    aRect.y         = (short)nY;
    aRect.width     = (unsigned short)nDX;
    aRect.height    = (unsigned short)nDY;

    XUnionRectWithRegion( &aRect, _GetClipRegion(), _GetClipRegion() );

    return TRUE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::EndSetClipRegion()
{
    _IsPenGC()      = FALSE;
    _IsFontGC()     = FALSE;
    _IsBrushGC()    = FALSE;
    _IsMonoGC()     = FALSE;
    _IsCopyGC()     = FALSE;
    _IsInvertGC()   = FALSE;
    _IsInvert50GC() = FALSE;
    _IsStippleGC()  = FALSE;
    _IsTrackingGC() = FALSE;

//  if( _GetPaintRegion() )
//      XIntersectRegion( _GetClipRegion(), _GetPaintRegion(), _GetClipRegion() );
//  else

    if( XEmptyRegion( _GetClipRegion() ) )
    {
        XDestroyRegion( _GetClipRegion() );
        _GetClipRegion()    = NULL;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::SetLineColor()
{
    if( _GetPenColor() != 0xFFFFFFFF )
    {
        _GetPenColor()      = 0xFFFFFFFF;
        _IsPenGC()          = FALSE;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::SetLineColor( SalColor nSalColor )
{
    if( _GetPenColor() != nSalColor )
    {
        _GetPenColor()      = nSalColor;
        _GetPenPixel()      = _GetPixel( nSalColor );
        _IsPenGC()          = FALSE;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::SetFillColor()
{
    if( _GetBrushColor() != 0xFFFFFFFF )
    {
        _IsDitherBrush()    = FALSE;
        _GetBrushColor()    = 0xFFFFFFFF;
        _IsBrushGC()        = FALSE;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::SetFillColor( SalColor nSalColor )
{
    if( _GetBrushColor() != nSalColor )
    {
        _IsDitherBrush()    = FALSE;
        _GetBrushColor()    = nSalColor;
        _GetBrushPixel()    = _GetPixel( nSalColor );
        if( TrueColor != _GetColormap().GetVisual()->GetClass()
            && _GetColor( _GetBrushPixel() ) != _GetBrushColor()
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x00, 0x00 ) // black
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x00, 0x80 ) // blue
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x80, 0x00 ) // green
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x80, 0x80 ) // cyan
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x00, 0x00 ) // red
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x00, 0x80 ) // magenta
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x80, 0x00 ) // brown
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x80, 0x80 ) // gray
            && nSalColor != MAKE_SALCOLOR( 0xC0, 0xC0, 0xC0 ) // light gray
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x00, 0xFF ) // light blue
            && nSalColor != MAKE_SALCOLOR( 0x00, 0xFF, 0x00 ) // light green
            && nSalColor != MAKE_SALCOLOR( 0x00, 0xFF, 0xFF ) // light cyan
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0x00, 0x00 ) // light red
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0x00, 0xFF ) // light magenta
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0xFF, 0x00 ) // light brown
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF ) )
             _IsDitherBrush() = maGraphicsData.GetDitherPixmap(nSalColor);
        _IsBrushGC()        = FALSE;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SAL_ROP_0 : // 0
            _GetPenPixel() = (Pixel)0;
            break;
        case SAL_ROP_1 : // 1
            _GetPenPixel() = (Pixel)(1 << _GetVisual()->GetDepth()) - 1;
            break;
        case SAL_ROP_INVERT : // 2
            _GetPenPixel() = (Pixel)(1 << _GetVisual()->GetDepth()) - 1;
            break;
    }
    _GetPenColor()  = _GetColor( _GetPenPixel() );
    _IsPenGC()      = FALSE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SAL_ROP_0 : // 0
            _GetBrushPixel() = (Pixel)0;
            break;
        case SAL_ROP_1 : // 1
            _GetBrushPixel() = (Pixel)(1 << _GetVisual()->GetDepth()) - 1;
            break;
        case SAL_ROP_INVERT : // 2
            _GetBrushPixel() = (Pixel)(1 << _GetVisual()->GetDepth()) - 1;
            break;
    }
    _IsDitherBrush()    = FALSE;
    _GetBrushColor()    = _GetColor( _GetBrushPixel() );
    _IsBrushGC()        = FALSE;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::SetXORMode( BOOL bSet )
{
    if( !_IsXORMode() == bSet )
    {
        _IsXORMode()    = bSet;
        _IsPenGC()      = FALSE;
        _IsBrushGC()    = FALSE;
        _IsMonoGC()     = FALSE;
        _IsCopyGC()     = FALSE;
        _IsInvertGC()   = FALSE;
        _IsInvert50GC() = FALSE;
        _IsStippleGC()  = FALSE;
        _IsTrackingGC() = FALSE;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::DrawPixel( long nX, long nY )
{
    if( _GetPenColor() !=  0xFFFFFFFF )
        XDrawPoint( _GetXDisplay(), _GetDrawable(), _SelectPen(), nX, nY );
}

final void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor )
{
    if( nSalColor != 0xFFFFFFFF )
    {
        Display *pDisplay = _GetXDisplay();

        if( _GetPenColor() == 0xFFFFFFFF && !_IsPenGC() )
        {
            SetLineColor( nSalColor );
            XDrawPoint( pDisplay, _GetDrawable(), _SelectPen(), nX, nY );
            _GetPenColor() = 0xFFFFFFFF;
            _IsPenGC() = False;
        }
        else
        {
            GC pGC = _SelectPen();

            if( nSalColor != _GetPenColor() )
                XSetForeground( pDisplay, pGC, _GetPixel( nSalColor ) );

            XDrawPoint( pDisplay, _GetDrawable(), pGC, nX, nY );

            if( nSalColor != _GetPenColor() )
                XSetForeground( pDisplay, pGC, _GetPenPixel() );
        }
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( _GetPenColor() != 0xFFFFFFFF )
    {
        if ( _GetDisplay()->GetProperties() & PROPERTY_BUG_DrawLine )
        {
            GC aGC = _SelectPen();
            XDrawPoint (_GetXDisplay(), _GetDrawable(), aGC, (int)nX1, (int)nY1);
            XDrawPoint (_GetXDisplay(), _GetDrawable(), aGC, (int)nX2, (int)nY2);
            XDrawLine  (_GetXDisplay(), _GetDrawable(), aGC, nX1, nY1, nX2, nY2 );
        }
        else
            XDrawLine( _GetXDisplay(), _GetDrawable(),_SelectPen(),
                       nX1, nY1, nX2, nY2 );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalGraphics::DrawRect( long nX, long nY, long nDX, long nDY )
{
    if( _GetBrushColor() != 0xFFFFFFFF )
        XFillRectangle( _GetXDisplay(),
                        _GetDrawable(),
                        _SelectBrush(),
                        nX, nY, nDX, nDY );

    // Beschreibung DrawRect verkehrt, deshalb -1
    if( _GetPenColor() != 0xFFFFFFFF )
        XDrawRectangle( _GetXDisplay(),
                        _GetDrawable(),
                        _SelectPen(),
                        nX, nY, nDX-1, nDY-1 );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#if 0
// Ausgetauscht durch SalGraphics::Invert
final void SalGraphics::InvertTracking( ULONG nPoints, const SalPoint *pPtAry )
{
    SalPolyLine Points( nPoints, pPtAry );

    maGraphicsData.DrawLines( nPoints, Points, _GetTrackingGC() );
}
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint *pPtAry )
{
    if( _GetPenColor() != 0xFFFFFFFF )
    {
        SalPolyLine Points( nPoints, pPtAry );

        maGraphicsData.DrawLines( nPoints, Points, _SelectPen() );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    if( nPoints < 3 )
    {
        if( !nPoints )
        {
#ifdef DEBUG
            fprintf( stderr, "SalGraphics::DrawPolygon !nPoints\n" );
#endif
        }
        else if( !_IsXORMode() )
        {
            if( 1 == nPoints  )
                DrawPixel( pPtAry[0].mnX, pPtAry[0].mnY );
            else
                DrawLine( pPtAry[0].mnX, pPtAry[0].mnY,
                          pPtAry[1].mnX, pPtAry[1].mnY );
        }
        return;
    }

    SalPolyLine Points( nPoints, pPtAry );

    nPoints++;

    if( _GetBrushColor() != 0xFFFFFFFF )
        XFillPolygon( _GetXDisplay(),
                      _GetDrawable(),
                      _SelectBrush(),
                      &Points[0], nPoints,
                      Complex, CoordModeOrigin );

    if( _GetPenColor() != 0xFFFFFFFF )
        maGraphicsData.DrawLines( nPoints, Points, _SelectPen() );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalGraphics::DrawPolyPolygon( ULONG            nPoly,
                                   const ULONG     *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    if( _GetBrushColor() != 0xFFFFFFFF )
    {
        ULONG       i, n;
        XLIB_Region pXRegA  = NULL;

#ifdef CLIPPING
        Size aOutSize = pOutDev->GetOutputSizePixel();

        ClipRectangle aClippingRect( Point(0,0),
                                     Point(aOutSize.Width(), aOutSize.Height()) );

        ULONG nMax = pPoints[0];
        for( i = 1; i < nPoly; i++ )
            if( pPoints[i] > nMax )
                nMax = pPoints[i];

        SalPolyLine Points( nMax * 2 );

        for( i = 0; i < nPoly; i++ )
        {
            n = aClippingRect.ClipPolygon( pPoints[i], pPtAry[i], &Points[0] );
#else
        for( i = 0; i < nPoly; i++ ) {
            n = pPoints[i];
            SalPolyLine Points( n, pPtAry[i] );
#endif
            if( n > 2 )
            {
                XLIB_Region pXRegB = XPolygonRegion( &Points[0], n+1, WindingRule );
                if( !pXRegA )
                    pXRegA = pXRegB;
                else
                {
                    XXorRegion( pXRegA, pXRegB, pXRegA );
                    XDestroyRegion( pXRegB );
                }
            }
        }

        if( pXRegA )
        {
            XRectangle aXRect;
            XClipBox( pXRegA, &aXRect );

            GC pGC = _SelectBrush();
            maGraphicsData.SetClipRegion( pGC, pXRegA ); // ??? doppelt
            XDestroyRegion( pXRegA );
            _IsBrushGC() = FALSE;

            XFillRectangle( _GetXDisplay(),
                            _GetDrawable(),
                            pGC,
                            aXRect.x, aXRect.y, aXRect.width, aXRect.height );
        }
    }

    if( _GetPenColor() != 0xFFFFFFFF )
        for( ULONG i = 0; i < nPoly; i++ )
            DrawPolyLine( pPoints[i], pPtAry[i] );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

beta void SalGraphics::Invert( ULONG nPoints,
                                const SalPoint* pPtAry,
                                SalInvert nFlags )
{
    SalDisplay *pDisp = _GetDisplay();
    SalPolyLine Points ( nPoints, pPtAry );

    GC pGC;
    if( SAL_INVERT_50 & nFlags )
        pGC = maGraphicsData.GetInvert50GC();
    else
    if ( SAL_INVERT_TRACKFRAME & nFlags )
        pGC = maGraphicsData.GetTrackingGC();
    else
        pGC = maGraphicsData.GetInvertGC();

    maGraphicsData.DrawLines ( nPoints, Points, pGC );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#if 0
 struct XpEPS_trf
 {
     double origin_x, origin_y;
     double scale_x, scale_y;
     double rotate;
 };
 struct XpBox
 {
     double llx,lly;
     double urx,ury;
 };
 extern "C" void* XpEPS_Put( Display*, FILE*, struct XpEPS_trf* );
 extern "C" struct XpBox* XpEPS_GetBoundingBox( Display*, FILE* );
#endif

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
#ifndef PRINTER_DUMMY
    if( _IsPrinter() )
    {
        // convert \r to \n (#60367#, EPS-files with mac format)
        // this should not tangle with other lineends
        // in postscript files \r and \n count as whitespace
        // so they can only be addressed directly as hex codes <0a> etc.
        char* pRun = (char*)pPtr;
        while( (ULONG)(pRun - (char*)pPtr) < nSize )
        {
            if( *pRun == '\r' )
                *pRun = '\n';
            pRun++;
        }

        FILE* fp = tmpfile();
        if( ! fp )
            return FALSE;
        fwrite( pPtr, 1, nSize, fp );
        fseek( fp, 0, SEEK_SET );

        // holt die BoundingBox in pixel
        XpBox* pBox = XpEPS_GetBoundingBox( _GetXDisplay(), fp );
        fseek( fp, 0, SEEK_SET );
        XpEPS_trf aTransfer;
        aTransfer.origin_x = nX;
        aTransfer.origin_y = nY;
        aTransfer.rotate = 0;
        if( pBox )
        {
            aTransfer.scale_x = ((double)nWidth)/( pBox->urx - pBox->llx );
            aTransfer.scale_x = aTransfer.scale_x < 0 ?
                -aTransfer.scale_x : aTransfer.scale_x;
            aTransfer.scale_y = ((double)nHeight)/( pBox->lly - pBox->ury );
            aTransfer.scale_y = aTransfer.scale_y < 0 ?
                -aTransfer.scale_y : aTransfer.scale_y;
        }
        else
        {
            aTransfer.scale_x = 1;
            aTransfer.scale_y = 1;
#ifdef DEBUG
            fprintf( stderr, "Warning: XpEPS_GetBoundingBox returned NULL\n" );
#endif
        }

        XpEPS_Put( _GetXDisplay(), fp, &aTransfer );
        fclose( fp ); // deletes tmpfile
        free( pBox );

        return TRUE;
    }
#endif
    return FALSE;
}

