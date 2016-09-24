/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace css;
using namespace css::lang;
using namespace css::frame;

namespace sc_apitest {

class CheckXCellRangesQuery : public CalcUnoApiTest
{
public:
    CheckXCellRangesQuery();

    virtual void setUp() override;
    virtual void tearDown() override;

    uno::Reference< uno::XInterface > init();
    void checkEmptyCell();
    void checkFilledCell();

    void _queryColumnDifferences(const OUString& expected);
    void _queryRowDifferences(const OUString& expected);
    void _queryEmptyCells(const OUString& expected);

    CPPUNIT_TEST_SUITE(CheckXCellRangesQuery);
    CPPUNIT_TEST(checkEmptyCell);
    CPPUNIT_TEST(checkFilledCell);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference< sheet::XCellRangesQuery > m_xCell;
    OUString sSheetName;
};

CheckXCellRangesQuery::CheckXCellRangesQuery()
     : CalcUnoApiTest("/sc/qa/extras/testdocuments")
{
}

uno::Reference< uno::XInterface > CheckXCellRangesQuery::init()
{
        // create a calc document
        if (!mxComponent.is())
            // Load an empty document.
            mxComponent = loadFromDesktop("private:factory/scalc");

        uno::Reference< sheet::XSpreadsheetDocument > xSheetDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_MESSAGE("no calc document!", xSheetDoc.is());

        // Getting spreadsheet
        uno::Reference< sheet::XSpreadsheets > oSheets = xSheetDoc->getSheets();
        uno::Reference< container::XIndexAccess > oIndexSheets(oSheets, uno::UNO_QUERY_THROW);
        uno::Any aAny = oIndexSheets->getByIndex(0);
        uno::Reference<container::XNamed> xNamed;
        CPPUNIT_ASSERT(aAny >>= xNamed);
        sSheetName = xNamed->getName();

        // get the cell
        uno::Reference< sheet::XSpreadsheet > xSpreadSheet;
        CPPUNIT_ASSERT(aAny >>= xSpreadSheet);
        uno::Reference<uno::XInterface> oObj = xSpreadSheet->getCellByPosition(2, 3);
        m_xCell = uno::Reference<sheet::XCellRangesQuery>(oObj, uno::UNO_QUERY_THROW);

        // set one value for comparison.
        xSpreadSheet->getCellByPosition(1, 1)->setValue(15);
        xSpreadSheet->getCellByPosition(1, 3)->setValue(5);
        xSpreadSheet->getCellByPosition(2, 1)->setFormula("=B2+B4");

        return xSpreadSheet;
}

/**
 * Perform some tests on an empty cell:
 * <ol>
 * <li>compare an empty cell with a cell with a value in the same column</li>
 * <li>compare an empty cell with a cell with a value in the same row</li>
 * <li>query for empty cells</li>
 * <ol>
 */
void CheckXCellRangesQuery::checkEmptyCell()
{
    // compare an empty cell with a cell with a value
    _queryColumnDifferences(sSheetName + ".C4");
    // compare an empty cell with a cell with a value
    _queryRowDifferences(sSheetName + ".C4");
}

/**
 * Perform some tests on a filled cell:
 * <ol>
 * <li>compare an cell with value 5 with a cell with value 15 in the same
 * column</li>
 * <li>compare an cell with value 5 with a cell with value 15 in the same
 * row</li>
 * <li>query for an empty cell.</li>
 * <ol>
 */

void CheckXCellRangesQuery::checkFilledCell()
{
    uno::Reference< sheet::XSpreadsheet > xSpreadSheet(init(), uno::UNO_QUERY_THROW);
    // fill the cell with a value
    xSpreadSheet->getCellByPosition(2, 3)->setValue(15);

    // compare an cell with value 5 with a cell with value 15
    _queryColumnDifferences(sSheetName + ".C4");
    // compare an cell with value 5 with a cell with value 15
    _queryRowDifferences(sSheetName + ".C4");
    // try to get nothing
    _queryEmptyCells("");
}

/**
 * Query column differences between my cell(2,3) and (1,1).
 *
 * @param expected The expected outcome value.
 */
void CheckXCellRangesQuery::_queryColumnDifferences(const OUString& expected)
{
    //Query column differences
    uno::Reference<sheet::XSheetCellRanges> ranges = m_xCell->queryColumnDifferences(table::CellAddress(0, 1, 1));
    OUString getting = ranges->getRangeAddressesAsString();

    CPPUNIT_ASSERT_EQUAL(expected, getting);
}

/**
 * Query for an empty cell.
 *
 * @param expected The expected outcome value.
 */
void CheckXCellRangesQuery::_queryEmptyCells(const OUString& expected)
{
    //Query empty cells
    uno::Reference<sheet::XSheetCellRanges> ranges = m_xCell->queryEmptyCells();
    OUString getting = ranges->getRangeAddressesAsString();

    CPPUNIT_ASSERT_EQUAL(expected, getting);
}

/**
 * Query row differences between my cell(2,3) and (1,1).
 *
 * @param expected The expected outcome value.
 */
void CheckXCellRangesQuery::_queryRowDifferences(const OUString& expected) {
    //Query row differences
    uno::Reference<sheet::XSheetCellRanges> ranges = m_xCell->queryRowDifferences(table::CellAddress(0, 1, 1));
    OUString getting = ranges->getRangeAddressesAsString();

    CPPUNIT_ASSERT_EQUAL(expected, getting);
}

void CheckXCellRangesQuery::setUp()
{
    CalcUnoApiTest::setUp();
    init();
}

void CheckXCellRangesQuery::tearDown()
{
    closeDocument(mxComponent);
    mxComponent.clear();
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(CheckXCellRangesQuery);

}

CPPUNIT_PLUGIN_IMPLEMENT();
