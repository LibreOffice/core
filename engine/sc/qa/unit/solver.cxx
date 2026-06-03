/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/sheet/XSolver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <test/bootstrapfixture.hxx>

using namespace css;

namespace {

class LpSolverTest: public test::BootstrapFixture
{
    uno::Reference<sheet::XSpreadsheetDocument> m_xDocument;

#ifdef ENABLE_COINMP
    void testCoinMPSolver();
    void testSLPSolver();
    void testSLPSolverNonlinear();
#endif

#ifdef ENABLE_COINMP
    void testSolver(OUString const & rName);
#endif

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(LpSolverTest);
#ifdef ENABLE_COINMP
    CPPUNIT_TEST(testCoinMPSolver);
    CPPUNIT_TEST(testSLPSolver);
    CPPUNIT_TEST(testSLPSolverNonlinear);
#endif
    CPPUNIT_TEST_SUITE_END();
};

void LpSolverTest::setUp()
{
    test::BootstrapFixture::setUp();
    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(m_xContext);
    uno::Reference<lang::XComponent> xComponent(xComponentLoader->loadComponentFromURL(
            u"private:factory/scalc"_ustr, u"_blank"_ustr, 0,
            uno::Sequence < css::beans::PropertyValue >()));
    m_xDocument.set(xComponent, uno::UNO_QUERY_THROW);
}

void LpSolverTest::tearDown()
{
    uno::Reference<lang::XComponent>(m_xDocument, uno::UNO_QUERY_THROW)->dispose();
    m_xDocument.clear();
    test::BootstrapFixture::tearDown();
}

#ifdef ENABLE_COINMP
void LpSolverTest::testCoinMPSolver()
{
    testSolver(u"com.sun.star.comp.Calc.CoinMPSolver"_ustr);
}

// The sequential linear solver must also handle a plain linear model.
void LpSolverTest::testSLPSolver()
{
    testSolver(u"com.sun.star.comp.Calc.SLPSolver"_ustr);
}

// A genuinely nonlinear model: minimize the parabola A1*A1 - 4*A1 + 7, which
// has its minimum at A1 = 2 with value 3. The objective lives in a formula
// cell, so the solver only sees it as a black box and must converge to 2 by
// repeated linearization.
void LpSolverTest::testSLPSolverNonlinear()
{
    uno::Reference<container::XIndexAccess> xSheets(m_xDocument->getSheets(),
                                                    uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xSheets->getByIndex(0), uno::UNO_QUERY_THROW);
    // B1 holds the objective as a formula of the variable A1
    xSheet->getCellByPosition(1, 0)->setFormula(u"=A1*A1-4*A1+7"_ustr);

    uno::Reference<sheet::XSolver> xSolver(
        m_xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.comp.Calc.SLPSolver"_ustr, m_xContext),
        uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 1, 0); // B1
    uno::Sequence<table::CellAddress> aVariables{ { 0, 0, 0 } }; // A1
    uno::Sequence<sheet::SolverConstraint> aConstraints{
        { table::CellAddress(0, 0, 0), sheet::SolverConstraintOperator_GREATER_EQUAL,
          uno::Any(-10.0) },
        { table::CellAddress(0, 0, 0), sheet::SolverConstraintOperator_LESS_EQUAL,
          uno::Any(10.0) }
    };

    xSolver->setDocument(m_xDocument);
    xSolver->setObjective(aObjective);
    xSolver->setVariables(aVariables);
    xSolver->setConstraints(aConstraints);
    xSolver->setMaximize(false);

    xSolver->solve();
    CPPUNIT_ASSERT(xSolver->getSuccess());
    uno::Sequence<double> aSolution = xSolver->getSolution();
    CPPUNIT_ASSERT_EQUAL(aVariables.getLength(), aSolution.getLength());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, aSolution[0], 1E-4);
}
#endif

#ifdef ENABLE_COINMP
void LpSolverTest::testSolver(OUString const & rName)
{
    uno::Reference<sheet::XSolver> xSolver(m_xContext->getServiceManager()->
            createInstanceWithContext(rName, m_xContext), uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 0, 0);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables { {0, 0, 0 } };

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints{
        { /* Left     */ table::CellAddress(0, 0, 0),
          /* Operator */ sheet::SolverConstraintOperator_LESS_EQUAL,
          /* Right    */ uno::Any(5.0) }
    };

    // initialize solver
    xSolver->setDocument( m_xDocument );
    xSolver->setObjective( aObjective );
    xSolver->setVariables( aVariables );
    xSolver->setConstraints( aConstraints );
    xSolver->setMaximize( true );

    // test results
    xSolver->solve();
    CPPUNIT_ASSERT(xSolver->getSuccess());
    uno::Sequence<double> aSolution = xSolver->getSolution();
    CPPUNIT_ASSERT_EQUAL(aSolution.getLength(), aVariables.getLength());
    CPPUNIT_ASSERT_EQUAL(5.0, aSolution[0]);

    uno::Reference<sheet::XSolverDescription> xDesc(xSolver, uno::UNO_QUERY_THROW);
    const OString sMessage("Empty description for " + OUStringToOString(rName, RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT_MESSAGE(sMessage.getStr(), !xDesc->getComponentDescription().isEmpty());
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(LpSolverTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
