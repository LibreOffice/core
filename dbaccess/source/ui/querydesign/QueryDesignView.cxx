/*************************************************************************
 *
 *  $RCSfile: QueryDesignView.cxx,v $
 *
 *  $Revision: 1.68 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:12:50 $
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
#include <svtools/undo.hxx>
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
#ifndef DBAUI_TABLEFIELDDATA_HXX
#include "TableFieldData.hxx"
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
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif

using namespace ::dbaui;
using namespace ::utl;
using namespace ::connectivity;
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
                                    int &rLevel );

    SqlParseError GetOrderCriteria( OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const ::connectivity::OSQLParseNode* pParseRoot );

    SqlParseError AddFunctionCondition(OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const ::connectivity::OSQLParseNode * pCondition,
                                    const int nLevel,
                                    sal_Bool bHaving);

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
    ::rtl::OUString ParseCondition( OQueryController* pController
                                    ,const ::connectivity::OSQLParseNode* pCondition
                                    ,const ::rtl::OUString _sDecimal
                                    ,const ::com::sun::star::lang::Locale& _rLocale
                                    ,sal_uInt32 _nStartIndex)
    {
        ::rtl::OUString aCondition;
        Reference< XConnection> xConnection = pController->getConnection();
        if ( xConnection.is() )
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            sal_uInt32 nCount = pCondition->count();
            for(sal_uInt32 i = _nStartIndex ; i < nCount ; ++i)
                pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                xMetaData,
                                pController->getNumberFormatter(),
                                _rLocale,
                                static_cast<sal_Char>(_sDecimal.toChar()),
                                &pController->getParser()->getContext());
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

            if ( SQL_ISRULEOR2(pParseNode , qualified_join,joined_table) || SQL_ISRULE(pParseNode ,cross_union) )
                pJoinNode = pParseNode;
            else if(    pParseNode->count() == 4
                    &&  SQL_ISPUNCTUATION(pParseNode->getChild(0),"{")
                    &&  SQL_ISRULE(pParseNode,table_ref))
                pJoinNode = pParseNode->getChild(2);

            if ( pJoinNode )
            {
                if ( !InsertJoin(_pView,pJoinNode) )
                    bError = sal_True;
            }
        }
        // check if error occured
        if ( bError )
            eErrorCode = eIllegalJoin;

        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString QuoteField( const OQueryDesignView* _pView,const ::rtl::OUString& rValue, sal_Int32 aType )
    {
        ::rtl::OUString rNewValue;
        switch (rValue.toChar())
        {
            case '?':
                if (rValue.getLength() != 1)
                    break;
            case '\'':  // ::rtl::OUString Quotierung oder Datum
            //case '#': // Datumsquotierung // jetengine
            case ':':   // Parameter
            case '[':   // Parameter
                return rValue;
        }

        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        Reference< XDatabaseMetaData >  xMetaData;
        if(xConnection.is())
            xMetaData = xConnection->getMetaData();
        ::rtl::OUString aQuote;
        try
        {
            if(xMetaData.is())
                aQuote = xMetaData->getIdentifierQuoteString();

            switch( aType )
            {
                case DataType::DATE:
                case DataType::TIME:
                case DataType::TIMESTAMP:
                    if (rValue.toChar() != '{') // nur quoten, wenn kein Access Datum
                        rNewValue = ::dbtools::quoteName(aQuote,rValue);
                    else
                        rNewValue = rValue;
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                    rNewValue = ::dbtools::quoteName(aQuote,rValue);
                    break;
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                case DataType::BIGINT:
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                    rNewValue = rValue;
                    break;
                case DataType::BIT:
                    {
                        if(xMetaData.is())
                        {
                            ::comphelper::UStringMixEqual bCase(xMetaData->storesMixedCaseQuotedIdentifiers());
                            if (bCase(rValue, ::rtl::OUString(ModuleRes(STR_QUERY_TRUE))))
                                rNewValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TRUE"));
                            else if (bCase(rValue, ::rtl::OUString(ModuleRes(STR_QUERY_FALSE))))
                                rNewValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FALSE"));
                            else
                                rNewValue = rValue;
                        }
                    }
                    break;
                default:
                    DBG_ERROR( "QuoteField: illegal type" );
                break;
            }
        }
        catch(SQLException&)
        {
            DBG_ERROR( "QuoteField: Exception" );
        }
        return rNewValue;
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
        ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController*>(_pView->getController())->getParseIterator();
        rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

        if ( aTableRange.getLength() )
        {
            OQueryTableWindow*  pSTW = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( aTableRange );
            bErg = (pSTW && pSTW->ExistsField( aColumnName, _rDragInfo ) );
        }
        if ( !bErg )
            bErg = static_cast<OQueryTableView*>(_pView->getTableView())->FindTableFromField(aColumnName, _rDragInfo, nCntAccount);
        if ( !bErg )
        {
            eErrorCode = eColumnNotFound;
            String sError(ModuleRes(STR_QRY_COLUMN_NOT_FOUND));
            sError.SearchAndReplaceAscii("$name$",aColumnName);
            _pView->getController()->appendError(SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY0000") ),1000,Any()));

            try
            {
                Reference<XDatabaseMetaData> xMeta = _pView->getController()->getConnection()->getMetaData();
                if ( xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers() )
                    _pView->getController()->appendError(SQLException(String(ModuleRes(STR_QRY_CHECK_CASESENSITIVE)),NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY0000") ),1000,Any()));
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
        ::rtl::OUString aCondition;
        if ( !_xConnection.is() )
            return aCondition;

        OConnectionLineDataVec::iterator aIter = pLineDataList->begin();
        try
        {
            Reference< XDatabaseMetaData >  xMetaData = _xConnection->getMetaData();
            ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

            for(;aIter != pLineDataList->end();++aIter)
            {
                OConnectionLineDataRef pLineData = *aIter;
                if(pLineData->IsValid())
                {
                    if(aCondition.getLength())
                        aCondition += C_AND;
                    aCondition += quoteTableAlias(sal_True,pData->GetAliasName(JTCS_FROM),aQuote);
                    aCondition += ::dbtools::quoteName(aQuote, pLineData->GetFieldName(JTCS_FROM) );
                    aCondition += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" = "));
                    aCondition += quoteTableAlias(sal_True,pData->GetAliasName(JTCS_TO),aQuote);
                    aCondition += ::dbtools::quoteName(aQuote, pLineData->GetFieldName(JTCS_TO) );
                }
            }
        }
        catch(SQLException&)
        {
            OSL_ASSERT(!"Failure while building Join criteria!");
        }

        return aCondition;
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

        OQueryTableConnectionData* pData = static_cast< OQueryTableConnectionData*>(_pEntryConn->GetData());
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
                                const OQueryTableWindow* pEntryTab
                                )
    {
        ::rtl::OUString aDBName(pEntryTab->GetComposedName());

        //  Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if( _xConnection.is() )
        {
            try
            {
                Reference< XDatabaseMetaData >  xMetaData = _xConnection->getMetaData();

                sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(_xConnection,PROPERTY_USECATALOGINSELECT,sal_True);
                sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(_xConnection,PROPERTY_USESCHEMAINSELECT,sal_True);
                ::rtl::OUString aTableListStr = ::dbtools::quoteTableName(xMetaData,aDBName,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);

                ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
                if ( isAppendTableAliasEnabled(_xConnection) )
                {
                    aTableListStr += ::rtl::OUString(' ');
                    aTableListStr += ::dbtools::quoteName(aQuote, pEntryTab->GetAliasName());
                }
                aDBName = aTableListStr;
            }
            catch(SQLException&)
            {
                OSL_ENSURE(0,"Exception catched while building table table for query!");
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
        switch(pData->GetJoinType())
        {
            case LEFT_JOIN:
                aErg.AppendAscii(" LEFT OUTER ");
                break;
            case RIGHT_JOIN:
                aErg.AppendAscii(" RIGHT OUTER ");
                break;
            case INNER_JOIN:
                DBG_ERROR("OQueryDesignView::BuildJoin: This should not happen!");
                //aErg.AppendAscii(" INNER ");
                break;
            default:
                aErg.AppendAscii(" FULL OUTER ");
                break;
        }
        aErg.AppendAscii("JOIN ");
        aErg += String(rRh);
        aErg.AppendAscii(" ON ");
        aErg += String(BuildJoinCriteria(_xConnection,pData->GetConnLineDataList(),pData));

        return aErg;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const Reference< XConnection>& _xConnection,
                                OQueryTableWindow* pLh,
                                OQueryTableWindow* pRh,
                                OQueryTableConnectionData* pData
                                )
    {
        return BuildJoin(_xConnection,BuildTable(_xConnection,pLh),BuildTable(_xConnection,pRh),pData);
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
        OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData());
        if(pEntryConnData->GetJoinType() == INNER_JOIN)
            return;

        //  Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();

        if(!aJoin.getLength())
        {
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            aJoin = BuildJoin(_xConnection,pEntryTabFrom,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetDestWin())
        {
            ::rtl::OUString aTmpJoin('(');
            (aTmpJoin += aJoin) += ::rtl::OUString(')');
            aJoin = BuildJoin(_xConnection,aTmpJoin,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetSourceWin())
        {
            ::rtl::OUString aTmpJoin('(');
            (aTmpJoin += aJoin) += ::rtl::OUString(')');
            aJoin = BuildJoin(_xConnection,pEntryTabTo,aTmpJoin,pEntryConnData);
        }

        pEntryConn->SetVisited(sal_True);

        // first search for the "to" window
        const ::std::vector<OTableConnection*>* pConnections = pEntryConn->GetParent()->getTableConnections();
        ::std::vector<OTableConnection*>::const_iterator aIter = pConnections->begin();
        for(;aIter != pConnections->end();++aIter)
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
        if(aIter == pConnections->end())
        {
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            aIter = pConnections->begin();
            for(;aIter != pConnections->end();++aIter)
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
                                    const EJoinType& _eJoinType)
    {
        SqlParseError eErrorCode = eOk;
        if (pNode->count() == 3 &&  // Ausdruck is geklammert
            SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pNode->getChild(2),")"))
        {
            eErrorCode = InsertJoinConnection(_pView,pNode->getChild(1), _eJoinType);
        }
        else if (SQL_ISRULEOR2(pNode,search_condition,boolean_term) &&          // AND/OR-Verknuepfung:
                 pNode->count() == 3)
        {
            // nur AND Verknüpfung zulassen
            if (!SQL_ISTOKEN(pNode->getChild(1),AND))
                eErrorCode = eIllegalJoinCondition;
            else if ( eOk == (eErrorCode = InsertJoinConnection(_pView,pNode->getChild(0), _eJoinType)) )
                    eErrorCode = InsertJoinConnection(_pView,pNode->getChild(2), _eJoinType);
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
                _pView->getController()->appendError(SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY0000") ),1000,Any()));
                return eIllegalJoin;
            }

            OTableFieldDescRef aDragLeft  = new OTableFieldDesc();
            OTableFieldDescRef aDragRight = new OTableFieldDesc();
            if ( eOk != ( eErrorCode = FillDragInfo(_pView,pNode->getChild(0),aDragLeft)) ||
                eOk != ( eErrorCode = FillDragInfo(_pView,pNode->getChild(2),aDragRight)))
                return eErrorCode;

            OQueryTableView* pTableView = static_cast<OQueryTableView*>(_pView->getTableView());
            OQueryTableConnection* pConn = static_cast<OQueryTableConnection*>( pTableView->GetTabConn(static_cast<OQueryTableWindow*>(aDragLeft->GetTabWindow()),static_cast<OQueryTableWindow*>(aDragRight->GetTabWindow())));
            if(!pConn)
            {
                OQueryTableConnectionData aInfoData;
                aInfoData.InitFromDrag(aDragLeft, aDragRight);
                aInfoData.SetJoinType(_eJoinType);

                OQueryTableConnection aInfo(pTableView, &aInfoData);
                // da ein OQueryTableConnection-Objekt nie den Besitz der uebergebenen Daten uebernimmt, sondern sich nur den Zeiger merkt,
                // ist dieser Zeiger auf eine lokale Variable hier unkritisch, denn aInfoData und aInfo haben die selbe Lebensdauer
                pTableView->NotifyTabConnection( aInfo );
            }
            else
            {
                ::rtl::OUString aSourceFieldName(aDragLeft->GetField());
                ::rtl::OUString aDestFieldName(aDragRight->GetField());
                // the connection could point on the other side
                if(pConn->GetSourceWin() == aDragRight->GetTabWindow())
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
                pConn->Invalidate();
            }

        }
        else
            eErrorCode = eIllegalJoin;
        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    sal_Bool GetInnerJoinCriteria(  const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode *pCondition)
    {
        return InsertJoinConnection(_pView,pCondition, INNER_JOIN) != eOk;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString GenerateSelectList( const OQueryDesignView* _pView,
                                        OTableFields&   _rFieldList,
                                        sal_Bool bAlias)
    {
        ::rtl::OUString aTmpStr,aFieldListStr;

        sal_Bool bAsterix = sal_False;
        int nVis = 0;
        OTableFields::iterator aIter = _rFieldList.begin();
        for(;aIter != _rFieldList.end();++aIter)
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

        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if(!xConnection.is())
            return aFieldListStr;

        try
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

            OJoinTableView::OTableWindowMap* pTabList = _pView->getTableView()->GetTabWinMap();

            const static ::rtl::OUString sFieldSeparator(RTL_CONSTASCII_USTRINGPARAM(", "));

            aIter = _rFieldList.begin();
            for(;aIter != _rFieldList.end();++aIter)
            {
                OTableFieldDescRef pEntryField = *aIter;
                ::rtl::OUString rFieldName = pEntryField->GetField();
                if ( rFieldName.getLength() && pEntryField->IsVisible() )
                {
                    aTmpStr = ::rtl::OUString();
                    ::rtl::OUString rAlias = pEntryField->GetAlias();
                    ::rtl::OUString rFieldAlias = pEntryField->GetFieldAlias();

                    aTmpStr += quoteTableAlias((bAlias || bAsterix),rAlias,aQuote);

                    // if we have a none numeric field, the table alias could be in the name
                    // otherwise we are not allowed to do this (e.g. 0.1 * PRICE )
                    if  ( !pEntryField->isNumeric() )
                    {
                        // we have to look if we have alias.* here but before we have to check if the column doesn't already exist
                        String sTemp = rFieldName;
                        OTableFieldDescRef  aInfo = new OTableFieldDesc();
                        OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
                        sal_Bool bFound = sal_False;
                        for(;!bFound && aIter != pTabList->end() ;++aIter)
                        {
                            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);

                            if ( bFound = pTabWin->ExistsField( rFieldName, aInfo ) )
                            {
                                rFieldName = aInfo->GetField();
                            }
                        }
                        if ( rFieldName.toChar() != '*' )
                        {
                            OSL_ENSURE(pEntryField->GetTable().getLength(),"No table field name!");
                            aTmpStr += ::dbtools::quoteName(aQuote, rFieldName);
                        }
                        else
                            aTmpStr += rFieldName;
                    }
                    else
                        aTmpStr += rFieldName;

                    if  ( pEntryField->isAggreateFunction() )
                    {
                        DBG_ASSERT(pEntryField->GetFunction().getLength(),"Functionname darf hier nicht leer sein! ;-(");
                        ::rtl::OUString aTmpStr2( pEntryField->GetFunction());
                        aTmpStr2 +=  ::rtl::OUString('(');
                        aTmpStr2 += aTmpStr;
                        aTmpStr2 +=  ::rtl::OUString(')');
                        aTmpStr = aTmpStr2;
                    }

                    if (rFieldAlias.getLength()                         &&
                        (rFieldName.toChar() != '*'                     ||
                        pEntryField->isNumericOrAggreateFunction()      ||
                        pEntryField->isOtherFunction()))
                    {
                        aTmpStr += ::rtl::OUString::createFromAscii(" AS ");
                        aTmpStr += ::dbtools::quoteName(aQuote, rFieldAlias);
                    }
                    aFieldListStr += aTmpStr;
                    aFieldListStr += sFieldSeparator;
                }
            }
            if(aFieldListStr.getLength())
                aFieldListStr = aFieldListStr.replaceAt(aFieldListStr.getLength()-2,2, ::rtl::OUString() );
        }
        catch(SQLException&)
        {
            OSL_ASSERT(!"Failure while building select list!");
        }
        return aFieldListStr;
    }
    //------------------------------------------------------------------------------
    sal_Bool GenerateCriterias( OQueryDesignView* _pView,
                                ::rtl::OUString& rRetStr,
                                ::rtl::OUString& rHavingStr,
                                OTableFields& _rFieldList,
                                sal_Bool bMulti )
    {
        // * darf keine Filter enthalten : habe ich die entsprechende Warnung schon angezeigt ?
        sal_Bool bCritsOnAsterikWarning = sal_False;        // ** TMFS **

        ::rtl::OUString aFieldName,aCriteria,aWhereStr,aHavingStr,aWork/*,aOrderStr*/;
        // Zeilenweise werden die Ausdr"ucke mit AND verknuepft
        sal_uInt16 nMaxCriteria = 0;
        OTableFields::iterator aIter = _rFieldList.begin();
        for(;aIter != _rFieldList.end();++aIter)
        {
            nMaxCriteria = ::std::max<sal_uInt16>(nMaxCriteria,(*aIter)->GetCriteria().size());
        }
        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if(!xConnection.is())
            return FALSE;
        try
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
            const IParseContext& rContext = static_cast<OQueryController*>(_pView->getController())->getParser()->getContext();

            for (sal_uInt16 i=0 ; i < nMaxCriteria ; i++)
            {
                aHavingStr = aWhereStr = ::rtl::OUString();

                for(aIter = _rFieldList.begin();aIter != _rFieldList.end();++aIter)
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
                            ::connectivity::OSQLParseNode* pParseNode = _pView->getPredicateTreeFromEntry(pEntryField,aTmp,aErrorMsg,xColumn);
                            if (pParseNode)
                            {
                                if (bMulti && !(pEntryField->isOtherFunction() || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(pEntryField->GetAlias(),aFieldName);
                                ::rtl::OUString sHavingStr = aHavingStr;

                                sal_uInt32 nCount = pParseNode->count();
                                for( sal_uInt32 i = 1 ; i < nCount ; ++i)
                                    pParseNode->getChild(i)->parseNodeToStr(    sHavingStr,
                                                                xMetaData,
                                                                &rContext,
                                                                sal_False,
                                                                !pEntryField->isOtherFunction());
                                aHavingStr = sHavingStr;
                                delete pParseNode;
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
                            ::connectivity::OSQLParseNode* pParseNode = _pView->getPredicateTreeFromEntry(pEntryField,aTmp,aErrorMsg,xColumn);
                            if (pParseNode)
                            {
                                if (bMulti && !(pEntryField->isOtherFunction() || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(pEntryField->GetAlias(),aFieldName);
                                ::rtl::OUString aWhere = aWhereStr;
                                pParseNode->parseNodeToStr( aWhere,
                                                            xMetaData,
                                                            &rContext,
                                                            sal_False,
                                                            !pEntryField->isOtherFunction() );
                                aWhereStr = aWhere;
                                delete pParseNode;
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
                        rRetStr += C_OR;
                    else                                        // Klammern auf fuer 'OR' Zweig
                        rRetStr += ::rtl::OUString('(');
                    rRetStr += aWhereStr;
                }
                if (aHavingStr.getLength())
                {
                    aHavingStr += ::rtl::OUString(')');                     // Klammern zu fuer 'AND' Zweig
                    if (rHavingStr.getLength())                         // schon Feldbedingungen ?
                        rHavingStr += C_OR;
                    else                                        // Klammern auf fuer 'OR' Zweig
                        rHavingStr += ::rtl::OUString('(');
                    rHavingStr += aHavingStr;
                }
            }

            if (rRetStr.getLength())
                rRetStr += ::rtl::OUString(')');                                // Klammern zu fuer 'OR' Zweig
            if (rHavingStr.getLength())
                rHavingStr += ::rtl::OUString(')');                             // Klammern zu fuer 'OR' Zweig
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
        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if ( !xConnection.is() )
            return eNoConnection;

        SqlParseError eErrorCode = eOk;

        ::rtl::OUString aColumnName;
        ::rtl::OUString aWorkStr;
        try
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
            // * darf keine Filter enthalten : habe ich die entsprechende Warnung schon angezeigt ?
            sal_Bool bCritsOnAsterikWarning = sal_False;        // ** TMFS **
            OTableFields::iterator aIter = _rFieldList.begin();

            for(;aIter != _rFieldList.end();++aIter)
            {
                OTableFieldDescRef  pEntryField = *aIter;
                EOrderDir eOrder = pEntryField->GetOrderDir();

                // nur wenn eine Sortierung und ein Tabellenname vorhanden ist-> erzeugen
                // sonst werden die Expressions vom Order By im GenerateCriteria mit erzeugt
                if (eOrder != ORDER_NONE && pEntryField->GetTable().getLength())
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

                    if ( pEntryField->GetFieldAlias().getLength() )
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
                    else
                    {
                        aWorkStr += quoteTableAlias(bMulti,pEntryField->GetAlias(),aQuote);
                        aWorkStr += ::dbtools::quoteName(aQuote, aColumnName);
                    }
                    aWorkStr += ::rtl::OUString(' ');
                    aWorkStr += String::CreateFromAscii( ";ASC;DESC" ).GetToken( eOrder );
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
                sal_Int32 nMaxOrder = xMetaData->getMaxColumnsInOrderBy();
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

    ::rtl::OUString BuildACriteria( const OQueryDesignView* _pView,const ::rtl::OUString& _rVal, sal_Int32 aType )
    {

        ::rtl::OUString aRetStr;
        String  aVal,rVal;
        String aOpList;aOpList.AssignAscii("<>;>=;<=;<;>;=;LIKE");
        xub_StrLen  nOpListCnt = aOpList.GetTokenCount();

        String aToken;
        for( xub_StrLen nIdx=0 ; nIdx < nOpListCnt ; nIdx++ )
        {
            aToken = aOpList.GetToken(nIdx);
            if (rVal.Search( aToken ) == 0)
            {
                aRetStr = ::rtl::OUString(' ');
                aRetStr += aToken;
                aRetStr += ::rtl::OUString(' ');
                aVal = rVal.Copy( aToken.Len() );
                aVal.EraseLeadingChars( ' ' );
                if( aVal.Search( '\'' ) == STRING_NOTFOUND )//XXX O'Brien???
                {
                    aVal = QuoteField(_pView, aVal, aType );
                }
                aRetStr += aVal;
                break;
            }
        }

        if( !aRetStr.getLength()) //  == 0
        {
            aRetStr = rVal.Search( '%' ) == STRING_NOTFOUND ? ::rtl::OUString::createFromAscii(" = ") : ::rtl::OUString::createFromAscii(" LIKE ");
            aVal = rVal;
            if( aVal.Search( '\'' ) == STRING_NOTFOUND )//XXX O'Brien???
            {
                aVal = QuoteField( _pView,aVal, aType );
            }
            aRetStr += aVal;
        }

        return aRetStr;//XXX
    }
    //------------------------------------------------------------------------------
    void GenerateInnerJoinCriterias(const Reference< XConnection>& _xConnection,
                                    ::rtl::OUString& _rJoinCrit,
                                    const ::std::vector<OTableConnection*>* _pConnList)
    {
        ::std::vector<OTableConnection*>::const_iterator aIter = _pConnList->begin();
        for(;aIter != _pConnList->end();++aIter)
        {
            const OQueryTableConnection* pEntryConn = static_cast<const OQueryTableConnection*>(*aIter);
            OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData());
            if(pEntryConnData->GetJoinType() == INNER_JOIN)
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
                             map< ::rtl::OUString,sal_Bool,::comphelper::UStringMixLess>& _rTableNames,
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

        // generate outer join clause in from
        if(!pConnList->empty())
        {
            ::std::vector<OTableConnection*>::const_iterator aIter = pConnList->begin();
            for(;aIter != pConnList->end();++aIter)
                static_cast<OQueryTableConnection*>(*aIter)->SetVisited(sal_False);

            aIter = pConnList->begin();
            for(;aIter != pConnList->end();++aIter)
            {
                OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>(*aIter);
                if(!pEntryConn->IsVisited())
                {
                    ::rtl::OUString aJoin;
                    GetNextJoin(_xConnection,pEntryConn,static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()),aJoin);

                    if(aJoin.getLength())
                    {
                        sal_Bool bUseEscape = ::dbtools::isDataSourcePropertyEnabled(_xConnection,PROPERTY_OUTERJOINESCAPE,sal_True);
                        ::rtl::OUString aStr;
                        if ( bUseEscape )
                            aStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("{ OJ "));
                        aStr += aJoin;
                        if ( bUseEscape )
                            aStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" }"));
                        aStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                        aTableListStr += aStr;
                    }
                }
            }

            // and now all inner joins
            map< ::rtl::OUString,sal_Bool,::comphelper::UStringMixLess> aTableNames;
            aIter = pConnList->begin();
            for(;aIter != pConnList->end();++aIter)
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
        for(;aTabIter != pTabList->end();++aTabIter)
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

        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if(!xConnection.is())
            return ::rtl::OUString();

        ::rtl::OUString aGroupByStr;
        try
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

            OTableFields::iterator aIter = _rFieldList.begin();
            for(;aIter != _rFieldList.end();++aIter)
            {
                OTableFieldDescRef  pEntryField = *aIter;
                if ( pEntryField->IsGroupBy() )
                {
                    DBG_ASSERT(pEntryField->GetField().getLength(),"Kein FieldName vorhanden!;-(");
                    aGroupByStr += quoteTableAlias(bMulti,pEntryField->GetAlias(),aQuote);

                    // only quote the field name when it isn't calculated
                    if ( pEntryField->isNoneFunction() )
                        aGroupByStr += ::dbtools::quoteName(aQuote, pEntryField->GetField());
                    else
                        aGroupByStr += pEntryField->GetField();
                    aGroupByStr += ::rtl::OUString(',');
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
                                int& nLevel ,
                                sal_Bool bHaving = sal_False);
    // -----------------------------------------------------------------------------
    SqlParseError GetSelectionCriteria( OQueryDesignView* _pView,
                                        OSelectionBrowseBox* _pSelectionBrw,
                                        const ::connectivity::OSQLParseNode* pNode,
                                        int& rLevel,
                                        sal_Bool bJoinWhere = sal_False)
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

            // first extract the inner joins conditions
            GetInnerJoinCriteria(_pView,pNodeTmp);

            // it could happen that pCondition is not more valid
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pNodeTmp, rLevel);
        }
        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    SqlParseError GetANDCriteria(   OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const  ::connectivity::OSQLParseNode * pCondition,
                                    const int nLevel,
                                    sal_Bool bHaving );
    //------------------------------------------------------------------------------
    SqlParseError ComparisonPredicate(OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const int nLevel,
                            sal_Bool bHaving );
    //------------------------------------------------------------------------------
    SqlParseError GetORCriteria(OQueryDesignView* _pView,
                                OSelectionBrowseBox* _pSelectionBrw,
                                const ::connectivity::OSQLParseNode * pCondition,
                                int& nLevel ,
                                sal_Bool bHaving)
    {
        SqlParseError eErrorCode = eOk;

        // Runde Klammern um den Ausdruck
        if (pCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
        {
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(1),nLevel,bHaving);
        }
        // oder Verknuepfung
        // a searchcondition can only look like this: search_condition SQL_TOKEN_OR boolean_term
        else if (SQL_ISRULE(pCondition,search_condition))
        {
            for (int i = 0; i < 3 && eErrorCode == eOk ; i+=2)
            {
                if ( SQL_ISRULE(pCondition->getChild(i),search_condition) )
                    eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(i),nLevel,bHaving);
                else
                    eErrorCode = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(i), nLevel++,bHaving);
            }
        }
        else
            eErrorCode = GetANDCriteria( _pView,_pSelectionBrw,pCondition, nLevel, bHaving );

        return eErrorCode;
    }
    //--------------------------------------------------------------------------------------------------
    SqlParseError GetANDCriteria(   OQueryDesignView* _pView,
                                    OSelectionBrowseBox* _pSelectionBrw,
                                    const  ::connectivity::OSQLParseNode * pCondition,
                                    const int nLevel,
                                    sal_Bool bHaving )
    {
        ::com::sun::star::lang::Locale  aLocale = _pView->getLocale();
        ::rtl::OUString sDecimal = _pView->getDecimalSeparator();

        // ich werde ein paar Mal einen gecasteten Pointer auf meinen ::com::sun::star::sdbcx::Container brauchen
        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());
        SqlParseError eErrorCode = eOk;

        // Runde Klammern
        if (SQL_ISRULE(pCondition,boolean_primary))
        {
            int nLevel2 = nLevel;
            eErrorCode = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(1), nLevel2,bHaving );
        }
        // Das erste Element ist (wieder) eine AND-Verknuepfung
        else if ( SQL_ISRULE(pCondition,boolean_term) && pCondition->count() == 3 )
        {
            eErrorCode = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(0), nLevel,bHaving );
            if ( eErrorCode == eOk )
                eErrorCode = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(2), nLevel,bHaving );
        }
        else if (SQL_ISRULE( pCondition, comparison_predicate))
        {
            eErrorCode = ComparisonPredicate(_pView,_pSelectionBrw,pCondition,nLevel,bHaving);
        }
        else if( SQL_ISRULE(pCondition,like_predicate) )
        {
            if (SQL_ISRULE(pCondition->getChild(0), column_ref ) )
            {
                ::rtl::OUString aColumnName;
                ::rtl::OUString aCondition;
                Reference< XConnection> xConnection = pController->getConnection();
                if ( xConnection.is() )
                {
                    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                    // the international doesn't matter I have a string
                    pCondition->parseNodeToPredicateStr(aCondition,
                                                        xMetaData,
                                                        pController->getNumberFormatter(),
                                                        aLocale,
                                                        static_cast<sal_Char>(sDecimal.toChar()),
                                                        &pController->getParser()->getContext());

                    pCondition->getChild(0)->parseNodeToPredicateStr(   aColumnName,
                                                                        xMetaData,
                                                                        pController->getNumberFormatter(),
                                                                        aLocale,
                                                                        static_cast<sal_Char>(sDecimal.toChar()),
                                                                        &pController->getParser()->getContext());

                    // don't display the column name
                    aCondition = aCondition.copy(aColumnName.getLength());
                    aCondition = aCondition.trim();
                }

                OTableFieldDescRef aDragLeft = new OTableFieldDesc();
                if ( eOk == ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(0),aDragLeft) ))
                    _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
            }
            else
            {
                eErrorCode = eNoColumnInLike;
                String sError(ModuleRes(STR_QRY_LIKE_LEFT_NO_COLUMN));
                _pView->getController()->appendError(SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY0000") ),1000,Any()));
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
                                        bHaving);
            }
            else if ( SQL_ISRULE(pCondition->getChild(0), column_ref ) )
            {
                // parse condition
                ::rtl::OUString sCondition = ParseCondition(pController,pCondition,sDecimal,aLocale,1);
                OTableFieldDescRef  aDragLeft = new OTableFieldDesc();
                if ( eOk == ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(0),aDragLeft)) )
                _pSelectionBrw->AddCondition(aDragLeft, sCondition, nLevel);
            }
        }
        else if( SQL_ISRULEOR2(pCondition,existence_test,unique_test) )
        {

            // Funktions-Bedingung parsen
            ::rtl::OUString aCondition = ParseCondition(pController,pCondition,sDecimal,aLocale,0);

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
                            const int nLevel,
                            sal_Bool bHaving)
    {
        SqlParseError eErrorCode = eOk;
        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());
        OSL_ENSURE(SQL_ISRULEOR2(pCondition->getChild(0), set_fct_spec , general_set_fct ),"Illegal call!");
        ::rtl::OUString aCondition;
        OTableFieldDescRef aDragLeft = new OTableFieldDesc();

        OSQLParseNode* pFunction = pCondition->getChild(0);

        ::rtl::OUString aColumnName;
        Reference< XConnection> xConnection = pController->getConnection();
        if(xConnection.is())
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            pCondition->parseNodeToPredicateStr(aCondition,
                                                xMetaData,
                                                pController->getNumberFormatter(),
                                                _pView->getLocale(),
                                                static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                &pController->getParser()->getContext());

            pFunction->parseNodeToPredicateStr(aColumnName,
                                                xMetaData,
                                                pController->getNumberFormatter(),
                                                _pView->getLocale(),
                                                static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                &pController->getParser()->getContext());
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
                    for(;aIter != pTabList->end();++aIter)
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
                                                xConnection->getMetaData(),
                                                &pController->getParser()->getContext());
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
            _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
        }

        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    SqlParseError ComparisonPredicate(OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const int nLevel,
                            sal_Bool bHaving )
    {
        SqlParseError eErrorCode = eOk;
        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());

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

                OQueryTableConnection* pConn = static_cast<OQueryTableConnection*>(_pView->getTableView()->GetTabConn(static_cast<OQueryTableWindow*>(aDragLeft->GetTabWindow()),
                                                                                                            static_cast<OQueryTableWindow*>(aDragRight->GetTabWindow())));
                if(pConn)
                {
                    OConnectionLineDataVec* pLineDataList = pConn->GetData()->GetConnLineDataList();
                    OConnectionLineDataVec::iterator aIter = pLineDataList->begin();
                    for(;aIter != pLineDataList->end();++aIter)
                    {
                        if((*aIter)->GetSourceFieldName() == aDragLeft->GetField() ||
                           (*aIter)->GetDestFieldName() == aDragLeft->GetField() )
                            break;
                    }
                    if(aIter != pLineDataList->end())
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
                aCondition = ParseCondition(pController
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
                }

                // go backward
                Reference< XConnection> xConnection = pController->getConnection();
                if(xConnection.is())
                {
                    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                    for (; i >= 0; i--)
                        pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                                xMetaData,
                                                pController->getNumberFormatter(),
                                                _pView->getLocale(),
                                                static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                &pController->getParser()->getContext());
                }
            }
            // else ???


            if( eOk == ( eErrorCode = FillDragInfo(_pView,pCondition->getChild(nPos),aDragLeft)))
            {
                if(bHaving)
                    aDragLeft->SetGroupBy(sal_True);
                _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
            }
        }
        else if( SQL_ISRULEOR2(pCondition->getChild(0), set_fct_spec , general_set_fct ) )
        {
            AddFunctionCondition(   _pView,
                                    _pSelectionBrw,
                                    pCondition,
                                    nLevel,
                                    bHaving);
        }
        else // kann sich nur um einen Expr. Ausdruck handeln
        {
            ::rtl::OUString aName,aCondition;

            ::connectivity::OSQLParseNode *pLhs = pCondition->getChild(0);
            ::connectivity::OSQLParseNode *pRhs = pCondition->getChild(2);
            // Feldnamen
            Reference< XConnection> xConnection = pController->getConnection();
            if(xConnection.is())
            {
                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                pLhs->parseNodeToStr(aName,
                                            xMetaData,
                                            &pController->getParser()->getContext(),
                                            sal_True);
                // Kriterium
                aCondition = pCondition->getChild(1)->getTokenValue();
                pRhs->parseNodeToPredicateStr(aCondition,
                                                            xMetaData,
                                                            pController->getNumberFormatter(),
                                                            _pView->getLocale(),
                                                            static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                            &pController->getParser()->getContext());
            }

            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            aDragLeft->SetField(aName);
            aDragLeft->SetFunctionType(FKT_OTHER|FKT_NUMERIC);
            // und anh"angen
            _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
        }
        return eErrorCode;
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
        ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController*>(_pView->getController())->getParseIterator();
        rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

        sal_Bool bFound(sal_False);
        DBG_ASSERT(aColumnName.getLength(),"Columnname darf nicht leer sein");
        if (!aTableRange.getLength())
        {
            // SELECT column, ...
            OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
            for(;aIter != pTabList->end();++aIter)
            {
                OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);
                if (pTabWin && pTabWin->ExistsField( aColumnName, _raInfo ) )
                {
                    if(aColumnName.toChar() != '*')
                        _raInfo->SetFieldAlias(aColumnAlias);
                    bFound = sal_True;
                    break;
                }
            }
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
        else if (SQL_ISRULE(_pNode, joined_table))
            pJoinNode = _pNode->getChild(1);
        else if (! ( SQL_ISRULE(_pNode, table_ref) &&
                     (SQL_ISRULEOR2(_pNode->getChild(0), catalog_name, schema_name)  ||
                      SQL_ISRULE(_pNode->getChild(0), table_name))))
            bRet = sal_False;

        if (pJoinNode && !InsertJoin(_pView,pJoinNode))
            bRet = sal_False;
        return bRet;
    }
    //-----------------------------------------------------------------------------
    sal_Bool InsertJoin(const OQueryDesignView* _pView,
                        const ::connectivity::OSQLParseNode *pNode)
    {
        DBG_ASSERT(SQL_ISRULE(pNode, qualified_join) || SQL_ISRULE(pNode, joined_table),
            "OQueryDesignView::InsertJoin: Fehler im Parse Tree");

        if (SQL_ISRULE(pNode,joined_table))
            return InsertJoin(_pView,pNode->getChild(1));

        // first check the left and right side
        if ( !checkJoinConditions(_pView,pNode->getChild(0)) || !checkJoinConditions(_pView,pNode->getChild(3)))
            return sal_False;

        // named column join wird später vieleicht noch implementiert
        // SQL_ISRULE(pNode->getChild(4),named_columns_join)
        if (SQL_ISRULE(pNode->getChild(4),join_condition))
        {
            EJoinType eJoinType;
            ::connectivity::OSQLParseNode* pJoinType = pNode->getChild(1); // join_type
            if (SQL_ISRULE(pJoinType,join_type) && SQL_ISTOKEN(pJoinType->getChild(0),INNER))
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
            if ( InsertJoinConnection(_pView,pNode->getChild(4)->getChild(1), eJoinType) != eOk )
                return sal_False;
        }
        else
            return sal_False;

        return sal_True;
    }
    //------------------------------------------------------------------------------
    void insertUnUsedFields(OQueryDesignView* _pView,OSelectionBrowseBox* _pSelectionBrw)
    {
        // now we have to insert the fields which aren't in the statement
        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());
        OTableFields& rUnUsedFields = pController->getUnUsedFields();
        for(OTableFields::iterator aIter = rUnUsedFields.begin();aIter != rUnUsedFields.end();++aIter)
            if(_pSelectionBrw->InsertField(*aIter,BROWSER_INVALIDID,sal_False,sal_False).isValid())
                (*aIter) = NULL;
        OTableFields().swap( rUnUsedFields );
    }
    //------------------------------------------------------------------------------
    SqlParseError InitFromParseNodeImpl(OQueryDesignView* _pView,OSelectionBrowseBox* _pSelectionBrw)
    {
        SqlParseError eErrorCode = eOk;

        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());

        _pSelectionBrw->PreFill();
        _pSelectionBrw->SetReadOnly(pController->isReadOnly());
        _pSelectionBrw->Fill();


        ::connectivity::OSQLParseTreeIterator& aIterator = pController->getParseIterator();
        const ::connectivity::OSQLParseNode* pParseTree = aIterator.getParseTree();
        const ::connectivity::OSQLParseNode* pTableRefCommaList = 0;

        if (pParseTree)
        {
            if (!pController->isEsacpeProcessing()) // not allowed in this mode
                eErrorCode = eNativeMode;
            else if (SQL_ISRULE(pParseTree,select_statement))
            {
                ::connectivity::OSQLParseNode* pTree = pParseTree->getChild(1);

                const OSQLTables& aMap = aIterator.getTables();
                ::comphelper::UStringMixLess aTmp(aMap.key_comp());
                ::comphelper::UStringMixEqual aKeyComp(static_cast< ::comphelper::UStringMixLess*>(&aTmp)->isCaseSensitive());

                Reference< XConnection> xConnection = pController->getConnection();
                if(xConnection.is())
                {
                    sal_Int32 nMax = 0;
                    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                    try
                    {
                        nMax = xMetaData->getMaxTablesInSelect();

                        if(!nMax || nMax >= (sal_Int32)aMap.size()) // Anzahl der Tabellen im Select-Statement "uberpr"ufen
                        {
                            ::rtl::OUString sComposedName;
                            ::rtl::OUString aQualifierName;
                            ::rtl::OUString sAlias;

                            OQueryTableView* pTableView = static_cast<OQueryTableView*>(_pView->getTableView());
                            OSQLTables::const_iterator aIter = aMap.begin();
                            for(;aIter != aMap.end();++aIter)
                            {
                                OSQLTable xTable = aIter->second;
                                sComposedName = ::dbtools::composeTableName(xMetaData,Reference<XPropertySet>(xTable,UNO_QUERY),sal_False,::dbtools::eInDataManipulation);
                                sAlias = aIter->first;
                                if(aKeyComp(sComposedName,aIter->first))
                                {
                                    ::rtl::OUString sCatalog,sSchema,sTable;
                                    ::dbtools::qualifiedNameComponents(xMetaData,sComposedName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
                                    sAlias = sTable;
                                }


                                OQueryTableWindow* pExistentWin = pTableView->FindTable(sAlias);
                                if (!pExistentWin)
                                {
                                    pTableView->AddTabWin(sComposedName, sAlias,sal_False);// don't create data here
                                }
                                else
                                {
                                    // es existiert schon ein Fenster mit dem selben Alias ...
                                    if (!aKeyComp(pExistentWin->GetData()->GetComposedName(),sComposedName))
                                        // ... aber anderem Tabellennamen -> neues Fenster
                                        pTableView->AddTabWin(sComposedName, sAlias);
                                }
                            }

                            // now delete the data for which we haven't any tablewindow
                            OJoinTableView::OTableWindowMap* pTableMap = pTableView->GetTabWinMap();
                            OJoinTableView::OTableWindowMap::iterator aIterTableMap = pTableMap->begin();
                            for(;aIterTableMap != pTableMap->end();++aIterTableMap)
                            {
                                if(aMap.find(aIterTableMap->second->GetComposedName())  == aMap.end() &&
                                   aMap.find(aIterTableMap->first)                      == aMap.end())
                                    pTableView->RemoveTabWin(aIterTableMap->second);
                            }

                            if ( eOk == (eErrorCode = FillOuterJoins(_pView,pParseTree->getChild(3)->getChild(0)->getChild(1))) )
                            {
                                // check if we have a distinct statement
                                if(SQL_ISTOKEN(pParseTree->getChild(1),DISTINCT))
                                {
                                    pController->setDistinct(sal_True);
                                    pController->InvalidateFeature(ID_BROWSER_QUERY_DISTINCT_VALUES);
                                }
                                if ( (eErrorCode = InstallFields(_pView,pParseTree, pTableView->GetTabWinMap())) == eOk )
                                {
                                    // GetSelectionCriteria mu"s vor GetHavingCriteria aufgerufen werden
                                    int nLevel=0;

                                    if ( eOk == (eErrorCode = GetSelectionCriteria(_pView,_pSelectionBrw,pParseTree,nLevel,sal_True)) )
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
                        else
                            eErrorCode = eTooManyTables;
                    }
                    catch(SQLException&)
                    {
                        OSL_ASSERT(!"getMaxTablesInSelect!");
                    }
                }
            }
            else
                eErrorCode = eNoSelectStatement;
        }
        else
        {
            // now we have to insert the fields which aren't in the statement
            insertUnUsedFields(_pView,_pSelectionBrw);
        }

        // Durch das Neuerzeugen wurden wieder Undo-Actions in den Manager gestellt
        pController->getUndoMgr()->Clear();
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
        OTableFieldDescRef  aInfo = new OTableFieldDesc();
        OJoinTableView::OTableWindowMap::iterator aIter = _pTabList->begin();
        for(;aIter != _pTabList->end() && eOk == eErrorCode ;++aIter)
        {
            OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);

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

        ::connectivity::OSQLParseNode* pParseTree = pNode->getChild(2);
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
            OQueryController* pController = static_cast<OQueryController*>(_pView->getController());
            Reference< XConnection> xConnection = pController->getConnection();

            ::rtl::OUString aColumnName,aTableRange;
            for (sal_uInt32 i = 0; i < pParseTree->count() && eOk == eErrorCode ; ++i)
            {
                ::connectivity::OSQLParseNode * pColumnRef = pParseTree->getChild(i);

                if ( SQL_ISRULE(pColumnRef,select_sublist) )
                {
                     eErrorCode = fillSelectSubList(_pView,pTabList);
                }
                else if ( SQL_ISRULE(pColumnRef,derived_column) )
                {
                    if ( xConnection.is() )
                    {
                        Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                        ::rtl::OUString aColumnAlias(pController->getParseIterator().getColumnAlias(pColumnRef)); // kann leer sein
                        pColumnRef = pColumnRef->getChild(0);
                        OTableFieldDescRef aInfo = new OTableFieldDesc();

                        if (
                                pColumnRef->count() == 3 &&
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
                            pColumnRef->parseNodeToStr( aColumns,
                                                        xMetaData,
                                                        &pController->getParser()->getContext(),
                                                        sal_True,
                                                        sal_True); // quote is to true because we need quoted elements inside the function

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
                                    for(;aIter != pTabList->end();++aIter)
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
                                                            xMetaData,
                                                            &pController->getParser()->getContext(),
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
                                // get the type out of the funtion name
                                nFunctionType |= FKT_NUMERIC;
                                sal_Int32 nDataType = DataType::DOUBLE;
                                ::rtl::OUString sFieldName = aColumns;
                                OSQLParseNode* pFunctionName = pColumnRef->getChild(0);
                                if ( !SQL_ISPUNCTUATION(pFunctionName,"{") )
                                {
                                    if ( SQL_ISRULEOR2(pColumnRef,length_exp,char_value_fct) )
                                        pFunctionName = pFunctionName->getChild(0);

                                    ::rtl::OUString sFunctionName = pFunctionName->getTokenValue();
                                    if ( !sFunctionName.getLength() )
                                        sFunctionName = ::rtl::OStringToOUString(OSQLParser::TokenIDToStr(pFunctionName->getTokenID()),RTL_TEXTENCODING_UTF8);

                                    nDataType = OSQLParser::getFunctionReturnType(
                                                        sFunctionName
                                                        ,&pController->getParser()->getContext());
                                }
                                aInfo->SetDataType(nDataType);
                                aInfo->SetFieldType(TAB_NORMAL_FIELD);
                                aInfo->SetField(sFieldName);
                                aInfo->SetTabWindow(NULL);
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
                                                        xMetaData,
                                                        &pController->getParser()->getContext(),
                                                        sal_True,
                                                        sal_False);

                            aInfo->SetDataType(DataType::DOUBLE);
                            aInfo->SetFieldType(TAB_NORMAL_FIELD);
                            aInfo->SetTabWindow(NULL);
                            aInfo->SetField(aColumns);
                            aInfo->SetFieldAlias(aColumnAlias);
                            aInfo->SetFunctionType(FKT_NUMERIC | FKT_OTHER);

                            eErrorCode = _pView->InsertField(aInfo, sal_True, bFirstField);
                            bFirstField = sal_False;
                        }
                    }
                }
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
        if (!pParseRoot->getChild(3)->getChild(4)->isLeaf())
        {
            ::connectivity::OSQLParseNode* pNode = pParseRoot->getChild(3)->getChild(4)->getChild(2);
            ::connectivity::OSQLParseNode* pParamRef = NULL;
            ::rtl::OUString aField, aAlias;
            sal_uInt16 nPos = 0;

            EOrderDir eOrderDir;
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            for( sal_uInt32 i=0 ; i<pNode->count() ; i++ )
            {
                eOrderDir = ORDER_ASC;
                ::connectivity::OSQLParseNode*  pChild = pNode->getChild( i );

                if (SQL_ISTOKEN( pChild->getChild(1), DESC ) )
                    eOrderDir = ORDER_DESC;

                if(SQL_ISRULE(pChild->getChild(0),column_ref))
                {
                    if( eOk == FillDragInfo(_pView,pChild->getChild(0),aDragLeft))
                        _pSelectionBrw->AddOrder( aDragLeft, eOrderDir, nPos);
                    else // it could be a alias name for a field
                    {
                        ::rtl::OUString aTableRange,aColumnName;
                        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());

                        ::connectivity::OSQLParseTreeIterator& rParseIter = pController->getParseIterator();
                        rParseIter.getColumnRange( pChild->getChild(0), aColumnName, aTableRange );

                        OTableFields& aList = pController->getTableFieldDesc();
                        OTableFields::iterator aIter = aList.begin();
                        for(;aIter != aList.end();++aIter)
                        {
                            OTableFieldDescRef pEntry = *aIter;
                            if(pEntry.isValid() && pEntry->GetFieldAlias() == aColumnName)
                                pEntry->SetOrderDir( eOrderDir );
                        }
                    }
                }
                else if(SQL_ISRULE(pChild->getChild(0),general_set_fct) &&
                        SQL_ISRULE(pParamRef = pChild->getChild(0)->getChild(pChild->getChild(0)->count()-2),column_ref) &&
                        eOk == FillDragInfo(_pView,pParamRef,aDragLeft))
                    _pSelectionBrw->AddOrder( aDragLeft, eOrderDir, nPos);
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
                            int &rLevel )
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
        if (!pSelectRoot->getChild(3)->getChild(2)->isLeaf())
        {
            ::connectivity::OSQLParseNode* pGroupBy = pSelectRoot->getChild(3)->getChild(2)->getChild(2);
            OTableFieldDescRef aDragInfo = new OTableFieldDesc();
            for( sal_uInt32 i=0 ; i < pGroupBy->count() && eOk == eErrorCode; ++i )
            {
                ::connectivity::OSQLParseNode* pColumnRef = pGroupBy->getChild( i );
                if(SQL_ISRULE(pColumnRef,column_ref))
                {
                    if ( eOk == (eErrorCode = FillDragInfo(_pView,pColumnRef,aDragInfo)) )
                    {
                        aDragInfo->SetGroupBy(sal_True);
                        _pSelectionBrw->AddGroupBy(aDragInfo);
                    }
                }
            }
        }
        return eErrorCode;
    }
    //------------------------------------------------------------------------------
    void showParseError(IEnvironment* _pEnvironment,SqlParseError _eErrorCode)
    {
        USHORT nResId;
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
        //  ErrorBox( _pWindow, ModuleRes( nResId ) ).Execute();
        ModuleRes aRes(nResId);
        String sError(aRes);
        _pEnvironment->appendError(SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY0000") ),1000,Any()));
    }
    // -----------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
} // end of anonymouse namespace

OQueryDesignView::OQueryDesignView( OQueryContainerWindow* _pParent,
                                    OQueryController* _pController,
                                    const Reference< XMultiServiceFactory >& _rFactory)
    :OQueryView(_pParent,_pController,_rFactory)
    ,m_aSplitter( this )
    ,m_eChildFocus(NONE)
    ,m_bInKeyEvent(sal_False)
    ,m_bInSplitHandler( sal_False )
{
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

    setNoneVisbleRow(static_cast<OQueryController*>(getController())->getVisibleRows());
    m_pSelectionBox->Show();
    // Splitter einrichten
    m_aSplitter.SetSplitHdl(LINK(this, OQueryDesignView,SplitHdl));
    m_aSplitter.Show();

}
// -----------------------------------------------------------------------------
OQueryDesignView::~OQueryDesignView()
{
    ::std::auto_ptr<Window> aTemp(m_pSelectionBox);
    m_pSelectionBox = NULL;
}
//------------------------------------------------------------------------------
IMPL_LINK( OQueryDesignView, SplitHdl, void*, p )
{
    if (!getController()->isReadOnly())
    {
        m_bInSplitHandler = sal_True;
        long nTest = m_aSplitter.GetPosPixel().Y();
        m_aSplitter.SetPosPixel( Point( m_aSplitter.GetPosPixel().X(),m_aSplitter.GetSplitPosPixel() ) );
        static_cast<OQueryController*>(getController())->setSplitPos(m_aSplitter.GetSplitPosPixel());
        static_cast<OQueryController*>(getController())->setModified();
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
    if(static_cast<OQueryController*>(getController())->getSplitPos() != -1)
    {
        m_aSplitter.SetPosPixel( Point( m_aSplitter.GetPosPixel().X(),static_cast<OQueryController*>(getController())->getSplitPos() ) );
        m_aSplitter.SetSplitPosPixel(static_cast<OQueryController*>(getController())->getSplitPos());
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
    sal_Int32 nSplitPos = static_cast<OQueryController*>(getController())->getSplitPos();
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

            static_cast<OQueryController*>(getController())->setSplitPos(nSplitPos);
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

                static_cast< OQueryController* >(getController())->setSplitPos( nSplitPos );
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
void OQueryDesignView::setStatement(const ::rtl::OUString& _rsStatement)
{
}
// -----------------------------------------------------------------------------
void OQueryDesignView::copy()
{
    if( m_eChildFocus == SELECTION)
        m_pSelectionBox->copy();
}
// -----------------------------------------------------------------------------
BOOL OQueryDesignView::IsAddAllowed()
{
    return m_pTableView->IsAddAllowed();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isCutAllowed()
{
    sal_Bool bAllowed = sal_False;
    switch(m_eChildFocus)
    {
        case SELECTION:
            bAllowed = m_pSelectionBox->isCutAllowed();
            break;
        case TABLEVIEW:
            break;
    }
    return bAllowed;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isPasteAllowed()
{
    sal_Bool bAllowed = sal_False;
    switch(m_eChildFocus)
    {
        case SELECTION:
            bAllowed = m_pSelectionBox->isPasteAllowed();
            break;
        case TABLEVIEW:
            break;
    }
    return bAllowed;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::isCopyAllowed()
{
    sal_Bool bAllowed = sal_False;
    switch(m_eChildFocus)
    {
        case SELECTION:
            bAllowed = m_pSelectionBox->isCopyAllowed();
            break;
        case TABLEVIEW:
            break;
    }
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
        static_cast<OQueryController*>(getController())->setModified(sal_True);
    }
}
// -----------------------------------------------------------------------------
void OQueryDesignView::paste()
{
    if( m_eChildFocus == SELECTION)
    {
        m_pSelectionBox->paste();
        static_cast<OQueryController*>(getController())->setModified(sal_True);
    }
}
// -----------------------------------------------------------------------------
void OQueryDesignView::TableDeleted(const ::rtl::OUString& rAliasName)
{
    // Nachricht, dass Tabelle aus dem Fenster gel"oscht wurde
    DeleteFields(rAliasName);
    static_cast<OQueryController*>(getController())->InvalidateFeature(ID_BROWSER_ADDTABLE);    // view nochmal bescheid sagen
}
//------------------------------------------------------------------------------
void OQueryDesignView::DeleteFields( const ::rtl::OUString& rAliasName )
{
    m_pSelectionBox->DeleteFields( rAliasName );
}
// -----------------------------------------------------------------------------
void OQueryDesignView::SaveTabWinUIConfig(OQueryTableWindow* pWin)
{
    static_cast<OQueryController*>(getController())->SaveTabWinPosSize(pWin, m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos());
}
// -----------------------------------------------------------------------------
SqlParseError OQueryDesignView::InsertField( const OTableFieldDescRef& rInfo, sal_Bool bVis, sal_Bool bActivate)
{
    return m_pSelectionBox->InsertField( rInfo, BROWSER_INVALIDID,bVis, bActivate ).isValid() ? eOk : eTooManyColumns;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::getColWidth( const ::rtl::OUString& rAliasName, const ::rtl::OUString& rFieldName, sal_uInt32& nWidth )
{
    OTableFields& aFields = static_cast<OQueryController*>(getController())->getTableFieldDesc();
    OTableFields::iterator aIter = aFields.begin();
    for(;aIter != aFields.end();++aIter)
    {
        if( rAliasName == (*aIter)->GetFieldAlias() && rFieldName == (*aIter)->GetField())
        {
            nWidth = (*aIter)->GetColWidth();
            return sal_True;
        }
    }

    return sal_False;
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
    for(;aIter != pTabWins->end();++aIter)
    {
        OQueryTableWindow* pCurrentWin = static_cast<OQueryTableWindow*>(aIter->second);
        if (bAllTables || (pCurrentWin->GetAliasName() == sAliasName))
        {
            strCurrentPrefix = pCurrentWin->GetAliasName();
            strCurrentPrefix += ::rtl::OUString('.');

            pCurrentWin->EnumValidFields(aFields);

            ::std::vector< ::rtl::OUString>::iterator aStrIter = aFields.begin();
            for(;aStrIter != aFields.end();++aStrIter)
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
        bRet = m_pSelectionBox->Save(); // a error occured so we return no
    return bRet;
}
//-------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::getStatement()
{
    OQueryController* pController = static_cast<OQueryController*>(getController());
    m_pController->clearError();
    // used for fields which aren't any longer in the statement
    OTableFields& rUnUsedFields = pController->getUnUsedFields();
    OTableFields().swap( rUnUsedFields );

    // create the select columns
    sal_uInt32 nFieldcount = 0;
    OTableFields& rFieldList = pController->getTableFieldDesc();
    OTableFields::iterator aIter = rFieldList.begin();
    for(;aIter != rFieldList.end();++aIter)
    {
        OTableFieldDescRef pEntryField = *aIter;
        if ( pEntryField->GetField().getLength() && pEntryField->IsVisible() )
            ++nFieldcount;
        else if (pEntryField->GetField().getLength()            &&
                !pEntryField->HasCriteria()                 &&
                pEntryField->isNoneFunction()               &&
                pEntryField->GetOrderDir() == ORDER_NONE    &&
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
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    ::rtl::OUString aTableListStr(GenerateFromClause(xConnection,pTabList,pConnList));
    DBG_ASSERT(aTableListStr.getLength(), "OQueryDesignView::getStatement() : unerwartet : habe Felder, aber keine Tabellen !");
    // wenn es Felder gibt, koennen die nur durch Einfuegen aus einer schon existenten Tabelle entstanden sein; wenn andererseits
    // eine Tabelle geloescht wird, verschwinden auch die zugehoerigen Felder -> ergo KANN es das nicht geben, dass Felder
    // existieren, aber keine Tabellen (und aFieldListStr hat schon eine Laenge, das stelle ich oben sicher)
    ::rtl::OUString aHavingStr,aCriteriaListStr;
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
            aTmp += aCriteriaListStr;
        }
        aCriteriaListStr = aTmp;
    }
    // ----------------- Statement aufbauen ----------------------
    ::rtl::OUString aSqlCmd(::rtl::OUString::createFromAscii("SELECT "));
    if(static_cast<OQueryController*>(getController())->isDistinct())
        aSqlCmd += ::rtl::OUString::createFromAscii(" DISTINCT ");
    aSqlCmd += aFieldListStr;
    aSqlCmd += ::rtl::OUString::createFromAscii(" FROM ");
    aSqlCmd += aTableListStr;

    if (aCriteriaListStr.getLength())
    {
        aSqlCmd += ::rtl::OUString::createFromAscii(" WHERE ");
        aSqlCmd += aCriteriaListStr;
    }
    // ----------------- GroupBy aufbauen und Anh"angen ------------
    Reference<XDatabaseMetaData> xMeta;
    if ( xConnection.is() )
        xMeta = xConnection->getMetaData();
    sal_Bool bUseAlias = nTabcount > 1;
    if ( xMeta.is() )
        bUseAlias = bUseAlias || !xMeta->supportsGroupByUnrelated();

    aSqlCmd += GenerateGroupBy(this,rFieldList,bUseAlias);
    // ----------------- having Anh"angen ------------
    if(aHavingStr.getLength())
    {
        aSqlCmd += ::rtl::OUString::createFromAscii(" HAVING ");
        aSqlCmd += aHavingStr;
    }
    // ----------------- Sortierung aufbauen und Anh"angen ------------
    ::rtl::OUString sOrder;
    SqlParseError eErrorCode = eOk;
    if ( (eErrorCode = GenerateOrder(this,rFieldList,nTabcount > 1,sOrder)) == eOk)
        aSqlCmd += sOrder;
    else
    {
        if ( !m_pController->hasError() ) // fill generell error string
            showParseError(m_pController,eErrorCode);

        SQLException aError;
        m_pController->getError(aError);
        m_pController->showError(aError);
    }

    return aSqlCmd;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void OQueryDesignView::setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable)
{
    sal_uInt16 nRow;
    switch (_nSlotId)
    {
        case ID_BROWSER_QUERY_VIEW_FUNCTIONS:
            nRow = BROW_FUNCTION_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_TABLES:
            nRow = BROW_TABLE_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_ALIASES:
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
        case ID_BROWSER_QUERY_VIEW_FUNCTIONS:
            nRow = BROW_FUNCTION_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_TABLES:
            nRow = BROW_TABLE_ROW;
            break;
        case ID_BROWSER_QUERY_VIEW_ALIASES:
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
void OQueryDesignView::zoomTableView(const Fraction& _rFraction)
{
    m_pTableView->SetZoom(_rFraction);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::SaveUIConfig()
{
    OQueryController* pCtrl = static_cast<OQueryController*>(getController());
    if (pCtrl)
    {
        pCtrl->SaveTabWinsPosSize( m_pTableView->GetTabWinMap(), m_pScrollWindow->GetHScrollBar()->GetThumbPos(), m_pScrollWindow->GetVScrollBar()->GetThumbPos() );
        //  pCtrl->SaveTabFieldsWidth( m_pSelectionBox );
        pCtrl->setVisibleRows( m_pSelectionBox->GetNoneVisibleRows() );
        pCtrl->setSplitPos( m_aSplitter.GetSplitPosPixel() );
    }
}
// -----------------------------------------------------------------------------
OSQLParseNode* OQueryDesignView::getPredicateTreeFromEntry(OTableFieldDescRef pEntry,
                                                           const String& _sCriteria,
                                                           ::rtl::OUString& _rsErrorMessage,
                                                           Reference<XPropertySet>& _rxColumn)
{
    OSL_ENSURE(pEntry.isValid(),"Entry is null!");
    if(!pEntry.isValid())
        return NULL;
    Reference< XConnection> xConnection = static_cast<OQueryController*>(getController())->getConnection();
    if(!xConnection.is())
        return NULL;

    ::connectivity::OSQLParser* pParser = static_cast<OQueryController*>(getController())->getParser();
    OQueryTableWindow* pWin = static_cast<OQueryTableWindow*>(pEntry->GetTabWindow());

    String sTest(_sCriteria);
    // special handling for functions
    if ( pEntry->GetFunctionType() & (FKT_OTHER | FKT_AGGREGATE | FKT_NUMERIC) )
    {
        // we have a function here so we have to distinguish the type of return value
        String sFunction;
        if ( pEntry->isNumericOrAggreateFunction() )
            sFunction = pEntry->GetFunction();
        else
            sFunction = pEntry->GetField();

        if(sFunction.GetTokenCount('(') > 1)
            sFunction = sFunction.GetToken(0,'('); // this should be the name of the function

        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionReturnType(sFunction,&pParser->getContext());
        if ( nType == DataType::OTHER || (!sFunction.Len() && pEntry->isNumericOrAggreateFunction()) )
        {
            // first try the international version
            ::rtl::OUString sSql;
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * "));
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM x WHERE "));
            sSql += pEntry->GetField();
            sSql += _sCriteria;
            OSQLParseNode* pParseNode = pParser->parseTree( _rsErrorMessage,sSql,sal_True);
            nType = DataType::DOUBLE;
            if ( pParseNode )
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
                delete pParseNode;
            }
        }

        Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
        parse::OParseColumn* pColumn = new parse::OParseColumn( pEntry->GetField(),
                                                                ::rtl::OUString(),
                                                                ::rtl::OUString(),
                                                                ColumnValue::NULLABLE_UNKNOWN,
                                                                0,
                                                                0,
                                                                nType,
                                                                sal_False,
                                                                sal_False,
                                                                xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers());
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

    OSQLParseNode* pParseNode = pParser->predicateTree( _rsErrorMessage,
                                                        sTest,
                                                        static_cast<OQueryController*>(getController())->getNumberFormatter(),
                                                        _rxColumn);
    return pParseNode;
}
// -----------------------------------------------------------------------------
void OQueryDesignView::GetFocus()
{
    OQueryView::GetFocus();
    if ( m_pSelectionBox && !m_pSelectionBox->HasChildPathFocus() )
    {
        // first we have to deactivate the current cell to refill when nescessary
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
sal_Bool OQueryDesignView::InitFromParseNode()
{
    SqlParseError eErrorCode;
    m_pController->clearError();
    if ( (eErrorCode = InitFromParseNodeImpl(this,m_pSelectionBox)) != eOk )
    {
        if ( !m_pController->hasError() ) // fill generell error string
            showParseError(m_pController,eErrorCode);

        SQLException aError;
        m_pController->getError(aError);
        m_pController->showError(aError);
    }
    return eErrorCode == eOk;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::HasTable() const
{
    return !m_pTableView->GetTabWinMap()->empty();
}
// -----------------------------------------------------------------------------


