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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYTABLEVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYTABLEVIEW_HXX

#include "JoinTableView.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include "querycontroller.hxx"

namespace dbaui
{
    struct TabWinsChangeNotification
    {
        enum ACTION_TYPE    { AT_ADDED_WIN, AT_REMOVED_WIN };
        ACTION_TYPE         atActionPerformed;
        OUString            strAffectedTable;

        TabWinsChangeNotification(ACTION_TYPE at, const OUString& str) : atActionPerformed(at), strAffectedTable(str) { }
    };

    class OQueryTabWinUndoAct;
    class OQueryTableConnection;
    class OQueryTableWindow;
    class OQueryDesignView;

    class OQueryTableView : public OJoinTableView
    {
    protected:
        virtual void ConnDoubleClicked(OTableConnection* pConnection) override;
        virtual void KeyInput(const KeyEvent& rEvt) override;

        virtual VclPtr<OTableWindow> createWindow(const TTableWindowData::value_type& _pData) override;

        /** called when init fails at the tablewindowdata because the m_xTable
            object could not provide columns, but no exception was thrown.
            Expected to throw. */
        virtual void    onNoColumns_throw() override;

        virtual bool supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const override;

    public:
        OQueryTableView(vcl::Window* pParent,OQueryDesignView* pView);

        /// base class overwritten: create and delete windows
        /// (not really delete, as it becomes an UndoAction)
        bool ContainsTabWin(const OTableWindow& rTabWin); // #i122589# Allow to check if OTableWindow is registered
        virtual void AddTabWin( const OUString& _rTableName, const OUString& _rAliasName, bool bNewTable = false ) override;
        virtual void RemoveTabWin(OTableWindow* pTabWin) override;

        /// AddTabWin, setting an alias
        void    AddTabWin(const OUString& strDatabase, const OUString& strTableName, const OUString& strAlias, bool bNewTable = false);
        /// search TabWin
        OQueryTableWindow*  FindTable(const OUString& rAliasName);
        bool                FindTableFromField(const OUString& rFieldName, OTableFieldDescRef& rInfo, sal_uInt16& rCnt);

        /// base class overwritten: create and delete Connections
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest) override;

        virtual bool RemoveConnection( OTableConnection* _pConn ,bool _bDelete) override;

        // transfer of connections from and to UndoAction

        /// Inserting a Connection the structure
        void GetConnection(OQueryTableConnection* pConn);
        /** Removing a Connection from the structure

            This results effectively in complete reset of request form, as all
            windows are hidden, as are all Connections to these windows and all
            request columns based on those tables */
        void DropConnection(OQueryTableConnection* pConn);

        // show and hide TabWin (NOT create or delete)
        bool ShowTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction, bool _bAppend);
        void HideTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction);

        /// ensure visibility of TabWins (+ and invalidate connections)
        virtual void EnsureVisible(const OTableWindow* _pWin) override;

        /// how many tables with a certain alias do I already have?
        sal_Int32 CountTableAlias(const OUString& rName, sal_Int32& rMax);

        /// insert field (simply passed to parents)
        void InsertField(const OTableFieldDescRef& rInfo);

        /// rebuild everything (TabWins, Connections)
        /// (PRECONDITION: ClearAll was called previously)
        virtual void ReSync() override;

        /// delete everything hard (TabWins, Connections), without any notifications
        virtual void ClearAll() override;

        // used by AddTabDlg to see if tables can still be added
        //virtual sal_Bool IsAddAllowed();

        /// announce new Connection and insert it, if not existing yet
        void NotifyTabConnection(const OQueryTableConnection& rNewConn, bool _bCreateUndoAction = true);

        bool ExistsAVisitedConn(const OQueryTableWindow* pFrom) const;

        virtual OTableWindowData* CreateImpl(const OUString& _rComposedName
                                            ,const OUString& _sTableName
                                            ,const OUString& _rWinName) override;

        /** opens the join dialog and allows to create a new join connection */
        void createNewConnection();

    private:
        using OJoinTableView::EnsureVisible;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
