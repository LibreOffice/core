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

#include <basegfx/curve/CubicSpline.hxx>

#include <basegfx/numeric/CyclicTridiagonalSolver.hxx>
#include <basegfx/numeric/TridiagonalSolver.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <cstddef>
#include <span>
#include <vector>

namespace basegfx
{
namespace
{
/** Solve the natural-cubic-spline tridiagonal system on uniform parameters
 *  for both axes at once. Sets rMx[0] = rMx[n] = rMy[0] = rMy[n] = 0 and
 *  fills the interior values so that the resulting cubic interpolation
 *  passes through the input ordinates. rX and rY hold the coordinates at
 *  the n + 1 parameter points. The matrix has diagonal 4 and unit off-
 *  diagonals, the same for both axes, so one factorisation serves both.
 */
bool solveNaturalSpline(const std::vector<double>& rX, const std::vector<double>& rY,
                        std::vector<double>& rMx, std::vector<double>& rMy)
{
    const size_t nN = rX.size() - 1;
    rMx.assign(nN + 1, 0.0);
    rMy.assign(nN + 1, 0.0);
    if (nN < 2)
        return true;

    // Interior system on M_1 .. M_{n-1}. The natural boundary forces
    // M_0 = M_n = 0, which leaves nInner equations with diagonal 4
    // and unit off-diagonals.
    const size_t nInner = nN - 1;
    std::vector<double> aSubDiagonal(nInner, 1.0);
    std::vector<double> aDiagonal(nInner, 4.0);
    std::vector<double> aSuperDiagonal(nInner, 1.0);
    std::vector<double> aRhsX(nInner);
    std::vector<double> aRhsY(nInner);
    for (size_t nI = 0; nI < nInner; ++nI)
    {
        aRhsX[nI] = 6.0 * (rX[nI] - 2.0 * rX[nI + 1] + rX[nI + 2]);
        aRhsY[nI] = 6.0 * (rY[nI] - 2.0 * rY[nI + 1] + rY[nI + 2]);
    }

    std::span<double> aRhsList[] = { aRhsX, aRhsY };
    TridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, aRhsList);
    if (!aSolver.solve())
        return false;

    for (size_t nI = 0; nI < nInner; ++nI)
    {
        rMx[nI + 1] = aRhsX[nI];
        rMy[nI + 1] = aRhsY[nI];
    }
    return true;
}

/** Solve the periodic-cubic-spline system on uniform parameters for both
 *  axes at once. The input must have x_0 == x_n and y_0 == y_n (the
 *  caller checks this), and the result satisfies M_0 == M_n. The two-
 *  and three-point cases fall back to closed-form expressions because
 *  the Sherman-Morrison shift the cyclic solver picks needs more rows
 *  than that to act on.
 */
bool solvePeriodicSpline(const std::vector<double>& rX, const std::vector<double>& rY,
                         std::vector<double>& rMx, std::vector<double>& rMy)
{
    const size_t nN = rX.size() - 1;
    rMx.assign(nN + 1, 0.0);
    rMy.assign(nN + 1, 0.0);
    if (nN < 2)
        return true;

    if (nN == 2)
    {
        // Three points with first == last give two segments around a
        // single interior knot. Solving the cyclic 2x2 system gives:
        const double fHelpX = 6.0 * (rX[0] - rX[1]);
        const double fHelpY = 6.0 * (rY[0] - rY[1]);
        rMx[1] = fHelpX;
        rMx[2] = -fHelpX;
        rMx[0] = rMx[2];
        rMy[1] = fHelpY;
        rMy[2] = -fHelpY;
        rMy[0] = rMy[2];
        return true;
    }

    if (nN == 3)
    {
        // Four points, three segments. The cyclic 3x3 system collapses
        // to these closed-form expressions when h is uniform.
        const double fX0 = rX[1] - rX[0];
        const double fX1 = rX[2] - rX[1];
        const double fX2 = rX[0] - rX[2];
        rMx[1] = 2.0 * (fX1 - fX0);
        rMx[2] = 2.0 * (fX2 - fX1);
        rMx[3] = 2.0 * (fX0 - fX2);
        rMx[0] = rMx[3];
        const double fY0 = rY[1] - rY[0];
        const double fY1 = rY[2] - rY[1];
        const double fY2 = rY[0] - rY[2];
        rMy[1] = 2.0 * (fY1 - fY0);
        rMy[2] = 2.0 * (fY2 - fY1);
        rMy[3] = 2.0 * (fY0 - fY2);
        rMy[0] = rMy[3];
        return true;
    }

    // General case: cyclic tridiagonal system on M_1 .. M_n with
    // diagonal 4, unit off-diagonals, and unit corner entries (since
    // h is uniform).
    std::vector<double> aSubDiagonal(nN, 1.0);
    std::vector<double> aDiagonal(nN, 4.0);
    std::vector<double> aSuperDiagonal(nN, 1.0);
    std::vector<double> aRhsX(nN);
    std::vector<double> aRhsY(nN);
    for (size_t nI = 0; nI + 1 < nN; ++nI)
    {
        aRhsX[nI] = 6.0 * (rX[nI + 2] - 2.0 * rX[nI + 1] + rX[nI]);
        aRhsY[nI] = 6.0 * (rY[nI + 2] - 2.0 * rY[nI + 1] + rY[nI]);
    }
    // The last equation wraps: M_n couples to M_1 through the corner.
    aRhsX[nN - 1] = 6.0 * (rX[1] - 2.0 * rX[nN] + rX[nN - 1]);
    aRhsY[nN - 1] = 6.0 * (rY[1] - 2.0 * rY[nN] + rY[nN - 1]);

    std::span<double> aRhsList[] = { aRhsX, aRhsY };
    CyclicTridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, 1.0, 1.0, aRhsList);
    if (!aSolver.solve())
        return false;

    for (size_t nI = 0; nI < nN; ++nI)
    {
        rMx[nI + 1] = aRhsX[nI];
        rMy[nI + 1] = aRhsY[nI];
    }
    rMx[0] = rMx[nN];
    rMy[0] = rMy[nN];
    return true;
}

} // anonymous namespace

void CubicSpline::build(const std::vector<B2DPoint>& rPoints, BoundaryCondition eBoundary)
{
    if (rPoints.size() < 2)
        return;

    meBoundary = eBoundary;

    maX.resize(rPoints.size());
    maY.resize(rPoints.size());
    for (size_t nI = 0; nI < rPoints.size(); ++nI)
    {
        maX[nI] = rPoints[nI].getX();
        maY[nI] = rPoints[nI].getY();
    }

    if (eBoundary == BoundaryCondition::Periodic)
    {
        if (maX.front() != maX.back() || maY.front() != maY.back())
            return;
        if (rPoints.size() < 3)
            return;
        if (!solvePeriodicSpline(maX, maY, maMx, maMy))
            return;
    }
    else
    {
        if (!solveNaturalSpline(maX, maY, maMx, maMy))
            return;
    }

    mbValid = true;
}

CubicSpline::CubicSpline(const std::vector<B2DPoint>& rPoints, BoundaryCondition eBoundary)
{
    build(rPoints, eBoundary);
}

B2DPolygon CubicSpline::getPolygon() const
{
    B2DPolygon aPolygon;
    if (!mbValid)
        return aPolygon;

    const size_t nSegments = segmentCount();

    aPolygon.append(B2DPoint(maX[0], maY[0]));
    for (size_t nK = 0; nK < nSegments; ++nK)
    {
        // Cubic Bezier (P0, P1, P2, P3) matching the spline endpoints,
        // first derivatives, and second derivatives at t = k and t = k + 1.
        // With h = 1 and the spline S(u) on [k, k+1] equal to
        //   (1-u) y_k + u y_{k+1} + ((1-u)^3 - (1-u)) M_k / 6 + (u^3 - u) M_{k+1} / 6,
        // the slopes give P1 = P0 + dY/3 - M_k/9 - M_{k+1}/18 and similarly
        // for P2; collecting terms with c = M / 6 produces the form used here.
        double fCxk = maMx[nK] / 6.0;
        double fCxk1 = maMx[nK + 1] / 6.0;
        double fCyk = maMy[nK] / 6.0;
        double fCyk1 = maMy[nK + 1] / 6.0;
        double fDx = maX[nK + 1] - maX[nK];
        double fDy = maY[nK + 1] - maY[nK];

        double fP1x = maX[nK] + (fDx - 2.0 * fCxk - fCxk1) / 3.0;
        double fP1y = maY[nK] + (fDy - 2.0 * fCyk - fCyk1) / 3.0;
        double fP2x = maX[nK + 1] - (fDx + fCxk + 2.0 * fCxk1) / 3.0;
        double fP2y = maY[nK + 1] - (fDy + fCyk + 2.0 * fCyk1) / 3.0;

        aPolygon.appendBezierSegment(B2DPoint(fP1x, fP1y), B2DPoint(fP2x, fP2y),
                                     B2DPoint(maX[nK + 1], maY[nK + 1]));
    }
    return aPolygon;
}

B2DPoint CubicSpline::getPosition(double fT) const
{
    if (!mbValid)
        return B2DPoint(0.0, 0.0);

    const size_t nSegments = segmentCount();
    if (nSegments == 0)
        return B2DPoint(0.0, 0.0);

    double fScaled = fT * static_cast<double>(nSegments);
    if (fScaled <= 0.0)
        return B2DPoint(maX[0], maY[0]);
    if (fScaled >= static_cast<double>(nSegments))
        return B2DPoint(maX[nSegments], maY[nSegments]);

    size_t nK = static_cast<size_t>(fScaled);
    if (nK >= nSegments)
        nK = nSegments - 1;
    double fU = fScaled - static_cast<double>(nK);
    double fA = 1.0 - fU;
    double fB = fU;

    double fX = fA * maX[nK] + fB * maX[nK + 1]
                + ((fA * fA * fA - fA) * maMx[nK] + (fB * fB * fB - fB) * maMx[nK + 1]) / 6.0;
    double fY = fA * maY[nK] + fB * maY[nK + 1]
                + ((fA * fA * fA - fA) * maMy[nK] + (fB * fB * fB - fB) * maMy[nK + 1]) / 6.0;
    return B2DPoint(fX, fY);
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
