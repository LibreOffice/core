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

#include <basegfx/numeric/CyclicTridiagonalSolver.hxx>

#include <basegfx/numeric/TridiagonalSolver.hxx>

#include <vector>

namespace basegfx
{
CyclicTridiagonalSolver::CyclicTridiagonalSolver(std::span<const double> aSubDiagonal,
                                                 std::span<double> aDiagonal,
                                                 std::span<const double> aSuperDiagonal,
                                                 double fTopRight, double fBottomLeft,
                                                 std::span<const std::span<double>> aRhsList)
    : maSubDiagonal(aSubDiagonal)
    , maDiagonal(aDiagonal)
    , maSuperDiagonal(aSuperDiagonal)
    , mfTopRight(fTopRight)
    , mfBottomLeft(fBottomLeft)
    , maRhsList(aRhsList)
{
}

bool CyclicTridiagonalSolver::solve()
{
    const size_t nSize = maDiagonal.size();
    if (nSize < 2)
        return false;
    if (maDiagonal[0] == 0.0)
        return false;

    // Sherman-Morrison: write A = A' + u v^T where A' is a regular
    // tridiagonal matrix and u, v are rank-one correction vectors. Then
    // A^{-1} b = A'^{-1} b - (v^T A'^{-1} b) / (1 + v^T A'^{-1} u)
    // times A'^{-1} u. Both inner solves use the same factored A'.
    //
    // Choose gamma = -diag[0] so A' has 2 * diag[0] in its top-left
    // entry. The corresponding u and v vectors are:
    //   u = [gamma, 0, ..., 0, fBottomLeft]
    //   v = [1,     0, ..., 0, fTopRight / gamma]
    // and A' = A - u v^T means subtracting gamma from diag[0] and
    // fTopRight * fBottomLeft / gamma from diag[n-1].
    const double fGamma = -maDiagonal[0];
    const size_t nLast = nSize - 1;

    std::vector<double> aU(nSize, 0.0);
    aU[0] = fGamma;
    aU[nLast] = mfBottomLeft;

    maDiagonal[0] -= fGamma;
    maDiagonal[nLast] -= mfTopRight * mfBottomLeft / fGamma;

    // Inner solve handles all original right-hand sides plus u in one
    // factorisation.
    std::vector<std::span<double>> aExtendedList(maRhsList.begin(), maRhsList.end());
    aExtendedList.push_back(aU);

    TridiagonalSolver aInnerSolver(maSubDiagonal, maDiagonal, maSuperDiagonal, aExtendedList);
    if (!aInnerSolver.solve())
        return false;

    // aU now holds z = A'^{-1} u. Compute the correction denominator
    // 1 + v^T z = 1 + z[0] + (fTopRight / gamma) z[n-1] once.
    const double fInvGamma = 1.0 / fGamma;
    const double fDenominator = 1.0 + aU[0] + mfTopRight * fInvGamma * aU[nLast];
    if (fDenominator == 0.0)
        return false;

    // Each original RHS now holds y = A'^{-1} b. Apply the rank-one
    // correction in place to get x = y - ((v^T y) / fDenominator) z.
    for (const std::span<double>& aRhs : maRhsList)
    {
        const double fVy = aRhs[0] + mfTopRight * fInvGamma * aRhs[nLast];
        const double fFactor = fVy / fDenominator;
        for (size_t nI = 0; nI < nSize; ++nI)
            aRhs[nI] -= fFactor * aU[nI];
    }

    return true;
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
