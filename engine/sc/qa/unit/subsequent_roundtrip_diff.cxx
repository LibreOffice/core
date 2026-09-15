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

/* A self-labelling fidelity harness for the xlsx filter.
 *
 * For every document of a corpus we take three dumps of the model:
 *
 *   D  --import-->  A   --export/import-->  B   --export/import-->  C
 *
 * A difference between A and B is something the export and the import lost between them. A
 * difference between B and C is worse, because the file read for it is one we wrote ourselves.
 * Neither comparison needs a reference file or a person, which is what lets it run over the whole
 * corpus and rank what the filter loses by the number of documents it happens in.
 *
 * A cell keeps its address whatever else changes, so the two sides of a comparison line up even
 * where rows or columns were added, which is the thing that makes a drawing hard to compare.
 *
 * Driven by the environment:
 *
 *   SC_RT_RUN      run at all; without it, and without SC_RT_CORPUS, this is a no-op
 *   SC_RT_CORPUS   directory of documents to read (default sc/qa/unit/data/xlsx)
 *   SC_RT_REPORT   report file, appended to and resumable (default sc-roundtrip.log)
 *   SC_RT_LIMIT    stop after that many documents
 *   SC_RT_CELLS    how many cells of a sheet to read the formatting of (default 2000)
 *   SC_RT_DUMPDIR  if set, write the A, B and C dumps of every document there
 */

#include <sal/config.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <string_view>
#include <vector>

#include <o3tl/string_view.hxx>

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <comphelper/anytostring.hxx>
#include <osl/file.hxx>
#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

using namespace css;
using namespace ::cpo;

namespace
{
/** Longest value the report prints. The comparison reads the whole value, because a border or a
    geometry states more than this and a difference beyond the cut would otherwise be invisible. */
constexpr sal_Int32 MAX_VALUE_LEN = 400;

/** How far apart two numbers lie and still count as the same value. */
enum class Slack
{
    /** A count, a constant or a colour, where one more is a different value. */
    None,
    /** A measure, which a conversion through another unit leaves one off, and further off the
        longer it is. */
    Unit,
    /** A value carried as a fraction, which a conversion moves by a fraction of itself. */
    Fraction
};

bool numbersEqual(double fLeft, double fRight, Slack eSlack)
{
    if (eSlack == Slack::None)
        return fLeft == fRight;
    const double fMagnitude = std::max(std::abs(fLeft), std::abs(fRight));
    const double fAllowed
        = eSlack == Slack::Unit ? std::max(1.0, 0.002 * fMagnitude) : 0.002 * fMagnitude;
    return std::abs(fLeft - fRight) <= fAllowed;
}

/** Values that measure something. A length written in one unit and read back in another comes
    back a little moved, and the move grows with the length, because a position carries the
    rounding of every column and row in front of it. A point, a size, a rectangle and a matrix
    measure by what they are; the rest are named. */
bool measuresSomething(std::u16string_view rValue, std::string_view rName)
{
    if (rValue.starts_with(u"(com.sun.star.awt.Point)")
        || rValue.starts_with(u"(com.sun.star.awt.Size)")
        || rValue.starts_with(u"(com.sun.star.awt.Rectangle)")
        || rValue.starts_with(u"(com.sun.star.drawing.HomogenMatrix3)"))
        return true;

    static const std::set<std::string_view> aMeasured{ "Width",
                                                       "Height",
                                                       "HoriOrientPosition",
                                                       "VertOrientPosition",
                                                       "TextLeftDistance",
                                                       "TextRightDistance",
                                                       "TextUpperDistance",
                                                       "TextLowerDistance",
                                                       "TextMinimumFrameWidth",
                                                       "TextMinimumFrameHeight",
                                                       "TextMaximumFrameWidth",
                                                       "TextMaximumFrameHeight" };
    return aMeasured.count(rName) != 0;
}

/** A value states the type it is of before it states itself, and a value built out of others
    states the type of each of them, so the type nearest in front of a number is the one it is of.
    The type says how exactly to read a number that measures nothing. */
Slack slackOf(std::u16string_view rText, size_t nTypeStart, bool bMeasured)
{
    if (bMeasured)
        return Slack::Unit;
    const size_t nEnd = rText.find(u')', nTypeStart);
    if (nEnd == std::u16string_view::npos)
        return Slack::None;
    const std::u16string_view aType = rText.substr(nTypeStart + 1, nEnd - nTypeStart - 1);
    // A value carried as a fraction is never written down exactly.
    if (aType == u"float" || aType == u"double")
        return Slack::Fraction;
    return Slack::None;
}

/** Properties whose values must match exactly: what a cell holds and what it is shown as, and the
    names and counts the comparison lines the two sides up by. A number inside a formula or a
    number format is not a length, and one more there is a different cell or one more decimal. */
bool comparedExactly(std::string_view rKey)
{
    static const std::set<std::string_view> aExact{
        "Formula",    "Type",       "Value",       "Name",     "NumberFormatCode", "ShapeType",
        "SheetCount", "ShapeCount", "UsedColumns", "UsedRows", "CellsRead"
    };
    const size_t nAt = rKey.rfind('@');
    if (nAt == std::string_view::npos)
        return false;
    return aExact.count(rKey.substr(nAt + 1)) != 0;
}

/** Properties that differ between two saves of the same model for reasons that say nothing about
    fidelity: names the application makes up, and what it keeps for itself. */
bool isNoise(const OUString& rName)
{
    static const std::set<OUString> aNoise{
        // The handle a cell format is held under, which is counted afresh for every document.
        u"FormatID"_ustr,
        // The picture a shape is drawn into, which differs in a byte or two every time it is
        // produced, and the handle a live object is held under.
        u"MetaFile"_ustr, u"RuntimeUID"_ustr,
        // Names the application makes up, and what it keeps for itself.
        u"AbsoluteName"_ustr, u"PrivateTempFileURL"_ustr, u"PreviewMetafile"_ustr,
        u"PreviewBitmap"_ustr, u"Guid"_ustr, u"InteropGrabBag"_ustr,
        u"UserDefinedAttributes"_ustr
    };
    return aNoise.count(rName) != 0;
}

/** The address a person reads, so that a line of the report says where to look. */
OString cellAddress(sal_Int32 nColumn, sal_Int32 nRow)
{
    OStringBuffer aBuffer;
    for (sal_Int32 nRest = nColumn; ; nRest = nRest / 26 - 1)
    {
        aBuffer.insert(0, char('A' + nRest % 26));
        if (nRest < 26)
            break;
    }
    aBuffer.append(nRow + 1);
    return aBuffer.makeStringAndClear();
}

/** Reads back what cellAddress wrote, as the column and the row counted from zero. */
bool parseCellAddress(std::string_view rAddress, sal_Int32& rColumn, sal_Int32& rRow)
{
    size_t nPos = 0;
    sal_Int32 nColumn = 0;
    while (nPos < rAddress.size() && rAddress[nPos] >= 'A' && rAddress[nPos] <= 'Z')
        nColumn = nColumn * 26 + (rAddress[nPos++] - 'A' + 1);
    if (nPos == 0 || nPos == rAddress.size())
        return false;
    rColumn = nColumn - 1;
    rRow = o3tl::toInt32(rAddress.substr(nPos)) - 1;
    return rRow >= 0;
}

/** Walks a spreadsheet and writes every reachable property into a flat map keyed by the sheet, the
    cell address and the property name. Deliberately generic: it asks XPropertySetInfo what is
    there instead of naming properties, so it never falls behind the model. */
class ModelDumper
{
public:
    std::map<OString, OUString> maValues;

    explicit ModelDumper(sal_Int32 nCellLimit)
        : mnCellLimit(nCellLimit)
    {
    }

    void dump(const uno::Reference<lang::XComponent>& xComponent)
    {
        uno::Reference<sheet::XSpreadsheetDocument> xDocument(xComponent, uno::UNO_QUERY);
        if (!xDocument.is())
            return;
        if (uno::Reference<util::XNumberFormatsSupplier> xSupplier{ xComponent, uno::UNO_QUERY })
            mxNumberFormats = xSupplier->getNumberFormats();
        dumpProperties(uno::Reference<beans::XPropertySet>(xComponent, uno::UNO_QUERY),
                       "document"_ostr);

        uno::Reference<container::XIndexAccess> xSheets(xDocument->getSheets(), uno::UNO_QUERY);
        if (!xSheets.is())
            return;
        add("document"_ostr, "SheetCount", OUString::number(xSheets->getCount()));
        for (sal_Int32 i = 0; i < xSheets->getCount(); ++i)
        {
            uno::Reference<sheet::XSpreadsheet> xSheet(xSheets->getByIndex(i), uno::UNO_QUERY);
            if (xSheet.is())
                dumpSheet(xSheet, indexed("sheet", i));
        }
    }

private:
    sal_Int32 mnCellLimit;
    uno::Reference<util::XNumberFormats> mxNumberFormats;

    static OString indexed(std::string_view rTag, sal_Int32 nIndex)
    {
        return OString::Concat(rTag) + "[" + OString::number(nIndex) + "]";
    }

    void add(const OString& rPath, std::string_view rName, const OUString& rValue)
    {
        maValues[OString::Concat(rPath) + "@" + rName] = rValue;
    }

    void dumpSheet(const uno::Reference<sheet::XSpreadsheet>& xSheet, const OString& rPath)
    {
        uno::Reference<container::XNamed> xNamed(xSheet, uno::UNO_QUERY);
        if (xNamed.is())
            add(rPath, "Name", xNamed->getName());
        dumpProperties(uno::Reference<beans::XPropertySet>(xSheet, uno::UNO_QUERY), rPath);

        table::CellRangeAddress aUsedArea;
        try
        {
            uno::Reference<sheet::XSheetCellCursor> xCursor(xSheet->createCursor());
            uno::Reference<sheet::XUsedAreaCursor> xUsedArea(xCursor, uno::UNO_QUERY_THROW);
            xUsedArea->gotoEndOfUsedArea(false);
            aUsedArea
                = uno::Reference<sheet::XCellRangeAddressable>(xCursor, uno::UNO_QUERY_THROW)
                      ->getRangeAddress();
        }
        catch (...)
        {
            return;
        }
        add(rPath, "UsedColumns", OUString::number(aUsedArea.EndColumn + 1));
        add(rPath, "UsedRows", OUString::number(aUsedArea.EndRow + 1));

        dumpColumnsAndRows(xSheet, rPath, aUsedArea);
        dumpCells(xSheet, rPath, aUsedArea);
        dumpShapes(xSheet, rPath);
    }

    void dumpColumnsAndRows(const uno::Reference<sheet::XSpreadsheet>& xSheet,
                            std::string_view rPath,
                            const table::CellRangeAddress& rUsedArea)
    {
        uno::Reference<table::XColumnRowRange> xRange(xSheet, uno::UNO_QUERY);
        if (!xRange.is())
            return;
        // A sheet says it uses everything up to the last cell that holds anything, so one value
        // in the far corner makes that a million rows. Columns and rows are read up to the same
        // limit the cells are.
        uno::Reference<container::XIndexAccess> xColumns = xRange->getColumns();
        for (sal_Int32 i = 0; xColumns.is() && i <= rUsedArea.EndColumn && i < mnCellLimit
                              && i < xColumns->getCount();
             ++i)
        {
            uno::Reference<beans::XPropertySet> xColumn(xColumns->getByIndex(i), uno::UNO_QUERY);
            dumpProperties(xColumn, OString::Concat(rPath) + "/" + indexed("column", i));
        }
        uno::Reference<container::XIndexAccess> xRows = xRange->getRows();
        for (sal_Int32 i = 0;
             xRows.is() && i <= rUsedArea.EndRow && i < mnCellLimit && i < xRows->getCount(); ++i)
        {
            uno::Reference<beans::XPropertySet> xRow(xRows->getByIndex(i), uno::UNO_QUERY);
            dumpProperties(xRow, OString::Concat(rPath) + "/" + indexed("row", i));
        }
    }

    static OString cellPath(std::string_view rPath, sal_Int32 nColumn, sal_Int32 nRow)
    {
        return OString::Concat(rPath) + "/cell[" + cellAddress(nColumn, nRow) + "]";
    }

    void dumpCells(const uno::Reference<sheet::XSpreadsheet>& xSheet, const OString& rPath,
                   const table::CellRangeAddress& rUsedArea)
    {
        // Reading the formatting of every cell of a large sheet costs more than it says, so a
        // sheet is read up to a limit and the report states where it stopped.
        sal_Int32 nRead = 0;
        for (sal_Int32 nRow = 0; nRow <= rUsedArea.EndRow && nRead < mnCellLimit; ++nRow)
        {
            for (sal_Int32 nColumn = 0; nColumn <= rUsedArea.EndColumn && nRead < mnCellLimit;
                 ++nColumn)
            {
                try
                {
                    dumpProperties(uno::Reference<beans::XPropertySet>(
                                       xSheet->getCellByPosition(nColumn, nRow), uno::UNO_QUERY),
                                   cellPath(rPath, nColumn, nRow));
                }
                catch (...)
                {
                    continue;
                }
                ++nRead;
            }
        }
        if (nRead >= mnCellLimit)
            add(rPath, "CellsRead", OUString::number(nRead));

        dumpCellContents(xSheet, rPath);
    }

    /** Writes what each cell of the sheet holds.

        The cells that hold something are asked for rather than looked for. A sheet says it uses
        everything up to the last cell that holds anything, so one value in the far corner makes
        that area the whole sheet, and reading it cell by cell would take the rest of the day. */
    void dumpCellContents(const uno::Reference<sheet::XSpreadsheet>& xSheet,
                          std::string_view rPath)
    {
        uno::Reference<sheet::XCellRangesQuery> xQuery(xSheet, uno::UNO_QUERY);
        if (!xQuery.is())
            return;

        uno::Reference<sheet::XSheetCellRanges> xRanges;
        try
        {
            xRanges = xQuery->queryContentCells(sheet::CellFlags::VALUE | sheet::CellFlags::DATETIME
                                                | sheet::CellFlags::STRING
                                                | sheet::CellFlags::FORMULA);
        }
        catch (...)
        {
            return;
        }
        if (!xRanges.is())
            return;

        uno::Reference<container::XEnumerationAccess> xCells = xRanges->getCells();
        if (!xCells.is())
            return;
        uno::Reference<container::XEnumeration> xEnumeration = xCells->createEnumeration();
        while (xEnumeration.is() && xEnumeration->hasMoreElements())
        {
            uno::Reference<table::XCell> xCell(xEnumeration->nextElement(), uno::UNO_QUERY);
            uno::Reference<sheet::XCellAddressable> xAddressable(xCell, uno::UNO_QUERY);
            if (!xCell.is() || !xAddressable.is())
                continue;

            const table::CellAddress aAddress = xAddressable->getCellAddress();
            const OString aPath = cellPath(rPath, aAddress.Column, aAddress.Row);
            const table::CellContentType eType = xCell->getType();
            add(aPath, "Formula", xCell->getFormula());
            add(aPath, "Type", OUString::number(sal_Int32(eType)));
            if (eType == table::CellContentType_VALUE)
                add(aPath, "Value", OUString::number(xCell->getValue()));
        }
    }

    void dumpShapes(const uno::Reference<sheet::XSpreadsheet>& xSheet, const OString& rPath)
    {
        uno::Reference<drawing::XDrawPageSupplier> xSupplier(xSheet, uno::UNO_QUERY);
        if (!xSupplier.is())
            return;
        uno::Reference<drawing::XShapes> xShapes = xSupplier->getDrawPage();
        if (!xShapes.is())
            return;
        add(rPath, "ShapeCount", OUString::number(xShapes->getCount()));
        for (sal_Int32 i = 0; i < xShapes->getCount(); ++i)
        {
            uno::Reference<drawing::XShape> xShape(xShapes->getByIndex(i), uno::UNO_QUERY);
            if (!xShape.is())
                continue;
            const OString aPath = rPath + OString::Concat("/") + indexed("shape", i);
            add(aPath, "ShapeType", xShape->getShapeType());
            dumpProperties(uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY), aPath);
        }
    }

    /** Writes the format a number is shown in, rather than the key it is held under. The key is
        counted afresh for every document, so two documents showing the same thing disagree on it
        and two saves of one document need not agree either. */
    void addNumberFormat(const uno::Reference<beans::XPropertySet>& xPropertySet,
                         const OString& rPath)
    {
        if (!mxNumberFormats.is())
            return;
        try
        {
            sal_Int32 nKey = 0;
            xPropertySet->getPropertyValue(u"NumberFormat"_ustr) >>= nKey;
            uno::Reference<beans::XPropertySet> xFormat(mxNumberFormats->getByKey(nKey));
            if (!xFormat.is())
                return;
            OUString aCode;
            xFormat->getPropertyValue(u"FormatString"_ustr) >>= aCode;
            add(rPath, "NumberFormatCode", aCode);
        }
        catch (...)
        {
        }
    }

    void dumpProperties(const uno::Reference<beans::XPropertySet>& xPropertySet,
                        const OString& rPath)
    {
        if (!xPropertySet.is())
            return;
        uno::Reference<beans::XPropertySetInfo> xInfo;
        try
        {
            xInfo = xPropertySet->getPropertySetInfo();
        }
        catch (...)
        {
            return;
        }
        if (!xInfo.is())
            return;

        // Everything but a cell states the borders of every cell it holds as one value, which
        // says nothing once those cells disagree.
        const bool bHoldsCells = rPath.indexOf("/cell[") < 0;

        for (const beans::Property& rProperty : xInfo->getProperties())
        {
            if (rProperty.Type.getTypeClass() == cpo::uno::TypeClass_INTERFACE
                || isNoise(rProperty.Name))
                continue;
            if (bHoldsCells && rProperty.Name.startsWith("TableBorder"))
                continue;
            // The key a number format is held under is counted afresh for every document, so the
            // format itself is written instead, below.
            if (rProperty.Name == "NumberFormat")
            {
                addNumberFormat(xPropertySet, rPath);
                continue;
            }

            cpo::uno::Any aValue;
            try
            {
                aValue = xPropertySet->getPropertyValue(rProperty.Name);
            }
            catch (...)
            {
                continue;
            }
            if (!aValue.hasValue() || aValue.getValueTypeClass() == cpo::uno::TypeClass_INTERFACE)
                continue;

            maValues[OString::Concat(rPath) + "@"
                     + OUStringToOString(rProperty.Name, RTL_TEXTENCODING_UTF8)]
                = comphelper::anyToString(aValue);
        }
    }
};

enum class DiffKind
{
    Lost,
    Added,
    Changed
};

struct Diff
{
    DiffKind eKind;
    OString aKey;
    OUString aBefore;
    OUString aAfter;
};

/** Compares two values number aware, so that a width converted through another unit and back does
    not read as a difference. */
bool valuesEqual(const OUString& rLeft, const OUString& rRight, bool bMeasured)
{
    if (rLeft == rRight)
        return true;

    sal_Int32 nLeft = 0, nRight = 0;
    const sal_Int32 nLeftLen = rLeft.getLength(), nRightLen = rRight.getLength();

    auto isNumberStart = [](const OUString& rText, sal_Int32 nPos) {
        sal_Unicode c = rText[nPos];
        if (rtl::isAsciiDigit(c))
            return true;
        return (c == '-' || c == '+') && nPos + 1 < rText.getLength()
               && rtl::isAsciiDigit(rText[nPos + 1]);
    };
    auto readNumber = [](const OUString& rText, sal_Int32& rPos) {
        sal_Int32 nStart = rPos;
        if (rText[rPos] == '-' || rText[rPos] == '+')
            ++rPos;
        while (rPos < rText.getLength() && (rtl::isAsciiDigit(rText[rPos]) || rText[rPos] == '.'))
            ++rPos;
        return o3tl::toDouble(rText.subView(nStart, rPos - nStart));
    };

    // The text of a string is compared as it is: a style called Heading 1 is not Heading 2.
    bool bInString = false;
    Slack eSlack = Slack::None;
    while (nLeft < nLeftLen && nRight < nRightLen)
    {
        if (!bInString && isNumberStart(rLeft, nLeft) && isNumberStart(rRight, nRight))
        {
            if (!numbersEqual(readNumber(rLeft, nLeft), readNumber(rRight, nRight), eSlack))
                return false;
        }
        else if (rLeft[nLeft] == rRight[nRight])
        {
            if (rLeft[nLeft] == '"')
                bInString = !bInString;
            else if (!bInString && rLeft[nLeft] == '(')
                eSlack = slackOf(rLeft, nLeft, bMeasured);
            else if (bInString && rLeft[nLeft] == '\\' && nLeft + 1 < nLeftLen
                     && nRight + 1 < nRightLen)
            {
                // An escaped character, which neither ends the string nor starts a number.
                if (rLeft[nLeft + 1] != rRight[nRight + 1])
                    return false;
                ++nLeft;
                ++nRight;
            }
            ++nLeft;
            ++nRight;
        }
        else
            return false;
    }
    return nLeft == nLeftLen && nRight == nRightLen;
}

bool valuesEqual(std::string_view rKey, const OUString& rLeft, const OUString& rRight)
{
    if (comparedExactly(rKey))
        return rLeft == rRight;
    const size_t nAt = rKey.rfind('@');
    const bool bMeasured
        = nAt != std::string_view::npos && measuresSomething(rLeft, rKey.substr(nAt + 1));
    return valuesEqual(rLeft, rRight, bMeasured);
}

/** Reads a number the dumper wrote under this key, or -1 where the side has none. */
sal_Int32 readCount(const std::map<OString, OUString>& rValues, const OString& rKey)
{
    const auto it = rValues.find(rKey);
    return it == rValues.end() ? -1 : it->second.toInt32();
}

/** Says whether the part of the path in front of this tag names the same object on both sides.

    Sheets and shapes are held by their index, so once one side holds a different number of them,
    every index after the first one missing names a different object, and comparing those says
    only that the count changed, which is reported on its own. */
bool sameObjectsOnBothSides(const std::map<OString, OUString>& rBefore,
                            const std::map<OString, OUString>& rAfter, std::string_view rKey,
                            std::string_view rTag, std::string_view rCount)
{
    const size_t nTag = rKey.find(rTag);
    if (nTag == std::string_view::npos)
        return true;
    const OString aCountKey
        = OString::Concat(nTag == 0 ? "document" : rKey.substr(0, nTag)) + "@" + rCount;
    return readCount(rBefore, aCountKey) == readCount(rAfter, aCountKey);
}

/** Says whether a cell, a column or a row that only one side holds tells anything.

    Each side is read as far as the sheet says it is used, and the two sides do not always agree on
    that: a cell the file lists with nothing but the default format counts as used when it is read
    and is not written again, which moves the end of the sheet. A cell beyond where the other side
    stopped reading was never compared, so calling it lost says only that the sheet ends earlier.
    A cell inside both is compared, and losing what the last cell held is exactly what moves the
    end, so it must not be hidden with the rest. The same goes for the formatting of a cell, which
    is read for only so many cells of a sheet, counted row by row. */
bool otherSideReadIt(const std::map<OString, OUString>& rOther, std::string_view rKey)
{
    const size_t nAt = rKey.rfind('@');
    if (nAt == std::string_view::npos)
        return true;

    auto sheetOf = [&rKey](size_t nBounded) { return OString(rKey.substr(0, nBounded)); };
    auto indexIn = [&rKey](size_t nOpen) {
        const size_t nClose = rKey.find(']', nOpen);
        return rKey.substr(nOpen, nClose == std::string_view::npos ? 0 : nClose - nOpen);
    };

    if (const size_t nColumn = rKey.find("/column["); nColumn != std::string_view::npos)
    {
        const sal_Int32 nUsed = readCount(rOther, sheetOf(nColumn) + "@UsedColumns");
        return nUsed < 0 || o3tl::toInt32(indexIn(nColumn + 8)) < nUsed;
    }
    if (const size_t nRow = rKey.find("/row["); nRow != std::string_view::npos)
    {
        const sal_Int32 nUsed = readCount(rOther, sheetOf(nRow) + "@UsedRows");
        return nUsed < 0 || o3tl::toInt32(indexIn(nRow + 5)) < nUsed;
    }

    const size_t nCell = rKey.find("/cell[");
    if (nCell == std::string_view::npos)
        return true;
    sal_Int32 nColumn = 0, nRow = 0;
    if (!parseCellAddress(indexIn(nCell + 6), nColumn, nRow))
        return true;
    const OString aSheet = sheetOf(nCell);
    const sal_Int32 nUsedColumns = readCount(rOther, aSheet + "@UsedColumns");
    const sal_Int32 nUsedRows = readCount(rOther, aSheet + "@UsedRows");
    if (nUsedColumns < 0 || nUsedRows < 0)
        return true;
    if (nColumn >= nUsedColumns || nRow >= nUsedRows)
        return false;

    // What the cell holds is read for every cell, its formatting only up to the limit.
    const std::string_view aProperty = rKey.substr(nAt + 1);
    if (aProperty == "Formula" || aProperty == "Type" || aProperty == "Value")
        return true;
    const sal_Int32 nFormatted = readCount(rOther, aSheet + "@CellsRead");
    return nFormatted < 0 || sal_Int64(nRow) * nUsedColumns + nColumn < nFormatted;
}

/** Says whether a difference at this key tells anything about the filter. */
bool tellsSomething(const std::map<OString, OUString>& rBefore,
                    const std::map<OString, OUString>& rAfter, std::string_view rKey)
{
    if (!sameObjectsOnBothSides(rBefore, rAfter, rKey, "sheet[", "SheetCount"))
        return false;
    if (!sameObjectsOnBothSides(rBefore, rAfter, rKey, "/shape[", "ShapeCount"))
        return false;

    // A shape of another type at the same index is another shape, so of all it states only the
    // type itself is worth a line.
    if (const size_t nShape = rKey.find("/shape["); nShape != std::string_view::npos)
    {
        const size_t nAt = rKey.rfind('@');
        if (rKey.substr(nAt + 1) != "ShapeType")
        {
            const OString aTypeKey = OString::Concat(rKey.substr(0, nAt)) + "@ShapeType";
            const auto aFirst = rBefore.find(aTypeKey);
            const auto aSecond = rAfter.find(aTypeKey);
            if (aFirst != rBefore.end() && aSecond != rAfter.end()
                && aFirst->second != aSecond->second)
                return false;
        }
    }
    return true;
}

std::vector<Diff> compare(const std::map<OString, OUString>& rBefore,
                          const std::map<OString, OUString>& rAfter)
{
    std::vector<Diff> aDiffs;
    for (const auto& rEntry : rBefore)
    {
        if (!tellsSomething(rBefore, rAfter, rEntry.first))
            continue;
        auto it = rAfter.find(rEntry.first);
        if (it == rAfter.end())
        {
            if (otherSideReadIt(rAfter, rEntry.first))
                aDiffs.push_back({ DiffKind::Lost, rEntry.first, rEntry.second, OUString() });
        }
        else if (!valuesEqual(rEntry.first, rEntry.second, it->second))
            aDiffs.push_back({ DiffKind::Changed, rEntry.first, rEntry.second, it->second });
    }
    for (const auto& rEntry : rAfter)
    {
        if (rBefore.find(rEntry.first) == rBefore.end()
            && tellsSomething(rBefore, rAfter, rEntry.first)
            && otherSideReadIt(rBefore, rEntry.first))
            aDiffs.push_back({ DiffKind::Added, rEntry.first, OUString(), rEntry.second });
    }
    return aDiffs;
}

const char* kindName(DiffKind eKind)
{
    switch (eKind)
    {
        case DiffKind::Lost:
            return "LOST";
        case DiffKind::Added:
            return "ADDED";
        case DiffKind::Changed:
            return "CHANGED";
    }
    return "?";
}

/** The report is tab separated, so a value must not carry tabs or newlines. */
OString flatten(std::u16string_view rText)
{
    OUStringBuffer aBuffer(rText.size() > MAX_VALUE_LEN ? rText.substr(0, MAX_VALUE_LEN)
                                                        : rText);
    for (sal_Int32 i = 0; i < aBuffer.getLength(); ++i)
    {
        if (aBuffer[i] == '\t' || aBuffer[i] == '\n' || aBuffer[i] == '\r')
            aBuffer[i] = ' ';
    }
    return OUStringToOString(aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
}

OString environment(const char* pName, const char* pDefault)
{
    const char* pValue = std::getenv(pName);
    return OString(pValue && *pValue ? pValue : pDefault);
}
}

class ScRoundtripDiffTest : public UnoApiXmlTest
{
public:
    ScRoundtripDiffTest()
        : UnoApiXmlTest(u"/sc/qa/unit/data/"_ustr)
    {
    }

    void testComparison();
    void testCorpus();

    CPPUNIT_TEST_SUITE(ScRoundtripDiffTest);
    CPPUNIT_TEST(testComparison);
    CPPUNIT_TEST(testCorpus);
    CPPUNIT_TEST_SUITE_END();

private:
    std::vector<OUString> collectCorpus(const OUString& rDirectoryUrl);
    void writeDump(const OString& rDirectory, std::string_view rName, std::string_view rStage,
                   const std::map<OString, OUString>& rValues);
};

std::vector<OUString> ScRoundtripDiffTest::collectCorpus(const OUString& rDirectoryUrl)
{
    std::vector<OUString> aFiles;
    osl::Directory aDirectory(rDirectoryUrl);
    if (aDirectory.open() != osl::FileBase::E_None)
        return aFiles;

    osl::DirectoryItem aItem;
    while (aDirectory.getNextItem(aItem) == osl::FileBase::E_None)
    {
        osl::FileStatus aStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName
                                | osl_FileStatus_Mask_FileURL);
        if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
            continue;
        if (aStatus.getFileType() != osl::FileStatus::Regular)
            continue;
        if (!aStatus.getFileName().endsWithIgnoreAsciiCase(u".xlsx"))
            continue;
        aFiles.push_back(aStatus.getFileURL());
    }
    std::sort(aFiles.begin(), aFiles.end());
    return aFiles;
}

void ScRoundtripDiffTest::writeDump(const OString& rDirectory, std::string_view rName,
                                    std::string_view rStage,
                                    const std::map<OString, OUString>& rValues)
{
    if (rDirectory.isEmpty())
        return;
    const OString aPath = OString::Concat(rDirectory) + "/" + rName + "." + rStage + ".dump";
    std::ofstream aStream(aPath.getStr());
    for (const auto& rEntry : rValues)
        aStream << rEntry.first << "\t" << flatten(rEntry.second) << "\n";
}

void ScRoundtripDiffTest::testComparison()
{
    // A length that went through another unit and back is the same length.
    CPPUNIT_ASSERT(valuesEqual("s@Width", u"(long) 2258"_ustr, u"(long) 2257"_ustr));
    // A small number is a constant: one more is another underline.
    CPPUNIT_ASSERT(!valuesEqual("s@CharUnderline", u"(short) 0"_ustr, u"(short) 1"_ustr));
    // A whole number that measures nothing is compared as it is, so one alignment is not
    // another.
    CPPUNIT_ASSERT(!valuesEqual("s@VertJustify", u"(long) 4"_ustr, u"(long) 3"_ustr));
    // Red and magenta are told apart, although a colour is a whole number like a length.
    CPPUNIT_ASSERT(!valuesEqual("s@CharColor", u"(long) 16711680"_ustr, u"(long) 16711935"_ustr));
    // A measure carried as a fraction moves by a fraction of itself.
    CPPUNIT_ASSERT(valuesEqual("s@CharHeight", u"(float) 12"_ustr, u"(float) 12.01"_ustr));
    // A number inside a string is part of the text.
    CPPUNIT_ASSERT(!valuesEqual("s@CellStyle", u"\"Heading 1\""_ustr, u"\"Heading 2\""_ustr));
    // What a cell holds and shows is compared as it is.
    CPPUNIT_ASSERT(!valuesEqual("s@NumberFormatCode", u"0.00"_ustr, u"0.000"_ustr));
    CPPUNIT_ASSERT(!valuesEqual("s@Formula", u"=SUM(A1:A10)"_ustr, u"=SUM(A1:A11)"_ustr));
    CPPUNIT_ASSERT(!valuesEqual("s@Type", u"1"_ustr, u"2"_ustr));

    sal_Int32 nColumn = 0, nRow = 0;
    CPPUNIT_ASSERT(parseCellAddress(cellAddress(27, 41), nColumn, nRow));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27), nColumn);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(41), nRow);

    // The last cell lost its content, which moved the end of the sheet: the loss is reported, the
    // cell beyond where the other side read is not.
    const std::map<OString, OUString> aBefore{
        { "document@SheetCount"_ostr, u"1"_ustr },
        { "sheet[0]@UsedColumns"_ostr, u"2"_ustr },
        { "sheet[0]@UsedRows"_ostr, u"3"_ustr },
        { "sheet[0]/cell[A3]@Formula"_ostr, u"x"_ustr },
        { "sheet[0]/cell[B3]@Formula"_ostr, u"y"_ustr },
        { "sheet[0]@ShapeCount"_ostr, u"2"_ustr },
        { "sheet[0]/shape[1]@Width"_ostr, u"(long) 100"_ustr },
    };
    const std::map<OString, OUString> aAfter{
        { "document@SheetCount"_ostr, u"1"_ustr },
        { "sheet[0]@UsedColumns"_ostr, u"1"_ustr },
        { "sheet[0]@UsedRows"_ostr, u"3"_ustr },
        { "sheet[0]@ShapeCount"_ostr, u"1"_ustr },
    };
    std::set<OString> aKeys;
    for (const Diff& rDiff : compare(aBefore, aAfter))
        aKeys.insert(rDiff.aKey);
    CPPUNIT_ASSERT(aKeys.count("sheet[0]/cell[A3]@Formula"_ostr));
    CPPUNIT_ASSERT(!aKeys.count("sheet[0]/cell[B3]@Formula"_ostr));
    CPPUNIT_ASSERT(aKeys.count("sheet[0]@UsedColumns"_ostr));
    // Once the shapes are counted differently, an index names another shape.
    CPPUNIT_ASSERT(aKeys.count("sheet[0]@ShapeCount"_ostr));
    CPPUNIT_ASSERT(!aKeys.count("sheet[0]/shape[1]@Width"_ostr));
}

void ScRoundtripDiffTest::testCorpus()
{
    // A corpus run takes hours, so it never happens as part of make check.
    if (!std::getenv("SC_RT_RUN") && !std::getenv("SC_RT_CORPUS"))
        return;

    skipValidation();

    OUString aCorpusUrl;
    const OString aCorpus = environment("SC_RT_CORPUS", "");
    if (aCorpus.isEmpty())
        aCorpusUrl = createFileURL(u"xlsx/");
    else
        osl::FileBase::getFileURLFromSystemPath(OStringToOUString(aCorpus, RTL_TEXTENCODING_UTF8),
                                                aCorpusUrl);

    const OString aReportPath = environment("SC_RT_REPORT", "sc-roundtrip.log");
    const OString aDumpDirectory = environment("SC_RT_DUMPDIR", "");
    const sal_Int32 nLimit = environment("SC_RT_LIMIT", "0").toInt32();
    const sal_Int32 nCellLimit = environment("SC_RT_CELLS", "2000").toInt32();

    // Resume: a document already reported on is not read again, so that the run can be restarted
    // after one of them takes the process down. The one that did has a START line and nothing
    // after it; reading it again would take the process down again, so it is reported as the
    // cause and passed over.
    std::set<OString> aDone;
    std::vector<OString> aStarted;
    {
        std::ifstream aStream(aReportPath.getStr());
        std::string aLine;
        auto nameIn = [&aLine](size_t nTagLength) {
            const std::string aRest = aLine.substr(nTagLength);
            return OString(aRest.substr(0, aRest.find('\t')).c_str());
        };
        while (std::getline(aStream, aLine))
        {
            if (aLine.compare(0, 5, "FILE\t") == 0)
                aDone.insert(nameIn(5));
            else if (aLine.compare(0, 6, "START\t") == 0)
                aStarted.push_back(nameIn(6));
        }
    }

    std::vector<OUString> aFiles = collectCorpus(aCorpusUrl);
    CPPUNIT_ASSERT_MESSAGE("no documents in the corpus", !aFiles.empty());

    std::ofstream aReport(aReportPath.getStr(), std::ios::app);
    for (const OString& rName : aStarted)
    {
        if (aDone.insert(rName).second)
            aReport << "FILE\t" << rName << "\tCRASH\n";
    }
    aReport.flush();
    sal_Int32 nProcessed = 0;

    for (const OUString& rUrl : aFiles)
    {
        const OString aName
            = OUStringToOString(rUrl.subView(rUrl.lastIndexOf('/') + 1), RTL_TEXTENCODING_UTF8);
        if (aDone.count(aName))
            continue;
        if (nLimit && nProcessed >= nLimit)
            break;
        ++nProcessed;

        // Named before the work starts: if the process dies on this document, the report still
        // says which one it was.
        aReport << "START\t" << aName << "\n";
        aReport.flush();

        ModelDumper aFirst(nCellLimit), aSecond(nCellLimit), aThird(nCellLimit);
        try
        {
            loadFromURL(rUrl);
            aFirst.dump(mxComponent);
            saveAndReload(TestFilter::XLSX);
            aSecond.dump(mxComponent);
            saveAndReload(TestFilter::XLSX);
            aThird.dump(mxComponent);
        }
        catch (const std::exception& rException)
        {
            aReport << "FILE\t" << aName << "\tERROR\t"
                    << flatten(OStringToOUString(rException.what(), RTL_TEXTENCODING_UTF8))
                    << "\n";
            aReport.flush();
            continue;
        }
        catch (...)
        {
            aReport << "FILE\t" << aName << "\tERROR\tunknown exception\n";
            aReport.flush();
            continue;
        }

        writeDump(aDumpDirectory, aName, "a", aFirst.maValues);
        writeDump(aDumpDirectory, aName, "b", aSecond.maValues);
        writeDump(aDumpDirectory, aName, "c", aThird.maValues);

        const std::vector<Diff> aLossy = compare(aFirst.maValues, aSecond.maValues);
        const std::vector<Diff> aUnstable = compare(aSecond.maValues, aThird.maValues);

        aReport << "FILE\t" << aName << "\tOK\t" << aFirst.maValues.size() << "\t" << aLossy.size()
                << "\t" << aUnstable.size() << "\n";
        for (const Diff& rDiff : aLossy)
            aReport << "D1\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        for (const Diff& rDiff : aUnstable)
            aReport << "D2\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        aReport.flush();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScRoundtripDiffTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
