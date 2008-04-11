/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_canvashelper_texturefill.cxx,v $
 * $Revision: 1.3 $
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

#include <canvas/debug.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/rendering/TexturingMode.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/tools/tools.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <canvas/parametricpolypolygon.hxx>

#include "dx_spritecanvas.hxx"
#include "dx_canvashelper.hxx"
#include "dx_impltools.hxx"
#include "dx_surfacegraphics.hxx"

#include <boost/scoped_ptr.hpp>


using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        typedef ::boost::shared_ptr< Gdiplus::PathGradientBrush >   PathGradientBrushSharedPtr;

        bool fillLinearGradient( SurfaceGraphicsSharedPtr&                      rGraphics,
                                 const Gdiplus::Color&                          rColor1,
                                 const Gdiplus::Color&                          rColor2,
                                 const GraphicsPathSharedPtr&                   rFillPath,
                                 const rendering::Texture&                      texture )
        {
            // setup a linear gradient with two colors
            // ---------------------------------------

            Gdiplus::LinearGradientBrush aBrush(
                Gdiplus::PointF(0.0f,
                                0.5f),
                Gdiplus::PointF(1.0f,
                                0.5f),
                rColor1,
                rColor2 );

            // render background color, as LinearGradientBrush does not
            // properly support the WrapModeClamp repeat mode
            Gdiplus::SolidBrush aBackgroundBrush( rColor1 );
            (*rGraphics)->FillPath( &aBackgroundBrush, rFillPath.get() );

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
            if( Gdiplus::Ok != (*rGraphics)->SetClip( rFillPath.get(),
                                                      Gdiplus::CombineModeIntersect ) )
            {
                return false;
            }

            Gdiplus::SolidBrush aBackgroundBrush2( rColor2 );
            (*rGraphics)->FillPath( &aBackgroundBrush2, &aSolidFillPath );

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
            if( Gdiplus::Ok != (*rGraphics)->SetClip( &aClipPath,
                                                      Gdiplus::CombineModeIntersect ) )
            {
                return false;
            }

            // now, finally, output the gradient
            Gdiplus::Matrix aMatrix;
            tools::gdiPlusMatrixFromAffineMatrix2D( aMatrix,
                                                    texture.AffineTransform );
            aBrush.SetTransform( &aMatrix );

            (*rGraphics)->FillRectangle( &aBrush, aBounds );

            return true;
        }

        bool fillAxialGradient( SurfaceGraphicsSharedPtr&                      rGraphics,
                                const Gdiplus::Color&                          rColor1,
                                const Gdiplus::Color&                          rColor2,
                                const GraphicsPathSharedPtr&                   rFillPath,
                                const rendering::Texture&                      texture )
        {
            // setup a linear gradient with three colors
            // -----------------------------------------

            Gdiplus::LinearGradientBrush aBrush(
                Gdiplus::PointF(0.0f,
                                0.5f),
                Gdiplus::PointF(1.0f,
                                0.5f),
                rColor1,
                rColor1 );

            Gdiplus::Color aColors[] =
                {
                    rColor1,    // at 0.0
                    rColor2,    // at 0.5
                    rColor1     // at 1.0
                };

            Gdiplus::REAL aPositions[] =
                {
                    0.0,
                    0.5,
                    1.0
                };

            if( Gdiplus::Ok != aBrush.SetInterpolationColors( aColors,
                                                              aPositions,
                                                              sizeof( aPositions ) / sizeof(Gdiplus::REAL) ) )
            {
                return false;
            }

            // render background color, as LinearGradientBrush does not
            // properly support the WrapModeClamp repeat mode
            Gdiplus::SolidBrush aBackgroundBrush( rColor1 );
            (*rGraphics)->FillPath( &aBackgroundBrush, rFillPath.get() );

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
            if( Gdiplus::Ok != (*rGraphics)->SetClip( rFillPath.get(),
                                                      Gdiplus::CombineModeIntersect ) )
            {
                return false;
            }
            if( Gdiplus::Ok != (*rGraphics)->SetClip( &aClipPath,
                                                      Gdiplus::CombineModeIntersect ) )
            {
                return false;
            }

            // now, finally, output the gradient
            Gdiplus::Matrix aMatrix;
            tools::gdiPlusMatrixFromAffineMatrix2D( aMatrix,
                                                    texture.AffineTransform );
            aBrush.SetTransform( &aMatrix );

            (*rGraphics)->FillRectangle( &aBrush, aBounds );

            return true;
        }

        PathGradientBrushSharedPtr createPathGradientBrush( const GraphicsPathSharedPtr& rGradientPath,
                                                            const Gdiplus::Color&        rColor1,
                                                            const Gdiplus::Color&        rColor2        )
        {
            PathGradientBrushSharedPtr pGradientBrush(
                new Gdiplus::PathGradientBrush( rGradientPath.get() ) );

            Gdiplus::Color aColors[] =
                {
                    rColor1
                };

            INT nCount(1);

            pGradientBrush->SetSurroundColors( aColors,
                                               &nCount );
            pGradientBrush->SetCenterColor( rColor2 );

            return pGradientBrush;
        }

        bool fillPolygonalGradient( const ::canvas::ParametricPolyPolygon::Values& rValues,
                                    SurfaceGraphicsSharedPtr&                      rGraphics,
                                    const Gdiplus::Color&                          rColor1,
                                    const Gdiplus::Color&                          rColor2,
                                    const GraphicsPathSharedPtr&                   rPath,
                                    const rendering::Texture&                      texture )
        {
            Gdiplus::Matrix aMatrix;
            tools::gdiPlusMatrixFromAffineMatrix2D( aMatrix,
                                                    texture.AffineTransform );

            // copy original fill path object, might have to change it
            // below
            GraphicsPathSharedPtr pFillPath( rPath );

            // clone original gradient path object, we need to change it
            // below
            GraphicsPathSharedPtr pGradientPath(
                tools::graphicsPathFromB2DPolygon( rValues.maGradientPoly ) );

            ENSURE_AND_RETURN( pGradientPath.get(),
                               "ParametricPolyPolygon::fillPolygonalGradient(): Could not clone path" );

            PathGradientBrushSharedPtr pGradientBrush;

            // fill background uniformly with end color
            Gdiplus::SolidBrush aBackgroundBrush( rColor1 );
            (*rGraphics)->FillPath( &aBackgroundBrush, pFillPath.get() );

            // scale focus according to aspect ratio: for wider-than-tall
            // bounds (nAspectRatio > 1.0), the focus must have non-zero
            // width. Specifically, a bound rect twice as wide as tall has
            // a focus of half it's width.
            if( !::rtl::math::approxEqual(rValues.mnAspectRatio,
                                          1.0) )
            {
                // KLUDGE 1:
                //
                // And here comes the greatest shortcoming of the GDI+
                // gradients ever: SetFocusScales completely ignores
                // transformations, both when set at the PathGradientBrush
                // and for the world coordinate system. Thus, to correctly
                // display anisotrophic path gradients, we have to render
                // them by hand. WTF.

                // TODO(F2): This does not yet support other repeat modes
                // except clamp, and probably also no multi-texturing

                // limit output to to-be-filled polygon
                if( Gdiplus::Ok != (*rGraphics)->SetClip( pFillPath.get(),
                                                          Gdiplus::CombineModeIntersect ) )
                {
                    return false;
                }

                (*rGraphics)->MultiplyTransform( &aMatrix );

                // disable anti-aliasing, if any
                const Gdiplus::SmoothingMode eOldAAMode( (*rGraphics)->GetSmoothingMode() );
                (*rGraphics)->SetSmoothingMode( Gdiplus::SmoothingModeHighSpeed );


                // determine number of steps to use
                // --------------------------------

                // TODO(Q2): Unify step calculations with VCL canvas
                const int nColorSteps(
                    ::std::max(
                        labs( rColor1.GetRed() - rColor2.GetRed() ),
                        ::std::max(
                            labs( rColor1.GetGreen() - rColor2.GetGreen() ),
                            labs( rColor1.GetBlue() - rColor2.GetBlue() ) ) ) );

                Gdiplus::Matrix aWorldTransformMatrix;
                (*rGraphics)->GetTransform( &aWorldTransformMatrix );

                Gdiplus::RectF  aBounds;
                pGradientPath->GetBounds( &aBounds, &aWorldTransformMatrix, NULL );

                // longest line in gradient bound rect
                const int nGradientSize(
                    static_cast<int>( hypot( aBounds.Width, aBounds.Height ) + 1.0 ) );

                // typical number for pixel of the same color (strip size)
                const int nStripSize( 2 );

                // use at least three steps, and at utmost the number of
                // color steps.
                const int nStepCount(
                    ::std::max(
                        3,
                        ::std::min(
                            nGradientSize / nStripSize,
                            nColorSteps ) ) + 1 );


                Gdiplus::SolidBrush     aFillBrush( rColor1 );
                Gdiplus::Matrix         aGDIScaleMatrix;
                ::basegfx::B2DHomMatrix aScaleMatrix;

                // calc relative size for anisotrophic polygon scaling:
                // when the aspect ratio is e.g. 2.0, that denotes a
                // gradient which is twice as wide as high. Then, to
                // generate a symmetric gradient, the x direction is only
                // scaled to 0.5 times the gradient width. Similarly, when
                // the aspect ratio is 4.0, the focus has 3/4 the width of
                // the overall gradient.
                const double nRelativeFocusSize( rValues.mnAspectRatio > 1.0 ?
                                                 1.0 - 1.0/rValues.mnAspectRatio :
                                                 1.0 - rValues.mnAspectRatio );

                for( int i=1; i<nStepCount; ++i )
                {
                    // lerp color. Funnily, the straight-forward integer
                    // lerp ((nStepCount - i)*val + i*val)/nStepCount gets
                    // fully botched by MSVC, at least for anything that
                    // really inlines inlines (i.e. every compile without
                    // debug=t)
                    const double nFrac( (double)i/nStepCount );

                    const Gdiplus::Color aFillColor(
                        static_cast<BYTE>( (1.0 - nFrac)*rColor1.GetRed() + nFrac*rColor2.GetRed() ),
                        static_cast<BYTE>( (1.0 - nFrac)*rColor1.GetGreen() + nFrac*rColor2.GetGreen() ),
                        static_cast<BYTE>( (1.0 - nFrac)*rColor1.GetBlue() + nFrac*rColor2.GetBlue() ) );

                    aFillBrush.SetColor( aFillColor );

                    const double nCurrScale( (nStepCount-i)/(double)nStepCount );
                    aScaleMatrix.identity();
                    aScaleMatrix.translate( -0.5, -0.5 );

                    // handle anisotrophic polygon scaling
                    if( rValues.mnAspectRatio < 1.0 )
                    {
                        // height > width case
                        aScaleMatrix.scale( nCurrScale,
                                            // lerp with nCurrScale
                                            // between 1.0 and
                                            // relative focus height
                                            nCurrScale + (1.0-nCurrScale)*nRelativeFocusSize );
                    }
                    else if( rValues.mnAspectRatio > 1.0 )
                    {
                        // width > height case
                        aScaleMatrix.scale( nCurrScale + (1.0-nCurrScale)*nRelativeFocusSize,
                                            // lerp with nCurrScale
                                            // between 1.0 and
                                            // relative focus width
                                            nCurrScale );
                    }
                    else
                    {
                        aScaleMatrix.scale( nCurrScale,
                                            nCurrScale );
                    }

                    aScaleMatrix.translate( 0.5, 0.5 );

                    tools::gdiPlusMatrixFromB2DHomMatrix( aGDIScaleMatrix,
                                                          aScaleMatrix );

                    GraphicsPathSharedPtr pScaledGradientPath(
                        tools::graphicsPathFromB2DPolygon( rValues.maGradientPoly ) );
                    pScaledGradientPath->Transform( &aGDIScaleMatrix );

                    (*rGraphics)->FillPath( &aFillBrush, pScaledGradientPath.get() );
                }

                // reset to old anti-alias mode
                (*rGraphics)->SetSmoothingMode( eOldAAMode );
            }
            else
            {
                // KLUDGE 2:
                //
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

                pGradientPath->Transform( &aMatrix );

                pGradientBrush = createPathGradientBrush(
                    pGradientPath,
                    rColor1,
                    rColor2 );

                // explicitely setup center point. Since the center of GDI+
                // gradients are by default the _centroid_ of the path
                // (i.e. the weighted sum of edge points), it will not
                // necessarily coincide with our notion of center.
                Gdiplus::PointF aCenterPoint(0.5, 0.5);
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
                (*rGraphics)->FillPath( pGradientBrush.get(), pFillPath.get() );
            }

#if defined(VERBOSE) && defined(DBG_UTIL)
            (*rGraphics)->MultiplyTransform( &aMatrix );

            Gdiplus::Pen aPen( Gdiplus::Color( 255, 255, 0, 0 ),
                               0.0001f );

            (*rGraphics)->DrawRectangle( &aPen,
                                         Gdiplus::RectF( 0.0f, 0.0f,
                                                         1.0f, 1.0f ) );
#endif

            return true;
        }

        bool fillGradient( const ::canvas::ParametricPolyPolygon::Values& rValues,
                           const Gdiplus::Color&                          rColor1,
                           const Gdiplus::Color&                          rColor2,
                           SurfaceGraphicsSharedPtr&                      rGraphics,
                           const GraphicsPathSharedPtr&                   rPath,
                           const rendering::Texture&                      texture )
        {
            switch( rValues.meType )
            {
                case ::canvas::ParametricPolyPolygon::GRADIENT_LINEAR:
                    fillLinearGradient( rGraphics,
                                        rColor1,
                                        rColor2,
                                        rPath,
                                        texture  );
                    break;

                case ::canvas::ParametricPolyPolygon::GRADIENT_AXIAL:
                    fillAxialGradient( rGraphics,
                                       rColor1,
                                       rColor2,
                                       rPath,
                                       texture );
                    break;

                case ::canvas::ParametricPolyPolygon::GRADIENT_ELLIPTICAL:
                    // FALLTHROUGH intended
                case ::canvas::ParametricPolyPolygon::GRADIENT_RECTANGULAR:
                    fillPolygonalGradient( rValues,
                                           rGraphics,
                                           rColor1,
                                           rColor2,
                                           rPath,
                                           texture );
                    break;

                default:
                    ENSURE_AND_THROW( false,
                                      "CanvasHelper::fillGradient(): Unexpected case" );
            }

            return true;
        }

        void fillBitmap( const uno::Reference< rendering::XBitmap >& xBitmap,
                         SurfaceGraphicsSharedPtr&                   rGraphics,
                         const GraphicsPathSharedPtr&                rPath,
                         const rendering::Texture&                   rTexture )
        {
            OSL_ENSURE( rTexture.RepeatModeX ==
                        rTexture.RepeatModeY,
                        "CanvasHelper::fillBitmap(): GDI+ cannot handle differing X/Y repeat mode." );

            const bool bClamp( rTexture.RepeatModeX == rendering::TexturingMode::CLAMP &&
                               rTexture.RepeatModeY == rendering::TexturingMode::CLAMP );

            const geometry::IntegerSize2D aBmpSize( xBitmap->getSize() );
            CHECK_AND_THROW( aBmpSize.Width != 0 &&
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
            pBrush->ScaleTransform( static_cast< Gdiplus::REAL >(1.0/aBmpSize.Width),
                                    static_cast< Gdiplus::REAL >(1.0/aBmpSize.Height) );
            pBrush->MultiplyTransform( &aTextureTransform );

            // TODO(F1): FillRule
            ENSURE_AND_THROW(
                Gdiplus::Ok == (*rGraphics)->FillPath( pBrush.get(),
                                                       rPath.get() ),
                "CanvasHelper::fillTexturedPolyPolygon(): GDI+ call failed" );
        }
    }

    // -------------------------------------------------------------

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
        ENSURE_AND_THROW( xPolyPolygon.is(),
                          "CanvasHelper::fillTexturedPolyPolygon: polygon is NULL");
        ENSURE_AND_THROW( textures.getLength(),
                          "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

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

                    const Gdiplus::Color aColor1(tools::sequenceToArgb(rValues.maColor1));
                    const Gdiplus::Color aColor2(tools::sequenceToArgb(rValues.maColor2));

                    // TODO(E1): Return value
                    // TODO(F1): FillRule
                    fillGradient( rValues,
                                  aColor1,
                                  aColor2,
                                  aGraphics,
                                  tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ),
                                  textures[0] );
                }
            }
            else if( textures[0].Bitmap.is() )
            {
                // TODO(E1): Return value
                // TODO(F1): FillRule
                fillBitmap( textures[0].Bitmap,
                            aGraphics,
                            tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ),
                            textures[0] );
            }
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }
}
