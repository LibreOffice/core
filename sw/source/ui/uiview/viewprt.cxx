/*************************************************************************
 *
 *  $RCSfile: viewprt.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-28 12:32:55 $
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

#include <com/sun/star/text/NotePrintMode.hpp>

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif


#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_PRINTDLG_HXX_ //autogen
#include <svtools/printdlg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_PRNMON_HXX //autogen
#include <sfx2/prnmon.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_MISCCFG_HXX
#include <sfx2/misccfg.hxx>
#endif
#ifndef _SVX_PRTQRY_HXX
#include <svx/prtqry.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX
#include <svtools/flagitem.hxx>
#endif

#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>
#endif
#ifndef _OPTPAGE_HXX
#include <optpage.hxx>
#endif
#ifndef _CFGITEMS_HXX
#include <cfgitems.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _VIEWFUNC_HXX
#include <viewfunc.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _VIEW_HRC
#include <view.hrc>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif

#define C2U(cChar) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))

/*--------------------------------------------------------------------
    Beschreibung:   Drucker an Sfx uebergeben
 --------------------------------------------------------------------*/


SfxPrinter* __EXPORT SwView::GetPrinter( BOOL bCreate )
{
    SfxPrinter *pOld = GetWrtShell().GetPrt( FALSE );
    SfxPrinter *pPrt = GetWrtShell().GetPrt( bCreate );
    if ( pOld != pPrt )
    {
        BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
        ::SetAppPrintOptions( &GetWrtShell(), bWeb );
    }
    return pPrt;
}

/*--------------------------------------------------------------------
    Beschreibung:   Druckerwechsel weitermelden
 --------------------------------------------------------------------*/

void SetPrinter( SfxPrinter* pNew, BOOL bWeb )
{
    SwPrintOptions* pOpt = SW_MOD()->GetPrtOptions(bWeb);
    if( !pOpt)
        return;

    // Applikationseigene Druckoptionen aus SfxPrinter auslesen
    const SfxItemSet& rSet = pNew->GetOptions();

    const SwAddPrinterItem* pAddPrinterAttr;
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_ADDPRINTER, FALSE,
        (const SfxPoolItem**)&pAddPrinterAttr ) )
    {
//      Wir lassen die Druckoptionen so, wie sie sind.
//      pAddPrinterAttr->SetPrintOptions( pOpt );
        if( pAddPrinterAttr->GetFax().getLength() )
            pOpt->SetFaxName(pAddPrinterAttr->GetFax());
    }
}


USHORT __EXPORT SwView::SetPrinter(SfxPrinter* pNew, USHORT nDiffFlags )
{
    SwWrtShell &rSh = GetWrtShell();
    if ( (SFX_PRINTER_JOBSETUP | SFX_PRINTER_PRINTER) & nDiffFlags )
    {
        rSh.SetPrt( pNew );
        if ( nDiffFlags & SFX_PRINTER_PRINTER )
            rSh.SetModified();
    }
    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    if ( nDiffFlags & SFX_PRINTER_OPTIONS )
        ::SetPrinter( pNew, bWeb );

    const BOOL bChgOri = nDiffFlags & SFX_PRINTER_CHG_ORIENTATION ? TRUE : FALSE;
    const BOOL bChgSize= nDiffFlags & SFX_PRINTER_CHG_SIZE ? TRUE : FALSE;
    if ( bChgOri || bChgSize )
    {
        rSh.StartAllAction();
        if ( bChgOri )
            rSh.ChgAllPageOrientation( USHORT(pNew->GetOrientation()) );
        if ( bChgSize )
        {
            Size aSz( SvxPaperInfo::GetPaperSize( pNew ) );
            rSh.ChgAllPageSize( aSz );
        }
        rSh.SetModified();
        rSh.EndAllAction();
        InvalidateRulerPos();
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

ErrCode SwView::DoPrint( SfxPrinter *pPrinter, PrintDialog *pDlg,
                                     BOOL bSilent )
{
    // First test
    SwWrtShell* pSh = &GetWrtShell();
    SwNewDBMgr* pMgr = pSh->GetNewDBMgr();

    int bPrintSelection = -1;
    if( DBMGR_MERGE_MAILMERGE != pMgr->GetMergeType() && !pDlg && !bSilent
        && !bIsApi && ( pSh->IsSelection() || pSh->IsFrmSelected() ||
        pSh->IsObjSelected() ) )
    {
        short nBtn = SvxPrtQryBox(&GetEditWin()).Execute();
        if( RET_CANCEL == nBtn )
            return ERRCODE_IO_ABORT;

        if( RET_OK == nBtn )
            bPrintSelection = 1;
        else
            bPrintSelection = 0;
    }


//  Der PrintProgress stellt Handler am Printer der ViewShell ein.
//  Das kann natuerlich erste nach dem evtl. Umschalten des Druckers korrekt
//  funktionieren. #55210#
//  SfxPrintProgress *pProgress = new SfxPrintProgress( this, !bSilent );
    SfxPrintProgress *pProgress = 0;
    SfxPrinter *pDocPrinter = GetPrinter(TRUE);
    if ( !pPrinter )
        pPrinter = pDocPrinter;
    else if ( pDocPrinter != pPrinter )
    {
        //Da der Doc-Drucker beim SetPrinter geloescht wird,
        // muss man ihn vorher clonen
        SfxPrinter* pClone = pDocPrinter->Clone();
        SetPrinter( pPrinter, SFX_PRINTER_PRINTER );
        pProgress = new SfxPrintProgress( this, !bSilent );
        pProgress->RestoreOnEndPrint( pClone );
    }
    if(!pProgress)
        pProgress = new SfxPrintProgress( this, !bSilent );
    pProgress->SetWaitMode(FALSE);

    BOOL bStartJob = pPrinter->InitJob( &GetEditWin(), pSh->HasDrawView() &&
                    pSh->GetDrawView()->GetModel()->HasTransparentObjects());
    if( bStartJob )
    {
        // Drucker starten
        PreparePrint( pDlg );
        SfxObjectShell *pObjShell = GetViewFrame()->GetObjectShell();
        SwPrtOptions aOpts( pObjShell->GetTitle(0) );
        BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
        if( pMgr->GetMergeType() == DBMGR_MERGE_MAILMERGE )
        {
            SwView::MakeOptions( pDlg, aOpts, 0, bWeb, GetPrinter(),
                            pSh->GetPrintData() );
            bStartJob = pMgr->MergePrint( *this, aOpts, *pProgress );
        }
        else
        {
            const BOOL bLockedView = pSh->IsViewLocked();
            pSh->LockView( TRUE );

            //BrowseView abschalten und die View gegen alle Paints locken.
            FASTBOOL bBrowse = pSh->IsBrowseMode();
            SfxAllItemSet aSet( SFX_APP()->GetPool() );
            SfxBoolItem aBrowse( SID_BROWSER_MODE, FALSE );
            if ( bBrowse )
            {
                if ( pSh->GetWin() )
                    pSh->GetWin()->Update();
                pSh->LockPaint();
                aSet.Put( aBrowse, aBrowse.Which() );
                SfxRequest aReq( SID_BROWSER_MODE, 0, aSet );
                GetDocShell()->Execute( aReq );
            }

            // die Felder aktualisieren
            BOOL bIsModified = pSh->IsModified();
            pSh->StartAllAction();
            SwDocStat aDocStat;
            pSh->UpdateDocStat( aDocStat );
            pSh->EndAllTblBoxEdit();
            pSh->ViewShell::UpdateFlds(TRUE);
            if( pSh->IsCrsrInTbl() )
            {
                pSh->ClearTblBoxCntnt();
                pSh->SaveTblBoxCntnt();
            }
            pSh->EndAllAction();

            if( !bIsModified )
                pSh->ResetModified();

            BOOL bPrtPros;
            SwView::MakeOptions( pDlg, aOpts, &bPrtPros, bWeb, GetPrinter(),
                            pSh->GetPrintData() );
            if( -1 != bPrintSelection )
                aOpts.bPrintSelection = 0 != bPrintSelection;

            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue> aViewProperties(15);
            com::sun::star::beans::PropertyValue* pViewProperties =  aViewProperties.getArray();
            pViewProperties[1].Name = C2U("PrintGraphics");
            pViewProperties[1].Value <<= (sal_Bool)aOpts.IsPrintGraphic();
            pViewProperties[2].Name = C2U("PrintTables");
            pViewProperties[2].Value <<= (sal_Bool)aOpts.IsPrintTable();
            pViewProperties[3].Name = C2U("PrintDrawings");
            pViewProperties[3].Value <<= (sal_Bool)aOpts.IsPrintDraw();
            pViewProperties[4].Name = C2U("PrintLeftPages");
            pViewProperties[4].Value <<= (sal_Bool)aOpts.IsPrintLeftPage();
            pViewProperties[5].Name = C2U("PrintRightPages");
            pViewProperties[5].Value <<= (sal_Bool)aOpts.IsPrintRightPage();
            pViewProperties[6].Name = C2U("PrintControls");
            pViewProperties[6].Value <<= (sal_Bool)aOpts.IsPrintControl();
            pViewProperties[7].Name = C2U("PrintReversed");
            pViewProperties[7].Value <<= (sal_Bool)aOpts.IsPrintReverse();
            pViewProperties[8].Name = C2U("PrintPaperFromSetup");
            pViewProperties[8].Value <<= (sal_Bool)aOpts.IsPaperFromSetup();
            pViewProperties[9].Name = C2U("PrintFaxName");
            pViewProperties[9].Value <<= aOpts.GetFaxName();
            pViewProperties[10].Name = C2U("PrintAnnotationMode");
            pViewProperties[10].Value <<= (::com::sun::star::text::NotePrintMode) aOpts.GetPrintPostIts();
            pViewProperties[11].Name = C2U("PrintProspect");
            pViewProperties[11].Value <<= (sal_Bool)aOpts.IsPrintProspect();
            pViewProperties[12].Name = C2U("PrintPageBackground");
            pViewProperties[12].Value <<= (sal_Bool)aOpts.IsPrintPageBackground();
            pViewProperties[13].Name = C2U("PrintBlackFonts");
            pViewProperties[13].Value <<= (sal_Bool)aOpts.IsPrintBlackFont();
            pViewProperties[0].Name = C2U("IsSinglePrintJob");
            pViewProperties[0].Value <<= (sal_Bool)aOpts.IsPrintSingleJobs();
            pViewProperties[14].Name = C2U("Selection");
            pViewProperties[14].Value <<= (sal_Bool)aOpts.bPrintSelection;
            SetAdditionalPrintOptions(aViewProperties);

            SfxViewShell::Print(*pProgress);
            if ( !pProgress->IsAborted() )
            {
                if( bPrtPros )
                {
                    bStartJob = pPrinter->StartJob( aOpts.GetJobName() );
                    if( bStartJob )
                        pSh->PrintProspect( aOpts, *pProgress );
                }
                else
                    bStartJob = pSh->Prt( aOpts, *pProgress );

                if ( bBrowse )
                {
                    aBrowse.SetValue( TRUE );
                    aSet.Put( aBrowse, aBrowse.Which() );
                    SfxRequest aReq( SID_BROWSER_MODE, 0, aSet );
                    GetDocShell()->Execute( aReq );
                    pSh->UnlockPaint();
                }
            }
            else
                bStartJob = FALSE;

            pSh->LockView( bLockedView );
        }
    }

    bIsApi = FALSE;
    if( !bStartJob )
    {
        // Printer konnte nicht gestartet werden
        delete pProgress;
        return pPrinter->GetError();
    }

    pProgress->Stop();
    pProgress->DeleteOnEndPrint();
    pPrinter->EndJob();
    return pPrinter->GetError();
}



/*--------------------------------------------------------------------
    Beschreibung:   TabPage fuer applikationsspezifische Druckoptionen
 --------------------------------------------------------------------*/

SfxTabPage* __EXPORT SwView::CreatePrintOptionsPage(Window* pParent,
                                                    const SfxItemSet& rSet)
{
    return ::CreatePrintOptionsPage( pParent, rSet, FALSE );
}

/*--------------------------------------------------------------------
    Beschreibung:   Druckerdialog
 --------------------------------------------------------------------*/

PrintDialog* CreatePrintDialog( Window* pParent, USHORT nPg, SwWrtShell* pSh )
{
    PrintDialog *pDlg =
        new PrintDialog( pParent );
//  pDlg->ChangeFirstPage( 1 );

    if ( !nPg )
        nPg = 1;
//  pDlg->ChangeLastPage( nPg );
//  pDlg->ChangeMaxPage( 9999 );
    pDlg->EnableRange( PRINTDIALOG_FROMTO );

    if (pSh && (pSh->IsSelection() || pSh->IsFrmSelected() || pSh->IsObjSelected()))
        pDlg->EnableRange( PRINTDIALOG_SELECTION );

    pDlg->SetRangeText( String::CreateFromInt32(nPg) );
    pDlg->EnableRange( PRINTDIALOG_RANGE );
    pDlg->EnableCollate();
    return pDlg;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

PrintDialog* __EXPORT SwView::CreatePrintDialog( Window* pParent )
{
    // AMA: Hier sollte vielleicht die virtuelle Seitennummer angezeigt werden,
    //      aber nur, wenn das Drucken virtuelle Seitennummern und nicht wie
    //      bisher (auch beim SWG 2.0) physikalische beachtet werden.
    return ::CreatePrintDialog( pParent, GetWrtShell().GetPhyPageNum(), &GetWrtShell() );
}

/*--------------------------------------------------------------------
    Beschreibung:   Print-Dispatcher
 --------------------------------------------------------------------*/

void __EXPORT SwView::ExecutePrint(SfxRequest& rReq)
{
    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    ::SetAppPrintOptions( &GetWrtShell(), bWeb );
    switch (rReq.GetSlot())
    {
        case FN_FAX:
        {
            SwPrintOptions* pOpt = SW_MOD()->GetPrtOptions(bWeb);
            String sFaxName(pOpt->GetFaxName());
            if (sFaxName.Len())
            {
                SfxStringItem aPrinterName(SID_PRINTER_NAME, sFaxName);
                SfxBoolItem aSilent( SID_SILENT, TRUE );
                GetViewFrame()->GetDispatcher()->Execute( SID_PRINTDOC,
                            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
                            &aPrinterName, &aSilent, 0L );
            }
            else
            {
                InfoBox aInfoBox(&GetEditWin(), SW_RES(MSG_ERR_NO_FAX));
                String sMsg = aInfoBox.GetMessText();
                USHORT nResNo = bWeb ? STR_WEBOPTIONS : STR_TEXTOPTIONS;
                sMsg.SearchAndReplace(String::CreateFromAscii("%1"), String(SW_RES(nResNo)));
                aInfoBox.SetMessText(sMsg);
                aInfoBox.Execute();
                SfxUInt16Item aDefPage(SID_SW_EDITOPTIONS, TP_OPTPRINT_PAGE);
                GetViewFrame()->GetDispatcher()->Execute(SID_SW_EDITOPTIONS,
                            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
                            &aDefPage, 0L );
            }
        }
        break;
        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
        {
            SwWrtShell* pSh = &GetWrtShell();
            SwViewOption* pOrgViewOption = 0;
            SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, FALSE);
            BOOL bSilent = pSilentItem ? pSilentItem->GetValue() : FALSE;
            SFX_REQUEST_ARG(rReq, pPrintFromMergeItem, SfxBoolItem, FN_QRY_MERGE, FALSE);
            if(pPrintFromMergeItem)
                rReq.RemoveItem(FN_QRY_MERGE);
            BOOL bFromMerge = pPrintFromMergeItem ? pPrintFromMergeItem->GetValue() : FALSE;
            if(!bSilent && !bFromMerge && pSh->IsAnyDatabaseFieldInDoc())
            {
                QueryBox aBox( &GetEditWin(), SW_RES( MSG_PRINT_AS_MERGE ));
                if(RET_YES == aBox.Execute())
                {
                    SfxBoolItem aBool(FN_QRY_MERGE, TRUE);
                    GetViewFrame()->GetDispatcher()->Execute(
                                FN_QRY_MERGE, SFX_CALLMODE_ASYNCHRON, &aBool, 0L);
                    rReq.Ignore();
                    return;
                }
            }
            if(!bSilent && pSh->GetViewOptions()->IsFldName() && pSh->IsAnyFieldInDoc())
            {
                QueryBox aBox( &GetEditWin(), SW_RES( DLG_PRT_FIELDNAME ) );
                USHORT nRet = aBox.Execute();
                if( RET_CANCEL == nRet)
                    return;
                // disable field commands
                if( RET_NO == nRet )
                {
                    pOrgViewOption = new SwViewOption(*pSh->GetViewOptions());
                    pOrgViewOption->SetFldName(FALSE);
                    SW_MOD()->ApplyUsrPref(*pOrgViewOption, this, VIEWOPT_DEST_VIEW_ONLY );
                }
            }
            bIsApi = rReq.IsAPI();
            SfxViewShell::ExecuteSlot( rReq, SfxViewShell::GetInterface() );
            if(pOrgViewOption)
            {
                pOrgViewOption->SetFldName(TRUE);
                SW_MOD()->ApplyUsrPref(*pOrgViewOption, this, VIEWOPT_DEST_VIEW_ONLY );
                delete pOrgViewOption;
            }
            return;
        }
        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Page Drucker/Zusaetze erzeugen fuer SwView und
                    SwPagePreview
 --------------------------------------------------------------------*/

SfxTabPage* CreatePrintOptionsPage( Window *pParent,
                                const SfxItemSet &rOptions, BOOL bPreview )
{
    SwAddPrinterTabPage* pPage = ( SwAddPrinterTabPage* )
                            SwAddPrinterTabPage::Create(pParent, rOptions);
    pPage->SetPreview(bPreview);

    SvStringsDtor aFaxList;
    const USHORT nCount = Printer::GetQueueCount();
    pPage->Reset(rOptions);
    for (USHORT i = 0; i < nCount; ++i)
    {
        String* pString = new String( Printer::GetQueueInfo( i ).GetPrinterName() );
        aFaxList.Insert(pString, 0);
    }
    pPage->SetFax( aFaxList );
    return pPage;
}


void SetAppPrintOptions( ViewShell* pSh, BOOL bWeb )
{
    SwPrintData aPrtData = *SW_MOD()->GetPrtOptions(bWeb);
    SwPrintData* pShellPrintData = pSh->GetPrintData();
    if(pShellPrintData)
        aPrtData = *pShellPrintData;
    if( pSh && pSh->GetPrt())
    {
        // Applikationseigene Druckoptionen in SfxPrinter schiessen
        SwAddPrinterItem aAddPrinterItem (FN_PARAM_ADDPRINTER, aPrtData);
        SfxItemSet aSet( pSh->GetAttrPool(),
                    FN_PARAM_ADDPRINTER,        FN_PARAM_ADDPRINTER,
                    SID_HTML_MODE,              SID_HTML_MODE,
                    SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                    0 );

            SfxMiscCfg* pMisc = SFX_APP()->GetMiscConfig();

        if(bWeb)
            aSet.Put(SfxUInt16Item(SID_HTML_MODE,
                    ::GetHtmlMode(((SwWrtShell*)pSh)->GetView().GetDocShell())));
        aSet.Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN,
                        pMisc->IsNotFoundWarning() ));
        aSet.Put(aAddPrinterItem);
        aSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC,
            (pMisc->IsPaperSizeWarning() ? SFX_PRINTER_CHG_SIZE : 0)   |
            (pMisc->IsPaperOrientationWarning()  ? SFX_PRINTER_CHG_ORIENTATION : 0 )));

        pSh->GetPrt()->SetOptions( aSet );
    }

}

