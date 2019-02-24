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

#include <QueryTableView.hxx>
#include "TableFieldInfo.hxx"
#include <TableFieldDescription.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <helpids.h>
#include "QTableWindow.hxx"
#include "QTableConnection.hxx"
#include "QTableConnectionData.hxx"
#include <QueryDesignView.hxx>
#include <querycontroller.hxx>
#include "QueryAddTabConnUndoAction.hxx"
#include "QueryTabWinShowUndoAct.hxx"
#include <browserids.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <JAccess.hxx>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <stringconstants.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/sequence.hxx>
#include "querydlg.hxx"
#include <JoinExchange.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <vcl/treelistentry.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;

namespace
{
    /** appends a new TabAdd Undo action at controller
        @param  _pView          the view which we use
        @param  _pUndoAction    the undo action which should be added
        @param  _pConnection    the connection for which the undo action should be appended
        @param  _bOwner         is the undo action the owner
    */
    void addUndoAction( OQueryTableView const * _pView,
                        std::unique_ptr<OQueryTabConnUndoAction> _pUndoAction,
                        OQueryTableConnection* _pConnection,
                        bool _bOwner = false)
    {
        _pUndoAction->SetOwnership(_bOwner);
        _pUndoAction->SetConnection(_pConnection);
        _pView->getDesignView()->getController().addUndoActionAndInvalidate(std::move(_pUndoAction));
    }
    /** openJoinDialog opens the join dialog with this connection data
        @param  _pView              the view which we use
        @param  _pConnectionData    the connection data

        @return true when OK was pressed otherwise false
    */
    bool openJoinDialog(OQueryTableView* _pView,const TTableConnectionData::value_type& _pConnectionData,bool _bSelectableTables)
    {
        OQueryTableConnectionData* pData = static_cast< OQueryTableConnectionData*>(_pConnectionData.get());

        ScopedVclPtrInstance< DlgQryJoin > aDlg(_pView,_pConnectionData,&_pView->GetTabWinMap(),_pView->getDesignView()->getController().getConnection(),_bSelectableTables);
        bool bOk = aDlg->Execute() == RET_OK;
        if( bOk )
        {
            pData->SetJoinType(aDlg->GetJoinType());
            _pView->getDesignView()->getController().setModified(true);
        }

        return bOk;
    }
    /** connectionModified adds an undo action for the modified connection and forces an redraw
        @param  _pView              the view which we use
        @param  _pConnection    the connection which was modified
        @param  _bAddUndo       true when an undo action should be appended
    */
    void connectionModified(OQueryTableView* _pView,
                            OTableConnection* _pConnection,
                            bool _bAddUndo)
    {
        OSL_ENSURE(_pConnection,"Invalid connection!");
        _pConnection->UpdateLineList();

        // add an undo action
        if ( _bAddUndo )
            addUndoAction(  _pView,
                            std::make_unique<OQueryAddTabConnUndoAction>(_pView),
                            static_cast< OQueryTableConnection*>(_pConnection));
        // redraw
        _pConnection->RecalcLines();
        // force an invalidation of the bounding rectangle
        _pConnection->InvalidateConnection();

        _pView->Invalidate(InvalidateFlags::NoChildren);
    }
    void addConnections(OQueryTableView* _pView,
                        const OQueryTableWindow& _rSource,
                        const OQueryTableWindow& _rDest,
                        const Reference<XNameAccess>& _rxSourceForeignKeyColumns)
    {
        if ( _rSource.GetData()->isQuery() || _rDest.GetData()->isQuery() )
            // nothing to do if one of both denotes a query
            return;

        // we found a table in our view where we can insert some connections
        // the key columns have a property called RelatedColumn
        // build OQueryTableConnectionData
        OQueryTableConnectionData* pNewConnData = new OQueryTableConnectionData( _rSource.GetData(), _rDest.GetData() );
        TTableConnectionData::value_type aNewConnData(pNewConnData);

        Reference<XIndexAccess> xReferencedKeys( _rDest.GetData()->getKeys());
        OUString sRelatedColumn;

        // iterate through all foreignkey columns to create the connections
        for(const OUString& rElement : _rxSourceForeignKeyColumns->getElementNames())
        {
            Reference<XPropertySet> xColumn;
            if ( !( _rxSourceForeignKeyColumns->getByName(rElement) >>= xColumn ) )
            {
                OSL_FAIL( "addConnections: invalid foreign key column!" );
                continue;
            }

            xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN) >>= sRelatedColumn;

            {
                sal_Int32 nFindIndex = ::comphelper::findValue(_rSource.GetOriginalColumns()->getElementNames(),rElement);
                if(nFindIndex != -1)
                    pNewConnData->SetFieldIndex(JTCS_FROM,nFindIndex+1);
                else
                    OSL_FAIL("Column not found!");
            }
            // get the position inside the table
            Reference<XNameAccess> xRefColumns = _rDest.GetOriginalColumns();
            if(xRefColumns.is())
            {
                sal_Int32 nFindIndex = ::comphelper::findValue(xRefColumns->getElementNames(),sRelatedColumn);
                if(nFindIndex != -1)
                    pNewConnData->SetFieldIndex(JTCS_TO,nFindIndex+1);
                else
                    OSL_FAIL("Column not found!");
            }
            pNewConnData->AppendConnLine(rElement,sRelatedColumn);

            // now add the Conn itself
            ScopedVclPtrInstance< OQueryTableConnection > aNewConn(_pView, aNewConnData);
            // referring to the local variable is not important, as NotifyQueryTabConn creates a new copy
            // to add me (if not existent)
            _pView->NotifyTabConnection(*aNewConn, false);
                // don't create an Undo-Action for the new connection : the connection is
                // covered by the Undo-Action for the tabwin, as the "Undo the insert" will
                // automatically remove all connections adjacent to the win.
                // (Because of this automatism we would have an ownership ambiguity for
                // the connection data if we would insert the conn-Undo-Action)
        }
    }
}

// class OQueryTableView
OQueryTableView::OQueryTableView( vcl::Window* pParent,OQueryDesignView* pView)
    : OJoinTableView( pParent,pView)
{
    SetHelpId(HID_CTL_QRYDGNTAB);
}

sal_Int32 OQueryTableView::CountTableAlias(const OUString& rName, sal_Int32& rMax)
{
    sal_Int32 nRet = 0;

    OTableWindowMap::const_iterator aIter = GetTabWinMap().find(rName);
    while(aIter != GetTabWinMap().end())
    {
        OUString aNewName = rName + "_" + OUString::number(++nRet);
        aIter = GetTabWinMap().find(aNewName);
    }

    rMax = nRet;

    return nRet;
}

void OQueryTableView::ReSync()
{
    TTableWindowData& rTabWinDataList = m_pView->getController().getTableWindowData();
    OSL_ENSURE((getTableConnections().empty()) && (GetTabWinMap().empty()),
        "before calling OQueryTableView::ReSync() please call ClearAll !");

    // I need a collection of all window names that cannot be created so that I do not initialize connections for them.
    std::vector<OUString> arrInvalidTables;

    TTableWindowData::const_reverse_iterator aIter = rTabWinDataList.rbegin();
    // Create the window and add it

    for(;aIter != rTabWinDataList.rend();++aIter)
    {
        OQueryTableWindowData* pData = static_cast<OQueryTableWindowData*>(aIter->get());
        VclPtr<OTableWindow> pTabWin = createWindow(*aIter);

        // I don't use ShowTabWin as this adds the window data to the list of documents.
        // This would be bad as I am getting them from there.
        // Instead, I do it step by step
        if (!pTabWin->Init())
        {
            // The initialisation has gone wrong, this TabWin is not available, so
            // I must clean up the data and the document
            pTabWin->clearListBox();
            pTabWin.disposeAndClear();
            arrInvalidTables.push_back(pData->GetAliasName());

            rTabWinDataList.erase( std::remove(rTabWinDataList.begin(), rTabWinDataList.end(), *aIter), rTabWinDataList.end());
            continue;
        }

        GetTabWinMap()[pData->GetAliasName()] = pTabWin; // add at the beginning as I am going backwards through the DataList
        // Use the default if there is no position or size
        if (!pData->HasPosition() && !pData->HasSize())
            SetDefaultTabWinPosSize(pTabWin);

        pTabWin->Show();
    }

    // Add the connections
    TTableConnectionData& rTabConnDataList = m_pView->getController().getTableConnectionData();
    TTableConnectionData::const_reverse_iterator aConIter = rTabConnDataList.rbegin();

    for(;aConIter != rTabConnDataList.rend();++aConIter)
    {
        OQueryTableConnectionData* pTabConnData =  static_cast<OQueryTableConnectionData*>(aConIter->get());

        // do both tables for the connection exist ?
        OUString strTabExistenceTest = pTabConnData->getReferencingTable()->GetWinName();
        bool bInvalid = std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();
        strTabExistenceTest = pTabConnData->getReferencedTable()->GetWinName();
        bInvalid = bInvalid && std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();

        if (bInvalid)
        {
            // no -> bad luck, no connection
            rTabConnDataList.erase( std::remove(rTabConnDataList.begin(), rTabConnDataList.end(), *aConIter), rTabConnDataList.end());
            continue;
        }

        // adds a new connection to join view and notifies our accessible and invalidates the controller
        addConnection(VclPtr<OQueryTableConnection>::Create(this, *aConIter));
    }
}

void OQueryTableView::ClearAll()
{
    OJoinTableView::ClearAll();

    SetUpdateMode(true);
    m_pView->getController().setModified(true);
}

VclPtr<OTableWindow> OQueryTableView::createWindow(const TTableWindowData::value_type& _pData)
{
    return VclPtr<OQueryTableWindow>::Create(this,_pData);
}

void OQueryTableView::NotifyTabConnection(const OQueryTableConnection& rNewConn, bool _bCreateUndoAction)
{
    // let's first check if I have the connection already
    OQueryTableConnection* pTabConn = nullptr;
    const auto& rConnections = getTableConnections();
    auto aEnd = rConnections.end();
    auto aIter = std::find(   rConnections.begin(),
                                aEnd,
                                VclPtr<OTableConnection>(const_cast<OTableConnection*>(static_cast<const OTableConnection*>(&rNewConn)))
                            );
    if(aIter == aEnd)
    {
        for (auto const& connection : rConnections)
        {
            if(*static_cast<OQueryTableConnection*>(connection.get()) == rNewConn)
            {
                pTabConn = static_cast<OQueryTableConnection*>(connection.get());
                break;
            }
        }
    }
    else
        pTabConn = static_cast<OQueryTableConnection*>((*aIter).get());

    // no -> insert
    if (pTabConn == nullptr)
    {
        // the new data ...
        OQueryTableConnectionData* pNewData = static_cast< OQueryTableConnectionData*>(rNewConn.GetData()->NewInstance());
        pNewData->CopyFrom(*rNewConn.GetData());
        TTableConnectionData::value_type aData(pNewData);
        VclPtrInstance<OQueryTableConnection> pNewConn(this, aData);
        GetConnection(pNewConn);

        connectionModified(this,pNewConn,_bCreateUndoAction);
    }
}

OTableWindowData* OQueryTableView::CreateImpl(const OUString& _rComposedName
                                             ,const OUString& _sTableName
                                             ,const OUString& _rWinName)
{
    return new OQueryTableWindowData( _rComposedName, _sTableName,_rWinName );
}

void OQueryTableView::AddTabWin(const OUString& _rTableName, const OUString& _rAliasName, bool bNewTable)
{
    // this method has been inherited from the base class, linking back to the parent and which constructs
    // an Alias and which passes on to my other AddTabWin

    // pity _rTableName is fully qualified, OQueryDesignView expects a string which only
    // contains schema and tables but no catalog.
    Reference< XConnection> xConnection = m_pView->getController().getConnection();
    if(!xConnection.is())
        return;
    try
    {
        Reference< XDatabaseMetaData > xMetaData = xConnection->getMetaData();
        OUString sCatalog, sSchema, sTable;
        ::dbtools::qualifiedNameComponents(xMetaData,
                                    _rTableName,
                                    sCatalog,
                                    sSchema,
                                    sTable,
                                    ::dbtools::EComposeRule::InDataManipulation);
        OUString sRealName(sSchema);
        if (!sRealName.isEmpty())
            sRealName += ".";
        sRealName += sTable;

        AddTabWin(_rTableName, sRealName, _rAliasName, bNewTable);
    }
    catch(SQLException&)
    {
        OSL_FAIL("qualifiedNameComponents");
    }
}

// find the table which has a foreign key with this referencedTable name
static Reference<XPropertySet> getKeyReferencedTo(const Reference<XIndexAccess>& _rxKeys,const OUString& _rReferencedTable)
{
    if(!_rxKeys.is())
        return Reference<XPropertySet>();

    // search the one and only primary key
    const sal_Int32 nCount = _rxKeys->getCount();
    for(sal_Int32 i=0;i<nCount ;++i)
    {
        Reference<XPropertySet> xKey(_rxKeys->getByIndex(i),UNO_QUERY);
        if(xKey.is())
        {
            sal_Int32 nKeyType = 0;
            xKey->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
            if(KeyType::FOREIGN == nKeyType)
            {
                OUString sReferencedTable;
                xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
                // TODO check case
                if(sReferencedTable == _rReferencedTable)
                    return xKey;
            }
        }
    }
    return Reference<XPropertySet>();
}

void OQueryTableView::AddTabWin(const OUString& _rComposedName, const OUString& _rTableName, const OUString& strAlias, bool bNewTable)
{
    OSL_ENSURE(!_rTableName.isEmpty() || !strAlias.isEmpty(), "OQueryTableView::AddTabWin : no tables or aliases !");
        // If the table is not set, then it is a dummy window, but at least the alias must be set

    // build a new data structure
    // first check if this already has its data
    bool bAppend = bNewTable;
    TTableWindowData::value_type pNewTabWinData;
    TTableWindowData& rWindowData = getDesignView()->getController().getTableWindowData();
    bool bFoundElem = false;
    for (auto const& elem : rWindowData)
    {
        pNewTabWinData = elem;
        if (pNewTabWinData && pNewTabWinData->GetWinName() == strAlias && pNewTabWinData->GetComposedName() == _rComposedName && pNewTabWinData->GetTableName() == _rTableName)
        {
            bFoundElem = true;
            break;
        }
    }
    if ( !bAppend )
        bAppend = !bFoundElem;
    if ( bAppend )
        pNewTabWinData = createTableWindowData(_rComposedName, _rTableName, strAlias);
        // I do not need to add TabWinData to the DocShell list, ShowTabWin does that.

    // Create a new window
    VclPtr<OQueryTableWindow> pNewTabWin = static_cast<OQueryTableWindow*>(createWindow(pNewTabWinData).get());
    // No need to initialize, as that happens in ShowTabWin

    // New UndoAction
    std::unique_ptr<OQueryTabWinShowUndoAct> pUndoAction(new OQueryTabWinShowUndoAct(this));
    pUndoAction->SetTabWin(pNewTabWin); // Window
    bool bSuccess = ShowTabWin(pNewTabWin, pUndoAction.get(), bAppend);
    if(!bSuccess)
    {
        // reset table window
        pUndoAction->SetTabWin(nullptr);
        pUndoAction->SetOwnership(false);
        return;
    }

    // Show the relations between the individual tables
    OTableWindowMap& rTabWins = GetTabWinMap();
    if(bNewTable && !rTabWins.empty() && !_rTableName.isEmpty())
    {
        modified();
        if ( m_pAccessible )
            m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                    Any(),
                                                    makeAny(pNewTabWin->GetAccessible())
                                                    );

        do {

        if ( pNewTabWin->GetData()->isQuery() )
            break;

        try
        {
            // find relations between the table an the tables already inserted
            Reference< XIndexAccess> xKeyIndex = pNewTabWin->GetData()->getKeys();
            if ( !xKeyIndex.is() )
                break;

            Reference<XNameAccess> xFKeyColumns;
            OUString aReferencedTable;
            Reference<XColumnsSupplier> xColumnsSupplier;

            const sal_Int32 nKeyCount = xKeyIndex->getCount();
            for ( sal_Int32 i=0; i<nKeyCount ; ++i )
            {
                Reference< XPropertySet > xProp( xKeyIndex->getByIndex(i), UNO_QUERY_THROW );
                xColumnsSupplier.set( xProp, UNO_QUERY_THROW );
                xFKeyColumns.set( xColumnsSupplier->getColumns(), UNO_QUERY_THROW );

                sal_Int32 nKeyType = 0;
                xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;

                switch ( nKeyType )
                {
                case KeyType::FOREIGN:
                {   // our new table has a foreign key
                    // so look if the referenced table is already in our list
                    xProp->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= aReferencedTable;
                    OSL_ENSURE(!aReferencedTable.isEmpty(),"Foreign key without referencedTableName");

                    OTableWindowMap::const_iterator aIter = rTabWins.find(aReferencedTable);
                    OTableWindowMap::const_iterator aEnd  = rTabWins.end();
                    if(aIter == aEnd)
                    {
                        for(aIter = rTabWins.begin();aIter != aEnd;++aIter)
                        {
                            OQueryTableWindow* pTabWinTmp = static_cast<OQueryTableWindow*>(aIter->second.get());
                            OSL_ENSURE( pTabWinTmp,"TableWindow is null!" );
                            if ( pTabWinTmp != pNewTabWin && pTabWinTmp->GetComposedName() == aReferencedTable )
                                break;
                        }
                    }
                    if ( aIter != aEnd && pNewTabWin.get() != aIter->second.get() )
                        addConnections( this, *pNewTabWin, *static_cast<OQueryTableWindow*>(aIter->second.get()), xFKeyColumns );
                }
                break;

                case KeyType::PRIMARY:
                {
                    // we have a primary key so look in our list if there exists a key which this is referred to
                    for (auto const& tabWin : rTabWins)
                    {
                        OQueryTableWindow* pTabWinTmp = static_cast<OQueryTableWindow*>(tabWin.second.get());
                        if ( pTabWinTmp == pNewTabWin )
                            continue;

                        if ( pTabWinTmp->GetData()->isQuery() )
                            continue;

                        OSL_ENSURE(pTabWinTmp,"TableWindow is null!");
                        Reference< XPropertySet > xFKKey = getKeyReferencedTo( pTabWinTmp->GetData()->getKeys(), pNewTabWin->GetComposedName() );
                        if ( !xFKKey.is() )
                            continue;

                        Reference<XColumnsSupplier> xFKColumnsSupplier( xFKKey, UNO_QUERY_THROW );
                        Reference< XNameAccess > xTColumns( xFKColumnsSupplier->getColumns(), UNO_QUERY_THROW );
                        addConnections( this, *pTabWinTmp, *pNewTabWin, xTColumns );
                    }
                }
                break;
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        } while ( false );
    }

    // My parent needs to be informed about the delete
    m_pView->getController().addUndoActionAndInvalidate( std::move(pUndoAction) );
}

void OQueryTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    OQueryTableWindow* pSourceWin = static_cast< OQueryTableWindow*>(jxdSource.pListBox->GetTabWin());
    OQueryTableWindow* pDestWin = static_cast< OQueryTableWindow*>(jxdDest.pListBox->GetTabWin());

    OUString aSourceFieldName, aDestFieldName;
    aSourceFieldName    = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    aDestFieldName      = jxdDest.pListBox->GetEntryText(jxdDest.pEntry);

    OTableConnection* pConn = GetTabConn(pSourceWin,pDestWin,true);
    if ( !pConn )
    {
        // new data object
        OQueryTableConnectionData* pNewConnectionData = new OQueryTableConnectionData(pSourceWin->GetData(), pDestWin->GetData());
        TTableConnectionData::value_type aNewConnectionData(pNewConnectionData);

        sal_uInt32          nSourceFieldIndex, nDestFieldIndex;

        // Get name/position of both affected fields ...
        // Source
        nSourceFieldIndex = jxdSource.pListBox->GetModel()->GetAbsPos(jxdSource.pEntry);
        // Dest
        nDestFieldIndex = jxdDest.pListBox->GetModel()->GetAbsPos(jxdDest.pEntry);

        // ... and set them
        pNewConnectionData->SetFieldIndex(JTCS_FROM, nSourceFieldIndex);
        pNewConnectionData->SetFieldIndex(JTCS_TO, nDestFieldIndex);

        pNewConnectionData->AppendConnLine( aSourceFieldName,aDestFieldName );

        ScopedVclPtrInstance< OQueryTableConnection > aNewConnection(this, aNewConnectionData);
        NotifyTabConnection(*aNewConnection);
        // As usual with NotifyTabConnection, using a local variable is fine because a copy is made
    }
    else
    {
        // the connection could point on the other side
        if(pConn->GetSourceWin() == pDestWin)
        {
            OUString aTmp(aSourceFieldName);
            aSourceFieldName = aDestFieldName;
            aDestFieldName = aTmp;
        }

        pConn->GetData()->AppendConnLine( aSourceFieldName,aDestFieldName );

        connectionModified(this,pConn,false);
    }
}

void OQueryTableView::ConnDoubleClicked(VclPtr<OTableConnection>& rConnection)
{
    if (openJoinDialog(this, rConnection->GetData(), false))
    {
        connectionModified(this, rConnection, false);
        SelectConn(rConnection);
    }
}

void OQueryTableView::createNewConnection()
{
    TTableConnectionData::value_type pData(new OQueryTableConnectionData());
    if( openJoinDialog(this,pData,true) )
    {
        OTableWindowMap& rMap = GetTabWinMap();
        OQueryTableWindow* pSourceWin   = static_cast< OQueryTableWindow*>(rMap[pData->getReferencingTable()->GetWinName()].get());
        OQueryTableWindow* pDestWin     = static_cast< OQueryTableWindow*>(rMap[pData->getReferencedTable()->GetWinName()].get());
        // first we have to look if the this connection already exists
        OTableConnection* pConn = GetTabConn(pSourceWin,pDestWin,true);
        bool bNew = true;
        if ( pConn )
        {
            pConn->GetData()->CopyFrom( *pData );
            bNew = false;
        }
        else
        {
            // create a new connection and append it
            VclPtrInstance<OQueryTableConnection> pQConn(this, pData);
            GetConnection(pQConn);
            pConn = pQConn;
        }
        connectionModified(this,pConn,bNew);
        if ( !bNew && pConn == GetSelectedConn() ) // our connection was selected before so we have to reselect it
            SelectConn( pConn );
    }
}

bool OQueryTableView::RemoveConnection(VclPtr<OTableConnection>& rConnection, bool /*_bDelete*/)
{
    VclPtr<OQueryTableConnection> xConnection(static_cast<OQueryTableConnection*>(rConnection.get()));

    // we don't want that our connection will be deleted, we put it in the undo manager
    bool bRet = OJoinTableView::RemoveConnection(rConnection, false);

    // add undo action
    addUndoAction(this,
                  std::make_unique<OQueryDelTabConnUndoAction>(this),
                  xConnection.get(),
                  true);

    return bRet;
}

OQueryTableWindow* OQueryTableView::FindTable(const OUString& rAliasName)
{
    OSL_ENSURE(!rAliasName.isEmpty(), "OQueryTableView::FindTable : the  AliasName should not be empty !");
        // (it is harmless but does not make sense and indicates that there is probably an error in the caller)
    OTableWindowMap::const_iterator aIter = GetTabWinMap().find(rAliasName);
    if(aIter != GetTabWinMap().end())
        return static_cast<OQueryTableWindow*>(aIter->second.get());
    return nullptr;
}

bool OQueryTableView::FindTableFromField(const OUString& rFieldName, OTableFieldDescRef const & rInfo, sal_uInt16& rCnt)
{
    rCnt = 0;
    for (auto const& tabWin : GetTabWinMap())
    {
        if(static_cast<OQueryTableWindow*>(tabWin.second.get())->ExistsField(rFieldName, rInfo))
            ++rCnt;
    }
    // TODO JNA : what should we rCnt > 1?

    return rCnt == 1;
}

bool OQueryTableView::ContainsTabWin(const OTableWindow& rTabWin)
{

    for (auto const& tabWin : GetTabWinMap())
    {
        if ( tabWin.second == &rTabWin )
        {
            return true;
        }
    }

    return false;
}

void OQueryTableView::RemoveTabWin(OTableWindow* pTabWin)
{
    OSL_ENSURE(pTabWin != nullptr, "OQueryTableView::RemoveTabWin : Window should not be NULL !");

    if(pTabWin && ContainsTabWin(*pTabWin)) // #i122589# check if registered before deleting
    {
        // I need my parent so it can be informed about the deletion
        OQueryDesignView* pParent = static_cast<OQueryDesignView*>(getDesignView());

        SfxUndoManager& rUndoMgr = m_pView->getController().GetUndoManager();
        rUndoMgr.EnterListAction(DBA_RES(STR_QUERY_UNDO_TABWINDELETE) , OUString(), 0, ViewShellId(-1));

        // add the Undo-Action
        std::unique_ptr<OQueryTabWinDelUndoAct> pUndoAction(new OQueryTabWinDelUndoAct(this));
        pUndoAction->SetTabWin(static_cast< OQueryTableWindow*>(pTabWin));

        // and hide the window
        HideTabWin(static_cast< OQueryTableWindow*>(pTabWin), pUndoAction.get());

        // Undo Actions and delete the fields in SelectionBrowseBox
        pParent->TableDeleted( static_cast< OQueryTableWindowData*>(pTabWin->GetData().get())->GetAliasName() );

        m_pView->getController().addUndoActionAndInvalidate( std::move(pUndoAction) );
        rUndoMgr.LeaveListAction();

        modified();
        if ( m_pAccessible )
            m_pAccessible->notifyAccessibleEvent(   AccessibleEventId::CHILD,
                                                    makeAny(pTabWin->GetAccessible()),
                                                    Any()
                                                    );
    }
}

void OQueryTableView::EnsureVisible(const OTableWindow* pWin)
{

    Invalidate(InvalidateFlags::NoChildren);
    OJoinTableView::EnsureVisible(pWin);
}

void OQueryTableView::GetConnection(OQueryTableConnection* pConn)
{
    // add to me and the document

    addConnection( pConn );
}

void OQueryTableView::DropConnection(VclPtr<OQueryTableConnection> const & rConn)
{
    // Pay attention to the selection
    // remove from me and the document
    VclPtr<OTableConnection> xConn(rConn.get());
    RemoveConnection(xConn, false);
}

void OQueryTableView::HideTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction )
{
    OTableWindowMap& rTabWins = GetTabWinMap();

    // Window
    // save the position in its data
    getDesignView()->SaveTabWinUIConfig(pTabWin);
    // (I need to go via the parent, as only the parent knows the position of the scrollbars)
    // and then out of the TabWins list and hide
    OTableWindowMap::const_iterator aIter = std::find_if(rTabWins.begin(), rTabWins.end(),
        [&pTabWin](const OTableWindowMap::value_type& rEntry) { return rEntry.second == pTabWin; });
    if (aIter != rTabWins.end())
        rTabWins.erase( aIter );

    pTabWin->Hide();    // do not destroy it, as it is still in the undo list!!

    // the TabWin data must also be passed out of my responsibility
    TTableWindowData& rTabWinDataList = m_pView->getController().getTableWindowData();
    rTabWinDataList.erase( std::remove(rTabWinDataList.begin(), rTabWinDataList.end(), pTabWin->GetData()), rTabWinDataList.end());
        // The data should not be destroyed as TabWin itself - which is still alive - needs them
        // Either it goes back into my responsibility, (via ShowTabWin), then I add the data back,
        // or the Undo-Action, which currently has full responsibility for the window
        // and its data, gets destroyed and destroys both the window and its data

    if (m_pLastFocusTabWin == pTabWin)
        m_pLastFocusTabWin = nullptr;

    // collect connections belonging to the window and pass to UndoAction
    sal_Int16 nCnt = 0;
    const auto& rTabConList = getTableConnections();
    auto aIter2 = rTabConList.begin();
    for(;aIter2 != rTabConList.end();)// the end may change
    {
        VclPtr<OTableConnection> xTmpEntry = *aIter2;
        OQueryTableConnection* pTmpEntry = static_cast<OQueryTableConnection*>(xTmpEntry.get());
        OSL_ENSURE(pTmpEntry,"OQueryTableConnection is null!");
        if( pTmpEntry->GetAliasName(JTCS_FROM) == pTabWin->GetAliasName() ||
            pTmpEntry->GetAliasName(JTCS_TO) == pTabWin->GetAliasName() )
        {
            // add to undo list
            pUndoAction->InsertConnection(xTmpEntry);

            // call base class because we append an undo action
            // but this time we are in a undo action list
            OJoinTableView::RemoveConnection(xTmpEntry, false);
            aIter2 = rTabConList.begin();
            ++nCnt;
        }
        else
            ++aIter2;
    }

    if (nCnt)
        InvalidateConnections();

    m_pView->getController().InvalidateFeature(ID_BROWSER_ADDTABLE);

    // inform the UndoAction that the window and connections belong to it
    pUndoAction->SetOwnership(true);

    // by doing so, we have modified the document
    m_pView->getController().setModified( true );
    m_pView->getController().InvalidateFeature(SID_BROWSER_CLEAR_QUERY);
}

bool OQueryTableView::ShowTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction, bool _bAppend )
{

    bool bSuccess = false;

    if (pTabWin)
    {
        if (pTabWin->Init())
        {
            TTableWindowData::value_type pData = pTabWin->GetData();
            OSL_ENSURE(pData != nullptr, "OQueryTableView::ShowTabWin : TabWin has no data !");
            // If there is a position and size defined, we use them
            if (pData->HasPosition() && pData->HasSize())
            {
                Size aSize(CalcZoom(pData->GetSize().Width()),CalcZoom(pData->GetSize().Height()));
                pTabWin->SetPosSizePixel(pData->GetPosition(), aSize);
            }
            else
                // else set a default position
                SetDefaultTabWinPosSize(pTabWin);

            // Show the window and add to the list
            OUString sName = static_cast< OQueryTableWindowData*>(pData.get())->GetAliasName();
            OSL_ENSURE(GetTabWinMap().find(sName) == GetTabWinMap().end(),"Alias name already in list!");
            GetTabWinMap().emplace(sName,pTabWin);

            pTabWin->Show();

            pTabWin->Update();
            // We must call Update() in order to show the connections in the window correctly. This sounds strange,
            // but the Listbox  has an internal Member which is initialized when the Listbox is first shown (after the Listbox
            // is filled in Init). This Member will eventually be needed for
            // GetEntryPos, and then in turn by the Connection, when its starting point to the window must be determined.

            // the Connections
            auto rTableCon = pUndoAction->GetTabConnList();
            for(const auto& conn : rTableCon)
                addConnection(conn); // add all connections from the undo action

            rTableCon.clear();

            // and add the window's data to the list (of the document)
            if(_bAppend)
                m_pView->getController().getTableWindowData().push_back(pTabWin->GetData());

            m_pView->getController().InvalidateFeature(ID_BROWSER_ADDTABLE);

            // and inform the UndoAction  that the window belongs to me
            pUndoAction->SetOwnership(false);

            bSuccess = true;
        }
        else
        {
            // Initialisation failed
            // (for example when the Connection to the database is not available at the moment)
            pTabWin->clearListBox();
            pTabWin->disposeOnce();
        }
    }

    // show that I have changed the document
    if(!m_pView->getController().isReadOnly())
        m_pView->getController().setModified( true );

    m_pView->getController().InvalidateFeature(SID_BROWSER_CLEAR_QUERY);

    return bSuccess;
}

void OQueryTableView::InsertField(const OTableFieldDescRef& rInfo)
{
    OSL_ENSURE(getDesignView() != nullptr, "OQueryTableView::InsertField : has no Parent !");
    static_cast<OQueryDesignView*>(getDesignView())->InsertField(rInfo);
}

bool OQueryTableView::ExistsAVisitedConn(const OQueryTableWindow* pFrom) const
{
    for(const auto& conn : getTableConnections())
    {
        OQueryTableConnection* pTemp = static_cast<OQueryTableConnection*>(conn.get());
        if (pTemp->IsVisited() &&
            (pFrom == static_cast< OQueryTableWindow*>(pTemp->GetSourceWin()) || pFrom == static_cast< OQueryTableWindow*>(pTemp->GetDestWin())))
            return true;
    }

    return false;
}

void OQueryTableView::onNoColumns_throw()
{
    OUString sError(DBA_RES(STR_STATEMENT_WITHOUT_RESULT_SET));
    ::dbtools::throwSQLException( sError, ::dbtools::StandardSQLState::GENERAL_ERROR, nullptr );
}

bool OQueryTableView::supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const
{
    OQueryTableConnectionData* pQueryData = static_cast<OQueryTableConnectionData*>(_pData.get());
    return pQueryData && (pQueryData->GetJoinType() == CROSS_JOIN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
