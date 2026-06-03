/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <conditio.hxx>
#include <helper/qahelper.hxx>
#include <tools/color.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ColorScaleEntryType.hpp>
#include <com/sun/star/sheet/ConditionEntryType.hpp>
#include <com/sun/star/sheet/XColorScaleEntry.hpp>
#include <com/sun/star/sheet/XConditionalFormats.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace css;

namespace sc_apitest
{
class ScColorScaleTest : public ScModelTestBase
{
public:
    ScColorScaleTest();

    void setUp() override;
    void tearDown() override;

    void testCreateColorScale2();
    void testCreateColorScale3();

    CPPUNIT_TEST_SUITE(ScColorScaleTest);
    CPPUNIT_TEST(testCreateColorScale2);
    CPPUNIT_TEST(testCreateColorScale3);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XMultiServiceFactory> m_xMSF;
    uno::Reference<sheet::XSpreadsheetDocument> m_xDoc;
    uno::Reference<container::XIndexAccess> m_xSheets;
    uno::Reference<sheet::XSpreadsheet> m_xSheet;
    uno::Reference<beans::XPropertySet> m_xSheetProperties;
    uno::Reference<sheet::XConditionalFormats> m_xConditionalFormats;
};

ScColorScaleTest::ScColorScaleTest()
    : ScModelTestBase(u"sc/qa/extras/testdocuments/"_ustr)
{
}

void ScColorScaleTest::setUp()
{
    ScModelTestBase::setUp();

    createScDoc();

    m_xMSF.set(mxComponent, uno::UNO_QUERY_THROW);
    m_xDoc.set(mxComponent, uno::UNO_QUERY_THROW);
    m_xSheets.set(m_xDoc->getSheets(), uno::UNO_QUERY_THROW);
    m_xSheet.set(m_xSheets->getByIndex(0), uno::UNO_QUERY_THROW);
    m_xSheetProperties.set(m_xSheet, uno::UNO_QUERY_THROW);
    m_xConditionalFormats.set(m_xSheetProperties->getPropertyValue("ConditionalFormats"),
                              uno::UNO_QUERY_THROW);
}

void ScColorScaleTest::tearDown()
{
    m_xConditionalFormats.clear();
    m_xSheetProperties.clear();
    m_xSheet.clear();
    m_xSheets.clear();
    m_xDoc.clear();
    m_xMSF.clear();

    ScModelTestBase::tearDown();
}

void ScColorScaleTest::testCreateColorScale2()
{
    // Tests creating a COLORSCALE2. This should allocate two entries that we can modify. See
    // tdf#171950. This also tests tdf#172333 as a side effect.

    // Set color scale conditional formatting on A1:A2
    uno::Reference<sheet::XSheetCellRangeContainer> xRanges(
        m_xMSF->createInstance("com.sun.star.sheet.SheetCellRanges"), uno::UNO_QUERY_THROW);
    table::CellRangeAddress a1a2 = { 0, 0, 0, 0, 1 };
    xRanges->addRangeAddress(a1a2, false);
    m_xConditionalFormats->createByRange(xRanges);
    uno::Reference<sheet::XConditionalFormat> xConditionalFormat
        = m_xConditionalFormats->getConditionalFormats()[0];
    xConditionalFormat->createEntry(sheet::ConditionEntryType::COLORSCALE2, 0);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xConditionalFormat->getCount());

    uno::Reference<beans::XPropertySet> xEntry(xConditionalFormat->getByIndex(0),
                                               uno::UNO_QUERY_THROW);
    uno::Sequence<uno::Reference<sheet::XColorScaleEntry>> aColorScales;
    CPPUNIT_ASSERT(xEntry->getPropertyValue("ColorScaleEntries") >>= aColorScales);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aColorScales.getLength());

    aColorScales[0]->setColor(sal_Int32(COL_MAGENTA));
    aColorScales[0]->setType(sheet::ColorScaleEntryType::COLORSCALE_MIN);

    aColorScales[1]->setColor(sal_Int32(COL_BROWN));
    aColorScales[1]->setType(sheet::ColorScaleEntryType::COLORSCALE_MAX);

    // Put 50 in A1 to make a minimum value
    insertStringToCell(u"A1"_ustr, u"50");
    // Put 100 in A2 to make a maximum value
    insertStringToCell(u"A2"_ustr, u"100");

    // A1 should be magenta
    {
        ScConditionalFormat* pCondFormat = getScDoc()->GetCondFormat(0, 0, 0);
        ScRefCellValue aCell(*getScDoc(), ScAddress(0, 0, 0));
        Color aColor = pCondFormat->GetData(aCell, ScAddress(0, 0, 0)).mxColorScale.value();
        CPPUNIT_ASSERT_EQUAL(COL_MAGENTA, aColor);
    }
    // A2 should be brown
    {
        ScConditionalFormat* pCondFormat = getScDoc()->GetCondFormat(0, 1, 0);
        ScRefCellValue aCell(*getScDoc(), ScAddress(0, 1, 0));
        Color aColor = pCondFormat->GetData(aCell, ScAddress(0, 1, 0)).mxColorScale.value();
        CPPUNIT_ASSERT_EQUAL(COL_BROWN, aColor);
    }
}

void ScColorScaleTest::testCreateColorScale3()
{
    // Tests creating a COLORSCALE3. This should allocate three entries that we can modify. See
    // tdf#171950. This also tests tdf#172333 as a side effect.

    // Set color scale conditional formatting on A1:A3
    uno::Reference<sheet::XSheetCellRangeContainer> xRanges(
        m_xMSF->createInstance("com.sun.star.sheet.SheetCellRanges"), uno::UNO_QUERY_THROW);
    table::CellRangeAddress a1a3 = { 0, 0, 0, 0, 2 };
    xRanges->addRangeAddress(a1a3, false);
    m_xConditionalFormats->createByRange(xRanges);
    uno::Reference<sheet::XConditionalFormat> xConditionalFormat
        = m_xConditionalFormats->getConditionalFormats()[0];
    xConditionalFormat->createEntry(sheet::ConditionEntryType::COLORSCALE3, 0);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xConditionalFormat->getCount());

    uno::Reference<beans::XPropertySet> xEntry(xConditionalFormat->getByIndex(0),
                                               uno::UNO_QUERY_THROW);
    uno::Sequence<uno::Reference<sheet::XColorScaleEntry>> aColorScales;
    CPPUNIT_ASSERT(xEntry->getPropertyValue("ColorScaleEntries") >>= aColorScales);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aColorScales.getLength());

    aColorScales[0]->setColor(sal_Int32(COL_MAGENTA));
    aColorScales[0]->setType(sheet::ColorScaleEntryType::COLORSCALE_MIN);

    aColorScales[1]->setColor(sal_Int32(COL_BROWN));
    aColorScales[1]->setType(sheet::ColorScaleEntryType::COLORSCALE_VALUE);
    aColorScales[1]->setFormula("75");

    aColorScales[2]->setColor(sal_Int32(COL_RED));
    aColorScales[2]->setType(sheet::ColorScaleEntryType::COLORSCALE_MAX);

    // Put 50 in A1 to make a minimum value
    insertStringToCell(u"A1"_ustr, u"50");
    // Put 75 in A2 to make a middle value
    insertStringToCell(u"A2"_ustr, u"75");
    // Put 100 in A3 to make a maximum value
    insertStringToCell(u"A3"_ustr, u"100");

    // A1 should be magenta
    {
        ScConditionalFormat* pCondFormat = getScDoc()->GetCondFormat(0, 0, 0);
        ScRefCellValue aCell(*getScDoc(), ScAddress(0, 0, 0));
        Color aColor = pCondFormat->GetData(aCell, ScAddress(0, 0, 0)).mxColorScale.value();
        CPPUNIT_ASSERT_EQUAL(COL_MAGENTA, aColor);
    }
    // A2 should be brown
    {
        ScConditionalFormat* pCondFormat = getScDoc()->GetCondFormat(0, 1, 0);
        ScRefCellValue aCell(*getScDoc(), ScAddress(0, 1, 0));
        Color aColor = pCondFormat->GetData(aCell, ScAddress(0, 1, 0)).mxColorScale.value();
        CPPUNIT_ASSERT_EQUAL(COL_BROWN, aColor);
    }
    // A3 should be red
    {
        ScConditionalFormat* pCondFormat = getScDoc()->GetCondFormat(0, 2, 0);
        ScRefCellValue aCell(*getScDoc(), ScAddress(0, 2, 0));
        Color aColor = pCondFormat->GetData(aCell, ScAddress(0, 2, 0)).mxColorScale.value();
        CPPUNIT_ASSERT_EQUAL(COL_RED, aColor);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScColorScaleTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
