/*************************************************************************
 *
 *  $RCSfile: dsEntriesNoExp.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:33:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBA_UNODATBR_HXX_
#include "unodatbr.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
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
            return etTable;
        case etQueryContainer:
            return etQuery;
    }
    return etUnknown;
}

// -----------------------------------------------------------------------------
String SbaTableQueryBrowser::GetEntryText( SvLBoxEntry* _pEntry )
{
    return m_pTreeView->getListBox()->GetEntryText(_pEntry);
}

// -----------------------------------------------------------------------------
SbaTableQueryBrowser::EntryType SbaTableQueryBrowser::getEntryType( SvLBoxEntry* _pEntry ) const
{
    if (!_pEntry)
        return etUnknown;

    SvLBoxEntry* pRootEntry     = m_pTreeView->getListBox()->GetRootLevelParent(_pEntry);
    SvLBoxEntry* pEntryParent   = m_pTreeView->getListBox()->GetParent(_pEntry);
    SvLBoxEntry* pTables        = m_pTreeView->getListBox()->GetEntry(pRootEntry, CONTAINER_TABLES);
    SvLBoxEntry* pQueries       = m_pTreeView->getListBox()->GetEntry(pRootEntry, CONTAINER_QUERIES);

#ifdef DBG_UTIL
    String sTest;
    if (pTables) sTest = m_pTreeView->getListBox()->GetEntryText(pTables);
    if (pQueries) sTest = m_pTreeView->getListBox()->GetEntryText(pQueries);
#endif

    if (pRootEntry == _pEntry)
        return etDatasource;

    if (pTables == _pEntry)
        return etTableContainer;

    if (pQueries == _pEntry)
        return etQueryContainer;

    if (pTables == pEntryParent)
        return etTable;

    if (pQueries == pEntryParent)
        return etQuery;

    return etUnknown;
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
    else
        DBG_ERROR("SbaTableQueryBrowser::select: invalid entry!");
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
    else
        DBG_ERROR("SbaTableQueryBrowser::isSelected: invalid entry!");
    return sal_False;
}
//------------------------------------------------------------------------------
void SbaTableQueryBrowser::AddSupportedFeatures()
{
    SbaXDataBrowserController::AddSupportedFeatures();

    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Title")]                           = ID_BROWSER_TITLE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/FormLetter")]    = ID_BROWSER_FORMLETTER;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/InsertColumns")] = ID_BROWSER_INSERTCOLUMNS;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/InsertContent")] = ID_BROWSER_INSERTCONTENT;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/ToggleExplore")] = ID_BROWSER_EXPLORER;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/DocumentDataSource")] = ID_BROWSER_DOCUMENT_DATASOURCE;

            // TODO reenable our own code if we really have a handling for the formslots
//  ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToFirst"),        SID_FM_RECORD_FIRST     ),
//  ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToLast"),     SID_FM_RECORD_LAST      ),
//  ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToNew"),      SID_FM_RECORD_NEW       ),
//  ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToNext"),     SID_FM_RECORD_NEXT      ),
//  ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToPrev"),     SID_FM_RECORD_PREV      )

}
// -------------------------------------------------------------------------
String SbaTableQueryBrowser::getURL() const
{
    return String();
}
//------------------------------------------------------------------------------
ToolBox* SbaTableQueryBrowser::CreateToolBox(Window* _pParent)
{
    return m_bShowToolbox ? new ToolBox( _pParent, ModuleRes( RID_BRW_QRY_TOOLBOX ) ) : NULL;
}
// -----------------------------------------------------------------------------
void SbaTableQueryBrowser::notifyHiContrastChanged()
{
    if ( m_pTreeView )
    {
        sal_Bool bHiContrast = isHiContrast();

        if ( m_bHiContrast != bHiContrast )
        {
            m_bHiContrast = bHiContrast;
            // change all bitmap entries
            DBTreeListBox* pListBox = m_pTreeView->getListBox();

            SvLBoxEntry* pEntryLoop = m_pTreeModel->First();
            while ( pEntryLoop )
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntryLoop->GetUserData());
                if ( pData )
                {
                    ModuleRes aResId(DBTreeListModel::getImageResId(pData->eType,isHiContrast()));
                    Image aImage(aResId);
                    USHORT nCount = pEntryLoop->ItemCount();
                    for (USHORT i=0;i<nCount;++i)
                    {
                        SvLBoxItem* pItem = pEntryLoop->GetItem(i);
                        if ( pItem && pItem->IsA() == SV_ITEM_ID_LBOXCONTEXTBMP)
                        {
                            static_cast<SvLBoxContextBmp*>(pItem)->SetBitmap1(pEntryLoop,aImage);
                            static_cast<SvLBoxContextBmp*>(pItem)->SetBitmap2(pEntryLoop,aImage);
                            break;
                        }
                    }
                }
                pEntryLoop = m_pTreeModel->Next(pEntryLoop);
            }
        }
    }
}
// -----------------------------------------------------------------------------
// .........................................................................
}   // namespace dbaui
// .........................................................................

