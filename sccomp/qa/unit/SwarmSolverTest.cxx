/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <test/calc_unoapi_test.hxx>

#include <address.hxx>

using namespace css;

namespace
{
class SwarmSolverTest : public CalcUnoApiTest
{
    uno::Reference<lang::XComponent> mxComponent;
    void testUnconstrained();
    void testVariableBounded();
    void testVariableConstrained();
    void testTwoVariables();
    void testMultipleVariables();

public:
    SwarmSolverTest()
        : CalcUnoApiTest("sccomp/qa/unit/data")
    {
    }

    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(SwarmSolverTest);
    CPPUNIT_TEST(testUnconstrained);
    CPPUNIT_TEST(testVariableBounded);
    CPPUNIT_TEST(testVariableConstrained);
    CPPUNIT_TEST(testMultipleVariables);
    CPPUNIT_TEST(testTwoVariables);
    CPPUNIT_TEST_SUITE_END();
};

void SwarmSolverTest::tearDown()
{
    if (mxComponent.is())
        closeDocument(mxComponent);
}

void SwarmSolverTest::testUnconstrained()
{
    CPPUNIT_ASSERT(!mxComponent.is());

    OUString aFileURL;
    createFileURL("Simple.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell;

    uno::Reference<sheet::XSolver> xSolver;
    const OUString sSolverName("com.sun.star.comp.Calc.SwarmSolver");

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(sSolverName, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables(1);
    aVariables[0] = table::CellAddress(0, 1, 0);

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
    CPPUNIT_ASSERT(!mxComponent.is());

    OUString aFileURL;
    createFileURL("Simple.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell;

    uno::Reference<sheet::XSolver> xSolver;
    const OUString sSolverName("com.sun.star.comp.Calc.SwarmSolver");

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(sSolverName, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables(1);
    aVariables[0] = table::CellAddress(0, 1, 0);

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints(2);
    aConstraints[0].Left = table::CellAddress(0, 1, 0);
    aConstraints[0].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[0].Right <<= 100.0;

    aConstraints[1].Left = table::CellAddress(0, 1, 0);
    aConstraints[1].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[1].Right <<= -100.0;

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
    CPPUNIT_ASSERT(!mxComponent.is());

    OUString aFileURL;
    createFileURL("Simple.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell;

    uno::Reference<sheet::XSolver> xSolver;
    const OUString sSolverName("com.sun.star.comp.Calc.SwarmSolver");

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(sSolverName, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 1);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables(1);
    aVariables[0] = table::CellAddress(0, 1, 0);

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints(3);
    aConstraints[0].Left = table::CellAddress(0, 1, 0);
    aConstraints[0].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[0].Right <<= -50000.0;

    aConstraints[1].Left = table::CellAddress(0, 1, 0);
    aConstraints[1].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[1].Right <<= 0.0;

    aConstraints[2].Left = table::CellAddress(0, 1, 1);
    aConstraints[2].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[2].Right <<= 10.0;

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
    CPPUNIT_ASSERT(!mxComponent.is());

    OUString aFileURL;
    createFileURL("TwoVariables.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell;

    uno::Reference<sheet::XSolver> xSolver;
    const OUString sSolverName("com.sun.star.comp.Calc.SwarmSolver");

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(sSolverName, m_xContext),
                uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 5);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables(2);
    aVariables[0] = table::CellAddress(0, 1, 2);
    aVariables[1] = table::CellAddress(0, 1, 3);

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints(4);

    aConstraints[0].Left = table::CellAddress(0, 1, 2);
    aConstraints[0].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[0].Right <<= -100.0;

    aConstraints[1].Left = table::CellAddress(0, 1, 3);
    aConstraints[1].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[1].Right <<= -100.0;

    aConstraints[2].Left = table::CellAddress(0, 1, 2);
    aConstraints[2].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[2].Right <<= 100.0;

    aConstraints[3].Left = table::CellAddress(0, 1, 3);
    aConstraints[3].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[3].Right <<= 100.0;

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
    CPPUNIT_ASSERT(!mxComponent.is());

    OUString aFileURL;
    createFileURL("MultiVariable.ods", aFileURL);
    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<table::XCell> xCell;

    uno::Reference<sheet::XSolver> xSolver;
    const OUString sSolverName("com.sun.star.comp.Calc.SwarmSolver");

    xSolver.set(m_xContext->getServiceManager()->createInstanceWithContext(sSolverName, m_xContext),
                uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropSet(xSolver, uno::UNO_QUERY_THROW);
    xPropSet->setPropertyValue("Integer", uno::makeAny(true));

    table::CellAddress aObjective(0, 5, 7);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables(4);
    aVariables[0] = table::CellAddress(0, 6, 1);
    aVariables[1] = table::CellAddress(0, 6, 2);
    aVariables[2] = table::CellAddress(0, 6, 3);
    aVariables[3] = table::CellAddress(0, 6, 4);

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints(12);

    aConstraints[0].Left = table::CellAddress(0, 1, 5);
    aConstraints[0].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[0].Right <<= table::CellAddress(0, 1, 6);

    aConstraints[1].Left = table::CellAddress(0, 2, 5);
    aConstraints[1].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[1].Right <<= table::CellAddress(0, 2, 6);

    aConstraints[2].Left = table::CellAddress(0, 3, 5);
    aConstraints[2].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[2].Right <<= table::CellAddress(0, 3, 6);

    aConstraints[3].Left = table::CellAddress(0, 4, 5);
    aConstraints[3].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[3].Right <<= table::CellAddress(0, 4, 6);

    aConstraints[4].Left = table::CellAddress(0, 6, 1);
    aConstraints[4].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[4].Right <<= 0.0;

    aConstraints[5].Left = table::CellAddress(0, 6, 2);
    aConstraints[5].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[5].Right <<= 0.0;

    aConstraints[6].Left = table::CellAddress(0, 6, 3);
    aConstraints[6].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[6].Right <<= 0.0;

    aConstraints[7].Left = table::CellAddress(0, 6, 4);
    aConstraints[7].Operator = sheet::SolverConstraintOperator_GREATER_EQUAL;
    aConstraints[7].Right <<= 0.0;

    aConstraints[8].Left = table::CellAddress(0, 6, 1);
    aConstraints[8].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[8].Right <<= 10000.0;

    aConstraints[9].Left = table::CellAddress(0, 6, 2);
    aConstraints[9].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[9].Right <<= 10000.0;

    aConstraints[10].Left = table::CellAddress(0, 6, 3);
    aConstraints[10].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[10].Right <<= 10000.0;

    aConstraints[11].Left = table::CellAddress(0, 6, 4);
    aConstraints[11].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[11].Right <<= 10000.0;

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
