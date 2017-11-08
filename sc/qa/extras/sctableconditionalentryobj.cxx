/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/xsheetconditionalentry.hxx>
#include <test/sheet/xsheetcondition.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <unonames.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
#define NUMBER_OF_TESTS 5

class ScTableConditionalEntryObj : public CalcUnoApiTest,
                                   public apitest::XSheetConditionalEntry,
                                   public apitest::XSheetCondition
{
public:
    ScTableConditionalEntryObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScTableConditionalEntryObj);

    // XSheetConditionalEntry
    CPPUNIT_TEST(testGetSetStyleName);

    // XSheetCondition
    CPPUNIT_TEST(testGetSetFormula1);
    CPPUNIT_TEST(testGetSetFormula2);
    CPPUNIT_TEST(testGetSetOperator);
    CPPUNIT_TEST(testGetSetSourcePosition);

    CPPUNIT_TEST_SUITE_END();

private:
    static sal_Int32 nTest;
    static uno::Reference<lang::XComponent> mxComponent;
};

sal_Int32 ScTableConditionalEntryObj::nTest = 0;
uno::Reference<lang::XComponent> ScTableConditionalEntryObj::mxComponent;

ScTableConditionalEntryObj::ScTableConditionalEntryObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference<uno::XInterface> ScTableConditionalEntryObj::init()
{
    // create a calc document
    if (!mxComponent.is())
        mxComponent = loadFromDesktop("private:factory/scalc");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet->getCellByPosition(5, 5)->setValue(15);
    xSheet->getCellByPosition(1, 4)->setValue(10);
    xSheet->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Reference<beans::XPropertySet> xPropSet(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetConditionalEntries> xSheetConditionalEntries;
    xSheetConditionalEntries.set(xPropSet->getPropertyValue(SC_UNONAME_CONDFMT),
                                 uno::UNO_QUERY_THROW);

    uno::Sequence<beans::PropertyValue> aPropValue(5);
    aPropValue[0].Name = SC_UNONAME_STYLENAME;
    aPropValue[0].Value <<= OUString("Result2");
    aPropValue[1].Name = SC_UNONAME_FORMULA1;
    aPropValue[1].Value <<= OUString("$Sheet1.$B$5");
    aPropValue[2].Name = SC_UNONAME_FORMULA2;
    aPropValue[2].Value <<= OUString("");
    aPropValue[3].Name = SC_UNONAME_OPERATOR;
    aPropValue[3].Value <<= sheet::ConditionOperator_EQUAL;
    aPropValue[4].Name = SC_UNONAME_SOURCEPOS;
    aPropValue[4].Value <<= table::CellAddress(0, 1, 5);
    xSheetConditionalEntries->addNew(aPropValue);

    uno::Reference<sheet::XSheetConditionalEntry> xSheetConditionalEntry(
        xSheetConditionalEntries->getByIndex(0), uno::UNO_QUERY_THROW);
    return xSheetConditionalEntry;
}

void ScTableConditionalEntryObj::setUp()
{
    nTest++;
    CPPUNIT_ASSERT(nTest <= NUMBER_OF_TESTS);
    CalcUnoApiTest::setUp();
}

void ScTableConditionalEntryObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableConditionalEntryObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
