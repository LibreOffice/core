/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/basegfxdllapi.h>
#include <basegfx/point/b2dpoint.hxx>
#include <sal/types.h>

#include <cstddef>
#include <vector>

namespace basegfx
{
class B2DPolygon;
}

namespace basegfx
{
/** Interpolating clamped B-spline curve through a sequence of 2D points.
 *
 *  Construction solves the matrix system that puts the curve through every
 *  input point in order, using centripetal parameterisation for the knot
 *  vector. The curve is kept in piecewise Bezier form so the cubic Bezier
 *  polygon comes out without extra work, and arbitrary sampling uses
 *  de Casteljau on the stored Bezier segments.
 *
 *  Degree is clamped to the range [1, 15]. The input is filtered to remove
 *  identical adjacent points. What remains must contain at least
 *  nDegree + 1 points and successive points must differ, otherwise
 *  isValid() returns false.
 */
class BASEGFX_DLLPUBLIC BSpline
{
public:
    BSpline(const std::vector<B2DPoint>& rPoints, sal_uInt32 nDegree);

    BSpline(const BSpline&) = delete;
    BSpline& operator=(const BSpline&) = delete;

    bool isValid() const { return mbValid; }

    /** Centripetal parameter values for the input points that were used to
     *  build the curve. The first value is 0.0 and the last is 1.0, with
     *  strictly increasing values between. The size matches the number of
     *  input points after adjacent duplicates were removed. Empty when
     *  isValid() returns false.
     */
    const std::vector<double>& getInputParameters() const { return maInputParameters; }

    /** Bezier polygon representation of the curve. Cubic Bezier segments
     *  for degree 3, quadratic for degree 2, plain line segments for
     *  degree 1. Higher degrees are subdivided into cubic Bezier
     *  approximations whose midpoint matches the true curve within about
     *  one percent of each segment's chord length. Returns an empty
     *  polygon when isValid() returns false.
     */
    B2DPolygon getPolygon() const;

    /** Position on the curve at the given parameter value. fT is clamped
     *  to the range [0.0, 1.0]. fT == 0.0 gives the first input point and
     *  fT == 1.0 gives the last. Returns the origin when isValid() is
     *  false.
     */
    B2DPoint getPosition(double fT) const;

private:
    void build(const std::vector<B2DPoint>& rPoints, sal_uInt32 nDegree);

    /** fT is the curve parameter in [0, 1] (0.0 maps to the first input
     *  point, 1.0 to the last). Splits fT into the index of the Bezier
     *  segment that contains it (rSegment) and the position inside that
     *  segment (rLocalT, also in [0, 1]).
     */
    void locate(double fT, size_t& rSegment, double& rLocalT) const;

    bool mbValid = false;
    sal_uInt32 mnDegree = 0;
    size_t mnSegments = 0;

    // Centripetal parameter values for the input points after duplicate
    // removal. Size matches the deduplicated point count.
    std::vector<double> maInputParameters;

    // Segment boundary parameters. Size is mnSegments + 1, monotone with
    // first value 0.0 and last 1.0.
    std::vector<double> maSegmentBoundaries;

    // Bezier control points after Boehm knot insertion. Size is
    // mnDegree * mnSegments + 1. Segment i takes points
    // [i * mnDegree .. i * mnDegree + mnDegree] inclusive (p + 1 entries).
    std::vector<double> maBezierX;
    std::vector<double> maBezierY;
};

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
