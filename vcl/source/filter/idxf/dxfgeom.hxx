/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <vector>

// Geometry shared by the DXF entities, in the plane of the entity coordinate system

// A vertex of a 2D polyline: the bulge and the widths are those of the segment starting here.
struct DXFPathVertex
{
    basegfx::B2DPoint maPoint;
    // tangent of a quarter of the included angle of the arc to the next vertex,
    // positive for counter-clockwise
    double mfBulge = 0.0;
    double mfStartWidth = 0.0;
    double mfEndWidth = 0.0;
};

namespace dxf
{
// Appends the arc of the ellipse center + U * cos(t) + V * sin(t) from fStart over fSweep
// (radians, counter-clockwise if positive) as curve segments. The start point is appended
// only if bWithStart is set, otherwise the polygon must already end there.
void appendEllipseArc(basegfx::B2DPolygon& rPolygon, const basegfx::B2DPoint& rCenter,
                      const basegfx::B2DVector& rU, const basegfx::B2DVector& rV, double fStart,
                      double fSweep, bool bWithStart);

// Appends the segment from the last point of the polygon to rEnd: a straight line or, for a
// bulge, a circular arc.
void appendBulge(basegfx::B2DPolygon& rPolygon, const basegfx::B2DPoint& rEnd, double fBulge);

// The centre line of a 2D polyline, with arcs as curves.
basegfx::B2DPolygon createPolyline(const std::vector<DXFPathVertex>& rVertices, bool bClosed);

// The outlines of the segments of a polyline with widths, one filled polygon per segment.
// The segments without a width are not part of it.
basegfx::B2DPolyPolygon createWidePolyline(const std::vector<DXFPathVertex>& rVertices,
                                           bool bClosed);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
