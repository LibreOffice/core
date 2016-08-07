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
#include <iostream>
#include <basegfx/basegfxdllapi.h>


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
    class BASEGFX_DLLPUBLIC DebugPlotter : private ::boost::noncopyable
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
