/*************************************************************************
 *
 *  $RCSfile: tabview.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_TABVIEW_HXX
#define SC_TABVIEW_HXX

#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif

#ifndef _IPCLIENT_HXX //autogen
#include <so3/ipclient.hxx>
#endif

#ifndef SO2_DECL_SVINPLACECLIENT_DEFINED
#define SO2_DECL_SVINPLACECLIENT_DEFINED
SO2_DECL_REF(SvInPlaceClient)
#endif

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
class SchMemChart;
struct ChartSelectionInfo;
class SdrHdlList;

#define SPLIT_HANDLE_SIZE   3

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

    SvInPlaceClientRef  aIPClient;

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
    USHORT              nBlockStartX;
    USHORT              nBlockEndX;
    USHORT              nBlockStartY;
    USHORT              nBlockEndY;
    USHORT              nBlockStartZ;
    USHORT              nBlockEndZ;

    SvxZoomType         eZoomType;
    BOOL                bMinimized;
    BOOL                bInUpdateHeader;
    BOOL                bInActivatePart;
    BOOL                bInZoomUpdate;
    BOOL                bMoveIsShift;


    void            Init();

    void            DoAddWin( ScGridWindow* pWin );

    void            InitScrollBar( ScrollBar& rScrollBar, long nMaxVal );
    DECL_LINK(      ScrollHdl, ScrollBar* );
    DECL_LINK(      EndScrollHdl, ScrollBar* );

    DECL_LINK(      SplitHdl, Splitter* );
    void            DoHSplit(long nSplitPos);
    void            DoVSplit(long nSplitPos);

    DECL_LINK(      TimerHdl, Timer* );

    void            UpdateHeaderWidth( const ScVSplitPos* pWhich = NULL,
                                        const USHORT* pPosY = NULL );
    void            UpdateVarZoom();

protected:
    void            HideTip();
    void            ShowRefTip();

    void            ZoomChanged();
    void            UpdateShow();
    void            GetBorderSize( SvBorder& rBorder, const Size& rSize );

    void            ResetDrawDragMode();
    BOOL            IsDrawTextEdit() const;
    void            DrawEnableAnim(BOOL bSet);
    void            DrawShowMarkHdl(BOOL bShow);

    void            MakeDrawView();

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
    void            SetTabBarWidth( long nNewWidth );
    long            GetTabBarWidth();

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
    void            SetZoom( const Fraction& rNewX, const Fraction& rNewY );
    void            SetPagebreakMode( BOOL bSet );

    void            UpdateLayerLocks();

    void            UpdateDrawTextOutliner();

    void            UpdateInputLine();

    void            InitRefMode( USHORT nCurX, USHORT nCurY, USHORT nCurZ, ScRefType eType,
                                    BOOL bPaint = TRUE );
    void            DoneRefMode( BOOL bContinue = FALSE );
    void            UpdateRef( USHORT nCurX, USHORT nCurY, USHORT nCurZ );
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

    void            ClickCursor( USHORT nPosX, USHORT nPosY, BOOL bControl );

    void            SetCursor( USHORT nPosX, USHORT nPosY, BOOL bNew = FALSE );

    void            CellContentChanged();
    void            SelectionChanged();
    void            CursorPosChanged();

    void            InvertHorizontal( ScVSplitPos eWhich, long nDragPos );
    void            InvertVertical( ScHSplitPos eWhich, long nDragPos );

    Point           GetInsertPos();

    void            UpdateAutoFillMark();

    void            HideCursor();               // nur aktiver Teil
    void            ShowCursor();
    void            HideAllCursors();
    void            ShowAllCursors();

    void            AlignToCursor( short nCurX, short nCurY, ScFollowMode eMode,
                                    const ScSplitPos* pWhich = NULL );

    SvxZoomType     GetZoomType() const             { return eZoomType; }
    void            SetZoomType( SvxZoomType eNew ) { eZoomType = eNew; }
    USHORT          CalcZoom( SvxZoomType eType, USHORT nOldZoom );

//  void            CalcZoom( SvxZoomType eType, USHORT& rZoom, USHORT& rCol, USHORT& rRow );

    void            StartDataSelect();

                    //  MoveCursorAbs       - absolut
                    //  MoveCursorRel       - einzelne Zellen
                    //  MoveCursorPage      - Bildschirmseite
                    //  MoveCursorArea      - Datenblock
                    //  MoveCursorEnd       - links oben / benutzter Bereich

    void            MoveCursorAbs( short nCurX, short nCurY, ScFollowMode eMode,
                                    BOOL bShift, BOOL bControl,
                                    BOOL bKeepOld = FALSE, BOOL bKeepSel = FALSE );
    void            MoveCursorRel( short nMovX, short nMovY, ScFollowMode eMode,
                                    BOOL bShift, BOOL bKeepSel = FALSE );
    void            MoveCursorPage( short nMovX, short nMovY, ScFollowMode eMode, BOOL bShift );
    void            MoveCursorArea( short nMovX, short nMovY, ScFollowMode eMode, BOOL bShift );
    void            MoveCursorEnd( short nMovX, short nMovY, ScFollowMode eMode, BOOL bShift );
    void            MoveCursorScreen( short nMovX, short nMovY, ScFollowMode eMode, BOOL bShift );

    void            MoveCursorEnter( BOOL bShift );     // Shift fuer Richtung (kein Markieren)

    void            FindNextUnprot( BOOL bShift, BOOL bInSelection = TRUE );

    void            SetTabNo( USHORT nTab, BOOL bNew = FALSE );
    void            SelectNextTab( short nDir );

    void            ActivateView( BOOL bActivate, BOOL bFirst );
    void            ActivatePart( ScSplitPos eWhich );
    BOOL            IsInActivatePart() const    { return bInActivatePart; }

    void            SetTimer( ScGridWindow* pWin, const MouseEvent& rMEvt );
    void            ResetTimer();

    void            ScrollX( long nDeltaX, ScHSplitPos eWhich, BOOL bUpdBars = TRUE );
    void            ScrollY( long nDeltaY, ScVSplitPos eWhich, BOOL bUpdBars = TRUE );
    void            ScrollLines( long nDeltaX, long nDeltaY );              // aktives

    BOOL            ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos );

    void            ScrollToObject( SdrObject* pDrawObj );
    void            MakeVisible( const Rectangle& rHMMRect );

                                    // Zeichnen

    void            InvertBlockMark(USHORT nBlockStartX, USHORT nBlockStartY,
                                USHORT nBlockEndX, USHORT nBlockEndY);

    void            DrawDragRect( USHORT nStartX, USHORT nStartY, USHORT nEndX, USHORT nEndY,
                                    ScSplitPos ePos );

    void            PaintCell( USHORT nCol, USHORT nRow, USHORT nTab );
    void            PaintArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                        ScUpdateMode eMode = SC_UPDATE_ALL );

    void            PaintGrid();

    void            PaintTopCol( USHORT nCol );
    void            PaintTopArea( USHORT nStartCol, USHORT nEndCol );
    void            PaintTop();

    void            PaintLeftRow( USHORT nRow );
    void            PaintLeftArea( USHORT nStartRow, USHORT nEndRow );
    void            PaintLeft();

    BOOL            PaintExtras();

    void            CreateAnchorHandles(SdrHdlList& rHdl, const ScAddress& rAddress);

    void            UpdateFormulas();
    void            InterpretVisible();
    void            CheckNeedsRepaint();

    void            PaintRangeFinder( long nNumber = -1 );
    void            AddHighlightRange( const ScRange& rRange, const Color& rColor );
    void            ClearHighlightRanges();

    long            DoChartSelection( ChartSelectionInfo &rInfo, const SchMemChart& rMemChart );

    long            GetGridWidth( ScHSplitPos eWhich );
    long            GetGridHeight( ScVSplitPos eWhich );

    void            UpdateScrollBars();
    void            SetNewVisArea();

    void            InvalidateAttribs();

    void            MakeEditView( ScEditEngineDefaulter* pEngine, USHORT nCol, USHORT nRow );
    void            KillEditView( BOOL bNoPaint );
    void            UpdateEditView();


                                    //  Bloecke

    void            SelectAll( BOOL bContinue = FALSE );
    void            SelectAllTables();

    void            MarkCursor( USHORT nCurX, USHORT nCurY, USHORT nCurZ,
                                    BOOL bCols = FALSE, BOOL bRows = FALSE );
    void            InitBlockMode( USHORT nCurX, USHORT nCurY, USHORT nCurZ,
                                    BOOL bTestNeg = FALSE,
                                    BOOL bCols = FALSE, BOOL bRows = FALSE );
    void            InitOwnBlockMode();
    void            DoneBlockMode( BOOL bContinue = FALSE );

    BOOL            IsBlockMode() const     { return bIsBlockMode; }

    void            MarkColumns();
    void            MarkRows();
    void            MarkDataArea( BOOL bIncludeCursor = TRUE );
    void            Unmark();

    void            MarkRange( const ScRange& rRange, BOOL bSetCursor = TRUE );

    BOOL            IsMarking( USHORT nCol, USHORT nRow, USHORT nTab ) const;

    void            PaintMarks( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow );
    void            PaintBlock( BOOL bReset = FALSE );

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
};



#endif

