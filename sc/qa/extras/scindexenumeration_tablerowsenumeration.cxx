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

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScIndexEnumeration_TableRowsEnumeration : public UnoApiTest, public apitest::XEnumeration
{
public:
    ScIndexEnumeration_TableRowsEnumeration();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScIndexEnumeration_TableRowsEnumeration);

    // XEnumeration
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);

    CPPUNIT_TEST_SUITE_END();
};

ScIndexEnumeration_TableRowsEnumeration::ScIndexEnumeration_TableRowsEnumeration()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScIndexEnumeration_TableRowsEnumeration::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XNameAccess> xNA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xNA->getByName(xNA->getElementNames()[0]),
                                                uno::UNO_QUERY_THROW);

    // limit the range the XEnumeration has to walk over (see fdo#45337).
    uno::Reference<table::XCellRange> xCR(xSheet0->getCellRangeByName(u"A1:A4"_ustr),
                                          uno::UNO_SET_THROW);

    uno::Reference<table::XColumnRowRange> xColRowRange(xCR, uno::UNO_QUERY_THROW);
    uno::Reference<table::XTableRows> xTableRows(xColRowRange->getRows(), uno::UNO_SET_THROW);
    uno::Reference<container::XEnumerationAccess> xEA(xTableRows, uno::UNO_QUERY_THROW);

    return xEA->createEnumeration();
}

void ScIndexEnumeration_TableRowsEnumeration::setUp()
{
    UnoApiTest::setUp();
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScIndexEnumeration_TableRowsEnumeration);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
