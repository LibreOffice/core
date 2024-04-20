/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pivot/PivotTableFormatOutput.hxx>
#include <pivot/DPOutLevelData.hxx>

#include <dpoutput.hxx>
#include <dpobject.hxx>
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
    ScDPTableData& mrTableData;
    ScDPCache const& mrCache;

    std::unordered_map<sal_Int32, std::vector<OUString>> maNameCache;

    void fillNamesForDimension(std::vector<OUString>& rNames, sal_Int32 nDimension)
    {
        for (const auto& rItemData : mrCache.GetDimMemberValues(nDimension))
        {
            OUString sFormattedName;
            if (rItemData.HasStringData() || rItemData.IsEmpty())
                sFormattedName = rItemData.GetString();
            else
                sFormattedName = ScDPObject::GetFormattedString(&mrTableData, nDimension,
                                                                rItemData.GetValue());
            rNames.push_back(sFormattedName);
        }
    }

public:
    NameResolver(ScDPTableData& rTableData, ScDPCache const& rCache)
        : mrTableData(rTableData)
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
    for (size_t i = 0; i < rFields.size(); i++)
    {
        size_t nFieldLength(rFields[i].maResult.getLength());
        if (rLines.size() < nFieldLength)
            rLines.resize(nFieldLength);

        for (LineData& rLineData : rLines)
        {
            rLineData.maFields.resize(rFields.size());
        }
    }
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

        for (auto const& rSelection : rFormat.aSelections)
        {
            if (!rSelection.nIndices.empty() && rSelection.nField == rOutputField.nDimension)
            {
                if (rSelection.nIndices.size() > nSelectionIndex)
                    rOutputField.nIndex = rSelection.nIndices[nSelectionIndex];
                else
                    rOutputField.nIndex = rSelection.nIndices[0];

                if (rOutputField.nDimension == -2)
                    rOutputField.aName = "DATA";
                else
                    rOutputField.aName = rNameResolver.getNameForIndex(rOutputField.nIndex,
                                                                       rOutputField.nDimension);

                rOutputField.bSet = true;
            }
        }
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

    NameResolver aNameResolver(*pTableData, rCache);

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

        if (nMaxNumberOfIndices == 0)
            continue;

        for (size_t nSelectionIndex = 0; nSelectionIndex < nMaxNumberOfIndices; nSelectionIndex++)
        {
            sc::FormatOutputEntry aEntry;
            aEntry.pPattern = rFormat.pPattern;
            aEntry.onTab = nTab;
            aEntry.eType = rFormat.eType;

            initFormatOutputField(nSelectionIndex, aEntry.aRowOutputFields, rRowFields, rFormat,
                                  aNameResolver);

            // If column fields list is empty, but there is a data field in columns that is not part of column fields
            if (rColumnFields.size() == 0 && bColumnFieldIsDataOnly)
            {
                // Initialize column output fields to have 1 data output field
                aEntry.aColumnOutputFields.resize(1);
                FormatOutputField& rOutputField = aEntry.aColumnOutputFields[0];

                for (auto const& rSelection : rFormat.aSelections)
                {
                    if (rSelection.nField == -2)
                    {
                        if (rSelection.nIndices.size() > 1)
                            rOutputField.nIndex = rSelection.nIndices[nSelectionIndex];
                        else
                            rOutputField.nIndex = rSelection.nIndices[0];
                        rOutputField.aName = "DATA";
                        rOutputField.bSet = true;
                    }
                }
            }
            else
            {
                initFormatOutputField(nSelectionIndex, aEntry.aColumnOutputFields, rColumnFields,
                                      rFormat, aNameResolver);
            }

            maFormatOutputEntries.push_back(aEntry);
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
        fillLineAndFieldData(maRowLines, nFieldIndex, rField, nMemberIndex, rMember, nRowPos,
                             nColPos);
    else if (eResultDirection == sc::FormatResultDirection::COLUMN)
        fillLineAndFieldData(maColumnLines, nFieldIndex, rField, nMemberIndex, rMember, nColPos,
                             nRowPos);
}
namespace
{
void checkForMatchingLines(std::vector<LineData> const& rLines,
                           std::vector<FormatOutputField> const& rFormatOutputField,
                           std::vector<std::reference_wrapper<const LineData>>& rMatches,
                           std::vector<std::reference_wrapper<const LineData>>& rMaybeMatches)
{
    for (LineData const& rLineData : rLines)
    {
        size_t nMatch = 0;
        size_t nMaybeMatch = 0;
        size_t nNoOfFields = rLineData.maFields.size();

        for (size_t nIndex = 0; nIndex < nNoOfFields; nIndex++)
        {
            FieldData const& rFieldData = rLineData.maFields[nIndex];
            FormatOutputField const& rFormatEntry = rFormatOutputField[nIndex];
            bool bFieldMatch = false;
            bool bFieldMaybeMatch = false;

            tools::Long nDimension = rFieldData.mnDimension;
            if (nDimension == rFormatEntry.nDimension)
            {
                if (rFormatEntry.bSet)
                {
                    if (nDimension == -2 && rFieldData.nIndex == rFormatEntry.nIndex)
                        bFieldMatch = true;
                    else if (nDimension != -2 && rFieldData.aName == rFormatEntry.aName)
                        bFieldMatch = true;
                }
                else if (!rFormatEntry.bSet && !rFieldData.bIsMember && !rFieldData.bContinue)
                {
                    bFieldMatch = true;
                }
                else
                {
                    bFieldMaybeMatch = true;
                }
            }

            if (!bFieldMatch && !bFieldMaybeMatch)
                break;

            if (bFieldMatch)
                nMatch++;

            if (bFieldMaybeMatch)
                nMaybeMatch++;
        }

        if (nMatch == nNoOfFields)
        {
            rMatches.push_back(std::cref(rLineData));
            break;
        }
        else if (nMatch + nMaybeMatch == nNoOfFields && nMatch != 0)
        {
            rMaybeMatches.push_back(std::cref(rLineData));
        }
    }
}

/** Check the lines in matches and maybe matches and output */
void evaluateMatches(ScDocument& rDocument,
                     std::vector<std::reference_wrapper<const LineData>> const& rMatches,
                     std::vector<std::reference_wrapper<const LineData>> const& rMaybeMatches,
                     std::optional<SCCOLROW>& oRow, std::optional<SCCOLROW>& oColumn,
                     FormatOutputEntry const& rOutputEntry, FormatResultDirection eResultDirection)
{
    // We expect that tab and pattern to be set or this method shouldn't be called at all
    assert(rOutputEntry.onTab);
    assert(rOutputEntry.pPattern);

    // We can output only if there is exactly 1 match or 1 maybe match
    if (rMatches.size() != 1 && rMaybeMatches.size() != 1)
        return;

    LineData const& rLineData = !rMatches.empty() ? rMatches.back() : rMaybeMatches.back();

    // Can't continue if we don't have complete row/column data
    if (!rLineData.oLine || !rLineData.oPosition)
        return;

    if (rOutputEntry.eType == FormatType::Label)
    {
        // Primary axis is set to column (line) then row (position)
        SCCOLROW nColumn = *rLineData.oLine;
        SCCOLROW nRow = *rLineData.oPosition;

        // In row orientation, the primary axis is row, then column, so we need to swap
        if (eResultDirection == FormatResultDirection::ROW)
            std::swap(nRow, nColumn);

        // Set the pattern to the sheet
        rDocument.ApplyPattern(nColumn, nRow, *rOutputEntry.onTab, *rOutputEntry.pPattern);
    }
    else if (rOutputEntry.eType == FormatType::Data)
    {
        if (eResultDirection == FormatResultDirection::ROW)
            oRow = rLineData.oLine;
        else if (eResultDirection == FormatResultDirection::COLUMN)
            oColumn = rLineData.oLine;
    }
}

} // end anonymous namespace

void FormatOutput::apply(ScDocument& rDocument)
{
    if (!mpFormats)
        return;

    for (auto const& rOutputEntry : maFormatOutputEntries)
    {
        if (!rOutputEntry.onTab || !rOutputEntry.pPattern)
            continue;

        std::optional<SCCOLROW> oRow;
        std::optional<SCCOLROW> oColumn;
        {
            std::vector<std::reference_wrapper<const LineData>> rMatches;
            std::vector<std::reference_wrapper<const LineData>> rMaybeMatches;

            checkForMatchingLines(maRowLines, rOutputEntry.aRowOutputFields, rMatches,
                                  rMaybeMatches);
            evaluateMatches(rDocument, rMatches, rMaybeMatches, oRow, oColumn, rOutputEntry,
                            FormatResultDirection::ROW);
        }

        {
            std::vector<std::reference_wrapper<const LineData>> rMatches;
            std::vector<std::reference_wrapper<const LineData>> rMaybeMatches;

            checkForMatchingLines(maColumnLines, rOutputEntry.aColumnOutputFields, rMatches,
                                  rMaybeMatches);
            evaluateMatches(rDocument, rMatches, rMaybeMatches, oRow, oColumn, rOutputEntry,
                            FormatResultDirection::COLUMN);
        }

        if (oColumn && oRow && rOutputEntry.eType == FormatType::Data)
        {
            rDocument.ApplyPattern(*oColumn, *oRow, *rOutputEntry.onTab, *rOutputEntry.pPattern);
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
