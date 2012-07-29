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

#include "hiranges.hxx"
#include "viewutil.hxx"
#include "select.hxx"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class ScEditEngineDefaulter;
class ScGridWindow;
class ScOutlineWindow;
class ScRowBar;
class ScColBar;
class ScTabControl;
class ScTabViewShell;
struct ScRangeFindData;
class ScDrawView;
class SvBorder;
class FuPoor;
class Splitter;
class ScTabSplitter;
class SdrView;
class SdrObject;
class ScHintWindow;
class ScPageBreakData;
class SdrHdlList;

namespace com { namespace sun { namespace star {
namespace chart2 { namespace data {
    struct HighlightedRange;
}}}}}

#define SC_FORCEMODE_NONE   0xff

// ---------------------------------------------------------------------------
//      Help - Window

class ScCornerButton : public Window
{
private:
    ScViewData*     pViewData;
    bool            bAdd;

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
public:
                    ScCornerButton( Window* pParent, ScViewData* pData, bool bAdditional );
                    ~ScCornerButton();

    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};


// ---------------------------------------------------------------------------

class ScTabView : boost::noncopyable
{
private:
    enum BlockMode { None = 0, Normal = 1, Own = 2 };

    Window*             pFrameWin;              // First !!!
    ScViewData          aViewData;              // must be at the front !

    ScViewSelectionEngine*  pSelEngine;
    ScViewFunctionSet       aFunctionSet;

    ScHeaderSelectionEngine* pHdrSelEng;
    ScHeaderFunctionSet      aHdrFunc;

    SfxInPlaceClient*   pIPClient;

    ScDrawView*         pDrawView;

    Size                aFrameSize;             // passed on as for DoResize
    Point               aBorderPos;

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
    ScrollBar           aVScrollBottom;         // initially visible
    ScrollBar           aHScrollLeft;           // initially visible
    ScrollBar           aHScrollRight;
    ScCornerButton      aCornerButton;
    ScCornerButton      aTopButton;
    ScrollBarBox        aScrollBarBox;

    boost::scoped_ptr<ScHintWindow> mpInputHintWindow; // popup window for data validation

    ScPageBreakData*    pPageBreakData;
    std::vector<ScHighlightEntry>   maHighlightRanges;

    ScDocument*         pBrushDocument;         // cell formats for format paint brush
    SfxItemSet*         pDrawBrushSet;          // drawing object attributes for paint brush

    Timer               aScrollTimer;
    ScGridWindow*       pTimerWindow;
    MouseEvent          aTimerMEvt;

    sal_uLong               nTipVisible;

    long                nPrevDragPos;

    BlockMode           meBlockMode;           // Marks block

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

    bool                bMinimized:1;
    bool                bInUpdateHeader:1;
    bool                bInActivatePart:1;
    bool                bInZoomUpdate:1;
    bool                bMoveIsShift:1;
    bool                bDrawSelMode:1;           // Only select draw objects ?
    bool                bLockPaintBrush:1;        // keep for more than one use?
    bool                bDragging:1;              // for scroll bars
    bool                bBlockNeg:1;              // is no longer highlighted?
    bool                bBlockCols:1;             // are whole columns selected?
    bool                bBlockRows:1;             // are whole rows selected?

    void            Init();

    void            DoAddWin( ScGridWindow* pWin );

    void            InitScrollBar( ScrollBar& rScrollBar, long nMaxVal );
    DECL_LINK(      ScrollHdl, ScrollBar* );
    DECL_LINK(      EndScrollHdl, ScrollBar* );

    DECL_LINK(      SplitHdl, Splitter* );
    void            DoHSplit(long nSplitPos);
    void            DoVSplit(long nSplitPos);

    DECL_LINK(      TimerHdl, void* );

    void            UpdateVarZoom();

    static void     SetScrollBar( ScrollBar& rScroll, long nRangeMax, long nVisible, long nPos, bool bLayoutRTL );
    static long     GetScrollBarPos( ScrollBar& rScroll, bool bLayoutRTL );

    void            GetPageMoveEndPosition(SCsCOL nMovX, SCsROW nMovY, SCsCOL& rPageX, SCsROW& rPageY);
    void            GetAreaMoveEndPosition(SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                           SCsCOL& rAreaX, SCsROW& rAreaY, ScFollowMode& rMode);

    void            SkipCursorHorizontal(SCsCOL& rCurX, SCsROW& rCurY, SCsCOL nOldX, SCsROW nMovX);
    void            SkipCursorVertical(SCsCOL& rCurX, SCsROW& rCurY, SCsROW nOldY, SCsROW nMovY);

    /**
     *
     *  @brief Update marks for a selected Range. This is a helper function
     *  for PaintRangeFinder.
     *
     *  @param pData: Range to update for painting.
     *  @param nTab: Current tab.
     *
     **/

    void            PaintRangeFinderEntry (ScRangeFindData* pData, SCTAB nTab);

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
    bool            IsDrawTextEdit() const;
    void            DrawEnableAnim(bool bSet);

    void            MakeDrawView( sal_uInt8 nForceDesignMode = SC_FORCEMODE_NONE );

    void            HideNoteMarker();

    void            UpdateIMap( SdrObject* pObj );

public:
                    ScTabView( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ~ScTabView();

    void            MakeDrawLayer();

    void            HideListBox();

    bool            HasHintWindow() const;
    void            RemoveHintWindow();
    void            TestHintWindow();


    DECL_LINK(      TabBarResize, void* );
    /** Sets an absolute tab bar width (in pixels). */
    void            SetTabBarWidth( long nNewWidth );
    /** Sets a relative tab bar width.
        @param fRelTabBarWidth  Tab bar width relative to frame window width (0.0 ... 1.0). */
    SC_DLLPUBLIC void            SetRelTabBarWidth( double fRelTabBarWidth );
    /** Sets a relative tab bar width. Tab bar is resized again in next DoResize().
        @param fRelTabBarWidth  Tab bar width relative to frame window width (0.0 ... 1.0). */
    void            SetPendingRelTabBarWidth( double fRelTabBarWidth );
    /** Returns the current tab bar width in pixels. */
    long            GetTabBarWidth() const;
    /** Returns the current tab bar width relative to the frame window width (0.0 ... 1.0). */
    SC_DLLPUBLIC double          GetRelTabBarWidth() const;
    /** Returns the pending tab bar width relative to the frame window width (0.0 ... 1.0). */
    double          GetPendingRelTabBarWidth() const;

    void            DoResize( const Point& rOffset, const Size& rSize, bool bInner = false );
    void            RepeatResize( bool bUpdateFix = true );
    void            UpdateFixPos();
    Point           GetGridOffset() const;

    bool            IsDrawSelMode() const       { return bDrawSelMode; }
    void            SetDrawSelMode(bool bNew)   { bDrawSelMode = bNew; }

    void            SetDrawFuncPtr(FuPoor* pFuncPtr)    { pDrawActual = pFuncPtr; }
    void            SetDrawFuncOldPtr(FuPoor* pFuncPtr) { pDrawOld = pFuncPtr; }
    FuPoor*         GetDrawFuncPtr()                    { return pDrawActual; }
    FuPoor*         GetDrawFuncOldPtr()                 { return pDrawOld; }

    void            DrawDeselectAll();
    void            DrawMarkListHasChanged();
    void            UpdateAnchorHandles();

    ScPageBreakData* GetPageBreakData()     { return pPageBreakData; }
    const std::vector<ScHighlightEntry>& GetHighlightRanges()   { return maHighlightRanges; }

    void            UpdatePageBreakData( bool bForcePaint = false );

    void            DrawMarkRect( const Rectangle& rRect );

    ScViewData*         GetViewData()       { return &aViewData; }
    const ScViewData*   GetViewData() const { return &aViewData; }

    ScViewFunctionSet*      GetFunctionSet()    { return &aFunctionSet; }
    ScViewSelectionEngine*  GetSelEngine()      { return pSelEngine; }

    bool            SelMouseButtonDown( const MouseEvent& rMEvt );

    ScDrawView*     GetScDrawView()         { return pDrawView; }
    SdrView*        GetSdrView();           // gegen CLOKs

    bool            IsMinimized() const     { return bMinimized; }

    /**
     * Called after moving, copying, inserting or deleting a sheet.
     *
     * @param bSameTabButMoved true if the same sheet as before is activated.
     */
    void            TabChanged( bool bSameTabButMoved = false );
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY, bool bAll );
    SC_DLLPUBLIC void            RefreshZoom();
    void            SetPagebreakMode( bool bSet );

    void            UpdateLayerLocks();

    void            UpdateDrawTextOutliner();
    void            DigitLanguageChanged();

    void            UpdateInputLine();

    void            InitRefMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScRefType eType,
                                 bool bPaint = true );
    void            DoneRefMode( bool bContinue = false );
    void            UpdateRef( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ );
    void            StopRefMode();

    void            StopMarking();
    void            FakeButtonUp( ScSplitPos eWhich );

    Window*         GetActiveWin();
    Window*         GetWindowByPos( ScSplitPos ePos );

    ScSplitPos      FindWindow( Window* pWindow ) const;

    void            SetActivePointer( const Pointer& rPointer );

    void            ActiveGrabFocus();

    void            ClickCursor( SCCOL nPosX, SCROW nPosY, bool bControl );

    SC_DLLPUBLIC void SetCursor( SCCOL nPosX, SCROW nPosY, bool bNew = false );

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

    void            ShowCursor();
    void            HideAllCursors();
    void            ShowAllCursors();

    void            AlignToCursor( SCsCOL nCurX, SCsROW nCurY, ScFollowMode eMode,
                                    const ScSplitPos* pWhich = NULL );

    SvxZoomType     GetZoomType() const;
    void            SetZoomType( SvxZoomType eNew, bool bAll );
    sal_uInt16          CalcZoom( SvxZoomType eType, sal_uInt16 nOldZoom );

    bool            HasPageFieldDataAtCursor() const;
    void            StartDataSelect();

                    //  MoveCursorAbs       - absolute
                    //  MoveCursorRel       - single cells
                    //  MoveCursorPage      - screen
                    //  MoveCursorArea      - Data block
                    //  MoveCursorEnd       - top left / user range

    SC_DLLPUBLIC void MoveCursorAbs( SCsCOL nCurX, SCsROW nCurY, ScFollowMode eMode,
                                     bool bShift, bool bControl,
                                     bool bKeepOld = false, bool bKeepSel = false );
    void            MoveCursorRel( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                   bool bShift, bool bKeepSel = false );
    void            MoveCursorPage( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    bool bShift, bool bKeepSel = false );
    void            MoveCursorArea( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    bool bShift, bool bKeepSel = false );
    void            MoveCursorEnd( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                   bool bShift, bool bKeepSel = false );
    void            MoveCursorScreen( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode, bool bShift );

    void            MoveCursorEnter( bool bShift );     // Shift for direction (select nothing)

    bool            MoveCursorKeyInput( const KeyEvent& rKeyEvent );

    void            FindNextUnprot( bool bShift, bool bInSelection = true );

    SC_DLLPUBLIC void SetTabNo( SCTAB nTab, bool bNew = false, bool bExtendSelection = false, bool bSameTabButMoved = false );
    void            SelectNextTab( short nDir, bool bExtendSelection = false );

    void            ActivateView( bool bActivate, bool bFirst );
    void            ActivatePart( ScSplitPos eWhich );
    bool            IsInActivatePart() const    { return bInActivatePart; }

    void            SetTimer( ScGridWindow* pWin, const MouseEvent& rMEvt );
    void            ResetTimer();

    void            ScrollX( long nDeltaX, ScHSplitPos eWhich, bool bUpdBars = true );
    void            ScrollY( long nDeltaY, ScVSplitPos eWhich, bool bUpdBars = true );
    SC_DLLPUBLIC void           ScrollLines( long nDeltaX, long nDeltaY );              // active

    bool            ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos );

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

    bool            PaintExtras();

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
    void            KillEditView( bool bNoPaint );
    void            UpdateEditView();


                                    //  Blocks

    void            SelectAll( bool bContinue = false );
    void            SelectAllTables();
    void            DeselectAllTables();

    void            MarkCursor( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                bool bCols = false, bool bRows = false, bool bCellSelection = false );
    void            InitBlockMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                   bool bTestNeg = false,
                                   bool bCols = false, bool bRows = false );
    void            InitOwnBlockMode();
    void            DoneBlockMode( bool bContinue = false );

    bool            IsBlockMode() const;

    void            ExpandBlock(SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode);
    void            ExpandBlockPage(SCsCOL nMovX, SCsROW nMovY);
    void            ExpandBlockArea(SCsCOL nMovX, SCsROW nMovY);

    void            MarkColumns();
    void            MarkRows();
    void            MarkDataArea( bool bIncludeCursor = true );
    void            MarkMatrixFormula();
    void            Unmark();

    void            MarkRange( const ScRange& rRange, bool bSetCursor = true, bool bContinue = false );

    bool            IsMarking( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    void            PaintMarks( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    void            PaintBlock( bool bReset = false );

    void            SetMarkData( const ScMarkData& rNew );
    void            MarkDataChanged();

    void            LockModifiers( sal_uInt16 nModifiers );
    sal_uInt16          GetLockedModifiers() const;
    void            ViewOptionsHasChanged( bool bHScrollChanged,
                                           bool bGraphicsChanged = false);

    Point           GetMousePosPixel();

    void            SnapSplitPos( Point& rScreenPosPixel );
    void            FreezeSplitters( bool bFreeze );
    void            RemoveSplit();
    void            SplitAtCursor();
    void            SplitAtPixel( const Point& rPixel, bool bHor, bool bVer );
    void            InvalidateSplit();

    void            ErrorMessage( sal_uInt16 nGlobStrId );
    Window*         GetParentOrChild( sal_uInt16 nChildId );

    void            EnableRefInput(bool bFlag = true);

    Window*         GetFrameWin() const { return pFrameWin; }

    bool            HasPaintBrush() const           { return pBrushDocument || pDrawBrushSet; }
    ScDocument*     GetBrushDocument() const        { return pBrushDocument; }
    SfxItemSet*     GetDrawBrushSet() const         { return pDrawBrushSet; }
    bool            IsPaintBrushLocked() const      { return bLockPaintBrush; }
    void            SetBrushDocument( ScDocument* pNew, bool bLock );
    void            SetDrawBrushSet( SfxItemSet* pNew, bool bLock );
    void            ResetBrushDocument();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
