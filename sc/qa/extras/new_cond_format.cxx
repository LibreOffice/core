/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XConditionalFormats.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/DataBarAxis.hpp>
#include <com/sun/star/sheet/XDataBarEntry.hpp>
#include <com/sun/star/sheet/DataBarEntryType.hpp>
#include <com/sun/star/sheet/ColorScaleEntryType.hpp>
#include <com/sun/star/sheet/XColorScaleEntry.hpp>
#include <unonames.hxx>

using namespace css;

namespace sc_apitest {

#define NUMBER_OF_TESTS 5

class ScConditionalFormatTest : public CalcUnoApiTest
{
public:
    ScConditionalFormatTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    uno::Reference< uno::XInterface > init(sal_Int32 nIndex = 0);
    void testRequestCondFormatListFromSheet();
    void testCondFormatListProperties();
    void testCondFormatListFormats();
    void testCondFormatProperties();
    void testCondFormatXIndex();
    void testDataBarProperties();
    void testColorScaleProperties();

    CPPUNIT_TEST_SUITE(ScConditionalFormatTest);
    CPPUNIT_TEST(testRequestCondFormatListFromSheet);
    CPPUNIT_TEST(testCondFormatListProperties);
    CPPUNIT_TEST(testCondFormatListFormats);
    CPPUNIT_TEST(testCondFormatProperties);
    CPPUNIT_TEST(testCondFormatXIndex);
    CPPUNIT_TEST(testDataBarProperties);
    CPPUNIT_TEST(testColorScaleProperties);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScConditionalFormatTest::nTest = 0;
uno::Reference< lang::XComponent > ScConditionalFormatTest::mxComponent;

ScConditionalFormatTest::ScConditionalFormatTest()
    : CalcUnoApiTest("sc/qa/extras/testdocuments/")
{
}

uno::Reference< uno::XInterface > ScConditionalFormatTest::init(sal_Int32 nIndex)
{
    if(!mxComponent.is())
    {
        // get the test file
        OUString aFileURL;
        createFileURL(OUString("new_cond_format_api.ods"), aFileURL);
        mxComponent = loadFromDesktop(aFileURL);
    }
    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    // get the first sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(nIndex), uno::UNO_QUERY_THROW);

    return xSheet;
}

void ScConditionalFormatTest::testRequestCondFormatListFromSheet()
{
    uno::Reference<sheet::XSpreadsheet> xSheet(init(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
    uno::Any aAny = xProps->getPropertyValue("ConditionalFormats");
    uno::Reference<sheet::XConditionalFormats> xCondFormats;
    CPPUNIT_ASSERT(aAny >>= xCondFormats);
    CPPUNIT_ASSERT(xCondFormats.is());
}

namespace {

uno::Reference<sheet::XConditionalFormats> getConditionalFormatList(uno::Reference<uno::XInterface> xInterface)
{
    uno::Reference<sheet::XSpreadsheet> xSheet(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
    uno::Any aAny = xProps->getPropertyValue("ConditionalFormats");
    uno::Reference<sheet::XConditionalFormats> xCondFormats;
    CPPUNIT_ASSERT(aAny >>= xCondFormats);
    CPPUNIT_ASSERT(xCondFormats.is());

    return xCondFormats;
}

}

void ScConditionalFormatTest::testCondFormatListProperties()
{
    uno::Reference<sheet::XConditionalFormats> xCondFormat =
        getConditionalFormatList(init());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xCondFormat->getLength());
}

void ScConditionalFormatTest::testCondFormatListFormats()
{
    uno::Reference<sheet::XConditionalFormats> xCondFormatList =
        getConditionalFormatList(init());

    uno::Sequence<uno::Reference<sheet::XConditionalFormat> > xCondFormats =
        xCondFormatList->getConditionalFormats();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xCondFormats.getLength());
    for (sal_Int32 i = 0, n = xCondFormats.getLength(); i < n; ++i)
    {
        CPPUNIT_ASSERT(xCondFormats[i].is());
    }
}

void ScConditionalFormatTest::testCondFormatProperties()
{
    uno::Reference<sheet::XConditionalFormats> xCondFormatList =
        getConditionalFormatList(init(1));

    uno::Sequence<uno::Reference<sheet::XConditionalFormat> > xCondFormats =
        xCondFormatList->getConditionalFormats();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCondFormats.getLength());

    uno::Reference<sheet::XConditionalFormat> xCondFormat = xCondFormats[0];
    CPPUNIT_ASSERT(xCondFormat.is());
    uno::Reference<beans::XPropertySet> xPropSet(xCondFormat, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("Range");
    uno::Reference<sheet::XSheetCellRanges> xCellRanges;
    CPPUNIT_ASSERT(aAny >>= xCellRanges);
    CPPUNIT_ASSERT(xCellRanges.is());
    uno::Sequence<table::CellRangeAddress> aRanges = xCellRanges->getRangeAddresses();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRanges.getLength());
    table::CellRangeAddress aRange = aRanges[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aRange.Sheet);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRange.StartColumn);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aRange.StartRow);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aRange.EndColumn);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aRange.EndRow);
}

void ScConditionalFormatTest::testCondFormatXIndex()
{
    uno::Reference<sheet::XConditionalFormats> xCondFormatList =
        getConditionalFormatList(init(1));

    uno::Sequence<uno::Reference<sheet::XConditionalFormat> > xCondFormats =
        xCondFormatList->getConditionalFormats();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCondFormats.getLength());

    uno::Reference<sheet::XConditionalFormat> xCondFormat = xCondFormats[0];
    CPPUNIT_ASSERT(xCondFormat.is());

    uno::Type aType = xCondFormat->getElementType();
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.beans.XPropertySet"), aType.getTypeName());

    CPPUNIT_ASSERT(xCondFormat->hasElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xCondFormat->getCount());

    uno::Any aAny = xCondFormat->getByIndex(0);
    CPPUNIT_ASSERT(aAny.hasValue());
}

namespace {

void testAxisPosition(uno::Reference<beans::XPropertySet> xPropSet, sal_Int32 ePos)
{
    sal_Int32 eAxisPos;
    uno::Any aAny = xPropSet->getPropertyValue("AxisPosition");
    CPPUNIT_ASSERT(aAny >>= eAxisPos);
    CPPUNIT_ASSERT_EQUAL(ePos, eAxisPos);
}

void testShowValue(uno::Reference<beans::XPropertySet> xPropSet, bool bShowVal)
{
    bool bShow;
    uno::Any aAny = xPropSet->getPropertyValue("ShowValue");
    CPPUNIT_ASSERT(aAny >>= bShow);
    CPPUNIT_ASSERT_EQUAL(bShowVal, bShow);
}

void testUseGradient(uno::Reference<beans::XPropertySet> xPropSet, bool bUseGradient)
{
    bool bGradient;
    uno::Any aAny = xPropSet->getPropertyValue("UseGradient");
    CPPUNIT_ASSERT(aAny >>= bGradient);
    CPPUNIT_ASSERT_EQUAL(bUseGradient, bGradient);
}

void testPositiveColor(uno::Reference<beans::XPropertySet> xPropSet, Color aColor)
{
    sal_Int32 nColor = 0;
    uno::Any aAny = xPropSet->getPropertyValue("Color");
    CPPUNIT_ASSERT(aAny >>= nColor);
    CPPUNIT_ASSERT_EQUAL(aColor.GetColor(), sal_uInt32(nColor));
}

void testNegativeColor(uno::Reference<beans::XPropertySet> xPropSet, Color aColor)
{
    sal_Int32 nColor = 0;
    uno::Any aAny = xPropSet->getPropertyValue("NegativeColor");
    CPPUNIT_ASSERT(aAny >>= nColor);
    CPPUNIT_ASSERT_EQUAL(aColor.GetColor(), sal_uInt32(nColor));
}

void testAxisColor(uno::Reference<beans::XPropertySet> xPropSet, Color aColor)
{
    sal_Int32 nColor = 0;
    uno::Any aAny = xPropSet->getPropertyValue("AxisColor");
    CPPUNIT_ASSERT(aAny >>= nColor);
    CPPUNIT_ASSERT_EQUAL(aColor.GetColor(), sal_uInt32(nColor));
}

void testDataBarEntryValue(uno::Reference<sheet::XDataBarEntry> xEntry,
        const OUString& rExpectedValue, sal_Int32 nType)
{
    switch (nType)
    {
        case sheet::DataBarEntryType::DATABAR_VALUE:
        case sheet::DataBarEntryType::DATABAR_PERCENT:
        case sheet::DataBarEntryType::DATABAR_PERCENTILE:
        case sheet::DataBarEntryType::DATABAR_FORMULA:
        {
            OUString aString = xEntry->getFormula();
            CPPUNIT_ASSERT_EQUAL(rExpectedValue, aString);
        }
        break;
        default:
        break;
    }
}

void testDataBarEntries(uno::Reference<beans::XPropertySet> xPropSet,
        const OUString& rExpectedMinString, sal_Int32 nExpectedMinType,
        const OUString& rExpectedMaxString, sal_Int32 nExpectedMaxType)
{
    uno::Any aAny = xPropSet->getPropertyValue("DataBarEntries");
    uno::Sequence<uno::Reference<sheet::XDataBarEntry> > aEntries;
    CPPUNIT_ASSERT(aAny >>= aEntries);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aEntries.getLength());

    sal_Int32 nMinType = aEntries[0]->getType();
    CPPUNIT_ASSERT_EQUAL(nExpectedMinType, nMinType);

    sal_Int32 nMaxType = aEntries[1]->getType();
    CPPUNIT_ASSERT_EQUAL(nExpectedMaxType, nMaxType);

    testDataBarEntryValue(aEntries[0], rExpectedMinString, nMinType);
    testDataBarEntryValue(aEntries[1], rExpectedMaxString, nMaxType);
}

}

void ScConditionalFormatTest::testDataBarProperties()
{
    uno::Reference<sheet::XConditionalFormats> xCondFormatList =
        getConditionalFormatList(init(2));

    uno::Sequence<uno::Reference<sheet::XConditionalFormat> > xCondFormats =
        xCondFormatList->getConditionalFormats();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCondFormats.getLength());

    uno::Reference<sheet::XConditionalFormat> xCondFormat = xCondFormats[0];
    CPPUNIT_ASSERT(xCondFormat.is());

    uno::Type aType = xCondFormat->getElementType();
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.beans.XPropertySet"), aType.getTypeName());

    CPPUNIT_ASSERT(xCondFormat->hasElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xCondFormat->getCount());

    uno::Reference<beans::XPropertySet> xPropSet;
    {
        uno::Any aAny = xCondFormat->getByIndex(0);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testAxisPosition(xPropSet, sheet::DataBarAxis::AXIS_AUTOMATIC);
        testShowValue(xPropSet, true);
        testUseGradient(xPropSet, true);
        testPositiveColor(xPropSet, COL_LIGHTBLUE);
        testNegativeColor(xPropSet, COL_LIGHTRED);
        testAxisColor(xPropSet, COL_BLACK);
        testDataBarEntries(xPropSet, "", sheet::DataBarEntryType::DATABAR_AUTO,
                "", sheet::DataBarEntryType::DATABAR_MAX);
    }
    {
        uno::Any aAny = xCondFormat->getByIndex(1);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testAxisPosition(xPropSet, sheet::DataBarAxis::AXIS_AUTOMATIC);
        testShowValue(xPropSet, true);
        testUseGradient(xPropSet, true);
        testPositiveColor(xPropSet, COL_LIGHTBLUE);
        testNegativeColor(xPropSet, COL_LIGHTRED);
        testAxisColor(xPropSet, COL_BLACK);
        testDataBarEntries(xPropSet, "", sheet::DataBarEntryType::DATABAR_MIN,
                "90", sheet::DataBarEntryType::DATABAR_PERCENTILE);
    }
    {
        uno::Any aAny = xCondFormat->getByIndex(2);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testAxisPosition(xPropSet, sheet::DataBarAxis::AXIS_AUTOMATIC);
        testShowValue(xPropSet, true);
        testUseGradient(xPropSet, true);
        testPositiveColor(xPropSet, COL_LIGHTBLUE);
        testNegativeColor(xPropSet, COL_LIGHTRED);
        testAxisColor(xPropSet, COL_BLACK);
        testDataBarEntries(xPropSet, "2", sheet::DataBarEntryType::DATABAR_VALUE,
                "80", sheet::DataBarEntryType::DATABAR_PERCENT);
    }
    {
        uno::Any aAny = xCondFormat->getByIndex(3);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testAxisPosition(xPropSet, sheet::DataBarAxis::AXIS_AUTOMATIC);
        testShowValue(xPropSet, true);
        testUseGradient(xPropSet, true);
        testPositiveColor(xPropSet, COL_LIGHTBLUE);
        testNegativeColor(xPropSet, COL_LIGHTRED);
        testAxisColor(xPropSet, COL_BLACK);
        /*
         * TODO: implement FORMULA
        testDataBarEntries(xPropSet, "=A1", sheet::DataBarEntryType::DATABAR_FORMULA,
                "", sheet::DataBarEntryType::DATABAR_AUTO);
                */
    }
    {
        uno::Any aAny = xCondFormat->getByIndex(4);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testAxisPosition(xPropSet, sheet::DataBarAxis::AXIS_MIDDLE);
        testShowValue(xPropSet, true);
        testUseGradient(xPropSet, false);
        testPositiveColor(xPropSet, sal_uInt32(10092390));
        testNegativeColor(xPropSet, sal_uInt32(52428));
        testAxisColor(xPropSet, sal_uInt32(16777113));
        testDataBarEntries(xPropSet, "", sheet::DataBarEntryType::DATABAR_AUTO,
                "", sheet::DataBarEntryType::DATABAR_AUTO);
    }
}

namespace {

void testColorScaleEntry(uno::Reference<sheet::XColorScaleEntry> xEntry,
        sal_Int32 nType, const OUString& rString, sal_uInt32 nColor)
{
    CPPUNIT_ASSERT_EQUAL(nType, xEntry->getType());
    CPPUNIT_ASSERT_EQUAL(nColor, sal_uInt32(xEntry->getColor()));
    switch (nType)
    {
        case sheet::ColorScaleEntryType::COLORSCALE_VALUE:
        case sheet::ColorScaleEntryType::COLORSCALE_PERCENT:
        case sheet::ColorScaleEntryType::COLORSCALE_PERCENTILE:
        // case sheet::ColorScaleEntryType::COLORSCALE_FORMULA:
        {
            CPPUNIT_ASSERT_EQUAL(rString, xEntry->getFormula());
        }
        break;
        default:
        break;
    }
}

void testColorScaleEntries(uno::Reference<beans::XPropertySet> xPropSet, sal_Int32 nEntries,
        sal_Int32 nMinType, const OUString& rMinString, sal_uInt32 nMinColor,
        sal_Int32 nMediumType, const OUString& rMediumString, sal_uInt32 nMediumColor,
        sal_Int32 nMaxType, const OUString& rMaxString, sal_uInt32 nMaxColor)
{
    uno::Any aAny = xPropSet->getPropertyValue("ColorScaleEntries");
    CPPUNIT_ASSERT(aAny.hasValue());
    uno::Sequence<uno::Reference<sheet::XColorScaleEntry> > aEntries;
    CPPUNIT_ASSERT(aAny >>= aEntries);

    CPPUNIT_ASSERT_EQUAL(nEntries, aEntries.getLength());
    testColorScaleEntry(aEntries[0], nMinType, rMinString, nMinColor);
    size_t nMaxEntry = 1;
    if (nEntries == 3)
    {
        nMaxEntry = 2;
        testColorScaleEntry(aEntries[1], nMediumType, rMediumString, nMediumColor);
    }
    testColorScaleEntry(aEntries[nMaxEntry], nMaxType, rMaxString, nMaxColor);
}

}

void ScConditionalFormatTest::testColorScaleProperties()
{
    uno::Reference<sheet::XConditionalFormats> xCondFormatList =
        getConditionalFormatList(init(3));

    uno::Sequence<uno::Reference<sheet::XConditionalFormat> > xCondFormats =
        xCondFormatList->getConditionalFormats();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCondFormats.getLength());

    uno::Reference<sheet::XConditionalFormat> xCondFormat = xCondFormats[0];
    CPPUNIT_ASSERT(xCondFormat.is());

    uno::Type aType = xCondFormat->getElementType();
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.beans.XPropertySet"), aType.getTypeName());

    CPPUNIT_ASSERT(xCondFormat->hasElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xCondFormat->getCount());

    uno::Reference<beans::XPropertySet> xPropSet;
    {
        uno::Any aAny = xCondFormat->getByIndex(0);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testColorScaleEntries(xPropSet, 3, sheet::ColorScaleEntryType::COLORSCALE_MIN, "", sal_uInt32(16777113),
                sheet::ColorScaleEntryType::COLORSCALE_PERCENTILE, "50", sal_uInt32(16737792),
                sheet::ColorScaleEntryType::COLORSCALE_MAX, "", sal_uInt32(16724787));
    }
    {
        uno::Any aAny = xCondFormat->getByIndex(1);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testColorScaleEntries(xPropSet, 3, sheet::ColorScaleEntryType::COLORSCALE_VALUE, "0", sal_uInt32(16711680),
                sheet::ColorScaleEntryType::COLORSCALE_PERCENTILE, "50", sal_uInt32(10092390),
                sheet::ColorScaleEntryType::COLORSCALE_PERCENT, "90", sal_uInt32(255));
    }
    {
        uno::Any aAny = xCondFormat->getByIndex(2);
        CPPUNIT_ASSERT(aAny.hasValue());
        CPPUNIT_ASSERT(aAny >>= xPropSet);
        testColorScaleEntries(xPropSet, 2, sheet::ColorScaleEntryType::COLORSCALE_FORMULA, "=A1", COL_WHITE,
                sheet::ColorScaleEntryType::COLORSCALE_PERCENTILE, "not used", sal_uInt32(1),
                sheet::ColorScaleEntryType::COLORSCALE_VALUE, "10", COL_BLACK);
    }
}

void ScConditionalFormatTest::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScConditionalFormatTest::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScConditionalFormatTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
