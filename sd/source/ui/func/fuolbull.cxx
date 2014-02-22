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

#include "fuolbull.hxx"
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/numitem.hxx>
#include "sdresid.hxx"
#include "glob.hrc"

#include <editeng/editdata.hxx>
#include <svx/svxids.hrc>
#include "OutlineView.hxx"
#include "OutlineViewShell.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "sdabstdlg.hxx"
#include <svx/nbdtmg.hxx>
#include <svx/nbdtmgfact.hxx>
#include <svx/svdoutl.hxx>
#include <boost/scoped_ptr.hpp>
using namespace svx::sidebar;
namespace sd {

TYPEINIT1( FuOutlineBullet, FuPoor );


FuOutlineBullet::FuOutlineBullet(ViewShell* pViewShell, ::sd::Window* pWindow,
                                 ::sd::View* pView, SdDrawDocument* pDoc,
                                 SfxRequest& rReq)
       : FuPoor(pViewShell, pWindow, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuOutlineBullet::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuOutlineBullet( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuOutlineBullet::DoExecute( SfxRequest& rReq )
{
    const sal_uInt16 nSId = rReq.GetSlot();
    if ( nSId == FN_SVX_SET_BULLET || nSId == FN_SVX_SET_NUMBER )
    {
        SetCurrentBulletsNumbering(rReq);
        return;
    }

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );

        SfxItemSet aNewAttr( mpViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );
        aNewAttr.Put( aEditAttr, false );

        
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        SfxAbstractTabDialog* pDlg = pFact ? pFact->CreateSdOutlineBulletTabDlg( NULL, &aNewAttr, mpView ) : 0;
        if( pDlg )
        {
            sal_uInt16 nResult = pDlg->Execute();

            switch( nResult )
            {
                case RET_OK:
                {
                    SfxItemSet aSet( *pDlg->GetOutputItemSet() );

                    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                    boost::scoped_ptr< OutlineViewModelChangeGuard > aGuard;

                    if (mpView->ISA(OutlineView))
                    {
                        pOLV = static_cast<OutlineView*>(mpView)
                            ->GetViewByWindow(mpViewShell->GetActiveWindow());

                        aGuard.reset( new OutlineViewModelChangeGuard( static_cast<OutlineView&>(*mpView) ) );
                    }

                    if( pOLV )
                        pOLV->EnableBullets();

                    rReq.Done( aSet );
                    pArgs = rReq.GetArgs();
                }
                break;

                default:
                {
                    delete pDlg;
                    return;
                }
            }

            delete pDlg;
        }
    }

    /* not direct to pOlView; therefore, SdDrawView::SetAttributes can catch
       changes to master page and redirect to a template */
    mpView->SetAttributes(*pArgs);

/* #i35937#
    
    mpViewShell->Invalidate( FN_NUM_BULLET_ON );
*/
}

void FuOutlineBullet::SetCurrentBulletsNumbering(SfxRequest& rReq)
{
    if (!mpDoc || !mpView)
        return;

    const sal_uInt16 nSId = rReq.GetSlot();
    if ( nSId != FN_SVX_SET_BULLET && nSId != FN_SVX_SET_NUMBER )
    {
        
        return;
    }

    SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, nSId, false );
    if ( !pItem )
    {
        rReq.Done();
        return;
    }

    SfxItemSet aNewAttr( mpViewShell->GetPool(), EE_ITEMS_START, EE_ITEMS_END );
    {
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );
        aNewAttr.Put( aEditAttr, false );
    }

    const DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);
    
    const bool bInMasterView = pDrawViewShell && pDrawViewShell->GetEditMode() == EM_MASTERPAGE;
    if ( bInMasterView )
    {
        SdrObject* pObj = mpView->GetTextEditObject();
        if( pObj && pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
        {
            const sal_uInt16 nLevel = mpView->GetSelectionLevel();
            if( nLevel != 0xFFFF )
            {
                
                SfxItemSet aStoreSet( aNewAttr );
                aNewAttr.ClearItem();
                
                aNewAttr.MergeRange( SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL );
                aNewAttr.Put( aStoreSet );
                
                aNewAttr.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, nLevel ) );
            }
        }
    }

    sal_uInt16 nIdx = pItem->GetValue();
    bool bToggle = false;
    bool bSwitchOff = false;
    if( nIdx == (sal_uInt16)0xFFFF )
    {
        
        nIdx = 1;
        bToggle = true;
    }
    else if (nIdx == DEFAULT_NONE)
    {
        bSwitchOff = true;
    }
    nIdx--;

    sal_uInt32 nNumItemId = SID_ATTR_NUMBERING_RULE;
    const SfxPoolItem* pTmpItem = GetNumBulletItem( aNewAttr, nNumItemId );
    SvxNumRule* pNumRule = NULL;
    if ( pTmpItem )
    {
        pNumRule = new SvxNumRule(*((SvxNumBulletItem*)pTmpItem)->GetNumRule());

        
        NBOTypeMgrBase* pNumRuleMgr =
            NBOutlineTypeMgrFact::CreateInstance(
                nSId == FN_SVX_SET_BULLET ? eNBOType::MIXBULLETS : eNBOType::NUMBERING );
        if ( pNumRuleMgr )
        {
            sal_uInt16 nActNumLvl = (sal_uInt16)0xFFFF;
            const SfxPoolItem* pNumLevelItem = NULL;
            if(SFX_ITEM_SET == aNewAttr.GetItemState(SID_PARAM_CUR_NUM_LEVEL, false, &pNumLevelItem))
                nActNumLvl = ((const SfxUInt16Item*)pNumLevelItem)->GetValue();

            pNumRuleMgr->SetItems(&aNewAttr);
            SvxNumRule aTmpRule( *pNumRule );
            if ( nSId == FN_SVX_SET_BULLET && bToggle && nIdx==0 )
            {
                
                pNumRuleMgr->ApplyNumRule( aTmpRule, nIdx, nActNumLvl, true );
            }
            else
            {
                pNumRuleMgr->ApplyNumRule( aTmpRule, nIdx, nActNumLvl );
            }

            sal_uInt16 nMask = 1;
            for(sal_uInt16 i = 0; i < pNumRule->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                {
                    SvxNumberFormat aFmt(aTmpRule.GetLevel(i));
                    pNumRule->SetLevel(i, aFmt);
                }
                nMask <<= 1;
            }
        }
    }

    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
    boost::scoped_ptr< OutlineViewModelChangeGuard > aGuard;
    {
        if (mpView->ISA(OutlineView))
        {
            pOLV = static_cast<OutlineView*>(mpView)
                ->GetViewByWindow(mpViewShell->GetActiveWindow());

            aGuard.reset( new OutlineViewModelChangeGuard( static_cast<OutlineView&>(*mpView) ) );
        }
    }

    SdrOutliner* pOwner = bInMasterView ? mpView->GetTextEditOutliner() : 0;
    const bool bOutlinerUndoEnabled = pOwner && !pOwner->IsInUndo() && pOwner->IsUndoEnabled();
    SdrModel* pSdrModel = bInMasterView ? mpView->GetModel() : 0;
    const bool bModelUndoEnabled = pSdrModel && pSdrModel->IsUndoEnabled();

    if ( bOutlinerUndoEnabled )
    {
        pOwner->UndoActionStart( OLUNDO_ATTR );
    }
    else if ( bModelUndoEnabled )
    {
        pSdrModel->BegUndo();
    }

    if ( pOLV )
    {
        if ( bSwitchOff )
        {
            pOLV->SwitchOffBulletsNumbering( true );
        }
        else
        {
            pOLV->ToggleBulletsNumbering( bToggle, nSId == FN_SVX_SET_BULLET, bInMasterView ? 0 : pNumRule );
        }
    }
    else
    {
        mpView->ChangeMarkedObjectsBulletsNumbering( bToggle, nSId == FN_SVX_SET_BULLET, bInMasterView ? 0 : pNumRule, bSwitchOff );
    }
    if ( bInMasterView )
    {
        SfxItemSet aSetAttr( mpViewShell->GetPool(), EE_ITEMS_START, EE_ITEMS_END );
        aSetAttr.Put(SvxNumBulletItem( *pNumRule ), nNumItemId);
        mpView->SetAttributes(aSetAttr);
    }

    if( bOutlinerUndoEnabled )
    {
        pOwner->UndoActionEnd( OLUNDO_ATTR );
    }
    else if ( bModelUndoEnabled )
    {
        pSdrModel->EndUndo();
    }

    delete pNumRule;
    rReq.Done();
}

const SfxPoolItem* FuOutlineBullet::GetNumBulletItem(SfxItemSet& aNewAttr, sal_uInt32& nNumItemId)
{
    
    const SfxPoolItem* pTmpItem = NULL;

    if(aNewAttr.GetItemState(nNumItemId, false, &pTmpItem) == SFX_ITEM_SET)
    {
        return pTmpItem;
    }
    else
    {
        nNumItemId = aNewAttr.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        SfxItemState eState = aNewAttr.GetItemState(nNumItemId, false, &pTmpItem);
        if (eState == SFX_ITEM_SET)
            return pTmpItem;
        else
        {
            sal_Bool bOutliner = sal_False;
            sal_Bool bTitle = sal_False;

            if( mpView )
            {
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                const sal_uInt32 nCount = rMarkList.GetMarkCount();

                for(sal_uInt32 nNum = 0; nNum < nCount; nNum++)
                {
                    SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
                    if( pObj->GetObjInventor() == SdrInventor )
                    {
                        switch(pObj->GetObjIdentifier())
                        {
                        case OBJ_TITLETEXT:
                            bTitle = sal_True;
                            break;
                        case OBJ_OUTLINETEXT:
                            bOutliner = sal_True;
                            break;
                        }
                    }
                }
            }

            const SvxNumBulletItem *pItem = NULL;
            if(bOutliner)
            {
                SfxStyleSheetBasePool* pSSPool = mpView->GetDocSh()->GetStyleSheetPool();
                OUString aStyleName(SD_RESSTR(STR_LAYOUT_OUTLINE) + " 1");
                SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);
                if( pFirstStyleSheet )
                    pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, (const SfxPoolItem**)&pItem);
            }

            if( pItem == NULL )
                pItem = (SvxNumBulletItem*) aNewAttr.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET);

            

            aNewAttr.Put(*pItem, EE_PARA_NUMBULLET);

            if(bTitle && aNewAttr.GetItemState(EE_PARA_NUMBULLET,true) == SFX_ITEM_ON )
            {
                SvxNumBulletItem* pBulletItem = (SvxNumBulletItem*)aNewAttr.GetItem(EE_PARA_NUMBULLET,true);
                SvxNumRule* pLclRule = pBulletItem->GetNumRule();
                if(pLclRule)
                {
                    SvxNumRule aNewRule( *pLclRule );
                    aNewRule.SetFeatureFlag( NUM_NO_NUMBERS, sal_True );

                    SvxNumBulletItem aNewItem( aNewRule, EE_PARA_NUMBULLET );
                    aNewAttr.Put(aNewItem);
                }
            }

            SfxItemState eItemState = aNewAttr.GetItemState(nNumItemId, false, &pTmpItem);
            if (eItemState == SFX_ITEM_SET)
                return pTmpItem;

        }
        
    }
    return pTmpItem;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
