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

#ifndef INCLUDED_SC_SOURCE_UI_INC_GRIDWIN_HXX
#define INCLUDED_SC_SOURCE_UI_INC_GRIDWIN_HXX

#include <svtools/transfer.hxx>
#include "viewutil.hxx"
#include "viewdata.hxx"
#include "cbutton.hxx"
#include <svx/sdr/overlay/overlayobject.hxx>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace editeng {
    struct MisspellRanges;
}

namespace sc {
    struct SpellCheckContext;
}

struct ScTableInfo;
class ScDPObject;
class ScCheckListMenuWindow;
class ScDPFieldButton;
class ScOutputData;
class ScFilterListBox;
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

// predefines
namespace sdr { namespace overlay { class OverlayObjectList; }}

class ScGridWindow : public vcl::Window, public DropTargetHelper, public DragSourceHelper
{
    // ScFilterListBox is always used for selection list
    friend class ScFilterListBox;

    enum RfCorner
    {
        NONE,
        LEFT_UP,
        RIGHT_UP,
        LEFT_DOWN,
        RIGHT_DOWN
    };

    // #114409#
    ::sdr::overlay::OverlayObjectList*              mpOOCursors;
    ::sdr::overlay::OverlayObjectList*              mpOOSelection;
    ::sdr::overlay::OverlayObjectList*              mpOOSelectionBorder;
    ::sdr::overlay::OverlayObjectList*              mpOOAutoFill;
    ::sdr::overlay::OverlayObjectList*              mpOODragRect;
    ::sdr::overlay::OverlayObjectList*              mpOOHeader;
    ::sdr::overlay::OverlayObjectList*              mpOOShrink;

    boost::scoped_ptr<Rectangle> mpAutoFillRect;

    struct MouseEventState;

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
        bool set(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    };

    VisibleRange maVisibleRange;

    boost::scoped_ptr<sc::SpellCheckContext> mpSpellCheckCxt;

    ScViewData*             pViewData;
    ScSplitPos              eWhich;
    ScHSplitPos             eHWhich;
    ScVSplitPos             eVWhich;

    ScNoteMarker*           pNoteMarker;

    ScFilterListBox*        pFilterBox;
    FloatingWindow*         pFilterFloat;
    boost::scoped_ptr<ScCheckListMenuWindow> mpAutoFilterPopup;
    boost::scoped_ptr<ScCheckListMenuWindow> mpDPFieldPopup;
    boost::scoped_ptr<ScDPFieldButton>       mpFilterButton;

    sal_uInt16              nCursorHideCount;

    sal_uInt16              nButtonDown;
    sal_uInt8               nMouseStatus;
    sal_uInt8               nNestedButtonState;     // track nested button up/down calls

    long                    nDPField;
    ScDPObject*             pDragDPObj; //! name?

    sal_uInt16              nRFIndex;
    SCsCOL                  nRFAddX;
    SCsROW                  nRFAddY;

    sal_uInt16              nPagebreakMouse;        // Page break mode, Drag
    SCCOLROW                nPagebreakBreak;
    SCCOLROW                nPagebreakPrev;
    ScRange                 aPagebreakSource;
    ScRange                 aPagebreakDrag;

    sal_uInt8               nPageScript;

    long                    nLastClickX;
    long                    nLastClickY;

    SCCOL                   nDragStartX;
    SCROW                   nDragStartY;
    SCCOL                   nDragEndX;
    SCROW                   nDragEndY;
    InsCellCmd              meDragInsertMode;

    sal_uInt16              nCurrentPointer;

    ScDDComboBoxButton      aComboButton;

    Point                   aCurMousePos;

    sal_uInt16              nPaintCount;
    Rectangle               aRepaintPixel;

    ScAddress               aAutoMarkPos;
    ScAddress               aListValPos;

    Rectangle               aInvertRect;

    RfCorner                aRFSelectedCorned;

    bool                    bEEMouse:1;               // Edit Engine has mouse
    bool                    bDPMouse:1;               // DataPilot D&D (new Pivot table)
    bool                    bRFMouse:1;               // RangeFinder drag
    bool                    bRFSize:1;
    bool                    bPagebreakDrawn:1;
    bool                    bDragRect:1;
    bool                    bIsInScroll:1;
    bool                    bIsInPaint:1;
    bool                    bNeedsRepaint:1;
    bool                    bAutoMarkVisible:1;
    bool                    bListValButton:1;

    DECL_LINK( PopupModeEndHdl, void* );
    DECL_LINK( PopupSpellingHdl, SpellCallbackInfo* );

    bool            TestMouse( const MouseEvent& rMEvt, bool bAction );

    bool            DoPageFieldSelection( SCCOL nCol, SCROW nRow );
    bool            DoAutoFilterButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt );
    void DoPushPivotButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt, bool bButton, bool bPopup );

    void            DPMouseMove( const MouseEvent& rMEvt );
    void            DPMouseButtonUp( const MouseEvent& rMEvt );
    void            DPTestMouse( const MouseEvent& rMEvt, bool bMove );

    /**
     * Check if the mouse click is on a field popup button.
     *
     * @return true if the field popup menu has been launched and no further
     *         mouse event handling is necessary, false otherwise.
     */
    bool DPTestFieldPopupArrow(const MouseEvent& rMEvt, const ScAddress& rPos, const ScAddress& rDimPos, ScDPObject* pDPObj);
    void            DPLaunchFieldPopupMenu(
        const Point& rScrPos, const Size& rScrSize, const ScAddress& rPos, ScDPObject* pDPObj);

    void            RFMouseMove( const MouseEvent& rMEvt, bool bUp );

    void            PagebreakMove( const MouseEvent& rMEvt, bool bUp );

    void            UpdateDragRect( bool bShowRange, const Rectangle& rPosRect );

    bool            IsAutoFilterActive( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void            ExecFilter( sal_uLong nSel, SCCOL nCol, SCROW nRow,
                                const OUString& aValue, bool bCheckForDates );
    void            FilterSelect( sal_uLong nSel );

    void            ExecDataSelect( SCCOL nCol, SCROW nRow, const OUString& rStr );

    void            ExecPageFieldSelect( SCCOL nCol, SCROW nRow, bool bHasSelection, const OUString& rStr );

    bool            HasScenarioButton( const Point& rPosPixel, ScRange& rScenRange );

    bool            DropScroll( const Point& rMousePos );

    sal_Int8        AcceptPrivateDrop( const AcceptDropEvent& rEvt );
    sal_Int8        ExecutePrivateDrop( const ExecuteDropEvent& rEvt );
    sal_Int8        DropTransferObj( ScTransferObj* pTransObj, SCCOL nDestPosX, SCROW nDestPosY,
                                     const Point& rLogicPos, sal_Int8 nDndAction );

    void            HandleMouseButtonDown( const MouseEvent& rMEvt, MouseEventState& rState );

    bool            DrawMouseButtonDown(const MouseEvent& rMEvt);
    bool            DrawMouseButtonUp(const MouseEvent& rMEvt);
    bool            DrawMouseMove(const MouseEvent& rMEvt);
    bool            DrawKeyInput(const KeyEvent& rKEvt);
    bool            DrawCommand(const CommandEvent& rCEvt);
    bool            DrawHasMarkedObj();
    void            DrawEndAction();
    void            DrawMarkDropObj( SdrObject* pObj );
    SdrObject*      GetEditObject();
    bool            IsMyModel(SdrEditView* pSdrView);

    void            DrawRedraw( ScOutputData& rOutputData, ScUpdateMode eMode, sal_uLong nLayer );
    void            DrawSdrGrid( const Rectangle& rDrawingRect, OutputDevice* pContentDev );
    void            DrawAfterScroll();
    Rectangle       GetListValButtonRect( const ScAddress& rButtonPos );

    void            DrawPagePreview( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, OutputDevice* pContentDev );

    bool            GetEditUrl( const Point& rPos,
                                OUString* pName=0, OUString* pUrl=0, OUString* pTarget=0 );

    bool IsSpellErrorAtPos( const Point& rPos, SCCOL nCol1, SCROW nRow );

    bool            HitRangeFinder( const Point& rMouse, RfCorner& rCorner, sal_uInt16* pIndex = NULL,
                                    SCsCOL* pAddX = NULL, SCsROW* pAddY = NULL );

    sal_uInt16          HitPageBreak( const Point& rMouse, ScRange* pSource = NULL,
                                    SCCOLROW* pBreak = NULL, SCCOLROW* pPrev = NULL );

    void            PasteSelection( const Point& rPosPixel );

    void            SelectForContextMenu( const Point& rPosPixel, SCsCOL nCellX, SCsROW nCellY );

    void            GetSelectionRects( ::std::vector< Rectangle >& rPixelRects );

protected:
    using Window::Resize;
    virtual void    Resize( const Size& rSize );
    virtual void    PrePaint() SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;

    virtual void    RequestHelp( const HelpEvent& rEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

public:
    enum AutoFilterMode { Normal, Top10, Custom, Empty, NonEmpty, SortAscending, SortDescending };

    ScGridWindow( vcl::Window* pParent, ScViewData* pData, ScSplitPos eWhichPos );
    virtual ~ScGridWindow();

    // #i70788# flush and get overlay
    rtl::Reference<sdr::overlay::OverlayManager> getOverlayManager();
    void flushOverlayManager();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;

    void            PaintTile( VirtualDevice& rDevice,
                               int nOutputWidth, int nOutputHeight,
                               int nTilePosX, int nTilePosY,
                               long nTileWidth, long nTileHeight );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

    void            FakeButtonUp();

    Point           GetMousePosPixel() const { return aCurMousePos; }
    void            UpdateStatusPosSize();

    void            ClickExtern();

    void            SetPointer( const Pointer& rPointer );

    void            MoveMouseStatus( ScGridWindow &rDestWin );

    void            ScrollPixel( long nDifX, long nDifY );
    void            UpdateEditViewPos();

    void            UpdateFormulas();

    void            LaunchDataSelectMenu( SCCOL nCol, SCROW nRow, bool bDataSelect );
    void            DoScenarioMenu( const ScRange& rScenRange );

    void            LaunchAutoFilterMenu(SCCOL nCol, SCROW nRow);
    void            RefreshAutoFilterButton(const ScAddress& rPos);
    void            UpdateAutoFilterFromMenu(AutoFilterMode eMode);

    void            LaunchPageFieldMenu( SCCOL nCol, SCROW nRow );
    void            LaunchDPFieldMenu( SCCOL nCol, SCROW nRow );

    ::com::sun::star::sheet::DataPilotFieldOrientation GetDPFieldOrientation( SCCOL nCol, SCROW nRow ) const;

    void DrawButtons( SCCOL nX1, SCCOL nX2, ScTableInfo& rTabInfo, OutputDevice* pContentDev);

    using Window::Draw;
    void            Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                          ScUpdateMode eMode = SC_UPDATE_ALL );

    void            CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress);

    void            HideCursor();
    void            ShowCursor();
    void            UpdateAutoFillMark(bool bMarked, const ScRange& rMarkRange);

    void            UpdateListValPos( bool bVisible, const ScAddress& rPos );

    bool            ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, bool bKeyboard );
    void            HideNoteMarker();

    MapMode         GetDrawMapMode( bool bForce = false );

    void            ContinueDrag();

    void            StopMarking();
    void            UpdateInputContext();

    bool            NeedsRepaint() { return bNeedsRepaint; }

    void            DoInvertRect( const Rectangle& rPixel );

    void            CheckNeedsRepaint();
    virtual void SwitchView();

    void            UpdateDPFromFieldPopupMenu();
    bool            UpdateVisibleRange();

    void            SetInRefMode( bool bRefMode );

    // #114409#
    void CursorChanged();
    void DrawLayerCreated();
    bool ContinueOnlineSpelling();
    void EnableAutoSpell( bool bEnable );
    void ResetAutoSpell();
    void SetAutoSpellData( SCCOL nPosX, SCROW nPosY, const std::vector<editeng::MisspellRanges>* pRanges );

    void            DeleteCopySourceOverlay();
    void            UpdateCopySourceOverlay();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
