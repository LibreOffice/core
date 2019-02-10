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

#include <QueryDesignView.hxx>
#include <QueryTableView.hxx>
#include "QTableWindow.hxx"
#include <vcl/toolbox.hxx>
#include <querycontroller.hxx>
#include <sqlbison.hxx>
#include <vcl/split.hxx>
#include <svl/undo.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <adtabdlg.hxx>
#include <vcl/svapp.hxx>
#include <vcl/combobox.hxx>
#include <vcl/weld.hxx>
#include <browserids.hxx>
#include "SelectionBrowseBox.hxx"
#include <strings.hrc>
#include <strings.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/string.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <connectivity/PColumn.hxx>
#include "QTableConnection.hxx"
#include <ConnectionLine.hxx>
#include <ConnectionLineData.hxx>
#include "QTableConnectionData.hxx"
#include <core_resource.hxx>
#include <stringconstants.hxx>
#include <UITools.hxx>
#include <querycontainerwindow.hxx>
#include <sqlmessage.hxx>
#include <unotools/syslocale.hxx>
#include <memory>
#include <set>

using namespace ::dbaui;
using namespace ::utl;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

// here we define our functions used in the anonymous namespace to get our header file smaller
// please look at the book LargeScale C++ to know why
namespace
{
    static const char C_AND[] = " AND ";
    static const char C_OR[] = " OR ";

    bool InsertJoin(    const OQueryDesignView* _pView,
                            const ::connectivity::OSQLParseNode *pNode);

    SqlParseError InstallFields(OQueryDesignView* _pView,
                                const ::connectivity::OSQLParseNode* pNode,
                                OJoinTableView::OTableWindowMap* pTabList );

    SqlParseError GetGroupCriteria( OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const ::connectivity::OSQLParseNode* pSelectRoot );

    SqlParseError GetHavingCriteria(OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const ::connectivity::OSQLParseNode* pSelectRoot,
                                    sal_uInt16& rLevel );

    SqlParseError GetOrderCriteria( OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const ::connectivity::OSQLParseNode* pParseRoot );

    SqlParseError AddFunctionCondition(OQueryDesignView const * _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const ::connectivity::OSQLParseNode * pCondition,
                                    const sal_uInt16 nLevel,
                                    bool bHaving,
                                    bool _bAddOrOnOneLine);

    OUString quoteTableAlias(bool _bQuote, const OUString& _sAliasName, const OUString& _sQuote)
    {
        OUString sRet;
        if ( _bQuote && !_sAliasName.isEmpty() )
        {
            sRet = ::dbtools::quoteName(_sQuote,_sAliasName) + ".";
        }
        return sRet;
    }
    OUString getTableRange(const OQueryDesignView* _pView,const ::connectivity::OSQLParseNode* _pTableRef)
    {
        Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();
        OUString sTableRange;
        if ( _pTableRef )
        {
            sTableRange = ::connectivity::OSQLParseNode::getTableRange(_pTableRef);
            if ( sTableRange.isEmpty() )
                _pTableRef->parseNodeToStr(sTableRange,xConnection,nullptr,false,false);
        }
        return sTableRange;
    }
    void insertConnection(const OQueryDesignView* _pView,const EJoinType& _eJoinType, const OTableFieldDescRef& _aDragLeft, const OTableFieldDescRef& _aDragRight, bool _bNatural = false)
    {
        OQueryTableView* pTableView = static_cast<OQueryTableView*>(_pView->getTableView());
        OQueryTableConnection* pConn = static_cast<OQueryTableConnection*>( pTableView->GetTabConn(static_cast<OTableWindow*>(_aDragLeft->GetTabWindow()),static_cast<OTableWindow*>(_aDragRight->GetTabWindow()),true));

        if ( !pConn )
        {
            OQueryTableConnectionData* pInfoData = new OQueryTableConnectionData();
            TTableConnectionData::value_type aInfoData(pInfoData);
            pInfoData->InitFromDrag(_aDragLeft, _aDragRight);
            pInfoData->SetJoinType(_eJoinType);

            if ( _bNatural )
            {
                aInfoData->ResetConnLines();
                pInfoData->setNatural(_bNatural);
                try
                {
                    Reference<XNameAccess> xReferencedTableColumns(aInfoData->getReferencedTable()->getColumns());
                    Sequence< OUString> aSeq = aInfoData->getReferencingTable()->getColumns()->getElementNames();
                    const OUString* pIter = aSeq.getConstArray();
                    const OUString* pEnd   = pIter + aSeq.getLength();
                    for(;pIter != pEnd;++pIter)
                    {
                        if ( xReferencedTableColumns->hasByName(*pIter) )
                            aInfoData->AppendConnLine(*pIter,*pIter);
                    }
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
                }
            }

            ScopedVclPtrInstance< OQueryTableConnection > aInfo(pTableView, aInfoData);
            // Because OQueryTableConnection never takes ownership of the data passed to it, but only remembers the pointer,
            // this pointer to a local variable is not critical, as aInfoData and aInfo have the same lifetime
            pTableView->NotifyTabConnection( *aInfo );
        }
        else
        {
            OUString aSourceFieldName(_aDragLeft->GetField());
            OUString aDestFieldName(_aDragRight->GetField());
            // the connection could point on the other side
            if(pConn->GetSourceWin() == _aDragRight->GetTabWindow())
            {
                OUString aTmp(aSourceFieldName);
                aSourceFieldName = aDestFieldName;
                aDestFieldName = aTmp;
            }
            pConn->GetData()->AppendConnLine( aSourceFieldName,aDestFieldName);
            pConn->UpdateLineList();
            // Modified-Flag
            //  SetModified();
            // and redraw
            pConn->RecalcLines();
                // for the following Invalidate, the new Connection must first be able
                // to determine its BoundingRect
            pConn->InvalidateConnection();
        }
    }
    OUString ParseCondition( OQueryController& rController
                                    ,const ::connectivity::OSQLParseNode* pCondition
                                    ,const OUString& _sDecimal
                                    ,const css::lang::Locale& _rLocale
                                    ,sal_uInt32 _nStartIndex)
    {
        OUString aCondition;
        Reference< XConnection> xConnection = rController.getConnection();
        if ( xConnection.is() )
        {
            sal_uInt32 nCount = pCondition->count();
            for(sal_uInt32 i = _nStartIndex ; i < nCount ; ++i)
                pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                xConnection,
                                rController.getNumberFormatter(),
                                _rLocale,
                                static_cast<sal_Char>(_sDecimal.toChar()),
                                &rController.getParser().getContext());
        }
        return aCondition;
    }
    SqlParseError FillOuterJoins(OQueryDesignView const * _pView,
                                const ::connectivity::OSQLParseNode* pTableRefList)
    {
        SqlParseError eErrorCode = eOk;
        sal_uInt32 nCount = pTableRefList->count();
        bool bError = false;
        for (sal_uInt32 i=0; !bError && i < nCount; ++i)
        {
            const ::connectivity::OSQLParseNode* pParseNode = pTableRefList->getChild(i);
            const ::connectivity::OSQLParseNode* pJoinNode = nullptr;

            if ( SQL_ISRULE( pParseNode, qualified_join ) || SQL_ISRULE( pParseNode, joined_table ) || SQL_ISRULE( pParseNode, cross_union ) )
                pJoinNode = pParseNode;
            else if(    SQL_ISRULE(pParseNode,table_ref)
                    &&  pParseNode->count() == 4 ) // '{' SQL_TOKEN_OJ joined_table '}'
                pJoinNode = pParseNode->getChild(2);

            if ( pJoinNode )
            {
                if ( !InsertJoin(_pView,pJoinNode) )
                    bError = true;
            }
        }
        // check if error occurred
        if ( bError )
            eErrorCode = eIllegalJoin;

        return eErrorCode;
    }

    /** FillDragInfo fills the field description out of the table
    */
    SqlParseError FillDragInfo( const OQueryDesignView* _pView,
                            const ::connectivity::OSQLParseNode* pColumnRef,
                            OTableFieldDescRef const & _rDragInfo)
    {
        SqlParseError eErrorCode = eOk;

        bool bErg = false;

        OUString aTableRange,aColumnName;
        sal_uInt16 nCntAccount;
        ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController&>(_pView->getController()).getParseIterator();
        rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

        if ( !aTableRange.isEmpty() )
        {
            OQueryTableWindow*  pSTW = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( aTableRange );
            bErg = (pSTW && pSTW->ExistsField( aColumnName, _rDragInfo ) );
        }
        if ( !bErg )
        {
            bErg = static_cast<OQueryTableView*>(_pView->getTableView())->FindTableFromField(aColumnName, _rDragInfo, nCntAccount);
            if ( !bErg )
                bErg = _pView->HasFieldByAliasName(aColumnName, _rDragInfo);
        }
        if ( !bErg )
        {
            eErrorCode = eColumnNotFound;
            OUString sError(DBA_RES(STR_QRY_COLUMN_NOT_FOUND));
            sError = sError.replaceFirst("$name$",aColumnName);
            _pView->getController().appendError( sError );

            try
            {
                Reference<XDatabaseMetaData> xMeta = _pView->getController().getConnection()->getMetaData();
                if ( xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers() )
                    _pView->getController().appendError(DBA_RES(STR_QRY_CHECK_CASESENSITIVE));
            }
            catch(Exception&)
            {
            }
        }

        return eErrorCode;
    }
    OUString BuildJoinCriteria(  const Reference< XConnection>& _xConnection,
                                        const OConnectionLineDataVec* pLineDataList,
                                        const OQueryTableConnectionData* pData)
    {
        OUStringBuffer aCondition;
        if ( _xConnection.is() )
        {
            try
            {
                const Reference< XDatabaseMetaData >  xMetaData = _xConnection->getMetaData();
                const OUString aQuote = xMetaData->getIdentifierQuoteString();

                for (auto const& lineData : *pLineDataList)
                {
                    if(!aCondition.isEmpty())
                        aCondition.append(C_AND);
                    aCondition.append(quoteTableAlias(true,pData->GetAliasName(JTCS_FROM),aQuote));
                    aCondition.append(::dbtools::quoteName(aQuote, lineData->GetFieldName(JTCS_FROM) ));
                    aCondition.append(" = ");
                    aCondition.append(quoteTableAlias(true,pData->GetAliasName(JTCS_TO),aQuote));
                    aCondition.append(::dbtools::quoteName(aQuote, lineData->GetFieldName(JTCS_TO) ));
                }
            }
            catch(SQLException&)
            {
                OSL_FAIL("Failure while building Join criteria!");
            }
        }

        return aCondition.makeStringAndClear();
    }
    /** JoinCycle looks for a join cycle and append it to the string
        @param  _xConnection    the connection
        @param  _pEntryConn     the table connection which holds the data
        @param  _pEntryTabTo    the corresponding table window
        @param  _rJoin          the String which will contain the resulting string
    */
    void JoinCycle( const Reference< XConnection>& _xConnection,
                    OQueryTableConnection* _pEntryConn,
                    const OQueryTableWindow* _pEntryTabTo,
                    OUString& _rJoin )
    {
        OSL_ENSURE(_pEntryConn,"TableConnection can not be null!");

        OQueryTableConnectionData* pData = static_cast< OQueryTableConnectionData*>(_pEntryConn->GetData().get());
        if ( pData->GetJoinType() != INNER_JOIN && _pEntryTabTo->ExistsAVisitedConn() )
        {
            bool bBrace = false;
            if(_rJoin.endsWith(")"))
            {
                bBrace = true;
                _rJoin = _rJoin.replaceAt(_rJoin.getLength()-1,1,OUString(' '));
            }
            _rJoin += C_AND + BuildJoinCriteria(_xConnection,&pData->GetConnLineDataList(),pData);
            if(bBrace)
                _rJoin += ")";
            _pEntryConn->SetVisited(true);
        }
    }
    OUString BuildTable( const Reference< XConnection>& _xConnection,
                                const OQueryTableWindow* pEntryTab,
                                bool _bForce = false
                                )
    {
        OUString aDBName(pEntryTab->GetComposedName());

        if( _xConnection.is() )
        {
            try
            {
                Reference< XDatabaseMetaData >  xMetaData = _xConnection->getMetaData();

                OUString sCatalog, sSchema, sTable;
                ::dbtools::qualifiedNameComponents( xMetaData, aDBName, sCatalog, sSchema, sTable, ::dbtools::EComposeRule::InDataManipulation );
                OUString aTableListStr = ::dbtools::composeTableNameForSelect( _xConnection, sCatalog, sSchema, sTable );

                OUString aQuote = xMetaData->getIdentifierQuoteString();
                if ( _bForce || isAppendTableAliasEnabled( _xConnection ) || pEntryTab->GetAliasName() != aDBName )
                {
                    aTableListStr += " ";
                    if ( generateAsBeforeTableAlias( _xConnection ) )
                        aTableListStr += "AS ";
                    aTableListStr += ::dbtools::quoteName( aQuote, pEntryTab->GetAliasName() );
                }
                aDBName = aTableListStr;
            }
            catch(const SQLException&)
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
        return aDBName;
    }
    OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                const OUString& rLh,
                                const OUString& rRh,
                                const OQueryTableConnectionData* pData)
    {

        OUString aErg(rLh);
        if ( pData->isNatural() && pData->GetJoinType() != CROSS_JOIN )
            aErg += " NATURAL ";
        switch(pData->GetJoinType())
        {
            case LEFT_JOIN:
                aErg += " LEFT OUTER ";
                break;
            case RIGHT_JOIN:
                aErg += " RIGHT OUTER ";
                break;
            case CROSS_JOIN:
                OSL_ENSURE(!pData->isNatural(),"OQueryDesignView::BuildJoin: This should not happen!");
                aErg += " CROSS ";
                break;
            case INNER_JOIN:
                OSL_ENSURE(pData->isNatural(),"OQueryDesignView::BuildJoin: This should not happen!");
                aErg += " INNER ";
                break;
            default:
                aErg += " FULL OUTER ";
                break;
        }
        aErg += "JOIN " + rRh;
        if ( CROSS_JOIN != pData->GetJoinType() && !pData->isNatural() )
        {
            aErg += " ON " + BuildJoinCriteria(_xConnection,&pData->GetConnLineDataList(),pData);
        }

        return aErg;
    }
    OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                const OQueryTableWindow* pLh,
                                const OQueryTableWindow* pRh,
                                const OQueryTableConnectionData* pData
                                )
    {
        bool bForce = pData->GetJoinType() == CROSS_JOIN || pData->isNatural();
        return BuildJoin(_xConnection,BuildTable(_xConnection,pLh,bForce),BuildTable(_xConnection,pRh,bForce),pData);
    }
    OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                const OUString &rLh,
                                const OQueryTableWindow* pRh,
                                const OQueryTableConnectionData* pData
                                )
    {
        return BuildJoin(_xConnection,rLh,BuildTable(_xConnection,pRh),pData);
    }
    OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                const OQueryTableWindow* pLh,
                                const OUString &rRh,
                                const OQueryTableConnectionData* pData
                                )
    {
        // strict ANSI SQL:
        // - does not support any bracketing of JOINS
        // - supports nested joins only in the LEFT HAND SIDE
        // In this case, we are trying to build a join with a nested join
        // in the right hand side.
        // So switch the direction of the join and both hand sides.
        OQueryTableConnectionData data(*pData);
        switch (data.GetJoinType())
        {
        case LEFT_JOIN:
            data.SetJoinType(RIGHT_JOIN);
            break;
        case RIGHT_JOIN:
            data.SetJoinType(LEFT_JOIN);
            break;
        default:
            // the other join types are symmetric, so nothing to change
            break;
        }
        return BuildJoin(_xConnection, rRh, BuildTable(_xConnection,pLh), &data);
    }
    void addConnectionTableNames( const Reference< XConnection>& _xConnection,
                                  const OQueryTableConnection* const pEntryConn,
                                  std::set<OUString> &_rTableNames )
    {
            // insert tables into table list to avoid double entries
            const OQueryTableWindow* const pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            const OQueryTableWindow* const pEntryTabTo = static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin());
            _rTableNames.insert(BuildTable(_xConnection,pEntryTabFrom));
            _rTableNames.insert(BuildTable(_xConnection,pEntryTabTo));
    }
    void GetNextJoin(   const Reference< XConnection>& _xConnection,
                        OQueryTableConnection* pEntryConn,
                        OQueryTableWindow const * pEntryTabTo,
                        OUString &aJoin,
                        std::set<OUString> &_rTableNames)
    {
        OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData().get());
        if ( pEntryConnData->GetJoinType() == INNER_JOIN && !pEntryConnData->isNatural() )
            return;

        if(aJoin.isEmpty())
        {
            addConnectionTableNames(_xConnection, pEntryConn, _rTableNames);
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            aJoin = BuildJoin(_xConnection,pEntryTabFrom,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetDestWin())
        {
            addConnectionTableNames(_xConnection, pEntryConn, _rTableNames);
            aJoin = BuildJoin(_xConnection,aJoin,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetSourceWin())
        {
            addConnectionTableNames(_xConnection, pEntryConn, _rTableNames);
            aJoin = BuildJoin(_xConnection,pEntryTabTo,aJoin,pEntryConnData);
        }

        pEntryConn->SetVisited(true);

        // first search for the "to" window
        const auto& rConnections = pEntryConn->GetParent()->getTableConnections();
        bool bFound = false;
        for (auto const& connection : rConnections)
        {
            OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(connection.get());
            if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabTo || pNext->GetDestWin() == pEntryTabTo))
            {
                OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabTo ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
                // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
                JoinCycle(_xConnection,pNext,pEntryTab,aJoin);
                if(!pNext->IsVisited())
                    GetNextJoin(_xConnection, pNext, pEntryTab, aJoin, _rTableNames);
                bFound = true;
            }
        }

        // when nothing found look for the "from" window
        if(!bFound)
        {
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            for (auto const& connection : rConnections)
            {
                OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(connection.get());
                if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabFrom || pNext->GetDestWin() == pEntryTabFrom))
                {
                    OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabFrom ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
                    // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
                    JoinCycle(_xConnection,pNext,pEntryTab,aJoin);
                    if(!pNext->IsVisited())
                        GetNextJoin(_xConnection, pNext, pEntryTab, aJoin, _rTableNames);
                }
            }
        }
    }
    SqlParseError InsertJoinConnection( const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode *pNode,
                                    const EJoinType& _eJoinType,
                                    const ::connectivity::OSQLParseNode *pLeftTable,
                                    const ::connectivity::OSQLParseNode *pRightTable)
    {
        SqlParseError eErrorCode = eOk;
        if (pNode->count() == 3 &&  // statement between brackets
            SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pNode->getChild(2),")"))
        {
            eErrorCode = InsertJoinConnection(_pView,pNode->getChild(1), _eJoinType,pLeftTable,pRightTable);
        }
        else if (SQL_ISRULEOR2(pNode,search_condition,boolean_term) &&          // AND/OR-joints:
                 pNode->count() == 3)
        {
            // only allow AND joints
            if (!SQL_ISTOKEN(pNode->getChild(1),AND))
                eErrorCode = eIllegalJoinCondition;
            else if ( eOk == (eErrorCode = InsertJoinConnection(_pView,pNode->getChild(0), _eJoinType,pLeftTable,pRightTable)) )
                    eErrorCode = InsertJoinConnection(_pView,pNode->getChild(2), _eJoinType,pLeftTable,pRightTable);
        }
        else if (SQL_ISRULE(pNode,comparison_predicate))
        {
            // only the comparison of columns is allowed
            OSL_ENSURE(pNode->count() == 3,"OQueryDesignView::InsertJoinConnection: Error in Parse Tree");
            if (!(SQL_ISRULE(pNode->getChild(0),column_ref) &&
                  SQL_ISRULE(pNode->getChild(2),column_ref) &&
                   pNode->getChild(1)->getNodeType() == SQLNodeType::Equal))
            {
                OUString sError(DBA_RES(STR_QRY_JOIN_COLUMN_COMPARE));
                _pView->getController().appendError( sError );
                return eIllegalJoin;
            }

            OTableFieldDescRef aDragLeft  = new OTableFieldDesc();
            OTableFieldDescRef aDragRight = new OTableFieldDesc();
            if ( eOk != ( eErrorCode = FillDragInfo(_pView,pNode->getChild(0),aDragLeft)) ||
                eOk != ( eErrorCode = FillDragInfo(_pView,pNode->getChild(2),aDragRight)))
                return eErrorCode;

            if ( pLeftTable )
            {
                OQueryTableWindow*  pLeftWindow = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( getTableRange(_pView,pLeftTable->getByRule(OSQLParseNode::table_ref) ));
                if ( pLeftWindow == aDragLeft->GetTabWindow() )
                    insertConnection(_pView,_eJoinType,aDragLeft,aDragRight);
                else
                    insertConnection(_pView,_eJoinType,aDragRight,aDragLeft);
            }
            else
                insertConnection(_pView,_eJoinType,aDragLeft,aDragRight);
        }
        else
            eErrorCode = eIllegalJoin;
        return eErrorCode;
    }
    bool GetInnerJoinCriteria(  const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode *pCondition)
    {
        return InsertJoinConnection(_pView,pCondition, INNER_JOIN,nullptr,nullptr) != eOk;
    }
    OUString GenerateSelectList( const OQueryDesignView* _pView,
                                        OTableFields&   _rFieldList,
                                        bool bAlias)
    {
        Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();
        if ( !xConnection.is() )
            return OUString();

        OUStringBuffer aTmpStr,aFieldListStr;

        bool bAsterisk = false;
        int nVis = 0;
        for (auto const& field : _rFieldList)
        {
            if ( field->IsVisible() )
            {
                if ( field->GetField().toChar() == '*' )
                    bAsterisk = true;
                ++nVis;
            }
        }
        if(nVis == 1)
            bAsterisk = false;

        try
        {
            const Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            const OUString aQuote = xMetaData->getIdentifierQuoteString();

            OJoinTableView::OTableWindowMap& rTabList = _pView->getTableView()->GetTabWinMap();

            for (auto const& field : _rFieldList)
            {
                OUString rFieldName = field->GetField();
                if ( !rFieldName.isEmpty() && field->IsVisible() )
                {
                    aTmpStr = "";
                    const OUString rAlias = field->GetAlias();
                    const OUString rFieldAlias = field->GetFieldAlias();

                    aTmpStr.append(quoteTableAlias((bAlias || bAsterisk),rAlias,aQuote));

                    // if we have a none numeric field, the table alias could be in the name
                    // otherwise we are not allowed to do this (e.g. 0.1 * PRICE )
                    if  ( !field->isOtherFunction() )
                    {
                        // we have to look if we have alias.* here but before we have to check if the column doesn't already exist
                        OTableFieldDescRef  aInfo = new OTableFieldDesc();
                        for (auto const& table : rTabList)
                        {
                            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(table.second.get());

                            if ( pTabWin->ExistsField( rFieldName, aInfo ) )
                            {
                                rFieldName = aInfo->GetField();
                                break;
                            }
                        }
                        if ( ( rFieldName.toChar() != '*' ) && ( rFieldName.indexOf( aQuote ) == -1 ) )
                        {
                            OSL_ENSURE(!field->GetTable().isEmpty(),"No table field name!");
                            aTmpStr.append(::dbtools::quoteName(aQuote, rFieldName));
                        }
                        else
                            aTmpStr.append(rFieldName);
                    }
                    else
                        aTmpStr.append(rFieldName);

                    if  ( field->isAggregateFunction() )
                    {
                        OSL_ENSURE(!field->GetFunction().isEmpty(),"Function name must not be empty! ;-(");
                        OUStringBuffer aTmpStr2( field->GetFunction());
                        aTmpStr2.append("(");
                        aTmpStr2.append(aTmpStr.makeStringAndClear());
                        aTmpStr2.append(")");
                        aTmpStr = aTmpStr2;
                    }

                    if (!rFieldAlias.isEmpty()                         &&
                        (rFieldName.toChar() != '*'                     ||
                        field->isNumericOrAggregateFunction()      ||
                        field->isOtherFunction()))
                    {
                        aTmpStr.append(" AS ");
                        aTmpStr.append(::dbtools::quoteName(aQuote, rFieldAlias));
                    }
                    aFieldListStr.append(aTmpStr.makeStringAndClear());
                    aFieldListStr.append(", ");
                }
            }
            if(!aFieldListStr.isEmpty())
                aFieldListStr.setLength(aFieldListStr.getLength()-2);
        }
        catch(SQLException&)
        {
            OSL_FAIL("Failure while building select list!");
        }
        return aFieldListStr.makeStringAndClear();
    }
    bool GenerateCriterias( OQueryDesignView const * _pView,
                                OUStringBuffer& rRetStr,
                                OUStringBuffer& rHavingStr,
                                OTableFields& _rFieldList,
                                bool bMulti )
    {
        Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();
        if(!xConnection.is())
            return false;

        OUString aFieldName,aCriteria,aWhereStr,aHavingStr,aWork/*,aOrderStr*/;
        // print line by line joined with AND
        sal_uInt16 nMaxCriteria = 0;
        for (auto const& field : _rFieldList)
        {
            nMaxCriteria = std::max<sal_uInt16>(nMaxCriteria,static_cast<sal_uInt16>(field->GetCriteria().size()));
        }
        try
        {
            const Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            const OUString aQuote = xMetaData->getIdentifierQuoteString();
            const IParseContext& rContext = static_cast<OQueryController&>(_pView->getController()).getParser().getContext();
            // * must not contain a filter : have I already shown the correct warning ?
            bool bCritsOnAsterikWarning = false;        // ** TMFS **

            for (sal_uInt16 i=0 ; i < nMaxCriteria ; i++)
            {
                aHavingStr.clear();
                aWhereStr.clear();

                for (auto const& field : _rFieldList)
                {
                    aFieldName = field->GetField();

                    if (aFieldName.isEmpty())
                        continue;
                    aCriteria = field->GetCriteria( i );
                    if ( !aCriteria.isEmpty() )
                    {
                        // * is not allowed to contain any filter, only when used in combination an aggregate function
                        if ( aFieldName.toChar() == '*' && field->isNoneFunction() )
                        {
                            // only show the messagebox the first time
                            if (!bCritsOnAsterikWarning)
                            {
                                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(_pView->GetFrameWeld(),
                                                                          VclMessageType::Warning, VclButtonsType::Ok,
                                                                          DBA_RES(STR_QRY_CRITERIA_ON_ASTERISK)));
                                xBox->run();
                            }
                            bCritsOnAsterikWarning = true;
                            continue;
                        }
                        aWork = quoteTableAlias(bMulti,field->GetAlias(),aQuote);

                        if ( (field->GetFunctionType() & (FKT_OTHER|FKT_NUMERIC)) || (aFieldName.toChar() == '*') )
                            aWork += aFieldName;
                        else
                            aWork += ::dbtools::quoteName(aQuote, aFieldName);

                        if ( field->isAggregateFunction() || field->IsGroupBy() )
                        {
                            if (aHavingStr.isEmpty())            // no more criteria
                                aHavingStr += "(";               // bracket
                            else
                                aHavingStr += C_AND;

                            if ( field->isAggregateFunction() )
                            {
                                OSL_ENSURE(!field->GetFunction().isEmpty(),"No function name for aggregate given!");
                                aHavingStr += field->GetFunction() + "(" + aWork + ")";       // bracket
                            }
                            else
                                aHavingStr += aWork;

                            OUString aErrorMsg;
                            Reference<XPropertySet> xColumn;
                            std::unique_ptr< ::connectivity::OSQLParseNode> pParseNode(_pView->getPredicateTreeFromEntry(field,aCriteria,aErrorMsg,xColumn));
                            if (pParseNode)
                            {
                                if (bMulti && !(field->isOtherFunction() || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(field->GetAlias(),aFieldName);
                                OUString sHavingStr = aHavingStr;

                                sal_uInt32 nCount = pParseNode->count();
                                for( sal_uInt32 node = 1 ; node < nCount ; ++node)
                                    pParseNode->getChild(node)->parseNodeToStr( sHavingStr,
                                                                xConnection,
                                                                &rContext,
                                                                false,
                                                                !field->isOtherFunction());
                                aHavingStr = sHavingStr;
                            }
                            else
                                aHavingStr += aCriteria;
                        }
                        else
                        {
                            if ( aWhereStr.isEmpty() )           // no more criteria
                                aWhereStr += "(";                // bracket
                            else
                                aWhereStr += C_AND;

                            aWhereStr += " ";
                            // aCriteria could have some German numbers so I have to be sure here
                            OUString aErrorMsg;
                            Reference<XPropertySet> xColumn;
                            std::unique_ptr< ::connectivity::OSQLParseNode> pParseNode( _pView->getPredicateTreeFromEntry(field,aCriteria,aErrorMsg,xColumn));
                            if (pParseNode)
                            {
                                if (bMulti && !(field->isOtherFunction() || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(field->GetAlias(),aFieldName);
                                OUString aWhere = aWhereStr;
                                pParseNode->parseNodeToStr( aWhere,
                                                            xConnection,
                                                            &rContext,
                                                            false,
                                                            !field->isOtherFunction() );
                                aWhereStr = aWhere;
                            }
                            else
                            {
                                aWhereStr += aWork + "=" + aCriteria;
                            }
                        }
                    }
                    // only once for each field
                    else if ( !i && field->isCondition() )
                    {
                        if (aWhereStr.isEmpty())         // no more criteria
                            aWhereStr += "(";            // bracket
                        else
                            aWhereStr += C_AND;
                        aWhereStr += field->GetField();
                    }
                }
                if (!aWhereStr.isEmpty())
                {
                    aWhereStr += ")";                          // close bracket for the AND branch
                    if (!rRetStr.isEmpty())                            // are there conditions on the field?
                        rRetStr.append(C_OR);
                    else                                        // open bracket for the OR branch
                        rRetStr.append('(');
                    rRetStr.append(aWhereStr);
                }
                if (!aHavingStr.isEmpty())
                {
                    aHavingStr +=  ")";                        // close bracket for the AND branch
                    if (!rHavingStr.isEmpty())                         // are there conditions on the field?
                        rHavingStr.append(C_OR);
                    else                                        // Open bracket for the OR branch
                        rHavingStr.append('(');
                    rHavingStr.append(aHavingStr);
                }
            }

            if (!rRetStr.isEmpty())
                rRetStr.append(')');                               // close bracket for the OR branch
            if (!rHavingStr.isEmpty())
                rHavingStr.append(')');                                // close bracket for the OR branch
        }
        catch(SQLException&)
        {
            OSL_FAIL("Failure while building where clause!");
        }
        return true;
    }
    SqlParseError GenerateOrder(    OQueryDesignView const * _pView,
                                    OTableFields& _rFieldList,
                                    bool bMulti,
                                    OUString& _rsRet)
    {
        const OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        const Reference< XConnection>& xConnection = rController.getConnection();
        if ( !xConnection.is() )
            return eNoConnection;

        SqlParseError eErrorCode = eOk;

        OUString aColumnName;
        OUString aWorkStr;
        try
        {
            const bool bColumnAliasInOrderBy = rController.getSdbMetaData().supportsColumnAliasInOrderBy();
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            OUString aQuote = xMetaData->getIdentifierQuoteString();
            // * must not contain filter - have I already shown the warning?
            bool bCritsOnAsterikWarning = false;        // ** TMFS **
            for (auto const& field : _rFieldList)
            {
                EOrderDir eOrder = field->GetOrderDir();
                // only create a sort expression when the table name and the sort criteria are defined
                // otherwise they will be built in GenerateCriteria
                if ( eOrder != ORDER_NONE )
                {
                    aColumnName = field->GetField();
                    if(aColumnName.toChar() == '*')
                    {
                        // only show the  MessageBox the first time
                        if (!bCritsOnAsterikWarning)
                        {
                            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(_pView->GetFrameWeld(),
                                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                                      DBA_RES(STR_QRY_ORDERBY_ON_ASTERISK)));
                            xBox->run();
                        }
                        bCritsOnAsterikWarning = true;
                        continue;
                    }

                    if ( bColumnAliasInOrderBy && !field->GetFieldAlias().isEmpty() )
                    {
                        aWorkStr += ::dbtools::quoteName(aQuote, field->GetFieldAlias());
                    }
                    else if ( field->isNumericOrAggregateFunction() )
                    {
                        OSL_ENSURE(!field->GetFunction().isEmpty(),"Function name cannot be empty! ;-(");
                        aWorkStr += field->GetFunction() + "("
                            + quoteTableAlias(
                                bMulti, field->GetAlias(), aQuote);
                        // only quote column name when we don't have a numeric
                        if ( field->isNumeric() )
                            aWorkStr += aColumnName;
                        else
                            aWorkStr += ::dbtools::quoteName(aQuote, aColumnName);

                        aWorkStr += ")";
                    }
                    else if ( field->isOtherFunction() )
                    {
                        aWorkStr += aColumnName;
                    }
                    else
                    {
                        aWorkStr += quoteTableAlias(bMulti,field->GetAlias(),aQuote) + ::dbtools::quoteName(aQuote, aColumnName);
                    }
                    aWorkStr += " " + OUString( ";ASC;DESC" ).getToken( static_cast<sal_uInt16>(eOrder), ';' ) + ",";
                }
            }

            {
                OUString sTemp(comphelper::string::stripEnd(aWorkStr, ','));
                aWorkStr = sTemp;
            }

            if ( !aWorkStr.isEmpty() )
            {
                const sal_Int32 nMaxOrder = xMetaData->getMaxColumnsInOrderBy();
                if ( nMaxOrder && nMaxOrder < comphelper::string::getTokenCount(aWorkStr, ',') )
                    eErrorCode = eStatementTooLong;
                else
                {
                    _rsRet = " ORDER BY " + aWorkStr;
                }
            }
        }
        catch(SQLException&)
        {
            OSL_FAIL("Failure while building group by!");
        }

        return eErrorCode;
    }

    void GenerateInnerJoinCriterias(const Reference< XConnection>& _xConnection,
                                    OUString& _rJoinCrit,
                                    const std::vector<VclPtr<OTableConnection> >& _rConnList)
    {
        for (auto const& connection : _rConnList)
        {
            const OQueryTableConnection* pEntryConn = static_cast<const OQueryTableConnection*>(connection.get());
            OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData().get());
            if ( pEntryConnData->GetJoinType() == INNER_JOIN && !pEntryConnData->isNatural() )
            {
                if(!_rJoinCrit.isEmpty())
                    _rJoinCrit += C_AND;
                _rJoinCrit += BuildJoinCriteria(_xConnection,&pEntryConnData->GetConnLineDataList(),pEntryConnData);
            }
        }
    }
    void searchAndAppendName(const Reference< XConnection>& _xConnection,
                             const OQueryTableWindow* _pTableWindow,
                             std::set<OUString>& _rTableNames,
                             OUString& _rsTableListStr
                             )
    {
        OUString sTabName(BuildTable(_xConnection,_pTableWindow));

        if(_rTableNames.insert(sTabName).second)
        {
            _rsTableListStr += sTabName + ",";
        }
    }
    OUString GenerateFromClause( const Reference< XConnection>& _xConnection,
                                        const OQueryTableView::OTableWindowMap* pTabList,
                                        const std::vector<VclPtr<OTableConnection> >& rConnList
                                        )
    {

        OUString aTableListStr;
        // used to avoid putting a table twice in FROM clause
        std::set<OUString> aTableNames;

        // generate outer join clause in from
        if(!rConnList.empty())
        {
            std::map<OTableWindow*,sal_Int32> aConnectionCount;
            auto aEnd = rConnList.end();
            for (auto const& connection : rConnList)
            {
                static_cast<OQueryTableConnection*>(connection.get())->SetVisited(false);
                ++aConnectionCount[connection->GetSourceWin()];
                ++aConnectionCount[connection->GetDestWin()];
            }
            std::multimap<sal_Int32 , OTableWindow*> aMulti;
            for (auto const& elem : aConnectionCount)
            {
                aMulti.emplace(elem.second,elem.first);
            }

            const bool bUseEscape = ::dbtools::getBooleanDataSourceSetting( _xConnection, PROPERTY_OUTERJOINESCAPE );
            std::multimap<sal_Int32 , OTableWindow*>::const_reverse_iterator aRIter = aMulti.rbegin();
            std::multimap<sal_Int32 , OTableWindow*>::const_reverse_iterator aREnd = aMulti.rend();
            for(;aRIter != aREnd;++aRIter)
            {
                auto aConIter = aRIter->second->getTableView()->getTableConnections(aRIter->second);
                for(;aConIter != aEnd;++aConIter)
                {
                    OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>((*aConIter).get());
                    if(!pEntryConn->IsVisited() && pEntryConn->GetSourceWin() == aRIter->second )
                    {
                        OUString aJoin;
                        GetNextJoin(_xConnection,
                                    pEntryConn,
                                    static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()),
                                    aJoin,
                                    aTableNames);

                        if(!aJoin.isEmpty())
                        {
                            OUString aStr;
                            switch(static_cast<OQueryTableConnectionData*>(pEntryConn->GetData().get())->GetJoinType())
                            {
                                case LEFT_JOIN:
                                case RIGHT_JOIN:
                                case FULL_JOIN:
                                    {
                                        // create outer join
                                        if ( bUseEscape )
                                            aStr += "{ oj ";
                                        aStr += aJoin;
                                        if ( bUseEscape )
                                            aStr += " }";
                                    }
                                    break;
                                default:
                                    aStr += aJoin;
                                    break;
                            }
                            aStr += ",";
                            aTableListStr += aStr;
                        }
                    }
                }
            }

            // and now all inner joins
            // these are implemented as
            // "FROM tbl1, tbl2 WHERE tbl1.col1=tlb2.col2"
            // rather than
            // "FROM tbl1 INNER JOIN tbl2 ON tbl1.col1=tlb2.col2"
            for (auto const& connection : rConnList)
            {
                OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>(connection.get());
                if(!pEntryConn->IsVisited())
                {
                    searchAndAppendName(_xConnection,
                                        static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin()),
                                        aTableNames,
                                        aTableListStr);

                    searchAndAppendName(_xConnection,
                                        static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()),
                                        aTableNames,
                                        aTableListStr);
                }
            }
        }
        // all tables that haven't a connection to anyone
        for (auto const& table : *pTabList)
        {
            const OQueryTableWindow* pEntryTab = static_cast<const OQueryTableWindow*>(table.second.get());
            if(!pEntryTab->ExistsAConn())
            {
                aTableListStr += BuildTable(_xConnection,pEntryTab) + ",";
            }
        }

        if(!aTableListStr.isEmpty())
            aTableListStr = aTableListStr.replaceAt(aTableListStr.getLength()-1,1, OUString() );
        return aTableListStr;
    }
    OUString GenerateGroupBy(const OQueryDesignView* _pView,OTableFields& _rFieldList, bool bMulti )
    {
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        const Reference< XConnection> xConnection = rController.getConnection();
        if(!xConnection.is())
            return OUString();

        std::map< OUString,bool> aGroupByNames;

        OUString aGroupByStr;
        try
        {
            const Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            const OUString aQuote = xMetaData->getIdentifierQuoteString();

            for (auto const& field : _rFieldList)
            {
                if ( field->IsGroupBy() )
                {
                    OSL_ENSURE(!field->GetField().isEmpty(),"No Field Name available!;-(");
                    OUString sGroupByPart = quoteTableAlias(bMulti,field->GetAlias(),aQuote);

                    // only quote the field name when it isn't calculated
                    if ( field->isNoneFunction() )
                    {
                        sGroupByPart += ::dbtools::quoteName(aQuote, field->GetField());
                    }
                    else
                    {
                        OUString aTmp = field->GetField();
                        OUString aErrorMsg;
                        Reference<XPropertySet> xColumn;
                        std::unique_ptr< ::connectivity::OSQLParseNode> pParseNode(_pView->getPredicateTreeFromEntry(field,aTmp,aErrorMsg,xColumn));
                        if (pParseNode)
                        {
                            OUString sGroupBy;
                            pParseNode->getChild(0)->parseNodeToStr(    sGroupBy,
                                                        xConnection,
                                                        &rController.getParser().getContext(),
                                                        false,
                                                        !field->isOtherFunction());
                            sGroupByPart += sGroupBy;
                        }
                        else
                            sGroupByPart += field->GetField();
                    }
                    if ( aGroupByNames.find(sGroupByPart) == aGroupByNames.end() )
                    {
                        aGroupByNames.emplace(sGroupByPart,true);
                        aGroupByStr += sGroupByPart + ",";
                    }
                }
            }
            if ( !aGroupByStr.isEmpty() )
            {
                aGroupByStr = aGroupByStr.replaceAt(aGroupByStr.getLength()-1,1, OUString(' ') );
                OUString aGroupByStr2 = " GROUP BY " + aGroupByStr;
                aGroupByStr = aGroupByStr2;
            }
        }
        catch(SQLException&)
        {
            OSL_FAIL("Failure while building group by!");
        }
        return aGroupByStr;
    }
    SqlParseError GetORCriteria(OQueryDesignView* _pView,
                                OSelectionBrowseBox* _pSelectionBrw,
                                const ::connectivity::OSQLParseNode * pCondition,
                                sal_uInt16& nLevel ,
                                bool bHaving = false,
                                bool bAddOrOnOneLine = false);
    SqlParseError GetSelectionCriteria( OQueryDesignView* _pView,
                                        OSelectionBrowseBox* _pSelectionBrw,
                                        const ::connectivity::OSQLParseNode* pNode,
                                        sal_uInt16& rLevel )
    {
        if (!pNode || !SQL_ISRULE(pNode, select_statement))
            return eNoSelectStatement;

        // nyi: more checking for the correct structure!
        pNode = pNode->getChild(3)->getChild(1);
        // no where clause found
        if (!pNode || pNode->isLeaf())
            return eOk;

        // Next free sentence...
        SqlParseError eErrorCode = eOk;
        ::connectivity::OSQLParseNode * pCondition = pNode->getChild(1);
        if ( pCondition ) // no where clause
        {
            // now we have to check the other conditions
            // first make the logical easier
            ::connectivity::OSQLParseNode::negateSearchCondition(pCondition);
            ::connectivity::OSQLParseNode *pNodeTmp = pNode->getChild(1);

            ::connectivity::OSQLParseNode::disjunctiveNormalForm(pNodeTmp);
            pNodeTmp = pNode->getChild(1);
            ::connectivity::OSQLParseNode::absorptions(pNodeTmp);
            pNodeTmp = pNode->getChild(1);
            // compress sort the criteria @see https://bz.apache.org/ooo/show_bug.cgi?id=24079
            OSQLParseNode::compress(pNodeTmp);
            pNodeTmp = pNode->getChild(1);

            // first extract the inner joins conditions
            GetInnerJoinCriteria(_pView,pNodeTmp);
            // now simplify again, join are checked in ComparisonPredicate
            ::connectivity::OSQLParseNode::absorptions(pNodeTmp);
            pNodeTmp = pNode->getChild(1);

            // it could happen that pCondition is not more valid
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pNodeTmp, rLevel);
        }
        return eErrorCode;
    }
    SqlParseError GetANDCriteria(   OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const  ::connectivity::OSQLParseNode * pCondition,
                                    sal_uInt16& nLevel,
                                    bool bHaving,
                                    bool bAddOrOnOneLine);
    SqlParseError ComparisonPredicate(OQueryDesignView const * _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const sal_uInt16 nLevel,
                            bool bHaving,
                            bool bAddOrOnOneLine);
    SqlParseError GetORCriteria(OQueryDesignView* _pView,
                                OSelectionBrowseBox* _pSelectionBrw,
                                const ::connectivity::OSQLParseNode * pCondition,
                                sal_uInt16& nLevel ,
                                bool bHaving,
                                bool bAddOrOnOneLine)
    {
        SqlParseError eErrorCode = eOk;

        // round brackets around the printout
        if (pCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
        {
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(1),nLevel,bHaving,bAddOrOnOneLine);
        }
        // OR condition
        // a searchcondition can only look like this: search_condition SQL_TOKEN_OR boolean_term
        else if (SQL_ISRULE(pCondition,search_condition))
        {
            for (int i = 0; i < 3 && eErrorCode == eOk ; i+=2)
            {
                const  ::connectivity::OSQLParseNode* pChild = pCondition->getChild(i);
                if ( SQL_ISRULE(pChild,search_condition) )
                    eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pChild,nLevel,bHaving,bAddOrOnOneLine);
                else
                {
                    eErrorCode = GetANDCriteria(_pView,_pSelectionBrw,pChild, nLevel,bHaving, i != 0 && bAddOrOnOneLine);
                    if ( !bAddOrOnOneLine)
                        nLevel++;
                }
            }
        }
        else
            eErrorCode = GetANDCriteria( _pView,_pSelectionBrw,pCondition, nLevel, bHaving,bAddOrOnOneLine );

        return eErrorCode;
    }
    bool CheckOrCriteria(const ::connectivity::OSQLParseNode* _pCondition,::connectivity::OSQLParseNode* _pFirstColumnRef)
    {
        bool bRet = true;
        ::connectivity::OSQLParseNode* pFirstColumnRef = _pFirstColumnRef;
        for (size_t i = 0; bRet && i < _pCondition->count(); ++i)
        {
            const  ::connectivity::OSQLParseNode* pChild = _pCondition->getChild(i);
            if ( pChild->isToken() )
                continue;
            else if ( SQL_ISRULE(pChild,search_condition) )
                bRet = CheckOrCriteria(pChild,pFirstColumnRef);
            else
            {
                // this is a simple way to test columns are the same, may be we have to adjust this algo a little bit in future. :-)
                ::connectivity::OSQLParseNode* pSecondColumnRef = pChild->getByRule(::connectivity::OSQLParseNode::column_ref);
                if ( pFirstColumnRef && pSecondColumnRef )
                    bRet = *pFirstColumnRef == *pSecondColumnRef;
                else if ( !pFirstColumnRef )
                    pFirstColumnRef = pSecondColumnRef;
            }
        }
        return bRet;
    }
    SqlParseError GetANDCriteria(   OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const  ::connectivity::OSQLParseNode * pCondition,
                                    sal_uInt16& nLevel,
                                    bool bHaving,
                                    bool bAddOrOnOneLine)
    {
        const css::lang::Locale    aLocale = _pView->getLocale();
        const OUString sDecimal = _pView->getDecimalSeparator();

        // I will need a cast pointer to my css::sdbcx::Container
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        SqlParseError eErrorCode = eOk;

        // round brackets
        if (SQL_ISRULE(pCondition,boolean_primary))
        {
            // check if we have to put the or criteria on one line.
            const  ::connectivity::OSQLParseNode* pSearchCondition = pCondition->getChild(1);
            bool bMustAddOrOnOneLine = CheckOrCriteria(pSearchCondition,nullptr);
            if ( SQL_ISRULE( pSearchCondition, search_condition) ) // we have a or
            {
                _pSelectionBrw->DuplicateConditionLevel( nLevel);
                eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSearchCondition->getChild(0), nLevel,bHaving,bMustAddOrOnOneLine );
                if ( eErrorCode == eOk )
                {
                    ++nLevel;
                    eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSearchCondition->getChild(2), nLevel,bHaving,bMustAddOrOnOneLine );
                }
            }
            else
                eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSearchCondition, nLevel,bHaving,bMustAddOrOnOneLine );
        }
        // The first element is (again) an AND condition
        else if ( SQL_ISRULE(pCondition,boolean_term) )
        {
            OSL_ENSURE(pCondition->count() == 3,"Illegal definition of boolean_term");
            eErrorCode = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(0), nLevel,bHaving,bAddOrOnOneLine );
            if ( eErrorCode == eOk )
                eErrorCode = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(2), nLevel,bHaving,bAddOrOnOneLine );
        }
        else if (SQL_ISRULE( pCondition, comparison_predicate))
        {
            eErrorCode = ComparisonPredicate(_pView,_pSelectionBrw,pCondition,nLevel,bHaving,bAddOrOnOneLine);
        }
        else if( SQL_ISRULE(pCondition,like_predicate) )
        {
            const  ::connectivity::OSQLParseNode* pValueExp = pCondition->getChild(0);
            if (SQL_ISRULE(pValueExp, column_ref ) )
            {
                OUString aCondition;
                Reference< XConnection> xConnection = rController.getConnection();
                if ( xConnection.is() )
                {
                    OUString aColumnName;
                    // the international doesn't matter I have a string
                    pCondition->parseNodeToPredicateStr(aCondition,
                                                        xConnection,
                                                        rController.getNumberFormatter(),
                                                        aLocale,
                                                        static_cast<sal_Char>(sDecimal.toChar()),
                                                        &rController.getParser().getContext());

                    pValueExp->parseNodeToPredicateStr( aColumnName,
                                                        xConnection,
                                                        rController.getNumberFormatter(),
                                                        aLocale,
                                                        static_cast<sal_Char>(sDecimal.toChar()),
                                                        &rController.getParser().getContext());

                    // don't display the column name
                    aCondition = aCondition.copy(aColumnName.getLength());
                    aCondition = aCondition.trim();
                }

                OTableFieldDescRef aDragLeft = new OTableFieldDesc();
                if ( eOk == ( eErrorCode = FillDragInfo(_pView,pValueExp,aDragLeft) ))
                {
                    if ( bHaving )
                        aDragLeft->SetGroupBy(true);
                    _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel,bAddOrOnOneLine);
                }
            }
            else if(SQL_ISRULEOR3(pValueExp, general_set_fct, set_fct_spec, position_exp)  ||
                      SQL_ISRULEOR3(pValueExp, extract_exp, fold, char_substring_fct)       ||
                      SQL_ISRULEOR2(pValueExp, length_exp, char_value_fct))
            {
                AddFunctionCondition(   _pView,
                                        _pSelectionBrw,
                                        pCondition,
                                        nLevel,
                                        bHaving,
                                        bAddOrOnOneLine);
            }
            else
            {
                eErrorCode = eNoColumnInLike;
                OUString sError(DBA_RES(STR_QRY_LIKE_LEFT_NO_COLUMN));
                _pView->getController().appendError( sError );
            }
        }
        else if(    SQL_ISRULEOR2(pCondition,test_for_null,in_predicate)
                ||  SQL_ISRULEOR2(pCondition,all_or_any_predicate,between_predicate))
        {
            if ( SQL_ISRULEOR2(pCondition->getChild(0), set_fct_spec , general_set_fct ) )
            {
                AddFunctionCondition(   _pView,
                                        _pSelectionBrw,
                                        pCondition,
                                        nLevel,
                                        bHaving,
                                        bAddOrOnOneLine);
            }
            else if ( SQL_ISRULE(pCondition->getChild(0), column_ref ) )
            {
                // parse condition
                OUString sCondition = ParseCondition(rController,pCondition,sDecimal,aLocale,1);
                OTableFieldDescRef  aDragLeft = new OTableFieldDesc();
                if ( eOk == ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(0),aDragLeft)) )
                {
                    if ( bHaving )
                        aDragLeft->SetGroupBy(true);
                    _pSelectionBrw->AddCondition(aDragLeft, sCondition, nLevel,bAddOrOnOneLine);
                }
            }
            else
            {
                // Parse the function condition
                OUString sCondition = ParseCondition(rController,pCondition,sDecimal,aLocale,1);
                Reference< XConnection> xConnection = rController.getConnection();
                    // the international doesn't matter I have a string
                OUString sName;
                pCondition->getChild(0)->parseNodeToPredicateStr(sName,
                                                    xConnection,
                                                    rController.getNumberFormatter(),
                                                    aLocale,
                                                    static_cast<sal_Char>(sDecimal.toChar()),
                                                    &rController.getParser().getContext());

                OTableFieldDescRef aDragLeft = new OTableFieldDesc();
                aDragLeft->SetField(sName);
                aDragLeft->SetFunctionType(FKT_OTHER);

                if ( bHaving )
                    aDragLeft->SetGroupBy(true);
                _pSelectionBrw->AddCondition(aDragLeft, sCondition, nLevel,bAddOrOnOneLine);
            }
        }
        else if( SQL_ISRULEOR2(pCondition,existence_test,unique_test) )
        {
            // Parse the function condition
            OUString aCondition = ParseCondition(rController,pCondition,sDecimal,aLocale,0);

            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            aDragLeft->SetField(aCondition);
            aDragLeft->SetFunctionType(FKT_CONDITION);

            eErrorCode = _pSelectionBrw->InsertField(aDragLeft,BROWSER_INVALIDID,false).is() ? eOk : eTooManyColumns;
        }
        else //! TODO not supported yet
            eErrorCode = eStatementTooComplex;
        // Pass on the error code
        return eErrorCode;
    }
    SqlParseError AddFunctionCondition(OQueryDesignView const * _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const sal_uInt16 nLevel,
                            bool bHaving,
                            bool bAddOrOnOneLine)
    {
        SqlParseError eErrorCode = eOk;
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());

        OSQLParseNode* pFunction = pCondition->getChild(0);

        OSL_ENSURE(SQL_ISRULEOR3(pFunction, general_set_fct, set_fct_spec, position_exp)  ||
                     SQL_ISRULEOR3(pFunction, extract_exp, fold, char_substring_fct)      ||
                     SQL_ISRULEOR2(pFunction,length_exp,char_value_fct),
                   "Illegal call!");

        Reference< XConnection> xConnection = rController.getConnection();
        if(xConnection.is())
        {
            OUString aCondition;
            OUString aColumnName;
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            pCondition->parseNodeToPredicateStr(aCondition,
                                                xConnection,
                                                rController.getNumberFormatter(),
                                                _pView->getLocale(),
                                                static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                &rController.getParser().getContext());

            pFunction->parseNodeToStr(  aColumnName,
                                        xConnection,
                                        &rController.getParser().getContext(),
                                        true); // quote is to true because we need quoted elements inside the function
            // don't display the column name
            aCondition = aCondition.copy(aColumnName.getLength());
            aCondition = aCondition.trim();
            if ( aCondition.startsWith("=") ) // ignore the equal sign
                aCondition = aCondition.copy(1);

            if ( SQL_ISRULE(pFunction, general_set_fct ) )
            {
                sal_Int32 nFunctionType = FKT_AGGREGATE;
                OSQLParseNode* pParamNode = pFunction->getChild(pFunction->count()-2);
                if ( pParamNode && pParamNode->getTokenValue().toChar() == '*' )
                {
                    OJoinTableView::OTableWindowMap& rTabList = _pView->getTableView()->GetTabWinMap();
                    for (auto const& table : rTabList)
                    {
                        OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(table.second.get());
                        if (pTabWin->ExistsField( "*", aDragLeft ))
                        {
                            aDragLeft->SetAlias(OUString());
                            aDragLeft->SetTable(OUString());
                            break;
                        }
                    }
                }
                else if (pParamNode && eOk != (eErrorCode = FillDragInfo(_pView,pParamNode,aDragLeft))
                        && SQL_ISRULE(pParamNode,num_value_exp))
                {
                    OUString sParameterValue;
                    pParamNode->parseNodeToStr( sParameterValue,
                                                xConnection,
                                                &rController.getParser().getContext());
                    nFunctionType |= FKT_NUMERIC;
                    aDragLeft->SetField(sParameterValue);
                    eErrorCode = eOk;
                }
                aDragLeft->SetFunctionType(nFunctionType);
                if ( bHaving )
                    aDragLeft->SetGroupBy(true);
                aDragLeft->SetFunction(aColumnName.getToken(0, '('));
            }
            else
            {
                // for an unknown function we write the whole text in the field
                aDragLeft->SetField(aColumnName);
                if(bHaving)
                    aDragLeft->SetGroupBy(true);
                aDragLeft->SetFunctionType(FKT_OTHER|FKT_NUMERIC);
            }
            _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel,bAddOrOnOneLine);
        }

        return eErrorCode;
    }
    SqlParseError ComparisonPredicate(OQueryDesignView const * _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const sal_uInt16 nLevel,
                            bool bHaving
                            ,bool bAddOrOnOneLine)
    {
        SqlParseError eErrorCode = eOk;
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());

        OSL_ENSURE(SQL_ISRULE( pCondition, comparison_predicate),"ComparisonPredicate: pCondition is not a Comparison Predicate");
        if ( SQL_ISRULE(pCondition->getChild(0), column_ref )
            || SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref) )
        {
            OUString aCondition;
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();

            if ( SQL_ISRULE(pCondition->getChild(0), column_ref ) && SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ) )
            {
                OTableFieldDescRef aDragRight = new OTableFieldDesc();
                if (eOk != ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(0),aDragLeft)) ||
                    eOk != ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(2),aDragRight)))
                    return eErrorCode;

                OQueryTableConnection* pConn = static_cast<OQueryTableConnection*>(
                                                    _pView->getTableView()->GetTabConn(static_cast<OQueryTableWindow*>(aDragLeft->GetTabWindow()),
                                                                                       static_cast<OQueryTableWindow*>(aDragRight->GetTabWindow()),
                                                                                       true));
                if ( pConn )
                {
                    OConnectionLineDataVec& rLineDataList = pConn->GetData()->GetConnLineDataList();
                    for (auto const& lineData : rLineDataList)
                    {
                        if(lineData->GetSourceFieldName() == aDragLeft->GetField() ||
                           lineData->GetDestFieldName() == aDragLeft->GetField() )
                            return eOk;
                    }
                }
            }

            sal_uInt32 nPos = 0;
            if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
            {
                nPos = 0;
                sal_uInt32 i=1;

                // don't display the equal
                if (pCondition->getChild(i)->getNodeType() == SQLNodeType::Equal)
                    i++;

                // parse the condition
                aCondition = ParseCondition(rController
                                            ,pCondition
                                            ,_pView->getDecimalSeparator()
                                            ,_pView->getLocale()
                                            ,i);
            }
            else if( SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ) )
            {
                nPos = pCondition->count()-1;

                sal_Int32 i = static_cast<sal_Int32>(pCondition->count() - 2);
                switch (pCondition->getChild(i)->getNodeType())
                {
                    case SQLNodeType::Equal:
                        // don't display the equal
                        i--;
                        break;
                    case SQLNodeType::Less:
                        // take the opposite as we change the order
                        i--;
                        aCondition += ">";
                        break;
                    case SQLNodeType::LessEq:
                        // take the opposite as we change the order
                        i--;
                        aCondition += ">=";
                        break;
                    case SQLNodeType::Great:
                        // take the opposite as we change the order
                        i--;
                        aCondition += "<";
                        break;
                    case SQLNodeType::GreatEq:
                        // take the opposite as we change the order
                        i--;
                        aCondition += "<=";
                        break;
                    default:
                        break;
                }

                // go backward
                Reference< XConnection> xConnection = rController.getConnection();
                if(xConnection.is())
                {
                    for (; i >= 0; i--)
                        pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                                xConnection,
                                                rController.getNumberFormatter(),
                                                _pView->getLocale(),
                                                static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                &rController.getParser().getContext());
                }
            }
            // else ???

            if( eOk == ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(nPos),aDragLeft)))
            {
                if(bHaving)
                    aDragLeft->SetGroupBy(true);
                _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel,bAddOrOnOneLine);
            }
        }
        else if( SQL_ISRULEOR2(pCondition->getChild(0), set_fct_spec , general_set_fct ) )
        {
            AddFunctionCondition(   _pView,
                                    _pSelectionBrw,
                                    pCondition,
                                    nLevel,
                                    bHaving,
                                    bAddOrOnOneLine);
        }
        else // it can only be an Expr
        {
            OUString aName,aCondition;

            // Field name
            Reference< XConnection> xConnection = rController.getConnection();
            if(xConnection.is())
            {
                ::connectivity::OSQLParseNode *pLhs = pCondition->getChild(0);
                ::connectivity::OSQLParseNode *pRhs = pCondition->getChild(2);
                pLhs->parseNodeToStr(aName,
                                     xConnection,
                                     &rController.getParser().getContext(),
                                     true);
                // Criteria
                aCondition = pCondition->getChild(1)->getTokenValue();
                pRhs->parseNodeToPredicateStr(aCondition,
                                                            xConnection,
                                                            rController.getNumberFormatter(),
                                                            _pView->getLocale(),
                                                            static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                            &rController.getParser().getContext());
            }

            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            aDragLeft->SetField(aName);
            aDragLeft->SetFunctionType(FKT_OTHER|FKT_NUMERIC);
            // and add it on
            _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel,bAddOrOnOneLine);
        }
        return eErrorCode;
    }

    OQueryTableWindow* lcl_findColumnInTables( const OUString& _rColumName, const OJoinTableView::OTableWindowMap& _rTabList, OTableFieldDescRef const & _rInfo )
    {
        for (auto const& table : _rTabList)
        {
            OQueryTableWindow* pTabWin = static_cast< OQueryTableWindow* >( table.second.get() );
            if ( pTabWin && pTabWin->ExistsField( _rColumName, _rInfo ) )
                return pTabWin;
        }
        return nullptr;
    }

    void InsertColumnRef(const OQueryDesignView* _pView,
                        const ::connectivity::OSQLParseNode * pColumnRef,
                        OUString& aColumnName,
                        const OUString& aColumnAlias,
                        OUString& aTableRange,
                        OTableFieldDescRef const & _raInfo,
                        OJoinTableView::OTableWindowMap const * pTabList)
    {

        // Put the table names together
        ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController&>(_pView->getController()).getParseIterator();
        rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

        bool bFound(false);
        OSL_ENSURE(!aColumnName.isEmpty(),"Column name must not be empty");
        if (aTableRange.isEmpty())
        {
            // SELECT column, ...
            bFound = nullptr != lcl_findColumnInTables( aColumnName, *pTabList, _raInfo );
            if ( bFound && ( aColumnName.toChar() != '*' ) )
                _raInfo->SetFieldAlias(aColumnAlias);
        }
        else
        {
            // SELECT range.column, ...
            OQueryTableWindow* pTabWin = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable(aTableRange);

            if (pTabWin && pTabWin->ExistsField(aColumnName, _raInfo))
            {
                if(aColumnName.toChar() != '*')
                    _raInfo->SetFieldAlias(aColumnAlias);
                bFound = true;
            }
        }
        if (!bFound)
        {
            _raInfo->SetTable(OUString());
            _raInfo->SetAlias(OUString());
            _raInfo->SetField(aColumnName);
            _raInfo->SetFieldAlias(aColumnAlias);   // nyi : here it continues Expr_1, Expr_2 ...
            _raInfo->SetFunctionType(FKT_OTHER);
        }
    }
    bool checkJoinConditions(   const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode* _pNode )
    {
        const ::connectivity::OSQLParseNode* pJoinNode = nullptr;
        bool bRet = true;
        if (SQL_ISRULE(_pNode,qualified_join))
            pJoinNode = _pNode;
        else if (SQL_ISRULE(_pNode,table_ref)
                &&  _pNode->count() == 3
                &&  SQL_ISPUNCTUATION(_pNode->getChild(0),"(")
                &&  SQL_ISPUNCTUATION(_pNode->getChild(2),")") ) // '(' joined_table ')'
            pJoinNode = _pNode->getChild(1);
        else if (! ( SQL_ISRULE(_pNode, table_ref) && _pNode->count() == 2) ) // table_node table_primary_as_range_column
            bRet = false;

        if (pJoinNode && !InsertJoin(_pView,pJoinNode))
            bRet = false;
        return bRet;
    }
    bool InsertJoin(const OQueryDesignView* _pView,
                        const ::connectivity::OSQLParseNode *pNode)
    {
        OSL_ENSURE( SQL_ISRULE( pNode, qualified_join ) || SQL_ISRULE( pNode, joined_table ) || SQL_ISRULE( pNode, cross_union ),
            "OQueryDesignView::InsertJoin: Error in the Parse Tree");

        if (SQL_ISRULE(pNode,joined_table))
            return InsertJoin(_pView,pNode->getChild(1));

        // first check the left and right side
        const ::connectivity::OSQLParseNode* pRightTableRef = pNode->getChild(3); // table_ref
        if ( SQL_ISRULE(pNode, qualified_join) && SQL_ISTOKEN(pNode->getChild(1),NATURAL) )
            pRightTableRef = pNode->getChild(4); // table_ref

        if ( !checkJoinConditions(_pView,pNode->getChild(0)) || !checkJoinConditions(_pView,pRightTableRef))
            return false;

        // named column join may be implemented later
        // SQL_ISRULE(pNode->getChild(4),named_columns_join)
        EJoinType eJoinType = INNER_JOIN;
        bool bNatural = false;
        if ( SQL_ISRULE(pNode, qualified_join) )
        {
            ::connectivity::OSQLParseNode* pJoinType = pNode->getChild(1); // join_type
            if ( SQL_ISTOKEN(pJoinType,NATURAL) )
            {
                bNatural = true;
                pJoinType = pNode->getChild(2);
            }

            if (SQL_ISRULE(pJoinType,join_type) && (!pJoinType->count() || SQL_ISTOKEN(pJoinType->getChild(0),INNER)))
            {
                eJoinType = INNER_JOIN;
            }
            else
            {
                if (SQL_ISRULE(pJoinType,join_type))       // one level deeper
                    pJoinType = pJoinType->getChild(0);

                if (SQL_ISTOKEN(pJoinType->getChild(0),LEFT))
                    eJoinType = LEFT_JOIN;
                else if(SQL_ISTOKEN(pJoinType->getChild(0),RIGHT))
                    eJoinType = RIGHT_JOIN;
                else
                    eJoinType = FULL_JOIN;
            }
            if ( SQL_ISRULE(pNode->getChild(4),join_condition) )
            {
                if ( InsertJoinConnection(_pView,pNode->getChild(4)->getChild(1), eJoinType,pNode->getChild(0),pRightTableRef) != eOk )
                    return false;
            }
        }
        else if ( SQL_ISRULE(pNode, cross_union) )
        {
            eJoinType = CROSS_JOIN;
            pRightTableRef = pNode->getChild(pNode->count() - 1);
        }
        else
            return false;

        if ( eJoinType == CROSS_JOIN || bNatural )
        {

            OQueryTableWindow*  pLeftWindow = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( getTableRange(_pView,pNode->getChild(0)) );
            OQueryTableWindow*  pRightWindow = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( getTableRange(_pView,pRightTableRef) );
            OSL_ENSURE(pLeftWindow && pRightWindow,"Table Windows could not be found!");
            if ( !pLeftWindow || !pRightWindow )
                return false;

            OTableFieldDescRef aDragLeft  = new OTableFieldDesc();
            aDragLeft->SetTabWindow(pLeftWindow);
            aDragLeft->SetTable(pLeftWindow->GetTableName());
            aDragLeft->SetAlias(pLeftWindow->GetAliasName());

            OTableFieldDescRef aDragRight = new OTableFieldDesc();
            aDragRight->SetTabWindow(pRightWindow);
            aDragRight->SetTable(pRightWindow->GetTableName());
            aDragRight->SetAlias(pRightWindow->GetAliasName());

            insertConnection(_pView,eJoinType,aDragLeft,aDragRight,bNatural);
        }

        return true;
    }
    void insertUnUsedFields(OQueryDesignView const * _pView,OSelectionBrowseBox* _pSelectionBrw)
    {
        // now we have to insert the fields which aren't in the statement
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        OTableFields& rUnUsedFields = rController.getUnUsedFields();
        for (auto & unusedField : rUnUsedFields)
            if(_pSelectionBrw->InsertField(unusedField,BROWSER_INVALIDID,false,false).is())
                unusedField = nullptr;
        OTableFields().swap( rUnUsedFields );
    }

    SqlParseError InitFromParseNodeImpl(OQueryDesignView* _pView,OSelectionBrowseBox* _pSelectionBrw)
    {
        SqlParseError eErrorCode = eOk;

        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());

        _pSelectionBrw->PreFill();
        _pSelectionBrw->SetReadOnly(rController.isReadOnly());
        _pSelectionBrw->Fill();

        ::connectivity::OSQLParseTreeIterator& aIterator = rController.getParseIterator();
        const ::connectivity::OSQLParseNode* pParseTree = aIterator.getParseTree();

        do
        {
            if ( !pParseTree )
            {
                // now we have to insert the fields which aren't in the statement
                insertUnUsedFields(_pView,_pSelectionBrw);
                break;
            }

            if ( !rController.isEscapeProcessing() ) // not allowed in this mode
            {
                eErrorCode = eNativeMode;
                break;
            }

            if ( !( SQL_ISRULE( pParseTree, select_statement ) ) )
            {
                eErrorCode = eNoSelectStatement;
                break;
            }

            const OSQLParseNode* pTableExp = pParseTree->getChild(3);
            if ( pTableExp->getChild(7)->count() > 0 || pTableExp->getChild(8)->count() > 0)
            {
                eErrorCode = eStatementTooComplex;
                break;
            }

            Reference< XConnection> xConnection = rController.getConnection();
            if ( !xConnection.is() )
            {
                OSL_FAIL( "InitFromParseNodeImpl: no connection? no connection!" );
                break;
            }

            const OSQLTables& aMap = aIterator.getTables();
            ::comphelper::UStringMixLess aTmp(aMap.key_comp());
            ::comphelper::UStringMixEqual aKeyComp( aTmp.isCaseSensitive() );

            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            try
            {
                sal_Int32 nMax = xMetaData->getMaxTablesInSelect();
                if ( nMax && nMax < static_cast<sal_Int32>(aMap.size()) )
                {
                    eErrorCode = eTooManyTables;
                    break;
                }

                OUString sComposedName;
                OUString sAlias;

                OQueryTableView* pTableView = static_cast<OQueryTableView*>(_pView->getTableView());
                pTableView->clearLayoutInformation();
                for (auto const& elem : aMap)
                {
                    OSQLTable xTable = elem.second;
                    Reference< XPropertySet > xTableProps( xTable, UNO_QUERY_THROW );

                    sAlias = elem.first;

                    // check whether this is a query
                    Reference< XPropertySetInfo > xPSI = xTableProps->getPropertySetInfo();
                    bool bIsQuery = xPSI.is() && xPSI->hasPropertyByName( PROPERTY_COMMAND );

                    if ( bIsQuery )
                        OSL_VERIFY( xTableProps->getPropertyValue( PROPERTY_NAME ) >>= sComposedName );
                    else
                    {
                        sComposedName = ::dbtools::composeTableName( xMetaData, xTableProps, ::dbtools::EComposeRule::InDataManipulation, false );

                        // if the alias is the complete (composed) table, then shorten it
                        if ( aKeyComp( sComposedName, elem.first ) )
                        {
                            OUString sCatalog, sSchema, sTable;
                            ::dbtools::qualifiedNameComponents( xMetaData, sComposedName, sCatalog, sSchema, sTable, ::dbtools::EComposeRule::InDataManipulation );
                            sAlias = sTable;
                        }
                    }

                    // find the existent window for this alias
                    OQueryTableWindow* pExistentWin = pTableView->FindTable( sAlias );
                    if ( !pExistentWin )
                    {
                        pTableView->AddTabWin( sComposedName, sAlias );  // don't create data here
                    }
                    else
                    {
                        // there already exists a window for this alias ....
                        if ( !aKeyComp( pExistentWin->GetData()->GetComposedName(), sComposedName ) )
                            // ... but for another complete table name -> new window
                            pTableView->AddTabWin(sComposedName, sAlias);
                    }
                }

                // now delete the data for which we haven't any tablewindow
                OJoinTableView::OTableWindowMap aTableMap(pTableView->GetTabWinMap());
                for (auto const& table : aTableMap)
                {
                    if(aMap.find(table.second->GetComposedName())  == aMap.end() &&
                        aMap.find(table.first)                     == aMap.end())
                        pTableView->RemoveTabWin(table.second);
                }

                if ( eOk == (eErrorCode = FillOuterJoins(_pView,pTableExp->getChild(0)->getChild(1))) )
                {
                    // check if we have a distinct statement
                    if(SQL_ISTOKEN(pParseTree->getChild(1),DISTINCT))
                    {
                        rController.setDistinct(true);
                        rController.InvalidateFeature(SID_QUERY_DISTINCT_VALUES);
                    }
                    else
                    {
                        rController.setDistinct(false);
                    }

                    ///check if query has a limit
                    if( pTableExp->getChild(6)->count() >= 2 && pTableExp->getChild(6)->getChild(1) )
                    {
                        rController.setLimit(
                            pTableExp->getChild(6)->getChild(1)->getTokenValue().toInt64() );
                    }
                    else
                    {
                        rController.setLimit(-1);
                    }

                    if ( (eErrorCode = InstallFields(_pView, pParseTree, &pTableView->GetTabWinMap())) == eOk )
                    {
                        // GetSelectionCriteria must be called before GetHavingCriteria
                        sal_uInt16 nLevel=0;

                        if ( eOk == (eErrorCode = GetSelectionCriteria(_pView,_pSelectionBrw,pParseTree,nLevel)) )
                        {
                            if ( eOk == (eErrorCode = GetGroupCriteria(_pView,_pSelectionBrw,pParseTree)) )
                            {
                                if ( eOk == (eErrorCode = GetHavingCriteria(_pView,_pSelectionBrw,pParseTree,nLevel)) )
                                {
                                    if ( eOk == (eErrorCode = GetOrderCriteria(_pView,_pSelectionBrw,pParseTree)) )
                                        insertUnUsedFields(_pView,_pSelectionBrw);
                                }
                            }
                        }
                    }
                }
            }
            catch(SQLException&)
            {
                OSL_FAIL("getMaxTablesInSelect!");
            }
        }
        while ( false );

        // New Undo-Actions were created in the Manager by the regeneration
        rController.ClearUndoManager();
        _pSelectionBrw->Invalidate();
        return eErrorCode;
    }
    /** fillSelectSubList
        @return
            <TRUE/> when columns could be inserted otherwise <FALSE/>
    */
    SqlParseError fillSelectSubList(    OQueryDesignView* _pView,
                                OJoinTableView::OTableWindowMap* _pTabList)
    {
        SqlParseError eErrorCode = eOk;
        bool bFirstField = true;
        for (auto const& table : *_pTabList)
        {
            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(table.second.get());
            OTableFieldDescRef  aInfo = new OTableFieldDesc();
            if (pTabWin->ExistsField( "*", aInfo ))
            {
                eErrorCode = _pView->InsertField(aInfo, bFirstField);
                bFirstField = false;
                if (eErrorCode != eOk)
                    break;
            }
        }
        return eErrorCode;
    }
    SqlParseError InstallFields(OQueryDesignView* _pView,
                                const ::connectivity::OSQLParseNode* pNode,
                                OJoinTableView::OTableWindowMap* pTabList )
    {
        if( pNode==nullptr || !SQL_ISRULE(pNode,select_statement))
            return eNoSelectStatement;

        ::connectivity::OSQLParseNode* pParseTree = pNode->getChild(2); // selection
        bool bFirstField = true;    // When initializing, the first field must be reactivated

        SqlParseError eErrorCode = eOk;

        if ( pParseTree->isRule() && SQL_ISPUNCTUATION(pParseTree->getChild(0),"*") )
        {
            // SELECT * ...
            eErrorCode = fillSelectSubList(_pView,pTabList);
        }
        else if (SQL_ISRULE(pParseTree,scalar_exp_commalist) )
        {
            // SELECT column, ...
            OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
            Reference< XConnection> xConnection = rController.getConnection();

            OUString aColumnName,aTableRange;
            for (size_t i = 0; i < pParseTree->count() && eOk == eErrorCode ; ++i)
            {
                ::connectivity::OSQLParseNode * pColumnRef = pParseTree->getChild(i);

                do {

                if ( SQL_ISRULE(pColumnRef,select_sublist) )
                {
                     eErrorCode = fillSelectSubList(_pView,pTabList);
                     break;
                }

                if ( SQL_ISRULE(pColumnRef,derived_column) )
                {
                    OUString aColumnAlias(connectivity::OSQLParseTreeIterator::getColumnAlias(pColumnRef)); // might be empty
                    pColumnRef = pColumnRef->getChild(0);
                    OTableFieldDescRef aInfo = new OTableFieldDesc();

                    if (    pColumnRef->getKnownRuleID() != OSQLParseNode::subquery &&
                            pColumnRef->count() == 3 &&
                            SQL_ISPUNCTUATION(pColumnRef->getChild(0),"(") &&
                            SQL_ISPUNCTUATION(pColumnRef->getChild(2),")")
                        )
                        pColumnRef = pColumnRef->getChild(1);

                    if (SQL_ISRULE(pColumnRef,column_ref))
                    {
                        InsertColumnRef(_pView,pColumnRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList);
                        eErrorCode = _pView->InsertField(aInfo, bFirstField);
                        bFirstField = false;
                    }
                    else if(SQL_ISRULEOR3(pColumnRef, general_set_fct, set_fct_spec, position_exp)  ||
                              SQL_ISRULEOR3(pColumnRef, extract_exp, fold, char_substring_fct)      ||
                              SQL_ISRULEOR2(pColumnRef,length_exp,char_value_fct))
                    {
                        OUString aColumns;
                        pColumnRef->parseNodeToPredicateStr(aColumns,
                                                            xConnection,
                                                            rController.getNumberFormatter(),
                                                            _pView->getLocale(),
                                                            static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                            &rController.getParser().getContext());

                        sal_Int32 nFunctionType = FKT_NONE;
                        ::connectivity::OSQLParseNode* pParamRef = nullptr;
                        sal_Int32 nColumnRefPos = pColumnRef->count() - 2;
                        if ( nColumnRefPos >= 0 && static_cast<sal_uInt32>(nColumnRefPos) < pColumnRef->count() )
                            pParamRef = pColumnRef->getChild(nColumnRefPos);

                        if ( SQL_ISRULE(pColumnRef,general_set_fct)
                            && pParamRef &&  SQL_ISRULE(pParamRef,column_ref) )
                        {
                            // Check the parameters for Column references
                            InsertColumnRef(_pView,pParamRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList);
                        }
                        else if ( SQL_ISRULE(pColumnRef,general_set_fct) )
                        {
                            if ( pParamRef && pParamRef->getTokenValue().toChar() == '*' )
                            {
                                for (auto const& table : *pTabList)
                                {
                                    OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(table.second.get());
                                    if (pTabWin->ExistsField( "*", aInfo ))
                                    {
                                        aInfo->SetAlias(OUString());
                                        aInfo->SetTable(OUString());
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                OUString sFieldName = aColumns;
                                if ( pParamRef )
                                {   // we got an aggregate function but without column name inside
                                    // so we set the whole argument of the function as field name
                                    nFunctionType |= FKT_NUMERIC;
                                    sFieldName.clear();
                                    pParamRef->parseNodeToStr(  sFieldName,
                                                        xConnection,
                                                        &rController.getParser().getContext(),
                                                        true); // quote is to true because we need quoted elements inside the function
                                }
                                aInfo->SetDataType(DataType::DOUBLE);
                                aInfo->SetFieldType(TAB_NORMAL_FIELD);
                                aInfo->SetField(sFieldName);
                            }
                            aInfo->SetTabWindow(nullptr);
                            aInfo->SetFieldAlias(aColumnAlias);
                        }
                        else
                        {
                            _pView->fillFunctionInfo(pColumnRef,aColumns,aInfo);
                            aInfo->SetFieldAlias(aColumnAlias);
                        }

                        if ( SQL_ISRULE(pColumnRef,general_set_fct) )
                        {
                            aInfo->SetFunctionType(nFunctionType|FKT_AGGREGATE);
                            aInfo->SetFunction(comphelper::string::stripEnd(aColumns.getToken(0,'('), ' '));
                        }
                        else
                            aInfo->SetFunctionType(nFunctionType|FKT_OTHER);

                        eErrorCode = _pView->InsertField(aInfo, bFirstField);
                        bFirstField = false;
                    }
                    else
                    {
                        OUString aColumns;
                        pColumnRef->parseNodeToStr( aColumns,
                                                    xConnection,
                                                    &rController.getParser().getContext(),
                                                    true); // quote is to true because we need quoted elements inside the function

                        aInfo->SetTabWindow( nullptr );

                        // since we support queries in queries, the thingie might belong to an existing "table"
                        OQueryTableWindow* pExistingTable = lcl_findColumnInTables( aColumns, *pTabList, aInfo );
                        if ( pExistingTable )
                        {
                            aInfo->SetTabWindow( pExistingTable );
                            aInfo->SetTable( pExistingTable->GetTableName() );
                            aInfo->SetAlias( pExistingTable->GetAliasName() );
                        }

                        aInfo->SetDataType(DataType::DOUBLE);
                        aInfo->SetFieldType(TAB_NORMAL_FIELD);
                        aInfo->SetField(aColumns);
                        aInfo->SetFieldAlias(aColumnAlias);
                        aInfo->SetFunctionType(FKT_NUMERIC | FKT_OTHER);

                        eErrorCode = _pView->InsertField(aInfo, bFirstField);
                        bFirstField = false;
                    }

                    break;
                }

                OSL_FAIL( "InstallFields: don't know how to interpret this parse node!" );

                } while ( false );
            }
        }
        else
            eErrorCode = eStatementTooComplex;

        return eErrorCode;
    }
    SqlParseError GetOrderCriteria( OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pParseRoot )
    {
        SqlParseError eErrorCode = eOk;
        if (!pParseRoot->getChild(3)->getChild(ORDER_BY_CHILD_POS)->isLeaf())
        {
            ::connectivity::OSQLParseNode* pNode = pParseRoot->getChild(3)->getChild(ORDER_BY_CHILD_POS)->getChild(2);
            ::connectivity::OSQLParseNode* pParamRef = nullptr;

            OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
            EOrderDir eOrderDir;
            for( size_t i=0 ; i<pNode->count() ; i++ )
            {
                OTableFieldDescRef aDragLeft = new OTableFieldDesc();
                eOrderDir = ORDER_ASC;
                ::connectivity::OSQLParseNode*  pChild = pNode->getChild( i );

                if (SQL_ISTOKEN( pChild->getChild(1), DESC ) )
                    eOrderDir = ORDER_DESC;

                ::connectivity::OSQLParseNode* pArgument = pChild->getChild(0);

                if(SQL_ISRULE(pArgument,column_ref))
                {
                    if( eOk == FillDragInfo(_pView,pArgument,aDragLeft))
                        _pSelectionBrw->AddOrder( aDragLeft, eOrderDir, i);
                    else // it could be a alias name for a field
                    {
                        OUString aTableRange,aColumnName;
                        ::connectivity::OSQLParseTreeIterator& rParseIter = rController.getParseIterator();
                        rParseIter.getColumnRange( pArgument, aColumnName, aTableRange );

                        OTableFields& aList = rController.getTableFieldDesc();
                        for (auto const& elem : aList)
                        {
                            if(elem.is() && elem->GetFieldAlias() == aColumnName)
                                elem->SetOrderDir( eOrderDir );
                        }
                    }
                }
                else if(SQL_ISRULE(pArgument, general_set_fct ) &&
                        SQL_ISRULE(pParamRef = pArgument->getChild(pArgument->count()-2),column_ref) &&
                        eOk == FillDragInfo(_pView,pParamRef,aDragLeft))
                    _pSelectionBrw->AddOrder( aDragLeft, eOrderDir, i );
                else if( SQL_ISRULE(pArgument, set_fct_spec ) )
                {

                    Reference< XConnection> xConnection = rController.getConnection();
                    if(xConnection.is())
                    {
                        OUString sCondition;
                        pArgument->parseNodeToPredicateStr(sCondition,
                                                            xConnection,
                                                            rController.getNumberFormatter(),
                                                            _pView->getLocale(),
                                                            static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                            &rController.getParser().getContext());
                        _pView->fillFunctionInfo(pArgument,sCondition,aDragLeft);
                        aDragLeft->SetFunctionType(FKT_OTHER);
                        aDragLeft->SetOrderDir(eOrderDir);
                        aDragLeft->SetVisible(false);
                        _pSelectionBrw->AddOrder( aDragLeft, eOrderDir, i );
                    }
                    else
                        eErrorCode = eColumnNotFound;
                }
                else
                    eErrorCode = eColumnNotFound;
            }
        }
        return eErrorCode;
    }
    SqlParseError GetHavingCriteria(    OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot,
                            sal_uInt16& rLevel )
    {
        SqlParseError eErrorCode = eOk;
        if (!pSelectRoot->getChild(3)->getChild(3)->isLeaf())
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSelectRoot->getChild(3)->getChild(3)->getChild(1),rLevel, true);
        return eErrorCode;
    }
    SqlParseError GetGroupCriteria( OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot )
    {
        SqlParseError eErrorCode = eOk;
        if (!pSelectRoot->getChild(3)->getChild(2)->isLeaf()) // opt_group_by_clause
        {
            OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
            ::connectivity::OSQLParseNode* pGroupBy = pSelectRoot->getChild(3)->getChild(2)->getChild(2);

            for( size_t i=0 ; i < pGroupBy->count() && eOk == eErrorCode; ++i )
            {
                OTableFieldDescRef aDragInfo = new OTableFieldDesc();
                ::connectivity::OSQLParseNode* pParamRef = nullptr;
                ::connectivity::OSQLParseNode* pArgument = pGroupBy->getChild( i );
                if(SQL_ISRULE(pArgument,column_ref))
                {
                    if ( eOk == (eErrorCode = FillDragInfo(_pView,pArgument,aDragInfo)) )
                    {
                        aDragInfo->SetGroupBy(true);
                        _pSelectionBrw->AddGroupBy(aDragInfo);
                    }
                }
                else if(SQL_ISRULE(pArgument, general_set_fct ) &&
                        SQL_ISRULE(pParamRef = pArgument->getChild(pArgument->count()-2),column_ref) &&
                        eOk == FillDragInfo(_pView,pParamRef,aDragInfo))
                {
                    aDragInfo->SetGroupBy(true);
                    _pSelectionBrw->AddGroupBy( aDragInfo );
                }
                else if( SQL_ISRULE(pArgument, set_fct_spec ) )
                {
                    Reference< XConnection> xConnection = rController.getConnection();
                    if(xConnection.is())
                    {
                        OUString sGroupByExpression;
                        pArgument->parseNodeToStr(  sGroupByExpression,
                                                    xConnection,
                                                    &rController.getParser().getContext(),
                                                    true); // quote is to true because we need quoted elements inside the function
                        _pView->fillFunctionInfo(pArgument,sGroupByExpression,aDragInfo);
                        aDragInfo->SetFunctionType(FKT_OTHER);
                        aDragInfo->SetGroupBy(true);
                        aDragInfo->SetVisible(false);
                        _pSelectionBrw->AddGroupBy( aDragInfo );
                    }
                    else
                        eErrorCode = eColumnNotFound;
                }
            }
        }
        return eErrorCode;
    }

    OUString getParseErrorMessage( SqlParseError _eErrorCode )
    {
        const char* pResId;
        switch (_eErrorCode)
        {
            case eIllegalJoin:
                pResId = STR_QRY_ILLEGAL_JOIN;
                break;
            case eStatementTooLong:
                pResId = STR_QRY_TOO_LONG_STATEMENT;
                break;
            case eNoConnection:
                pResId = STR_QRY_SYNTAX;
                break;
            case eNoSelectStatement:
                pResId = STR_QRY_NOSELECT;
                break;
            case eNoColumnInLike:
                pResId = STR_QRY_SYNTAX;
                break;
            case eColumnNotFound:
                pResId = STR_QRY_SYNTAX;
                break;
            case eNativeMode:
                pResId = STR_QRY_NATIVE;
                break;
            case eTooManyTables:
                pResId = STR_QRY_TOO_MANY_TABLES;
                break;
            case eTooManyColumns:
                pResId = STR_QRY_TOO_MANY_COLUMNS;
                break;
            case eStatementTooComplex:
                pResId = STR_QRY_TOOCOMPLEX;
                break;
            default:
                pResId = STR_QRY_SYNTAX;
                break;
        }
        ;
        return DBA_RES(pResId);
    }
}

// end of anonymous namespace

OQueryDesignView::OQueryDesignView( OQueryContainerWindow* _pParent,
                                    OQueryController& _rController,
                                    const Reference< XComponentContext >& _rxContext)
    :OJoinDesignView( _pParent, _rController, _rxContext )
    ,m_aSplitter( VclPtr<Splitter>::Create(this) )
    ,m_eChildFocus(NONE)
    ,m_bInSplitHandler( false )
{

    try
    {
        SvtSysLocale aSysLocale;
        m_aLocale = aSysLocale.GetLanguageTag().getLocale();
        m_sDecimalSep = aSysLocale.GetLocaleData().getNumDecimalSep();
    }
    catch(Exception&)
    {
    }

    m_pSelectionBox = VclPtr<OSelectionBrowseBox>::Create(this);

    setNoneVisbleRow(static_cast<OQueryController&>(getController()).getVisibleRows());
    m_pSelectionBox->Show();
    // setup Splitter
    m_aSplitter->SetSplitHdl(LINK(this, OQueryDesignView,SplitHdl));
    m_aSplitter->Show();

}

OQueryDesignView::~OQueryDesignView()
{
    disposeOnce();
}

void OQueryDesignView::dispose()
{
    if ( m_pTableView )
        ::dbaui::notifySystemWindow(this,m_pTableView,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    m_pSelectionBox.disposeAndClear();
    m_aSplitter.disposeAndClear();
    OJoinDesignView::dispose();
}

IMPL_LINK_NOARG( OQueryDesignView, SplitHdl, Splitter*, void )
{
    if (!getController().isReadOnly())
    {
        m_bInSplitHandler = true;
        m_aSplitter->SetPosPixel( Point( m_aSplitter->GetPosPixel().X(),m_aSplitter->GetSplitPosPixel() ) );
        static_cast<OQueryController&>(getController()).setSplitPos(m_aSplitter->GetSplitPosPixel());
        static_cast<OQueryController&>(getController()).setModified( true );
        Resize();
        m_bInSplitHandler = true;
    }
}

void OQueryDesignView::Construct()
{
    m_pTableView = VclPtr<OQueryTableView>::Create(m_pScrollWindow,this);
    ::dbaui::notifySystemWindow(this,m_pTableView,::comphelper::mem_fun(&TaskPaneList::AddWindow));
    OJoinDesignView::Construct();
}

void OQueryDesignView::initialize()
{
    if(static_cast<OQueryController&>(getController()).getSplitPos() != -1)
    {
        m_aSplitter->SetPosPixel( Point( m_aSplitter->GetPosPixel().X(),static_cast<OQueryController&>(getController()).getSplitPos() ) );
        m_aSplitter->SetSplitPosPixel(static_cast<OQueryController&>(getController()).getSplitPos());
    }
    m_pSelectionBox->initialize();
    reset();
}

void OQueryDesignView::resizeDocumentView(tools::Rectangle& _rPlayground)
{
    Point aPlaygroundPos( _rPlayground.TopLeft() );
    Size aPlaygroundSize( _rPlayground.GetSize() );

    // calc the split pos, and forward it to the controller
    sal_Int32 nSplitPos = static_cast<OQueryController&>(getController()).getSplitPos();
    if ( 0 != aPlaygroundSize.Height() )
    {
        if  (   ( -1 == nSplitPos )
            ||  ( nSplitPos >= aPlaygroundSize.Height() )
            )
        {
            // let the selection browse box determine an optimal size
            Size aSelectionBoxSize = m_pSelectionBox->CalcOptimalSize( aPlaygroundSize );
            nSplitPos = aPlaygroundSize.Height() - aSelectionBoxSize.Height() - m_aSplitter->GetSizePixel().Height();
            // still an invalid size?
            if ( nSplitPos == -1 || nSplitPos >= aPlaygroundSize.Height() )
                nSplitPos = sal_Int32(aPlaygroundSize.Height()*0.6);

            static_cast<OQueryController&>(getController()).setSplitPos(nSplitPos);
        }

        if ( !m_bInSplitHandler )
        {   // the resize is triggered by something else than the split handler
            // our main focus is to try to preserve the size of the selectionbrowse box
            Size aSelBoxSize = m_pSelectionBox->GetSizePixel();
            if ( aSelBoxSize.Height() )
            {
                // keep the size of the sel box constant
                nSplitPos = aPlaygroundSize.Height() - m_aSplitter->GetSizePixel().Height() - aSelBoxSize.Height();

                // and if the box is smaller than the optimal size, try to do something about it
                Size aSelBoxOptSize = m_pSelectionBox->CalcOptimalSize( aPlaygroundSize );
                if ( aSelBoxOptSize.Height() > aSelBoxSize.Height() )
                {
                    nSplitPos = aPlaygroundSize.Height() - m_aSplitter->GetSizePixel().Height() - aSelBoxOptSize.Height();
                }

                static_cast< OQueryController& >(getController()).setSplitPos( nSplitPos );
            }
        }
    }

    // normalize the split pos
    Point   aSplitPos( _rPlayground.Left(), nSplitPos );
    Size    aSplitSize( _rPlayground.GetSize().Width(), m_aSplitter->GetSizePixel().Height() );

    if( ( aSplitPos.Y() + aSplitSize.Height() ) > ( aPlaygroundSize.Height() ))
        aSplitPos.setY( aPlaygroundSize.Height() - aSplitSize.Height() );

    if( aSplitPos.Y() <= aPlaygroundPos.Y() )
        aSplitPos.setY( aPlaygroundPos.Y() + sal_Int32(aPlaygroundSize.Height() * 0.2) );

    // position the table
    Size aTableViewSize(aPlaygroundSize.Width(), aSplitPos.Y() - aPlaygroundPos.Y());
    m_pScrollWindow->SetPosSizePixel(aPlaygroundPos, aTableViewSize);

    // position the selection browse box
    Point aPos( aPlaygroundPos.X(), aSplitPos.Y() + aSplitSize.Height() );
    m_pSelectionBox->SetPosSizePixel( aPos, Size( aPlaygroundSize.Width(), aPlaygroundSize.Height() - aSplitSize.Height() - aTableViewSize.Height() ));

    // set the size of the splitter
    m_aSplitter->SetPosSizePixel( aSplitPos, aSplitSize );
    m_aSplitter->SetDragRectPixel( _rPlayground );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

void OQueryDesignView::setReadOnly(bool _bReadOnly)
{
    m_pSelectionBox->SetReadOnly(_bReadOnly);
}

void OQueryDesignView::clear()
{
    m_pSelectionBox->ClearAll(); // clear the whole selection
    m_pTableView->ClearAll();
}

void OQueryDesignView::copy()
{
    if( m_eChildFocus == SELECTION)
        m_pSelectionBox->copy();
}

bool OQueryDesignView::isCutAllowed()
{
    bool bAllowed = false;
    if ( SELECTION == m_eChildFocus )
        bAllowed = m_pSelectionBox->isCutAllowed();
    return bAllowed;
}

bool OQueryDesignView::isPasteAllowed()
{
    bool bAllowed = false;
    if ( SELECTION == m_eChildFocus )
        bAllowed = m_pSelectionBox->isPasteAllowed();
    return bAllowed;
}

bool OQueryDesignView::isCopyAllowed()
{
    bool bAllowed = false;
    if ( SELECTION == m_eChildFocus )
        bAllowed = m_pSelectionBox->isCopyAllowed();
    return bAllowed;
}

void OQueryDesignView::stopTimer()
{
    m_pSelectionBox->stopTimer();
}

void OQueryDesignView::startTimer()
{
    m_pSelectionBox->startTimer();
}

void OQueryDesignView::cut()
{
    if( m_eChildFocus == SELECTION)
    {
        m_pSelectionBox->cut();
        static_cast<OQueryController&>(getController()).setModified(true);
    }
}

void OQueryDesignView::paste()
{
    if( m_eChildFocus == SELECTION)
    {
        m_pSelectionBox->paste();
        static_cast<OQueryController&>(getController()).setModified(true);
    }
}

void OQueryDesignView::TableDeleted(const OUString& rAliasName)
{
    // message that the table was removed from the window
    m_pSelectionBox->DeleteFields( rAliasName );
    static_cast<OQueryController&>(getController()).InvalidateFeature(ID_BROWSER_ADDTABLE); // inform the view again
}

bool OQueryDesignView::HasFieldByAliasName(const OUString& rFieldName, OTableFieldDescRef const & rInfo)  const
{
    return m_pSelectionBox->HasFieldByAliasName( rFieldName, rInfo);
}

SqlParseError OQueryDesignView::InsertField( const OTableFieldDescRef& rInfo, bool bActivate)
{
    return m_pSelectionBox->InsertField( rInfo, BROWSER_INVALIDID, true/*bVis*/, bActivate ).is() ? eOk : eTooManyColumns;
}

sal_Int32 OQueryDesignView::getColWidth(sal_uInt16 _nColPos) const
{
    static sal_Int32 s_nDefaultWidth = GetTextWidth("0") * 15;
    sal_Int32 nWidth = static_cast<OQueryController&>(getController()).getColWidth(_nColPos);
    if ( !nWidth )
        nWidth = s_nDefaultWidth;
    return nWidth;
}

void OQueryDesignView::fillValidFields(const OUString& sAliasName, ComboBox* pFieldList)
{
    OSL_ENSURE(pFieldList != nullptr, "OQueryDesignView::FillValidFields : What the hell do you think I can do with a NULL-ptr ? This will crash !");
    pFieldList->Clear();

    bool bAllTables = sAliasName.isEmpty();

    OJoinTableView::OTableWindowMap& rTabWins = m_pTableView->GetTabWinMap();
    OUString strCurrentPrefix;
    std::vector< OUString> aFields;
    for (auto const& tabWin : rTabWins)
    {
        OQueryTableWindow* pCurrentWin = static_cast<OQueryTableWindow*>(tabWin.second.get());
        if (bAllTables || (pCurrentWin->GetAliasName() == sAliasName))
        {
            strCurrentPrefix = pCurrentWin->GetAliasName() + ".";

            pCurrentWin->EnumValidFields(aFields);

            for (auto const& field : aFields)
            {
                if (bAllTables || field.toChar() == '*')
                    pFieldList->InsertEntry(strCurrentPrefix + field);
                else
                    pFieldList->InsertEntry(field);
            }

            if (!bAllTables)
                // this means that I came into this block because the table name was exactly what I was looking for so I can end here
                // (and I prevent that fields get added more than once, if a table is repeated in TabWin)
                break;
        }
    }
}

bool OQueryDesignView::PreNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
    {
        if ( m_pSelectionBox && m_pSelectionBox->HasChildPathFocus() )
            m_eChildFocus = SELECTION;
        else
            m_eChildFocus = TABLEVIEW;
    }

    return OJoinDesignView::PreNotify(rNEvt);
}

// check if the statement is correct when not returning false
bool OQueryDesignView::checkStatement()
{
    bool bRet = true;
    if ( m_pSelectionBox )
        bRet = m_pSelectionBox->Save(); // an error occurred so we return no
    return bRet;
}

OUString OQueryDesignView::getStatement()
{
    OQueryController& rController = static_cast<OQueryController&>(getController());
    m_rController.clearError();
    // used for fields which aren't any longer in the statement
    OTableFields& rUnUsedFields = rController.getUnUsedFields();
    OTableFields().swap( rUnUsedFields );

    // create the select columns
    sal_uInt32 nFieldcount = 0;
    OTableFields& rFieldList = rController.getTableFieldDesc();
    for (auto const& field : rFieldList)
    {
        if (!field->GetField().isEmpty() && field->IsVisible() )
            ++nFieldcount;
        else if (!field->GetField().isEmpty()            &&
                !field->HasCriteria()                 &&
                field->isNoneFunction()               &&
                field->GetOrderDir() == ORDER_NONE    &&
                !field->IsGroupBy()                   &&
                field->GetFunction().isEmpty() )
            rUnUsedFields.push_back(field);
    }
    if ( !nFieldcount ) // no visible fields so return
    {
        rUnUsedFields = rFieldList;
        return OUString();
    }

    OQueryTableView::OTableWindowMap& rTabList   = m_pTableView->GetTabWinMap();
    sal_uInt32 nTabcount        = rTabList.size();

    OUString aFieldListStr(GenerateSelectList(this,rFieldList,nTabcount>1));
    if( aFieldListStr.isEmpty() )
        return OUString();

    // Exception handling, if no fields have been passed we should not
    // change the tab page
    // TabBarSelectHdl will query the SQL-OUString for STATEMENT_NOFIELDS
    // and trigger a error message
    // ----------------- Build table list ----------------------

    const auto& rConnList = m_pTableView->getTableConnections();
    Reference< XConnection> xConnection = rController.getConnection();
    OUString aTableListStr(GenerateFromClause(xConnection,&rTabList,rConnList));
    OSL_ENSURE(!aTableListStr.isEmpty(), "OQueryDesignView::getStatement() : unexpected : have Fields, but no Tables !");
    // if fields exist now, these only can be created by inserting from an already existing table; if on the other hand
    // a table is deleted, also the belonging fields will be deleted -> therefore it CANNOT occur that fields
    // exist but no tables exist (and aFieldListStr has its length, I secure this above)
    OUStringBuffer aHavingStr,aCriteriaListStr;

    // ----------------- build the criteria ----------------------
    if (!GenerateCriterias(this,aCriteriaListStr,aHavingStr,rFieldList, nTabcount > 1))
        return OUString();

    OUString aJoinCrit;
    GenerateInnerJoinCriterias(xConnection,aJoinCrit,rConnList);
    if(!aJoinCrit.isEmpty())
    {
        OUString aTmp = "( " + aJoinCrit + " )";
        if(!aCriteriaListStr.isEmpty())
        {
            aTmp += C_AND + aCriteriaListStr.makeStringAndClear();
        }
        aCriteriaListStr = aTmp;
    }
    // ----------------- construct statement  ----------------------
    OUStringBuffer aSqlCmd("SELECT ");
    if(rController.isDistinct())
        aSqlCmd.append(" DISTINCT ");
    aSqlCmd.append(aFieldListStr);
    aSqlCmd.append(" FROM ");
    aSqlCmd.append(aTableListStr);

    if (!aCriteriaListStr.isEmpty())
    {
        aSqlCmd.append(" WHERE ");
        aSqlCmd.append(aCriteriaListStr.makeStringAndClear());
    }
    Reference<XDatabaseMetaData> xMeta;
    if ( xConnection.is() )
        xMeta = xConnection->getMetaData();
    bool bUseAlias = nTabcount > 1;
    if ( xMeta.is() )
        bUseAlias = bUseAlias || !xMeta->supportsGroupByUnrelated();

    aSqlCmd.append(GenerateGroupBy(this,rFieldList,bUseAlias));
    // ----------------- construct GroupBy and attach ------------
    if(!aHavingStr.isEmpty())
    {
        aSqlCmd.append(" HAVING ");
        aSqlCmd.append(aHavingStr.makeStringAndClear());
    }
    // ----------------- construct sorting and attach ------------
    OUString sOrder;
    SqlParseError eErrorCode = eOk;
    if ( (eErrorCode = GenerateOrder(this,rFieldList,nTabcount > 1,sOrder)) == eOk)
        aSqlCmd.append(sOrder);
    else
    {
        if ( !m_rController.hasError() )
            m_rController.appendError( getParseErrorMessage( eErrorCode ) );

        m_rController.displayError();
    }
    // --------------------- Limit Clause -------------------
    {
        const sal_Int64 nLimit = rController.getLimit();
        if( nLimit != -1 )
        {
            aSqlCmd.append( " LIMIT " ).append( OUString::number(nLimit) );
        }
    }

    OUString sSQL = aSqlCmd.makeStringAndClear();
    if ( xConnection.is() )
    {
        ::connectivity::OSQLParser& rParser( rController.getParser() );
        OUString sErrorMessage;
        std::unique_ptr<OSQLParseNode> pParseNode( rParser.parseTree( sErrorMessage, sSQL, true ) );
        if (pParseNode)
        {
            OSQLParseNode* pNode = pParseNode->getChild(3)->getChild(1);
            if ( pNode->count() > 1 )
            {
                ::connectivity::OSQLParseNode * pCondition = pNode->getChild(1);
                if ( pCondition ) // no where clause
                {
                    OSQLParseNode::compress(pCondition);
                    OUString sTemp;
                    pParseNode->parseNodeToStr(sTemp,xConnection);
                    sSQL = sTemp;
                }
            }
        }
    }
    return sSQL;
}

void OQueryDesignView::setSlotEnabled(sal_Int32 _nSlotId, bool _bEnable)
{
    sal_uInt16 nRow;
    switch (_nSlotId)
    {
        case SID_QUERY_VIEW_FUNCTIONS:
            nRow = BROW_FUNCTION_ROW;
            break;
        case SID_QUERY_VIEW_TABLES:
            nRow = BROW_TABLE_ROW;
            break;
        case SID_QUERY_VIEW_ALIASES:
            nRow = BROW_COLUMNALIAS_ROW;
            break;
        default:
            // ????????????
            nRow = 0;
            break;
    }
    m_pSelectionBox->SetRowVisible(nRow,_bEnable);
    m_pSelectionBox->Invalidate();
}

bool OQueryDesignView::isSlotEnabled(sal_Int32 _nSlotId)
{
    sal_uInt16 nRow;
    switch (_nSlotId)
    {
        case SID_QUERY_VIEW_FUNCTIONS:
            nRow = BROW_FUNCTION_ROW;
            break;
        case SID_QUERY_VIEW_TABLES:
            nRow = BROW_TABLE_ROW;
            break;
        case SID_QUERY_VIEW_ALIASES:
            nRow = BROW_COLUMNALIAS_ROW;
            break;
        default:
            // ?????????
            nRow = 0;
            break;
    }
    return m_pSelectionBox->IsRowVisible(nRow);
}

void OQueryDesignView::SaveUIConfig()
{
    OQueryController& rCtrl = static_cast<OQueryController&>(getController());
    rCtrl.SaveTabWinsPosSize( &m_pTableView->GetTabWinMap(), m_pScrollWindow->GetHScrollBar().GetThumbPos(), m_pScrollWindow->GetVScrollBar().GetThumbPos() );
    rCtrl.setVisibleRows( m_pSelectionBox->GetNoneVisibleRows() );
    if ( m_aSplitter->GetSplitPosPixel() != 0 )
        rCtrl.setSplitPos( m_aSplitter->GetSplitPosPixel() );
}

std::unique_ptr<OSQLParseNode> OQueryDesignView::getPredicateTreeFromEntry(const OTableFieldDescRef& pEntry,
                                                           const OUString& _sCriteria,
                                                           OUString& _rsErrorMessage,
                                                           Reference<XPropertySet>& _rxColumn) const
{
    OSL_ENSURE(pEntry.is(),"Entry is null!");
    if(!pEntry.is())
        return nullptr;
    Reference< XConnection> xConnection = static_cast<OQueryController&>(getController()).getConnection();
    if(!xConnection.is())
        return nullptr;

    ::connectivity::OSQLParser& rParser( static_cast<OQueryController&>(getController()).getParser() );
    OQueryTableWindow* pWin = static_cast<OQueryTableWindow*>(pEntry->GetTabWindow());

    // special handling for functions
    if ( pEntry->GetFunctionType() & (FKT_OTHER | FKT_AGGREGATE | FKT_NUMERIC) )
    {
        // we have a function here so we have to distinguish the type of return vOUalue
        OUString sFunction;
        if ( pEntry->isNumericOrAggregateFunction() )
            sFunction = pEntry->GetFunction().getToken(0, '(');

        if ( sFunction.isEmpty() )
            sFunction = pEntry->GetField().getToken(0, '(');

        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionReturnType(sFunction,&rParser.getContext());
        if ( nType == DataType::OTHER || (sFunction.isEmpty() && pEntry->isNumericOrAggregateFunction()) )
        {
            // first try the international version
            OUString sSql;
            sSql += "SELECT * FROM x WHERE " + pEntry->GetField() + _sCriteria;
            std::unique_ptr<OSQLParseNode> pParseNode( rParser.parseTree( _rsErrorMessage, sSql, true ) );
            nType = DataType::DOUBLE;
            if (pParseNode)
            {
                OSQLParseNode* pColumnRef = pParseNode->getByRule(OSQLParseNode::column_ref);
                if ( pColumnRef )
                {
                    OTableFieldDescRef aField = new OTableFieldDesc();
                    if ( eOk == FillDragInfo(this,pColumnRef,aField) )
                    {
                        nType = aField->GetDataType();
                    }
                }
            }
        }

        Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
        parse::OParseColumn* pColumn = new parse::OParseColumn( pEntry->GetField(),
                                                                OUString(),
                                                                OUString(),
                                                                OUString(),
                                                                ColumnValue::NULLABLE_UNKNOWN,
                                                                0,
                                                                0,
                                                                nType,
                                                                false,
                                                                false,
                                                                xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),
                                                                OUString(),
                                                                OUString(),
                                                                OUString());
        _rxColumn = pColumn;
        pColumn->setFunction(true);
        pColumn->setRealName(pEntry->GetField());
    }
    else
    {
        if (pWin)
        {
            Reference<XNameAccess> xColumns = pWin->GetOriginalColumns();
            if (xColumns.is() && xColumns->hasByName(pEntry->GetField()))
                xColumns->getByName(pEntry->GetField()) >>= _rxColumn;
        }
    }

    // _rxColumn, if it is a "lookup" column, not a computed column,
    // is guaranteed to be the column taken from the *source* of the column,
    // that is either a table or another query.
    // _rxColumn is *not* taken from the columns of the query we are constructing
    // (and rightfully so, since it may not be part of these columns; SELECT A FROM t WHERE B = foo)
    // If it is a computed column, we just constructed it above, with same Name and RealName.
    // In all cases, we should use the "external" name of the column, not the "RealName";
    // the latter is the name that the column had in the source of the source query.
    // An example: we are designing "SELECT A, B FROM q WHERE C='foo'"
    // q itself is query "SELECT aye AS A, bee as B, cee as C FROM t"
    // We are currently treating the entry "C='foo'"
    // Then _rxColumn has Name "C" and RealName "cee". We should *obviously* use "C", not "cee".
    std::unique_ptr<OSQLParseNode> pParseNode = rParser.predicateTree(  _rsErrorMessage,
                                                        _sCriteria,
                                                        static_cast<OQueryController&>(getController()).getNumberFormatter(),
                                                        _rxColumn,
                                                        false);
    return pParseNode;
}

void OQueryDesignView::GetFocus()
{
    OJoinDesignView::GetFocus();
    if ( m_pSelectionBox && !m_pSelectionBox->HasChildPathFocus() )
    {
        // first we have to deactivate the current cell to refill when necessary
        m_pSelectionBox->DeactivateCell();
        m_pSelectionBox->ActivateCell(m_pSelectionBox->GetCurRow(), m_pSelectionBox->GetCurColumnId());
        m_pSelectionBox->GrabFocus();
    }
}

void OQueryDesignView::reset()
{
    m_pTableView->ClearAll();
    m_pTableView->ReSync();
}

void OQueryDesignView::setNoneVisbleRow(sal_Int32 _nRows)
{
    m_pSelectionBox->SetNoneVisbleRow(_nRows);
}

void OQueryDesignView::initByFieldDescriptions( const Sequence< PropertyValue >& i_rFieldDescriptions )
{
    OQueryController& rController = static_cast< OQueryController& >( getController() );

    m_pSelectionBox->PreFill();
    m_pSelectionBox->SetReadOnly( rController.isReadOnly() );
    m_pSelectionBox->Fill();

    for ( auto const & field : i_rFieldDescriptions )
    {
        ::rtl::Reference< OTableFieldDesc > pField( new OTableFieldDesc() );
        pField->Load( field, true );
        InsertField( pField, false );
    }

    rController.ClearUndoManager();
    m_pSelectionBox->Invalidate();
}

bool OQueryDesignView::initByParseIterator( ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    SqlParseError eErrorCode = eNativeMode;
    m_rController.clearError();

    try
    {
        eErrorCode = InitFromParseNodeImpl( this, m_pSelectionBox );

        if ( eErrorCode != eOk )
        {
            if ( !m_rController.hasError() )
                m_rController.appendError( getParseErrorMessage( eErrorCode ) );

            if ( _pErrorInfo )
            {
                *_pErrorInfo = m_rController.getError();
            }
            else
            {
                m_rController.displayError();
            }
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return eErrorCode == eOk;
}

// Utility function for fillFunctionInfo
namespace {
    sal_Int32 char_datatype(const::connectivity::OSQLParseNode* pDataType, const unsigned int offset) {
        int cnt = pDataType->count() - offset;
        if ( cnt < 0 )
        {
            OSL_FAIL("internal error in decoding character datatype specification");
            return DataType::VARCHAR;
        }
        else if ( cnt == 0 )
        {
            if ( offset == 0 )
            {
                // The datatype is the node itself
                if ( SQL_ISTOKENOR2 (pDataType, CHARACTER, CHAR) )
                    return DataType::CHAR;
                else if ( SQL_ISTOKEN (pDataType, VARCHAR) )
                    return DataType::VARCHAR;
                else if ( SQL_ISTOKEN (pDataType, CLOB) )
                    return DataType::CLOB;
                else
                {
                    OSL_FAIL("unknown/unexpected token in decoding character datatype specification");
                    return DataType::VARCHAR;
                }
            }
            else
            {
                // No child left to read!
                OSL_FAIL("incomplete datatype in decoding character datatype specification");
                return DataType::VARCHAR;
            }
        }

        if ( SQL_ISTOKEN(pDataType->getChild(offset), NATIONAL) )
            return char_datatype(pDataType, offset+1);
        else if ( SQL_ISTOKENOR3(pDataType->getChild(offset), CHARACTER, CHAR, NCHAR) )
        {
            if ( cnt > 2 && SQL_ISTOKEN(pDataType->getChild(offset+1), LARGE) && SQL_ISTOKEN(pDataType->getChild(offset+2), OBJECT) )
                return DataType::CLOB;
            else if ( cnt > 1 && SQL_ISTOKEN(pDataType->getChild(offset+1), VARYING) )
                return DataType::VARCHAR;
            else
                return DataType::CHAR;
        }
        else if ( SQL_ISTOKEN (pDataType->getChild(offset), VARCHAR) )
            return DataType::VARCHAR;
        else if ( SQL_ISTOKENOR2 (pDataType->getChild(offset), CLOB, NCLOB) )
            return DataType::CLOB;

        OSL_FAIL("unrecognised character datatype");
        return DataType::VARCHAR;
    }
}

// Try to guess the type of an expression in simple cases.
// Originally meant to be called only on a function call (hence the misnomer),
// but now tries to do the best it can also in other cases.
// Don't completely rely on fillFunctionInfo,
// it won't look at the function's arguments to find the return type
// (in particular, in the case of general_set_fct,
//  the return type is the type of the argument;
//  if that is (as is typical) a column reference,
//  it is the type of the column).
// TODO: There is similar "guess the expression's type" code in several places:
//       SelectionBrowseBox.cxx: OSelectionBrowseBox::saveField
//       QueryDesignView.cxx: InstallFields, GetOrderCriteria, GetGroupCriteria
//       If possible, they should be factorised into this function
//       (which should then be renamed...)

void OQueryDesignView::fillFunctionInfo(  const ::connectivity::OSQLParseNode* pNode
                                        ,const OUString& sFunctionTerm
                                        ,OTableFieldDescRef& aInfo)
{
    // get the type of the expression, as far as easily possible
    OQueryController& rController = static_cast<OQueryController&>(getController());
    sal_Int32 nDataType = DataType::DOUBLE;
    switch(pNode->getNodeType())
    {
    case SQLNodeType::Concat:
    case SQLNodeType::String:
        nDataType = DataType::VARCHAR;
        break;
    case SQLNodeType::IntNum:
        nDataType = DataType::INTEGER;
        break;
    case SQLNodeType::ApproxNum:
        nDataType = DataType::DOUBLE;
        break;
    case SQLNodeType::AccessDate:
        nDataType = DataType::TIMESTAMP;
        break;
    case SQLNodeType::Equal:
    case SQLNodeType::Less:
    case SQLNodeType::Great:
    case SQLNodeType::LessEq:
    case SQLNodeType::GreatEq:
    case SQLNodeType::NotEqual:
        nDataType = DataType::BOOLEAN;
        break;
    case SQLNodeType::Name:
    case SQLNodeType::ListRule:
    case SQLNodeType::CommaListRule:
    case SQLNodeType::Keyword:
    case SQLNodeType::Punctuation:
        OSL_FAIL("Unexpected SQL Node Type");
        break;
    case SQLNodeType::Rule:
        switch(pNode->getKnownRuleID())
        {
        case OSQLParseNode::select_statement:
        case OSQLParseNode::table_exp:
        case OSQLParseNode::table_ref_commalist:
        case OSQLParseNode::table_ref:
        case OSQLParseNode::catalog_name:
        case OSQLParseNode::schema_name:
        case OSQLParseNode::table_name:
        case OSQLParseNode::opt_column_commalist:
        case OSQLParseNode::column_commalist:
        case OSQLParseNode::column_ref_commalist:
        case OSQLParseNode::column_ref:
        case OSQLParseNode::opt_order_by_clause:
        case OSQLParseNode::ordering_spec_commalist:
        case OSQLParseNode::ordering_spec:
        case OSQLParseNode::opt_asc_desc:
        case OSQLParseNode::where_clause:
        case OSQLParseNode::opt_where_clause:
        case OSQLParseNode::opt_escape:
        case OSQLParseNode::scalar_exp_commalist:
        case OSQLParseNode::scalar_exp: // Seems to never be generated?
        case OSQLParseNode::parameter_ref:
        case OSQLParseNode::parameter:
        case OSQLParseNode::range_variable:
        case OSQLParseNode::delete_statement_positioned:
        case OSQLParseNode::delete_statement_searched:
        case OSQLParseNode::update_statement_positioned:
        case OSQLParseNode::update_statement_searched:
        case OSQLParseNode::assignment_commalist:
        case OSQLParseNode::assignment:
        case OSQLParseNode::insert_statement:
        case OSQLParseNode::insert_atom_commalist:
        case OSQLParseNode::insert_atom:
        case OSQLParseNode::from_clause:
        case OSQLParseNode::qualified_join:
        case OSQLParseNode::cross_union:
        case OSQLParseNode::select_sublist:
        case OSQLParseNode::join_type:
        case OSQLParseNode::named_columns_join:
        case OSQLParseNode::joined_table:
        case OSQLParseNode::sql_not:
        case OSQLParseNode::manipulative_statement:
        case OSQLParseNode::value_exp_commalist:
        case OSQLParseNode::union_statement:
        case OSQLParseNode::outer_join_type:
        case OSQLParseNode::selection:
        case OSQLParseNode::base_table_def:
        case OSQLParseNode::base_table_element_commalist:
        case OSQLParseNode::data_type:
        case OSQLParseNode::column_def:
        case OSQLParseNode::table_node:
        case OSQLParseNode::as_clause:
        case OSQLParseNode::opt_as:
        case OSQLParseNode::op_column_commalist:
        case OSQLParseNode::table_primary_as_range_column:
        case OSQLParseNode::character_string_type:
        case OSQLParseNode::comparison:
            OSL_FAIL("Unexpected SQL RuleID");
            break;
        case OSQLParseNode::column:
        case OSQLParseNode::column_val:
            OSL_FAIL("Cannot guess column type");
            break;
        case OSQLParseNode::values_or_query_spec:
            OSL_FAIL("Cannot guess VALUES type");
            break;
        case OSQLParseNode::derived_column:
            OSL_FAIL("Cannot guess computed column type");
            break;
        case OSQLParseNode::subquery:
            OSL_FAIL("Cannot guess subquery return type");
            break;
        case OSQLParseNode::search_condition:
        case OSQLParseNode::comparison_predicate:
        case OSQLParseNode::between_predicate:
        case OSQLParseNode::like_predicate:
        case OSQLParseNode::test_for_null:
        case OSQLParseNode::boolean_term:
        case OSQLParseNode::boolean_primary:
        case OSQLParseNode::in_predicate:
        case OSQLParseNode::existence_test:
        case OSQLParseNode::unique_test:
        case OSQLParseNode::all_or_any_predicate:
        case OSQLParseNode::join_condition:
        case OSQLParseNode::boolean_factor:
        case OSQLParseNode::comparison_predicate_part_2:
        case OSQLParseNode::parenthesized_boolean_value_expression:
        case OSQLParseNode::other_like_predicate_part_2:
        case OSQLParseNode::between_predicate_part_2:
            nDataType = DataType::BOOLEAN;
            break;
        case OSQLParseNode::num_value_exp:
        case OSQLParseNode::extract_exp:
        case OSQLParseNode::term:
        case OSQLParseNode::factor:
            // Might by an integer or a float; take the most generic
            nDataType = DataType::DOUBLE;
            break;
        case OSQLParseNode::value_exp_primary:
        case OSQLParseNode::value_exp:
        case OSQLParseNode::odbc_call_spec:
            // Really, we don't know. Let the default.
            break;
        case OSQLParseNode::position_exp:
        case OSQLParseNode::length_exp:
            nDataType = DataType::INTEGER;
            break;
        case OSQLParseNode::char_value_exp:
        case OSQLParseNode::char_value_fct:
        case OSQLParseNode::fold:
        case OSQLParseNode::char_substring_fct:
        case OSQLParseNode::char_factor:
        case OSQLParseNode::concatenation:
            nDataType = DataType::VARCHAR;
            break;
        case OSQLParseNode::datetime_primary:
            nDataType = DataType::TIMESTAMP;
            break;
        case OSQLParseNode::bit_value_fct:
            nDataType = DataType::BINARY;
            break;
        case OSQLParseNode::general_set_fct: // May depend on argument; ignore that for now
        case OSQLParseNode::set_fct_spec:
        {
            if (pNode->count() == 0)
            {
                // This is not a function call, no sense to continue with a function return type lookup
                OSL_FAIL("Got leaf SQL node where non-leaf expected");
                break;
            }
            const OSQLParseNode* pFunctionName = pNode->getChild(0);
            if ( SQL_ISPUNCTUATION(pFunctionName,"{") )
            {
                if ( pNode->count() == 3 )
                    return fillFunctionInfo( pNode->getChild(1), sFunctionTerm, aInfo );
                else
                    OSL_FAIL("ODBC escape not in recognised form");
                break;
            }
            else
            {
                if ( SQL_ISRULEOR2(pNode,length_exp,char_value_fct) )
                    pFunctionName = pFunctionName->getChild(0);

                OUString sFunctionName = pFunctionName->getTokenValue();
                if ( sFunctionName.isEmpty() )
                    sFunctionName = OStringToOUString(OSQLParser::TokenIDToStr(pFunctionName->getTokenID()),RTL_TEXTENCODING_UTF8);

                nDataType = OSQLParser::getFunctionReturnType(
                    sFunctionName
                    ,&rController.getParser().getContext());
            }
            break;
        }
        case OSQLParseNode::odbc_fct_spec:
        {
            if (pNode->count() != 2)
            {
                OSL_FAIL("interior of ODBC escape not in recognised shape");
                break;
            }

            const OSQLParseNode* const pEscapeType = pNode->getChild(0);
            if (SQL_ISTOKEN(pEscapeType, TS))
                nDataType = DataType::TIMESTAMP;
            else if (SQL_ISTOKEN(pEscapeType, D))
                nDataType = DataType::DATE;
            else if (SQL_ISTOKEN(pEscapeType, T))
                nDataType = DataType::TIME;
            else if (SQL_ISTOKEN(pEscapeType, FN))
                return fillFunctionInfo( pNode->getChild(1), sFunctionTerm, aInfo );
            else
                OSL_FAIL("Unknown ODBC escape");
            break;
        }
        case OSQLParseNode::cast_spec:
        {
            if ( pNode->count() != 6 || !SQL_ISTOKEN(pNode->getChild(3), AS) )
            {
                OSL_FAIL("CAST not in recognised shape");
                break;
            }
            const OSQLParseNode *pCastTarget = pNode->getChild(4);
            if ( SQL_ISTOKENOR2(pCastTarget, INTEGER, INT) )
                nDataType = DataType::INTEGER;
            else if ( SQL_ISTOKEN(pCastTarget, SMALLINT) )
                nDataType = DataType::SMALLINT;
            else if ( SQL_ISTOKEN(pCastTarget, BIGINT) )
                nDataType = DataType::BIGINT;
            else if ( SQL_ISTOKEN(pCastTarget, FLOAT) )
                nDataType = DataType::FLOAT;
            else if ( SQL_ISTOKEN(pCastTarget, REAL) )
                nDataType = DataType::REAL;
            else if ( SQL_ISTOKEN(pCastTarget, DOUBLE) )
                nDataType = DataType::DOUBLE;
            else if ( SQL_ISTOKEN(pCastTarget, BOOLEAN) )
                nDataType = DataType::BOOLEAN;
            else if ( SQL_ISTOKEN(pCastTarget, DATE) )
                nDataType = DataType::DATE;
            else if ( pCastTarget->count() > 0 )
            {
                const OSQLParseNode *pDataType = pCastTarget->getChild(0);
                while (pDataType->count() > 0)
                {
                    pCastTarget = pDataType;
                    pDataType = pDataType->getChild(0);
                }
                if ( SQL_ISTOKEN (pDataType, TIME) )
                    nDataType = DataType::TIME;
                else if ( SQL_ISTOKEN (pDataType, TIMESTAMP) )
                    nDataType = DataType::TIMESTAMP;
                else if ( SQL_ISTOKENOR3 (pDataType, CHARACTER, CHAR, NCHAR) )
                    nDataType = char_datatype(pCastTarget, 0);
                else if ( SQL_ISTOKEN (pDataType, VARCHAR) )
                    nDataType = DataType::VARCHAR;
                else if ( SQL_ISTOKEN (pDataType, CLOB) )
                    nDataType = DataType::CLOB;
                else if ( SQL_ISTOKEN (pDataType, NATIONAL) )
                    nDataType = char_datatype(pCastTarget, 1);
                else if ( SQL_ISTOKEN (pDataType, BINARY) )
                {
                    if ( pCastTarget->count() > 2 && SQL_ISTOKEN(pCastTarget->getChild(1), LARGE) && SQL_ISTOKEN(pCastTarget->getChild(2), OBJECT) )
                        nDataType = DataType::BLOB;
                    else if ( pCastTarget->count() > 1 && SQL_ISTOKEN(pCastTarget->getChild(1), VARYING) )
                        nDataType = DataType::VARBINARY;
                    else
                        nDataType = DataType::BINARY;
                }
                else if ( SQL_ISTOKEN (pDataType, VARBINARY) )
                    nDataType = DataType::VARBINARY;
                else if ( SQL_ISTOKEN (pDataType, BLOB) )
                    nDataType = DataType::BLOB;
                else if ( SQL_ISTOKEN (pDataType, NUMERIC) )
                    nDataType = DataType::NUMERIC;
                else if ( SQL_ISTOKENOR2 (pDataType, DECIMAL, DEC) )
                    nDataType = DataType::DECIMAL;
                else if ( SQL_ISTOKEN (pDataType, FLOAT) )
                    nDataType = DataType::FLOAT;
                else if ( SQL_ISTOKEN (pDataType, DOUBLE) )
                    nDataType = DataType::DOUBLE;
                else if ( SQL_ISTOKEN (pDataType, INTERVAL) )
                    // Not in DataType published constant (because not in JDBC...)
                    nDataType = DataType::VARCHAR;
                else
                    OSL_FAIL("Failed to decode CAST target");
            }
            else
                OSL_FAIL("Could not decipher CAST target");
            break;
        }
        default:
            OSL_FAIL("Unknown SQL RuleID");
            break;
        }
        break;
    default:
        OSL_FAIL("Unknown SQL Node Type");
        break;
    }

    aInfo->SetDataType(nDataType);
    aInfo->SetFieldType(TAB_NORMAL_FIELD);
    aInfo->SetField(sFunctionTerm);
    aInfo->SetTabWindow(nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
