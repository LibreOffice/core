/*************************************************************************
 *
 *  $RCSfile: salbmp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-25 10:39:04 $
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

#define _SV_SALBMP_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/shm.h>
#include <prex.h>
#include <postx.h>
#include <salunx.h>
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif

// -----------
// - Defines -
// -----------

#define SAL_DRAWPIXMAP_MAX_EXT 4096

// -------------
// - SalBitmap -
// -------------

ImplSalBitmapCache* SalBitmap::mpCache = NULL;
ULONG               SalBitmap::mnCacheInstCount = 0;

// -----------------------------------------------------------------------------

SalBitmap::SalBitmap() :
    mpDIB( NULL ),
    mpDDB( NULL )
{
}

// -----------------------------------------------------------------------------

SalBitmap::~SalBitmap()
{
    Destroy();
}

// -----------------------------------------------------------------------------

void SalBitmap::ImplCreateCache()
{
    if( !mnCacheInstCount++ )
        mpCache = new ImplSalBitmapCache;
}

// -----------------------------------------------------------------------------

void SalBitmap::ImplDestroyCache()
{
    DBG_ASSERT( mnCacheInstCount, "SalBitmap::ImplDestroyCache(): underflow" );

    if( mnCacheInstCount && !--mnCacheInstCount )
        delete mpCache, mpCache = NULL;
}

// -----------------------------------------------------------------------------

void SalBitmap::ImplRemovedFromCache()
{
    if( mpDDB )
        delete mpDDB, mpDDB = NULL;
}

// -----------------------------------------------------------------------------

BitmapBuffer* SalBitmap::ImplCreateDIB( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
    DBG_ASSERT( nBitCount == 1 || nBitCount == 4 || nBitCount == 8 || nBitCount == 24, "Unsupported BitCount!" );

    BitmapBuffer* pDIB;

    if( rSize.Width() && rSize.Height() )
    {
        pDIB = new BitmapBuffer;

        if( pDIB )
        {
            const USHORT nColors = ( nBitCount <= 8 ) ? ( 1 << nBitCount ) : 0;

            pDIB->mnFormat = BMP_FORMAT_BOTTOM_UP;

            switch( nBitCount )
            {
                case( 1 ): pDIB->mnFormat |= BMP_FORMAT_1BIT_MSB_PAL; break;
                case( 4 ): pDIB->mnFormat |= BMP_FORMAT_4BIT_MSN_PAL; break;
                case( 8 ): pDIB->mnFormat |= BMP_FORMAT_8BIT_PAL; break;

                default:
                    pDIB->mnFormat |= BMP_FORMAT_24BIT_TC_BGR;
                break;
            }

            pDIB->mnWidth = rSize.Width();
            pDIB->mnHeight = rSize.Height();
            pDIB->mnScanlineSize = AlignedWidth4Bytes( pDIB->mnWidth * nBitCount );
            pDIB->mnBitCount = nBitCount;

            if( nColors )
            {
                pDIB->maPalette = rPal;
                pDIB->maPalette.SetEntryCount( nColors );
            }

            pDIB->mpBits = new BYTE[ pDIB->mnScanlineSize * pDIB->mnHeight ];
            rtl_zeroMemory( pDIB->mpBits, pDIB->mnScanlineSize * pDIB->mnHeight );
        }
    }
    else
        pDIB = NULL;

    return pDIB;
}

// -----------------------------------------------------------------------------

BitmapBuffer* SalBitmap::ImplCreateDIB( Drawable aDrawable,
                                        long nDrawableDepth,
                                        long nX, long nY,
                                        long nWidth, long nHeight )
{
    BitmapBuffer* pDIB = NULL;

    if( aDrawable && nWidth && nHeight && nDrawableDepth )
    {
        SalDisplay* pSalDisp = GetSalData()->GetCurDisp();
        SalXLib*    pXLib = pSalDisp->GetXLib();
        Display*    pXDisp = pSalDisp->GetDisplay();

        // do not die on XError here
        // alternatively one could check the coordinates for being offscreen
        // but this call can actually work on servers with backing store
        // defaults even if the rectangle is offscreen
        // so better catch the XError
        BOOL bXError = pXLib->GetIgnoreXErrors();
        pXLib->SetIgnoreXErrors( TRUE );
        XImage* pImage = XGetImage( pXDisp, aDrawable, nX, nY, nWidth, nHeight, AllPlanes, ZPixmap );
        BOOL bWasError = pXLib->WasXError();
        pXLib->SetIgnoreXErrors( bXError );

        if( ! bWasError && pImage && pImage->data )
        {
            const SalTwoRect        aTwoRect = { 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight };
            BitmapBuffer            aSrcBuf;
            ULONG                   nDstFormat = BMP_FORMAT_BOTTOM_UP;
            const BitmapPalette*    pDstPal = NULL;

            aSrcBuf.mnFormat = BMP_FORMAT_TOP_DOWN;
            aSrcBuf.mnWidth = nWidth;
            aSrcBuf.mnHeight = nHeight;
            aSrcBuf.mnBitCount = pImage->bits_per_pixel;
            aSrcBuf.mnScanlineSize = pImage->bytes_per_line;
            aSrcBuf.mpBits = (BYTE*) pImage->data;

            pImage->red_mask = pSalDisp->GetVisual()->red_mask;
            pImage->green_mask = pSalDisp->GetVisual()->green_mask;
            pImage->blue_mask = pSalDisp->GetVisual()->blue_mask;

            switch( aSrcBuf.mnBitCount )
            {
                case( 1 ):
                {
                    aSrcBuf.mnFormat |= ( LSBFirst == pImage->bitmap_bit_order ? BMP_FORMAT_1BIT_LSB_PAL : BMP_FORMAT_1BIT_MSB_PAL );
                    nDstFormat |= BMP_FORMAT_1BIT_MSB_PAL;
                }
                break;

                case( 4 ):
                {
                    aSrcBuf.mnFormat |= ( LSBFirst == pImage->bitmap_bit_order ? BMP_FORMAT_4BIT_LSN_PAL : BMP_FORMAT_4BIT_MSN_PAL );
                    nDstFormat |= BMP_FORMAT_4BIT_MSN_PAL;
                }
                break;

                case( 8 ):
                {
                    aSrcBuf.mnFormat |= BMP_FORMAT_8BIT_PAL;
                    nDstFormat |= BMP_FORMAT_8BIT_PAL;
                }
                break;

                case( 16 ):
                {
                    aSrcBuf.mnFormat |= BMP_FORMAT_16BIT_TC_MASK;
                    nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;

                    if( LSBFirst == pImage->byte_order )
                        aSrcBuf.maColorMask = ColorMask( pImage->red_mask, pImage->green_mask, pImage->blue_mask );
                    else
                        aSrcBuf.maColorMask = ColorMask( SWAPSHORT( pImage->red_mask ), SWAPSHORT( pImage->green_mask ), SWAPSHORT( pImage->blue_mask ) );
                }
                break;

                case( 24 ):
                {
                    if( ( LSBFirst == pImage->byte_order ) && ( pImage->red_mask == 0xFF ) )
                        aSrcBuf.mnFormat |= BMP_FORMAT_24BIT_TC_RGB;
                    else
                        aSrcBuf.mnFormat |= BMP_FORMAT_24BIT_TC_BGR;

                    nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;
                }
                break;

                case( 32 ):
                {
                    if( LSBFirst == pImage->byte_order )
                        aSrcBuf.mnFormat |= (  pSalDisp->GetVisual()->red_mask == 0xFF ? BMP_FORMAT_32BIT_TC_RGBA : BMP_FORMAT_32BIT_TC_BGRA );
                    else
                        aSrcBuf.mnFormat |= (  pSalDisp->GetVisual()->red_mask == 0xFF ? BMP_FORMAT_32BIT_TC_ABGR : BMP_FORMAT_32BIT_TC_ARGB );

                    nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;
                }
                break;
            }

            BitmapPalette& rPal = aSrcBuf.maPalette;

            if( aSrcBuf.mnBitCount == 1 )
            {
                rPal.SetEntryCount( 2 );
                pDstPal = &rPal;

                rPal[ 0 ] = Color( COL_BLACK );
                rPal[ 1 ] = Color( COL_WHITE );
            }
            else if( aSrcBuf.mnBitCount <= 8 )
            {
                SalColormap&    rColMap = pSalDisp->GetColormap();
                const USHORT    nCols = Min( (USHORT)rColMap.GetUsed(), 1 << nDrawableDepth );

                rPal.SetEntryCount( nCols );
                pDstPal = &rPal;

                for( USHORT i = 0; i < nCols; i++ )
                {
                    const SalColor  nColor( rColMap.GetColor( i ) );
                    BitmapColor&    rBmpCol = rPal[ i ];

                    rBmpCol.SetRed( SALCOLOR_RED( nColor ) );
                    rBmpCol.SetGreen( SALCOLOR_GREEN( nColor ) );
                    rBmpCol.SetBlue( SALCOLOR_BLUE( nColor ) );
                }
            }

            pDIB = StretchAndConvert( aSrcBuf, aTwoRect, nDstFormat, const_cast<BitmapPalette*>(pDstPal) );
            XDestroyImage( pImage );
        }
    }

    return pDIB;
}

// -----------------------------------------------------------------------------

XImage* SalBitmap::ImplCreateXImage( SalDisplay *pSalDisp, long nDepth, const SalTwoRect& rTwoRect ) const
{
    XImage* pImage = NULL;

    if( !mpDIB && mpDDB )
    {
        ( (SalBitmap*) this )->mpDIB = ImplCreateDIB( mpDDB->ImplGetPixmap(),
                                                      mpDDB->ImplGetDepth(),
                                                      0, 0,
                                                      mpDDB->ImplGetWidth(),
                                                      mpDDB->ImplGetHeight() );
    }

    if( mpDIB && mpDIB->mnWidth && mpDIB->mnHeight )
    {
        Display*    pXDisp = pSalDisp->GetDisplay();
        long        nWidth = rTwoRect.mnDestWidth;
        long        nHeight = rTwoRect.mnDestHeight;

        if( 1 == GetBitCount() )
            nDepth = 1;

        pImage = XCreateImage( pXDisp, pSalDisp->GetVisual()->GetVisual(),
                               nDepth, ( 1 == nDepth ) ? XYBitmap :ZPixmap, 0, NULL,
                               nWidth, nHeight, 32, 0 );

        if( pImage )
        {
            BitmapBuffer*   pDstBuf;
            ULONG           nDstFormat = BMP_FORMAT_TOP_DOWN;
            BitmapPalette*  pPal = NULL;
            ColorMask*      pMask = NULL;

            switch( pImage->bits_per_pixel )
            {
                case( 1 ):
                    nDstFormat |= ( LSBFirst == pImage->bitmap_bit_order ? BMP_FORMAT_1BIT_LSB_PAL : BMP_FORMAT_1BIT_MSB_PAL );
                break;

                case( 4 ):
                    nDstFormat |= ( LSBFirst == pImage->bitmap_bit_order ? BMP_FORMAT_4BIT_LSN_PAL : BMP_FORMAT_4BIT_MSN_PAL );
                break;

                case( 8 ):
                    nDstFormat |= BMP_FORMAT_8BIT_PAL;
                break;

                case( 16 ):
                {
                    nDstFormat |= BMP_FORMAT_16BIT_TC_MASK;

                    if( LSBFirst == pImage->byte_order )
                        pMask = new ColorMask( pImage->red_mask, pImage->green_mask, pImage->blue_mask );
                    else
                        pMask = new ColorMask( SWAPSHORT( pImage->red_mask ), SWAPSHORT( pImage->green_mask ), SWAPSHORT( pImage->blue_mask ) );
                }
                break;

                case( 24 ):
                {
                    if( ( LSBFirst == pImage->byte_order ) && ( pImage->red_mask == 0xFF ) )
                        nDstFormat |= BMP_FORMAT_24BIT_TC_RGB;
                    else
                        nDstFormat |= BMP_FORMAT_24BIT_TC_BGR;
                }
                break;

                case( 32 ):
                {
                    if( LSBFirst == pImage->byte_order )
                        nDstFormat |= ( pImage->red_mask == 0xFF ? BMP_FORMAT_32BIT_TC_RGBA : BMP_FORMAT_32BIT_TC_BGRA );
                    else
                        nDstFormat |= ( pImage->red_mask == 0xFF ? BMP_FORMAT_32BIT_TC_ABGR : BMP_FORMAT_32BIT_TC_ARGB );
                }
                break;
            }

            if( pImage->depth == 1 )
            {
                pPal = new BitmapPalette( 2 );
                (*pPal)[ 0 ] = Color( COL_BLACK );
                (*pPal)[ 1 ] = Color( COL_WHITE );
            }
            else if( pImage->depth <= 8 )
            {
                SalColormap& rColMap = pSalDisp->GetColormap();
                const USHORT nCols = Min( (USHORT)rColMap.GetUsed(), 1 << pImage->depth );

                pPal = new BitmapPalette( nCols );

                for( USHORT i = 0; i < nCols; i++ )
                {
                    const SalColor  nColor( rColMap.GetColor( i ) );
                    BitmapColor&    rBmpCol = (*pPal)[ i ];

                    rBmpCol.SetRed( SALCOLOR_RED( nColor ) );
                    rBmpCol.SetGreen( SALCOLOR_GREEN( nColor ) );
                    rBmpCol.SetBlue( SALCOLOR_BLUE( nColor ) );
                }
            }

            pDstBuf = StretchAndConvert( *mpDIB, rTwoRect, nDstFormat, pPal, pMask );
            delete pPal;
            delete pMask;

            if( pDstBuf && pDstBuf->mpBits )
            {
                // set data in buffer as data member in pImage
                pImage->data = (char*) pDstBuf->mpBits;

                // destroy buffer; don't destroy allocated data in buffer
                delete pDstBuf;
            }
            else
            {
                XDestroyImage( pImage );
                pImage = NULL;
            }
        }
    }

    return pImage;
}

// -----------------------------------------------------------------------------

BOOL SalBitmap::ImplCreateFromDrawable( Drawable aDrawable, long nDrawableDepth,
                                        long nX, long nY, long nWidth, long nHeight )
{
    Destroy();

    if( aDrawable && nWidth && nHeight && nDrawableDepth )
        mpDDB = new ImplSalDDB( aDrawable, nDrawableDepth, nX, nY, nWidth, nHeight );

    return( mpDDB != NULL );
}

// -----------------------------------------------------------------------------

void SalBitmap::ImplDraw( Drawable aDrawable, long nDrawableDepth,
                          const SalTwoRect& rTwoRect, const GC& rGC ) const
{
    if( !mpDDB || !mpDDB->ImplMatches( nDrawableDepth, rTwoRect ) )
    {
        if( mpDDB )
        {
            // do we already have a DIB? if not, create aDIB from current DDB first
            if( !mpDIB )
            {
                ( (SalBitmap*) this )->mpDIB = ImplCreateDIB( mpDDB->ImplGetPixmap(),
                                                              mpDDB->ImplGetDepth(),
                                                              0, 0,
                                                              mpDDB->ImplGetWidth(),
                                                              mpDDB->ImplGetHeight() );
            }

            delete mpDDB, ( (SalBitmap*) this )->mpDDB = NULL;
        }

        if( mpCache )
            mpCache->ImplRemove( const_cast<SalBitmap*>(this) );

        SalTwoRect aTwoRect( rTwoRect );

        // create new DDB from DIB
        if( aTwoRect.mnSrcWidth == aTwoRect.mnDestWidth &&
            aTwoRect.mnSrcHeight == aTwoRect.mnDestHeight )
        {
            const Size aSize( GetSize() );

            aTwoRect.mnSrcX = aTwoRect.mnSrcY = aTwoRect.mnDestX = aTwoRect.mnDestY = 0;
            aTwoRect.mnSrcWidth = aTwoRect.mnDestWidth = aSize.Width();
            aTwoRect.mnSrcHeight = aTwoRect.mnDestHeight = aSize.Height();
        }

        XImage* pImage = ImplCreateXImage( GetSalData()->GetCurDisp(),
                nDrawableDepth, aTwoRect );

        if( pImage )
        {
            ( (SalBitmap*) this )->mpDDB = new ImplSalDDB( pImage, aDrawable, aTwoRect );
            delete[] pImage->data, pImage->data = NULL;
            XDestroyImage( pImage );

            if( mpCache )
                mpCache->ImplAdd( const_cast<SalBitmap*>(this), mpDDB->ImplGetMemSize() );
        }
    }

    if( mpDDB )
        mpDDB->ImplDraw( aDrawable, nDrawableDepth, rTwoRect, rGC );
}

// -----------------------------------------------------------------------------

BOOL SalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
    Destroy();
    mpDIB = ImplCreateDIB( rSize, nBitCount, rPal );

    return( mpDIB != NULL );
}

// -----------------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp )
{
    Destroy();

    if( rSalBmp.mpDIB )
    {
        const Size aSize( rSalBmp.GetSize() );

        mpDIB = ImplCreateDIB( rSalBmp.GetSize(), rSalBmp.GetBitCount(), rSalBmp.mpDIB->maPalette );

        if( mpDIB )
            memcpy( mpDIB->mpBits, rSalBmp.mpDIB->mpBits, mpDIB->mnScanlineSize * mpDIB->mnHeight );
    }
    else if(  rSalBmp.mpDDB )
    {
        ImplCreateFromDrawable( rSalBmp.mpDDB->ImplGetPixmap(), rSalBmp.mpDDB->ImplGetDepth(),
                                0, 0, rSalBmp.mpDDB->ImplGetWidth(), rSalBmp.mpDDB->ImplGetHeight() );
    }

    return( ( !rSalBmp.mpDIB && !rSalBmp.mpDDB ) ||
            ( rSalBmp.mpDIB && ( mpDIB != NULL ) ) ||
            ( rSalBmp.mpDDB && ( mpDDB != NULL ) ) );
}

// -----------------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    return FALSE;
}

// -----------------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
    return FALSE;
}

// -----------------------------------------------------------------------------

void SalBitmap::Destroy()
{
    if( mpDIB )
    {
        delete[] mpDIB->mpBits;
        delete mpDIB, mpDIB = NULL;
    }

    if( mpDDB )
        delete mpDDB, mpDDB = NULL;

    if( mpCache )
        mpCache->ImplRemove( this );
}

// -----------------------------------------------------------------------------

Size SalBitmap::GetSize() const
{
    Size aSize;

    if( mpDIB )
        aSize.Width() = mpDIB->mnWidth, aSize.Height() = mpDIB->mnHeight;
    else if( mpDDB )
        aSize.Width() = mpDDB->ImplGetWidth(), aSize.Height() = mpDDB->ImplGetHeight();

    return aSize;
}

// -----------------------------------------------------------------------------

USHORT SalBitmap::GetBitCount() const
{
    USHORT nBitCount;

    if( mpDIB )
        nBitCount = mpDIB->mnBitCount;
    else if( mpDDB )
    {
        nBitCount = mpDDB->ImplGetDepth();

        if( nBitCount && nBitCount > 1 )
        {
            if( nBitCount <= 4 )
                nBitCount = 4;
            else if( nBitCount <= 8 )
                nBitCount = 8;
            else
                nBitCount = 24;
        }
    }
    else
        nBitCount = 0;

    return nBitCount;
}

// -----------------------------------------------------------------------------

BitmapBuffer* SalBitmap::AcquireBuffer( BOOL bReadOnly )
{
    if( !mpDIB && mpDDB )
    {
        mpDIB = ImplCreateDIB( mpDDB->ImplGetPixmap(), mpDDB->ImplGetDepth(),
                               0, 0, mpDDB->ImplGetWidth(), mpDDB->ImplGetHeight() );
    }

    return mpDIB;
}

// -----------------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
    if( !bReadOnly )
    {
        if( mpDDB )
            delete mpDDB, mpDDB = NULL;

        if( mpCache )
            mpCache->ImplRemove( this );
    }
}

// --------------
// - ImplSalDDB -
// --------------

ImplSalDDB::ImplSalDDB( XImage* pImage, Drawable aDrawable, const SalTwoRect& rTwoRect ) :
    maPixmap    ( 0 ),
    maTwoRect   ( rTwoRect ),
    mnDepth     ( pImage->depth )
{
    SalDisplay* pSalDisp = GetSalData()->GetCurDisp();
    Display*    pXDisp = pSalDisp->GetDisplay();

    if( maPixmap = XCreatePixmap( pXDisp, aDrawable, ImplGetWidth(), ImplGetHeight(), ImplGetDepth() ) )
    {
        XGCValues   aValues;
        GC          aGC;
        int         nValues = GCFunction;

        aValues.function = GXcopy;

        if( 1 == mnDepth )
        {
            nValues |= ( GCForeground | GCBackground );
            aValues.foreground = 1, aValues.background = 0;
        }

        aGC = XCreateGC( pXDisp, maPixmap, nValues, &aValues );
        XPutImage( pXDisp, maPixmap, aGC, pImage, 0, 0, 0, 0, maTwoRect.mnDestWidth, maTwoRect.mnDestHeight );
        XFreeGC( pXDisp, aGC );
    }
}

// -----------------------------------------------------------------------------

ImplSalDDB::ImplSalDDB( Drawable aDrawable, long nDrawableDepth, long nX, long nY, long nWidth, long nHeight ) :
    mnDepth( nDrawableDepth )
{
    SalDisplay* pSalDisp = GetSalData()->GetCurDisp();
    Display*    pXDisp = pSalDisp->GetDisplay();

    if( maPixmap = XCreatePixmap( pXDisp, aDrawable, nWidth, nHeight, nDrawableDepth ) )
    {
        XGCValues   aValues;
        GC          aGC;
        int         nValues = GCFunction;

        aValues.function = GXcopy;

        if( 1 == mnDepth )
        {
            nValues |= ( GCForeground | GCBackground );
            aValues.foreground = 1, aValues.background = 0;
        }

        aGC = XCreateGC( pXDisp, maPixmap, nValues, &aValues );
        ImplDraw( aDrawable, nDrawableDepth, maPixmap, mnDepth,
                  nX, nY, nWidth, nHeight, 0, 0, aGC );
        XFreeGC( pXDisp, aGC );

        maTwoRect.mnSrcX = maTwoRect.mnSrcY = maTwoRect.mnDestX = maTwoRect.mnDestY = 0;
        maTwoRect.mnSrcWidth = maTwoRect.mnDestWidth = nWidth;
        maTwoRect.mnSrcHeight = maTwoRect.mnDestHeight = nHeight;
    }
}

// -----------------------------------------------------------------------------

ImplSalDDB::~ImplSalDDB()
{
    if( maPixmap && ImplGetSVData() )
        XFreePixmap( GetSalData()->GetCurDisp()->GetDisplay(), maPixmap );
}

// -----------------------------------------------------------------------------

BOOL ImplSalDDB::ImplMatches( long nDepth, const SalTwoRect& rTwoRect ) const
{
    BOOL bRet = FALSE;

    if( ( maPixmap != 0 ) && ( ( mnDepth == nDepth ) || ( 1 == mnDepth ) ) )
    {
        if( rTwoRect.mnSrcX == maTwoRect.mnSrcX && rTwoRect.mnSrcY == maTwoRect.mnSrcY &&
            rTwoRect.mnSrcWidth == maTwoRect.mnSrcWidth && rTwoRect.mnSrcHeight == maTwoRect.mnSrcHeight &&
            rTwoRect.mnDestWidth == maTwoRect.mnDestWidth && rTwoRect.mnDestHeight == maTwoRect.mnDestHeight )
        {
            // absolutely indentically
            bRet = TRUE;
        }
        else if( rTwoRect.mnSrcWidth == rTwoRect.mnDestWidth && rTwoRect.mnSrcHeight == rTwoRect.mnDestHeight &&
                 maTwoRect.mnSrcWidth == maTwoRect.mnDestWidth && maTwoRect.mnSrcHeight == maTwoRect.mnDestHeight &&
                 rTwoRect.mnSrcX >= maTwoRect.mnSrcX && rTwoRect.mnSrcY >= maTwoRect.mnSrcY &&
                 ( rTwoRect.mnSrcX + rTwoRect.mnSrcWidth ) <= ( maTwoRect.mnSrcX + maTwoRect.mnSrcWidth ) &&
                 ( rTwoRect.mnSrcY + rTwoRect.mnSrcHeight ) <= ( maTwoRect.mnSrcY + maTwoRect.mnSrcHeight ) )
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void ImplSalDDB::ImplDraw( Drawable aDrawable, long nDrawableDepth, const SalTwoRect& rTwoRect, const GC& rGC ) const
{
    ImplDraw( maPixmap, mnDepth, aDrawable, nDrawableDepth,
              rTwoRect.mnSrcX - maTwoRect.mnSrcX, rTwoRect.mnSrcY - maTwoRect.mnSrcY,
              rTwoRect.mnDestWidth, rTwoRect.mnDestHeight,
              rTwoRect.mnDestX, rTwoRect.mnDestY, rGC );
}

// -----------------------------------------------------------------------------

void ImplSalDDB::ImplDraw( Drawable aSrcDrawable, long nSrcDrawableDepth,
                           Drawable aDstDrawable, long nDstDrawableDepth,
                           long nSrcX, long nSrcY,
                           long nDestWidth, long nDestHeight,
                           long nDestX, long nDestY, const GC& rGC )
{
    SalDisplay* pSalDisp = GetSalData()->GetCurDisp();
    Display*    pXDisp = pSalDisp->GetDisplay();

    if( 1 == nSrcDrawableDepth )
    {
        XCopyPlane( pXDisp, aSrcDrawable, aDstDrawable, rGC,
                    nSrcX, nSrcY, nDestWidth, nDestHeight, nDestX, nDestY, 1 );
    }
    else
    {
        if( nDestWidth > SAL_DRAWPIXMAP_MAX_EXT )
        {
            // !!! Broken XCopyArea
            XCopyArea( pXDisp, aSrcDrawable, aDstDrawable, rGC,
                       nSrcX, nSrcY, nDestWidth, nDestHeight, nDestX, nDestY );
        }
        else
        {
            XCopyArea( pXDisp, aSrcDrawable, aDstDrawable, rGC,
                       nSrcX, nSrcY, nDestWidth, nDestHeight, nDestX, nDestY );
        }
    }
}

// ----------------------
// - ImplSalBitmapCache -
// ----------------------

struct ImplBmpObj
{
    SalBitmap*  mpBmp;
    ULONG       mnMemSize;
    ULONG       mnFlags;

                ImplBmpObj( SalBitmap* pBmp, ULONG nMemSize, ULONG nFlags ) :
                    mpBmp( pBmp ), mnMemSize( nMemSize ), mnFlags( nFlags ) {}
};

// -----------------------------------------------------------------------------

ImplSalBitmapCache::ImplSalBitmapCache() :
    mnTotalSize( 0UL )
{
}

// -----------------------------------------------------------------------------

ImplSalBitmapCache::~ImplSalBitmapCache()
{
    ImplClear();
}

// -----------------------------------------------------------------------------

void ImplSalBitmapCache::ImplAdd( SalBitmap* pBmp, ULONG nMemSize, ULONG nFlags )
{
    ImplBmpObj* pObj;
    BOOL        bFound = FALSE;

    for( pObj = (ImplBmpObj*) maBmpList.Last(); pObj && !bFound; pObj = (ImplBmpObj*) maBmpList.Prev() )
        if( pObj->mpBmp == pBmp )
            bFound = TRUE;

    mnTotalSize += nMemSize;

    if( bFound )
    {
        mnTotalSize -= pObj->mnMemSize;
        pObj->mnMemSize = nMemSize, pObj->mnFlags = nFlags;
    }
    else
        maBmpList.Insert( new ImplBmpObj( pBmp, nMemSize, nFlags ), LIST_APPEND );
}

// -----------------------------------------------------------------------------

void ImplSalBitmapCache::ImplRemove( SalBitmap* pBmp )
{
    for( ImplBmpObj* pObj = (ImplBmpObj*) maBmpList.Last(); pObj; pObj = (ImplBmpObj*) maBmpList.Prev() )
    {
        if( pObj->mpBmp == pBmp )
        {
            maBmpList.Remove( pObj );
            pObj->mpBmp->ImplRemovedFromCache();
            mnTotalSize -= pObj->mnMemSize;
            delete pObj;
            break;
        }
    }
}

// -----------------------------------------------------------------------------

void ImplSalBitmapCache::ImplClear()
{
    for( ImplBmpObj* pObj = (ImplBmpObj*) maBmpList.First(); pObj; pObj = (ImplBmpObj*) maBmpList.Next() )
    {
        pObj->mpBmp->ImplRemovedFromCache();
        delete pObj;
    }

    maBmpList.Clear();
    mnTotalSize = 0;
}
