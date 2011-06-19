/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _BASEGFX_TESTTOOLS_HXX
#define _BASEGFX_TESTTOOLS_HXX

#include <basegfx/point/b2dpoint.hxx>

#include <vector>
#include <iostream>


namespace basegfx
{
    class B2DCubicBezier;
    class B2DPolygon;
    class B2DPolyPolygon;
    class B2DRange;

    namespace testtools
    {
        class Plotter
        {
        public:
            /** Create a plotter for the given output stream

                This class can be used to generate gnuplot scripts for
                a number of basegfx graphics primitives, useful for
                debugging, regression-testing and comparing basegfx.
             */
            Plotter( ::std::ostream& rOutputStream );

            /** Delete the plotter

                This implicitely flushes all potential pending writes
                to the output stream
             */
            ~Plotter();

            /** Plot a 2d polygon into the current graph
             */
            void plot( const B2DPolygon& rPoly );

            /** Plot a 2d polyPolygon into the current graph
             */
            void plot( const B2DPolyPolygon& rPolyPoly );

            /** Plot a 2d point into the current graph
             */
            void plot( const B2DPoint& rPoint );

            /** Plot a 2d rectangle into the current graph
             */
            void plot( const B2DRange& rRect );

            /** Plot a 2d line into the current graph
             */
            void plot( const B2DPoint& rStartPoint, const B2DPoint& rEndPoint );

            /** Plot a 2d cubic bezier curve into the current graph
             */
            void plot( const B2DCubicBezier& rCurve );

        private:
            void writeSeparator();

            ::std::ostream&             mrOutputStream;
            ::std::vector< B2DPoint >   maPoints;
            bool                        mbFirstElement;
        };
    }
}

#endif /* _BASEGFX_TESTTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
