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

#include <TextObjectBar.hxx>

#include <svx/svxids.hrc>

#include <editeng/eeitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/numitem.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svl/style.hxx>
#include <sfx2/namedcolor.hxx>
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
#include <svx/svdpagv.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/scriptsetitem.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/tstpitem.hxx>

#include <app.hrc>
#include <strings.hrc>
#include <sdresid.hxx>
#include <prlayout.hxx>
#include <ViewShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <OutlineView.hxx>
#include <Window.hxx>
#include <futempl.hxx>
#include <DrawDocShell.hxx>
#include <futext.hxx>
#include <editeng/colritem.hxx>

#include <memory>

namespace sd {

/**
 * Process SfxRequests
 */
void TextObjectBar::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    // Default indent used e.g. in SID_DEC_INDENT, SID_INC_INDENT and SID_HANGING_INDENT
    const ::tools::Long nIndentDefaultDist = 1000; // 1000 twips

    std::unique_ptr<OutlineViewModelChangeGuard, o3tl::default_delete<OutlineViewModelChangeGuard>> aGuard;

    if (OutlineView* pOView = dynamic_cast<OutlineView*>(mpView))
    {
        pOLV = pOView->GetViewByWindow(mrViewShell.GetActiveWindow());
        aGuard.reset( new OutlineViewModelChangeGuard( static_cast<OutlineView&>(*mpView) ) );
    }

    switch (nSlot)
    {
        case SID_STYLE_APPLY:
        {
            if( pArgs )
            {
                SdDrawDocument& rDoc = mpView->GetDoc();
                assert(mrViewShell.GetViewShell());
                rtl::Reference<FuPoor> xFunc( FuTemplate::Create( mrViewShell, static_cast< ::sd::Window*>( mrViewShell.GetViewShell()->GetWindow()), mpView, rDoc, rReq ) );

                if(xFunc.is())
                {
                    xFunc->Activate();
                    xFunc->Deactivate();

                    if( rReq.GetSlot() == SID_STYLE_APPLY )
                    {
                        if (mrViewShell.GetViewFrame())
                            mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                    }
                }
            }
            else
            {
                if (mrViewShell.GetViewFrame())
                    mrViewShell.GetViewFrame()->GetDispatcher()->Execute( SID_STYLE_DESIGNER, SfxCallMode::ASYNCHRON );
            }

            rReq.Done();
        }
        break;

        case SID_INC_INDENT:
        case SID_DEC_INDENT:
        {
            if( pOLV )
            {
                ESelection aSel = pOLV->GetSelection();
                aSel.Adjust();
                sal_Int32 nStartPara = aSel.start.nPara;
                sal_Int32 nEndPara = aSel.end.nPara;
                if( !aSel.HasRange() )
                {
                    nStartPara = 0;
                    nEndPara = pOLV->GetOutliner().GetParagraphCount() - 1;
                }

                pOLV->GetOutliner().UndoActionStart( OLUNDO_ATTR );
                for( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
                {
                    SfxStyleSheet* pStyleSheet = pOLV->GetOutliner().GetStyleSheet(nPara);
                    if (pStyleSheet != nullptr)
                    {
                        SfxItemSet aAttr( pStyleSheet->GetItemSet() );
                        SfxItemSet aTmpSet( pOLV->GetOutliner().GetParaAttribs( nPara ) );
                        aAttr.Put( aTmpSet, false );
                        const SvxLRSpaceItem& rItem = aAttr.Get( EE_PARA_LRSPACE );
                        std::unique_ptr<SvxLRSpaceItem> pNewItem(rItem.Clone());

                        ::tools::Long nLeft = pNewItem->ResolveLeft({});
                        if( nSlot == SID_INC_INDENT )
                            nLeft += nIndentDefaultDist;
                        else
                        {
                            nLeft -= nIndentDefaultDist;
                            nLeft = std::max<::tools::Long>( nLeft, 0 );
                        }
                        pNewItem->SetLeft(SvxIndentValue::twips(static_cast<sal_uInt16>(nLeft)));

                        aAttr.Put( std::move(pNewItem) );
                        pOLV->GetOutliner().SetParaAttribs( nPara, aAttr );
                    }
                }
                pOLV->GetOutliner().UndoActionEnd();
                mrViewShell.Invalidate( SID_UNDO );
            }
            rReq.Done();

            Invalidate();
            // to refresh preview (in outline mode), slot has to be invalidated:
            mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );

        }
        break;

        case SID_PARASPACE_INCREASE:
        case SID_PARASPACE_DECREASE:
        {
            if( pOLV )
            {
                ESelection aSel = pOLV->GetSelection();
                aSel.Adjust();
                sal_Int32 nStartPara = aSel.start.nPara;
                sal_Int32 nEndPara = aSel.end.nPara;
                if( !aSel.HasRange() )
                {
                    nStartPara = 0;
                    nEndPara = pOLV->GetOutliner().GetParagraphCount() - 1;
                }

                pOLV->GetOutliner().UndoActionStart( OLUNDO_ATTR );
                for( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
                {
                    SfxStyleSheet* pStyleSheet = pOLV->GetOutliner().GetStyleSheet(nPara);
                    if (pStyleSheet != nullptr)
                    {
                        SfxItemSet aAttr( pStyleSheet->GetItemSet() );
                        SfxItemSet aTmpSet( pOLV->GetOutliner().GetParaAttribs( nPara ) );
                        aAttr.Put( aTmpSet, false ); // sal_False= InvalidItems is not default, handle it as "holes"
                        const SvxULSpaceItem& rItem = aAttr.Get( EE_PARA_ULSPACE );
                        std::unique_ptr<SvxULSpaceItem> pNewItem(rItem.Clone());

                        ::tools::Long nUpper = pNewItem->GetUpper();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nUpper += 100;
                        else
                        {
                            nUpper -= 100;
                            nUpper = std::max<::tools::Long>( nUpper, 0 );
                        }
                        pNewItem->SetUpper( static_cast<sal_uInt16>(nUpper) );

                        ::tools::Long nLower = pNewItem->GetLower();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nLower += 100;
                        else
                        {
                            nLower -= 100;
                            nLower = std::max<::tools::Long>( nLower, 0 );
                        }
                        pNewItem->SetLower( static_cast<sal_uInt16>(nLower) );

                        SfxItemSet aNewAttrs(std::move(aAttr));
                        aNewAttrs.Put( std::move(pNewItem) );
                        pOLV->GetOutliner().SetParaAttribs( nPara, aNewAttrs );
                    }
                }
                pOLV->GetOutliner().UndoActionEnd();
                mrViewShell.Invalidate( SID_UNDO );
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
                    const SvxULSpaceItem& rItem = aEditAttr.Get( EE_PARA_ULSPACE );
                    std::unique_ptr<SvxULSpaceItem> pNewItem(rItem.Clone());
                    ::tools::Long nUpper = pNewItem->GetUpper();

                    if( nSlot == SID_PARASPACE_INCREASE )
                        nUpper += 100;
                    else
                    {
                        nUpper -= 100;
                        nUpper = std::max<::tools::Long>( nUpper, 0 );
                    }
                    pNewItem->SetUpper( static_cast<sal_uInt16>(nUpper) );

                    ::tools::Long nLower = pNewItem->GetLower();
                    if( nSlot == SID_PARASPACE_INCREASE )
                        nLower += 100;
                    else
                    {
                        nLower -= 100;
                        nLower = std::max<::tools::Long>( nLower, 0 );
                    }
                    pNewItem->SetLower( static_cast<sal_uInt16>(nLower) );

                    aNewAttrs.Put( std::move(pNewItem) );

                    mpView->SetAttributes( aNewAttrs );
                }
            }
            rReq.Done();

            Invalidate();
            // to refresh preview (in outline mode), slot has to be invalidated:
            mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
            mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_ATTR_PARA_ULSPACE, true );
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
                mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
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
                mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
            }
            rReq.Done();
        }
        break;

        case SID_ATTR_PARA_LRSPACE:
        {
            SvxLRSpaceItem aLRSpace = static_cast<const SvxLRSpaceItem&>(pArgs->Get(
                SID_ATTR_PARA_LRSPACE));

            SfxItemSet aEditAttr(SfxItemSet::makeFixedSfxItemSet<EE_PARA_LRSPACE, EE_PARA_LRSPACE>(mpView->GetDoc().GetPool()));
            aLRSpace.SetWhich( EE_PARA_LRSPACE );

            aEditAttr.Put( aLRSpace );
            mpView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_LRSPACE);
        }
        break;

        case SID_HANGING_INDENT:
        {
            SfxItemSet aLRSpaceSet(SfxItemSet::makeFixedSfxItemSet<EE_PARA_LRSPACE, EE_PARA_LRSPACE>(mpView->GetDoc().GetPool()));
            mpView->GetAttributes( aLRSpaceSet );
            SvxLRSpaceItem aParaMargin( aLRSpaceSet.Get( EE_PARA_LRSPACE ) );

            SvxLRSpaceItem aNewMargin( EE_PARA_LRSPACE );

            auto nIndentDist = aParaMargin.ResolveTextFirstLineOffset({});

            if (nIndentDist == 0.0)
                nIndentDist = nIndentDefaultDist;

            aNewMargin.SetTextLeft(
                SvxIndentValue::twips(aParaMargin.ResolveTextLeft({}) + nIndentDist));
            aNewMargin.SetRight(aParaMargin.GetRight());
            aNewMargin.SetTextFirstLineOffset(SvxIndentValue::twips(nIndentDist * -1));

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
                mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
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
                mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
            }
            rReq.Done();
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        {
            mpView->SdrEndTextEdit();
            // tdf#131571: SdrEndTextEdit invalidates pTextEditOutlinerView, the pointer retrieved for pOLV
            // so reinitialize pOLV
            pOLV=mpView->GetTextEditOutlinerView();
            SfxItemSet aAttr(SfxItemSet::makeFixedSfxItemSet<SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION>(mpView->GetDoc().GetPool()));
            aAttr.Put( SvxWritingModeItem(
                nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                    css::text::WritingMode_LR_TB : css::text::WritingMode_TB_RL,
                    SDRATTR_TEXTDIRECTION ) );
            rReq.Done( aAttr );
            mpView->SetAttributes( aAttr );
            Invalidate();
            mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
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

                    ::Outliner& rOL = pOLV->GetOutliner();
                    const SvxNumBulletItem *pItem = nullptr;
                    SfxStyleSheetBasePool* pSSPool = mpView->GetDocSh()->GetStyleSheetPool();
                    OUString sStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " 1");
                    SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find(sStyleName, SfxStyleFamily::Pseudo);
                    if( pFirstStyleSheet )
                        pItem = pFirstStyleSheet->GetItemSet().GetItemIfSet(EE_PARA_NUMBULLET, false);

                    if (pItem )
                    {
                        SvxNumRule aNewRule(pItem->GetNumRule());
                        ESelection aSel = pOLV->GetSelection();
                        aSel.Adjust();
                        sal_Int32 nStartPara = aSel.start.nPara;
                        sal_Int32 nEndPara = aSel.end.nPara;
                        for (sal_Int32 nPara = nStartPara; nPara <= nEndPara; ++nPara)
                        {
                            sal_uInt16 nLevel = rOL.GetDepth(nPara);
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

                        pFirstStyleSheet->GetItemSet().Put(SvxNumBulletItem(std::move(aNewRule), EE_PARA_NUMBULLET));

                        SdStyleSheet::BroadcastSdStyleSheetChange(pFirstStyleSheet, PresentationObjects::Outline_1, pSSPool);
                    }
                }
                SfxBindings& rBindings = mrViewShell.GetViewFrame()->GetBindings();
                rBindings.Invalidate( FN_NUM_BULLET_ON );
                rBindings.Invalidate( FN_NUM_NUMBERING_ON );
            }
            break;
        }
        case SID_GROW_FONT_SIZE:
        case SID_SHRINK_FONT_SIZE:
        {
            const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(mrViewShell.GetDocSh()->GetItem( SID_ATTR_CHAR_FONTLIST ));
            const FontList* pFontList = pFonts ? pFonts->GetFontList(): nullptr;
            if( pFontList )
            {
                FuText::ChangeFontSize( nSlot == SID_GROW_FONT_SIZE, pOLV, pFontList, mpView );
                if( pOLV )
                    pOLV->SetAttribs( pOLV->GetEditView().GetEmptyItemSet() );
                mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            rReq.Done();
        }
        break;

        case SID_THES:
        {
            OUString aReplaceText;
            const SfxStringItem* pItem2 = rReq.GetArg(FN_PARAM_THES_WORD_REPLACE);
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
                SvxAdjust eAdjst;
                SdrTextHorzAdjust eAnchor;
                switch ( nSlot )
                {
                    case SID_ATTR_CHAR_WEIGHT:
                    {
                        FontWeight eFW = aEditAttr.Get( EE_CHAR_WEIGHT ).GetWeight();
                        aNewAttr.Put( SvxWeightItem( eFW == WEIGHT_NORMAL ?
                                            WEIGHT_BOLD : WEIGHT_NORMAL,
                                            EE_CHAR_WEIGHT ) );
                    }
                    break;
                    case SID_ATTR_CHAR_POSTURE:
                    {
                        FontItalic eFI = aEditAttr.Get( EE_CHAR_ITALIC ).GetPosture();
                        aNewAttr.Put( SvxPostureItem( eFI == ITALIC_NORMAL ?
                                            ITALIC_NONE : ITALIC_NORMAL,
                                            EE_CHAR_ITALIC ) );
                    }
                    break;
                    case SID_ATTR_CHAR_UNDERLINE:
                    {
                        FontLineStyle eFU = aEditAttr.Get( EE_CHAR_UNDERLINE ).GetLineStyle();
                        aNewAttr.Put( SvxUnderlineItem( eFU == LINESTYLE_SINGLE ?
                                            LINESTYLE_NONE : LINESTYLE_SINGLE,
                                            EE_CHAR_UNDERLINE ) );
                    }
                    break;

                    case SID_ULINE_VAL_NONE:
                    {
                        aNewAttr.Put(SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE));
                        break;
                    }

                    case SID_ULINE_VAL_SINGLE:
                    case SID_ULINE_VAL_DOUBLE:
                    case SID_ULINE_VAL_DOTTED:
                    {
                        FontLineStyle eOld = aEditAttr.Get(EE_CHAR_UNDERLINE).GetLineStyle();
                        FontLineStyle eNew = eOld;

                        switch (nSlot)
                        {
                            case SID_ULINE_VAL_SINGLE:
                                eNew = ( eOld == LINESTYLE_SINGLE ) ? LINESTYLE_NONE : LINESTYLE_SINGLE;
                                break;
                            case SID_ULINE_VAL_DOUBLE:
                                eNew = ( eOld == LINESTYLE_DOUBLE ) ? LINESTYLE_NONE : LINESTYLE_DOUBLE;
                                break;
                            case SID_ULINE_VAL_DOTTED:
                                eNew = ( eOld == LINESTYLE_DOTTED ) ? LINESTYLE_NONE : LINESTYLE_DOTTED;
                                break;
                        }

                        SvxUnderlineItem aUnderline(eNew, EE_CHAR_UNDERLINE);
                        aNewAttr.Put(aUnderline);
                    }
                    break;

                    case SID_ATTR_CHAR_OVERLINE:
                    {
                        FontLineStyle eFO = aEditAttr.Get( EE_CHAR_OVERLINE ).GetLineStyle();
                        aNewAttr.Put( SvxOverlineItem( eFO == LINESTYLE_SINGLE ?
                                            LINESTYLE_NONE : LINESTYLE_SINGLE,
                                            EE_CHAR_OVERLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_CONTOUR:
                    {
                        aNewAttr.Put( SvxContourItem( !aEditAttr.Get( EE_CHAR_OUTLINE ).GetValue(), EE_CHAR_OUTLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_SHADOWED:
                    {
                        aNewAttr.Put( SvxShadowedItem( !aEditAttr.Get( EE_CHAR_SHADOW ).GetValue(), EE_CHAR_SHADOW ) );
                    }
                    break;
                    case SID_ATTR_CHAR_CASEMAP:
                    {
                        aNewAttr.Put( aEditAttr.Get( EE_CHAR_CASEMAP ) );
                    }
                    break;
                    case SID_ATTR_CHAR_STRIKEOUT:
                    {
                        FontStrikeout eFSO = aEditAttr.Get( EE_CHAR_STRIKEOUT ).GetStrikeout();
                        aNewAttr.Put( SvxCrossedOutItem( eFSO == STRIKEOUT_SINGLE ?
                                            STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
                    }
                    break;

                    case SID_ATTR_PARA_ADJUST_LEFT:  eAdjst = SvxAdjust::Left;  eAnchor = SDRTEXTHORZADJUST_LEFT;  goto SET_ADJUST;
                    case SID_ATTR_PARA_ADJUST_CENTER:  eAdjst = SvxAdjust::Center;  eAnchor = SDRTEXTHORZADJUST_CENTER;  goto SET_ADJUST;
                    case SID_ATTR_PARA_ADJUST_RIGHT:  eAdjst = SvxAdjust::Right;  eAnchor = SDRTEXTHORZADJUST_RIGHT;  goto SET_ADJUST;
                    case SID_ATTR_PARA_ADJUST_BLOCK:  eAdjst = SvxAdjust::Block;  eAnchor = SDRTEXTHORZADJUST_BLOCK;  goto SET_ADJUST;
SET_ADJUST:
                    {
                        aNewAttr.Put(SvxAdjustItem(eAdjst, EE_PARA_JUST));
                        // set anchor
                        if (pOLV)
                        {
                            ESelection aSel = pOLV->GetSelection();
                            aSel.Adjust();
                            sal_Int32 nStartPara = aSel.start.nPara;
                            if (!aSel.HasRange())
                                nStartPara = 0;

                            if (nStartPara == 0)
                                aNewAttr.Put(SdrTextHorzAdjustItem(eAnchor));
                        }
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
                        SvxEscapement eEsc = aEditAttr.Get(EE_CHAR_ESCAPEMENT).GetEscapement();

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
                        SvxEscapement eEsc = aEditAttr.Get(EE_CHAR_ESCAPEMENT).GetEscapement();

                        if( eEsc == SvxEscapement::Subscript )
                            aItem.SetEscapement( SvxEscapement::Off );
                        else
                            aItem.SetEscapement( SvxEscapement::Subscript );
                        aNewAttr.Put( aItem );
                    }
                    break;

                    case SID_SET_SMALL_CAPS:
                    {
                        SvxCaseMap eCaseMap = aEditAttr.Get(EE_CHAR_CASEMAP).GetCaseMap();
                        if (eCaseMap == SvxCaseMap::SmallCaps)
                            eCaseMap = SvxCaseMap::NotMapped;
                        else
                            eCaseMap = SvxCaseMap::SmallCaps;
                        SvxCaseMapItem aItem(eCaseMap, EE_CHAR_CASEMAP);
                        aNewAttr.Put(aItem);
                    }
                    break;

                    // attributes for TextObjectBar
                    case SID_ATTR_CHAR_FONT:
                    case SID_ATTR_CHAR_FONTHEIGHT:
                        mrViewShell.GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SfxCallMode::ASYNCHRON );
                    break;
                    case SID_ATTR_CHAR_COLOR:
                    case SID_ATTR_CHAR_BACK_COLOR:
                    {
                        const sal_uInt16 nEEWhich
                            = aEditAttr.GetPool()->GetWhichIDFromSlotID(nSlot);
                        const std::optional<NamedColor> oColor
                            = mrViewShell.GetDocSh()->GetRecentColor(nSlot);
                        if (oColor.has_value())
                        {
                            const model::ComplexColor aCol = (*oColor).getComplexColor();
                            aNewAttr.Put(SvxColorItem(aCol.getFinalColor(), aCol, nEEWhich));
                        }
                    }
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
                if( const SvxAdjustItem* pAdjustItem = aEditAttr.GetItemIfSet(EE_PARA_JUST) )
                    nAdjust = pAdjustItem->GetAdjust();

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

                Invalidate(SID_RULER_TEXT_RIGHT_TO_LEFT);
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
                aSvxScriptSetItem.PutItemForScriptType( nScriptType, pArgs->Get( rPool.GetWhichIDFromSlotID( nSlot ) ) );
                aNewAttr.Put( aSvxScriptSetItem.GetItemSet() );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if (nSlot == SID_ATTR_PARA_ADJUST_LEFT ||
                nSlot == SID_ATTR_PARA_ADJUST_CENTER ||
                nSlot == SID_ATTR_PARA_ADJUST_RIGHT ||
                nSlot == SID_ATTR_PARA_ADJUST_BLOCK)
            {
                SvxAdjust eAdjst;
                SdrTextHorzAdjust eAnchor;
                if (nSlot == SID_ATTR_PARA_ADJUST_LEFT) {
                    eAdjst = SvxAdjust::Left;  eAnchor = SDRTEXTHORZADJUST_LEFT;
                }
                else if (nSlot == SID_ATTR_PARA_ADJUST_CENTER) {
                    eAdjst = SvxAdjust::Center;  eAnchor = SDRTEXTHORZADJUST_CENTER;
                }
                else if (nSlot == SID_ATTR_PARA_ADJUST_RIGHT) {
                    eAdjst = SvxAdjust::Right;  eAnchor = SDRTEXTHORZADJUST_RIGHT;
                }
                else {
                    eAdjst = SvxAdjust::Block;  eAnchor = SDRTEXTHORZADJUST_BLOCK;
                }

                aNewAttr.Put(SvxAdjustItem(eAdjst, EE_PARA_JUST));
                // set anchor
                ESelection aSel = pOLV->GetSelection();
                aSel.Adjust();
                sal_Int32 nStartPara = aSel.start.nPara;
                if (!aSel.HasRange())
                    nStartPara = 0;

                if (nStartPara == 0)
                    aNewAttr.Put(SdrTextHorzAdjustItem(eAnchor));

                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if(nSlot == SID_ATTR_CHAR_KERNING)
            {
                aNewAttr.Put(pArgs->Get(pArgs->GetPool()->GetWhichIDFromSlotID(nSlot)));
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if(nSlot ==  SID_SET_SUPER_SCRIPT )
            {
                SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                SvxEscapement eEsc = aEditAttr.Get(EE_CHAR_ESCAPEMENT).GetEscapement();

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
                SvxEscapement eEsc = aEditAttr.Get(EE_CHAR_ESCAPEMENT).GetEscapement();

                if( eEsc == SvxEscapement::Subscript )
                    aItem.SetEscapement( SvxEscapement::Off );
                else
                    aItem.SetEscapement( SvxEscapement::Subscript );
                aNewAttr.Put( aItem );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }

            std::unique_ptr<SfxItemSet> pNewArgs = pArgs->Clone();
            mpView->SetAttributes(*pNewArgs);

            // invalidate entire shell because of performance and
            // extension reasons
            Invalidate();

            // to refresh preview (in outline mode), slot has to be invalidated:
            mrViewShell.GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, true );
        }
        break;
    }

    if ( nSlot != SID_STYLE_APPLY && pOLV )
    {
        pOLV->ShowCursor();
        pOLV->GetWindow()->GrabFocus();
    }

    Invalidate(SID_OUTLINE_LEFT);
    Invalidate(SID_OUTLINE_RIGHT);
    Invalidate(SID_OUTLINE_UP);
    Invalidate(SID_OUTLINE_DOWN);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
