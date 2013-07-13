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

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>


#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/rc.h>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/imagerepository.hxx>
#include <impimagetree.hxx>
#include <image.h>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

DBG_NAME( Image )
DBG_NAME( ImageList )

using namespace ::com::sun::star;

Image::Image() :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );
}

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

Image::Image( const Image& rImage ) :
    mpImplData( rImage.mpImplData )
{
    DBG_CTOR( Image, NULL );

    if( mpImplData )
        ++mpImplData->mnRefCount;
}

Image::Image( const BitmapEx& rBitmapEx ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    ImplInit( rBitmapEx );
}

Image::Image( const Bitmap& rBitmap ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    ImplInit( rBitmap );
}

Image::Image( const Bitmap& rBitmap, const Bitmap& rMaskBitmap ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    const BitmapEx aBmpEx( rBitmap, rMaskBitmap );

    ImplInit( aBmpEx );
}

Image::Image( const Bitmap& rBitmap, const Color& rColor ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    const BitmapEx aBmpEx( rBitmap, rColor );

    ImplInit( aBmpEx );
}

Image::Image( const uno::Reference< graphic::XGraphic >& rxGraphic ) :
    mpImplData( NULL )
{
    DBG_CTOR( Image, NULL );

    const Graphic aGraphic( rxGraphic );
    ImplInit( aGraphic.GetBitmapEx() );
}

Image::~Image()
{
    DBG_DTOR( Image, NULL );

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;
}

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

uno::Reference< graphic::XGraphic > Image::GetXGraphic() const
{
    const Graphic aGraphic( GetBitmapEx() );

    return aGraphic.GetXGraphic();
}

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

ImageList::ImageList( sal_uInt16 nInit, sal_uInt16 nGrow ) :
    mpImplData( NULL ),
    mnInitSize( nInit ),
    mnGrowSize( nGrow )
{
    DBG_CTOR( ImageList, NULL );
}

ImageList::ImageList( const ResId& rResId ) :
    mpImplData( NULL ),
    mnInitSize( 1 ),
    mnGrowSize( 4 )
{
    SAL_INFO( "vcl.gdi", "vcl: ImageList::ImageList( const ResId& rResId )" );

    DBG_CTOR( ImageList, NULL );

    rResId.SetRT( RSC_IMAGELIST );

    ResMgr* pResMgr = rResId.GetResMgr();

    if( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        sal_uLong                               nObjMask = pResMgr->ReadLong();
        pResMgr->ReadString(); //skip string
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
            OUString aName = pResMgr->ReadString();
            sal_uInt16 nId = static_cast< sal_uInt16 >( pResMgr->ReadLong() );
            mpImplData->AddImage( aName, nId, aEmpty );
        }

        if( nObjMask & RSC_IMAGELIST_IDCOUNT )
            pResMgr->ReadShort();
    }
}

ImageList::ImageList( const ::std::vector< OUString >& rNameVector,
                      const OUString& rPrefix,
                      const Color* ) :
    mpImplData( NULL ),
    mnInitSize( 1 ),
    mnGrowSize( 4 )
{
    SAL_INFO( "vcl.gdi", "vcl: ImageList::ImageList(const vector< OUString >& ..." );

    DBG_CTOR( ImageList, NULL );

    ImplInit( sal::static_int_cast< sal_uInt16 >( rNameVector.size() ), Size() );

    mpImplData->maPrefix = rPrefix;
    for( sal_uInt32 i = 0; i < rNameVector.size(); ++i )
    {
        mpImplData->AddImage( rNameVector[ i ], static_cast< sal_uInt16 >( i ) + 1, BitmapEx() );
    }
}

ImageList::ImageList( const ImageList& rImageList ) :
    mpImplData( rImageList.mpImplData ),
    mnInitSize( rImageList.mnInitSize ),
    mnGrowSize( rImageList.mnGrowSize )
{
    DBG_CTOR( ImageList, NULL );

    if( mpImplData )
        ++mpImplData->mnRefCount;
}

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

void ImageAryData::Load(const OUString &rPrefix)
{
    static ImplImageTreeSingletonRef aImageTree;

    OUString aSymbolsStyle = Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyleName();

    BitmapEx aBmpEx;

    OUString aFileName = rPrefix;
    aFileName += maName;
#if OSL_DEBUG_LEVEL > 0
    bool bSuccess =
#endif
        aImageTree->loadImage( aFileName, aSymbolsStyle, maBitmapEx, true );
#if OSL_DEBUG_LEVEL > 0
    if ( !bSuccess )
    {
        OStringBuffer aMessage;
        aMessage.append( "ImageAryData::Load: failed to load image '" );
        aMessage.append( OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8 ).getStr() );
        aMessage.append( "'" );
        OSL_FAIL( aMessage.makeStringAndClear().getStr() );
    }
#endif
}

// FIXME: Rather a performance hazard
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

void ImageList::InsertFromHorizontalStrip( const BitmapEx &rBitmapEx,
                                           const std::vector< OUString > &rNameVector )
{
    sal_uInt16 nItems = sal::static_int_cast< sal_uInt16 >( rNameVector.size() );

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

    std::vector< OUString > aNames( nCount );
    InsertFromHorizontalStrip( aBmpEx, aNames );
}

sal_uInt16 ImageList::ImplGetImageId( const OUString& rImageName ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    ImageAryData *pImg = mpImplData->maNameHash[ rImageName ];
    if( pImg )
        return pImg->mnId;
    else
        return 0;
}

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

void ImageList::AddImage( const OUString& rImageName, const Image& rImage )
{
    DBG_ASSERT( GetImagePos( rImageName ) == IMAGELIST_IMAGE_NOTFOUND, "ImageList::AddImage() - ImageName already exists" );

    if( !mpImplData )
        ImplInit( 0, rImage.GetSizePixel() );

    mpImplData->AddImage( rImageName, GetImageCount() + 1,
                          rImage.GetBitmapEx() );
}

void ImageList::ReplaceImage( const OUString& rImageName, const Image& rImage )
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

Image ImageList::GetImage( sal_uInt16 nId ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    Image aRet;

    if( mpImplData )
    {
        std::vector<ImageAryData *>::iterator aIter;
        for( aIter = mpImplData->maImages.begin();
             aIter != mpImplData->maImages.end(); ++aIter)
        {
            if ((*aIter)->mnId == nId)
            {
                if( (*aIter)->IsLoadable() )
                    (*aIter)->Load( mpImplData->maPrefix );

                aRet = Image( (*aIter)->maBitmapEx );
            }
        }
    }

    if (!aRet)
    {
        BitmapEx rBitmap;
        bool res = ::vcl::ImageRepository::loadDefaultImage(rBitmap);
        if (res)
            aRet =  Image(rBitmap);
    }

    return aRet;
}

Image ImageList::GetImage( const OUString& rImageName ) const
{
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

    return Image();
}

sal_uInt16 ImageList::GetImageCount() const
{
    DBG_CHKTHIS( ImageList, NULL );

    return mpImplData ? static_cast< sal_uInt16 >( mpImplData->maImages.size() ) : 0;
}

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

sal_uInt16 ImageList::GetImagePos( const OUString& rImageName ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && !rImageName.isEmpty() )
    {
        for( sal_uInt32 i = 0; i < mpImplData->maImages.size(); i++ )
        {
            if (mpImplData->maImages[i]->maName == rImageName)
                return static_cast< sal_uInt16 >( i );
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

sal_uInt16 ImageList::GetImageId( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && (nPos < GetImageCount()) )
        return mpImplData->maImages[ nPos ]->mnId;

    return 0;
}

OUString ImageList::GetImageName( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if( mpImplData && (nPos < GetImageCount()) )
        return mpImplData->maImages[ nPos ]->maName;

    return OUString();
}

void ImageList::GetImageNames( ::std::vector< OUString >& rNames ) const
{
    SAL_INFO( "vcl.gdi", "vcl: ImageList::GetImageNames" );

    DBG_CHKTHIS( ImageList, NULL );

    rNames = ::std::vector< OUString >();

    if( mpImplData )
    {
        for( sal_uInt32 i = 0; i < mpImplData->maImages.size(); i++ )
        {
            const OUString& rName( mpImplData->maImages[ i ]->maName );
            if( !rName.isEmpty())
                rNames.push_back( rName );
        }
    }
}

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
    return aRet;
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
