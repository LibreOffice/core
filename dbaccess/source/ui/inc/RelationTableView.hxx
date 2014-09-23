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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONTABLEVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONTABLEVIEW_HXX

#include "JoinTableView.hxx"
#include <comphelper/containermultiplexer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>

namespace dbaui
{
    class ORelationDesignView;

    class ORelationTableView : public ::cppu::BaseMutex,
                               public OJoinTableView,
                               public ::comphelper::OContainerListener
    {
        OTableConnection* m_pExistingConnection; ///< is set when a connection was draged on an existing connection
        TTableConnectionData::value_type m_pCurrentlyTabConnData; ///< set when we creating a connection with more than one keycolumn
        ::rtl::Reference< comphelper::OContainerListenerAdapter> m_pContainerListener;
        bool m_bInRemove;

        virtual void ConnDoubleClicked( OTableConnection* pConnection ) SAL_OVERRIDE;
        virtual void AddTabWin(const OUString& _rComposedName, const OUString& rWinName, bool bNewTable = false) SAL_OVERRIDE;

        virtual OTableWindow* createWindow(const TTableWindowData::value_type& _pData) SAL_OVERRIDE;

        /** determines whether the classes Init method should accept a query
            name, or only table names */
        virtual bool    allowQueries() const SAL_OVERRIDE;

        // OContainerListener
        virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void _elementRemoved( const  ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    public:
        ORelationTableView( vcl::Window* pParent, ORelationDesignView* pView );
        virtual ~ORelationTableView();

        virtual void RemoveTabWin( OTableWindow* pTabWin ) SAL_OVERRIDE;
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest) SAL_OVERRIDE;
        virtual bool RemoveConnection(OTableConnection* pConn,bool _bDelete) SAL_OVERRIDE;

        virtual void ReSync() SAL_OVERRIDE;

        /// Creates a dialogue for a completely new relation.
        void AddNewRelation();

        /// used by AddTabDlg to check if tables can be added
        virtual bool IsAddAllowed() SAL_OVERRIDE;

        virtual void lookForUiActivities() SAL_OVERRIDE;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
