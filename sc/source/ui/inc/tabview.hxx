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

#include <array>
#include <memory>
#include <svtools/scrolladaptor.hxx>
#include <vcl/help.hxx>

#include "hiranges.hxx"
#include "viewutil.hxx"
#include "select.hxx"
#include "gridwin.hxx"
#include "drawview.hxx"

namespace editeng {
    struct MisspellRanges;
}

class ScEditEngineDefaulter;
class ScOutlineWindow;
class ScRowBar;
class ScColBar;
class ScTabControl;
class ScTabViewShell;
struct ScRangeFindData;
class SvBorder;
class FuPoor;
class Splitter;
class ScTabSplitter;
class SdrView;
class SdrObject;
class ScPageBreakData;
class SdrHdlList;
class TabBar;
namespace com::sun::star::chart2::data { struct HighlightedRange; }
namespace tools { class JsonWriter; }

enum HeaderType
{
    COLUMN_HEADER,
    ROW_HEADER,
    BOTH_HEADERS
};

//      Help - Window

class ScCornerButton : public vcl::Window
{
private:
    ScViewData*     pViewData;

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
public:
                    ScCornerButton( vcl::Window* pParent, ScViewData* pData );
                    virtual ~ScCornerButton() override;

    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
};

class ScExtraEditViewManager
{
private:
    enum ModifierTagType { Adder, Remover };

public:
    ScExtraEditViewManager(ScTabViewShell* pThisViewShell, std::array<VclPtr<ScGridWindow>, 4> const & pGridWin)
        : mpThisViewShell(pThisViewShell)
        , mpGridWin(pGridWin)
        , mpOtherEditView(nullptr)
        , nTotalWindows(0)
    {}

    ~ScExtraEditViewManager();

    void Add(SfxViewShell* pViewShell, ScSplitPos eWhich);

    void Remove(SfxViewShell* pViewShell, ScSplitPos eWhich);

private:
    template<ModifierTagType ModifierTag>
    void Apply(SfxViewShell* pViewShell, ScSplitPos eWhich);

    template<ModifierTagType ModifierTag>
    void Modifier(ScGridWindow* pWin);

private:
    ScTabViewShell* mpThisViewShell;
    std::array<VclPtr<ScGridWindow>, 4> const & mpGridWin;
    EditView* mpOtherEditView;
    int nTotalWindows;
};

class ScTabView
{
private:
    enum BlockMode { None = 0, Normal = 1, Own = 2 };

    VclPtr<vcl::Window>             pFrameWin;              // First !!!
    ScViewData          aViewData;              // must be at the front !
    ScViewRenderingOptions aViewRenderingData;

    std::unique_ptr<ScViewSelectionEngine> pSelEngine;
    ScViewFunctionSet       aFunctionSet;

    std::unique_ptr<ScHeaderSelectionEngine> pHdrSelEng;
    ScHeaderFunctionSet      aHdrFunc;

    std::unique_ptr<ScDrawView> pDrawView;

    Size                aFrameSize;             // passed on as for DoResize
    Point               aBorderPos;

    // The ownership of these two is rather weird. we seem to need
    // to keep an old copy alive for some period of time to avoid crashing.
    FuPoor*             pDrawActual;
    FuPoor*             pDrawOld;

    std::shared_ptr<weld::MessageDialog> m_xMessageBox;

    std::array<VclPtr<ScGridWindow>, 4> pGridWin;
    std::array<VclPtr<ScColBar>, 2> pColBar;
    std::array<VclPtr<ScRowBar>, 2> pRowBar;
    std::array<VclPtr<ScOutlineWindow>, 2> pColOutline;
    std::array<VclPtr<ScOutlineWindow>, 2> pRowOutline;
    VclPtr<ScTabSplitter>       pHSplitter;
    VclPtr<ScTabSplitter>       pVSplitter;
    VclPtr<ScTabControl>        pTabControl;
    VclPtr<ScrollAdaptor>       aVScrollTop;
    VclPtr<ScrollAdaptor>       aVScrollBottom;         // initially visible
    VclPtr<ScrollAdaptor>       aHScrollLeft;           // initially visible
    VclPtr<ScrollAdaptor>       aHScrollRight;
    VclPtr<ScCornerButton>      aCornerButton;
    VclPtr<ScCornerButton>      aTopButton;

    std::shared_ptr<sc::SpellCheckContext> mpSpellCheckCxt;

    std::unique_ptr<sdr::overlay::OverlayObjectList> mxInputHintOO; // help hint for data validation

    std::unique_ptr<ScPageBreakData>  pPageBreakData;
    std::vector<ScHighlightEntry>     maHighlightRanges;

    ScDocumentUniquePtr               pBrushDocument;         // cell formats for format paint brush
    std::unique_ptr<SfxItemSet>       pDrawBrushSet;          // drawing object attributes for paint brush

    Timer               aScrollTimer;
    VclPtr<ScGridWindow>       pTimerWindow;
    MouseEvent          aTimerMEvt;

    ScExtraEditViewManager aExtraEditViewManager;

    void*                   nTipVisible;
    tools::Rectangle               aTipRectangle;
    QuickHelpFlags          nTipAlign;
    OUString                sTipString;
    VclPtr<vcl::Window>     sTopParent;

    tools::Long                nPrevDragPos;

    BlockMode           meBlockMode;           // Marks block
    BlockMode           meHighlightBlockMode;  // Highlight row/col

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

    SCROW               mnLOKStartHeaderRow;
    SCROW               mnLOKEndHeaderRow;
    SCCOL               mnLOKStartHeaderCol;
    SCCOL               mnLOKEndHeaderCol;

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

    double              mfLastZoomScale = 0;
    double              mfAccumulatedZoom = 0;
    tools::Long         mnPendingaHScrollLeftDelta = 0;
    tools::Long         mnPendingaHScrollRightDelta = 0;

    void            Init();

    void            DoAddWin( ScGridWindow* pWin );

    void            InitScrollBar(ScrollAdaptor& rScrollBar, tools::Long nMaxVal, const Link<weld::Scrollbar&, void>& rLink);
    DECL_LINK(HScrollLeftHdl, weld::Scrollbar&, void );
    DECL_LINK(HScrollRightHdl, weld::Scrollbar&, void );
    DECL_LINK(VScrollTopHdl, weld::Scrollbar&, void );
    DECL_LINK(VScrollBottomHdl, weld::Scrollbar&, void );
    DECL_LINK(EndScrollHdl, const MouseEvent&, bool);
    void ScrollHdl(ScrollAdaptor* rScrollBar);

    DECL_LINK(SplitHdl, Splitter*, void);
    void            DoHSplit(tools::Long nSplitPos);
    void            DoVSplit(tools::Long nSplitPos);

    DECL_LINK( TimerHdl, Timer*, void );

    void            UpdateVarZoom();

    static void     SetScrollBar( ScrollAdaptor& rScroll, tools::Long nRangeMax, tools::Long nVisible, tools::Long nPos, bool bLayoutRTL );
    static tools::Long     GetScrollBarPos( const ScrollAdaptor& rScroll, bool bLayoutRTL );

    void            GetAreaMoveEndPosition(SCCOL nMovX, SCROW nMovY, ScFollowMode eMode,
                                           SCCOL& rAreaX, SCROW& rAreaY, ScFollowMode& rMode,
                                           bool bInteractiveByUser = false);

    void            SkipCursorHorizontal(SCCOL& rCurX, SCROW& rCurY, SCCOL nOldX, SCCOL nMovX);
    void            SkipCursorVertical(SCCOL& rCurX, SCROW& rCurY, SCROW nOldY, SCROW nMovY);

    /**
     *
     *  @brief Update marks for a selected Range. This is a helper function
     *  for PaintRangeFinder.
     *
     *  @param pData: Range to update for painting.
     *  @param nTab: Current tab.
     *
     **/

    void            PaintRangeFinderEntry (const ScRangeFindData* pData, SCTAB nTab);

    void            SetZoomPercentFromCommand(sal_uInt16 nZoomPercent);

    DECL_STATIC_LINK(ScTabView, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*);

    void            UpdateHighlightOverlay();
    void            ImplTabChanged(bool bSameTabButMoved);

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

    enum SplitMethod { SC_SPLIT_METHOD_COL, SC_SPLIT_METHOD_ROW, SC_SPLIT_METHOD_CURSOR };

    void            MakeDrawLayer();

    void            HideListBox();

    bool            HasHintWindow() const;
    void            RemoveHintWindow();
    void            TestHintWindow();

    DECL_LINK( TabBarResize, ::TabBar*, void );
    /** Sets an absolute tab bar width (in pixels). */
    void            SetTabBarWidth( tools::Long nNewWidth );
    /** Sets a relative tab bar width.
        @param fRelTabBarWidth  Tab bar width relative to frame window width (0.0 ... 1.0). */
    SC_DLLPUBLIC void            SetRelTabBarWidth( double fRelTabBarWidth );
    /** Sets a relative tab bar width. Tab bar is resized again in next DoResize().
        @param fRelTabBarWidth  Tab bar width relative to frame window width (0.0 ... 1.0). */
    void            SetPendingRelTabBarWidth( double fRelTabBarWidth );
    /** Returns the current tab bar width in pixels. */
    tools::Long            GetTabBarWidth() const;
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

    ScPageBreakData* GetPageBreakData()     { return pPageBreakData.get(); }
    const std::vector<ScHighlightEntry>& GetHighlightRanges() const { return maHighlightRanges; }

    void            UpdatePageBreakData( bool bForcePaint = false );

    ScViewData&         GetViewData()       { return aViewData; }
    const ScViewData&   GetViewData() const { return aViewData; }

    const ScViewRenderingOptions& GetViewRenderingData() const { return aViewRenderingData; }
    void SetViewRenderingData(const ScViewRenderingOptions& rViewRenderingData) { aViewRenderingData = rViewRenderingData; }

    ScViewFunctionSet&      GetFunctionSet()    { return aFunctionSet; }
    ScViewSelectionEngine*  GetSelEngine()      { return pSelEngine.get(); }

    bool            SelMouseButtonDown( const MouseEvent& rMEvt );

    ScDrawView*     GetScDrawView()         { return pDrawView.get(); }

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
    vcl::Window*    GetWindowByPos( ScSplitPos ePos ) const { return pGridWin[ePos]; }

    ScSplitPos      FindWindow( const vcl::Window* pWindow ) const;

    void            SetActivePointer( PointerStyle nPointer );

    void            ActiveGrabFocus();

    void            ClickCursor( SCCOL nPosX, SCROW nPosY, bool bControl );

    SC_DLLPUBLIC void SetCursor( SCCOL nPosX, SCROW nPosY, bool bNew = false );

    SC_DLLPUBLIC void           CellContentChanged();
    void            SelectionChanged( bool bFromPaste = false );
    void            CursorPosChanged();
    void            UpdateInputContext();

    void            CheckSelectionTransfer();

    void            InvertHorizontal( ScVSplitPos eWhich, tools::Long nDragPos );
    void            InvertVertical( ScHSplitPos eWhich, tools::Long nDragPos );

    Point           GetInsertPos() const;

    Point           GetChartInsertPos( const Size& rSize, const ScRange& rCellRange );
    Point           GetChartDialogPos( const Size& rDialogSize, const tools::Rectangle& rLogicChart );

    void            UpdateAutoFillMark( bool bFromPaste = false );

    void            ShowCursor();
    void            HideAllCursors();
    void            ShowAllCursors();

    void            AlignToCursor( SCCOL nCurX, SCROW nCurY, ScFollowMode eMode,
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

    SC_DLLPUBLIC void MoveCursorAbs( SCCOL nCurX, SCROW nCurY, ScFollowMode eMode,
                                     bool bShift, bool bControl,
                                     bool bKeepOld = false, bool bKeepSel = false );
    void            MoveCursorRel( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode,
                                   bool bShift, bool bKeepSel = false );
    void            MoveCursorPage( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode,
                                    bool bShift, bool bKeepSel = false );
    void            MoveCursorArea( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode,
                                    bool bShift, bool bKeepSel = false,
                                    bool bInteractiveByUser = false );
    void            MoveCursorEnd( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode,
                                   bool bShift, bool bKeepSel = false );
    void            MoveCursorScreen( SCCOL nMovX, SCROW nMovY, ScFollowMode eMode, bool bShift );

    void            MoveCursorEnter( bool bShift );     // Shift for direction (select nothing)

    bool            MoveCursorKeyInput( const KeyEvent& rKeyEvent );

    void            FindNextUnprot( bool bShift, bool bInSelection );

    void            GetPageMoveEndPosition(SCCOL nMovX, SCROW nMovY, SCCOL& rPageX, SCROW& rPageY);

    SC_DLLPUBLIC void SetTabNo( SCTAB nTab, bool bNew = false, bool bExtendSelection = false, bool bSameTabButMoved = false );
    void            SelectNextTab( short nDir, bool bExtendSelection );
    void            SelectTabPage( const sal_uInt16 nTab );

    void            ActivateView( bool bActivate, bool bFirst );
    void            ActivatePart( ScSplitPos eWhich );
    bool            IsInActivatePart() const    { return bInActivatePart; }

    void            SetTimer( ScGridWindow* pWin, const MouseEvent& rMEvt );
    void            ResetTimer();

    void            ScrollX( tools::Long nDeltaX, ScHSplitPos eWhich, bool bUpdBars = true );
    void            ScrollY( tools::Long nDeltaY, ScVSplitPos eWhich, bool bUpdBars = true );
    SC_DLLPUBLIC void           ScrollLines( tools::Long nDeltaX, tools::Long nDeltaY );              // active

    bool            ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos );
    bool            GestureZoomCommand(const CommandEvent& rCEvt);

    void            ScrollToObject( const SdrObject* pDrawObj );
    void            MakeVisible( const tools::Rectangle& rHMMRect );

                                    // Drawing

    void            PaintArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                               ScUpdateMode eMode = ScUpdateMode::All,
                               tools::Long nMaxWidthAffectedHint = -1 );

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

    void            UpdateFormulas( SCCOL nStartCol = -1, SCROW nStartRow = -1, SCCOL nEndCol = -1, SCROW nEndRow = -1 );
    void            InterpretVisible();
    void            CheckNeedsRepaint();
    bool            NeedsRepaint();

    void            PaintRangeFinder( tools::Long nNumber );
    void            AddHighlightRange( const ScRange& rRange, const Color& rColor );
    void            ClearHighlightRanges();

    void            DoChartSelection( const css::uno::Sequence< css::chart2::data::HighlightedRange > & rHilightRanges );
    void            DoDPFieldPopup(std::u16string_view rPivotTableName, sal_Int32 nDimensionIndex, Point aPoint, Size aSize);

    tools::Long            GetGridWidth( ScHSplitPos eWhich );
    tools::Long            GetGridHeight( ScVSplitPos eWhich );

    void            UpdateScrollBars( HeaderType eHeaderType = BOTH_HEADERS );
    void            SetNewVisArea();
    void            SetTabProtectionSymbol( SCTAB nTab, const bool bProtect ); // for protection icon of a tab on tabbar

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
    void            InitOwnBlockMode( const ScRange& rMarkRange );
    void            DoneBlockMode( bool bContinue = false );
    void            InitBlockModeHighlight( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, bool bCols, bool bRows );
    void            DoneBlockModeHighlight( bool bContinue );

    bool            IsBlockMode() const;

    void            ExpandBlock(SCCOL nMovX, SCROW nMovY, ScFollowMode eMode);
    void            ExpandBlockPage(SCCOL nMovX, SCROW nMovY);
    void            ExpandBlockArea(SCCOL nMovX, SCROW nMovY);

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
    void            HighlightOverlay();

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

    void            FreezeSplitters( bool bFreeze, SplitMethod eSplitMethod = SC_SPLIT_METHOD_CURSOR, SCCOLROW nFreezeIndex = -1 );
    void            RemoveSplit();
    void            SplitAtCursor();
    void            SplitAtPixel( const Point& rPixel );
    void            InvalidateSplit();

    void            ErrorMessage(TranslateId pGlobStrId);

    void            EnableRefInput(bool bFlag);

    vcl::Window*         GetFrameWin() const { return pFrameWin; }

    bool            HasPaintBrush() const           { return pBrushDocument || pDrawBrushSet; }
    ScDocument*     GetBrushDocument() const        { return pBrushDocument.get(); }
    SfxItemSet*     GetDrawBrushSet() const         { return pDrawBrushSet.get(); }
    bool            IsPaintBrushLocked() const      { return bLockPaintBrush; }
    void            SetBrushDocument( ScDocumentUniquePtr pNew, bool bLock );
    void            SetDrawBrushSet( std::unique_ptr<SfxItemSet> pNew, bool bLock );
    void            ResetBrushDocument();

    SC_DLLPUBLIC bool IsAutoSpell() const;
    void EnableAutoSpell( bool bEnable );
    void ResetAutoSpell();
    void ResetAutoSpellForContentChange();
    void SetAutoSpellData( SCCOL nPosX, SCROW nPosY, const std::vector<editeng::MisspellRanges>* pRanges );
    /// @see ScModelObj::getRowColumnHeaders().
    void getRowColumnHeaders(const tools::Rectangle& rRectangle, tools::JsonWriter& rJsonWriter);
    /// @see ScModelObj::getSheetGeometryData()
    OString getSheetGeometryData(bool bColumns, bool bRows, bool bSizes, bool bHidden,
                                 bool bFiltered, bool bGroups);
    void extendTiledAreaIfNeeded();

    static void OnLOKNoteStateChanged(const ScPostIt* pNote);

    SCROW GetLOKStartHeaderRow() const { return mnLOKStartHeaderRow; }
    SCROW GetLOKEndHeaderRow() const { return mnLOKEndHeaderRow; }
    SCCOL GetLOKStartHeaderCol() const { return mnLOKStartHeaderCol; }
    SCCOL GetLOKEndHeaderCol() const { return mnLOKEndHeaderCol; }

    void SyncGridWindowMapModeFromDrawMapMode();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
