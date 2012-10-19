/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVIMPLBOX_HXX
#define _SVIMPLBOX_HXX

#include <vcl/seleng.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclevent.hxx>
// #102891# ----------------
#include <unotools/intlwrapper.hxx>
// #97680# -----------------
#include <vector>
#include "svtaccessiblefactory.hxx"

class SvTreeListBox;
class Point;
class SvTreeList;
class SvImpLBox;
class SvTreeListEntry;
class SvLBoxTab;
namespace comphelper
{
    namespace string
    {
        class NaturalStringSorter;
    }
}

class ImpLBSelEng : public FunctionSet
{
    SvImpLBox*          pImp;
    SelectionEngine*    pSelEng;
    SvTreeListBox*      pView;

public:
    ImpLBSelEng( SvImpLBox* pImp, SelectionEngine* pSelEng,
                 SvTreeListBox* pView );
    virtual ~ImpLBSelEng();
    void        BeginDrag();
    void        CreateAnchor();
    void        DestroyAnchor();
    sal_Bool        SetCursorAtPoint( const Point& rPoint,
                    sal_Bool bDontSelectAtCursor=sal_False );
    sal_Bool        IsSelectionAtPoint( const Point& rPoint );
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
    SvTreeList*     pTree;
    SvTreeListEntry*        pCursor;
    SvTreeListEntry*        pStartEntry;
    SvTreeListEntry*        pAnchor;
    SvTreeListEntry*        pMostRightEntry;
    SvLBoxButton*       pActiveButton;
    SvTreeListEntry*        pActiveEntry;
    SvLBoxTab*          pActiveTab;

    ScrollBar           aVerSBar;
    ScrollBar           aHorSBar;
    ScrollBarBox        aScrBarBox;

    ::svt::AccessibleFactoryAccess
                        m_aFactoryAccess;

    static Image*       s_pDefCollapsed;
    static Image*       s_pDefExpanded;
    static oslInterlockedCount  s_nImageRefCount; /// When 0 all static images will be destroyed

    // Node Bitmaps
    enum ImageType
    {
        itNodeExpanded = 0,     // node is expanded ( usually a bitmap showing a minus )
        itNodeCollapsed,        // node is collapsed ( usually a bitmap showing a plus )
        itNodeDontKnow,         // don't know the node state
        itEntryDefExpanded,     // default for expanded entries
        itEntryDefCollapsed,    // default for collapsed entries

        IT_IMAGE_COUNT
    };

    // all our images
    Image               m_aNodeAndEntryImages[ IT_IMAGE_COUNT ];

    // wg. kompat. hier
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
    sal_uLong               nVisibleCount;  // Anzahl Zeilen im Control
    sal_uLong               nCurUserEvent; //-1 == kein Userevent amn Laufen
    short               nHorSBarHeight, nVerSBarWidth;
    sal_uInt16              nFlags;
    sal_uInt16              nCurTabPos;

    WinBits             m_nStyle;
    ExtendedWinBits     nExtendedWinBits;
    sal_Bool                bSimpleTravel : 1; // ist sal_True bei SINGLE_SELECTION
    sal_Bool                bUpdateMode : 1;
    sal_Bool                bInVScrollHdl : 1;
    sal_Bool                bAsyncBeginDrag : 1;
    sal_Bool                bSubLstOpRet : 1;   // open/close sublist with return/enter, defaulted with sal_False
    sal_Bool                bSubLstOpLR : 1;    // open/close sublist with cursor left/right, defaulted with sal_False
    sal_Bool                bContextMenuHandling : 1;
    sal_Bool                bIsCellFocusEnabled : 1;

    sal_Bool            bAreChildrenTransient;

    Point               aEditClickPos;
    Timer               aEditTimer;

    // #102891# -------------------
    comphelper::string::NaturalStringSorter *m_pStringSorter;

    // #97680# --------------------
    std::vector< short > aContextBmpWidthVector;

    DECL_LINK(EditTimerCall, void *);

    DECL_LINK( BeginDragHdl, void* );
    DECL_LINK( MyUserEvent,void*);
    void                StopUserEvent();

    void                InvalidateEntriesFrom( long nY ) const;
    void                InvalidateEntry( long nY ) const;
    void                ShowVerSBar();
    // setzt Thumb auf FirstEntryToDraw
    void                SyncVerThumb();
    sal_Bool                IsLineVisible( long nY ) const;
    long                GetEntryLine( SvTreeListEntry* pEntry ) const;
    void                FillView();
    void                CursorDown();
    void                CursorUp();
    void                KeyLeftRight( long nDiff );
    void                PageDown( sal_uInt16 nDelta );
    void                PageUp( sal_uInt16 nDelta );

    void                SetCursor( SvTreeListEntry* pEntry, sal_Bool bForceNoSelect = sal_False );

    void                DrawNet();

    // ScrollBar-Handler
    DECL_LINK( ScrollUpDownHdl, ScrollBar * );
    DECL_LINK( ScrollLeftRightHdl, ScrollBar * );
    DECL_LINK( EndScrollHdl, void * );

    void                SetNodeBmpYOffset( const Image& );
    void                SetNodeBmpTabDistance();

    // Selection-Engine
    SvTreeListEntry*        MakePointVisible( const Point& rPoint,
                            sal_Bool bNotifyScroll=sal_True );

    void                SetAnchorSelection( SvTreeListEntry* pOld,
                            SvTreeListEntry* pNewCursor );
    void                BeginDrag();
    sal_Bool                ButtonDownCheckCtrl( const MouseEvent& rMEvt,
                            SvTreeListEntry* pEntry, long nY    );
    sal_Bool                MouseMoveCheckCtrl( const MouseEvent& rMEvt,
                            SvTreeListEntry* pEntry );
    sal_Bool                ButtonUpCheckCtrl( const MouseEvent& rMEvt );
    sal_Bool                ButtonDownCheckExpand( const MouseEvent&,
                            SvTreeListEntry*,long nY );

    void                PositionScrollBars( Size& rOSize, sal_uInt16 nMask );
    sal_uInt16              AdjustScrollBars( Size& rSize );

    void                BeginScroll();
    void                EndScroll();
    sal_Bool                InScroll() const { return (sal_Bool)(nFlags & F_IN_SCROLLING)!=0;}
    Rectangle           GetVisibleArea() const;
    sal_Bool                EntryReallyHit(SvTreeListEntry* pEntry,const Point& rPos,long nLine);
    void                InitScrollBarBox();
    SvLBoxTab*          NextTab( SvLBoxTab* );

    sal_Bool                SetMostRight( SvTreeListEntry* pEntry );
    void                FindMostRight( SvTreeListEntry* EntryToIgnore );
    void                FindMostRight( SvTreeListEntry* pParent, SvTreeListEntry* EntryToIgnore );
    void                FindMostRight_Impl( SvTreeListEntry* pParent,SvTreeListEntry* EntryToIgnore  );
    void                NotifyTabsChanged();

    inline sal_Bool         IsExpandable() const        // if element at cursor can be expanded in general
                            { return pCursor->HasChildren() || pCursor->HasChildrenOnDemand(); }
    inline sal_Bool         IsNowExpandable() const     // if element at cursor can be expanded at this moment
                            { return IsExpandable() && !pView->IsExpanded( pCursor ); }

    static  void        implInitDefaultNodeImages();

    void UpdateStringSorter();

    // #97680# --------------------
    short               UpdateContextBmpWidthVector( SvTreeListEntry* pEntry, short nWidth );
    void                UpdateContextBmpWidthMax( SvTreeListEntry* pEntry );
    void                UpdateContextBmpWidthVectorFromMovedEntry( SvTreeListEntry* pEntry );

    void                CalcCellFocusRect( SvTreeListEntry* pEntry, Rectangle& rRect );

    inline sal_Bool     AreChildrenTransient() const { return bAreChildrenTransient; }
    inline void         SetChildrenNotTransient() { bAreChildrenTransient = sal_False; }

public:
    SvImpLBox( SvTreeListBox* pView, SvTreeList*, WinBits nWinStyle );
    ~SvImpLBox();

    void                Clear();
    void                SetStyle( WinBits i_nWinStyle );
    void                SetExtendedWindowBits( ExtendedWinBits _nBits );
    ExtendedWinBits     GetExtendedWindowBits() const { return nExtendedWinBits; }
    void                SetModel( SvTreeList* pModel ) { pTree = pModel;}

    void                EntryInserted( SvTreeListEntry*);
    void                RemovingEntry( SvTreeListEntry* pEntry );
    void                EntryRemoved();
    void                MovingEntry( SvTreeListEntry* pEntry );
    void                EntryMoved( SvTreeListEntry* pEntry );
    void                TreeInserted( SvTreeListEntry* pEntry );

    void                EntryExpanded( SvTreeListEntry* pEntry );
    void                EntryCollapsed( SvTreeListEntry* pEntry );
    void                CollapsingEntry( SvTreeListEntry* pEntry );
    void                EntrySelected( SvTreeListEntry*, sal_Bool bSelect );

    void                Paint( const Rectangle& rRect );
    void                MouseButtonDown( const MouseEvent& );
    void                MouseButtonUp( const MouseEvent& );
    void                MouseMove( const MouseEvent&);
    sal_Bool                KeyInput( const KeyEvent& );
    void                Resize();
    void                GetFocus();
    void                LoseFocus();
    void                UpdateAll(
                            sal_Bool bInvalidateCompleteView= sal_True,
                            sal_Bool bUpdateVerSBar = sal_True );
    void                SetEntryHeight( short nHeight );
    void                PaintEntry( SvTreeListEntry* pEntry );
    void                InvalidateEntry( SvTreeListEntry* );
    void                RecalcFocusRect();

    inline void         SelectEntry( SvTreeListEntry* pEntry, sal_Bool bSelect );
    void                SetDragDropMode( DragDropMode eDDMode );
    void                SetSelectionMode( SelectionMode eSelMode  );
    void                SetAddMode( sal_Bool ) { aSelEng.AddAlways(sal_False); }
    sal_Bool                IsAddMode() const { return aSelEng.IsAlwaysAdding(); }

    SvTreeListEntry*        GetCurrentEntry() const { return pCursor; }
    sal_Bool                IsEntryInView( SvTreeListEntry* ) const;
    SvTreeListEntry*        GetEntry( const Point& rPos ) const;
    // gibt letzten Eintrag zurueck, falls Pos unter letztem Eintrag
    SvTreeListEntry*        GetClickedEntry( const Point& ) const;
    SvTreeListEntry*        GetCurEntry() const { return pCursor; }
    void                SetCurEntry( SvTreeListEntry* );
    Point               GetEntryPosition( SvTreeListEntry* ) const;
    void                MakeVisible( SvTreeListEntry* pEntry, sal_Bool bMoveToTop=sal_False );
    void                ScrollToAbsPos( long nPos );

    void                PaintDDCursor( SvTreeListEntry* );

    // Images
    inline Image&       implGetImageLocation( const ImageType _eType );

    inline void         SetExpandedNodeBmp(  const Image& _rImg );
    inline void         SetCollapsedNodeBmp( const Image& _rImg );

    inline const Image& GetExpandedNodeBmp( );
    inline const Image& GetCollapsedNodeBmp( );
    inline const Image& GetDontKnowNodeBmp( );

    inline void         SetDefaultEntryExpBmp( const Image& _rImg );
    inline void         SetDefaultEntryColBmp( const Image& _rImg );
    inline const Image& GetDefaultEntryExpBmp( );
    inline const Image& GetDefaultEntryColBmp( );

    static const Image& GetDefaultExpandedNodeImage( );
    static const Image& GetDefaultCollapsedNodeImage( );

    const Size&         GetOutputSize() const { return aOutputSize;}
    void                KeyUp( sal_Bool bPageUp, sal_Bool bNotifyScroll = sal_True );
    void                KeyDown( sal_Bool bPageDown, sal_Bool bNotifyScroll = sal_True );
    void                Command( const CommandEvent& rCEvt );

    void                Invalidate();
    void                DestroyAnchor() { pAnchor=0; aSelEng.Reset(); }
    void                SelAllDestrAnch( sal_Bool bSelect,
                            sal_Bool bDestroyAnchor = sal_True,
                            sal_Bool bSingleSelToo = sal_False );
    void                ShowCursor( sal_Bool bShow );

    sal_Bool                RequestHelp( const HelpEvent& rHEvt );
    void                EndSelection();
    sal_Bool                IsNodeButton( const Point& rPosPixel, SvTreeListEntry* pEntry ) const;
    void                RepaintScrollBars();
    void                EnableAsyncDrag( sal_Bool b) { bAsyncBeginDrag = b; }
    void                SetUpdateMode( sal_Bool );
    sal_Bool                GetUpdateMode() const { return bUpdateMode; }
    Rectangle           GetClipRegionRect() const;
    sal_Bool                HasHorScrollBar() const { return aHorSBar.IsVisible(); }
    void                ShowFocusRect( const SvTreeListEntry* pEntry );
    void                CallEventListeners( sal_uLong nEvent, void* pData = NULL );

    /** Enables, that one cell of a tablistbox entry can be focused */
    inline sal_Bool         IsCellFocusEnabled() const { return bIsCellFocusEnabled; }
    inline void         EnableCellFocus() { bIsCellFocusEnabled = sal_True; }
    bool                SetCurrentTabPos( sal_uInt16 _nNewPos );
    inline sal_uInt16       GetCurrentTabPos() const { return nCurTabPos; }

    bool                IsSelectable( const SvTreeListEntry* pEntry );
};

inline Image& SvImpLBox::implGetImageLocation( const ImageType _eType )
{
    DBG_ASSERT( ( _eType >= 0 ) && ( _eType < IT_IMAGE_COUNT ),
        "SvImpLBox::implGetImageLocation: invalid image index (will crash)!" );

    Image* _pSet = m_aNodeAndEntryImages;
    return *( _pSet + (sal_Int32)_eType );
}

inline void SvImpLBox::SetExpandedNodeBmp( const Image& rImg )
{
    implGetImageLocation( itNodeExpanded ) = rImg;
    SetNodeBmpYOffset( rImg );
}

inline void SvImpLBox::SetCollapsedNodeBmp( const Image& rImg )
{
    implGetImageLocation( itNodeCollapsed ) = rImg;
    SetNodeBmpYOffset( rImg );
}

inline const Image& SvImpLBox::GetDontKnowNodeBmp( )
{
    return implGetImageLocation( itNodeDontKnow );
}

inline const Image& SvImpLBox::GetExpandedNodeBmp( )
{
    return implGetImageLocation( itNodeExpanded );
}

inline const Image& SvImpLBox::GetCollapsedNodeBmp( )
{
    return implGetImageLocation( itNodeCollapsed );
}

inline void SvImpLBox::SetDefaultEntryExpBmp( const Image& _rImg )
{
    implGetImageLocation( itEntryDefExpanded ) = _rImg;
}

inline void SvImpLBox::SetDefaultEntryColBmp( const Image& _rImg )
{
    implGetImageLocation( itEntryDefCollapsed ) = _rImg;
}

inline const Image& SvImpLBox::GetDefaultEntryExpBmp( )
{
    return implGetImageLocation( itEntryDefExpanded );
}

inline const Image& SvImpLBox::GetDefaultEntryColBmp( )
{
    return implGetImageLocation( itEntryDefCollapsed );
}

inline Point SvImpLBox::GetEntryPosition( SvTreeListEntry* pEntry ) const
{
    return Point( 0, GetEntryLine( pEntry ) );
}

inline void SvImpLBox::PaintEntry( SvTreeListEntry* pEntry )
{
    long nY = GetEntryLine( pEntry );
    pView->PaintEntry( pEntry, nY );
}

inline sal_Bool SvImpLBox::IsLineVisible( long nY ) const
{
    sal_Bool bRet = sal_True;
    if ( nY < 0 || nY >= aOutputSize.Height() )
        bRet = sal_False;
    return bRet;
}

inline void SvImpLBox::TreeInserted( SvTreeListEntry* pInsTree )
{
    EntryInserted( pInsTree );
}

#endif // #ifndef _SVIMPLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
