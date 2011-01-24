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
#ifndef _FESH_HXX
#define _FESH_HXX

#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>

#include <svx/svdobj.hxx>
#include "swdllapi.h"
#include <editsh.hxx>
#include <flyenum.hxx>

#include <svx/svdtypes.hxx>

#include <svtools/embedhlp.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif


class SwFlyFrm;
class SwTabCols;
class SvxBrushItem;
class SvxFrameDirectionItem;
class SwTableAutoFmt;
class SwFrm;
class SwFmtFrmSize;
class SwFmtRowSplit;
class SvxBorderLine;
class SdrObject;
class Color;
class Outliner;
class SotDataObject;
class SwFrmFmt;
struct SwSortOptions;
class SdrMarkList;

namespace svx
{
    class ISdrObjectFilter;
}


// return values for GetFrmType() und GetSelFrmType().
//! values can be combined via logival or
#define FRMTYPE_NONE            (USHORT)     0
#define FRMTYPE_PAGE            (USHORT)     1
#define FRMTYPE_HEADER          (USHORT)     2
#define FRMTYPE_FOOTER          (USHORT)     4
#define FRMTYPE_BODY            (USHORT)     8
#define FRMTYPE_COLUMN          (USHORT)    16
#define FRMTYPE_TABLE           (USHORT)    32
#define FRMTYPE_FLY_FREE        (USHORT)    64
#define FRMTYPE_FLY_ATCNT       (USHORT)   128
#define FRMTYPE_FLY_INCNT       (USHORT)   256
#define FRMTYPE_FOOTNOTE        (USHORT)   512
#define FRMTYPE_FTNPAGE         (USHORT)  1024
#define FRMTYPE_FLY_ANY         (USHORT)  2048
#define FRMTYPE_DRAWOBJ         (USHORT)  4096
#define FRMTYPE_COLSECT         (USHORT)  8192
#define FRMTYPE_COLSECTOUTTAB   (USHORT) 16384

#define FRMTYPE_ANYCOLSECT ( FRMTYPE_COLSECT | FRMTYPE_COLSECTOUTTAB )

//! values can be combined via logival or
#define GOTOOBJ_DRAW_CONTROL    (USHORT)  1
#define GOTOOBJ_DRAW_SIMPLE     (USHORT)  2
#define GOTOOBJ_DRAW_ANY        (USHORT)  3
#define GOTOOBJ_FLY_FRM         (USHORT)  4
#define GOTOOBJ_FLY_GRF         (USHORT)  8
#define GOTOOBJ_FLY_OLE         (USHORT) 16
#define GOTOOBJ_FLY_ANY         (USHORT) 28
#define GOTOOBJ_GOTO_ANY        (USHORT) 31

//! values can be combined via logival or
#define FLYPROTECT_CONTENT      (USHORT)  1
#define FLYPROTECT_SIZE         (USHORT)  2
#define FLYPROTECT_POS          (USHORT)  4
#define FLYPROTECT_PARENT       (USHORT)  8     // Check only parents.
#define FLYPROTECT_FIXED        (USHORT) 16     // Only protection that cannot be withdrawn
                                                // e.g. by OLE-server; also relevant for dialog.

// For figuring out contents by position (D&D)
enum ObjCntType
{
    OBJCNT_NONE,
    OBJCNT_FLY,
    OBJCNT_GRF,
    OBJCNT_OLE,
    OBJCNT_SIMPLE,
    OBJCNT_CONTROL,
    OBJCNT_URLBUTTON,

    OBJCNT_GROUPOBJ,
    OBJCNT_DONTCARE     // Not determinable - different objects are selected.
};

//For GetAnyCurRect
enum CurRectType
{
    RECT_PAGE,                  // Rect of current page.
    RECT_PAGE_CALC,             // ... page will be formated if required.
    RECT_PAGE_PRT,              // Rect of current PrtArea of page.
    RECT_FRM,                   // Rect of current frame.
    RECT_FLY_EMBEDDED,          // Rect of current FlyFrm.
    RECT_FLY_PRT_EMBEDDED,      // Rect of PrtArea of FlyFrm
    RECT_SECTION,               // Rect of current section.
    RECT_OUTTABSECTION,         // Rect of current section but outside of table.
    RECT_SECTION_PRT,           // Rect of current PrtArea of section.
    RECT_OUTTABSECTION_PRT,     // Rect of current PrtArea of section but outside table.
    RECT_HEADERFOOTER,          // Rect of current header/footer
    RECT_HEADERFOOTER_PRT,      // Rect of PrtArea of current headers/footers

    RECT_PAGES_AREA             //Rect covering the pages area
};

struct SwGetCurColNumPara
{
    const SwFrmFmt* pFrmFmt;
    const SwRect* pPrtRect, *pFrmRect;
    SwGetCurColNumPara() : pFrmFmt( 0 ), pPrtRect( 0 ), pFrmRect( 0 ) {}
};

#define SW_PASTESDR_INSERT      1
#define SW_PASTESDR_REPLACE     2
#define SW_PASTESDR_SETATTR     3

#define SW_ADD_SELECT   1
#define SW_ENTER_GROUP  2
#define SW_LEAVE_FRAME  4

#define SW_MOVE_UP      0
#define SW_MOVE_DOWN    1
#define SW_MOVE_LEFT    2
#define SW_MOVE_RIGHT   3

#define SW_TABCOL_NONE          0
#define SW_TABCOL_HORI          1
#define SW_TABCOL_VERT          2
#define SW_TABROW_HORI          3
#define SW_TABROW_VERT          4

#define SW_TABSEL_HORI          5
#define SW_TABSEL_HORI_RTL      6
#define SW_TABROWSEL_HORI       7
#define SW_TABROWSEL_HORI_RTL   8
#define SW_TABCOLSEL_HORI       9
#define SW_TABSEL_VERT          10
#define SW_TABROWSEL_VERT       11
#define SW_TABCOLSEL_VERT       12

class SdrDropMarkerOverlay;

class SW_DLLPUBLIC SwFEShell : public SwEditShell
{
    SdrDropMarkerOverlay *pChainFrom, *pChainTo;
    BOOL bCheckForOLEInCaption;

    SW_DLLPRIVATE SwFlyFrm *FindFlyFrm() const;
    SW_DLLPRIVATE SwFlyFrm *FindFlyFrm( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&  ) const;

    // Terminate actions for all shells and call ChangeLink.
    SW_DLLPRIVATE void EndAllActionAndCall();

    SW_DLLPRIVATE void ScrollTo( const Point &rPt );

    SW_DLLPRIVATE void ChangeOpaque( SdrLayerID nLayerId );

    // Used for mouse operations on a table:
    // Returns a cell frame that is 'close' to rPt.
    SW_DLLPRIVATE const SwFrm *GetBox( const Point &rPt, bool* pbRow = 0, bool* pbCol = 0 ) const;

    // 0 == not in any column.
    SW_DLLPRIVATE USHORT _GetCurColNum( const SwFrm *pFrm,
                          SwGetCurColNumPara* pPara ) const;

    SW_DLLPRIVATE void _GetTabCols( SwTabCols &rToFill, const SwFrm *pBox ) const;
    SW_DLLPRIVATE void _GetTabRows( SwTabCols &rToFill, const SwFrm *pBox ) const;

    SW_DLLPRIVATE BOOL ImpEndCreate();

    SW_DLLPRIVATE ObjCntType GetObjCntType( const SdrObject& rObj ) const;

    // Methods for copying of draw objects.
    SW_DLLPRIVATE BOOL CopyDrawSel( SwFEShell* pDestShell, const Point& rSttPt,
                                const Point& rInsPt, BOOL bIsMove,
                                BOOL bSelectInsert );

    // Get list of marked SdrObjects;
    // helper method for GetSelFrmType, IsSelContainsControl.
    SW_DLLPRIVATE const SdrMarkList* _GetMarkList() const;

    SW_DLLPRIVATE BOOL CheckHeadline( bool bRepeat ) const;

    using SwEditShell::Copy;

public:

    using SwEditShell::Insert;

    TYPEINFO();
    SwFEShell( SwDoc& rDoc, Window *pWin, const SwViewOption *pOpt = 0 );
    SwFEShell( SwEditShell& rShell, Window *pWin );
    virtual ~SwFEShell();

    // Copy and Paste methods for internal clipboard.
    BOOL Copy( SwDoc* pClpDoc, const String* pNewClpTxt = 0 );
    BOOL Paste( SwDoc* pClpDoc, BOOL bIncludingPageFrames = sal_False);

    //Paste some pages into another doc - used in mailmerge.
    BOOL PastePages( SwFEShell& rToFill, USHORT nStartPage, USHORT nEndPage);

    // Copy-Method for Drag&Drop
    BOOL Copy( SwFEShell*, const Point& rSttPt, const Point& rInsPt,
                BOOL bIsMove = FALSE, BOOL bSelectInsert = TRUE );

    void SelectFlyFrm( SwFlyFrm& rFrm, BOOL bNew = FALSE );

    // Is selected frame within another frame?
    const SwFrmFmt* IsFlyInFly();


    // If an object as been given, exactly this object is selected
    // (instead of searching over position).
    BOOL SelectObj( const Point& rSelPt, BYTE nFlag = 0, SdrObject *pObj = 0 );
    void DelSelectedObj();

    // Move selection upwards or downwards (Z-Order).
    // TRUE = to top or bottom.
    // FALSE = run past one other.
    void SelectionToTop   ( BOOL bTop = TRUE );
    void SelectionToBottom( BOOL bBottom = TRUE );

    short GetLayerId() const;   // 1 Heaven, 0 Hell, -1 Ambiguous.
    void  SelectionToHeaven();  // Above document.
    void  SelectionToHell();    // Below document.

    // The following two methods return enum SdrHdlKind.
    // Declared as int in order to spare including SVDRAW.HXX.
    bool IsObjSelectable( const Point& rPt );
    int IsInsideSelectedObj( const Point& rPt );    //!! returns enum values

    // Test if there is a draw object at that position and if it should be selected.
    // The 'should' is aimed at Writer text fly frames which may be in front of
    // the draw object.
    sal_Bool ShouldObjectBeSelected(const Point& rPt);

    sal_Bool MoveAnchor( USHORT nDir );

    // Returns if Upper of frame at current position is section frame
    // Currently only used by the rules. To be replaced by something more
    // sophisticated one day.
    bool IsDirectlyInSection() const;

    // For return valies see above FrmType.
    // pPt: Cursr or DocPos respectively; bStopAtFly: Stop at flys or continue over anchor.
    // Although (0,TRUE) is kind of a standard, the parameters are not defaulted here
    // in order to force more conscious use especially of bStopAtFly.
    USHORT GetFrmType( const Point *pPt, BOOL bStopAtFly ) const;
    USHORT GetSelFrmType() const;               //Selektion (Drawing)

    /** check whether selected frame contains a control;
     * companion method to GetSelFrmType, used for preventing
     * drag&drop of controls into header */
    bool IsSelContainsControl() const;

    ObjCntType GetObjCntType( const Point &rPt, SdrObject *&rpObj ) const;
    ObjCntType GetObjCntTypeOfSelection( SdrObject** ppObj = 0 ) const;

    // For adjustment of PosAttr when anchor changes.
    SwRect  GetObjRect() const;

    // For moving flys with keyboard.
    SwRect  GetFlyRect() const;
    // i#17567 - adjustments to allow negative vertical positions for fly frames anchored
    //          to paragraph or to character.
    // i#18732 - adjustments for new option 'FollowTextFlow'
    // i#22341 - adjustments for new vertical alignment at top of line
    void CalcBoundRect( SwRect& _orRect,
                        const RndStdIds _nAnchorId,
                        const sal_Int16 _eHoriRelOrient = com::sun::star::text::RelOrientation::FRAME,
                        const sal_Int16 _eVertRelOrient = com::sun::star::text::RelOrientation::FRAME,
                        const SwPosition* _pToCharCntntPos = NULL,
                        const bool _bFollowTextFlow = false,
                        bool _bMirror = false,
                        Point* _opRef = NULL,
                        Size* _opPercent = NULL ) const;

    // Set size of draw objects.
    void SetObjRect( const SwRect& rRect );

    long BeginDrag( const Point *pPt, BOOL bProp );
    long Drag     ( const Point *pPt, BOOL bProp );
    long EndDrag  ( const Point *pPt, BOOL bProp );
    void BreakDrag();

    //Methods for status line.
    Point GetAnchorObjDiff() const;
    Point GetObjAbsPos()     const;
    Size  GetObjSize()       const;

    // SS for envelopes: get all page-bound objects and set them to new page.
    void GetPageObjs( SvPtrarr& rFillArr );
    void SetPageObjsNewPage( SvPtrarr& rFillArr, int nOffset = 1 );

    // Show current selection (frame / draw object as required).
    virtual void MakeSelVisible();

    // Return FrmFmt of object that may be under Point.
    // Object does not become selected!
    const SwFrmFmt* GetFmtFromObj( const Point& rPt, SwRect** pRectToFill = 0 ) const;

    // Returns a format too, if the point is over the text of any fly.
    const SwFrmFmt* GetFmtFromAnyObj( const Point& rPt ) const;

    // Which Protection is set at selected object?
    //!! Returns several flags in BYTE.
    BYTE IsSelObjProtected( USHORT /*FLYPROTECT_...*/ eType ) const;

    // Deliver graphic in rName besides graphic name. If graphic is
    // linked give name with path. rbLink is TRUE if graphic is linked.
    const Graphic *GetGrfAtPos( const Point &rDocPos,
                                String &rName, BOOL &rbLink ) const;

    const String GetObjTitle() const;
    void SetObjTitle( const String& rTitle );
    const String GetObjDescription() const;
    void SetObjDescription( const String& rDescription );


    BOOL IsFrmSelected() const;
    BOOL GetFlyFrmAttr( SfxItemSet &rSet ) const;
    BOOL SetFlyFrmAttr( SfxItemSet &rSet );
    BOOL ResetFlyFrmAttr( USHORT nWhich, const SfxItemSet* pSet = 0 );
    const SwFrmFmt *NewFlyFrm( const SfxItemSet &rSet, BOOL bAnchValid = FALSE,
                         SwFrmFmt *pParent = 0 );
    void SetFlyPos( const Point &rAbsPos);
    Point FindAnchorPos( const Point &rAbsPos, BOOL bMoveIt = FALSE );

    // Determines whether a frame or its environment is vertically formatted and right-to-left.
    BOOL IsFrmVertical(BOOL bEnvironment, BOOL& bRightToLeft) const;

    SwFrmFmt* GetCurFrmFmt() const; //If frame then frame style, else 0.
    void SetFrmFmt( SwFrmFmt *pFmt, BOOL bKeepOrient = FALSE, Point* pDocPos = 0 ); //If frame then set frame style.
    const SwFlyFrm *GetCurrFlyFrm() const { return FindFlyFrm(); }

    // Find/delete fly containing the cursor.
    SwFrmFmt* WizzardGetFly();

    // Independent selecting of flys.
    BOOL GotoNextFly( USHORT /*GOTOOBJ_...*/ eType = GOTOOBJ_FLY_ANY )
                                { return GotoObj( TRUE, eType ); }
    BOOL GotoPrevFly( USHORT /*GOTOOBJ_...*/ eType = GOTOOBJ_FLY_ANY)
                                { return GotoObj( FALSE, eType); }

   // Iterate over flys  - for Basic-collections.
    USHORT GetFlyCount( FlyCntType eType = FLYCNTTYPE_ALL ) const;
    const SwFrmFmt* GetFlyNum(USHORT nIdx, FlyCntType eType = FLYCNTTYPE_ALL) const;

    // If a fly is selected, it draws cursor into the first CntntFrm.
    const SwFrmFmt* SelFlyGrabCrsr();

    //Get FlyFrameFormat; fuer UI Macro Anbindung an Flys
    const SwFrmFmt* GetFlyFrmFmt() const;
          SwFrmFmt* GetFlyFrmFmt();

    // OLE. Server requires new size. Desired values are adjusted as frame attributes.
    // If the values are not allowed, the formating clips and determines scaling.
    // See CalcAndSetScale().
    // The return value is the applied size.
    Size RequestObjectResize( const SwRect &rRect, const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );

    //The layout has been changed, so the active object has to be moved after that
    virtual void MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset );

    // Client for OleObject has to be up-to-date regarding scaling.
    // Implemented in WrtShell.
    // If a pointer is passed on a size, this is the object's current core-size.
    // Else the size is provided via GetCurFlyRect().
    virtual void CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect = 0,
                                  const SwRect *pFlyFrmRect = 0 ) = 0;

    // Connect objects with ActivateWhenVisible at Paint.
    // Called by notxtfrm::Paint, implemented in wrtsh.
    virtual void ConnectObj( svt::EmbeddedObjectRef&,
                             const SwRect &rPrt,
                             const SwRect &rFrm ) = 0;

    // Set visible range on object, if it is not yet visible.
    void MakeObjVisible( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& ) const;

    // Check resize of OLE-Object.
    BOOL IsCheckForOLEInCaption() const         { return bCheckForOLEInCaption; }
    void SetCheckForOLEInCaption( BOOL bFlag )  { bCheckForOLEInCaption = bFlag; }

    // Set name at selected FlyFrame.
    void SetFlyName( const String& rName );
    const String& GetFlyName() const;

    // Created unique name for frame.
    String GetUniqueGrfName() const;
    String GetUniqueOLEName() const;
    String GetUniqueFrameName() const;

    // Jump to named Fly (graphic/OLE).
    BOOL GotoFly( const String& rName, FlyCntType eType = FLYCNTTYPE_ALL,
                    BOOL bSelFrame = TRUE );

    // Position is a graphic with URL?
    const SwFrmFmt* IsURLGrfAtPos( const Point& rPt, String* pURL = 0,
                                    String *pTargetFrameName = 0,
                                    String *pURLDescription = 0 ) const;

    // For Chain always connect Fly specified by format with that hit by point.
    // rRect contains rect of Fly (for its highlight).
    int Chainable( SwRect &rRect, const SwFrmFmt &rSource, const Point &rPt ) const;
    int Chain( SwFrmFmt &rSource, const Point &rPt );
    int Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest );
    void Unchain( SwFrmFmt &rFmt );
    void HideChainMarker();
    void SetChainMarker();

    Size GetGraphicDefaultSize() const;

    // Temporary work around for bug.
    void CheckUnboundObjects();

    // Attention: Ambiguities if multiple selections.
    BOOL GetObjAttr( SfxItemSet &rSet ) const;
    BOOL SetObjAttr( const SfxItemSet &rSet );

    const SdrObject* GetBestObject( BOOL bNext, USHORT eType = GOTOOBJ_DRAW_ANY, BOOL bFlat = TRUE, const ::svx::ISdrObjectFilter* pFilter = NULL );
    BOOL GotoObj( BOOL bNext, USHORT /*GOTOOBJ_...*/ eType = GOTOOBJ_DRAW_ANY);

    // Set DragMode (e.g. Rotae), but do nothing when frame is selected.
    void SetDragMode( UINT16 eSdrDragMode );

    USHORT IsObjSelected() const;   // Returns object count, but doesn't count the objects in groups.
    sal_Bool IsObjSelected( const SdrObject& rObj ) const;

    void EndTextEdit();             // Deletes object if required.

    // Anchor type of selected object, -1 if ambiguous or in case of frame selection.
    // Else FLY_AT_PAGE or FLY_AT_PARA resp. from frmatr.hxx.
    short GetAnchorId() const;

    // Process of creating draw objects. At the beginning object type is passed.
    // At the end a Cmd can be passed. Here, SDRCREATE_RESTRAINTEND for end
    // or SDRCREATE_NEXTPOINT for a polygon may be relevant.
    // After RESTRAINTEND the object is created and selected.
    // BreakCreate interrupts the process. In this case no object is selected.
    BOOL BeginCreate( UINT16 /*SdrObjKind ?*/ eSdrObjectKind, const Point &rPos );
    BOOL BeginCreate( UINT16 /*SdrObjKind ?*/ eSdrObjectKind, UINT32 eObjInventor, const Point &);
    void MoveCreate ( const Point &rPos );
    BOOL EndCreate  ( UINT16 eSdrCreateCmd );
    void BreakCreate();
    BOOL IsDrawCreate() const;
    void CreateDefaultShape( UINT16 /*SdrObjKind ?*/ eSdrObjectKind, const Rectangle& rRect, USHORT nSlotId);

    // Functions for Rubberbox, ti select Draw-Objects
    BOOL BeginMark( const Point &rPos );
    void MoveMark ( const Point &rPos );
    BOOL EndMark  ();
    void BreakMark();

    // Create and destroy group, don't when frame is selected.
    BOOL IsGroupSelected();     // Can be a mixed selection!
    void GroupSelection();      // Afterwards the group is selected.
    void UnGroupSelection();    // The individual objects are selected, but
                                // it is possible that there are groups included.

    bool IsGroupAllowed() const;

    void MirrorSelection( BOOL bHorizontal );   //Vertical if FALSE.

    // frmatr.hxx. Here no enum because of dependencies.
    // BOOL value only for internal use! Anchor is newly set according
    // to current document position. Anchor is not re-set.
    void ChgAnchor( int eAnchorId, BOOL bSameOnly = FALSE,
                                   BOOL bPosCorr = TRUE );

    BOOL SetDrawingAttr( SfxItemSet &rSet );

    // Get selected DrawObj as graphics (MetaFile/Bitmap).
    // Return value indicates if it was converted.
    BOOL GetDrawObjGraphic( ULONG nFmt, Graphic& rGrf ) const;

    void Paste( SvStream& rStm, USHORT nAction, const Point* pPt = 0 );
    BOOL Paste( const Graphic &rGrf );
    BOOL Paste( SotDataObject& rObj, const Point& rPt );

    BOOL IsAlignPossible() const;
    void SetCalcFieldValueHdl(Outliner* pOutliner);

    void Insert(const String& rGrfName,
                const String& rFltName,
                const Graphic* pGraphic = 0,
                const SfxItemSet* pFlyAttrSet = 0,
                const SfxItemSet* pGrfAttrSet = 0,
                SwFrmFmt* = 0 );

    // Insertion of a drawing object which have to be already inserted in the DrawModel.
    void InsertDrawObj( SdrObject& rDrawObj,
                        const Point& rInsertPosition );

    BOOL ReplaceSdrObj( const String& rGrfName, const String& rFltName,
                        const Graphic* pGrf = 0 );



    // Provide information about content situated closes to given Point.
    Point GetCntntPos( const Point& rPoint, BOOL bNext ) const;

    // Convert document position into position relative to the current page.
    Point GetRelativePagePosition(const Point& rDocPos);

    // Hide or show layout-selection and pass call to CrsrSh.
    void ShLooseFcs();
    void ShGetFcs( BOOL bUpdate = TRUE );

    // PageDescriptor-interface
    void   ChgCurPageDesc( const SwPageDesc& );
    USHORT GetCurPageDesc( const BOOL bCalcFrm = TRUE ) const;
    USHORT GetMousePageDesc( const Point &rPt ) const;
    USHORT GetPageDescCnt() const;
    SwPageDesc* FindPageDescByName( const String& rName,
                                    BOOL bGetFromPool = FALSE,
                                    USHORT* pPos = 0 );

    const SwPageDesc& GetPageDesc( USHORT i ) const;
    void  ChgPageDesc( USHORT i, const SwPageDesc& );
    // if inside all selection only one PageDesc, return this.
    // Otherwise return 0 pointer
    const SwPageDesc* GetSelectedPageDescs() const;

    const SwRect& GetAnyCurRect( CurRectType eType,
                                 const Point* pPt = 0,
                                 const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& =
                                 ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >() ) const;


    // Page number of the page containing Point, O if no page.
    USHORT GetPageNumber( const Point &rPoint ) const;
    BOOL GetPageNumber( long nYPos, BOOL bAtCrsrPos, USHORT& rPhyNum, USHORT& rVirtNum, String &rDisplay ) const;

    SwFlyFrmFmt* InsertObject( const svt::EmbeddedObjectRef&,
                const SfxItemSet* pFlyAttrSet = 0,
                const SfxItemSet* pGrfAttrSet = 0,
                SwFrmFmt* = 0 );
    BOOL    FinishOLEObj(); //Shutdown server.

    void GetTblAttr( SfxItemSet & ) const;
    void SetTblAttr( const SfxItemSet & );

    BOOL HasWholeTabSelection() const;

    // Is content of a table cell or at least a table cell completely selected?
    BOOL HasBoxSelection() const;

    BOOL InsertRow( USHORT nCnt, BOOL bBehind );
    BOOL InsertCol( USHORT nCnt, BOOL bBehind );  // 0 == at the end.
    BOOL DeleteCol();
    BOOL DeleteRow();

    BOOL DeleteTblSel();        // Current selection, may be whole table.

    USHORT MergeTab();          // Merge selected parts of table.
                                // Return error via enum.

    // Split cell vertically or horizontally.
    BOOL SplitTab( BOOL nVert = TRUE, USHORT nCnt = 1, BOOL bSameHeight = FALSE );
    BOOL Sort(const SwSortOptions&);

    void SetRowHeight( const SwFmtFrmSize &rSz );

    // Pointer must be detroyed by caller != 0.
    void GetRowHeight( SwFmtFrmSize *&rpSz ) const;

    void SetRowSplit( const SwFmtRowSplit &rSz );
    void GetRowSplit( SwFmtRowSplit *&rpSz ) const;

    void   SetBoxAlign( USHORT nOrient );
    USHORT GetBoxAlign() const;         // USHRT_MAX if ambiguous.

    // Adjustment of Rowheights. Determine via bTstOnly if more than one row is selected.
    BOOL BalanceRowHeight( BOOL bTstOnly );

    void SetTabBorders( const SfxItemSet& rSet );
    void GetTabBorders(       SfxItemSet& rSet) const;
    void SetTabLineStyle(const Color* pColor, BOOL bSetLine = FALSE, const SvxBorderLine* pBorderLine = NULL);

    void SetTabBackground( const SvxBrushItem &rNew );
    void GetTabBackground( SvxBrushItem &rToFill ) const;

    void SetBoxBackground( const SvxBrushItem &rNew );
    BOOL GetBoxBackground( SvxBrushItem &rToFill ) const; //FALSE ambiguous.

    void SetBoxDirection( const SvxFrameDirectionItem& rNew );
    BOOL GetBoxDirection( SvxFrameDirectionItem& rToFill ) const; //FALSE ambiguous.

    void SetRowBackground( const SvxBrushItem &rNew );
    BOOL GetRowBackground( SvxBrushItem &rToFill ) const; //FALSE ambiguous.

    BYTE WhichMouseTabCol( const Point &rPt ) const;
    void GetTabCols( SwTabCols &rToFill ) const; // Info about columns and margins.
    void SetTabCols( const SwTabCols &rNew, BOOL bCurRowOnly = TRUE );
    void GetMouseTabCols( SwTabCols &rToFill, const Point &rPt ) const;
    void SetMouseTabCols( const SwTabCols &rNew, BOOL bCurRowOnly,
                          const Point &rPt );

    // pEnd will be used during MouseMove
    bool SelTblRowCol( const Point& rPt, const Point* pEnd = 0, bool bRowDrag = false );

    void GetTabRows( SwTabCols &rToFill ) const;
    void SetTabRows( const SwTabCols &rNew, BOOL bCurColOnly );
    void GetMouseTabRows( SwTabCols &rToFill, const Point &rPt ) const;
    void SetMouseTabRows( const SwTabCols &rNew, BOOL bCurColOnly, const Point &rPt );

    void ProtectCells();    // If a table selection exists it is destroyed in case
                            // cursor is not allowed in readonly.
    void UnProtectCells();  // Refers to table selection.
    void UnProtectTbls();   // Unprotect all tables in selection.
    BOOL HasTblAnyProtection( const String* pTblName = 0,
                                BOOL* pFullTblProtection = 0 );
    BOOL CanUnProtectCells() const;

    USHORT GetRowsToRepeat() const;
    void SetRowsToRepeat( USHORT nNumOfRows );
    USHORT GetVirtPageNum( const BOOL bCalcFrm = TRUE );

    //Returns the number of table rows currently selected
    //if the selection start at the top of the table.
    USHORT    GetRowSelectionFromTop() const;

    BOOL IsInRepeatedHeadline() const { return CheckHeadline( true ); }
    BOOL IsInHeadline() const { return CheckHeadline( false ); }

    // Adjusts cell widths in such a way, that their content
    // does not need to be wrapped (if possible).
    // bBalance provides for adjustment of selected columns.
    void AdjustCellWidth( BOOL bBalance = FALSE );

    // Not allowed if only empty cells are selected.
    BOOL IsAdjustCellWidthAllowed( BOOL bBalance = FALSE ) const;

    // Adjustment of cell-widths; determine via bTstOnly if more than one cell is selected.
    BOOL BalanceCellWidth( BOOL bTstOnly );

    // AutoFormat for table/ table selection.
    BOOL SetTableAutoFmt( const SwTableAutoFmt& rNew );

    BOOL GetTableAutoFmt( SwTableAutoFmt& rGet );

    BOOL SetColRowWidthHeight( USHORT eType, USHORT nDiff = 283 );

    BOOL GetAutoSum( String& rFml ) const;

    // Phy: real page count.
    // Virt: consider offset that may have been set by user.
    USHORT  GetPhyPageNum();

    void SetNewPageOffset( USHORT nOffset );
    void SetPageOffset( USHORT nOffset );   //Changes last page offset.
    USHORT GetPageOffset() const;           //Returns last page offset.

    void InsertLabel( const SwLabelType eType, const String &rTxt, const String& rSeparator,
                      const String& rNumberSeparator,
                      const BOOL bBefore, const USHORT nId,
                      const String& rCharacterStyle,
                      const BOOL bCpyBrd = TRUE );

    // The ruler needs some information too.
    USHORT GetCurColNum( SwGetCurColNumPara* pPara = 0 ) const; //0 == not in any column.
    USHORT GetCurMouseColNum( const Point &rPt,
                            SwGetCurColNumPara* pPara = 0 ) const;
    USHORT GetCurTabColNum() const;     //0 == not in any table.
    USHORT GetCurMouseTabColNum( const Point &rPt ) const;
    USHORT GetCurOutColNum( SwGetCurColNumPara* pPara = 0 ) const;  // Current outer column.

    BOOL IsTableRightToLeft() const;
    BOOL IsMouseTableRightToLeft( const Point &rPt ) const;
    BOOL IsTableVertical() const;

    BOOL IsLastCellInRow() const;

    // Width of current range for column-dialog.
    long GetSectionWidth( SwFmt& rFmt ) const;

    void GetConnectableFrmFmts
    (SwFrmFmt & rFmt, const String & rReference, BOOL bSuccessors,
     ::std::vector< String > & aPrevPageVec,
     ::std::vector< String > & aThisPageVec,
     ::std::vector< String > & aNextPageVec,
     ::std::vector< String > & aRestVec);

    /** SwFEShell::GetShapeBackgrd

        method determines background color of the page the selected drawing
        object is on and returns this color.
        If no color is found, because no drawing object is selected or ...,
        color COL_BLACK (default color on constructing object of class Color)
        is returned.

        @author OD

        @returns an object of class Color
    */
    const Color GetShapeBackgrd() const;

    /** Is default horizontal text direction for selected drawing object right-to-left

        Because drawing objects only painted for each page only, the default
        horizontal text direction of a drawing object is given by the corresponding
        page property.

        @author OD

        @returns boolean, indicating, if the horizontal text direction of the
        page, the selected drawing object is on, is right-to-left.
    */
    bool IsShapeDefaultHoriTextDirR2L() const;

    void ParkCursorInTab();

    SwTxtNode * GetNumRuleNodeAtPos(const Point &rPot);
    BOOL IsNumLabel( const Point &rPt, int nMaxOffset = -1 );

    bool IsVerticalModeAtNdAndPos( const SwTxtNode& _rTxtNode,
                                   const Point& _rDocPos ) const;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
