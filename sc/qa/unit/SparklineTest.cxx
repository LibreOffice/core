/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include <docsh.hxx>
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

using namespace css;

class SparklineTest : public ScBootstrapFixture
{
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;

public:
    SparklineTest()
        : ScBootstrapFixture("sc/qa/unit/data")
    {
    }

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
        // which is a private symbol to us, gets called
        m_xCalcComponent = getMultiServiceFactory()->createInstance(
            "com.sun.star.comp.Calc.SpreadsheetDocument");
        CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
    }

    virtual void tearDown() override
    {
        uno::Reference<lang::XComponent>(m_xCalcComponent, uno::UNO_QUERY_THROW)->dispose();
        test::BootstrapFixture::tearDown();
    }

    void testAddSparkline();

    CPPUNIT_TEST_SUITE(SparklineTest);
    CPPUNIT_TEST(testAddSparkline);
    CPPUNIT_TEST_SUITE_END();
};

void SparklineTest::testAddSparkline()
{
    ScDocShellRef xDocSh = loadEmptyDocument();
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDocument = xDocSh->GetDocument();
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();

    sc::Sparkline* pSparkline = rDocument.CreateSparkline(ScAddress(0, 0, 0), pSparklineGroup);
    CPPUNIT_ASSERT(pSparkline);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SparklineTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
