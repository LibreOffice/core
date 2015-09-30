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

#ifndef INCLUDED_SVTOOLS_SOURCE_INC_SVIMPBOX_HXX
#define INCLUDED_SVTOOLS_SOURCE_INC_SVIMPBOX_HXX

#include <vcl/seleng.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclevent.hxx>
#include <unotools/intlwrapper.hxx>
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
    VclPtr<SvTreeListBox>  pView;

public:
    ImpLBSelEng( SvImpLBox* pImp, SelectionEngine* pSelEng,
                 SvTreeListBox* pView );
    virtual ~ImpLBSelEng();
    void        BeginDrag() SAL_OVERRIDE;
    void        CreateAnchor() SAL_OVERRIDE;
    void        DestroyAnchor() SAL_OVERRIDE;
    bool        SetCursorAtPoint( const Point& rPoint,
                    bool bDontSelectAtCursor=false ) SAL_OVERRIDE;
    bool        IsSelectionAtPoint( const Point& rPoint ) SAL_OVERRIDE;
    void        DeselectAtPoint( const Point& rPoint ) SAL_OVERRIDE;
    void        DeselectAll() SAL_OVERRIDE;
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
    VclPtr<SvTreeListBox>   pView;
    SvTreeList*     pTree;
    SvTreeListEntry*        pCursor;
    SvTreeListEntry*        pStartEntry;
    SvTreeListEntry*        pAnchor;
    SvTreeListEntry*        pMostRightEntry;
    SvLBoxButton*       pActiveButton;
    SvTreeListEntry*        pActiveEntry;
    SvLBoxTab*          pActiveTab;

    VclPtr<ScrollBar>    aVerSBar;
    VclPtr<ScrollBar>    aHorSBar;
    VclPtr<ScrollBarBox> aScrBarBox;

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

    Size                aOutputSize;
    SelectionEngine     aSelEng;
    ImpLBSelEng         aFctSet;
    Idle                aAsyncBeginDragIdle;
    Point               aAsyncBeginDragPos;

    long                nYoffsNodeBmp;
    long                nNodeBmpTabDistance; // typical smaller than 0
    long                nNodeBmpWidth;
    long                nNextVerVisSize;
    long                nMostRight;
    sal_uLong               nVisibleCount;  // Number of lines in control
    ImplSVEvent *       nCurUserEvent;
    short               nHorSBarHeight, nVerSBarWidth;
    sal_uInt16              nFlags;
    sal_uInt16              nCurTabPos;

    WinBits             m_nStyle;
    ExtendedWinBits     nExtendedWinBits;
    bool                bSimpleTravel : 1; // ist true bei SINGLE_SELECTION
    bool                bUpdateMode : 1;
    bool                bInVScrollHdl : 1;
    bool                bAsyncBeginDrag : 1;
    bool                bSubLstOpRet : 1;   // open/close sublist with return/enter, defaulted with false
    bool                bSubLstOpLR : 1;    // open/close sublist with cursor left/right, defaulted with false
    bool                bContextMenuHandling : 1;
    bool                bIsCellFocusEnabled : 1;

    bool                bAreChildrenTransient;

    Point               aEditClickPos;
    Idle                aEditIdle;

    comphelper::string::NaturalStringSorter *m_pStringSorter;

    std::vector< short > aContextBmpWidthVector;

    DECL_LINK_TYPED(EditTimerCall, Idle *, void);

    DECL_LINK_TYPED( BeginDragHdl, Idle*, void );
    DECL_LINK_TYPED( MyUserEvent,  void*, void);
    void                StopUserEvent();

    void                InvalidateEntriesFrom( long nY ) const;
    void                InvalidateEntry( long nY ) const;
    void                ShowVerSBar();
    // setzt Thumb auf FirstEntryToDraw
    void                SyncVerThumb();
    bool                IsLineVisible( long nY ) const;
    long                GetEntryLine( SvTreeListEntry* pEntry ) const;
    void                FillView();
    void                CursorDown();
    void                CursorUp();
    void                KeyLeftRight( long nDiff );
    void                PageDown( sal_uInt16 nDelta );
    void                PageUp( sal_uInt16 nDelta );

    void                SetCursor( SvTreeListEntry* pEntry, bool bForceNoSelect = false );

    void                DrawNet(vcl::RenderContext& rRenderContext);

    // ScrollBar-Handler
    DECL_LINK_TYPED( ScrollUpDownHdl, ScrollBar*, void );
    DECL_LINK_TYPED( ScrollLeftRightHdl, ScrollBar*, void );
    DECL_LINK_TYPED( EndScrollHdl, ScrollBar*, void );

    void                SetNodeBmpYOffset( const Image& );
    void                SetNodeBmpTabDistance();

    // Selection-Engine
    SvTreeListEntry* MakePointVisible( const Point& rPoint, bool bNotifyScroll=true );

    void                SetAnchorSelection( SvTreeListEntry* pOld,
                            SvTreeListEntry* pNewCursor );
    void                BeginDrag();
    bool ButtonDownCheckCtrl( const MouseEvent& rMEvt, SvTreeListEntry* pEntry, long nY );
    bool MouseMoveCheckCtrl( const MouseEvent& rMEvt, SvTreeListEntry* pEntry );
    bool ButtonUpCheckCtrl( const MouseEvent& rMEvt );
    bool ButtonDownCheckExpand( const MouseEvent&, SvTreeListEntry*,long nY );

    void                PositionScrollBars( Size& rOSize, sal_uInt16 nMask );
    sal_uInt16              AdjustScrollBars( Size& rSize );

    void                BeginScroll();
    void                EndScroll();
    Rectangle           GetVisibleArea() const;
    bool EntryReallyHit(SvTreeListEntry* pEntry, const Point& rPos, long nLine);
    void                InitScrollBarBox();
    SvLBoxTab*          NextTab( SvLBoxTab* );

    bool SetMostRight( SvTreeListEntry* pEntry );
    void                FindMostRight( SvTreeListEntry* EntryToIgnore );
    void                FindMostRight( SvTreeListEntry* pParent, SvTreeListEntry* EntryToIgnore );
    void                FindMostRight_Impl( SvTreeListEntry* pParent,SvTreeListEntry* EntryToIgnore  );
    void                NotifyTabsChanged();

    // if element at cursor can be expanded in general
    bool IsExpandable() const;

    // if element at cursor can be expanded at this moment
    bool IsNowExpandable() const;

    static  void        implInitDefaultNodeImages();

    void UpdateStringSorter();

    short               UpdateContextBmpWidthVector( SvTreeListEntry* pEntry, short nWidth );
    void                UpdateContextBmpWidthMax( SvTreeListEntry* pEntry );
    void                UpdateContextBmpWidthVectorFromMovedEntry( SvTreeListEntry* pEntry );

    void                CalcCellFocusRect( SvTreeListEntry* pEntry, Rectangle& rRect );

    bool AreChildrenTransient() const { return bAreChildrenTransient; }
    inline void         SetChildrenNotTransient() { bAreChildrenTransient = false; }

public:
    SvImpLBox( SvTreeListBox* pView, SvTreeList*, WinBits nWinStyle );
    ~SvImpLBox();

    void                Clear();
    void                SetStyle( WinBits i_nWinStyle );
    void                SetExtendedWindowBits( ExtendedWinBits _nBits );
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
    void                EntrySelected( SvTreeListEntry* pEntry, bool bSelect );

    void                Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
    void                MouseButtonDown( const MouseEvent& );
    void                MouseButtonUp( const MouseEvent& );
    void                MouseMove( const MouseEvent&);
    bool                KeyInput( const KeyEvent& );
    void                Resize();
    void                GetFocus();
    void                LoseFocus();
    void UpdateAll( bool bInvalidateCompleteView= true, bool bUpdateVerSBar = true );
    void                SetEntryHeight( short nHeight );
    void                InvalidateEntry( SvTreeListEntry* );
    void                RecalcFocusRect();

    void SelectEntry( SvTreeListEntry* pEntry, bool bSelect );
    void                SetDragDropMode( DragDropMode eDDMode );
    void                SetSelectionMode( SelectionMode eSelMode  );

    SvTreeListEntry*        GetCurrentEntry() const { return pCursor; }
    bool IsEntryInView( SvTreeListEntry* pEntry ) const;
    SvTreeListEntry*        GetEntry( const Point& rPos ) const;
    // returns last entry, if Pos below last entry
    SvTreeListEntry*        GetClickedEntry( const Point& ) const;
    SvTreeListEntry*        GetCurEntry() const { return pCursor; }
    void                SetCurEntry( SvTreeListEntry* );
    Point               GetEntryPosition( SvTreeListEntry* ) const;
    void                MakeVisible( SvTreeListEntry* pEntry, bool bMoveToTop = false );
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
    void                KeyUp( bool bPageUp, bool bNotifyScroll = true );
    void                KeyDown( bool bPageDown, bool bNotifyScroll = true );
    void                Command( const CommandEvent& rCEvt );

    void                Invalidate();
    void                DestroyAnchor() { pAnchor=0; aSelEng.Reset(); }
    void SelAllDestrAnch( bool bSelect, bool bDestroyAnchor = true, bool bSingleSelToo = false );
    void ShowCursor( bool bShow );

    bool RequestHelp( const HelpEvent& rHEvt );
    void                EndSelection();
    bool IsNodeButton( const Point& rPosPixel, SvTreeListEntry* pEntry ) const;
    void EnableAsyncDrag( bool b ) { bAsyncBeginDrag = b; }
    void SetUpdateMode( bool bMode );
    bool GetUpdateMode() const { return bUpdateMode; }
    Rectangle           GetClipRegionRect() const;
    bool HasHorScrollBar() const { return aHorSBar->IsVisible(); }
    void                ShowFocusRect( const SvTreeListEntry* pEntry );
    void                CallEventListeners( sal_uLong nEvent, void* pData = NULL );

    /** Enables, that one cell of a tablistbox entry can be focused */
    bool IsCellFocusEnabled() const { return bIsCellFocusEnabled; }
    inline void         EnableCellFocus() { bIsCellFocusEnabled = true; }
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

inline bool SvImpLBox::IsLineVisible( long nY ) const
{
    bool bRet = true;
    if ( nY < 0 || nY >= aOutputSize.Height() )
        bRet = false;
    return bRet;
}

inline void SvImpLBox::TreeInserted( SvTreeListEntry* pInsTree )
{
    EntryInserted( pInsTree );
}

#endif // INCLUDED_SVTOOLS_SOURCE_INC_SVIMPBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
