/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygonrasterconverter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:29:47 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONRASTERCONVERTER_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONRASTERCONVERTER_HXX

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX
#include <basegfx/polygon/b2dpolypolygonfillrule.hxx>
#endif

#include <vector>
#include <utility>

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
    class B2DPolyPolygonRasterConverter
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
