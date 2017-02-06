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

#ifndef INCLUDED_SVTOOLS_SOURCE_CONTNR_IMIVCTL_HXX
#define INCLUDED_SVTOOLS_SOURCE_CONTNR_IMIVCTL_HXX

#include <vcl/virdev.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/seleng.hxx>
#include <tools/debug.hxx>
#include "svtaccessiblefactory.hxx"

#include <limits.h>

#include <svtools/ivctrl.hxx>

#include <memory>
#include <map>

class IcnCursor_Impl;
class SvtIconChoiceCtrl;
class SvxIconChoiceCtrlEntry;
class IcnViewEdit_Impl;
class IcnGridMap_Impl;


// some defines

#define PAINTFLAG_HOR_CENTERED  0x0001
#define PAINTFLAG_VER_CENTERED  0x0002

enum class IconChoiceFlags {
    NONE                         = 0x0000,
    AddMode                      = 0x0001,
    SelectingRect                = 0x0002,
    DownCtrl                     = 0x0004,
    DownDeselect                 = 0x0008,
    StartEditTimerInMouseUp      = 0x0010,
    EntryListPosValid            = 0x0020,
    ClearingSelection            = 0x0040,
    Arranging                    = 0x0080
};
namespace o3tl {
    template<> struct typed_flags<IconChoiceFlags> : is_typed_flags<IconChoiceFlags, 0x00ff> {};
}

// unit = pixels
// distances from window borders
#define LROFFS_WINBORDER            4
#define TBOFFS_WINBORDER            4
// for the bounding rectangle
#define LROFFS_BOUND                2
#define TBOFFS_BOUND                2
// distance icon to text
#define HOR_DIST_BMP_STRING         3
#define VER_DIST_BMP_STRING         3
//  width offset of highlight rectangle for Text
#define LROFFS_TEXT                 2

#define DEFAULT_MAX_VIRT_WIDTH      200
#define DEFAULT_MAX_VIRT_HEIGHT     200

#define VIEWMODE_MASK   (WB_ICON | WB_SMALLICON | WB_DETAILS)


enum class IcnViewFieldType
{
    Image,
    Text
};


// Data about the focus of entries

struct LocalFocus
{
    bool        bOn;
    Rectangle   aRect;
    Color       aPenColor;

    LocalFocus() { bOn = false; }
};


// Entry-List

typedef ::std::vector< SvxIconChoiceCtrlEntry* > SvxIconChoiceCtrlEntryList_impl;

class EntryList_Impl
{
private:
    SvxIconChoiceCtrlEntryList_impl maIconChoiceCtrlEntryList;
    SvxIconChoiceCtrl_Impl*         _pOwner;

public:
                            explicit EntryList_Impl( SvxIconChoiceCtrl_Impl* );
                            ~EntryList_Impl();

    void                    clear();

    size_t                  size()
                            {
                                return maIconChoiceCtrlEntryList.size();
                            }
    size_t                  size() const
                            {
                                return maIconChoiceCtrlEntryList.size();
                            }

    SvxIconChoiceCtrlEntry* operator[]( size_t nPos )
                            {
                                return  ( nPos < maIconChoiceCtrlEntryList.size() )
                                        ? maIconChoiceCtrlEntryList[ nPos ]
                                        : nullptr;
                            }
    SvxIconChoiceCtrlEntry* operator[]( size_t nPos ) const
                            {
                                return  ( nPos < maIconChoiceCtrlEntryList.size() )
                                        ? maIconChoiceCtrlEntryList[ nPos ]
                                        : nullptr;
                            }
    void                    insert( size_t nPos, SvxIconChoiceCtrlEntry* pEntry );
};


// Implementation-class of IconChoiceCtrl


typedef std::map<sal_uInt16, std::unique_ptr<SvxIconChoiceCtrlColumnInfo>> SvxIconChoiceCtrlColumnInfoMap;
typedef std::vector<SvxIconChoiceCtrlEntry*> SvxIconChoiceCtrlEntryPtrVec;

class SvxIconChoiceCtrl_Impl
{
    friend class IcnCursor_Impl;
    friend class EntryList_Impl;
    friend class IcnGridMap_Impl;

    bool                    bChooseWithCursor;
    EntryList_Impl          aEntries;
    VclPtr<ScrollBar>       aVerSBar;
    VclPtr<ScrollBar>       aHorSBar;
    VclPtr<ScrollBarBox>    aScrBarBox;
    Rectangle               aCurSelectionRect;
    std::vector<Rectangle*> aSelectedRectList;
    Idle                    aEditIdle;                 // for editing in place
    Idle                    aAutoArrangeIdle;
    Idle                    aDocRectChangedIdle;
    Idle                    aVisRectChangedIdle;
    Idle                    aCallSelectHdlIdle;
    Size                    aVirtOutputSize;
    Size                    aImageSize;
    Size                    aDefaultTextSize;
    Size                    aOutputSize; // Pixel
    Point                   aDDLastRectPos;
    VclPtr<SvtIconChoiceCtrl>  pView;
    IcnCursor_Impl*         pImpCursor;
    IcnGridMap_Impl*        pGridMap;
    long                    nMaxVirtWidth;  // max. width aVirtOutputSize for ALIGN_TOP
    long                    nMaxVirtHeight; // max. height aVirtOutputSize for ALIGN_LEFT
    SvxIconChoiceCtrlEntryList_impl*    pZOrderList;
    SvxIconChoiceCtrlColumnInfoMap* m_pColumns;
    VclPtr<IcnViewEdit_Impl>   pEdit;
    WinBits                 nWinBits;
    long                    nMaxBoundHeight;            // height of highest BoundRects
    IconChoiceFlags         nFlags;
    DrawTextFlags           nCurTextDrawFlags;
    ImplSVEvent *           nUserEventAdjustScrBars;
    ImplSVEvent *           nUserEventShowCursor;
    SvxIconChoiceCtrlEntry* pCurHighlightFrame;
    bool                    bHighlightFramePressed;
    SvxIconChoiceCtrlEntry* pHead;                      // top left entry
    SvxIconChoiceCtrlEntry* pCursor;
    SvxIconChoiceCtrlEntry* pPrevDropTarget;
    SvxIconChoiceCtrlEntry* pHdlEntry;
    SvxIconChoiceCtrlEntry* pDDRefEntry;
    VclPtr<VirtualDevice>   pDDDev;
    VclPtr<VirtualDevice>   pDDBufDev;
    VclPtr<VirtualDevice>   pDDTempDev;
    VclPtr<VirtualDevice>   pEntryPaintDev;
    SvxIconChoiceCtrlEntry* pAnchor;                    // for selection
    LocalFocus              aFocus;                             // Data for focusrect
    ::svt::AccessibleFactoryAccess aAccFactory;

    SvxIconChoiceCtrlEntry* pCurEditedEntry;
    SvxIconChoiceCtrlTextMode eTextMode;
    SelectionMode           eSelectionMode;
    sal_Int32               nSelectionCount;
    SvxIconChoiceCtrlPositionMode ePositionMode;
    bool                    bBoundRectsDirty;
    bool                    bUpdateMode;
    bool                    bEntryEditingEnabled;

    void                ShowCursor( bool bShow );

    void                ImpArrange( bool bKeepPredecessors );
    void                AdjustVirtSize( const Rectangle& );
    void                ResetVirtSize();
    void                CheckScrollBars();

                        DECL_LINK( ScrollUpDownHdl, ScrollBar*, void );
                        DECL_LINK( ScrollLeftRightHdl, ScrollBar*, void );
                        DECL_LINK( EditTimeoutHdl, Timer *, void);
                        DECL_LINK( UserEventHdl, void*, void );
                        DECL_LINK( AutoArrangeHdl, Timer*, void );
                        DECL_LINK( DocRectChangedHdl, Timer*, void );
                        DECL_LINK( VisRectChangedHdl, Timer*, void );
                        DECL_LINK( CallSelectHdlHdl, Timer*, void );

    void                AdjustScrollBars();
    void                PositionScrollBars( long nRealWidth, long nRealHeight );
    static long         GetScrollBarPageSize( long nVisibleRange )
                        {
                            return ((nVisibleRange*75)/100);
                        }
    long                GetScrollBarLineSize() const
                        {
                            return nMaxBoundHeight / 2;
                        }
    bool                HandleScrollCommand( const CommandEvent& rCmd );
    void                ToDocPos( Point& rPosPixel )
                        {
                            rPosPixel -= pView->GetMapMode().GetOrigin();
                        }
    void                InitScrollBarBox();
    void                ToggleSelection( SvxIconChoiceCtrlEntry* );
    void                DeselectAllBut( SvxIconChoiceCtrlEntry*, bool bPaintSync = false );
    void                Center( SvxIconChoiceCtrlEntry* pEntry ) const;
    void                StopEditTimer() { aEditIdle.Stop(); }
    void                StartEditTimer() { aEditIdle.Start(); }
    void                CallSelectHandler( SvxIconChoiceCtrlEntry* );
    void                SelectRect(
                            SvxIconChoiceCtrlEntry* pEntry1,
                            SvxIconChoiceCtrlEntry* pEntry2,
                            bool bAdd,
                            std::vector<Rectangle*>* pOtherRects
                        );

    void                SelectRange(
                            SvxIconChoiceCtrlEntry* pStart,
                            SvxIconChoiceCtrlEntry* pEnd,
                            bool bAdd
                        );

    void                AddSelectedRect( const Rectangle& );
    void                AddSelectedRect(
                            SvxIconChoiceCtrlEntry* pEntry1,
                            SvxIconChoiceCtrlEntry* pEntry2
                        );

    void                ClearSelectedRectList();
    void                ClearColumnList();
    Rectangle           CalcMaxTextRect( const SvxIconChoiceCtrlEntry* pEntry ) const;

    void                ClipAtVirtOutRect( Rectangle& rRect ) const;
    void                AdjustAtGrid( const SvxIconChoiceCtrlEntryPtrVec& rRow );
    Point               AdjustAtGrid(
                            const Rectangle& rCenterRect, // balance point of object (typically Bmp-Rect)
                            const Rectangle& rBoundRect
                        ) const;
    sal_uLong           GetPredecessorGrid( const Point& rDocPos) const;

    void                InitPredecessors();
    void                ClearPredecessors();

    bool                CheckVerScrollBar();
    bool                CheckHorScrollBar();
    void                CancelUserEvents();
    void                EntrySelected(
                            SvxIconChoiceCtrlEntry* pEntry,
                            bool bSelect,
                            bool bSyncPaint
                        );
    void                RepaintSelectedEntries();
    void                SetListPositions();
    void                SetDefaultTextSize();
    bool                IsAutoArrange() const
                        {
                            return (ePositionMode == IcnViewPositionModeAutoArrange);
                        }
    void                DocRectChanged() { aDocRectChangedIdle.Start(); }
    void                VisRectChanged() { aVisRectChangedIdle.Start(); }
    void                SetOrigin( const Point& );

                        DECL_LINK(TextEditEndedHdl, LinkParamNone*, void);

    void                ShowFocus ( Rectangle& rRect );
    void                DrawFocusRect(vcl::RenderContext& rRenderContext);

    bool                IsMnemonicChar( sal_Unicode cChar, sal_uLong& rPos ) const;

    // Copy assignment is forbidden and not implemented.
    SvxIconChoiceCtrl_Impl (const SvxIconChoiceCtrl_Impl &) = delete;
    SvxIconChoiceCtrl_Impl & operator= (const SvxIconChoiceCtrl_Impl &) = delete;

public:

    long                nGridDX;
    long                nGridDY;
    long                nHorSBarHeight;
    long                nVerSBarWidth;

                        SvxIconChoiceCtrl_Impl( SvtIconChoiceCtrl* pView, WinBits nWinStyle );
                        ~SvxIconChoiceCtrl_Impl();

    bool                SetChoiceWithCursor() { bool bOld = bChooseWithCursor; bChooseWithCursor = true; return bOld; }
    void                Clear( bool bInCtor );
    void                SetStyle( WinBits nWinStyle );
    WinBits             GetStyle() const { return nWinBits; }
    void                InsertEntry( SvxIconChoiceCtrlEntry*, size_t nPos );
    void                CreateAutoMnemonics( MnemonicGenerator* _pGenerator );
    void                FontModified();
    void                SelectAll();
    void                SelectEntry(
                            SvxIconChoiceCtrlEntry*,
                            bool bSelect,
                            bool bAddToSelection = false,
                            bool bSyncPaint = false
                        );
    void                Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
    bool                MouseButtonDown( const MouseEvent& );
    bool                MouseButtonUp( const MouseEvent& );
    bool                MouseMove( const MouseEvent&);
    bool                RequestHelp( const HelpEvent& rHEvt );
    void                SetCursor_Impl(
                            SvxIconChoiceCtrlEntry* pOldCursor,
                            SvxIconChoiceCtrlEntry* pNewCursor,
                            bool bMod1,
                            bool bShift,
                            bool bPaintSync
                        );
    bool                KeyInput( const KeyEvent& );
    void                Resize();
    void                GetFocus();
    void                LoseFocus();
    void                SetUpdateMode( bool bUpdate );
    bool                GetUpdateMode() const { return bUpdateMode; }
    void                PaintEntry(SvxIconChoiceCtrlEntry*, const Point&, vcl::RenderContext& rRenderContext);

    void                SetEntryPos(
                            SvxIconChoiceCtrlEntry* pEntry,
                            const Point& rPos
                        );

    void                InvalidateEntry( SvxIconChoiceCtrlEntry* );

    void                SetNoSelection();

    SvxIconChoiceCtrlEntry* GetCurEntry() const { return pCursor; }
    void                SetCursor( SvxIconChoiceCtrlEntry* );

    SvxIconChoiceCtrlEntry* GetEntry( const Point& rDocPos, bool bHit = false );

    void                MakeEntryVisible( SvxIconChoiceCtrlEntry* pEntry, bool bBound = true );

    void                Arrange(
                            bool bKeepPredecessors,
                            long nSetMaxVirtWidth,
                            long nSetMaxVirtHeight
                        );

    Rectangle           CalcFocusRect( SvxIconChoiceCtrlEntry* );
    Rectangle           CalcBmpRect( SvxIconChoiceCtrlEntry*, const Point* pPos = nullptr );
    Rectangle           CalcTextRect(
                            SvxIconChoiceCtrlEntry*,
                            const Point* pPos = nullptr,
                            bool bForInplaceEdit = false,
                            const OUString* pStr = nullptr
                        );

    long                CalcBoundingWidth( SvxIconChoiceCtrlEntry* ) const;
    long                CalcBoundingHeight( SvxIconChoiceCtrlEntry* ) const;
    Size                CalcBoundingSize( SvxIconChoiceCtrlEntry* ) const;
    void                FindBoundingRect( SvxIconChoiceCtrlEntry* pEntry );
    void                SetBoundingRect_Impl(
                            SvxIconChoiceCtrlEntry* pEntry,
                            const Point& rPos,
                            const Size& rBoundingSize
                        );
    // recalculates all invalid BoundRects
    void                RecalcAllBoundingRectsSmart();
    const Rectangle&    GetEntryBoundRect( SvxIconChoiceCtrlEntry* );
    void                InvalidateBoundingRect( Rectangle& rRect )
                        {
                            rRect.Right() = LONG_MAX;
                            bBoundRectsDirty = true;
                        }
    static bool         IsBoundingRectValid( const Rectangle& rRect ) { return ( rRect.Right() != LONG_MAX ); }

    void                PaintEmphasis(const Rectangle& rRect1, bool bSelected,
                                      bool bDropTarget, bool bCursored, vcl::RenderContext& rRenderContext );

    void                PaintItem(const Rectangle& rRect, IcnViewFieldType eItem, SvxIconChoiceCtrlEntry* pEntry,
                            sal_uInt16 nPaintFlags, vcl::RenderContext& rRenderContext);

    // recalculates all BoundingRects if bMustRecalcBoundingRects == true
    void                CheckBoundingRects() { if (bBoundRectsDirty) RecalcAllBoundingRectsSmart(); }
    void                Command( const CommandEvent& rCEvt );
    void                ToTop( SvxIconChoiceCtrlEntry* );

    sal_Int32           GetSelectionCount() const;
    void                SetGrid( const Size& );
    Size                GetMinGrid() const;
    void                Scroll( long nDeltaX, long nDeltaY );
    const Size&         GetItemSize( SvxIconChoiceCtrlEntry*, IcnViewFieldType ) const;

    void                HideDDIcon();

    static bool         IsOver(
                            std::vector<Rectangle*>* pSelectedRectList,
                            const Rectangle& rEntryBoundRect
                        );

    void                SelectRect(
                            const Rectangle&,
                            bool bAdd,
                            std::vector<Rectangle*>* pOtherRects
                        );

    bool               IsTextHit( SvxIconChoiceCtrlEntry* pEntry, const Point& rDocPos );
    void               MakeVisible(
                            const Rectangle& rDocPos,
                            bool bInScrollBarEvent=false
                        );

    void                AdjustEntryAtGrid();
#ifdef DBG_UTIL
    void                SetEntryTextMode(
                            SvxIconChoiceCtrlTextMode,
                            SvxIconChoiceCtrlEntry* pEntry
                        );
#endif
    bool                IsEntryEditing() const { return (pCurEditedEntry!=nullptr); }
    void                EditEntry( SvxIconChoiceCtrlEntry* pEntry );
    void                StopEntryEditing();
    size_t              GetEntryCount() const { return aEntries.size(); }
    SvxIconChoiceCtrlEntry* GetEntry( size_t nPos )
                            {
                                return aEntries[ nPos ];
                            }
    SvxIconChoiceCtrlEntry* GetEntry( size_t nPos ) const
                            {
                                return aEntries[ nPos ];
                            }
    SvxIconChoiceCtrlEntry* GetFirstSelectedEntry() const;
    SvxIconChoiceCtrlTextMode GetEntryTextModeSmart( const SvxIconChoiceCtrlEntry* pEntry ) const;
    void                SetSelectionMode( SelectionMode eMode ) { eSelectionMode=eMode; }
    sal_Int32           GetEntryListPos( SvxIconChoiceCtrlEntry* ) const;
    void                InitSettings();
    Rectangle           GetOutputRect() const;

    void                SetEntryPredecessor(SvxIconChoiceCtrlEntry* pEntry,SvxIconChoiceCtrlEntry* pPredecessor);
    // only delivers valid results when in AutoArrange mode!
    SvxIconChoiceCtrlEntry* FindEntryPredecessor( SvxIconChoiceCtrlEntry* pEntry, const Point& );

    void                SetPositionMode( SvxIconChoiceCtrlPositionMode );

    void                SetColumn( sal_uInt16 nIndex, const SvxIconChoiceCtrlColumnInfo& );
    const SvxIconChoiceCtrlColumnInfo* GetColumn( sal_uInt16 nIndex ) const;

    void                SetEntryHighlightFrame(
                            SvxIconChoiceCtrlEntry* pEntry,
                            bool bKeepHighlightFlags
                        );
    void                DrawHighlightFrame(vcl::RenderContext& rRenderContext, const Rectangle& rBmpRect);

    void                CallEventListeners( VclEventId nEvent, void* pData );

    ::svt::IAccessibleFactory& GetAccessibleFactory()
    {
        return aAccFactory.getFactory();
    }
};

typedef std::map<sal_uInt16, SvxIconChoiceCtrlEntryPtrVec> IconChoiceMap;

class IcnCursor_Impl
{
    SvxIconChoiceCtrl_Impl* pView;
    std::unique_ptr<IconChoiceMap> xColumns;
    std::unique_ptr<IconChoiceMap> xRows;
    long                    nCols;
    long                    nRows;
    short                   nDeltaWidth;
    short                   nDeltaHeight;
    SvxIconChoiceCtrlEntry* pCurEntry;
    void                    SetDeltas();
    void                    ImplCreate();
    void                    Create() {  if( !xColumns ) ImplCreate(); }

    sal_uInt16              GetSortListPos(
                                SvxIconChoiceCtrlEntryPtrVec& rList,
                                long nValue,
                                bool bVertical);
    SvxIconChoiceCtrlEntry* SearchCol(
                                sal_uInt16 nCol,
                                sal_uInt16 nTop,
                                sal_uInt16 nBottom,
                                bool bDown,
                                bool bSimple
                            );

    SvxIconChoiceCtrlEntry* SearchRow(
                                sal_uInt16 nRow,
                                sal_uInt16 nLeft,
                                sal_uInt16 nRight,
                                bool bRight,
                                bool bSimple
                            );

public:
                            explicit IcnCursor_Impl( SvxIconChoiceCtrl_Impl* pOwner );
                            ~IcnCursor_Impl();
    void                    Clear();

    // for Cursortravelling etc.
    SvxIconChoiceCtrlEntry* GoLeftRight( SvxIconChoiceCtrlEntry*, bool bRight );
    SvxIconChoiceCtrlEntry* GoUpDown( SvxIconChoiceCtrlEntry*, bool bDown );
    SvxIconChoiceCtrlEntry* GoPageUpDown( SvxIconChoiceCtrlEntry*, bool bDown );

    // Creates a list of entries for every row (height = nGridDY) sorted by
    // BoundRect.Left(). A list may be empty. The lists become the property of
    // the caller and have to be deleted with DestroyGridAdjustData.
    void                    CreateGridAjustData( IconChoiceMap& pLists );
    static void             DestroyGridAdjustData( IconChoiceMap& rLists );
};


typedef sal_uLong GridId;

#define GRID_NOT_FOUND  ((GridId)ULONG_MAX)

class IcnGridMap_Impl
{
    Rectangle               _aLastOccupiedGrid;
    SvxIconChoiceCtrl_Impl* _pView;
    bool    *               _pGridMap;
    sal_uInt16              _nGridCols, _nGridRows;

    void                Expand();
    void                Create_Impl();
    void                Create() { if(!_pGridMap) Create_Impl(); }

    void                GetMinMapSize( sal_uInt16& rDX, sal_uInt16& rDY ) const;

public:
                        explicit IcnGridMap_Impl(SvxIconChoiceCtrl_Impl* pView);
                        ~IcnGridMap_Impl();

    void                Clear();

    GridId              GetGrid( const Point& rDocPos );
    GridId              GetGrid( sal_uInt16 nGridX, sal_uInt16 nGridY );
    GridId              GetUnoccupiedGrid();

    void                OccupyGrids( const SvxIconChoiceCtrlEntry* );
    void                OccupyGrid( GridId nId )
                        {
                            DBG_ASSERT(!_pGridMap || nId<(sal_uLong)(_nGridCols*_nGridRows),"OccupyGrid: Bad GridId");
                            if(_pGridMap && nId < (sal_uLong)(_nGridCols *_nGridRows) )
                                _pGridMap[ nId ] = true;
                        }

    Rectangle           GetGridRect( GridId );
    void                GetGridCoord( GridId, sal_uInt16& rGridX, sal_uInt16& rGridY );
    static sal_uLong    GetGridCount(
                            const Size& rSizePixel,
                            sal_uInt16 nGridWidth,
                            sal_uInt16 nGridHeight
                        );

    void                OutputSizeChanged();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
