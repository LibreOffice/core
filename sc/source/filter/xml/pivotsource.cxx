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

PivotTableSources::SelectedPages::SelectedPages( ScDPObject* pObj, const SelectedPagesType& rSelected ) :
    mpDP(pObj), maSelectedPages(rSelected) {}

PivotTableSources::SheetSource::SheetSource( ScDPObject* pObj, const ScSheetSourceDesc& rDesc ) :
    mpDP(pObj), maDesc(rDesc) {}

PivotTableSources::DBSource::DBSource( ScDPObject* pObj, const ScImportSourceDesc& rDesc ) :
    mpDP(pObj), maDesc(rDesc) {}

PivotTableSources::ServiceSource::ServiceSource( ScDPObject* pObj, const ScDPServiceDesc& rDesc ) :
    mpDP(pObj), maDesc(rDesc) {}

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

void PivotTableSources::appendSelectedPages( ScDPObject* pObj, const SelectedPagesType& rSelected )
{
    if (rSelected.empty())
        return;

    maSelectedPagesList.emplace_back(pObj, rSelected);
}

namespace {

struct SelectedPageProcessor
{
    void operator() ( PivotTableSources::SelectedPages& rItem )
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
};

struct PivotSheetDescSetter
{
    void operator() ( sc::PivotTableSources::SheetSource& rSrc )
    {
        ScDPObject* pObj = rSrc.mpDP;
        pObj->SetSheetDesc(rSrc.maDesc);
    }
};

struct PivotDBDescSetter
{
    void operator() ( sc::PivotTableSources::DBSource& rSrc )
    {
        ScDPObject* pObj = rSrc.mpDP;
        pObj->SetImportDesc(rSrc.maDesc);
    }
};

struct PivotServiceDataSetter
{
    void operator() ( sc::PivotTableSources::ServiceSource& rSrc )
    {
        ScDPObject* pObj = rSrc.mpDP;
        pObj->SetServiceData(rSrc.maDesc);
    }
};

}

void PivotTableSources::process()
{
    std::for_each(maSheetSources.begin(), maSheetSources.end(), PivotSheetDescSetter());
    std::for_each(maDBSources.begin(), maDBSources.end(), PivotDBDescSetter());
    std::for_each(maServiceSources.begin(), maServiceSources.end(), PivotServiceDataSetter());
    std::for_each(maSelectedPagesList.begin(), maSelectedPagesList.end(), SelectedPageProcessor());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
