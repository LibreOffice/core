/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xnamed.hxx>
#include <test/sheet/sheetlink.hxx>
#include <test/util/xrefreshable.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
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
class ScSheetLinkObj : public CalcUnoApiTest,
                       public apitest::SheetLink,
                       public apitest::XNamed,
                       public apitest::XRefreshable
{
public:
    ScSheetLinkObj();

    virtual uno::Reference<uno::XInterface> init() override;

    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScSheetLinkObj);

    // SheetLink
    CPPUNIT_TEST(testSheetLinkProperties);

    // XNamed
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testSetNameByScSheetLinkObj);

    // XRefreshable
    CPPUNIT_TEST(testRefreshListener);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScSheetLinkObj::ScSheetLinkObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XNamed(m_directories.getURLFromSrc(u"/sc/qa/extras/testdocuments/ScSheetLinkObj.ods"))
{
}

uno::Reference<uno::XInterface> ScSheetLinkObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSheetLinkable> xSL(xSheet, uno::UNO_QUERY_THROW);
    xSL->link(m_directories.getURLFromSrc(u"/sc/qa/extras/testdocuments/ScSheetLinkObj.ods"),
              "Sheet1", "", "", sheet::SheetLinkMode_VALUE);

    uno::Reference<beans::XPropertySet> xPropSet(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> sheetLinks;
    CPPUNIT_ASSERT(xPropSet->getPropertyValue("SheetLinks") >>= sheetLinks);
    CPPUNIT_ASSERT(sheetLinks.is());

    uno::Any aAny = sheetLinks->getByName(sheetLinks->getElementNames()[0]);
    uno::Reference<beans::XPropertySet> sheetLink;
    aAny >>= sheetLink;
    return sheetLink;
}

void ScSheetLinkObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScSheetLinkObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSheetLinkObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
