/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvastools.cxx,v $
 * $Revision: 1.13 $
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
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>

#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

#include <canvas/canvastools.hxx>
#include <canvas/base/linepolypolygonbase.hxx>

#include <limits>


using namespace ::com::sun::star;

namespace canvas
{
    namespace tools
    {
        geometry::RealSize2D createInfiniteSize2D()
        {
            return geometry::RealSize2D(
                ::std::numeric_limits<double>::infinity(),
                ::std::numeric_limits<double>::infinity() );
        }

        rendering::RenderState& initRenderState( rendering::RenderState& renderState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( renderState.AffineTransform );
            renderState.Clip = uno::Reference< rendering::XPolyPolygon2D >();
            renderState.DeviceColor = uno::Sequence< double >();
            renderState.CompositeOperation = rendering::CompositeOperation::OVER;

            return renderState;
        }

        rendering::ViewState& initViewState( rendering::ViewState& viewState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( viewState.AffineTransform );
            viewState.Clip = uno::Reference< rendering::XPolyPolygon2D >();

            return viewState;
        }

        ::basegfx::B2DHomMatrix& getViewStateTransform( ::basegfx::B2DHomMatrix&    transform,
                                                        const rendering::ViewState& viewState )
        {
            return ::basegfx::unotools::homMatrixFromAffineMatrix( transform, viewState.AffineTransform );
        }

        rendering::ViewState& setViewStateTransform( rendering::ViewState&          viewState,
                                                     const ::basegfx::B2DHomMatrix& transform )
        {
            ::basegfx::unotools::affineMatrixFromHomMatrix( viewState.AffineTransform, transform );

            return viewState;
        }

        ::basegfx::B2DHomMatrix& getRenderStateTransform( ::basegfx::B2DHomMatrix&      transform,
                                                          const rendering::RenderState& renderState )
        {
            return ::basegfx::unotools::homMatrixFromAffineMatrix( transform, renderState.AffineTransform );
        }

        rendering::RenderState& setRenderStateTransform( rendering::RenderState&        renderState,
                                                         const ::basegfx::B2DHomMatrix& transform )
        {
            ::basegfx::unotools::affineMatrixFromHomMatrix( renderState.AffineTransform, transform );

            return renderState;
        }

        rendering::RenderState& appendToRenderState( rendering::RenderState&        renderState,
                                                   const ::basegfx::B2DHomMatrix&   rTransform )
        {
            ::basegfx::B2DHomMatrix transform;

            getRenderStateTransform( transform, renderState );
            return setRenderStateTransform( renderState, transform * rTransform );
        }

        rendering::ViewState& appendToViewState( rendering::ViewState&          viewState,
                                                 const ::basegfx::B2DHomMatrix& rTransform )
        {
            ::basegfx::B2DHomMatrix transform;

            getViewStateTransform( transform, viewState );
            return setViewStateTransform( viewState, transform * rTransform );
        }

        rendering::RenderState& prependToRenderState( rendering::RenderState&           renderState,
                                                      const ::basegfx::B2DHomMatrix&    rTransform )
        {
            ::basegfx::B2DHomMatrix transform;

            getRenderStateTransform( transform, renderState );
            return setRenderStateTransform( renderState, rTransform * transform );
        }

        rendering::ViewState& prependToViewState( rendering::ViewState&             viewState,
                                                  const ::basegfx::B2DHomMatrix&        rTransform )
        {
            ::basegfx::B2DHomMatrix transform;

            getViewStateTransform( transform, viewState );
            return setViewStateTransform( viewState, rTransform * transform );
        }

        ::basegfx::B2DHomMatrix& mergeViewAndRenderTransform( ::basegfx::B2DHomMatrix&      combinedTransform,
                                                              const rendering::ViewState&   viewState,
                                                              const rendering::RenderState& renderState )
        {
            ::basegfx::B2DHomMatrix viewTransform;

            ::basegfx::unotools::homMatrixFromAffineMatrix( combinedTransform, renderState.AffineTransform );
            ::basegfx::unotools::homMatrixFromAffineMatrix( viewTransform, viewState.AffineTransform );

            // this statement performs combinedTransform = viewTransform * combinedTransform
            combinedTransform *= viewTransform;

            return combinedTransform;
        }

        rendering::ViewState& mergeViewAndRenderState( rendering::ViewState&                        resultViewState,
                                                       const rendering::ViewState&                  viewState,
                                                       const rendering::RenderState&                renderState,
                                                       const uno::Reference< rendering::XCanvas >&  /*xCanvas*/     )
        {
            ::basegfx::B2DHomMatrix     aTmpMatrix;
            geometry::AffineMatrix2D    convertedMatrix;

            resultViewState.Clip = NULL; // TODO(F2): intersect clippings

            return setViewStateTransform(
                resultViewState,
                mergeViewAndRenderTransform( aTmpMatrix,
                                             viewState,
                                             renderState ) );
        }

        geometry::AffineMatrix2D& setIdentityAffineMatrix2D( geometry::AffineMatrix2D& matrix )
        {
            matrix.m00 = 1.0;
            matrix.m01 = 0.0;
            matrix.m02 = 0.0;
            matrix.m10 = 0.0;
            matrix.m11 = 1.0;
            matrix.m12 = 0.0;

            return matrix;
        }

        geometry::Matrix2D& setIdentityMatrix2D( geometry::Matrix2D& matrix )
        {
            matrix.m00 = 1.0;
            matrix.m01 = 0.0;
            matrix.m10 = 0.0;
            matrix.m11 = 1.0;

            return matrix;
        }

        void setDeviceColor( ::com::sun::star::rendering::RenderState&  o_renderState,
                             const double&                              rColor0,
                             const double&                              rColor1,
                             const double&                              rColor2,
                             const double&                              rColor3 )
        {
            o_renderState.DeviceColor.realloc( 4 );
            double* pColors = o_renderState.DeviceColor.getArray();

            pColors[0] = rColor0;
            pColors[1] = rColor1;
            pColors[2] = rColor2;
            pColors[3] = rColor3;
        }

        void getDeviceColor( double&                                            o_rColor0,
                             double&                                            o_rColor1,
                             double&                                            o_rColor2,
                             double&                                            o_rColor3,
                             const ::com::sun::star::rendering::RenderState&    rRenderState )
        {
            o_rColor0 = rRenderState.DeviceColor[0];
            o_rColor1 = rRenderState.DeviceColor[1];
            o_rColor2 = rRenderState.DeviceColor[2];
            o_rColor3 = rRenderState.DeviceColor.getLength() > 3 ?
                rRenderState.DeviceColor[3] : 1.0;
        }

        bool operator==( const rendering::RenderState& renderState1,
                         const rendering::RenderState& renderState2 )
        {
            if( renderState1.Clip != renderState2.Clip )
                return false;

            if( renderState1.DeviceColor != renderState2.DeviceColor )
                return false;

            if( renderState1.CompositeOperation != renderState2.CompositeOperation )
                return false;

            ::basegfx::B2DHomMatrix mat1, mat2;
            getRenderStateTransform( mat1, renderState1 );
            getRenderStateTransform( mat2, renderState2 );
            if( mat1 != mat2 )
                return false;

            return true;
        }

        bool operator==( const rendering::ViewState& viewState1,
                         const rendering::ViewState& viewState2 )
        {
            if( viewState1.Clip != viewState2.Clip )
                return false;

            ::basegfx::B2DHomMatrix mat1, mat2;
            getViewStateTransform( mat1, viewState1 );
            getViewStateTransform( mat2, viewState2 );
            if( mat1 != mat2 )
                return false;

            return true;
        }

        // Create a corrected view transformation out of the give one,
        // which ensures that the rectangle given by (0,0) and
        // rSpriteSize is mapped with its left,top corner to (0,0)
        // again. This is required to properly render sprite
        // animations to buffer bitmaps.
        ::basegfx::B2DHomMatrix& calcRectToOriginTransform( ::basegfx::B2DHomMatrix&            o_transform,
                                                            const ::basegfx::B2DRange&          i_srcRect,
                                                            const ::basegfx::B2DHomMatrix&      i_transformation )
        {
            if( i_srcRect.isEmpty() )
                return o_transform=i_transformation;

            // transform by given transformation
            ::basegfx::B2DRectangle aTransformedRect;

            calcTransformedRectBounds( aTransformedRect,
                                       i_srcRect,
                                       i_transformation );

            // now move resulting left,top point of bounds to (0,0)
            ::basegfx::B2DHomMatrix aCorrectedTransform;
            aCorrectedTransform.translate( -aTransformedRect.getMinX(),
                                           -aTransformedRect.getMinY() );

            // prepend to original transformation
            o_transform = aCorrectedTransform * i_transformation;

            return o_transform;
        }

        ::basegfx::B2DRange& calcTransformedRectBounds( ::basegfx::B2DRange&            outRect,
                                                        const ::basegfx::B2DRange&      inRect,
                                                        const ::basegfx::B2DHomMatrix&  transformation )
        {
            outRect.reset();

            if( inRect.isEmpty() )
                return outRect;

            // transform all four extremal points of the rectangle,
            // take bounding rect of those.

            // transform left-top point
            outRect.expand( transformation * inRect.getMinimum() );

            // transform bottom-right point
            outRect.expand( transformation * inRect.getMaximum() );

            ::basegfx::B2DPoint aPoint;

            // transform top-right point
            aPoint.setX( inRect.getMaxX() );
            aPoint.setY( inRect.getMinY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // transform bottom-left point
            aPoint.setX( inRect.getMinX() );
            aPoint.setY( inRect.getMaxY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // over and out.
            return outRect;
        }

        ::basegfx::B2DHomMatrix& calcRectToRectTransform( ::basegfx::B2DHomMatrix&          o_transform,
                                                          const ::basegfx::B2DRange&        destRect,
                                                          const ::basegfx::B2DRange&        srcRect,
                                                          const ::basegfx::B2DHomMatrix&    transformation )
        {
            if( srcRect.isEmpty() ||
                destRect.isEmpty() )
            {
                return o_transform=transformation;
            }

            // transform inputRect by transformation
            ::basegfx::B2DRectangle aTransformedRect;
            calcTransformedRectBounds( aTransformedRect,
                                       srcRect,
                                       transformation );

            // now move resulting left,top point of bounds to (0,0)
            ::basegfx::B2DHomMatrix aCorrectedTransform;
            aCorrectedTransform.translate( -aTransformedRect.getMinX(),
                                           -aTransformedRect.getMinY() );

            // scale to match outRect
            const double xDenom( aTransformedRect.getWidth() );
            const double yDenom( aTransformedRect.getHeight() );
            if( xDenom != 0.0 && yDenom != 0.0 )
                aCorrectedTransform.scale( destRect.getWidth() / xDenom,
                                           destRect.getHeight() / yDenom );
            // TODO(E2): error handling

            // translate to final position
            aCorrectedTransform.translate( destRect.getMinX(),
                                           destRect.getMinY() );

            ::basegfx::B2DHomMatrix transform( transformation );
            o_transform = aCorrectedTransform * transform;

            return o_transform;
        }

        bool isInside( const ::basegfx::B2DRange&       rContainedRect,
                       const ::basegfx::B2DRange&       rTransformRect,
                       const ::basegfx::B2DHomMatrix&   rTransformation )
        {
            if( rContainedRect.isEmpty() || rTransformRect.isEmpty() )
                return false;

            ::basegfx::B2DPolygon aPoly(
                ::basegfx::tools::createPolygonFromRect( rTransformRect ) );
            aPoly.transform( rTransformation );

            return ::basegfx::tools::isInside( aPoly,
                                               ::basegfx::tools::createPolygonFromRect(
                                                   rContainedRect ),
                                               true );
        }

        namespace
        {
            bool clipAreaImpl( ::basegfx::B2IRange*       o_pDestArea,
                               ::basegfx::B2IRange&       io_rSourceArea,
                               ::basegfx::B2IPoint&       io_rDestPoint,
                               const ::basegfx::B2IRange& rSourceBounds,
                               const ::basegfx::B2IRange& rDestBounds )
            {
                const ::basegfx::B2IPoint aSourceTopLeft(
                    io_rSourceArea.getMinimum() );

                ::basegfx::B2IRange aLocalSourceArea( io_rSourceArea );

                // clip source area (which must be inside rSourceBounds)
                aLocalSourceArea.intersect( rSourceBounds );

                if( aLocalSourceArea.isEmpty() )
                    return false;

                // calc relative new source area points (relative to orig
                // source area)
                const ::basegfx::B2IVector aUpperLeftOffset(
                    aLocalSourceArea.getMinimum()-aSourceTopLeft );
                const ::basegfx::B2IVector aLowerRightOffset(
                    aLocalSourceArea.getMaximum()-aSourceTopLeft );

                ::basegfx::B2IRange aLocalDestArea( io_rDestPoint + aUpperLeftOffset,
                                                    io_rDestPoint + aLowerRightOffset );

                // clip dest area (which must be inside rDestBounds)
                aLocalDestArea.intersect( rDestBounds );

                if( aLocalDestArea.isEmpty() )
                    return false;

                // calc relative new dest area points (relative to orig
                // source area)
                const ::basegfx::B2IVector aDestUpperLeftOffset(
                    aLocalDestArea.getMinimum()-io_rDestPoint );
                const ::basegfx::B2IVector aDestLowerRightOffset(
                    aLocalDestArea.getMaximum()-io_rDestPoint );

                io_rSourceArea = ::basegfx::B2IRange( aSourceTopLeft + aDestUpperLeftOffset,
                                                      aSourceTopLeft + aDestLowerRightOffset );
                io_rDestPoint  = aLocalDestArea.getMinimum();

                if( o_pDestArea )
                    *o_pDestArea = aLocalDestArea;

                return true;
            }
        }

        bool clipScrollArea( ::basegfx::B2IRange&                  io_rSourceArea,
                             ::basegfx::B2IPoint&                  io_rDestPoint,
                             ::std::vector< ::basegfx::B2IRange >& o_ClippedAreas,
                             const ::basegfx::B2IRange&            rBounds )
        {
            ::basegfx::B2IRange aResultingDestArea;

            // compute full destination area (to determine uninitialized
            // areas below)
            const ::basegfx::B2I64Tuple& rRange( io_rSourceArea.getRange() );
            ::basegfx::B2IRange aInputDestArea( io_rDestPoint.getX(),
                                                io_rDestPoint.getY(),
                                                (io_rDestPoint.getX()
                                                 + static_cast<sal_Int32>(rRange.getX())),
                                                (io_rDestPoint.getY()
                                                 + static_cast<sal_Int32>(rRange.getY())) );
            // limit to output area (no point updating outside of it)
            aInputDestArea.intersect( rBounds );

            // clip to rBounds
            if( !clipAreaImpl( &aResultingDestArea,
                               io_rSourceArea,
                               io_rDestPoint,
                               rBounds,
                               rBounds ) )
                return false;

            // finally, compute all areas clipped off the total
            // destination area.
            ::basegfx::computeSetDifference( o_ClippedAreas,
                                             aInputDestArea,
                                             aResultingDestArea );

            return true;
        }

        bool clipBlit( ::basegfx::B2IRange&       io_rSourceArea,
                       ::basegfx::B2IPoint&       io_rDestPoint,
                       const ::basegfx::B2IRange& rSourceBounds,
                       const ::basegfx::B2IRange& rDestBounds )
        {
            return clipAreaImpl( NULL,
                                 io_rSourceArea,
                                 io_rDestPoint,
                                 rSourceBounds,
                                 rDestBounds );
        }

        ::basegfx::B2IRange spritePixelAreaFromB2DRange( const ::basegfx::B2DRange& rRange )
        {
            if( rRange.isEmpty() )
                return ::basegfx::B2IRange();

            const ::basegfx::B2IPoint aTopLeft( ::basegfx::fround( rRange.getMinX() ),
                                                ::basegfx::fround( rRange.getMinY() ) );
            return ::basegfx::B2IRange( aTopLeft,
                                        aTopLeft + ::basegfx::B2IPoint(
                                            ::basegfx::fround( rRange.getWidth() ),
                                            ::basegfx::fround( rRange.getHeight() ) ) );
        }

        ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D( const uno::Reference< rendering::XPolyPolygon2D >& xPoly )
        {
            ::canvas::LinePolyPolygonBase* pPolyImpl =
                dynamic_cast< ::canvas::LinePolyPolygonBase* >( xPoly.get() );

            if( pPolyImpl )
            {
                return pPolyImpl->getPolyPolygon();
            }
            else
            {
                const sal_Int32 nPolys( xPoly->getNumberOfPolygons() );

                // not a known implementation object - try data source
                // interfaces
                uno::Reference< rendering::XBezierPolyPolygon2D > xBezierPoly(
                    xPoly,
                    uno::UNO_QUERY );

                if( xBezierPoly.is() )
                {
                    return ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence(
                        xBezierPoly->getBezierSegments( 0,
                                                        nPolys,
                                                        0,
                                                        -1 ) );
                }
                else
                {
                    uno::Reference< rendering::XLinePolyPolygon2D > xLinePoly(
                        xPoly,
                        uno::UNO_QUERY );

                    // no implementation class and no data provider
                    // found - contract violation.
                    CHECK_AND_THROW( xLinePoly.is(),
                                     "canvas::tools::polyPolygonFromXPolyPolygon2D(): Invalid input "
                                     "poly-polygon, cannot retrieve vertex data" );

                    return ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence(
                        xLinePoly->getPoints( 0,
                                              nPolys,
                                              0,
                                              -1 ) );
                }
            }
        }

        uno::Sequence< uno::Any >& getDeviceInfo( const uno::Reference< rendering::XCanvas >& i_rxCanvas,
                                                  uno::Sequence< uno::Any >&                  o_rxParams )
        {
            o_rxParams.realloc( 0 );

            if( i_rxCanvas.is() )
            {
                try
                {
                    uno::Reference< rendering::XGraphicDevice > xDevice( i_rxCanvas->getDevice(),
                                                                         uno::UNO_QUERY_THROW );

                    uno::Reference< lang::XServiceInfo >  xServiceInfo( xDevice,
                                                                        uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xPropSet( xDevice,
                                                                    uno::UNO_QUERY_THROW );

                    o_rxParams.realloc( 2 );

                    o_rxParams[ 0 ] = uno::makeAny( xServiceInfo->getImplementationName() );
                    o_rxParams[ 1 ] = uno::makeAny( xPropSet->getPropertyValue(
                                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DeviceHandle") ) ) );
                }
                catch( uno::Exception& )
                {
                    // ignore, but return empty sequence
                }
            }

            return o_rxParams;
        }

        awt::Rectangle getAbsoluteWindowRect( const awt::Rectangle&                  rRect,
                                              const uno::Reference< awt::XWindow2 >& xWin  )
        {
            awt::Rectangle aRetVal( rRect );

            ::Window* pWindow = VCLUnoHelper::GetWindow(xWin);
            if( pWindow )
            {
                ::Point aPoint( aRetVal.X,
                                aRetVal.Y );

                aPoint = pWindow->OutputToScreenPixel( aPoint );

                aRetVal.X = aPoint.X();
                aRetVal.Y = aPoint.Y();
            }

            return aRetVal;
        }

        ::basegfx::B2DPolyPolygon getBoundMarksPolyPolygon( const ::basegfx::B2DRange& rRange )
        {
            ::basegfx::B2DPolyPolygon aPolyPoly;
            ::basegfx::B2DPolygon     aPoly;

            const double nX0( rRange.getMinX() );
            const double nY0( rRange.getMinY() );
            const double nX1( rRange.getMaxX() );
            const double nY1( rRange.getMaxY() );

            aPoly.append( ::basegfx::B2DPoint( nX0+4,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY0+4 ) );
            aPolyPoly.append( aPoly ); aPoly.clear();

            aPoly.append( ::basegfx::B2DPoint( nX1-4,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY0+4 ) );
            aPolyPoly.append( aPoly ); aPoly.clear();

            aPoly.append( ::basegfx::B2DPoint( nX0+4,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY1-4 ) );
            aPolyPoly.append( aPoly ); aPoly.clear();

            aPoly.append( ::basegfx::B2DPoint( nX1-4,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY1-4 ) );
            aPolyPoly.append( aPoly );

            return aPolyPoly;
        }

    } // namespace tools

} // namespace canvas
