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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <basegfx/numeric/CyclicTridiagonalSolver.hxx>

#include <span>
#include <vector>

namespace basegfx
{
class CyclicTridiagonalSolverTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(CyclicTridiagonalSolverTest, testIdentityWithZeroCornersIsRegular)
{
    // With both corner entries equal to zero the cyclic system collapses
    // to a regular tridiagonal one. Identity gives the right-hand side
    // back unchanged.
    std::vector<double> aSubDiagonal{ 0.0, 0.0, 0.0 };
    std::vector<double> aDiagonal{ 1.0, 1.0, 1.0 };
    std::vector<double> aSuperDiagonal{ 0.0, 0.0, 0.0 };
    std::vector<double> aRhs{ 2.0, 3.0, 4.0 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    CyclicTridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, 0.0, 0.0, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aRhs[0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aRhs[1], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, aRhs[2], 1e-12);
}

CPPUNIT_TEST_FIXTURE(CyclicTridiagonalSolverTest, testKnownCyclicSystem)
{
    // The 4 by 4 cyclic system with every diagonal entry 4 and every
    // off-diagonal entry (including the two corners) 1:
    //   4 1 0 1
    //   1 4 1 0
    //   0 1 4 1
    //   1 0 1 4
    // Right-hand side (6, 6, 6, 6) has the constant solution (1, 1, 1, 1)
    // because each row sums to 6.
    std::vector<double> aSubDiagonal{ 0.0, 1.0, 1.0, 1.0 };
    std::vector<double> aDiagonal{ 4.0, 4.0, 4.0, 4.0 };
    std::vector<double> aSuperDiagonal{ 1.0, 1.0, 1.0, 0.0 };
    std::vector<double> aRhs{ 6.0, 6.0, 6.0, 6.0 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    CyclicTridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, 1.0, 1.0, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    for (double fX : aRhs)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, fX, 1e-10);
}

CPPUNIT_TEST_FIXTURE(CyclicTridiagonalSolverTest, testAsymmetricCyclic)
{
    // Cross-check against a hand-solved 4 by 4 system. The matrix has
    // diagonal 4 and unit off-diagonals plus corners, so it is the same
    // shape as the periodic cubic spline matrix. Right-hand side picked
    // so the solution is (1, -1, 1, -1).
    // Row 0: 4(1) + 1(-1) + 0 + 1(-1) = 4 - 1 - 1 = 2
    // Row 1: 1(1) + 4(-1) + 1(1)  + 0 = 1 - 4 + 1 = -2
    // Row 2: 0 + 1(-1) + 4(1) + 1(-1) = -1 + 4 - 1 = 2
    // Row 3: 1(1) + 0 + 1(1) + 4(-1) = 1 + 1 - 4 = -2
    std::vector<double> aSubDiagonal{ 0.0, 1.0, 1.0, 1.0 };
    std::vector<double> aDiagonal{ 4.0, 4.0, 4.0, 4.0 };
    std::vector<double> aSuperDiagonal{ 1.0, 1.0, 1.0, 0.0 };
    std::vector<double> aRhs{ 2.0, -2.0, 2.0, -2.0 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    CyclicTridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, 1.0, 1.0, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aRhs[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhs[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aRhs[2], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhs[3], 1e-10);
}

CPPUNIT_TEST_FIXTURE(CyclicTridiagonalSolverTest, testCyclicSharedFactorisation)
{
    // The cyclic system is built once. Both right-hand sides must come
    // out with their own solutions in a single solve call.
    std::vector<double> aSubDiagonal{ 0.0, 1.0, 1.0, 1.0 };
    std::vector<double> aDiagonal{ 4.0, 4.0, 4.0, 4.0 };
    std::vector<double> aSuperDiagonal{ 1.0, 1.0, 1.0, 0.0 };
    std::vector<double> aRhsA{ 6.0, 6.0, 6.0, 6.0 };
    std::vector<double> aRhsB{ 2.0, -2.0, 2.0, -2.0 };
    std::vector<std::span<double>> aRhsList{ aRhsA, aRhsB };

    CyclicTridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, 1.0, 1.0, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    for (double fX : aRhsA)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, fX, 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aRhsB[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhsB[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aRhsB[2], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhsB[3], 1e-10);
}

CPPUNIT_TEST_FIXTURE(CyclicTridiagonalSolverTest, testZeroFirstDiagonalReturnsFalse)
{
    // The Sherman-Morrison shift chooses gamma = -diag[0]. A zero entry
    // there leaves no nonzero gamma to pick, so the solver bails out.
    std::vector<double> aSubDiagonal{ 0.0, 1.0, 1.0 };
    std::vector<double> aDiagonal{ 0.0, 4.0, 4.0 };
    std::vector<double> aSuperDiagonal{ 1.0, 1.0, 0.0 };
    std::vector<double> aRhs{ 1.0, 1.0, 1.0 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    CyclicTridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, 1.0, 1.0, aRhsList);
    CPPUNIT_ASSERT(!aSolver.solve());
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
