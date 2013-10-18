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


#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/metaact.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svl/solar.hrc>
#include <vcl/salbtype.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>
#include <com/sun/star/text/GraphicCrop.hpp>

#include "graphic.hxx"
#include "transformer.hxx"

using namespace com::sun::star;

namespace unographic {

// ----------------------
// - GraphicTransformer -
// ----------------------

GraphicTransformer::GraphicTransformer()
{
}

// ------------------------------------------------------------------------------

GraphicTransformer::~GraphicTransformer()
{
}

// ------------------------------------------------------------------------------

void setAlpha( Bitmap& rBitmap, AlphaMask& rAlpha, sal_uInt8 cIndexFrom, sal_Int8 nAlphaTo )
{
    BitmapWriteAccess* pWriteAccess = rAlpha.AcquireWriteAccess();
    BitmapReadAccess* pReadAccess = rBitmap.AcquireReadAccess();
    if ( pReadAccess && pWriteAccess )
    {
        for ( sal_Int32 nY = 0; nY < pReadAccess->Height(); nY++ )
        {
            for ( sal_Int32 nX = 0; nX < pReadAccess->Width(); nX++ )
            {
                const sal_uInt8 cIndex = pReadAccess->GetPixelIndex( nY, nX );
                if ( cIndex == cIndexFrom )
                    pWriteAccess->SetPixelIndex( nY, nX, nAlphaTo );
            }
        }
    }
    rBitmap.ReleaseAccess( pReadAccess );
    rAlpha.ReleaseAccess( pWriteAccess );
}

// XGraphicTransformer
uno::Reference< graphic::XGraphic > SAL_CALL GraphicTransformer::colorChange(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
        throw ( lang::IllegalArgumentException, uno::RuntimeException)
{
    const uno::Reference< uno::XInterface > xIFace( rxGraphic, uno::UNO_QUERY );
    ::Graphic aGraphic( *::unographic::Graphic::getImplementation( xIFace ) );

    BitmapColor aColorFrom( static_cast< sal_uInt8 >( nColorFrom ), static_cast< sal_uInt8 >( nColorFrom >> 8 ), static_cast< sal_uInt8 >( nColorFrom >> 16 ) );
    BitmapColor aColorTo( static_cast< sal_uInt8 >( nColorTo ), static_cast< sal_uInt8 >( nColorTo >> 8 ), static_cast< sal_uInt8 >( nColorTo  >> 16 ) );
    const sal_uInt8 cIndexFrom = aColorFrom.GetBlueOrIndex();

    if ( aGraphic.GetType() == GRAPHIC_BITMAP )
    {
        BitmapEx    aBitmapEx( aGraphic.GetBitmapEx() );
        Bitmap      aBitmap( aBitmapEx.GetBitmap() );

        if ( aBitmapEx.IsAlpha() )
        {
            AlphaMask aAlphaMask( aBitmapEx.GetAlpha() );
            setAlpha( aBitmap, aAlphaMask, cIndexFrom, nAlphaTo );
            aBitmap.Replace( aColorFrom, aColorTo, nTolerance );
            aGraphic = ::Graphic( BitmapEx( aBitmap, aAlphaMask ) );
        }
        else if ( aBitmapEx.IsTransparent() )
        {
            if ( ( nAlphaTo == 0 ) || ( nAlphaTo == sal::static_int_cast<sal_Int8>(0xff) ) )
            {
                Bitmap aMask( aBitmapEx.GetMask() );
                Bitmap aMask2( aBitmap.CreateMask( aColorFrom, nTolerance ) );
                aMask.CombineSimple( aMask2, BMP_COMBINE_OR );
                aBitmap.Replace( aColorFrom, aColorTo, nTolerance );
                aGraphic = ::Graphic( BitmapEx( aBitmap, aMask ) );
            }
            else
            {
                AlphaMask aAlphaMask( aBitmapEx.GetMask() );
                setAlpha( aBitmap, aAlphaMask, cIndexFrom, nAlphaTo );
                aBitmap.Replace( aColorFrom, aColorTo, nTolerance );
                aGraphic = ::Graphic( BitmapEx( aBitmap, aAlphaMask ) );
            }
        }
        else
        {
            if ( ( nAlphaTo == 0 ) || ( nAlphaTo == sal::static_int_cast<sal_Int8>(0xff) ) )
            {
                Bitmap aMask( aBitmap.CreateMask( aColorFrom, nTolerance ) );
                    aBitmap.Replace( aColorFrom, aColorTo, nTolerance );
                aGraphic = ::Graphic( BitmapEx( aBitmap, aMask ) );
            }
            else
            {
                AlphaMask aAlphaMask( aBitmapEx.GetSizePixel() );
                setAlpha( aBitmap, aAlphaMask, cIndexFrom, nAlphaTo );
                aBitmap.Replace( aColorFrom, aColorTo, nTolerance );
                aGraphic = ::Graphic( BitmapEx( aBitmap, aAlphaMask ) );
            }
        }
    }
    ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic();
    pUnoGraphic->init( aGraphic );
    uno::Reference< graphic::XGraphic > xRet( pUnoGraphic );
    return xRet;
}

uno::Reference< graphic::XGraphic > SAL_CALL GraphicTransformer::applyDuotone(
    const uno::Reference< graphic::XGraphic >& rxGraphic, sal_Int32 nColorOne, sal_Int32 nColorTwo )
        throw ( lang::IllegalArgumentException, uno::RuntimeException)
{
    const uno::Reference< uno::XInterface > xIFace( rxGraphic, uno::UNO_QUERY );
    ::Graphic aGraphic( *::unographic::Graphic::getImplementation( xIFace ) );

    BitmapEx    aBitmapEx( aGraphic.GetBitmapEx() );
    Bitmap      aBitmap( aBitmapEx.GetBitmap() );
    BmpFilterParam aFilter( (sal_uLong) nColorOne, (sal_uLong) nColorTwo );
    aBitmap.Filter( BMP_FILTER_DUOTONE, &aFilter );
    aGraphic = ::Graphic( BitmapEx( aBitmap ) );

    ::unographic::Graphic* pUnoGraphic = new ::unographic::Graphic();
    pUnoGraphic->init( aGraphic );
    uno::Reference< graphic::XGraphic > xRet( pUnoGraphic );
    return xRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
