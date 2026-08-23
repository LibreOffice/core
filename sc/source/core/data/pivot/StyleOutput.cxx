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

#include <attrib.hxx>
#include <document.hxx>
#include <patattr.hxx>

#include <algorithm>

namespace
{
constexpr sal_uInt16 SC_DP_FRAME_WIDTH = SvxBorderLineWidth::Thin;

constexpr Color SC_DP_FRAME_COLOR(0, 0, 0);

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

void lcl_SetLeftAligned(ScDocument& rDoc, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
                        SCROW nRow2)
{
    ScPatternAttr aPattern(rDoc.getCellAttributeHelper());
    aPattern.ItemSetPut(SvxHorJustifyItem(SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY));
    rDoc.ApplyPatternAreaTab(nCol1, nRow1, nCol2, nRow2, nTab, aPattern);
}

void lcl_SetBold(ScDocument& rDoc, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    ScPatternAttr aPattern(rDoc.getCellAttributeHelper());
    aPattern.ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
    aPattern.ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_CJK_FONT_WEIGHT));
    aPattern.ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_CTL_FONT_WEIGHT));
    rDoc.ApplyPatternAreaTab(nCol1, nRow1, nCol2, nRow2, nTab, aPattern);
}

/** Draws the block frames of the table: each block gets a thin box around it. The rows and
 *  columns where subtotal and member blocks begin are collected with AddRow and AddCol, and
 *  OutputDataArea then frames the blocks between them. */
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
        ::editeng::SvxBorderLine aLine(&color, SC_DP_FRAME_WIDTH);

        SvxBoxItem aBox(ATTR_BORDER);
        aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
        aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
        aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
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
        //output rows area outer frame
        if (mnTabStartCol != mnDataStartCol)
        {
            if (mnTabStartRow != mnDataStartRow)
                OutputBlockFrame(mnTabStartCol, mnTabStartRow, mnDataStartCol - 1,
                                 mnDataStartRow - 1);
            OutputBlockFrame(mnTabStartCol, mnDataStartRow, mnDataStartCol - 1, mnTabEndRow);
        }
        //output cols area outer frame
        OutputBlockFrame(mnDataStartCol, mnTabStartRow, mnTabEndCol, mnDataStartRow - 1);
    }
};

} // end anonymous namespace

namespace sc::pivot
{
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
                         SC_DP_FRAME_WIDTH);
    }
    for (PageFieldValueCell const& rCell : maPageFieldValueCells)
    {
        lcl_SetFrame(mrDocument, nTab, rCell.nCol, rCell.nRow, rCell.nCol, rCell.nRow,
                     SC_DP_FRAME_WIDTH);
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
            lcl_SetLeftAligned(mrDocument, nTab, rSpan.nStartCol, rSpan.nRow, rSpan.nEndCol,
                               maGeometry.mnDataStartRow - 1);
        }
        else
        {
            lcl_SetLeftAligned(mrDocument, nTab, rSpan.nStartCol, rSpan.nRow, rSpan.nStartCol,
                               maGeometry.mnDataStartRow - 1);
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

    // Subtotal and grand total columns. The label cells in the header area are bold and aligned
    // to the left, the result cells below them are bold.
    for (SubtotalColumn const& rColumn : maSubtotalColumns)
    {
        aBlockFrames.AddCol(rColumn.nCol);
        aBlockFrames.OutputBlockFrame(rColumn.nCol, rColumn.nStartRow, rColumn.nCol,
                                      maGeometry.mnDataStartRow - 1);
        lcl_SetBold(mrDocument, nTab, rColumn.nCol, rColumn.nStartRow, rColumn.nCol,
                    maGeometry.mnDataStartRow - 1);
        lcl_SetLeftAligned(mrDocument, nTab, rColumn.nCol, rColumn.nStartRow, rColumn.nCol,
                           maGeometry.mnDataStartRow - 1);
        lcl_SetBold(mrDocument, nTab, rColumn.nCol, maGeometry.mnDataStartRow, rColumn.nCol,
                    maGeometry.mnTabEndRow);
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

            lcl_SetLeftAligned(mrDocument, nTab, rSpan.nCol, rSpan.nStartRow,
                               maGeometry.mnDataStartCol - 1, rSpan.nEndRow);
        }
        else
        {
            lcl_SetLeftAligned(mrDocument, nTab, rSpan.nCol, rSpan.nStartRow,
                               maGeometry.mnDataStartCol - 1, rSpan.nStartRow);
        }
    }

    // Subtotal and grand total rows. The label cells in the header area are bold and aligned to
    // the left, the result cells to the right of them are bold.
    for (SubtotalRow const& rRow : maSubtotalRows)
    {
        aBlockFrames.AddRow(rRow.nRow);
        aBlockFrames.OutputBlockFrame(rRow.nStartCol, rRow.nRow, maGeometry.mnDataStartCol - 1,
                                      rRow.nRow);
        lcl_SetBold(mrDocument, nTab, rRow.nStartCol, rRow.nRow, maGeometry.mnDataStartCol - 1,
                    rRow.nRow);
        lcl_SetLeftAligned(mrDocument, nTab, rRow.nStartCol, rRow.nRow,
                           maGeometry.mnDataStartCol - 1, rRow.nRow);
        lcl_SetBold(mrDocument, nTab, maGeometry.mnDataStartCol, rRow.nRow, maGeometry.mnTabEndCol,
                    rRow.nRow);
    }

    // Member indents and the expand and collapse buttons. The indent shows up because the member
    // cells are aligned to the left, an indent has no effect in a centered cell.
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
