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

#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <vcl/image.hxx>
#include "ImageList.hxx"

ImageList::ImageList()
{
}

ImageList::ImageList(const std::vector< OUString >& rNameVector,
                     const OUString& rPrefix)
{
    SAL_INFO( "vcl", "vcl: ImageList::ImageList(const vector< OUString >& ..." );

    maImages.reserve( rNameVector.size() );

    maPrefix = rPrefix;
    for( size_t i = 0; i < rNameVector.size(); ++i )
        ImplAddImage( rPrefix, rNameVector[ i ], static_cast< sal_uInt16 >( i ) + 1, Image() );
}

// FIXME: Rather a performance hazard
BitmapEx ImageList::GetAsHorizontalStrip() const
{
    sal_uInt16 nCount = maImages.size();
    if( !nCount )
        return BitmapEx();

    BitmapEx aTempl = maImages[ 0 ]->maImage.GetBitmapEx();
    Size aImageSize(aTempl.GetSizePixel());
    Size aSize(aImageSize.Width() * nCount, aImageSize.Height());
    BitmapEx aResult( aTempl, Point(), aSize );

    tools::Rectangle aSrcRect( Point( 0, 0 ), aImageSize );
    for (sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++)
    {
        tools::Rectangle aDestRect( Point( nIdx * aImageSize.Width(), 0 ), aImageSize );
        ImageAryData *pData = maImages[ nIdx ].get();
        BitmapEx aTmp = pData->maImage.GetBitmapEx();
        aResult.CopyPixel( aDestRect, aSrcRect, &aTmp);
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
    aSize.setWidth( aSize.Width() / nItems );
    maImages.clear();
    maNameHash.clear();
    maImages.reserve( nItems );
    maPrefix.clear();

    for (sal_uInt16 nIdx = 0; nIdx < nItems; nIdx++)
    {
        BitmapEx aBitmap( rBitmapEx, Point( nIdx * aSize.Width(), 0 ), aSize );
        ImplAddImage( maPrefix, rNameVector[ nIdx ], nIdx + 1, Image( aBitmap ) );
    }
}

sal_uInt16 ImageList::ImplGetImageId( const OUString& rImageName ) const
{
    auto it = maNameHash.find( rImageName );
    if (it == maNameHash.end())
        return 0;
    return it->second->mnId;
}

void ImageList::AddImage( const OUString& rImageName, const Image& rImage )
{
    SAL_WARN_IF( GetImagePos( rImageName ) != IMAGELIST_IMAGE_NOTFOUND, "vcl", "ImageList::AddImage() - ImageName already exists" );

    ImplAddImage( maPrefix, rImageName, GetImageCount() + 1, rImage );
}

void ImageList::ReplaceImage( const OUString& rImageName, const Image& rImage )
{
    const sal_uInt16 nId = ImplGetImageId( rImageName );

    if( nId )
    {
        // Just replace the bitmap rather than doing RemoveImage / AddImage
        // which breaks index-based iteration.
        ImageAryData *pImg = maNameHash[ rImageName ];
        pImg->maImage = rImage;
    }
}

void ImageList::RemoveImage( sal_uInt16 nId )
{
    for( size_t i = 0; i < maImages.size(); ++i )
    {
        if( maImages[ i ]->mnId == nId )
        {
            ImplRemoveImage( static_cast< sal_uInt16 >( i ) );
            break;
        }
    }
}

Image ImageList::GetImage( const OUString& rImageName ) const
{
    auto it = maNameHash.find( rImageName );
    if (it == maNameHash.end())
        return Image();
    return it->second->maImage;
}

sal_uInt16 ImageList::GetImageCount() const
{
    return static_cast< sal_uInt16 >( maImages.size() );
}

sal_uInt16 ImageList::GetImagePos( const OUString& rImageName ) const
{
    if( !rImageName.isEmpty() )
    {
        for( size_t i = 0; i < maImages.size(); i++ )
        {
            if (maImages[i]->maName == rImageName)
                return static_cast< sal_uInt16 >( i );
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

sal_uInt16 ImageList::GetImageId( sal_uInt16 nPos ) const
{
    return maImages[ nPos ]->mnId;
}

OUString ImageList::GetImageName( sal_uInt16 nPos ) const
{
    return maImages[ nPos ]->maName;
}

void ImageList::GetImageNames( std::vector< OUString >& rNames ) const
{
    SAL_INFO( "vcl", "vcl: ImageList::GetImageNames" );

    rNames = std::vector< OUString >();

    for(auto const & pImage : maImages)
    {
        const OUString& rName( pImage->maName );
        if( !rName.isEmpty())
            rNames.push_back( rName );
    }
}

void ImageList::ImplAddImage( const OUString &aPrefix, const OUString &aName,
                              sal_uInt16 nId, const Image &aImage )
{
    Image aInsert = aImage;
    if (!aInsert)
        aInsert = Image( "private:graphicrepository/" + aPrefix + aName );

    ImageAryData *pImg = new ImageAryData{ aName, nId, aInsert };
    maImages.emplace_back( pImg );
    if( !aName.isEmpty() )
        maNameHash [ aName ] = pImg;
}

void ImageList::ImplRemoveImage( sal_uInt16 nPos )
{
    ImageAryData *pImg = maImages[ nPos ].get();
    if( !pImg->maName.isEmpty() )
        maNameHash.erase( pImg->maName );
    maImages.erase( maImages.begin() + nPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
