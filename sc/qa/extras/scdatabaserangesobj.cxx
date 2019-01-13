/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/sheet/xdatabaseranges.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScDatabaseRangesObj : public CalcUnoApiTest,
                            public apitest::XDatabaseRanges,
                            public apitest::XEnumerationAccess,
                            public apitest::XIndexAccess
{
public:
    ScDatabaseRangesObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScDatabaseRangesObj);

    // XDatabaseRanges
    CPPUNIT_TEST(testAddRemoveDbRanges);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScDatabaseRangesObj::ScDatabaseRangesObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XIndexAccess(1)
{
}

uno::Reference<uno::XInterface> ScDatabaseRangesObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropSet(xDoc, UNO_QUERY_THROW);
    uno::Reference<sheet::XDatabaseRanges> xDbRanges(xPropSet->getPropertyValue("DatabaseRanges"),
                                                     UNO_QUERY_THROW);

    if (!xDbRanges->hasByName("DbRange"))
        xDbRanges->addNewByName("DbRange", table::CellRangeAddress(0, 2, 4, 5, 6));

    return xDbRanges;
}

void ScDatabaseRangesObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScDatabaseRangesObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDatabaseRangesObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
