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
#ifndef SC_TABVIEW_HXX
#define SC_TABVIEW_HXX

#include <vcl/scrbar.hxx>

#include <sfx2/ipclient.hxx>

#include "viewutil.hxx"
#include "select.hxx"

class ScEditEngineDefaulter;
class ScGridWindow;
class ScOutlineWindow;
class ScRowBar;
class ScColBar;
class ScTabControl;
class ScTabViewShell;
class SfxPrinter;
class ScDrawView;
class SvBorder;
class FuPoor;
class Splitter;
class ScTabSplitter;
class SdrView;
class SdrObject;
class ScHintWindow;
class ScPageBreakData;
class ScHighlightRanges;
struct ChartSelectionInfo;
class SdrHdlList;

namespace com { namespace sun { namespace star {
namespace chart2 { namespace data {
    struct HighlightedRange;
}}}}}

#define SPLIT_HANDLE_SIZE   3
#define SC_FORCEMODE_NONE   0xff

// ---------------------------------------------------------------------------
//      Hilfs - Fenster

class ScCornerButton : public Window
{
private:
    ScViewData*     pViewData;
    sal_Bool            bAdd;

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
public:
                    ScCornerButton( Window* pParent, ScViewData* pData, sal_Bool bAdditional );
                    ~ScCornerButton();

    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};


// ---------------------------------------------------------------------------

class ScTabView
{
private:
    Window*             pFrameWin;              // als erstes !!!
    ScViewData          aViewData;              // muss ganz vorne stehen !

    ScViewSelectionEngine*  pSelEngine;
    ScViewFunctionSet       aFunctionSet;

    ScHeaderSelectionEngine* pHdrSelEng;
    ScHeaderFunctionSet      aHdrFunc;

    SfxInPlaceClient*   pIPClient;

    ScDrawView*         pDrawView;

    Size                aFrameSize;             // wie bei DoResize uebergeben
    Point               aBorderPos;

    sal_Bool                bDrawSelMode;           // nur Zeichenobjekte selektieren ?

    FuPoor*             pDrawActual;
    FuPoor*             pDrawOld;

    ScGridWindow*       pGridWin[4];
    ScColBar*           pColBar[2];
    ScRowBar*           pRowBar[2];
    ScOutlineWindow*    pColOutline[2];
    ScOutlineWindow*    pRowOutline[2];
    ScTabSplitter*      pHSplitter;
    ScTabSplitter*      pVSplitter;
    ScTabControl*       pTabControl;
    ScrollBar           aVScrollTop;
    ScrollBar           aVScrollBottom;         // anfangs sichtbar
    ScrollBar           aHScrollLeft;           // anfangs sichtbar
    ScrollBar           aHScrollRight;
    ScCornerButton      aCornerButton;
    ScCornerButton      aTopButton;
    ScrollBarBox        aScrollBarBox;

    ScHintWindow*       pInputHintWindow;       // Eingabemeldung bei Gueltigkeit

    ScPageBreakData*    pPageBreakData;         // fuer Seitenumbruch-Modus
    ScHighlightRanges*  pHighlightRanges;

    ScDocument*         pBrushDocument;         // cell formats for format paint brush
    SfxItemSet*         pDrawBrushSet;          // drawing object attributes for paint brush
    sal_Bool                bLockPaintBrush;        // keep for more than one use?

    Timer               aScrollTimer;
    ScGridWindow*       pTimerWindow;
    MouseEvent          aTimerMEvt;

    sal_uLong               nTipVisible;

    sal_Bool                bDragging;              // fuer Scrollbars
    long                nPrevDragPos;

    sal_Bool                bIsBlockMode;           // Block markieren
    sal_Bool                bBlockNeg;              // wird Markierung aufgehoben?
    sal_Bool                bBlockCols;             // werden ganze Spalten markiert?
    sal_Bool                bBlockRows;             // werden ganze Zeilen markiert?

    SCCOL               nBlockStartX;
    SCCOL               nBlockStartXOrig;
    SCCOL               nBlockEndX;

    SCROW               nBlockStartY;
    SCROW               nBlockStartYOrig;
    SCROW               nBlockEndY;

    SCTAB               nBlockStartZ;
    SCTAB               nBlockEndZ;

    SCCOL               nOldCurX;
    SCROW               nOldCurY;

    double              mfPendingTabBarWidth;       // Tab bar width relative to frame window width.

    sal_Bool                bMinimized;
    sal_Bool                bInUpdateHeader;
    sal_Bool                bInActivatePart;
    sal_Bool                bInZoomUpdate;
    sal_Bool                bMoveIsShift;

    void            Init();

    void            DoAddWin( ScGridWindow* pWin );

    void            InitScrollBar( ScrollBar& rScrollBar, long nMaxVal );
    DECL_LINK(      ScrollHdl, ScrollBar* );
    DECL_LINK(      EndScrollHdl, ScrollBar* );

    DECL_LINK(      SplitHdl, Splitter* );
    void            DoHSplit(long nSplitPos);
    void            DoVSplit(long nSplitPos);

    DECL_LINK(      TimerHdl, Timer* );

    void            UpdateVarZoom();

    static void     SetScrollBar( ScrollBar& rScroll, long nRangeMax, long nVisible, long nPos, sal_Bool bLayoutRTL );
    static long     GetScrollBarPos( ScrollBar& rScroll, sal_Bool bLayoutRTL );

    void            GetPageMoveEndPosition(SCsCOL nMovX, SCsROW nMovY, SCsCOL& rPageX, SCsROW& rPageY);
    void            GetAreaMoveEndPosition(SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                           SCsCOL& rAreaX, SCsROW& rAreaY, ScFollowMode& rMode);

    void            SkipCursorHorizontal(SCsCOL& rCurX, SCsROW& rCurY, SCsCOL nOldX, SCsROW nMovX);
    void            SkipCursorVertical(SCsCOL& rCurX, SCsROW& rCurY, SCsROW nOldY, SCsROW nMovY);

protected:
    void            UpdateHeaderWidth( const ScVSplitPos* pWhich = NULL,
                                        const SCROW* pPosY = NULL );

    void            HideTip();
    void            ShowRefTip();

    void            ZoomChanged();
    void            UpdateShow();
    void            UpdateVisibleRange();
    void            GetBorderSize( SvBorder& rBorder, const Size& rSize );

    void            ResetDrawDragMode();
    sal_Bool            IsDrawTextEdit() const;
    void            DrawEnableAnim(sal_Bool bSet);
    //HMHvoid           DrawShowMarkHdl(sal_Bool bShow);

    void            MakeDrawView( sal_uInt8 nForceDesignMode = SC_FORCEMODE_NONE );

    void            HideNoteMarker();

    void            UpdateIMap( SdrObject* pObj );

public:
                    ScTabView( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ~ScTabView();

    void            MakeDrawLayer();

    void            HideListBox();

    sal_Bool            HasHintWindow() const   { return pInputHintWindow != NULL; }
    void            RemoveHintWindow();
    void            TestHintWindow();


    DECL_LINK(      TabBarResize, void* );
    /** Sets an absolute tab bar width (in pixels). */
    void            SetTabBarWidth( long nNewWidth );
    /** Sets a relative tab bar width.
        @param fRelTabBarWidth  Tab bar width relative to frame window width (0.0 ... 1.0). */
    void            SetRelTabBarWidth( double fRelTabBarWidth );
    /** Sets a relative tab bar width. Tab bar is resized again in next DoResize().
        @param fRelTabBarWidth  Tab bar width relative to frame window width (0.0 ... 1.0). */
    void            SetPendingRelTabBarWidth( double fRelTabBarWidth );
    /** Returns the current tab bar width in pixels. */
    long            GetTabBarWidth() const;
    /** Returns the current tab bar width relative to the frame window width (0.0 ... 1.0). */
    double          GetRelTabBarWidth() const;
    /** Returns the pending tab bar width relative to the frame window width (0.0 ... 1.0). */
    double          GetPendingRelTabBarWidth() const;

    void            DoResize( const Point& rOffset, const Size& rSize, sal_Bool bInner = false );
    void            RepeatResize( sal_Bool bUpdateFix = sal_True );
    void            UpdateFixPos();
    Point           GetGridOffset() const;

    sal_Bool            IsDrawSelMode() const       { return bDrawSelMode; }
    void            SetDrawSelMode(sal_Bool bNew)   { bDrawSelMode = bNew; }

    void            SetDrawFuncPtr(FuPoor* pFuncPtr)    { pDrawActual = pFuncPtr; }
    void            SetDrawFuncOldPtr(FuPoor* pFuncPtr) { pDrawOld = pFuncPtr; }
    FuPoor*         GetDrawFuncPtr()                    { return pDrawActual; }
    FuPoor*         GetDrawFuncOldPtr()                 { return pDrawOld; }

    void            DrawDeselectAll();
    void            DrawMarkListHasChanged();
    void            UpdateAnchorHandles();

    ScPageBreakData* GetPageBreakData()     { return pPageBreakData; }
    ScHighlightRanges* GetHighlightRanges() { return pHighlightRanges; }

    void            UpdatePageBreakData( sal_Bool bForcePaint = false );

    void            DrawMarkRect( const Rectangle& rRect );

    ScViewData*         GetViewData()       { return &aViewData; }
    const ScViewData*   GetViewData() const { return &aViewData; }

    ScViewFunctionSet*      GetFunctionSet()    { return &aFunctionSet; }
    ScViewSelectionEngine*  GetSelEngine()      { return pSelEngine; }

    sal_Bool            SelMouseButtonDown( const MouseEvent& rMEvt );

    ScDrawView*     GetScDrawView()         { return pDrawView; }
    SdrView*        GetSdrView();           // gegen CLOKs

    sal_Bool            IsMinimized() const     { return bMinimized; }

    // bSameTabButMoved = true if the same sheet as before is activated, used after moving/copying/inserting/deleting a sheet
    void            TabChanged( bool bSameTabButMoved = false );
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY, sal_Bool bAll );
    SC_DLLPUBLIC void            RefreshZoom();
    void            SetPagebreakMode( sal_Bool bSet );

    void            UpdateLayerLocks();

    void            UpdateDrawTextOutliner();
    void            DigitLanguageChanged();

    void            UpdateInputLine();

    void            InitRefMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScRefType eType,
                                    sal_Bool bPaint = sal_True );
    void            DoneRefMode( sal_Bool bContinue = false );
    void            UpdateRef( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ );
    void            StopRefMode();

    void            StopMarking();
    void            FakeButtonUp( ScSplitPos eWhich );

    Window*         GetActiveWin();
    Window*         GetWindowByPos( ScSplitPos ePos );

    ScSplitPos      FindWindow( Window* pWindow ) const;

    void            SetActivePointer( const Pointer& rPointer );

    void            ActiveGrabFocus();

    void            ClickCursor( SCCOL nPosX, SCROW nPosY, sal_Bool bControl );

    SC_DLLPUBLIC void           SetCursor( SCCOL nPosX, SCROW nPosY, sal_Bool bNew = false );

    SC_DLLPUBLIC void           CellContentChanged();
    void            SelectionChanged();
    void            CursorPosChanged();
    void            UpdateInputContext();

    void            CheckSelectionTransfer();

    void            InvertHorizontal( ScVSplitPos eWhich, long nDragPos );
    void            InvertVertical( ScHSplitPos eWhich, long nDragPos );

    Point           GetInsertPos();

    Point           GetChartInsertPos( const Size& rSize, const ScRange& rCellRange );
    Point           GetChartDialogPos( const Size& rDialogSize, const Rectangle& rLogicChart );

    void            UpdateAutoFillMark();

    void            HideCursor();               // nur aktiver Teil
    void            ShowCursor();
    void            HideAllCursors();
    void            ShowAllCursors();

    void            AlignToCursor( SCsCOL nCurX, SCsROW nCurY, ScFollowMode eMode,
                                    const ScSplitPos* pWhich = NULL );

    SvxZoomType     GetZoomType() const;
    void            SetZoomType( SvxZoomType eNew, sal_Bool bAll );
    sal_uInt16          CalcZoom( SvxZoomType eType, sal_uInt16 nOldZoom );

//  void            CalcZoom( SvxZoomType eType, sal_uInt16& rZoom, SCCOL& rCol, SCROW& rRow );

    sal_Bool        HasPageFieldDataAtCursor() const;
    void            StartDataSelect();

                    //  MoveCursorAbs       - absolut
                    //  MoveCursorRel       - einzelne Zellen
                    //  MoveCursorPage      - Bildschirmseite
                    //  MoveCursorArea      - Datenblock
                    //  MoveCursorEnd       - links oben / benutzter Bereich

    SC_DLLPUBLIC void           MoveCursorAbs( SCsCOL nCurX, SCsROW nCurY, ScFollowMode eMode,
                                    sal_Bool bShift, sal_Bool bControl,
                                    sal_Bool bKeepOld = false, sal_Bool bKeepSel = false );
    void            MoveCursorRel( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    sal_Bool bShift, sal_Bool bKeepSel = false );
    void            MoveCursorPage( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    sal_Bool bShift, sal_Bool bKeepSel = false );
    void            MoveCursorArea( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    sal_Bool bShift, sal_Bool bKeepSel = false );
    void            MoveCursorEnd( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    sal_Bool bShift, sal_Bool bKeepSel = false );
    void            MoveCursorScreen( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode, sal_Bool bShift );

    void            MoveCursorEnter( sal_Bool bShift );     // Shift fuer Richtung (kein Markieren)

    sal_Bool            MoveCursorKeyInput( const KeyEvent& rKeyEvent );

    void            FindNextUnprot( sal_Bool bShift, sal_Bool bInSelection = sal_True );

    SC_DLLPUBLIC void SetTabNo( SCTAB nTab, bool bNew = false, bool bExtendSelection = false, bool bSameTabButMoved = false );
    void            SelectNextTab( short nDir, sal_Bool bExtendSelection = false );

    void            ActivateView( sal_Bool bActivate, sal_Bool bFirst );
    void            ActivatePart( ScSplitPos eWhich );
    sal_Bool            IsInActivatePart() const    { return bInActivatePart; }

    void            SetTimer( ScGridWindow* pWin, const MouseEvent& rMEvt );
    void            ResetTimer();

    void            ScrollX( long nDeltaX, ScHSplitPos eWhich, sal_Bool bUpdBars = sal_True );
    void            ScrollY( long nDeltaY, ScVSplitPos eWhich, sal_Bool bUpdBars = sal_True );
    SC_DLLPUBLIC void           ScrollLines( long nDeltaX, long nDeltaY );              // aktives

    sal_Bool            ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos );

    void            ScrollToObject( SdrObject* pDrawObj );
    void            MakeVisible( const Rectangle& rHMMRect );

                                    // Zeichnen

    void            PaintArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                        ScUpdateMode eMode = SC_UPDATE_ALL );

    void            PaintGrid();

    void            PaintTopArea( SCCOL nStartCol, SCCOL nEndCol );
    void            PaintTop();

    void            PaintLeftArea( SCROW nStartRow, SCROW nEndRow );
    void            PaintLeft();

    sal_Bool            PaintExtras();

    void            RecalcPPT();

    void            CreateAnchorHandles(SdrHdlList& rHdl, const ScAddress& rAddress);

    void            UpdateCopySourceOverlay();
    void            UpdateSelectionOverlay();
    void            UpdateShrinkOverlay();
    void            UpdateAllOverlays();

    void            UpdateFormulas();
    void            InterpretVisible();
    void            CheckNeedsRepaint();

    void            PaintRangeFinder( long nNumber = -1 );
    void            AddHighlightRange( const ScRange& rRange, const Color& rColor );
    void            ClearHighlightRanges();

    void            DoChartSelection( const ::com::sun::star::uno::Sequence<
                                      ::com::sun::star::chart2::data::HighlightedRange > & rHilightRanges );

    long            GetGridWidth( ScHSplitPos eWhich );
    long            GetGridHeight( ScVSplitPos eWhich );

    void            UpdateScrollBars();
    void            SetNewVisArea();

    void            InvalidateAttribs();

    void            MakeEditView( ScEditEngineDefaulter* pEngine, SCCOL nCol, SCROW nRow );
    void            KillEditView( sal_Bool bNoPaint );
    void            UpdateEditView();


                                    //  Bloecke

    void            SelectAll( sal_Bool bContinue = false );
    void            SelectAllTables();
    void            DeselectAllTables();

    void            MarkCursor( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                sal_Bool bCols = false, sal_Bool bRows = false, sal_Bool bCellSelection = false );
    void            InitBlockMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                    sal_Bool bTestNeg = false,
                                    sal_Bool bCols = false, sal_Bool bRows = false );
    void            InitOwnBlockMode();
    void            DoneBlockMode( sal_Bool bContinue = false );

    sal_Bool            IsBlockMode() const     { return bIsBlockMode; }

    void            ExpandBlock(SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode);
    void            ExpandBlockPage(SCsCOL nMovX, SCsROW nMovY);
    void            ExpandBlockArea(SCsCOL nMovX, SCsROW nMovY);

    void            MarkColumns();
    void            MarkRows();
    void            MarkDataArea( sal_Bool bIncludeCursor = sal_True );
    void            MarkMatrixFormula();
    void            Unmark();

    void            MarkRange( const ScRange& rRange, sal_Bool bSetCursor = sal_True, sal_Bool bContinue = false );

    sal_Bool            IsMarking( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    void            PaintMarks( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    void            PaintBlock( sal_Bool bReset = false );

    void            SetMarkData( const ScMarkData& rNew );
    void            MarkDataChanged();

    void            LockModifiers( sal_uInt16 nModifiers );
    sal_uInt16          GetLockedModifiers() const;
    void            ViewOptionsHasChanged( sal_Bool bHScrollChanged,
                                            sal_Bool bGraphicsChanged = false);

    Point           GetMousePosPixel();

    void            SnapSplitPos( Point& rScreenPosPixel );
    void            FreezeSplitters( sal_Bool bFreeze );
    void            RemoveSplit();
    void            SplitAtCursor();
    void            SplitAtPixel( const Point& rPixel, sal_Bool bHor, sal_Bool bVer );
    void            InvalidateSplit();

    void            ErrorMessage( sal_uInt16 nGlobStrId );
    Window*         GetParentOrChild( sal_uInt16 nChildId );

    void            EnableRefInput(sal_Bool bFlag=sal_True);

    Window*         GetFrameWin() const { return pFrameWin; }

    sal_Bool            HasPaintBrush() const           { return pBrushDocument || pDrawBrushSet; }
    ScDocument*     GetBrushDocument() const        { return pBrushDocument; }
    SfxItemSet*     GetDrawBrushSet() const         { return pDrawBrushSet; }
    sal_Bool            IsPaintBrushLocked() const      { return bLockPaintBrush; }
    void            SetBrushDocument( ScDocument* pNew, sal_Bool bLock );
    void            SetDrawBrushSet( SfxItemSet* pNew, sal_Bool bLock );
    void            ResetBrushDocument();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
