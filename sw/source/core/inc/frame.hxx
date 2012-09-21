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

#ifndef SW_FRAME_HXX
#define SW_FRAME_HXX

#include <vector>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <editeng/borderline.hxx>
#include "swtypes.hxx"
#include "swrect.hxx"
#include "calbck.hxx"
#include <svl/brdcst.hxx>
#include "IDocumentDrawModelAccess.hxx"

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
class ViewShell;
class Color;
class SwBorderAttrs;
class SwCache;
class SvxBrushItem;
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

// The type of the frame is internal represented by the 4-bit value nType,
// which can expanded to the types above by shifting a bit (0x1 << nType)
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
#define SWRECTFN( pFrm )    sal_Bool bVert = pFrm->IsVertical(); \
                            sal_Bool bRev = pFrm->IsReverse(); \
                            sal_Bool bVertL2R = pFrm->IsVertLR(); \
                            SwRectFn fnRect = bVert ? \
                                ( bRev ? fnRectVL2R : ( bVertL2R ? fnRectVertL2R : fnRectVert ) ): \
                                ( bRev ? fnRectB2T : fnRectHori );
#define SWRECTFNX( pFrm )   sal_Bool bVertX = pFrm->IsVertical(); \
                            sal_Bool bRevX = pFrm->IsReverse(); \
                            sal_Bool bVertL2RX = pFrm->IsVertLR(); \
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
#define SWRECTFN2( pFrm )   sal_Bool bVert = pFrm->IsVertical(); \
                sal_Bool bVertL2R = pFrm->IsVertLR(); \
                            sal_Bool bNeighb = pFrm->IsNeighbourFrm(); \
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
    friend SwFrm *SaveCntnt( SwLayoutFrm *, SwFrm* pStart = NULL );
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
    static SwCache *pCache;

    // #i65250#
    // frame ID is now in general available - used for layout loop control
    static sal_uInt32 mnLastFrmId;
    const  sal_uInt32 mnFrmId;

    SwRootFrm   *mpRoot;
    SwLayoutFrm *pUpper;
    SwFrm       *pNext;
    SwFrm       *pPrev;

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
    SwSortedObjs* pDrawObjs;    // draw objects, can be 0

    SwRect  aFrm;   // absolute position in document and size of the Frm
    SwRect  aPrt;   // position relatively to Frm and size of PrtArea

    sal_uInt16 bReverse     : 1; // Next line above/at the right side instead
                                 // under/at the left side of the previous line
    sal_uInt16 bInvalidR2L  : 1;
    sal_uInt16 bDerivedR2L  : 1;
    sal_uInt16 bRightToLeft : 1;
    sal_uInt16 bInvalidVert : 1;
    sal_uInt16 bDerivedVert : 1;
    sal_uInt16 bVertical    : 1;
    // Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    sal_uInt16 bVertLR      : 1;
    sal_uInt16 nType        : 4;  //Who am I?

    sal_Bool bValidPos      : 1;
    sal_Bool bValidPrtArea  : 1;
    sal_Bool bValidSize     : 1;
    sal_Bool bValidLineNum  : 1;
    sal_Bool bFixSize       : 1;
    sal_Bool bUnUsed1       : 1;
    // if sal_True, frame will be painted completely even content was changed
    // only partially. For CntntFrms a border (from Action) will exclusively
    // painted if <bCompletePaint> is sal_True.
    sal_Bool bCompletePaint : 1;
    sal_Bool bRetouche      : 1; // frame is responsible for retouching

protected:
    sal_Bool bInfInvalid    : 1;  // InfoFlags are invalid
    sal_Bool bInfBody       : 1;  // Frm is in document body
    sal_Bool bInfTab        : 1;  // Frm is in a table
    sal_Bool bInfFly        : 1;  // Frm is in a Fly
    sal_Bool bInfFtn        : 1;  // Frm is in a footnote
    sal_Bool bInfSct        : 1;  // Frm is in a section
    sal_Bool bColLocked     : 1;  // lock Grow/Shrink for column-wise section
                                  // or fly frames, will be set in Format

    void ColLock()      { bColLocked = sal_True; }
    void ColUnlock()    { bColLocked = sal_False; }

    void Destroy(); // for ~SwRootFrm

    // Only used by SwRootFrm Ctor to get 'this' into mpRoot...
    void setRootFrm( SwRootFrm* pRoot ) { mpRoot = pRoot; }

    SwPageFrm *InsertPage( SwPageFrm *pSibling, sal_Bool bFtn );
    void PrepareMake();
    void OptPrepareMake();
    void MakePos();
    // Format next frame of table frame to assure keeping attributes.
    // In case of nested tables method <SwFrm::MakeAll()> is called to
    // avoid formating of superior table frame.
    friend SwFrm* lcl_FormatNextCntntForKeep( SwTabFrm* pTabFrm );

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
    TYPEINFO(); // already in base class

    sal_uInt16 GetType() const { return 0x1 << nType; }

    static SwCache &GetCache()                { return *pCache; }
    static SwCache *GetCachePtr()             { return pCache;  }
    static void     SetCache( SwCache *pNew ) { pCache = pNew;  }

    // change PrtArea size and FrmSize
    SwTwips Shrink( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    SwTwips Grow  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );

    // different methods for inserting in layout tree (for performance reasons)

    // insert before pBehind or at the end of the chain below pUpper
    void InsertBefore( SwLayoutFrm* pParent, SwFrm* pBehind );
    // insert after pBefore or at the beginnig of the chain below pUpper
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
    const SwSortedObjs *GetDrawObjs() const { return pDrawObjs; }
          SwSortedObjs *GetDrawObjs()         { return pDrawObjs; }
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
                             const Color*& rpColor,
                             SwRect &rOrigRect,
                             sal_Bool bLowerMode ) const;

    inline void SetCompletePaint() const;
    inline void ResetCompletePaint() const;
    inline sal_Bool IsCompletePaint() const { return bCompletePaint; }

    inline void SetRetouche() const;
    inline void ResetRetouche() const;
    inline sal_Bool IsRetouche() const { return bRetouche; }

    void SetInfFlags();
    inline void InvalidateInfFlags() { bInfInvalid = sal_True; }
    inline sal_Bool IsInDocBody() const;    // use InfoFlags, determine flags
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

    inline sal_Bool IsReverse() const { return bReverse; }
    inline void SetReverse( sal_Bool bNew ){ bReverse = bNew ? 1 : 0; }
    inline sal_Bool IsVertical() const;
    //Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    inline sal_Bool IsVertLR() const;
    inline sal_Bool GetVerticalFlag() const;
    inline void SetVertical( sal_Bool bNew ){ bVertical = bNew ? 1 : 0; }
    //Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    inline void SetbVertLR( sal_Bool bNew ) { bVertLR = bNew ? 1 : 0; }
    inline void SetDerivedVert( sal_Bool bNew ){ bDerivedVert = bNew ? 1 : 0; }
    inline void SetInvalidVert( sal_Bool bNew) { bInvalidVert = bNew ? 1 : 0; }
    inline sal_Bool IsRightToLeft() const;
    inline sal_Bool GetRightToLeftFlag() const;
    inline void SetRightToLeft( sal_Bool bNew ){ bRightToLeft = bNew ? 1 : 0; }
    inline void SetDerivedR2L( sal_Bool bNew ) { bDerivedR2L  = bNew ? 1 : 0; }
    inline void SetInvalidR2L( sal_Bool bNew ) { bInvalidR2L  = bNew ? 1 : 0; }

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

    virtual void  CheckDirection( sal_Bool bVert );

    void ReinitializeFrmSizeAttrFlags();

    const SwAttrSet *GetAttrSet() const;

    inline sal_Bool HasFixSize() const { return bFixSize; }
    inline void SetFixSize( sal_Bool bNew ) { bFixSize = bNew; }

    // check all pages (starting from the given) and correct them if needed
    static void CheckPageDescs( SwPageFrm *pStart, sal_Bool bNotifyFields = sal_True );

    // might return 0, with and without const
    SwFrm               *GetNext()  { return pNext; }
    SwFrm               *GetPrev()  { return pPrev; }
    SwLayoutFrm         *GetUpper() { return pUpper; }
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
    const SwFrm         *GetNext()  const { return pNext; }
    const SwFrm         *GetPrev()  const { return pPrev; }
    const SwLayoutFrm   *GetUpper() const { return pUpper; }
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
        { return ( pPrev || !IsInSct() ) ? pPrev : _GetIndPrev(); }

    SwFrm* GetIndNext()
        { return ( pNext || !IsInSct() ) ? pNext : _GetIndNext(); }
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
    const  SwRect &Frm() const { return aFrm; }
    const  SwRect &Prt() const { return aPrt; }

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
    SwRect &Frm() { return aFrm; }
    SwRect &Prt() { return aPrt; }

    virtual Size ChgSize( const Size& aNewSize );

    virtual void Cut() = 0;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) = 0;

    void ValidateLineNum() { bValidLineNum = sal_True; }

    sal_Bool GetValidPosFlag()    const { return bValidPos; }
    sal_Bool GetValidPrtAreaFlag()const { return bValidPrtArea; }
    sal_Bool GetValidSizeFlag()   const { return bValidSize; }
    sal_Bool GetValidLineNumFlag()const { return bValidLineNum; }
    sal_Bool IsValid() const { return bValidPos && bValidSize && bValidPrtArea; }

    // Only invalidate Frm
    // #i28701# - add call to method <_ActionOnInvalidation(..)>
    //            for all invalidation methods.
    // #i28701# - use method <_InvalidationAllowed(..)> to
    //            decide, if invalidation will to be performed or not.
    // #i26945# - no additional invalidation, if it's already
    //            invalidate.
    void _InvalidateSize()
    {
        if ( bValidSize && _InvalidationAllowed( INVALID_SIZE ) )
        {
            bValidSize = sal_False;
            _ActionOnInvalidation( INVALID_SIZE );
        }
    }
    void _InvalidatePrt()
    {
        if ( bValidPrtArea && _InvalidationAllowed( INVALID_PRTAREA ) )
        {
            bValidPrtArea = sal_False;
            _ActionOnInvalidation( INVALID_PRTAREA );
        }
    }
    void _InvalidatePos()
    {
        if ( bValidPos && _InvalidationAllowed( INVALID_POS ) )
        {
            bValidPos = sal_False;
            _ActionOnInvalidation( INVALID_POS );
        }
    }
    void _InvalidateLineNum()
    {
        if ( bValidLineNum && _InvalidationAllowed( INVALID_LINENUM ) )
        {
            bValidLineNum = sal_False;
            _ActionOnInvalidation( INVALID_LINENUM );
        }
    }
    void _InvalidateAll()
    {
        if ( ( bValidSize || bValidPrtArea || bValidPos ) &&
             _InvalidationAllowed( INVALID_ALL ) )
        {
            bValidSize = bValidPrtArea = bValidPos = sal_False;
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

    virtual sal_Bool    GetCrsrOfst( SwPosition *, Point&,
                                 SwCrsrMoveState* = 0 ) const;
    virtual sal_Bool    GetCharRect( SwRect &, const SwPosition&,
                                 SwCrsrMoveState* = 0 ) const;
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;

    // HACK: shortcut between frame and formatting
    // It's your own fault if you cast void* incorrectly! In any case check
    // the void* for 0.
    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, sal_Bool bNotify = sal_True );

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

    sal_Bool IsColLocked()  const { return bColLocked; }

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
    inline sal_Bool IsNeighbourFrm() const
        { return GetType() & FRM_NEIGHBOUR ? sal_True : sal_False; }

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

inline sal_Bool SwFrm::IsInDocBody() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfBody;
}
inline sal_Bool SwFrm::IsInFtn() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfFtn;
}
inline sal_Bool SwFrm::IsInTab() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfTab;
}
inline sal_Bool SwFrm::IsInFly() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfFly;
}
inline sal_Bool SwFrm::IsInSct() const
{
    if ( bInfInvalid )
        ((SwFrm*)this)->SetInfFlags();
    return bInfSct;
}
sal_Bool SwFrm::IsVertical() const
{
    if( bInvalidVert )
        ((SwFrm*)this)->SetDirFlags( sal_True );
    return bVertical != 0;
}
//Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
inline sal_Bool SwFrm::IsVertLR() const
{
    return bVertLR != 0;
}
//End of SCMS
sal_Bool SwFrm::GetVerticalFlag() const
{
    return bVertical != 0;
}
inline sal_Bool SwFrm::IsRightToLeft() const
{
    if( bInvalidR2L )
        ((SwFrm*)this)->SetDirFlags( sal_False );
    return bRightToLeft != 0;
}
sal_Bool SwFrm::GetRightToLeftFlag() const
{
    return bRightToLeft != 0;
}

inline void SwFrm::SetCompletePaint() const
{
    ((SwFrm*)this)->bCompletePaint = sal_True;
}
inline void SwFrm::ResetCompletePaint() const
{
    ((SwFrm*)this)->bCompletePaint = sal_False;
}

inline void SwFrm::SetRetouche() const
{
    ((SwFrm*)this)->bRetouche = sal_True;
}
inline void SwFrm::ResetRetouche() const
{
    ((SwFrm*)this)->bRetouche = sal_False;
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
    if ( bValidSize )
        ImplInvalidateSize();
}
inline void SwFrm::InvalidatePrt()
{
    if ( bValidPrtArea )
        ImplInvalidatePrt();
}
inline void SwFrm::InvalidatePos()
{
    if ( bValidPos )
        ImplInvalidatePos();
}
inline void SwFrm::InvalidateLineNum()
{
    if ( bValidLineNum )
        ImplInvalidateLineNum();
}
inline void SwFrm::InvalidateAll()
{
    if ( _InvalidationAllowed( INVALID_ALL ) )
    {
        if ( bValidPrtArea && bValidSize && bValidPos  )
            ImplInvalidatePos();
        bValidPrtArea = bValidSize = bValidPos = sal_False;

        // #i28701#
        _ActionOnInvalidation( INVALID_ALL );
    }
}
inline void SwFrm::InvalidateNextPos( sal_Bool bNoFtn )
{
    if ( pNext && !pNext->IsSctFrm() )
        pNext->InvalidatePos();
#ifndef C30 // maybe it works for C40?
    else
        ImplInvalidateNextPos( bNoFtn );
#else
    if ( !pNext )
        ImplInvalidateNextPos( bNoFtn );
#endif
}

inline void SwFrm::Calc() const
{
    if ( !bValidPos || !bValidPrtArea || !bValidSize )
        ((SwFrm*)this)->PrepareMake();
}
inline void SwFrm::OptCalc() const
{
    if ( !bValidPos || !bValidPrtArea || !bValidSize )
        ((SwFrm*)this)->OptPrepareMake();
}

inline Point SwFrm::GetRelPos() const
{
    Point aRet( aFrm.Pos() );
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
    if ( pNext )
        return pNext;
    else
        return _FindNext();
}
inline const SwFrm *SwFrm::FindNext() const
{
    if ( pNext )
        return pNext;
    else
        return ((SwFrm*)this)->_FindNext();
}
// #i27138# - add parameter <_bInSameFtn>
inline SwCntntFrm *SwFrm::FindNextCnt( const bool _bInSameFtn )
{
    if ( pNext && pNext->IsCntntFrm() )
        return (SwCntntFrm*)pNext;
    else
        return _FindNextCnt( _bInSameFtn );
}
// #i27138# - add parameter <_bInSameFtn>
inline const SwCntntFrm *SwFrm::FindNextCnt( const bool _bInSameFtn ) const
{
    if ( pNext && pNext->IsCntntFrm() )
        return (SwCntntFrm*)pNext;
    else
        return ((SwFrm*)this)->_FindNextCnt( _bInSameFtn );
}
inline SwFrm *SwFrm::FindPrev()
{
    if ( pPrev && !pPrev->IsSctFrm() )
        return pPrev;
    else
        return _FindPrev();
}
inline const SwFrm *SwFrm::FindPrev() const
{
    if ( pPrev && !pPrev->IsSctFrm() )
        return pPrev;
    else
        return ((SwFrm*)this)->_FindPrev();
}


inline bool SwFrm::IsLayoutFrm() const
{
    return GetType() & FRM_LAYOUT;
}
inline bool SwFrm::IsRootFrm() const
{
    return nType == FRMC_ROOT;
}
inline bool SwFrm::IsPageFrm() const
{
    return nType == FRMC_PAGE;
}
inline bool SwFrm::IsColumnFrm() const
{
    return nType == FRMC_COLUMN;
}
inline bool SwFrm::IsFtnBossFrm() const
{
    return GetType() & FRM_FTNBOSS;
}
inline bool SwFrm::IsHeaderFrm() const
{
    return nType == FRMC_HEADER;
}
inline bool SwFrm::IsFooterFrm() const
{
    return nType == FRMC_FOOTER;
}
inline bool SwFrm::IsFtnContFrm() const
{
    return nType == FRMC_FTNCONT;
}
inline bool SwFrm::IsFtnFrm() const
{
    return nType == FRMC_FTN;
}
inline bool SwFrm::IsBodyFrm() const
{
    return nType == FRMC_BODY;
}
inline bool SwFrm::IsFlyFrm() const
{
    return nType == FRMC_FLY;
}
inline bool SwFrm::IsSctFrm() const
{
    return nType == FRMC_SECTION;
}
inline bool SwFrm::IsTabFrm() const
{
    return nType == FRMC_TAB;
}
inline bool SwFrm::IsRowFrm() const
{
    return nType == FRMC_ROW;
}
inline bool SwFrm::IsCellFrm() const
{
    return nType == FRMC_CELL;
}
inline bool SwFrm::IsCntntFrm() const
{
    return GetType() & FRM_CNTNT;
}
inline bool SwFrm::IsTxtFrm() const
{
    return nType == FRMC_TXT;
}
inline bool SwFrm::IsNoTxtFrm() const
{
    return nType == FRMC_NOTXT;
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
