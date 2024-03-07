/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "workbookhelper.hxx"

#include <memory>
#include <vector>
#include <optional>
#include <oox/helper/attributelist.hxx>

namespace oox::xls
{
class PivotTable;
class PivotTableFormat;
class PivotTableReference;

enum class PivotAreaType
{
    None,
    Normal,
    Data,
    All,
    Origin,
    Button,
    TopRight
};

class PivotTableFormat : public WorkbookHelper
{
public:
    // DXF
    sal_Int32 mnDxfId = -1;

private:
    // PivotArea
    std::optional<sal_Int32> mnField;
    PivotAreaType meType = PivotAreaType::Normal;
    bool mbDataOnly = true;
    bool mbLabelOnly = false;
    bool mbGrandRow = false;
    bool mbGrandCol = false;
    bool mbCacheIndex = false;
    bool mbOutline = true;
    std::optional<OUString> msOffset;
    bool mbCollapsedLevelsAreSubtotals = false;
    // TODO Axis
    std::optional<sal_uInt32> mnFieldPosition;

    std::vector<std::shared_ptr<PivotTableReference>> maReferences;

public:
    explicit PivotTableFormat(const PivotTable& rPivotTable);
    void importPivotArea(const oox::AttributeList& rAttribs);
    void importFormat(const oox::AttributeList& rAttribs);
    PivotTableReference& createReference();
};

class PivotTableReference : public WorkbookHelper
{
    //PivotTableFormat const& mrFormat;

    std::optional<sal_uInt32> mnField;
    std::optional<sal_uInt32> mnCount;
    bool mbSelected = true;
    bool mbByPosition = false;
    bool mbRelative = false;
    bool mbDefaultSubtotal = false;
    bool mbSumSubtotal = false;
    bool mbCountASubtotal = false;
    bool mbAvgSubtotal = false;
    bool mbMaxSubtotal = false;
    bool mbMinSubtotal = false;
    bool mbProductSubtotal = false;
    bool mbCountSubtotal = false;
    bool mbStdDevSubtotal = false;
    bool mbStdDevPSubtotal = false;
    bool mbVarSubtotal = false;
    bool mbVarPSubtotal = false;

    std::vector<sal_uInt32> maFieldItemsIndices;

public:
    explicit PivotTableReference(const PivotTableFormat& rFormat);

    void importReference(const oox::AttributeList& rAttribs);
    void addFieldItem(const oox::AttributeList& rAttribs);
};

typedef std::vector<std::shared_ptr<PivotTableFormat>> PivotTableFormatVector;

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
