/*************************************************************************
 *
 *  $RCSfile: drwtxtex.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-27 21:45:13 $
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
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_PARAITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX
#   ifndef ITEMID_FIELD
#       ifndef _MyEDITDATA_HXX //autogen
#           include <svx/editdata.hxx>  // das include wird wg. EE_FEATURE_FIELD benoetigt
#       endif
#       define ITEMID_FIELD EE_FEATURE_FIELD  /* wird fuer #include <flditem.hxx> benoetigt */
#   endif
#   ifndef _SVX_FLDITEM_HXX //autogen
#       include <svx/flditem.hxx>
#   endif
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _CHRDLG_HXX
#include <chrdlg.hxx>
#endif
#ifndef _PARDLG_HXX
#include <pardlg.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _DRWTXTSH_HXX
#include <drwtxtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>               // fuer SpellPointer
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDrawTextShell::Execute( SfxRequest &rReq )
{
    SwWrtShell &rSh = GetShell();

    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    sal_uInt16 nSlot = rReq.GetSlot();
    sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    const SfxItemSet *pNewAttrs = rReq.GetArgs();

    switch (nSlot)
    {
    case SID_ATTR_CHAR_FONT:
    case SID_ATTR_CHAR_FONTHEIGHT:
    case SID_ATTR_CHAR_WEIGHT:
    case SID_ATTR_CHAR_POSTURE:
        {
            SfxItemPool* pPool = aEditAttr.GetPool()->GetSecondaryPool();
            if( !pPool )
                pPool = aEditAttr.GetPool();
            SvxScriptSetItem aSetItem( nSlot, *pPool );
            aSetItem.PutItemForScriptType( pOLV->GetSelectedScriptType(),
                                            pNewAttrs->Get( nWhich ));
            aNewAttr.Put( aSetItem.GetItemSet() );
        }
        break;

    case SID_ATTR_CHAR_COLOR:
        aNewAttr.Put( pNewAttrs->Get(nWhich), EE_CHAR_COLOR );
        break;

        case SID_ATTR_CHAR_UNDERLINE:
        {
             FontUnderline eFU = ((const SvxUnderlineItem&)aEditAttr.Get(EE_CHAR_UNDERLINE)).GetUnderline();
            aNewAttr.Put(SvxUnderlineItem(eFU == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_UNDERLINE));
        }
        break;

        case SID_ATTR_CHAR_CONTOUR:
            aNewAttr.Put((const SvxContourItem&)pNewAttrs->Get(nWhich), EE_CHAR_OUTLINE);
            break;
        case SID_ATTR_CHAR_SHADOWED:
            aNewAttr.Put((const SvxShadowedItem&)pNewAttrs->Get(nWhich), EE_CHAR_SHADOW);
            break;
        case SID_ATTR_CHAR_STRIKEOUT:
            aNewAttr.Put((const SvxCrossedOutItem&)pNewAttrs->Get(nWhich), EE_CHAR_STRIKEOUT);
            break;

        case SID_ATTR_PARA_ADJUST_LEFT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_LEFT, EE_PARA_JUST));
            break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST));
            break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_RIGHT, EE_PARA_JUST));
            break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_BLOCK, EE_PARA_JUST));
            break;

        case SID_ATTR_PARA_LINESPACE_10:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_LINE, EE_PARA_SBL);
            aItem.SetPropLineSpace(100);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_15:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_POINT_FIVE_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(150);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_20:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_TWO_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(200);
            aNewAttr.Put(aItem);
        }
        break;

        case FN_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
        }
        break;
        case FN_SET_SUB_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
        }
        break;

        case SID_CHAR_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if( !pArgs )
            {
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                SfxItemSet aDlgAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);

                // util::Language gibts an der EditEngine nicht! Daher nicht im Set.

                aDlgAttr.Put( aEditAttr );
                aDlgAttr.Put( SvxKerningItem() );

                SwCharDlg* pDlg = new SwCharDlg(pView->GetWindow(), *pView, aDlgAttr, 0, sal_True);
                USHORT nRet = pDlg->Execute();
                if(RET_OK == nRet )
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                delete( pDlg );
                if(RET_OK != nRet)
                    return ;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;

        case SID_PARA_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (!pArgs)
            {
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                SfxItemSet aDlgAttr(GetPool(),
                                    EE_ITEMS_START, EE_ITEMS_END,
                                    SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                                    SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                                    SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                                    SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                                    0);

                aDlgAttr.Put(aEditAttr);

                // Die Werte sind erst einmal uebernommen worden, um den Dialog anzuzeigen.
                // Muss natuerlich noch geaendert werden
                // aDlgAttr.Put( SvxParaDlgLimitsItem( 567 * 50, 5670) );

                aDlgAttr.Put( SvxHyphenZoneItem() );
                aDlgAttr.Put( SvxFmtBreakItem() );
                aDlgAttr.Put( SvxFmtSplitItem() );
                aDlgAttr.Put( SvxWidowsItem() );
                aDlgAttr.Put( SvxOrphansItem() );

                SwParaDlg* pDlg = new SwParaDlg(GetView().GetWindow(), GetView(), aDlgAttr, DLG_STD, 0, sal_True);

                USHORT nRet = pDlg->Execute();
                if(RET_OK == nRet)
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                delete( pDlg );
                if(RET_OK != nRet)
                    return;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;
        case SID_AUTOSPELL_MARKOFF:
        case SID_AUTOSPELL_CHECK:
        {
//!! JP 16.03.2001: why??           pSdrView = rSh.GetDrawView();
//!! JP 16.03.2001: why??           pOutliner = pSdrView->GetTextEditOutliner();
            sal_uInt32 nCtrl = pOutliner->GetControlWord();

            sal_Bool bSet = ((const SfxBoolItem&)rReq.GetArgs()->Get(
                                                    nSlot)).GetValue();
            if(nSlot == SID_AUTOSPELL_MARKOFF)
            {
                if(bSet)
                    nCtrl |= EE_CNTRL_NOREDLINES;
                else
                    nCtrl &= ~EE_CNTRL_NOREDLINES;
            }
            else
            {
                if(bSet)
                    nCtrl |= EE_CNTRL_ONLINESPELLING|EE_CNTRL_ALLOWBIGOBJS;
                else
                    nCtrl &= ~EE_CNTRL_ONLINESPELLING;
            }
            pOutliner->SetControlWord(nCtrl);

            SW_MOD()->ExecuteSlot(rReq);
        }
        break;
        case SID_HYPERLINK_SETLINK:
        {
            const SfxPoolItem* pItem = 0;
            if(pNewAttrs)
                pNewAttrs->GetItemState(nSlot, sal_False, &pItem);

            if(pItem)
            {
                const SvxHyperlinkItem& rHLinkItem = *(const SvxHyperlinkItem *)pItem;
                SvxURLField aFld(rHLinkItem.GetURL(), rHLinkItem.GetName(), SVXURLFORMAT_APPDEFAULT);
                aFld.SetTargetFrame(rHLinkItem.GetTargetFrame());

                const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

                if (pFieldItem && pFieldItem->GetField()->ISA(SvxURLField))
                {
                    // Feld selektieren, so dass es beim Insert geloescht wird
                    ESelection aSel = pOLV->GetSelection();
                    aSel.nEndPos++;
                    pOLV->SetSelection(aSel);
                }
                pOLV->InsertField(aFld);
            }
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            // Shellwechsel!
            {
                SdrObject* pTmpObj = pSdrView->GetMarkList().GetMark(0)->GetObj();
                SdrPageView* pTmpPV = pSdrView->GetPageViewPvNum(0);
                SdrView* pTmpView = pSdrView;

                rSh.EndTextEdit();

                SfxItemSet aAttr( *aNewAttr.GetPool(),
                            SDRATTR_TEXTDIRECTION_LEFT_TO_RIGHT,
                            SDRATTR_TEXTDIRECTION_LEFT_TO_RIGHT );
                aAttr.Put( SfxBoolItem( SDRATTR_TEXTDIRECTION_LEFT_TO_RIGHT,
                        BOOL( nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ) ) );
                pTmpView->SetAttributes( aAttr );

                rSh.GetView().BeginTextEdit( pTmpObj, pTmpPV,
                                    &rSh.GetView().GetEditWin(), FALSE );
                rSh.GetView().AttrChangedNotify( &rSh );
            }
            return;


        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
    SetAttrToMarked(aNewAttr);

    GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    if (IsTextEdit() && pOLV->GetOutliner()->IsModified())
        rSh.SetModified();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDrawTextShell::GetState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Sonst manchmal Absturz!
        return;

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    SfxItemSet aEditAttr( pOLV->GetAttribs() );
    const SfxPoolItem *pAdjust = 0, *pLSpace = 0, *pEscItem = 0;
    int eAdjust, nLSpace, nEsc;

    while(nWhich)
    {
        USHORT nSlotId = GetPool().GetSlotId( nWhich );
        BOOL bFlag = FALSE;
        switch( nSlotId )
        {
         case SID_ATTR_PARA_ADJUST_LEFT:    eAdjust = SVX_ADJUST_LEFT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_RIGHT:    eAdjust = SVX_ADJUST_RIGHT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_CENTER:   eAdjust = SVX_ADJUST_CENTER; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_BLOCK:    eAdjust = SVX_ADJUST_BLOCK; goto ASK_ADJUST;
ASK_ADJUST:
            {
                if( !pAdjust )
                    aEditAttr.GetItemState( EE_PARA_JUST, sal_False, &pAdjust);

                if( !pAdjust || IsInvalidItem( pAdjust ))
                    rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                else
                    bFlag = eAdjust == ((SvxAdjustItem*)pAdjust)->GetAdjust();
            }
            break;

        case SID_ATTR_PARA_LINESPACE_10:    nLSpace = 100;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_15:    nLSpace = 150;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_20:    nLSpace = 200;  goto ASK_LINESPACE;
ASK_LINESPACE:
            {
                if( !pLSpace )
                    aEditAttr.GetItemState( EE_PARA_SBL, sal_False, &pLSpace );

                if( !pLSpace || IsInvalidItem( pLSpace ))
                    rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                else if( nLSpace == ((const SvxLineSpacingItem*)pLSpace)->
                                                GetPropLineSpace() )
                    bFlag = sal_True;
                else
                    nSlotId = 0;
            }
            break;

        case FN_SET_SUPER_SCRIPT:   nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
                                    goto ASK_ESCAPE;
        case FN_SET_SUB_SCRIPT:     nEsc = SVX_ESCAPEMENT_SUBSCRIPT;
                                    goto ASK_ESCAPE;
ASK_ESCAPE:
            {
                if( !pEscItem )
                    pEscItem = &aEditAttr.Get( EE_CHAR_ESCAPEMENT );

                if( nEsc == ((const SvxEscapementItem*)
                                                pEscItem)->GetEnumValue() )
                    bFlag = sal_True;
                else
                    nSlotId = 0;
            }
            break;

        case FN_THESAURUS_DLG:
            {
                // disable "Thesaurus" if the language is not supported
                const SfxPoolItem &rItem = GetShell().GetDoc()->GetDefault(
                                GetWhichOfScript( RES_CHRATR_LANGUAGE,
                                GetScriptTypeOfLanguage( GetAppLanguage())) );
                LanguageType nLang = ((const SvxLanguageItem &)
                                                        rItem).GetLanguage();
                //
                uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                if (!xThes.is() || nLang == LANGUAGE_NONE ||
                    !xThes->hasLocale( SvxCreateLocale( nLang ) ))
                    rSet.DisableItem( FN_THESAURUS_DLG );
                nSlotId = 0;
            }
            break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            if( pOutliner )
                bFlag = pOutliner->IsVertical() ==
                        (SID_TEXTDIRECTION_TOP_TO_BOTTOM == nSlotId);
            else
                bFlag = (SID_TEXTDIRECTION_LEFT_TO_RIGHT == nSlotId) ==
                        ((SfxBoolItem&)aEditAttr.Get(
                            SDRATTR_TEXTDIRECTION_LEFT_TO_RIGHT )).GetValue();
            break;

        default:
            nSlotId = 0;                // don't know this slot
            break;
        }

        if( nSlotId )
            rSet.Put( SfxBoolItem( nWhich, bFlag ));

        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::GetDrawTxtCtrlState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    USHORT nScriptType = pOLV->GetSelectedScriptType();

    while(nWhich)
    {
        USHORT nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
            {
                SfxItemPool* pPool = aEditAttr.GetPool()->GetSecondaryPool();
                if( !pPool )
                    pPool = aEditAttr.GetPool();
                SvxScriptSetItem aSetItem( nSlotId, *pPool );
                aSetItem.GetItemSet().Put( aEditAttr, FALSE );
                const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                if( pI )
                    rSet.Put( *pI, nWhich );
                else
                    rSet.InvalidateItem( nWhich );
            }
            break;



        case SID_ATTR_CHAR_COLOR:
            rSet.Put(aEditAttr.Get(EE_CHAR_COLOR, sal_True), nWhich);
            break;
        case SID_ATTR_CHAR_UNDERLINE:
            rSet.Put(aEditAttr.Get(EE_CHAR_UNDERLINE, sal_True), nWhich);
            break;
        case SID_ATTR_CHAR_CONTOUR:
            rSet.Put(aEditAttr.Get(EE_CHAR_OUTLINE, sal_True), nWhich);
            break;
        case SID_ATTR_CHAR_SHADOWED:
            rSet.Put(aEditAttr.Get(EE_CHAR_SHADOW, sal_True), nWhich);
            break;
        case SID_ATTR_CHAR_STRIKEOUT:
            rSet.Put(aEditAttr.Get(EE_CHAR_STRIKEOUT, sal_True), nWhich);
            break;
        case SID_AUTOSPELL_MARKOFF:
        case SID_AUTOSPELL_CHECK:
            const SfxPoolItem* pState = SW_MOD()->GetSlotState(nWhich);
            if (pState)
                rSet.Put(SfxBoolItem(nWhich, ((const SfxBoolItem*)pState)->GetValue()));
            else
                rSet.DisableItem( nWhich );
        break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecClpbrd(SfxRequest &rReq)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_CUT:
            pOLV->Cut();
            return;

        case SID_COPY:
            pOLV->Copy();
            return;

        case SID_PASTE:
            pOLV->PasteSpecial();
            break;

        default:
            DBG_ERROR("falscher Dispatcher");
            return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   ClipBoard-Status
 --------------------------------------------------------------------*/



void SwDrawTextShell::StateClpbrd(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    ESelection aSel(pOLV->GetSelection());
    const sal_Bool bCopy = (aSel.nStartPara != aSel.nEndPara) ||
                           (aSel.nStartPos != aSel.nEndPos);


    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_CUT:
            case SID_COPY:
                if( !bCopy )
                    rSet.DisableItem( nWhich );
                break;

            case SID_PASTE:
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard() );

                    if( !aDataHelper.GetTransferable().is() ||
                        !SwTransferable::IsPaste( GetShell(), aDataHelper ))
                        rSet.DisableItem( SID_PASTE );
                }
                break;

            case FN_PASTESPECIAL:
                rSet.DisableItem( FN_PASTESPECIAL );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Hyperlink-Status
 --------------------------------------------------------------------*/

void SwDrawTextShell::StateInsert(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Sonst Absturz!
        return;

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    aHLinkItem.SetInsertMode(HLINK_FIELD);

                    const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

                    if (pFieldItem)
                    {
                        const SvxFieldData* pField = pFieldItem->GetField();

                        if (pField->ISA(SvxURLField))
                        {
                            aHLinkItem.SetName(((const SvxURLField*) pField)->GetRepresentation());
                            aHLinkItem.SetURL(((const SvxURLField*) pField)->GetURL());
                            aHLinkItem.SetTargetFrame(((const SvxURLField*) pField)->GetTargetFrame());
                        }
                    }
                    else
                    {
                        String sSel(pOLV->GetSelected());
                        sSel.Erase(255);
                        sSel.EraseTrailingChars();
                        aHLinkItem.SetName(sSel);
                    }

                    sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}




