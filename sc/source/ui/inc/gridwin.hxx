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

    std::unique_ptr<sdr::overlay::OverlayObjectList> mpOOCursors;
    std::unique_ptr<sdr::overlay::OverlayObjectList> mpOOSelection;
    std::unique_ptr<sdr::overlay::OverlayObjectList> mpOOSelectionBorder;
    std::unique_ptr<sdr::overlay::OverlayObjectList> mpOOAutoFill;
    std::unique_ptr<sdr::overlay::OverlayObjectList> mpOODragRect;
    std::unique_ptr<sdr::overlay::OverlayObjectList> mpOOHeader;
    std::unique_ptr<sdr::overlay::OverlayObjectList> mpOOShrink;

    std::unique_ptr<Rectangle> mpAutoFillRect;

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

    std::unique_ptr<sc::SpellCheckContext> mpSpellCheckCxt;

    ScViewData*             pViewData;
    ScSplitPos              eWhich;
    ScHSplitPos             eHWhich;
    ScVSplitPos             eVWhich;

    std::unique_ptr<ScNoteMarker> mpNoteMarker;

    VclPtr<ScFilterListBox>          mpFilterBox;
    VclPtr<FloatingWindow>           mpFilterFloat;
    VclPtr<ScCheckListMenuWindow>    mpAutoFilterPopup;
    VclPtr<ScCheckListMenuWindow>    mpDPFieldPopup;
    std::unique_ptr<ScDPFieldButton> mpFilterButton;

    sal_uInt16              nCursorHideCount;

    sal_uInt16              nButtonDown;
    sal_uInt8               nMouseStatus;
    enum class ScNestedButtonState { NONE, Down, Up };
    ScNestedButtonState     nNestedButtonState;     // track nested button up/down calls

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

    SvtScriptType           nPageScript;

    SCCOL                   nDragStartX;
    SCROW                   nDragStartY;
    SCCOL                   nDragEndX;
    SCROW                   nDragEndY;
    InsCellCmd              meDragInsertMode;

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

    DECL_LINK( PopupModeEndHdl, FloatingWindow*, void );
    DECL_LINK( PopupSpellingHdl, SpellCallbackInfo&, void );

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

    void DPLaunchFieldPopupMenu(const Point& rScrPos, const Size& rScrSize, const ScAddress& rPos, ScDPObject* pDPObj);

    void            RFMouseMove( const MouseEvent& rMEvt, bool bUp );

    void            PagebreakMove( const MouseEvent& rMEvt, bool bUp );

    void            UpdateDragRect( bool bShowRange, const Rectangle& rPosRect );

    bool            IsAutoFilterActive( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void            ExecFilter( sal_uLong nSel, SCCOL nCol, SCROW nRow,
                                const OUString& aValue );
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

    void            DrawPagePreview( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, vcl::RenderContext& rRenderContext);

    bool            GetEditUrl( const Point& rPos,
                                OUString* pName=nullptr, OUString* pUrl=nullptr, OUString* pTarget=nullptr );

    bool IsSpellErrorAtPos( const Point& rPos, SCCOL nCol1, SCROW nRow );

    bool            HitRangeFinder( const Point& rMouse, RfCorner& rCorner, sal_uInt16* pIndex,
                                    SCsCOL* pAddX, SCsROW* pAddY );

    sal_uInt16      HitPageBreak( const Point& rMouse, ScRange* pSource,
                                  SCCOLROW* pBreak, SCCOLROW* pPrev );

    /** The cell may be covered by text that overflows from a previous cell.

        @return if true, the given cell is covered by (overflowing) text and
        rTextStartPosX returns the column where the text that overflows
        starts.
    */
    bool            IsCellCoveredByText(SCsCOL nPosX, SCsROW nPosY, SCTAB nTab, SCsCOL &rTextStartPosX);

    void            PasteSelection( const Point& rPosPixel );

    void            SelectForContextMenu( const Point& rPosPixel, SCsCOL nCellX, SCsROW nCellY );

    void            GetSelectionRects( ::std::vector< Rectangle >& rPixelRects );


protected:
    virtual void    PrePaint(vcl::RenderContext& rRenderContext) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    virtual void    RequestHelp( const HelpEvent& rEvt ) override;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

public:
    enum AutoFilterMode { Normal, Top10, Custom, Empty, NonEmpty, SortAscending, SortDescending };

    ScGridWindow( vcl::Window* pParent, ScViewData* pData, ScSplitPos eWhichPos );
    virtual ~ScGridWindow() override;
    virtual void dispose() override;

    virtual void    KeyInput(const KeyEvent& rKEvt) override;
    // #i70788# flush and get overlay
    rtl::Reference<sdr::overlay::OverlayManager> getOverlayManager();
    void flushOverlayManager();

    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;

    void            PaintTile( VirtualDevice& rDevice,
                               int nOutputWidth, int nOutputHeight,
                               int nTilePosX, int nTilePosY,
                               long nTileWidth, long nTileHeight );

    /// @see OutputDevice::LogicInvalidate().
    void LogicInvalidate(const Rectangle* pRectangle) override;

    /// Update the cell selection according to what handles have been dragged.
    /// @see vcl::ITiledRenderable::setTextSelection() for the values of nType.
    /// Coordinates are in pixels.
    void SetCellSelectionPixel(int nType, int nPixelX, int nPixelY);
    /// Get the cell selection, coordinates are in logic units.
    void GetCellSelection(std::vector<Rectangle>& rLogicRects);

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    void            FakeButtonUp();

    const Point&    GetMousePosPixel() const { return aCurMousePos; }
    void            UpdateStatusPosSize();

    void            ClickExtern();

    using Window::SetPointer;

    void            MoveMouseStatus( ScGridWindow &rDestWin );

    void            ScrollPixel( long nDifX, long nDifY );
    void            UpdateEditViewPos();

    void            UpdateFormulas();

    void            LaunchDataSelectMenu( SCCOL nCol, SCROW nRow );
    void            DoScenarioMenu( const ScRange& rScenRange );

    void            LaunchAutoFilterMenu(SCCOL nCol, SCROW nRow);
    void            RefreshAutoFilterButton(const ScAddress& rPos);
    void            UpdateAutoFilterFromMenu(AutoFilterMode eMode);

    void            LaunchPageFieldMenu( SCCOL nCol, SCROW nRow );
    void            LaunchDPFieldMenu( SCCOL nCol, SCROW nRow );

    css::sheet::DataPilotFieldOrientation GetDPFieldOrientation( SCCOL nCol, SCROW nRow ) const;

    void DPLaunchFieldPopupMenu(const Point& rScrPos, const Size& rScrSize,
                                long nDimIndex, ScDPObject* pDPObj);

    void DrawButtons(SCCOL nX1, SCCOL nX2, const ScTableInfo& rTabInfo, OutputDevice* pContentDev);

    using Window::Draw;
    void            Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                          ScUpdateMode eMode );

    /// Draw content of the gridwindow; shared between the desktop and the tiled rendering.
    void DrawContent(OutputDevice &rDevice, const ScTableInfo& rTableInfo, ScOutputData& aOutputData, bool bLogicText, ScUpdateMode eMode);

    void            CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress);

    void            HideCursor();
    void            ShowCursor();
    void            UpdateAutoFillMark(bool bMarked, const ScRange& rMarkRange);

    void            UpdateListValPos( bool bVisible, const ScAddress& rPos );

    bool            ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, bool bKeyboard );
    void            HideNoteMarker();

    /// MapMode for the drawinglayer objects.
    MapMode         GetDrawMapMode( bool bForce = false );

    void            StopMarking();
    void            UpdateInputContext();

    bool            NeedsRepaint() { return bNeedsRepaint; }

    void            DoInvertRect( const Rectangle& rPixel );

    void            CheckNeedsRepaint();

    void            UpdateDPFromFieldPopupMenu();
    bool            UpdateVisibleRange();

    void CursorChanged();
    void DrawLayerCreated();
    bool ContinueOnlineSpelling();
    void EnableAutoSpell( bool bEnable );
    void ResetAutoSpell();
    void SetAutoSpellData( SCCOL nPosX, SCROW nPosY, const std::vector<editeng::MisspellRanges>* pRanges );
    const std::vector<editeng::MisspellRanges>* GetAutoSpellData( SCCOL nPosX, SCROW nPosY );
    bool InsideVisibleRange( SCCOL nPosX, SCROW nPosY );

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

    /// @see ScModelObj::getCellCursor().
    OString         getCellCursor(const Fraction& rZoomX,
                                  const Fraction& rZoomY) const;
    OString         getCellCursor(int nOutputWidth,
                                  int nOutputHeight,
                                  long nTileWidth,
                                  long nTileHeight);
    void updateLibreOfficeKitCellCursor(SfxViewShell* pOtherShell) const;

    ScViewData* getViewData();
    virtual FactoryFunction GetUITestFactory() const override;

protected:
    void ImpCreateOverlayObjects();
    void ImpDestroyOverlayObjects();

private:

#ifdef DBG_UTIL
    void dumpCellProperties();
    void dumpColumnInformationPixel();
    void dumpColumnInformationHmm();
    void dumpGraphicInformation();
    void dumpColumnCellStorage();
#endif

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
