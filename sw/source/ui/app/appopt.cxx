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

#include <cmdid.h>          // Function-Ids


#include <com/sun/star/i18n/ScriptType.hpp>

#include <hintids.hxx>
#include <cmdid.h>          // Funktion-Ids
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/bindings.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/tstpitem.hxx>
#include <svx/optgrid.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <fontcfg.hxx>
#include <optload.hxx>
#include <optcomp.hxx>
#include <edtwin.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <doc.hxx>
#include <wrtsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <uitool.hxx>
#include <initui.hxx>                   // for ::GetGlossaries()
#include <fldbas.hxx>      //for UpdateFields
#include <wview.hxx>
#include <cfgitems.hxx>
#include <prtopt.hxx>
#include <pview.hxx>
#include <usrpref.hxx>
#include <modcfg.hxx>
#include <glosdoc.hxx>
#include <uiitems.hxx>
#include <editeng/langitem.hxx>
#include <unotools/lingucfg.hxx>
#include <editeng/unolingu.hxx>
#include <globals.hrc>
#include <globals.h>        // e.g. global Constants
#include <svl/slstitm.hxx>
#include "swabstdlg.hxx"
#include <swwrtshitem.hxx>

#include <unomid.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

SfxItemSet*  SwModule::CreateItemSet( sal_uInt16 nId )
{
    bool bTextDialog = (nId == SID_SW_EDITOPTIONS);

    // the options for the Web- and Textdialog are put together here
        SwViewOption aViewOpt = *GetUsrPref(!bTextDialog);
        SwMasterUsrPref* pPref = bTextDialog ? pUsrPref : pWebUsrPref;
        // no MakeUsrPref, because only options from textdoks can be used here
        SwView* pAppView = GetView();
        if(pAppView && pAppView->GetViewFrame() != SfxViewFrame::Current())
            pAppView = 0;
        if(pAppView)
        {
        // if Text then no WebView and vice versa
            bool bWebView = 0 != PTR_CAST(SwWebView, pAppView);
            if( (bWebView &&  !bTextDialog) ||(!bWebView &&  bTextDialog))
            {
                aViewOpt = *pAppView->GetWrtShell().GetViewOptions();
            }
            else
                pAppView = 0; // with View, there's nothing to win here
        }

    // Options/Edit
    SfxItemSet* pRet = new SfxItemSet (GetPool(),   FN_PARAM_DOCDISP,       FN_PARAM_ELEM,
                                    SID_PRINTPREVIEW,       SID_PRINTPREVIEW,
                                    SID_ATTR_GRID_OPTIONS,  SID_ATTR_GRID_OPTIONS,
                                    FN_PARAM_PRINTER,       FN_PARAM_STDFONTS,
                                    FN_PARAM_WRTSHELL,      FN_PARAM_WRTSHELL,
                                    FN_PARAM_ADDPRINTER,    FN_PARAM_ADDPRINTER,
                                    SID_ATTR_METRIC,        SID_ATTR_METRIC,
                                    SID_ATTR_APPLYCHARUNIT, SID_ATTR_APPLYCHARUNIT,
                                    SID_ATTR_DEFTABSTOP,    SID_ATTR_DEFTABSTOP,
                                    RES_BACKGROUND,         RES_BACKGROUND,
                                    SID_HTML_MODE,          SID_HTML_MODE,
                                    FN_PARAM_SHADOWCURSOR,  FN_PARAM_SHADOWCURSOR,
                                    FN_PARAM_CRSR_IN_PROTECTED, FN_PARAM_CRSR_IN_PROTECTED,
                                    FN_HSCROLL_METRIC,      FN_VSCROLL_METRIC,
                                    SID_ATTR_LANGUAGE,      SID_ATTR_LANGUAGE,
                                    SID_ATTR_CHAR_CJK_LANGUAGE,   SID_ATTR_CHAR_CJK_LANGUAGE,
                                    SID_ATTR_CHAR_CTL_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
#if OSL_DEBUG_LEVEL > 1
                                    FN_PARAM_SWTEST,        FN_PARAM_SWTEST,
#endif
                                    0);

    pRet->Put( SwDocDisplayItem( aViewOpt, FN_PARAM_DOCDISP) );
    pRet->Put( SwElemItem( aViewOpt, FN_PARAM_ELEM) );
    if( bTextDialog )
    {
        pRet->Put( SwShadowCursorItem( aViewOpt, FN_PARAM_SHADOWCURSOR ));
        pRet->Put( SfxBoolItem(FN_PARAM_CRSR_IN_PROTECTED, aViewOpt.IsCursorInProtectedArea()));
    }

    if( pAppView )
    {
        SwWrtShell& rWrtShell = pAppView->GetWrtShell();

        SfxPrinter* pPrt = rWrtShell.getIDocumentDeviceAccess()->getPrinter( false );
        if( pPrt )
            pRet->Put(SwPtrItem(FN_PARAM_PRINTER, pPrt));
        pRet->Put(SwPtrItem(FN_PARAM_WRTSHELL, &rWrtShell));

        pRet->Put((const SvxLanguageItem&)
            rWrtShell.GetDefault(RES_CHRATR_LANGUAGE), SID_ATTR_LANGUAGE);

        pRet->Put((const SvxLanguageItem&)
            rWrtShell.GetDefault(RES_CHRATR_CJK_LANGUAGE), SID_ATTR_CHAR_CJK_LANGUAGE);

        pRet->Put((const SvxLanguageItem&)
            rWrtShell.GetDefault(RES_CHRATR_CTL_LANGUAGE), SID_ATTR_CHAR_CTL_LANGUAGE);
    }
    else
    {
        SvtLinguConfig aLinguCfg;
        com::sun::star::lang::Locale aLocale;
        LanguageType nLang;

        using namespace ::com::sun::star::i18n::ScriptType;

        Any aLang = aLinguCfg.GetProperty(OUString("DefaultLocale"));
        aLang >>= aLocale;
        nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aLocale, false), LATIN);
        pRet->Put(SvxLanguageItem(nLang, SID_ATTR_LANGUAGE));

        aLang = aLinguCfg.GetProperty(OUString("DefaultLocale_CJK"));
        aLang >>= aLocale;
        nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aLocale, false), ASIAN);
        pRet->Put(SvxLanguageItem(nLang, SID_ATTR_CHAR_CJK_LANGUAGE));

        aLang = aLinguCfg.GetProperty(OUString("DefaultLocale_CTL"));
        aLang >>= aLocale;
        nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aLocale, false), COMPLEX);
        pRet->Put(SvxLanguageItem(nLang, SID_ATTR_CHAR_CTL_LANGUAGE));
    }
    if(bTextDialog)
        pRet->Put(SwPtrItem(FN_PARAM_STDFONTS, GetStdFontConfig()));
    if( PTR_CAST( SwPagePreView, SfxViewShell::Current())!=0)
    {
        SfxBoolItem aBool(SfxBoolItem(SID_PRINTPREVIEW, sal_True));
        pRet->Put(aBool);
    }

    FieldUnit eUnit = pPref->GetHScrollMetric();
    if(pAppView)
        pAppView->GetHRulerMetric(eUnit);
    pRet->Put(SfxUInt16Item( FN_HSCROLL_METRIC, static_cast< sal_uInt16 >(eUnit)));

    eUnit = pPref->GetVScrollMetric();
    if(pAppView)
        pAppView->GetVRulerMetric(eUnit);
    pRet->Put(SfxUInt16Item( FN_VSCROLL_METRIC, static_cast< sal_uInt16 >(eUnit) ));
    pRet->Put(SfxUInt16Item( SID_ATTR_METRIC, static_cast< sal_uInt16 >(pPref->GetMetric()) ));
    pRet->Put(SfxBoolItem(SID_ATTR_APPLYCHARUNIT, pPref->IsApplyCharUnit()));
    if(bTextDialog)
    {
        if(pAppView)
        {
            const SvxTabStopItem& rDefTabs =
                    (const SvxTabStopItem&)pAppView->GetWrtShell().
                                        GetDefault(RES_PARATR_TABSTOP);
                pRet->Put( SfxUInt16Item( SID_ATTR_DEFTABSTOP, (sal_uInt16)::GetTabDist(rDefTabs)));
        }
        else
            pRet->Put(SfxUInt16Item( SID_ATTR_DEFTABSTOP, (sal_uInt16)pPref->GetDefTab()));
    }

    // Options for GridTabPage
    SvxGridItem aGridItem( SID_ATTR_GRID_OPTIONS);

    aGridItem.SetUseGridSnap( aViewOpt.IsSnap());
    aGridItem.SetSynchronize( aViewOpt.IsSynchronize());
    aGridItem.SetGridVisible( aViewOpt.IsGridVisible());

    const Size& rSnapSize = aViewOpt.GetSnapSize();
    aGridItem.SetFldDrawX( (sal_uInt16) (rSnapSize.Width() ));
    aGridItem.SetFldDrawY( (sal_uInt16) (rSnapSize.Height()));

    aGridItem.SetFldDivisionX( aViewOpt.GetDivisionX());
    aGridItem.SetFldDivisionY( aViewOpt.GetDivisionY());

    pRet->Put(aGridItem);

    // Options for PrintTabPage
    const SwPrintData* pOpt = pAppView ?
                        &pAppView->GetWrtShell().getIDocumentDeviceAccess()->getPrintData() :
                        0;

    if(!pOpt)
        pOpt = GetPrtOptions(!bTextDialog);

    SwAddPrinterItem aAddPrinterItem (FN_PARAM_ADDPRINTER, *pOpt );
    pRet->Put(aAddPrinterItem);

    // Options for Web background
    if(!bTextDialog)
    {
        pRet->Put(SvxBrushItem(aViewOpt.GetRetoucheColor(), RES_BACKGROUND));
    }

    if(!bTextDialog)
        pRet->Put(SfxUInt16Item(SID_HTML_MODE, HTMLMODE_ON));
    return pRet;
}

void SwModule::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
    bool bTextDialog = nId == SID_SW_EDITOPTIONS;
    SwView* pAppView = GetView();
    if(pAppView && pAppView->GetViewFrame() != SfxViewFrame::Current())
        pAppView = 0;
    if(pAppView)
    {
        // the text dialog mustn't apply data to the web view and vice versa
        bool bWebView = 0 != PTR_CAST(SwWebView, pAppView);
        if(bWebView == bTextDialog)
            pAppView = 0;
    }

    SwViewOption aViewOpt = *GetUsrPref(!bTextDialog);
    SwMasterUsrPref* pPref = bTextDialog ? pUsrPref : pWebUsrPref;

    const SfxPoolItem* pItem;
    SfxBindings *pBindings = pAppView ? &pAppView->GetViewFrame()->GetBindings()
                                 : NULL;


    // Interpret the page Documentview
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_DOCDISP, sal_False, &pItem ))
    {
        const SwDocDisplayItem* pDocDispItem = (const SwDocDisplayItem*)pItem;

        if(!aViewOpt.IsViewMetaChars())
        {
            if(     (!aViewOpt.IsTab( sal_True ) &&  pDocDispItem->bTab) ||
                    (!aViewOpt.IsBlank( sal_True ) && pDocDispItem->bSpace) ||
                    (!aViewOpt.IsParagraph( sal_True ) && pDocDispItem->bParagraphEnd) ||
                    (!aViewOpt.IsLineBreak( sal_True ) && pDocDispItem->bManualBreak) )
            {
                aViewOpt.SetViewMetaChars(sal_True);
                if(pBindings)
                    pBindings->Invalidate(FN_VIEW_META_CHARS);
            }

        }
        pDocDispItem->FillViewOptions( aViewOpt );
        if(pBindings)
        {
            pBindings->Invalidate(FN_VIEW_GRAPHIC);
            pBindings->Invalidate(FN_VIEW_HIDDEN_PARA);
        }
    }

    // Elements - interpret Item
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_ELEM, sal_False, &pItem ) )
    {
        const SwElemItem* pElemItem = (const SwElemItem*)pItem;
        pElemItem->FillViewOptions( aViewOpt );

    }

    if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_METRIC, sal_False, &pItem ) )
    {
        SFX_APP()->SetOptions(rSet);
        PutItem(*pItem);
        const SfxUInt16Item* pMetricItem = (const SfxUInt16Item*)pItem;
        ::SetDfltMetric((FieldUnit)pMetricItem->GetValue(), !bTextDialog);
    }
    if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_APPLYCHARUNIT,
                                                    sal_False, &pItem ) )
    {
        SFX_APP()->SetOptions(rSet);
        const SfxBoolItem* pCharItem = (const SfxBoolItem*)pItem;
        ::SetApplyCharUnit(pCharItem->GetValue(), !bTextDialog);
    }

    if( SFX_ITEM_SET == rSet.GetItemState(FN_HSCROLL_METRIC, sal_False, &pItem ) )
    {
        const SfxUInt16Item* pMetricItem = (const SfxUInt16Item*)pItem;
        FieldUnit eUnit = (FieldUnit)pMetricItem->GetValue();
        pPref->SetHScrollMetric(eUnit);
        if(pAppView)
            pAppView->ChangeTabMetric(eUnit);
    }

    if( SFX_ITEM_SET == rSet.GetItemState(FN_VSCROLL_METRIC, sal_False, &pItem ) )
    {
        const SfxUInt16Item* pMetricItem = (const SfxUInt16Item*)pItem;
        FieldUnit eUnit = (FieldUnit)pMetricItem->GetValue();
        pPref->SetVScrollMetric(eUnit);
        if(pAppView)
            pAppView->ChangeVRulerMetric(eUnit);
    }

    if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_DEFTABSTOP, sal_False, &pItem ) )
    {
        sal_uInt16 nTabDist = ((const SfxUInt16Item*)pItem)->GetValue();
        pPref->SetDefTab(nTabDist);
        if(pAppView)
        {
            SvxTabStopItem aDefTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
            MakeDefTabs( nTabDist, aDefTabs );
            pAppView->GetWrtShell().SetDefault( aDefTabs );
        }
    }

    // Background only in WebDialog
    if(SFX_ITEM_SET == rSet.GetItemState(RES_BACKGROUND))
    {
        const SvxBrushItem& rBrushItem = (const SvxBrushItem&)rSet.Get(
                                RES_BACKGROUND);
        aViewOpt.SetRetoucheColor( rBrushItem.GetColor() );
    }

    // Interpret page Grid Settings
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS, sal_False, &pItem ))
    {
        const SvxGridItem* pGridItem = (const SvxGridItem*)pItem;

        aViewOpt.SetSnap( pGridItem->GetUseGridSnap() );
        aViewOpt.SetSynchronize(pGridItem->GetSynchronize());
        if( aViewOpt.IsGridVisible() != pGridItem->GetGridVisible() )
            aViewOpt.SetGridVisible( pGridItem->GetGridVisible());
        Size aSize = Size( pGridItem->GetFldDrawX()  ,
                            pGridItem->GetFldDrawY()  );
        if( aViewOpt.GetSnapSize() != aSize )
            aViewOpt.SetSnapSize( aSize );
        short nDiv = (short)pGridItem->GetFldDivisionX() ;
        if( aViewOpt.GetDivisionX() != nDiv  )
            aViewOpt.SetDivisionX( nDiv );
        nDiv = (short)pGridItem->GetFldDivisionY();
        if( aViewOpt.GetDivisionY() != nDiv  )
            aViewOpt.SetDivisionY( nDiv  );

        if(pBindings)
        {
            pBindings->Invalidate(SID_GRID_VISIBLE);
            pBindings->Invalidate(SID_GRID_USE);
        }
    }

    // Interpret Writer Printer Options
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_ADDPRINTER, sal_False, &pItem ))
    {
        SwPrintOptions* pOpt = GetPrtOptions(!bTextDialog);
        if (pOpt)
        {
            const SwAddPrinterItem* pAddPrinterAttr = (const SwAddPrinterItem*)pItem;
            *pOpt = *pAddPrinterAttr;

            if(pAppView)
                pAppView->GetWrtShell().getIDocumentDeviceAccess()->setPrintData( *pOpt );
        }

    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, sal_False, &pItem ))
    {
        ((SwShadowCursorItem*)pItem)->FillViewOptions( aViewOpt );
        if(pBindings)
            pBindings->Invalidate(FN_SHADOWCURSOR);
    }

    if( pAppView )
    {
        SwWrtShell &rWrtSh = pAppView->GetWrtShell();
        const bool bAlignFormulas = rWrtSh.GetDoc()->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT );
        pPref->SetAlignMathObjectsToBaseline( bAlignFormulas );

        // don't align formulas in documents that are currently loading
        if (bAlignFormulas && !rWrtSh.GetDoc()->IsInReading())
            rWrtSh.AlignAllFormulasToBaseline();
    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_CRSR_IN_PROTECTED, sal_False, &pItem ))
    {
        aViewOpt.SetCursorInProtectedArea(((const SfxBoolItem*)pItem)->GetValue());
    }


        // set elements for the current view and shell
    ApplyUsrPref( aViewOpt, pAppView, bTextDialog? VIEWOPT_DEST_TEXT : VIEWOPT_DEST_WEB);
}

SfxTabPage* SwModule::CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage* pRet = NULL;
    SfxAllItemSet aSet(*(rSet.GetPool()));
    switch( nId )
    {
        case RID_SW_TP_CONTENT_OPT:
        case RID_SW_TP_HTML_CONTENT_OPT:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
            break;
        }
        case RID_SW_TP_HTML_OPTGRID_PAGE:
        case RID_SVXPAGE_GRID:
            pRet = SvxGridTabPage::Create(pParent, rSet);
        break;

        case RID_SW_TP_STD_FONT:
        case RID_SW_TP_STD_FONT_CJK:
        case RID_SW_TP_STD_FONT_CTL:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
            if(RID_SW_TP_STD_FONT != nId)
            {
                aSet.Put (SfxUInt16Item(SID_FONTMODE_TYPE, RID_SW_TP_STD_FONT_CJK == nId ? FONT_GROUP_CJK : FONT_GROUP_CTL));
                pRet->PageCreated(aSet);
            }
        }
        break;
        case RID_SW_TP_HTML_OPTPRINT_PAGE:
        case RID_SW_TP_OPTPRINT_PAGE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
            aSet.Put (SfxBoolItem(SID_FAX_LIST, sal_True));
            pRet->PageCreated(aSet);
        }
        break;
        case RID_SW_TP_HTML_OPTTABLE_PAGE:
        case RID_SW_TP_OPTTABLE_PAGE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
            SwView* pCurrView = GetView();
            if(pCurrView)
            {
                // if text then not WebView and vice versa
                bool bWebView = 0 != PTR_CAST(SwWebView, pCurrView);
                if( (bWebView &&  RID_SW_TP_HTML_OPTTABLE_PAGE == nId) ||
                    (!bWebView &&  RID_SW_TP_HTML_OPTTABLE_PAGE != nId) )
                {
                    aSet.Put (SwWrtShellItem(SID_WRT_SHELL,pCurrView->GetWrtShellPtr()));
                    pRet->PageCreated(aSet);
                }
            }
        }
        break;
        case RID_SW_TP_OPTSHDWCRSR:
        case RID_SW_TP_HTML_OPTSHDWCRSR:
        case RID_SW_TP_REDLINE_OPT:
        case RID_SW_TP_COMPARISON_OPT:
        case RID_SW_TP_OPTLOAD_PAGE:
        case RID_SW_TP_OPTCOMPATIBILITY_PAGE:
        case RID_SW_TP_MAILCONFIG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
            if (pRet && (nId == RID_SW_TP_OPTSHDWCRSR || nId == RID_SW_TP_HTML_OPTSHDWCRSR))
            {
                SwView* pCurrView = GetView();
                if(pCurrView)
                {
                    aSet.Put( SwWrtShellItem( SID_WRT_SHELL, pCurrView->GetWrtShellPtr() ) );
                    pRet->PageCreated(aSet);
                }
            }
        }
        break;
        case  RID_SW_TP_OPTTEST_PAGE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( nId );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
            break;
        }
        case  RID_SW_TP_BACKGROUND:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
            break;
        }
        case TP_OPTCAPTION_PAGE:
        case RID_SW_TP_OPTCAPTION_PAGE:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            if ( pFact )
            {
                ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SW_TP_OPTCAPTION_PAGE );
                if ( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, rSet );
            }
        }
        break;
    }

    if(!pRet)
        SAL_WARN( "sw", "SwModule::CreateTabPage(): Unknown tabpage id " << nId );
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
