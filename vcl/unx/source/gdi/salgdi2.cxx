/*************************************************************************
 *
 *  $RCSfile: salgdi2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:43 $
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
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef PRINTER_DUMMY
#define Font XLIB_Font
#define Region XLIB_Region
#include <xprinter/xp.h>
#undef Font
#undef Region
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

#if defined DEBUG && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )                              \
{                                                                   \
    XCopyArea( pXDisp, _def_drawable, aDrawable, _GetCopyGC(),      \
               0, 0,                                                \
               pPosAry->mnDestWidth, pPosAry->mnDestHeight,         \
               0, 0 );                                              \
}
#else // DEBUG && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )
#endif // DEBUG && defined SALGDI2_TESTTRANS

// -=-= SalGraphicsData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final GC SalGraphicsData::CreateGC( Drawable hDrawable, unsigned long nMask )
{
    XGCValues values;

    values.graphics_exposures   = True;
    values.foreground           = xColormap_->GetBlackPixel()
                                  ^ xColormap_->GetWhitePixel();
    values.function             = GXxor;
    values.line_width           = 1;
    values.fill_style           = FillStippled;
    values.stipple              = GetDisplay()->GetInvert50();

    return XCreateGC( GetXDisplay(), hDrawable, nMask, &values );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final inline GC SalGraphicsData::GetMonoGC( Pixmap hPixmap )
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
final inline GC SalGraphicsData::GetCopyGC()
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
final GC SalGraphicsData::GetInvertGC()
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
final GC SalGraphicsData::GetInvert50GC()
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
final inline GC SalGraphicsData::GetStippleGC()
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
final int SalGraphicsData::Clip( XLIB_Region   pRegion,
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
final int SalGraphicsData::Clip( int          &nX,
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
final GC SalGraphicsData::SetMask( int           &nX,
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
#if defined DEBUG || defined DBG_UTIL
        fprintf( stderr, "SalGraphicsData::SetMask !hPixmap\n" );
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

// -=-= SalGraphics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::CopyBits( const SalTwoRect *pPosAry,
                                  SalGraphics      *pSrcGraphics )
{
    if( pPosAry->mnSrcWidth <= 0
        || pPosAry->mnSrcHeight <= 0
        || pPosAry->mnDestWidth <= 0
        || pPosAry->mnDestHeight <= 0 )
    {
        return;
    }

    int n;
    if( !pSrcGraphics )
    {
        pSrcGraphics = this;
        n = 2;
    }
    else if( pSrcGraphics->_IsWindow() )
        // window or compatible virtual device
        if( pSrcGraphics->_GetDisplay() == _GetDisplay() )
            n = 2; // same Display
        else
            n = 1; // printer or other display
    else if( pSrcGraphics->_IsVirtualDevice() )
        // printer compatible virtual device
        if( _IsPrinter() )
            n = 2; // printer or compatible virtual device == same display
        else
            n = 1; // window or compatible virtual device
    else
        n = 0;

    if( n == 2
        && pPosAry->mnSrcWidth  == pPosAry->mnDestWidth
        && pPosAry->mnSrcHeight == pPosAry->mnDestHeight )
    {
        if( _IsXORMode()
            && !pSrcGraphics->_IsVirtualDevice()
            && (_GetDisplay()->GetProperties() & PROPERTY_BUG_XCopyArea_GXxor) )
        {
            Pixmap hPixmap = XCreatePixmap( _GetXDisplay(),
                                            pSrcGraphics->_GetDrawable(),       // source
                                            pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                                            pSrcGraphics->GetBitCount() );
            XCopyArea( _GetXDisplay(),
                       pSrcGraphics->_GetDrawable(),        // source
                       hPixmap,                             // destination
                       _GetDisplay()->GetCopyGC(),          // no clipping
                       pPosAry->mnSrcX,     pPosAry->mnSrcY,
                       pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                       0,                   0 );            // destination
            XCopyArea( _GetXDisplay(),
                       hPixmap,                             // source
                       _GetDrawable(),                      // destination
                       maGraphicsData.GetInvertGC(),        // destination clipping
                       0,                   0,              // source
                       pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                       pPosAry->mnDestX,    pPosAry->mnDestY );
            XFreePixmap( _GetXDisplay(), hPixmap );
        }
        else
            XCopyArea( _GetXDisplay(),
                       pSrcGraphics->_GetDrawable(),        // source
                       _GetDrawable(),                      // destination
                       maGraphicsData.GetCopyGC(),          // destination clipping
                       pPosAry->mnSrcX,     pPosAry->mnSrcY,
                       pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                       pPosAry->mnDestX,    pPosAry->mnDestY );
    }
    else if( n )
    {
        SalBitmap *pDDB = pSrcGraphics->GetBitmap( pPosAry->mnSrcX,
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
        DrawBitmap( &aPosAry, *pDDB );

        delete pDDB;
    }
    else
        stderr0( "SalGraphics::CopyBits from Printer not yet implemented\n" );
}

// --------------------------------------------------------------------------

final void SalGraphics::CopyArea ( long nDestX,    long nDestY,
                                   long nSrcX,     long nSrcY,
                                   long nSrcWidth, long nSrcHeight,
                                   USHORT nFlags )
{
    SalTwoRect aPosAry;

    aPosAry.mnDestX = nDestX;
    aPosAry.mnDestY = nDestY;
    aPosAry.mnDestWidth  = nSrcWidth;
    aPosAry.mnDestHeight = nSrcHeight;

    aPosAry.mnSrcX = nSrcX;
    aPosAry.mnSrcY = nSrcY;
    aPosAry.mnSrcWidth  = nSrcWidth;
    aPosAry.mnSrcHeight = nSrcHeight;

    CopyBits ( &aPosAry, 0 );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
    SalDisplay*         pSalDisp = maGraphicsData.GetDisplay();
    Display*            pXDisp = pSalDisp->GetDisplay();
    const Drawable      aDrawable( maGraphicsData.GetDrawable() );
    const SalColormap&  rColMap = pSalDisp->GetColormap();
    const long          nDepth = maGraphicsData.GetDisplay()->GetVisual()->GetDepth();
    GC                  aGC( maGraphicsData.GetCopyGC() );
    XGCValues           aOldVal, aNewVal;
    int                 nValues = GCForeground | GCBackground;

    // set foreground/background values for 1Bit bitmaps
    XGetGCValues( pXDisp, aGC, nValues, &aOldVal );
    aNewVal.foreground = rColMap.GetWhitePixel(), aNewVal.background = rColMap.GetBlackPixel();
    XChangeGC( pXDisp, aGC, nValues, &aNewVal );

    if( _IsPrinter() )
    {
        SalTwoRect  aTwoRect = { pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                                 0, 0, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight };
        XImage*     pImage = rSalBitmap.ImplCreateXImage( pSalDisp, nDepth, aTwoRect );

        if( pImage )
        {
            XpStretchPutImage( maGraphicsData.GetDisplay()->GetDisplay(), aDrawable, aGC, pImage,
                               0, 0, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight,
                               pPosAry->mnDestX, pPosAry->mnDestY, pPosAry->mnDestWidth, pPosAry->mnDestHeight );

            delete[] pImage->data, pImage->data = NULL;
            XDestroyImage( pImage );
        }
    }
    else
        rSalBitmap.ImplDraw( aDrawable, nDepth, *pPosAry, aGC );

    XChangeGC( pXDisp, aGC, nValues, &aOldVal );
    XFlush( pXDisp );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

final void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransBitmap )
{
    DBG_ASSERT( !_IsPrinter(), "Drawing of transparent bitmaps on printer devices is strictly forbidden" );

    SalDisplay*     pSalDisp = maGraphicsData.GetDisplay();
    Display*        pXDisp = pSalDisp->GetDisplay();
    Drawable        aDrawable( maGraphicsData.GetDrawable() );
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
        rSalBitmap.ImplDraw( aFG, nDepth, aTmpRect, aTmpGC );
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
        rTransBitmap.ImplDraw( aFG, 1, aTmpRect, aTmpGC );
        DBG_TESTTRANS( aFG );

        // mask out background in pixmap #2 (nontransparent areas 0)
        aValues.function = GXand, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
        XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
        rTransBitmap.ImplDraw( aBG, 1, aTmpRect, aTmpGC );
        DBG_TESTTRANS( aBG );

        // merge pixmap #1 and pixmap #2 in pixmap #2
        aValues.function = GXxor, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
        XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
        XCopyArea( pXDisp, aFG, aBG, aTmpGC,
                   0, 0,
                   pPosAry->mnDestWidth, pPosAry->mnDestHeight,
                   0, 0 );
        DBG_TESTTRANS( aBG );

        // copy pixmap #2 (result) to background
        XCopyArea( pXDisp, aBG, aDrawable, maGraphicsData.GetCopyGC(),
                   0, 0,
                   pPosAry->mnDestWidth, pPosAry->mnDestHeight,
                   pPosAry->mnDestX, pPosAry->mnDestY );
        DBG_TESTTRANS( aBG );

        XFreeGC( pXDisp, aTmpGC );
        XFlush( pXDisp );
    }
    else
        DrawBitmap( pPosAry, rSalBitmap );

    if( aFG )
        XFreePixmap( pXDisp, aFG );

    if( aBG )
        XFreePixmap( pXDisp, aBG );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap,
                              SalColor nTransparentColor )
{
    DBG_ERROR( "::DrawBitmap with transparent color not supported" );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::DrawMask( const SalTwoRect* pPosAry, const SalBitmap &rSalBitmap,
                                  SalColor nMaskColor )
{
    SalDisplay* pSalDisp = maGraphicsData.GetDisplay();
    Display*    pXDisp = pSalDisp->GetDisplay();
    Drawable    aDrawable( maGraphicsData.GetDrawable() );
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
        rSalBitmap.ImplDraw( aStipple, 1, aTwoRect, aTmpGC );
        XFreeGC( pXDisp, aTmpGC );

        // Set stipple and draw rectangle
        GC  aStippleGC( maGraphicsData.GetStippleGC() );
        int nX = pPosAry->mnDestX, nY = pPosAry->mnDestY;

        XSetStipple( pXDisp, aStippleGC, aStipple );
        XSetTSOrigin( pXDisp, aStippleGC, nX, nY );
        XSetForeground( pXDisp, aStippleGC, _GetPixel( nMaskColor ) );
        XFillRectangle( pXDisp, aDrawable, aStippleGC,
                        nX, nY,
                        pPosAry->mnDestWidth, pPosAry->mnDestHeight );
        XFreePixmap( pXDisp, aStipple );
        XFlush( pXDisp );
    }
    else
        DrawBitmap( pPosAry, rSalBitmap );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final SalBitmap *SalGraphics::GetBitmap( long nX, long nY, long nDX, long nDY )
{
    if( _IsPrinter() && !_IsVirtualDevice() )
        return NULL;

    if( _IsWindow() && !_IsVirtualDevice() )
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

        XGetWindowAttributes( _GetXDisplay(), _GetDrawable(), &aAttrib );
        if( aAttrib.map_state != IsViewable )
        {
            stderr0( "SalGraphics::GetBitmap drawable not viewable\n" );
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
            stderr0( "SalGraphics::GetBitmap zero sized bitmap after clipping\n" );
            return NULL;
        }
    }

    SalBitmap*  pSalBitmap = new SalBitmap;
    USHORT      nBitCount = GetBitCount();

    if( &_GetDisplay()->GetColormap() != &_GetColormap() )
        nBitCount = 1;

    pSalBitmap->ImplCreateFromDrawable( _GetDrawable(), nBitCount, nX, nY, nDX, nDY );

    return pSalBitmap;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final SalColor SalGraphics::GetPixel( long nX, long nY )
{
    if( _IsWindow() && !_IsVirtualDevice() )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( _GetXDisplay(), _GetDrawable(), &aAttrib );
        if( aAttrib.map_state != IsViewable )
        {
            stderr0( "SalGraphics::GetPixel drawable not viewable\n" );
            return 0;
        }
    }

    XImage *pXImage = XGetImage( _GetXDisplay(),
                                 _GetDrawable(),
                                 nX, nY,
                                 1,  1,
                                 AllPlanes,
                                 ZPixmap );
    if( !pXImage )
    {
        stderr0( "SalGraphics::GetPixel !XGetImage()\n" );
        return 0;
    }

    XColor aXColor;

    aXColor.pixel = XGetPixel( pXImage, 0, 0 );
    XDestroyImage( pXImage );

    return _GetColormap().GetColor( aXColor.pixel );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final void SalGraphics::Invert( long        nX,
                                long        nY,
                                long        nDX,
                                long        nDY,
                                SalInvert   nFlags )
{
    SalDisplay *pDisp = _GetDisplay();

    GC pGC;
    if( SAL_INVERT_50 & nFlags )
    {
        pGC = maGraphicsData.GetInvert50GC();
        XFillRectangle( _GetXDisplay(), _GetDrawable(), pGC, nX, nY, nDX, nDY );
    }
    else
    if ( SAL_INVERT_TRACKFRAME & nFlags )
    {
        pGC = maGraphicsData.GetTrackingGC();
        XDrawRectangle( _GetXDisplay(), _GetDrawable(),  pGC, nX, nY, nDX, nDY );
    }
    else
    {
        pGC = maGraphicsData.GetInvertGC();
        XFillRectangle( _GetXDisplay(), _GetDrawable(),  pGC, nX, nY, nDX, nDY );
    }
}

