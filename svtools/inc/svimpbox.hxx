/*************************************************************************
 *
 *  $RCSfile: svimpbox.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:53 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _SVIMPLBOX_HXX
#define _SVIMPLBOX_HXX

#ifndef _SELENG_HXX
#include <vcl/seleng.hxx>
#endif

#ifndef _SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

class SvTreeListBox;
class Point;
class DropEvent;
class SvLBoxTreeList;
class SvImpLBox;
class SvLBoxEntry;
class SvLBoxItem;
class SvLBoxTab;
class TabBar;

class ImpLBSelEng : public FunctionSet
{
    SvImpLBox*          pImp;
    SelectionEngine*    pSelEng;
    SvTreeListBox*      pView;

public:
    ImpLBSelEng( SvImpLBox* pImp, SelectionEngine* pSelEng,
                 SvTreeListBox* pView );
    ~ImpLBSelEng();
    void        BeginDrag();
    void        CreateAnchor();
    void        DestroyAnchor();
    BOOL        SetCursorAtPoint( const Point& rPoint,
                    BOOL bDontSelectAtCursor=FALSE );
    BOOL        IsSelectionAtPoint( const Point& rPoint );
    void        DeselectAtPoint( const Point& rPoint );
    void        DeselectAll();
};

// Flags fuer nFlag
#define F_VER_SBARSIZE_WITH_HBAR        0x0001
#define F_HOR_SBARSIZE_WITH_VBAR        0x0002
#define F_IGNORE_NEXT_MOUSEMOVE         0x0004  // OS/2 only
#define F_IN_SCROLLING                  0x0008
#define F_DESEL_ALL                     0x0010
#define F_START_EDITTIMER               0x0020  // MAC only
#define F_IGNORE_SELECT                 0x0040
#define F_IN_RESIZE                     0x0080
#define F_REMOVED_ENTRY_INVISIBLE       0x0100
#define F_REMOVED_RECALC_MOST_RIGHT     0x0200
#define F_IGNORE_CHANGED_TABS           0x0400
#define F_PAINTED                       0x0800
#define F_IN_PAINT                      0x1000
#define F_ENDSCROLL_SET_VIS_SIZE        0x2000
#define F_FILLING                       0x4000


class SvImpLBox
{
friend class ImpLBSelEng;
friend class SvTreeListBox;
private:
    SvTreeListBox*      pView;
    SvLBoxTreeList*     pTree;
    SvLBoxEntry*        pCursor;
    SvLBoxEntry*        pStartEntry;
    SvLBoxEntry*        pAnchor;
    SvLBoxEntry*        pMostRightEntry;
    SvLBoxButton*       pActiveButton;
    SvLBoxEntry*        pActiveEntry;
    SvLBoxTab*          pActiveTab;
    TabBar*             pTabBar;

    ScrollBar           aVerSBar;
    ScrollBar           aHorSBar;
    ScrollBarBox        aScrBarBox;

    // wg. kompat. hier
    Image               aDontKnowNodeBmp;
    Image               aExpNodeBmp, aCollNodeBmp;
    Size                aOutputSize;
    SelectionEngine     aSelEng;
    ImpLBSelEng         aFctSet;
    Timer               aAsyncBeginDragTimer;
    Point               aAsyncBeginDragPos;

    long                nYoffsNodeBmp;
    long                nNodeBmpTabDistance; // typisch kleiner 0
    long                nNodeBmpWidth;
    long                nNextVerVisSize;
    long                nMostRight;
    ULONG               nVisibleCount;  // Anzahl Zeilen im Control
    ULONG               nCurUserEvent; //-1 == kein Userevent amn Laufen
    short               nHorSBarHeight, nVerSBarWidth;
    USHORT              nFlags;

    WinBits             nWinBits;
    FASTBOOL            bSimpleTravel; // ist TRUE bei SINGLE_SELECTION
    BOOL                bUpdateMode;
    BOOL                bInVScrollHdl;
    BOOL                bAsyncBeginDrag;

//#if defined (MAC) || defined(OV_DEBUG)
    Timer               aEditTimer;
    DECL_LINK( EditTimerCall, Timer * );
//#endif

    DECL_LINK( BeginDragHdl, void* );
    DECL_LINK( MyUserEvent,void*);
    void                StopUserEvent();

    void                InvalidateEntriesFrom( long nY ) const;
    void                InvalidateEntry( long nY ) const;
    void                ShowVerSBar();
    // setzt Thumb auf FirstEntryToDraw
    void                SyncVerThumb();
    BOOL                IsLineVisible( long nY ) const;
    long                GetEntryLine( SvLBoxEntry* pEntry ) const;
    void                FillView();
    void                CursorDown();
    void                CursorUp();
    void                KeyLeftRight( long nDiff );
    void                PageDown( USHORT nDelta );
    void                PageUp( USHORT nDelta );

    void                SetCursor( SvLBoxEntry* pEntry, BOOL bForceNoSelect = FALSE );

    void                DrawNet();

    // ScrollBar-Handler
    DECL_LINK( ScrollUpDownHdl, ScrollBar * );
    DECL_LINK( ScrollLeftRightHdl, ScrollBar * );
    DECL_LINK( EndScrollHdl, ScrollBar * );

    void                SetNodeBmpYOffset( const Image& );
    void                SetNodeBmpTabDistance();

    // Selection-Engine
    SvLBoxEntry*        MakePointVisible( const Point& rPoint,
                            BOOL bNotifyScroll=TRUE );

    void                SetAnchorSelection( SvLBoxEntry* pOld,
                            SvLBoxEntry* pNewCursor );
    void                BeginDrag();
    BOOL                ButtonDownCheckCtrl( const MouseEvent& rMEvt,
                            SvLBoxEntry* pEntry, long nY    );
    BOOL                MouseMoveCheckCtrl( const MouseEvent& rMEvt,
                            SvLBoxEntry* pEntry );
    BOOL                ButtonUpCheckCtrl( const MouseEvent& rMEvt );
    BOOL                ButtonDownCheckExpand( const MouseEvent&,
                            SvLBoxEntry*,long nY );

    void                PositionScrollBars( Size& rOSize, USHORT nMask );
    USHORT              AdjustScrollBars( Size& rSize );

    void                BeginScroll();
    void                EndScroll();
    BOOL                InScroll() const { return (BOOL)(nFlags & F_IN_SCROLLING)!=0;}
    Rectangle           GetVisibleArea() const;
    BOOL                EntryReallyHit(SvLBoxEntry* pEntry,const Point& rPos,long nLine);
    void                InitScrollBarBox();
    SvLBoxTab*          NextTab( SvLBoxTab* );

    BOOL                SetMostRight( SvLBoxEntry* pEntry );
    void                FindMostRight( SvLBoxEntry* EntryToIgnore );
    void                FindMostRight( SvLBoxEntry* pParent, SvLBoxEntry* EntryToIgnore );
    void                FindMostRight_Impl( SvLBoxEntry* pParent,SvLBoxEntry* EntryToIgnore  );
    void                NotifyTabsChanged();

public:
    SvImpLBox( SvTreeListBox* pView, SvLBoxTreeList*, WinBits nWinStyle );
    ~SvImpLBox();

    void                Clear();
    void                SetWindowBits( WinBits nWinStyle );
    void                SetModel( SvLBoxTreeList* pModel ) { pTree = pModel;}

    void                EntryInserted( SvLBoxEntry*);
    void                RemovingEntry( SvLBoxEntry* pEntry );
    void                EntryRemoved();
    void                MovingEntry( SvLBoxEntry* pEntry );
    void                EntryMoved( SvLBoxEntry* pEntry );
    void                TreeInserted( SvLBoxEntry* pEntry );

    void                IndentChanged( short nIndentPixel );
    void                EntryExpanded( SvLBoxEntry* pEntry );
    void                EntryCollapsed( SvLBoxEntry* pEntry );
    void                CollapsingEntry( SvLBoxEntry* pEntry );
    void                EntrySelected( SvLBoxEntry*, BOOL bSelect );

    void                Paint( const Rectangle& rRect );
    void                RepaintSelectionItems();
    void                MouseButtonDown( const MouseEvent& );
    void                MouseButtonUp( const MouseEvent& );
    void                MouseMove( const MouseEvent&);
    BOOL                KeyInput( const KeyEvent& );
    void                Resize();
    void                GetFocus();
    void                LoseFocus();
    void                UpdateAll(
                            BOOL bInvalidateCompleteView= TRUE,
                            BOOL bUpdateVerSBar = TRUE );
    void                SetEntryHeight( short nHeight );
    void                PaintEntry( SvLBoxEntry* pEntry );
    void                InvalidateEntry( SvLBoxEntry* );
    void                RecalcFocusRect();

    inline void         SelectEntry( SvLBoxEntry* pEntry, BOOL bSelect );
    void                SetDragDropMode( DragDropMode eDDMode );
    void                SetSelectionMode( SelectionMode eSelMode  );
    void                SetAddMode( BOOL bAdd ) { aSelEng.AddAlways(FALSE); }
    BOOL                IsAddMode() const { return aSelEng.IsAlwaysAdding(); }

    SvLBoxEntry*        GetCurrentEntry() const { return pCursor; }
    BOOL                IsEntryInView( SvLBoxEntry* ) const;
    SvLBoxEntry*        GetEntry( const Point& rPos ) const;
    // gibt letzten Eintrag zurueck, falls Pos unter letztem Eintrag
    SvLBoxEntry*        GetClickedEntry( const Point& ) const;
    SvLBoxEntry*        GetCurEntry() const { return pCursor; }
    void                SetCurEntry( SvLBoxEntry* );
    Point               GetEntryPos( SvLBoxEntry* ) const;
    void                MakeVisible( SvLBoxEntry* pEntry, BOOL bMoveToTop=FALSE );

    void                PaintDDCursor( SvLBoxEntry* );

    void                SetExpandedNodeBmp( const Image& );
    void                SetCollapsedNodeBmp( const Image& );
    void                SetDontKnowNodeBmp( const Image& rImg ) { aDontKnowNodeBmp = rImg; }
    const Image&        GetExpandedNodeBmp() const { return aExpNodeBmp; }
    const Image&        GetCollapsedNodeBmp() const { return aCollNodeBmp; }
    const Image&        GetDontKnowNodeBmp() const { return aDontKnowNodeBmp; }

    const Size&         GetOutputSize() const { return aOutputSize;}
    void                KeyUp( BOOL bPageUp, BOOL bNotifyScroll = TRUE );
    void                KeyDown( BOOL bPageDown, BOOL bNotifyScroll = TRUE );
    void                Command( const CommandEvent& rCEvt );

    void                Invalidate();
    void                DestroyAnchor() { pAnchor=0; aSelEng.Reset(); }
    void                SelAllDestrAnch( BOOL bSelect,
                            BOOL bDestroyAnchor = TRUE,
                            BOOL bSingleSelToo = FALSE );
    void                ShowCursor( BOOL bShow );

    BOOL                RequestHelp( const HelpEvent& rHEvt );
    void                EndSelection();
    BOOL                IsNodeButton( const Point& rPosPixel, SvLBoxEntry* pEntry ) const;
    void                RepaintScrollBars();
    void                EnableAsyncDrag( BOOL b) { bAsyncBeginDrag = b; }
    void                SetUpdateMode( BOOL );
    void                SetUpdateModeFast( BOOL );
    BOOL                GetUpdateMode() const { return bUpdateMode; }
    Rectangle           GetClipRegionRect() const;
    BOOL                HasHorScrollBar() const { return aHorSBar.IsVisible(); }
    void                ShowFocusRect( const SvLBoxEntry* pEntry );
    void                SetTabBar( TabBar* pTabBar );
};

inline void SvImpLBox::SetExpandedNodeBmp( const Image& rBmp )
{
    aExpNodeBmp = rBmp;
    SetNodeBmpYOffset( rBmp );
}

inline void SvImpLBox::SetCollapsedNodeBmp( const Image& rBmp )
{
    aCollNodeBmp = rBmp;
    SetNodeBmpYOffset( rBmp );
}

inline Point SvImpLBox::GetEntryPos( SvLBoxEntry* pEntry ) const
{
    return Point( 0, GetEntryLine( pEntry ) );
}

inline void SvImpLBox::PaintEntry( SvLBoxEntry* pEntry )
{
    long nY = GetEntryLine( pEntry );
    pView->PaintEntry( pEntry, nY );
}


inline BOOL SvImpLBox::IsLineVisible( long nY ) const
{
    BOOL bRet = TRUE;
    if ( nY < 0 || nY >= aOutputSize.Height() )
        bRet = FALSE;
    return bRet;
}

inline void SvImpLBox::TreeInserted( SvLBoxEntry* pTree )
{
    EntryInserted( pTree );
}


#endif


