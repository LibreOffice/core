/*************************************************************************
 *
 *  $RCSfile: RelationTableView.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:23:10 $
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
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

using namespace dbaui;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;

TYPEINIT1(ORelationTableWindow, OTableWindow);

//==================================================================
// class ORelationTableView
//==================================================================
DBG_NAME(ORelationTableView);
//------------------------------------------------------------------------
ORelationTableView::ORelationTableView( Window* pParent, ORelationDesignView* pView )
    : OJoinTableView( pParent, pView )
    ,m_pExistingConnection(NULL)
    ,m_pCurrentlyTabConnData(NULL)
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
    ::std::vector< OTableWindowData*>* pTabWinDataList = m_pView->getController()->getTableWindowData();
    ::std::vector< OTableWindowData*>::reverse_iterator aIter = pTabWinDataList->rbegin();
    for(;aIter != pTabWinDataList->rend();++aIter)
    {
        OTableWindowData* pData = *aIter;
        OTableWindow* pTabWin = createWindow(pData);

        if (!pTabWin->Init())
        {
            // das Initialisieren ging schief, dass heisst, dieses TabWin steht nicht zur Verfuegung, also muss ich es inklusive
            // seiner Daten am Dokument aufraeumen
            pTabWin->clearListBox();
            delete pTabWin;
            arrInvalidTables.push_back(pData->GetTableName());

            pTabWinDataList->erase( ::std::find(pTabWinDataList->begin(),pTabWinDataList->end(),*aIter) );
            delete pData;
            continue;
        }

        (*GetTabWinMap())[pData->GetComposedName()] = pTabWin;  // am Anfang einfuegen, da ich die DataList ja rueckwaerts durchlaufe
        // wenn in den Daten keine Position oder Groesse steht -> Default
        if (!pData->HasPosition() && !pData->HasSize())
            SetDefaultTabWinPosSize(pTabWin);

        pTabWin->Show();
    }

    // Verbindungen einfuegen
    ::std::vector< OTableConnectionData*>* pTabConnDataList = m_pView->getController()->getTableConnectionData();
    ::std::vector< OTableConnectionData*>::reverse_iterator aConIter = pTabConnDataList->rbegin();

    for(;aConIter != pTabConnDataList->rend();++aConIter)
    {
        ORelationTableConnectionData* pTabConnData = static_cast<ORelationTableConnectionData*>(*aConIter);
        if ( !arrInvalidTables.empty() )
        {
            // gibt es die beiden Tabellen zur Connection ?
            ::rtl::OUString strTabExistenceTest = pTabConnData->GetSourceWinName();
            sal_Bool bInvalid = ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();
            strTabExistenceTest = pTabConnData->GetDestWinName();
            bInvalid |= ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();

            if (bInvalid)
            {   // nein -> Pech gehabt, die Connection faellt weg
                pTabConnDataList->erase( ::std::find(pTabConnDataList->begin(),pTabConnDataList->end(),*aConIter) );
                delete pTabConnData;
                continue;
            }
        }

        addConnection( new ORelationTableConnection(this, pTabConnData), sal_False ); // don't add the data again
    }

    if ( !GetTabWinMap()->empty() )
        GetTabWinMap()->begin()->second->GrabFocus();
}
//------------------------------------------------------------------------------
BOOL ORelationTableView::IsAddAllowed()
{
    DBG_CHKTHIS(ORelationTableView,NULL);

    BOOL bAllowed = !m_pView->getController()->isReadOnly();
    if ( bAllowed )
    {
        Reference<XConnection> xConnection = m_pView->getController()->getConnection();
        if(!xConnection.is())
            bAllowed = FALSE;
        else
        {
            try
            {
                Reference < XDatabaseMetaData > xMetaData( xConnection->getMetaData() );
                OSL_ENSURE(xMetaData.is(),"ORelationTableView::IsAddAllowed: The metadata is null!");

                bAllowed = xMetaData.is() && xMetaData->supportsIntegrityEnhancementFacility();
            }
            catch(const SQLException&)
            {
            }
        }
    }
    return bAllowed;
}
//------------------------------------------------------------------------
void ORelationTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    // Aus selektierten Feldnamen LineDataObject setzen
    // check if relation already exists
    OTableWindow* pSourceWin = (OTableWindow*)jxdSource.pListBox->GetTabWin();
    OTableWindow* pDestWin = (OTableWindow*)jxdDest.pListBox->GetTabWin();

    ::std::vector<OTableConnection*>::const_iterator aIter = getTableConnections()->begin();
    for(;aIter != getTableConnections()->end();++aIter)
    {
        OTableConnection* pFirst = *aIter;
        if((pFirst->GetSourceWin() == pSourceWin && pFirst->GetDestWin() == pDestWin) ||
           (pFirst->GetSourceWin() == pDestWin  && pFirst->GetDestWin() == pSourceWin))
        {
            m_pExistingConnection = pFirst;
            return;
        }
    }
    // insert table connection into view


    Reference<XTablesSupplier> xTablesSup(getDesignView()->getController()->getConnection(),UNO_QUERY);
    ORelationTableConnectionData* pTabConnData = NULL;
    OSL_ENSURE(xTablesSup.is(),"ORelationTableView::AddConnection no TablesSupplier");
    if(xTablesSup.is())
        pTabConnData = new ORelationTableConnectionData(xTablesSup->getTables(),
                                                        jxdSource.pListBox->GetTabWin()->GetComposedName(),
                                                        jxdDest.pListBox->GetTabWin()->GetComposedName());

    // die Namen der betroffenen Felder
    String aSourceFieldName = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    String aDestFieldName = jxdDest.pListBox->GetEntryText(jxdDest.pEntry);

    // die Anzahl der PKey-Felder in der Quelle

    UINT16 nSourceKeys(0);
    ::std::vector< Reference< XNameAccess> > aPkeys = ::dbaui::getKeyColumns(pSourceWin->GetTable(),KeyType::PRIMARY);
    if ( aPkeys.size() == 1 ) // there can be only one. But we can not assert here, that would freeze our office
    {
        Reference< XNameAccess> xColumns = pSourceWin->GetOriginalColumns();
        if(xColumns.is())
        {
            Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
            const ::rtl::OUString* pBegin = aNames.getConstArray();
            const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
            for(;pBegin != pEnd;++pBegin)
            {
                if((*aPkeys.begin())->hasByName(*pBegin))
                    pTabConnData->SetConnLine( nSourceKeys++, *pBegin, String() );
            }
        }
    }

    if ( nSourceKeys > 1 )
        m_pCurrentlyTabConnData = pTabConnData;
    else
    {
        pTabConnData->ResetConnLines();
        pTabConnData->SetConnLine( 0, aSourceFieldName, aDestFieldName );

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
            else
                delete pTabConnData;
        }
        catch(const SQLException&)
        {
            delete pTabConnData;
            throw;
        }
        catch(const Exception&)
        {
            delete pTabConnData;
            OSL_ENSURE(0,"ORelationTableView::AddConnection: Exception oocured!");
        }
    }
}


//------------------------------------------------------------------------
void ORelationTableView::ConnDoubleClicked( OTableConnection* pConnection )
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    Reference<XConnection> xConnection = getDesignView()->getController()->getConnection();
    if(xConnection.is())
    {
        ORelationTableConnectionData* pRelTabConnData = (ORelationTableConnectionData*)((ORelationTableConnection*)pConnection)->GetData();
        ORelationDialog aRelDlg( this, pRelTabConnData );
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
}

//------------------------------------------------------------------------------
void ORelationTableView::AddNewRelation()
{
    DBG_CHKTHIS(ORelationTableView,NULL);

    Reference<XTablesSupplier> xTablesSup(getDesignView()->getController()->getConnection(),UNO_QUERY);
    if(xTablesSup.is())
    {
        ORelationTableConnectionData* pNewConnData = new ORelationTableConnectionData(xTablesSup->getTables());
        ORelationDialog aRelDlg(this, pNewConnData, TRUE);

        BOOL bSuccess = (aRelDlg.Execute() == RET_OK);
        if (bSuccess)
        {
            // already updated by the dialog
            // dem Dokument bekanntgeben
            addConnection( new ORelationTableConnection(this, pNewConnData) );
            // neu zeichnen
            //  Invalidate(INVALIDATE_NOCHILDREN);
        }
        else
            delete pNewConnData;
    }
}

//------------------------------------------------------------------------------
::std::vector<OTableConnection*>::const_iterator ORelationTableView::RemoveConnection( OTableConnection* pConn ,sal_Bool _bDelete)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    ::std::vector<OTableConnection*>::const_iterator aNextPos = getTableConnections()->end();
    ORelationTableConnectionData* pTabConnData = (ORelationTableConnectionData*)pConn->GetData();
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
void ORelationTableView::AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rWinName, BOOL bNewTable)
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
    OTableWindowData* pNewTabWinData = CreateImpl( _rComposedName, rWinName );
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
        delete pNewTabWinData;
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
        OJoinTableView::RemoveTabWin( pTabWin );
        m_pView->getController()->InvalidateFeature(ID_RELATION_ADD_RELATION);
    }
}
// -----------------------------------------------------------------------------
void ORelationTableView::lookForUiActivities()
{
    if(m_pExistingConnection)
    {
        OSQLMessageBox aDlg(this,ModuleRes(STR_QUERY_REL_EDIT_RELATION),String(),WB_YES_NO|WB_DEF_YES);
        if(aDlg.Execute() == RET_YES)
            ConnDoubleClicked(m_pExistingConnection);
        m_pExistingConnection = NULL;
    }
    else if(m_pCurrentlyTabConnData)
    {
        ORelationDialog aRelDlg( this, m_pCurrentlyTabConnData );
        if (aRelDlg.Execute() == RET_OK)
        {
            // already updated by the dialog
            addConnection( new ORelationTableConnection( this, m_pCurrentlyTabConnData ) );
            //  Invalidate(INVALIDATE_NOCHILDREN);
        }
        else
            delete m_pCurrentlyTabConnData;
        m_pCurrentlyTabConnData = NULL;
    }
}
// -----------------------------------------------------------------------------
OTableWindow* ORelationTableView::createWindow(OTableWindowData* _pData)
{
    return new ORelationTableWindow(this,_pData);
}
// -----------------------------------------------------------------------------

