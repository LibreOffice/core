/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XViewFreezable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <cool.hpp>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <scriptinterop/XRange.hpp>
#include <scriptinterop/XSheet.hpp>
#include <scriptinterop/XSpreadsheet.hpp>
#include <test/unoapi_test.hxx>

namespace
{
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/scriptinterop/qa/unit/data/"_ustr)
    {
    }

protected:
    // Loads a fresh spreadsheet and makes its frame the active one, which is what
    // getActiveSpreadsheet resolves against.
    cpo::uno::Reference<scriptinterop::XSpreadsheet> loadSpreadsheet()
    {
        mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
        cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent,
                                                             cpo::uno::UNO_QUERY_THROW);
        auto const xDesktop
            = css::frame::Desktop::create(comphelper::getProcessComponentContext());
        xDesktop->setActiveFrame(xModel->getCurrentController()->getFrame());
        return cool::get(comphelper::getProcessComponentContext())->getActiveSpreadsheet();
    }
};

CPPUNIT_TEST_FIXTURE(Test, testActiveSheetAndName)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    // A fresh document's first sheet carries the default name.
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, xSheet->getName());
}

CPPUNIT_TEST_FIXTURE(Test, testRangeByNameAndValues)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1:B2"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xRange->getRow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xRange->getColumn());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xRange->getNumRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xRange->getNumColumns());
    cpo::uno::Sequence<cpo::uno::Any> const row0{ cpo::uno::Any(1.0), cpo::uno::Any(u"two"_ustr) };
    cpo::uno::Sequence<cpo::uno::Any> const row1{ cpo::uno::Any(u"=A1+1"_ustr),
                                                  cpo::uno::Any(4.0) };
    xRange->setValues(cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>>{ row0, row1 });
    auto const values = xRange->getValues();
    double d = 0;
    OUString s;
    CPPUNIT_ASSERT(values[0][0] >>= d);
    CPPUNIT_ASSERT_EQUAL(1.0, d);
    CPPUNIT_ASSERT(values[0][1] >>= s);
    CPPUNIT_ASSERT_EQUAL(u"two"_ustr, s);
    // A formula cell reports its computed result, not its text.
    CPPUNIT_ASSERT(values[1][0] >>= d);
    CPPUNIT_ASSERT_EQUAL(2.0, d);
    CPPUNIT_ASSERT(values[1][1] >>= d);
    CPPUNIT_ASSERT_EQUAL(4.0, d);
}

CPPUNIT_TEST_FIXTURE(Test, testRangeByPosition)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    // A row and a column on their own name one cell. A row count reaches down from it, and a
    // column count reaches to the right as well.
    auto const xCell = xSheet->getRangeAtCell(2, 3);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xCell->getRow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xCell->getColumn());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCell->getNumRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCell->getNumColumns());
    auto const xColumn = xSheet->getRangeAtRows(2, 3, 4);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xColumn->getNumRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xColumn->getNumColumns());
    auto const xBlock = xSheet->getRangeAt(2, 3, 4, 5);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xBlock->getNumRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xBlock->getNumColumns());
}

CPPUNIT_TEST_FIXTURE(Test, testSetValuesRejectsMismatchedShape)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1:B2"_ustr);
    cpo::uno::Sequence<cpo::uno::Any> const oneCell{ cpo::uno::Any(1.0) };
    // Too few rows.
    CPPUNIT_ASSERT_THROW(
        xRange->setValues(cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>>{ oneCell }),
        cpo::uno::RuntimeException);
    cpo::uno::Sequence<cpo::uno::Any> const row0{ cpo::uno::Any(1.0), cpo::uno::Any(2.0) };
    cpo::uno::Sequence<cpo::uno::Any> const shortRow1{ cpo::uno::Any(3.0) };
    // Right row count, but one row too short.
    CPPUNIT_ASSERT_THROW(
        xRange->setValues(
            cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>>{ row0, shortRow1 }),
        cpo::uno::RuntimeException);
    // A rejected call leaves every cell in the range untouched, not partially written.
    CPPUNIT_ASSERT(!xRange->getValue().hasValue());
}

CPPUNIT_TEST_FIXTURE(Test, testSetValuesRejectsUnwritableValue)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1:B2"_ustr);
    cpo::uno::Sequence<cpo::uno::Any> const row0{ cpo::uno::Any(1.0), cpo::uno::Any(2.0) };
    // An interface has no cell representation, and it sits last so that every value in front
    // of it is one a cell would have taken.
    cpo::uno::Sequence<cpo::uno::Any> const row1{ cpo::uno::Any(3.0), cpo::uno::Any(xSheet) };
    CPPUNIT_ASSERT_THROW(
        xRange->setValues(cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>>{ row0, row1 }),
        cpo::uno::RuntimeException);
    // No cell in the range was written.
    auto const values = xRange->getValues();
    for (auto const& row : values)
    {
        for (auto const& value : row)
        {
            CPPUNIT_ASSERT(!value.hasValue());
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testSetValueFormulaDetection)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    // A string starting with "=" becomes a real formula, evaluated rather than stored as text.
    auto const xFormula = xSheet->getRange(u"C1"_ustr);
    xFormula->setValue(cpo::uno::Any(u"=1+1"_ustr));
    double d = 0;
    CPPUNIT_ASSERT(xFormula->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(2.0, d);
    // Any other string is stored as plain text.
    auto const xText = xSheet->getRange(u"C2"_ustr);
    xText->setValue(cpo::uno::Any(u"plain text"_ustr));
    OUString s;
    CPPUNIT_ASSERT(xText->getValue() >>= s);
    CPPUNIT_ASSERT_EQUAL(u"plain text"_ustr, s);
    // A formula whose result is text reports that text, not its formula source.
    auto const xFormulaText = xSheet->getRange(u"C3"_ustr);
    xFormulaText->setValue(cpo::uno::Any(u"=\"hi \"&\"there\""_ustr));
    CPPUNIT_ASSERT(xFormulaText->getValue() >>= s);
    CPPUNIT_ASSERT_EQUAL(u"hi there"_ustr, s);
}

CPPUNIT_TEST_FIXTURE(Test, testSetValueFillsWholeRange)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    // setValue() on a multi-cell range fills every cell with the same value, the way it does in
    // SpreadsheetApp-style code, rather than only the top-left cell.
    auto const xRange = xSheet->getRange(u"D1:D3"_ustr);
    xRange->setValue(cpo::uno::Any(5.0));
    auto const values = xRange->getValues();
    for (auto const& row : values)
    {
        double d = 0;
        CPPUNIT_ASSERT(row[0] >>= d);
        CPPUNIT_ASSERT_EQUAL(5.0, d);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testSetValueRejectsUnsupportedType)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    cpo::uno::Sequence<double> const unsupported{ 1.0, 2.0 };
    CPPUNIT_ASSERT_THROW(xRange->setValue(cpo::uno::Any(unsupported)), cpo::uno::RuntimeException);
    // A rejected setValue() leaves the cell empty rather than partially applied.
    CPPUNIT_ASSERT(!xRange->getValue().hasValue());
}

CPPUNIT_TEST_FIXTURE(Test, testFormulaErrorValue)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    // A formula that errors reports its error text, not a numeric value standing in for it.
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setValue(cpo::uno::Any(u"=1/0"_ustr));
    OUString s;
    CPPUNIT_ASSERT(xRange->getValue() >>= s);
    CPPUNIT_ASSERT_EQUAL(u"#DIV/0!"_ustr, s);
    // A multi-cell read reports the same error text for that cell, not a blank value.
    auto const values = xSheet->getRange(u"A1:A1"_ustr)->getValues();
    CPPUNIT_ASSERT(values[0][0] >>= s);
    CPPUNIT_ASSERT_EQUAL(u"#DIV/0!"_ustr, s);
}

CPPUNIT_TEST_FIXTURE(Test, testActiveRangeRejectsSelectionOnOtherSheet)
{
    auto const xSpreadsheet = loadSpreadsheet();
    auto const xSheet1 = xSpreadsheet->getActiveSheet();
    cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheetDocument> const xDoc(mxComponent,
                                                                     cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheets> const xSheets(xDoc->getSheets());
    xSheets->insertNewByName(u"Other"_ustr, 1);
    cpo::uno::Reference<css::container::XNameAccess> const xSheetsByName(xSheets,
                                                                         cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheet> xOtherRaw;
    xSheetsByName->getByName(u"Other"_ustr) >>= xOtherRaw;
    cpo::uno::Reference<css::sheet::XSpreadsheetView> const xView(xModel->getCurrentController(),
                                                                  cpo::uno::UNO_QUERY_THROW);
    // The selection now sits on "Other", not on the Sheet1 wrapper obtained earlier.
    xView->setActiveSheet(xOtherRaw);
    cpo::uno::Reference<css::table::XCellRange> const xOtherRange(xOtherRaw,
                                                                   cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::view::XSelectionSupplier> const xSelectionSupplier(
        xModel->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(
        xSelectionSupplier->select(cpo::uno::Any(xOtherRange->getCellRangeByName(u"A1"_ustr))));
    CPPUNIT_ASSERT_THROW(xSheet1->getActiveRange(), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xSheet1->getActiveCell(), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testActiveRangeAndCell)
{
    auto const xSpreadsheet = loadSpreadsheet();
    auto const xSheet = xSpreadsheet->getActiveSheet();
    cpo::uno::Reference<css::sheet::XSpreadsheetDocument> const xDoc(mxComponent,
                                                                     cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::container::XIndexAccess> const xSheets(xDoc->getSheets(),
                                                                    cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::table::XCellRange> xRawSheet;
    xSheets->getByIndex(0) >>= xRawSheet;
    auto const xRawRange = xRawSheet->getCellRangeByName(u"B2:C3"_ustr);
    cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::view::XSelectionSupplier> const xSelectionSupplier(
        xModel->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xSelectionSupplier->select(cpo::uno::Any(xRawRange)));
    auto const xActive = xSheet->getActiveRange();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xActive->getRow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xActive->getColumn());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xActive->getNumRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xActive->getNumColumns());
    // The active cell is the top-left cell of the selection.
    auto const xCell = xSheet->getActiveCell();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xCell->getRow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xCell->getColumn());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCell->getNumRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xCell->getNumColumns());
}

CPPUNIT_TEST_FIXTURE(Test, testDataRange)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    xSheet->getRange(u"B2"_ustr)->setValue(cpo::uno::Any(1.0));
    xSheet->getRange(u"D5"_ustr)->setValue(cpo::uno::Any(2.0));
    // The data range is the bounding box of every non-empty cell, not just the two written ones.
    auto const xData = xSheet->getDataRange();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xData->getRow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xData->getColumn());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xData->getNumRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xData->getNumColumns());
}

CPPUNIT_TEST_FIXTURE(Test, testMaxRowsColumns)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    cpo::uno::Reference<css::table::XColumnRowRange> const xColumnRowRange(
        xSheet->getuno(), cpo::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(xColumnRowRange->getRows()->getCount(), xSheet->getMaxRows());
    CPPUNIT_ASSERT_EQUAL(xColumnRowRange->getColumns()->getCount(), xSheet->getMaxColumns());
}

CPPUNIT_TEST_FIXTURE(Test, testFrozenPanes)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSheet->getFrozenRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSheet->getFrozenColumns());
    cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XViewFreezable> const xFreezable(
        xModel->getCurrentController(), cpo::uno::UNO_QUERY_THROW);
    xFreezable->freezeAtPosition(2, 3);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSheet->getFrozenRows());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSheet->getFrozenColumns());
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteRowsAndColumns)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    xSheet->getRange(u"A1"_ustr)->setValue(cpo::uno::Any(1.0));
    xSheet->getRange(u"A2"_ustr)->setValue(cpo::uno::Any(2.0));
    xSheet->getRange(u"A3"_ustr)->setValue(cpo::uno::Any(3.0));
    xSheet->deleteRows(2, 1);
    // The row below the deleted one shifted up; this is a real deletion, not content clearing.
    double d = 0;
    CPPUNIT_ASSERT(xSheet->getRange(u"A2"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(3.0, d);
    xSheet->getRange(u"B1"_ustr)->setValue(cpo::uno::Any(1.0));
    xSheet->getRange(u"C1"_ustr)->setValue(cpo::uno::Any(2.0));
    xSheet->deleteColumns(2, 1);
    CPPUNIT_ASSERT(xSheet->getRange(u"B1"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(2.0, d);
    // A single row or column comes off by position alone, without a count.
    xSheet->getRange(u"A9"_ustr)->setValue(cpo::uno::Any(4.0));
    xSheet->deleteRow(8);
    CPPUNIT_ASSERT(xSheet->getRange(u"A8"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(4.0, d);
    xSheet->getRange(u"E1"_ustr)->setValue(cpo::uno::Any(5.0));
    xSheet->deleteColumn(4);
    CPPUNIT_ASSERT(xSheet->getRange(u"D1"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(5.0, d);
}

CPPUNIT_TEST_FIXTURE(Test, testOffset)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    xSheet->getRange(u"B2"_ustr)->setValue(cpo::uno::Any(5.0));
    auto const xOffset = xSheet->getRange(u"A1"_ustr)->offset(1, 1, 1, 1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xOffset->getRow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xOffset->getColumn());
    double d = 0;
    CPPUNIT_ASSERT(xOffset->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(5.0, d);
}

CPPUNIT_TEST_FIXTURE(Test, testInputValidation)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    CPPUNIT_ASSERT_THROW(xSheet->getRange(u"not a range"_ustr), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xSheet->getRangeAt(0, 1, 1, 1), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xSheet->getRangeAtCell(1, 0), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xSheet->getRangeAtRows(1, 1, 0), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xSheet->deleteRow(0), cpo::uno::RuntimeException);
    xSheet->getRange(u"A1"_ustr)->setValue(cpo::uno::Any(9.0));
    // A rejected delete leaves the sheet unchanged.
    CPPUNIT_ASSERT_THROW(xSheet->deleteRows(1, 0), cpo::uno::RuntimeException);
    double d = 0;
    CPPUNIT_ASSERT(xSheet->getRange(u"A1"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(9.0, d);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
