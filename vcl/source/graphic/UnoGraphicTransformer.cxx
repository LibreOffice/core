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


#include <graphic/UnoGraphicTransformer.hxx>

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/metaact.hxx>
#include <svl/solar.hrc>
#include <vcl/virdev.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapDuoToneFilter.hxx>

#include <com/sun/star/text/GraphicCrop.hpp>

using namespace com::sun::star;

namespace unographic {


GraphicTransformer::GraphicTransformer()
{
}


GraphicTransformer::~GraphicTransformer()
{
}


// XGraphicTransformer
uno::Reference< graphic::XGraphic > SAL_CALL GraphicTransformer::colorChange(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
{
    ::Graphic aGraphic(rxGraphic);
    ::Graphic aReturnGraphic;

    BitmapColor aBmpColorFrom(static_cast< sal_uInt8 >(nColorFrom), static_cast< sal_uInt8 >(nColorFrom >> 8), static_cast< sal_uInt8 >(nColorFrom >> 16));
    BitmapColor aBmpColorTo( static_cast< sal_uInt8 >(nColorTo), static_cast< sal_uInt8 >(nColorTo >> 8), static_cast< sal_uInt8 >(nColorTo  >> 16));

    Color aColorFrom(aBmpColorFrom);
    Color aColorTo(aBmpColorTo);

    const sal_uInt8 cIndexFrom = aBmpColorFrom.GetIndex();

    if (aGraphic.GetType() == GraphicType::Bitmap ||
        aGraphic.GetType() == GraphicType::GdiMetafile)
    {
        BitmapEx aBitmapEx(aGraphic.GetBitmapEx());
        Bitmap aBitmap(aBitmapEx.GetBitmap());

        if (aBitmapEx.IsAlpha())
        {
            aBitmapEx.setAlphaFrom( cIndexFrom, nAlphaTo );
            aBitmapEx.Replace(aColorFrom, aColorTo, nTolerance);
            aReturnGraphic = ::Graphic(aBitmapEx);
        }
        else if (aBitmapEx.IsTransparent())
        {
            if (nAlphaTo == sal::static_int_cast< sal_Int8 >(0xff))
            {
                Bitmap aMask(aBitmapEx.GetMask());
                Bitmap aMask2(aBitmap.CreateMask(aColorFrom, nTolerance));
                aMask.CombineSimple(aMask2, BmpCombine::Or);
                aBitmap.Replace(aColorFrom, aColorTo, nTolerance);
                aReturnGraphic = ::Graphic(BitmapEx(aBitmap, aMask));
            }
            else
            {
                aBitmapEx.setAlphaFrom(cIndexFrom, 0xff - nAlphaTo);
                aBitmapEx.Replace(aColorFrom, aColorTo, nTolerance);
                aReturnGraphic = ::Graphic(aBitmapEx);
            }
        }
        else
        {
            if ((nAlphaTo == 0) || (nAlphaTo == sal::static_int_cast< sal_Int8 >(0xff)))
            {
                Bitmap aMask(aBitmap.CreateMask(aColorFrom, nTolerance));
                aBitmap.Replace(aColorFrom, aColorTo, nTolerance);
                aReturnGraphic = ::Graphic(BitmapEx(aBitmap, aMask));
            }
            else
            {
                aBitmapEx.setAlphaFrom(cIndexFrom, nAlphaTo);
                aBitmapEx.Replace(aColorFrom, aColorTo, nTolerance);
                aReturnGraphic = ::Graphic(aBitmapEx);
            }
        }
    }

    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());
    return aReturnGraphic.GetXGraphic();
}

uno::Reference< graphic::XGraphic > SAL_CALL GraphicTransformer::applyDuotone(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nColorOne, sal_Int32 nColorTwo )
{
    ::Graphic aGraphic(rxGraphic);
    ::Graphic aReturnGraphic;

    BitmapEx    aBitmapEx( aGraphic.GetBitmapEx() );
    AlphaMask   aMask( aBitmapEx.GetAlpha() );
    Bitmap      aBitmap( aBitmapEx.GetBitmap() );

    BitmapEx    aTmpBmpEx(aBitmap);
    BitmapFilter::Filter(aTmpBmpEx, BitmapDuoToneFilter(static_cast<sal_uLong>(nColorOne), static_cast<sal_uLong>(nColorTwo)));
    aBitmap = aTmpBmpEx.GetBitmap();

    aReturnGraphic = ::Graphic( BitmapEx( aBitmap, aMask ) );
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());
    return aReturnGraphic.GetXGraphic();
}

uno::Reference< graphic::XGraphic > SAL_CALL GraphicTransformer::applyBrightnessContrast(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nBrightness, sal_Int32 nContrast, sal_Bool mso )
{
    ::Graphic aGraphic(rxGraphic);
    ::Graphic aReturnGraphic;

    BitmapEx aBitmapEx(aGraphic.GetBitmapEx());
    aBitmapEx.Adjust(nBrightness, nContrast, 0, 0, 0, 0, false, mso);
    aReturnGraphic = ::Graphic(aBitmapEx);
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());
    return aReturnGraphic.GetXGraphic();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
