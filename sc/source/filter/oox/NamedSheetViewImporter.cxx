/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <NamedSheetViewImporter.hxx>

#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <addressconverter.hxx>
#include <autofilterbuffer.hxx>
#include <dbdata.hxx>
#include <document.hxx>
#include <queryparam.hxx>
#include <SheetView.hxx>
#include <SheetViewManager.hxx>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <dbdocfun.hxx>
#include <sortparam.hxx>

namespace oox::xls::nsv
{
using namespace ::com::sun::star;

NamedSheetViewImporter::NamedSheetViewImporter(const WorkbookHelper& rHelper, SCTAB nTab)
    : WorkbookHelper(rHelper)
    , mnTab(nTab)
{
}

void NamedSheetViewImporter::setNamedSheetViews(std::vector<NamedSheetViewData>&& rData)
{
    maNamedSheetViews = std::move(rData);
}

void NamedSheetViewImporter::finalizeImport()
{
    if (maNamedSheetViews.empty())
        return;

    ScDocument& rDoc = getScDocument();

    for (const auto& rViewData : maNamedSheetViews)
    {
        auto[nSheetViewID, nViewTab] = rDoc.CreateNewSheetView(mnTab);
        if (nSheetViewID == sc::InvalidSheetViewID)
            continue;

        // Set the imported name on the sheet view
        auto pSheetViewManager = rDoc.GetSheetViewManager(mnTab);
        if (!pSheetViewManager)
            continue;

        auto pSheetView = pSheetViewManager->get(nSheetViewID);
        if (pSheetView)
            pSheetView->SetName(rViewData.maName);

        // Apply filters and sort for each nsvFilter
        for (const auto& rNsvFilter : rViewData.maNsvFilters)
        {
            // Parse the filter range reference
            ScRange aFilterRange;
            if (rNsvFilter.maRef.isEmpty())
                continue;

            if (!getAddressConverter().convertToCellRange(aFilterRange, rNsvFilter.maRef, nViewTab,
                                                          true, true))
                continue;

            // Adjust the range to the view tab
            aFilterRange.aStart.SetTab(nViewTab);
            aFilterRange.aEnd.SetTab(nViewTab);

            // Build an AutoFilter from the NSV column filter data and apply it
            bool bHasColumnFilters
                = std::any_of(rNsvFilter.maColumnFilters.begin(), rNsvFilter.maColumnFilters.end(),
                              [](const auto& rCF) { return !rCF.maFilters.empty(); });

            if (bHasColumnFilters)
            {
                try
                {
                    AutoFilter aAutoFilter(*this);
                    aAutoFilter.setRange(aFilterRange);

                    for (const auto& rColFilter : rNsvFilter.maColumnFilters)
                    {
                        for (const auto& rFilter : rColFilter.maFilters)
                        {
                            if (rFilter.mxSettings)
                            {
                                FilterColumn& rFilterColumn = aAutoFilter.createFilterColumn();
                                rFilterColumn.setColumnData(rFilter.maColumnID, rFilter.mxSettings);
                            }
                        }
                    }

                    uno::Reference<css::sheet::XDatabaseRange> xDatabaseRange
                        = createUnnamedDatabaseRangeObject(aFilterRange);
                    if (xDatabaseRange.is())
                    {
                        PropertySet aRangeProps(xDatabaseRange);
                        aRangeProps.setProperty(PROP_AutoFilter, true);
                        aAutoFilter.finalizeImport(xDatabaseRange, nViewTab);

                        // Execute the filter query to actually hide rows
                        ScDBData* pDBData = rDoc.GetAnonymousDBData(nViewTab);
                        ScDocShell* pFilterDocShell = rDoc.GetDocumentShell();
                        if (pDBData && pFilterDocShell)
                        {
                            ScQueryParam aQueryParam;
                            pDBData->GetQueryParam(aQueryParam);
                            ScDBDocFunc aDBDocFunc(*pFilterDocShell);
                            aDBDocFunc.Query(nViewTab, aQueryParam, nullptr, false, false);
                        }
                    }
                }
                catch (const css::uno::Exception&)
                {
                    SAL_WARN("sc", "Failed to apply filter to named sheet view");
                }
            }

            // Apply sort rules if present
            if (rNsvFilter.maSortRules)
            {
                const auto& rSortRules = *rNsvFilter.maSortRules;
                if (!rSortRules.maRules.empty())
                {
                    ScSortParam aParam;
                    aParam.nCol1 = aFilterRange.aStart.Col();
                    aParam.nRow1 = aFilterRange.aStart.Row();
                    aParam.nCol2 = aFilterRange.aEnd.Col();
                    aParam.nRow2 = aFilterRange.aEnd.Row();
                    aParam.bHasHeader = true;
                    aParam.bByRow = true;
                    aParam.bCaseSens = rSortRules.mbCaseSensitive;

                    size_t nKeyIdx = 0;
                    for (const auto& rSortRule : rSortRules.maRules)
                    {
                        if (nKeyIdx >= aParam.GetSortKeyCount())
                            aParam.maKeyState.resize(nKeyIdx + 1);

                        // Parse the sort condition reference to get the column
                        ScRange aSortRange;
                        if (!rSortRule.maRef.isEmpty()
                            && getAddressConverter().convertToCellRange(aSortRange, rSortRule.maRef,
                                                                        nViewTab, true, true))
                        {
                            aParam.maKeyState[nKeyIdx].bDoSort = true;
                            aParam.maKeyState[nKeyIdx].bAscending = !rSortRule.mbDescending;
                            aParam.maKeyState[nKeyIdx].nField = aSortRange.aStart.Col();
                            ++nKeyIdx;
                        }
                    }

                    if (nKeyIdx > 0)
                    {
                        ScDocShell* pDocShell = rDoc.GetDocumentShell();
                        if (pDocShell)
                        {
                            ScDBDocFunc aFunc(*pDocShell);
                            aFunc.SortTab(nViewTab, aParam, false, false, true);
                        }
                    }
                }
            }
        }
    }
}

} // namespace oox::xls::nsv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
