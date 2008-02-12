/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testtools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:26:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
