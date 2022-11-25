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

#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace ::com::sun::star;

class Chart2UiChartTest : public ChartTest
{
public:
    Chart2UiChartTest()
        : ChartTest("/chart2/qa/extras/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf120348)
{
    loadFromURL(u"ods/tdf120348.ods");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDocument->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    std::vector<std::vector<double>> aExpected;

    for (sal_Int32 nRowIdx = 1; nRowIdx < 159; ++nRowIdx)
    {
        std::vector<double> aRow;
        for (sal_Int32 nColIdx = 5; nColIdx < 9; ++nColIdx)
        {
            uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(nColIdx, nRowIdx);
            aRow.push_back(xCell->getValue());
        }
        aExpected.push_back(aRow);
    }

    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToObject", OUString("Object 2")),
    };
    dispatchCommand(mxComponent, ".uno:GoToObject", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    // create a new document
    load("private:factory/scalc");

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart::XChartDataArray> xDataArray(xChartDoc->getDataProvider(),
                                                      UNO_QUERY_THROW);
    Sequence<OUString> aColumnDesc = xDataArray->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aColumnDesc.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("Finland"), aColumnDesc[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sweden"), aColumnDesc[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Poland"), aColumnDesc[2]);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aColumnDesc[3]);
    Sequence<Sequence<double>> aData = xDataArray->getData();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(158), aData.getLength());

    for (sal_Int32 nRowIdx = 0; nRowIdx < 158; ++nRowIdx)
    {
        for (sal_Int32 nColIdx = 0; nColIdx < 4; ++nColIdx)
        {
            double nValue = aData[nRowIdx][nColIdx];
            double nExpected = aExpected[nRowIdx][nColIdx];
            OString sMessage("Incorrect value in Col: " + OString::number(nColIdx)
                             + " Row: " + OString::number(nRowIdx));

            if (std::isnan(nValue))
            {
                // On paste, 0 becomes NaN, check whether it's expected
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), 0.0, nExpected);
            }
            else
            {
                // Without the fix in place, this test would have failed with
                // - Expected: 0
                // - Actual  : 3.33625955201419
                // - Incorrect value in Col: 2 Row: 51
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), nExpected, nValue, 1e-1);
            }
        }
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
