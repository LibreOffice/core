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

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // create a new document
    load("private:factory/scalc");

    dispatchCommand(mxComponent, ".uno:Paste", {});

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

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf99969)
{
    loadFromFile(u"ods/tdf99969.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);
    sal_Int32 nColumns = 2;
    sal_Int32 nRows = 6;
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart::XChartDataArray> xChartData(xChartDoc->getData(), uno::UNO_QUERY_THROW);

    uno::Sequence<OUString> aExpectedColumnDescriptions = xChartData->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in origin file", nColumns,
                                 aExpectedColumnDescriptions.getLength());

    uno::Sequence<OUString> aExpectedRowDescriptions = xChartData->getRowDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of rows in origin file", nRows,
                                 aExpectedRowDescriptions.getLength());

    Sequence<Sequence<double>> aExpectedData = xChartData->getData();

    dispatchCommand(mxComponent, ".uno:GoToCell",
                    { comphelper::makePropertyValue("ToPoint", uno::Any(OUString("C2:L25"))) });

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // create a new document
    load("private:factory/scalc");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    uno::Reference<chart2::XChartDocument> xChartDoc2 = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc2.is());

    uno::Reference<chart::XChartDataArray> xDataArray(xChartDoc2->getDataProvider(),
                                                      UNO_QUERY_THROW);

    Sequence<OUString> aColumnDesc = xDataArray->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in destination file", nColumns,
                                 aColumnDesc.getLength());
    for (sal_Int32 i = 0; i < nColumns; ++i)
    {
        // Without the fix in place, this test would have failed with
        // - Expected: ABC
        // - Actual  :
        // - Incorrect description in column: 0
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

            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), nExpected, nValue);
        }
    }
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf120348)
{
    loadFromFile(u"ods/tdf120348.ods");
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
        = { u"Ωφέλιμο"_ustr, u"Επικίνδυνο"_ustr, u"Απόσταση"_ustr, u"Μάσκα"_ustr, u"Εμβόλιο"_ustr };

    loadFromFile(u"ods/tdf151091.ods");
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

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // create a new writer document
    load("private:factory/swriter");

    dispatchCommand(mxComponent, ".uno:Paste", {});

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
    loadFromFile(u"ods/tdf107097.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getPivotChartDocFromSheet(1, mxComponent),
                                                    uno::UNO_QUERY_THROW);
    testCopyPasteToNewSheet(xChartDoc, "Object 1", 4, 12);
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf136011)
{
    loadFromFile(u"ods/tdf136011.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);
    testCopyPasteToNewSheet(xChartDoc, "Object 1", 3, 9);

    loadFromFile(u"ods/tdf136011.ods");
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
    loadFromFile(u"ods/tdf62057.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 7
    // - Incorrect number of columns in destination file
    testCopyPasteToNewSheet(xChartDoc, "Object 1", 2, 6);
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf98690)
{
    loadFromFile(u"xlsx/tdf98690.xlsx");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart::XChartDataArray> xChartData(xChartDoc->getData(), uno::UNO_QUERY_THROW);
    uno::Sequence<OUString> aSeriesList = xChartData->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), aSeriesList.getLength());

    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToObject", OUString("Chart 2")),
    };
    dispatchCommand(mxComponent, ".uno:GoToObject", aPropertyValues);

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // create a new document
    load("private:factory/scalc");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    uno::Reference<chart::XChartDocument> xChartDoc2(getChartCompFromSheet(0, 0, mxComponent),
                                                     uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc2.is());
    uno::Reference<chart::XChartDataArray> xChartData2(xChartDoc2->getData(), uno::UNO_QUERY_THROW);
    uno::Sequence<OUString> aSeriesList2 = xChartData2->getColumnDescriptions();

    // Without the fix in place, this test would have failed with
    // - Expected: 12
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(12), aSeriesList2.getLength());
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf101894)
{
    loadFromFile(u"ods/tdf101894.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart::XChartDataArray> xChartData(xChartDoc->getData(), uno::UNO_QUERY_THROW);

    uno::Sequence<OUString> aExpectedColumnDescriptions = xChartData->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in origin file",
                                 static_cast<sal_Int32>(12),
                                 aExpectedColumnDescriptions.getLength());

    uno::Sequence<OUString> aExpectedRowDescriptions = xChartData->getRowDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of rows in origin file",
                                 static_cast<sal_Int32>(8), aExpectedRowDescriptions.getLength());

    Sequence<Sequence<double>> aExpectedData = xChartData->getData();

    // Create a copy of the sheet and move to the end
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "DocName", uno::Any(OUString("tdf101894")) },
                                           { "Index", uno::Any(sal_uInt16(32767)) },
                                           { "Copy", uno::Any(true) } }));
    dispatchCommand(mxComponent, ".uno:Move", aArgs);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xIA->getCount());

    for (sal_Int32 sheetIndex = 0; sheetIndex < 2; ++sheetIndex)
    {
        uno::Reference<chart::XChartDocument> xChartDoc2(
            getChartCompFromSheet(sheetIndex, 0, mxComponent), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xChartDoc2.is());
        uno::Reference<chart::XChartDataArray> xChartData2(xChartDoc2->getData(),
                                                           uno::UNO_QUERY_THROW);

        uno::Sequence<OUString> aColumnDescriptions = xChartData2->getColumnDescriptions();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in origin file",
                                     static_cast<sal_Int32>(12), aColumnDescriptions.getLength());
        for (sal_Int32 i = 0; i < 12; ++i)
        {
            OString sMessage("Incorrect description in column: " + OString::number(i));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aExpectedColumnDescriptions[i],
                                         aColumnDescriptions[i]);
        }

        uno::Sequence<OUString> aRowDescriptions = xChartData2->getRowDescriptions();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of rows in origin file",
                                     static_cast<sal_Int32>(8), aRowDescriptions.getLength());
        for (sal_Int32 i = 0; i < 8; ++i)
        {
            OString sMessage("Incorrect description in row: " + OString::number(i));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aExpectedRowDescriptions[i],
                                         aRowDescriptions[i]);
        }

        Sequence<Sequence<double>> aData = xChartData2->getData();

        for (sal_Int32 nRowIdx = 0; nRowIdx < 8; ++nRowIdx)
        {
            for (sal_Int32 nColIdx = 0; nColIdx < 12; ++nColIdx)
            {
                double nValue = aData[nRowIdx][nColIdx];
                double nExpected = aExpectedData[nRowIdx][nColIdx];
                OString sMessage("Incorrect value in Col: " + OString::number(nColIdx)
                                 + " Row: " + OString::number(nRowIdx));

                // Without the fix in place, this test would have failed with
                // - Expected: 1
                // - Actual  : 2.2250738585072e-308
                // - Incorrect value in Col: 0 Row: 0
                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), nExpected, nValue, 1e-1);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testCopyPasteChartWithDotInSheetName)
{
    loadFromFile(u"ods/chartWithDotInSheetName.ods");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart::XChartDataArray> xChartData(xChartDoc->getData(), uno::UNO_QUERY_THROW);

    uno::Sequence<OUString> aExpectedColumnDescriptions = xChartData->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in origin file",
                                 static_cast<sal_Int32>(4),
                                 aExpectedColumnDescriptions.getLength());

    uno::Sequence<OUString> aExpectedRowDescriptions = xChartData->getRowDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of rows in origin file",
                                 static_cast<sal_Int32>(7), aExpectedRowDescriptions.getLength());

    Sequence<Sequence<double>> aExpectedData = xChartData->getData();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Name", uno::Any(OUString("NewTab")) }, { "Index", uno::Any(sal_uInt16(2)) } }));
    dispatchCommand(mxComponent, ".uno:Insert", aArgs);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xIA->getCount());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    for (sal_Int32 sheetIndex = 0; sheetIndex < 2; ++sheetIndex)
    {
        uno::Reference<chart::XChartDocument> xChartDoc2(
            getChartCompFromSheet(sheetIndex, 0, mxComponent), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xChartDoc2.is());
        uno::Reference<chart::XChartDataArray> xChartData2(xChartDoc2->getData(),
                                                           uno::UNO_QUERY_THROW);

        uno::Sequence<OUString> aColumnDescriptions = xChartData2->getColumnDescriptions();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of columns in origin file",
                                     static_cast<sal_Int32>(4), aColumnDescriptions.getLength());
        for (sal_Int32 i = 0; i < 4; ++i)
        {
            OString sMessage("Incorrect description in column: " + OString::number(i));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aExpectedColumnDescriptions[i],
                                         aColumnDescriptions[i]);
        }

        uno::Sequence<OUString> aRowDescriptions = xChartData2->getRowDescriptions();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect number of rows in origin file",
                                     static_cast<sal_Int32>(7), aRowDescriptions.getLength());
        for (sal_Int32 i = 0; i < 7; ++i)
        {
            OString sMessage("Incorrect description in row: " + OString::number(i));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), aExpectedRowDescriptions[i],
                                         aRowDescriptions[i]);
        }

        Sequence<Sequence<double>> aData = xChartData2->getData();

        for (sal_Int32 nRowIdx = 0; nRowIdx < 7; ++nRowIdx)
        {
            for (sal_Int32 nColIdx = 0; nColIdx < 4; ++nColIdx)
            {
                double nValue = aData[nRowIdx][nColIdx];
                double nExpected = aExpectedData[nRowIdx][nColIdx];
                OString sMessage("Incorrect value in Col: " + OString::number(nColIdx)
                                 + " Row: " + OString::number(nRowIdx));

                CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sMessage.getStr(), nExpected, nValue, 1e-1);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf158223)
{
    loadFromFile(u"ods/tdf158223.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xIA->getCount());

    for (sal_Int32 sheetIndex = 0; sheetIndex < 2; ++sheetIndex)
    {
        OUString sExpectedValuesX("$Tabelle" + OUString::number(sheetIndex + 1) + ".$A$2:$A$11");
        OUString sExpectedValuesY("$Tabelle" + OUString::number(sheetIndex + 1) + ".$B$2:$B$11");
        uno::Reference<chart2::XChartDocument> xChartDoc
            = getChartDocFromSheet(sheetIndex, mxComponent);
        Reference<chart2::data::XDataSequence> xValuesX
            = getDataSequenceFromDocByRole(xChartDoc, u"values-x");
        CPPUNIT_ASSERT_EQUAL(sExpectedValuesX, xValuesX->getSourceRangeRepresentation());
        Reference<chart2::data::XDataSequence> xValuesY
            = getDataSequenceFromDocByRole(xChartDoc, u"values-y");
        CPPUNIT_ASSERT_EQUAL(sExpectedValuesY, xValuesY->getSourceRangeRepresentation());
    }

    // Remove last sheet
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(3)) } }));
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xIA->getCount());

    for (sal_Int32 sheetIndex = 0; sheetIndex < 2; ++sheetIndex)
    {
        OUString sExpectedValuesX("$Tabelle" + OUString::number(sheetIndex + 1) + ".$A$2:$A$11");
        OUString sExpectedValuesY("$Tabelle" + OUString::number(sheetIndex + 1) + ".$B$2:$B$11");
        uno::Reference<chart2::XChartDocument> xChartDoc
            = getChartDocFromSheet(sheetIndex, mxComponent);
        Reference<chart2::data::XDataSequence> xValuesX
            = getDataSequenceFromDocByRole(xChartDoc, u"values-x");

        // Without the fix in place, this test would have failed with
        // - Expected: $Tabelle2.$A$2:$A$11
        // - Actual  : $Tabelle2.$A$2:$Tabelle1.$A$11
        CPPUNIT_ASSERT_EQUAL(sExpectedValuesX, xValuesX->getSourceRangeRepresentation());
        Reference<chart2::data::XDataSequence> xValuesY
            = getDataSequenceFromDocByRole(xChartDoc, u"values-y");
        CPPUNIT_ASSERT_EQUAL(sExpectedValuesY, xValuesY->getSourceRangeRepresentation());
    }
}

CPPUNIT_TEST_FIXTURE(Chart2UiChartTest, testTdf153706)
{
    // Load a spreadsheet with a to-page XY scatter chart with the sheet as data source
    loadFromFile(u"ods/tdf153706_XY_scatter_chart.ods");

    // Select the cell range around the chart, and copy the range to clipboard, including the chart
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"D1:K23"_ustr) });
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // create a new document
    load(u"private:factory/scalc"_ustr);

    // Paste; this must create a chart with own data source having a proper copy of the data
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    css::uno::Reference xChartDoc(getChartDocFromSheet(0, mxComponent), css::uno::UNO_SET_THROW);
    auto xDataArray(xChartDoc->getDataProvider().queryThrow<chart::XChartDataArray>());

    css::uno::Sequence<Sequence<double>> aData = xDataArray->getData();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aData.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aData[0].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aData[1].getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aData[2].getLength());
    CPPUNIT_ASSERT_EQUAL(2.0, aData[0][0]);
    CPPUNIT_ASSERT_EQUAL(3.0, aData[0][1]);
    CPPUNIT_ASSERT_EQUAL(3.0, aData[1][0]);
    CPPUNIT_ASSERT_EQUAL(2.0, aData[1][1]);
    CPPUNIT_ASSERT_EQUAL(4.0, aData[2][0]);
    CPPUNIT_ASSERT_EQUAL(1.0, aData[2][1]);

    // Without the fix, this would fail with
    // - Expected: 1
    // - Actual  : 2
    // i.e., the X values were treated as another Y series
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));

    auto xSeries(getDataSeriesFromDoc(xChartDoc, 0).queryThrow<chart2::data::XDataSource>());
    auto sequences = xSeries->getDataSequences();
    // Without the fix, this would fail with
    // - Expected: 2
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), sequences.getLength());

    auto propX(sequences[0]->getValues().queryThrow<beans::XPropertySet>());
    // Without the fix, this would fail with
    // - Expected: values-x
    // - Actual  : values-y
    CPPUNIT_ASSERT_EQUAL(u"values-x"_ustr, propX->getPropertyValue("Role").get<OUString>());

    auto propY(sequences[1]->getValues().queryThrow<beans::XPropertySet>());
    CPPUNIT_ASSERT_EQUAL(u"values-y"_ustr, propY->getPropertyValue("Role").get<OUString>());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
