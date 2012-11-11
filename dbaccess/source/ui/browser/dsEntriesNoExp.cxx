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


#include "unodatbr.hxx"
#include "browserids.hxx"
#include "listviewitems.hxx"
#include "imageprovider.hxx"
#include <osl/diagnose.h>
#include "dbtreeview.hxx"
#include "dbtreelistbox.hxx"
#include "dbu_brw.hrc"
#include "dbtreemodel.hxx"

using namespace ::com::sun::star::frame;
using namespace ::dbtools;
using namespace ::svx;

// .........................................................................
namespace dbaui
{
// .........................................................................
// -----------------------------------------------------------------------------
SbaTableQueryBrowser::EntryType SbaTableQueryBrowser::getChildType( SvTreeListEntry* _pEntry ) const
{
    OSL_ENSURE(isContainer(_pEntry), "SbaTableQueryBrowser::getChildType: invalid entry!");
    switch (getEntryType(_pEntry))
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

// -----------------------------------------------------------------------------
String SbaTableQueryBrowser::GetEntryText( SvTreeListEntry* _pEntry ) const
{
    return m_pTreeView->getListBox().GetEntryText(_pEntry);
}

// -----------------------------------------------------------------------------
SbaTableQueryBrowser::EntryType SbaTableQueryBrowser::getEntryType( const SvTreeListEntry* _pEntry ) const
{
    if (!_pEntry)
        return etUnknown;

    SvTreeListEntry* pRootEntry     = m_pTreeView->getListBox().GetRootLevelParent(const_cast<SvTreeListEntry*>(_pEntry));
    SvTreeListEntry* pEntryParent   = m_pTreeView->getListBox().GetParent(const_cast<SvTreeListEntry*>(_pEntry));
    SvTreeListEntry* pTables        = m_pTreeView->getListBox().GetEntry(pRootEntry, CONTAINER_TABLES);
    SvTreeListEntry* pQueries       = m_pTreeView->getListBox().GetEntry(pRootEntry, CONTAINER_QUERIES);

#ifdef DBG_UTIL
    String sTest;
    if (pTables) sTest = m_pTreeView->getListBox().GetEntryText(pTables);
    if (pQueries) sTest = m_pTreeView->getListBox().GetEntryText(pQueries);
#endif

    if (pRootEntry == _pEntry)
        return etDatasource;

    if (pTables == _pEntry)
        return etTableContainer;

    if (pQueries == _pEntry)
        return etQueryContainer;

    if (pTables == pEntryParent)
        return etTableOrView;

    if (pQueries == pEntryParent)
    {
        DBTreeListUserData* pEntryData = static_cast<DBTreeListUserData*>(_pEntry->GetUserData());
        if ( pEntryData )
            return pEntryData->eType;

        return etQuery;
    }
    while( pEntryParent != pQueries )
    {
        pEntryParent = m_pTreeView->getListBox().GetParent(pEntryParent);
        if ( !pEntryParent )
            return etUnknown;
    }

    return etQueryContainer;
}
//------------------------------------------------------------------------------
void SbaTableQueryBrowser::select(SvTreeListEntry* _pEntry, sal_Bool _bSelect)
{
    SvLBoxItem* pTextItem = _pEntry ? _pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING) : NULL;
    if (pTextItem)
    {
        static_cast<OBoldListboxString*>(pTextItem)->emphasize(_bSelect);
        m_pTreeModel->InvalidateEntry(_pEntry);
    }
    else {
        OSL_FAIL("SbaTableQueryBrowser::select: invalid entry!");
    }
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::selectPath(SvTreeListEntry* _pEntry, sal_Bool _bSelect)
{
    while (_pEntry)
    {
        select(_pEntry, _bSelect);
        _pEntry = m_pTreeModel->GetParent(_pEntry);
    }
}
//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::isSelected(SvTreeListEntry* _pEntry) const
{
    SvLBoxItem* pTextItem = _pEntry ? _pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING) : NULL;
    if (pTextItem)
        return static_cast<OBoldListboxString*>(pTextItem)->isEmphasized();
    else {
        OSL_FAIL("SbaTableQueryBrowser::isSelected: invalid entry!");
    }
    return sal_False;
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void SbaTableQueryBrowser::describeSupportedFeatures()
{
    SbaXDataBrowserController::describeSupportedFeatures();

    implDescribeSupportedFeature( ".uno:Title",                             ID_BROWSER_TITLE );
    if ( !m_bShowMenu )
    {
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

// -----------------------------------------------------------------------------
sal_Int32 SbaTableQueryBrowser::getDatabaseObjectType( EntryType _eType )
{
    switch ( _eType )
    {
    case etQuery:
    case etQueryContainer:
        return DatabaseObject::QUERY;
    case etTableOrView:
    case etTableContainer:
        return DatabaseObject::TABLE;
    default:
        break;
    }
    OSL_FAIL( "SbaTableQueryBrowser::getDatabaseObjectType: folder types and 'Unknown' not allowed here!" );
    return DatabaseObject::TABLE;
}

// -----------------------------------------------------------------------------
void SbaTableQueryBrowser::notifyHiContrastChanged()
{
    if ( m_pTreeView )
    {
        // change all bitmap entries
        SvTreeListEntry* pEntryLoop = m_pTreeModel->First();
        while ( pEntryLoop )
        {
            DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pEntryLoop->GetUserData());
            if ( !pData )
            {
                pEntryLoop = m_pTreeModel->Next(pEntryLoop);
                continue;
            }

            // the connection to which this entry belongs, if any
            ::std::auto_ptr< ImageProvider > pImageProvider( getImageProviderFor( pEntryLoop ) );

            // the images for this entry
            Image aImage;
            if ( pData->eType == etDatasource )
                aImage = pImageProvider->getDatabaseImage();
            else
            {
                bool bIsFolder = !isObject( pData->eType );
                if ( bIsFolder )
                {
                    sal_Int32 nObjectType( getDatabaseObjectType( pData->eType ) );
                    aImage = pImageProvider->getFolderImage( nObjectType );
                }
                else
                {
                    sal_Int32 nObjectType( getDatabaseObjectType( pData->eType ) );
                    pImageProvider->getImages( GetEntryText( pEntryLoop ), nObjectType, aImage );
                }
            }

            // find the proper item, and set its icons
            sal_uInt16 nCount = pEntryLoop->ItemCount();
            for (sal_uInt16 i=0;i<nCount;++i)
            {
                SvLBoxItem* pItem = pEntryLoop->GetItem(i);
                if ( !pItem || ( pItem->IsA() != SV_ITEM_ID_LBOXCONTEXTBMP ) )
                    continue;

                SvLBoxContextBmp* pContextBitmapItem = static_cast< SvLBoxContextBmp* >( pItem );

                pContextBitmapItem->SetBitmap1( aImage );
                pContextBitmapItem->SetBitmap2( aImage );
                break;
            }

            pEntryLoop = m_pTreeModel->Next(pEntryLoop);
        }
    }
}
// -----------------------------------------------------------------------------
// .........................................................................
}   // namespace dbaui
// .........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
