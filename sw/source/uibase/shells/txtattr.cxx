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

#include <svl/whiter.hxx>
#include <svl/stritem.hxx>
#include <svl/itemiter.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/escapementitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/cmapitem.hxx>
#include <paratr.hxx>

#include <fmtinfmt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <textsh.hxx>
#include <num.hxx>
#include <swundo.hxx>
#include <fmtcol.hxx>

#include <cmdid.h>
#include <globals.h>
#include <SwStyleNameMapper.hxx>
#include <swabstdlg.hxx>
#include <outline.hxx>
#include <memory>

const sal_uInt32 nFontInc = 40;      // 2pt
const sal_uInt32 nFontMaxSz = 19998; // 999.9pt

void SwTextShell::ExecCharAttr(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    const SfxItemSet  *pArgs   = rReq.GetArgs();
    int        eState = STATE_TOGGLE;
    sal_uInt16 nWhich = rReq.GetSlot();

    if(pArgs )
    {
        const SfxPoolItem* pItem;
        pArgs->GetItemState(nWhich, false, &pItem);
        eState =  static_cast<const SfxBoolItem &>( pArgs->
                                Get( nWhich )).GetValue() ? STATE_ON : STATE_OFF;
    }

    SfxItemSet aSet( GetPool(), svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END-1>{} );
    if (STATE_TOGGLE == eState)
        rSh.GetCurAttr( aSet );

    switch ( nWhich )
    {
        case FN_SET_SUB_SCRIPT:
        case FN_SET_SUPER_SCRIPT:
        {
            SvxEscapement eEscape = SvxEscapement::Subscript;
            switch (eState)
            {
            case STATE_TOGGLE:
            {
                short nTmpEsc = aSet.Get( RES_CHRATR_ESCAPEMENT ).GetEsc();
                eEscape = nWhich == FN_SET_SUPER_SCRIPT ?
                                SvxEscapement::Superscript:
                                SvxEscapement::Subscript;
                if( (nWhich == FN_SET_SUB_SCRIPT && nTmpEsc < 0) ||
                            (nWhich == FN_SET_SUPER_SCRIPT && nTmpEsc > 0) )
                    eEscape = SvxEscapement::Off;

                SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();
                if( nWhich == FN_SET_SUB_SCRIPT )
                    rBind.SetState( SfxBoolItem( FN_SET_SUPER_SCRIPT,
                                                                    false ) );
                else
                    rBind.SetState( SfxBoolItem( FN_SET_SUB_SCRIPT,
                                                                    false ) );

            }
            break;
            case STATE_ON:
                eEscape = nWhich == FN_SET_SUPER_SCRIPT ?
                                SvxEscapement::Superscript:
                                SvxEscapement::Subscript;
                break;
            case STATE_OFF:
                eEscape = SvxEscapement::Off;
                break;
            }
            SvxEscapementItem aEscape( eEscape, RES_CHRATR_ESCAPEMENT );
            if(eEscape == SvxEscapement::Superscript)
                aEscape.GetEsc() = DFLT_ESC_AUTO_SUPER;
            else if(eEscape == SvxEscapement::Subscript)
                aEscape.GetEsc() = DFLT_ESC_AUTO_SUB;
            rSh.SetAttrItem( aEscape );
            rReq.AppendItem( aEscape );
            rReq.Done();
        }
        break;

        case FN_SET_SMALL_CAPS:
        {
            SvxCaseMap eCaseMap = SvxCaseMap::SmallCaps;
            switch (eState)
            {
            case STATE_TOGGLE:
            {
                SvxCaseMap eTmpCaseMap = aSet.Get(RES_CHRATR_CASEMAP).GetCaseMap();
                if (eTmpCaseMap == SvxCaseMap::SmallCaps)
                    eCaseMap = SvxCaseMap::NotMapped;
            }
            break;
            case STATE_ON:
                // Nothing to do, already set.
                break;
            case STATE_OFF:
                eCaseMap = SvxCaseMap::NotMapped;
                break;
            }
            SvxCaseMapItem aCaseMap(eCaseMap, RES_CHRATR_CASEMAP);
            rSh.SetAttrItem(aCaseMap);
            rReq.AppendItem(aCaseMap);
            rReq.Done();
        }
        break;

        case FN_UPDATE_STYLE_BY_EXAMPLE:
            rSh.QuickUpdateStyle();
            rReq.Done();
            break;
        case FN_UNDERLINE_DOUBLE:
        {
            FontLineStyle eUnderline =
                            aSet.Get(RES_CHRATR_UNDERLINE).GetLineStyle();
            switch( eState )
            {
                case STATE_TOGGLE:
                    eUnderline = eUnderline == LINESTYLE_DOUBLE ?
                        LINESTYLE_NONE :
                            LINESTYLE_DOUBLE;
                break;
                case STATE_ON:
                    eUnderline = LINESTYLE_DOUBLE;
                break;
                case STATE_OFF:
                    eUnderline = LINESTYLE_NONE;
                break;
            }
            SvxUnderlineItem aUnderline(eUnderline, RES_CHRATR_UNDERLINE );
            rSh.SetAttrItem( aUnderline );
            rReq.AppendItem( aUnderline );
            rReq.Done();
        }
        break;
        case FN_REMOVE_DIRECT_CHAR_FORMATS:
            if( !rSh.HasReadonlySel() && rSh.IsEndPara())
                rSh.DontExpandFormat();
        break;
        default:
            OSL_FAIL("wrong  dispatcher");
            return;
    }
}

void SwTextShell::ExecCharAttrArgs(SfxRequest &rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    bool bArgs = pArgs != nullptr && pArgs->Count() > 0;
    SwWrtShell& rWrtSh = GetShell();
    SwTextFormatColl* pColl = nullptr;

    // Is only set if the whole paragraph is selected and AutoUpdateFormat is set.
    if (rWrtSh.HasSelection() && rWrtSh.IsSelFullPara())
    {
        pColl = rWrtSh.GetCurTextFormatColl();
        if ( pColl && !pColl->IsAutoUpdateFormat() )
            pColl = nullptr;
    }
    SfxItemPool& rPool = GetPool();
    sal_uInt16 nWhich = rPool.GetWhich( nSlot );
    switch (nSlot)
    {
        case FN_TXTATR_INET:
        // Special treatment of the PoolId of the SwFormatInetFormat
        if(bArgs)
        {
            const SfxPoolItem& rItem = pArgs->Get( nWhich );

            SwFormatINetFormat aINetFormat( static_cast<const SwFormatINetFormat&>(rItem) );
            if ( USHRT_MAX == aINetFormat.GetVisitedFormatId() )
            {
                OSL_ENSURE( false, "<SwTextShell::ExecCharAttrArgs(..)> - unexpected visited character format ID at hyperlink attribute" );
                aINetFormat.SetVisitedFormatAndId(
                        aINetFormat.GetVisitedFormat(),
                        SwStyleNameMapper::GetPoolIdFromUIName( aINetFormat.GetVisitedFormat(), SwGetPoolIdFromName::ChrFmt ) );
            }
            if ( USHRT_MAX == aINetFormat.GetINetFormatId() )
            {
                OSL_ENSURE( false, "<SwTextShell::ExecCharAttrArgs(..)> - unexpected unvisited character format ID at hyperlink attribute" );
                aINetFormat.SetINetFormatAndId(
                        aINetFormat.GetINetFormat(),
                        SwStyleNameMapper::GetPoolIdFromUIName( aINetFormat.GetINetFormat(), SwGetPoolIdFromName::ChrFmt ) );
            }

            if ( pColl )
                pColl->SetFormatAttr( aINetFormat );
            else
                rWrtSh.SetAttrItem( aINetFormat );
            rReq.Done();
        }
        break;

        case FN_GROW_FONT_SIZE:
        case FN_SHRINK_FONT_SIZE:
        {
            SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT, rPool );
            rWrtSh.GetCurAttr( aSetItem.GetItemSet() );
            SfxItemSet aAttrSet( rPool, aSetItem.GetItemSet().GetRanges() );

            SvtScriptType nScriptTypes = rWrtSh.GetScriptType();
            const SvxFontHeightItem* pSize( static_cast<const SvxFontHeightItem*>(
                                        aSetItem.GetItemOfScript( nScriptTypes ) ) );
            std::vector<std::pair< const SfxPoolItem*, std::unique_ptr<SwPaM> >> vItems;
            if ( pSize ) // selected text has one size
            {
                // must create new one, otherwise document is without pam
                SwPaM* pPaM = rWrtSh.GetCursor();
                vItems.emplace_back( pSize, std::make_unique<SwPaM>( *(pPaM->GetMark()), *(pPaM->GetPoint())) );
            }
            else
                vItems = rWrtSh.GetItemWithPaM( RES_CHRATR_FONTSIZE );

            rWrtSh.StartUndo( SwUndoId::INSATTR );
            for( std::pair< const SfxPoolItem*, std::unique_ptr<SwPaM> >& iPair : vItems )
            {
                std::unique_ptr<SwPaM> pPaM = std::move(iPair.second);
                const SfxPoolItem* pItem = iPair.first;
                rWrtSh.GetPaMAttr( pPaM.get(), aSetItem.GetItemSet() );
                aAttrSet.SetRanges( aSetItem.GetItemSet().GetRanges() );

                pSize = static_cast<const SvxFontHeightItem*>( pItem );
                if (pSize)
                {
                    SvxFontHeightItem aSize(*pSize);

                    sal_uInt32 nSize = aSize.GetHeight();

                    if ( nSlot == FN_GROW_FONT_SIZE && ( nSize += nFontInc ) > nFontMaxSz )
                        nSize = nFontMaxSz;
                    else if ( nSlot == FN_SHRINK_FONT_SIZE && ( nSize -= nFontInc ) < nFontInc )
                        nSize = nFontInc;

                    aSize.SetHeight( nSize );
                    aSetItem.PutItemForScriptType( nScriptTypes, aSize );
                    aAttrSet.Put( aSetItem.GetItemSet() );
                    if( pColl )
                        pColl->SetFormatAttr( aAttrSet );
                    else
                        rWrtSh.SetAttrSet( aAttrSet, SetAttrMode::DEFAULT, pPaM.get() );
                }
            }
            rWrtSh.EndUndo( SwUndoId::INSATTR );
            rReq.Done();
        }
        break;

        default:
            OSL_FAIL("wrong  dispatcher");
            return;
    }
}

void SwTextShell::ExecParaAttr(SfxRequest &rReq)
{
    SvxAdjust eAdjst;
    sal_uInt16 ePropL;
    const SfxItemSet* pArgs = rReq.GetArgs();

    // Get both attributes immediately isn't more expensive!!
    SfxItemSet aSet( GetPool(),
        svl::Items<RES_PARATR_LINESPACING, RES_PARATR_ADJUST,
        RES_FRAMEDIR, RES_FRAMEDIR>{} );

    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_ATTR_PARA_ADJUST:
        {
            if( pArgs && SfxItemState::SET == pArgs->GetItemState(RES_PARATR_ADJUST) )
            {
                const SvxAdjustItem& rAdj = pArgs->Get(RES_PARATR_ADJUST);
                SvxAdjustItem aAdj( rAdj.GetAdjust(), RES_PARATR_ADJUST );
                if ( rAdj.GetAdjust() == SvxAdjust::Block )
                {
                    aAdj.SetLastBlock( rAdj.GetLastBlock() );
                    aAdj.SetOneWord( rAdj.GetOneWord() );
                }

                aSet.Put(aAdj);
            }
        }
        break;
        case SID_ATTR_PARA_ADJUST_LEFT:     eAdjst =  SvxAdjust::Left;      goto SET_ADJUST;
        case SID_ATTR_PARA_ADJUST_RIGHT:    eAdjst =  SvxAdjust::Right;     goto SET_ADJUST;
        case SID_ATTR_PARA_ADJUST_CENTER:   eAdjst =  SvxAdjust::Center;    goto SET_ADJUST;
        case SID_ATTR_PARA_ADJUST_BLOCK:    eAdjst =  SvxAdjust::Block;     goto SET_ADJUST;
SET_ADJUST:
        {
            aSet.Put(SvxAdjustItem(eAdjst,RES_PARATR_ADJUST));
            rReq.AppendItem( SfxBoolItem( GetPool().GetWhich(nSlot), true ) );
        }
        break;

        case SID_ATTR_PARA_LINESPACE:
            if(pArgs && SfxItemState::SET == pArgs->GetItemState( GetPool().GetWhich(nSlot) ))
            {
                SvxLineSpacingItem aLineSpace = static_cast<const SvxLineSpacingItem&>( pArgs->Get(
                                                            GetPool().GetWhich(nSlot)));
                aSet.Put( aLineSpace );
            }
        break;
        case SID_ATTR_PARA_LINESPACE_10:    ePropL = 100;   goto SET_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_15:    ePropL = 150;   goto SET_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_20:    ePropL = 200;   goto SET_LINESPACE;

SET_LINESPACE:
        {

            SvxLineSpacingItem aLineSpacing(ePropL, RES_PARATR_LINESPACING );
            aLineSpacing.SetLineSpaceRule( SvxLineSpaceRule::Auto );
            if( 100 == ePropL )
                aLineSpacing.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
            else
                aLineSpacing.SetPropLineSpace(ePropL);
            aSet.Put( aLineSpacing );
        }
        break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT :
        case SID_ATTR_PARA_RIGHT_TO_LEFT :
        {
            SfxItemSet aAdjustSet( GetPool(),
                    svl::Items<RES_PARATR_ADJUST, RES_PARATR_ADJUST>{} );
            GetShell().GetCurAttr(aAdjustSet);
            bool bChgAdjust = false;
            SfxItemState eAdjustState = aAdjustSet.GetItemState(RES_PARATR_ADJUST, false);
            if(eAdjustState  >= SfxItemState::DEFAULT)
            {
                SvxAdjust eAdjust =
                        aAdjustSet.Get(RES_PARATR_ADJUST).GetAdjust();
                bChgAdjust = (SvxAdjust::Left  == eAdjust  &&  SID_ATTR_PARA_RIGHT_TO_LEFT == nSlot) ||
                             (SvxAdjust::Right == eAdjust  &&  SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot);
            }
            else
                bChgAdjust = true;

            SvxFrameDirection eFrameDirection =
                    (SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot) ?
                        SvxFrameDirection::Horizontal_LR_TB : SvxFrameDirection::Horizontal_RL_TB;
            aSet.Put( SvxFrameDirectionItem( eFrameDirection, RES_FRAMEDIR ) );

            if (bChgAdjust)
            {
                SvxAdjust eAdjust = (SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot) ?
                        SvxAdjust::Left : SvxAdjust::Right;
                SvxAdjustItem aAdjust( eAdjust, RES_PARATR_ADJUST );
                aSet.Put( aAdjust );
                aAdjust.SetWhich(SID_ATTR_PARA_ADJUST);
                GetView().GetViewFrame()->GetBindings().SetState( aAdjust );
                // Toggle numbering alignment
                const SwNumRule* pCurRule = GetShell().GetNumRuleAtCurrCursorPos();
                if( pCurRule )
                {
                    SvxNumRule aRule = pCurRule->MakeSvxNumRule();

                    for(sal_uInt16 i = 0; i < aRule.GetLevelCount(); i++)
                    {
                        SvxNumberFormat aFormat(aRule.GetLevel(i));
                        if(SvxAdjust::Left == aFormat.GetNumAdjust())
                            aFormat.SetNumAdjust( SvxAdjust::Right );

                        else if(SvxAdjust::Right == aFormat.GetNumAdjust())
                            aFormat.SetNumAdjust( SvxAdjust::Left );

                        aRule.SetLevel(i, aFormat, aRule.Get(i) != nullptr);
                    }
                    SwNumRule aSetRule( pCurRule->GetName(),
                                        pCurRule->Get( 0 ).GetPositionAndSpaceMode() );
                    aSetRule.SetSvxRule( aRule, GetShell().GetDoc());
                    aSetRule.SetAutoRule( true );
                    // no start or continuation of a list - list style is only changed
                    GetShell().SetCurNumRule( aSetRule, false );
                }
            }
        }
        break;

        default:
            OSL_FAIL("wrong  dispatcher");
            return;
    }
    SwWrtShell& rWrtSh = GetShell();
    SwTextFormatColl* pColl = rWrtSh.GetCurTextFormatColl();
    if(pColl && pColl->IsAutoUpdateFormat())
    {
        rWrtSh.AutoUpdatePara(pColl, aSet);
    }
    else
        rWrtSh.SetAttrSet( aSet, SetAttrMode::DEFAULT, nullptr, true);
    rReq.Done();
}

void SwTextShell::ExecParaAttrArgs(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem *pItem = nullptr;

    sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), false, &pItem);
    switch ( nSlot )
    {
        case FN_DROP_CHAR_STYLE_NAME:
            if( pItem )
            {
                OUString sCharStyleName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                SfxItemSet aSet(GetPool(), svl::Items<RES_PARATR_DROP, RES_PARATR_DROP>{});
                rSh.GetCurAttr(aSet);
                SwFormatDrop aDropItem(aSet.Get(RES_PARATR_DROP));
                SwCharFormat* pFormat = nullptr;
                if(!sCharStyleName.isEmpty())
                    pFormat = rSh.FindCharFormatByName( sCharStyleName );
                aDropItem.SetCharFormat( pFormat );
                aSet.Put(aDropItem);
                rSh.SetAttrSet(aSet);
            }
        break;
        case FN_FORMAT_DROPCAPS:
        {
            if(pItem)
            {
                rSh.SetAttrItem(*pItem);
                rReq.Done();
            }
            else
            {
                SfxItemSet aSet(GetPool(), svl::Items<RES_PARATR_DROP, RES_PARATR_DROP,
                                           HINT_END, HINT_END>{});
                rSh.GetCurAttr(aSet);
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateSwDropCapsDialog(GetView().GetFrameWeld(), aSet));
                if (pDlg->Execute() == RET_OK)
                {
                    rSh.StartAction();
                    rSh.StartUndo( SwUndoId::START );
                    if ( SfxItemState::SET == aSet.GetItemState(HINT_END,false,&pItem) )
                    {
                        if ( !static_cast<const SfxStringItem*>(pItem)->GetValue().isEmpty() )
                            rSh.ReplaceDropText(static_cast<const SfxStringItem*>(pItem)->GetValue());
                    }
                    rSh.SetAttrSet(*pDlg->GetOutputItemSet());
                    rSh.EndUndo( SwUndoId::END );
                    rSh.EndAction();
                    rReq.Done(*pDlg->GetOutputItemSet());
                }
            }
        }
         break;
        case SID_ATTR_PARA_PAGEBREAK:
            if(pItem)
            {
                rSh.SetAttrItem( *pItem );
                rReq.Done();
            }
        break;
        case SID_ATTR_PARA_MODEL:
        {
            if(pItem)
            {
                SfxItemSet aCoreSet( GetPool(),
                    svl::Items<RES_PAGEDESC,   RES_PAGEDESC,
                    SID_ATTR_PARA_MODEL, SID_ATTR_PARA_MODEL>{});
                aCoreSet.Put(*pItem);
                SfxToSwPageDescAttr( rSh, aCoreSet);
                rSh.SetAttrSet(aCoreSet);
                rReq.Done();
            }
        }
        break;

        default:
            OSL_FAIL("wrong  dispatcher");
            return;
    }
}

void SwTextShell::GetAttrState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxItemPool& rPool = GetPool();
    SfxItemSet aCoreSet(rPool, aTextFormatCollSetRange);
    // Request *all* text attributes from the core.
    // fdo#78737: this is called from SvxRuler, which requires the list indents!
    rSh.GetCurAttr(aCoreSet, /* bMergeIndentValuesOfNumRule = */ true);

    SfxWhichIter aIter(rSet);
    sal_uInt16 nSlot = aIter.FirstWhich();
    bool bFlag = false;
    SfxBoolItem aFlagItem;
    const SfxPoolItem* pItem = nullptr;
    SvxAdjust eAdjust = SvxAdjust::Left;
    bool bAdjustGood = false;
    SfxItemState eState = aCoreSet.GetItemState(RES_PARATR_ADJUST, false, &pItem);

    if( SfxItemState::DEFAULT == eState )
        pItem = &rPool.GetDefaultItem(RES_PARATR_ADJUST);
    if( SfxItemState::DEFAULT <= eState )
    {
        eAdjust = static_cast<const SvxAdjustItem* >( pItem)->GetAdjust();
        bAdjustGood = true;
    }

    short nEsc = 0;
    eState =  aCoreSet.GetItemState(RES_CHRATR_ESCAPEMENT, false, &pItem);
    if( SfxItemState::DEFAULT == eState )
        pItem = &rPool.GetDefaultItem(RES_CHRATR_ESCAPEMENT);
    if( eState >= SfxItemState::DEFAULT )
        nEsc = static_cast<const SvxEscapementItem* >(pItem)->GetEsc();

    sal_uInt16 nLineSpace = 0;
    eState =  aCoreSet.GetItemState(RES_PARATR_LINESPACING, false, &pItem);
    if( SfxItemState::DEFAULT == eState )
        pItem = &rPool.GetDefaultItem(RES_PARATR_LINESPACING);
    if( SfxItemState::DEFAULT <= eState &&
            static_cast<const SvxLineSpacingItem* >(pItem)->GetLineSpaceRule() == SvxLineSpaceRule::Auto )
    {
        if(SvxInterLineSpaceRule::Off ==
                    static_cast<const SvxLineSpacingItem* >(pItem)->GetInterLineSpaceRule())
            nLineSpace = 100;
        else
            nLineSpace = static_cast<const SvxLineSpacingItem* >(pItem)->GetPropLineSpace();
    }

    SvxCaseMap eCaseMap = SvxCaseMap::NotMapped;
    eState = aCoreSet.GetItemState(RES_CHRATR_CASEMAP, false, &pItem);
    if (eState == SfxItemState::DEFAULT)
        pItem = &rPool.GetDefaultItem(RES_CHRATR_CASEMAP);
    if (eState >= SfxItemState::DEFAULT)
        eCaseMap = static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap();

    while (nSlot)
    {
        switch(nSlot)
        {
            case FN_SET_SUPER_SCRIPT:
                    bFlag = 0 < nEsc;
                break;
            case FN_SET_SUB_SCRIPT:
                    bFlag = 0 > nEsc;
                break;
            case FN_SET_SMALL_CAPS:
                bFlag = eCaseMap == SvxCaseMap::SmallCaps;
                break;
            case SID_ATTR_PARA_ADJUST_LEFT:
                if (!bAdjustGood)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                    bFlag = SvxAdjust::Left == eAdjust;
                break;
            case SID_ATTR_PARA_ADJUST_RIGHT:
                if (!bAdjustGood)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                    bFlag = SvxAdjust::Right == eAdjust;
                break;
            case SID_ATTR_PARA_ADJUST_CENTER:
                if (!bAdjustGood)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                    bFlag = SvxAdjust::Center == eAdjust;
                break;
            case SID_ATTR_PARA_ADJUST_BLOCK:
            {
                if (!bAdjustGood)
                {
                    rSet.InvalidateItem( nSlot );
                    nSlot = 0;
                }
                else
                {
                    bFlag = SvxAdjust::Block == eAdjust;
                    sal_uInt16 nHtmlMode = GetHtmlMode(rSh.GetView().GetDocShell());
                    if((nHtmlMode & HTMLMODE_ON) && !(nHtmlMode & HTMLMODE_FULL_STYLES ))
                    {
                        rSet.DisableItem( nSlot );
                        nSlot = 0;
                    }
                }
            }
            break;
            case SID_ATTR_PARA_LINESPACE_10:
                bFlag = nLineSpace == 100;
            break;
            case SID_ATTR_PARA_LINESPACE_15:
                bFlag = nLineSpace == 150;
            break;
            case SID_ATTR_PARA_LINESPACE_20:
                bFlag = nLineSpace == 200;
            break;
            case FN_GROW_FONT_SIZE:
            case FN_SHRINK_FONT_SIZE:
            {
                SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT,
                                            *rSet.GetPool() );
                aSetItem.GetItemSet().Put( aCoreSet, false );
                const SvxFontHeightItem* pSize( static_cast<const SvxFontHeightItem*>(
                                            aSetItem.GetItemOfScript( rSh.GetScriptType() ) ) );

                if( pSize ) // selection is of one size
                {
                    sal_uInt32 nSize = pSize->GetHeight();
                    if( nSize == nFontMaxSz )
                        rSet.DisableItem( FN_GROW_FONT_SIZE );
                    else if( nSize == nFontInc )
                        rSet.DisableItem( FN_SHRINK_FONT_SIZE );
                }
                else
                {
                    std::vector<std::pair< const SfxPoolItem*, std::unique_ptr<SwPaM> >>
                        vFontHeight = rSh.GetItemWithPaM( RES_CHRATR_FONTSIZE );
                    for ( const std::pair< const SfxPoolItem*, std::unique_ptr<SwPaM>>& aIt : vFontHeight )
                    {
                        if (!aIt.first)
                        {
                            rSet.DisableItem(FN_GROW_FONT_SIZE);
                            rSet.DisableItem(FN_SHRINK_FONT_SIZE);
                            break;
                        }
                        pSize = static_cast<const SvxFontHeightItem*>( aIt.first );
                        sal_uInt32 nSize = pSize->GetHeight();
                        if( nSize == nFontMaxSz )
                            rSet.DisableItem( FN_GROW_FONT_SIZE );
                        else if( nSize == nFontInc )
                            rSet.DisableItem( FN_SHRINK_FONT_SIZE );
                    }
                }
                nSlot = 0;
            }
            break;
            case FN_UNDERLINE_DOUBLE:
            {
                eState = aCoreSet.GetItemState(RES_CHRATR_UNDERLINE);
                if( eState >= SfxItemState::DEFAULT )
                {
                    FontLineStyle eUnderline =
                            aCoreSet.Get(RES_CHRATR_UNDERLINE).GetLineStyle();
                    rSet.Put(SfxBoolItem(nSlot, eUnderline == LINESTYLE_DOUBLE));
                }
                else
                    rSet.InvalidateItem(nSlot);
                nSlot = 0;
            }
            break;
            case SID_ATTR_PARA_ADJUST:
                if (!bAdjustGood)
                    rSet.InvalidateItem( nSlot );
                else
                    rSet.Put(SvxAdjustItem(eAdjust, SID_ATTR_PARA_ADJUST ));
                nSlot = 0;
            break;
            case SID_ATTR_PARA_LRSPACE:
            case SID_ATTR_PARA_LEFTSPACE:
            case SID_ATTR_PARA_RIGHTSPACE:
            case SID_ATTR_PARA_FIRSTLINESPACE:
            {
                eState = aCoreSet.GetItemState(RES_LR_SPACE);
                if( eState >= SfxItemState::DEFAULT )
                {
                    SvxLRSpaceItem aLR = aCoreSet.Get( RES_LR_SPACE );
                    aLR.SetWhich(nSlot);
                    rSet.Put(aLR);
                }
                else
                    rSet.InvalidateItem(nSlot);
                nSlot = 0;
            }
            break;

            case SID_ATTR_PARA_LEFT_TO_RIGHT :
            case SID_ATTR_PARA_RIGHT_TO_LEFT :
            {
                if ( !SW_MOD()->GetCTLOptions().IsCTLFontEnabled() )
                {
                    rSet.DisableItem( nSlot );
                    nSlot = 0;
                }
                else
                {
                    // is the item set?
                    sal_uInt16 nHtmlMode = GetHtmlMode(rSh.GetView().GetDocShell());
                    if((!(nHtmlMode & HTMLMODE_ON) || (0 != (nHtmlMode & HTMLMODE_SOME_STYLES))) &&
                    aCoreSet.GetItemState( RES_FRAMEDIR, false ) >= SfxItemState::DEFAULT)
                    {
                        SvxFrameDirection eFrameDir =
                                aCoreSet.Get(RES_FRAMEDIR).GetValue();
                        if (SvxFrameDirection::Environment == eFrameDir)
                        {
                            eFrameDir = rSh.IsInRightToLeftText() ?
                                    SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB;
                        }
                        bFlag = (SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot &&
                                            SvxFrameDirection::Horizontal_LR_TB == eFrameDir) ||
                                (SID_ATTR_PARA_RIGHT_TO_LEFT == nSlot &&
                                            SvxFrameDirection::Horizontal_RL_TB == eFrameDir);
                    }
                    else
                    {
                        rSet.InvalidateItem(nSlot);
                        nSlot = 0;
                    }
                }
            }
            break;

            case SID_ATTR_CHAR_LANGUAGE:
            case SID_ATTR_CHAR_KERNING:
            case RES_PARATR_DROP:
            {
#if OSL_DEBUG_LEVEL > 1
                const SfxPoolItem& rItem = aCoreSet.Get(GetPool().GetWhich(nSlot), true);
                rSet.Put(rItem);
#else
                rSet.Put(aCoreSet.Get( GetPool().GetWhich(nSlot)));
#endif
                nSlot = 0;
            }
            break;
            case SID_ATTR_PARA_MODEL:
            {
                SfxItemSet aTemp(GetPool(),
                        svl::Items<RES_PAGEDESC,RES_PAGEDESC,
                        SID_ATTR_PARA_MODEL,SID_ATTR_PARA_MODEL>{});
                aTemp.Put(aCoreSet);
                ::SwToSfxPageDescAttr(aTemp);
                rSet.Put(aTemp.Get(SID_ATTR_PARA_MODEL));
                nSlot = 0;
            }
            break;
            case RES_TXTATR_INETFMT:
            {
                SfxItemSet aSet(GetPool(), svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>{});
                rSh.GetCurAttr(aSet);
                const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT);
                rSet.Put(rItem);
                nSlot = 0;
            }
            break;

            default:
            // Do nothing
            nSlot = 0;
            break;

        }
        if( nSlot )
        {
            aFlagItem.SetWhich( nSlot );
            aFlagItem.SetValue( bFlag );
            rSet.Put( aFlagItem );
        }
        nSlot = aIter.NextWhich();
    }

    rSet.Put(aCoreSet,false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
