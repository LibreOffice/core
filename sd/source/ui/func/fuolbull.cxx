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

FunctionReference FuOutlineBullet::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuOutlineBullet( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuOutlineBullet::DoExecute( SfxRequest& rReq )
{
    sal_uInt16 nSId = rReq.GetSlot();
    if (nSId == FN_SVX_SET_BULLET){
        SetCurrentBullet(rReq);
        return;
    }
    else if (nSId == FN_SVX_SET_NUMBER){
        SetCurrentNumbering(rReq);
        return;
    }

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        // fill ItemSet for Dialog
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );

        SfxItemSet aNewAttr( mpViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );
        aNewAttr.Put( aEditAttr, sal_False );

        // create and execute dialog
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
    // invalidate possible affected fields
    mpViewShell->Invalidate( FN_NUM_BULLET_ON );
*/
}

void FuOutlineBullet::SetCurrentNumbering(SfxRequest& rReq)
{
    if (!mpDoc || !mpView)
        return;

    SfxItemSet aEditAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aEditAttr );

    SfxItemSet aNewAttr( mpViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );
    aNewAttr.Put( aEditAttr, sal_False );

    SfxItemSet aSetAttr( mpViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );

    //Init bullet level in "Customize" tab page in bullet dialog in master page view
    if( mpView && mpViewShell && mpViewShell->ISA(DrawViewShell)
        && ((DrawViewShell *)mpViewShell)->GetEditMode() == EM_MASTERPAGE )
    {
        SdrObject* pObj = mpView->GetTextEditObject();
        if( pObj && pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
        {
            sal_uInt16 nLevel = mpView->GetSelectionLevel();
            if( nLevel != 0xFFFF )
            {

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
    //End of add

    sal_uInt16 nActNumLvl = (sal_uInt16)0xFFFF;
    SvxNumRule* pNumRule = NULL;
    const SfxPoolItem* pTmpItem=NULL;
    sal_uInt32 nNumItemId = SID_ATTR_NUMBERING_RULE;

    if(SFX_ITEM_SET == aNewAttr.GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pTmpItem))
        nActNumLvl = ((const SfxUInt16Item*)pTmpItem)->GetValue();

    pTmpItem=GetNumBulletItem(aNewAttr, nNumItemId);

    if (pTmpItem)
        pNumRule = new SvxNumRule(*((SvxNumBulletItem*)pTmpItem)->GetNumRule());

    SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, FN_SVX_SET_NUMBER , sal_False );
    if (pItem && pNumRule)
    {
        sal_uInt16 nIdx = pItem->GetValue();
        // If the nIdx is (sal_uInt16)0xFFFF, means set bullet status to on/off
        // And the bullet default status is 1.
        bool bBulletSwitch = false;
        sal_Bool isRemoveNum =false;
        if( nIdx == (sal_uInt16)0xFFFF )
        {
            nIdx = 1;
            bBulletSwitch = true;
        }
        if (nIdx == DEFAULT_NONE)
        {
            bBulletSwitch = false;
            isRemoveNum = true;
        }
        nIdx--;

        NBOTypeMgrBase* pNumbering = NBOutlineTypeMgrFact::CreateInstance(eNBOType::NUMBERING);
        if ( pNumbering )
        {
            //Sym3_2508, set unit attribute to NB Manager
            pNumbering->SetItems(&aNewAttr);
            SvxNumRule aTmpRule( *pNumRule );
            pNumbering->ApplyNumRule(aTmpRule,nIdx,nActNumLvl);
            sal_uInt16 nMask = 1;
            for(sal_uInt16 i = 0; i < pNumRule->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                {
                    SvxNumberFormat aFmt(aTmpRule.GetLevel(i));
                    pNumRule->SetLevel(i, aFmt);
                }
                nMask <<= 1 ;
            }
            aSetAttr.Put(SvxNumBulletItem( *pNumRule ), nNumItemId);
            OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

            boost::scoped_ptr< OutlineViewModelChangeGuard > aGuard;

            if (mpView->ISA(OutlineView))
            {
                pOLV = static_cast<OutlineView*>(mpView)
                    ->GetViewByWindow(mpViewShell->GetActiveWindow());

                aGuard.reset( new OutlineViewModelChangeGuard( static_cast<OutlineView&>(*mpView) ) );
            }

            SdrOutliner* pOwner = mpView->GetTextEditOutliner();
            bool bMasterView = false;

            DrawViewShell* pDrawViewShell = static_cast< DrawViewShell* >(mpViewShell);

            if ( pOwner && pDrawViewShell && pDrawViewShell->GetEditMode() == EM_MASTERPAGE )
                bMasterView = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();

            if( bMasterView )
            {
                pOwner->UndoActionStart( OLUNDO_ATTR );
                pOLV->ToggleBullets( bBulletSwitch, sal_False, bMasterView, pNumRule,isRemoveNum);
                mpView->SetAttributes(aSetAttr); //Modify for Sym2_3151
                pOwner->UndoActionEnd( OLUNDO_ATTR );
            }
            else if( pOLV )
                pOLV->ToggleBullets( bBulletSwitch, sal_False, bMasterView, pNumRule ,isRemoveNum);
            else
            {
                sal_Bool bInMasterView = pDrawViewShell && pDrawViewShell->GetEditMode() == EM_MASTERPAGE;
                SdrModel* pSdrModel = mpView->GetModel();
                sal_Bool bModelUndoEnabled = pSdrModel ? pSdrModel->IsUndoEnabled() : sal_False;
                if (bInMasterView && bModelUndoEnabled)
                {
                    pSdrModel->BegUndo();
                }
                mpView->ToggleMarkedObjectsBullets(bBulletSwitch, sal_False, bInMasterView, pNumRule,isRemoveNum);
                if (bInMasterView)
                {
                    mpView->SetAttributes(aSetAttr);
                }
                if (bInMasterView && bModelUndoEnabled)
                {
                    pSdrModel->EndUndo();
                }
            }
        }
        //End
    }
    delete pNumRule;
    rReq.Done();
}

void FuOutlineBullet::SetCurrentBullet(SfxRequest& rReq)
{
    if (!mpDoc || !mpView)
        return;

    SfxItemSet aEditAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aEditAttr );

    SfxItemSet aNewAttr( mpViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );
    aNewAttr.Put( aEditAttr, sal_False );

    //Add for Sym2_3151, should add new attributes in an empty item set, then use this item set as parameter in SetAttributes()
    SfxItemSet aSetAttr( mpViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );

    //Init bullet level in "Customize" tab page in bullet dialog in master page view
    if( mpView && mpViewShell && mpViewShell->ISA(DrawViewShell)
        && ((DrawViewShell *)mpViewShell)->GetEditMode() == EM_MASTERPAGE )
    {
        SdrObject* pObj = mpView->GetTextEditObject();
        if( pObj && pObj->GetObjIdentifier() == OBJ_OUTLINETEXT )
        {
            sal_uInt16 nLevel = mpView->GetSelectionLevel();
            if( nLevel != 0xFFFF )
            {
                //aNewAttr.MergeRange( SID_ATTR_NUMBERING_RULE, SID_PARAM_CUR_NUM_LEVEL );
                //aNewAttr.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, nLevel ) );
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
    //End of add

    sal_uInt16 nActNumLvl = (sal_uInt16)0xFFFF;
    SvxNumRule* pNumRule = NULL;
    const SfxPoolItem* pTmpItem=NULL;
    sal_uInt32 nNumItemId = SID_ATTR_NUMBERING_RULE;

    if(SFX_ITEM_SET == aNewAttr.GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pTmpItem))
        nActNumLvl = ((const SfxUInt16Item*)pTmpItem)->GetValue();

    pTmpItem=GetNumBulletItem(aNewAttr, nNumItemId);

    if (pTmpItem)
        pNumRule = new SvxNumRule(*((SvxNumBulletItem*)pTmpItem)->GetNumRule());

    SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, FN_SVX_SET_BULLET , sal_False );
    if (pItem && pNumRule)
    {
        sal_uInt16 nIdx = pItem->GetValue();
        // If the nIdx is (sal_uInt16)0xFFFF, means set bullet status to on/off
        // And the bullet default status is 2.
        bool bBulletSwitch = false;
        sal_Bool isRemoveNum =false;
        if( nIdx == (sal_uInt16)0xFFFF )
        {
            nIdx = 1;
            bBulletSwitch = true;
        }
        if (nIdx == DEFAULT_NONE)
        {
            bBulletSwitch = false;
            isRemoveNum = true;
        }

        nIdx--;
        //Modified for Numbering&Bullets Dialog UX Enh(Story 992) by chengjh,2011.8.7

        NBOTypeMgrBase* pBullets = NBOutlineTypeMgrFact::CreateInstance(eNBOType::MIXBULLETS);
        if ( pBullets )
        {
            //Sym3_2508, set unit attribute to NB Manager
            pBullets->SetItems(&aNewAttr);
            SvxNumRule aTmpRule( *pNumRule );
            //Sym3_3423 Always apply the "." if wants a default numbering rule
            if (bBulletSwitch==true && nIdx==0) //want to reset bullet
            {
                pBullets->ApplyNumRule(aTmpRule,nIdx,nActNumLvl,true);
            }
            else {
                pBullets->ApplyNumRule(aTmpRule,nIdx,nActNumLvl);
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
            aSetAttr.Put(SvxNumBulletItem( *pNumRule ), nNumItemId);

            OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

            boost::scoped_ptr< OutlineViewModelChangeGuard > aGuard;

            if (mpView->ISA(OutlineView))
            {
                pOLV = static_cast<OutlineView*>(mpView)
                    ->GetViewByWindow(mpViewShell->GetActiveWindow());

                aGuard.reset( new OutlineViewModelChangeGuard( static_cast<OutlineView&>(*mpView) ) );
            }

            SdrOutliner* pOwner = mpView->GetTextEditOutliner();
            bool bMasterView = false;

            DrawViewShell* pDrawViewShell = static_cast< DrawViewShell* >(mpViewShell);

            if ( pOwner && pDrawViewShell && pDrawViewShell->GetEditMode() == EM_MASTERPAGE )
                bMasterView = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();

            if( bMasterView )
            {
                pOwner->UndoActionStart( OLUNDO_ATTR );
                pOLV->ToggleBullets( bBulletSwitch, sal_True, bMasterView, pNumRule, isRemoveNum );
                mpView->SetAttributes(aSetAttr); //Modify for Sym2_3151
                pOwner->UndoActionEnd( OLUNDO_ATTR );
            }
            else if( pOLV )
                pOLV->ToggleBullets( bBulletSwitch, sal_True, bMasterView, pNumRule, isRemoveNum );
            else
            {
                sal_Bool bInMasterView = pDrawViewShell && pDrawViewShell->GetEditMode() == EM_MASTERPAGE;
                SdrModel* pSdrModel = mpView->GetModel();
                sal_Bool bModelUndoEnabled = pSdrModel ? pSdrModel->IsUndoEnabled() : sal_False;
                if (bInMasterView && bModelUndoEnabled)
                {
                    pSdrModel->BegUndo();
                }
                mpView->ToggleMarkedObjectsBullets(bBulletSwitch, sal_True, bInMasterView, pNumRule, isRemoveNum );
                if (bInMasterView)
                {
                    mpView->SetAttributes(aSetAttr);
                }
                if (bInMasterView && bModelUndoEnabled)
                {
                    pSdrModel->EndUndo();
                }
            }
        }
        //End
    }
    delete pNumRule;
    rReq.Done();
}

const SfxPoolItem* FuOutlineBullet::GetNumBulletItem(SfxItemSet& aNewAttr, sal_uInt32& nNumItemId)
{
    //SvxNumBulletItem* pRetItem = NULL;
    const SfxPoolItem* pTmpItem = NULL;

    if(aNewAttr.GetItemState(nNumItemId, sal_False, &pTmpItem) == SFX_ITEM_SET)
    {
        return pTmpItem;
    }
    else
    {
        nNumItemId = aNewAttr.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        SfxItemState eState = aNewAttr.GetItemState(nNumItemId, sal_False, &pTmpItem);
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
                String aStyleName((SdResId((sal_uInt16)STR_LAYOUT_OUTLINE)));
                aStyleName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " 1" ) );
                SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( aStyleName, SD_STYLE_FAMILY_PSEUDO);
                if( pFirstStyleSheet )
                    pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, sal_False, (const SfxPoolItem**)&pItem);
            }

            if( pItem == NULL )
                pItem = (SvxNumBulletItem*) aNewAttr.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET);

            //DBG_ASSERT( pItem, "Kein EE_PARA_NUMBULLET im Pool! [CL]" );

            aNewAttr.Put(*pItem, EE_PARA_NUMBULLET);

            if(bTitle && aNewAttr.GetItemState(EE_PARA_NUMBULLET,sal_True) == SFX_ITEM_ON )
            {
                SvxNumBulletItem* pBulletItem = (SvxNumBulletItem*)aNewAttr.GetItem(EE_PARA_NUMBULLET,sal_True);
                SvxNumRule* pLclRule = pBulletItem->GetNumRule();
                if(pLclRule)
                {
                    SvxNumRule aNewRule( *pLclRule );
                    aNewRule.SetFeatureFlag( NUM_NO_NUMBERS, sal_True );

                    SvxNumBulletItem aNewItem( aNewRule, EE_PARA_NUMBULLET );
                    aNewAttr.Put(aNewItem);
                }
            }

            SfxItemState eItemState = aNewAttr.GetItemState(nNumItemId, sal_False, &pTmpItem);
            if (eItemState == SFX_ITEM_SET)
                return pTmpItem;

        }
        //DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!")
    }
    return pTmpItem;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
