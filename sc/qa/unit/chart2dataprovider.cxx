/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>

#include <chart2uno.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>

#include "helper/qahelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScChart2DataProviderTest : public ScModelTestBase
{
public:
    ScChart2DataProviderTest();

    void testHeaderExpansion();

    CPPUNIT_TEST_SUITE(ScChart2DataProviderTest);
    CPPUNIT_TEST(testHeaderExpansion);
    CPPUNIT_TEST_SUITE_END();
};

static void lcl_createAndCheckDataProvider(ScDocument& rDoc, const OUString& cellRange,
                                           bool hasCategories, bool firstCellAsLabel,
                                           sal_Int32 expectedRows, sal_Int32 expectedCols)
{
    uno::Reference<chart2::data::XDataProvider> xDataProvider = new ScChart2DataProvider(&rDoc);
    CPPUNIT_ASSERT(xDataProvider.is());

    uno::Sequence<beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"CellRangeRepresentation"_ustr, cellRange),
        comphelper::makePropertyValue(u"HasCategories"_ustr, hasCategories),
        comphelper::makePropertyValue(u"FirstCellAsLabel"_ustr, firstCellAsLabel),
        comphelper::makePropertyValue(u"DataRowSource"_ustr, chart::ChartDataRowSource_COLUMNS)
    };

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
    createScDoc("ods/chart2dataprovider.ods");

    ScDocument* pDoc = getScDoc();

    lcl_createAndCheckDataProvider(*pDoc, u"$Sheet1.$A$1:$D$4"_ustr, false, false, 4, 4);
    lcl_createAndCheckDataProvider(*pDoc, u"$Sheet1.$A$1:$D$4"_ustr, true, true, 4, 3);

    lcl_createAndCheckDataProvider(*pDoc, u"$Sheet1.$A$17:$D$20"_ustr, true, true, 3, 2);

    lcl_createAndCheckDataProvider(*pDoc, u"$Sheet1.$A$25:$D$28"_ustr, true, true, 4, 2);
}

ScChart2DataProviderTest::ScChart2DataProviderTest()
    : ScModelTestBase(u"sc/qa/unit/data"_ustr)
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScChart2DataProviderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
