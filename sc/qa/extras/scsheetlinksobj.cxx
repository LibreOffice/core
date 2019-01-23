/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/lang/xserviceinfo.hxx>

#include <cppu/unotype.hxx>
#include <rtl/string.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/SheetLinkMode.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScSheetLinksObj : public CalcUnoApiTest,
                        public apitest::XElementAccess,
                        public apitest::XEnumerationAccess,
                        public apitest::XIndexAccess,
                        public apitest::XNameAccess,
                        public apitest::XServiceInfo
{
public:
    ScSheetLinksObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScSheetLinksObj);

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

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScSheetLinksObj::ScSheetLinksObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<beans::XPropertySet>::get())
    , XIndexAccess(1)
    , XNameAccess(m_directories.getURLFromSrc("/sc/qa/extras/testdocuments/ScSheetLinksObj.ods"))
    , XServiceInfo("ScSheetLinksObj", "com.sun.star.sheet.SheetLinks")
{
}

uno::Reference<uno::XInterface> ScSheetLinksObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSheetLinkable> xSL(xSheet0, uno::UNO_QUERY_THROW);
    OUString aFileURL;
    createFileURL("ScSheetLinksObj.ods", aFileURL);
    xSL->link(aFileURL, "Sheet1", "", "", sheet::SheetLinkMode_VALUE);

    uno::Reference<beans::XPropertySet> xPropertySet(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA;
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue("SheetLinks") >>= xNA);

    return xNA;
}

void ScSheetLinksObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScSheetLinksObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSheetLinksObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
