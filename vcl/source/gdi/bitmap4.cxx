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
#include <vcl/BitmapPopArtFilter.hxx>
#include <vcl/BitmapDuoToneFilter.hxx>

#include <bitmapwriteaccess.hxx>

#include <memory>
#include <stdlib.h>

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
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapDuoToneFilter(pFilterParam->mnProgressStart,
                                                                    pFilterParam->mnProgressEnd));
            *this = aBmpEx.GetBitmap();
        }
        break;

        default:
            OSL_FAIL( "Bitmap::Convert(): Unsupported filter" );
        break;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
