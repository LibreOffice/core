/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/xsheetconditionalentries.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <unonames.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/propertyvalue.hxx>
#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScTableConditionalFormat : public UnoApiTest,
                                 public apitest::XElementAccess,
                                 public apitest::XEnumerationAccess,
                                 public apitest::XIndexAccess,
                                 public apitest::XNameAccess,
                                 public apitest::XServiceInfo,
                                 public apitest::XSheetConditionalEntries
{
public:
    ScTableConditionalFormat();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Sequence<beans::PropertyValue> createCondition(const sal_Int32 nr) override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScTableConditionalFormat);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    // XSheetConditionalEntries
    CPPUNIT_TEST(testAddNew);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST(testRemoveByIndex);

    CPPUNIT_TEST_SUITE_END();
};

ScTableConditionalFormat::ScTableConditionalFormat()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<sheet::XSheetConditionalEntry>::get())
    , XIndexAccess(2)
    , XNameAccess("Entry1")
    , XServiceInfo("ScTableConditionalFormat", "com.sun.star.sheet.TableConditionalFormat")
{
}

uno::Reference<uno::XInterface> ScTableConditionalFormat::init()
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

    xSheetConditionalEntries->addNew(createCondition(5));
    xSheetConditionalEntries->addNew(createCondition(2));

    return xSheetConditionalEntries;
}

uno::Sequence<beans::PropertyValue> ScTableConditionalFormat::createCondition(const sal_Int32 nr)
{
    uno::Sequence<beans::PropertyValue> aPropValue{
        comphelper::makePropertyValue(SC_UNONAME_STYLENAME, OUString("Result2")),
        comphelper::makePropertyValue(SC_UNONAME_FORMULA1, "$Sheet1.$B$" + OUString::number(nr)),
        comphelper::makePropertyValue(SC_UNONAME_FORMULA2, OUString("")),
        comphelper::makePropertyValue(SC_UNONAME_OPERATOR, sheet::ConditionOperator_EQUAL),
        comphelper::makePropertyValue(SC_UNONAME_SOURCEPOS, table::CellAddress(0, 1, 5))
    };

    return aPropValue;
}

void ScTableConditionalFormat::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableConditionalFormat);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
