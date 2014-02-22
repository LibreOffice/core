/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

        
        if ( _bAddUndo )
            addUndoAction(  _pView,
                            new OQueryAddTabConnUndoAction(_pView),
                            static_cast< OQueryTableConnection*>(_pConnection));
        
        _pConnection->RecalcLines();
        
        _pConnection->InvalidateConnection();

        _pView->Invalidate(INVALIDATE_NOCHILDREN);
    }
    void addConnections(OQueryTableView* _pView,
                        const OQueryTableWindow& _rSource,
                        const OQueryTableWindow& _rDest,
                        const Reference<XNameAccess>& _rxSourceForeignKeyColumns)
    {
        if ( _rSource.GetData()->isQuery() || _rDest.GetData()->isQuery() )
            
            return;

        
        
        
        OQueryTableConnectionData* pNewConnData = new OQueryTableConnectionData( _rSource.GetData(), _rDest.GetData() );
        TTableConnectionData::value_type aNewConnData(pNewConnData);

        Reference<XIndexAccess> xReferencedKeys( _rDest.GetData()->getKeys());
        OUString sRelatedColumn;

        
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
                Sequence< sal_Int16> aFind(::comphelper::findValue(_rSource.GetOriginalColumns()->getElementNames(),*pIter,true));
                if(aFind.getLength())
                    pNewConnData->SetFieldIndex(JTCS_FROM,aFind[0]+1);
                else
                    OSL_FAIL("Column not found!");
            }
            
            Reference<XNameAccess> xRefColumns = _rDest.GetOriginalColumns();
            if(xRefColumns.is())
            {
                Sequence< sal_Int16> aFind(::comphelper::findValue(xRefColumns->getElementNames(),sRelatedColumn,true));
                if(aFind.getLength())
                    pNewConnData->SetFieldIndex(JTCS_TO,aFind[0]+1);
                else
                    OSL_FAIL("Column not found!");
            }
            pNewConnData->AppendConnLine(*pIter,sRelatedColumn);

            
            OQueryTableConnection aNewConn(_pView, aNewConnData);
            
            
            _pView->NotifyTabConnection(aNewConn, sal_False);
                
                
                
                
                
        }
    }
}


OQueryTableView::OQueryTableView( Window* pParent,OQueryDesignView* pView)
    : OJoinTableView( pParent,pView)
{
    SetHelpId(HID_CTL_QRYDGNTAB);
}

OQueryTableView::~OQueryTableView()
{
}

sal_Int32 OQueryTableView::CountTableAlias(const OUString& rName, sal_Int32& rMax)
{
    sal_Int32 nRet = 0;

    OTableWindowMap::iterator aIter = GetTabWinMap()->find(rName);
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
    TTableWindowData* pTabWinDataList = m_pView->getController().getTableWindowData();
    OSL_ENSURE((getTableConnections()->size()==0) && (GetTabWinMap()->size()==0),
        "before calling OQueryTableView::ReSync() please call ClearAll !");

    
    ::std::vector<OUString> arrInvalidTables;

    TTableWindowData::reverse_iterator aIter = pTabWinDataList->rbegin();
    

    for(;aIter != pTabWinDataList->rend();++aIter)
    {
        OQueryTableWindowData* pData = static_cast<OQueryTableWindowData*>(aIter->get());
        OTableWindow* pTabWin = createWindow(*aIter);

        
        
        
        if (!pTabWin->Init())
        {
            
            
            pTabWin->clearListBox();
            delete pTabWin;
            arrInvalidTables.push_back(pData->GetAliasName());

            pTabWinDataList->erase( ::std::remove(pTabWinDataList->begin(),pTabWinDataList->end(),*aIter) ,pTabWinDataList->end());
            continue;
        }

        (*GetTabWinMap())[pData->GetAliasName()] = pTabWin; 
        
        if (!pData->HasPosition() && !pData->HasSize())
            SetDefaultTabWinPosSize(pTabWin);

        pTabWin->Show();
    }

    
    TTableConnectionData* pTabConnDataList = m_pView->getController().getTableConnectionData();
    TTableConnectionData::reverse_iterator aConIter = pTabConnDataList->rbegin();

    for(;aConIter != pTabConnDataList->rend();++aConIter)
    {
        OQueryTableConnectionData* pTabConnData =  static_cast<OQueryTableConnectionData*>(aConIter->get());

        
        OUString strTabExistenceTest = pTabConnData->getReferencingTable()->GetWinName();
        sal_Bool bInvalid = ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();
        strTabExistenceTest = pTabConnData->getReferencedTable()->GetWinName();
        bInvalid = bInvalid && ::std::find(arrInvalidTables.begin(),arrInvalidTables.end(),strTabExistenceTest) != arrInvalidTables.end();

        if (bInvalid)
        {
            
            pTabConnDataList->erase( ::std::remove(pTabConnDataList->begin(),pTabConnDataList->end(),*aConIter) ,pTabConnDataList->end());
            continue;
        }

        
        addConnection(new OQueryTableConnection(this, *aConIter));
    }
}

void OQueryTableView::ClearAll()
{
    OJoinTableView::ClearAll();

    SetUpdateMode(true);
    m_pView->getController().setModified(sal_True);
}

OTableWindow* OQueryTableView::createWindow(const TTableWindowData::value_type& _pData)
{
    return new OQueryTableWindow(this,_pData);
}

void OQueryTableView::NotifyTabConnection(const OQueryTableConnection& rNewConn, sal_Bool _bCreateUndoAction)
{
    
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

    
    if (pTabConn == NULL)
    {
        
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


Reference<XPropertySet> getKeyReferencedTo(const Reference<XIndexAccess>& _rxKeys,const OUString& _rReferencedTable)
{
    if(!_rxKeys.is())
        return Reference<XPropertySet>();

    if ( !_rxKeys.is() )
        return Reference<XPropertySet>();
    
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
                
                if(sReferencedTable == _rReferencedTable)
                    return xKey;
            }
        }
    }
    return Reference<XPropertySet>();
}

void OQueryTableView::AddTabWin(const OUString& _rComposedName, const OUString& _rTableName, const OUString& strAlias, sal_Bool bNewTable)
{
    OSL_ENSURE(!_rTableName.isEmpty() || !strAlias.isEmpty(), "OQueryTableView::AddTabWin : no tables or aliases !");
        

    
    
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
        

    
    OQueryTableWindow* pNewTabWin = static_cast<OQueryTableWindow*>(createWindow(pNewTabWinData));
    

    
    OQueryTabWinShowUndoAct* pUndoAction = new OQueryTabWinShowUndoAct(this);
    pUndoAction->SetTabWin(pNewTabWin); 
    sal_Bool bSuccess = ShowTabWin(pNewTabWin, pUndoAction,bAppend);
    if(!bSuccess)
    {
        
        pUndoAction->SetTabWin(NULL);
        pUndoAction->SetOwnership(sal_False);

        delete pUndoAction;
        return;
    }

    
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
                {   
                    
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

    
    m_pView->getController().addUndoActionAndInvalidate( pUndoAction );

    if (bSuccess && m_lnkTabWinsChangeHandler.IsSet())
    {
        TabWinsChangeNotification aHint(TabWinsChangeNotification::AT_ADDED_WIN, pNewTabWin->GetAliasName());
        m_lnkTabWinsChangeHandler.Call(&aHint);
    }
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
        
        OQueryTableConnectionData* pNewConnectionData = new OQueryTableConnectionData(pSourceWin->GetData(), pDestWin->GetData());
        TTableConnectionData::value_type aNewConnectionData(pNewConnectionData);

        sal_uInt32          nSourceFieldIndex, nDestFieldIndex;
        ETableFieldType eSourceFieldType, eDestFieldType;

        
        

        nSourceFieldIndex = jxdSource.pListBox->GetModel()->GetAbsPos(jxdSource.pEntry);
        eSourceFieldType = static_cast< OTableFieldInfo*>(jxdSource.pEntry->GetUserData())->GetKeyType();

        

        nDestFieldIndex = jxdDest.pListBox->GetModel()->GetAbsPos(jxdDest.pEntry);
        eDestFieldType = static_cast< OTableFieldInfo*>(jxdDest.pEntry->GetUserData())->GetKeyType();

        
        pNewConnectionData->SetFieldIndex(JTCS_FROM, nSourceFieldIndex);
        pNewConnectionData->SetFieldIndex(JTCS_TO, nDestFieldIndex);

        pNewConnectionData->SetFieldType(JTCS_FROM, eSourceFieldType);
        pNewConnectionData->SetFieldType(JTCS_TO, eDestFieldType);

        pNewConnectionData->AppendConnLine( aSourceFieldName,aDestFieldName );

        OQueryTableConnection aNewConnection(this, aNewConnectionData);
        NotifyTabConnection(aNewConnection);
        
    }
    else
    {
        
        if(pConn->GetSourceWin() == pDestWin)
        {
            OUString aTmp(aSourceFieldName);
            aSourceFieldName = aDestFieldName;
            aDestFieldName = aTmp;
        }

        pConn->GetData()->AppendConnLine( aSourceFieldName,aDestFieldName );

        connectionModified(this,pConn,sal_False);
    }
}

void OQueryTableView::ConnDoubleClicked(OTableConnection* pConnection)
{
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
        
        OTableConnection* pConn = GetTabConn(pSourceWin,pDestWin,true);
        sal_Bool bNew = sal_True;
        if ( pConn )
        {
            pConn->GetData()->CopyFrom( *pData );
            bNew = sal_False;
        }
        else
        {
            
            OQueryTableConnection* pQConn = new OQueryTableConnection(this, pData);
            GetConnection(pQConn);
            pConn = pQConn;
        }
        connectionModified(this,pConn,bNew);
        if ( !bNew && pConn == GetSelectedConn() ) 
            SelectConn( pConn );
    }
}

bool OQueryTableView::RemoveConnection( OTableConnection* _pConnection,sal_Bool /*_bDelete*/ )
{

    
    bool bRet = OJoinTableView::RemoveConnection( _pConnection,sal_False);

    
    addUndoAction(  this,
                    new OQueryDelTabConnUndoAction(this),
                    static_cast< OQueryTableConnection*>(_pConnection),
                    sal_True);
    return bRet;
}

void OQueryTableView::KeyInput( const KeyEvent& rEvt )
{
    OJoinTableView::KeyInput( rEvt );
}

OQueryTableWindow* OQueryTableView::FindTable(const OUString& rAliasName)
{
    OSL_ENSURE(!rAliasName.isEmpty(), "OQueryTableView::FindTable : the  AliasName should not be empty !");
        
    OTableWindowMap::const_iterator aIter = GetTabWinMap()->find(rAliasName);
    if(aIter != GetTabWinMap()->end())
        return static_cast<OQueryTableWindow*>(aIter->second);
    return NULL;
}

sal_Bool OQueryTableView::FindTableFromField(const OUString& rFieldName, OTableFieldDescRef& rInfo, sal_uInt16& rCnt)
{
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
    OSL_ENSURE(pTabWin != NULL, "OQueryTableView::RemoveTabWin : Window should not be NULL !");

    if(pTabWin && ContainsTabWin(*pTabWin)) 
    {
        
        OQueryDesignView* pParent = static_cast<OQueryDesignView*>(getDesignView());

        SfxUndoManager& rUndoMgr = m_pView->getController().GetUndoManager();
        rUndoMgr.EnterListAction( OUString( ModuleRes(STR_QUERY_UNDO_TABWINDELETE) ), OUString() );

        
        OQueryTabWinDelUndoAct* pUndoAction = new OQueryTabWinDelUndoAct(this);
        pUndoAction->SetTabWin(static_cast< OQueryTableWindow*>(pTabWin));

        
        HideTabWin(static_cast< OQueryTableWindow*>(pTabWin), pUndoAction);

        
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

    Invalidate(INVALIDATE_NOCHILDREN);
    OJoinTableView::EnsureVisible(pWin);
}

void OQueryTableView::GetConnection(OQueryTableConnection* pConn)
{
    

    addConnection( pConn );
}

void OQueryTableView::DropConnection(OQueryTableConnection* pConn)
{
    
    
    RemoveConnection( pConn ,sal_False);
}

void OQueryTableView::HideTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction )
{
    OTableWindowMap* pTabWins = GetTabWinMap();
    OSL_ENSURE(pTabWins != NULL, "OQueryTableView::HideTabWin : have no TabWins !");

    if (pTabWin)
    {
        
        
        getDesignView()->SaveTabWinUIConfig(pTabWin);
        
        
        OTableWindowMap::iterator aIter = pTabWins->begin();
        OTableWindowMap::iterator aEnd  = pTabWins->end();
        for ( ;aIter != aEnd ; ++aIter )
            if ( aIter->second == pTabWin )
            {
                pTabWins->erase( aIter );
                break;
            }

        pTabWin->Hide();    

        
        TTableWindowData* pTabWinDataList = m_pView->getController().getTableWindowData();
        pTabWinDataList->erase( ::std::remove(pTabWinDataList->begin(),pTabWinDataList->end(),pTabWin->GetData()),pTabWinDataList->end());
            
            
            
            

        if (m_pLastFocusTabWin == pTabWin)
            m_pLastFocusTabWin = NULL;

        
        sal_Int16 nCnt = 0;
        const ::std::vector<OTableConnection*>* pTabConList = getTableConnections();
        ::std::vector<OTableConnection*>::const_iterator aIter2 = pTabConList->begin();
        for(;aIter2 != pTabConList->end();)
        {
            OQueryTableConnection* pTmpEntry = static_cast<OQueryTableConnection*>(*aIter2);
            OSL_ENSURE(pTmpEntry,"OQueryTableConnection is null!");
            if( pTmpEntry->GetAliasName(JTCS_FROM) == pTabWin->GetAliasName() ||
                pTmpEntry->GetAliasName(JTCS_TO) == pTabWin->GetAliasName() )
            {
                
                pUndoAction->InsertConnection(pTmpEntry);

                
                
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

        
        pUndoAction->SetOwnership(sal_True);

        
        m_pView->getController().setModified( sal_True );
        m_pView->getController().InvalidateFeature(SID_BROWSER_CLEAR_QUERY);
    }
}

sal_Bool OQueryTableView::ShowTabWin( OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction,sal_Bool _bAppend )
{

    sal_Bool bSuccess = sal_False;

    if (pTabWin)
    {
        if (pTabWin->Init())
        {
            TTableWindowData::value_type pData = pTabWin->GetData();
            OSL_ENSURE(pData != 0, "OQueryTableView::ShowTabWin : TabWin has no data !");
            
            if (pData->HasPosition() && pData->HasSize())
            {
                Size aSize(CalcZoom(pData->GetSize().Width()),CalcZoom(pData->GetSize().Height()));
                pTabWin->SetPosSizePixel(pData->GetPosition(), aSize);
            }
            else
                
                SetDefaultTabWinPosSize(pTabWin);

            
            OUString sName = static_cast< OQueryTableWindowData*>(pData.get())->GetAliasName();
            OSL_ENSURE(GetTabWinMap()->find(sName) == GetTabWinMap()->end(),"Alias name already in list!");
            GetTabWinMap()->insert(OTableWindowMap::value_type(sName,pTabWin));

            pTabWin->Show();

            pTabWin->Update();
            
            
            
            

            
            ::std::vector<OTableConnection*>* pTableCon = pUndoAction->GetTabConnList();
            ::std::vector<OTableConnection*>::iterator aIter = pTableCon->begin();
            ::std::vector<OTableConnection*>::iterator aEnd = pTableCon->end();

            for(;aIter != aEnd;++aIter)
                addConnection(*aIter); 

            pTableCon->clear();

            
            if(_bAppend)
                m_pView->getController().getTableWindowData()->push_back(pTabWin->GetData());

            m_pView->getController().InvalidateFeature(ID_BROWSER_ADDTABLE);

            
            pUndoAction->SetOwnership(sal_False);

            bSuccess = sal_True;
        }
        else
        {
            
            
            pTabWin->clearListBox();
            delete pTabWin;
        }
    }

    
    if(!m_pView->getController().isReadOnly())
        m_pView->getController().setModified( sal_True );

    m_pView->getController().InvalidateFeature(SID_BROWSER_CLEAR_QUERY);

    return bSuccess;
}

void OQueryTableView::InsertField(const OTableFieldDescRef& rInfo)
{
    OSL_ENSURE(getDesignView() != NULL, "OQueryTableView::InsertField : has no Parent !");
    static_cast<OQueryDesignView*>(getDesignView())->InsertField(rInfo);
}

sal_Bool OQueryTableView::ExistsAVisitedConn(const OQueryTableWindow* pFrom) const
{
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
    OUString sError( ModuleRes( STR_STATEMENT_WITHOUT_RESULT_SET ) );
    ::dbtools::throwSQLException( sError, ::dbtools::SQL_GENERAL_ERROR, NULL );
}

bool OQueryTableView::supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const
{
    OQueryTableConnectionData* pQueryData = static_cast<OQueryTableConnectionData*>(_pData.get());
    return pQueryData && (pQueryData->GetJoinType() == CROSS_JOIN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
