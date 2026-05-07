/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dppivotstyle.hxx>

#include <document.hxx>
#include <dpobject.hxx>
#include <patattr.hxx>
#include <scitems.hxx>
#include <tablestyle.hxx>

#include <map>

namespace sc
{
namespace
{
/// Apply a pattern (sparse — only items it explicitly contains) over the
/// given absolute cell range using merge semantics.
void applyOverRange(ScDocument& rDoc, SCTAB nTab, SCCOL nCol1, SCROW nRow1,
                    SCCOL nCol2, SCROW nRow2, const ScPatternAttr& rPattern)
{
    if (nCol2 < nCol1 || nRow2 < nRow1)
        return;
    rDoc.ApplyPatternAreaTab(nCol1, nRow1, nCol2, nRow2, nTab, rPattern);
}

/// Apply a pattern to a single row across the column range.
void applyOverRow(ScDocument& rDoc, SCTAB nTab, SCROW nRow, SCCOL nCol1, SCCOL nCol2,
                  const ScPatternAttr& rPattern)
{
    applyOverRange(rDoc, nTab, nCol1, nRow, nCol2, nRow, rPattern);
}

/// Apply a pattern to a single column across the row range.
void applyOverCol(ScDocument& rDoc, SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2,
                  const ScPatternAttr& rPattern)
{
    applyOverRange(rDoc, nTab, nCol, nRow1, nCol, nRow2, rPattern);
}

/// Translate a pivot-style level (0..2) to the matching subtotal-row /
/// subtotal-column / row-subheading / col-subheading element type.
ScTableStyleElement subtotalRowElement(sal_Int8 nLevel)
{
    switch (nLevel % 3)
    {
        case 0:
            return ScTableStyleElement::FirstSubtotalRow;
        case 1:
            return ScTableStyleElement::SecondSubtotalRow;
        default:
            return ScTableStyleElement::ThirdSubtotalRow;
    }
}

ScTableStyleElement subtotalColumnElement(sal_Int8 nLevel)
{
    switch (nLevel % 3)
    {
        case 0:
            return ScTableStyleElement::FirstSubtotalColumn;
        case 1:
            return ScTableStyleElement::SecondSubtotalColumn;
        default:
            return ScTableStyleElement::ThirdSubtotalColumn;
    }
}

ScTableStyleElement rowSubheadingElement(size_t nLevel)
{
    switch (nLevel % 3)
    {
        case 0:
            return ScTableStyleElement::FirstRowSubheading;
        case 1:
            return ScTableStyleElement::SecondRowSubheading;
        default:
            return ScTableStyleElement::ThirdRowSubheading;
    }
}

ScTableStyleElement columnSubheadingElement(size_t nLevel)
{
    switch (nLevel % 3)
    {
        case 0:
            return ScTableStyleElement::FirstColumnSubheading;
        case 1:
            return ScTableStyleElement::SecondColumnSubheading;
        default:
            return ScTableStyleElement::ThirdColumnSubheading;
    }
}

/// Lookup helper — returns the pattern for an element or nullptr.
const ScPatternAttr* getPattern(
    const std::map<ScTableStyleElement, const ScPatternAttr*>& rPatterns,
    ScTableStyleElement eElement)
{
    auto it = rPatterns.find(eElement);
    return it == rPatterns.end() ? nullptr : it->second;
}

} // anonymous namespace

void applyPivotTableStyle(ScDocument& rDoc, SCTAB nTab,
                          const PivotTableStyleInfo& rStyleInfo,
                          const ScDPOutput::StyleRoles& rRoles)
{
    if (rStyleInfo.maName.isEmpty())
        return;

    const ScTableStyles* pStyles = rDoc.GetTableStyles();
    if (!pStyles)
        return;

    const ScTableStyle* pStyle = pStyles->GetTableStyle(rStyleInfo.maName);
    if (!pStyle)
        return;

    const auto aPatterns = pStyle->GetSetPatterns();
    if (aPatterns.empty())
        return;


    // Pivot region geometry
    const SCCOL nDataCol1 = rRoles.nDataStartCol;
    const SCCOL nDataCol2 = rRoles.nTabEndCol;
    const SCROW nDataRow1 = rRoles.nDataStartRow;
    const SCROW nDataRow2 = rRoles.nTabEndRow;
    const SCCOL nTabCol1 = rRoles.nTabStartCol;
    const SCROW nTabRow1 = rRoles.nTabStartRow;

    // ECMA-376 §18.18.83 application order: general → specific. Later layers
    // override earlier (ScDocument::ApplyPatternAreaTab uses merge semantics
    // so set items in the new pattern replace those in existing patterns).

    // 1. wholeTable — over the entire pivot range, including any page fields
    //    rows above the table proper.
    if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::WholeTable))
    {
        const SCCOL nCol1 = nTabCol1;
        const SCROW nRow1 = (rRoles.nPageFieldFirstRow >= 0) ? rRoles.nPageFieldFirstRow : nTabRow1;
        applyOverRange(rDoc, nTab, nCol1, nRow1, nDataCol2, nDataRow2, *p);
    }

    // 2. firstColumnStripe / secondColumnStripe — alternate pure-data columns.
    //    Stripe row index counts only pure-data columns (subtotal/grand cols
    //    do not advance the counter). Gated by showColStripes.
    if (rStyleInfo.mbShowColStripes && rRoles.pColIsPureData
        && !rRoles.pColIsPureData->empty())
    {
        const ScPatternAttr* p1 = getPattern(aPatterns, ScTableStyleElement::FirstColumnStripe);
        const ScPatternAttr* p2 = getPattern(aPatterns, ScTableStyleElement::SecondColumnStripe);
        if (p1 || p2)
        {
            sal_Int32 nFirstSize = pStyle->GetFirstColumnStripeSize();
            sal_Int32 nSecondSize = pStyle->GetSecondColumnStripeSize();
            if (nFirstSize < 1)
                nFirstSize = 1;
            if (nSecondSize < 1)
                nSecondSize = 1;
            const sal_Int32 nTotal = nFirstSize + nSecondSize;
            sal_Int32 nDataIdx = 0;
            for (size_t i = 0; i < rRoles.pColIsPureData->size(); ++i)
            {
                if (!(*rRoles.pColIsPureData)[i])
                    continue;
                const SCCOL nColPos = nDataCol1 + static_cast<SCCOL>(i);
                const bool bFirst = (nDataIdx % nTotal) < nFirstSize;
                if (bFirst && p1)
                    applyOverCol(rDoc, nTab, nColPos, nDataRow1, nDataRow2, *p1);
                else if (!bFirst && p2)
                    applyOverCol(rDoc, nTab, nColPos, nDataRow1, nDataRow2, *p2);
                ++nDataIdx;
            }
        }
    }

    // 3. firstRowStripe / secondRowStripe — alternate pure-data rows. Same
    //    counter semantics as columns.
    if (rStyleInfo.mbShowRowStripes && rRoles.pRowIsPureData
        && !rRoles.pRowIsPureData->empty())
    {
        const ScPatternAttr* p1 = getPattern(aPatterns, ScTableStyleElement::FirstRowStripe);
        const ScPatternAttr* p2 = getPattern(aPatterns, ScTableStyleElement::SecondRowStripe);
        if (p1 || p2)
        {
            sal_Int32 nFirstSize = pStyle->GetFirstRowStripeSize();
            sal_Int32 nSecondSize = pStyle->GetSecondRowStripeSize();
            if (nFirstSize < 1)
                nFirstSize = 1;
            if (nSecondSize < 1)
                nSecondSize = 1;
            const sal_Int32 nTotal = nFirstSize + nSecondSize;
            sal_Int32 nDataIdx = 0;
            for (size_t i = 0; i < rRoles.pRowIsPureData->size(); ++i)
            {
                if (!(*rRoles.pRowIsPureData)[i])
                    continue;
                const SCROW nRowPos = nDataRow1 + static_cast<SCROW>(i);
                const bool bFirst = (nDataIdx % nTotal) < nFirstSize;
                if (bFirst && p1)
                    applyOverRow(rDoc, nTab, nRowPos, nDataCol1, nDataCol2, *p1);
                else if (!bFirst && p2)
                    applyOverRow(rDoc, nTab, nRowPos, nDataCol1, nDataCol2, *p2);
                ++nDataIdx;
            }
        }
    }

    // 4. firstColumn / lastColumn — first-row-label-column and last column.
    //    showRowHeaders gates firstColumn (it's the leftmost row-label col).
    //    showLastColumn gates lastColumn (rightmost data column or grand
    //    total col).
    if (rStyleInfo.mbShowRowHeaders)
    {
        if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::FirstColumn))
            applyOverCol(rDoc, nTab, nTabCol1, nDataRow1, nDataRow2, *p);
    }
    if (rStyleInfo.mbShowLastColumn)
    {
        if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::LastColumn))
            applyOverCol(rDoc, nTab, nDataCol2, nDataRow1, nDataRow2, *p);
    }

    // 5. Row subheadings — apply each element to the row-label column at the
    //    matching field-index level (0..2 mod 3). The row-axis subheadings
    //    cover the row-label columns within the data row range.
    if (rStyleInfo.mbShowRowHeaders)
    {
        for (size_t nLevel = 0; nLevel < rRoles.nNumRowFields; ++nLevel)
        {
            const ScPatternAttr* p = getPattern(aPatterns, rowSubheadingElement(nLevel));
            if (!p)
                continue;
            // Row-label columns sit between nTabStartCol and nDataStartCol-1;
            // each row-axis level corresponds to a column at that offset (in
            // non-compact layouts). For compact layouts multiple fields share
            // a single column — we still apply at the level's offset since
            // the same pattern applies to the same column anyway.
            const SCCOL nCol = nTabCol1 + static_cast<SCCOL>(nLevel);
            if (nCol >= nDataCol1)
                continue; // out of row-label band
            applyOverCol(rDoc, nTab, nCol, nDataRow1, nDataRow2, *p);
        }
    }

    // 6. Column subheadings — apply to the column-field button rows at the
    //    matching field-index level. Each col-axis field has its own row
    //    starting at nMemberStartRow.
    if (rStyleInfo.mbShowColHeaders)
    {
        for (size_t nLevel = 0; nLevel < rRoles.nNumColFields; ++nLevel)
        {
            const ScPatternAttr* p = getPattern(aPatterns, columnSubheadingElement(nLevel));
            if (!p)
                continue;
            const SCROW nRow = rRoles.nMemberStartRow + static_cast<SCROW>(nLevel);
            if (nRow >= nDataRow1)
                continue;
            applyOverRow(rDoc, nTab, nRow, nDataCol1, nDataCol2, *p);
        }
    }

    // 7. Subtotal rows (level-specific) — rows where maRowSubtotalLevel >= 0.
    if (rRoles.pRowSubtotalLevel)
    {
        for (size_t i = 0; i < rRoles.pRowSubtotalLevel->size(); ++i)
        {
            const sal_Int8 nLevel = (*rRoles.pRowSubtotalLevel)[i];
            if (nLevel < 0)
                continue;
            const ScPatternAttr* p = getPattern(aPatterns, subtotalRowElement(nLevel));
            if (!p)
                continue;
            const SCROW nRowPos = nDataRow1 + static_cast<SCROW>(i);
            applyOverRow(rDoc, nTab, nRowPos, nTabCol1, nDataCol2, *p);
        }
    }

    // Subtotal columns (level-specific).
    if (rRoles.pColSubtotalLevel)
    {
        for (size_t i = 0; i < rRoles.pColSubtotalLevel->size(); ++i)
        {
            const sal_Int8 nLevel = (*rRoles.pColSubtotalLevel)[i];
            if (nLevel < 0)
                continue;
            const ScPatternAttr* p = getPattern(aPatterns, subtotalColumnElement(nLevel));
            if (!p)
                continue;
            const SCCOL nColPos = nDataCol1 + static_cast<SCCOL>(i);
            applyOverCol(rDoc, nTab, nColPos, nDataRow1, nDataRow2, *p);
        }
    }

    // 8. blankRow — separator rows in compact layout. Currently not tracked
    //    (maRowIsBlank stays empty/false until a future compact-layout pass
    //    populates it). Default false → no-op.
    if (rRoles.pRowIsBlank && getPattern(aPatterns, ScTableStyleElement::BlankRow))
    {
        const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::BlankRow);
        for (size_t i = 0; i < rRoles.pRowIsBlank->size(); ++i)
        {
            if (!(*rRoles.pRowIsBlank)[i])
                continue;
            const SCROW nRowPos = nDataRow1 + static_cast<SCROW>(i);
            applyOverRow(rDoc, nTab, nRowPos, nTabCol1, nDataCol2, *p);
        }
    }

    // 9. pageFieldLabels / pageFieldValues — the rows above the pivot proper
    //    when page fields are present. Label is column maStartPos.Col(),
    //    value is +1 (per outputPageFields convention).
    if (rRoles.nPageFieldFirstRow >= 0)
    {
        if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::PageFieldLabels))
        {
            applyOverRange(rDoc, nTab, nTabCol1, rRoles.nPageFieldFirstRow,
                           nTabCol1, rRoles.nPageFieldLastRow, *p);
        }
        if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::PageFieldValues))
        {
            applyOverRange(rDoc, nTab, nTabCol1 + 1, rRoles.nPageFieldFirstRow,
                           nTabCol1 + 1, rRoles.nPageFieldLastRow, *p);
        }
    }

    // 10. headerRow — the topmost band of the pivot output, where field-button
    //     captions live. Spans [nMemberStartRow, nDataStartRow-1] inclusive
    //     when nMemberStartRow < nDataStartRow (i.e. there are column fields),
    //     and the single row at nDataStartRow-1 otherwise (the row-field
    //     button caption row that always sits just above the data).
    if (rStyleInfo.mbShowColHeaders)
    {
        if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::HeaderRow))
        {
            const SCROW nFromRow = (rRoles.nMemberStartRow < nDataRow1)
                                       ? rRoles.nMemberStartRow
                                       : (nDataRow1 > 0 ? nDataRow1 - 1 : nDataRow1);
            const SCROW nToRow = (nDataRow1 > 0) ? nDataRow1 - 1 : nDataRow1;
            if (nFromRow <= nToRow)
                applyOverRange(rDoc, nTab, nTabCol1, nFromRow, nDataCol2, nToRow, *p);
        }
    }

    // 11. totalRow — grand total row at the bottom (any row with
    //     maRowIsGrandTotal set).
    if (rRoles.pRowIsGrandTotal)
    {
        if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::TotalRow))
        {
            for (size_t i = 0; i < rRoles.pRowIsGrandTotal->size(); ++i)
            {
                if (!(*rRoles.pRowIsGrandTotal)[i])
                    continue;
                const SCROW nRowPos = nDataRow1 + static_cast<SCROW>(i);
                applyOverRow(rDoc, nTab, nRowPos, nTabCol1, nDataCol2, *p);
            }
        }
    }

    // 12. Corner cells — most specific. firstHeaderCell / lastHeaderCell
    //     overlay the column-header band's leftmost / rightmost cell.
    //     firstTotalCell / lastTotalCell overlay the grand-total row's
    //     leftmost / rightmost cell.
    if (rStyleInfo.mbShowColHeaders)
    {
        const SCROW nHdrRow = (nDataRow1 > 0) ? nDataRow1 - 1 : nDataRow1;
        if (nHdrRow >= nTabRow1 && nHdrRow < nDataRow1)
        {
            if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::FirstHeaderCell))
                applyOverRange(rDoc, nTab, nTabCol1, nHdrRow, nTabCol1, nHdrRow, *p);
            if (const ScPatternAttr* p = getPattern(aPatterns, ScTableStyleElement::LastHeaderCell))
                applyOverRange(rDoc, nTab, nDataCol2, nHdrRow, nDataCol2, nHdrRow, *p);
        }
    }
    if (rRoles.pRowIsGrandTotal)
    {
        const ScPatternAttr* pFirst = getPattern(aPatterns, ScTableStyleElement::FirstTotalCell);
        const ScPatternAttr* pLast = getPattern(aPatterns, ScTableStyleElement::LastTotalCell);
        if (pFirst || pLast)
        {
            for (size_t i = 0; i < rRoles.pRowIsGrandTotal->size(); ++i)
            {
                if (!(*rRoles.pRowIsGrandTotal)[i])
                    continue;
                const SCROW nRowPos = nDataRow1 + static_cast<SCROW>(i);
                if (pFirst)
                    applyOverRange(rDoc, nTab, nTabCol1, nRowPos, nTabCol1, nRowPos, *pFirst);
                if (pLast)
                    applyOverRange(rDoc, nTab, nDataCol2, nRowPos, nDataCol2, nRowPos, *pLast);
            }
        }
    }
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
