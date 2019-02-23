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

#include <RelationTableView.hxx>
#include <JoinExchange.hxx>
#include <core_resource.hxx>
#include <browserids.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <stringconstants.hxx>
#include <connectivity/dbtools.hxx>
#include <helpids.h>
#include <RelationDesignView.hxx>
#include <JoinController.hxx>
#include <TableWindow.hxx>
#include <TableWindowData.hxx>
#include "RTableConnection.hxx"
#include <RTableConnectionData.hxx>
#include <RelationDlg.hxx>
#include <sqlmessage.hxx>
#include <strings.hrc>
#include <UITools.hxx>
#include <connectivity/dbexception.hxx>
#include "RTableWindow.hxx"
#include <JAccess.hxx>
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
ORelationTableView::ORelationTableView( vcl::Window* pParent, ORelationDesignView* pView )
    :OJoinTableView( pParent, pView )
    , ::comphelper::OContainerListener(m_aMutex)
    ,m_pExistingConnection(nullptr)
    ,m_bInRemove(false)

{
    SetHelpId(HID_CTL_RELATIONTAB);
}

ORelationTableView::~ORelationTableView()
{
    disposeOnce();
}

void ORelationTableView::dispose()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
    m_pExistingConnection.clear();
    OJoinTableView::dispose();
}

void ORelationTableView::ReSync()
{
    if ( !m_pContainerListener.is() )
    {
        Reference< XConnection> xConnection = m_pView->getController().getConnection();
        Reference< XTablesSupplier > xTableSupp( xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xTables = xTableSupp->getTables();
        Reference< XContainer> xContainer(xTables,uno::UNO_QUERY);
        if ( xContainer.is() )
            m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
    }
    // Tables could have been hidden in the database, which are part of a relation. Or a table was in layout
    // (quite often without a relation) and does not exist anymore. In both cases creation of TabWins will fail
    // and all TabWinDatas and related ConnDates should be deleted.
    std::vector< OUString> arrInvalidTables;

    // create and insert windows
    TTableWindowData& rTabWinDataList = m_pView->getController().getTableWindowData();
    TTableWindowData::const_reverse_iterator aIter = rTabWinDataList.rbegin();
    for(;aIter != rTabWinDataList.rend();++aIter)
    {
        TTableWindowData::value_type pData = *aIter;
        VclPtr<OTableWindow> pTabWin = createWindow(pData);

        if (!pTabWin->Init())
        {
            // initialisation failed, which means this TabWin is not available, therefore,
            // it should be cleaned up, including its data in the document
            pTabWin->clearListBox();
            pTabWin.disposeAndClear();
            arrInvalidTables.push_back(pData->GetTableName());

            rTabWinDataList.erase( std::remove(rTabWinDataList.begin(), rTabWinDataList.end(), *aIter), rTabWinDataList.end());
            continue;
        }

        GetTabWinMap()[pData->GetComposedName()] = pTabWin;  // insert at the beginning, as the Datalist is walked through backward
        // if there's no position or size contained in the data -> Default
        if (!pData->HasPosition() && !pData->HasSize())
            SetDefaultTabWinPosSize(pTabWin);

        pTabWin->Show();
    }

    // insert connection
    TTableConnectionData& rTabConnDataList = m_pView->getController().getTableConnectionData();
    TTableConnectionData::const_reverse_iterator aConIter = rTabConnDataList.rbegin();

    for(;aConIter != rTabConnDataList.rend();++aConIter)
    {
        ORelationTableConnectionData* pTabConnData = static_cast<ORelationTableConnectionData*>(aConIter->get());
        if ( !arrInvalidTables.empty() )
        {
            // do the tables to the  connection exist?
            OUString strTabExistenceTest = pTabConnData->getReferencingTable()->GetTableName();
            bool bInvalid = std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();
            strTabExistenceTest = pTabConnData->getReferencedTable()->GetTableName();
            bInvalid = bInvalid || std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();

            if (bInvalid)
            {
                // no -> bad luck, the connection is gone
                rTabConnDataList.erase( std::remove(rTabConnDataList.begin(), rTabConnDataList.end(), *aConIter), rTabConnDataList.end() );
                continue;
            }
        }

        addConnection( VclPtr<ORelationTableConnection>::Create(this, *aConIter), false );
    }

    if ( !GetTabWinMap().empty() )
        GetTabWinMap().begin()->second->GrabFocus();
}

bool ORelationTableView::IsAddAllowed()
{

    return !m_pView->getController().isReadOnly();
}

void ORelationTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    // Set LineDataObject based on selected fieldname
    // check if relation already exists
    OTableWindow* pSourceWin = jxdSource.pListBox->GetTabWin();
    OTableWindow* pDestWin = jxdDest.pListBox->GetTabWin();

    for(VclPtr<OTableConnection> const & pFirst : getTableConnections())
    {
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

    // the names of the affected fields
    OUString sSourceFieldName = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    OUString sDestFieldName = jxdDest.pListBox->GetEntryText(jxdDest.pEntry);

    // the number of PKey-Fields in the source
    const Reference< XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(pSourceWin->GetData()->getTable());
    bool bAskUser = xPrimaryKeyColumns.is() && Reference< XIndexAccess>(xPrimaryKeyColumns,UNO_QUERY)->getCount() > 1;

    pTabConnData->SetConnLine( 0, sSourceFieldName, sDestFieldName );

    if ( bAskUser || m_pExistingConnection )
        m_pCurrentlyTabConnData = pTabConnData; // this implies that we ask the user what to do
    else
    {
        try
        {
            // hand over data to the database
            if( pTabConnData->Update() )
            {
                // enter UI-object into ConnList
                addConnection( VclPtr<ORelationTableConnection>::Create( this, pTabConnData ) );
            }
        }
        catch(const SQLException&)
        {
            throw;
        }
        catch(const Exception&)
        {
            OSL_FAIL("ORelationTableView::AddConnection: Exception occurred!");
        }
    }
}

void ORelationTableView::ConnDoubleClicked(VclPtr<OTableConnection>& rConnection)
{
    ScopedVclPtrInstance< ORelationDialog > aRelDlg( this, rConnection->GetData() );
    switch (aRelDlg->Execute())
    {
        case RET_OK:
            // successfully updated
            rConnection->UpdateLineList();
            // The connection references 1 ConnData and n ConnLines, each ConnData references n LineDatas, each Line exactly 1 LineData
            // As the Dialog and the ConnData->Update may have changed the LineDatas we have to restore the consistent state
            break;

        case RET_NO:
            // tried at least one update, but did not succeed -> the original connection is lost
            RemoveConnection(rConnection ,true);
            break;

        case RET_CANCEL:
            // no break, as nothing happened and we don't need the code below
            return;

    }

    Invalidate(InvalidateFlags::NoChildren);
}

void ORelationTableView::AddNewRelation()
{

    TTableConnectionData::value_type pNewConnData( new ORelationTableConnectionData() );
    ScopedVclPtrInstance< ORelationDialog > aRelDlg(this, pNewConnData, true);

    bool bSuccess = (aRelDlg->Execute() == RET_OK);
    if (bSuccess)
    {
        // already updated by the dialog
        // announce it to the document
        addConnection( VclPtr<ORelationTableConnection>::Create(this, pNewConnData) );
    }
}

bool ORelationTableView::RemoveConnection(VclPtr<OTableConnection>& rConn, bool /*_bDelete*/)
{
    ORelationTableConnectionData* pTabConnData = static_cast<ORelationTableConnectionData*>(rConn->GetData().get());
    try
    {
        if (!m_bInRemove)
            pTabConnData->DropRelation();
        return OJoinTableView::RemoveConnection(rConn, true);
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

void ORelationTableView::AddTabWin(const OUString& _rComposedName, const OUString& rWinName, bool /*bNewTable*/)
{
    OSL_ENSURE(!_rComposedName.isEmpty(),"There must be a table name supplied!");
    OJoinTableView::OTableWindowMap::const_iterator aIter = GetTabWinMap().find(_rComposedName);

    if(aIter != GetTabWinMap().end())
    {
        aIter->second->SetZOrder(nullptr, ZOrderFlags::First);
        aIter->second->GrabFocus();
        EnsureVisible(aIter->second);
        // no new one
        return;
    }

    // enter the new data structure into DocShell
    TTableWindowData::value_type pNewTabWinData(createTableWindowData( _rComposedName, rWinName,rWinName ));
    pNewTabWinData->ShowAll(false);

    // link new window into the window list
    VclPtr<OTableWindow> pNewTabWin = createWindow( pNewTabWinData );
    if(pNewTabWin->Init())
    {
        m_pView->getController().getTableWindowData().push_back( pNewTabWinData);
        // when we already have a table with this name insert the full qualified one instead
        GetTabWinMap()[_rComposedName] = pNewTabWin;

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
        pNewTabWin.disposeAndClear();
    }
}

void ORelationTableView::RemoveTabWin( OTableWindow* pTabWin )
{
    OSQLWarningBox aDlg(GetFrameWeld(), DBA_RES(STR_QUERY_REL_DELETE_WINDOW), MessBoxStyle::YesNo | MessBoxStyle::DefaultYes);
    if (m_bInRemove || aDlg.run() == RET_YES)
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
        OUString sTitle(DBA_RES(STR_RELATIONDESIGN));
        sTitle = sTitle.copy(3);
        OSQLMessageBox aDlg(GetFrameWeld(), DBA_RES(STR_QUERY_REL_EDIT_RELATION), OUString(), MessBoxStyle::NONE);
        aDlg.set_title(sTitle);
        aDlg.add_button(DBA_RES(STR_QUERY_REL_EDIT), RET_OK);
        aDlg.set_default_response(RET_OK);
        aDlg.add_button(DBA_RES(STR_QUERY_REL_CREATE), RET_YES);
        aDlg.add_button(Button::GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
        sal_uInt16 nRet = aDlg.run();
        if (nRet == RET_CANCEL)
        {
            m_pCurrentlyTabConnData.reset();
        }
        else if ( nRet == RET_OK ) // EDIT
        {
            ConnDoubleClicked(m_pExistingConnection);
            m_pCurrentlyTabConnData.reset();
        }
        m_pExistingConnection = nullptr;
    }
    if(m_pCurrentlyTabConnData)
    {
        ScopedVclPtrInstance< ORelationDialog > aRelDlg( this, m_pCurrentlyTabConnData );
        if (aRelDlg->Execute() == RET_OK)
        {
            // already updated by the dialog
            addConnection( VclPtr<ORelationTableConnection>::Create( this, m_pCurrentlyTabConnData ) );
        }
        m_pCurrentlyTabConnData.reset();
    }
}

VclPtr<OTableWindow> ORelationTableView::createWindow(const TTableWindowData::value_type& _pData)
{
    return VclPtr<ORelationTableWindow>::Create(this,_pData);
}

bool ORelationTableView::allowQueries() const
{
    return false;
}

void ORelationTableView::_elementInserted( const container::ContainerEvent& /*_rEvent*/ )
{

}

void ORelationTableView::_elementRemoved( const container::ContainerEvent& _rEvent )
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

void ORelationTableView::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
