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

#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <tools/poly.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/tools/tools.hxx>
#include <basegfx/tools/lerp.hxx>
#include <basegfx/tools/keystoplerp.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <comphelper/sequence.hxx>

#include <canvas/canvastools.hxx>
#include <canvas/parametricpolypolygon.hxx>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include "spritecanvas.hxx"
#include "canvashelper.hxx"
#include "impltools.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        bool textureFill( OutputDevice&         rOutDev,
                          GraphicObject&        rGraphic,
                          const ::Point&        rPosPixel,
                          const ::Size&         rNextTileX,
                          const ::Size&         rNextTileY,
                          sal_Int32             nTilesX,
                          sal_Int32             nTilesY,
                          const ::Size&         rTileSize,
                          const GraphicAttr&    rAttr)
        {
            bool bRet( false );
            Point   aCurrPos;
            int     nX, nY;

            for( nY=0; nY < nTilesY; ++nY )
            {
                aCurrPos.X() = rPosPixel.X() + nY*rNextTileY.Width();
                aCurrPos.Y() = rPosPixel.Y() + nY*rNextTileY.Height();

                for( nX=0; nX < nTilesX; ++nX )
                {
                    
                    
                    bRet |= rGraphic.Draw( &rOutDev,
                                           aCurrPos,
                                           rTileSize,
                                           &rAttr );

                    aCurrPos.X() += rNextTileX.Width();
                    aCurrPos.Y() += rNextTileX.Height();
                }
            }

            return bRet;
        }


        /** Fill linear or axial gradient

            Since most of the code for linear and axial gradients are
            the same, we've a unified method here
         */
        void fillLinearGradient( OutputDevice&                                  rOutDev,
                                 const ::basegfx::B2DHomMatrix&                 rTextureTransform,
                                 const ::Rectangle&                             rBounds,
                                 unsigned int                                   nStepCount,
                                 const ::canvas::ParametricPolyPolygon::Values& rValues,
                                 const std::vector< ::Color >&                  rColors )
        {
            
            
            

            ::basegfx::B2DPoint aLeftTop( 0.0, 0.0 );
            ::basegfx::B2DPoint aLeftBottom( 0.0, 1.0 );
            ::basegfx::B2DPoint aRightTop( 1.0, 0.0 );
            ::basegfx::B2DPoint aRightBottom( 1.0, 1.0 );

            aLeftTop    *= rTextureTransform;
            aLeftBottom *= rTextureTransform;
            aRightTop   *= rTextureTransform;
            aRightBottom*= rTextureTransform;

            
            const ::basegfx::B2DVector aBoundRectDiagonal(
                ::vcl::unotools::b2DPointFromPoint( rBounds.TopLeft() ) -
                ::vcl::unotools::b2DPointFromPoint( rBounds.BottomRight() ) );
            const double nDiagonalLength( aBoundRectDiagonal.getLength() );

            
            
            
            
            
            
            ::basegfx::B2DVector aDirection( aRightTop - aLeftTop );
            aDirection.normalize();

            
            
            
            ::basegfx::tools::infiniteLineFromParallelogram( aLeftTop,
                                                             aLeftBottom,
                                                             aRightTop,
                                                             aRightBottom,
                                                             ::vcl::unotools::b2DRectangleFromRectangle( rBounds ) );


            
            

            
            
            

            
            
            
            
            
            ::Polygon aTempPoly( static_cast<sal_uInt16>(5) );

            OSL_ENSURE( nStepCount >= 3,
                        "fillLinearGradient(): stepcount smaller than 3" );


            
            
            

            
            
            
            
            
            const ::basegfx::B2DPoint& rPoint1( aLeftTop - 2.0*nDiagonalLength*aDirection );
            aTempPoly[1] = ::Point( ::basegfx::fround( rPoint1.getX() ),
                                    ::basegfx::fround( rPoint1.getY() ) );

            const ::basegfx::B2DPoint& rPoint2( aLeftBottom - 2.0*nDiagonalLength*aDirection );
            aTempPoly[2] = ::Point( ::basegfx::fround( rPoint2.getX() ),
                                    ::basegfx::fround( rPoint2.getY() ) );


            
            

            
            
            
            if( (rColors.size() % 2) != (nStepCount % 2) )
                ++nStepCount;

            rOutDev.SetLineColor();

            basegfx::tools::KeyStopLerp aLerper(rValues.maStops);

            
            
            for( unsigned int i=0; i<nStepCount-1; ++i )
            {
                std::ptrdiff_t nIndex;
                double fAlpha;
                boost::tuples::tie(nIndex,fAlpha)=aLerper.lerp(double(i)/nStepCount);

                rOutDev.SetFillColor(
                    Color( (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetRed(),rColors[nIndex+1].GetRed(),fAlpha)),
                           (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetGreen(),rColors[nIndex+1].GetGreen(),fAlpha)),
                           (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetBlue(),rColors[nIndex+1].GetBlue(),fAlpha)) ));

                
                
                aTempPoly[0] = aTempPoly[4] = aTempPoly[1];
                aTempPoly[3] = aTempPoly[2];

                
                
                
                
                
                const ::basegfx::B2DPoint& rPoint3(
                    (nStepCount - i-1)/double(nStepCount)*aLeftTop +
                    (i+1)/double(nStepCount)*aRightTop );
                aTempPoly[1] = ::Point( ::basegfx::fround( rPoint3.getX() ),
                                        ::basegfx::fround( rPoint3.getY() ) );

                const ::basegfx::B2DPoint& rPoint4(
                    (nStepCount - i-1)/double(nStepCount)*aLeftBottom +
                    (i+1)/double(nStepCount)*aRightBottom );
                aTempPoly[2] = ::Point( ::basegfx::fround( rPoint4.getX() ),
                                        ::basegfx::fround( rPoint4.getY() ) );

                rOutDev.DrawPolygon( aTempPoly );
            }

            
            
            

            
            
            aTempPoly[0] = aTempPoly[4] = aTempPoly[1];
            aTempPoly[3] = aTempPoly[2];

            
            
            
            const ::basegfx::B2DPoint& rPoint3( aRightTop + 2.0*nDiagonalLength*aDirection );
            aTempPoly[0] = aTempPoly[4] = ::Point( ::basegfx::fround( rPoint3.getX() ),
                                                   ::basegfx::fround( rPoint3.getY() ) );

            const ::basegfx::B2DPoint& rPoint4( aRightBottom + 2.0*nDiagonalLength*aDirection );
            aTempPoly[3] = ::Point( ::basegfx::fround( rPoint4.getX() ),
                                    ::basegfx::fround( rPoint4.getY() ) );

            rOutDev.SetFillColor( rColors.back() );

            rOutDev.DrawPolygon( aTempPoly );
        }

        void fillPolygonalGradient( OutputDevice&                                  rOutDev,
                                    const ::basegfx::B2DHomMatrix&                 rTextureTransform,
                                    const ::Rectangle&                             rBounds,
                                    unsigned int                                   nStepCount,
                                    bool                                           bFillNonOverlapping,
                                    const ::canvas::ParametricPolyPolygon::Values& rValues,
                                    const std::vector< ::Color >&                  rColors )
        {
            const ::basegfx::B2DPolygon& rGradientPoly( rValues.maGradientPoly );

            ENSURE_OR_THROW( rGradientPoly.count() > 2,
                              "fillPolygonalGradient(): polygon without area given" );

            
            
            
            
            ::basegfx::B2DPolygon   aOuterPoly( rGradientPoly );
            ::basegfx::B2DPolygon   aInnerPoly;

            
            
            if( aOuterPoly.areControlPointsUsed() )
                aOuterPoly = ::basegfx::tools::adaptiveSubdivideByAngle(aOuterPoly);

            aInnerPoly = aOuterPoly;

            
            
            
            aOuterPoly.transform( rTextureTransform );

            
            
            ::basegfx::B2DHomMatrix aInnerPolygonTransformMatrix;


            
            

            
            
            
            
            
            const double nAspectRatio( rValues.mnAspectRatio );
            if( nAspectRatio > 1.0 )
            {
                
                aInnerPolygonTransformMatrix.scale( 1.0 - 1.0/nAspectRatio,
                                                    0.0 );
            }
            else if( nAspectRatio < 1.0 )
            {
                
                aInnerPolygonTransformMatrix.scale( 0.0,
                                                    1.0 - nAspectRatio );
            }
            else
            {
                
                aInnerPolygonTransformMatrix.scale( 0.0, 0.0 );
            }

            
            aInnerPolygonTransformMatrix *= rTextureTransform;

            
            aInnerPoly.transform( aInnerPolygonTransformMatrix );


            const sal_uInt32 nNumPoints( aOuterPoly.count() );
            ::Polygon        aTempPoly( static_cast<sal_uInt16>(nNumPoints+1) );

            
            
            
            
            
            //
            
            
            
            
            //
            
            
            
            ++nStepCount;

            rOutDev.SetLineColor();

            basegfx::tools::KeyStopLerp aLerper(rValues.maStops);

            if( !bFillNonOverlapping )
            {
                
                rOutDev.SetFillColor( rColors.front() );
                rOutDev.DrawRect( rBounds );

                
                

                for( unsigned int i=1,p; i<nStepCount; ++i )
                {
                    const double fT( i/double(nStepCount) );

                    std::ptrdiff_t nIndex;
                    double fAlpha;
                    boost::tuples::tie(nIndex,fAlpha)=aLerper.lerp(fT);

                    
                    rOutDev.SetFillColor(
                        Color( (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetRed(),rColors[nIndex+1].GetRed(),fAlpha)),
                               (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetGreen(),rColors[nIndex+1].GetGreen(),fAlpha)),
                               (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetBlue(),rColors[nIndex+1].GetBlue(),fAlpha)) ));

                    
                    

                    for( p=0; p<nNumPoints; ++p )
                    {
                        const ::basegfx::B2DPoint& rOuterPoint( aOuterPoly.getB2DPoint(p) );
                        const ::basegfx::B2DPoint& rInnerPoint( aInnerPoly.getB2DPoint(p) );

                        aTempPoly[(sal_uInt16)p] = ::Point(
                            basegfx::fround( fT*rInnerPoint.getX() + (1-fT)*rOuterPoint.getX() ),
                            basegfx::fround( fT*rInnerPoint.getY() + (1-fT)*rOuterPoint.getY() ) );
                    }

                    
                    aTempPoly[(sal_uInt16)p] = aTempPoly[0];

                    
                    
                    
                    
                    rOutDev.DrawPolygon( aTempPoly );
                }
            }
            else
            {
                
                

                
                
                
                
                ::PolyPolygon           aTempPolyPoly;
                ::Polygon               aTempPoly2( static_cast<sal_uInt16>(nNumPoints+1) );

                aTempPoly2[0] = rBounds.TopLeft();
                aTempPoly2[1] = rBounds.TopRight();
                aTempPoly2[2] = rBounds.BottomRight();
                aTempPoly2[3] = rBounds.BottomLeft();
                aTempPoly2[4] = rBounds.TopLeft();

                aTempPolyPoly.Insert( aTempPoly );
                aTempPolyPoly.Insert( aTempPoly2 );

                for( unsigned int i=0,p; i<nStepCount; ++i )
                {
                    const double fT( (i+1)/double(nStepCount) );

                    std::ptrdiff_t nIndex;
                    double fAlpha;
                    boost::tuples::tie(nIndex,fAlpha)=aLerper.lerp(fT);

                    
                    rOutDev.SetFillColor(
                        Color( (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetRed(),rColors[nIndex+1].GetRed(),fAlpha)),
                               (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetGreen(),rColors[nIndex+1].GetGreen(),fAlpha)),
                               (sal_uInt8)(basegfx::tools::lerp(rColors[nIndex].GetBlue(),rColors[nIndex+1].GetBlue(),fAlpha)) ));

#if OSL_DEBUG_LEVEL > 2
                    if( i && !(i % 10) )
                        rOutDev.SetFillColor( COL_RED );
#endif

                    
                    
                    

                    for( p=0; p<nNumPoints; ++p )
                    {
                        const ::basegfx::B2DPoint& rOuterPoint( aOuterPoly.getB2DPoint(p) );
                        const ::basegfx::B2DPoint& rInnerPoint( aInnerPoly.getB2DPoint(p) );

                        aTempPoly[(sal_uInt16)p] = ::Point(
                            basegfx::fround( fT*rInnerPoint.getX() + (1-fT)*rOuterPoint.getX() ),
                            basegfx::fround( fT*rInnerPoint.getY() + (1-fT)*rOuterPoint.getY() ) );
                    }

                    
                    aTempPoly[(sal_uInt16)p] = aTempPoly[0];

                    
                    aTempPolyPoly.Replace( aTempPolyPoly.GetObject( 1 ), 0 );

                    if( i+1<nStepCount )
                    {
                        
                        
                        
                        
                        
                        aTempPolyPoly.Replace( aTempPoly, 1 );
                    }
                    else
                    {
                        
                        
                        
                        aTempPolyPoly.Remove( 1 );
                    }

                    rOutDev.DrawPolyPolygon( aTempPolyPoly );
                }
            }
        }

        void doGradientFill( OutputDevice&                                  rOutDev,
                             const ::canvas::ParametricPolyPolygon::Values& rValues,
                             const std::vector< ::Color >&                  rColors,
                             const ::basegfx::B2DHomMatrix&                 rTextureTransform,
                             const ::Rectangle&                             rBounds,
                             unsigned int                                   nStepCount,
                             bool                                           bFillNonOverlapping )
        {
            switch( rValues.meType )
            {
                case ::canvas::ParametricPolyPolygon::GRADIENT_LINEAR:
                    fillLinearGradient( rOutDev,
                                        rTextureTransform,
                                        rBounds,
                                        nStepCount,
                                        rValues,
                                        rColors );
                    break;

                case ::canvas::ParametricPolyPolygon::GRADIENT_ELLIPTICAL:
                    
                case ::canvas::ParametricPolyPolygon::GRADIENT_RECTANGULAR:
                    fillPolygonalGradient( rOutDev,
                                           rTextureTransform,
                                           rBounds,
                                           nStepCount,
                                           bFillNonOverlapping,
                                           rValues,
                                           rColors );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                      "CanvasHelper::doGradientFill(): Unexpected case" );
            }
        }

        int numColorSteps( const ::Color& rColor1, const ::Color& rColor2 )
        {
            return ::std::max(
                labs( rColor1.GetRed() - rColor2.GetRed() ),
                ::std::max(
                    labs( rColor1.GetGreen() - rColor2.GetGreen() ),
                    labs( rColor1.GetBlue()  - rColor2.GetBlue() ) ) );
        }

        bool gradientFill( OutputDevice&                                   rOutDev,
                           OutputDevice*                                   p2ndOutDev,
                           const ::canvas::ParametricPolyPolygon::Values&  rValues,
                           const std::vector< ::Color >&                   rColors,
                           const PolyPolygon&                              rPoly,
                           const rendering::ViewState&                     viewState,
                           const rendering::RenderState&                   renderState,
                           const rendering::Texture&                       texture,
                           int                                             nTransparency )
        {
            
            
            
            

            
            
            int nColorSteps = 0;
            for( size_t i=0; i<rColors.size()-1; ++i )
                nColorSteps += numColorSteps(rColors[i],rColors[i+1]);

            ::basegfx::B2DHomMatrix aTotalTransform;
            const int nStepCount=
                ::canvas::tools::calcGradientStepCount(aTotalTransform,
                                                       viewState,
                                                       renderState,
                                                       texture,
                                                       nColorSteps);

            rOutDev.SetLineColor();

            
            
            const ::Rectangle aPolygonDeviceRectOrig(
                rPoly.GetBoundRect() );

            if( tools::isRectangle( rPoly ) )
            {
                
                

                
                
                
                
                
                

                rOutDev.Push( PUSH_CLIPREGION );
                rOutDev.IntersectClipRegion( aPolygonDeviceRectOrig );
                doGradientFill( rOutDev,
                                rValues,
                                rColors,
                                aTotalTransform,
                                aPolygonDeviceRectOrig,
                                nStepCount,
                                false );
                rOutDev.Pop();

                if( p2ndOutDev && nTransparency < 253 )
                {
                    
                    
                    
                    
                    p2ndOutDev->SetFillColor( COL_BLACK );
                    p2ndOutDev->DrawRect( aPolygonDeviceRectOrig );
                }
            }
            else
            {
                const Region aPolyClipRegion( rPoly );

                rOutDev.Push( PUSH_CLIPREGION );
                rOutDev.SetClipRegion( aPolyClipRegion );

                doGradientFill( rOutDev,
                                rValues,
                                rColors,
                                aTotalTransform,
                                aPolygonDeviceRectOrig,
                                nStepCount,
                                false );
                rOutDev.Pop();

                if( p2ndOutDev && nTransparency < 253 )
                {
                    
                    
                    
                    
                    p2ndOutDev->SetFillColor( COL_BLACK );
                    p2ndOutDev->DrawPolyPolygon( rPoly );
                }
            }

#if OSL_DEBUG_LEVEL > 3
            
            {
                ::basegfx::B2DRectangle aRect(0.0, 0.0, 1.0, 1.0);
                ::basegfx::B2DRectangle aTextureDeviceRect;
                ::basegfx::B2DHomMatrix aTextureTransform;
                ::canvas::tools::calcTransformedRectBounds( aTextureDeviceRect,
                                                            aRect,
                                                            aTextureTransform );
                rOutDev.SetLineColor( COL_RED );
                rOutDev.SetFillColor();
                rOutDev.DrawRect( ::vcl::unotools::rectangleFromB2DRectangle( aTextureDeviceRect ) );

                rOutDev.SetLineColor( COL_BLUE );
                ::Polygon aPoly1(
                    ::vcl::unotools::rectangleFromB2DRectangle( aRect ));
                ::basegfx::B2DPolygon aPoly2( aPoly1.getB2DPolygon() );
                aPoly2.transform( aTextureTransform );
                ::Polygon aPoly3( aPoly2 );
                rOutDev.DrawPolygon( aPoly3 );
            }
#endif

            return true;
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas*                          pCanvas,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "CanvasHelper::fillPolyPolygon(): polygon is NULL");
        ENSURE_ARG_OR_THROW( textures.getLength(),
                         "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

        if( mpOutDev )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );

            const int nTransparency( setupOutDevState( viewState, renderState, IGNORE_COLOR ) );
            PolyPolygon aPolyPoly( tools::mapPolyPolygon(
                                       ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon),
                                       viewState, renderState ) );

            
            if( textures[0].Gradient.is() )
            {
                
                
                ::canvas::ParametricPolyPolygon* pGradient =
                      dynamic_cast< ::canvas::ParametricPolyPolygon* >( textures[0].Gradient.get() );

                if( pGradient && pGradient->getValues().maColors.getLength() )
                {
                    
                    
                    const ::canvas::ParametricPolyPolygon::Values& rValues(
                        pGradient->getValues() );

                    if( rValues.maColors.getLength() < 2 )
                    {
                        rendering::RenderState aTempState=renderState;
                        aTempState.DeviceColor = rValues.maColors[0];
                        fillPolyPolygon(pCanvas, xPolyPolygon, viewState, aTempState);
                    }
                    else
                    {
                        std::vector< ::Color > aColors(rValues.maColors.getLength());
                        std::transform(&rValues.maColors[0],
                                       &rValues.maColors[0]+rValues.maColors.getLength(),
                                       aColors.begin(),
                                       boost::bind(
                                           &vcl::unotools::stdColorSpaceSequenceToColor,
                                           _1));

                        
                        
                        gradientFill( mpOutDev->getOutDev(),
                                      mp2ndOutDev.get() ? &mp2ndOutDev->getOutDev() : (OutputDevice*)NULL,
                                      rValues,
                                      aColors,
                                      aPolyPoly,
                                      viewState,
                                      renderState,
                                      textures[0],
                                      nTransparency );
                    }
                }
                else
                {
                    
                    ENSURE_OR_THROW( false,
                                      "CanvasHelper::fillTexturedPolyPolygon(): unknown parametric polygon encountered" );
                }
            }
            else if( textures[0].Bitmap.is() )
            {
                const geometry::IntegerSize2D aBmpSize( textures[0].Bitmap->getSize() );

                ENSURE_ARG_OR_THROW( aBmpSize.Width != 0 &&
                                 aBmpSize.Height != 0,
                                 "CanvasHelper::fillTexturedPolyPolygon(): zero-sized texture bitmap" );

                
                
                const ::Rectangle aPolygonDeviceRect(
                    aPolyPoly.GetBoundRect() );


                
                
                

                const bool bRectangularPolygon( tools::isRectangle( aPolyPoly ) );

                ::basegfx::B2DHomMatrix aTotalTransform;
                ::canvas::tools::mergeViewAndRenderTransform(aTotalTransform,
                                                             viewState,
                                                             renderState);
                ::basegfx::B2DHomMatrix aTextureTransform;
                ::basegfx::unotools::homMatrixFromAffineMatrix( aTextureTransform,
                                                                textures[0].AffineTransform );

                aTotalTransform *= aTextureTransform;

                const ::basegfx::B2DRectangle aRect(0.0, 0.0, 1.0, 1.0);
                ::basegfx::B2DRectangle aTextureDeviceRect;
                ::canvas::tools::calcTransformedRectBounds( aTextureDeviceRect,
                                                            aRect,
                                                            aTotalTransform );

                const ::Rectangle aIntegerTextureDeviceRect(
                    ::vcl::unotools::rectangleFromB2DRectangle( aTextureDeviceRect ) );

                if( bRectangularPolygon &&
                    aIntegerTextureDeviceRect == aPolygonDeviceRect )
                {
                    rendering::RenderState aLocalState( renderState );
                    ::canvas::tools::appendToRenderState(aLocalState,
                                                         aTextureTransform);
                    ::basegfx::B2DHomMatrix aScaleCorrection;
                    aScaleCorrection.scale( 1.0/aBmpSize.Width,
                                            1.0/aBmpSize.Height );
                    ::canvas::tools::appendToRenderState(aLocalState,
                                                         aScaleCorrection);

                    
                    if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                   1.0 ) )
                    {
                        
                        aLocalState.DeviceColor.realloc(4);
                        double* pColor = aLocalState.DeviceColor.getArray();
                        pColor[0] =
                        pColor[1] =
                        pColor[2] = 0.0;
                        pColor[3] = textures[0].Alpha;

                        return drawBitmapModulated( pCanvas,
                                                    textures[0].Bitmap,
                                                    viewState,
                                                    aLocalState );
                    }
                    else
                    {
                        return drawBitmap( pCanvas,
                                           textures[0].Bitmap,
                                           viewState,
                                           aLocalState );
                    }
                }
                else
                {
                    
                    
                    

                    BitmapEx aBmpEx( tools::bitmapExFromXBitmap( textures[0].Bitmap ) );

                    
                    
                    ::basegfx::B2DHomMatrix aScaling;
                    ::basegfx::B2DHomMatrix aPureTotalTransform; 
                    aScaling.scale( 1.0/aBmpSize.Width,
                                    1.0/aBmpSize.Height );

                    aTotalTransform = aTextureTransform * aScaling;
                    aPureTotalTransform = aTextureTransform;

                    
                    ::basegfx::B2DHomMatrix aMatrix;
                    ::canvas::tools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

                    
                    
                    aTotalTransform *= aMatrix;
                    aPureTotalTransform *= aMatrix;

                    
                    
                    ::basegfx::B2DVector aScale;
                    ::basegfx::B2DPoint  aOutputPos;
                    double               nRotate;
                    double               nShearX;
                    aTotalTransform.decompose( aScale, aOutputPos, nRotate, nShearX );

                    GraphicAttr             aGrfAttr;
                    GraphicObjectSharedPtr  pGrfObj;

                    if( ::basegfx::fTools::equalZero( nShearX ) )
                    {
                        
                        
                        

                        
                        aGrfAttr.SetMirrorFlags(
                            ( aScale.getX() < 0.0 ? BMP_MIRROR_HORZ : 0 ) |
                            ( aScale.getY() < 0.0 ? BMP_MIRROR_VERT : 0 ) );
                        aGrfAttr.SetRotation( static_cast< sal_uInt16 >(::basegfx::fround( nRotate*10.0 )) );

                        pGrfObj.reset( new GraphicObject( aBmpEx ) );
                    }
                    else
                    {
                        
                        
                        aBmpEx = tools::transformBitmap( aBmpEx,
                                                         aTotalTransform,
                                                         uno::Sequence< double >(),
                                                         tools::MODULATE_NONE);

                        pGrfObj.reset( new GraphicObject( aBmpEx ) );

                        
                        
                        aScale.setX( 0.0 ); aScale.setY( 0.0 );
                    }


                    
                    

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    ::basegfx::B2DVector aNextTileX( 1.0, 0.0 );
                    ::basegfx::B2DVector aNextTileY( 0.0, 1.0 );
                    aNextTileX *= aPureTotalTransform;
                    aNextTileY *= aPureTotalTransform;

                    ::basegfx::B2DHomMatrix aInverseTextureTransform( aPureTotalTransform );

                    ENSURE_ARG_OR_THROW( aInverseTextureTransform.isInvertible(),
                                     "CanvasHelper::fillTexturedPolyPolygon(): singular texture matrix" );

                    aInverseTextureTransform.invert();

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    ::basegfx::B2DRectangle aTextureSpacePolygonRect;
                    ::canvas::tools::calcTransformedRectBounds( aTextureSpacePolygonRect,
                                                                ::vcl::unotools::b2DRectangleFromRectangle(
                                                                    aPolygonDeviceRect ),
                                                                aInverseTextureTransform );

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    const sal_Int32 nX1( ::canvas::tools::roundDown( aTextureSpacePolygonRect.getMinX() ) );
                    const sal_Int32 nY1( ::canvas::tools::roundDown( aTextureSpacePolygonRect.getMinY() ) );
                    const sal_Int32 nX2( ::canvas::tools::roundUp( aTextureSpacePolygonRect.getMaxX() ) );
                    const sal_Int32 nY2( ::canvas::tools::roundUp( aTextureSpacePolygonRect.getMaxY() ) );
                    const ::basegfx::B2DRectangle aSingleTextureRect(
                        nX1, nY1,
                        nX1 + 1.0,
                        nY1 + 1.0 );

                    
                    ::basegfx::B2DRectangle aSingleDeviceTextureRect;
                    ::canvas::tools::calcTransformedRectBounds( aSingleDeviceTextureRect,
                                                                aSingleTextureRect,
                                                                aPureTotalTransform );

                    const ::Point aPtRepeat( ::vcl::unotools::pointFromB2DPoint(
                                                 aSingleDeviceTextureRect.getMinimum() ) );
                    const ::Size  aSz( ::basegfx::fround( aScale.getX() * aBmpSize.Width ),
                                       ::basegfx::fround( aScale.getY() * aBmpSize.Height ) );
                    const ::Size  aIntegerNextTileX( ::vcl::unotools::sizeFromB2DSize(aNextTileX) );
                    const ::Size  aIntegerNextTileY( ::vcl::unotools::sizeFromB2DSize(aNextTileY) );

                    const ::Point aPt( textures[0].RepeatModeX == rendering::TexturingMode::NONE ?
                                       ::basegfx::fround( aOutputPos.getX() ) : aPtRepeat.X(),
                                       textures[0].RepeatModeY == rendering::TexturingMode::NONE ?
                                       ::basegfx::fround( aOutputPos.getY() ) : aPtRepeat.Y() );
                    const sal_Int32 nTilesX( textures[0].RepeatModeX == rendering::TexturingMode::NONE ?
                                             1 : nX2 - nX1 );
                    const sal_Int32 nTilesY( textures[0].RepeatModeX == rendering::TexturingMode::NONE ?
                                             1 : nY2 - nY1 );

                    OutputDevice& rOutDev( mpOutDev->getOutDev() );

                    if( bRectangularPolygon )
                    {
                        
                        

                        
                        
                        
                        
                        
                        

                        
                        if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                       1.0 ) )
                        {
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            aGrfAttr.SetTransparency(
                                static_cast< sal_uInt8 >(
                                    ::basegfx::fround( 255.0*( 1.0 - textures[0].Alpha ) ) ) );
                        }

                        rOutDev.IntersectClipRegion( aPolygonDeviceRect );
                        textureFill( rOutDev,
                                     *pGrfObj,
                                     aPt,
                                     aIntegerNextTileX,
                                     aIntegerNextTileY,
                                     nTilesX,
                                     nTilesY,
                                     aSz,
                                     aGrfAttr );

                        if( mp2ndOutDev )
                        {
                            OutputDevice& r2ndOutDev( mp2ndOutDev->getOutDev() );
                            r2ndOutDev.IntersectClipRegion( aPolygonDeviceRect );
                            textureFill( r2ndOutDev,
                                         *pGrfObj,
                                         aPt,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );
                        }
                    }
                    else
                    {
                        
                        
                        

                        if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                       1.0 ) )
                        {
                            
                            
                            
                            
                            VirtualDevice aVDev( rOutDev );
                            aVDev.SetOutputSizePixel( aPolygonDeviceRect.GetSize() );

                            
                            const ::Point aOutPos( aPt - aPolygonDeviceRect.TopLeft() );
                            aPolyPoly.Translate( ::Point( -aPolygonDeviceRect.Left(),
                                                          -aPolygonDeviceRect.Top() ) );

                            const Region aPolyClipRegion( aPolyPoly );

                            aVDev.SetClipRegion( aPolyClipRegion );
                            textureFill( aVDev,
                                         *pGrfObj,
                                         aOutPos,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );

                            
                            
                            const ::Point aEmptyPoint;
                            Bitmap aContentBmp(
                                aVDev.GetBitmap( aEmptyPoint,
                                                 aVDev.GetOutputSizePixel() ) );

                            sal_uInt8 nCol( static_cast< sal_uInt8 >(
                                           ::basegfx::fround( 255.0*( 1.0 - textures[0].Alpha ) ) ) );
                            AlphaMask aAlpha( aVDev.GetOutputSizePixel(),
                                              &nCol );

                            BitmapEx aOutputBmpEx( aContentBmp, aAlpha );
                            rOutDev.DrawBitmapEx( aPolygonDeviceRect.TopLeft(),
                                                  aOutputBmpEx );

                            if( mp2ndOutDev )
                                mp2ndOutDev->getOutDev().DrawBitmapEx( aPolygonDeviceRect.TopLeft(),
                                                                       aOutputBmpEx );
                        }
                        else
                        {
                            const Region aPolyClipRegion( aPolyPoly );

                            rOutDev.Push( PUSH_CLIPREGION );
                            rOutDev.SetClipRegion( aPolyClipRegion );

                            textureFill( rOutDev,
                                         *pGrfObj,
                                         aPt,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );
                            rOutDev.Pop();

                            if( mp2ndOutDev )
                            {
                                OutputDevice& r2ndOutDev( mp2ndOutDev->getOutDev() );
                                r2ndOutDev.Push( PUSH_CLIPREGION );

                                r2ndOutDev.SetClipRegion( aPolyClipRegion );
                                textureFill( r2ndOutDev,
                                             *pGrfObj,
                                             aPt,
                                             aIntegerNextTileX,
                                             aIntegerNextTileY,
                                             nTilesX,
                                             nTilesY,
                                             aSz,
                                             aGrfAttr );
                                r2ndOutDev.Pop();
                            }
                        }
                    }
                }
            }
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
