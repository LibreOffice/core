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

#include <osl/diagnose.h>
#include <vcl/bitmapaccess.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapSmoothenFilter.hxx>
#include <vcl/BitmapSharpenFilter.hxx>
#include <vcl/BitmapMedianFilter.hxx>
#include <vcl/BitmapSobelGreyFilter.hxx>
#include <vcl/BitmapSolarizeFilter.hxx>
#include <vcl/BitmapSepiaFilter.hxx>
#include <vcl/BitmapMosaicFilter.hxx>
#include <vcl/BitmapEmbossGreyFilter.hxx>
#include <vcl/BitmapSepiaFilter.hxx>
#include <vcl/BitmapPopArtFilter.hxx>

#include <bitmapwriteaccess.hxx>

#include <memory>
#include <stdlib.h>

static inline sal_uInt8 lcl_getDuotoneColorComponent( sal_uInt8 base, sal_uInt16 color1, sal_uInt16 color2 )
{
    color2 = color2*base/0xFF;
    color1 = color1*(0xFF-base)/0xFF;

    return static_cast<sal_uInt8>(color1+color2);
}

bool Bitmap::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam )
{
    bool bRet = false;

    switch( eFilter )
    {
        case BmpFilter::Smooth:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSmoothenFilter(pFilterParam->mnRadius));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::Sharpen:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSharpenFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::RemoveNoise:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapMedianFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::SobelGrey:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSobelGreyFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::Solarize:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSolarizeFilter(pFilterParam->mcSolarGreyThreshold));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::Sepia:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapSepiaFilter(pFilterParam->mnSepiaPercent));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::Mosaic:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapMosaicFilter(pFilterParam->maMosaicTileSize.mnTileWidth,
                                                                   pFilterParam->maMosaicTileSize.mnTileHeight));
            *this = aBmpEx.GetBitmap();
        }

        break;

        case BmpFilter::EmbossGrey:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapEmbossGreyFilter(pFilterParam->maEmbossAngles.mnAzimuthAngle100,
                                                                     pFilterParam->maEmbossAngles.mnElevationAngle100));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::PopArt:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapPopArtFilter());
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpFilter::DuoTone:
            bRet = ImplDuotoneFilter( pFilterParam->mnProgressStart, pFilterParam->mnProgressEnd );
        break;

        default:
            OSL_FAIL( "Bitmap::Convert(): Unsupported filter" );
        break;
    }

    return bRet;
}

bool Bitmap::ImplDuotoneFilter( const sal_uLong nColorOne, const sal_uLong nColorTwo )
{
    const long  nWidth = GetSizePixel().Width();
    const long  nHeight = GetSizePixel().Height();

    Bitmap aResultBitmap( GetSizePixel(), 24);
    ScopedReadAccess pReadAcc(*this);
    BitmapScopedWriteAccess pWriteAcc(aResultBitmap);
    const BitmapColor aColorOne( static_cast< sal_uInt8 >( nColorOne >> 16 ), static_cast< sal_uInt8 >( nColorOne >> 8 ), static_cast< sal_uInt8 >( nColorOne ) );
    const BitmapColor aColorTwo( static_cast< sal_uInt8 >( nColorTwo >> 16 ), static_cast< sal_uInt8 >( nColorTwo >> 8 ), static_cast< sal_uInt8 >( nColorTwo ) );

    for( long x = 0; x < nWidth; x++ )
    {
        for( long y = 0; y < nHeight; y++ )
        {
            BitmapColor aColor = pReadAcc->GetColor( y, x );
            sal_uInt8 luminance = aColor.GetLuminance();
            BitmapColor aResultColor(
                    lcl_getDuotoneColorComponent( luminance, aColorOne.GetRed(), aColorTwo.GetRed() ) ,
                    lcl_getDuotoneColorComponent( luminance, aColorOne.GetGreen(), aColorTwo.GetGreen() ) ,
                    lcl_getDuotoneColorComponent( luminance, aColorOne.GetBlue(), aColorTwo.GetBlue() ) );
            pWriteAcc->SetPixel( y, x, aResultColor );
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();
    ReassignWithSize(aResultBitmap);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
