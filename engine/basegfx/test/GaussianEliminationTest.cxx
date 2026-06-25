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

#include <basegfx/numeric/GaussianElimination.hxx>

#include <cstddef>
#include <span>
#include <vector>

namespace basegfx
{
class GaussianEliminationTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(GaussianEliminationTest, testIdentitySolveReturnsRhs)
{
    // An identity matrix leaves the right-hand side unchanged.
    std::vector<std::vector<double>> aRows{ { 1.0 }, { 1.0 }, { 1.0 } };
    std::vector<size_t> aOffsets{ 0, 1, 2 };
    std::vector<double> aRhs{ 7.0, -3.5, 11.25 };
    std::span<double> aRhsList[] = { aRhs };

    GaussianElimination aSolver(aRows, aOffsets, 0 /*bandwidth*/, aRhsList);
    CPPUNIT_ASSERT(aSolver.solveBanded());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, aRhs[0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.5, aRhs[1], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.25, aRhs[2], 1e-12);
}

CPPUNIT_TEST_FIXTURE(GaussianEliminationTest, testKnownTridiagonalSystem)
{
    // The system:
    //   2a +   b      = 1
    //    a +  2b +  c = 4
    //          b + 2c = 1
    // has the unique solution (-1, 3, -1). In the banded layout each
    // row's first stored cell is its first non-zero column, so row 2
    // starts at column 1 (offset 1, cells [1, 2, trailing 0]).
    std::vector<std::vector<double>> aRows{ { 2.0, 1.0, 0.0 },
                                            { 1.0, 2.0, 1.0 },
                                            { 1.0, 2.0, 0.0 } };
    std::vector<size_t> aOffsets{ 0, 0, 1 };
    std::vector<double> aRhs{ 1.0, 4.0, 1.0 };
    std::span<double> aRhsList[] = { aRhs };

    GaussianElimination aSolver(aRows, aOffsets, 2 /*bandwidth*/, aRhsList);
    CPPUNIT_ASSERT(aSolver.solveBanded());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhs[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aRhs[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhs[2], 1e-10);
}

CPPUNIT_TEST_FIXTURE(GaussianEliminationTest, testSingularMatrixReturnsFalse)
{
    // Two identical rows leave the matrix rank-deficient. The pivot
    // search cannot find a nonzero pivot for the second column.
    std::vector<std::vector<double>> aRows{ { 1.0, 2.0, 0.0 },
                                            { 1.0, 2.0, 0.0 },
                                            { 0.0, 0.0, 1.0 } };
    std::vector<size_t> aOffsets{ 0, 0, 0 };
    std::vector<double> aRhs{ 3.0, 3.0, 5.0 };
    std::span<double> aRhsList[] = { aRhs };

    GaussianElimination aSolver(aRows, aOffsets, 2, aRhsList);
    CPPUNIT_ASSERT(!aSolver.solveBanded());
}

CPPUNIT_TEST_FIXTURE(GaussianEliminationTest, testTwoRightHandSidesSharedFactorisation)
{
    // Solve the same tridiagonal system for two independent right-hand
    // sides at once. Each one must come out with its own solution
    // because they share only the matrix, not the values.
    std::vector<std::vector<double>> aRows{ { 2.0, 1.0, 0.0 },
                                            { 1.0, 2.0, 1.0 },
                                            { 1.0, 2.0, 0.0 } };
    std::vector<size_t> aOffsets{ 0, 0, 1 };
    std::vector<double> aRhsA{ 1.0, 4.0, 1.0 };
    std::vector<double> aRhsB{ 4.0, 8.0, 4.0 };
    std::span<double> aRhsList[] = { aRhsA, aRhsB };

    GaussianElimination aSolver(aRows, aOffsets, 2, aRhsList);
    CPPUNIT_ASSERT(aSolver.solveBanded());
    // First right-hand side solution from testKnownTridiagonalSystem.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhsA[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aRhsA[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aRhsA[2], 1e-10);
    // Second right-hand side:
    // 2a + b       = 4,
    //  a + 2b +  c = 8,
    //       b + 2c = 4.
    // By symmetry a = c. Eliminating gives a = 0, b = 4, c = 0.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aRhsB[0], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, aRhsB[1], 1e-10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aRhsB[2], 1e-10);
}

CPPUNIT_TEST_FIXTURE(GaussianEliminationTest, testPivotingHandlesZeroDiagonal)
{
    // The first row has zero at the diagonal entry, so without partial
    // pivoting the algorithm would divide by zero. The pivot search
    // must swap a later row into position zero and recover.
    //   0a + 1b = 1
    //   1a + 0b = 2
    // Expected: a = 2, b = 1.
    std::vector<std::vector<double>> aRows{ { 0.0, 1.0 }, { 1.0, 0.0 } };
    std::vector<size_t> aOffsets{ 0, 0 };
    std::vector<double> aRhs{ 1.0, 2.0 };
    std::span<double> aRhsList[] = { aRhs };

    GaussianElimination aSolver(aRows, aOffsets, 1, aRhsList);
    CPPUNIT_ASSERT(aSolver.solveBanded());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aRhs[0], 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aRhs[1], 1e-12);
}

} // namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
