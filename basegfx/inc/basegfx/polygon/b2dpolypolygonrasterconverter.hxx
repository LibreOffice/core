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



#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONRASTERCONVERTER_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONRASTERCONVERTER_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygonfillrule.hxx>
#include <vector>
#include <utility>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    /** Raster-convert a poly-polygon.

        This class can raster-convert a given poly-polygon. Simply
        derive from this, and override the span() method, which will
        get called for every scanline span of the poly-polygon.

        @derive
        Overwrite span() with the render output method of your choice.
     */
    class BASEGFX_DLLPUBLIC B2DPolyPolygonRasterConverter
    {
    public:
        /** Create raster-converter for given poly-polygon
         */
        B2DPolyPolygonRasterConverter(const B2DPolyPolygon& rPolyPolyRaster);

        /** Create raster-converter for given poly-polygon and raster
            area.

            @param rPolyPolyRaster
            Poly-Polygon to raster convert

            @param rMinUpdateArea
            Minimal area to touch when raster-converting. The
            rectangle given here is guaranteed to be iterated through
            scanline by scanline (but the raster converter might
            actually use more scanlines, e.g. if the poly-polygon's
            bound rect is larger). One of the cases where this
            parameter comes in handy is when rendering in the 'off'
            spans, and a certain area must be filled. <em>Do not</em>
            use this for clipping, as described above, the touched
            area might also be larger.
         */
        B2DPolyPolygonRasterConverter(const B2DPolyPolygon& rPolyPolyRaster,
                                      const B2DRectangle&   rRasterArea );

        virtual ~B2DPolyPolygonRasterConverter();

        /** Raster-convert the contained poly-polygon

            @param eFillRule
            Fill rule to use for span filling
         */
        void rasterConvert( FillRule eFillRule);

        /** Override this method, to be called for every scanline span
            of the poly-polygon

            @param rfXLeft
            The left end of the current horizontal span

            @param rfXRight
            The right end of the current horizontal span

            @param nY
            The y position of the current horizontal span

            @param bOn
            Denotes whether this span is on or off, according to the
            active fill rule.
        */
        virtual void span(const double& rfXLeft,
                          const double& rfXRight,
                          sal_Int32     nY,
                          bool          bOn ) = 0;

        /// @internal
        struct Vertex
        {
            inline Vertex();
            inline Vertex( const B2DPoint&, const B2DPoint&, bool );

            B2DPoint    aP1;
            B2DPoint    aP2;
            bool        bDownwards;
        };

    private:
        // default: disabled copy/assignment
        B2DPolyPolygonRasterConverter(const B2DPolyPolygonRasterConverter&);
        B2DPolyPolygonRasterConverter& operator=( const B2DPolyPolygonRasterConverter& );

        void init();

        typedef ::std::vector<Vertex>               VectorOfVertices;
        typedef ::std::vector<VectorOfVertices>     VectorOfVertexVectors;

        /// The poly-polygon to raster-convert
        B2DPolyPolygon                              maPolyPolygon;
        /// Total bound rect of the poly-polygon
        const B2DRectangle                          maPolyPolyRectangle;

        /** Vector containing for each scanline a vector which in turn
            contains all vertices that start on the specific scanline
         */
        VectorOfVertexVectors                       maScanlines;
    };
}

#endif /* _BGFX_POLYGON_B2DPOLYPOLYGONRASTERCONVERTER_HXX */
