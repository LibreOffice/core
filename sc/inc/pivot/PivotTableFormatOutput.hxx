/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/uno/Sequence.hxx>
#include <tools/long.hxx>
#include <address.hxx>
#include <dptypes.hxx>
#include "PivotTableFormats.hxx"

#include <memory>
#include <vector>

namespace com::sun::star::sheet
{
struct MemberResult;
}

class ScDPObject;
struct ScDPOutLevelData;

namespace sc
{
/// Direction or orientation of the results
enum class FormatResultDirection
{
    ROW,
    COLUMN
};

/// A field data for a format output,
struct FormatOutputField
{
    tools::Long nDimension = -2;
    OUString aName;
    sal_Int32 nIndex = -1;
    bool bMatchesAll = false;
    bool bSet = false;
};

/// Data for a entry of a format output, which matches a one format entry
struct FormatOutputEntry
{
    FormatType eType = FormatType::None;
    std::optional<SCTAB> onTab = std::nullopt;
    std::shared_ptr<ScPatternAttr> pPattern;

    std::vector<FormatOutputField> aRowOutputFields;
    std::vector<FormatOutputField> aColumnOutputFields;
};

/// data of one "field" that is part of a line
struct FieldData
{
    tools::Long mnDimension = -2;
    OUString aName;
    tools::Long nIndex;

    bool bIsSet = false;
    bool bIsMember = false;
    bool bSubtotal = false;
    bool bContinue = false;
};

/// data for a "line" of one row/column axis
struct LineData
{
    /// primary axis depending if we are row or column oriented (when selecting a column or row we get a "line")
    std::optional<SCCOLROW> oLine = std::nullopt;
    /// secondary axis depending if we are row or column oriented (position in a line)
    std::optional<SCCOLROW> oPosition = std::nullopt;
    /// fields of a line
    std::vector<FieldData> maFields;
};

/** Format output class, which is responsible to check if any of the formats and matches the
 *  pivot table output, and applies the cell attributes when the match is found. */
class FormatOutput
{
private:
    ScDPObject& mrObject;

public:
    FormatOutput(ScDPObject& rObject)
        : mrObject(rObject)
    {
    }

    std::unique_ptr<sc::PivotTableFormats> mpFormats;
    std::vector<sc::FormatOutputEntry> maFormatOutputEntries;

    std::vector<LineData> maRowLines;
    std::vector<LineData> maColumnLines;

    void setFormats(sc::PivotTableFormats const& rPivotTableFormats)
    {
        mpFormats.reset(new sc::PivotTableFormats(rPivotTableFormats));
    }

    void insertFieldMember(size_t nFieldIndex, const ScDPOutLevelData& rField,
                           tools::Long nMemberIndex, css::sheet::MemberResult const& rMember,
                           SCCOL nColPos, SCROW nRowPos, FormatResultDirection eResultDirection);

    void insertEmptyDataColumn(SCCOL nColPos, SCROW nRowPos);

    void apply(ScDocument& rDocument);
    void prepare(SCTAB nTab, std::vector<ScDPOutLevelData> const& rColumnFields,
                 std::vector<ScDPOutLevelData> const& rRowFields, bool bColumnFieldIsDataOnly);
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
