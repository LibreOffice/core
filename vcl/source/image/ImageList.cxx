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

#include <osl/file.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/rc.h>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/imagerepository.hxx>
#include <vcl/implimagetree.hxx>
#include <image.h>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

ImageList::ImageList() :
    mpImplData( nullptr )
{
}

ImageList::ImageList( const ResId& rResId ) :
    mpImplData( nullptr )
{
    SAL_INFO( "vcl.gdi", "vcl: ImageList::ImageList( const ResId& rResId )" );

    rResId.SetRT( RSC_IMAGELIST );

    ResMgr* pResMgr = rResId.GetResMgr();

    if( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        RscImageListFlags nObjMask = (RscImageListFlags)pResMgr->ReadLong();
        pResMgr->ReadString(); //skip string
        std::unique_ptr< Color >        xMaskColor;

        if( nObjMask & RscImageListFlags::MaskColor )
            xMaskColor.reset( new Color( ResId( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()), *pResMgr ) ) );

        pResMgr->Increment( ResMgr::GetObjSize( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()) ) );

        if( nObjMask & RscImageListFlags::IdList )
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

        if( nObjMask & RscImageListFlags::IdCount )
            pResMgr->ReadShort();
    }
}

ImageList::ImageList( const std::vector< OUString >& rNameVector,
                      const OUString& rPrefix) :
    mpImplData( nullptr )
{
    SAL_INFO( "vcl.gdi", "vcl: ImageList::ImageList(const vector< OUString >& ..." );

    ImplInit( sal::static_int_cast< sal_uInt16 >( rNameVector.size() ), Size() );

    mpImplData->maPrefix = rPrefix;
    for( size_t i = 0; i < rNameVector.size(); ++i )
    {
        mpImplData->AddImage( rNameVector[ i ], static_cast< sal_uInt16 >( i ) + 1, BitmapEx() );
    }
}

ImageList::ImageList( const ImageList& rImageList ) :
    mpImplData( rImageList.mpImplData )
{

    if( mpImplData )
        ++mpImplData->mnRefCount;
}

ImageList::~ImageList()
{
    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;
}

void ImageList::ImplInit( sal_uInt16 nItems, const Size &rSize )
{
    mpImplData = new ImplImageList;
    mpImplData->maImages.reserve( nItems );
    mpImplData->maImageSize = rSize;
}

void ImageAryData::Load(const OUString &rPrefix)
{
    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

    OUString aFileName = rPrefix;
    aFileName += maName;
#if OSL_DEBUG_LEVEL > 0
    bool bSuccess =
#endif
        ImplImageTree::get().loadImage(aFileName, aIconTheme, maBitmapEx, true);
#if OSL_DEBUG_LEVEL > 0
    if ( !bSuccess )
    {
        OStringBuffer aMessage;
        aMessage.append( "ImageAryData::Load: failed to load image '" );
        aMessage.append( OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8 ).getStr() );
        aMessage.append( "'" );
        aMessage.append( " from icon theme '" );
        aMessage.append( OUStringToOString( aIconTheme, RTL_TEXTENCODING_UTF8 ).getStr() );
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

    ImageAryData *pImg = mpImplData->maNameHash[ rImageName ];
    if( pImg )
        return pImg->mnId;
    else
        return 0;
}

void ImageList::AddImage( const OUString& rImageName, const Image& rImage )
{
    SAL_WARN_IF( GetImagePos( rImageName ) != IMAGELIST_IMAGE_NOTFOUND, "vcl", "ImageList::AddImage() - ImageName already exists" );

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
        //Just replace the bitmap rather than doing RemoveImage / AddImage
        //which breaks index-based iteration.
        ImageAryData *pImg = mpImplData->maNameHash[ rImageName ];
        pImg->maBitmapEx = rImage.GetBitmapEx();
    }
}

void ImageList::RemoveImage( sal_uInt16 nId )
{

    for( size_t i = 0; i < mpImplData->maImages.size(); ++i )
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
        bool res = vcl::ImageRepository::loadDefaultImage(rBitmap);
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

    return mpImplData ? static_cast< sal_uInt16 >( mpImplData->maImages.size() ) : 0;
}

sal_uInt16 ImageList::GetImagePos( sal_uInt16 nId ) const
{

    if( mpImplData && nId )
    {
        for( size_t i = 0; i < mpImplData->maImages.size(); ++i )
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

    if( mpImplData && !rImageName.isEmpty() )
    {
        for( size_t i = 0; i < mpImplData->maImages.size(); i++ )
        {
            if (mpImplData->maImages[i]->maName == rImageName)
                return static_cast< sal_uInt16 >( i );
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

sal_uInt16 ImageList::GetImageId( sal_uInt16 nPos ) const
{

    if( mpImplData && (nPos < GetImageCount()) )
        return mpImplData->maImages[ nPos ]->mnId;

    return 0;
}

OUString ImageList::GetImageName( sal_uInt16 nPos ) const
{

    if( mpImplData && (nPos < GetImageCount()) )
        return mpImplData->maImages[ nPos ]->maName;

    return OUString();
}

void ImageList::GetImageNames( std::vector< OUString >& rNames ) const
{
    SAL_INFO( "vcl.gdi", "vcl: ImageList::GetImageNames" );

    rNames = std::vector< OUString >();

    if( mpImplData )
    {
        for(const ImageAryData* pImage : mpImplData->maImages)
        {
            const OUString& rName( pImage->maName );
            if( !rName.isEmpty())
                rNames.push_back( rName );
        }
    }
}

Size ImageList::GetImageSize() const
{

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

    if( rImageList.mpImplData )
        ++rImageList.mpImplData->mnRefCount;

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;

    mpImplData = rImageList.mpImplData;

    return *this;
}

bool ImageList::operator==( const ImageList& rImageList ) const
{

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
