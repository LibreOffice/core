/*************************************************************************
 *
 *  $RCSfile: imgcons.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
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

#include <bmpacc.hxx>
#include <bitmapex.hxx>
#include <image.hxx>
#include <imgcons.hxx>

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
    mnStatus( 0UL ),
    mpPal   ( NULL ),
    mpMapper( NULL ),
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

void ImageConsumer::Init( ULONG nWidth, ULONG nHeight )
{
    maSize = Size( nWidth, nHeight );
    maBitmap = maMask = Bitmap();
    mnStatus = 0UL;
    mbTrans = FALSE;
}

// -----------------------------------------------------------------------------

void ImageConsumer::SetColorModel( USHORT nBitCount,
                                   ULONG nPalEntries, const ULONG* pRGBAPal,
                                   ULONG nRMask, ULONG nGMask, ULONG nBMask, ULONG nAMask )
{
    DBG_ASSERT( maSize.Width() && maSize.Height(), "Missing call to ImageConsumer::Init(...)!" );

    BitmapPalette aPal( Min( (USHORT) nPalEntries, (USHORT) 256 ) );

    if( nPalEntries )
    {
        BitmapColor aCol;
        const ULONG* pTmp = pRGBAPal;

        delete mpMapper;
        mpMapper = NULL;

        delete[] mpPal;
        mpPal = new Color[ nPalEntries ];

        for( ULONG i = 0; i < nPalEntries; i++, pTmp++ )
        {
            Color&  rCol = mpPal[ i ];
            BYTE    cVal;

            cVal = (BYTE) ( ( *pTmp & 0xff000000UL ) >> 24UL );
            rCol.SetRed( cVal );
            if( i < 256UL )
                aPal[ (USHORT) i ].SetRed( cVal );

            cVal = (BYTE) ( ( *pTmp & 0x00ff0000UL ) >> 16UL );
            rCol.SetGreen( cVal );
            if( i < 256UL )
                aPal[ (USHORT) i ].SetGreen( cVal );

            cVal = (BYTE) ( ( *pTmp & 0x0000ff00UL ) >> 8UL );
            rCol.SetBlue( cVal );
            if( i < 256UL )
                aPal[ (USHORT) i ].SetBlue( cVal );

            rCol.SetTransparency( (BYTE) ( ( *pTmp & 0x000000ffUL ) ) );
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

void ImageConsumer::SetPixelsByBytes( ULONG nConsX, ULONG nConsY,
                                      ULONG nConsWidth, ULONG nConsHeight,
                                      const BYTE* pData, ULONG nOffset, ULONG nScanSize )
{
    DBG_ASSERT( !!maBitmap && !!maMask, "Missing call to ImageConsumer::SetColorModel(...)!" );

    BitmapWriteAccess* pBmpAcc = maBitmap.AcquireWriteAccess();
    BitmapWriteAccess* pMskAcc = maMask.AcquireWriteAccess();

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

                DataChanged();
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

                DataChanged();
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

                DataChanged();
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
}

// -----------------------------------------------------------------------------

void ImageConsumer::SetPixelsByLongs( ULONG nConsX, ULONG nConsY,
                                      ULONG nConsWidth, ULONG nConsHeight,
                                      const ULONG* pData, ULONG nOffset, ULONG nScanSize )
{
    DBG_ASSERT( !!maBitmap && !!maMask, "Missing call to ImageConsumer::SetColorModel(...)!" );

    BitmapWriteAccess* pBmpAcc = maBitmap.AcquireWriteAccess();
    BitmapWriteAccess* pMskAcc = maMask.AcquireWriteAccess();

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
                    const ULONG* pTmp = pData + ( nY - nStartY ) * nScanSize + nOffset;

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

                DataChanged();
            }
            else if( mpPal && ( pBmpAcc->GetBitCount() <= 8 ) )
            {
                BitmapColor aIndex( (BYTE) 0 );
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const ULONG* pTmp = pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const ULONG     nIndex = *pTmp++;
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

                DataChanged();
            }
            else if( mpPal && ( pBmpAcc->GetBitCount() > 8 ) )
            {
                BitmapColor aCol;
                BitmapColor aMskWhite( pMskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                for( long nY = nStartY; nY <= nEndY; nY++ )
                {
                    const ULONG* pTmp = pData + ( nY - nStartY ) * nScanSize + nOffset;

                    for( long nX = nStartX; nX <= nEndX; nX++ )
                    {
                        const ULONG     nIndex = *pTmp++;
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

                DataChanged();
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
}

// -----------------------------------------------------------------------------

void ImageConsumer::Completed( ULONG nStatus /*, ImageProducer& rProducer */ )
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

ULONG ImageConsumer::GetStatus() const
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
