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

/** A cell with a field caption. When bFrame is true, the cell is part of the table area and
 *  carries a thin frame around it. */
struct FieldCell
{
    SCCOL nCol = -1;
    SCROW nRow = -1;
    bool bFrame = false;
};

/** A cell holding the selected value of a page field. */
struct PageFieldValueCell
{
    SCCOL nCol = -1;
    SCROW nRow = -1;
};

/** One member of a column field, spanning the columns nStartCol to nEndCol in the header row
 *  nRow. nField is the position of the field among the column fields. */
struct ColumnMemberSpan
{
    size_t nField = 0;
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    SCROW nRow = -1;
};

/** One member of a row field, spanning the rows nStartRow to nEndRow in the header column
 *  nCol. nField is the position of the field among the row fields. */
struct RowMemberSpan
{
    size_t nField = 0;
    SCCOL nCol = -1;
    SCROW nStartRow = -1;
    SCROW nEndRow = -1;
};

/** A column holding subtotal results. The header cells reach from nStartRow down to the row
 *  above the data start row, and the result cells fill the column below. bGrandTotal is true
 *  for the grand total column. */
struct SubtotalColumn
{
    SCCOL nCol = -1;
    SCROW nStartRow = -1;
    bool bGrandTotal = false;
};

/** A row holding subtotal results. The header cells reach from nStartCol to the column left
 *  of the data start column, and the result cells fill the row to the right. bGrandTotal is
 *  true for the grand total row. */
struct SubtotalRow
{
    SCROW nRow = -1;
    SCCOL nStartCol = -1;
    bool bGrandTotal = false;
};

/** A row field member cell with indented content. The indent value is in twips. */
struct Indent
{
    SCCOL nCol = -1;
    SCROW nRow = -1;
    tools::Long nIndent = 0;
};

/** A row field member cell that shows an expand or collapse button. */
struct Expander
{
    SCCOL nCol = -1;
    SCROW nRow = -1;
    ScMF nFlags = ScMF::NONE;
};

/** Collects the sections of a pivot table while the table content is written to the sheet, and
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
    StyleOutput(ScDocument& rDocument)
        : mrDocument(rDocument)
    {
    }

    void setGeometry(Geometry const& rGeometry) { maGeometry = rGeometry; }

    void addFieldCell(SCCOL nCol, SCROW nRow, bool bFrame)
    {
        maFieldCells.push_back(FieldCell{ nCol, nRow, bFrame });
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

    void clear();
    void apply();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
