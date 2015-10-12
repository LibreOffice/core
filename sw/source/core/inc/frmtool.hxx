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

class SwPageFrm;
class SwFlyFrm;
class SwContentFrm;
class SwRootFrm;
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

#define FAR_AWAY LONG_MAX - 20000  // initial position of a Fly
#define BROWSE_HEIGHT 56700L * 10L // 10 Meters
#define GRFNUM_NO 0
#define GRFNUM_YES 1
#define GRFNUM_REPLACE 2

void AppendObjs( const SwFrameFormats *pTable, sal_uLong nIndex,
                       SwFrm *pFrm, SwPageFrm *pPage, SwDoc* doc );

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
SwFlyFrm *GetFlyFromMarked( const SdrMarkList *pLst, SwViewShell *pSh );

SwFrm *SaveContent( SwLayoutFrm *pLay, SwFrm *pStart = NULL );
void RestoreContent( SwFrm *pSav, SwLayoutFrm *pParent, SwFrm *pSibling, bool bGrow );

// Get ContentNodes, create ContentFrms, and add them to LayFrm.
void _InsertCnt( SwLayoutFrm *pLay, SwDoc *pDoc, sal_uLong nIndex,
                 bool bPages = false, sal_uLong nEndIndex = 0,
                 SwFrm *pPrv = 0 );

// Creation of frames for a specific section (uses _InsertCnt)
void MakeFrms( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
                            const SwNodeIndex &rEndIdx );

// prevent creation of Flys in _InsertCnt, e.g. for table headlines
extern bool bDontCreateObjects;

// for FlyCnts, see SwFlyAtCntFrm::MakeAll()
extern bool bSetCompletePaintOnInvalidate;

// for table settings via keyboard
long CalcRowRstHeight( SwLayoutFrm *pRow );
long CalcHeightWithFlys( const SwFrm *pFrm );

SwPageFrm *InsertNewPage( SwPageDesc &rDesc, SwFrm *pUpper,
                          bool bOdd, bool bFirst, bool bInsertEmpty, bool bFootnote,
                          SwFrm *pSibling );

// connect Flys with page
void RegistFlys( SwPageFrm*, const SwLayoutFrm* );

// notification of Fly's background if needed
void Notify( SwFlyFrm *pFly, SwPageFrm *pOld, const SwRect &rOld,
             const SwRect* pOldRect = 0 );

void Notify_Background( const SdrObject* pObj,
                        SwPageFrm* pPage,
                        const SwRect& rRect,
                        const PrepareHint eHint,
                        const bool bInva );

const SwFrm* GetVirtualUpper( const SwFrm* pFrm, const Point& rPos );

bool Is_Lower_Of( const SwFrm *pCurrFrm, const SdrObject* pObj );

// FIXME: EasyHack (refactoring): rename method and parameter name in all files
const SwFrm *FindKontext( const SwFrm *pFrm, sal_uInt16 nAdditionalKontextTyp );

bool IsFrmInSameKontext( const SwFrm *pInnerFrm, const SwFrm *pFrm );

const SwFrm * FindPage( const SwRect &rRect, const SwFrm *pPage );

// used by SwContentNode::GetFrm and SwFlyFrm::GetFrm
SwFrm* GetFrmOfModify( const SwRootFrm* pLayout,
                       SwModify const&,
                       sal_uInt16 const nFrmType,
                       const Point* = 0,
                       const SwPosition *pPos = 0,
                       const bool bCalcFrm = false );

// Should extra data (redline stroke, line numbers) be painted?
bool IsExtraData( const SwDoc *pDoc );

// #i11760# - method declaration <CalcContent(..)>
void CalcContent( SwLayoutFrm *pLay,
                bool bNoColl = false,
                bool bNoCalcFollow = false );

// Notify classes memorize the current sizes in their constructor and do
// the necessary notifications in their destructor if needed
class SwFrmNotify
{
protected:
    SwFrm *pFrm;
    const SwRect aFrm;
    const SwRect aPrt;
    SwTwips mnFlyAnchorOfst;
    SwTwips mnFlyAnchorOfstNoWrap;
    bool     bHadFollow;
    bool     bInvaKeep;
    bool     bValidSize;
    // #i49383#
    bool mbFrmDeleted;

public:
    SwFrmNotify( SwFrm *pFrm );
    ~SwFrmNotify();

    const SwRect &Frm() const { return aFrm; }
    void SetInvaKeep() { bInvaKeep = true; }
};

class SwLayNotify : public SwFrmNotify
{
    bool bLowersComplete;

    SwLayoutFrm *GetLay() { return static_cast<SwLayoutFrm*>(pFrm); }

public:
    SwLayNotify( SwLayoutFrm *pLayFrm );
    ~SwLayNotify();

    void SetLowersComplete( bool b ) { bLowersComplete = b; }
    bool IsLowersComplete()          { return bLowersComplete; }
};

class SwFlyNotify : public SwLayNotify
{
    SwPageFrm *pOldPage;
    const SwRect aFrmAndSpace;
    SwFlyFrm *GetFly() { return static_cast<SwFlyFrm*>(pFrm); }

public:
    SwFlyNotify( SwFlyFrm *pFlyFrm );
    ~SwFlyNotify();
};

class SwContentNotify : public SwFrmNotify
{
private:
    // #i11859#
    bool    mbChkHeightOfLastLine;
    SwTwips mnHeightOfLastLine;

    // #i25029#
    bool        mbInvalidatePrevPrtArea;
    bool        mbBordersJoinedWithPrev;

    SwContentFrm *GetCnt();

public:
    SwContentNotify( SwContentFrm *pCntFrm );
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

    // #i25029# - If <_pPrevFrm> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    void _GetTopLine   ( const SwFrm& _rFrm,
                         const SwFrm* _pPrevFrm = 0L );
    void _GetBottomLine( const SwFrm& _rFrm );

    // calculate cached values <m_bJoinedWithPrev> and <m_bJoinedWithNext>
    // #i25029# - If <_pPrevFrm> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    void _CalcJoinedWithPrev( const SwFrm& _rFrm,
                              const SwFrm* _pPrevFrm = 0L );
    void _CalcJoinedWithNext( const SwFrm& _rFrm );

    // internal helper method for _CalcJoinedWithPrev and _CalcJoinedWithNext
    bool _JoinWithCmp( const SwFrm& _rCallerFrm,
                       const SwFrm& _rCmpFrm ) const;

    // Are the left and right line and the LRSpace equal?
    bool CmpLeftRight( const SwBorderAttrs &rCmpAttrs,
                       const SwFrm *pCaller,
                       const SwFrm *pCmp ) const;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwBorderAttrs)

    SwBorderAttrs( const SwModify *pOwner, const SwFrm *pConstructor );
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
           long CalcLeft( const SwFrm *pCaller ) const;
           long CalcRight( const SwFrm *pCaller ) const;

    inline bool IsLine() const;

    inline const Size &GetSize()     const { return m_aFrameSize; }

    inline bool IsBorderDist() const { return m_bBorderDist; }

    // Should upper (or lower) border be evaluated for this frame?
    // #i25029# - If <_pPrevFrm> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    inline sal_uInt16 GetTopLine   ( const SwFrm& _rFrm,
                                 const SwFrm* _pPrevFrm = 0L ) const;
    inline sal_uInt16 GetBottomLine( const SwFrm& _rFrm ) const;
    inline void   SetGetCacheLine( bool bNew ) const;

    // Accessors for cached values <m_bJoinedWithPrev> and <m_bJoinedWithNext>
    // #i25029# - If <_pPrevFrm> is set, its value is taken for testing, if
    // borders/shadow have to be joined with previous frame.
    bool JoinedWithPrev( const SwFrm& _rFrm,
                         const SwFrm* _pPrevFrm = 0L ) const;
    bool JoinedWithNext( const SwFrm& _rFrm ) const;
};

class SwBorderAttrAccess : public SwCacheAccess
{
    const SwFrm *pConstructor;      //opt: for passing on to SwBorderAttrs

protected:
    virtual SwCacheObj *NewObj() override;

public:
    SwBorderAttrAccess( SwCache &rCache, const SwFrm *pOwner );

    SwBorderAttrs *Get();
};

// Iterator for draw objects of a page. The objects will be iterated sorted by
// their Z-order. Iterating is not cheap since for each operation the _whole_
// SortArray needs to be traversed.
class SwOrderIter
{
    const SwPageFrm *pPage;
    const SdrObject *pCurrent;
    const bool bFlysOnly;

public:
    SwOrderIter( const SwPageFrm *pPage, bool bFlysOnly = true );

    void             Current( const SdrObject *pNew ) { pCurrent = pNew; }
    const SdrObject *operator()() const { return pCurrent; }
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
// #i25029# - If <_pPrevFrm> is set, its value is taken for testing, if
// borders/shadow have to be joined with previous frame.
inline sal_uInt16 SwBorderAttrs::GetTopLine ( const SwFrm& _rFrm,
                                          const SwFrm* _pPrevFrm ) const
{
    if ( !m_bCachedGetTopLine || _pPrevFrm )
    {
        const_cast<SwBorderAttrs*>(this)->_GetTopLine( _rFrm, _pPrevFrm );
    }
    return m_nGetTopLine;
}
inline sal_uInt16 SwBorderAttrs::GetBottomLine( const SwFrm& _rFrm ) const
{
    if ( !m_bCachedGetBottomLine )
        const_cast<SwBorderAttrs*>(this)->_GetBottomLine( _rFrm );
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

    @param rFrm
    input parameter - frame, for which the spacing values are determined.

    @param onPrevLowerSpacing
    output parameter - lower spacing of the frame in SwTwips

    @param onPrevLineSpacing
    output parameter - line spacing of the frame in SwTwips

    @param obIsLineSpacingProportional
*/
void GetSpacingValuesOfFrm( const SwFrm& rFrm,
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

const SwContentFrm* GetCellContent( const SwLayoutFrm& rCell_ );

/** helper class to check if a frame has been deleted during an operation
 *  WARNING! This should only be used as a last and desperate means to make the
 *  code robust.
 */

class SwDeletionChecker
{
private:
    const SwFrm* mpFrm;
    const SwModify* mpRegIn;

public:
    SwDeletionChecker( const SwFrm* pFrm )
            : mpFrm( pFrm ),
              mpRegIn( pFrm ? const_cast<SwFrm*>(pFrm)->GetRegisteredIn() : 0 )
    {
    }

    /**
     *  return
     *    true if mpFrm != 0 and mpFrm is not client of pRegIn
     *    false otherwise
     */
    bool HasBeenDeleted();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
