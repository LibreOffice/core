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

#include "doc.hxx"
#include "frmtool.hxx"
#include "hints.hxx"
#include <fmtornt.hxx>
#include "rootfrm.hxx"
#include "txtfrm.hxx"
#include "flyfrms.hxx"
#include <dflyobj.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>

SwFlyInCntFrm::SwFlyInCntFrm( SwFlyFrameFormat *pFormat, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFrm( pFormat, pSib, pAnch )
{
    m_bInCnt = bInvalidLayout = bInvalidContent = true;
    SwTwips nRel = pFormat->GetVertOrient().GetPos();
    // OD 2004-05-27 #i26791# - member <aRelPos> moved to <SwAnchoredObject>
    Point aRelPos;
    if( pAnch && pAnch->IsVertical() )
        aRelPos.setX(pAnch->IsReverse() ? nRel : -nRel);
    else
        aRelPos.setY(nRel);
    SetCurrRelPos( aRelPos );
}

void SwFlyInCntFrm::DestroyImpl()
{
    if ( !GetFormat()->GetDoc()->IsInDtor() && GetAnchorFrm() )
    {
        SwRect aTmp( GetObjRectWithSpaces() );
        SwFlyInCntFrm::NotifyBackground( FindPageFrm(), aTmp, PREP_FLY_LEAVE );
    }

    SwFlyFrm::DestroyImpl();
}

SwFlyInCntFrm::~SwFlyInCntFrm()
{
}

// #i28701#

void SwFlyInCntFrm::SetRefPoint( const Point& rPoint,
                                 const Point& rRelAttr,
                                 const Point& rRelPos )
{
    // OD 2004-05-27 #i26791# - member <aRelPos> moved to <SwAnchoredObject>
    OSL_ENSURE( rPoint != aRef || rRelAttr != GetCurrRelPos(), "SetRefPoint: no change" );
    SwFlyNotify *pNotify = NULL;
    // No notify at a locked fly frame, if a fly frame is locked, there's
    // already a SwFlyNotify object on the stack (MakeAll).
    if( !IsLocked() )
        pNotify = new SwFlyNotify( this );
    aRef = rPoint;
    SetCurrRelPos( rRelAttr );
    SWRECTFN( GetAnchorFrm() )
    (Frm().*fnRect->fnSetPos)( rPoint + rRelPos );
    // #i68520#
    InvalidateObjRectWithSpaces();
    if( pNotify )
    {
        InvalidatePage();
        mbValidPos = false;
        m_bInvalid  = true;
        Calc(getRootFrm()->GetCurrShell()->GetOut());
        delete pNotify;
    }
}

void SwFlyInCntFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    bool bCallPrepare = false;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    if (RES_ATTRSET_CHG == nWhich && pNew)
    {
        if(pOld &&
            (SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->
            GetItemState(RES_SURROUND, false) ||
            SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->
            GetItemState(RES_FRMMACRO, false)) )
        {
            SwAttrSetChg aOld( *static_cast<const SwAttrSetChg*>(pOld) );
            SwAttrSetChg aNew( *static_cast<const SwAttrSetChg*>(pNew) );

            aOld.ClearItem( RES_SURROUND );
            aNew.ClearItem( RES_SURROUND );
            aOld.ClearItem( RES_FRMMACRO );
            aNew.ClearItem( RES_FRMMACRO );
            if( aNew.Count() )
            {
                SwFlyFrm::Modify( &aOld, &aNew );
                bCallPrepare = true;
            }
        }
        else if( static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->Count())
        {
            SwFlyFrm::Modify( pOld, pNew );
            bCallPrepare = true;
        }
    }
    else if( nWhich != RES_SURROUND && RES_FRMMACRO != nWhich )
    {
        SwFlyFrm::Modify( pOld, pNew );
        bCallPrepare = true;
    }

    if ( bCallPrepare && GetAnchorFrm() )
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG, GetFormat() );
}

/// Here the content gets formatted initially.
void SwFlyInCntFrm::Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs )
{
    if ( !Frm().Height() )
    {
        Lock(); //don't format the anchor on the crook.
        SwContentFrm *pContent = ContainsContent();
        while ( pContent )
        {   pContent->Calc(pRenderContext);
            pContent = pContent->GetNextContentFrm();
        }
        Unlock();
    }
    SwFlyFrm::Format( pRenderContext, pAttrs );
}

/** Calculate object position
 *
 * @note: In contrast to other Frames, we only calculate the relative position
 *        here. The absolute position is only calculated using SetAbsPos.
 **/
void SwFlyInCntFrm::MakeObjPos()
{
    if ( !mbValidPos )
    {
        mbValidPos = true;
        SwFlyFrameFormat *pFormat = GetFormat();
        const SwFormatVertOrient &rVert = pFormat->GetVertOrient();
        //Update the current values in the format if needed, during this we of
        //course must not send any Modify.
        const bool bVert = GetAnchorFrm()->IsVertical();
        const bool bRev = GetAnchorFrm()->IsReverse();
        SwTwips nOld = rVert.GetPos();
        SwTwips nAct = bVert ? -GetCurrRelPos().X() : GetCurrRelPos().Y();
        if( bRev )
            nAct = -nAct;
        if( nAct != nOld )
        {
            SwFormatVertOrient aVert( rVert );
            aVert.SetPos( nAct );
            pFormat->LockModify();
            pFormat->SetFormatAttr( aVert );
            pFormat->UnlockModify();
        }
    }
}

void SwFlyInCntFrm::_ActionOnInvalidation( const InvalidationType _nInvalid )
{
    if ( INVALID_POS == _nInvalid || INVALID_ALL == _nInvalid )
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG, &GetFrameFormat() );
}

void SwFlyInCntFrm::NotifyBackground( SwPageFrm *, const SwRect& rRect,
                                       PrepareHint eHint)
{
    if ( eHint == PREP_FLY_ATTR_CHG )
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG );
    else
        AnchorFrm()->Prepare( eHint, static_cast<void const *>(&rRect) );
}

const Point SwFlyInCntFrm::GetRelPos() const
{
    Calc(getRootFrm()->GetCurrShell()->GetOut());
    return GetCurrRelPos();
}

/// @see SwRowFrm::RegistFlys()
void SwFlyInCntFrm::RegistFlys()
{
    SwPageFrm *pPage = FindPageFrm();
    OSL_ENSURE( pPage, "Register Flys without pages?" );
    ::RegistFlys( pPage, this );
}

void SwFlyInCntFrm::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    // OD 2004-01-19 #110582#
    if ( !GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrm() || IsLocked() || IsColLocked() || !FindPageFrm() )
        return;

    Lock(); // The curtain falls

        //does the notification in the DTor
    const SwFlyNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    if ( IsClipped() )
        mbValidSize = m_bHeightClipped = m_bWidthClipped = false;

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea || !m_bValidContentPos )
    {
        //Only stop, if the flag is set!!
        if ( !mbValidSize )
        {
            mbValidPrtArea = false;
        }

        if ( !mbValidPrtArea )
        {
            MakePrtArea( rAttrs );
            m_bValidContentPos = false;
        }

        if ( !mbValidSize )
            Format( getRootFrm()->GetCurrShell()->GetOut(), &rAttrs );

        if ( !mbValidPos )
        {
            MakeObjPos();
        }

        if ( !m_bValidContentPos )
            MakeContentPos( rAttrs );

        // re-activate clipping of as-character anchored Writer fly frames
        // depending on compatibility option <ClipAsCharacterAnchoredWriterFlyFrames>
        if ( mbValidPos && mbValidSize &&
             GetFormat()->getIDocumentSettingAccess().get( DocumentSettingId::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME ) )
        {
            SwFrm* pFrm = AnchorFrm();
            if ( Frm().Left() == (pFrm->Frm().Left()+pFrm->Prt().Left()) &&
                 Frm().Width() > pFrm->Prt().Width() )
            {
                Frm().Width( pFrm->Prt().Width() );
                mbValidPrtArea = false;
                m_bWidthClipped = true;
            }
        }
    }
    Unlock();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
