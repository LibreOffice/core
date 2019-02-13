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

#ifndef INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX
#define INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XParametricPolyPolygon2D.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <canvas/canvastoolsdllapi.h>

namespace com::sun::star::rendering { class XGraphicDevice; }


/* Definition of ParametricPolyPolygon class */

namespace canvas
{
    typedef cppu::WeakComponentImplHelper< css::rendering::XParametricPolyPolygon2D,
                                           css::lang::XServiceInfo > ParametricPolyPolygon_Base;

    class CANVASTOOLS_DLLPUBLIC ParametricPolyPolygon final : public ::cppu::BaseMutex,
                                  public ParametricPolyPolygon_Base
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
            Values( const ::basegfx::B2DPolygon&                        rGradientPoly,
                    const css::uno::Sequence< css::uno::Sequence< double > >& rColors,
                    const css::uno::Sequence< double >&                 rStops,
                    double                                              nAspectRatio,
                    GradientType                                        eType ) :
                maGradientPoly( rGradientPoly ),
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
            const css::uno::Sequence< css::uno::Sequence< double > >   maColors;

            /// Gradient color stops
            const css::uno::Sequence< double >                  maStops;

            /// Type of gradient to render (as e.g. linear grads are not represented by maGradientPoly)
            const GradientType                                  meType;
        };

        static css::uno::Sequence< OUString > getAvailableServiceNames();
        static ParametricPolyPolygon* create(
            const css::uno::Reference< css::rendering::XGraphicDevice >& rDevice,
            const OUString& rServiceName,
            const css::uno::Sequence< css::uno::Any >& rArgs );

        /// Dispose all internal references
        virtual void SAL_CALL disposing() override;

        // XParametricPolyPolygon2D
        virtual css::uno::Reference< css::rendering::XPolyPolygon2D > SAL_CALL getOutline( double t ) override;
        virtual css::uno::Sequence< double > SAL_CALL getColor( double t ) override;
        virtual css::uno::Sequence< double > SAL_CALL getPointColor( const css::geometry::RealPoint2D& point ) override;
        virtual css::uno::Reference< css::rendering::XColorSpace > SAL_CALL getColorSpace() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        /// Query all defining values of this object atomically
        Values getValues() const;

    private:
        virtual ~ParametricPolyPolygon() override; // we're a ref-counted UNO class. _We_ destroy ourselves.
        ParametricPolyPolygon(const ParametricPolyPolygon&) = delete;
        ParametricPolyPolygon& operator=( const ParametricPolyPolygon& ) = delete;

        static ParametricPolyPolygon* createLinearHorizontalGradient( const css::uno::Reference<
                                                                         css::rendering::XGraphicDevice >& rDevice,
                                                                      const css::uno::Sequence< css::uno::Sequence< double > >& colors,
                                                                      const css::uno::Sequence< double >& stops );
        static ParametricPolyPolygon* createEllipticalGradient( const css::uno::Reference<
                                                                   css::rendering::XGraphicDevice >& rDevice,
                                                                const css::uno::Sequence< css::uno::Sequence< double > >& colors,
                                                                const css::uno::Sequence< double >& stops,
                                                                double fAspect );
        static ParametricPolyPolygon* createRectangularGradient( const css::uno::Reference<
                                                                    css::rendering::XGraphicDevice >& rDevice,
                                                                 const css::uno::Sequence< css::uno::Sequence< double > >& colors,
                                                                 const css::uno::Sequence< double >& stops,
                                                                 double fAspect );

        /// Private, because objects can only be created from the static factories
        ParametricPolyPolygon( const css::uno::Reference<
                                   css::rendering::XGraphicDevice >&            rDevice,
                               const ::basegfx::B2DPolygon&                     rGradientPoly,
                               GradientType                                     eType,
                               const css::uno::Sequence< css::uno::Sequence< double > >&  colors,
                               const css::uno::Sequence< double >&              stops,
                               double                                           nAspectRatio );
        ParametricPolyPolygon( const css::uno::Reference<
                                   css::rendering::XGraphicDevice >&            rDevice,
                               GradientType                                     eType,
                               const css::uno::Sequence< css::uno::Sequence< double > >&  colors,
                               const css::uno::Sequence< double >&              stops );

        css::uno::Reference<
            css::rendering::XGraphicDevice >    mxDevice;

        /// All defining values of this object
        const Values                                         maValues;
    };
}

#endif /* INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
