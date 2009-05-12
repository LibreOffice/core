/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: alpha.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/debug.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/color.hxx>
#include <vcl/alpha.hxx>

// -------------
// - AlphaMask -
// -------------

AlphaMask::AlphaMask()
{
}

// -----------------------------------------------------------------------------

AlphaMask::AlphaMask( const Bitmap& rBitmap ) :
    Bitmap( rBitmap )
{
    if( !!rBitmap )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );
}

// -----------------------------------------------------------------------------

AlphaMask::AlphaMask( const AlphaMask& rAlphaMask ) :
    Bitmap( rAlphaMask )
{
}

// -----------------------------------------------------------------------------

AlphaMask::AlphaMask( const Size& rSizePixel, BYTE* pEraseTransparency ) :
    Bitmap( rSizePixel, 8, &Bitmap::GetGreyPalette( 256 ) )
{
    if( pEraseTransparency )
        Bitmap::Erase( Color( *pEraseTransparency, *pEraseTransparency, *pEraseTransparency ) );
}

// -----------------------------------------------------------------------------

AlphaMask::~AlphaMask()
{
}

// -----------------------------------------------------------------------------

AlphaMask& AlphaMask::operator=( const Bitmap& rBitmap )
{
    *(Bitmap*) this = rBitmap;

    if( !!rBitmap )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return *this;
}

// -----------------------------------------------------------------------------

const Bitmap& AlphaMask::ImplGetBitmap() const
{
    return( (const Bitmap&) *this );
}

// -----------------------------------------------------------------------------

void AlphaMask::ImplSetBitmap( const Bitmap& rBitmap )
{
    DBG_ASSERT( ( 8 == rBitmap.GetBitCount() ) && rBitmap.HasGreyPalette(), "AlphaMask::ImplSetBitmap: invalid bitmap" );
    *(Bitmap*) this = rBitmap;
}

// -----------------------------------------------------------------------------

Bitmap AlphaMask::GetBitmap() const
{
    return ImplGetBitmap();
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Crop( const Rectangle& rRectPixel )
{
    return Bitmap::Crop( rRectPixel );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Expand( ULONG nDX, ULONG nDY, BYTE* pInitTransparency )
{
    Color aColor;

    if( pInitTransparency )
        aColor = Color( *pInitTransparency, *pInitTransparency, *pInitTransparency );

    return Bitmap::Expand( nDX, nDY, pInitTransparency ? &aColor : NULL );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc,
                           const AlphaMask* pAlphaSrc )
{
    // Note: this code is copied from Bitmap::CopyPixel but avoids any palette lookups
    // this optimization is possible because the palettes of AlphaMasks are always identical (8bit GreyPalette, see ctor)

    const Size  aSizePix( GetSizePixel() );
    Rectangle   aRectDst( rRectDst );
    BOOL        bRet = FALSE;

    aRectDst.Intersection( Rectangle( Point(), aSizePix ) );

    if( !aRectDst.IsEmpty() )
    {
        if( pAlphaSrc && ( *pAlphaSrc != *this ) )
        {
            Bitmap*         pSrc = (Bitmap*) pAlphaSrc;
            const Size      aCopySizePix( pSrc->GetSizePixel() );
            Rectangle       aRectSrc( rRectSrc );

            aRectSrc.Intersection( Rectangle( Point(), aCopySizePix ) );

            if( !aRectSrc.IsEmpty() )
            {
                BitmapReadAccess* pReadAcc = pSrc->AcquireReadAccess();

                if( pReadAcc )
                {
                    BitmapWriteAccess* pWriteAcc = AcquireWriteAccess();

                    if( pWriteAcc )
                    {
                        const long  nWidth = Min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                        const long  nHeight = Min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
                        const long  nSrcEndX = aRectSrc.Left() + nWidth;
                        const long  nSrcEndY = aRectSrc.Top() + nHeight;
                        long        nDstY = aRectDst.Top();

                        for( long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
                            for( long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                                pWriteAcc->SetPixel( nDstY, nDstX, pReadAcc->GetPixel( nSrcY, nSrcX ) );

                        ReleaseAccess( pWriteAcc );
                        bRet = ( nWidth > 0L ) && ( nHeight > 0L );
                    }

                    pSrc->ReleaseAccess( pReadAcc );
                }
            }
        }
        else
        {
            Rectangle aRectSrc( rRectSrc );

            aRectSrc.Intersection( Rectangle( Point(), aSizePix ) );

            if( !aRectSrc.IsEmpty() && ( aRectSrc != aRectDst ) )
            {
                BitmapWriteAccess*  pWriteAcc = AcquireWriteAccess();

                if( pWriteAcc )
                {
                    const long  nWidth = Min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                    const long  nHeight = Min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
                    const long  nSrcX = aRectSrc.Left();
                    const long  nSrcY = aRectSrc.Top();
                    const long  nSrcEndX1 = nSrcX + nWidth - 1L;
                    const long  nSrcEndY1 = nSrcY + nHeight - 1L;
                    const long  nDstX = aRectDst.Left();
                    const long  nDstY = aRectDst.Top();
                    const long  nDstEndX1 = nDstX + nWidth - 1L;
                    const long  nDstEndY1 = nDstY + nHeight - 1L;

                    if( ( nDstX <= nSrcX ) && ( nDstY <= nSrcY ) )
                    {
                        for( long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
                            for( long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                                pWriteAcc->SetPixel( nYN, nXN, pWriteAcc->GetPixel( nY, nX ) );
                    }
                    else if( ( nDstX <= nSrcX ) && ( nDstY >= nSrcY ) )
                    {
                        for( long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
                            for( long nX = nSrcX, nXN = nDstX; nX <= nSrcEndX1; nX++, nXN++ )
                                pWriteAcc->SetPixel( nYN, nXN, pWriteAcc->GetPixel( nY, nX ) );
                    }
                    else if( ( nDstX >= nSrcX ) && ( nDstY <= nSrcY ) )
                    {
                        for( long nY = nSrcY, nYN = nDstY; nY <= nSrcEndY1; nY++, nYN++ )
                            for( long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                                pWriteAcc->SetPixel( nYN, nXN, pWriteAcc->GetPixel( nY, nX ) );
                    }
                    else
                    {
                        for( long nY = nSrcEndY1, nYN = nDstEndY1; nY >= nSrcY; nY--, nYN-- )
                            for( long nX = nSrcEndX1, nXN = nDstEndX1; nX >= nSrcX; nX--, nXN-- )
                                pWriteAcc->SetPixel( nYN, nXN, pWriteAcc->GetPixel( nY, nX ) );
                    }

                    ReleaseAccess( pWriteAcc );
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;

}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Erase( BYTE cTransparency )
{
    return Bitmap::Erase( Color( cTransparency, cTransparency, cTransparency ) );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Invert()
{
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    BOOL                bRet = FALSE;

    if( pAcc && pAcc->GetBitCount() == 8 )
    {
        BitmapColor aCol( 0 );
        const long  nWidth = pAcc->Width(), nHeight = pAcc->Height();
        BYTE*       pMap = new BYTE[ 256 ];

        for( long i = 0; i < 256; i++ )
            pMap[ i ] = ~(BYTE) i;

        for( long nY = 0L; nY < nHeight; nY++ )
        {
            for( long nX = 0L; nX < nWidth; nX++ )
            {
                aCol.SetIndex( pMap[ pAcc->GetPixel( nY, nX ).GetIndex() ] );
                pAcc->SetPixel( nY, nX, aCol );
            }
        }

        delete[] pMap;
        bRet = TRUE;
    }

    if( pAcc )
        ReleaseAccess( pAcc );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Mirror( ULONG nMirrorFlags )
{
    return Bitmap::Mirror( nMirrorFlags );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Scale( const Size& rNewSize, ULONG nScaleFlag )
{
    BOOL bRet = Bitmap::Scale( rNewSize, nScaleFlag );

    if( bRet && ( nScaleFlag == BMP_SCALE_INTERPOLATE ) )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Scale( const double& rScaleX, const double& rScaleY, ULONG nScaleFlag )
{
    BOOL bRet = Bitmap::Scale( rScaleX, rScaleY, nScaleFlag );

    if( bRet && ( nScaleFlag == BMP_SCALE_INTERPOLATE ) )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Rotate( long nAngle10, BYTE cFillTransparency )
{
    return Bitmap::Rotate( nAngle10, Color( cFillTransparency, cFillTransparency, cFillTransparency ) );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Replace( const Bitmap& rMask, BYTE cReplaceTransparency )
{
    BitmapReadAccess*   pMaskAcc = ( (Bitmap&) rMask ).AcquireReadAccess();
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    BOOL                bRet = FALSE;

    if( pMaskAcc && pAcc )
    {
        const BitmapColor   aReplace( cReplaceTransparency );
        const long          nWidth = Min( pMaskAcc->Width(), pAcc->Width() );
        const long          nHeight = Min( pMaskAcc->Height(), pAcc->Height() );
        const BitmapColor   aMaskWhite( pMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

        for( long nY = 0L; nY < nHeight; nY++ )
            for( long nX = 0L; nX < nWidth; nX++ )
                if( pMaskAcc->GetPixel( nY, nX ) == aMaskWhite )
                    pAcc->SetPixel( nY, nX, aReplace );
    }

    ( (Bitmap&) rMask ).ReleaseAccess( pMaskAcc );
    ReleaseAccess( pAcc );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Replace( BYTE cSearchTransparency, BYTE cReplaceTransparency, ULONG
#ifdef DBG_UTIL
nTol
#endif
)
{
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    BOOL                bRet = FALSE;

    DBG_ASSERT( !nTol, "AlphaMask::Replace: nTol not used yet" );

    if( pAcc && pAcc->GetBitCount() == 8 )
    {
        const long nWidth = pAcc->Width(), nHeight = pAcc->Height();

        if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
        {
            for( long nY = 0L; nY < nHeight; nY++ )
            {
                Scanline pScan = pAcc->GetScanline( nY );

                for( long nX = 0L; nX < nWidth; nX++, pScan++ )
                {
                    if( *pScan == cSearchTransparency )
                        *pScan = cReplaceTransparency;
                }
            }
        }
        else
        {
            BitmapColor aReplace( cReplaceTransparency );

            for( long nY = 0L; nY < nHeight; nY++ )
            {
                for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pAcc->GetPixel( nY, nX ).GetIndex() == cSearchTransparency )
                        pAcc->SetPixel( nY, nX, aReplace );
                }
            }
        }

        bRet = TRUE;
    }

    if( pAcc )
        ReleaseAccess( pAcc );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Replace( BYTE* pSearchTransparencies, BYTE* pReplaceTransparencies,
                         ULONG nColorCount, ULONG* pTols )
{
    Color*  pSearchColors = new Color[ nColorCount ];
    Color*  pReplaceColors = new Color[ nColorCount ];
    BOOL    bRet;

    for( ULONG i = 0; i < nColorCount; i++ )
    {
        const BYTE cSearchTransparency = pSearchTransparencies[ i ];
        const BYTE cReplaceTransparency = pReplaceTransparencies[ i ];

        pSearchColors[ i ] = Color( cSearchTransparency, cSearchTransparency, cSearchTransparency );
        pReplaceColors[ i ] = Color( cReplaceTransparency, cReplaceTransparency, cReplaceTransparency );
    }

    bRet = Bitmap::Replace( pSearchColors, pReplaceColors, nColorCount, pTols ) &&
           Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    delete[] pSearchColors;
    delete[] pReplaceColors;

    return bRet;
}

// -----------------------------------------------------------------------------

void AlphaMask::ReleaseAccess( BitmapReadAccess* pAccess )
{
    if( pAccess )
    {
        Bitmap::ReleaseAccess( pAccess );
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );
    }
}
