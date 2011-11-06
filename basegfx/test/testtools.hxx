/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
