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

#ifndef _IMPICNVW_HXX
#define _IMPICNVW_HXX

#include <vcl/virdev.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/timer.hxx>
#include <vcl/seleng.hxx>
#include <tools/debug.hxx>
#include "svtaccessiblefactory.hxx"

#include <limits.h>

#include <ivctrl.hxx>
#include <boost/ptr_container/ptr_map.hpp>

class IcnCursor_Impl;
class SvtIconChoiceCtrl;
class SvxIconChoiceCtrlEntry;
class IcnViewEdit_Impl;
class IcnGridMap_Impl;

///////////////////////////////////////////////////////////////////////////////
//
// some defines
//
#define PAINTFLAG_HOR_CENTERED  0x0001
#define PAINTFLAG_VER_CENTERED  0x0002

#define F_VER_SBARSIZE_WITH_HBAR        0x0001
#define F_HOR_SBARSIZE_WITH_VBAR        0x0002
#define F_PAINTED                       0x0004  // sal_True after first paint
#define F_ADD_MODE                      0x0008
#define F_SELECTING_RECT                0x0020
#define F_DOWN_CTRL                     0x0080
#define F_DOWN_DESELECT                 0x0100
#define F_START_EDITTIMER_IN_MOUSEUP    0x0400
#define F_MOVED_ENTRIES                 0x0800
#define F_ENTRYLISTPOS_VALID            0x1000
#define F_CLEARING_SELECTION            0x2000
#define F_ARRANGING                     0x4000

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

///////////////////////////////////////////////////////////////////////////////
//
//
//
enum IcnViewFieldType
{
    IcnViewFieldTypeDontknow = 0,
    IcnViewFieldTypeImage = 1,
    IcnViewFieldTypeText = 2
};

///////////////////////////////////////////////////////////////////////////////
//
// Data about the focus of entries
//
struct LocalFocus
{
    sal_Bool    bOn;
    Rectangle   aRect;
    Color       aPenColor;

    LocalFocus() { bOn = sal_False; }
};

///////////////////////////////////////////////////////////////////////////////
//
// Entry-List
//
typedef ::std::vector< SvxIconChoiceCtrlEntry* > SvxIconChoiceCtrlEntryList_impl;

class EntryList_Impl
{
private:
    SvxIconChoiceCtrlEntryList_impl maIconChoiceCtrlEntryList;
    SvxIconChoiceCtrl_Impl*         _pOwner;

public:
                            EntryList_Impl( SvxIconChoiceCtrl_Impl* );
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
                                        : NULL;
                            }
    SvxIconChoiceCtrlEntry* operator[]( size_t nPos ) const
                            {
                                return  ( nPos < maIconChoiceCtrlEntryList.size() )
                                        ? maIconChoiceCtrlEntryList[ nPos ]
                                        : NULL;
                            }
    void                    insert( size_t nPos, SvxIconChoiceCtrlEntry* pEntry );
};


///////////////////////////////////////////////////////////////////////////////
//
// Implementation-class of IconChoiceCtrl
//

typedef boost::ptr_map<sal_uInt16, SvxIconChoiceCtrlColumnInfo> SvxIconChoiceCtrlColumnInfoMap;
typedef std::vector<SvxIconChoiceCtrlEntry*> SvxIconChoiceCtrlEntryPtrVec;

class SvxIconChoiceCtrl_Impl
{
    friend class IcnCursor_Impl;
    friend class EntryList_Impl;
    friend class IcnGridMap_Impl;

    sal_Bool                bChooseWithCursor;
    EntryList_Impl          aEntries;
    ScrollBar               aVerSBar;
    ScrollBar               aHorSBar;
    ScrollBarBox            aScrBarBox;
    Rectangle               aCurSelectionRect;
    std::vector<Rectangle*> aSelectedRectList;
    Timer                   aEditTimer;                 // for editing in place
    Timer                   aAutoArrangeTimer;
    Timer                   aDocRectChangedTimer;
    Timer                   aVisRectChangedTimer;
    Timer                   aCallSelectHdlTimer;
    Size                    aVirtOutputSize;
    Size                    aImageSize;
    Size                    aDefaultTextSize;
    Size                    aOutputSize; // Pixel
    Point                   aDDLastEntryPos;
    Point                   aDDLastRectPos;
    Point                   aDDPaintOffs;
    Point                   aDDStartPos;
    SvtIconChoiceCtrl*      pView;
    IcnCursor_Impl*         pImpCursor;
    IcnGridMap_Impl*        pGridMap;
    long                    nMaxVirtWidth;  // max. width aVirtOutputSize for ALIGN_TOP
    long                    nMaxVirtHeight; // max. height aVirtOutputSize for ALIGN_LEFT
    SvxIconChoiceCtrlEntryList_impl*    pZOrderList;
    SvxIconChoiceCtrlColumnInfoMap* pColumns;
    IcnViewEdit_Impl*       pEdit;
    WinBits                 nWinBits;
    long                    nMaxBoundHeight;            // height of highest BoundRects
    sal_uInt16              nFlags;
    sal_uInt16              nCurTextDrawFlags;
    sal_uLong               nUserEventAdjustScrBars;
    sal_uLong               nUserEventShowCursor;
    SvxIconChoiceCtrlEntry* pCurHighlightFrame;
    sal_Bool                bHighlightFramePressed;
    SvxIconChoiceCtrlEntry* pHead;                      // top left entry
    SvxIconChoiceCtrlEntry* pCursor;
    SvxIconChoiceCtrlEntry* pPrevDropTarget;
    SvxIconChoiceCtrlEntry* pHdlEntry;
    SvxIconChoiceCtrlEntry* pDDRefEntry;
    VirtualDevice*          pDDDev;
    VirtualDevice*          pDDBufDev;
    VirtualDevice*          pDDTempDev;
    VirtualDevice*          pEntryPaintDev;
    SvxIconChoiceCtrlEntry* pAnchor;                    // for selection
    LocalFocus              aFocus;                             // Data for focusrect
    ::svt::AccessibleFactoryAccess aAccFactory;

    SvxIconChoiceCtrlEntry* pCurEditedEntry;
    SvxIconChoiceCtrlTextMode eTextMode;
    SelectionMode           eSelectionMode;
    sal_uLong               nSelectionCount;
    SvxIconChoiceCtrlPositionMode ePositionMode;
    sal_Bool                bBoundRectsDirty;
    sal_Bool                bUpdateMode;
    sal_Bool                bEntryEditingEnabled;

    void                ShowCursor( sal_Bool bShow );

    void                ImpArrange( sal_Bool bKeepPredecessors = sal_False );
    void                AdjustVirtSize( const Rectangle& );
    void                ResetVirtSize();
    void                CheckScrollBars();

                        DECL_LINK( ScrollUpDownHdl, ScrollBar * );
                        DECL_LINK( ScrollLeftRightHdl, ScrollBar * );
                        DECL_LINK(EditTimeoutHdl, void *);
                        DECL_LINK( UserEventHdl, void* );
                        DECL_LINK( EndScrollHdl, void* );
                        DECL_LINK( AutoArrangeHdl, void* );
                        DECL_LINK( DocRectChangedHdl, void* );
                        DECL_LINK( VisRectChangedHdl, void* );
                        DECL_LINK( CallSelectHdlHdl, void* );

    void                AdjustScrollBars( sal_Bool bVirtSizeGrowedOnly = sal_False);
    void                PositionScrollBars( long nRealWidth, long nRealHeight );
    long                GetScrollBarPageSize( long nVisibleRange ) const
                        {
                            return ((nVisibleRange*75)/100);
                        }
    long                GetScrollBarLineSize() const
                        {
                            return nMaxBoundHeight / 2;
                        }
    sal_Bool            HandleScrollCommand( const CommandEvent& rCmd );
    void                ToDocPos( Point& rPosPixel )
                        {
                            rPosPixel -= pView->GetMapMode().GetOrigin();
                        }
    void                InitScrollBarBox();
    void                ToggleSelection( SvxIconChoiceCtrlEntry* );
    void                DeselectAllBut( SvxIconChoiceCtrlEntry*, sal_Bool bPaintSync=sal_False );
    void                Center( SvxIconChoiceCtrlEntry* pEntry ) const;
    void                StopEditTimer() { aEditTimer.Stop(); }
    void                StartEditTimer() { aEditTimer.Start(); }
    void                ImpHideDDIcon();
    void                CallSelectHandler( SvxIconChoiceCtrlEntry* );
    void                SelectRect(
                            SvxIconChoiceCtrlEntry* pEntry1,
                            SvxIconChoiceCtrlEntry* pEntry2,
                            sal_Bool bAdd = sal_True,
                            std::vector<Rectangle*>* pOtherRects = 0
                        );

    void                SelectRange(
                            SvxIconChoiceCtrlEntry* pStart,
                            SvxIconChoiceCtrlEntry* pEnd,
                            sal_Bool bAdd = sal_True
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
    void                AdjustAtGrid( const SvxIconChoiceCtrlEntryPtrVec& rRow, SvxIconChoiceCtrlEntry* pStart=0 );
    Point               AdjustAtGrid(
                            const Rectangle& rCenterRect, // balance point of object (typically Bmp-Rect)
                            const Rectangle& rBoundRect
                        ) const;
    sal_uLong           GetPredecessorGrid( const Point& rDocPos) const;

    void                InitPredecessors();
    void                ClearPredecessors();

    sal_Bool            CheckVerScrollBar();
    sal_Bool            CheckHorScrollBar();
    void                CancelUserEvents();
    void                EntrySelected(
                            SvxIconChoiceCtrlEntry* pEntry,
                            sal_Bool bSelect,
                            sal_Bool bSyncPaint
                        );
    void                RepaintEntries( sal_uInt16 nEntryFlagsMask );
    void                SetListPositions();
    void                SetDefaultTextSize();
    sal_Bool            IsAutoArrange() const
                        {
                            return (sal_Bool)(ePositionMode == IcnViewPositionModeAutoArrange);
                        }
    sal_Bool            IsAutoAdjust() const
                        {
                            return (sal_Bool)(ePositionMode == IcnViewPositionModeAutoAdjust);
                        }
    void                DocRectChanged() { aDocRectChangedTimer.Start(); }
    void                VisRectChanged() { aVisRectChangedTimer.Start(); }
    void                SetOrigin( const Point&, sal_Bool bDoNotUpdateWallpaper = sal_False );

                        DECL_LINK(TextEditEndedHdl, void *);

    void                ShowFocus ( Rectangle& rRect );
    void                DrawFocusRect ( OutputDevice* pOut );

    sal_Bool            IsMnemonicChar( sal_Unicode cChar, sal_uLong& rPos ) const;

public:

    long                nGridDX;
    long                nGridDY;
    long                nHorSBarHeight;
    long                nVerSBarWidth;

                        SvxIconChoiceCtrl_Impl( SvtIconChoiceCtrl* pView, WinBits nWinStyle );
                        ~SvxIconChoiceCtrl_Impl();

    sal_Bool            SetChoiceWithCursor ( sal_Bool bDo = sal_True ) { sal_Bool bOld=bChooseWithCursor; bChooseWithCursor = bDo; return bOld; }
    void                Clear( sal_Bool bInCtor = sal_False );
    void                SetStyle( WinBits nWinStyle );
    WinBits             GetStyle() const { return nWinBits; }
    void                InsertEntry( SvxIconChoiceCtrlEntry*, size_t nPos, const Point* pPos=0 );
    void                CreateAutoMnemonics( MnemonicGenerator* _pGenerator = NULL );
    void                FontModified();
    void                SelectAll( sal_Bool bSelect = sal_True, sal_Bool bPaint = sal_True );
    void                SelectEntry(
                            SvxIconChoiceCtrlEntry*,
                            sal_Bool bSelect,
                            sal_Bool bCallHdl = sal_True,
                            sal_Bool bAddToSelection = sal_False,
                            sal_Bool bSyncPaint = sal_False
                        );
    void                Paint( const Rectangle& rRect );
    sal_Bool            MouseButtonDown( const MouseEvent& );
    sal_Bool            MouseButtonUp( const MouseEvent& );
    sal_Bool            MouseMove( const MouseEvent&);
    sal_Bool            RequestHelp( const HelpEvent& rHEvt );
    void                SetCursor_Impl(
                            SvxIconChoiceCtrlEntry* pOldCursor,
                            SvxIconChoiceCtrlEntry* pNewCursor,
                            sal_Bool bMod1,
                            sal_Bool bShift,
                            sal_Bool bPaintSync = sal_False
                        );
    sal_Bool            KeyInput( const KeyEvent& );
    void                Resize();
    void                GetFocus();
    void                LoseFocus();
    void                SetUpdateMode( sal_Bool bUpdate );
    sal_Bool            GetUpdateMode() const { return bUpdateMode; }
    void                PaintEntry(
                            SvxIconChoiceCtrlEntry* pEntry,
                            sal_Bool bIsBackgroundPainted=sal_False
                        );
    void                PaintEntry(
                            SvxIconChoiceCtrlEntry*,
                            const Point&,
                            OutputDevice* pOut = 0,
                            sal_Bool bIsBackgroundPainted = sal_False
                        );
    void                PaintEntryVirtOutDev( SvxIconChoiceCtrlEntry* );

    void                SetEntryPos(
                            SvxIconChoiceCtrlEntry* pEntry,
                            const Point& rPos,
                            sal_Bool bAdjustRow = sal_False,
                            sal_Bool bCheckScrollBars = sal_False,
                            sal_Bool bKeepGridMap = sal_False
                        );

    void                InvalidateEntry( SvxIconChoiceCtrlEntry* );

    void                SetNoSelection();

    SvxIconChoiceCtrlEntry* GetCurEntry() const { return pCursor; }
    void                SetCursor(
                            SvxIconChoiceCtrlEntry*,
                            // sal_True == carry selection when single-selecting
                            sal_Bool bSyncSingleSelection = sal_True,
                            sal_Bool bShowFocusAsync = sal_False
                        );

    SvxIconChoiceCtrlEntry* GetEntry( const Point& rDocPos, sal_Bool bHit = sal_False );

    Point               GetEntryPos( SvxIconChoiceCtrlEntry* );
    void                MakeEntryVisible( SvxIconChoiceCtrlEntry* pEntry, sal_Bool bBound = sal_True );

    void                Arrange(
                            sal_Bool bKeepPredecessors = sal_False,
                            long nSetMaxVirtWidth =0,
                            long nSetMaxVirtHeight =0
                        );

    Rectangle           CalcFocusRect( SvxIconChoiceCtrlEntry* );
    Rectangle           CalcBmpRect( SvxIconChoiceCtrlEntry*, const Point* pPos = 0 );
    Rectangle           CalcTextRect(
                            SvxIconChoiceCtrlEntry*,
                            const Point* pPos = 0,
                            sal_Bool bForInplaceEdit = sal_False,
                            const String* pStr = 0
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
                            bBoundRectsDirty = sal_True;
                        }
    sal_Bool            IsBoundingRectValid( const Rectangle& rRect ) const { return (sal_Bool)( rRect.Right() != LONG_MAX ); }

    void                PaintEmphasis(
                            const Rectangle& rRect1,
                            const Rectangle& rRect2,
                            sal_Bool bSelected,
                            sal_Bool bDropTarget,
                            sal_Bool bCursored,
                            OutputDevice* pOut,
                            sal_Bool bIsBackgroundPainted = sal_False
                        );

    void                PaintItem(
                            const Rectangle& rRect,
                            IcnViewFieldType eItem,
                            SvxIconChoiceCtrlEntry* pEntry,
                            sal_uInt16 nPaintFlags,
                            OutputDevice* pOut,
                            const String* pStr = 0,
                            ::vcl::ControlLayoutData* _pLayoutData = NULL
                        );

    // recalculates all BoundingRects if bMustRecalcBoundingRects == sal_True
    void                CheckBoundingRects() { if (bBoundRectsDirty) RecalcAllBoundingRectsSmart(); }
    // recalculates all invalidated BoundingRects
    void                ShowTargetEmphasis( SvxIconChoiceCtrlEntry* pEntry, sal_Bool bShow );
    void                Command( const CommandEvent& rCEvt );
    void                ToTop( SvxIconChoiceCtrlEntry* );

    sal_uLong           GetSelectionCount() const;
    void                SetGrid( const Size& );
    Size                GetMinGrid() const;
    void                Scroll( long nDeltaX, long nDeltaY, sal_Bool bScrollBar = sal_False );
    const Size&         GetItemSize( SvxIconChoiceCtrlEntry*, IcnViewFieldType ) const;

    void                HideDDIcon();

    bool                IsOver(
                            std::vector<Rectangle*>* pSelectedRectList,
                            const Rectangle& rEntryBoundRect
                        ) const;

    void                SelectRect(
                            const Rectangle&,
                            bool bAdd = true,
                            std::vector<Rectangle*>* pOtherRects = 0
                        );

    sal_Bool            IsTextHit( SvxIconChoiceCtrlEntry* pEntry, const Point& rDocPos );
    void                MakeVisible(
                            const Rectangle& rDocPos,
                            sal_Bool bInScrollBarEvent=sal_False,
                            sal_Bool bCallRectChangedHdl = sal_True
                        );

    void                AdjustEntryAtGrid( SvxIconChoiceCtrlEntry* pStart = 0 );
    void                SetEntryTextMode(
                            SvxIconChoiceCtrlTextMode,
                            SvxIconChoiceCtrlEntry* pEntry = 0
                        );
    void                EnableEntryEditing( sal_Bool bEnable ) { bEntryEditingEnabled = bEnable; }
    sal_Bool            IsEntryEditingEnabled() const { return bEntryEditingEnabled; }
    sal_Bool            IsEntryEditing() const { return (sal_Bool)(pCurEditedEntry!=0); }
    void                EditEntry( SvxIconChoiceCtrlEntry* pEntry );
    void                StopEntryEditing( sal_Bool bCancel );
    size_t              GetEntryCount() const { return aEntries.size(); }
    SvxIconChoiceCtrlEntry* GetEntry( size_t nPos )
                            {
                                return aEntries[ nPos ];
                            }
    SvxIconChoiceCtrlEntry* GetEntry( size_t nPos ) const
                            {
                                return aEntries[ nPos ];
                            }
    SvxIconChoiceCtrlEntry* GetFirstSelectedEntry( sal_uLong& ) const;
    SvxIconChoiceCtrlEntry* GetHdlEntry() const { return pHdlEntry; }
    void                SetHdlEntry( SvxIconChoiceCtrlEntry* pEntry ) { pHdlEntry = pEntry; }

    SvxIconChoiceCtrlTextMode GetEntryTextModeSmart( const SvxIconChoiceCtrlEntry* pEntry ) const;
    void                SetSelectionMode( SelectionMode eMode ) { eSelectionMode=eMode; }
    SelectionMode       GetSelectionMode() const { return eSelectionMode; }
    sal_Bool            AreEntriesMoved() const { return (sal_Bool)((nFlags & F_MOVED_ENTRIES)!=0); }
    void                SetEntriesMoved( sal_Bool bMoved )
                        {
                            if( bMoved )
                                nFlags |= F_MOVED_ENTRIES;
                            else
                                nFlags &= ~(F_MOVED_ENTRIES);
                        }
    sal_uLong           GetEntryListPos( SvxIconChoiceCtrlEntry* ) const;
    void                SetEntryImageSize( const Size& rSize ) { aImageSize = rSize; }
    void                InitSettings();
    Rectangle           GetOutputRect() const;

    sal_Bool            ArePredecessorsSet() const { return (sal_Bool)(pHead != 0); }
    SvxIconChoiceCtrlEntry* GetPredecessorHead() const { return pHead; }
    void                SetEntryPredecessor(SvxIconChoiceCtrlEntry* pEntry,SvxIconChoiceCtrlEntry* pPredecessor);
    // only delivers valid results when in AutoArrange mode!
    SvxIconChoiceCtrlEntry* FindEntryPredecessor( SvxIconChoiceCtrlEntry* pEntry, const Point& );

    void                SetPositionMode( SvxIconChoiceCtrlPositionMode );
    SvxIconChoiceCtrlPositionMode GetPositionMode() const { return ePositionMode;}

    void                SetColumn( sal_uInt16 nIndex, const SvxIconChoiceCtrlColumnInfo& );
    const SvxIconChoiceCtrlColumnInfo* GetColumn( sal_uInt16 nIndex ) const;

    Rectangle           GetDocumentRect() const { return Rectangle( Point(), aVirtOutputSize ); }
    Rectangle           GetVisibleRect() const { return GetOutputRect(); }

    void                SetEntryHighlightFrame(
                            SvxIconChoiceCtrlEntry* pEntry,
                            sal_Bool bKeepHighlightFlags=sal_False
                        );
    void                HideEntryHighlightFrame();
    void                DrawHighlightFrame(
                            OutputDevice* pOut,
                            const Rectangle& rBmpRect,
                            sal_Bool bHide
                        );
    void                StopSelectTimer() { aCallSelectHdlTimer.Stop(); }

    void                CallEventListeners( sal_uLong nEvent, void* pData = NULL );

    inline ::svt::IAccessibleFactory& GetAccessibleFactory()
                        {
                            return aAccFactory.getFactory();
                        }
};

// ----------------------------------------------------------------------------------------------

typedef std::map<sal_uInt16, SvxIconChoiceCtrlEntryPtrVec> IconChoiceMap;

class IcnCursor_Impl
{
    SvxIconChoiceCtrl_Impl* pView;
    boost::scoped_ptr<IconChoiceMap> pColumns;
    boost::scoped_ptr<IconChoiceMap> pRows;
    long                    nCols;
    long                    nRows;
    short                   nDeltaWidth;
    short                   nDeltaHeight;
    SvxIconChoiceCtrlEntry* pCurEntry;
    void                    SetDeltas();
    void                    ImplCreate();
    void                    Create() {  if( !pColumns ) ImplCreate(); }

    sal_uInt16              GetSortListPos(
                                SvxIconChoiceCtrlEntryPtrVec& rList,
                                long nValue,
                                int bVertical);
    SvxIconChoiceCtrlEntry* SearchCol(
                                sal_uInt16 nCol,
                                sal_uInt16 nTop,
                                sal_uInt16 nBottom,
                                sal_uInt16 nPref,
                                bool bDown,
                                bool bSimple
                            );

    SvxIconChoiceCtrlEntry* SearchRow(
                                sal_uInt16 nRow,
                                sal_uInt16 nRight,
                                sal_uInt16 nLeft,
                                sal_uInt16 nPref,
                                bool bRight,
                                bool bSimple
                            );

public:
                            IcnCursor_Impl( SvxIconChoiceCtrl_Impl* pOwner );
                            ~IcnCursor_Impl();
    void                    Clear();

    // for Cursortravelling etc.
    SvxIconChoiceCtrlEntry* GoLeftRight( SvxIconChoiceCtrlEntry*, sal_Bool bRight );
    SvxIconChoiceCtrlEntry* GoUpDown( SvxIconChoiceCtrlEntry*, sal_Bool bDown );
    SvxIconChoiceCtrlEntry* GoPageUpDown( SvxIconChoiceCtrlEntry*, sal_Bool bDown );

    // Creates a list of entries for every row (height = nGridDY) sorted by
    // BoundRect.Left(). A list may be empty. The lists become the property of
    // the caller and have to be deleted with DestroyGridAdjustData.
    void                    CreateGridAjustData( IconChoiceMap& pLists, SvxIconChoiceCtrlEntry* pRow=0);
    static void             DestroyGridAdjustData( IconChoiceMap& rLists );
};

// ----------------------------------------------------------------------------------------------

typedef sal_uLong GridId;

#define GRID_NOT_FOUND  ((GridId)ULONG_MAX)

class IcnGridMap_Impl
{
    Rectangle               _aLastOccupiedGrid;
    SvxIconChoiceCtrl_Impl* _pView;
    sal_Bool*               _pGridMap;
    sal_uInt16              _nGridCols, _nGridRows;

    void                Expand();
    void                Create_Impl();
    void                Create() { if(!_pGridMap) Create_Impl(); }

    void                GetMinMapSize( sal_uInt16& rDX, sal_uInt16& rDY ) const;

public:
                        IcnGridMap_Impl(SvxIconChoiceCtrl_Impl* pView);
                        ~IcnGridMap_Impl();

    void                Clear();

    GridId              GetGrid( const Point& rDocPos, sal_Bool* pbClipped = 0 );
    GridId              GetGrid( sal_uInt16 nGridX, sal_uInt16 nGridY );
    GridId              GetUnoccupiedGrid( sal_Bool bOccupyFound=sal_True );

    void                OccupyGrids( const SvxIconChoiceCtrlEntry*, sal_Bool bOccupy = sal_True );
    void                OccupyGrid( GridId nId, sal_Bool bOccupy = sal_True )
                        {
                            DBG_ASSERT(!_pGridMap || nId<(sal_uLong)(_nGridCols*_nGridRows),"OccupyGrid: Bad GridId");
                            if(_pGridMap && nId < (sal_uLong)(_nGridCols *_nGridRows) )
                                _pGridMap[ nId ] = bOccupy;
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
