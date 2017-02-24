/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/dispatch.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <unonames.hxx>

#include "tabvwsh.hxx"
#include "docsh.hxx"

#include "sc.hrc"

using namespace css;

namespace sc_apitest {

class ScConditionalFormatTest : public CalcUnoApiTest
{
public:
    ScConditionalFormatTest();

    virtual void tearDown() override;

    uno::Reference< uno::XInterface > init();
    void testCondFormat();

    CPPUNIT_TEST_SUITE(ScConditionalFormatTest);
    CPPUNIT_TEST(testCondFormat);
    CPPUNIT_TEST_SUITE_END();
private:

    uno::Reference< lang::XComponent > mxComponent;
};

ScConditionalFormatTest::ScConditionalFormatTest()
    : CalcUnoApiTest("sc/qa/unit/data/ods")
{
}

uno::Reference< uno::XInterface > ScConditionalFormatTest::init()
{
    // get the test file
    OUString aFileURL;
    createFileURL("new_cond_format_test.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    // get the first sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    return xSheet;
}

void ScConditionalFormatTest::testCondFormat()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xProps;
    uno::Reference< sheet::XSheetConditionalEntries > xSheetConditionalEntries;

    xProps.set(aSheet->getCellRangeByPosition(1, 1, 1, 21), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSheetConditionalEntries->getCount());

    uno::Sequence< beans::PropertyValue > aPropertyValueList(5);
    aPropertyValueList[0].Name = SC_UNONAME_STYLENAME;
    aPropertyValueList[0].Value <<= OUString("Result2");
    aPropertyValueList[1].Name = SC_UNONAME_FORMULA1;
    aPropertyValueList[1].Value <<= OUString("$Sheet1.$B$2");
    aPropertyValueList[2].Name = SC_UNONAME_FORMULA2;
    aPropertyValueList[2].Value <<= OUString("$Sheet1.$A$2");
    aPropertyValueList[3].Name = SC_UNONAME_OPERATOR;
    aPropertyValueList[3].Value <<= sheet::ConditionOperator_EQUAL;
    aPropertyValueList[4].Name = SC_UNONAME_SOURCEPOS;
    aPropertyValueList[4].Value <<= table::CellAddress(0, 0, 16);
    xSheetConditionalEntries->addNew(aPropertyValueList);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSheetConditionalEntries->getCount());
    xProps->setPropertyValue(SC_UNONAME_CONDFMT, uno::makeAny(xSheetConditionalEntries));

    xProps.set(aSheet->getCellByPosition(0, 16), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSheetConditionalEntries->getCount());

    // clear conditional formatting in cell $B$17
    xProps.set(aSheet->getCellByPosition(1, 16), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSheetConditionalEntries->getCount());
    xSheetConditionalEntries->clear();
    xProps->setPropertyValue(SC_UNONAME_CONDFMT, uno::makeAny(xSheetConditionalEntries));

    xProps.set(aSheet->getCellByPosition(1, 16), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    // This was 1 before - conditional formats were not removed
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSheetConditionalEntries->getCount());
}

void ScConditionalFormatTest::tearDown()
{
    if (mxComponent.is())
    {
        closeDocument(mxComponent);
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScConditionalFormatTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
