/*************************************************************************
 *
 *  $RCSfile: QueryDesignView.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:10:01 $
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
#define DBAUI_QUERYDESIGNVIEW_HXX

#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

namespace connectivity
{
    class OSQLParseNode;
}

class ComboBox;
namespace dbaui
{
    class OQueryViewSwitch;
    class OAddTableDlg;
    class OQueryTableWindow;
    class OTableFieldDesc;
    class OSelectionBrowseBox;
    class OTableConnection;
    class OQueryTableConnectionData;
    class OConnectionLineData;

    class OQueryDesignView : public OQueryView
    {
        friend class OQueryViewSwitch;

        Splitter                            m_aSplitter;

        ::com::sun::star::lang::Locale      m_aLocale;
        ::rtl::OUString                     m_sDecimalSep;

        OSelectionBrowseBox*    m_pSelectionBox;    // presents the lower window

        ::rtl::OUString QuoteField( const ::rtl::OUString& rValue, sal_Int32 aType );
        void            InitFromParseNode();
        void            GetTable( ::rtl::OUString& rDatabase, ::rtl::OUString& rTable, ::rtl::OUString& rAlias, ::connectivity::OSQLParseNode* pNode );

        void            GenerateInnerJoinCriterias(::rtl::OUString& _rJoinCrit,const ::std::vector<OTableConnection*>*  _pConnList);
        // erzeugt das Group Argument, falls vorhanden
        ::rtl::OUString GenerateGroupBy(::std::vector<OTableFieldDesc*>* pFieldList, sal_Bool bMulti );
        // erzeugt Where und Having Argument, falls vorhanden
        sal_Bool        GenerateCriterias(::rtl::OUString& aRetStr,::rtl::OUString& rHavingStr/*,::rtl::OUString& rOrderStr*/, ::std::vector<OTableFieldDesc*>* pFieldList, sal_Bool bMulti );
        ::rtl::OUString GenerateOrder( ::std::vector<OTableFieldDesc*>* pFieldList , sal_Bool bMulti);
        // Erzeugt die SelectList. bAlias mu"s gesetzt sein, wenn mehr als 1 Tabelle vorhanden ist
        ::rtl::OUString GenerateSelectList(::std::vector<OTableFieldDesc*>* pFieldList,sal_Bool bAlias);
        // Erzeugt die Tabellenliste mit Joins aus pConnList, wenn bJoin == True. pConnList kann == NULL wenn bJoin == sal_False
        ::rtl::OUString GenerateFromClause(const OJoinTableView::OTableWindowMap*   pTabMap,::std::vector<OTableConnection*>*   pConnList);

        ::rtl::OUString BuildACriteria( const ::rtl::OUString& rVal, sal_Int32 aType );
        int             InstallFields( const ::connectivity::OSQLParseNode* pNode, OJoinTableView::OTableWindowMap* pTabList );

        // nLevel gibt die Zeile an in der die Bedingungen eingetragen werden sollen
        int         GetSelectionCriteria(const  ::connectivity::OSQLParseNode* pNode ,int &nLevel,  sal_Bool bJoinWhere=sal_False);
        void        GetHavingCriteria(const  ::connectivity::OSQLParseNode* pNode ,int &nLevel);

        int         GetORCriteria(const ::connectivity::OSQLParseNode * pCondition, int& nLevel,  sal_Bool bHaving = sal_False );
        int         GetANDCriteria(const ::connectivity::OSQLParseNode * pCondition, const int nLevel,  sal_Bool bHaving);
        int         ComparsionPredicate( const ::connectivity::OSQLParseNode * pCondition, const int nLevel,  sal_Bool bHaving );

        void        GetOrderCriteria( const ::connectivity::OSQLParseNode* pNode );
        void        GetGroupCriteria( const ::connectivity::OSQLParseNode* pNode );

        sal_Bool    GetInnerJoinCriteria(const ::connectivity::OSQLParseNode *pCondition);
        void        FillOuterJoins(const ::connectivity::OSQLParseNode* pParseNode);
        sal_Bool    InsertJoin(const ::connectivity::OSQLParseNode *pNode);
        sal_Bool    InsertJoinConnection(const ::connectivity::OSQLParseNode *pNode, const EJoinType& _eJoinType);

        sal_Bool    FillDragInfo(const ::connectivity::OSQLParseNode* pTableRef,OTableFieldDesc& aDragInfo);

        sal_Bool    HasFields();
        ::rtl::OUString     BuildTable(const OQueryTableWindow* pEntryTab);

        ::rtl::OUString     BuildJoin(OQueryTableWindow* pLh, OQueryTableWindow* pRh, OQueryTableConnectionData* pData);
        ::rtl::OUString     BuildJoin(const ::rtl::OUString &rLh, OQueryTableWindow* pRh, OQueryTableConnectionData* pData);
        ::rtl::OUString     BuildJoin(OQueryTableWindow* pLh, const ::rtl::OUString &rRh, OQueryTableConnectionData* pData);
        ::rtl::OUString     BuildJoin(const ::rtl::OUString& rLh, const ::rtl::OUString& rRh, OQueryTableConnectionData* pData);

        ::rtl::OUString     BuildJoinCriteria(::std::vector<OConnectionLineData*>* pLineDataList,OQueryTableConnectionData* pData);

        void        GetNextJoin(OQueryTableConnection* pEntryConn,::rtl::OUString &aJoin,OQueryTableWindow* pEntryTabTo);
        void        JoinCycle(OQueryTableConnection* pEntryConn,::rtl::OUString &rJoin,OQueryTableWindow* pEntryTabTo);

        int         InsertColumnRef(const ::connectivity::OSQLParseNode * pColumnRef,
                                    ::rtl::OUString& aColumnName,const ::rtl::OUString& aColumnAlias,
                                    ::rtl::OUString& aTableRange,OTableFieldDesc& aInfo,
                                    OJoinTableView::OTableWindowMap* pTabList);

        sal_Int32   GetColumnFormatKey(const ::connectivity::OSQLParseNode* pColumnRef);
    public:
        OQueryDesignView(Window* pParent, OQueryController* _pController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~OQueryDesignView();

        virtual sal_Bool isCutAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // clears the whole query
        virtual void clear();
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // set the statement for representation
        virtual void setStatement(const ::rtl::OUString& _rsStatement);
        // returns the current sql statement
        virtual ::rtl::OUString getStatement();
        /// late construction
        virtual void Construct(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel);
        virtual void initialize();
        // window overloads
        virtual long        PreNotify( NotifyEvent& rNEvt );

        BOOL IsAddAllowed();
        sal_Bool isSlotEnabled(sal_Int32 _nSlotId);
        void setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable);

        ::com::sun::star::lang::Locale      getLocale() const           { return m_aLocale;}
        ::rtl::OUString                     getDecimalSeparator() const { return m_sDecimalSep;}

        sal_Bool HasTable() const {return m_pTableView->GetTabWinMap()->size() != 0;}
        sal_Bool InsertField( const OTableFieldDesc& rInfo, sal_Bool bVis=sal_True, sal_Bool bActivate = sal_True);
        // save the position of the table window and the pos of the splitters
        void SaveTabWinUIConfig(OQueryTableWindow* pWin);
        // called when fields are deleted
        void DeleteFields( const ::rtl::OUString& rAliasName );
        // called when a table from tabeview was deleted
        void TableDeleted(const ::rtl::OUString& rAliasName);

        BOOL getColWidth( const ::rtl::OUString& rAliasName, const ::rtl::OUString& rFieldName, ULONG& nWidth );
        void fillValidFields(const ::rtl::OUString& strTableName, ComboBox* pFieldList);
        void zoomTableView(const Fraction& _rFraction);

        void SaveUIConfig();
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeControl(Rectangle& rRect);
        DECL_LINK( SplitHdl, void* );
    };
}
#endif // DBAUI_QUERYDESIGNVIEW_HXX

