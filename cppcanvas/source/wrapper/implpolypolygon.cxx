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


#include <rtl/math.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>

#include "implpolypolygon.hxx"
#include "tools.hxx"


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
