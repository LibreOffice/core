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

#include <matrixdecomposition.hxx>

#include <kahan.hxx>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <utility>

namespace sc {

namespace {

/** How far from orthogonal a pair of columns may stay. The measure it is
    compared against is a cosine, so this number carries no unit and holds for
    an input of any magnitude.
 */
constexpr double fOrthogonalityTolerance = 8.0 * std::numeric_limits<double>::epsilon();

/** Sweeps to spend before settling for what has been reached. One-sided Jacobi
    needs six to ten sweeps on ordinary input and gains an order of magnitude
    of orthogonality per sweep near the end, so this limit bounds the work done
    on an input that keeps drifting.
 */
constexpr std::size_t nMaximumSweepCount = 60;

/** The point above which a cotangent squared already swamps the one that is
    added to it. This is one over the square root of the epsilon, which for a
    double is two to the twenty-sixth power.
 */
constexpr double fLargeCotangent = 0x1p26;

/// Inner product of two columns of a matrix held column by column.
double lcl_GetColumnInnerProduct(const std::vector<double>& rMatrix, std::size_t nRowCount,
                                 std::size_t nFirstColumn, std::size_t nSecondColumn)
{
    const double* pFirst = rMatrix.data() + nFirstColumn * nRowCount;
    const double* pSecond = rMatrix.data() + nSecondColumn * nRowCount;
    KahanSum aSum;
    for (std::size_t nRow = 0; nRow < nRowCount; ++nRow)
        aSum += pFirst[nRow] * pSecond[nRow];
    return aSum.get();
}

/** Turn two columns of a matrix held column by column through the angle whose
    cosine and sine are given.
 */
void lcl_RotateColumnPair(std::vector<double>& rMatrix, std::size_t nRowCount,
                          std::size_t nFirstColumn, std::size_t nSecondColumn, double fCosine,
                          double fSine)
{
    double* pFirst = rMatrix.data() + nFirstColumn * nRowCount;
    double* pSecond = rMatrix.data() + nSecondColumn * nRowCount;
    for (std::size_t nRow = 0; nRow < nRowCount; ++nRow)
    {
        const double fFirst = pFirst[nRow];
        const double fSecond = pSecond[nRow];
        pFirst[nRow] = fCosine * fFirst - fSine * fSecond;
        pSecond[nRow] = fSine * fFirst + fCosine * fSecond;
    }
}

} // namespace

void DecomposeSingularValues(const std::vector<double>& rValues, std::size_t nRowCount,
                             std::size_t nColumnCount, SingularValueDecompositionResult& rResult)
{
    assert(nRowCount > 0 && nColumnCount > 0);
    assert(rValues.size() == nRowCount * nColumnCount);

    // Jacobi rotations orthogonalize columns, so work on whichever of the input
    // and its transpose has at least as many rows as columns. That keeps the
    // pair loop and the rotation accumulator down to the rank of the input.
    const bool bTransposed = nRowCount < nColumnCount;
    const std::size_t nWorkRows = bTransposed ? nColumnCount : nRowCount;
    const std::size_t nWorkColumns = bTransposed ? nRowCount : nColumnCount;

    std::vector<double> aWork(nWorkRows * nWorkColumns);
    if (bTransposed)
    {
        for (std::size_t nColumn = 0; nColumn < nColumnCount; ++nColumn)
        {
            for (std::size_t nRow = 0; nRow < nRowCount; ++nRow)
            {
                aWork[nRow * nColumnCount + nColumn] = rValues[nColumn * nRowCount + nRow];
            }
        }
    }
    else
    {
        aWork = rValues;
    }

    // Every element is squared while an angle is worked out, so bring the whole
    // matrix into a range where the squares are representable and take the
    // factor back out of the singular values at the end. The factor is the same
    // for every element, which is what makes each singular value of the scaled
    // matrix simply the original one divided by it.
    double fScale = 0.0;
    for (double fValue : aWork)
    {
        fScale = std::max(fScale, std::abs(fValue));
    }

    if (fScale == 0.0)
    {
        fScale = 1.0;
    }
    else
    {
        for (double& rValue : aWork)
        {
            rValue /= fScale;
        }
    }

    // Each rotation is recorded here, so this grows into the matrix of right
    // singular vectors. Every column keeps length one because a rotation
    // preserves length.
    std::vector<double> aRotations(nWorkColumns * nWorkColumns, 0.0);
    for (std::size_t nDiagonal = 0; nDiagonal < nWorkColumns; ++nDiagonal)
    {
        aRotations[nDiagonal * nWorkColumns + nDiagonal] = 1.0;
    }

    // A lone column has no pair to be orthogonal to.
    bool bConverged = nWorkColumns < 2;
    std::size_t nSweep = 0;
    while (!bConverged && nSweep < nMaximumSweepCount)
    {
        ++nSweep;
        double fLargestCosine = 0.0;
        for (std::size_t nLeft = 0; nLeft + 1 < nWorkColumns; ++nLeft)
        {
            for (std::size_t nRight = nLeft + 1; nRight < nWorkColumns; ++nRight)
            {
                const double fLeftSquare
                    = lcl_GetColumnInnerProduct(aWork, nWorkRows, nLeft, nLeft);
                const double fRightSquare
                    = lcl_GetColumnInnerProduct(aWork, nWorkRows, nRight, nRight);
                const double fCrossProduct
                    = lcl_GetColumnInnerProduct(aWork, nWorkRows, nLeft, nRight);

                // A column of length zero counts as already at right angles to
                // every other column.
                if (fLeftSquare == 0.0 || fRightSquare == 0.0)
                {
                    continue;
                }

                // The cosine of the angle between the pair says how far from
                // orthogonal they are, whatever their lengths.
                const double fCosine
                    = std::abs(fCrossProduct) / std::sqrt(fLeftSquare * fRightSquare);
                fLargestCosine = std::max(fLargestCosine, fCosine);
                if (fCosine <= fOrthogonalityTolerance)
                {
                    continue;
                }

                // The turn that drives the inner product of the pair to zero
                // has a tangent solving t*t + 2*fCotangent*t - 1 = 0. Take the
                // root of smaller size, so the turn stays within 45 degrees,
                // and write it as a reciprocal so that a large cotangent keeps
                // its accuracy. The cosine test above leaves fCrossProduct
                // non-zero.
                const double fCotangent = (fRightSquare - fLeftSquare) / (2.0 * fCrossProduct);
                const double fSign = fCotangent >= 0.0 ? 1.0 : -1.0;
                double fTangent;
                if (std::abs(fCotangent) > fLargeCotangent)
                {
                    // Here one plus the cotangent squared is the cotangent
                    // squared to working precision, which turns the root of
                    // smaller size into a plain reciprocal and keeps the square
                    // out of the picture.
                    fTangent = 0.5 / fCotangent;
                }
                else
                {
                    fTangent = fSign
                               / (std::abs(fCotangent)
                                  + std::sqrt(1.0 + fCotangent * fCotangent));
                }

                const double fCosineRotation = 1.0 / std::sqrt(1.0 + fTangent * fTangent);
                const double fSineRotation = fCosineRotation * fTangent;

                lcl_RotateColumnPair(aWork, nWorkRows, nLeft, nRight, fCosineRotation,
                                     fSineRotation);
                lcl_RotateColumnPair(aRotations, nWorkColumns, nLeft, nRight, fCosineRotation,
                                     fSineRotation);
            }
        }
        bConverged = fLargestCosine <= fOrthogonalityTolerance;
    }

    // The columns are mutually orthogonal now, and their lengths are the
    // singular values of the scaled matrix.
    std::vector<double> aNorms(nWorkColumns);
    for (std::size_t nColumn = 0; nColumn < nWorkColumns; ++nColumn)
    {
        aNorms[nColumn]
            = std::sqrt(lcl_GetColumnInnerProduct(aWork, nWorkRows, nColumn, nColumn));
    }

    // A stable sort keeps columns that share a singular value in the order the
    // sweeps left them, so the same input always gives the same vectors.
    std::vector<std::size_t> aOrder(nWorkColumns);
    std::iota(aOrder.begin(), aOrder.end(), std::size_t(0));
    std::stable_sort(aOrder.begin(), aOrder.end(),
                     [&aNorms](std::size_t nFirst, std::size_t nSecond)
                     { return aNorms[nFirst] > aNorms[nSecond]; });

    const double fNullThreshold = aNorms[aOrder[0]] * static_cast<double>(nWorkColumns)
                                  * std::numeric_limits<double>::epsilon();

    std::vector<double> aSingularValues(nWorkColumns);
    std::vector<double> aLeftVectors(nWorkRows * nWorkColumns, 0.0);
    std::vector<double> aRightVectors(nWorkColumns * nWorkColumns);
    for (std::size_t nColumn = 0; nColumn < nWorkColumns; ++nColumn)
    {
        const std::size_t nSource = aOrder[nColumn];
        const double fNorm = aNorms[nSource];

        // The scale factor was divided out of the matrix, so it belongs to the
        // singular values alone. The columns being divided by fNorm here still
        // hold scaled values, which is what makes them come out at length one.
        aSingularValues[nColumn] = fNorm * fScale;
        if (fNorm > fNullThreshold)
        {
            for (std::size_t nRow = 0; nRow < nWorkRows; ++nRow)
                aLeftVectors[nColumn * nWorkRows + nRow]
                    = aWork[nSource * nWorkRows + nRow] / fNorm;
        }
        // Any direction at right angles to the others suits a singular value of
        // zero equally well, so all zeros is the choice that stays the same
        // whatever way the arithmetic rounds. The input still comes back out of
        // the three factors, because that column meets a zero singular value.

        for (std::size_t nRow = 0; nRow < nWorkColumns; ++nRow)
        {
            aRightVectors[nColumn * nWorkColumns + nRow]
                = aRotations[nSource * nWorkColumns + nRow];
        }
    }

    rResult.maSingularValues = std::move(aSingularValues);
    if (bTransposed)
    {
        // Transposing swaps the two families: the right vectors of the
        // transpose are the left vectors of the input.
        rResult.maLeftVectors = std::move(aRightVectors);
        rResult.maRightVectors = std::move(aLeftVectors);
    }
    else
    {
        rResult.maLeftVectors = std::move(aLeftVectors);
        rResult.maRightVectors = std::move(aRightVectors);
    }
    rResult.mnSweepCount = nSweep;
    rResult.mbConverged = bConverged;
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
