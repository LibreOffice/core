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

    void test();
    void testSolver(const uno::Reference<sheet::XSolver>& xSolver);

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(LpSolverTest);
    CPPUNIT_TEST(test);
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

void LpSolverTest::test()
{
    uno::Reference<container::XContentEnumerationAccess> xEnAc(
            m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> xEnum = xEnAc->
        createContentEnumeration( "com.sun.star.sheet.Solver" );
    CPPUNIT_ASSERT(xEnum.is());

    sal_Int32 nCount = 0;
    while (xEnum->hasMoreElements())
    {
        uno::Reference<uno::XInterface> xIntFac;
        xEnum->nextElement() >>= xIntFac;
        CPPUNIT_ASSERT(xIntFac.is());
        uno::Reference<lang::XServiceInfo> xInfo(xIntFac, uno::UNO_QUERY_THROW);
        const OUString sName(xInfo->getImplementationName());
        uno::Reference<sheet::XSolver> xSolver(m_xContext->getServiceManager()->
                createInstanceWithContext(sName, m_xContext), uno::UNO_QUERY_THROW);
        testSolver(xSolver);

        uno::Reference<sheet::XSolverDescription> xDesc(xSolver, uno::UNO_QUERY_THROW);
        const OString sMessage("Empty description for " +
                OUStringToOString(sName, RTL_TEXTENCODING_UTF8));
        CPPUNIT_ASSERT_MESSAGE(sMessage.getStr(), !xDesc->getComponentDescription().isEmpty());
        ++nCount;
    }
    sal_Int32 nExpected = 0;
#ifdef ENABLE_COINMP
    ++nExpected;
#endif
#ifdef ENABLE_LPSOLVE
    ++nExpected;
#endif
    CPPUNIT_ASSERT_EQUAL(nExpected, nCount);
}

void LpSolverTest::testSolver(const uno::Reference<sheet::XSolver>& xSolver)
{
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
    CPPUNIT_ASSERT_EQUAL(aSolution[0], (double)5.0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(LpSolverTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
