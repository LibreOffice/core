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

#include <basegfx/curve/BSpline.hxx>

#include <basegfx/numeric/GaussianElimination.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <algorithm>
#include <cmath>
#include <span>
#include <utility>

namespace basegfx
{
namespace
{
// Largest degree the matrix solver still handles in a sensible amount of time.
constexpr sal_uInt32 nMaxDegree = 15;

// Maximum recursion depth for the high-degree subdivision in getPolygon().
constexpr sal_uInt32 nHighDegreeRecursionLimit = 10;

// Maximum allowed midpoint approximation error, as a fraction of the chord length.
constexpr double fHighDegreeMidpointTolerance = 0.01;

/** Centripetal parameter values for the input points. Returns false when
 *  two adjacent points are identical (and the parameter cannot be defined)
 *  or the total chord length is zero.
 */
bool buildCentripetalParameters(const std::vector<B2DPoint>& rPoints,
                                std::vector<double>& rParameters)
{
    const size_t nLastIndex = rPoints.size() - 1;
    rParameters.assign(nLastIndex + 1, 0.0);

    std::vector<double> aIncrements(nLastIndex);
    double fTotal = 0.0;
    for (size_t nI = 1; nI <= nLastIndex; ++nI)
    {
        double fDx = rPoints[nI].getX() - rPoints[nI - 1].getX();
        double fDy = rPoints[nI].getY() - rPoints[nI - 1].getY();
        if (fDx == 0.0 && fDy == 0.0)
            return false;
        // Centripetal parameterisation with alpha = 1/2: increment is the
        // fourth root of the squared chord length.
        double fIncrement = std::sqrt(std::hypot(fDx, fDy));
        aIncrements[nI - 1] = fIncrement;
        fTotal += fIncrement;
    }
    if (fTotal == 0.0)
        return false;

    double fRunning = 0.0;
    for (size_t nI = 1; nI < nLastIndex; ++nI)
    {
        fRunning += aIncrements[nI - 1];
        rParameters[nI] = fRunning / fTotal;
    }
    rParameters[nLastIndex] = 1.0;

    // The construction above keeps the sequence weakly monotone. A cheap
    // strict-monotone check confirms the values are strictly increasing
    // even after floating-point rounding.
    double fPrev = 0.0;
    for (size_t nI = 1; nI <= nLastIndex; ++nI)
    {
        if (fPrev >= rParameters[nI])
            return false;
        fPrev = rParameters[nI];
    }
    return true;
}

/** Clamped non-uniform knot vector. The first p + 1 and last p + 1 knots
 *  are 0 and 1 respectively. Interior knots are the moving average of
 *  the input parameters.
 */
void buildClampedKnotVector(const std::vector<double>& rT, sal_uInt32 nDegree,
                            std::vector<double>& rKnots)
{
    const size_t nLastIndex = rT.size() - 1;
    const size_t nLastKnotIndex = nLastIndex + nDegree + 1;
    rKnots.assign(nLastKnotIndex + 1, 0.0);

    for (size_t nI = nLastKnotIndex - nDegree; nI <= nLastKnotIndex; ++nI)
        rKnots[nI] = 1.0;

    for (size_t nJ = 1; nJ + nDegree <= nLastIndex; ++nJ)
    {
        double fSum = 0.0;
        for (sal_uInt32 nI = 0; nI < nDegree; ++nI)
            fSum += rT[nJ + nI];
        rKnots[nJ + nDegree] = fSum / static_cast<double>(nDegree);
    }
}

/** Fill one row of the basis-function matrix at parameter fT. The row
 *  stores only the nDegree + 1 nonzero entries N_{nIndex - nDegree} .. N_{nIndex}
 *  in a compact layout. nIndex identifies the leftmost basis function.
 */
void applyBasisFunctionsAtParameter(size_t nIndex, double fT, sal_uInt32 nDegree,
                                    const std::vector<double>& rKnots, double* pRow)
{
    pRow[nDegree] = 1.0;
    for (sal_uInt32 nLevel = 1; nLevel <= nDegree; ++nLevel)
    {
        double fLeftFactor = 0.0;
        double fRightFactor
            = (rKnots[nIndex + 1] - fT) / (rKnots[nIndex + 1] - rKnots[nIndex - nLevel + 1]);
        pRow[nDegree - nLevel] = fRightFactor * pRow[nDegree - nLevel + 1];

        for (sal_uInt32 nJ = nLevel - 1; nJ >= 1; --nJ)
        {
            fLeftFactor
                = (fT - rKnots[nIndex - nJ]) / (rKnots[nIndex - nJ + nLevel] - rKnots[nIndex - nJ]);
            fRightFactor = (rKnots[nIndex - nJ + nLevel + 1] - fT)
                           / (rKnots[nIndex - nJ + nLevel + 1] - rKnots[nIndex - nJ + 1]);
            pRow[nDegree - nJ]
                = fLeftFactor * pRow[nDegree - nJ] + fRightFactor * pRow[nDegree - nJ + 1];
        }

        fLeftFactor = (fT - rKnots[nIndex]) / (rKnots[nIndex + nLevel] - rKnots[nIndex]);
        pRow[nDegree] = fLeftFactor * pRow[nDegree];
    }
}

/** Build the banded basis-function matrix A of the interpolation
 *  system A * c = P mapping spline control points to input points.
 *  Each row stores
 *  the nDegree + 1 nonzero entries N_{rShift[k]} .. N_{rShift[k] + nDegree}
 *  in a compact layout. The first and last rows pin the first and last
 *  control points to the first and last input, which is what makes the
 *  curve interpolating at the endpoints.
 */
void buildBandedBasisMatrix(const std::vector<double>& rT, const std::vector<double>& rKnots,
                            sal_uInt32 nDegree, std::vector<std::vector<double>>& rMatrix,
                            std::vector<size_t>& rShift)
{
    const size_t nLastIndex = rT.size() - 1;
    rMatrix.assign(nLastIndex + 1, std::vector<double>(nDegree + 1, 0.0));
    rShift.assign(nLastIndex + 1, 0);

    rMatrix[0][0] = 1.0;
    rMatrix[nLastIndex][0] = 1.0;
    rShift[nLastIndex] = nLastIndex;

    for (size_t nK = 1; nK + 1 <= nLastIndex; ++nK)
    {
        size_t nI = nDegree;
        while (rKnots[nI] > rT[nK] || rT[nK] >= rKnots[nI + 1])
            ++nI;
        rShift[nK] = nI - nDegree;
        applyBasisFunctionsAtParameter(nI, rT[nK], nDegree, rKnots, rMatrix[nK].data());
    }
}

/** de Casteljau evaluation of a single Bezier segment at fT in [0, 1].
 *  The control points are stored consecutively for one coordinate axis.
 */
double evaluateBezierDeCasteljau(std::span<const double> aControlPoints, sal_uInt32 nDegree,
                                 double fT)
{
    std::vector<double> aWork(aControlPoints.begin(), aControlPoints.begin() + (nDegree + 1));
    for (sal_uInt32 nLevel = 1; nLevel <= nDegree; ++nLevel)
    {
        for (sal_uInt32 nI = 0; nI + nLevel <= nDegree; ++nI)
            aWork[nI] = (1.0 - fT) * aWork[nI] + fT * aWork[nI + 1];
    }
    return aWork[0];
}

/** Recursively replace a high-degree (p > 3) Bezier piece by one or more
 *  cubic Bezier approximations. The convergence test compares the true
 *  curve midpoint at t = 0.5 with the candidate cubic's midpoint, so it
 *  tracks real approximation error.
 */
void appendHighDegreeAsCubic(std::span<const double> aControlPointsX,
                             std::span<const double> aControlPointsY, sal_uInt32 nDegree,
                             B2DPolygon& rPolygon, sal_uInt32 nRecursionDepth)
{
    B2DPoint aStart(aControlPointsX[0], aControlPointsY[0]);
    B2DPoint aEnd(aControlPointsX[nDegree], aControlPointsY[nDegree]);

    // Cubic that matches endpoints and end-tangents of the degree-p curve.
    // Degree-p tangent at t=0 is p * (P_1 - P_0). Cubic tangent at t=0 is
    // 3 * (Q_1 - Q_0). Equating them gives Q_1 = P_0 + (p / 3) * (P_1 - P_0).
    double fScale = static_cast<double>(nDegree) / 3.0;
    B2DPoint aControlA(aControlPointsX[0] + fScale * (aControlPointsX[1] - aControlPointsX[0]),
                       aControlPointsY[0] + fScale * (aControlPointsY[1] - aControlPointsY[0]));
    B2DPoint aControlB(aControlPointsX[nDegree]
                           + fScale * (aControlPointsX[nDegree - 1] - aControlPointsX[nDegree]),
                       aControlPointsY[nDegree]
                           + fScale * (aControlPointsY[nDegree - 1] - aControlPointsY[nDegree]));

    if (nRecursionDepth == 0)
    {
        rPolygon.appendBezierSegment(aControlA, aControlB, aEnd);
        return;
    }

    // de Casteljau triangle at t = 0.5: gives both the exact curve
    // midpoint at the apex and the two subdivided halves along its sides.
    std::vector<std::vector<double>> aTriangleX(nDegree + 1);
    std::vector<std::vector<double>> aTriangleY(nDegree + 1);
    aTriangleX[0].assign(aControlPointsX.begin(), aControlPointsX.begin() + (nDegree + 1));
    aTriangleY[0].assign(aControlPointsY.begin(), aControlPointsY.begin() + (nDegree + 1));
    for (sal_uInt32 nLevel = 1; nLevel <= nDegree; ++nLevel)
    {
        aTriangleX[nLevel].resize(nDegree + 1 - nLevel);
        aTriangleY[nLevel].resize(nDegree + 1 - nLevel);
        for (sal_uInt32 nI = 0; nI + nLevel <= nDegree; ++nI)
        {
            aTriangleX[nLevel][nI]
                = 0.5 * (aTriangleX[nLevel - 1][nI] + aTriangleX[nLevel - 1][nI + 1]);
            aTriangleY[nLevel][nI]
                = 0.5 * (aTriangleY[nLevel - 1][nI] + aTriangleY[nLevel - 1][nI + 1]);
        }
    }

    double fTrueMidX = aTriangleX[nDegree][0];
    double fTrueMidY = aTriangleY[nDegree][0];
    double fCubicMidX
        = 0.125 * (aStart.getX() + 3.0 * aControlA.getX() + 3.0 * aControlB.getX() + aEnd.getX());
    double fCubicMidY
        = 0.125 * (aStart.getY() + 3.0 * aControlA.getY() + 3.0 * aControlB.getY() + aEnd.getY());

    double fError = std::hypot(fTrueMidX - fCubicMidX, fTrueMidY - fCubicMidY);
    double fChord = B2DVector(aEnd - aStart).getLength();

    if (fError <= fChord * fHighDegreeMidpointTolerance)
    {
        rPolygon.appendBezierSegment(aControlA, aControlB, aEnd);
        return;
    }

    std::vector<double> aLeftX(nDegree + 1), aLeftY(nDegree + 1);
    std::vector<double> aRightX(nDegree + 1), aRightY(nDegree + 1);
    for (sal_uInt32 nI = 0; nI <= nDegree; ++nI)
    {
        aLeftX[nI] = aTriangleX[nI][0];
        aLeftY[nI] = aTriangleY[nI][0];
        aRightX[nI] = aTriangleX[nDegree - nI][nI];
        aRightY[nI] = aTriangleY[nDegree - nI][nI];
    }

    appendHighDegreeAsCubic(aLeftX, aLeftY, nDegree, rPolygon, nRecursionDepth - 1);
    appendHighDegreeAsCubic(aRightX, aRightY, nDegree, rPolygon, nRecursionDepth - 1);
}

/** Raises each distinct internal knot of a clamped B-spline to
 *  multiplicity nDegree using Boehm's algorithm. After convert() returns,
 *  the knot vector and the control-point arrays describe the same curve
 *  in piecewise Bezier form: each maximal-multiplicity internal knot is
 *  a Bezier segment boundary, and the control points between two such
 *  knots are the cubic (or higher) Bezier control points for that
 *  segment.
 */
class BSplineToBezierConverter
{
public:
    BSplineToBezierConverter(std::vector<double>& rKnots, std::vector<double>& rControlPointsX,
                             std::vector<double>& rControlPointsY, sal_uInt32 nDegree)
        : mrKnots(rKnots)
        , mrControlPointsX(rControlPointsX)
        , mrControlPointsY(rControlPointsY)
        , mnDegree(nDegree)
    {
    }

    void convert();

private:
    void insertKnot(size_t nSpan, double fT);

    std::vector<double>& mrKnots;
    std::vector<double>& mrControlPointsX;
    std::vector<double>& mrControlPointsY;
    sal_uInt32 mnDegree;
};

void BSplineToBezierConverter::convert()
{
    // The clamped knot vector has nLastIndex + nDegree + 2 entries. The
    // first nDegree + 1 and last nDegree + 1 are the clamp values, the
    // rest are the strictly interior knots.
    const size_t nLastIndex = mrKnots.size() - mnDegree - 2;

    // Walk the interior knots once and collect each distinct value, then
    // raise each one to multiplicity nDegree by repeated single-knot
    // insertion. The scan happens before any insertion, so it sees the
    // original knot multiplicities.
    std::vector<double> aDistinctInternal;
    for (size_t nI = mnDegree + 1; nI <= nLastIndex; ++nI)
    {
        if (aDistinctInternal.empty() || mrKnots[nI] != aDistinctInternal.back())
            aDistinctInternal.push_back(mrKnots[nI]);
    }

    for (double fKnotValue : aDistinctInternal)
    {
        for (sal_uInt32 nInsertion = 0; nInsertion + 1 < mnDegree; ++nInsertion)
        {
            size_t nSpan = mnDegree;
            while (nSpan + 1 < mrKnots.size() && mrKnots[nSpan + 1] <= fKnotValue)
                ++nSpan;
            insertKnot(nSpan, fKnotValue);
        }
    }
}

/** Boehm's single knot insertion. Inserts fT into the knot vector at the
 *  span k (mrKnots[k] <= fT <= mrKnots[k+1]) and produces the new
 *  control points by convex combination.
 */
void BSplineToBezierConverter::insertKnot(size_t nSpan, double fT)
{
    const size_t nOld = mrControlPointsX.size();
    std::vector<double> aNewX(nOld + 1);
    std::vector<double> aNewY(nOld + 1);

    for (size_t nI = 0; nI + mnDegree <= nSpan; ++nI)
    {
        aNewX[nI] = mrControlPointsX[nI];
        aNewY[nI] = mrControlPointsY[nI];
    }

    for (size_t nI = nSpan - mnDegree + 1; nI <= nSpan; ++nI)
    {
        const double fDenominator = mrKnots[nI + mnDegree] - mrKnots[nI];
        const double fAlpha = (fDenominator != 0.0) ? (fT - mrKnots[nI]) / fDenominator : 0.0;
        aNewX[nI] = (1.0 - fAlpha) * mrControlPointsX[nI - 1] + fAlpha * mrControlPointsX[nI];
        aNewY[nI] = (1.0 - fAlpha) * mrControlPointsY[nI - 1] + fAlpha * mrControlPointsY[nI];
    }

    for (size_t nI = nSpan + 1; nI <= nOld; ++nI)
    {
        aNewX[nI] = mrControlPointsX[nI - 1];
        aNewY[nI] = mrControlPointsY[nI - 1];
    }

    mrControlPointsX = std::move(aNewX);
    mrControlPointsY = std::move(aNewY);
    mrKnots.insert(mrKnots.begin() + nSpan + 1, fT);
}

} // anonymous namespace

void BSpline::build(const std::vector<B2DPoint>& rPoints, sal_uInt32 nDegree)
{
    if (nDegree < 1 || nDegree > nMaxDegree)
        return;
    if (rPoints.size() < 2)
        return;

    // Drop adjacent duplicate input points so every chord between
    // consecutive points has a positive length, which the centripetal
    // parameter step further down relies on.
    std::vector<B2DPoint> aUnique = rPoints;
    aUnique.erase(std::unique(aUnique.begin(), aUnique.end()), aUnique.end());

    const size_t nLastIndex = aUnique.size() - 1;
    if (nLastIndex < 1 || nDegree > nLastIndex)
        return;

    if (!buildCentripetalParameters(aUnique, maInputParameters))
        return;

    std::vector<double> aKnots;
    buildClampedKnotVector(maInputParameters, nDegree, aKnots);

    std::vector<std::vector<double>> aBasisMatrix;
    std::vector<size_t> aShift;
    buildBandedBasisMatrix(maInputParameters, aKnots, nDegree, aBasisMatrix, aShift);

    // Solve the linear system A * c = P for the spline control points,
    // for X and Y simultaneously since both share the same matrix A.
    // Each axis is initialised with the input point coordinates - the
    // points the curve must pass through - and the solve overwrites
    // them in place with the spline control points c.
    std::vector<double> aSplineX(nLastIndex + 1);
    std::vector<double> aSplineY(nLastIndex + 1);
    for (size_t nI = 0; nI <= nLastIndex; ++nI)
    {
        aSplineX[nI] = aUnique[nI].getX();
        aSplineY[nI] = aUnique[nI].getY();
    }
    std::span<double> aRhs[] = { aSplineX, aSplineY };
    GaussianElimination aSolver(aBasisMatrix, aShift, nDegree, aRhs);
    if (!aSolver.solveBanded())
        return;

    mnDegree = nDegree;
    mnSegments = nLastIndex - nDegree + 1;

    // Segment boundary parameters come from aKnots before the Bezier
    // conversion modifies it.
    maSegmentBoundaries.assign(mnSegments + 1, 0.0);
    for (size_t nI = 0; nI <= mnSegments; ++nI)
        maSegmentBoundaries[nI] = aKnots[nDegree + nI];

    // Convert the B-spline to piecewise Bezier form in place on the
    // member buffers. aKnots is consumed by the conversion.
    maBezierX = std::move(aSplineX);
    maBezierY = std::move(aSplineY);
    BSplineToBezierConverter(aKnots, maBezierX, maBezierY, nDegree).convert();

    mbValid = true;
}

void BSpline::locate(double fT, size_t& rSegment, double& rLocalT) const
{
    if (fT <= 0.0)
    {
        rSegment = 0;
        rLocalT = 0.0;
        return;
    }
    if (fT >= 1.0)
    {
        rSegment = mnSegments - 1;
        rLocalT = 1.0;
        return;
    }
    size_t nLow = 0;
    size_t nHigh = mnSegments;
    while (nHigh - nLow > 1)
    {
        size_t nMid = (nLow + nHigh) / 2;
        if (maSegmentBoundaries[nMid] > fT)
            nHigh = nMid;
        else
            nLow = nMid;
    }
    rSegment = nLow;
    double fSpan = maSegmentBoundaries[nLow + 1] - maSegmentBoundaries[nLow];
    rLocalT = (fSpan != 0.0) ? (fT - maSegmentBoundaries[nLow]) / fSpan : 0.0;
}

BSpline::BSpline(const std::vector<B2DPoint>& rPoints, sal_uInt32 nDegree)
{
    build(rPoints, nDegree);
}

B2DPolygon BSpline::getPolygon() const
{
    B2DPolygon aPolygon;
    if (!mbValid)
        return aPolygon;

    aPolygon.append(B2DPoint(maBezierX[0], maBezierY[0]));
    for (size_t nSegment = 0; nSegment < mnSegments; ++nSegment)
    {
        size_t nBase = nSegment * mnDegree;
        switch (mnDegree)
        {
            case 1:
                aPolygon.append(B2DPoint(maBezierX[nBase + 1], maBezierY[nBase + 1]));
                break;
            case 2:
                aPolygon.appendQuadraticBezierSegment(
                    B2DPoint(maBezierX[nBase + 1], maBezierY[nBase + 1]),
                    B2DPoint(maBezierX[nBase + 2], maBezierY[nBase + 2]));
                break;
            case 3:
                aPolygon.appendBezierSegment(B2DPoint(maBezierX[nBase + 1], maBezierY[nBase + 1]),
                                             B2DPoint(maBezierX[nBase + 2], maBezierY[nBase + 2]),
                                             B2DPoint(maBezierX[nBase + 3], maBezierY[nBase + 3]));
                break;
            default:
                appendHighDegreeAsCubic(std::span(maBezierX).subspan(nBase, mnDegree + 1),
                                        std::span(maBezierY).subspan(nBase, mnDegree + 1), mnDegree,
                                        aPolygon, nHighDegreeRecursionLimit);
                break;
        }
    }
    return aPolygon;
}

B2DPoint BSpline::getPosition(double fT) const
{
    if (!mbValid)
        return B2DPoint(0.0, 0.0);

    size_t nSegment = 0;
    double fLocal = 0.0;
    locate(fT, nSegment, fLocal);

    const size_t nBase = nSegment * mnDegree;
    double fX = evaluateBezierDeCasteljau(std::span(maBezierX).subspan(nBase, mnDegree + 1),
                                          mnDegree, fLocal);
    double fY = evaluateBezierDeCasteljau(std::span(maBezierY).subspan(nBase, mnDegree + 1),
                                          mnDegree, fLocal);
    return B2DPoint(fX, fY);
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
