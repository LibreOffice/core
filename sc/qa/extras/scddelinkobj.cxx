/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xnamed.hxx>
#include <test/sheet/xddelink.hxx>
#include <test/util/xrefreshable.hxx>

#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/XDDELink.hpp>
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
class ScDDELinkObj : public UnoApiTest,
                     public apitest::XDDELink,
                     public apitest::XNamed,
                     public apitest::XRefreshable
{
public:
    ScDDELinkObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScDDELinkObj);

    // XDDELink
    CPPUNIT_TEST(testGetApplication);
    CPPUNIT_TEST(testGetItem);
    CPPUNIT_TEST(testGetTopic);

    // XNamed
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetNameThrowsException);

    // XRefreshable
    CPPUNIT_TEST(testRefreshListener);

    CPPUNIT_TEST_SUITE_END();
};

ScDDELinkObj::ScDDELinkObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XDDELink(maTempFile.GetURL())
    , XNamed("soffice|" + maTempFile.GetURL() + "!Sheet1.A1")
{
}

uno::Reference<uno::XInterface> ScDDELinkObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);

    const OUString testdoc = maTempFile.GetURL();

    xSheet->getCellByPosition(5, 5)->setFormula("=DDE(\"soffice\";\"" + testdoc
                                                + "\";\"Sheet1.A1\")");
    xSheet->getCellByPosition(1, 4)->setFormula("=DDE(\"soffice\";\"" + testdoc
                                                + "\";\"Sheet1.A1\")");

    uno::Reference<beans::XPropertySet> xPropSet(xDoc, UNO_QUERY_THROW);
    uno::Any aDDELinks = xPropSet->getPropertyValue("DDELinks");
    uno::Reference<container::XNameAccess> xNA(aDDELinks, UNO_QUERY_THROW);
    uno::Sequence<OUString> sLinkNames = xNA->getElementNames();
    uno::Reference<sheet::XDDELink> xDDELink(xNA->getByName(sLinkNames[0]), UNO_QUERY_THROW);
    return xDDELink;
}

void ScDDELinkObj::setUp()
{
    Application::SetAppName("soffice"); // Enable DDE
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");

    createTempCopy(u"ScDDELinksObj.ods");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDDELinkObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
