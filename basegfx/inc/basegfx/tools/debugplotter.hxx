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

#ifndef _BGFX_TOOLS_DEBUGPLOTTER_HXX
#define _BGFX_TOOLS_DEBUGPLOTTER_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <rtl/string.hxx>
#include <boost/utility.hpp> // for noncopyable
#include <vector>
#include <utility>
#include <iosfwd>


namespace basegfx
{
    class B2DCubicBezier;

    /** Generates debug output for various basegfx data types.

        Use this class to produce debug (trace) output for various
        basegfx geometry data types. By default, this class outputs
        via OSL_TRACE (i.e. to stderr), and uses the gnuplot output
        format.

        To be able to generate one coherent block of output, this
        class delays actual writing to its destructor
     */
    class DebugPlotter : private ::boost::noncopyable
    {
    public:
        /** Create new debug output object

            @param pTitle
            Title of the debug output, will appear in trace output
         */
        explicit DebugPlotter( const sal_Char* pTitle );

        /** Create new debug output object

            @param pTitle
            Title of the debug output, will appear in trace output

            @param rOutputStream
            Stream to write output to. Must stay valid over the
            lifetime of this object!
         */
        DebugPlotter( const sal_Char* pTitle,
                      ::std::ostream& rOutputStream );

        ~DebugPlotter();

        void plot( const B2DPoint&          rPoint,
                   const sal_Char*          pTitle );
        void plot( const B2DVector&         rVec,
                   const sal_Char*          pTitle );
        void plot( const B2DCubicBezier&    rBezier,
                   const sal_Char*          pTitle );
        void plot( const B2DRange&          rRange,
                   const sal_Char*          pTitle );
        void plot( const B2DPolygon&        rPoly,
                   const sal_Char*          pTitle );
        void plot( const B2DPolyPolygon&    rPoly,
                   const sal_Char*          pTitle );

    private:
        void print( const sal_Char* );

        ::rtl::OString                                              maTitle;
        ::std::vector< ::std::pair< B2DPoint, ::rtl::OString > >    maPoints;
        ::std::vector< ::std::pair< B2DVector, ::rtl::OString > >   maVectors;
        ::std::vector< ::std::pair< B2DRange, ::rtl::OString > >    maRanges;
        ::std::vector< ::std::pair< B2DPolygon, ::rtl::OString > >  maPolygons;

        ::std::ostream*                                             mpOutputStream;
    };
}

#endif /* _BGFX_TOOLS_DEBUGPLOTTER_HXX */
