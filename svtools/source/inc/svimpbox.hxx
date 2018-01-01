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
#include <o3tl/enumarray.hxx>
#include <memory>
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
    VclPtr<SvTreeListBox>  pView;

public:
    ImpLBSelEng( SvImpLBox* pImp, SvTreeListBox* pView );
    virtual ~ImpLBSelEng() override;
    void        BeginDrag() override;
    void        CreateAnchor() override;
    void        DestroyAnchor() override;
    bool        SetCursorAtPoint( const Point& rPoint,
                    bool bDontSelectAtCursor=false ) override;
    bool        IsSelectionAtPoint( const Point& rPoint ) override;
    void        DeselectAtPoint( const Point& rPoint ) override;
    void        DeselectAll() override;
};

// Flags for nFlag
enum class LBoxFlags {
    NONE                        = 0x0000,
    InScrolling                 = 0x0001,
    DeselectAll                 = 0x0002,
    StartEditTimer              = 0x0004,  // MAC only
    IgnoreSelect                = 0x0008,
    InResize                    = 0x0010,
    RemovedEntryInvisible       = 0x0020,
    RemovedRecalcMostRight      = 0x0040,
    IgnoreChangedTabs           = 0x0080,
    InPaint                     = 0x0100,
    EndScrollSetVisSize         = 0x0200,
    Filling                     = 0x0400,
};
namespace o3tl
{
    template<> struct typed_flags<LBoxFlags> : is_typed_flags<LBoxFlags, 0x07ff> {};
}

#define NODE_BMP_TABDIST_NOTVALID   -2000000
#define FIRST_ENTRY_TAB             1

class SvImpLBox
{
friend class ImpLBSelEng;
friend class SvTreeListBox;
friend class IconView;
private:
    SvTreeList*          pTree;
    SvTreeListEntry*     pAnchor;
    SvTreeListEntry*     pMostRightEntry;
    SvLBoxButton*        pActiveButton;
    SvTreeListEntry*     pActiveEntry;
    SvLBoxTab*           pActiveTab;

    VclPtr<ScrollBar>    aHorSBar;
    VclPtr<ScrollBarBox> aScrBarBox;

    ::svt::AccessibleFactoryAccess
                        m_aFactoryAccess;

    static Image*       s_pDefCollapsed;
    static Image*       s_pDefExpanded;
    static oslInterlockedCount  s_nImageRefCount; /// When 0 all static images will be destroyed

    // Node Bitmaps
    enum class ImageType
    {
        NodeExpanded = 0,     // node is expanded ( usually a bitmap showing a minus )
        NodeCollapsed,        // node is collapsed ( usually a bitmap showing a plus )
        NodeDontKnow,         // don't know the node state
        EntryDefExpanded,     // default for expanded entries
        EntryDefCollapsed,    // default for collapsed entries
        LAST = EntryDefCollapsed
    };

    // all our images
    o3tl::enumarray<ImageType, Image>
                        m_aNodeAndEntryImages;

    ImpLBSelEng         aFctSet;
    Idle                aAsyncBeginDragIdle;
    Point               aAsyncBeginDragPos;

    long                nNodeBmpWidth;
    long                nMostRight;
    short               nHorSBarHeight, nVerSBarWidth;
    sal_uInt16          nCurTabPos;

    bool                bUpdateMode : 1;
    bool                bAsyncBeginDrag : 1;
    bool                bSubLstOpRet : 1;   // open/close sublist with return/enter, defaulted with false
    bool                bSubLstOpLR : 1;    // open/close sublist with cursor left/right, defaulted with false
    bool                bSubLstOpDblClick : 1; // open/close sublist with mouse double click, defaulted with true
    bool                bContextMenuHandling : 1;
    bool                bIsCellFocusEnabled : 1;
    bool                bAreChildrenTransient;
    bool                mbForceMakeVisible;

    Point               aEditClickPos;
    Idle                aEditIdle;

    std::unique_ptr<comphelper::string::NaturalStringSorter> m_pStringSorter;

    std::vector< short > aContextBmpWidthVector;

    DECL_LINK(EditTimerCall, Timer *, void);

    DECL_LINK( BeginDragHdl, Timer*, void );

    void                InvalidateEntriesFrom( long nY ) const;
    bool                IsLineVisible( long nY ) const;
    void                KeyLeftRight( long nDiff );

    void                DrawNet(vcl::RenderContext& rRenderContext);

    // ScrollBar-Handler
    DECL_LINK( ScrollUpDownHdl, ScrollBar*, void );
    DECL_LINK( ScrollLeftRightHdl, ScrollBar*, void );
    DECL_LINK( EndScrollHdl, ScrollBar*, void );

    void                SetNodeBmpWidth( const Image& );
    void                SetNodeBmpTabDistance();

    // Selection-Engine
    SvTreeListEntry*    MakePointVisible( const Point& rPoint );

    void                SetAnchorSelection( SvTreeListEntry* pOld,
                            SvTreeListEntry* pNewCursor );
    void                BeginDrag();
    bool                ButtonDownCheckCtrl( const MouseEvent& rMEvt, SvTreeListEntry* pEntry );
    bool                MouseMoveCheckCtrl( const MouseEvent& rMEvt, SvTreeListEntry const * pEntry );
    bool                ButtonUpCheckCtrl( const MouseEvent& rMEvt );
    bool                ButtonDownCheckExpand( const MouseEvent&, SvTreeListEntry* );

    bool                EntryReallyHit(SvTreeListEntry* pEntry, const Point& rPos, long nLine);
    void                InitScrollBarBox();
    SvLBoxTab*          NextTab( SvLBoxTab const * );

    bool                SetMostRight( SvTreeListEntry* pEntry );
    void                FindMostRight( SvTreeListEntry* pParent, SvTreeListEntry* EntryToIgnore );
    void                FindMostRight_Impl( SvTreeListEntry* pParent,SvTreeListEntry* EntryToIgnore  );
    void                NotifyTabsChanged();

    // if element at cursor can be expanded in general
    bool                IsExpandable() const;

    static  void        implInitDefaultNodeImages();

    void                UpdateStringSorter();

    short               UpdateContextBmpWidthVector( SvTreeListEntry const * pEntry, short nWidth );
    void                UpdateContextBmpWidthMax( SvTreeListEntry const * pEntry );
    void                UpdateContextBmpWidthVectorFromMovedEntry( SvTreeListEntry* pEntry );

    void                CalcCellFocusRect( SvTreeListEntry const * pEntry, tools::Rectangle& rRect );

    bool                AreChildrenTransient() const { return bAreChildrenTransient; }
    void                SetChildrenNotTransient() { bAreChildrenTransient = false; }

protected:
    VclPtr<SvTreeListBox>   pView;
    VclPtr<ScrollBar>       aVerSBar;
    SvTreeListEntry*        pCursor;
    SvTreeListEntry*        pStartEntry;
    ImplSVEvent*            nCurUserEvent;
    Size                    aOutputSize;
    LBoxFlags               nFlags;
    WinBits                 m_nStyle;
    bool                    mbNoAutoCurEntry; // disable the behavior of automatically selecting a "CurEntry" upon painting the control
    SelectionEngine         aSelEng;
    sal_uLong               nVisibleCount;  // Number of lines in control
    bool                    bInVScrollHdl : 1;
    bool                    bSimpleTravel : 1; // is true if SelectionMode::Single
    long                    nNextVerVisSize;
    long                    nNodeBmpTabDistance; // typical smaller than 0

    virtual long        GetEntryLine( SvTreeListEntry* pEntry ) const;
    virtual void        CursorDown();
    virtual void        CursorUp();
    virtual void        PageDown( sal_uInt16 nDelta );
    virtual void        PageUp( sal_uInt16 nDelta );
    // set Thumb to FirstEntryToDraw
    virtual void        SyncVerThumb();
    virtual void        AdjustScrollBars( Size& rSize );
    virtual void        InvalidateEntry( long nY ) const;

    tools::Rectangle           GetVisibleArea() const;
    void                SetCursor( SvTreeListEntry* pEntry, bool bForceNoSelect = false );
    void                BeginScroll();
    void                EndScroll();
    void                PositionScrollBars( Size& rOSize, sal_uInt16 nMask );
    void                FindMostRight( SvTreeListEntry const * EntryToIgnore );
    void                FillView();
    void                ShowVerSBar();
    void                StopUserEvent();

    DECL_LINK( MyUserEvent,  void*, void);

public:
    SvImpLBox( SvTreeListBox* pView, SvTreeList*, WinBits nWinStyle );
    virtual ~SvImpLBox();

    void                Clear();
    void                SetStyle( WinBits i_nWinStyle );
    void                SetNoAutoCurEntry( bool b );
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

    virtual void        Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
    void                MouseButtonDown( const MouseEvent& );
    void                MouseButtonUp( const MouseEvent& );
    void                MouseMove( const MouseEvent&);
    virtual bool        KeyInput( const KeyEvent& );
    void                Resize();
    void                GetFocus();
    void                LoseFocus();
    virtual void        UpdateAll( bool bInvalidateCompleteView );
    void                SetEntryHeight();
    void                InvalidateEntry( SvTreeListEntry* );
    void                RecalcFocusRect();

    void                SelectEntry( SvTreeListEntry* pEntry, bool bSelect );
    void                SetDragDropMode( DragDropMode eDDMode );
    void                SetSelectionMode( SelectionMode eSelMode  );

    SvTreeListEntry*    GetCurrentEntry() const { return pCursor; }
    virtual bool        IsEntryInView( SvTreeListEntry* pEntry ) const;
    virtual SvTreeListEntry*    GetEntry( const Point& rPos ) const;
    // returns last entry, if Pos below last entry
    virtual SvTreeListEntry*    GetClickedEntry( const Point& ) const;
    SvTreeListEntry*    GetCurEntry() const { return pCursor; }
    void                SetCurEntry( SvTreeListEntry* );
    virtual Point       GetEntryPosition( SvTreeListEntry* ) const;
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
    virtual void        KeyUp( bool bPageUp );
    virtual void        KeyDown( bool bPageDown );
    void                Command( const CommandEvent& rCEvt );

    void                Invalidate();
    void                DestroyAnchor() { pAnchor=nullptr; aSelEng.Reset(); }
    void                SelAllDestrAnch( bool bSelect, bool bDestroyAnchor = true, bool bSingleSelToo = false );
    void                ShowCursor( bool bShow );

    bool                RequestHelp( const HelpEvent& rHEvt );
    void                EndSelection();
    bool                IsNodeButton( const Point& rPosPixel, SvTreeListEntry* pEntry ) const;
    void                EnableAsyncDrag( bool b ) { bAsyncBeginDrag = b; }
    void                SetUpdateMode( bool bMode );
    bool                GetUpdateMode() const { return bUpdateMode; }
    tools::Rectangle    GetClipRegionRect() const;
    bool                HasHorScrollBar() const { return aHorSBar->IsVisible(); }
    void                ShowFocusRect( const SvTreeListEntry* pEntry );
    void                CallEventListeners( VclEventId nEvent, void* pData = nullptr );

    /** Enables, that one cell of a tablistbox entry can be focused */
    bool                IsCellFocusEnabled() const { return bIsCellFocusEnabled; }
    void                EnableCellFocus() { bIsCellFocusEnabled = true; }
    bool                SetCurrentTabPos( sal_uInt16 _nNewPos );
    sal_uInt16          GetCurrentTabPos() const { return nCurTabPos; }

    bool                IsSelectable( const SvTreeListEntry* pEntry );
    void                SetForceMakeVisible(bool bEnable) { mbForceMakeVisible = bEnable; }
};

inline Image& SvImpLBox::implGetImageLocation( const ImageType _eType )
{
    return m_aNodeAndEntryImages[_eType];
}

inline void SvImpLBox::SetExpandedNodeBmp( const Image& rImg )
{
    implGetImageLocation( ImageType::NodeExpanded ) = rImg;
    SetNodeBmpWidth( rImg );
}

inline void SvImpLBox::SetCollapsedNodeBmp( const Image& rImg )
{
    implGetImageLocation( ImageType::NodeCollapsed ) = rImg;
    SetNodeBmpWidth( rImg );
}

inline const Image& SvImpLBox::GetDontKnowNodeBmp( )
{
    return implGetImageLocation( ImageType::NodeDontKnow );
}

inline const Image& SvImpLBox::GetExpandedNodeBmp( )
{
    return implGetImageLocation( ImageType::NodeExpanded );
}

inline const Image& SvImpLBox::GetCollapsedNodeBmp( )
{
    return implGetImageLocation( ImageType::NodeCollapsed );
}

inline void SvImpLBox::SetDefaultEntryExpBmp( const Image& _rImg )
{
    implGetImageLocation( ImageType::EntryDefExpanded ) = _rImg;
}

inline void SvImpLBox::SetDefaultEntryColBmp( const Image& _rImg )
{
    implGetImageLocation( ImageType::EntryDefCollapsed ) = _rImg;
}

inline const Image& SvImpLBox::GetDefaultEntryExpBmp( )
{
    return implGetImageLocation( ImageType::EntryDefExpanded );
}

inline const Image& SvImpLBox::GetDefaultEntryColBmp( )
{
    return implGetImageLocation( ImageType::EntryDefCollapsed );
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
