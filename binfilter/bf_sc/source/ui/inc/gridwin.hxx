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

#ifndef SC_GRIDWIN_HXX
#define SC_GRIDWIN_HXX

#include <tools/string.hxx>

#include <bf_svtools/transfer.hxx>

// nur auf dem MAC Auto-Filter per Popup
#ifdef MAC
#define AUTOFILTER_POPUP
#else
#undef AUTOFILTER_POPUP
#endif

#include "viewutil.hxx"

#include "viewdata.hxx"

#include "cbutton.hxx"
class FloatingWindow;
namespace binfilter {

// ---------------------------------------------------------------------------

struct RowInfo;
class ScViewSelectionEngine;
class ScPivot;
class ScDPObject;
class ScOutputData;
class ScFilterListBox;
class AutoFilterPopup;
class SdrObject;
class SdrEditView;
class ScNoteMarker;
class SdrHdlList;

        //	Maus-Status (nMouseStatus)

#define SC_GM_NONE			0
#define SC_GM_TABDOWN		1
#define SC_GM_DBLDOWN		2
#define SC_GM_FILTER		3
#define SC_GM_IGNORE		4
#define SC_GM_WATERUNDO		5
#define SC_GM_URLDOWN		6

        //	Page-Drag-Modus

#define SC_PD_NONE			0
#define SC_PD_RANGE_L		1
#define SC_PD_RANGE_R		2
#define SC_PD_RANGE_T		4
#define SC_PD_RANGE_B		8
#define SC_PD_RANGE_TL		(SC_PD_RANGE_T|SC_PD_RANGE_L)
#define SC_PD_RANGE_TR		(SC_PD_RANGE_T|SC_PD_RANGE_R)
#define SC_PD_RANGE_BL		(SC_PD_RANGE_B|SC_PD_RANGE_L)
#define SC_PD_RANGE_BR		(SC_PD_RANGE_B|SC_PD_RANGE_R)
#define SC_PD_BREAK_H		16
#define SC_PD_BREAK_V		32




class ScGridWindow : public Window, public DropTargetHelper, public DragSourceHelper
{
    //	ScFilterListBox wird immer fuer Auswahlliste benutzt
    friend class ScFilterListBox;
#ifdef AUTOFILTER_POPUP
    friend class AutoFilterPopup;
#endif

private:
    ScViewData*				pViewData;
    ScSplitPos				eWhich;
    ScHSplitPos				eHWhich;
    ScVSplitPos				eVWhich;

    ScNoteMarker*			pNoteMarker;

    ScFilterListBox*		pFilterBox;
    USHORT					nFilterBoxCol;
    USHORT					nFilterBoxRow;
    FloatingWindow*			pFilterFloat;

    USHORT					nCursorHideCount;

    BOOL					bMarking;

    USHORT					nButtonDown;
    BOOL					bEEMouse;				// Edit-Engine hat Maus
    BYTE					nMouseStatus;

    BOOL					bPivotMouse;			// Pivot-D&D (alte Pivottabellen)
    ScPivot*				pDragPivot;
    BOOL					bPivotColField;
    USHORT					nPivotCol;
    USHORT					nPivotField;

    BOOL					bDPMouse;				// DataPilot-D&D (neue Pivottabellen)
    long					nDPField;
    ScDPObject*				pDragDPObj;	//! name?

    BOOL					bRFMouse;				// RangeFinder-Drag
    BOOL					bRFSize;
    USHORT					nRFIndex;
    short					nRFAddX;
    short					nRFAddY;

    USHORT					nPagebreakMouse;		// Pagebreak-Modus Drag
    USHORT					nPagebreakBreak;
    USHORT					nPagebreakPrev;
    ScRange					aPagebreakSource;
    ScRange					aPagebreakDrag;
    BOOL					bPagebreakDrawn;

    BYTE					nPageScript;

    long					nLastClickX;
    long					nLastClickY;

    BOOL					bDragRect;
    USHORT					nDragStartX;
    USHORT					nDragStartY;
    USHORT					nDragEndX;
    USHORT					nDragEndY;

    USHORT					nCurrentPointer;

    BOOL					bIsInScroll;
    BOOL					bIsInPaint;

    ScDDComboBoxButton		aComboButton;

    Point					aCurMousePos;

    USHORT					nPaintCount;
    Rectangle				aRepaintPixel;
    BOOL					bNeedsRepaint;

    BOOL					bAutoMarkVisible;
    ScAddress				aAutoMarkPos;

    Rectangle				aInvertRect;













    void			DrawEndAction();
    SdrObject*		GetEditObject();
    void			DrawStartTimer();

    void			DrawRedraw( ScOutputData& rOutputData, const Rectangle& rDrawingRect,
                                        ScUpdateMode eMode, ULONG nLayer );
    void			DrawSdrGrid( const Rectangle& rDrawingRect );
    void			DrawMarks();
    BOOL			NeedDrawMarks();





#ifdef AUTOFILTER_POPUP
#endif


protected:



public:
    ScGridWindow( Window* pParent, ScViewData* pData, ScSplitPos eWhichPos );
    ~ScGridWindow();






    void			ClickExtern();




    void			UpdateFormulas();


    void			DrawButtons( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    RowInfo* pRowInfo, USHORT nArrCount );

    void			Draw( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                        ScUpdateMode eMode = SC_UPDATE_ALL );





    void			HideCursor();
    void			ShowCursor();
    void 			DrawCursor();
    void			DrawAutoFillMark();
     void			UpdateAutoFillMark(BOOL bMarked, const ScRange& rMarkRange);

    void			HideNoteMarker();

    MapMode			GetDrawMapMode( BOOL bForce = FALSE );

    void			ContinueDrag();

    void			StopMarking();

    void			CheckInverted()		{ if (nPaintCount) bNeedsRepaint = TRUE; }


    void			CheckNeedsRepaint();
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
