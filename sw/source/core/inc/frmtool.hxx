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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FRMTOOL_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FRMTOOL_HXX

#include "swtypes.hxx"
#include "layfrm.hxx"
#include "frmatr.hxx"
#include "swcache.hxx"
#include <editeng/lrspitem.hxx>
#include <swfont.hxx>
#include <flyfrm.hxx>

class SwPageFrame;
class SwFlyFrame;
class SwContentFrame;
class SwRootFrame;
class SwDoc;
class SwAttrSet;
class SdrObject;
class SvxBrushItem;
class XFillStyleItem;
class XFillGradientItem;
class SdrMarkList;
class SwNodeIndex;
class OutputDevice;
class GraphicObject;
class GraphicAttr;
class SwPageDesc;
class SwFrameFormats;
class SwRegionRects;

#define FAR_AWAY (SAL_MAX_INT32 - 20000)  // initial position of a Fly
#define BROWSE_HEIGHT (56700L * 10L) // 10 Meters
#define GRFNUM_NO 0
#define GRFNUM_YES 1
#define GRFNUM_REPLACE 2

void AppendObjs( const SwFrameFormats *pTable, sal_uLong nIndex,
                       SwFrame *pFrame, SwPageFrame *pPage, SwDoc* doc );

// draw background with brush or graphics
// The 6th parameter indicates that the method should consider background
// transparency, saved in the color of the brush item.
void DrawGraphic(
    const SvxBrushItem *,
    OutputDevice *,
    const SwRect &rOrg,
    const SwRect &rOut,
    const sal_uInt8 nGrfNum = GRFNUM_NO,
    const bool bConsiderBackgroundTransparency = false );
bool DrawFillAttributes(
    const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
    const SwRect& rOriginalLayoutRect,
    const SwRegionRects& rPaintRegion,
    OutputDevice& rOut);

void paintGraphicUsingPrimitivesHelper(
        OutputDevice & rOutputDevice,
        GraphicObject const& rGraphicObj, GraphicAttr const& rGraphicAttr,
        SwRect const& rAlignedGrfArea);

// method to align rectangle.
// Created declaration here to avoid <extern> declarations
void SwAlignRect( SwRect &rRect, const SwViewShell *pSh, const vcl::RenderContext* pRenderContext );

// method to align graphic rectangle
// Created declaration here to avoid <extern> declarations
void SwAlignGrfRect( SwRect *pGrfRect, const OutputDevice &rOut );

/**
 * Paint border around a run of characters using frame painting code.
 *
 * @param[in]   rFont            font object of actual text, which specify the border
 * @param[in]   rPaintArea       rectangle area in which line portion takes place
 * @param[in]   bVerticalLayout  corresponding text frame verticality
 * @param[in]   bJoinWithPrev    leave border with which actual border joins to the previous portion
 * @param[in]   bJoinWithNext    leave border with which actual border joins to the next portion
**/
void PaintCharacterBorder(
    const SwFont& rFont, const SwRect& rPaintArea, const bool bVerticalLayout,
    const bool bJoinWithPrev, const bool bJoinWithNext );

// get Fly, if no List is given use the current shell
// Implementation in feshview.cxx
SwFlyFrame *GetFlyFromMarked( const SdrMarkList *pLst, SwViewShell *pSh );

SwFrame *SaveContent( SwLayoutFrame *pLay, SwFrame *pStart = nullptr );
void RestoreContent( SwFrame *pSav, SwLayoutFrame *pParent, SwFrame *pSibling, bool bGrow );

// Get ContentNodes, create ContentFrames, and add them to LayFrame.
void _InsertCnt( SwLayoutFrame *pLay, SwDoc *pDoc, sal_uLong nIndex,
                 bool bPages = false, sal_uLong nEndIndex = 0,
                 SwFrame *pPrv = nullptr );

// Creation of frames for a specific section (uses _InsertCnt)
void MakeFrames( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
                            const SwNodeIndex &rEndIdx );

// prevent creation of Flys in _InsertCnt, e.g. for table headlines
extern bool bDontCreateObjects;

// for FlyCnts, see SwFlyAtContentFrame::MakeAll()
extern bool bSetCompletePaintOnInvalidate;

// for table settings via keyboard
long CalcRowRstHeight( SwLayoutFrame *pRow );
long CalcHeightWithFlys( const SwFrame *pFrame );

SwPageFrame *InsertNewPage( SwPageDesc &rDesc, SwFrame *pUpper,
                          bool bOdd, bool bFirst, bool bInsertEmpty, bool bFootnote,
                          SwFrame *pSibling );

// connect Flys with page
void RegistFlys( SwPageFrame*, const SwLayoutFrame* );

// notification of Fly's background if needed
void Notify( SwFlyFrame *pFly, SwPageFrame *pOld, const SwRect &rOld,
             const SwRect* pOldRect = nullptr );

void Notify_Background( const SdrObject* pObj,
                        SwPageFrame* pPage,
                        const SwRect& rRect,
                        const PrepareHint eHint,
                        const bool bInva );

const SwFrame* GetVirtualUpper( const SwFrame* pFrame, const Point& rPos );

bool Is_Lower_Of( const SwFrame *pCurrFrame, const SdrObject* pObj );

// FIXME: EasyHack (refactoring): rename method and parameter name in all files
const SwFrame *FindKontext( const SwFrame *pFrame, sal_uInt16 nAdditionalKontextTyp );

bool IsFrameInSameKontext( const SwFrame *pInnerFrame, const SwFrame *pFrame );

const SwFrame * FindPage( const SwRect &rRect, const SwFrame *pPage );

// used by SwContentNode::GetFrame and SwFlyFrame::GetFrame
SwFrame* GetFrameOfModify( const SwRootFrame* pLayout,
                       SwModify const&,
                       sal_uInt16 const nFrameType,
                       const Point* = nullptr,
                       const SwPosition *pPos = nullptr,
                       const bool bCalcFrame = false );

// Should extra data (redline stroke, line numbers) be painted?
bool IsExtraData( const SwDoc *pDoc );

// #i11760# - method declaration <CalcContent(..)>
void CalcContent( SwLayoutFrame *pLay,
                bool bNoColl = false,
                bool bNoCalcFollow = false );

// Notify classes memorize the current sizes in their constructor and do
// the necessary notifications in their destructor if needed
class SwFrameNotify
{
protected:
    SwFrame *mpFrame;
    const SwRect maFrame;
    const SwRect maPrt;
    SwTwips mnFlyAnchorOfst;
    SwTwips mnFlyAnchorOfstNoWrap;
    bool     mbHadFollow;
    bool     mbInvaKeep;
    bool     mbValidSize;
    // #i49383#
    bool mbFrameDeleted;

public:
    SwFrameNotify( SwFrame *pFrame );
    ~SwFrameNotify();

    const SwRect &Frame() const { return maFrame; }
    void SetInvaKeep() { mbInvaKeep = true; }
};

class SwLayNotify : public SwFrameNotify
{
    bool bLowersComplete;

    SwLayoutFrame *GetLay() { return static_cast<SwLayoutFrame*>(mpFrame); }

public:
    SwLayNotify( SwLayoutFrame *pLayFrame );
    ~SwLayNotify();

    void SetLowersComplete( bool b ) { bLowersComplete = b; }
    bool IsLowersComplete()          { return bLowersComplete; }
};

class SwFlyNotify : public SwLayNotify
{
    SwPageFrame *pOldPage;
    const SwRect aFrameAndSpace;
    SwFlyFrame *GetFly() { return static_cast<SwFlyFrame*>(mpFrame); }

public:
    SwFlyNotify( SwFlyFrame *pFlyFrame );
    ~SwFlyNotify();
};

class SwContentNotify : public SwFrameNotify
{
private:
    // #i11859#
    bool    mbChkHeightOfLastLine;
    SwTwips mnHeightOfLastLine;

    // #i25029#
    bool        mbInvalidatePrevPrtArea;
    bool        mbBordersJoinedWithPrev;

    SwContentFrame *GetCnt();

public:
    SwContentNotify( SwContentFrame *pContentFrame );
    ~SwContentNotify();

    // #i25029#
    void SetInvalidatePrevPrtArea()
    {
        mbInvalidatePrevPrtArea = true;
    }

    void SetBordersJoinedWithPrev()
    {
        mbBordersJoinedWithPrev = true;
    }
};

// SwBorderAttrs encapsulates the calculation for margin attributes including
// border. The whole class is cached.

// WARNING! If more attributes should be cached also adjust the method
//          Modify::Modify!
class SwBorderAttrs : public SwCacheObj
{
    const SwAttrSet      &m_rAttrSet;
    const SvxULSpaceItem &m_rUL;
    // #i96772#
    SvxLRSpaceItem m_rLR;
    const SvxBoxItem     &m_rBox;
    const SvxShadowItem  &m_rShadow;
    const Size            m_aFrameSize;

    // Is it a frame that can have a margin without a border?
    bool m_bBorderDist  : 1;

    // the following bool values set the cached values to INVALID - until they
    // are calculated for the first time
    bool m_bTopLine     : 1;
    bool m_bBottomLine  : 1;
    bool m_bLeftLine    : 1;
    bool m_bRightLine   : 1;
    bool m_bTop         : 1;
    bool m_bBottom      : 1;
    bool m_bLine        : 1;

    bool m_bIsLine      : 1; // border on at least one side?

    bool m_bCacheGetLine        : 1; // cache GetTopLine(), GetBottomLine()?
    bool m_bCachedGetTopLine    : 1; // is GetTopLine() cached?
    bool m_bCachedGetBottomLine : 1; // is GetBottomLine() cached?
    // Booleans indicate that <m_bJoinedWithPrev> and <m_bJoinedWithNext> are
    // cached and valid.
    // Caching depends on value of <m_bCacheGetLine>.
    mutable bool m_bCachedJoinedWithPrev : 1;
    mutable bool m_bCachedJoinedWithNext : 1;
    // Booleans indicate that borders are joined with previous/next frame.
    bool m_bJoinedWithPrev :1;
    bool m_bJoinedWithNext :1;

    // The cached values (un-defined until calculated for the first time)
    sal_uInt16 m_nTopLine,
           m_nBottomLine,
           m_nLeftLine,
           m_nRightLine,
           m_nTop,
           m_nBottom,
           m_nGetTopLine,
           m_nGetBottomLine;

    // only calculate lines and shadow
    void _CalcTopLine();
    void _CalcBottomLine();
    void _CalcLeftLine();
    void _CalcRightLine();

    // lines + shadow + margin
    void _CalcTop();
    void _CalcBottom();

    void _IsLine();

    // #i25029# - If <_pPrevFrame> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    void _GetTopLine   ( const SwFrame& _rFrame,
                         const SwFrame* _pPrevFrame = nullptr );
    void _GetBottomLine( const SwFrame& _rFrame );

    // calculate cached values <m_bJoinedWithPrev> and <m_bJoinedWithNext>
    // #i25029# - If <_pPrevFrame> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    void _CalcJoinedWithPrev( const SwFrame& _rFrame,
                              const SwFrame* _pPrevFrame = nullptr );
    void _CalcJoinedWithNext( const SwFrame& _rFrame );

    // internal helper method for _CalcJoinedWithPrev and _CalcJoinedWithNext
    bool _JoinWithCmp( const SwFrame& _rCallerFrame,
                       const SwFrame& _rCmpFrame ) const;

    // Are the left and right line and the LRSpace equal?
    bool CmpLeftRight( const SwBorderAttrs &rCmpAttrs,
                       const SwFrame *pCaller,
                       const SwFrame *pCmp ) const;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwBorderAttrs)

    SwBorderAttrs( const SwModify *pOwner, const SwFrame *pConstructor );
    virtual ~SwBorderAttrs();

    inline const SwAttrSet      &GetAttrSet() const { return m_rAttrSet;  }
    inline const SvxULSpaceItem &GetULSpace() const { return m_rUL;       }
    inline const SvxBoxItem     &GetBox()     const { return m_rBox;      }
    inline const SvxShadowItem  &GetShadow()  const { return m_rShadow;   }

    inline sal_uInt16 CalcTopLine() const;
    inline sal_uInt16 CalcBottomLine() const;
    inline sal_uInt16 CalcLeftLine() const;
    inline sal_uInt16 CalcRightLine() const;
    inline sal_uInt16 CalcTop() const;
    inline sal_uInt16 CalcBottom() const;
           long CalcLeft( const SwFrame *pCaller ) const;
           long CalcRight( const SwFrame *pCaller ) const;

    inline bool IsLine() const;

    inline const Size &GetSize()     const { return m_aFrameSize; }

    inline bool IsBorderDist() const { return m_bBorderDist; }

    // Should upper (or lower) border be evaluated for this frame?
    // #i25029# - If <_pPrevFrame> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    inline sal_uInt16 GetTopLine   ( const SwFrame& _rFrame,
                                 const SwFrame* _pPrevFrame = nullptr ) const;
    inline sal_uInt16 GetBottomLine( const SwFrame& _rFrame ) const;
    inline void   SetGetCacheLine( bool bNew ) const;

    // Accessors for cached values <m_bJoinedWithPrev> and <m_bJoinedWithNext>
    // #i25029# - If <_pPrevFrame> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    bool JoinedWithPrev( const SwFrame& _rFrame,
                         const SwFrame* _pPrevFrame = nullptr ) const;
    bool JoinedWithNext( const SwFrame& _rFrame ) const;
};

class SwBorderAttrAccess : public SwCacheAccess
{
    const SwFrame *pConstructor;      //opt: for passing on to SwBorderAttrs

protected:
    virtual SwCacheObj *NewObj() override;

public:
    SwBorderAttrAccess( SwCache &rCache, const SwFrame *pOwner );

    SwBorderAttrs *Get();
};

// Iterator for draw objects of a page. The objects will be iterated sorted by
// their Z-order. Iterating is not cheap since for each operation the _whole_
// SortArray needs to be traversed.
class SwOrderIter
{
    const SwPageFrame *m_pPage;
    const SdrObject *m_pCurrent;
    const bool m_bFlysOnly;

public:
    SwOrderIter( const SwPageFrame *pPage, bool bFlysOnly = true );

    void             Current( const SdrObject *pNew ) { m_pCurrent = pNew; }
    const SdrObject *operator()() const { return m_pCurrent; }
    const SdrObject *Top();
    const SdrObject *Bottom();
    const SdrObject *Next();
    const SdrObject *Prev();
};

class StackHack
{
    static sal_uInt8 nCnt;
    static bool bLocked;

public:
    StackHack()
    {
        if ( ++StackHack::nCnt > 50 )
            StackHack::bLocked = true;
    }
    ~StackHack()
    {
        if ( --StackHack::nCnt < 5 )
            StackHack::bLocked = false;
    }

    static bool IsLocked()  { return StackHack::bLocked; }
    static sal_uInt8 Count()        { return StackHack::nCnt; }
};

// Should upper (or lower) border be evaluated for this frame?
// #i25029# - If <_pPrevFrame> is set, its value is taken for testing, if
// borders/shadow have to be joined with previous frame.
inline sal_uInt16 SwBorderAttrs::GetTopLine ( const SwFrame& _rFrame,
                                          const SwFrame* _pPrevFrame ) const
{
    if ( !m_bCachedGetTopLine || _pPrevFrame )
    {
        const_cast<SwBorderAttrs*>(this)->_GetTopLine( _rFrame, _pPrevFrame );
    }
    return m_nGetTopLine;
}
inline sal_uInt16 SwBorderAttrs::GetBottomLine( const SwFrame& _rFrame ) const
{
    if ( !m_bCachedGetBottomLine )
        const_cast<SwBorderAttrs*>(this)->_GetBottomLine( _rFrame );
    return m_nGetBottomLine;
}
inline void SwBorderAttrs::SetGetCacheLine( bool bNew ) const
{
    const_cast<SwBorderAttrs*>(this)->m_bCacheGetLine = bNew;
    const_cast<SwBorderAttrs*>(this)->m_bCachedGetBottomLine =
    const_cast<SwBorderAttrs*>(this)->m_bCachedGetTopLine = false;
    // invalidate cache for values <m_bJoinedWithPrev> and <m_bJoinedWithNext>
    m_bCachedJoinedWithPrev = false;
    m_bCachedJoinedWithNext = false;
}

inline sal_uInt16 SwBorderAttrs::CalcTopLine() const
{
    if ( m_bTopLine )
        const_cast<SwBorderAttrs*>(this)->_CalcTopLine();
    return m_nTopLine;
}
inline sal_uInt16 SwBorderAttrs::CalcBottomLine() const
{
    if ( m_bBottomLine )
        const_cast<SwBorderAttrs*>(this)->_CalcBottomLine();
    return m_nBottomLine;
}
inline sal_uInt16 SwBorderAttrs::CalcLeftLine() const
{
    if ( m_bLeftLine )
        const_cast<SwBorderAttrs*>(this)->_CalcLeftLine();
    return m_nLeftLine;
}
inline sal_uInt16 SwBorderAttrs::CalcRightLine() const
{
    if ( m_bRightLine )
        const_cast<SwBorderAttrs*>(this)->_CalcRightLine();
    return m_nRightLine;
}
inline sal_uInt16 SwBorderAttrs::CalcTop() const
{
    if ( m_bTop )
        const_cast<SwBorderAttrs*>(this)->_CalcTop();
    return m_nTop;
}
inline sal_uInt16 SwBorderAttrs::CalcBottom() const
{
    if ( m_bBottom )
        const_cast<SwBorderAttrs*>(this)->_CalcBottom();
    return m_nBottom;
}
inline bool SwBorderAttrs::IsLine() const
{
    if ( m_bLine )
        const_cast<SwBorderAttrs*>(this)->_IsLine();
    return m_bIsLine;
}

/** method to determine the spacing values of a frame

    #i28701#
    Values only provided for flow frames (table, section or text frames)
    Note: line spacing value is only determined for text frames
    #i102458#
    Add output parameter <obIsLineSpacingProportional>

    @param rFrame
    input parameter - frame, for which the spacing values are determined.

    @param onPrevLowerSpacing
    output parameter - lower spacing of the frame in SwTwips

    @param onPrevLineSpacing
    output parameter - line spacing of the frame in SwTwips

    @param obIsLineSpacingProportional
*/
void GetSpacingValuesOfFrame( const SwFrame& rFrame,
                            SwTwips& onLowerSpacing,
                            SwTwips& onLineSpacing,
                            bool& obIsLineSpacingProportional );

/** method to get the content of the table cell

    Content from any nested tables will be omitted.
    Note: line spacing value is only determined for text frames

    @param rCell_
    input parameter - the cell which should be searched for content.

    return
        pointer to the found content frame or 0
*/

const SwContentFrame* GetCellContent( const SwLayoutFrame& rCell_ );

/** helper class to check if a frame has been deleted during an operation
 *  WARNING! This should only be used as a last and desperate means to make the
 *  code robust.
 */

class SwDeletionChecker
{
private:
    const SwFrame* mpFrame;
    const SwModify* mpRegIn;

public:
    SwDeletionChecker( const SwFrame* pFrame )
            : mpFrame( pFrame ),
              mpRegIn( pFrame ? const_cast<SwFrame*>(pFrame)->GetRegisteredIn() : nullptr )
    {
    }

    /**
     *  return
     *    true if mpFrame != 0 and mpFrame is not client of pRegIn
     *    false otherwise
     */
    bool HasBeenDeleted();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
