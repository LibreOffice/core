/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dlinegeometry.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:27:30 $
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

#ifndef _BGFX_POLYGON_B2DLINEGEOMETRY_HXX
#define _BGFX_POLYGON_B2DLINEGEOMETRY_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        /** Descriptor for type of Line Joint
        */
        enum B2DLineJoin
        {
            B2DLINEJOIN_NONE,       // no rounding
            B2DLINEJOIN_MIDDLE,     // calc middle value between joints
            B2DLINEJOIN_BEVEL,      // join edges with line
            B2DLINEJOIN_MITER,      // extend till cut
            B2DLINEJOIN_ROUND       // create arc
        };

        // create area geometry for given polygon. Edges are joined using the given
        // join type. fHalfLineWidth defines the relative width.
        // fDegreeStepWidth is used when rounding edges.
        // fMiterMinimumAngle is used to define when miter is forced to bevel.
        // All created polygons will be positively or neuteral oriented and free of
        // self intersections.
        B2DPolyPolygon createAreaGeometryForPolygon(
            const B2DPolygon& rCandidate,
            double fHalfLineWidth,
            B2DLineJoin eJoin,
            double fDegreeStepWidth = (10.0 * F_PI180),
            double fMiterMinimumAngle = (15.0 * F_PI180));

        // create line start/end geometry element, mostly arrows and things like that.
        // rCandidate is the polygon which needs to get that line ends and needs to have
        // two points at least.
        // rArrow is the line start/end geometry. It is assumed that the tip is pointing
        // upwards. Result will be rotated and scaled to fit.
        // bStart describes if creation is for start or end of candidate.
        // fWidth defines the size of the element, it's describing the target width in X
        // of the arrow.
        // fDockingPosition needs to be in [0.0 ..1.0] range, where 0.0 means that the tip
        // of the arrow will be aligned with the polygon start, 1.0 means the bottom. The
        // default of 0.5 describes a centered arrow.
        // With pConsumedLength it is possible to get back how much from the candidate
        // geometry is overlapped by the creted element.
        B2DPolyPolygon createAreaGeometryForLineStartEnd(
            const B2DPolygon& rCandidate,
            const B2DPolyPolygon& rArrow,
            bool bStart,
            double fWidth,
            double fDockingPosition = 0.5, // 0->top, 1->bottom
            double* pConsumedLength = 0L);

    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_B2DLINEGEOMETRY_HXX */
// eof
