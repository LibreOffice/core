/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/subtotaldescriptor.hxx>
#include <test/sheet/xsubtotaldescriptor.hxx>
#include <cppu/unotype.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#include <com/sun/star/sheet/XSubTotalField.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace css;

namespace sc_apitest
{
class ScSubTotalDescriptorBase : public UnoApiTest,
                                 public apitest::SubTotalDescriptor,
                                 public apitest::XElementAccess,
                                 public apitest::XEnumerationAccess,
                                 public apitest::XIndexAccess,
                                 public apitest::XPropertySet,
                                 public apitest::XServiceInfo,
                                 public apitest::XSubTotalDescriptor
{
public:
    ScSubTotalDescriptorBase();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScSubTotalDescriptorBase);

    // SubTotalDescriptor
    CPPUNIT_TEST(testSubTotalDescriptorProperties);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XSubTotalDescriptor
    CPPUNIT_TEST(testAddNew);
    CPPUNIT_TEST(testClear);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();
};

ScSubTotalDescriptorBase::ScSubTotalDescriptorBase()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<sheet::XSubTotalField>::get())
    , XIndexAccess(1)
    , XServiceInfo("ScSubTotalDescriptorBase", "com.sun.star.sheet.SubTotalDescriptor")
{
}

uno::Reference<uno::XInterface> ScSubTotalDescriptorBase::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSubTotalCalculatable> xSTC(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSubTotalDescriptor> xSTD = xSTC->createSubTotalDescriptor(true);

    uno::Sequence<sheet::SubTotalColumn> xCols{ { /* Column   */ 5,
                                                  /* Function */ sheet::GeneralFunction_SUM } };
    xSTD->addNew(xCols, 10);

    return xSTD;
}

void ScSubTotalDescriptorBase::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSubTotalDescriptorBase);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
