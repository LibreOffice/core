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
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/imagerepository.hxx>
#include <vcl/ImageTree.hxx>
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
    {
        ImplAddImage( rNameVector[ i ], static_cast< sal_uInt16 >( i ) + 1, BitmapEx() );
    }
}

// FIXME: Rather a performance hazard
BitmapEx ImageList::GetAsHorizontalStrip() const
{
    sal_uInt16 nCount = maImages.size();
    if( !nCount )
        return BitmapEx();
    Size aSize( maImageSize.Width() * nCount, maImageSize.Height() );

    // Load any stragglers
    for (sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++)
    {
        ImageAryData *pData = maImages[ nIdx ].get();
        if( pData->IsLoadable() )
            ImplLoad(*pData);
    }

    BitmapEx aTempl = maImages[ 0 ]->maBitmapEx;
    BitmapEx aResult( aTempl, Point(), aSize );

    tools::Rectangle aSrcRect( Point( 0, 0 ), maImageSize );
    for (sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++)
    {
        tools::Rectangle aDestRect( Point( nIdx * maImageSize.Width(), 0 ),
                             maImageSize );
        ImageAryData *pData = maImages[ nIdx ].get();
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
    aSize.setWidth( aSize.Width() / nItems );
    maImages.clear();
    maNameHash.clear();
    maImages.reserve( nItems );
    maImageSize = aSize;
    maPrefix.clear();

    for (sal_uInt16 nIdx = 0; nIdx < nItems; nIdx++)
    {
        BitmapEx aBitmap( rBitmapEx, Point( nIdx * aSize.Width(), 0 ), aSize );
        ImplAddImage( rNameVector[ nIdx ], nIdx + 1, aBitmap );
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

    ImplAddImage( rImageName, GetImageCount() + 1,
                          rImage.GetBitmapEx() );
}

void ImageList::ReplaceImage( const OUString& rImageName, const Image& rImage )
{
    const sal_uInt16 nId = ImplGetImageId( rImageName );

    if( nId )
    {
        //Just replace the bitmap rather than doing RemoveImage / AddImage
        //which breaks index-based iteration.
        ImageAryData *pImg = maNameHash[ rImageName ];
        pImg->maBitmapEx = rImage.GetBitmapEx();
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
    ImageAryData *pImg = it->second;
    if( pImg->IsLoadable() )
        ImplLoad( *pImg );
    return Image( pImg->maBitmapEx );
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

void ImageList::ImplAddImage( const OUString &aName,
                              sal_uInt16 nId, const BitmapEx &aBitmapEx )
{
    ImageAryData *pImg = new ImageAryData{ aName, nId, aBitmapEx };
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

void ImageList::ImplLoad(ImageAryData& rImageData) const
{
    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

    OUString aFileName = maPrefix + rImageData.maName;

    bool bSuccess = ImageTree::get().loadImage(aFileName, aIconTheme, rImageData.maBitmapEx, true);

    /* If the uno command has parameters, passed in from a toolbar,
     * recover from failure by removing the parameters from the file name
     */
    if (!bSuccess && aFileName.indexOf("%3f") > 0)
    {
        sal_Int32 nStart = aFileName.indexOf("%3f");
        sal_Int32 nEnd = aFileName.lastIndexOf(".");

        aFileName = aFileName.replaceAt(nStart, nEnd - nStart, "");
        bSuccess = ImageTree::get().loadImage(aFileName, aIconTheme, rImageData.maBitmapEx, true);
    }

    SAL_WARN_IF(!bSuccess, "fwk.uiconfiguration", "Failed to load image '" << aFileName
              << "' from icon theme '" << aIconTheme << "'");
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
