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

#include <tools/debug.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/color.hxx>
#include <vcl/alpha.hxx>

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
    // This optimization is possible because the palettes of AlphaMasks are always identical (8bit GreyPalette, see ctor)
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
                        const long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                        const long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
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
                    const long  nWidth = std::min( aRectSrc.GetWidth(), aRectDst.GetWidth() );
                    const long  nHeight = std::min( aRectSrc.GetHeight(), aRectDst.GetHeight() );
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
        const long          nWidth = std::min( pMaskAcc->Width(), pAcc->Width() );
        const long          nHeight = std::min( pMaskAcc->Height(), pAcc->Height() );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
