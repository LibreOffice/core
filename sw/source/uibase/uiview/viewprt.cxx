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

#include <com/sun/star/text/NotePrintMode.hpp>
#include <cstdarg>
#include <cmdid.h>
#include <sfx2/request.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/app.hxx>
#include <vcl/layout.hxx>
#include <vcl/oldprintadaptor.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/prnmon.hxx>
#include <editeng/paperinf.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/misccfg.hxx>
#include <svx/prtqry.hxx>
#include <svx/svdview.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/flagitem.hxx>
#include <sfx2/linkmgr.hxx>

#include <modcfg.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <prtopt.hxx>
#include <fontcfg.hxx>
#include <cfgitems.hxx>
#include <dbmgr.hxx>
#include <docstat.hxx>
#include <viewfunc.hxx>
#include <swmodule.hxx>
#include <wview.hxx>
#include <doc.hxx>
#include <fldbas.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <globals.hrc>
#include <view.hrc>
#include <app.hrc>
#include <swwrtshitem.hxx>
#include "swabstdlg.hxx"
#include <svl/slstitm.hxx>

#include <unomid.h>

using namespace ::com::sun::star;

// Hand over the printer to Sfx

SfxPrinter* SwView::GetPrinter( bool bCreate )
{
    const IDocumentDeviceAccess* pIDDA = GetWrtShell().getIDocumentDeviceAccess();
    SfxPrinter *pOld = pIDDA->getPrinter( false );
    SfxPrinter *pPrt = pIDDA->getPrinter( bCreate );
    if ( pOld != pPrt )
    {
        bool bWeb = 0 != dynamic_cast<SwWebView*>(this);
        ::SetAppPrintOptions( &GetWrtShell(), bWeb );
    }
    return pPrt;
}

// Propagate printer change

void SetPrinter( IDocumentDeviceAccess* pIDDA, SfxPrinter* pNew, bool bWeb )
{
    SwPrintOptions* pOpt = SW_MOD()->GetPrtOptions(bWeb);
    if( !pOpt)
        return;

    // Reading Application own printing options from SfxPrinter
    const SfxItemSet& rSet = pNew->GetOptions();

    const SwAddPrinterItem* pAddPrinterAttr;
    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_ADDPRINTER, false,
        (const SfxPoolItem**)&pAddPrinterAttr ) )
    {
        if( pIDDA )
            pIDDA->setPrintData( *pAddPrinterAttr );
        if( !pAddPrinterAttr->GetFax().isEmpty() )
            pOpt->SetFaxName(pAddPrinterAttr->GetFax());
    }
}

sal_uInt16 SwView::SetPrinter(SfxPrinter* pNew, sal_uInt16 nDiffFlags, bool  )
{
    SwWrtShell &rSh = GetWrtShell();
    SfxPrinter* pOld = rSh.getIDocumentDeviceAccess()->getPrinter( false );
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    if ( (SFX_PRINTER_JOBSETUP | SFX_PRINTER_PRINTER) & nDiffFlags )
    {
        rSh.getIDocumentDeviceAccess()->setPrinter( pNew, true, true );
        if ( nDiffFlags & SFX_PRINTER_PRINTER )
            rSh.SetModified();
    }
    bool bWeb = this->ISA(SwWebView);
    if ( nDiffFlags & SFX_PRINTER_OPTIONS )
        ::SetPrinter( rSh.getIDocumentDeviceAccess(), pNew, bWeb );

    const bool bChgOri = nDiffFlags & SFX_PRINTER_CHG_ORIENTATION;
    const bool bChgSize= nDiffFlags & SFX_PRINTER_CHG_SIZE;
    if ( bChgOri || bChgSize )
    {
        rSh.StartAllAction();
        if ( bChgOri )
            rSh.ChgAllPageOrientation( pNew->GetOrientation() );
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

bool SwView::HasPrintOptionsPage() const
{
    return true;
}

// TabPage for application-specific print options

SfxTabPage* SwView::CreatePrintOptionsPage(vcl::Window* pParent,
                                                    const SfxItemSet& rSet)
{
    return ::CreatePrintOptionsPage( pParent, rSet, false );
}

// Print dispatcher

void SwView::ExecutePrint(SfxRequest& rReq)
{
    bool bWeb = 0 != PTR_CAST(SwWebView, this);
    ::SetAppPrintOptions( &GetWrtShell(), bWeb );
    switch (rReq.GetSlot())
    {
        case FN_FAX:
        {
            SwPrintOptions* pPrintOptions = SW_MOD()->GetPrtOptions(bWeb);
            OUString sFaxName(pPrintOptions->GetFaxName());
            if (!sFaxName.isEmpty())
            {
                SfxStringItem aPrinterName(SID_PRINTER_NAME, sFaxName);
                SfxBoolItem aSilent( SID_SILENT, true );
                GetViewFrame()->GetDispatcher()->Execute( SID_PRINTDOC,
                            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
                            &aPrinterName, &aSilent, 0L );
            }
            else
            {
                MessageDialog aInfoBox(&GetEditWin(), SW_RES(STR_ERR_NO_FAX), VCL_MESSAGE_INFO);
                sal_uInt16 nResNo = bWeb ? STR_WEBOPTIONS : STR_TEXTOPTIONS;
                aInfoBox.set_primary_text(aInfoBox.get_primary_text().replaceFirst("%1", OUString(SW_RES(nResNo))));
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
            SFX_REQUEST_ARG(rReq, pSilentItem, SfxBoolItem, SID_SILENT, false);
            bool bSilent = pSilentItem ? pSilentItem->GetValue() : sal_False;
            SFX_REQUEST_ARG(rReq, pPrintFromMergeItem, SfxBoolItem, FN_QRY_MERGE, false);
            if(pPrintFromMergeItem)
                rReq.RemoveItem(FN_QRY_MERGE);
            bool bFromMerge = pPrintFromMergeItem ? pPrintFromMergeItem->GetValue() : sal_False;
            SwMiscConfig aMiscConfig;
            bool bPrintSelection = false;
            if(!bSilent && !bFromMerge &&
                    SW_MOD()->GetModuleConfig()->IsAskForMailMerge() && pSh->IsAnyDatabaseFieldInDoc())
            {
                MessageDialog aBox(&GetEditWin(), "PrintMergeDialog",
                                   "modules/swriter/ui/printmergedialog.ui");
                short nRet = aBox.Execute();
                if(RET_YES == nRet)
                {
                    SfxBoolItem aBool(FN_QRY_MERGE, true);
                    GetViewFrame()->GetDispatcher()->Execute(
                                FN_QRY_MERGE, SFX_CALLMODE_ASYNCHRON, &aBool, 0L);
                    rReq.Ignore();
                    return;
                }
            }
            else if( rReq.GetSlot() == SID_PRINTDOCDIRECT && ! bSilent )
            {
                if( ( pSh->IsSelection() || pSh->IsFrmSelected() || pSh->IsObjSelected() ) )
                {
                    short nBtn = SvxPrtQryBox(&GetEditWin()).Execute();
                    if( RET_CANCEL == nBtn )
                        return;

                    if( RET_OK == nBtn )
                        bPrintSelection = true;
                }
            }

            //#i61455# if master documentes are printed silently without loaded links then update the links now
            if( bSilent && pSh->IsGlobalDoc() && !pSh->IsGlblDocSaveLinks() )
            {
                pSh->GetLinkManager().UpdateAllLinks( false, false, false, 0 );
            }
            SfxRequest aReq( rReq );
            SfxBoolItem aBool(SID_SELECTION, bPrintSelection);
            aReq.AppendItem( aBool );
            SfxViewShell::ExecuteSlot( aReq, SfxViewShell::GetInterface() );
            return;
        }
        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
}

// Create page printer/additions for SwView and SwPagePreview

SfxTabPage* CreatePrintOptionsPage( vcl::Window *pParent,
                                const SfxItemSet &rOptions, bool bPreview )
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "No Print Dialog");
    if (!pFact)
        return NULL;

    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc(TP_OPTPRINT_PAGE);
    OSL_ENSURE(pFact, "No Page Creator");
    if (!fnCreatePage)
        return NULL;

    SfxTabPage* pPage = (*fnCreatePage)(pParent, &rOptions);
    OSL_ENSURE(pPage, "No page");
    if (!pPage)
        return NULL;

    SfxAllItemSet aSet(*(rOptions.GetPool()));
    aSet.Put(SfxBoolItem(SID_PREVIEWFLAG_TYPE, bPreview));
    aSet.Put(SfxBoolItem(SID_FAX_LIST, true));
    pPage->PageCreated(aSet);
    return pPage;
}

void SetAppPrintOptions( SwViewShell* pSh, bool bWeb )
{
    const IDocumentDeviceAccess* pIDDA = pSh->getIDocumentDeviceAccess();
    SwPrintData aPrtData = pIDDA->getPrintData();

    if( pIDDA->getPrinter( false ) )
    {
        // Close application own printing options in SfxPrinter.
        SwAddPrinterItem aAddPrinterItem (FN_PARAM_ADDPRINTER, aPrtData);
        SfxItemSet aSet( pSh->GetAttrPool(),
                    FN_PARAM_ADDPRINTER,        FN_PARAM_ADDPRINTER,
                    SID_HTML_MODE,              SID_HTML_MODE,
                    SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                    0 );

        utl::MiscCfg aMisc;

        if(bWeb)
            aSet.Put(SfxUInt16Item(SID_HTML_MODE,
                    ::GetHtmlMode(((SwWrtShell*)pSh)->GetView().GetDocShell())));
        aSet.Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN,
                        aMisc.IsNotFoundWarning() ));
        aSet.Put(aAddPrinterItem);
        aSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC,
            (aMisc.IsPaperSizeWarning() ? SFX_PRINTER_CHG_SIZE : 0)   |
            (aMisc.IsPaperOrientationWarning()  ? SFX_PRINTER_CHG_ORIENTATION : 0 )));

        pIDDA->getPrinter( true )->SetOptions( aSet );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
