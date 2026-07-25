/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
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

#include <pivot/StyleOutput.hxx>

#include <scitems.hxx>

#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/wghtitem.hxx>
#include <osl/diagnose.h>

#include <attrib.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>

#include <algorithm>

namespace
{
constexpr sal_uInt16 SC_DP_FRAME_INNER_BOLD = 20;
constexpr sal_uInt16 SC_DP_FRAME_OUTER_BOLD = 40;

constexpr Color SC_DP_FRAME_COLOR(0, 0, 0); //( 0x20, 0x40, 0x68 )

void lcl_SetStyleById(ScDocument& rDoc, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
                      SCROW nRow2, TranslateId pStrId)
{
    if (nCol1 > nCol2 || nRow1 > nRow2)
    {
        OSL_FAIL("SetStyleById: invalid range");
        return;
    }

    OUString aStyleName = ScResId(pStrId);
    ScStyleSheetPool* pStlPool = rDoc.GetStyleSheetPool();
    ScStyleSheet* pStyle
        = static_cast<ScStyleSheet*>(pStlPool->Find(aStyleName, SfxStyleFamily::Para));
    if (!pStyle)
    {
        //  create new style (was in ScPivot::SetStyle)

        pStyle = static_cast<ScStyleSheet*>(
            &pStlPool->Make(aStyleName, SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined));
        pStyle->SetParent(ScResId(STR_STYLENAME_STANDARD));
        SfxItemSet& rSet = pStyle->GetItemSet();
        if (pStrId == STR_PIVOT_STYLENAME_RESULT || pStrId == STR_PIVOT_STYLENAME_TITLE)
        {
            rSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
            rSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_CJK_FONT_WEIGHT));
            rSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_CTL_FONT_WEIGHT));
        }
        if (pStrId == STR_PIVOT_STYLENAME_CATEGORY || pStrId == STR_PIVOT_STYLENAME_TITLE)
            rSet.Put(SvxHorJustifyItem(SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY));
    }

    rDoc.ApplyStyleAreaTab(nCol1, nRow1, nCol2, nRow2, nTab, *pStyle);
}

void lcl_SetFrame(ScDocument& rDoc, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                  sal_uInt16 nWidth)
{
    ::editeng::SvxBorderLine aLine(nullptr, nWidth, SvxBorderLineStyle::SOLID);
    SvxBoxItem aBox(ATTR_BORDER);
    aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
    aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);
    SvxBoxInfoItem aBoxInfo(ATTR_BORDER_INNER);
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI, false);
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::VERT, false);
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::DISTANCE, false);

    rDoc.ApplyFrameAreaTab(ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab), aBox, aBoxInfo);
}

/** Draws the block frames of the table: each block gets a box around it, with a bold line on the
 *  outer edges of the table. The rows and columns where subtotal and member blocks begin are
 *  collected with AddRow and AddCol, and OutputDataArea then frames the blocks between them. */
class BlockFrameOutput
{
    ScDocument& mrDocument;
    SCTAB mnTab;
    std::vector<bool> mbNeedLineCols;
    std::vector<SCCOL> mnCols;

    std::vector<bool> mbNeedLineRows;
    std::vector<SCROW> mnRows;

    SCCOL mnTabStartCol;
    SCROW mnTabStartRow;

    SCCOL mnDataStartCol;
    SCROW mnDataStartRow;
    SCCOL mnTabEndCol;
    SCROW mnTabEndRow;

public:
    BlockFrameOutput(ScDocument& rDocument, sc::pivot::Geometry const& rGeometry)
        : mrDocument(rDocument)
        , mnTab(rGeometry.mnTab)
        , mnTabStartCol(rGeometry.mnTabStartCol)
        , mnTabStartRow(rGeometry.mnTabStartRow)
        , mnDataStartCol(rGeometry.mnDataStartCol)
        , mnDataStartRow(rGeometry.mnDataStartRow)
        , mnTabEndCol(rGeometry.mnTabEndCol)
        , mnTabEndRow(rGeometry.mnTabEndRow)
    {
        mbNeedLineCols.resize(mnTabEndCol - mnDataStartCol + 1, false);
        mbNeedLineRows.resize(mnTabEndRow - mnDataStartRow + 1, false);
    }

    void AddRow(SCROW nRow)
    {
        if (!mbNeedLineRows[nRow - mnDataStartRow])
        {
            mbNeedLineRows[nRow - mnDataStartRow] = true;
            mnRows.push_back(nRow);
        }
    }

    void AddCol(SCCOL nCol)
    {
        if (!mbNeedLineCols[nCol - mnDataStartCol])
        {
            mbNeedLineCols[nCol - mnDataStartCol] = true;
            mnCols.push_back(nCol);
        }
    }

    void OutputBlockFrame(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                          bool bHori = false)
    {
        Color color = SC_DP_FRAME_COLOR;
        ::editeng::SvxBorderLine aLine(&color, SC_DP_FRAME_INNER_BOLD);
        ::editeng::SvxBorderLine aOutLine(&color, SC_DP_FRAME_OUTER_BOLD);

        SvxBoxItem aBox(ATTR_BORDER);

        if (nStartCol == mnTabStartCol)
            aBox.SetLine(&aOutLine, SvxBoxItemLine::LEFT);
        else
            aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);

        if (nStartRow == mnTabStartRow)
            aBox.SetLine(&aOutLine, SvxBoxItemLine::TOP);
        else
            aBox.SetLine(&aLine, SvxBoxItemLine::TOP);

        if (nEndCol == mnTabEndCol) //bottom row
            aBox.SetLine(&aOutLine, SvxBoxItemLine::RIGHT);
        else
            aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);

        if (nEndRow == mnTabEndRow) //bottom
            aBox.SetLine(&aOutLine, SvxBoxItemLine::BOTTOM);
        else
            aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);

        SvxBoxInfoItem aBoxInfo(ATTR_BORDER_INNER);
        aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::VERT, false);
        if (bHori)
        {
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI);
            aBoxInfo.SetLine(&aLine, SvxBoxInfoItemLine::HORI);
        }
        else
            aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI, false);

        aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::DISTANCE, false);

        mrDocument.ApplyFrameAreaTab(ScRange(nStartCol, nStartRow, mnTab, nEndCol, nEndRow, mnTab),
                                     aBox, aBoxInfo);
    }

    void OutputDataArea()
    {
        AddRow(mnDataStartRow);
        AddCol(mnDataStartCol);

        mnCols.push_back(mnTabEndCol + 1); //set last row bottom
        mnRows.push_back(mnTabEndRow + 1); //set last col bottom

        bool bAllRows = ((mnTabEndRow - mnDataStartRow + 2) == static_cast<SCROW>(mnRows.size()));

        std::sort(mnCols.begin(), mnCols.end());
        std::sort(mnRows.begin(), mnRows.end());

        for (SCCOL nCol = 0; nCol < static_cast<SCCOL>(mnCols.size()) - 1; nCol++)
        {
            if (!bAllRows)
            {
                if (nCol < static_cast<SCCOL>(mnCols.size()) - 2)
                {
                    for (SCROW i = nCol % 2; i < static_cast<SCROW>(mnRows.size()) - 2; i += 2)
                        OutputBlockFrame(mnCols[nCol], mnRows[i], mnCols[nCol + 1] - 1,
                                         mnRows[i + 1] - 1);
                    if (mnRows.size() >= 2)
                        OutputBlockFrame(mnCols[nCol], mnRows[mnRows.size() - 2],
                                         mnCols[nCol + 1] - 1, mnRows[mnRows.size() - 1] - 1);
                }
                else
                {
                    for (SCROW i = 0; i < static_cast<SCROW>(mnRows.size()) - 1; i++)
                        OutputBlockFrame(mnCols[nCol], mnRows[i], mnCols[nCol + 1] - 1,
                                         mnRows[i + 1] - 1);
                }
            }
            else
                OutputBlockFrame(mnCols[nCol], mnRows.front(), mnCols[nCol + 1] - 1,
                                 mnRows.back() - 1, bAllRows);
        }
        //out put rows area outer framer
        if (mnTabStartCol != mnDataStartCol)
        {
            if (mnTabStartRow != mnDataStartRow)
                OutputBlockFrame(mnTabStartCol, mnTabStartRow, mnDataStartCol - 1,
                                 mnDataStartRow - 1);
            OutputBlockFrame(mnTabStartCol, mnDataStartRow, mnDataStartCol - 1, mnTabEndRow);
        }
        //out put cols area outer framer
        OutputBlockFrame(mnDataStartCol, mnTabStartRow, mnTabEndCol, mnDataStartRow - 1);
    }
};

} // end anonymous namespace

namespace sc::pivot
{
void StyleOutput::applyAreaStyles()
{
    SCTAB nTab = maGeometry.mnTab;

    if (maGeometry.mnDataStartRow > maGeometry.mnTabStartRow)
    {
        lcl_SetStyleById(mrDocument, nTab, maGeometry.mnTabStartCol, maGeometry.mnTabStartRow,
                         maGeometry.mnTabEndCol, maGeometry.mnDataStartRow - 1,
                         STR_PIVOT_STYLENAME_TOP);
    }
    lcl_SetStyleById(mrDocument, nTab, maGeometry.mnDataStartCol, maGeometry.mnDataStartRow,
                     maGeometry.mnTabEndCol, maGeometry.mnTabEndRow, STR_PIVOT_STYLENAME_INNER);
}

void StyleOutput::addFieldCell(SCCOL nCol, SCROW nRow, bool bFrame)
{
    maFieldCells.push_back(FieldCell{ nCol, nRow, bFrame });

    lcl_SetStyleById(mrDocument, maGeometry.mnTab, nCol, nRow, nCol, nRow,
                     STR_PIVOT_STYLENAME_FIELDNAME);
}

void StyleOutput::addColumnMemberSpan(size_t nField, SCCOL nStartCol, SCCOL nEndCol, SCROW nRow)
{
    maColumnMemberSpans.push_back(ColumnMemberSpan{ nField, nStartCol, nEndCol, nRow });

    if (nField + 1 < maGeometry.mnColumnFieldCount)
    {
        lcl_SetStyleById(mrDocument, maGeometry.mnTab, nStartCol, nRow, nEndCol,
                         maGeometry.mnDataStartRow - 1, STR_PIVOT_STYLENAME_CATEGORY);
    }
    else
    {
        lcl_SetStyleById(mrDocument, maGeometry.mnTab, nStartCol, nRow, nStartCol,
                         maGeometry.mnDataStartRow - 1, STR_PIVOT_STYLENAME_CATEGORY);
    }
}

void StyleOutput::addRowMemberSpan(size_t nField, SCCOL nCol, SCROW nStartRow, SCROW nEndRow)
{
    maRowMemberSpans.push_back(RowMemberSpan{ nField, nCol, nStartRow, nEndRow });

    if (nField + 1 < maGeometry.mnRowFieldCount)
    {
        lcl_SetStyleById(mrDocument, maGeometry.mnTab, nCol, nStartRow,
                         maGeometry.mnDataStartCol - 1, nEndRow, STR_PIVOT_STYLENAME_CATEGORY);
    }
    else
    {
        lcl_SetStyleById(mrDocument, maGeometry.mnTab, nCol, nStartRow,
                         maGeometry.mnDataStartCol - 1, nStartRow, STR_PIVOT_STYLENAME_CATEGORY);
    }
}

void StyleOutput::addSubtotalColumn(SCCOL nCol, SCROW nStartRow, bool bGrandTotal)
{
    maSubtotalColumns.push_back(SubtotalColumn{ nCol, nStartRow, bGrandTotal });

    lcl_SetStyleById(mrDocument, maGeometry.mnTab, nCol, nStartRow, nCol,
                     maGeometry.mnDataStartRow - 1, STR_PIVOT_STYLENAME_TITLE);
    lcl_SetStyleById(mrDocument, maGeometry.mnTab, nCol, maGeometry.mnDataStartRow, nCol,
                     maGeometry.mnTabEndRow, STR_PIVOT_STYLENAME_RESULT);
}

void StyleOutput::addSubtotalRow(SCROW nRow, SCCOL nStartCol, bool bGrandTotal)
{
    maSubtotalRows.push_back(SubtotalRow{ nRow, nStartCol, bGrandTotal });

    lcl_SetStyleById(mrDocument, maGeometry.mnTab, nStartCol, nRow, maGeometry.mnDataStartCol - 1,
                     nRow, STR_PIVOT_STYLENAME_TITLE);
    lcl_SetStyleById(mrDocument, maGeometry.mnTab, maGeometry.mnDataStartCol, nRow,
                     maGeometry.mnTabEndCol, nRow, STR_PIVOT_STYLENAME_RESULT);
}

void StyleOutput::clear()
{
    maGeometry = Geometry();
    maFieldCells.clear();
    maPageFieldValueCells.clear();
    maColumnMemberSpans.clear();
    maRowMemberSpans.clear();
    maSubtotalColumns.clear();
    maSubtotalRows.clear();
    maIndentCells.clear();
    maExpanderCells.clear();
}

void StyleOutput::apply()
{
    SCTAB nTab = maGeometry.mnTab;

    // Field caption cells and page field value cells
    for (FieldCell const& rCell : maFieldCells)
    {
        if (rCell.bFrame)
            lcl_SetFrame(mrDocument, nTab, rCell.nCol, rCell.nRow, rCell.nCol, rCell.nRow,
                         SC_DP_FRAME_INNER_BOLD);
    }
    for (PageFieldValueCell const& rCell : maPageFieldValueCells)
    {
        lcl_SetFrame(mrDocument, nTab, rCell.nCol, rCell.nRow, rCell.nCol, rCell.nRow,
                     SC_DP_FRAME_INNER_BOLD);
    }

    BlockFrameOutput aBlockFrames(mrDocument, maGeometry);

    // Column field members
    for (ColumnMemberSpan const& rSpan : maColumnMemberSpans)
    {
        if (rSpan.nField + 1 < maGeometry.mnColumnFieldCount)
        {
            if (rSpan.nField + 2 == maGeometry.mnColumnFieldCount)
            {
                aBlockFrames.AddCol(rSpan.nStartCol);
                if (rSpan.nStartCol + 1 == rSpan.nEndCol)
                    aBlockFrames.OutputBlockFrame(rSpan.nStartCol, rSpan.nRow, rSpan.nEndCol,
                                                  rSpan.nRow + 1, true);
            }
            else
            {
                aBlockFrames.OutputBlockFrame(rSpan.nStartCol, rSpan.nRow, rSpan.nEndCol,
                                              rSpan.nRow);
            }
        }
    }

    // A single column field shares its header row with the data description, so the frame around
    // the description row spans the whole width of the data columns.
    if (maGeometry.mnColumnFieldCount == 1
        && maGeometry.mnMemberStartRow > maGeometry.mnTabStartRow)
    {
        aBlockFrames.OutputBlockFrame(maGeometry.mnDataStartCol, maGeometry.mnTabStartRow,
                                      maGeometry.mnTabEndCol, maGeometry.mnMemberStartRow - 1);
    }

    // Subtotal and grand total columns
    for (SubtotalColumn const& rColumn : maSubtotalColumns)
    {
        aBlockFrames.AddCol(rColumn.nCol);
        aBlockFrames.OutputBlockFrame(rColumn.nCol, rColumn.nStartRow, rColumn.nCol,
                                      maGeometry.mnDataStartRow - 1);
    }

    // Row field members. The first member that begins in a row also gets a frame around the whole
    // width of the table for its row span.
    std::vector<bool> aBorderSet(maGeometry.mnTabEndRow - maGeometry.mnDataStartRow + 1, false);
    for (RowMemberSpan const& rSpan : maRowMemberSpans)
    {
        if (rSpan.nField + 1 < maGeometry.mnRowFieldCount)
        {
            aBlockFrames.AddRow(rSpan.nStartRow);
            if (!aBorderSet[rSpan.nStartRow - maGeometry.mnDataStartRow])
            {
                aBlockFrames.OutputBlockFrame(rSpan.nCol, rSpan.nStartRow, maGeometry.mnTabEndCol,
                                              rSpan.nEndRow);
                aBorderSet[rSpan.nStartRow - maGeometry.mnDataStartRow] = true;
            }
            aBlockFrames.OutputBlockFrame(rSpan.nCol, rSpan.nStartRow, rSpan.nCol, rSpan.nEndRow);

            if (rSpan.nField == maGeometry.mnRowFieldCount - 2)
                aBlockFrames.OutputBlockFrame(rSpan.nCol + 1, rSpan.nStartRow, rSpan.nCol + 1,
                                              rSpan.nEndRow);
        }
    }

    // Subtotal and grand total rows
    for (SubtotalRow const& rRow : maSubtotalRows)
    {
        aBlockFrames.AddRow(rRow.nRow);
        aBlockFrames.OutputBlockFrame(rRow.nStartCol, rRow.nRow, maGeometry.mnDataStartCol - 1,
                                      rRow.nRow);
    }

    // Member indents and expand/collapse buttons
    for (Indent const& rIndent : maIndentCells)
        mrDocument.ApplyAttr(rIndent.nCol, rIndent.nRow, nTab, ScIndentItem(rIndent.nIndent));

    for (Expander const& rExpander : maExpanderCells)
    {
        mrDocument.ApplyFlagsTab(rExpander.nCol, rExpander.nRow, rExpander.nCol, rExpander.nRow,
                                 nTab, rExpander.nFlags);
    }

    aBlockFrames.OutputDataArea();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
