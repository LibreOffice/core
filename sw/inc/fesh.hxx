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

#include <svx/svdobj.hxx>
#include "swdllapi.h"
#include "editsh.hxx"
#include "flyenum.hxx"

#include <svx/svdtypes.hxx>
#include <sot/formats.hxx>
#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <comphelper/interfacecontainer2.hxx>

#include <vector>
#include <memory>

namespace editeng { class SvxBorderLine; }

class SwFlyFrame;
class SwTabCols;
class SvxBrushItem;
class SvxFrameDirectionItem;
class SwTableAutoFormat;
class SwFrame;
class SwFormatFrameSize;
class SwFormatRowSplit;
class Outliner;
class SwFrameFormat;
struct SwSortOptions;
class SdrMarkList;
enum class RndStdIds;

namespace svx
{
    class ISdrObjectFilter;
}
namespace com { namespace sun { namespace star { namespace embed { class XEmbeddedObject; } } } }

// return values for GetFrameType() and GetSelFrameType().
//! values can be combined via logical or
enum class FrameTypeFlags {
    NONE            =     0,
    PAGE            =     1,
    HEADER          =     2,
    FOOTER          =     4,
    BODY            =     8,
    COLUMN          =    16,
    TABLE           =    32,
    FLY_FREE        =    64,
    FLY_ATCNT       =   128,
    FLY_INCNT       =   256,
    FOOTNOTE        =   512,
    FTNPAGE         =  1024,
    FLY_ANY         =  2048,
    DRAWOBJ         =  4096,
    COLSECT         =  8192,
    COLSECTOUTTAB   = 16384
};
namespace o3tl
{
    template<> struct typed_flags<FrameTypeFlags> : is_typed_flags<FrameTypeFlags, 0x7fff> {};
}

//! values can be combined via logical or
enum class GotoObjFlags
{
    NONE           =  0,
    DrawControl    =  1,
    DrawSimple     =  2,
    DrawAny        = DrawControl | DrawSimple,
    FlyFrame         =  4,
    FlyGrf         =  8,
    FlyOLE         = 16,
    FlyAny         = FlyOLE | FlyGrf | FlyFrame,
    Any            = FlyAny | DrawAny,
};
namespace o3tl
{
    template<> struct typed_flags<GotoObjFlags> : is_typed_flags<GotoObjFlags, 31> {};
}

//! values can be combined via logical or
enum class FlyProtectFlags
{
    NONE         = 0,
    Content      = 1,
    Size         = 2,
    Pos          = 4,
    Parent       = 8,      ///< Check only parents.
    Fixed        = 16,    /**< Only protection that cannot be withdrawn
                                                    e.g. by OLE-server; also relevant for dialog. */
};
namespace o3tl
{
    template<> struct typed_flags<FlyProtectFlags> : is_typed_flags<FlyProtectFlags, 31> {};
}

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
enum class CurRectType
{
    Page,                 ///< Rect of current page.
    PageCalc,             ///< ... page will be formatted if required.
    PagePrt,              ///< Rect of current PrtArea of page.
    Frame,                ///< Rect of current frame.
    FlyEmbedded,          ///< Rect of current FlyFrame.
    FlyEmbeddedPrt,       ///< Rect of PrtArea of FlyFrame
    Section,              ///< Rect of current section.
    SectionOutsideTable,  ///< Rect of current section but outside of table.
    SectionPrt,           ///< Rect of current PrtArea of section.
    HeaderFooter,         ///< Rect of current header/footer
    PagesArea             ///< Rect covering the pages area
};

struct SwGetCurColNumPara
{
    const SwFrameFormat* pFrameFormat;
    const SwRect* pPrtRect;
    SwGetCurColNumPara() : pFrameFormat( nullptr ), pPrtRect( nullptr ) {}
};

enum class SwPasteSdr
{
    NONE        = 0,
    Insert      = 1,
    Replace     = 2,
    SetAttr     = 3
};

#define SW_ADD_SELECT   1
#define SW_ENTER_GROUP  2
#define SW_LEAVE_FRAME  4
/// Allow SwFEShell::SelectObj() to select the TextBox of a shape.
#define SW_ALLOW_TEXTBOX  8

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
struct SwColCache;

class SW_DLLPUBLIC SwFEShell : public SwEditShell
{
private:
    mutable std::unique_ptr<SwColCache> m_pColumnCache;
    mutable std::unique_ptr<SwColCache> m_pRowCache;
    std::unique_ptr<SdrDropMarkerOverlay> m_pChainTo;
    std::unique_ptr<SdrDropMarkerOverlay> m_pChainFrom;
    bool m_bCheckForOLEInCaption;
    comphelper::OInterfaceContainerHelper2 m_aPasteListeners;

    SAL_DLLPRIVATE SwFlyFrame *FindFlyFrame( const css::uno::Reference < css::embed::XEmbeddedObject >&  ) const;

    /// Terminate actions for all shells and call ChangeLink.
    SAL_DLLPRIVATE void EndAllActionAndCall();

    SAL_DLLPRIVATE void ScrollTo( const Point &rPt );

    SAL_DLLPRIVATE void ChangeOpaque( SdrLayerID nLayerId );

    /** Used for mouse operations on a table:
     @return a cell frame that is 'close' to rPt. */
    SAL_DLLPRIVATE const SwFrame *GetBox( const Point &rPt, bool* pbRow = nullptr, bool* pbCol = nullptr ) const;

    // 0 == not in any column.
    SAL_DLLPRIVATE static sal_uInt16 GetCurColNum_( const SwFrame *pFrame,
                          SwGetCurColNumPara* pPara );

    SAL_DLLPRIVATE void GetTabCols_(SwTabCols &rToFill, const SwFrame *pBox) const;
    SAL_DLLPRIVATE void GetTabRows_(SwTabCols &rToFill, const SwFrame *pBox) const;

    SAL_DLLPRIVATE bool ImpEndCreate();

    /// Methods for copying of draw objects.
    SAL_DLLPRIVATE bool CopyDrawSel( SwFEShell* pDestShell, const Point& rSttPt,
                                const Point& rInsPt, bool bIsMove,
                                bool bSelectInsert );

    /// Get list of marked SdrObjects;
    /// helper method for GetSelFrameType, IsSelContainsControl.
    SAL_DLLPRIVATE const SdrMarkList* GetMarkList_() const;

    SAL_DLLPRIVATE bool CheckHeadline( bool bRepeat ) const;

    using SwEditShell::Copy;

public:

    using SwEditShell::Insert;

    SwFEShell( SwDoc& rDoc, vcl::Window *pWin, const SwViewOption *pOpt );
    SwFEShell( SwEditShell& rShell, vcl::Window *pWin );
    virtual ~SwFEShell() override;

    /// Copy and Paste methods for internal clipboard.
    void Copy( SwDoc* pClpDoc, const OUString* pNewClpText = nullptr );
    bool Paste( SwDoc* pClpDoc );

    /// Paste some pages into another doc - used in mailmerge.
    void PastePages( SwFEShell& rToFill, sal_uInt16 nStartPage, sal_uInt16 nEndPage);

    /// Copy-Method for Drag&Drop
    bool Copy( SwFEShell*, const Point& rSttPt, const Point& rInsPt,
               bool bIsMove = false, bool bSelectInsert = true );

    void SelectFlyFrame( SwFlyFrame& rFrame );

    /// Is selected frame within another frame?
    const SwFrameFormat* IsFlyInFly();

    /** If an object as been given, exactly this object is selected
     (instead of searching over position). */
    bool SelectObj( const Point& rSelPt, sal_uInt8 nFlag = 0, SdrObject *pObj = nullptr );
    void DelSelectedObj();

    /** Move selection upwards or downwards (Z-Order).
     TRUE = to top or bottom.
     FALSE = run past one other. */
    void SelectionToTop   ( bool bTop = true );
    void SelectionToBottom( bool bBottom = true );

    SdrLayerID GetLayerId() const;   ///< 1 Heaven, 0 Hell, SDRLAYER_NOTFOUND Ambiguous.
    void  SelectionToHeaven();  ///< Above document.
    void  SelectionToHell();    ///< Below document.

    /** The following two methods return enum SdrHdlKind.
     Declared as int in order to spare including SVDRAW.HXX. */
    bool IsObjSelectable( const Point& rPt );
    /// Same as IsObjSelectable(), but return the object as well.
    SdrObject* GetObjAt(const Point& rPt);
    bool IsInsideSelectedObj( const Point& rPt );    ///< returns enum values
    /** Test if there is a draw object at that position and if it should be selected.
     The 'should' is aimed at Writer text fly frames which may be in front of
     the draw object. */
    bool ShouldObjectBeSelected(const Point& rPt);

    bool MoveAnchor( SwMove nDir );

    /** @return if Upper of frame at current position is section frame
     Currently only used by the rules. To be replaced by something more
     sophisticated one day. */
    bool IsDirectlyInSection() const;

    /** For return values see above FrameType.
     pPt: Cursr or DocPos respectively; bStopAtFly: Stop at flys or continue over anchor.
     Although (0,TRUE) is kind of a standard, the parameters are not defaulted here
     in order to force more conscious use especially of bStopAtFly. */
    FrameTypeFlags GetFrameType( const Point *pPt, bool bStopAtFly ) const;
    FrameTypeFlags GetSelFrameType() const;               //Selection (Drawing)

    /** check whether selected frame contains a control;
     * companion method to GetSelFrameType, used for preventing
     * drag&drop of controls into header */
    bool IsSelContainsControl() const;

    static ObjCntType GetObjCntType( const SdrObject& rObj );
    ObjCntType GetObjCntType( const Point &rPt, SdrObject *&rpObj ) const;
    ObjCntType GetObjCntTypeOfSelection() const;

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
                        const sal_Int16 _eHoriRelOrient = css::text::RelOrientation::FRAME,
                        const sal_Int16 _eVertRelOrient = css::text::RelOrientation::FRAME,
                        const SwPosition* _pToCharContentPos = nullptr,
                        const bool _bFollowTextFlow = false,
                        bool _bMirror = false,
                        Point* _opRef = nullptr,
                        Size* _opPercent = nullptr,
                        const SwFormatFrameSize* pFormatFrameSize = nullptr ) const;

    /// Set size of draw objects.
    void SetObjRect( const SwRect& rRect );

    void BeginDrag( const Point *pPt, bool bProp );
    void Drag     ( const Point *pPt, bool bProp );
    void EndDrag  ();
    void BreakDrag();

    /// Methods for status line.
    Point GetAnchorObjDiff() const;
    Point GetObjAbsPos()     const;
    Size  GetObjSize()       const;

    /// SS for envelopes: get all page-bound objects and set them to new page.
    void GetPageObjs( std::vector<SwFrameFormat*>& rFillArr );
    void SetPageObjsNewPage( std::vector<SwFrameFormat*>& rFillArr );

    /// Show current selection (frame / draw object as required).
    virtual void MakeSelVisible() override;

    /** @return FrameFormat of object that may be under Point.
     Object does not become selected! */
    const SwFrameFormat* GetFormatFromObj( const Point& rPt, SwRect** pRectToFill = nullptr ) const;

    /// @return a format too, if the point is over the text of any fly.
    const SwFrameFormat* GetFormatFromAnyObj( const Point& rPt ) const;

    /** Which Protection is set at selected object? */
    FlyProtectFlags IsSelObjProtected( FlyProtectFlags eType ) const;

    /** Deliver graphic in rName besides graphic name. If graphic is
     linked give name with path. rbLink is TRUE if graphic is linked. */
    const Graphic *GetGrfAtPos( const Point &rDocPos,
                                OUString &rName, bool &rbLink ) const;

    OUString GetObjTitle() const;
    void SetObjTitle( const OUString& rTitle );
    OUString GetObjDescription() const;
    void SetObjDescription( const OUString& rDescription );

    bool IsFrameSelected() const;
    bool GetFlyFrameAttr( SfxItemSet &rSet ) const;
    bool SetFlyFrameAttr( SfxItemSet &rSet );
    static SfxItemSet makeItemSetFromFormatAnchor(SfxItemPool& rPool, const SwFormatAnchor &rAnchor);
    void ResetFlyFrameAttr( const SfxItemSet* pSet );
    const SwFrameFormat *NewFlyFrame( const SfxItemSet &rSet, bool bAnchValid = false,
                         SwFrameFormat *pParent = nullptr );
    void SetFlyPos( const Point &rAbsPos);
    Point FindAnchorPos( const Point &rAbsPos, bool bMoveIt = false );

    /** Determines whether a frame or its environment is vertically formatted and right-to-left.
     also determines, if frame or its environment is in Mongolian layout (vertical left-to-right)
     - add output parameter <bVertL2R> */
    bool IsFrameVertical(const bool bEnvironment, bool& bRightToLeft, bool& bVertL2R) const;

    SwFrameFormat* GetSelectedFrameFormat() const; ///< If frame then frame style, else 0.
    void SetFrameFormat( SwFrameFormat *pFormat, bool bKeepOrient = false, Point const * pDocPos = nullptr ); ///< If frame then set frame style.

    // Get selected fly
    SwFlyFrame* GetSelectedFlyFrame() const;

    // Get current fly in which the cursor is positioned
    SwFlyFrame* GetCurrFlyFrame(const bool bCalcFrame = true) const;

    // Get selected fly, but if none Get current fly in which the cursor is positioned
    SwFlyFrame* GetSelectedOrCurrFlyFrame() const;

    /// Find/delete fly containing the cursor.
    SwFrameFormat* WizardGetFly();

    /// Independent selecting of flys.
    bool GotoNextFly( GotoObjFlags eType = GotoObjFlags::FlyAny )
                                { return GotoObj( true, eType ); }
    bool GotoPrevFly( GotoObjFlags eType = GotoObjFlags::FlyAny)
                                { return GotoObj( false, eType); }

    /// Iterate over flys  - for Basic-collections.
    size_t GetFlyCount( FlyCntType eType, bool bIgnoreTextBoxes = false ) const;
    const SwFrameFormat* GetFlyNum(size_t nIdx, FlyCntType eType, bool bIgnoreTextBoxes = false) const;

    std::vector<SwFrameFormat const*> GetFlyFrameFormats(
            FlyCntType eType, bool bIgnoreTextBoxes);

    /// If a fly is selected, it draws cursor into the first ContentFrame.
    const SwFrameFormat* SelFlyGrabCursor();

    /// Get FlyFrameFormat; for UI macro linkage at Flys
    const SwFrameFormat* GetFlyFrameFormat() const;
          SwFrameFormat* GetFlyFrameFormat();

    /** OLE. Server requires new size. Desired values are adjusted as frame attributes.
     If the values are not allowed, the formatting clips and determines scaling.
     See CalcAndSetScale().
     The @return value is the applied size. */
    Size RequestObjectResize( const SwRect &rRect, const css::uno::Reference < css::embed::XEmbeddedObject >& );

    /// The layout has been changed, so the active object has to be moved after that
    virtual void MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset );

    /** Client for OleObject has to be up-to-date regarding scaling.
     Implemented in WrtShell.
     If a pointer is passed on a size, this is the object's current core-size.
     Else the size is provided via GetCurFlyRect(). */
    virtual void CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect = nullptr,
                                  const SwRect *pFlyFrameRect = nullptr,
                                  const bool bNoTextFramePrtAreaChanged = false ) = 0;

    /** Connect objects with ActivateWhenVisible at Paint.
     Called by notxtfrm::Paint, implemented in wrtsh. */
    virtual void ConnectObj( svt::EmbeddedObjectRef&,
                             const SwRect &rPrt,
                             const SwRect &rFrame ) = 0;

    /// Check resize of OLE-Object.
    bool IsCheckForOLEInCaption() const         { return m_bCheckForOLEInCaption; }
    void SetCheckForOLEInCaption( bool bFlag )  { m_bCheckForOLEInCaption = bFlag; }

    /// Set name at selected FlyFrame.
    void SetFlyName( const OUString& rName );
    OUString GetFlyName() const;

    /// get reference to OLE object (if there is one) for selected FlyFrame
    const css::uno::Reference < css::embed::XEmbeddedObject > GetOleRef() const;

    /// Created unique name for frame.
    OUString GetUniqueGrfName() const;
    OUString GetUniqueOLEName() const;
    OUString GetUniqueFrameName() const;
    OUString GetUniqueShapeName() const;

    /// Jump to named Fly (graphic/OLE).
    bool GotoFly( const OUString& rName, FlyCntType eType,
                  bool bSelFrame );

    /// Position is a graphic with URL?
    const SwFrameFormat* IsURLGrfAtPos( const Point& rPt, OUString* pURL = nullptr,
                                    OUString *pTargetFrameName = nullptr,
                                    OUString *pURLDescription = nullptr ) const;

    /** For Chain always connect Fly specified by format with that hit by point.
     rRect contains rect of Fly (for its highlight). */
    SwChainRet Chainable( SwRect &rRect, const SwFrameFormat &rSource, const Point &rPt ) const;
    SwChainRet Chain( SwFrameFormat &rSource, const Point &rPt );
    void       Chain( SwFrameFormat &rSource, const SwFrameFormat &rDest );
    void Unchain( SwFrameFormat &rFormat );
    void HideChainMarker();
    void SetChainMarker();

    Size GetGraphicDefaultSize() const;

    /// Temporary work around for bug.
    void CheckUnboundObjects();

    /// Attention: Ambiguities if multiple selections.
    bool GetObjAttr( SfxItemSet &rSet ) const;
    void SetObjAttr( const SfxItemSet &rSet );

    const SdrObject* GetBestObject( bool bNext, GotoObjFlags eType, bool bFlat = true, const svx::ISdrObjectFilter* pFilter = nullptr );
    bool GotoObj( bool bNext, GotoObjFlags eType = GotoObjFlags::DrawAny);

    /// Set DragMode (e.g. Rotate), but do nothing when frame is selected.
    void SetDragMode( SdrDragMode eSdrDragMode );

    // Get the current drag mode
    SdrDragMode GetDragMode() const;

    // Start cropping the selected image
    void StartCropImage();

    // RotGrfFlyFrame: check if RotationMode is possible
    bool IsRotationOfSwGrfNodePossible() const;

    size_t IsObjSelected() const;   ///< @return object count, but doesn't count the objects in groups.
    bool IsObjSelected( const SdrObject& rObj ) const;
    bool IsObjSameLevelWithMarked(const SdrObject* pObj) const;
    const SdrMarkList* GetMarkList() const{ return GetMarkList_(); };

    void EndTextEdit();             ///< Deletes object if required.

    /** Anchor type of selected object, RndStdIds::UNKNOWN if ambiguous or in case of frame selection.
     Else RndStdIds::FLY_AT_PAGE or RndStdIds::FLY_AT_PARA. */
    RndStdIds GetAnchorId() const;

    /** Process of creating draw objects. At the beginning object type is passed.
     At the end a Cmd can be passed. Here, SDRCREATE_RESTRAINTEND for end
     or SdrCreateCmd::NextPoint for a polygon may be relevant.
     After RESTRAINTEND the object is created and selected.
     BreakCreate interrupts the process. In this case no object is selected. */
    bool BeginCreate( sal_uInt16 /*SdrObjKind ?*/ eSdrObjectKind, const Point &rPos );
    bool BeginCreate( sal_uInt16 /*SdrObjKind ?*/ eSdrObjectKind, SdrInventor eObjInventor, const Point &);
    void MoveCreate ( const Point &rPos );
    bool EndCreate  ( SdrCreateCmd eSdrCreateCmd );
    void BreakCreate();
    bool IsDrawCreate() const;
    void CreateDefaultShape( sal_uInt16 /*SdrObjKind ?*/ eSdrObjectKind, const tools::Rectangle& rRect, sal_uInt16 nSlotId);

    /// Functions for Rubberbox, ti select Draw-Objects
    bool BeginMark( const Point &rPos );
    void MoveMark ( const Point &rPos );
    bool EndMark  ();

    /// Create and destroy group, don't when frame is selected.
    bool IsGroupSelected();     ///< Can be a mixed selection!
    void GroupSelection();          ///< Afterwards the group is selected.
    void UnGroupSelection();        /**< The individual objects are selected, but
                                    it is possible that there are groups included. */

    bool IsGroupAllowed() const;
    bool IsUnGroupAllowed() const;

    void MirrorSelection( bool bHorizontal );   ///< Vertical if FALSE.

    /** frmatr.hxx. Here no enum because of dependencies.
     bool value only for internal use! Anchor is newly set according
     to current document position. Anchor is not re-set. */
    void ChgAnchor( RndStdIds eAnchorId, bool bSameOnly = false,
                                   bool bPosCorr = true );

    bool SetDrawingAttr( SfxItemSet &rSet );

    /** Get selected DrawObj as graphics (MetaFile/Bitmap).
     Return value indicates if it was converted. */
    bool GetDrawObjGraphic( SotClipboardFormatId nFormat, Graphic& rGrf ) const;

    void Paste( SvStream& rStm, SwPasteSdr nAction, const Point* pPt );
    bool Paste( const Graphic &rGrf, const OUString& rURL );

    comphelper::OInterfaceContainerHelper2& GetPasteListeners();

    bool IsAlignPossible() const;
    void SetCalcFieldValueHdl(Outliner* pOutliner);

    void Insert(const OUString& rGrfName,
                const OUString& rFltName,
                const Graphic* pGraphic,
                const SfxItemSet* pFlyAttrSet );

    /// Insertion of a drawing object which have to be already inserted in the DrawModel.
    void InsertDrawObj( SdrObject& rDrawObj,
                        const Point& rInsertPosition );

    void ReplaceSdrObj( const OUString& rGrfName, const Graphic* pGrf );

    // --> #i972#
    /** for starmath formulas anchored 'as char' it aligns it baseline to baseline
     changing the previous vertical orientation */
    void AlignFormulaToBaseline( const css::uno::Reference < css::embed::XEmbeddedObject >& xObj );

    /// aligns all formulas with anchor 'as char' to baseline
    void AlignAllFormulasToBaseline();

    /// Provide information about content situated closes to given Point.
    Point GetContentPos( const Point& rPoint, bool bNext ) const;

    /// Convert document position into position relative to the current page.
    Point GetRelativePagePosition(const Point& rDocPos);

    /// Hide or show layout-selection and pass call to CursorSh.
    void ShellLoseFocus();
    void ShellGetFocus();

    /// PageDescriptor-interface
    void   ChgCurPageDesc( const SwPageDesc& );
    size_t GetCurPageDesc( const bool bCalcFrame = true ) const;
    size_t GetMousePageDesc( const Point &rPt ) const;
    size_t GetPageDescCnt() const;
    SwPageDesc* FindPageDescByName( const OUString& rName,
                                    bool bGetFromPool = false,
                                    size_t* pPos = nullptr );

    const SwPageDesc& GetPageDesc( size_t i ) const;
    void  ChgPageDesc( size_t i, const SwPageDesc& );
    /** if inside all selection only one PageDesc, @return this.
     Otherwise @return 0 pointer */
    const SwPageDesc* GetSelectedPageDescs() const;

    const SwRect& GetAnyCurRect( CurRectType eType,
                                 const Point* pPt = nullptr,
                                 const css::uno::Reference < css::embed::XEmbeddedObject >& =
                                 css::uno::Reference < css::embed::XEmbeddedObject >() ) const;

    /// Page number of the page containing Point, O if no page.
    sal_uInt16 GetPageNumber( const Point &rPoint ) const;
    bool GetPageNumber( long nYPos, bool bAtCursorPos, sal_uInt16& rPhyNum, sal_uInt16& rVirtNum, OUString &rDisplay ) const;

    SwFlyFrameFormat* InsertObject( const svt::EmbeddedObjectRef&,
                const SfxItemSet* pFlyAttrSet );
    bool    FinishOLEObj(); ///< Shutdown server.

    void GetTableAttr( SfxItemSet & ) const;
    void SetTableAttr( const SfxItemSet & );

    bool HasWholeTabSelection() const;

    /// Is content of a table cell or at least a table cell completely selected?
    bool HasBoxSelection() const;

    void InsertRow( sal_uInt16 nCnt, bool bBehind );
    void InsertCol( sal_uInt16 nCnt, bool bBehind );  // 0 == at the end.
    bool DeleteCol();
    void DeleteTable();
    bool DeleteRow(bool bCompleteTable = false);

    bool DeleteTableSel();        ///< Current selection, may be whole table.

    TableMergeErr MergeTab();          /**< Merge selected parts of table */

    /// Split cell vertically or horizontally.
    void SplitTab( bool bVert, sal_uInt16 nCnt, bool bSameHeight );
    bool Sort(const SwSortOptions&);    // sorting

    void SetRowHeight( const SwFormatFrameSize &rSz );

    /// Pointer must be destroyed by caller != 0.
    std::unique_ptr<SwFormatFrameSize> GetRowHeight() const;

    void SetRowSplit( const SwFormatRowSplit &rSz );
    std::unique_ptr<SwFormatRowSplit> GetRowSplit() const;

    void   SetBoxAlign( sal_uInt16 nOrient );
    sal_uInt16 GetBoxAlign() const;         ///< USHRT_MAX if ambiguous.

    bool BalanceRowHeight( bool bTstOnly, const bool bOptimize = false );

    void SetTabBorders( const SfxItemSet& rSet );
    void GetTabBorders(       SfxItemSet& rSet) const;
    void SetTabLineStyle(const Color* pColor, bool bSetLine = false, const editeng::SvxBorderLine* pBorderLine = nullptr);

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
    void SetTabCols( const SwTabCols &rNew, bool bCurRowOnly );
    void GetMouseTabCols( SwTabCols &rToFill, const Point &rPt ) const;
    void SetMouseTabCols( const SwTabCols &rNew, bool bCurRowOnly,
                          const Point &rPt );

    /// pEnd will be used during MouseMove
    bool SelTableRowCol( const Point& rPt, const Point* pEnd, bool bRowDrag );

    void GetTabRows( SwTabCols &rToFill ) const;
    void SetTabRows( const SwTabCols &rNew, bool bCurColOnly );
    void GetMouseTabRows( SwTabCols &rToFill, const Point &rPt ) const;
    void SetMouseTabRows( const SwTabCols &rNew, bool bCurColOnly, const Point &rPt );

    void ProtectCells();    /**< If a table selection exists it is destroyed in case
                             cursor is not allowed in readonly. */
    void UnProtectCells();  ///< Refers to table selection.
    void UnProtectTables();   ///< Unprotect all tables in selection.
    bool HasTableAnyProtection( const OUString* pTableName,
                              bool* pFullTableProtection );
    bool CanUnProtectCells() const;

    sal_uInt16 GetRowsToRepeat() const;
    void SetRowsToRepeat( sal_uInt16 nNumOfRows );
    sal_uInt16 GetVirtPageNum();

    /** @return the number of table rows currently selected
    if the selection start at the top of the table. */
    sal_uInt16    GetRowSelectionFromTop() const;

    bool IsInRepeatedHeadline() const { return CheckHeadline( true ); }
    bool IsInHeadline() const { return CheckHeadline( false ); }

    void AdjustCellWidth( bool bBalance, const bool bNoShrink, const bool bColumnWidth );

    /// Not allowed if only empty cells are selected.
    bool IsAdjustCellWidthAllowed( bool bBalance = false ) const;

    /// Set table style of the current table.
    bool SetTableStyle(const OUString& rStyleName);
    bool SetTableStyle(const SwTableAutoFormat& rNew);

    /// Update the direct formatting according to the current table style.
    /// @param pTableNode Table node to update.  When nullptr, current cursor position is used.
    /// @param bResetDirect Reset direct formatting that might be applied to the cells.
    /// @param pStyleName new style to apply
    bool UpdateTableStyleFormatting(SwTableNode *pTableNode = nullptr, bool bResetDirect = false, OUString const* pStyleName = nullptr);

    bool GetTableAutoFormat( SwTableAutoFormat& rGet );

    void SetColRowWidthHeight( TableChgWidthHeightType eType, sal_uInt16 nDiff );

    void GetAutoSum( OUString& rFormula ) const;

    /** Phy: real page count.
     Virt: consider offset that may have been set by user. */
    sal_uInt16  GetPhyPageNum();

    void SetNewPageOffset( sal_uInt16 nOffset );
    void SetPageOffset( sal_uInt16 nOffset );   ///< Changes last page offset.
    sal_uInt16 GetPageOffset() const;           ///< @return last page offset.

    void InsertLabel( const SwLabelType eType, const OUString &rText, const OUString& rSeparator,
                      const OUString& rNumberSeparator,
                      const bool bBefore, const sal_uInt16 nId,
                      const OUString& rCharacterStyle,
                      const bool bCpyBrd );

    /// The ruler needs some information too.
    sal_uInt16 GetCurColNum( SwGetCurColNumPara* pPara = nullptr ) const; //0 == not in any column.
    sal_uInt16 GetCurMouseColNum( const Point &rPt ) const;
    size_t GetCurTabColNum() const;     //0 == not in any table.
    size_t GetCurMouseTabColNum( const Point &rPt ) const;
    sal_uInt16 GetCurOutColNum() const;  ///< Current outer column.

    bool IsColRightToLeft() const;
    bool IsTableRightToLeft() const;
    bool IsMouseTableRightToLeft( const Point &rPt ) const;
    bool IsTableVertical() const;

    bool IsLastCellInRow() const;

    /// Width of current range for column-dialog.
    long GetSectionWidth( SwFormat const & rFormat ) const;

    void GetConnectableFrameFormats
    (SwFrameFormat & rFormat, const OUString & rReference, bool bSuccessors,
     std::vector< OUString > & aPrevPageVec,
     std::vector< OUString > & aThisPageVec,
     std::vector< OUString > & aNextPageVec,
     std::vector< OUString > & aRestVec);

    /** SwFEShell::GetShapeBackgrd

        method determines background color of the page the selected drawing
        object is on and returns this color.
        If no color is found, because no drawing object is selected or ...,
        color COL_BLACK (default color on constructing object of class Color)
        is returned.

        @returns an object of class Color
    */
    const Color GetShapeBackgrd() const;

    /** Is default horizontal text direction for selected drawing object right-to-left

        Because drawing objects only painted for each page only, the default
        horizontal text direction of a drawing object is given by the corresponding
        page property.

        @returns boolean, indicating, if the horizontal text direction of the
        page, the selected drawing object is on, is right-to-left.
    */
    bool IsShapeDefaultHoriTextDirR2L() const;

    void ParkCursorInTab();

    SwTextNode * GetNumRuleNodeAtPos(const Point &rPot);
    bool IsNumLabel( const Point &rPt, int nMaxOffset = -1 );

    static bool IsVerticalModeAtNdAndPos( const SwTextNode& _rTextNode,
                                   const Point& _rDocPos );

    void ToggleHeaderFooterEdit( );
    static void SetLineEnds(SfxItemSet& rAttr, SdrObject const & rObj, sal_uInt16 nSlotId);

    SAL_DLLPRIVATE void ClearColumnRowCache(SwTabFrame const*);
};

void ClearFEShellTabCols(SwDoc & rDoc, SwTabFrame const*const pFrame);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
