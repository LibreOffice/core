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

#include <CellRangeInventory.hxx>

#include <attrib.hxx>
#include <dbdata.hxx>
#include <dociter.hxx>
#include <document.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <patattr.hxx>

#include <tools/json_writer.hxx>

#include <algorithm>
#include <optional>
#include <unordered_map>

namespace sc
{
namespace
{
// Get the range of the cell at rPos, widened to its merged area.
ScRange getMergedCellRange(ScDocument& rDoc, const ScAddress& rPos)
{
    ScRange aRange(rPos);
    if (rDoc.IsMerged(rPos))
        rDoc.ExtendMerge(aRange);
    return aRange;
}

// Write the column and row indexes of a range into an open array.
void putRangeIndexes(tools::JsonWriter& rWriter, const ScRange& rRange)
{
    rWriter.putSimpleValue(rRange.aStart.Col());
    rWriter.putSimpleValue(rRange.aStart.Row());
    rWriter.putSimpleValue(rRange.aEnd.Col());
    rWriter.putSimpleValue(rRange.aEnd.Row());
}

// Get the range of a database range that is a styled table; none for a regular one.
std::optional<ScRange> getTableRange(const ScDBData& rData)
{
    if (!rData.GetTableStyleInfo())
        return std::nullopt;

    ScRange aRange;
    rData.GetArea(aRange);
    return aRange;
}

// Test whether the cells of rRange can be edited.
bool isEditable(const ScDocument& rDoc, const ScRange& rRange)
{
    return ScEditableTester::CreateAndTestBlock(rDoc, rRange.aStart.Tab(), rRange.aStart.Col(),
                                                rRange.aStart.Row(), rRange.aEnd.Col(),
                                                rRange.aEnd.Row())
        .IsEditable();
}
}

void putRange(tools::JsonWriter& rWriter, std::string_view aName, const ScRange& rRange)
{
    const auto aArray = rWriter.startArray(aName);
    putRangeIndexes(rWriter, rRange);
}

void putRange(tools::JsonWriter& rWriter, const ScRange& rRange)
{
    const auto aArray = rWriter.startAnonArray();
    putRangeIndexes(rWriter, rRange);
}

void putMarkedRanges(tools::JsonWriter& rWriter, std::string_view aName,
                     const ScMarkData& rMarkData, SCTAB nTab)
{
    ScRangeList aMarkedRanges;
    rMarkData.FillRangeListWithMarks(&aMarkedRanges, false, nTab);

    const auto aArray = rWriter.startArray(aName);
    for (const ScRange& rRange : aMarkedRanges)
        putRange(rWriter, rRange);
}

std::vector<TableRangeEntry> collectTableRangeEntries(const ScDocument& rDoc, SCTAB nTab)
{
    std::vector<TableRangeEntry> aEntries;

    const ScDBCollection* pDBs = rDoc.GetDBCollection();
    if (!pDBs)
        return aEntries;

    for (const auto& rxData : pDBs->getNamedDBs())
    {
        std::optional<ScRange> oRange = getTableRange(*rxData);
        if (!oRange || oRange->aStart.Tab() != nTab)
            continue;

        aEntries.push_back({ *oRange, isEditable(rDoc, *oRange) });
    }

    return aEntries;
}

DocumentRangeInventory collectDocumentRangeInventory(ScDocument& rDoc)
{
    DocumentRangeInventory aInventory;

    if (const ScDBCollection* pDBs = rDoc.GetDBCollection())
    {
        for (const auto& rxData : pDBs->getNamedDBs())
        {
            std::optional<ScRange> oRange = getTableRange(*rxData);
            if (!oRange)
                continue;

            aInventory[oRange->aStart.Tab()].aTables.push_back(
                { *oRange, isEditable(rDoc, *oRange) });
        }
    }

    for (SCTAB nTab = 0; nTab < rDoc.GetTableCount(); ++nTab)
    {
        std::vector<SparklineGroupRangeEntry> aGroups
            = collectSparklineGroupRangeEntries(rDoc, nTab);
        if (!aGroups.empty())
            aInventory[nTab].aSparklineGroups = std::move(aGroups);

        std::vector<ScRange> aMergedCells = collectMergedCellRanges(rDoc, nTab);
        if (!aMergedCells.empty())
            aInventory[nTab].aMergedCells = std::move(aMergedCells);
    }

    return aInventory;
}

std::vector<ScRange> collectMergedCellRanges(ScDocument& rDoc, SCTAB nTab)
{
    std::vector<ScRange> aRanges;

    // A merged cell allocates the columns it starts in.
    const SCCOL nLastCol = rDoc.GetAllocatedColumnsCount(nTab) - 1;
    const SCROW nLastRow = rDoc.MaxRow();
    if (nLastCol < 0
        || !rDoc.HasAttrib(0, 0, nTab, nLastCol, nLastRow, nTab, HasAttrFlags::Merged))
        return aRanges;

    ScDocAttrIterator aIter(rDoc, nTab, 0, 0, nLastCol, nLastRow);
    SCCOL nCol = 0;
    SCROW nRow1 = 0;
    SCROW nRow2 = 0;
    while (const ScPatternAttr* pPattern = aIter.GetNext(nCol, nRow1, nRow2))
    {
        const ScMergeAttr& rMerge = pPattern->GetItem(ATTR_MERGE);
        if (!rMerge.IsMerged())
            continue;

        // Each row of the run starts a merged cell of the same size.
        const SCCOL nColSpan = std::max<SCCOL>(1, rMerge.GetColMerge());
        const SCROW nRowSpan = std::max<SCROW>(1, rMerge.GetRowMerge());
        for (SCROW nRow = nRow1; nRow <= nRow2; nRow += nRowSpan)
            aRanges.emplace_back(nCol, nRow, nTab, nCol + nColSpan - 1, nRow + nRowSpan - 1, nTab);
    }

    return aRanges;
}

std::vector<SparklineGroupRangeEntry> collectSparklineGroupRangeEntries(ScDocument& rDoc,
                                                                        SCTAB nTab)
{
    std::vector<SparklineGroupRangeEntry> aEntries;
    std::unordered_map<const SparklineGroup*, std::size_t> aGroupIndices;
    for (const auto& [rPos, pGroup] : rDoc.GetSparklinePositions(nTab))
    {
        // See if it's a new group...
        auto [it, bInserted] = aGroupIndices.try_emplace(pGroup, aEntries.size());
        if (bInserted)
            aEntries.emplace_back(); // new group - create a slot at the end

        aEntries[it->second].Join(getMergedCellRange(rDoc, rPos));
    }
    return aEntries;
}

void writeCellRangeInventory(tools::JsonWriter& rWriter, ScDocument& rDoc, SCTAB nTab)
{
    // part means sheet index
    rWriter.put("part", static_cast<sal_Int32>(nTab));

    {
        const auto aTablesArray = rWriter.startArray("tables");
        for (const TableRangeEntry& rEntry : collectTableRangeEntries(rDoc, nTab))
        {
            const auto aTable = rWriter.startStruct();
            putRange(rWriter, "range", rEntry.aRange);
            rWriter.put("editable", rEntry.bEditable);
        }
    }

    {
        const auto aGroupsArray = rWriter.startArray("sparklineGroups");
        for (const SparklineGroupRangeEntry& rRanges :
             collectSparklineGroupRangeEntries(rDoc, nTab))
        {
            const auto aGroup = rWriter.startAnonArray();
            for (const ScRange& rRange : rRanges)
                putRange(rWriter, rRange);
        }
    }

    const auto aMergedArray = rWriter.startArray("mergedCells");
    for (const ScRange& rRange : collectMergedCellRanges(rDoc, nTab))
        putRange(rWriter, rRange);
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
