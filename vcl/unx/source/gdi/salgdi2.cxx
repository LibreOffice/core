/*************************************************************************
 *
 *  $RCSfile: salgdi2.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:45:51 $
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

#define _SV_SALGDI2_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <stdio.h>

#include <salunx.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif


#ifndef _USE_PRINT_EXTENSION_
#include <psprint/printergfx.hxx>
#include <bmpacc.hxx>
#endif

#undef SALGDI2_TESTTRANS

// -=-= debugging =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifdef DBG_UTIL

static void sal_PrintImage( char *s, XImage*p )
{
    fprintf( stderr, "%s %d %d %d\n", s, p->depth, p->width, p->height );
    int nW = Min( 64, p->width*p->bits_per_pixel >> 3 );
    for( int i = 0; i < Min( 16, p->height ); i++ )
    {
        for( int j = 0; j < nW; j++ )
            fprintf( stderr, "%02X", (UINT8)p->data[i*p->bytes_per_line+j] );
        fprintf( stderr, "\n" );
    }
}

#endif // DBG_UTIL

// -----------------------------------------------------------------------------

#if (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )                              \
{                                                                   \
    XCopyArea( pXDisp, _def_drawable, aDrawable, GetCopyGC(),       \
               0, 0,                                                \
               pPosAry->mnDestWidth, pPosAry->mnDestHeight,         \
               0, 0 );                                              \
}
#else // (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )
#endif // (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS

// -=-= X11SalGraphics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GC X11SalGraphics::CreateGC( Drawable hDrawable, unsigned long nMask )
{
    XGCValues values;

    values.graphics_exposures   = True;
    values.foreground           = xColormap_->GetBlackPixel()
                                  ^ xColormap_->GetWhitePixel();
    values.function             = GXxor;
    values.line_width           = 1;
    values.fill_style           = FillStippled;
    values.stipple              = GetDisplay()->GetInvert50();
    values.subwindow_mode       = ClipByChildren;

    return XCreateGC( GetXDisplay(), hDrawable, nMask | GCSubwindowMode, &values );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline GC X11SalGraphics::GetMonoGC( Pixmap hPixmap )
{
    if( !pMonoGC_ )
        pMonoGC_ = CreateGC( hPixmap );

    if( !bMonoGC_ )
    {
        SetClipRegion( pMonoGC_ );
        bMonoGC_ = TRUE;
    }

    return pMonoGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline GC X11SalGraphics::GetCopyGC()
{
    if( bXORMode_ ) return GetInvertGC();

    if( !pCopyGC_ )
        pCopyGC_ = CreateGC( GetDrawable() );

    if( !bCopyGC_ )
    {
        SetClipRegion( pCopyGC_ );
        bCopyGC_ = TRUE;
    }
    return pCopyGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GC X11SalGraphics::GetInvertGC()
{
    if( !pInvertGC_ )
        pInvertGC_ = CreateGC( GetDrawable(),
                               GCGraphicsExposures
                               | GCForeground
                               | GCFunction
                               | GCLineWidth );

    if( !bInvertGC_ )
    {
        SetClipRegion( pInvertGC_ );
        bInvertGC_ = TRUE;
    }
    return pInvertGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GC X11SalGraphics::GetInvert50GC()
{
    if( !pInvert50GC_ )
    {
        XGCValues values;

        values.graphics_exposures   = True;
        values.foreground           = xColormap_->GetWhitePixel();
        values.background           = xColormap_->GetBlackPixel();
        values.function             = GXinvert;
        values.line_width           = 1;
        values.line_style           = LineSolid;
        unsigned long nValueMask =
                                  GCGraphicsExposures
                                  | GCForeground
                                  | GCBackground
                                  | GCFunction
                                  | GCLineWidth
                                  | GCLineStyle
                                  | GCFillStyle
                                  | GCStipple;

        char* pEnv = getenv( "SAL_DO_NOT_USE_INVERT50" );
        if( pEnv && ! strcasecmp( pEnv, "true" ) )
        {
            values.fill_style = FillSolid;
            nValueMask &= ~ GCStipple;
        }
        else
        {
            values.fill_style           = FillStippled;
            values.stipple              = GetDisplay()->GetInvert50();
        }

        pInvert50GC_ = XCreateGC( GetXDisplay(), GetDrawable(),
                                  nValueMask,
                                  &values );
    }

    if( !bInvert50GC_ )
    {
        SetClipRegion( pInvert50GC_ );
        bInvert50GC_ = TRUE;
    }
    return pInvert50GC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline GC X11SalGraphics::GetStippleGC()
{
    if( !pStippleGC_ )
        pStippleGC_ = CreateGC( GetDrawable(),
                                GCGraphicsExposures
                                | GCFillStyle
                                | GCLineWidth );

    if( !bStippleGC_ )
    {
        XSetFunction( GetXDisplay(), pStippleGC_, bXORMode_ ? GXxor : GXcopy );
        SetClipRegion( pStippleGC_ );
        bStippleGC_ = TRUE;
    }

    return pStippleGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int X11SalGraphics::Clip( XLIB_Region   pRegion,
                                 int          &nX,
                                 int          &nY,
                                 unsigned int &nDX,
                                 unsigned int &nDY,
                                 int          &nSrcX,
                                 int          &nSrcY ) const
{
    XRectangle aRect;
    XClipBox( pRegion, &aRect );

    if( nX + nDX <= aRect.x || nX >= aRect.x + aRect.width )
        return RectangleOut;
    if( nY + nDY <= aRect.y || nY >= aRect.y + aRect.height )
        return RectangleOut;

    if( nX < aRect.x )
    {
        nSrcX += aRect.x - nX;
        nDX   -= aRect.x - nX;
        nX     = aRect.x;
    }
    else if( nX + nDX > aRect.x + aRect.width )
        nDX = aRect.x + aRect.width - nX;

    if( nY < aRect.y )
    {
        nSrcY += aRect.y - nY;
        nDY   -= aRect.y - nY;
        nY     = aRect.y;
    }
    else if( nY + nDY > aRect.y + aRect.height )
        nDY = aRect.y + aRect.height - nY;

    return RectangleIn;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int X11SalGraphics::Clip( int          &nX,
                                 int          &nY,
                                 unsigned int &nDX,
                                 unsigned int &nDY,
                                 int          &nSrcX,
                                 int          &nSrcY ) const

{
    if( pPaintRegion_
        && RectangleOut == Clip( pPaintRegion_, nX, nY, nDX, nDY, nSrcX, nSrcY ) )
        return RectangleOut;

    if( pClipRegion_
        && RectangleOut == Clip( pClipRegion_,  nX, nY, nDX, nDY, nSrcX, nSrcY ) )
        return RectangleOut;

    int nPaint;
    if( pPaintRegion_ )
    {
        nPaint = XRectInRegion( pPaintRegion_, nX, nY, nDX, nDY );
        if( RectangleOut == nPaint )
            return RectangleOut;
    }
    else
        nPaint = RectangleIn;

    int nClip;
    if( pClipRegion_ )
    {
        nClip = XRectInRegion( pClipRegion_, nX, nY, nDX, nDY );
        if( RectangleOut == nClip )
            return RectangleOut;
    }
    else
        nClip = RectangleIn;

    return RectangleIn == nClip && RectangleIn == nPaint
           ? RectangleIn
           : RectanglePart;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GC X11SalGraphics::SetMask( int           &nX,
                                   int           &nY,
                                   unsigned int &nDX,
                                   unsigned int &nDY,
                                   int          &nSrcX,
                                   int          &nSrcY,
                                   Pixmap        hClipMask )
{
    int n = Clip( nX, nY, nDX, nDY, nSrcX, nSrcY );
    if( RectangleOut == n )
        return NULL;

    Display *pDisplay = GetXDisplay();

    if( !pMaskGC_ )
        pMaskGC_ = CreateGC( GetDrawable() );

    if( RectangleIn == n )
    {
        XSetClipMask( pDisplay, pMaskGC_, hClipMask );
        XSetClipOrigin( pDisplay, pMaskGC_, nX - nSrcX, nY - nSrcY );
        return pMaskGC_;
    }

    // - - - - create alternate clip pixmap for region clipping - - - -
    Pixmap hPixmap  = XCreatePixmap( pDisplay, hClipMask, nDX, nDY, 1 );

    if( !hPixmap )
    {
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
        fprintf( stderr, "X11SalGraphics::SetMask !hPixmap\n" );
#endif
        return NULL;
    }

    // - - - - reset pixmap; all 0 - - - - - - - - - - - - - - - - - - -
    XFillRectangle( pDisplay,
                    hPixmap,
                    GetDisplay()->GetMonoGC(),
                    0,   0,
                    nDX, nDY );

    // - - - - copy pixmap only within region - - - - - - - - - - - - -
    GC pMonoGC = GetMonoGC( hPixmap );
    XSetClipOrigin( pDisplay, pMonoGC, -nX, -nY );
    XCopyArea( pDisplay,
               hClipMask,           // Source
               hPixmap,             // Destination
               pMonoGC,
               nSrcX, nSrcY,        // Source
               nDX,   nDY,          // Width & Height
               0,     0 );          // Destination

    XSetClipMask( pDisplay, pMaskGC_, hPixmap );
    XSetClipOrigin( pDisplay, pMaskGC_, nX, nY );

    XFreePixmap( pDisplay, hPixmap );
    return pMaskGC_;
}

// ----- Implementation of PrinterBmp by means of SalBitmap/BitmapBuffer ---------------

#ifndef _USE_PRINT_EXTENSION_
class SalPrinterBmp : public psp::PrinterBmp
{
    private:

        BitmapBuffer*       mpBmpBuffer;

        FncGetPixel         mpFncGetPixel;
        Scanline            mpScanAccess;
        sal_Int32           mnScanOffset;

        sal_uInt32          ColorOf (BitmapColor& rColor) const;
        sal_uInt8           GrayOf  (BitmapColor& rColor) const;

                            SalPrinterBmp ();

    public:

                            SalPrinterBmp (BitmapBuffer* pBitmap);
        virtual             ~SalPrinterBmp ();
        virtual sal_uInt32  GetPaletteColor (sal_uInt32 nIdx) const;
        virtual sal_uInt32  GetPaletteEntryCount () const;
        virtual sal_uInt32  GetPixelRGB  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelIdx  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt32  GetWidth () const;
        virtual sal_uInt32  GetHeight() const;
        virtual sal_uInt32  GetDepth () const;
};

SalPrinterBmp::SalPrinterBmp (BitmapBuffer* pBuffer) :
        mpBmpBuffer (pBuffer)
{
    DBG_ASSERT (mpBmpBuffer, "SalPrinterBmp::SalPrinterBmp () can't acquire Bitmap");

    // calibrate scanline buffer
    if( BMP_SCANLINE_ADJUSTMENT( mpBmpBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN )
    {
        mpScanAccess = mpBmpBuffer->mpBits;
        mnScanOffset = mpBmpBuffer->mnScanlineSize;
    }
    else
    {
        mpScanAccess = mpBmpBuffer->mpBits
                       + (mpBmpBuffer->mnHeight - 1) * mpBmpBuffer->mnScanlineSize;
        mnScanOffset = - mpBmpBuffer->mnScanlineSize;
    }

    // request read access to the pixels
    switch( BMP_SCANLINE_FORMAT( mpBmpBuffer->mnFormat ) )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_1BIT_MSB_PAL;  break;
        case BMP_FORMAT_1BIT_LSB_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_1BIT_LSB_PAL;  break;
        case BMP_FORMAT_4BIT_MSN_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_4BIT_MSN_PAL;  break;
        case BMP_FORMAT_4BIT_LSN_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_4BIT_LSN_PAL;  break;
        case BMP_FORMAT_8BIT_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_8BIT_PAL;      break;
        case BMP_FORMAT_8BIT_TC_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_8BIT_TC_MASK;  break;
        case BMP_FORMAT_16BIT_TC_MSB_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_16BIT_TC_MSB_MASK; break;
        case BMP_FORMAT_16BIT_TC_LSB_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_16BIT_TC_LSB_MASK; break;
        case BMP_FORMAT_24BIT_TC_BGR:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_24BIT_TC_BGR;  break;
        case BMP_FORMAT_24BIT_TC_RGB:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_24BIT_TC_RGB;  break;
        case BMP_FORMAT_24BIT_TC_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_24BIT_TC_MASK; break;
        case BMP_FORMAT_32BIT_TC_ABGR:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_ABGR; break;
        case BMP_FORMAT_32BIT_TC_ARGB:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_ARGB; break;
        case BMP_FORMAT_32BIT_TC_BGRA:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_BGRA; break;
        case BMP_FORMAT_32BIT_TC_RGBA:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_RGBA; break;
        case BMP_FORMAT_32BIT_TC_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_MASK; break;

        default:
            DBG_ERROR("Error: SalPrinterBmp::SalPrinterBmp() unknown bitmap format");
        break;
    }
}

SalPrinterBmp::~SalPrinterBmp ()
{
}

sal_uInt32
SalPrinterBmp::GetWidth () const
{
    return mpBmpBuffer->mnWidth;
}

sal_uInt32
SalPrinterBmp::GetHeight () const
{
    return mpBmpBuffer->mnHeight;
}

sal_uInt32
SalPrinterBmp::GetDepth () const
{
    sal_uInt32 nDepth;

    switch (mpBmpBuffer->mnBitCount)
    {
        case 1:
            nDepth = 1;
            break;

        case 4:
        case 8:
            nDepth = 8;
            break;

        case 16:
        case 24:
        case 32:
            nDepth = 24;
            break;

        default:
            DBG_ERROR ("Error: unsupported bitmap depth in SalPrinterBmp::GetDepth()");
            break;
    }

    return nDepth;
}

sal_uInt32
SalPrinterBmp::ColorOf (BitmapColor& rColor) const
{
    if (rColor.IsIndex())
        return ColorOf (mpBmpBuffer->maPalette[rColor.GetIndex()]);
    else
        return    ((rColor.GetBlue())        & 0x000000ff)
                | ((rColor.GetGreen() <<  8) & 0x0000ff00)
                | ((rColor.GetRed()   << 16) & 0x00ff0000);
}

sal_uInt8
SalPrinterBmp::GrayOf (BitmapColor& rColor) const
{
    if (rColor.IsIndex())
        return GrayOf (mpBmpBuffer->maPalette[rColor.GetIndex()]);
    else
        return (  rColor.GetBlue()  *  28UL
                + rColor.GetGreen() * 151UL
                + rColor.GetRed()   *  77UL ) >> 8;
}

sal_uInt32
SalPrinterBmp::GetPaletteEntryCount () const
{
    return mpBmpBuffer->maPalette.GetEntryCount ();
}

sal_uInt32
SalPrinterBmp::GetPaletteColor (sal_uInt32 nIdx) const
{
    return ColorOf (mpBmpBuffer->maPalette[nIdx]);
}

sal_uInt32
SalPrinterBmp::GetPixelRGB (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel (pScan, nColumn, mpBmpBuffer->maColorMask);

    return ColorOf (aColor);
}

sal_uInt8
SalPrinterBmp::GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel (pScan, nColumn, mpBmpBuffer->maColorMask);

    return GrayOf (aColor);
}

sal_uInt8
SalPrinterBmp::GetPixelIdx (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel (pScan, nColumn, mpBmpBuffer->maColorMask);

    if (aColor.IsIndex())
        return aColor.GetIndex();
    else
        return 0;
}
#endif

// -=-= SalGraphics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::copyBits( const SalTwoRect *pPosAry,
                                  SalGraphics      *pSSrcGraphics )
{
    X11SalGraphics* pSrcGraphics = pSSrcGraphics
        ? static_cast<X11SalGraphics*>(pSSrcGraphics)
        : this;
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_ERROR( "Error: PrinterGfx::CopyBits() not implemented" );
    }
    else
    {
#endif

        if( pPosAry->mnSrcWidth <= 0
            || pPosAry->mnSrcHeight <= 0
            || pPosAry->mnDestWidth <= 0
            || pPosAry->mnDestHeight <= 0 )
        {
            return;
        }

        int n;
        if( pSrcGraphics == this )
        {
            n = 2;
        }
        else if( pSrcGraphics->bWindow_ )
        {
            // window or compatible virtual device
            if( pSrcGraphics->GetDisplay() == GetDisplay() )
                n = 2; // same Display
            else
                n = 1; // printer or other display
        }
        else if( pSrcGraphics->bVirDev_ )
        {
            // printer compatible virtual device
            if( bPrinter_ )
                n = 2; // printer or compatible virtual device == same display
            else
                n = 1; // window or compatible virtual device
        }
        else
            n = 0;

        if( n == 2
            && pPosAry->mnSrcWidth  == pPosAry->mnDestWidth
            && pPosAry->mnSrcHeight == pPosAry->mnDestHeight )
        {
            if( bXORMode_
                && !pSrcGraphics->bVirDev_
                && (GetDisplay()->GetProperties() & PROPERTY_BUG_XCopyArea_GXxor) )
            {
                Pixmap hPixmap = XCreatePixmap( GetXDisplay(),
                                                pSrcGraphics->GetDrawable(),        // source
                                                pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                                                pSrcGraphics->GetBitCount() );
                XCopyArea( GetXDisplay(),
                           pSrcGraphics->GetDrawable(),     // source
                           hPixmap,                             // destination
                           GetDisplay()->GetCopyGC(),           // no clipping
                           pPosAry->mnSrcX,     pPosAry->mnSrcY,
                           pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                           0,                   0 );            // destination
                XCopyArea( GetXDisplay(),
                           hPixmap,                             // source
                           GetDrawable(),                       // destination
                           GetInvertGC(),       // destination clipping
                           0,                   0,              // source
                           pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                           pPosAry->mnDestX,    pPosAry->mnDestY );
                XFreePixmap( GetXDisplay(), hPixmap );
            }
            else
                XCopyArea( GetXDisplay(),
                           pSrcGraphics->GetDrawable(),     // source
                           GetDrawable(),                       // destination
                           GetCopyGC(),         // destination clipping
                           pPosAry->mnSrcX,     pPosAry->mnSrcY,
                           pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                           pPosAry->mnDestX,    pPosAry->mnDestY );
        }
        else if( n )
        {
            SalBitmap *pDDB = pSrcGraphics->getBitmap( pPosAry->mnSrcX,
                                                       pPosAry->mnSrcY,
                                                       pPosAry->mnSrcWidth,
                                                       pPosAry->mnSrcHeight );

            if( !pDDB )
            {
                stderr0( "SalGraphics::CopyBits !pSrcGraphics->GetBitmap()\n" );
                return;
            }

            SalTwoRect aPosAry( *pPosAry );

            aPosAry.mnSrcX = 0, aPosAry.mnSrcY = 0;
            drawBitmap( &aPosAry, *pDDB );

            delete pDDB;
        }
        else
            stderr0( "X11SalGraphics::CopyBits from Printer not yet implemented\n" );

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// --------------------------------------------------------------------------

void X11SalGraphics::copyArea ( long nDestX,    long nDestY,
                                   long nSrcX,     long nSrcY,
                                   long nSrcWidth, long nSrcHeight,
                                   USHORT nFlags )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_ERROR( "Error: PrinterGfx::CopyArea() not implemented" );
    }
    else
    {
#endif

        SalTwoRect aPosAry;

        aPosAry.mnDestX = nDestX;
        aPosAry.mnDestY = nDestY;
        aPosAry.mnDestWidth  = nSrcWidth;
        aPosAry.mnDestHeight = nSrcHeight;

        aPosAry.mnSrcX = nSrcX;
        aPosAry.mnSrcY = nSrcY;
        aPosAry.mnSrcWidth  = nSrcWidth;
        aPosAry.mnSrcHeight = nSrcHeight;

        copyBits ( &aPosAry, 0 );

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        Rectangle aSrc (Point(pPosAry->mnSrcX, pPosAry->mnSrcY),
                        Size(pPosAry->mnSrcWidth, pPosAry->mnSrcHeight));
        Rectangle aDst (Point(pPosAry->mnDestX, pPosAry->mnDestY),
                        Size(pPosAry->mnDestWidth, pPosAry->mnDestHeight));

        BitmapBuffer* pBuffer= const_cast<SalBitmap&>(rSalBitmap).AcquireBuffer(sal_True);

        SalPrinterBmp aBmp (pBuffer);
        m_pPrinterGfx->DrawBitmap (aDst, aSrc, aBmp);

        const_cast<SalBitmap&>(rSalBitmap).ReleaseBuffer (pBuffer, sal_True);
    }
    else
    {
#endif

        SalDisplay*         pSalDisp = GetDisplay();
        Display*            pXDisp = pSalDisp->GetDisplay();
        const Drawable      aDrawable( GetDrawable() );
        const SalColormap&  rColMap = pSalDisp->GetColormap();
        const long          nDepth = GetDisplay()->GetVisual()->GetDepth();
        GC                  aGC( GetCopyGC() );
        XGCValues           aOldVal, aNewVal;
        int                 nValues = GCForeground | GCBackground;

        // set foreground/background values for 1Bit bitmaps
        XGetGCValues( pXDisp, aGC, nValues, &aOldVal );
        aNewVal.foreground = rColMap.GetWhitePixel(), aNewVal.background = rColMap.GetBlackPixel();
        XChangeGC( pXDisp, aGC, nValues, &aNewVal );

        static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aDrawable, nDepth, *pPosAry, aGC );

        XChangeGC( pXDisp, aGC, nValues, &aOldVal );
        XFlush( pXDisp );

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalGraphics::drawBitmap( const SalTwoRect* pPosAry,
                                 const SalBitmap& rSalBitmap,
                                 const SalBitmap& rTransBitmap )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_ERROR("Error: no PrinterGfx::DrawBitmap() for transparent bitmap");
    }
    else
    {
#endif

        DBG_ASSERT( !bPrinter_, "Drawing of transparent bitmaps on printer devices is strictly forbidden" );

        SalDisplay*     pSalDisp = GetDisplay();
        Display*        pXDisp = pSalDisp->GetDisplay();
        Drawable        aDrawable( GetDrawable() );
        const USHORT    nDepth = pSalDisp->GetVisual()->GetDepth();
        Pixmap          aFG( XCreatePixmap( pXDisp, aDrawable, pPosAry->mnDestWidth,
                                            pPosAry->mnDestHeight, nDepth ) );
        Pixmap          aBG( XCreatePixmap( pXDisp, aDrawable, pPosAry->mnDestWidth,
                                            pPosAry->mnDestHeight, nDepth ) );

        if( aFG && aBG )
        {
            GC                  aTmpGC;
            XGCValues           aValues;
            const SalColormap&  rColMap = pSalDisp->GetColormap();
            const int           nBlack = rColMap.GetBlackPixel(), nWhite = rColMap.GetWhitePixel();
            const int           nValues = GCFunction | GCForeground | GCBackground;
            SalTwoRect          aTmpRect( *pPosAry ); aTmpRect.mnDestX = aTmpRect.mnDestY = 0;

            // draw paint bitmap in pixmap #1
            aValues.function = GXcopy, aValues.foreground = nWhite, aValues.background = nBlack;
            aTmpGC = XCreateGC( pXDisp, aFG, nValues, &aValues );
            static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aFG, nDepth, aTmpRect, aTmpGC );
            DBG_TESTTRANS( aFG );

            // draw background in pixmap #2
            XCopyArea( pXDisp, aDrawable, aBG, aTmpGC,
                       pPosAry->mnDestX, pPosAry->mnDestY,
                       pPosAry->mnDestWidth, pPosAry->mnDestHeight,
                       0, 0 );
            DBG_TESTTRANS( aBG );

            // mask out paint bitmap in pixmap #1 (transparent areas 0)
            aValues.function = GXand, aValues.foreground = 0x00000000, aValues.background = 0xffffffff;
            XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
            static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aFG, 1, aTmpRect, aTmpGC );

            DBG_TESTTRANS( aFG );

            // #105055# For XOR mode, keep background behind bitmap intact
            if( !bXORMode_ )
            {
                // mask out background in pixmap #2 (nontransparent areas 0)
                aValues.function = GXand, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
                XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
                static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aBG, 1, aTmpRect, aTmpGC );

                DBG_TESTTRANS( aBG );
            }

            // merge pixmap #1 and pixmap #2 in pixmap #2
            aValues.function = GXxor, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
            XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
            XCopyArea( pXDisp, aFG, aBG, aTmpGC,
                       0, 0,
                       pPosAry->mnDestWidth, pPosAry->mnDestHeight,
                   0, 0 );
            DBG_TESTTRANS( aBG );

            // #105055# Disable XOR temporarily
            BOOL bOldXORMode( bXORMode_ );
            bXORMode_ = FALSE;

            // copy pixmap #2 (result) to background
            XCopyArea( pXDisp, aBG, aDrawable, GetCopyGC(),
                       0, 0,
                       pPosAry->mnDestWidth, pPosAry->mnDestHeight,
                       pPosAry->mnDestX, pPosAry->mnDestY );
            DBG_TESTTRANS( aBG );

            bXORMode_ = bOldXORMode;

            XFreeGC( pXDisp, aTmpGC );
            XFlush( pXDisp );
        }
        else
            drawBitmap( pPosAry, rSalBitmap );

        if( aFG )
            XFreePixmap( pXDisp, aFG );

        if( aBG )
            XFreePixmap( pXDisp, aBG );

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::drawBitmap( const SalTwoRect* pPosAry,
                                 const SalBitmap& rSalBitmap,
                                 SalColor nTransparentColor )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_ERROR("Error: no PrinterGfx::DrawBitmap() for transparent color");
    }
    else
    {
#endif

        DBG_ERROR( "::DrawBitmap with transparent color not supported" );

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::drawMask( const SalTwoRect* pPosAry,
                               const SalBitmap &rSalBitmap,
                               SalColor nMaskColor )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_ERROR("Error: PrinterGfx::DrawMask() not implemented");
    }
    else
    {
#endif

        SalDisplay* pSalDisp = GetDisplay();
        Display*    pXDisp = pSalDisp->GetDisplay();
        Drawable    aDrawable( GetDrawable() );
        Pixmap      aStipple( XCreatePixmap( pXDisp, aDrawable,
                                             pPosAry->mnDestWidth,
                                             pPosAry->mnDestHeight, 1 ) );

        if( aStipple )
        {
            SalTwoRect  aTwoRect( *pPosAry ); aTwoRect.mnDestX = aTwoRect.mnDestY = 0;
            GC          aTmpGC;
            XGCValues   aValues;

            // create a stipple bitmap first (set bits are changed to unset bits and vice versa)
            aValues.function = GXcopyInverted;
            aValues.foreground = 1, aValues.background = 0;
            aTmpGC = XCreateGC( pXDisp, aStipple, GCFunction | GCForeground | GCBackground, &aValues );
            static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aStipple, 1, aTwoRect, aTmpGC );

            XFreeGC( pXDisp, aTmpGC );

            // Set stipple and draw rectangle
            GC  aStippleGC( GetStippleGC() );
            int nX = pPosAry->mnDestX, nY = pPosAry->mnDestY;

            XSetStipple( pXDisp, aStippleGC, aStipple );
            XSetTSOrigin( pXDisp, aStippleGC, nX, nY );
            XSetForeground( pXDisp, aStippleGC, GetPixel( nMaskColor ) );
            XFillRectangle( pXDisp, aDrawable, aStippleGC,
                            nX, nY,
                            pPosAry->mnDestWidth, pPosAry->mnDestHeight );
            XFreePixmap( pXDisp, aStipple );
            XFlush( pXDisp );
        }
        else
            drawBitmap( pPosAry, rSalBitmap );

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalBitmap *X11SalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_WARNING ("Warning: PrinterGfx::GetBitmap() not implemented");
        return NULL;
    }
    else
    {
#endif

        if( bPrinter_ && !bVirDev_ )
            return NULL;

        if( bWindow_ && !bVirDev_ )
        {
            // normalize
            if( nDX < 0 )
            {
                nX += nDX;
                nDX = -nDX;
            }
            if ( nDY < 0 )
            {
                nY += nDY;
                nDY = -nDY;
            }

            XWindowAttributes aAttrib;

            XGetWindowAttributes( GetXDisplay(), GetDrawable(), &aAttrib );
            if( aAttrib.map_state != IsViewable )
            {
                stderr0( "X11SalGraphics::GetBitmap drawable not viewable\n" );
                return NULL;
            }

            // am Window clippen (eg)
            if ( nX < 0 )
            {
                nDX += nX;
                nX   = 0;
            }
            if ( nY < 0 )
            {
                nDY += nY;
                nY   = 0;
            }
            if( nX + nDX > aAttrib.width )
                nDX = aAttrib.width  - nX;
            if( nY + nDY > aAttrib.height )
                nDY = aAttrib.height - nY;

            // nun alles ok ?
            if( nDX <= 0 || nDY <= 0 )
            {
                stderr0( "X11SalGraphics::GetBitmap zero sized bitmap after clipping\n" );
                return NULL;
            }
        }

        X11SalBitmap*   pSalBitmap = new X11SalBitmap;
        USHORT          nBitCount = GetBitCount();

        if( &GetDisplay()->GetColormap() != &GetColormap() )
            nBitCount = 1;


        pSalBitmap->ImplCreateFromDrawable( GetDrawable(), nBitCount, nX, nY, nDX, nDY );
        return pSalBitmap;

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColor X11SalGraphics::getPixel( long nX, long nY )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_ERROR ("Warning: PrinterGfx::GetPixel() not implemented");
        return 0;
    }
    else
    {
#endif

        if( bWindow_ && !bVirDev_ )
        {
            XWindowAttributes aAttrib;

            XGetWindowAttributes( GetXDisplay(), GetDrawable(), &aAttrib );
            if( aAttrib.map_state != IsViewable )
            {
                stderr0( "X11SalGraphics::GetPixel drawable not viewable\n" );
                return 0;
            }
        }

        XImage *pXImage = XGetImage( GetXDisplay(),
                                     GetDrawable(),
                                     nX, nY,
                                     1,  1,
                                     AllPlanes,
                                     ZPixmap );
        if( !pXImage )
        {
            stderr0( "X11SalGraphics::GetPixel !XGetImage()\n" );
            return 0;
        }

        XColor aXColor;

        aXColor.pixel = XGetPixel( pXImage, 0, 0 );
        XDestroyImage( pXImage );

        return GetColormap().GetColor( aXColor.pixel );

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::invert( long       nX,
                                long        nY,
                                long        nDX,
                                long        nDY,
                                SalInvert   nFlags )
{
#ifndef _USE_PRINT_EXTENSION_
    if (m_pPrinterGfx != NULL)
    {
        DBG_ERROR ("Warning: PrinterGfx::Invert() not implemented");
    }
    else
    {
#endif

        SalDisplay *pDisp = GetDisplay();

        GC pGC;
        if( SAL_INVERT_50 & nFlags )
        {
            pGC = GetInvert50GC();
            XFillRectangle( GetXDisplay(), GetDrawable(), pGC, nX, nY, nDX, nDY );
        }
        else
            if ( SAL_INVERT_TRACKFRAME & nFlags )
            {
                pGC = GetTrackingGC();
                XDrawRectangle( GetXDisplay(), GetDrawable(),  pGC, nX, nY, nDX, nDY );
            }
            else
            {
                pGC = GetInvertGC();
                XFillRectangle( GetXDisplay(), GetDrawable(),  pGC, nX, nY, nDX, nDY );
            }

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

