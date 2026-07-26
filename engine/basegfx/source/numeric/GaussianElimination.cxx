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

#include <basegfx/numeric/GaussianElimination.hxx>

#include <algorithm>
#include <cassert>
#include <utility>

namespace basegfx
{
GaussianElimination::GaussianElimination(std::vector<std::vector<double>>& rRows,
                                         std::vector<size_t>& rRowOffsets, sal_uInt32 nBandwidth,
                                         std::span<const std::span<double>> aRhsList)
    : mrRows(rRows)
    , mrRowOffsets(rRowOffsets)
    , mnBandwidth(nBandwidth)
    , maRhsList(aRhsList)
{
}

bool GaussianElimination::solveBanded()
{
    if (!forwardEliminate())
        return false;
    backSubstitute();
    return true;
}

/** Forward sweep with partial pivoting. Leaves the matrix upper-
 *  triangular with 1s on the diagonal. Returns false on a zero pivot.
 */
bool GaussianElimination::forwardEliminate()
{
    const size_t nLastIndex = mrRows.size() - 1;
    for (size_t nColumn = 0; nColumn <= nLastIndex; ++nColumn)
    {
        // Partial pivot: walk down the column until a nonzero entry is
        // found, then swap that row into the pivot position.
        size_t nRow = nColumn;
        while (nRow < nLastIndex && mrRows[nRow][nColumn - mrRowOffsets[nRow]] == 0.0)
            ++nRow;
        if (mrRows[nRow][nColumn - mrRowOffsets[nRow]] == 0.0)
            return false;
        if (nRow != nColumn)
        {
            std::swap(mrRows[nRow], mrRows[nColumn]);
            std::swap(mrRowOffsets[nRow], mrRowOffsets[nColumn]);
            for (const std::span<double>& aRhs : maRhsList)
                std::swap(aRhs[nRow], aRhs[nColumn]);
        }

        // Normalise the pivot row so the diagonal entry becomes 1.
        double fDivisor = mrRows[nColumn][nColumn - mrRowOffsets[nColumn]];
        for (sal_uInt32 nI = 0; nI <= mnBandwidth; ++nI)
            mrRows[nColumn][nI] /= fDivisor;
        for (const std::span<double>& aRhs : maRhsList)
            aRhs[nColumn] /= fDivisor;

        // Eliminate the column below the pivot. The banded layout means
        // only rows whose stored band reaches the pivot column are
        // touched. Everything else already has a zero there.
        for (size_t nElimRow = nColumn + 1;
             nElimRow < nLastIndex + 1 && mrRowOffsets[nElimRow] <= nColumn; ++nElimRow)
        {
            // The shift also happens when the leading cell is already
            // zero, so every row starts at its diagonal by the time it
            // becomes the pivot.
            double fEliminate = mrRows[nElimRow][0];
            for (sal_uInt32 nI = 1; nI <= mnBandwidth; ++nI)
                mrRows[nElimRow][nI - 1] = mrRows[nElimRow][nI] - fEliminate * mrRows[nColumn][nI];
            mrRows[nElimRow][mnBandwidth] = 0.0;
            ++mrRowOffsets[nElimRow];
            if (fEliminate == 0.0)
                continue;
            for (const std::span<double>& aRhs : maRhsList)
                aRhs[nElimRow] -= fEliminate * aRhs[nColumn];
        }
    }
    return true;
}

/** Back substitution. Cancels the entries above each diagonal 1, so
 *  each right-hand side ends up holding its solution.
 */
void GaussianElimination::backSubstitute()
{
    const size_t nLastIndex = mrRows.size() - 1;
    for (size_t nBackColumn = nLastIndex; nBackColumn >= 1; --nBackColumn)
    {
        size_t nRow = nBackColumn - 1;
        // After forward elimination each row starts at its diagonal, so the
        // entries to cancel sit up to mnBandwidth columns above it.
        while (nBackColumn - nRow <= mnBandwidth)
        {
            assert(mrRowOffsets[nRow] == nRow
                   && "forward elimination leaves each row starting at its diagonal");
            double fEliminate = mrRows[nRow][nBackColumn - mrRowOffsets[nRow]];
            if (fEliminate != 0.0)
            {
                mrRows[nRow][nBackColumn - mrRowOffsets[nRow]] = 0.0;
                for (const std::span<double>& aRhs : maRhsList)
                    aRhs[nRow] -= fEliminate * aRhs[nBackColumn];
            }
            if (nRow == 0)
                break;
            --nRow;
        }
    }
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
