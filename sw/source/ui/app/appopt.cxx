/*************************************************************************
 *
 *  $RCSfile: appopt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-09-28 15:22:17 $
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
#include <sot/dtrans.hxx>
#include "uiparam.hxx"
#include "hintids.hxx"


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
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
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
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _OPTPAGE_HXX //autogen
#include <optpage.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif

#include "optins.hxx"
#include "optload.hxx"
#include <edtwin.hxx>
#include "finder.hxx"
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "uitool.hxx"
#include "initui.hxx"                   // fuer ::GetGlossaries()
#include "fldbas.hxx"      //fuer UpdateFields
#include "cmdid.h"          // Funktion-Ids
#include "globals.hrc"
#include "globals.h"        // globale Konstanten z.B.
#include "wview.hxx"
#include "cfgitems.hxx"
#include "prtopt.hxx"
#include "pview.hxx"
#include "usrpref.hxx"
#include "modcfg.hxx"
#include "glosdoc.hxx"
#include "uiitems.hxx"



/* -----------------12.02.99 12:28-------------------
 *
 * --------------------------------------------------*/
SfxItemSet*  SwModule::CreateItemSet( USHORT nId )
{
    BOOL bTextDialog = (nId == SID_SW_EDITOPTIONS) ? TRUE : FALSE;

    // hier werden die Optionen fuer die Web- und den Textdialog zusmmengesetzt
        SwViewOption aViewOpt = *GetUsrPref(!bTextDialog);
        SwMasterUsrPref* pPref = bTextDialog ? pUsrPref : pWebUsrPref;
        BOOL bFrameDoc = FALSE;
        //kein MakeUsrPref, da hier nur die Optionen von Textdoks genommen werden duerfen
        SwView* pAppView = GetView();
        if(pAppView)
        {
        // wenn Text dann nicht WebView und umgekehrt
            BOOL bWebView = 0 != PTR_CAST(SwWebView, pAppView);
            if( (bWebView &&  !bTextDialog) ||(!bWebView &&  bTextDialog))
            {
                aViewOpt = *pAppView->GetWrtShell().GetViewOptions();
                SfxFrameIterator aIter( *pAppView->GetViewFrame()->GetTopFrame() );
                if( aIter.FirstFrame() )
                    bFrameDoc = TRUE;
            }
            else
                pAppView = 0; // mit View kann hier nichts gewonnen werden
        }
    //  rApp.MakeUsrPref(aViewOpt);


    /********************************************************************/
    /*                                                                  */
    /* Optionen/Bearbeiten                                              */
    /*                                                                  */
    /********************************************************************/
    SfxItemSet* pRet = new SfxItemSet (GetPool(),   FN_PARAM_DOCDISP,       FN_PARAM_ELEM,
                                    SID_PRINTPREVIEW,       SID_PRINTPREVIEW,
                                    SID_ATTR_GRID_OPTIONS,  SID_ATTR_GRID_OPTIONS,
                                    FN_PARAM_PRINTER,       FN_PARAM_WRTSHELL,
                                    FN_PARAM_ADDPRINTER,    FN_PARAM_ADDPRINTER,
                                    SID_ATTR_METRIC,        SID_ATTR_METRIC,
                                    SID_ATTR_DEFTABSTOP,    SID_ATTR_DEFTABSTOP,
                                    RES_BACKGROUND,         RES_BACKGROUND,
                                    SID_HTML_MODE,          SID_HTML_MODE,
                                    FN_PARAM_SHADOWCURSOR,  FN_PARAM_SHADOWCURSOR,
                                    FN_PARAM_CRSR_IN_PROTECTED, FN_PARAM_CRSR_IN_PROTECTED,
                                    FN_VIEW_IN_FRAME,       FN_VIEW_IN_FRAME,
#ifndef PRODUCT
                                    FN_PARAM_SWTEST,        FN_PARAM_SWTEST,
#endif
                                    0);

    if(bFrameDoc)
        pRet->Put(SfxBoolItem(FN_VIEW_IN_FRAME, TRUE));
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
    }
    if(bTextDialog)
        pRet->Put(SwPtrItem(FN_PARAM_STDFONTS, GetStdFontConfig()));
    if( PTR_CAST( SwPagePreView, SfxViewShell::Current())!=0)
    {
        SfxBoolItem aBool(SfxBoolItem(SID_PRINTPREVIEW, TRUE));
        pRet->Put(aBool);
    }

    pRet->Put(SfxUInt16Item( SID_ATTR_METRIC, pPref->GetMetric()));
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
            pRet->Put(SfxUInt16Item( SID_ATTR_DEFTABSTOP, pPref->GetDefTab()));
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
    SwPrintOptions* pOpt = GetPrtOptions(!bTextDialog);
    SwAddPrinterItem aAddPrinterItem (FN_PARAM_ADDPRINTER, pOpt );
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
    SwViewOption aViewOpt = *GetUsrPref(!bTextDialog);
    SwModuleOptions* pMCfg = GetModuleConfig();

        const SfxPoolItem* pItem;
        SfxBindings &rBnd = pAppView->GetViewFrame()->GetBindings();
        /*--------------------------------------------------------------------------
                Seite Dokumentansicht auswerten
        ----------------------------------------------------------------------------*/
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
                        rBnd.Invalidate(FN_VIEW_META_CHARS);
                    }

                }
                pDocDispItem->FillViewOptions( aViewOpt );
                rBnd.Invalidate(FN_VIEW_GRAPHIC);
                rBnd.Invalidate(FN_VIEW_HIDDEN_PARA);
            }

        /*--------------------------------------------------------------------------
                    Elemente - Item auswerten
        ----------------------------------------------------------------------------*/

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

        /*--------------------------------------------------------------------------
                    Seite Rastereinstellungen auswerten
        ----------------------------------------------------------------------------*/

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

                rBnd.Invalidate(SID_GRID_VISIBLE);
                rBnd.Invalidate(SID_GRID_USE);
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
                    pAddPrinterAttr->SetPrintOptions(pOpt);
                    pOpt->SetFaxName( pAddPrinterAttr->GetFax());
                }

            }

            if( SFX_ITEM_SET == rSet.GetItemState(
                                FN_PARAM_SHADOWCURSOR, FALSE, &pItem ))
            {
                ((SwShadowCursorItem*)pItem)->FillViewOptions( aViewOpt );
                rBnd.Invalidate(FN_SHADOWCURSOR);
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
        ApplyUsrPref(aViewOpt, pAppView,
                     bTextDialog? VIEWOPT_DEST_TEXT : VIEWOPT_DEST_WEB);
}
/* -----------------12.02.99 12:28-------------------
 *
 * --------------------------------------------------*/
SfxTabPage*  SwModule::CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage*  pRet = 0;

    switch(nId)
    {
        case RID_SW_TP_CONTENT_OPT:
        case RID_SW_TP_HTML_CONTENT_OPT:
        pRet = SwContentOptPage::Create(pParent, rSet); break;
        case RID_SW_TP_HTML_LAYOUT_OPT:
        case RID_SW_TP_LAYOUT_OPT:
            pRet = SwLayoutOptPage::Create(pParent, rSet);
        break;
        case RID_SW_TP_HTML_OPTGRID_PAGE:
        case RID_SVXPAGE_GRID:
            pRet = SvxGridTabPage::Create(pParent, rSet);
            ((SvxGridTabPage*)pRet)->HideSnapGrid();
        break;

        break;
        case RID_SW_TP_STD_FONT:        pRet = SwStdFontTabPage::Create(pParent, rSet); break;
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
        case RID_SW_TP_OPTSHDWCRSR:     pRet = SwShdwCrsrOptionsTabPage::Create(pParent, rSet); break;
        case RID_SW_TP_REDLINE_OPT:     pRet = SwRedlineOptionsTabPage::Create(pParent, rSet); break;
        case RID_SW_TP_OPTINSERT_HTML_PAGE:
        case RID_SW_TP_OPTINSERT_PAGE:  pRet = SwInsertOptPage::Create(pParent, rSet); break;
        case RID_SW_TP_OPTLOAD_PAGE:    pRet = SwLoadOptPage::Create(pParent, rSet); break;
#ifndef PRODUCT
        case  RID_SW_TP_OPTTEST_PAGE:   pRet = SwTestTabPage::Create(pParent, rSet); break;
#endif
        case  RID_SW_TP_BACKGROUND:     pRet = SvxBackgroundTabPage::Create(pParent, rSet); break;
        case  RID_SW_TP_OPTSRCVIEW:     pRet = SwSourceViewOptionsTabPage::Create(pParent, rSet); break;
    }
    DBG_ASSERT(pRet, "Id unbekannt")
    return pRet;
}

/*-------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:31  hr
    initial import

    Revision 1.163  2000/09/18 16:05:10  willem.vandorp
    OpenOffice header added.

    Revision 1.162  2000/09/07 15:59:19  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.161  2000/09/07 08:25:44  os
    FaxName now in SwPrintOptions

    Revision 1.160  2000/05/10 11:52:26  os
    Basic API removed

    Revision 1.159  2000/03/03 15:16:58  os
    StarView remainders removed

    Revision 1.158  2000/02/11 14:42:32  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.157  1999/06/25 06:34:50  OS
    #67193# default tabstop from current document


      Rev 1.156   25 Jun 1999 08:34:50   OS
   #67193# default tabstop from current document

      Rev 1.155   10 Jun 1999 10:52:08   JP
   have to change: no AppWin from SfxApp

      Rev 1.154   08 Jun 1999 15:29:16   OS
   #66606# HTML background: use WhichId

      Rev 1.153   19 Mar 1999 15:32:18   OS
   #63750# Tabellenmode auch wieder an der akt. Tabelle setzen

      Rev 1.152   04 Mar 1999 13:54:40   OS
   #62613# eigene Ids fuer alle HTML-Pages

      Rev 1.151   25 Feb 1999 10:19:04   OS
   #62329# eigene Id fuer Sw-Html-Einfuegen-Page

      Rev 1.150   17 Feb 1999 15:07:36   OS
   #61890# Methoden fuer alte Optionsdialoge geloescht

      Rev 1.149   16 Feb 1999 08:08:20   OS
   #61890# Syntaxfehler TestTabPage

      Rev 1.148   15 Feb 1999 09:20:36   OS
   #61890# Virt. Methoden fuer neuen Optionendialog ueberladen

      Rev 1.147   27 Jan 1999 10:05:22   OS
   #58677# Cursor in Readonly-Bereichen

      Rev 1.146   04 Dec 1998 13:35:26   OS
   #60117# Raster-Slots nach dem Dialog invalidieren

      Rev 1.145   08 Sep 1998 16:47:58   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.144   18 Aug 1998 16:48:54   TJ
   include

      Rev 1.143   13 Jul 1998 14:39:54   OS
   include #51814#

      Rev 1.142   13 Jul 1998 08:48:00   OS
   Dialog-Parent richtig setzen #51814#

      Rev 1.141   06 Jul 1998 14:53:46   OS
   Scrollbars in FrameDocs disablen

      Rev 1.140   29 Apr 1998 09:27:40   MA
   BackgroundBrush -> RetoucheColor

      Rev 1.139   05 Mar 1998 14:44:10   OM
   Redline-Attribute in Module-Cfg speichern

      Rev 1.138   02 Mar 1998 12:50:20   OS
   Schattencursor-Slot bei Bedarf invalidieren #47601#

      Rev 1.137   23 Feb 1998 12:40:06   OM
   Redlining-Optionen

      Rev 1.136   28 Nov 1997 14:26:52   MA
   includes

      Rev 1.135   24 Nov 1997 14:22:48   MA
   includes

      Rev 1.134   14 Nov 1997 17:14:04   OS
   TblChgMode auch an der akt. Tabelle setzen #45521#

      Rev 1.133   11 Nov 1997 14:02:58   MA
   precomp entfernt

      Rev 1.132   03 Nov 1997 16:13:08   JP
   neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor

      Rev 1.131   09 Aug 1997 12:46:32   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.130   08 Aug 1997 17:26:54   OM
   Headerfile-Umstellung

      Rev 1.129   08 Jul 1997 14:04:14   OS
   ConfigItems von der App ans Module

      Rev 1.128   19 Jun 1997 16:02:42   OS
   DefPaqe an den Dialog weiterreichen #40584#

      Rev 1.127   11 Jun 1997 09:17:04   OS
   Def-Page-Item fuer den Optionsdialog wieder auswerten #40584#

      Rev 1.126   09 Jun 1997 14:28:26   MA
   chg: Browse-Flag nur noch am Doc

      Rev 1.125   06 Jun 1997 12:43:12   MA
   chg: versteckte Absaetze ausblenden

      Rev 1.124   25 Mar 1997 15:55:46   OS
   _if_ fuer Semikolon ?

      Rev 1.123   05 Feb 1997 13:34:46   OS
   PrintOptins auch fuer Web

      Rev 1.122   03 Feb 1997 12:07:22   OS
   kompletter Einstellungesdialog fuer HTML-Dok

      Rev 1.121   30 Jan 1997 11:10:08   OS
   UsrPrefs verdoppelt

      Rev 1.120   14 Jan 1997 14:26:48   OS
   TabDist auch an der Shell setzen

      Rev 1.119   14 Jan 1997 08:46:26   MA
   includes

      Rev 1.118   13 Jan 1997 16:46:06   OS
   Metric und TabStop am Module

      Rev 1.117   19 Dec 1996 17:38:38   OS
   werden MetaChars ausgewaehlt, muessen auch die Sonderzeichen eingeschaltet werden

      Rev 1.116   11 Dec 1996 14:16:46   OS
   Optionenumbau - Reste

      Rev 1.115   04 Dec 1996 14:56:10   OS
   Umbau Optionsdialoge

      Rev 1.114   04 Dec 1996 14:19:30   OS
   neue Ids fuer Optionendialoge

      Rev 1.113   03 Dec 1996 16:57:02   AMA
   Chg: Der Drucker wird nur im !Browsemodus angelegt.

      Rev 1.112   11 Nov 1996 19:32:06   OS
   alten Code entfernt

      Rev 1.111   06 Nov 1996 19:49:24   MH
   SwSplCfg -> OfaSplCfg

      Rev 1.110   01 Nov 1996 18:02:42   MA
   Writer Module

      Rev 1.109   21 Oct 1996 09:37:04   OS
   ColorTable sichern

      Rev 1.108   07 Sep 1996 13:49:52   OS
   Hintergrundpage auch ohne BrowseView

      Rev 1.107   04 Sep 1996 08:03:48   OS
   neu: Grundschriften-Tabpage

      Rev 1.106   03 Sep 1996 16:54:18   OS
   Tabpage Standardfonts

      Rev 1.105   03 Sep 1996 10:15:58   OS
   Reste

      Rev 1.104   31 Aug 1996 17:15:08   OS
   neue Optionendialoge

      Rev 1.103   30 Aug 1996 12:36:20   OS
   UpdateGlosPath kann Blockliste aktualisieren

      Rev 1.102   27 Aug 1996 11:50:54   OS
   alten cast entfernt

      Rev 1.101   26 Aug 1996 16:55:40   OS
   neu: Brush fuer BrowseView

      Rev 1.100   29 Jul 1996 19:37:42   MA
   includes

      Rev 1.99   17 Jul 1996 13:41:06   OS
   Printereinstellungen auch ohne View

      Rev 1.98   11 Jun 1996 16:44:52   OS
   UndoActionCount als Null uebergeben, wenn fuer die akt. Shell DoesUndo() == FALSE, Bug #28570#

      Rev 1.98   11 Jun 1996 16:33:22   OS
   UndoActionCount als Null uebergeben, wenn fuer die akt. Shell DoesUndo() == FALSE, Bug #28570#

      Rev 1.97   10 Jun 1996 13:03:02   OS
   Range fuer Optionendialog fuer neue Proxies erweitert

      Rev 1.96   24 May 1996 14:58:58   OS
   HelpIdas aktualisiert

      Rev 1.95   10 May 1996 11:35:52   NF
   Zeile 76 auskommentiert

      Rev 1.94   09 May 1996 12:38:00   OS
   Range fuer Optionendialog erweitert

      Rev 1.93   30 Apr 1996 14:00:02   OS
   Item fuer Preview schicken, DocDisplayTabPage nicht mehr on demand

      Rev 1.92   27 Apr 1996 17:01:52   OS
   MakeUsrPref benutzen

      Rev 1.91   25 Apr 1996 16:25:28   OS
   ViewOptions-Umbau: alle Aktionen an pVOpt

      Rev 1.90   24 Apr 1996 15:02:50   OS
   Umstellung UsrPref/ViewOption

 -------------------------------------------------------------------------*/


