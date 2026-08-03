/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <parametricpolypolygon.hxx>
#include <canvas.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace canvas
{
    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::create(
        std::u16string_view rServiceName,
        const ::cpo::uno::Sequence< ::cpo::uno::Sequence< double > >& colorSequence,
        const ::cpo::uno::Sequence< double >& colorStops,
        double fAspectRatio )
    {
        if ( rServiceName == u"LinearGradient" )
        {
            return createLinearHorizontalGradient(colorSequence, colorStops);
        }
        else if ( rServiceName == u"EllipticalGradient" )
        {
            return createEllipticalGradient(colorSequence, colorStops, fAspectRatio);
        }
        else if ( rServiceName == u"RectangularGradient" )
        {
            return createRectangularGradient(colorSequence, colorStops, fAspectRatio);
        }
        else
            assert(false && "unsupported service");

        return nullptr;
    }

    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::createLinearHorizontalGradient(
        const cpo::uno::Sequence< cpo::uno::Sequence< double > >&     colors,
        const cpo::uno::Sequence< double >&                      stops )
    {
        // TODO(P2): hold gradient brush statically, and only setup
        // the colors
        return new ParametricPolyPolygon( GradientType::Linear, colors, stops );
    }

    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::createEllipticalGradient(
        const cpo::uno::Sequence< cpo::uno::Sequence< double > >&     colors,
        const cpo::uno::Sequence< double >&                      stops,
        double                                              fAspectRatio )
    {
        // TODO(P2): hold gradient polygon statically, and only setup
        // the colors
        return new ParametricPolyPolygon(
            ::basegfx::utils::createPolygonFromCircle(
                ::basegfx::B2DPoint(0,0), 1 ),
            GradientType::Elliptical,
            colors, stops, fAspectRatio );
    }

    rtl::Reference<ParametricPolyPolygon> ParametricPolyPolygon::createRectangularGradient(
                                                                             const cpo::uno::Sequence< cpo::uno::Sequence< double > >&    colors,
                                                                             const cpo::uno::Sequence< double >&                     stops,
                                                                             double                                             fAspectRatio )
    {
        // TODO(P2): hold gradient polygon statically, and only setup
        // the colors
        return new ParametricPolyPolygon(
            ::basegfx::utils::createPolygonFromRect(
                ::basegfx::B2DRectangle( -1, -1, 1, 1 ) ),
            GradientType::Rectangular,
            colors, stops, fAspectRatio );
    }

    ParametricPolyPolygon::~ParametricPolyPolygon()
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( const ::basegfx::B2DPolygon&                          rGradientPoly,
                                                  GradientType                                          eType,
                                                  const cpo::uno::Sequence< cpo::uno::Sequence< double > >&       rColors,
                                                  const cpo::uno::Sequence< double >&                        rStops,
                                                  double                                                nAspectRatio ) :
        maValues( rGradientPoly,
                  rColors,
                  rStops,
                  nAspectRatio,
                  eType )
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( GradientType                                          eType,
                                                  const cpo::uno::Sequence< cpo::uno::Sequence< double > >&       rColors,
                                                  const cpo::uno::Sequence< double >&                        rStops ) :
        maValues( ::basegfx::B2DPolygon(),
                  rColors,
                  rStops,
                  1.0,
                  eType )
    {
    }

    const ParametricPolyPolygon::Values & ParametricPolyPolygon::getValues() const
    {
        return maValues;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
