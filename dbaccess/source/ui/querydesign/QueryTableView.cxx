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

#include "QueryTableView.hxx"
#include "TableFieldInfo.hxx"
#include "TableFieldDescription.hxx"
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include "dbaccess_helpid.hrc"
#include "QTableWindow.hxx"
#include "QTableConnection.hxx"
#include "QTableConnectionData.hxx"
#include "QueryDesignView.hxx"
#include "querycontroller.hxx"
#include "QueryAddTabConnUndoAction.hxx"
#include "QueryTabWinShowUndoAct.hxx"
#include "browserids.hxx"
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include "JAccess.hxx"
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "dbustrings.hrc"
#include <connectivity/dbtools.hxx>
#include <comphelper/sequence.hxx>
#include "querydlg.hxx"
#include "JoinExchange.hxx"
#include <comphelper/extract.hxx>
#include "dbu_qry.hrc"
#include <vcl/msgbox.hxx>
#include "svtools/treelistentry.hxx"

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;

namespace
{
    sal_Bool isColumnInKeyType(const Reference<XIndexAccess>& _rxKeys,const OUString& _rColumnName,sal_Int32 _nKeyType)
    {
        sal_Bool bReturn = sal_False;
        if(_rxKeys.is())
        {
            Reference<XColumnsSupplier> xColumnsSupplier;
            // search the one and only primary key
            const sal_Int32 nCount = _rxKeys->getCount();
            for(sal_Int32 i=0;i< nCount;++i)
            {
                Reference<XPropertySet> xProp(_rxKeys->getByIndex(i),UNO_QUERY);
                if(xProp.is())
                {
                    sal_Int32 nKeyType = 0;
                    xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                    if(_nKeyType == nKeyType)
                    {
                        xColumnsSupplier.set(xProp,UNO_QUERY);
                        if(xColumnsSupplier.is())
                        {
                            Reference<XNameAccess> xColumns = xColumnsSupplier->getColumns();
                            if(xColumns.is() && xColumns->hasByName(_rColumnName))
                            {
                                bReturn = sal_True;
                                break;
                            }
                        }
                    }
                }
            }
        }
        return bReturn;
    }
    /** appends a new TabAdd Undo action at controller
        @param  _pView          the view which we use
        @param  _pUndoAction    the undo action which should be added
        @param  _pConnection    the connection for which the undo action should be appended
        @param  _bOwner         is the undo action the owner
    */
    void addUndoAction( OQueryTableView* _pView,
                        OQueryTabConnUndoAction* _pUndoAction,
                        OQueryTableConnection* _pConnection,
                        sal_Bool _bOwner = sal_False)
    {
        _pUndoAction->SetOwnership(_bOwner);
        _pUndoAction->SetConnection(_pConnection);
        _pView->getDesignView()->getController().addUndoActionAndInvalidate(_pUndoAction);
    }
    /** openJoinDialog opens the join dialog with this connection data
        @param  _pView              the view which we use
        @param  _pConnectionData    the connection data

        @return true when OK was pressed otherwise false
    */
    sal_Bool openJoinDialog(OQueryTableView* _pView,const TTableConnectionData::value_type& _pConnectionData,sal_Bool _bSelectableTables)
    {
        OQueryTableConnectionData* pData = static_cast< OQueryTableConnectionData*>(_pConnectionData.get());

        DlgQryJoin aDlg(_pView,_pConnectionData,_pView->GetTabWinMap(),_pView->getDesignView()->getController().getConnection(),_bSelectableTables);
        sal_Bool bOk = aDlg.Execute() == RET_OK;
        if( bOk )
        {
            pData->SetJoinType(aDlg.GetJoinType());
            _pView->getDesignView()->getController().setModified(sal_True);
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
                            sal_Bool _bAddUndo)
    {
        OSL_ENSURE(_pConnection,"Invalid connection!");
        _pConnection->UpdateLineList();

        // add an undo action
        if ( _bAddUndo )
            addUndoAction(  _pView,
                            new OQueryAddTabConnUndoAction(_pView),
                            static_cast< OQueryTableConnection*>(_pConnection));
        // redraw
        _pConnection->RecalcLines();
        // force an invalidation of the bounding rectangle
        _pConnection->InvalidateConnection();

        _pView->Invalidate(INVALIDATE_NOCHILDREN);
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
        // OQueryTableConnectionData aufbauen
        OQueryTableConnectionData* pNewConnData = new OQueryTableConnectionData( _rSource.GetData(), _rDest.GetData() );
        TTableConnectionData::value_type aNewConnData(pNewConnData);

        Reference<XIndexAccess> xReferencedKeys( _rDest.GetData()->getKeys());
        OUString sRelatedColumn;

        // iterate through all foreignkey columns to create the connections
        Sequence< OUString> aElements(_rxSourceForeignKeyColumns->getElementNames());
        const OUString* pIter = aElements.getConstArray();
        const OUString* pEnd   = pIter + aElements.getLength();
        for(sal_Int32 i=0;pIter != pEnd;++pIter,++i)
        {
            Reference<XPropertySet> xColumn;
            if ( !( _rxSourceForeignKeyColumns->getByName(*pIter) >>= xColumn ) )
            {
                OSL_FAIL( "addConnections: invalid foreign key column!" );
                continue;
            }

            pNewConnData->SetFieldType(JTCS_FROM,TAB_NORMAL_FIELD);

            xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN) >>= sRelatedColumn;
            pNewConnData->SetFieldType(JTCS_TO,isColumnInKeyType(xReferencedKeys,sRelatedColumn,KeyType::PRIMARY) ? TAB_PRIMARY_FIELD : TAB_NORMAL_FIELD);

            {
                Sequence< sal_Int16> aFind(::comphelper::findValue(_rSource.GetOriginalColumns()->getElementNames(),*pIter,sal_True));
                if(aFind.getLength())
                    pNewConnData->SetFieldIndex(JTCS_FROM,aFind[0]+1);
                else
                    OSL_FAIL("Column not found!");
            }
            // get the position inside the tabe
            Reference<XNameAccess> xRefColumns = _rDest.GetOriginalColumns();
            if(xRefColumns.is())
            {
                Sequence< sal_Int16> aFind(::comphelper::findValue(xRefColumns->getElementNames(),sRelatedColumn,sal_True));
                if(aFind.getLength())
                    pNewConnData->SetFieldIndex(JTCS_TO,aFind[0]+1);
                else
                    OSL_FAIL("Column not found!");
            }
            pNewConnData->AppendConnLine(*pIter,sRelatedColumn);

            // now add the Conn itself
            OQueryTableConnection aNewConn(_pView, aNewConnData);
            // refering to the local variable is not important, as NotifyQueryTabConn creates a new copy
            // to add me (if not existent)
            _pView->NotifyTabConnection(aNewConn, sal_False);
                // don't create an Undo-Action for the new connection : the connection is
                // covered by the Undo-Action for the tabwin, as the "Undo the insert" will
                // automatically remove all connections adjacent to the win.
                // (Because of this automatism we would have an ownerhsip ambiguity for
                // the connection data if we would insert the conn-Undo-Action)
        }
    }
}

// class OQueryTableView
DBG_NAME(OQueryTableView)
OQueryTableView::OQueryTableView( Window* pParent,OQueryDesignView* pView)
    : OJoinTableView( pParent,pView)
{
    DBG_CTOR(OQueryTableView,NULL);
    SetHelpId(HID_CTL_QRYDGNTAB);
}

OQueryTableView::~OQueryTableView()
{
    DBG_DTOR(OQueryTableView,NULL);
}

sal_Int32 OQueryTableView::CountTableAlias(const String& rName, sal_Int32& rMax)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    sal_Int32 nRet = 0;

    OTableWindowMapIterator aIter = GetTabWinMap()->find(rName);
    while(aIter != GetTabWinMap()->end())
    {
        OUString aNewName = OUString(rName) + "_" + OUString::number(++nRet);
        aIter = GetTabWinMap()->find(aNewName);
    }

    rMax = nRet;

    return nRet;
}

void OQueryTableView::ReSync()
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    TTableWindowData* pTabWinDataList = m_pView->getController().getTableWindowData();
    OSL_ENSURE((getTableConnections()->size()==0) && (GetTabWinMap()->size()==0),
        "before calling OQueryTableView::ReSync() please call ClearAll !");

    // I need a collection of all window names that cannot be created so that I do not initialize connections for them.
    ::std::vector<String> arrInvalidTables;

    TTableWindowData::reverse_iterator aIter = pTabWinDataList->rbegin();
    // Create the window and add it

    for(;aIter != pTabWinDataList->rend();++aIter)
    {
        OQueryTableWindowData* pData = static_cast<OQueryTableWindowData*>(aIter->get());
        OTableWindow* pTabWin = createWindow(*aIter);

        // I dont't use ShowTabWin as this adds the window data to the list of documents.
        // This would be bad as I am getting them from there.
        // Instead, I do it step by step
        if (!pTabWin->Init())
        {
            // The initialisation has gone wrong, this TabWin is not available, so
            // I must clean up the data and the document
            pTabWin->clearListBox();
            delete pTabWin;
            arrInvalidTables.push_back(pData->GetAliasName());

            pTabWinDataList->erase( ::std::remove(pTabWinDataList->begin(),pTabWinDataList->end(),*aIter) ,pTabWinDataList->end());
            continue;
        }

        (*GetTabWinMap())[pData->GetAliasName()] = pTabWin; // add at the beginning as I am going backwards through the DataList
        // Use the default if there is no position or size
        if (!pData->HasPosition() && !pData->HasSize())
            SetDefaultTabWinPosSize(pTabWin);

        pTabWin->Show();
    }

    // Add the connections
    TTableConnectionData* pTabConnDataList = m_pView->getController().getTableConnectionData();
    TTableConnectionData::reverse_iterator aConIter = pTabConnDataList->rbegin();

    for(;aConIter != pTabConnDataList->rend();++aConIter)
    {
        OQueryTableConnectionData* pTabConnData =  static_cast<OQueryTableConnectionData*>(aConIter->get());

        // do both tables for the connection exist ?
        String strTabExistenceTest = pTabConnData->getReferencingTable()->GetWinName();
        sal_Bool bInvalid = ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();
        strTabExistenceTest = pTabConnData->getReferencedTable()->GetWinName();
        bInvalid = bInvalid && ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();

        if (bInvalid)
        {
            // no -> bad luck, no connection
            pTabConnDataList->erase( ::std::remove(pTabConnDataList->begin(),pTabConnDataList->end(),*aConIter) ,pTabConnDataList->end());
            continue;
        }

        // adds a new connection to join view and notifies our accessible and invaldates the controller
        addConnection(new OQueryTableConnection(this, *aConIter));
    }
}

void OQueryTableView::ClearAll()
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OJoinTableView::ClearAll();

    SetUpdateMode(sal_True);
    m_pView->getController().setModified(sal_True);
}

OTableWindow* OQueryTableView::createWindow(const TTableWindowData::value_type& _pData)
{
    return new OQueryTableWindow(this,_pData);
}

void OQueryTableView::NotifyTabConnection(const OQueryTableConnection& rNewConn, sal_Bool _bCreateUndoAction)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // let's first check if I have the connection already
    OQueryTableConnection* pTabConn = NULL;
    const ::std::vector<OTableConnection*>* pConnections = getTableConnections();
    ::std::vector<OTableConnection*>::const_iterator aEnd = pConnections->end();
    ::std::vector<OTableConnection*>::const_iterator aIter = ::std::find(   pConnections->begin(),
                                                    aEnd,
                                                    static_cast<const OTableConnection*>(&rNewConn)
                                                    );
    if(aIter == aEnd )
    {
        aIter = pConnections->begin();
        for(;aIter != aEnd;++aIter)
        {
            if(*static_cast<OQueryTableConnection*>(*aIter) == rNewConn)
            {
                pTabConn = static_cast<OQueryTableConnection*>(*aIter);
                break;
            }
        }
    }
    else
        pTabConn = static_cast<OQueryTableConnection*>(*aIter);

    // no -> insert
    if (pTabConn == NULL)
    {
        // the new data ...
        OQueryTableConnectionData* pNewData = static_cast< OQueryTableConnectionData*>(rNewConn.GetData()->NewInstance());
        pNewData->CopyFrom(*rNewConn.GetData());
        TTableConnectionData::value_type aData(pNewData);
        OQueryTableConnection* pNewConn = new OQueryTableConnection(this, aData);
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

void OQueryTableView::AddTabWin(const OUString& _rTableName, const OUString& _rAliasName, sal_Bool bNewTable)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
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
                                    ::dbtools::eInDataManipulation);
        OUString sRealName(sSchema);
        if (!sRealName.isEmpty())
            sRealName+= OUString('.');
        sRealName += sTable;

        AddTabWin(_rTableName, sRealName, _rAliasName, bNewTable);
    }
    catch(SQLException&)
    {
        OSL_FAIL("qualifiedNameComponents");
    }
}

// find the table which has a foreign key with this referencedTable name
Reference<XPropertySet> getKeyReferencedTo(const Reference<XIndexAccess>& _rxKeys,const OUString& _rReferencedTable)
{
    if(!_rxKeys.is())
        return Reference<XPropertySet>();

    if ( !_rxKeys.is() )
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

void OQueryTableView::AddTabWin(const OUString& _rComposedName, const OUString& _rTableName, const OUString& strAlias, sal_Bool bNewTable)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OSL_ENSURE(!_rTableName.isEmpty() || !strAlias.isEmpty(), "OQueryTableView::AddTabWin : no tables or aliases !");
        // If the table is not set, then it is a dummy window, but at least the alias must be set

    // build a new data structure
    // first check if this already has its data
    sal_Bool bAppend = bNewTable;
    TTableWindowData::value_type pNewTabWinData;
    TTableWindowData* pWindowData = getDesignView()->getController().getTableWindowData();
    TTableWindowData::iterator aWinIter = pWindowData->begin();
    TTableWindowData::iterator aWinEnd = pWindowData->end();
    for(;aWinIter != aWinEnd;++aWinIter)
    {
        pNewTabWinData = *aWinIter;
        if (pNewTabWinData && pNewTabWinData->GetWinName() == strAlias && pNewTabWinData->GetComposedName() == _rComposedName && pNewTabWinData->GetTableName() == _rTableName)
            break;
    }
    if ( !bAppend )
        bAppend = ( aWinIter == aWinEnd );
    if ( bAppend )
        pNewTabWinData = createTableWindowData(_rComposedName, _rTableName, strAlias);
        // I do not need to add TabWinData to the DocShell list, ShowTabWin does that.

    // Create a new window
    OQueryTableWindow* pNewTabWin = static_cast<OQueryTableWindow*>(createWindow(pNewTabWinData));
    // No need to initialize, as that happens in ShowTabWin

    // New UndoAction
    OQueryTabWinShowUndoAct* pUndoAction = new OQueryTabWinShowUndoAct(this);
    pUndoAction->SetTabWin(pNewTabWin); // Window
    sal_Bool bSuccess = ShowTabWin(pNewTabWin, pUndoAction,bAppend);
    if(!bSuccess)
    {
        // reset table window
        pUndoAction->SetTabWin(NULL);
        pUndoAction->SetOwnership(sal_False);

        delete pUndoAction;
        return;
    }

    // Show the relations between the individual tables
    OTableWindowMap* pTabWins = GetTabWinMap();
    if(bNewTable && !pTabWins->empty() && !_rTableName.isEmpty())
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

                    OTableWindowMap::const_iterator aIter = pTabWins->find(aReferencedTable);
                    OTableWindowMap::const_iterator aEnd  = pTabWins->end();
                    if(aIter == aEnd)
                    {
                        for(aIter = pTabWins->begin();aIter != aEnd;++aIter)
                        {
                            OQueryTableWindow* pTabWinTmp = static_cast<OQueryTableWindow*>(aIter->second);
                            OSL_ENSURE( pTabWinTmp,"TableWindow is null!" );
                            if ( pTabWinTmp != pNewTabWin && pTabWinTmp->GetComposedName() == aReferencedTable )
                                break;
                        }
                    }
                    if ( aIter != aEnd && pNewTabWin != aIter->second )
                        addConnections( this, *pNewTabWin, *static_cast<OQueryTableWindow*>(aIter->second), xFKeyColumns );
                }
                break;

                case KeyType::PRIMARY:
                {
                    // we have a primary key so look in our list if there exsits a key which this is refered to
                    OTableWindowMap::const_iterator aIter = pTabWins->begin();
                    OTableWindowMap::const_iterator aEnd  = pTabWins->end();
                    for(;aIter != aEnd;++aIter)
                    {
                        OQueryTableWindow* pTabWinTmp = static_cast<OQueryTableWindow*>(aIter->second);
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
            DBG_UNHANDLED_EXCEPTION();
        }

        } while ( false );
    }

    // My parent needs to be informed about the delete
    m_pView->getController().addUndoActionAndInvalidate( pUndoAction );

    if (bSuccess && m_lnkTabWinsChangeHandler.IsSet())
    {
        TabWinsChangeNotification aHint(TabWinsChangeNotification::AT_ADDED_WIN, pNewTabWin->GetAliasName());
        m_lnkTabWinsChangeHandler.Call(&aHint);
    }
}

void OQueryTableView::AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OQueryTableWindow* pSourceWin = static_cast< OQueryTableWindow*>(jxdSource.pListBox->GetTabWin());
    OQueryTableWindow* pDestWin = static_cast< OQueryTableWindow*>(jxdDest.pListBox->GetTabWin());

    String aSourceFieldName, aDestFieldName;
    aSourceFieldName    = jxdSource.pListBox->GetEntryText(jxdSource.pEntry);
    aDestFieldName      = jxdDest.pListBox->GetEntryText(jxdDest.pEntry);

    OTableConnection* pConn = GetTabConn(pSourceWin,pDestWin,true);
    if ( !pConn )
    {
        // new data object
        OQueryTableConnectionData* pNewConnectionData = new OQueryTableConnectionData(pSourceWin->GetData(), pDestWin->GetData());
        TTableConnectionData::value_type aNewConnectionData(pNewConnectionData);

        sal_uInt32          nSourceFieldIndex, nDestFieldIndex;
        ETableFieldType eSourceFieldType, eDestFieldType;

        // Get name/position/type of both affected fields ...
        // Source

        nSourceFieldIndex = jxdSource.pListBox->GetModel()->GetAbsPos(jxdSource.pEntry);
        eSourceFieldType = static_cast< OTableFieldInfo*>(jxdSource.pEntry->GetUserData())->GetKeyType();

        // Dest

        nDestFieldIndex = jxdDest.pListBox->GetModel()->GetAbsPos(jxdDest.pEntry);
        eDestFieldType = static_cast< OTableFieldInfo*>(jxdDest.pEntry->GetUserData())->GetKeyType();

        // ... and set them
        pNewConnectionData->SetFieldIndex(JTCS_FROM, nSourceFieldIndex);
        pNewConnectionData->SetFieldIndex(JTCS_TO, nDestFieldIndex);

        pNewConnectionData->SetFieldType(JTCS_FROM, eSourceFieldType);
        pNewConnectionData->SetFieldType(JTCS_TO, eDestFieldType);

        pNewConnectionData->AppendConnLine( aSourceFieldName,aDestFieldName );

        OQueryTableConnection aNewConnection(this, aNewConnectionData);
        NotifyTabConnection(aNewConnection);
        // As usual with NotifyTabConnection, using a local variable is fine because a copy is made
    }
    else
    {
        // the connection could point on the other side
        if(pConn->GetSourceWin() == pDestWin)
        {
            String aTmp(aSourceFieldName);
            aSourceFieldName = aDestFieldName;
            aDestFieldName = aTmp;
        }

        pConn->GetData()->AppendConnLine( aSourceFieldName,aDestFieldName );

        connectionModified(this,pConn,sal_False);
    }
}

void OQueryTableView::ConnDoubleClicked(OTableConnection* pConnection)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    if( openJoinDialog(this,pConnection->GetData(),sal_False) )
    {
        connectionModified(this,pConnection,sal_False);
        SelectConn( pConnection );
    }
}

void OQueryTableView::createNewConnection()
{
    TTableConnectionData::value_type pData(new OQueryTableConnectionData());
    if( openJoinDialog(this,pData,sal_True) )
    {
        OTableWindowMap* pMap = GetTabWinMap();
        OQueryTableWindow* pSourceWin   = static_cast< OQueryTableWindow*>((*pMap)[pData->getReferencingTable()->GetWinName()]);
        OQueryTableWindow* pDestWin     = static_cast< OQueryTableWindow*>((*pMap)[pData->getReferencedTable()->GetWinName()]);
        // first we have to look if the this connection already exists
        OTableConnection* pConn = GetTabConn(pSourceWin,pDestWin,true);
        sal_Bool bNew = sal_True;
        if ( pConn )
        {
            pConn->GetData()->CopyFrom( *pData );
            bNew = sal_False;
        }
        else
        {
            // create a new conenction and append it
            OQueryTableConnection* pQConn = new OQueryTableConnection(this, pData);
            GetConnection(pQConn);
            pConn = pQConn;
        }
        connectionModified(this,pConn,bNew);
        if ( !bNew && pConn == GetSelectedConn() ) // our connection was selected before so we have to reselect it
            SelectConn( pConn );
    }
}

bool OQueryTableView::RemoveConnection( OTableConnection* _pConnection,sal_Bool /*_bDelete*/ )
{
    DBG_CHKTHIS(OQueryTableView,NULL);

    // we don't want that our connection will be deleted, we put it in the undo manager
    bool bRet = OJoinTableView::RemoveConnection( _pConnection,sal_False);

    // add undo action
    addUndoAction(  this,
                    new OQueryDelTabConnUndoAction(this),
                    static_cast< OQueryTableConnection*>(_pConnection),
                    sal_True);
    return bRet;
}

void OQueryTableView::KeyInput( const KeyEvent& rEvt )
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OJoinTableView::KeyInput( rEvt );
}

OQueryTableWindow* OQueryTableView::FindTable(const String& rAliasName)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OSL_ENSURE(rAliasName.Len(), "OQueryTableView::FindTable : the  AliasName should not be empty !");
        // (it is harmless but does not make sense and indicates that there is probably an error in the caller)
    OTableWindowMap::const_iterator aIter = GetTabWinMap()->find(rAliasName);
    if(aIter != GetTabWinMap()->end())
        return static_cast<OQueryTableWindow*>(aIter->second);
    return NULL;
}

sal_Bool OQueryTableView::FindTableFromField(const String& rFieldName, OTableFieldDescRef& rInfo, sal_uInt16& rCnt)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    rCnt = 0;
    OTableWindowMap::const_iterator aIter = GetTabWinMap()->begin();
    OTableWindowMap::const_iterator aEnd  = GetTabWinMap()->end();
    for(;aIter != aEnd;++aIter)
    {
        if(static_cast<OQueryTableWindow*>(aIter->second)->ExistsField(rFieldName, rInfo))
            ++rCnt;
    }

    return rCnt == 1;
}

bool OQueryTableView::ContainsTabWin(const OTableWindow& rTabWin)
{
    OTableWindowMap* pTabWins = GetTabWinMap();
    OSL_ENSURE(pTabWins != NULL, "OQueryTableView::RemoveTabWin : Window should not be NULL !");

    OTableWindowMap::iterator aIter = pTabWins->begin();
    OTableWindowMap::iterator aEnd  = pTabWins->end();

    for ( ;aIter != aEnd ; ++aIter )
    {
        if ( aIter->second == &rTabWin )
        {
            return true;
        }
    }

    return false;
}

void OQueryTableView::RemoveTabWin(OTableWindow* pTabWin)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OSL_ENSURE(pTabWin != NULL, "OQueryTableView::RemoveTabWin : Window should not be NULL !");

    if(pTabWin && ContainsTabWin(*pTabWin)) // #i122589# check if registered before deleting
    {
        // I need my parent so it can be informed about the deletion
        OQueryDesignView* pParent = static_cast<OQueryDesignView*>(getDesignView());

        SfxUndoManager& rUndoMgr = m_pView->getController().GetUndoManager();
        rUndoMgr.EnterListAction( String( ModuleRes(STR_QUERY_UNDO_TABWINDELETE) ), String() );

        // add the Undo-Action
        OQueryTabWinDelUndoAct* pUndoAction = new OQueryTabWinDelUndoAct(this);
        pUndoAction->SetTabWin(static_cast< OQueryTableWindow*>(pTabWin));

        // and hide the window
        HideTabWin(static_cast< OQueryTableWindow*>(pTabWin), pUndoAction);

        // Undo Actions and delete the fields in SelectionBrowseBox
        pParent->TableDeleted( static_cast< OQueryTableWindowData*>(pTabWin->GetData().get())->GetAliasName() );

        m_pView->getController().addUndoActionAndInvalidate( pUndoAction );
        rUndoMgr.LeaveListAction();

        if (m_lnkTabWinsChangeHandler.IsSet())
        {
            TabWinsChangeNotification aHint(TabWinsChangeNotification::AT_REMOVED_WIN, static_cast< OQueryTableWindow*>(pTabWin)->GetAliasName());
            m_lnkTabWinsChangeHandler.Call(&aHint);
        }

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
    DBG_CHKTHIS(OQueryTableView,NULL);

    Invalidate(INVALIDATE_NOCHILDREN);
    OJoinTableView::EnsureVisible(pWin);
}

void OQueryTableView::GetConnection(OQueryTableConnection* pConn)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // add to me and the document

    addConnection( pConn );
}

void OQueryTableView::DropConnection(OQueryTableConnection* pConn)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    // Pay attention to the selection
    // remove from me and the document
    RemoveConnection( pConn ,sal_False);
}

void OQueryTableView::HideTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction )
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OTableWindowMap* pTabWins = GetTabWinMap();
    OSL_ENSURE(pTabWins != NULL, "OQueryTableView::HideTabWin : have no TabWins !");

    if (pTabWin)
    {
        // Window
        // save the position in its data
        getDesignView()->SaveTabWinUIConfig(pTabWin);
        // (I need to go via the parent, as only the parent knows the position of the scrollbars)
        // and then out of the TabWins list and hide
        OTableWindowMap::iterator aIter = pTabWins->begin();
        OTableWindowMap::iterator aEnd  = pTabWins->end();
        for ( ;aIter != aEnd ; ++aIter )
            if ( aIter->second == pTabWin )
            {
                pTabWins->erase( aIter );
                break;
            }

        pTabWin->Hide();    // do not destroy it, as it is still in the undo list!!

        // the TabWin data must also be passed out of my responsibility
        TTableWindowData* pTabWinDataList = m_pView->getController().getTableWindowData();
        pTabWinDataList->erase( ::std::remove(pTabWinDataList->begin(),pTabWinDataList->end(),pTabWin->GetData()),pTabWinDataList->end());
            // The data should not be destroyed as TabWin itself - which is still alive - needs them
            // Either it goes back into my responsibility, (via ShowTabWin), then I add the data back,
            // or the Undo-Action, which currently has full responsibility for the window
            // and its data, gets destroyed and destroys both the window and its data

        if (m_pLastFocusTabWin == pTabWin)
            m_pLastFocusTabWin = NULL;

        // collect connections belonging to the window and pass to UndoAction
        sal_Int16 nCnt = 0;
        const ::std::vector<OTableConnection*>* pTabConList = getTableConnections();
        ::std::vector<OTableConnection*>::const_iterator aIter2 = pTabConList->begin();
        for(;aIter2 != pTabConList->end();)// the end may change
        {
            OQueryTableConnection* pTmpEntry = static_cast<OQueryTableConnection*>(*aIter2);
            OSL_ENSURE(pTmpEntry,"OQueryTableConnection is null!");
            if( pTmpEntry->GetAliasName(JTCS_FROM) == pTabWin->GetAliasName() ||
                pTmpEntry->GetAliasName(JTCS_TO) == pTabWin->GetAliasName() )
            {
                // add to undo list
                pUndoAction->InsertConnection(pTmpEntry);

                // call base class because we append an undo action
                // but this time we are in a undo action list
                OJoinTableView::RemoveConnection(pTmpEntry,sal_False);
                aIter2 = pTabConList->begin();
                ++nCnt;
            }
            else
                ++aIter2;
        }

        if (nCnt)
            InvalidateConnections();

        m_pView->getController().InvalidateFeature(ID_BROWSER_ADDTABLE);

        // inform the UndoAction that the window and connections belong to it
        pUndoAction->SetOwnership(sal_True);

        // by doing so, we have modified the document
        m_pView->getController().setModified( sal_True );
        m_pView->getController().InvalidateFeature(SID_BROWSER_CLEAR_QUERY);
    }
}

sal_Bool OQueryTableView::ShowTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction,sal_Bool _bAppend )
{
    DBG_CHKTHIS(OQueryTableView,NULL);

    sal_Bool bSuccess = sal_False;

    if (pTabWin)
    {
        if (pTabWin->Init())
        {
            TTableWindowData::value_type pData = pTabWin->GetData();
            OSL_ENSURE(pData != 0, "OQueryTableView::ShowTabWin : TabWin has no data !");
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
            OSL_ENSURE(GetTabWinMap()->find(sName) == GetTabWinMap()->end(),"Alias name already in list!");
            GetTabWinMap()->insert(OTableWindowMap::value_type(sName,pTabWin));

            pTabWin->Show();

            pTabWin->Update();
            // We must call Update() in order to show the connections in the window correctly. This sounds strange,
            // but the Listbox  has an internal Member which is initialized when the Listbox is first shown (after the Listbox
            // is filled in Init). This Member will eventually be needed for
            // GetEntryPos, and then in turn by the Connection, when its starting point to the window must be determined.

            // the Connections
            ::std::vector<OTableConnection*>* pTableCon = pUndoAction->GetTabConnList();
            ::std::vector<OTableConnection*>::iterator aIter = pTableCon->begin();
            ::std::vector<OTableConnection*>::iterator aEnd = pTableCon->end();

            for(;aIter != aEnd;++aIter)
                addConnection(*aIter); // add all connections from the undo action

            pTableCon->clear();

            // and add the window's data to the list (of the document)
            if(_bAppend)
                m_pView->getController().getTableWindowData()->push_back(pTabWin->GetData());

            m_pView->getController().InvalidateFeature(ID_BROWSER_ADDTABLE);

            // and inform the UndoAction  that the window belongs to me
            pUndoAction->SetOwnership(sal_False);

            bSuccess = sal_True;
        }
        else
        {
            // Initialisation failed
            // (for example when the Connection to the database is not available at the moment)
            pTabWin->clearListBox();
            delete pTabWin;
        }
    }

    // show that I have changed the document
    if(!m_pView->getController().isReadOnly())
        m_pView->getController().setModified( sal_True );

    m_pView->getController().InvalidateFeature(SID_BROWSER_CLEAR_QUERY);

    return bSuccess;
}

void OQueryTableView::InsertField(const OTableFieldDescRef& rInfo)
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    OSL_ENSURE(getDesignView() != NULL, "OQueryTableView::InsertField : has no Parent !");
    static_cast<OQueryDesignView*>(getDesignView())->InsertField(rInfo);
}

sal_Bool OQueryTableView::ExistsAVisitedConn(const OQueryTableWindow* pFrom) const
{
    DBG_CHKTHIS(OQueryTableView,NULL);
    const ::std::vector<OTableConnection*>* pList = getTableConnections();
    if (pList)
    {
        ::std::vector<OTableConnection*>::const_iterator aIter = pList->begin();
        ::std::vector<OTableConnection*>::const_iterator aEnd = pList->end();
        for(;aIter != aEnd;++aIter)
        {
            OQueryTableConnection* pTemp = static_cast<OQueryTableConnection*>(*aIter);
            if (pTemp->IsVisited() &&
                (pFrom == static_cast< OQueryTableWindow*>(pTemp->GetSourceWin()) || pFrom == static_cast< OQueryTableWindow*>(pTemp->GetDestWin())))
                return pTemp != NULL;
        }
    }

    return sal_False;
}

void OQueryTableView::onNoColumns_throw()
{
    String sError( ModuleRes( STR_STATEMENT_WITHOUT_RESULT_SET ) );
    ::dbtools::throwSQLException( sError, ::dbtools::SQL_GENERAL_ERROR, NULL );
}

bool OQueryTableView::supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const
{
    OQueryTableConnectionData* pQueryData = static_cast<OQueryTableConnectionData*>(_pData.get());
    return pQueryData && (pQueryData->GetJoinType() == CROSS_JOIN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
