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
#include <test/sheet/xdatabaseranges.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
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
class ScDatabaseRangesObj : public UnoApiTest,
                            public apitest::XDatabaseRanges,
                            public apitest::XElementAccess,
                            public apitest::XEnumerationAccess,
                            public apitest::XIndexAccess,
                            public apitest::XNameAccess,
                            public apitest::XServiceInfo
{
public:
    ScDatabaseRangesObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScDatabaseRangesObj);

    // XDatabaseRanges
    CPPUNIT_TEST(testAddRemoveDbRanges);

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

    CPPUNIT_TEST_SUITE_END();
};

ScDatabaseRangesObj::ScDatabaseRangesObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XElementAccess(cppu::UnoType<sheet::XDatabaseRange>::get())
    , XIndexAccess(1)
    , XNameAccess(u"DbRange"_ustr)
    , XServiceInfo(u"ScDatabaseRangesObj"_ustr, u"com.sun.star.sheet.DatabaseRanges"_ustr)
{
}

uno::Reference<uno::XInterface> ScDatabaseRangesObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropSet(xDoc, UNO_QUERY_THROW);
    uno::Reference<sheet::XDatabaseRanges> xDbRanges(
        xPropSet->getPropertyValue(u"DatabaseRanges"_ustr), UNO_QUERY_THROW);

    if (!xDbRanges->hasByName(u"DbRange"_ustr))
        xDbRanges->addNewByName(u"DbRange"_ustr, table::CellRangeAddress(0, 2, 4, 5, 6));

    return xDbRanges;
}

void ScDatabaseRangesObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDatabaseRangesObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
