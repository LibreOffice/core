/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

/** Draws a thin box around the given area. With bHorizontalLines the area also gets the same
 *  line between each pair of rows inside it. */
void applyFrame(ScDocument& rDocument, SCTAB nTab, SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol,
                SCROW nEndRow, bool bHorizontalLines)
{
    Color aColor = SC_DP_FRAME_COLOR;
    ::editeng::SvxBorderLine aLine(&aColor, SC_DP_FRAME_WIDTH);

    SvxBoxItem aBox(ATTR_BORDER);
    aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
    aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);

    SvxBoxInfoItem aBoxInfo(ATTR_BORDER_INNER);
    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::VERT, false);
    if (bHorizontalLines)
    {
        aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI);
        aBoxInfo.SetLine(&aLine, SvxBoxInfoItemLine::HORI);
    }
    else
        aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI, false);

    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::DISTANCE, false);

    rDocument.ApplyFrameAreaTab(ScRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab), aBox,
                                aBoxInfo);
}

/** Draws the block frames of the table: each block gets a thin box around it. The rows and
 *  columns where subtotal and member blocks begin are collected with addRow and addColumn, and
 *  outputDataArea then frames the blocks between them. */
class BlockFrameOutput
{
    ScDocument& mrDocument;
    SCTAB mnTab;
    std::vector<bool> maNeedLineCols;
    std::vector<SCCOL> maCols;

    std::vector<bool> maNeedLineRows;
    std::vector<SCROW> maRows;

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
        maNeedLineCols.resize(mnTabEndCol - mnDataStartCol + 1, false);
        maNeedLineRows.resize(mnTabEndRow - mnDataStartRow + 1, false);
    }

    void addRow(SCROW nRow)
    {
        if (!maNeedLineRows[nRow - mnDataStartRow])
        {
            maNeedLineRows[nRow - mnDataStartRow] = true;
            maRows.push_back(nRow);
        }
    }

    void addColumn(SCCOL nCol)
    {
        if (!maNeedLineCols[nCol - mnDataStartCol])
        {
            maNeedLineCols[nCol - mnDataStartCol] = true;
            maCols.push_back(nCol);
        }
    }

    void outputBlockFrame(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                          bool bHorizontalLines = false)
    {
        applyFrame(mrDocument, mnTab, nStartCol, nStartRow, nEndCol, nEndRow, bHorizontalLines);
    }

    void outputDataArea()
    {
        addRow(mnDataStartRow);
        addColumn(mnDataStartCol);

        maCols.push_back(mnTabEndCol + 1); // right edge of the last block
        maRows.push_back(mnTabEndRow + 1); // bottom edge of the last block

        bool bAllRows = ((mnTabEndRow - mnDataStartRow + 2) == static_cast<SCROW>(maRows.size()));

        std::sort(maCols.begin(), maCols.end());
        std::sort(maRows.begin(), maRows.end());

        for (SCCOL nCol = 0; nCol < static_cast<SCCOL>(maCols.size()) - 1; nCol++)
        {
            if (!bAllRows)
            {
                if (nCol < static_cast<SCCOL>(maCols.size()) - 2)
                {
                    for (SCROW i = nCol % 2; i < static_cast<SCROW>(maRows.size()) - 2; i += 2)
                        outputBlockFrame(maCols[nCol], maRows[i], maCols[nCol + 1] - 1,
                                         maRows[i + 1] - 1);
                    if (maRows.size() >= 2)
                        outputBlockFrame(maCols[nCol], maRows[maRows.size() - 2],
                                         maCols[nCol + 1] - 1, maRows[maRows.size() - 1] - 1);
                }
                else
                {
                    for (SCROW i = 0; i < static_cast<SCROW>(maRows.size()) - 1; i++)
                        outputBlockFrame(maCols[nCol], maRows[i], maCols[nCol + 1] - 1,
                                         maRows[i + 1] - 1);
                }
            }
            else
                outputBlockFrame(maCols[nCol], maRows.front(), maCols[nCol + 1] - 1,
                                 maRows.back() - 1, bAllRows);
        }

        // the outer frame around the row header area
        if (mnTabStartCol != mnDataStartCol)
        {
            if (mnTabStartRow != mnDataStartRow)
                outputBlockFrame(mnTabStartCol, mnTabStartRow, mnDataStartCol - 1,
                                 mnDataStartRow - 1);
            outputBlockFrame(mnTabStartCol, mnDataStartRow, mnDataStartCol - 1, mnTabEndRow);
        }

        // the outer frame around the column header area
        outputBlockFrame(mnDataStartCol, mnTabStartRow, mnTabEndCol, mnDataStartRow - 1);
    }
};

} // end anonymous namespace

namespace sc::pivot
{
void StyleOutput::reset(Geometry const& rGeometry)
{
    maGeometry = rGeometry;
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

    // The member cells are aligned to the left, which makes their indent visible. An indent shows
    // only in a left-aligned cell. The subtotal and grand total labels and results are bold.
    ScPatternAttr aLeftAligned(mrDocument.getCellAttributeHelper());
    aLeftAligned.ItemSetPut(SvxHorJustifyItem(SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY));

    ScPatternAttr aBold(mrDocument.getCellAttributeHelper());
    aBold.ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
    aBold.ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_CJK_FONT_WEIGHT));
    aBold.ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_CTL_FONT_WEIGHT));

    ScPatternAttr aBoldAndLeftAligned(aBold);
    aBoldAndLeftAligned.ItemSetPut(SvxHorJustifyItem(SvxCellHorJustify::Left, ATTR_HOR_JUSTIFY));

    // Field caption cells and page field value cells. Only a caption inside the table area gets a
    // frame. A caption above the table keeps the flat look of the filter button.
    for (FieldCell const& rCell : maFieldCells)
    {
        if (rCell.mbInTable)
            applyFrame(mrDocument, nTab, rCell.mnCol, rCell.mnRow, rCell.mnCol, rCell.mnRow, false);
    }
    for (PageFieldValueCell const& rCell : maPageFieldValueCells)
        applyFrame(mrDocument, nTab, rCell.mnCol, rCell.mnRow, rCell.mnCol, rCell.mnRow, false);

    BlockFrameOutput aBlockFrames(mrDocument, maGeometry);

    // Column field members
    for (ColumnMemberSpan const& rSpan : maColumnMemberSpans)
    {
        if (rSpan.mnField + 1 < maGeometry.mnColumnFieldCount)
        {
            if (rSpan.mnField + 2 == maGeometry.mnColumnFieldCount)
            {
                aBlockFrames.addColumn(rSpan.mnStartCol);
                if (rSpan.mnStartCol + 1 == rSpan.mnEndCol)
                    aBlockFrames.outputBlockFrame(rSpan.mnStartCol, rSpan.mnRow, rSpan.mnEndCol,
                                                  rSpan.mnRow + 1, true);
            }
            else
            {
                aBlockFrames.outputBlockFrame(rSpan.mnStartCol, rSpan.mnRow, rSpan.mnEndCol,
                                              rSpan.mnRow);
            }
            mrDocument.ApplyPatternAreaTab(rSpan.mnStartCol, rSpan.mnRow, rSpan.mnEndCol,
                                           maGeometry.mnDataStartRow - 1, nTab, aLeftAligned);
        }
        else
        {
            mrDocument.ApplyPatternAreaTab(rSpan.mnStartCol, rSpan.mnRow, rSpan.mnStartCol,
                                           maGeometry.mnDataStartRow - 1, nTab, aLeftAligned);
        }
    }

    // A single column field shares its header row with the data description, so the frame around
    // the description row spans the whole width of the data columns.
    if (maGeometry.mnColumnFieldCount == 1
        && maGeometry.mnMemberStartRow > maGeometry.mnTabStartRow)
    {
        aBlockFrames.outputBlockFrame(maGeometry.mnDataStartCol, maGeometry.mnTabStartRow,
                                      maGeometry.mnTabEndCol, maGeometry.mnMemberStartRow - 1);
    }

    // Subtotal and grand total columns
    for (SubtotalColumn const& rColumn : maSubtotalColumns)
    {
        aBlockFrames.addColumn(rColumn.mnCol);
        aBlockFrames.outputBlockFrame(rColumn.mnCol, rColumn.mnStartRow, rColumn.mnCol,
                                      maGeometry.mnDataStartRow - 1);
        mrDocument.ApplyPatternAreaTab(rColumn.mnCol, rColumn.mnStartRow, rColumn.mnCol,
                                       maGeometry.mnDataStartRow - 1, nTab, aBoldAndLeftAligned);
        mrDocument.ApplyPatternAreaTab(rColumn.mnCol, maGeometry.mnDataStartRow, rColumn.mnCol,
                                       maGeometry.mnTabEndRow, nTab, aBold);
    }

    // Row field members. The first member that begins in a row also gets a frame around the whole
    // width of the table for its row span.
    std::vector<bool> aBorderSet(maGeometry.mnTabEndRow - maGeometry.mnDataStartRow + 1, false);
    for (RowMemberSpan const& rSpan : maRowMemberSpans)
    {
        if (rSpan.mnField + 1 < maGeometry.mnRowFieldCount)
        {
            aBlockFrames.addRow(rSpan.mnStartRow);
            if (!aBorderSet[rSpan.mnStartRow - maGeometry.mnDataStartRow])
            {
                aBlockFrames.outputBlockFrame(rSpan.mnCol, rSpan.mnStartRow, maGeometry.mnTabEndCol,
                                              rSpan.mnEndRow);
                aBorderSet[rSpan.mnStartRow - maGeometry.mnDataStartRow] = true;
            }
            aBlockFrames.outputBlockFrame(rSpan.mnCol, rSpan.mnStartRow, rSpan.mnCol,
                                          rSpan.mnEndRow);

            if (rSpan.mnField == maGeometry.mnRowFieldCount - 2)
                aBlockFrames.outputBlockFrame(rSpan.mnCol + 1, rSpan.mnStartRow, rSpan.mnCol + 1,
                                              rSpan.mnEndRow);

            mrDocument.ApplyPatternAreaTab(rSpan.mnCol, rSpan.mnStartRow,
                                           maGeometry.mnDataStartCol - 1, rSpan.mnEndRow, nTab,
                                           aLeftAligned);
        }
        else
        {
            mrDocument.ApplyPatternAreaTab(rSpan.mnCol, rSpan.mnStartRow,
                                           maGeometry.mnDataStartCol - 1, rSpan.mnStartRow, nTab,
                                           aLeftAligned);
        }
    }

    // Subtotal and grand total rows
    for (SubtotalRow const& rRow : maSubtotalRows)
    {
        aBlockFrames.addRow(rRow.mnRow);
        aBlockFrames.outputBlockFrame(rRow.mnStartCol, rRow.mnRow, maGeometry.mnDataStartCol - 1,
                                      rRow.mnRow);
        mrDocument.ApplyPatternAreaTab(rRow.mnStartCol, rRow.mnRow, maGeometry.mnDataStartCol - 1,
                                       rRow.mnRow, nTab, aBoldAndLeftAligned);
        mrDocument.ApplyPatternAreaTab(maGeometry.mnDataStartCol, rRow.mnRow,
                                       maGeometry.mnTabEndCol, rRow.mnRow, nTab, aBold);
    }

    // Member indents and the expand and collapse buttons
    for (Indent const& rIndent : maIndentCells)
        mrDocument.ApplyAttr(rIndent.mnCol, rIndent.mnRow, nTab, ScIndentItem(rIndent.mnIndent));

    for (Expander const& rExpander : maExpanderCells)
    {
        mrDocument.ApplyFlagsTab(rExpander.mnCol, rExpander.mnRow, rExpander.mnCol, rExpander.mnRow,
                                 nTab, rExpander.mnFlags);
    }

    aBlockFrames.outputDataArea();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
