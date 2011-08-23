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

#include <bf_svx/zoomitem.hxx>

#include <bf_so3/ipclient.hxx>

#include "viewutil.hxx"
#include "select.hxx"
class SvBorder;
class Splitter;
namespace binfilter {

#ifndef SO2_DECL_SVINPLACECLIENT_DEFINED
#define SO2_DECL_SVINPLACECLIENT_DEFINED
SO2_DECL_REF(SvInPlaceClient)
#endif

class ScEditEngineDefaulter;
class ScGridWindow;
class ScOutlineWindow;
class ScRowBar;
class ScColBar;
class ScTabControl;
class ScTabViewShell;
class SfxPrinter;
class ScDrawView;
class FuPoor;
class ScTabSplitter;
class SdrView;
class SdrObject;
class ScHintWindow;
class ScPageBreakData;
class ScHighlightRanges;
class SchMemChart;
struct ChartSelectionInfo;
class SdrHdlList;

#define	SPLIT_HANDLE_SIZE	3
#define SC_FORCEMODE_NONE	0xff

// ---------------------------------------------------------------------------
//		Hilfs - Fenster

class ScCornerButton : public Window
{
private:
    ScViewData*		pViewData;
    BOOL			bAdd;

protected:
    virtual void	Resize();
public:
                    ScCornerButton( Window* pParent, ScViewData* pData, BOOL bAdditional );
                    ~ScCornerButton();

    virtual	void	StateChanged( StateChangedType nType );
};


// ---------------------------------------------------------------------------

class ScTabView
{
private:
    Window*				pFrameWin;				// als erstes !!!
    ScViewData			aViewData;				// muss ganz vorne stehen !

    ScViewSelectionEngine*	pSelEngine;
    ScViewFunctionSet		aFunctionSet;

    ScHeaderSelectionEngine* pHdrSelEng;
    ScHeaderFunctionSet		 aHdrFunc;

    SvInPlaceClientRef	aIPClient;

    ScDrawView*			pDrawView;

    Size				aFrameSize;				// wie bei DoResize uebergeben
    Point				aBorderPos;

    BOOL				bDrawSelMode;			// nur Zeichenobjekte selektieren ?

    FuPoor* 			pDrawActual;
    FuPoor*				pDrawOld;

    ScGridWindow*		pGridWin[4];
    ScColBar*			pColBar[2];
    ScRowBar*			pRowBar[2];
    ScOutlineWindow*	pColOutline[2];
    ScOutlineWindow*	pRowOutline[2];
    ScTabSplitter*		pHSplitter;
    ScTabSplitter*		pVSplitter;
    ScTabControl*		pTabControl;
    ScrollBar			aVScrollTop;
    ScrollBar			aVScrollBottom;			// anfangs sichtbar
    ScrollBar			aHScrollLeft;			// anfangs sichtbar
    ScrollBar			aHScrollRight;
    ScCornerButton		aCornerButton;
    ScCornerButton		aTopButton;
    ScrollBarBox		aScrollBarBox;

    ScHintWindow*		pInputHintWindow;		// Eingabemeldung bei Gueltigkeit

    ScPageBreakData*	pPageBreakData;			// fuer Seitenumbruch-Modus
    ScHighlightRanges*	pHighlightRanges;

    Timer				aScrollTimer;
    ScGridWindow*		pTimerWindow;
    MouseEvent			aTimerMEvt;

    ULONG				nTipVisible;

    BOOL				bDragging;				// fuer Scrollbars
    long				nPrevDragPos;

    BOOL				bIsBlockMode;           // Block markieren
    BOOL				bBlockNeg;				// wird Markierung aufgehoben?
    BOOL				bBlockCols;				// werden ganze Spalten markiert?
    BOOL				bBlockRows;				// werden ganze Zeilen markiert?
    USHORT				nBlockStartX;
    USHORT				nBlockEndX;
    USHORT				nBlockStartY;
    USHORT				nBlockEndY;
    USHORT				nBlockStartZ;
    USHORT				nBlockEndZ;

    SvxZoomType			eZoomType;
    BOOL				bMinimized;
    BOOL				bInUpdateHeader;
    BOOL				bInActivatePart;
    BOOL				bInZoomUpdate;
    BOOL				bMoveIsShift;
    BOOL				bNewStartIfMarking;


    void			Init();


    void			InitScrollBar( ScrollBar& rScrollBar, long nMaxVal );
    DECL_LINK(		ScrollHdl, ScrollBar* );
    DECL_LINK(		EndScrollHdl, ScrollBar* );

    DECL_LINK(		SplitHdl, Splitter* );

    DECL_LINK(		TimerHdl, Timer* );

    void			UpdateHeaderWidth( const ScVSplitPos* pWhich = NULL,
                                        const USHORT* pPosY = NULL );
    void			UpdateVarZoom();

protected:

    void			ZoomChanged();
    void			UpdateShow();
    void			GetBorderSize( SvBorder& rBorder, const Size& rSize );

    void			DrawEnableAnim(BOOL bSet);

    void			MakeDrawView( BYTE nForceDesignMode = SC_FORCEMODE_NONE );

    void			HideNoteMarker();


public:
                    ScTabView( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell );
                     ScTabView( Window* pParent, const ScTabView& rScTabView, ScTabViewShell* pViewShell );
                    ~ScTabView();


    void			HideListBox();

    void			TestHintWindow();


    DECL_LINK(		TabBarResize, void* );

    void			DoResize( const Point& rOffset, const Size& rSize, BOOL bInner = FALSE );
    void			RepeatResize( BOOL bUpdateFix = TRUE );
    void			UpdateFixPos();

    BOOL			IsDrawSelMode() const 		{ return bDrawSelMode; }
    void			SetDrawSelMode(BOOL bNew)	{ bDrawSelMode = bNew; }

    void			SetDrawFuncPtr(FuPoor* pFuncPtr)	{ pDrawActual = pFuncPtr; }
    void			SetDrawFuncOldPtr(FuPoor* pFuncPtr)	{ pDrawOld = pFuncPtr; }
    FuPoor*			GetDrawFuncPtr()					{ return pDrawActual; }
    FuPoor*			GetDrawFuncOldPtr()					{ return pDrawOld; }


    ScPageBreakData* GetPageBreakData()		{ return pPageBreakData; }
    ScHighlightRanges* GetHighlightRanges()	{ return pHighlightRanges; }



    ScViewData*			GetViewData() 		{ return &aViewData; }
    const ScViewData*	GetViewData() const	{ return &aViewData; }

    ScViewFunctionSet*		GetFunctionSet()	{ return &aFunctionSet; }
    ScViewSelectionEngine*	GetSelEngine()		{ return pSelEngine; }


    ScDrawView*		GetScDrawView()			{ return pDrawView; }
    SdrView*		GetSdrView();			// gegen CLOKs

    BOOL			IsMinimized() const		{ return bMinimized; }

    void			SetZoom( const Fraction& rNewX, const Fraction& rNewY );

    void			UpdateLayerLocks();


    void			UpdateInputLine();


    void			StopMarking();

    Window*			GetActiveWin();
    Window*			GetWindowByPos( ScSplitPos ePos );





    void			SetCursor( USHORT nPosX, USHORT nPosY, BOOL bNew = FALSE );


    void			CheckSelectionTransfer();



    void			UpdateAutoFillMark();


    void			AlignToCursor( short nCurX, short nCurY, ScFollowMode eMode,
                                    const ScSplitPos* pWhich = NULL );

    SvxZoomType		GetZoomType() const				{ return eZoomType; }
    void			SetZoomType( SvxZoomType eNew ) { eZoomType = eNew; }
    USHORT			CalcZoom( SvxZoomType eType, USHORT nOldZoom ){DBG_BF_ASSERT(0, "STRIP"); return 0; } //STRIP001 USHORT			CalcZoom( SvxZoomType eType, USHORT nOldZoom );

//	void			CalcZoom( SvxZoomType eType, USHORT& rZoom, USHORT& rCol, USHORT& rRow );


                    //	MoveCursorAbs		- absolut
                    //	MoveCursorRel		- einzelne Zellen
                    //	MoveCursorPage		- Bildschirmseite
                    //	MoveCursorArea		- Datenblock
                    //	MoveCursorEnd		- links oben / benutzter Bereich





    void			SetTabNo( USHORT nTab, BOOL bNew = FALSE );

    BOOL			IsInActivatePart() const	{ return bInActivatePart; }


    void 			ScrollX( long nDeltaX, ScHSplitPos eWhich, BOOL bUpdBars = TRUE );
    void 			ScrollY( long nDeltaY, ScVSplitPos eWhich, BOOL bUpdBars = TRUE );


    void			ScrollToObject( SdrObject* pDrawObj );

                                    // Zeichnen



    void			PaintArea( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                        ScUpdateMode eMode = SC_UPDATE_ALL );

    void			PaintGrid();

    void			PaintTopArea( USHORT nStartCol, USHORT nEndCol );
    void			PaintTop();

    void			PaintLeftArea( USHORT nStartRow, USHORT nEndRow );
    void			PaintLeft();

    BOOL			PaintExtras();



    void			UpdateFormulas();
    void			InterpretVisible();



    long			GetGridWidth( ScHSplitPos eWhich );
    long			GetGridHeight( ScVSplitPos eWhich );

    void			UpdateScrollBars();
    void			SetNewVisArea();


    void			KillEditView( BOOL bNoPaint );


                                    //	Bloecke


    void			InitOwnBlockMode();
    void			DoneBlockMode( BOOL bContinue = FALSE );

    BOOL			IsBlockMode() const		{ return bIsBlockMode; }

    void			MarkMatrixFormula();
    void			Unmark();

    void            MarkRange( const ScRange& rRange, BOOL bSetCursor = TRUE, BOOL bContinue = FALSE );



    USHORT			GetLockedModifiers() const;
    void			ViewOptionsHasChanged( BOOL bHScrollChanged,
                                            BOOL bGraphicsChanged = FALSE);


    void			FreezeSplitters( BOOL bFreeze );
    void			RemoveSplit();
    void			SplitAtPixel( const Point& rPixel, BOOL bHor, BOOL bVer );
    void			InvalidateSplit();

    void			ErrorMessage( USHORT nGlobStrId );


    Window*			GetFrameWin() const	{ return pFrameWin; }
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
