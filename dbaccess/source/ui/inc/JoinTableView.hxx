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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_JOINTABLEVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_JOINTABLEVIEW_HXX

#include <vcl/window.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/transfer.hxx>

#include "callbacks.hxx"
#include "TableConnectionData.hxx"
#include "TableWindowData.hxx"

#include <map>
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

    // this class contains only the scrollbars to avoid that
    // the tablewindows clip the scrollbars
    class OJoinTableView;
    class OScrollWindowHelper : public vcl::Window
    {
        VclPtr<ScrollBar>          m_aHScrollBar;
        VclPtr<ScrollBar>          m_aVScrollBar;
        VclPtr<vcl::Window>        m_pCornerWindow;
        VclPtr<OJoinTableView>     m_pTableView;

    protected:
        virtual void Resize() override;

    public:
        OScrollWindowHelper( vcl::Window* pParent);
        virtual ~OScrollWindowHelper() override;
        virtual void dispose() override;

        void setTableView(OJoinTableView* _pTableView);

        void resetRange(const Point& _aSize);

        // own methods
        ScrollBar& GetHScrollBar() { return *m_aHScrollBar.get(); }
        ScrollBar& GetVScrollBar() { return *m_aVScrollBar.get(); }
    };


    class OJoinTableView : public vcl::Window,
                           public IDragTransferableListener,
                           public DropTargetHelper
    {
        friend class OJoinMoveTabWinUndoAct;

    public:
        typedef std::map<OUString, VclPtr<OTableWindow> > OTableWindowMap;

    private:
        OTableWindowMap     m_aTableMap;
        std::vector<VclPtr<OTableConnection> >    m_vTableConnection;

        Idle                m_aDragScrollIdle;
        tools::Rectangle           m_aDragRect;
        tools::Rectangle           m_aSizingRect;
        Point               m_aDragOffset;
        Point               m_aScrollOffset;
        Point               m_ptPrevDraggingPos;
        Size                m_aOutputSize;


        VclPtr<OTableWindow>           m_pDragWin;
        VclPtr<OTableWindow>           m_pSizingWin;
        VclPtr<OTableConnection>       m_pSelectedConn;


        DECL_LINK(OnDragScrollTimer, Timer*, void);

    protected:
        VclPtr<OTableWindow>               m_pLastFocusTabWin;
        VclPtr<OJoinDesignView>            m_pView;
        OJoinDesignViewAccess*      m_pAccessible;

    public:
        OJoinTableView( vcl::Window* pParent, OJoinDesignView* pView );
        virtual ~OJoinTableView() override;
        virtual void dispose() override;

        // window override
        virtual void StateChanged( StateChangedType nStateChange ) override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual void KeyInput( const KeyEvent& rEvt ) override;
        // Accessibility
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        // own methods
        ScrollBar& GetHScrollBar() { return static_cast<OScrollWindowHelper*>(GetParent())->GetHScrollBar(); }
        ScrollBar& GetVScrollBar() { return static_cast<OScrollWindowHelper*>(GetParent())->GetVScrollBar(); }
        DECL_LINK( ScrollHdl, ScrollBar*, void );

        void DrawConnections(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
        void InvalidateConnections();

        void BeginChildMove( OTableWindow* pTabWin, const Point& rMousePos );
        void BeginChildSizing( OTableWindow* pTabWin, PointerStyle nPointer );

        void NotifyTitleClicked( OTableWindow* pTabWin, const Point& rMousePos );

        virtual void AddTabWin(const OUString& _rComposedName, const OUString& rWinName, bool bNewTable = false);
        virtual void RemoveTabWin( OTableWindow* pTabWin );

        // hide all TabWins (does NOT delete them; they are put in an UNDO action)
        void HideTabWins();

        virtual void AddConnection(const OJoinExchangeData& jxdSource, const OJoinExchangeData& jxdDest) = 0;

        /** RemoveConnection allows to remove connections from join table view

            it implies that the same as addConnection

            @param  rConnection the connection which should be removed
            @param  bDelete     when true then the connection will be deleted

            @return an iterator to next valid connection, so it can be used in any loop
        */
        virtual bool RemoveConnection(VclPtr<OTableConnection>& rConnection, bool bDelete);

        /** allows to add new connections to join table view

            it implies an invalidation of the features ID_BROWSER_ADDTABLE and
            SID_RELATION_ADD_RELATION also the modified flag will be set to true

            @param  _pConnection the connection which should be added
            @param  _bAddData    when true then the data should also be appended
        */
        void addConnection(OTableConnection* _pConnection,bool _bAddData = true);

        bool ScrollPane( long nDelta, bool bHoriz, bool bPaintScrollBars );
        sal_uLong GetTabWinCount();
        const Point& GetScrollOffset() const { return m_aScrollOffset; }

        OJoinDesignView* getDesignView() const { return m_pView; }
        OTableWindow* GetTabWindow( const OUString& rName );

        VclPtr<OTableConnection>& GetSelectedConn() { return m_pSelectedConn; }
        /** @note NULL is explicitly allowed (then no-op) */
        void DeselectConn(OTableConnection* pConn);
        void SelectConn(OTableConnection* pConn);

        OTableWindowMap& GetTabWinMap() { return m_aTableMap; }

        /** gives a read only access to the connection vector
        */
        const std::vector<VclPtr<OTableConnection> >& getTableConnections() const { return m_vTableConnection; }

        bool ExistsAConn(const OTableWindow* pFromWin) const;

        /** search for all connections of a table

            @param  _pFromWin   the table for which connections should be found
            @return an iterator which can be used to travel all connections of the table
        */
        std::vector<VclPtr<OTableConnection> >::const_iterator getTableConnections(const OTableWindow* _pFromWin) const;

        /** how many connection belongs to single table

            @param  _pFromWin the table for which connections should be found
            @return the count of connections which belongs to this table
        */
        sal_Int32 getConnectionCount(const OTableWindow* _pFromWin) const;

        OTableConnection* GetTabConn(const OTableWindow* pLhs,const OTableWindow* pRhs,bool _bSupressCrossOrNaturalJoin = false) const;

        /** clear the window map and connection vector without destroying it

            that means that the data of the windows and connection will be
            untouched
         */
        void clearLayoutInformation();

        /** set the focus to that tab win which most recently had it
            (or to the first available one) **/
        void GrabTabWinFocus();

        /** take all WinData and ConnData from the document and create the
            corresponding Wins and Conns */
        virtual void ReSync() { }

        /** Hard deletion

            That means that all Conns and Wins are deleted from their respective
            lists and the corresponding Datas removed from the document */
        virtual void ClearAll();

        /** @note used by AddTabDlg to see if more tables can be added */
        virtual bool IsAddAllowed();
        virtual bool PreNotify(NotifyEvent& rNEvt) override;

        // DnD stuff
        virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
        virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

        /** @note can be used for special ui handling after d&d */
        virtual void lookForUiActivities();

        /** Hook that is called after moving/resizing TabWins

            The position is 'virtual': the container has a virtual area of
            which only a part - changeable by scroll bar - is visible.
            Therefore: ptOldPosition is always positive, even if it represents
            a point with a negative physical ordinate above the visible area

            @note The standard implementation just passes the new data to the
                  Wins
         */
        void TabWinMoved(OTableWindow* ptWhich, const Point& ptOldPosition);

        void TabWinSized(OTableWindow* ptWhich, const Point& ptOldPosition, const Size& szOldSize);

        void modified();

        /** check if the given window is visible.

            @param _rPoint  The Point to check
            @param _rSize   The Size to be check as well
            @return true if the area is visible, false otherwise
        */
        bool isMovementAllowed(const Point& _rPoint,const Size& _rSize);

        const Size& getRealOutputSize() const { return m_aOutputSize; }

        virtual void EnsureVisible(const OTableWindow* _pWin);
        void EnsureVisible(const Point& _rPoint,const Size& _rSize);

        TTableWindowData::value_type createTableWindowData(const OUString& _rComposedName
                                            ,const OUString& _sTableName
                                            ,const OUString& _rWinName);

    protected:
        virtual void MouseButtonUp( const MouseEvent& rEvt ) override;
        virtual void MouseButtonDown( const MouseEvent& rEvt ) override;
        virtual void Tracking( const TrackingEvent& rTEvt ) override;
        virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
        virtual void ConnDoubleClicked(VclPtr<OTableConnection>& rConnection);
        void SetDefaultTabWinPosSize( OTableWindow* pTabWin );
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

        virtual void Resize() override;

        virtual void dragFinished( ) override;
        /// @note here the physical position (that can be changed while
        ///     resizing) is used, as no scrolling can take place while resizing
        virtual void Command(const CommandEvent& rEvt) override;

        virtual OTableWindowData* CreateImpl(const OUString& _rComposedName
                                            ,const OUString& _sTableName
                                            ,const OUString& _rWinName);

        /** factory method to create table windows

            @param _pData The data corresponding to the window.
            @return The new TableWindow
        */
        virtual VclPtr<OTableWindow> createWindow(const TTableWindowData::value_type& _pData) = 0;

        /** determines whether the classes Init method should accept a query
            name, or only table names */
        virtual bool allowQueries() const;

        /** called when init fails at the tablewindowdata because the m_xTable
            object could not provide columns, but no exception was thrown.
            Expected to throw. */
        virtual void onNoColumns_throw();

        virtual bool supressCrossNaturalJoin(const TTableConnectionData::value_type& _pData) const;

    private:
        void InitColors();
        void ScrollWhileDragging();

        /** opens the context menu to delete a connection
            @param _aPos the position where the popup menu should appear
            @param _pSelConnection the connection which should be deleted
        */
        void executePopup(const Point& _aPos, VclPtr<OTableConnection>& rSelConnection);

        /** invalidates this window without children and set the controller
            modified
            @param _pAction a possible undo action to add at the controller
        */
        void invalidateAndModify(std::unique_ptr<SfxUndoAction> _pAction);

    private:
        using Window::Scroll;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
