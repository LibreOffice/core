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
#include <test/sheet/xddelinks.hxx>

#include <cppu/unotype.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/app.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/sheet/XDDELinks.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScDDELinksObj : public CalcUnoApiTest,
                      public apitest::XDDELinks,
                      public apitest::XElementAccess,
                      public apitest::XEnumerationAccess,
                      public apitest::XIndexAccess,
                      public apitest::XNameAccess,
                      public apitest::XServiceInfo
{
public:
    ScDDELinksObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScDDELinksObj);

    // XDDELinks
    CPPUNIT_TEST(testAddDDELink);

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

ScDDELinksObj::ScDDELinksObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XDDELinks(m_directories.getURLFromSrc("/sc/qa/unoapi/testdocuments/ScDDELinksObj.ods"))
    , XElementAccess(cppu::UnoType<sheet::XDDELink>::get())
    , XIndexAccess(1)
    , XNameAccess("soffice|"
                  + m_directories.getURLFromSrc("/sc/qa/unoapi/testdocuments/ScDDELinksObj.ods")
                  + "!Sheet1.A1")
    , XServiceInfo("ScDDELinksObj", "com.sun.star.sheet.DDELinks")
{
}

uno::Reference<uno::XInterface> ScDDELinksObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    const OUString testdoc
        = m_directories.getURLFromSrc("/sc/qa/unoapi/testdocuments/ScDDELinksObj.ods");

    xSheet->getCellByPosition(5, 5)->setFormula("=DDE(\"soffice\";\"" + testdoc
                                                + "\";\"Sheet1.A1\")");
    xSheet->getCellByPosition(1, 4)->setFormula("=DDE(\"soffice\";\"" + testdoc
                                                + "\";\"Sheet1.A1\")");
    xSheet->getCellByPosition(2, 0)->setFormula("=DDE(\"soffice\";\"" + testdoc
                                                + "\";\"Sheet1.A1\")");

    uno::Reference<beans::XPropertySet> xPropSet(xDoc, uno::UNO_QUERY_THROW);
    uno::Any aDDELinks = xPropSet->getPropertyValue("DDELinks");
    uno::Reference<sheet::XDDELinks> xDDELinks(aDDELinks, uno::UNO_QUERY_THROW);

    return xDDELinks;
}

void ScDDELinksObj::setUp()
{
    Application::SetAppName("soffice"); // Enable DDE
    CalcUnoApiTest::setUp();
    // create a calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScDDELinksObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDDELinksObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
