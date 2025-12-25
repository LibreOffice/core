/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pivotsource.hxx"

#include <dpsave.hxx>

#include <algorithm>

namespace sc {

PivotTableSources::SelectedPages::SelectedPages( ScDPObject* pObj, SelectedPagesType&& rSelected ) :
    mpDP(pObj), maSelectedPages(std::move(rSelected)) {}

PivotTableSources::SheetSource::SheetSource( ScDPObject* pObj, ScSheetSourceDesc aDesc ) :
    mpDP(pObj), maDesc(std::move(aDesc)) {}

PivotTableSources::DBSource::DBSource( ScDPObject* pObj, ScImportSourceDesc aDesc ) :
    mpDP(pObj), maDesc(std::move(aDesc)) {}

PivotTableSources::ServiceSource::ServiceSource( ScDPObject* pObj, ScDPServiceDesc aDesc ) :
    mpDP(pObj), maDesc(std::move(aDesc)) {}

PivotTableSources::PivotTableSources() {}

void PivotTableSources::appendSheetSource( ScDPObject* pObj, const ScSheetSourceDesc& rDesc )
{
    maSheetSources.emplace_back(pObj, rDesc);
}

void PivotTableSources::appendDBSource( ScDPObject* pObj, const ScImportSourceDesc& rDesc )
{
    maDBSources.emplace_back(pObj, rDesc);
}

void PivotTableSources::appendServiceSource( ScDPObject* pObj, const ScDPServiceDesc& rDesc )
{
    maServiceSources.emplace_back(pObj, rDesc);
}

void PivotTableSources::appendSelectedPages( ScDPObject* pObj, SelectedPagesType&& rSelected )
{
    if (rSelected.empty())
        return;

    maSelectedPagesList.emplace_back(pObj, std::move(rSelected));
}

void PivotTableSources::process()
{
    for (SheetSource const& rSource : maSheetSources)
    {
        ScDPObject* pObj = rSource.mpDP;
        pObj->SetSheetDesc(rSource.maDesc);
    }

    for (DBSource const& rSource: maDBSources)
    {
        ScDPObject* pObj = rSource.mpDP;
        pObj->SetImportDesc(rSource.maDesc);
    }

    for (ServiceSource const& rSource: maServiceSources)
    {
        ScDPObject* pObj = rSource.mpDP;
        pObj->SetServiceData(rSource.maDesc);
    }

    for (SelectedPages const& rItem: maSelectedPagesList)
    {
        // Set selected pages after building all dimension members.
        if (!rItem.mpDP)
            return;

        rItem.mpDP->BuildAllDimensionMembers();
        ScDPSaveData* pSaveData = rItem.mpDP->GetSaveData();
        if (!pSaveData)
            return;

        for (const auto& [rDimName, rSelected] : rItem.maSelectedPages)
        {
            ScDPSaveDimension* pDim = pSaveData->GetExistingDimensionByName(rDimName);
            if (!pDim)
                continue;

            pDim->SetCurrentPage(&rSelected);
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
