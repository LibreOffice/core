/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <sal/types.h>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScIndexEnumeration_DDELinksEnumeration : public UnoApiTest, public apitest::XEnumeration
{
public:
    ScIndexEnumeration_DDELinksEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScIndexEnumeration_DDELinksEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();
};

ScIndexEnumeration_DDELinksEnumeration::ScIndexEnumeration_DDELinksEnumeration()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScIndexEnumeration_DDELinksEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet0->getCellByPosition(5, 5)->setFormula(
        u"=DDE(\"soffice\";\"ScDDELinksObj.ods\";\"Sheet1.A1\""_ustr);
    xSheet0->getCellByPosition(1, 4)->setFormula(
        u"=DDE(\"soffice\";\"ScDDELinksObj.ods\";\"Sheet1.A1\""_ustr);
    xSheet0->getCellByPosition(2, 0)->setFormula(
        u"=DDE(\"soffice\";\"ScDDELinksObj.ods\";\"Sheet1.A1\""_ustr);

    uno::Reference<beans::XPropertySet> xPropertySet(xDoc, uno::UNO_QUERY_THROW);

    uno::Reference<container::XEnumerationAccess> xEA(
        xPropertySet->getPropertyValue(u"DDELinks"_ustr), uno::UNO_QUERY_THROW);
    return xEA->createEnumeration();
}

void ScIndexEnumeration_DDELinksEnumeration::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScIndexEnumeration_DDELinksEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
