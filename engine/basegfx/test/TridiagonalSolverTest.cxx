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

#include <basegfx/numeric/TridiagonalSolver.hxx>

#include <span>
#include <vector>

namespace basegfx
{
class TridiagonalSolverTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(TridiagonalSolverTest, testIdentityLeavesRhsUnchanged)
{
    // A matrix with 1s on the diagonal and zeros elsewhere leaves the
    // right-hand side unchanged.
    std::vector<double> aSubDiagonal{ 0.0, 0.0, 0.0 };
    std::vector<double> aDiagonal{ 1.0, 1.0, 1.0 };
    std::vector<double> aSuperDiagonal{ 0.0, 0.0, 0.0 };
    std::vector<double> aRhs{ 7.0, -3.5, 11.25 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    TridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, aRhs[0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.5, aRhs[1], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.25, aRhs[2], 1e-12);
}

CPPUNIT_TEST_FIXTURE(TridiagonalSolverTest, testKnownSymmetricSystem)
{
    // The system
    //   2 x0 +   x1        = 1
    //     x0 + 2 x1 +   x2 = 4
    //            x1 + 2 x2 = 1
    // has the unique solution (-1, 3, -1). Sub-diagonal entry 0 and
    // super-diagonal entry 2 are unused.
    std::vector<double> aSubDiagonal{ 0.0, 1.0, 1.0 };
    std::vector<double> aDiagonal{ 2.0, 2.0, 2.0 };
    std::vector<double> aSuperDiagonal{ 1.0, 1.0, 0.0 };
    std::vector<double> aRhs{ 1.0, 4.0, 1.0 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    TridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhs[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aRhs[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhs[2], 1e-10);
}

CPPUNIT_TEST_FIXTURE(TridiagonalSolverTest, testTridiagonalSharedFactorisation)
{
    // Solve the same tridiagonal system for two independent right-hand
    // sides at once. Each one must come out with its own solution
    // because they share only the matrix, not the values.
    std::vector<double> aSubDiagonal{ 0.0, 1.0, 1.0 };
    std::vector<double> aDiagonal{ 2.0, 2.0, 2.0 };
    std::vector<double> aSuperDiagonal{ 1.0, 1.0, 0.0 };
    std::vector<double> aRhsA{ 1.0, 4.0, 1.0 };
    std::vector<double> aRhsB{ 4.0, 8.0, 4.0 };
    std::vector<std::span<double>> aRhsList{ aRhsA, aRhsB };

    TridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    // First right-hand side solution: same as testKnownSymmetricSystem above.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhsA[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aRhsA[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhsA[2], 1e-10);
    // Second right-hand side: solve 2 a + b = 4, a + 2 b + c = 8, b + 2 c = 4.
    // By symmetry a = c. Eliminating gives a = 0, b = 4, c = 0.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aRhsB[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, aRhsB[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aRhsB[2], 1e-10);
}

CPPUNIT_TEST_FIXTURE(TridiagonalSolverTest, testZeroPivotReturnsFalse)
{
    // A zero on the diagonal stops the algorithm. Thomas does not
    // pivot.
    std::vector<double> aSubDiagonal{ 0.0, 1.0 };
    std::vector<double> aDiagonal{ 0.0, 1.0 };
    std::vector<double> aSuperDiagonal{ 1.0, 0.0 };
    std::vector<double> aRhs{ 1.0, 2.0 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    TridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, aRhsList);
    CPPUNIT_ASSERT(!aSolver.solve());
}

CPPUNIT_TEST_FIXTURE(TridiagonalSolverTest, testEmptySystemSucceeds)
{
    // An empty system has nothing to solve, so it succeeds instead of
    // being reported like a zero-pivot failure.
    std::vector<std::span<double>> aRhsList;

    TridiagonalSolver aSolver({}, {}, {}, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
}

CPPUNIT_TEST_FIXTURE(TridiagonalSolverTest, testSingleEquation)
{
    // A one-row matrix is just a scalar divide.
    std::vector<double> aSubDiagonal{ 0.0 };
    std::vector<double> aDiagonal{ 5.0 };
    std::vector<double> aSuperDiagonal{ 0.0 };
    std::vector<double> aRhs{ 15.0 };
    std::vector<std::span<double>> aRhsList{ aRhs };

    TridiagonalSolver aSolver(aSubDiagonal, aDiagonal, aSuperDiagonal, aRhsList);
    CPPUNIT_ASSERT(aSolver.solve());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aRhs[0], 1e-12);
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
