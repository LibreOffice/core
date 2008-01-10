/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transformer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 13:23:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX_
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_IMAGE_HXX_
#include <vcl/image.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <tools/resmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/solar.hrc>
#include <vcl/salbtype.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>

#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif

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

    BitmapColor aColorFrom( static_cast< sal_uInt8 >( nColorFrom ), static_cast< sal_uInt8 >( nColorFrom >> 16 ), static_cast< sal_uInt8 >( nColorFrom >> 24 ) );
    BitmapColor aColorTo( static_cast< sal_uInt8 >( nColorTo ), static_cast< sal_uInt8 >( nColorTo >> 16 ), static_cast< sal_uInt8 >( nColorTo  >> 24 ) );

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
