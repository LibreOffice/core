/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabview.hxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 15:33:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_TABVIEW_HXX
#define SC_TABVIEW_HXX

#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

//REMOVE    #ifndef SO2_DECL_SVINPLACECLIENT_DEFINED
//REMOVE    #define SO2_DECL_SVINPLACECLIENT_DEFINED
//REMOVE    SO2_DECL_REF(SvInPlaceClient)
//REMOVE    #endif

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
    BOOL            bAdd;

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
public:
                    ScCornerButton( Window* pParent, ScViewData* pData, BOOL bAdditional );
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

    BOOL                bDrawSelMode;           // nur Zeichenobjekte selektieren ?

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
    BOOL                bLockPaintBrush;        // keep for more than one use?

    Timer               aScrollTimer;
    ScGridWindow*       pTimerWindow;
    MouseEvent          aTimerMEvt;

    ULONG               nTipVisible;

    BOOL                bDragging;              // fuer Scrollbars
    long                nPrevDragPos;

    BOOL                bIsBlockMode;           // Block markieren
    BOOL                bBlockNeg;              // wird Markierung aufgehoben?
    BOOL                bBlockCols;             // werden ganze Spalten markiert?
    BOOL                bBlockRows;             // werden ganze Zeilen markiert?

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

    BOOL                bMinimized;
    BOOL                bInUpdateHeader;
    BOOL                bInActivatePart;
    BOOL                bInZoomUpdate;
    BOOL                bMoveIsShift;
    BOOL                bNewStartIfMarking;

    BOOL                bOldSelection;          // old style (inverting) of selection


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

    static void     SetScrollBar( ScrollBar& rScroll, long nRangeMax, long nVisible, long nPos, BOOL bLayoutRTL );
    static long     GetScrollBarPos( ScrollBar& rScroll, BOOL bLayoutRTL );

protected:
    void            UpdateHeaderWidth( const ScVSplitPos* pWhich = NULL,
                                        const SCROW* pPosY = NULL );

    void            HideTip();
    void            ShowRefTip();

    void            ZoomChanged();
    void            UpdateShow();
    void            GetBorderSize( SvBorder& rBorder, const Size& rSize );

    void            ResetDrawDragMode();
    BOOL            IsDrawTextEdit() const;
    void            DrawEnableAnim(BOOL bSet);
    //HMHvoid           DrawShowMarkHdl(BOOL bShow);

    void            MakeDrawView( BYTE nForceDesignMode = SC_FORCEMODE_NONE );

    void            HideNoteMarker();

    void            UpdateIMap( SdrObject* pObj );

public:
                    ScTabView( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                    ScTabView( Window* pParent, const ScTabView& rScTabView, ScTabViewShell* pViewShell );
                    ~ScTabView();

    void            MakeDrawLayer();

    void            HideListBox();

    BOOL            HasHintWindow() const   { return pInputHintWindow != NULL; }
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

    void            DoResize( const Point& rOffset, const Size& rSize, BOOL bInner = FALSE );
    void            RepeatResize( BOOL bUpdateFix = TRUE );
    void            UpdateFixPos();
    Point           GetGridOffset() const;

    BOOL            IsDrawSelMode() const       { return bDrawSelMode; }
    void            SetDrawSelMode(BOOL bNew)   { bDrawSelMode = bNew; }

    void            SetDrawFuncPtr(FuPoor* pFuncPtr)    { pDrawActual = pFuncPtr; }
    void            SetDrawFuncOldPtr(FuPoor* pFuncPtr) { pDrawOld = pFuncPtr; }
    FuPoor*         GetDrawFuncPtr()                    { return pDrawActual; }
    FuPoor*         GetDrawFuncOldPtr()                 { return pDrawOld; }

    void            DrawDeselectAll();
    void            DrawMarkListHasChanged();
    void            UpdateAnchorHandles();
    String          GetSelectedChartName() const;

    ScPageBreakData* GetPageBreakData()     { return pPageBreakData; }
    ScHighlightRanges* GetHighlightRanges() { return pHighlightRanges; }

    void            UpdatePageBreakData( BOOL bForcePaint = FALSE );

    void            DrawMarkRect( const Rectangle& rRect );

    ScViewData*         GetViewData()       { return &aViewData; }
    const ScViewData*   GetViewData() const { return &aViewData; }

    ScViewFunctionSet*      GetFunctionSet()    { return &aFunctionSet; }
    ScViewSelectionEngine*  GetSelEngine()      { return pSelEngine; }

    BOOL            SelMouseButtonDown( const MouseEvent& rMEvt );

    ScDrawView*     GetScDrawView()         { return pDrawView; }
    SdrView*        GetSdrView();           // gegen CLOKs

    BOOL            IsMinimized() const     { return bMinimized; }

    void            TabChanged();
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY, BOOL bAll );
    void            RefreshZoom();
    void            SetPagebreakMode( BOOL bSet );

    void            UpdateLayerLocks();

    void            UpdateDrawTextOutliner();
    void            DigitLanguageChanged();

    void            UpdateInputLine();

    void            InitRefMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScRefType eType,
                                    BOOL bPaint = TRUE );
    void            DoneRefMode( BOOL bContinue = FALSE );
    void            UpdateRef( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ );
    void            StopRefMode();

    void            EndSelection();
    void            StopMarking();
    void            FakeButtonUp( ScSplitPos eWhich );

    Window*         GetActiveWin();
    Window*         GetWindowByPos( ScSplitPos ePos );

    ScSplitPos      FindWindow( Window* pWindow ) const;

    void            SetActivePointer( const Pointer& rPointer );
    void            SetActivePointer( const ResId& rId );

    void            ActiveGrabFocus();
    void            ActiveCaptureMouse();
    void            ActiveReleaseMouse();
    Point           ActivePixelToLogic( const Point& rDevicePoint );

    void            ClickCursor( SCCOL nPosX, SCROW nPosY, BOOL bControl );

    SC_DLLPUBLIC void           SetCursor( SCCOL nPosX, SCROW nPosY, BOOL bNew = FALSE );

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
    void            SetZoomType( SvxZoomType eNew, BOOL bAll );
    USHORT          CalcZoom( SvxZoomType eType, USHORT nOldZoom );

//  void            CalcZoom( SvxZoomType eType, USHORT& rZoom, SCCOL& rCol, SCROW& rRow );

    sal_Bool        HasPageFieldDataAtCursor() const;
    void            StartDataSelect();

                    //  MoveCursorAbs       - absolut
                    //  MoveCursorRel       - einzelne Zellen
                    //  MoveCursorPage      - Bildschirmseite
                    //  MoveCursorArea      - Datenblock
                    //  MoveCursorEnd       - links oben / benutzter Bereich

    SC_DLLPUBLIC void           MoveCursorAbs( SCsCOL nCurX, SCsROW nCurY, ScFollowMode eMode,
                                    BOOL bShift, BOOL bControl,
                                    BOOL bKeepOld = FALSE, BOOL bKeepSel = FALSE );
    void            MoveCursorRel( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    BOOL bShift, BOOL bKeepSel = FALSE );
    void            MoveCursorPage( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    BOOL bShift, BOOL bKeepSel = FALSE );
    void            MoveCursorArea( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    BOOL bShift, BOOL bKeepSel = FALSE );
    void            MoveCursorEnd( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                    BOOL bShift, BOOL bKeepSel = FALSE );
    void            MoveCursorScreen( SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode, BOOL bShift );

    void            MoveCursorEnter( BOOL bShift );     // Shift fuer Richtung (kein Markieren)

    BOOL            MoveCursorKeyInput( const KeyEvent& rKeyEvent );

    void            FindNextUnprot( BOOL bShift, BOOL bInSelection = TRUE );

    void            SetNewStartIfMarking();

    void            SetTabNo( SCTAB nTab, BOOL bNew = FALSE, BOOL bExtendSelection = FALSE );
    void            SelectNextTab( short nDir, BOOL bExtendSelection = FALSE );

    void            ActivateView( BOOL bActivate, BOOL bFirst );
    void            ActivatePart( ScSplitPos eWhich );
    BOOL            IsInActivatePart() const    { return bInActivatePart; }

    void            SetTimer( ScGridWindow* pWin, const MouseEvent& rMEvt );
    void            ResetTimer();

    void            ScrollX( long nDeltaX, ScHSplitPos eWhich, BOOL bUpdBars = TRUE );
    void            ScrollY( long nDeltaY, ScVSplitPos eWhich, BOOL bUpdBars = TRUE );
    SC_DLLPUBLIC void           ScrollLines( long nDeltaX, long nDeltaY );              // aktives

    BOOL            ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos );

    void            ScrollToObject( SdrObject* pDrawObj );
    void            MakeVisible( const Rectangle& rHMMRect );

                                    // Zeichnen

    void            InvertBlockMark(SCCOL nBlockStartX, SCROW nBlockStartY,
                                SCCOL nBlockEndX, SCROW nBlockEndY);

    void            DrawDragRect( SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY,
                                    ScSplitPos ePos );

    void            PaintCell( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void            PaintArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                        ScUpdateMode eMode = SC_UPDATE_ALL );

    void            PaintGrid();

    void            PaintTopCol( SCCOL nCol );
    void            PaintTopArea( SCCOL nStartCol, SCCOL nEndCol );
    void            PaintTop();

    void            PaintLeftRow( SCROW nRow );
    void            PaintLeftArea( SCROW nStartRow, SCROW nEndRow );
    void            PaintLeft();

    BOOL            PaintExtras();

    void            RecalcPPT();

    void            CreateAnchorHandles(SdrHdlList& rHdl, const ScAddress& rAddress);

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
    void            KillEditView( BOOL bNoPaint );
    void            UpdateEditView();


                                    //  Bloecke

    void            SelectAll( BOOL bContinue = FALSE );
    void            SelectAllTables();

    void            MarkCursor( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                BOOL bCols = FALSE, BOOL bRows = FALSE, BOOL bCellSelection = FALSE );
    void            InitBlockMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                    BOOL bTestNeg = FALSE,
                                    BOOL bCols = FALSE, BOOL bRows = FALSE );
    void            InitOwnBlockMode();
    void            DoneBlockMode( BOOL bContinue = FALSE );

    BOOL            IsBlockMode() const     { return bIsBlockMode; }

    void            MarkColumns();
    void            MarkRows();
    void            MarkDataArea( BOOL bIncludeCursor = TRUE );
    void            MarkMatrixFormula();
    void            Unmark();

    void            MarkRange( const ScRange& rRange, BOOL bSetCursor = TRUE, BOOL bContinue = FALSE );

    BOOL            IsMarking( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    void            PaintMarks( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow );
    void            PaintBlock( BOOL bReset = FALSE );

    void            SetMarkData( const ScMarkData& rNew );
    void            MarkDataChanged();

    void            LockModifiers( USHORT nModifiers );
    USHORT          GetLockedModifiers() const;
    void            ViewOptionsHasChanged( BOOL bHScrollChanged,
                                            BOOL bGraphicsChanged = FALSE);

    Point           GetMousePosPixel();

    void            SnapSplitPos( Point& rScreenPosPixel );
    void            FreezeSplitters( BOOL bFreeze );
    void            RemoveSplit();
    void            SplitAtCursor();
    void            SplitAtPixel( const Point& rPixel, BOOL bHor, BOOL bVer );
    void            InvalidateSplit();

    void            ErrorMessage( USHORT nGlobStrId );
    Window*         GetParentOrChild( USHORT nChildId );

    void            EnableRefInput(BOOL bFlag=TRUE);

    Window*         GetFrameWin() const { return pFrameWin; }

    BOOL            HasPaintBrush() const           { return pBrushDocument || pDrawBrushSet; }
    ScDocument*     GetBrushDocument() const        { return pBrushDocument; }
    SfxItemSet*     GetDrawBrushSet() const         { return pDrawBrushSet; }
    BOOL            IsPaintBrushLocked() const      { return bLockPaintBrush; }
    void            SetBrushDocument( ScDocument* pNew, BOOL bLock );
    void            SetDrawBrushSet( SfxItemSet* pNew, BOOL bLock );
    void            ResetBrushDocument();

    void            UpdateSelectionType();
    BOOL            IsOldSelection() const          { return bOldSelection; }
};



#endif

