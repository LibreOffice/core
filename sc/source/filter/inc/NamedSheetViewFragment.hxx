/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "NamedSheetViewFragment.hxx"
#include "excelhandlers.hxx"
#include "autofilterbuffer.hxx"

#include <oox/token/tokens.hxx>

namespace oox::xls::nsv
{
// Schema for NamedSheetViews :
// https://learn.microsoft.com/ga-ie/openspecs/office_standards/ms-xlsx/55396aef-0c07-4ffb-9ffb-e48b6d339abe

struct FilterData
{
    sal_Int32 maColumnID; // required
    std::shared_ptr<FilterSettingsBase> mxSettings;
};

struct ColumnFilterData
{
    sal_Int32 maColumnID; // required
    OUString maID; // GUID, optional
    std::vector<FilterData> maFilters; // min 0, max unbounded
};

struct SortRuleData
{
    sal_Int32 maColumnID; // required
    OUString maID; // GUID, optional

    // Part of <sortCondition> (x14:CT_SortCondition)
    bool mbDescending = false; // optional, default false
    sal_Int32 maSortBy = XML_value; // optional, default "value"
    OUString maRef; // required

    // customList, x:ST_Xstring, optional
    // dxfId, x:ST_DxfId, optional
    // iconSet, ST_IconSetType, optional, default "3Arrows"
    // iconId, sal_uInt32, optional
};

struct SortRulesData
{
    bool mbCaseSensitive = false; // optional, default false
    sal_Int32 mnMethod = XML_none; // optional, default none
    std::vector<SortRuleData> maRules; // min 0, max 64
};

struct NsvFilterData
{
    OUString maFilterID; // GUID, required
    OUString maRef; // optional
    std::optional<sal_uInt32> maTableID; // optional
    std::vector<ColumnFilterData> maColumnFilters; // min 0, max unbounded
    std::optional<SortRulesData> maSortRules; // optional
};

struct NamedSheetViewData
{
    OUString maName; // required
    OUString maID; // GUID, required
    std::vector<NsvFilterData> maNsvFilters; // min 0, max unbounded
};

/** Handles parsing of one <filter> element and sub-elements */
class FilterContext final : public WorksheetContextBase
{
    FilterData& mrFilterData;

public:
    explicit FilterContext(WorksheetContextBase& rParent, FilterData& rFilterData);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs) override;
};

/** Handles parsing of one <columnFilter> element and sub-elements */
class ColumnFilterContext final : public WorksheetContextBase
{
    ColumnFilterData& mrColumnFilterData;

public:
    explicit ColumnFilterContext(WorksheetContextBase& rParent,
                                 ColumnFilterData& rColumnFilterData);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs) override;
};

/** Handles parsing of one <sortRule> element and sub-elements */
class SortRuleContext final : public WorksheetContextBase
{
    SortRuleData& mrSortRuleData;

public:
    explicit SortRuleContext(WorksheetContextBase& rParent, SortRuleData& rSortRuleData);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs) override;
};

/** Handles parsing of one <nsvFilter> element and sub-elements */
class NsvFilterContext final : public WorksheetContextBase
{
    NsvFilterData& mrNsvFilterData;

public:
    explicit NsvFilterContext(WorksheetContextBase& rParent, NsvFilterData& rNsvFilterData);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs) override;
};

/** Handles parsing of one <namedSheetView> element and sub-elements */
class NamedSheetViewContext final : public WorksheetContextBase
{
    NamedSheetViewData& mrNamedSheetViewData;

public:
    explicit NamedSheetViewContext(WorksheetFragmentBase& rParent,
                                   NamedSheetViewData& rNamedSheetViewData);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs) override;
};

/** Handles parsing of named sheet view data contained in namedSheetView*.xml file */
class NamedSheetViewFragment final : public WorksheetFragmentBase
{
    std::vector<NamedSheetViewData> maNamedSheetViews;

public:
    explicit NamedSheetViewFragment(WorksheetHelper const& rHelper, OUString const& rFragmentPath);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         AttributeList const& rAttribs) override;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
