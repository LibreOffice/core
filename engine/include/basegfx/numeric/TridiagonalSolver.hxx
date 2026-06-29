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
/** Thomas-algorithm solver for a regular tridiagonal system A x = b.
 *
 *  The class wraps the three diagonals and one or more right-hand sides.
 *  The solve method runs forward elimination and back substitution in
 *  place. The diagonal array is modified during the solve and is not in
 *  a usable state afterwards. Each right-hand side is overwritten with
 *  its solution on success; when the solve fails, the right-hand sides
 *  hold partially eliminated values, not the original b.
 */
class BASEGFX_DLLPUBLIC TridiagonalSolver
{
public:
    /** Set up a solver for the tridiagonal A x = b. All three diagonal
     *  arrays have size n. aSubDiagonal[i] holds A[i][i-1] for i in
     *  1..n-1 (entry 0 is unused). aDiagonal[i] holds A[i][i]. And
     *  aSuperDiagonal[i] holds A[i][i+1] for i in 0..n-2 (entry n-1 is
     *  unused). Each right-hand side in aRhsList is overwritten with
     *  its solution by solve(). They share the factorisation and the
     *  viewed storage must outlive this solver.
     */
    TridiagonalSolver(std::span<const double> aSubDiagonal, std::span<double> aDiagonal,
                      std::span<const double> aSuperDiagonal,
                      std::span<const std::span<double>> aRhsList);

    TridiagonalSolver(const TridiagonalSolver&) = delete;
    TridiagonalSolver& operator=(const TridiagonalSolver&) = delete;

    /** Solve A x = b in place. An empty system trivially succeeds.
     *  Returns false when a zero pivot appears during elimination,
     *  meaning the matrix cannot be factored without pivoting.
     */
    bool solve();

private:
    bool forwardEliminate();
    void backSubstitute();

    std::span<const double> maSubDiagonal;
    std::span<double> maDiagonal;
    std::span<const double> maSuperDiagonal;
    std::span<const std::span<double>> maRhsList;
};

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
