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

#include <hintids.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/brushitem.hxx>
#include <numrule.hxx>

#include "cmdid.h"
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "wdocsh.hxx"
#include "textsh.hxx"
#include "uiitems.hxx"
#include "swabstdlg.hxx"
#include <globals.hrc>
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
        SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, FN_PARAM_1 , false );
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
            SfxBindings& rBindings = GetView().GetViewFrame()->GetBindings();
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
        SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, FN_PARAM_1 , false );
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
            SfxBindings& rBindings = GetView().GetViewFrame()->GetBindings();
            SfxBoolItem aItem(FN_NUM_BULLET_ON,!bNewResult);
            rBindings.SetState(aItem);
            SfxBoolItem aNewItem(FN_NUM_BULLET_ON,bNewResult);
            rBindings.SetState(aNewItem);
        }
        GetShell().EndAllAction();
    }
    break;

    case FN_NUMBER_BULLETS:
    case SID_OUTLINE_BULLET:
    {
        SfxItemSet aSet( GetPool(),
                         SID_HTML_MODE, SID_HTML_MODE,
                         SID_ATTR_NUMBERING_RULE, SID_PARAM_CUR_NUM_LEVEL,
                         0 );
        SwDocShell* pDocSh = GetView().GetDocShell();
        const bool bHtml = 0 != PTR_CAST( SwWebDocShell, pDocSh );
        const SwNumRule* pNumRuleAtCurrentSelection = GetShell().GetNumRuleAtCurrentSelection();
        if ( pNumRuleAtCurrentSelection != NULL )
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
                        aFormat.SetNumberingType(SvxExtNumType(SVX_NUM_BITMAP|LINK_TOKEN));
                    aRule.SetLevel(i, aFormat, aRule.Get(i) != 0);
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
            const bool bRightToLeft = GetShell().IsInRightToLeftText( 0 );

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
                        aFormat.SetNumAdjust( SVX_ADJUST_RIGHT );
                    }
                    aSvxRule.SetLevel( n, aFormat, false );
                }
                aSvxRule.SetFeatureFlag(SvxNumRuleFlags::ENABLE_EMBEDDED_BMP, false);
            }
            aSet.Put( SvxNumBulletItem( aSvxRule ) );
        }

        aSet.Put( SfxBoolItem( SID_PARAM_NUM_PRESET,false ));

        // Before the dialogue of the HTML mode will be dropped at the Docshell.
        pDocSh->PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(pDocSh)));

        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialog creation failed!");
        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSwTabDialog( DLG_SVXTEST_NUM_BULLET,
                                                        GetView().GetWindow(), &aSet, GetShell()));
        OSL_ENSURE(pDlg, "Dialog creation failed!");
        SFX_REQUEST_ARG( rReq, pPageItem, SfxStringItem, FN_PARAM_1, false );
        if ( pPageItem )
            pDlg->SetCurPageId( OUStringToOString( pPageItem->GetValue(), RTL_TEXTENCODING_UTF8 ) );
        const short nRet = pDlg->Execute();
        const SfxPoolItem* pItem;
        if ( RET_OK == nRet )
        {
            if( SfxItemState::SET == pDlg->GetOutputItemSet()->GetItemState( SID_ATTR_NUMBERING_RULE, false, &pItem ))
            {
                rReq.AppendItem( *pItem );
                rReq.Done();
                SvxNumRule* pSetRule = static_cast<const SvxNumBulletItem*>( pItem )->GetNumRule();
                pSetRule->UnLinkGraphics();
                SwNumRule aSetRule( pNumRuleAtCurrentSelection != NULL
                                    ? pNumRuleAtCurrentSelection->GetName()
                                    : GetShell().GetUniqueNumRuleName(),
                    numfunc::GetDefaultPositionAndSpaceMode() );
                aSetRule.SetSvxRule( *pSetRule, GetShell().GetDoc() );
                aSetRule.SetAutoRule( true );
                // No start of new list, if an existing list style is edited.
                // Otherwise start a new list.
                const bool bCreateList = ( pNumRuleAtCurrentSelection == NULL );
                GetShell().SetCurNumRule( aSetRule, bCreateList );
            }
            // If the Dialog was leaved with OK but nothing was chosen then the
            // numbering must be at least activated, if it is not already.
            else if ( pNumRuleAtCurrentSelection == NULL
                      && SfxItemState::SET == aSet.GetItemState( SID_ATTR_NUMBERING_RULE, false, &pItem ) )
            {
                rReq.AppendItem( *pItem );
                rReq.Done();
                SvxNumRule* pSetRule = static_cast<const SvxNumBulletItem*>( pItem )->GetNumRule();
                SwNumRule aSetRule(
                    GetShell().GetUniqueNumRuleName(),
                    numfunc::GetDefaultPositionAndSpaceMode() );
                aSetRule.SetSvxRule( *pSetRule, GetShell().GetDoc() );
                aSetRule.SetAutoRule( true );
                // start new list
                GetShell().SetCurNumRule( aSetRule, true );
            }
        }
        else if ( RET_USER == nRet )
            GetShell().DelNumRules();
    }
        break;

    default:
        OSL_FAIL("wrong dispatcher");
        return;
    }
}


void SwTextShell::ExecSetNumber(SfxRequest &rReq)
{
    const sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
    case FN_SVX_SET_NUMBER:
    case FN_SVX_SET_BULLET:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, nSlot, false );
            if ( pItem != NULL )
            {
                const sal_uInt16 nChoosenItemIdx = pItem->GetValue();
                svx::sidebar::NBOTypeMgrBase* pNBOTypeMgr =
                    nSlot == FN_SVX_SET_NUMBER
                        ? svx::sidebar::NBOutlineTypeMgrFact::CreateInstance( svx::sidebar::eNBOType::NUMBERING )
                        : svx::sidebar::NBOutlineTypeMgrFact::CreateInstance( svx::sidebar::eNBOType::BULLETS );
                if ( pNBOTypeMgr != NULL )
                {
                    const SwNumRule* pNumRuleAtCurrentSelection = GetShell().GetNumRuleAtCurrentSelection();
                    sal_uInt16 nActNumLvl = USHRT_MAX;
                    if ( pNumRuleAtCurrentSelection != NULL )
                    {
                        const sal_uInt16 nLevel = GetShell().GetNumLevel();
                        if ( nLevel < MAXLEVEL )
                        {
                            nActNumLvl = 1 << nLevel;
                        }
                    }
                    SwNumRule aNewNumRule(
                        pNumRuleAtCurrentSelection != NULL ? pNumRuleAtCurrentSelection->GetName() : GetShell().GetUniqueNumRuleName(),
                        numfunc::GetDefaultPositionAndSpaceMode() );
                    SvxNumRule aNewSvxNumRule = pNumRuleAtCurrentSelection != NULL
                                                    ? pNumRuleAtCurrentSelection->MakeSvxNumRule()
                                                    : aNewNumRule.MakeSvxNumRule();
                    // set unit attribute to NB Manager
                    SfxItemSet aSet( GetPool(), SID_ATTR_NUMBERING_RULE, SID_PARAM_CUR_NUM_LEVEL, 0 );
                    aSet.Put( SvxNumBulletItem( aNewSvxNumRule ) );
                    pNBOTypeMgr->SetItems( &aSet );
                    pNBOTypeMgr->ApplyNumRule( aNewSvxNumRule, nChoosenItemIdx - 1, nActNumLvl );

                    aNewNumRule.SetSvxRule( aNewSvxNumRule, GetShell().GetDoc() );
                    aNewNumRule.SetAutoRule( true );
                    const bool bCreateNewList = ( pNumRuleAtCurrentSelection == NULL );
                    GetShell().SetCurNumRule( aNewNumRule, bCreateNewList );
                }
            }
        }
        break;

    default:
        OSL_ENSURE(false, "wrong Dispatcher");
        return;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
