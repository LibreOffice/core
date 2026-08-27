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

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XViewFreezable.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <cpo/uno/Reference.hxx>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
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
    CPPUNIT_ASSERT_THROW(xSheet->autoResizeColumn(0), cpo::uno::RuntimeException);
    CPPUNIT_ASSERT_THROW(xSheet->deleteRow(0), cpo::uno::RuntimeException);
    xSheet->getRange(u"A1"_ustr)->setValue(cpo::uno::Any(9.0));
    // A rejected delete leaves the sheet unchanged.
    CPPUNIT_ASSERT_THROW(xSheet->deleteRows(1, 0), cpo::uno::RuntimeException);
    double d = 0;
    CPPUNIT_ASSERT(xSheet->getRange(u"A1"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(9.0, d);
}

CPPUNIT_TEST_FIXTURE(Test, testGetSheetByName)
{
    auto const xSpreadsheet = loadSpreadsheet();
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, xSpreadsheet->getSheetByName(u"Sheet1"_ustr)->getName());
    // A name no sheet carries is not an error; the lookup reports nothing.
    CPPUNIT_ASSERT(!xSpreadsheet->getSheetByName(u"NoSuchSheet"_ustr).is());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertSheet)
{
    auto const xSpreadsheet = loadSpreadsheet();
    auto const xNewSheet = xSpreadsheet->insertSheet(u"Extra"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Extra"_ustr, xNewSheet->getName());
    CPPUNIT_ASSERT_EQUAL(u"Extra"_ustr, xSpreadsheet->getSheetByName(u"Extra"_ustr)->getName());
    // A duplicate sheet name is rejected.
    CPPUNIT_ASSERT_THROW(xSpreadsheet->insertSheet(u"Extra"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testGetRangeByNameAndOffset)
{
    auto const xSpreadsheet = loadSpreadsheet();
    cpo::uno::Reference<css::sheet::XSpreadsheetDocument> const xDoc(mxComponent,
                                                                     cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheets> const xSheets(xDoc->getSheets());
    xSheets->insertNewByName(u"Data"_ustr, 1);
    cpo::uno::Reference<css::container::XNameAccess> const xSheetsByName(xSheets,
                                                                         cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::table::XCellRange> xDataSheet;
    xSheetsByName->getByName(u"Data"_ustr) >>= xDataSheet;
    xDataSheet->getCellByPosition(0, 0)->setValue(1.0);
    xDataSheet->getCellByPosition(0, 1)->setValue(2.0);
    cpo::uno::Reference<css::beans::XPropertySet> const xDocProps(mxComponent,
                                                                   cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XNamedRanges> xNamedRanges;
    xDocProps->getPropertyValue(u"NamedRanges"_ustr) >>= xNamedRanges;
    xNamedRanges->addNewByName(u"FirstPlayer"_ustr, u"$Data.$A$1"_ustr,
                               css::table::CellAddress(0, 0, 0), 0);
    // The active sheet when the named range is resolved is not the sheet the range lives on.
    cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheetView> const xView(xModel->getCurrentController(),
                                                                  cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheet> xSheet1;
    xSheetsByName->getByName(u"Sheet1"_ustr) >>= xSheet1;
    xView->setActiveSheet(xSheet1);

    auto const xRange = xSpreadsheet->getRangeByName(u"FirstPlayer"_ustr);
    double d = 0;
    CPPUNIT_ASSERT(xRange->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(1.0, d);
    // offset() must resolve the range's own sheet (Data), not whichever sheet is active.
    auto const xBelow = xRange->offset(1, 0, 1, 1);
    CPPUNIT_ASSERT(xBelow->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(2.0, d);

    // A name that nothing in the document holds is not an error; the lookup reports nothing.
    CPPUNIT_ASSERT(!xSpreadsheet->getRangeByName(u"NoSuchRange"_ustr).is());
}

CPPUNIT_TEST_FIXTURE(Test, testGetRangeByNameFindsASheetScopedName)
{
    auto const xSpreadsheet = loadSpreadsheet();
    cpo::uno::Reference<css::sheet::XSpreadsheetDocument> const xDoc(mxComponent,
                                                                     cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheets> const xSheets(xDoc->getSheets());
    xSheets->insertNewByName(u"Rates"_ustr, 1);
    cpo::uno::Reference<css::container::XNameAccess> const xSheetsByName(xSheets,
                                                                         cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheet> xRates;
    xSheetsByName->getByName(u"Rates"_ustr) >>= xRates;
    cpo::uno::Reference<css::table::XCellRange> const xRatesCells(xRates,
                                                                   cpo::uno::UNO_QUERY_THROW);
    xRatesCells->getCellByPosition(0, 0)->setValue(7.0);
    // The name lives on the Rates sheet's own list, not on the document's.
    cpo::uno::Reference<css::beans::XPropertySet> const xSheetProps(xRates,
                                                                     cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XNamedRanges> xSheetRanges;
    xSheetProps->getPropertyValue(u"NamedRanges"_ustr) >>= xSheetRanges;
    xSheetRanges->addNewByName(u"TaxRates"_ustr, u"$Rates.$A$1"_ustr,
                               css::table::CellAddress(1, 0, 0), 0);

    double d = 0;
    CPPUNIT_ASSERT(xSpreadsheet->getRangeByName(u"TaxRates"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(7.0, d);
    // Naming the sheet in front of it reaches the same range.
    CPPUNIT_ASSERT(xSpreadsheet->getRangeByName(u"Rates!TaxRates"_ustr)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(7.0, d);
    // A sheet that holds no such name, and a sheet that is not there at all, both report nothing.
    CPPUNIT_ASSERT(!xSpreadsheet->getRangeByName(u"Sheet1!TaxRates"_ustr).is());
    CPPUNIT_ASSERT(!xSpreadsheet->getRangeByName(u"NoSuchSheet!TaxRates"_ustr).is());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertSheetWithDefaultName)
{
    auto const xSpreadsheet = loadSpreadsheet();
    // The document opens with Sheet1, so the next free name is Sheet2.
    auto const xFirst = xSpreadsheet->insertSheetWithDefaultName();
    CPPUNIT_ASSERT_EQUAL(u"Sheet2"_ustr, xFirst->getName());
    auto const xSecond = xSpreadsheet->insertSheetWithDefaultName();
    CPPUNIT_ASSERT_EQUAL(u"Sheet3"_ustr, xSecond->getName());
    // A name already taken is stepped over rather than collided with.
    xSpreadsheet->insertSheet(u"Sheet4"_ustr);
    auto const xThird = xSpreadsheet->insertSheetWithDefaultName();
    CPPUNIT_ASSERT_EQUAL(u"Sheet5"_ustr, xThird->getName());
}

CPPUNIT_TEST_FIXTURE(Test, testGetRangeByNameRejectsNonRangeExpression)
{
    auto const xSpreadsheet = loadSpreadsheet();
    cpo::uno::Reference<css::beans::XPropertySet> const xDocProps(mxComponent,
                                                                   cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XNamedRanges> xNamedRanges;
    xDocProps->getPropertyValue(u"NamedRanges"_ustr) >>= xNamedRanges;
    // A named range can also hold a formula that does not resolve to a plain cell range.
    xNamedRanges->addNewByName(u"NotARange"_ustr, u"1+1"_ustr, css::table::CellAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_THROW(xSpreadsheet->getRangeByName(u"NotARange"_ustr),
                         cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testSetBackgroundColor)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setBackgroundColor(u"#2a6099"_ustr);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xRange->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nColor = 0;
    xProps->getPropertyValue(u"CellBackColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x2a6099), nColor);
    // A malformed color string is rejected.
    CPPUNIT_ASSERT_THROW(xRange->setBackgroundColor(u"blue"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testSetColumnWidth)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    xSheet->setColumnWidth(1, 96);
    cpo::uno::Reference<css::table::XColumnRowRange> const xColumnRowRange(
        xSheet->getuno(), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::beans::XPropertySet> const xColumnProps(
        xColumnRowRange->getColumns()->getByIndex(0), cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nWidth = 0;
    xColumnProps->getPropertyValue(u"Width"_ustr) >>= nWidth;
    // 96 pixels is one inch, which is exactly 2540 in 1/100 mm; the round trip through twips
    // loses a little.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2540.0, static_cast<double>(nWidth), 5.0);
    CPPUNIT_ASSERT_THROW(xSheet->setColumnWidth(1, -1), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testClear)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    // A cell untouched by the test stands in for what "no color set" looks like, so the
    // assertion does not depend on knowing the engine's exact sentinel value for it.
    cpo::uno::Reference<css::beans::XPropertySet> const xUntouchedProps(
        xSheet->getRange(u"B1"_ustr)->getuno(), cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nDefaultColor = 0;
    bool const bDefaultHasColor
        = (xUntouchedProps->getPropertyValue(u"CellBackColor"_ustr) >>= nDefaultColor);

    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setValue(cpo::uno::Any(1.0));
    xRange->setBackgroundColor(u"#ff0000"_ustr);
    xSheet->clear();

    CPPUNIT_ASSERT(!xRange->getValue().hasValue());
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xRange->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nColor = 0;
    bool const bHasColor = (xProps->getPropertyValue(u"CellBackColor"_ustr) >>= nColor);
    CPPUNIT_ASSERT_EQUAL(bDefaultHasColor, bHasColor);
    if (bDefaultHasColor)
    {
        CPPUNIT_ASSERT_EQUAL(nDefaultColor, nColor);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testWritesOnNonActiveSheet)
{
    auto const xSpreadsheet = loadSpreadsheet();
    xSpreadsheet->insertSheet(u"Other"_ustr);
    // "Other" is not the active sheet for the rest of the test.
    cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheetView> const xView(xModel->getCurrentController(),
                                                                  cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheetDocument> const xDoc(mxComponent,
                                                                     cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::container::XNameAccess> const xSheetsByName(xDoc->getSheets(),
                                                                         cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::sheet::XSpreadsheet> xSheet1;
    xSheetsByName->getByName(u"Sheet1"_ustr) >>= xSheet1;
    xView->setActiveSheet(xSheet1);

    auto const xOther = xSpreadsheet->getSheetByName(u"Other"_ustr);
    xOther->getRangeAt(1, 1, 1, 1)->setValue(cpo::uno::Any(5.0));
    xOther->getRangeAt(1, 1, 1, 1)->setBackgroundColor(u"#ff0000"_ustr);
    xOther->setColumnWidth(2, 96);

    // Re-fetch by name, the way a second script invocation would, rather than reusing the same
    // wrapper - this rules out anything cached on the wrapper instead of committed to the model.
    auto const xOtherAgain = xSpreadsheet->getSheetByName(u"Other"_ustr);
    double d = 0;
    CPPUNIT_ASSERT(xOtherAgain->getRangeAt(1, 1, 1, 1)->getValue() >>= d);
    CPPUNIT_ASSERT_EQUAL(5.0, d);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(
        xOtherAgain->getRangeAt(1, 1, 1, 1)->getuno(), cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nColor = 0;
    xProps->getPropertyValue(u"CellBackColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), nColor);

    xOtherAgain->clear();
    CPPUNIT_ASSERT(!xOther->getRangeAt(1, 1, 1, 1)->getValue().hasValue());
}

CPPUNIT_TEST_FIXTURE(Test, testSetFontWeight)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setFontWeight(u"bold"_ustr);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xRange->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    float fWeight = 0;
    xProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(css::awt::FontWeight::BOLD, fWeight);
    xRange->setFontWeight(u"normal"_ustr);
    xProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(css::awt::FontWeight::NORMAL, fWeight);
    // Any other weight is refused, and the range keeps the weight it had.
    CPPUNIT_ASSERT_THROW(xRange->setFontWeight(u"bolder"_ustr), cpo::uno::RuntimeException);
    xProps->getPropertyValue(u"CharWeight"_ustr) >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(css::awt::FontWeight::NORMAL, fWeight);
}

CPPUNIT_TEST_FIXTURE(Test, testSetFontStyle)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setFontStyle(u"italic"_ustr);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xRange->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    css::awt::FontSlant eSlant = css::awt::FontSlant_NONE;
    xProps->getPropertyValue(u"CharPosture"_ustr) >>= eSlant;
    CPPUNIT_ASSERT_EQUAL(css::awt::FontSlant_ITALIC, eSlant);
    xRange->setFontStyle(u"normal"_ustr);
    xProps->getPropertyValue(u"CharPosture"_ustr) >>= eSlant;
    CPPUNIT_ASSERT_EQUAL(css::awt::FontSlant_NONE, eSlant);
    CPPUNIT_ASSERT_THROW(xRange->setFontStyle(u"oblique"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testSetFontColor)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setFontColor(u"#2a6099"_ustr);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xRange->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nColor = 0;
    xProps->getPropertyValue(u"CharColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x2a6099), nColor);
    CPPUNIT_ASSERT_THROW(xRange->setFontColor(u"blue"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testSetBorder)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setBorder(true, true, false, false, u"#000000"_ustr);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xRange->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    css::table::TableBorder2 aBorder;
    xProps->getPropertyValue(u"TableBorder2"_ustr) >>= aBorder;
    // Only the requested edges get an actual line; the others are left without one.
    CPPUNIT_ASSERT(aBorder.TopLine.LineWidth > 0);
    CPPUNIT_ASSERT(aBorder.LeftLine.LineWidth > 0);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aBorder.BottomLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aBorder.RightLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aBorder.TopLine.Color);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x000000), aBorder.LeftLine.Color);
}

CPPUNIT_TEST_FIXTURE(Test, testSetNumberFormat)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    auto const xRange = xSheet->getRange(u"A1"_ustr);
    xRange->setNumberFormat(u"0.00%"_ustr);
    cpo::uno::Reference<css::beans::XPropertySet> const xProps(xRange->getuno(),
                                                               cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nKey = 0;
    xProps->getPropertyValue(u"NumberFormat"_ustr) >>= nKey;
    // The same format string resolves to the same key, rather than creating a duplicate entry.
    cpo::uno::Reference<css::frame::XModel> const xModel(mxComponent, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::util::XNumberFormatsSupplier> const xSupplier(
        xModel, cpo::uno::UNO_QUERY_THROW);
    auto const xFormats = xSupplier->getNumberFormats();
    css::lang::Locale const aLocale;
    CPPUNIT_ASSERT_EQUAL(nKey, xFormats->queryKey(u"0.00%"_ustr, aLocale, false));
    // An unknown bracketed keyword is not a valid format code.
    CPPUNIT_ASSERT_THROW(xRange->setNumberFormat(u"[QQQ]0.00"_ustr), cpo::uno::RuntimeException);
}

CPPUNIT_TEST_FIXTURE(Test, testGetLastRowAndColumn)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    xSheet->getRange(u"B2"_ustr)->setValue(cpo::uno::Any(1.0));
    xSheet->getRange(u"D5"_ustr)->setValue(cpo::uno::Any(2.0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xSheet->getLastRow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xSheet->getLastColumn());
}

CPPUNIT_TEST_FIXTURE(Test, testAutoResizeColumnsAndRows)
{
    auto const xSheet = loadSpreadsheet()->getActiveSheet();
    xSheet->setColumnWidth(1, 200);
    xSheet->getRange(u"A1"_ustr)->setValue(cpo::uno::Any(u"hi"_ustr));
    cpo::uno::Reference<css::table::XColumnRowRange> const xColumnRowRange(
        xSheet->getuno(), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::beans::XPropertySet> const xColumnProps(
        xColumnRowRange->getColumns()->getByIndex(0), cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nWidthBefore = 0;
    xColumnProps->getPropertyValue(u"Width"_ustr) >>= nWidthBefore;
    xSheet->autoResizeColumns(1, 1);
    sal_Int32 nWidthAfter = 0;
    xColumnProps->getPropertyValue(u"Width"_ustr) >>= nWidthAfter;
    // The artificially wide column shrinks back down to fit its short content.
    CPPUNIT_ASSERT(nWidthAfter < nWidthBefore);

    // The singular form takes a column on its own and resizes just that one.
    xSheet->setColumnWidth(2, 200);
    xSheet->getRange(u"B1"_ustr)->setValue(cpo::uno::Any(u"hi"_ustr));
    cpo::uno::Reference<css::beans::XPropertySet> const xSecondColumnProps(
        xColumnRowRange->getColumns()->getByIndex(1), cpo::uno::UNO_QUERY_THROW);
    sal_Int32 nSecondBefore = 0;
    xSecondColumnProps->getPropertyValue(u"Width"_ustr) >>= nSecondBefore;
    xSheet->autoResizeColumn(2);
    sal_Int32 nSecondAfter = 0;
    xSecondColumnProps->getPropertyValue(u"Width"_ustr) >>= nSecondAfter;
    CPPUNIT_ASSERT(nSecondAfter < nSecondBefore);

    cpo::uno::Reference<css::beans::XPropertySet> const xRowProps(
        xColumnRowRange->getRows()->getByIndex(0), cpo::uno::UNO_QUERY_THROW);
    xSheet->autoResizeRows(1, 1);
    sal_Int32 nHeightAfter = 0;
    xRowProps->getPropertyValue(u"Height"_ustr) >>= nHeightAfter;
    CPPUNIT_ASSERT(nHeightAfter > 0);

    CPPUNIT_ASSERT_THROW(xSheet->autoResizeColumns(0, 1), cpo::uno::RuntimeException);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
