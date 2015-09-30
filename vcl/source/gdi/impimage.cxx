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

#include <vcl/outdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/alpha.hxx>
#include <vcl/window.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>

#include <image.h>
#include <memory>

#define IMPSYSIMAGEITEM_MASK        ( 0x01 )
#define IMPSYSIMAGEITEM_ALPHA       ( 0x02 )

ImageAryData::ImageAryData( const ImageAryData& rData ) :
    maName( rData.maName ),
    mnId( rData.mnId ),
    maBitmapEx( rData.maBitmapEx )
{
}

ImageAryData::ImageAryData( const OUString &aName,
                            sal_uInt16 nId, const BitmapEx &aBitmap )
        : maName( aName ), mnId( nId ), maBitmapEx( aBitmap )
{
}

ImageAryData::~ImageAryData()
{
}

ImageAryData& ImageAryData::operator=( const ImageAryData& rData )
{
    maName = rData.maName;
    mnId = rData.mnId;
    maBitmapEx = rData.maBitmapEx;

    return *this;
}

ImplImageList::ImplImageList()
    : mnRefCount(1)
{
}

ImplImageList::ImplImageList( const ImplImageList &aSrc )
    : maPrefix(aSrc.maPrefix)
    , maImageSize(aSrc.maImageSize)
    , mnRefCount(1)
{
    maImages.reserve( aSrc.maImages.size() );
    for ( ImageAryDataVec::const_iterator aIt = aSrc.maImages.begin(), aEnd = aSrc.maImages.end(); aIt != aEnd; ++aIt )
    {
        ImageAryData* pAryData = new ImageAryData( **aIt );
        maImages.push_back( pAryData );
        if( !pAryData->maName.isEmpty() )
            maNameHash [ pAryData->maName ] = pAryData;
    }
}

ImplImageList::~ImplImageList()
{
    for ( ImageAryDataVec::iterator aIt = maImages.begin(), aEnd = maImages.end(); aIt != aEnd; ++aIt )
        delete *aIt;
}

void ImplImageList::AddImage( const OUString &aName,
                              sal_uInt16 nId, const BitmapEx &aBitmapEx )
{
    ImageAryData *pImg = new ImageAryData( aName, nId, aBitmapEx );
    maImages.push_back( pImg );
    if( !aName.isEmpty() )
        maNameHash [ aName ] = pImg;
}

void ImplImageList::RemoveImage( sal_uInt16 nPos )
{
    ImageAryData *pImg = maImages[ nPos ];
    if( !pImg->maName.isEmpty() )
        maNameHash.erase( pImg->maName );
    maImages.erase( maImages.begin() + nPos );
}

ImplImageData::ImplImageData( const BitmapEx& rBmpEx ) :
    mpImageBitmap( NULL ),
    maBmpEx( rBmpEx )
{
}

ImplImageData::~ImplImageData()
{
    delete mpImageBitmap;
}

bool ImplImageData::IsEqual( const ImplImageData& rData )
{
    return( maBmpEx == rData.maBmpEx );
}

ImplImage::ImplImage()
    : mnRefCount(1)
    , mpData(NULL)
    , meType(IMAGETYPE_BITMAP)
{
}

ImplImage::~ImplImage()
{
    switch( meType )
    {
        case IMAGETYPE_BITMAP:
            delete static_cast< Bitmap* >( mpData );
        break;

        case IMAGETYPE_IMAGE:
            delete static_cast< ImplImageData* >( mpData );
        break;
    }
}

ImplImageBmp::ImplImageBmp() :
    mpDisplayBmp( NULL ),
    mpInfoAry( NULL ),
    mnSize( 0 )
{
}

ImplImageBmp::~ImplImageBmp()
{
    delete[] mpInfoAry;
    delete mpDisplayBmp;
}

void ImplImageBmp::Create( const BitmapEx& rBmpEx, long nItemWidth, long nItemHeight, sal_uInt16 nInitSize )
{
    maBmpEx = rBmpEx;
    maDisabledBmpEx.SetEmpty();

    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    maSize = Size( nItemWidth, nItemHeight );
    mnSize = nInitSize;

    delete[] mpInfoAry;
    mpInfoAry = new sal_uInt8[ mnSize ];
    memset( mpInfoAry,
            rBmpEx.IsAlpha() ? IMPSYSIMAGEITEM_ALPHA : ( rBmpEx.IsTransparent() ? IMPSYSIMAGEITEM_MASK : 0 ),
            mnSize );
}

void ImplImageBmp::Draw( OutputDevice* pOutDev,
                         const Point& rPos, DrawImageFlags nStyle,
                         const Size* pSize )
{
    if( pOutDev->IsDeviceOutputNecessary() )
    {
        const Point aSrcPos(0, 0);
        Size        aOutSize;

        aOutSize = ( pSize ? *pSize : pOutDev->PixelToLogic( maSize ) );

        if( nStyle & DrawImageFlags::Disable )
        {
            BitmapChecksum aChecksum = maBmpEx.GetChecksum();
            if (maBitmapChecksum != aChecksum)
            {
                maBitmapChecksum = aChecksum;
                ImplUpdateDisabledBmpEx();
            }
            pOutDev->DrawBitmapEx( rPos, aOutSize, aSrcPos, maSize, maDisabledBmpEx );
        }
        else
        {
            if( nStyle & ( DrawImageFlags::ColorTransform |
                           DrawImageFlags::Highlight | DrawImageFlags::Deactive | DrawImageFlags::SemiTransparent ) )
            {
                BitmapEx        aTmpBmpEx;
                const Rectangle aCropRect( aSrcPos, maSize );

                if( mpInfoAry[0] & ( IMPSYSIMAGEITEM_MASK | IMPSYSIMAGEITEM_ALPHA ) )
                    aTmpBmpEx = maBmpEx;
                else
                    aTmpBmpEx = maBmpEx.GetBitmap();

                aTmpBmpEx.Crop( aCropRect );

                Bitmap aTmpBmp( aTmpBmpEx.GetBitmap() );

                if( nStyle & ( DrawImageFlags::Highlight | DrawImageFlags::Deactive ) )
                {
                    BitmapWriteAccess* pAcc = aTmpBmp.AcquireWriteAccess();

                    if( pAcc )
                    {
                        const StyleSettings&    rSettings = pOutDev->GetSettings().GetStyleSettings();
                        Color                   aColor;
                        BitmapColor             aCol;
                        const long              nW = pAcc->Width();
                        const long              nH = pAcc->Height();
                        std::unique_ptr<sal_uInt8[]> pMapR(new sal_uInt8[ 256 ]);
                        std::unique_ptr<sal_uInt8[]> pMapG(new sal_uInt8[ 256 ]);
                        std::unique_ptr<sal_uInt8[]> pMapB(new sal_uInt8[ 256 ]);
                        long                    nX, nY;

                        if( nStyle & DrawImageFlags::Highlight )
                            aColor = rSettings.GetHighlightColor();
                        else
                            aColor = rSettings.GetDeactiveColor();

                        const sal_uInt8 cR = aColor.GetRed();
                        const sal_uInt8 cG = aColor.GetGreen();
                        const sal_uInt8 cB = aColor.GetBlue();

                        for( nX = 0L; nX < 256L; nX++ )
                        {
                            pMapR[ nX ] = (sal_uInt8) ( ( ( nY = ( nX + cR ) >> 1 ) > 255 ) ? 255 : nY );
                            pMapG[ nX ] = (sal_uInt8) ( ( ( nY = ( nX + cG ) >> 1 ) > 255 ) ? 255 : nY );
                            pMapB[ nX ] = (sal_uInt8) ( ( ( nY = ( nX + cB ) >> 1 ) > 255 ) ? 255 : nY );
                        }

                        if( pAcc->HasPalette() )
                        {
                            for( sal_uInt16 i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++ )
                            {
                                const BitmapColor& rCol = pAcc->GetPaletteColor( i );
                                aCol.SetRed( pMapR[ rCol.GetRed() ] );
                                aCol.SetGreen( pMapG[ rCol.GetGreen() ] );
                                aCol.SetBlue( pMapB[ rCol.GetBlue() ] );
                                pAcc->SetPaletteColor( i, aCol );
                            }
                        }
                        else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
                        {
                            for( nY = 0L; nY < nH; nY++ )
                            {
                                Scanline pScan = pAcc->GetScanline( nY );

                                for( nX = 0L; nX < nW; nX++ )
                                {
                                    *pScan = pMapB[ *pScan ]; pScan++;
                                    *pScan = pMapG[ *pScan ]; pScan++;
                                    *pScan = pMapR[ *pScan ]; pScan++;
                                }
                            }
                        }
                        else
                        {
                            for( nY = 0L; nY < nH; nY++ )
                            {
                                for( nX = 0L; nX < nW; nX++ )
                                {
                                    aCol = pAcc->GetPixel( nY, nX );
                                    aCol.SetRed( pMapR[ aCol.GetRed() ] );
                                    aCol.SetGreen( pMapG[ aCol.GetGreen() ] );
                                    aCol.SetBlue( pMapB[ aCol.GetBlue() ] );
                                    pAcc->SetPixel( nY, nX, aCol );
                                }
                            }
                        }

                        Bitmap::ReleaseAccess( pAcc );
                    }
                }

                if( nStyle & DrawImageFlags::SemiTransparent )
                {
                    if( aTmpBmpEx.IsTransparent()  )
                    {
                        Bitmap aAlphaBmp( aTmpBmpEx.GetAlpha().GetBitmap() );

                        aAlphaBmp.Adjust( 50 );
                        aTmpBmpEx = BitmapEx( aTmpBmp, AlphaMask( aAlphaBmp ) );
                    }
                    else
                    {
                        sal_uInt8 cErase = 128;
                        aTmpBmpEx = BitmapEx( aTmpBmp, AlphaMask( aTmpBmp.GetSizePixel(),  &cErase ) );
                    }
                }
                else
                {
                    if( aTmpBmpEx.IsAlpha() )
                        aTmpBmpEx = BitmapEx( aTmpBmp, aTmpBmpEx.GetAlpha() );
                    else if( aTmpBmpEx.IsTransparent() )
                        aTmpBmpEx = BitmapEx( aTmpBmp, aTmpBmpEx.GetMask() );
                }

                pOutDev->DrawBitmapEx( rPos, aOutSize, aTmpBmpEx );
            }
            else
            {
                const BitmapEx* pOutputBmp;

                if( pOutDev->GetOutDevType() == OUTDEV_WINDOW )
                {
                    ImplUpdateDisplayBmp( pOutDev );
                    pOutputBmp = mpDisplayBmp;
                }
                else
                    pOutputBmp = &maBmpEx;

                if( pOutputBmp )
                    pOutDev->DrawBitmapEx( rPos, aOutSize, aSrcPos, maSize, *pOutputBmp );
            }
        }
    }
}

void ImplImageBmp::ImplUpdateDisplayBmp( OutputDevice*
#if defined WNT
pOutDev
#endif
)
{
    if( !mpDisplayBmp && !maBmpEx.IsEmpty() )
    {
#if defined WNT
        if( !maBmpEx.IsAlpha() )
        {
            // FIXME: this looks like rather an obsolete code-path to me.
            const Bitmap aBmp( maBmpEx.GetBitmap().CreateDisplayBitmap( pOutDev ) );

            if( maBmpEx.IsTransparent() )
                mpDisplayBmp = new BitmapEx( aBmp, maBmpEx.GetMask().CreateDisplayBitmap( pOutDev ) );
            else
                mpDisplayBmp = new BitmapEx( aBmp );
        }
        else
#endif
            mpDisplayBmp = new BitmapEx( maBmpEx );
    }
}

void ImplImageBmp::ImplUpdateDisabledBmpEx()
{
    const Size aTotalSize( maBmpEx.GetSizePixel() );

    if( maDisabledBmpEx.IsEmpty() )
    {
        Bitmap      aGrey( aTotalSize, 8, &Bitmap::GetGreyPalette( 256 ) );
        AlphaMask   aGreyAlphaMask( aTotalSize );

        maDisabledBmpEx = BitmapEx( aGrey, aGreyAlphaMask );
    }

    Bitmap              aBmp( maBmpEx.GetBitmap() );
    BitmapReadAccess*   pBmp( aBmp.AcquireReadAccess() );
    AlphaMask           aBmpAlphaMask( maBmpEx.GetAlpha() );
    BitmapReadAccess*   pBmpAlphaMask( aBmpAlphaMask.AcquireReadAccess() );
    Bitmap              aGrey( maDisabledBmpEx.GetBitmap() );
    BitmapWriteAccess*  pGrey( aGrey.AcquireWriteAccess() );
    AlphaMask           aGreyAlphaMask( maDisabledBmpEx.GetAlpha() );
    BitmapWriteAccess*  pGreyAlphaMask( aGreyAlphaMask.AcquireWriteAccess() );

    if( pBmp && pBmpAlphaMask && pGrey && pGreyAlphaMask )
    {
        BitmapColor aGreyVal( 0 );
        BitmapColor aGreyAlphaMaskVal( 0 );

        const int nLeft = 0;
        const int nRight = nLeft + maSize.Width();
        const int nTop = 0;
        const int nBottom = nTop + maSize.Height();

        for( int nY = nTop; nY < nBottom; ++nY )
        {
            for( int nX = nLeft; nX < nRight; ++nX )
            {
                aGreyVal.SetIndex( pBmp->GetLuminance( nY, nX ) );
                pGrey->SetPixel( nY, nX, aGreyVal );

                const BitmapColor aBmpAlphaMaskVal( pBmpAlphaMask->GetPixel( nY, nX ) );

                aGreyAlphaMaskVal.SetIndex( static_cast< sal_uInt8 >( ::std::min( aBmpAlphaMaskVal.GetIndex() + 178ul, 255ul ) ) );
                pGreyAlphaMask->SetPixel( nY, nX, aGreyAlphaMaskVal );
            }
        }
    }

    Bitmap::ReleaseAccess( pBmp );
    aBmpAlphaMask.ReleaseAccess( pBmpAlphaMask );
    Bitmap::ReleaseAccess( pGrey );
    aGreyAlphaMask.ReleaseAccess( pGreyAlphaMask );

    maDisabledBmpEx = BitmapEx( aGrey, aGreyAlphaMask );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
