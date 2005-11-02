/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygonclipper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:53:12 $
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

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#define _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B2DRange;

    namespace tools
    {
        // This method clips the given PolyPolygon against a horizontal or vertical axis (parallell to X or Y axis). The axis is
        // defined by bParallelToXAxis (true -> it's parallel to the X-Axis of the coordinate system, else to the Y-Axis) and the
        // fValueOnOtherAxis (gives the translation to the coordinate system axis). For example, when You want to define
        // a clip axis parallel to X.Axis and 100 above it, use bParallelToXAxis = true and fValueOnOtherAxis = 100.
        // The value bAboveAxis defines on which side the return value will be (true -> above X, right of Y).
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B2DPolyPolygon clipPolyPolygonOnParallelAxis(const B2DPolyPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke);

        // version for Polygons
        B2DPolyPolygon clipPolygonOnParallelAxis(const B2DPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke);

        // Clip the given PolyPolygon against the given range. bInside defines if the result will contain the
        // parts which are contained in the range or vice versa.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B2DPolyPolygon clipPolyPolygonOnRange(const B2DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);

        // version for Polygons
        B2DPolyPolygon clipPolygonOnRange(const B2DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke);

        // Clip given PolyPolygon against the endless edge defined by the given two points. bAbove defines on which side
        // of the edge the result will be together with the definition of the edge. If the edge is seen as a vector
        // from A to B and bAbove is true, the result will contain the geometry left of the vector.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        B2DPolyPolygon clipPolyPolygonOnEdge(const B2DPolyPolygon& rCandidate, const B2DPoint& rPointA, const B2DPoint& rPointB, bool bAbove, bool bStroke);

        // version for Polygons
        B2DPolyPolygon clipPolygonOnEdge(const B2DPolygon& rCandidate, const B2DPoint& rPointA, const B2DPoint& rPointB, bool bAbove, bool bStroke);

        // Clip given PolyPolygon against given clipping polygon.
        // The switch bStroke decides if the polygon is interpreted as area (false) or strokes (true).
        // With stroke polygons, You get all ine snippets inside rCip. For convenience, You may use bInvert to get everything outside rClip.
        // With filled polygons, You get all PolyPolygon parts which were inside rClip. Those polygons are in no way reduced, so
        // it may be necessary to clean them up using their overlapping information.
        B2DPolyPolygon clipPolyPolygonOnPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bStroke, bool bInvert = false);

        // version for Polygons
        B2DPolyPolygon clipPolygonOnPolyPolygon(const B2DPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bStroke, bool bInvert = false);

        // clip the given polygon against the given range. the resulting polygon will always contain
        // the inside parts which will always be interpreted as areas. the incoming polygon is expected
        // to be a simple triangle list. the result is also a simple triangle list.
        B2DPolygon clipTriangleListOnRange( const B2DPolygon& rCandidate, const B2DRange& rRange );

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX */
