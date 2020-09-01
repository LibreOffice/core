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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/virdev.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/vclptr.hxx>
#include <tools/debug.hxx>
#include <vcl/svtaccessiblefactory.hxx>

#include <limits.h>


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
    EntryListPosValid            = 0x0010,
    ClearingSelection            = 0x0020,
    Arranging                    = 0x0040
};
namespace o3tl {
    template<> struct typed_flags<IconChoiceFlags> : is_typed_flags<IconChoiceFlags, 0x007f> {};
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
    tools::Rectangle   aRect;
    Color              aPenColor;
};


// Implementation-class of IconChoiceCtrl


typedef std::map<sal_uInt16, std::unique_ptr<SvxIconChoiceCtrlColumnInfo>> SvxIconChoiceCtrlColumnInfoMap;
typedef std::vector<SvxIconChoiceCtrlEntry*> SvxIconChoiceCtrlEntryPtrVec;

class SvxIconChoiceCtrl_Impl
{
    friend class IcnCursor_Impl;
    friend class IcnGridMap_Impl;

    std::vector< std::unique_ptr<SvxIconChoiceCtrlEntry> > maEntries;
    VclPtr<ScrollBar>       aVerSBar;
    VclPtr<ScrollBar>       aHorSBar;
    VclPtr<ScrollBarBox>    aScrBarBox;
    tools::Rectangle               aCurSelectionRect;
    std::vector<tools::Rectangle> aSelectedRectList;
    Idle                    aAutoArrangeIdle;
    Idle                    aDocRectChangedIdle;
    Idle                    aVisRectChangedIdle;
    Idle                    aCallSelectHdlIdle;
    Size                    aVirtOutputSize;
    Size                    aImageSize;
    Size                    aDefaultTextSize;
    Size                    aOutputSize; // Pixel
    VclPtr<SvtIconChoiceCtrl>  pView;
    std::unique_ptr<IcnCursor_Impl> pImpCursor;
    std::unique_ptr<IcnGridMap_Impl> pGridMap;
    long                    nMaxVirtWidth;  // max. width aVirtOutputSize for ALIGN_TOP
    long                    nMaxVirtHeight; // max. height aVirtOutputSize for ALIGN_LEFT
    std::vector< SvxIconChoiceCtrlEntry* > maZOrderList;
    std::unique_ptr<SvxIconChoiceCtrlColumnInfoMap> m_pColumns;
    WinBits                 nWinBits;
    long                    nMaxBoundHeight;            // height of highest BoundRects
    IconChoiceFlags         nFlags;
    DrawTextFlags           nCurTextDrawFlags;
    ImplSVEvent *           nUserEventAdjustScrBars;
    SvxIconChoiceCtrlEntry* pCurHighlightFrame;
    bool                    bHighlightFramePressed;
    SvxIconChoiceCtrlEntry* pHead = nullptr;            // top left entry
    SvxIconChoiceCtrlEntry* pCursor;
    SvxIconChoiceCtrlEntry* pHdlEntry;
    SvxIconChoiceCtrlEntry* pAnchor;                    // for selection
    LocalFocus              aFocus;                             // Data for focusrect
    ::vcl::AccessibleFactoryAccess aAccFactory;

    SvxIconChoiceCtrlTextMode eTextMode;
    SelectionMode           eSelectionMode;
    sal_Int32               nSelectionCount;
    SvxIconChoiceCtrlPositionMode ePositionMode;
    bool                    bBoundRectsDirty;
    bool                    bUpdateMode;

    void                ShowCursor( bool bShow );

    void                ImpArrange( bool bKeepPredecessors );
    void                AdjustVirtSize( const tools::Rectangle& );
    void                ResetVirtSize();
    void                CheckScrollBars();

                        DECL_LINK( ScrollUpDownHdl, ScrollBar*, void );
                        DECL_LINK( ScrollLeftRightHdl, ScrollBar*, void );
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
    void                DeselectAllBut( SvxIconChoiceCtrlEntry const * );
    void                Center( SvxIconChoiceCtrlEntry* pEntry ) const;
    void                CallSelectHandler();
    void                SelectRect(
                            SvxIconChoiceCtrlEntry* pEntry1,
                            SvxIconChoiceCtrlEntry* pEntry2,
                            bool bAdd,
                            std::vector<tools::Rectangle>* pOtherRects
                        );

    void                SelectRange(
                            SvxIconChoiceCtrlEntry const * pStart,
                            SvxIconChoiceCtrlEntry const * pEnd,
                            bool bAdd
                        );

    void                AddSelectedRect( const tools::Rectangle& );
    void                AddSelectedRect(
                            SvxIconChoiceCtrlEntry* pEntry1,
                            SvxIconChoiceCtrlEntry* pEntry2
                        );

    void                ClearSelectedRectList();
    tools::Rectangle           CalcMaxTextRect( const SvxIconChoiceCtrlEntry* pEntry ) const;

    void                ClipAtVirtOutRect( tools::Rectangle& rRect ) const;
    sal_uLong           GetPredecessorGrid( const Point& rDocPos) const;

    void                InitPredecessors();
    void                ClearPredecessors();

    bool                CheckVerScrollBar();
    bool                CheckHorScrollBar();
    void                CancelUserEvents();
    void                EntrySelected(
                            SvxIconChoiceCtrlEntry* pEntry,
                            bool bSelect
                        );
    void                RepaintSelectedEntries();
    void                SetListPositions();
    void                SetDefaultTextSize();
    bool                IsAutoArrange() const
                        {
                            return (ePositionMode == SvxIconChoiceCtrlPositionMode::AutoArrange);
                        }
    void                DocRectChanged() { aDocRectChangedIdle.Start(); }
    void                VisRectChanged() { aVisRectChangedIdle.Start(); }
    void                SetOrigin( const Point& );

    void                ShowFocus ( tools::Rectangle const & rRect );
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

    void                Clear( bool bInCtor );
    void                SetStyle( WinBits nWinStyle );
    WinBits             GetStyle() const { return nWinBits; }
    void                InsertEntry( std::unique_ptr<SvxIconChoiceCtrlEntry>, size_t nPos );
    void                RemoveEntry( size_t nPos );
    void                FontModified();
    void                SelectAll();
    void                SelectEntry(
                            SvxIconChoiceCtrlEntry*,
                            bool bSelect,
                            bool bAddToSelection = false
                        );
    void                Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
    bool                MouseButtonDown( const MouseEvent& );
    bool                MouseButtonUp( const MouseEvent& );
    bool                MouseMove( const MouseEvent&);
    bool                RequestHelp( const HelpEvent& rHEvt );
    void                SetCursor_Impl(
                            SvxIconChoiceCtrlEntry* pOldCursor,
                            SvxIconChoiceCtrlEntry* pNewCursor,
                            bool bMod1,
                            bool bShift
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

    tools::Rectangle           CalcFocusRect( SvxIconChoiceCtrlEntry* );
    tools::Rectangle           CalcBmpRect( SvxIconChoiceCtrlEntry*, const Point* pPos = nullptr );
    tools::Rectangle           CalcTextRect(
                            SvxIconChoiceCtrlEntry*,
                            const Point* pPos = nullptr,
                            const OUString* pStr = nullptr
                        );

    long                CalcBoundingWidth() const;
    long                CalcBoundingHeight() const;
    Size                CalcBoundingSize() const;
    void                FindBoundingRect( SvxIconChoiceCtrlEntry* pEntry );
    void                SetBoundingRect_Impl(
                            SvxIconChoiceCtrlEntry* pEntry,
                            const Point& rPos,
                            const Size& rBoundingSize
                        );
    // recalculates all invalid BoundRects
    void                RecalcAllBoundingRectsSmart();
    const tools::Rectangle&    GetEntryBoundRect( SvxIconChoiceCtrlEntry* );
    void                InvalidateBoundingRect( tools::Rectangle& rRect )
                        {
                            rRect.SetRight(LONG_MAX);
                            bBoundRectsDirty = true;
                        }
    static bool         IsBoundingRectValid( const tools::Rectangle& rRect ) { return ( rRect.Right() != LONG_MAX ); }

    static void         PaintEmphasis(const tools::Rectangle& rRect1, bool bSelected,
                                      vcl::RenderContext& rRenderContext );

    void                PaintItem(const tools::Rectangle& rRect, IcnViewFieldType eItem, SvxIconChoiceCtrlEntry* pEntry,
                            sal_uInt16 nPaintFlags, vcl::RenderContext& rRenderContext);

    // recalculates all BoundingRects if bMustRecalcBoundingRects == true
    void                CheckBoundingRects() { if (bBoundRectsDirty) RecalcAllBoundingRectsSmart(); }
    void                Command( const CommandEvent& rCEvt );
    void                ToTop( SvxIconChoiceCtrlEntry* );

    sal_Int32           GetSelectionCount() const;
    void                SetGrid( const Size& );
    Size                GetMinGrid() const;
    void                Scroll( long nDeltaX, long nDeltaY );
    const Size&         GetItemSize( IcnViewFieldType ) const;

    void                HideDDIcon();

    static bool         IsOver(
                            std::vector<tools::Rectangle>* pSelectedRectList,
                            const tools::Rectangle& rEntryBoundRect
                        );

    void                SelectRect(
                            const tools::Rectangle&,
                            bool bAdd,
                            std::vector<tools::Rectangle>* pOtherRects
                        );

    void               MakeVisible(
                            const tools::Rectangle& rDocPos,
                            bool bInScrollBarEvent=false
                        );

#ifdef DBG_UTIL
    void                SetEntryTextMode(
                            SvxIconChoiceCtrlTextMode,
                            SvxIconChoiceCtrlEntry* pEntry
                        );
#endif
    size_t              GetEntryCount() const { return maEntries.size(); }
    SvxIconChoiceCtrlEntry* GetEntry( size_t nPos )
                            {
                                return maEntries[ nPos ].get();
                            }
    SvxIconChoiceCtrlEntry* GetEntry( size_t nPos ) const
                            {
                                return maEntries[ nPos ].get();
                            }
    SvxIconChoiceCtrlEntry* GetFirstSelectedEntry() const;
    sal_Int32           GetEntryListPos( SvxIconChoiceCtrlEntry const * ) const;
    void                InitSettings();
    tools::Rectangle           GetOutputRect() const;

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
    void                DrawHighlightFrame(vcl::RenderContext& rRenderContext, const tools::Rectangle& rBmpRect);

    void                CallEventListeners( VclEventId nEvent, void* pData );

    ::vcl::IAccessibleFactory& GetAccessibleFactory()
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
};


typedef sal_uLong GridId;

#define GRID_NOT_FOUND  ((GridId)ULONG_MAX)

class IcnGridMap_Impl
{
    tools::Rectangle               _aLastOccupiedGrid;
    SvxIconChoiceCtrl_Impl* _pView;
    std::unique_ptr<bool[]> _pGridMap;
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
                            DBG_ASSERT(!_pGridMap || nId<o3tl::make_unsigned(_nGridCols*_nGridRows),"OccupyGrid: Bad GridId");
                            if(_pGridMap && nId < o3tl::make_unsigned(_nGridCols *_nGridRows) )
                                _pGridMap[ nId ] = true;
                        }

    tools::Rectangle           GetGridRect( GridId );
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
