/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/xsheetconditionalentry.hxx>
#include <test/sheet/xsheetcondition.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <unonames.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>

#include <comphelper/propertyvalue.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScTableConditionalEntryObj : public UnoApiTest,
                                   public apitest::XSheetConditionalEntry,
                                   public apitest::XSheetCondition
{
public:
    ScTableConditionalEntryObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScTableConditionalEntryObj);

    // XSheetConditionalEntry
    CPPUNIT_TEST(testGetSetStyleName);

    // XSheetCondition
    CPPUNIT_TEST(testGetSetFormula1);
    CPPUNIT_TEST(testGetSetFormula2);
    CPPUNIT_TEST(testGetSetOperator);
    CPPUNIT_TEST(testGetSetSourcePosition);

    CPPUNIT_TEST_SUITE_END();
};

ScTableConditionalEntryObj::ScTableConditionalEntryObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScTableConditionalEntryObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet->getCellByPosition(5, 5)->setValue(15);
    xSheet->getCellByPosition(1, 4)->setValue(10);
    xSheet->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Reference<beans::XPropertySet> xPropSet(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetConditionalEntries> xSheetConditionalEntries;
    xSheetConditionalEntries.set(xPropSet->getPropertyValue(SC_UNONAME_CONDFMT),
                                 uno::UNO_QUERY_THROW);

    uno::Sequence<beans::PropertyValue> aPropValue{
        comphelper::makePropertyValue(SC_UNONAME_STYLENAME, u"Result2"_ustr),
        comphelper::makePropertyValue(SC_UNONAME_FORMULA1, u"$Sheet1.$B$5"_ustr),
        comphelper::makePropertyValue(SC_UNONAME_FORMULA2, u""_ustr),
        comphelper::makePropertyValue(SC_UNONAME_OPERATOR, sheet::ConditionOperator_EQUAL),
        comphelper::makePropertyValue(SC_UNONAME_SOURCEPOS, table::CellAddress(0, 1, 5))
    };
    xSheetConditionalEntries->addNew(aPropValue);

    uno::Reference<sheet::XSheetConditionalEntry> xSheetConditionalEntry(
        xSheetConditionalEntries->getByIndex(0), uno::UNO_QUERY_THROW);
    return xSheetConditionalEntry;
}

void ScTableConditionalEntryObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableConditionalEntryObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
