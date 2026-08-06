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

#pragma once

#include <address.hxx>
#include <attrib.hxx>
#include <tools/long.hxx>

#include <vector>

class ScDocument;

namespace sc::pivot
{
/** Cell positions that describe the layout of the pivot table output area.
 *
 *  The whole table spans from the tab start to the tab end position. The data (result) cells
 *  start at the data start position; the cells left of the data start column hold the row
 *  field members and the cells above the data start row hold the column field members, which
 *  begin at the member start position. */
struct Geometry
{
    SCTAB mnTab = -1;
    SCCOL mnTabStartCol = -1;
    SCROW mnTabStartRow = -1;
    SCCOL mnMemberStartCol = -1;
    SCROW mnMemberStartRow = -1;
    SCCOL mnDataStartCol = -1;
    SCROW mnDataStartRow = -1;
    SCCOL mnTabEndCol = -1;
    SCROW mnTabEndRow = -1;

    size_t mnColumnFieldCount = 0;
    size_t mnRowFieldCount = 0;
};

/** A cell with a field caption. mbInTable is true for a caption that sits inside the table
 *  area and false for one above it, such as the caption of a page field. */
struct FieldCell
{
    SCCOL mnCol = -1;
    SCROW mnRow = -1;
    bool mbInTable = false;
};

/** A cell holding the selected value of a page field. */
struct PageFieldValueCell
{
    SCCOL mnCol = -1;
    SCROW mnRow = -1;
};

/** One member of a column field. The member spans the columns mnStartCol to mnEndCol in the
 *  header row mnRow. mnField is the position of the field among the column fields. */
struct ColumnMemberSpan
{
    size_t mnField = 0;
    SCCOL mnStartCol = -1;
    SCCOL mnEndCol = -1;
    SCROW mnRow = -1;
};

/** One member of a row field. The member spans the rows mnStartRow to mnEndRow in the header
 *  column mnCol. mnField is the position of the field among the row fields. */
struct RowMemberSpan
{
    size_t mnField = 0;
    SCCOL mnCol = -1;
    SCROW mnStartRow = -1;
    SCROW mnEndRow = -1;
};

/** A column holding subtotal results. Its header cells begin in the row mnStartRow, and
 *  mbGrandTotal is true for the column that holds the grand total. */
struct SubtotalColumn
{
    SCCOL mnCol = -1;
    SCROW mnStartRow = -1;
    bool mbGrandTotal = false;
};

/** A row holding subtotal results. Its header cells begin in the column mnStartCol, and
 *  mbGrandTotal is true for the row that holds the grand total. */
struct SubtotalRow
{
    SCROW mnRow = -1;
    SCCOL mnStartCol = -1;
    bool mbGrandTotal = false;
};

/** A row field member cell whose content is indented by mnIndent twips. */
struct Indent
{
    SCCOL mnCol = -1;
    SCROW mnRow = -1;
    tools::Long mnIndent = 0;
};

/** A row field member cell that shows an expand or a collapse button. */
struct Expander
{
    SCCOL mnCol = -1;
    SCROW mnRow = -1;
    ScMF mnFlags = ScMF::NONE;
};

/** Collects the sections of a pivot table while the header content is written to the sheet, and
 *  then draws the whole visual formatting of the pivot table (block frames, member indents and
 *  the expand and collapse buttons) in one pass over the collected sections. */
class StyleOutput
{
private:
    ScDocument& mrDocument;

    Geometry maGeometry;

    std::vector<FieldCell> maFieldCells;
    std::vector<PageFieldValueCell> maPageFieldValueCells;
    std::vector<ColumnMemberSpan> maColumnMemberSpans;
    std::vector<RowMemberSpan> maRowMemberSpans;
    std::vector<SubtotalColumn> maSubtotalColumns;
    std::vector<SubtotalRow> maSubtotalRows;
    std::vector<Indent> maIndentCells;
    std::vector<Expander> maExpanderCells;

public:
    explicit StyleOutput(ScDocument& rDocument)
        : mrDocument(rDocument)
    {
    }

    void addFieldCell(SCCOL nCol, SCROW nRow, bool bInTable)
    {
        maFieldCells.push_back(FieldCell{ nCol, nRow, bInTable });
    }

    void addPageFieldValueCell(SCCOL nCol, SCROW nRow)
    {
        maPageFieldValueCells.push_back(PageFieldValueCell{ nCol, nRow });
    }

    void addColumnMemberSpan(size_t nField, SCCOL nStartCol, SCCOL nEndCol, SCROW nRow)
    {
        maColumnMemberSpans.push_back(ColumnMemberSpan{ nField, nStartCol, nEndCol, nRow });
    }

    void addRowMemberSpan(size_t nField, SCCOL nCol, SCROW nStartRow, SCROW nEndRow)
    {
        maRowMemberSpans.push_back(RowMemberSpan{ nField, nCol, nStartRow, nEndRow });
    }

    void addSubtotalColumn(SCCOL nCol, SCROW nStartRow, bool bGrandTotal)
    {
        maSubtotalColumns.push_back(SubtotalColumn{ nCol, nStartRow, bGrandTotal });
    }

    void addSubtotalRow(SCROW nRow, SCCOL nStartCol, bool bGrandTotal)
    {
        maSubtotalRows.push_back(SubtotalRow{ nRow, nStartCol, bGrandTotal });
    }

    void addIndent(SCCOL nCol, SCROW nRow, tools::Long nIndent)
    {
        maIndentCells.push_back(Indent{ nCol, nRow, nIndent });
    }

    void addExpander(SCCOL nCol, SCROW nRow, ScMF nFlags)
    {
        maExpanderCells.push_back(Expander{ nCol, nRow, nFlags });
    }

    /// Drops the sections of the previous run and starts a new one with the given geometry.
    void reset(Geometry const& rGeometry);

    void apply();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
