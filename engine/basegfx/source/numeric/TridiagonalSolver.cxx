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

#include <basegfx/numeric/TridiagonalSolver.hxx>

#include <algorithm>
#include <cassert>

namespace basegfx
{
TridiagonalSolver::TridiagonalSolver(std::span<const double> aSubDiagonal,
                                     std::span<double> aDiagonal,
                                     std::span<const double> aSuperDiagonal,
                                     std::span<const std::span<double>> aRhsList)
    : maSubDiagonal(aSubDiagonal)
    , maDiagonal(aDiagonal)
    , maSuperDiagonal(aSuperDiagonal)
    , maRhsList(aRhsList)
{
    assert(maSubDiagonal.size() == maDiagonal.size());
    assert(maSuperDiagonal.size() == maDiagonal.size());
    assert(std::all_of(maRhsList.begin(), maRhsList.end(), [this](std::span<const double> aRhs) {
        return aRhs.size() == maDiagonal.size();
    }));
}

bool TridiagonalSolver::solve()
{
    // An empty system has nothing to solve.
    if (maDiagonal.empty())
        return true;
    if (!forwardEliminate())
        return false;
    if (maDiagonal.back() == 0.0)
        return false;
    backSubstitute();
    return true;
}

bool TridiagonalSolver::forwardEliminate()
{
    const size_t nLast = maDiagonal.size() - 1;
    for (size_t nI = 1; nI <= nLast; ++nI)
    {
        if (maDiagonal[nI - 1] == 0.0)
            return false;
        const double fMultiplier = maSubDiagonal[nI] / maDiagonal[nI - 1];
        maDiagonal[nI] -= fMultiplier * maSuperDiagonal[nI - 1];
        for (const std::span<double>& aRhs : maRhsList)
            aRhs[nI] -= fMultiplier * aRhs[nI - 1];
    }
    return true;
}

void TridiagonalSolver::backSubstitute()
{
    const size_t nLast = maDiagonal.size() - 1;
    for (const std::span<double>& aRhs : maRhsList)
    {
        aRhs[nLast] /= maDiagonal[nLast];
        for (size_t nI = nLast; nI >= 1; --nI)
        {
            const size_t nK = nI - 1;
            aRhs[nK] = (aRhs[nK] - maSuperDiagonal[nK] * aRhs[nK + 1]) / maDiagonal[nK];
        }
    }
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
