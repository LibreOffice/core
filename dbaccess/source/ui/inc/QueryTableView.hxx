/*************************************************************************
 *
 *  $RCSfile: QueryTableView.hxx,v $
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
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#define DBAUI_QUERYTABLEVIEW_HXX

#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
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
    class OTableFieldDesc;
    class OQueryTabWinUndoAct;
    class OQueryTabConnUndoAction;
    class OQueryTableConnection;
    class OQueryTableWindow;
    class OQueryDesignView;
    class OQueryTableView : public OJoinTableView
    {
        Link    m_lnkTabWinsChangeHandler;

        void addConnections(const OQueryTableWindow* _pSource,const OQueryTableWindow* _pDest,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxForeignKeyColumns);
    protected:
        virtual void ConnDoubleClicked(OTableConnection* pConnection);
        virtual void KeyInput(const KeyEvent& rEvt);

        // wird nach Verschieben/Groessenaenderung der TabWins aufgerufen (die Standardimplementation reicht die neuen Daten einfach
        // an die Daten des Wins weiter)
        virtual void TabWinMoved(OTableWindow* pWhich, const Point& ptOldPosition);
        virtual void TabWinSized(OTableWindow* ptWhich, const Point& ptOldPosition, const Size& szOldSize);

    public:
        TYPEINFO();
        OQueryTableView(Window* pParent,OQueryDesignView* pView);
        virtual ~OQueryTableView();

        // Basisklasse ueberschrieben : Fenster kreieren und loeschen
        // (eigentlich nicht wirklich LOESCHEN, es geht in die Verantwortung einer UNDO-Action ueber)
        virtual void AddTabWin(const ::rtl::OUString& strDatabase, const ::rtl::OUString& strTableName, BOOL bNewTable = FALSE);
        virtual void RemoveTabWin(OTableWindow* pTabWin);

        // und ein AddTabWin, das einen Alias vorgibt
        void    AddTabWin(const ::rtl::OUString& strDatabase, const ::rtl::OUString& strTableName, const ::rtl::OUString& strAlias, BOOL bNewTable = FALSE);
        // Fenster an Hand des AliasNamen loeschen (zieht sich natuerlich auf voriges RemoveTabWin zurueck)
        BOOL    RemoveTabWin(const String& rAliasName);

        // TabWin suchen
        OQueryTableWindow*  FindTable(const String& rAliasName);
        BOOL            FindTableFromField(const String& rFieldName, OTableFieldDesc& rInfo, USHORT& rCnt);

        // Basisklasse ueberschrieben : Connections kreieren und loeschen
        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest);
        virtual BOOL RemoveConnection(OTableConnection* pConn);

        // Transfer von Connections von/zu einer UndoAction
        void GetConnection(OQueryTableConnection* pConn);
            // Einfuegen einer Connection in meine Struktur
        void DropConnection(OQueryTableConnection* pConn);
            // Entfernen einer Connection aus meiner Struktur

        // alle TabWins verstecken (NICHT loeschen, sie werden in eine Undo-Action gepackt)
        void    HideTabWins();
            // das resultiert effektiv in einem voelligen Leeren des Abfrageentwurfs, da alle Fenster versteckt werden, und dabei
            // natuerlich alle Connections an diesen Fenstern und alle Abfrage-Spalten, die auf diesen Tabellen basierten.

        // TabWin anzeigen oder verstecken (NICHT kreieren oder loeschen)
        BOOL    ShowTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction,sal_Bool _bAppend);
        void    HideTabWin(OQueryTableWindow* pTabWin, OQueryTabWinUndoAct* pUndoAction);

        // Sichbarkeit eines TabWins sicherstellen (+ Invalidieren der Connections)
        void    EnsureVisible(const OTableWindow* _pWin);

        // wieviel Tabellen mit einem bestimmten Namen habe ich schon ?
        USHORT  CountTableAlias(const String& rName, USHORT& rMax);

        // ein Feld einfuegen (wird einfach an das Elter weitergereicht
        void InsertField(const OTableFieldDesc& rInfo);

        // alles (TabWins, Connections) neu aufbauen (PRECONDITION : vorher wurde ClearAll gerufen)
        virtual void ReSync();
        // alles (TabWins, Connections) loeschen, und zwar hart, es erfolgen also keinerlei Notifications
        virtual void ClearAll();

        // wird vom AddTabDlg benutzt, um festzustellen, ob noch Tabellen hinzugefuegt werden duerfen
        //virtual BOOL IsAddAllowed();

        // eine neu Connection bekanntgeben und einfuegen lassen, wenn nicht schon existent
        void NotifyTabConnection(const OQueryTableConnection& rNewConn, BOOL _bCreateUndoAction = TRUE);

        // fuer Droppen von Tabellen
        BOOL Drop(const DropEvent& rEvt);
        BOOL QueryDrop(DropEvent& rEvt);

        Link    SetTabWinsChangeHandler(const Link& lnk) { Link lnkRet = m_lnkTabWinsChangeHandler; m_lnkTabWinsChangeHandler = lnk; return lnkRet; }
            // der Handler bekommt einen Zeiger auf eine TabWinsChangeNotification-Struktur

        BOOL ExistsAVisitedConn(const OQueryTableWindow* pFrom) const;

        virtual OTableWindowData* CreateImpl(const ::rtl::OUString& _rComposedName,
                                             const ::rtl::OUString& _rWinName);
    };
}
#endif // DBAUI_QUERYTABLEVIEW_HXX





