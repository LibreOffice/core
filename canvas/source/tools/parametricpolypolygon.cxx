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

#include <limits>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/tools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include <canvas/canvastools.hxx>
#include <canvas/parametricpolypolygon.hxx>

using namespace ::com::sun::star;

namespace canvas
{
    uno::Sequence<OUString> ParametricPolyPolygon::getAvailableServiceNames()
    {
        return {"LinearGradient",
                "EllipticalGradient",
                "RectangularGradient"};
    }

    ParametricPolyPolygon* ParametricPolyPolygon::create(
        const uno::Reference< rendering::XGraphicDevice >& rDevice,
        const OUString& rServiceName,
        const uno::Sequence< uno::Any >& rArgs )
    {
        uno::Sequence< uno::Sequence< double > > colorSequence(2);
        uno::Sequence< double > colorStops(2);
        double fAspectRatio=1.0;

        // defaults
        uno::Sequence< rendering::RGBColor > rgbColors(1);
        rgbColors[0] = rendering::RGBColor(0,0,0);
        colorSequence[0] = rDevice->getDeviceColorSpace()->convertFromRGB(rgbColors);
        rgbColors[0] = rendering::RGBColor(1,1,1);
        colorSequence[1] = rDevice->getDeviceColorSpace()->convertFromRGB(rgbColors);
        colorStops[0] = 0;
        colorStops[1] = 1;

        // extract args
        for( sal_Int32 i=0; i<rArgs.getLength(); ++i )
        {
            beans::PropertyValue aProp;
            if( rArgs[i] >>= aProp )
            {
                if ( aProp.Name == "Colors" )
                {
                    aProp.Value >>= colorSequence;
                }
                else if ( aProp.Name == "Stops" )
                {
                    aProp.Value >>= colorStops;
                }
                else if ( aProp.Name == "AspectRatio" )
                {
                    aProp.Value >>= fAspectRatio;
                }
            }
        }

        if ( rServiceName == "LinearGradient" )
        {
            return createLinearHorizontalGradient(rDevice, colorSequence, colorStops);
        }
        else if ( rServiceName == "EllipticalGradient" )
        {
            return createEllipticalGradient(rDevice, colorSequence, colorStops, fAspectRatio);
        }
        else if ( rServiceName == "RectangularGradient" )
        {
            return createRectangularGradient(rDevice, colorSequence, colorStops, fAspectRatio);
        }
        else if ( rServiceName == "VerticalLineHatch" )
        {
            // TODO: NYI
        }
        else if ( rServiceName == "OrthogonalLinesHatch" )
        {
            // TODO: NYI
        }
        else if ( rServiceName == "ThreeCrossingLinesHatch" )
        {
            // TODO: NYI
        }
        else if ( rServiceName == "FourCrossingLinesHatch" )
        {
            // TODO: NYI
        }

        return nullptr;
    }

    ParametricPolyPolygon* ParametricPolyPolygon::createLinearHorizontalGradient(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const uno::Sequence< uno::Sequence< double > >&     colors,
        const uno::Sequence< double >&                      stops )
    {
        // TODO(P2): hold gradient brush statically, and only setup
        // the colors
        return new ParametricPolyPolygon( rDevice, GradientType::Linear, colors, stops );
    }

    ParametricPolyPolygon* ParametricPolyPolygon::createEllipticalGradient(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const uno::Sequence< uno::Sequence< double > >&     colors,
        const uno::Sequence< double >&                      stops,
        double                                              fAspectRatio )
    {
        // TODO(P2): hold gradient polygon statically, and only setup
        // the colors
        return new ParametricPolyPolygon(
            rDevice,
            ::basegfx::utils::createPolygonFromCircle(
                ::basegfx::B2DPoint(0,0), 1 ),
            GradientType::Elliptical,
            colors, stops, fAspectRatio );
    }

    ParametricPolyPolygon* ParametricPolyPolygon::createRectangularGradient( const uno::Reference< rendering::XGraphicDevice >& rDevice,
                                                                             const uno::Sequence< uno::Sequence< double > >&    colors,
                                                                             const uno::Sequence< double >&                     stops,
                                                                             double                                             fAspectRatio )
    {
        // TODO(P2): hold gradient polygon statically, and only setup
        // the colors
        return new ParametricPolyPolygon(
            rDevice,
            ::basegfx::utils::createPolygonFromRect(
                ::basegfx::B2DRectangle( -1, -1, 1, 1 ) ),
            GradientType::Rectangular,
            colors, stops, fAspectRatio );
    }

    void SAL_CALL ParametricPolyPolygon::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mxDevice.clear();
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL ParametricPolyPolygon::getOutline( double /*t*/ )
    {
        // TODO(F1): outline NYI
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    uno::Sequence< double > SAL_CALL ParametricPolyPolygon::getColor( double /*t*/ )
    {
        // TODO(F1): color NYI
        return uno::Sequence< double >();
    }

    uno::Sequence< double > SAL_CALL ParametricPolyPolygon::getPointColor( const geometry::RealPoint2D& /*point*/ )
    {
        // TODO(F1): point color NYI
        return uno::Sequence< double >();
    }

    uno::Reference< rendering::XColorSpace > SAL_CALL ParametricPolyPolygon::getColorSpace()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mxDevice.is() ? mxDevice->getDeviceColorSpace() : uno::Reference< rendering::XColorSpace >();
    }


    OUString SAL_CALL ParametricPolyPolygon::getImplementationName(  )
    {
        return OUString( "Canvas::ParametricPolyPolygon" );
    }

    sal_Bool SAL_CALL ParametricPolyPolygon::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    uno::Sequence< OUString > SAL_CALL ParametricPolyPolygon::getSupportedServiceNames(  )
    {
        return { "com.sun.star.rendering.ParametricPolyPolygon" };
    }

    ParametricPolyPolygon::~ParametricPolyPolygon()
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( const uno::Reference< rendering::XGraphicDevice >&    rDevice,
                                                  const ::basegfx::B2DPolygon&                          rGradientPoly,
                                                  GradientType                                          eType,
                                                  const uno::Sequence< uno::Sequence< double > >&       rColors,
                                                  const uno::Sequence< double >&                        rStops,
                                                  double                                                nAspectRatio ) :
        ParametricPolyPolygon_Base( m_aMutex ),
        mxDevice( rDevice ),
        maValues( rGradientPoly,
                  rColors,
                  rStops,
                  nAspectRatio,
                  eType )
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( const uno::Reference< rendering::XGraphicDevice >&    rDevice,
                                                  GradientType                                          eType,
                                                  const uno::Sequence< uno::Sequence< double > >&       rColors,
                                                  const uno::Sequence< double >&                        rStops ) :
        ParametricPolyPolygon_Base( m_aMutex ),
        mxDevice( rDevice ),
        maValues( ::basegfx::B2DPolygon(),
                  rColors,
                  rStops,
                  1.0,
                  eType )
    {
    }

    ParametricPolyPolygon::Values ParametricPolyPolygon::getValues() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maValues;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
