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

void lclModifyOrientation(uno::Reference<sheet::XDataPilotDescriptor> xDescriptor,
                          OUString sFieldName,
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

bool lclCheckSequence(std::vector<double> const & reference,
                      uno::Sequence<uno::Any> const & values,
                      double delta)
{
    if (reference.size() != size_t(values.getLength()))
        return false;

    for (size_t i = 0; i < reference.size(); ++i)
    {
        double value = values[i].get<double>();

        if (std::fabs(reference[i] - value) > delta)
        {
            printf ("Value %f is not the same as reference %f (delta %f)\n", value, reference[i], delta);
            return false;
        }
    }
    return true;
}

void PivotChartTest::testRoundtrip()
{
    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    load("/chart2/qa/extras/data/ods/", "PivotChart.ods");

    xChartDoc = Reference<chart2::XChartDocument>(getChartDocFromSheet(1, mxComponent), uno::UNO_QUERY);

    std::vector<double> aReference1 { 10162.033139, 16614.523063, 27944.146101 };
    OUString aExpectedLabel1("Exp.");

    std::vector<double> aReference2 { 101879.458079, 178636.929704, 314626.484864 };
    OUString aExpectedLabel2("Rev.");

    CPPUNIT_ASSERT(xChartDoc.is());
    {
        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0);
        xSequence = xDataSequence->getData();

        CPPUNIT_ASSERT(lclCheckSequence(aReference1, xSequence, 1E-4));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 0);
        xSequence = xLabelDataSequence->getData();

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel1, xSequence[0].get<OUString>());
    }
    {
        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1);
        xSequence = xDataSequence->getData();

        CPPUNIT_ASSERT(lclCheckSequence(aReference2, xSequence, 1E-4));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 1);
        xSequence = xLabelDataSequence->getData();

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel2, xSequence[0].get<OUString>());
    }

    reload("calc8");

    xChartDoc = Reference<chart2::XChartDocument>(getChartDocFromSheet(1, mxComponent), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    {
        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0);
        xSequence = xDataSequence->getData();

        CPPUNIT_ASSERT(lclCheckSequence(aReference1, xSequence, 1E-4));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 0);
        xSequence = xLabelDataSequence->getData();

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel1, xSequence[0].get<OUString>());
    }
    {
        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1);
        xSequence = xDataSequence->getData();

        CPPUNIT_ASSERT(lclCheckSequence(aReference2, xSequence, 1E-4));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 1);
        xSequence = xLabelDataSequence->getData();

        CPPUNIT_ASSERT_EQUAL(aExpectedLabel2, xSequence[0].get<OUString>());
    }
}


void PivotChartTest::testChangePivotTable()
{
    uno::Sequence<uno::Any> xSequence;
    Reference<chart2::XChartDocument> xChartDoc;

    load("/chart2/qa/extras/data/ods/", "PivotChart2.ods");

    xChartDoc = Reference<chart2::XChartDocument>(getChartDocFromSheet(1, mxComponent), uno::UNO_QUERY);

    CPPUNIT_ASSERT(xChartDoc.is());
    {
        std::vector<double> aReference { 10162.033139, 16614.523063, 27944.146101 };
        OUString aExpectedLabel("Exp.");

        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0);
        xSequence = xDataSequence->getData();
        CPPUNIT_ASSERT(lclCheckSequence(aReference, xSequence, 1E-4));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 0);
        xSequence = xLabelDataSequence->getData();
        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, xSequence[0].get<OUString>());
    }
    {
        std::vector<double> aReference { 101879.458079, 178636.929704, 314626.484864 };
        OUString aExpectedLabel("Rev.");

        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1);
        xSequence = xDataSequence->getData();
        CPPUNIT_ASSERT(lclCheckSequence(aReference, xSequence, 1E-4));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 1);
        xSequence = xLabelDataSequence->getData();
        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, xSequence[0].get<OUString>());
    }

    // modify the pivot table

    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSheetIndexAccess(xDoc->getSheets(), UNO_QUERY_THROW);
        uno::Any aAny = xSheetIndexAccess->getByIndex(1);
        uno::Reference<sheet::XSpreadsheet> xSheet;
        CPPUNIT_ASSERT(aAny >>= xSheet);
        uno::Reference<sheet::XDataPilotTablesSupplier> xDataPilotTablesSupplier(xSheet, uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XDataPilotTables> xDataPilotTables = xDataPilotTablesSupplier->getDataPilotTables();
        uno::Reference<sheet::XDataPilotTable> xDataPilotTable(xDataPilotTables->getByName("DataPilot1"), UNO_QUERY_THROW);
        uno::Reference<sheet::XDataPilotDescriptor> xDataPilotDescriptor(xDataPilotTable, UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xPilotIndexAccess(xDataPilotDescriptor->getDataPilotFields(), UNO_QUERY_THROW);

        lclModifyOrientation(xDataPilotDescriptor, "Service Month", sheet::DataPilotFieldOrientation_ROW);
        lclModifyOrientation(xDataPilotDescriptor, "Group Segment", sheet::DataPilotFieldOrientation_COLUMN);
        lclModifyOrientation(xDataPilotDescriptor, "Rev.", sheet::DataPilotFieldOrientation_HIDDEN);
    }

    // check again

    xChartDoc = Reference<chart2::XChartDocument>(getChartDocFromSheet(1, mxComponent), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    {
        std::vector<double> aReference { 2855.559, 1780.326, 2208.713, 2130.064, 1187.371 };
        OUString aExpectedLabel("Big");

        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 0);
        xSequence = xDataSequence->getData();
        CPPUNIT_ASSERT(lclCheckSequence(aReference, xSequence, 1E-3));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 0);
        xSequence = xLabelDataSequence->getData();
        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, xSequence[0].get<OUString>());
    }
    {
        std::vector<double> aReference { 4098.908, 2527.286, 4299.716, 2362.225, 3326.389 };
        OUString aExpectedLabel("Medium");

        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 1);
        xSequence = xDataSequence->getData();
        CPPUNIT_ASSERT(lclCheckSequence(aReference, xSequence, 1E-3));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 1);
        xSequence = xLabelDataSequence->getData();
        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, xSequence[0].get<OUString>());
    }
    {
        std::vector<double> aReference { 4926.303, 5684.060, 4201.398, 7290.795, 5841.591 };
        OUString aExpectedLabel("Small");

        Reference<chart2::data::XDataSequence> xDataSequence = getDataSequenceFromDocByRole(xChartDoc, "values-y", 2);
        xSequence = xDataSequence->getData();
        CPPUNIT_ASSERT(lclCheckSequence(aReference, xSequence, 1E-3));

        Reference<chart2::data::XDataSequence> xLabelDataSequence = getLabelDataSequenceFromDoc(xChartDoc, 2);
        xSequence = xLabelDataSequence->getData();
        CPPUNIT_ASSERT_EQUAL(aExpectedLabel, xSequence[0].get<OUString>());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PivotChartTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
