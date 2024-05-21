/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/propertyvalue.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScIndexEnumeration_TableConditionalEntryEnumeration : public UnoApiTest,
                                                            public apitest::XEnumeration
{
public:
    ScIndexEnumeration_TableConditionalEntryEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScIndexEnumeration_TableConditionalEntryEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();
};

ScIndexEnumeration_TableConditionalEntryEnumeration::
    ScIndexEnumeration_TableConditionalEntryEnumeration()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScIndexEnumeration_TableConditionalEntryEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet0->getCellByPosition(5, 5)->setValue(15);
    xSheet0->getCellByPosition(1, 4)->setValue(10);
    xSheet0->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Sequence<beans::PropertyValue> aConditions{
        comphelper::makePropertyValue(u"StyleName"_ustr, u"Result2"_ustr),
        comphelper::makePropertyValue(u"Formula1"_ustr, u"$Sheet1.$B$5"_ustr),
        comphelper::makePropertyValue(u"Formula2"_ustr, u""_ustr),
        comphelper::makePropertyValue(u"Operator"_ustr, sheet::ConditionOperator_EQUAL),
        comphelper::makePropertyValue(u"SourcePosition"_ustr, table::CellAddress(0, 1, 5))
    };

    uno::Reference<beans::XPropertySet> xPropertySet(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetConditionalEntries> xSCE(
        xPropertySet->getPropertyValue(u"ConditionalFormat"_ustr), uno::UNO_QUERY_THROW);
    xSCE->addNew(aConditions);

    uno::Any aProperty;
    aProperty <<= xSCE;
    xPropertySet->setPropertyValue(u"ConditionalFormat"_ustr, aProperty);

    uno::Reference<container::XEnumerationAccess> xEA(xSCE, uno::UNO_QUERY_THROW);

    return xEA->createEnumeration();
}

void ScIndexEnumeration_TableConditionalEntryEnumeration::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScIndexEnumeration_TableConditionalEntryEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
