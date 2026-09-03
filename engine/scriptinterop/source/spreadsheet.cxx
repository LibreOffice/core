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

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XDiagram.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/FormulaResult.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XViewFreezable.hpp>
#include <com/sun/star/sheet/XViewSplitable.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/text/XText.hpp>
#include <cpo/uno/Reference.hxx>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Exception.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/Sequence.hxx>
#include <cpo/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <scriptinterop/XChart.hpp>
#include <scriptinterop/XPivotTable.hpp>
#include <scriptinterop/XRange.hpp>
#include <scriptinterop/XSheet.hpp>
#include <scriptinterop/XSpreadsheet.hpp>

#include "conversions.hxx"
#include "spreadsheet.hxx"

using scriptinterop::detail::extentToHundredthMm;
using scriptinterop::detail::parseHexColor;
using scriptinterop::detail::pixelsToHundredthMm;

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

cpo::uno::Reference<css::sheet::XSpreadsheets>
documentSheets(cpo::uno::Reference<css::frame::XModel> const& model)
{
    cpo::uno::Reference<css::sheet::XSpreadsheetDocument> const doc(model,
                                                                    cpo::uno::UNO_QUERY_THROW);
    return doc->getSheets();
}

// Calc gives a chart and a pivot table a name of its own, and both have to be free across the
// whole document rather than only on the sheet the new one goes on, because both live in pools
// the document shares. A name another sheet has taken makes Calc refuse a chart outright and
// drop a pivot table without a word, so the collections of every sheet are asked here.
OUString freeName(std::u16string_view prefix,
                  std::vector<cpo::uno::Reference<css::container::XNameAccess>> const& taken)
{
    for (sal_Int32 i = 1;; ++i)
    {
        OUString const name = prefix + OUString::number(i);
        if (std::none_of(taken.begin(), taken.end(),
                         [&name](auto const& names) { return names->hasByName(name); }))
        {
            return name;
        }
    }
}

// SpreadsheetApp-style code names a sheet in front of the cells with "!", as in "Data!A1:B2",
// and single-quotes a name holding spaces or a "!" of its own, as in "'Rate data'!A1". Calc's
// own parser writes that same reference as "Data.A1", so a qualified name is split here and
// resolved against the sheet it names. The last "!" is the separator, which keeps a quoted
// name containing one intact. Sets cells either way, to the whole name when it carries no
// sheet, and reports whether there was a sheet to split off at all.
bool splitSheetQualifiedName(OUString const& a1Notation, OUString& sheetName, OUString& cells)
{
    auto const separator = a1Notation.lastIndexOf('!');
    if (separator < 0)
    {
        cells = a1Notation;
        return false;
    }
    auto name = a1Notation.copy(0, separator);
    if (name.getLength() >= 2 && name.startsWith(u"'"_ustr) && name.endsWith(u"'"_ustr))
    {
        // A quoted name carries its own quotes doubled, the way Calc and SpreadsheetApp both
        // write them.
        name = name.copy(1, name.getLength() - 2).replaceAll(u"''"_ustr, u"'"_ustr);
    }
    sheetName = name;
    cells = a1Notation.copy(separator + 1);
    return true;
}

// SpreadsheetApp names the way a pivot value is summarized after its own
// PivotTableSummarizeFunction constants. Two of those names cross over against Calc's: what
// SpreadsheetApp calls COUNTA, counting every non-empty value, is Calc's COUNT, and what it
// calls COUNT, counting only numbers, is Calc's COUNTNUMS.
css::sheet::GeneralFunction summarizeFunction(OUString const& name, std::u16string_view caller)
{
    auto const wanted = name.toAsciiLowerCase();
    if (wanted == "sum")
    {
        return css::sheet::GeneralFunction_SUM;
    }
    if (wanted == "counta")
    {
        return css::sheet::GeneralFunction_COUNT;
    }
    if (wanted == "count")
    {
        return css::sheet::GeneralFunction_COUNTNUMS;
    }
    if (wanted == "average")
    {
        return css::sheet::GeneralFunction_AVERAGE;
    }
    if (wanted == "max")
    {
        return css::sheet::GeneralFunction_MAX;
    }
    if (wanted == "min")
    {
        return css::sheet::GeneralFunction_MIN;
    }
    if (wanted == "product")
    {
        return css::sheet::GeneralFunction_PRODUCT;
    }
    if (wanted == "stdev")
    {
        return css::sheet::GeneralFunction_STDEV;
    }
    if (wanted == "stdevp")
    {
        return css::sheet::GeneralFunction_STDEVP;
    }
    if (wanted == "var")
    {
        return css::sheet::GeneralFunction_VAR;
    }
    if (wanted == "varp")
    {
        return css::sheet::GeneralFunction_VARP;
    }
    // SpreadsheetApp also offers countunique, median and a custom formula, which Calc's pivot
    // tables have no equivalent for.
    throw cpo::uno::RuntimeException(
        OUString::Concat(caller)
        + ": expected sum, counta, count, average, max, min, product, stdev, stdevp, "
          "var or varp, got "
        + name);
}

// A pivot table keeps the collection it lives in and its own name rather than the table, for
// the same reason a chart does: the table has to be looked up again for each call so that one
// somebody has meanwhile deleted is noticed. Pivot names are per sheet, unlike chart names.
class PivotTableImpl : public cppu::WeakImplHelper<scriptinterop::XPivotTable>
{
public:
    PivotTableImpl(cpo::uno::Reference<css::sheet::XDataPilotTables> const& tables,
                   OUString const& name)
        : tables_(tables)
        , name_(name)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return liveTable(); }

    OUString SAL_CALL getName() override { return name_; }

    cpo::uno::Reference<scriptinterop::XPivotTable> SAL_CALL addRowGroup(sal_Int32 sourceColumn)
        override
    {
        orient(sourceColumn, css::sheet::DataPilotFieldOrientation_ROW, u"addRowGroup"_ustr);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPivotTable> SAL_CALL addColumnGroup(sal_Int32 sourceColumn)
        override
    {
        orient(sourceColumn, css::sheet::DataPilotFieldOrientation_COLUMN,
               u"addColumnGroup"_ustr);
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPivotTable>
        SAL_CALL addPivotValue(sal_Int32 sourceColumn, OUString const& function) override
    {
        auto const wanted = summarizeFunction(function, u"addPivotValue"_ustr);
        try
        {
            auto const props = fieldProperties(sourceColumn, u"addPivotValue"_ustr);
            props->setPropertyValue(u"Orientation"_ustr,
                                    cpo::uno::Any(css::sheet::DataPilotFieldOrientation_DATA));
            props->setPropertyValue(u"Function"_ustr, cpo::uno::Any(wanted));
            refresh();
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("addPivotValue: ") + e.Message);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPivotTable>
        SAL_CALL addFilter(sal_Int32 sourceColumn, OUString const& visibleValue) override
    {
        try
        {
            auto const props = fieldProperties(sourceColumn, u"addFilter"_ustr);
            props->setPropertyValue(u"Orientation"_ustr,
                                    cpo::uno::Any(css::sheet::DataPilotFieldOrientation_PAGE));
            // The page field shows one value at a time, which is the part of SpreadsheetApp's
            // richer filter criteria that a pivot page field can carry.
            props->setPropertyValue(u"SelectedPage"_ustr, cpo::uno::Any(visibleValue));
            props->setPropertyValue(u"UseSelectedPage"_ustr, cpo::uno::Any(true));
            refresh();
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("addFilter: ") + e.Message);
        }
        return this;
    }

    void SAL_CALL remove() override
    {
        try
        {
            tables_->removeByName(name_);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("remove: ") + e.Message);
        }
    }

private:
    cpo::uno::Reference<css::sheet::XDataPilotTable> liveTable()
    {
        if (!tables_->hasByName(name_))
        {
            throw cpo::uno::RuntimeException(OUString::Concat("the pivot table ") + name_
                                             + " is no longer on its sheet");
        }
        return cpo::uno::Reference<css::sheet::XDataPilotTable>(tables_->getByName(name_),
                                                                  cpo::uno::UNO_QUERY_THROW);
    }

    // SpreadsheetApp counts the source columns from 1, and the fields sit in that same order.
    // The field list carries one more entry than the source has columns, a synthetic "Data"
    // field that stands for the values themselves, so the source range decides what a caller
    // may ask for rather than the length of that list.
    cpo::uno::Reference<css::beans::XPropertySet> fieldProperties(sal_Int32 sourceColumn,
                                                                    std::u16string_view caller)
    {
        cpo::uno::Reference<css::sheet::XDataPilotDescriptor> const descriptor(
            liveTable(), cpo::uno::UNO_QUERY_THROW);
        auto const source = descriptor->getSourceRange();
        auto const columns = source.EndColumn - source.StartColumn + 1;
        if (sourceColumn < 1 || sourceColumn > columns)
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat(caller) + ": expected a source column between 1 and "
                + OUString::number(columns) + ", got " + OUString::number(sourceColumn));
        }
        return cpo::uno::Reference<css::beans::XPropertySet>(
            descriptor->getDataPilotFields()->getByIndex(sourceColumn - 1),
            cpo::uno::UNO_QUERY_THROW);
    }

    void orient(sal_Int32 sourceColumn, css::sheet::DataPilotFieldOrientation orientation,
                std::u16string_view caller)
    {
        try
        {
            fieldProperties(sourceColumn, caller)
                ->setPropertyValue(u"Orientation"_ustr, cpo::uno::Any(orientation));
            refresh();
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat(caller) + ": " + e.Message);
        }
    }

    // The output only catches up with a changed field once the table is told to rebuild it.
    void refresh() { liveTable()->refresh(); }

    cpo::uno::Reference<css::sheet::XDataPilotTables> tables_;
    OUString name_;
};

// Builds the range at a 1-based row/column/size, checked before the sheet is asked for it.
cpo::uno::Reference<scriptinterop::XRange>
rangeAt(cpo::uno::Reference<css::frame::XModel> const& model,
        cpo::uno::Reference<css::sheet::XSpreadsheet> const& sheet, sal_Int32 row, sal_Int32 column,
        sal_Int32 numRows, sal_Int32 numColumns);

class RangeImpl : public cppu::WeakImplHelper<scriptinterop::XRange>
{
public:
    // The range keeps the document, not the sheet it happened to come from: a named range can
    // refer to cells on a sheet other than whichever one was active when it was looked up, so the
    // owning sheet is resolved from the range's own address instead of being trusted from the
    // caller.
    RangeImpl(cpo::uno::Reference<css::frame::XModel> const& model,
              cpo::uno::Reference<css::table::XCellRange> const& range)
        : model_(model)
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
        return rangeAt(model_, sheetAt(a.Sheet), a.StartRow + rowOffset + 1,
                       a.StartColumn + columnOffset + 1, numRows, numColumns);
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL setBackgroundColor(OUString const& hexColor) override
    {
        cpo::uno::Reference<css::beans::XPropertySet> const props(range_,
                                                                  cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"CellBackColor"_ustr, cpo::uno::Any(parseHexColor(hexColor)));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL setFontWeight(OUString const& fontWeight) override
    {
        float weight = css::awt::FontWeight::NORMAL;
        if (fontWeight == u"bold")
        {
            weight = css::awt::FontWeight::BOLD;
        }
        else if (fontWeight != u"normal")
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat("setFontWeight: expected \"bold\" or \"normal\", got ")
                + fontWeight);
        }
        cpo::uno::Reference<css::beans::XPropertySet> const props(range_,
                                                                  cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"CharWeight"_ustr, cpo::uno::Any(weight));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL setFontStyle(OUString const& fontStyle) override
    {
        auto slant = css::awt::FontSlant_NONE;
        if (fontStyle == u"italic")
        {
            slant = css::awt::FontSlant_ITALIC;
        }
        else if (fontStyle != u"normal")
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat("setFontStyle: expected \"italic\" or \"normal\", got ")
                + fontStyle);
        }
        cpo::uno::Reference<css::beans::XPropertySet> const props(range_,
                                                                  cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"CharPosture"_ustr, cpo::uno::Any(slant));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL setFontColor(OUString const& hexColor)
        override
    {
        cpo::uno::Reference<css::beans::XPropertySet> const props(range_,
                                                                  cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"CharColor"_ustr, cpo::uno::Any(parseHexColor(hexColor)));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL setBorder(bool top, bool left, bool bottom, bool right, OUString const& hexColor)
        override
    {
        // A single fixed solid, medium-width line for every requested edge; no style or
        // thickness parameter yet, and the internal vertical/horizontal gridlines are never
        // touched. An edge whose flag is false is left as it already was, not cleared: this
        // can only add borders, not remove ones a cell already has.
        css::table::BorderLine2 line;
        line.Color = parseHexColor(hexColor);
        line.InnerLineWidth = 0;
        line.OuterLineWidth = 35;
        line.LineDistance = 0;
        line.LineStyle = css::table::BorderLineStyle::SOLID;
        line.LineWidth = 35;
        css::table::TableBorder2 border;
        border.TopLine = line;
        border.IsTopLineValid = top;
        border.BottomLine = line;
        border.IsBottomLineValid = bottom;
        border.LeftLine = line;
        border.IsLeftLineValid = left;
        border.RightLine = line;
        border.IsRightLineValid = right;
        border.IsVerticalLineValid = false;
        border.IsHorizontalLineValid = false;
        border.IsDistanceValid = false;
        cpo::uno::Reference<css::beans::XPropertySet> const props(range_,
                                                                  cpo::uno::UNO_QUERY_THROW);
        props->setPropertyValue(u"TableBorder2"_ustr, cpo::uno::Any(border));
        return this;
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL setNumberFormat(OUString const& format)
        override
    {
        try
        {
            cpo::uno::Reference<css::util::XNumberFormatsSupplier> const supplier(
                model_, cpo::uno::UNO_QUERY_THROW);
            auto const formats = supplier->getNumberFormats();
            // The empty locale matches the format codes a script writes literally (like "0.00%"),
            // the same codes across every document language rather than the locale-specific ones
            // a user would type into the Format Cells dialog.
            css::lang::Locale const locale;
            auto key = formats->queryKey(format, locale, false);
            if (key == -1)
            {
                key = formats->addNew(format, locale);
            }
            cpo::uno::Reference<css::beans::XPropertySet> const props(range_,
                                                                       cpo::uno::UNO_QUERY_THROW);
            props->setPropertyValue(u"NumberFormat"_ustr, cpo::uno::Any(key));
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setNumberFormat: ") + e.Message);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XPivotTable>
        SAL_CALL createPivotTable(cpo::uno::Reference<scriptinterop::XRange> const& sourceData)
        override
    {
        if (!sourceData.is())
        {
            throw cpo::uno::RuntimeException(u"createPivotTable: expected a source range"_ustr);
        }
        css::table::CellRangeAddress source;
        auto const anchor = address();
        cpo::uno::Reference<css::sheet::XDataPilotTables> tables;
        OUString name;
        try
        {
            cpo::uno::Reference<css::sheet::XCellRangeAddressable> const addressable(
                sourceData->getuno(), cpo::uno::UNO_QUERY_THROW);
            source = addressable->getRangeAddress();
            // The table goes on the sheet this range sits on, while its source may live on
            // another one.
            cpo::uno::Reference<css::sheet::XDataPilotTablesSupplier> const supplier(
                sheetAt(anchor.Sheet), cpo::uno::UNO_QUERY_THROW);
            tables = supplier->getDataPilotTables();
            name = freePivotTableName();
            auto const descriptor = tables->createDataPilotDescriptor();
            descriptor->setSourceRange(source);
            // The table starts empty, the way SpreadsheetApp hands one back before any group
            // or value has been added to it.
            tables->insertNewByName(
                name, css::table::CellAddress(anchor.Sheet, anchor.StartColumn, anchor.StartRow),
                descriptor);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("createPivotTable: ") + e.Message);
        }
        if (!tables->hasByName(name))
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat("createPivotTable: the table ") + name + " was not created");
        }
        return new PivotTableImpl(tables, name);
    }

private:
    OUString freePivotTableName()
    {
        cpo::uno::Reference<css::container::XIndexAccess> const sheets(
            documentSheets(model_), cpo::uno::UNO_QUERY_THROW);
        auto const sheetCount = sheets->getCount();
        std::vector<cpo::uno::Reference<css::container::XNameAccess>> taken;
        taken.reserve(sheetCount);
        for (sal_Int32 i = 0; i != sheetCount; ++i)
        {
            cpo::uno::Reference<css::sheet::XDataPilotTablesSupplier> const supplier(
                sheets->getByIndex(i), cpo::uno::UNO_QUERY_THROW);
            taken.emplace_back(supplier->getDataPilotTables(), cpo::uno::UNO_QUERY_THROW);
        }
        return freeName(u"Pivot "_ustr, taken);
    }

    css::table::CellRangeAddress address()
    {
        cpo::uno::Reference<css::sheet::XCellRangeAddressable> const addr(
            range_, cpo::uno::UNO_QUERY_THROW);
        return addr->getRangeAddress();
    }

    cpo::uno::Reference<css::table::XCell> topLeftCell() { return range_->getCellByPosition(0, 0); }

    cpo::uno::Reference<css::sheet::XSpreadsheet> sheetAt(sal_Int16 tab)
    {
        cpo::uno::Reference<css::container::XIndexAccess> const sheets(
            documentSheets(model_), cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::sheet::XSpreadsheet> sheet;
        sheets->getByIndex(tab) >>= sheet;
        return sheet;
    }

    cpo::uno::Reference<css::frame::XModel> model_;
    cpo::uno::Reference<css::table::XCellRange> range_;
};

cpo::uno::Reference<scriptinterop::XRange>
rangeAt(cpo::uno::Reference<css::frame::XModel> const& model,
        cpo::uno::Reference<css::sheet::XSpreadsheet> const& sheet, sal_Int32 row, sal_Int32 column,
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
    return new RangeImpl(model, range);
}

// SpreadsheetApp-style code names a chart type the way its own Charts.ChartType constants do,
// and each of those names one of Calc's diagram services. A column chart and a bar chart are
// the same diagram lying on a different axis, so they share a service and differ in Vertical.
struct DiagramKind
{
    OUString service;
    bool horizontal;
};

DiagramKind diagramKind(OUString const& chartType, std::u16string_view caller)
{
    auto const wanted = chartType.toAsciiLowerCase();
    if (wanted == "line")
    {
        return { u"com.sun.star.chart.LineDiagram"_ustr, false };
    }
    if (wanted == "area")
    {
        return { u"com.sun.star.chart.AreaDiagram"_ustr, false };
    }
    if (wanted == "pie")
    {
        return { u"com.sun.star.chart.PieDiagram"_ustr, false };
    }
    if (wanted == "donut")
    {
        return { u"com.sun.star.chart.DonutDiagram"_ustr, false };
    }
    if (wanted == "scatter")
    {
        return { u"com.sun.star.chart.XYDiagram"_ustr, false };
    }
    if (wanted == "column")
    {
        return { u"com.sun.star.chart.BarDiagram"_ustr, false };
    }
    if (wanted == "bar")
    {
        return { u"com.sun.star.chart.BarDiagram"_ustr, true };
    }
    throw cpo::uno::RuntimeException(
        OUString::Concat(caller) + ": expected line, area, pie, donut, scatter, column or bar, got "
        + chartType);
}

css::chart::ChartLegendPosition legendPosition(OUString const& position,
                                               std::u16string_view caller)
{
    auto const wanted = position.toAsciiLowerCase();
    if (wanted == "right")
    {
        return css::chart::ChartLegendPosition_RIGHT;
    }
    if (wanted == "left")
    {
        return css::chart::ChartLegendPosition_LEFT;
    }
    if (wanted == "top")
    {
        return css::chart::ChartLegendPosition_TOP;
    }
    if (wanted == "bottom")
    {
        return css::chart::ChartLegendPosition_BOTTOM;
    }
    throw cpo::uno::RuntimeException(
        OUString::Concat(caller) + ": expected none, right, left, top or bottom, got " + position);
}

// A chart keeps the collection it lives in and its own name rather than the chart object,
// because the settings a script changes all live on the embedded chart document behind it,
// which has to be fetched fresh each time.
class ChartImpl : public cppu::WeakImplHelper<scriptinterop::XChart>
{
public:
    ChartImpl(cpo::uno::Reference<css::table::XTableCharts> const& charts, OUString const& name)
        : charts_(charts)
        , name_(name)
    {
    }

    cpo::uno::Reference<cpo::uno::XInterface> SAL_CALL getuno() override { return tableChart(); }

    OUString SAL_CALL getName() override { return name_; }

    cpo::uno::Reference<scriptinterop::XChart> SAL_CALL setChartType(OUString const& chartType)
        override
    {
        auto const kind = diagramKind(chartType, u"setChartType"_ustr);
        try
        {
            auto const document = chartDocument();
            cpo::uno::Reference<css::lang::XMultiServiceFactory> const factory(
                document, cpo::uno::UNO_QUERY_THROW);
            cpo::uno::Reference<css::chart::XDiagram> const diagram(
                factory->createInstance(kind.service), cpo::uno::UNO_QUERY_THROW);
            document->setDiagram(diagram);
            cpo::uno::Reference<css::beans::XPropertySet> const props(diagram,
                                                                       cpo::uno::UNO_QUERY_THROW);
            if (props->getPropertySetInfo()->hasPropertyByName(u"Vertical"_ustr))
            {
                props->setPropertyValue(u"Vertical"_ustr,
                                        cpo::uno::Any(kind.horizontal));
            }
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setChartType: ") + e.Message);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XChart> SAL_CALL setTitle(OUString const& title) override
    {
        try
        {
            auto const document = chartDocument();
            cpo::uno::Reference<css::beans::XPropertySet> const documentProps(
                document, cpo::uno::UNO_QUERY_THROW);
            documentProps->setPropertyValue(u"HasMainTitle"_ustr, cpo::uno::Any(true));
            cpo::uno::Reference<css::beans::XPropertySet> const titleProps(
                document->getTitle(), cpo::uno::UNO_QUERY_THROW);
            titleProps->setPropertyValue(u"String"_ustr, cpo::uno::Any(title));
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setTitle: ") + e.Message);
        }
        return this;
    }

    cpo::uno::Reference<scriptinterop::XChart> SAL_CALL setLegendPosition(OUString const& position)
        override
    {
        if (position.toAsciiLowerCase() == "none")
        {
            try
            {
                cpo::uno::Reference<css::beans::XPropertySet> const documentProps(
                    chartDocument(), cpo::uno::UNO_QUERY_THROW);
                documentProps->setPropertyValue(u"HasLegend"_ustr, cpo::uno::Any(false));
            }
            catch (cpo::uno::Exception const& e)
            {
                throw cpo::uno::RuntimeException(OUString::Concat("setLegendPosition: ")
                                                 + e.Message);
            }
            return this;
        }
        auto const alignment = legendPosition(position, u"setLegendPosition"_ustr);
        try
        {
            auto const document = chartDocument();
            cpo::uno::Reference<css::beans::XPropertySet> const documentProps(
                document, cpo::uno::UNO_QUERY_THROW);
            documentProps->setPropertyValue(u"HasLegend"_ustr, cpo::uno::Any(true));
            cpo::uno::Reference<css::beans::XPropertySet> const legendProps(
                document->getLegend(), cpo::uno::UNO_QUERY_THROW);
            legendProps->setPropertyValue(u"Alignment"_ustr, cpo::uno::Any(alignment));
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setLegendPosition: ") + e.Message);
        }
        return this;
    }

    void SAL_CALL remove() override
    {
        try
        {
            charts_->removeByName(name_);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("remove: ") + e.Message);
        }
    }

private:
    cpo::uno::Reference<css::table::XTableChart> tableChart()
    {
        if (!charts_->hasByName(name_))
        {
            throw cpo::uno::RuntimeException(OUString::Concat("the chart ") + name_
                                             + " is no longer on its sheet");
        }
        return cpo::uno::Reference<css::table::XTableChart>(charts_->getByName(name_),
                                                             cpo::uno::UNO_QUERY_THROW);
    }

    cpo::uno::Reference<css::chart::XChartDocument> chartDocument()
    {
        cpo::uno::Reference<css::document::XEmbeddedObjectSupplier> const supplier(
            tableChart(), cpo::uno::UNO_QUERY_THROW);
        return cpo::uno::Reference<css::chart::XChartDocument>(supplier->getEmbeddedObject(),
                                                                cpo::uno::UNO_QUERY_THROW);
    }

    cpo::uno::Reference<css::table::XTableCharts> charts_;
    OUString name_;
};

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
        return new RangeImpl(model_, cellRangeByA1(a1Notation, u"getRange"_ustr));
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getRangeAtCell(sal_Int32 row,
                                                                       sal_Int32 column) override
    {
        return rangeAt(model_, sheet_, row, column, 1, 1);
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL getRangeAtRows(sal_Int32 row, sal_Int32 column, sal_Int32 numRows) override
    {
        return rangeAt(model_, sheet_, row, column, numRows, 1);
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getRangeAt(sal_Int32 row, sal_Int32 column,
                                                                   sal_Int32 numRows,
                                                                   sal_Int32 numColumns) override
    {
        return rangeAt(model_, sheet_, row, column, numRows, numColumns);
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getActiveRange() override
    {
        return new RangeImpl(model_, activeCellRange());
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getActiveCell() override
    {
        // The top-left cell of the first selected range stands in for "the active cell".
        auto const range = activeCellRange();
        return new RangeImpl(model_, range->getCellRangeByPosition(0, 0, 0, 0));
    }

    cpo::uno::Reference<scriptinterop::XRange> SAL_CALL getDataRange() override
    {
        auto const a = usedArea();
        // A cursor cannot take bulk value reads/writes; a genuine sub-range at the same address
        // can.
        cpo::uno::Reference<css::table::XCellRange> const sheetRange(sheet_,
                                                                     cpo::uno::UNO_QUERY_THROW);
        auto const range
            = sheetRange->getCellRangeByPosition(a.StartColumn, a.StartRow, a.EndColumn, a.EndRow);
        return new RangeImpl(model_, range);
    }

    sal_Int32 SAL_CALL getLastRow() override { return usedArea().EndRow + 1; }

    sal_Int32 SAL_CALL getLastColumn() override { return usedArea().EndColumn + 1; }

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

    void SAL_CALL setColumnWidth(sal_Int32 column, sal_Int32 pixels) override
    {
        if (column < 1)
        {
            throw cpo::uno::RuntimeException(
                u"setColumnWidth: expected a column of at least 1"_ustr);
        }
        auto const widthHundredthMm = pixelsToHundredthMm(pixels);
        try
        {
            cpo::uno::Reference<css::beans::XPropertySet> const columnProps(
                columnRowRange()->getColumns()->getByIndex(column - 1), cpo::uno::UNO_QUERY_THROW);
            columnProps->setPropertyValue(u"Width"_ustr, cpo::uno::Any(widthHundredthMm));
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("setColumnWidth: ") + e.Message);
        }
    }

    void SAL_CALL clear() override
    {
        cpo::uno::Reference<css::sheet::XSheetOperation> const op(sheet_,
                                                                  cpo::uno::UNO_QUERY_THROW);
        op->clearContents(css::sheet::CellFlags::VALUE | css::sheet::CellFlags::DATETIME
                          | css::sheet::CellFlags::STRING | css::sheet::CellFlags::ANNOTATION
                          | css::sheet::CellFlags::FORMULA | css::sheet::CellFlags::HARDATTR
                          | css::sheet::CellFlags::STYLES | css::sheet::CellFlags::OBJECTS
                          | css::sheet::CellFlags::EDITATTR | css::sheet::CellFlags::FORMATTED);
    }

    void SAL_CALL autoResizeColumn(sal_Int32 column) override
    {
        resizeColumnsToFit(column, 1, u"autoResizeColumn"_ustr);
    }

    void SAL_CALL autoResizeColumns(sal_Int32 startColumn, sal_Int32 numColumns) override
    {
        resizeColumnsToFit(startColumn, numColumns, u"autoResizeColumns"_ustr);
    }

private:
    void resizeColumnsToFit(sal_Int32 startColumn, sal_Int32 numColumns,
                            std::u16string_view name)
    {
        if (startColumn < 1 || numColumns < 1)
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat(name) + ": expected a column and count of at least 1");
        }
        try
        {
            auto const columns = columnRowRange()->getColumns();
            for (sal_Int32 i = 0; i != numColumns; ++i)
            {
                cpo::uno::Reference<css::beans::XPropertySet> const columnProps(
                    columns->getByIndex(startColumn - 1 + i), cpo::uno::UNO_QUERY_THROW);
                columnProps->setPropertyValue(u"OptimalWidth"_ustr, cpo::uno::Any(true));
            }
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat(name) + ": " + e.Message);
        }
    }

public:
    void SAL_CALL autoResizeRows(sal_Int32 startRow, sal_Int32 numRows) override
    {
        if (startRow < 1 || numRows < 1)
        {
            throw cpo::uno::RuntimeException(
                u"autoResizeRows: expected a row and count of at least 1"_ustr);
        }
        try
        {
            auto const rows = columnRowRange()->getRows();
            for (sal_Int32 i = 0; i != numRows; ++i)
            {
                cpo::uno::Reference<css::beans::XPropertySet> const rowProps(
                    rows->getByIndex(startRow - 1 + i), cpo::uno::UNO_QUERY_THROW);
                rowProps->setPropertyValue(u"OptimalHeight"_ustr, cpo::uno::Any(true));
            }
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("autoResizeRows: ") + e.Message);
        }
    }

    cpo::uno::Reference<scriptinterop::XChart> SAL_CALL insertChart(
        cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XRange>> const& dataRanges,
        sal_Int32 anchorRow, sal_Int32 anchorColumn, double width, double height,
        bool firstRowAsHeaders, bool firstColumnAsHeaders) override
    {
        if (anchorRow < 1 || anchorColumn < 1)
        {
            throw cpo::uno::RuntimeException(
                u"insertChart: expected an anchor row and column of at least 1"_ustr);
        }
        if (!dataRanges.hasElements())
        {
            throw cpo::uno::RuntimeException(u"insertChart: expected at least one data range"_ustr);
        }
        auto const widthHundredthMm = extentToHundredthMm(width);
        auto const heightHundredthMm = extentToHundredthMm(height);
        std::vector<css::table::CellRangeAddress> addresses;
        for (auto const& dataRange : dataRanges)
        {
            if (!dataRange.is())
            {
                throw cpo::uno::RuntimeException(u"insertChart: expected a data range"_ustr);
            }
            // A range reports the cells behind it, which carry the sheet they sit on, so a
            // chart can take its data from a sheet other than the one holding the chart.
            cpo::uno::Reference<css::sheet::XCellRangeAddressable> const addressable(
                dataRange->getuno(), cpo::uno::UNO_QUERY_THROW);
            addresses.emplace_back(addressable->getRangeAddress());
        }
        cpo::uno::Reference<css::table::XTableChartsSupplier> const supplier(
            sheet_, cpo::uno::UNO_QUERY_THROW);
        auto const charts = supplier->getCharts();
        auto const name = freeChartName();
        // The chart sits where the anchor cell starts, which is the total width of the columns
        // and the total height of the rows in front of it.
        css::awt::Rectangle const rect(columnsExtent(anchorColumn - 1), rowsExtent(anchorRow - 1),
                                       widthHundredthMm, heightHundredthMm);
        try
        {
            charts->addNewByName(name, rect,
                                 cpo::uno::Sequence<css::table::CellRangeAddress>(addresses.data(),
                                                                                  addresses.size()),
                                 firstRowAsHeaders, firstColumnAsHeaders);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("insertChart: ") + e.Message);
        }
        if (!charts->hasByName(name))
        {
            // Calc drops the request without a word when the chart module is not part of the
            // installation.
            throw cpo::uno::RuntimeException(u"insertChart: this build cannot create charts"_ustr);
        }
        return new ChartImpl(charts, name);
    }

    // The charts come back in the order the collection keeps them, which is neither the order
    // a script added them nor the order they lie on the sheet.
    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XChart>> SAL_CALL getCharts() override
    {
        try
        {
            cpo::uno::Reference<css::table::XTableChartsSupplier> const supplier(
                sheet_, cpo::uno::UNO_QUERY_THROW);
            auto const charts = supplier->getCharts();
            auto const names = charts->getElementNames();
            std::vector<cpo::uno::Reference<scriptinterop::XChart>> result;
            result.reserve(names.getLength());
            for (auto const& name : names)
            {
                result.emplace_back(new ChartImpl(charts, name));
            }
            return cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XChart>>(result.data(),
                                                                                   result.size());
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getCharts: ") + e.Message);
        }
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPivotTable>> SAL_CALL getPivotTables()
        override
    {
        try
        {
            cpo::uno::Reference<css::sheet::XDataPilotTablesSupplier> const supplier(
                sheet_, cpo::uno::UNO_QUERY_THROW);
            auto const tables = supplier->getDataPilotTables();
            auto const names = tables->getElementNames();
            std::vector<cpo::uno::Reference<scriptinterop::XPivotTable>> result;
            result.reserve(names.getLength());
            for (auto const& name : names)
            {
                result.emplace_back(new PivotTableImpl(tables, name));
            }
            return cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XPivotTable>>(
                result.data(), result.size());
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getPivotTables: ") + e.Message);
        }
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

    // Resolves an A1 name against this sheet, or against the sheet a qualified name points at.
    cpo::uno::Reference<css::table::XCellRange> cellRangeByA1(OUString const& a1Notation,
                                                                std::u16string_view caller)
    {
        OUString qualifiedSheet;
        OUString cells;
        cpo::uno::Reference<css::table::XCellRange> sheetRange;
        if (splitSheetQualifiedName(a1Notation, qualifiedSheet, cells))
        {
            cpo::uno::Reference<css::container::XNameAccess> const sheets(
                documentSheets(model_), cpo::uno::UNO_QUERY_THROW);
            if (!sheets->hasByName(qualifiedSheet))
            {
                throw cpo::uno::RuntimeException(OUString::Concat(caller) + ": no sheet named "
                                                 + qualifiedSheet);
            }
            sheetRange.set(sheets->getByName(qualifiedSheet), cpo::uno::UNO_QUERY_THROW);
        }
        else
        {
            sheetRange.set(sheet_, cpo::uno::UNO_QUERY_THROW);
        }
        cpo::uno::Reference<css::table::XCellRange> range;
        try
        {
            range = sheetRange->getCellRangeByName(cells);
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat(caller) + ": " + e.Message);
        }
        if (!range.is())
        {
            throw cpo::uno::RuntimeException(
                OUString::Concat(caller) + ": expected a valid A1-style range, got " + a1Notation);
        }
        return range;
    }

    OUString freeChartName()
    {
        cpo::uno::Reference<css::container::XIndexAccess> const sheets(
            documentSheets(model_), cpo::uno::UNO_QUERY_THROW);
        auto const sheetCount = sheets->getCount();
        std::vector<cpo::uno::Reference<css::container::XNameAccess>> taken;
        taken.reserve(sheetCount);
        for (sal_Int32 i = 0; i != sheetCount; ++i)
        {
            cpo::uno::Reference<css::table::XTableChartsSupplier> const supplier(
                sheets->getByIndex(i), cpo::uno::UNO_QUERY_THROW);
            taken.emplace_back(supplier->getCharts(), cpo::uno::UNO_QUERY_THROW);
        }
        return freeName(u"Chart "_ustr, taken);
    }

    // A hidden column or row keeps the width it would have if it were shown, but takes up no
    // space on the sheet, so it must not push the anchor along.
    static sal_Int32 visibleExtent(cpo::uno::Reference<css::container::XIndexAccess> const& lines,
                                   OUString const& sizeProperty, sal_Int32 count)
    {
        sal_Int32 extent = 0;
        for (sal_Int32 i = 0; i != count; ++i)
        {
            cpo::uno::Reference<css::beans::XPropertySet> const props(lines->getByIndex(i),
                                                                       cpo::uno::UNO_QUERY_THROW);
            bool visible = true;
            props->getPropertyValue(u"IsVisible"_ustr) >>= visible;
            if (!visible)
            {
                continue;
            }
            sal_Int32 size = 0;
            props->getPropertyValue(sizeProperty) >>= size;
            extent += size;
        }
        return extent;
    }

    sal_Int32 columnsExtent(sal_Int32 count)
    {
        return visibleExtent(columnRowRange()->getColumns(), u"Width"_ustr, count);
    }

    sal_Int32 rowsExtent(sal_Int32 count)
    {
        return visibleExtent(columnRowRange()->getRows(), u"Height"_ustr, count);
    }

    // The bounding box of every non-empty cell on the sheet.
    css::table::CellRangeAddress usedArea()
    {
        auto const cursor = sheet_->createCursor();
        cpo::uno::Reference<css::sheet::XUsedAreaCursor> const used(cursor,
                                                                    cpo::uno::UNO_QUERY_THROW);
        used->gotoStartOfUsedArea(false);
        used->gotoEndOfUsedArea(true);
        cpo::uno::Reference<css::sheet::XCellRangeAddressable> const addr(
            cursor, cpo::uno::UNO_QUERY_THROW);
        return addr->getRangeAddress();
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

    cpo::uno::Reference<scriptinterop::XSheet>
        SAL_CALL getSheetByName(OUString const& name) override
    {
        cpo::uno::Reference<css::container::XNameAccess> const sheets(documentSheets(model_),
                                                                       cpo::uno::UNO_QUERY_THROW);
        // A name that no sheet carries reports as an empty reference, which a script sees as
        // null, the way SpreadsheetApp reports it.
        if (!sheets->hasByName(name))
        {
            return {};
        }
        cpo::uno::Reference<css::sheet::XSpreadsheet> sheet;
        sheets->getByName(name) >>= sheet;
        return new SheetImpl(model_, sheet);
    }

    cpo::uno::Reference<scriptinterop::XSheet> SAL_CALL insertSheet(OUString const& name) override
    {
        return insertSheetNamed(name);
    }

private:
    cpo::uno::Reference<scriptinterop::XSheet> insertSheetNamed(OUString const& name)
    {
        auto const sheets = documentSheets(model_);
        cpo::uno::Reference<css::container::XIndexAccess> const indexed(sheets,
                                                                         cpo::uno::UNO_QUERY_THROW);
        auto const count = indexed->getCount();
        if (count > SAL_MAX_INT16)
        {
            throw cpo::uno::RuntimeException(u"insertSheet: the document already has too many "
                                             u"sheets to add another"_ustr);
        }
        sheets->insertNewByName(name, static_cast<sal_Int16>(count));
        cpo::uno::Reference<css::container::XNameAccess> const byName(sheets,
                                                                       cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::sheet::XSpreadsheet> sheet;
        byName->getByName(name) >>= sheet;
        return new SheetImpl(model_, sheet);
    }

    // The named ranges a sheet keeps for itself, as against the document's own list.
    static cpo::uno::Reference<css::sheet::XNamedRanges>
    sheetNamedRanges(cpo::uno::Reference<css::sheet::XSpreadsheet> const& sheet)
    {
        cpo::uno::Reference<css::beans::XPropertySet> const props(sheet, cpo::uno::UNO_QUERY);
        if (!props.is())
        {
            return {};
        }
        cpo::uno::Reference<css::sheet::XNamedRanges> ranges;
        props->getPropertyValue(u"NamedRanges"_ustr) >>= ranges;
        return ranges;
    }

    static cpo::uno::Reference<css::sheet::XNamedRange>
    namedRangeFrom(cpo::uno::Reference<css::sheet::XNamedRanges> const& ranges,
                   OUString const& name)
    {
        if (!ranges.is() || !ranges->hasByName(name))
        {
            return {};
        }
        cpo::uno::Reference<css::sheet::XNamedRange> range;
        ranges->getByName(name) >>= range;
        return range;
    }

    cpo::uno::Reference<css::sheet::XNamedRange> findNamedRange(OUString const& name)
    {
        cpo::uno::Reference<css::container::XNameAccess> const byName(documentSheets(model_),
                                                                       cpo::uno::UNO_QUERY_THROW);
        auto const bang = name.lastIndexOf('!');
        if (bang != -1)
        {
            auto const sheetName = name.copy(0, bang);
            auto const bareName = name.copy(bang + 1);
            if (!byName->hasByName(sheetName))
            {
                return {};
            }
            cpo::uno::Reference<css::sheet::XSpreadsheet> sheet;
            byName->getByName(sheetName) >>= sheet;
            return namedRangeFrom(sheetNamedRanges(sheet), bareName);
        }
        cpo::uno::Reference<css::beans::XPropertySet> const docProps(model_,
                                                                      cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::sheet::XNamedRanges> documentRanges;
        docProps->getPropertyValue(u"NamedRanges"_ustr) >>= documentRanges;
        if (auto const range = namedRangeFrom(documentRanges, name); range.is())
        {
            return range;
        }
        cpo::uno::Reference<css::container::XIndexAccess> const indexed(byName,
                                                                         cpo::uno::UNO_QUERY_THROW);
        for (sal_Int32 i = 0; i != indexed->getCount(); ++i)
        {
            cpo::uno::Reference<css::sheet::XSpreadsheet> sheet;
            indexed->getByIndex(i) >>= sheet;
            if (auto const range = namedRangeFrom(sheetNamedRanges(sheet), name); range.is())
            {
                return range;
            }
        }
        return {};
    }

public:

    // The name Calc's own new-sheet command would give it: "Sheet" and the lowest number that
    // leaves the name free across the document.
    cpo::uno::Reference<scriptinterop::XSheet> SAL_CALL insertSheetWithDefaultName() override
    {
        cpo::uno::Reference<css::container::XNameAccess> const byName(documentSheets(model_),
                                                                       cpo::uno::UNO_QUERY_THROW);
        OUString name;
        for (sal_Int32 n = 1;; ++n)
        {
            name = OUString::Concat("Sheet") + OUString::number(n);
            if (!byName->hasByName(name))
            {
                break;
            }
        }
        return insertSheetNamed(name);
    }

    cpo::uno::Reference<scriptinterop::XRange>
        SAL_CALL getRangeByName(OUString const& name) override
    {
        // A name belongs either to the document or to one sheet, and a sheet keeps its own in a
        // list of its own. A name may say which sheet holds it, written the way SpreadsheetApp
        // writes it, with the sheet name unquoted in front of an exclamation mark. A plain name
        // is looked for in the document's list first and then in each sheet's list, left to
        // right. A name that nothing holds reports as an empty reference, which a script sees as
        // null.
        auto const namedRange = findNamedRange(name);
        if (!namedRange.is())
        {
            return {};
        }
        cpo::uno::Reference<css::sheet::XCellRangeReferrer> const referrer(
            namedRange, cpo::uno::UNO_QUERY_THROW);
        auto const cells = referrer->getReferredCells();
        if (!cells.is())
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getRangeByName: ") + name
                                             + " does not refer to a plain cell range");
        }
        return new RangeImpl(model_, cells);
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

    OUString SAL_CALL getName() override
    {
        try
        {
            // A saved document is named after its file, without the folders in front of it and
            // without the extension, which is the name SpreadsheetApp-style code expects. One
            // that has never been saved has no file, so its own placeholder title stands in.
            auto const url = model_->getURL();
            if (url.isEmpty())
            {
                cpo::uno::Reference<css::frame::XTitle> const title(model_,
                                                                     cpo::uno::UNO_QUERY_THROW);
                return title->getTitle();
            }
            auto const file = rtl::Uri::decode(url.copy(url.lastIndexOf('/') + 1),
                                               rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
            auto const extension = file.lastIndexOf('.');
            // A leading dot belongs to the name rather than starting an extension.
            return extension > 0 ? file.copy(0, extension) : file;
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getName: ") + e.Message);
        }
    }

    cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XSheet>> SAL_CALL getSheets() override
    {
        try
        {
            cpo::uno::Reference<css::container::XIndexAccess> const indexed(
                documentSheets(model_), cpo::uno::UNO_QUERY_THROW);
            auto const n = indexed->getCount();
            std::vector<cpo::uno::Reference<scriptinterop::XSheet>> sheets;
            sheets.reserve(n);
            for (sal_Int32 i = 0; i != n; ++i)
            {
                cpo::uno::Reference<css::sheet::XSpreadsheet> const sheet(
                    indexed->getByIndex(i), cpo::uno::UNO_QUERY_THROW);
                sheets.emplace_back(new SheetImpl(model_, sheet));
            }
            return cpo::uno::Sequence<cpo::uno::Reference<scriptinterop::XSheet>>(sheets.data(),
                                                                                   sheets.size());
        }
        catch (cpo::uno::Exception const& e)
        {
            throw cpo::uno::RuntimeException(OUString::Concat("getSheets: ") + e.Message);
        }
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
