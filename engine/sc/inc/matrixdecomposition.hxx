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

#include "scdllapi.h"

#include <cstddef>
#include <vector>

namespace sc {

/** The three factors of a thin singular value decomposition.

    For an input with nRowCount rows and nColumnCount columns, and with
    nRank = min(nRowCount, nColumnCount), the input equals
    maLeftVectors * diag(maSingularValues) * transpose(maRightVectors).

    Every member holds its elements column by column, so the element at a given
    row and column sits at the column number times the row count, plus the row
    number.
 */
struct SingularValueDecompositionResult
{
    /** nRowCount rows by nRank columns. A column is either of length one or,
        when the singular value that belongs to it counts as zero, all zeros.
     */
    std::vector<double> maLeftVectors;

    /// nRank values, none of them negative, largest first.
    std::vector<double> maSingularValues;

    /// nColumnCount rows by nRank columns, every column of length one.
    std::vector<double> maRightVectors;

    /// Sweeps performed over the column pairs.
    std::size_t mnSweepCount = 0;

    /** True when the columns reached mutual orthogonality, false when the
        sweep limit stopped the search first.
     */
    bool mbConverged = false;
};

/** Decompose a real matrix into its singular values and the two families of
    singular vectors, using one-sided Jacobi rotations.

    rValues holds nRowCount * nColumnCount elements column by column. Both
    counts must be greater than zero.
 */
SC_DLLPUBLIC void DecomposeSingularValues(const std::vector<double>& rValues, std::size_t nRowCount,
                                          std::size_t nColumnCount,
                                          SingularValueDecompositionResult& rResult);

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
