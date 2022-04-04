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
        auto pSparkline = rDocument.GetSparkline(ScAddress(0, 1, 0)); // A2
        CPPUNIT_ASSERT(pSparkline);
        CPPUNIT_ASSERT_EQUAL(OUString("{1C5C5DE0-3C09-4CB3-A3EC-9E763301EC82}"),
                             pSparkline->getSparklineGroup()->getID());

        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, rAttributes.getType());

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), rAttributes.getColorSeries());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b050), rAttributes.getColorNegative());
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), rAttributes.getColorAxis());
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), rAttributes.getColorMarkers());
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), rAttributes.getColorFirst());
        CPPUNIT_ASSERT_EQUAL(Color(0xff0000), rAttributes.getColorLast());
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), rAttributes.getColorHigh());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), rAttributes.getColorLow());

        CPPUNIT_ASSERT_EQUAL(1.0, rAttributes.getLineWeight());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isDateAxis());
        CPPUNIT_ASSERT_EQUAL(sc::DisplayEmptyCellsAs::Gap, rAttributes.getDisplayEmptyCellsAs());

        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isMarkers());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isHigh());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLow());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isFirst());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLast());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isNegative());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.shouldDisplayXAxis());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.shouldDisplayHidden());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isRightToLeft());

        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMax()));
        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMin()));
    }
    // Sparkline at Sheet1:A3
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(0, 2, 0)); // A3
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, rAttributes.getType());

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), rAttributes.getColorSeries());
        CPPUNIT_ASSERT_EQUAL(Color(0xff0000), rAttributes.getColorNegative());
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), rAttributes.getColorAxis());
        CPPUNIT_ASSERT_EQUAL(Color(0xd00000), rAttributes.getColorMarkers());
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), rAttributes.getColorFirst());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), rAttributes.getColorLast());
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), rAttributes.getColorHigh());
        CPPUNIT_ASSERT_EQUAL(Color(0xffc000), rAttributes.getColorLow());

        CPPUNIT_ASSERT_EQUAL(0.75, rAttributes.getLineWeight());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isDateAxis());
        CPPUNIT_ASSERT_EQUAL(sc::DisplayEmptyCellsAs::Gap, rAttributes.getDisplayEmptyCellsAs());

        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isMarkers());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isHigh());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLow());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isFirst());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLast());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isNegative());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.shouldDisplayXAxis());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.shouldDisplayHidden());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isRightToLeft());

        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMax()));
        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMin()));
    }
    // Sparkline at Sheet2:B1
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 0, 1)); //B1
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, rAttributes.getType());
    }
    // Sparkline at Sheet2:B2
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 1, 1)); //B2
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, rAttributes.getType());
    }
    // Sparkline at Sheet2:B2
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 1, 1)); //B2
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, rAttributes.getType());
    }
    // Sparkline doesn't exists at A4
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(0, 3, 0)); //A4
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
