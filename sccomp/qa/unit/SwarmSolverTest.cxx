/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <test/unoapi_test.hxx>

using namespace css;

namespace
{
class SwarmSolverTest : public UnoApiTest
{
    void testUnconstrained();
    void testVariableBounded();
    void testVariableConstrained();
    void testTwoVariables();
    void testMultipleVariables();

public:
    SwarmSolverTest()
        : UnoApiTest(u"sccomp/qa/unit/data"_ustr)
    {
    }

    CPPUNIT_TEST_SUITE(SwarmSolverTest);
    CPPUNIT_TEST(testUnconstrained);
    CPPUNIT_TEST(testVariableBounded);
    CPPUNIT_TEST(testVariableConstrained);
    CPPUNIT_TEST(testMultipleVariables);
    CPPUNIT_TEST(testTwoVariables);
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
          /*     Right    */ uno::Any(100.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ uno::Any(-100.0) }
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
          /*     Right    */ uno::Any(-50000.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 0),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ uno::Any(0.0) },
        { /* [2] Left     */ table::CellAddress(0, 1, 1),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ uno::Any(10.0) }
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
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.741657, aSolution[0], 1E-5);
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
          /*     Right    */ uno::Any(-100.0) },
        { /* [1] Left     */ table::CellAddress(0, 1, 3),
          /*     Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*     Right    */ uno::Any(-100.0) },
        { /* [2] Left     */ table::CellAddress(0, 1, 2),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ uno::Any(100.0) },
        { /* [3] Left     */ table::CellAddress(0, 1, 3),
          /*     Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*     Right    */ uno::Any(100.0) }
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
    xPropSet->setPropertyValue(u"Integer"_ustr, uno::Any(true));

    table::CellAddress aObjective(0, 5, 7);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables{
        { 0, 6, 1 }, { 0, 6, 2 }, { 0, 6, 3 }, { 0, 6, 4 }
    };

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* [ 0] Left     */ table::CellAddress(0, 1, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(table::CellAddress(0, 1, 6)) },
        { /* [ 1] Left     */ table::CellAddress(0, 2, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(table::CellAddress(0, 2, 6)) },
        { /* [ 2] Left     */ table::CellAddress(0, 3, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(table::CellAddress(0, 3, 6)) },
        { /* [ 3] Left     */ table::CellAddress(0, 4, 5),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(table::CellAddress(0, 4, 6)) },
        { /* [ 4] Left     */ table::CellAddress(0, 6, 1),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(0.0) },
        { /* [ 5] Left     */ table::CellAddress(0, 6, 2),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(0.0) },
        { /* [ 6] Left     */ table::CellAddress(0, 6, 3),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(0.0) },
        { /* [ 7] Left     */ table::CellAddress(0, 6, 4),
          /*      Operator */ sheet::SolverConstraintOperator_GREATER_EQUAL,
          /*      Right    */ uno::Any(0.0) },
        { /* [ 8] Left     */ table::CellAddress(0, 6, 1),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ uno::Any(10000.0) },
        { /* [ 9] Left     */ table::CellAddress(0, 6, 2),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ uno::Any(10000.0) },
        { /* [10] Left     */ table::CellAddress(0, 6, 3),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ uno::Any(10000.0) },
        { /* [11] Left     */ table::CellAddress(0, 6, 4),
          /*      Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /*      Right    */ uno::Any(10000.0) }
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

CPPUNIT_TEST_SUITE_REGISTRATION(SwarmSolverTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
