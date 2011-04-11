/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svtools.hxx"

#include <rtl/uuid.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/metaact.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <tools/resmgr.hxx>
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

void setAlpha( Bitmap& rBitmap, AlphaMask& rAlpha, sal_Int32 nColorFrom, sal_Int8 nAlphaTo )
{
    BitmapWriteAccess* pWriteAccess = rAlpha.AcquireWriteAccess();
    BitmapReadAccess* pReadAccess = rBitmap.AcquireReadAccess();
    BitmapColor aColorFrom( static_cast< sal_uInt8 >( nColorFrom >> 16 ),
        static_cast< sal_uInt8 >( nColorFrom >> 8 ),
        static_cast< sal_uInt8 >( nColorFrom ) );
    if ( pReadAccess && pWriteAccess )
    {
        for ( sal_Int32 nY = 0; nY < pReadAccess->Height(); nY++ )
        {
            for ( sal_Int32 nX = 0; nX < pReadAccess->Width(); nX++ )
            {
                BitmapColor aColor( pReadAccess->GetPixel( nY, nX ) );
                if ( aColor == aColorFrom )
                    pWriteAccess->SetPixel( nY, nX, nAlphaTo );
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

    if ( aGraphic.GetType() == GRAPHIC_BITMAP )
    {
        BitmapEx    aBitmapEx( aGraphic.GetBitmapEx() );
        Bitmap      aBitmap( aBitmapEx.GetBitmap() );

        if ( aBitmapEx.IsAlpha() )
        {
            AlphaMask aAlphaMask( aBitmapEx.GetAlpha() );
            setAlpha( aBitmap, aAlphaMask, aColorFrom, nAlphaTo );
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
                setAlpha( aBitmap, aAlphaMask, aColorFrom, nAlphaTo );
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
                setAlpha( aBitmap, aAlphaMask, aColorFrom, nAlphaTo );
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
