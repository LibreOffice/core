/*************************************************************************
 *
 *  $RCSfile: RelationDlg.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:03:08 $
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
//#ifndef _SVX_TABWIN_HXX
//#include "tabwin.hxx"
//#endif
#ifndef DBAUI_RELATIONDIALOG_HRC
#include "RelationDlg.hrc"
#endif
#ifndef DBAUI_RELATIONDIALOG_HXX
#include "RelationDlg.hxx"
#endif

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace dbaui;
using namespace dbtools;


const USHORT SOURCE_COLUMN = 1;
const USHORT DEST_COLUMN = 2;

//========================================================================
// class ORelationControl
//========================================================================
DBG_NAME(ORelationControl);
//------------------------------------------------------------------------
ORelationControl::ORelationControl( ORelationDialog* pParent )
    :DbBrowseBox( pParent, DBBF_NOROWPICTURE, WB_TABSTOP | WB_3DLOOK | WB_BORDER )
    ,m_pListCell( NULL )
    ,m_pConnData( NULL )
    ,m_xSourceDef( NULL )
    ,m_xDestDef( NULL )
    ,m_nDeActivateEvent(0)
{
    DBG_CTOR(ORelationControl,NULL);
}

//------------------------------------------------------------------------
ORelationControl::~ORelationControl()
{
    DBG_DTOR(ORelationControl,NULL);
    if (m_nDeActivateEvent)
        Application::RemoveUserEvent(m_nDeActivateEvent);

    delete m_pListCell;
}

//------------------------------------------------------------------------
void ORelationControl::Init(ORelationTableConnectionData* _pConnData)
{
    DBG_CHKTHIS(ORelationControl,NULL);

    m_pConnData = _pConnData;
    m_xSourceDef = m_pConnData->getSource();
    m_xDestDef = m_pConnData->getDest();

    BOOL bFirstCall = (ColCount() == 0);

    if (bFirstCall)
    {
        //////////////////////////////////////////////////////////////////////
        // Spalten einfuegen
        ::rtl::OUString sSrcComposedName,sDestComposedName;
        if(m_xSourceDef.is())
            ::dbaui::composeTableName(static_cast<ORelationDialog*>(GetParent())->getConnection()->getMetaData(),m_xSourceDef,sSrcComposedName,sal_False);
        if(m_xDestDef.is())
            ::dbaui::composeTableName(static_cast<ORelationDialog*>(GetParent())->getConnection()->getMetaData(),m_xDestDef,sDestComposedName,sal_False);
        InsertDataColumn( 1, sSrcComposedName, 100);
        InsertDataColumn( 2, sDestComposedName, 100);
            // wenn es die Defs noch nicht gibt, dann muessen sie noch mit SetSource-/-DestDef gesetzt werden !

        m_pListCell = new DbListBoxCtrl( &GetDataWindow() );

        //////////////////////////////////////////////////////////////////////
        // Browser Mode setzen
        BrowserMode nMode = BROWSER_COLUMNSELECTION |
            BROWSER_HLINESFULL | BROWSER_VLINESFULL |
            BROWSER_HIDECURSOR | BROWSER_HIDESELECT;

        SetMode(nMode);
    }
    else
        // not the first call
        RowRemoved(0, GetRowCount());

    RowInserted(0, m_pConnData->GetConnLineDataList()->size(), TRUE);

}

//------------------------------------------------------------------------------
void ORelationControl::Resize()
{
    DBG_CHKTHIS(ORelationControl,NULL);
    DbBrowseBox::Resize();
    long nOutputWidth = GetOutputSizePixel().Width()-20;
    SetColumnWidth(1, (long)(nOutputWidth*0.5));
    SetColumnWidth(2, (long)(nOutputWidth*0.5));
}

//------------------------------------------------------------------------------
long ORelationControl::PreNotify(NotifyEvent& rNEvt)
{
    DBG_CHKTHIS(ORelationControl,NULL);
    if (rNEvt.GetType() == EVENT_LOSEFOCUS)
        if (!HasChildPathFocus())
        {
            if (m_nDeActivateEvent)
                Application::RemoveUserEvent(m_nDeActivateEvent);
            m_nDeActivateEvent = Application::PostUserEvent(LINK(this, ORelationControl, AsynchDeactivate));
        }
    if (rNEvt.GetType() == EVENT_GETFOCUS)
    {
        if (m_nDeActivateEvent)
            Application::RemoveUserEvent(m_nDeActivateEvent);
        m_nDeActivateEvent = Application::PostUserEvent(LINK(this, ORelationControl, AsynchActivate));
    }
    return DbBrowseBox::PreNotify(rNEvt);
}

//------------------------------------------------------------------------------
IMPL_LINK(ORelationControl, AsynchActivate, void*, EMPTYARG)
{
    m_nDeActivateEvent = 0;
    ActivateCell();
    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(ORelationControl, AsynchDeactivate, void*, EMPTYARG)
{
    m_nDeActivateEvent = 0;
    DeactivateCell();
    return 0L;
}

//------------------------------------------------------------------------------
BOOL ORelationControl::IsTabAllowed(BOOL bForward) const
{
    DBG_CHKTHIS(ORelationControl,NULL);
    long nRow = GetCurRow();
    USHORT nCol = GetCurColumnId();

    if (bForward && (nCol == 2) && (nRow == GetRowCount() - 1))
        return FALSE;

    if (!bForward && (nCol == 1) && (nRow == 0))
        return FALSE;

    return DbBrowseBox::IsTabAllowed(bForward);
}

//------------------------------------------------------------------------------
BOOL ORelationControl::SaveModified()
{
    DBG_CHKTHIS(ORelationControl,NULL);
    OSL_ENSURE((sal_Int32)m_pConnData->GetConnLineDataList()->size() > GetCurRow(),"Invalid Index!");
    OConnectionLineData* pConnLineData = (*m_pConnData->GetConnLineDataList())[GetCurRow()];
    switch( GetCurColumnId() )
    {
    case SOURCE_COLUMN:
        pConnLineData->SetSourceFieldName( m_pListCell->GetSelectEntry() );
        break;

    case DEST_COLUMN:
        pConnLineData->SetDestFieldName( m_pListCell->GetSelectEntry() );
        break;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
String ORelationControl::GetCellText( long nRow, USHORT nColId )
{
    DBG_CHKTHIS(ORelationControl,NULL);
    OSL_ENSURE((sal_Int32)m_pConnData->GetConnLineDataList()->size() > nRow,"Invalid Index!");
    OConnectionLineData* pConnLineData = (*m_pConnData->GetConnLineDataList())[nRow];

    if( !pConnLineData )
        return String();

    switch( nColId )
    {
    case SOURCE_COLUMN:
        return pConnLineData->GetSourceFieldName();
    case DEST_COLUMN:
        return pConnLineData->GetDestFieldName();
    }

    return String();
}

//------------------------------------------------------------------------------
void ORelationControl::InitController( DbCellControllerRef& rController, long nRow, USHORT nColumnId )
{
    DBG_CHKTHIS(ORelationControl,NULL);
    String aText;

    switch (nColumnId)
    {
        case SOURCE_COLUMN:
            //////////////////////////////////////////////////////////////
            // Auffuellen der ComboBox
            m_pListCell->Clear();
            if( m_xSourceDef.is() )
            {
                Reference<XColumnsSupplier> xSup(m_xSourceDef,UNO_QUERY);
                Reference<XNameAccess> xColumns = xSup->getColumns();
                Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
                const ::rtl::OUString* pBegin = aNames.getConstArray();
                const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                    m_pListCell->InsertEntry( *pBegin );
                m_pListCell->InsertEntry(String(), 0);
            }

            //////////////////////////////////////////////////////////////
            // Setzen des Edits
            aText = GetCellText( nRow, nColumnId );

            m_pListCell->SelectEntry(aText);

            m_pListCell->SetHelpId(HID_RELATIONDIALOG_LEFTFIELDCELL);
            m_pListCell->SetHelpText(String());
            break;
        case DEST_COLUMN:
            //////////////////////////////////////////////////////////////
            // Auffuellen der ComboBox
            m_pListCell->Clear();
            if( m_xDestDef.is() )
            {
                Reference<XColumnsSupplier> xSup(m_xDestDef,UNO_QUERY);
                Reference<XNameAccess> xColumns = xSup->getColumns();
                Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
                const ::rtl::OUString* pBegin = aNames.getConstArray();
                const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                    m_pListCell->InsertEntry( *pBegin );
                m_pListCell->InsertEntry(String(), 0);
            }

            //////////////////////////////////////////////////////////////
            // Setzen des Edits
            aText = GetCellText( nRow, nColumnId );
            m_pListCell->SelectEntry(aText);
            m_pListCell->SetText( aText );

            m_pListCell->SetHelpId(HID_RELATIONDIALOG_RIGHTFIELDCELL);
            m_pListCell->SetHelpText(String());
            break;
    }
}

//------------------------------------------------------------------------------
DbCellController* ORelationControl::GetController( long nRow, USHORT nColumnId )
{
    DBG_CHKTHIS(ORelationControl,NULL);
    return new DbListBoxCellController( m_pListCell );
}


//------------------------------------------------------------------------------
BOOL ORelationControl::SeekRow( long nRow )
{
    DBG_CHKTHIS(ORelationControl,NULL);
    m_nDataPos = nRow;
    return TRUE;
}

//------------------------------------------------------------------------------
void ORelationControl::PaintCell( OutputDevice& rDev, const Rectangle& rRect, USHORT nColumnId ) const
{
    DBG_CHKTHIS(ORelationControl,NULL);
    String aText = ((ORelationControl*)this)->GetCellText( m_nDataPos, nColumnId );

    Point aPos( rRect.TopLeft() );
    Size aTextSize( GetDataWindow().GetTextHeight(),GetDataWindow().GetTextWidth( aText ));

    if( aPos.X() < rRect.Right() || aPos.X() + aTextSize.Width() > rRect.Right() ||
        aPos.Y() < rRect.Top() || aPos.Y() + aTextSize.Height() > rRect.Bottom() )
        rDev.SetClipRegion( rRect );

    rDev.DrawText( aPos, aText );

    if( rDev.IsClipRegion() )
        rDev.SetClipRegion();
}



//------------------------------------------------------------------------
void ORelationControl::SetSourceDef(const Reference< XPropertySet>& xNewSource)
{
    DBG_CHKTHIS(ORelationControl,NULL);
    m_xSourceDef = xNewSource;
    SetDef(m_xSourceDef,1);
}
// -----------------------------------------------------------------------------
void ORelationControl::SetDef(const Reference< XPropertySet>& xDest,sal_Int32 _nPos)
{
    // wenn ich hier gerade editiere, ausblenden
    BOOL bWasEditing = IsEditing();
    if (bWasEditing)
        DeactivateCell();

    // Spaltenname setzen
    ::rtl::OUString sComposedName;
    if(xDest.is())
        ::dbaui::composeTableName(static_cast<ORelationDialog*>(GetParent())->getConnection()->getMetaData(),xDest,sComposedName,sal_False);

    SetColumnTitle(_nPos, sComposedName);

    // beide (!) Spalten loeschen
    ::std::vector<OConnectionLineData*>* pLines = m_pConnData->GetConnLineDataList();
    ::std::vector<OConnectionLineData*>::iterator aIter = pLines->begin();

    for(;aIter != pLines->end();++aIter)
    {
        (*aIter)->SetSourceFieldName(String());
        (*aIter)->SetDestFieldName(String());
    }
    // neu zeichnen
    Invalidate();

    if (bWasEditing)
    {
        GoToRow(0);
        ActivateCell();
    }
}
//------------------------------------------------------------------------
void ORelationControl::SetDestDef(const Reference< XPropertySet>& xNewDest)
{
    DBG_CHKTHIS(ORelationControl,NULL);
    m_xDestDef = xNewDest;
    SetDef(m_xDestDef,2);
}

//------------------------------------------------------------------------
void ORelationControl::CellModified()
{
    DBG_CHKTHIS(ORelationControl,NULL);
    DbBrowseBox::CellModified();
    SaveModified();
    ((ORelationDialog*)GetParent())->NotifyCellChange();
}

//========================================================================
// class ORelationDialog
//========================================================================

//------------------------------------------------------------------------
ORelationDialog::ORelationDialog( OJoinTableView* pParent,
                                 ORelationTableConnectionData* pConnectionData,
                                 BOOL bAllowTableSelect )
    : ModalDialog( pParent, ModuleRes(DLG_REL_PROPERTIES) )
    ,aGB_InvolvedTables(    this, ModuleRes(GB_INVOLVED_TABLES))
    ,m_lmbLeftTable(        this, ModuleRes(LB_LEFT_TABLE))
    ,m_lmbRightTable(       this, ModuleRes(LB_RIGHT_TABLE))
    ,aGB_InvolvedFields(    this, ModuleRes(GB_INVOLVED_FIELDS))
    ,aGB_CascUpd(           this, ModuleRes(GB_CASC_UPD) )
    ,aRB_NoCascUpd(         this, ModuleRes(RB_NO_CASC_UPD) )
    ,aRB_CascUpd(           this, ModuleRes(RB_CASC_UPD) )
    ,aRB_CascUpdNull(       this, ModuleRes(RB_CASC_UPD_NULL) )
    ,aRB_CascUpdDefault(    this, ModuleRes(RB_CASC_UPD_DEFAULT) )
    ,aGB_CascDel(           this, ModuleRes(GB_CASC_DEL) )
    ,aRB_NoCascDel(         this, ModuleRes(RB_NO_CASC_DEL) )
    ,aRB_CascDel(           this, ModuleRes(RB_CASC_DEL) )
    ,aRB_CascDelNull(       this, ModuleRes(RB_CASC_DEL_NULL) )
    ,aRB_CascDelDefault(    this, ModuleRes(RB_CASC_DEL_DEFAULT) )
    ,m_pOrigConnData( pConnectionData )

    ,aPB_OK( this, ResId( PB_OK ) )
    ,aPB_CANCEL( this, ResId( PB_CANCEL ) )
    ,aPB_HELP( this, ResId( PB_HELP ) )
    ,m_bTriedOneUpdate(FALSE)
{
    m_xConnection = pParent->getDesignView()->getController()->getConnection();

    //////////////////////////////////////////////////////////////////////
    // Connection kopieren
    m_pConnData = new ORelationTableConnectionData( *pConnectionData );
    m_pRC_Tables = new ORelationControl( this );
    m_pRC_Tables->SetHelpId(HID_RELDLG_KEYFIELDS);
    m_pRC_Tables->Init( );
    m_pRC_Tables->SetZOrder(&m_lmbRightTable, WINDOW_ZORDER_BEHIND);

    //////////////////////////////////////////////////////////////////////
    // BrowseBox positionieren
    Point aDlgPoint = LogicToPixel( Point(12,55), MAP_APPFONT );
    Size aDlgSize = LogicToPixel( Size(162,68), MAP_APPFONT );
    m_pRC_Tables->SetPosSizePixel( aDlgPoint, aDlgSize );
    m_pRC_Tables->Show();

    //////////////////////////////////////////////////////////////////////
    // Radio-Buttons enablen/disablen
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

    // Update Rules
//  sal_Int16 nAttributes;
//  rMetaData->getInfo(DatabaseInfo::FOREIGN_KEY_UPDATE_RULE) >>= nAttributes;
//  if( !(nAttributes & KeyRule::NO_ACTION) ){
//      aRB_NoCascUpd.Enable( FALSE );
//  }
//  if( !(nAttributes & KeyRule::CASCADE) ){
//      aRB_CascUpd.Enable( FALSE );
//  }
//  if( !(nAttributes & KeyRule::SET_NULL) ){
//      aRB_CascUpdNull.Enable( FALSE );
//  }
//  if( !(nAttributes & KeyRule::SET_DEFAULT) ){
//      aRB_CascUpdDefault.Enable( FALSE );
//  }
//
//  // Delete Rules
//  rMetaData->getInfo(DatabaseInfo::FOREIGN_KEY_DELETE_RULE) >>= nAttributes;
//  if( !(nAttributes & KeyRule::NO_ACTION) ){
//      aRB_NoCascDel.Enable( FALSE );
//  }
//  if( !(nAttributes & KeyRule::CASCADE) ){
//      aRB_CascDel.Enable( FALSE );
//  }
//  if( !(nAttributes & KeyRule::SET_NULL) ){
//      aRB_CascDelNull.Enable( FALSE );
//  }
//  if( !(nAttributes & KeyRule::SET_DEFAULT) ){
//      aRB_CascDelDefault.Enable( FALSE );
//  }


    Init(m_pConnData);


    aPB_OK.SetClickHdl( LINK(this, ORelationDialog, OKClickHdl) );

    if (bAllowTableSelect)
    {
        OJoinTableView::OTableWindowMap*    pTabWins = pParent->GetTabWinMap();
        DBG_ASSERT(pTabWins->size() >= 2, "ORelationDialog::ORelationDialog : brauche mindestens zwei TabWins !");
        OTableWindow* pInitialLeft = NULL;
        OTableWindow* pInitialRight = NULL;

        // die Namen aller TabWins einsammeln
        OJoinTableView::OTableWindowMap::iterator aIter = pTabWins->begin();
        for(;aIter != pTabWins->end();++aIter)
        {
            m_lmbLeftTable.InsertEntry(aIter->first);
            m_lmbRightTable.InsertEntry(aIter->first);

            if (!pInitialLeft)
                pInitialLeft = aIter->second;
            else if (!pInitialRight)
                pInitialRight = aIter->second;
        }

        // links das erste, rechts das zweite selektieren
        m_lmbLeftTable.SelectEntryPos(0);
        m_lmbRightTable.SelectEntryPos(1);

        // die entsprechenden Defs an mein Controls
        m_pRC_Tables->SetSourceDef(pInitialLeft->GetTable());
        m_pRC_Tables->SetDestDef(pInitialRight->GetTable());

        // die in einer ComboBox ausgewaehlte Tabelle darf nicht in der anderen zur Verfuegung stehen
        m_strCurrentLeft = pInitialLeft->GetTableName();
        m_strCurrentRight = pInitialRight->GetTableName();
        if (pTabWins->size() > 2)
        {
            m_lmbLeftTable.RemoveEntry(m_strCurrentRight);
            m_lmbRightTable.RemoveEntry(m_strCurrentLeft);
        }

        m_lmbLeftTable.SetSelectHdl(LINK(this, ORelationDialog, OnTableChanged));
        m_lmbRightTable.SetSelectHdl(LINK(this, ORelationDialog, OnTableChanged));

        m_lmbLeftTable.GrabFocus();
    }
    else
    {
        m_lmbLeftTable.InsertEntry(pConnectionData->GetSourceWinName());
        m_lmbLeftTable.SelectEntryPos(0);
        m_lmbLeftTable.Disable();

        m_lmbRightTable.InsertEntry(pConnectionData->GetDestWinName());
        m_lmbRightTable.SelectEntryPos(0);
        m_lmbRightTable.Disable();
    }

    NotifyCellChange();

    FreeResource();
}

//------------------------------------------------------------------------
void ORelationDialog::Init(ORelationTableConnectionData* _m_pConnData)
{
    // Update Rules
    switch (_m_pConnData->GetUpdateRules())
    {
    case KeyRule::NO_ACTION:
        aRB_NoCascUpd.Check( TRUE );
        break;

    case KeyRule::CASCADE:
        aRB_CascUpd.Check( TRUE );
        break;

    case KeyRule::SET_NULL:
        aRB_CascUpdNull.Check( TRUE );
        break;
    case KeyRule::SET_DEFAULT:
        aRB_CascUpdDefault.Check( TRUE );
        break;
    }

    // Delete Rules
    switch (_m_pConnData->GetDeleteRules())
    {
    case KeyRule::NO_ACTION:
        aRB_NoCascDel.Check( TRUE );
        break;

    case KeyRule::CASCADE:
        aRB_CascDel.Check( TRUE );
        break;

    case KeyRule::SET_NULL:
        aRB_CascDelNull.Check( TRUE );
        break;
    case KeyRule::SET_DEFAULT:
        aRB_CascDelDefault.Check( TRUE );
        break;
    }


    m_pRC_Tables->Init( _m_pConnData );
}

//------------------------------------------------------------------------
ORelationDialog::~ORelationDialog()
{
    delete m_pConnData;
    delete m_pRC_Tables;
}

//------------------------------------------------------------------------
void ORelationDialog::NotifyCellChange()
{
    // den Ok-Button en- oder disablen, je nachdem, ob ich eine gueltige Situation habe
    BOOL bValid = TRUE;
    USHORT nEmptyRows = 0;
    ::std::vector<OConnectionLineData*>* pLines = m_pConnData->GetConnLineDataList();
    ::std::vector<OConnectionLineData*>::iterator aIter = pLines->begin();
    for(;aIter != pLines->end();++aIter)
    {
        if (((*aIter)->GetDestFieldName().getLength() != 0) != ((*aIter)->GetSourceFieldName().getLength() != 0))
            bValid = FALSE;
            // wenn nich beide leer oder beide voll sind -> ungueltig
        if (((*aIter)->GetDestFieldName().getLength() == 0) && ((*aIter)->GetSourceFieldName().getLength() == 0))
            ++nEmptyRows;
    }
    aPB_OK.Enable(bValid && (nEmptyRows != pLines->size()));
        // nur leere Zeilen -> ungueltig

    if (nEmptyRows == 0)
    {
        m_pConnData->AppendConnLine(String(), String());
        m_pRC_Tables->DeactivateCell();
        m_pRC_Tables->RowInserted(m_pRC_Tables->GetRowCount(), 1, TRUE);
        m_pRC_Tables->ActivateCell();
    }
}

//------------------------------------------------------------------------
IMPL_LINK( ORelationDialog, OKClickHdl, Button*, pButton )
{
    //////////////////////////////////////////////////////////////////////
    // RadioButtons auslesen
    UINT16 nAttrib = 0;

    // Delete Rules
    if( aRB_NoCascDel.IsChecked() )
        nAttrib |= KeyRule::NO_ACTION;
    if( aRB_CascDel.IsChecked() )
        nAttrib |= KeyRule::CASCADE;
    if( aRB_CascDelNull.IsChecked() )
        nAttrib |= KeyRule::SET_NULL;
    if( aRB_CascDelDefault.IsChecked() )
        nAttrib |= KeyRule::SET_DEFAULT;

    m_pConnData->SetDeleteRules( nAttrib );


    // Update Rules
    nAttrib = 0;
    if( aRB_NoCascUpd.IsChecked() )
        nAttrib |= KeyRule::NO_ACTION;
    if( aRB_CascUpd.IsChecked() )
        nAttrib |= KeyRule::CASCADE;
    if( aRB_CascUpdNull.IsChecked() )
        nAttrib |= KeyRule::SET_NULL;
    if( aRB_CascUpdDefault.IsChecked() )
        nAttrib |= KeyRule::SET_DEFAULT;
    m_pConnData->SetUpdateRules( nAttrib );

    m_pRC_Tables->SaveModified();

    // wenn die ComboBoxen fuer die Tabellenauswahl enabled sind (Constructor mit bAllowTableSelect==TRUE), dann muss ich in die
    // Connection auch die Tabellennamen stecken
    m_pConnData->SetSourceWinName(m_lmbLeftTable.GetSelectEntry());
    m_pConnData->SetDestWinName(m_lmbRightTable.GetSelectEntry());

    // noch ein wenig Normalisierung auf den LineDatas : leere Lines vom Anfang an das Ende verschieben
    ::std::vector<OConnectionLineData*>* pLines = m_pConnData->GetConnLineDataList();
    sal_Int32 nCount = pLines->size();
    for(sal_Int32 i=0;i<nCount;)
    {
        if(!(*pLines)[i]->GetSourceFieldName().getLength() && !(*pLines)[i]->GetDestFieldName().getLength())
        {
            OConnectionLineData* pData = (*pLines)[i];
            pLines->erase(pLines->begin()+i);
            pLines->push_back(pData);
            --nCount;
        }
        else
            ++i;
    }

    // try to create the relation
    try
    {
        if (m_pConnData->Update())
        {
            *m_pOrigConnData = *m_pConnData;
            EndDialog( RET_OK );
            return 0L;
        }
    }
    catch(SQLException& e)
    {
        ::dbaui::showError( SQLExceptionInfo(e),
                            this,
                            static_cast<OJoinTableView*>(GetParent())->getDesignView()->getController()->getORB());
    }
    catch(Exception&)
    {
        OSL_ASSERT(0);
    }

    m_bTriedOneUpdate = TRUE;
    // this means that the original connection may be lost (if m_pConnData was not a newly created but an
    // existent conn to be modified), which we reflect by returning RET_NO (see ::Execute)

    // try again
    Init(m_pConnData);

    return 0;
}

//------------------------------------------------------------------------
IMPL_LINK( ORelationDialog, OnTableChanged, ListBox*, pListBox )
{
    String strSelected(pListBox->GetSelectEntry());
    OJoinTableView::OTableWindowMap*    pTabWins = static_cast<OJoinTableView*>(GetParent())->GetTabWinMap();

    // eine Sonderbehandlung : wenn es nur zwei Tabellen gibt, muss ich bei Wechsel in einer LB auch in der anderen umschalten
    if (pTabWins->size() == 2)
    {
        ListBox* pOther;
        if (pListBox == &m_lmbLeftTable)
            pOther = &m_lmbRightTable;
        else
            pOther = &m_lmbLeftTable;
        pOther->SelectEntryPos(1 - pOther->GetSelectEntryPos());

        OJoinTableView::OTableWindowMap::const_iterator aIter = pTabWins->begin();
        OTableWindow* pFirst = aIter->second;
        ++aIter;
        OTableWindow* pSecond = aIter->second;

        Reference< XPropertySet> xLeftTable, xRightTable;
        if (m_lmbLeftTable.GetSelectEntry() == String(pFirst->GetTableName()))
        {
            xLeftTable = pFirst->GetTable();
            xRightTable = pSecond->GetTable();
        }
        else
        {
            xRightTable = pFirst->GetTable();
            xLeftTable = pSecond->GetTable();
        }
        m_pRC_Tables->SetSourceDef(xLeftTable);
        m_pRC_Tables->SetDestDef(xRightTable);
    }
    else
    {
        // zuerst brauche ich die TableDef zur Tabelle, dazu das TabWin
        OTableWindow* pLoop = (*pTabWins)[strSelected];
        DBG_ASSERT(pLoop != NULL, "ORelationDialog::OnTableChanged : ungueltiger Eintrag in ListBox !");
            // da ich die ListBoxen selber mit eben diesen Tabellennamen, mit denen ich sie jetzt vergleiche, gefuellt habe,
            // MUSS ich strSelected finden
        if (pListBox == &m_lmbLeftTable)
        {
            m_pRC_Tables->SetSourceDef(pLoop->GetTable());

            // den vorher links selektierten Eintrag wieder rein rechts
            m_lmbRightTable.InsertEntry(m_strCurrentLeft);
            // und den jetzt selektierten raus
            m_lmbRightTable.RemoveEntry(strSelected);
            m_strCurrentLeft = strSelected;

            m_lmbLeftTable.GrabFocus();
        }
        else
        {
            m_pRC_Tables->SetDestDef(pLoop->GetTable());

            // den vorher rechts selektierten Eintrag wieder rein links
            m_lmbLeftTable.InsertEntry(m_strCurrentRight);
            // und den jetzt selektierten raus
            m_lmbLeftTable.RemoveEntry(strSelected);
            m_strCurrentRight = strSelected;
        }
    }

    pListBox->GrabFocus();

    NotifyCellChange();
    return 0;
}

//------------------------------------------------------------------------
short ORelationDialog::Execute()
{
    short nResult = ModalDialog::Execute();
    if ((nResult != RET_OK) && m_bTriedOneUpdate)
        return RET_NO;

    return nResult;
}
// -----------------------------------------------------------------------------




