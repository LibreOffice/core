/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <dpshttab.hxx>
#include <dpsdbtab.hxx>
#include <dpobject.hxx>

#include <unordered_map>
#include <vector>

namespace sc
{
/**
 * Store pivot table data that need to be post-processed at the end of the
 * import.
 */
struct PivotTableSources
{
    typedef std::unordered_map<OUString, OUString> SelectedPagesType;

    struct SelectedPages
    {
        ScDPObject* mpDP;
        SelectedPagesType maSelectedPages;

        SelectedPages(ScDPObject* pObj, const SelectedPagesType& rSelected);
    };

    struct SheetSource
    {
        ScDPObject* mpDP;
        ScSheetSourceDesc maDesc;

        SheetSource(ScDPObject* pObj, const ScSheetSourceDesc& rDesc);
    };

    struct DBSource
    {
        ScDPObject* mpDP;
        ScImportSourceDesc maDesc;

        DBSource(ScDPObject* pObj, const ScImportSourceDesc& rDesc);
    };

    struct ServiceSource
    {
        ScDPObject* mpDP;
        ScDPServiceDesc maDesc;

        ServiceSource(ScDPObject* pObj, const ScDPServiceDesc& rDesc);
    };

    std::vector<SelectedPages> maSelectedPagesList;
    std::vector<SheetSource> maSheetSources;
    std::vector<DBSource> maDBSources;
    std::vector<ServiceSource> maServiceSources;

    PivotTableSources();

    void appendSheetSource(ScDPObject* pObj, const ScSheetSourceDesc& rDesc);
    void appendDBSource(ScDPObject* pObj, const ScImportSourceDesc& rDesc);
    void appendServiceSource(ScDPObject* pObj, const ScDPServiceDesc& rDesc);

    void appendSelectedPages(ScDPObject* pObj, const SelectedPagesType& rSelected);

    void process();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
