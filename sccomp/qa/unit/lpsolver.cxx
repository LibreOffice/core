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
#include <test/bootstrapfixture.hxx>

#include <address.hxx>

using namespace css;

namespace {

class LpSolverTest: public test::BootstrapFixture
{
    uno::Reference<sheet::XSpreadsheetDocument> m_xDocument;

    void testLpSolver();
    void testCoinMPSolver();

    void testSolver(OUString const & rName);

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(LpSolverTest);
    CPPUNIT_TEST(testLpSolver);
    CPPUNIT_TEST(testCoinMPSolver);
    CPPUNIT_TEST_SUITE_END();
};

void LpSolverTest::setUp()
{
    test::BootstrapFixture::setUp();
    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(m_xContext);
    uno::Reference<lang::XComponent> xComponent(xComponentLoader->loadComponentFromURL(
            "private:factory/scalc", "_blank", 0,
            uno::Sequence < css::beans::PropertyValue >()));
    m_xDocument.set(xComponent, uno::UNO_QUERY_THROW);
}

void LpSolverTest::tearDown()
{
    uno::Reference<lang::XComponent>(m_xDocument, uno::UNO_QUERY_THROW)->dispose();
    test::BootstrapFixture::tearDown();
}

void LpSolverTest::testLpSolver()
{
#ifdef ENABLE_LPSOLVE
    testSolver("com.sun.star.comp.Calc.LpsolveSolver");
#endif
}

void LpSolverTest::testCoinMPSolver()
{
#ifdef ENABLE_COINMP
    testSolver("com.sun.star.comp.Calc.CoinMPSolver");
#endif
}

void LpSolverTest::testSolver(OUString const & rName)
{
    uno::Reference<sheet::XSolver> xSolver(m_xContext->getServiceManager()->
            createInstanceWithContext(rName, m_xContext), uno::UNO_QUERY_THROW);

    table::CellAddress aObjective(0, 0, 0);

    // "changing cells" - unknown variables
    uno::Sequence<table::CellAddress> aVariables(1);
    aVariables[0] = table::CellAddress(0, 0, 0);

    // constraints
    uno::Sequence<sheet::SolverConstraint> aConstraints(1);
    aConstraints[0].Left = table::CellAddress(0, 0, 0);
    aConstraints[0].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    aConstraints[0].Right <<= 5.0;

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
    CPPUNIT_ASSERT_EQUAL(aSolution[0], 5.0);

    uno::Reference<sheet::XSolverDescription> xDesc(xSolver, uno::UNO_QUERY_THROW);
    const OString sMessage("Empty description for " + OUStringToOString(rName, RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT_MESSAGE(sMessage.getStr(), !xDesc->getComponentDescription().isEmpty());
}

CPPUNIT_TEST_SUITE_REGISTRATION(LpSolverTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
