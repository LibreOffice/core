/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>

#include <chart2uno.hxx>
#include <drwlayer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>

#include "helper/qahelper.hxx"

using namespace ::com::sun::star;
using namespace ::cpo;
using namespace ::cpo::uno;

class ScChart2DataProviderTest : public ScModelTestBase
{
public:
    ScChart2DataProviderTest();

    void testHeaderExpansion();
    void testTdf66209();
    void testTdf66209DeleteRow();
    void testTdf66209InsertColumn();
    void testTdf66209Grouped();

    CPPUNIT_TEST_SUITE(ScChart2DataProviderTest);
    CPPUNIT_TEST(testHeaderExpansion);
    CPPUNIT_TEST(testTdf66209);
    CPPUNIT_TEST(testTdf66209DeleteRow);
    CPPUNIT_TEST(testTdf66209InsertColumn);
    CPPUNIT_TEST(testTdf66209Grouped);
    CPPUNIT_TEST_SUITE_END();

private:
    void checkTdf66209Ranges(SCCOL nColumnOffset, SCROW nRowOffset);
};

static void lcl_createAndCheckDataProvider(ScDocument& rDoc, const OUString& cellRange,
                                           bool hasCategories, bool firstCellAsLabel,
                                           sal_Int32 expectedRows, sal_Int32 expectedCols)
{
    uno::Reference<chart2::data::XDataProvider> xDataProvider = new ScChart2DataProvider(&rDoc);
    CPPUNIT_ASSERT(xDataProvider.is());

    cpo::uno::Sequence<beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"CellRangeRepresentation"_ustr, cellRange),
        comphelper::makePropertyValue(u"HasCategories"_ustr, hasCategories),
        comphelper::makePropertyValue(u"FirstCellAsLabel"_ustr, firstCellAsLabel),
        comphelper::makePropertyValue(u"DataRowSource"_ustr, chart::ChartDataRowSource_COLUMNS)
    };

    uno::Reference<chart2::data::XDataSource> xDataSource = xDataProvider->createDataSource(aArgs);
    CPPUNIT_ASSERT(xDataSource.is());

    cpo::uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>> xSequences
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

void ScChart2DataProviderTest::testTdf66209()
{
    createScDoc("ods/tdf66209.ods");
    ScDocument* pDoc = getScDoc();

    // Do not load Sheet2's chart before changing the source sheet. The ODS
    // omits draw:notify-on-update-of-ranges and only Sheet1 has been viewed.
    CPPUNIT_ASSERT(pDoc->InsertRow(ScRange(0, 0, 0, pDoc->MaxCol(), 0, 0)));
    checkTdf66209Ranges(0, 1);
}

void ScChart2DataProviderTest::testTdf66209DeleteRow()
{
    createScDoc("ods/tdf66209.ods");
    ScDocument* pDoc = getScDoc();
    pDoc->DeleteRow(ScRange(0, 0, 0, pDoc->MaxCol(), 0, 0));
    checkTdf66209Ranges(0, -1);
}

void ScChart2DataProviderTest::testTdf66209InsertColumn()
{
    createScDoc("ods/tdf66209.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc->InsertCol(ScRange(0, 0, 0, 0, pDoc->MaxRow(), 0)));
    checkTdf66209Ranges(1, 0);
}

void ScChart2DataProviderTest::checkTdf66209Ranges(SCCOL nColumnOffset, SCROW nRowOffset)
{
    ScDocument* pDoc = getScDoc();
    SdrPage* pPage = pDoc->GetDrawLayer()->GetPage(1);
    SdrObject* pObject = pPage->GetObj(0);
    if (pObject->IsGroupObject())
        pObject = pObject->GetSubList()->GetObj(0);
    auto* pChart = dynamic_cast<SdrOle2Obj*>(pObject);
    CPPUNIT_ASSERT(pChart);
    ScRangeList aRanges = getChartRanges(*pDoc, *pChart);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aRanges.size());
    CPPUNIT_ASSERT_EQUAL(
        ScRange(nColumnOffset, 2 + nRowOffset, 0, nColumnOffset, 7 + nRowOffset, 0), aRanges[0]);
    CPPUNIT_ASSERT_EQUAL(
        ScRange(1 + nColumnOffset, 1 + nRowOffset, 0, 1 + nColumnOffset, 1 + nRowOffset, 0),
        aRanges[1]);
    CPPUNIT_ASSERT_EQUAL(
        ScRange(1 + nColumnOffset, 2 + nRowOffset, 0, 1 + nColumnOffset, 7 + nRowOffset, 0),
        aRanges[2]);
}

void ScChart2DataProviderTest::testTdf66209Grouped()
{
    createScDoc("ods/tdf66209-grouped.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc->InsertRow(ScRange(0, 0, 0, pDoc->MaxCol(), 0, 0)));
    checkTdf66209Ranges(0, 1);
}

ScChart2DataProviderTest::ScChart2DataProviderTest()
    : ScModelTestBase(u"sc/qa/unit/data"_ustr)
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScChart2DataProviderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
