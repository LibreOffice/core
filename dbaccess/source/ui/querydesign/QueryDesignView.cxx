/*************************************************************************
 *
 *  $RCSfile: QueryDesignView.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: oj $ $Date: 2002-02-11 12:55:21 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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

using namespace ::dbaui;
using namespace ::utl;
using namespace ::connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

extern ::rtl::OUString ConvertAlias(const ::rtl::OUString& rName);
// here we define our functions used in the anonymous namespace to get our header file smaller
// please look at the book LargeScale C++ to know why
namespace
{
    static const ::rtl::OUString C_AND = ::rtl::OUString::createFromAscii(" AND ");
    static const ::rtl::OUString C_OR  = ::rtl::OUString::createFromAscii(" OR ");

    // forward declarations
    sal_Bool InsertJoin(    const OQueryDesignView* _pView,
                            const ::connectivity::OSQLParseNode *pNode);

    int InstallFields(      OQueryDesignView* _pView,
                            const ::connectivity::OSQLParseNode* pNode,
                            OJoinTableView::OTableWindowMap* pTabList );

    void GetGroupCriteria(  OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot );

    void GetHavingCriteria( OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot,
                            int &rLevel );

    void GetOrderCriteria(  OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pParseRoot );

    //------------------------------------------------------------------------------
    void FillOuterJoins(OQueryDesignView* _pView,
                        const ::connectivity::OSQLParseNode* pTableRefList)
    {
        sal_uInt32 ncount = pTableRefList->count();

        if (ncount == 0)
        {
            ErrorBox( _pView, ModuleRes( ERR_QRY_ILLEGAL_JOIN ) ).Execute();
        }
        else
        {
            for (sal_uInt32 i=0; i < ncount; i++)
            {
                const ::connectivity::OSQLParseNode* pParseNode = pTableRefList->getChild(i);
                if (SQL_ISRULE(pParseNode , qualified_join) ||
                    SQL_ISRULE(pParseNode , joined_table))
                {
                    if (!InsertJoin(_pView,pParseNode))
                    {
                        ErrorBox( _pView, ModuleRes( ERR_QRY_ILLEGAL_JOIN ) ).Execute();
                        return;
                    }
                }
                else if(pParseNode->count() == 4 && SQL_ISPUNCTUATION(pParseNode->getChild(0),"{") && SQL_ISRULE(pParseNode,table_ref))
                {
                    if (!InsertJoin(_pView,pParseNode->getChild(2)))
                    {
                        ErrorBox( _pView, ModuleRes( ERR_QRY_ILLEGAL_JOIN ) ).Execute();
                        return;
                    }
                }
            }
        }
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
                                rNewValue = ::rtl::OUString::createFromAscii("TRUE");
                            else if (bCase(rValue, ::rtl::OUString(ModuleRes(STR_QUERY_FALSE))))
                                rNewValue = ::rtl::OUString::createFromAscii("FALSE");
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

    //------------------------------------------------------------------------------
    sal_Bool FillDragInfo(  const OQueryDesignView* _pView,
                            const ::connectivity::OSQLParseNode* pColumnRef,
                            OTableFieldDescRef& _rDragInfo)
    {

        sal_Bool bErg = sal_False;

        ::rtl::OUString aTableRange,aColumnName;
        sal_uInt16 nCntAccount;
        ::connectivity::OSQLParseTreeIterator& rParseIter = static_cast<OQueryController*>(_pView->getController())->getParseIterator();
        rParseIter.getColumnRange( pColumnRef, aColumnName, aTableRange );

        if (aTableRange.getLength())
        {
            OQueryTableWindow*  pSTW = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable( aTableRange );
            if(pSTW && pSTW->ExistsField( aColumnName, _rDragInfo ) )
                bErg = sal_True;
        }
        if(!bErg)
            bErg = static_cast<OQueryTableView*>(_pView->getTableView())->FindTableFromField(aColumnName, _rDragInfo, nCntAccount);

        return bErg;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoinCriteria(  const OQueryDesignView* _pView,
                                        OConnectionLineDataVec* pLineDataList,
                                        OQueryTableConnectionData* pData)
    {
        ::rtl::OUString aCondition;
        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if(!xConnection.is())
            return aCondition;

        OConnectionLineDataVec::iterator aIter = pLineDataList->begin();
        try
        {
            Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
            ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();

            for(;aIter != pLineDataList->end();++aIter)
            {
                OConnectionLineDataRef pLineData = *aIter;
                if(pLineData->IsValid())
                {
                    if(aCondition.getLength())
                        aCondition += C_AND;
                    aCondition += ::dbtools::quoteName(aQuote, ConvertAlias( pData->GetAliasName(JTCS_FROM) ));
                    aCondition += ::rtl::OUString('.');
                    aCondition += ::dbtools::quoteName(aQuote, pLineData->GetFieldName(JTCS_FROM) );
                    aCondition += ::rtl::OUString::createFromAscii(" = ");
                    aCondition += ::dbtools::quoteName(aQuote, ConvertAlias( pData->GetAliasName(JTCS_TO) ));
                    aCondition += ::rtl::OUString('.');
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
        @param  _pView          the view
        @param  _pEntryConn     the table connection which holds the data
        @param  _pEntryTabTo    the corresponding table window
        @param  _rJoin          the String which will contain the resulting string
    */
    void JoinCycle( const OQueryDesignView* _pView,
                    OQueryTableConnection* _pEntryConn,
                    const OQueryTableWindow* _pEntryTabTo,
                    ::rtl::OUString& _rJoin )
    {
        OSL_ENSURE(_pView,"The view can not be null!");
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
            (_rJoin += C_AND) += BuildJoinCriteria(_pView,pData->GetConnLineDataList(),pData);
            if(bBrace)
                _rJoin += ::rtl::OUString(')');
            _pEntryConn->SetVisited(sal_True);
        }
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildTable(const OQueryDesignView* _pView,const OQueryTableWindow* pEntryTab)
    {
        ::rtl::OUString aDBName(pEntryTab->GetComposedName());

        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if( xConnection.is() )
        {
            try
            {
                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                ::rtl::OUString aCatalog,aSchema,aTable,sComposedName;
                ::dbtools::qualifiedNameComponents(xMetaData,aDBName,aCatalog,aSchema,aTable);
                ::dbtools::composeTableName(xMetaData,aCatalog,aSchema,aTable,sComposedName,sal_True);

                ::rtl::OUString aQuote = xMetaData->getIdentifierQuoteString();
                ::rtl::OUString aTableListStr(sComposedName);
                aTableListStr += ::rtl::OUString(' ');
                aTableListStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryTab->GetAliasName())).getStr();
                aDBName = aTableListStr;
            }
            catch(SQLException&)
            {
            }
        }
        return aDBName;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const OQueryDesignView* _pView,
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
        aErg += String(BuildJoinCriteria(_pView,pData->GetConnLineDataList(),pData));

        return aErg;
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const OQueryDesignView* _pView,
                                OQueryTableWindow* pLh,
                                OQueryTableWindow* pRh,
                                OQueryTableConnectionData* pData)
    {
        return BuildJoin(_pView,BuildTable(_pView,pLh),BuildTable(_pView,pRh),pData);
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const OQueryDesignView* _pView,
                                const ::rtl::OUString &rLh,
                                OQueryTableWindow* pRh,
                                OQueryTableConnectionData* pData)
    {
        return BuildJoin(_pView,rLh,BuildTable(_pView,pRh),pData);
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString BuildJoin(  const OQueryDesignView* _pView,
                                OQueryTableWindow* pLh,
                                const ::rtl::OUString &rRh,
                                OQueryTableConnectionData* pData)
    {
        return BuildJoin(_pView,BuildTable(_pView,pLh),rRh,pData);
    }
    //------------------------------------------------------------------------------
    void GetNextJoin(   const OQueryDesignView* _pView,
                        OQueryTableConnection* pEntryConn,
                        OQueryTableWindow* pEntryTabTo,
                        ::rtl::OUString &aJoin)
    {
        OQueryTableConnectionData* pEntryConnData = static_cast<OQueryTableConnectionData*>(pEntryConn->GetData());
        if(pEntryConnData->GetJoinType() == INNER_JOIN)
            return;

        if(!aJoin.getLength())
        {
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            aJoin = BuildJoin(_pView,pEntryTabFrom,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetDestWin())
        {
            ::rtl::OUString aTmpJoin('(');
            (aTmpJoin += aJoin) += ::rtl::OUString(')');
            aJoin = BuildJoin(_pView,aTmpJoin,pEntryTabTo,pEntryConnData);
        }
        else if(pEntryTabTo == pEntryConn->GetSourceWin())
        {
            ::rtl::OUString aTmpJoin('(');
            (aTmpJoin += aJoin) += ::rtl::OUString(')');
            aJoin = BuildJoin(_pView,pEntryTabTo,aTmpJoin,pEntryConnData);
        }

        pEntryConn->SetVisited(sal_True);

        // first search for the "to" window
        ::std::vector<OTableConnection*>::iterator aIter = pEntryConn->GetParent()->GetTabConnList()->begin();
        for(;aIter != pEntryConn->GetParent()->GetTabConnList()->end();++aIter)
        {
            OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(*aIter);
            if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabTo || pNext->GetDestWin() == pEntryTabTo))
            {
                OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabTo ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
                // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
                JoinCycle(_pView,pNext,pEntryTab,aJoin);
                if(!pNext->IsVisited())
                    GetNextJoin(_pView,pNext,pEntryTab,aJoin);
            }
        }

        // when nothing found found look for the "from" window
        if(aIter == pEntryConn->GetParent()->GetTabConnList()->end())
        {
            OQueryTableWindow* pEntryTabFrom = static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin());
            aIter = pEntryConn->GetParent()->GetTabConnList()->begin();
            for(;aIter != pEntryConn->GetParent()->GetTabConnList()->end();++aIter)
            {
                OQueryTableConnection* pNext = static_cast<OQueryTableConnection*>(*aIter);
                if(!pNext->IsVisited() && (pNext->GetSourceWin() == pEntryTabFrom || pNext->GetDestWin() == pEntryTabFrom))
                {
                    OQueryTableWindow* pEntryTab = pNext->GetSourceWin() == pEntryTabFrom ? static_cast<OQueryTableWindow*>(pNext->GetDestWin()) : static_cast<OQueryTableWindow*>(pNext->GetSourceWin());
                    // exists there a connection to a OQueryTableWindow that holds a connection that has been already visited
                    JoinCycle(_pView,pNext,pEntryTab,aJoin);
                    if(!pNext->IsVisited())
                        GetNextJoin(_pView,pNext,pEntryTab,aJoin);
                }
            }
        }
    }
    //------------------------------------------------------------------------------
    sal_Bool InsertJoinConnection(  const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode *pNode,
                                    const EJoinType& _eJoinType)
    {
        if (pNode->count() == 3 &&  // Ausdruck is geklammert
            SQL_ISPUNCTUATION(pNode->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pNode->getChild(2),")"))
        {
            return InsertJoinConnection(_pView,pNode->getChild(1), _eJoinType);
        }
        else if (SQL_ISRULE(pNode,search_condition) ||
                 SQL_ISRULE(pNode,boolean_term) &&          // AND/OR-Verknuepfung:
                 pNode->count() == 3)
        {
            // nur AND Verknüpfung zulassen
            if (!SQL_ISTOKEN(pNode->getChild(1),AND))
                return sal_False;
            else
                return InsertJoinConnection(_pView,pNode->getChild(0), _eJoinType) &&
                       InsertJoinConnection(_pView,pNode->getChild(2), _eJoinType);
        }
        else if (SQL_ISRULE(pNode,comparison_predicate))
        {
            // Nur ein Vergleich auf Spalten ist erlaubt
            DBG_ASSERT(pNode->count() == 3,"OQueryDesignView::InsertJoinConnection: Fehler im Parse Tree");
            if (!(SQL_ISRULE(pNode->getChild(0),column_ref) &&
                  SQL_ISRULE(pNode->getChild(2),column_ref) &&
                   pNode->getChild(1)->getNodeType() == SQL_NODE_EQUAL))
                return sal_False;

            OTableFieldDescRef aDragLeft  = new OTableFieldDesc();
            OTableFieldDescRef aDragRight = new OTableFieldDesc();
            if (!FillDragInfo(_pView,pNode->getChild(0),aDragLeft) ||
                !FillDragInfo(_pView,pNode->getChild(2),aDragRight))
                return sal_False;

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
            return sal_True;
        }
        return sal_False;
    }
    //------------------------------------------------------------------------------
    sal_Bool GetInnerJoinCriteria(  const OQueryDesignView* _pView,
                                    const ::connectivity::OSQLParseNode *pCondition)
    {
        return InsertJoinConnection(_pView,pCondition, INNER_JOIN);
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
            if(pEntryField->IsVisible())
            {
                if(pEntryField->GetField().toChar() == '*')
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

            aIter = _rFieldList.begin();
            for(;aIter != _rFieldList.end();++aIter)
            {
                OTableFieldDescRef pEntryField = *aIter;
                ::rtl::OUString rFieldName = pEntryField->GetField();
                if (rFieldName.getLength() && pEntryField->IsVisible())
                {
                    aTmpStr = ::rtl::OUString();
                    ::rtl::OUString rAlias = pEntryField->GetAlias();
                    ::rtl::OUString rFieldAlias = pEntryField->GetFieldAlias();
                    if((bAlias || bAsterix) && rAlias.getLength())
                    {
                        aTmpStr += ::dbtools::quoteName(aQuote,ConvertAlias(rAlias));
                        aTmpStr += ::rtl::OUString('.');
                    }
                    // we have to look if we have alias.* here
                    String sTemp = rFieldName;
                    if(sTemp.GetTokenCount('.') == 2)
                        rFieldName = sTemp.GetToken(1,'.');

                    if(pEntryField->GetTable().getLength() && rFieldName.toChar() != '*')
                        aTmpStr += ::dbtools::quoteName(aQuote, rFieldName).getStr();
                    else
                        aTmpStr += rFieldName;

                    if(pEntryField->GetFunctionType() == FKT_AGGREGATE)
                    {
                        DBG_ASSERT(pEntryField->GetFunction().getLength(),"Functionname darf hier nicht leer sein! ;-(");
                        ::rtl::OUString aTmpStr2(pEntryField->GetFunction());
                        aTmpStr2 +=  ::rtl::OUString('(');
                        aTmpStr2 += aTmpStr;
                        aTmpStr2 +=  ::rtl::OUString(')');
                        aTmpStr = aTmpStr2;
                    }

                    if( rFieldAlias.getLength()                         &&
                        (rFieldName.toChar() != '*'                     ||
                        pEntryField->GetFunctionType() == FKT_AGGREGATE ||
                        pEntryField->GetFunctionType() == FKT_OTHER))
                    {
                        aTmpStr += ::rtl::OUString::createFromAscii(" AS ");
                        aTmpStr += ::dbtools::quoteName(aQuote, rFieldAlias);
                    }
                    aFieldListStr += aTmpStr;
                    aFieldListStr += ::rtl::OUString::createFromAscii(", ");
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
            const OParseContext& rContext = static_cast<OQueryController*>(_pView->getController())->getParser()->getContext();

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
                    if (aCriteria.getLength())
                    {
                        if (aFieldName.toChar() == '*' && pEntryField->GetFunctionType() == FKT_NONE)               // * darf keine Filter besitzen
                        {
                            // only show the messagebox the first time
                            if (!bCritsOnAsterikWarning)
                                ErrorBox(_pView, ModuleRes( ERR_QRY_CRITERIA_ON_ASTERISK)).Execute();
                            bCritsOnAsterikWarning = sal_True;
                            continue;
                        }
                        aWork = ::rtl::OUString();


                        if (bMulti)
                        {
                            if(pEntryField->GetFunctionType() == FKT_OTHER || (aFieldName.toChar() == '*'))
                                aWork += aFieldName;
                            else
                                aWork += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias()));
                            aWork += ::rtl::OUString('.');
                        }
                        if(pEntryField->GetFunctionType() == FKT_OTHER || (aFieldName.toChar() == '*'))
                            aWork += aFieldName;
                        else
                            aWork += ::dbtools::quoteName(aQuote, aFieldName).getStr();

                        if(pEntryField->GetFunctionType() == FKT_AGGREGATE || pEntryField->IsGroupBy())
                        {
                            if (!aHavingStr.getLength())            // noch keine Kriterien
                                aHavingStr += ::rtl::OUString('(');         // Klammern
                            else
                                aHavingStr += C_AND;

                            if(pEntryField->GetFunctionType() == FKT_AGGREGATE)
                            {
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
                                if (bMulti && !(pEntryField->GetFunctionType() == FKT_OTHER || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(ConvertAlias(pEntryField->GetAlias()),aFieldName);
                                ::rtl::OUString sHavingStr = aHavingStr;
                                OSL_ENSURE(pParseNode->count() == 3,"Count must be three here!");
                                pParseNode->getChild(1)->parseNodeToStr(    sHavingStr,
                                                            xMetaData,
                                                            &rContext,
                                                            sal_False,
                                                            pEntryField->GetFunctionType() != FKT_OTHER);
                                pParseNode->getChild(2)->parseNodeToStr(    sHavingStr,
                                                            xMetaData,
                                                            &rContext,
                                                            sal_False,
                                                            pEntryField->GetFunctionType() != FKT_OTHER);
                                aHavingStr = sHavingStr;
                                delete pParseNode;
                            }
                            else
                                aHavingStr += aCriteria;
                        }
                        else
                        {
                            if (!aWhereStr.getLength())         // noch keine Kriterien
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
                                if (bMulti && !(pEntryField->GetFunctionType() == FKT_OTHER || (aFieldName.toChar() == '*')))
                                    pParseNode->replaceNodeValue(ConvertAlias(pEntryField->GetAlias()),aFieldName);
                                ::rtl::OUString aWhere = aWhereStr;
                                pParseNode->parseNodeToStr( aWhere,
                                                            xMetaData,
                                                            &rContext,
                                                            sal_False,
                                                            pEntryField->GetFunctionType() != FKT_OTHER);
                                aWhereStr = aWhere;
                                delete pParseNode;
                            }
                            else
                            {
                                aWhereStr += aWork;
                                aWhereStr += aCriteria;
                            }
                        }
                    }
                    // nur einmal für jedes Feld
                    else if(!i && pEntryField->GetFunctionType() == FKT_CONDITION)
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
    ::rtl::OUString GenerateOrder( OQueryDesignView* _pView,OTableFields& _rFieldList,sal_Bool bMulti )
    {

        ::rtl::OUString aRetStr, aColumnName;
        String aWorkStr;
        Reference< XConnection> xConnection = static_cast<OQueryController*>(_pView->getController())->getConnection();
        if(!xConnection.is())
            return aRetStr;

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
                    if(pEntryField->GetFunctionType() == FKT_NONE)
                    {
                        if (bMulti && pEntryField->GetAlias().getLength())
                        {
                            aWorkStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias())).getStr();
                            aWorkStr += String('.');
                        }
                        aWorkStr += ::dbtools::quoteName(aQuote, aColumnName).getStr();
                    }
                    else if(pEntryField->GetFieldAlias().getLength())
                    {
                        aWorkStr += ::dbtools::quoteName(aQuote, pEntryField->GetFieldAlias()).getStr();
                    }
                    else if(pEntryField->GetFunctionType() == FKT_AGGREGATE)
                    {
                        DBG_ASSERT(pEntryField->GetFunction().getLength(),"Functionname darf hier nicht leer sein! ;-(");
                        aWorkStr += pEntryField->GetFunction().getStr();
                        aWorkStr +=  String('(');
                        if (bMulti && pEntryField->GetAlias().getLength())
                        {
                            aWorkStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias())).getStr();
                            aWorkStr += String('.');
                        }
                        aWorkStr += ::dbtools::quoteName(aQuote, aColumnName).getStr();
                        aWorkStr +=  String(')');
                    }
                    else
                    {
                        if (bMulti && pEntryField->GetAlias().getLength())
                        {
                            aWorkStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias())).getStr();
                            aWorkStr += String('.');
                        }
                        aWorkStr += ::dbtools::quoteName(aQuote, aColumnName).getStr();
                    }
                    aWorkStr += String(' ');
                    aWorkStr += String::CreateFromAscii( ";ASC;DESC" ).GetToken( eOrder );
                    aWorkStr += String(',');
                }
            }

            aWorkStr.EraseTrailingChars( ',' );

            if( aWorkStr.Len() )
            {
                sal_Int32 nMaxOrder = xMetaData->getMaxColumnsInOrderBy();
                if(nMaxOrder && nMaxOrder < aWorkStr.GetTokenCount(','))
                {
                    ErrorBox aBox( _pView, ModuleRes( ERR_QRY_TOO_LONG_STATEMENT ) );
                    aBox.Execute();
                }
                else
                {
                    aRetStr = ::rtl::OUString::createFromAscii(" ORDER BY ");
                    aRetStr += aWorkStr;
                }
            }
        }
        catch(SQLException&)
        {
            OSL_ASSERT(!"Failure while building group by!");
        }

        return aRetStr;
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
    void GenerateInnerJoinCriterias(const OQueryDesignView* _pView,
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
                _rJoinCrit += BuildJoinCriteria(_pView,pEntryConnData->GetConnLineDataList(),pEntryConnData);
            }
        }
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString GenerateFromClause( const OQueryDesignView* _pView,
                                        const OQueryTableView::OTableWindowMap* pTabList,
                                        ::std::vector<OTableConnection*>*   pConnList)
    {

        ::rtl::OUString aTableListStr;
        // wird gebraucht um sicher zustelllen das eine Tabelle nicht doppelt vorkommt

        // generate outer join clause in from
        if(!pConnList->empty())
        {
            ::std::vector<OTableConnection*>::iterator aIter = pConnList->begin();
            for(;aIter != pConnList->end();++aIter)
                static_cast<OQueryTableConnection*>(*aIter)->SetVisited(sal_False);

            aIter = pConnList->begin();
            for(;aIter != pConnList->end();++aIter)
            {
                OQueryTableConnection* pEntryConn = static_cast<OQueryTableConnection*>(*aIter);
                if(!pEntryConn->IsVisited())
                {
                    ::rtl::OUString aJoin;
                    GetNextJoin(_pView,pEntryConn,static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()),aJoin);

                    if(aJoin.getLength())
                    {
                        ::rtl::OUString aStr = ::rtl::OUString::createFromAscii("{ OJ ");
                        aStr += aJoin;
                        aStr += ::rtl::OUString::createFromAscii(" },");
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
                    ::rtl::OUString aTabName(BuildTable(_pView,static_cast<OQueryTableWindow*>(pEntryConn->GetSourceWin())));
                    if(aTableNames.find(aTabName) == aTableNames.end())
                    {
                        aTableNames[aTabName] = sal_True;
                        aTableListStr += aTabName;
                        aTableListStr += ::rtl::OUString(',');
                    }
                    aTabName = BuildTable(_pView,static_cast<OQueryTableWindow*>(pEntryConn->GetDestWin()));
                    if(aTableNames.find(aTabName) == aTableNames.end())
                    {
                        aTableNames[aTabName] = sal_True;
                        aTableListStr += aTabName;
                        aTableListStr += ::rtl::OUString(',');
                    }
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
                aTableListStr += BuildTable(_pView,pEntryTab);
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
                if(pEntryField->IsGroupBy())
                {
                    DBG_ASSERT(pEntryField->GetField().getLength(),"Kein FieldName vorhanden!;-(");
                    if (bMulti)
                    {
                        aGroupByStr += ::dbtools::quoteName(aQuote, ConvertAlias(pEntryField->GetAlias()));
                        aGroupByStr += ::rtl::OUString('.');
                    }

                    aGroupByStr += ::dbtools::quoteName(aQuote, pEntryField->GetField());
                    aGroupByStr += ::rtl::OUString(',');
                }
            }
            if(aGroupByStr.getLength())
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
    int GetORCriteria(  OQueryDesignView* _pView,
                        OSelectionBrowseBox* _pSelectionBrw,
                        const ::connectivity::OSQLParseNode * pCondition,
                        int& nLevel ,
                        sal_Bool bHaving = sal_False);
    // -----------------------------------------------------------------------------
    int GetSelectionCriteria(   OQueryDesignView* _pView,
                                OSelectionBrowseBox* _pSelectionBrw,
                                const ::connectivity::OSQLParseNode* pNode,
                                int& rLevel,
                                sal_Bool bJoinWhere = sal_False)
    {

        // Naechster freier Satz ...
        int nRet = 0;
        int nJoins=0;

        if (!SQL_ISRULE(pNode, select_statement))
        {
            ErrorBox    aBox( _pView, ModuleRes( ERR_QRY_NOSELECT ) );
            aBox.Execute();
            return 1;
        }

        // nyi: mehr Pruefung auf korrekte Struktur!
        pNode = pNode ? pNode->getChild(3)->getChild(1) : NULL;
        // Keine WHERE-Klausel.
        if (!pNode || pNode->isLeaf())
            return nRet;

        ::connectivity::OSQLParseNode * pCondition = pNode->getChild(1);
        if (!pCondition) // kein where clause
            return nRet;

        // jetzt die anderen Bedingungen eintragen
        ::connectivity::OSQLParseNode::negateSearchCondition(pCondition);

        ::connectivity::OSQLParseNode *pNodeTmp = pNode->getChild(1);
        ::connectivity::OSQLParseNode::disjunctiveNormalForm(pNodeTmp);
        pNodeTmp = pNode->getChild(1);
        ::connectivity::OSQLParseNode::absorptions(pNodeTmp);
        pNodeTmp = pNode->getChild(1);

        // first extract the inner joins conditions
        GetInnerJoinCriteria(_pView,pNodeTmp);

        // it could happen that pCondition is not more valid
        nRet = GetORCriteria(_pView,_pSelectionBrw,pNodeTmp, rLevel);

        if (nRet != 0)                                             // mindestens eine OR Verknuepfung
        {
            ErrorBox aBox( _pView, ModuleRes( ERR_QRY_TOOCOMPLEX ) );
            aBox.Execute();
            return 99;
        }
        return nRet;
    }
    //------------------------------------------------------------------------------
    int GetANDCriteria( OQueryDesignView* _pView,
                        OSelectionBrowseBox* _pSelectionBrw,
                        const  ::connectivity::OSQLParseNode * pCondition,
                        const int nLevel,
                        sal_Bool bHaving );
    //------------------------------------------------------------------------------
    int ComparsionPredicate(OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const int nLevel,
                            sal_Bool bHaving );
    //------------------------------------------------------------------------------
    int GetORCriteria(  OQueryDesignView* _pView,
                        OSelectionBrowseBox* _pSelectionBrw,
                        const ::connectivity::OSQLParseNode * pCondition,
                        int& nLevel ,
                        sal_Bool bHaving)
    {
        int nRet = 0;

        // Runde Klammern um den Ausdruck
        if (pCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
        {
            nRet = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(1),nLevel,bHaving);
        }
        // oder Verknuepfung
        // a searchcondition can only look like this: search_condition SQL_TOKEN_OR boolean_term
        else if (SQL_ISRULE(pCondition,search_condition))
        {
            for (int i = 0; i < 3; i+=2)
            {
                // Ist das erste Element wieder eine OR-Verknuepfung?
                // Dann rekursiv absteigen ...
                //if (!i && SQL_ISRULE(pCondition->getChild(i),search_condition))
                if (SQL_ISRULE(pCondition->getChild(i),search_condition))
                {
                    nRet = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(i),nLevel,bHaving);
                }
                else if (!nRet)
                {
                    nRet = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(i), nLevel++,bHaving);
                }
                else
                {
                    ErrorBox aBox( _pView, ModuleRes( ERR_QRY_TOOMANYCOND ) );
                    aBox.Execute();
                    return 1;
                }
            }
        }
        else
        {
            nRet = GetANDCriteria( _pView,_pSelectionBrw,pCondition, nLevel, bHaving );
        }
        return nRet;
    }
    //--------------------------------------------------------------------------------------------------
    int GetANDCriteria( OQueryDesignView* _pView,
                        OSelectionBrowseBox* _pSelectionBrw,
                        const  ::connectivity::OSQLParseNode * pCondition,
                        const int nLevel,
                        sal_Bool bHaving )
    {
        ::com::sun::star::lang::Locale  aLocale = _pView->getLocale();
        ::rtl::OUString sDecimal = _pView->getDecimalSeparator();

        // ich werde ein paar Mal einen gecasteten Pointer auf meinen ::com::sun::star::sdbcx::Container brauchen
        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());
        int nRet = 0;

        // Runde Klammern
        if (SQL_ISRULE(pCondition,boolean_primary))
        {
            int nLevel2 = nLevel;
            nRet = GetORCriteria(_pView,_pSelectionBrw,pCondition->getChild(1), nLevel2,bHaving );
        }
        // Das erste Element ist (wieder) eine AND-Verknuepfung
        else if ( SQL_ISRULE(pCondition,boolean_term) && pCondition->count() == 3 )
        {
            nRet = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(0), nLevel,bHaving );
            if (!nRet)
                nRet = GetANDCriteria(_pView,_pSelectionBrw,pCondition->getChild(2), nLevel,bHaving );
        }
        else if (SQL_ISRULE( pCondition, comparison_predicate))
        {
            nRet = ComparsionPredicate(_pView,_pSelectionBrw,pCondition,nLevel,bHaving);
        }
        else if((SQL_ISRULE(pCondition,like_predicate)))
        {
            ::rtl::OUString aCondition;
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
            {
                ::rtl::OUString aColumnName;
                Reference< XConnection> xConnection = pController->getConnection();
                if(xConnection.is())
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

                if (FillDragInfo(_pView,pCondition->getChild(0),aDragLeft))
                    _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
                else
                {
                    ErrorBox( _pView, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
                    nRet = 5;
                }
            }
            else
            {
                ErrorBox( _pView, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
                nRet = 5;
            }
        }
        else if(        SQL_ISRULE(pCondition,test_for_null)
                    ||  SQL_ISRULE(pCondition,in_predicate)
                    ||  SQL_ISRULE(pCondition,all_or_any_predicate)
                    ||  SQL_ISRULE(pCondition,between_predicate))
        {
            ::rtl::OUString aCondition;
            OTableFieldDescRef  aDragLeft = new OTableFieldDesc();
            if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
            {
                // parse condition
                Reference< XConnection> xConnection = pController->getConnection();
                if(xConnection.is())
                {
                    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                    for(sal_uInt16 i=1;i< pCondition->count();i++)
                        pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                        xMetaData,
                                        pController->getNumberFormatter(),
                                        aLocale,
                                        static_cast<sal_Char>(sDecimal.toChar()),
                                        &pController->getParser()->getContext());
                }
            }

            if (FillDragInfo(_pView,pCondition->getChild(0),aDragLeft))
                _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
            else
            {
                ErrorBox( _pView, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
                nRet = 5;
            }
        }
        else if((SQL_ISRULE(pCondition,existence_test) || SQL_ISRULE(pCondition,unique_test)))
        {
            ::rtl::OUString aCondition;
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();

            // Funktions-Bedingung parsen
            Reference< XConnection> xConnection = pController->getConnection();
            if(xConnection.is())
            {
                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                for(sal_uInt16 i=0;i< pCondition->count();i++)
                    pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                                xMetaData,
                                                pController->getNumberFormatter(),
                                                aLocale,
                                                static_cast<sal_Char>(sDecimal.toChar()),
                                                &pController->getParser()->getContext());
            }

            aDragLeft->SetField(aCondition);
            aDragLeft->SetFunctionType(FKT_CONDITION);

            _pSelectionBrw->InsertField(aDragLeft,-1,sal_False,sal_True);
        }
        else
        {
            // Etwas anderes unterstuetzen wir (noch) nicht. Basta!
            ErrorBox( _pView, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
            nRet = 5;
        }
        // Fehler einfach weiterreichen.
        return nRet;
    }
    //------------------------------------------------------------------------------
    int ComparsionPredicate(OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode * pCondition,
                            const int nLevel,
                            sal_Bool bHaving )
    {
        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());

        DBG_ASSERT(SQL_ISRULE( pCondition, comparison_predicate),"ComparsionPredicate: pCondition ist kein ComparsionPredicate");
        sal_uInt32 nRet = 0;
        if(SQL_ISRULE(pCondition->getChild(0), column_ref ) || SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ))
        {
            ::rtl::OUString aCondition;
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            sal_uInt32 nPos;
            if(SQL_ISRULE(pCondition->getChild(0), column_ref ) && SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ))
            {
                OTableFieldDescRef aDragRight = new OTableFieldDesc();
                if (!FillDragInfo(_pView,pCondition->getChild(0),aDragLeft) ||
                    !FillDragInfo(_pView,pCondition->getChild(2),aDragRight))
                    return nRet;

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
                        return 0;
                }
            }

            if(SQL_ISRULE(pCondition->getChild(0), column_ref ))
            {
                nPos = 0;
                sal_uInt32 i=1;

                // don't display the equal
                if (pCondition->getChild(i)->getNodeType() == SQL_NODE_EQUAL)
                    i++;

                // Bedingung parsen
                Reference< XConnection> xConnection = pController->getConnection();
                if(xConnection.is())
                {
                    Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                    for(;i< pCondition->count();i++)
                        pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                                                        xMetaData,
                                                                        pController->getNumberFormatter(),
                                                                        _pView->getLocale(),
                                                                        static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                                        &pController->getParser()->getContext());
                }
            }
            else if(SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref ))
            {
                nPos = pCondition->count()-1;

                sal_uInt32 i = pCondition->count() - 2;
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
            if(FillDragInfo(_pView,pCondition->getChild(nPos),aDragLeft))
            {
                if(bHaving)
                    aDragLeft->SetGroupBy(sal_True);
                _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
            }
            else
            {
                ErrorBox( _pView, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
                nRet = 5;
            }
        }
        else if(SQL_ISRULE(pCondition->getChild(0), set_fct_spec ) || SQL_ISRULE(pCondition->getChild(0), general_set_fct ))
        {
            ::rtl::OUString aName,
                            aCondition;
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
                if(aCondition.indexOf('=',0) == 1) // ignore the equal sign
                    aCondition = aCondition.copy(1);


                if(SQL_ISRULE(pFunction, general_set_fct ))
                {
                    if(!FillDragInfo(_pView,pFunction->getChild(pFunction->count()-2),aDragLeft))
                    {
                        OSL_ENSURE(pFunction->count() > 3,"Invalid format for general_set_fct!");
                        ::rtl::OUString sParameterValue;
                        pFunction->getChild(pFunction->count()-2)->parseNodeToPredicateStr(sParameterValue,
                                                    xMetaData,
                                                    pController->getNumberFormatter(),
                                                    _pView->getLocale(),
                                                    static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                    &pController->getParser()->getContext());
                        aDragLeft->SetField(sParameterValue);
                    }
                    aDragLeft->SetFunctionType(FKT_AGGREGATE);
                    if(bHaving)
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
                    aDragLeft->SetFunctionType(FKT_OTHER);
                }
                _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
            }
        }
        else // kann sich nur um einen Expr. Ausdruck handeln
        {
            OTableFieldDescRef aDragLeft = new OTableFieldDesc();
            ::rtl::OUString aName,aCondition;

            ::connectivity::OSQLParseNode *pLhs = pCondition->getChild(0);
            ::connectivity::OSQLParseNode *pRhs = pCondition->getChild(2);
            // Feldnamen
            Reference< XConnection> xConnection = pController->getConnection();
            if(xConnection.is())
            {
                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                for(sal_uInt16 i=0;i< pLhs->count();i++)
                    pCondition->getChild(i)->parseNodeToStr(aName,
                                                xMetaData,
                                                &pController->getParser()->getContext(),
                                                sal_True);
                // Kriterium
                aCondition = pCondition->getChild(1)->getTokenValue();
                for(i=0;i< pRhs->count();i++)
                    pCondition->getChild(i)->parseNodeToPredicateStr(aCondition,
                                                                xMetaData,
                                                                pController->getNumberFormatter(),
                                                                _pView->getLocale(),
                                                                static_cast<sal_Char>(_pView->getDecimalSeparator().toChar()),
                                                                &pController->getParser()->getContext());
            }

            aDragLeft->SetField(aName);
            aDragLeft->SetFunctionType(FKT_OTHER);
            // und anh"angen
            _pSelectionBrw->AddCondition(aDragLeft, aCondition, nLevel);
        }
        return nRet;
    }
    //------------------------------------------------------------------------------
    int InsertColumnRef(const OQueryDesignView* _pView,
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

        DBG_ASSERT(aColumnName.getLength(),"Columnname darf nicht leer sein");
        if (!aTableRange.getLength())
        {
            // SELECT column, ...
            sal_Bool bFound(sal_False);
            OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
            for(;aIter != pTabList->end();++aIter)
            {
                OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);
                if (pTabWin->ExistsField( aColumnName, _raInfo ) )
                {
                    if(aColumnName.toChar() != '*')
                        _raInfo->SetFieldAlias(aColumnAlias);
                    bFound = sal_True;
                    break;
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
        else
        {
            // SELECT range.column, ...
            OQueryTableWindow* pTabWin = static_cast<OQueryTableView*>(_pView->getTableView())->FindTable(aTableRange);

            if (pTabWin && pTabWin->ExistsField(aColumnName, _raInfo))
            {
                if(aColumnName.toChar() != '*')
                    _raInfo->SetFieldAlias(aColumnAlias);
            }
            else
            {
                _raInfo->SetTable(::rtl::OUString());
                _raInfo->SetAlias(::rtl::OUString());
                _raInfo->SetField(aColumnName);
                _raInfo->SetFieldAlias(aColumnAlias);   // nyi : hier ein fortlaufendes Expr_1, Expr_2 ...
                _raInfo->SetFunctionType(FKT_OTHER);
            }
        }
        return 0;
    }
    //-----------------------------------------------------------------------------
    sal_Bool InsertJoin(const OQueryDesignView* _pView,const ::connectivity::OSQLParseNode *pNode)
    {
        DBG_ASSERT(SQL_ISRULE(pNode, qualified_join) || SQL_ISRULE(pNode, joined_table),
            "OQueryDesignView::InsertJoin: Fehler im Parse Tree");

        if (SQL_ISRULE(pNode,joined_table))
            return InsertJoin(_pView,pNode->getChild(1));

        if (SQL_ISRULE(pNode->getChild(0),qualified_join))
        {
            if (!InsertJoin(_pView,pNode->getChild(0)))
                return sal_False;
        }
        else if (SQL_ISRULE(pNode->getChild(0), joined_table))
        {
            if (!InsertJoin(_pView,pNode->getChild(0)->getChild(1)))
                return sal_False;
        }
        else if (!(SQL_ISRULE(pNode->getChild(0), table_ref) && (
                 SQL_ISRULE(pNode->getChild(0)->getChild(0), catalog_name)   ||
                 SQL_ISRULE(pNode->getChild(0)->getChild(0), schema_name)    ||
                 SQL_ISRULE(pNode->getChild(0)->getChild(0), table_name))))
            return sal_False;

        // geschachtelter join?
        if (SQL_ISRULE(pNode->getChild(3),qualified_join))
        {
            if (!InsertJoin(_pView,pNode->getChild(3)))
                return sal_False;
        }
        else if (SQL_ISRULE(pNode->getChild(3), joined_table))
        {
            if (!InsertJoin(_pView,pNode->getChild(3)->getChild(1)))
                return sal_False;
        }
        // sonst sollte es eine Tabelle sein
        else if (!(SQL_ISRULE(pNode->getChild(3), table_ref) && (
                 SQL_ISRULE(pNode->getChild(3)->getChild(0), catalog_name)   ||
                 SQL_ISRULE(pNode->getChild(3)->getChild(0), schema_name)    ||
                 SQL_ISRULE(pNode->getChild(3)->getChild(0), table_name))))
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
            if(!InsertJoinConnection(_pView,pNode->getChild(4)->getChild(1), eJoinType))
                return sal_False;
        }
        else
            return sal_False;

        return sal_True;
    }
    //------------------------------------------------------------------------------
    void InitFromParseNodeImpl(OQueryDesignView* _pView,OSelectionBrowseBox* _pSelectionBrw)
    {
        _pSelectionBrw->PreFill();
        _pSelectionBrw->Fill();

        OQueryController* pController = static_cast<OQueryController*>(_pView->getController());
        ::connectivity::OSQLParseTreeIterator& aIterator = pController->getParseIterator();
        const ::connectivity::OSQLParseNode* pParseTree = aIterator.getParseTree();
        const ::connectivity::OSQLParseNode* pTableRefCommaList = 0;

        if (pParseTree)
        {
            if (!pController->isEsacpeProcessing())
            {
                WarningBox( _pView, ModuleRes(WARN_QRY_NATIVE) ).Execute();
            }
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
                                ::dbaui::composeTableName(xMetaData,Reference<XPropertySet>(xTable,UNO_QUERY),sComposedName,sal_False);
                                sAlias = aIter->first;
                                if(aKeyComp(sComposedName,aIter->first))
                                {
                                    ::rtl::OUString sCatalog,sSchema,sTable;
                                    ::dbtools::qualifiedNameComponents(xMetaData,sComposedName,sCatalog,sSchema,sTable);
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

                            FillOuterJoins(_pView,pParseTree->getChild(3)->getChild(0)->getChild(1));

                            // check if we have a distinct statement
                            if(SQL_ISTOKEN(pParseTree->getChild(1),DISTINCT))
                            {
                                pController->setDistinct(sal_True);
                                pController->InvalidateFeature(ID_BROWSER_QUERY_DISTINCT_VALUES);
                            }
                            if (!InstallFields(_pView,pParseTree, pTableView->GetTabWinMap()))
                            {
                                // GetSelectionCriteria mu"s vor GetHavingCriteria aufgerufen werden
                                int nLevel=0;

                                GetSelectionCriteria(_pView,_pSelectionBrw,pParseTree,nLevel,sal_True);
                                GetGroupCriteria(_pView,_pSelectionBrw,pParseTree);
                                GetHavingCriteria(_pView,_pSelectionBrw,pParseTree,nLevel);
                                GetOrderCriteria(_pView,_pSelectionBrw,pParseTree);
                                // now we have to insert the fields which aren't in the statement
                                OTableFields& rUnUsedFields = pController->getUnUsedFields();
                                for(OTableFields::iterator aIter = rUnUsedFields.begin();aIter != rUnUsedFields.end();++aIter)
                                    if(_pSelectionBrw->InsertField(*aIter,-1,sal_False,sal_False).isValid())
                                        (*aIter) = NULL;
                                OTableFields().swap( rUnUsedFields );
                            }
                        }
                        else
                        {
                            ErrorBox aBox(_pView, ModuleRes( ERR_QRY_TOO_MANY_TABLES));
                            aBox.Execute();
                        }
                    }
                    catch(SQLException&)
                    {
                        OSL_ASSERT(!"getMaxTablesInSelect!");
                    }
                }
            }
            else
            {
                ErrorBox aBox(_pView, ModuleRes( ERR_QRY_NOSELECT));
                aBox.Execute();
            }
        }

        // Durch das Neuerzeugen wurden wieder Undo-Actions in den Manager gestellt
        pController->getUndoMgr()->Clear();
        _pSelectionBrw->Invalidate();
    }
    //------------------------------------------------------------------------------
    int InstallFields(  OQueryDesignView* _pView,
                        const ::connectivity::OSQLParseNode* pNode,
                        OJoinTableView::OTableWindowMap* pTabList )
    {
        if( pNode==0 || !SQL_ISRULE(pNode,select_statement))
        {
            ErrorBox    aBox( _pView, ModuleRes( ERR_QRY_NOSELECT ) );
            aBox.Execute();
            return 1;
        }

        ::connectivity::OSQLParseNode* pParseTree = pNode->getChild(2);
        sal_Bool bFirstField = sal_True;    // bei der Initialisierung muß auf alle Faelle das erste Feld neu aktiviert werden

        if(pParseTree->isRule() && SQL_ISPUNCTUATION(pParseTree->getChild(0),"*"))
        {
            // SELECT * ...

            OTableFieldDescRef  aInfo = new OTableFieldDesc();
            OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
            for(;aIter != pTabList->end();++aIter)
            {
                OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);

                if (pTabWin->ExistsField( ::rtl::OUString::createFromAscii("*"), aInfo ))
                {
                    if( !_pView->InsertField(aInfo, sal_True, bFirstField) )
                        return 1;
                    bFirstField = sal_False;
                }
            }

            // Einfach alle Columns der Datei direkt uebernehmen:
        }
        else if (SQL_ISRULE(pParseTree,scalar_exp_commalist) )
        {
            // SELECT column, ...

            ::rtl::OUString aColumnName,aTableRange;
            for (sal_uInt32 i = 0; i < pParseTree->count(); i++)
            {
                ::connectivity::OSQLParseNode * pColumnRef = pParseTree->getChild(i);

                if (SQL_ISRULE(pColumnRef,select_sublist))
                {
                    OTableFieldDescRef aInfo = new OTableFieldDesc();
                    OJoinTableView::OTableWindowMap::iterator aIter = pTabList->begin();
                    for(;aIter != pTabList->end();++aIter)
                    {
                        OQueryTableWindow* pTabWin = static_cast<OQueryTableWindow*>(aIter->second);

                        if (pTabWin->ExistsField( ::rtl::OUString::createFromAscii("*"), aInfo ))
                        {
                            if( !_pView->InsertField(aInfo, sal_True, bFirstField) )
                                return 1;

                            bFirstField = sal_False;
                        }
                    }
                }
                else if (SQL_ISRULE(pColumnRef,derived_column))
                {
                    OQueryController* pController = static_cast<OQueryController*>(_pView->getController());
                    Reference< XConnection> xConnection = pController->getConnection();
                    if(xConnection.is())
                    {
                        Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                        ::rtl::OUString aColumnAlias(pController->getParseIterator().getColumnAlias(pColumnRef)); // kann leer sein
                        pColumnRef = pColumnRef->getChild(0);
                        if (SQL_ISRULE(pColumnRef,column_ref))
                        {
                            OTableFieldDescRef aInfo = new OTableFieldDesc();
                            switch(InsertColumnRef(_pView,pColumnRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList))
                            {
                                case 5:
                                    ErrorBox( _pView, ModuleRes( ERR_QRY_AMB_FIELD ) ).Execute();
                                    break;
                                default:
                                    if( !_pView->InsertField(aInfo, sal_True, bFirstField) )
                                        return 1;
                                bFirstField = sal_False;
                            }
                        }
                        else if(SQL_ISRULE(pColumnRef,general_set_fct)  || SQL_ISRULE(pColumnRef,set_fct_spec)  ||
                                SQL_ISRULE(pColumnRef,position_exp)     || SQL_ISRULE(pColumnRef,extract_exp)   ||
                                SQL_ISRULE(pColumnRef,length_exp)       || SQL_ISRULE(pColumnRef,char_value_fct))
                        {
                            OTableFieldDescRef  aInfo = new OTableFieldDesc();
                            ::rtl::OUString aColumns;
                            pColumnRef->parseNodeToStr( aColumns,
                                                        xMetaData,
                                                        &pController->getParser()->getContext(),
                                                        sal_True,
                                                        sal_True); // quote is to true because we need quoted elements inside the function

                            ::connectivity::OSQLParseNode * pParamRef = pColumnRef->getChild(pColumnRef->count()-2);
                            if (SQL_ISRULE(pColumnRef,general_set_fct)
                                && SQL_ISRULE(pParamRef = pColumnRef->getChild(pColumnRef->count()-2),column_ref))
                            {
                                // Parameter auf Columnref pr"ufen
                                switch(InsertColumnRef(_pView,pParamRef,aColumnName,aColumnAlias,aTableRange,aInfo,pTabList))
                                {
                                    case 5:
                                        ErrorBox( _pView, ModuleRes( ERR_QRY_AMB_FIELD ) ).Execute();
                                        break;
                                }
                            }
                            else
                            {
                                if(pParamRef && pParamRef->getTokenValue().toChar() == '*')
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
                                    aInfo->SetDataType(DataType::DOUBLE);
                                    aInfo->SetFieldType(TAB_NORMAL_FIELD);
                                    aInfo->SetField(aColumns);

                                }
                                aInfo->SetTabWindow(NULL);
                                aInfo->SetFieldAlias(aColumnAlias);
                            }

                            if(SQL_ISRULE(pColumnRef,general_set_fct))
                            {
                                aInfo->SetFunctionType(FKT_AGGREGATE);
                                String aCol(aColumns);
                                aInfo->SetFunction(aCol.GetToken(0,'(').EraseTrailingChars(' '));
                            }
                            else
                                aInfo->SetFunctionType(FKT_OTHER);

                            if( !_pView->InsertField(aInfo, sal_True, bFirstField) )
                                return 1;
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

                            OTableFieldDescRef  aInfo = new OTableFieldDesc();
                            aInfo->SetDataType(DataType::DOUBLE);
                            aInfo->SetFieldType(TAB_NORMAL_FIELD);
                            aInfo->SetTabWindow(NULL);
                            aInfo->SetField(aColumns);
                            aInfo->SetFieldAlias(aColumnAlias);
                            aInfo->SetFunctionType(FKT_OTHER);

                            if( !_pView->InsertField(aInfo, sal_True, bFirstField) )
                                return 1;
                            bFirstField = sal_False;
                        }
                    }
                }
            }
        }
        else
        {
            ErrorBox( _pView, ModuleRes( ERR_QRY_SYNTAX ) ).Execute();
            return 4;
        }

        return 0;
    }
    //------------------------------------------------------------------------------
    void GetOrderCriteria(  OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pParseRoot )
    {
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
                    if(FillDragInfo(_pView,pChild->getChild(0),aDragLeft))
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
                            if(pEntry.isValid() && pEntry->GetFieldAlias() == aColumnName.getStr())
                                pEntry->SetOrderDir( eOrderDir );
                        }
                    }
                }
                else if(SQL_ISRULE(pChild->getChild(0),general_set_fct) &&
                        SQL_ISRULE(pParamRef = pChild->getChild(0)->getChild(pChild->getChild(0)->count()-2),column_ref) &&
                        FillDragInfo(_pView,pParamRef,aDragLeft))
                    _pSelectionBrw->AddOrder( aDragLeft, eOrderDir, nPos);
            }
        }
    }
    //------------------------------------------------------------------------------
    void GetHavingCriteria( OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot,
                            int &rLevel )
    {
        if (!pSelectRoot->getChild(3)->getChild(3)->isLeaf())
            GetORCriteria(_pView,_pSelectionBrw,pSelectRoot->getChild(3)->getChild(3)->getChild(1),rLevel, sal_True);
    }
    //------------------------------------------------------------------------------
    void GetGroupCriteria(  OQueryDesignView* _pView,
                            OSelectionBrowseBox* _pSelectionBrw,
                            const ::connectivity::OSQLParseNode* pSelectRoot )
    {
        if (!pSelectRoot->getChild(3)->getChild(2)->isLeaf())
        {
            ::connectivity::OSQLParseNode* pGroupBy = pSelectRoot->getChild(3)->getChild(2)->getChild(2);
            OTableFieldDescRef aDragInfo = new OTableFieldDesc();
            for( sal_uInt32 i=0 ; i < pGroupBy->count() ; i++ )
            {
                ::connectivity::OSQLParseNode* pColumnRef = pGroupBy->getChild( i );
                if(SQL_ISRULE(pColumnRef,column_ref))
                {
                    if(FillDragInfo(_pView,pColumnRef,aDragInfo))
                    {
                        aDragInfo->SetGroupBy(sal_True);
                        _pSelectionBrw->AddGroupBy(aDragInfo);
                    }
                }
            }
        }
    }
    //------------------------------------------------------------------------------
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
{
    try
    {
        String sLanguage, sCountry;
        ConvertLanguageToIsoNames(Window::GetSettings().GetLanguage(), sLanguage, sCountry);
        m_aLocale = Locale(sLanguage, sCountry, ::rtl::OUString());

        Reference< XLocaleData> xLocaleData = Reference<XLocaleData>(_rFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.LocaleData")),UNO_QUERY);
        LocaleDataItem aData = xLocaleData->getLocaleItem(m_aLocale);
        m_sDecimalSep = aData.decimalSeparator;
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
    delete m_pSelectionBox;
}
//------------------------------------------------------------------------------
IMPL_LINK( OQueryDesignView, SplitHdl, void*, p )
{
    if (!static_cast<OQueryController*>(getController())->isReadOnly())
    {
        long nTest = m_aSplitter.GetPosPixel().Y();
        m_aSplitter.SetPosPixel( Point( m_aSplitter.GetPosPixel().X(),m_aSplitter.GetSplitPosPixel() ) );
        if(!static_cast<OQueryController*>(getController())->isReadOnly())
        {
            static_cast<OQueryController*>(getController())->setSplitPos(m_aSplitter.GetSplitPosPixel());
            static_cast<OQueryController*>(getController())->setModified();
        }
        Resize();
    }
    return 0L;
}
// -------------------------------------------------------------------------
void OQueryDesignView::Construct()
{
    m_pTableView    = new OQueryTableView(m_pScrollWindow,this);
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
    if( nSplitPos == -1 || nSplitPos >= aPlaygroundSize.Height())
    {
        nSplitPos = sal_Int32(aPlaygroundSize.Height()*0.6);
        static_cast<OQueryController*>(getController())->setSplitPos(nSplitPos);
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
            bAllowed = sal_True;
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
            bAllowed = sal_True;
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
sal_Bool OQueryDesignView::InsertField( const OTableFieldDescRef& rInfo, sal_Bool bVis, sal_Bool bActivate)
{
    if(!m_pSelectionBox->InsertField( rInfo, -1,bVis, bActivate ).isValid())
    {
        ErrorBox aBox(this, ModuleRes( ERR_QRY_TOO_MANY_COLUMNS));
        aBox.Execute();
        return sal_False;
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::getColWidth( const ::rtl::OUString& rAliasName, const ::rtl::OUString& rFieldName, sal_uInt32& nWidth )
{
    OTableFields& aFields = static_cast<OQueryController*>(getController())->getTableFieldDesc();
    OTableFields::iterator aIter = aFields.begin();
    for(;aIter != aFields.end();++aIter)
    {
        if( rAliasName == (*aIter)->GetFieldAlias())
        {
            if( rFieldName == (*aIter)->GetField())
            {
                nWidth = (*aIter)->GetColWidth();
                return sal_True;
            }
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
    BOOL bHandled = FALSE;
    switch (rNEvt.GetType())
    {
        case EVENT_KEYINPUT:
            {
                const KeyCode& rCode = rNEvt.GetKeyEvent()->GetKeyCode();
                if (!rCode.IsMod1() && !rCode.IsMod2() && rCode.GetCode() == KEY_F6)
                {
                    if ( m_pSelectionBox && m_pTableView && m_pTableView->HasChildPathFocus() && !rCode.IsShift() )
                    {
                        m_pSelectionBox->GrabFocus();
                        m_eChildFocus = SELECTION;
                        bHandled = sal_True;
                    }
                    else if ( m_pTableView && m_pSelectionBox && m_pSelectionBox->HasChildPathFocus() && rCode.IsShift() )
                    {
                        m_pTableView->GrabTabWinFocus();
                        m_eChildFocus = TABLEVIEW;
                        bHandled = sal_True;
                    }
                    else
                        m_bInKeyEvent = sal_True;
                }
            }
            break;
    }

    return bHandled ? 1L : OQueryView::PreNotify(rNEvt);
}
//------------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// check if the statement is correct when not returning false
sal_Bool OQueryDesignView::checkStatement()
{
    return m_pSelectionBox->Save(); // a error occured so we return no
}
//-------------------------------------------------------------------------------
::rtl::OUString OQueryDesignView::getStatement()
{
    OQueryController* pController = static_cast<OQueryController*>(getController());
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
        if(pEntryField->GetField().getLength() && pEntryField->IsVisible())
            nFieldcount++;
        else if(pEntryField->GetField().getLength()         &&
                !pEntryField->HasCriteria()                 &&
                pEntryField->GetFunctionType() == FKT_NONE  &&
                pEntryField->GetOrderDir() == ORDER_NONE    &&
                !pEntryField->GetFunction().getLength())
            rUnUsedFields.push_back(pEntryField);
    }
    if(!nFieldcount)    // keine Felder sichtbar also zur"uck
        return ::rtl::OUString();

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

    ::std::vector<OTableConnection*>*   pConnList  = m_pTableView->GetTabConnList();
    ::rtl::OUString aTableListStr(GenerateFromClause(this,pTabList,pConnList));
    DBG_ASSERT(aTableListStr.getLength(), "OQueryDesignView::getStatement() : unerwartet : habe Felder, aber keine Tabellen !");
    // wenn es Felder gibt, koennen die nur durch Einfuegen aus einer schon existenten Tabelle entstanden sein; wenn andererseits
    // eine Tabelle geloescht wird, verschwinden auch die zugehoerigen Felder -> ergo KANN es das nicht geben, dass Felder
    // existieren, aber keine Tabellen (und aFieldListStr hat schon eine Laenge, das stelle ich oben sicher)
    ::rtl::OUString aHavingStr,aCriteriaListStr;
    // ----------------- Kriterien aufbauen ----------------------
    if (!GenerateCriterias(this,aCriteriaListStr,aHavingStr,rFieldList, nTabcount > 1))
        return ::rtl::OUString();

    ::rtl::OUString aJoinCrit;
    GenerateInnerJoinCriterias(this,aJoinCrit,pConnList);
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
    Reference<XConnection> xConnection = pController->getConnection();
    Reference<XDatabaseMetaData> xMeta;
    if(xConnection.is())
        xMeta = xConnection->getMetaData();
    sal_Bool bUseAlias = nTabcount > 1;
    if(xMeta.is())
    {
        bUseAlias = bUseAlias || !xMeta->supportsGroupByUnrelated();
    }
    aSqlCmd += GenerateGroupBy(this,rFieldList,bUseAlias);
    // ----------------- having Anh"angen ------------
    if(aHavingStr.getLength())
    {
        aSqlCmd += ::rtl::OUString::createFromAscii(" HAVING ");
        aSqlCmd += aHavingStr;
    }
    // ----------------- Sortierung aufbauen und Anh"angen ------------
    aSqlCmd += GenerateOrder(this,rFieldList,nTabcount > 1);

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
    if(pEntry->GetFunctionType() == FKT_OTHER || pEntry->GetFunctionType() == FKT_AGGREGATE)
    {
        // we have a function here so we have to distinguish the type of return value
        String sFunction;
        if(pEntry->GetFunctionType() == FKT_AGGREGATE)
            sFunction = pEntry->GetFunction();
        else
            sFunction = pEntry->GetField();

        if(sFunction.GetTokenCount('(') > 1)
            sFunction = sFunction.GetToken(0,'('); // this should be the name of the function
        OSL_ENSURE(sFunction.Len(),"Function name is empty!");

        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionReturnType(sFunction,&pParser->getContext());

        parse::OParseColumn* pColumn = new parse::OParseColumn( pEntry->GetField(),
                                                                ::rtl::OUString(),
                                                                ::rtl::OUString(),
                                                                ColumnValue::NULLABLE_UNKNOWN,
                                                                0,
                                                                0,
                                                                nType,
                                                                sal_False,
                                                                sal_False,
                                                                xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
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
    // if true then we come from a PreNotify with KeyEvent F6
    if ( m_bInKeyEvent )
    {
        switch( m_eChildFocus )
        {
            case TABLEVIEW:
                m_pSelectionBox->GrabFocus();
                break;
            default:
                m_pTableView->GrabTabWinFocus();
        }
        m_bInKeyEvent = sal_False;
    }

    // set focus if no one has the focus
    if (m_pTableView && !m_pTableView->HasChildPathFocus() &&
        m_pSelectionBox && !m_pSelectionBox->HasChildPathFocus())
    {
        m_pTableView->GrabTabWinFocus();
    }

    if( m_pTableView && m_pTableView->HasChildPathFocus() )
    {
        if(m_pSelectionBox->IsVisible())
            m_pSelectionBox->DeactivateCell();
        m_eChildFocus = TABLEVIEW;
    }
    else if( m_pSelectionBox && m_pSelectionBox->HasChildPathFocus() )
    {
        m_pSelectionBox->ActivateCell(m_pSelectionBox->GetCurRow(), m_pSelectionBox->GetCurColumnId());
        m_eChildFocus = SELECTION;
    }
}
// -----------------------------------------------------------------------------
void OQueryDesignView::reset()
{
    m_pTableView->ClearAll();
    m_pTableView->ReSync();
    m_pSelectionBox->PreFill();
    m_pSelectionBox->SetReadOnly(static_cast<OQueryController*>(getController())->isReadOnly());
    m_pSelectionBox->Fill();
}
// -----------------------------------------------------------------------------
void OQueryDesignView::setNoneVisbleRow(sal_Int32 _nRows)
{
    m_pSelectionBox->SetNoneVisbleRow(_nRows);
}
// -----------------------------------------------------------------------------
void OQueryDesignView::InitFromParseNode()
{
    InitFromParseNodeImpl(this,m_pSelectionBox);
}
// -----------------------------------------------------------------------------
sal_Bool OQueryDesignView::HasTable() const
{
    return m_pTableView->GetTabWinMap()->size() != 0;
}
// -----------------------------------------------------------------------------

