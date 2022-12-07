/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"

#include <comphelper/propertyvalue.hxx>
#include <vcl/scheduler.hxx>

using namespace ::com::sun::star;

class Chart2UiChartTest : public ChartTest
{
public:
    Chart2UiChartTest()
        : ChartTest("/chart2/qa/extras/data/")
    {
    }

    void testCopyPasteToNewSheet(uno::Reference<chart::XChartDocument> xChartDoc,
                                 OUString aObjectName, sal_Int32 nColumns, sal_Int32 nRows);
};

void Chart2UiChartTest::testCopyPasteToNewSheet(uno::Reference<chart::XChartDocument> xChartDoc,
                                                OUString aObjectName, sal_Int32 nColumns,
                                                sal_Int32 nRows)
{
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart::XChartDataArray> xChartData(xChartDoc->getData(), uno::UNO_QUERY_THROW);

    uno::Sequence<OUString> aExpectedColumnDescriptions = xChartData->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in origin file", nColumns,
                                 aExpectedColumnDescriptions.getLength());

    uno::Sequence<OUString> aExpectedRowDescriptions = xChartData->getRowDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of rows in origin file", nRows,
                                 aExpectedRowDescriptions.getLength());

    Sequence<Sequence<double>> aExpectedData = xChartData->getData();

    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToObject", aObjectName),
    };
    dispatchCommand(mxComponent, ".uno:GoToObject", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    // create a new document
    load("private:factory/scalc");

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    uno::Reference<chart2::XChartDocument> xChartDoc2 = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc2.is());

    uno::Reference<chart::XChartDataArray> xDataArray(xChartDoc2->getDataProvider(),
                                                      UNO_QUERY_THROW);

    Sequence<OUString> aColumnDesc = xDataArray->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in destination file", nColumns,
                                 aColumnDesc.getLength());
    for (sal_Int32 i = 0; i < nColumns; ++i)
    {
        OString sMessage("Incorrect description in column: " + OString::number(i));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aExpectedColumnDescriptions[i],
                                     aColumnDesc[i]);
    }

    Sequence<OUString> aRowDesc = xDataArray->getRowDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of rows in destination file", nRows,
                                 aRowDesc.getLength());
    for (sal_Int32 i = 0; i < nRows; ++i)
    {
        OString sMessage("Incorrect description in row: " + OString::number(i));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aExpectedRowDescriptions[i], aRowDesc[i]);
    }

    Sequence<Sequence<double>> aData = xDataArray->getData();

    for (sal_Int32 nRowIdx = 0; nRowIdx < nRows; ++nRowIdx)
    {
        for (sal_Int32 nColIdx = 0; nColIdx < nColumns; ++nColIdx)
        {
            double nValue = aData[nRowIdx][nColIdx];
            double nExpected = aExpectedData[nRowIdx][nColIdx];
            OString sMessage("Incorrect value in Col: " + OString::number(nColIdx)
                             + " Row: " + OString::number(nRowIdx));

            if (std::isnan(nValue))
            {
                // On paste, 0 becomes NaN, check whether it's expected
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), 0.0, nExpected, 1e-1);
            }
            else
            {
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), nExpected, nValue, 1e-1);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf120348)
{
    loadFromURL(u"ods/tdf120348.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 3.33625955201419
    // - Incorrect value in Col: 2 Row: 51
    testCopyPasteToNewSheet(xChartDoc, "Object 2", 4, 158);
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf151091)
{
    std::vector<OUString> aExpected
        = { u"Ωφέλιμο", u"Επικίνδυνο", u"Απόσταση", u"Μάσκα", u"Εμβόλιο" };

    loadFromURL(u"ods/tdf151091.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart::XChartDataArray> xChartData(xChartDoc->getData(), uno::UNO_QUERY_THROW);
    uno::Sequence<OUString> aSeriesList = xChartData->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aSeriesList.getLength());

    for (size_t i = 0; i < 5; ++i)
        CPPUNIT_ASSERT_EQUAL(aExpected[i], aSeriesList[i]);

    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToObject", OUString("Object 1")),
    };
    dispatchCommand(mxComponent, ".uno:GoToObject", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    // create a new writer document
    load("private:factory/swriter");

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    aSeriesList = getWriterChartColumnDescriptions(mxComponent);

    // Without the fix in place, this test would have failed with
    // - Expected: 5
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aSeriesList.getLength());

    for (size_t i = 0; i < 5; ++i)
        CPPUNIT_ASSERT_EQUAL(aExpected[i], aSeriesList[i]);
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf107097)
{
    loadFromURL(u"ods/tdf107097.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getPivotChartDocFromSheet(1, mxComponent),
                                                    uno::UNO_QUERY_THROW);
    testCopyPasteToNewSheet(xChartDoc, "Object 1", 4, 12);
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf136011)
{
    loadFromURL(u"ods/tdf136011.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);
    testCopyPasteToNewSheet(xChartDoc, "Object 1", 3, 9);

    loadFromURL(u"ods/tdf136011.ods");
    uno::Reference<chart::XChartDocument> xChartDoc2(getChartCompFromSheet(0, 1, mxComponent),
                                                     uno::UNO_QUERY_THROW);

    // Without the fix in place, this test would have failed with
    // - Expected: Test 1 1
    // - Actual  : Test 1
    // - Incorrect description in row: 0
    testCopyPasteToNewSheet(xChartDoc2, "Object 2", 3, 9);
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf62057)
{
    loadFromURL(u"ods/tdf62057.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 7
    // - Incorrect number of columns in destination file
    testCopyPasteToNewSheet(xChartDoc, "Object 1", 2, 6);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
