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
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutInfo.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XTablePivotCharts.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormat.hpp>

#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>

namespace com::sun::star::table { class XCellRange; }
namespace com::sun::star::util { class XNumberFormats; }

class PivotChartTest : public ChartTest
{
public:
    PivotChartTest()
        : ChartTest(u"/chart2/qa/extras/data/"_ustr)
    {}

    void testRoundtrip();
    void testChangePivotTable();
    void testPivotChartWithOneColumnField();
    void testPivotChartWithOneRowField();
    void testPivotTableDataProvider_PivotTableFields();
    void testPivotChartRowFieldInOutlineMode();
    void testPivotChartWithDateRowField();

    CPPUNIT_TEST_SUITE(PivotChartTest);
    CPPUNIT_TEST(testRoundtrip);
    CPPUNIT_TEST(testChangePivotTable);
    CPPUNIT_TEST(testPivotChartWithOneColumnField);
    CPPUNIT_TEST(testPivotChartWithOneRowField);
    CPPUNIT_TEST(testPivotTableDataProvider_PivotTableFields);
    CPPUNIT_TEST(testPivotChartRowFieldInOutlineMode);
    CPPUNIT_TEST(testPivotChartWithDateRowField);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{

void lclModifyOrientation(uno::Reference<sheet::XDataPilotDescriptor> const & xDescriptor,
                          std::u16string_view sFieldName,
                          sheet::DataPilotFieldOrientation eOrientation)
{
    uno::Reference<container::XIndexAccess> xIndexAccess(xDescriptor->getDataPilotFields(), UNO_SET_THROW);
    sal_Int32 nCount = xIndexAccess->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<container::XNamed> xNamed(xIndexAccess->getByIndex(i), UNO_QUERY_THROW);
        OUString aName = xNamed->getName();
        uno::Reference<beans::XPropertySet> xPropSet(xNamed, UNO_QUERY_THROW);
        if (aName == sFieldName)
            xPropSet->setPropertyValue(u"Orientation"_ustr, uno::Any(eOrientation));
    }
}

void lclModifyFunction(uno::Reference<sheet::XDataPilotDescriptor> const & xDescriptor,
                          std::u16string_view sFieldName,
                          sheet::GeneralFunction eFunction)
{
    uno::Reference<container::XIndexAccess> xPilotIndexAccess(xDescriptor->getDataPilotFields(), UNO_SET_THROW);
    sal_Int32 nCount = xPilotIndexAccess->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<container::XNamed> xNamed(xPilotIndexAccess->getByIndex(i), UNO_QUERY_THROW);
        OUString aName = xNamed->getName();
        uno::Reference<beans::XPropertySet> xPropSet(xNamed, UNO_QUERY_THROW);
        if (aName == sFieldName)
            xPropSet->setPropertyValue(u"Function"_ustr, uno::Any(eFunction));
    }
}

void lclModifyLayoutInfo(uno::Reference<sheet::XDataPilotDescriptor> const & xDescriptor,
                          std::u16string_view sFieldName,
                          sheet::DataPilotFieldLayoutInfo aLayoutInfo)
{
    uno::Reference<container::XIndexAccess> xIndexAccess(xDescriptor->getDataPilotFields(), UNO_SET_THROW);
    sal_Int32 nCount = xIndexAccess->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<container::XNamed> xNamed(xIndexAccess->getByIndex(i), UNO_QUERY_THROW);
        OUString aName = xNamed->getName();
        uno::Reference<beans::XPropertySet> xPropSet(xNamed, UNO_QUERY_THROW);
        if (aName == sFieldName)
        {
            uno::Any aValue;
            aValue <<= aLayoutInfo;
            xPropSet->setPropertyValue(u"LayoutInfo"_ustr, aValue);
        }
    }
}

void lclModifySubtotals(uno::Reference<sheet::XDataPilotDescriptor> const & xDescriptor,
                        std::u16string_view sFieldName,
                        uno::Sequence<sheet::GeneralFunction> const & rSubtotalFunctions)
{
    uno::Reference<container::XIndexAccess> xIndexAccess(xDescriptor->getDataPilotFields(), UNO_SET_THROW);
    sal_Int32 nCount = xIndexAccess->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference<container::XNamed> xNamed(xIndexAccess->getByIndex(i), UNO_QUERY_THROW);
        OUString aName = xNamed->getName();
        uno::Reference<beans::XPropertySet> xPropSet(xNamed, UNO_QUERY_THROW);
        if (aName == sFieldName)
        {
            uno::Any aValue;
            aValue <<= rSubtotalFunctions;
            xPropSet->setPropertyValue(u"Subtotals"_ustr, aValue);
        }
    }
}

void lclModifyColumnGrandTotal(uno::Reference<sheet::XDataPilotDescriptor> const & xDataPilotDescriptor, bool bTotal)
{
        uno::Reference<beans::XPropertySet> xProperties(xDataPilotDescriptor, uno::UNO_QUERY_THROW);
        xProperties->setPropertyValue(u"ColumnGrand"_ustr, uno::Any(bTotal));
}

void lclModifyRowGrandTotal(uno::Reference<sheet::XDataPilotDescriptor> const & xDataPilotDescriptor, bool bTotal)
{
        uno::Reference<beans::XPropertySet> xProperties(xDataPilotDescriptor, uno::UNO_QUERY_THROW);
        xProperties->setPropertyValue(u"RowGrand"_ustr, uno::Any(bTotal));
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

void lclCheckCategories(std::vector<OUString> const & reference,
                        uno::Reference<chart2::data::XDataSequence> const & xSequence)
{
    uno::Reference<chart2::data::XTextualDataSequence> xTextualDataSequence(xSequence, uno::UNO_QUERY_THROW);
    uno::Sequence<OUString> aText = xTextualDataSequence->getTextualData();

    CPPUNIT_ASSERT_EQUAL(reference.size(), size_t(aText.getLength()));
    for (size_t i = 0; i < reference.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(reference[i], aText[i]);
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

uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>>
    lclGetCategories(Reference<chart2::XChartDocument> const & xChartDoc)
{
    uno::Sequence<beans::PropertyValue> aArguments( comphelper::InitPropertySequence(
            {{"CellRangeRepresentation", uno::Any(u"PT@categories"_ustr)}} ));

    uno::Reference<chart2::data::XDataProvider> xDataProvider(xChartDoc->getDataProvider(), uno::UNO_SET_THROW);
    return xDataProvider->createDataSource(aArguments)->getDataSequences();
}

struct Value
{
    OUString maString;
    double mfValue;
    bool mbIsValue;

    Value(OUString const & rString)
        : maString(rString)
        , mfValue(0.0)
        , mbIsValue(false)
    {}

    Value(double fValue)
        : mfValue(fValue)
        , mbIsValue(true)
    {}
};

uno::Reference< sheet::XDataPilotTables>
lclGetDataPilotTables(sal_Int32 nIndex, uno::Reference<sheet::XSpreadsheetDocument> const & xSheetDoc)
{
    uno::Reference<sheet::XSpreadsheets> xSpreadsheets = xSheetDoc->getSheets();
    uno::Reference<container::XIndexAccess> oIndexAccess(xSpreadsheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet;
    CPPUNIT_ASSERT(oIndexAccess->getByIndex(nIndex) >>= xSheet);

    // create the test objects
    uno::Reference< sheet::XDataPilotTablesSupplier> xDataPilotTablesSupplier(xSheet, uno::UNO_QUERY_THROW);
    return xDataPilotTablesSupplier->getDataPilotTables();
}

table::CellRangeAddress lclCreateTestData(uno::Reference<sheet::XSpreadsheetDocument> const & xSheetDoc)
{
    CPPUNIT_ASSERT_MESSAGE("no calc document!", xSheetDoc.is());

    std::vector<OUString> aHeaders {
        u"Country"_ustr, u"City"_ustr, u"Type"_ustr, u"Sales T1"_ustr, u"Sales T2"_ustr, u"Sales T3"_ustr, u"Sales T4"_ustr, u"Date"_ustr
    };

    std::vector<std::vector<Value>> aData {
        { {u"FR"_ustr}, {u"Paris"_ustr},     {u"A"_ustr}, {123.0}, {223.0}, {323.0}, {423.0}, {u"12/14/15"_ustr} },
        { {u"EN"_ustr}, {u"London"_ustr},    {u"A"_ustr}, {456.0}, {556.0}, {656.0}, {756.0}, {u"12/11/15"_ustr} },
        { {u"DE"_ustr}, {u"Berlin"_ustr},    {u"A"_ustr}, {468.0}, {568.0}, {668.0}, {768.0}, {u"12/11/15"_ustr} },
        { {u"FR"_ustr}, {u"Nantes"_ustr},    {u"A"_ustr}, {694.0}, {794.0}, {894.0}, {994.0}, {u"12/11/15"_ustr} },
        { {u"EN"_ustr}, {u"Glasgow"_ustr},   {u"A"_ustr}, {298.0}, {398.0}, {498.0}, {598.0}, {u"12/11/15"_ustr} },
        { {u"DE"_ustr}, {u"Munich"_ustr},    {u"A"_ustr}, {369.0}, {469.0}, {569.0}, {669.0}, {u"12/11/15"_ustr} },
        { {u"FR"_ustr}, {u"Paris"_ustr},     {u"B"_ustr}, {645.0}, {745.0}, {845.0}, {945.0}, {u"12/11/15"_ustr} },
        { {u"EN"_ustr}, {u"London"_ustr},    {u"B"_ustr}, {687.0}, {787.0}, {887.0}, {987.0}, {u"03/21/17"_ustr} },
        { {u"DE"_ustr}, {u"Munich"_ustr},    {u"B"_ustr}, {253.0}, {353.0}, {453.0}, {553.0}, {u"12/17/15"_ustr} },
        { {u"FR"_ustr}, {u"Nantes"_ustr},    {u"B"_ustr}, {474.0}, {574.0}, {674.0}, {774.0}, {u"01/20/16"_ustr} },
        { {u"EN"_ustr}, {u"Liverpool"_ustr}, {u"B"_ustr}, {562.0}, {662.0}, {762.0}, {862.0}, {u"01/20/16"_ustr} },
        { {u"DE"_ustr}, {u"Berlin"_ustr},    {u"B"_ustr}, {648.0}, {748.0}, {848.0}, {948.0}, {u"01/20/16"_ustr} }
    };

    // Getting spreadsheet
    uno::Reference<sheet::XSpreadsheets> xSpreadsheets = xSheetDoc->getSheets();
    uno::Reference<container::XIndexAccess> oIndexAccess(xSpreadsheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet;
    CPPUNIT_ASSERT(oIndexAccess->getByIndex(0) >>= xSheet);

    uno::Reference<sheet::XSpreadsheet> oPivotTableSheet;
    xSpreadsheets->insertNewByName(u"Pivot Table"_ustr, 1);
    CPPUNIT_ASSERT(oIndexAccess->getByIndex(1) >>= oPivotTableSheet);

    sal_Int32 currentRow = 0;
    for (size_t column = 0; column < aHeaders.size(); ++column)
    {
        xSheet->getCellByPosition(column, currentRow)->setFormula(aHeaders[column]);
    }
    currentRow++;

    for (std::vector<Value> const & rRowOfData : aData)
    {
        for (size_t column = 0; column < rRowOfData.size(); ++column)
        {
            Value const & rValue = rRowOfData[column];
            uno::Reference<table::XCell> xCell(xSheet->getCellByPosition(column, currentRow));
            if (rValue.mbIsValue)
                xCell->setValue(rValue.mfValue);
            else
                xCell->setFormula(rValue.maString);
        }
        currentRow++;
    }

    sal_Int32 nEndCol = sal_Int32(aHeaders.size() - 1);
    sal_Int32 nEndRow = sal_Int32(1/*HEADER*/ + aData.size() - 1);

    // Apply date format to the last column
    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(xSheetDoc, UNO_QUERY_THROW);
    uno::Reference<util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
    uno::Reference<util::XNumberFormatTypes> xNumberFormatTypes(xNumberFormats, UNO_QUERY_THROW);
    lang::Locale aLocale;
    sal_Int32 nDateKey = xNumberFormatTypes->getStandardFormat(util::NumberFormat::DATE, aLocale);
    uno::Reference<table::XCellRange> xCellRange = xSheet->getCellRangeByPosition(nEndCol, 1, nEndCol, nEndRow);
    uno::Reference<beans::XPropertySet> xCellProp(xCellRange, UNO_QUERY_THROW);
    xCellProp->setPropertyValue(u"NumberFormat"_ustr, uno::Any(nDateKey));

    table::CellRangeAddress sCellRangeAddress;
    sCellRangeAddress.Sheet = 0;
    sCellRangeAddress.StartColumn = 0;
    sCellRangeAddress.StartRow = 0;
    sCellRangeAddress.EndColumn = nEndCol;
    sCellRangeAddress.EndRow = nEndRow;

    return sCellRangeAddress;
}

} // end anonymous namespace

void PivotChartTest::testRoundtrip()
{
    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    std::vector<double> aReference1 { 10162.033139, 16614.523063, 27944.146101 };

    std::vector<double> aReference2 { 101879.458079, 178636.929704, 314626.484864 };

    loadFromFile(u"ods/PivotChartRoundTrip.ods");

    xChartDoc = getPivotChartDocFromSheet(1, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getNumberOfDataSeries(xChartDoc));

    // Check the data series
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference1, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(u"Exp."_ustr, lclGetLabel(xChartDoc, 0));
    }
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 1)->getData();
        lclCheckSequence(aReference2, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(u"Rev."_ustr, lclGetLabel(xChartDoc, 1));
    }

    // Modify the pivot table
    {
        uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, u"DataPilot1"_ustr, mxComponent);
        uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor(xDataPilotTable, UNO_QUERY_THROW);
        lclModifyOrientation(xDataPilotDescriptor, u"Exp.", sheet::DataPilotFieldOrientation_HIDDEN);
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));

    // Check again the data series
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference2, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, lclGetLabel(xChartDoc, 0));
    }

    saveAndReload(u"calc8"_ustr);

    xChartDoc = getPivotChartDocFromSheet(1, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));

    // Check again the data series
    {
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference2, xSequence, 1E-4);
        CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, lclGetLabel(xChartDoc, 0));
    }
}

void PivotChartTest::testChangePivotTable()
{
    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    loadFromFile(u"ods/PivotTableExample.ods");

    // Check we have the Pivot Table
    OUString sPivotTableName(u"DataPilot1"_ustr);
    uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, sPivotTableName, mxComponent);
    CPPUNIT_ASSERT(xDataPilotTable.is());

    // Check that we don't have any pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(1, mxComponent);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTablePivotCharts, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Create a new pivot chart
    xTablePivotCharts->addNewByName(u"Chart"_ustr, awt::Rectangle{0, 0, 9000, 9000}, sPivotTableName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Get the pivot chart document so we can access its data
    xChartDoc.set(getPivotChartDocFromSheet(xTablePivotCharts, 0));

    CPPUNIT_ASSERT(xChartDoc.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getNumberOfDataSeries(xChartDoc));

    // Check first data series
    {
        std::vector<double> aReference { 10162.033139, 16614.523063, 27944.146101 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"Exp."_ustr, lclGetLabel(xChartDoc, 0));
    }

    // Check second data series
    {
        std::vector<double> aReference { 101879.458079, 178636.929704, 314626.484864 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"Rev."_ustr, lclGetLabel(xChartDoc, 1));
    }

    // Modify the pivot table, move "Group Segment" to Column fields,
    // add "Service Month" to Row fields, remove "Rev." Data field
    {
        uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor(xDataPilotTable, UNO_QUERY_THROW);

        lclModifyOrientation(xDataPilotDescriptor, u"Service Month", sheet::DataPilotFieldOrientation_ROW);
        lclModifyOrientation(xDataPilotDescriptor, u"Group Segment", sheet::DataPilotFieldOrientation_COLUMN);
        lclModifyOrientation(xDataPilotDescriptor, u"Rev.", sheet::DataPilotFieldOrientation_HIDDEN);
    }

    // Check the pivot chart again as we expect it has been updated when we updated the pivot table

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getNumberOfDataSeries(xChartDoc));

    // Check the first data series
    {
        std::vector<double> aReference { 2855.559, 1780.326, 2208.713, 2130.064, 1187.371 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);

        CPPUNIT_ASSERT_EQUAL(u"Big"_ustr, lclGetLabel(xChartDoc, 0));
    }

    // Check the second data series
    {
        std::vector<double> aReference { 4098.908, 2527.286, 4299.716, 2362.225, 3326.389 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);

        CPPUNIT_ASSERT_EQUAL(u"Medium"_ustr, lclGetLabel(xChartDoc, 1));
    }

    // Check the third data series
    {
        std::vector<double> aReference { 4926.303, 5684.060, 4201.398, 7290.795, 5841.591 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 2)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);

        CPPUNIT_ASSERT_EQUAL(u"Small"_ustr, lclGetLabel(xChartDoc, 2));
    }

    // Remove "Service Month" so row fields are empty - check we handle empty rows
    {
        uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor(xDataPilotTable, uno::UNO_QUERY_THROW);
        lclModifyOrientation(xDataPilotDescriptor, u"Service Month", sheet::DataPilotFieldOrientation_HIDDEN);
    }

    // Check the pivot chart again as we expect it has been updated when we updated the pivot table

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getNumberOfDataSeries(xChartDoc));

    // Check the first data series
    {
        std::vector<double> aReference { 10162.033139 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(u"Big"_ustr, lclGetLabel(xChartDoc, 0));
    }
    // Check the second data series
    {
        std::vector<double> aReference { 16614.523063 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(u"Medium"_ustr, lclGetLabel(xChartDoc, 1));
    }
    // Check the third data series
    {
        std::vector<double> aReference { 27944.146101 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 2)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(u"Small"_ustr, lclGetLabel(xChartDoc, 2));
    }

    // Enable column totals and check the data is still unchanged
    {
        uno::Reference<beans::XPropertySet> xProperties(xDataPilotTable, uno::UNO_QUERY_THROW);
        xProperties->setPropertyValue(u"ColumnGrand"_ustr, uno::Any(true));
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getNumberOfDataSeries(xChartDoc));

    // Check the first data series
    {
        std::vector<double> aReference { 10162.033139 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(u"Big"_ustr, lclGetLabel(xChartDoc, 0));
    }
    // Check the second data series
    {
        std::vector<double> aReference { 16614.523063 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(u"Medium"_ustr, lclGetLabel(xChartDoc, 1));
    }
    // Check the third data series
    {
        std::vector<double> aReference { 27944.146101 };
        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 2)->getData();
        lclCheckSequence(aReference, xSequence, 1E-3);
        CPPUNIT_ASSERT_EQUAL(u"Small"_ustr, lclGetLabel(xChartDoc, 2));
    }
}

void PivotChartTest::testPivotChartWithOneColumnField()
{
    // We put one field as COLUMN field only and one DATA field. We expect we will get as many data series
    // in the pivot table as many distinct column values we have (with this example data 3: DE, EN, FR).

    // SETUP DATA and PIVOT TABLE

    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    OUString sPivotTableName(u"DataPilotTable"_ustr);

    table::CellRangeAddress sCellRangeAddress = lclCreateTestData(xSheetDoc);

    uno::Reference<sheet::XDataPilotTables> xDataPilotTables = lclGetDataPilotTables(0, xSheetDoc);

    uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor = xDataPilotTables->createDataPilotDescriptor();
    xDataPilotDescriptor->setSourceRange(sCellRangeAddress);

    lclModifyOrientation(xDataPilotDescriptor, u"Country", sheet::DataPilotFieldOrientation_COLUMN);
    lclModifyOrientation(xDataPilotDescriptor, u"Sales T1", sheet::DataPilotFieldOrientation_DATA);
    lclModifyFunction(xDataPilotDescriptor, u"Sales T1", sheet::GeneralFunction_SUM);

    xDataPilotTables->insertNewByName(sPivotTableName, table::CellAddress{1, 0, 0}, xDataPilotDescriptor);

    // TEST

    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    // Check we have the Pivot Table

    uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, sPivotTableName, mxComponent);
    CPPUNIT_ASSERT(xDataPilotTable.is());

    // Check that we don't have any pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(1, mxComponent);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTablePivotCharts, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Create a new pivot chart
    xTablePivotCharts->addNewByName(u"PivotChart"_ustr, awt::Rectangle{ 9000, 9000, 21000, 18000 }, sPivotTableName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Get the pivot chart document so we can access its data
    xChartDoc.set(getPivotChartDocFromSheet(xTablePivotCharts, 0));

    CPPUNIT_ASSERT(xChartDoc.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getNumberOfDataSeries(xChartDoc));
    // Check data series 1
    {
        std::vector<double> aReference { 1738.0 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"DE"_ustr, lclGetLabel(xChartDoc, 0));
    }

    // Check data series 2
    {
        std::vector<double> aReference { 2003.0 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 1)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"EN"_ustr, lclGetLabel(xChartDoc, 1));
    }
    // Check data series 3
    {
        std::vector<double> aReference { 1936.0 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 2)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"FR"_ustr, lclGetLabel(xChartDoc, 2));
    }
}

void PivotChartTest::testPivotChartWithOneRowField()
{
    // We put one field as ROW field only and one DATA field. We expect we will get one data series
    // in the pivot table.

    // SETUP DATA and PIVOT TABLE

    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    OUString sPivotTableName(u"DataPilotTable"_ustr);

    table::CellRangeAddress sCellRangeAddress = lclCreateTestData(xSheetDoc);

    uno::Reference<sheet::XDataPilotTables> xDataPilotTables = lclGetDataPilotTables(0, xSheetDoc);

    uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor = xDataPilotTables->createDataPilotDescriptor();
    xDataPilotDescriptor->setSourceRange(sCellRangeAddress);

    lclModifyOrientation(xDataPilotDescriptor, u"Country", sheet::DataPilotFieldOrientation_ROW);
    lclModifyOrientation(xDataPilotDescriptor, u"Sales T1", sheet::DataPilotFieldOrientation_DATA);
    lclModifyFunction(xDataPilotDescriptor, u"Sales T1", sheet::GeneralFunction_SUM);

    xDataPilotTables->insertNewByName(sPivotTableName, table::CellAddress{1, 0, 0}, xDataPilotDescriptor);

    // TEST

    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    // Check we have the Pivot Table

    uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, sPivotTableName, mxComponent);
    CPPUNIT_ASSERT(xDataPilotTable.is());

    // Check that we don't have any pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(1, mxComponent);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTablePivotCharts, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Create a new pivot chart
    xTablePivotCharts->addNewByName(u"PivotChart"_ustr, awt::Rectangle{ 9000, 9000, 21000, 18000 }, sPivotTableName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Get the pivot chart document so we can access its data
    xChartDoc.set(getPivotChartDocFromSheet(xTablePivotCharts, 0));

    CPPUNIT_ASSERT(xChartDoc.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));
    // Check data series 1
    {
        std::vector<double> aReference { 1738.0, 2003.0, 1936.0 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, lclGetLabel(xChartDoc, 0));
    }
}

void PivotChartTest::testPivotTableDataProvider_PivotTableFields()
{
    // SETUP DATA and PIVOT TABLE

    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    OUString sPivotTableName(u"DataPilotTable"_ustr);

    table::CellRangeAddress sCellRangeAddress = lclCreateTestData(xSheetDoc);

    uno::Reference<sheet::XDataPilotTables> xDataPilotTables = lclGetDataPilotTables(0, xSheetDoc);

    uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor = xDataPilotTables->createDataPilotDescriptor();
    xDataPilotDescriptor->setSourceRange(sCellRangeAddress);

    lclModifyOrientation(xDataPilotDescriptor, u"City", sheet::DataPilotFieldOrientation_ROW);
    lclModifyOrientation(xDataPilotDescriptor, u"Country", sheet::DataPilotFieldOrientation_COLUMN);
    lclModifyOrientation(xDataPilotDescriptor, u"Type", sheet::DataPilotFieldOrientation_COLUMN);
    lclModifyOrientation(xDataPilotDescriptor, u"Sales T1", sheet::DataPilotFieldOrientation_DATA);
    lclModifyFunction(xDataPilotDescriptor, u"Sales T1", sheet::GeneralFunction_SUM);
    lclModifyOrientation(xDataPilotDescriptor, u"Sales T2", sheet::DataPilotFieldOrientation_DATA);
    lclModifyFunction(xDataPilotDescriptor, u"Sales T2", sheet::GeneralFunction_SUM);

    lclModifyColumnGrandTotal(xDataPilotDescriptor, true);
    lclModifyRowGrandTotal(xDataPilotDescriptor, true);

    xDataPilotTables->insertNewByName(sPivotTableName, table::CellAddress{1, 0, 0}, xDataPilotDescriptor);

    // TEST
    Reference<chart2::XChartDocument> xChartDoc;

    // Check we have the Pivot Table
    uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, sPivotTableName, mxComponent);
    CPPUNIT_ASSERT(xDataPilotTable.is());

    // refetch the XDataPilotDescriptor
    xDataPilotDescriptor.set(xDataPilotTable, uno::UNO_QUERY_THROW);

    // Check that we don't have any pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(1, mxComponent);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTablePivotCharts, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Create a new pivot chart
    xTablePivotCharts->addNewByName(u"PivotChart"_ustr, awt::Rectangle{ 9000, 9000, 21000, 18000 }, sPivotTableName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Get the pivot chart document so we can access its data
    xChartDoc.set(getPivotChartDocFromSheet(xTablePivotCharts, 0));

    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider(xChartDoc->getDataProvider(), UNO_QUERY_THROW);
    uno::Sequence<chart2::data::PivotTableFieldEntry> aFieldEntries;

    aFieldEntries = xPivotTableDataProvider->getColumnFields();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aFieldEntries.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Country"_ustr, aFieldEntries[0].Name);
    CPPUNIT_ASSERT_EQUAL(u"Type"_ustr, aFieldEntries[1].Name);

    aFieldEntries = xPivotTableDataProvider->getRowFields();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aFieldEntries.getLength());
    CPPUNIT_ASSERT_EQUAL(u"City"_ustr, aFieldEntries[0].Name);
    CPPUNIT_ASSERT_EQUAL(u"Data"_ustr, aFieldEntries[1].Name);

    aFieldEntries = xPivotTableDataProvider->getDataFields();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aFieldEntries.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Sum - Sales T1"_ustr, aFieldEntries[0].Name);
    CPPUNIT_ASSERT_EQUAL(u"Sum - Sales T2"_ustr, aFieldEntries[1].Name);

    // Data to column fields
    lclModifyOrientation(xDataPilotDescriptor, u"Data", sheet::DataPilotFieldOrientation_COLUMN);

    // Change the order of column fields: expected data, type, country
    lclModifyOrientation(xDataPilotDescriptor, u"Country", sheet::DataPilotFieldOrientation_HIDDEN);
    lclModifyOrientation(xDataPilotDescriptor, u"Type", sheet::DataPilotFieldOrientation_HIDDEN);

    lclModifyOrientation(xDataPilotDescriptor, u"Type", sheet::DataPilotFieldOrientation_COLUMN);
    lclModifyOrientation(xDataPilotDescriptor, u"Country", sheet::DataPilotFieldOrientation_COLUMN);

    // set the XPivotTableDataProvider again as the old one was exchanged
    xPivotTableDataProvider.set(xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW);

    aFieldEntries = xPivotTableDataProvider->getColumnFields();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aFieldEntries.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Data"_ustr, aFieldEntries[0].Name);
    CPPUNIT_ASSERT_EQUAL(u"Type"_ustr, aFieldEntries[1].Name);
    CPPUNIT_ASSERT_EQUAL(u"Country"_ustr, aFieldEntries[2].Name);

    aFieldEntries = xPivotTableDataProvider->getRowFields();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aFieldEntries.getLength());
    CPPUNIT_ASSERT_EQUAL(u"City"_ustr, aFieldEntries[0].Name);

    aFieldEntries = xPivotTableDataProvider->getDataFields();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aFieldEntries.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Sum - Sales T1"_ustr, aFieldEntries[0].Name);
    CPPUNIT_ASSERT_EQUAL(u"Sum - Sales T2"_ustr, aFieldEntries[1].Name);
}

void PivotChartTest::testPivotChartRowFieldInOutlineMode()
{
    // SETUP DATA and PIVOT TABLE

    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    OUString sPivotTableName(u"DataPilotTable"_ustr);

    table::CellRangeAddress sCellRangeAddress = lclCreateTestData(xSheetDoc);

    uno::Reference<sheet::XDataPilotTables> xDataPilotTables = lclGetDataPilotTables(0, xSheetDoc);

    uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor = xDataPilotTables->createDataPilotDescriptor();
    xDataPilotDescriptor->setSourceRange(sCellRangeAddress);

    lclModifyOrientation(xDataPilotDescriptor, u"Country", sheet::DataPilotFieldOrientation_ROW);
    lclModifyOrientation(xDataPilotDescriptor, u"City", sheet::DataPilotFieldOrientation_ROW);
    lclModifyOrientation(xDataPilotDescriptor, u"Sales T1", sheet::DataPilotFieldOrientation_DATA);
    lclModifyFunction(xDataPilotDescriptor, u"Sales T1", sheet::GeneralFunction_SUM);
    xDataPilotTables->insertNewByName(sPivotTableName, table::CellAddress{1, 0, 0}, xDataPilotDescriptor);

    // TEST
    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    // Check we have the Pivot Table
    uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, sPivotTableName, mxComponent);
    CPPUNIT_ASSERT(xDataPilotTable.is());

    // refetch the XDataPilotDescriptor
    xDataPilotDescriptor.set(xDataPilotTable, uno::UNO_QUERY_THROW);

    // Check that we don't have any pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(1, mxComponent);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTablePivotCharts, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Create a new pivot chart
    xTablePivotCharts->addNewByName(u"PivotChart"_ustr, awt::Rectangle{ 9000, 9000, 21000, 18000 }, sPivotTableName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Get the pivot chart document so we can access its data
    xChartDoc.set(getPivotChartDocFromSheet(xTablePivotCharts, 0));

    CPPUNIT_ASSERT(xChartDoc.is());

    // Test case with defaults

    // Check when using defaults the data is as expected
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));
    {
        std::vector<double> aReference { 1116.0, 622.0, 298.0, 562.0, 1143.0, 1168.0, 768.0 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, lclGetLabel(xChartDoc, 0));
    }
    // Check the categories
    {
        lclCheckCategories({ u"DE"_ustr, u""_ustr, u"EN"_ustr, u""_ustr, u""_ustr, u"FR"_ustr, u""_ustr},
                           lclGetCategories(xChartDoc)[0]->getValues());
        lclCheckCategories({ u"Berlin"_ustr, u"Munich"_ustr, u"Glasgow"_ustr, u"Liverpool"_ustr, u"London"_ustr, u"Nantes"_ustr, u"Paris"_ustr},
                           lclGetCategories(xChartDoc)[1]->getValues());
    }

    sheet::DataPilotFieldLayoutInfo aLayoutInfoValue;

    // Test case where we enable subtotals (auto) and set the outline subtotals at the bottom
    // We don't expect any change in data as every extra subtotal row should be ignored

    // Enable subtotals - set to auto
    uno::Sequence<sheet::GeneralFunction> aGeneralFunctionSequence{ sheet::GeneralFunction_AUTO };
    lclModifySubtotals(xDataPilotDescriptor, u"Country", aGeneralFunctionSequence);
    // Set Subtotals layout to bottom + add empty lines
    aLayoutInfoValue.AddEmptyLines = true;
    aLayoutInfoValue.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_BOTTOM;
    lclModifyLayoutInfo(xDataPilotDescriptor, u"Country", aLayoutInfoValue);

    // Check data is unchanged
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));
    {
        std::vector<double> aReference { 1116.0, 622.0, 298.0, 562.0, 1143.0, 1168.0, 768.0 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, lclGetLabel(xChartDoc, 0));
    }
    // Check categories
    {
        lclCheckCategories({ u"DE"_ustr, u""_ustr, u"EN"_ustr, u""_ustr, u""_ustr, u"FR"_ustr, u""_ustr},
                           lclGetCategories(xChartDoc)[0]->getValues());
        lclCheckCategories({ u"Berlin"_ustr, u"Munich"_ustr, u"Glasgow"_ustr, u"Liverpool"_ustr, u"London"_ustr, u"Nantes"_ustr, u"Paris"_ustr},
                           lclGetCategories(xChartDoc)[1]->getValues());
    }

    // Test case where we enable subtotals (auto) and set the outline subtotals at the top
    // We don't expect any change in data as every extra subtotal row should be ignored

    // Enable subtotals - set to auto
    aGeneralFunctionSequence.getArray()[0] = sheet::GeneralFunction_AUTO;
    lclModifySubtotals(xDataPilotDescriptor, u"Country", aGeneralFunctionSequence);
    // Set Subtotals layout to top + add empty lines
    aLayoutInfoValue.AddEmptyLines = true;
    aLayoutInfoValue.LayoutMode = sheet::DataPilotFieldLayoutMode::OUTLINE_SUBTOTALS_TOP;
    lclModifyLayoutInfo(xDataPilotDescriptor, u"Country", aLayoutInfoValue);

    // Check data is unchanged
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getNumberOfDataSeries(xChartDoc));
    {
        std::vector<double> aReference { 1116.0, 622.0, 298.0, 562.0, 1143.0, 1168.0, 768.0 };

        xSequence = getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0)->getData();
        lclCheckSequence(aReference, xSequence, 1E-4);

        CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, lclGetLabel(xChartDoc, 0));
    }
    // Check categories
    {
        lclCheckCategories({ u"DE"_ustr, u""_ustr, u"EN"_ustr, u""_ustr, u""_ustr, u"FR"_ustr, u""_ustr},
                           lclGetCategories(xChartDoc)[0]->getValues());
        lclCheckCategories({ u"Berlin"_ustr, u"Munich"_ustr, u"Glasgow"_ustr, u"Liverpool"_ustr, u"London"_ustr, u"Nantes"_ustr, u"Paris"_ustr},
                           lclGetCategories(xChartDoc)[1]->getValues());
    }
}

void PivotChartTest::testPivotChartWithDateRowField()
{
    // SETUP DATA and PIVOT TABLE

    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    OUString sPivotTableName(u"DataPilotTable"_ustr);

    table::CellRangeAddress sCellRangeAddress = lclCreateTestData(xSheetDoc);

    uno::Reference<sheet::XDataPilotTables> xDataPilotTables = lclGetDataPilotTables(0, xSheetDoc);

    uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor = xDataPilotTables->createDataPilotDescriptor();
    xDataPilotDescriptor->setSourceRange(sCellRangeAddress);

    lclModifyOrientation(xDataPilotDescriptor, u"Date", sheet::DataPilotFieldOrientation_ROW);
    lclModifyOrientation(xDataPilotDescriptor, u"City", sheet::DataPilotFieldOrientation_ROW);
    lclModifyOrientation(xDataPilotDescriptor, u"Country", sheet::DataPilotFieldOrientation_ROW);
    lclModifyOrientation(xDataPilotDescriptor, u"Type", sheet::DataPilotFieldOrientation_COLUMN);
    lclModifyOrientation(xDataPilotDescriptor, u"Sales T1", sheet::DataPilotFieldOrientation_DATA);
    lclModifyFunction(xDataPilotDescriptor, u"Sales T1", sheet::GeneralFunction_SUM);

    lclModifyColumnGrandTotal(xDataPilotDescriptor, true);
    lclModifyRowGrandTotal(xDataPilotDescriptor, true);

    xDataPilotTables->insertNewByName(sPivotTableName, table::CellAddress{1, 0, 0}, xDataPilotDescriptor);

    // TEST
    Reference<chart2::XChartDocument> xChartDoc;

    // Check we have the Pivot Table
    uno::Reference<sheet::XDataPilotTable> xDataPilotTable = lclGetPivotTableByName(1, sPivotTableName, mxComponent);
    CPPUNIT_ASSERT(xDataPilotTable.is());

    // refetch the XDataPilotDescriptor
    xDataPilotDescriptor.set(xDataPilotTable, uno::UNO_QUERY_THROW);

    // Check that we don't have any pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(1, mxComponent);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTablePivotCharts, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Create a new pivot chart
    xTablePivotCharts->addNewByName(u"PivotChart"_ustr, awt::Rectangle{ 9000, 9000, 21000, 18000 }, sPivotTableName);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Get the pivot chart document so we can access its data
    xChartDoc.set(getPivotChartDocFromSheet(xTablePivotCharts, 0));

    CPPUNIT_ASSERT(xChartDoc.is());

    // Check if Date category is date formatted.
    lclCheckCategories( { u"12/11/15"_ustr, u""_ustr, u""_ustr, u""_ustr, u""_ustr, u""_ustr, u"12/14/15"_ustr, u"12/17/15"_ustr, u"01/20/16"_ustr, u""_ustr, u""_ustr, u"03/21/17"_ustr },
                        lclGetCategories( xChartDoc )[0]->getValues() );
}


CPPUNIT_TEST_SUITE_REGISTRATION(PivotChartTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
