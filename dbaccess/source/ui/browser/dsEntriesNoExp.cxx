/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <memory>

#include <unodatbr.hxx>
#include <browserids.hxx>
#include <imageprovider.hxx>
#include <osl/diagnose.h>
#include "dbtreeview.hxx"
#include <dbtreelistbox.hxx>
#include "dbtreemodel.hxx"

using namespace ::com::sun::star::frame;
using namespace ::dbtools;
using namespace ::svx;

namespace dbaui
{
SbaTableQueryBrowser::EntryType SbaTableQueryBrowser::getChildType(const weld::TreeIter& rEntry) const
{
    OSL_ENSURE(isContainer(rEntry), "SbaTableQueryBrowser::getChildType: invalid entry!");
    switch (getEntryType(rEntry))
    {
        case etTableContainer:
            return etTableOrView;
        case etQueryContainer:
            return etQuery;
        default:
            break;
    }
    return etUnknown;
}

OUString SbaTableQueryBrowser::GetEntryText(const weld::TreeIter& rEntry) const
{
    return m_pTreeView->getListBox().GetWidget().get_text(rEntry);
}

SbaTableQueryBrowser::EntryType SbaTableQueryBrowser::getEntryType(const weld::TreeIter& rEntry) const
{
    std::unique_ptr<weld::TreeIter> xRootEntry = m_pTreeView->getListBox().GetRootLevelParent(&rEntry);
    weld::TreeView& rTreeView = m_pTreeView->getListBox().GetWidget();

    if (rTreeView.iter_compare(*xRootEntry, rEntry) == 0)
        return etDatasource;

    std::unique_ptr<weld::TreeIter> xEntryParent(rTreeView.make_iterator(&rEntry));
    if (!rTreeView.iter_parent(*xEntryParent))
        xEntryParent.reset();

    std::unique_ptr<weld::TreeIter> xTables;
    std::unique_ptr<weld::TreeIter> xQueries;

    std::unique_ptr<weld::TreeIter> xContainer = rTreeView.make_iterator(xRootEntry.get());
    if (rTreeView.iter_children(*xContainer))
    {
        // 1st child is queries
        xQueries = rTreeView.make_iterator(xContainer.get());

        if (rTreeView.iter_next_sibling(*xContainer))
        {
            // 2nd child is tables
            xTables = rTreeView.make_iterator(xContainer.get());
        }
    }

    if (xTables && rTreeView.iter_compare(*xTables, rEntry) == 0)
        return etTableContainer;

    if (xQueries && rTreeView.iter_compare(*xQueries, rEntry) == 0)
        return etQueryContainer;

    if (xTables && xEntryParent && rTreeView.iter_compare(*xTables, *xEntryParent) == 0)
        return etTableOrView;

    if (xQueries && xEntryParent)
    {
        if (rTreeView.iter_compare(*xQueries, *xEntryParent) == 0)
        {
            DBTreeListUserData* pEntryData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(rEntry).toUInt64());
            if (pEntryData)
                return pEntryData->eType;
            return etQuery;
        }

        while (rTreeView.iter_compare(*xEntryParent, *xQueries) != 0)
        {
            if (!rTreeView.iter_parent(*xEntryParent))
                return etUnknown;
        }
    }

    return etQueryContainer;
}

void SbaTableQueryBrowser::select(weld::TreeIter* pEntry, bool bSelect)
{
    if (pEntry)
    {
        weld::TreeView& rTreeView = m_pTreeView->getListBox().GetWidget();
        rTreeView.set_text_emphasis(*pEntry, bSelect, 0);
    }
    else
    {
        OSL_FAIL("SbaTableQueryBrowser::select: invalid entry!");
    }
}

void SbaTableQueryBrowser::selectPath(weld::TreeIter* pEntry, bool bSelect)
{
    if (!pEntry)
        return;
    weld::TreeView& rTreeView = m_pTreeView->getListBox().GetWidget();
    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator(pEntry));
    do
    {
        select(xEntry.get(), bSelect);
    }
    while (rTreeView.iter_parent(*xEntry));
}

bool SbaTableQueryBrowser::isSelected(const weld::TreeIter& rEntry) const
{
    weld::TreeView& rTreeView = m_pTreeView->getListBox().GetWidget();
    return rTreeView.get_text_emphasis(rEntry, 0);
}

void SbaTableQueryBrowser::SelectionChanged()
{
    if ( !m_bShowMenu )
    {
        InvalidateFeature(ID_BROWSER_INSERTCOLUMNS);
        InvalidateFeature(ID_BROWSER_INSERTCONTENT);
        InvalidateFeature(ID_BROWSER_FORMLETTER);
    }
    InvalidateFeature(ID_BROWSER_COPY);
    InvalidateFeature(ID_BROWSER_CUT);
}

void SbaTableQueryBrowser::describeSupportedFeatures()
{
    SbaXDataBrowserController::describeSupportedFeatures();

    implDescribeSupportedFeature( ".uno:Title",                             ID_BROWSER_TITLE );
    if ( !m_bShowMenu )
    {
        implDescribeSupportedFeature( ".uno:DSBEditDB",          ID_TREE_EDIT_DATABASE );
        implDescribeSupportedFeature( ".uno:DSBCloseConnection", ID_TREE_CLOSE_CONN );
        implDescribeSupportedFeature( ".uno:DSBAdministrate",    ID_TREE_ADMINISTRATE );

        implDescribeSupportedFeature( ".uno:DSBrowserExplorer",                 ID_BROWSER_EXPLORER, CommandGroup::VIEW );

        implDescribeSupportedFeature( ".uno:DSBFormLetter",                     ID_BROWSER_FORMLETTER, CommandGroup::DOCUMENT );
        implDescribeSupportedFeature( ".uno:DSBInsertColumns",                  ID_BROWSER_INSERTCOLUMNS, CommandGroup::INSERT );
        implDescribeSupportedFeature( ".uno:DSBInsertContent",                  ID_BROWSER_INSERTCONTENT, CommandGroup::INSERT );
        implDescribeSupportedFeature( ".uno:DSBDocumentDataSource",             ID_BROWSER_DOCUMENT_DATASOURCE, CommandGroup::VIEW );

        implDescribeSupportedFeature( ".uno:DataSourceBrowser/FormLetter",          ID_BROWSER_FORMLETTER );
        implDescribeSupportedFeature( ".uno:DataSourceBrowser/InsertColumns",       ID_BROWSER_INSERTCOLUMNS );
        implDescribeSupportedFeature( ".uno:DataSourceBrowser/InsertContent",       ID_BROWSER_INSERTCONTENT );
        implDescribeSupportedFeature( ".uno:DataSourceBrowser/DocumentDataSource",  ID_BROWSER_DOCUMENT_DATASOURCE );
    }

    implDescribeSupportedFeature( ".uno:CloseWin",      ID_BROWSER_CLOSE, CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:DBRebuildData", ID_BROWSER_REFRESH_REBUILD, CommandGroup::DATA );
}

sal_Int32 SbaTableQueryBrowser::getDatabaseObjectType( EntryType _eType )
{
    switch ( _eType )
    {
    case etQuery:
    case etQueryContainer:
        return css::sdb::application::DatabaseObject::QUERY;
    case etTableOrView:
    case etTableContainer:
        return css::sdb::application::DatabaseObject::TABLE;
    default:
        break;
    }
    OSL_FAIL( "SbaTableQueryBrowser::getDatabaseObjectType: folder types and 'Unknown' not allowed here!" );
    return css::sdb::application::DatabaseObject::TABLE;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
