/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RelationTableView.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-05 12:37:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_RELATION_TABLEVIEW_HXX
#include "RelationTableView.hxx"
#endif
#ifndef DBAUI_JOINEXCHANGE_HXX
#include "JoinExchange.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef DBAUI_RELATIONDESIGNVIEW_HXX
#include "RelationDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef DBAUI_RTABLECONNECTION_HXX
#include "RTableConnection.hxx"
#endif
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#include "RTableConnectionData.hxx"
#endif
#ifndef DBAUI_RELATIONDIALOG_HXX
#include "RelationDlg.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBU_REL_HRC_
#include "dbu_rel.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBAUI_RELTABLEWINDOW_HXX
#include "RTableWindow.hxx"
#endif
#ifndef DBACCESS_JACCESS_HXX
#include "JAccess.hxx"
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

using namespace dbaui;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;

//==================================================================
// class ORelationTableView
//==================================================================
DBG_NAME(ORelationTableView)
//------------------------------------------------------------------------
ORelationTableView::ORelationTableView( Window* pParent, ORelationDesignView* pView )
    : OJoinTableView( pParent, pView )
    ,m_pExistingConnection(NULL)
{
    DBG_CTOR(ORelationTableView,NULL);
    SetHelpId(HID_CTL_RELATIONTAB);
}

//------------------------------------------------------------------------
ORelationTableView::~ORelationTableView()
{
    DBG_DTOR(ORelationTableView,NULL);
}

//------------------------------------------------------------------------
void ORelationTableView::ReSync()
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    // Es kann sein, dass in der DB Tabellen ausgeblendet wurden, die eigentlich Bestandteil einer Relation sind. Oder eine Tabelle
    // befand sich im Layout (durchaus ohne Relation), existiert aber nicht mehr. In beiden Faellen wird das Anlegen des TabWins schief
    // gehen, und alle solchen TabWinDatas oder darauf bezogenen ConnDatas muss ich dann loeschen.
    ::std::vector< ::rtl::OUString> arrInvalidTables;

    //////////////////////////////////////////////////////////////////////
    // create and insert windows
    TTableWindowData* pTabWinDataList = m_pView->getController()->getTableWindowData();
    TTableWindowData::reverse_iterator aIter = pTabWinDataList->rbegin();
    for(;aIter != pTabWinDataList->rend();++aIter)
    {
        TTableWindowData::value_type pData = *aIter;
        OTableWindow* pTabWin = createWindow(pData);

        if (!pTabWin->Init())
        {
            // das Initialisieren ging schief, dass heisst, dieses TabWin steht nicht zur Verfuegung, also muss ich es inklusive
            // seiner Daten am Dokument aufraeumen
            pTabWin->clearListBox();
            delete pTabWin;
            arrInvalidTables.push_back(pData->GetTableName());

            pTabWinDataList->erase( ::std::remove(pTabWinDataList->begin(),pTabWinDataList->end(),*aIter) ,pTabWinDataList->end());
            continue;
        }

        (*GetTabWinMap())[pData->GetComposedName()] = pTabWin;  // am Anfang einfuegen, da ich die DataList ja rueckwaerts durchlaufe
        // wenn in den Daten keine Position oder Groesse steht -> Default
        if (!pData->HasPosition() && !pData->HasSize())
            SetDefaultTabWinPosSize(pTabWin);

        pTabWin->Show();
    }

    // Verbindungen einfuegen
    TTableConnectionData* pTabConnDataList = m_pView->getController()->getTableConnectionData();
    TTableConnectionData::reverse_iterator aConIter = pTabConnDataList->rbegin();

    for(;aConIter != pTabConnDataList->rend();++aConIter)
    {
        ORelationTableConnectionData* pTabConnData = static_cast<ORelationTableConnectionData*>(aConIter->get());
        if ( !arrInvalidTables.empty() )
        {
            // gibt es die beiden Tabellen zur Connection ?
            ::rtl::OUString strTabExistenceTest = pTabConnData->getReferencingTable()->GetTableName();
            sal_Bool bInvalid = ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();
            strTabExistenceTest = pTabConnData->getReferencedTable()->GetTableName();
            bInvalid = bInvalid || ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();

            if (bInvalid)
            {   // nein -> Pech gehabt, die Connection faellt weg
                pTabConnDataList->erase( ::std::remove(pTabConnDataList->begin(),pTabConnDataList->end(),*aConIter),pTabConnDataList->end() );
                continue;
            }
        }

        addConnection( new ORelationTableConnection(this, *aConIter), sal_False ); // don't add the data again
    }

    if ( !GetTabWinMap()->empty() )
        GetTabWinMap()->begin()->second->GrabFocus();
}
//------------------------------------------------------------------------------
BOOL ORelationTableView::IsAddAllowed()
{
    DBG_CHKTHIS(ORelationTableView,NULL);

    return !m_pView->getController()->isReadOnly();
}
//------------------------------------------------------------------------
void ORelationTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    // Aus selektierten Feldnamen LineDataObject setzen
    // check if relation already exists
    OTableWindow* pSourceWin = jxdSource.pListBox->GetTabWin();
    OTableWindow* pDestWin = jxdDest.pListBox->GetTabWin();

    ::std::vector<OTableConnection*>::const_iterator aIter = getTableConnections()->begin();
    for(;aIter != getTableConnections()->end();++aIter)
    {
        OTableConnection* pFirst = *aIter;
        if((pFirst->GetSourceWin() == pSourceWin && pFirst->GetDestWin() == pDestWin) ||
           (pFirst->GetSourceWin() == pDestWin  && pFirst->GetDestWin() == pSourceWin))
        {
            m_pExistingConnection = pFirst;
            break;
        }
    }
    // insert table connection into view

    TTableConnectionData::value_type pTabConnData(new ORelationTableConnectionData(pSourceWin->GetData(),
                                                                                   pDestWin->GetData()));

    // die Namen der betroffenen Felder
    ::rtl::OUString sSourceFieldName = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    ::rtl::OUString sDestFieldName = jxdDest.pListBox->GetEntryText(jxdDest.pEntry);

    // die Anzahl der PKey-Felder in der Quelle

    ::std::vector< Reference< XNameAccess> > aPkeys = ::dbaui::getKeyColumns(pSourceWin->GetTable(),KeyType::PRIMARY);
    bool bAskUser = aPkeys.size() == 1 && Reference< XIndexAccess>(aPkeys[0],UNO_QUERY)->getCount() > 1;

    pTabConnData->SetConnLine( 0, sSourceFieldName, sDestFieldName );

    if ( bAskUser || m_pExistingConnection )
        m_pCurrentlyTabConnData = pTabConnData; // this implies that we ask the user what to do
    else
    {
        try
        {
            //////////////////////////////////////////////////////////////////////
            // Daten der Datenbank uebergeben
            if( pTabConnData->Update() )
            {
                //////////////////////////////////////////////////////////////////////
                // UI-Object in ConnListe eintragen
                addConnection( new ORelationTableConnection( this, pTabConnData ) );
            }
        }
        catch(const SQLException&)
        {
            throw;
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"ORelationTableView::AddConnection: Exception oocured!");
        }
    }
}


//------------------------------------------------------------------------
void ORelationTableView::ConnDoubleClicked( OTableConnection* pConnection )
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    ORelationDialog aRelDlg( this, pConnection->GetData() );
    switch (aRelDlg.Execute())
    {
        case RET_OK:
            // successfully updated
            pConnection->UpdateLineList();
            // The connection references 1 ConnData and n ConnLines, each ConnData references n LineDatas, each Line exactly 1 LineData
            // As the Dialog and the ConnData->Update may have changed the LineDatas we have to restore the consistent state
            break;

        case RET_NO:
            // tried at least one update, but did not succeed -> the original connection is lost
            RemoveConnection( pConnection ,sal_True);
            break;

        case RET_CANCEL:
            // no break, as nothing happened and we don't need the code below
            return;

    }

    Invalidate(INVALIDATE_NOCHILDREN);
}

//------------------------------------------------------------------------------
void ORelationTableView::AddNewRelation()
{
    DBG_CHKTHIS(ORelationTableView,NULL);

    TTableConnectionData::value_type pNewConnData( new ORelationTableConnectionData() );
    ORelationDialog aRelDlg(this, pNewConnData, TRUE);

    BOOL bSuccess = (aRelDlg.Execute() == RET_OK);
    if (bSuccess)
    {
        // already updated by the dialog
        // dem Dokument bekanntgeben
        addConnection( new ORelationTableConnection(this, pNewConnData) );
    }
}

//------------------------------------------------------------------------------
::std::vector<OTableConnection*>::const_iterator ORelationTableView::RemoveConnection( OTableConnection* pConn ,sal_Bool /*_bDelete*/)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    ::std::vector<OTableConnection*>::const_iterator aNextPos = getTableConnections()->end();
    ORelationTableConnectionData* pTabConnData = (ORelationTableConnectionData*)pConn->GetData().get();
    try
    {
        if (pTabConnData->DropRelation())
            aNextPos = OJoinTableView::RemoveConnection( pConn ,sal_True);
    }
    catch(SQLException& e)
    {
        getDesignView()->getController()->showError(SQLExceptionInfo(e));
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"ORelationTableView::RemoveConnection: Something other than SQLException occured!");
    }
    return aNextPos;
}

//------------------------------------------------------------------------------
void ORelationTableView::AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rWinName, BOOL /*bNewTable*/)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    OSL_ENSURE(_rComposedName.getLength(),"There must be a table name supplied!");
    OJoinTableView::OTableWindowMap::iterator aIter = GetTabWinMap()->find(_rComposedName);

    if(aIter != GetTabWinMap()->end())
    {
        aIter->second->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
        aIter->second->GrabFocus();
        EnsureVisible(aIter->second);
        // no new one
        return;
    }

    //////////////////////////////////////////////////////////////////
    // Neue Datenstruktur in DocShell eintragen
    TTableWindowData::value_type pNewTabWinData(createTableWindowData( _rComposedName, rWinName,rWinName ));
    pNewTabWinData->ShowAll(FALSE);

    //////////////////////////////////////////////////////////////////
    // Neues Fenster in Fensterliste eintragen
    OTableWindow* pNewTabWin = createWindow( pNewTabWinData );
    if(pNewTabWin->Init())
    {
        m_pView->getController()->getTableWindowData()->push_back( pNewTabWinData);
        // when we already have a table with this name insert the full qualified one instead
        (*GetTabWinMap())[_rComposedName] = pNewTabWin;

        SetDefaultTabWinPosSize( pNewTabWin );
        pNewTabWin->Show();

        modified();

        if ( m_pAccessible )
            m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                    Any(),
                                                    makeAny(pNewTabWin->GetAccessible()));
    }
    else
    {
        pNewTabWin->clearListBox();
        delete pNewTabWin;
    }
}
// -----------------------------------------------------------------------------
void ORelationTableView::RemoveTabWin( OTableWindow* pTabWin )
{
    OSQLMessageBox aDlg(this,ModuleRes(STR_QUERY_REL_DELETE_WINDOW),String(),WB_YES_NO|WB_DEF_YES,OSQLMessageBox::Warning);
    if(aDlg.Execute() == RET_YES)
    {
        m_pView->getController()->getUndoMgr()->Clear();
        OJoinTableView::RemoveTabWin( pTabWin );

        m_pView->getController()->InvalidateFeature(SID_RELATION_ADD_RELATION);
        m_pView->getController()->InvalidateFeature(ID_BROWSER_UNDO);
        m_pView->getController()->InvalidateFeature(ID_BROWSER_REDO);
    }
}
// -----------------------------------------------------------------------------
//namespace
//{
//    class OReleationAskDialog : public ButtonDialog
//    {
//        FixedImage        m_aInfoImage;
//        FixedText     m_aTitle;
//      FixedText       m_aMessage;
//    public:
//        OReleationDialog(Window* _pParent) : ButtonDialog(_pParent,WB_HORZ | WB_STDDIALOG)
//            ,m_aInfoImage(this)
//            ,m_aTitle(this,WB_WORDBREAK | WB_LEFT)
//          ,m_aMessage(this,WB_WORDBREAK | WB_LEFT)
//        {
//            m_aMessage.SetText(ModuleRes(STR_QUERY_REL_EDIT_RELATION));
//            m_aMessage.Show();
//
//            // Changed as per BugID 79541 Branding/Configuration
//            String sDialogTitle( lcl_getProductName() );
//            SetText( sDialogTitle.AppendAscii( " Base" ) );
//            m_aTitle.Show();
//        }
//    };
//}
// -----------------------------------------------------------------------------
void ORelationTableView::lookForUiActivities()
{
    if(m_pExistingConnection)
    {
        String sTitle(ModuleRes(STR_RELATIONDESIGN));
        sTitle.Erase(0,3);
        OSQLMessageBox aDlg(this,ModuleRes(STR_QUERY_REL_EDIT_RELATION),String(),0);
        aDlg.SetText(sTitle);
        aDlg.RemoveButton(aDlg.GetButtonId(0));
        aDlg.AddButton( ModuleRes(STR_QUERY_REL_EDIT), BUTTONID_OK, BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON);
        aDlg.AddButton( ModuleRes(STR_QUERY_REL_CREATE), BUTTONID_YES, 0);
        aDlg.AddButton(BUTTON_CANCEL,BUTTONID_CANCEL,0);
        UINT16 nRet = aDlg.Execute();
        if( nRet == RET_CANCEL)
        {
            m_pCurrentlyTabConnData.reset();
        }
        else if ( nRet == RET_OK ) // EDIT
        {
            ConnDoubleClicked(m_pExistingConnection);
            m_pCurrentlyTabConnData.reset();
        }
        m_pExistingConnection = NULL;
    }
    if(m_pCurrentlyTabConnData)
    {
        ORelationDialog aRelDlg( this, m_pCurrentlyTabConnData );
        if (aRelDlg.Execute() == RET_OK)
        {
            // already updated by the dialog
            addConnection( new ORelationTableConnection( this, m_pCurrentlyTabConnData ) );
        }
        m_pCurrentlyTabConnData.reset();
    }
}

// -----------------------------------------------------------------------------
OTableWindow* ORelationTableView::createWindow(const TTableWindowData::value_type& _pData)
{
    return new ORelationTableWindow(this,_pData);
}
// -----------------------------------------------------------------------------
bool ORelationTableView::allowQueries() const
{
    return false;
}

