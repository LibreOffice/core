/*************************************************************************
 *
 *  $RCSfile: JoinTableView.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-13 16:43:57 $
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
#ifndef DBAUI_JOINTABLEVIEW_HXX
#define DBAUI_JOINTABLEVIEW_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


namespace dbaui
{
    class OTableConnection;
    class OTableWindow;
    struct OJoinExchangeData;
    class OQueryDesignView;

    ////////////////////////////////////////////////////////////////
    // Konstanten fuer das Fensterlayout
    const long TABWIN_SPACING_X  = 17;
    const long TABWIN_SPACING_Y  = 17;

    const long TABWIN_WIDTH_STD  = 120;
    const long TABWIN_HEIGHT_STD = 120;

    const long TABWIN_WIDTH_MIN  = 90;
    const long TABWIN_HEIGHT_MIN = 80;

    // this class conatins only the scrollbars to avoid that the tablewindows clip the scrollbars
    class OJoinTableView;
    class OScrollWindowHelper : public Window
    {
        ScrollBar           m_aHScrollBar;
        ScrollBar           m_aVScrollBar;
        Window*             m_pCornerWindow;
        OJoinTableView*     m_pTableView;

    protected:
        virtual void Resize();
    public:
        OScrollWindowHelper( Window* pParent);
        ~OScrollWindowHelper();

        void setTableView(OJoinTableView* _pTableView);

        // own methods
        ScrollBar* GetHScrollBar() { return &m_aHScrollBar; }
        ScrollBar* GetVScrollBar() { return &m_aVScrollBar; }
    };


    class OJoinTableView : public Window
    {
    public:
        DECLARE_STL_USTRINGACCESS_MAP(OTableWindow*,OTableWindowMap);
    private:
        OTableWindowMap     m_aTableMap;
        ::std::vector<OTableConnection*>    m_vTableConnection;

        Timer               m_aDragScrollTimer;
        Rectangle           m_aDragRect;
        Rectangle           m_aSizingRect;
        Point               m_aDragOffset;
        Point               m_aScrollOffset;
        Point               m_ptPrevDraggingPos;
        Size                m_aOutputSize;


        OTableWindow*       m_pDragWin;
        OTableWindow*       m_pSizingWin;
        OTableConnection*   m_pSelectedConn;

        BOOL                m_bTrackingInitiallyMoved;

        DECL_LINK(OnDragScrollTimer, void*);

    protected:
        OTableWindow*       m_pLastFocusTabWin;
        OQueryDesignView*   m_pView;

    public:
        TYPEINFO();
        OJoinTableView( Window* pParent, OQueryDesignView* pView );
        virtual ~OJoinTableView();

        // window override
        virtual void StateChanged( StateChangedType nStateChange );

        // own methods
        ScrollBar* GetHScrollBar() { return static_cast<OScrollWindowHelper*>(GetParent())->GetHScrollBar(); }
        ScrollBar* GetVScrollBar() { return static_cast<OScrollWindowHelper*>(GetParent())->GetVScrollBar(); }
        DECL_LINK( ScrollHdl, ScrollBar* );

        void DrawConnections( const Rectangle& rRect );
        void InvalidateConnections();

        void BeginChildMove( OTableWindow* pTabWin, const Point& rMousePos );
        void BeginChildSizing( OTableWindow* pTabWin, const Pointer& rPointer );

        void NotifyTitleClicked( OTableWindow* pTabWin, const Point rMousePos );

        virtual void AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rWinName, BOOL bNewTable = FALSE);
        virtual void RemoveTabWin( OTableWindow* pTabWin );

        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest);
        virtual BOOL RemoveConnection(OTableConnection* pConn);

        BOOL            Scroll( long nDelta, BOOL bHoriz, BOOL bPaintScrollBars );
        ULONG           GetTabWinCount();
        Point           GetScrollOffset() const { return m_aScrollOffset; }

        OQueryDesignView*           getDesignView() { return m_pView; }
        OTableWindow*               GetWindow( const String& rName );

        OTableConnection*           GetSelectedConn() { return m_pSelectedConn; }
        void                        DeselectConn(OTableConnection* pConn);  // NULL ist ausdruecklich zugelassen, dann passiert nichts
        void                        SelectConn(OTableConnection* pConn);

        OTableWindowMap*            GetTabWinMap() { return &m_aTableMap; }
        const OTableWindowMap*      GetTabWinMap() const { return &m_aTableMap; }
        ::std::vector<OTableConnection*>*       GetTabConnList() { return &m_vTableConnection; }
        //  ::std::vector<OTableConnection*>*   GetTabConnList() const { return &m_vTableConnection; }


        BOOL                        ExistsAConn(const OTableWindow* pFromWin) const;
        OTableConnection*           GetTabConn(OTableWindow* pLhs,OTableWindow* pRhs, OTableConnection* _rpFirstAfter = NULL);

        // set the focus to that tab win which most recently had it (or to the first available one)
        void GrabTabWinFocus();

        // ReSync ist dazu gedacht, aus dem Dokument alle WinData und ConnData zu holen und entsprechend Wins und Conns anzulegen
        virtual void ReSync() { }
        // ClearAll implementiert ein hartes Loeschen, es werden alle Conns und alle Wins aus ihren jeweiligen Listen geloescht
        // sowie die entsprechenden Datas aus dem Dokument ausgetragen
        virtual void ClearAll();

        // wird vom AddTabDlg benutzt, um festzustellen, ob noch Tabellen hinzugefuegt werden duerfen
        virtual BOOL IsAddAllowed();
        virtual long PreNotify(NotifyEvent& rNEvt);

    protected:
        virtual void MouseButtonUp( const MouseEvent& rEvt );
        virtual void MouseButtonDown( const MouseEvent& rEvt );
        virtual void Tracking( const TrackingEvent& rTEvt );
        virtual void Paint( const Rectangle& rRect );
        virtual void ConnDoubleClicked( OTableConnection* pConnection );
        virtual void KeyInput( const KeyEvent& rEvt );
        virtual void SetDefaultTabWinPosSize( OTableWindow* pTabWin );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

        virtual void Resize();
        virtual BOOL Drop( const DropEvent& rEvt );
        virtual BOOL QueryDrop( DropEvent& rEvt );

        // wird nach Verschieben/Groessenaenderung der TabWins aufgerufen (die Standardimplementation reicht die neuen Daten einfach
        // an die Daten des Wins weiter)
        virtual void TabWinMoved(OTableWindow* ptWhich, const Point& ptOldPosition);
            // die Position ist "virtuell" : der Container hat sozusagen eine virtuelle Flaeche, von der immer nur ein bestimmter Bereich
            // - der mittels der Scrollbar veraendert werden kann - zu sehen ist. Insbesondere hat ptOldPosition immer positive Koordinaten,
            // auch wenn er einen Punkt oberhalb des aktuell sichtbaren Bereichs bezeichnet, dessen physische Ordinate eigentlich
            // negativ ist.
        virtual void TabWinSized(OTableWindow* ptWhich, const Point& ptOldPosition, const Size& szOldSize);
            // hier ist die Position (die sich waehrend des Sizings aendern kann) physisch, da waehrend des Sizens nicht gescrollt wird
        virtual void Command(const CommandEvent& rEvt);

        virtual void EnsureVisible(const OTableWindow* _pWin);

    private:
        void    InitColors();
        BOOL    ScrollWhileDragging();
    };
}
#endif // DBAUI_JOINTABLEVIEW_HXX
