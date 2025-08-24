/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/sheet/sheetcell.hxx>
#include <test/sheet/xcelladdressable.hxx>
#include <test/sheet/xformulaquery.hxx>
#include <test/sheet/xsheetannotationanchor.hxx>
#include <test/table/xcell.hxx>
#include <test/table/xcolumnrowrange.hxx>
#include <test/text/xsimpletext.hxx>
#include <test/util/xindent.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScCellObj : public UnoApiTest,
                  public apitest::SheetCell,
                  public apitest::XCell,
                  public apitest::XCellAddressable,
                  public apitest::XColumnRowRange,
                  public apitest::XEnumerationAccess,
                  public apitest::XFormulaQuery,
                  public apitest::XIndent,
                  public apitest::XSheetAnnotationAnchor,
                  public apitest::XSimpleText
{
public:
    ScCellObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<uno::XInterface> getXSpreadsheet() override;
    virtual void setUp() override;

    void testInsertVarious_ScCellObj();

    CPPUNIT_TEST_SUITE(ScCellObj);

    // SheetCell
    CPPUNIT_TEST(testSheetCellProperties);

    // XCell
    CPPUNIT_TEST(testGetError);
    CPPUNIT_TEST(testGetType);
    CPPUNIT_TEST(testSetGetFormula);
    CPPUNIT_TEST(testSetGetValue);

    // XCellAddressable
    CPPUNIT_TEST(testGetCellAddress);

    // XColumnRowRange
    CPPUNIT_TEST(testGetColumns);
    CPPUNIT_TEST(testGetRows);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XFormulaQuery
    CPPUNIT_TEST(testQueryDependents);
    CPPUNIT_TEST(testQueryPrecedents);

    // XIndent
    CPPUNIT_TEST(testIncrementIndent);
    CPPUNIT_TEST(testDecrementIndent);

    // XSheetAnnotationAnchor
    CPPUNIT_TEST(testGetAnnotation);

    // XSimpleText
    CPPUNIT_TEST(testCreateTextCursor);
    CPPUNIT_TEST(testCreateTextCursorByRange);
    CPPUNIT_TEST(testInsertString);
    CPPUNIT_TEST(testInsertControlCharacter);

    CPPUNIT_TEST(testInsertVarious_ScCellObj);

    CPPUNIT_TEST_SUITE_END();
};

ScCellObj::ScCellObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , apitest::XFormulaQuery(table::CellRangeAddress(0, 2, 3, 2, 3),
                             table::CellRangeAddress(0, 0, 0, 3, 0), 0, 0)
{
}

uno::Reference<uno::XInterface> ScCellObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xSheetDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<sheet::XSheetAnnotationsSupplier> xSheetAnnosSupplier(xSheet, UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotations> xSheetAnnos(xSheetAnnosSupplier->getAnnotations(),
                                                         UNO_SET_THROW);
    xSheetAnnos->insertNew(table::CellAddress(0, 2, 3), u"xSheetAnnotation"_ustr);

    return xSheet->getCellByPosition(2, 3);
}

uno::Reference<uno::XInterface> ScCellObj::getXSpreadsheet()
{
    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xSheetDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    setXCell(xSheet->getCellByPosition(2, 3));

    return xSheet;
}

void ScCellObj::setUp()
{
    UnoApiTest::setUp();
    loadFromURL(u"private:factory/scalc"_ustr);
}

void ScCellObj::testInsertVarious_ScCellObj()
{
    auto xText = init().queryThrow<text::XText>();

    xText->setString(u"foo"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, xText->getString());

    // Test different kinds of XTextRange passed to insertString: cursor, start/end, itself.
    // The test relies on createTextCursor() giving the position at the end of the text; this
    // is true for Calc cells, but e.g. Writer produces cursors at the start - who is right?

    xText->insertString(xText, u"b"_ustr, true); // must replace all
    xText->insertString(xText->createTextCursor(), u"c"_ustr, false);
    xText->insertString(xText->getStart(), u"a"_ustr, false);
    xText->insertString(xText->getEnd(), u"d"_ustr, false);
    xText->insertString(xText, u"e"_ustr, false);

    // Test insertString interleaved with insertTextContent

    auto makeField = [xF = mxComponent.queryThrow<lang::XMultiServiceFactory>()](const OUString& s)
    {
        auto xField = xF->createInstance(u"com.sun.star.text.textfield.URL"_ustr)
                          .queryThrow<beans::XPropertySet>();
        xField->setPropertyValue(u"URL"_ustr, uno::Any(u"http://www.example.org/"_ustr));
        xField->setPropertyValue(u"Representation"_ustr, uno::Any(s));
        return xField.queryThrow<text::XTextContent>();
    };

    xText->insertString(xText->createTextCursor(), u" 1: "_ustr, false);
    xText->insertTextContent(xText->createTextCursor(), makeField(u"h1"_ustr), false);

    xText->insertString(xText->getEnd(), u" 2: "_ustr, false);
    xText->insertTextContent(xText->getEnd(), makeField(u"h2"_ustr), false);

    xText->insertString(xText, u" 3: "_ustr, false);
    xText->insertTextContent(xText, makeField(u"h3"_ustr), false);

    CPPUNIT_ASSERT_EQUAL(u"abcde 1: h1 2: h2 3: h3"_ustr, xText->getString());
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
