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


#ifndef DBAUI_RELATION_TABLEVIEW_HXX
#define DBAUI_RELATION_TABLEVIEW_HXX

#include "JoinTableView.hxx"
#include <comphelper/containermultiplexer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>

namespace dbaui
{
    class ORelationDesignView;
    class ORelationTableView :  public ::cppu::BaseMutex
                            ,   public OJoinTableView
                            ,   public ::comphelper::OContainerListener
    {
        OTableConnection*                m_pExistingConnection; // is set when a connection was draged on an existing connection
        TTableConnectionData::value_type m_pCurrentlyTabConnData; // set when we creating a connection with more than one keycolumn
        ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                         m_pContainerListener;
        bool                             m_bInRemove;

        virtual void ConnDoubleClicked( OTableConnection* pConnection );
        virtual void AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rWinName, sal_Bool bNewTable = sal_False);

        virtual OTableWindow* createWindow(const TTableWindowData::value_type& _pData);

        /** determines whether the classes Init method should accept a query name, or only table names
        */
        virtual bool    allowQueries() const;

        // OContainerListener
        virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void _elementRemoved( const  ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);

    public:
        ORelationTableView( Window* pParent, ORelationDesignView* pView );
        virtual ~ORelationTableView();

        virtual void RemoveTabWin( OTableWindow* pTabWin );
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest);
        virtual bool RemoveConnection(OTableConnection* pConn,sal_Bool _bDelete);

        virtual void ReSync();

        void AddNewRelation();
            // reisst den Dialog fuer eine voellig neue Relation hoch
        // wird vom AddTabDlg benutzt, um festzustellen, ob noch Tabellen hinzugefuegt werden duerfen
        virtual sal_Bool IsAddAllowed();

        virtual void lookForUiActivities();
    };
}
#endif // DBAUI_RELATION_TABLEVIEW_HXX


