/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <NamedSheetViewFragment.hxx>

#include <biffhelper.hxx>
#include <richstringcontext.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/helper/attributelist.hxx>
#include <autofiltercontext.hxx>

namespace oox::xls::nsv
{
using namespace ::oox::core;

FilterContext::FilterContext(WorksheetContextBase& rParent, FilterData& rFilterData)
    : WorksheetContextBase(rParent)
    , mrFilterData(rFilterData)
{
}

ContextHandlerRef FilterContext::onCreateContext(sal_Int32 nElement,
                                                 const AttributeList& /*rAttribs*/)
{
    switch (nElement)
    {
        case XLS_TOKEN(filters):
        {
            mrFilterData.mxSettings = std::make_shared<DiscreteFilter>(*this);
            return new FilterSettingsContext(*this, *mrFilterData.mxSettings);
        }
        case XLS_TOKEN(top10):
        {
            mrFilterData.mxSettings = std::make_shared<Top10Filter>(*this);
            return new FilterSettingsContext(*this, *mrFilterData.mxSettings);
        }
        case XLS_TOKEN(customFilters):
        {
            mrFilterData.mxSettings = std::make_shared<CustomFilter>(*this);
            return new FilterSettingsContext(*this, *mrFilterData.mxSettings);
        }
        case XLS_TOKEN(colorFilter):
        {
            mrFilterData.mxSettings = std::make_shared<ColorFilter>(*this);
            return new FilterSettingsContext(*this, *mrFilterData.mxSettings);
        }
    }

    return nullptr;
}

ColumnFilterContext::ColumnFilterContext(WorksheetContextBase& rParent,
                                         ColumnFilterData& rColumnFilterData)
    : WorksheetContextBase(rParent)
    , mrColumnFilterData(rColumnFilterData)
{
}

ContextHandlerRef ColumnFilterContext::onCreateContext(sal_Int32 nElement,
                                                       const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case XNSV_TOKEN(filter):
        {
            auto& rFilter = mrColumnFilterData.maFilters.emplace_back();
            rFilter.maColumnID = rAttribs.getInteger(XML_colId, 0);
            return new FilterContext(*this, rFilter);
        }
    }

    return nullptr;
}

SortRuleContext::SortRuleContext(WorksheetContextBase& rParent, SortRuleData& rSortRuleData)
    : WorksheetContextBase(rParent)
    , mrSortRuleData(rSortRuleData)
{
}

ContextHandlerRef SortRuleContext::onCreateContext(sal_Int32 nElement,
                                                   const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case XLS14_TOKEN(sortCondition):
            mrSortRuleData.maRef = rAttribs.getString(XML_ref, {}); // required
            mrSortRuleData.mbDescending = rAttribs.getBool(XML_descending, false); // optional
            mrSortRuleData.maSortBy = rAttribs.getToken(XML_sortBy, XML_value); // optional
            return this;
    }

    return nullptr;
}

NsvFilterContext::NsvFilterContext(WorksheetContextBase& rParent, NsvFilterData& rNsvFilterData)
    : WorksheetContextBase(rParent)
    , mrNsvFilterData(rNsvFilterData)
{
}

ContextHandlerRef NsvFilterContext::onCreateContext(sal_Int32 nElement,
                                                    const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case XNSV_TOKEN(columnFilter):
        {
            auto& rColumnFilter = mrNsvFilterData.maColumnFilters.emplace_back();
            rColumnFilter.maColumnID = rAttribs.getInteger(XML_colId, 0); // required
            rColumnFilter.maID = rAttribs.getString(XML_id, {}); // optional
            return new ColumnFilterContext(*this, rColumnFilter);
        }
        case XNSV_TOKEN(sortRules):
        {
            mrNsvFilterData.maSortRules.reset();
            mrNsvFilterData.maSortRules->mnMethod = rAttribs.getToken(XML_sortMethod, XML_none);
            mrNsvFilterData.maSortRules->mbCaseSensitive
                = rAttribs.getBool(XML_caseSensitive, false);
            return this;
        }
        case XNSV_TOKEN(sortRule):
        {
            if (mrNsvFilterData.maSortRules)
            {
                auto& rSortRules = *mrNsvFilterData.maSortRules;

                auto& rSortRule = rSortRules.maRules.emplace_back();
                SAL_WARN_IF(rSortRules.maRules.size() > 64, "sc",
                            "Max number of sort rules according to schema is 64!");
                rSortRule.maColumnID = rAttribs.getInteger(XML_colId, 0); // required
                rSortRule.maID = rAttribs.getString(XML_id, {}); // optional
                return new SortRuleContext(*this, rSortRule);
            }
        }
    }

    return nullptr;
}

NamedSheetViewContext::NamedSheetViewContext(WorksheetFragmentBase& rParent,
                                             NamedSheetViewData& rNamedSheetViewData)
    : WorksheetContextBase(rParent)
    , mrNamedSheetViewData(rNamedSheetViewData)
{
}

ContextHandlerRef NamedSheetViewContext::onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case XNSV_TOKEN(namedSheetView):
        {
            mrNamedSheetViewData.maName = rAttribs.getString(XML_name, {});
            mrNamedSheetViewData.maID = rAttribs.getString(XML_id, {});
            return this;
        }
        case XNSV_TOKEN(nsvFilter):
        {
            auto& rNsvFilter = mrNamedSheetViewData.maNsvFilters.emplace_back();
            rNsvFilter.maFilterID = rAttribs.getString(XML_filterId, {});
            rNsvFilter.maRef = rAttribs.getString(XML_ref, {});
            rNsvFilter.maTableID = rAttribs.getInteger(XML_tableId);
            return new NsvFilterContext(*this, rNsvFilter);
        }
        default:
            break;
    }

    return nullptr;
}

NamedSheetViewFragment::NamedSheetViewFragment(WorksheetHelper const& rHelper,
                                               OUString const& rFragmentPath)
    : WorksheetFragmentBase(rHelper, rFragmentPath)
{
}

ContextHandlerRef NamedSheetViewFragment::onCreateContext(sal_Int32 nElement,
                                                          AttributeList const& /*rAttribs*/)
{
    switch (nElement)
    {
        case XNSV_TOKEN(namedSheetViews):
        {
            return new NamedSheetViewContext(*this, maNamedSheetViews.emplace_back());
        }
    }
    return nullptr;
}

} // namespace oox::xls::nsv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
