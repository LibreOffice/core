/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string_view>

#pragma once

class ScViewData;

namespace sc
{
/** Operations that can be performed on a sheet. */
enum class OperationType
{
    Unknown,
    DeleteContent,
    DeleteCell,
    TransliterateText,
    SetNormalString,
    SetNoteText,
    ReplaceNoteText,
    InsertColumnsBefore,
    InsertColumnsAfter,
    InsertRowsBefore,
    InsertRowsAfter,
    InsertCellsDown,
    InsertCellsRight,
    DeleteColumns,
    DeleteRows,
    DeleteCellsLeft,
    DeleteCellsUp,
    MoveBlock,
    ClearItems,
    ChangeIndent,
    AutoFormat,
    EnterMatrix,
    TabOperation,
    FillSimple,
    FillSeries,
    FillAuto,
    MergeCells,
    InsertNameList,
    ConvertFormulaToValue,
    Sort,
    Query,
    SubTotals,
    PivotTableUpdate,
    PivotTableRemove,
    PivotTableCreate,
    SparklineInsert,
    SparklineDelete,
    SparklineChange,
    SparklineGroup,
    SparklineUngroup,
    SparklineGroupDelete,
    SparklineGroupChange,
    EnterData,
};

/** Tester for operations on sheet views and default views
 *
 * Tests if an operation can be performed in a sheet view or the operation on the default view unsyncs the sheet view(s).
 */
class SheetViewOperationsTester
{
    ScViewData* mpViewData;

public:
    SheetViewOperationsTester(ScViewData* pViewData)
        : mpViewData(pViewData)
    {
    }

    bool check(OperationType eOperationType) const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
