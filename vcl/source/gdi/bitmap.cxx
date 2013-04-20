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


#include <rtl/crc.h>
#include <tools/stream.hxx>
#include <tools/poly.hxx>
#include <tools/rc.h>
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>

#include <impbmp.hxx>
#include <salbmp.hxx>

Bitmap::Bitmap() :
    mpImpBmp( NULL )
{
}

Bitmap::Bitmap( const ResId& rResId ) :
    mpImpBmp( NULL )
{
    const BitmapEx aBmpEx( rResId );

    if( !aBmpEx.IsEmpty() )
        *this = aBmpEx.GetBitmap();
}

Bitmap::Bitmap( const Bitmap& rBitmap ) :
    maPrefMapMode   ( rBitmap.maPrefMapMode ),
    maPrefSize      ( rBitmap.maPrefSize )
{
    mpImpBmp = rBitmap.mpImpBmp;

    if ( mpImpBmp )
        mpImpBmp->ImplIncRefCount();
}

Bitmap::Bitmap( SalBitmap* pSalBitmap )
{
    mpImpBmp = new ImpBitmap();
    mpImpBmp->ImplSetSalBitmap( pSalBitmap );
    maPrefMapMode = MapMode( MAP_PIXEL );
    maPrefSize = mpImpBmp->ImplGetSize();
}

Bitmap::Bitmap( const Size& rSizePixel, sal_uInt16 nBitCount, const BitmapPalette* pPal )
{
    if( rSizePixel.Width() && rSizePixel.Height() )
    {
        BitmapPalette   aPal;
        BitmapPalette*  pRealPal = NULL;

        if( nBitCount <= 8 )
        {
            if( !pPal )
            {
                if( 1 == nBitCount )
                {
                    aPal.SetEntryCount( 2 );
                    aPal[ 0 ] = Color( COL_BLACK );
                    aPal[ 1 ] = Color( COL_WHITE );
                }
                else if( ( 4 == nBitCount ) || ( 8 == nBitCount ) )
                {
                    aPal.SetEntryCount( 1 << nBitCount );
                    aPal[ 0 ] = Color( COL_BLACK );
                    aPal[ 1 ] = Color( COL_BLUE );
                    aPal[ 2 ] = Color( COL_GREEN );
                    aPal[ 3 ] = Color( COL_CYAN );
                    aPal[ 4 ] = Color( COL_RED );
                    aPal[ 5 ] = Color( COL_MAGENTA );
                    aPal[ 6 ] = Color( COL_BROWN );
                    aPal[ 7 ] = Color( COL_GRAY );
                    aPal[ 8 ] = Color( COL_LIGHTGRAY );
                    aPal[ 9 ] = Color( COL_LIGHTBLUE );
                    aPal[ 10 ] = Color( COL_LIGHTGREEN );
                    aPal[ 11 ] = Color( COL_LIGHTCYAN );
                    aPal[ 12 ] = Color( COL_LIGHTRED );
                    aPal[ 13 ] = Color( COL_LIGHTMAGENTA );
                    aPal[ 14 ] = Color( COL_YELLOW );
                    aPal[ 15 ] = Color( COL_WHITE );

                    // Create dither palette
                    if( 8 == nBitCount )
                    {
                        sal_uInt16 nActCol = 16;

                        for( sal_uInt16 nB = 0; nB < 256; nB += 51 )
                            for( sal_uInt16 nG = 0; nG < 256; nG += 51 )
                                for( sal_uInt16 nR = 0; nR < 256; nR += 51 )
                                    aPal[ nActCol++ ] = BitmapColor( (sal_uInt8) nR, (sal_uInt8) nG, (sal_uInt8) nB );

                        // Set standard Office colors
                        aPal[ nActCol++ ] = BitmapColor( 0, 184, 255 );
                    }
                }
            }
            else
                pRealPal = (BitmapPalette*) pPal;
        }

        mpImpBmp = new ImpBitmap;
        mpImpBmp->ImplCreate( rSizePixel, nBitCount, pRealPal ? *pRealPal : aPal );
    }
    else
        mpImpBmp = NULL;
}

Bitmap::~Bitmap()
{
    ImplReleaseRef();
}

const BitmapPalette& Bitmap::GetGreyPalette( int nEntries )
{
    static BitmapPalette aGreyPalette2;
    static BitmapPalette aGreyPalette4;
    static BitmapPalette aGreyPalette16;
    static BitmapPalette aGreyPalette256;

    // Create greyscale palette with 2, 4, 16 or 256 entries
    if( 2 == nEntries || 4 == nEntries || 16 == nEntries || 256 == nEntries )
    {
        if( 2 == nEntries )
        {
            if( !aGreyPalette2.GetEntryCount() )
            {
                aGreyPalette2.SetEntryCount( 2 );
                aGreyPalette2[ 0 ] = BitmapColor( 0, 0, 0 );
                aGreyPalette2[ 1 ] = BitmapColor( 255, 255, 255 );
            }

            return aGreyPalette2;
        }
        else if( 4 == nEntries )
        {
            if( !aGreyPalette4.GetEntryCount() )
            {
                aGreyPalette4.SetEntryCount( 4 );
                aGreyPalette4[ 0 ] = BitmapColor( 0, 0, 0 );
                aGreyPalette4[ 1 ] = BitmapColor( 85, 85, 85 );
                aGreyPalette4[ 2 ] = BitmapColor( 170, 170, 170 );
                aGreyPalette4[ 3 ] = BitmapColor( 255, 255, 255 );
            }

            return aGreyPalette4;
        }
        else if( 16 == nEntries )
        {
            if( !aGreyPalette16.GetEntryCount() )
            {
                sal_uInt8 cGrey = 0, cGreyInc = 17;

                aGreyPalette16.SetEntryCount( 16 );

                for( sal_uInt16 i = 0; i < 16; i++, cGrey = sal::static_int_cast<sal_uInt8>(cGrey + cGreyInc) )
                    aGreyPalette16[ i ] = BitmapColor( cGrey, cGrey, cGrey );
            }

            return aGreyPalette16;
        }
        else
        {
            if( !aGreyPalette256.GetEntryCount() )
            {
                aGreyPalette256.SetEntryCount( 256 );

                for( sal_uInt16 i = 0; i < 256; i++ )
                    aGreyPalette256[ i ] = BitmapColor( (sal_uInt8) i, (sal_uInt8) i, (sal_uInt8) i );
            }

            return aGreyPalette256;
        }
    }
    else
    {
        OSL_FAIL( "Bitmap::GetGreyPalette: invalid entry count (2/4/16/256 allowed)" );
        return aGreyPalette2;
    }
}

bool BitmapPalette::IsGreyPalette() const
{
    const int nEntryCount = GetEntryCount();
    if( !nEntryCount ) // NOTE: an empty palette means 1:1 mapping
        return true;
    // See above: only certain entry values will result in a valid call to GetGreyPalette
    if( nEntryCount == 2 || nEntryCount == 4 || nEntryCount == 16 || nEntryCount == 256 )
    {
        const BitmapPalette& rGreyPalette = Bitmap::GetGreyPalette( nEntryCount );
        if( rGreyPalette == *this )
            return true;
    }

    bool bRet = false;
    // TODO: is it worth to compare the entries for the general case?
    if (nEntryCount == 2)
    {
       const BitmapColor& rCol0(mpBitmapColor[0]);
       const BitmapColor& rCol1(mpBitmapColor[1]);
       bRet = rCol0.GetRed() == rCol0.GetGreen() && rCol0.GetRed() == rCol0.GetBlue() &&
              rCol1.GetRed() == rCol1.GetGreen() && rCol1.GetRed() == rCol1.GetBlue();
    }
    return bRet;
}

Bitmap& Bitmap::operator=( const Bitmap& rBitmap )
{
    maPrefSize = rBitmap.maPrefSize;
    maPrefMapMode = rBitmap.maPrefMapMode;

    if ( rBitmap.mpImpBmp )
        rBitmap.mpImpBmp->ImplIncRefCount();

    ImplReleaseRef();
    mpImpBmp = rBitmap.mpImpBmp;

    return *this;
}

sal_Bool Bitmap::IsEqual( const Bitmap& rBmp ) const
{
    return( IsSameInstance( rBmp ) ||
            ( rBmp.GetSizePixel() == GetSizePixel() &&
              rBmp.GetBitCount() == GetBitCount() &&
              rBmp.GetChecksum() == GetChecksum() ) );
}

void Bitmap::SetEmpty()
{
    maPrefMapMode = MapMode();
    maPrefSize = Size();

    ImplReleaseRef();
    mpImpBmp = NULL;
}

Size Bitmap::GetSizePixel() const
{
    return( mpImpBmp ? mpImpBmp->ImplGetSize() : Size() );
}

void Bitmap::SetSourceSizePixel( const Size& rSize)
{
    if( mpImpBmp )
        mpImpBmp->ImplSetSourceSize( rSize);
}

sal_uInt16 Bitmap::GetBitCount() const
{
    return( mpImpBmp ? mpImpBmp->ImplGetBitCount() : 0 );
}

sal_Bool Bitmap::HasGreyPalette() const
{
    const sal_uInt16    nBitCount = GetBitCount();
    sal_Bool            bRet = nBitCount == 1 ? sal_True : sal_False;

    BitmapReadAccess* pRAcc = ( (Bitmap*) this )->AcquireReadAccess();

    if( pRAcc )
    {
        bRet = pRAcc->HasPalette() && pRAcc->GetPalette().IsGreyPalette();
        ( (Bitmap*) this )->ReleaseAccess( pRAcc );
    }

    return bRet;
}

sal_uLong Bitmap::GetChecksum() const
{
    sal_uLong nRet = 0UL;

    if( mpImpBmp )
    {
        nRet = mpImpBmp->ImplGetChecksum();

        if( !nRet )
        {
            BitmapReadAccess* pRAcc = ( (Bitmap*) this )->AcquireReadAccess();

            if( pRAcc && pRAcc->Width() && pRAcc->Height() )
            {
                sal_uInt32  nCrc = 0;
                SVBT32      aBT32;

                pRAcc->ImplZeroInitUnusedBits();

                UInt32ToSVBT32( pRAcc->Width(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pRAcc->Height(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pRAcc->GetBitCount(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pRAcc->GetColorMask().GetRedMask(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pRAcc->GetColorMask().GetGreenMask(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( pRAcc->GetColorMask().GetBlueMask(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                if( pRAcc->HasPalette() )
                {
                    nCrc = rtl_crc32( nCrc, pRAcc->GetPalette().ImplGetColorBuffer(),
                                      pRAcc->GetPaletteEntryCount() * sizeof( BitmapColor ) );
                }

                nCrc = rtl_crc32( nCrc, pRAcc->GetBuffer(), pRAcc->GetScanlineSize() * pRAcc->Height() );

                mpImpBmp->ImplSetChecksum( nRet = nCrc );
            }

            if (pRAcc) ( (Bitmap*) this )->ReleaseAccess( pRAcc );
        }
    }

    return nRet;
}

void Bitmap::ImplReleaseRef()
{
    if( mpImpBmp )
    {
        if( mpImpBmp->ImplGetRefCount() > 1UL )
            mpImpBmp->ImplDecRefCount();
        else
        {
            delete mpImpBmp;
            mpImpBmp = NULL;
        }
    }
}

void Bitmap::ImplMakeUnique()
{
    if( mpImpBmp && mpImpBmp->ImplGetRefCount() > 1UL )
    {
        ImpBitmap* pOldImpBmp = mpImpBmp;

        pOldImpBmp->ImplDecRefCount();

        mpImpBmp = new ImpBitmap;
        mpImpBmp->ImplCreate( *pOldImpBmp );
    }
}

void Bitmap::ImplAssignWithSize( const Bitmap& rBitmap )
{
    const Size      aOldSizePix( GetSizePixel() );
    const Size      aNewSizePix( rBitmap.GetSizePixel() );
    const MapMode   aOldMapMode( maPrefMapMode );
    Size            aNewPrefSize;

    if( ( aOldSizePix != aNewSizePix ) && aOldSizePix.Width() && aOldSizePix.Height() )
    {
        aNewPrefSize.Width() = FRound( maPrefSize.Width() * aNewSizePix.Width() / aOldSizePix.Width() );
        aNewPrefSize.Height() = FRound( maPrefSize.Height() * aNewSizePix.Height() / aOldSizePix.Height() );
    }
    else
        aNewPrefSize = maPrefSize;

    *this = rBitmap;

    maPrefSize = aNewPrefSize;
    maPrefMapMode = aOldMapMode;
}

ImpBitmap* Bitmap::ImplGetImpBitmap() const
{
    return mpImpBmp;
}

void Bitmap::ImplSetImpBitmap( ImpBitmap* pImpBmp )
{
    if( pImpBmp != mpImpBmp )
    {
        ImplReleaseRef();
        mpImpBmp = pImpBmp;
    }
}

BitmapReadAccess* Bitmap::AcquireReadAccess()
{
    BitmapReadAccess* pReadAccess = new BitmapReadAccess( *this );

    if( !*pReadAccess )
    {
        delete pReadAccess;
        pReadAccess = NULL;
    }

    return pReadAccess;
}

BitmapWriteAccess* Bitmap::AcquireWriteAccess()
{
    BitmapWriteAccess* pWriteAccess = new BitmapWriteAccess( *this );

    if( !*pWriteAccess )
    {
        delete pWriteAccess;
        pWriteAccess = NULL;
    }

    return pWriteAccess;
}

void Bitmap::ReleaseAccess( BitmapReadAccess* pBitmapAccess )
{
    delete pBitmapAccess;
}

sal_Bool Bitmap::Erase( const Color& rFillColor )
{
    if( !(*this) )
        return sal_True;

    BitmapWriteAccess*  pWriteAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pWriteAcc )
    {
        const sal_uLong nFormat = pWriteAcc->GetScanlineFormat();
        sal_uInt8       cIndex = 0;
        sal_Bool        bFast = sal_False;

        switch( nFormat )
        {
            case( BMP_FORMAT_1BIT_MSB_PAL ):
            case( BMP_FORMAT_1BIT_LSB_PAL ):
            {
                cIndex = (sal_uInt8) pWriteAcc->GetBestPaletteIndex( rFillColor );
                cIndex = ( cIndex ? 255 : 0 );
                bFast = sal_True;
            }
            break;

            case( BMP_FORMAT_4BIT_MSN_PAL ):
            case( BMP_FORMAT_4BIT_LSN_PAL ):
            {
                cIndex = (sal_uInt8) pWriteAcc->GetBestPaletteIndex( rFillColor );
                cIndex = cIndex | ( cIndex << 4 );
                bFast = sal_True;
            }
            break;

            case( BMP_FORMAT_8BIT_PAL ):
            {
                cIndex = (sal_uInt8) pWriteAcc->GetBestPaletteIndex( rFillColor );
                bFast = sal_True;
            }
            break;

            case( BMP_FORMAT_24BIT_TC_BGR ):
            case( BMP_FORMAT_24BIT_TC_RGB ):
            {
                if( ( rFillColor.GetRed() == rFillColor.GetGreen() ) &&
                    ( rFillColor.GetRed() == rFillColor.GetBlue() ) )
                {
                    cIndex = rFillColor.GetRed();
                    bFast = sal_True;
                }
                else
                    bFast = sal_False;
            }
            break;

            default:
                bFast = sal_False;
            break;
        }

        if( bFast )
        {
            const sal_uLong nBufSize = pWriteAcc->GetScanlineSize() * pWriteAcc->Height();
            memset( pWriteAcc->GetBuffer(), cIndex, nBufSize );
        }
        else
        {
            Point aTmpPoint;
            const Rectangle aRect( aTmpPoint, Size( pWriteAcc->Width(), pWriteAcc->Height() ) );
            pWriteAcc->SetFillColor( rFillColor );
            pWriteAcc->FillRect( aRect );
        }

        ReleaseAccess( pWriteAcc );
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool Bitmap::Invert()
{
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pAcc )
    {
        if( pAcc->HasPalette() )
        {
            BitmapPalette   aBmpPal( pAcc->GetPalette() );
            const sal_uInt16    nCount = aBmpPal.GetEntryCount();

            for( sal_uInt16 i = 0; i < nCount; i++ )
                aBmpPal[ i ].Invert();

            pAcc->SetPalette( aBmpPal );
        }
        else
        {
            const long  nWidth = pAcc->Width();
            const long  nHeight = pAcc->Height();

            for( long nX = 0L; nX < nWidth; nX++ )
                for( long nY = 0L; nY < nHeight; nY++ )
                    pAcc->SetPixel( nY, nX, pAcc->GetPixel( nY, nX ).Invert() );
        }

        ReleaseAccess( pAcc );
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool Bitmap::Mirror( sal_uLong nMirrorFlags )
{
    sal_Bool bHorz = ( ( nMirrorFlags & BMP_MIRROR_HORZ ) == BMP_MIRROR_HORZ );
    sal_Bool bVert = ( ( nMirrorFlags & BMP_MIRROR_VERT ) == BMP_MIRROR_VERT );
    sal_Bool bRet = sal_False;

    if( bHorz && !bVert )
    {
        BitmapWriteAccess*  pAcc = AcquireWriteAccess();

        if( pAcc )
        {
            const long  nWidth = pAcc->Width();
            const long  nHeight = pAcc->Height();
            const long  nWidth1 = nWidth - 1L;
            const long  nWidth_2 = nWidth >> 1L;

            for( long nY = 0L; nY < nHeight; nY++ )
            {
                for( long nX = 0L, nOther = nWidth1; nX < nWidth_2; nX++, nOther-- )
                {
                    const BitmapColor aTemp( pAcc->GetPixel( nY, nX ) );

                    pAcc->SetPixel( nY, nX, pAcc->GetPixel( nY, nOther ) );
                    pAcc->SetPixel( nY, nOther, aTemp );
                }
            }

            ReleaseAccess( pAcc );
            bRet = sal_True;
        }
    }
    else if( bVert && !bHorz )
    {
        BitmapWriteAccess*  pAcc = AcquireWriteAccess();

        if( pAcc )
        {
            const long  nScanSize = pAcc->GetScanlineSize();
            sal_uInt8*      pBuffer = new sal_uInt8[ nScanSize ];
            const long  nHeight = pAcc->Height();
            const long  nHeight1 = nHeight - 1L;
            const long  nHeight_2 = nHeight >> 1L;

            for( long nY = 0L, nOther = nHeight1; nY < nHeight_2; nY++, nOther-- )
            {
                memcpy( pBuffer, pAcc->GetScanline( nY ), nScanSize );
                memcpy( pAcc->GetScanline( nY ), pAcc->GetScanline( nOther ), nScanSize );
                memcpy( pAcc->GetScanline( nOther ), pBuffer, nScanSize );
            }

            delete[] pBuffer;
            ReleaseAccess( pAcc );
            bRet = sal_True;
        }
    }
    else if( bHorz && bVert )
    {
        BitmapWriteAccess*  pAcc = AcquireWriteAccess();

        if( pAcc )
        {
            const long  nWidth = pAcc->Width();
            const long  nWidth1 = nWidth - 1L;
            const long  nHeight = pAcc->Height();
            long        nHeight_2 = nHeight >> 1;

            for( long nY = 0L, nOtherY = nHeight - 1L; nY < nHeight_2; nY++, nOtherY-- )
            {
                for( long nX = 0L, nOtherX = nWidth1; nX < nWidth; nX++, nOtherX-- )
                {
                    const BitmapColor aTemp( pAcc->GetPixel( nY, nX ) );

                    pAcc->SetPixel( nY, nX, pAcc->GetPixel( nOtherY, nOtherX ) );
                    pAcc->SetPixel( nOtherY, nOtherX, aTemp );
                }
            }

            // ggf. noch mittlere Zeile horizontal spiegeln
            if( nHeight & 1 )
            {
                for( long nX = 0L, nOtherX = nWidth1, nWidth_2 = nWidth >> 1; nX < nWidth_2; nX++, nOtherX-- )
                {
                    const BitmapColor aTemp( pAcc->GetPixel( nHeight_2, nX ) );
                    pAcc->SetPixel( nHeight_2, nX, pAcc->GetPixel( nHeight_2, nOtherX ) );
                    pAcc->SetPixel( nHeight_2, nOtherX, aTemp );
                }
            }

            ReleaseAccess( pAcc );
            bRet = sal_True;
        }
    }
    else
        bRet = sal_True;

    return bRet;
}

sal_Bool Bitmap::Rotate( long nAngle10, const Color& rFillColor )
{
    sal_Bool bRet = sal_False;

    nAngle10 %= 3600L;
    nAngle10 = ( nAngle10 < 0L ) ? ( 3599L + nAngle10 ) : nAngle10;

    if( !nAngle10    )
        bRet = sal_True;
    else if( 1800L == nAngle10 )
        bRet = Mirror( BMP_MIRROR_HORZ | BMP_MIRROR_VERT );
    else
    {
        BitmapReadAccess*   pReadAcc = AcquireReadAccess();
        Bitmap              aRotatedBmp;

        if( pReadAcc )
        {
            const Size  aSizePix( GetSizePixel() );

            if( ( 900L == nAngle10 ) || ( 2700L == nAngle10 ) )
            {
                const Size          aNewSizePix( aSizePix.Height(), aSizePix.Width() );
                Bitmap              aNewBmp( aNewSizePix, GetBitCount(), &pReadAcc->GetPalette() );
                BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

                if( pWriteAcc )
                {
                    const long  nWidth = aSizePix.Width();
                    const long  nWidth1 = nWidth - 1L;
                    const long  nHeight = aSizePix.Height();
                    const long  nHeight1 = nHeight - 1L;
                    const long  nNewWidth = aNewSizePix.Width();
                    const long  nNewHeight = aNewSizePix.Height();

                    if( 900L == nAngle10 )
                    {
                        for( long nY = 0L, nOtherX = nWidth1; nY < nNewHeight; nY++, nOtherX-- )
                            for( long nX = 0L, nOtherY = 0L; nX < nNewWidth; nX++ )
                                pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nOtherY++, nOtherX ) );
                    }
                    else if( 2700L == nAngle10 )
                    {
                        for( long nY = 0L, nOtherX = 0L; nY < nNewHeight; nY++, nOtherX++ )
                            for( long nX = 0L, nOtherY = nHeight1; nX < nNewWidth; nX++ )
                                pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nOtherY--, nOtherX ) );
                    }

                    aNewBmp.ReleaseAccess( pWriteAcc );
                }

                aRotatedBmp = aNewBmp;
            }
            else
            {
                Point       aTmpPoint;
                Rectangle   aTmpRectangle( aTmpPoint, aSizePix );
                Polygon     aPoly( aTmpRectangle );
                aPoly.Rotate( aTmpPoint, (sal_uInt16) nAngle10 );

                Rectangle           aNewBound( aPoly.GetBoundRect() );
                const Size          aNewSizePix( aNewBound.GetSize() );
                Bitmap              aNewBmp( aNewSizePix, GetBitCount(), &pReadAcc->GetPalette() );
                BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

                if( pWriteAcc )
                {
                    const BitmapColor   aFillColor( pWriteAcc->GetBestMatchingColor( rFillColor ) );
                    const double        fCosAngle = cos( nAngle10 * F_PI1800 );
                    const double        fSinAngle = sin( nAngle10 * F_PI1800 );
                    const double        fXMin = aNewBound.Left();
                    const double        fYMin = aNewBound.Top();
                    const long          nWidth = aSizePix.Width();
                    const long          nHeight = aSizePix.Height();
                    const long          nNewWidth = aNewSizePix.Width();
                    const long          nNewHeight = aNewSizePix.Height();
                    long                nX;
                    long                nY;
                    long                nRotX;
                    long                nRotY;
                    long                nSinY;
                    long                nCosY;
                    long*               pCosX = new long[ nNewWidth ];
                    long*               pSinX = new long[ nNewWidth ];
                    long*               pCosY = new long[ nNewHeight ];
                    long*               pSinY = new long[ nNewHeight ];

                    for ( nX = 0; nX < nNewWidth; nX++ )
                    {
                        const double fTmp = ( fXMin + nX ) * 64.;

                        pCosX[ nX ] = FRound( fCosAngle * fTmp );
                        pSinX[ nX ] = FRound( fSinAngle * fTmp );
                    }

                    for ( nY = 0; nY < nNewHeight; nY++ )
                    {
                        const double fTmp = ( fYMin + nY ) * 64.;

                        pCosY[ nY ] = FRound( fCosAngle * fTmp );
                        pSinY[ nY ] = FRound( fSinAngle * fTmp );
                    }

                    for( nY = 0L; nY < nNewHeight; nY++ )
                    {
                        nSinY = pSinY[ nY ];
                        nCosY = pCosY[ nY ];

                        for( nX = 0L; nX < nNewWidth; nX++ )
                        {
                            nRotX = ( pCosX[ nX ] - nSinY ) >> 6;
                            nRotY = ( pSinX[ nX ] + nCosY ) >> 6;

                            if ( ( nRotX > -1L ) && ( nRotX < nWidth ) && ( nRotY > -1L ) && ( nRotY < nHeight ) )
                                pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nRotY, nRotX ) );
                            else
                                pWriteAcc->SetPixel( nY, nX, aFillColor );
                        }
                    }

                    delete[] pSinX;
                    delete[] pCosX;
                    delete[] pSinY;
                    delete[] pCosY;

                    aNewBmp.ReleaseAccess( pWriteAcc );
                }

                aRotatedBmp = aNewBmp;
            }

            ReleaseAccess( pReadAcc );
        }

        if( ( bRet = !!aRotatedBmp ) == sal_True )
            ImplAssignWithSize( aRotatedBmp );
    }

    return bRet;
};

sal_Bool Bitmap::Crop( const Rectangle& rRectPixel )
{
    const Size          aSizePix( GetSizePixel() );
    Rectangle           aRect( rRectPixel );
    sal_Bool                bRet = sal_False;

    aRect.Intersection( Rectangle( Point(), aSizePix ) );

    if( !aRect.IsEmpty() )
    {
        BitmapReadAccess* pReadAcc = AcquireReadAccess();

        if( pReadAcc )
        {
            Point               aTmpPoint;
            const Rectangle     aNewRect( aTmpPoint, aRect.GetSize() );
            Bitmap              aNewBmp( aNewRect.GetSize(), GetBitCount(), &pReadAcc->GetPalette() );
            BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pWriteAcc )
            {
                const long nOldX = aRect.Left();
                const long nOldY = aRect.Top();
                const long nNewWidth = aNewRect.GetWidth();
                const long nNewHeight = aNewRect.GetHeight();

                for( long nY = 0, nY2 = nOldY; nY < nNewHeight; nY++, nY2++ )
                    for( long nX = 0, nX2 = nOldX; nX < nNewWidth; nX++, nX2++ )
                        pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nY2, nX2 ) );

                aNewBmp.ReleaseAccess( pWriteAcc );
                bRet = sal_True;
            }

            ReleaseAccess( pReadAcc );

            if( bRet )
                ImplAssignWithSize( aNewBmp );
        }
    }

    return bRet;
};

sal_Bool Bitmap::CopyPixel( const Rectangle& rRectDst,
                        const Rectangle& rRectSrc, const Bitmap* pBmpSrc )
{
    const Size  aSizePix( GetSizePixel() );
    Rectangle   aRectDst( rRectDst );
    sal_Bool        bRet = sal_False;

    aRectDst.Intersection( Rectangle( Point(), aSizePix ) );

    if( !aRectDst.IsEmpty() )
    {
        if( pBmpSrc && ( *pBmpSrc != *this ) )
        {
            Bitmap*         pSrc = (Bitmap*) pBmpSrc;
            const Size      aCopySizePix( pSrc->GetSizePixel() );
            Rectangle       aRectSrc( rRectSrc );
            const sal_uInt16    nSrcBitCount = pBmpSrc->GetBitCount();
            const sal_uInt16    nDstBitCount = GetBitCount();

            if( nSrcBitCount > nDstBitCount )
            {
                long nNextIndex = 0L;

                if( ( nSrcBitCount == 24 ) && ( nDstBitCount < 24 ) )
                    Convert( BMP_CONVERSION_24BIT );
                else if( ( nSrcBitCount == 8 ) && ( nDstBitCount < 8 ) )
                {
                    Convert( BMP_CONVERSION_8BIT_COLORS );
                    nNextIndex = 16;
                }
                else if( ( nSrcBitCount == 4 ) && ( nDstBitCount < 4 ) )
                {
                    Convert( BMP_CONVERSION_4BIT_COLORS );
                    nNextIndex = 2;
                }

                if( nNextIndex )
                {
                    BitmapReadAccess*   pSrcAcc = pSrc->AcquireReadAccess();
                    BitmapWriteAccess*  pDstAcc = AcquireWriteAccess();

                    if( pSrcAcc && pDstAcc )
                    {
                        const long      nSrcCount = pDstAcc->GetPaletteEntryCount();
                        const long      nDstCount = 1 << nDstBitCount;
                        sal_Bool            bFound;

                        for( long i = 0L; ( i < nSrcCount ) && ( nNextIndex < nSrcCount ); i++ )
                        {
                            const BitmapColor& rSrcCol = pSrcAcc->GetPaletteColor( (sal_uInt16) i );

                            bFound = sal_False;

                            for( long j = 0L; j < nDstCount; j++ )
                            {
                                if( rSrcCol == pDstAcc->GetPaletteColor( (sal_uInt16) j ) )
                                {
                                    bFound = sal_True;
                                    break;
                                }
                            }

                            if( !bFound )
                                pDstAcc->SetPaletteColor( (sal_uInt16) nNextIndex++, rSrcCol );
                        }
                    }

                    if( pSrcAcc )
                        pSrc->ReleaseAccess( pSrcAcc );

                    if( pDstAcc )
                        ReleaseAccess( pDstAcc );
                }
            }

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

                        if( pReadAcc->HasPalette() && pWriteAcc->HasPalette() )
                        {
                            const sal_uInt16    nCount = pReadAcc->GetPaletteEntryCount();
                            sal_uInt8*          pMap = new sal_uInt8[ nCount ];

                            // Create index map for the color table, as the bitmap should be copied
                            // retaining it's color information relatively well
                            for( sal_uInt16 i = 0; i < nCount; i++ )
                                pMap[ i ] = (sal_uInt8) pWriteAcc->GetBestPaletteIndex( pReadAcc->GetPaletteColor( i ) );

                            for( long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
                                for( long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                                    pWriteAcc->SetPixel( nDstY, nDstX, pMap[ pReadAcc->GetPixel( nSrcY, nSrcX ).GetIndex() ] );

                            delete[] pMap;
                        }
                        else if( pReadAcc->HasPalette() )
                        {
                            for( long nSrcY = aRectSrc.Top(); nSrcY < nSrcEndY; nSrcY++, nDstY++ )
                                for( long nSrcX = aRectSrc.Left(), nDstX = aRectDst.Left(); nSrcX < nSrcEndX; nSrcX++, nDstX++ )
                                    pWriteAcc->SetPixel( nDstY, nDstX, pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nSrcY, nSrcX ) ) );
                        }
                        else
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

sal_Bool Bitmap::Expand( sal_uLong nDX, sal_uLong nDY, const Color* pInitColor )
{
    sal_Bool bRet = sal_False;

    if( nDX || nDY )
    {
        const Size          aSizePixel( GetSizePixel() );
        const long          nWidth = aSizePixel.Width();
        const long          nHeight = aSizePixel.Height();
        const Size          aNewSize( nWidth + nDX, nHeight + nDY );
        BitmapReadAccess*   pReadAcc = AcquireReadAccess();

        if( pReadAcc )
        {
            BitmapPalette       aBmpPal( pReadAcc->GetPalette() );
            Bitmap              aNewBmp( aNewSize, GetBitCount(), &aBmpPal );
            BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pWriteAcc )
            {
                BitmapColor aColor;
                const long  nNewX = nWidth;
                const long  nNewY = nHeight;
                const long  nNewWidth = pWriteAcc->Width();
                const long  nNewHeight = pWriteAcc->Height();
                long        nX;
                long        nY;

                if( pInitColor )
                    aColor = pWriteAcc->GetBestMatchingColor( *pInitColor );

                for( nY = 0L; nY < nHeight; nY++ )
                {
                    pWriteAcc->CopyScanline( nY, *pReadAcc );

                    if( pInitColor && nDX )
                        for( nX = nNewX; nX < nNewWidth; nX++ )
                            pWriteAcc->SetPixel( nY, nX, aColor );
                }

                if( pInitColor && nDY )
                    for( nY = nNewY; nY < nNewHeight; nY++ )
                        for( nX = 0; nX < nNewWidth; nX++ )
                            pWriteAcc->SetPixel( nY, nX, aColor );

                aNewBmp.ReleaseAccess( pWriteAcc );
                bRet = sal_True;
            }

            ReleaseAccess( pReadAcc );

            if( bRet )
                ImplAssignWithSize( aNewBmp );
        }
    }

    return bRet;
}

Bitmap Bitmap::CreateMask( const Color& rTransColor, sal_uLong nTol ) const
{
    Bitmap              aNewBmp( GetSizePixel(), 1 );
    BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pWriteAcc )
    {
        BitmapReadAccess* pReadAcc = ( (Bitmap*) this )->AcquireReadAccess();

        if( pReadAcc )
        {
            const long          nWidth = pReadAcc->Width();
            const long          nHeight = pReadAcc->Height();
            const BitmapColor   aBlack( pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
            const BitmapColor   aWhite( pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

            if( !nTol )
            {
                const BitmapColor   aTest( pReadAcc->GetBestMatchingColor( rTransColor ) );
                long nX, nY;

                if( pReadAcc->GetScanlineFormat() == BMP_FORMAT_4BIT_MSN_PAL ||
                    pReadAcc->GetScanlineFormat() == BMP_FORMAT_4BIT_LSN_PAL )
                {
                    // optimized for 4Bit-MSN/LSN source palette
                    const sal_uInt8 cTest = aTest.GetIndex();
                    const long nShiftInit = ( ( pReadAcc->GetScanlineFormat() == BMP_FORMAT_4BIT_MSN_PAL ) ? 4 : 0 );

                    if( pWriteAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL &&
                        aWhite.GetIndex() == 1 )
                    {
                        // optimized for 1Bit-MSB destination palette
                        for( nY = 0L; nY < nHeight; nY++ )
                        {
                            Scanline pSrc = pReadAcc->GetScanline( nY );
                            Scanline pDst = pWriteAcc->GetScanline( nY );
                            long nShift = 0;
                            for( nX = 0L, nShift = nShiftInit; nX < nWidth; nX++, nShift ^= 4 )
                            {
                                if( cTest == ( ( pSrc[ nX >> 1 ] >> nShift ) & 0x0f ) )
                                    pDst[ nX >> 3 ] |= 1 << ( 7 - ( nX & 7 ) );
                                else
                                    pDst[ nX >> 3 ] &= ~( 1 << ( 7 - ( nX & 7 ) ) );
                            }
                        }
                    }
                    else
                    {
                        for( nY = 0L; nY < nHeight; nY++ )
                        {
                            Scanline pSrc = pReadAcc->GetScanline( nY );
                            long nShift = 0;
                            for( nX = 0L, nShift = nShiftInit; nX < nWidth; nX++, nShift ^= 4 )
                            {
                                if( cTest == ( ( pSrc[ nX >> 1 ] >> nShift ) & 0x0f ) )
                                    pWriteAcc->SetPixel( nY, nX, aWhite );
                                else
                                    pWriteAcc->SetPixel( nY, nX, aBlack );
                            }
                        }
                    }
                }
                else if( pReadAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                {
                    // optimized for 8Bit source palette
                    const sal_uInt8 cTest = aTest.GetIndex();

                    if( pWriteAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL &&
                        aWhite.GetIndex() == 1 )
                    {
                        // optimized for 1Bit-MSB destination palette
                        for( nY = 0L; nY < nHeight; nY++ )
                        {
                            Scanline pSrc = pReadAcc->GetScanline( nY );
                            Scanline pDst = pWriteAcc->GetScanline( nY );
                            for( nX = 0L; nX < nWidth; nX++ )
                            {
                                if( cTest == pSrc[ nX ] )
                                    pDst[ nX >> 3 ] |= 1 << ( 7 - ( nX & 7 ) );
                                else
                                    pDst[ nX >> 3 ] &= ~( 1 << ( 7 - ( nX & 7 ) ) );
                            }
                        }
                    }
                    else
                    {
                        for( nY = 0L; nY < nHeight; nY++ )
                        {
                            Scanline pSrc = pReadAcc->GetScanline( nY );
                            for( nX = 0L; nX < nWidth; nX++ )
                            {
                                if( cTest == pSrc[ nX ] )
                                    pWriteAcc->SetPixel( nY, nX, aWhite );
                                else
                                    pWriteAcc->SetPixel( nY, nX, aBlack );
                            }
                        }
                    }
                }
                else
                {
                    // not optimized
                    for( nY = 0L; nY < nHeight; nY++ )
                    {
                        for( nX = 0L; nX < nWidth; nX++ )
                        {
                            if( aTest == pReadAcc->GetPixel( nY, nX ) )
                                pWriteAcc->SetPixel( nY, nX, aWhite );
                            else
                                pWriteAcc->SetPixel( nY, nX, aBlack );
                        }
                    }
                }
            }
            else
            {
                BitmapColor aCol;
                long        nR, nG, nB;
                const long  nMinR = MinMax( (long) rTransColor.GetRed() - nTol, 0, 255 );
                const long  nMaxR = MinMax( (long) rTransColor.GetRed() + nTol, 0, 255 );
                const long  nMinG = MinMax( (long) rTransColor.GetGreen() - nTol, 0, 255 );
                const long  nMaxG = MinMax( (long) rTransColor.GetGreen() + nTol, 0, 255 );
                const long  nMinB = MinMax( (long) rTransColor.GetBlue() - nTol, 0, 255 );
                const long  nMaxB = MinMax( (long) rTransColor.GetBlue() + nTol, 0, 255 );

                if( pReadAcc->HasPalette() )
                {
                    for( long nY = 0L; nY < nHeight; nY++ )
                    {
                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aCol = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nX ) );
                            nR = aCol.GetRed();
                            nG = aCol.GetGreen();
                            nB = aCol.GetBlue();

                            if( nMinR <= nR && nMaxR >= nR &&
                                nMinG <= nG && nMaxG >= nG &&
                                nMinB <= nB && nMaxB >= nB )
                            {
                                pWriteAcc->SetPixel( nY, nX, aWhite );
                            }
                            else
                                pWriteAcc->SetPixel( nY, nX, aBlack );
                        }
                    }
                }
                else
                {
                    for( long nY = 0L; nY < nHeight; nY++ )
                    {
                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            aCol = pReadAcc->GetPixel( nY, nX );
                            nR = aCol.GetRed();
                            nG = aCol.GetGreen();
                            nB = aCol.GetBlue();

                            if( nMinR <= nR && nMaxR >= nR &&
                                nMinG <= nG && nMaxG >= nG &&
                                nMinB <= nB && nMaxB >= nB )
                            {
                                pWriteAcc->SetPixel( nY, nX, aWhite );
                            }
                            else
                                pWriteAcc->SetPixel( nY, nX, aBlack );
                        }
                    }
                }
            }

            ( (Bitmap*) this )->ReleaseAccess( pReadAcc );
            bRet = sal_True;
        }

        aNewBmp.ReleaseAccess( pWriteAcc );
    }

    if( bRet )
    {
        aNewBmp.maPrefSize = maPrefSize;
        aNewBmp.maPrefMapMode = maPrefMapMode;
    }
    else
        aNewBmp = Bitmap();

    return aNewBmp;
}

Region Bitmap::CreateRegion( const Color& rColor, const Rectangle& rRect ) const
{
    Region              aRegion;
    Rectangle           aRect( rRect );
    BitmapReadAccess*   pReadAcc = ( (Bitmap*) this )->AcquireReadAccess();

    aRect.Intersection( Rectangle( Point(), GetSizePixel() ) );
    aRect.Justify();

    if( pReadAcc )
    {
        Rectangle           aSubRect;
        const long          nLeft = aRect.Left();
        const long          nTop = aRect.Top();
        const long          nRight = aRect.Right();
        const long          nBottom = aRect.Bottom();
        const BitmapColor   aMatch( pReadAcc->GetBestMatchingColor( rColor ) );

        aRegion.ImplBeginAddRect();

        for( long nY = nTop; nY <= nBottom; nY++ )
        {
            aSubRect.Top() = aSubRect.Bottom() = nY;

            for( long nX = nLeft; nX <= nRight; )
            {
                while( ( nX <= nRight ) && ( aMatch != pReadAcc->GetPixel( nY, nX ) ) )
                    nX++;

                if( nX <= nRight )
                {
                    aSubRect.Left() = nX;

                    while( ( nX <= nRight ) && ( aMatch == pReadAcc->GetPixel( nY, nX ) ) )
                        nX++;

                    aSubRect.Right() = nX - 1L;
                    aRegion.ImplAddRect( aSubRect );
                }
            }
        }

        aRegion.ImplEndAddRect();
        ( (Bitmap*) this )->ReleaseAccess( pReadAcc );
    }
    else
        aRegion = aRect;

    return aRegion;
}

sal_Bool Bitmap::Replace( const Bitmap& rMask, const Color& rReplaceColor )
{
    BitmapReadAccess*   pMaskAcc = ( (Bitmap&) rMask ).AcquireReadAccess();
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pMaskAcc && pAcc )
    {
        const long          nWidth = std::min( pMaskAcc->Width(), pAcc->Width() );
        const long          nHeight = std::min( pMaskAcc->Height(), pAcc->Height() );
        const BitmapColor   aMaskWhite( pMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
        BitmapColor         aReplace;

        if( pAcc->HasPalette() )
        {
            const sal_uInt16 nActColors = pAcc->GetPaletteEntryCount();
            const sal_uInt16 nMaxColors = 1 << pAcc->GetBitCount();

            // For a start, choose the next color
            aReplace = pAcc->GetBestMatchingColor( rReplaceColor );

            // If it's a pallette picture and the color that should be set
            // is not in the pallette, we try finding a free entry (expensive)
            if( pAcc->GetPaletteColor( (sal_uInt8) aReplace ) != BitmapColor( rReplaceColor ) )
            {
                // See first if we can put the ReplaceColor at a free entry at the end of the pallette
                if( nActColors < nMaxColors )
                {
                    pAcc->SetPaletteEntryCount( nActColors + 1 );
                    pAcc->SetPaletteColor( nActColors, rReplaceColor );
                    aReplace = BitmapColor( (sal_uInt8) nActColors );
                }
                else
                {
                    sal_Bool* pFlags = new sal_Bool[ nMaxColors ];

                    // Set all entries to 0
                    memset( pFlags, 0, nMaxColors );

                    for( long nY = 0L; nY < nHeight; nY++ )
                        for( long nX = 0L; nX < nWidth; nX++ )
                            pFlags[ (sal_uInt8) pAcc->GetPixel( nY, nX ) ] = sal_True;

                    for( sal_uInt16 i = 0UL; i < nMaxColors; i++ )
                    {
                        // Hurray, we do have an unsused entry
                        if( !pFlags[ i ] )
                        {
                            pAcc->SetPaletteColor( (sal_uInt16) i, rReplaceColor );
                            aReplace = BitmapColor( (sal_uInt8) i );
                        }
                    }

                    delete[] pFlags;
                }
            }
        }
        else
            aReplace = rReplaceColor;

        for( long nY = 0L; nY < nHeight; nY++ )
            for( long nX = 0L; nX < nWidth; nX++ )
                if( pMaskAcc->GetPixel( nY, nX ) == aMaskWhite )
                    pAcc->SetPixel( nY, nX, aReplace );

        bRet = sal_True;
    }

    ( (Bitmap&) rMask ).ReleaseAccess( pMaskAcc );
    ReleaseAccess( pAcc );

    return bRet;
}

sal_Bool Bitmap::Replace( const AlphaMask& rAlpha, const Color& rMergeColor )
{
    Bitmap              aNewBmp( GetSizePixel(), 24 );
    BitmapReadAccess*   pAcc = AcquireReadAccess();
    BitmapReadAccess*   pAlphaAcc = ( (AlphaMask&) rAlpha ).AcquireReadAccess();
    BitmapWriteAccess*  pNewAcc = aNewBmp.AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pAcc && pAlphaAcc && pNewAcc )
    {
        BitmapColor aCol;
        const long  nWidth = std::min( pAlphaAcc->Width(), pAcc->Width() );
        const long  nHeight = std::min( pAlphaAcc->Height(), pAcc->Height() );

        for( long nY = 0L; nY < nHeight; nY++ )
        {
            for( long nX = 0L; nX < nWidth; nX++ )
            {
                aCol = pAcc->GetColor( nY, nX );
                pNewAcc->SetPixel( nY, nX, aCol.Merge( rMergeColor, 255 - (sal_uInt8) pAlphaAcc->GetPixel( nY, nX ) ) );
            }
        }

        bRet = sal_True;
    }

    ReleaseAccess( pAcc );
    ( (AlphaMask&) rAlpha ).ReleaseAccess( pAlphaAcc );
    aNewBmp.ReleaseAccess( pNewAcc );

    if( bRet )
    {
        const MapMode   aMap( maPrefMapMode );
        const Size      aSize( maPrefSize );

        *this = aNewBmp;

        maPrefMapMode = aMap;
        maPrefSize = aSize;
    }

    return bRet;
}

sal_Bool Bitmap::Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol )
{
    // Bitmaps with 1 bit color depth can cause problems
    // if they have other entries than black/white in their palette
    if( 1 == GetBitCount() )
        Convert( BMP_CONVERSION_4BIT_COLORS );

    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pAcc )
    {
        const long  nMinR = MinMax( (long) rSearchColor.GetRed() - nTol, 0, 255 );
        const long  nMaxR = MinMax( (long) rSearchColor.GetRed() + nTol, 0, 255 );
        const long  nMinG = MinMax( (long) rSearchColor.GetGreen() - nTol, 0, 255 );
        const long  nMaxG = MinMax( (long) rSearchColor.GetGreen() + nTol, 0, 255 );
        const long  nMinB = MinMax( (long) rSearchColor.GetBlue() - nTol, 0, 255 );
        const long  nMaxB = MinMax( (long) rSearchColor.GetBlue() + nTol, 0, 255 );

        if( pAcc->HasPalette() )
        {
            for( sal_uInt16 i = 0, nPalCount = pAcc->GetPaletteEntryCount(); i < nPalCount; i++ )
            {
                const BitmapColor& rCol = pAcc->GetPaletteColor( i );

                if( nMinR <= rCol.GetRed() && nMaxR >= rCol.GetRed() &&
                    nMinG <= rCol.GetGreen() && nMaxG >= rCol.GetGreen() &&
                    nMinB <= rCol.GetBlue() && nMaxB >= rCol.GetBlue() )
                {
                    pAcc->SetPaletteColor( i, rReplaceColor );
                }
            }
        }
        else
        {
            BitmapColor         aCol;
            const BitmapColor   aReplace( pAcc->GetBestMatchingColor( rReplaceColor ) );

            for( long nY = 0L, nHeight = pAcc->Height(); nY < nHeight; nY++ )
            {
                for( long nX = 0L, nWidth = pAcc->Width(); nX < nWidth; nX++ )
                {
                    aCol = pAcc->GetPixel( nY, nX );

                    if( nMinR <= aCol.GetRed() && nMaxR >= aCol.GetRed() &&
                        nMinG <= aCol.GetGreen() && nMaxG >= aCol.GetGreen() &&
                        nMinB <= aCol.GetBlue() && nMaxB >= aCol.GetBlue() )
                    {
                        pAcc->SetPixel( nY, nX, aReplace );
                    }
                }
            }
        }

        ReleaseAccess( pAcc );
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool Bitmap::Replace( const Color* pSearchColors, const Color* pReplaceColors,
                      sal_uLong nColorCount, sal_uLong* _pTols )
{
    // Bitmaps with 1 bit color depth can cause problems
    // if they have other entries than black/white in their palette
    if( 1 == GetBitCount() )
        Convert( BMP_CONVERSION_4BIT_COLORS );

    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pAcc )
    {
        long*   pMinR = new long[ nColorCount ];
        long*   pMaxR = new long[ nColorCount ];
        long*   pMinG = new long[ nColorCount ];
        long*   pMaxG = new long[ nColorCount ];
        long*   pMinB = new long[ nColorCount ];
        long*   pMaxB = new long[ nColorCount ];
        long*   pTols;
        sal_uLong   i;

        if( !_pTols )
        {
            pTols = new long[ nColorCount ];
            memset( pTols, 0, nColorCount * sizeof( long ) );
        }
        else
            pTols = (long*) _pTols;

        for( i = 0UL; i < nColorCount; i++ )
        {
            const Color&    rCol = pSearchColors[ i ];
            const long      nTol = pTols[ i ];

            pMinR[ i ] = MinMax( (long) rCol.GetRed() - nTol, 0, 255 );
            pMaxR[ i ] = MinMax( (long) rCol.GetRed() + nTol, 0, 255 );
            pMinG[ i ] = MinMax( (long) rCol.GetGreen() - nTol, 0, 255 );
            pMaxG[ i ] = MinMax( (long) rCol.GetGreen() + nTol, 0, 255 );
            pMinB[ i ] = MinMax( (long) rCol.GetBlue() - nTol, 0, 255 );
            pMaxB[ i ] = MinMax( (long) rCol.GetBlue() + nTol, 0, 255 );
        }

        if( pAcc->HasPalette() )
        {
            for( sal_uInt16 nEntry = 0, nPalCount = pAcc->GetPaletteEntryCount(); nEntry < nPalCount; nEntry++ )
            {
                const BitmapColor& rCol = pAcc->GetPaletteColor( nEntry );

                for( i = 0UL; i < nColorCount; i++ )
                {
                    if( pMinR[ i ] <= rCol.GetRed() && pMaxR[ i ] >= rCol.GetRed() &&
                        pMinG[ i ] <= rCol.GetGreen() && pMaxG[ i ] >= rCol.GetGreen() &&
                        pMinB[ i ] <= rCol.GetBlue() && pMaxB[ i ] >= rCol.GetBlue() )
                    {
                        pAcc->SetPaletteColor( (sal_uInt16)nEntry, pReplaceColors[ i ] );
                        break;
                    }
                }
            }
        }
        else
        {
            BitmapColor     aCol;
            BitmapColor*    pReplaces = new BitmapColor[ nColorCount ];

            for( i = 0UL; i < nColorCount; i++ )
                pReplaces[ i ] = pAcc->GetBestMatchingColor( pReplaceColors[ i ] );

            for( long nY = 0L, nHeight = pAcc->Height(); nY < nHeight; nY++ )
            {
                for( long nX = 0L, nWidth = pAcc->Width(); nX < nWidth; nX++ )
                {
                    aCol = pAcc->GetPixel( nY, nX );

                    for( i = 0UL; i < nColorCount; i++ )
                    {
                        if( pMinR[ i ] <= aCol.GetRed() && pMaxR[ i ] >= aCol.GetRed() &&
                            pMinG[ i ] <= aCol.GetGreen() && pMaxG[ i ] >= aCol.GetGreen() &&
                            pMinB[ i ] <= aCol.GetBlue() && pMaxB[ i ] >= aCol.GetBlue() )
                        {
                            pAcc->SetPixel( nY, nX, pReplaces[ i ] );
                            break;
                        }
                    }
                }
            }

            delete[] pReplaces;
        }

        if( !_pTols )
            delete[] pTols;

        delete[] pMinR;
        delete[] pMaxR;
        delete[] pMinG;
        delete[] pMaxG;
        delete[] pMinB;
        delete[] pMaxB;
        ReleaseAccess( pAcc );
        bRet = sal_True;
    }

    return bRet;
}

Bitmap Bitmap::CreateDisplayBitmap( OutputDevice* pDisplay )
{
    Bitmap aDispBmp( *this );

    if( mpImpBmp && ( pDisplay->mpGraphics || pDisplay->ImplGetGraphics() ) )
    {
        ImpBitmap* pImpDispBmp = new ImpBitmap;

        if( pImpDispBmp->ImplCreate( *mpImpBmp, pDisplay->mpGraphics ) )
            aDispBmp.ImplSetImpBitmap( pImpDispBmp );
        else
            delete pImpDispBmp;
    }

    return aDispBmp;
}

sal_Bool Bitmap::CombineSimple( const Bitmap& rMask, BmpCombine eCombine )
{
    BitmapReadAccess*   pMaskAcc = ( (Bitmap&) rMask ).AcquireReadAccess();
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pMaskAcc && pAcc )
    {
        const long          nWidth = std::min( pMaskAcc->Width(), pAcc->Width() );
        const long          nHeight = std::min( pMaskAcc->Height(), pAcc->Height() );
        const Color         aColBlack( COL_BLACK );
        BitmapColor         aPixel;
        BitmapColor         aMaskPixel;
        const BitmapColor   aWhite( pAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
        const BitmapColor   aBlack( pAcc->GetBestMatchingColor( aColBlack ) );
        const BitmapColor   aMaskBlack( pMaskAcc->GetBestMatchingColor( aColBlack ) );

        switch( eCombine )
        {
            case( BMP_COMBINE_COPY ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pMaskAcc->GetPixel( nY, nX ) == aMaskBlack )
                        pAcc->SetPixel( nY, nX, aBlack );
                    else
                        pAcc->SetPixel( nY, nX, aWhite );
                }
            }
            break;

            case( BMP_COMBINE_INVERT ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pAcc->GetPixel( nY, nX ) == aBlack )
                        pAcc->SetPixel( nY, nX, aWhite );
                    else
                        pAcc->SetPixel( nY, nX, aBlack );
                }
            }
            break;

            case( BMP_COMBINE_AND ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pMaskAcc->GetPixel( nY, nX ) != aMaskBlack && pAcc->GetPixel( nY, nX ) != aBlack )
                        pAcc->SetPixel( nY, nX, aWhite );
                    else
                        pAcc->SetPixel( nY, nX, aBlack );
                }
            }
            break;

            case( BMP_COMBINE_NAND ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pMaskAcc->GetPixel( nY, nX ) != aMaskBlack && pAcc->GetPixel( nY, nX ) != aBlack )
                        pAcc->SetPixel( nY, nX, aBlack );
                    else
                        pAcc->SetPixel( nY, nX, aWhite );
                }
            }
            break;

            case( BMP_COMBINE_OR ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pMaskAcc->GetPixel( nY, nX ) != aMaskBlack || pAcc->GetPixel( nY, nX ) != aBlack )
                        pAcc->SetPixel( nY, nX, aWhite );
                    else
                        pAcc->SetPixel( nY, nX, aBlack );
                }
            }
            break;

            case( BMP_COMBINE_NOR ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pMaskAcc->GetPixel( nY, nX ) != aMaskBlack || pAcc->GetPixel( nY, nX ) != aBlack )
                        pAcc->SetPixel( nY, nX, aBlack );
                    else
                        pAcc->SetPixel( nY, nX, aWhite );
                }
            }
            break;

            case( BMP_COMBINE_XOR ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    aPixel = pAcc->GetPixel( nY, nX );
                    aMaskPixel = pMaskAcc->GetPixel( nY, nX );

                    if( ( aMaskPixel != aMaskBlack && aPixel == aBlack ) ||
                        ( aMaskPixel == aMaskBlack && aPixel != aBlack ) )
                    {
                        pAcc->SetPixel( nY, nX, aWhite );
                    }
                    else
                        pAcc->SetPixel( nY, nX, aBlack );
                }
            }
            break;

            case( BMP_COMBINE_NXOR ):
            {
                for( long nY = 0L; nY < nHeight; nY++ ) for( long nX = 0L; nX < nWidth; nX++ )
                {
                    aPixel = pAcc->GetPixel( nY, nX );
                    aMaskPixel = pMaskAcc->GetPixel( nY, nX );

                    if( ( aMaskPixel != aMaskBlack && aPixel == aBlack ) ||
                        ( aMaskPixel == aMaskBlack && aPixel != aBlack ) )
                    {
                        pAcc->SetPixel( nY, nX, aBlack );
                    }
                    else
                        pAcc->SetPixel( nY, nX, aWhite );
                }
            }
            break;
        }

        bRet = sal_True;
    }

    ( (Bitmap&) rMask ).ReleaseAccess( pMaskAcc );
    ReleaseAccess( pAcc );

    return bRet;
}

// TODO: Have a look at OutputDevice::ImplDrawAlpha() for some
// optimizations. Might even consolidate the code here and there.
sal_Bool Bitmap::Blend( const AlphaMask& rAlpha, const Color& rBackgroundColor )
{
    // Convert to a truecolor bitmap, if we're a paletted one. There's
    // room for tradeoff decision here, maybe later for an overload (or a flag)
    if( GetBitCount() <= 8 )
        Convert( BMP_CONVERSION_24BIT );

    BitmapReadAccess*   pAlphaAcc = const_cast<AlphaMask&>(rAlpha).AcquireReadAccess();
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    sal_Bool                bRet = sal_False;

    if( pAlphaAcc && pAcc )
    {
        const long          nWidth = std::min( pAlphaAcc->Width(), pAcc->Width() );
        const long          nHeight = std::min( pAlphaAcc->Height(), pAcc->Height() );

        for( long nY = 0L; nY < nHeight; ++nY )
            for( long nX = 0L; nX < nWidth; ++nX )
                pAcc->SetPixel( nY, nX,
                                pAcc->GetPixel( nY, nX ).Merge( rBackgroundColor,
                                                                255 - pAlphaAcc->GetPixel( nY, nX ) ) );

        bRet = sal_True;
    }

    const_cast<AlphaMask&>(rAlpha).ReleaseAccess( pAlphaAcc );
    ReleaseAccess( pAcc );

    return bRet;
}

sal_Bool Bitmap::MakeMono( sal_uInt8 cThreshold )
{
    return ImplMakeMono( cThreshold );
}

bool Bitmap::GetSystemData( BitmapSystemData& rData ) const
{
    bool bRet = false;
    if( mpImpBmp )
    {
        SalBitmap* pSalBitmap = mpImpBmp->ImplGetSalBitmap();
        if( pSalBitmap )
            bRet = pSalBitmap->GetSystemData( rData );
    }

    return bRet;
}

bool Bitmap::HasAlpha()
{
    bool bRet = false;
    if( mpImpBmp )
    {
        SalBitmap* pSalBitmap = mpImpBmp->ImplGetSalBitmap();
        if( pSalBitmap )
            bRet = pSalBitmap->HasAlpha();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
