/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/math.hxx>

#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/metric.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <canvas/canvastools.hxx>
#include <o3tl/compat_functional.hxx>

#include "impltools.hxx"
#include "canvasbitmap.hxx"

#include <numeric>


using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace tools
    {
        ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
        {
            
            
            CanvasBitmap* pBitmapImpl = dynamic_cast< CanvasBitmap* >( xBitmap.get() );

            if( pBitmapImpl )
            {
                return pBitmapImpl->getBitmap();
            }
            else
            {
                SpriteCanvas* pCanvasImpl = dynamic_cast< SpriteCanvas* >( xBitmap.get() );
                if( pCanvasImpl && pCanvasImpl->getBackBuffer() )
                {
                    
                    const ::OutputDevice& rDev( pCanvasImpl->getBackBuffer()->getOutDev() );
                    const ::Point aEmptyPoint;
                    return rDev.GetBitmapEx( aEmptyPoint,
                                             rDev.GetOutputSizePixel() );
                }

                
                uno::Reference< rendering::XIntegerReadOnlyBitmap > xIntBmp(
                    xBitmap, uno::UNO_QUERY_THROW );

                ::BitmapEx aBmpEx = ::vcl::unotools::bitmapExFromXBitmap( xIntBmp );
                if( !!aBmpEx )
                    return aBmpEx;

                
                ENSURE_OR_THROW( false,
                                  "bitmapExFromXBitmap(): could not extract bitmap" );
            }

            return ::BitmapEx();
        }

        bool setupFontTransform( ::Point&                       o_rPoint,
                                 ::Font&                        io_rVCLFont,
                                 const rendering::ViewState&    rViewState,
                                 const rendering::RenderState&  rRenderState,
                                 ::OutputDevice&                rOutDev )
        {
            ::basegfx::B2DHomMatrix aMatrix;

            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DTuple aScale;
            ::basegfx::B2DTuple aTranslate;
            double nRotate, nShearX;

            aMatrix.decompose( aScale, aTranslate, nRotate, nShearX );

            
            if( !::rtl::math::approxEqual(aScale.getX(), aScale.getY()) )
            {
                
                const sal_Int32 nFontWidth( rOutDev.GetFontMetric( io_rVCLFont ).GetWidth() );

                const sal_Int32 nScaledFontWidth( ::basegfx::fround(nFontWidth * aScale.getX()) );

                if( !nScaledFontWidth )
                {
                    
                    
                    return false;
                }

                io_rVCLFont.SetWidth( nScaledFontWidth );
            }

            if( !::rtl::math::approxEqual(aScale.getY(), 1.0) )
            {
                const sal_Int32 nFontHeight( io_rVCLFont.GetHeight() );
                io_rVCLFont.SetHeight( ::basegfx::fround(nFontHeight * aScale.getY()) );
            }

            io_rVCLFont.SetOrientation( static_cast< short >( ::basegfx::fround(-fmod(nRotate, 2*M_PI)*(1800.0/M_PI)) ) );

            
            o_rPoint.X() = ::basegfx::fround(aTranslate.getX());
            o_rPoint.Y() = ::basegfx::fround(aTranslate.getY());

            return true;
        }

        bool isRectangle( const PolyPolygon& rPolyPoly )
        {
            
            if( rPolyPoly.Count() != 1 )
                return false;

            const ::Polygon& rPoly( rPolyPoly[0] );

            sal_uInt16 nCount( rPoly.GetSize() );
            if( nCount < 4 )
                return false;

            
            return ::basegfx::tools::isRectangle( rPoly.getB2DPolygon() );
        }


        
        

        ::Point mapRealPoint2D( const geometry::RealPoint2D&    rPoint,
                                const rendering::ViewState&     rViewState,
                                const rendering::RenderState&   rRenderState )
        {
            ::basegfx::B2DPoint aPoint( ::basegfx::unotools::b2DPointFromRealPoint2D(rPoint) );

            ::basegfx::B2DHomMatrix aMatrix;
            aPoint *= ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                                   rViewState,
                                                                   rRenderState);

            return ::vcl::unotools::pointFromB2DPoint( aPoint );
        }

        ::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&  rPoly,
                                      const rendering::ViewState&       rViewState,
                                      const rendering::RenderState&     rRenderState )
        {
            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DPolyPolygon aTemp( rPoly );

            aTemp.transform( aMatrix );

            return ::PolyPolygon( aTemp );
        }

        ::BitmapEx transformBitmap( const BitmapEx&                 rBitmap,
                                    const ::basegfx::B2DHomMatrix&  rTransform,
                                    const uno::Sequence< double >&  rDeviceColor,
                                    ModulationMode                  eModulationMode )
        {
            SAL_INFO( "canvas.vcl", "::vclcanvas::tools::transformBitmap()" );
            SAL_INFO( "canvas.vcl", "::vclcanvas::tools::transformBitmap: 0x" << std::hex << &rBitmap );

            
            
            
            
            const Size                  aBmpSize( rBitmap.GetSizePixel() );
            ::basegfx::B2DRectangle     aDestRect;

            bool bCopyBack( false );

            
            const ::basegfx::B2DRectangle aSrcRect( 0, 0,
                                                    aBmpSize.Width(),
                                                    aBmpSize.Height() );
            ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                        aSrcRect,
                                                        rTransform );

            
            
            
            
            ::basegfx::B2DHomMatrix aLocalTransform;
            ::canvas::tools::calcRectToOriginTransform( aLocalTransform,
                                                        aSrcRect,
                                                        rTransform );

            const bool bModulateColors( eModulationMode == MODULATE_WITH_DEVICECOLOR &&
                                        rDeviceColor.getLength() > 2 );
            const double nRedModulation( bModulateColors ? rDeviceColor[0] : 1.0 );
            const double nGreenModulation( bModulateColors ? rDeviceColor[1] : 1.0 );
            const double nBlueModulation( bModulateColors ? rDeviceColor[2] : 1.0 );
            const double nAlphaModulation( bModulateColors && rDeviceColor.getLength() > 3 ?
                                           rDeviceColor[3] : 1.0 );

            Bitmap aSrcBitmap( rBitmap.GetBitmap() );
            Bitmap aSrcAlpha;

            
            
            if( rBitmap.IsTransparent() )
            {
                if( rBitmap.IsAlpha() )
                    aSrcAlpha = rBitmap.GetAlpha().GetBitmap();
                else
                    aSrcAlpha = rBitmap.GetMask();
            }

            Bitmap::ScopedReadAccess pReadAccess( aSrcBitmap );
            Bitmap::ScopedReadAccess pAlphaReadAccess( rBitmap.IsTransparent() ?
                                                     aSrcAlpha.AcquireReadAccess() :
                                                     (BitmapReadAccess*)NULL,
                                                     aSrcAlpha );

            if( pReadAccess.get() == NULL ||
                (pAlphaReadAccess.get() == NULL && rBitmap.IsTransparent()) )
            {
                
                ENSURE_OR_THROW( false,
                                  "transformBitmap(): could not access source bitmap" );
            }

            
            
            
            sal_uInt8 aAlphaMap[256];

            if( rBitmap.IsTransparent() )
            {
                if( rBitmap.IsAlpha() )
                {
                    
                    
                    ::o3tl::iota( aAlphaMap, &aAlphaMap[256], 0 );
                }
                else
                {
                    
                    const BitmapColor& rCol0( pAlphaReadAccess->GetPaletteColor( 0 ) );
                    const BitmapColor& rCol1( pAlphaReadAccess->GetPaletteColor( 1 ) );

                    
                    
                    aAlphaMap[0] = rCol0.GetRed();
                    aAlphaMap[1] = rCol1.GetRed();
                }
            }
            

            const Size aDestBmpSize( ::basegfx::fround( aDestRect.getWidth() ),
                                     ::basegfx::fround( aDestRect.getHeight() ) );

            if( aDestBmpSize.Width() == 0 || aDestBmpSize.Height() == 0 )
                return BitmapEx();

            Bitmap aDstBitmap( aDestBmpSize, aSrcBitmap.GetBitCount(), &pReadAccess->GetPalette() );
            Bitmap aDstAlpha( AlphaMask( aDestBmpSize ).GetBitmap() );

            {
                
                
                
                
                
                Bitmap::ScopedWriteAccess pWriteAccess( aDstBitmap );
                Bitmap::ScopedWriteAccess pAlphaWriteAccess( aDstAlpha );


                if( pWriteAccess.get() != NULL &&
                    pAlphaWriteAccess.get() != NULL &&
                    rTransform.isInvertible() )
                {
                    
                    
                    
                    ::basegfx::B2DHomMatrix aTransform( aLocalTransform );
                    aTransform.invert();

                    
                    for( int y=0; y<aDestBmpSize.Height(); ++y )
                    {
                        if( bModulateColors )
                        {
                            
                            
                            

                            
                            

                            
                            
                            if( rBitmap.IsTransparent() )
                            {
                                
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        
                                        
                                        
                                        
                                        
                                        
                                        
                                        const sal_uInt8 cMappedAlphaIdx = aAlphaMap[ pAlphaReadAccess->GetPixelIndex( nSrcY, nSrcX ) ];
                                        const sal_uInt8 cModulatedAlphaIdx = 255U - static_cast<sal_uInt8>( nAlphaModulation* (255U - cMappedAlphaIdx) + .5 );
                                        pAlphaWriteAccess->SetPixelIndex( y, x, cModulatedAlphaIdx );
                                        BitmapColor aColor( pReadAccess->GetPixel( nSrcY, nSrcX ) );

                                        aColor.SetRed(
                                            static_cast<sal_uInt8>(
                                                nRedModulation *
                                                aColor.GetRed() + .5 ));
                                        aColor.SetGreen(
                                            static_cast<sal_uInt8>(
                                                nGreenModulation *
                                                aColor.GetGreen() + .5 ));
                                        aColor.SetBlue(
                                            static_cast<sal_uInt8>(
                                                nBlueModulation *
                                                aColor.GetBlue() + .5 ));

                                        pWriteAccess->SetPixel( y, x,
                                                                aColor );
                                    }
                                }
                            }
                            else
                            {
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        
                                        
                                        
                                        
                                        
                                        
                                        
                                        pAlphaWriteAccess->SetPixel( y, x,
                                                                     BitmapColor(
                                                                         255U -
                                                                         static_cast<sal_uInt8>(
                                                                             nAlphaModulation*255.0
                                                                             + .5 ) ) );

                                        BitmapColor aColor( pReadAccess->GetPixel( nSrcY,
                                                                                   nSrcX ) );

                                        aColor.SetRed(
                                            static_cast<sal_uInt8>(
                                                nRedModulation *
                                                aColor.GetRed() + .5 ));
                                        aColor.SetGreen(
                                            static_cast<sal_uInt8>(
                                                nGreenModulation *
                                                aColor.GetGreen() + .5 ));
                                        aColor.SetBlue(
                                            static_cast<sal_uInt8>(
                                                nBlueModulation *
                                                aColor.GetBlue() + .5 ));

                                        pWriteAccess->SetPixel( y, x,
                                                                aColor );
                                    }
                                }
                            }
                        }
                        else
                        {
                            
                            
                            if( rBitmap.IsTransparent() )
                            {
                                
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixelIndex( y, x, 255 );
                                    }
                                    else
                                    {
                                        const sal_uInt8 cAlphaIdx = pAlphaReadAccess->GetPixelIndex( nSrcY, nSrcX );
                                        pAlphaWriteAccess->SetPixelIndex( y, x, aAlphaMap[ cAlphaIdx ] );
                                        pWriteAccess->SetPixel( y, x, pReadAccess->GetPixel( nSrcY, nSrcX ) );
                                    }
                                }
                            }
                            else
                            {
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(0) );
                                        pWriteAccess->SetPixel( y, x, pReadAccess->GetPixel( nSrcY,
                                                                                             nSrcX ) );
                                    }
                                }
                            }
                        }
                    }

                    bCopyBack = true;
                }
                else
                {
                    
                    ENSURE_OR_THROW( false,
                                      "transformBitmap(): could not access bitmap" );
                }
            }

            if( bCopyBack )
                return BitmapEx( aDstBitmap, AlphaMask( aDstAlpha ) );
            else
                return BitmapEx();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
