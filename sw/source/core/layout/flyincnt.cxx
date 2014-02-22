/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "doc.hxx"
#include "frmtool.hxx"
#include "hints.hxx"
#include <fmtornt.hxx>
#include "txtfrm.hxx"
#include "flyfrms.hxx"

#include <dflyobj.hxx>


void DeepCalc( const SwFrm *pFrm );

SwFlyInCntFrm::SwFlyInCntFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFrm( pFmt, pSib, pAnch )
{
    bInCnt = bInvalidLayout = bInvalidCntnt = sal_True;
    SwTwips nRel = pFmt->GetVertOrient().GetPos();
    
    Point aRelPos;
    if( pAnch && pAnch->IsVertical() )
        aRelPos.setX(pAnch->IsReverse() ? nRel : -nRel);
    else
        aRelPos.setY(nRel);
    SetCurrRelPos( aRelPos );
}

SwFlyInCntFrm::~SwFlyInCntFrm()
{
    if ( !GetFmt()->GetDoc()->IsInDtor() && GetAnchorFrm() )
    {
        SwRect aTmp( GetObjRectWithSpaces() );
        SwFlyInCntFrm::NotifyBackground( FindPageFrm(), aTmp, PREP_FLY_LEAVE );
    }
}


TYPEINIT1(SwFlyInCntFrm,SwFlyFrm);

void SwFlyInCntFrm::SetRefPoint( const Point& rPoint,
                                 const Point& rRelAttr,
                                 const Point& rRelPos )
{
    
    OSL_ENSURE( rPoint != aRef || rRelAttr != GetCurrRelPos(), "SetRefPoint: no change" );
    SwFlyNotify *pNotify = NULL;
    
    
    if( !IsLocked() )
        pNotify = new SwFlyNotify( this );
    aRef = rPoint;
    SetCurrRelPos( rRelAttr );
    SWRECTFN( GetAnchorFrm() )
    (Frm().*fnRect->fnSetPos)( rPoint + rRelPos );
    
    InvalidateObjRectWithSpaces();
    if( pNotify )
    {
        InvalidatePage();
        mbValidPos = sal_False;
        bInvalid  = sal_True;
        Calc();
        delete pNotify;
    }
}

void SwFlyInCntFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    bool bCallPrepare = false;
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    if( RES_ATTRSET_CHG == nWhich )
    {
        if( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->
            GetItemState( RES_SURROUND, false ) ||
            SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->
            GetItemState( RES_FRMMACRO, false ) )
        {
            SwAttrSetChg aOld( *(SwAttrSetChg*)pOld );
            SwAttrSetChg aNew( *(SwAttrSetChg*)pNew );

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
        else if( ((SwAttrSetChg*)pNew)->GetChgSet()->Count())
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
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG, GetFmt() );
}


void SwFlyInCntFrm::Format( const SwBorderAttrs *pAttrs )
{
    if ( !Frm().Height() )
    {
        Lock(); 
        SwCntntFrm *pCntnt = ContainsCntnt();
        while ( pCntnt )
        {   pCntnt->Calc();
            pCntnt = pCntnt->GetNextCntntFrm();
        }
        Unlock();
    }
    SwFlyFrm::Format( pAttrs );
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
        mbValidPos = sal_True;
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
        const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
        
        
        const bool bVert = GetAnchorFrm()->IsVertical();
        const bool bRev = GetAnchorFrm()->IsReverse();
        SwTwips nOld = rVert.GetPos();
        SwTwips nAct = bVert ? -GetCurrRelPos().X() : GetCurrRelPos().Y();
        if( bRev )
            nAct = -nAct;
        if( nAct != nOld )
        {
            SwFmtVertOrient aVert( rVert );
            aVert.SetPos( nAct );
            pFmt->LockModify();
            pFmt->SetFmtAttr( aVert );
            pFmt->UnlockModify();
        }
    }
}


void SwFlyInCntFrm::_ActionOnInvalidation( const InvalidationType _nInvalid )
{
    if ( INVALID_POS == _nInvalid || INVALID_ALL == _nInvalid )
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG, &GetFrmFmt() );
}

void SwFlyInCntFrm::NotifyBackground( SwPageFrm *, const SwRect& rRect,
                                       PrepareHint eHint)
{
    if ( eHint == PREP_FLY_ATTR_CHG )
        AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG );
    else
        AnchorFrm()->Prepare( eHint, (void*)&rRect );
}

const Point SwFlyInCntFrm::GetRelPos() const
{
    Calc();
    return GetCurrRelPos();
}


void SwFlyInCntFrm::RegistFlys()
{
    SwPageFrm *pPage = FindPageFrm();
    OSL_ENSURE( pPage, "Register Flys without pages?" );
    ::RegistFlys( pPage, this );
}

void SwFlyInCntFrm::MakeAll()
{
    
    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrm() || IsLocked() || IsColLocked() || !FindPageFrm() )
        return;

    Lock(); 

        
    const SwFlyNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    if ( IsClipped() )
        mbValidSize = bHeightClipped = bWidthClipped = sal_False;

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        
        if ( !mbValidSize )
        {
            mbValidPrtArea = sal_False;
        }

        if ( !mbValidPrtArea )
            MakePrtArea( rAttrs );

        if ( !mbValidSize )
            Format( &rAttrs );

        if ( !mbValidPos )
        {
            MakeObjPos();
        }

        
        
        if ( mbValidPos && mbValidSize &&
             GetFmt()->getIDocumentSettingAccess()->get( IDocumentSettingAccess::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME ) )
        {
            SwFrm* pFrm = AnchorFrm();
            if ( Frm().Left() == (pFrm->Frm().Left()+pFrm->Prt().Left()) &&
                 Frm().Width() > pFrm->Prt().Width() )
            {
                Frm().Width( pFrm->Prt().Width() );
                mbValidPrtArea = sal_False;
                bWidthClipped = sal_True;
            }
        }
    }
    Unlock();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
