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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/outdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/alpha.hxx>
#include <vcl/window.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>
#include <vcl/image.hxx>

#include <image.h>

// -----------
// - Defines -
// -----------

#define IMPSYSIMAGEITEM_MASK        ( 0x01 )
#define IMPSYSIMAGEITEM_ALPHA       ( 0x02 )
#define DISA_ALL                    ( 0xffff )

// ----------------
// - ImageAryData -
// ----------------

ImageAryData::ImageAryData() :
    maName(),
    mnId( 0 ),
    maBitmapEx()
{
}

// -----------------------------------------------------------------------

ImageAryData::ImageAryData( const ImageAryData& rData ) :
    maName( rData.maName ),
    mnId( rData.mnId ),
    maBitmapEx( rData.maBitmapEx )
{
}

ImageAryData::ImageAryData( const rtl::OUString &aName,
                            sal_uInt16 nId, const BitmapEx &aBitmap )
        : maName( aName ), mnId( nId ), maBitmapEx( aBitmap )
{
}

// -----------------------------------------------------------------------

ImageAryData::~ImageAryData()
{
}

// -----------------------------------------------------------------------

ImageAryData& ImageAryData::operator=( const ImageAryData& rData )
{
    maName = rData.maName;
    mnId = rData.mnId;
    maBitmapEx = rData.maBitmapEx;

    return *this;
}

// -----------------
// - ImplImageList -
// -----------------

ImplImageList::ImplImageList()
{
}

ImplImageList::ImplImageList( const ImplImageList &aSrc ) :
    maPrefix( aSrc.maPrefix ),
    maImageSize( aSrc.maImageSize ),
    mnRefCount( 1 )
{
    maImages.reserve( aSrc.maImages.size() );
    for ( ImageAryDataVec::const_iterator aIt = aSrc.maImages.begin(), aEnd = aSrc.maImages.end(); aIt != aEnd; ++aIt )
    {
        ImageAryData* pAryData = new ImageAryData( **aIt );
        maImages.push_back( pAryData );
        if( pAryData->maName.getLength() )
            maNameHash [ pAryData->maName ] = pAryData;
    }
}

ImplImageList::~ImplImageList()
{
    for ( ImageAryDataVec::iterator aIt = maImages.begin(), aEnd = maImages.end(); aIt != aEnd; ++aIt )
        delete *aIt;
}

void ImplImageList::AddImage( const ::rtl::OUString &aName,
                              sal_uInt16 nId, const BitmapEx &aBitmapEx )
{
    ImageAryData *pImg = new ImageAryData( aName, nId, aBitmapEx );
    maImages.push_back( pImg );
    if( aName.getLength() )
        maNameHash [ aName ] = pImg;
}

void ImplImageList::RemoveImage( sal_uInt16 nPos )
{
    ImageAryData *pImg = maImages[ nPos ];
    if( pImg->maName.getLength() )
        maNameHash.erase( pImg->maName );
    maImages.erase( maImages.begin() + nPos );
}

sal_uInt16 ImplImageList::GetImageCount() const
{
    return sal::static_int_cast< sal_uInt16 >( maImages.size() );
}

// -----------------
// - ImplImageData -
// -----------------

ImplImageData::ImplImageData( const BitmapEx& rBmpEx ) :
    mpImageBitmap( NULL ),
    maBmpEx( rBmpEx )
{
}

// -----------------------------------------------------------------------

ImplImageData::~ImplImageData()
{
    delete mpImageBitmap;
}

// -----------------
// - ImplImageData -
// -----------------

sal_Bool ImplImageData::IsEqual( const ImplImageData& rData )
{
    return( maBmpEx == rData.maBmpEx );
}

// -------------
// - ImplImage -
// -------------

ImplImage::ImplImage()
{
}

// ------------------------------------------------------------------------------

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

// ----------------
// - ImplImageBmp -
// ----------------

ImplImageBmp::ImplImageBmp() :
    mpDisplayBmp( NULL ),
    mpInfoAry( NULL ),
    mnSize( 0 )
{
}

// -------------
// - ImplImage -
// -------------

ImplImageBmp::~ImplImageBmp()
{
    delete[] mpInfoAry;
    delete mpDisplayBmp;
}

// -----------------------------------------------------------------------

void ImplImageBmp::Create( long nItemWidth, long nItemHeight, sal_uInt16 nInitSize )
{
    const Size aTotalSize( nInitSize * nItemWidth, nItemHeight );

    maBmpEx = Bitmap( aTotalSize, 24 );
    maDisabledBmpEx.SetEmpty();

    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    maSize = Size( nItemWidth, nItemHeight );
    mnSize = nInitSize;

    delete[] mpInfoAry;
    mpInfoAry = new sal_uInt8[ mnSize ];
    memset( mpInfoAry, 0, mnSize );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void ImplImageBmp::Expand( sal_uInt16 nGrowSize )
{
    const sal_uLong     nDX = nGrowSize * maSize.Width();
    const sal_uInt16    nOldSize = mnSize;
    sal_uInt8*          pNewAry = new sal_uInt8[ mnSize = sal::static_int_cast<sal_uInt16>(mnSize+nGrowSize) ];

    maBmpEx.Expand( nDX, 0UL );

    if( !maDisabledBmpEx.IsEmpty() )
        maDisabledBmpEx.Expand( nDX, 0UL );

    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    memset( pNewAry, 0, mnSize );
    memcpy( pNewAry, mpInfoAry, nOldSize );
    delete[] mpInfoAry;
    mpInfoAry = pNewAry;
}

// -----------------------------------------------------------------------

void ImplImageBmp::Invert()
{
    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    maBmpEx.Invert();
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( sal_uInt16 nPos, sal_uInt16 nSrcPos )
{
    const Point     aSrcPos( nSrcPos * maSize.Width(), 0L ), aPos( nPos * maSize.Width(), 0L );
    const Rectangle aSrcRect( aSrcPos, maSize );
    const Rectangle aDstRect( aPos, maSize );

    maBmpEx.CopyPixel( aDstRect, aSrcRect );

    if( !maDisabledBmpEx.IsEmpty() )
        maDisabledBmpEx.CopyPixel( aDstRect, aSrcRect );

    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    mpInfoAry[ nPos ] = mpInfoAry[ nSrcPos ];
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( sal_uInt16 nPos, const ImplImageBmp& rImageBmp, sal_uInt16 nSrcPos )
{
    const Point     aSrcPos( nSrcPos * maSize.Width(), 0L ), aPos( nPos * maSize.Width(), 0L );
    const Rectangle aSrcRect( aSrcPos, maSize );
    const Rectangle aDstRect( aPos, maSize );

    maBmpEx.CopyPixel( aDstRect, aSrcRect, &rImageBmp.maBmpEx );

    ImplUpdateDisabledBmpEx( nPos );
    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    mpInfoAry[ nPos ] = rImageBmp.mpInfoAry[ nSrcPos ];
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( sal_uInt16 nPos, const BitmapEx& rBmpEx )
{
    const Point     aNullPos, aPos( nPos * maSize.Width(), 0L );
    const Rectangle aSrcRect( aNullPos, maSize );
    const Rectangle aDstRect( aPos, maSize );

    maBmpEx.CopyPixel( aDstRect, aSrcRect, &rBmpEx );

    ImplUpdateDisabledBmpEx( nPos );
    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    mpInfoAry[ nPos ] &= ~( IMPSYSIMAGEITEM_MASK | IMPSYSIMAGEITEM_ALPHA );
    mpInfoAry[ nPos ] |= ( rBmpEx.IsAlpha() ? IMPSYSIMAGEITEM_ALPHA : ( rBmpEx.IsTransparent() ? IMPSYSIMAGEITEM_MASK : 0 ) );
}

// -----------------------------------------------------------------------

void ImplImageBmp::ReplaceColors( const Color* pSrcColors, const Color* pDstColors, sal_uLong nColorCount )
{
    maBmpEx.Replace( pSrcColors, pDstColors, nColorCount );
    delete mpDisplayBmp;
    mpDisplayBmp = NULL;
}

// -----------------------------------------------------------------------

void ImplImageBmp::ColorTransform( BmpColorMode eColorMode )
{
    maBmpEx = maBmpEx.GetColorTransformedBitmapEx( eColorMode );
    delete mpDisplayBmp;
    mpDisplayBmp = NULL;
}

// -----------------------------------------------------------------------

BitmapEx ImplImageBmp::GetBitmapEx( sal_uInt16 nPosCount, sal_uInt16* pPosAry ) const
{
    const Bitmap    aNewBmp( Size( nPosCount * maSize.Width(), maSize.Height() ),  maBmpEx.GetBitmap().GetBitCount() );
    BitmapEx        aRet;
    if( maBmpEx.IsAlpha() )
    {
        // initialize target bitmap with an empty alpha mask
        // which allows for using an optimized copypixel later on (see AlphaMask::CopyPixel)
        // that avoids palette lookups
        AlphaMask aAlpha( Size( nPosCount * maSize.Width(), maSize.Height() ) );
        aRet = BitmapEx( aNewBmp, aAlpha );
    }
    else
        aRet  = BitmapEx( aNewBmp );

    for( sal_uInt16 i = 0; i < nPosCount; i++ )
    {
        const Point     aSrcPos( pPosAry[ i ] * maSize.Width(), 0L );
        const Point     aPos( i * maSize.Width(), 0L );
        const Rectangle aSrcRect( aSrcPos, maSize );
        const Rectangle aDstRect( aPos, maSize );

        aRet.CopyPixel( aDstRect, aSrcRect, &maBmpEx );
    }

    return aRet;
}

// -----------------------------------------------------------------------

void ImplImageBmp::Draw( sal_uInt16 nPos, OutputDevice* pOutDev,
                         const Point& rPos, sal_uInt16 nStyle,
                         const Size* pSize )
{
    if( pOutDev->IsDeviceOutputNecessary() )
    {
        const Point aSrcPos( nPos * maSize.Width(), 0 );
        Size        aOutSize;

        aOutSize = ( pSize ? *pSize : pOutDev->PixelToLogic( maSize ) );

        if( nStyle & IMAGE_DRAW_DISABLE )
        {
            ImplUpdateDisabledBmpEx( nPos);
            pOutDev->DrawBitmapEx( rPos, aOutSize, aSrcPos, maSize, maDisabledBmpEx );
        }
        else
        {
            if( nStyle & ( IMAGE_DRAW_COLORTRANSFORM |
                           IMAGE_DRAW_MONOCHROME_BLACK | IMAGE_DRAW_MONOCHROME_WHITE |
                           IMAGE_DRAW_HIGHLIGHT | IMAGE_DRAW_DEACTIVE | IMAGE_DRAW_SEMITRANSPARENT ) )
            {
                BitmapEx        aTmpBmpEx;
                const Rectangle aCropRect( aSrcPos, maSize );

                if( mpInfoAry[ nPos ] & ( IMPSYSIMAGEITEM_MASK | IMPSYSIMAGEITEM_ALPHA ) )
                    aTmpBmpEx = maBmpEx;
                else
                    aTmpBmpEx = maBmpEx.GetBitmap();

                aTmpBmpEx.Crop( aCropRect );

                if( nStyle & ( IMAGE_DRAW_COLORTRANSFORM | IMAGE_DRAW_MONOCHROME_BLACK | IMAGE_DRAW_MONOCHROME_WHITE ) )
                {
                    const BmpColorMode eMode = ( nStyle & IMAGE_DRAW_COLORTRANSFORM ) ? BMP_COLOR_HIGHCONTRAST :
                                                ( ( nStyle & IMAGE_DRAW_MONOCHROME_BLACK ) ? BMP_COLOR_MONOCHROME_BLACK : BMP_COLOR_MONOCHROME_WHITE );

                    aTmpBmpEx = aTmpBmpEx.GetColorTransformedBitmapEx( eMode );
                }

                Bitmap aTmpBmp( aTmpBmpEx.GetBitmap() );

                if( nStyle & ( IMAGE_DRAW_HIGHLIGHT | IMAGE_DRAW_DEACTIVE ) )
                {
                    BitmapWriteAccess* pAcc = aTmpBmp.AcquireWriteAccess();

                    if( pAcc )
                    {
                        const StyleSettings&    rSettings = pOutDev->GetSettings().GetStyleSettings();
                        Color                   aColor;
                        BitmapColor             aCol;
                        const long              nW = pAcc->Width();
                        const long              nH = pAcc->Height();
                        sal_uInt8*                  pMapR = new sal_uInt8[ 256 ];
                        sal_uInt8*                  pMapG = new sal_uInt8[ 256 ];
                        sal_uInt8*                  pMapB = new sal_uInt8[ 256 ];
                        long                    nX, nY;

                        if( nStyle & IMAGE_DRAW_HIGHLIGHT )
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

                        delete[] pMapR;
                        delete[] pMapG;
                        delete[] pMapB;
                        aTmpBmp.ReleaseAccess( pAcc );
                    }
                }

                if( nStyle & IMAGE_DRAW_SEMITRANSPARENT )
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
                    else if( aTmpBmpEx.IsAlpha() )
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

// -----------------------------------------------------------------------

void ImplImageBmp::ImplUpdateDisplayBmp( OutputDevice*
#if defined WNT
pOutDev
#endif
)
{
    if( !mpDisplayBmp && !maBmpEx.IsEmpty() )
    {
#if defined WNT
        if( maBmpEx.IsAlpha() )
            mpDisplayBmp = new BitmapEx( maBmpEx );
        else
        {
            const Bitmap aBmp( maBmpEx.GetBitmap().CreateDisplayBitmap( pOutDev ) );

            if( maBmpEx.IsTransparent() )
                mpDisplayBmp = new BitmapEx( aBmp, maBmpEx.GetMask().CreateDisplayBitmap( pOutDev ) );
            else
                mpDisplayBmp = new BitmapEx( aBmp );
        }
#else
        mpDisplayBmp = new BitmapEx( maBmpEx );
#endif
    }
}

// -----------------------------------------------------------------------

void ImplImageBmp::ImplUpdateDisabledBmpEx( int nPos )
{
    const Size aTotalSize( maBmpEx.GetSizePixel() );

    if( maDisabledBmpEx.IsEmpty() )
    {
        Bitmap      aGrey( aTotalSize, 8, &Bitmap::GetGreyPalette( 256 ) );
        AlphaMask   aGreyAlphaMask( aTotalSize );

        maDisabledBmpEx = BitmapEx( aGrey, aGreyAlphaMask );
        nPos = -1;
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
        const Point aPos( ( nPos < 0 ) ? 0 : ( nPos * maSize.Width() ), 0 );
        const int  nLeft = aPos.X(), nRight = nLeft + ( ( nPos < 0 ) ? aTotalSize.Width() : maSize.Width() );
        const int  nTop = aPos.Y(), nBottom = nTop + maSize.Height();

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

    aBmp.ReleaseAccess( pBmp );
    aBmpAlphaMask.ReleaseAccess( pBmpAlphaMask );
    aGrey.ReleaseAccess( pGrey );
    aGreyAlphaMask.ReleaseAccess( pGreyAlphaMask );

    maDisabledBmpEx = BitmapEx( aGrey, aGreyAlphaMask );
}
