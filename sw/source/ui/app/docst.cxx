/*************************************************************************
 *
 *  $RCSfile: docst.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:42:33 $
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

#include <hintids.hxx>

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif
#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _NEWSTYLE_HXX //autogen
#include <sfx2/newstyle.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

#include "view.hxx"
#include "wrtsh.hxx"
#include "docsh.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "viewopt.hxx"
#include "doc.hxx"
#include "swstyle.h"
#include "frmfmt.hxx"
#include "charfmt.hxx"
#include "poolfmt.hxx"
#include "pagedesc.hxx"
#include "tmpdlg.hxx"
#include "docstyle.hxx"
#include "uiitems.hxx"
#include "fmtcol.hxx"
#include "frmmgr.hxx"       //SwFrmValid
#include "swevent.hxx"
#include "edtwin.hxx"

#include "app.hrc"


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void  SwDocShell::StateStyleSheet(SfxItemSet& rSet, SwWrtShell* pSh)
{
    SfxWhichIter aIter(rSet);
    USHORT  nWhich  = aIter.FirstWhich();
    USHORT nActualFamily = USHRT_MAX;

    SwWrtShell* pShell = pSh ? pSh : GetWrtShell();
    if(!pShell)
    {
        while (nWhich)
        {
            rSet.DisableItem(nWhich);
            nWhich = aIter.NextWhich();
        }
        return;
    }
    else
    {
        SfxViewFrame* pFrame = pShell->GetView().GetViewFrame();
        const ISfxTemplateCommon* pCommon = SFX_APP()->GetCurrentTemplateCommon(pFrame->GetBindings());
        if( pCommon )
            nActualFamily = pCommon->GetActualFamily();
    }

    while (nWhich)
    {
        // aktuelle Vorlage zu jeder Familie ermitteln
        //
        String aName;
        switch (nWhich)
        {
            case SID_STYLE_APPLY:
            {//hier wird die Vorlage und ihre Familie an die StyleBox
             //uebergeben, damit diese Familie angezeigt wird
                if(pShell->IsFrmSelected())
                {
                    SwFrmFmt* pFmt = pShell->GetCurFrmFmt();
                    if( pFmt )
                        aName = pFmt->GetName();
                }
                else
                {
                    SwTxtFmtColl* pColl = pShell->GetCurTxtFmtColl();
                    if(pColl)
                        aName = pColl->GetName();
                }
                rSet.Put(SfxTemplateItem(nWhich, aName));
            }
            break;
            case SID_STYLE_FAMILY1:
                if( !pShell->IsFrmSelected() )
                {
                    SwCharFmt* pFmt = pShell->GetCurCharFmt();
                    if(pFmt)
                        aName = pFmt->GetName();
                    else
                        aName = *SwStyleNameMapper::GetTextUINameArray()[
                            RES_POOLCOLL_STANDARD - RES_POOLCOLL_TEXT_BEGIN ];
                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
                break;

            case SID_STYLE_FAMILY2:
                if(!pShell->IsFrmSelected())
                {
                    SwTxtFmtColl* pColl = pShell->GetCurTxtFmtColl();
                    if(pColl)
                        aName = pColl->GetName();

                    SfxTemplateItem aItem(nWhich, aName);

                    USHORT nMask = 0;
                    if( pDoc->IsHTMLMode() )
                        nMask = SWSTYLEBIT_HTML;
                    else
                    {
                        const int nSelection = pShell->GetFrmType(0,TRUE);
                        if(pShell->GetCurTOX())
                            nMask = SWSTYLEBIT_IDX  ;
                        else if(nSelection & FRMTYPE_HEADER     ||
                                nSelection & FRMTYPE_FOOTER     ||
                                nSelection & FRMTYPE_TABLE      ||
                                nSelection & FRMTYPE_FLY_ANY    ||
                                nSelection & FRMTYPE_FOOTNOTE   ||
                                nSelection & FRMTYPE_FTNPAGE)
                            nMask = SWSTYLEBIT_EXTRA;
                        else
                            nMask = SWSTYLEBIT_TEXT;
                    }

                    aItem.SetValue(nMask);
                    rSet.Put(aItem);
                }

                break;

            case SID_STYLE_FAMILY3:

                if( pDoc->IsHTMLMode() )
                    rSet.DisableItem( nWhich );
                else
                {
                    SwFrmFmt* pFmt = pShell->GetCurFrmFmt();
                    if(pFmt && pShell->IsFrmSelected())
                    {
                        aName = pFmt->GetName();
                        rSet.Put(SfxTemplateItem(nWhich, aName));
                    }
                }
                break;

            case SID_STYLE_FAMILY4:
            {
                OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
                if( pDoc->IsHTMLMode() && !pHtmlOpt->IsPrintLayoutExtension())
                    rSet.DisableItem( nWhich );
                else
                {
                    USHORT n = pShell->GetCurPageDesc( FALSE );
                    if( n < pShell->GetPageDescCnt() )
                        aName = pShell->GetPageDesc( n ).GetName();

                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
            }
            break;
            case SID_STYLE_FAMILY5:
                {
                    const SwNumRule* pRule = pShell->GetCurNumRule();
                    if( pRule )
                        aName = pRule->GetName();

                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
                break;

            case SID_STYLE_WATERCAN:
            {
                SwEditWin& rEdtWin = pShell->GetView().GetEditWin();
                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                rSet.Put(SfxBoolItem(nWhich, pApply && pApply->eType != 0));
            }
            break;
            case SID_STYLE_UPDATE_BY_EXAMPLE:
                if( pShell->IsFrmSelected()
                        ? SFX_STYLE_FAMILY_FRAME != nActualFamily
                        : ( SFX_STYLE_FAMILY_FRAME == nActualFamily ||
                            SFX_STYLE_FAMILY_PAGE == nActualFamily ||
                            (SFX_STYLE_FAMILY_PSEUDO == nActualFamily && !pShell->GetCurNumRule())) )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_STYLE_NEW_BY_EXAMPLE:
                if( (pShell->IsFrmSelected()
                        ? SFX_STYLE_FAMILY_FRAME != nActualFamily
                        : SFX_STYLE_FAMILY_FRAME == nActualFamily) ||
                    (SFX_STYLE_FAMILY_PSEUDO == nActualFamily && !pShell->GetCurNumRule()) )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            default:
                DBG_ERROR( "Invalid SlotId");
        }
        nWhich = aIter.NextWhich();
    }
}


/*--------------------------------------------------------------------
    Beschreibung:   StyleSheet-Requeste auswerten
 --------------------------------------------------------------------*/


void SwDocShell::ExecStyleSheet( SfxRequest& rReq )
{
    USHORT  nSlot   = rReq.GetSlot();
    USHORT  nRet    = 0xffff;

    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    SwWrtShell* pActShell = 0;
    BOOL bSetReturn = TRUE;
    switch (nSlot)
    {
    case SID_STYLE_NEW:
        if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILY,
            FALSE, &pItem ))
        {
            USHORT nFamily = ((const SfxUInt16Item*)pItem)->GetValue();

            String sName;
            USHORT nMask = 0;
            if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_NEW,
                FALSE, &pItem ))
                sName = ((const SfxStringItem*)pItem)->GetValue();
            if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_MASK,
                FALSE, &pItem ))
                nMask = ((const SfxUInt16Item*)pItem)->GetValue();
            String sParent;
            if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_REFERENCE,
                FALSE, &pItem ))
                sParent = ((const SfxStringItem*)pItem)->GetValue();

            nRet = Edit( sName, sParent, nFamily, nMask, TRUE, FALSE, 0, rReq.IsAPI() );
        }
        break;

        case SID_STYLE_APPLY:
            if( !pArgs )
            {
                GetView()->GetViewFrame()->GetDispatcher()->Execute(SID_STYLE_DESIGNER, FALSE);
                break;
            }   // Fall through
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_WATERCAN:
        case SID_STYLE_FAMILY:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            String aParam;
            USHORT nFamily, nMask = 0;

            if( !pArgs )
            {
                nFamily = SFX_STYLE_FAMILY_PARA;

                switch (nSlot)
                {
                    case SID_STYLE_NEW_BY_EXAMPLE:
                    {
                        SfxNewStyleDlg *pDlg = new SfxNewStyleDlg( 0,
                                                    *GetStyleSheetPool());
                        if(RET_OK == pDlg->Execute())
                        {
                            aParam = pDlg->GetName();
                            rReq.AppendItem(SfxStringItem(nSlot, aParam));
                        }

                        delete pDlg;
                    }
                    break;

                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                    case SID_STYLE_EDIT:
                    {
                        SwTxtFmtColl* pColl = GetWrtShell()->GetCurTxtFmtColl();
                        if(pColl)
                        {
                            aParam = pColl->GetName();
                            rReq.AppendItem(SfxStringItem(nSlot, aParam));
                        }
                    }
                    break;
                }
            }
            else
            {
                ASSERT( pArgs->Count(), "SfxBug ItemSet ist leer");

                SwWrtShell* pShell = GetWrtShell();
                if( SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem ))
                    aParam = ((const SfxStringItem*)pItem)->GetValue();

                if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_FAMILY,
                    FALSE, &pItem ))
                    nFamily = ((const SfxUInt16Item*)pItem)->GetValue();

                if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_MASK,
                    FALSE, &pItem ))
                    nMask = ((const SfxUInt16Item*)pItem)->GetValue();
                if( SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_WRTSHELL,
                    FALSE, &pItem ))
                    pActShell = pShell = (SwWrtShell*)((SwPtrItem*)pItem)->GetValue();

                if( nSlot == SID_STYLE_UPDATE_BY_EXAMPLE )
                {
                    switch( nFamily )
                    {
                        case SFX_STYLE_FAMILY_PARA:
                        {
                            SwTxtFmtColl* pColl = pShell->GetCurTxtFmtColl();
                            if(pColl)
                                aParam = pColl->GetName();
                        }
                        break;
                        case SFX_STYLE_FAMILY_FRAME:
                        {
                            SwFrmFmt* pFrm = pWrtShell->GetCurFrmFmt();
                            if( pFrm )
                                aParam = pFrm->GetName();
                        }
                        break;
                        case SFX_STYLE_FAMILY_CHAR:
                        {
                            SwCharFmt* pChar = pWrtShell->GetCurCharFmt();
                            if( pChar )
                                aParam = pChar->GetName();
                        }
                        break;
                        case SFX_STYLE_FAMILY_PSEUDO:
                        if(SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_UPD_BY_EX_NAME, FALSE, &pItem))
                        {
                            aParam = ((const SfxStringItem*)pItem)->GetValue();
                        }
                        break;
                    }
                    rReq.AppendItem(SfxStringItem(nSlot, aParam));
                }
            }
            if (aParam.Len() || nSlot == SID_STYLE_WATERCAN )
            {
                switch(nSlot)
                {
                    case SID_STYLE_EDIT:
                        nRet = Edit(aParam, aEmptyStr, nFamily, nMask, FALSE, FALSE, pActShell );
                        break;
                    case SID_STYLE_DELETE:
                        nRet = Delete(aParam, nFamily);
                        break;
                    case SID_STYLE_APPLY:
                        // Shellwechsel in ApplyStyles
                        nRet = ApplyStyles(aParam, nFamily, pActShell, rReq.GetModifier() );
                        break;
                    case SID_STYLE_WATERCAN:
                        nRet = DoWaterCan(aParam, nFamily);
                        break;
                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                        nRet = UpdateStyle(aParam, nFamily, pActShell);
                        break;
                    case SID_STYLE_NEW_BY_EXAMPLE:
                    {
                        nRet = MakeByExample(aParam, nFamily, nMask, pActShell );
                        SfxTemplateDialog* pDlg = SFX_APP()->GetTemplateDialog();

                        if(pDlg && pDlg->IsVisible())
                            pDlg->Update();
                    }
                    break;

                    default:
                        DBG_ERROR( "Falsche Slot-Id");
                }

                rReq.Done();
            }

            break;
        }
    }

    if(bSetReturn)
    {
        if(rReq.IsAPI()) // Basic bekommt nur TRUE oder FALSE
            rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet !=0));
        else
            rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet));
    }

}

/*--------------------------------------------------------------------
    Beschreibung:   Edit
 --------------------------------------------------------------------*/


USHORT SwDocShell::Edit( const String &rName, const String &rParent, USHORT nFamily, USHORT nMask,
                         BOOL bNew, BOOL bColumn, SwWrtShell* pActShell,
                         BOOL bBasic )
{
    ASSERT(GetWrtShell(), "Keine Shell, keine Styles");
    SfxStyleSheetBase *pStyle = 0;

    USHORT nRet = nMask;
    BOOL bModified = pDoc->IsModified();

    if( bNew )
    {
        if( SFXSTYLEBIT_ALL != nMask && SFXSTYLEBIT_USED != nMask )
            nMask |= SFXSTYLEBIT_USERDEF;
        else
            nMask = SFXSTYLEBIT_USERDEF;

        pStyle = &pBasePool->Make( rName, (SfxStyleFamily)nFamily, nMask );

        // die aktuellen als Parent setzen
        SwDocStyleSheet* pDStyle = (SwDocStyleSheet*)pStyle;
        switch( nFamily )
        {
            case SFX_STYLE_FAMILY_PARA:
            {
                if(rParent.Len())
                {
                    SwTxtFmtColl* pColl = pWrtShell->FindTxtFmtCollByName( rParent );
                    if(!pColl)
                    {
                        USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, GET_POOLID_TXTCOLL);
                        if(USHRT_MAX != nId)
                            pColl =  pWrtShell->GetTxtCollFromPool( nId );
                    }
                    pDStyle->GetCollection()->SetDerivedFrom( pColl );
                    pDStyle->PresetParent( rParent );
                }
                else
                {
                    SwTxtFmtColl* pColl = pWrtShell->GetCurTxtFmtColl();
                    pDStyle->GetCollection()->SetDerivedFrom( pColl );
                    if( pColl )
                        pDStyle->PresetParent( pColl->GetName() );
                }
            }
            break;
            case SFX_STYLE_FAMILY_CHAR:
            {
                if(rParent.Len())
                {
                    SwCharFmt* pCFmt = pWrtShell->FindCharFmtByName( rParent );
                    if(!pCFmt)
                    {
                        USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, GET_POOLID_CHRFMT);
                        if(USHRT_MAX != nId)
                            pCFmt =  pWrtShell->GetCharFmtFromPool( nId );
                    }

                    pDStyle->GetCharFmt()->SetDerivedFrom( pCFmt );
                    pDStyle->PresetParent( rParent );
                }
                else
                {
                    SwCharFmt* pCFmt = pWrtShell->GetCurCharFmt();
                    pDStyle->GetCharFmt()->SetDerivedFrom( pCFmt );
                        if( pCFmt )
                            pDStyle->PresetParent( pCFmt->GetName() );
                }
            }
            break;
            case SFX_STYLE_FAMILY_FRAME :
            {
                if(rParent.Len())
                {
                    SwFrmFmt* pFFmt = pWrtShell->GetDoc()->FindFrmFmtByName( rParent );
                    if(!pFFmt)
                    {
                        USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, GET_POOLID_FRMFMT);
                        if(USHRT_MAX != nId)
                            pFFmt =  pWrtShell->GetFrmFmtFromPool( nId );
                    }
                    pDStyle->GetFrmFmt()->SetDerivedFrom( pFFmt );
                    pDStyle->PresetParent( rParent );
                }
            }
            break;
        }
    }
    else
    {
        pStyle = pBasePool->Find( rName, (SfxStyleFamily)nFamily );
        ASSERT(pStyle, "Vorlage nicht gefunden");
    }

    if(!pStyle)
        return FALSE;

    // Dialoge zusammenstoepseln
    //
    SwDocStyleSheet aTmp( *(SwDocStyleSheet*)pStyle );
    if( SFX_STYLE_FAMILY_PARA == nFamily )
    {
        SfxItemSet& rSet = aTmp.GetItemSet();
        ::SwToSfxPageDescAttr( rSet );
        // erstmal nur eine Null
        rSet.Put(SwBackgroundDestinationItem(SID_PARA_BACKGRND_DESTINATION, 0));
    }
/*  else if( SFX_STYLE_FAMILY_FRAME == nFamily )
    {
        // Auskommentiert wegen Bug #45776 (per default keine Breite&Groesse in Rahmenvorlagen)
        SfxItemSet& rSet = aTmp.GetItemSet();
        if( SFX_ITEM_SET != rSet.GetItemState( RES_FRM_SIZE ))
        {
            // dann sollten wir spaetesten hier eines anlegen
            SwFrmValid aFrmDefValues;
            rSet.Put( SwFmtFrmSize( ATT_VAR_SIZE, aFrmDefValues.nWidth,
                                    aFrmDefValues.nHeight ));
        }
    }*/
    else if( SFX_STYLE_FAMILY_CHAR == nFamily )
    {
        SfxItemSet& rSet = aTmp.GetItemSet();
        const SfxPoolItem *pTmpBrush;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BACKGROUND,
            TRUE, &pTmpBrush ) )
        {
            SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
            aTmpBrush.SetWhich( RES_BACKGROUND );
            rSet.Put( aTmpBrush );
        }
    }
    if (!bBasic)
    {
        // vor dem Dialog wird der HtmlMode an der DocShell versenkt
        USHORT nHtmlMode = ::GetHtmlMode(this);
        PutItem(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));
        FieldUnit eMetric = ::GetDfltMetric(0 != (HTMLMODE_ON&nHtmlMode));
        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
        SwTemplateDlg* pDlg = new SwTemplateDlg( 0, aTmp, nFamily, bColumn,
                            pActShell ? pActShell : pWrtShell, bNew);
        if(RET_OK == pDlg->Execute())
        {
            GetWrtShell()->StartAllAction();

            // nur bei Absatz-Vorlagen die Maske neu setzen
            if( bNew )
            {
                nRet = SFX_STYLE_FAMILY_PARA == pStyle->GetFamily()
                        ? aTmp.GetMask()
                        : SFXSTYLEBIT_USERDEF;
            }
            else if( pStyle->GetMask() != aTmp.GetMask() )
                nRet = aTmp.GetMask();

            if( SFX_STYLE_FAMILY_PARA == nFamily )
            {
                SfxItemSet aSet( *pDlg->GetOutputItemSet() );
                ::SfxToSwPageDescAttr( *GetWrtShell(), aSet  );
                aTmp.SetItemSet( aSet );
            }
            else
            {
                if(SFX_STYLE_FAMILY_PAGE == nFamily)
                {
                    static const USHORT aInval[] = {
                        SID_IMAGE_ORIENTATION,
                        SID_ATTR_CHAR_FONT,
                        FN_INSERT_CTRL, FN_INSERT_OBJ_CTRL, 0};
                    pView->GetViewFrame()->GetBindings().Invalidate(aInval);
                }
                SfxItemSet aTmpSet( *pDlg->GetOutputItemSet() );
                if( SFX_STYLE_FAMILY_CHAR == nFamily )
                {
                    const SfxPoolItem *pTmpBrush;
                    if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND,
                        FALSE, &pTmpBrush ) )
                    {
                        SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                        aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                        aTmpSet.Put( aTmpBrush );
                    }
                    aTmpSet.ClearItem( RES_BACKGROUND );
                }
                aTmp.SetItemSet( aTmpSet );
            }
            if(SFX_STYLE_FAMILY_PAGE == nFamily)
                pView->InvalidateRulerPos();

            if( bNew )
                pBasePool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, aTmp ) );

            // JP 19.09.97:
            // Dialog vorm EndAction zerstoeren - bei Seitenvorlagen kann
            // muss der ItemSet zerstoert werden, damit die Cursor aus den
            // Kopf-/Fusszeilen entfernt werden. Sonst kommts zu GPFs!!!
            delete pDlg;

            pDoc->SetModified();
            if( !bModified )    // Bug 57028
                pDoc->SetUndoNoResetModified();

            GetWrtShell()->EndAllAction();
        }
        else
        {
            if( bNew )
                pBasePool->Erase( &aTmp );
            if( !bModified )
                pDoc->ResetModified();
            delete pDlg;
        }
    }
    else
    {
        // vor dem Dialog wird der HtmlMode an der DocShell versenkt
        PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));

        GetWrtShell()->StartAllAction();

        // nur bei Absatz-Vorlagen die Maske neu setzen
        if( bNew )
        {
            nRet = SFX_STYLE_FAMILY_PARA == pStyle->GetFamily()
                    ? aTmp.GetMask()
                    : SFXSTYLEBIT_USERDEF;
        }
        else if( pStyle->GetMask() != aTmp.GetMask() )
            nRet = aTmp.GetMask();

        if( SFX_STYLE_FAMILY_PARA == nFamily )
            ::SfxToSwPageDescAttr( *GetWrtShell(), aTmp.GetItemSet() );
        else
        {
            SfxItemSet aTmpSet( aTmp.GetItemSet() );
            if( SFX_STYLE_FAMILY_CHAR == nFamily )
            {
                const SfxPoolItem *pTmpBrush;
                if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND,
                    FALSE, &pTmpBrush ) )
                {
                    SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                    aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                    aTmpSet.Put( aTmpBrush );
                }
                aTmpSet.ClearItem( RES_BACKGROUND );
            }
            aTmp.SetItemSet( aTmpSet );
        }
        if(SFX_STYLE_FAMILY_PAGE == nFamily)
            pView->InvalidateRulerPos();

        if( bNew )
            pBasePool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, aTmp ) );

        pDoc->SetModified();
        if( !bModified )        // Bug 57028
            pDoc->SetUndoNoResetModified();
        GetWrtShell()->EndAllAction();
    }

    return nRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Delete
 --------------------------------------------------------------------*/


USHORT SwDocShell::Delete(const String &rName, USHORT nFamily)
{
    SfxStyleSheetBase *pStyle = pBasePool->Find(rName, (SfxStyleFamily)nFamily);

    if(pStyle)
    {
        ASSERT(GetWrtShell(), "Keine Shell, keine Styles");

        GetWrtShell()->StartAllAction();
        pBasePool->Erase(pStyle);
        GetWrtShell()->EndAllAction();

        return TRUE;
    }
    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Vorlage anwenden
 --------------------------------------------------------------------*/


USHORT SwDocShell::ApplyStyles(const String &rName, USHORT nFamily,
                               SwWrtShell* pShell, USHORT nMode )
{
    SwDocStyleSheet* pStyle =
        (SwDocStyleSheet*)pBasePool->Find(rName, (SfxStyleFamily)nFamily);

    ASSERT(pStyle, "Wo ist der StyleSheet");
    if(!pStyle)
        return FALSE;

    SwWrtShell *pSh = pShell ? pShell : GetWrtShell();

    ASSERT( pSh, "Keine Shell, keine Styles");

    pSh->StartAllAction();

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwFmtCharFmt aFmt(pStyle->GetCharFmt());
            pSh->SetAttr( aFmt, (nMode & KEY_SHIFT) ? SETATTR_DONTREPLACE : SETATTR_DEFAULT );
            break;
        }
        case SFX_STYLE_FAMILY_PARA:
        {
            pSh->SetTxtFmtColl(pStyle->GetCollection());
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            if ( pSh->IsFrmSelected() )
                pSh->SetFrmFmt( pStyle->GetFrmFmt() );
            break;
        }
        case SFX_STYLE_FAMILY_PAGE:
        {
            pSh->SetPageStyle(pStyle->GetPageDesc()->GetName());
            break;
        }
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            pSh->SetCurNumRule( *pStyle->GetNumRule() );
            break;
        }
        default:
            DBG_ERROR("Unbekannte Familie");
    }
    pSh->EndAllAction();

    return nFamily;
}

/*--------------------------------------------------------------------
    Beschreibung:   Giesskanne starten
 --------------------------------------------------------------------*/



USHORT SwDocShell::DoWaterCan(const String &rName, USHORT nFamily)
{
    ASSERT(GetWrtShell(), "Keine Shell, keine Styles");

    SwEditWin& rEdtWin = pView->GetEditWin();
    SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
    BOOL bWaterCan = !(pApply && pApply->eType != 0);
    if( !rName.Len() )
        bWaterCan = FALSE;
    SwApplyTemplate aTemplate;
    aTemplate.eType = nFamily;

    if(bWaterCan)
    {
        SwDocStyleSheet* pStyle =
            (SwDocStyleSheet*)pBasePool->Find(rName, (SfxStyleFamily)nFamily);

        ASSERT(pStyle, "Wo ist der StyleSheet");
        if(!pStyle) return nFamily;

        switch(nFamily)
        {
            case SFX_STYLE_FAMILY_CHAR:
                aTemplate.aColl.pCharFmt = pStyle->GetCharFmt();
                break;
            case SFX_STYLE_FAMILY_PARA:
                aTemplate.aColl.pTxtColl = pStyle->GetCollection();
                break;
            case SFX_STYLE_FAMILY_FRAME:
                aTemplate.aColl.pFrmFmt = pStyle->GetFrmFmt();
                break;
            case SFX_STYLE_FAMILY_PAGE:
                aTemplate.aColl.pPageDesc = (SwPageDesc*)pStyle->GetPageDesc();
                break;
            case SFX_STYLE_FAMILY_PSEUDO:
                aTemplate.aColl.pNumRule = (SwNumRule*)pStyle->GetNumRule();
                break;

            default:
                DBG_ERROR( "Unbekannte Familie");
        }
    }
    else
        aTemplate.eType = 0;

    // Template anwenden
    pView->GetEditWin().SetApplyTemplate(aTemplate);

    return nFamily;
}

/*--------------------------------------------------------------------
    Beschreibung:   Vorlage Updaten
 --------------------------------------------------------------------*/


USHORT SwDocShell::UpdateStyle(const String &rName, USHORT nFamily, SwWrtShell* pShell)
{
    SwWrtShell* pWrtShell = pShell ? pShell : GetWrtShell();
    ASSERT(GetWrtShell(), "Keine Shell, keine Styles");

    SwDocStyleSheet* pStyle =
        (SwDocStyleSheet*)pBasePool->Find(rName, (SfxStyleFamily)nFamily);

    if(!pStyle)
        return nFamily;

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA:
        {
            SwTxtFmtColl* pColl = pStyle->GetCollection();
            if(pColl && !pColl->IsDefault())
            {
                GetWrtShell()->StartAllAction();
                GetWrtShell()->FillByEx(pColl);
                    // Vorlage auch anwenden, um harte Attributierung
                    // zu entfernen
                GetWrtShell()->SetTxtFmtColl( pColl );
                GetWrtShell()->EndAllAction();
            }
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrmFmt* pFrm = pStyle->GetFrmFmt();
            if( pWrtShell->IsFrmSelected() && pFrm && !pFrm->IsDefault() )
            {
                SfxItemSet aSet( GetPool(), aFrmFmtSetRange );
                pWrtShell->StartAllAction();
                pWrtShell->GetFlyFrmAttr( aSet );

                // JP 10.06.98: nur automatische Orientierungen uebernehmen
/*              #61359# jetzt auch wieder alle Orientierungen
 *              const SfxPoolItem* pItem;
                if( SFX_ITEM_SET == aSet.GetItemState( RES_VERT_ORIENT,
                    FALSE, &pItem ) &&
                    VERT_NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient())
                    aSet.ClearItem( RES_VERT_ORIENT );

                if( SFX_ITEM_SET == aSet.GetItemState( RES_HORI_ORIENT,
                    FALSE, &pItem ) &&
                    HORI_NONE == ((SwFmtHoriOrient*)pItem)->GetHoriOrient())
                    aSet.ClearItem( RES_HORI_ORIENT );*/

                pFrm->SetAttr( aSet );

                    // Vorlage auch anwenden, um harte Attributierung
                    // zu entfernen
                pWrtShell->SetFrmFmt( pFrm, TRUE );
                pWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFmt* pChar = pStyle->GetCharFmt();
            if( pChar && !pChar->IsDefault() )
            {
                pWrtShell->StartAllAction();
                pWrtShell->FillByEx(pChar);
                    // Vorlage auch anwenden, um harte Attributierung
                    // zu entfernen
                pWrtShell->EndAllAction();
            }

        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            const SwNumRule* pCurRule;
            if( pStyle->GetNumRule() &&
                0 != ( pCurRule = pWrtShell->GetCurNumRule() ))
            {
                SwNumRule aRule( *pCurRule );
                aRule.SetName( pStyle->GetNumRule()->GetName() );
                pWrtShell->ChgNumRuleFmts( aRule );
            }
        }
        break;
    }
    return nFamily;
}

/*--------------------------------------------------------------------
    Beschreibung:   NewByExample
 --------------------------------------------------------------------*/


USHORT SwDocShell::MakeByExample( const String &rName, USHORT nFamily,
                                    USHORT nMask, SwWrtShell* pShell )
{
    SwWrtShell* pWrtShell = pShell ? pShell : GetWrtShell();
    SwDocStyleSheet* pStyle = (SwDocStyleSheet*)pBasePool->Find(
                                            rName, (SfxStyleFamily)nFamily );
    if(!pStyle)
    {
        // JP 07.07.95: behalte die akt. Maske vom PI bei, dadurch werden
        //              neue sofort in den sichtbaren Bereich einsortiert
        if( SFXSTYLEBIT_ALL == nMask || SFXSTYLEBIT_USED == nMask )
            nMask = SFXSTYLEBIT_USERDEF;
        else
            nMask |= SFXSTYLEBIT_USERDEF;

        pStyle = (SwDocStyleSheet*)&pBasePool->Make(rName,
                                (SfxStyleFamily)nFamily, nMask );
    }

    switch(nFamily)
    {
        case  SFX_STYLE_FAMILY_PARA:
        {
            SwTxtFmtColl* pColl = pStyle->GetCollection();
            if(pColl && !pColl->IsDefault())
            {
                pWrtShell->StartAllAction();
                pWrtShell->FillByEx(pColl);
                    // Vorlage auch anwenden, um harte Attributierung
                    // zu entfernen
                pColl->SetDerivedFrom(pWrtShell->GetCurTxtFmtColl());

                    // setze die Maske noch an der Collection:
                USHORT nId = pColl->GetPoolFmtId() & 0x87ff;
                switch( nMask & 0x0fff )
                {
                case SWSTYLEBIT_TEXT:
                    nId |= COLL_TEXT_BITS;
                    break;
                case SWSTYLEBIT_CHAPTER:
                    nId |= COLL_DOC_BITS;
                    break;
                case SWSTYLEBIT_LIST:
                    nId |= COLL_LISTS_BITS;
                    break;
                case SWSTYLEBIT_IDX:
                    nId |= COLL_REGISTER_BITS;
                    break;
                case SWSTYLEBIT_EXTRA:
                    nId |= COLL_EXTRA_BITS;
                    break;
                case SWSTYLEBIT_HTML:
                    nId |= COLL_HTML_BITS;
                    break;
                }
                pColl->SetPoolFmtId(nId);

                pWrtShell->SetTxtFmtColl(pColl);
                pWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrmFmt* pFrm = pStyle->GetFrmFmt();
            if(pWrtShell->IsFrmSelected() && pFrm && !pFrm->IsDefault())
            {
                pWrtShell->StartAllAction();

                SfxItemSet aSet(GetPool(), aFrmFmtSetRange );
                pWrtShell->GetFlyFrmAttr( aSet );

                // JP 10.06.98: nur automatische Orientierungen uebernehmen
/*              #61359# jetzt auch wieder alle Orientierungen
                const SfxPoolItem* pItem;
                if( SFX_ITEM_SET == aSet.GetItemState( RES_VERT_ORIENT,
                    FALSE, &pItem ) &&
                    VERT_NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient())
                    aSet.ClearItem( RES_VERT_ORIENT );

                if( SFX_ITEM_SET == aSet.GetItemState( RES_HORI_ORIENT,
                    FALSE, &pItem ) &&
                    HORI_NONE == ((SwFmtHoriOrient*)pItem)->GetHoriOrient())
                    aSet.ClearItem( RES_HORI_ORIENT );
 */

                pFrm->SetAttr( aSet );
                    // Vorlage auch anwenden, um harte Attributierung
                    // zu entfernen
                pWrtShell->SetFrmFmt( pFrm );
                pWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFmt* pChar = pStyle->GetCharFmt();
            if(pChar && !pChar->IsDefault())
            {
                pWrtShell->StartAllAction();
                pWrtShell->FillByEx( pChar );
                pChar->SetDerivedFrom( pWrtShell->GetCurCharFmt() );
                SwFmtCharFmt aFmt( pChar );
                pWrtShell->SetAttr( aFmt );
                pWrtShell->EndAllAction();
            }
        }
        break;

        case SFX_STYLE_FAMILY_PAGE:
        {
            pWrtShell->StartAllAction();
            USHORT nPgDsc = pWrtShell->GetCurPageDesc();
            SwPageDesc& rSrc = (SwPageDesc&)pWrtShell->GetPageDesc( nPgDsc );
            SwPageDesc& rDest = *(SwPageDesc*)pStyle->GetPageDesc();

            USHORT nPoolId = rDest.GetPoolFmtId();
            USHORT nHId = rDest.GetPoolHelpId();
            BYTE nHFId = rDest.GetPoolHlpFileId();

            pWrtShell->GetDoc()->CopyPageDesc( rSrc, rDest );

            // PoolId darf NIE kopiert werden!
            rDest.SetPoolFmtId( nPoolId );
            rDest.SetPoolHelpId( nHId );
            rDest.SetPoolHlpFileId( nHFId );

            // werden Kopf-/Fusszeilen angelegt, so gibt es kein Undo mehr!
            pWrtShell->GetDoc()->DelAllUndoObj();

            pWrtShell->EndAllAction();
        }
        break;

        case SFX_STYLE_FAMILY_PSEUDO:
        {
            pWrtShell->StartAllAction();

            SwNumRule aRule( *pWrtShell->GetCurNumRule() );
            String sOrigRule( aRule.GetName() );
            aRule.SetName( pStyle->GetNumRule()->GetName() );
            pWrtShell->ChgNumRuleFmts( aRule );

            pWrtShell->ReplaceNumRule( sOrigRule, aRule.GetName() );


            pWrtShell->EndAllAction();
        }
        break;
    }
    return nFamily;
}



void  SwDocShell::LoadStyles( SfxObjectShell& rSource )
{
/*  [Beschreibung]

    Diese Methode wird vom SFx gerufen, wenn aus einer Dokument-Vorlage
    Styles nachgeladen werden sollen. Bestehende Styles soll dabei
    "uberschrieben werden. Das Dokument mu"s daher neu formatiert werden.
    Daher werden die Applikationen in der Regel diese Methode "uberladen
    und in ihrer Implementierung die Implementierung der Basisklasse
    rufen.
*/
    // ist die Source unser Document, dann uebernehmen wir das
    // abpruefen selbst (wesentlich schneller und laeuft nicht ueber
    // die Kruecke SfxStylePool
    if( rSource.ISA( SwDocShell ))
    {
        //JP 28.05.99: damit die Kopf-/Fusszeilen nicht den fixen Inhalt
        //              der Vorlage erhalten, einmal alle FixFelder der
        //              Source aktualisieren
        ((SwDocShell&)rSource).pDoc->SetFixFields();
        if( pWrtShell )
        {
            pWrtShell->StartAllAction();
            pDoc->ReplaceStyles( *((SwDocShell&)rSource).pDoc );
            pWrtShell->EndAllAction();
        }
        else
        {
            BOOL bModified = pDoc->IsModified();
            pDoc->ReplaceStyles( *((SwDocShell&)rSource).pDoc );
            if( !bModified && pDoc->IsModified() && !pView )
            {
                // die View wird spaeter angelegt, ueberschreibt aber das
                // Modify-Flag. Per Undo ist sowieso nichts mehr zu machen
                pDoc->SetUndoNoResetModified();
            }
        }
    }
    else
        SfxObjectShell::LoadStyles( rSource );
}


void SwDocShell::FormatPage( const String& rPage, BOOL bColumn, SwWrtShell*     pActShell )
{
    Edit( rPage, aEmptyStr, SFX_STYLE_FAMILY_PAGE, 0, FALSE, bColumn, pActShell);
}

Bitmap SwDocShell::GetStyleFamilyBitmap( SfxStyleFamily eFamily, BmpColorMode eColorMode )
{
    if( SFX_STYLE_FAMILY_PSEUDO == eFamily )
    {
        if ( eColorMode == BMP_COLOR_NORMAL )
            return Bitmap( SW_RES( BMP_STYLES_FAMILY_NUM ));
        else
            return Bitmap( SW_RES( BMP_STYLES_FAMILY_NUM_HC ));
    }

    return SfxObjectShell::GetStyleFamilyBitmap( eFamily, eColorMode );
}



