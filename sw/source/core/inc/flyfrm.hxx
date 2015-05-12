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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FLYFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FLYFRM_HXX

#include "layfrm.hxx"
#include <list>
#include "frmfmt.hxx"

class SwPageFrm;
class SwFormatFrmSize;
struct SwCrsrMoveState;
class SwBorderAttrs;
class SwVirtFlyDrawObj;
class SwFrameFormats;
class SwAttrSetChg;
namespace tools { class PolyPolygon; }
class SwFlyDrawContact;
class SwFormat;

#include <anchoredobject.hxx>

/** search an anchor for paragraph bound frames starting from pOldAnch

    needed for dragging of objects bound to a paragraph for showing an anchor
    indicator as well as for changing the anchor.

    implemented in layout/flycnt.cxx
 */
const SwContentFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const bool bBody = false );

/** calculate rectangle in that the object can be moved or rather be resized */
bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, bool bMove = true );

/** general base class for all free-flowing frames

    #i26791# - inherit also from <SwAnchoredFlyFrm>
*/
class SwFlyFrm : public SwLayoutFrm, public SwAnchoredObject
{
    // is allowed to lock, implemented in frmtool.cxx
    friend void AppendObjs   ( const SwFrameFormats *, sal_uLong, SwFrm *, SwPageFrm *, SwDoc* );
    friend void Notify( SwFlyFrm *, SwPageFrm *pOld, const SwRect &rOld,
                        const SwRect* pOldPrt );

    void InitDrawObj( bool bNotify ); // these to methods are called in the
    void FinitDrawObj();                  // constructors

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    using SwLayoutFrm::CalcRel;

    sal_uInt32 _GetOrdNumForNewRef( const SwFlyDrawContact* );
    SwVirtFlyDrawObj* CreateNewRef( SwFlyDrawContact* );

protected:
    // Predecessor/Successor for chaining with text flow
    SwFlyFrm *pPrevLink, *pNextLink;

private:
    // It must be possible to block Content-bound flys so that they will be not
    // formatted; in this case MakeAll() returns immediately. This is necessary
    // for page changes during formattting. In addition, it is needed during
    // the constructor call of the root object since otherwise the anchor will
    // be formatted before the root is anchored correctly to a shell and
    // because too much would be formatted as a result.
    bool bLocked :1;
    // true if the background of NotifyDTor needs to be notified at the end
    // of a MakeAll() call.
    bool bNotifyBack :1;

protected:
    // Pos, PrtArea or SSize have been invalidated - they will be evaluated
    // again immediately because they have to be valid _at all time_.
    // The invalidation is tracked here so that LayAction knows about it and
    // can handle it properly. Exceptions prove the rule.
    bool bInvalid :1;

    // true if the proposed height of an attribute is a minimal height
    // (this means that the frame can grow higher if needed)
    bool bMinHeight :1;
    // true if the fly frame could not format position/size based on its
    // attributes, e.g. because there was not enough space.
    bool bHeightClipped :1;
    bool bWidthClipped :1;
    // If true then call only the format after adjusting the width (CheckClip);
    // but the width will not be re-evaluated based on the attributes.
    bool bFormatHeightOnly :1;

    bool bInCnt :1;        ///< FLY_AS_CHAR, anchored as character
    bool bAtCnt :1;        ///< FLY_AT_PARA, anchored at paragraph
    bool bLayout :1;       ///< FLY_AT_PAGE, FLY_AT_FLY, at page or at frame
    bool bAutoPosition :1; ///< FLY_AT_CHAR, anchored at character

    bool bNoShrink :1;     ///< temporary forbid shrinking to avoid loops
    // If true, the content of the fly frame will not be deleted when it
    // is moved to an invisible layer.
    bool bLockDeleteContent :1;

    friend class SwNoTextFrm; // is allowed to call NotifyBackground

    Point m_aContentPos;        // content area's position relatively to Frm
    bool m_bValidContentPos;

    virtual void Format( const SwBorderAttrs *pAttrs = 0 ) SAL_OVERRIDE;
    void MakePrtArea( const SwBorderAttrs &rAttrs );
    void MakeContentPos( const SwBorderAttrs &rAttrs );

    void Lock()         { bLocked = true; }
    void Unlock()       { bLocked = false; }

    void SetMinHeight()  { bMinHeight = true; }
    void ResetMinHeight(){ bMinHeight = false; }

    Size CalcRel( const SwFormatFrmSize &rSz ) const;
    SwTwips CalcAutoWidth() const;

    SwFlyFrm( SwFlyFrameFormat*, SwFrm*, SwFrm *pAnchor );

    virtual void DestroyImpl() SAL_OVERRIDE;
    virtual ~SwFlyFrm();

    /** method to assure that anchored object is registered at the correct
        page frame

        #i28701#
    */
    virtual void RegisterAtCorrectPage() SAL_OVERRIDE;

    virtual bool _SetObjTop( const SwTwips _nTop ) SAL_OVERRIDE;
    virtual bool _SetObjLeft( const SwTwips _nLeft ) SAL_OVERRIDE;

    virtual const SwRect GetObjBoundRect() const SAL_OVERRIDE;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;

    virtual const IDocumentDrawModelAccess* getIDocumentDrawModelAccess( ) SAL_OVERRIDE;

    SwTwips CalcContentHeight(const SwBorderAttrs *pAttrs, const SwTwips nMinHeight, const SwTwips nUL);

public:
    // #i26791#
    TYPEINFO_OVERRIDE();

    // get client information
    virtual bool GetInfo( SfxPoolItem& ) const SAL_OVERRIDE;
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const SAL_OVERRIDE;
    virtual Size ChgSize( const Size& aNewSize ) SAL_OVERRIDE;
    virtual bool GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0, bool bTestBackground = false ) const SAL_OVERRIDE;

    virtual void CheckDirection( bool bVert ) SAL_OVERRIDE;
    virtual void Cut() SAL_OVERRIDE;
#ifdef DBG_UTIL
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) SAL_OVERRIDE;
#endif

    SwTwips _Shrink( SwTwips, bool bTst );
    SwTwips _Grow  ( SwTwips, bool bTst );
    void    _Invalidate( SwPageFrm *pPage = 0 );

    bool FrmSizeChg( const SwFormatFrmSize & );

    SwFlyFrm *GetPrevLink() const { return pPrevLink; }
    SwFlyFrm *GetNextLink() const { return pNextLink; }

    static void ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );
    static void UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );

    SwFlyFrm *FindChainNeighbour( SwFrameFormat &rFormat, SwFrm *pAnch = 0 );

    // #i26791#
    const SwVirtFlyDrawObj* GetVirtDrawObj() const;
    SwVirtFlyDrawObj *GetVirtDrawObj();
    void NotifyDrawObj();

    void ChgRelPos( const Point &rAbsPos );
    bool IsInvalid() const { return bInvalid; }
    void Invalidate() const { const_cast<SwFlyFrm*>(this)->bInvalid = true; }
    void Validate() const { const_cast<SwFlyFrm*>(this)->bInvalid = false; }

    bool IsMinHeight()  const { return bMinHeight; }
    bool IsLocked()     const { return bLocked; }
    bool IsAutoPos()    const { return bAutoPosition; }
    bool IsFlyInCntFrm() const { return bInCnt; }
    bool IsFlyFreeFrm() const { return bAtCnt || bLayout; }
    bool IsFlyLayFrm() const { return bLayout; }
    bool IsFlyAtCntFrm() const { return bAtCnt; }

    bool IsNotifyBack() const { return bNotifyBack; }
    void SetNotifyBack()      { bNotifyBack = true; }
    void ResetNotifyBack()    { bNotifyBack = false; }
    bool IsNoShrink()   const { return bNoShrink; }
    void SetNoShrink( bool bNew ) { bNoShrink = bNew; }
    bool IsLockDeleteContent()  const { return bLockDeleteContent; }
    void SetLockDeleteContent( bool bNew ) { bLockDeleteContent = bNew; }

    bool IsClipped()        const   { return bHeightClipped || bWidthClipped; }
    bool IsHeightClipped()  const   { return bHeightClipped; }
    bool IsWidthClipped()   const   { return bWidthClipped;  }

    bool IsLowerOf( const SwLayoutFrm* pUpper ) const;
    inline bool IsUpperOf( const SwFlyFrm& _rLower ) const
    {
        return _rLower.IsLowerOf( this );
    }

    SwFrm *FindLastLower();

    // #i13147# - add parameter <_bForPaint> to avoid load of
    // the graphic during paint. Default value: false
    bool GetContour( tools::PolyPolygon&   rContour,
                     const bool _bForPaint = false ) const;

    // Paint on this shell (consider Preview, print flag, etc. recursively)?
    static bool IsPaint( SdrObject *pObj, const SwViewShell *pSh );

    /** SwFlyFrm::IsBackgroundTransparent

        determines if background of fly frame has to be drawn transparently

        definition found in /core/layout/paintfrm.cxx

        @return true, if background color is transparent or a existing background
        graphic is transparent.
    */
    bool IsBackgroundTransparent() const;

    void Chain( SwFrm* _pAnchor );
    void Unchain();
    void InsertCnt();
    void DeleteCnt();
    void InsertColumns();

    // #i26791# - pure virtual methods of base class <SwAnchoredObject>
    virtual void MakeObjPos() SAL_OVERRIDE;
    virtual void InvalidateObjPos() SAL_OVERRIDE;

    virtual SwFrameFormat& GetFrameFormat() SAL_OVERRIDE;
    virtual const SwFrameFormat& GetFrameFormat() const SAL_OVERRIDE;

    virtual const SwRect GetObjRect() const SAL_OVERRIDE;

    /** method to determine if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwAnchoredObject::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame is locked resp. col-locked.
    */
    virtual bool IsFormatPossible() const SAL_OVERRIDE;
    static void GetAnchoredObjects( std::list<SwAnchoredObject*>&, const SwFormat& rFormat );

    // overwriting "SwFrameFormat *SwLayoutFrm::GetFormat" to provide the correct derived return type.
    // (This is in order to skip on the otherwise necessary casting of the result to
    // 'SwFlyFrameFormat *' after calls to this function. The casting is now done in this function.)
    virtual const SwFlyFrameFormat *GetFormat() const SAL_OVERRIDE;
    virtual       SwFlyFrameFormat *GetFormat() SAL_OVERRIDE;

    virtual void dumpAsXml( xmlTextWriterPtr writer ) const SAL_OVERRIDE { SwLayoutFrm::dumpAsXml( writer ); };

    virtual void Calc() const SAL_OVERRIDE;

    const Point& ContentPos() const { return m_aContentPos; }
    Point& ContentPos() { return m_aContentPos; }

    void InvalidateContentPos();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
