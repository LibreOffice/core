/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <scitems.hxx>
#include <attrib.hxx>
#include <formulacell.hxx>
#include <table.hxx>
#include <column.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <global.hxx>
#include <stlpool.hxx>
#include <tabprotection.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <segmenttree.hxx>
#include <columniterator.hxx>
#include <globalnames.hxx>
#include <scmod.hxx>
#include <printopt.hxx>
#include <bcaslot.hxx>
#include <compressedarray.hxx>
#include <userdat.hxx>
#include <conditio.hxx>
#include <colorscale.hxx>
#include <cellform.hxx>

#include <com/sun/star/sheet/TablePageBreakData.hpp>

#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <osl/diagnose.h>
#include <svl/numformat.hxx>

#include <algorithm>
#include <limits>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::sheet::TablePageBreakData;
using ::std::set;

void ScTable::UpdatePageBreaks(const ScRange* pUserArea)
{
    if (rDocument.IsImportingXML())
        return;

    // pUserArea != NULL -> print area is specified.  We need to force-update
    // the page breaks.

    if (!pUserArea)
    {
        if (!bPageSizeValid)
            return;

        // Always update breaks if force breaks option has changed
        if (mbPageBreaksValid
            && mbForceBreaks == ScModule::get()->GetPrintOptions().GetForceBreaks())
            return;
    }

    SfxStyleSheetBase* pStyle
        = rDocument.GetStyleSheetPool()->Find(aPageStyle, SfxStyleFamily::Page);
    if (!pStyle)
    {
        OSL_FAIL("UpdatePageBreaks: Style not found");
        return;
    }
    SfxItemSet* pStyleSet = &pStyle->GetItemSet();

    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = rDocument.MaxCol();
    SCROW nEndRow = rDocument.MaxRow();
    if (pUserArea)
    {
        nStartCol = pUserArea->aStart.Col();
        nStartRow = pUserArea->aStart.Row();
        nEndCol = pUserArea->aEnd.Col();
        nEndRow = pUserArea->aEnd.Row();
    }
    else
    {
        sal_uInt16 nAreaCount = GetPrintRangeCount();
        if (nAreaCount > 1)
        {
            // Show nothing, when multiple ranges

            for (SCCOL nX : GetColumnsRange(0, rDocument.MaxCol()))
                RemoveColBreak(nX, true, false);

            RemoveRowPageBreaks(0, rDocument.MaxRow() - 1);

            return;
        }
        else if (nAreaCount == 1)
        {
            const ScRange* pArea = GetPrintRange(0);
            if (pArea)
            {
                nStartCol = pArea->aStart.Col();
                nStartRow = pArea->aStart.Row();
                nEndCol = pArea->aEnd.Col();
                nEndRow = pArea->aEnd.Row();
            }
        } // otherwise show everything
    }

    // get bSkipColBreaks/bSkipRowBreaks flags:
    // fdo#40788 - print range scale settings can cause manual breaks to be
    // ignored (see below). This behaviour may now be set by the user.
    mbForceBreaks = ScModule::get()->GetPrintOptions().GetForceBreaks();
    bool bSkipColBreaks = false;
    bool bSkipRowBreaks = false;

    if (!mbForceBreaks)
    {
        if (const SfxUInt16Item* pItem = pStyleSet->GetItemIfSet(ATTR_PAGE_SCALETOPAGES, false))
        {
            bSkipColBreaks = bSkipRowBreaks = pItem->GetValue() > 0;
        }

        const ScPageScaleToItem* pScaleToItem;
        if (!bSkipColBreaks && (pScaleToItem = pStyleSet->GetItemIfSet(ATTR_PAGE_SCALETO, false)))
        {
            // #i54993# when fitting to width or height, ignore only manual breaks in that direction
            if (pScaleToItem->GetWidth() > 0)
                bSkipColBreaks = true;
            if (pScaleToItem->GetHeight() > 0)
                bSkipRowBreaks = true;
        }
    }

    tools::Long nPageSizeX = aPageSizeTwips.Width();
    tools::Long nPageSizeY = aPageSizeTwips.Height();

    //  Beginning: Remove breaks

    for (SCCOL nX : GetColumnsRange(0, nStartCol - 1))
        RemoveColBreak(nX, true, false);
    RemoveRowPageBreaks(0, nStartRow - 1);

    if (nStartCol > 0)
        SetColBreak(nStartCol, true, false); // AREABREAK
    if (nStartRow > 0)
        SetRowBreak(nStartRow, true, false); // AREABREAK

    //  Middle part: Distribute breaks

    bool bRepeatCol = (nRepeatStartX != SCCOL_REPEAT_NONE);
    bool bColFound = false;
    tools::Long nSizeX = 0;
    for (SCCOL nX = nStartCol; nX <= nEndCol; nX++)
    {
        bool bStartOfPage = false;
        tools::Long nThisX = ColHidden(nX) ? 0 : mpColWidth->GetValue(nX);
        bool bManualBreak = HasColManualBreak(nX);
        if ((nSizeX + nThisX > nPageSizeX) || (bManualBreak && !bSkipColBreaks))
        {
            SetColBreak(nX, true, false);
            nSizeX = 0;
            bStartOfPage = true;
        }
        else if (nX != nStartCol)
            RemoveColBreak(nX, true, false);
        else
            bStartOfPage = true;

        if (bStartOfPage && bRepeatCol && nX > nRepeatStartX && !bColFound)
        {
            // subtract size of repeat columns from page size
            for (SCCOL i = nRepeatStartX; i <= nRepeatEndX; i++)
                nPageSizeX -= ColHidden(i) ? 0 : mpColWidth->GetValue(i);
            while (nX <= nRepeatEndX)
                RemoveColBreak(++nX, true, false);
            bColFound = true;
        }

        nSizeX += nThisX;
    }

    // Remove all page breaks in range.
    RemoveRowPageBreaks(nStartRow + 1, nEndRow);

    // And set new page breaks.
    bool bRepeatRow = (nRepeatStartY != SCROW_REPEAT_NONE);
    bool bRowFound = false;
    tools::Long nSizeY = 0;
    ScFlatBoolRowSegments::ForwardIterator aIterHidden(*maFilterData.mpHiddenRows);
    ScFlatUInt16RowSegments::ForwardIterator aIterHeights(*mpRowHeights);
    SCROW nNextManualBreak = GetNextManualBreak(nStartRow); // -1 => no more manual breaks
    for (SCROW nY = nStartRow; nY <= nEndRow; ++nY)
    {
        bool bStartOfPage = false;
        bool bThisRowHidden = false;
        const bool bHasValue = aIterHidden.getValue(nY, bThisRowHidden);
        assert(bHasValue);
        (void)bHasValue;
        tools::Long nThisY = 0;
        if (!bThisRowHidden)
        {
            sal_uInt16 nTmp;
            const bool bHasHeight = aIterHeights.getValue(nY, nTmp);
            assert(bHasHeight);
            if (bHasHeight)
                nThisY = static_cast<tools::Long>(nTmp);
        }

        bool bManualBreak = false;
        if (nNextManualBreak >= 0)
        {
            bManualBreak = (nY == nNextManualBreak);
            if (nY >= nNextManualBreak)
                // Query the next manual break position.
                nNextManualBreak = GetNextManualBreak(nY + 1);
        }

        if ((nSizeY + nThisY > nPageSizeY) || (bManualBreak && !bSkipRowBreaks))
        {
            SetRowBreak(nY, true, false);
            nSizeY = 0;
            bStartOfPage = true;
        }
        else if (nY != nStartRow)
            ; // page break already removed
        else
            bStartOfPage = true;

        if (bStartOfPage && bRepeatRow && nY > nRepeatStartY && !bRowFound)
        {
            // subtract size of repeat rows from page size
            tools::Long nHeights = GetTotalRowHeight(nRepeatStartY, nRepeatEndY);
#if OSL_DEBUG_LEVEL > 0
            if (nHeights == ::std::numeric_limits<tools::Long>::max())
                OSL_FAIL("ScTable::UpdatePageBreaks: row heights overflow");
#endif
            nPageSizeY -= nHeights;
            if (nY <= nRepeatEndY)
                RemoveRowPageBreaks(nY, nRepeatEndY);
            bRowFound = true;
        }

        if (bThisRowHidden)
        {
            // Hidden row range.  Skip them unless there is a manual break.
            SCROW nLastCommon = aIterHidden.getLastPos();
            if (nNextManualBreak >= 0)
                nLastCommon = ::std::min(nLastCommon, nNextManualBreak - 1);
            nY = nLastCommon;
        }
        else
        {
            // Visible row range.

            SCROW nLastHidden = aIterHidden.getLastPos();
            SCROW nLastHeight = aIterHeights.getLastPos();
            SCROW nLastCommon = ::std::min(nLastHidden, nLastHeight);
            if (nNextManualBreak >= 0)
                nLastCommon = ::std::min(nLastCommon, nNextManualBreak - 1);

            if (nLastCommon > nY)
            {
                tools::Long nMaxMultiple = static_cast<tools::Long>(nLastCommon - nY);
                tools::Long nMultiple = (nPageSizeY - nSizeY) / nThisY;
                if (nMultiple > nMaxMultiple)
                    nMultiple = nMaxMultiple;
                if (nMultiple > 1)
                {
                    nSizeY += nThisY * (nMultiple - 1);
                    nY += nMultiple - 1;
                }
            }
        }

        nSizeY += nThisY;
    }

    //  End: Remove Break

    if (nEndCol < rDocument.MaxCol())
    {
        SetColBreak(nEndCol + 1, true, false); // AREABREAK
        for (SCCOL nCol : GetColumnsRange(nEndCol + 2, rDocument.MaxCol()))
            RemoveColBreak(nCol, true, false);
    }
    if (nEndRow < rDocument.MaxRow())
    {
        SetRowBreak(nEndRow + 1, true, false); // AREABREAK
        if (nEndRow + 2 <= rDocument.MaxRow())
            RemoveRowPageBreaks(nEndRow + 2, rDocument.MaxRow());
    }
    mbPageBreaksValid
        = !pUserArea; // #i116881# the valid flag can only apply to the "no user area" case
}

void ScTable::RemoveManualBreaks()
{
    maRowManualBreaks.clear();
    maColManualBreaks.clear();
    InvalidatePageBreaks();

    SetStreamValid(false);
}

bool ScTable::HasManualBreaks() const
{
    return !maRowManualBreaks.empty() || !maColManualBreaks.empty();
}

void ScTable::SetRowManualBreaks(::std::set<SCROW>&& rBreaks)
{
    maRowManualBreaks = std::move(rBreaks);
    InvalidatePageBreaks();
    SetStreamValid(false);
}

void ScTable::SetColManualBreaks(::std::set<SCCOL>&& rBreaks)
{
    maColManualBreaks = std::move(rBreaks);
    InvalidatePageBreaks();
    SetStreamValid(false);
}

void ScTable::GetAllRowBreaks(set<SCROW>& rBreaks, bool bPage, bool bManual) const
{
    if (bPage)
        rBreaks = maRowPageBreaks;

    if (bManual)
    {
        copy(maRowManualBreaks.begin(), maRowManualBreaks.end(),
             inserter(rBreaks, rBreaks.begin()));
    }
}

void ScTable::GetAllColBreaks(set<SCCOL>& rBreaks, bool bPage, bool bManual) const
{
    if (bPage)
        rBreaks = maColPageBreaks;

    if (bManual)
    {
        copy(maColManualBreaks.begin(), maColManualBreaks.end(),
             inserter(rBreaks, rBreaks.begin()));
    }
}

bool ScTable::HasRowPageBreak(SCROW nRow) const
{
    if (!ValidRow(nRow))
        return false;

    return maRowPageBreaks.find(nRow) != maRowPageBreaks.end();
}

bool ScTable::HasColPageBreak(SCCOL nCol) const
{
    if (!ValidCol(nCol))
        return false;

    return maColPageBreaks.find(nCol) != maColPageBreaks.end();
}

bool ScTable::HasRowManualBreak(SCROW nRow) const
{
    if (!ValidRow(nRow))
        return false;

    return maRowManualBreaks.find(nRow) != maRowManualBreaks.end();
}

bool ScTable::HasColManualBreak(SCCOL nCol) const
{
    if (!ValidCol(nCol))
        return false;

    return maColManualBreaks.find(nCol) != maColManualBreaks.end();
}

SCROW ScTable::GetNextManualBreak(SCROW nRow) const
{
    set<SCROW>::const_iterator itr = maRowManualBreaks.lower_bound(nRow);
    return itr == maRowManualBreaks.end() ? -1 : *itr;
}

void ScTable::RemoveRowPageBreaks(SCROW nStartRow, SCROW nEndRow)
{
    if (!ValidRow(nStartRow) || !ValidRow(nEndRow))
        return;

    set<SCROW>::iterator low = maRowPageBreaks.lower_bound(nStartRow);
    set<SCROW>::iterator high = maRowPageBreaks.upper_bound(nEndRow);
    maRowPageBreaks.erase(low, high);
}

void ScTable::RemoveRowBreak(SCROW nRow, bool bPage, bool bManual)
{
    if (!ValidRow(nRow))
        return;

    if (bPage)
        maRowPageBreaks.erase(nRow);

    if (bManual)
    {
        maRowManualBreaks.erase(nRow);
        InvalidatePageBreaks();
    }
}

void ScTable::RemoveColBreak(SCCOL nCol, bool bPage, bool bManual)
{
    if (!ValidCol(nCol))
        return;

    if (bPage)
        maColPageBreaks.erase(nCol);

    if (bManual)
    {
        maColManualBreaks.erase(nCol);
        InvalidatePageBreaks();
    }
}

void ScTable::SetRowBreak(SCROW nRow, bool bPage, bool bManual)
{
    if (!ValidRow(nRow))
        return;

    if (bPage)
        maRowPageBreaks.insert(nRow);

    if (bManual)
    {
        maRowManualBreaks.insert(nRow);
        InvalidatePageBreaks();
    }
}

void ScTable::SetColBreak(SCCOL nCol, bool bPage, bool bManual)
{
    if (!ValidCol(nCol))
        return;

    if (bPage)
        maColPageBreaks.insert(nCol);

    if (bManual)
    {
        maColManualBreaks.insert(nCol);
        InvalidatePageBreaks();
    }
}

Sequence<TablePageBreakData> ScTable::GetRowBreakData() const
{
    using ::std::inserter;

    set<SCROW> aRowBreaks = maRowPageBreaks;
    copy(maRowManualBreaks.begin(), maRowManualBreaks.end(),
         inserter(aRowBreaks, aRowBreaks.begin()));

    Sequence<TablePageBreakData> aSeq(aRowBreaks.size());
    std::transform(aRowBreaks.begin(), aRowBreaks.end(), aSeq.getArray(), [this](const SCROW nRow) {
        return TablePageBreakData(nRow, HasRowManualBreak(nRow));
    });

    return aSeq;
}

bool ScTable::RowHidden(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    return maFilterData.rowHidden(nRow, pFirstRow, pLastRow);
}

bool ScTable::RowHiddenLeaf(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    return maFilterData.rowHiddenLeaf(nRow, pFirstRow, pLastRow);
}

bool ScTable::HasHiddenRows(SCROW nStartRow, SCROW nEndRow) const
{
    return maFilterData.hasHiddenRows(nStartRow, nEndRow);
}

bool ScTable::ColHidden(SCCOL nCol, SCCOL* pFirstCol, SCCOL* pLastCol) const
{
    return maFilterData.colHidden(nCol, pFirstCol, pLastCol);
}

void ScTable::updateObjectsForColsChanged(SCCOL nStartCol, SCCOL nEndCol, bool bHidden,
                                          bool bChanged)
{
    // Cell anchored objects might change visibility
    ScDrawLayer* pDrawLayer = GetDoc().GetDrawLayer();
    if (pDrawLayer)
    {
        std::vector<SdrObject*> aColDrawObjects;
        aColDrawObjects = pDrawLayer->GetObjectsAnchoredToCols(GetTab(), nStartCol, nEndCol);
        for (auto aObj : aColDrawObjects)
        {
            ScDrawObjData* pData = ScDrawLayer::GetObjData(aObj);
            if (pData)
            {
                if (bHidden)
                    aObj->SetVisible(false);
                else if (!GetDoc().RowHidden(pData->maStart.Row(), pData->maStart.Tab()))
                {
                    // Only change visibility if object is not hidden by a hidden row
                    aObj->SetVisible(true);
                }
            }
        }
    }

    if (bChanged)
        SetStreamValid(false);
}

void ScTable::updateObjectsForRowsChanged(SCROW nStartRow, SCROW nEndRow, bool bHidden,
                                          bool bChanged)
{
    // Cell anchored objects might change visibility
    ScDrawLayer* pDrawLayer = GetDoc().GetDrawLayer();
    if (pDrawLayer)
    {
        std::vector<SdrObject*> aRowDrawObjects;
        aRowDrawObjects = pDrawLayer->GetObjectsAnchoredToRows(GetTab(), nStartRow, nEndRow);
        for (auto aObj : aRowDrawObjects)
        {
            ScDrawObjData* pData = ScDrawLayer::GetObjData(aObj);
            if (pData)
            {
                if (bHidden)
                    aObj->SetVisible(false);
                else if (!GetDoc().ColHidden(pData->maStart.Col(), pData->maStart.Tab()))
                {
                    // Only change visibility if object is not hidden by a hidden col
                    aObj->SetVisible(true);
                }
            }
        }
    }

    if (bChanged)
    {
        SetStreamValid(false);

        { // Scoped bulk broadcast.
            // Only subtotal formula cells will accept the notification of
            // SfxHintId::ScHiddenRowsChanged, leaving the bulk will track
            // those and broadcast SfxHintId::ScDataChanged to notify all
            // dependents.
            ScBulkBroadcast aBulkBroadcast(GetDoc().GetBASM(), SfxHintId::ScDataChanged);
            for (SCCOL i = 0; i < aCol.size(); i++)
            {
                aCol[i].BroadcastRows(nStartRow, nEndRow, SfxHintId::ScHiddenRowsChanged);
            }
        }
    }
}

bool ScTable::SetRowHidden(SCROW nStartRow, SCROW nEndRow, bool bHidden)
{
    return maFilterData.setRowHidden(nStartRow, nEndRow, bHidden);
}

void ScTable::SetColHidden(SCCOL nStartCol, SCCOL nEndCol, bool bHidden)
{
    maFilterData.setColHidden(nStartCol, nEndCol, bHidden);
}

void ScTable::CopyColHidden(const ScTable& rTable, SCCOL nStartCol, SCCOL nEndCol)
{
    return maFilterData.copyColHidden(rTable.getFilterData(), nStartCol, nEndCol);
}

void ScTable::CopyRowHidden(const ScTable& rTable, SCROW nStartRow, SCROW nEndRow)
{
    return maFilterData.copyRowHidden(rTable.getFilterData(), nStartRow, nEndRow);
}

void ScTable::CopyRowHeight(const ScTable& rSrcTable, SCROW nStartRow, SCROW nEndRow,
                            SCROW nSrcOffset)
{
    SCROW nRow = nStartRow;
    ScFlatUInt16RowSegments::RangeData aSrcData;
    while (nRow <= nEndRow)
    {
        if (!rSrcTable.mpRowHeights->getRangeData(nRow + nSrcOffset, aSrcData))
            // Something is wrong !
            return;

        SCROW nLastRow = aSrcData.mnRow2 - nSrcOffset;
        if (nLastRow > nEndRow)
            nLastRow = nEndRow;

        mpRowHeights->setValue(nRow, nLastRow, aSrcData.mnValue);
        nRow = nLastRow + 1;
    }
}

SCROW ScTable::FirstVisibleRow(SCROW nStartRow, SCROW nEndRow) const
{
    return maFilterData.firstVisibleRow(nStartRow, nEndRow);
}

SCROW ScTable::LastVisibleRow(SCROW nStartRow, SCROW nEndRow) const
{
    return maFilterData.lastVisibleRow(nStartRow, nEndRow);
}

SCROW ScTable::CountVisibleRows(SCROW nStartRow, SCROW nEndRow) const
{
    return maFilterData.countVisibleRows(nStartRow, nEndRow);
}

tools::Long ScTable::GetTotalRowHeight(SCROW nStartRow, SCROW nEndRow, bool bHiddenAsZero) const
{
    tools::Long nHeight = 0;
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!maFilterData.mpHiddenRows->getRangeData(nRow, aData))
            break;

        if (aData.mnRow2 > nEndRow)
            aData.mnRow2 = nEndRow;

        if (!(bHiddenAsZero && aData.mbValue))
            // visible row range.
            nHeight += mpRowHeights->getSumValue(nRow, aData.mnRow2);

        nRow = aData.mnRow2 + 1;
    }

    return nHeight;
}

SCCOL ScTable::CountVisibleCols(SCCOL nStartCol, SCCOL nEndCol) const
{
    return maFilterData.countVisibleCols(nStartCol, nEndCol);
}

SCCOLROW ScTable::LastHiddenColRow(SCCOLROW nPos, bool bCol) const
{
    return maFilterData.lastHiddenColRow(nPos, bCol);
}

Color ScTable::GetCellBackgroundColor(ScAddress aPos) const
{
    Color backgroundColor;
    bool bHasConditionalBackgroundColor = false;
    // Check background color from cond. formatting
    const ScPatternAttr* pPattern = GetDoc().GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
    if (pPattern)
    {
        if (!pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
        {
            const SfxItemSet* pCondSet = GetDoc().GetCondResult(aPos.Col(), aPos.Row(), aPos.Tab());
            const SvxBrushItem* pBackgroundColor = &pPattern->GetItem(ATTR_BACKGROUND, pCondSet);
            backgroundColor = pBackgroundColor->GetColor();
            bHasConditionalBackgroundColor = true;
        }
    }

    // Color scale needs a different handling
    ScConditionalFormat* pCondFormat = GetDoc().GetCondFormat(aPos.Col(), aPos.Row(), aPos.Tab());
    if (pCondFormat)
    {
        for (size_t i = 0; i < pCondFormat->size(); i++)
        {
            auto aEntry = pCondFormat->GetEntry(i);
            if (aEntry->GetType() == ScFormatEntry::Type::Colorscale)
            {
                const ScColorScaleFormat* pColFormat
                    = static_cast<const ScColorScaleFormat*>(aEntry);
                std::optional<Color> oColor = pColFormat->GetColor(aPos);
                if (oColor)
                {
                    backgroundColor = oColor.value();
                    bHasConditionalBackgroundColor = true;
                }
            }
        }
    }
    return bHasConditionalBackgroundColor ? backgroundColor
                                          : GetDoc().GetAttr(aPos, ATTR_BACKGROUND)->GetColor();
}

Color ScTable::GetCellTextColor(ScAddress aPos) const
{
    // Check text & background color from cond. formatting
    const ScPatternAttr* pPattern = GetDoc().GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
    if (pPattern)
    {
        if (!pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
        {
            const SfxItemSet* pCondSet = GetDoc().GetCondResult(aPos.Col(), aPos.Row(), aPos.Tab());
            const SvxColorItem* pColor = &pPattern->GetItem(ATTR_FONT_COLOR, pCondSet);
            return pColor->GetValue();
        }

        if (pPattern->GetItem(ATTR_VALUE_FORMAT).GetValue())
        {
            const SfxUInt32Item pItem = pPattern->GetItem(ATTR_VALUE_FORMAT);
            auto& rDoc = const_cast<ScDocument&>(GetDoc());
            const Color* pColor;
            ScRefCellValue aCell(rDoc, aPos);
            ScCellFormat::GetString(rDoc, aPos, pItem.GetValue(), &pColor, nullptr, false, false);
            if (pColor)
                return *pColor;
        }
    }

    const SvxColorItem* pColor = GetDoc().GetAttr(aPos, ATTR_FONT_COLOR);
    return pColor->GetValue();
}

bool ScTable::IsManualRowHeight(SCROW nRow) const
{
    return bool(pRowFlags->GetValue(nRow) & CRFlags::ManualSize);
}

namespace
{
void lcl_syncFlags(const ScDocument* pDocument, ScFlatBoolColSegments& rColSegments,
                   const ScFlatBoolRowSegments& rRowSegments,
                   ScBitMaskCompressedArray<SCCOL, CRFlags>* pColFlags,
                   ScBitMaskCompressedArray<SCROW, CRFlags>* pRowFlags, const CRFlags nFlagMask)
{
    CRFlags nFlagMaskComplement = ~nFlagMask;

    pRowFlags->AndValue(0, pDocument->MaxRow(), nFlagMaskComplement);
    pColFlags->AndValue(0, pDocument->MaxCol() + 1, nFlagMaskComplement);

    {
        // row hidden flags.

        SCROW nRow = 0;
        ScFlatBoolRowSegments::RangeData aData;
        while (nRow <= pDocument->MaxRow())
        {
            if (!rRowSegments.getRangeData(nRow, aData))
                break;

            if (aData.mbValue)
                pRowFlags->OrValue(nRow, aData.mnRow2, nFlagMask);

            nRow = aData.mnRow2 + 1;
        }
    }

    {
        // column hidden flags.

        SCCOL nCol = 0;
        ScFlatBoolColSegments::RangeData aData;
        while (nCol <= pDocument->MaxCol())
        {
            if (!rColSegments.getRangeData(nCol, aData))
                break;

            if (aData.mbValue)
                pColFlags->OrValue(nCol, aData.mnCol2, nFlagMask);

            nCol = aData.mnCol2 + 1;
        }
    }
}
}

void ScTable::SyncColRowFlags()
{
    CRFlags nManualBreakComplement = ~CRFlags::ManualBreak;

    // Manual breaks.
    pRowFlags->AndValue(0, rDocument.MaxRow(), nManualBreakComplement);
    mpColFlags->AndValue(0, rDocument.MaxCol() + 1, nManualBreakComplement);

    for (const auto& rBreakPos : maRowManualBreaks)
        pRowFlags->OrValue(rBreakPos, CRFlags::ManualBreak);

    for (const auto& rBreakPos : maColManualBreaks)
        mpColFlags->OrValue(rBreakPos, CRFlags::ManualBreak);

    // Hidden flags.
    lcl_syncFlags(&rDocument, *maFilterData.mpHiddenCols, *maFilterData.mpHiddenRows,
                  mpColFlags.get(), pRowFlags.get(), CRFlags::Hidden);
    lcl_syncFlags(&rDocument, *maFilterData.mpFilteredCols, *maFilterData.mpFilteredRows,
                  mpColFlags.get(), pRowFlags.get(), CRFlags::Filtered);
}

void ScTable::SetPageSize(const Size& rSize)
{
    if (!rSize.IsEmpty())
    {
        if (aPageSizeTwips != rSize)
            InvalidatePageBreaks();

        bPageSizeValid = true;
        aPageSizeTwips = rSize;
    }
    else
        bPageSizeValid = false;
}

bool ScTable::IsProtected() const { return pTabProtection && pTabProtection->isProtected(); }

void ScTable::SetProtection(const ScTableProtection* pProtect)
{
    if (pProtect)
        pTabProtection.reset(new ScTableProtection(*pProtect));
    else
        pTabProtection.reset();

    SetStreamValid(false);
}

const ScTableProtection* ScTable::GetProtection() const { return pTabProtection.get(); }

Size ScTable::GetPageSize() const
{
    if (bPageSizeValid)
        return aPageSizeTwips;
    else
        return Size(); // blank
}

void ScTable::SetRepeatArea(SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow)
{
    // #i117952# page break calculation uses these values (set from ScPrintFunc), not pRepeatColRange/pRepeatRowRange
    if (nStartCol != nRepeatStartX || nEndCol != nRepeatEndX || nStartRow != nRepeatStartY
        || nEndRow != nRepeatEndY)
        InvalidatePageBreaks();

    nRepeatStartX = nStartCol;
    nRepeatEndX = nEndCol;
    nRepeatStartY = nStartRow;
    nRepeatEndY = nEndRow;
}

void ScTable::StartListening(const ScAddress& rAddress, SvtListener* pListener)
{
    if (!ValidCol(rAddress.Col()))
        return;

    CreateColumnIfNotExists(rAddress.Col()).StartListening(*pListener, rAddress.Row());
}

void ScTable::EndListening(const ScAddress& rAddress, SvtListener* pListener)
{
    if (!ValidCol(rAddress.Col()))
        return;

    if (rAddress.Col() < aCol.size())
        aCol[rAddress.Col()].EndListening(*pListener, rAddress.Row());
}

void ScTable::StartListening(sc::StartListeningContext& rCxt, const ScAddress& rAddress,
                             SvtListener& rListener)
{
    if (!ValidCol(rAddress.Col()))
        return;

    CreateColumnIfNotExists(rAddress.Col()).StartListening(rCxt, rAddress, rListener);
}

void ScTable::EndListening(sc::EndListeningContext& rCxt, const ScAddress& rAddress,
                           SvtListener& rListener)
{
    if (!ValidCol(rAddress.Col()))
        return;

    if (rAddress.Col() < aCol.size())
        aCol[rAddress.Col()].EndListening(rCxt, rAddress, rListener);
}

void ScTable::SetPageStyle(const OUString& rName)
{
    if (aPageStyle == rName)
        return;

    OUString aStrNew = rName;
    SfxStyleSheetBasePool* pStylePool = rDocument.GetStyleSheetPool();
    SfxStyleSheetBase* pNewStyle = pStylePool->Find(aStrNew, SfxStyleFamily::Page);

    if (!pNewStyle)
    {
        aStrNew = ScResId(STR_STYLENAME_STANDARD);
        pNewStyle = pStylePool->Find(aStrNew, SfxStyleFamily::Page);
    }

    if (aPageStyle == aStrNew)
        return;

    SfxStyleSheetBase* pOldStyle = pStylePool->Find(aPageStyle, SfxStyleFamily::Page);
    if (pOldStyle && pNewStyle)
    {
        SfxItemSet& rOldSet = pOldStyle->GetItemSet();
        SfxItemSet& rNewSet = pNewStyle->GetItemSet();
        auto getScaleValue = [](const SfxItemSet& rSet, sal_uInt16 nId) {
            return static_cast<const SfxUInt16Item&>(rSet.Get(nId)).GetValue();
        };

        const sal_uInt16 nOldScale = getScaleValue(rOldSet, ATTR_PAGE_SCALE);
        const sal_uInt16 nOldScaleToPages = getScaleValue(rOldSet, ATTR_PAGE_SCALETOPAGES);
        const sal_uInt16 nNewScale = getScaleValue(rNewSet, ATTR_PAGE_SCALE);
        const sal_uInt16 nNewScaleToPages = getScaleValue(rNewSet, ATTR_PAGE_SCALETOPAGES);

        if ((nOldScale != nNewScale) || (nOldScaleToPages != nNewScaleToPages))
            InvalidateTextWidth(nullptr, nullptr, false, false);
    }

    if (pNewStyle) // also without the old one (for UpdateStdNames)
        aPageStyle = aStrNew;

    SetStreamValid(false);
}

void ScTable::PageStyleModified(const OUString& rNewName)
{
    aPageStyle = rNewName;
    InvalidateTextWidth(nullptr, nullptr, false, false); // don't know what was in the style before
}

void ScTable::InvalidateTextWidth(const ScAddress* pAdrFrom, const ScAddress* pAdrTo,
                                  bool bNumFormatChanged, bool bBroadcast)
{
    if (pAdrFrom && !pAdrTo)
    {
        // Special case: only process the "from" cell.
        SCCOL nCol = pAdrFrom->Col();
        SCROW nRow = pAdrFrom->Row();
        if (nCol >= aCol.size())
            return;
        ScColumn& rCol = aCol[nCol];
        ScRefCellValue aCell = rCol.GetCellValue(nRow);
        if (aCell.isEmpty())
            return;

        rCol.SetTextWidth(nRow, TEXTWIDTH_DIRTY);

        if (bNumFormatChanged)
            rCol.SetScriptType(nRow, SvtScriptType::UNKNOWN);

        if (bBroadcast)
        { // Only with CalcAsShown
            switch (aCell.getType())
            {
                case CELLTYPE_VALUE:
                    rCol.Broadcast(nRow);
                    break;
                case CELLTYPE_FORMULA:
                    aCell.getFormula()->SetDirty();
                    break;
                default:
                {
                    // added to avoid warnings
                }
            }
        }

        return;
    }

    const SCCOL nCol1 = pAdrFrom ? pAdrFrom->Col() : 0;
    const SCROW nRow1 = pAdrFrom ? pAdrFrom->Row() : 0;
    const SCCOL nCol2 = pAdrTo ? pAdrTo->Col() : aCol.size() - 1;
    const SCROW nRow2 = pAdrTo ? pAdrTo->Row() : rDocument.MaxRow();

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        ScColumnTextWidthIterator aIter(GetDoc(), aCol[nCol], nRow1, nRow2);
        sc::ColumnBlockPosition blockPos; // cache mdds position
        InitColumnBlockPosition(blockPos, nCol);

        for (; aIter.hasCell(); aIter.next())
        {
            SCROW nRow = aIter.getPos();
            aIter.setValue(TEXTWIDTH_DIRTY);
            ScRefCellValue aCell = aCol[nCol].GetCellValue(blockPos, nRow);
            if (aCell.isEmpty())
                continue;

            if (bNumFormatChanged)
                aCol[nCol].SetScriptType(nRow, SvtScriptType::UNKNOWN);

            if (bBroadcast)
            { // Only with CalcAsShown
                switch (aCell.getType())
                {
                    case CELLTYPE_VALUE:
                        aCol[nCol].Broadcast(nRow);
                        break;
                    case CELLTYPE_FORMULA:
                        aCell.getFormula()->SetDirty();
                        break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
