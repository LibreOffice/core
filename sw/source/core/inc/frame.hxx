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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FRAME_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FRAME_HXX

#include <vector>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <editeng/borderline.hxx>
#include "swtypes.hxx"
#include "swrect.hxx"
#include "calbck.hxx"
#include <svl/SfxBroadcaster.hxx>
#include "IDocumentDrawModelAccess.hxx"

#include <com/sun/star/style/TabStop.hpp>

class SwLayoutFrm;
class SwRootFrm;
class SwPageFrm;
class SwFlyFrm;
class SwSectionFrm;
class SwFootnoteFrm;
class SwFootnoteBossFrm;
class SwTabFrm;
class SwRowFrm;
class SwFlowFrm;
class SwContentFrm;
class SfxPoolItem;
class SwAttrSet;
class SwViewShell;
class Color;
class SwBorderAttrs;
class SwCache;
class SvxBrushItem;
class XFillStyleItem;
class XFillGradientItem;
class SwSelectionList;
struct SwPosition;
struct SwCrsrMoveState;
class SwFormat;
class SwPrintData;
class SwSortedObjs;
class SwAnchoredObject;
typedef struct _xmlTextWriter *xmlTextWriterPtr;

// Each FrmType is represented here as a bit.
// The bits must be set in a way that it can be determined with masking of
// which kind of FrmType an instance is _and_ from what classes it was derived.
// Each frame has in its base class a member that must be set by the
// constructors accordingly.
#define FRM_ROOT        0x0001
#define FRM_PAGE        0x0002
#define FRM_COLUMN      0x0004
#define FRM_HEADER      0x0008
#define FRM_FOOTER      0x0010
#define FRM_FTNCONT     0x0020
#define FRM_FTN         0x0040
#define FRM_BODY        0x0080
#define FRM_FLY         0x0100
#define FRM_SECTION     0x0200
#define FRM_UNUSED      0x0400
#define FRM_TAB         0x0800
#define FRM_ROW         0x1000
#define FRM_CELL        0x2000
#define FRM_TXT         0x4000
#define FRM_NOTXT       0x8000

// for internal use some common combinations
#define FRM_LAYOUT      0x3FFF
#define FRM_CNTNT       0xC000
#define FRM_FTNBOSS     0x0006
#define FRM_ACCESSIBLE (FRM_HEADER|FRM_FOOTER|FRM_FTN|FRM_TXT|FRM_ROOT|FRM_FLY|FRM_TAB|FRM_CELL|FRM_PAGE)

#define FRM_NEIGHBOUR   0x2004
#define FRM_NOTE_VERT   0x7a60
#define FRM_HEADFOOT    0x0018
#define FRM_BODYFTNC    0x00a0

class SwFrm;
typedef long (SwFrm:: *SwFrmGet)() const;
typedef bool (SwFrm:: *SwFrmMax)( long );
typedef void (SwFrm:: *SwFrmMakePos)( const SwFrm*, const SwFrm*, bool );
typedef long (*SwOperator)( long, long );
typedef void (SwFrm:: *SwFrmSet)( long, long );

struct SwRectFnCollection
{
    SwRectGet     fnGetTop;
    SwRectGet     fnGetBottom;
    SwRectGet     fnGetLeft;
    SwRectGet     fnGetRight;
    SwRectGet     fnGetWidth;
    SwRectGet     fnGetHeight;
    SwRectPoint   fnGetPos;
    SwRectSize    fnGetSize;

    SwRectSet     fnSetTop;
    SwRectSet     fnSetBottom;
    SwRectSet     fnSetLeft;
    SwRectSet     fnSetRight;
    SwRectSet     fnSetWidth;
    SwRectSet     fnSetHeight;

    SwRectSet     fnSubTop;
    SwRectSet     fnAddBottom;
    SwRectSet     fnSubLeft;
    SwRectSet     fnAddRight;
    SwRectSet     fnAddWidth;
    SwRectSet     fnAddHeight;

    SwRectSet     fnSetPosX;
    SwRectSet     fnSetPosY;

    SwFrmGet      fnGetTopMargin;
    SwFrmGet      fnGetBottomMargin;
    SwFrmGet      fnGetLeftMargin;
    SwFrmGet      fnGetRightMargin;
    SwFrmSet      fnSetXMargins;
    SwFrmSet      fnSetYMargins;
    SwFrmGet      fnGetPrtTop;
    SwFrmGet      fnGetPrtBottom;
    SwFrmGet      fnGetPrtLeft;
    SwFrmGet      fnGetPrtRight;
    SwRectDist    fnTopDist;
    SwRectDist    fnBottomDist;
    SwRectDist    fnLeftDist;
    SwRectDist    fnRightDist;
    SwFrmMax      fnSetLimit;
    SwRectMax     fnOverStep;

    SwRectSetPos  fnSetPos;
    SwFrmMakePos  fnMakePos;
    SwOperator    fnXDiff;
    SwOperator    fnYDiff;
    SwOperator    fnXInc;
    SwOperator    fnYInc;

    SwRectSetTwice fnSetLeftAndWidth;
    SwRectSetTwice fnSetTopAndHeight;
};

typedef SwRectFnCollection* SwRectFn;

extern SwRectFn fnRectHori, fnRectVert, fnRectB2T, fnRectVL2R, fnRectVertL2R;
#define SWRECTFN( pFrm )    bool bVert = pFrm->IsVertical(); \
                            bool bRev = pFrm->IsReverse(); \
                            bool bVertL2R = pFrm->IsVertLR(); \
                            SwRectFn fnRect = bVert ? \
                                ( bRev ? fnRectVL2R : ( bVertL2R ? fnRectVertL2R : fnRectVert ) ): \
                                ( bRev ? fnRectB2T : fnRectHori );
#define SWRECTFNX( pFrm )   bool bVertX = pFrm->IsVertical(); \
                            bool bRevX = pFrm->IsReverse(); \
                            bool bVertL2RX = pFrm->IsVertLR(); \
                            SwRectFn fnRectX = bVertX ? \
                                ( bRevX ? fnRectVL2R : ( bVertL2RX ? fnRectVertL2R : fnRectVert ) ): \
                                ( bRevX ? fnRectB2T : fnRectHori );
#define SWREFRESHFN( pFrm ) { if( bVert != pFrm->IsVertical() || \
                                  bRev  != pFrm->IsReverse() ) \
                                bVert = pFrm->IsVertical(); \
                                bRev = pFrm->IsReverse(); \
                                bVertL2R = pFrm->IsVertLR(); \
                                fnRect = bVert ? \
                                    ( bRev ? fnRectVL2R : ( bVertL2R ? fnRectVertL2R : fnRectVert ) ): \
                                    ( bRev ? fnRectB2T : fnRectHori ); }
#define SWRECTFN2( pFrm )   bool bVert = pFrm->IsVertical(); \
                bool bVertL2R = pFrm->IsVertLR(); \
                            bool bNeighb = pFrm->IsNeighbourFrm(); \
                            SwRectFn fnRect = bVert == bNeighb ? \
                                fnRectHori : ( bVertL2R ? fnRectVertL2R : fnRectVert );

#define POS_DIFF( aFrm1, aFrm2 ) \
            ( (aFrm1.*fnRect->fnGetTop)() != (aFrm2.*fnRect->fnGetTop)() || \
            (aFrm1.*fnRect->fnGetLeft)() != (aFrm2.*fnRect->fnGetLeft)() )

// for GetNextLeaf/GetPrevLeaf.
enum MakePageType
{
    MAKEPAGE_NONE,      // do not create page/footnote
    MAKEPAGE_APPEND,    // only append page if needed
    MAKEPAGE_INSERT,    // add or append page if needed
    MAKEPAGE_FTN,       // add footnote if needed
    MAKEPAGE_NOSECTION  // Don't create section frames
};

//UUUU
namespace drawinglayer { namespace attribute {
    class SdrAllFillAttributesHelper;
    typedef std::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
}}

/**
 * Base class of the Writer layout elements.
 *
 * This includes not only fly frames, but everything down to the paragraph
 * level: pages, headers, footers, etc. (Inside a paragraph SwLinePortion
 * instances are used.)
 */
class SW_DLLPUBLIC SwFrm: public SwClient, public SfxBroadcaster
{
    // the hidden Frm
    friend class SwFlowFrm;
    friend class SwLayoutFrm;
    friend class SwLooping;

    // voids lower during creation of a column
    friend SwFrm *SaveContent( SwLayoutFrm *, SwFrm* pStart );
    friend void   RestoreContent( SwFrm *, SwLayoutFrm *, SwFrm *pSibling, bool bGrow );

    // for validating a mistakenly invalidated one in SwContentFrm::MakeAll
    friend void ValidateSz( SwFrm *pFrm );
    // implemented in text/txtftn.cxx, prevents Footnote oscillation
    friend void ValidateText( SwFrm *pFrm );

    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );

    // cache for (border) attributes
    static SwCache *mpCache;

    bool mbIfAccTableShouldDisposing;
    bool mbInDtor;

    // #i65250#
    // frame ID is now in general available - used for layout loop control
    static sal_uInt32 mnLastFrmId;
    const  sal_uInt32 mnFrmId;

    SwRootFrm   *mpRoot;
    SwLayoutFrm *mpUpper;
    SwFrm       *mpNext;
    SwFrm       *mpPrev;

    SwFrm *_FindNext();
    SwFrm *_FindPrev();

    /** method to determine next content frame in the same environment
        for a flow frame (content frame, table frame, section frame)

        #i27138# - adding documentation:
        Travelling downwards through the layout to determine the next content
        frame in the same environment. There are several environments in a
        document, which form a closed context regarding this function. These
        environments are:
        - Each page header
        - Each page footer
        - Each unlinked fly frame
        - Each group of linked fly frames
        - All footnotes
        - All document body frames
        #i27138# - adding parameter <_bInSameFootnote>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        @param _bInSameFootnote
        input parameter - boolean indicating, that the found next content
        frame has to be in the same footnote frame. This parameter is only
        relevant for flow frames in footnotes.

        @return SwContentFrm*
        pointer to the found next content frame. It's NULL, if none exists.
    */
    SwContentFrm* _FindNextCnt( const bool _bInSameFootnote = false );

    /** method to determine previous content frame in the same environment
        for a flow frame (content frame, table frame, section frame)

        #i27138#
        Travelling upwards through the layout to determine the previous content
        frame in the same environment. There are several environments in a
        document, which form a closed context regarding this function. These
        environments are:
        - Each page header
        - Each page footer
        - Each unlinked fly frame
        - Each group of linked fly frames
        - All footnotes
        - All document body frames
        #i27138# - adding parameter <_bInSameFootnote>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        @param _bInSameFootnote
        input parameter - boolean indicating, that the found previous content
        frame has to be in the same footnote frame. This parameter is only
        relevant for flow frames in footnotes.

        @return SwContentFrm*
        pointer to the found previous content frame. It's NULL, if none exists.
    */
    SwContentFrm* _FindPrevCnt( const bool _bInSameFootnote = false );

    void _UpdateAttrFrm( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 & );
    SwFrm* _GetIndNext();
    void SetDirFlags( bool bVert );

    const SwLayoutFrm* ImplGetNextLayoutLeaf( bool bFwd ) const;

protected:
    SwSortedObjs* mpDrawObjs;    // draw objects, can be 0

    SwRect  maFrm;   // absolute position in document and size of the Frm
    SwRect  maPrt;   // position relatively to Frm and size of PrtArea

    sal_uInt16 mnFrmType;  //Who am I?

    bool mbReverse     : 1; // Next line above/at the right side instead
                                 // under/at the left side of the previous line
    bool mbInvalidR2L  : 1;
    bool mbDerivedR2L  : 1;
    bool mbRightToLeft : 1;
    bool mbInvalidVert : 1;
    bool mbDerivedVert : 1;
    bool mbVertical    : 1;

    bool mbVertLR      : 1;

    bool mbValidPos      : 1;
    bool mbValidPrtArea  : 1;
    bool mbValidSize     : 1;
    bool mbValidLineNum  : 1;
    bool mbFixSize       : 1;
    // if true, frame will be painted completely even content was changed
    // only partially. For ContentFrms a border (from Action) will exclusively
    // painted if <mbCompletePaint> is true.
    bool mbCompletePaint : 1;
    bool mbRetouche      : 1; // frame is responsible for retouching

    bool mbInfInvalid    : 1;  // InfoFlags are invalid
    bool mbInfBody       : 1;  // Frm is in document body
    bool mbInfTab        : 1;  // Frm is in a table
    bool mbInfFly        : 1;  // Frm is in a Fly
    bool mbInfFootnote        : 1;  // Frm is in a footnote
    bool mbInfSct        : 1;  // Frm is in a section
    bool mbColLocked     : 1;  // lock Grow/Shrink for column-wise section
                                  // or fly frames, will be set in Format
    bool m_isInDestroy : 1;
    bool mbForbidDelete : 1;

    void ColLock()      { mbColLocked = true; }
    void ColUnlock()    { mbColLocked = false; }

    virtual void DestroyImpl();
    virtual ~SwFrm();

    // Only used by SwRootFrm Ctor to get 'this' into mpRoot...
    void setRootFrm( SwRootFrm* pRoot ) { mpRoot = pRoot; }

    SwPageFrm *InsertPage( SwPageFrm *pSibling, bool bFootnote );
    void PrepareMake(vcl::RenderContext* pRenderContext);
    void OptPrepareMake();
    void MakePos();
    // Format next frame of table frame to assure keeping attributes.
    // In case of nested tables method <SwFrm::MakeAll()> is called to
    // avoid formatting of superior table frame.
    friend SwFrm* sw_FormatNextContentForKeep( SwTabFrm* pTabFrm );

    virtual void MakeAll(vcl::RenderContext* pRenderContext) = 0;
    // adjust frames of a page
    SwTwips AdjustNeighbourhood( SwTwips nDiff, bool bTst = false );

    // change only frame size not the size of PrtArea
    virtual SwTwips ShrinkFrm( SwTwips, bool bTst = false, bool bInfo = false ) = 0;
    virtual SwTwips GrowFrm  ( SwTwips, bool bTst = false, bool bInfo = false ) = 0;

    SwModify        *GetDep()       { return GetRegisteredInNonConst(); }
    const SwModify  *GetDep() const { return GetRegisteredIn(); }

    SwFrm( SwModify*, SwFrm* );

    void CheckDir( sal_uInt16 nDir, bool bVert, bool bOnlyBiDi, bool bBrowse );

    /** enumeration for the different invalidations
        #i28701#
    */
    enum InvalidationType
    {
        INVALID_SIZE, INVALID_PRTAREA, INVALID_POS, INVALID_LINENUM, INVALID_ALL
    };

    /** method to determine, if an invalidation is allowed.
        #i28701
    */
    virtual bool _InvalidationAllowed( const InvalidationType _nInvalid ) const;

    /** method to perform additional actions on an invalidation

        #i28701#
        Method has *only* to contain actions, which has to be performed on
        *every* assignment of the corresponding flag to <false>.
    */
    virtual void _ActionOnInvalidation( const InvalidationType _nInvalid );

    // draw shadow and borders
    void PaintShadow( const SwRect&, SwRect&, const SwBorderAttrs& ) const;
    virtual void  Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;

    virtual const IDocumentDrawModelAccess& getIDocumentDrawModelAccess( );

public:
    virtual css::uno::Sequence< css::style::TabStop > GetTabStopInfo( SwTwips )
    {
        return css::uno::Sequence< css::style::TabStop >();
    }

    TYPEINFO_OVERRIDE(); // already in base class

    sal_uInt16 GetType() const { return mnFrmType; }

    static SwCache &GetCache()                { return *mpCache; }
    static SwCache *GetCachePtr()             { return mpCache;  }
    static void     SetCache( SwCache *pNew ) { mpCache = pNew;  }

    // change PrtArea size and FrmSize
    SwTwips Shrink( SwTwips, bool bTst = false, bool bInfo = false );
    SwTwips Grow  ( SwTwips, bool bTst = false, bool bInfo = false );

    // different methods for inserting in layout tree (for performance reasons)

    // insert before pBehind or at the end of the chain below mpUpper
    void InsertBefore( SwLayoutFrm* pParent, SwFrm* pBehind );
    // insert after pBefore or at the beginning of the chain below mpUpper
    void InsertBehind( SwLayoutFrm *pParent, SwFrm *pBefore );
    // insert before pBehind or at the end of the chain while considering
    // the siblings of pSct
    bool InsertGroupBefore( SwFrm* pParent, SwFrm* pWhere, SwFrm* pSct );
    void RemoveFromLayout();

    // For internal use only - who ignores this will be put in a sack and has
    // to stay there for two days
    // Does special treatment for _Get[Next|Prev]Leaf() (for tables).
    SwLayoutFrm *GetLeaf( MakePageType eMakePage, bool bFwd );
    SwLayoutFrm *GetNextLeaf   ( MakePageType eMakePage );
    SwLayoutFrm *GetNextFootnoteLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetNextSctLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetNextCellLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetPrevLeaf   ( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevFootnoteLeaf( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevSctLeaf( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevCellLeaf( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    const SwLayoutFrm *GetLeaf ( MakePageType eMakePage, bool bFwd,
                                 const SwFrm *pAnch ) const;

    bool WrongPageDesc( SwPageFrm* pNew );

    //#i28701# - new methods to append/remove drawing objects
    void AppendDrawObj( SwAnchoredObject& _rNewObj );
    void RemoveDrawObj( SwAnchoredObject& _rToRemoveObj );

    // work with chain of FlyFrms
    void  AppendFly( SwFlyFrm *pNew );
    void  RemoveFly( SwFlyFrm *pToRemove );
    const SwSortedObjs *GetDrawObjs() const { return mpDrawObjs; }
          SwSortedObjs *GetDrawObjs()         { return mpDrawObjs; }
    // #i28701# - change purpose of method and adjust its name
    void InvalidateObjs( const bool _bInvaPosOnly,
                         const bool _bNoInvaOfAsCharAnchoredObjs = true );

    virtual void PaintBorder( const SwRect&, const SwPageFrm *pPage,
                              const SwBorderAttrs & ) const;
    void PaintBaBo( const SwRect&, const SwPageFrm *pPage = 0,
                    const bool bLowerBorder = false, const bool bOnlyTextBackground = false ) const;
    void PaintBackground( const SwRect&, const SwPageFrm *pPage,
                          const SwBorderAttrs &,
                          const bool bLowerMode = false,
                          const bool bLowerBorder = false,
                          const bool bOnlyTextBackground = false ) const;
    void PaintBorderLine( const SwRect&, const SwRect&, const SwPageFrm*,
                          const Color *pColor, const editeng::SvxBorderStyle =
                ::com::sun::star::table::BorderLineStyle::SOLID ) const;

    drawinglayer::processor2d::BaseProcessor2D * CreateProcessor2D( ) const;
    void ProcessPrimitives( const drawinglayer::primitive2d::Primitive2DSequence& rSequence ) const;

    // FIXME: EasyHack (refactoring): rename method name in all files
    // retouch, not in the area of the given Rect!
    void Retouche( const SwPageFrm *pPage, const SwRect &rRect ) const;

    bool GetBackgroundBrush(
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
        const SvxBrushItem*& rpBrush,
        const Color*& rpColor,
        SwRect &rOrigRect,
        bool bLowerMode ) const;

    inline void SetCompletePaint() const;
    inline void ResetCompletePaint() const;
    inline bool IsCompletePaint() const { return mbCompletePaint; }

    inline void SetRetouche() const;
    inline void ResetRetouche() const;
    inline bool IsRetouche() const { return mbRetouche; }

    void SetInfFlags();
    inline void InvalidateInfFlags() { mbInfInvalid = true; }
    inline bool IsInDocBody() const;    // use InfoFlags, determine flags
    inline bool IsInFootnote() const;        // if necessary
    inline bool IsInTab() const;
    inline bool IsInFly() const;
    inline bool IsInSct() const;

    // If frame is inside a split table row, this function returns
    // the corresponding row frame in the follow table.
    const SwRowFrm* IsInSplitTableRow() const;

    // If frame is inside a follow flow row, this function returns
    // the corresponding row frame master table
    const SwRowFrm* IsInFollowFlowRow() const;

    bool IsInBalancedSection() const;

    inline bool IsReverse() const { return mbReverse; }
    inline bool IsVertical() const;

    inline bool IsVertLR() const;
    inline bool GetVerticalFlag() const { return mbVertical; }

    inline void SetDerivedVert( bool bNew ){ mbDerivedVert = bNew; }
    inline void SetInvalidVert( bool bNew) { mbInvalidVert = bNew; }
    inline bool IsRightToLeft() const;
    inline bool GetRightToLeftFlag() const { return mbRightToLeft; }
    inline void SetDerivedR2L( bool bNew ) { mbDerivedR2L  = bNew; }
    inline void SetInvalidR2L( bool bNew ) { mbInvalidR2L  = bNew; }

    void CheckDirChange();
    // returns upper left frame position for LTR and
    // upper right frame position for Asian / RTL frames
    Point   GetFrmAnchorPos( bool bIgnoreFlysAnchoredAtThisFrame ) const;

    /** determine, if frame is moveable in given environment

        method replaced 'old' method <bool IsMoveable() const>.
        Determines, if frame is moveable in given environment. if no environment
        is given (parameter _pLayoutFrm == 0L), the movability in the actual
        environment (<this->GetUpper()) is checked.

        @param _pLayoutFrm
        input parameter - given environment (layout frame), in which the movability
        will be checked. If not set ( == 0L ), actual environment is taken.

        @return boolean, indicating, if frame is moveable in given environment
    */
    bool IsMoveable( const SwLayoutFrm* _pLayoutFrm = 0L ) const;

    // Is it permitted for the (Text)Frm to add a footnote in the current
    // environment (not e.g. for repeating table headlines)
    bool IsFootnoteAllowed() const;

    virtual void  Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = 0 );

    virtual void CheckDirection( bool bVert );

    void ReinitializeFrmSizeAttrFlags();

    const SwAttrSet *GetAttrSet() const;

    inline bool HasFixSize() const { return mbFixSize; }

    // check all pages (starting from the given) and correct them if needed
    static void CheckPageDescs( SwPageFrm *pStart, bool bNotifyFields = true, SwPageFrm** ppPrev = 0);

    // might return 0, with and without const
    SwFrm               *GetNext()  { return mpNext; }
    SwFrm               *GetPrev()  { return mpPrev; }
    SwLayoutFrm         *GetUpper() { return mpUpper; }
    SwRootFrm           *getRootFrm(){ return mpRoot; }
    SwPageFrm           *FindPageFrm();
    SwFrm               *FindColFrm();
    SwRowFrm            *FindRowFrm();
    SwFootnoteBossFrm        *FindFootnoteBossFrm( bool bFootnotes = false );
    SwTabFrm            *ImplFindTabFrm();
    SwFootnoteFrm            *ImplFindFootnoteFrm();
    SwFlyFrm            *ImplFindFlyFrm();
    SwSectionFrm        *ImplFindSctFrm();
    SwFrm               *FindFooterOrHeader();
    SwFrm               *GetLower();
    const SwFrm         *GetNext()  const { return mpNext; }
    const SwFrm         *GetPrev()  const { return mpPrev; }
    const SwLayoutFrm   *GetUpper() const { return mpUpper; }
    const SwRootFrm     *getRootFrm()   const { return mpRoot; }
    inline SwTabFrm     *FindTabFrm();
    inline SwFootnoteFrm     *FindFootnoteFrm();
    inline SwFlyFrm     *FindFlyFrm();
    inline SwSectionFrm *FindSctFrm();
    inline SwFrm        *FindNext();
    // #i27138# - add parameter <_bInSameFootnote>
    SwContentFrm* FindNextCnt( const bool _bInSameFootnote = false );
    inline SwFrm        *FindPrev();
    inline const SwPageFrm *FindPageFrm() const;
    inline const SwFootnoteBossFrm *FindFootnoteBossFrm( bool bFootnote = false ) const;
    inline const SwFrm     *FindColFrm() const;
    inline const SwFrm     *FindFooterOrHeader() const;
    inline const SwTabFrm  *FindTabFrm() const;
    inline const SwFootnoteFrm  *FindFootnoteFrm() const;
    inline const SwFlyFrm  *FindFlyFrm() const;
    inline const SwSectionFrm *FindSctFrm() const;
    inline const SwFrm     *FindNext() const;
    // #i27138# - add parameter <_bInSameFootnote>
    const SwContentFrm* FindNextCnt( const bool _bInSameFootnote = false ) const;
    inline const SwFrm     *FindPrev() const;
           const SwFrm     *GetLower()  const;

    SwContentFrm* FindPrevCnt( const bool _bInSameFootnote = false );

    const SwContentFrm* FindPrevCnt( const bool _bInSameFootnote = false ) const;

    // #i79774#
    SwFrm* _GetIndPrev() const;
    SwFrm* GetIndPrev() const
        { return ( mpPrev || !IsInSct() ) ? mpPrev : _GetIndPrev(); }

    SwFrm* GetIndNext()
        { return ( mpNext || !IsInSct() ) ? mpNext : _GetIndNext(); }
    const SwFrm* GetIndNext() const { return const_cast<SwFrm*>(this)->GetIndNext(); }

    sal_uInt16 GetPhyPageNum() const;   // page number without offset
    sal_uInt16 GetVirtPageNum() const;  // page number with offset
    bool OnRightPage() const { return 0 != GetPhyPageNum() % 2; };
    bool WannaRightPage() const;
    bool OnFirstPage() const;

    inline const  SwLayoutFrm *GetPrevLayoutLeaf() const;
    inline const  SwLayoutFrm *GetNextLayoutLeaf() const;
    inline SwLayoutFrm *GetPrevLayoutLeaf();
    inline SwLayoutFrm *GetNextLayoutLeaf();

    virtual void Calc(vcl::RenderContext* pRenderContext) const; // here might be "formatted"
    inline void OptCalc() const;    // here we assume (for optimization) that
                                    // the predecessors are already formatted

    Point   GetRelPos() const;
    const  SwRect &Frm() const { return maFrm; }
    const  SwRect &Prt() const { return maPrt; }

    // PaintArea is the area where content might be displayed.
    // The margin of a page or the space between columns belongs to it.
    const SwRect PaintArea() const;
    // UnionFrm is the union of Frm- and PrtArea, normally identical
    // to the FrmArea except in case of negative Prt margins.
    const SwRect UnionFrm( bool bBorder = false ) const;

    // HACK: Here we exceptionally allow direct access to members.
    // This should not delude into changing those value randomly; it is the
    // only option to circumvent compiler problems (same method with public
    // and protected).
    SwRect &Frm() { return maFrm; }
    SwRect &Prt() { return maPrt; }

    virtual Size ChgSize( const Size& aNewSize );

    virtual void Cut() = 0;
    //Add a method to change the acc table dispose state.
    void SetAccTableDispose(bool bDispose) { mbIfAccTableShouldDisposing = bDispose;}
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) = 0;

    void ValidateLineNum() { mbValidLineNum = true; }

    bool GetValidPosFlag()    const { return mbValidPos; }
    bool GetValidPrtAreaFlag()const { return mbValidPrtArea; }
    bool GetValidSizeFlag()   const { return mbValidSize; }
    bool GetValidLineNumFlag()const { return mbValidLineNum; }
    bool IsValid() const { return mbValidPos && mbValidSize && mbValidPrtArea; }

    // Only invalidate Frm
    // #i28701# - add call to method <_ActionOnInvalidation(..)>
    //            for all invalidation methods.
    // #i28701# - use method <_InvalidationAllowed(..)> to
    //            decide, if invalidation will to be performed or not.
    // #i26945# - no additional invalidation, if it's already
    //            invalidate.
    void _InvalidateSize()
    {
        if ( mbValidSize && _InvalidationAllowed( INVALID_SIZE ) )
        {
            mbValidSize = false;
            _ActionOnInvalidation( INVALID_SIZE );
        }
    }
    void _InvalidatePrt()
    {
        if ( mbValidPrtArea && _InvalidationAllowed( INVALID_PRTAREA ) )
        {
            mbValidPrtArea = false;
            _ActionOnInvalidation( INVALID_PRTAREA );
        }
    }
    void _InvalidatePos()
    {
        if ( mbValidPos && _InvalidationAllowed( INVALID_POS ) )
        {
            mbValidPos = false;
            _ActionOnInvalidation( INVALID_POS );
        }
    }
    void _InvalidateLineNum()
    {
        if ( mbValidLineNum && _InvalidationAllowed( INVALID_LINENUM ) )
        {
            mbValidLineNum = false;
            _ActionOnInvalidation( INVALID_LINENUM );
        }
    }
    void _InvalidateAll()
    {
        if ( ( mbValidSize || mbValidPrtArea || mbValidPos ) &&
             _InvalidationAllowed( INVALID_ALL ) )
        {
            mbValidSize = mbValidPrtArea = mbValidPos = false;
            _ActionOnInvalidation( INVALID_ALL );
        }
    }
    // also notify page at the same time
    inline void InvalidateSize();
    inline void InvalidatePrt();
    inline void InvalidatePos();
    inline void InvalidateLineNum();
    inline void InvalidateAll();
    void ImplInvalidateSize();
    void ImplInvalidatePrt();
    void ImplInvalidatePos();
    void ImplInvalidateLineNum();

    inline void InvalidateNextPos( bool bNoFootnote = false );
    void ImplInvalidateNextPos( bool bNoFootnote = false );

    /** method to invalidate printing area of next frame
        #i11859#
    */
    void InvalidateNextPrtArea();

    void InvalidatePage( const SwPageFrm *pPage = 0 ) const;

    virtual bool    FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    virtual bool    GetCrsrOfst( SwPosition *, Point&,
                                 SwCrsrMoveState* = 0, bool bTestBackground = false ) const;
    virtual bool    GetCharRect( SwRect &, const SwPosition&,
                                 SwCrsrMoveState* = 0 ) const;
    virtual void Paint( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;

    // HACK: shortcut between frame and formatting
    // It's your own fault if you cast void* incorrectly! In any case check
    // the void* for 0.
    virtual bool Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, bool bNotify = true );

    // true if it is the correct class, false otherwise
    inline bool IsLayoutFrm() const;
    inline bool IsRootFrm() const;
    inline bool IsPageFrm() const;
    inline bool IsColumnFrm() const;
    inline bool IsFootnoteBossFrm() const;  // footnote bosses might be PageFrms or ColumnFrms
    inline bool IsHeaderFrm() const;
    inline bool IsFooterFrm() const;
    inline bool IsFootnoteContFrm() const;
    inline bool IsFootnoteFrm() const;
    inline bool IsBodyFrm() const;
    inline bool IsColBodyFrm() const;  // implemented in layfrm.hxx, BodyFrm above ColumnFrm
    inline bool IsPageBodyFrm() const; // implemented in layfrm.hxx, BodyFrm above PageFrm
    inline bool IsFlyFrm() const;
    inline bool IsSctFrm() const;
    inline bool IsTabFrm() const;
    inline bool IsRowFrm() const;
    inline bool IsCellFrm() const;
    inline bool IsContentFrm() const;
    inline bool IsTextFrm() const;
    inline bool IsNoTextFrm() const;
    // Frms where its PrtArea depends on their neighbors and that are
    // positioned in the content flow
    inline bool IsFlowFrm() const;
    // Frms that are capable of retouching or that might need to retouch behind
    // themselves
    inline bool IsRetoucheFrm() const;
    inline bool IsAccessibleFrm() const;

    void PrepareCrsr();                 // CrsrShell is allowed to call this

    // Is the Frm (or the section containing it) protected? Same for Fly in
    // Fly in ... and footnotes
    bool IsProtected() const;

    bool IsColLocked()  const { return mbColLocked; }
    bool IsDeleteForbidden()  const { return mbForbidDelete; }

    /// this is the only way to delete a SwFrm instance
    static void DestroyFrm(SwFrm *const pFrm);

    bool IsInDtor() const { return mbInDtor; }

    // No inline cause we need the function pointers
    long GetTopMargin() const;
    long GetBottomMargin() const;
    long GetLeftMargin() const;
    long GetRightMargin() const;
    void SetTopBottomMargins( long, long );
    void SetBottomTopMargins( long, long );
    void SetLeftRightMargins( long, long );
    void SetRightLeftMargins( long, long );
    long GetPrtLeft() const;
    long GetPrtBottom() const;
    long GetPrtRight() const;
    long GetPrtTop() const;
    bool SetMinLeft( long );
    bool SetMaxBottom( long );
    bool SetMaxRight( long );
    bool SetMinTop( long );
    void MakeBelowPos( const SwFrm*, const SwFrm*, bool );
    void MakeUpperPos( const SwFrm*, const SwFrm*, bool );
    void MakeLeftPos( const SwFrm*, const SwFrm*, bool );
    void MakeRightPos( const SwFrm*, const SwFrm*, bool );
    inline bool IsNeighbourFrm() const
        { return (GetType() & FRM_NEIGHBOUR) != 0; }

    // #i65250#
    inline sal_uInt32 GetFrmId() const { return mnFrmId; }

    // NEW TABLES
    // Some functions for covered/covering table cells. This way unnecessary
    // includes can be avoided
    bool IsLeaveUpperAllowed() const;
    bool IsCoveredCell() const;
    bool IsInCoveredCell() const;

    // #i81146# new loop control
    bool KnowsFormat( const SwFormat& rFormat ) const;
    void RegisterToFormat( SwFormat& rFormat );
    void ValidateThisAndAllLowers( const sal_uInt16 nStage );

    void ForbidDelete()      { mbForbidDelete = true; }
    void AllowDelete()    { mbForbidDelete = false; }

    //UUUU
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const;
    bool supportsFullDrawingLayerFillAttributeSet() const;

public:
    // if writer is NULL, dumps the layout structure as XML in layout.xml
    virtual void dumpAsXml(xmlTextWriterPtr writer = NULL) const;
    void dumpInfosAsXml(xmlTextWriterPtr writer) const;
    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const;
    void dumpChildrenAsXml(xmlTextWriterPtr writer) const;
    bool IsCollapse() const;
};

inline bool SwFrm::IsInDocBody() const
{
    if ( mbInfInvalid )
        const_cast<SwFrm*>(this)->SetInfFlags();
    return mbInfBody;
}
inline bool SwFrm::IsInFootnote() const
{
    if ( mbInfInvalid )
        const_cast<SwFrm*>(this)->SetInfFlags();
    return mbInfFootnote;
}
inline bool SwFrm::IsInTab() const
{
    if ( mbInfInvalid )
        const_cast<SwFrm*>(this)->SetInfFlags();
    return mbInfTab;
}
inline bool SwFrm::IsInFly() const
{
    if ( mbInfInvalid )
        const_cast<SwFrm*>(this)->SetInfFlags();
    return mbInfFly;
}
inline bool SwFrm::IsInSct() const
{
    if ( mbInfInvalid )
        const_cast<SwFrm*>(this)->SetInfFlags();
    return mbInfSct;
}
bool SwFrm::IsVertical() const
{
    if( mbInvalidVert )
        const_cast<SwFrm*>(this)->SetDirFlags( true );
    return mbVertical;
}
inline bool SwFrm::IsVertLR() const
{
    return mbVertLR;
}
inline bool SwFrm::IsRightToLeft() const
{
    if( mbInvalidR2L )
        const_cast<SwFrm*>(this)->SetDirFlags( false );
    return mbRightToLeft;
}

inline void SwFrm::SetCompletePaint() const
{
    const_cast<SwFrm*>(this)->mbCompletePaint = true;
}
inline void SwFrm::ResetCompletePaint() const
{
    const_cast<SwFrm*>(this)->mbCompletePaint = false;
}

inline void SwFrm::SetRetouche() const
{
    const_cast<SwFrm*>(this)->mbRetouche = true;
}
inline void SwFrm::ResetRetouche() const
{
    const_cast<SwFrm*>(this)->mbRetouche = false;
}

inline SwLayoutFrm *SwFrm::GetNextLayoutLeaf()
{
    return const_cast<SwLayoutFrm*>(static_cast<const SwFrm*>(this)->GetNextLayoutLeaf());
}
inline SwLayoutFrm *SwFrm::GetPrevLayoutLeaf()
{
    return const_cast<SwLayoutFrm*>(static_cast<const SwFrm*>(this)->GetPrevLayoutLeaf());
}
inline const SwLayoutFrm *SwFrm::GetNextLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( true );
}
inline const SwLayoutFrm *SwFrm::GetPrevLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( false );
}

inline void SwFrm::InvalidateSize()
{
    if ( mbValidSize )
        ImplInvalidateSize();
}
inline void SwFrm::InvalidatePrt()
{
    if ( mbValidPrtArea )
        ImplInvalidatePrt();
}
inline void SwFrm::InvalidatePos()
{
    if ( mbValidPos )
        ImplInvalidatePos();
}
inline void SwFrm::InvalidateLineNum()
{
    if ( mbValidLineNum )
        ImplInvalidateLineNum();
}
inline void SwFrm::InvalidateAll()
{
    if ( _InvalidationAllowed( INVALID_ALL ) )
    {
        if ( mbValidPrtArea && mbValidSize && mbValidPos  )
            ImplInvalidatePos();
        mbValidPrtArea = mbValidSize = mbValidPos = false;

        // #i28701#
        _ActionOnInvalidation( INVALID_ALL );
    }
}
inline void SwFrm::InvalidateNextPos( bool bNoFootnote )
{
    if ( mpNext && !mpNext->IsSctFrm() )
        mpNext->InvalidatePos();
    else
        ImplInvalidateNextPos( bNoFootnote );
}

inline void SwFrm::OptCalc() const
{
    if ( !mbValidPos || !mbValidPrtArea || !mbValidSize )
        const_cast<SwFrm*>(this)->OptPrepareMake();
}
inline const SwPageFrm *SwFrm::FindPageFrm() const
{
    return const_cast<SwFrm*>(this)->FindPageFrm();
}
inline const SwFrm *SwFrm::FindColFrm() const
{
    return const_cast<SwFrm*>(this)->FindColFrm();
}
inline const SwFrm *SwFrm::FindFooterOrHeader() const
{
    return const_cast<SwFrm*>(this)->FindFooterOrHeader();
}
inline SwTabFrm *SwFrm::FindTabFrm()
{
    return IsInTab() ? ImplFindTabFrm() : 0;
}
inline const SwFootnoteBossFrm *SwFrm::FindFootnoteBossFrm( bool bFootnote ) const
{
    return const_cast<SwFrm*>(this)->FindFootnoteBossFrm( bFootnote );
}
inline SwFootnoteFrm *SwFrm::FindFootnoteFrm()
{
    return IsInFootnote() ? ImplFindFootnoteFrm() : 0;
}
inline SwFlyFrm *SwFrm::FindFlyFrm()
{
    return IsInFly() ? ImplFindFlyFrm() : 0;
}
inline SwSectionFrm *SwFrm::FindSctFrm()
{
    return IsInSct() ? ImplFindSctFrm() : 0;
}

inline const SwTabFrm *SwFrm::FindTabFrm() const
{
    return IsInTab() ? const_cast<SwFrm*>(this)->ImplFindTabFrm() : 0;
}
inline const SwFootnoteFrm *SwFrm::FindFootnoteFrm() const
{
    return IsInFootnote() ? const_cast<SwFrm*>(this)->ImplFindFootnoteFrm() : 0;
}
inline const SwFlyFrm *SwFrm::FindFlyFrm() const
{
    return IsInFly() ? const_cast<SwFrm*>(this)->ImplFindFlyFrm() : 0;
}
inline const SwSectionFrm *SwFrm::FindSctFrm() const
{
    return IsInSct() ? const_cast<SwFrm*>(this)->ImplFindSctFrm() : 0;
}
inline SwFrm *SwFrm::FindNext()
{
    if ( mpNext )
        return mpNext;
    else
        return _FindNext();
}
inline const SwFrm *SwFrm::FindNext() const
{
    if ( mpNext )
        return mpNext;
    else
        return const_cast<SwFrm*>(this)->_FindNext();
}
inline SwFrm *SwFrm::FindPrev()
{
    if ( mpPrev && !mpPrev->IsSctFrm() )
        return mpPrev;
    else
        return _FindPrev();
}
inline const SwFrm *SwFrm::FindPrev() const
{
    if ( mpPrev && !mpPrev->IsSctFrm() )
        return mpPrev;
    else
        return const_cast<SwFrm*>(this)->_FindPrev();
}

inline bool SwFrm::IsLayoutFrm() const
{
    return (GetType() & FRM_LAYOUT) != 0;
}
inline bool SwFrm::IsRootFrm() const
{
    return mnFrmType == FRM_ROOT;
}
inline bool SwFrm::IsPageFrm() const
{
    return mnFrmType == FRM_PAGE;
}
inline bool SwFrm::IsColumnFrm() const
{
    return mnFrmType == FRM_COLUMN;
}
inline bool SwFrm::IsFootnoteBossFrm() const
{
    return (GetType() & FRM_FTNBOSS) != 0;
}
inline bool SwFrm::IsHeaderFrm() const
{
    return mnFrmType == FRM_HEADER;
}
inline bool SwFrm::IsFooterFrm() const
{
    return mnFrmType == FRM_FOOTER;
}
inline bool SwFrm::IsFootnoteContFrm() const
{
    return mnFrmType == FRM_FTNCONT;
}
inline bool SwFrm::IsFootnoteFrm() const
{
    return mnFrmType == FRM_FTN;
}
inline bool SwFrm::IsBodyFrm() const
{
    return mnFrmType == FRM_BODY;
}
inline bool SwFrm::IsFlyFrm() const
{
    return mnFrmType == FRM_FLY;
}
inline bool SwFrm::IsSctFrm() const
{
    return mnFrmType == FRM_SECTION;
}
inline bool SwFrm::IsTabFrm() const
{
    return mnFrmType == FRM_TAB;
}
inline bool SwFrm::IsRowFrm() const
{
    return mnFrmType == FRM_ROW;
}
inline bool SwFrm::IsCellFrm() const
{
    return mnFrmType == FRM_CELL;
}
inline bool SwFrm::IsContentFrm() const
{
    return (GetType() & FRM_CNTNT) != 0;
}
inline bool SwFrm::IsTextFrm() const
{
    return mnFrmType == FRM_TXT;
}
inline bool SwFrm::IsNoTextFrm() const
{
    return mnFrmType == FRM_NOTXT;
}
inline bool SwFrm::IsFlowFrm() const
{
    return (GetType() & (FRM_CNTNT|FRM_TAB|FRM_SECTION)) != 0;
}
inline bool SwFrm::IsRetoucheFrm() const
{
    return (GetType() & (FRM_CNTNT|FRM_TAB|FRM_SECTION|FRM_FTN)) != 0;
}
inline bool SwFrm::IsAccessibleFrm() const
{
    return (GetType() & FRM_ACCESSIBLE) != 0;
}

//use this to protect a SwFrm for a given scope from getting deleted
class SwFrmDeleteGuard
{
private:
    SwFrm *m_pFrm;
    bool m_bOldDeleteAllowed;
public:
    //Flag pFrm for SwFrmDeleteGuard lifetime that we shouldn't delete
    //it in e.g. SwSectionFrm::MergeNext etc because we will need it
    //again after the SwFrmDeleteGuard dtor
    SwFrmDeleteGuard(SwFrm* pFrm)
        : m_pFrm(pFrm)
    {
        m_bOldDeleteAllowed = m_pFrm && !m_pFrm->IsDeleteForbidden();
        if (m_bOldDeleteAllowed)
            m_pFrm->ForbidDelete();
    }

    ~SwFrmDeleteGuard()
    {
        if (m_bOldDeleteAllowed)
            m_pFrm->AllowDelete();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
