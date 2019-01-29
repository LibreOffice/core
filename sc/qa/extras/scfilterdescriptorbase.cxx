/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/sheetfilterdescriptor.hxx>
#include <test/sheet/xsheetfilterdescriptor.hxx>
#include <test/sheet/xsheetfilterdescriptor2.hxx>
#include <test/sheet/xsheetfilterdescriptor3.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSheetFilterable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScFilterDescriptorBase : public CalcUnoApiTest,
                               public apitest::SheetFilterDescriptor,
                               public apitest::XPropertySet,
                               public apitest::XServiceInfo,
                               public apitest::XSheetFilterDescriptor,
                               public apitest::XSheetFilterDescriptor2,
                               public apitest::XSheetFilterDescriptor3
{
public:
    ScFilterDescriptorBase();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual uno::Reference<uno::XInterface> init() override;

    CPPUNIT_TEST_SUITE(ScFilterDescriptorBase);

    // SheetFilterDescriptor
    CPPUNIT_TEST(testSheetFilterDescriptorProperties);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    // XSheetFilterDescriptor
    CPPUNIT_TEST(testGetSetFilterFields);

    // XSheetFilterDescriptor2
    CPPUNIT_TEST(testGetSetFilterFields2);

    // XSheetFilterDescriptor3
    CPPUNIT_TEST(testGetSetFilterFields3);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScFilterDescriptorBase::ScFilterDescriptorBase()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XPropertySet({ "Orientation", "OutputPosition" })
    , XServiceInfo("ScFilterDescriptorBase", "com.sun.star.sheet.SheetFilterDescriptor")
{
}

uno::Reference<uno::XInterface> ScFilterDescriptorBase::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndexAccess(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet->getCellByPosition(5, 5)->setValue(15);
    xSheet->getCellByPosition(1, 4)->setValue(10);
    xSheet->getCellByPosition(2, 0)->setValue(-5.15);

    uno::Reference<sheet::XSheetFilterable> xSF(xSheet, uno::UNO_QUERY_THROW);

    return xSF->createFilterDescriptor(true);
}

void ScFilterDescriptorBase::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScFilterDescriptorBase::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFilterDescriptorBase);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
