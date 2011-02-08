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
#include "precompiled_sw.hxx"

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>

#include <hintids.hxx>
#include <cmdid.h>          // Funktion-Ids
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/printer.hxx>
#include <svx/htmlmode.hxx>
#include <sfx2/bindings.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/tstpitem.hxx>
#include <svx/optgrid.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <i18npool/mslangid.hxx>
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
#include <initui.hxx>                   // fuer ::GetGlossaries()
#include <fldbas.hxx>      //fuer UpdateFields
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
#include <globals.h>        // globale Konstanten z.B.
#include <svl/slstitm.hxx>
#include "swabstdlg.hxx"
#include <swwrtshitem.hxx>

#include <unomid.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

/* -----------------12.02.99 12:28-------------------
 *
 * --------------------------------------------------*/

SfxItemSet*  SwModule::CreateItemSet( sal_uInt16 nId )
{
    sal_Bool bTextDialog = (nId == SID_SW_EDITOPTIONS) ? sal_True : sal_False;

    // hier werden die Optionen fuer die Web- und den Textdialog zusmmengesetzt
        SwViewOption aViewOpt = *GetUsrPref(!bTextDialog);
        SwMasterUsrPref* pPref = bTextDialog ? pUsrPref : pWebUsrPref;
        //kein MakeUsrPref, da hier nur die Optionen von Textdoks genommen werden duerfen
        SwView* pAppView = GetView();
        if(pAppView && pAppView->GetViewFrame() != SfxViewFrame::Current())
            pAppView = 0;
        if(pAppView)
        {
        // wenn Text dann nicht WebView und umgekehrt
            sal_Bool bWebView = 0 != PTR_CAST(SwWebView, pAppView);
            if( (bWebView &&  !bTextDialog) ||(!bWebView &&  bTextDialog))
            {
                aViewOpt = *pAppView->GetWrtShell().GetViewOptions();
            }
            else
                pAppView = 0; // mit View kann hier nichts gewonnen werden
        }

    /********************************************************************/
    /*                                                                  */
    /* Optionen/Bearbeiten                                              */
    /*                                                                  */
    /********************************************************************/
    SfxItemSet* pRet = new SfxItemSet (GetPool(),   FN_PARAM_DOCDISP,       FN_PARAM_ELEM,
                                    SID_PRINTPREVIEW,       SID_PRINTPREVIEW,
                                    SID_ATTR_GRID_OPTIONS,  SID_ATTR_GRID_OPTIONS,
                                    FN_PARAM_PRINTER,       FN_PARAM_STDFONTS,
                                    FN_PARAM_WRTSHELL,      FN_PARAM_WRTSHELL,
                                    FN_PARAM_ADDPRINTER,    FN_PARAM_ADDPRINTER,
                                    SID_ATTR_METRIC,        SID_ATTR_METRIC,
                                    SID_ATTR_DEFTABSTOP,    SID_ATTR_DEFTABSTOP,
                                    RES_BACKGROUND,         RES_BACKGROUND,
                                    SID_HTML_MODE,          SID_HTML_MODE,
                                    FN_PARAM_SHADOWCURSOR,  FN_PARAM_SHADOWCURSOR,
                                    FN_PARAM_CRSR_IN_PROTECTED, FN_PARAM_CRSR_IN_PROTECTED,
                                    FN_HSCROLL_METRIC,      FN_VSCROLL_METRIC,
                                    SID_ATTR_LANGUAGE,      SID_ATTR_LANGUAGE,
                                    SID_ATTR_CHAR_CJK_LANGUAGE,   SID_ATTR_CHAR_CJK_LANGUAGE,
                                    SID_ATTR_CHAR_CTL_LANGUAGE, SID_ATTR_CHAR_CTL_LANGUAGE,
#ifdef DBG_UTIL
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
/*      Der Drucker wird jetzt von der TabPage erzeugt und auch geloescht
 *      SfxItemSet* pSet = new SfxItemSet( SFX_APP()->GetPool(),
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );

        pPrt = new SfxPrinter(pSet);
        pRet->Put(SwPtrItem(FN_PARAM_PRINTER, pPrt));*/

        SvtLinguConfig aLinguCfg;
        Locale aLocale;
        LanguageType nLang;

        using namespace ::com::sun::star::i18n::ScriptType;

        Any aLang = aLinguCfg.GetProperty(C2U("DefaultLocale"));
        aLang >>= aLocale;
        nLang = MsLangId::resolveSystemLanguageByScriptType(MsLangId::convertLocaleToLanguage(aLocale), LATIN);
        pRet->Put(SvxLanguageItem(nLang, SID_ATTR_LANGUAGE));

        aLang = aLinguCfg.GetProperty(C2U("DefaultLocale_CJK"));
        aLang >>= aLocale;
        nLang = MsLangId::resolveSystemLanguageByScriptType(MsLangId::convertLocaleToLanguage(aLocale), ASIAN);
        pRet->Put(SvxLanguageItem(nLang, SID_ATTR_CHAR_CJK_LANGUAGE));

        aLang = aLinguCfg.GetProperty(C2U("DefaultLocale_CTL"));
        aLang >>= aLocale;
        nLang = MsLangId::resolveSystemLanguageByScriptType(MsLangId::convertLocaleToLanguage(aLocale), COMPLEX);
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
        pAppView->GetHLinealMetric(eUnit);
    pRet->Put(SfxUInt16Item( FN_HSCROLL_METRIC, static_cast< sal_uInt16 >(eUnit)));

    eUnit = pPref->GetVScrollMetric();
    if(pAppView)
        pAppView->GetVLinealMetric(eUnit);
    pRet->Put(SfxUInt16Item( FN_VSCROLL_METRIC, static_cast< sal_uInt16 >(eUnit) ));
    pRet->Put(SfxUInt16Item( SID_ATTR_METRIC, static_cast< sal_uInt16 >(pPref->GetMetric()) ));
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

    /*-----------------01.02.97 11.13-------------------
    Optionen fuer GridTabPage
    --------------------------------------------------*/

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

    /*-----------------01.02.97 13.02-------------------
        Optionen fuer PrintTabPage
    --------------------------------------------------*/
    const SwPrintData* pOpt = pAppView ?
                        &pAppView->GetWrtShell().getIDocumentDeviceAccess()->getPrintData() :
                        0;

    if(!pOpt)
        pOpt = GetPrtOptions(!bTextDialog);

    SwAddPrinterItem aAddPrinterItem (FN_PARAM_ADDPRINTER, *pOpt );
    pRet->Put(aAddPrinterItem);

    /*-----------------01.02.97 13.12-------------------
        Optionen fuer Web-Hintergrund
    --------------------------------------------------*/
    if(!bTextDialog)
    {
        pRet->Put(SvxBrushItem(aViewOpt.GetRetoucheColor(), RES_BACKGROUND));
    }

#ifdef DBG_UTIL
    /*-----------------01.02.97 13.02-------------------
        Test-Optionen
    --------------------------------------------------*/
        SwTestItem aTestItem(FN_PARAM_SWTEST);
        aTestItem.bTest1 = aViewOpt.IsTest1();
        aTestItem.bTest2 = aViewOpt.IsTest2();
        aTestItem.bTest3 = aViewOpt.IsTest3();
        aTestItem.bTest4 =  aViewOpt.IsTest4();
        aTestItem.bTest5 = aViewOpt.IsTest5();
        aTestItem.bTest6 = aViewOpt.IsTest6();
        aTestItem.bTest7 = aViewOpt.IsTest7();
        aTestItem.bTest8 = aViewOpt.IsTest8();
        aTestItem.bTest9 = SwViewOption::IsTest9();
        aTestItem.bTest10 = aViewOpt.IsTest10();
        pRet->Put(aTestItem);
#endif
    /*-----------------01.02.97 13.04-------------------

    --------------------------------------------------*/
    if(!bTextDialog)
        pRet->Put(SfxUInt16Item(SID_HTML_MODE, HTMLMODE_ON));
//  delete pPrt;
    return pRet;
}
/* -----------------12.02.99 12:28-------------------
 *
 * --------------------------------------------------*/
void SwModule::ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet )
{
    sal_Bool bTextDialog = nId == SID_SW_EDITOPTIONS;
    SwView* pAppView = GetView();
    if(pAppView && pAppView->GetViewFrame() != SfxViewFrame::Current())
        pAppView = 0;
    if(pAppView)
    {
        // the text dialog mustn't apply data to the web view and vice versa
        sal_Bool bWebView = 0 != PTR_CAST(SwWebView, pAppView);
        if( (bWebView == bTextDialog))
            pAppView = 0; //
    }

    SwViewOption aViewOpt = *GetUsrPref(!bTextDialog);
    SwMasterUsrPref* pPref = bTextDialog ? pUsrPref : pWebUsrPref;

    const SfxPoolItem* pItem;
    SfxBindings *pBindings = pAppView ? &pAppView->GetViewFrame()->GetBindings()
                                 : NULL;

    /*---------------------------------------------------------------------
            Seite Dokumentansicht auswerten
    -----------------------------------------------------------------------*/
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

    /*---------------------------------------------------------------------
                Elemente - Item auswerten
    -----------------------------------------------------------------------*/

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_ELEM, sal_False, &pItem ) )
    {
        const SwElemItem* pElemItem = (const SwElemItem*)pItem;
        pElemItem->FillViewOptions( aViewOpt );

    }

    if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_METRIC, sal_False, &pItem ) )
    {
        SFX_APP()->SetOptions(rSet);
        const SfxUInt16Item* pMetricItem = (const SfxUInt16Item*)pItem;
        ::SetDfltMetric((FieldUnit)pMetricItem->GetValue(), !bTextDialog);
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
            pAppView->ChangeVLinealMetric(eUnit);
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


    /*-----------------01.02.97 11.36-------------------
        Hintergrund nur im WebDialog
    --------------------------------------------------*/
    if(SFX_ITEM_SET == rSet.GetItemState(RES_BACKGROUND))
    {
        const SvxBrushItem& rBrushItem = (const SvxBrushItem&)rSet.Get(
                                RES_BACKGROUND);
        aViewOpt.SetRetoucheColor( rBrushItem.GetColor() );
    }

    /*--------------------------------------------------------------------
            Seite Rastereinstellungen auswerten
    ----------------------------------------------------------------------*/

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

    //--------------------------------------------------------------------------
    //      Writer Drucker Zusatzeinstellungen auswerten
    //----------------------------------------------------------------------------

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


#ifdef DBG_UTIL
    /*--------------------------------------------------------------------------
                Writer Testseite auswerten
    ----------------------------------------------------------------------------*/

            if( SFX_ITEM_SET == rSet.GetItemState(
                        FN_PARAM_SWTEST, sal_False, &pItem ))
            {
                const SwTestItem* pTestItem = (const SwTestItem*)pItem;
                aViewOpt.SetTest1((sal_Bool)pTestItem->bTest1);
                aViewOpt.SetTest2((sal_Bool)pTestItem->bTest2);
                aViewOpt.SetTest3((sal_Bool)pTestItem->bTest3);
                aViewOpt.SetTest4((sal_Bool)pTestItem->bTest4);
                aViewOpt.SetTest5((sal_Bool)pTestItem->bTest5);
                aViewOpt.SetTest6((sal_Bool)pTestItem->bTest6);
                aViewOpt.SetTest7((sal_Bool)pTestItem->bTest7);
                aViewOpt.SetTest8((sal_Bool)pTestItem->bTest8);
                SwViewOption::SetTest9((sal_Bool)pTestItem->bTest9);
                aViewOpt.SetTest10((sal_Bool)pTestItem->bTest10);
            }
#endif
        // dann an der akt. View und Shell die entsp. Elemente setzen
    ApplyUsrPref( aViewOpt, pAppView, bTextDialog? VIEWOPT_DEST_TEXT : VIEWOPT_DEST_WEB);
}
/* -----------------12.02.99 12:28-------------------
 *
 * --------------------------------------------------*/
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
                // wenn Text dann nicht WebView und umgekehrt
                sal_Bool bWebView = 0 != PTR_CAST(SwWebView, pCurrView);
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
#ifdef DBG_UTIL
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
#endif
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

    DBG_ASSERT( pRet, "SwModule::CreateTabPage(): Unknown tabpage id" );
    return pRet;
}

