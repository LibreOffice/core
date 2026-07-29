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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XParametricPolyPolygon2D.hpp>
#include <comphelper/compbase.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <canvas/canvastoolsdllapi.h>
#include <rtl/ref.hxx>
#include "XGraphicDevice.hxx"
#include <utility>

namespace com::sun::star::rendering { class XGraphicDevice; }


/* Definition of ParametricPolyPolygon class */

namespace canvas
{
    typedef comphelper::WeakComponentImplHelper< css::rendering::XParametricPolyPolygon2D,
                                           css::lang::XServiceInfo > ParametricPolyPolygon_Base;

    class CANVASTOOLS_DLLPUBLIC ParametricPolyPolygon final : public ParametricPolyPolygon_Base
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
                    const cpo::uno::Sequence< cpo::uno::Sequence< double > >& rColors,
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
            const cpo::uno::Sequence< cpo::uno::Sequence< double > >   maColors;

            /// Gradient color stops
            const cpo::uno::Sequence< double >                  maStops;

            /// Type of gradient to render (as e.g. linear grads are not represented by maGradientPoly)
            const GradientType                                  meType;
        };

        static rtl::Reference<ParametricPolyPolygon> create(
            const css::uno::Reference< vclcanvas::XGraphicDevice >& rDevice,
            std::u16string_view rServiceName,
            const ::cpo::uno::Sequence< ::cpo::uno::Sequence< double > >& colors,
            const ::cpo::uno::Sequence< double >& stops,
            double aspectRatio );

        /// Dispose all internal references
        virtual void disposing(std::unique_lock<std::mutex>&) override;

        // XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

        /// Query all defining values of this object atomically
        const Values & getValues() const;

    private:
        virtual ~ParametricPolyPolygon() override; // we're a ref-counted UNO class. _We_ destroy ourselves.
        ParametricPolyPolygon(const ParametricPolyPolygon&) = delete;
        ParametricPolyPolygon& operator=( const ParametricPolyPolygon& ) = delete;

        static rtl::Reference<ParametricPolyPolygon> createLinearHorizontalGradient( const css::uno::Reference<
                                                                         vclcanvas::XGraphicDevice >& rDevice,
                                                                      const cpo::uno::Sequence< cpo::uno::Sequence< double > >& colors,
                                                                      const cpo::uno::Sequence< double >& stops );
        static rtl::Reference<ParametricPolyPolygon> createEllipticalGradient( const css::uno::Reference<
                                                                   vclcanvas::XGraphicDevice >& rDevice,
                                                                const cpo::uno::Sequence< cpo::uno::Sequence< double > >& colors,
                                                                const cpo::uno::Sequence< double >& stops,
                                                                double fAspect );
        static rtl::Reference<ParametricPolyPolygon> createRectangularGradient( const css::uno::Reference<
                                                                    vclcanvas::XGraphicDevice >& rDevice,
                                                                 const cpo::uno::Sequence< cpo::uno::Sequence< double > >& colors,
                                                                 const cpo::uno::Sequence< double >& stops,
                                                                 double fAspect );

        /// Private, because objects can only be created from the static factories
        ParametricPolyPolygon( css::uno::Reference<
                                   vclcanvas::XGraphicDevice >             xDevice,
                               const ::basegfx::B2DPolygon&                     rGradientPoly,
                               GradientType                                     eType,
                               const cpo::uno::Sequence< cpo::uno::Sequence< double > >&  colors,
                               const cpo::uno::Sequence< double >&              stops,
                               double                                           nAspectRatio );
        ParametricPolyPolygon( css::uno::Reference<
                                   vclcanvas::XGraphicDevice >             xDevice,
                               GradientType                                     eType,
                               const cpo::uno::Sequence< cpo::uno::Sequence< double > >&  colors,
                               const cpo::uno::Sequence< double >&              stops );

        css::uno::Reference<
            vclcanvas::XGraphicDevice >    mxDevice;

        /// All defining values of this object
        const Values                                         maValues;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
