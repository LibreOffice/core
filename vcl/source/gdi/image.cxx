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

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

#include <rtl/logfile.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <tools/rc.hxx>
#ifndef _SV_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#ifndef _SV_IMPIMAGETREE_H
#include <vcl/impimagetree.hxx>
#endif
#include <vcl/image.h>
#include <vcl/image.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

DBG_NAME( Image )
DBG_NAME( ImageList )

#define IMAGE_FILE_VERSION 100

using namespace ::com::sun::star;

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
    if( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        BitmapEx    aBmpEx;
        sal_uLong       nObjMask = pResMgr->ReadLong();

        if( nObjMask & RSC_IMAGE_IMAGEBITMAP )
        {
            aBmpEx = BitmapEx( ResId( (RSHEADER_TYPE*)pResMgr->GetClass(), *pResMgr ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }

        if( nObjMask & RSC_IMAGE_MASKBITMAP )
        {
            if( !aBmpEx.IsEmpty() && aBmpEx.GetTransparentType() == TRANSPARENT_NONE )
            {
                const Bitmap aMaskBitmap( ResId( (RSHEADER_TYPE*)pResMgr->GetClass(), *pResMgr ) );
                aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMaskBitmap );
            }

            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }

        if( nObjMask & RSC_IMAGE_MASKCOLOR )
        {
            if( !aBmpEx.IsEmpty() && aBmpEx.GetTransparentType() == TRANSPARENT_NONE )
            {
                const Color aMaskColor( ResId( (RSHEADER_TYPE*)pResMgr->GetClass(), *pResMgr ) );
                aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMaskColor );
            }

            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }
        if( ! aBmpEx.IsEmpty() )
            ImplInit( aBmpEx );
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

Image::Image( const uno::Reference< graphic::XGraphic >& rxGraphic ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    const Graphic aGraphic( rxGraphic );
    ImplInit( aGraphic.GetBitmapEx() );
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
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

uno::Reference< graphic::XGraphic > Image::GetXGraphic() const
{
    const Graphic aGraphic( GetBitmapEx() );

    return aGraphic.GetXGraphic();
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
            sal_uLong   nColorCount = 0;

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
                                     Color*& rpSrcColor, Color*& rpDstColor, sal_uLong& rColorCount )
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

sal_Bool Image::operator==( const Image& rImage ) const
{
    DBG_CHKTHIS( Image, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );

    bool bRet = false;

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

ImageList::ImageList( sal_uInt16 nInit, sal_uInt16 nGrow ) :
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
    RTL_LOGFILE_CONTEXT( aLog, "vcl: ImageList::ImageList( const ResId& rResId )" );

    DBG_CTOR( ImageList, NULL );

    rResId.SetRT( RSC_IMAGELIST );

    ResMgr* pResMgr = rResId.GetResMgr();

    if( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        sal_uLong                               nObjMask = pResMgr->ReadLong();
        const String                        aPrefix( pResMgr->ReadString() );
        ::boost::scoped_ptr< Color >        spMaskColor;

        if( nObjMask & RSC_IMAGE_MASKCOLOR )
            spMaskColor.reset( new Color( ResId( (RSHEADER_TYPE*)pResMgr->GetClass(), *pResMgr ) ) );

        pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );

        if( nObjMask & RSC_IMAGELIST_IDLIST )
        {
            for( sal_Int32 i = 0, nCount = pResMgr->ReadLong(); i < nCount; ++i )
                pResMgr->ReadLong();
        }

        sal_Int32 nCount = pResMgr->ReadLong();
        ImplInit( static_cast< sal_uInt16 >( nCount ), Size() );

        BitmapEx aEmpty;
        for( sal_Int32 i = 0; i < nCount; ++i )
        {
            rtl::OUString aName = pResMgr->ReadString();
            sal_uInt16 nId = static_cast< sal_uInt16 >( pResMgr->ReadLong() );
            mpImplData->AddImage( aName, nId, aEmpty );
        }

        if( nObjMask & RSC_IMAGELIST_IDCOUNT )
            pResMgr->ReadShort();
    }
}

// -----------------------------------------------------------------------

ImageList::ImageList( const ::std::vector< ::rtl::OUString >& rNameVector,
                      const ::rtl::OUString& rPrefix,
                      const Color* ) :
    mpImplData( NULL ),
    mnInitSize( 1 ),
    mnGrowSize( 4 )
{
    RTL_LOGFILE_CONTEXT( aLog, "vcl: ImageList::ImageList(const vector< OUString >& ..." );

    DBG_CTOR( ImageList, NULL );

    ImplInit( sal::static_int_cast< sal_uInt16 >( rNameVector.size() ), Size() );

    mpImplData->maPrefix = rPrefix;
    for( sal_uInt32 i = 0; i < rNameVector.size(); ++i )
    {
//      fprintf (stderr, "List %p [%d]: '%s'\n",
//               this, i, rtl::OUStringToOString( rNameVector[i], RTL_TEXTENCODING_UTF8 ).getStr() );
        mpImplData->AddImage( rNameVector[ i ], static_cast< sal_uInt16 >( i ) + 1, BitmapEx() );
    }
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

ImageList::~ImageList()
{
    DBG_DTOR( ImageList, NULL );

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;
}

void ImageList::ImplInit( sal_uInt16 nItems, const Size &rSize )
{
    mpImplData = new ImplImageList;
    mpImplData->mnRefCount = 1;
    mpImplData->maImages.reserve( nItems );
    mpImplData->maImageSize = rSize;
}

// -----------------------------------------------------------------------

void ImageAryData::Load(const rtl::OUString &rPrefix)
{
    static ImplImageTreeSingletonRef aImageTree;

    ::rtl::OUString aSymbolsStyle = Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyleName();

    BitmapEx aBmpEx;

//  fprintf (stderr, "Attempt load of '%s'\n",
//           rtl::OUStringToOString( maName, RTL_TEXTENCODING_UTF8 ).getStr() );

    rtl::OUString aFileName = rPrefix;
    aFileName += maName;
#if OSL_DEBUG_LEVEL > 0
    bool bSuccess =
#endif
        aImageTree->loadImage( aFileName, aSymbolsStyle, maBitmapEx, true );
#if OSL_DEBUG_LEVEL > 0
    if ( !bSuccess )
    {
        ::rtl::OStringBuffer aMessage;
        aMessage.append( "ImageAryData::Load: failed to load image '" );
        aMessage.append( ::rtl::OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8 ).getStr() );
        aMessage.append( "'" );
        OSL_ENSURE( false, aMessage.makeStringAndClear().getStr() );
    }
#endif
}

// -----------------------------------------------------------------------

void ImageList::ImplMakeUnique()
{
    if( mpImplData && mpImplData->mnRefCount > 1 )
    {
        --mpImplData->mnRefCount;
        mpImplData = new ImplImageList( *mpImplData ) ;
    }
}

// -----------------------------------------------------------------------
// Rather a performance hazard:
BitmapEx ImageList::GetAsHorizontalStrip() const
{
    Size aSize( mpImplData->maImageSize );
    sal_uInt16 nCount = GetImageCount();
    if( !nCount )
        return BitmapEx();
    aSize.Width() *= nCount;

    // Load any stragglers
    for (sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++)
    {
        ImageAryData *pData = mpImplData->maImages[ nIdx ];
        if( pData->IsLoadable() )
            pData->Load( mpImplData->maPrefix );
    }

    BitmapEx aTempl = mpImplData->maImages[ 0 ]->maBitmapEx;
    BitmapEx aResult;
    Bitmap aPixels( aSize, aTempl.GetBitmap().GetBitCount() );
    if( aTempl.IsAlpha() )
        aResult = BitmapEx( aPixels, AlphaMask( aSize ) );
    else if( aTempl.IsTransparent() )
        aResult = BitmapEx( aPixels, Bitmap( aSize, aTempl.GetMask().GetBitCount() ) );
    else
        aResult = BitmapEx( aPixels );

    Rectangle aSrcRect( Point( 0, 0 ), mpImplData->maImageSize );
    for (sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++)
    {
        Rectangle aDestRect( Point( nIdx * mpImplData->maImageSize.Width(), 0 ),
                             mpImplData->maImageSize );
        ImageAryData *pData = mpImplData->maImages[ nIdx ];
        aResult.CopyPixel( aDestRect, aSrcRect, &pData->maBitmapEx);
    }

    return aResult;
}

// -----------------------------------------------------------------------

void ImageList::InsertFromHorizontalStrip( const BitmapEx &rBitmapEx,
                                           const std::vector< rtl::OUString > &rNameVector )
{
    sal_uInt16 nItems = sal::static_int_cast< sal_uInt16 >( rNameVector.size() );

//  fprintf (stderr, "InsertFromHorizontalStrip (1) [%d items]\n", nItems);

    if (!nItems)
            return;

    Size aSize( rBitmapEx.GetSizePixel() );
    DBG_ASSERT (rBitmapEx.GetSizePixel().Width() % nItems == 0,
                "ImageList::InsertFromHorizontalStrip - very odd size");
    aSize.Width() /= nItems;
    ImplInit( nItems, aSize );

    for (sal_uInt16 nIdx = 0; nIdx < nItems; nIdx++)
    {
        BitmapEx aBitmap( rBitmapEx, Point( nIdx * aSize.Width(), 0 ), aSize );
        mpImplData->AddImage( rNameVector[ nIdx ], nIdx + 1, aBitmap );
    }
}

// -----------------------------------------------------------------------

void ImageList::InsertFromHorizontalBitmap( const ResId& rResId,
                                            sal_uInt16       nCount,
                                            const Color *pMaskColor,
                                            const Color *pSearchColors,
                                            const Color *pReplaceColors,
                                            sal_uLong        nColorCount)
{
    BitmapEx aBmpEx( rResId );
    if (!aBmpEx.IsTransparent())
    {
        if( pMaskColor )
            aBmpEx = BitmapEx( aBmpEx.GetBitmap(), *pMaskColor );
        else
            aBmpEx = BitmapEx( aBmpEx.GetBitmap() );
    }
    if ( nColorCount && pSearchColors && pReplaceColors )
        aBmpEx.Replace( pSearchColors, pReplaceColors, nColorCount );

    std::vector< rtl::OUString > aNames( nCount );
    InsertFromHorizontalStrip( aBmpEx, aNames );
}

// -----------------------------------------------------------------------

sal_uInt16 ImageList::ImplGetImageId( const ::rtl::OUString& rImageName ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    ImageAryData *pImg = mpImplData->maNameHash[ rImageName ];
    if( pImg )
        return pImg->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ImageList::AddImage( sal_uInt16 nId, const Image& rImage )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );
    DBG_ASSERT( nId, "ImageList::AddImage(): ImageId == 0" );
    DBG_ASSERT( GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND, "ImageList::AddImage() - ImageId already exists" );
    DBG_ASSERT( rImage.mpImplData, "ImageList::AddImage(): Wrong Size" );
    DBG_ASSERT( !mpImplData || (rImage.GetSizePixel() == mpImplData->maImageSize), "ImageList::AddImage(): Wrong Size" );

    if( !mpImplData )
        ImplInit( 0, rImage.GetSizePixel() );

    mpImplData->AddImage( rtl::OUString(), nId, rImage.GetBitmapEx());
}

// -----------------------------------------------------------------------

void ImageList::AddImage( const ::rtl::OUString& rImageName, const Image& rImage )
{
    DBG_ASSERT( GetImagePos( rImageName ) == IMAGELIST_IMAGE_NOTFOUND, "ImageList::AddImage() - ImageName already exists" );

    if( !mpImplData )
        ImplInit( 0, rImage.GetSizePixel() );

    mpImplData->AddImage( rImageName, GetImageCount() + 1,
                          rImage.GetBitmapEx() );
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( sal_uInt16 nId, const Image& rImage )
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
    const sal_uInt16 nId = ImplGetImageId( rImageName );

    if( nId )
    {
        RemoveImage( nId );

        if( !mpImplData )
            ImplInit( 0, rImage.GetSizePixel() );
        mpImplData->AddImage( rImageName, nId, rImage.GetBitmapEx());
    }
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( sal_uInt16 nId, sal_uInt16 nReplaceId )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_ASSERT( GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND, "ImageList::ReplaceImage(): Unknown nId" );
    DBG_ASSERT( GetImagePos( nReplaceId ) != IMAGELIST_IMAGE_NOTFOUND, "ImageList::ReplaceImage(): Unknown nReplaceId" );

    sal_uLong nPosDest = GetImagePos( nId );
    sal_uLong nPosSrc = GetImagePos( nReplaceId );
    if( nPosDest != IMAGELIST_IMAGE_NOTFOUND &&
        nPosSrc != IMAGELIST_IMAGE_NOTFOUND )
    {
        ImplMakeUnique();
        mpImplData->maImages[nPosDest] = mpImplData->maImages[nPosSrc];
    }
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( const ::rtl::OUString& rImageName, const ::rtl::OUString& rReplaceName )
{
    const sal_uInt16 nId1 = ImplGetImageId( rImageName ), nId2 = ImplGetImageId( rReplaceName );

    if( nId1 && nId2 )
        ReplaceImage( nId1, nId2 );
}

// -----------------------------------------------------------------------

void ImageList::RemoveImage( sal_uInt16 nId )
{
    DBG_CHKTHIS( ImageList, NULL );

    for( sal_uInt32 i = 0; i < mpImplData->maImages.size(); ++i )
    {
        if( mpImplData->maImages[ i ]->mnId == nId )
        {
            mpImplData->RemoveImage( static_cast< sal_uInt16 >( i ) );
            break;
        }
    }
}

// -----------------------------------------------------------------------

void ImageList::RemoveImage( const ::rtl::OUString& rImageName )
{
    const sal_uInt16 nId = ImplGetImageId( rImageName );

    if( nId )
        RemoveImage( nId );
}

// -----------------------------------------------------------------------

Image ImageList::GetImage( sal_uInt16 nId ) const
{
    DBG_CHKTHIS( ImageList, NULL );

//  fprintf (stderr, "GetImage %d\n", nId);

    Image aRet;

    if( mpImplData )
    {
        std::vector<ImageAryData *>::iterator aIter;
        for( aIter = mpImplData->maImages.begin();
             aIter != mpImplData->maImages.end(); aIter++)
        {
            if ((*aIter)->mnId == nId)
            {
                if( (*aIter)->IsLoadable() )
                    (*aIter)->Load( mpImplData->maPrefix );

                aRet = Image( (*aIter)->maBitmapEx );
            }
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

Image ImageList::GetImage( const ::rtl::OUString& rImageName ) const
{
//  fprintf (stderr, "GetImage '%s'\n",
//           rtl::OUStringToOString( rImageName, RTL_TEXTENCODING_UTF8 ).getStr() );

    if( mpImplData )
    {
        ImageAryData *pImg = mpImplData->maNameHash[ rImageName ];

        if( pImg )
        {
            if( pImg->IsLoadable() )
                pImg->Load( mpImplData->maPrefix );
            return Image( pImg->maBitmapEx );
        }
    }
//  fprintf (stderr, "no such image\n");

    return Image();
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

sal_uInt16 ImageList::GetImageCount() const
{
    DBG_CHKTHIS( ImageList, NULL );

    return mpImplData ? static_cast< sal_uInt16 >( mpImplData->maImages.size() ) : 0;
}

// -----------------------------------------------------------------------

sal_uInt16 ImageList::GetImagePos( sal_uInt16 nId ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && nId )
    {
        for( sal_uInt32 i = 0; i < mpImplData->maImages.size(); ++i )
        {
            if (mpImplData->maImages[ i ]->mnId == nId)
                return static_cast< sal_uInt16 >( i );
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

bool ImageList::HasImageAtPos( sal_uInt16 nId ) const
{
    return GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 ImageList::GetImagePos( const ::rtl::OUString& rImageName ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && rImageName.getLength() )
    {
        for( sal_uInt32 i = 0; i < mpImplData->maImages.size(); i++ )
        {
            if (mpImplData->maImages[i]->maName == rImageName)
                return static_cast< sal_uInt16 >( i );
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_uInt16 ImageList::GetImageId( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && (nPos < GetImageCount()) )
        return mpImplData->maImages[ nPos ]->mnId;

    return 0;
}

// -----------------------------------------------------------------------

void ImageList::GetImageIds( ::std::vector< sal_uInt16 >& rIds ) const
{
    RTL_LOGFILE_CONTEXT( aLog, "vcl: ImageList::GetImageIds" );

    DBG_CHKTHIS( ImageList, NULL );

    rIds = ::std::vector< sal_uInt16 >();

    if( mpImplData )
    {
        for( sal_uInt32 i = 0; i < mpImplData->maImages.size(); i++ )
            rIds.push_back( mpImplData->maImages[i]->mnId );
    }
}

// -----------------------------------------------------------------------

::rtl::OUString ImageList::GetImageName( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && (nPos < GetImageCount()) )
        return mpImplData->maImages[ nPos ]->maName;

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------

void ImageList::GetImageNames( ::std::vector< ::rtl::OUString >& rNames ) const
{
    RTL_LOGFILE_CONTEXT( aLog, "vcl: ImageList::GetImageNames" );

    DBG_CHKTHIS( ImageList, NULL );

    rNames = ::std::vector< ::rtl::OUString >();

    if( mpImplData )
    {
        for( sal_uInt32 i = 0; i < mpImplData->maImages.size(); i++ )
        {
            const rtl::OUString& rName( mpImplData->maImages[ i ]->maName );
            if( rName.getLength() != 0 )
                rNames.push_back( rName );
        }
    }
}

// -----------------------------------------------------------------------

Size ImageList::GetImageSize() const
{
    DBG_CHKTHIS( ImageList, NULL );

    Size aRet;

    if( mpImplData )
    {
        aRet = mpImplData->maImageSize;

        // force load of 1st image to see - uncommon case.
        if( aRet.Width() == 0 && aRet.Height() == 0 &&
            !mpImplData->maImages.empty() )
        {
            Image aTmp = GetImage( mpImplData->maImages[ 0 ]->mnId );
            aRet = mpImplData->maImageSize = aTmp.GetSizePixel();
        }
    }
//  fprintf (stderr, "GetImageSize returns %d, %d\n",
//           aRet.Width(), aRet.Height());

    return aRet;
}

// -----------------------------------------------------------------------

ImageList& ImageList::operator=( const ImageList& rImageList )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    if( rImageList.mpImplData )
        ++rImageList.mpImplData->mnRefCount;

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;

    mpImplData = rImageList.mpImplData;

    return *this;
}

// -----------------------------------------------------------------------

sal_Bool ImageList::operator==( const ImageList& rImageList ) const
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    bool bRet = false;

    if( rImageList.mpImplData == mpImplData )
        bRet = true;
    else if( !rImageList.mpImplData || !mpImplData )
        bRet = false;
    else if( rImageList.GetImageCount() == GetImageCount() &&
              rImageList.mpImplData->maImageSize == mpImplData->maImageSize )
        bRet = true; // strange semantic

    return bRet;
}
