/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XTablePivotChart.hpp>
#include <com/sun/star/table/XTablePivotCharts.hpp>
#include <com/sun/star/table/XTablePivotChartsSupplier.hpp>

#include <rtl/strbuf.hxx>

#include <algorithm>

class PivotChartTest : public ChartTest
{
public:
    PivotChartTest() : ChartTest()
    {}

    void testRoundtrip();
    void testChangePivotTable();

    CPPUNIT_TEST_SUITE(PivotChartTest);
    CPPUNIT_TEST(testRoundtrip);
    CPPUNIT_TEST(testChangePivotTable);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{

void lclModifyOrientation(uno::Reference<sheet::XDataPilotDescriptor> const & xDescriptor,
                          OUString const & sFieldName,
                          sheet::DataPilotFieldOrientation eOrientation)
{
    uno::Reference<container::XIndexAccess> xPilotIndexAccess(xDescriptor->getDataPilotFields(), UNO_QUERY_THROW);
    sal_Int32 nCount = xPilotIndexAccess->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<container::XNamed> xNamed(xPilotIndexAccess->getByIndex(i), UNO_QUERY_THROW);
        OUString aName = xNamed->getName();
        uno::Reference<beans::XPropertySet> xPropSet(xNamed, UNO_QUERY_THROW);
        if (aName == sFieldName)
            xPropSet->setPropertyValue("Orientation", uno::makeAny(eOrientation));
    }
}

void lclCheckSequence(std::vector<double> const & reference,
                      uno::Sequence<uno::Any> const & values,
                      double delta)
{
    CPPUNIT_ASSERT_EQUAL(reference.size(), size_t(values.getLength()));
    for (size_t i = 0; i < reference.size(); ++i)
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(
            reference[i], values[i].get<double>(), delta);
    }
}

OUString lclGetLabel(Reference<chart2::XChartDocument> const & xChartDoc, sal_Int32 nSeriesIndex)
{
    Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, nSeriesIndex);
    return xLabelDataSequence->getData()[0].get<OUString>();
}

uno::Reference<sheet::XDataPilotTable> lclGetPivotTableByName(sal_Int32 nIndex, OUString const & sPivotTableName,
                                                           uno::Reference<lang::XComponent> const & xComponent)
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xSheetIndexAccess(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Any aAny = xSheetIndexAccess->getByIndex(nIndex);
    uno::Reference<sheet::XSpreadsheet> xSheet;
    CPPUNIT_ASSERT(aAny >>= xSheet);
    uno::Reference<sheet::XDataPilotTablesSupplier> xDataPilotTablesSupplier(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotTables> xDataPilotTables = xDataPilotTablesSupplier->getDataPilotTables();
    return uno::Reference<sheet::XDataPilotTable>(xDataPilotTables->getByName(sPivotTableName), UNO_QUERY_THROW);
}

} // end anonymous namespace

void PivotChartTest::testRoundtrip()
{
    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    std::vector<double> aReference1 { 10162.033139, 16614.523063, 27944.146101 };
    OUString aExpectedLabel1("Exp.");

    std::vector<double> aReference2 { 101879.458079, 178636.929704, 314626.484864 };
    OUString aExpectedLabel2("Rev.");

    load("/chart2/qa/extras/data/ods/", "PivotChartRoundTrip.ods");

    xChartDoc = getPivotChartDocFromSheet(1, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getNumberOfDataSeries(xChartDoc));

    // Check the data series
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0)->getData();
        lclCheckSequence(aReference1, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(aExpectedLabel1, lclGetLabel(xChartDoc, 0));
    }
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1)->getData();
        lclCheckSequence(aReference2, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(aExpectedLabel2, lclGetLabel(xChartDoc, 1));
    }

    // Modify the pivot table
    {
        uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, "DataPilot1", mxComponent);
        uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor(xDataPilotTable, UNO_QUERY_THROW);

        lclModifyOrientation(xDataPilotDescriptor, "Exp.", sheet::DataPilotFieldOrientation_HIDDEN);
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));

    // Check again the data series
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0)->getData();
        lclCheckSequence(aReference2, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(OUString(""), lclGetLabel(xChartDoc, 0));
    }

    reload("calc8");

    xChartDoc = getPivotChartDocFromSheet(1, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));

    // Check again the data series
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0)->getData();
        lclCheckSequence(aReference2, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(OUString(""), lclGetLabel(xChartDoc, 0));
    }
}

void PivotChartTest::testChangePivotTable()
{
    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    load("/chart2/qa/extras/data/ods/", "PivotTableExample.ods");

    // Check that we don't have any pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(1, mxComponent);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTablePivotCharts, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Create a new pivot chart
    xTablePivotCharts->addNewByName("Chart", awt::Rectangle{0, 0, 9000, 9000}, "DataPilot1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Get the pivot chart document so we ca access its data
    xChartDoc.set(getPivotChartDocFromSheet(xTablePivotCharts, 0));

    CPPUNIT_ASSERT(xChartDoc.is());

    // Check first data series
    {
        std::vector<double> aReference { 10162.033139, 16614.523063, 27944.146101 };
        OUString aExpectedLabel("Exp.");

        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, lclGetLabel(xChartDoc, 0));
    }

    // Check second data series
    {
        std::vector<double> aReference { 101879.458079, 178636.929704, 314626.484864 };
        OUString aExpectedLabel("Rev.");

        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, lclGetLabel(xChartDoc, 1));
    }

    // Modify the pivot table
    {
        uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, "DataPilot1", mxComponent);
        uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor(xDataPilotTable, UNO_QUERY_THROW);

        lclModifyOrientation(xDataPilotDescriptor, "Service Month", sheet::DataPilotFieldOrientation_ROW);
        lclModifyOrientation(xDataPilotDescriptor, "Group Segment", sheet::DataPilotFieldOrientation_COLUMN);
        lclModifyOrientation(xDataPilotDescriptor, "Rev.", sheet::DataPilotFieldOrientation_HIDDEN);
    }

    // Check the pivot chart again as we expect it has been updated when we updated the pivot table

    CPPUNIT_ASSERT(xChartDoc.is());

    // Check the first data series
    {
        std::vector<double> aReference { 2855.559, 1780.326, 2208.713, 2130.064, 1187.371 };
        OUString aExpectedLabel("Big");

        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, lclGetLabel(xChartDoc, 0));
    }

    // Check the second data series
    {
        std::vector<double> aReference { 4098.908, 2527.286, 4299.716, 2362.225, 3326.389 };
        OUString aExpectedLabel("Medium");

        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, lclGetLabel(xChartDoc, 1));
    }

    // Check the third data series
    {
        std::vector<double> aReference { 4926.303, 5684.060, 4201.398, 7290.795, 5841.591 };
        OUString aExpectedLabel("Small");

        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 2)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, lclGetLabel(xChartDoc, 2));
    }

    // Modify the pivot table
    {
        uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, "DataPilot1", mxComponent);
        uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor(xDataPilotTable, UNO_QUERY_THROW);

        lclModifyOrientation(xDataPilotDescriptor, "Service Month", sheet::DataPilotFieldOrientation_HIDDEN);
    }

    // Check the pivot chart again as we expect it has been updated when we updated the pivot table

    CPPUNIT_ASSERT(xChartDoc.is());

    // Check the first data series
    {
        std::vector<double> aReference { 10162.033139 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(OUString("Big"), lclGetLabel(xChartDoc, 0));
    }
    // Check the second data series
    {
        std::vector<double> aReference { 16614.523063 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(OUString("Medium"), lclGetLabel(xChartDoc, 1));
    }
    // Check the third data series
    {
        std::vector<double> aReference { 27944.146101 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 2)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(OUString("Small"), lclGetLabel(xChartDoc, 2));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PivotChartTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
