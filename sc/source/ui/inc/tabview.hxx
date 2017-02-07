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
#ifndef INCLUDED_SC_SOURCE_UI_INC_TABVIEW_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TABVIEW_HXX

#include <vcl/scrbar.hxx>

#include <sfx2/ipclient.hxx>

#include "hiranges.hxx"
#include "viewutil.hxx"
#include "select.hxx"
#include "gridwin.hxx"
#include "drawview.hxx"

namespace editeng {
    struct MisspellRanges;
}

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
class ScPageBreakData;
class SdrHdlList;
class TabBar;

namespace com { namespace sun { namespace star {
namespace chart2 { namespace data {
    struct HighlightedRange;
}}}}}

//      Help - Window

class ScCornerButton : public vcl::Window
{
private:
    ScViewData*     pViewData;
    bool            bAdd;

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
public:
                    ScCornerButton( vcl::Window* pParent, ScViewData* pData, bool bAdditional );
                    virtual ~ScCornerButton() override;

    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
};

class ScExtraEditViewManager
{
private:
    enum ModifierTagType { Adder, Remover };

public:
    ScExtraEditViewManager(ScTabViewShell* pThisViewShell, VclPtr<ScGridWindow>* pGridWin)
        : mpThisViewShell(pThisViewShell)
        , mpGridWin(pGridWin)
        , mpOtherEditView(nullptr)
        , nTotalWindows(0)
    {}

    ~ScExtraEditViewManager();

    void Add(SfxViewShell* pViewShell, ScSplitPos eWhich)
    {
        Apply<Adder>(pViewShell, eWhich);
    }

    void Remove(SfxViewShell* pViewShell, ScSplitPos eWhich)
    {
        Apply<Remover>(pViewShell, eWhich);
    }

private:
    template<ModifierTagType ModifierTag>
    void Apply(SfxViewShell* pViewShell, ScSplitPos eWhich);

    template<ModifierTagType ModifierTag>
    void Modifier(ScGridWindow* pWin);

private:
    ScTabViewShell* mpThisViewShell;
    VclPtr<ScGridWindow>* mpGridWin;
    EditView* mpOtherEditView;
    int nTotalWindows;
};

class ScTabView
{
private:
    enum BlockMode { None = 0, Normal = 1, Own = 2 };

    VclPtr<vcl::Window>             pFrameWin;              // First !!!
    ScViewData          aViewData;              // must be at the front !

    ScViewSelectionEngine*  pSelEngine;
    ScViewFunctionSet       aFunctionSet;

    ScHeaderSelectionEngine* pHdrSelEng;
    ScHeaderFunctionSet      aHdrFunc;

    ScDrawView*         pDrawView;

    Size                aFrameSize;             // passed on as for DoResize
    Point               aBorderPos;

    FuPoor*             pDrawActual;
    FuPoor*             pDrawOld;

    VclPtr<ScGridWindow>        pGridWin[4];
    VclPtr<ScColBar>            pColBar[2];
    VclPtr<ScRowBar>            pRowBar[2];
    VclPtr<ScOutlineWindow>     pColOutline[2];
    VclPtr<ScOutlineWindow>     pRowOutline[2];
    VclPtr<ScTabSplitter>       pHSplitter;
    VclPtr<ScTabSplitter>       pVSplitter;
    VclPtr<ScTabControl>        pTabControl;
    VclPtr<ScrollBar>           aVScrollTop;
    VclPtr<ScrollBar>           aVScrollBottom;         // initially visible
    VclPtr<ScrollBar>           aHScrollLeft;           // initially visible
    VclPtr<ScrollBar>           aHScrollRight;
    VclPtr<ScCornerButton>      aCornerButton;
    VclPtr<ScCornerButton>      aTopButton;
    VclPtr<ScrollBarBox>        aScrollBarBox;

    std::unique_ptr<sdr::overlay::OverlayObjectList> mxInputHintOO; // help hint for data validation

    ScPageBreakData*    pPageBreakData;
    std::vector<ScHighlightEntry>   maHighlightRanges;

    ScDocument*         pBrushDocument;         // cell formats for format paint brush
    SfxItemSet*         pDrawBrushSet;          // drawing object attributes for paint brush

    Timer               aScrollTimer;
    VclPtr<ScGridWindow>       pTimerWindow;
    MouseEvent          aTimerMEvt;

    ScExtraEditViewManager aExtraEditViewManager;

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
    bool                mbInlineWithScrollbar:1;  // should inline with scrollbar?

    void            Init();

    void            DoAddWin( ScGridWindow* pWin );

    void            InitScrollBar( ScrollBar& rScrollBar, long nMaxVal );
    DECL_LINK(ScrollHdl, ScrollBar*, void );
    DECL_LINK(EndScrollHdl, ScrollBar*, void );

    DECL_LINK(SplitHdl, Splitter*, void);
    void            DoHSplit(long nSplitPos);
    void            DoVSplit(long nSplitPos);

    DECL_LINK( TimerHdl, Timer*, void );

    void            UpdateVarZoom();

    static void     SetScrollBar( ScrollBar& rScroll, long nRangeMax, long nVisible, long nPos, bool bLayoutRTL );
    static long     GetScrollBarPos( ScrollBar& rScroll );

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
    void            UpdateHeaderWidth( const ScVSplitPos* pWhich = nullptr,
                                        const SCROW* pPosY = nullptr );

    void            HideTip();
    void            ShowRefTip();

    void            ZoomChanged();
    void            UpdateShow();
    bool            UpdateVisibleRange();
    void            GetBorderSize( SvBorder& rBorder, const Size& rSize );

    void            ResetDrawDragMode();
    bool            IsDrawTextEdit() const;
    void            DrawEnableAnim(bool bSet);

    void            MakeDrawView( TriState nForceDesignMode );

    void            HideNoteMarker();

    void            UpdateIMap( SdrObject* pObj );

public:
    /** make noncopyable */
                    ScTabView(const ScTabView&) = delete;
    const ScTabView&    operator=(const ScTabView&) = delete;

                    ScTabView( vcl::Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ~ScTabView();

    enum SplitMethod { SC_SPLIT_METHOD_FIRST_COL, SC_SPLIT_METHOD_FIRST_ROW, SC_SPLIT_METHOD_CURSOR };

    void            MakeDrawLayer();

    void            HideListBox();

    bool            HasHintWindow() const;
    void            RemoveHintWindow();
    void            TestHintWindow();

    DECL_LINK( TabBarResize, ::TabBar*, void );
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
    SC_DLLPUBLIC static double   GetRelTabBarWidth();
    /** Returns the pending tab bar width relative to the frame window width (0.0 ... 1.0). */
    double          GetPendingRelTabBarWidth() const { return mfPendingTabBarWidth;}

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

    ScViewData&         GetViewData()       { return aViewData; }
    const ScViewData&   GetViewData() const { return aViewData; }

    ScViewFunctionSet&      GetFunctionSet()    { return aFunctionSet; }
    ScViewSelectionEngine*  GetSelEngine()      { return pSelEngine; }

    bool            SelMouseButtonDown( const MouseEvent& rMEvt );

    ScDrawView*     GetScDrawView()         { return pDrawView; }
    // gegen CLOKs
    SdrView*        GetSdrView()            { return pDrawView; }

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

    static void     UpdateInputLine();

    void            InitRefMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScRefType eType );
    void            DoneRefMode( bool bContinue = false );
    void            UpdateRef( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ );
    void            StopRefMode();

    void            StopMarking();
    void            FakeButtonUp( ScSplitPos eWhich );

    ScGridWindow*   GetActiveWin();
    vcl::Window*         GetWindowByPos( ScSplitPos ePos ) { return pGridWin[ePos]; }

    ScSplitPos      FindWindow( vcl::Window* pWindow ) const;

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
                                    const ScSplitPos* pWhich = nullptr );

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

    void            FindNextUnprot( bool bShift, bool bInSelection );

    void            GetPageMoveEndPosition(SCsCOL nMovX, SCsROW nMovY, SCsCOL& rPageX, SCsROW& rPageY);

    SC_DLLPUBLIC void SetTabNo( SCTAB nTab, bool bNew = false, bool bExtendSelection = false, bool bSameTabButMoved = false );
    void            SelectNextTab( short nDir, bool bExtendSelection );
    void            SelectTabPage( const sal_uInt16 nTab );

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
    bool            NeedsRepaint();

    void            PaintRangeFinder( long nNumber );
    void            AddHighlightRange( const ScRange& rRange, const Color& rColor );
    void            ClearHighlightRanges();

    void            DoChartSelection( const css::uno::Sequence< css::chart2::data::HighlightedRange > & rHilightRanges );
    void            DoDPFieldPopup(Point aPoint, Size aSize);

    long            GetGridWidth( ScHSplitPos eWhich );
    long            GetGridHeight( ScVSplitPos eWhich );

    void            UpdateScrollBars();
    void            SetNewVisArea();

    void            InvalidateAttribs();

    void            OnLibreOfficeKitTabChanged();
    void            AddWindowToForeignEditView(SfxViewShell* pViewShell, ScSplitPos eWhich);
    void            RemoveWindowFromForeignEditView(SfxViewShell* pViewShell, ScSplitPos eWhich);
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
                                   bool bCols = false, bool bRows = false, bool bForceNeg = false );
    void            InitOwnBlockMode();
    void            DoneBlockMode( bool bContinue = false );

    bool            IsBlockMode() const;

    void            ExpandBlock(SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode);
    void            ExpandBlockPage(SCsCOL nMovX, SCsROW nMovY);
    void            ExpandBlockArea(SCsCOL nMovX, SCsROW nMovY);

    void            MarkColumns();
    void            MarkRows();

    /**
     * Called to select the specified full column.
     *
     * @param nCol: Column number to do operation on
     * @param nModifier: 0, KEY_SHIFT, KEY_MOD1, KEY_SHIFT | KEY_MOD1
     */

    void            MarkColumns(SCCOL nCol, sal_Int16 nModifier);
    /**
     * Called to select the specified full row.
     *
     * @param nRow: Row number to do operation on
     * @param nModifier: 0, KEY_SHIFT, KEY_MOD1, KEY_SHIFT | KEY_MOD1
     */
    void            MarkRows(SCROW nRow, sal_Int16 nModifier);

    void            MarkDataArea( bool bIncludeCursor = true );
    void            MarkMatrixFormula();
    void            Unmark();

    void            MarkRange( const ScRange& rRange, bool bSetCursor = true, bool bContinue = false );

    bool            IsMarking( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    void            PaintMarks( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    void            PaintBlock( bool bReset );

    void            SetMarkData( const ScMarkData& rNew );
    void            MarkDataChanged();

    void            LockModifiers( sal_uInt16 nModifiers );
    sal_uInt16          GetLockedModifiers() const;
    void            ViewOptionsHasChanged( bool bHScrollChanged,
                                           bool bGraphicsChanged);

    Point           GetMousePosPixel();

    void            FreezeSplitters( bool bFreeze, SplitMethod eSplitMethod = SC_SPLIT_METHOD_CURSOR );
    void            RemoveSplit();
    void            SplitAtCursor();
    void            SplitAtPixel( const Point& rPixel );
    void            InvalidateSplit();

    void            ErrorMessage( sal_uInt16 nGlobStrId );

    void            EnableRefInput(bool bFlag);

    vcl::Window*         GetFrameWin() const { return pFrameWin; }

    bool            HasPaintBrush() const           { return pBrushDocument || pDrawBrushSet; }
    ScDocument*     GetBrushDocument() const        { return pBrushDocument; }
    SfxItemSet*     GetDrawBrushSet() const         { return pDrawBrushSet; }
    bool            IsPaintBrushLocked() const      { return bLockPaintBrush; }
    void            SetBrushDocument( ScDocument* pNew, bool bLock );
    void            SetDrawBrushSet( SfxItemSet* pNew, bool bLock );
    void            ResetBrushDocument();

    bool ContinueOnlineSpelling();
    void EnableAutoSpell( bool bEnable );
    void ResetAutoSpell();
    void SetAutoSpellData( SCCOL nPosX, SCROW nPosY, const std::vector<editeng::MisspellRanges>* pRanges );
    /// @see ScModelObj::getRowColumnHeaders().
    OUString getRowColumnHeaders(const Rectangle& rRectangle);
    static void OnLOKNoteStateChanged(const ScPostIt* pNote);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
