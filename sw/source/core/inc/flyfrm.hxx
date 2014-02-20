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
class SwFmtFrmSize;
struct SwCrsrMoveState;
class SwBorderAttrs;
class SwVirtFlyDrawObj;
class SwFrmFmts;
class SwAttrSetChg;
class PolyPolygon;
class SwFlyDrawContact;
class SwFmt;

#include <anchoredobject.hxx>

/** search an anchor for paragraph bound frames starting from pOldAnch

    needed for dragging of objects bound to a paragraph for showing an anchor
    indicator as well as for changing the anchor.

    implemented in layout/flycnt.cxx
 */
const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const sal_Bool bBody = sal_False );

/** calculate rectangle in that the object can be moved or rather be resized */
sal_Bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, sal_Bool bMove = sal_True );

/** general base class for all free-flowing frames

    #i26791# - inherit also from <SwAnchoredFlyFrm>
*/
class SwFlyFrm : public SwLayoutFrm, public SwAnchoredObject
{
    // is allowed to lock, implemented in frmtool.cxx
    friend void AppendObjs   ( const SwFrmFmts *, sal_uLong, SwFrm *, SwPageFrm * );
    friend void Notify( SwFlyFrm *, SwPageFrm *pOld, const SwRect &rOld,
                        const SwRect* pOldPrt );

    void InitDrawObj( sal_Bool bNotify ); // these to methods are called in the
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
    // It must be possible to block Cntnt-bound flys so that they will be not
    // formatted; in this case MakeAll() returns immediately. This is necessary
    // for page changes during formattting. In addition, it is needed during
    // the constructor call of the root object since otherwise the anchor will
    // be formatted before the root is anchored correctly to a shell and
    // because too much would be formatted as a result.
    sal_Bool bLocked :1;
    // sal_True if the background of NotifyDTor needs to be notified at the end
    // of a MakeAll() call.
    sal_Bool bNotifyBack :1;

protected:
    // Pos, PrtArea or SSize have been invalidated - they will be evaluated
    // again immediately because they have to be valid _at all time_.
    // The invalidation is tracked here so that LayAction knows about it and
    // can handle it properly. Exceptions prove the rule.
    sal_Bool bInvalid :1;

    // sal_True if the proposed height of an attribute is a minimal height
    // (this means that the frame can grow higher if needed)
    sal_Bool bMinHeight :1;
    // sal_True if the fly frame could not format position/size based on its
    // attributes, e.g. because there was not enough space.
    sal_Bool bHeightClipped :1;
    sal_Bool bWidthClipped :1;
    // If sal_True call only the format after adjusting the width (CheckClip);
    // but the width will not be re-evaluated based on the attributes.
    sal_Bool bFormatHeightOnly :1;

    sal_Bool bInCnt :1;        ///< FLY_AS_CHAR, anchored as character
    sal_Bool bAtCnt :1;        ///< FLY_AT_PARA, anchored at paragraph
    sal_Bool bLayout :1;       ///< FLY_AT_PAGE, FLY_AT_FLY, at page or at frame
    sal_Bool bAutoPosition :1; ///< FLY_AT_CHAR, anchored at character

    sal_Bool bNoShrink :1;     ///< temporary forbid shrinking to avoid loops
    // If sal_True, the content of the fly frame will not be deleted when it
    // is moved to an invisible layer.
    sal_Bool bLockDeleteContent :1;

    friend class SwNoTxtFrm; // is allowed to call NotifyBackground

    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    void MakePrtArea( const SwBorderAttrs &rAttrs );

    void Lock()         { bLocked = sal_True; }
    void Unlock()       { bLocked = sal_False; }

    void SetMinHeight()  { bMinHeight = sal_True; }
    void ResetMinHeight(){ bMinHeight = sal_False; }

    Size CalcRel( const SwFmtFrmSize &rSz ) const;
    SwTwips CalcAutoWidth() const;

    SwFlyFrm( SwFlyFrmFmt*, SwFrm*, SwFrm *pAnchor );

    /** method to assure that anchored object is registered at the correct
        page frame

        #i28701#
    */
    virtual void RegisterAtCorrectPage();

    virtual bool _SetObjTop( const SwTwips _nTop );
    virtual bool _SetObjLeft( const SwTwips _nLeft );

    virtual const SwRect GetObjBoundRect() const;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

    virtual const IDocumentDrawModelAccess* getIDocumentDrawModelAccess( );

public:
    // #i26791#
    TYPEINFO();

    virtual ~SwFlyFrm();
    // get client information
    virtual bool GetInfo( SfxPoolItem& ) const;
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual Size ChgSize( const Size& aNewSize );
    virtual bool GetCrsrOfst( SwPosition *, Point&,
                              SwCrsrMoveState* = 0, bool bTestBackground = false ) const;

    virtual void CheckDirection( bool bVert );
    virtual void Cut();
#ifdef DBG_UTIL
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

    SwTwips _Shrink( SwTwips, sal_Bool bTst );
    SwTwips _Grow  ( SwTwips, sal_Bool bTst );
    void    _Invalidate( SwPageFrm *pPage = 0 );

    sal_Bool FrmSizeChg( const SwFmtFrmSize & );

    SwFlyFrm *GetPrevLink() const { return pPrevLink; }
    SwFlyFrm *GetNextLink() const { return pNextLink; }

    static void ChainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );
    static void UnchainFrames( SwFlyFrm *pMaster, SwFlyFrm *pFollow );

    SwFlyFrm *FindChainNeighbour( SwFrmFmt &rFmt, SwFrm *pAnch = 0 );

    // #i26791#
    const SwVirtFlyDrawObj* GetVirtDrawObj() const;
    SwVirtFlyDrawObj *GetVirtDrawObj();
    void NotifyDrawObj();

    void ChgRelPos( const Point &rAbsPos );
    sal_Bool IsInvalid() const { return bInvalid; }
    void Invalidate() const { ((SwFlyFrm*)this)->bInvalid = sal_True; }
    void Validate() const { ((SwFlyFrm*)this)->bInvalid = sal_False; }

    sal_Bool IsMinHeight()  const { return bMinHeight; }
    sal_Bool IsLocked()     const { return bLocked; }
    sal_Bool IsAutoPos()    const { return bAutoPosition; }
    sal_Bool IsFlyInCntFrm() const { return bInCnt; }
    sal_Bool IsFlyFreeFrm() const { return bAtCnt || bLayout; }
    sal_Bool IsFlyLayFrm() const { return bLayout; }
    sal_Bool IsFlyAtCntFrm() const { return bAtCnt; }

    sal_Bool IsNotifyBack() const { return bNotifyBack; }
    void SetNotifyBack()      { bNotifyBack = sal_True; }
    void ResetNotifyBack()    { bNotifyBack = sal_False; }
    sal_Bool IsNoShrink()   const { return bNoShrink; }
    void SetNoShrink( sal_Bool bNew ) { bNoShrink = bNew; }
    sal_Bool IsLockDeleteContent()  const { return bLockDeleteContent; }
    void SetLockDeleteContent( sal_Bool bNew ) { bLockDeleteContent = bNew; }


    sal_Bool IsClipped()        const   { return bHeightClipped || bWidthClipped; }
    sal_Bool IsHeightClipped()  const   { return bHeightClipped; }
    sal_Bool IsWidthClipped()   const   { return bWidthClipped;  }

    sal_Bool IsLowerOf( const SwLayoutFrm* pUpper ) const;
    inline sal_Bool IsUpperOf( const SwFlyFrm& _rLower ) const
    {
        return _rLower.IsLowerOf( this );
    }

    SwFrm *FindLastLower();

    // #i13147# - add parameter <_bForPaint> to avoid load of
    // the graphic during paint. Default value: sal_False
    sal_Bool GetContour( PolyPolygon&   rContour,
                     const sal_Bool _bForPaint = sal_False ) const;


    // Paint on this shell (consider Preview, print flag, etc. recursively)?
    static sal_Bool IsPaint( SdrObject *pObj, const SwViewShell *pSh );

    /** SwFlyFrm::IsBackgroundTransparent

        determines if background of fly frame has to be drawn transparently

        definition found in /core/layout/paintfrm.cxx

        @return true, if background color is transparent or a existing background
        graphic is transparent.
    */
    bool IsBackgroundTransparent() const;

    /** SwFlyFrm::IsShadowTransparent

        determine if shadow color of fly frame has to be drawn transparently

        definition found in /core/layout/paintfrm.cxx

        @return true, if shadow color is transparent.
    */
    bool IsShadowTransparent() const;

    void Chain( SwFrm* _pAnchor );
    void Unchain();
    void InsertCnt();
    void DeleteCnt();
    void InsertColumns();

    // #i26791# - pure virtual methods of base class <SwAnchoredObject>
    virtual void MakeObjPos();
    virtual void InvalidateObjPos();

    virtual SwFrmFmt& GetFrmFmt();
    virtual const SwFrmFmt& GetFrmFmt() const;

    virtual const SwRect GetObjRect() const;

    /** method to determine if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwAnchoredObject::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame is locked resp. col-locked.
    */
    virtual bool IsFormatPossible() const;
    static void GetAnchoredObjects( std::list<SwAnchoredObject*>&, const SwFmt& rFmt );

    // overwriting "SwFrmFmt *SwLayoutFrm::GetFmt" to provide the correct derived return type.
    // (This is in order to skip on the otherwise necessary casting of the result to
    // 'SwFlyFrmFmt *' after calls to this function. The casting is now done in this function.)
    virtual const SwFlyFrmFmt *GetFmt() const;
    virtual       SwFlyFrmFmt *GetFmt();

    virtual void dumpAsXml( xmlTextWriterPtr writer ) { SwLayoutFrm::dumpAsXml( writer ); };
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
