/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pivot/PivotTableFormatOutput.hxx>
#include <set>
#include <pivot/DPOutLevelData.hxx>

#include <dpoutput.hxx>
#include <dpobject.hxx>
#include <pivot.hxx>
#include <dptabdat.hxx>
#include <dpcache.hxx>
#include <document.hxx>

#include <com/sun/star/sheet/MemberResultFlags.hpp>

namespace sc
{
namespace
{
class NameResolver
{
private:
    ScDPObject& mrObject;
    ScDPCache const& mrCache;

    std::unordered_map<sal_Int32, std::vector<OUString>> maNameCache;

    void fillNamesForItems(std::vector<OUString>& rNames, ScDPCache::ScDPItemDataVec const& rItems,
                           sal_Int32 nDimension)
    {
        for (const auto& rItemData : rItems)
        {
            OUString sFormattedName;
            if (rItemData.HasStringData() || rItemData.IsEmpty())
                sFormattedName = rItemData.GetString();
            else
                sFormattedName = mrCache.GetFormattedString(nDimension, rItemData, false);
            rNames.push_back(sFormattedName);
        }
    }

    void fillNamesForDimension(std::vector<OUString>& rNames, sal_Int32 nDimension)
    {
        // Try to get member names in pivot field order first.
        // This matches the order used by the pivot table output.
        std::vector<ScDPLabelData::Member> aMembers;
        if (mrObject.GetMembers(nDimension, 0, aMembers) && !aMembers.empty())
        {
            for (const auto& rMember : aMembers)
                rNames.push_back(rMember.maName);
            return;
        }

        // Fallback to cache order
        if (mrCache.IsValidDimensionIndex(nDimension))
        {
            fillNamesForItems(rNames, mrCache.GetDimMemberValues(nDimension), nDimension);
        }
        else
        {
            auto* pGroup = mrCache.GetGroupItems(nDimension);
            if (pGroup)
            {
                fillNamesForItems(rNames, pGroup->maItems, nDimension);
            }
        }
    }

public:
    NameResolver(ScDPObject& rObject, ScDPCache const& rCache)
        : mrObject(rObject)
        , mrCache(rCache)
    {
    }

    OUString getNameForIndex(sal_uInt32 nIndex, sal_Int32 nDimension)
    {
        auto iterator = maNameCache.find(nDimension);
        if (iterator == maNameCache.end())
        {
            std::vector<OUString> aNames;
            fillNamesForDimension(aNames, nDimension);
            iterator = maNameCache.emplace(nDimension, aNames).first;
        }

        const std::vector<OUString>& rNames = iterator->second;
        if (nIndex >= rNames.size())
            return OUString();
        return rNames[nIndex];
    }
};

void initLines(std::vector<LineData>& rLines, std::vector<ScDPOutLevelData> const& rFields)
{
    for (const ScDPOutLevelData& rField : rFields)
    {
        size_t nFieldLength(rField.maResult.getLength());
        if (rLines.size() < nFieldLength)
            rLines.resize(nFieldLength);

        for (LineData& rLineData : rLines)
        {
            rLineData.maFields.resize(rFields.size());
        }
    }
}

Selection const* findSelection(PivotTableFormat const& rFormat, tools::Long nDimension)
{
    for (Selection const& rSelection : rFormat.getSelections())
    {
        if (rSelection.nField == nDimension)
            return &rSelection;
    }
    return nullptr;
}

void fillOutputFieldFromSelection(FormatOutputField& rOutputField, Selection const& rSelection,
                                  size_t nSelectionIndex, NameResolver& rNameResolver)
{
    if (rSelection.nIndices.empty())
    {
        rOutputField.bMatchesAll = true;
    }
    else
    {
        if (rSelection.nIndices.size() > 1 && rSelection.nIndices.size() > nSelectionIndex)
            rOutputField.nIndex = rSelection.nIndices[nSelectionIndex];
        else
            rOutputField.nIndex = rSelection.nIndices[0];

        if (rOutputField.nDimension == constDataDimension)
            rOutputField.aName = "DATA";
        else
            rOutputField.aName
                = rNameResolver.getNameForIndex(rOutputField.nIndex, rOutputField.nDimension);
    }
    rOutputField.bSelected = rSelection.bSelected;
    rOutputField.bHasSubtotal = rSelection.bHasSubtotal;
    rOutputField.bSet = true;
}

void initFormatOutputField(size_t nSelectionIndex, std::vector<FormatOutputField>& rOutputFields,
                           std::vector<ScDPOutLevelData> const& rFields,
                           PivotTableFormat const& rFormat, NameResolver& rNameResolver)
{
    rOutputFields.resize(rFields.size());
    for (size_t i = 0; i < rOutputFields.size(); i++)
    {
        FormatOutputField& rOutputField = rOutputFields[i];
        if (!rFields[i].mbDataLayout)
            rOutputField.nDimension = rFields[i].mnDim;

        Selection const* pSelection = findSelection(rFormat, rOutputField.nDimension);
        if (pSelection == nullptr)
            continue;

        fillOutputFieldFromSelection(rOutputField, *pSelection, nSelectionIndex, rNameResolver);
    }
}

} // end anonymous namespace

void FormatOutput::prepare(SCTAB nTab, std::vector<ScDPOutLevelData> const& rColumnFields,
                           std::vector<ScDPOutLevelData> const& rRowFields,
                           bool bColumnFieldIsDataOnly)
{
    if (!mpFormats)
        return;

    // initialize row and column lines, so the number of fields matches the pivot table output
    initLines(maRowLines, rRowFields);

    if (rColumnFields.size() == 0 && bColumnFieldIsDataOnly)
    {
        maColumnLines.resize(1);
        maColumnLines[0].maFields.resize(1);
    }
    else
    {
        initLines(maColumnLines, rColumnFields);
    }

    // check the table data exists
    auto* pTableData = mrObject.GetTableData();
    if (!pTableData)
        return;

    ScDPFilteredCache const& rFilteredCache = pTableData->GetCacheTable();
    ScDPCache const& rCache = rFilteredCache.getCache();

    NameResolver aNameResolver(mrObject, rCache);

    // Initialize format output entries (FormatOutputEntry) and set the data already available from output fields
    // (rColumnFields and rRowFields) and the pivot table format list (PivotTableFormat).

    for (PivotTableFormat const& rFormat : mpFormats->getVector())
    {
        size_t nMaxNumberOfIndices = 1;
        for (auto const& rSelection : rFormat.aSelections)
        {
            if (rSelection.nIndices.size() > 1)
                nMaxNumberOfIndices = rSelection.nIndices.size();
        }

        for (size_t nSelectionIndex = 0; nSelectionIndex < nMaxNumberOfIndices; nSelectionIndex++)
        {
            sc::FormatOutputEntry aEntry;
            aEntry.pPattern = rFormat.pPattern;
            aEntry.onTab = nTab;
            aEntry.eType = rFormat.eType;
            aEntry.bGrandRow = rFormat.bGrandRow;
            aEntry.bGrandColumn = rFormat.bGrandColumn;
            aEntry.oOffset = rFormat.oOffset;

            initFormatOutputField(nSelectionIndex, aEntry.aRowOutputFields, rRowFields, rFormat,
                                  aNameResolver);

            // If column fields list is empty, but there is a data field in columns that is not part of column fields
            if (rColumnFields.size() == 0 && bColumnFieldIsDataOnly)
            {
                // Initialize column output fields to have 1 data output field
                aEntry.aColumnOutputFields.resize(1);
                FormatOutputField& rOutputField = aEntry.aColumnOutputFields[0];
                rOutputField.nDimension = constDataDimension;
                Selection const* pSelection = findSelection(rFormat, constDataDimension);
                if (pSelection)
                    fillOutputFieldFromSelection(rOutputField, *pSelection, nSelectionIndex,
                                                 aNameResolver);
            }
            else
            {
                initFormatOutputField(nSelectionIndex, aEntry.aColumnOutputFields, rColumnFields,
                                      rFormat, aNameResolver);
            }

            maFormatOutputEntries.push_back(std::move(aEntry));
        }
    }
}

void FormatOutput::insertEmptyDataColumn(SCCOL nColPos, SCROW nRowPos)
{
    if (!mpFormats)
        return;

    LineData& rLine = maColumnLines[0];
    rLine.oLine = nColPos;
    rLine.oPosition = nRowPos;

    FieldData& rFieldData = rLine.maFields[0];
    rFieldData.nIndex = 0;
    rFieldData.bIsSet = true;
}

namespace
{
void fillLineAndFieldData(std::vector<LineData>& rLineDataVector, size_t nFieldIndex,
                          ScDPOutLevelData const& rField, tools::Long nMemberIndex,
                          sheet::MemberResult const& rMember, SCCOLROW nLine, SCCOLROW nPosition)
{
    LineData& rLine = rLineDataVector[nMemberIndex];
    rLine.oLine = nLine;
    rLine.oPosition = nPosition;

    FieldData& rFieldData = rLine.maFields[nFieldIndex];
    if (!rField.mbDataLayout)
        rFieldData.mnDimension = rField.mnDim;
    rFieldData.aName = rMember.Name;
    rFieldData.nIndex = nMemberIndex;
    rFieldData.bIsSet = true;
    rFieldData.bIsMember = rMember.Flags & sheet::MemberResultFlags::HASMEMBER;
    rFieldData.bSubtotal = rMember.Flags & sheet::MemberResultFlags::SUBTOTAL;
    rFieldData.bContinue = rMember.Flags & sheet::MemberResultFlags::CONTINUE;

    // Search previous entries for the name / value
    if (rFieldData.bContinue)
    {
        tools::Long nCurrent = nMemberIndex - 1;
        while (nCurrent >= 0 && rLineDataVector[nCurrent].maFields[nFieldIndex].bContinue)
            nCurrent--;

        if (nCurrent >= 0)
        {
            FieldData& rCurrentFieldData = rLineDataVector[nCurrent].maFields[nFieldIndex];
            rFieldData.aName = rCurrentFieldData.aName;
            rFieldData.nIndex = rCurrentFieldData.nIndex;
        }
    }
}
} // end anonymous namespace

void FormatOutput::insertFieldMember(size_t nFieldIndex, ScDPOutLevelData const& rField,
                                     tools::Long nMemberIndex, sheet::MemberResult const& rMember,
                                     SCCOL nColPos, SCROW nRowPos,
                                     sc::FormatResultDirection eResultDirection)
{
    if (!mpFormats)
        return;

    if (eResultDirection == sc::FormatResultDirection::ROW)
    {
        fillLineAndFieldData(maRowLines, nFieldIndex, rField, nMemberIndex, rMember, nRowPos,
                             nColPos);
    }
    else if (eResultDirection == sc::FormatResultDirection::COLUMN)
    {
        fillLineAndFieldData(maColumnLines, nFieldIndex, rField, nMemberIndex, rMember, nColPos,
                             nRowPos);
    }
}
namespace
{
/** Find lines matching the format's field criteria.
 *
 * For labels: unreferenced fields must have a member/continuation.
 * For data: exact matches take priority over broad matches. Falls back to broad if no exact.
 */
void findMatchingLines(std::vector<LineData> const& rLines,
                       std::vector<FormatOutputField> const& rFormatOutputField, FormatType eType,
                       std::vector<std::reference_wrapper<const LineData>>& rMatches)
{
    std::vector<std::reference_wrapper<const LineData>> aBroadMatches;

    for (LineData const& rLineData : rLines)
    {
        size_t nNoOfFields = rLineData.maFields.size();
        bool bAllMatch = true;
        bool bHasWildcardMember = false;

        for (size_t nIndex = 0; nIndex < nNoOfFields && bAllMatch; nIndex++)
        {
            FieldData const& rFieldData = rLineData.maFields[nIndex];
            FormatOutputField const& rFormatEntry = rFormatOutputField[nIndex];

            if (rFieldData.mnDimension != rFormatEntry.nDimension)
            {
                bAllMatch = false;
                break;
            }

            if (!rFormatEntry.bSet)
            {
                if (eType == FormatType::Label && !rFieldData.bIsMember && !rFieldData.bContinue)
                    bAllMatch = false;
                else if (eType == FormatType::Data
                         && (rFieldData.bIsMember || rFieldData.bContinue))
                    bHasWildcardMember = true;
                continue;
            }

            bool bFieldMatch = false;

            if (rFormatEntry.bHasSubtotal && rFieldData.aName == rFormatEntry.aName)
                bFieldMatch = true;
            else if (rFormatEntry.bMatchesAll && !rFieldData.bSubtotal)
                bFieldMatch = true;
            else if (rFormatEntry.nDimension == constDataDimension
                     && rFieldData.nIndex == rFormatEntry.nIndex)
                bFieldMatch = true;
            else if (rFormatEntry.nDimension != constDataDimension
                     && rFieldData.aName == rFormatEntry.aName)
                bFieldMatch = true;

            if (!bFieldMatch)
                bAllMatch = false;
        }

        if (bAllMatch)
        {
            if (bHasWildcardMember)
                aBroadMatches.push_back(std::cref(rLineData));
            else
                rMatches.push_back(std::cref(rLineData));
        }
    }

    // For data, use exact matching if available, otherwise fall back to broad
    if (eType == FormatType::Data && rMatches.empty() && !aBroadMatches.empty())
        rMatches = std::move(aBroadMatches);
}

/** Apply matched lines — labels go directly to cells, data collects rows/columns */
void applyMatchedLines(ScDocument& rDocument,
                       std::vector<std::reference_wrapper<const LineData>> const& rMatches,
                       std::vector<SCCOLROW>& aRows, std::vector<SCCOLROW>& aColumns,
                       FormatOutputEntry const& rEntry, FormatResultDirection eResultDirection,
                       SCCOL nTabStartColumn, SCROW nTabStartRow, SCCOL nDataStartColumn,
                       SCROW nDataStartRow)
{
    assert(rEntry.onTab);
    assert(rEntry.pPattern);

    for (LineData const& rLineData : rMatches)
    {
        if (!rLineData.oLine || !rLineData.oPosition)
            continue;

        if (rEntry.eType == FormatType::Label)
        {
            SCCOLROW nColumn = *rLineData.oLine;
            SCCOLROW nRow = *rLineData.oPosition;

            if (eResultDirection == FormatResultDirection::ROW)
                std::swap(nRow, nColumn);

            // Apply offset if present — redirects the label to a specific cell
            if (rEntry.oOffset)
            {
                if (eResultDirection == FormatResultDirection::ROW
                    && nDataStartColumn > nTabStartColumn)
                {
                    SCCOL nOffsetColumn = rEntry.oOffset->aStart.Col();
                    nColumn = nTabStartColumn
                              + std::min(SCCOLROW(nOffsetColumn),
                                         SCCOLROW(nDataStartColumn - nTabStartColumn - 1));
                }
                else if (eResultDirection == FormatResultDirection::COLUMN
                         && nDataStartRow > nTabStartRow)
                {
                    SCROW nOffsetRow = rEntry.oOffset->aStart.Row();
                    nRow = nTabStartRow
                           + std::min(SCCOLROW(nOffsetRow),
                                      SCCOLROW(nDataStartRow - nTabStartRow - 1));
                }
            }

            rDocument.ApplyPattern(nColumn, nRow, *rEntry.onTab, *rEntry.pPattern);
        }
        else if (rEntry.eType == FormatType::Data)
        {
            if (eResultDirection == FormatResultDirection::ROW)
                aRows.push_back(*rLineData.oLine);
            else if (eResultDirection == FormatResultDirection::COLUMN)
                aColumns.push_back(*rLineData.oLine);
        }
    }
}

/** Check if any output field in the vector has been set (has a reference). */
bool hasSetFields(std::vector<FormatOutputField> const& rFields)
{
    if (rFields.empty())
        return false;
    return std::any_of(rFields.begin(), rFields.end(),
                       [](auto const& rField) { return rField.bSet; });
}

/** Iterate matched lines and call the applicator for each line position, but
  * skip the grand total position. */
template <typename LinesT>
void applyToMatchedLines(LinesT const& rMatchedLines, SCCOLROW nGrandTotalPositionToSkip,
                         std::function<void(SCCOLROW nLinePosition)> const& rApplicator)
{
    for (LineData const& rLine : rMatchedLines)
    {
        if (!rLine.oLine)
            continue;
        if (nGrandTotalPositionToSkip >= 0 && *rLine.oLine == nGrandTotalPositionToSkip)
            continue;
        rApplicator(*rLine.oLine);
    }
}

/** Apply a grand total data format along one axis.
 *  If there are references, then find specific lines, otherwise apply to all
 *  lines. Skips the grand total position to avoid setting the intersection
 *  cell. */
void applyGrandTotalDataFormat(std::vector<LineData> const& rLines,
                               std::vector<FormatOutputField> const& rOutputFields,
                               SCCOLROW nGrandTotalPositionToSkip,
                               std::function<void(SCCOLROW nLinePosition)> const& rApplicator)
{
    if (hasSetFields(rOutputFields))
    {
        // Use FormatType::None to collect all matching lines (both exact and broad)
        // because grand totals need to apply to all matches, not just the narrower set
        std::vector<std::reference_wrapper<const LineData>> aMatches;
        findMatchingLines(rLines, rOutputFields, FormatType::None, aMatches);
        applyToMatchedLines(aMatches, nGrandTotalPositionToSkip, rApplicator);
    }
    else
    {
        applyToMatchedLines(rLines, nGrandTotalPositionToSkip, rApplicator);
    }
}

} // end anonymous namespace

bool FormatOutput::tryHandleGrandTotals(ScDocument& rDocument, sc::FormatOutputEntry const& rEntry)
{
    if (rEntry.bGrandRow && rEntry.bGrandColumn && mnGrandTotalRow >= 0 && mnGrandTotalColumn >= 0)
    {
        rDocument.ApplyPattern(mnGrandTotalColumn, mnGrandTotalRow, *rEntry.onTab,
                               *rEntry.pPattern);
        return true;
    }

    if (rEntry.bGrandRow && mnGrandTotalRow >= 0)
    {
        if (rEntry.eType == FormatType::Data)
        {
            applyGrandTotalDataFormat(maColumnLines, rEntry.aColumnOutputFields, mnGrandTotalColumn,
                                      [&](SCCOLROW nColumnPosition) {
                                          rDocument.ApplyPattern(nColumnPosition, mnGrandTotalRow,
                                                                 *rEntry.onTab, *rEntry.pPattern);
                                      });
        }
        else if (rEntry.eType == FormatType::Label)
        {
            if (rEntry.oOffset && mnTabStartColumn >= 0 && mnDataStartColumn > mnTabStartColumn)
            {
                // Offset column is relative to the label area
                SCCOL nOffsetColumn = rEntry.oOffset->aStart.Col();
                SCCOL nLabelCount = mnDataStartColumn - mnTabStartColumn;
                SCCOL nTargetColumn
                    = mnTabStartColumn + std::min(nOffsetColumn, SCCOL(nLabelCount - 1));
                rDocument.ApplyPattern(nTargetColumn, mnGrandTotalRow, *rEntry.onTab,
                                       *rEntry.pPattern);
            }
            else if (mnTabStartColumn >= 0 && mnDataStartColumn > mnTabStartColumn)
            {
                // No offset: apply to all label columns
                for (SCCOL nColumn = mnTabStartColumn; nColumn < mnDataStartColumn; ++nColumn)
                {
                    rDocument.ApplyPattern(nColumn, mnGrandTotalRow, *rEntry.onTab,
                                           *rEntry.pPattern);
                }
            }
        }
        return true;
    }

    if (rEntry.bGrandColumn && mnGrandTotalColumn >= 0)
    {
        if (rEntry.eType == FormatType::Data)
        {
            applyGrandTotalDataFormat(maRowLines, rEntry.aRowOutputFields, mnGrandTotalRow,
                                      [&](SCCOLROW nRowPosition) {
                                          rDocument.ApplyPattern(mnGrandTotalColumn, nRowPosition,
                                                                 *rEntry.onTab, *rEntry.pPattern);
                                      });
        }
        else if (rEntry.eType == FormatType::Label)
        {
            // Apply to the column header rows at the grand total column.
            // Use the column lines' positions to find header row positions.
            std::set<SCROW> aHeaderRows;
            for (LineData const& rColLine : maColumnLines)
            {
                if (rColLine.oPosition)
                    aHeaderRows.insert(*rColLine.oPosition);
            }
            for (SCROW nRow : aHeaderRows)
            {
                rDocument.ApplyPattern(mnGrandTotalColumn, nRow, *rEntry.onTab, *rEntry.pPattern);
            }
        }
        return true;
    }

    return false;
}

void FormatOutput::apply(ScDocument& rDocument)
{
    if (!mpFormats)
        return;

    for (auto const& rEntry : maFormatOutputEntries)
    {
        if (!rEntry.onTab || !rEntry.pPattern)
            continue;

        // Handle grand total formats
        if (tryHandleGrandTotals(rDocument, rEntry))
            continue;

        std::vector<SCCOLROW> aRows;
        std::vector<SCCOLROW> aColumns;

        bool bHasRowReferences = hasSetFields(rEntry.aRowOutputFields);
        bool bHasColumnReferences = hasSetFields(rEntry.aColumnOutputFields);

        // For labels, only match the axis that has references to avoid
        // wildcard matching applying labels to the wrong axis cells.
        bool bMatchRows = (rEntry.eType != FormatType::Label) || bHasRowReferences;
        bool bMatchColumns = (rEntry.eType != FormatType::Label) || bHasColumnReferences;

        if (bMatchRows)
        {
            std::vector<std::reference_wrapper<const LineData>> aRowMatches;
            findMatchingLines(maRowLines, rEntry.aRowOutputFields, rEntry.eType, aRowMatches);
            applyMatchedLines(rDocument, aRowMatches, aRows, aColumns, rEntry,
                              FormatResultDirection::ROW, mnTabStartColumn, mnTabStartRow,
                              mnDataStartColumn, mnDataStartRow);
        }

        if (bMatchColumns)
        {
            std::vector<std::reference_wrapper<const LineData>> aColumnMatches;
            findMatchingLines(maColumnLines, rEntry.aColumnOutputFields, rEntry.eType,
                              aColumnMatches);
            applyMatchedLines(rDocument, aColumnMatches, aRows, aColumns, rEntry,
                              FormatResultDirection::COLUMN, mnTabStartColumn, mnTabStartRow,
                              mnDataStartColumn, mnDataStartRow);
        }

        if (!aColumns.empty() && !aRows.empty() && rEntry.eType == FormatType::Data)
        {
            for (SCCOLROW nRow : aRows)
            {
                for (SCCOLROW nColumn : aColumns)
                {
                    rDocument.ApplyPattern(nColumn, nRow, *rEntry.onTab, *rEntry.pPattern);
                }
            }
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
