/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <com/sun/star/lang/XComponent.hpp>
#include <docsh.hxx>
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

using namespace css;

class SparklineImportExportTest : public ScBootstrapFixture
{
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;

public:
    SparklineImportExportTest()
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

    void testSparklines();

    CPPUNIT_TEST_SUITE(SparklineImportExportTest);
    CPPUNIT_TEST(testSparklines);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{
void checkSparklines(ScDocument& rDocument)
{
    // Sparkline at Sheet1:A2
    {
        sc::Sparkline* pSparkline = rDocument.GetSparkline(ScAddress(0, 1, 0)); // A2
        CPPUNIT_ASSERT(pSparkline);
        auto pSparklineGroup = pSparkline->getSparklineGroup();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, pSparklineGroup->m_eType);

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), pSparklineGroup->m_aColorSeries);
        CPPUNIT_ASSERT_EQUAL(Color(0x00b050), pSparklineGroup->m_aColorNegative);
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), pSparklineGroup->m_aColorAxis);
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), pSparklineGroup->m_aColorMarkers);
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), pSparklineGroup->m_aColorFirst);
        CPPUNIT_ASSERT_EQUAL(Color(0xff0000), pSparklineGroup->m_aColorLast);
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), pSparklineGroup->m_aColorHigh);
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), pSparklineGroup->m_aColorLow);

        CPPUNIT_ASSERT_EQUAL(1.0, pSparklineGroup->m_fLineWeight);
        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bDateAxis);
        CPPUNIT_ASSERT_EQUAL(sc::DisplayEmptyCellAs::Gap, pSparklineGroup->m_eDisplayEmptyCellsAs);

        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bMarkers);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bHigh);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bLow);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bFirst);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bLast);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bNegative);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bDisplayXAxis);
        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bDisplayHidden);
        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bRightToLeft);

        CPPUNIT_ASSERT_EQUAL(false, bool(pSparklineGroup->m_aManualMax));
        CPPUNIT_ASSERT_EQUAL(false, bool(pSparklineGroup->m_aManualMin));
    }
    // Sparkline at Sheet1:A3
    {
        sc::Sparkline* pSparkline = rDocument.GetSparkline(ScAddress(0, 2, 0)); // A3
        CPPUNIT_ASSERT(pSparkline);
        auto pSparklineGroup = pSparkline->getSparklineGroup();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, pSparklineGroup->m_eType);

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), pSparklineGroup->m_aColorSeries);
        CPPUNIT_ASSERT_EQUAL(Color(0xff0000), pSparklineGroup->m_aColorNegative);
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), pSparklineGroup->m_aColorAxis);
        CPPUNIT_ASSERT_EQUAL(Color(0xd00000), pSparklineGroup->m_aColorMarkers);
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), pSparklineGroup->m_aColorFirst);
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), pSparklineGroup->m_aColorLast);
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), pSparklineGroup->m_aColorHigh);
        CPPUNIT_ASSERT_EQUAL(Color(0xffc000), pSparklineGroup->m_aColorLow);

        CPPUNIT_ASSERT_EQUAL(0.75, pSparklineGroup->m_fLineWeight);
        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bDateAxis);
        CPPUNIT_ASSERT_EQUAL(sc::DisplayEmptyCellAs::Gap, pSparklineGroup->m_eDisplayEmptyCellsAs);

        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bMarkers);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bHigh);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bLow);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bFirst);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bLast);
        CPPUNIT_ASSERT_EQUAL(true, pSparklineGroup->m_bNegative);
        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bDisplayXAxis);
        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bDisplayHidden);
        CPPUNIT_ASSERT_EQUAL(false, pSparklineGroup->m_bRightToLeft);

        CPPUNIT_ASSERT_EQUAL(false, bool(pSparklineGroup->m_aManualMax));
        CPPUNIT_ASSERT_EQUAL(false, bool(pSparklineGroup->m_aManualMin));
    }
    // Sparkline at Sheet2:B1
    {
        sc::Sparkline* pSparkline = rDocument.GetSparkline(ScAddress(1, 0, 1)); //B1
        CPPUNIT_ASSERT(pSparkline);
        auto pSparklineGroup = pSparkline->getSparklineGroup();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, pSparklineGroup->m_eType);
    }
    // Sparkline at Sheet2:B2
    {
        sc::Sparkline* pSparkline = rDocument.GetSparkline(ScAddress(1, 1, 1)); //B2
        CPPUNIT_ASSERT(pSparkline);
        auto pSparklineGroup = pSparkline->getSparklineGroup();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, pSparklineGroup->m_eType);
    }
    // Sparkline at Sheet2:B2
    {
        sc::Sparkline* pSparkline = rDocument.GetSparkline(ScAddress(1, 1, 1)); //B2
        CPPUNIT_ASSERT(pSparkline);
        auto pSparklineGroup = pSparkline->getSparklineGroup();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, pSparklineGroup->m_eType);
    }
    // Sparkline doesn't exists at A4
    {
        sc::Sparkline* pSparkline = rDocument.GetSparkline(ScAddress(0, 3, 0)); //A4
        CPPUNIT_ASSERT(!pSparkline);
    }
}
} // end anonymous namespace

void SparklineImportExportTest::testSparklines()
{
    ScDocShellRef xDocSh = loadDoc(u"Sparklines.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh);

    checkSparklines(xDocSh->GetDocument());

    xDocSh = saveAndReload(*xDocSh, FORMAT_XLSX);

    checkSparklines(xDocSh->GetDocument());

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SparklineImportExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
