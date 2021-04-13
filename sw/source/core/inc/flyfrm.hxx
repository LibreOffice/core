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
#include <vector>
#include <frmfmt.hxx>
#include <anchoredobject.hxx>
#include <swdllapi.h>

class SwFormatAnchor;
class SwPageFrame;
class SwFormatFrameSize;
struct SwCursorMoveState;
class SwBorderAttrs;
class SwVirtFlyDrawObj;
class SwAttrSetChg;
namespace tools { class PolyPolygon; }
class SwFormat;
class SwViewShell;
class SwFEShell;
class SwWrtShell;


/** search an anchor for paragraph bound frames starting from pOldAnch

    needed for dragging of objects bound to a paragraph for showing an anchor
    indicator as well as for changing the anchor.

    implemented in layout/flycnt.cxx
 */
const SwContentFrame *FindAnchor( const SwFrame *pOldAnch, const Point &rNew,
                              const bool bBody = false );

/** calculate rectangle in that the object can be moved or rather be resized */
bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, bool bMove = true );

/** general base class for all free-flowing frames

    #i26791# - inherit also from <SwAnchoredFlyFrame>
*/
class SW_DLLPUBLIC SwFlyFrame : public SwLayoutFrame, public SwAnchoredObject
{
    // is allowed to lock, implemented in frmtool.cxx
    friend void AppendObj(SwFrame *const pFrame, SwPageFrame *const pPage, SwFrameFormat *const pFormat, const SwFormatAnchor & rAnch);
    friend void Notify( SwFlyFrame *, SwPageFrame *pOld, const SwRect &rOld,
                        const SwRect* pOldPrt );

    void InitDrawObj();     // these to methods are called in the
    void FinitDrawObj();    // constructors

    void UpdateAttr_( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = nullptr, SwAttrSetChg *pb = nullptr );

    using SwLayoutFrame::CalcRel;

protected:
    // Predecessor/Successor for chaining with text flow
    SwFlyFrame *m_pPrevLink, *m_pNextLink;

private:
    // It must be possible to block Content-bound flys so that they will be not
    // formatted; in this case MakeAll() returns immediately. This is necessary
    // for page changes during formatting. In addition, it is needed during
    // the constructor call of the root object since otherwise the anchor will
    // be formatted before the root is anchored correctly to a shell and
    // because too much would be formatted as a result.
    bool m_bLocked :1;
    // true if the background of NotifyDTor needs to be notified at the end
    // of a MakeAll() call.
    bool m_bNotifyBack :1;

protected:
    // Pos, PrtArea or SSize have been invalidated - they will be evaluated
    // again immediately because they have to be valid _at all time_.
    // The invalidation is tracked here so that LayAction knows about it and
    // can handle it properly. Exceptions prove the rule.
    bool m_bInvalid :1;

    // true if the proposed height of an attribute is a minimal height
    // (this means that the frame can grow higher if needed)
    bool m_bMinHeight :1;
    // true if the fly frame could not format position/size based on its
    // attributes, e.g. because there was not enough space.
    bool m_bHeightClipped :1;
    bool m_bWidthClipped :1;
    // If true then call only the format after adjusting the width (CheckClip);
    // but the width will not be re-evaluated based on the attributes.
    bool m_bFormatHeightOnly :1;

    bool m_bInCnt :1;        ///< RndStdIds::FLY_AS_CHAR, anchored as character
    bool m_bAtCnt :1;        ///< RndStdIds::FLY_AT_PARA, anchored at paragraph
    bool m_bLayout :1;       ///< RndStdIds::FLY_AT_PAGE, RndStdIds::FLY_AT_FLY, at page or at frame
    bool m_bAutoPosition :1; ///< RndStdIds::FLY_AT_CHAR, anchored at character

    friend class SwNoTextFrame; // is allowed to call NotifyBackground

    Point m_aContentPos;        // content area's position relatively to Frame
    bool m_bValidContentPos;

    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    void MakePrtArea( const SwBorderAttrs &rAttrs );
    void MakeContentPos( const SwBorderAttrs &rAttrs );

    void Lock()         { m_bLocked = true; }
    void Unlock()       { m_bLocked = false; }

    Size CalcRel( const SwFormatFrameSize &rSz ) const;

    SwFlyFrame( SwFlyFrameFormat*, SwFrame*, SwFrame *pAnchor );

    virtual void DestroyImpl() override;
    virtual ~SwFlyFrame() override;

    /** method to assure that anchored object is registered at the correct
        page frame
    */
    virtual void RegisterAtCorrectPage() override;
    virtual void RegisterAtPage(SwPageFrame &) override;

    virtual bool SetObjTop_( const SwTwips _nTop ) override;
    virtual bool SetObjLeft_( const SwTwips _nLeft ) override;

    virtual const SwRect GetObjBoundRect() const override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
    virtual void SwClientNotify(const SwModify& rMod, const SfxHint& rHint) override;

    virtual const IDocumentDrawModelAccess& getIDocumentDrawModelAccess( ) override;

    SwTwips CalcContentHeight(const SwBorderAttrs *pAttrs, const SwTwips nMinHeight, const SwTwips nUL);

public:
    // #i26791#

    // get client information
    virtual bool GetInfo( SfxPoolItem& ) const override;
    virtual void PaintSwFrame( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = nullptr ) const override;
    virtual Size ChgSize( const Size& aNewSize ) override;
    virtual bool GetCursorOfst( SwPosition *, Point&,
                              SwCursorMoveState* = nullptr, bool bTestBackground = false ) const override;

    virtual void CheckDirection( bool bVert ) override;
    virtual void Cut() override;
#ifdef DBG_UTIL
    virtual void Paste( SwFrame* pParent, SwFrame* pSibling = nullptr ) override;
#endif

    SwTwips Shrink_( SwTwips, bool bTst );
    SwTwips Grow_  ( SwTwips, bool bTst );
    void    Invalidate_( SwPageFrame const *pPage = nullptr );

    bool FrameSizeChg( const SwFormatFrameSize & );

    SwFlyFrame *GetPrevLink() const { return m_pPrevLink; }
    SwFlyFrame *GetNextLink() const { return m_pNextLink; }

    static void ChainFrames( SwFlyFrame *pMaster, SwFlyFrame *pFollow );
    static void UnchainFrames( SwFlyFrame *pMaster, SwFlyFrame *pFollow );

    SwFlyFrame *FindChainNeighbour( SwFrameFormat const &rFormat, SwFrame *pAnch = nullptr );

    // #i26791#
    const SwVirtFlyDrawObj* GetVirtDrawObj() const;
    SwVirtFlyDrawObj *GetVirtDrawObj();
    void NotifyDrawObj();

    void ChgRelPos( const Point &rAbsPos );
    bool IsInvalid() const { return m_bInvalid; }
    void Invalidate() const { const_cast<SwFlyFrame*>(this)->m_bInvalid = true; }
    void Validate() const { const_cast<SwFlyFrame*>(this)->m_bInvalid = false; }

    bool IsMinHeight()  const { return m_bMinHeight; }
    bool IsLocked()     const { return m_bLocked; }
    bool IsAutoPos()    const { return m_bAutoPosition; }
    bool IsFlyInContentFrame() const { return m_bInCnt; }
    bool IsFlyFreeFrame() const { return m_bAtCnt || m_bLayout; }
    bool IsFlyLayFrame() const { return m_bLayout; }
    bool IsFlyAtContentFrame() const { return m_bAtCnt; }

    bool IsNotifyBack() const { return m_bNotifyBack; }
    void SetNotifyBack()      { m_bNotifyBack = true; }
    void ResetNotifyBack()    { m_bNotifyBack = false; }

    bool IsClipped()        const   { return m_bHeightClipped || m_bWidthClipped; }
    bool IsHeightClipped()  const   { return m_bHeightClipped; }

    bool IsLowerOf( const SwLayoutFrame* pUpper ) const;
    bool IsUpperOf( const SwFlyFrame& _rLower ) const
    {
        return _rLower.IsLowerOf( this );
    }

    SwFrame *FindLastLower();

    // #i13147# - add parameter <_bForPaint> to avoid load of
    // the graphic during paint. Default value: false
    bool GetContour( tools::PolyPolygon&   rContour,
                     const bool _bForPaint = false ) const;

    // Paint on this shell (consider Preview, print flag, etc. recursively)?
    static bool IsPaint( SdrObject *pObj, const SwViewShell *pSh );

    /** SwFlyFrame::IsBackgroundTransparent

        determines if background of fly frame has to be drawn transparently

        definition found in /core/layout/paintfrm.cxx

        @return true, if background color is transparent or a existing background
        graphic is transparent.
    */
    bool IsBackgroundTransparent() const;

    void Chain( SwFrame* _pAnchor );
    void Unchain();
    void InsertCnt();
    void DeleteCnt();
    void InsertColumns();

    // #i26791# - pure virtual methods of base class <SwAnchoredObject>
    virtual void MakeObjPos() override;
    virtual void InvalidateObjPos() override;

    virtual SwFrameFormat& GetFrameFormat() override;
    virtual const SwFrameFormat& GetFrameFormat() const override;

    virtual const SwRect GetObjRect() const override;

    /** method to determine if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwAnchoredObject::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame is locked resp. col-locked.
    */
    virtual bool IsFormatPossible() const override;
    static void GetAnchoredObjects( std::vector<SwAnchoredObject*>&, const SwFormat& rFormat );

    // overwriting "SwFrameFormat *SwLayoutFrame::GetFormat" to provide the correct derived return type.
    // (This is in order to skip on the otherwise necessary casting of the result to
    // 'SwFlyFrameFormat *' after calls to this function. The casting is now done in this function.)
    virtual const SwFlyFrameFormat *GetFormat() const override;
    virtual       SwFlyFrameFormat *GetFormat() override;

    virtual void dumpAsXml( xmlTextWriterPtr writer ) const override { SwLayoutFrame::dumpAsXml( writer ); };

    virtual void Calc(vcl::RenderContext* pRenderContext) const override;

    const Point& ContentPos() const { return m_aContentPos; }
    Point& ContentPos() { return m_aContentPos; }

    void InvalidateContentPos();

    void SelectionHasChanged(SwFEShell* pShell);
    bool IsShowUnfloatButton(SwWrtShell* pWrtSh) const;

    // For testing only (see uiwriter)
    void ActiveUnfloatButton(SwWrtShell* pWrtSh);

private:
    void UpdateUnfloatButton(SwWrtShell* pWrtSh, bool bShow) const;
    void PaintDecorators() const;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
