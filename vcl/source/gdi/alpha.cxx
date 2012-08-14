/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

AlphaMask::AlphaMask( const Bitmap& rBitmap ) :
    Bitmap( rBitmap )
{
    if( !!rBitmap )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );
}

AlphaMask::AlphaMask( const AlphaMask& rAlphaMask ) :
    Bitmap( rAlphaMask )
{
}

AlphaMask::AlphaMask( const Size& rSizePixel, sal_uInt8* pEraseTransparency ) :
    Bitmap( rSizePixel, 8, &Bitmap::GetGreyPalette( 256 ) )
{
    if( pEraseTransparency )
        Bitmap::Erase( Color( *pEraseTransparency, *pEraseTransparency, *pEraseTransparency ) );
}

AlphaMask::~AlphaMask()
{
}

AlphaMask& AlphaMask::operator=( const Bitmap& rBitmap )
{
    *(Bitmap*) this = rBitmap;

    if( !!rBitmap )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return *this;
}

const Bitmap& AlphaMask::ImplGetBitmap() const
{
    return( (const Bitmap&) *this );
}

void AlphaMask::ImplSetBitmap( const Bitmap& rBitmap )
{
    DBG_ASSERT( ( 8 == rBitmap.GetBitCount() ) && rBitmap.HasGreyPalette(), "AlphaMask::ImplSetBitmap: invalid bitmap" );
    *(Bitmap*) this = rBitmap;
}

Bitmap AlphaMask::GetBitmap() const
{
    return ImplGetBitmap();
}

sal_Bool AlphaMask::CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc,
                           const AlphaMask* pAlphaSrc )
{
    // Note: this code is copied from Bitmap::CopyPixel but avoids any palette lookups
    // this optimization is possible because the palettes of AlphaMasks are always identical (8bit GreyPalette, see ctor)

    const Size  aSizePix( GetSizePixel() );
    Rectangle   aRectDst( rRectDst );
    sal_Bool        bRet = sal_False;

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
                    bRet = sal_True;
                }
            }
        }
    }

    return bRet;

}

sal_Bool AlphaMask::Erase( sal_uInt8 cTransparency )
{
    return Bitmap::Erase( Color( cTransparency, cTransparency, cTransparency ) );
}

sal_Bool AlphaMask::Replace( const Bitmap& rMask, sal_uInt8 cReplaceTransparency )
{
    BitmapReadAccess*   pMaskAcc = ( (Bitmap&) rMask ).AcquireReadAccess();
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

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

sal_Bool AlphaMask::Replace( sal_uInt8 cSearchTransparency, sal_uInt8 cReplaceTransparency, sal_uLong
#ifdef DBG_UTIL
nTol
#endif
)
{
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

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

        bRet = sal_True;
    }

    if( pAcc )
        ReleaseAccess( pAcc );

    return bRet;
}

void AlphaMask::ReleaseAccess( BitmapReadAccess* pAccess )
{
    if( pAccess )
    {
        Bitmap::ReleaseAccess( pAccess );
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );
    }
}

sal_Bool AlphaMask::Scale( const Size& rNewSize, sal_uLong nScaleFlag )
{
    sal_Bool bRet = Bitmap::Scale( rNewSize, nScaleFlag );

    if( bRet )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return bRet;
}

sal_Bool AlphaMask::Scale( const double& rScaleX, const double& rScaleY, sal_uLong nScaleFlag )
{
    sal_Bool bRet = Bitmap::Scale( rScaleX, rScaleY, nScaleFlag );

    if( bRet )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return bRet;
}

sal_Bool AlphaMask::ScaleCropRotate(
        const double& rScaleX, const double& rScaleY, const Rectangle& rRectPixel, long nAngle10,
        const Color& rFillColor, sal_uLong  nScaleFlag  )
{
    sal_Bool bRet = Bitmap::ScaleCropRotate( rScaleX, rScaleY, rRectPixel, nAngle10, rFillColor, nScaleFlag );
    if( bRet )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
