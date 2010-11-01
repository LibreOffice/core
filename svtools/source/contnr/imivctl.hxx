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

#ifndef _IMPICNVW_HXX
#define _IMPICNVW_HXX

#include <vcl/virdev.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/timer.hxx>
#include <vcl/seleng.hxx>
#include <tools/debug.hxx>
#include "svtaccessiblefactory.hxx"

#include <limits.h>

#include "ivctrl.hxx"
#include <svl/svarray.hxx>

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
#define F_PAINTED                       0x0004  // TRUE nach erstem Paint
#define F_ADD_MODE                      0x0008
#define F_SELECTING_RECT                0x0020
#define F_DOWN_CTRL                     0x0080
#define F_DOWN_DESELECT                 0x0100
#define F_START_EDITTIMER_IN_MOUSEUP    0x0400
#define F_MOVED_ENTRIES                 0x0800
#define F_ENTRYLISTPOS_VALID            0x1000
#define F_CLEARING_SELECTION            0x2000
#define F_ARRANGING                     0x4000

// alle Angaben in Pixel
// Abstaende von Fensterraendern
#define LROFFS_WINBORDER            4
#define TBOFFS_WINBORDER            4
// fuer das Bounding-Rectangle
#define LROFFS_BOUND                2
#define TBOFFS_BOUND                2
// Abstand Fokusrechteck - Icon
#define LROFFS_ICON                 2
#define TBOFFS_ICON                 2
// Abstaende Icon - Text
#define HOR_DIST_BMP_STRING         3
#define VER_DIST_BMP_STRING         3
// Breitenoffset Highlight-Rect bei Text
#define LROFFS_TEXT                 2

#define DEFAULT_MAX_VIRT_WIDTH  200
#define DEFAULT_MAX_VIRT_HEIGHT 200

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
    BOOL        bOn;
    Rectangle   aRect;
    Color       aPenColor;

    LocalFocus() { bOn = FALSE; }
};

///////////////////////////////////////////////////////////////////////////////
//
// Entry-List
//
class EntryList_Impl : public List
{
private:

    using List::Replace;

    SvxIconChoiceCtrl_Impl*         _pOwner;

    void                    Removed_Impl( SvxIconChoiceCtrlEntry* pEntry );

public:
                            EntryList_Impl(
                                SvxIconChoiceCtrl_Impl*,
                                USHORT _nInitSize = 1024,
                                USHORT _nReSize = 1024 );
                            EntryList_Impl(
                                SvxIconChoiceCtrl_Impl*,
                                USHORT _nBlockSize,
                                USHORT _nInitSize,
                                USHORT _nReSize );
                            ~EntryList_Impl();

    void                    Clear();
    void                    Insert( SvxIconChoiceCtrlEntry* pEntry, ULONG nPos );
    SvxIconChoiceCtrlEntry*         Remove( ULONG nPos );
    void                    Remove( SvxIconChoiceCtrlEntry* pEntry );
};


///////////////////////////////////////////////////////////////////////////////
//
// Implementation-class of IconChoiceCtrl
//
class SvxIconChoiceCtrl_Impl
{
    friend class IcnCursor_Impl;
    friend class EntryList_Impl;
    friend class IcnGridMap_Impl;

    BOOL            bChooseWithCursor;
    EntryList_Impl  aEntries;
    ScrollBar       aVerSBar;
    ScrollBar       aHorSBar;
    ScrollBarBox    aScrBarBox;
    Rectangle       aCurSelectionRect;
    SvPtrarr        aSelectedRectList;
    Timer           aEditTimer;                 // fuer Inplace-Editieren
    Timer           aAutoArrangeTimer;
    Timer           aDocRectChangedTimer;
    Timer           aVisRectChangedTimer;
    Timer           aCallSelectHdlTimer;
    Size            aVirtOutputSize;
    Size            aImageSize;
    Size            aDefaultTextSize;
    Size            aOutputSize; // Pixel
    Point           aDDLastEntryPos;
    Point           aDDLastRectPos;
    Point           aDDPaintOffs;
    Point           aDDStartPos;
    SvtIconChoiceCtrl*      pView;
    IcnCursor_Impl* pImpCursor;
    IcnGridMap_Impl* pGridMap;
    long            nMaxVirtWidth;  // max. Breite aVirtOutputSize bei ALIGN_TOP
    long            nMaxVirtHeight; // max. Hoehe aVirtOutputSize bei ALIGN_LEFT
    List*           pZOrderList;
    SvPtrarr*       pColumns;
    IcnViewEdit_Impl* pEdit;
    WinBits         nWinBits;
    long            nMaxBoundHeight;            // Hoehe des hoechsten BoundRects
    USHORT          nFlags;
    USHORT          nCurTextDrawFlags;
    ULONG           nUserEventAdjustScrBars;
    ULONG           nUserEventShowCursor;
    SvxIconChoiceCtrlEntry* pCurHighlightFrame;
    BOOL            bHighlightFramePressed;
    SvxIconChoiceCtrlEntry* pHead;                      // Eintrag oben links
    SvxIconChoiceCtrlEntry* pCursor;
    SvxIconChoiceCtrlEntry* pPrevDropTarget;
    SvxIconChoiceCtrlEntry* pHdlEntry;
    SvxIconChoiceCtrlEntry* pDDRefEntry;
    VirtualDevice*  pDDDev;
    VirtualDevice*  pDDBufDev;
    VirtualDevice*  pDDTempDev;
    VirtualDevice*  pEntryPaintDev;
    SvxIconChoiceCtrlEntry* pAnchor;                    // fuer Selektion
    LocalFocus      aFocus;                             // Data for focusrect
    ::svt::AccessibleFactoryAccess aAccFactory;

    List*           pDraggedSelection;
    SvxIconChoiceCtrlEntry* pCurEditedEntry;
    SvxIconChoiceCtrlTextMode eTextMode;
    SelectionMode   eSelectionMode;
    ULONG           nSelectionCount;
    SvxIconChoiceCtrlPositionMode ePositionMode;
    BOOL            bBoundRectsDirty;
    BOOL            bUpdateMode;
    BOOL            bEntryEditingEnabled;
    BOOL            bInDragDrop;

    void            ShowCursor( BOOL bShow );

    void            ImpArrange( BOOL bKeepPredecessors = FALSE );
    void            AdjustVirtSize( const Rectangle& );
    void            ResetVirtSize();
    void            CheckScrollBars();

                    DECL_LINK( ScrollUpDownHdl, ScrollBar * );
                    DECL_LINK( ScrollLeftRightHdl, ScrollBar * );
                    DECL_LINK( EditTimeoutHdl, Timer* );
                    DECL_LINK( UserEventHdl, void* );
                    DECL_LINK( EndScrollHdl, void* );
                    DECL_LINK( AutoArrangeHdl, void* );
                    DECL_LINK( DocRectChangedHdl, void* );
                    DECL_LINK( VisRectChangedHdl, void* );
                    DECL_LINK( CallSelectHdlHdl, void* );

    void            AdjustScrollBars( BOOL bVirtSizeGrowedOnly = FALSE);
    void            PositionScrollBars( long nRealWidth, long nRealHeight );
    long            GetScrollBarPageSize( long nVisibleRange ) const { return ((nVisibleRange*75)/100); }
    long            GetScrollBarLineSize() const { return nMaxBoundHeight / 2; }
    BOOL            HandleScrollCommand( const CommandEvent& rCmd );
    void            ToDocPos( Point& rPosPixel ) { rPosPixel -= pView->GetMapMode().GetOrigin(); }
    void            InitScrollBarBox();
    SvxIconChoiceCtrlEntry* FindNewCursor();
    void            ToggleSelection( SvxIconChoiceCtrlEntry* );
    void            DeselectAllBut( SvxIconChoiceCtrlEntry*, BOOL bPaintSync=FALSE );
    void            Center( SvxIconChoiceCtrlEntry* pEntry ) const;
    void            StopEditTimer() { aEditTimer.Stop(); }
    void            StartEditTimer() { aEditTimer.Start(); }
    void            ImpHideDDIcon();
    void            CallSelectHandler( SvxIconChoiceCtrlEntry* );
    void            SelectRect(
                        SvxIconChoiceCtrlEntry* pEntry1,
                        SvxIconChoiceCtrlEntry* pEntry2,
                        BOOL bAdd = TRUE,
                        SvPtrarr* pOtherRects = 0 );

    void            SelectRange(
                        SvxIconChoiceCtrlEntry* pStart,
                        SvxIconChoiceCtrlEntry* pEnd,
                        BOOL bAdd = TRUE );

    void            AddSelectedRect( const Rectangle& );
    void            AddSelectedRect(
                        SvxIconChoiceCtrlEntry* pEntry1,
                        SvxIconChoiceCtrlEntry* pEntry2 );

    void            ClearSelectedRectList();
    void            ClearColumnList();
    Rectangle       CalcMaxTextRect( const SvxIconChoiceCtrlEntry* pEntry ) const;

    void            ClipAtVirtOutRect( Rectangle& rRect ) const;
    void            AdjustAtGrid( const SvPtrarr& rRow, SvxIconChoiceCtrlEntry* pStart=0 );
    Point           AdjustAtGrid(
                        const Rectangle& rCenterRect, // "Schwerpunkt" des Objekts (typ. Bmp-Rect)
                        const Rectangle& rBoundRect ) const;
    ULONG           GetPredecessorGrid( const Point& rDocPos) const;

    void            InitPredecessors();
    void            ClearPredecessors();

    BOOL            CheckVerScrollBar();
    BOOL            CheckHorScrollBar();
    void            CancelUserEvents();
    void            EntrySelected( SvxIconChoiceCtrlEntry* pEntry, BOOL bSelect,
                        BOOL bSyncPaint );
    void            SaveSelection( List** );
    void            RepaintEntries( USHORT nEntryFlagsMask );
    void            SetListPositions();
    void            SetDefaultTextSize();
    BOOL            IsAutoArrange() const {
                        return (BOOL)(ePositionMode == IcnViewPositionModeAutoArrange); }
    BOOL            IsAutoAdjust() const {
                        return (BOOL)(ePositionMode == IcnViewPositionModeAutoAdjust); }
    void            DocRectChanged() { aDocRectChangedTimer.Start(); }
    void            VisRectChanged() { aVisRectChangedTimer.Start(); }
    void            SetOrigin( const Point&, BOOL bDoNotUpdateWallpaper = FALSE );

                    DECL_LINK( TextEditEndedHdl, IcnViewEdit_Impl* );

    void ShowFocus ( Rectangle& rRect );
    void HideFocus ();
    void DrawFocusRect ( OutputDevice* pOut );

    BOOL            IsMnemonicChar( sal_Unicode cChar, ULONG& rPos ) const;

public:

    long            nGridDX,
                    nGridDY;
    long            nHorSBarHeight,
                    nVerSBarWidth;

                    SvxIconChoiceCtrl_Impl( SvtIconChoiceCtrl* pView, WinBits nWinStyle );
                    ~SvxIconChoiceCtrl_Impl();

    BOOL            SetChoiceWithCursor ( BOOL bDo = TRUE ) { BOOL bOld=bChooseWithCursor; bChooseWithCursor = bDo; return bOld; }
    void            Clear( BOOL bInCtor = FALSE );
    void            SetStyle( WinBits nWinStyle );
    WinBits         GetStyle() const { return nWinBits; }
    void            InsertEntry( SvxIconChoiceCtrlEntry*, ULONG nPos, const Point* pPos=0 );
    void            CreateAutoMnemonics( MnemonicGenerator* _pGenerator = NULL );
    void            RemoveEntry( SvxIconChoiceCtrlEntry* pEntry );
    void            FontModified();
    void            SelectAll( BOOL bSelect = TRUE, BOOL bPaint = TRUE );
    void            SelectEntry(
                        SvxIconChoiceCtrlEntry*,
                        BOOL bSelect,
                        BOOL bCallHdl = TRUE,
                        BOOL bAddToSelection = FALSE,
                        BOOL bSyncPaint = FALSE );
    void            Paint( const Rectangle& rRect );
    BOOL            MouseButtonDown( const MouseEvent& );
    BOOL            MouseButtonUp( const MouseEvent& );
    BOOL            MouseMove( const MouseEvent&);
    BOOL            RequestHelp( const HelpEvent& rHEvt );
    void            SetCursor_Impl(
                        SvxIconChoiceCtrlEntry* pOldCursor,
                        SvxIconChoiceCtrlEntry* pNewCursor,
                        BOOL bMod1,
                        BOOL bShift,
                        BOOL bPaintSync = FALSE);
    BOOL            KeyInput( const KeyEvent& );
    void            Resize();
    void            GetFocus();
    void            LoseFocus();
    void            SetUpdateMode( BOOL bUpdate );
    BOOL            GetUpdateMode() const { return bUpdateMode; }
    void            PaintEntry( SvxIconChoiceCtrlEntry* pEntry, BOOL bIsBackgroundPainted=FALSE );
    void            PaintEntry(
                        SvxIconChoiceCtrlEntry*,
                        const Point&,
                        OutputDevice* pOut = 0,
                        BOOL bIsBackgroundPainted = FALSE);
    void            PaintEntryVirtOutDev( SvxIconChoiceCtrlEntry* );

    void            SetEntryPos(
                        SvxIconChoiceCtrlEntry* pEntry,
                        const Point& rPos,
                        BOOL bAdjustRow = FALSE,
                        BOOL bCheckScrollBars = FALSE,
                        BOOL bKeepGridMap = FALSE );

    void            InvalidateEntry( SvxIconChoiceCtrlEntry* );
    IcnViewFieldType    GetItem( SvxIconChoiceCtrlEntry*, const Point& rAbsPos );

    void            SetNoSelection();

    SvxIconChoiceCtrlEntry* GetCurEntry() const { return pCursor; }
    void            SetCursor(
                        SvxIconChoiceCtrlEntry*,
                        // TRUE == bei Single-Selection die Sel. mitfuehren
                        BOOL bSyncSingleSelection = TRUE,
                        BOOL bShowFocusAsync = FALSE );

    SvxIconChoiceCtrlEntry* GetEntry( const Point& rDocPos, BOOL bHit = FALSE );
    SvxIconChoiceCtrlEntry* GetNextEntry( const Point& rDocPos, SvxIconChoiceCtrlEntry* pCurEntry );
    SvxIconChoiceCtrlEntry* GetPrevEntry( const Point& rDocPos, SvxIconChoiceCtrlEntry* pCurEntry  );

    Point           GetEntryPos( SvxIconChoiceCtrlEntry* );
    void            MakeEntryVisible( SvxIconChoiceCtrlEntry* pEntry, BOOL bBound = TRUE );

    void            Arrange(BOOL bKeepPredecessors = FALSE, long nSetMaxVirtWidth =0, long nSetMaxVirtHeight =0 );

    Rectangle       CalcFocusRect( SvxIconChoiceCtrlEntry* );
    Rectangle       CalcBmpRect( SvxIconChoiceCtrlEntry*, const Point* pPos = 0 );
    Rectangle       CalcTextRect(
                        SvxIconChoiceCtrlEntry*,
                        const Point* pPos = 0,
                        BOOL bForInplaceEdit = FALSE,
                        const String* pStr = 0 );

    long            CalcBoundingWidth( SvxIconChoiceCtrlEntry* ) const;
    long            CalcBoundingHeight( SvxIconChoiceCtrlEntry* ) const;
    Size            CalcBoundingSize( SvxIconChoiceCtrlEntry* ) const;
    void            FindBoundingRect( SvxIconChoiceCtrlEntry* pEntry );
    void            SetBoundingRect_Impl(
                        SvxIconChoiceCtrlEntry* pEntry,
                        const Point& rPos,
                        const Size& rBoundingSize );
    // berechnet alle BoundRects neu
    void            RecalcAllBoundingRects();
    // berechnet alle ungueltigen BoundRects neu
    void            RecalcAllBoundingRectsSmart();
    const Rectangle&  GetEntryBoundRect( SvxIconChoiceCtrlEntry* );
    void            InvalidateBoundingRect( SvxIconChoiceCtrlEntry* );
    void            InvalidateBoundingRect( Rectangle& rRect ) { rRect.Right() = LONG_MAX; bBoundRectsDirty = TRUE; }
    BOOL            IsBoundingRectValid( const Rectangle& rRect ) const { return (BOOL)( rRect.Right() != LONG_MAX ); }

    void            PaintEmphasis(
                        const Rectangle& rRect1,
                        const Rectangle& rRect2,
                        BOOL bSelected,
                        BOOL bDropTarget,
                        BOOL bCursored,
                        OutputDevice* pOut,
                        BOOL bIsBackgroundPainted = FALSE);

    void            PaintItem(
                        const Rectangle& rRect,
                        IcnViewFieldType eItem,
                        SvxIconChoiceCtrlEntry* pEntry,
                        USHORT nPaintFlags,
                        OutputDevice* pOut,
                        const String* pStr = 0,
                        ::vcl::ControlLayoutData* _pLayoutData = NULL );

    // berechnet alle BoundingRects neu, wenn bMustRecalcBoundingRects == TRUE
    void            CheckBoundingRects() { if (bBoundRectsDirty) RecalcAllBoundingRectsSmart(); }
    // berechnet alle invalidierten BoundingRects neu
    void            UpdateBoundingRects();
    void            ShowTargetEmphasis( SvxIconChoiceCtrlEntry* pEntry, BOOL bShow );
    void            PrepareCommandEvent( const CommandEvent& );
    void            Command( const CommandEvent& rCEvt );
    void            ToTop( SvxIconChoiceCtrlEntry* );

    ULONG           GetSelectionCount() const;
    void            SetGrid( const Size& );
    Size            GetMinGrid() const;
    ULONG           GetGridCount(
                        const Size& rSize,
                        BOOL bCheckScrBars,
                        BOOL bSmartScrBar ) const;
    void            Scroll( long nDeltaX, long nDeltaY, BOOL bScrollBar = FALSE );
    const Size&     GetItemSize( SvxIconChoiceCtrlEntry*, IcnViewFieldType ) const;

    void            HideDDIcon();
    void            ShowDDIcon( SvxIconChoiceCtrlEntry* pRefEntry, const Point& rPos );
    void            HideShowDDIcon(
                        SvxIconChoiceCtrlEntry* pRefEntry,
                        const Point& rPos );

    BOOL            IsOver(
                        SvPtrarr* pSelectedRectList,
                        const Rectangle& rEntryBoundRect ) const;

    void            SelectRect(
                        const Rectangle&,
                        BOOL bAdd = TRUE,
                        SvPtrarr* pOtherRects = 0 );

    void            CalcScrollOffsets(
                        const Point& rRefPosPixel,
                        long& rX,
                        long& rY,
                        BOOL bDragDrop = FALSE,
                        USHORT nBorderWidth = 10 );

    BOOL            IsTextHit( SvxIconChoiceCtrlEntry* pEntry, const Point& rDocPos );
    void            MakeVisible(
                        const Rectangle& rDocPos,
                        BOOL bInScrollBarEvent=FALSE,
                        BOOL bCallRectChangedHdl = TRUE );

    void            AdjustEntryAtGrid( SvxIconChoiceCtrlEntry* pStart = 0 );
    void            SetEntryTextMode( SvxIconChoiceCtrlTextMode, SvxIconChoiceCtrlEntry* pEntry = 0 );
    SvxIconChoiceCtrlTextMode GetTextMode( const SvxIconChoiceCtrlEntry* pEntry = 0 ) const;
    void            ShowEntryFocusRect( const SvxIconChoiceCtrlEntry* pEntry );
    void            EnableEntryEditing( BOOL bEnable ) { bEntryEditingEnabled = bEnable; }
    BOOL            IsEntryEditingEnabled() const { return bEntryEditingEnabled; }
    BOOL            IsEntryEditing() const { return (BOOL)(pCurEditedEntry!=0); }
    void            EditEntry( SvxIconChoiceCtrlEntry* pEntry );
    void            StopEntryEditing( BOOL bCancel );
    void            LockEntryPos( SvxIconChoiceCtrlEntry* pEntry, BOOL bLock );
    ULONG           GetEntryCount() const { return aEntries.Count(); }
    SvxIconChoiceCtrlEntry* GetEntry( ULONG nPos ) const { return (SvxIconChoiceCtrlEntry*)aEntries.GetObject(nPos); }
    SvxIconChoiceCtrlEntry* GetFirstSelectedEntry( ULONG& ) const;
    SvxIconChoiceCtrlEntry* GetNextSelectedEntry( ULONG& ) const;
    SvxIconChoiceCtrlEntry* GetHdlEntry() const { return pHdlEntry; }
    void            SetHdlEntry( SvxIconChoiceCtrlEntry* pEntry ) { pHdlEntry = pEntry; }

    SvxIconChoiceCtrlTextMode GetEntryTextModeSmart( const SvxIconChoiceCtrlEntry* pEntry ) const;
    void            SetSelectionMode( SelectionMode eMode ) { eSelectionMode=eMode; }
    SelectionMode   GetSelectionMode() const { return eSelectionMode; }
    BOOL            AreEntriesMoved() const { return (BOOL)((nFlags & F_MOVED_ENTRIES)!=0); }
    void            SetEntriesMoved( BOOL bMoved )
                    {
                        if( bMoved ) nFlags |= F_MOVED_ENTRIES;
                        else nFlags &= ~(F_MOVED_ENTRIES);
                    }
    ULONG           GetEntryListPos( SvxIconChoiceCtrlEntry* ) const;
    void            SetEntryListPos( SvxIconChoiceCtrlEntry* pEntry, ULONG nNewPos );
    void            SetEntryImageSize( const Size& rSize ) { aImageSize = rSize; }
    void            SetEntryFlags( SvxIconChoiceCtrlEntry* pEntry, USHORT nFlags );
    SvxIconChoiceCtrlEntry* GoLeftRight( SvxIconChoiceCtrlEntry*, BOOL bRight );
    SvxIconChoiceCtrlEntry* GoUpDown( SvxIconChoiceCtrlEntry*, BOOL bDown );
    void            InitSettings();
    Rectangle       GetOutputRect() const;

    BOOL            ArePredecessorsSet() const { return (BOOL)(pHead != 0); }
    SvxIconChoiceCtrlEntry* GetPredecessorHead() const { return pHead; }
    void            SetEntryPredecessor(SvxIconChoiceCtrlEntry* pEntry,SvxIconChoiceCtrlEntry* pPredecessor);
    BOOL            GetEntryPredecessor(SvxIconChoiceCtrlEntry* pEntry,SvxIconChoiceCtrlEntry** ppPredecessor);
    // liefert gueltige Ergebnisse nur im AutoArrange-Modus!
    SvxIconChoiceCtrlEntry* FindEntryPredecessor( SvxIconChoiceCtrlEntry* pEntry, const Point& );

    void            SetPositionMode( SvxIconChoiceCtrlPositionMode );
    SvxIconChoiceCtrlPositionMode GetPositionMode() const { return ePositionMode;}

    void            Flush();
    void            SetColumn( USHORT nIndex, const SvxIconChoiceCtrlColumnInfo& );
    const SvxIconChoiceCtrlColumnInfo* GetColumn( USHORT nIndex ) const;
    const SvxIconChoiceCtrlColumnInfo* GetItemColumn( USHORT nSubItem, long& rLeft ) const;

    Rectangle       GetDocumentRect() const { return Rectangle( Point(), aVirtOutputSize ); }
    Rectangle       GetVisibleRect() const { return GetOutputRect(); }

    void            SetEntryHighlightFrame( SvxIconChoiceCtrlEntry* pEntry,BOOL bKeepHighlightFlags=FALSE );
    void            HideEntryHighlightFrame();
    void            DrawHighlightFrame( OutputDevice* pOut,
                        const Rectangle& rBmpRect, BOOL bHide );
    void            StopSelectTimer() { aCallSelectHdlTimer.Stop(); }
    void            Tracking( const TrackingEvent& rTEvt );
    Point           GetPopupMenuPosPixel() const;

    BOOL            HandleShortCutKey( const KeyEvent& rKeyEvent );

    void            CallEventListeners( ULONG nEvent, void* pData = NULL );

    inline ::svt::IAccessibleFactory&
        GetAccessibleFactory() { return aAccFactory.getFactory(); }
};

// ----------------------------------------------------------------------------------------------

class IcnCursor_Impl
{
    SvxIconChoiceCtrl_Impl* pView;
    SvPtrarr*       pColumns;
    SvPtrarr*       pRows;
    long            nCols;
    long            nRows;
    short           nDeltaWidth;
    short           nDeltaHeight;
    SvxIconChoiceCtrlEntry* pCurEntry;
    void            SetDeltas();
    void            ImplCreate();
    void            Create() {  if( !pColumns ) ImplCreate(); }

    USHORT          GetSortListPos( SvPtrarr* pList, long nValue, int bVertical);
    SvxIconChoiceCtrlEntry* SearchCol(USHORT nCol,USHORT nTop,USHORT nBottom,USHORT nPref,
                        BOOL bDown, BOOL bSimple );

    SvxIconChoiceCtrlEntry* SearchRow(USHORT nRow,USHORT nRight,USHORT nLeft,USHORT nPref,
                        BOOL bRight, BOOL bSimple );

public:
                    IcnCursor_Impl( SvxIconChoiceCtrl_Impl* pOwner );
                    ~IcnCursor_Impl();
    void            Clear();

    // fuer Cursortravelling usw.
    SvxIconChoiceCtrlEntry* GoLeftRight( SvxIconChoiceCtrlEntry*, BOOL bRight );
    SvxIconChoiceCtrlEntry* GoUpDown( SvxIconChoiceCtrlEntry*, BOOL bDown );
    SvxIconChoiceCtrlEntry* GoPageUpDown( SvxIconChoiceCtrlEntry*, BOOL bDown );

    // Erzeugt fuer jede Zeile (Hoehe=nGridDY) eine nach BoundRect.Left()
    // sortierte Liste der Eintraege, die in ihr stehen. Eine Liste kann
    // leer sein. Die Listen gehen in das Eigentum des Rufenden ueber und
    // muessen mit DestroyGridAdjustData geloescht werden
    void            CreateGridAjustData( SvPtrarr& pLists, SvxIconChoiceCtrlEntry* pRow=0);
    static void     DestroyGridAdjustData( SvPtrarr& rLists );
};

// ----------------------------------------------------------------------------------------------

typedef ULONG GridId;

#define GRID_NOT_FOUND  ((GridId)ULONG_MAX)

class IcnGridMap_Impl
{
    Rectangle       _aLastOccupiedGrid;
    SvxIconChoiceCtrl_Impl* _pView;
    BOOL*           _pGridMap;
    USHORT          _nGridCols, _nGridRows;

    void            Expand();
    void            Create_Impl();
    void            Create() { if(!_pGridMap) Create_Impl(); }

    void            GetMinMapSize( USHORT& rDX, USHORT& rDY ) const;

public:
                    IcnGridMap_Impl(SvxIconChoiceCtrl_Impl* pView);
                    ~IcnGridMap_Impl();

    void            Clear();

    GridId          GetGrid( const Point& rDocPos, BOOL* pbClipped = 0 );
    GridId          GetGrid( USHORT nGridX, USHORT nGridY );
    GridId          GetUnoccupiedGrid( BOOL bOccupyFound=TRUE );

    void            OccupyGrids( const Rectangle&, BOOL bOccupy = TRUE );
    void            OccupyGrids( const SvxIconChoiceCtrlEntry*, BOOL bOccupy = TRUE );
    void            OccupyGrid( GridId nId, BOOL bOccupy = TRUE )
                    {
                        DBG_ASSERT(!_pGridMap || nId<(ULONG)(_nGridCols*_nGridRows),"OccupyGrid: Bad GridId");
                        if(_pGridMap && nId < (ULONG)(_nGridCols *_nGridRows) )
                            _pGridMap[ nId ] = bOccupy;
                    }

    Rectangle       GetGridRect( GridId );
    void            GetGridCoord( GridId, USHORT& rGridX, USHORT& rGridY );
    static ULONG    GetGridCount( const Size& rSizePixel, USHORT nGridWidth, USHORT nGridHeight );

    void            OutputSizeChanged();
};





#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
