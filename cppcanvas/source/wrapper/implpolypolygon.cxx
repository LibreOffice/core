/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implpolypolygon.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:43:48 $
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

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _COM_SUN_STAR_RENDERING_PATHJOINTYPE_HPP_
#include <com/sun/star/rendering/PathJoinType.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_PATHCAPTYPE_HPP_
#include <com/sun/star/rendering/PathCapType.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#include <implpolypolygon.hxx>
#include <tools.hxx>


using namespace ::com::sun::star;


namespace cppcanvas
{
    namespace internal
    {
        ImplPolyPolygon::ImplPolyPolygon( const CanvasSharedPtr& rParentCanvas,
                                          const uno::Reference< rendering::XPolyPolygon2D >& rPolyPoly ) :
            CanvasGraphicHelper( rParentCanvas ),
            mxPolyPoly( rPolyPoly ),
            maStrokeAttributes(1.0,
                               10.0,
                               uno::Sequence< double >(),
                               uno::Sequence< double >(),
                               rendering::PathCapType::ROUND,
                               rendering::PathCapType::ROUND,
                               rendering::PathJoinType::ROUND ),
            maFillColor(),
            maStrokeColor(),
            mbFillColorSet( false ),
            mbStrokeColorSet( false )
        {
            OSL_ENSURE( mxPolyPoly.is(), "PolyPolygonImpl::PolyPolygonImpl: no valid polygon" );
        }

        ImplPolyPolygon::~ImplPolyPolygon()
        {
        }

        void ImplPolyPolygon::addPolygon( const ::basegfx::B2DPolygon& rPoly )
        {
            OSL_ENSURE( mxPolyPoly.is(),
                        "ImplPolyPolygon::addPolygon(): Invalid polygon" );

            if( !mxPolyPoly.is() )
                return;

            uno::Reference< rendering::XGraphicDevice > xDevice( getGraphicDevice() );

            OSL_ENSURE( xDevice.is(),
                        "ImplPolyPolygon::addPolygon(): Invalid graphic device" );

            if( !xDevice.is() )
                return;

            mxPolyPoly->addPolyPolygon( geometry::RealPoint2D(0.0, 0.0),
                                        ::basegfx::unotools::xPolyPolygonFromB2DPolygon(
                                            xDevice,
                                            rPoly) );
        }

        void ImplPolyPolygon::addPolyPolygon( const ::basegfx::B2DPolyPolygon& rPoly )
        {
            OSL_ENSURE( mxPolyPoly.is(),
                        "ImplPolyPolygon::addPolyPolygon(): Invalid polygon" );

            if( !mxPolyPoly.is() )
                return;

            uno::Reference< rendering::XGraphicDevice > xDevice( getGraphicDevice() );

            OSL_ENSURE( xDevice.is(),
                        "ImplPolyPolygon::addPolyPolygon(): Invalid graphic device" );

            if( !xDevice.is() )
                return;

            mxPolyPoly->addPolyPolygon( geometry::RealPoint2D(0.0, 0.0),
                                        ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                                            xDevice,
                                            rPoly) );
        }

        void ImplPolyPolygon::setRGBAFillColor( Color::IntSRGBA aColor )
        {
            maFillColor = tools::intSRGBAToDoubleSequence( getGraphicDevice(),
                                                           aColor );
            mbFillColorSet = true;
        }

        void ImplPolyPolygon::setRGBALineColor( Color::IntSRGBA aColor )
        {
            maStrokeColor = tools::intSRGBAToDoubleSequence( getGraphicDevice(),
                                                             aColor );
            mbStrokeColorSet = true;
        }

        Color::IntSRGBA ImplPolyPolygon::getRGBAFillColor() const
        {
            return tools::doubleSequenceToIntSRGBA( getGraphicDevice(),
                                                    maFillColor );
        }

        Color::IntSRGBA ImplPolyPolygon::getRGBALineColor() const
        {
            return tools::doubleSequenceToIntSRGBA( getGraphicDevice(),
                                                    maStrokeColor );
        }

        void ImplPolyPolygon::setStrokeWidth( const double& rStrokeWidth )
        {
            maStrokeAttributes.StrokeWidth = rStrokeWidth;
        }

        double ImplPolyPolygon::getStrokeWidth() const
        {
            return maStrokeAttributes.StrokeWidth;
        }

        bool ImplPolyPolygon::draw() const
        {
            CanvasSharedPtr pCanvas( getCanvas() );

            OSL_ENSURE( pCanvas.get() != NULL &&
                        pCanvas->getUNOCanvas().is(),
                        "ImplBitmap::draw: invalid canvas" );

            if( pCanvas.get() == NULL ||
                !pCanvas->getUNOCanvas().is() )
                return false;

            if( mbFillColorSet )
            {
                rendering::RenderState aLocalState( getRenderState() );
                aLocalState.DeviceColor = maFillColor;

                pCanvas->getUNOCanvas()->fillPolyPolygon( mxPolyPoly,
                                                          pCanvas->getViewState(),
                                                          aLocalState );
            }

            if( mbStrokeColorSet )
            {
                rendering::RenderState aLocalState( getRenderState() );
                aLocalState.DeviceColor = maStrokeColor;

                if( ::rtl::math::approxEqual(maStrokeAttributes.StrokeWidth, 1.0) )
                    pCanvas->getUNOCanvas()->drawPolyPolygon( mxPolyPoly,
                                                              pCanvas->getViewState(),
                                                              aLocalState );
                else
                    pCanvas->getUNOCanvas()->strokePolyPolygon( mxPolyPoly,
                                                                pCanvas->getViewState(),
                                                                aLocalState,
                                                                maStrokeAttributes );
            }

            return true;
        }

        uno::Reference< rendering::XPolyPolygon2D > ImplPolyPolygon::getUNOPolyPolygon() const
        {
            return mxPolyPoly;
        }

    }
}
