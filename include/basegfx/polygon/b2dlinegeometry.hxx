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

#ifndef INCLUDED_BASEGFX_POLYGON_B2DLINEGEOMETRY_HXX
#define INCLUDED_BASEGFX_POLYGON_B2DLINEGEOMETRY_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    namespace utils
    {
        /** Create line start/end geometry element, mostly arrows and things like that.

            @param rCandidate
            The polygon which needs to get that line ends and needs to have two points
            at least.

            @param rArrow
            The line start/end geometry. It is assumed that the tip is pointing
            upwards. Result will be rotated and scaled to fit.

            @param bStart
            describes if creation is for start or end of candidate.

            @param fWidth
            defines the size of the element, it's describing the target width in X
            of the arrow.

            @param fDockingPosition needs to be in [0.0 ..1.0] range, where 0.0 means
            that the tip of the arrow will be aligned with the polygon start, 1.0 means
            the bottom. The default of 0.5 describes a centered arrow.

            @param pConsumedLength
            Using this parameter it is possible to get back how much from the candidate
            geometry is overlapped by the created element (consumed).

            @param fCandidateLength
            This should contain the length of rCandidate to allow work without
            again calculating the length (which may be expensive with beziers). If 0.0 is
            given, the length is calculated on demand.

            @param fShift
            When it is necessary to count with the thickness of the line, it
            makes sense to move the start position slightly - so define the shift.

            @return
            The Line start and end polygon, correctly rotated and scaled
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon createAreaGeometryForLineStartEnd(
            const B2DPolygon& rCandidate,
            const B2DPolyPolygon& rArrow,
            bool bStart,
            double fWidth,
            double fCandidateLength, // 0.0 -> calculate self
            double fDockingPosition = 0.5, // 0->top, 1->bottom
            double* pConsumedLength = nullptr,
            double fShift = 0.0);

        /** create filled polygon geometry for lines with a line width

            This method will create bezier based, fillable polygons which
            will resample the curve if it was extended for the given half
            line width. It will remove extrema positions from contained
            bezier segments and get as close as possible and defined by
            the given parameters to the ideal result.

            It will check edges for trivial bezier to avoid unnecessary
            bezier polygons. Care is taken to produce the in-between
            polygon points (the ones original on the source polygon) since
            it has showed that without those, the raster converters leave
            non-filled gaps.

            @param rCandidate
            The source polygon defining the hairline polygon path

            @param fHalfLineWidth
            The width of the line to one side

            @param eJoin
            The LineJoin if the edges meeting in a point do not have a C1
            or C2 continuity

            @param eCap
            The kind of cap, which is added to the line.

            @param fMaxAllowedAngle
            Allows to hand over the maximum allowed angle between an edge and
            it's control vectors. The smaller, the more subdivisions will be
            needed to create the filled geometry. Allowed range is cropped to
            [F_PI2 .. 0.01 * F_PI2].

            @param fMaxPartOfEdge
            Allows to influence from with relative length of a control vector
            compared to its edge a split is forced. The smaller, the more
            subdivisions will be needed to create the filled geometry. Allowed
            range is cropped to [1.0 .. 0.01]

            @param fMiterMinimumAngle
            The minimum wanted angle between two edges when edge rounding
            is using miter. When an edge is smaller than this (tighter)
            the usual fallback to bevel is used. Allowed range is cropped
            to [F_PI .. 0.01 * F_PI].

            @return
            The tools::PolyPolygon containing the geometry of the extended line by
            it's line width. Contains bezier segments and edge roundings as
            needed and defined.
        */
        BASEGFX_DLLPUBLIC B2DPolyPolygon createAreaGeometry(
            const B2DPolygon& rCandidate,
            double fHalfLineWidth,
            B2DLineJoin eJoin,
            css::drawing::LineCap eCap = css::drawing::LineCap_BUTT,
            double fMaxAllowedAngle = (12.5 * F_PI180),
            double fMaxPartOfEdge = 0.4,
            double fMiterMinimumAngle = (15.0 * F_PI180));

    } // end of namespace utils
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_POLYGON_B2DLINEGEOMETRY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
