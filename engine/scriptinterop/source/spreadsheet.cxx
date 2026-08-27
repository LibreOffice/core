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

#include <utility>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XViewFreezable.hpp>
#include <com/sun/star/sheet/XViewSplitable.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Exception.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/Sequence.hxx>
#include <cpo/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <scriptinterop/XRange.hpp>
#include <scriptinterop/XSheet.hpp>
#include <scriptinterop/XSpreadsheet.hpp>

#include "spreadsheet.hxx"

namespace
{
// Reads a cell the way SpreadsheetApp-style code expects: a formula reports its computed result,
// not its text, and everything else reports its plain content.
//
// TODO: a number reports as a plain number, without the cell's number format. A date- or
// time-formatted cell therefore reports the serial number Calc keeps it as, where SpreadsheetApp
// hands back a date. Matching that means reading the number format here and returning a date.
cpo::uno::Any cellValue(cpo::uno::Reference<css::table::XCell> const& cell)
{
    switch (cell->getType())
    {
        case css::table::CellContentType_TEXT:
        {
            cpo::uno::Reference<css::text::XText> const text(cell, cpo::uno::UNO_QUERY_THROW);
            return cpo::uno::Any(text->getString());
        }
        case css::table::CellContentType_FORMULA:
        {
            cpo::uno::Reference<css::beans::XPropertySet> const props(cell,
                                                                       cpo::uno::UNO_QUERY_THROW);
            sal_Int32 resultType = css::sheet::FormulaResult::VALUE;
            props->getPropertyValue(u"FormulaResultType2"_ustr) >>= resultType;
            // A string result and a formula error both report through the cell's display
            // text (an error shows up there as "#DIV/0!" and the like); a plain value result
            // reports its computed number.
            if (resultType == css::sheet::FormulaResult::STRING
                || resultType == css::sheet::FormulaResult::ERROR)
            {
                cpo::uno::Reference<css::text::XText> const text(cell,
                                                                  cpo::uno::UNO_QUERY_THROW);
                return cpo::uno::Any(text->getString());
            }
            return cpo::uno::Any(cell->getValue());
        }
        case css::table::CellContentType_EMPTY:
            return cpo::uno::Any();
        default:
            return cpo::uno::Any(cell->getValue());
    }
}

// True for a value a cell can hold: a number, a boolean, a string, or an empty value, which
// stands for a cleared cell. A value of any other type has no cell representation at all.
bool isWritableValue(cpo::uno::Any const& value)
{
    double numberValue = 0;
    bool boolValue = false;
    OUString stringValue;
    return (value >>= numberValue) || (value >>= boolValue) || (value >>= stringValue)
           || !value.hasValue();
}

// Writes a value the way typing it into the cell would: a string goes through setFormula, which
// itself decides between a formula, a value or a text cell depending on whether it starts with
// "=" and how the rest parses. A number sets a value cell directly. A boolean becomes 1 or 0 -
// good enough until a real boolean-typed cell is needed. An empty value clears the cell.
void setCellValue(cpo::uno::Reference<css::table::XCell> const& cell, cpo::uno::Any const& value)
{
    double numberValue = 0;
    bool boolValue = false;
    OUString stringValue;
    if (value >>= numberValue)
    {
        cell->setValue(numberValue);
    }
    else if (value >>= boolValue)
    {
        cell->setValue(boolValue ? 1.0 : 0.0);
    }
    else if (value >>= stringValue)
    {
        cell->setFormula(stringValue);
    }
    else if (!value.hasValue())
    {
        cell->setFormula(OUString());
    }
    else
    {
        throw cpo::uno::RuntimeException(u"expected a number, string or boolean"_ustr);
    }
}

class RangeImpl;

// Builds the range at a 1-based row/column/size, checked before the sheet is asked for it.
cpo::uno::Reference<scriptinterop::XRange>
rangeAt(cpo::uno::Reference<css::sheet::XSpreadsheet> const& sheet, sal_Int32 row, sal_Int32 column,
        sal_Int32 numRows, sal_Int32 numColumns);

class RangeImpl : public cppu::WeakImplHelper<scriptinterop::XRange>
{
public:
    RangeImpl(cpo::uno::Reference<css::sheet::XSpreadsheet> const& sheet,
              cpo::uno::Reference<css::table::XCellRange> const& range)
        : sheet_(sheet)
        , range_(range)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return range_; }

    sal_Int32 SAL_CALL getRow() override { return address().StartRow + 1; }

    sal_Int32 SAL_CALL getColumn() override { return address().StartColumn + 1; }

    sal_Int32 SAL_CALL getNumRows() override
    {
        auto const a = address();
        return a.EndRow - a.StartRow + 1;
    }

    sal_Int32 SAL_CALL getNumColumns() override
    {
        auto const a = address();
        return a.EndColumn - a.StartColumn + 1;
    }

    cpo::uno::Any SAL_CALL getValue() override
    {
        try
        {
            return cellValue(topLeftCell());
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getValue: ") + e.Message);
        }
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL setValue(cpo::uno::Any const& value) override
    {
        try
        {
            // Fills every cell of the range with the same value, not just the top-left one:
            // getValue() reads a single cell, but setValue() is the range-wide fill counterpart
            // to it.
            auto const a = address();
            auto const numRows = a.EndRow - a.StartRow + 1;
            auto const numColumns = a.EndColumn - a.StartColumn + 1;
            for (sal_Int32 r = 0; r != numRows; ++r)
            {
                for (sal_Int32 c = 0; c != numColumns; ++c)
                {
                    setCellValue(range_->getCellByPosition(c, r), value);
                }
            }
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setValue: ") + e.Message);
        }
        return this;
    }

    cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> SAL_CALL getValues() override
    {
        try
        {
            // Cell by cell, through the same cellValue() a single-cell getValue() uses, rather
            // than XCellRangeData::getDataArray(): that call reports a formula error as an
            // empty value instead of the error text.
            auto const a = address();
            auto const numRows = a.EndRow - a.StartRow + 1;
            auto const numColumns = a.EndColumn - a.StartColumn + 1;
            cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> result(numRows);
            auto* const rows = result.getArray();
            for (sal_Int32 r = 0; r != numRows; ++r)
            {
                cpo::uno::Sequence<cpo::uno::Any> row(numColumns);
                auto* const cells = row.getArray();
                for (sal_Int32 c = 0; c != numColumns; ++c)
                {
                    cells[c] = cellValue(range_->getCellByPosition(c, r));
                }
                rows[r] = std::move(row);
            }
            return result;
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getValues: ") + e.Message);
        }
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL setValues(cpo::uno::Sequence<cpo::uno::Sequence<cpo::uno::Any>> const& values)
        override
    {
        // The array has to match the range's own shape: there is no sensible way to fill in
        // what a short array leaves out, and a long one names cells outside the range.
        auto const a = address();
        auto const expectedRows = a.EndRow - a.StartRow + 1;
        auto const expectedColumns = a.EndColumn - a.StartColumn + 1;
        auto const numRows = values.getLength();
        if (numRows != expectedRows)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setValues: expected ")
                                             + OUString::number(expectedRows)
                                             + " rows to match the range, got "
                                             + OUString::number(numRows));
        }
        // The whole array is checked before any cell is written: its shape first, then the type
        // of every value in it. A rejected call leaves the range exactly as it was, and the
        // error names the row and the column of the value at fault.
        for (sal_Int32 r = 0; r != numRows; ++r)
        {
            auto const& rowValues = values[r];
            if (rowValues.getLength() != expectedColumns)
            {
                throw cpo::uno::RuntimeException(OUString::Concat("setValues: expected ")
                                                 + OUString::number(expectedColumns)
                                                 + " columns to match the range, got "
                                                 + OUString::number(rowValues.getLength()));
            }
            for (sal_Int32 c = 0; c != expectedColumns; ++c)
            {
                if (!isWritableValue(rowValues[c]))
                {
                    throw cpo::uno::RuntimeException(
                        OUString::Concat("setValues: expected a number, string or boolean at row ")
                        + OUString::number(r + 1) + ", column " + OUString::number(c + 1));
                }
            }
        }
        try
        {
            // Cell by cell, not through XCellRangeData::setDataArray: that call stores every
            // string as literal text, without the formula detection setValue gives a single
            // cell. A failure from Calc partway through leaves the cells before it written.
            // Restoring those would mean copying the range out first.
            for (sal_Int32 r = 0; r != numRows; ++r)
            {
                auto const& rowValues = values[r];
                for (sal_Int32 c = 0; c != expectedColumns; ++c)
                {
                    setCellValue(range_->getCellByPosition(c, r), rowValues[c]);
                }
            }
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setValues: ") + e.Message);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL offset(sal_Int32 rowOffset, sal_Int32 columnOffset, sal_Int32 numRows,
                       sal_Int32 numColumns) override
    {
        auto const a = address();
        return rangeAt(sheet_, a.StartRow + rowOffset + 1, a.StartColumn + columnOffset + 1,
                       numRows, numColumns);
    }

private:
    css::table::CellRangeAddress address()
    {
        cpo::uno::Reference<css::sheet::XCellRangeAddressable> const addr(
            range_, cpo::uno::UNO_QUERY_THROW);
        return addr->getRangeAddress();
    }

    cpo::uno::Reference<css::table::XCell> topLeftCell() { return range_->getCellByPosition(0, 0); }

    cpo::uno::Reference<css::sheet::XSpreadsheet> sheet_;
    cpo::uno::Reference<css::table::XCellRange> range_;
};

cpo::uno::Reference<scriptinterop::XRange>
rangeAt(cpo::uno::Reference<css::sheet::XSpreadsheet> const& sheet, sal_Int32 row, sal_Int32 column,
        sal_Int32 numRows, sal_Int32 numColumns)
{
    if (row < 1 || column < 1 || numRows < 1 || numColumns < 1)
    {
        throw cpo::uno::RuntimeException(
            u"expected a row, column, row count and column count of at least 1"_ustr);
    }
    cpo::uno::Reference<css::table::XCellRange> const sheetRange(sheet, cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::table::XCellRange> range;
    try
    {
        range = sheetRange->getCellRangeByPosition(column - 1, row - 1, column + numColumns - 2,
                                                    row + numRows - 2);
    }
    catch (cpo::uno::Exception const& e)
    {
        throw cpo::uno::RuntimeException(OUString::Concat("expected a range that fits the sheet: ")
                                         + e.Message);
    }
    return new RangeImpl(sheet, range);
}

class SheetImpl : public cppu::WeakImplHelper<scriptinterop::XSheet>
{
public:
    SheetImpl(cpo::uno::Reference<css::frame::XModel> const& model,
              cpo::uno::Reference<css::sheet::XSpreadsheet> const& sheet)
        : model_(model)
        , sheet_(sheet)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return sheet_; }

    OUString SAL_CALL getName() override
    {
        cpo::uno::Reference<css::container::XNamed> const named(sheet_, cpo::uno::UNO_QUERY_THROW);
        return named->getName();
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL getRange(OUString const& a1Notation) override
    {
        cpo::uno::Reference<css::table::XCellRange> const sheetRange(sheet_,
                                                                     cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::table::XCellRange> range;
        try
        {
            range = sheetRange->getCellRangeByName(a1Notation);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getRange: ") + e.Message);
        }
        if (!range.is())
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat("getRange: expected a valid A1-style range, got ") + a1Notation);
        }
        return new RangeImpl(sheet_, range);
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getRangeAtCell(sal_Int32 row,
                                                                       sal_Int32 column) override
    {
        return rangeAt(sheet_, row, column, 1, 1);
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL getRangeAtRows(sal_Int32 row, sal_Int32 column, sal_Int32 numRows) override
    {
        return rangeAt(sheet_, row, column, numRows, 1);
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getRangeAt(sal_Int32 row, sal_Int32 column,
                                                                   sal_Int32 numRows,
                                                                   sal_Int32 numColumns) override
    {
        return rangeAt(sheet_, row, column, numRows, numColumns);
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getActiveRange() override
    {
        return new RangeImpl(sheet_, activeCellRange());
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getActiveCell() override
    {
        // The top-left cell of the first selected range stands in for "the active cell".
        auto const range = activeCellRange();
        return new RangeImpl(sheet_, range->getCellRangeByPosition(0, 0, 0, 0));
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getDataRange() override
    {
        auto const cursor = sheet_->createCursor();
        cpo::uno::Reference<css::sheet::XUsedAreaCursor> const used(cursor,
                                                                    cpo::uno::UNO_QUERY_THROW);
        used->gotoStartOfUsedArea(false);
        used->gotoEndOfUsedArea(true);
        cpo::uno::Reference<css::sheet::XCellRangeAddressable> const addr(
            cursor, cpo::uno::UNO_QUERY_THROW);
        auto const a = addr->getRangeAddress();
        // The cursor itself cannot take bulk value reads/writes; a genuine sub-range at the same
        // address can.
        cpo::uno::Reference<css::table::XCellRange> const sheetRange(sheet_,
                                                                     cpo::uno::UNO_QUERY_THROW);
        auto const range
            = sheetRange->getCellRangeByPosition(a.StartColumn, a.StartRow, a.EndColumn, a.EndRow);
        return new RangeImpl(sheet_, range);
    }

    sal_Int32 SAL_CALL getMaxRows() override { return columnRowRange()->getRows()->getCount(); }

    sal_Int32 SAL_CALL getMaxColumns() override
    {
        return columnRowRange()->getColumns()->getCount();
    }

    sal_Int32 SAL_CALL getFrozenRows() override { return frozenPanes().first; }

    sal_Int32 SAL_CALL getFrozenColumns() override { return frozenPanes().second; }

    void SAL_CALL deleteRow(sal_Int32 row) override { removeRows(row, 1, u"deleteRow"_ustr); }

    void SAL_CALL deleteRows(sal_Int32 startRow, sal_Int32 numRows) override
    {
        removeRows(startRow, numRows, u"deleteRows"_ustr);
    }

    void SAL_CALL deleteColumn(sal_Int32 column) override
    {
        removeColumns(column, 1, u"deleteColumn"_ustr);
    }

    void SAL_CALL deleteColumns(sal_Int32 startColumn, sal_Int32 numColumns) override
    {
        removeColumns(startColumn, numColumns, u"deleteColumns"_ustr);
    }

private:
    void removeRows(sal_Int32 startRow, sal_Int32 numRows, std::u16string_view name)
    {
        if (startRow < 1 || numRows < 1)
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat(name) + ": expected a row and count of at least 1");
        }
        try
        {
            columnRowRange()->getRows()->removeByIndex(startRow - 1, numRows);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat(name) + ": " + e.Message);
        }
    }

    void removeColumns(sal_Int32 startColumn, sal_Int32 numColumns,
                       std::u16string_view name)
    {
        if (startColumn < 1 || numColumns < 1)
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat(name) + ": expected a column and count of at least 1");
        }
        try
        {
            columnRowRange()->getColumns()->removeByIndex(startColumn - 1, numColumns);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat(name) + ": " + e.Message);
        }
    }

    cpo::uno::Reference<css::table::XColumnRowRange> columnRowRange()
    {
        return cpo::uno::Reference<css::table::XColumnRowRange>(sheet_, cpo::uno::UNO_QUERY_THROW);
    }

    // The selection tracked by the controller belongs to the document, not to any one XSheet
    // object: it can sit on a sheet other than the one this SheetImpl wraps, for instance when a
    // script keeps an XSheet from before the user switched sheets. Reject a selection that is not
    // on this sheet rather than silently handing back another sheet's range.
    cpo::uno::Reference<css::table::XCellRange> activeCellRange()
    {
        cpo::uno::Reference<css::view::XSelectionSupplier> const sup(
            model_->getCurrentController(), cpo::uno::UNO_QUERY);
        cpo::uno::Reference<css::table::XCellRange> range;
        if (sup.is())
        {
            auto const selection = sup->getSelection();
            // A single selected range comes back directly; a multi-range selection comes back as
            // an indexed container of ranges, and the first one stands in for "the" active range.
            selection >>= range;
            if (!range.is())
            {
                cpo::uno::Reference<css::container::XIndexAccess> ranges;
                selection >>= ranges;
                if (ranges.is() && ranges->getCount() > 0)
                {
                    ranges->getByIndex(0) >>= range;
                }
            }
        }
        if (!range.is())
        {
            throw cpo::uno::RuntimeException(
                u"getActiveRange: no selection in the active sheet"_ustr);
        }
        cpo::uno::Reference<css::sheet::XCellRangeAddressable> const rangeAddr(
            range, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::sheet::XCellRangeAddressable> const sheetAddr(
            sheet_, cpo::uno::UNO_QUERY_THROW);
        if (rangeAddr->getRangeAddress().Sheet != sheetAddr->getRangeAddress().Sheet)
        {
            throw cpo::uno::RuntimeException(
                u"getActiveRange: the selection is on a different sheet"_ustr);
        }
        return range;
    }

    // Returns {frozen rows, frozen columns}, or {0, 0} when the view has no frozen panes.
    std::pair<sal_Int32, sal_Int32> frozenPanes()
    {
        auto const controller = model_->getCurrentController();
        cpo::uno::Reference<css::sheet::XViewFreezable> const freezable(controller,
                                                                        cpo::uno::UNO_QUERY);
        if (!freezable.is() || !freezable->hasFrozenPanes())
        {
            return { 0, 0 };
        }
        cpo::uno::Reference<css::sheet::XViewSplitable> const splitable(controller,
                                                                        cpo::uno::UNO_QUERY_THROW);
        return { splitable->getSplitRow(), splitable->getSplitColumn() };
    }

    cpo::uno::Reference<css::frame::XModel> model_;
    cpo::uno::Reference<css::sheet::XSpreadsheet> sheet_;
};

class SpreadsheetImpl : public cppu::WeakImplHelper<scriptinterop::XSpreadsheet>
{
public:
    explicit SpreadsheetImpl(cpo::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return model_; }

    cpo::uno::Reference<scriptinterop::XSheet> SAL_CALL getActiveSheet() override
    {
        cpo::uno::Reference<css::sheet::XSpreadsheetView> const view(model_->getCurrentController(),
                                                                     cpo::uno::UNO_QUERY_THROW);
        auto const sheet = view->getActiveSheet();
        if (!sheet.is())
        {
            throw cpo::uno::RuntimeException(u"getActiveSheet: no active sheet"_ustr);
        }
        return new SheetImpl(model_, sheet);
    }

    // Recalculates every formula in the document, including ones a script's writes did not
    // touch. This is a full recalculation, not the narrower "commit pending changes" flush of
    // some other spreadsheet scripting APIs.
    void SAL_CALL flush() override
    {
        cpo::uno::Reference<css::sheet::XCalculatable> const calc(model_,
                                                                  cpo::uno::UNO_QUERY_THROW);
        calc->calculateAll();
    }

private:
    cpo::uno::Reference<css::frame::XModel> model_;
};
}

namespace scriptinterop::detail
{
cpo::uno::Reference<scriptinterop::XSpreadsheet>
createSpreadsheet(cpo::uno::Reference<css::frame::XModel> const& model)
{
    return new SpreadsheetImpl(model);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
