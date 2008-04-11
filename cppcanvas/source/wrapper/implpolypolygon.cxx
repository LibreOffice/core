/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implpolypolygon.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_cppcanvas.hxx"
#include <rtl/math.hxx>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>

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
