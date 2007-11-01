/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drtxtob1.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:29:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TextObjectBar.hxx"


#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _LSPCITEM_HXX
#include <svx/lspcitem.hxx>
#endif
#ifndef _ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#include <svx/xtable.hxx>
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_SRIPTTYPEITEM_HXX //autogen
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#ifndef _SVX_WRITINGMODEITEM_HXX
#include <svx/writingmodeitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif


#include "app.hrc"

#include "eetext.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_OUTLINE_VIEW_HXX
#include "OutlineView.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_FU_TEMPLATE_HXX
#include "futempl.hxx"
#endif

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
                    SfxItemSet aAttr( pOLV->GetOutliner()->GetStyleSheet( nPara )->GetItemSet() );
                    SfxItemSet aTmpSet( pOLV->GetOutliner()->GetParaAttribs( nPara ) );
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
                    pOLV->GetOutliner()->SetParaAttribs( nPara, aNewAttrs );
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

                // #96551# trigger preview refresh
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

                // #96551# trigger preview refresh
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

                // #96551# trigger preview refresh
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

                // #96551# trigger preview refresh
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
                                        Get( EE_CHAR_UNDERLINE ) ).GetUnderline();
                        aNewAttr.Put( SvxUnderlineItem( eFU == UNDERLINE_SINGLE ?
                                            UNDERLINE_NONE : UNDERLINE_SINGLE,
                                            EE_CHAR_UNDERLINE ) );
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
