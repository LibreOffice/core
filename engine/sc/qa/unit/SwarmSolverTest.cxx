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

#include <ParticelSwarmOptimization.hxx>

#include <cmath>

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

class SwarmSolverTest : public UnoApiTest
{
    void testUnconstrained();
    void testVariableBounded();
    void testVariableConstrained();
    void testTwoVariables();
    void testMultipleVariables();
    void testInfeasibleConstraints();
    void testLargeObjectiveStillSolvable();
    void testParticleSwarmResultLength();
    void testParticleSwarmVelocityNotInitializedAsPosition();
    void testUnreadableConstraintStillChecksOthers();
    void testContradictoryBoundsTerminate();
    void testUnboundedIntegerVariable();
    void testRepeatedSolveResetsState();
    void testControllersUnlockedAfterError();
    void testConstrainedLinearProgram();

public:
    SwarmSolverTest()
        : UnoApiTest(u"sc/qa/unit/data/solver"_ustr)
    {
    }

    CPPUNIT_TEST_SUITE(SwarmSolverTest);
    CPPUNIT_TEST(testUnconstrained);
    CPPUNIT_TEST(testVariableBounded);
    CPPUNIT_TEST(testVariableConstrained);
    CPPUNIT_TEST(testMultipleVariables);
    CPPUNIT_TEST(testTwoVariables);
    CPPUNIT_TEST(testInfeasibleConstraints);
    CPPUNIT_TEST(testLargeObjectiveStillSolvable);
    CPPUNIT_TEST(testParticleSwarmResultLength);
    CPPUNIT_TEST(testParticleSwarmVelocityNotInitializedAsPosition);
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

    uno::Reference<sheet::XSolver> xSolver;

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints;

    // initialize solver
    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    // test results
    xSolver->solve();
    CPPUNIT_ASSERT(xSolver->getSuccess());
    uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    // It happens that the unconstrained test does not find a solution in the
    // timeframe or number of generations it has available as the search space is
    // too big and the values might not converge to solution. So for now just run
    // the test so we know for sure the algorithm is guaranteed to finish
    // and doesn't cause any seg faults.
    //CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aSolution[0], .9);
}

void SwarmSolverTest::testVariableBounded()
{
    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver;

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints{
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
    CPPUNIT_ASSERT(xSolver->getSuccess());
    uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aSolution[0], 1E-5);
}

void SwarmSolverTest::testVariableConstrained()
{
    loadFromFile(u"Simple.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver;

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints{
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
    CPPUNIT_ASSERT(xSolver->getSuccess());
    uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    // The hybrid default reaches the optimum to within a stochastic tolerance.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.741657, aSolution[0], 1E-4);
}

void SwarmSolverTest::testTwoVariables()
{
    loadFromFile(u"TwoVariables.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver;

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 5);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 2 }, { 0, 1, 3 } };

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints{
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
    CPPUNIT_ASSERT(xSolver->getSuccess());
    uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.666667, aSolution[0], 1E-5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.666667, aSolution[1], 1E-5);
}

void SwarmSolverTest::testMultipleVariables()
{
    loadFromFile(u"MultiVariable.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSolver> xSolver;

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(u"Integer"_ustr, cpo::uno::Any(true));

    table::CellAddress aObjective(0, 5, 7);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables{
        { 0, 6, 1 }, { 0, 6, 2 }, { 0, 6, 3 }, { 0, 6, 4 }
    };

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints{
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
    CPPUNIT_ASSERT(xSolver->getSuccess());
    uno::Sequence<double> aSolution = xSolver->getSolution();

    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
#if 0
    // Disable for now, needs algorithm stability improvements
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

        uno::Reference<sheet::XSolver> xSolver;
        xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                        u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                    uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
        xPropSet->setPropertyValue(u"Algorithm"_ustr, cpo::uno::Any(nAlgorithm));

        table::CellAddress aObjective(0, 1, 1);
        uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

        uno::Sequence<sheet::SolverConstraint> aConstraints{
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

        uno::Reference<sheet::XSolver> xSolver;
        xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                        u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                    uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
        xPropSet->setPropertyValue(u"Algorithm"_ustr, cpo::uno::Any(nAlgorithm));

        table::CellAddress aObjective(0, 1, 1);
        uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

        // The variable is pushed far enough out that the objective
        // 10*B1^2 - 60*B1 - 40 reaches the 1e39 range, beyond what a float can
        // hold. Minimizing turns that into a fitness near -1e39. The objective
        // cap is a non-bounded constraint, so the upper part of the box is
        // infeasible while the lower end stays feasible.
        uno::Sequence<sheet::SolverConstraint> aConstraints{
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

        CPPUNIT_ASSERT_MESSAGE("Solvable model must report success", xSolver->getSuccess());

        uno::Sequence<double> aSolution = xSolver->getSolution();
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
    ParticleSwarmOptimizationSolver<MockProvider> aAlgorithm(aProvider, 8);

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
    ParticleSwarmOptimizationSolver<MockProvider> aAlgorithm(aProvider, 8);

    aAlgorithm.initialize();

    CPPUNIT_ASSERT_EQUAL(8, aProvider.mnInitCalls);
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

    uno::Reference<sheet::XSolver> xSolver;
    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // Both constraints act on the objective cell, not the variable, so they end
    // up as non-bounded constraints in the order given here.
    uno::Sequence<sheet::SolverConstraint> aConstraints{
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

    uno::Reference<sheet::XSolver> xSolver;
    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // Both act on the variable, so they fold into its bounds: lower 10, upper 5
    uno::Sequence<sheet::SolverConstraint> aConstraints{
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

    uno::Reference<sheet::XSolver> xSolver;
    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue(u"Integer"_ustr, cpo::uno::Any(true));

    table::CellAddress aObjective(0, 1, 1);
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

    // No constraints, so the variable keeps the default unbounded range.
    uno::Sequence<sheet::SolverConstraint> aConstraints;

    xSolver->setDocument(xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    xSolver->solve();

    CPPUNIT_ASSERT_MESSAGE("An unbounded integer variable must still be solvable",
                           xSolver->getSuccess());

    uno::Sequence<double> aSolution = xSolver->getSolution();
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

    uno::Reference<sheet::XSolver> xSolver;
    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);
    uno::Sequence<table::CellAddress> aVariables{ { 0, 1, 0 } };

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

    uno::Sequence<double> aSolution = xSolver->getSolution();
    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aSolution[0], 1E-5);
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

    uno::Reference<sheet::XSolver> xSolver;
    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);
    // sheet index 99 does not exist, so reading or writing this cell throws
    uno::Sequence<table::CellAddress> aVariables{ { 99, 1, 0 } };

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
    catch (const uno::Exception&)
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

    uno::Reference<sheet::XSolver> xSolver;
    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.Calc.SwarmSolver"_ustr, m_xContext),
                uno::UNO_QUERY_THROW);

    // objective 2x + 4y is in B3
    table::CellAddress aObjective(0, 1, 2);
    // x is A1, y is A2
    uno::Sequence<table::CellAddress> aVariables{ { 0, 0, 0 }, { 0, 0, 1 } };

    uno::Sequence<sheet::SolverConstraint> aConstraints{
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

    CPPUNIT_ASSERT(xSolver->getSuccess());

    uno::Sequence<double> aSolution = xSolver->getSolution();
    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, aSolution[0], 1E-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, aSolution[1], 1E-3);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwarmSolverTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
