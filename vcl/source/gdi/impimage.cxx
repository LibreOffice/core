/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impimage.cxx,v $
 * $Revision: 1.24 $
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
#include <vcl/image.h>
#include <vcl/image.hxx>

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
                            USHORT nId, const BitmapEx &aBitmap )
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
                              USHORT nId, const BitmapEx &aBitmapEx )
{
    ImageAryData *pImg = new ImageAryData( aName, nId, aBitmapEx );
    maImages.push_back( pImg );
    if( aName.getLength() )
        maNameHash [ aName ] = pImg;
}

void ImplImageList::RemoveImage( USHORT nPos )
{
    ImageAryData *pImg = maImages[ nPos ];
    if( pImg->maName.getLength() )
        maNameHash.erase( pImg->maName );
    maImages.erase( maImages.begin() + nPos );
}

USHORT ImplImageList::GetImageCount() const
{
    return sal::static_int_cast< USHORT >( maImages.size() );
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

BOOL ImplImageData::IsEqual( const ImplImageData& rData )
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

void ImplImageBmp::Create( long nItemWidth, long nItemHeight, USHORT nInitSize )
{
    const Size aTotalSize( nInitSize * nItemWidth, nItemHeight );

    maBmpEx = Bitmap( aTotalSize, 24 );
    maDisabledBmp.SetEmpty();

    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    maSize = Size( nItemWidth, nItemHeight );
    mnSize = nInitSize;

    delete[] mpInfoAry;
    mpInfoAry = new BYTE[ mnSize ];
    memset( mpInfoAry, 0, mnSize );
}

// -----------------------------------------------------------------------

void ImplImageBmp::Create( const BitmapEx& rBmpEx, long nItemWidth, long nItemHeight, USHORT nInitSize )
{
    maBmpEx = rBmpEx;
    maDisabledBmp.SetEmpty();

    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    maSize = Size( nItemWidth, nItemHeight );
    mnSize = nInitSize;

    delete[] mpInfoAry;
    mpInfoAry = new BYTE[ mnSize ];
    memset( mpInfoAry,
            rBmpEx.IsAlpha() ? IMPSYSIMAGEITEM_ALPHA : ( rBmpEx.IsTransparent() ? IMPSYSIMAGEITEM_MASK : 0 ),
            mnSize );
}

// -----------------------------------------------------------------------

void ImplImageBmp::Expand( USHORT nGrowSize )
{
    const ULONG     nDX = nGrowSize * maSize.Width();
    const USHORT    nOldSize = mnSize;
    BYTE*           pNewAry = new BYTE[ mnSize = sal::static_int_cast<USHORT>(mnSize+nGrowSize) ];

    maBmpEx.Expand( nDX, 0UL );

    if( !maDisabledBmp.IsEmpty() )
        maDisabledBmp.Expand( nDX, 0UL );

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

void ImplImageBmp::Replace( USHORT nPos, USHORT nSrcPos )
{
    const Point     aSrcPos( nSrcPos * maSize.Width(), 0L ), aPos( nPos * maSize.Width(), 0L );
    const Rectangle aSrcRect( aSrcPos, maSize );
    const Rectangle aDstRect( aPos, maSize );

    maBmpEx.CopyPixel( aDstRect, aSrcRect );

    if( !maDisabledBmp.IsEmpty() )
        maDisabledBmp.CopyPixel( aDstRect, aSrcRect );

    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    mpInfoAry[ nPos ] = mpInfoAry[ nSrcPos ];
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( USHORT nPos, const ImplImageBmp& rImageBmp, USHORT nSrcPos )
{
    const Point     aSrcPos( nSrcPos * maSize.Width(), 0L ), aPos( nPos * maSize.Width(), 0L );
    const Rectangle aSrcRect( aSrcPos, maSize );
    const Rectangle aDstRect( aPos, maSize );

    maBmpEx.CopyPixel( aDstRect, aSrcRect, &rImageBmp.maBmpEx );

    ImplUpdateDisabledBmp( nPos );
    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    mpInfoAry[ nPos ] = rImageBmp.mpInfoAry[ nSrcPos ];
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( USHORT nPos, const BitmapEx& rBmpEx )
{
    const Point     aNullPos, aPos( nPos * maSize.Width(), 0L );
    const Rectangle aSrcRect( aNullPos, maSize );
    const Rectangle aDstRect( aPos, maSize );

    maBmpEx.CopyPixel( aDstRect, aSrcRect, &rBmpEx );

    ImplUpdateDisabledBmp( nPos );
    delete mpDisplayBmp;
    mpDisplayBmp = NULL;

    mpInfoAry[ nPos ] &= ~( IMPSYSIMAGEITEM_MASK | IMPSYSIMAGEITEM_ALPHA );
    mpInfoAry[ nPos ] |= ( rBmpEx.IsAlpha() ? IMPSYSIMAGEITEM_ALPHA : ( rBmpEx.IsTransparent() ? IMPSYSIMAGEITEM_MASK : 0 ) );
}

// -----------------------------------------------------------------------

void ImplImageBmp::ReplaceColors( const Color* pSrcColors, const Color* pDstColors, ULONG nColorCount )
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

BitmapEx ImplImageBmp::GetBitmapEx( USHORT nPosCount, USHORT* pPosAry ) const
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

    for( USHORT i = 0; i < nPosCount; i++ )
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

void ImplImageBmp::Draw( USHORT nPos, OutputDevice* pOutDev,
                         const Point& rPos, USHORT nStyle,
                         const Size* pSize )
{
    if( pOutDev->IsDeviceOutputNecessary() )
    {
        const Point aSrcPos( nPos * maSize.Width(), 0 );
        Size        aOutSize;

        aOutSize = ( pSize ? *pSize : pOutDev->PixelToLogic( maSize ) );

        if( nStyle & IMAGE_DRAW_DISABLE )
        {
            const Point             aOutPos1( rPos.X() + 1, rPos.Y() + 1 );
            const StyleSettings&    rSettings = pOutDev->GetSettings().GetStyleSettings();

            ImplUpdateDisabledBmp( -1 );

            pOutDev->DrawMask( aOutPos1, aOutSize, aSrcPos, maSize, maDisabledBmp, rSettings.GetLightColor() );
            pOutDev->DrawMask( rPos, aOutSize, aSrcPos, maSize, maDisabledBmp, rSettings.GetShadowColor() );
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
                        BYTE*                   pMapR = new BYTE[ 256 ];
                        BYTE*                   pMapG = new BYTE[ 256 ];
                        BYTE*                   pMapB = new BYTE[ 256 ];
                        long                    nX, nY;

                        if( nStyle & IMAGE_DRAW_HIGHLIGHT )
                            aColor = rSettings.GetHighlightColor();
                        else
                            aColor = rSettings.GetDeactiveColor();

                        const BYTE cR = aColor.GetRed();
                        const BYTE cG = aColor.GetGreen();
                        const BYTE cB = aColor.GetBlue();

                        for( nX = 0L; nX < 256L; nX++ )
                        {
                            pMapR[ nX ] = (BYTE) ( ( ( nY = ( nX + cR ) >> 1 ) > 255 ) ? 255 : nY );
                            pMapG[ nX ] = (BYTE) ( ( ( nY = ( nX + cG ) >> 1 ) > 255 ) ? 255 : nY );
                            pMapB[ nX ] = (BYTE) ( ( ( nY = ( nX + cB ) >> 1 ) > 255 ) ? 255 : nY );
                        }

                        if( pAcc->HasPalette() )
                        {
                            for( USHORT i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++ )
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
                        BYTE cErase = 128;
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
#if defined WIN || defined WNT
pOutDev
#endif
)
{
    if( !mpDisplayBmp && !maBmpEx.IsEmpty() )
    {
#if defined WIN || defined WNT
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

void ImplImageBmp::ImplUpdateDisabledBmp( int nPos )
{
    if( ( nPos >= 0 && !maDisabledBmp.IsEmpty() ) ||
        ( nPos < 0 && maDisabledBmp.IsEmpty() ) )
    {
        Bitmap aBmp( maBmpEx.GetBitmap() );
        Bitmap aMask;

        if( maBmpEx.IsTransparent() )
            aMask = maBmpEx.GetMask();
        else
        {
            aMask = aBmp;
            aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
        }

        if( maDisabledBmp.IsEmpty() )
            maDisabledBmp = Bitmap( aBmp.GetSizePixel(), 1 );

        BitmapReadAccess*   pAcc = aBmp.AcquireReadAccess();
        BitmapReadAccess*   pMsk = aMask.AcquireReadAccess();
        BitmapWriteAccess*  pDis = maDisabledBmp.AcquireWriteAccess();

        if( pAcc && pMsk && pDis )
        {
            const Color         aWhite( COL_WHITE );
            const Color         aBlack( COL_BLACK );
            const BitmapColor   aAccWhite( pAcc->GetBestMatchingColor( aWhite ) );
            const BitmapColor   aMskWhite( pMsk->GetBestMatchingColor( aWhite ) );
            const BitmapColor   aDisWhite( pDis->GetBestMatchingColor( aWhite ) );
            const BitmapColor   aDisBlack( pDis->GetBestMatchingColor( aBlack ) );
            long                nLeft, nTop, nRight, nBottom;
            long                nCurLeft, nCurRight;
            const long          nBlackThreshold = FRound( maSize.Width() * maSize.Height() * 0.10 );

            if( nPos >= 0 )
            {
                const Point aPos( nPos * maSize.Width(), 0 );

                nLeft = aPos.X();
                nTop = 0;
                nRight = nLeft + maSize.Width();
                nBottom = nTop + maSize.Height();
            }
            else
            {
                nLeft = nTop = 0L;
                nRight = pDis->Width();
                nBottom = pDis->Height();
            }

            nCurLeft = nLeft;
            nCurRight = nCurLeft + maSize.Width();

            while( nCurLeft < nRight )
            {
                sal_Int32 nBlackCount = 0;

                if( pAcc->GetScanlineFormat() == BMP_FORMAT_4BIT_MSN_PAL &&
                    pMsk->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL )
                {
                    // optimized version
                    const BYTE cAccTest = aAccWhite.GetIndex();
                    const BYTE cMskTest = aMskWhite.GetIndex();

                    for( long nY = nTop; nY < nBottom; nY++ )
                    {
                        Scanline pAccScan = pAcc->GetScanline( nY );
                        Scanline pMskScan = pMsk->GetScanline( nY );

                        for( long nX = nCurLeft; nX < nCurRight; nX++ )
                        {
                            if( ( cMskTest == ( pMskScan[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 ) ) ||
                                ( cAccTest == ( ( pAccScan[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f ) ) )
                            {
                                pDis->SetPixel( nY, nX, aDisWhite );
                            }
                            else
                            {
                                pDis->SetPixel( nY, nX, aDisBlack );
                                ++nBlackCount;
                            }
                        }
                    }
                }
                else if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL &&
                        pMsk->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL )
                {
                    // optimized version
                    const BYTE cAccTest = aAccWhite.GetIndex();
                    const BYTE cMskTest = aMskWhite.GetIndex();

                    for( long nY = nTop; nY < nBottom; nY++ )
                    {
                        Scanline pAccScan = pAcc->GetScanline( nY );
                        Scanline pMskScan = pMsk->GetScanline( nY );

                        for( long nX = nCurLeft; nX < nCurRight; nX++ )
                        {
                            if( ( cMskTest == ( pMskScan[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 ) ) ||
                                ( cAccTest == pAccScan[ nX ] ) )
                            {
                                pDis->SetPixel( nY, nX, aDisWhite );
                            }
                            else
                            {
                                pDis->SetPixel( nY, nX, aDisBlack );
                                ++nBlackCount;
                            }
                        }
                    }
                }
                else
                {
                    for( long nY = nTop; nY < nBottom; nY++ )
                    {
                        for( long nX = nCurLeft; nX < nCurRight; nX++ )
                        {
                            if( ( aMskWhite == pMsk->GetPixel( nY, nX ) ) ||
                                ( aAccWhite == pAcc->GetPixel( nY, nX ) ) )
                            {
                                pDis->SetPixel( nY, nX, aDisWhite );
                            }
                            else
                            {
                                pDis->SetPixel( nY, nX, aDisBlack );
                                ++nBlackCount;
                            }
                        }
                    }
                }

                if( nBlackCount < nBlackThreshold )
                {
                    // emergency solution if paint bitmap is mostly white
                    for( long nY = nTop; nY < nBottom; nY++ )
                    {
                        for( long nX = nCurLeft; nX < nCurRight; nX++ )
                        {
                            if( aMskWhite == pMsk->GetPixel( nY, nX ) )
                                pDis->SetPixel( nY, nX, aDisWhite );
                            else
                                pDis->SetPixel( nY, nX, aDisBlack );
                        }
                    }
                }

                nCurLeft += maSize.Width();
                nCurRight += maSize.Width();
            }
        }

        aBmp.ReleaseAccess( pAcc );
        aMask.ReleaseAccess( pMsk );
        maDisabledBmp.ReleaseAccess( pDis );
    }
}
