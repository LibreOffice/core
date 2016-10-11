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
#include <memory>
using namespace svx::sidebar;
namespace sd {


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
    const SfxStringItem* pPageItem = SfxItemSet::GetItem<SfxStringItem>(pArgs, FN_PARAM_1, false);

    if ( !pArgs || pPageItem )
    {
        // fill ItemSet for Dialog
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );

        SfxItemSet aNewAttr( mpViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );
        aNewAttr.Put( aEditAttr, false );

        // create and execute dialog
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact ? pFact->CreateSdOutlineBulletTabDlg(mpViewShell->GetActiveWindow(), &aNewAttr, mpView) : nullptr);
        if( pDlg )
        {
            if ( pPageItem )
                pDlg->SetCurPageId( OUStringToOString( pPageItem->GetValue(), RTL_TEXTENCODING_UTF8 ) );
            sal_uInt16 nResult = pDlg->Execute();

            switch( nResult )
            {
                case RET_OK:
                {
                    SfxItemSet aSet( *pDlg->GetOutputItemSet() );

                    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

                    std::unique_ptr< OutlineViewModelChangeGuard > aGuard;

                    if( dynamic_cast< const OutlineView *>( mpView ) !=  nullptr)
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
                    return;
            }
        }
    }

    /* not direct to pOlView; therefore, SdDrawView::SetAttributes can catch
       changes to master page and redirect to a template */
    mpView->SetAttributes(*pArgs);

/* #i35937#
    // invalidate possible affected fields
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
        // unexpected SfxRequest
        return;
    }

    const SfxUInt16Item* pItem = rReq.GetArg<SfxUInt16Item>(nSId);
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
    if( nIdx == (sal_uInt16)0xFFFF )
    {
        // If the nIdx is (sal_uInt16)0xFFFF, means set bullet status to on/off
        nIdx = 1;
        bToggle = true;
    }
    nIdx--;

    sal_uInt32 nNumItemId = SID_ATTR_NUMBERING_RULE;
    const SfxPoolItem* pTmpItem = GetNumBulletItem( aNewAttr, nNumItemId );
    SvxNumRule* pNumRule = nullptr;
    if ( pTmpItem )
    {
        pNumRule = new SvxNumRule(*static_cast<const SvxNumBulletItem*>(pTmpItem)->GetNumRule());

        // get numbering rule corresponding to <nIdx> and apply the needed number formats to <pNumRule>
        NBOTypeMgrBase* pNumRuleMgr =
            NBOutlineTypeMgrFact::CreateInstance(
                nSId == FN_SVX_SET_BULLET ? NBOType::Bullets : NBOType::Numbering );
        if ( pNumRuleMgr )
        {
            sal_uInt16 nActNumLvl = (sal_uInt16)0xFFFF;
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
                    SvxNumberFormat aFmt(aTmpRule.GetLevel(i));
                    pNumRule->SetLevel(i, aFmt);
                }
                nMask <<= 1;
            }
        }
    }

    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();
    std::unique_ptr< OutlineViewModelChangeGuard > aGuard;
    {
        if( dynamic_cast< const OutlineView *>( mpView ) !=  nullptr)
        {
            pOLV = static_cast<OutlineView*>(mpView)
                ->GetViewByWindow(mpViewShell->GetActiveWindow());

            aGuard.reset( new OutlineViewModelChangeGuard( static_cast<OutlineView&>(*mpView) ) );
        }
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
        pOLV->ToggleBulletsNumbering( bToggle, nSId == FN_SVX_SET_BULLET, bInMasterView ? nullptr : pNumRule );
    }
    else
    {
        mpView->ChangeMarkedObjectsBulletsNumbering( bToggle, nSId == FN_SVX_SET_BULLET, bInMasterView ? nullptr : pNumRule );
    }
    if ( bInMasterView )
    {
        SfxItemSet aSetAttr( mpViewShell->GetPool(), EE_ITEMS_START, EE_ITEMS_END );
        aSetAttr.Put(SvxNumBulletItem( *pNumRule, nNumItemId ));
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
                    if( pObj->GetObjInventor() == SdrInventor )
                    {
                        switch(pObj->GetObjIdentifier())
                        {
                        case OBJ_TITLETEXT:
                            bTitle = true;
                            break;
                        case OBJ_OUTLINETEXT:
                            bOutliner = true;
                            break;
                        }
                    }
                }
            }

            const SvxNumBulletItem *pItem = nullptr;
            if(bOutliner)
            {
                SfxStyleSheetBasePool* pSSPool = mpView->GetDocSh()->GetStyleSheetPool();
                OUString aStyleName(SD_RESSTR(STR_LAYOUT_OUTLINE) + " 1");
                SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);
                if( pFirstStyleSheet )
                    pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, reinterpret_cast<const SfxPoolItem**>(&pItem));
            }

            if( pItem == nullptr )
                pItem = static_cast<const SvxNumBulletItem*>( aNewAttr.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET) );

            //DBG_ASSERT( pItem, "No EE_PARA_NUMBULLET in the Pool!" );

            std::unique_ptr<SfxPoolItem> pNewItem(pItem->CloneSetWhich(EE_PARA_NUMBULLET));
            aNewAttr.Put(*pNewItem);

            if(bTitle && aNewAttr.GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET )
            {
                const SvxNumBulletItem* pBulletItem = static_cast<const SvxNumBulletItem*>( aNewAttr.GetItem(EE_PARA_NUMBULLET) );
                SvxNumRule* pLclRule = pBulletItem->GetNumRule();
                if(pLclRule)
                {
                    SvxNumRule aNewRule( *pLclRule );
                    aNewRule.SetFeatureFlag( SvxNumRuleFlags::NO_NUMBERS );

                    SvxNumBulletItem aNewItem( aNewRule, EE_PARA_NUMBULLET );
                    aNewAttr.Put(aNewItem);
                }
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
