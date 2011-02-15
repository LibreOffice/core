/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#define DBAUI_QUERYTABLEVIEW_HXX

#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif

namespace dbaui
{

    struct TabWinsChangeNotification
    {
        enum ACTION_TYPE    { AT_ADDED_WIN, AT_REMOVED_WIN };
        ACTION_TYPE         atActionPerformed;
        String              strAffectedTable;

        TabWinsChangeNotification(ACTION_TYPE at, const String& str) : atActionPerformed(at), strAffectedTable(str) { }
    };

    //========================================================================
    class OQueryTabWinUndoAct;
    class OQueryTabConnUndoAction;
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

        /** called when init fails at the tablewindowdata because the m_xTable object could not provide columns, but no
            exception was thrown. Expected to throw.
        */
        virtual void    onNoColumns_throw();

        virtual bool supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const;
    public:
        OQueryTableView(Window* pParent,OQueryDesignView* pView);
        virtual ~OQueryTableView();

        // Basisklasse ueberschrieben : Fenster kreieren und loeschen
        // (eigentlich nicht wirklich LOESCHEN, es geht in die Verantwortung einer UNDO-Action ueber)
        virtual void AddTabWin( const ::rtl::OUString& _rTableName, const ::rtl::OUString& _rAliasName, sal_Bool bNewTable = sal_False );
        virtual void RemoveTabWin(OTableWindow* pTabWin);

        // und ein AddTabWin, das einen Alias vorgibt
        void    AddTabWin(const ::rtl::OUString& strDatabase, const ::rtl::OUString& strTableName, const ::rtl::OUString& strAlias, sal_Bool bNewTable = sal_False);
        // TabWin suchen
        OQueryTableWindow*  FindTable(const String& rAliasName);
        sal_Bool            FindTableFromField(const String& rFieldName, OTableFieldDescRef& rInfo, sal_uInt16& rCnt);

        // Basisklasse ueberschrieben : Connections kreieren und loeschen
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest);

        virtual bool RemoveConnection( OTableConnection* _pConn ,sal_Bool _bDelete);

        // Transfer von Connections von/zu einer UndoAction
        void GetConnection(OQueryTableConnection* pConn);
            // Einfuegen einer Connection in meine Struktur
        void DropConnection(OQueryTableConnection* pConn);
            // Entfernen einer Connection aus meiner Struktur

            // das resultiert effektiv in einem voelligen Leeren des Abfrageentwurfs, da alle Fenster versteckt werden, und dabei
            // natuerlich alle Connections an diesen Fenstern und alle Abfrage-Spalten, die auf diesen Tabellen basierten.

        // TabWin anzeigen oder verstecken (NICHT kreieren oder loeschen)
        sal_Bool    ShowTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction,sal_Bool _bAppend);
        void    HideTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction);

        // Sichbarkeit eines TabWins sicherstellen (+ Invalidieren der Connections)
        virtual void EnsureVisible(const OTableWindow* _pWin);

        // wieviel Tabellen mit einem bestimmten Namen habe ich schon ?
        sal_Int32   CountTableAlias(const String& rName, sal_Int32& rMax);

        // ein Feld einfuegen (wird einfach an das Elter weitergereicht
        void InsertField(const OTableFieldDescRef& rInfo);

        // alles (TabWins, Connections) neu aufbauen (PRECONDITION : vorher wurde ClearAll gerufen)
        virtual void ReSync();
        // alles (TabWins, Connections) loeschen, und zwar hart, es erfolgen also keinerlei Notifications
        virtual void ClearAll();

        // wird vom AddTabDlg benutzt, um festzustellen, ob noch Tabellen hinzugefuegt werden duerfen
        //virtual sal_Bool IsAddAllowed();

        // eine neu Connection bekanntgeben und einfuegen lassen, wenn nicht schon existent
        void NotifyTabConnection(const OQueryTableConnection& rNewConn, sal_Bool _bCreateUndoAction = sal_True);

        Link    SetTabWinsChangeHandler(const Link& lnk) { Link lnkRet = m_lnkTabWinsChangeHandler; m_lnkTabWinsChangeHandler = lnk; return lnkRet; }
            // der Handler bekommt einen Zeiger auf eine TabWinsChangeNotification-Struktur

        sal_Bool ExistsAVisitedConn(const OQueryTableWindow* pFrom) const;

        virtual OTableWindowData* CreateImpl(const ::rtl::OUString& _rComposedName
                                            ,const ::rtl::OUString& _sTableName
                                            ,const ::rtl::OUString& _rWinName);

        /** createNewConnection opens the join dialog and allows to create a new join connection
        */
        void createNewConnection();

    private:
        using OJoinTableView::EnsureVisible;
    };
}
#endif // DBAUI_QUERYTABLEVIEW_HXX





