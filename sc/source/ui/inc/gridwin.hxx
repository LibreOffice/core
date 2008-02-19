/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gridwin.hxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 15:32:45 $
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

#ifndef SC_GRIDWIN_HXX
#define SC_GRIDWIN_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
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

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#include <svx/sdr/overlay/overlayobject.hxx>
#endif

#include <vector>

// ---------------------------------------------------------------------------

struct ScTableInfo;
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
class ScGridWindow;

enum ScOverlayType { SC_OVERLAY_INVERT, SC_OVERLAY_HATCH, SC_OVERLAY_SOLID,
                     SC_OVERLAY_TRANSPARENT, SC_OVERLAY_LIGHT_TRANSPARENT, SC_OVERLAY_BORDER_TRANSPARENT };

// #114409#
namespace sdr
{
    namespace overlay
    {
        // predefines
        class OverlayObjectList;

        // OverlayObjectCell - used for cell cursor, selection and AutoFill handle

        class OverlayObjectCell : public OverlayObject
        {
        public:
            typedef ::std::vector< basegfx::B2DRange > RangeVector;

        private:
            ScOverlayType   mePaintType;
            RangeVector     maRectangles;

            virtual void drawGeometry(OutputDevice& rOutputDevice);
            virtual void createBaseRange(OutputDevice& rOutputDevice);

        public:
            OverlayObjectCell( ScOverlayType eType, const Color& rColor, const RangeVector& rRects);
            virtual ~OverlayObjectCell();

            virtual void transform(const basegfx::B2DHomMatrix& rMatrix);
        };

    } // end of namespace overlay
} // end of namespace sdr

// ---------------------------------------------------------------------------

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

private:
    ScViewData*             pViewData;
    ScSplitPos              eWhich;
    ScHSplitPos             eHWhich;
    ScVSplitPos             eVWhich;

    ScNoteMarker*           pNoteMarker;

    ScFilterListBox*        pFilterBox;
    FloatingWindow*         pFilterFloat;

    USHORT                  nCursorHideCount;

    BOOL                    bMarking;

    USHORT                  nButtonDown;
    BOOL                    bEEMouse;               // Edit-Engine hat Maus
    BYTE                    nMouseStatus;
    BYTE                    nNestedButtonState;     // track nested button up/down calls

    BOOL                    bPivotMouse;            // Pivot-D&D (alte Pivottabellen)
    ScPivot*                pDragPivot;
    BOOL                    bPivotColField;
    SCCOL                   nPivotCol;
    SCCOL                   nPivotField;

    BOOL                    bDPMouse;               // DataPilot-D&D (neue Pivottabellen)
    long                    nDPField;
    ScDPObject*             pDragDPObj; //! name?

    BOOL                    bRFMouse;               // RangeFinder-Drag
    BOOL                    bRFSize;
    USHORT                  nRFIndex;
    SCsCOL                  nRFAddX;
    SCsROW                  nRFAddY;

    USHORT                  nPagebreakMouse;        // Pagebreak-Modus Drag
    SCCOLROW                nPagebreakBreak;
    SCCOLROW                nPagebreakPrev;
    ScRange                 aPagebreakSource;
    ScRange                 aPagebreakDrag;
    BOOL                    bPagebreakDrawn;

    BYTE                    nPageScript;

    long                    nLastClickX;
    long                    nLastClickY;

    BOOL                    bDragRect;
    SCCOL                   nDragStartX;
    SCROW                   nDragStartY;
    SCCOL                   nDragEndX;
    SCROW                   nDragEndY;
    InsCellCmd              meDragInsertMode;

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

    BOOL                    bListValButton;
    ScAddress               aListValPos;

    Rectangle               aInvertRect;

    DECL_LINK( PopupModeEndHdl, FloatingWindow* );
    DECL_LINK( PopupSpellingHdl, SpellCallbackInfo* );

    BOOL            TestMouse( const MouseEvent& rMEvt, BOOL bAction );

    BOOL            DoPageFieldSelection( SCCOL nCol, SCROW nRow );
    void            DoPushButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt );
    void            PivotMouseMove( const MouseEvent& rMEvt );
    void            PivotMouseButtonUp( const MouseEvent& rMEvt );
    BOOL            PivotTestMouse( const MouseEvent& rMEvt, BOOL bMove );
    void            DoPivotDrop( BOOL bDelete, BOOL bToCols, SCSIZE nDestPos );

    void            DPMouseMove( const MouseEvent& rMEvt );
    void            DPMouseButtonUp( const MouseEvent& rMEvt );
    void            DPTestMouse( const MouseEvent& rMEvt, BOOL bMove );

    void            RFMouseMove( const MouseEvent& rMEvt, BOOL bUp );

    void            PagebreakMove( const MouseEvent& rMEvt, BOOL bUp );

    void            UpdateDragRect( BOOL bShowRange, const Rectangle& rPosRect );

    BOOL            IsAutoFilterActive( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void            ExecFilter( ULONG nSel, SCCOL nCol, SCROW nRow,
                                const String& aValue );
    void            FilterSelect( ULONG nSel );

    void            ExecDataSelect( SCCOL nCol, SCROW nRow, const String& rStr );

    void            ExecPageFieldSelect( SCCOL nCol, SCROW nRow, BOOL bHasSelection, const String& rStr );

    BOOL            HasScenarioButton( const Point& rPosPixel, ScRange& rScenRange );

    BOOL            DropScroll( const Point& rMousePos );

    sal_Int8        AcceptPrivateDrop( const AcceptDropEvent& rEvt );
    sal_Int8        ExecutePrivateDrop( const ExecuteDropEvent& rEvt );
    sal_Int8        DropTransferObj( ScTransferObj* pTransObj, SCCOL nDestPosX, SCROW nDestPosY,
                                    const Point& rLogicPos, sal_Int8 nDndAction );

    void            HandleMouseButtonDown( const MouseEvent& rMEvt );

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
    //void          DrawStartTimer();

    void            DrawRedraw( ScOutputData& rOutputData, ScUpdateMode eMode, ULONG nLayer );
    void            DrawSdrGrid( const Rectangle& rDrawingRect, OutputDevice* pContentDev );
    //BOOL          DrawBeforeScroll();
    void            DrawAfterScroll(/*BOOL bVal*/);
    void            OutlinerViewPaint( const Rectangle& rRect );
    //void          DrawMarks();
    //BOOL          NeedDrawMarks();
    void            DrawComboButton( const Point&   rCellPos,
                                     long           nCellSizeX,
                                     long           nCellSizeY,
                                     BOOL           bArrowState,
                                     BOOL           bBtnIn  = FALSE );
    Rectangle       GetListValButtonRect( const ScAddress& rButtonPos );

    void            DrawPagePreview( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, OutputDevice* pContentDev );

    BOOL            GetEditUrl( const Point& rPos,
                                String* pName=0, String* pUrl=0, String* pTarget=0 );
    BOOL            GetEditUrlOrError( BOOL bSpellErr, const Point& rPos,
                                String* pName=0, String* pUrl=0, String* pTarget=0 );

    BOOL            HitRangeFinder( const Point& rMouse, BOOL& rCorner, USHORT* pIndex = NULL,
                                        SCsCOL* pAddX = NULL, SCsROW* pAddY = NULL );

    USHORT          HitPageBreak( const Point& rMouse, ScRange* pSource = NULL,
                                    SCCOLROW* pBreak = NULL, SCCOLROW* pPrev = NULL );

    void            PasteSelection( const Point& rPosPixel );

    void            SelectForContextMenu( const Point& rPosPixel );

    void            GetSelectionRects( ::std::vector< Rectangle >& rPixelRects );

protected:
    using Window::Resize;
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

    void            DoAutoFilterMenue( SCCOL nCol, SCROW nRow, BOOL bDataSelect );
    void            DoScenarioMenue( const ScRange& rScenRange );
    void            DoPageFieldMenue( SCCOL nCol, SCROW nRow );

    BOOL            HasPageFieldData( SCCOL nCol, SCROW nRow ) const;

    void            DrawButtons( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    ScTableInfo& rTabInfo, OutputDevice* pContentDev );

    using Window::Draw;
    void            Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                        ScUpdateMode eMode = SC_UPDATE_ALL );

    void            InvertSimple( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    BOOL bTestMerge = FALSE, BOOL bRepeat = FALSE );

    void            DrawDragRect( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );

    void            DrawRefMark( SCCOL nRefStartX, SCROW nRefStartY,
                                    SCCOL nRefEndX, SCROW nRefEndY,
                                    const Color& rColor, BOOL bHandle );

    void            CreateAnchorHandle(SdrHdlList& rHdl, const ScAddress& rAddress);

    void            HideCursor();
    void            ShowCursor();
    void            DrawCursor();
    void            DrawAutoFillMark();
    void            UpdateAutoFillMark(BOOL bMarked, const ScRange& rMarkRange);

    void            UpdateListValPos( BOOL bVisible, const ScAddress& rPos );

    BOOL            ShowNoteMarker( SCsCOL nPosX, SCsROW nPosY, BOOL bKeyboard );
    void            HideNoteMarker();

    MapMode         GetDrawMapMode( BOOL bForce = FALSE );

    void            ContinueDrag();

    void            StopMarking();
    void            UpdateInputContext();

    void            CheckInverted()     { if (nPaintCount) bNeedsRepaint = TRUE; }

    void            DoInvertRect( const Rectangle& rPixel );

    void            CheckNeedsRepaint();

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

