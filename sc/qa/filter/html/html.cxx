/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>
#include <test/htmltesttools.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <comphelper/propertyvalue.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sc/source/filter/html/ fixes.
class Test : public UnoApiXmlTest, public HtmlTestTools
{
public:
    Test()
        : UnoApiXmlTest("/sc/qa/filter/html/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdAsText)
{
    // Given a document with an A2 cell that contains "02" as text:
    OUString aURL = createFileURL(u"text.html");

    // When loading that document to Calc:
    uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue("DocumentService",
                                      OUString("com.sun.star.sheet.SpreadsheetDocument")),
    };
    loadWithParams(aURL, aParams);

    // Then make sure "01" is not auto-converted to 1, as a number:
    uno::Reference<sheet::XSpreadsheetDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSheets(xDocument->getSheets(), uno::UNO_QUERY);
    uno::Reference<table::XCellRange> xSheet(xSheets->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell(xSheet->getCellByPosition(0, 1), uno::UNO_QUERY);
    table::CellContentType eType{};
    xCell->getPropertyValue("CellContentType") >>= eType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (TEXT)
    // - Actual  : 1 (VALUE)
    // i.e. data-sheets-value was ignored on import.
    CPPUNIT_ASSERT_EQUAL(table::CellContentType_TEXT, eType);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
