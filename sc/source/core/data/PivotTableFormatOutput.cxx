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
            maNameCache.emplace(nDimension, aNames);
            return aNames[nIndex];
        }
        else
        {
            std::vector<OUString>& rNames = iterator->second;
            return rNames[nIndex];
        }
    }
};

namespace
{
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

void initFormatOutputField(std::vector<FormatOutputField>& rOutputFields,
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
            if (rSelection.nField == rOutputField.nDimension)
            {
                if (rOutputField.nDimension == -2)
                {
                    rOutputField.aName = "DATA";
                    rOutputField.nIndex = rSelection.nDataIndex;
                    rOutputField.bSet = true;
                }
                else
                {
                    rOutputField.aName
                        = rNameResolver.getNameForIndex(rSelection.nDataIndex, rSelection.nField);
                    rOutputField.nIndex = rSelection.nDataIndex;
                    rOutputField.bSet = true;
                }
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

    auto* pTableData = mrObject.GetTableData();
    if (!pTableData)
        return;

    ScDPFilteredCache const& rFilteredCache = pTableData->GetCacheTable();
    ScDPCache const& rCache = rFilteredCache.getCache();

    NameResolver aNameResolver(*pTableData, rCache);

    size_t nFormatIndex = 0;
    for (PivotTableFormat const& rFormat : mpFormats->getVector())
    {
        sc::FormatOutputEntry& rEntry = maFormatOutputEntries[nFormatIndex];
        rEntry.pPattern = rFormat.pPattern;
        rEntry.onTab = nTab;
        rEntry.eType = rFormat.eType;

        initFormatOutputField(rEntry.aRowOutputFields, rRowFields, rFormat, aNameResolver);

        if (rColumnFields.size() == 0 && bColumnFieldIsDataOnly)
        {
            rEntry.aColumnOutputFields.resize(1);
            FormatOutputField& rOutputField = rEntry.aColumnOutputFields[0];

            for (auto const& rSelection : rFormat.aSelections)
            {
                if (rSelection.nField == -2)
                {
                    rOutputField.aName = "DATA";
                    rOutputField.nIndex = rSelection.nDataIndex;
                    rOutputField.bSet = true;
                }
            }
        }
        else
        {
            initFormatOutputField(rEntry.aColumnOutputFields, rColumnFields, rFormat,
                                  aNameResolver);
        }
        nFormatIndex++;
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

        for (LineData const& rLineData : maRowLines)
        {
            bool bMatchesAll = true;

            for (size_t nIndex = 0; nIndex < rLineData.maFields.size(); nIndex++)
            {
                FieldData const& rFieldData = rLineData.maFields[nIndex];
                FormatOutputField const& rFormatEntry = rOutputEntry.aRowOutputFields[nIndex];

                tools::Long nDimension = rFieldData.mnDimension;
                if (nDimension == rFormatEntry.nDimension)
                {
                    if (rFormatEntry.bSet)
                    {
                        if (nDimension == -2 && rFieldData.nIndex != rFormatEntry.nIndex)
                            bMatchesAll = false;
                        else if (nDimension != -2 && rFieldData.aName != rFormatEntry.aName)
                            bMatchesAll = false;
                    }
                }
                else
                {
                    bMatchesAll = false;
                }
            }

            if (bMatchesAll)
            {
                if (rLineData.oLine && rLineData.oPosition
                    && rOutputEntry.eType == FormatType::Label)
                    rDocument.ApplyPattern(*rLineData.oPosition, *rLineData.oLine,
                                           *rOutputEntry.onTab, *rOutputEntry.pPattern);
                else if (rOutputEntry.eType == FormatType::Data)
                    oRow = rLineData.oLine;
                break;
            }
        }

        for (LineData const& rLineData : maColumnLines)
        {
            bool bMatchesAll = true;
            for (size_t nIndex = 0; nIndex < rLineData.maFields.size(); nIndex++)
            {
                FieldData const& rFieldData = rLineData.maFields[nIndex];
                FormatOutputField const& rFormatEntry = rOutputEntry.aColumnOutputFields[nIndex];

                tools::Long nDimension = rFieldData.mnDimension;
                if (nDimension == rFormatEntry.nDimension)
                {
                    if (rFormatEntry.bSet)
                    {
                        if (nDimension == -2 && rFieldData.nIndex != rFormatEntry.nIndex)
                            bMatchesAll = false;
                        else if (nDimension != -2 && rFieldData.aName != rFormatEntry.aName)
                            bMatchesAll = false;
                    }
                }
                else
                {
                    bMatchesAll = false;
                }
            }
            if (bMatchesAll)
            {
                if (rLineData.oLine && rLineData.oPosition
                    && rOutputEntry.eType == FormatType::Label)
                    rDocument.ApplyPattern(*rLineData.oLine, *rLineData.oPosition,
                                           *rOutputEntry.onTab, *rOutputEntry.pPattern);
                else if (rOutputEntry.eType == FormatType::Data)
                    oColumn = rLineData.oLine;
                break;
            }
        }
        if (oColumn && oRow && rOutputEntry.eType == FormatType::Data)
        {
            rDocument.ApplyPattern(*oColumn, *oRow, *rOutputEntry.onTab, *rOutputEntry.pPattern);
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
