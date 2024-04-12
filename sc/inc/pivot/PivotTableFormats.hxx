/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <vector>
#include <rtl/ustring.hxx>
#include <patattr.hxx>
#include <memory>

namespace sc
{
/** Type of a pivot table cell format to which a selection can be made. */
enum class FormatType
{
    None,
    Data,
    Label
};

/** Information to make a selection in the pivot table. */
struct Selection
{
    bool bSelected = false;
    sal_Int32 nField = 0;
    sal_uInt32 nDataIndex = 0;
};

/** Holds cell patter attributes and a selection information to which cells in the pivot table
 *  the pattern should be applied.
 */
struct PivotTableFormat
{
    FormatType eType = FormatType::None;

    bool bDataOnly = true;
    bool bLabelOnly = false;
    bool bSelected = false;
    bool bOutline = false;
    std::optional<sal_uInt32> oFieldPosition = std::nullopt;

    std::vector<Selection> aSelections;
    std::shared_ptr<ScPatternAttr> pPattern;

    std::vector<Selection> const& getSelections() const { return aSelections; }
};

/** A holder for a collection of PivotTableFormat */
class PivotTableFormats
{
    std::vector<PivotTableFormat> maFormats;

public:
    void add(PivotTableFormat const& rPivotTableFormat) { maFormats.push_back(rPivotTableFormat); }

    size_t size() const { return maFormats.size(); }

    std::vector<PivotTableFormat> const& getVector() const { return maFormats; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
