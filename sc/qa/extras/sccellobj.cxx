/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/sheet/sheetcell.hxx>
#include <test/sheet/xcelladdressable.hxx>
#include <test/sheet/xformulaquery.hxx>
#include <test/sheet/xsheetannotationanchor.hxx>
#include <test/table/xcell.hxx>
#include <test/table/xcolumnrowrange.hxx>
#include <test/text/xsimpletext.hxx>
#include <test/util/xindent.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScCellObj : public CalcUnoApiTest,
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
    virtual void tearDown() override;

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

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
};

ScCellObj::ScCellObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , apitest::XFormulaQuery(table::CellRangeAddress(0, 2, 3, 2, 3),
                             table::CellRangeAddress(0, 0, 0, 3, 0), 0, 0)
{
}

uno::Reference<uno::XInterface> ScCellObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xSheetDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<sheet::XSheetAnnotationsSupplier> xSheetAnnosSupplier(xSheet, UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotations> xSheetAnnos(xSheetAnnosSupplier->getAnnotations(),
                                                         UNO_QUERY_THROW);
    xSheetAnnos->insertNew(table::CellAddress(0, 2, 3), "xSheetAnnotation");

    return xSheet->getCellByPosition(2, 3);
}

uno::Reference<uno::XInterface> ScCellObj::getXSpreadsheet()
{
    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xSheetDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    setXCell(xSheet->getCellByPosition(2, 3));

    return xSheet;
}

void ScCellObj::setUp()
{
    CalcUnoApiTest::setUp();
    mxComponent = loadFromDesktop("private:factory/scalc");
}

void ScCellObj::tearDown()
{
    closeDocument(mxComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
