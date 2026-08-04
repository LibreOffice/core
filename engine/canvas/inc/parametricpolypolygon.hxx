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

#pragma once

#include <basegfx/polygon/b2dpolygon.hxx>

#include <canvas/canvastoolsdllapi.h>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/color.hxx>
#include <utility>

namespace vclcanvas { class Canvas; }


/* Definition of ParametricPolyPolygon class */

namespace canvas
{
    class CANVASTOOLS_DLLPUBLIC ParametricPolyPolygon final : public salhelper::SimpleReferenceObject
    {
    public:
        enum class GradientType
        {
            Linear,
            Elliptical,
            Rectangular
        };

        /** Structure of defining values for the ParametricPolyPolygon

            This is used to copy the state of the
            ParametricPolyPolygon atomically.
         */
        struct Values
        {
            Values( ::basegfx::B2DPolygon                               aGradientPoly,
                    const std::vector< ::Color >&                       rColors,
                    const cpo::uno::Sequence< double >&                 rStops,
                    double                                              nAspectRatio,
                    GradientType                                        eType ) :
                maGradientPoly(std::move( aGradientPoly )),
                mnAspectRatio( nAspectRatio ),
                maColors( rColors ),
                maStops( rStops ),
                meType( eType )
            {
            }

            /// Polygonal gradient shape (ignored for linear and axial gradient)
            const ::basegfx::B2DPolygon                         maGradientPoly;

            /// Aspect ratio of gradient, affects scaling of innermost gradient polygon
            const double                                        mnAspectRatio;

            /// Gradient colors
            const std::vector<::Color>                          maColors;

            /// Gradient color stops
            const cpo::uno::Sequence< double >                  maStops;

            /// Type of gradient to render (as e.g. linear grads are not represented by maGradientPoly)
            const GradientType                                  meType;
        };

        static rtl::Reference<ParametricPolyPolygon> create(
            std::u16string_view rServiceName,
            const std::vector< ::Color >& colors,
            const ::cpo::uno::Sequence< double >& stops,
            double aspectRatio );

        /// Query all defining values of this object atomically
        const Values & getValues() const;

    private:
        virtual ~ParametricPolyPolygon() override; // we're a ref-counted UNO class. _We_ destroy ourselves.
        ParametricPolyPolygon(const ParametricPolyPolygon&) = delete;
        ParametricPolyPolygon& operator=( const ParametricPolyPolygon& ) = delete;

        static rtl::Reference<ParametricPolyPolygon> createLinearHorizontalGradient(
                                                                      const std::vector< ::Color >& colors,
                                                                      const cpo::uno::Sequence< double >& stops );
        static rtl::Reference<ParametricPolyPolygon> createEllipticalGradient(
                                                                const std::vector< ::Color >& colors,
                                                                const cpo::uno::Sequence< double >& stops,
                                                                double fAspect );
        static rtl::Reference<ParametricPolyPolygon> createRectangularGradient(
                                                                 const std::vector< ::Color >& colors,
                                                                 const cpo::uno::Sequence< double >& stops,
                                                                 double fAspect );

        /// Private, because objects can only be created from the static factories
        ParametricPolyPolygon( const ::basegfx::B2DPolygon&                     rGradientPoly,
                               GradientType                                     eType,
                               const std::vector< ::Color >&                    colors,
                               const cpo::uno::Sequence< double >&              stops,
                               double                                           nAspectRatio );
        ParametricPolyPolygon( GradientType                                     eType,
                               const std::vector< ::Color >&                    colors,
                               const cpo::uno::Sequence< double >&              stops );

        /// All defining values of this object
        const Values                                         maValues;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
