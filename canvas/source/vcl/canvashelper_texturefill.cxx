/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvashelper_texturefill.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:20:00 $
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

#include <canvas/debug.hxx>

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _COM_SUN_STAR_RENDERING_TEXTDIRECTION_HPP__
#include <com/sun/star/rendering/TextDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_TEXTURINGMODE_HPP_
#include <com/sun/star/rendering/TexturingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_PATHCAPTYPE_HPP_
#include <com/sun/star/rendering/PathCapType.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_PATHJOINTYPE_HPP_
#include <com/sun/star/rendering/PathJoinType.hpp>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DLINEGEOMETRY_HXX
#include <basegfx/polygon/b2dlinegeometry.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#include <utility>

#include <comphelper/sequence.hxx>
#include <canvas/canvastools.hxx>

#include "textlayout.hxx"
#include "parametricpolypolygon.hxx"
#include "canvashelper.hxx"
#include "canvasbitmap.hxx"
#include "impltools.hxx"
#include "canvasfont.hxx"

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
            BOOL    bRet( false );
            Point   aCurrPos;
            int     nX, nY;

            for( nY=0; nY < nTilesY; ++nY )
            {
                aCurrPos.X() = rPosPixel.X() + nY*rNextTileY.Width();
                aCurrPos.Y() = rPosPixel.Y() + nY*rNextTileY.Height();

                for( nX=0; nX < nTilesX; ++nX )
                {
                    // update return value. This method should return true, if
                    // at least one of the looped Draws succeeded.
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

        inline sal_Int32 roundDown( const double& rVal )
        {
            return static_cast< sal_Int32 >( floor( rVal ) );
        }

        inline sal_Int32 roundUp( const double& rVal )
        {
            return static_cast< sal_Int32 >( ceil( rVal ) );
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas&                          rCanvas,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
        CHECK_AND_THROW( xPolyPolygon.is(),
                         "CanvasHelper::fillPolyPolygon(): polygon is NULL");
        CHECK_AND_THROW( textures.getLength(),
                         "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

        if( mpOutDev.get() )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );

            const int nTransparency( setupOutDevState( viewState, renderState, IGNORE_COLOR ) );
            PolyPolygon aPolyPoly( tools::mapPolyPolygon( tools::polyPolygonFromXPolyPolygon2D(xPolyPolygon),
                                                          viewState, renderState ) );

            // TODO(F1): Multi-texturing
            if( textures[0].Gradient.is() )
            {
                uno::Reference< lang::XServiceInfo > xRef( textures[0].Gradient,
                                                           uno::UNO_QUERY );

                if( xRef.is() &&
                    xRef->getImplementationName().equals(
                        ::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                PARAMETRICPOLYPOLYGON_IMPLEMENTATION_NAME))) )
                {
                    // TODO(Q1): Maybe use dynamic_cast here

                    // TODO(E1): Return value
                    // TODO(F1): FillRule
                    static_cast<ParametricPolyPolygon*>(textures[0].Gradient.get())->fill(
                        mpOutDev->getOutDev(),
                        mp2ndOutDev.get() ? &mp2ndOutDev->getOutDev() : (OutputDevice*)NULL,
                        aPolyPoly,
                        viewState,
                        renderState,
                        textures[0],
                        nTransparency );
                }
            }
            else if( textures[0].Bitmap.is() )
            {
                OSL_ENSURE( textures[0].RepeatModeX == rendering::TexturingMode::REPEAT &&
                            textures[0].RepeatModeY == rendering::TexturingMode::REPEAT,
                            "CanvasHelper::fillTexturedPolyPolygon(): VCL canvas cannot currently clamp textures." );

                const geometry::IntegerSize2D aBmpSize( textures[0].Bitmap->getSize() );

                CHECK_AND_THROW( aBmpSize.Width != 0 &&
                                 aBmpSize.Height != 0,
                                 "CanvasHelper::fillTexturedPolyPolygon(): zero-sized texture bitmap" );

                // determine maximal bound rect of texture-filled
                // polygon
                const ::Rectangle aPolygonDeviceRect(
                    aPolyPoly.GetBoundRect() );


                // first of all, determine whether we have a
                // drawBitmap() in disguise
                // =========================================

                const bool bRectangularPolygon( tools::isPolyPolygonEqualRectangle( aPolyPoly,
                                                                                    aPolygonDeviceRect ) );

                ::basegfx::B2DHomMatrix aTextureTransform;
                ::basegfx::unotools::homMatrixFromAffineMatrix( aTextureTransform,
                                                                textures[0].AffineTransform );

                const ::basegfx::B2DRectangle aRect(0.0, 0.0, 1.0, 1.0);
                ::basegfx::B2DRectangle aTextureDeviceRect;
                ::canvas::tools::calcTransformedRectBounds( aTextureDeviceRect,
                                                            aRect,
                                                            aTextureTransform );

                const ::Rectangle aIntegerTextureDeviceRect(
                    ::vcl::unotools::rectangleFromB2DRectangle( aTextureDeviceRect ) );

                if( bRectangularPolygon &&
                    aIntegerTextureDeviceRect == aPolygonDeviceRect )
                {
                    // need alpha modulation?
                    if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                   1.0 ) )
                    {
                        // setup alpha modulation values
                        rendering::RenderState aLocalState( renderState );
                        ::canvas::tools::setDeviceColor( aLocalState,
                                                         0.0, 0.0, 0.0, textures[0].Alpha );

                        return drawBitmapModulated( rCanvas,
                                                    textures[0].Bitmap,
                                                    viewState,
                                                    aLocalState );
                    }
                    else
                    {
                        return drawBitmap( rCanvas,
                                           textures[0].Bitmap,
                                           viewState,
                                           renderState );
                    }
                }
                else
                {
                    // No easy mapping to drawBitmap() - calculate
                    // texturing parameters
                    // ===========================================

                    BitmapEx aBmpEx( tools::bitmapExFromXBitmap( textures[0].Bitmap ) );

                    // scale down bitmap to [0,1]x[0,1] rect, as required
                    // from the XCanvas interface.
                    ::basegfx::B2DHomMatrix aScaling;
                    ::basegfx::B2DHomMatrix aTotalTransform; // with extra bitmap down-scaling
                    ::basegfx::B2DHomMatrix aPureTotalTransform; // pure view*render*texture transform
                    aScaling.scale( 1.0/aBmpSize.Width,
                                    1.0/aBmpSize.Height );

                    aTotalTransform = aTextureTransform * aScaling;
                    aPureTotalTransform = aTextureTransform;

                    // combine with view and render transform
                    ::basegfx::B2DHomMatrix aMatrix;
                    ::canvas::tools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

                    // combine all three transformations into one
                    // global texture-to-device-space transformation
                    aTotalTransform *= aMatrix;
                    aPureTotalTransform *= aMatrix;

                    // analyze transformation, and setup an
                    // appropriate GraphicObject
                    ::basegfx::B2DVector aScale;
                    ::basegfx::B2DPoint  aOutputPos;
                    double               nRotate;
                    double               nShearX;
                    aTotalTransform.decompose( aScale, aOutputPos, nRotate, nShearX );

                    GraphicAttr             aGrfAttr;
                    GraphicObjectSharedPtr  pGrfObj;

                    if( ::basegfx::fTools::equalZero( nShearX ) )
                    {
                        // no shear, GraphicObject is enough (the
                        // GraphicObject only supports scaling, rotation
                        // and translation)

                        // setup GraphicAttr
                        aGrfAttr.SetMirrorFlags(
                            ( aScale.getX() < 0.0 ? BMP_MIRROR_HORZ : 0 ) |
                            ( aScale.getY() < 0.0 ? BMP_MIRROR_VERT : 0 ) );
                        aGrfAttr.SetRotation( static_cast< USHORT >(::basegfx::fround( nRotate*10.0 )) );

                        pGrfObj.reset( new GraphicObject( aBmpEx ) );
                    }
                    else
                    {
                        // complex transformation, use generic affine bitmap
                        // transformation
                        aBmpEx = tools::transformBitmap( aBmpEx,
                                                         aTotalTransform,
                                                         uno::Sequence< double >(),
                                                         tools::MODULATE_NONE);

                        pGrfObj.reset( new GraphicObject( aBmpEx ) );

                        // clear scale values, generated bitmap already
                        // contains scaling
                        aScale.setX( 0.0 ); aScale.setY( 0.0 );
                    }


                    // render texture tiled into polygon
                    // =================================

                    // calc device space direction vectors. We employ
                    // the followin approach for tiled output: the
                    // texture bitmap is output in texture space
                    // x-major order, i.e. tile neighbors in texture
                    // space x direction are rendered back-to-back in
                    // device coordinate space (after the full device
                    // transformation). Thus, the aNextTile* vectors
                    // denote the output position updates in device
                    // space, to get from one tile to the next.
                    ::basegfx::B2DVector aNextTileX( 1.0, 0.0 );
                    ::basegfx::B2DVector aNextTileY( 0.0, 1.0 );
                    aNextTileX *= aPureTotalTransform;
                    aNextTileY *= aPureTotalTransform;

                    ::basegfx::B2DHomMatrix aInverseTextureTransform( aPureTotalTransform );

                    CHECK_AND_THROW( aInverseTextureTransform.isInvertible(),
                                     "CanvasHelper::fillTexturedPolyPolygon(): singular texture matrix" );

                    aInverseTextureTransform.invert();

                    // calc bound rect of extended texture area in
                    // device coordinates. Therefore, we first calc
                    // the area of the polygon bound rect in texture
                    // space. To maintain texture phase, this bound
                    // rect is then extended to integer coordinates
                    // (extended, because shrinking might leave some
                    // inner polygon areas unfilled).
                    // Finally, the bound rect is transformed back to
                    // device coordinate space, were we determine the
                    // start point from it.
                    ::basegfx::B2DRectangle aTextureSpacePolygonRect;
                    ::canvas::tools::calcTransformedRectBounds( aTextureSpacePolygonRect,
                                                                ::vcl::unotools::b2DRectangleFromRectangle(
                                                                    aPolygonDeviceRect ),
                                                                aInverseTextureTransform );

                    // calc left, top of extended polygon rect in
                    // texture space, create one-texture instance rect
                    // from it (i.e. rect from start point extending
                    // 1.0 units to the right and 1.0 units to the
                    // bottom). Note that the rounding employed here
                    // is a bit subtle, since we need to round up/down
                    // as _soon_ as any fractional amount is
                    // encountered. This is to ensure that the full
                    // polygon area is filled with texture tiles.
                    const sal_Int32 nX1( roundDown( aTextureSpacePolygonRect.getMinX() ) );
                    const sal_Int32 nY1( roundDown( aTextureSpacePolygonRect.getMinY() ) );
                    const sal_Int32 nX2( roundUp( aTextureSpacePolygonRect.getMaxX() ) );
                    const sal_Int32 nY2( roundUp( aTextureSpacePolygonRect.getMaxY() ) );
                    const ::basegfx::B2DRectangle aSingleTextureRect(
                        nX1, nY1,
                        nX1 + 1.0,
                        nY1 + 1.0 );

                    // and convert back to device space
                    ::basegfx::B2DRectangle aSingleDeviceTextureRect;
                    ::canvas::tools::calcTransformedRectBounds( aSingleDeviceTextureRect,
                                                                aSingleTextureRect,
                                                                aPureTotalTransform );

                    const ::Point aPt( ::vcl::unotools::pointFromB2DPoint(
                                           aSingleDeviceTextureRect.getMinimum() ) );
                    const ::Size  aSz( ::basegfx::fround( aScale.getX() * aBmpSize.Width ),
                                       ::basegfx::fround( aScale.getY() * aBmpSize.Height ) );
                    const ::Size  aIntegerNextTileX( ::vcl::unotools::sizeFromB2DSize(aNextTileX) );
                    const ::Size  aIntegerNextTileY( ::vcl::unotools::sizeFromB2DSize(aNextTileY) );

                    const sal_Int32 nTilesX( nX2 - nX1 );
                    const sal_Int32 nTilesY( nY2 - nY1 );

                    OutputDevice& rOutDev( mpOutDev->getOutDev() );

                    if( bRectangularPolygon )
                    {
                        // use optimized output path
                        // -------------------------

                        // this distinction really looks like a
                        // micro-optimisation, but in fact greatly speeds up
                        // especially complex fills. That's because when using
                        // clipping, we can output polygons instead of
                        // poly-polygons, and don't have to output the gradient
                        // twice for XOR

                        // setup alpha modulation
                        if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                       1.0 ) )
                        {
                            // TODO(F1): Note that the GraphicManager has
                            // a subtle difference in how it calculates
                            // the resulting alpha value: it's using the
                            // inverse alpha values (i.e. 'transparency'),
                            // and calculates transOrig + transModulate,
                            // instead of transOrig + transModulate -
                            // transOrig*transModulate (which would be
                            // equivalent to the origAlpha*modulateAlpha
                            // the DX canvas performs)
                            aGrfAttr.SetTransparency(
                                static_cast< BYTE >(
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
                        // output texture the hard way: XORing out the
                        // polygon
                        // ===========================================

                        if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                       1.0 ) )
                        {
                            // uh-oh. alpha blending is required,
                            // cannot do direct XOR, but have to
                            // prepare the filled polygon within a
                            // VDev
                            VirtualDevice aVDev( rOutDev );
                            aVDev.SetOutputSizePixel( aPolygonDeviceRect.GetSize() );

                            // shift output to origin of VDev
                            const ::Point aOutPos( aPt - aPolygonDeviceRect.TopLeft() );
                            aPolyPoly.Translate( ::Point( -aPolygonDeviceRect.Left(),
                                                          -aPolygonDeviceRect.Top() ) );

                            aVDev.SetRasterOp( ROP_XOR );
                            textureFill( aVDev,
                                         *pGrfObj,
                                         aOutPos,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );
                            aVDev.SetFillColor( COL_BLACK );
                            aVDev.SetRasterOp( ROP_0 );
                            aVDev.DrawPolyPolygon( aPolyPoly );
                            aVDev.SetRasterOp( ROP_XOR );
                            textureFill( aVDev,
                                         *pGrfObj,
                                         aOutPos,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );

                            // output VDev content alpha-blended to
                            // target position.
                            const ::Point aEmptyPoint;
                            Bitmap aContentBmp(
                                aVDev.GetBitmap( aEmptyPoint,
                                                 aVDev.GetOutputSizePixel() ) );

                            BYTE nCol( static_cast< BYTE >(
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
                            // output via repeated XORing
                            rOutDev.Push( PUSH_RASTEROP );
                            rOutDev.SetRasterOp( ROP_XOR );
                            textureFill( rOutDev,
                                         *pGrfObj,
                                         aPt,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );
                            rOutDev.SetFillColor( COL_BLACK );
                            rOutDev.SetRasterOp( ROP_0 );
                            rOutDev.DrawPolyPolygon( aPolyPoly );
                            rOutDev.SetRasterOp( ROP_XOR );
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
                                r2ndOutDev.Push( PUSH_RASTEROP );
                                r2ndOutDev.SetRasterOp( ROP_XOR );
                                textureFill( r2ndOutDev,
                                             *pGrfObj,
                                             aPt,
                                             aIntegerNextTileX,
                                             aIntegerNextTileY,
                                             nTilesX,
                                             nTilesY,
                                             aSz,
                                             aGrfAttr );
                                r2ndOutDev.SetFillColor( COL_BLACK );
                                r2ndOutDev.SetRasterOp( ROP_0 );
                                r2ndOutDev.DrawPolyPolygon( aPolyPoly );
                                r2ndOutDev.SetRasterOp( ROP_XOR );
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

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

}
