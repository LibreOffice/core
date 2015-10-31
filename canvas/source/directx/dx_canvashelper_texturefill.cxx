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

#include <sal/config.h>

#include <boost/tuple/tuple.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/keystoplerp.hxx>
#include <basegfx/tools/lerp.hxx>
#include <basegfx/tools/tools.hxx>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/parametricpolypolygon.hxx>

#include "dx_canvashelper.hxx"
#include "dx_impltools.hxx"
#include "dx_spritecanvas.hxx"


using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        typedef std::shared_ptr< Gdiplus::PathGradientBrush >   PathGradientBrushSharedPtr;

        bool fillLinearGradient( GraphicsSharedPtr&                             rGraphics,
                                 const ::canvas::ParametricPolyPolygon::Values& /*rValues*/,
                                 const std::vector< Gdiplus::Color >&           rColors,
                                 const std::vector< Gdiplus::REAL >&            rStops,
                                 const GraphicsPathSharedPtr&                   rFillPath,
                                 const rendering::Texture&                      texture )
        {
            // setup a linear gradient with given colors


            Gdiplus::LinearGradientBrush aBrush(
                Gdiplus::PointF(0.0f,
                                0.5f),
                Gdiplus::PointF(1.0f,
                                0.5f),
                rColors[0],
                rColors[1] );

            aBrush.SetInterpolationColors(&rColors[0],
                                          &rStops[0],
                                          rColors.size());

            // render background color, as LinearGradientBrush does not
            // properly support the WrapModeClamp repeat mode
            Gdiplus::SolidBrush aBackgroundBrush( rColors[0] );
            rGraphics->FillPath( &aBackgroundBrush, rFillPath.get() );

            // TODO(F2): This does not yet support other repeat modes
            // except clamp, and probably also no multi-texturing

            // calculate parallelogram of gradient in object space, extend
            // top and bottom of it such that they cover the whole fill
            // path bound area
            ::basegfx::B2DHomMatrix aTextureTransform;
            ::basegfx::unotools::homMatrixFromAffineMatrix( aTextureTransform,
                                                            texture.AffineTransform );

            ::basegfx::B2DPoint aLeftTop( 0.0, 0.0 );
            ::basegfx::B2DPoint aLeftBottom( 0.0, 1.0 );
            ::basegfx::B2DPoint aRightTop( 1.0, 0.0 );
            ::basegfx::B2DPoint aRightBottom( 1.0, 1.0 );

            aLeftTop    *= aTextureTransform;
            aLeftBottom *= aTextureTransform;
            aRightTop   *= aTextureTransform;
            aRightBottom*= aTextureTransform;

            Gdiplus::RectF aBounds;
            rFillPath->GetBounds( &aBounds, NULL, NULL );

            // now, we potentially have to enlarge our gradient area
            // atop and below the transformed [0,1]x[0,1] unit rect,
            // for the gradient to fill the complete bound rect.
            ::basegfx::tools::infiniteLineFromParallelogram( aLeftTop,
                                                             aLeftBottom,
                                                             aRightTop,
                                                             aRightBottom,
                                                             tools::b2dRangeFromGdiPlusRectF( aBounds ) );

            // calc length of bound rect diagonal
            const double nDiagonalLength(
                hypot( aBounds.Width,
                       aBounds.Height ) );

            // generate a path which covers the 'right' side of the
            // gradient, extending two times the bound rect diagonal to
            // the right (and thus covering the whole half plane 'right'
            // of the gradient). Take the middle of the gradient as the
            // 'left' side of the polygon, to not fall victim to rounding
            // errors at the edge.
            ::basegfx::B2DVector aDirection( aLeftTop - aLeftBottom );
            aDirection = ::basegfx::getNormalizedPerpendicular( aDirection );
            aDirection *= nDiagonalLength;

            const ::basegfx::B2DPoint aHalfPlaneLeftTop( (aLeftTop + aRightTop) * 0.5 );
            const ::basegfx::B2DPoint aHalfPlaneLeftBottom( (aLeftBottom + aRightBottom) * 0.5 );
            const ::basegfx::B2DPoint aHalfPlaneRightTop( aRightTop + aDirection );
            const ::basegfx::B2DPoint aHalfPlaneRightBottom( aRightBottom + aDirection );

            Gdiplus::GraphicsPath aSolidFillPath;
            aSolidFillPath.AddLine( static_cast<Gdiplus::REAL>(aHalfPlaneLeftTop.getX()),
                                    static_cast<Gdiplus::REAL>(aHalfPlaneLeftTop.getY()),
                                    static_cast<Gdiplus::REAL>(aHalfPlaneRightTop.getX()),
                                    static_cast<Gdiplus::REAL>(aHalfPlaneRightTop.getY()) );
            aSolidFillPath.AddLine( static_cast<Gdiplus::REAL>(aHalfPlaneRightBottom.getX()),
                                    static_cast<Gdiplus::REAL>(aHalfPlaneRightBottom.getY()),
                                    static_cast<Gdiplus::REAL>(aHalfPlaneLeftBottom.getX()),
                                    static_cast<Gdiplus::REAL>(aHalfPlaneLeftBottom.getY()) );
            aSolidFillPath.CloseFigure();

            // limit output to fill path, we've just generated a path that
            // might be substantially larger
            if( Gdiplus::Ok != rGraphics->SetClip( rFillPath.get(),
                                                   Gdiplus::CombineModeIntersect ) )
            {
                return false;
            }

            Gdiplus::SolidBrush aBackgroundBrush2( rColors.back() );
            rGraphics->FillPath( &aBackgroundBrush2, &aSolidFillPath );

            // generate clip polygon from the extended parallelogram
            // (exploit the feature that distinct lines in a figure are
            // automatically closed by a straight line)
            Gdiplus::GraphicsPath aClipPath;
            aClipPath.AddLine( static_cast<Gdiplus::REAL>(aLeftTop.getX()),
                               static_cast<Gdiplus::REAL>(aLeftTop.getY()),
                               static_cast<Gdiplus::REAL>(aRightTop.getX()),
                               static_cast<Gdiplus::REAL>(aRightTop.getY()) );
            aClipPath.AddLine( static_cast<Gdiplus::REAL>(aRightBottom.getX()),
                               static_cast<Gdiplus::REAL>(aRightBottom.getY()),
                               static_cast<Gdiplus::REAL>(aLeftBottom.getX()),
                               static_cast<Gdiplus::REAL>(aLeftBottom.getY()) );
            aClipPath.CloseFigure();

            // limit output to a _single_ strip of the gradient (have to
            // clip here, since GDI+ wrapmode clamp does not work here)
            if( Gdiplus::Ok != rGraphics->SetClip( &aClipPath,
                                                   Gdiplus::CombineModeIntersect ) )
            {
                return false;
            }

            // now, finally, output the gradient
            Gdiplus::Matrix aMatrix;
            tools::gdiPlusMatrixFromAffineMatrix2D( aMatrix,
                                                    texture.AffineTransform );
            aBrush.SetTransform( &aMatrix );

            rGraphics->FillRectangle( &aBrush, aBounds );

            return true;
        }

        int numColorSteps( const Gdiplus::Color& rColor1, const Gdiplus::Color& rColor2 )
        {
            return ::std::max(
                labs( rColor1.GetRed() - rColor2.GetRed() ),
                ::std::max(
                    labs( rColor1.GetGreen() - rColor2.GetGreen() ),
                    labs( rColor1.GetBlue()  - rColor2.GetBlue() ) ) );
        }

        bool fillPolygonalGradient( const ::canvas::ParametricPolyPolygon::Values& rValues,
                                    const std::vector< Gdiplus::Color >&           rColors,
                                    const std::vector< Gdiplus::REAL >&            rStops,
                                    GraphicsSharedPtr&                             rGraphics,
                                    const GraphicsPathSharedPtr&                   rPath,
                                    const rendering::ViewState&                    viewState,
                                    const rendering::RenderState&                  renderState,
                                    const rendering::Texture&                      texture )
        {
            // copy original fill path object, might have to change it
            // below
            GraphicsPathSharedPtr pFillPath( rPath );
            const ::basegfx::B2DPolygon& rGradientPoly( rValues.maGradientPoly );

            PathGradientBrushSharedPtr pGradientBrush;

            // fill background uniformly with end color
            Gdiplus::SolidBrush aBackgroundBrush( rColors[0] );
            rGraphics->FillPath( &aBackgroundBrush, pFillPath.get() );

            Gdiplus::Matrix aMatrix;
            // scale focus according to aspect ratio: for wider-than-tall
            // bounds (nAspectRatio > 1.0), the focus must have non-zero
            // width. Specifically, a bound rect twice as wide as tall has
            // a focus of half its width.
            if( !::rtl::math::approxEqual(rValues.mnAspectRatio,
                                          1.0) )
            {
                // KLUDGE 1:

                // And here comes the greatest shortcoming of the GDI+
                // gradients ever: SetFocusScales completely ignores
                // transformations, both when set at the PathGradientBrush
                // and for the world coordinate system. Thus, to correctly
                // display anisotrophic path gradients, we have to render
                // them by hand. WTF.

                // TODO(F2): This does not yet support other repeat modes
                // except clamp, and probably also no multi-texturing

                // limit output to to-be-filled polygon
                if( Gdiplus::Ok != rGraphics->SetClip( pFillPath.get(),
                                                       Gdiplus::CombineModeIntersect ) )
                {
                    return false;
                }

                // disable anti-aliasing, if any
                const Gdiplus::SmoothingMode eOldAAMode( rGraphics->GetSmoothingMode() );
                rGraphics->SetSmoothingMode( Gdiplus::SmoothingModeHighSpeed );


                // determine number of steps to use


                // TODO(Q2): Unify step calculations with VCL canvas
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

                ::basegfx::B2DHomMatrix aTextureTransform;
                ::basegfx::unotools::homMatrixFromAffineMatrix( aTextureTransform,
                                                                texture.AffineTransform );
                // determine overall transformation for inner polygon (might
                // have to be prefixed by anisotrophic scaling)
                ::basegfx::B2DHomMatrix aInnerPolygonTransformMatrix;

                // For performance reasons, we create a temporary VCL polygon
                // here, keep it all the way and only change the vertex values
                // in the loop below (as ::Polygon is a pimpl class, creating
                // one every loop turn would really stress the mem allocator)
                ::basegfx::B2DPolygon   aOuterPoly( rGradientPoly );
                ::basegfx::B2DPolygon   aInnerPoly;

                // subdivide polygon _before_ rendering, would otherwise have
                // to be performed on every loop turn.
                if( aOuterPoly.areControlPointsUsed() )
                    aOuterPoly = ::basegfx::tools::adaptiveSubdivideByAngle(aOuterPoly);

                aInnerPoly = aOuterPoly;
                aOuterPoly.transform(aTextureTransform);


                // apply scaling (possibly anisotrophic) to inner polygon


                // scale inner polygon according to aspect ratio: for
                // wider-than-tall bounds (nAspectRatio > 1.0), the inner
                // polygon, representing the gradient focus, must have
                // non-zero width. Specifically, a bound rect twice as wide as
                // tall has a focus polygon of half its width.
                const double nAspectRatio( rValues.mnAspectRatio );
                if( nAspectRatio > 1.0 )
                {
                    // width > height case
                    aInnerPolygonTransformMatrix.scale( 1.0 - 1.0/nAspectRatio,
                                                        0.0 );
                }
                else if( nAspectRatio < 1.0 )
                {
                    // width < height case
                    aInnerPolygonTransformMatrix.scale( 0.0,
                                                        1.0 - nAspectRatio );
                }
                else
                {
                    // isotrophic case
                    aInnerPolygonTransformMatrix.scale( 0.0, 0.0 );
                }

                // and finally, add texture transform to it.
                aInnerPolygonTransformMatrix *= aTextureTransform;

                // apply final matrix to polygon
                aInnerPoly.transform( aInnerPolygonTransformMatrix );

                Gdiplus::GraphicsPath aCurrPath;
                Gdiplus::SolidBrush   aFillBrush( rColors[0] );
                const sal_uInt32      nNumPoints( aOuterPoly.count() );
                basegfx::tools::KeyStopLerp aLerper(rValues.maStops);
                for( int i=1; i<nStepCount; ++i )
                {
                    std::ptrdiff_t nIndex;
                    double fAlpha;
                    const double fT( i/double(nStepCount) );
                    boost::tuples::tie(nIndex,fAlpha)=aLerper.lerp(fT);

                    const Gdiplus::Color aFillColor(
                        static_cast<BYTE>( basegfx::tools::lerp(rColors[nIndex].GetRed(),rColors[nIndex+1].GetRed(),fAlpha) ),
                        static_cast<BYTE>( basegfx::tools::lerp(rColors[nIndex].GetGreen(),rColors[nIndex+1].GetGreen(),fAlpha) ),
                        static_cast<BYTE>( basegfx::tools::lerp(rColors[nIndex].GetBlue(),rColors[nIndex+1].GetBlue(),fAlpha) ) );

                    aFillBrush.SetColor( aFillColor );
                    aCurrPath.Reset(); aCurrPath.StartFigure();
                    for( unsigned int p=1; p<nNumPoints; ++p )
                    {
                        const ::basegfx::B2DPoint& rOuterPoint1( aOuterPoly.getB2DPoint(p-1) );
                        const ::basegfx::B2DPoint& rInnerPoint1( aInnerPoly.getB2DPoint(p-1) );
                        const ::basegfx::B2DPoint& rOuterPoint2( aOuterPoly.getB2DPoint(p) );
                        const ::basegfx::B2DPoint& rInnerPoint2( aInnerPoly.getB2DPoint(p) );

                        aCurrPath.AddLine(
                            Gdiplus::REAL(fT*rInnerPoint1.getX() + (1-fT)*rOuterPoint1.getX()),
                            Gdiplus::REAL(fT*rInnerPoint1.getY() + (1-fT)*rOuterPoint1.getY()),
                            Gdiplus::REAL(fT*rInnerPoint2.getX() + (1-fT)*rOuterPoint2.getX()),
                            Gdiplus::REAL(fT*rInnerPoint2.getY() + (1-fT)*rOuterPoint2.getY()));
                    }
                    aCurrPath.CloseFigure();

                    rGraphics->FillPath( &aFillBrush, &aCurrPath );
                }

                // reset to old anti-alias mode
                rGraphics->SetSmoothingMode( eOldAAMode );
            }
            else
            {
                // KLUDGE 2:

                // We're generating a PathGradientBrush from scratch here,
                // and put in a transformed GraphicsPath (transformed with
                // the texture transform). This is because the
                // straight-forward approach to store a Brush pointer at
                // this class and set a texture transform via
                // PathGradientBrush::SetTransform() is spoiled by MS: it
                // seems that _either_ the texture transform, _or_ the
                // transform at the Graphics can be set, but not both. If
                // one sets both, only the translational components of the
                // texture is respected.

                tools::gdiPlusMatrixFromAffineMatrix2D( aMatrix,
                                                        texture.AffineTransform );
                GraphicsPathSharedPtr pGradientPath(
                    tools::graphicsPathFromB2DPolygon( rValues.maGradientPoly ));
                pGradientPath->Transform( &aMatrix );

                pGradientBrush.reset(
                    new Gdiplus::PathGradientBrush( pGradientPath.get() ) );
                pGradientBrush->SetInterpolationColors( &rColors[0],
                                                        &rStops[0],
                                                        rStops.size() );

                // explicitly setup center point. Since the center of GDI+
                // gradients are by default the _centroid_ of the path
                // (i.e. the weighted sum of edge points), it will not
                // necessarily coincide with our notion of center.
                Gdiplus::PointF aCenterPoint(0, 0);
                aMatrix.TransformPoints( &aCenterPoint );
                pGradientBrush->SetCenterPoint( aCenterPoint );

                const bool bTileX( texture.RepeatModeX != rendering::TexturingMode::CLAMP );
                const bool bTileY( texture.RepeatModeY != rendering::TexturingMode::CLAMP );

                if( bTileX && bTileY )
                    pGradientBrush->SetWrapMode( Gdiplus::WrapModeTile );
                else
                {
                    OSL_ENSURE( bTileY == bTileX,
                                "ParametricPolyPolygon::fillPolygonalGradient(): Cannot have repeat x and repeat y differ!" );

                    pGradientBrush->SetWrapMode( Gdiplus::WrapModeClamp );
                }

                // render actual gradient
                rGraphics->FillPath( pGradientBrush.get(), pFillPath.get() );
            }

#if OSL_DEBUG_LEVEL > 2
            Gdiplus::Pen aPen( Gdiplus::Color( 255, 255, 0, 0 ),
                               0.0001f );

            rGraphics->DrawRectangle( &aPen,
                                      Gdiplus::RectF( 0.0f, 0.0f,
                                                      1.0f, 1.0f ) );
#endif

            return true;
        }

        bool fillGradient( const ::canvas::ParametricPolyPolygon::Values& rValues,
                           const std::vector< Gdiplus::Color >&           rColors,
                           const std::vector< Gdiplus::REAL >&            rStops,
                           GraphicsSharedPtr&                             rGraphics,
                           const GraphicsPathSharedPtr&                   rPath,
                           const rendering::ViewState&                    viewState,
                           const rendering::RenderState&                  renderState,
                           const rendering::Texture&                      texture )
        {
            switch( rValues.meType )
            {
                case ::canvas::ParametricPolyPolygon::GRADIENT_LINEAR:
                    fillLinearGradient( rGraphics,
                                        rValues,
                                        rColors,
                                        rStops,
                                        rPath,
                                        texture  );
                    break;

                case ::canvas::ParametricPolyPolygon::GRADIENT_ELLIPTICAL:
                    // FALLTHROUGH intended
                case ::canvas::ParametricPolyPolygon::GRADIENT_RECTANGULAR:
                    fillPolygonalGradient( rValues,
                                           rColors,
                                           rStops,
                                           rGraphics,
                                           rPath,
                                           viewState,
                                           renderState,
                                           texture );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                      "CanvasHelper::fillGradient(): Unexpected case" );
            }

            return true;
        }

        void fillBitmap( const uno::Reference< rendering::XBitmap >& xBitmap,
                         GraphicsSharedPtr&                          rGraphics,
                         const GraphicsPathSharedPtr&                rPath,
                         const rendering::Texture&                   rTexture )
        {
            OSL_ENSURE( rTexture.RepeatModeX ==
                        rTexture.RepeatModeY,
                        "CanvasHelper::fillBitmap(): GDI+ cannot handle differing X/Y repeat mode." );

            const bool bClamp( rTexture.RepeatModeX == rendering::TexturingMode::NONE &&
                               rTexture.RepeatModeY == rendering::TexturingMode::NONE );

            const geometry::IntegerSize2D aBmpSize( xBitmap->getSize() );
            ENSURE_ARG_OR_THROW( aBmpSize.Width != 0 &&
                                 aBmpSize.Height != 0,
                                 "CanvasHelper::fillBitmap(): zero-sized texture bitmap" );

            // TODO(P3): Detect case that path is rectangle and
            // bitmap is just scaled into that. Then, we can
            // render directly, without generating a temporary
            // GDI+ bitmap (this is significant, because drawing
            // layer presents background object bitmap in that
            // way!)
            BitmapSharedPtr pBitmap(
                tools::bitmapFromXBitmap( xBitmap ) );

            TextureBrushSharedPtr pBrush;
            if( ::rtl::math::approxEqual( rTexture.Alpha,
                                          1.0 ) )
            {
                pBrush.reset(
                    new Gdiplus::TextureBrush(
                        pBitmap.get(),
                        bClamp ? Gdiplus::WrapModeClamp : Gdiplus::WrapModeTile ) );
            }
            else
            {
                Gdiplus::ImageAttributes aImgAttr;

                tools::setModulateImageAttributes( aImgAttr,
                                                   1.0,
                                                   1.0,
                                                   1.0,
                                                   rTexture.Alpha );

                Gdiplus::Rect aRect(0,0,
                                    aBmpSize.Width,
                                    aBmpSize.Height);
                pBrush.reset(
                    new Gdiplus::TextureBrush(
                        pBitmap.get(),
                        aRect,
                        &aImgAttr ) );

                pBrush->SetWrapMode(
                    bClamp ? Gdiplus::WrapModeClamp : Gdiplus::WrapModeTile );
            }

            Gdiplus::Matrix aTextureTransform;
            tools::gdiPlusMatrixFromAffineMatrix2D( aTextureTransform,
                                                    rTexture.AffineTransform );

            // scale down bitmap to [0,1]x[0,1] rect, as required
            // from the XCanvas interface.
            pBrush->MultiplyTransform( &aTextureTransform );
            pBrush->ScaleTransform( static_cast< Gdiplus::REAL >(1.0/aBmpSize.Width),
                                    static_cast< Gdiplus::REAL >(1.0/aBmpSize.Height) );

            // TODO(F1): FillRule
            ENSURE_OR_THROW(
                Gdiplus::Ok == rGraphics->FillPath( pBrush.get(),
                                                    rPath.get() ),
                "CanvasHelper::fillTexturedPolyPolygon(): GDI+ call failed" );
        }
    }



    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
        ENSURE_OR_THROW( xPolyPolygon.is(),
                          "CanvasHelper::fillTexturedPolyPolygon: polygon is NULL");
        ENSURE_OR_THROW( textures.getLength(),
                          "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            // TODO(F1): Multi-texturing
            if( textures[0].Gradient.is() )
            {
                // try to cast XParametricPolyPolygon2D reference to
                // our implementation class.
                ::canvas::ParametricPolyPolygon* pGradient =
                      dynamic_cast< ::canvas::ParametricPolyPolygon* >( textures[0].Gradient.get() );

                if( pGradient )
                {
                    const ::canvas::ParametricPolyPolygon::Values& rValues(
                        pGradient->getValues() );

                    OSL_ASSERT(rValues.maColors.getLength() == rValues.maStops.getLength()
                               && rValues.maColors.getLength() > 1);

                    std::vector< Gdiplus::Color > aColors(rValues.maColors.getLength());
                    std::transform(&rValues.maColors[0],
                                   &rValues.maColors[0]+rValues.maColors.getLength(),
                                   aColors.begin(),
                                   [](const uno::Sequence< double >& aDoubleSequence) { return tools::sequenceToArgb(aDoubleSequence); } );
                    std::vector< Gdiplus::REAL > aStops;
                    comphelper::sequenceToContainer(aStops,rValues.maStops);

                    // TODO(E1): Return value
                    // TODO(F1): FillRule
                    fillGradient( rValues,
                                  aColors,
                                  aStops,
                                  pGraphics,
                                  tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ),
                                  viewState,
                                  renderState,
                                  textures[0] );
                }
            }
            else if( textures[0].Bitmap.is() )
            {
                // TODO(E1): Return value
                // TODO(F1): FillRule
                fillBitmap( textures[0].Bitmap,
                            pGraphics,
                            tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ),
                            textures[0] );
            }
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
