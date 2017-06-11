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

#include "TextObjectBar.hxx"

#include <svx/svxids.hrc>

#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/outliner.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <vcl/vclenum.hxx>
#include <sfx2/app.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svl/style.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flstitem.hxx>
#include <svl/intitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/itemiter.hxx>
#include <editeng/cmapitem.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "prlayout.hxx"
#include "ViewShell.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "OutlineView.hxx"
#include "Window.hxx"
#include "futempl.hxx"
#include "DrawDocShell.hxx"
#include "Outliner.hxx"
#include "futext.hxx"

#include <memory>

namespace sd {

/**
 * Process SfxRequests
 */

void TextObjectBar::Execute( SfxRequest &rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pPoolItem = nullptr;
    sal_uInt16 nSlot = rReq.GetSlot();
    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    std::unique_ptr< OutlineViewModelChangeGuard > aGuard;

    if( dynamic_cast< const OutlineView *>( mpView ) !=  nullptr)
    {
        pOLV = static_cast<OutlineView*>(mpView)
            ->GetViewByWindow(mpViewShell->GetActiveWindow());

        aGuard.reset( new OutlineViewModelChangeGuard( static_cast<OutlineView&>(*mpView) ) );
    }

    switch (nSlot)
    {
        case SID_STYLE_APPLY:
        {
            if( pArgs )
            {
                SdDrawDocument& rDoc = mpView->GetDoc();
                assert(mpViewShell->GetViewShell());
                rtl::Reference<FuPoor> xFunc( FuTemplate::Create( mpViewShell, static_cast< ::sd::Window*>( mpViewShell->GetViewShell()->GetWindow()), mpView, &rDoc, rReq ) );

                if(xFunc.is())
                {
                    xFunc->Activate();
                    xFunc->Deactivate();

                    if( rReq.GetSlot() == SID_STYLE_APPLY )
                    {
                        if (mpViewShell->GetViewFrame())
                            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                    }
                }
            }
            else
            {
                if( mpViewShell && mpViewShell->GetViewFrame() )
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_STYLE_DESIGNER, SfxCallMode::ASYNCHRON );
            }

            rReq.Done();
        }
        break;

        case SID_PARASPACE_INCREASE:
        case SID_PARASPACE_DECREASE:
        {
            if( pOLV )
            {
                ESelection aSel = pOLV->GetSelection();
                aSel.Adjust();
                sal_Int32 nStartPara = aSel.nStartPara;
                sal_Int32 nEndPara = aSel.nEndPara;
                if( !aSel.HasRange() )
                {
                    nStartPara = 0;
                    nEndPara = pOLV->GetOutliner()->GetParagraphCount() - 1;
                }
                for( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
                {
                    SfxStyleSheet* pStyleSheet = nullptr;
                    if (pOLV->GetOutliner() != nullptr)
                        pStyleSheet = pOLV->GetOutliner()->GetStyleSheet(nPara);
                    if (pStyleSheet != nullptr)
                    {
                        SfxItemSet aAttr( pStyleSheet->GetItemSet() );
                        SfxItemSet aTmpSet( pOLV->GetOutliner()->GetParaAttribs( nPara ) );
                        aAttr.Put( aTmpSet, false ); // sal_False= InvalidItems is not default, handle it as "holes"
                        const SvxULSpaceItem& rItem = static_cast<const SvxULSpaceItem&>( aAttr.Get( EE_PARA_ULSPACE ) );
                        std::unique_ptr<SvxULSpaceItem> pNewItem(static_cast<SvxULSpaceItem*>(rItem.Clone()));

                        long nUpper = pNewItem->GetUpper();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nUpper += 100;
                        else
                        {
                            nUpper -= 100;
                            nUpper = std::max<long>( nUpper, 0 );
                        }
                        pNewItem->SetUpper( (sal_uInt16) nUpper );

                        long nLower = pNewItem->GetLower();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nLower += 100;
                        else
                        {
                            nLower -= 100;
                            nLower = std::max<long>( nLower, 0 );
                        }
                        pNewItem->SetLower( (sal_uInt16) nLower );

                        SfxItemSet aNewAttrs( aAttr );
                        aNewAttrs.Put( *pNewItem );
                        pNewItem.reset();
                        pOLV->GetOutliner()->SetParaAttribs( nPara, aNewAttrs );
                    }
                }
            }
            else
            {
                // the following code could be enabled, if I get a correct
                // DontCare status from JOE.

                // gets enabled, through it doesn't really work (see above)
                SfxItemSet aEditAttr( mpView->GetDoc().GetPool() );
                mpView->GetAttributes( aEditAttr );
                if( aEditAttr.GetItemState( EE_PARA_ULSPACE ) >= SfxItemState::DEFAULT )
                {
                    SfxItemSet aNewAttrs(*(aEditAttr.GetPool()), aEditAttr.GetRanges());
                    const SvxULSpaceItem& rItem = static_cast<const SvxULSpaceItem&>( aEditAttr.Get( EE_PARA_ULSPACE ) );
                    std::unique_ptr<SvxULSpaceItem> pNewItem(static_cast<SvxULSpaceItem*>( rItem.Clone() ));
                    long nUpper = pNewItem->GetUpper();

                    if( nSlot == SID_PARASPACE_INCREASE )
                        nUpper += 100;
                    else
                    {
                        nUpper -= 100;
                        nUpper = std::max<long>( nUpper, 0 );
                    }
                    pNewItem->SetUpper( (sal_uInt16) nUpper );

                    long nLower = pNewItem->GetLower();
                    if( nSlot == SID_PARASPACE_INCREASE )
                        nLower += 100;
                    else
                    {
                        nLower -= 100;
                        nLower = std::max<long>( nLower, 0 );
                    }
                    pNewItem->SetLower( (sal_uInt16) nLower );

                    aNewAttrs.Put( *pNewItem );
                    pNewItem.reset();

                    mpView->SetAttributes( aNewAttrs );
                }
            }
            rReq.Done();

            Invalidate();
            // to refresh preview (in outline mode), slot has to be invalidated:
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
        }
        break;

        case SID_OUTLINE_LEFT:
        {
            if (pOLV)
            {
                pOLV->AdjustDepth( -1 );

                // Ensure bold/italic etc. icon state updates
                Invalidate();
                // trigger preview refresh
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
            }
            rReq.Done();
        }
        break;

        case SID_OUTLINE_RIGHT:
        {
            if (pOLV)
            {
                pOLV->AdjustDepth( 1 );

                // Ensure bold/italic etc. icon state updates
                Invalidate();
                // trigger preview refresh
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
            }
            rReq.Done();
        }
        break;

        case SID_ATTR_PARA_LRSPACE:
        {
            SvxLRSpaceItem aLRSpace = static_cast<const SvxLRSpaceItem&>(pArgs->Get(
                GetPool().GetWhich(SID_ATTR_PARA_LRSPACE)));

            SfxItemSet aEditAttr( GetPool(), svl::Items<EE_PARA_LRSPACE, EE_PARA_LRSPACE>{} );
            aLRSpace.SetWhich( EE_PARA_LRSPACE );

            aEditAttr.Put( aLRSpace );
            mpView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_LRSPACE);
        }
        break;

        case SID_HANGING_INDENT:
        {
            SfxItemSet aLRSpaceSet( GetPool(), svl::Items<EE_PARA_LRSPACE, EE_PARA_LRSPACE>{} );
            mpView->GetAttributes( aLRSpaceSet );
            SvxLRSpaceItem aParaMargin( static_cast<const SvxLRSpaceItem&>( aLRSpaceSet.Get( EE_PARA_LRSPACE ) ) );

            SvxLRSpaceItem aNewMargin( EE_PARA_LRSPACE );
            aNewMargin.SetTextLeft( aParaMargin.GetTextLeft() + aParaMargin.GetTextFirstLineOfst() );
            aNewMargin.SetRight( aParaMargin.GetRight() );
            aNewMargin.SetTextFirstLineOfst( ( aParaMargin.GetTextFirstLineOfst() ) * (-1) );
            aLRSpaceSet.Put( aNewMargin );
            mpView->SetAttributes( aLRSpaceSet );

            Invalidate(SID_ATTR_PARA_LRSPACE);
        }
        break;

        case SID_OUTLINE_UP:
        {
            if (pOLV)
            {
                pOLV->AdjustHeight( -1 );

                // trigger preview refresh
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
            }
            rReq.Done();
        }
        break;

        case SID_OUTLINE_DOWN:
        {
            if (pOLV)
            {
                pOLV->AdjustHeight( 1 );

                // trigger preview refresh
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
            }
            rReq.Done();
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        {
            mpView->SdrEndTextEdit();
            SfxItemSet aAttr( mpView->GetDoc().GetPool(), svl::Items<SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION>{} );
            aAttr.Put( SvxWritingModeItem(
                nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                    css::text::WritingMode_LR_TB : css::text::WritingMode_TB_RL,
                    SDRATTR_TEXTDIRECTION ) );
            rReq.Done( aAttr );
            mpView->SetAttributes( aAttr );
            Invalidate();
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
        }
        break;

        case FN_NUM_BULLET_ON:
        {
            if (pOLV)
            {
                bool bMasterPage = false;
                SdrPageView* pPageView = mpView->GetSdrPageView();
                if (pPageView)
                {
                    SdPage* pPage = static_cast<SdPage*>(pPageView->GetPage());
                    bMasterPage = pPage && (pPage->GetPageKind() == PageKind::Standard) && pPage->IsMasterPage();
                }

                if (!bMasterPage)
                    pOLV->ToggleBullets();
                else
                {
                    //Resolves: fdo#78151 in master pages if we toggle bullets on
                    //and off then just disable/enable the bulleting, but do not
                    //change the *level* of the paragraph, because the paragraph is
                    //effectively a preview of the equivalent style level, and
                    //changing the level disconnects it from the style

                    ::Outliner* pOL = pOLV->GetOutliner();
                    if (pOL)
                    {
                        const SvxNumBulletItem *pItem = nullptr;
                        SfxStyleSheetBasePool* pSSPool = mpView->GetDocSh()->GetStyleSheetPool();
                        OUString sStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " 1");
                        SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find(sStyleName, SD_STYLE_FAMILY_PSEUDO);
                        if( pFirstStyleSheet )
                            pFirstStyleSheet->GetItemSet().GetItemState(EE_PARA_NUMBULLET, false, reinterpret_cast<const SfxPoolItem**>(&pItem));

                        if (pItem )
                        {
                            SvxNumRule aNewRule(*pItem->GetNumRule());
                            ESelection aSel = pOLV->GetSelection();
                            aSel.Adjust();
                            sal_Int32 nStartPara = aSel.nStartPara;
                            sal_Int32 nEndPara = aSel.nEndPara;
                            for (sal_Int32 nPara = nStartPara; nPara <= nEndPara; ++nPara)
                            {
                                sal_uInt16 nLevel = pOL->GetDepth(nPara);
                                SvxNumberFormat aFmt(aNewRule.GetLevel(nLevel));

                                if (aFmt.GetNumberingType() == SVX_NUM_NUMBER_NONE)
                                {
                                    aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                                    SdStyleSheetPool::setDefaultOutlineNumberFormatBulletAndIndent(nLevel, aFmt);
                                }
                                else
                                {
                                    aFmt.SetNumberingType(SVX_NUM_NUMBER_NONE);
                                    aFmt.SetAbsLSpace(0);
                                    aFmt.SetFirstLineOffset(0);
                                }

                                aNewRule.SetLevel(nLevel, aFmt);
                            }

                            pFirstStyleSheet->GetItemSet().Put(SvxNumBulletItem(aNewRule, EE_PARA_NUMBULLET));

                            SdStyleSheet::BroadcastSdStyleSheetChange(pFirstStyleSheet, PO_OUTLINE_1, pSSPool);
                        }
                    }
                }
            }
            break;
        }
        case SID_GROW_FONT_SIZE:
        case SID_SHRINK_FONT_SIZE:
        {
            const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(mpViewShell->GetDocSh()->GetItem( SID_ATTR_CHAR_FONTLIST ));
            const FontList* pFontList = pFonts ? pFonts->GetFontList(): nullptr;
            if( pFontList )
            {
                FuText::ChangeFontSize( nSlot == SID_GROW_FONT_SIZE, pOLV, pFontList, mpView );
                if( pOLV )
                    pOLV->SetAttribs( pOLV->GetEditView().GetEmptyItemSet() );
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            rReq.Done();
        }
        break;

        case SID_THES:
        {
            OUString aReplaceText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(SID_THES);
            if (pItem2)
                aReplaceText = pItem2->GetValue();
            if (!aReplaceText.isEmpty())
                ReplaceTextWithSynonym( pOLV->GetEditView(), aReplaceText );
        }
        break;

        default:
        {
            SfxItemSet aEditAttr( mpView->GetDoc().GetPool() );
            mpView->GetAttributes( aEditAttr );
            SfxItemSet aNewAttr(*(aEditAttr.GetPool()), aEditAttr.GetRanges());

            if( !pArgs )
            {
                //aNewAttr.InvalidateAllItems(); <- produces problems (#35465#)

                switch ( nSlot )
                {
                    case SID_ATTR_CHAR_WEIGHT:
                    {
                        FontWeight eFW = static_cast<const SvxWeightItem&>( aEditAttr.
                                        Get( EE_CHAR_WEIGHT ) ).GetWeight();
                        aNewAttr.Put( SvxWeightItem( eFW == WEIGHT_NORMAL ?
                                            WEIGHT_BOLD : WEIGHT_NORMAL,
                                            EE_CHAR_WEIGHT ) );
                    }
                    break;
                    case SID_ATTR_CHAR_POSTURE:
                    {
                        FontItalic eFI = static_cast<const SvxPostureItem&>( aEditAttr.
                                        Get( EE_CHAR_ITALIC ) ).GetPosture();
                        aNewAttr.Put( SvxPostureItem( eFI == ITALIC_NORMAL ?
                                            ITALIC_NONE : ITALIC_NORMAL,
                                            EE_CHAR_ITALIC ) );
                    }
                    break;
                    case SID_ATTR_CHAR_UNDERLINE:
                    {
                        FontLineStyle eFU = static_cast<const SvxUnderlineItem&>( aEditAttr.
                                        Get( EE_CHAR_UNDERLINE ) ).GetLineStyle();
                        aNewAttr.Put( SvxUnderlineItem( eFU == LINESTYLE_SINGLE ?
                                            LINESTYLE_NONE : LINESTYLE_SINGLE,
                                            EE_CHAR_UNDERLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_OVERLINE:
                    {
                        FontLineStyle eFO = static_cast<const SvxOverlineItem&>( aEditAttr.
                                        Get( EE_CHAR_OVERLINE ) ).GetLineStyle();
                        aNewAttr.Put( SvxOverlineItem( eFO == LINESTYLE_SINGLE ?
                                            LINESTYLE_NONE : LINESTYLE_SINGLE,
                                            EE_CHAR_OVERLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_CONTOUR:
                    {
                        aNewAttr.Put( SvxContourItem( !static_cast<const SvxContourItem&>( aEditAttr.
                                        Get( EE_CHAR_OUTLINE ) ).GetValue(), EE_CHAR_OUTLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_SHADOWED:
                    {
                        aNewAttr.Put( SvxShadowedItem( !static_cast<const SvxShadowedItem&>( aEditAttr.
                                        Get( EE_CHAR_SHADOW ) ).GetValue(), EE_CHAR_SHADOW ) );
                    }
                    break;
                    case SID_ATTR_CHAR_CASEMAP:
                    {
                        aNewAttr.Put( SvxCaseMapItem( static_cast<const SvxCaseMapItem&>( aEditAttr.
                                        Get( EE_CHAR_CASEMAP ) ) ) );
                    }
                    break;
                    case SID_ATTR_CHAR_STRIKEOUT:
                    {
                        FontStrikeout eFSO = ( static_cast<const SvxCrossedOutItem&>( aEditAttr.
                                        Get( EE_CHAR_STRIKEOUT ) ).GetStrikeout() );
                        aNewAttr.Put( SvxCrossedOutItem( eFSO == STRIKEOUT_SINGLE ?
                                            STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
                    }
                    break;

                    case SID_ATTR_PARA_ADJUST_LEFT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_CENTER:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_RIGHT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_BLOCK:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Block, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_10:
                    {
                        SvxLineSpacingItem aItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL );
                        aItem.SetPropLineSpace( 100 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_15:
                    {
                        SvxLineSpacingItem aItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL );
                        aItem.SetPropLineSpace( 150 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_20:
                    {
                        SvxLineSpacingItem aItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL );
                        aItem.SetPropLineSpace( 200 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_SET_SUPER_SCRIPT:
                    {
                        SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
                        SvxEscapement eEsc = (SvxEscapement ) static_cast<const SvxEscapementItem&>(
                                        aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                        if( eEsc == SvxEscapement::Superscript )
                            aItem.SetEscapement( SvxEscapement::Off );
                        else
                            aItem.SetEscapement( SvxEscapement::Superscript );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_SET_SUB_SCRIPT:
                    {
                        SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
                        SvxEscapement eEsc = (SvxEscapement ) static_cast<const SvxEscapementItem&>(
                                        aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                        if( eEsc == SvxEscapement::Subscript )
                            aItem.SetEscapement( SvxEscapement::Off );
                        else
                            aItem.SetEscapement( SvxEscapement::Subscript );
                        aNewAttr.Put( aItem );
                    }
                    break;

                    // attributes for TextObjectBar
                    case SID_ATTR_CHAR_FONT:
                        mpViewShell->GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SfxCallMode::ASYNCHRON );
                    break;
                    case SID_ATTR_CHAR_FONTHEIGHT:
                        mpViewShell->GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SfxCallMode::ASYNCHRON );
                    break;
                    case SID_ATTR_CHAR_COLOR:
                    break;
// #i35937# removed need for FN_NUM_BULLET_ON handling
                }

                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT ||
                      nSlot == SID_ATTR_PARA_RIGHT_TO_LEFT )
            {
                bool bLeftToRight = nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT;

                SvxAdjust nAdjust = SvxAdjust::Left;
                if( SfxItemState::SET == aEditAttr.GetItemState(EE_PARA_JUST, true, &pPoolItem ) )
                    nAdjust = static_cast<const SvxAdjustItem*>(pPoolItem)->GetAdjust();

                if( bLeftToRight )
                {
                    aNewAttr.Put( SvxFrameDirectionItem( SvxFrameDirection::Horizontal_LR_TB, EE_PARA_WRITINGDIR ) );
                    if( nAdjust == SvxAdjust::Right )
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );
                }
                else
                {
                    aNewAttr.Put( SvxFrameDirectionItem( SvxFrameDirection::Horizontal_RL_TB, EE_PARA_WRITINGDIR ) );
                    if( nAdjust == SvxAdjust::Left )
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
                }

                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();

                Invalidate( SID_RULER_TEXT_RIGHT_TO_LEFT );
            }
            else if ( nSlot == SID_ATTR_CHAR_FONT       ||
                      nSlot == SID_ATTR_CHAR_FONTHEIGHT ||
                      nSlot == SID_ATTR_CHAR_POSTURE    ||
                      nSlot == SID_ATTR_CHAR_WEIGHT )
            {
                // #i78017 establish the same behaviour as in Writer
                SvtScriptType nScriptType = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
                if (nSlot == SID_ATTR_CHAR_FONT)
                    nScriptType = mpView->GetScriptType();

                SfxItemPool& rPool = mpView->GetDoc().GetPool();
                SvxScriptSetItem aSvxScriptSetItem( nSlot, rPool );
                aSvxScriptSetItem.PutItemForScriptType( nScriptType, pArgs->Get( rPool.GetWhich( nSlot ) ) );
                aNewAttr.Put( aSvxScriptSetItem.GetItemSet() );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if (nSlot == SID_ATTR_PARA_ADJUST_LEFT ||
                nSlot == SID_ATTR_PARA_ADJUST_CENTER ||
                nSlot == SID_ATTR_PARA_ADJUST_RIGHT ||
                nSlot == SID_ATTR_PARA_ADJUST_BLOCK)
            {
                switch( nSlot )
                {
                case SID_ATTR_PARA_ADJUST_LEFT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );
                    }
                    break;
                case SID_ATTR_PARA_ADJUST_CENTER:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );
                    }
                    break;
                case SID_ATTR_PARA_ADJUST_RIGHT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
                    }
                    break;
                case SID_ATTR_PARA_ADJUST_BLOCK:
                    {
                        aNewAttr.Put( SvxAdjustItem( SvxAdjust::Block, EE_PARA_JUST ) );
                    }
                    break;
                }
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if(nSlot == SID_ATTR_CHAR_KERNING)
            {
                aNewAttr.Put(pArgs->Get(pArgs->GetPool()->GetWhich(nSlot)));
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if(nSlot ==  SID_SET_SUPER_SCRIPT )
            {
                SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                SvxEscapement eEsc = (SvxEscapement) static_cast<const SvxEscapementItem&>(
                                aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                if( eEsc == SvxEscapement::Superscript )
                    aItem.SetEscapement( SvxEscapement::Off );
                else
                    aItem.SetEscapement( SvxEscapement::Superscript );
                aNewAttr.Put( aItem );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if( nSlot ==  SID_SET_SUB_SCRIPT )
            {
                SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                SvxEscapement eEsc = (SvxEscapement) static_cast<const SvxEscapementItem&>(
                                aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                if( eEsc == SvxEscapement::Subscript )
                    aItem.SetEscapement( SvxEscapement::Off );
                else
                    aItem.SetEscapement( SvxEscapement::Subscript );
                aNewAttr.Put( aItem );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }

            mpView->SetAttributes(*pArgs);

            // invalidate entire shell because of performance and
            // extension reasons
            Invalidate();

            // to refresh preview (in outline mode), slot has to be invalidated:
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
        }
        break;
    }

    Invalidate( SID_OUTLINE_LEFT );
    Invalidate( SID_OUTLINE_RIGHT );
    Invalidate( SID_OUTLINE_UP );
    Invalidate( SID_OUTLINE_DOWN );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
