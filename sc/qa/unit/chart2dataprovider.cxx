/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <docsh.hxx>
#include <chart2uno.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>

#include "helper/qahelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScChart2DataProviderTest : public ScBootstrapFixture
{
public:
    ScChart2DataProviderTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testHeaderExpansion();

    CPPUNIT_TEST_SUITE(ScChart2DataProviderTest);
    CPPUNIT_TEST(testHeaderExpansion);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

static void lcl_createAndCheckDataProvider(ScDocument& rDoc, const OUString& cellRange,
                                           bool hasCategories, bool firstCellAsLabel,
                                           sal_Int32 expectedRows, sal_Int32 expectedCols)
{
    uno::Reference<chart2::data::XDataProvider> xDataProvider = new ScChart2DataProvider(&rDoc);
    CPPUNIT_ASSERT(xDataProvider.is());

    uno::Sequence<beans::PropertyValue> aArgs(4);

    aArgs[0].Name = "CellRangeRepresentation";
    aArgs[0].Value <<= cellRange;

    aArgs[1].Name = "HasCategories";
    aArgs[1].Value <<= hasCategories;

    aArgs[2].Name = "FirstCellAsLabel";
    aArgs[2].Value <<= firstCellAsLabel;

    aArgs[3].Name = "DataRowSource";
    aArgs[3].Value <<= chart::ChartDataRowSource_COLUMNS;

    uno::Reference<chart2::data::XDataSource> xDataSource = xDataProvider->createDataSource(aArgs);
    CPPUNIT_ASSERT(xDataSource.is());

    css::uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>> xSequences
        = xDataSource->getDataSequences();

    CPPUNIT_ASSERT_EQUAL(expectedRows, xSequences.getLength());

    sal_Int32 nStartRow = hasCategories ? 1 : 0;
    for (sal_Int32 nIdx = nStartRow; nIdx < xSequences.getLength(); ++nIdx)
    {
        Reference<chart2::data::XDataSequence> xValues(xSequences[nIdx]->getValues());
        if (xValues.is())
        {
            sal_Int32 colsNum = xValues->getData().getLength();
            CPPUNIT_ASSERT_EQUAL(expectedCols, colsNum);
        }
    }
}

void ScChart2DataProviderTest::testHeaderExpansion()
{
    ScDocShellRef xDocSh = loadDoc(u"chart2dataprovider.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load ch.ods.", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    lcl_createAndCheckDataProvider(rDoc, "$Sheet1.$A$1:$D$4", false, false, 4, 4);
    lcl_createAndCheckDataProvider(rDoc, "$Sheet1.$A$1:$D$4", true, true, 4, 3);

    lcl_createAndCheckDataProvider(rDoc, "$Sheet1.$A$17:$D$20", true, true, 3, 2);

    lcl_createAndCheckDataProvider(rDoc, "$Sheet1.$A$25:$D$28", true, true, 4, 2);

    xDocSh->DoClose();
}

ScChart2DataProviderTest::ScChart2DataProviderTest()
    : ScBootstrapFixture("sc/qa/unit/data")
{
}

void ScChart2DataProviderTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScChart2DataProviderTest::tearDown()
{
    uno::Reference<lang::XComponent>(m_xCalcComponent, UNO_QUERY_THROW)->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScChart2DataProviderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
