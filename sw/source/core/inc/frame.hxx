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
#include <svl/brdcst.hxx>
#include "IDocumentDrawModelAccess.hxx"

#include <com/sun/star/style/TabStop.hpp>

using namespace ::com::sun::star;
class SwLayoutFrm;
class SwRootFrm;
class SwPageFrm;
class SwFlyFrm;
class SwSectionFrm;
class SwFtnFrm;
class SwFtnBossFrm;
class SwTabFrm;
class SwRowFrm;
class SwFlowFrm;
class SwCntntFrm;
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
class SwFmt;
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

// The type of the frame is internal represented by the 4-bit value mnType,
// which can expanded to the types above by shifting a bit (0x1 << mnType)
// Here are the corresponding defines for the compressed representation:
#define FRMC_ROOT        0
#define FRMC_PAGE        1
#define FRMC_COLUMN      2
#define FRMC_HEADER      3
#define FRMC_FOOTER      4
#define FRMC_FTNCONT     5
#define FRMC_FTN         6
#define FRMC_BODY        7
#define FRMC_FLY         8
#define FRMC_SECTION     9
#define FRMC_UNUSED      10
#define FRMC_TAB         11
#define FRMC_ROW         12
#define FRMC_CELL        13
#define FRMC_TXT         14
#define FRMC_NOTXT       15

#define FRM_NEIGHBOUR   0x2004
#define FRM_NOTE_VERT   0x7a60
#define FRM_HEADFOOT    0x0018
#define FRM_BODYFTNC    0x00a0

class SwFrm;
typedef long (SwFrm:: *SwFrmGet)() const;
typedef sal_Bool (SwFrm:: *SwFrmMax)( long );
typedef void (SwFrm:: *SwFrmMakePos)( const SwFrm*, const SwFrm*, sal_Bool );
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

// Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
//End of SCMS
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

/**
 * Base class of the Writer layout elements.
 *
 * This includes not only fly frames, but everything down to the paragraph
 * level: pages, headers, footers, etc. (Inside a paragraph SwLinePortion
 * instances are used.)
 */
class SwFrm: public SwClient, public SfxBroadcaster
{
    // the hidden Frm
    friend class SwFlowFrm;
    friend class SwLayoutFrm;
    friend class SwLooping;

    // voids lower during creation of a column
    friend SwFrm *SaveCntnt( SwLayoutFrm *, SwFrm* pStart );
    friend void   RestoreCntnt( SwFrm *, SwLayoutFrm *, SwFrm *pSibling, bool bGrow );

#ifdef DBG_UTIL
    // remove empty SwSectionFrms from a chain
    friend SwFrm* SwClearDummies( SwFrm* pFrm );
#endif

    // for validating a mistakenly invalidated one in SwCntntFrm::MakeAll
    friend void ValidateSz( SwFrm *pFrm );
    // implemented in text/txtftn.cxx, prevents Ftn oscillation
    friend void ValidateTxt( SwFrm *pFrm );

    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );

    // cache for (border) attributes
    static SwCache *mpCache;

    bool mbIfAccTableShouldDisposing;

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
        #i27138# - adding parameter <_bInSameFtn>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        @param _bInSameFtn
        input parameter - boolean indicating, that the found next content
        frame has to be in the same footnote frame. This parameter is only
        relevant for flow frames in footnotes.

        @return SwCntntFrm*
        pointer to the found next content frame. It's NULL, if none exists.
    */
    SwCntntFrm* _FindNextCnt( const bool _bInSameFtn = false );

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
        #i27138# - adding parameter <_bInSameFtn>
        Its default value is <false>. If its value is <true>, the environment
        'All footnotes' is no longer treated. Instead each footnote is treated
        as an own environment.

        @param _bInSameFtn
        input parameter - boolean indicating, that the found previous content
        frame has to be in the same footnote frame. This parameter is only
        relevant for flow frames in footnotes.

        @return SwCntntFrm*
        pointer to the found previous content frame. It's NULL, if none exists.
    */
    SwCntntFrm* _FindPrevCnt( const bool _bInSameFtn = false );

    void _UpdateAttrFrm( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 & );
    SwFrm* _GetIndNext();
    void SetDirFlags( sal_Bool bVert );

    const SwLayoutFrm* ImplGetNextLayoutLeaf( bool bFwd ) const;

protected:
    SwSortedObjs* mpDrawObjs;    // draw objects, can be 0

    SwRect  maFrm;   // absolute position in document and size of the Frm
    SwRect  maPrt;   // position relatively to Frm and size of PrtArea

    sal_uInt16 mbReverse     : 1; // Next line above/at the right side instead
                                 // under/at the left side of the previous line
    sal_uInt16 mbInvalidR2L  : 1;
    sal_uInt16 mbDerivedR2L  : 1;
    sal_uInt16 mbRightToLeft : 1;
    sal_uInt16 mbInvalidVert : 1;
    sal_uInt16 mbDerivedVert : 1;
    sal_uInt16 mbVertical    : 1;
    // Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    sal_uInt16 mbVertLR      : 1;
    sal_uInt16 mnType        : 4;  //Who am I?

    sal_Bool mbValidPos      : 1;
    sal_Bool mbValidPrtArea  : 1;
    sal_Bool mbValidSize     : 1;
    sal_Bool mbValidLineNum  : 1;
    sal_Bool mbFixSize       : 1;
    // if sal_True, frame will be painted completely even content was changed
    // only partially. For CntntFrms a border (from Action) will exclusively
    // painted if <mbCompletePaint> is sal_True.
    sal_Bool mbCompletePaint : 1;
    sal_Bool mbRetouche      : 1; // frame is responsible for retouching

protected:
    sal_Bool mbInfInvalid    : 1;  // InfoFlags are invalid
    sal_Bool mbInfBody       : 1;  // Frm is in document body
    sal_Bool mbInfTab        : 1;  // Frm is in a table
    sal_Bool mbInfFly        : 1;  // Frm is in a Fly
    sal_Bool mbInfFtn        : 1;  // Frm is in a footnote
    sal_Bool mbInfSct        : 1;  // Frm is in a section
    sal_Bool mbColLocked     : 1;  // lock Grow/Shrink for column-wise section
                                  // or fly frames, will be set in Format

    void ColLock()      { mbColLocked = sal_True; }
    void ColUnlock()    { mbColLocked = sal_False; }

    void Destroy(); // for ~SwRootFrm

    // Only used by SwRootFrm Ctor to get 'this' into mpRoot...
    void setRootFrm( SwRootFrm* pRoot ) { mpRoot = pRoot; }

    SwPageFrm *InsertPage( SwPageFrm *pSibling, sal_Bool bFtn );
    void PrepareMake();
    void OptPrepareMake();
    void MakePos();
    // Format next frame of table frame to assure keeping attributes.
    // In case of nested tables method <SwFrm::MakeAll()> is called to
    // avoid formatting of superior table frame.
    friend SwFrm* sw_FormatNextCntntForKeep( SwTabFrm* pTabFrm );

    virtual void MakeAll() = 0;
    // adjust frames of a page
    SwTwips AdjustNeighbourhood( SwTwips nDiff, sal_Bool bTst = sal_False );

    // change only frame size not the size of PrtArea
    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False ) = 0;
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False ) = 0;

    SwModify        *GetDep()       { return GetRegisteredInNonConst(); }
    const SwModify  *GetDep() const { return GetRegisteredIn(); }

    SwFrm( SwModify*, SwFrm* );

    void CheckDir( sal_uInt16 nDir, sal_Bool bVert, sal_Bool bOnlyBiDi, sal_Bool bBrowse );

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
        *every* assignment of the corresponding flag to <sal_False>.
    */
    virtual void _ActionOnInvalidation( const InvalidationType _nInvalid );

    // draw shadow and borders
    void PaintShadow( const SwRect&, SwRect&, const SwBorderAttrs& ) const;
    virtual void  Modify( const SfxPoolItem*, const SfxPoolItem* );

    virtual const IDocumentDrawModelAccess* getIDocumentDrawModelAccess( );

public:
    virtual uno::Sequence< style::TabStop > GetTabStopInfo( SwTwips )
    {
        return uno::Sequence< style::TabStop >();
    }

    TYPEINFO(); // already in base class

    sal_uInt16 GetType() const { return 0x1 << mnType; }

    static SwCache &GetCache()                { return *mpCache; }
    static SwCache *GetCachePtr()             { return mpCache;  }
    static void     SetCache( SwCache *pNew ) { mpCache = pNew;  }

    // change PrtArea size and FrmSize
    SwTwips Shrink( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    SwTwips Grow  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );

    // different methods for inserting in layout tree (for performance reasons)

    // insert before pBehind or at the end of the chain below mpUpper
    void InsertBefore( SwLayoutFrm* pParent, SwFrm* pBehind );
    // insert after pBefore or at the beginnig of the chain below mpUpper
    void InsertBehind( SwLayoutFrm *pParent, SwFrm *pBefore );
    // insert before pBehind or at the end of the chain while considering
    // the siblings of pSct
    void InsertGroupBefore( SwFrm* pParent, SwFrm* pWhere, SwFrm* pSct );
    void Remove();

    // For internal use only - who ignores this will be put in a sack and has
    // to stay there for two days
    // Does special treatment for _Get[Next|Prev]Leaf() (for tables).
    SwLayoutFrm *GetLeaf( MakePageType eMakePage, sal_Bool bFwd );
    SwLayoutFrm *GetNextLeaf   ( MakePageType eMakePage );
    SwLayoutFrm *GetNextFtnLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetNextSctLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetNextCellLeaf( MakePageType eMakePage );
    SwLayoutFrm *GetPrevLeaf   ( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevFtnLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevSctLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    SwLayoutFrm *GetPrevCellLeaf( MakePageType eMakeFtn = MAKEPAGE_FTN );
    const SwLayoutFrm *GetLeaf ( MakePageType eMakePage, sal_Bool bFwd,
                                 const SwFrm *pAnch ) const;

    sal_Bool WrongPageDesc( SwPageFrm* pNew );

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
                    const sal_Bool bLowerBorder = sal_False, const bool bOnlyTxtBackground = false ) const;
    void PaintBackground( const SwRect&, const SwPageFrm *pPage,
                          const SwBorderAttrs &,
                          const sal_Bool bLowerMode = sal_False,
                          const sal_Bool bLowerBorder = sal_False,
                          const bool bOnlyTxtBackground = false ) const;
    void PaintBorderLine( const SwRect&, const SwRect&, const SwPageFrm*,
                          const Color *pColor, const editeng::SvxBorderStyle =
                ::com::sun::star::table::BorderLineStyle::SOLID ) const;

    drawinglayer::processor2d::BaseProcessor2D * CreateProcessor2D( ) const;
    void ProcessPrimitives( const drawinglayer::primitive2d::Primitive2DSequence& rSequence ) const;

// FIXME: EasyHack (refactoring): rename method name in all files
    // retouch, not in the area of the given Rect!
    void Retouche( const SwPageFrm *pPage, const SwRect &rRect ) const;

    sal_Bool GetBackgroundBrush( const SvxBrushItem*& rpBrush,
                             const XFillStyleItem* & rpFillStyle,
                             const XFillGradientItem* & rpFillGradient,
                             const Color*& rpColor,
                             SwRect &rOrigRect,
                             sal_Bool bLowerMode ) const;

    inline void SetCompletePaint() const;
    inline void ResetCompletePaint() const;
    inline sal_Bool IsCompletePaint() const { return mbCompletePaint; }

    inline void SetRetouche() const;
    inline void ResetRetouche() const;
    inline sal_Bool IsRetouche() const { return mbRetouche; }

    void SetInfFlags();
    inline void InvalidateInfFlags() { mbInfInvalid = sal_True; }
    inline bool IsInDocBody() const;    // use InfoFlags, determine flags
    inline sal_Bool IsInFtn() const;        // if necessary
    inline sal_Bool IsInTab() const;
    inline sal_Bool IsInFly() const;
    inline sal_Bool IsInSct() const;

    // If frame is inside a split table row, this function returns
    // the corresponding row frame in the follow table.
    const SwRowFrm* IsInSplitTableRow() const;

    // If frame is inside a follow flow row, this function returns
    // the corresponding row frame master table
    const SwRowFrm* IsInFollowFlowRow() const;

    bool IsInBalancedSection() const;

    inline bool IsReverse() const { return mbReverse; }
    inline void SetReverse( sal_Bool bNew ){ mbReverse = bNew ? 1 : 0; }
    inline bool IsVertical() const;
    //Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    inline sal_Bool IsVertLR() const;
    inline sal_Bool GetVerticalFlag() const;
    inline void SetVertical( sal_Bool bNew ){ mbVertical = bNew ? 1 : 0; }
    //Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    inline void SetbVertLR( sal_Bool bNew ) { mbVertLR = bNew ? 1 : 0; }
    inline void SetDerivedVert( sal_Bool bNew ){ mbDerivedVert = bNew ? 1 : 0; }
    inline void SetInvalidVert( sal_Bool bNew) { mbInvalidVert = bNew ? 1 : 0; }
    inline bool IsRightToLeft() const;
    inline sal_Bool GetRightToLeftFlag() const;
    inline void SetRightToLeft( sal_Bool bNew ){ mbRightToLeft = bNew ? 1 : 0; }
    inline void SetDerivedR2L( sal_Bool bNew ) { mbDerivedR2L  = bNew ? 1 : 0; }
    inline void SetInvalidR2L( sal_Bool bNew ) { mbInvalidR2L  = bNew ? 1 : 0; }

    void CheckDirChange();
    // returns upper left frame position for LTR and
    // upper right frame position for Asian / RTL frames
    Point   GetFrmAnchorPos( sal_Bool bIgnoreFlysAnchoredAtThisFrame ) const;

    /** determine, if frame is moveable in given environment

        method replaced 'old' method <sal_Bool IsMoveable() const>.
        Determines, if frame is moveable in given environment. if no environment
        is given (parameter _pLayoutFrm == 0L), the movability in the actual
        environment (<this->GetUpper()) is checked.

        @param _pLayoutFrm
        input parameter - given environment (layout frame), in which the movability
        will be checked. If not set ( == 0L ), actual environment is taken.

        @return boolean, indicating, if frame is moveable in given environment
    */
    bool IsMoveable( const SwLayoutFrm* _pLayoutFrm = 0L ) const;

    // Is it permitted for the (Txt)Frm to add a footnote in the current
    // environment (not e.g. for repeating table headlines)
    sal_Bool IsFtnAllowed() const;

    virtual void  Format( const SwBorderAttrs *pAttrs = 0 );

    virtual void CheckDirection( bool bVert );

    void ReinitializeFrmSizeAttrFlags();

    const SwAttrSet *GetAttrSet() const;

    inline sal_Bool HasFixSize() const { return mbFixSize; }
    inline void SetFixSize( sal_Bool bNew ) { mbFixSize = bNew; }

    // check all pages (starting from the given) and correct them if needed
    static void CheckPageDescs( SwPageFrm *pStart, sal_Bool bNotifyFields = sal_True, SwPageFrm** ppPrev = 0);

    // might return 0, with and without const
    SwFrm               *GetNext()  { return mpNext; }
    SwFrm               *GetPrev()  { return mpPrev; }
    SwLayoutFrm         *GetUpper() { return mpUpper; }
    SwRootFrm           *getRootFrm(){ return mpRoot; }
    SwPageFrm           *FindPageFrm();
    SwFrm               *FindColFrm();
    SwRowFrm            *FindRowFrm();
    SwFtnBossFrm        *FindFtnBossFrm( sal_Bool bFootnotes = sal_False );
    SwTabFrm            *ImplFindTabFrm();
    SwFtnFrm            *ImplFindFtnFrm();
    SwFlyFrm            *ImplFindFlyFrm();
    SwSectionFrm        *ImplFindSctFrm();
    SwFrm               *FindFooterOrHeader();
    SwFrm               *GetLower();
    const SwFrm         *GetNext()  const { return mpNext; }
    const SwFrm         *GetPrev()  const { return mpPrev; }
    const SwLayoutFrm   *GetUpper() const { return mpUpper; }
    const SwRootFrm     *getRootFrm()   const { return mpRoot; }
    inline SwTabFrm     *FindTabFrm();
    inline SwFtnFrm     *FindFtnFrm();
    inline SwFlyFrm     *FindFlyFrm();
    inline SwSectionFrm *FindSctFrm();
    inline SwFrm        *FindNext();
    // #i27138# - add parameter <_bInSameFtn>
    inline SwCntntFrm* FindNextCnt( const bool _bInSameFtn = false );
    inline SwFrm        *FindPrev();
    inline const SwPageFrm *FindPageFrm() const;
    inline const SwFtnBossFrm *FindFtnBossFrm( sal_Bool bFtn = sal_False ) const;
    inline const SwFrm     *FindColFrm() const;
    inline const SwFrm     *FindFooterOrHeader() const;
    inline const SwTabFrm  *FindTabFrm() const;
    inline const SwFtnFrm  *FindFtnFrm() const;
    inline const SwFlyFrm  *FindFlyFrm() const;
    inline const SwSectionFrm *FindSctFrm() const;
    inline const SwFrm     *FindNext() const;
    // #i27138# - add parameter <_bInSameFtn>
    inline const SwCntntFrm* FindNextCnt( const bool _bInSameFtn = false ) const;
    inline const SwFrm     *FindPrev() const;
           const SwFrm     *GetLower()  const;

    /** inline wrapper method for <_FindPrevCnt(..)>
        #i27138#
    */
    inline SwCntntFrm* FindPrevCnt( const bool _bInSameFtn = false )
    {
        if ( GetPrev() && GetPrev()->IsCntntFrm() )
            return (SwCntntFrm*)(GetPrev());
        else
            return _FindPrevCnt( _bInSameFtn );
    }

    /** inline const wrapper method for <_FindPrevCnt(..)>
        #i27138#
    */
    inline const SwCntntFrm* FindPrevCnt( const bool _bInSameFtn = false ) const
    {
        if ( GetPrev() && GetPrev()->IsCntntFrm() )
            return (const SwCntntFrm*)(GetPrev());
        else
            return const_cast<SwFrm*>(this)->_FindPrevCnt( _bInSameFtn );
    }

    // #i79774#
    SwFrm* _GetIndPrev() const;
    SwFrm* GetIndPrev() const
        { return ( mpPrev || !IsInSct() ) ? mpPrev : _GetIndPrev(); }

    SwFrm* GetIndNext()
        { return ( mpNext || !IsInSct() ) ? mpNext : _GetIndNext(); }
    const SwFrm* GetIndNext() const { return ((SwFrm*)this)->GetIndNext(); }

    sal_uInt16 GetPhyPageNum() const;   // page number without offset
    sal_uInt16 GetVirtPageNum() const;  // page number with offset
    sal_Bool OnRightPage() const { return 0 != GetPhyPageNum() % 2; };
    sal_Bool WannaRightPage() const;
    bool OnFirstPage() const;

    inline const  SwLayoutFrm *GetPrevLayoutLeaf() const;
    inline const  SwLayoutFrm *GetNextLayoutLeaf() const;
    inline SwLayoutFrm *GetPrevLayoutLeaf();
    inline SwLayoutFrm *GetNextLayoutLeaf();

    inline void Calc() const;       // here might be "formatted"
    inline void OptCalc() const;    // here we assume (for optimization) that
                                    // the predecessors are already formatted

    inline Point   GetRelPos() const;
    const  SwRect &Frm() const { return maFrm; }
    const  SwRect &Prt() const { return maPrt; }

    // PaintArea is the area where content might be displayed.
    // The margin of a page or the space between columns belongs to it.
    const SwRect PaintArea() const;
    // UnionFrm is the union of Frm- and PrtArea, normally identical
    // to the FrmArea except in case of negative Prt margins.
    const SwRect UnionFrm( sal_Bool bBorder = sal_False ) const;

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

    void ValidateLineNum() { mbValidLineNum = sal_True; }

    sal_Bool GetValidPosFlag()    const { return mbValidPos; }
    sal_Bool GetValidPrtAreaFlag()const { return mbValidPrtArea; }
    sal_Bool GetValidSizeFlag()   const { return mbValidSize; }
    sal_Bool GetValidLineNumFlag()const { return mbValidLineNum; }
    sal_Bool IsValid() const { return mbValidPos && mbValidSize && mbValidPrtArea; }

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
            mbValidSize = sal_False;
            _ActionOnInvalidation( INVALID_SIZE );
        }
    }
    void _InvalidatePrt()
    {
        if ( mbValidPrtArea && _InvalidationAllowed( INVALID_PRTAREA ) )
        {
            mbValidPrtArea = sal_False;
            _ActionOnInvalidation( INVALID_PRTAREA );
        }
    }
    void _InvalidatePos()
    {
        if ( mbValidPos && _InvalidationAllowed( INVALID_POS ) )
        {
            mbValidPos = sal_False;
            _ActionOnInvalidation( INVALID_POS );
        }
    }
    void _InvalidateLineNum()
    {
        if ( mbValidLineNum && _InvalidationAllowed( INVALID_LINENUM ) )
        {
            mbValidLineNum = sal_False;
            _ActionOnInvalidation( INVALID_LINENUM );
        }
    }
    void _InvalidateAll()
    {
        if ( ( mbValidSize || mbValidPrtArea || mbValidPos ) &&
             _InvalidationAllowed( INVALID_ALL ) )
        {
            mbValidSize = mbValidPrtArea = mbValidPos = sal_False;
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

    inline void InvalidateNextPos( sal_Bool bNoFtn = sal_False );
    void ImplInvalidateNextPos( sal_Bool bNoFtn = sal_False );

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
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;

    // HACK: shortcut between frame and formatting
    // It's your own fault if you cast void* incorrectly! In any case check
    // the void* for 0.
    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, bool bNotify = true );

    // sal_True if it is the correct class, sal_False otherwise
    inline bool IsLayoutFrm() const;
    inline bool IsRootFrm() const;
    inline bool IsPageFrm() const;
    inline bool IsColumnFrm() const;
    inline bool IsFtnBossFrm() const;  // footnote bosses might be PageFrms or ColumnFrms
    inline bool IsHeaderFrm() const;
    inline bool IsFooterFrm() const;
    inline bool IsFtnContFrm() const;
    inline bool IsFtnFrm() const;
    inline bool IsBodyFrm() const;
    inline bool IsColBodyFrm() const;  // implemented in layfrm.hxx, BodyFrm above ColumnFrm
    inline bool IsPageBodyFrm() const; // implemented in layfrm.hxx, BodyFrm above PageFrm
    inline bool IsFlyFrm() const;
    inline bool IsSctFrm() const;
    inline bool IsTabFrm() const;
    inline bool IsRowFrm() const;
    inline bool IsCellFrm() const;
    inline bool IsCntntFrm() const;
    inline bool IsTxtFrm() const;
    inline bool IsNoTxtFrm() const;
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
    sal_Bool IsProtected() const;

    sal_Bool IsColLocked()  const { return mbColLocked; }

    virtual ~SwFrm();

    // No inline cause we need the function pointers
    long GetTopMargin() const;
    long GetBottomMargin() const;
    long GetLeftMargin() const;
    long GetRightMargin() const;
    void SetTopBottomMargins( long, long );
    void SetBottomTopMargins( long, long );
    void SetLeftRightMargins( long, long );
    void SetRightLeftMargins( long, long );
    void SetLeftAndWidth( long nLeft, long nWidth );
    void SetTopAndHeight( long nTop, long nHeight );
    void SetRightAndWidth( long nRight, long nWidth );
    void SetBottomAndHeight( long nBottom, long nHeight );
    long GetPrtLeft() const;
    long GetPrtBottom() const;
    long GetPrtRight() const;
    long GetPrtTop() const;
    sal_Bool SetMinLeft( long );
    sal_Bool SetMaxBottom( long );
    sal_Bool SetMaxRight( long );
    sal_Bool SetMinTop( long );
    void MakeBelowPos( const SwFrm*, const SwFrm*, sal_Bool );
    void MakeUpperPos( const SwFrm*, const SwFrm*, sal_Bool );
    void MakeLeftPos( const SwFrm*, const SwFrm*, sal_Bool );
    void MakeRightPos( const SwFrm*, const SwFrm*, sal_Bool );
    inline bool IsNeighbourFrm() const
        { return (GetType() & FRM_NEIGHBOUR) != 0; }

    // #i65250#
    inline sal_uInt32 GetFrmId() const { return mnFrmId; }
    inline sal_uInt32 GetLastFrmId() const { return mnLastFrmId; }

    // NEW TABELS
    // Some functions for covered/covering table cells. This way unnessessary
    // includes can be avoided
    bool IsLeaveUpperAllowed() const;
    bool IsCoveredCell() const;
    bool IsInCoveredCell() const;

    // #i81146# new loop control
    bool KnowsFormat( const SwFmt& rFmt ) const;
    void RegisterToFormat( SwFmt& rFmt );
    void ValidateThisAndAllLowers( const sal_uInt16 nStage );

public:
    // if writer is NULL, dumps the layout structure as XML in layout.xml
    virtual void dumpAsXml(xmlTextWriterPtr writer = NULL);
    virtual void dumpInfosAsXml(xmlTextWriterPtr writer);
    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer);
    void dumpChildrenAsXml(xmlTextWriterPtr writer);
    bool IsCollapse() const;
};

inline bool SwFrm::IsInDocBody() const
{
    if ( mbInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return mbInfBody;
}
inline sal_Bool SwFrm::IsInFtn() const
{
    if ( mbInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return mbInfFtn;
}
inline sal_Bool SwFrm::IsInTab() const
{
    if ( mbInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return mbInfTab;
}
inline sal_Bool SwFrm::IsInFly() const
{
    if ( mbInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return mbInfFly;
}
inline sal_Bool SwFrm::IsInSct() const
{
    if ( mbInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return mbInfSct;
}
bool SwFrm::IsVertical() const
{
    if( mbInvalidVert )
        ((SwFrm*)this)->SetDirFlags( sal_True );
    return mbVertical != 0;
}
//Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
inline sal_Bool SwFrm::IsVertLR() const
{
    return mbVertLR != 0;
}
//End of SCMS
sal_Bool SwFrm::GetVerticalFlag() const
{
    return mbVertical != 0;
}
inline bool SwFrm::IsRightToLeft() const
{
    if( mbInvalidR2L )
        ((SwFrm*)this)->SetDirFlags( sal_False );
    return mbRightToLeft != 0;
}
sal_Bool SwFrm::GetRightToLeftFlag() const
{
    return mbRightToLeft != 0;
}

inline void SwFrm::SetCompletePaint() const
{
    ((SwFrm*)this)->mbCompletePaint = sal_True;
}
inline void SwFrm::ResetCompletePaint() const
{
    ((SwFrm*)this)->mbCompletePaint = sal_False;
}

inline void SwFrm::SetRetouche() const
{
    ((SwFrm*)this)->mbRetouche = sal_True;
}
inline void SwFrm::ResetRetouche() const
{
    ((SwFrm*)this)->mbRetouche = sal_False;
}

inline SwLayoutFrm *SwFrm::GetNextLayoutLeaf()
{
    return (SwLayoutFrm*)((const SwFrm*)this)->GetNextLayoutLeaf();
}
inline SwLayoutFrm *SwFrm::GetPrevLayoutLeaf()
{
    return (SwLayoutFrm*)((const SwFrm*)this)->GetPrevLayoutLeaf();
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
        mbValidPrtArea = mbValidSize = mbValidPos = sal_False;

        // #i28701#
        _ActionOnInvalidation( INVALID_ALL );
    }
}
inline void SwFrm::InvalidateNextPos( sal_Bool bNoFtn )
{
    if ( mpNext && !mpNext->IsSctFrm() )
        mpNext->InvalidatePos();
    else
        ImplInvalidateNextPos( bNoFtn );
}

inline void SwFrm::Calc() const
{
    if ( !mbValidPos || !mbValidPrtArea || !mbValidSize )
        ((SwFrm*)this)->PrepareMake();
}
inline void SwFrm::OptCalc() const
{
    if ( !mbValidPos || !mbValidPrtArea || !mbValidSize )
        ((SwFrm*)this)->OptPrepareMake();
}

inline Point SwFrm::GetRelPos() const
{
    Point aRet( maFrm.Pos() );
    // here we cast since SwLayoutFrm is declared only as forwarded
    aRet -= ((SwFrm*)GetUpper())->Prt().Pos();
    aRet -= ((SwFrm*)GetUpper())->Frm().Pos();
    return aRet;
}

inline const SwPageFrm *SwFrm::FindPageFrm() const
{
    return ((SwFrm*)this)->FindPageFrm();
}
inline const SwFrm *SwFrm::FindColFrm() const
{
    return ((SwFrm*)this)->FindColFrm();
}
inline const SwFrm *SwFrm::FindFooterOrHeader() const
{
    return ((SwFrm*)this)->FindFooterOrHeader();
}
inline SwTabFrm *SwFrm::FindTabFrm()
{
    return IsInTab() ? ImplFindTabFrm() : 0;
}
inline const SwFtnBossFrm *SwFrm::FindFtnBossFrm( sal_Bool bFtn ) const
{
    return ((SwFrm*)this)->FindFtnBossFrm( bFtn );
}
inline SwFtnFrm *SwFrm::FindFtnFrm()
{
    return IsInFtn() ? ImplFindFtnFrm() : 0;
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
    return IsInTab() ? ((SwFrm*)this)->ImplFindTabFrm() : 0;
}
inline const SwFtnFrm *SwFrm::FindFtnFrm() const
{
    return IsInFtn() ? ((SwFrm*)this)->ImplFindFtnFrm() : 0;
}
inline const SwFlyFrm *SwFrm::FindFlyFrm() const
{
    return IsInFly() ? ((SwFrm*)this)->ImplFindFlyFrm() : 0;
}
inline const SwSectionFrm *SwFrm::FindSctFrm() const
{
    return IsInSct() ? ((SwFrm*)this)->ImplFindSctFrm() : 0;
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
        return ((SwFrm*)this)->_FindNext();
}
// #i27138# - add parameter <_bInSameFtn>
inline SwCntntFrm *SwFrm::FindNextCnt( const bool _bInSameFtn )
{
    if ( mpNext && mpNext->IsCntntFrm() )
        return (SwCntntFrm*)mpNext;
    else
        return _FindNextCnt( _bInSameFtn );
}
// #i27138# - add parameter <_bInSameFtn>
inline const SwCntntFrm *SwFrm::FindNextCnt( const bool _bInSameFtn ) const
{
    if ( mpNext && mpNext->IsCntntFrm() )
        return (SwCntntFrm*)mpNext;
    else
        return ((SwFrm*)this)->_FindNextCnt( _bInSameFtn );
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
        return ((SwFrm*)this)->_FindPrev();
}

inline bool SwFrm::IsLayoutFrm() const
{
    return GetType() & FRM_LAYOUT;
}
inline bool SwFrm::IsRootFrm() const
{
    return mnType == FRMC_ROOT;
}
inline bool SwFrm::IsPageFrm() const
{
    return mnType == FRMC_PAGE;
}
inline bool SwFrm::IsColumnFrm() const
{
    return mnType == FRMC_COLUMN;
}
inline bool SwFrm::IsFtnBossFrm() const
{
    return GetType() & FRM_FTNBOSS;
}
inline bool SwFrm::IsHeaderFrm() const
{
    return mnType == FRMC_HEADER;
}
inline bool SwFrm::IsFooterFrm() const
{
    return mnType == FRMC_FOOTER;
}
inline bool SwFrm::IsFtnContFrm() const
{
    return mnType == FRMC_FTNCONT;
}
inline bool SwFrm::IsFtnFrm() const
{
    return mnType == FRMC_FTN;
}
inline bool SwFrm::IsBodyFrm() const
{
    return mnType == FRMC_BODY;
}
inline bool SwFrm::IsFlyFrm() const
{
    return mnType == FRMC_FLY;
}
inline bool SwFrm::IsSctFrm() const
{
    return mnType == FRMC_SECTION;
}
inline bool SwFrm::IsTabFrm() const
{
    return mnType == FRMC_TAB;
}
inline bool SwFrm::IsRowFrm() const
{
    return mnType == FRMC_ROW;
}
inline bool SwFrm::IsCellFrm() const
{
    return mnType == FRMC_CELL;
}
inline bool SwFrm::IsCntntFrm() const
{
    return GetType() & FRM_CNTNT;
}
inline bool SwFrm::IsTxtFrm() const
{
    return mnType == FRMC_TXT;
}
inline bool SwFrm::IsNoTxtFrm() const
{
    return mnType == FRMC_NOTXT;
}
inline bool SwFrm::IsFlowFrm() const
{
    return GetType() & 0xCA00;   //TabFrm, CntntFrm, SectionFrm
}
inline bool SwFrm::IsRetoucheFrm() const
{
    return GetType() & 0xCA40;   //TabFrm, CntntFrm, SectionFrm, Ftnfrm
}
inline bool SwFrm::IsAccessibleFrm() const
{
    return GetType() & FRM_ACCESSIBLE;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
