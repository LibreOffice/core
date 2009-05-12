/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imgcons.cxx,v $
 * $Revision: 1.8 $
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

#include <tools/stream.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>
#include <vcl/imgcons.hxx>

// -------------------
// - ImplColorMapper -
// -------------------

class ImplColorMapper
{
    Color               maCol;
    ULONG               mnR;
    ULONG               mnG;
    ULONG               mnB;
    ULONG               mnT;
    ULONG               mnRShift;
    ULONG               mnGShift;
    ULONG               mnBShift;
    ULONG               mnTShift;

    ULONG               ImplCalcMaskShift( ULONG nVal );

public:

                        ImplColorMapper( ULONG nRMask, ULONG nGMask, ULONG nBMask, ULONG nTMask );
                        ~ImplColorMapper();

    const Color&        ImplGetColor( ULONG nColor )
                        {
                            maCol.SetRed( (UINT8) ( ( nColor & mnR ) >> mnRShift ) );
                            maCol.SetGreen( (UINT8) ( ( nColor & mnG ) >> mnGShift ) );
                            maCol.SetBlue( (UINT8) ( ( nColor & mnB ) >> mnBShift ) );
                            maCol.SetTransparency( (UINT8) ( ( nColor & mnT ) >> mnTShift ) );
                            return maCol;
                        }
};

// -----------------------------------------------------------------------------

ImplColorMapper::ImplColorMapper( ULONG nRMask, ULONG nGMask, ULONG nBMask, ULONG nTMask ) :
        mnR( nRMask ),
        mnG( nGMask ),
        mnB( nBMask ),
        mnT( nTMask )
{
    mnRShift = ImplCalcMaskShift( mnR );
    mnGShift = ImplCalcMaskShift( mnG );
    mnBShift = ImplCalcMaskShift( mnB );
    mnTShift = ImplCalcMaskShift( mnT );
}

// -----------------------------------------------------------------------------

ImplColorMapper::~ImplColorMapper()
{
}

// -----------------------------------------------------------------------------

ULONG ImplColorMapper::ImplCalcMaskShift( ULONG nVal )
{
    DBG_ASSERT( nVal > 0, "Mask has no value!" );

    ULONG nRet = 0UL;

    for( ULONG i = 0UL; i < 32; i++ )
    {
        if( nVal & ( 1UL << i ) )
        {
            nRet = i;
            break;
        }
    }

    return nRet;
}

// -----------------
// - ImageConsumer -
// -----------------

ImageConsumer::ImageConsumer() :
    mpMapper( NULL ),
    mpPal   ( NULL ),
    mnStatus( 0UL ),
    mbTrans ( FALSE )
{
}

// -----------------------------------------------------------------------------

ImageConsumer::~ImageConsumer()
{
    delete[] mpPal;
    delete mpMapper;
}

// -----------------------------------------------------------------------------

void ImageConsumer::Init( sal_uInt32 nWidth, sal_uInt32 nHeight )
{
    maSize = Size( nWidth, nHeight );
    maBitmap = maMask = Bitmap();
    mnStatus = 0UL;
    mbTrans = FALSE;
}

// -----------------------------------------------------------------------------

void ImageConsumer::SetColorModel( USHORT nBitCount,
                                   sal_uInt32 nPalEntries, const sal_uInt32* pRGBAPal,
                                   sal_uInt32 nRMask, sal_uInt32 nGMask, sal_uInt32 nBMask, sal_uInt32 nAMask )
{
    DBG_ASSERT( maSize.Width() && maSize.Height(), "Missing call to ImageConsumer::Init(...)!" );

    BitmapPalette aPal( Min( (USHORT) nPalEntries, (USHORT) 256 ) );

    if( nPalEntries )
    {
        BitmapColor aCol;
        const sal_Int32* pTmp = (sal_Int32*) pRGBAPal;

        delete mpMapper;
        mpMapper = NULL;

        delete[] mpPal;
        mpPal = new Color[ nPalEntries ];

        for( ULONG i = 0; i < nPalEntries; i++, pTmp++ )
        {
            Color&  rCol = mpPal[ i ];
            BYTE    cVal;

            cVal = (BYTE) ( ( *pTmp & 0xff000000UL ) >> 24L );
            rCol.SetRed( cVal );

            if( i < (ULONG) aPal.GetEntryCount() )
                aPal[ (USHORT) i ].SetRed( cVal );

            cVal = (BYTE) ( ( *pTmp & 0x00ff0000UL ) >> 16L );
            rCol.SetGreen( cVal );

            if( i < (ULONG) aPal.GetEntryCount() )
                aPal[ (USHORT) i ].SetGreen( cVal );

            cVal = (BYTE) ( ( *pTmp & 0x0000ff00UL ) >> 8L );
            rCol.SetBlue( cVal );

            if( i < (ULONG) aPal.GetEntryCount() )
                aPal[ (USHORT) i ].SetBlue( cVal );

            rCol.SetTransparency( (BYTE) ( ( *pTmp & 0x000000ffL ) ) );
        }

        if( nBitCount <= 1 )
            nBitCount = 1;
        else if( nBitCount <= 4 )
            nBitCount = 4;
        else if( nBitCount <= 8 )
            nBitCount = 8;
        else
            nBitCount = 24;
    }
    else
    {
        delete mpMapper;
        mpMapper = new ImplColorMapper( nRMask, nGMask, nBMask, nAMask );

        delete[] mpPal;
        mpPal = NULL;

        nBitCount = 24;
    }

    if( !maBitmap )
    {

        maBitmap = Bitmap( maSize, nBitCount, &aPal );
        maMask = Bitmap( maSize, 1 );
        maMask.Erase( COL_BLACK );
        mbTrans = FALSE;
    }
}

// -----------------------------------------------------------------------------

void ImageConsumer::SetPixelsByBytes( sal_uInt32 nConsX, sal_uInt32 nConsY,
                                      sal_uInt32 nConsWidth, sal_uInt32 nConsHeight,
                                      const BYTE* pData, sal_uInt32 nOffset, sal_uInt32 nScanSize )
{
    DBG_ASSERT( !!maBitmap && !!maMask, "Missing call to ImageConsumer::SetColorModel(...)!" );

    BitmapWriteAccess*  pBmpAcc = maBitmap.AcquireWriteAccess();
    BitmapWriteAccess*  pMskAcc = maMask.AcquireWriteAccess();
    sal_Bool            bDataChanged = sal_False;

    if( pBmpAcc && pMskAcc )
    {
        const long  nWidth = pBmpAcc->Width();
        const long  nHeight = pBmpAcc->Height();

        maChangedRect = Rectangle( Point(), Size( nWidth, nHeight ) );
        maChangedRect.Intersection( Rectangle( Point( nConsX, nConsY ), Size( nConsWidth, nConsHeight ) ) );

        if( !maChangedRect.IsEmpty() )
        {
            const long nStartX = maChangedRect.Left();
            const long nEndX = maChangedRect.Right();
            const long nStartY = maChangedRect.Top();
            const long nEndY = maChangedRect.Bottom();

            if( mpMapper && ( pBmpAcc->GetBitCount() > 8 ) )
            {
                BitmapColor aCol;
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const BYTE* pTmp = pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const Color& rCol = mpMapper->ImplGetColor( *pTmp++ );

                        // 0: Transparent; >0: Non-Transparent
                        if( !rCol.GetTransparency() )
                        {
                            pMskAcc->SetPixel( nY, nX, aMskWhite );
                            mbTrans = TRUE;
                        }
                        else
                        {
                            aCol.SetRed( rCol.GetRed() );
                            aCol.SetGreen( rCol.GetGreen() );
                            aCol.SetBlue( rCol.GetBlue() );
                            pBmpAcc->SetPixel( nY, nX, aCol );
                        }
                    }
                }

                bDataChanged = sal_True;
            }
            else if( mpPal && ( pBmpAcc->GetBitCount() <= 8 ) )
            {
                BitmapColor aIndex( (BYTE) 0 );
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const BYTE* pTmp = pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const BYTE      cIndex = *pTmp++;
                        const Color&    rCol = mpPal[ cIndex ];

                        // 0: Transparent; >0: Non-Transparent
                        if( !rCol.GetTransparency() )
                        {
                            pMskAcc->SetPixel( nY, nX, aMskWhite );
                            mbTrans = TRUE;
                        }
                        else
                        {
                            aIndex.SetIndex( cIndex );
                            pBmpAcc->SetPixel( nY, nX, aIndex );
                        }
                    }
                }

                bDataChanged = sal_True;
            }
            else if( mpPal && ( pBmpAcc->GetBitCount() > 8 ) )
            {
                BitmapColor aCol;
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const BYTE* pTmp = pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const BYTE      cIndex = *pTmp++;
                        const Color&    rCol = mpPal[ cIndex ];

                        // 0: Transparent; >0: Non-Transparent
                        if( !rCol.GetTransparency() )
                        {
                            pMskAcc->SetPixel( nY, nX, aMskWhite );
                            mbTrans = TRUE;
                        }
                        else
                        {
                            aCol.SetRed( rCol.GetRed() );
                            aCol.SetGreen( rCol.GetGreen() );
                            aCol.SetBlue( rCol.GetBlue() );
                            pBmpAcc->SetPixel( nY, nX, aCol );
                        }
                    }
                }

                bDataChanged = sal_True;
            }
            else
            {
                DBG_ERROR( "Producer format error!" );
                maChangedRect.SetEmpty();
            }
        }
    }
    else
        maChangedRect.SetEmpty();

    maBitmap.ReleaseAccess( pBmpAcc );
    maMask.ReleaseAccess( pMskAcc );

    if( bDataChanged )
        DataChanged();
}

// -----------------------------------------------------------------------------

void ImageConsumer::SetPixelsByLongs( sal_uInt32 nConsX, sal_uInt32 nConsY,
                                      sal_uInt32 nConsWidth, sal_uInt32 nConsHeight,
                                      const sal_uInt32* pData, sal_uInt32 nOffset, sal_uInt32 nScanSize )
{
    DBG_ASSERT( !!maBitmap && !!maMask, "Missing call to ImageConsumer::SetColorModel(...)!" );

    BitmapWriteAccess*  pBmpAcc = maBitmap.AcquireWriteAccess();
    BitmapWriteAccess*  pMskAcc = maMask.AcquireWriteAccess();
    sal_Bool            bDataChanged = sal_False;

    if( pBmpAcc && pMskAcc )
    {
        const long  nWidth = pBmpAcc->Width();
        const long  nHeight = pBmpAcc->Height();

        maChangedRect = Rectangle( Point(), Size( nWidth, nHeight ) );
        maChangedRect.Intersection( Rectangle( Point( nConsX, nConsY ), Size( nConsWidth, nConsHeight ) ) );

        if( !maChangedRect.IsEmpty() )
        {
            const long nStartX = maChangedRect.Left();
            const long nEndX = maChangedRect.Right();
            const long nStartY = maChangedRect.Top();
            const long nEndY = maChangedRect.Bottom();

            if( mpMapper && ( pBmpAcc->GetBitCount() > 8 ) )
            {
                BitmapColor aCol;
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const sal_Int32* pTmp = (sal_Int32*) pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const Color& rCol = mpMapper->ImplGetColor( *pTmp++ );

                        // 0: Transparent; >0: Non-Transparent
                        if( !rCol.GetTransparency() )
                        {
                            pMskAcc->SetPixel( nY, nX, aMskWhite );
                            mbTrans = TRUE;
                        }
                        else
                        {
                            aCol.SetRed( rCol.GetRed() );
                            aCol.SetGreen( rCol.GetGreen() );
                            aCol.SetBlue( rCol.GetBlue() );
                            pBmpAcc->SetPixel( nY, nX, aCol );
                        }
                    }
                }

                bDataChanged = sal_True;
            }
            else if( mpPal && ( pBmpAcc->GetBitCount() <= 8 ) )
            {
                BitmapColor aIndex( (BYTE) 0 );
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const sal_Int32* pTmp = (sal_Int32*) pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const sal_Int32 nIndex = *pTmp++;
                        const Color&    rCol = mpPal[ nIndex ];

                        // 0: Transparent; >0: Non-Transparent
                        if( !rCol.GetTransparency() )
                        {
                            pMskAcc->SetPixel( nY, nX, aMskWhite );
                            mbTrans = TRUE;
                        }
                        else
                        {
                            aIndex.SetIndex( (BYTE) nIndex );
                            pBmpAcc->SetPixel( nY, nX, aIndex );
                        }
                    }
                }

                bDataChanged = sal_True;
            }
            else if( mpPal && ( pBmpAcc->GetBitCount() > 8 ) )
            {
                BitmapColor aCol;
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const sal_Int32* pTmp = (sal_Int32*) pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const sal_Int32 nIndex = *pTmp++;
                        const Color&    rCol = mpPal[ nIndex ];

                        // 0: Transparent; >0: Non-Transparent
                        if( !rCol.GetTransparency() )
                        {
                            pMskAcc->SetPixel( nY, nX, aMskWhite );
                            mbTrans = TRUE;
                        }
                        else
                        {
                            aCol.SetRed( rCol.GetRed() );
                            aCol.SetGreen( rCol.GetGreen() );
                            aCol.SetBlue( rCol.GetBlue() );
                            pBmpAcc->SetPixel( nY, nX, aCol );
                        }
                    }
                }

                bDataChanged = sal_True;
            }
            else
            {
                DBG_ERROR( "Producer format error!" );
                maChangedRect.SetEmpty();
            }
        }
    }
    else
        maChangedRect.SetEmpty();

    maBitmap.ReleaseAccess( pBmpAcc );
    maMask.ReleaseAccess( pMskAcc );

    if( bDataChanged )
        DataChanged();
}

// -----------------------------------------------------------------------------

void ImageConsumer::Completed( sal_uInt32 nStatus /*, ImageProducer& rProducer */ )
{
    delete mpMapper;
    mpMapper = NULL;
    delete[] mpPal;
    mpPal = NULL;
    maSize = Size();
    mnStatus = nStatus;

    switch( nStatus )
    {
        case( SINGLEFRAMEDONE ):
        case( STATICIMAGEDONE ):
        {
            if( !mbTrans )
                maMask = Bitmap();
        }
        break;

        case( IMAGEERROR ):
        case( IMAGEABORTED ):
            maBitmap = maMask = Bitmap();
        break;

        default:
        break;
    }

//  rProducer.RemoveConsumer( *this );

    if( maDoneLink.IsSet() )
        maDoneLink.Call( this );
}

// -----------------------------------------------------------------------------

void ImageConsumer::DataChanged()
{
    if( maChgLink.IsSet() )
        maChgLink.Call( this );
}

// -----------------------------------------------------------------------------

sal_uInt32 ImageConsumer::GetStatus() const
{
    return mnStatus;
}

// -----------------------------------------------------------------------------

BOOL ImageConsumer::GetData( BitmapEx& rBmpEx ) const
{
    const BOOL bRet = ( SINGLEFRAMEDONE == mnStatus || STATICIMAGEDONE == mnStatus );

    if( bRet )
    {
        if( !!maMask )
            rBmpEx = BitmapEx( maBitmap, maMask );
        else
            rBmpEx = BitmapEx( maBitmap );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL ImageConsumer::GetData( Image& rImage ) const
{
    const BOOL bRet = ( SINGLEFRAMEDONE == mnStatus || STATICIMAGEDONE == mnStatus );

    if( bRet )
    {
        if( !!maMask )
            rImage = Image( maBitmap, maMask );
        else
            rImage = Image( maBitmap );
    }

    return bRet;
}
