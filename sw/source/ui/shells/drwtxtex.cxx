/*************************************************************************
 *
 *  $RCSfile: drwtxtex.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
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

#include "hintids.hxx"

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
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
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

#include "doc.hxx"

#ifndef _COM_SUN_STAR_LINGUISTIC_XTHESAURUS_HPP_
#include <com/sun/star/linguistic/XThesaurus.hpp>
#endif

#include "wview.hxx"
#include "viewopt.hxx"
#include "wrtsh.hxx"
#include "uiparam.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "shells.hrc"
#include "chrdlg.hxx"
#include "pardlg.hxx"
#include "dataex.hxx"
#include "drwtxtsh.hxx"
#include "swmodule.hxx"
#include "initui.hxx"               // fuer SpellPointer

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::Execute(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();

    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*(aEditAttr.GetPool()), aEditAttr.GetRanges());

    sal_uInt16 nSlot = rReq.GetSlot();
    sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    const SfxItemSet *pNewAttrs = rReq.GetArgs();

    switch (nSlot)
    {
        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_COLOR:
        {
            if (!pNewAttrs)
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(SID_CHAR_DLG, sal_False);
            else
            {
                switch (nSlot)
                {
                    case SID_ATTR_CHAR_FONT:
                        aNewAttr.Put(((const SvxFontItem&)pNewAttrs->Get(nWhich)), EE_CHAR_FONTINFO);
                        break;
                    case SID_ATTR_CHAR_FONTHEIGHT:
                        aNewAttr.Put(((const SvxFontHeightItem&)pNewAttrs->Get(nWhich)), EE_CHAR_FONTHEIGHT);
                        break;
                    case SID_ATTR_CHAR_COLOR:
                        aNewAttr.Put(((const SvxColorItem&)pNewAttrs->Get(nWhich)), EE_CHAR_COLOR);
                        break;
                }
            }
        }
        break;

        case SID_ATTR_CHAR_WEIGHT:
        {
             FontWeight eFW = ((const SvxWeightItem&)aEditAttr.Get(EE_CHAR_WEIGHT)).GetWeight();
            aNewAttr.Put(SvxWeightItem(eFW == WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL, EE_CHAR_WEIGHT));
        }
        break;

        case SID_ATTR_CHAR_POSTURE:
        {
            FontItalic eFI = ((const SvxPostureItem&)aEditAttr.Get(EE_CHAR_ITALIC)).GetPosture();
            aNewAttr.Put(SvxPostureItem(eFI == ITALIC_NORMAL ? ITALIC_NONE : ITALIC_NORMAL, EE_CHAR_ITALIC));
        }
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

                sal_uInt16 nResult = pDlg->Execute();

                switch( nResult )
                {
                    case RET_OK:
                    {
                        pArgs = rReq.GetArgs();
                    }
                    break;

                    default:
                    {
                        delete pDlg;
                    }
                    return; // Abbruch
                }
                delete( pDlg );
            }
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

                sal_uInt16 nResult = pDlg->Execute();

                switch( nResult )
                {
                    case RET_OK:
                    {
                        pArgs = rReq.GetArgs();
                    }
                    break;

                    default:
                    {
                        delete pDlg;
                    }
                    return; // Abbruch
                }
                delete( pDlg );
            }
            aNewAttr.Put(*pArgs);
        }
        break;
        case SID_AUTOSPELL_MARKOFF:
        case SID_AUTOSPELL_CHECK:
        {
            pSdrView = rSh.GetDrawView();
            pOutliner = pSdrView->GetTextEditOutliner();
            sal_uInt32 nCtrl = pOutliner->GetControlWord();

            sal_Bool bSet = ((const SfxBoolItem&)rReq.GetArgs()->Get(nSlot)).GetValue();
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

    SfxItemSet aEditAttr(pOLV->GetAttribs());
    sal_uInt16 nWhich = 0;

    const SfxPoolItem* pItem = 0;
    SfxItemState eState = aEditAttr.GetItemState(EE_PARA_JUST, sal_False, &pItem);
    if (eState == SFX_ITEM_DONTCARE || !pItem)
    {
        rSet.InvalidateItem( SID_ATTR_PARA_ADJUST_LEFT );
        rSet.InvalidateItem( SID_ATTR_PARA_ADJUST_CENTER );
        rSet.InvalidateItem( SID_ATTR_PARA_ADJUST_RIGHT );
        rSet.InvalidateItem( SID_ATTR_PARA_ADJUST_BLOCK );
    }
    else
    {
        SvxAdjust eAdjust = ((const SvxAdjustItem*)pItem)->GetAdjust();
        switch( eAdjust )
        {
            case SVX_ADJUST_LEFT:   nWhich = SID_ATTR_PARA_ADJUST_LEFT;     break;
            case SVX_ADJUST_CENTER: nWhich = SID_ATTR_PARA_ADJUST_CENTER;   break;
            case SVX_ADJUST_RIGHT:  nWhich = SID_ATTR_PARA_ADJUST_RIGHT;    break;
            case SVX_ADJUST_BLOCK:  nWhich = SID_ATTR_PARA_ADJUST_BLOCK;            break;
        }
        if( nWhich )
            rSet.Put( SfxBoolItem( nWhich, sal_True ) );
    }

    eState = aEditAttr.GetItemState(EE_PARA_SBL, sal_False, &pItem);
    if (eState == SFX_ITEM_DONTCARE || !pItem)
    {
        rSet.InvalidateItem( SID_ATTR_PARA_LINESPACE_10 );
        rSet.InvalidateItem( SID_ATTR_PARA_LINESPACE_15 );
        rSet.InvalidateItem( SID_ATTR_PARA_LINESPACE_20 );
    }
    else
    {
        const sal_Int8 nSpace = ((const SvxLineSpacingItem*)pItem)->GetPropLineSpace();
        nWhich = 0;
        switch( nSpace )
        {
            case 100:
                nWhich = SID_ATTR_PARA_LINESPACE_10;
                break;
            case 150:
                nWhich = SID_ATTR_PARA_LINESPACE_15;
                break;
            case 200:
                nWhich = SID_ATTR_PARA_LINESPACE_20;
                break;
        }
        if( nWhich )
            rSet.Put( SfxBoolItem( nWhich, sal_True ) );
    }

    SvxEscapement eEsc = (SvxEscapement)((const SvxEscapementItem&)
                    aEditAttr.Get(EE_CHAR_ESCAPEMENT)).GetEnumValue();

    if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
        rSet.Put( SfxBoolItem( FN_SET_SUPER_SCRIPT, sal_True ) );
    else if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
        rSet.Put( SfxBoolItem( FN_SET_SUB_SCRIPT, sal_True ) );


    // disable "Thesaurus" if the language is not supported
    const SfxPoolItem &rItem = GetShell().GetDoc()->GetDefault(RES_CHRATR_LANGUAGE);
    LanguageType nLang = ((const SvxLanguageItem &) rItem).GetLanguage();
    //
    uno::Reference< linguistic::XThesaurus >  xThes( ::GetThesaurus() );
    if (!xThes.is() || nLang == LANGUAGE_NONE ||
        !xThes->hasLocale( SvxCreateLocale( nLang ) ))
        rSet.DisableItem( FN_THESAURUS_DLG );
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

    while(nWhich)
    {
        switch(GetPool().GetSlotId(nWhich))
        {
            case SID_ATTR_CHAR_FONT:
                rSet.Put(aEditAttr.Get(EE_CHAR_FONTINFO, sal_True), nWhich);
                break;
            case SID_ATTR_CHAR_FONTHEIGHT:
                rSet.Put(aEditAttr.Get(EE_CHAR_FONTHEIGHT, sal_True), nWhich);
                break;
            case SID_ATTR_CHAR_COLOR:
                rSet.Put(aEditAttr.Get(EE_CHAR_COLOR, sal_True), nWhich);
                break;
            case SID_ATTR_CHAR_WEIGHT:
                rSet.Put(aEditAttr.Get(EE_CHAR_WEIGHT, sal_True), nWhich);
                break;
            case SID_ATTR_CHAR_POSTURE:
                rSet.Put(aEditAttr.Get(EE_CHAR_ITALIC, sal_True), nWhich);
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
    const sal_Bool bCopy = (aSel.nStartPara != aSel.nEndPara) || (aSel.nStartPos != aSel.nEndPos);


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
                    //!!OS: das muss ein if/else bleiben, weil BLC das
                    //      nicht anders versteht
                    SwModule* pMod = SW_MOD();
                    SvDataObjectRef xObj;
                    if( pMod->pClipboard )
                        xObj = pMod->pClipboard;
                    else
                        xObj = SvDataObject::PasteClipboard();

                    if( !xObj.Is() ||
                        !SwDataExchange::IsPaste( GetShell(), *xObj ) )
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

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.69  2000/09/18 16:06:03  willem.vandorp
      OpenOffice header added.

      Revision 1.68  2000/09/07 15:59:29  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.67  2000/05/26 07:21:32  os
      old SW Basic API Slots removed

      Revision 1.66  2000/04/18 14:58:23  os
      UNICODE

      Revision 1.65  2000/03/23 07:49:14  os
      UNO III

      Revision 1.64  2000/02/16 21:00:17  tl
      #72219# Locale Umstellung

      Revision 1.63  2000/02/11 14:57:32  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.62  2000/01/21 13:34:45  tl
      #70503# GetState method added for FN_THESAURUS_DLG

      Revision 1.61  1999/11/30 09:44:05  jp
      StateClpbrd: pObj contains sometimes no Pointer

      Revision 1.60  1999/09/24 14:38:32  os
      hlnkitem.hxx now in SVX

      Revision 1.59  1999/08/04 09:09:50  JP
      have to change: Outliner -> SdrOutlines


      Rev 1.58   04 Aug 1999 11:09:50   JP
   have to change: Outliner -> SdrOutlines

      Rev 1.57   18 Mar 1999 14:41:18   OS
   #61169# #61489# Masseinheiten fuer Text u. HTML am Module setzen, nicht an der App

      Rev 1.56   04 Nov 1998 17:29:54   OM
   #58904# Assertion umpropelt

      Rev 1.55   08 Sep 1998 17:03:08   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.54   25 Jun 1998 14:11:18   JP
   SvDataObject -> SotObject

      Rev 1.53   08 Apr 1998 13:47:22   OM
   #42505 Keine util::URL-Buttons in Html-Dokumenten

      Rev 1.52   12 Mar 1998 11:28:12   OM
   #48017# GPF entfernt

      Rev 1.51   05 Feb 1998 14:48:14   OS
   Absatzdialog enthaelt keinen TableMode

      Rev 1.50   29 Nov 1997 15:52:12   MA
   includes

      Rev 1.49   24 Nov 1997 09:47:00   MA
   includes

      Rev 1.48   03 Nov 1997 13:55:46   MA
   precomp entfernt

      Rev 1.47   17 Sep 1997 13:09:18   OM
   Editierbare Links in DrawText-Objekten

      Rev 1.46   01 Sep 1997 13:24:00   OS
   DLL-Umstellung

      Rev 1.45   11 Aug 1997 08:50:18   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.44   08 Aug 1997 17:28:46   OM
   Headerfile-Umstellung

      Rev 1.43   07 Jul 1997 12:28:14   OM
   #41258# Execute: Nur benoetigte Items putten

      Rev 1.42   17 Jun 1997 16:00:00   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.41   04 Jun 1997 11:24:06   TRI
   svwin.h nur unter WIN oder WNT includen

      Rev 1.40   04 Jun 1997 09:50:08   NF
   Includes...

      Rev 1.39   28 May 1997 15:06:30   OM
   #40067# DontCare-State fuer Absatzausrichtung beruecksichtigen

      Rev 1.38   16 May 1997 17:16:40   OM
   Links aus Hyperlinkleiste in DrawText-Objekte einfuegen

      Rev 1.37   19 Mar 1997 12:48:42   AMA
   Fix #37035: Speller am Outliner setzen.

      Rev 1.36   23 Feb 1997 18:20:08   AMA
   Fix #36843#: GPF in der EditEngine durch nicht gesetzten SpellChecker

      Rev 1.35   22 Jan 1997 18:42:26   MH
   add: include

      Rev 1.34   16 Jan 1997 16:36:52   OS
   Metric vor Dialogaufruf an der App setzen

      Rev 1.33   14 Jan 1997 09:31:34   TRI
   includes

      Rev 1.32   16 Dec 1996 19:10:40   HJS
   includes

      Rev 1.31   13 Dec 1996 16:38:22   OS
   Autospell fuer aktives DrawTextObject

      Rev 1.30   22 Nov 1996 14:53:32   OS
   FN_SET_JUSTIFY_PARA -> SID_ATTR_PARA_ADJUST_BLOCK

      Rev 1.29   28 Aug 1996 15:55:10   OS
   includes

      Rev 1.28   22 Mar 1996 15:14:32   TRI
   sfxiiter.hxx included

      Rev 1.27   05 Feb 1996 17:28:42   OM
   Parent-Win an Par/Chardlg uebergeben

      Rev 1.26   12 Dec 1995 17:43:58   OM
   Text-Attribute richtig setzen

      Rev 1.25   11 Dec 1995 18:21:26   OM
   Neuer DrawText Dialog

      Rev 1.24   08 Dec 1995 18:10:28   OM
   CharAttr-Stati richtig updaten

      Rev 1.23   07 Dec 1995 18:25:36   OM
   Neu: GetDrawTxtCtrlState

      Rev 1.22   24 Nov 1995 16:59:34   OM
   PCH->PRECOMPILED

      Rev 1.21   10 Nov 1995 18:24:24   OM
   Init-Fkt optimiert/repariert

      Rev 1.20   09 Nov 1995 18:00:12   OS
   Id fuer FormatPage/CharDlg auf SID_ umgestellt

      Rev 1.19   31 Oct 1995 18:40:42   OM
   GetFrameWindow entfernt

      Rev 1.18   17 Oct 1995 19:27:14   OM
   Clipboard fuer Drawtext

      Rev 1.17   14 Oct 1995 17:58:10   OM
   Prophylaktische Absturzvorsorge...

      Rev 1.16   14 Oct 1995 17:33:44   OM
   Bug 20805: DrawTextShell komplettiert

      Rev 1.15   05 Oct 1995 18:40:44   OM
   Aufgeraeumt

      Rev 1.14   04 Oct 1995 18:43:10   OM
   Text-Reset angefangen

      Rev 1.13   13 Sep 1995 17:28:06   OM
   Modified fuer Drawtext-Objekte geaendert

      Rev 1.12   28 Aug 1995 19:03:34   MA
   Renovierung: IDL, Shells, Textshell-Doktrin aufgegeben

      Rev 1.11   21 Aug 1995 21:00:00   MA
   chg: svxitems-header entfernt

      Rev 1.10   16 Aug 1995 17:50:12   MA
   Riesenheader dialogs entfernt.

      Rev 1.9   09 Aug 1995 18:39:10   OM
   Statusmethode abgesichert

      Rev 1.8   07 Aug 1995 18:41:54   OM
   Zeichendialog richtig attributiert

      Rev 1.7   06 Aug 1995 18:27:32   OM
   DrawTextShell-Popup

      Rev 1.6   23 Jul 1995 17:04:24   OS
   Zeilenabstand kann nur 100/150/200 sein

      Rev 1.5   14 Jul 1995 18:16:40   OM
   Farbcontroller rausgeschmissen

      Rev 1.4   04 May 1995 10:10:04   JP
   Items Optimierung

      Rev 1.3   03 May 1995 23:32:52   ER
   add: editdata.hxx

      Rev 1.2   28 Mar 1995 20:07:58   ER
   header-adder

      Rev 1.1   05 Mar 1995 20:44:44   OM
   Linienabstand, drawing::Alignment, ...

      Rev 1.0   05 Mar 1995 16:36:00   OM
   Textfarbe setzen

      Rev 1.6   04 Mar 1995 19:20:44   OM
   Aenderungen fuer 241

      Rev 1.5   04 Mar 1995 19:03:18   OM
   Execute und State fuer Texthintergrund

      Rev 1.4   03 Mar 1995 18:16:06   OM
   virtual GetPool implementiert

      Rev 1.3   03 Mar 1995 02:07:32   OM
   State-Methode fuer Font-Controller entfernt

      Rev 1.2   02 Mar 1995 19:19:26   OM
   Font-Controller eingebunden

      Rev 1.1   01 Mar 1995 19:07:18   OM
   DrawTextShell 2.Runde

      Rev 1.0   28 Feb 1995 19:45:08   OM
   Initial revision.

*************************************************************************/




