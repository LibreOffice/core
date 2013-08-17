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

#include "RelationTableView.hxx"
#include "JoinExchange.hxx"
#include <comphelper/extract.hxx>
#include "browserids.hxx"
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "dbustrings.hrc"
#include <connectivity/dbtools.hxx>
#include <comphelper/sequence.hxx>
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "RelationDesignView.hxx"
#include "JoinController.hxx"
#include "TableWindow.hxx"
#include "TableWindowData.hxx"
#include "RTableConnection.hxx"
#include "RTableConnectionData.hxx"
#include "RelationDlg.hxx"
#include "sqlmessage.hxx"
#include "dbu_rel.hrc"
#include "UITools.hxx"
#include <connectivity/dbexception.hxx>
#include "RTableWindow.hxx"
#include "JAccess.hxx"
#include <svl/undo.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

using namespace dbaui;
using namespace ::dbtools;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;

// class ORelationTableView
DBG_NAME(ORelationTableView)
ORelationTableView::ORelationTableView( Window* pParent, ORelationDesignView* pView )
    :OJoinTableView( pParent, pView )
    , ::comphelper::OContainerListener(m_aMutex)
    ,m_pExistingConnection(NULL)
    ,m_bInRemove(false)

{
    DBG_CTOR(ORelationTableView,NULL);
    SetHelpId(HID_CTL_RELATIONTAB);
}

ORelationTableView::~ORelationTableView()
{
    DBG_DTOR(ORelationTableView,NULL);
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

void ORelationTableView::ReSync()
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    if ( !m_pContainerListener.is() )
    {
        Reference< XConnection> xConnection = m_pView->getController().getConnection();
        Reference< XTablesSupplier > xTableSupp( xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xTables = xTableSupp->getTables();
        Reference< XContainer> xContainer(xTables,uno::UNO_QUERY);
        if ( xContainer.is() )
            m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
    }
    // Es kann sein, dass in der DB Tabellen ausgeblendet wurden, die eigentlich Bestandteil einer Relation sind. Oder eine Tabelle
    // befand sich im Layout (durchaus ohne Relation), existiert aber nicht mehr. In beiden Faellen wird das Anlegen des TabWins schief
    // gehen, und alle solchen TabWinDatas oder darauf bezogenen ConnDatas muss ich dann loeschen.
    ::std::vector< OUString> arrInvalidTables;

    // create and insert windows
    TTableWindowData* pTabWinDataList = m_pView->getController().getTableWindowData();
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
    TTableConnectionData* pTabConnDataList = m_pView->getController().getTableConnectionData();
    TTableConnectionData::reverse_iterator aConIter = pTabConnDataList->rbegin();

    for(;aConIter != pTabConnDataList->rend();++aConIter)
    {
        ORelationTableConnectionData* pTabConnData = static_cast<ORelationTableConnectionData*>(aConIter->get());
        if ( !arrInvalidTables.empty() )
        {
            // gibt es die beiden Tabellen zur Connection ?
            OUString strTabExistenceTest = pTabConnData->getReferencingTable()->GetTableName();
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

sal_Bool ORelationTableView::IsAddAllowed()
{
    DBG_CHKTHIS(ORelationTableView,NULL);

    return !m_pView->getController().isReadOnly();
}

void ORelationTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    // Aus selektierten Feldnamen LineDataObject setzen
    // check if relation already exists
    OTableWindow* pSourceWin = jxdSource.pListBox->GetTabWin();
    OTableWindow* pDestWin = jxdDest.pListBox->GetTabWin();

    ::std::vector<OTableConnection*>::const_iterator aIter = getTableConnections()->begin();
    ::std::vector<OTableConnection*>::const_iterator aEnd = getTableConnections()->end();
    for(;aIter != aEnd;++aIter)
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
    OUString sSourceFieldName = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    OUString sDestFieldName = jxdDest.pListBox->GetEntryText(jxdDest.pEntry);

    // die Anzahl der PKey-Felder in der Quelle
    const Reference< XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(pSourceWin->GetData()->getTable());
    bool bAskUser = xPrimaryKeyColumns.is() && Reference< XIndexAccess>(xPrimaryKeyColumns,UNO_QUERY)->getCount() > 1;

    pTabConnData->SetConnLine( 0, sSourceFieldName, sDestFieldName );

    if ( bAskUser || m_pExistingConnection )
        m_pCurrentlyTabConnData = pTabConnData; // this implies that we ask the user what to do
    else
    {
        try
        {
            // Daten der Datenbank uebergeben
            if( pTabConnData->Update() )
            {
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
            OSL_FAIL("ORelationTableView::AddConnection: Exception oocured!");
        }
    }
}

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

void ORelationTableView::AddNewRelation()
{
    DBG_CHKTHIS(ORelationTableView,NULL);

    TTableConnectionData::value_type pNewConnData( new ORelationTableConnectionData() );
    ORelationDialog aRelDlg(this, pNewConnData, sal_True);

    sal_Bool bSuccess = (aRelDlg.Execute() == RET_OK);
    if (bSuccess)
    {
        // already updated by the dialog
        // dem Dokument bekanntgeben
        addConnection( new ORelationTableConnection(this, pNewConnData) );
    }
}

bool ORelationTableView::RemoveConnection( OTableConnection* pConn ,sal_Bool /*_bDelete*/)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    ORelationTableConnectionData* pTabConnData = (ORelationTableConnectionData*)pConn->GetData().get();
    try
    {
        if ( m_bInRemove || pTabConnData->DropRelation())
            return OJoinTableView::RemoveConnection( pConn ,sal_True);
    }
    catch(SQLException& e)
    {
        getDesignView()->getController().showError(SQLExceptionInfo(e));
    }
    catch(Exception&)
    {
        OSL_FAIL("ORelationTableView::RemoveConnection: Something other than SQLException occurred!");
    }
    return false;
}

void ORelationTableView::AddTabWin(const OUString& _rComposedName, const OUString& rWinName, sal_Bool /*bNewTable*/)
{
    DBG_CHKTHIS(ORelationTableView,NULL);
    OSL_ENSURE(!_rComposedName.isEmpty(),"There must be a table name supplied!");
    OJoinTableView::OTableWindowMap::iterator aIter = GetTabWinMap()->find(_rComposedName);

    if(aIter != GetTabWinMap()->end())
    {
        aIter->second->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
        aIter->second->GrabFocus();
        EnsureVisible(aIter->second);
        // no new one
        return;
    }

    // Neue Datenstruktur in DocShell eintragen
    TTableWindowData::value_type pNewTabWinData(createTableWindowData( _rComposedName, rWinName,rWinName ));
    pNewTabWinData->ShowAll(sal_False);

    // Neues Fenster in Fensterliste eintragen
    OTableWindow* pNewTabWin = createWindow( pNewTabWinData );
    if(pNewTabWin->Init())
    {
        m_pView->getController().getTableWindowData()->push_back( pNewTabWinData);
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

void ORelationTableView::RemoveTabWin( OTableWindow* pTabWin )
{
    OSQLWarningBox aDlg( this, ModuleRes( STR_QUERY_REL_DELETE_WINDOW ), WB_YES_NO | WB_DEF_YES );
    if ( m_bInRemove || aDlg.Execute() == RET_YES )
    {
        m_pView->getController().ClearUndoManager();
        OJoinTableView::RemoveTabWin( pTabWin );

        m_pView->getController().InvalidateFeature(SID_RELATION_ADD_RELATION);
        m_pView->getController().InvalidateFeature(ID_BROWSER_UNDO);
        m_pView->getController().InvalidateFeature(ID_BROWSER_REDO);
    }
}

void ORelationTableView::lookForUiActivities()
{
    if(m_pExistingConnection)
    {
        String sTitle(ModuleRes(STR_RELATIONDESIGN));
        sTitle.Erase(0,3);
        OSQLMessageBox aDlg(this,ModuleRes(STR_QUERY_REL_EDIT_RELATION),String(),0);
        aDlg.SetText(sTitle);
        aDlg.RemoveButton(aDlg.GetButtonId(0));
        aDlg.AddButton( ModuleRes(STR_QUERY_REL_EDIT), RET_OK, BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON);
        aDlg.AddButton( ModuleRes(STR_QUERY_REL_CREATE), RET_YES, 0);
        aDlg.AddButton(BUTTON_CANCEL,RET_CANCEL,0);
        sal_uInt16 nRet = aDlg.Execute();
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

OTableWindow* ORelationTableView::createWindow(const TTableWindowData::value_type& _pData)
{
    return new ORelationTableWindow(this,_pData);
}

bool ORelationTableView::allowQueries() const
{
    return false;
}

void ORelationTableView::_elementInserted( const container::ContainerEvent& /*_rEvent*/ )  throw(::com::sun::star::uno::RuntimeException)
{

}

void ORelationTableView::_elementRemoved( const container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    m_bInRemove = true;
    OUString sName;
    if ( _rEvent.Accessor >>= sName )
    {
        OTableWindow* pTableWindow = GetTabWindow(sName);
        if ( pTableWindow )
        {
            m_pView->getController().ClearUndoManager();
            OJoinTableView::RemoveTabWin( pTableWindow );

            m_pView->getController().InvalidateFeature(SID_RELATION_ADD_RELATION);
            m_pView->getController().InvalidateFeature(ID_BROWSER_UNDO);
            m_pView->getController().InvalidateFeature(ID_BROWSER_REDO);
        }
    }
    m_bInRemove = false;
}

void ORelationTableView::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ ) throw(::com::sun::star::uno::RuntimeException)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
