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

#include <fuolbull.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/numitem.hxx>
#include <strings.hxx>

#include <svx/svxids.hrc>
#include <OutlineView.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <sdabstdlg.hxx>
#include <svx/nbdtmg.hxx>
#include <svx/nbdtmgfact.hxx>
#include <svx/svdoutl.hxx>
#include <memory>

using namespace svx::sidebar;
namespace sd {

FuBulletAndPosition::FuBulletAndPosition(ViewShell* pViewShell, ::sd::Window* pWindow,
                                 ::sd::View* pView, SdDrawDocument* pDoc,
                                 SfxRequest& rReq)
       : FuPoor(pViewShell, pWindow, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuBulletAndPosition::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuBulletAndPosition( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuBulletAndPosition::DoExecute( SfxRequest& rReq )
{
    const sal_uInt16 nSId = rReq.GetSlot();
    if ( nSId == FN_SVX_SET_BULLET || nSId == FN_SVX_SET_NUMBER )
    {
        SetCurrentBulletsNumbering(rReq);
        return;
    }

    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxStringItem* pPageItem = SfxItemSet::GetItem<SfxStringItem>(pArgs, FN_PARAM_1, false);

    if ( pArgs && !pPageItem )
    {
        /* not direct to pOlView; therefore, SdDrawView::SetAttributes can catch
           changes to master page and redirect to a template */
        mpView->SetAttributes(*pArgs);
        return;
    }

    // fill ItemSet for Dialog
    SfxItemSet aEditAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aEditAttr );

    SfxItemSetFixed<EE_PARA_NUMBULLET, EE_PARA_BULLET> aNewAttr( mpViewShell->GetPool() );
    aNewAttr.Put( aEditAttr, false );

    auto pView = mpView;

    // create and execute dialog
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxBulletAndPositionDlg> pDlg(pFact->CreateSvxBulletAndPositionDlg(mpViewShell->GetFrameWeld(), &aNewAttr, mpView));
    sal_uInt16 nResult = pDlg->Execute();

    if( nResult == RET_OK )
    {
        OutlinerView* pOLV = pView->GetTextEditOutlinerView();

        std::unique_ptr<OutlineViewModelChangeGuard, o3tl::default_delete<OutlineViewModelChangeGuard>> aGuard;

        if (OutlineView* pOutlineView = dynamic_cast<OutlineView*>(pView))
        {
            pOLV = pOutlineView->GetViewByWindow(mpViewShell->GetActiveWindow());
            aGuard.reset(new OutlineViewModelChangeGuard(*pOutlineView));
        }

        if( pOLV )
            pOLV->EnableBullets();

        const SfxItemSet pOutputSet( *pDlg->GetOutputItemSet( &aNewAttr ) );
        pView->SetAttributes(pOutputSet, /*bReplaceAll=*/false, /*bSlide*/ pDlg->IsSlideScope(), /*bMaster=*/pDlg->IsApplyToMaster());
    }

    rReq.Done();
}

void FuBulletAndPosition::SetCurrentBulletsNumbering(SfxRequest& rReq)
{
    if (!mpDoc || !mpView)
        return;

    const sal_uInt16 nSId = rReq.GetSlot();
    if ( nSId != FN_SVX_SET_BULLET && nSId != FN_SVX_SET_NUMBER )
    {
        // unexpected SfxRequest
        return;
    }

    const SfxUInt16Item* pItem = rReq.GetArg<SfxUInt16Item>(nSId);
    if ( !pItem )
    {
        rReq.Done();
        return;
    }

    SfxItemSetFixed<EE_ITEMS_START, EE_ITEMS_END> aNewAttr( mpViewShell->GetPool() );
    {
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );
        aNewAttr.Put( aEditAttr, false );
    }

    const DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);
    //Init bullet level in "Customize" tab page in bullet dialog in master page view
    const bool bInMasterView = pDrawViewShell && pDrawViewShell->GetEditMode() == EditMode::MasterPage;
    if ( bInMasterView )
    {
        SdrObject* pObj = mpView->GetTextEditObject();
        if( pObj && pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
        {
            const sal_uInt16 nLevel = mpView->GetSelectionLevel();
            if( nLevel != 0xFFFF )
            {
                //save the itemset value
                SfxItemSet aStoreSet( aNewAttr );
                aNewAttr.ClearItem();
                //extend range
                aNewAttr.MergeRange( SID_PARAM_NUM_PRESET, SID_PARAM_CUR_NUM_LEVEL );
                aNewAttr.Put( aStoreSet );
                //put current level user selected
                aNewAttr.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, nLevel ) );
            }
        }
    }

    sal_uInt16 nIdx = pItem->GetValue();
    bool bToggle = false;
    if( nIdx == sal_uInt16(0xFFFF) )
    {
        // If the nIdx is (sal_uInt16)0xFFFF, means set bullet status to on/off
        nIdx = 1;
        bToggle = true;
    }
    nIdx--;

    sal_uInt32 nNumItemId = SID_ATTR_NUMBERING_RULE;
    const SfxPoolItem* pTmpItem = GetNumBulletItem( aNewAttr, nNumItemId );
    std::unique_ptr<SvxNumRule> pNumRule;
    if ( pTmpItem )
    {
        pNumRule.reset(new SvxNumRule(static_cast<const SvxNumBulletItem*>(pTmpItem)->GetNumRule()));

        // get numbering rule corresponding to <nIdx> and apply the needed number formats to <pNumRule>
        NBOTypeMgrBase* pNumRuleMgr =
            NBOutlineTypeMgrFact::CreateInstance(
                nSId == FN_SVX_SET_BULLET ? NBOType::Bullets : NBOType::Numbering );
        if ( pNumRuleMgr )
        {
            sal_uInt16 nActNumLvl = sal_uInt16(0xFFFF);
            const SfxPoolItem* pNumLevelItem = nullptr;
            if(SfxItemState::SET == aNewAttr.GetItemState(SID_PARAM_CUR_NUM_LEVEL, false, &pNumLevelItem))
                nActNumLvl = static_cast<const SfxUInt16Item*>(pNumLevelItem)->GetValue();

            pNumRuleMgr->SetItems(&aNewAttr);
            SvxNumRule aTmpRule( *pNumRule );
            if ( nSId == FN_SVX_SET_BULLET && bToggle && nIdx==0 )
            {
                // for toggling bullets get default numbering rule
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
                    const SvxNumberFormat& aFmt(aTmpRule.GetLevel(i));
                    pNumRule->SetLevel(i, aFmt);
                }
                nMask <<= 1;
            }
        }
    }

    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
    std::unique_ptr<OutlineViewModelChangeGuard, o3tl::default_delete<OutlineViewModelChangeGuard>> aGuard;
    if (OutlineView* pView = dynamic_cast<OutlineView*>(mpView))
    {
        pOLV = pView->GetViewByWindow(mpViewShell->GetActiveWindow());
        aGuard.reset(new OutlineViewModelChangeGuard(*pView));
    }

    SdrOutliner* pOwner = bInMasterView ? mpView->GetTextEditOutliner() : nullptr;
    const bool bOutlinerUndoEnabled = pOwner && !pOwner->IsInUndo() && pOwner->IsUndoEnabled();
    SdrModel* pSdrModel = bInMasterView ? mpView->GetModel() : nullptr;
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
        pOLV->ToggleBulletsNumbering( bToggle, nSId == FN_SVX_SET_BULLET, bInMasterView ? nullptr : pNumRule.get() );
    }
    else
    {
        mpView->ChangeMarkedObjectsBulletsNumbering( bToggle, nSId == FN_SVX_SET_BULLET, bInMasterView ? nullptr : pNumRule.get() );
    }

    if (bInMasterView && pNumRule)
    {
        SfxItemSetFixed<EE_ITEMS_START, EE_ITEMS_END> aSetAttr( mpViewShell->GetPool() );
        aSetAttr.Put(SvxNumBulletItem( *pNumRule, nNumItemId ));
        mpView->SetAttributes(aSetAttr);
    }

    if( bOutlinerUndoEnabled )
    {
        pOwner->UndoActionEnd();
    }
    else if ( bModelUndoEnabled )
    {
        pSdrModel->EndUndo();
    }

    pNumRule.reset();
    rReq.Done();
}

const SfxPoolItem* FuBulletAndPosition::GetNumBulletItem(SfxItemSet& aNewAttr, sal_uInt32& nNumItemId)
{
    //SvxNumBulletItem* pRetItem = NULL;
    const SfxPoolItem* pTmpItem = nullptr;

    if(aNewAttr.GetItemState(nNumItemId, false, &pTmpItem) == SfxItemState::SET)
    {
        return pTmpItem;
    }
    else
    {
        nNumItemId = aNewAttr.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        SfxItemState eState = aNewAttr.GetItemState(nNumItemId, false, &pTmpItem);
        if (eState == SfxItemState::SET)
            return pTmpItem;
        else
        {
            bool bOutliner = false;
            bool bTitle = false;

            if( mpView )
            {
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                const size_t nCount = rMarkList.GetMarkCount();

                for(size_t nNum = 0; nNum < nCount; ++nNum)
                {
                    SdrObject* pObj = rMarkList.GetMark(nNum)->GetMarkedSdrObj();
                    if( pObj->GetObjInventor() == SdrInventor::Default )
                    {
                        switch(pObj->GetObjIdentifier())
                        {
                        case OBJ_TITLETEXT:
                            bTitle = true;
                            break;
                        case OBJ_OUTLINETEXT:
                            bOutliner = true;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }

            const SvxNumBulletItem *pItem = nullptr;
            if(bOutliner)
            {
                SfxStyleSheetBasePool* pSSPool = mpView->GetDocSh()->GetStyleSheetPool();
                SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( STR_LAYOUT_OUTLINE + " 1", SfxStyleFamily::Pseudo);
                if( pFirstStyleSheet )
                    pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, reinterpret_cast<const SfxPoolItem**>(&pItem));
            }

            if( pItem == nullptr )
                pItem = aNewAttr.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET);

            //DBG_ASSERT( pItem, "No EE_PARA_NUMBULLET in the Pool!" );

            std::unique_ptr<SfxPoolItem> pNewItem(pItem->CloneSetWhich(EE_PARA_NUMBULLET));
            aNewAttr.Put(*pNewItem);

            if(bTitle && aNewAttr.GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET )
            {
                const SvxNumBulletItem* pBulletItem = aNewAttr.GetItem(EE_PARA_NUMBULLET);
                const SvxNumRule& rLclRule = pBulletItem->GetNumRule();
                SvxNumRule aNewRule( rLclRule );
                aNewRule.SetFeatureFlag( SvxNumRuleFlags::NO_NUMBERS );

                SvxNumBulletItem aNewItem( std::move(aNewRule), EE_PARA_NUMBULLET );
                aNewAttr.Put(aNewItem);
            }

            SfxItemState eItemState = aNewAttr.GetItemState(nNumItemId, false, &pTmpItem);
            if (eItemState == SfxItemState::SET)
                return pTmpItem;

        }
        //DBG_ASSERT(eState == SfxItemState::SET, "No item found");
    }
    return pTmpItem;
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
