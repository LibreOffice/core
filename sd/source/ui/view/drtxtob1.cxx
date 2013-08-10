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

#include "app.hrc"

#include <editeng/outliner.hxx>
#include "ViewShell.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "OutlineView.hxx"
#include "Window.hxx"
#include "futempl.hxx"
#include "DrawDocShell.hxx"
#include "Outliner.hxx"
#include "futext.hxx"

namespace sd {

/**
 * Process SfxRequests
 */

void TextObjectBar::Execute( SfxRequest &rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 nSlot = rReq.GetSlot();
    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< OutlineViewModelChangeGuard > aGuard;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    if (mpView->ISA(OutlineView))
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
                OSL_ASSERT (mpViewShell->GetViewShell()!=NULL);
                FunctionReference xFunc( FuTemplate::Create( mpViewShell, static_cast< ::sd::Window*>( mpViewShell->GetViewShell()->GetWindow()), mpView, &rDoc, rReq ) );

                if(xFunc.is())
                {
                    xFunc->Activate();
                    xFunc->Deactivate();

                    if( rReq.GetSlot() == SID_STYLE_APPLY )
                    {
                        if( mpViewShell && mpViewShell->GetViewFrame() )
                            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                    }
                }
            }
            else
            {
                if( mpViewShell && mpViewShell->GetViewFrame() )
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_STYLE_DESIGNER, SFX_CALLMODE_ASYNCHRON );
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
                    SfxStyleSheet* pStyleSheet = NULL;
                    if (pOLV->GetOutliner() != NULL)
                        pStyleSheet = pOLV->GetOutliner()->GetStyleSheet(nPara);
                    if (pStyleSheet != NULL)
                    {
                        SfxItemSet aAttr( pStyleSheet->GetItemSet() );
                        SfxItemSet aTmpSet( pOLV->GetOutliner()->GetParaAttribs( nPara ) );
                        aAttr.Put( aTmpSet, sal_False ); // sal_False= InvalidItems is not default, handle it as "holes"
                        const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) aAttr.Get( EE_PARA_ULSPACE );
                        SvxULSpaceItem* pNewItem = (SvxULSpaceItem*) rItem.Clone();

                        long nUpper = pNewItem->GetUpper();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nUpper += 100;
                        else
                        {
                            nUpper -= 100;
                            nUpper = std::max( (long) nUpper, 0L );
                        }
                        pNewItem->SetUpper( (sal_uInt16) nUpper );

                        long nLower = pNewItem->GetLower();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nLower += 100;
                        else
                        {
                            nLower -= 100;
                            nLower = std::max( (long) nLower, 0L );
                        }
                        pNewItem->SetLower( (sal_uInt16) nLower );

                        SfxItemSet aNewAttrs( aAttr );
                        aNewAttrs.Put( *pNewItem );
                        delete pNewItem;
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
                if( aEditAttr.GetItemState( EE_PARA_ULSPACE ) >= SFX_ITEM_AVAILABLE )
                {
                    SfxItemSet aNewAttrs(*(aEditAttr.GetPool()), aEditAttr.GetRanges());
                    const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) aEditAttr.Get( EE_PARA_ULSPACE );
                    SvxULSpaceItem* pNewItem = (SvxULSpaceItem*) rItem.Clone();
                    long nUpper = pNewItem->GetUpper();

                    if( nSlot == SID_PARASPACE_INCREASE )
                        nUpper += 100;
                    else
                    {
                        nUpper -= 100;
                        nUpper = std::max( (long) nUpper, 0L );
                    }
                    pNewItem->SetUpper( (sal_uInt16) nUpper );

                    long nLower = pNewItem->GetLower();
                    if( nSlot == SID_PARASPACE_INCREASE )
                        nLower += 100;
                    else
                    {
                        nLower -= 100;
                        nLower = std::max( (long) nLower, 0L );
                    }
                    pNewItem->SetLower( (sal_uInt16) nLower );

                    aNewAttrs.Put( *pNewItem );
                    delete pNewItem;

                    mpView->SetAttributes( aNewAttrs );
                }
            }
            rReq.Done();

            Invalidate();
            // to refresh preview (in outline mode), slot has to be invalidated:
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
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
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
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
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
            }
            rReq.Done();
        }
        break;

        case SID_ATTR_PARA_LRSPACE:
        {
            sal_uInt16 nSpaceSlot = SID_ATTR_PARA_LRSPACE;
            SvxLRSpaceItem aLRSpace = (const SvxLRSpaceItem&)pArgs->Get(
                GetPool().GetWhich(nSpaceSlot));

            SfxItemSet aEditAttr( GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE );
            aLRSpace.SetWhich( EE_PARA_LRSPACE );

            aEditAttr.Put( aLRSpace );
            mpView->SetAttributes( aEditAttr );

            Invalidate(SID_ATTR_PARA_LRSPACE);
        }
        break;

        case SID_OUTLINE_UP:
        {
            if (pOLV)
            {
                pOLV->AdjustHeight( -1 );

                // trigger preview refresh
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
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
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
            }
            rReq.Done();
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        {
            mpView->SdrEndTextEdit();
            SfxItemSet aAttr( mpView->GetDoc().GetPool(), SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION, 0 );
            aAttr.Put( SvxWritingModeItem(
                nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                    com::sun::star::text::WritingMode_LR_TB : com::sun::star::text::WritingMode_TB_RL,
                    SDRATTR_TEXTDIRECTION ) );
            rReq.Done( aAttr );
            mpView->SetAttributes( aAttr );
            Invalidate();
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
        }
        break;

        case FN_NUM_BULLET_ON:
            if( pOLV )
                pOLV->ToggleBullets();
            break;

        case SID_GROW_FONT_SIZE:
        case SID_SHRINK_FONT_SIZE:
        {
            const SvxFontListItem* pFonts = (const SvxFontListItem*)mpViewShell->GetDocSh()->GetItem( SID_ATTR_CHAR_FONTLIST );
            const FontList* pFontList = pFonts ? pFonts->GetFontList(): 0;
            if( pFontList )
            {
                FuText::ChangeFontSize( nSlot == SID_GROW_FONT_SIZE, pOLV, pFontList, mpView );
                if( pOLV )
                {
                    SfxItemSet aSet( pOLV->GetEditView().GetAttribs() );
                    SfxItemSet aNewAttrs (pOLV->GetEditView().GetEmptyItemSet() );

                    aNewAttrs.Put( aSet.Get( EE_CHAR_FONTHEIGHT ), EE_CHAR_FONTHEIGHT );
                    aNewAttrs.Put( aSet.Get( EE_CHAR_FONTHEIGHT_CJK ), EE_CHAR_FONTHEIGHT_CJK );
                    aNewAttrs.Put( aSet.Get( EE_CHAR_FONTHEIGHT_CTL ), EE_CHAR_FONTHEIGHT_CTL );

                    mpView->SetAttributes( aNewAttrs );
                }
                Invalidate();
                // to refresh preview (in outline mode), slot has to be invalidated:
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
            }
            rReq.Done();
        }
        break;

        case SID_THES:
        {
            OUString aReplaceText;
            SFX_REQUEST_ARG( rReq, pItem2, SfxStringItem, SID_THES, sal_False );
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
                        FontWeight eFW = ( (const SvxWeightItem&) aEditAttr.
                                        Get( EE_CHAR_WEIGHT ) ).GetWeight();
                        aNewAttr.Put( SvxWeightItem( eFW == WEIGHT_NORMAL ?
                                            WEIGHT_BOLD : WEIGHT_NORMAL,
                                            EE_CHAR_WEIGHT ) );
                    }
                    break;
                    case SID_ATTR_CHAR_POSTURE:
                    {
                        FontItalic eFI = ( (const SvxPostureItem&) aEditAttr.
                                        Get( EE_CHAR_ITALIC ) ).GetPosture();
                        aNewAttr.Put( SvxPostureItem( eFI == ITALIC_NORMAL ?
                                            ITALIC_NONE : ITALIC_NORMAL,
                                            EE_CHAR_ITALIC ) );
                    }
                    break;
                    case SID_ATTR_CHAR_UNDERLINE:
                    {
                        FontUnderline eFU = ( (const SvxUnderlineItem&) aEditAttr.
                                        Get( EE_CHAR_UNDERLINE ) ).GetLineStyle();
                        aNewAttr.Put( SvxUnderlineItem( eFU == UNDERLINE_SINGLE ?
                                            UNDERLINE_NONE : UNDERLINE_SINGLE,
                                            EE_CHAR_UNDERLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_OVERLINE:
                    {
                        FontUnderline eFO = ( (const SvxOverlineItem&) aEditAttr.
                                        Get( EE_CHAR_OVERLINE ) ).GetLineStyle();
                        aNewAttr.Put( SvxOverlineItem( eFO == UNDERLINE_SINGLE ?
                                            UNDERLINE_NONE : UNDERLINE_SINGLE,
                                            EE_CHAR_OVERLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_CONTOUR:
                    {
                        aNewAttr.Put( SvxContourItem( !( (const SvxContourItem&) aEditAttr.
                                        Get( EE_CHAR_OUTLINE ) ).GetValue(), EE_CHAR_OUTLINE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_SHADOWED:
                    {
                        aNewAttr.Put( SvxShadowedItem( !( (const SvxShadowedItem&) aEditAttr.
                                        Get( EE_CHAR_SHADOW ) ).GetValue(), EE_CHAR_SHADOW ) );
                    }
                    break;
                    case SID_ATTR_CHAR_STRIKEOUT:
                    {
                        FontStrikeout eFSO = ( ( (const SvxCrossedOutItem&) aEditAttr.
                                        Get( EE_CHAR_STRIKEOUT ) ).GetStrikeout() );
                        aNewAttr.Put( SvxCrossedOutItem( eFSO == STRIKEOUT_SINGLE ?
                                            STRIKEOUT_NONE : STRIKEOUT_SINGLE, EE_CHAR_STRIKEOUT ) );
                    }
                    break;

                    case SID_ATTR_PARA_ADJUST_LEFT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_CENTER:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_RIGHT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_BLOCK:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_BLOCK, EE_PARA_JUST ) );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_10:
                    {
                        SvxLineSpacingItem aItem( SVX_LINESPACE_ONE_LINE, EE_PARA_SBL );
                        aItem.SetPropLineSpace( 100 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_15:
                    {
                        SvxLineSpacingItem aItem( SVX_LINESPACE_ONE_POINT_FIVE_LINES, EE_PARA_SBL );
                        aItem.SetPropLineSpace( 150 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_20:
                    {
                        SvxLineSpacingItem aItem( SVX_LINESPACE_TWO_LINES, EE_PARA_SBL );
                        aItem.SetPropLineSpace( 200 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_SET_SUPER_SCRIPT:
                    {
                        SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
                        SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                                        aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                        if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                            aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
                        else
                            aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_SET_SUB_SCRIPT:
                    {
                        SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
                        SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                                        aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                        if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                            aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
                        else
                            aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
                        aNewAttr.Put( aItem );
                    }
                    break;

                    // attributes for TextObjectBar
                    case SID_ATTR_CHAR_FONT:
                    {
                        if( pArgs )
                        {
                            if( SFX_ITEM_SET == pArgs->GetItemState( EE_CHAR_FONTINFO, sal_True, &pPoolItem ) )
                                aNewAttr.Put( *pPoolItem );
                        }
                        else
                            mpViewShell->GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SFX_CALLMODE_ASYNCHRON );
                    }
                    break;
                    case SID_ATTR_CHAR_FONTHEIGHT:
                    {
                        if( pArgs )
                        {
                            if( SFX_ITEM_SET == pArgs->GetItemState( EE_CHAR_FONTHEIGHT, sal_True, &pPoolItem ) )
                                aNewAttr.Put( *pPoolItem );
                        }
                        else
                            mpViewShell->GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SFX_CALLMODE_ASYNCHRON );
                    }
                    break;
                    case SID_ATTR_CHAR_COLOR:
                    {
                        if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( EE_CHAR_COLOR, sal_True, &pPoolItem ) )
                            aNewAttr.Put( *pPoolItem );
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
                sal_Bool bLeftToRight = nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT;

                sal_uInt16 nAdjust = SVX_ADJUST_LEFT;
                if( SFX_ITEM_ON == aEditAttr.GetItemState(EE_PARA_JUST, sal_True, &pPoolItem ) )
                    nAdjust = ( (SvxAdjustItem*)pPoolItem)->GetEnumValue();

                if( bLeftToRight )
                {
                    aNewAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );
                    if( nAdjust == SVX_ADJUST_RIGHT )
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
                }
                else
                {
                    aNewAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
                    if( nAdjust == SVX_ADJUST_LEFT )
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
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
                sal_uInt16 nScriptType = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
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
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
                    }
                    break;
                case SID_ATTR_PARA_ADJUST_CENTER:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
                    }
                    break;
                case SID_ATTR_PARA_ADJUST_RIGHT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
                    }
                    break;
                case SID_ATTR_PARA_ADJUST_BLOCK:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_BLOCK, EE_PARA_JUST ) );
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
                SvxEscapement eEsc = (SvxEscapement) ( (const SvxEscapementItem&)
                                aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                    aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
                else
                    aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
                aNewAttr.Put( aItem );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if( nSlot ==  SID_SET_SUB_SCRIPT )
            {
                SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
                SvxEscapement eEsc = (SvxEscapement) ( (const SvxEscapementItem&)
                                aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

                if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                    aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
                else
                    aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
                aNewAttr.Put( aItem );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }

            mpView->SetAttributes(*pArgs);

            // invalidate entire shell because of performance and
            // extension reasons
            Invalidate();

            // to refresh preview (in outline mode), slot has to be invalidated:
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, sal_True, sal_False );
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
