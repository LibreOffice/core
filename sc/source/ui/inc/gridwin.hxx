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
#include <svtools/transfer.hxx>
#include "viewutil.hxx"
#include "viewdata.hxx"
#include "cbutton.hxx"
#include <svx/sdr/overlay/overlayobject.hxx>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

// ---------------------------------------------------------------------------

struct ScTableInfo;
class ScViewSelectionEngine;
class ScDPObject;
class ScDPFieldPopupWindow;
class ScDPFieldButton;
class ScOutputData;
class ScFilterListBox;
class AutoFilterPopup;
class SdrObject;
class SdrEditView;
class ScNoteMarker;
class FloatingWindow;
class SdrHdlList;
class ScTransferObj;
struct SpellCallbackInfo;

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

// ---------------------------------------------------------------------------
// predefines
namespace sdr { namespace overlay { class OverlayObjectList; }}

class ScGridWindow : public Window, public DropTargetHelper, public DragSourceHelper
{
    //  ScFilterListBox wird immer fuer Auswahlliste benutzt
    friend class ScFilterListBox;

private:
    // #114409#
    ::sdr::overlay::OverlayObjectList*              mpOOCursors;
    ::sdr::overlay::OverlayObjectList*              mpOOSelection;
    ::sdr::overlay::OverlayObjectList*              mpOOAutoFill;
    ::sdr::overlay::OverlayObjectList*              mpOODragRect;
    ::sdr::overlay::OverlayObjectList*              mpOOHeader;
    ::sdr::overlay::OverlayObjectList*              mpOOShrink;

    ::boost::shared_ptr<Rectangle> mpAutoFillRect;

    /**
     * Stores current visible column and row ranges, used to avoid expensive
     * operations on objects that are outside visible area.
     */
    struct VisibleRange
    {
        SCCOL mnCol1;
        SCCOL mnCol2;
        SCROW mnRow1;
        SCROW mnRow2;

        VisibleRange();

        bool isInside(SCCOL nCol, SCROW nRow) const;
    };
    VisibleRange maVisibleRange;

private:
    ScViewData*             pViewData;
    ScSplitPos              eWhich;
    ScHSplitPos             eHWhich;
    ScVSplitPos             eVWhich;

    ScNoteMarker*           pNoteMarker;

    ScFilterListBox*        pFilterBox;
    FloatingWindow*         pFilterFloat;
    ::std::auto_ptr<ScDPFieldPopupWindow> mpDPFieldPopup;
    ::std::auto_ptr<ScDPFieldButton>      mpFilterButton;

    sal_uInt16                  nCursorHideCount;

    sal_Bool                    bMarking;

    sal_uInt16                  nButtonDown;
    sal_Bool                    bEEMouse;               // Edit-Engine hat Maus
    sal_uInt8                   nMouseStatus;
    sal_uInt8                    nNestedButtonState;     // track nested button up/down calls

    sal_Bool                    bDPMouse;               // DataPilot-D&D (neue Pivottabellen)
    long                    nDPField;
    ScDPObject*             pDragDPObj; //! name?

    sal_Bool                    bRFMouse;               // RangeFinder-Drag
    sal_Bool                    bRFSize;
    sal_uInt16                  nRFIndex;
    SCsCOL                  nRFAddX;
    SCsROW                  nRFAddY;

    sal_uInt16                  nPagebreakMouse;        // Pagebreak-Modus Drag
    SCCOLROW                nPagebreakBreak;
    SCCOLROW                nPagebreakPrev;
    ScRange                 aPagebreakSource;
    ScRange                 aPagebreakDrag;
    sal_Bool                    bPagebreakDrawn;

    sal_uInt8                   nPageScript;

    long                    nLastClickX;
    long                    nLastClickY;

    sal_Bool                    bDragRect;
    SCCOL                   nDragStartX;
    SCROW                   nDragStartY;
    SCCOL                   nDragEndX;
    SCROW                   nDragEndY;
    InsCellCmd              meDragInsertMode;

    sal_uInt16                  nCurrentPointer;

    sal_Bool                    bIsInScroll;
    sal_Bool                    bIsInPaint;

    ScDDComboBoxButton      aComboButton;

    Point                   aCurMousePos;

    sal_uInt16                  nPaintCount;
    Rectangle               aRepaintPixel;
    sal_Bool                    bNeedsRepaint;

    sal_Bool                    bAutoMarkVisible;
    ScAddress               aAutoMarkPos;

    sal_Bool                    bListValButton;
    ScAddress               aListValPos;

    Rectangle               aInvertRect;

    DECL_LINK( PopupModeEndHdl, FloatingWindow* );
    DECL_LINK( PopupSpellingHdl, SpellCallbackInfo* );

    sal_Bool            TestMouse( const MouseEvent& rMEvt, sal_Bool bAction );

    sal_Bool            DoPageFieldSelection( SCCOL nCol, SCROW nRow );
    bool            DoAutoFilterButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt );
    void            DoPushButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt );

    void            DPMouseMove( const MouseEvent& rMEvt );
    void            DPMouseButtonUp( const MouseEvent& rMEvt );
    void            DPTestMouse( const MouseEvent& rMEvt, sal_Bool bMove );

    /**
     * Check if the mouse click is on a field popup button.
     *
     * @return bool true if the field popup menu has been launched and no
     *         further mouse event handling is necessary, false otherwise.
     */
    bool            DPTestFieldPopupArrow(const MouseEvent& rMEvt, const ScAddress& rPos, ScDPObject* pDPObj);
    void            DPLaunchFieldPopupMenu(
        const Point& rScrPos, const Size& rScrSize, const ScAddress& rPos, ScDPObject* pDPObj);

    void            RFMouseMove( const MouseEvent& rMEvt, sal_Bool bUp );

    void            PagebreakMove( const MouseEvent& rMEvt, sal_Bool bUp );

    void            UpdateDragRect( sal_Bool bShowRange, const Rectangle& rPosRect );

    sal_Bool            IsAutoFilterActive( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void            ExecFilter( sal_uLong nSel, SCCOL nCol, SCROW nRow,
                                const String& aValue, bool bCheckForDates );
    void            FilterSelect( sal_uLong nSel );

    void            ExecDataSelect( SCCOL nCol, SCROW nRow, const String& rStr );

    void            ExecPageFieldSelect( SCCOL nCol, SCROW nRow, sal_Bool bHasSelection, const String& rStr );

    sal_Bool            HasScenarioButton( const Point& rPosPixel, ScRange& rScenRange );

    sal_Bool            DropScroll( const Point& rMousePos );

    sal_Int8        AcceptPrivateDrop( const AcceptDropEvent& rEvt );
    sal_Int8        ExecutePrivateDrop( const ExecuteDropEvent& rEvt );
    sal_Int8        DropTransferObj( ScTransferObj* pTransObj, SCCOL nDestPosX, SCROW nDestPosY,
                                    const Point& rLogicPos, sal_Int8 nDndAction );

    void            HandleMouseButtonDown( const MouseEvent& rMEvt );

    sal_Bool            DrawMouseButtonDown(const MouseEvent& rMEvt);
    sal_Bool            DrawMouseButtonUp(const MouseEvent& rMEvt);
    sal_Bool            DrawMouseMove(const MouseEvent& rMEvt);
    sal_Bool            DrawKeyInput(const KeyEvent& rKEvt);
    sal_Bool            DrawCommand(const CommandEvent& rCEvt);
    sal_Bool            DrawHasMarkedObj();
    void            DrawEndAction();
    void            DrawMarkDropObj( SdrObject* pObj );
    SdrObject*      GetEditObject();
    sal_Bool            IsMyModel(SdrEditView* pSdrView);
    //void          DrawStartTimer();

    void            DrawRedraw( ScOutputData& rOutputData, ScUpdateMode eMode, sal_uLong nLayer );
    void            DrawSdrGrid( const Rectangle& rDrawingRect, OutputDevice* pContentDev );
    //sal_Bool          DrawBeforeScroll();
    void            DrawAfterScroll(/*sal_Bool bVal*/);
    //void          DrawMarks();
    //sal_Bool          NeedDrawMarks();
    void            DrawComboButton( const Point&   rCellPos,
                                     long           nCellSizeX,
                                     long           nCellSizeY,
                                     sal_Bool           bArrowState,
                                     sal_Bool           bBtnIn  = sal_False );
    Rectangle       GetListValButtonRect( const ScAddress& rButtonPos );

    void            DrawPagePreview( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, OutputDevice* pContentDev );

    sal_Bool            GetEditUrl( const Point& rPos,
                                String* pName=0, String* pUrl=0, String* pTarget=0 );
    sal_Bool            GetEditUrlOrError( sal_Bool bSpellErr, const Point& rPos,
                                String* pName=0, String* pUrl=0, String* pTarget=0 );

    sal_Bool            HitRangeFinder( const Point& rMouse, sal_Bool& rCorner, sal_uInt16* pIndex = NULL,
                                        SCsCOL* pAddX = NULL, SCsROW* pAddY = NULL );

    sal_uInt16          HitPageBreak( const Point& rMouse, ScRange* pSource = NULL,
                                    SCCOLROW* pBreak = NULL, SCCOLROW* pPrev = NULL );

    void            PasteSelection( const Point& rPosPixel );

    void            SelectForContextMenu( const Point& rPosPixel, SCsCOL nCellX, SCsROW nCellY );

    void            GetSelectionRects( ::std::vector< Rectangle >& rPixelRects );

protected:
    using Window::Resize;
    virtual void    Resize( const Size& rSize );
    virtual void    PrePaint();
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

    // #i70788# flush and get overlay
    ::sdr::overlay::OverlayManager* getOverlayManager();
    void flushOverlayManager();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    void            FakeButtonUp();

    Point           GetMousePosPixel() const;
    void            UpdateStatusPosSize();

    void            ClickExtern();

    void            SetPointer( const Pointer& rPointer );

    void            MoveMouseStatus( ScGridWindow &rDestWin );

    void            ScrollPixel( long nDifX, long nDifY );
    void            UpdateEditViewPos();

    void            UpdateFormulas();

    void            DoAutoFilterMenue( SCCOL nCol, SCROW nRow, sal_Bool bDataSelect );
    void            DoScenarioMenue( const ScRange& rScenRange );

    void            LaunchPageFieldMenu( SCCOL nCol, SCROW nRow );
    void            LaunchDPFieldMenu( SCCOL nCol, SCROW nRow );

    ::com::sun::star::sheet::DataPilotFieldOrientation GetDPFieldOrientation( SCCOL nCol, SCROW nRow ) const;

    void            DrawButtons( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    ScTableInfo& rTabInfo, OutputDevice* pContentDev );

    using Window::Draw;
    void            Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                        ScUpdateMode eMode = SC_UPDATE_ALL );

    void            InvertSimple( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    sal_Bool bTestMerge = sal_False, sal_Bool bRepeat = sal_False );

//UNUSED2008-05  void           DrawDragRect( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );

    void            CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress);

    void            HideCursor();
    void            ShowCursor();
    void            DrawCursor();
    void            DrawAutoFillMark();
    void            UpdateAutoFillMark(sal_Bool bMarked, const ScRange& rMarkRange);

    void            UpdateListValPos( sal_Bool bVisible, const ScAddress& rPos );

    sal_Bool            ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, sal_Bool bKeyboard );
    void            HideNoteMarker();

    MapMode         GetDrawMapMode( sal_Bool bForce = sal_False );

    void            ContinueDrag();

    void            StopMarking();
    void            UpdateInputContext();

    void            CheckInverted()     { if (nPaintCount) bNeedsRepaint = sal_True; }

    void            DoInvertRect( const Rectangle& rPixel );

    void            CheckNeedsRepaint();

    void            UpdateDPFromFieldPopupMenu();

    void            UpdateVisibleRange();

    // #114409#
    void CursorChanged();
    void DrawLayerCreated();

    void            DeleteCursorOverlay();
    void            UpdateCursorOverlay();
    void            DeleteSelectionOverlay();
    void            UpdateSelectionOverlay();
    void            DeleteAutoFillOverlay();
    void            UpdateAutoFillOverlay();
    void            DeleteDragRectOverlay();
    void            UpdateDragRectOverlay();
    void            DeleteHeaderOverlay();
    void            UpdateHeaderOverlay();
    void            DeleteShrinkOverlay();
    void            UpdateShrinkOverlay();
    void            UpdateAllOverlays();

protected:
    // #114409#
    void ImpCreateOverlayObjects();
    void ImpDestroyOverlayObjects();

};



#endif

