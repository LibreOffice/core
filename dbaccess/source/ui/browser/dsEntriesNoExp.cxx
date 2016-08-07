/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"

#ifndef _SBA_UNODATBR_HXX_
#include "unodatbr.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
#ifndef DBACCESS_IMAGEPROVIDER_HXX
#include "imageprovider.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#include "dbtreeview.hxx"
#endif
#ifndef DBAUI_DBTREELISTBOX_HXX
#include "dbtreelistbox.hxx"
#endif
#ifndef _DBU_BRW_HRC_
#include "dbu_brw.hrc"
#endif
#ifndef DBAUI_DBTREEMODEL_HXX
#include "dbtreemodel.hxx"
#endif

using namespace ::com::sun::star::frame;
using namespace ::dbtools;
using namespace ::svx;

// .........................................................................
namespace dbaui
{
// .........................................................................
// -----------------------------------------------------------------------------
SbaTableQueryBrowser::EntryType SbaTableQueryBrowser::getChildType( SvLBoxEntry* _pEntry ) const
{
    DBG_ASSERT(isContainer(_pEntry), "SbaTableQueryBrowser::getChildType: invalid entry!");
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
String SbaTableQueryBrowser::GetEntryText( SvLBoxEntry* _pEntry ) const
{
    return m_pTreeView->getListBox().GetEntryText(_pEntry);
}

// -----------------------------------------------------------------------------
SbaTableQueryBrowser::EntryType SbaTableQueryBrowser::getEntryType( SvLBoxEntry* _pEntry ) const
{
    if (!_pEntry)
        return etUnknown;

    SvLBoxEntry* pRootEntry     = m_pTreeView->getListBox().GetRootLevelParent(_pEntry);
    SvLBoxEntry* pEntryParent   = m_pTreeView->getListBox().GetParent(_pEntry);
    SvLBoxEntry* pTables        = m_pTreeView->getListBox().GetEntry(pRootEntry, CONTAINER_TABLES);
    SvLBoxEntry* pQueries       = m_pTreeView->getListBox().GetEntry(pRootEntry, CONTAINER_QUERIES);

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
void SbaTableQueryBrowser::select(SvLBoxEntry* _pEntry, sal_Bool _bSelect)
{
    SvLBoxItem* pTextItem = _pEntry ? _pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING) : NULL;
    if (pTextItem)
    {
        static_cast<OBoldListboxString*>(pTextItem)->emphasize(_bSelect);
        m_pTreeModel->InvalidateEntry(_pEntry);
    }
    else {
        DBG_ERROR("SbaTableQueryBrowser::select: invalid entry!");
    }
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::selectPath(SvLBoxEntry* _pEntry, sal_Bool _bSelect)
{
    while (_pEntry)
    {
        select(_pEntry, _bSelect);
        _pEntry = m_pTreeModel->GetParent(_pEntry);
    }
}
//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::isSelected(SvLBoxEntry* _pEntry) const
{
    SvLBoxItem* pTextItem = _pEntry ? _pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING) : NULL;
    if (pTextItem)
        return static_cast<OBoldListboxString*>(pTextItem)->isEmphasized();
    else {
        DBG_ERROR("SbaTableQueryBrowser::isSelected: invalid entry!");
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
    } // if ( !m_bShowMenu )
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
    OSL_ENSURE( false, "SbaTableQueryBrowser::getDatabaseObjectType: folder types and 'Unknown' not allowed here!" );
    return DatabaseObject::TABLE;
}

// -----------------------------------------------------------------------------
void SbaTableQueryBrowser::notifyHiContrastChanged()
{
    if ( m_pTreeView )
    {
        // change all bitmap entries
        SvLBoxEntry* pEntryLoop = m_pTreeModel->First();
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
            Image aImage, aImageHC;
            if ( pData->eType == etDatasource )
            {
                aImage = pImageProvider->getDatabaseImage( false );
                aImageHC = pImageProvider->getDatabaseImage( true );
            }
            else
            {
                bool bIsFolder = !isObject( pData->eType );
                if ( bIsFolder )
                {
                    sal_Int32 nObjectType( getDatabaseObjectType( pData->eType ) );
                    aImage = pImageProvider->getFolderImage( nObjectType, false );
                    aImageHC = pImageProvider->getFolderImage( nObjectType, true );
                }
                else
                {
                    sal_Int32 nObjectType( getDatabaseObjectType( pData->eType ) );
                    pImageProvider->getImages( GetEntryText( pEntryLoop ), nObjectType, aImage, aImageHC );
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

                pContextBitmapItem->SetBitmap1( aImage, BMP_COLOR_NORMAL );
                pContextBitmapItem->SetBitmap2( aImage, BMP_COLOR_NORMAL );
                pContextBitmapItem->SetBitmap1( aImageHC, BMP_COLOR_HIGHCONTRAST );
                pContextBitmapItem->SetBitmap2( aImageHC, BMP_COLOR_HIGHCONTRAST );
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

