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
enum class FormatResultDirection
{
    ROW,
    COLUMN
};

struct FormatOutputField
{
    tools::Long nDimension = -2;
    OUString aName;
    sal_Int32 nIndex = -1;
    bool bSet = false;
};

struct FormatOutputEntry
{
    FormatType eType = FormatType::None;
    std::optional<SCTAB> onTab = std::nullopt;
    std::shared_ptr<ScPatternAttr> pPattern;

    std::vector<FormatOutputField> aRowOutputFields;
    std::vector<FormatOutputField> aColumnOutputFields;
};

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

struct LineData
{
    std::optional<SCCOLROW> oLine = std::nullopt;
    std::optional<SCCOLROW> oPosition = std::nullopt;
    std::vector<FieldData> maFields;
};

class NameResolver;

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
        maFormatOutputEntries.resize(mpFormats->size());
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
