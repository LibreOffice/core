/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#include "QTableWindow.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _UNDO_HXX
#include <svl/undo.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef DBAUI_QYDLGTAB_HXX
#include "adtabdlg.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX
#include "SelectionBrowseBox.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XLOCALEDATA_HPP_
#include <com/sun/star/i18n/XLocaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CONNECTIVITY_PCOLUMN_HXX_
#include <connectivity/PColumn.hxx>
#endif
#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#include "QTableConnection.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINE_HXX
#include "ConnectionLine.hxx"
#endif
#ifndef DBAUI_CONNECTIONLINEDATA_HXX
#include "ConnectionLineData.hxx"
#endif
#ifndef DBAUI_QTABLECONNECTIONDATA_HXX
#include "QTableConnectionData.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#include "querycontainerwindow.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <unotools/syslocale.hxx>
#endif

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

#define SQL_ISRULEOR2(pParseNode, e1,e2)    ((pParseNode)->isRule() && (\
                                            (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e1) || \
                                            (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::e2)))

// here we define our functions used in the anonymous namespace to get our header file smaller
// please look at the book LargeScale C++ to know why
namespace
{
    static const ::rtl::OUString C_AND = ::rtl::OUString::createFromAscii(" AND ");
    static const ::rtl::OUString C_OR  = ::rtl::OUString::createFromAscii(" OR ");

    // forward declarations
    sal_Bool InsertJoin(    const OQueryDesignView* _pView,
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

    SqlParseError AddFunctionCondition(OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const ::connectivity::OSQLParseNode * pCondition,
                                    const sal_uInt16 nLevel,
                                    sal_Bool bHaving,
                                    bool _bAddOrOnOneLine);

    //------------------------------------------------------------------------------
    ::rtl::OUString quoteTableAlias(sal_Bool _bQuote, const ::rtl::OUString& _sAliasName, const ::rtl::OUString& _sQuote)
    {
        ::rtl::OUString sRet;
        if ( _bQuote && _sAliasName.getLength() )
        {
            sRet = ::dbtools::quoteName(_sQuote,_sAliasName);
            const static ::rtl::OUString sTableSeparater('.');
            sRet += sTableSeparater;
        }
        return sRet;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString getTableRange(const OQueryDesignView* _pView,const ::connectivity::OSQLParseNode* _pTableRef)
    {
        Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();
        ::rtl::OUString sTableRange;
        if ( _pTableRef )
        {
            sTableRange = ::connectivity::OSQLParseNode::getTableRange(_pTableRef);
            if ( !sTableRange.getLength() )
                _pTableRef->parseNodeToStr(sTableRange,xConnection,NULL,sal_False,sal_False);
        }
        return sTableRange;
    }
    //------------------------------------------------------------------------------
    void insertConnection(const OQueryDesignView* _pView,const EJoinType& _eJoinType,OTableFieldDescRef _aDragLeft,OTableFieldDescRef _aDragRight,bool _bNatural = false)
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
                    Sequence< ::rtl::OUString> aSeq = aInfoData->getReferencingTable()->getColumns()->getElementNames();
                    const ::rtl::OUString* pIter = aSeq.getConstArray();
                    const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
                    for(;pIter != pEnd;++pIter)
                    {
                        if ( xReferencedTableColumns->hasByName(*pIter) )
                            aInfoData->AppendConnLine(*pIter,*pIter);
                    }
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }

            OQueryTableConnection aInfo(pTableView, aInfoData);
            // da ein OQueryTableConnection-Objekt nie den Besitz der uebergebenen Daten uebernimmt, sondern sich nur den Zeiger merkt,
            // ist dieser Zeiger auf eine lokale Variable hier unkritisch, denn aInfoData und aInfo haben die selbe Lebensdauer
            pTableView->NotifyTabConnection( aInfo );
        }
        else
        {
            ::rtl::OUString aSourceFieldName(_aDragLeft->GetField());
            ::rtl::OUString aDestFieldName(_aDragRight->GetField());
            // the connection could point on the other side
            if(pConn->GetSourceWin() == _aDragRight->GetTabWindow())
            {
                ::rtl::OUString aTmp(aSourceFieldName);
                aSourceFieldName = aDestFieldName;
                aDestFieldName = aTmp;
            }
            pConn->GetData()->AppendConnLine( aSourceFieldName,aDestFieldName);
            pConn->UpdateLineList();
            // Modified-Flag
            //  SetModified();
            // und neu zeichnen
            pConn->RecalcLines();
                // fuer das unten folgende Invalidate muss ich dieser neuen Connection erst mal die Moeglichkeit geben,
                // ihr BoundingRect zu ermitteln
            pConn->InvalidateConnection();
        }
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString ParseCondition( OQueryController& rController
                                    ,const ::connectivity::OSQLParseNode* pCondition
                                    ,const ::rtl::OUString _sDecimal
                                    ,const ::com::sun::star::lang::Locale& _rLocale
                                    ,sal_uInt32 _nStartIndex)
    {
        ::rtl::OUString aCondition;
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
    //------------------------------------------------------------------------------
    SqlParseError FillOuterJoins(OQueryDesignView* _pView,
                                const ::connectivity::OSQLParseNode* pTableRefList)
    {
        SqlParseError eErrorCode = eOk;
        sal_uInt32 nCount = pTableRefList->count();
        sal_Bool bError = sal_False;
        for (sal_uInt32 i=0; !bError && i < nCount; ++i)
        {
            const ::connectivity::OSQLParseNode* pParseNode = pTableRefList->getChild(i);
            const ::connectivity::OSQLParseNode* pJoinNode = NULL;

            if ( SQL_ISRULE( pParseNode, qualified_join ) || SQL_ISRULE( pParseNode, joined_table ) || SQL_ISRULE( pParseNode, cross_union ) )
                pJoinNode = pParseNode;
            else if(    SQL_ISRULE(pParseNode,table_ref)
                    &&  pParseNode->count() == 4 ) // '{' SQL_TOKEN_OJ joined_table '}'
                pJoinNode = pParseNode->getChild(2);

            if ( pJoinNode )
            {
                if ( !InsertJoin(_pView,pJoinNode) )
                    bError = sal_True;
            }
        }
        // check if error occurred
        if ( bError )
            eErrorCode = eIllegalJoin;

        return eErrorCode;
    }
    // -----------------------------------------------------------------------------

    /** FillDragInfo fills the field description out of the table
    */
    //------------------------------------------------------------------------------
    SqlParseError FillDragInfo( const OQueryDesignView* _pView,
                            const ::connectivity::OSQLParseNode* pColumnRef,
                            OTableFieldDescRef& _rDragInfo)
    {
        SqlParseError eErrorCode = eOk;

        sal_Bool bErg = sal_False;

        ::rtl::OUString aTableRange,aColumnName;
        sal_uInt16 nCntAccount;
        ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController&>(_pView->getController()).getParseIterator();
        rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

        if ( aTableRange.getLength() )
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
            String sError(ModuleRes(STR_QRY_COLUMN_NOT_FOUND));
            sError.SearchAndReplaceAscii("$name$",aColumnName);
            _pView->getController().appendError( sError );

            try
            {
                Reference<XDatabaseMetaData> xMeta = _pView->getController().getConnection()->getMetaData();
                if ( xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers() )
                    _pView->getController().appendError( String( ModuleRes( STR_QRY_CHECK_CASESENSITIVE ) ) );
            }
            catch(Exception&)
            {
            }
        }

        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoinCriteria(  const Reference< XConnection>& _xConnection,
                                        OConnectionLineDataVec* pLineDataList,
                                        OQueryTableConnectionData* pData)
    {
        ::rtl::OUStringBuffer aCondition;
        if ( _xConnection.is() )
        {
            OConnectionLineDataVec::iterator aIter = pLineDataList->begin();
            OConnectionLineDataVec::iterator aEnd = pLineDataList->end();
            try
            {
                const Reference< XDatabaseMetaData >  xMetaData = _xConnection->getMetaData();
                const ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
                const ::rtl::OUString sEqual(RTL_CONSTASCII_USTRINGPARAM(" = "));

                for(;aIter != aEnd;++aIter)
                {
                    OConnectionLineDataRef pLineData = *aIter;
                    if(aCondition.getLength())
                        aCondition.append(C_AND);
                    aCondition.append(quoteTableAlias(sal_True,pData->GetAliasName(JTCS_FROM),aQuote));
                    aCondition.append(::dbtools::quoteName(aQuote, pLineData->GetFieldName(JTCS_FROM) ));
                    aCondition.append(sEqual);
                    aCondition.append(quoteTableAlias(sal_True,pData->GetAliasName(JTCS_TO),aQuote));
                    aCondition.append(::dbtools::quoteName(aQuote, pLineData->GetFieldName(JTCS_TO) ));
                }
            }
            catch(SQLException&)
            {
                OSL_ASSERT(!"Failure while building Join criteria!");
            }
        }

        return aCondition.makeStringAndClear();
    }
    //------------------------------------------------------------------------------
    /** JoinCycle looks for a join cycle and append it to the string
        @param  _xConnection    the connection
        @param  _pEntryConn     the table connection which holds the data
        @param  _pEntryTabTo    the corresponding table window
        @param  _rJoin          the String which will contain the resulting string
    */
    void JoinCycle( const Reference< XConnection>& _xConnection,
                    OQueryTableConnection* _pEntryConn,
                    const OQueryTableWindow* _pEntryTabTo,
                    ::rtl::OUString& _rJoin )
    {
        OSL_ENSURE(_pEntryConn,"TableConnection can not be null!");

        OQueryTableConnectionData* pData = static_cast< OQueryTableConnectionData*>(_pEntryConn->GetData().get());
        if ( pData->GetJoinType() != INNER_JOIN && _pEntryTabTo->ExistsAVisitedConn() )
        {
            sal_Bool bBrace = sal_False;
            if(_rJoin.getLength() && _rJoin.lastIndexOf(')') == (_rJoin.getLength()-1))
            {
                bBrace = sal_True;
                _rJoin = _rJoin.replaceAt(_rJoin.getLength()-1,1,::rtl::OUString(' '));
            }
            (_rJoin += C_AND) += BuildJoinCriteria(_xConnection,pData->GetConnLineDataList(),pData);
            if(bBrace)
                _rJoin += ::rtl::OUString(')');
            _pEntryConn->SetVisited(sal_True);
        }
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildTable( const Reference< XConnection>& _xConnection,
                                const OQueryTableWindow* pEntryTab,
                                bool _bForce = false
                                )
    {
        ::rtl::OUString aDBName(pEntryTab->GetComposedName());

        //  Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();
        if( _xConnection.is() )
        {
            try
            {
                Reference< XDatabaseMetaData >  xMetaData = _xConnection->getMetaData();

                ::rtl::OUString sCatalog, sSchema, sTable;
                ::dbtools::qualifiedNameComponents( xMetaData, aDBName, sCatalog, sSchema, sTable, ::dbtools::eInDataManipulation );
                ::rtl::OUString aTableListStr = ::dbtools::composeTableNameForSelect( _xConnection, sCatalog, sSchema, sTable );

                ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
                if ( _bForce || isAppendTableAliasEnabled( _xConnection ) || pEntryTab->GetAliasName() != aDBName )
                {
                    aTableListStr += ::rtl::OUString::createFromAscii(" ");
                    if ( generateAsBeforeTableAlias( _xConnection ) )
                        aTableListStr += ::rtl::OUString::createFromAscii("AS ");
                    aTableListStr += ::dbtools::quoteName( aQuote, pEntryTab->GetAliasName() );
                }
                aDBName = aTableListStr;
            }
            catch(const SQLException&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return aDBName;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                const ::rtl::OUString& rLh,
                                const ::rtl::OUString& rRh,
                                OQueryTableConnectionData* pData)
    {

        String aErg(rLh);
        if ( pData->isNatural() && pData->GetJoinType() != CROSS_JOIN )
            aErg.AppendAscii(" NATURAL ");
        switch(pData->GetJoinType())
        {
            case LEFT_JOIN:
                aErg.AppendAscii(" LEFT OUTER ");
                break;
            case RIGHT_JOIN:
                aErg.AppendAscii(" RIGHT OUTER ");
                break;
            case CROSS_JOIN:
                OSL_ENSURE(!pData->isNatural(),"OQueryDesignView::BuildJoin: This should not happen!");
                aErg.AppendAscii(" CROSS ");
                break;
            case INNER_JOIN:
                OSL_ENSURE(pData->isNatural(),"OQueryDesignView::BuildJoin: This should not happen!");
                aErg.AppendAscii(" INNER ");
                break;
            default:
                aErg.AppendAscii(" FULL OUTER ");
                break;
        }
        aErg.AppendAscii("JOIN ");
        aErg += String(rRh);
        if ( CROSS_JOIN != pData->GetJoinType() && !pData->isNatural() )
        {
            aErg.AppendAscii(" ON ");
            aErg += String(BuildJoinCriteria(_xConnection,pData->GetConnLineDataList(),pData));
        }

        return aErg;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                OQueryTableWindow* pLh,
                                OQueryTableWindow* pRh,
                                OQueryTableConnectionData* pData
                                )
    {
        bool bForce = pData->GetJoinType() == CROSS_JOIN || pData->isNatural();
        return BuildJoin(_xConnection,BuildTable(_xConnection,pLh,bForce),BuildTable(_xConnection,pRh,bForce),pData);
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                const ::rtl::OUString &rLh,
                                OQueryTableWindow* pRh,
                                OQueryTableConnectionData* pData
                                )
    {
        return BuildJoin(_xConnection,rLh,BuildTable(_xConnection,pRh),pData);
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                OQueryTableWindow* pLh,
                                const ::rtl::OUString &rRh,
                                OQueryTableConnectionData* pData
                                )
    {
        return BuildJoin(_xConnection,BuildTable(_xConnection,pLh),rRh,pData);
    }
    //------------------------------------------------------------------------------
    void GetNextJoin(   const Reference< XConnection>& _xConnection,
                        OQueryTableConnection* pEntryConn,
                        OQueryTableWindow* pEntryTabTo,
                        ::rtl::OUString &aJoin)
    {
        OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData().get());
        if ( pEntryConnData->GetJoinType() == INNER_JOIN && !pEntryConnData->isNatural() )
            return;

        //  Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();

        if(!aJoin.getLength())
        {
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            aJoin = BuildJoin(_xConnection,pEntryTabFrom,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetDestWin())
        {
            aJoin = BuildJoin(_xConnection,aJoin,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetSourceWin())
        {
            aJoin = BuildJoin(_xConnection,pEntryTabTo,aJoin,pEntryConnData);
        }

        pEntryConn->SetVisited(sal_True);

        // first search for the "to" window
        const ::std::vector<OTableConnection*>* pConnections = pEntryConn->GetParent()->getTableConnections();
        ::std::vector<OTableConnection*>::const_iterator aIter = pConnections->begin();
        ::std::vector<OTableConnection*>::const_iterator aEnd = pConnections->end();
        for(;aIter != aEnd;++aIter)
        {
            OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(*aIter);
            if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabTo || pNext->GetDestWin() == pEntryTabTo))
            {
                OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabTo ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
                // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
                JoinCycle(_xConnection,pNext,pEntryTab,aJoin);
                if(!pNext->IsVisited())
                    GetNextJoin(_xConnection,pNext,pEntryTab,aJoin);
            }
        }

        // when nothing found found look for the "from" window
        if(aIter == aEnd)
        {
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            aIter = pConnections->begin();
            for(;aIter != aEnd;++aIter)
            {
                OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(*aIter);
                if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabFrom || pNext->GetDestWin() == pEntryTabFrom))
                {
                    OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabFrom ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
                    // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
                    JoinCycle(_xConnection,pNext,pEntryTab,aJoin);
                    if(!pNext->IsVisited())
                        GetNextJoin(_xConnection,pNext,pEntryTab,aJoin);
                }
            }
        }
    }
    //------------------------------------------------------------------------------
    SqlParseError InsertJoinConnection( const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode *pNode,
                                    const EJoinType& _eJoinType,
                                    const ::connectivity::OSQLParseNode *pLeftTable,
                                    const ::connectivity::OSQLParseNode *pRightTable)
    {
        SqlParseError eErrorCode = eOk;
        if (pNode->count() == 3 &&  // Ausdruck is geklammert
            SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pNode->getChild(2),")"))
        {
            eErrorCode = InsertJoinConnection(_pView,pNode->getChild(1), _eJoinType,pLeftTable,pRightTable);
        }
        else if (SQL_ISRULEOR2(pNode,search_condition,boolean_term) &&          // AND/OR-Verknuepfung:
                 pNode->count() == 3)
        {
            // nur AND Verknüpfung zulassen
            if (!SQL_ISTOKEN(pNode->getChild(1),AND))
                eErrorCode = eIllegalJoinCondition;
            else if ( eOk == (eErrorCode = InsertJoinConnection(_pView,pNode->getChild(0), _eJoinType,pLeftTable,pRightTable)) )
                    eErrorCode = InsertJoinConnection(_pView,pNode->getChild(2), _eJoinType,pLeftTable,pRightTable);
        }
        else if (SQL_ISRULE(pNode,comparison_predicate))
        {
            // only the comparison of columns is allowed
            DBG_ASSERT(pNode->count() == 3,"OQueryDesignView::InsertJoinConnection: Fehler im Parse Tree");
            if (!(SQL_ISRULE(pNode->getChild(0),column_ref) &&
                  SQL_ISRULE(pNode->getChild(2),column_ref) &&
                   pNode->getChild(1)->getNodeType() == SQL_NODE_EQUAL))
            {
                String sError(ModuleRes(STR_QRY_JOIN_COLUMN_COMPARE));
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
                // OQueryTableWindow*   pRightWindow = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( getTableRange(_pView,pRightTable->getByRule(OSQLParseNode::table_ref) ));
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
    //------------------------------------------------------------------------------
    sal_Bool GetInnerJoinCriteria(  const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode *pCondition)
    {
        return InsertJoinConnection(_pView,pCondition, INNER_JOIN,NULL,NULL) != eOk;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString GenerateSelectList( const OQueryDesignView* _pView,
                                        OTableFields&   _rFieldList,
                                        sal_Bool bAlias)
    {
        Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();
        if ( !xConnection.is() )
            return ::rtl::OUString();

        ::rtl::OUStringBuffer aTmpStr,aFieldListStr;

        sal_Bool bAsterix = sal_False;
        int nVis = 0;
        OTableFields::iterator aIter = _rFieldList.begin();
        OTableFields::iterator aEnd = _rFieldList.end();
        for(;aIter != aEnd;++aIter)
        {
            OTableFieldDescRef pEntryField = *aIter;
            if ( pEntryField->IsVisible() )
            {
                if ( pEntryField->GetField().toChar() == '*' )
                    bAsterix = sal_True;
                ++nVis;
            }
        }
        if(nVis == 1)
            bAsterix = sal_False;

        try
        {
            const Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            const ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

            OJoinTableView::OTableWindowMap* pTabList = _pView->getTableView()->GetTabWinMap();

            const static ::rtl::OUString sFieldSeparator(RTL_CONSTASCII_USTRINGPARAM(", "));
            const static ::rtl::OUString s_sAs(RTL_CONSTASCII_USTRINGPARAM(" AS "));

            aIter = _rFieldList.begin();
            for(;aIter != aEnd;++aIter)
            {
                OTableFieldDescRef pEntryField = *aIter;
                ::rtl::OUString rFieldName = pEntryField->GetField();
                if ( rFieldName.getLength() && pEntryField->IsVisible() )
                {
                    aTmpStr = ::rtl::OUString();
                    const ::rtl::OUString rAlias = pEntryField->GetAlias();
                    const ::rtl::OUString rFieldAlias = pEntryField->GetFieldAlias();

                    aTmpStr.append(quoteTableAlias((bAlias || bAsterix),rAlias,aQuote));

                    // if we have a none numeric field, the table alias could be in the name
                    // otherwise we are not allowed to do this (e.g. 0.1 * PRICE )
                    if  ( !pEntryField->isOtherFunction() )
                    {
                        // we have to look if we have alias.* here but before we have to check if the column doesn't already exist
                        String sTemp = rFieldName;
                        OTableFieldDescRef  aInfo = new OTableFieldDesc();
                        OJoinTableView::OTableWindowMap::iterator tableIter = pTabList->begin();
                        OJoinTableView::OTableWindowMap::iterator tableEnd = pTabList->end();
                        sal_Bool bFound = sal_False;
                        for(;!bFound && tableIter != tableEnd ;++tableIter)
                        {
                            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(tableIter->second);

                            bFound = pTabWin->ExistsField( rFieldName, aInfo );
                            if ( bFound )
                                rFieldName = aInfo->GetField();
                        }
                        if ( ( rFieldName.toChar() != '*' ) && ( rFieldName.indexOf( aQuote ) == -1 ) )
                        {
                            OSL_ENSURE(pEntryField->GetTable().getLength(),"No table field name!");
                            aTmpStr.append(::dbtools::quoteName(aQuote, rFieldName));
                        }
                        else
                            aTmpStr.append(rFieldName);
                    }
                    else
                        aTmpStr.append(rFieldName);

                    if  ( pEntryField->isAggreateFunction() )
                    {
                        DBG_ASSERT(pEntryField->GetFunction().getLength(),"Functionname darf hier nicht leer sein! ;-(");
                        ::rtl::OUStringBuffer aTmpStr2( pEntryField->GetFunction());
                        aTmpStr2.appendAscii("(");
                        aTmpStr2.append(aTmpStr.makeStringAndClear());
                        aTmpStr2.appendAscii(")");
                        aTmpStr = aTmpStr2;
                    }

                    if (rFieldAlias.getLength()                         &&
                        (rFieldName.toChar() != '*'                     ||
                        pEntryField->isNumericOrAggreateFunction()      ||
                        pEntryField->isOtherFunction()))
                    {
                        aTmpStr.append(s_sAs);
                        aTmpStr.append(::dbtools::quoteName(aQuote, rFieldAlias));
                    }
                    aFieldListStr.append(aTmpStr.makeStringAndClear());
                    aFieldListStr.append(sFieldSeparator);
                }
            }
            if(aFieldListStr.getLength())
                aFieldListStr.setLength(aFieldListStr.getLength()-2);
        }
        catch(SQLException&)
        {
            OSL_ASSERT(!"Failure while building select list!");
        }
        return aFieldListStr.makeStringAndClear();
    }
    //------------------------------------------------------------------------------
    sal_Bool GenerateCriterias( OQueryDesignView* _pView,
                                ::rtl::OUStringBuffer& rRetStr,
                                ::rtl::OUStringBuffer& rHavingStr,
                                OTableFields& _rFieldList,
                                sal_Bool bMulti )
    {
        // * darf keine Filter enthalten : habe ich die entsprechende Warnung schon angezeigt ?
        sal_Bool bCritsOnAsterikWarning = sal_False;        // ** TMFS **

        ::rtl::OUString aFieldName,aCriteria,aWhereStr,aHavingStr,aWork/*,aOrderStr*/;
        // Zeilenweise werden die Ausdr"ucke mit AND verknuepft
        sal_uInt16 nMaxCriteria = 0;
        OTableFields::iterator aIter = _rFieldList.begin();
        OTableFields::iterator aEnd = _rFieldList.end();
        for(;aIter != aEnd;++aIter)
        {
            nMaxCriteria = ::std::max<sal_uInt16>(nMaxCriteria,(sal_uInt16)(*aIter)->GetCriteria().size());
        }
        Reference< XConnection> xConnection = static_cast<OQueryController&>(_pView->getController()).getConnection();
        if(!xConnection.is())
            return sal_False;
        try
        {
            const Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            const ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
            const IParseContext& rContext = static_cast<OQueryController&>(_pView->getController()).getParser().getContext();

            for (sal_uInt16 i=0 ; i < nMaxCriteria ; i++)
            {
                aHavingStr = aWhereStr = ::rtl::OUString();

                for(aIter = _rFieldList.begin();aIter != aEnd;++aIter)
                {
                    OTableFieldDescRef  pEntryField = *aIter;
                    aFieldName = pEntryField->GetField();

                    if (!aFieldName.getLength())
                        continue;
                    aCriteria = pEntryField->GetCriteria( i );
                    if ( aCriteria.getLength() )
                    {
                        // * is not allowed to contain any filter, only when used in combination an aggregate function
                        if ( aFieldName.toChar() == '*' && pEntryField->isNoneFunction() )
                        {
                            // only show the messagebox the first time
                            if (!bCritsOnAsterikWarning)
                                ErrorBox(_pView, ModuleRes( ERR_QRY_CRITERIA_ON_ASTERISK)).Execute();
                            bCritsOnAsterikWarning = sal_True;
                            continue;
                        }
                        aWork = ::rtl::OUString();


                        aWork += quoteTableAlias(bMulti,pEntryField->GetAlias(),aQuote);

                        if ( (pEntryField->GetFunctionType() & (FKT_OTHER|FKT_NUMERIC)) || (aFieldName.toChar() == '*') )
                            aWork += aFieldName;
                        else
                            aWork += ::dbtools::quoteName(aQuote, aFieldName);

                        if ( pEntryField->isAggreateFunction() || pEntryField->IsGroupBy() )
                        {
                            if (!aHavingStr.getLength())            // noch keine Kriterien
                                aHavingStr += ::rtl::OUString('(');         // Klammern
                            else
                                aHavingStr += C_AND;

                            if ( pEntryField->isAggreateFunction() )
                            {
                                OSL_ENSURE(pEntryField->GetFunction().getLength(),"No function name for aggregate given!");
                                aHavingStr += pEntryField->GetFunction();
                                aHavingStr += ::rtl::OUString('(');         // Klammern
                                aHavingStr += aWork;
                                aHavingStr += ::rtl::OUString(')');         // Klammern
                            }
                            else
                                aHavingStr += aWork;

                            ::rtl::OUString aTmp = aCriteria;
                            ::rtl::OUString aErrorMsg;
                            Reference<XPropertySet> xColumn;
                            ::std::auto_ptr< ::connectivity::OSQLParseNode> pParseNode(_pView->getPredicateTreeFromEntry(pEntryField,aTmp,aErrorMsg,xColumn));
                            if (pParseNode.get())
                            {
                                if (bMulti && !(pEntryField->isOtherFunction() || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(pEntryField->GetAlias(),aFieldName);
                                ::rtl::OUString sHavingStr = aHavingStr;

                                sal_uInt32 nCount = pParseNode->count();
                                for( sal_uInt32 node = 1 ; node < nCount ; ++node)
                                    pParseNode->getChild(node)->parseNodeToStr( sHavingStr,
                                                                xConnection,
                                                                &rContext,
                                                                sal_False,
                                                                !pEntryField->isOtherFunction());
                                aHavingStr = sHavingStr;
                            }
                            else
                                aHavingStr += aCriteria;
                        }
                        else
                        {
                            if ( !aWhereStr.getLength() )           // noch keine Kriterien
                                aWhereStr += ::rtl::OUString('(');          // Klammern
                            else
                                aWhereStr += C_AND;

                            aWhereStr += ::rtl::OUString(' ');
                            // aCriteria could have some german numbers so I have to be sure here
                            ::rtl::OUString aTmp = aCriteria;
                            ::rtl::OUString aErrorMsg;
                            Reference<XPropertySet> xColumn;
                            ::std::auto_ptr< ::connectivity::OSQLParseNode> pParseNode( _pView->getPredicateTreeFromEntry(pEntryField,aTmp,aErrorMsg,xColumn));
                            if (pParseNode.get())
                            {
                                if (bMulti && !(pEntryField->isOtherFunction() || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(pEntryField->GetAlias(),aFieldName);
                                ::rtl::OUString aWhere = aWhereStr;
                                pParseNode->parseNodeToStr( aWhere,
                                                            xConnection,
                                                            &rContext,
                                                            sal_False,
                                                            !pEntryField->isOtherFunction() );
                                aWhereStr = aWhere;
                            }
                            else
                            {
                                aWhereStr += aWork;
                                aWhereStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                                aWhereStr += aCriteria;
                            }
                        }
                    }
                    // nur einmal für jedes Feld
                    else if ( !i && pEntryField->isCondition() )
                    {
                        if (!aWhereStr.getLength())         // noch keine Kriterien
                            aWhereStr += ::rtl::OUString('(');          // Klammern
                        else
                            aWhereStr += C_AND;
                        aWhereStr += pEntryField->GetField();
                    }
                }
                if (aWhereStr.getLength())
                {
                    aWhereStr += ::rtl::OUString(')');                      // Klammern zu fuer 'AND' Zweig
                    if (rRetStr.getLength())                            // schon Feldbedingungen ?
                        rRetStr.append(C_OR);
                    else                                        // Klammern auf fuer 'OR' Zweig
                        rRetStr.append(sal_Unicode('('));
                    rRetStr.append(aWhereStr);
                }
                if (aHavingStr.getLength())
                {
                    aHavingStr += ::rtl::OUString(')');                     // Klammern zu fuer 'AND' Zweig
                    if (rHavingStr.getLength())                         // schon Feldbedingungen ?
                        rHavingStr.append(C_OR);
                    else                                        // Klammern auf fuer 'OR' Zweig
                        rHavingStr.append(sal_Unicode('('));
                    rHavingStr.append(aHavingStr);
                }
            }

            if (rRetStr.getLength())
                rRetStr.append(sal_Unicode(')'));                               // Klammern zu fuer 'OR' Zweig
            if (rHavingStr.getLength())
                rHavingStr.append(sal_Unicode(')'));                                // Klammern zu fuer 'OR' Zweig
        }
        catch(SQLException&)
        {
            OSL_ASSERT(!"Failure while building where clause!");
        }
        return sal_True;
    }
    //------------------------------------------------------------------------------
    SqlParseError GenerateOrder(    OQueryDesignView* _pView,
                                    OTableFields& _rFieldList,
                                    sal_Bool bMulti,
                                    ::rtl::OUString& _rsRet)
    {
        const OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        Reference< XConnection> xConnection = rController.getConnection();
        if ( !xConnection.is() )
            return eNoConnection;

        SqlParseError eErrorCode = eOk;

        ::rtl::OUString aColumnName;
        ::rtl::OUString aWorkStr;
        try
        {
            const bool bColumnAliasInOrderBy = rController.getSdbMetaData().supportsColumnAliasInOrderBy();
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
            // * darf keine Filter enthalten : habe ich die entsprechende Warnung schon angezeigt ?
            sal_Bool bCritsOnAsterikWarning = sal_False;        // ** TMFS **
            OTableFields::iterator aIter = _rFieldList.begin();
            OTableFields::iterator aEnd = _rFieldList.end();
            for(;aIter != aEnd;++aIter)
            {
                OTableFieldDescRef  pEntryField = *aIter;
                EOrderDir eOrder = pEntryField->GetOrderDir();

                // nur wenn eine Sortierung und ein Tabellenname vorhanden ist-> erzeugen
                // sonst werden die Expressions vom Order By im GenerateCriteria mit erzeugt
                if ( eOrder != ORDER_NONE )
                {
                    aColumnName = pEntryField->GetField();
                    if(aColumnName.toChar() == '*')
                    {
                        // die entsprechende MessageBox nur beim ersten mal anzeigen
                        if (!bCritsOnAsterikWarning)
                            ErrorBox(_pView, ModuleRes( ERR_QRY_ORDERBY_ON_ASTERISK)).Execute();
                        bCritsOnAsterikWarning = sal_True;
                        continue;
                    }

                    if ( bColumnAliasInOrderBy && pEntryField->GetFieldAlias().getLength() )
                    {
                        aWorkStr += ::dbtools::quoteName(aQuote, pEntryField->GetFieldAlias());
                    }
                    else if ( pEntryField->isNumericOrAggreateFunction() )
                    {
                        DBG_ASSERT(pEntryField->GetFunction().getLength(),"Functionname darf hier nicht leer sein! ;-(");
                        aWorkStr += pEntryField->GetFunction();
                        aWorkStr +=  ::rtl::OUString('(');
                        aWorkStr += quoteTableAlias(bMulti,pEntryField->GetAlias(),aQuote);
                        // only quote column name when we don't have a numeric
                        if ( pEntryField->isNumeric() )
                            aWorkStr += aColumnName;
                        else
                            aWorkStr += ::dbtools::quoteName(aQuote, aColumnName);

                        aWorkStr +=  ::rtl::OUString(')');
                    }
                    else if ( pEntryField->isOtherFunction() )
                    {
                        aWorkStr += aColumnName;
                    }
                    else
                    {
                        aWorkStr += quoteTableAlias(bMulti,pEntryField->GetAlias(),aQuote);
                        aWorkStr += ::dbtools::quoteName(aQuote, aColumnName);
                    }
                    aWorkStr += ::rtl::OUString(' ');
                    aWorkStr += String::CreateFromAscii( ";ASC;DESC" ).GetToken( (sal_uInt16)eOrder );
                    aWorkStr += ::rtl::OUString(',');
                }
            }

            {
                String sTemp(aWorkStr);
                sTemp.EraseTrailingChars( ',' );
                aWorkStr = sTemp;
            }

            if ( aWorkStr.getLength() )
            {
                const sal_Int32 nMaxOrder = xMetaData->getMaxColumnsInOrderBy();
                String sToken(aWorkStr);
                if ( nMaxOrder && nMaxOrder < sToken.GetTokenCount(',') )
                    eErrorCode = eStatementTooLong;
                else
                {
                    _rsRet = ::rtl::OUString::createFromAscii(" ORDER BY ");
                    _rsRet += aWorkStr;
                }
            }
        }
        catch(SQLException&)
        {
            OSL_ASSERT(!"Failure while building group by!");
        }

        return eErrorCode;
    }

    //------------------------------------------------------------------------------
    void GenerateInnerJoinCriterias(const Reference< XConnection>& _xConnection,
                                    ::rtl::OUString& _rJoinCrit,
                                    const ::std::vector<OTableConnection*>* _pConnList)
    {
        ::std::vector<OTableConnection*>::const_iterator aIter = _pConnList->begin();
        ::std::vector<OTableConnection*>::const_iterator aEnd = _pConnList->end();
        for(;aIter != aEnd;++aIter)
        {
            const OQueryTableConnection* pEntryConn = static_cast<const OQueryTableConnection*>(*aIter);
            OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData().get());
            if ( pEntryConnData->GetJoinType() == INNER_JOIN && !pEntryConnData->isNatural() )
            {
                if(_rJoinCrit.getLength())
                    _rJoinCrit += C_AND;
                _rJoinCrit += BuildJoinCriteria(_xConnection,pEntryConnData->GetConnLineDataList(),pEntryConnData);
            }
        }
    }
    //------------------------------------------------------------------------------
    void searchAndAppendName(const Reference< XConnection>& _xConnection,
                             const OQueryTableWindow* _pTableWindow,
                             ::std::map< ::rtl::OUString,sal_Bool,::comphelper::UStringMixLess>& _rTableNames,
                             ::rtl::OUString& _rsTableListStr
                             )
    {
        ::rtl::OUString sTabName(BuildTable(_xConnection,_pTableWindow));

        if(_rTableNames.find(sTabName) == _rTableNames.end())
        {
            _rTableNames[sTabName] = sal_True;
            _rsTableListStr += sTabName;
            _rsTableListStr += ::rtl::OUString(',');
        }
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString GenerateFromClause( const Reference< XConnection>& _xConnection,
                                        const OQueryTableView::OTableWindowMap* pTabList,
                                        const ::std::vector<OTableConnection*>* pConnList
                                        )
    {

        ::rtl::OUString aTableListStr;
        // wird gebraucht um sicher zustelllen das eine Tabelle nicht doppelt vorkommt
        ::std::map< ::rtl::OUString,sal_Bool,::comphelper::UStringMixLess> aTableNames;

        // generate outer join clause in from
        if(!pConnList->empty())
        {
            ::std::vector<OTableConnection*>::const_iterator aIter = pConnList->begin();
            ::std::vector<OTableConnection*>::const_iterator aEnd = pConnList->end();
            ::std::map<OTableWindow*,sal_Int32> aConnectionCount;
            for(;aIter != aEnd;++aIter)
            {
                static_cast<OQueryTableConnection*>(*aIter)->SetVisited(sal_False);
                if ( aConnectionCount.find((*aIter)->GetSourceWin()) == aConnectionCount.end() )
                    aConnectionCount.insert(::std::map<OTableWindow*,sal_Int32>::value_type((*aIter)->GetSourceWin(),0));
                else
                    aConnectionCount[(*aIter)->GetSourceWin()]++;
                if ( aConnectionCount.find((*aIter)->GetDestWin()) == aConnectionCount.end() )
                    aConnectionCount.insert(::std::map<OTableWindow*,sal_Int32>::value_type((*aIter)->GetDestWin(),0));
                else
                    aConnectionCount[(*aIter)->GetDestWin()]++;
            }
            ::std::multimap<sal_Int32 , OTableWindow*> aMulti;
            ::std::map<OTableWindow*,sal_Int32>::iterator aCountIter = aConnectionCount.begin();
            ::std::map<OTableWindow*,sal_Int32>::iterator aCountEnd = aConnectionCount.end();
            for(;aCountIter != aCountEnd;++aCountIter)
            {
                aMulti.insert(::std::multimap<sal_Int32 , OTableWindow*>::value_type(aCountIter->second,aCountIter->first));
            }

            const sal_Bool bUseEscape = ::dbtools::getBooleanDataSourceSetting( _xConnection, PROPERTY_OUTERJOINESCAPE );
            ::std::multimap<sal_Int32 , OTableWindow*>::reverse_iterator aRIter = aMulti.rbegin();
            ::std::multimap<sal_Int32 , OTableWindow*>::reverse_iterator aREnd = aMulti.rend();
            for(;aRIter != aREnd;++aRIter)
            {
                ::std::vector<OTableConnection*>::const_iterator aConIter = aRIter->second->getTableView()->getTableConnections(aRIter->second);
                for(;aConIter != aEnd;++aConIter)
                {
                    OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>(*aConIter);
                    if(!pEntryConn->IsVisited() && pEntryConn->GetSourceWin() == aRIter->second )
                    {
                        ::rtl::OUString aJoin;
                        GetNextJoin(_xConnection,pEntryConn,static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()),aJoin);

                        if(aJoin.getLength())
                        {
                            // insert tables into table list to avoid double entries
                            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
                            OQueryTableWindow* pEntryTabTo = static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin());

                            ::rtl::OUString sTabName(BuildTable(_xConnection,pEntryTabFrom));
                            if(aTableNames.find(sTabName) == aTableNames.end())
                                aTableNames[sTabName] = sal_True;
                            sTabName = BuildTable(_xConnection,pEntryTabTo);
                            if(aTableNames.find(sTabName) == aTableNames.end())
                                aTableNames[sTabName] = sal_True;

                            ::rtl::OUString aStr;
                            switch(static_cast<OQueryTableConnectionData*>(pEntryConn->GetData().get())->GetJoinType())
                            {
                                case LEFT_JOIN:
                                case RIGHT_JOIN:
                                case FULL_JOIN:
                                    {
                                        // create outer join
                                        if ( bUseEscape )
                                            aStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("{ OJ "));
                                        aStr += aJoin;
                                        if ( bUseEscape )
                                            aStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" }"));
                                    }
                                    break;
                                default:
                                    aStr += aJoin;
                                    break;
                            }
                            aStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                            aTableListStr += aStr;
                        }
                    }
                }
            }

            // and now all inner joins
            aIter = pConnList->begin();
            for(;aIter != aEnd;++aIter)
            {
                OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>(*aIter);
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
        OQueryTableView::OTableWindowMap::const_iterator aTabIter = pTabList->begin();
        OQueryTableView::OTableWindowMap::const_iterator aTabEnd = pTabList->end();
        for(;aTabIter != aTabEnd;++aTabIter)
        {
            const OQueryTableWindow* pEntryTab = static_cast<const OQueryTableWindow*>(aTabIter->second);
            if(!pEntryTab->ExistsAConn())
            {
                aTableListStr += BuildTable(_xConnection,pEntryTab);
                aTableListStr += ::rtl::OUString(',');
            }
        }

        if(aTableListStr.getLength())
            aTableListStr = aTableListStr.replaceAt(aTableListStr.getLength()-1,1, ::rtl::OUString() );
        return aTableListStr;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString GenerateGroupBy(const OQueryDesignView* _pView,OTableFields& _rFieldList, sal_Bool bMulti )
    {
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        const Reference< XConnection> xConnection = rController.getConnection();
        if(!xConnection.is())
            return ::rtl::OUString();

        ::std::map< rtl::OUString,bool> aGroupByNames;

        ::rtl::OUString aGroupByStr;
        try
        {
            const Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            const ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

            OTableFields::iterator aIter = _rFieldList.begin();
            OTableFields::iterator aEnd = _rFieldList.end();
            for(;aIter != aEnd;++aIter)
            {
                OTableFieldDescRef  pEntryField = *aIter;
                if ( pEntryField->IsGroupBy() )
                {
                    DBG_ASSERT(pEntryField->GetField().getLength(),"Kein FieldName vorhanden!;-(");
                    ::rtl::OUString sGroupByPart = quoteTableAlias(bMulti,pEntryField->GetAlias(),aQuote);

                    // only quote the field name when it isn't calculated
                    if ( pEntryField->isNoneFunction() )
                    {
                        sGroupByPart += ::dbtools::quoteName(aQuote, pEntryField->GetField());
                    }
                    else
                    {
                        ::rtl::OUString aTmp = pEntryField->GetField();
                        ::rtl::OUString aErrorMsg;
                        Reference<XPropertySet> xColumn;
                        ::std::auto_ptr< ::connectivity::OSQLParseNode> pParseNode(_pView->getPredicateTreeFromEntry(pEntryField,aTmp,aErrorMsg,xColumn));
                        if (pParseNode.get())
                        {
                            ::rtl::OUString sGroupBy;
                            pParseNode->getChild(0)->parseNodeToStr(    sGroupBy,
                                                        xConnection,
                                                        &rController.getParser().getContext(),
                                                        sal_False,
                                                        !pEntryField->isOtherFunction());
                            sGroupByPart += sGroupBy;
                        }
                        else
                            sGroupByPart += pEntryField->GetField();
                    }
                    if ( aGroupByNames.find(sGroupByPart) == aGroupByNames.end() )
                    {
                        aGroupByNames.insert(::std::map< rtl::OUString,bool>::value_type(sGroupByPart,true));
                        aGroupByStr += sGroupByPart;
                        aGroupByStr += ::rtl::OUString(',');
                    }
                }
            }
            if ( aGroupByStr.getLength() )
            {
                aGroupByStr = aGroupByStr.replaceAt(aGroupByStr.getLength()-1,1, ::rtl::OUString(' ') );
                ::rtl::OUString aGroupByStr2 = ::rtl::OUString::createFromAscii(" GROUP BY ");
                aGroupByStr2 += aGroupByStr;
                aGroupByStr = aGroupByStr2;
            }
        }
        catch(SQLException&)
        {
            OSL_ASSERT(!"Failure while building group by!");
        }
        return aGroupByStr;
    }
    // -----------------------------------------------------------------------------
    SqlParseError GetORCriteria(OQueryDesignView* _pView,
                                OSelectionBrowseBox* _pSelectionBrw,
                                const ::connectivity::OSQLParseNode * pCondition,
                                sal_uInt16& nLevel ,
                                sal_Bool bHaving = sal_False,
                                bool bAddOrOnOneLine = false);
    // -----------------------------------------------------------------------------
    SqlParseError GetSelectionCriteria( OQueryDesignView* _pView,
                                        OSelectionBrowseBox* _pSelectionBrw,
                                        const ::connectivity::OSQLParseNode* pNode,
                                        sal_uInt16& rLevel )
    {
        if (!SQL_ISRULE(pNode, select_statement))
            return eNoSelectStatement;

        // nyi: mehr Pruefung auf korrekte Struktur!
        pNode = pNode ? pNode->getChild(3)->getChild(1) : NULL;
        // no where clause found
        if (!pNode || pNode->isLeaf())
            return eOk;

        // Naechster freier Satz ...
        SqlParseError eErrorCode = eOk;
        ::connectivity::OSQLParseNode * pCondition = pNode->getChild(1);
        if ( pCondition ) // no where clause
        {
            // now we have to chech the other conditions
            // first make the logical easier
            ::connectivity::OSQLParseNode::negateSearchCondition(pCondition);
            ::connectivity::OSQLParseNode *pNodeTmp = pNode->getChild(1);

            ::connectivity::OSQLParseNode::disjunctiveNormalForm(pNodeTmp);
            pNodeTmp = pNode->getChild(1);
            ::connectivity::OSQLParseNode::absorptions(pNodeTmp);
            pNodeTmp = pNode->getChild(1);
            // compress sort the criteria @see http://www.openoffice.org/issues/show_bug.cgi?id=24079
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
    //------------------------------------------------------------------------------
    SqlParseError GetANDCriteria(   OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const  ::connectivity::OSQLParseNode * pCondition,
                                    sal_uInt16& nLevel,
                                    sal_Bool bHaving,
                                    bool bAddOrOnOneLine);
    //------------------------------------------------------------------------------
    SqlParseError ComparisonPredicate(OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const sal_uInt16 nLevel,
                            sal_Bool bHaving,
                            bool bAddOrOnOneLine);
    //------------------------------------------------------------------------------
    SqlParseError GetORCriteria(OQueryDesignView* _pView,
                                OSelectionBrowseBox* _pSelectionBrw,
                                const ::connectivity::OSQLParseNode * pCondition,
                                sal_uInt16& nLevel ,
                                sal_Bool bHaving,
                                bool bAddOrOnOneLine)
    {
        SqlParseError eErrorCode = eOk;

        // Runde Klammern um den Ausdruck
        if (pCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
        {
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(1),nLevel,bHaving,bAddOrOnOneLine);
        }
        // oder Verknuepfung
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
                    eErrorCode = GetANDCriteria(_pView,_pSelectionBrw,pChild, nLevel,bHaving, i == 0 ? false : bAddOrOnOneLine);
                    if ( !bAddOrOnOneLine)
                        nLevel++;
                }
            }
        }
        else
            eErrorCode = GetANDCriteria( _pView,_pSelectionBrw,pCondition, nLevel, bHaving,bAddOrOnOneLine );

        return eErrorCode;
    }
    //--------------------------------------------------------------------------------------------------
    bool CheckOrCriteria(const ::connectivity::OSQLParseNode* _pCondition,::connectivity::OSQLParseNode* _pFirstColumnRef)
    {
        bool bRet = true;
        ::connectivity::OSQLParseNode* pFirstColumnRef = _pFirstColumnRef;
        for (int i = 0; i < 3 && bRet; i+=2)
        {
            const  ::connectivity::OSQLParseNode* pChild = _pCondition->getChild(i);
            if ( SQL_ISRULE(pChild,search_condition) )
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
    //--------------------------------------------------------------------------------------------------
    SqlParseError GetANDCriteria(   OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const  ::connectivity::OSQLParseNode * pCondition,
                                    sal_uInt16& nLevel,
                                    sal_Bool bHaving,
                                    bool bAddOrOnOneLine)
    {
        const ::com::sun::star::lang::Locale    aLocale = _pView->getLocale();
        const ::rtl::OUString sDecimal = _pView->getDecimalSeparator();

        // ich werde ein paar Mal einen gecasteten Pointer auf meinen ::com::sun::star::sdbcx::Container brauchen
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        SqlParseError eErrorCode = eOk;

        // Runde Klammern
        if (SQL_ISRULE(pCondition,boolean_primary))
        {
            // check if we have to put the or criteria on one line.
            const  ::connectivity::OSQLParseNode* pSearchCondition = pCondition->getChild(1);
            bool bMustAddOrOnOneLine = CheckOrCriteria(pSearchCondition,NULL);
            if ( SQL_ISRULE( pSearchCondition, search_condition) ) // we have a or
            {
                _pSelectionBrw->DuplicateConditionLevel( nLevel);
                eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSearchCondition->getChild(0), nLevel,bHaving,bMustAddOrOnOneLine );
                ++nLevel;
                eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSearchCondition->getChild(2), nLevel,bHaving,bMustAddOrOnOneLine );
            }
            else
                eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSearchCondition, nLevel,bHaving,bMustAddOrOnOneLine );
        }
        // Das erste Element ist (wieder) eine AND-Verknuepfung
        else if ( SQL_ISRULE(pCondition,boolean_term) )
        {
            OSL_ENSURE(pCondition->count() == 3,"Illegal definifiton of boolean_term");
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
                ::rtl::OUString aColumnName;
                ::rtl::OUString aCondition;
                Reference< XConnection> xConnection = rController.getConnection();
                if ( xConnection.is() )
                {
                    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
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
                        aDragLeft->SetGroupBy(sal_True);
                    _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel,bAddOrOnOneLine);
                }
            }
            else if(SQL_ISRULEOR2(pValueExp,general_set_fct ,set_fct_spec)          ||
                            SQL_ISRULEOR2(pValueExp,position_exp,extract_exp)   ||
                            SQL_ISRULEOR2(pValueExp,fold,char_substring_fct)    ||
                            SQL_ISRULEOR2(pValueExp,length_exp,char_value_fct))
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
                String sError(ModuleRes(STR_QRY_LIKE_LEFT_NO_COLUMN));
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
                ::rtl::OUString sCondition = ParseCondition(rController,pCondition,sDecimal,aLocale,1);
                OTableFieldDescRef  aDragLeft = new OTableFieldDesc();
                if ( eOk == ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(0),aDragLeft)) )
                {
                    if ( bHaving )
                        aDragLeft->SetGroupBy(sal_True);
                    _pSelectionBrw->AddCondition(aDragLeft, sCondition, nLevel,bAddOrOnOneLine);
                }
            }
            else
            {
                // Funktions-Bedingung parsen
                ::rtl::OUString sCondition = ParseCondition(rController,pCondition,sDecimal,aLocale,1);
                Reference< XConnection> xConnection = rController.getConnection();
                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                    // the international doesn't matter I have a string
                ::rtl::OUString sName;
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
                    aDragLeft->SetGroupBy(sal_True);
                _pSelectionBrw->AddCondition(aDragLeft, sCondition, nLevel,bAddOrOnOneLine);
            }
        }
        else if( SQL_ISRULEOR2(pCondition,existence_test,unique_test) )
        {
            // Funktions-Bedingung parsen
            ::rtl::OUString aCondition = ParseCondition(rController,pCondition,sDecimal,aLocale,0);

            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            aDragLeft->SetField(aCondition);
            aDragLeft->SetFunctionType(FKT_CONDITION);

            eErrorCode = _pSelectionBrw->InsertField(aDragLeft,BROWSER_INVALIDID,sal_False,sal_True).isValid() ? eOk : eTooManyColumns;
        }
        else //! TODO not supported yet
            eErrorCode = eStatementTooComplex;
        // Fehler einfach weiterreichen.
        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    SqlParseError AddFunctionCondition(OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const sal_uInt16 nLevel,
                            sal_Bool bHaving,
                            bool bAddOrOnOneLine)
    {
        SqlParseError eErrorCode = eOk;
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());

        OSQLParseNode* pFunction = pCondition->getChild(0);

        OSL_ENSURE(SQL_ISRULEOR2(pFunction,general_set_fct ,set_fct_spec)           ||
                            SQL_ISRULEOR2(pFunction,position_exp,extract_exp)   ||
                            SQL_ISRULEOR2(pFunction,fold,char_substring_fct)    ||
                            SQL_ISRULEOR2(pFunction,length_exp,char_value_fct),"Illegal call!");
        ::rtl::OUString aCondition;
        OTableFieldDescRef aDragLeft = new OTableFieldDesc();

        ::rtl::OUString aColumnName;
        Reference< XConnection> xConnection = rController.getConnection();
        if(xConnection.is())
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            pCondition->parseNodeToPredicateStr(aCondition,
                                                xConnection,
                                                rController.getNumberFormatter(),
                                                _pView->getLocale(),
                                                static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                &rController.getParser().getContext());

            pFunction->parseNodeToStr(  aColumnName,
                                        xConnection,
                                        &rController.getParser().getContext(),
                                        sal_True,
                                        sal_True); // quote is to true because we need quoted elements inside the function
            // i75557
            //pFunction->parseNodeToPredicateStr(aColumnName,
            //                                  xConnection,
            //                                  rController.getNumberFormatter(),
            //                                  _pView->getLocale(),
            //                                  static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
            //                                  &rController.getParser().getContext());
            // don't display the column name
            aCondition = aCondition.copy(aColumnName.getLength());
            aCondition = aCondition.trim();
            if ( aCondition.indexOf('=',0) == 0 ) // ignore the equal sign
                aCondition = aCondition.copy(1);


            if ( SQL_ISRULE(pFunction, general_set_fct ) )
            {
                sal_Int32 nFunctionType = FKT_AGGREGATE;
                OSQLParseNode* pParamNode = pFunction->getChild(pFunction->count()-2);
                if ( pParamNode && pParamNode->getTokenValue().toChar() == '*' )
                {
                    OJoinTableView::OTableWindowMap* pTabList = _pView->getTableView()->GetTabWinMap();
                    OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
                    OJoinTableView::OTableWindowMap::iterator aTabEnd = pTabList->end();
                    for(;aIter != aTabEnd;++aIter)
                    {
                        OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);
                        if (pTabWin->ExistsField( ::rtl::OUString::createFromAscii("*"), aDragLeft ))
                        {
                            aDragLeft->SetAlias(String());
                            aDragLeft->SetTable(String());
                            break;
                        }
                    }
                }
                else if( eOk != ( eErrorCode = FillDragInfo(_pView,pParamNode,aDragLeft))
                        && SQL_ISRULE(pParamNode,num_value_exp) )
                {
                    ::rtl::OUString sParameterValue;
                    pParamNode->parseNodeToStr( sParameterValue,
                                                xConnection,
                                                &rController.getParser().getContext());
                    nFunctionType |= FKT_NUMERIC;
                    aDragLeft->SetField(sParameterValue);
                    eErrorCode = eOk;
                }
                aDragLeft->SetFunctionType(nFunctionType);
                if ( bHaving )
                    aDragLeft->SetGroupBy(sal_True);
                sal_Int32 nIndex = 0;
                aDragLeft->SetFunction(aColumnName.getToken(0,'(',nIndex));
            }
            else
            {
                // bei unbekannten Funktionen wird der gesamte Text in das Field gechrieben
                aDragLeft->SetField(aColumnName);
                if(bHaving)
                    aDragLeft->SetGroupBy(sal_True);
                aDragLeft->SetFunctionType(FKT_OTHER|FKT_NUMERIC);
            }
            _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel,bAddOrOnOneLine);
        }

        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    SqlParseError ComparisonPredicate(OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const sal_uInt16 nLevel,
                            sal_Bool bHaving
                            ,bool bAddOrOnOneLine)
    {
        SqlParseError eErrorCode = eOk;
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());

        DBG_ASSERT(SQL_ISRULE( pCondition, comparison_predicate),"ComparisonPredicate: pCondition ist kein ComparisonPredicate");
        if ( SQL_ISRULE(pCondition->getChild(0), column_ref )
            || SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref) )
        {
            ::rtl::OUString aCondition;
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
                    OConnectionLineDataVec* pLineDataList = pConn->GetData()->GetConnLineDataList();
                    OConnectionLineDataVec::iterator aIter = pLineDataList->begin();
                    OConnectionLineDataVec::iterator aEnd = pLineDataList->end();
                    for(;aIter != aEnd;++aIter)
                    {
                        if((*aIter)->GetSourceFieldName() == aDragLeft->GetField() ||
                           (*aIter)->GetDestFieldName() == aDragLeft->GetField() )
                            break;
                    }
                    if(aIter != aEnd)
                        return eOk;
                }
            }

            sal_uInt32 nPos = 0;
            if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
            {
                nPos = 0;
                sal_uInt32 i=1;

                // don't display the equal
                if (pCondition->getChild(i)->getNodeType() == SQL_NODE_EQUAL)
                    i++;

                // Bedingung parsen
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
                    case SQL_NODE_EQUAL:
                        // don't display the equal
                        i--;
                        break;
                    case SQL_NODE_LESS:
                        // take the opposite as we change the order
                        i--;
                        aCondition = aCondition + ::rtl::OUString::createFromAscii(">");
                        break;
                    case SQL_NODE_LESSEQ:
                        // take the opposite as we change the order
                        i--;
                        aCondition = aCondition + ::rtl::OUString::createFromAscii(">=");
                        break;
                    case SQL_NODE_GREAT:
                        // take the opposite as we change the order
                        i--;
                        aCondition = aCondition + ::rtl::OUString::createFromAscii("<");
                        break;
                    case SQL_NODE_GREATEQ:
                        // take the opposite as we change the order
                        i--;
                        aCondition = aCondition + ::rtl::OUString::createFromAscii("<=");
                        break;
                    default:
                        break;
                }

                // go backward
                Reference< XConnection> xConnection = rController.getConnection();
                if(xConnection.is())
                {
                    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
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
                    aDragLeft->SetGroupBy(sal_True);
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
        else // kann sich nur um einen Expr. Ausdruck handeln
        {
            ::rtl::OUString aName,aCondition;

            ::connectivity::OSQLParseNode *pLhs = pCondition->getChild(0);
            ::connectivity::OSQLParseNode *pRhs = pCondition->getChild(2);
            // Feldnamen
            Reference< XConnection> xConnection = rController.getConnection();
            if(xConnection.is())
            {
                pLhs->parseNodeToStr(aName,
                                     xConnection,
                                     &rController.getParser().getContext(),
                                     sal_True);
                // Kriterium
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
            // und anh"angen
            _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel,bAddOrOnOneLine);
        }
        return eErrorCode;
    }

    //------------------------------------------------------------------------------
    namespace
    {
        OQueryTableWindow* lcl_findColumnInTables( const ::rtl::OUString& _rColumName, const OJoinTableView::OTableWindowMap& _rTabList, OTableFieldDescRef& _rInfo )
        {
            OJoinTableView::OTableWindowMap::const_iterator aIter = _rTabList.begin();
            OJoinTableView::OTableWindowMap::const_iterator aEnd = _rTabList.end();
            for ( ; aIter != aEnd; ++aIter )
            {
                OQueryTableWindow* pTabWin = static_cast< OQueryTableWindow* >( aIter->second );
                if ( pTabWin && pTabWin->ExistsField( _rColumName, _rInfo ) )
                    return pTabWin;
            }
            return NULL;
        }
    }

    //------------------------------------------------------------------------------
    void InsertColumnRef(const OQueryDesignView* _pView,
                        const ::connectivity::OSQLParseNode * pColumnRef,
                        ::rtl::OUString& aColumnName,
                        const ::rtl::OUString& aColumnAlias,
                        ::rtl::OUString& aTableRange,
                        OTableFieldDescRef& _raInfo,
                        OJoinTableView::OTableWindowMap* pTabList)
    {

        // Tabellennamen zusammen setzen
        ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController&>(_pView->getController()).getParseIterator();
        rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

        sal_Bool bFound(sal_False);
        DBG_ASSERT(aColumnName.getLength(),"Columnname darf nicht leer sein");
        if (!aTableRange.getLength())
        {
            // SELECT column, ...
            bFound = NULL != lcl_findColumnInTables( aColumnName, *pTabList, _raInfo );
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
                bFound = sal_True;
            }
        }
        if (!bFound)
        {
            _raInfo->SetTable(::rtl::OUString());
            _raInfo->SetAlias(::rtl::OUString());
            _raInfo->SetField(aColumnName);
            _raInfo->SetFieldAlias(aColumnAlias);   // nyi : hier ein fortlaufendes Expr_1, Expr_2 ...
            _raInfo->SetFunctionType(FKT_OTHER);
        }
    }
    //-----------------------------------------------------------------------------
    sal_Bool checkJoinConditions(   const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode* _pNode )
    {
        const ::connectivity::OSQLParseNode* pJoinNode = NULL;
        sal_Bool bRet = sal_True;
        if (SQL_ISRULE(_pNode,qualified_join))
            pJoinNode = _pNode;
        else if (SQL_ISRULE(_pNode,table_ref)
                &&  _pNode->count() == 3
                &&  SQL_ISPUNCTUATION(_pNode->getChild(0),"(")
                &&  SQL_ISPUNCTUATION(_pNode->getChild(2),")") ) // '(' joined_table ')'
            pJoinNode = _pNode->getChild(1);
        else if (! ( SQL_ISRULE(_pNode, table_ref) && _pNode->count() == 2) ) // table_node table_primary_as_range_column
            bRet = sal_False;

        if (pJoinNode && !InsertJoin(_pView,pJoinNode))
            bRet = sal_False;
        return bRet;
    }
    //-----------------------------------------------------------------------------
    sal_Bool InsertJoin(const OQueryDesignView* _pView,
                        const ::connectivity::OSQLParseNode *pNode)
    {
        DBG_ASSERT( SQL_ISRULE( pNode, qualified_join ) || SQL_ISRULE( pNode, joined_table ) || SQL_ISRULE( pNode, cross_union ),
            "OQueryDesignView::InsertJoin: Fehler im Parse Tree");

        if (SQL_ISRULE(pNode,joined_table))
            return InsertJoin(_pView,pNode->getChild(1));

        // first check the left and right side
        const ::connectivity::OSQLParseNode* pRightTableRef = pNode->getChild(3); // table_ref
        if ( SQL_ISRULE(pNode, qualified_join) && SQL_ISTOKEN(pNode->getChild(1),NATURAL) )
            pRightTableRef = pNode->getChild(4); // table_ref

        if ( !checkJoinConditions(_pView,pNode->getChild(0)) || !checkJoinConditions(_pView,pRightTableRef))
            return sal_False;

        // named column join wird später vieleicht noch implementiert
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
                if (SQL_ISRULE(pJoinType,join_type))       // eine Ebene tiefer
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
                    return sal_False;
            }
        }
        else if ( SQL_ISRULE(pNode, cross_union) )
        {
            eJoinType = CROSS_JOIN;
            pRightTableRef = pNode->getChild(pNode->count() - 1);
        }
        else
            return sal_False;

        if ( eJoinType == CROSS_JOIN || bNatural )
        {

            OQueryTableWindow*  pLeftWindow = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( getTableRange(_pView,pNode->getChild(0)) );
            OQueryTableWindow*  pRightWindow = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( getTableRange(_pView,pRightTableRef) );
            OSL_ENSURE(pLeftWindow && pRightWindow,"Table Windows could not be found!");
            if ( !pLeftWindow || !pRightWindow )
                return sal_False;

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


        return sal_True;
    }
    //------------------------------------------------------------------------------
    void insertUnUsedFields(OQueryDesignView* _pView,OSelectionBrowseBox* _pSelectionBrw)
    {
        // now we have to insert the fields which aren't in the statement
        OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
        OTableFields& rUnUsedFields = rController.getUnUsedFields();
        OTableFields::iterator aEnd = rUnUsedFields.end();
        for(OTableFields::iterator aIter = rUnUsedFields.begin();aIter != aEnd;++aIter)
            if(_pSelectionBrw->InsertField(*aIter,BROWSER_INVALIDID,sal_False,sal_False).isValid())
                (*aIter) = NULL;
        OTableFields().swap( rUnUsedFields );
    }

    //------------------------------------------------------------------------------
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

            if ( !rController.isEsacpeProcessing() ) // not allowed in this mode
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
            if ( pTableExp->getChild(6)->count() > 0 || pTableExp->getChild(7)->count() > 0 || pTableExp->getChild(8)->count() > 0)
            {
                eErrorCode = eStatementTooComplex;
                break;
            }

            Reference< XConnection> xConnection = rController.getConnection();
            if ( !xConnection.is() )
            {
                DBG_ERROR( "InitFromParseNodeImpl: no connection? no connection!" );
                break;
            }

            const OSQLTables& aMap = aIterator.getTables();
            ::comphelper::UStringMixLess aTmp(aMap.key_comp());
            ::comphelper::UStringMixEqual aKeyComp( aTmp.isCaseSensitive() );

            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            try
            {
                sal_Int32 nMax = xMetaData->getMaxTablesInSelect();
                if ( nMax && nMax < (sal_Int32)aMap.size() )
                {
                    eErrorCode = eTooManyTables;
                    break;
                }

                ::rtl::OUString sComposedName;
                ::rtl::OUString aQualifierName;
                ::rtl::OUString sAlias;

                OQueryTableView* pTableView = static_cast<OQueryTableView*>(_pView->getTableView());
                pTableView->clearLayoutInformation();
                OSQLTables::const_iterator aIter = aMap.begin();
                OSQLTables::const_iterator aEnd = aMap.end();
                for(;aIter != aEnd;++aIter)
                {
                    OSQLTable xTable = aIter->second;
                    Reference< XPropertySet > xTableProps( xTable, UNO_QUERY_THROW );

                    sAlias = aIter->first;

                    // check whether this is a query
                    Reference< XPropertySetInfo > xPSI = xTableProps->getPropertySetInfo();
                    bool bIsQuery = xPSI.is() && xPSI->hasPropertyByName( PROPERTY_COMMAND );

                    if ( bIsQuery )
                        OSL_VERIFY( xTableProps->getPropertyValue( PROPERTY_NAME ) >>= sComposedName );
                    else
                    {
                        sComposedName = ::dbtools::composeTableName( xMetaData, xTableProps, ::dbtools::eInDataManipulation, false, false, false );

                        // if the alias is the complete (composed) table, then shorten it
                        if ( aKeyComp( sComposedName, aIter->first ) )
                        {
                            ::rtl::OUString sCatalog, sSchema, sTable;
                            ::dbtools::qualifiedNameComponents( xMetaData, sComposedName, sCatalog, sSchema, sTable, ::dbtools::eInDataManipulation );
                            sAlias = sTable;
                        }
                    }

                    // find the existent window for this alias
                    OQueryTableWindow* pExistentWin = pTableView->FindTable( sAlias );
                    if ( !pExistentWin )
                    {
                        pTableView->AddTabWin( sComposedName, sAlias, sal_False );  // don't create data here
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
                OJoinTableView::OTableWindowMap aTableMap(*pTableView->GetTabWinMap());
                OJoinTableView::OTableWindowMap::iterator aIterTableMap = aTableMap.begin();
                OJoinTableView::OTableWindowMap::iterator aIterTableEnd = aTableMap.end();
                for(;aIterTableMap != aIterTableEnd;++aIterTableMap)
                {
                    if(aMap.find(aIterTableMap->second->GetComposedName())  == aMap.end() &&
                        aMap.find(aIterTableMap->first)                     == aMap.end())
                        pTableView->RemoveTabWin(aIterTableMap->second);
                }

                if ( eOk == (eErrorCode = FillOuterJoins(_pView,pTableExp->getChild(0)->getChild(1))) )
                {
                    // check if we have a distinct statement
                    if(SQL_ISTOKEN(pParseTree->getChild(1),DISTINCT))
                    {
                        rController.setDistinct(sal_True);
                        rController.InvalidateFeature(SID_QUERY_DISTINCT_VALUES);
                    }
                    if ( (eErrorCode = InstallFields(_pView,pParseTree, pTableView->GetTabWinMap())) == eOk )
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
                OSL_ASSERT(!"getMaxTablesInSelect!");
            }
        }
        while ( false );

        // Durch das Neuerzeugen wurden wieder Undo-Actions in den Manager gestellt
        rController.ClearUndoManager();
        _pSelectionBrw->Invalidate();
        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    /** fillSelectSubList
        @return
            <TRUE/> when columns could be inserted otherwise <FALSE/>
    */
    //------------------------------------------------------------------------------
    SqlParseError fillSelectSubList(    OQueryDesignView* _pView,
                                OJoinTableView::OTableWindowMap* _pTabList)
    {
        SqlParseError eErrorCode = eOk;
        sal_Bool bFirstField = sal_True;
        ::rtl::OUString sAsterix(RTL_CONSTASCII_USTRINGPARAM("*"));
        OJoinTableView::OTableWindowMap::iterator aIter = _pTabList->begin();
        OJoinTableView::OTableWindowMap::iterator aEnd = _pTabList->end();
        for(;aIter != aEnd && eOk == eErrorCode ;++aIter)
        {
            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);
            OTableFieldDescRef  aInfo = new OTableFieldDesc();
            if (pTabWin->ExistsField( sAsterix, aInfo ))
            {
                eErrorCode = _pView->InsertField(aInfo, sal_True, bFirstField);
                bFirstField = sal_False;
            }
        }
        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    SqlParseError InstallFields(OQueryDesignView* _pView,
                                const ::connectivity::OSQLParseNode* pNode,
                                OJoinTableView::OTableWindowMap* pTabList )
    {
        if( pNode==0 || !SQL_ISRULE(pNode,select_statement))
            return eNoSelectStatement;

        ::connectivity::OSQLParseNode* pParseTree = pNode->getChild(2); // selection
        sal_Bool bFirstField = sal_True;    // bei der Initialisierung muß auf alle Faelle das erste Feld neu aktiviert werden

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

            ::rtl::OUString aColumnName,aTableRange;
            for (sal_uInt32 i = 0; i < pParseTree->count() && eOk == eErrorCode ; ++i)
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
                    ::rtl::OUString aColumnAlias(rController.getParseIterator().getColumnAlias(pColumnRef)); // kann leer sein
                    pColumnRef = pColumnRef->getChild(0);
                    OTableFieldDescRef aInfo = new OTableFieldDesc();

                    if (    pColumnRef->count() == 3 &&
                            SQL_ISPUNCTUATION(pColumnRef->getChild(0),"(") &&
                            SQL_ISPUNCTUATION(pColumnRef->getChild(2),")")
                        )
                        pColumnRef = pColumnRef->getChild(1);

                    if (SQL_ISRULE(pColumnRef,column_ref))
                    {
                        InsertColumnRef(_pView,pColumnRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList);
                        eErrorCode = _pView->InsertField(aInfo, sal_True, bFirstField);
                        bFirstField = sal_False;
                    }
                    else if(SQL_ISRULEOR2(pColumnRef,general_set_fct ,set_fct_spec) ||
                            SQL_ISRULEOR2(pColumnRef,position_exp,extract_exp)      ||
                            SQL_ISRULEOR2(pColumnRef,fold,char_substring_fct)       ||
                            SQL_ISRULEOR2(pColumnRef,length_exp,char_value_fct))
                    {
                        ::rtl::OUString aColumns;
                        pColumnRef->parseNodeToPredicateStr(aColumns,
                                                            xConnection,
                                                            rController.getNumberFormatter(),
                                                            _pView->getLocale(),
                                                            static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                            &rController.getParser().getContext());
                        //pColumnRef->parseNodeToStr(   aColumns,
                        //                          xConnection,
                        //                          &rController.getParser().getContext(),
                        //                          sal_True,
                        //                          sal_True); // quote is to true because we need quoted elements inside the function

                        sal_Int32 nFunctionType = FKT_NONE;
                        ::connectivity::OSQLParseNode* pParamRef = NULL;
                        sal_Int32 nColumnRefPos = pColumnRef->count() - 2;
                        if ( nColumnRefPos >= 0 && static_cast<sal_uInt32>(nColumnRefPos) < pColumnRef->count() )
                            pParamRef = pColumnRef->getChild(nColumnRefPos);

                        if (    SQL_ISRULE(pColumnRef,general_set_fct)
                            &&  SQL_ISRULE(pParamRef,column_ref) )
                        {
                            // Parameter auf Columnref pr"ufen
                            InsertColumnRef(_pView,pParamRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList);
                        }
                        else if ( SQL_ISRULE(pColumnRef,general_set_fct) )
                        {
                            if ( pParamRef && pParamRef->getTokenValue().toChar() == '*' )
                            {
                                OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
                                OJoinTableView::OTableWindowMap::iterator aEnd  = pTabList->end();
                                for(;aIter != aEnd;++aIter)
                                {
                                    OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);
                                    if (pTabWin->ExistsField( ::rtl::OUString::createFromAscii("*"), aInfo ))
                                    {
                                        aInfo->SetAlias(String());
                                        aInfo->SetTable(String());
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                ::rtl::OUString sFieldName = aColumns;
                                if ( pParamRef )
                                {   // we got an aggregate function but without column name inside
                                    // so we set the whole argument of the function as field name
                                    nFunctionType |= FKT_NUMERIC;
                                    sFieldName = ::rtl::OUString();
                                    pParamRef->parseNodeToStr(  sFieldName,
                                                        xConnection,
                                                        &rController.getParser().getContext(),
                                                        sal_True,
                                                        sal_True); // quote is to true because we need quoted elements inside the function
                                }
                                aInfo->SetDataType(DataType::DOUBLE);
                                aInfo->SetFieldType(TAB_NORMAL_FIELD);
                                aInfo->SetField(sFieldName);
                            }
                            aInfo->SetTabWindow(NULL);
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
                            String aCol(aColumns);
                            aInfo->SetFunction(aCol.GetToken(0,'(').EraseTrailingChars(' '));
                        }
                        else
                            aInfo->SetFunctionType(nFunctionType|FKT_OTHER);

                        eErrorCode = _pView->InsertField(aInfo, sal_True, bFirstField);
                        bFirstField = sal_False;
                    }
                    else //if(SQL_ISRULE(pColumnRef,num_value_exp)  || SQL_ISRULE(pColumnRef,term))
                    {
                        ::rtl::OUString aColumns;
                        pColumnRef->parseNodeToStr( aColumns,
                                                    xConnection,
                                                    &rController.getParser().getContext(),
                                                    sal_True,
                                                    sal_True); // quote is to true because we need quoted elements inside the function

                        aInfo->SetTabWindow( NULL );

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

                        eErrorCode = _pView->InsertField(aInfo, sal_True, bFirstField);
                        bFirstField = sal_False;
                    }

                    break;
                }

                DBG_ERROR( "InstallFields: don't know how to interpret this parse node!" );

                } while ( false );
            }
        }
        else
            eErrorCode = eStatementTooComplex;

        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    SqlParseError GetOrderCriteria( OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pParseRoot )
    {
        SqlParseError eErrorCode = eOk;
        if (!pParseRoot->getChild(3)->getChild(ORDER_BY_CHILD_POS)->isLeaf())
        {
            ::connectivity::OSQLParseNode* pNode = pParseRoot->getChild(3)->getChild(ORDER_BY_CHILD_POS)->getChild(2);
            ::connectivity::OSQLParseNode* pParamRef = NULL;

            OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
            EOrderDir eOrderDir;
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            for( sal_uInt32 i=0 ; i<pNode->count() ; i++ )
            {
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
                        ::rtl::OUString aTableRange,aColumnName;
                        ::connectivity::OSQLParseTreeIterator& rParseIter = rController.getParseIterator();
                        rParseIter.getColumnRange( pArgument, aColumnName, aTableRange );

                        OTableFields& aList = rController.getTableFieldDesc();
                        OTableFields::iterator aIter = aList.begin();
                        OTableFields::iterator aEnd = aList.end();
                        for(;aIter != aEnd;++aIter)
                        {
                            OTableFieldDescRef pEntry = *aIter;
                            if(pEntry.isValid() && pEntry->GetFieldAlias() == aColumnName)
                                pEntry->SetOrderDir( eOrderDir );
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
                        ::rtl::OUString sCondition;
                        pArgument->parseNodeToPredicateStr(sCondition,
                                                            xConnection,
                                                            rController.getNumberFormatter(),
                                                            _pView->getLocale(),
                                                            static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                            &rController.getParser().getContext());
                        _pView->fillFunctionInfo(pArgument,sCondition,aDragLeft);
                        aDragLeft->SetFunctionType(FKT_OTHER);
                        aDragLeft->SetOrderDir(eOrderDir);
                        aDragLeft->SetVisible(sal_False);
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
    //------------------------------------------------------------------------------
    SqlParseError GetHavingCriteria(    OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot,
                            sal_uInt16& rLevel )
    {
        SqlParseError eErrorCode = eOk;
        if (!pSelectRoot->getChild(3)->getChild(3)->isLeaf())
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pSelectRoot->getChild(3)->getChild(3)->getChild(1),rLevel, sal_True);
        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    SqlParseError GetGroupCriteria( OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot )
    {
        SqlParseError eErrorCode = eOk;
        if (!pSelectRoot->getChild(3)->getChild(2)->isLeaf()) // opt_group_by_clause
        {
            OQueryController& rController = static_cast<OQueryController&>(_pView->getController());
            ::connectivity::OSQLParseNode* pGroupBy = pSelectRoot->getChild(3)->getChild(2)->getChild(2);

            for( sal_uInt32 i=0 ; i < pGroupBy->count() && eOk == eErrorCode; ++i )
            {
                OTableFieldDescRef aDragInfo = new OTableFieldDesc();
                ::connectivity::OSQLParseNode* pParamRef = NULL;
                ::connectivity::OSQLParseNode* pArgument = pGroupBy->getChild( i );
                if(SQL_ISRULE(pArgument,column_ref))
                {
                    if ( eOk == (eErrorCode = FillDragInfo(_pView,pArgument,aDragInfo)) )
                    {
                        aDragInfo->SetGroupBy(sal_True);
                        _pSelectionBrw->AddGroupBy(aDragInfo,i);
                    }
                }
                else if(SQL_ISRULE(pArgument, general_set_fct ) &&
                        SQL_ISRULE(pParamRef = pArgument->getChild(pArgument->count()-2),column_ref) &&
                        eOk == FillDragInfo(_pView,pParamRef,aDragInfo))
                {
                    aDragInfo->SetGroupBy(sal_True);
                    _pSelectionBrw->AddGroupBy( aDragInfo, i );
                }
                else if( SQL_ISRULE(pArgument, set_fct_spec ) )
                {
                    Reference< XConnection> xConnection = rController.getConnection();
                    if(xConnection.is())
                    {
                        ::rtl::OUString sGroupByExpression;
                        pArgument->parseNodeToStr(  sGroupByExpression,
                                                    xConnection,
                                                    &rController.getParser().getContext(),
                                                    sal_True,
                                                    sal_True); // quote is to true because we need quoted elements inside the function
                        _pView->fillFunctionInfo(pArgument,sGroupByExpression,aDragInfo);
                        aDragInfo->SetFunctionType(FKT_OTHER);
                        aDragInfo->SetGroupBy(sal_True);
                        aDragInfo->SetVisible(sal_False);
                        _pSelectionBrw->AddGroupBy( aDragInfo, i );
                    }
                    else
                        eErrorCode = eColumnNotFound;
                }
            }
        }
        return eErrorCode;
    }

    //------------------------------------------------------------------------------
    String getParseErrorMessage( SqlParseError _eErrorCode )
    {
        sal_uInt16 nResId;
        switch(_eErrorCode)
        {
            case eIllegalJoin:
                nResId = STR_QRY_ILLEGAL_JOIN;
                break;
            case eStatementTooLong:
                nResId = STR_QRY_TOO_LONG_STATEMENT;
                break;
            case eNoConnection:
                nResId = STR_QRY_SYNTAX;
                break;
            case eNoSelectStatement:
                nResId = STR_QRY_NOSELECT;
                break;
            case eColumnInLikeNotFound:
                nResId = STR_QRY_SYNTAX;
                break;
            case eNoColumnInLike:
                nResId = STR_QRY_SYNTAX;
                break;
            case eColumnNotFound:
                nResId = STR_QRY_SYNTAX;
                break;
            case eNativeMode:
                nResId = STR_QRY_NATIVE;
                break;
            case eTooManyTables:
                nResId = STR_QRY_TOO_MANY_TABLES;
                break;
            case eTooManyConditions:
                nResId = STR_QRY_TOOMANYCOND;
                break;
            case eTooManyColumns:
                nResId = STR_QRY_TOO_MANY_COLUMNS;
                break;
            case eStatementTooComplex:
                nResId = STR_QRY_TOOCOMPLEX;
                break;
            default:
                nResId = STR_QRY_SYNTAX;
                break;
        }
        ;
        return String( ModuleRes( nResId ) );
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
}
// end of anonymouse namespace
DBG_NAME(OQueryDesignView)

OQueryDesignView::OQueryDesignView( OQueryContainerWindow* _pParent,
                                    OQueryController& _rController,
                                    const Reference< XMultiServiceFactory >& _rFactory)
    :OQueryView( _pParent, _rController, _rFactory )
    ,m_aSplitter( this )
    ,m_eChildFocus(NONE)
    ,m_bInKeyEvent(sal_False)
    ,m_bInSplitHandler( sal_False )
{
    DBG_CTOR(OQueryDesignView,NULL);

    try
    {
        SvtSysLocale aSysLocale;
        m_aLocale = aSysLocale.GetLocaleData().getLocale();
        m_sDecimalSep = aSysLocale.GetLocaleData().getNumDecimalSep();
    }
    catch(Exception&)
    {
    }

    m_pSelectionBox = new OSelectionBrowseBox(this);

    setNoneVisbleRow(static_cast<OQueryController&>(getController()).getVisibleRows());
    m_pSelectionBox->Show();
    // Splitter einrichten
    m_aSplitter.SetSplitHdl(LINK(this, OQueryDesignView,SplitHdl));
    m_aSplitter.Show();

}
// -----------------------------------------------------------------------------
OQueryDesignView::~OQueryDesignView()
{
    if ( m_pTableView )
        ::dbaui::notifySystemWindow(this,m_pTableView,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    ::std::auto_ptr<Window> aTemp(m_pSelectionBox);
    m_pSelectionBox = NULL;

    DBG_DTOR(OQueryDesignView,NULL);
}
//------------------------------------------------------------------------------
IMPL_LINK( OQueryDesignView, SplitHdl, void*, /*p*/ )
{
    if (!getController().isReadOnly())
    {
        m_bInSplitHandler = sal_True;
        m_aSplitter.SetPosPixel( Point( m_aSplitter.GetPosPixel().X(),m_aSplitter.GetSplitPosPixel() ) );
        static_cast<OQueryController&>(getController()).setSplitPos(m_aSplitter.GetSplitPosPixel());
        static_cast<OQueryController&>(getController()).setModified( sal_True );
        Resize();
        m_bInSplitHandler = sal_True;
    }
    return 0L;
}
// -------------------------------------------------------------------------
void OQueryDesignView::Construct()
{
    m_pTableView = new OQueryTableView(m_pScrollWindow,this);
    ::dbaui::notifySystemWindow(this,m_pTableView,::comphelper::mem_fun(&TaskPaneList::AddWindow));
    OQueryView::Construct();
}
// -----------------------------------------------------------------------------
void OQueryDesignView::initialize()
{
    if(static_cast<OQueryController&>(getController()).getSplitPos() != -1)
    {
        m_aSplitter.SetPosPixel( Point( m_aSplitter.GetPosPixel().X(),static_cast<OQueryController&>(getController()).getSplitPos() ) );
        m_aSplitter.SetSplitPosPixel(static_cast<OQueryController&>(getController()).getSplitPos());
    }
    m_pSelectionBox->initialize();
    reset();
}
// -------------------------------------------------------------------------
void OQueryDesignView::resizeDocumentView(Rectangle& _rPlayground)
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
            nSplitPos = aPlaygroundSize.Height() - aSelectionBoxSize.Height() - m_aSplitter.GetSizePixel().Height();
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
                nSplitPos = aPlaygroundSize.Height() - m_aSplitter.GetSizePixel().Height() - aSelBoxSize.Height();

                // and if the box is smaller than the optimal size, try to do something about it
                Size aSelBoxOptSize = m_pSelectionBox->CalcOptimalSize( aPlaygroundSize );
                if ( aSelBoxOptSize.Height() > aSelBoxSize.Height() )
                {
                    nSplitPos = aPlaygroundSize.Height() - m_aSplitter.GetSizePixel().Height() - aSelBoxOptSize.Height();
                }

                static_cast< OQueryController& >(getController()).setSplitPos( nSplitPos );
            }
        }
    }

    // normalize the split pos
    Point   aSplitPos       = Point( _rPlayground.Left(), nSplitPos );
    Size    aSplitSize      = Size( _rPlayground.GetSize().Width(), m_aSplitter.GetSizePixel().Height() );

    if( ( aSplitPos.Y() + aSplitSize.Height() ) > ( aPlaygroundSize.Height() ))
        aSplitPos.Y() = aPlaygroundSize.Height() - aSplitSize.Height();

    if( aSplitPos.Y() <= aPlaygroundPos.Y() )
        aSplitPos.Y() = aPlaygroundPos.Y() + sal_Int32(aPlaygroundSize.Height() * 0.2);

    // position the table
    Size aTableViewSize(aPlaygroundSize.Width(), aSplitPos.Y() - aPlaygroundPos.Y());
    m_pScrollWindow->SetPosSizePixel(aPlaygroundPos, aTableViewSize);

    // position the selection browse box
    Point aPos( aPlaygroundPos.X(), aSplitPos.Y() + aSplitSize.Height() );
    m_pSelectionBox->SetPosSizePixel( aPos, Size( aPlaygroundSize.Width(), aPlaygroundSize.Height() - aSplitSize.Height() - aTableViewSize.Height() ));

    // set the size of the splitter
    m_aSplitter.SetPosSizePixel( aSplitPos, aSplitSize );
    m_aSplitter.SetDragRectPixel( _rPlayground );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}
// -----------------------------------------------------------------------------
void OQueryDesignView::setReadOnly(sal_Bool _bReadOnly)
{
    m_pSelectionBox->SetReadOnly(_bReadOnly);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::clear()
{
    m_pSelectionBox->ClearAll(); // clear the whole selection
    m_pTableView->ClearAll();
}
// -----------------------------------------------------------------------------
void OQueryDesignView::setStatement(const ::rtl::OUString& /*_rsStatement*/)
{
}
// -----------------------------------------------------------------------------
void OQueryDesignView::copy()
{
    if( m_eChildFocus == SELECTION)
        m_pSelectionBox->copy();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isCutAllowed()
{
    sal_Bool bAllowed = sal_False;
    if ( SELECTION == m_eChildFocus )
        bAllowed = m_pSelectionBox->isCutAllowed();
    return bAllowed;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isPasteAllowed()
{
    sal_Bool bAllowed = sal_False;
    if ( SELECTION == m_eChildFocus )
        bAllowed = m_pSelectionBox->isPasteAllowed();
    return bAllowed;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isCopyAllowed()
{
    sal_Bool bAllowed = sal_False;
    if ( SELECTION == m_eChildFocus )
        bAllowed = m_pSelectionBox->isCopyAllowed();
    return bAllowed;
}
// -----------------------------------------------------------------------------
void OQueryDesignView::stopTimer()
{
    m_pSelectionBox->stopTimer();
}
// -----------------------------------------------------------------------------
void OQueryDesignView::startTimer()
{
    m_pSelectionBox->startTimer();
}
// -----------------------------------------------------------------------------
void OQueryDesignView::cut()
{
    if( m_eChildFocus == SELECTION)
    {
        m_pSelectionBox->cut();
        static_cast<OQueryController&>(getController()).setModified(sal_True);
    }
}
// -----------------------------------------------------------------------------
void OQueryDesignView::paste()
{
    if( m_eChildFocus == SELECTION)
    {
        m_pSelectionBox->paste();
        static_cast<OQueryController&>(getController()).setModified(sal_True);
    }
}
// -----------------------------------------------------------------------------
void OQueryDesignView::TableDeleted(const ::rtl::OUString& rAliasName)
{
    // Nachricht, dass Tabelle aus dem Fenster gel"oscht wurde
    DeleteFields(rAliasName);
    static_cast<OQueryController&>(getController()).InvalidateFeature(ID_BROWSER_ADDTABLE); // view nochmal bescheid sagen
}
//------------------------------------------------------------------------------
void OQueryDesignView::DeleteFields( const ::rtl::OUString& rAliasName )
{
    m_pSelectionBox->DeleteFields( rAliasName );
}
// -----------------------------------------------------------------------------
bool OQueryDesignView::HasFieldByAliasName(const ::rtl::OUString& rFieldName, OTableFieldDescRef& rInfo)  const
{
    return m_pSelectionBox->HasFieldByAliasName( rFieldName, rInfo);
}
// -----------------------------------------------------------------------------
SqlParseError OQueryDesignView::InsertField( const OTableFieldDescRef& rInfo, sal_Bool bVis, sal_Bool bActivate)
{
    return m_pSelectionBox->InsertField( rInfo, BROWSER_INVALIDID,bVis, bActivate ).isValid() ? eOk : eTooManyColumns;
}
// -----------------------------------------------------------------------------
sal_Int32 OQueryDesignView::getColWidth(sal_uInt16 _nColPos) const
{
    static sal_Int32 s_nDefaultWidth = GetTextWidth(String(RTL_CONSTASCII_USTRINGPARAM("0"))) * 15;
    sal_Int32 nWidth = static_cast<OQueryController&>(getController()).getColWidth(_nColPos);
    if ( !nWidth )
        nWidth = s_nDefaultWidth;
    return nWidth;
}
//------------------------------------------------------------------------------
void OQueryDesignView::fillValidFields(const ::rtl::OUString& sAliasName, ComboBox* pFieldList)
{
    DBG_ASSERT(pFieldList != NULL, "OQueryDesignView::FillValidFields : What the hell do you think I can do with a NULL-ptr ? This will crash !");
    pFieldList->Clear();

    sal_Bool bAllTables = sAliasName.getLength() == 0;

    OJoinTableView::OTableWindowMap* pTabWins = m_pTableView->GetTabWinMap();
    ::rtl::OUString strCurrentPrefix;
    ::std::vector< ::rtl::OUString> aFields;
    OJoinTableView::OTableWindowMap::iterator aIter = pTabWins->begin();
    OJoinTableView::OTableWindowMap::iterator aEnd  = pTabWins->end();
    for(;aIter != aEnd;++aIter)
    {
        OQueryTableWindow* pCurrentWin = static_cast<OQueryTableWindow*>(aIter->second);
        if (bAllTables || (pCurrentWin->GetAliasName() == sAliasName))
        {
            strCurrentPrefix = pCurrentWin->GetAliasName();
            strCurrentPrefix += ::rtl::OUString('.');

            pCurrentWin->EnumValidFields(aFields);

            ::std::vector< ::rtl::OUString>::iterator aStrIter = aFields.begin();
            ::std::vector< ::rtl::OUString>::iterator aStrEnd = aFields.end();
            for(;aStrIter != aStrEnd;++aStrIter)
            {
                if (bAllTables || aStrIter->toChar() == '*')
                    pFieldList->InsertEntry(::rtl::OUString(strCurrentPrefix) += *aStrIter);
                else
                    pFieldList->InsertEntry(*aStrIter);
            }

            if (!bAllTables)
                // das heisst, dass ich in diesen Block kam, weil der Tabellenname genau der gesuchte war, also bin ich fertig
                // (dadurch verhindere ich auch das doppelte Einfuegen von Feldern, wenn eine Tabelle mehrmals als TabWin vorkommt)
                break;
        }
    }
}
// -----------------------------------------------------------------------------
long OQueryDesignView::PreNotify(NotifyEvent& rNEvt)
{
    switch (rNEvt.GetType())
    {
        case EVENT_GETFOCUS:
#if OSL_DEBUG_LEVEL > 0
            {
                Window* pFocus = Application::GetFocusWindow();
                (void)pFocus;
            }
#endif

            if ( m_pSelectionBox && m_pSelectionBox->HasChildPathFocus() )
                m_eChildFocus = SELECTION;
            else
                m_eChildFocus = TABLEVIEW;
            break;
    }

    return OQueryView::PreNotify(rNEvt);
}
//------------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// check if the statement is correct when not returning false
sal_Bool OQueryDesignView::checkStatement()
{
    sal_Bool bRet = sal_True;
    if ( m_pSelectionBox )
        bRet = m_pSelectionBox->Save(); // a error occurred so we return no
    return bRet;
}
//-------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::getStatement()
{
    OQueryController& rController = static_cast<OQueryController&>(getController());
    m_rController.clearError();
    // used for fields which aren't any longer in the statement
    OTableFields& rUnUsedFields = rController.getUnUsedFields();
    OTableFields().swap( rUnUsedFields );

    // create the select columns
    sal_uInt32 nFieldcount = 0;
    OTableFields& rFieldList = rController.getTableFieldDesc();
    OTableFields::iterator aIter = rFieldList.begin();
    OTableFields::iterator aEnd = rFieldList.end();
    for(;aIter != aEnd;++aIter)
    {
        OTableFieldDescRef pEntryField = *aIter;
        if ( pEntryField->GetField().getLength() && pEntryField->IsVisible() )
            ++nFieldcount;
        else if (pEntryField->GetField().getLength()            &&
                !pEntryField->HasCriteria()                 &&
                pEntryField->isNoneFunction()               &&
                pEntryField->GetOrderDir() == ORDER_NONE    &&
                !pEntryField->IsGroupBy()                   &&
                !pEntryField->GetFunction().getLength() )
            rUnUsedFields.push_back(pEntryField);
    }
    if ( !nFieldcount ) // keine Felder sichtbar also zur"uck
    {
        rUnUsedFields = rFieldList;
        return ::rtl::OUString();
    }

    OQueryTableView::OTableWindowMap* pTabList   = m_pTableView->GetTabWinMap();
    sal_uInt32 nTabcount        = pTabList->size();

    ::rtl::OUString aFieldListStr(GenerateSelectList(this,rFieldList,nTabcount>1));
    if( !aFieldListStr.getLength() )
        return ::rtl::OUString();
    // Ausnahmebehandlung, wenn keine Felder angegeben worden sind
    // Dann darf die Tabpage nicht gewechselt werden
    // Im TabBarSelectHdl wird der SQL-::rtl::OUString auf STATEMENT_NOFIELDS abgefragt
    // und eine Errormeldung erzeugt
    // ----------------- Tabellenliste aufbauen ----------------------

    const ::std::vector<OTableConnection*>* pConnList = m_pTableView->getTableConnections();
    Reference< XConnection> xConnection = rController.getConnection();
    ::rtl::OUString aTableListStr(GenerateFromClause(xConnection,pTabList,pConnList));
    DBG_ASSERT(aTableListStr.getLength(), "OQueryDesignView::getStatement() : unerwartet : habe Felder, aber keine Tabellen !");
    // wenn es Felder gibt, koennen die nur durch Einfuegen aus einer schon existenten Tabelle entstanden sein; wenn andererseits
    // eine Tabelle geloescht wird, verschwinden auch die zugehoerigen Felder -> ergo KANN es das nicht geben, dass Felder
    // existieren, aber keine Tabellen (und aFieldListStr hat schon eine Laenge, das stelle ich oben sicher)
    ::rtl::OUStringBuffer aHavingStr,aCriteriaListStr;
    // ----------------- Kriterien aufbauen ----------------------
    if (!GenerateCriterias(this,aCriteriaListStr,aHavingStr,rFieldList, nTabcount > 1))
        return ::rtl::OUString();

    ::rtl::OUString aJoinCrit;
    GenerateInnerJoinCriterias(xConnection,aJoinCrit,pConnList);
    if(aJoinCrit.getLength())
    {
        ::rtl::OUString aTmp = ::rtl::OUString::createFromAscii("( ");
        aTmp += aJoinCrit;
        aTmp += ::rtl::OUString::createFromAscii(" )");
        if(aCriteriaListStr.getLength())
        {
            aTmp += C_AND;
            aTmp += aCriteriaListStr.makeStringAndClear();
        }
        aCriteriaListStr = aTmp;
    }
    // ----------------- Statement aufbauen ----------------------
    ::rtl::OUStringBuffer aSqlCmd(::rtl::OUString::createFromAscii("SELECT "));
    if(static_cast<OQueryController&>(getController()).isDistinct())
        aSqlCmd.append(::rtl::OUString::createFromAscii(" DISTINCT "));
    aSqlCmd.append(aFieldListStr);
    aSqlCmd.append(::rtl::OUString::createFromAscii(" FROM "));
    aSqlCmd.append(aTableListStr);

    if (aCriteriaListStr.getLength())
    {
        aSqlCmd.append(::rtl::OUString::createFromAscii(" WHERE "));
        aSqlCmd.append(aCriteriaListStr.makeStringAndClear());
    }
    // ----------------- GroupBy aufbauen und Anh"angen ------------
    Reference<XDatabaseMetaData> xMeta;
    if ( xConnection.is() )
        xMeta = xConnection->getMetaData();
    sal_Bool bUseAlias = nTabcount > 1;
    if ( xMeta.is() )
        bUseAlias = bUseAlias || !xMeta->supportsGroupByUnrelated();

    aSqlCmd.append(GenerateGroupBy(this,rFieldList,bUseAlias));
    // ----------------- having Anh"angen ------------
    if(aHavingStr.getLength())
    {
        aSqlCmd.append(::rtl::OUString::createFromAscii(" HAVING "));
        aSqlCmd.append(aHavingStr.makeStringAndClear());
    }
    // ----------------- Sortierung aufbauen und Anh"angen ------------
    ::rtl::OUString sOrder;
    SqlParseError eErrorCode = eOk;
    if ( (eErrorCode = GenerateOrder(this,rFieldList,nTabcount > 1,sOrder)) == eOk)
        aSqlCmd.append(sOrder);
    else
    {
        if ( !m_rController.hasError() )
            m_rController.appendError( getParseErrorMessage( eErrorCode ) );

        m_rController.displayError();
    }

    ::rtl::OUString sSQL = aSqlCmd.makeStringAndClear();
    if ( xConnection.is() )
    {
        ::connectivity::OSQLParser& rParser( rController.getParser() );
        ::rtl::OUString sErrorMessage;
        ::std::auto_ptr<OSQLParseNode> pParseNode( rParser.parseTree( sErrorMessage, sSQL, sal_True ) );
        if ( pParseNode.get() )
        {
            OSQLParseNode* pNode = pParseNode->getChild(3)->getChild(1);
            if ( pNode->count() > 1 )
            {
                ::connectivity::OSQLParseNode * pCondition = pNode->getChild(1);
                if ( pCondition ) // no where clause
                {
                    OSQLParseNode::compress(pCondition);
                    ::rtl::OUString sTemp;
                    pParseNode->parseNodeToStr(sTemp,xConnection);
                    sSQL = sTemp;
                }
            }
        }
    }
    return sSQL;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void OQueryDesignView::setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable)
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
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isSlotEnabled(sal_Int32 _nSlotId)
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
// -----------------------------------------------------------------------------
void OQueryDesignView::SaveUIConfig()
{
    OQueryController& rCtrl = static_cast<OQueryController&>(getController());
    rCtrl.SaveTabWinsPosSize( m_pTableView->GetTabWinMap(), m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos() );
    //  rCtrl.SaveTabFieldsWidth( m_pSelectionBox );
    rCtrl.setVisibleRows( m_pSelectionBox->GetNoneVisibleRows() );
    if ( m_aSplitter.GetSplitPosPixel() != 0 )
        rCtrl.setSplitPos( m_aSplitter.GetSplitPosPixel() );
}
// -----------------------------------------------------------------------------
OSQLParseNode* OQueryDesignView::getPredicateTreeFromEntry(OTableFieldDescRef pEntry,
                                                           const String& _sCriteria,
                                                           ::rtl::OUString& _rsErrorMessage,
                                                           Reference<XPropertySet>& _rxColumn) const
{
    OSL_ENSURE(pEntry.isValid(),"Entry is null!");
    if(!pEntry.isValid())
        return NULL;
    Reference< XConnection> xConnection = static_cast<OQueryController&>(getController()).getConnection();
    if(!xConnection.is())
        return NULL;

    ::connectivity::OSQLParser& rParser( static_cast<OQueryController&>(getController()).getParser() );
    OQueryTableWindow* pWin = static_cast<OQueryTableWindow*>(pEntry->GetTabWindow());

    String sTest(_sCriteria);
    // special handling for functions
    if ( pEntry->GetFunctionType() & (FKT_OTHER | FKT_AGGREGATE | FKT_NUMERIC) )
    {
        // we have a function here so we have to distinguish the type of return value
        String sFunction;
        if ( pEntry->isNumericOrAggreateFunction() )
            sFunction = pEntry->GetFunction();

        if ( !sFunction.Len() )
            sFunction = pEntry->GetField();

        if(sFunction.GetTokenCount('(') > 1)
            sFunction = sFunction.GetToken(0,'('); // this should be the name of the function

        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionReturnType(sFunction,&rParser.getContext());
        if ( nType == DataType::OTHER || (!sFunction.Len() && pEntry->isNumericOrAggreateFunction()) )
        {
            // first try the international version
            ::rtl::OUString sSql;
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * "));
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM x WHERE "));
            sSql += pEntry->GetField();
            sSql += _sCriteria;
            ::std::auto_ptr<OSQLParseNode> pParseNode( rParser.parseTree( _rsErrorMessage, sSql, sal_True ) );
            nType = DataType::DOUBLE;
            if ( pParseNode.get() )
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
                                                                ::rtl::OUString(),
                                                                ::rtl::OUString(),
                                                                ::rtl::OUString(),
                                                                ColumnValue::NULLABLE_UNKNOWN,
                                                                0,
                                                                0,
                                                                nType,
                                                                sal_False,
                                                                sal_False,
                                                                xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());
        _rxColumn = pColumn;
        pColumn->setFunction(sal_True);
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

    OSQLParseNode* pParseNode = rParser.predicateTree(  _rsErrorMessage,
                                                        sTest,
                                                        static_cast<OQueryController&>(getController()).getNumberFormatter(),
                                                        _rxColumn);
    return pParseNode;
}
// -----------------------------------------------------------------------------
void OQueryDesignView::GetFocus()
{
    OQueryView::GetFocus();
    if ( m_pSelectionBox && !m_pSelectionBox->HasChildPathFocus() )
    {
        // first we have to deactivate the current cell to refill when necessary
        m_pSelectionBox->DeactivateCell();
        m_pSelectionBox->ActivateCell(m_pSelectionBox->GetCurRow(), m_pSelectionBox->GetCurColumnId());
        m_pSelectionBox->GrabFocus();
    }
}
// -----------------------------------------------------------------------------
void OQueryDesignView::reset()
{
    m_pTableView->ClearAll();
    m_pTableView->ReSync();
}
// -----------------------------------------------------------------------------
void OQueryDesignView::setNoneVisbleRow(sal_Int32 _nRows)
{
    m_pSelectionBox->SetNoneVisbleRow(_nRows);
}

// -----------------------------------------------------------------------------
void OQueryDesignView::initByFieldDescriptions( const Sequence< PropertyValue >& i_rFieldDescriptions )
{
    OQueryController& rController = static_cast< OQueryController& >( getController() );

    m_pSelectionBox->PreFill();
    m_pSelectionBox->SetReadOnly( rController.isReadOnly() );
    m_pSelectionBox->Fill();

    for (   const PropertyValue* field = i_rFieldDescriptions.getConstArray();
            field != i_rFieldDescriptions.getConstArray() + i_rFieldDescriptions.getLength();
            ++field
        )
    {
        ::vos::ORef< OTableFieldDesc > pField( new OTableFieldDesc() );
        pField->Load( *field, true );
        InsertField( pField, sal_True, sal_False );
    }

    rController.ClearUndoManager();
    m_pSelectionBox->Invalidate();
}

// -----------------------------------------------------------------------------
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
        DBG_UNHANDLED_EXCEPTION();
    }
    return eErrorCode == eOk;
}
//------------------------------------------------------------------------------
void OQueryDesignView::fillFunctionInfo(  const ::connectivity::OSQLParseNode* pNode
                                        ,const ::rtl::OUString& sFunctionTerm
                                        ,OTableFieldDescRef& aInfo)
{
    // get the type out of the function name
    OQueryController& rController = static_cast<OQueryController&>(getController());
    sal_Int32 nDataType = DataType::DOUBLE;
    ::rtl::OUString sFieldName = sFunctionTerm;
    OSQLParseNode* pFunctionName = pNode->getChild(0);
    if ( !SQL_ISPUNCTUATION(pFunctionName,"{") )
    {
        if ( SQL_ISRULEOR2(pNode,length_exp,char_value_fct) )
            pFunctionName = pFunctionName->getChild(0);

        ::rtl::OUString sFunctionName = pFunctionName->getTokenValue();
        if ( !sFunctionName.getLength() )
            sFunctionName = ::rtl::OStringToOUString(OSQLParser::TokenIDToStr(pFunctionName->getTokenID()),RTL_TEXTENCODING_UTF8);

        nDataType = OSQLParser::getFunctionReturnType(
                            sFunctionName
                            ,&rController.getParser().getContext());
    }
    aInfo->SetDataType(nDataType);
    aInfo->SetFieldType(TAB_NORMAL_FIELD);
    aInfo->SetField(sFieldName);
    aInfo->SetTabWindow(NULL);
}
// -----------------------------------------------------------------------------
