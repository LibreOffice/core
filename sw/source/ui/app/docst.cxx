/*************************************************************************
 *
 *  $RCSfile: docst.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-06 13:31:28 $
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
                        aName = *pShell->GetDoc()->GetTextNmArray()[
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
                SwEditWin& rEdtWin = pView->GetEditWin();
                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                rSet.Put(SfxBoolItem(nWhich, pApply && pApply->eType != 0));
                break;
            }

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
                            aParam = pDlg->GetName();
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
                        nRet = ApplyStyles(aParam, nFamily, pActShell,
                               rReq.GetModifier() );
                        // nur, wenn es kein API-Call war
                        if(!pActShell)
                            pView->GetEditWin().GrabFocus();
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
        PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));
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

Bitmap SwDocShell::GetStyleFamilyBitmap( SfxStyleFamily eFamily )
{
    if( SFX_STYLE_FAMILY_PSEUDO == eFamily )
        return Bitmap( SW_RES( BMP_STYLES_FAMILY_NUM ));
    return SfxObjectShell::GetStyleFamilyBitmap( eFamily );
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:31  hr
    initial import

    Revision 1.149  2000/09/18 16:05:11  willem.vandorp
    OpenOffice header added.

    Revision 1.148  2000/09/07 15:59:20  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.147  2000/07/07 13:26:09  jp
    must changes VCL

    Revision 1.146  2000/05/26 07:21:28  os
    old SW Basic API Slots removed

    Revision 1.145  2000/04/20 12:53:37  os
    GetName() returns String&

    Revision 1.144  2000/02/11 14:42:41  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.143  1999/11/29 17:25:33  jp
    some changes for the compat. attribut

    Revision 1.142  1999/06/09 17:34:54  JP
    have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp


      Rev 1.141   09 Jun 1999 19:34:54   JP
   have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp

      Rev 1.140   28 May 1999 10:24:20   JP
   zu Bug #66327#: LoadStyles - FixFelder aktualisieren

      Rev 1.139   22 Feb 1999 11:28:06   OS
   #61359# freie Positionierung von Rahmen auch fuer Vorlagen wieder erlaubt

      Rev 1.138   27 Nov 1998 14:49:04   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.137   23 Oct 1998 13:17:20   OS
   #44617# Parentvorlage fuer SID_NEW_STYLE aus dem Gestalter

      Rev 1.136   24 Sep 1998 11:55:30   JP
   Bug #57028#: ggfs. das Undo veranlassen, kein Modified zurueck zu setzen

      Rev 1.135   08 Sep 1998 18:00:36   OS
   #56134# Metric fuer Text und HTML getrennt - auch im ::Edit

      Rev 1.134   08 Sep 1998 16:48:20   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.133   17 Aug 1998 16:06:36   OS
   GPF nach Shellwechsel waehrend Recording #55041#

      Rev 1.132   27 Jul 1998 16:55:20   JP
   Bug #45529#: LoadStyles muss ggfs. das Zuruecksetzen des ModifiedFlags verhindern

      Rev 1.131   26 Jun 1998 17:36:46   OS
   SwBGDestItem mit Which #51751#

      Rev 1.130   17 Jun 1998 18:19:46   OS
   SwBackgroundDestinationItem

      Rev 1.129   17 Jun 1998 08:34:34   OS
   im UpdateByExample sel. Eintrag mit eigener Id mitschicken #51141#

      Rev 1.128   10 Jun 1998 14:51:40   JP
   SetFrmFmt: zusaetzlicher Parameter - Orientierung beibehalten

      Rev 1.127   16 Apr 1998 16:00:42   OS
   Printing extensions fuer HTML

      Rev 1.126   31 Mar 1998 10:20:36   OM
   #45776 Per default kein Size-Item in Rahmenvorlagen

      Rev 1.125   14 Mar 1998 13:06:44   OS
   Chg: GetCurrentTemplateCommen mit SfxBindings&

      Rev 1.124   20 Jan 1998 15:31:52   OS
   RES_CHRATR:BACKGROUND hat jetzt SlotId

      Rev 1.123   08 Jan 1998 12:55:08   OS
   Zeichenhintergrund auch im Absatzformatdialog editierbar #46180#

      Rev 1.122   06 Jan 1998 07:27:08   OS
   New/UpdateByExample fuer Numererierung nur wenn sie gesetzt ist erlauben #46348#

      Rev 1.121   09 Dec 1997 16:50:20   JP
   neu: ReplaceNumRule fuer MakeByExample fuer NumerierungsVorlagen

      Rev 1.120   04 Dec 1997 12:27:58   OS
   Edit/FormatPage: WrtShell nicht mehr const

      Rev 1.119   24 Nov 1997 16:57:24   JP
   GetState: String in der Schleife anlegen

      Rev 1.118   24 Nov 1997 14:22:38   MA
   includes

      Rev 1.117   17 Nov 1997 10:20:10   JP
   Umstellung Numerierung

      Rev 1.116   11 Nov 1997 13:22:14   JP
   neu: NumRule-Vorlage

      Rev 1.115   05 Nov 1997 09:33:30   JP
   Bug #45341#: SetItemSet nie mit dem eigenen ItemSet aufrufen

      Rev 1.114   19 Sep 1997 14:31:24   JP
   GPF beim Abschalten der Kopf-/Fusszeilen behoben

      Rev 1.113   04 Sep 1997 17:13:36   MA
   includes

      Rev 1.112   03 Sep 1997 15:53:52   OS
   DLL-Umbau

      Rev 1.111   21 Aug 1997 16:09:10   OS
   Return fuer Basic nur noch Null und Eins #43004#

      Rev 1.110   15 Aug 1997 11:45:06   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.109   12 Aug 1997 15:58:36   OS
   frmitems/textitem/paraitem aufgeteilt

      Rev 1.108   08 Aug 1997 17:26:42   OM
   Headerfile-Umstellung

      Rev 1.107   07 Aug 1997 14:59:24   OM
   Headerfile-Umstellung

      Rev 1.106   01 Aug 1997 17:03:30   OM
   Basic: NewStyle ohne Dialog ausfuehren

      Rev 1.105   31 Jul 1997 10:13:54   OM
   #42247# Template-GPF behoben

      Rev 1.104   15 Jul 1997 11:34:20   OS
   MacroName auch als sub(lib.module)

      Rev 1.103   05 Jul 1997 09:57:26   OS
   Macro und Script an Rahmenvorlagen setzen/lesen

      Rev 1.102   18 Jun 1997 10:47:58   JP
   GetStateStyleSheet: der Stylist gibt die Family vor, NIE selbst bestimmen

      Rev 1.101   16 Jun 1997 15:50:26   MA
   #40739# kein Anchor bei UpdateByExample

      Rev 1.100   28 May 1997 08:15:36   OS
   Metric fuer Dialog nicht in der Statusmethode setzen, sondern im Execute

      Rev 1.99   21 Feb 1997 15:31:20   OS
   Watercan ueber den Pointer am EditWin erkennen

      Rev 1.98   19 Feb 1997 15:47:00   JP
   Execute: UpdateByExample/NewByExample - akt. Family wie beim Gestalter festlegen

      Rev 1.97   11 Feb 1997 14:31:46   JP
   GetState: Family umschalten wenn das ApplyFlag erfragt wird

      Rev 1.96   10 Feb 1997 14:21:52   JP
   Im HTML-Mode nur die HTML-Vorlagen anzeigen

      Rev 1.95   04 Feb 1997 13:53:58   JP
   StateStyle: kein Disable bei den Familien

      Rev 1.94   28 Jan 1997 16:30:42   OS
   vor dem StyleDialog den HtmlMode in die DocShell putten

      Rev 1.93   16 Jan 1997 16:37:22   OS
   Metric vor Dialogaufruf an der App setzen

      Rev 1.92   13 Jan 1997 14:57:50   JP
   virtuelle StandardZeichenvorlage im Gestalter

      Rev 1.91   13 Jan 1997 13:46:06   JP
   Umstellungen fuers Update

      Rev 1.90   12 Dec 1996 13:24:38   JP
   GetState: Numerierung schaltet den Gestalter bei Automatisch auf Text (nicht mehr Num.)

      Rev 1.89   04 Dec 1996 17:44:10   JP
   StateStyleSheet: im HTML-Mode den HTML-Bereich defaulten

      Rev 1.88   04 Dec 1996 15:05:26   JP
   PoolVorlagen: neuer Bereich - HTML

      Rev 1.87   14 Nov 1996 19:14:32   OS
   Umstellung SlotIds

      Rev 1.86   06 Nov 1996 10:20:14   JP
   NewByExample: PageDesc - PoolIds beibehalten

      Rev 1.85   04 Nov 1996 15:02:58   JP
   neu: NewByExample fuer PageDescs

      Rev 1.84   29 Oct 1996 12:12:06   AMA
   New: Zeichenvorlagen duerfen jetzt denselben Bereich umfassen (DONTREPLACE)

      Rev 1.83   28 Oct 1996 16:59:10   AMA
   New: Modifier durchreichen

      Rev 1.82   01 Oct 1996 13:40:12   AMA
   Fix: RES_CHRATR_BACKGROUND nur bei Aenderung setzen.

      Rev 1.81   18 Sep 1996 14:30:44   AMA
   Chg: Zeichenhintergrund

      Rev 1.80   17 Sep 1996 16:20:06   OS
   UI-Anpassung bedingte Vorlagen

      Rev 1.79   09 Sep 1996 15:10:40   OS
   StyleSheet-Slots koennen auch WrtShell mitbekommen - API-Aufruf von Hintergrund-Views

      Rev 1.78   23 Aug 1996 12:26:40   JP
   Bug #30613#: GetStyleState - auf die richtige Shelle zugreifen

      Rev 1.77   29 Jul 1996 19:37:30   MA
   includes

      Rev 1.76   29 Jul 1996 15:14:46   OS
   FormatPage und Edit mit akt. View

      Rev 1.75   12 Jun 1996 14:55:40   JP
   Bug #28625#: Edit - neue Vorlage -> Pool benachrictigen

      Rev 1.74   07 May 1996 23:01:44   JP
   Bug #27306#: LoadStyles - Start/End-AllAction nur wenn Shell vorhanden ist

      Rev 1.73   22 Apr 1996 16:48:16   JP
   Bug #26860#: Doc als nicht modifiziert kennzeichnen

      Rev 1.72   28 Mar 1996 19:03:38   OS
   nach PageStyleDialog Lineal invalidieren

      Rev 1.71   22 Mar 1996 13:55:04   HJS
   umstellung 311

      Rev 1.70   11 Mar 1996 09:58:46   OS
   StateStyleSheet hat jetzt opt. WrtShell* als 2. Param. wg. Basic

      Rev 1.69   20 Feb 1996 16:27:26   OS
   neu: FormatPage - ruft Seitenformatdialog ggf. mit Spaltenseite auf

      Rev 1.68   13 Feb 1996 14:30:12   OS
   ExecStyleSheet: ReturnValue immer am Request setzen

      Rev 1.67   12 Jan 1996 19:39:24   JP
   ExecStyleSheet: Mask wird vom SFX mit gegeben, Edit und NewByExample wollen es haben

------------------------------------------------------------------------*/


