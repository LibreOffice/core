#/*************************************************************************
 *
 *  $RCSfile: image.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 17:47:23 $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _SV_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_IMPIMAGETREE_H
#include <impimagetree.hxx>
#endif
#ifndef _SV_IMAGE_H
#include <image.h>
#endif
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif

DBG_NAME( Image );
DBG_NAME( ImageList );

#define IMAGE_FILE_VERSION 100

// ---------
// - Image -
// ---------

Image::Image() :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );
}

// -----------------------------------------------------------------------

Image::Image( const ResId& rResId ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    rResId.SetRT( RSC_IMAGE );

    ResMgr* pResMgr = rResId.GetResMgr();

    if( !pResMgr )
        pResMgr = Resource::GetResManager();

    if( pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        BitmapEx    aBmpEx;
        USHORT      nObjMask = pResMgr->ReadShort();

        if( nObjMask & RSC_IMAGE_IMAGEBITMAP )
        {
            aBmpEx = BitmapEx( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }

        if( !aBmpEx.IsEmpty() )
        {
            if( nObjMask & RSC_IMAGE_MASKBITMAP )
            {
                if( aBmpEx.GetTransparentType() == TRANSPARENT_NONE )
                {
                    const Bitmap aMaskBitmap( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
                    aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMaskBitmap );
                }

                pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            }

            if( nObjMask & RSC_IMAGE_MASKCOLOR )
            {
                if( aBmpEx.GetTransparentType() == TRANSPARENT_NONE )
                {
                    const Color aMaskColor( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
                    aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMaskColor );
                }

                pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            }

            ImplInit( aBmpEx );
        }
    }
    else
    {
        DBG_ERROR( "Image::Image( const ResId& rResId ): No resource!" );
    }
}

// -----------------------------------------------------------------------

Image::Image( const Image& rImage ) :
    mpImplData( rImage.mpImplData )
{
    DBG_CTOR( Image, NULL );

    if( mpImplData )
        ++mpImplData->mnRefCount;
}

// -----------------------------------------------------------------------

Image::Image( const BitmapEx& rBitmapEx ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    ImplInit( rBitmapEx );
}

// -----------------------------------------------------------------------

Image::Image( const Bitmap& rBitmap ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    ImplInit( rBitmap );
}

// -----------------------------------------------------------------------

Image::Image( const Bitmap& rBitmap, const Bitmap& rMaskBitmap ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    const BitmapEx aBmpEx( rBitmap, rMaskBitmap );

    ImplInit( aBmpEx );
}

// -----------------------------------------------------------------------

Image::Image( const Bitmap& rBitmap, const Color& rColor ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    const BitmapEx aBmpEx( rBitmap, rColor );

    ImplInit( aBmpEx );
}

// -----------------------------------------------------------------------

Image::~Image()
{
    DBG_DTOR( Image, NULL );

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;
}

// -----------------------------------------------------------------------

void Image::ImplInit( const BitmapEx& rBmpEx )
{
    if( !rBmpEx.IsEmpty() )
    {
        mpImplData = new ImplImage;
        mpImplData->mnRefCount = 1;

        if( rBmpEx.GetTransparentType() == TRANSPARENT_NONE )
        {
            mpImplData->meType = IMAGETYPE_BITMAP;
            mpImplData->mpData = new Bitmap( rBmpEx.GetBitmap() );
        }
        else
        {
            mpImplData->meType = IMAGETYPE_IMAGE;
            mpImplData->mpData = new ImplImageData( rBmpEx );
        }
    }
}

// -----------------------------------------------------------------------

Size Image::GetSizePixel() const
{
    DBG_CHKTHIS( Image, NULL );

    Size aRet;

    if( mpImplData )
    {
        switch( mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
                aRet = static_cast< Bitmap* >( mpImplData->mpData )->GetSizePixel();
            break;

            case IMAGETYPE_IMAGE:
                aRet = static_cast< ImplImageData* >( mpImplData->mpData )->maBmpEx.GetSizePixel();
            break;

            case IMAGETYPE_IMAGEREF:
                aRet = static_cast< ImplImageRefData* >( mpImplData->mpData )->mpImplData->maImageSize;
            break;
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

BitmapEx Image::GetBitmapEx() const
{
    DBG_CHKTHIS( Image, NULL );

    BitmapEx aRet;

    if( mpImplData )
    {
        switch( mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
                aRet = *static_cast< Bitmap* >( mpImplData->mpData );
            break;

            case IMAGETYPE_IMAGE:
                aRet = static_cast< ImplImageData* >( mpImplData->mpData )->maBmpEx;
            break;

            case IMAGETYPE_IMAGEREF:
            {
                ImplImageRefData* pData = static_cast< ImplImageRefData* >( mpImplData->mpData );

                aRet = pData->mpImplData->mpImageBitmap->GetBitmapEx( 1, &pData->mnIndex );
            }
            break;
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

Image Image::GetColorTransformedImage( ImageColorTransform eColorTransform ) const
{
    DBG_CHKTHIS( Image, NULL );

    Image aRet;

    if( IMAGECOLORTRANSFORM_HIGHCONTRAST == eColorTransform )
    {
        BitmapEx aBmpEx( GetBitmapEx() );

        if( !aBmpEx.IsEmpty() )
        {
            Color*  pSrcColors = NULL;
            Color*  pDstColors = NULL;
            ULONG   nColorCount = 0;

            Image::GetColorTransformArrays( eColorTransform, pSrcColors, pDstColors, nColorCount );

            if( nColorCount && pSrcColors && pDstColors )
            {
                aBmpEx.Replace( pSrcColors, pDstColors, nColorCount );
                aRet = Image( aBmpEx );
            }

            delete[] pSrcColors;
            delete[] pDstColors;
        }
    }
    else if( IMAGECOLORTRANSFORM_MONOCHROME_BLACK == eColorTransform ||
             IMAGECOLORTRANSFORM_MONOCHROME_WHITE == eColorTransform  )
    {
        BitmapEx aBmpEx( GetBitmapEx() );

        if( !aBmpEx.IsEmpty() )
            aRet = Image( aBmpEx.GetColorTransformedBitmapEx( ( BmpColorMode )( eColorTransform ) ) );
    }

    if( !aRet )
        aRet = *this;

    return aRet;
}

// -----------------------------------------------------------------------

void Image::Invert()
{
    BitmapEx aInvertedBmp( GetBitmapEx() );
    aInvertedBmp.Invert();
    *this = aInvertedBmp;
}

// -----------------------------------------------------------------------

void Image::GetColorTransformArrays( ImageColorTransform eColorTransform,
                                     Color*& rpSrcColor, Color*& rpDstColor, ULONG& rColorCount )
{
    if( IMAGECOLORTRANSFORM_HIGHCONTRAST == eColorTransform )
    {
        rpSrcColor = new Color[ 4 ];
        rpDstColor = new Color[ 4 ];
        rColorCount = 4;

        rpSrcColor[ 0 ] = Color( COL_BLACK );
        rpDstColor[ 0 ] = Color( COL_WHITE );

        rpSrcColor[ 1 ] = Color( COL_WHITE );
        rpDstColor[ 1 ] = Color( COL_BLACK );

        rpSrcColor[ 2 ] = Color( COL_BLUE );
        rpDstColor[ 2 ] = Color( COL_WHITE );

        rpSrcColor[ 3 ] = Color( COL_LIGHTBLUE );
        rpDstColor[ 3 ] = Color( COL_WHITE );
    }
    else
    {
        rpSrcColor = rpDstColor = NULL;
        rColorCount = 0;
    }
}

// -----------------------------------------------------------------------

Image& Image::operator=( const Image& rImage )
{
    DBG_CHKTHIS( Image, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );

    if( rImage.mpImplData )
        ++rImage.mpImplData->mnRefCount;

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;

    mpImplData = rImage.mpImplData;

    return *this;
}

// -----------------------------------------------------------------------

BOOL Image::operator==( const Image& rImage ) const
{
    DBG_CHKTHIS( Image, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );

    BOOL bRet;

    if( rImage.mpImplData == mpImplData )
        bRet = true;
    else if( !rImage.mpImplData || !mpImplData )
        bRet = false;
    else if( rImage.mpImplData->mpData == mpImplData->mpData )
        bRet = true;
    else if( rImage.mpImplData->meType == mpImplData->meType )
    {
        switch( mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
                bRet = ( *static_cast< Bitmap* >( rImage.mpImplData->mpData ) == *static_cast< Bitmap* >( mpImplData->mpData ) );
            break;

            case IMAGETYPE_IMAGE:
                bRet = static_cast< ImplImageData* >( rImage.mpImplData->mpData )->IsEqual( *static_cast< ImplImageData* >( mpImplData->mpData ) );
            break;

            case IMAGETYPE_IMAGEREF:
                bRet = static_cast< ImplImageRefData* >( rImage.mpImplData->mpData )->IsEqual( *static_cast< ImplImageRefData* >( mpImplData->mpData ) );
            break;

            default:
                bRet = false;
            break;
        }
    }

    return bRet;
}

// -------------
// - ImageList -
// -------------

ImageList::ImageList( USHORT nInit, USHORT nGrow ) :
    mpImplData( NULL ),
    mnInitSize( nInit ),
    mnGrowSize( nGrow )
{
    DBG_CTOR( ImageList, NULL );
}

// -----------------------------------------------------------------------

ImageList::ImageList( const ResId& rResId ) :
    mpImplData( NULL ),
    mnInitSize( 1 ),
    mnGrowSize( 4 )
{
    DBG_CTOR( ImageList, NULL );

    rResId.SetRT( RSC_IMAGELIST );

    ResMgr* pResMgr = rResId.GetResMgr();

    if( !pResMgr )
        pResMgr = Resource::GetResManager();

    if( pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        static ImplImageTreeSingletonRef    aImageTree;
        USHORT                              nObjMask = pResMgr->ReadShort();
        const String                        aPrefix( pResMgr->ReadString() );
        Color                               aMaskColor;
        BitmapEx                            aBmpEx;

        if( nObjMask & RSC_IMAGE_MASKCOLOR )
               aMaskColor = Color( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );

        pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );

        if( nObjMask & RSC_IMAGELIST_IDLIST )
        {
            const USHORT nCount = pResMgr->ReadShort();
            for( int i = 0; i < nCount; ++i )
                pResMgr->ReadShort();
        }

        ::rtl::OUString aResMgrName( pResMgr->GetFileName() ), aUserImage;
        sal_Int32       nPos = aResMgrName.lastIndexOf( '\\' );
        sal_Int32       nCount = pResMgr->ReadShort();
        USHORT*         pIdAry = new USHORT[ nCount ];
        String*         pStringAry = new String[ nCount ];

        // load file entry list
        for( sal_Int32 i = 0; i < nCount; ++i )
        {
            pStringAry[ i ] = pResMgr->ReadString();
            pIdAry[ i ] = static_cast< USHORT >( pResMgr->ReadLong() );
        }

        // try to load cached image first
        if( -1 == nPos )
            nPos = aResMgrName.lastIndexOf( '/' );

        if( -1 != nPos++ )
        {
            const sal_Int32 nSecondPos = aResMgrName.lastIndexOf( '.' );
            aUserImage = aResMgrName.copy( nPos, ( ( -1 != nSecondPos ) ? nSecondPos : aResMgrName.getLength() ) - nPos );
        }

        if( !aImageTree->loadImage( aUserImage += String::CreateFromInt32( rResId.GetId() ), aBmpEx ) )
        {
            BitmapEx    aWorkBmpEx;
            Size        aItemSizePixel;
            bool        bInit = false;

            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                if( aImageTree->loadImage( pStringAry[ i ], aWorkBmpEx ) )
                {
                    const Size aWorkSizePixel( aWorkBmpEx.GetSizePixel() );

                    if( !bInit )
                    {
                        aItemSizePixel = aWorkSizePixel;
                        aBmpEx = Bitmap( Size( aWorkSizePixel.Width() * nCount, aWorkSizePixel.Height() ), 24 );
                        bInit = true;
                    }

                    DBG_ASSERT( aItemSizePixel == aWorkSizePixel, "Differerent dimensions in ItemList images" );

                    const Rectangle aRectDst( Point( aItemSizePixel.Width() * i, 0 ), aItemSizePixel );
                    const Rectangle aRectSrc( Point( 0, 0 ), aWorkSizePixel );

                    aBmpEx.CopyPixel( aRectDst, aRectSrc, &aWorkBmpEx );
                }
#ifdef DBG_UTIL
                else
                {
                    ByteString aErrorStr( "ImageList::ImageList( const ResId& rResId ): could not load image <" );
                    DBG_ERROR( ( ( aErrorStr += ByteString( pStringAry[ i ], RTL_TEXTENCODING_ASCII_US ) ) += '>' ).GetBuffer() );
                }
#endif
            }

            if( !aBmpEx.IsEmpty() )
                aImageTree->addUserImage( aUserImage, aBmpEx );
        }

        if( !aBmpEx.IsEmpty() && !aBmpEx.IsTransparent() && ( nObjMask & RSC_IMAGE_MASKCOLOR ) )
            aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMaskColor );

        if( nObjMask & RSC_IMAGELIST_IDCOUNT )
            pResMgr->ReadShort();

        ImplInit( aBmpEx, nCount, pIdAry, NULL, 4 );

        delete[] pIdAry;
        delete[] pStringAry;
    }
}

// -----------------------------------------------------------------------

ImageList::ImageList( const ::std::vector< ::rtl::OUString >& rNameVector, const Color* pMaskColor  ) :
    mpImplData( NULL ),
    mnInitSize( 1 ),
    mnGrowSize( 4 )
{
    DBG_CTOR( ImageList, NULL );

    static ImplImageTreeSingletonRef    aImageTree;
    ::rtl::OUString                     aProjectName;
    BitmapEx                            aBmpEx, aWorkBmpEx;
    Size                                aItemSizePixel;
    bool                                bInit = false;

    for( int i = 0; i < rNameVector.size(); ++i )
    {
        if( aImageTree->loadImage( rNameVector[ i ], aWorkBmpEx ) )
        {
            const Size aWorkSizePixel( aWorkBmpEx.GetSizePixel() );

            if( !bInit )
            {
                aItemSizePixel = aWorkSizePixel;
                aBmpEx = Bitmap( Size( aWorkSizePixel.Width() * rNameVector.size(), aWorkSizePixel.Height() ), 24 );
                bInit = true;
            }

            DBG_ASSERT( aItemSizePixel == aWorkSizePixel, "Differerent dimensions in ItemList images" );

            const Rectangle aRectDst( Point( aItemSizePixel.Width() * i, 0 ), aItemSizePixel );
            const Rectangle aRectSrc( Point( 0, 0 ), aWorkSizePixel );

            aBmpEx.CopyPixel( aRectDst, aRectSrc, &aWorkBmpEx );
        }
#ifdef DBG_UTIL
        else
        {
            ByteString aErrorStr( "ImageList::ImageList( const ::std::vector< ::rtl::OUString >& rNameVector, const Color* pMaskColor  ): could not load image <" );
            DBG_ERROR( ( ( aErrorStr += ByteString( String( rNameVector[ i ] ), RTL_TEXTENCODING_ASCII_US ) ) += '>' ).GetBuffer() );
        }
#endif
    }

    if( !aBmpEx.IsEmpty() && !aBmpEx.IsTransparent() && pMaskColor )
        aBmpEx = BitmapEx( aBmpEx.GetBitmap(), *pMaskColor );

    ImplInit( aBmpEx, static_cast< USHORT >( rNameVector.size() ), NULL, &rNameVector, 4 );
}

// -----------------------------------------------------------------------

ImageList::ImageList( const ImageList& rImageList ) :
    mpImplData( rImageList.mpImplData ),
    mnInitSize( rImageList.mnInitSize ),
    mnGrowSize( rImageList.mnGrowSize )
{
    DBG_CTOR( ImageList, NULL );

    if( mpImplData )
        ++mpImplData->mnRefCount;
}

// -----------------------------------------------------------------------

ImageList::ImageList( const BitmapEx& rBitmapEx,
                      USHORT nInit, USHORT* pIdAry, USHORT nGrow ) :
    mpImplData( NULL ),
    mnInitSize( nInit ),
    mnGrowSize( nGrow )
{
    DBG_CTOR( ImageList, NULL );

    ImplInit( rBitmapEx, nInit, pIdAry, NULL, nGrow );
}

// -----------------------------------------------------------------------

ImageList::ImageList( const Bitmap& rBitmap,
                      USHORT nInit, USHORT* pIdAry, USHORT nGrow ) :
    mpImplData( NULL ),
    mnInitSize( nInit ),
    mnGrowSize( nGrow )
{
    DBG_CTOR( ImageList, NULL );

    ImplInit( rBitmap, nInit, pIdAry, NULL, nGrow );
}

// -----------------------------------------------------------------------

ImageList::ImageList( const Bitmap& rBitmap, const Bitmap& rMaskBmp,
                      USHORT nInit, USHORT* pIdAry, USHORT nGrow ) :
    mpImplData( NULL ),
    mnInitSize( nInit ),
    mnGrowSize( nGrow )
{
    DBG_CTOR( ImageList, NULL );

    const BitmapEx aBmpEx( rBitmap, rMaskBmp );

    ImplInit( aBmpEx, nInit, pIdAry, NULL, nGrow );
}

// -----------------------------------------------------------------------

ImageList::ImageList( const Bitmap& rBitmap, const Color& rColor,
                      USHORT nInit, USHORT* pIdAry, USHORT nGrow )
{
    DBG_CTOR( ImageList, NULL );

    const BitmapEx aBmpEx( rBitmap, rColor );

    ImplInit( aBmpEx, nInit, pIdAry, NULL, nGrow );
}

// -----------------------------------------------------------------------

ImageList::~ImageList()
{
    DBG_DTOR( ImageList, NULL );

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) && ( 0 == mpImplData->mnIRefCount ) )
        delete mpImplData;
}

// -----------------------------------------------------------------------

void ImageList::ImplInit( const BitmapEx& rBitmapEx,
                          USHORT nInit, const USHORT* pIdAry,
                          const ::std::vector< ::rtl::OUString >* pNames,
                          USHORT nGrow  )
{
    if( !nInit )
    {
        mpImplData  = NULL;
        mnInitSize = 1;
        mnGrowSize = nGrow;
    }
    else
    {
        DBG_ASSERT( !nInit || rBitmapEx.GetSizePixel().Width(), "ImageList::ImageList(): nInitSize != 0 and BmpSize.Width() == 0" );
        DBG_ASSERT( (rBitmapEx.GetSizePixel().Width() % nInit) == 0, "ImageList::ImageList(): BmpSize % nInitSize != 0" );

        Size aBmpSize( rBitmapEx.GetSizePixel() );

        mpImplData = new ImplImageList;
        mnInitSize = nInit;
        mnGrowSize = nGrow;

        mpImplData->mnRefCount = 1;
        mpImplData->mnIRefCount = 0;
        mpImplData->mnCount = nInit;
        mpImplData->mnRealCount = nInit;
        mpImplData->mnArySize = nInit;
        mpImplData->mpAry = new ImageAryData[nInit];
        mpImplData->maImageSize = Size( aBmpSize.Width() / nInit, aBmpSize.Height() );

        for( USHORT i = 0; i < nInit; i++ )
        {
            mpImplData->mpAry[ i ].mnId = pIdAry ? pIdAry[ i ] : ( i + 1 );
            mpImplData->mpAry[ i ].mnRefCount = 1;

            if( pNames && ( i < pNames->size() ) )
                mpImplData->mpAry[ i ].maName = (*pNames)[ i ];
        }

        mpImplData->mpImageBitmap = new ImplImageBmp;
        mpImplData->mpImageBitmap->Create( rBitmapEx,
                                           mpImplData->maImageSize.Width(),
                                           mpImplData->maImageSize.Height(),
                                           nInit );
    }
}

// -----------------------------------------------------------------------

void ImageList::ImplMakeUnique()
{
    if( mpImplData && mpImplData->mnRefCount > 1 )
    {
        ImplImageList*  pNewData = new ImplImageList;
        USHORT          i = 0, n = 0;

        --mpImplData->mnRefCount;

        pNewData->mnRefCount = 1;
        pNewData->mnIRefCount = 0;
        pNewData->mnCount = mpImplData->mnCount;
        pNewData->mnRealCount = mpImplData->mnRealCount;
        pNewData->mnArySize = mpImplData->mnArySize;
        pNewData->mpAry = new ImageAryData[ pNewData->mnArySize ];
        pNewData->maImageSize = mpImplData->maImageSize;
        pNewData->mpImageBitmap = new ImplImageBmp;
        pNewData->mpImageBitmap->Create( pNewData->maImageSize.Width(), pNewData->maImageSize.Height(), pNewData->mnArySize );

        while( i < mpImplData->mnArySize )
        {
            if( mpImplData->mpAry[i].mnId )
            {
                pNewData->mpAry[n].maName = mpImplData->mpAry[i].maName;
                pNewData->mpAry[n].mnId = mpImplData->mpAry[i].mnId;
                pNewData->mpAry[n].mnRefCount = 1;
                pNewData->mpImageBitmap->Replace( n, *mpImplData->mpImageBitmap, i );
                ++n;
            }

            ++i;
        }

        mpImplData = pNewData;
    }
}

// -----------------------------------------------------------------------

USHORT ImageList::ImplGetImageId( const ::rtl::OUString& rImageName ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && rImageName.getLength() )
    {
        USHORT nPos = 0, i = 0;

        while( i < mpImplData->mnArySize )
        {
            if( mpImplData->mpAry[i].maName == rImageName )
                return mpImplData->mpAry[i].mnId;

            if ( mpImplData->mpAry[i].mnId )
                ++nPos;

            ++i;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void ImageList::AddImage( USHORT nId, const Image& rImage )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );
    DBG_ASSERT( nId, "ImageList::AddImage(): ImageId == 0" );
    DBG_ASSERT( GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND, "ImageList::AddImage() - ImageId already exists" );
    DBG_ASSERT( rImage.mpImplData, "ImageList::AddImage(): Wrong Size" );
    DBG_ASSERT( !mpImplData || (rImage.GetSizePixel() == mpImplData->maImageSize), "ImageList::AddImage(): Wrong Size" );

    bool        bHasImage = ( rImage.mpImplData != 0 );
    ImageType   eImageType  = IMAGETYPE_BITMAP;
    Size        aImageSize;
    USHORT      nIndex;

    if( bHasImage )
    {
        eImageType = rImage.mpImplData->meType;
        aImageSize = rImage.GetSizePixel();
    }
    else
    {
        if( mpImplData )
        {
            eImageType = IMAGETYPE_BITMAP;
            aImageSize = mpImplData->maImageSize;
        }
        else
            return;
    }

    if( !mpImplData )
    {
        mpImplData = new ImplImageList;
        mpImplData->mnRefCount = 1;
        mpImplData->mnIRefCount = 0;
        mpImplData->mnCount = 0;
        mpImplData->mnRealCount = 0;
        mpImplData->mnArySize = mnInitSize;
        mpImplData->mpAry = new ImageAryData[mnInitSize];
        mpImplData->maImageSize = aImageSize;
        mpImplData->mpImageBitmap = new ImplImageBmp;
        mpImplData->mpImageBitmap->Create( aImageSize.Width(), aImageSize.Height(), mnInitSize );
    }
    else
        ImplMakeUnique();

    if( mpImplData->mnRealCount == mpImplData->mnArySize )
    {
        ImageAryData*   pOldAry  = mpImplData->mpAry;
        USHORT          nOldSize = mpImplData->mnArySize;

        mpImplData->mnArySize += mnGrowSize;
        mpImplData->mpAry = new ImageAryData[mpImplData->mnArySize];

        for( USHORT i = 0; i < nOldSize; ++i )
            mpImplData->mpAry[ i ] = pOldAry[ i ];

        mpImplData->mpImageBitmap->Expand( mnGrowSize );
        delete[] pOldAry;
        nIndex = mpImplData->mnRealCount;
    }
    else
    {
        nIndex = 0;

        while( mpImplData->mpAry[nIndex].mnRefCount )
            ++nIndex;
    }

    switch( eImageType )
    {
        case IMAGETYPE_BITMAP:
        {
            if( !bHasImage )
            {
                const Bitmap    aBmp( aImageSize, 1 );
                const BitmapEx  aBmpEx( aBmp, COL_BLACK );

                mpImplData->mpImageBitmap->Replace( nIndex, aBmpEx );
            }
            else
                mpImplData->mpImageBitmap->Replace( nIndex, *static_cast< Bitmap* >( rImage.mpImplData->mpData ) );
        }
        break;

        case IMAGETYPE_IMAGE:
        {
            ImplImageData* pData = static_cast< ImplImageData* >( rImage.mpImplData->mpData );

            if( pData->mpImageBitmap )
                mpImplData->mpImageBitmap->Replace( nIndex, *pData->mpImageBitmap, 0 );
            else
                mpImplData->mpImageBitmap->Replace( nIndex, pData->maBmpEx );
        }
        break;

        case IMAGETYPE_IMAGEREF:
        {
            ImplImageRefData* pData = static_cast< ImplImageRefData* >( rImage.mpImplData->mpData );

            mpImplData->mpImageBitmap->Replace( nIndex, *pData->mpImplData->mpImageBitmap, pData->mnIndex );
        }
        break;
    }

    ++mpImplData->mnCount;
    ++mpImplData->mnRealCount;
    mpImplData->mpAry[nIndex].mnId = nId;
    mpImplData->mpAry[nIndex].mnRefCount = 1;
}

// -----------------------------------------------------------------------

void ImageList::AddImage( const ::rtl::OUString& rImageName, const Image& rImage )
{
    DBG_ASSERT( GetImagePos( rImageName ) == IMAGELIST_IMAGE_NOTFOUND, "ImageList::AddImage() - ImageName already exists" );

    if( mpImplData )
    {
        USHORT i, nMax = 0;

        for( i = 0; i < mpImplData->mnArySize; ++i )
        {
            if( mpImplData->mpAry[ i ].mnId > nMax )
            {
                nMax = mpImplData->mpAry[ i ].mnId;
            }
        }

        if( nMax < USHRT_MAX )
        {
            AddImage( ++nMax, rImage );

            for( i = 0; i < mpImplData->mnArySize; ++i )
            {
                if( mpImplData->mpAry[ i ].mnId == nMax )
                {
                    mpImplData->mpAry[ i ].maName = rImageName;
                    break;
                }
            }
        }
        else
        {
            DBG_ERROR( "No free image id left" );
        }
    }
    else
        AddImage( 1, rImage );
}

// -----------------------------------------------------------------------

void ImageList::CopyImage( USHORT nId, USHORT nCopyId )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_ASSERT( nId, "ImageList::CopyImage(): ImageId == 0" );
    DBG_ASSERT( GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND, "ImageList::CopyImage(): ImageId already exists" );
    DBG_ASSERT( GetImagePos( nCopyId ) != IMAGELIST_IMAGE_NOTFOUND, "ImageList::CopyImage(): Unknown nCopyId" );

    USHORT nIndex, nCopyIndex = 0;

    while( nCopyIndex < mpImplData->mnArySize )
    {
        if ( mpImplData->mpAry[nCopyIndex].mnId == nCopyId )
            break;

        ++nCopyIndex;
    }

    if( nCopyIndex < mpImplData->mnArySize )
    {
        ImplMakeUnique();

        if( mpImplData->mnRealCount == mpImplData->mnArySize )
        {
            ImageAryData*   pOldAry  = mpImplData->mpAry;
            USHORT          nOldSize = mpImplData->mnArySize;

            mpImplData->mnArySize += mnGrowSize;
            mpImplData->mpAry = new ImageAryData[mpImplData->mnArySize];

            for( USHORT i = 0; i < nOldSize; ++i )
                mpImplData->mpAry[ i ] = pOldAry[ i ];

            mpImplData->mpImageBitmap->Expand( mnGrowSize );
            delete[] pOldAry;
            nIndex = mpImplData->mnRealCount;
        }
        else
        {
            nIndex = 0;

            while( mpImplData->mpAry[nIndex].mnRefCount )
                nIndex++;
        }

        mpImplData->mpImageBitmap->Replace( nIndex, *mpImplData->mpImageBitmap, nCopyIndex );

        ++mpImplData->mnCount;
        ++mpImplData->mnRealCount;
        mpImplData->mpAry[nIndex].mnId = nId;
        mpImplData->mpAry[nIndex].mnRefCount = 1;
    }
}

// -----------------------------------------------------------------------

void ImageList::CopyImage( const ::rtl::OUString& rImageName, const ::rtl::OUString& rCopyName )
{
    CopyImage( ImplGetImageId( rImageName ), ImplGetImageId( rCopyName ) );
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( USHORT nId, const Image& rImage )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );
    DBG_ASSERT( GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND, "ImageList::ReplaceImage(): Unknown nId" );

    RemoveImage( nId );
    AddImage( nId, rImage );
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( const ::rtl::OUString& rImageName, const Image& rImage )
{
    ReplaceImage( ImplGetImageId( rImageName ), rImage );
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( USHORT nId, USHORT nReplaceId )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_ASSERT( GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND, "ImageList::ReplaceImage(): Unknown nId" );
    DBG_ASSERT( GetImagePos( nReplaceId ) != IMAGELIST_IMAGE_NOTFOUND, "ImageList::ReplaceImage(): Unknown nReplaceId" );

    USHORT nPos1 = 0, nPos2 = 0;

    while( nPos1 < mpImplData->mnArySize )
    {
        if ( mpImplData->mpAry[nPos1].mnId == nId )
            break;

        ++nPos1;
    }

    if( nPos1 < mpImplData->mnArySize )
    {
        while( nPos2 < mpImplData->mnArySize )
        {
            if( mpImplData->mpAry[nPos2].mnId == nReplaceId )
                break;

            ++nPos2;
        }

        if( nPos2 < mpImplData->mnArySize )
        {
            ImplMakeUnique();
            mpImplData->mpImageBitmap->Replace( nPos1, nPos2 );
        }
    }
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( const ::rtl::OUString& rImageName, const ::rtl::OUString& rReplaceName )
{
    ReplaceImage( ImplGetImageId( rImageName ), ImplGetImageId( rReplaceName ) );
}

// -----------------------------------------------------------------------

void ImageList::RemoveImage( USHORT nId )
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData )
    {
        USHORT i = 0;

        ImplMakeUnique();

        while( i < mpImplData->mnArySize )
        {
            if( mpImplData->mpAry[i].mnId == nId )
                break;

            ++i;
        }

        if( i < mpImplData->mnArySize )
        {
            --mpImplData->mpAry[i].mnRefCount;
            mpImplData->mpAry[i].mnId = 0;

            if( !mpImplData->mpAry[i].mnRefCount )
                --mpImplData->mnRealCount;

            --mpImplData->mnCount;
        }
    }
}

// -----------------------------------------------------------------------

void ImageList::RemoveImage( const ::rtl::OUString& rImageName )
{
    RemoveImage( ImplGetImageId( rImageName ) );
}

// -----------------------------------------------------------------------

Image ImageList::GetImage( USHORT nId ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    Image aImage;

    if( mpImplData )
    {
        USHORT i = 0;

        while( i < mpImplData->mnArySize )
        {
            if( mpImplData->mpAry[i].mnId == nId )
                break;

            ++i;
        }

        if( i < mpImplData->mnArySize )
        {
            ImplImageRefData* mpData = new ImplImageRefData;

            ++mpImplData->mnIRefCount;
            ++mpImplData->mpAry[i].mnRefCount;
            mpData->mpImplData = mpImplData;
            mpData->mnIndex = i;

            aImage.mpImplData = new ImplImage;
            aImage.mpImplData->mnRefCount = 1;
            aImage.mpImplData->meType = IMAGETYPE_IMAGEREF;
            aImage.mpImplData->mpData = mpData;
        }
    }

    return aImage;
}

// -----------------------------------------------------------------------

Image ImageList::GetImage( const ::rtl::OUString& rImageName ) const
{
    return GetImage( ImplGetImageId( rImageName ) );
}

// -----------------------------------------------------------------------

void ImageList::Clear()
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;

    mpImplData = NULL;
}

// -----------------------------------------------------------------------

USHORT ImageList::GetImageCount() const
{
    DBG_CHKTHIS( ImageList, NULL );

    return( mpImplData ? mpImplData->mnCount : 0 );
}

// -----------------------------------------------------------------------

USHORT ImageList::GetImagePos( USHORT nId ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && nId )
    {
        USHORT nPos = 0, i = 0;

        while( i < mpImplData->mnArySize )
        {
            if( mpImplData->mpAry[i].mnId == nId )
                return nPos;

            if ( mpImplData->mpAry[i].mnId )
                ++nPos;

            ++i;
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT ImageList::GetImagePos( const ::rtl::OUString& rImageName ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && rImageName.getLength() )
    {
        USHORT nPos = 0, i = 0;

        while( i < mpImplData->mnArySize )
        {
            if( mpImplData->mpAry[i].maName == rImageName )
                return nPos;

            if ( mpImplData->mpAry[i].mnId )
                ++nPos;

            ++i;
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT ImageList::GetImageId( USHORT nPos ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData )
    {
        USHORT nRealPos = 0, i = 0;

        while( i < mpImplData->mnArySize )
        {
            if( (nPos == nRealPos) && (mpImplData->mpAry[i].mnId) )
                return mpImplData->mpAry[i].mnId;

            if ( mpImplData->mpAry[i].mnId )
                ++nRealPos;

            ++i;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

::rtl::OUString ImageList::GetImageName( USHORT nPos ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData )
    {
        USHORT nRealPos = 0, i = 0;

        while( i < mpImplData->mnArySize )
        {
            if( (nPos == nRealPos) && (mpImplData->mpAry[i].mnId) )
                return mpImplData->mpAry[i].maName;

            if ( mpImplData->mpAry[i].mnId )
                ++nRealPos;

            ++i;
        }
    }

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------

Size ImageList::GetImageSize() const
{
    DBG_CHKTHIS( ImageList, NULL );

    Size aRet;

    if( mpImplData )
        aRet = mpImplData->maImageSize;

    return aRet;
}

// -----------------------------------------------------------------------

BitmapEx ImageList::GetBitmapEx() const
{
    DBG_CHKTHIS( ImageList, NULL );

    BitmapEx aRet;

    if( mpImplData )
    {
        USHORT* pPosAry = new USHORT[ mpImplData->mnCount ];
        USHORT  nPosCount = 0;

        for( USHORT i = 0; i < mpImplData->mnArySize; i++ )
        {
            if( mpImplData->mpAry[i].mnId )
            {
                pPosAry[ nPosCount ] = i;
                ++nPosCount;
            }
        }

        aRet = mpImplData->mpImageBitmap->GetBitmapEx( nPosCount, pPosAry );
        delete[] pPosAry;
    }

    return aRet;
}

// -----------------------------------------------------------------------

ImageList ImageList::GetColorTransformedImageList( ImageColorTransform eColorTransform ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    ImageList aRet;

    if( IMAGECOLORTRANSFORM_HIGHCONTRAST == eColorTransform )
    {
        Color*  pSrcColors = NULL;
        Color*  pDstColors = NULL;
        ULONG   nColorCount = 0;

        aRet = *this;
        aRet.ImplMakeUnique();

        Image::GetColorTransformArrays( eColorTransform, pSrcColors, pDstColors, nColorCount );

        if( nColorCount && pSrcColors && pDstColors && mpImplData )
            aRet.mpImplData->mpImageBitmap->ReplaceColors( pSrcColors, pDstColors, nColorCount );

        delete[] pSrcColors;
        delete[] pDstColors;
    }
    else if( IMAGECOLORTRANSFORM_MONOCHROME_BLACK == eColorTransform ||
             IMAGECOLORTRANSFORM_MONOCHROME_WHITE == eColorTransform )
    {
        aRet = *this;
        aRet.ImplMakeUnique();
           aRet.mpImplData->mpImageBitmap->ColorTransform( ( BmpColorMode )( eColorTransform ) );
    }

    if( !aRet.GetImageCount() )
        aRet = *this;

    return aRet;
}

// -----------------------------------------------------------------------

void ImageList::Invert()
{
    ImageList aNew( *this );
    aNew.ImplMakeUnique();
    aNew.mpImplData->mpImageBitmap->Invert();

    *this = aNew;
}

// -----------------------------------------------------------------------

ImageList& ImageList::operator=( const ImageList& rImageList )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    if( rImageList.mpImplData )
        ++rImageList.mpImplData->mnRefCount;

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) && ( 0 == mpImplData->mnIRefCount ) )
        delete mpImplData;

    mpImplData = rImageList.mpImplData;
    mnInitSize = rImageList.mnInitSize;
    mnGrowSize = rImageList.mnGrowSize;

    return *this;
}

// -----------------------------------------------------------------------

BOOL ImageList::operator==( const ImageList& rImageList ) const
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    BOOL bRet;

    if( rImageList.mpImplData == mpImplData )
        bRet = true;
    else if( !rImageList.mpImplData || !mpImplData )
        bRet = false;
    else if( ( rImageList.mpImplData->mnCount == mpImplData->mnCount ) &&
              ( rImageList.mpImplData->maImageSize == mpImplData->maImageSize ) )
    {
        bRet = true;
    }

    return bRet;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, ImageList& rImageList )
{
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    if( rImageList.mpImplData )
    {
        --rImageList.mpImplData->mnRefCount;

        if( ( 0 == rImageList.mpImplData->mnRefCount ) && ( 0 == rImageList.mpImplData->mnIRefCount ) )
            delete rImageList.mpImplData;
    }

    rImageList.mpImplData = NULL;

    USHORT  nVersion;
    Size    aImageSize;
    BOOL    bImageList;

    rIStream >> nVersion >> rImageList.mnInitSize >> rImageList.mnGrowSize >> bImageList;

    if( bImageList )
    {
        BitmapEx    aBmpEx;
        Bitmap      aBmp;
        BYTE        bMaskOrAlpha, bMaskColor;

        rIStream >> aImageSize.Width() >> aImageSize.Height();

        rImageList.mpImplData = new ImplImageList;
        rImageList.mpImplData->mnRefCount = 1;
        rImageList.mpImplData->mnIRefCount= 0;
        rImageList.mpImplData->mnCount = rImageList.mnInitSize;
        rImageList.mpImplData->mnRealCount = rImageList.mnInitSize;
        rImageList.mpImplData->mnArySize = rImageList.mnInitSize;
        rImageList.mpImplData->mpAry = new ImageAryData[ rImageList.mnInitSize ];
        rImageList.mpImplData->maImageSize = aImageSize;

        for( USHORT i = 0; i < rImageList.mnInitSize; ++i )
        {
            rIStream >> rImageList.mpImplData->mpAry[i].mnId;
            rImageList.mpImplData->mpAry[i].mnRefCount = 1;
        }

        rIStream >> aBmp >> bMaskOrAlpha;

        if( bMaskOrAlpha )
        {
            Bitmap aMaskOrAlpha;

            rIStream >> aMaskOrAlpha;

            if( aMaskOrAlpha.GetBitCount() == 8 && aMaskOrAlpha.HasGreyPalette() )
                aBmpEx = BitmapEx( aBmp, AlphaMask( aMaskOrAlpha ) );
            else
                aBmpEx = BitmapEx( aBmp, aMaskOrAlpha );
        }

        rIStream >> bMaskColor;

        if( bMaskColor )
        {
            Color aMaskColor;

            rIStream >> aMaskColor;

            if( !aBmpEx.IsAlpha() && !aBmpEx.IsTransparent() )
                aBmpEx = BitmapEx( aBmp, aMaskColor );
        }

        rImageList.mpImplData->mpImageBitmap = new ImplImageBmp;
        rImageList.mpImplData->mpImageBitmap->Create( aBmpEx, aImageSize.Width(), aImageSize.Height(), rImageList.mnInitSize );
    }

    return rIStream;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const ImageList& rImageList )
{
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    USHORT  nVersion = IMAGE_FILE_VERSION;
    BOOL    bImageList = rImageList.mpImplData ? true : false;

    rOStream << nVersion;

    if ( !bImageList || !rImageList.mpImplData->mnCount )
        rOStream << rImageList.mnInitSize << rImageList.mnGrowSize << ( bImageList = FALSE );
    else
    {
        rOStream << rImageList.mpImplData->mnCount;
        rOStream << rImageList.mnGrowSize;
        rOStream << bImageList;
        rOStream << rImageList.mpImplData->maImageSize.Width();
        rOStream << rImageList.mpImplData->maImageSize.Height();

        USHORT* mpPosAry = new USHORT[rImageList.mpImplData->mnCount];
        USHORT  nPosCount = 0;

        for( USHORT i = 0; i < rImageList.mpImplData->mnArySize; ++i )
        {
            if( rImageList.mpImplData->mpAry[i].mnId )
            {
                rOStream << rImageList.mpImplData->mpAry[i].mnId;
                mpPosAry[ nPosCount++ ] = i;
            }
        }

        BitmapEx    aBmpEx( rImageList.mpImplData->mpImageBitmap->GetBitmapEx( nPosCount, mpPosAry ) );
        const BOOL  bMaskOrAlpha = aBmpEx.IsAlpha() || aBmpEx.IsTransparent();
        const BOOL  bMaskColor = false;

        rOStream << aBmpEx.GetBitmap() << bMaskOrAlpha;

        if( bMaskOrAlpha )
            rOStream << ( aBmpEx.IsAlpha() ? aBmpEx.GetAlpha().ImplGetBitmap() : aBmpEx.GetMask() );

        // BitmapEx doesn't have internal mask colors anymore
        rOStream << bMaskColor;

        delete[] mpPosAry;
    }

    return rOStream;
}
