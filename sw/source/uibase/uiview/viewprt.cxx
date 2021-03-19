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

#include <libxml/xmlwriter.h>
#include <cmdid.h>
#include <officecfg/Office/Common.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/weld.hxx>
#include <sfx2/printer.hxx>
#include <editeng/paperinf.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/flagitem.hxx>
#include <sfx2/linkmgr.hxx>

#include <modcfg.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <prtopt.hxx>
#include <cfgitems.hxx>
#include "viewfunc.hxx"
#include <swmodule.hxx>
#include <wview.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <globals.hrc>
#include <strings.hrc>
#include <swabstdlg.hxx>

#include <uivwimp.hxx>

using namespace ::com::sun::star;

// Hand over the printer to Sfx

SfxPrinter* SwView::GetPrinter( bool bCreate )
{
    const IDocumentDeviceAccess& rIDDA = GetWrtShell().getIDocumentDeviceAccess();
    SfxPrinter *pOld = rIDDA.getPrinter( false );
    SfxPrinter *pPrt = rIDDA.getPrinter( bCreate );
    if ( pOld != pPrt )
    {
        bool bWeb = dynamic_cast<SwWebView*>(this) !=  nullptr;
        ::SetAppPrintOptions( &GetWrtShell(), bWeb );
    }
    return pPrt;
}

// Propagate printer change

void SetPrinter( IDocumentDeviceAccess* pIDDA, SfxPrinter const * pNew, bool bWeb )
{
    SwPrintOptions* pOpt = SW_MOD()->GetPrtOptions(bWeb);
    if( !pOpt)
        return;

    // Reading Application own printing options from SfxPrinter
    const SfxItemSet& rSet = pNew->GetOptions();

    const SwAddPrinterItem* pAddPrinterAttr;
    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_ADDPRINTER, false,
        reinterpret_cast<const SfxPoolItem**>(&pAddPrinterAttr) ) )
    {
        if( pIDDA )
            pIDDA->setPrintData( *pAddPrinterAttr );
        if( !pAddPrinterAttr->GetFaxName().isEmpty() )
            pOpt->SetFaxName(pAddPrinterAttr->GetFaxName());
    }
}

sal_uInt16 SwView::SetPrinter(SfxPrinter* pNew, SfxPrinterChangeFlags nDiffFlags  )
{
    SwWrtShell &rSh = GetWrtShell();
    SfxPrinter* pOld = rSh.getIDocumentDeviceAccess().getPrinter( false );
    if ( pOld && pOld->IsPrinting() )
        return SFX_PRINTERROR_BUSY;

    if ( (SfxPrinterChangeFlags::JOBSETUP | SfxPrinterChangeFlags::PRINTER) & nDiffFlags )
    {
        rSh.getIDocumentDeviceAccess().setPrinter( pNew, true, true );
        if ( nDiffFlags & SfxPrinterChangeFlags::PRINTER )
            rSh.SetModified();
    }
    bool bWeb = dynamic_cast< const SwWebView *>( this ) !=  nullptr;
    if ( nDiffFlags & SfxPrinterChangeFlags::OPTIONS )
        ::SetPrinter( &rSh.getIDocumentDeviceAccess(), pNew, bWeb );

    const bool bChgOri  = bool(nDiffFlags & SfxPrinterChangeFlags::CHG_ORIENTATION);
    const bool bChgSize = bool(nDiffFlags & SfxPrinterChangeFlags::CHG_SIZE);
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

namespace
{
    class SvxPrtQryBox
    {
    private:
        std::unique_ptr<weld::MessageDialog> m_xQueryBox;
    public:
        SvxPrtQryBox(weld::Window* pParent)
            : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Question, VclButtonsType::NONE, SvxResId(RID_SVXSTR_QRY_PRINT_MSG)))
        {
            m_xQueryBox->set_title(SvxResId(RID_SVXSTR_QRY_PRINT_TITLE));

            m_xQueryBox->add_button(SvxResId(RID_SVXSTR_QRY_PRINT_SELECTION), RET_OK);
            m_xQueryBox->add_button(SvxResId(RID_SVXSTR_QRY_PRINT_ALL), 2);
            m_xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
            m_xQueryBox->set_default_response(RET_OK);
        }
        short run() { return m_xQueryBox->run(); }
    };
}

// TabPage for application-specific print options

std::unique_ptr<SfxTabPage> SwView::CreatePrintOptionsPage(weld::Container* pPage, weld::DialogController* pController,
                                                  const SfxItemSet& rSet)
{
    return ::CreatePrintOptionsPage(pPage, pController, rSet, false);
}

// Print dispatcher

void SwView::ExecutePrint(SfxRequest& rReq)
{
    bool bWeb = dynamic_cast<SwWebView*>( this ) !=  nullptr;
    ::SetAppPrintOptions( &GetWrtShell(), bWeb );
    switch (rReq.GetSlot())
    {
        case FN_FAX:
        {
            SwPrintOptions* pPrintOptions = SW_MOD()->GetPrtOptions(bWeb);
            const OUString& sFaxName(pPrintOptions->GetFaxName());
            if (!sFaxName.isEmpty())
            {
                SfxStringItem aPrinterName(SID_PRINTER_NAME, sFaxName);
                SfxBoolItem aSilent( SID_SILENT, true );
                GetViewFrame()->GetDispatcher()->ExecuteList(SID_PRINTDOC,
                            SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
                            { &aPrinterName, &aSilent });
            }
            else
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetEditWin().GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SwResId(STR_ERR_NO_FAX)));
                const char* pResId = bWeb ? STR_WEBOPTIONS : STR_TEXTOPTIONS;
                xInfoBox->set_primary_text(xInfoBox->get_primary_text().replaceFirst("%1", SwResId(pResId)));
                xInfoBox->run();
                SfxUInt16Item aDefPage(SID_SW_EDITOPTIONS, TP_OPTPRINT_PAGE);
                GetViewFrame()->GetDispatcher()->ExecuteList(SID_SW_EDITOPTIONS,
                            SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
                            { &aDefPage });
            }
        }
        break;
        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
        {
            SwWrtShell* pSh = &GetWrtShell();
            const SfxBoolItem* pSilentItem = rReq.GetArg<SfxBoolItem>(SID_SILENT);
            bool bSilent = pSilentItem && pSilentItem->GetValue();
            const SfxBoolItem* pPrintFromMergeItem = rReq.GetArg<SfxBoolItem>(FN_QRY_MERGE);
            if(pPrintFromMergeItem)
                rReq.RemoveItem(FN_QRY_MERGE);
            bool bFromMerge = pPrintFromMergeItem && pPrintFromMergeItem->GetValue();
            bool bPrintSelection = false;
            if(!bSilent && !bFromMerge &&
                    SW_MOD()->GetModuleConfig()->IsAskForMailMerge() && pSh->IsAnyDatabaseFieldInDoc())
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetEditWin().GetFrameWeld(), "modules/swriter/ui/printmergedialog.ui"));
                std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("PrintMergeDialog"));
                short nRet = xBox->run();
                if(RET_NO != nRet)
                {
                    if(RET_YES == nRet)
                    {
                        SfxBoolItem aBool(FN_QRY_MERGE, true);
                        GetViewFrame()->GetDispatcher()->ExecuteList(
                                    FN_QRY_MERGE, SfxCallMode::ASYNCHRON,
                                    { &aBool });
                        rReq.Ignore();
                    }
                    return;
                }
            }
            else if( rReq.GetSlot() == SID_PRINTDOCDIRECT && ! bSilent )
            {
                if( pSh->IsSelection() || pSh->IsFrameSelected() || pSh->IsObjSelected() )
                {
                    SvxPrtQryBox aBox(GetEditWin().GetFrameWeld());
                    short nBtn = aBox.run();
                    if( RET_CANCEL == nBtn )
                        return;

                    if( RET_OK == nBtn )
                        bPrintSelection = true;
                }
            }

            //#i61455# if master documents are printed silently without loaded links then update the links now
            if( bSilent && pSh->IsGlobalDoc() && !pSh->IsGlblDocSaveLinks() )
            {
                pSh->GetLinkManager().UpdateAllLinks( false, false, nullptr );
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

int SwView::getPart() const
{
    return 0;
}

void SwView::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwView"));
    SfxViewShell::dumpAsXml(pWriter);
    if (m_pWrtShell)
        m_pWrtShell->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

void SwView::SetRedlineAuthor(const OUString& rAuthor)
{
    m_pViewImpl->m_sRedlineAuthor = rAuthor;
}

const OUString& SwView::GetRedlineAuthor() const
{
    return m_pViewImpl->m_sRedlineAuthor;
}

void SwView::NotifyCursor(SfxViewShell* pViewShell) const
{
    m_pWrtShell->NotifyCursor(pViewShell);
}

// Create page printer/additions for SwView and SwPagePreview

std::unique_ptr<SfxTabPage> CreatePrintOptionsPage(weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet &rOptions,
                                          bool bPreview)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();

    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc(TP_OPTPRINT_PAGE);
    OSL_ENSURE(pFact, "No Page Creator");
    if (!fnCreatePage)
        return nullptr;

    std::unique_ptr<SfxTabPage> xSfxPage = fnCreatePage(pPage, pController, &rOptions);
    OSL_ENSURE(xSfxPage, "No page");
    if (!xSfxPage)
        return nullptr;

    SfxAllItemSet aSet(*(rOptions.GetPool()));
    aSet.Put(SfxBoolItem(SID_PREVIEWFLAG_TYPE, bPreview));
    aSet.Put(SfxBoolItem(SID_FAX_LIST, true));
    xSfxPage->PageCreated(aSet);
    return xSfxPage;
}

void SetAppPrintOptions( SwViewShell* pSh, bool bWeb )
{
    const IDocumentDeviceAccess& rIDDA = pSh->getIDocumentDeviceAccess();
    const SwPrintData& aPrtData = rIDDA.getPrintData();

    if( !rIDDA.getPrinter( false ) )
        return;

    // Close application own printing options in SfxPrinter.
    SwAddPrinterItem aAddPrinterItem(aPrtData);
    SfxItemSet aSet(
        pSh->GetAttrPool(),
        svl::Items<
            SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
            SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
            SID_HTML_MODE, SID_HTML_MODE,
            FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER>{});

    if(bWeb)
        aSet.Put(SfxUInt16Item(SID_HTML_MODE,
                ::GetHtmlMode(static_cast<SwWrtShell*>(pSh)->GetView().GetDocShell())));
    aSet.Put(SfxBoolItem(SID_PRINTER_NOTFOUND_WARN,
                    officecfg::Office::Common::Print::Warning::NotFound::get() ));
    aSet.Put(aAddPrinterItem);
    aSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC,
        static_cast<int>(officecfg::Office::Common::Print::Warning::PaperSize::get()
            ? SfxPrinterChangeFlags::CHG_SIZE : SfxPrinterChangeFlags::NONE)   |
        static_cast<int>(officecfg::Office::Common::Print::Warning::PaperOrientation::get()
            ? SfxPrinterChangeFlags::CHG_ORIENTATION : SfxPrinterChangeFlags::NONE )));

    rIDDA.getPrinter( true )->SetOptions( aSet );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
