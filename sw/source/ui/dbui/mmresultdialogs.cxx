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

#include <mmresultdialogs.hxx>
#include <mmconfigitem.hxx>
#include <mailconfigpage.hxx>
#include "mmgreetingspage.hxx"
#include <printdata.hxx>
#include <swmessdialog.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <hintids.hxx>
#include <swmodule.hxx>

#include <vcl/QueueInfo.hxx>
#include <editeng/langitem.hxx>
#include <o3tl/temporary.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/scheduler.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <vcl/print.hxx>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>

#include <unotools/tempfile.hxx>
#include <osl/file.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <dbmgr.hxx>
#include <swunohelper.hxx>
#include <shellio.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/event.hxx>
#include <swevent.hxx>
#include <dbui.hxx>
#include <dbui.hrc>
#include <doc.hxx>
#include <sfx2/app.hxx>
#include <strings.hrc>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/string.hxx>
#include <iodetect.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define MM_DOCTYPE_OOO              1
#define MM_DOCTYPE_PDF              2
#define MM_DOCTYPE_WORD             3
#define MM_DOCTYPE_HTML             4
#define MM_DOCTYPE_TEXT             5

static OUString lcl_GetExtensionForDocType(sal_uLong nDocType)
{
    OUString sExtension;
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO : sExtension = "odt"; break;
        case MM_DOCTYPE_PDF : sExtension = "pdf"; break;
        case MM_DOCTYPE_WORD: sExtension = "doc"; break;
        case MM_DOCTYPE_HTML: sExtension = "html"; break;
        case MM_DOCTYPE_TEXT: sExtension = "txt"; break;
    }
    return sExtension;
}

static OUString lcl_GetColumnValueOf(const OUString& rColumn, Reference < container::XNameAccess> const & rxColAccess )
{
    OUString sRet;
    try
    {
        if (rxColAccess->hasByName(rColumn))
        {
            Any aCol = rxColAccess->getByName(rColumn);
            Reference< sdb::XColumn > xColumn;
            aCol >>= xColumn;
            if(xColumn.is())
                sRet = xColumn->getString();
        }
    }
    catch (const uno::Exception&)
    {
    }
    return sRet;
}

/**
 * Replace email server settings in rConfigItem with those set in Writer's global
 * mail merge config settings.
 */
static void lcl_UpdateEmailSettingsFromGlobalConfig(SwMailMergeConfigItem& rConfigItem)
{
    // newly created SwMailMergeConfigItem is initialized with values from (global) config
    SwMailMergeConfigItem aConfigItem;

    // take over email-related settings
    rConfigItem.SetMailDisplayName(aConfigItem.GetMailDisplayName());
    rConfigItem.SetMailAddress(aConfigItem.GetMailAddress());
    rConfigItem.SetMailReplyTo(aConfigItem.GetMailReplyTo());
    rConfigItem.SetMailReplyTo(aConfigItem.IsMailReplyTo());
    rConfigItem.SetMailServer(aConfigItem.GetMailServer());
    rConfigItem.SetMailPort(aConfigItem.GetMailPort());
    rConfigItem.SetSecureConnection(aConfigItem.IsSecureConnection());
    // authentication settings
    rConfigItem.SetAuthentication(aConfigItem.IsAuthentication());
    rConfigItem.SetSMTPAfterPOP(aConfigItem.IsSMTPAfterPOP());
    rConfigItem.SetMailUserName(aConfigItem.GetMailUserName());
    rConfigItem.SetMailPassword(aConfigItem.GetMailPassword());
    rConfigItem.SetInServerName(aConfigItem.GetInServerName());
    rConfigItem.SetInServerPort(aConfigItem.GetInServerPort());
    rConfigItem.SetInServerPOP(aConfigItem.IsInServerPOP());
    rConfigItem.SetInServerUserName(aConfigItem.GetInServerUserName());
    rConfigItem.SetInServerPassword(aConfigItem.GetInServerPassword());
}

namespace {

class SwSaveWarningBox_Impl : public SwMessageAndEditDialog
{
    DECL_LINK( ModifyHdl, weld::Entry&, void);
public:
    SwSaveWarningBox_Impl(weld::Window* pParent, const OUString& rFileName);

    OUString        GetFileName() const
    {
        return m_xEdit->get_text();
    }
};

class SwSendQueryBox_Impl : public SwMessageAndEditDialog
{
    bool            m_bIsEmptyAllowed;
    DECL_LINK( ModifyHdl, weld::Entry&, void);
public:
    SwSendQueryBox_Impl(weld::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription);

    void SetValue(const OUString& rSet)
    {
        m_xEdit->set_text(rSet);
        ModifyHdl(*m_xEdit);
    }

    OUString GetValue() const
    {
        return m_xEdit->get_text();
    }

    void SetIsEmptyTextAllowed(bool bSet)
    {
        m_bIsEmptyAllowed = bSet;
        ModifyHdl(*m_xEdit);
    }
};

}

SwSaveWarningBox_Impl::SwSaveWarningBox_Impl(weld::Window* pParent, const OUString& rFileName)
    : SwMessageAndEditDialog(pParent, u"AlreadyExistsDialog"_ustr,
        u"modules/swriter/ui/alreadyexistsdialog.ui"_ustr)
{
    m_xEdit->set_text(rFileName);
    m_xEdit->connect_changed(LINK(this, SwSaveWarningBox_Impl, ModifyHdl));

    INetURLObject aTmp(rFileName);
    m_xDialog->set_primary_text(m_xDialog->get_primary_text().replaceAll("%1", aTmp.getName(
            INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset)));

    ModifyHdl(*m_xEdit);
}

IMPL_LINK( SwSaveWarningBox_Impl, ModifyHdl, weld::Entry&, rEdit, void)
{
    m_xOKPB->set_sensitive(!rEdit.get_text().isEmpty());
}

SwSendQueryBox_Impl::SwSendQueryBox_Impl(weld::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription)
    : SwMessageAndEditDialog(pParent, rID, rUIXMLDescription)
    , m_bIsEmptyAllowed(true)
{
    m_xEdit->connect_changed(LINK(this, SwSendQueryBox_Impl, ModifyHdl));
    ModifyHdl(*m_xEdit);
}

IMPL_LINK( SwSendQueryBox_Impl, ModifyHdl, weld::Entry&, rEdit, void)
{
    m_xOKPB->set_sensitive(m_bIsEmptyAllowed  || !rEdit.get_text().isEmpty());
}

namespace {

class SwCopyToDialog : public SfxDialogController
{
    std::unique_ptr<weld::Entry> m_xCCED;
    std::unique_ptr<weld::Entry> m_xBCCED;

public:
    explicit SwCopyToDialog(weld::Window* pParent)
        : SfxDialogController(pParent, u"modules/swriter/ui/ccdialog.ui"_ustr, u"CCDialog"_ustr)
        , m_xCCED(m_xBuilder->weld_entry(u"cc"_ustr))
        , m_xBCCED(m_xBuilder->weld_entry(u"bcc"_ustr))
    {
    }

    OUString GetCC() const {return m_xCCED->get_text();}
    void SetCC(const OUString& rSet) {m_xCCED->set_text(rSet);}

    OUString GetBCC() const {return m_xBCCED->get_text();}
    void SetBCC(const OUString& rSet) {m_xBCCED->set_text(rSet);}
};

}

SwMMResultSaveDialog::SwMMResultSaveDialog(weld::Window* pParent)
    : SfxDialogController(pParent, u"modules/swriter/ui/mmresultsavedialog.ui"_ustr, u"MMResultSaveDialog"_ustr)
    , m_bCancelSaving(false)
    , m_xSaveAsOneRB(m_xBuilder->weld_radio_button(u"singlerb"_ustr))
    , m_xSaveIndividualRB(m_xBuilder->weld_radio_button(u"individualrb"_ustr))
    , m_xFromRB(m_xBuilder->weld_check_button(u"fromrb"_ustr))
    , m_xFromNF(m_xBuilder->weld_spin_button(u"from"_ustr))
    , m_xToFT(m_xBuilder->weld_label(u"toft"_ustr))
    , m_xToNF(m_xBuilder->weld_spin_button(u"to"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
{
    Link<weld::Toggleable&,void> aLink = LINK(this, SwMMResultSaveDialog, DocumentSelectionHdl_Impl);
    m_xSaveAsOneRB->connect_toggled(aLink);
    m_xSaveIndividualRB->connect_toggled(aLink);
    m_xFromRB->connect_toggled(aLink);
    // m_pSaveAsOneRB is the default, so disable m_xFromNF and m_xToNF initially.
    aLink.Call(*m_xSaveAsOneRB);
    if (SwView* pView = GetActiveView())
    {
        const std::shared_ptr<SwMailMergeConfigItem>& xConfigItem = pView->GetMailMergeConfigItem();
        assert(xConfigItem);
        sal_Int32 nCount = xConfigItem->GetMergedDocumentCount();
        m_xFromNF->set_max(nCount);
        m_xToNF->set_max(nCount);
        m_xToNF->set_value(nCount);
    }

    m_xOKButton->connect_clicked(LINK(this, SwMMResultSaveDialog, SaveOutputHdl_Impl));
}

SwMMResultSaveDialog::~SwMMResultSaveDialog()
{
}

SwMMResultPrintDialog::SwMMResultPrintDialog(weld::Window* pParent)
    : SfxDialogController(pParent, u"modules/swriter/ui/mmresultprintdialog.ui"_ustr, u"MMResultPrintDialog"_ustr)
    , m_xPrinterLB(m_xBuilder->weld_combo_box(u"printers"_ustr))
    , m_xPrinterSettingsPB(m_xBuilder->weld_button(u"printersettings"_ustr))
    , m_xPrintAllRB(m_xBuilder->weld_radio_button(u"printallrb"_ustr))
    , m_xFromRB(m_xBuilder->weld_radio_button(u"fromrb"_ustr))
    , m_xFromNF(m_xBuilder->weld_spin_button(u"from"_ustr))
    , m_xToFT(m_xBuilder->weld_label(u"toft"_ustr))
    , m_xToNF(m_xBuilder->weld_spin_button(u"to"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xPrinterLB->make_sorted();

    m_xPrinterLB->connect_changed(LINK(this, SwMMResultPrintDialog, PrinterChangeHdl_Impl));
    m_xPrinterSettingsPB->connect_clicked(LINK(this, SwMMResultPrintDialog, PrinterSetupHdl_Impl));

    Link<weld::Toggleable&,void> aLink = LINK(this, SwMMResultPrintDialog, DocumentSelectionHdl_Impl);
    m_xPrintAllRB->connect_toggled(aLink);
    m_xFromRB->connect_toggled(aLink);
    // m_pPrintAllRB is the default, so disable m_xFromNF and m_xToNF initially.
    aLink.Call(*m_xPrintAllRB);

    m_xOKButton->connect_clicked(LINK(this, SwMMResultPrintDialog, PrintHdl_Impl));

    FillInPrinterSettings();
}

SwMMResultPrintDialog::~SwMMResultPrintDialog()
{
}

SwMMResultEmailDialog::SwMMResultEmailDialog(weld::Window* pParent)
    : SfxDialogController(pParent, u"modules/swriter/ui/mmresultemaildialog.ui"_ustr, u"MMResultEmailDialog"_ustr)
    , m_sConfigureMail(SwResId(ST_CONFIGUREMAIL))
    , m_xMailToLB(m_xBuilder->weld_combo_box(u"mailto"_ustr))
    , m_xCopyToPB(m_xBuilder->weld_button(u"copyto"_ustr))
    , m_xSubjectED(m_xBuilder->weld_entry(u"subject"_ustr))
    , m_xSendAsLB(m_xBuilder->weld_combo_box(u"sendas"_ustr))
    , m_xSendAsPB(m_xBuilder->weld_button(u"sendassettings"_ustr))
    , m_xAttachmentGroup(m_xBuilder->weld_widget(u"attachgroup"_ustr))
    , m_xAttachmentED(m_xBuilder->weld_entry(u"attach"_ustr))
    , m_xPasswordFT(m_xBuilder->weld_label(u"passwordft"_ustr))
    , m_xPasswordLB(m_xBuilder->weld_combo_box(u"password"_ustr))
    , m_xPasswordCB(m_xBuilder->weld_check_button(u"passwordcb"_ustr))
    , m_xSendAllRB(m_xBuilder->weld_radio_button(u"sendallrb"_ustr))
    , m_xFromRB(m_xBuilder->weld_radio_button(u"fromrb"_ustr))
    , m_xFromNF(m_xBuilder->weld_spin_button(u"from"_ustr))
    , m_xToFT(m_xBuilder->weld_label(u"toft"_ustr))
    , m_xToNF(m_xBuilder->weld_spin_button(u"to"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xCopyToPB->connect_clicked(LINK(this, SwMMResultEmailDialog, CopyToHdl_Impl));
    m_xSendAsPB->connect_clicked(LINK(this, SwMMResultEmailDialog, SendAsHdl_Impl));
    m_xSendAsLB->connect_changed(LINK(this, SwMMResultEmailDialog, SendTypeHdl_Impl));
    m_xPasswordCB->connect_toggled( LINK( this, SwMMResultEmailDialog, CheckHdl ));

    Link<weld::Toggleable&,void> aLink = LINK(this, SwMMResultEmailDialog, DocumentSelectionHdl_Impl);
    m_xSendAllRB->connect_toggled(aLink);
    m_xFromRB->connect_toggled(aLink);
    // m_xSendAllRB is the default, so disable m_xFromNF and m_xToNF initially.
    aLink.Call(*m_xSendAllRB);

    m_xOKButton->connect_clicked(LINK(this, SwMMResultEmailDialog, SendDocumentsHdl_Impl));

    m_xPasswordCB->set_sensitive(false);
    m_xPasswordFT->set_sensitive(false);
    m_xPasswordLB->set_sensitive(false);

    FillInEmailSettings();
}

SwMMResultEmailDialog::~SwMMResultEmailDialog()
{
}

void SwMMResultPrintDialog::FillInPrinterSettings()
{
    //fill printer ListBox
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    const std::shared_ptr<SwMailMergeConfigItem>& xConfigItem = pView->GetMailMergeConfigItem();
    const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
    unsigned int nCount = rPrinters.size();
    bool bMergePrinterExists = false;

    for (unsigned int i = 0; i < nCount; ++i)
    {
        m_xPrinterLB->append_text( rPrinters[i] );
        if( !bMergePrinterExists && rPrinters[i] == xConfigItem->GetSelectedPrinter() )
            bMergePrinterExists = true;
    }

    assert(xConfigItem);
    if(!bMergePrinterExists)
    {
        SfxPrinter* pPrinter = pView->GetWrtShell().getIDocumentDeviceAccess().getPrinter( true );
        m_xPrinterLB->set_active_text(pPrinter->GetName());
    }
    else
    {
        m_xPrinterLB->set_active_text(xConfigItem->GetSelectedPrinter());
    }
    PrinterChangeHdl_Impl(*m_xPrinterLB);

    sal_Int32 count = xConfigItem->GetMergedDocumentCount();
    m_xFromNF->set_max(count);
    m_xToNF->set_value(count);
    m_xToNF->set_max(count);
}

void SwMMResultEmailDialog::FillInEmailSettings()
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    const std::shared_ptr<SwMailMergeConfigItem>& xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);

    SwView* pSourceView = xConfigItem->GetSourceView();
    OSL_ENSURE(pSourceView, "no source view exists");
    if (pSourceView)
    {
        SwDocShell* pDocShell = pSourceView->GetDocShell();
        if (pDocShell->HasName())
        {
            INetURLObject aTmp(pDocShell->GetMedium()->GetName());
            m_xAttachmentED->set_text(aTmp.getName(
                    INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset ));
        }
    }

    if (m_xAttachmentED->get_text().isEmpty())
    {
        OUString sAttach = "." + lcl_GetExtensionForDocType(m_xSendAsLB->get_active_id().toUInt32());
        m_xAttachmentED->set_text(sAttach);

    }

    //select first column
    uno::Reference< sdbcx::XColumnsSupplier > xColsSupp(xConfigItem->GetResultSet(), uno::UNO_QUERY);
    //get the name of the actual columns
    uno::Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
    uno::Sequence< OUString > aFields;
    if (xColAccess.is())
        aFields = xColAccess->getElementNames();

    // fill mail address and password ListBox
    assert(m_xMailToLB->get_count() == 0);
    assert(m_xPasswordLB->get_count() == 0);
    for (const OUString& rField : aFields)
    {
        m_xMailToLB->append_text(rField);
        m_xPasswordLB->append_text(rField);
    }

    m_xMailToLB->set_active(0);
    m_xPasswordLB->set_active(0);

    // then select the right one - may not be available
    const std::vector<std::pair<OUString, int>>& rHeaders = xConfigItem->GetDefaultAddressHeaders();
    OUString sEMailColumn = rHeaders[MM_PART_E_MAIL].first;
    Sequence< OUString> aAssignment = xConfigItem->GetColumnAssignment(xConfigItem->GetCurrentDBData());
    if (aAssignment.getLength() > MM_PART_E_MAIL && !aAssignment[MM_PART_E_MAIL].isEmpty())
        sEMailColumn = aAssignment[MM_PART_E_MAIL];
    if (int pos = m_xMailToLB->find_text(sEMailColumn); pos >= 0)
        m_xMailToLB->set_active(pos);

    // HTML format pre-selected
    m_xSendAsLB->set_active(3);
    SendTypeHdl_Impl(*m_xSendAsLB);

    const sal_Int32 nCount = xConfigItem->GetMergedDocumentCount();
    m_xFromNF->set_max(nCount);
    m_xToNF->set_max(nCount);
    m_xToNF->set_value(nCount);
}

IMPL_LINK_NOARG(SwMMResultSaveDialog, DocumentSelectionHdl_Impl, weld::Toggleable&, void)
{
    bool bEnableFromTo = m_xFromRB->get_active();
    m_xFromNF->set_sensitive(bEnableFromTo);
    m_xToFT->set_sensitive(bEnableFromTo);
    m_xToNF->set_sensitive(bEnableFromTo);
}

IMPL_LINK_NOARG(SwMMResultEmailDialog, CheckHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xPasswordCB->get_active();

    m_xPasswordFT->set_sensitive(bEnable);
    m_xPasswordLB->set_sensitive(bEnable);
}

IMPL_LINK_NOARG(SwMMResultPrintDialog, DocumentSelectionHdl_Impl, weld::Toggleable&, void)
{
    bool bEnableFromTo = m_xFromRB->get_active();
    m_xFromNF->set_sensitive(bEnableFromTo);
    m_xToFT->set_sensitive(bEnableFromTo);
    m_xToNF->set_sensitive(bEnableFromTo);
}

IMPL_LINK_NOARG(SwMMResultEmailDialog, DocumentSelectionHdl_Impl, weld::Toggleable&, void)
{
    bool bEnableFromTo = m_xFromRB->get_active();
    m_xFromNF->set_sensitive(bEnableFromTo);
    m_xToFT->set_sensitive(bEnableFromTo);
    m_xToNF->set_sensitive(bEnableFromTo);
}

IMPL_LINK_NOARG(SwMMResultEmailDialog, CopyToHdl_Impl, weld::Button&, void)
{
    SwCopyToDialog aDlg(m_xDialog.get());
    aDlg.SetCC(m_sCC );
    aDlg.SetBCC(m_sBCC);
    if (aDlg.run() == RET_OK)
    {
        m_sCC = aDlg.GetCC() ;
        m_sBCC = aDlg.GetBCC();
    }
}

namespace {

int documentStartPageNumber(SwMailMergeConfigItem* pConfigItem, int document, bool bIgnoreEmpty)
{
    SwView* pTargetView = pConfigItem->GetTargetView();
    assert( pTargetView );
    SwCursorShell& shell = pTargetView->GetWrtShell();
    const SwDocMergeInfo& info = pConfigItem->GetDocumentMergeInfo(document);
    sal_uInt16 page;
    shell.Push();
    shell.GotoMark( info.startPageInTarget );
    if (!bIgnoreEmpty)
        shell.GetPageNum(page, o3tl::temporary(sal_uInt16()));
    else
        page = shell.GetPageNumSeqNonEmpty();
    shell.Pop(SwCursorShell::PopMode::DeleteCurrent);
    return page;
}

int documentEndPageNumber(SwMailMergeConfigItem* pConfigItem, int document, bool bIgnoreEmpty)
{
    SwView* pTargetView = pConfigItem->GetTargetView();
    assert( pTargetView );
    SwWrtShell& shell = pTargetView->GetWrtShell();
    shell.Push();
    if (document < int(pConfigItem->GetMergedDocumentCount()) - 1)
    {
        // Go to the page before the starting page of the next merged document.
        const SwDocMergeInfo& info = pConfigItem->GetDocumentMergeInfo( document + 1 );
        shell.GotoMark( info.startPageInTarget );
        shell.EndPrvPg();
    }
    else
    {   // This is the last merged document, so it ends on the page at which the document ends.
        shell.SttEndDoc( false ); // go to doc end
    }
    sal_uInt16 page;
    if (!bIgnoreEmpty)
        shell.GetPageNum(page, o3tl::temporary(sal_uInt16()));
    else
        page = shell.GetPageNumSeqNonEmpty();
    shell.Pop(SwCursorShell::PopMode::DeleteCurrent);
    return page;
}

} // anonymous namespace

IMPL_LINK_NOARG(SwMMResultSaveDialog, SaveOutputHdl_Impl, weld::Button&, void)
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);

    const sal_uInt32 nDocumentCount = xConfigItem->GetMergedDocumentCount();
    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = nDocumentCount;

    if (m_xFromRB->get_active())
    {
        nBegin = static_cast<sal_Int32>(m_xFromNF->get_value() - 1);
        nEnd = static_cast<sal_Int32>(m_xToNF->get_value());
        if (nEnd > nDocumentCount)
            nEnd = nDocumentCount;
    }

    xConfigItem->SetBeginEnd(nBegin, nEnd);

    if (!xConfigItem->GetTargetView())
        SwDBManager::PerformMailMerge(pView);

    SwView* pTargetView = xConfigItem->GetTargetView();
    assert(pTargetView);

    OUString sFilter;
    OUString sPath = SwMailMergeHelper::CallSaveAsDialog(m_xDialog.get(), sFilter);
    if (sPath.isEmpty())
    {
        // just return back to the dialog
        return;
    }

    if (m_xSaveAsOneRB->get_active())
    {
        uno::Sequence< beans::PropertyValue > aValues { comphelper::makePropertyValue(u"FilterName"_ustr, sFilter) };

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        ErrCode nErrorCode = ERRCODE_NONE;
        try
        {
            xStore->storeToURL( sPath, aValues );
        }
        catch (const task::ErrorCodeIOException& rErrorEx)
        {
            nErrorCode = ErrCode(rErrorEx.ErrCode);
        }
        catch (const Exception&)
        {
            nErrorCode = ERRCODE_IO_GENERAL;
        }
        if( nErrorCode != ERRCODE_NONE )
        {
            SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC, pTargetView->GetDocShell()->GetTitle());
            ErrorHandler::HandleError( nErrorCode );
        }
    }
    else
    {
        OUString sTargetTempURL = URIHelper::SmartRel2Abs(
            INetURLObject(), utl::CreateTempName(),
            URIHelper::GetMaybeFileHdl());
        std::shared_ptr<const SfxFilter> pSfxFlt = SwIoSystem::GetFilterOfFormat(
                FILTER_XML,
                SwDocShell::Factory().GetFilterContainer() );

        uno::Sequence< beans::PropertyValue > aValues(1);
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = "FilterName";
        pValues[0].Value <<= pSfxFlt->GetFilterName();

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        ErrCode nErrorCode = ERRCODE_NONE;
        try
        {
            xStore->storeToURL( sTargetTempURL, aValues );
        }
        catch (const task::ErrorCodeIOException& rErrorEx)
        {
            nErrorCode = ErrCode(rErrorEx.ErrCode);
        }
        catch (const Exception&)
        {
            nErrorCode = ERRCODE_IO_GENERAL;
        }
        if( nErrorCode != ERRCODE_NONE )
        {
            SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC, pTargetView->GetDocShell()->GetTitle());
            ErrorHandler::HandleError( nErrorCode );
        }

        SwView* pSourceView = xConfigItem->GetSourceView();
        auto xSaveMonitor = std::make_shared<SaveMonitor>(m_xDialog.get());
        xSaveMonitor->m_xDocName->set_label(pSourceView->GetDocShell()->GetTitle(22));
        xSaveMonitor->m_xPrinter->set_label( INetURLObject( sPath ).getFSysPath( FSysStyle::Detect ) );
        m_bCancelSaving = false;
        weld::DialogController::runAsync(xSaveMonitor, [this, &xSaveMonitor](sal_Int32 nResult){
            if (nResult == RET_CANCEL)
                m_bCancelSaving = true;
            xSaveMonitor.reset();
        });

        for(sal_uInt32 nDoc = 0; nDoc < nEnd - nBegin && !m_bCancelSaving; ++nDoc)
        {
            INetURLObject aURL(sPath);
            OUString sExtension = aURL.getExtension();
            if (sExtension.isEmpty())
            {
                sExtension = pSfxFlt->GetWildcard().getGlob().getToken(1, '.');
                sPath += "." + sExtension;
            }
            OUString sStat = SwResId(STR_STATSTR_LETTER) + " " + OUString::number(nDoc + 1);
            xSaveMonitor->m_xPrintInfo->set_label(sStat);

            //now extract a document from the target document
            // the shell will be closed at the end, but it is more safe to use SfxObjectShellLock here
            SfxObjectShellLock xTempDocShell( new SwDocShell( SfxObjectCreateMode::STANDARD ) );
            xTempDocShell->DoInitNew();
            SfxViewFrame* pTempFrame = SfxViewFrame::LoadHiddenDocument( *xTempDocShell, SFX_INTERFACE_NONE );
            SwView* pTempView = static_cast<SwView*>( pTempFrame->GetViewShell() );
            pTargetView->GetWrtShell().StartAction();
            SwgReaderOption aOpt;
            aOpt.SetTextFormats( true );
            aOpt.SetFrameFormats( true );
            aOpt.SetPageDescs( true );
            aOpt.SetNumRules( true );
            aOpt.SetMerge( false );
            pTempView->GetDocShell()->LoadStylesFromFile(
                    sTargetTempURL, aOpt, true );
            pTempView->GetDocShell()->GetDoc()->ReplaceCompatibilityOptions( *pTargetView->GetDocShell()->GetDoc());
            pTempView->GetDocShell()->GetDoc()->ReplaceDefaults( *pTargetView->GetDocShell()->GetDoc());
            pTempView->GetDocShell()->GetDoc()->ReplaceDocumentProperties( *pTargetView->GetDocShell()->GetDoc(), true );

            uno::Reference<beans::XPropertySet> const xThisSet(
                pTargetView->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
            uno::Reference<beans::XPropertySet> const xRetSet(
                pTempView->GetDocShell()->GetBaseModel(), uno::UNO_QUERY_THROW);
            uno::Sequence<beans::PropertyValue> aInteropGrabBag;
            xThisSet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aInteropGrabBag;
            xRetSet->setPropertyValue(u"InteropGrabBag"_ustr, uno::Any(aInteropGrabBag));

            pTargetView->GetWrtShell().PastePages(
                pTempView->GetWrtShell(), documentStartPageNumber(xConfigItem.get(), nDoc, false),
                documentEndPageNumber(xConfigItem.get(), nDoc, false));
            pTargetView->GetWrtShell().EndAction();
            //then save it
            OUString sOutPath = aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
            OUString sCounter = "_" + OUString::number(nDoc + 1);
            sOutPath = sOutPath.replaceAt( sOutPath.getLength() - sExtension.getLength() - 1, 0, sCounter);

            while(true)
            {
                //time for other slots is needed
                Scheduler::ProcessEventsToIdle();

                bool bFailed = false;
                try
                {
                    pValues[0].Value <<= sFilter;
                    uno::Reference< frame::XStorable > xTempStore( xTempDocShell->GetModel(), uno::UNO_QUERY);
                    xTempStore->storeToURL( sOutPath, aValues   );
                }
                catch (const uno::Exception&)
                {
                    bFailed = true;
                }

                if(bFailed)
                {
                    std::unique_ptr<SwSaveWarningBox_Impl> xWarning(new SwSaveWarningBox_Impl(m_xDialog.get(), sOutPath));
                    if (RET_OK == xWarning->run())
                        sOutPath = xWarning->GetFileName();
                    else
                    {
                        xTempDocShell->DoClose();
                        m_xDialog->response(RET_OK);
                        return;
                    }
                }
                else
                {
                    xTempDocShell->DoClose();
                    m_xDialog->response(RET_OK);
                    break;
                }
            }
        }
        if (xSaveMonitor)
            xSaveMonitor->response(RET_OK);
        ::osl::File::remove( sTargetTempURL );
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwMMResultPrintDialog, PrinterChangeHdl_Impl, weld::ComboBox&, rBox, void)
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    const std::shared_ptr<SwMailMergeConfigItem>& xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);
    if (rBox.get_active() != -1)
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( rBox.get_active_text(), false );

        if( pInfo )
        {
            if ( !m_pTempPrinter )
            {
                m_pTempPrinter = VclPtr<Printer>::Create( *pInfo );
            }
            else
            {
                if( (m_pTempPrinter->GetName() != pInfo->GetPrinterName()) ||
                     (m_pTempPrinter->GetDriverName() != pInfo->GetDriver()) )
                {
                    m_pTempPrinter.disposeAndClear();
                    m_pTempPrinter = VclPtr<Printer>::Create( *pInfo );
                }
            }
        }
        else if( ! m_pTempPrinter )
            m_pTempPrinter = VclPtr<Printer>::Create();

        m_xPrinterSettingsPB->set_sensitive(m_pTempPrinter->HasSupport(PrinterSupport::SetupDialog));
    }
    else
        m_xPrinterSettingsPB->set_sensitive(false);

    xConfigItem->SetSelectedPrinter(rBox.get_active_text());
}

IMPL_LINK_NOARG(SwMMResultPrintDialog, PrintHdl_Impl, weld::Button&, void)
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);

    const sal_uInt32 nDocumentCount = xConfigItem->GetMergedDocumentCount();
    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = nDocumentCount;

    if (m_xFromRB->get_active())
    {
        nBegin = static_cast<sal_Int32>(m_xFromNF->get_value() - 1);
        nEnd = static_cast<sal_Int32>(m_xToNF->get_value());
        if (nEnd > nDocumentCount)
            nEnd = nDocumentCount;
    }

    xConfigItem->SetBeginEnd(nBegin, nEnd);

    if(!xConfigItem->GetTargetView())
        SwDBManager::PerformMailMerge(pView);

    SwView* pTargetView = xConfigItem->GetTargetView();
    assert(pTargetView);

    // If we skip autoinserted blanks, then the page numbers used in the print range string
    // refer to the non-blank pages as they appear in the document (see tdf#89708).
    const bool bIgnoreEmptyPages =
            !pTargetView->GetDocShell()->GetDoc()->getIDocumentDeviceAccess().getPrintData().IsPrintEmptyPages();
    const int nStartPage = documentStartPageNumber(xConfigItem.get(), 0, bIgnoreEmptyPages);
    const int nEndPage = documentEndPageNumber(xConfigItem.get(), nEnd - nBegin - 1, bIgnoreEmptyPages);

    const OUString sPages(OUString::number(nStartPage) + "-" + OUString::number(nEndPage));

    pTargetView->SetMailMergeConfigItem(xConfigItem);
    if(m_pTempPrinter)
    {
        SfxPrinter *const pDocumentPrinter = pTargetView->GetWrtShell()
            .getIDocumentDeviceAccess().getPrinter(true);
        pDocumentPrinter->SetPrinterProps(m_pTempPrinter);
        // this should be able to handle setting its own printer
        pTargetView->SetPrinter(pDocumentPrinter);
    }

    SfxObjectShell* pObjSh = pTargetView->GetViewFrame().GetObjectShell();
    SfxGetpApp()->NotifyEvent(SfxEventHint(SfxEventHintId::SwMailMerge, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE), pObjSh));

    uno::Sequence aProps{ comphelper::makePropertyValue(u"MonitorVisible"_ustr, true),
                          comphelper::makePropertyValue(u"Pages"_ustr, sPages) };

    pTargetView->ExecPrint( aProps, false, true );
    SfxGetpApp()->NotifyEvent(SfxEventHint(SfxEventHintId::SwMailMergeEnd, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE_END), pObjSh));

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SwMMResultPrintDialog, PrinterSetupHdl_Impl, weld::Button&, void)
{
    if (m_pTempPrinter)
        m_pTempPrinter->Setup(m_xDialog.get());
}

IMPL_LINK(SwMMResultEmailDialog, SendTypeHdl_Impl, weld::ComboBox&, rBox, void)
{
    auto nDocType = rBox.get_active_id().toUInt32();
    bool bEnable = MM_DOCTYPE_HTML != nDocType && MM_DOCTYPE_TEXT != nDocType;
    bool bIsPDF = nDocType == MM_DOCTYPE_PDF;
    m_xSendAsPB->set_sensitive(bEnable);
    m_xAttachmentGroup->set_sensitive(bEnable);
    if(bEnable)
    {
        //add the correct extension
        OUString sAttach(m_xAttachmentED->get_text());
        //do nothing if the user has removed the name - the warning will come early enough
        if (!sAttach.isEmpty())
        {
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sAttach, '.');
            if( 2 > nTokenCount)
            {
                sAttach += ".";
                ++nTokenCount;
            }
            sAttach = comphelper::string::setToken(sAttach, nTokenCount - 1, '.', lcl_GetExtensionForDocType( nDocType ));
            m_xAttachmentED->set_text(sAttach);
        }
    }

    if(bIsPDF)
    {
        m_xPasswordCB->set_sensitive(true);
        m_xPasswordFT->set_sensitive(true);
        m_xPasswordLB->set_sensitive(true);
        CheckHdl(*m_xPasswordCB);
    }
    else
    {
        m_xPasswordCB->set_sensitive(false);
        m_xPasswordFT->set_sensitive(false);
        m_xPasswordLB->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SwMMResultEmailDialog, SendAsHdl_Impl, weld::Button&, void)
{
    if (SwView* pView = GetActiveView())
    {
        SwMailBodyDialog aDlg(m_xDialog.get(), pView->GetMailMergeConfigItem());
        aDlg.SetBody(m_sBody);
        if (RET_OK == aDlg.run())
        {
            m_sBody = aDlg.GetBody();
        }
    }
}

// Send documents as e-mail
IMPL_LINK_NOARG(SwMMResultEmailDialog, SendDocumentsHdl_Impl, weld::Button&, void)
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);

    const sal_uInt32 nDocumentCount = xConfigItem->GetMergedDocumentCount();
    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = nDocumentCount;

    if (m_xFromRB->get_active())
    {
        nBegin = static_cast<sal_Int32>(m_xFromNF->get_value() - 1);
        nEnd = static_cast<sal_Int32>(m_xToNF->get_value());
        if (nEnd > nDocumentCount)
            nEnd = nDocumentCount;
    }

    xConfigItem->SetBeginEnd(nBegin, nEnd);

    if (!xConfigItem->GetTargetView())
        SwDBManager::PerformMailMerge(pView);

    //get the composed document
    SwView* pTargetView = xConfigItem->GetTargetView();
    SAL_WARN_IF(!pTargetView, "sw.ui", "No TargetView in SwMailMergeConfigItem");

    if (xConfigItem->GetMailServer().isEmpty() ||
            !SwMailMergeHelper::CheckMailAddress(xConfigItem->GetMailAddress()) )
    {
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       m_sConfigureMail));
        xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
        sal_uInt16 nRet = xQueryBox->run();
        if (RET_YES == nRet )
        {
            SwView* pConfigView = pTargetView ? pTargetView : pView;
            SfxAllItemSet aSet(pConfigView->GetPool());
            SwMailConfigDlg aDlg(m_xDialog.get(), aSet);
            nRet = aDlg.run();
        }

        if(nRet != RET_OK && nRet != RET_YES)
            return; // back to the dialog

        // SwMailConfigDlg writes mail merge email settings only to (global) config,
        // so copy them to the existing config item
        lcl_UpdateEmailSettingsFromGlobalConfig(*xConfigItem);
    }
    //add the documents
    bool bAsBody = false;
    rtl_TextEncoding eEncoding = ::osl_getThreadTextEncoding();
    SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
    std::shared_ptr<const SfxFilter> pSfxFlt;
    auto nDocType = m_xSendAsLB->get_active_id().toUInt32();
    OUString sExtension = lcl_GetExtensionForDocType(nDocType);
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO:
        {
            //Make sure we don't pick e.g. the flat xml filter
            //for this format
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                u"writer8"_ustr,
                SfxFilterFlags::EXPORT);
        }
        break;
        case MM_DOCTYPE_PDF:
        {
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                u"writer_pdf_Export"_ustr,
                SfxFilterFlags::EXPORT);
        }
        break;
        case MM_DOCTYPE_WORD:
        {
            //the method SwIOSystemGetFilterOfFormat( ) returns the template filter
            //because it uses the same user data :-(
            SfxFilterMatcher aMatcher( pFilterContainer->GetName() );
            SfxFilterMatcherIter aIter( aMatcher );
            std::shared_ptr<const SfxFilter> pFilter = aIter.First();
            while ( pFilter )
            {
                if( pFilter->GetUserData() == FILTER_WW8 && pFilter->CanExport() )
                {
                    pSfxFlt = pFilter;
                    break;
                }
                pFilter = aIter.Next();
            }

        }
        break;
        case MM_DOCTYPE_HTML:
        {
            bAsBody = true;
            eEncoding = RTL_TEXTENCODING_UTF8;
        }
        break;
        case MM_DOCTYPE_TEXT:
        {
            bAsBody = true;
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                u"Text (encoded)"_ustr, SfxFilterFlags::EXPORT);
        }
        break;
    }
    if(!pSfxFlt)
        pSfxFlt = pFilterContainer->GetFilter4Extension(sExtension, SfxFilterFlags::EXPORT);

    if(!pSfxFlt)
    {
        m_xDialog->response(RET_OK);
        return;
    }
    OUString sMimeType = pSfxFlt->GetMimeType();

    if (m_xSubjectED->get_text().isEmpty())
    {
        std::unique_ptr<SwSendQueryBox_Impl> xQuery(new SwSendQueryBox_Impl(m_xDialog.get(), u"SubjectDialog"_ustr,
                                                         u"modules/swriter/ui/subjectdialog.ui"_ustr));
        xQuery->SetIsEmptyTextAllowed(true);
        xQuery->SetValue(u""_ustr);
        if(RET_OK == xQuery->run())
        {
            if (!xQuery->GetValue().isEmpty())
                m_xSubjectED->set_text(xQuery->GetValue());
        }
        else
            return; // back to the dialog
    }
    if(!bAsBody && m_xAttachmentED->get_text().isEmpty())
    {
        std::unique_ptr<SwSendQueryBox_Impl> xQuery(new SwSendQueryBox_Impl(m_xDialog.get(), u"AttachNameDialog"_ustr,
                                                         u"modules/swriter/ui/attachnamedialog.ui"_ustr));
        xQuery->SetIsEmptyTextAllowed(false);
        if (RET_OK == xQuery->run())
        {
            OUString sAttach(xQuery->GetValue());
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sAttach, '.');
            if (2 > nTokenCount)
            {
                sAttach += ".";
                ++nTokenCount;
            }
            sAttach = comphelper::string::setToken(sAttach, nTokenCount - 1, '.', lcl_GetExtensionForDocType(
                                                   m_xSendAsLB->get_active_id().toUInt32()));
            m_xAttachmentED->set_text(sAttach);
        }
        else
            return; // back to the dialog
    }

    OUString sEMailColumn = m_xMailToLB->get_active_text();
    OSL_ENSURE( !sEMailColumn.isEmpty(), "No email column selected");
    Reference< sdbcx::XColumnsSupplier > xColsSupp( xConfigItem->GetResultSet(), UNO_QUERY);
    Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
    if(sEMailColumn.isEmpty() || !xColAccess.is() || !xColAccess->hasByName(sEMailColumn))
    {
        m_xDialog->response(RET_OK);
        return;
    }

    OUString sPasswordColumn = m_xPasswordLB->get_active_text();
    OSL_ENSURE( !sPasswordColumn.isEmpty(), "No password column selected");
    if(sPasswordColumn.isEmpty() || !xColAccess.is() || !xColAccess->hasByName(sPasswordColumn))
    {
        m_xDialog->response(RET_OK);
        return;
    }

    OUString sFilterOptions;
    if(MM_DOCTYPE_TEXT == nDocType)
    {
        SwAsciiOptions aOpt;
        sal_uInt16 nAppScriptType = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
        TypedWhichId<SvxLanguageItem> nWhich = GetWhichOfScript( RES_CHRATR_LANGUAGE, nAppScriptType);
        const SvxLanguageItem& rDefLangItem = pTargetView->GetWrtShell().GetDefault( nWhich );
        aOpt.SetLanguage( rDefLangItem.GetLanguage() );
        aOpt.SetParaFlags( LINEEND_CR );
        aOpt.WriteUserData( sFilterOptions );
    }
    else if(MM_DOCTYPE_HTML == nDocType)
    {
        sFilterOptions = "EmbedImages";
    }
    OUString sTargetTempURL = URIHelper::SmartRel2Abs(
        INetURLObject(), utl::CreateTempName(),
        URIHelper::GetMaybeFileHdl());
    std::shared_ptr<const SfxFilter> pTargetSfxFlt = SwIoSystem::GetFilterOfFormat(
            FILTER_XML,
            SwDocShell::Factory().GetFilterContainer() );

    uno::Sequence< beans::PropertyValue > aValues { comphelper::makePropertyValue(u"FilterName"_ustr, pTargetSfxFlt->GetFilterName()) };

    uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
    xStore->storeToURL( sTargetTempURL, aValues   );

    //create the send dialog
    std::shared_ptr<SwSendMailDialog> xDlg = std::make_shared<SwSendMailDialog>(Application::GetDefDialogParent(), *xConfigItem);

    xDlg->StartSend(nEnd - nBegin);
    weld::DialogController::runAsync(xDlg, [](sal_Int32 /*nResult*/){});

    //help to force painting the dialog
    //TODO/CLEANUP
    //predetermined breaking point
    Application::Reschedule( true );
    m_xDialog->response(RET_OK);
    for(sal_uInt32 nDoc = 0; nDoc < nEnd - nBegin; ++nDoc)
    {
        SwDocMergeInfo& rInfo = xConfigItem->GetDocumentMergeInfo(nDoc);

        //now extract a document from the target document
        // the shell will be closed at the end, but it is more safe to use SfxObjectShellLock here
        SfxObjectShellLock xTempDocShell( new SwDocShell( SfxObjectCreateMode::STANDARD ) );
        xTempDocShell->DoInitNew();
        SfxViewFrame* pTempFrame = SfxViewFrame::LoadHiddenDocument( *xTempDocShell, SFX_INTERFACE_NONE );
        SwView* pTempView = static_cast<SwView*>( pTempFrame->GetViewShell() );
        pTargetView->GetWrtShell().StartAction();
        SwgReaderOption aOpt;
        aOpt.SetTextFormats( true );
        aOpt.SetFrameFormats( true );
        aOpt.SetPageDescs( true );
        aOpt.SetNumRules( true );
        aOpt.SetMerge( false );
        pTempView->GetDocShell()->LoadStylesFromFile(
                sTargetTempURL, aOpt, true );
        pTempView->GetDocShell()->GetDoc()->ReplaceCompatibilityOptions( *pTargetView->GetDocShell()->GetDoc());
        pTempView->GetDocShell()->GetDoc()->ReplaceDefaults( *pTargetView->GetDocShell()->GetDoc());
        pTempView->GetDocShell()->GetDoc()->ReplaceDocumentProperties( *pTargetView->GetDocShell()->GetDoc(), true );
        pTargetView->GetWrtShell().PastePages(
            pTempView->GetWrtShell(), documentStartPageNumber(xConfigItem.get(), nDoc, false),
            documentEndPageNumber(xConfigItem.get(), nDoc, false));
        pTargetView->GetWrtShell().EndAction();

        //then save it
        SfxStringItem aName(SID_FILE_NAME,
                URIHelper::SmartRel2Abs(
                    INetURLObject(), utl::CreateTempName(),
                    URIHelper::GetMaybeFileHdl()) );

        {
            bool withFilterOptions = MM_DOCTYPE_TEXT == nDocType || MM_DOCTYPE_HTML == nDocType;
            bool withPasswordOptions = m_xPasswordCB->get_active();

            sal_Int32 nTarget = xConfigItem->MoveResultSet(rInfo.nDBRow);
            OSL_ENSURE( nTarget == rInfo.nDBRow, "row of current document could not be selected");
            OUString sPassword = lcl_GetColumnValueOf(sPasswordColumn, xColAccess);

            sal_Int32 nOptionCount = (withFilterOptions && withPasswordOptions) ? 4 : withPasswordOptions ? 3 : withFilterOptions ? 2 : 1;
            sal_Int32 nOpt = 0;
            uno::Sequence< beans::PropertyValue > aFilterValues(nOptionCount);
            beans::PropertyValue* pFilterValues = aFilterValues.getArray();

            pFilterValues[nOpt].Name = "FilterName";
            pFilterValues[nOpt].Value <<= pSfxFlt->GetFilterName();

            if(withFilterOptions)
            {
                nOpt++;
                pFilterValues[nOpt].Name = "FilterOptions";
                pFilterValues[nOpt].Value <<= sFilterOptions;
            }

            if(withPasswordOptions)
            {
                nOpt++;
                pFilterValues[nOpt].Name = "EncryptFile";
                pFilterValues[nOpt].Value <<= true;
                nOpt++;
                pFilterValues[nOpt].Name = "DocumentOpenPassword";
                pFilterValues[nOpt].Value <<= sPassword;
            }

            uno::Reference< frame::XStorable > xTempStore( pTempView->GetDocShell()->GetModel(), uno::UNO_QUERY);
            xTempStore->storeToURL( aName.GetValue(), aFilterValues );
        }
        xTempDocShell->DoClose();

        sal_Int32 nTarget = xConfigItem->MoveResultSet(rInfo.nDBRow);
        OSL_ENSURE( nTarget == rInfo.nDBRow, "row of current document could not be selected");
        OSL_ENSURE( !sEMailColumn.isEmpty(), "No email column selected");
        OUString sEMail = lcl_GetColumnValueOf(sEMailColumn, xColAccess);
        SwMailDescriptor aDesc;
        aDesc.sEMail = sEMail;
        OUStringBuffer sBody;
        if(bAsBody)
        {
            {
                //read in the temporary file and use it as mail body
                SfxMedium aMedium( aName.GetValue(),    StreamMode::READ);
                SvStream* pInStream = aMedium.GetInStream();
                if(pInStream)
                    pInStream->SetStreamCharSet( eEncoding );
                else
                {
                    OSL_FAIL("no output file created?");
                    continue;
                }
                OStringBuffer sLine;
                bool bDone = pInStream->ReadLine( sLine );
                while ( bDone )
                {
                    sBody.append( OStringToOUString(sLine, eEncoding) + "\n");
                    bDone = pInStream->ReadLine( sLine );
                }
            }
            //remove the temporary file
            SWUnoHelper::UCB_DeleteFile( aName.GetValue() );
        }
        else
        {
            sBody = m_sBody;
            aDesc.sAttachmentURL = aName.GetValue();
            OUString sAttachment(m_xAttachmentED->get_text());
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sAttachment, '.');
            if (2 > nTokenCount)
            {
                sAttachment += ".";
                sAttachment = comphelper::string::setToken(sAttachment, nTokenCount, '.', sExtension);
            }
            else if (o3tl::getToken(sAttachment, nTokenCount - 1, '.') != sExtension)
                sAttachment += sExtension;
            aDesc.sAttachmentName = sAttachment;
            aDesc.sMimeType = sMimeType;

            if (xConfigItem->IsGreetingLine(true))
            {
                OUString sNameColumn = xConfigItem->GetAssignedColumn(MM_PART_LASTNAME);
                OUString sName = lcl_GetColumnValueOf(sNameColumn, xColAccess);
                OUString sGreeting;
                if(!sName.isEmpty() && xConfigItem->IsIndividualGreeting(true))
                {
                    OUString sGenderColumn = xConfigItem->GetAssignedColumn(MM_PART_GENDER);
                    const OUString& sFemaleValue = xConfigItem->GetFemaleGenderValue();
                    OUString sGenderValue = lcl_GetColumnValueOf(sGenderColumn, xColAccess);
                    SwMailMergeConfigItem::Gender eGenderType = sGenderValue == sFemaleValue ?
                        SwMailMergeConfigItem::FEMALE :
                        SwMailMergeConfigItem::MALE;

                    sGreeting = SwAddressPreview::FillData(
                        xConfigItem->GetGreetings(eGenderType)
                        [xConfigItem->GetCurrentGreeting(eGenderType)],
                            *xConfigItem);
                }
                else
                {
                    sGreeting =
                        xConfigItem->GetGreetings(SwMailMergeConfigItem::NEUTRAL)
                        [xConfigItem->GetCurrentGreeting(SwMailMergeConfigItem::NEUTRAL)];

                }
                sGreeting += "\n";
                sBody.insert(0, sGreeting);
            }
        }
        aDesc.sBodyContent = sBody.makeStringAndClear();
        if(MM_DOCTYPE_HTML == nDocType)
        {
            aDesc.sBodyMimeType = "text/html; charset=" +
                OUString::createFromAscii(rtl_getBestMimeCharsetFromTextEncoding( eEncoding ));
        }
        else
            aDesc.sBodyMimeType = "text/plain; charset=UTF-8; format=flowed";

        aDesc.sSubject = m_xSubjectED->get_text();
        aDesc.sCC = m_sCC;
        aDesc.sBCC = m_sBCC;
        xDlg->AddDocument( aDesc );
        //help to force painting the dialog
        Application::Reschedule( true );
        //stop creating of data when dialog has been closed
        if (!xDlg->getDialog()->get_visible())
        {
            break;
        }
    }
    xDlg->EnableDestruction();
    ::osl::File::remove( sTargetTempURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
