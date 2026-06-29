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

#include <cstddef>
#include <vector>

namespace basegfx
{
class B2DPolygon;
}

namespace basegfx
{
/** Interpolating cubic spline through a sequence of 2D points.
 *
 *  Each axis is fitted independently as a parametric cubic spline against
 *  uniform integer parameters (t = 0, 1, ..., n), so the curve hits every
 *  input point in order. The spline is kept in piecewise cubic Bezier form
 *  so getPolygon comes out cheaply.
 *
 *  Two boundary kinds are supported. Natural sets the second derivative to
 *  zero at the first and last point. Periodic closes the curve smoothly
 *  back to the start; for that to be well-defined the first and last
 *  input points must coincide, otherwise isValid() returns false.
 */
class BASEGFX_DLLPUBLIC CubicSpline
{
public:
    enum class BoundaryCondition
    {
        Natural,
        Periodic
    };

    CubicSpline(const std::vector<B2DPoint>& rPoints, BoundaryCondition eBoundary);

    CubicSpline(const CubicSpline&) = delete;
    CubicSpline& operator=(const CubicSpline&) = delete;

    bool isValid() const { return mbValid; }

    /** Bezier polygon equivalent of the curve. Each interval between two
     *  consecutive input points becomes one cubic Bezier segment whose
     *  inner control points are derived from the second derivatives at the
     *  segment's endpoints. Returns an empty polygon when isValid() is
     *  false.
     */
    B2DPolygon getPolygon() const;

    /** Position on the curve at the given parameter value. fT is clamped
     *  to [0.0, 1.0]; fT == 0.0 maps to the first input point and
     *  fT == 1.0 to the last. Returns the origin when isValid() is false.
     */
    B2DPoint getPosition(double fT) const;

private:
    void build(const std::vector<B2DPoint>& rPoints, BoundaryCondition eBoundary);
    size_t segmentCount() const { return maX.empty() ? 0 : maX.size() - 1; }

    bool mbValid = false;
    BoundaryCondition meBoundary = BoundaryCondition::Natural;

    // Input X and Y coordinates at uniform parameter t = 0, 1, ..., n.
    std::vector<double> maX;
    std::vector<double> maY;

    // Second derivatives S''(t_k) for each axis at the same parameter
    // points; size matches maX / maY.
    std::vector<double> maMx;
    std::vector<double> maMy;
};

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
