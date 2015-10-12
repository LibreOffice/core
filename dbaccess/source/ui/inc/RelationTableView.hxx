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
        VclPtr<OTableConnection> m_pExistingConnection; ///< is set when a connection was draged on an existing connection
        TTableConnectionData::value_type m_pCurrentlyTabConnData; ///< set when we creating a connection with more than one keycolumn
        ::rtl::Reference< comphelper::OContainerListenerAdapter> m_pContainerListener;
        bool m_bInRemove;

        virtual void ConnDoubleClicked( OTableConnection* pConnection ) override;
        virtual void AddTabWin(const OUString& _rComposedName, const OUString& rWinName, bool bNewTable = false) override;

        virtual VclPtr<OTableWindow> createWindow(const TTableWindowData::value_type& _pData) override;

        /** determines whether the classes Init method should accept a query
            name, or only table names */
        virtual bool    allowQueries() const override;

        // OContainerListener
        virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    public:
        ORelationTableView( vcl::Window* pParent, ORelationDesignView* pView );
        virtual ~ORelationTableView();
        virtual void dispose() override;

        virtual void RemoveTabWin( OTableWindow* pTabWin ) override;
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest) override;
        virtual bool RemoveConnection(OTableConnection* pConn,bool _bDelete) override;

        virtual void ReSync() override;

        /// Creates a dialogue for a completely new relation.
        void AddNewRelation();

        /// used by AddTabDlg to check if tables can be added
        virtual bool IsAddAllowed() override;

        virtual void lookForUiActivities() override;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
