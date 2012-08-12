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
#ifndef DBAUI_JOINTABLEVIEW_HXX
#define DBAUI_JOINTABLEVIEW_HXX

#include <vcl/window.hxx>
#include <vcl/timer.hxx>
#include <vcl/scrbar.hxx>
#include <tools/rtti.hxx>
#include <svtools/transfer.hxx>

#include <comphelper/stl_types.hxx>
#include "callbacks.hxx"
#include "TableConnectionData.hxx"
#include "TableWindowData.hxx"
#include <memory>
#include <vector>

struct AcceptDropEvent;
struct ExecuteDropEvent;
class SfxUndoAction;
namespace dbaui
{
    class OTableConnection;
    class OTableWindow;
    struct OJoinExchangeData;
    class OJoinDesignView;
    class OTableWindowData;
    class OJoinDesignViewAccess;

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

        void resetRange(const Point& _aSize);

        // own methods
        ScrollBar* GetHScrollBar() { return &m_aHScrollBar; }
        ScrollBar* GetVScrollBar() { return &m_aVScrollBar; }
    };


    class OJoinTableView :  public Window
                            ,public IDragTransferableListener
                            ,public DropTargetHelper
    {
        friend class OJoinMoveTabWinUndoAct;
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


        OTableWindow*           m_pDragWin;
        OTableWindow*           m_pSizingWin;
        OTableConnection*       m_pSelectedConn;


        sal_Bool                    m_bTrackingInitiallyMoved;

        DECL_LINK(OnDragScrollTimer, void*);

    protected:
        OTableWindow*               m_pLastFocusTabWin;
        OJoinDesignView*            m_pView;
        OJoinDesignViewAccess*      m_pAccessible;

    public:
        OJoinTableView( Window* pParent, OJoinDesignView* pView );
        virtual ~OJoinTableView();

        // window override
        virtual void StateChanged( StateChangedType nStateChange );
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void KeyInput( const KeyEvent& rEvt );
        // Accessibility
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

        // own methods
        ScrollBar* GetHScrollBar() { return static_cast<OScrollWindowHelper*>(GetParent())->GetHScrollBar(); }
        ScrollBar* GetVScrollBar() { return static_cast<OScrollWindowHelper*>(GetParent())->GetVScrollBar(); }
        DECL_LINK( ScrollHdl, ScrollBar* );

        void DrawConnections( const Rectangle& rRect );
        void InvalidateConnections();

        void BeginChildMove( OTableWindow* pTabWin, const Point& rMousePos );
        void BeginChildSizing( OTableWindow* pTabWin, const Pointer& rPointer );

        void NotifyTitleClicked( OTableWindow* pTabWin, const Point rMousePos );

        virtual void AddTabWin(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rWinName, sal_Bool bNewTable = sal_False);
        virtual void RemoveTabWin( OTableWindow* pTabWin );

        // hide all TabWins (does NOT delete them; they are put in an UNDO action)
        virtual void    HideTabWins();

        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest) = 0;

        /** RemoveConnection allows to remove connections from join table view, it implies that the same as addConnection

            @param  _pConnection
                    the connection which should be removed
            @param  _bDelete
                    when truie then the connection will be deleted

            @return an iterator to next valid connection, so it can be used in any loop
        */
        virtual bool RemoveConnection(OTableConnection* _pConnection,sal_Bool _bDelete);

        /** allows to add new connections to join table view, it implies an invalidation of the features
            ID_BROWSER_ADDTABLE and SID_RELATION_ADD_RELATION also the modified flag will be set to true
            @param  _pConnection
                    the connection which should be added
            @param  _bAddData
                    <TRUE/> when the data should also be appended
        */
        void addConnection(OTableConnection* _pConnection,sal_Bool _bAddData = sal_True);

        sal_Bool            ScrollPane( long nDelta, sal_Bool bHoriz, sal_Bool bPaintScrollBars );
        sal_uLong           GetTabWinCount();
        Point           GetScrollOffset() const { return m_aScrollOffset; }

        OJoinDesignView*            getDesignView() const { return m_pView; }
        OTableWindow*               GetTabWindow( const String& rName );

        OTableConnection*           GetSelectedConn() { return m_pSelectedConn; }
        /** @note NULL is explicitly allowed (then no-op) */
        void                        DeselectConn(OTableConnection* pConn);
        void                        SelectConn(OTableConnection* pConn);

        OTableWindowMap*            GetTabWinMap() { return &m_aTableMap; }
        const OTableWindowMap*      GetTabWinMap() const { return &m_aTableMap; }

        /** gives a read only access to the connection vector
        */
        const ::std::vector<OTableConnection*>* getTableConnections() const { return &m_vTableConnection; }


        sal_Bool                        ExistsAConn(const OTableWindow* pFromWin) const;

        /** getTableConnections searchs for all connections of a table
            @param  _pFromWin   the table for which connections should be found

            @return an iterator which can be used to travel all connections of the table
        */
        ::std::vector<OTableConnection*>::const_iterator getTableConnections(const OTableWindow* _pFromWin) const;

        /** getConnectionCount returns how many connection belongs to single table
            @param  _pFromWin   the table for which connections should be found

            @return the count of connections wich belongs to this table
        */
        sal_Int32 getConnectionCount(const OTableWindow* _pFromWin) const;

        OTableConnection* GetTabConn(const OTableWindow* pLhs,const OTableWindow* pRhs,bool _bSupressCrossOrNaturalJoin = false,const OTableConnection* _rpFirstAfter = NULL) const;

        // clears the window map and connection vector without destroying it
        // that means teh data of the windows and connection will be untouched
        void clearLayoutInformation();

        // set the focus to that tab win which most recently had it (or to the first available one)
        void GrabTabWinFocus();

        // ReSync is meant to get all WinData and ConnData from the document
        // and create the corresponding Wins and Conns
        virtual void ReSync() { }

        // ClearAll implements hart deletion, all Conns and Wins are deleted
        // from their respective lists and the corresponding Datas removed
        // from the document
        virtual void ClearAll();

        // used by AddTabDlg to see if more tables can be added
        virtual sal_Bool IsAddAllowed();
        virtual long PreNotify(NotifyEvent& rNEvt);

        // DnD stuff
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

        /**
            can be used in derevied classes to make some special ui handling
            after d&d
        */
        virtual void lookForUiActivities();

        // called after moving/resizing TabWins
        // (the standard implementation just passes the new data to the Wins)
        virtual void TabWinMoved(OTableWindow* ptWhich, const Point& ptOldPosition);
            // the position is 'virtual': the container has a virtual area
            // of which only a part - changeable by scroll bar - is visible
            // therefore: ptOldPosition is always positive, even if it represents
            // a point with a negative physical ordinate above the visible area
        virtual void TabWinSized(OTableWindow* ptWhich, const Point& ptOldPosition, const Size& szOldSize);

        void modified();

        /** check if the given window is visible.
            @param  _rPoint
                The Point to check
            @param  _rSize
                The Size to be check as well
            @return
                <TRUE/> if the area is visible otherwise <FALSE/>

        */
        sal_Bool isMovementAllowed(const Point& _rPoint,const Size& _rSize);

        Size getRealOutputSize() const { return m_aOutputSize; }



        virtual void EnsureVisible(const OTableWindow* _pWin);
        virtual void EnsureVisible(const Point& _rPoint,const Size& _rSize);

        TTableWindowData::value_type createTableWindowData(const ::rtl::OUString& _rComposedName
                                            ,const ::rtl::OUString& _sTableName
                                            ,const ::rtl::OUString& _rWinName);

    protected:
        virtual void MouseButtonUp( const MouseEvent& rEvt );
        virtual void MouseButtonDown( const MouseEvent& rEvt );
        virtual void Tracking( const TrackingEvent& rTEvt );
        virtual void Paint( const Rectangle& rRect );
        virtual void ConnDoubleClicked( OTableConnection* pConnection );
        virtual void SetDefaultTabWinPosSize( OTableWindow* pTabWin );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

        virtual void Resize();

        virtual void dragFinished( );
        // here the physical position (that can be changed while resizing)
        // is used, as no scrolling can take place while resizing
        virtual void Command(const CommandEvent& rEvt);

        virtual OTableWindowData* CreateImpl(const ::rtl::OUString& _rComposedName
                                            ,const ::rtl::OUString& _sTableName
                                            ,const ::rtl::OUString& _rWinName);

        /** factory method to create table windows
            @param  _pData
                The data corresponding to the window.
            @return
                The new TableWindow
        */
        virtual OTableWindow* createWindow(const TTableWindowData::value_type& _pData) = 0;

        /** determines whether the classes Init method should accept a query name, or only table names
        */
        virtual bool    allowQueries() const;

        /** called when init fails at the tablewindowdata because the m_xTable object could not provide columns, but no
            exception was thrown. Expected to throw.
        */
        virtual void    onNoColumns_throw();

        virtual bool supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const;

    private:
        void    InitColors();
        sal_Bool    ScrollWhileDragging();

        /** executePopup opens the context menu to delate a connection
            @param  _aPos               the position where the popup menu should appear
            @param  _pSelConnection     the connection which should be deleted
        */
        void executePopup(const Point& _aPos,OTableConnection* _pSelConnection);

        /** invalidateAndModify invalidates this window without children and
            set the controller modified
            @param  _pAction a possible undo action to add at the controller
        */
        void invalidateAndModify(SfxUndoAction *_pAction=NULL);

    private:
        using Window::Scroll;
    };
}
#endif // DBAUI_JOINTABLEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
