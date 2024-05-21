/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <test/container/xnameaccess.hxx>
#include <test/document/xactionlockable.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <test/sheet/xnamedranges.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScNamedRangesObj : public UnoApiTest,
                         public apitest::XActionLockable,
                         public apitest::XElementAccess,
                         public apitest::XEnumerationAccess,
                         public apitest::XIndexAccess,
                         public apitest::XNameAccess,
                         public apitest::XNamedRanges,
                         public apitest::XPropertySet,
                         public apitest::XServiceInfo
{
public:
    ScNamedRangesObj();

    virtual void setUp() override;

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<uno::XInterface> getXNamedRanges(sal_Int32 nSheet = 0) override;

    CPPUNIT_TEST_SUITE(ScNamedRangesObj);

    // XActionLockable
    CPPUNIT_TEST(testAddRemoveActionLock);
    CPPUNIT_TEST(testSetResetActionLock);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNamedRanges
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XNamedRanges
    CPPUNIT_TEST(testAddNewByName);
    CPPUNIT_TEST(testAddNewFromTitles);
    //CPPUNIT_TEST_EXCEPTION(testRemoveByName, uno::RuntimeException);
    CPPUNIT_TEST(testOutputList);

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

    CPPUNIT_TEST_SUITE_END();
};

ScNamedRangesObj::ScNamedRangesObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XElementAccess(cppu::UnoType<sheet::XNamedRange>::get())
    , XIndexAccess(4)
    , XNameAccess(u"initial1"_ustr)
    , XServiceInfo(u"ScNamedRangesObj"_ustr, u"com.sun.star.sheet.NamedRanges"_ustr)
{
}

uno::Reference<uno::XInterface> ScNamedRangesObj::init()
{
    uno::Reference<beans::XPropertySet> xPropSet(mxComponent, UNO_QUERY_THROW);
    uno::Reference<sheet::XNamedRanges> xNamedRanges(
        xPropSet->getPropertyValue(u"NamedRanges"_ustr), UNO_QUERY_THROW);

    //set value from xnamedranges.hxx
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDoc->getSheets(), UNO_QUERY_THROW);
    xSheet.set(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    return xNamedRanges;
}

uno::Reference<uno::XInterface> ScNamedRangesObj::getXNamedRanges(sal_Int32 nSheet)
{
    uno::Reference<beans::XPropertySet> xPropSet(mxComponent, UNO_QUERY_THROW);
    uno::Reference<sheet::XNamedRanges> xNamedRanges(
        xPropSet->getPropertyValue(u"NamedRanges"_ustr), UNO_QUERY_THROW);

    //set value from xnamedranges.hxx
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDoc->getSheets(), UNO_QUERY_THROW);
    xSheet.set(xIndexAccess->getByIndex(nSheet), UNO_QUERY_THROW);

    return xNamedRanges;
}

void ScNamedRangesObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    loadFromFile(u"ScNamedRangeObj.ods");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScNamedRangesObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
