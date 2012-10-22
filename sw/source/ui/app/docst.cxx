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


#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <hintids.hxx>
#include <sfx2/app.hxx>
#include <svl/whiter.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/printer.hxx>
#include <svl/macitem.hxx>
#include <editeng/brshitem.hxx>
#include <svl/stritem.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/eeitem.hxx>
#include <svx/htmlmode.hxx>
#include <swmodule.hxx>
#include <wdocsh.hxx>
#include <fmtfsize.hxx>
#include <fchrfmt.hxx>
#include <svtools/htmlcfg.hxx>
#include <SwStyleNameMapper.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>

#include "view.hxx"
#include "wrtsh.hxx"
#include "docsh.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "viewopt.hxx"
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include "swstyle.h"
#include "frmfmt.hxx"
#include "charfmt.hxx"
#include "poolfmt.hxx"
#include "pagedesc.hxx"
#include "docstyle.hxx"
#include "uiitems.hxx"
#include "fmtcol.hxx"
#include "frmmgr.hxx"       //SwFrmValid
#include "swevent.hxx"
#include "edtwin.hxx"
#include "unochart.hxx"

#include "app.hrc"
#include <fmtui.hrc>
#include "swabstdlg.hxx"

#include <list.hxx>

#include <paratr.hxx>   //#outline level,add by zhaojianwei

using namespace ::com::sun::star;

void  SwDocShell::StateStyleSheet(SfxItemSet& rSet, SwWrtShell* pSh)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16  nWhich  = aIter.FirstWhich();
    sal_uInt16 nActualFamily = USHRT_MAX;

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
            nActualFamily = static_cast< sal_uInt16 >(pCommon->GetActualFamily());
    }

    while (nWhich)
    {
        // determine current template to every family
        String aName;
        switch (nWhich)
        {
            case SID_STYLE_APPLY:
            {// here the template and its family are passed to the StyleBox
             // so that this family is being showed
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
                        aName = SwStyleNameMapper::GetTextUINameArray()[
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

                    sal_uInt16 nMask = 0;
                    if( pDoc->get(IDocumentSettingAccess::HTML_MODE) )
                        nMask = SWSTYLEBIT_HTML;
                    else
                    {
                        const int nSelection = pShell->GetFrmType(0,sal_True);
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

                if( pDoc->get(IDocumentSettingAccess::HTML_MODE) )
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
                SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
                if( pDoc->get(IDocumentSettingAccess::HTML_MODE) && !rHtmlOpt.IsPrintLayoutExtension())
                    rSet.DisableItem( nWhich );
                else
                {
                    sal_uInt16 n = pShell->GetCurPageDesc( sal_False );
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
                OSL_FAIL("Invalid SlotId");
        }
        nWhich = aIter.NextWhich();
    }
}


/*--------------------------------------------------------------------
    Description:    evaluate StyleSheet-Requests
 --------------------------------------------------------------------*/


void SwDocShell::ExecStyleSheet( SfxRequest& rReq )
{
    sal_uInt16  nSlot   = rReq.GetSlot();
    sal_uInt16  nRet    = 0xffff;

    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    SwWrtShell* pActShell = 0;
    sal_Bool bSetReturn = sal_True;
    switch (nSlot)
    {
    case SID_STYLE_NEW:
        if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILY,
            sal_False, &pItem ))
        {
            sal_uInt16 nFamily = ((const SfxUInt16Item*)pItem)->GetValue();

            String sName;
            sal_uInt16 nMask = 0;
            if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_NEW,
                sal_False, &pItem ))
                sName = ((const SfxStringItem*)pItem)->GetValue();
            if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_MASK,
                sal_False, &pItem ))
                nMask = ((const SfxUInt16Item*)pItem)->GetValue();
            String sParent;
            if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_REFERENCE,
                sal_False, &pItem ))
                sParent = ((const SfxStringItem*)pItem)->GetValue();

            nRet = Edit( sName, sParent, nFamily, nMask, sal_True, sal_False, 0, rReq.IsAPI() );
        }
        break;

        case SID_STYLE_APPLY:
            if( !pArgs )
            {
                GetView()->GetViewFrame()->GetDispatcher()->Execute(SID_STYLE_DESIGNER, sal_False);
                break;
            }
            else
            {
                // convert internal StyleName to DisplayName (slot implementation uses the latter)
                SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, SID_APPLY_STYLE, sal_False );
                SFX_REQUEST_ARG( rReq, pFamilyItem, SfxStringItem, SID_STYLE_FAMILYNAME, sal_False );
                if ( pFamilyItem && pNameItem )
                {
                    uno::Reference< style::XStyleFamiliesSupplier > xModel(GetModel(), uno::UNO_QUERY);
                    try
                    {
                        uno::Reference< container::XNameAccess > xStyles;
                        uno::Reference< container::XNameAccess > xCont = xModel->getStyleFamilies();
                        xCont->getByName(pFamilyItem->GetValue()) >>= xStyles;
                        uno::Reference< beans::XPropertySet > xInfo;
                        xStyles->getByName( pNameItem->GetValue() ) >>= xInfo;
                        ::rtl::OUString aUIName;
                        xInfo->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayName")) ) >>= aUIName;
                        if ( !aUIName.isEmpty() )
                            rReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aUIName ) );
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }
            }

            // intentionally no break

        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_WATERCAN:
        case SID_STYLE_FAMILY:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            String aParam;
            sal_uInt16 nFamily = SFX_STYLE_FAMILY_PARA;
            sal_uInt16 nMask = 0;

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
                OSL_ENSURE( pArgs->Count(), "SfxBug ItemSet is empty");

                SwWrtShell* pShell = GetWrtShell();
                if( SFX_ITEM_SET == pArgs->GetItemState(nSlot, sal_False, &pItem ))
                    aParam = ((const SfxStringItem*)pItem)->GetValue();

                if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_FAMILY,
                    sal_False, &pItem ))
                    nFamily = ((const SfxUInt16Item*)pItem)->GetValue();

                if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_FAMILYNAME, sal_False, &pItem ))
                {
                    String aFamily = ((const SfxStringItem*)pItem)->GetValue();
                    if(aFamily.CompareToAscii("CharacterStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_CHAR;
                    else
                    if(aFamily.CompareToAscii("ParagraphStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_PARA;
                    else
                    if(aFamily.CompareToAscii("PageStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_PAGE;
                    else
                    if(aFamily.CompareToAscii("FrameStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_FRAME;
                    else
                    if(aFamily.CompareToAscii("NumberingStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_PSEUDO;
                }

                if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_MASK,
                    sal_False, &pItem ))
                    nMask = ((const SfxUInt16Item*)pItem)->GetValue();
                if( SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_WRTSHELL,
                    sal_False, &pItem ))
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
                        if(SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_UPD_BY_EX_NAME, sal_False, &pItem))
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
                        nRet = Edit(aParam, aEmptyStr, nFamily, nMask, sal_False, sal_False, pActShell );
                        break;
                    case SID_STYLE_DELETE:
                        nRet = Delete(aParam, nFamily);
                        break;
                    case SID_STYLE_APPLY:
                        // Shell-switch in ApplyStyles
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
                        OSL_FAIL("Invalid SlotId");
                }

                rReq.Done();
            }

            break;
        }
    }

    if(bSetReturn)
    {
        if(rReq.IsAPI()) // Basic only gets TRUE or FALSE
            rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet !=0));
        else
            rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet));
    }

}

/*--------------------------------------------------------------------
    Description:    Edit
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::Edit( const String &rName, const String &rParent, sal_uInt16 nFamily, sal_uInt16 nMask,
                         sal_Bool bNew, sal_uInt16 nPageId, SwWrtShell* pActShell,
                         sal_Bool bBasic )
{
    OSL_ENSURE(GetWrtShell(), "No Shell, no Styles");
    SfxStyleSheetBase *pStyle = 0;

    sal_uInt16 nRet = nMask;
    sal_Bool bModified = pDoc->IsModified();

    if( bNew )
    {
        if( SFXSTYLEBIT_ALL != nMask && SFXSTYLEBIT_USED != nMask )
            nMask |= SFXSTYLEBIT_USERDEF;
        else
            nMask = SFXSTYLEBIT_USERDEF;

        pStyle = &mxBasePool->Make( rName, (SfxStyleFamily)nFamily, nMask );

        // set the current one as Parent
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
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
                        if(USHRT_MAX != nId)
                            pColl =  pWrtShell->GetTxtCollFromPool( nId );
                    }
                    pDStyle->GetCollection()->SetDerivedFrom( pColl );
                    pDStyle->PresetParent( rParent );

                    //#outline level,add by zhaojianwei
                     /*When a new paragraph style is created based on a "to outline style
                        assigned" paragraph style, the outline level attribute and the list
                        style attribute of the new paragraph style have to be set to 0
                        respectively "".*/
                    if( pColl->IsAssignedToListLevelOfOutlineStyle())
                    {
                        SwNumRuleItem aItem(aEmptyStr);
                        pDStyle->GetCollection()->SetFmtAttr( aItem );
                        pDStyle->GetCollection()->SetAttrOutlineLevel( 0 );
                    }
                    //<-end,zhaojianwei

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
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
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
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT);
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
        pStyle = mxBasePool->Find( rName, (SfxStyleFamily)nFamily );
        OSL_ENSURE(pStyle, "Vorlage nicht gefunden");
    }

    if(!pStyle)
        return sal_False;

    // put dialogues together
    rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *(SwDocStyleSheet*)pStyle ) );
    if( SFX_STYLE_FAMILY_PARA == nFamily )
    {
        SfxItemSet& rSet = xTmp->GetItemSet();
        ::SwToSfxPageDescAttr( rSet );
        // firstly only a Zero
        rSet.Put(SwBackgroundDestinationItem(SID_PARA_BACKGRND_DESTINATION, 0));
        // merge list level indent attributes into the item set if needed
        xTmp->MergeIndentAttrsOfListStyle( rSet );
    }
    else if( SFX_STYLE_FAMILY_CHAR == nFamily )
    {
        SfxItemSet& rSet = xTmp->GetItemSet();
        const SfxPoolItem *pTmpBrush;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BACKGROUND,
            sal_True, &pTmpBrush ) )
        {
            SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
            aTmpBrush.SetWhich( RES_BACKGROUND );
            rSet.Put( aTmpBrush );
        }
    }
    if (!bBasic)
    {
        // prior to the dialog the HtmlMode at the DocShell is being sunk
        sal_uInt16 nHtmlMode = ::GetHtmlMode(this);

        // In HTML mode, we do not always have a printer. In order to show
        // the correct page size in the Format - Page dialog, we have to
        // get one here.
        SwWrtShell* pCurrShell = ( pActShell ? pActShell : pWrtShell );
        if( ( HTMLMODE_ON & nHtmlMode ) &&
            !pCurrShell->getIDocumentDeviceAccess()->getPrinter( false ) )
            pCurrShell->InitPrt( pCurrShell->getIDocumentDeviceAccess()->getPrinter( true ) );

        PutItem(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));
        FieldUnit eMetric = ::GetDfltMetric(0 != (HTMLMODE_ON&nHtmlMode));
        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialogdiet fail!");
        SfxAbstractTabDialog* pDlg = pFact->CreateTemplateDialog( DLG_TEMPLATE_BASE,
                                                    0, *(xTmp.get()), nFamily, nPageId,
                                                    pActShell ? pActShell : pWrtShell, bNew);
        OSL_ENSURE(pDlg, "Dialogdiet fail!");
        while (true)
        {
            short nButton = pDlg->Execute();
            if(RET_OK == nButton || RET_APPLY_TEMPLATE == nButton)
        {
            GetWrtShell()->StartAllAction();

            // newly set the mask only with paragraph-templates
            if( bNew )
            {
                nRet = SFX_STYLE_FAMILY_PARA == pStyle->GetFamily()
                        ? xTmp->GetMask()
                        : SFXSTYLEBIT_USERDEF;
            }
            else if( pStyle->GetMask() != xTmp->GetMask() )
                nRet = xTmp->GetMask();

            if( SFX_STYLE_FAMILY_PARA == nFamily )
            {
                SfxItemSet aSet( *pDlg->GetOutputItemSet() );
                ::SfxToSwPageDescAttr( *GetWrtShell(), aSet  );
                // reset indent attributes at paragraph style, if a list style
                // will be applied and no indent attributes will be applied.
                xTmp->SetItemSet( aSet, true );
            }
            else
            {
                if(SFX_STYLE_FAMILY_PAGE == nFamily)
                {
                    static const sal_uInt16 aInval[] = {
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
                        sal_False, &pTmpBrush ) )
                    {
                        SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                        aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                        aTmpSet.Put( aTmpBrush );
                    }
                    aTmpSet.ClearItem( RES_BACKGROUND );
                }
                xTmp->SetItemSet( aTmpSet );

                if( SFX_STYLE_FAMILY_PAGE == nFamily && SvtLanguageOptions().IsCTLFontEnabled() )
                {
                    const SfxPoolItem *pItem = NULL;
                    if( aTmpSet.GetItemState( GetPool().GetTrueWhich( SID_ATTR_FRAMEDIRECTION, sal_False ) , sal_True, &pItem ) == SFX_ITEM_SET )
                        SwChartHelper::DoUpdateAllCharts( pDoc );
                }
            }
            if(SFX_STYLE_FAMILY_PAGE == nFamily)
                pView->InvalidateRulerPos();

            if( bNew )
                mxBasePool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *xTmp.get() ) );

            // Destroy dialog before EndAction - with page-templates the
            // ItemSet must be destroyed, so that the cursors get removed
            // from Headers/Footers. Otherwise "GPF" happen!!!
            if(RET_OK == nButton)
                delete pDlg;

            pDoc->SetModified();
            if( !bModified )    // Bug 57028
            {
                pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
            }

            GetWrtShell()->EndAllAction();
        }
        else
        {
            if( bNew )
            {
                GetWrtShell()->Undo(1);
                pDoc->GetIDocumentUndoRedo().ClearRedo();
            }

            if( !bModified )
                pDoc->ResetModified();
            delete pDlg;
        }
            if(RET_APPLY_TEMPLATE != nButton)
                break;
        }
    }
    else
    {
        // prior to the dialog the HtmlMode at the DocShell is being sunk
        PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));

        GetWrtShell()->StartAllAction();

        // newly set the mask only with paragraph-templates
        if( bNew )
        {
            nRet = SFX_STYLE_FAMILY_PARA == pStyle->GetFamily()
                    ? xTmp->GetMask()
                    : SFXSTYLEBIT_USERDEF;
        }
        else if( pStyle->GetMask() != xTmp->GetMask() )
            nRet = xTmp->GetMask();

        if( SFX_STYLE_FAMILY_PARA == nFamily )
            ::SfxToSwPageDescAttr( *GetWrtShell(), xTmp->GetItemSet() );
        else
        {
            SfxItemSet aTmpSet( xTmp->GetItemSet() );
            if( SFX_STYLE_FAMILY_CHAR == nFamily )
            {
                const SfxPoolItem *pTmpBrush;
                if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND,
                    sal_False, &pTmpBrush ) )
                {
                    SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
                    aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
                    aTmpSet.Put( aTmpBrush );
                }
                aTmpSet.ClearItem( RES_BACKGROUND );
            }
            xTmp->SetItemSet( aTmpSet );
        }
        if(SFX_STYLE_FAMILY_PAGE == nFamily)
            pView->InvalidateRulerPos();

        if( bNew )
            mxBasePool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *xTmp.get() ) );

        pDoc->SetModified();
        if( !bModified )        // Bug 57028
        {
            pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
        }
        GetWrtShell()->EndAllAction();
    }

    return nRet;
}

/*--------------------------------------------------------------------
    Description:    Delete
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::Delete(const String &rName, sal_uInt16 nFamily)
{
    SfxStyleSheetBase *pStyle = mxBasePool->Find(rName, (SfxStyleFamily)nFamily);

    if(pStyle)
    {
        OSL_ENSURE(GetWrtShell(), "No Shell, no Styles");

        GetWrtShell()->StartAllAction();
        mxBasePool->Remove(pStyle);
        GetWrtShell()->EndAllAction();

        return sal_True;
    }
    return sal_False;
}

/*--------------------------------------------------------------------
    Description:    apply template
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::ApplyStyles(const String &rName, sal_uInt16 nFamily,
                               SwWrtShell* pShell, sal_uInt16 nMode )
{
    SwDocStyleSheet* pStyle =
        (SwDocStyleSheet*)mxBasePool->Find(rName, (SfxStyleFamily)nFamily);

    OSL_ENSURE(pStyle, "Where's the StyleSheet");

    if(!pStyle)
        return sal_False;

    SwWrtShell *pSh = pShell ? pShell : GetWrtShell();

    OSL_ENSURE( pSh, "No Shell, no Styles");

    pSh->StartAllAction();

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwFmtCharFmt aFmt(pStyle->GetCharFmt());
            pSh->SetAttr( aFmt, (nMode & KEY_SHIFT) ?
                nsSetAttrMode::SETATTR_DONTREPLACE : nsSetAttrMode::SETATTR_DEFAULT );
            break;
        }
        case SFX_STYLE_FAMILY_PARA:
        {
            // #i62675#
            // clear also list attributes at affected text nodes, if paragraph
            // style has the list style attribute set.
            pSh->SetTxtFmtColl( pStyle->GetCollection(), true );
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
            // reset indent attribute on applying list style
            // continue list of list style
            const SwNumRule* pNumRule = pStyle->GetNumRule();
            const String sListIdForStyle =pNumRule->GetDefaultListId();
            pSh->SetCurNumRule( *pNumRule, false, sListIdForStyle, true );
            break;
        }
        default:
            OSL_FAIL("Unknown family");
    }
    pSh->EndAllAction();

    return nFamily;
}

/*--------------------------------------------------------------------
    Description:    start watering-can
 --------------------------------------------------------------------*/



sal_uInt16 SwDocShell::DoWaterCan(const String &rName, sal_uInt16 nFamily)
{
    OSL_ENSURE(GetWrtShell(), "No Shell, no Styles");

    SwEditWin& rEdtWin = pView->GetEditWin();
    SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
    sal_Bool bWaterCan = !(pApply && pApply->eType != 0);

    if( !rName.Len() )
        bWaterCan = sal_False;

    SwApplyTemplate aTemplate;
    aTemplate.eType = nFamily;

    if(bWaterCan)
    {
        SwDocStyleSheet* pStyle =
            (SwDocStyleSheet*)mxBasePool->Find(rName, (SfxStyleFamily)nFamily);

        OSL_ENSURE (pStyle, "Where's the StyleSheet");

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
                OSL_FAIL("Unknown family");
        }
    }
    else
        aTemplate.eType = 0;

    pView->GetEditWin().SetApplyTemplate(aTemplate);

    return nFamily;
}

/*--------------------------------------------------------------------
    Description:    update template
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::UpdateStyle(const String &rName, sal_uInt16 nFamily, SwWrtShell* pShell)
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
    OSL_ENSURE(GetWrtShell(), "No Shell, no Styles");

    SwDocStyleSheet* pStyle =
        (SwDocStyleSheet*)mxBasePool->Find(rName, (SfxStyleFamily)nFamily);

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

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, pColl->GetName());

                GetWrtShell()->StartUndo(UNDO_INSFMTATTR, &aRewriter);
                GetWrtShell()->FillByEx(pColl);
                    // also apply template to remove hard set attributes
                GetWrtShell()->SetTxtFmtColl( pColl );
                GetWrtShell()->EndUndo();
                GetWrtShell()->EndAllAction();
            }
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrmFmt* pFrm = pStyle->GetFrmFmt();
            if( pCurrWrtShell->IsFrmSelected() && pFrm && !pFrm->IsDefault() )
            {
                SfxItemSet aSet( GetPool(), aFrmFmtSetRange );
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->GetFlyFrmAttr( aSet );

                // #i105535#
                // no update of anchor attribute
                aSet.ClearItem( RES_ANCHOR );

                pFrm->SetFmtAttr( aSet );

                    // also apply template to remove hard set attributes
                pCurrWrtShell->SetFrmFmt( pFrm, true );
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFmt* pChar = pStyle->GetCharFmt();
            if( pChar && !pChar->IsDefault() )
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx(pChar);
                    // also apply template to remove hard set attributes
                pCurrWrtShell->EndAllAction();
            }

        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            const SwNumRule* pCurRule;
            if( pStyle->GetNumRule() &&
                0 != ( pCurRule = pCurrWrtShell->GetCurNumRule() ))
            {
                SwNumRule aRule( *pCurRule );
                // #i91400#
                aRule.SetName( pStyle->GetNumRule()->GetName(),
                               *(pCurrWrtShell->GetDoc()) );
                pCurrWrtShell->ChgNumRuleFmts( aRule );
            }
        }
        break;
    }
    return nFamily;
}

/*--------------------------------------------------------------------
    Description:    NewByExample
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::MakeByExample( const String &rName, sal_uInt16 nFamily,
                                    sal_uInt16 nMask, SwWrtShell* pShell )
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
    SwDocStyleSheet* pStyle = (SwDocStyleSheet*)mxBasePool->Find(
                                            rName, (SfxStyleFamily)nFamily );
    if(!pStyle)
    {
        // preserve the current mask of PI, then the new one is
        // immediately merged with the viewable area
        if( SFXSTYLEBIT_ALL == nMask || SFXSTYLEBIT_USED == nMask )
            nMask = SFXSTYLEBIT_USERDEF;
        else
            nMask |= SFXSTYLEBIT_USERDEF;

        pStyle = (SwDocStyleSheet*)&mxBasePool->Make(rName,
                                (SfxStyleFamily)nFamily, nMask );
    }

    switch(nFamily)
    {
        case  SFX_STYLE_FAMILY_PARA:
        {
            SwTxtFmtColl* pColl = pStyle->GetCollection();
            if(pColl && !pColl->IsDefault())
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx(pColl);
                    // also apply template to remove hard set attributes
                pColl->SetDerivedFrom(pCurrWrtShell->GetCurTxtFmtColl());

                    // set the mask at the Collection:
                sal_uInt16 nId = pColl->GetPoolFmtId() & 0x87ff;
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

                pCurrWrtShell->SetTxtFmtColl(pColl);
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrmFmt* pFrm = pStyle->GetFrmFmt();
            if(pCurrWrtShell->IsFrmSelected() && pFrm && !pFrm->IsDefault())
            {
                pCurrWrtShell->StartAllAction();

                SfxItemSet aSet(GetPool(), aFrmFmtSetRange );
                pCurrWrtShell->GetFlyFrmAttr( aSet );

                SwFrmFmt* pFFmt = pCurrWrtShell->GetCurFrmFmt();
                pFrm->SetDerivedFrom( pFFmt );

                pFrm->SetFmtAttr( aSet );
                    // also apply template to remove hard set attributes
                pCurrWrtShell->SetFrmFmt( pFrm );
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFmt* pChar = pStyle->GetCharFmt();
            if(pChar && !pChar->IsDefault())
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx( pChar );
                pChar->SetDerivedFrom( pCurrWrtShell->GetCurCharFmt() );
                SwFmtCharFmt aFmt( pChar );
                pCurrWrtShell->SetAttr( aFmt );
                pCurrWrtShell->EndAllAction();
            }
        }
        break;

        case SFX_STYLE_FAMILY_PAGE:
        {
            pCurrWrtShell->StartAllAction();
            sal_uInt16 nPgDsc = pCurrWrtShell->GetCurPageDesc();
            SwPageDesc& rSrc = (SwPageDesc&)pCurrWrtShell->GetPageDesc( nPgDsc );
            SwPageDesc& rDest = *(SwPageDesc*)pStyle->GetPageDesc();

            sal_uInt16 nPoolId = rDest.GetPoolFmtId();
            sal_uInt16 nHId = rDest.GetPoolHelpId();
            sal_uInt8 nHFId = rDest.GetPoolHlpFileId();

            pCurrWrtShell->GetDoc()->CopyPageDesc( rSrc, rDest );

            // PoolId must NEVER be copied!
            rDest.SetPoolFmtId( nPoolId );
            rDest.SetPoolHelpId( nHId );
            rDest.SetPoolHlpFileId( nHFId );

            // when Headers/Footers are created, there is no Undo anymore!
            pCurrWrtShell->GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();

            pCurrWrtShell->EndAllAction();
        }
        break;

        case SFX_STYLE_FAMILY_PSEUDO:
        {
            pCurrWrtShell->StartAllAction();

            SwNumRule aRule( *pCurrWrtShell->GetCurNumRule() );
            String sOrigRule( aRule.GetName() );
            // #i91400#
            aRule.SetName( pStyle->GetNumRule()->GetName(),
                           *(pCurrWrtShell->GetDoc()) );
            pCurrWrtShell->ChgNumRuleFmts( aRule );

            pCurrWrtShell->ReplaceNumRule( sOrigRule, aRule.GetName() );


            pCurrWrtShell->EndAllAction();
        }
        break;
    }
    return nFamily;
}

void  SwDocShell::LoadStyles( SfxObjectShell& rSource )
{
    _LoadStyles(rSource, sal_False);
}
/* --------------------------------------------------
    bPreserveCurrentDocument determines whether SetFixFields() is called
    This call modifies the source document. This mustn't happen when the source
    is a document the user is working on.
    Calls of ::LoadStyles() normally use files especially loaded for the purpose
    of importing styles.
 --------------------------------------------------*/
void SwDocShell::_LoadStyles( SfxObjectShell& rSource, sal_Bool bPreserveCurrentDocument )
{
/*  [Description]

    This method is called by SFx if Styles have to be reloaded from a
    document-template. Existing Styles should be overwritten by that.
    That's why the document has to be reformatted. Therefore applications
    will usually overload this method and call the baseclass' implementation
    in their implementation.
*/
    // When the source is our document, we do the checking ourselves
    // (much quicker and doesn't use the crutch StxStylePool).
    if( rSource.ISA( SwDocShell ))
    {
        // in order for the Headers/Footers not to get the fixed content
        // of the template, update all the Source's
        // FixFields once.
        if(!bPreserveCurrentDocument)
            ((SwDocShell&)rSource).pDoc->SetFixFields(false, NULL);
        if( pWrtShell )
        {
            pWrtShell->StartAllAction();
            pDoc->ReplaceStyles( *((SwDocShell&)rSource).pDoc );
            pWrtShell->EndAllAction();
        }
        else
        {
            sal_Bool bModified = pDoc->IsModified();
            pDoc->ReplaceStyles( *((SwDocShell&)rSource).pDoc );
            if( !bModified && pDoc->IsModified() && !pView )
            {
                // the View is created later, but overwrites the Modify-Flag.
                // Undo doesn't work anymore anyways.
                pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
            }
        }
    }
    else
        SfxObjectShell::LoadStyles( rSource );
}


void SwDocShell::FormatPage( const String& rPage, sal_uInt16 nPageId, SwWrtShell*     pActShell )
{
    Edit( rPage, aEmptyStr, SFX_STYLE_FAMILY_PAGE, 0, sal_False, nPageId, pActShell);
}

Bitmap SwDocShell::GetStyleFamilyBitmap( SfxStyleFamily eFamily )
{
    if( SFX_STYLE_FAMILY_PSEUDO == eFamily )
        return Bitmap( SW_RES( BMP_STYLES_FAMILY_NUM ));
    return SfxObjectShell::GetStyleFamilyBitmap( eFamily );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
