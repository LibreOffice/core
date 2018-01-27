/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/sheet/sheetcell.hxx>
#include <test/sheet/xcelladdressable.hxx>
#include <test/sheet/xsheetannotationanchor.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScCellObj : public CalcUnoApiTest, public apitest::SheetCell,
                                         public apitest::XCellAddressable,
                                         public apitest::XSheetAnnotationAnchor
{
public:
    ScCellObj();

    virtual uno::Reference< uno::XInterface > init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScCellObj);

    // SheetCell
    CPPUNIT_TEST(testSheetCellProperties);

    // XCellAddressable
    CPPUNIT_TEST(testGetCellAddress);

    // XSheetAnnotationAnchor
    CPPUNIT_TEST(testGetAnnotation);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScCellObj::ScCellObj()
        : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > ScCellObj::init()
{
    uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xSheetDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets (xSheetDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<sheet::XSheetAnnotationsSupplier> xSheetAnnosSupplier(xSheet, UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotations> xSheetAnnos(xSheetAnnosSupplier->getAnnotations(),
                                                         UNO_QUERY_THROW);
    xSheetAnnos->insertNew(table::CellAddress(0, 2, 3), "xSheetAnnotation");

    return xSheet->getCellByPosition(2, 3);
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
