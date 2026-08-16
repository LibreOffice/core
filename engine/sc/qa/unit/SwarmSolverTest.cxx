/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <test/unoapi_test.hxx>

#include <ParticleSwarmOptimization.hxx>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>

using namespace css;

namespace
{
// Minimal data provider that drives the algorithm template without a document.
class MockProvider
{
    size_t mnDimensionality;

public:
    int mnInitCalls = 0;

    explicit MockProvider(size_t nDimensionality)
        : mnDimensionality(nDimensionality)
    {
    }

    size_t getDimensionality() const { return mnDimensionality; }

    void initializeVariables(std::vector<double>& rVariables, std::mt19937& rGenerator)
    {
        std::uniform_real_distribution<double> aRandom(-10.0, 10.0);
        rVariables.resize(mnDimensionality);
        for (double& rValue : rVariables)
            rValue = aRandom(rGenerator);
        mnInitCalls++;
    }

    // Separable, with its maximum where every variable is zero, so the swarm
    // always has somewhere to move towards and improvements actually happen.
    static double calculateFitness(std::vector<double> const& rVariables)
    {
        double fFitness = 0.0;
        for (double fValue : rVariables)
            fFitness -= fValue * fValue;
        return fFitness;
    }

    static double clampVariable(size_t, double fValue) { return fValue; }
};

// Seeds the tests solve with. One is drawn per run, so a run stays short while
// the pool gets covered as runs accumulate.
constexpr sal_Int32 constTestSeeds[]
    = { 1, 42, 137, 1009, 4242, 12345, 65537, 271828, 999983, 7654321 };

// The seed this run solves with, drawn once and printed. SC_SOLVER_TEST_SEED
// forces one, to repeat a run.
sal_Int32 testSeed()
{
    static const sal_Int32 nSeed = [] {
        const char* pForced = std::getenv("SC_SOLVER_TEST_SEED");
        const sal_Int32 nPicked
            = pForced ? sal_Int32(std::atoi(pForced))
                      : constTestSeeds[std::random_device()() % std::size(constTestSeeds)];
        std::cerr << "swarm solver tests: random seed " << nPicked << std::endl;
        return nPicked;
    }();

    return nSeed;
}

// The seed as a line of text, so a failure says which run to repeat.
std::string seedMessage() { return "random seed " + std::to_string(testSeed()); }

class SwarmSolverTest : public UnoApiTest
{
    void testUnconstrained();
    void testVariableBounded();
    void testResultValueIsTheObjective();
    void testVariableConstrained();
    void testTwoVariables();
    void testMultipleVariables();
    void testInfeasibleConstraints();
    void testLargeObjectiveStillSolvable();
    void testParticleSwarmResultLength();
    void testParticleSwarmVelocityNotInitializedAsPosition();
    void testSameSeedRepeatsTheRun();
    void testSameSeedRepeatsTheSolve();
    void testUnreadableConstraintStillChecksOthers();
    void testContradictoryBoundsTerminate();
    void testUnboundedIntegerVariable();
    void testRepeatedSolveResetsState();
    void testControllersUnlockedAfterError();
    void testConstrainedLinearProgram();

    // A solver with this run's seed set.
    uno::Reference<sheet::XSolver> createSolver()
    {
        uno::Reference<sheet::XSolver> xSolver(
            m_xContext->getServiceManager()->createInstanceWithContext(
                u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
            uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
        xPropSet->setPropertyValue(u"RandomSeed"_ustr, cpo::uno::Any(testSeed()));

        return xSolver;
    }

public:
    SwarmSolverTest()
        : UnoApiTest(u"sc/qa/unit/data/solver"_ustr)
    {
    }

    CPPUNIT_TEST_SUITE(SwarmSolverTest);
    CPPUNIT_TEST(testUnconstrained);
    CPPUNIT_TEST(testVariableBounded);
    CPPUNIT_TEST(testResultValueIsTheObjective);
    CPPUNIT_TEST(testVariableConstrained);
    CPPUNIT_TEST(testMultipleVariables);
    CPPUNIT_TEST(testTwoVariables);
    CPPUNIT_TEST(testInfeasibleConstraints);
    CPPUNIT_TEST(testLargeObjectiveStillSolvable);
    CPPUNIT_TEST(testParticleSwarmResultLength);
    CPPUNIT_TEST(testParticleSwarmVelocityNotInitializedAsPosition);
    CPPUNIT_TEST(testSameSeedRepeatsTheRun);
    CPPUNIT_TEST(testSameSeedRepeatsTheSolve);
    CPPUNIT_TEST(testUnreadableConstraintStillChecksOthers);
    CPPUNIT_TEST(testContradictoryBoundsTerminate);
    CPPUNIT_TEST(testUnboundedIntegerVariable);
    CPPUNIT_TEST(testRepeatedSolveResetsState);
    CPPUNIT_TEST(testControllersUnlockedAfterError);
    CPPUNIT_TEST(testConstrainedLinearProgram);
    CPPUNIT_TEST_SUITE_END();
};

void SwarmSolverTest::testUnconstrained()
{
    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // constraints
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints;

    // initialize solver
    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    // test results
    xSolver->solve();
    CPPUNIT_ASSERT_MESSAGE(seedMessage(), xSolver->getSuccess());
    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    // The search reaches the optimum even with nothing constraining it.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), 3.0, aSolution[0], 1E-4);
}

void SwarmSolverTest::testVariableBounded()
{
    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // constraints
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* [0] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(100.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ cpo::uno::Any(-100.0) }
    };

    // initialize solver
    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    // test results
    xSolver->solve();
    CPPUNIT_ASSERT_MESSAGE(seedMessage(), xSolver->getSuccess());
    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), 3.0, aSolution[0], 1E-5);
}

void SwarmSolverTest::testResultValueIsTheObjective()
{
    // A solved model reports the objective its solution reaches. The objective
    // B2 is 10*B1^2 - 60*B1 - 40, whose lowest value is -130 at B1 = 3.
    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* [0] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(100.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ cpo::uno::Any(-100.0) }
    };

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    xSolver->solve();

    CPPUNIT_ASSERT_MESSAGE(seedMessage(), xSolver->getSuccess());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), -130.0, xSolver->getResultValue(), 1E-4);
}

void SwarmSolverTest::testVariableConstrained()
{
    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // constraints
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* [0] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ cpo::uno::Any(-50000.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(0.0) },
        { /* [2] Left     */ table::CellAddress(0, 1, 1),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ cpo::uno::Any(10.0) }
    };

    // initialize solver
    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    // test results
    xSolver->solve();
    CPPUNIT_ASSERT_MESSAGE(seedMessage(), xSolver->getSuccess());
    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    // The hybrid default reaches the optimum to within a stochastic tolerance.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), -0.741657, aSolution[0], 1E-4);
}

void SwarmSolverTest::testTwoVariables()
{
    loadFromFile(u"TwoVariables.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 5);

    // "changing cells" - unknown variables
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 2 }, { 0, 1, 3 } };

    // constraints
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* [0] Left     */ table::CellAddress(0, 1, 2),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ cpo::uno::Any(-100.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 3),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ cpo::uno::Any(-100.0) },
        { /* [2] Left     */ table::CellAddress(0, 1, 2),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(100.0) },
        { /* [3] Left     */ table::CellAddress(0, 1, 3),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(100.0) }
    };

    // initialize solver
    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(true);

    // test results
    xSolver->solve();
    CPPUNIT_ASSERT_MESSAGE(seedMessage(), xSolver->getSuccess());
    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    // FIXME increased the delta to cope with failures seen on jenkins
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), 0.666667, aSolution[0], 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), -1.666667, aSolution[1], 1E-4);
}

void SwarmSolverTest::testMultipleVariables()
{
    loadFromFile(u"MultiVariable.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(u"Integer"_ustr, cpo::uno::Any(true));

    table::CellAddress aObjective(0, 5, 7);

    // "changing cells" - unknown variables
    cpo::uno::Sequence<table::CellAddress> aVariables{
        { 0, 6, 1 }, { 0, 6, 2 }, { 0, 6, 3 }, { 0, 6, 4 }
    };

    // constraints
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* [ 0] Left     */ table::CellAddress(0, 1, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(table::CellAddress(0, 1, 6)) },
        { /* [ 1] Left     */ table::CellAddress(0, 2, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(table::CellAddress(0, 2, 6)) },
        { /* [ 2] Left     */ table::CellAddress(0, 3, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(table::CellAddress(0, 3, 6)) },
        { /* [ 3] Left     */ table::CellAddress(0, 4, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(table::CellAddress(0, 4, 6)) },
        { /* [ 4] Left     */ table::CellAddress(0, 6, 1),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(0.0) },
        { /* [ 5] Left     */ table::CellAddress(0, 6, 2),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(0.0) },
        { /* [ 6] Left     */ table::CellAddress(0, 6, 3),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(0.0) },
        { /* [ 7] Left     */ table::CellAddress(0, 6, 4),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ cpo::uno::Any(0.0) },
        { /* [ 8] Left     */ table::CellAddress(0, 6, 1),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ cpo::uno::Any(10000.0) },
        { /* [ 9] Left     */ table::CellAddress(0, 6, 2),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ cpo::uno::Any(10000.0) },
        { /* [10] Left     */ table::CellAddress(0, 6, 3),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ cpo::uno::Any(10000.0) },
        { /* [11] Left     */ table::CellAddress(0, 6, 4),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ cpo::uno::Any(10000.0) }
    };

    // initialize solver
    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    // test results
    xSolver->solve();
    CPPUNIT_ASSERT_MESSAGE(seedMessage(), xSolver->getSuccess());
    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
#if 0
    // Disabled: some seeds reach the point below, others stop at 0, 5, 0, 0.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aSolution[0], 1E-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aSolution[1], 1E-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aSolution[2], 1E-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aSolution[3], 1E-5);
#endif
}

void SwarmSolverTest::testInfeasibleConstraints()
{
    // Regression: the solver used to report success even when no point satisfies
    // the model.

    // Run both algorithms: Differential Evolution returns no candidate at all,
    // Particle Swarm Optimization returns its starting point. Either way the
    // result is infeasible and getSuccess must be false.
    for (sal_Int32 nAlgorithm : { sal_Int32(0), sal_Int32(1) })
    {
        loadFromFile(u"Simple.ods");

        uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

        uno::Reference<sheet::XSolver> xSolver = createSolver();

        uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
        xPropSet->setPropertyValue(u"Algorithm"_ustr, cpo::uno::Any(nAlgorithm));

        table::CellAddress aObjective(0, 1, 1);
        cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

        cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
            { /* [0] Left     */ table::CellAddress(0, 1, 0),
              /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
              /*     Right    */ cpo::uno::Any(-100.0) },
            { /* [1] Left     */ table::CellAddress(0, 1, 0),
              /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
              /*     Right    */ cpo::uno::Any(100.0) },
            // The objective B2 is 10*B1^2 - 60*B1 - 40, whose lowest value is
            // -130 at B1 = 3, so forcing it below -1000 can never be met.
            { /* [2] Left     */ table::CellAddress(0, 1, 1),
              /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
              /*     Right    */ cpo::uno::Any(-1000.0) }
        };

        xSolver->setDocument(xDocument);
        xSolver->setObjective(aObjective);
        xSolver->setVariables(aVariables);
        xSolver->setConstraints(aConstraints);
        xSolver->setMaximize(false);

        xSolver->solve();

        CPPUNIT_ASSERT_MESSAGE("Infeasible model must not report success", !xSolver->getSuccess());
    }
}

void SwarmSolverTest::testLargeObjectiveStillSolvable()
{
    // Regression: a feasible objective below the float range used to lose to the
    // infeasible penalty, so the search gave up on a solvable model. The Particle
    // Swarm result also used to grow past the variable count, which the length
    // check below guards.

    for (sal_Int32 nAlgorithm : { sal_Int32(0), sal_Int32(1) })
    {
        loadFromFile(u"Simple.ods");

        uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

        uno::Reference<sheet::XSolver> xSolver = createSolver();

        uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
        xPropSet->setPropertyValue(u"Algorithm"_ustr, cpo::uno::Any(nAlgorithm));

        table::CellAddress aObjective(0, 1, 1);
        cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

        // The variable is pushed far enough out that the objective
        // 10*B1^2 - 60*B1 - 40 reaches the 1e39 range, beyond what a float can
        // hold. Minimizing turns that into a fitness near -1e39. The objective
        // cap is a non-bounded constraint, so the upper part of the box is
        // infeasible while the lower end stays feasible.
        cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
            { /* [0] Left     */ table::CellAddress(0, 1, 0),
              /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
              /*     Right    */ cpo::uno::Any(1.0e19) },
            { /* [1] Left     */ table::CellAddress(0, 1, 0),
              /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
              /*     Right    */ cpo::uno::Any(2.0e19) },
            { /* [2] Left     */ table::CellAddress(0, 1, 1),
              /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
              /*     Right    */ cpo::uno::Any(2.0e39) }
        };

        xSolver->setDocument(xDocument);
        xSolver->setObjective(aObjective);
        xSolver->setVariables(aVariables);
        xSolver->setConstraints(aConstraints);
        xSolver->setMaximize(false);

        xSolver->solve();

        CPPUNIT_ASSERT_MESSAGE("Solvable model must report success, " + seedMessage(),
                               xSolver->getSuccess());

        cpo::uno::Sequence<double> aSolution = xSolver->getSolution();
        CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
        // the returned point must lie in the feasible part of the box
        CPPUNIT_ASSERT(aSolution[0] >= 1.0e19);
        CPPUNIT_ASSERT(aSolution[0] <= 2.0e19);
    }
}

void SwarmSolverTest::testParticleSwarmResultLength()
{
    // Regression: the best-position vectors were updated with insert at the front
    // instead of assign, so every improvement prepended another copy and the result
    // grew past the variable count.

    MockProvider aProvider(3);
    sc::ParticleSwarmOptimizationSolver<MockProvider> aAlgorithm(aProvider, 8, testSeed());

    aAlgorithm.initialize();
    for (int i = 0; i < 20; ++i)
        aAlgorithm.next();

    CPPUNIT_ASSERT_EQUAL(size_t(3), aAlgorithm.getResult().size());
}

void SwarmSolverTest::testParticleSwarmVelocityNotInitializedAsPosition()
{
    // Regression: the velocity was seeded by initializeVariables, which fills it
    // with a random position. Setup must call initializeVariables once per particle
    // (for the position only), not twice.

    MockProvider aProvider(3);
    sc::ParticleSwarmOptimizationSolver<MockProvider> aAlgorithm(aProvider, 8, testSeed());

    aAlgorithm.initialize();

    CPPUNIT_ASSERT_EQUAL(8, aProvider.mnInitCalls);
}

void SwarmSolverTest::testSameSeedRepeatsTheRun()
{
    // The search is stochastic, but a seed repeats the whole random sequence, so
    // two runs over the same model land on the same result.

    MockProvider aProviderOne(3);
    sc::ParticleSwarmOptimizationSolver<MockProvider> aFirst(aProviderOne, 8, testSeed());
    aFirst.initialize();
    for (int i = 0; i < 20; ++i)
        aFirst.next();

    MockProvider aProviderTwo(3);
    sc::ParticleSwarmOptimizationSolver<MockProvider> aSecond(aProviderTwo, 8, testSeed());
    aSecond.initialize();
    for (int i = 0; i < 20; ++i)
        aSecond.next();

    std::vector<double> const& rFirstResult = aFirst.getResult();
    std::vector<double> const& rSecondResult = aSecond.getResult();

    CPPUNIT_ASSERT_EQUAL(rFirstResult.size(), rSecondResult.size());
    for (size_t i = 0; i < rFirstResult.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(rFirstResult[i], rSecondResult[i]);
}

void SwarmSolverTest::testSameSeedRepeatsTheSolve()
{
    // With a seed set through the properties, the whole solve repeats: the same
    // model solved twice from the same document comes out at the same values.

    std::vector<cpo::uno::Sequence<double>> aSolutions;

    for (int nRun = 0; nRun < 2; ++nRun)
    {
        // A solve leaves its result in the variable cell, so each run reloads
        // the file and both start from the same values.
        loadFromFile(u"Simple.ods");

        uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

        uno::Reference<sheet::XSolver> xSolver = createSolver();

        table::CellAddress aObjective(0, 1, 1);
        cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

        cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
            { /* [0] Left     */ table::CellAddress(0, 1, 0),
              /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
              /*     Right    */ cpo::uno::Any(100.0) },
            { /* [1] Left     */ table::CellAddress(0, 1, 0),
              /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
              /*     Right    */ cpo::uno::Any(-100.0) }
        };

        xSolver->setDocument(xDocument);
        xSolver->setObjective(aObjective);
        xSolver->setVariables(aVariables);
        xSolver->setConstraints(aConstraints);
        xSolver->setMaximize(false);

        xSolver->solve();

        CPPUNIT_ASSERT(xSolver->getSuccess());
        aSolutions.push_back(xSolver->getSolution());
    }

    CPPUNIT_ASSERT_EQUAL(aSolutions[0].getLength(), aSolutions[1].getLength());
    for (sal_Int32 i = 0; i < aSolutions[0].getLength(); ++i)
        CPPUNIT_ASSERT_EQUAL(aSolutions[0][i], aSolutions[1][i]);
}

void SwarmSolverTest::testUnreadableConstraintStillChecksOthers()
{
    // Regression: a constraint whose right hand side could not be read as a cell
    // or a number used to make the feasibility check return "satisfied" for the
    // whole point, hiding every later constraint. Here an unreadable constraint
    // comes before an impossible one, so the model must still be reported as not
    // solved.
    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // Both constraints act on the objective cell, not the variable, so they end
    // up as non-bounded constraints in the order given here.
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        // right hand side is a string, so it reads as neither a cell nor a
        // number
        { /* [0] Left     */ table::CellAddress(0, 1, 1),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(u"not a number"_ustr) },
        // forced below the objective's global minimum of -130, so impossible
        { /* [1] Left     */ table::CellAddress(0, 1, 1),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(-1000.0) }
    };

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    xSolver->solve();

    CPPUNIT_ASSERT_MESSAGE("an unreadable constraint must not hide a later one",
                           !xSolver->getSuccess());
}

void SwarmSolverTest::testContradictoryBoundsTerminate()
{
    // Regression: a variable bounded below 5 and above 10 has an empty, reversed
    // range. The reflection in boundVariable used to keep wrapping such a value
    // without ever landing inside the range, so solve never returned. It must now
    // finish and report the model as not solved.

    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // Both act on the variable, so they fold into its bounds: lower 10, upper 5
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* [0] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ cpo::uno::Any(10.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ cpo::uno::Any(5.0) }
    };

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    // The guarantee here is that solve returns at all.
    xSolver->solve();

    CPPUNIT_ASSERT_MESSAGE("A model with contradictory bounds is not solvable",
                           !xSolver->getSuccess());
}

void SwarmSolverTest::testUnboundedIntegerVariable()
{
    // Regression: an integer variable with no explicit bound kept the default
    // range around the float limits. Turning those limits into a 64 bit integer
    // collapsed both ends to the same value, so the whole population started at one
    // point, never varied, and the search returned nothing. The variable must
    // instead be seeded across a usable range and the model reported as solved.

    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(u"Integer"_ustr, cpo::uno::Any(true));

    table::CellAddress aObjective(0, 1, 1);
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // No constraints, so the variable keeps the default unbounded range.
    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints;

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    xSolver->solve();

    CPPUNIT_ASSERT_MESSAGE("An unbounded integer variable must still be solvable",
                           xSolver->getSuccess());

    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();
    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT(std::isfinite(aSolution[0]));
    CPPUNIT_ASSERT_EQUAL(aSolution[0], std::trunc(aSolution[0]));
}

void SwarmSolverTest::testRepeatedSolveResetsState()
{
    // Regression: solving twice with the same solver kept state from the first
    // run. The collected constraints only ever grew and the bounds were not reset,
    // so the second solve saw a leftover impossible constraint and stale bounds and
    // failed on a model it should solve.

    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setMaximize(false);

    // First run: a bound on the variable plus an impossible constraint on the
    // objective, so it cannot be solved.
    xSolver->setConstraints({ { /* Left */ table::CellAddress(0, 1, 0),
                                /* Op   */ sheet::SolverConstraintOperator_LESS_EQUAL,
                                /* Right*/ cpo::uno::Any(0.0) },
                              { /* Left */ table::CellAddress(0, 1, 1),
                                /* Op   */ sheet::SolverConstraintOperator_LESS_EQUAL,
                                /* Right*/ cpo::uno::Any(-1000.0) } });
    xSolver->solve();
    CPPUNIT_ASSERT(!xSolver->getSuccess());

    // Second run on the same solver: a plain bounded model with minimum at 3.
    // It is bounded only by its own constraints.
    xSolver->setConstraints({ { /* Left */ table::CellAddress(0, 1, 0),
                                /* Op   */ sheet::SolverConstraintOperator_GREATER_EQUAL,
                                /* Right*/ cpo::uno::Any(-100.0) },
                              { /* Left */ table::CellAddress(0, 1, 0),
                                /* Op   */ sheet::SolverConstraintOperator_LESS_EQUAL,
                                /* Right*/ cpo::uno::Any(100.0) } });
    xSolver->solve();
    CPPUNIT_ASSERT_MESSAGE("The second solve must not inherit the first run's state",
                           xSolver->getSuccess());

    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();
    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aSolution[0], 1E-4);
}

void SwarmSolverTest::testControllersUnlockedAfterError()
{
    // Regression: solve locked the document controllers and only unlocked them at
    // the end. A cell access in between could throw, leaving the document locked
    // for good. Point a variable at a sheet that does not exist so a cell access
    // throws, then check the controllers are unlocked again.

    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<frame::XModel> xModel(xDocument, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    table::CellAddress aObjective(0, 1, 1);
    // sheet index 99 does not exist, so reading or writing this cell throws
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 99, 1, 0 } };

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints({});
    xSolver->setMaximize(false);

    bool bThrew = false;
    try
    {
        xSolver->solve();
    }
    catch (const cpo::uno::Exception&)
    {
        bThrew = true;
    }

    CPPUNIT_ASSERT(bThrew);
    CPPUNIT_ASSERT_MESSAGE("Controllers must be unlocked after a failed solve",
                           !xModel->hasControllersLocked());
}

void SwarmSolverTest::testConstrainedLinearProgram()
{
    // A small linear program with two binding inequality constraints on formula
    // cells. Maximize 2x + 4y subject to x + y <= 4 and x + 3y <= 6, with both
    // variables in [0, 10]. The two constraints meet at x = 3, y = 1, where the
    // objective reaches its maximum of 10.

    loadFromFile(u"ConstrainedLinear.fods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver = createSolver();

    // objective 2x + 4y is in B3
    table::CellAddress aObjective(0, 1, 2);
    // x is A1, y is A2
    cpo::uno::Sequence<table::CellAddress> aVariables{ { 0, 0, 0 }, { 0, 0, 1 } };

    cpo::uno::Sequence<sheet::SolverConstraint> aConstraints{
        // box bounds on the two variables
        { table::CellAddress(0, 0, 0), sheet::SolverConstraintOperator_GREATER_EQUAL,
          cpo::uno::Any(0.0) },
        { table::CellAddress(0, 0, 0), sheet::SolverConstraintOperator_LESS_EQUAL,
          cpo::uno::Any(10.0) },
        { table::CellAddress(0, 0, 1), sheet::SolverConstraintOperator_GREATER_EQUAL,
          cpo::uno::Any(0.0) },
        { table::CellAddress(0, 0, 1), sheet::SolverConstraintOperator_LESS_EQUAL,
          cpo::uno::Any(10.0) },
        // x + y <= 4 (cell B1)
        { table::CellAddress(0, 1, 0), sheet::SolverConstraintOperator_LESS_EQUAL,
          cpo::uno::Any(4.0) },
        // x + 3y <= 6 (cell B2)
        { table::CellAddress(0, 1, 1), sheet::SolverConstraintOperator_LESS_EQUAL,
          cpo::uno::Any(6.0) }
    };

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(true);

    xSolver->solve();

    CPPUNIT_ASSERT_MESSAGE(seedMessage(), xSolver->getSuccess());

    cpo::uno::Sequence<double> aSolution = xSolver->getSolution();
    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), 3.0, aSolution[0], 1E-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(seedMessage(), 1.0, aSolution[1], 1E-3);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwarmSolverTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
