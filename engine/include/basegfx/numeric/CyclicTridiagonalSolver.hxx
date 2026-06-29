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

#include <span>

namespace basegfx
{
/** Sherman-Morrison solver for a cyclic tridiagonal system A x = b.
 *
 *  A cyclic tridiagonal matrix is a tridiagonal matrix with two extra
 *  corner entries at A[0][n-1] and A[n-1][0]. The class wraps the three
 *  diagonals, the two corner values, and one or more right-hand sides.
 *
 *  Sherman-Morrison reduces the cyclic system to two regular tridiagonal
 *  solves with the same matrix, followed by a rank-one correction. The
 *  inner solve is delegated to TridiagonalSolver, so all matrix-side
 *  bookkeeping is shared. The diagonal array is modified during the
 *  solve and is not in a usable state afterwards. Each right-hand side
 *  is overwritten with its solution on success; when the solve fails,
 *  the right-hand sides hold partially eliminated values, not the
 *  original b.
 */
class BASEGFX_DLLPUBLIC CyclicTridiagonalSolver
{
public:
    /** Set up a solver for the cyclic tridiagonal A x = b. All three
     *  diagonal arrays have size n. aSubDiagonal[i] holds A[i][i-1] for
     *  i in 1..n-1, aDiagonal[i] holds A[i][i], and aSuperDiagonal[i]
     *  holds A[i][i+1] for i in 0..n-2. fTopRight is A[0][n-1] and
     *  fBottomLeft is A[n-1][0]. Each right-hand side in aRhsList is
     *  overwritten with its solution by solve(). They share the
     *  factorisation and the viewed storage must outlive this solver.
     */
    CyclicTridiagonalSolver(std::span<const double> aSubDiagonal, std::span<double> aDiagonal,
                            std::span<const double> aSuperDiagonal, double fTopRight,
                            double fBottomLeft, std::span<const std::span<double>> aRhsList);

    CyclicTridiagonalSolver(const CyclicTridiagonalSolver&) = delete;
    CyclicTridiagonalSolver& operator=(const CyclicTridiagonalSolver&) = delete;

    /** Solve A x = b in place. Returns false when the matrix cannot be
     *  factored along the Sherman-Morrison path. This happens for a
     *  zero first diagonal entry or when the rank-one denominator
     *  collapses to zero.
     */
    bool solve();

private:
    std::span<const double> maSubDiagonal;
    std::span<double> maDiagonal;
    std::span<const double> maSuperDiagonal;
    double mfTopRight;
    double mfBottomLeft;
    std::span<const std::span<double>> maRhsList;
};

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
