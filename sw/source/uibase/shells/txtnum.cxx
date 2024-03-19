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

#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/brushitem.hxx>
#include <osl/diagnose.h>
#include <numrule.hxx>

#include <cmdid.h>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wdocsh.hxx>
#include <poolfmt.hxx>
#include <textsh.hxx>
#include <swabstdlg.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/nbdtmg.hxx>
#include <svx/nbdtmgfact.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <memory>

void SwTextShell::ExecEnterNum(SfxRequest &rReq)
{
    //Because the record before any shell exchange.
    switch(rReq.GetSlot())
    {
    case FN_NUM_NUMBERING_ON:
    {
        GetShell().StartAllAction();
        const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_1);
        bool bMode = !GetShell().SelectionHasNumber(); // #i29560#
        if ( pItem )
            bMode = pItem->GetValue();
        else
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bMode ) );

        if ( bMode != (GetShell().SelectionHasNumber()) ) // #i29560#
        {
            rReq.Done();
            if( bMode )
                GetShell().NumOn();
            else
                GetShell().NumOrBulletOff(); // #i29560#
        }
        bool bNewResult = GetShell().SelectionHasNumber();
        if (bNewResult!=bMode) {
            SfxBindings& rBindings = GetView().GetViewFrame().GetBindings();
            SfxBoolItem aItem(FN_NUM_NUMBERING_ON,!bNewResult);
            rBindings.SetState(aItem);
            SfxBoolItem aNewItem(FN_NUM_NUMBERING_ON,bNewResult);
            rBindings.SetState(aNewItem);
        }
        GetShell().EndAllAction();
    }
    break;
    case FN_NUM_BULLET_ON:
    {
        GetShell().StartAllAction();
        const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_1);
        bool bMode = !GetShell().SelectionHasBullet(); // #i29560#
        if ( pItem )
            bMode = pItem->GetValue();
        else
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bMode ) );

        if ( bMode != (GetShell().SelectionHasBullet()) ) // #i29560#
        {
            rReq.Done();
            if( bMode )
                GetShell().BulletOn();
            else
                GetShell().NumOrBulletOff(); // #i29560#
        }
        bool bNewResult = GetShell().SelectionHasBullet();
        if (bNewResult!=bMode) {
            SfxBindings& rBindings = GetView().GetViewFrame().GetBindings();
            SfxBoolItem aItem(FN_NUM_BULLET_ON,!bNewResult);
            rBindings.SetState(aItem);
            SfxBoolItem aNewItem(FN_NUM_BULLET_ON,bNewResult);
            rBindings.SetState(aNewItem);
        }
        GetShell().EndAllAction();
    }
    break;

    case FN_NUM_BULLET_OFF:
    {
        GetShell().StartAllAction();
        SfxRequest aReq(GetView().GetViewFrame(), FN_NUM_BULLET_ON);
        aReq.AppendItem(SfxBoolItem(FN_PARAM_1, false));
        aReq.Done();
        GetShell().DelNumRules();
        GetShell().EndAllAction();
    }
    break;

    case FN_NUMBER_BULLETS:
    case SID_OUTLINE_BULLET:
    {
        SfxItemSetFixed<SID_HTML_MODE, SID_HTML_MODE,
                     SID_ATTR_NUMBERING_RULE, SID_PARAM_CUR_NUM_LEVEL>  aSet( GetPool() );
        SwDocShell* pDocSh = GetView().GetDocShell();
        const bool bHtml = dynamic_cast<SwWebDocShell*>( pDocSh  ) !=  nullptr;
        const SwNumRule* pNumRuleAtCurrentSelection = GetShell().GetNumRuleAtCurrentSelection();
        if ( pNumRuleAtCurrentSelection != nullptr )
        {
            SvxNumRule aRule = pNumRuleAtCurrentSelection->MakeSvxNumRule();

            //convert type of linked bitmaps from SVX_NUM_BITMAP to (SVX_NUM_BITMAP|LINK_TOKEN)
            for ( sal_uInt16 i = 0; i < aRule.GetLevelCount(); i++ )
            {
                SvxNumberFormat aFormat( aRule.GetLevel( i ) );
                if ( SVX_NUM_BITMAP == aFormat.GetNumberingType() )
                {
                    const SvxBrushItem* pBrush = aFormat.GetBrush();
                    if(pBrush && !pBrush->GetGraphicLink().isEmpty())
                        aFormat.SetNumberingType(SvxNumType(SVX_NUM_BITMAP|LINK_TOKEN));
                    aRule.SetLevel(i, aFormat, aRule.Get(i) != nullptr);
                }
            }
            if(bHtml)
                aRule.SetFeatureFlag(SvxNumRuleFlags::ENABLE_EMBEDDED_BMP, false);

            aSet.Put(SvxNumBulletItem(aRule));
            OSL_ENSURE( GetShell().GetNumLevel() < MAXLEVEL,
                    "<SwTextShell::ExecEnterNum()> - numbered node without valid list level. Serious defect." );
            sal_uInt16 nLevel = GetShell().GetNumLevel();
            if( nLevel < MAXLEVEL )
            {
                nLevel = 1 << nLevel;
                aSet.Put( SfxUInt16Item( SID_PARAM_CUR_NUM_LEVEL, nLevel ) );
            }
        }
        else
        {
            SwNumRule aRule( GetShell().GetUniqueNumRuleName(),
                             // #i89178#
                             numfunc::GetDefaultPositionAndSpaceMode() );
            SvxNumRule aSvxRule = aRule.MakeSvxNumRule();
            const bool bRightToLeft = GetShell().IsInRightToLeftText();

            if ( bHtml || bRightToLeft )
            {
                for ( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
                {
                    SvxNumberFormat aFormat( aSvxRule.GetLevel( n ) );
                    if ( n && bHtml )
                    {
                        // 1/2" for HTML
                        aFormat.SetAbsLSpace(n * 720);
                    }
                    // #i38904#  Default alignment for
                    // numbering/bullet should be rtl in rtl paragraph:
                    if ( bRightToLeft )
                    {
                        aFormat.SetNumAdjust( SvxAdjust::Right );
                    }
                    aSvxRule.SetLevel( n, aFormat, false );
                }
                aSvxRule.SetFeatureFlag(SvxNumRuleFlags::ENABLE_EMBEDDED_BMP, false);
            }
            aSet.Put( SvxNumBulletItem( std::move(aSvxRule) ) );
        }

        aSet.Put( SfxBoolItem( SID_PARAM_NUM_PRESET,false ));

        // Before the dialogue of the HTML mode will be dropped at the Docshell.
        pDocSh->PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(pDocSh)));

        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        weld::Window *pParent = rReq.GetFrameWeld();
        VclPtr<AbstractNumBulletDialog> pDlg(pFact->CreateSvxNumBulletTabDialog(pParent, aSet, GetShell()));
        const SfxStringItem* pPageItem = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
        if ( pPageItem )
            pDlg->SetCurPageId( pPageItem->GetValue() );

        auto xRequest = std::make_shared<SfxRequest>(rReq);
        rReq.Ignore(); // the 'old' request is not relevant any more

        pDlg->StartExecuteAsync([pDlg, pNumRuleAtCurrentSelection, xRequest=std::move(xRequest), this](sal_Int32 nResult){
            if (RET_OK == nResult)
            {
                const SvxNumBulletItem* pBulletItem = pDlg->GetOutputItemSet()->GetItemIfSet(SID_ATTR_NUMBERING_RULE, false);
                if (pBulletItem)
                {
                    xRequest->AppendItem(*pBulletItem);
                    xRequest->Done();
                    SvxNumRule& rSetRule = const_cast<SvxNumRule&>(pBulletItem->GetNumRule());
                    rSetRule.UnLinkGraphics();
                    SwNumRule aSetRule(pNumRuleAtCurrentSelection != nullptr
                                       ? pNumRuleAtCurrentSelection->GetName()
                                       : GetShell().GetUniqueNumRuleName(),
                        numfunc::GetDefaultPositionAndSpaceMode());
                    aSetRule.SetSvxRule(rSetRule, GetShell().GetDoc());
                    aSetRule.SetAutoRule(true);
                    // No start of new list, if an existing list style is edited.
                    // Otherwise start a new list.
                    const bool bCreateList = (pNumRuleAtCurrentSelection == nullptr);
                    GetShell().SetCurNumRule(aSetRule, bCreateList);
                }
                // If the Dialog was leaved with OK but nothing was chosen then the
                // numbering must be at least activated, if it is not already.
                else if (pNumRuleAtCurrentSelection == nullptr
                         && (pBulletItem = pDlg->GetInputItemSet()->GetItemIfSet(SID_ATTR_NUMBERING_RULE, false)))
                {
                    xRequest->AppendItem(*pBulletItem);
                    xRequest->Done();
                    const SvxNumRule& rSetRule = pBulletItem->GetNumRule();
                    SwNumRule aSetRule(
                        GetShell().GetUniqueNumRuleName(),
                        numfunc::GetDefaultPositionAndSpaceMode());
                    aSetRule.SetSvxRule(rSetRule, GetShell().GetDoc());
                    aSetRule.SetAutoRule(true);
                    // start new list
                    GetShell().SetCurNumRule(aSetRule, true);
                }
            }
            else if (RET_USER == nResult)
                GetShell().DelNumRules();
            pDlg->disposeOnce();
        });
    }
    break;

    default:
        OSL_FAIL("wrong dispatcher");
        return;
    }
}


void SwTextShell::ExecSetNumber(SfxRequest const &rReq)
{
    const sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
    case FN_SVX_SET_NUMBER:
    case FN_SVX_SET_BULLET:
    case FN_SVX_SET_OUTLINE:
        {
            const SfxUInt16Item* pItem = rReq.GetArg<SfxUInt16Item>(nSlot);
            if ( pItem != nullptr )
            {
                const sal_uInt16 nChosenItemIdx = pItem->GetValue();
                svx::sidebar::NBOType nNBOType = svx::sidebar::NBOType::Bullets;
                if ( nSlot == FN_SVX_SET_NUMBER )
                    nNBOType = svx::sidebar::NBOType::Numbering;
                else if ( nSlot == FN_SVX_SET_OUTLINE )
                    nNBOType = svx::sidebar::NBOType::Outline;

                svx::sidebar::NBOTypeMgrBase* pNBOTypeMgr = svx::sidebar::NBOutlineTypeMgrFact::CreateInstance( nNBOType );

                if ( pNBOTypeMgr != nullptr )
                {
                    const SwNumRule* pNumRuleAtCurrentSelection = GetShell().GetNumRuleAtCurrentSelection();
                    sal_uInt16 nActNumLvl = USHRT_MAX;
                    if ( pNumRuleAtCurrentSelection != nullptr )
                    {
                        const sal_uInt16 nLevel = GetShell().GetNumLevel();
                        if ( nLevel < MAXLEVEL )
                        {
                            nActNumLvl = 1 << nLevel;
                        }
                    }
                    SwNumRule aNewNumRule(
                        pNumRuleAtCurrentSelection != nullptr ? pNumRuleAtCurrentSelection->GetName() : GetShell().GetUniqueNumRuleName(),
                        numfunc::GetDefaultPositionAndSpaceMode() );
                    SvxNumRule aNewSvxNumRule = pNumRuleAtCurrentSelection != nullptr
                                                    ? pNumRuleAtCurrentSelection->MakeSvxNumRule()
                                                    : aNewNumRule.MakeSvxNumRule();

                    OUString aNumCharFormat, aBulletCharFormat;
                    SwStyleNameMapper::FillUIName( RES_POOLCHR_NUM_LEVEL, aNumCharFormat );
                    SwStyleNameMapper::FillUIName( RES_POOLCHR_BULLET_LEVEL, aBulletCharFormat );

                    SfxAllItemSet aSet( GetPool() );
                    aSet.Put( SfxStringItem( SID_NUM_CHAR_FMT, aNumCharFormat ) );
                    aSet.Put( SfxStringItem( SID_BULLET_CHAR_FMT, aBulletCharFormat ) );
                    aSet.Put( SvxNumBulletItem( aNewSvxNumRule, SID_ATTR_NUMBERING_RULE ) );

                    pNBOTypeMgr->SetItems( &aSet );
                    pNBOTypeMgr->ApplyNumRule( aNewSvxNumRule, nChosenItemIdx - 1, nActNumLvl );

                    aNewNumRule.SetSvxRule( aNewSvxNumRule, GetShell().GetDoc() );
                    aNewNumRule.SetAutoRule( true );
                    const bool bCreateNewList = ( pNumRuleAtCurrentSelection == nullptr );
                    GetShell().SetCurNumRule( aNewNumRule, bCreateNewList );
                }
            }
            else if (nSlot == FN_SVX_SET_OUTLINE)
            {
                // no outline provided: launch dialog to request a specific outline
                SfxBindings& rBindings = GetView().GetViewFrame().GetBindings();
                const SfxStringItem aPage(FN_PARAM_1, "outlinenum");
                const SfxPoolItem* aItems[] = { &aPage, nullptr };
                rBindings.Execute(SID_OUTLINE_BULLET, aItems);
            }
        }
        break;

    default:
        OSL_ENSURE(false, "wrong Dispatcher");
        return;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
