/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryTableView.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:28:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

        virtual OTableWindow* createWindow(OTableWindowData* _pData);
    public:
        TYPEINFO();
        OQueryTableView(Window* pParent,OQueryDesignView* pView);
        virtual ~OQueryTableView();

        // Basisklasse ueberschrieben : Fenster kreieren und loeschen
        // (eigentlich nicht wirklich LOESCHEN, es geht in die Verantwortung einer UNDO-Action ueber)
        virtual void AddTabWin( const ::rtl::OUString& _rTableName, const ::rtl::OUString& _rAliasName, BOOL bNewTable = FALSE );
        virtual void RemoveTabWin(OTableWindow* pTabWin);

        // und ein AddTabWin, das einen Alias vorgibt
        void    AddTabWin(const ::rtl::OUString& strDatabase, const ::rtl::OUString& strTableName, const ::rtl::OUString& strAlias, BOOL bNewTable = FALSE);
        // Fenster an Hand des AliasNamen loeschen (zieht sich natuerlich auf voriges RemoveTabWin zurueck)
        BOOL    RemoveTabWin(const String& rAliasName);

        // TabWin suchen
        OQueryTableWindow*  FindTable(const String& rAliasName);
        BOOL            FindTableFromField(const String& rFieldName, OTableFieldDescRef& rInfo, USHORT& rCnt);

        // Basisklasse ueberschrieben : Connections kreieren und loeschen
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest);

        virtual ::std::vector<OTableConnection*>::const_iterator RemoveConnection( OTableConnection* _pConn ,sal_Bool _bDelete);

        // Transfer von Connections von/zu einer UndoAction
        void GetConnection(OQueryTableConnection* pConn);
            // Einfuegen einer Connection in meine Struktur
        void DropConnection(OQueryTableConnection* pConn);
            // Entfernen einer Connection aus meiner Struktur

            // das resultiert effektiv in einem voelligen Leeren des Abfrageentwurfs, da alle Fenster versteckt werden, und dabei
            // natuerlich alle Connections an diesen Fenstern und alle Abfrage-Spalten, die auf diesen Tabellen basierten.

        // TabWin anzeigen oder verstecken (NICHT kreieren oder loeschen)
        BOOL    ShowTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction,sal_Bool _bAppend);
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
        //virtual BOOL IsAddAllowed();

        // eine neu Connection bekanntgeben und einfuegen lassen, wenn nicht schon existent
        void NotifyTabConnection(const OQueryTableConnection& rNewConn, BOOL _bCreateUndoAction = TRUE);

        Link    SetTabWinsChangeHandler(const Link& lnk) { Link lnkRet = m_lnkTabWinsChangeHandler; m_lnkTabWinsChangeHandler = lnk; return lnkRet; }
            // der Handler bekommt einen Zeiger auf eine TabWinsChangeNotification-Struktur

        BOOL ExistsAVisitedConn(const OQueryTableWindow* pFrom) const;

        virtual OTableWindowData* CreateImpl(const ::rtl::OUString& _rComposedName,
                                             const ::rtl::OUString& _rWinName);

        /** createNewConnection opens the join dialog and allows to create a new join connection
        */
        void createNewConnection();

    private:
        using OJoinTableView::EnsureVisible;
    };
}
#endif // DBAUI_QUERYTABLEVIEW_HXX





