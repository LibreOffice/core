/*************************************************************************
 *
 *  $RCSfile: appopt.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:59:59 $
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


#pragma hdrstop

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>          // Funktion-Ids
#endif

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_OPTGRID_HXX //autogen
#include <svx/optgrid.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif
#ifndef _OFF_OFAIDS_HRC
#include <offmgr/ofaids.hrc>
#endif

#ifndef _OPTPAGE_HXX //autogen
#include <optpage.hxx>
#endif
#ifndef _OPTLOAD_HXX
#include <optload.hxx>
#endif
#ifndef _OPTCOMP_HXX
#include <optcomp.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>                   // fuer ::GetGlossaries()
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>      //fuer UpdateFields
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _CFGITEMS_HXX
#include <cfgitems.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _GLOBALS_H
#include <globals.h>        // globale Konstanten z.B.
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
/* -----------------12.02.99 12:28-------------------
 *
 * --------------------------------------------------*/

SfxItemSet*  SwModule::CreateItemSet( USHORT nId )
{
    BOOL bTextDialog = (nId == SID_SW_EDITOPTIONS) ? TRUE : FALSE;

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
            BOOL bWebView = 0 != PTR_CAST(SwWebView, pAppView);
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
#ifndef PRODUCT
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

    SfxPrinter* pPrt = 0;
    if( pAppView )
    {
        SwWrtShell& rWrtShell = pAppView->GetWrtShell();

        SfxPrinter* pPrt = rWrtShell.GetPrt();
        if( pPrt )
            pRet->Put(SwPtrItem(FN_PARAM_PRINTER, pPrt));
        pRet->Put(SwPtrItem(FN_PARAM_WRTSHELL, &rWrtShell));

        pRet->Put((const SvxLanguageItem&)
            rWrtShell.GetDefault(RES_CHRATR_LANGUAGE), SID_ATTR_LANGUAGE);

        pRet->Put((const SvxLanguageItem&)
            rWrtShell.GetDefault(RES_CHRATR_CJK_LANGUAGE), SID_ATTR_CHAR_CJK_LANGUAGE);
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

        // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
        SwLinguConfig aLinguCfg;

        Any aLang = aLinguCfg.GetProperty(C2U("DefaultLocale"));
        Locale aLocale;
        aLang >>= aLocale;
        pRet->Put(SvxLanguageItem(SvxLocaleToLanguage( aLocale ), SID_ATTR_LANGUAGE));

        aLang = aLinguCfg.GetProperty(C2U("DefaultLocale_CJK"));
        aLang >>= aLocale;
        pRet->Put(SvxLanguageItem(SvxLocaleToLanguage( aLocale ), SID_ATTR_CHAR_CJK_LANGUAGE));

    }
    if(bTextDialog)
        pRet->Put(SwPtrItem(FN_PARAM_STDFONTS, GetStdFontConfig()));
    if( PTR_CAST( SwPagePreView, SfxViewShell::Current())!=0)
    {
        SfxBoolItem aBool(SfxBoolItem(SID_PRINTPREVIEW, TRUE));
        pRet->Put(aBool);
    }

    FieldUnit eUnit = pPref->GetHScrollMetric();
    if(pAppView)
        pAppView->GetHLinealMetric(eUnit);
    pRet->Put(SfxUInt16Item( FN_HSCROLL_METRIC, eUnit));

    eUnit = pPref->GetVScrollMetric();
    if(pAppView)
        pAppView->GetVLinealMetric(eUnit);
    pRet->Put(SfxUInt16Item( FN_VSCROLL_METRIC, eUnit));
    pRet->Put(SfxUInt16Item(SID_ATTR_METRIC, pPref->GetMetric()));
    if(bTextDialog)
    {
        if(pAppView)
        {
            const SvxTabStopItem& rDefTabs =
                    (const SvxTabStopItem&)pAppView->GetWrtShell().
                                        GetDefault(RES_PARATR_TABSTOP);
                pRet->Put( SfxUInt16Item( SID_ATTR_DEFTABSTOP, (USHORT)::GetTabDist(rDefTabs)));
        }
        else
            pRet->Put(SfxUInt16Item( SID_ATTR_DEFTABSTOP, (UINT16)pPref->GetDefTab()));
    }

    /*-----------------01.02.97 11.13-------------------
    Optionen fuer GridTabPage
    --------------------------------------------------*/

    SvxGridItem aGridItem( SID_ATTR_GRID_OPTIONS);

    aGridItem.SetUseGridSnap( aViewOpt.IsSnap());
    aGridItem.SetSynchronize( aViewOpt.IsSynchronize());
    aGridItem.SetGridVisible( aViewOpt.IsGridVisible());

    const Size& rSnapSize = aViewOpt.GetSnapSize();
    aGridItem.SetFldDrawX( (USHORT) (rSnapSize.Width() ));
    aGridItem.SetFldDrawY( (USHORT) (rSnapSize.Height()));

    aGridItem.SetFldDivisionX( aViewOpt.GetDivisionX());
    aGridItem.SetFldDivisionY( aViewOpt.GetDivisionY());

    pRet->Put(aGridItem);

    /*-----------------01.02.97 13.02-------------------
        Optionen fuer PrintTabPage
    --------------------------------------------------*/
    SwPrintData* pOpt = pAppView ? pAppView->GetWrtShell().GetPrintData() : 0;
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

#ifndef PRODUCT
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
void SwModule::ApplyItemSet( USHORT nId, const SfxItemSet& rSet )
{
    BOOL bTextDialog = nId == SID_SW_EDITOPTIONS;
    SwView* pAppView = GetView();
    if(pAppView && pAppView->GetViewFrame() != SfxViewFrame::Current())
        pAppView = 0;
    if(pAppView)
    {
        // the text dialog mustn't apply data to the web view and vice versa
        BOOL bWebView = 0 != PTR_CAST(SwWebView, pAppView);
        if( (bWebView == bTextDialog))
            pAppView = 0; //
    }

    SwViewOption aViewOpt = *GetUsrPref(!bTextDialog);
    SwModuleOptions* pMCfg = GetModuleConfig();

    const SfxPoolItem* pItem;
    SfxBindings *pBindings = pAppView ? &pAppView->GetViewFrame()->GetBindings()
                                 : NULL;

    /*---------------------------------------------------------------------
            Seite Dokumentansicht auswerten
    -----------------------------------------------------------------------*/
    if( SFX_ITEM_SET == rSet.GetItemState(
                FN_PARAM_DOCDISP, FALSE, &pItem ))
    {
        const SwDocDisplayItem* pDocDispItem = (const SwDocDisplayItem*)pItem;

        if(!aViewOpt.IsViewMetaChars())
        {
            if(     !aViewOpt.IsTab( TRUE ) &&  pDocDispItem->bTab ||
                    !aViewOpt.IsBlank( TRUE ) && pDocDispItem->bSpace ||
                    !aViewOpt.IsParagraph( TRUE ) && pDocDispItem->bParagraphEnd ||
                    !aViewOpt.IsLineBreak( TRUE ) && pDocDispItem->bManualBreak )
            {
                aViewOpt.SetViewMetaChars(TRUE);
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

    if( SFX_ITEM_SET == rSet.GetItemState(
                                    FN_PARAM_ELEM, FALSE, &pItem ) )
    {
        const SwElemItem* pElemItem = (const SwElemItem*)pItem;
        pElemItem->FillViewOptions( aViewOpt );

    }

    if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_METRIC,
                                                    FALSE, &pItem ) )
    {
        SFX_APP()->SetOptions(rSet);
        const SfxUInt16Item* pMetricItem = (const SfxUInt16Item*)pItem;
        ::SetDfltMetric((FieldUnit)pMetricItem->GetValue(), !bTextDialog);
    }
    if( SFX_ITEM_SET == rSet.GetItemState(FN_HSCROLL_METRIC,
                                                    FALSE, &pItem ) )
    {
        SFX_APP()->SetOptions(rSet);
        const SfxUInt16Item* pMetricItem = (const SfxUInt16Item*)pItem;
        FieldUnit eUnit = (FieldUnit)pMetricItem->GetValue();
        pUsrPref->SetHScrollMetric(eUnit);
        if(pAppView)
            pAppView->ChangeTabMetric(eUnit);
    }

    if( SFX_ITEM_SET == rSet.GetItemState(FN_VSCROLL_METRIC,
                                                    FALSE, &pItem ) )
    {
        SFX_APP()->SetOptions(rSet);
        const SfxUInt16Item* pMetricItem = (const SfxUInt16Item*)pItem;
        FieldUnit eUnit = (FieldUnit)pMetricItem->GetValue();
        pUsrPref->SetVScrollMetric(eUnit);
        if(pAppView)
            pAppView->ChangeVLinealMetric(eUnit);
    }

    if( SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_DEFTABSTOP,
                                                    FALSE, &pItem ) )
    {
        USHORT nTabDist = ((const SfxUInt16Item*)pItem)->GetValue();
        pUsrPref->SetDefTab(nTabDist);
        if(pAppView)
        {
            SvxTabStopItem aDefTabs( 0, 0 );
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

    if( SFX_ITEM_SET == rSet.GetItemState(
                                SID_ATTR_GRID_OPTIONS, FALSE, &pItem ))
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

    if( SFX_ITEM_SET == rSet.GetItemState(
                                FN_PARAM_ADDPRINTER, FALSE, &pItem ))
    {
        SwPrintOptions* pOpt = GetPrtOptions(!bTextDialog);
        if (pOpt)
        {
            const SwAddPrinterItem* pAddPrinterAttr = (const SwAddPrinterItem*)pItem;
            *pOpt = *pAddPrinterAttr;

            if(pAppView)
                pAppView->GetWrtShell().SetPrintData(*pOpt);
        }

    }

    if( SFX_ITEM_SET == rSet.GetItemState(
                        FN_PARAM_SHADOWCURSOR, FALSE, &pItem ))
    {
        ((SwShadowCursorItem*)pItem)->FillViewOptions( aViewOpt );
        if(pBindings)
            pBindings->Invalidate(FN_SHADOWCURSOR);
    }

    if( SFX_ITEM_SET == rSet.GetItemState(
                        FN_PARAM_CRSR_IN_PROTECTED, FALSE, &pItem ))
    {
        aViewOpt.SetCursorInProtectedArea(((const SfxBoolItem*)pItem)->GetValue());
    }


#ifndef PRODUCT
    /*--------------------------------------------------------------------------
                Writer Testseite auswerten
    ----------------------------------------------------------------------------*/

            if( SFX_ITEM_SET == rSet.GetItemState(
                        FN_PARAM_SWTEST, FALSE, &pItem ))
            {
                const SwTestItem* pTestItem = (const SwTestItem*)pItem;
                aViewOpt.SetTest1((BOOL)pTestItem->bTest1);
                aViewOpt.SetTest2((BOOL)pTestItem->bTest2);
                aViewOpt.SetTest3((BOOL)pTestItem->bTest3);
                aViewOpt.SetTest4((BOOL)pTestItem->bTest4);
                aViewOpt.SetTest5((BOOL)pTestItem->bTest5);
                aViewOpt.SetTest6((BOOL)pTestItem->bTest6);
                aViewOpt.SetTest7((BOOL)pTestItem->bTest7);
                aViewOpt.SetTest8((BOOL)pTestItem->bTest8);
                SwViewOption::SetTest9((BOOL)pTestItem->bTest9);
                aViewOpt.SetTest10((BOOL)pTestItem->bTest10);
            }
#endif
        // dann an der akt. View und Shell die entsp. Elemente setzen
    ApplyUsrPref( aViewOpt, pAppView,
                 bTextDialog? VIEWOPT_DEST_TEXT : VIEWOPT_DEST_WEB);
}
/* -----------------12.02.99 12:28-------------------
 *
 * --------------------------------------------------*/
SfxTabPage* SwModule::CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage* pRet = NULL;

    switch( nId )
    {
        case RID_SW_TP_CONTENT_OPT:
        case RID_SW_TP_HTML_CONTENT_OPT:
        pRet = SwContentOptPage::Create(pParent, rSet); break;
        case RID_SW_TP_HTML_OPTGRID_PAGE:
        case RID_SVXPAGE_GRID:
            pRet = SvxGridTabPage::Create(pParent, rSet);
        break;

        break;
        case RID_SW_TP_STD_FONT:
        case RID_SW_TP_STD_FONT_CJK:
        case RID_SW_TP_STD_FONT_CTL:
            pRet = SwStdFontTabPage::Create(pParent, rSet);
            if(RID_SW_TP_STD_FONT != nId)
                ((SwStdFontTabPage*)pRet)->SetFontMode(
                        RID_SW_TP_STD_FONT_CJK == nId ? FONT_GROUP_CJK : FONT_GROUP_CTL);
        break;
        case RID_SW_TP_HTML_OPTPRINT_PAGE:
        case RID_SW_TP_OPTPRINT_PAGE:
        {
            pRet = SwAddPrinterTabPage::Create(pParent, rSet);
            SvStringsDtor aFaxList;
            const USHORT nCount = Printer::GetQueueCount();
            for (USHORT i = 0; i < nCount; ++i)
            {
                String* pString = new String( Printer::GetQueueInfo( i ).GetPrinterName());
                String* &rpString = pString;
                aFaxList.Insert(rpString, 0);
            }
            (( SwAddPrinterTabPage* )pRet)->SetFax( aFaxList );
        }
        break;
        case RID_SW_TP_HTML_OPTTABLE_PAGE:
        case RID_SW_TP_OPTTABLE_PAGE:
        {
            pRet = SwTableOptionsTabPage::Create(pParent, rSet);
            SwView* pView = GetView();
            if(pView)
            {
                // wenn Text dann nicht WebView und umgekehrt
                BOOL bWebView = 0 != PTR_CAST(SwWebView, pView);
                if( (bWebView &&  RID_SW_TP_HTML_OPTTABLE_PAGE == nId) ||
                    (!bWebView &&  RID_SW_TP_HTML_OPTTABLE_PAGE != nId) )
                {
                    ((SwTableOptionsTabPage*)pRet)->SetWrtShell(pView->GetWrtShellPtr());
                }
            }
        }
        break;
        case RID_SW_TP_OPTSHDWCRSR:
        case RID_SW_TP_HTML_OPTSHDWCRSR:
            pRet = SwShdwCrsrOptionsTabPage::Create(pParent, rSet);
        break;
        case RID_SW_TP_REDLINE_OPT:     pRet = SwRedlineOptionsTabPage::Create(pParent, rSet); break;
        case RID_SW_TP_OPTLOAD_PAGE:    pRet = SwLoadOptPage::Create(pParent, rSet); break;
        case RID_SW_TP_OPTCOMPATIBILITY_PAGE:   pRet = SwCompatibilityOptPage::Create(pParent, rSet); break;
#ifndef PRODUCT
        case  RID_SW_TP_OPTTEST_PAGE:   pRet = SwTestTabPage::Create(pParent, rSet); break;
#endif
        case  RID_SW_TP_BACKGROUND:     pRet = SvxBackgroundTabPage::Create(pParent, rSet); break;
    }

    DBG_ASSERT( pRet, "SwModule::CreateTabPage(): Unknown tabpage id" )
    return pRet;
}



