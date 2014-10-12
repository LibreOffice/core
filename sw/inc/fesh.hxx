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
#ifndef INCLUDED_SW_INC_FESH_HXX
#define INCLUDED_SW_INC_FESH_HXX

#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svx/svdobj.hxx>
#include "swdllapi.h"
#include <editsh.hxx>
#include <flyenum.hxx>

#include <svx/svdtypes.hxx>
#include <rtl/ustring.hxx>
#include <svtools/embedhlp.hxx>

#include <vector>

namespace editeng { class SvxBorderLine; }

class SwFlyFrm;
class SwTabCols;
class SvxBrushItem;
class SvxFrameDirectionItem;
class SwTableAutoFmt;
class SwFrm;
class SwFmtFrmSize;
class SwFmtRowSplit;
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
#define FRMTYPE_NONE            (sal_uInt16)     0
#define FRMTYPE_PAGE            (sal_uInt16)     1
#define FRMTYPE_HEADER          (sal_uInt16)     2
#define FRMTYPE_FOOTER          (sal_uInt16)     4
#define FRMTYPE_BODY            (sal_uInt16)     8
#define FRMTYPE_COLUMN          (sal_uInt16)    16
#define FRMTYPE_TABLE           (sal_uInt16)    32
#define FRMTYPE_FLY_FREE        (sal_uInt16)    64
#define FRMTYPE_FLY_ATCNT       (sal_uInt16)   128
#define FRMTYPE_FLY_INCNT       (sal_uInt16)   256
#define FRMTYPE_FOOTNOTE        (sal_uInt16)   512
#define FRMTYPE_FTNPAGE         (sal_uInt16)  1024
#define FRMTYPE_FLY_ANY         (sal_uInt16)  2048
#define FRMTYPE_DRAWOBJ         (sal_uInt16)  4096
#define FRMTYPE_COLSECT         (sal_uInt16)  8192
#define FRMTYPE_COLSECTOUTTAB   (sal_uInt16) 16384

//! values can be combined via logival or
#define GOTOOBJ_DRAW_CONTROL    (sal_uInt16)  1
#define GOTOOBJ_DRAW_SIMPLE     (sal_uInt16)  2
#define GOTOOBJ_DRAW_ANY        (sal_uInt16)  3
#define GOTOOBJ_FLY_FRM         (sal_uInt16)  4
#define GOTOOBJ_FLY_GRF         (sal_uInt16)  8
#define GOTOOBJ_FLY_OLE         (sal_uInt16) 16
#define GOTOOBJ_FLY_ANY         (sal_uInt16) 28
#define GOTOOBJ_GOTO_ANY        (sal_uInt16) 31

//! values can be combined via logival or
#define FLYPROTECT_CONTENT      (sal_uInt16)  1
#define FLYPROTECT_SIZE         (sal_uInt16)  2
#define FLYPROTECT_POS          (sal_uInt16)  4
#define FLYPROTECT_PARENT       (sal_uInt16)  8     ///< Check only parents.
#define FLYPROTECT_FIXED        (sal_uInt16) 16     /**< Only protection that cannot be withdrawn
                                                    e.g. by OLE-server; also relevant for dialog. */

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
    OBJCNT_DONTCARE     ///< Not determinable - different objects are selected.
};

//For GetAnyCurRect
enum CurRectType
{
    RECT_PAGE,                  ///< Rect of current page.
    RECT_PAGE_CALC,             ///< ... page will be formated if required.
    RECT_PAGE_PRT,              ///< Rect of current PrtArea of page.
    RECT_FRM,                   ///< Rect of current frame.
    RECT_FLY_EMBEDDED,          ///< Rect of current FlyFrm.
    RECT_FLY_PRT_EMBEDDED,      ///< Rect of PrtArea of FlyFrm
    RECT_SECTION,               ///< Rect of current section.
    RECT_OUTTABSECTION,         ///< Rect of current section but outside of table.
    RECT_SECTION_PRT,           ///< Rect of current PrtArea of section.
    RECT_OUTTABSECTION_PRT,     ///< Rect of current PrtArea of section but outside table.
    RECT_HEADERFOOTER,          ///< Rect of current header/footer
    RECT_HEADERFOOTER_PRT,      ///< Rect of PrtArea of current headers/footers

    RECT_PAGES_AREA             ///< Rect covering the pages area
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

enum class SwMove
{
    UP      = 0,
    DOWN    = 1,
    LEFT    = 2,
    RIGHT   = 3
};

// return values for WhichMouseTabCol
enum class SwTab
{
    COL_NONE          = 0,
    COL_HORI          = 1,
    COL_VERT          = 2,
    ROW_HORI          = 3,
    ROW_VERT          = 4,
    SEL_HORI          = 5,
    SEL_HORI_RTL      = 6,
    ROWSEL_HORI       = 7,
    ROWSEL_HORI_RTL   = 8,
    COLSEL_HORI       = 9,
    SEL_VERT          = 10,
    ROWSEL_VERT       = 11,
    COLSEL_VERT       = 12
};

class SdrDropMarkerOverlay;

class SW_DLLPUBLIC SwFEShell : public SwEditShell
{
    SdrDropMarkerOverlay *pChainFrom, *pChainTo;
    bool bCheckForOLEInCaption;

    SAL_DLLPRIVATE SwFlyFrm *FindFlyFrm() const;
    SAL_DLLPRIVATE SwFlyFrm *FindFlyFrm( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >&  ) const;

    /// Terminate actions for all shells and call ChangeLink.
    SAL_DLLPRIVATE void EndAllActionAndCall();

    SAL_DLLPRIVATE void ScrollTo( const Point &rPt );

    SAL_DLLPRIVATE void ChangeOpaque( SdrLayerID nLayerId );

    /** Used for mouse operations on a table:
     @return a cell frame that is 'close' to rPt. */
    SAL_DLLPRIVATE const SwFrm *GetBox( const Point &rPt, bool* pbRow = 0, bool* pbCol = 0 ) const;

    // 0 == not in any column.
    SAL_DLLPRIVATE sal_uInt16 _GetCurColNum( const SwFrm *pFrm,
                          SwGetCurColNumPara* pPara ) const;

    SAL_DLLPRIVATE void _GetTabCols( SwTabCols &rToFill, const SwFrm *pBox ) const;
    SAL_DLLPRIVATE void _GetTabRows( SwTabCols &rToFill, const SwFrm *pBox ) const;

    SAL_DLLPRIVATE bool ImpEndCreate();

    SAL_DLLPRIVATE ObjCntType GetObjCntType( const SdrObject& rObj ) const;

    /// Methods for copying of draw objects.
    SAL_DLLPRIVATE bool CopyDrawSel( SwFEShell* pDestShell, const Point& rSttPt,
                                const Point& rInsPt, bool bIsMove,
                                bool bSelectInsert );

    /// Get list of marked SdrObjects;
    /// helper method for GetSelFrmType, IsSelContainsControl.
    SAL_DLLPRIVATE const SdrMarkList* _GetMarkList() const;

    SAL_DLLPRIVATE bool CheckHeadline( bool bRepeat ) const;

    using SwEditShell::Copy;

public:

    using SwEditShell::Insert;

    TYPEINFO_OVERRIDE();
    SwFEShell( SwDoc& rDoc, vcl::Window *pWin, const SwViewOption *pOpt = 0 );
    SwFEShell( SwEditShell& rShell, vcl::Window *pWin );
    virtual ~SwFEShell();

    /// Copy and Paste methods for internal clipboard.
    bool Copy( SwDoc* pClpDoc, const OUString* pNewClpTxt = 0 );
    bool Paste( SwDoc* pClpDoc, bool bIncludingPageFrames = false);

    /// Paste some pages into another doc - used in mailmerge.
    bool PastePages( SwFEShell& rToFill, sal_uInt16 nStartPage, sal_uInt16 nEndPage);

    /// Copy-Method for Drag&Drop
    bool Copy( SwFEShell*, const Point& rSttPt, const Point& rInsPt,
               bool bIsMove = false, bool bSelectInsert = true );

    void SelectFlyFrm( SwFlyFrm& rFrm, bool bNew = false );

    /// Is selected frame within another frame?
    const SwFrmFmt* IsFlyInFly();

    /** If an object as been given, exactly this object is selected
     (instead of searching over position). */
    bool SelectObj( const Point& rSelPt, sal_uInt8 nFlag = 0, SdrObject *pObj = 0 );
    void DelSelectedObj();

    /** Move selection upwards or downwards (Z-Order).
     TRUE = to top or bottom.
     FALSE = run past one other. */
    void SelectionToTop   ( bool bTop = true );
    void SelectionToBottom( bool bBottom = true );

    short GetLayerId() const;   ///< 1 Heaven, 0 Hell, -1 Ambiguous.
    void  SelectionToHeaven();  ///< Above document.
    void  SelectionToHell();    ///< Below document.

    /** The following two methods return enum SdrHdlKind.
     Declared as int in order to spare including SVDRAW.HXX. */
    bool IsObjSelectable( const Point& rPt );
    /// Same as IsObjSelectable(), but return the object as well.
    SdrObject* GetObjAt(const Point& rPt);
    int IsInsideSelectedObj( const Point& rPt );    ///< returns enum values
    /** Test if there is a draw object at that position and if it should be selected.
     The 'should' is aimed at Writer text fly frames which may be in front of
     the draw object. */
    bool ShouldObjectBeSelected(const Point& rPt);

    bool MoveAnchor( SwMove nDir );

    /** @return if Upper of frame at current position is section frame
     Currently only used by the rules. To be replaced by something more
     sophisticated one day. */
    bool IsDirectlyInSection() const;

    /** For return valies see above FrmType.
     pPt: Cursr or DocPos respectively; bStopAtFly: Stop at flys or continue over anchor.
     Although (0,TRUE) is kind of a standard, the parameters are not defaulted here
     in order to force more conscious use especially of bStopAtFly. */
    sal_uInt16 GetFrmType( const Point *pPt, bool bStopAtFly ) const;
    sal_uInt16 GetSelFrmType() const;               //Selektion (Drawing)

    /** check whether selected frame contains a control;
     * companion method to GetSelFrmType, used for preventing
     * drag&drop of controls into header */
    bool IsSelContainsControl() const;

    ObjCntType GetObjCntType( const Point &rPt, SdrObject *&rpObj ) const;
    ObjCntType GetObjCntTypeOfSelection( SdrObject** ppObj = 0 ) const;

    /// For adjustment of PosAttr when anchor changes.
    SwRect  GetObjRect() const;

    /// For moving flys with keyboard.
    SwRect  GetFlyRect() const;
    /** i#17567 - adjustments to allow negative vertical positions for fly frames anchored
              to paragraph or to character.
     i#18732 - adjustments for new option 'FollowTextFlow'
     i#22341 - adjustments for new vertical alignment at top of line */
    void CalcBoundRect( SwRect& _orRect,
                        const RndStdIds _nAnchorId,
                        const sal_Int16 _eHoriRelOrient = com::sun::star::text::RelOrientation::FRAME,
                        const sal_Int16 _eVertRelOrient = com::sun::star::text::RelOrientation::FRAME,
                        const SwPosition* _pToCharCntntPos = NULL,
                        const bool _bFollowTextFlow = false,
                        bool _bMirror = false,
                        Point* _opRef = NULL,
                        Size* _opPercent = NULL,
                        const SwFmtFrmSize* pFmtFrmSize = 0 ) const;

    /// Set size of draw objects.
    void SetObjRect( const SwRect& rRect );

    long BeginDrag( const Point *pPt, bool bProp );
    long Drag     ( const Point *pPt, bool bProp );
    long EndDrag  ( const Point *pPt, bool bProp );
    void BreakDrag();

    /// Methods for status line.
    Point GetAnchorObjDiff() const;
    Point GetObjAbsPos()     const;
    Size  GetObjSize()       const;

    /// SS for envelopes: get all page-bound objects and set them to new page.
    void GetPageObjs( std::vector<SwFrmFmt*>& rFillArr );
    void SetPageObjsNewPage( std::vector<SwFrmFmt*>& rFillArr, int nOffset = 1 );

    /// Show current selection (frame / draw object as required).
    virtual void MakeSelVisible() SAL_OVERRIDE;

    /** @return FrmFmt of object that may be under Point.
     Object does not become selected! */
    const SwFrmFmt* GetFmtFromObj( const Point& rPt, SwRect** pRectToFill = 0 ) const;

    /// @return a format too, if the point is over the text of any fly.
    const SwFrmFmt* GetFmtFromAnyObj( const Point& rPt ) const;

    /** Which Protection is set at selected object?
     returns several flags in sal_uInt8 */
    sal_uInt8 IsSelObjProtected( sal_uInt16 /*FLYPROTECT_...*/ eType ) const;

    /** Deliver graphic in rName besides graphic name. If graphic is
     linked give name with path. rbLink is TRUE if graphic is linked. */
    const Graphic *GetGrfAtPos( const Point &rDocPos,
                                OUString &rName, bool &rbLink ) const;

    OUString GetObjTitle() const;
    void SetObjTitle( const OUString& rTitle );
    OUString GetObjDescription() const;
    void SetObjDescription( const OUString& rDescription );

    bool IsFrmSelected() const;
    bool GetFlyFrmAttr( SfxItemSet &rSet ) const;
    bool SetFlyFrmAttr( SfxItemSet &rSet );
    bool ResetFlyFrmAttr( sal_uInt16 nWhich, const SfxItemSet* pSet = 0 );
    const SwFrmFmt *NewFlyFrm( const SfxItemSet &rSet, bool bAnchValid = false,
                         SwFrmFmt *pParent = 0 );
    void SetFlyPos( const Point &rAbsPos);
    Point FindAnchorPos( const Point &rAbsPos, bool bMoveIt = false );

    /** Determines whether a frame or its environment is vertically formatted and right-to-left.
     also determines, if frame or its environmane is in mongolianlayout (vertical left-to-right)
     - add output parameter <bVertL2R> */
    bool IsFrmVertical(const bool bEnvironment, bool& bRightToLeft, bool& bVertL2R) const;

    SwFrmFmt* GetCurFrmFmt() const; ///< If frame then frame style, else 0.
    void SetFrmFmt( SwFrmFmt *pFmt, bool bKeepOrient = false, Point* pDocPos = 0 ); ///< If frame then set frame style.
    const SwFlyFrm *GetCurrFlyFrm() const { return FindFlyFrm(); }

    /// Find/delete fly containing the cursor.
    SwFrmFmt* WizzardGetFly();

    /// Independent selecting of flys.
    bool GotoNextFly( sal_uInt16 /*GOTOOBJ_...*/ eType = GOTOOBJ_FLY_ANY )
                                { return GotoObj( true, eType ); }
    bool GotoPrevFly( sal_uInt16 /*GOTOOBJ_...*/ eType = GOTOOBJ_FLY_ANY)
                                { return GotoObj( false, eType); }

   /// Iterate over flys  - for Basic-collections.
    sal_uInt16 GetFlyCount( FlyCntType eType = FLYCNTTYPE_ALL, bool bIgnoreTextBoxes = false ) const;
    const SwFrmFmt* GetFlyNum(sal_uInt16 nIdx, FlyCntType eType = FLYCNTTYPE_ALL, bool bIgnoreTextBoxes = false) const;

    /// If a fly is selected, it draws cursor into the first CntntFrm.
    const SwFrmFmt* SelFlyGrabCrsr();

    /// Get FlyFrameFormat; fuer UI Macro Anbindung an Flys
    const SwFrmFmt* GetFlyFrmFmt() const;
          SwFrmFmt* GetFlyFrmFmt();

    /** OLE. Server requires new size. Desired values are adjusted as frame attributes.
     If the values are not allowed, the formatting clips and determines scaling.
     See CalcAndSetScale().
     The @return value is the applied size. */
    Size RequestObjectResize( const SwRect &rRect, const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& );

    /// The layout has been changed, so the active object has to be moved after that
    virtual void MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset );

    /** Client for OleObject has to be up-to-date regarding scaling.
     Implemented in WrtShell.
     If a pointer is passed on a size, this is the object's current core-size.
     Else the size is provided via GetCurFlyRect(). */
    virtual void CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect = 0,
                                  const SwRect *pFlyFrmRect = 0,
                                  const bool bNoTxtFrmPrtAreaChanged = false ) = 0;

    /** Connect objects with ActivateWhenVisible at Paint.
     Called by notxtfrm::Paint, implemented in wrtsh. */
    virtual void ConnectObj( svt::EmbeddedObjectRef&,
                             const SwRect &rPrt,
                             const SwRect &rFrm ) = 0;

    /// Set visible range on object, if it is not yet visible.
    void MakeObjVisible( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& ) const;

    /// Check resize of OLE-Object.
    bool IsCheckForOLEInCaption() const         { return bCheckForOLEInCaption; }
    void SetCheckForOLEInCaption( bool bFlag )  { bCheckForOLEInCaption = bFlag; }

    /// Set name at selected FlyFrame.
    void SetFlyName( const OUString& rName );
    OUString GetFlyName() const;

    /// get reference to OLE object (if there is one) for selected FlyFrame
    const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetOleRef() const;

    /// Created unique name for frame.
    OUString GetUniqueGrfName() const;
    OUString GetUniqueOLEName() const;
    OUString GetUniqueFrameName() const;

    /// Jump to named Fly (graphic/OLE).
    bool GotoFly( const OUString& rName, FlyCntType eType = FLYCNTTYPE_ALL,
                  bool bSelFrame = true );

    /// Position is a graphic with URL?
    const SwFrmFmt* IsURLGrfAtPos( const Point& rPt, OUString* pURL = 0,
                                    OUString *pTargetFrameName = 0,
                                    OUString *pURLDescription = 0 ) const;

    /** For Chain always connect Fly specified by format with that hit by point.
     rRect contains rect of Fly (for its highlight). */
    int Chainable( SwRect &rRect, const SwFrmFmt &rSource, const Point &rPt ) const;
    int Chain( SwFrmFmt &rSource, const Point &rPt );
    int Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest );
    void Unchain( SwFrmFmt &rFmt );
    void HideChainMarker();
    void SetChainMarker();

    Size GetGraphicDefaultSize() const;

    /// Temporary work around for bug.
    void CheckUnboundObjects();

    /// Attention: Ambiguities if multiple selections.
    bool GetObjAttr( SfxItemSet &rSet ) const;
    bool SetObjAttr( const SfxItemSet &rSet );

    const SdrObject* GetBestObject( bool bNext, sal_uInt16 eType = GOTOOBJ_DRAW_ANY, bool bFlat = true, const ::svx::ISdrObjectFilter* pFilter = NULL );
    bool GotoObj( bool bNext, sal_uInt16 /*GOTOOBJ_...*/ eType = GOTOOBJ_DRAW_ANY);

    /// Set DragMode (e.g. Rotate), but do nothing when frame is selected.
    void SetDragMode( sal_uInt16 eSdrDragMode );

    sal_uInt16 IsObjSelected() const;   ///< @return object count, but doesn't count the objects in groups.
    bool IsObjSelected( const SdrObject& rObj ) const;
    bool IsObjSameLevelWithMarked(const SdrObject* pObj) const;
    const SdrMarkList* GetMarkList() const{ return _GetMarkList(); };

    void EndTextEdit();             ///< Deletes object if required.

    /** Anchor type of selected object, -1 if ambiguous or in case of frame selection.
     Else FLY_AT_PAGE or FLY_AT_PARA resp. from frmatr.hxx. */
    short GetAnchorId() const;

    /** Process of creating draw objects. At the beginning object type is passed.
     At the end a Cmd can be passed. Here, SDRCREATE_RESTRAINTEND for end
     or SDRCREATE_NEXTPOINT for a polygon may be relevant.
     After RESTRAINTEND the object is created and selected.
     BreakCreate interrupts the process. In this case no object is selected. */
    bool BeginCreate( sal_uInt16 /*SdrObjKind ?*/ eSdrObjectKind, const Point &rPos );
    bool BeginCreate( sal_uInt16 /*SdrObjKind ?*/ eSdrObjectKind, sal_uInt32 eObjInventor, const Point &);
    void MoveCreate ( const Point &rPos );
    bool EndCreate  ( sal_uInt16 eSdrCreateCmd );
    void BreakCreate();
    bool IsDrawCreate() const;
    void CreateDefaultShape( sal_uInt16 /*SdrObjKind ?*/ eSdrObjectKind, const Rectangle& rRect, sal_uInt16 nSlotId);

    /// Functions for Rubberbox, ti select Draw-Objects
    bool BeginMark( const Point &rPos );
    void MoveMark ( const Point &rPos );
    bool EndMark  ();
    void BreakMark();

    /// Create and destroy group, don't when frame is selected.
    bool IsGroupSelected();     ///< Can be a mixed selection!
    void GroupSelection();          ///< Afterwards the group is selected.
    void UnGroupSelection();        /**< The individual objects are selected, but
                                    it is possible that there are groups included. */

    bool IsGroupAllowed() const;

    void MirrorSelection( bool bHorizontal );   ///< Vertical if FALSE.

    /** frmatr.hxx. Here no enum because of dependencies.
     bool value only for internal use! Anchor is newly set according
     to current document position. Anchor is not re-set. */
    void ChgAnchor( int eAnchorId, bool bSameOnly = false,
                                   bool bPosCorr = true );

    bool SetDrawingAttr( SfxItemSet &rSet );

    /** Get selected DrawObj as graphics (MetaFile/Bitmap).
     Return value indicates if it was converted. */
    bool GetDrawObjGraphic( sal_uLong nFmt, Graphic& rGrf ) const;

    void Paste( SvStream& rStm, sal_uInt16 nAction, const Point* pPt = 0 );
    bool Paste( const Graphic &rGrf, const OUString& rURL );
    bool Paste( SotDataObject& rObj, const Point& rPt );

    bool IsAlignPossible() const;
    void SetCalcFieldValueHdl(Outliner* pOutliner);

    void Insert(const OUString& rGrfName,
                const OUString& rFltName,
                const Graphic* pGraphic = 0,
                const SfxItemSet* pFlyAttrSet = 0,
                const SfxItemSet* pGrfAttrSet = 0,
                SwFrmFmt* = 0 );

    /// Insertion of a drawing object which have to be already inserted in the DrawModel.
    void InsertDrawObj( SdrObject& rDrawObj,
                        const Point& rInsertPosition );

    bool ReplaceSdrObj( const OUString& rGrfName, const OUString& rFltName,
                        const Graphic* pGrf = 0 );

    // --> #i972#
    /** for starmath formulas anchored 'as char' it alignes it baseline to baseline
     changing the previous vertical orientation */
    void AlignFormulaToBaseline( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj, SwFlyFrm * pFly = 0 );

    /// aligns all formulas with anchor 'as char' to baseline
    void AlignAllFormulasToBaseline();

    /// Provide information about content situated closes to given Point.
    Point GetCntntPos( const Point& rPoint, bool bNext ) const;

    /// Convert document position into position relative to the current page.
    Point GetRelativePagePosition(const Point& rDocPos);

    /// Hide or show layout-selection and pass call to CrsrSh.
    void ShLooseFcs();
    void ShGetFcs( bool bUpdate = true );

    /// PageDescriptor-interface
    void   ChgCurPageDesc( const SwPageDesc& );
    sal_uInt16 GetCurPageDesc( const bool bCalcFrm = true ) const;
    sal_uInt16 GetMousePageDesc( const Point &rPt ) const;
    sal_uInt16 GetPageDescCnt() const;
    SwPageDesc* FindPageDescByName( const OUString& rName,
                                    bool bGetFromPool = false,
                                    sal_uInt16* pPos = 0 );

    const SwPageDesc& GetPageDesc( sal_uInt16 i ) const;
    void  ChgPageDesc( sal_uInt16 i, const SwPageDesc& );
    /** if inside all selection only one PageDesc, @return this.
     Otherwise @return 0 pointer */
    const SwPageDesc* GetSelectedPageDescs() const;

    const SwRect& GetAnyCurRect( CurRectType eType,
                                 const Point* pPt = 0,
                                 const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& =
                                 ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >() ) const;

    /// Page number of the page containing Point, O if no page.
    sal_uInt16 GetPageNumber( const Point &rPoint ) const;
    bool GetPageNumber( long nYPos, bool bAtCrsrPos, sal_uInt16& rPhyNum, sal_uInt16& rVirtNum, OUString &rDisplay ) const;

    SwFlyFrmFmt* InsertObject( const svt::EmbeddedObjectRef&,
                const SfxItemSet* pFlyAttrSet = 0,
                const SfxItemSet* pGrfAttrSet = 0,
                SwFrmFmt* = 0 );
    bool    FinishOLEObj(); ///< Shutdown server.

    void GetTblAttr( SfxItemSet & ) const;
    void SetTblAttr( const SfxItemSet & );

    bool HasWholeTabSelection() const;

    /// Is content of a table cell or at least a table cell completely selected?
    bool HasBoxSelection() const;

    bool InsertRow( sal_uInt16 nCnt, bool bBehind );
    bool InsertCol( sal_uInt16 nCnt, bool bBehind );  // 0 == at the end.
    bool DeleteCol();
    bool DeleteTable();
    bool DeleteRow(bool bCompleteTable = false);

    bool DeleteTblSel();        ///< Current selection, may be whole table.

    sal_uInt16 MergeTab();          /**< Merge selected parts of table.
                                      @return error via enum. */

    /// Split cell vertically or horizontally.
    bool SplitTab( bool nVert = true, sal_uInt16 nCnt = 1, bool bSameHeight = false );
    bool Sort(const SwSortOptions&);    //Sortieren.

    void SetRowHeight( const SwFmtFrmSize &rSz );

    /// Pointer must be detroyed by caller != 0.
    void GetRowHeight( SwFmtFrmSize *&rpSz ) const;

    void SetRowSplit( const SwFmtRowSplit &rSz );
    void GetRowSplit( SwFmtRowSplit *&rpSz ) const;

    void   SetBoxAlign( sal_uInt16 nOrient );
    sal_uInt16 GetBoxAlign() const;         ///< USHRT_MAX if ambiguous.

    /// Adjustment of Rowheights. Determine via bTstOnly if more than one row is selected.
    bool BalanceRowHeight( bool bTstOnly );

    void SetTabBorders( const SfxItemSet& rSet );
    void GetTabBorders(       SfxItemSet& rSet) const;
    void SetTabLineStyle(const Color* pColor, bool bSetLine = false, const editeng::SvxBorderLine* pBorderLine = NULL);

    void SetTabBackground( const SvxBrushItem &rNew );
    void GetTabBackground( SvxBrushItem &rToFill ) const;

    void SetBoxBackground( const SvxBrushItem &rNew );
    bool GetBoxBackground( SvxBrushItem &rToFill ) const; ///< FALSE ambiguous.

    void SetBoxDirection( const SvxFrameDirectionItem& rNew );
    bool GetBoxDirection( SvxFrameDirectionItem& rToFill ) const; ///< FALSE ambiguous.

    void SetRowBackground( const SvxBrushItem &rNew );
    bool GetRowBackground( SvxBrushItem &rToFill ) const; ///< FALSE ambiguous.

    SwTab WhichMouseTabCol( const Point &rPt ) const;
    void GetTabCols( SwTabCols &rToFill ) const; ///< Info about columns and margins.
    void SetTabCols( const SwTabCols &rNew, bool bCurRowOnly = true );
    void GetMouseTabCols( SwTabCols &rToFill, const Point &rPt ) const;
    void SetMouseTabCols( const SwTabCols &rNew, bool bCurRowOnly,
                          const Point &rPt );

    /// pEnd will be used during MouseMove
    bool SelTblRowCol( const Point& rPt, const Point* pEnd = 0, bool bRowDrag = false );

    void GetTabRows( SwTabCols &rToFill ) const;
    void SetTabRows( const SwTabCols &rNew, bool bCurColOnly );
    void GetMouseTabRows( SwTabCols &rToFill, const Point &rPt ) const;
    void SetMouseTabRows( const SwTabCols &rNew, bool bCurColOnly, const Point &rPt );

    void ProtectCells();    /**< If a table selection exists it is destroyed in case
                             cursor is not allowed in readonly. */
    void UnProtectCells();  ///< Refers to table selection.
    void UnProtectTbls();   ///< Unprotect all tables in selection.
    bool HasTblAnyProtection( const OUString* pTblName = 0,
                              bool* pFullTblProtection = 0 );
    bool CanUnProtectCells() const;

    sal_uInt16 GetRowsToRepeat() const;
    void SetRowsToRepeat( sal_uInt16 nNumOfRows );
    sal_uInt16 GetVirtPageNum( const bool bCalcFrm = true );

    /** @return the number of table rows currently selected
    if the selection start at the top of the table. */
    sal_uInt16    GetRowSelectionFromTop() const;

    bool IsInRepeatedHeadline() const { return CheckHeadline( true ); }
    bool IsInHeadline() const { return CheckHeadline( false ); }

    /** Adjusts cell widths in such a way, that their content
     does not need to be wrapped (if possible).
     bBalance provides for adjustment of selected columns. */
    void AdjustCellWidth( bool bBalance = false );

    /// Not allowed if only empty cells are selected.
    bool IsAdjustCellWidthAllowed( bool bBalance = false ) const;

    /// Adjustment of cell-widths; determine via bTstOnly if more than one cell is selected.
    bool BalanceCellWidth( bool bTstOnly );

    /// AutoFormat for table/ table selection.
    bool SetTableAutoFmt( const SwTableAutoFmt& rNew );

    bool GetTableAutoFmt( SwTableAutoFmt& rGet );

    bool SetColRowWidthHeight( sal_uInt16 eType, sal_uInt16 nDiff = 283 );

    bool GetAutoSum( OUString& rFml ) const;

    /** Phy: real page count.
     Virt: consider offset that may have been set by user. */
    sal_uInt16  GetPhyPageNum();

    void SetNewPageOffset( sal_uInt16 nOffset );
    void SetPageOffset( sal_uInt16 nOffset );   ///< Changes last page offset.
    sal_uInt16 GetPageOffset() const;           ///< @return last page offset.

    void InsertLabel( const SwLabelType eType, const OUString &rTxt, const OUString& rSeparator,
                      const OUString& rNumberSeparator,
                      const bool bBefore, const sal_uInt16 nId,
                      const OUString& rCharacterStyle,
                      const bool bCpyBrd = true );

    /// The ruler needs some information too.
    sal_uInt16 GetCurColNum( SwGetCurColNumPara* pPara = 0 ) const; //0 == not in any column.
    sal_uInt16 GetCurMouseColNum( const Point &rPt,
                            SwGetCurColNumPara* pPara = 0 ) const;
    size_t GetCurTabColNum() const;     //0 == not in any table.
    size_t GetCurMouseTabColNum( const Point &rPt ) const;
    sal_uInt16 GetCurOutColNum( SwGetCurColNumPara* pPara = 0 ) const;  ///< Current outer column.

    bool IsTableRightToLeft() const;
    bool IsMouseTableRightToLeft( const Point &rPt ) const;
    bool IsTableVertical() const;

    bool IsLastCellInRow() const;

    /// Width of current range for column-dialog.
    long GetSectionWidth( SwFmt& rFmt ) const;

    void GetConnectableFrmFmts
    (SwFrmFmt & rFmt, const OUString & rReference, bool bSuccessors,
     ::std::vector< OUString > & aPrevPageVec,
     ::std::vector< OUString > & aThisPageVec,
     ::std::vector< OUString > & aNextPageVec,
     ::std::vector< OUString > & aRestVec);

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
    bool IsNumLabel( const Point &rPt, int nMaxOffset = -1 );

    bool IsVerticalModeAtNdAndPos( const SwTxtNode& _rTxtNode,
                                   const Point& _rDocPos ) const;

    virtual void ToggleHeaderFooterEdit( );
};

void ClearFEShellTabCols();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
