/*************************************************************************
 *
 *  $RCSfile: canvastools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:07:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_AFFINEMATRIX2D_HPP_
#include <drafts/com/sun/star/geometry/AffineMatrix2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_MATRIX2D_HPP_
#include <drafts/com/sun/star/geometry/Matrix2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <drafts/com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_VIEWSTATE_HPP__
#include <drafts/com/sun/star/rendering/ViewState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <drafts/com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_COMPOSITEOPERATION_HPP__
#include <drafts/com/sun/star/rendering/CompositeOperation.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP__
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#include <canvas/canvastools.hxx>

#include <memory>
#include <algorithm>
#include <cstdio>


using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;

namespace canvas
{
    namespace tools
    {
        // prototypes
        uno::Sequence< uno::Any >& getVCLDeviceInfo( uno::Reference< rendering::XGraphicDevice > xDevice, uno::Sequence< uno::Any >& o_rxParams );
        uno::Sequence< uno::Any >& getDXDeviceInfo( uno::Reference< rendering::XGraphicDevice > xDevice, uno::Sequence< uno::Any >& o_rxParams );

        // ---------------------------------------------------------------------

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
                                                       const uno::Reference< rendering::XCanvas >   xCanvas     )
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
            double left, top, bottom, right;

            // transform all four extremal points of the rectangle,
            // take bounding rect of those.
            ::basegfx::B2DPoint aPoint;

            // transform left-top point
            aPoint.setX( inRect.getMinX() );
            aPoint.setY( inRect.getMinY() );

            aPoint *= transformation;
            left = right = aPoint.getX();
            top = bottom = aPoint.getY();

            // transform bottom-right point
            aPoint.setX( inRect.getMaxX() );
            aPoint.setY( inRect.getMaxY() );

            aPoint *= transformation;
            left   = ::std::min(left, aPoint.getX());
            top    = ::std::min(top, aPoint.getY());
            right  = ::std::max(right, aPoint.getX());
            bottom = ::std::max(bottom, aPoint.getY());

            // transform top-right point
            aPoint.setX( inRect.getMaxX() );
            aPoint.setY( inRect.getMinY() );

            aPoint *= transformation;
            left   = ::std::min(left, aPoint.getX());
            top    = ::std::min(top, aPoint.getY());
            right  = ::std::max(right, aPoint.getX());
            bottom = ::std::max(bottom, aPoint.getY());

            // transform bottom-left point
            aPoint.setX( inRect.getMinX() );
            aPoint.setY( inRect.getMaxY() );

            aPoint *= transformation;
            left   = ::std::min(left, aPoint.getX());
            top    = ::std::min(top, aPoint.getY());
            right  = ::std::max(right, aPoint.getX());
            bottom = ::std::max(bottom, aPoint.getY());

            // over and out.
            outRect = ::basegfx::B2DRectangle( left, top, right, bottom );
            return outRect;
        }

        ::basegfx::B2DHomMatrix& calcRectToRectTransform( ::basegfx::B2DHomMatrix&          o_transform,
                                                          const ::basegfx::B2DRange&        destRect,
                                                          const ::basegfx::B2DRange&        srcRect,
                                                          const ::basegfx::B2DHomMatrix&    transformation )
        {
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
    } // namespace tools

} // namespace canvas
