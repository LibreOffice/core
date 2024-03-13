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
#include <test/sheet/xlabelranges.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScLabelRangesObj : public UnoApiTest,
                         public apitest::XElementAccess,
                         public apitest::XEnumerationAccess,
                         public apitest::XIndexAccess,
                         public apitest::XLabelRanges
{
public:
    ScLabelRangesObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScLabelRangesObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XLabelRanges
    CPPUNIT_TEST(testAddNew);
    CPPUNIT_TEST(testRemoveByIndex);

    CPPUNIT_TEST_SUITE_END();
};

ScLabelRangesObj::ScLabelRangesObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<sheet::XLabelRange>::get())
    , XIndexAccess(1)
{
}

uno::Reference<uno::XInterface> ScLabelRangesObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xProp(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XLabelRanges> xLabelRanges(xProp->getPropertyValue("ColumnLabelRanges"),
                                                     uno::UNO_QUERY_THROW);

    table::CellRangeAddress aCellRanageAddr1(0, 0, 1, 0, 6);
    table::CellRangeAddress aCellRanageAddr2(0, 0, 0, 0, 1);
    xLabelRanges->addNew(aCellRanageAddr1, aCellRanageAddr2);

    return xLabelRanges;
}

void ScLabelRangesObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScLabelRangesObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
