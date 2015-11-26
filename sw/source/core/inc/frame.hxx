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

class SwLayoutFrame;
class SwRootFrame;
class SwPageFrame;
class SwFlyFrame;
class SwSectionFrame;
class SwFootnoteFrame;
class SwFootnoteBossFrame;
class SwTabFrame;
class SwRowFrame;
class SwFlowFrame;
class SwContentFrame;
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
struct SwCursorMoveState;
class SwFormat;
class SwPrintData;
class SwSortedObjs;
class SwAnchoredObject;
typedef struct _xmlTextWriter *xmlTextWriterPtr;

// Each FrameType is represented here as a bit.
// The bits must be set in a way that it can be determined with masking of
// which kind of FrameType an instance is _and_ from what classes it was derived.
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

class SwFrame;
typedef long (SwFrame:: *SwFrameGet)() const;
typedef bool (SwFrame:: *SwFrameMax)( long );
typedef void (SwFrame:: *SwFrameMakePos)( const SwFrame*, const SwFrame*, bool );
typedef long (*SwOperator)( long, long );
typedef void (SwFrame:: *SwFrameSet)( long, long );

struct SwRectFnCollection
{
    SwRectGet     fnGetTop;
    SwRectGet     fnGetBottom;
    SwRectGet     fnGetLeft;
    SwRectGet     fnGetRight;
    SwRectGet     fnGetWidth;
    SwRectGet     fnGetHeight;
    SwRectPoint   fnGetPos;

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

    SwFrameGet      fnGetTopMargin;
    SwFrameGet      fnGetBottomMargin;
    SwFrameGet      fnGetLeftMargin;
    SwFrameGet      fnGetRightMargin;
    SwFrameSet      fnSetXMargins;
    SwFrameSet      fnSetYMargins;
    SwFrameGet      fnGetPrtTop;
    SwFrameGet      fnGetPrtBottom;
    SwFrameGet      fnGetPrtLeft;
    SwFrameGet      fnGetPrtRight;
    SwRectDist      fnTopDist;
    SwRectDist      fnBottomDist;
    SwFrameMax      fnSetLimit;
    SwRectMax       fnOverStep;

    SwRectSetPos    fnSetPos;
    SwFrameMakePos  fnMakePos;
    SwOperator      fnXDiff;
    SwOperator      fnYDiff;
    SwOperator      fnYInc;

    SwRectSetTwice  fnSetLeftAndWidth;
    SwRectSetTwice  fnSetTopAndHeight;
};

typedef SwRectFnCollection* SwRectFn;

extern SwRectFn fnRectHori, fnRectVert, fnRectB2T, fnRectVL2R, fnRectVertL2R;
#define SWRECTFN( pFrame )    bool bVert = pFrame->IsVertical(); \
                            bool bRev = pFrame->IsReverse(); \
                            bool bVertL2R = pFrame->IsVertLR(); \
                            SwRectFn fnRect = bVert ? \
                                ( bRev ? fnRectVL2R : ( bVertL2R ? fnRectVertL2R : fnRectVert ) ): \
                                ( bRev ? fnRectB2T : fnRectHori );
#define SWRECTFNX( pFrame )   bool bVertX = pFrame->IsVertical(); \
                            bool bRevX = pFrame->IsReverse(); \
                            bool bVertL2RX = pFrame->IsVertLR(); \
                            SwRectFn fnRectX = bVertX ? \
                                ( bRevX ? fnRectVL2R : ( bVertL2RX ? fnRectVertL2R : fnRectVert ) ): \
                                ( bRevX ? fnRectB2T : fnRectHori );
#define SWREFRESHFN( pFrame ) { if( bVert != pFrame->IsVertical() || \
                                  bRev  != pFrame->IsReverse() ) \
                                bVert = pFrame->IsVertical(); \
                                bRev = pFrame->IsReverse(); \
                                bVertL2R = pFrame->IsVertLR(); \
                                fnRect = bVert ? \
                                    ( bRev ? fnRectVL2R : ( bVertL2R ? fnRectVertL2R : fnRectVert ) ): \
                                    ( bRev ? fnRectB2T : fnRectHori ); }
#define SWRECTFN2( pFrame )   bool bVert = pFrame->IsVertical(); \
                bool bVertL2R = pFrame->IsVertLR(); \
                            bool bNeighb = pFrame->IsNeighbourFrame(); \
                            SwRectFn fnRect = bVert == bNeighb ? \
                                fnRectHori : ( bVertL2R ? fnRectVertL2R : fnRectVert );

#define POS_DIFF( aFrame1, aFrame2 ) \
            ( (aFrame1.*fnRect->fnGetTop)() != (aFrame2.*fnRect->fnGetTop)() || \
            (aFrame1.*fnRect->fnGetLeft)() != (aFrame2.*fnRect->fnGetLeft)() )

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
class SW_DLLPUBLIC SwFrame: public SwClient, public SfxBroadcaster
{
    // the hidden Frame
    friend class SwFlowFrame;
    friend class SwLayoutFrame;
    friend class SwLooping;

    // voids lower during creation of a column
    friend SwFrame *SaveContent( SwLayoutFrame *, SwFrame* pStart );
    friend void   RestoreContent( SwFrame *, SwLayoutFrame *, SwFrame *pSibling, bool bGrow );

    // for validating a mistakenly invalidated one in SwContentFrame::MakeAll
    friend void ValidateSz( SwFrame *pFrame );
    // implemented in text/txtftn.cxx, prevents Footnote oscillation
    friend void ValidateText( SwFrame *pFrame );

    friend void MakeNxt( SwFrame *pFrame, SwFrame *pNxt );

    // cache for (border) attributes
    static SwCache *mpCache;

    bool mbIfAccTableShouldDisposing;
    bool mbInDtor;

    // #i65250#
    // frame ID is now in general available - used for layout loop control
    static sal_uInt32 mnLastFrameId;
    const  sal_uInt32 mnFrameId;

    SwRootFrame   *mpRoot;
    SwLayoutFrame *mpUpper;
    SwFrame       *mpNext;
    SwFrame       *mpPrev;

    SwFrame *_FindNext();
    SwFrame *_FindPrev();

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

        @return SwContentFrame*
        pointer to the found next content frame. It's NULL, if none exists.
    */
    SwContentFrame* _FindNextCnt( const bool _bInSameFootnote = false );

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

        @return SwContentFrame*
        pointer to the found previous content frame. It's NULL, if none exists.
    */
    SwContentFrame* _FindPrevCnt( const bool _bInSameFootnote = false );

    void _UpdateAttrFrame( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 & );
    SwFrame* _GetIndNext();
    void SetDirFlags( bool bVert );

    const SwLayoutFrame* ImplGetNextLayoutLeaf( bool bFwd ) const;

    SwPageFrame* ImplFindPageFrame();

protected:
    SwSortedObjs* mpDrawObjs;    // draw objects, can be 0

    SwRect  maFrame;   // absolute position in document and size of the Frame
    SwRect  maPrt;   // position relatively to Frame and size of PrtArea

    sal_uInt16 mnFrameType;  //Who am I?

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
    // only partially. For ContentFrames a border (from Action) will exclusively
    // painted if <mbCompletePaint> is true.
    bool mbCompletePaint : 1;
    bool mbRetouche      : 1; // frame is responsible for retouching

    bool mbInfInvalid    : 1;  // InfoFlags are invalid
    bool mbInfBody       : 1;  // Frame is in document body
    bool mbInfTab        : 1;  // Frame is in a table
    bool mbInfFly        : 1;  // Frame is in a Fly
    bool mbInfFootnote        : 1;  // Frame is in a footnote
    bool mbInfSct        : 1;  // Frame is in a section
    bool mbColLocked     : 1;  // lock Grow/Shrink for column-wise section
                                  // or fly frames, will be set in Format
    bool m_isInDestroy : 1;
    bool mbForbidDelete : 1;

    void ColLock()      { mbColLocked = true; }
    void ColUnlock()    { mbColLocked = false; }

    virtual void DestroyImpl();
    virtual ~SwFrame();

    // Only used by SwRootFrame Ctor to get 'this' into mpRoot...
    void setRootFrame( SwRootFrame* pRoot ) { mpRoot = pRoot; }

    SwPageFrame *InsertPage( SwPageFrame *pSibling, bool bFootnote );
    void PrepareMake(vcl::RenderContext* pRenderContext);
    void OptPrepareMake();
    void MakePos();
    // Format next frame of table frame to assure keeping attributes.
    // In case of nested tables method <SwFrame::MakeAll()> is called to
    // avoid formatting of superior table frame.
    friend SwFrame* sw_FormatNextContentForKeep( SwTabFrame* pTabFrame );

    virtual void MakeAll(vcl::RenderContext* pRenderContext) = 0;
    // adjust frames of a page
    SwTwips AdjustNeighbourhood( SwTwips nDiff, bool bTst = false );

    // change only frame size not the size of PrtArea
    virtual SwTwips ShrinkFrame( SwTwips, bool bTst = false, bool bInfo = false ) = 0;
    virtual SwTwips GrowFrame  ( SwTwips, bool bTst = false, bool bInfo = false ) = 0;

    SwModify        *GetDep()       { return GetRegisteredInNonConst(); }
    const SwModify  *GetDep() const { return GetRegisteredIn(); }

    SwFrame( SwModify*, SwFrame* );

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
    virtual void  Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

    virtual const IDocumentDrawModelAccess& getIDocumentDrawModelAccess( );

public:
    virtual css::uno::Sequence< css::style::TabStop > GetTabStopInfo( SwTwips )
    {
        return css::uno::Sequence< css::style::TabStop >();
    }


    sal_uInt16 GetType() const { return mnFrameType; }

    static SwCache &GetCache()                { return *mpCache; }
    static SwCache *GetCachePtr()             { return mpCache;  }
    static void     SetCache( SwCache *pNew ) { mpCache = pNew;  }

    // change PrtArea size and FrameSize
    SwTwips Shrink( SwTwips, bool bTst = false, bool bInfo = false );
    SwTwips Grow  ( SwTwips, bool bTst = false, bool bInfo = false );

    // different methods for inserting in layout tree (for performance reasons)

    // insert before pBehind or at the end of the chain below mpUpper
    void InsertBefore( SwLayoutFrame* pParent, SwFrame* pBehind );
    // insert after pBefore or at the beginning of the chain below mpUpper
    void InsertBehind( SwLayoutFrame *pParent, SwFrame *pBefore );
    // insert before pBehind or at the end of the chain while considering
    // the siblings of pSct
    bool InsertGroupBefore( SwFrame* pParent, SwFrame* pWhere, SwFrame* pSct );
    void RemoveFromLayout();

    // For internal use only - who ignores this will be put in a sack and has
    // to stay there for two days
    // Does special treatment for _Get[Next|Prev]Leaf() (for tables).
    SwLayoutFrame *GetLeaf( MakePageType eMakePage, bool bFwd );
    SwLayoutFrame *GetNextLeaf   ( MakePageType eMakePage );
    SwLayoutFrame *GetNextFootnoteLeaf( MakePageType eMakePage );
    SwLayoutFrame *GetNextSctLeaf( MakePageType eMakePage );
    SwLayoutFrame *GetNextCellLeaf( MakePageType eMakePage );
    SwLayoutFrame *GetPrevLeaf   ( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    SwLayoutFrame *GetPrevFootnoteLeaf( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    SwLayoutFrame *GetPrevSctLeaf( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    SwLayoutFrame *GetPrevCellLeaf( MakePageType eMakeFootnote = MAKEPAGE_FTN );
    const SwLayoutFrame *GetLeaf ( MakePageType eMakePage, bool bFwd,
                                 const SwFrame *pAnch ) const;

    bool WrongPageDesc( SwPageFrame* pNew );

    //#i28701# - new methods to append/remove drawing objects
    void AppendDrawObj( SwAnchoredObject& _rNewObj );
    void RemoveDrawObj( SwAnchoredObject& _rToRemoveObj );

    // work with chain of FlyFrames
    void  AppendFly( SwFlyFrame *pNew );
    void  RemoveFly( SwFlyFrame *pToRemove );
    const SwSortedObjs *GetDrawObjs() const { return mpDrawObjs; }
          SwSortedObjs *GetDrawObjs()         { return mpDrawObjs; }
    // #i28701# - change purpose of method and adjust its name
    void InvalidateObjs( const bool _bInvaPosOnly,
                         const bool _bNoInvaOfAsCharAnchoredObjs = true );

    virtual void PaintBorder( const SwRect&, const SwPageFrame *pPage,
                              const SwBorderAttrs & ) const;
    void PaintBaBo( const SwRect&, const SwPageFrame *pPage = nullptr,
                    const bool bLowerBorder = false, const bool bOnlyTextBackground = false ) const;
    void PaintBackground( const SwRect&, const SwPageFrame *pPage,
                          const SwBorderAttrs &,
                          const bool bLowerMode = false,
                          const bool bLowerBorder = false,
                          const bool bOnlyTextBackground = false ) const;
    void PaintBorderLine( const SwRect&, const SwRect&, const SwPageFrame*,
                          const Color *pColor,
                          const editeng::SvxBorderStyle = css::table::BorderLineStyle::SOLID ) const;

    drawinglayer::processor2d::BaseProcessor2D * CreateProcessor2D( ) const;
    void ProcessPrimitives( const drawinglayer::primitive2d::Primitive2DSequence& rSequence ) const;

    // FIXME: EasyHack (refactoring): rename method name in all files
    // retouch, not in the area of the given Rect!
    void Retouche( const SwPageFrame *pPage, const SwRect &rRect ) const;

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
    const SwRowFrame* IsInSplitTableRow() const;

    // If frame is inside a follow flow row, this function returns
    // the corresponding row frame master table
    const SwRowFrame* IsInFollowFlowRow() const;

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
    Point   GetFrameAnchorPos( bool bIgnoreFlysAnchoredAtThisFrame ) const;

    /** determine, if frame is moveable in given environment

        method replaced 'old' method <bool IsMoveable() const>.
        Determines, if frame is moveable in given environment. if no environment
        is given (parameter _pLayoutFrame == 0L), the movability in the actual
        environment (<this->GetUpper()) is checked.

        @param _pLayoutFrame
        input parameter - given environment (layout frame), in which the movability
        will be checked. If not set ( == 0L ), actual environment is taken.

        @return boolean, indicating, if frame is moveable in given environment
    */
    bool IsMoveable( const SwLayoutFrame* _pLayoutFrame = nullptr ) const;

    // Is it permitted for the (Text)Frame to add a footnote in the current
    // environment (not e.g. for repeating table headlines)
    bool IsFootnoteAllowed() const;

    virtual void  Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr );

    virtual void CheckDirection( bool bVert );

    void ReinitializeFrameSizeAttrFlags();

    const SwAttrSet *GetAttrSet() const;

    inline bool HasFixSize() const { return mbFixSize; }

    // check all pages (starting from the given) and correct them if needed
    static void CheckPageDescs( SwPageFrame *pStart, bool bNotifyFields = true, SwPageFrame** ppPrev = nullptr);

    // might return 0, with and without const
    SwFrame               *GetNext()  { return mpNext; }
    SwFrame               *GetPrev()  { return mpPrev; }
    SwLayoutFrame         *GetUpper() { return mpUpper; }
    SwRootFrame           *getRootFrame(){ return mpRoot; }
    SwPageFrame           *FindPageFrame() { return IsPageFrame() ? reinterpret_cast<SwPageFrame*>(this) : ImplFindPageFrame(); }
    SwFrame               *FindColFrame();
    SwRowFrame            *FindRowFrame();
    SwFootnoteBossFrame   *FindFootnoteBossFrame( bool bFootnotes = false );
    SwTabFrame            *ImplFindTabFrame();
    SwFootnoteFrame       *ImplFindFootnoteFrame();
    SwFlyFrame            *ImplFindFlyFrame();
    SwSectionFrame        *ImplFindSctFrame();
    SwFrame               *FindFooterOrHeader();
    SwFrame               *GetLower();
    const SwFrame         *GetNext()  const { return mpNext; }
    const SwFrame         *GetPrev()  const { return mpPrev; }
    const SwLayoutFrame   *GetUpper() const { return mpUpper; }
    const SwRootFrame     *getRootFrame()   const { return mpRoot; }
    inline SwTabFrame     *FindTabFrame();
    inline SwFootnoteFrame     *FindFootnoteFrame();
    inline SwFlyFrame     *FindFlyFrame();
    inline SwSectionFrame *FindSctFrame();
    inline SwFrame        *FindNext();
    // #i27138# - add parameter <_bInSameFootnote>
    SwContentFrame* FindNextCnt( const bool _bInSameFootnote = false );
    inline SwFrame        *FindPrev();
    inline const SwPageFrame *FindPageFrame() const;
    inline const SwFootnoteBossFrame *FindFootnoteBossFrame( bool bFootnote = false ) const;
    inline const SwFrame     *FindColFrame() const;
    inline const SwFrame     *FindFooterOrHeader() const;
    inline const SwTabFrame  *FindTabFrame() const;
    inline const SwFootnoteFrame  *FindFootnoteFrame() const;
    inline const SwFlyFrame  *FindFlyFrame() const;
    inline const SwSectionFrame *FindSctFrame() const;
    inline const SwFrame     *FindNext() const;
    // #i27138# - add parameter <_bInSameFootnote>
    const SwContentFrame* FindNextCnt( const bool _bInSameFootnote = false ) const;
    inline const SwFrame     *FindPrev() const;
           const SwFrame     *GetLower()  const;

    SwContentFrame* FindPrevCnt( const bool _bInSameFootnote = false );

    const SwContentFrame* FindPrevCnt( const bool _bInSameFootnote = false ) const;

    // #i79774#
    SwFrame* _GetIndPrev() const;
    SwFrame* GetIndPrev() const
        { return ( mpPrev || !IsInSct() ) ? mpPrev : _GetIndPrev(); }

    SwFrame* GetIndNext()
        { return ( mpNext || !IsInSct() ) ? mpNext : _GetIndNext(); }
    const SwFrame* GetIndNext() const { return const_cast<SwFrame*>(this)->GetIndNext(); }

    sal_uInt16 GetPhyPageNum() const;   // page number without offset
    sal_uInt16 GetVirtPageNum() const;  // page number with offset
    bool OnRightPage() const { return 0 != GetPhyPageNum() % 2; };
    bool WannaRightPage() const;
    bool OnFirstPage() const;

    inline const  SwLayoutFrame *GetPrevLayoutLeaf() const;
    inline const  SwLayoutFrame *GetNextLayoutLeaf() const;
    inline SwLayoutFrame *GetPrevLayoutLeaf();
    inline SwLayoutFrame *GetNextLayoutLeaf();

    virtual void Calc(vcl::RenderContext* pRenderContext) const; // here might be "formatted"
    inline void OptCalc() const;    // here we assume (for optimization) that
                                    // the predecessors are already formatted

    Point   GetRelPos() const;
    const  SwRect &Frame() const { return maFrame; }
    const  SwRect &Prt() const { return maPrt; }

    // PaintArea is the area where content might be displayed.
    // The margin of a page or the space between columns belongs to it.
    const SwRect PaintArea() const;
    // UnionFrame is the union of Frame- and PrtArea, normally identical
    // to the FrameArea except in case of negative Prt margins.
    const SwRect UnionFrame( bool bBorder = false ) const;

    // HACK: Here we exceptionally allow direct access to members.
    // This should not delude into changing those value randomly; it is the
    // only option to circumvent compiler problems (same method with public
    // and protected).
    SwRect &Frame() { return maFrame; }
    SwRect &Prt() { return maPrt; }

    virtual Size ChgSize( const Size& aNewSize );

    virtual void Cut() = 0;
    //Add a method to change the acc table dispose state.
    void SetAccTableDispose(bool bDispose) { mbIfAccTableShouldDisposing = bDispose;}
    virtual void Paste( SwFrame* pParent, SwFrame* pSibling = nullptr ) = 0;

    void ValidateLineNum() { mbValidLineNum = true; }

    bool GetValidPosFlag()    const { return mbValidPos; }
    bool GetValidPrtAreaFlag()const { return mbValidPrtArea; }
    bool GetValidSizeFlag()   const { return mbValidSize; }
    bool GetValidLineNumFlag()const { return mbValidLineNum; }
    bool IsValid() const { return mbValidPos && mbValidSize && mbValidPrtArea; }

    // Only invalidate Frame
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

    void InvalidatePage( const SwPageFrame *pPage = nullptr ) const;

    virtual bool    FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    virtual bool    GetCursorOfst( SwPosition *, Point&,
                                 SwCursorMoveState* = nullptr, bool bTestBackground = false ) const;
    virtual bool    GetCharRect( SwRect &, const SwPosition&,
                                 SwCursorMoveState* = nullptr ) const;
    virtual void Paint( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = nullptr ) const;

    // HACK: shortcut between frame and formatting
    // It's your own fault if you cast void* incorrectly! In any case check
    // the void* for 0.
    virtual bool Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = nullptr, bool bNotify = true );

    // true if it is the correct class, false otherwise
    inline bool IsLayoutFrame() const;
    inline bool IsRootFrame() const;
    inline bool IsPageFrame() const;
    inline bool IsColumnFrame() const;
    inline bool IsFootnoteBossFrame() const;  // footnote bosses might be PageFrames or ColumnFrames
    inline bool IsHeaderFrame() const;
    inline bool IsFooterFrame() const;
    inline bool IsFootnoteContFrame() const;
    inline bool IsFootnoteFrame() const;
    inline bool IsBodyFrame() const;
    inline bool IsColBodyFrame() const;  // implemented in layfrm.hxx, BodyFrame above ColumnFrame
    inline bool IsPageBodyFrame() const; // implemented in layfrm.hxx, BodyFrame above PageFrame
    inline bool IsFlyFrame() const;
    inline bool IsSctFrame() const;
    inline bool IsTabFrame() const;
    inline bool IsRowFrame() const;
    inline bool IsCellFrame() const;
    inline bool IsContentFrame() const;
    inline bool IsTextFrame() const;
    inline bool IsNoTextFrame() const;
    // Frames where its PrtArea depends on their neighbors and that are
    // positioned in the content flow
    inline bool IsFlowFrame() const;
    // Frames that are capable of retouching or that might need to retouch behind
    // themselves
    inline bool IsRetoucheFrame() const;
    inline bool IsAccessibleFrame() const;

    void PrepareCursor();                 // CursorShell is allowed to call this

    // Is the Frame (or the section containing it) protected? Same for Fly in
    // Fly in ... and footnotes
    bool IsProtected() const;

    bool IsColLocked()  const { return mbColLocked; }
    bool IsDeleteForbidden()  const { return mbForbidDelete; }

    /// this is the only way to delete a SwFrame instance
    static void DestroyFrame(SwFrame *const pFrame);

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
    void MakeBelowPos( const SwFrame*, const SwFrame*, bool );
    void MakeUpperPos( const SwFrame*, const SwFrame*, bool );
    void MakeLeftPos( const SwFrame*, const SwFrame*, bool );
    void MakeRightPos( const SwFrame*, const SwFrame*, bool );
    inline bool IsNeighbourFrame() const
        { return (GetType() & FRM_NEIGHBOUR) != 0; }

    // #i65250#
    inline sal_uInt32 GetFrameId() const { return mnFrameId; }

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
    virtual void dumpAsXml(xmlTextWriterPtr writer = nullptr) const;
    void dumpInfosAsXml(xmlTextWriterPtr writer) const;
    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const;
    void dumpChildrenAsXml(xmlTextWriterPtr writer) const;
    bool IsCollapse() const;
};

inline bool SwFrame::IsInDocBody() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfBody;
}
inline bool SwFrame::IsInFootnote() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfFootnote;
}
inline bool SwFrame::IsInTab() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfTab;
}
inline bool SwFrame::IsInFly() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfFly;
}
inline bool SwFrame::IsInSct() const
{
    if ( mbInfInvalid )
        const_cast<SwFrame*>(this)->SetInfFlags();
    return mbInfSct;
}
bool SwFrame::IsVertical() const
{
    if( mbInvalidVert )
        const_cast<SwFrame*>(this)->SetDirFlags( true );
    return mbVertical;
}
inline bool SwFrame::IsVertLR() const
{
    return mbVertLR;
}
inline bool SwFrame::IsRightToLeft() const
{
    if( mbInvalidR2L )
        const_cast<SwFrame*>(this)->SetDirFlags( false );
    return mbRightToLeft;
}

inline void SwFrame::SetCompletePaint() const
{
    const_cast<SwFrame*>(this)->mbCompletePaint = true;
}
inline void SwFrame::ResetCompletePaint() const
{
    const_cast<SwFrame*>(this)->mbCompletePaint = false;
}

inline void SwFrame::SetRetouche() const
{
    const_cast<SwFrame*>(this)->mbRetouche = true;
}
inline void SwFrame::ResetRetouche() const
{
    const_cast<SwFrame*>(this)->mbRetouche = false;
}

inline SwLayoutFrame *SwFrame::GetNextLayoutLeaf()
{
    return const_cast<SwLayoutFrame*>(static_cast<const SwFrame*>(this)->GetNextLayoutLeaf());
}
inline SwLayoutFrame *SwFrame::GetPrevLayoutLeaf()
{
    return const_cast<SwLayoutFrame*>(static_cast<const SwFrame*>(this)->GetPrevLayoutLeaf());
}
inline const SwLayoutFrame *SwFrame::GetNextLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( true );
}
inline const SwLayoutFrame *SwFrame::GetPrevLayoutLeaf() const
{
    return ImplGetNextLayoutLeaf( false );
}

inline void SwFrame::InvalidateSize()
{
    if ( mbValidSize )
        ImplInvalidateSize();
}
inline void SwFrame::InvalidatePrt()
{
    if ( mbValidPrtArea )
        ImplInvalidatePrt();
}
inline void SwFrame::InvalidatePos()
{
    if ( mbValidPos )
        ImplInvalidatePos();
}
inline void SwFrame::InvalidateLineNum()
{
    if ( mbValidLineNum )
        ImplInvalidateLineNum();
}
inline void SwFrame::InvalidateAll()
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
inline void SwFrame::InvalidateNextPos( bool bNoFootnote )
{
    if ( mpNext && !mpNext->IsSctFrame() )
        mpNext->InvalidatePos();
    else
        ImplInvalidateNextPos( bNoFootnote );
}

inline void SwFrame::OptCalc() const
{
    if ( !mbValidPos || !mbValidPrtArea || !mbValidSize )
        const_cast<SwFrame*>(this)->OptPrepareMake();
}
inline const SwPageFrame *SwFrame::FindPageFrame() const
{
    return const_cast<SwFrame*>(this)->FindPageFrame();
}
inline const SwFrame *SwFrame::FindColFrame() const
{
    return const_cast<SwFrame*>(this)->FindColFrame();
}
inline const SwFrame *SwFrame::FindFooterOrHeader() const
{
    return const_cast<SwFrame*>(this)->FindFooterOrHeader();
}
inline SwTabFrame *SwFrame::FindTabFrame()
{
    return IsInTab() ? ImplFindTabFrame() : nullptr;
}
inline const SwFootnoteBossFrame *SwFrame::FindFootnoteBossFrame( bool bFootnote ) const
{
    return const_cast<SwFrame*>(this)->FindFootnoteBossFrame( bFootnote );
}
inline SwFootnoteFrame *SwFrame::FindFootnoteFrame()
{
    return IsInFootnote() ? ImplFindFootnoteFrame() : nullptr;
}
inline SwFlyFrame *SwFrame::FindFlyFrame()
{
    return IsInFly() ? ImplFindFlyFrame() : nullptr;
}
inline SwSectionFrame *SwFrame::FindSctFrame()
{
    return IsInSct() ? ImplFindSctFrame() : nullptr;
}

inline const SwTabFrame *SwFrame::FindTabFrame() const
{
    return IsInTab() ? const_cast<SwFrame*>(this)->ImplFindTabFrame() : nullptr;
}
inline const SwFootnoteFrame *SwFrame::FindFootnoteFrame() const
{
    return IsInFootnote() ? const_cast<SwFrame*>(this)->ImplFindFootnoteFrame() : nullptr;
}
inline const SwFlyFrame *SwFrame::FindFlyFrame() const
{
    return IsInFly() ? const_cast<SwFrame*>(this)->ImplFindFlyFrame() : nullptr;
}
inline const SwSectionFrame *SwFrame::FindSctFrame() const
{
    return IsInSct() ? const_cast<SwFrame*>(this)->ImplFindSctFrame() : nullptr;
}
inline SwFrame *SwFrame::FindNext()
{
    if ( mpNext )
        return mpNext;
    else
        return _FindNext();
}
inline const SwFrame *SwFrame::FindNext() const
{
    if ( mpNext )
        return mpNext;
    else
        return const_cast<SwFrame*>(this)->_FindNext();
}
inline SwFrame *SwFrame::FindPrev()
{
    if ( mpPrev && !mpPrev->IsSctFrame() )
        return mpPrev;
    else
        return _FindPrev();
}
inline const SwFrame *SwFrame::FindPrev() const
{
    if ( mpPrev && !mpPrev->IsSctFrame() )
        return mpPrev;
    else
        return const_cast<SwFrame*>(this)->_FindPrev();
}

inline bool SwFrame::IsLayoutFrame() const
{
    return (GetType() & FRM_LAYOUT) != 0;
}
inline bool SwFrame::IsRootFrame() const
{
    return mnFrameType == FRM_ROOT;
}
inline bool SwFrame::IsPageFrame() const
{
    return mnFrameType == FRM_PAGE;
}
inline bool SwFrame::IsColumnFrame() const
{
    return mnFrameType == FRM_COLUMN;
}
inline bool SwFrame::IsFootnoteBossFrame() const
{
    return (GetType() & FRM_FTNBOSS) != 0;
}
inline bool SwFrame::IsHeaderFrame() const
{
    return mnFrameType == FRM_HEADER;
}
inline bool SwFrame::IsFooterFrame() const
{
    return mnFrameType == FRM_FOOTER;
}
inline bool SwFrame::IsFootnoteContFrame() const
{
    return mnFrameType == FRM_FTNCONT;
}
inline bool SwFrame::IsFootnoteFrame() const
{
    return mnFrameType == FRM_FTN;
}
inline bool SwFrame::IsBodyFrame() const
{
    return mnFrameType == FRM_BODY;
}
inline bool SwFrame::IsFlyFrame() const
{
    return mnFrameType == FRM_FLY;
}
inline bool SwFrame::IsSctFrame() const
{
    return mnFrameType == FRM_SECTION;
}
inline bool SwFrame::IsTabFrame() const
{
    return mnFrameType == FRM_TAB;
}
inline bool SwFrame::IsRowFrame() const
{
    return mnFrameType == FRM_ROW;
}
inline bool SwFrame::IsCellFrame() const
{
    return mnFrameType == FRM_CELL;
}
inline bool SwFrame::IsContentFrame() const
{
    return (GetType() & FRM_CNTNT) != 0;
}
inline bool SwFrame::IsTextFrame() const
{
    return mnFrameType == FRM_TXT;
}
inline bool SwFrame::IsNoTextFrame() const
{
    return mnFrameType == FRM_NOTXT;
}
inline bool SwFrame::IsFlowFrame() const
{
    return (GetType() & (FRM_CNTNT|FRM_TAB|FRM_SECTION)) != 0;
}
inline bool SwFrame::IsRetoucheFrame() const
{
    return (GetType() & (FRM_CNTNT|FRM_TAB|FRM_SECTION|FRM_FTN)) != 0;
}
inline bool SwFrame::IsAccessibleFrame() const
{
    return (GetType() & FRM_ACCESSIBLE) != 0;
}

//use this to protect a SwFrame for a given scope from getting deleted
class SwFrameDeleteGuard
{
private:
    SwFrame *m_pFrame;
    bool m_bOldDeleteAllowed;
public:
    //Flag pFrame for SwFrameDeleteGuard lifetime that we shouldn't delete
    //it in e.g. SwSectionFrame::MergeNext etc because we will need it
    //again after the SwFrameDeleteGuard dtor
    explicit SwFrameDeleteGuard(SwFrame* pFrame)
        : m_pFrame(pFrame)
    {
        m_bOldDeleteAllowed = m_pFrame && !m_pFrame->IsDeleteForbidden();
        if (m_bOldDeleteAllowed)
            m_pFrame->ForbidDelete();
    }

    ~SwFrameDeleteGuard()
    {
        if (m_bOldDeleteAllowed)
            m_pFrame->AllowDelete();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
