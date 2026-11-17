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

#pragma once

#include <address.hxx>
#include <rangelst.hxx>

#include <map>
#include <string_view>
#include <vector>

class ScDocument;
class ScMarkData;

namespace tools
{
class JsonWriter;
}

namespace sc
{
/**
 * One styled table: its range, and whether its cells can be edited.
 */
struct TableRangeEntry
{
    ScRange aRange;
    bool bEditable = false;

    bool operator==(const TableRangeEntry&) const = default;
};

/**
 * One sparkline group: the cells it covers.
 */
using SparklineGroupRangeEntry = ScRangeList;

/**
 * The cell ranges of interest in one sheet.
 */
struct SheetRangeInventory
{
    std::vector<TableRangeEntry> aTables;
    std::vector<SparklineGroupRangeEntry> aSparklineGroups;
    std::vector<ScRange> aMergedCells;

    bool operator==(const SheetRangeInventory&) const = default;
};

/**
 * The inventory of every sheet that has any ranges of interest, keyed by sheet index.
 */
using DocumentRangeInventory = std::map<SCTAB, SheetRangeInventory>;

/**
 * Write a range as [startCol, startRow, endCol, endRow] under aName.
 */
void putRange(tools::JsonWriter& rWriter, std::string_view aName, const ScRange& rRange);

/**
 * Write a range as [startCol, startRow, endCol, endRow] into an array.
 */
void putRange(tools::JsonWriter& rWriter, const ScRange& rRange);

/**
 * Write the marked ranges of sheet nTab as an array of ranges under aName.
 */
void putMarkedRanges(tools::JsonWriter& rWriter, std::string_view aName,
                     const ScMarkData& rMarkData, SCTAB nTab);

/**
 * Collect the styled tables of sheet nTab.
 */
std::vector<TableRangeEntry> collectTableRangeEntries(const ScDocument& rDoc, SCTAB nTab);

/**
 * Collect the inventory of every sheet.
 */
DocumentRangeInventory collectDocumentRangeInventory(ScDocument& rDoc);

/**
 * Collect the cells of each sparkline group of sheet nTab.
 */
std::vector<SparklineGroupRangeEntry> collectSparklineGroupRangeEntries(ScDocument& rDoc,
                                                                        SCTAB nTab);

/**
 * Collect the merged cells of sheet nTab, each as the range it covers.
 */
std::vector<ScRange> collectMergedCellRanges(ScDocument& rDoc, SCTAB nTab);

/**
 * Write the inventory of sheet nTab.
 */
void writeCellRangeInventory(tools::JsonWriter& rWriter, ScDocument& rDoc, SCTAB nTab);

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
