/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_FILER_XML_PIVOTSOURCE_HXX
#define SC_FILER_XML_PIVOTSOURCE_HXX

#include <dpshttab.hxx>
#include <dpsdbtab.hxx>
#include <dpobject.hxx>

#include <vector>
#include <boost/unordered_map.hpp>

namespace sc {

/**
 * Store pivot table data that need to be post-processeed at the end of the
 * import.
 */
struct PivotTableSources
{
    typedef boost::unordered_map<OUString, OUString, OUStringHash> SelectedPagesType;
    typedef boost::unordered_map<ScDPObject*, SelectedPagesType> SelectedPagesMapType;

    struct SelectedPages
    {
        ScDPObject* mpDP;
        SelectedPagesType maSelectedPages;

        SelectedPages( ScDPObject* pObj, const SelectedPagesType& rSelected );
    };

    struct SheetSource
    {
        ScDPObject* mpDP;
        ScSheetSourceDesc maDesc;

        SheetSource( ScDPObject* pObj, const ScSheetSourceDesc& rDesc );
    };

    struct DBSource
    {
        ScDPObject* mpDP;
        ScImportSourceDesc maDesc;

        DBSource( ScDPObject* pObj, const ScImportSourceDesc& rDesc );
    };

    struct ServiceSource
    {
        ScDPObject* mpDP;
        ScDPServiceDesc maDesc;

        ServiceSource( ScDPObject* pObj, const ScDPServiceDesc& rDesc );
    };

    typedef std::vector<SelectedPages> SelectedPagesListType;

    typedef std::vector<SheetSource>    SheetSourcesType;
    typedef std::vector<DBSource>       DBSourcesType;
    typedef std::vector<ServiceSource>  ServiceSourcesType;

    SelectedPagesListType maSelectedPagesList;

    SheetSourcesType    maSheetSources;
    DBSourcesType       maDBSources;
    ServiceSourcesType  maServiceSources;

    PivotTableSources();

    void appendSheetSource( ScDPObject* pObj, const ScSheetSourceDesc& rDesc );
    void appendDBSource( ScDPObject* pObj, const ScImportSourceDesc& rDesc );
    void appendServiceSource( ScDPObject* pObj, const ScDPServiceDesc& rDesc );

    void appendSelectedPages( ScDPObject* pObj, const SelectedPagesType& rSelected );

    void process();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
