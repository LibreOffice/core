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
#include <sal/types.h>

#include <cstddef>
#include <span>
#include <vector>

namespace basegfx
{
/** Gaussian elimination with partial pivoting for solving linear systems.
 *
 *  The class wraps the matrix and right-hand sides. The public solve
 *  method then drives forward elimination and back substitution on the
 *  stored data. One instance solves one system: the matrix is modified
 *  in place during the solve and is not in a usable state afterwards.
 */
class BASEGFX_DLLPUBLIC GaussianElimination
{
public:
    /** Set up a solver for A x = b. A is square, stored in banded form:
     *  rRows[i] holds row i's nBandwidth + 1 nonzero cells starting at
     *  column rRowOffsets[i]. Each right-hand side in aRhsList is
     *  overwritten with its solution by solveBanded(). They share the
     *  factorisation and the viewed storage must outlive this solver.
     */
    GaussianElimination(std::vector<std::vector<double>>& rRows, std::vector<size_t>& rRowOffsets,
                        sal_uInt32 nBandwidth, std::span<const std::span<double>> aRhsList);

    GaussianElimination(const GaussianElimination&) = delete;
    GaussianElimination& operator=(const GaussianElimination&) = delete;

    /** Solve A x = b in place by Gaussian elimination with partial
     *  pivoting. Returns false when a zero pivot survives the pivot
     *  search, meaning the matrix is singular.
     */
    bool solveBanded();

private:
    bool forwardEliminate();
    void backSubstitute();

    std::vector<std::vector<double>>& mrRows;
    std::vector<size_t>& mrRowOffsets;
    sal_uInt32 mnBandwidth;
    std::span<const std::span<double>> maRhsList;
};

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
