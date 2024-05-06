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

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include <parametricpolypolygon.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace canvas
{
    uno::Sequence<OUString> ParametricPolyPolygon::getAvailableServiceNames()
    {
        return {u"LinearGradient"_ustr,
                u"EllipticalGradient"_ustr,
                u"RectangularGradient"_ustr};
    }

    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::create(
        const uno::Reference< rendering::XGraphicDevice >& rDevice,
        std::u16string_view rServiceName,
        const uno::Sequence< uno::Any >& rArgs )
    {
        double fAspectRatio=1.0;

        // defaults
        uno::Sequence< uno::Sequence< double > > colorSequence{
            rDevice->getDeviceColorSpace()->convertFromRGB({ rendering::RGBColor(0,0,0) }),
            rDevice->getDeviceColorSpace()->convertFromRGB({ rendering::RGBColor(1,1,1) })
        };
        uno::Sequence< double > colorStops{ 0, 1 };

        // extract args
        for( const uno::Any& rArg : rArgs )
        {
            beans::PropertyValue aProp;
            if( rArg >>= aProp )
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

        if ( rServiceName == u"LinearGradient" )
        {
            return createLinearHorizontalGradient(rDevice, colorSequence, colorStops);
        }
        else if ( rServiceName == u"EllipticalGradient" )
        {
            return createEllipticalGradient(rDevice, colorSequence, colorStops, fAspectRatio);
        }
        else if ( rServiceName == u"RectangularGradient" )
        {
            return createRectangularGradient(rDevice, colorSequence, colorStops, fAspectRatio);
        }
        else if ( rServiceName == u"VerticalLineHatch" )
        {
            // TODO: NYI
        }
        else if ( rServiceName == u"OrthogonalLinesHatch" )
        {
            // TODO: NYI
        }
        else if ( rServiceName == u"ThreeCrossingLinesHatch" )
        {
            // TODO: NYI
        }
        else if ( rServiceName == u"FourCrossingLinesHatch" )
        {
            // TODO: NYI
        }

        return nullptr;
    }

    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::createLinearHorizontalGradient(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const uno::Sequence< uno::Sequence< double > >&     colors,
        const uno::Sequence< double >&                      stops )
    {
        // TODO(P2): hold gradient brush statically, and only setup
        // the colors
        return new ParametricPolyPolygon( rDevice, GradientType::Linear, colors, stops );
    }

    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::createEllipticalGradient(
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

    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::createRectangularGradient( const uno::Reference< rendering::XGraphicDevice >& rDevice,
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

    void ParametricPolyPolygon::disposing(std::unique_lock<std::mutex>&)
    {
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
        std::unique_lock aGuard( m_aMutex );

        return mxDevice.is() ? mxDevice->getDeviceColorSpace() : uno::Reference< rendering::XColorSpace >();
    }


    OUString SAL_CALL ParametricPolyPolygon::getImplementationName(  )
    {
        return u"Canvas::ParametricPolyPolygon"_ustr;
    }

    sal_Bool SAL_CALL ParametricPolyPolygon::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    uno::Sequence< OUString > SAL_CALL ParametricPolyPolygon::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.rendering.ParametricPolyPolygon"_ustr };
    }

    ParametricPolyPolygon::~ParametricPolyPolygon()
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( uno::Reference< rendering::XGraphicDevice >           xDevice,
                                                  const ::basegfx::B2DPolygon&                          rGradientPoly,
                                                  GradientType                                          eType,
                                                  const uno::Sequence< uno::Sequence< double > >&       rColors,
                                                  const uno::Sequence< double >&                        rStops,
                                                  double                                                nAspectRatio ) :
        mxDevice(std::move( xDevice )),
        maValues( rGradientPoly,
                  rColors,
                  rStops,
                  nAspectRatio,
                  eType )
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( uno::Reference< rendering::XGraphicDevice >           xDevice,
                                                  GradientType                                          eType,
                                                  const uno::Sequence< uno::Sequence< double > >&       rColors,
                                                  const uno::Sequence< double >&                        rStops ) :
        mxDevice(std::move( xDevice )),
        maValues( ::basegfx::B2DPolygon(),
                  rColors,
                  rStops,
                  1.0,
                  eType )
    {
    }

    ParametricPolyPolygon::Values ParametricPolyPolygon::getValues() const
    {
        return maValues;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
