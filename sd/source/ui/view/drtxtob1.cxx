/*************************************************************************
 *
 *  $RCSfile: drtxtob1.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dl $ $Date: 2001-03-05 12:27:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
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
#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
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


#pragma hdrstop

#include "app.hrc"

#include "eetext.hxx"
#include "viewshel.hxx"
#include "drtxtob.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "outlview.hxx"
#include "sdwindow.hxx"
#include "futempl.hxx"

/*************************************************************************
|*
|* Bearbeitung der SfxRequests
|*
\************************************************************************/

void SdDrawTextObjectBar::Execute( SfxRequest &rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pPoolItem = NULL;
    USHORT nSlot = rReq.GetSlot();
    BOOL bOutlineMode = FALSE;
    OutlinerView* pOLV = pView->GetTextEditOutlinerView();

    if (pView->ISA(SdOutlineView))
    {
        bOutlineMode = TRUE;
        pOLV = ((SdOutlineView*) pView)->GetViewByWindow(pViewShell->GetActiveWindow());
    }

    switch (nSlot)
    {
        case SID_STYLE_APPLY:
        {
            if( pArgs )
            {
                SdDrawDocument* pDoc = pView->GetDoc();
                FuPoor* pFuActual = new FuTemplate( pViewShell,
                                    (SdWindow*) pViewShell->GetWindow(),
                                    pView, pDoc, rReq );

                if (pFuActual)
                {
                    pFuActual->Activate();
                    pFuActual->Deactivate();
                    delete pFuActual;

                    if( rReq.GetSlot() == SID_STYLE_APPLY )
                        pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                }
            }
            else
                pViewShell->GetViewFrame()->GetDispatcher()->
                Execute( SID_STYLE_DESIGNER, SFX_CALLMODE_ASYNCHRON );

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
                    const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) aAttr.Get( ITEMID_ULSPACE );
                    SvxULSpaceItem* pNewItem = (SvxULSpaceItem*) rItem.Clone();

                    USHORT nUpper = pNewItem->GetUpper();
                    if( nSlot == SID_PARASPACE_INCREASE )
                        nUpper += 100;
                    else
                    {
                        nUpper -= 100;
                        nUpper = (USHORT) Max( (long) nUpper, 0L );
                    }
                    pNewItem->SetUpper( nUpper );

                    USHORT nLower = pNewItem->GetLower();
                    if( nSlot == SID_PARASPACE_INCREASE )
                        nLower += 100;
                    else
                    {
                        nLower -= 100;
                        nLower = (USHORT) Max( (long) nLower, 0L );
                    }
                    pNewItem->SetLower( nLower );

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
                SfxItemSet aEditAttr( pView->GetDoc()->GetPool() );
                pView->GetAttributes( aEditAttr );
                if( aEditAttr.GetItemState( ITEMID_ULSPACE ) >= SFX_ITEM_AVAILABLE )
                {
                    SfxItemSet aNewAttrs(*(aEditAttr.GetPool()), aEditAttr.GetRanges());
                    const SvxULSpaceItem& rItem = (const SvxULSpaceItem&) aEditAttr.Get( ITEMID_ULSPACE );
                    SvxULSpaceItem* pNewItem = (SvxULSpaceItem*) rItem.Clone();
                    USHORT nUpper = pNewItem->GetUpper();

                    if( nSlot == SID_PARASPACE_INCREASE )
                        nUpper += 100;
                    else
                    {
                        nUpper -= 100;
                        nUpper = (USHORT) Max( (long) nUpper, 0L );
                    }
                    pNewItem->SetUpper( nUpper );

                    USHORT nLower = pNewItem->GetLower();
                    if( nSlot == SID_PARASPACE_INCREASE )
                        nLower += 100;
                    else
                    {
                        nLower -= 100;
                        nLower = (USHORT) Max( (long) nLower, 0L );
                    }
                    pNewItem->SetLower( nLower );

                    aNewAttrs.Put( *pNewItem );
                    delete pNewItem;

                    pView->SetAttributes( aNewAttrs );
                }
            }
            rReq.Done();

            Invalidate();
            // Um die Preview (im Gliederungsmodus) zu aktualisieren muss
            // der Slot invalidiert werden:
            pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
        }
        break;

        case SID_OUTLINE_LEFT:
        {
            if (pOLV)
            {
                pOLV->AdjustDepth( -1 );
            }
            rReq.Done();
        }
        break;

        case SID_OUTLINE_RIGHT:
        {
            if (pOLV)
            {
                pOLV->AdjustDepth( 1 );
            }
            rReq.Done();
        }
        break;

        case SID_OUTLINE_UP:
        {
            if (pOLV)
            {
                pOLV->AdjustHeight( -1 );
            }
            rReq.Done();
        }
        break;

        case SID_OUTLINE_DOWN:
        {
            if (pOLV)
            {
                pOLV->AdjustHeight( 1 );
            }
            rReq.Done();
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        {
            SdrOutliner* pOutl = pView->GetTextEditOutliner();
            if( pOutl )
            {
                if( nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT )
                {
                    if( pOutl->IsVertical() )
                        pOutl->SetVertical( FALSE );
                }
                else
                {
                    if( !pOutl->IsVertical() )
                        pOutl->SetVertical( TRUE );
                }
            }

            SfxItemSet aAttr( pView->GetDoc()->GetPool(), SID_TEXTDIRECTION_LEFT_TO_RIGHT, SID_TEXTDIRECTION_TOP_TO_BOTTOM, 0 );
            aAttr.Put( SfxBoolItem( nSlot, TRUE ) );
            rReq.Done( aAttr );
            pView->SetAttributes( aAttr );

            Invalidate();
            pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
        }
        break;

        default:
        {
            SfxItemSet aEditAttr( pView->GetDoc()->GetPool() );
            pView->GetAttributes( aEditAttr );
            SfxItemSet aNewAttr(*(aEditAttr.GetPool()), aEditAttr.GetRanges());

            if( !pArgs )
            {
                //aNewAttr.InvalidateAllItems(); <- Macht Probleme (#35465#)

                switch ( nSlot )
                {
                    case SID_ATTR_CHAR_WEIGHT:
                    {
                        FontWeight eFW = ( (const SvxWeightItem&) aEditAttr.
                                        Get( ITEMID_WEIGHT ) ).GetWeight();
                        aNewAttr.Put( SvxWeightItem( eFW == WEIGHT_NORMAL ?
                                            WEIGHT_BOLD : WEIGHT_NORMAL ) );
                    }
                    break;
                    case SID_ATTR_CHAR_POSTURE:
                    {
                        FontItalic eFI = ( (const SvxPostureItem&) aEditAttr.
                                        Get( ITEMID_POSTURE ) ).GetPosture();
                        aNewAttr.Put( SvxPostureItem( eFI == ITALIC_NORMAL ?
                                            ITALIC_NONE : ITALIC_NORMAL ) );
                    }
                    break;
                    case SID_ATTR_CHAR_UNDERLINE:
                    {
                        FontUnderline eFU = ( (const SvxUnderlineItem&) aEditAttr.
                                        Get( ITEMID_UNDERLINE ) ).GetUnderline();
                        aNewAttr.Put( SvxUnderlineItem( eFU == UNDERLINE_SINGLE ?
                                            UNDERLINE_NONE : UNDERLINE_SINGLE ) );
                    }
                    break;
                    case SID_ATTR_CHAR_CONTOUR:
                    {
                        aNewAttr.Put( SvxContourItem( !( (const SvxContourItem&) aEditAttr.
                                        Get( ITEMID_CONTOUR ) ).GetValue() ) );
                    }
                    break;
                    case SID_ATTR_CHAR_SHADOWED:
                    {
                        aNewAttr.Put( SvxShadowedItem( !( (const SvxShadowedItem&) aEditAttr.
                                        Get( ITEMID_SHADOWED ) ).GetValue() ) );
                    }
                    break;
                    case SID_ATTR_CHAR_STRIKEOUT:
                    {
                        FontStrikeout eFSO = ( ( (const SvxCrossedOutItem&) aEditAttr.
                                        Get( ITEMID_CROSSEDOUT ) ).GetStrikeout() );
                        aNewAttr.Put( SvxCrossedOutItem( eFSO == STRIKEOUT_SINGLE ?
                                            STRIKEOUT_NONE : STRIKEOUT_SINGLE ) );
                    }
                    break;

                    case SID_ATTR_PARA_ADJUST_LEFT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_CENTER:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_RIGHT:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT ) );
                    }
                    break;
                    case SID_ATTR_PARA_ADJUST_BLOCK:
                    {
                        aNewAttr.Put( SvxAdjustItem( SVX_ADJUST_BLOCK ) );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_10:
                    {
                        SvxLineSpacingItem aItem( SVX_LINESPACE_ONE_LINE );
                        aItem.SetPropLineSpace( 100 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_15:
                    {
                        SvxLineSpacingItem aItem( SVX_LINESPACE_ONE_POINT_FIVE_LINES );
                        aItem.SetPropLineSpace( 150 );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_ATTR_PARA_LINESPACE_20:
                    {
                        SvxLineSpacingItem aItem( SVX_LINESPACE_TWO_LINES );
                        aItem.SetPropLineSpace( 200 );
                        aNewAttr.Put( aItem );
                    }
                    break;

                    case SID_SET_SUPER_SCRIPT:
                    {
                        SvxEscapementItem aItem;
                        SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                                        aEditAttr.Get( ITEMID_ESCAPEMENT ) ).GetEnumValue();

                        if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                            aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
                        else
                            aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
                        aNewAttr.Put( aItem );
                    }
                    break;
                    case SID_SET_SUB_SCRIPT:
                    {
                        SvxEscapementItem aItem;
                        SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                                        aEditAttr.Get( ITEMID_ESCAPEMENT ) ).GetEnumValue();

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
                            if( SFX_ITEM_SET == pArgs->GetItemState( ITEMID_FONT, TRUE, &pPoolItem ) )
                                aNewAttr.Put( *pPoolItem );
                        }
                        else
                            pViewShell->GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SFX_CALLMODE_ASYNCHRON );
                    }
                    break;
                    case SID_ATTR_CHAR_FONTHEIGHT:
                    {
                        if( pArgs )
                        {
                            if( SFX_ITEM_SET == pArgs->GetItemState( ITEMID_FONTHEIGHT, TRUE, &pPoolItem ) )
                                aNewAttr.Put( *pPoolItem );
                        }
                        else
                            pViewShell->GetViewFrame()->GetDispatcher()->
                            Execute( SID_CHAR_DLG, SFX_CALLMODE_ASYNCHRON );
                    }
                    break;
                    case SID_ATTR_CHAR_COLOR:
                    {
                        if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( ITEMID_COLOR, TRUE, &pPoolItem ) )
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
            else if ( nSlot == SID_ATTR_CHAR_FONT       ||
                      nSlot == SID_ATTR_CHAR_FONTHEIGHT ||
                      nSlot == SID_ATTR_CHAR_POSTURE    ||
                      nSlot == SID_ATTR_CHAR_WEIGHT )
            {
                USHORT nScriptType = pView->GetScriptType();
                SfxItemPool& rPool = pView->GetDoc()->GetPool();
                SvxScriptSetItem aSvxScriptSetItem( nSlot, rPool );
                aSvxScriptSetItem.PutItemForScriptType( nScriptType, pArgs->Get( rPool.GetWhich( nSlot ) ) );
                aNewAttr.Put( *aSvxScriptSetItem.GetItemOfScript( nScriptType ) );
                rReq.Done( aNewAttr );
                pArgs = rReq.GetArgs();
            }

            pView->SetAttributes(*pArgs);

            // Aus Performance- und Erweiterungsgruenden wird
            // jetzt die komplette Shell invalidiert
            Invalidate();

            // Um die Preview (im Gliederungsmodus) zu aktualisieren muss
            // der Slot invalidiert werden:
            pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_PREVIEW_STATE, TRUE, FALSE );
        }
        break;
    }

    Invalidate( SID_OUTLINE_LEFT );
    Invalidate( SID_OUTLINE_RIGHT );
    Invalidate( SID_OUTLINE_UP );
    Invalidate( SID_OUTLINE_DOWN );
}

