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
#ifndef _SVIMPICN_HXX
#define _SVIMPICN_HXX

#include <vcl/virdev.hxx>

#include <vcl/scrbar.hxx>
#include <limits.h>

namespace binfilter
{

class SvPtrarr;
class SvLBoxEntry;
class SvLBoxTreeList;
class SvImpIconView;
class ImpIcnCursor;

#define PAINTFLAG_HOR_CENTERED	0x0001
#define PAINTFLAG_VER_CENTERED	0x0002

#define SELRECT_BORDER_OFFS	-7
// Flags
#define F_VER_SBARSIZE_WITH_HBAR		0x00000001
#define F_HOR_SBARSIZE_WITH_VBAR		0x00000002
#define F_IGNORE_NEXT_MOUSEMOVE			0x00000004	// OS/2 only
#define F_ENTRY_REMOVED					0x00000008
// ist gesetzt, wenn nach Clear oder Ctor mind. einmal gepaintet wurde
#define F_PAINTED						0x00000010
#define F_ADD_MODE						0x00000020
#define F_MOVING_SIBLING				0x00000040
#define F_SELRECT_VISIBLE				0x00000080
#define F_CMD_ARRIVED					0x00000100
#define F_DRAG_SOURCE					0x00000200
#define F_GRIDMODE						0x00000400
// beim Einfuegen eines Eintrags ergibt sich dessen Position
// durch simples Addieren auf die Position des zuletzt eingefuegten Eintrags
#define F_GRID_INSERT					0x00000800
#define F_DOWN_CTRL						0x00001000
#define F_DOWN_DESELECT					0x00002000
// Hack fuer D&D: Hintergrund des Entries nicht painten
#define F_NO_EMPHASIS					0x00004000
// Selektion per Gummiband
#define F_RUBBERING						0x00008000
#define F_START_EDITTIMER_IN_MOUSEUP	0x00010000

class SvImpIconView
{
    friend class ImpIcnCursor;
    ScrollBar   	aVerSBar;
    ScrollBar   	aHorSBar;
    Rectangle		aCurSelectionRect;
    SvPtrarr		aSelectedRectList;
    MouseEvent		aMouseMoveEvent;
    Timer			aEditTimer;  // fuer Inplace-Editieren
    Timer			aMouseMoveTimer; // generiert MouseMoves bei Gummibandselektion
    // Boundrect des zuletzt eingefuegten Entries
    Rectangle		aPrevBoundRect;
    Size			aOutputSize;		// Pixel
    Size			aVirtOutputSize;    // expandiert automatisch
    Point			aDDLastEntryPos;
    Point			aDDLastRectPos;

    SvLBoxTreeList* pModel;
    SvIconView*		pView;
    ImpIcnCursor*	pImpCursor;
    long			nMaxVirtWidth;      // max.breite aVirtOutputSize
    SvPtrarr*		pZOrderList;
    long			nGridDX,
                    nGridDY;
    long			nHorSBarHeight,
                    nVerSBarWidth;
    WinBits			nWinBits;
    int				nViewMode;
    long			nHorDist;
    long			nVerDist;
    long			nMaxBmpWidth;
    long			nMaxBmpHeight;
    long			nMaxTextWidth;
    long			nMaxBoundHeight; // Hoehe des hoechsten BoundRects
    ULONG			nFlags;
    ULONG			nCurUserEvent;
    SvLBoxEntry*	pCurParent;
    SvLBoxEntry* 	pCursor;
    SvLBoxEntry*	pNextCursor; // wird in MovingEntry gesetzt und ist
                                 // nur in EntryMoved gueltig!
    SvLBoxEntry*	pDDRefEntry;
    VirtualDevice*	pDDDev;
    VirtualDevice*	pDDBufDev;
    VirtualDevice*  pDDTempDev;

    SvIconViewTextMode eTextMode;
    BOOL			bMustRecalcBoundingRects;

    void			CheckAllSizes();
    void 			CheckSizes( SvLBoxEntry* pEntry,
                        const SvIcnVwDataEntry* pViewData = 0  );
    void 			ShowCursor( BOOL bShow );

    void			SetNextEntryPos(const Point& rPos);
    Point			FindNextEntryPos( const Size& rBoundSize );
    void			ImpArrange();
    void			AdjustVirtSize( const Rectangle& );
    void			ResetVirtSize();
    void			CheckScrollBars();

                    DECL_LINK( ScrollUpDownHdl, ScrollBar * );
                    DECL_LINK( ScrollLeftRightHdl, ScrollBar * );
                    DECL_LINK( MouseMoveTimeoutHdl, Timer* );
                    DECL_LINK( EditTimeoutHdl, Timer* );
                    DECL_LINK( UserEventHdl, void* );
    void			AdjustScrollBars();
    void			PositionScrollBars( long nRealWidth, long nRealHeight );
    void			CalcDocPos( Point& aMousePos );
    BOOL			GetResizeRect( Rectangle& );
    void			PaintResizeRect( const Rectangle& );
    SvLBoxEntry*	GetNewCursor();
    void			ToggleSelection( SvLBoxEntry* );
    void			DeselectAllBut( SvLBoxEntry* );
    void			Center( SvLBoxEntry* pEntry, SvIcnVwDataEntry* ) const;
    void			StopEditTimer() { aEditTimer.Stop(); }
    void			StartEditTimer() { aEditTimer.Start(); }
    void 			ImpHideDDIcon();
    void 			ImpDrawXORRect( const Rectangle& rRect );
    void			AddSelectedRect( const Rectangle&, short nOffset = SELRECT_BORDER_OFFS );
    void			ClearSelectedRectList();
    Rectangle		CalcMaxTextRect( const SvLBoxEntry* pEntry,
                                    const SvIcnVwDataEntry* pViewData ) const;

    void			ClipAtVirtOutRect( Rectangle& rRect ) const;
    void 			AdjustAtGrid( const SvPtrarr& rRow, SvLBoxEntry* pStart=0 );
    Point			AdjustAtGrid(
                        const Rectangle& rCenterRect, // "Schwerpunkt" des Objekts (typ. Bmp-Rect)
                        const Rectangle& rBoundRect ) const;
    SvIconViewTextMode GetEntryTextModeSmart( const SvLBoxEntry* pEntry,
                        const SvIcnVwDataEntry* pViewData ) const;

    BOOL			CheckVerScrollBar();
    BOOL			CheckHorScrollBar();
    void 			CancelUserEvent();

public:

                    SvImpIconView( SvIconView* pView, SvLBoxTreeList*, WinBits nWinStyle );
                    ~SvImpIconView();

    void 			Clear( BOOL bInCtor = FALSE );
    void 			SetWindowBits( WinBits nWinStyle );
    void 			SetModel( SvLBoxTreeList* pTree, SvLBoxEntry* pParent )
                        { pModel = pTree; SetCurParent(pParent); }
    void 			EntryInserted( SvLBoxEntry*);
    void 			RemovingEntry( SvLBoxEntry* pEntry );
    void 			EntryRemoved();
    void 			MovingEntry( SvLBoxEntry* pEntry );
    void 			EntryMoved( SvLBoxEntry* pEntry );
    void 			TreeInserted( SvLBoxEntry* pEntry );
    void 			ChangedFont();
    void 			ModelHasEntryInvalidated( SvListEntry* );
    void 			EntryExpanded( SvLBoxEntry* pEntry );
    void 			EntryCollapsed( SvLBoxEntry* pEntry );
    void 			CollapsingEntry( SvLBoxEntry* pEntry );
    void 			EntrySelected( SvLBoxEntry*, BOOL bSelect );

    void 			Paint( const Rectangle& rRect );
    void 			RepaintSelectionItems();
    void 			MouseButtonDown( const MouseEvent& );
    void 			MouseButtonUp( const MouseEvent& );
    void 			MouseMove( const MouseEvent&);
    BOOL 			KeyInput( const KeyEvent& );
    void 			Resize();
    void 			GetFocus();
    void 			LoseFocus();
    void 			UpdateAll();
    void 			PaintEntry( SvLBoxEntry* pEntry,
                        SvIcnVwDataEntry* pViewData = 0 );
    void 			PaintEntry( SvLBoxEntry*, const Point&,
                        SvIcnVwDataEntry* pViewData = 0, OutputDevice* pOut = 0);
    void 			SetEntryPosition( SvLBoxEntry* pEntry, const Point& rPos,
                                      BOOL bAdjustRow = FALSE,
                                      BOOL bCheckScrollBars = FALSE );
    void 			InvalidateEntry( SvLBoxEntry* );
    void 			ViewDataInitialized( SvLBoxEntry* pEntry );
    SvLBoxItem* 	GetItem( SvLBoxEntry*, const Point& rAbsPos );

    void 			SetNoSelection();
    void 			SetDragDropMode( DragDropMode eDDMode );
    void 			SetSelectionMode( SelectionMode eSelMode  );

    void 			SttDrag( const Point& rPos );
    void 			EndDrag();

    SvLBoxEntry* 	GetCurEntry() const { return pCursor; }
    void 			SetCursor( SvLBoxEntry* );

    BOOL			IsEntryInView( SvLBoxEntry* );
    SvLBoxEntry* 	GetEntry( const Point& rDocPos );
    SvLBoxEntry*	GetNextEntry( const Point& rDocPos, SvLBoxEntry* pCurEntry );
    SvLBoxEntry*	GetPrevEntry( const Point& rDocPos, SvLBoxEntry* pCurEntry  );

    Point 			GetEntryPosition( SvLBoxEntry* );
    void 			MakeVisible( SvLBoxEntry* pEntry );

    void			Arrange();

    void			SetSpaceBetweenEntries( long nHor, long Ver );
    long			GetHorSpaceBetweenEntries() const { return nHorDist; }
    long			GetVerSpaceBetweenEntries() const { return nVerDist; }

    Rectangle		CalcFocusRect( SvLBoxEntry* );

    Rectangle		CalcBmpRect( SvLBoxEntry*, const Point* pPos = 0,
                        SvIcnVwDataEntry* pViewData=0 );
    Rectangle		CalcTextRect( SvLBoxEntry*, SvLBoxString* pItem = 0,
                                  const Point* pPos = 0,
                                  BOOL bForInplaceEdit = FALSE,
                                  SvIcnVwDataEntry* pViewData = 0 );

    long			CalcBoundingWidth( SvLBoxEntry*, const SvIcnVwDataEntry* pViewData = 0) const;
    long			CalcBoundingHeight( SvLBoxEntry*, const SvIcnVwDataEntry* pViewData= 0 ) const;
    Size			CalcBoundingSize( SvLBoxEntry*,
                        SvIcnVwDataEntry* pViewData = 0 ) const;
    void			FindBoundingRect( SvLBoxEntry* pEntry,
                        SvIcnVwDataEntry* pViewData = 0 );
    // berechnet alle BoundRects neu
    void			RecalcAllBoundingRects();
    // berechnet alle ungueltigen BoundRects neu
    void			RecalcAllBoundingRectsSmart();
    const Rectangle&  GetBoundingRect( SvLBoxEntry*,
                        SvIcnVwDataEntry* pViewData=0);
    void			InvalidateBoundingRect( SvLBoxEntry* );
    void			InvalidateBoundingRect( Rectangle& rRect ) { rRect.Right() = LONG_MAX; }
    BOOL			IsBoundingRectValid( const Rectangle& rRect ) const { return (BOOL)( rRect.Right() != LONG_MAX ); }

    void 			PaintEmphasis( const Rectangle&, BOOL bSelected,
                                   BOOL bCursored, OutputDevice* pOut = 0 );
    void 			PaintItem( const Rectangle& rRect, SvLBoxItem* pItem,
                        SvLBoxEntry* pEntry, USHORT nPaintFlags, OutputDevice* pOut = 0 );
    // berechnet alle BoundingRects neu, wenn bMustRecalcBoundingRects == TRUE
    void			CheckBoundingRects() { if (bMustRecalcBoundingRects) RecalcAllBoundingRects(); }
    // berechnet alle invalidierten BoundingRects neu
    void			UpdateBoundingRects();
    void			ShowTargetEmphasis( SvLBoxEntry* pEntry, BOOL bShow );
    SvLBoxEntry* 	GetDropTarget( const Point& rPosPixel );
    BOOL			NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos );
    BOOL			NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos );

    void 			WriteDragServerInfo( const Point&, SvLBoxDDInfo* );
    void 			ReadDragServerInfo( const Point&, SvLBoxDDInfo* );
    void			ToTop( SvLBoxEntry* );

    void			SetCurParent( SvLBoxEntry* pNewParent );
    SvLBoxEntry* 	GetCurParent() const { return pCurParent; }
    USHORT			GetSelectionCount() const;
    void			SetGrid( long nDX, long nDY );
    void			Scroll( long nDeltaX, long nDeltaY, BOOL bScrollBar = FALSE );
    const Size&		GetItemSize( SvIconView* pView, SvLBoxEntry*, SvLBoxItem*,
                        const SvIcnVwDataEntry* pViewData = 0 ) const;
    void			PrepareCommandEvent( const Point& rPt );

    void 			HideDDIcon();
    void 			ShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos );
    void			HideShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos );

    SvLBoxEntry* 	mpViewData;

    BOOL			IsOver(	SvPtrarr* pSelectedRectList, const Rectangle& rEntryBoundRect ) const;
    void			SelectRect( const Rectangle&, BOOL bAdd = TRUE,
                        SvPtrarr* pOtherRects = 0,
                        short nOffs = SELRECT_BORDER_OFFS );
    void			DrawSelectionRect( const Rectangle& );
    void			HideSelectionRect();
    void			CalcScrollOffsets( const Point& rRefPosPixel,
                        long& rX, long& rY, BOOL bDragDrop = FALSE,
                        USHORT nBorderWidth = 10 );
    void			EndTracking();
    BOOL			IsTextHit( SvLBoxEntry* pEntry, const Point& rDocPos );
    void			MakeVisible( const Rectangle& rDocPos,BOOL bInScrollBarEvent=FALSE);
    void			AdjustAtGrid( SvLBoxEntry* pStart = 0 );
    void			SetTextMode( SvIconViewTextMode, SvLBoxEntry* pEntry = 0 );
    SvIconViewTextMode GetTextMode( const SvLBoxEntry* pEntry = 0,
                    const SvIcnVwDataEntry* pViewData = 0 ) const;
    void			ShowFocusRect( const SvLBoxEntry* pEntry );
};

inline void SvImpIconView::MakeVisible( SvLBoxEntry* pEntry )
{
    const Rectangle& rRect = GetBoundingRect( pEntry );
    MakeVisible( rRect );
}

}

#endif // #ifndef _SVIMPICN_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
