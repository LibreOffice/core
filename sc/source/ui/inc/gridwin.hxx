/*************************************************************************
 *
 *  $RCSfile: gridwin.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-30 19:12:07 $
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

#ifndef SC_GRIDWIN_HXX
#define SC_GRIDWIN_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

// nur auf dem MAC Auto-Filter per Popup
#ifdef MAC
#define AUTOFILTER_POPUP
#else
#undef AUTOFILTER_POPUP
#endif

#ifndef SC_VIEWUTIL_HXX
#include "viewutil.hxx"
#endif

#ifndef SC_VIEWDATA_HXX
#include "viewdata.hxx"
#endif

#ifndef SC_CBUTTON_HXX
#include "cbutton.hxx"
#endif

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
class FloatingWindow;
class SdrHdlList;

        //  Maus-Status (nMouseStatus)

#define SC_GM_NONE          0
#define SC_GM_TABDOWN       1
#define SC_GM_DBLDOWN       2
#define SC_GM_FILTER        3
#define SC_GM_IGNORE        4
#define SC_GM_WATERUNDO     5
#define SC_GM_URLDOWN       6

        //  Page-Drag-Modus

#define SC_PD_NONE          0
#define SC_PD_RANGE_L       1
#define SC_PD_RANGE_R       2
#define SC_PD_RANGE_T       4
#define SC_PD_RANGE_B       8
#define SC_PD_RANGE_TL      (SC_PD_RANGE_T|SC_PD_RANGE_L)
#define SC_PD_RANGE_TR      (SC_PD_RANGE_T|SC_PD_RANGE_R)
#define SC_PD_RANGE_BL      (SC_PD_RANGE_B|SC_PD_RANGE_L)
#define SC_PD_RANGE_BR      (SC_PD_RANGE_B|SC_PD_RANGE_R)
#define SC_PD_BREAK_H       16
#define SC_PD_BREAK_V       32


class ScHideTextCursor
{
private:
    ScViewData* pViewData;
    ScSplitPos  eWhich;

public:
            ScHideTextCursor( ScViewData* pData, ScSplitPos eW );
            ~ScHideTextCursor();
};


class ScGridWindow : public Window, public DropTargetHelper, public DragSourceHelper
{
    //  ScFilterListBox wird immer fuer Auswahlliste benutzt
    friend class ScFilterListBox;
#ifdef AUTOFILTER_POPUP
    friend class AutoFilterPopup;
#endif

private:
    ScViewData*             pViewData;
    ScSplitPos              eWhich;
    ScHSplitPos             eHWhich;
    ScVSplitPos             eVWhich;

    ScNoteMarker*           pNoteMarker;

    ScFilterListBox*        pFilterBox;
    USHORT                  nFilterBoxCol;
    USHORT                  nFilterBoxRow;
    FloatingWindow*         pFilterFloat;

    USHORT                  nCursorHideCount;

    BOOL                    bMarking;

    USHORT                  nButtonDown;
    BOOL                    bEEMouse;               // Edit-Engine hat Maus
    BYTE                    nMouseStatus;

    BOOL                    bPivotMouse;            // Pivot-D&D (alte Pivottabellen)
    ScPivot*                pDragPivot;
    BOOL                    bPivotColField;
    USHORT                  nPivotCol;
    USHORT                  nPivotField;

    BOOL                    bDPMouse;               // DataPilot-D&D (neue Pivottabellen)
    long                    nDPField;
    ScDPObject*             pDragDPObj; //! name?

    BOOL                    bRFMouse;               // RangeFinder-Drag
    BOOL                    bRFSize;
    USHORT                  nRFIndex;
    short                   nRFAddX;
    short                   nRFAddY;

    USHORT                  nPagebreakMouse;        // Pagebreak-Modus Drag
    USHORT                  nPagebreakBreak;
    USHORT                  nPagebreakPrev;
    ScRange                 aPagebreakSource;
    ScRange                 aPagebreakDrag;
    BOOL                    bPagebreakDrawn;

    BYTE                    nPageScript;

    long                    nLastClickX;
    long                    nLastClickY;

    BOOL                    bDragRect;
    USHORT                  nDragStartX;
    USHORT                  nDragStartY;
    USHORT                  nDragEndX;
    USHORT                  nDragEndY;

    USHORT                  nCurrentPointer;

    BOOL                    bIsInScroll;
    BOOL                    bIsInPaint;

    ScDDComboBoxButton      aComboButton;

    Point                   aCurMousePos;

    USHORT                  nPaintCount;
    Rectangle               aRepaintPixel;
    BOOL                    bNeedsRepaint;

    BOOL                    bAutoMarkVisible;
    ScAddress               aAutoMarkPos;

    Rectangle               aInvertRect;

    DECL_LINK( PopupModeEndHdl, FloatingWindow* );

    BOOL            TestMouse( const MouseEvent& rMEvt, BOOL bAction );

    void            DoPushButton( USHORT nCol, USHORT nRow, const MouseEvent& rMEvt );
    void            PivotMouseMove( const MouseEvent& rMEvt );
    void            PivotMouseButtonUp( const MouseEvent& rMEvt );
    BOOL            PivotTestMouse( const MouseEvent& rMEvt, BOOL bMove );
    void            DoPivotDrop( BOOL bDelete, BOOL bToCols, short nDestPos );

    void            DPMouseMove( const MouseEvent& rMEvt );
    void            DPMouseButtonUp( const MouseEvent& rMEvt );
    void            DPTestMouse( const MouseEvent& rMEvt, BOOL bMove );

    void            RFMouseMove( const MouseEvent& rMEvt, BOOL bUp );

    void            PagebreakMove( const MouseEvent& rMEvt, BOOL bUp );

    void            UpdateDragRect( BOOL bShowRange, const Rectangle& rPosRect );

    BOOL            IsAutoFilterActive( USHORT nCol, USHORT nRow, USHORT nTab );
    void            ExecFilter( ULONG nSel, USHORT nCol, USHORT nRow,
                                const String& aValue );
    void            FilterSelect( ULONG nSel );

    void            ExecDataSelect( USHORT nCol, USHORT nRow, const String& rStr );

    BOOL            HasScenarioButton( const Point& rPosPixel, ScRange& rScenRange );

    BOOL            DropScroll( const Point& rMousePos );

    sal_Int8        AcceptPrivateDrop( const AcceptDropEvent& rEvt );
    sal_Int8        ExecutePrivateDrop( const ExecuteDropEvent& rEvt );

    BOOL            DrawMouseButtonDown(const MouseEvent& rMEvt);
    BOOL            DrawMouseButtonUp(const MouseEvent& rMEvt);
    BOOL            DrawMouseMove(const MouseEvent& rMEvt);
    BOOL            DrawKeyInput(const KeyEvent& rKEvt);
    BOOL            DrawCommand(const CommandEvent& rCEvt);
    BOOL            DrawHasMarkedObj();
    void            DrawEndAction();
    void            DrawMarkDropObj( SdrObject* pObj );
    SdrObject*      GetEditObject();
    BOOL            IsMyModel(SdrEditView* pSdrView);
    void            DrawStartTimer();

    void            DrawRedraw( ScOutputData& rOutputData, const Rectangle& rDrawingRect,
                                        ScUpdateMode eMode, ULONG nLayer );
    void            DrawSdrGrid( const Rectangle& rDrawingRect );
    BOOL            DrawBeforeScroll();
    void            DrawAfterScroll(BOOL bVal);
    void            OutlinerViewPaint( const Rectangle& rRect );
    void            DrawMarks();
    BOOL            NeedDrawMarks();
    void            DrawComboButton( const Point&   rCellPos,
                                     long           nCellSizeX,
                                     long           nCellSizeY,
                                     BOOL           bBtnIn  = FALSE );

    void            DrawPagePreview( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 );

    BOOL            GetEditUrl( const Point& rPos,
                                String* pName=0, String* pUrl=0, String* pTarget=0 );
    BOOL            GetEditUrlOrError( BOOL bSpellErr, const Point& rPos,
                                String* pName=0, String* pUrl=0, String* pTarget=0 );

    BOOL            HitRangeFinder( const Point& rMouse, BOOL& rCorner, USHORT* pIndex = NULL,
                                        short* pAddX = NULL, short* pAddY = NULL );

    USHORT          HitPageBreak( const Point& rMouse, ScRange* pSource = NULL,
                                    USHORT* pBreak = NULL, USHORT* pPrev = NULL );

#ifdef AUTOFILTER_POPUP
    void            DoAutoFilterPopup( USHORT nCol, USHORT nRow, BOOL bDataSelect );
#endif

protected:
    virtual void    Resize( const Size& rSize );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual void    GetFocus();
    virtual void    LoseFocus();

    virtual void    RequestHelp( const HelpEvent& rEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );

public:
    ScGridWindow( Window* pParent, ScViewData* pData, ScSplitPos eWhichPos );
    ~ScGridWindow();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );

    void            FakeButtonUp();

    Point           GetMousePosPixel() const;
    void            UpdateStatusPosSize();

    void            ClickExtern();

    void            SetPointer( const Pointer& rPointer );

    void            MoveMouseStatus( ScGridWindow &rDestWin );

    void            ScrollPixel( long nDifX, long nDifY );
    void            UpdateEditViewPos();

    void            UpdateFormulas();

    void            DoAutoFilterMenue( USHORT nCol, USHORT nRow, BOOL bDataSelect );
    void            DoScenarioMenue( const ScRange& rScenRange );

    void            DrawButtons( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    RowInfo* pRowInfo, USHORT nArrCount );

    void            Draw( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                        ScUpdateMode eMode = SC_UPDATE_ALL );

    void            InvertSimple( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    BOOL bTestMerge = FALSE, BOOL bRepeat = FALSE );

    void            DrawDragRect( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    BOOL bMarkDrop = TRUE );

    void            DrawRefMark( USHORT nRefStartX, USHORT nRefStartY,
                                    USHORT nRefEndX, USHORT nRefEndY,
                                    const Color& rColor, BOOL bHandle );

    void            CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress);

    void            HideCursor();
    void            ShowCursor();
    void            DrawCursor();
    void            DrawAutoFillMark();
    void            UpdateAutoFillMark(BOOL bMarked, const ScRange& rMarkRange);

    void            HideNoteMarker();

    MapMode         GetDrawMapMode( BOOL bForce = FALSE );

    void            ContinueDrag();

    void            StopMarking();
    void            UpdateInputContext();

    void            CheckInverted()     { if (nPaintCount) bNeedsRepaint = TRUE; }

    void            DoInvertRect( const Rectangle& rPixel );

    void            CheckNeedsRepaint();
};



#endif

