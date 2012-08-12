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
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#define DBAUI_QUERYTABLEVIEW_HXX

#include "JoinTableView.hxx"
#include <com/sun/star/container/XNameAccess.hpp>
#include "querycontroller.hxx"

namespace dbaui
{
    struct TabWinsChangeNotification
    {
        enum ACTION_TYPE    { AT_ADDED_WIN, AT_REMOVED_WIN };
        ACTION_TYPE         atActionPerformed;
        String              strAffectedTable;

        TabWinsChangeNotification(ACTION_TYPE at, const String& str) : atActionPerformed(at), strAffectedTable(str) { }
    };

    class OQueryTabWinUndoAct;
    class OQueryTableConnection;
    class OQueryTableWindow;
    class OQueryDesignView;

    class OQueryTableView : public OJoinTableView
    {
        Link    m_lnkTabWinsChangeHandler;

    protected:
        virtual void ConnDoubleClicked(OTableConnection* pConnection);
        virtual void KeyInput(const KeyEvent& rEvt);

        virtual OTableWindow* createWindow(const TTableWindowData::value_type& _pData);

        /** called when init fails at the tablewindowdata because the m_xTable
            object could not provide columns, but no exception was thrown.
            Expected to throw. */
        virtual void    onNoColumns_throw();

        virtual bool supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const;

    public:
        OQueryTableView(Window* pParent,OQueryDesignView* pView);
        virtual ~OQueryTableView();

        /// base class overwritten: create and delete windows
        /// (not really delete, as it becomes an UndoAction)
        virtual void AddTabWin( const ::rtl::OUString& _rTableName, const ::rtl::OUString& _rAliasName, sal_Bool bNewTable = sal_False );
        virtual void RemoveTabWin(OTableWindow* pTabWin);

        /// AddTabWin, setting an alias
        void    AddTabWin(const ::rtl::OUString& strDatabase, const ::rtl::OUString& strTableName, const ::rtl::OUString& strAlias, sal_Bool bNewTable = sal_False);
        /// search TabWin
        OQueryTableWindow*  FindTable(const String& rAliasName);
        sal_Bool            FindTableFromField(const String& rFieldName, OTableFieldDescRef& rInfo, sal_uInt16& rCnt);

        /// base class overwritten: create and delete Connections
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest);

        virtual bool RemoveConnection( OTableConnection* _pConn ,sal_Bool _bDelete);

        // transfer of connections from and to UndoAction

        /// Inserting a Connection the structure
        void GetConnection(OQueryTableConnection* pConn);
        /** Removing a Connection from the structure

            This results effectively in complete reset of request form, as all
            windows are hidden, as are all Connections to these windows and all
            request columns based on those tables */
        void DropConnection(OQueryTableConnection* pConn);

        // show and hide TabWin (NOT create or delete)
        sal_Bool ShowTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction,sal_Bool _bAppend);
        void HideTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction);

        /// ensure visibility of TabWins (+ and invalidate connections)
        virtual void EnsureVisible(const OTableWindow* _pWin);

        /// how many tables with a certain alias do I already have?
        sal_Int32 CountTableAlias(const String& rName, sal_Int32& rMax);

        /// insert field (simply passed to parents)
        void InsertField(const OTableFieldDescRef& rInfo);

        /// rebuild everything (TabWins, Connections)
        /// (PRECONDITION: ClearAll was called previously)
        virtual void ReSync();

        /// delete everything hard (TabWins, Connections), without any notifications
        virtual void ClearAll();

        // used by AddTabDlg to see if tables can still be added
        //virtual sal_Bool IsAddAllowed();

        /// announce new Connection and insert it, if not existant yet
        void NotifyTabConnection(const OQueryTableConnection& rNewConn, sal_Bool _bCreateUndoAction = sal_True);

        /// @note the Handler receives a pointer to a TabWinsChangeNotification struct
        Link SetTabWinsChangeHandler(const Link& lnk) { Link lnkRet = m_lnkTabWinsChangeHandler; m_lnkTabWinsChangeHandler = lnk; return lnkRet; }

        sal_Bool ExistsAVisitedConn(const OQueryTableWindow* pFrom) const;

        virtual OTableWindowData* CreateImpl(const ::rtl::OUString& _rComposedName
                                            ,const ::rtl::OUString& _sTableName
                                            ,const ::rtl::OUString& _rWinName);

        /** opens the join dialog and allows to create a new join connection */
        void createNewConnection();

    private:
        using OJoinTableView::EnsureVisible;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
