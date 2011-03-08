/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TextObjectBar.hxx"


#include <svx/svxids.hrc>

#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/outliner.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <vcl/vclenum.hxx>
#include <sfx2/app.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svl/style.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/shdditem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/editeng.hxx>
#include <svl/intitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/itemiter.hxx>


#include "app.hrc"

#include "eetext.hxx"
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

/*************************************************************************
|*
|* Bearbeitung der SfxRequests
|*
\************************************************************************/

void TextObjectBar::Execute( SfxRequest &rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pPoolItem = NULL;
    USHORT nSlot = rReq.GetSlot();
    BOOL bOutlineMode = FALSE;
    OutlinerView* pOLV = mpView->GetTextEditOutlinerView();

    std::auto_ptr< OutlineViewModelChangeGuard > aGuard;

    if (mpView->ISA(OutlineView))
    {
        bOutlineMode = TRUE;
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
                SdDrawDocument* pDoc = mpView->GetDoc();
                OSL_ASSERT (mpViewShell->GetViewShell()!=NULL);
                FunctionReference xFunc( FuTemplate::Create( mpViewShell, static_cast< ::sd::Window*>( mpViewShell->GetViewShell()->GetWindow()), mpView, pDoc, rReq ) );

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
                ULONG nStartPara = aSel.nStartPara;
                ULONG nEndPara = aSel.nEndPara;
                if( !aSel.HasRange() )
                {
                    nStartPara = 0;
                    nEndPara = pOLV->GetOutliner()->GetParagraphCount() - 1;
                }
                for( ULONG nPara = nStartPara; nPara <= nEndPara; nPara++ )
                {
                    SfxStyleSheet* pStyleSheet = NULL;
                    if (pOLV->GetOutliner() != NULL)
                        pStyleSheet = pOLV->GetOutliner()->GetStyleSheet(nPara);
                    if (pStyleSheet != NULL)
                    {
                        SfxItemSet aAttr( pStyleSheet->GetItemSet() );
                        SfxItemSet aTmpSet( pOLV->GetOutliner()->GetParaAttribs( (USHORT) nPara ) );
                        aAttr.Put( aTmpSet, FALSE ); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
                        const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) aAttr.Get( EE_PARA_ULSPACE );
                        SvxULSpaceItem* pNewItem = (SvxULSpaceItem*) rItem.Clone();

                        long nUpper = pNewItem->GetUpper();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nUpper += 100;
                        else
                        {
                            nUpper -= 100;
                            nUpper = Max( (long) nUpper, 0L );
                        }
                        pNewItem->SetUpper( (USHORT) nUpper );

                        long nLower = pNewItem->GetLower();
                        if( nSlot == SID_PARASPACE_INCREASE )
                            nLower += 100;
                        else
                        {
                            nLower -= 100;
                            nLower = Max( (long) nLower, 0L );
                        }
                        pNewItem->SetLower( (USHORT) nLower );

                        SfxItemSet aNewAttrs( aAttr );
                        aNewAttrs.Put( *pNewItem );
                        delete pNewItem;
                        pOLV->GetOutliner()->SetParaAttribs( (USHORT)nPara, aNewAttrs );
                    }
                }
            }
            else
            {
                // Der folgende Code kann enabled werden, wenn ich von
                // JOE einen richtigen Status (DontCare) bekomme;

                // Wird enabled, obwohl es nicht richtig funktioniert (s.o.)
                SfxItemSet aEditAttr( mpView->GetDoc()->GetPool() );
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
                        nUpper = Max( (long) nUpper, 0L );
                    }
                    pNewItem->SetUpper( (USHORT) nUpper );

                    long nLower = pNewItem->GetLower();
                    if( nSlot == SID_PARASPACE_INCREASE )
                        nLower += 100;
                    else
                    {
                        nLower -= 100;
                        nLower = Max( (long) nLower, 0L );
                    }
                    pNewItem->SetLower( (USHORT) nLower );

                    aNewAttrs.Put( *pNewItem );
                    delete pNewItem;

                    mpView->SetAttributes( aNewAttrs );
                }
            }
            rReq.Done();

            Invalidate();
            // Um die Preview (im Gliederungsmodus) zu aktualisieren muss
            // der Slot invalidiert werden:
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
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
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
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
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
            }
            rReq.Done();
        }
        break;

        case SID_OUTLINE_UP:
        {
            if (pOLV)
            {
                pOLV->AdjustHeight( -1 );

                // trigger preview refresh
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
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
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
            }
            rReq.Done();
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        {
            mpView->SdrEndTextEdit();
            SfxItemSet aAttr( mpView->GetDoc()->GetPool(), SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION, 0 );
            aAttr.Put( SvxWritingModeItem(
                nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                    com::sun::star::text::WritingMode_LR_TB : com::sun::star::text::WritingMode_TB_RL,
                    SDRATTR_TEXTDIRECTION ) );
            rReq.Done( aAttr );
            mpView->SetAttributes( aAttr );
            Invalidate();
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
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
                mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            rReq.Done();
        }
        break;

        case SID_THES:
        {
            String aReplaceText;
            SFX_REQUEST_ARG( rReq, pItem2, SfxStringItem, SID_THES, sal_False );
            if (pItem2)
                aReplaceText = pItem2->GetValue();
            if (aReplaceText.Len() > 0)
                ReplaceTextWithSynonym( pOLV->GetEditView(), aReplaceText );
        }
        break;

        default:
        {
            SfxItemSet aEditAttr( mpView->GetDoc()->GetPool() );
            mpView->GetAttributes( aEditAttr );
            SfxItemSet aNewAttr(*(aEditAttr.GetPool()), aEditAttr.GetRanges());

            if( !pArgs )
            {
                //aNewAttr.InvalidateAllItems(); <- Macht Probleme (#35465#)

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

                    // Attribute fuer die TextObjectBar
                    case SID_ATTR_CHAR_FONT:
                    {
                        if( pArgs )
                        {
                            if( SFX_ITEM_SET == pArgs->GetItemState( EE_CHAR_FONTINFO, TRUE, &pPoolItem ) )
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
                            if( SFX_ITEM_SET == pArgs->GetItemState( EE_CHAR_FONTHEIGHT, TRUE, &pPoolItem ) )
                                aNewAttr.Put( *pPoolItem );
                        }
                        else
                            mpViewShell->GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SFX_CALLMODE_ASYNCHRON );
                    }
                    break;
                    case SID_ATTR_CHAR_COLOR:
                    {
                        if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( EE_CHAR_COLOR, TRUE, &pPoolItem ) )
                            aNewAttr.Put( *pPoolItem );
                    }
                    break;
/* #i35937#
                    case FN_NUM_BULLET_ON:
                    {
                        if (aEditAttr.GetItemState(EE_PARA_BULLETSTATE) == SFX_ITEM_ON)
                        {
                            SfxUInt16Item aBulletState((const SfxUInt16Item&) aEditAttr.Get(EE_PARA_BULLETSTATE));

                            if (aBulletState.GetValue() != 0)
                            {
                                // Ausschalten
                                aNewAttr.Put(SfxUInt16Item(EE_PARA_BULLETSTATE, 0));
                            }
                            else
                            {
                                // Einschalten
                                aNewAttr.Put(SfxUInt16Item(EE_PARA_BULLETSTATE, 1));
                            }
                        }
                        else
                        {
                            // Einschalten
                            aNewAttr.Put(SfxUInt16Item(EE_PARA_BULLETSTATE, 1));
                        }
                    }
                    break;
*/
                }

                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }
            else if ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT ||
                      nSlot == SID_ATTR_PARA_RIGHT_TO_LEFT )
            {
                sal_Bool bLeftToRight = nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT;

                USHORT nAdjust = SVX_ADJUST_LEFT;
                if( SFX_ITEM_ON == aEditAttr.GetItemState(EE_PARA_JUST, TRUE, &pPoolItem ) )
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
                USHORT nScriptType = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
                if (nSlot == SID_ATTR_CHAR_FONT)
                    nScriptType = mpView->GetScriptType();

                SfxItemPool& rPool = mpView->GetDoc()->GetPool();
                SvxScriptSetItem aSvxScriptSetItem( nSlot, rPool );
                aSvxScriptSetItem.PutItemForScriptType( nScriptType, pArgs->Get( rPool.GetWhich( nSlot ) ) );
                aNewAttr.Put( aSvxScriptSetItem.GetItemSet() );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }

            mpView->SetAttributes(*pArgs);

            // Aus Performance- und Erweiterungsgruenden wird
            // jetzt die komplette Shell invalidiert
            Invalidate();

            // Um die Preview (im Gliederungsmodus) zu aktualisieren muss
            // der Slot invalidiert werden:
            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
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
