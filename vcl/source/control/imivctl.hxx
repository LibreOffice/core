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

#pragma once

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <vcl/virdev.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/vclptr.hxx>
#include <tools/debug.hxx>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/toolkit/scrbar.hxx>

#include <limits.h>


#include <memory>
#include <map>

class IcnCursor_Impl;
class SvtIconChoiceCtrl;
class SvxIconChoiceCtrlEntry;
class IcnGridMap_Impl;


// some defines

#define PAINTFLAG_HOR_CENTERED  0x0001
#define PAINTFLAG_VER_CENTERED  0x0002

enum class IconChoiceFlags {
    NONE                         = 0x0000,
    AddMode                      = 0x0001,
    ClearingSelection            = 0x0002,
    Arranging                    = 0x0004,
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
// distance icon to text
#define HOR_DIST_BMP_STRING         3
#define VER_DIST_BMP_STRING         3
//  width offset of highlight rectangle for Text
#define LROFFS_TEXT                 2
// Vertical text padding when the item contains only text
#define VERT_TEXT_PADDING           4
// Width of the marker used for the selected tab (native controls only)
#define TAB_MARK_WIDTH              3

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


typedef sal_uLong GridId;

// Implementation-class of IconChoiceCtrl


typedef std::vector<SvxIconChoiceCtrlEntry*> SvxIconChoiceCtrlEntryPtrVec;

class SvxIconChoiceCtrl_Impl
{
    friend class IcnCursor_Impl;
    friend class IcnGridMap_Impl;

    std::vector< std::unique_ptr<SvxIconChoiceCtrlEntry> > maEntries;
    VclPtr<ScrollBar>       aVerSBar;
    VclPtr<ScrollBar>       aHorSBar;
    VclPtr<ScrollBarBox>    aScrBarBox;
    Idle                    aDocRectChangedIdle;
    Idle                    aVisRectChangedIdle;
    Size                    aVirtOutputSize;
    Size                    aImageSize;
    Size                    aDefaultTextSize;
    Size                    aOutputSize; // Pixel
    VclPtr<SvtIconChoiceCtrl>  pView;
    std::unique_ptr<IcnCursor_Impl> pImpCursor;
    std::unique_ptr<IcnGridMap_Impl> pGridMap;
    tools::Long                    nMaxVirtHeight; // max. height aVirtOutputSize
    std::vector< SvxIconChoiceCtrlEntry* > maZOrderList;
    WinBits                 nWinBits;
    tools::Long                    nMaxBoundHeight;            // height of highest BoundRects
    IconChoiceFlags         nFlags;
    DrawTextFlags           nCurTextDrawFlags;
    ImplSVEvent *           nUserEventAdjustScrBars;
    SvxIconChoiceCtrlEntry* pCurHighlightFrame;
    SvxIconChoiceCtrlEntry* pCursor;
    LocalFocus              aFocus;                             // Data for focusrect

    bool                    bBoundRectsDirty;

    void                ShowCursor( bool bShow );

    void                ImpArrange();
    void                AdjustVirtSize( const tools::Rectangle& );
    void                ResetVirtSize();
    void                CheckScrollBars();

                        DECL_LINK( ScrollUpDownHdl, ScrollBar*, void );
                        DECL_LINK( ScrollLeftRightHdl, ScrollBar*, void );
                        DECL_LINK( UserEventHdl, void*, void );
                        DECL_LINK( DocRectChangedHdl, Timer*, void );
                        DECL_LINK( VisRectChangedHdl, Timer*, void );

    void                AdjustScrollBars();
    void                PositionScrollBars( tools::Long nRealWidth, tools::Long nRealHeight );
    static tools::Long         GetScrollBarPageSize( tools::Long nVisibleRange )
                        {
                            return ((nVisibleRange*75)/100);
                        }
    tools::Long                GetScrollBarLineSize() const
                        {
                            return nMaxBoundHeight / 2;
                        }
    bool                HandleScrollCommand( const CommandEvent& rCmd );
    void                ToDocPos( Point& rPosPixel )
                        {
                            rPosPixel -= pView->GetMapMode().GetOrigin();
                        }
    void                InitScrollBarBox();
    void                DeselectAllBut( SvxIconChoiceCtrlEntry const * );

    tools::Rectangle           CalcMaxTextRect( const SvxIconChoiceCtrlEntry* pEntry ) const;

    void                ClipAtVirtOutRect( tools::Rectangle& rRect ) const;

    bool                CheckVerScrollBar();
    bool                CheckHorScrollBar();
    void                CancelUserEvents();
    void                EntrySelected(
                            SvxIconChoiceCtrlEntry* pEntry,
                            bool bSelect
                        );
    void                RepaintSelectedEntries();
    void                SetDefaultTextSize();
    void                DocRectChanged() { aDocRectChangedIdle.Start(); }
    void                VisRectChanged() { aVisRectChangedIdle.Start(); }
    void                SetOrigin( const Point& );

    void                ShowFocus ( tools::Rectangle const & rRect );
    void                DrawFocusRect(vcl::RenderContext& rRenderContext, SvxIconChoiceCtrlEntry* pEntry);

    bool                IsMnemonicChar( sal_Unicode cChar, sal_uLong& rPos ) const;

    // Copy assignment is forbidden and not implemented.
    SvxIconChoiceCtrl_Impl (const SvxIconChoiceCtrl_Impl &) = delete;
    SvxIconChoiceCtrl_Impl & operator= (const SvxIconChoiceCtrl_Impl &) = delete;

public:

    tools::Long                nGridDX;
    tools::Long                nGridDY;
    tools::Long                nHorSBarHeight;
    tools::Long                nVerSBarWidth;

                        SvxIconChoiceCtrl_Impl( SvtIconChoiceCtrl* pView, WinBits nWinStyle );
                        ~SvxIconChoiceCtrl_Impl();

    void                Clear( bool bInCtor );
    void                SetStyle( WinBits nWinStyle );
    void                InsertEntry( std::unique_ptr<SvxIconChoiceCtrlEntry>, size_t nPos );
    void                RemoveEntry( size_t nPos );
    void                FontModified();
    void                SelectEntry(
                            SvxIconChoiceCtrlEntry*,
                            bool bSelect,
                            bool bAddToSelection = false
                        );
    void                Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
    bool                MouseButtonDown( const MouseEvent& );
    bool                MouseMove( const MouseEvent&);
    bool                RequestHelp( const HelpEvent& rHEvt );
    void                SetCursor_Impl(SvxIconChoiceCtrlEntry* pNewCursor);
    bool                KeyInput( const KeyEvent& );
    void                Resize();
    void                GetFocus();
    void                LoseFocus();
    void                PaintEntry(SvxIconChoiceCtrlEntry*, const Point&, vcl::RenderContext& rRenderContext);

    SvxIconChoiceCtrlEntry* GetCurEntry() const { return pCursor; }
    void                SetCursor( SvxIconChoiceCtrlEntry* );

    SvxIconChoiceCtrlEntry* GetEntry( const Point& rDocPos );

    void                MakeEntryVisible(SvxIconChoiceCtrlEntry* pEntry);

    void                Arrange(tools::Long nSetMaxVirtHeight);

    tools::Rectangle           CalcFocusRect( SvxIconChoiceCtrlEntry* );
    tools::Rectangle           CalcBmpRect( SvxIconChoiceCtrlEntry*, const Point* pPos = nullptr );
    tools::Rectangle           CalcTextRect(
                            SvxIconChoiceCtrlEntry*,
                            const Point* pPos = nullptr
                        );

    tools::Long                CalcBoundingWidth() const;
    tools::Long                CalcBoundingHeight() const;
    Size                CalcBoundingSize() const;
    void                FindBoundingRect( SvxIconChoiceCtrlEntry* pEntry );
    // recalculates all invalid BoundRects
    void                RecalcAllBoundingRectsSmart();
    const tools::Rectangle&    GetEntryBoundRect( SvxIconChoiceCtrlEntry* );
    void                InvalidateBoundingRect( tools::Rectangle& rRect )
                        {
                            rRect.SetRight(LONG_MAX);
                            bBoundRectsDirty = true;
                        }
    static bool         IsBoundingRectValid( const tools::Rectangle& rRect ) { return ( rRect.Right() != LONG_MAX ); }

    static void         PaintEmphasis(const tools::Rectangle& rRect1, vcl::RenderContext& rRenderContext);

    void                PaintItem(const tools::Rectangle& rRect, IcnViewFieldType eItem, const SvxIconChoiceCtrlEntry* pEntry,
                            sal_uInt16 nPaintFlags, vcl::RenderContext& rRenderContext);

    // recalculates all BoundingRects if bMustRecalcBoundingRects == true
    void                CheckBoundingRects() { if (bBoundRectsDirty) RecalcAllBoundingRectsSmart(); }
    void                Command( const CommandEvent& rCEvt );
    void                ToTop( SvxIconChoiceCtrlEntry* );

    void                SetGrid( const Size& );
    Size                GetMinGrid() const;
    void                Scroll( tools::Long nDeltaX, tools::Long nDeltaY );
    const Size&         GetItemSize( IcnViewFieldType ) const;

    void               MakeVisible(
                            const tools::Rectangle& rDocPos,
                            bool bInScrollBarEvent=false
                        );

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

    void                SetEntryHighlightFrame(SvxIconChoiceCtrlEntry* pEntry);
    static void         DrawHighlightFrame(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);

    void                CallEventListeners( VclEventId nEvent, void* pData );
};

typedef std::map<sal_uInt16, SvxIconChoiceCtrlEntryPtrVec> IconChoiceMap;

class IcnCursor_Impl
{
    SvxIconChoiceCtrl_Impl* pView;
    std::unique_ptr<IconChoiceMap> xColumns;
    std::unique_ptr<IconChoiceMap> xRows;
    tools::Long                    nCols;
    tools::Long                    nRows;
    short                   nDeltaWidth;
    short                   nDeltaHeight;
    SvxIconChoiceCtrlEntry* pCurEntry;
    void                    SetDeltas();
    void                    ImplCreate();
    void                    Create() {  if( !xColumns ) ImplCreate(); }

    sal_uInt16              GetSortListPos(
                                SvxIconChoiceCtrlEntryPtrVec& rList,
                                tools::Long nValue,
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
    SvxIconChoiceCtrlEntry* GoUpDown( const SvxIconChoiceCtrlEntry*, bool bDown );
    SvxIconChoiceCtrlEntry* GoPageUpDown( const SvxIconChoiceCtrlEntry*, bool bDown );
};

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
