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
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <mailconfigpage.hxx>
#include <mmgreetingspage.hxx>
#include <swmessdialog.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <hintids.hxx>
#include <swmodule.hxx>

#include <editeng/scripttypeitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <vcl/print.hxx>
#include <vcl/settings.hxx>

#include <unotools/tempfile.hxx>
#include <osl/file.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <dbmgr.hxx>
#include <swunohelper.hxx>
#include <shellio.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/event.hxx>
#include <swevent.hxx>
#include <dbui.hxx>
#include <dbui.hrc>
#include <helpid.h>
#include <doc.hxx>
#include <sfx2/app.hxx>
#include <strings.hrc>
#include <unomid.h>
#include <comphelper/string.hxx>

using namespace svt;
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

static OUString lcl_GetColumnValueOf(const OUString& rColumn, Reference < container::XNameAccess>& rxColAccess )
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

class SwSaveWarningBox_Impl : public SwMessageAndEditDialog
{
    DECL_LINK( ModifyHdl, Edit&, void);
public:
    SwSaveWarningBox_Impl(vcl::Window* pParent, const OUString& rFileName);

    OUString        GetFileName() const
    {
        return m_pEdit->GetText();
    }
};

class SwSendQueryBox_Impl : public SwMessageAndEditDialog
{
    bool            bIsEmptyAllowed;
    DECL_LINK( ModifyHdl, Edit&, void);
public:
    SwSendQueryBox_Impl(vcl::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription);

    void SetValue(const OUString& rSet)
    {
        m_pEdit->SetText(rSet);
        ModifyHdl(*m_pEdit);
    }

    OUString GetValue() const
    {
        return m_pEdit->GetText();
    }

    void SetIsEmptyTextAllowed(bool bSet)
    {
        bIsEmptyAllowed = bSet;
        ModifyHdl(*m_pEdit);
    }
};

SwSaveWarningBox_Impl::SwSaveWarningBox_Impl(vcl::Window* pParent, const OUString& rFileName)
    : SwMessageAndEditDialog(pParent, "AlreadyExistsDialog",
        "modules/swriter/ui/alreadyexistsdialog.ui")
{
    m_pEdit->SetText(rFileName);
    m_pEdit->SetModifyHdl(LINK(this, SwSaveWarningBox_Impl, ModifyHdl));

    INetURLObject aTmp(rFileName);
    m_pPrimaryMessage->SetText(m_pPrimaryMessage->GetText().replaceAll("%1", aTmp.getName(
            INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset)));

    ModifyHdl(*m_pEdit);
}

IMPL_LINK( SwSaveWarningBox_Impl, ModifyHdl, Edit&, rEdit, void)
{
    m_pOKPB->Enable(!rEdit.GetText().isEmpty());
}

SwSendQueryBox_Impl::SwSendQueryBox_Impl(vcl::Window* pParent, const OUString& rID,
        const OUString& rUIXMLDescription)
    : SwMessageAndEditDialog(pParent, rID, rUIXMLDescription)
    , bIsEmptyAllowed(true)
{
    m_pImageIM->SetImage(QueryBox::GetStandardImage());
    m_pEdit->SetModifyHdl(LINK(this, SwSendQueryBox_Impl, ModifyHdl));
    ModifyHdl(*m_pEdit);
}

IMPL_LINK( SwSendQueryBox_Impl, ModifyHdl, Edit&, rEdit, void)
{
    m_pOKPB->Enable(bIsEmptyAllowed  || !rEdit.GetText().isEmpty());
}

class SwCopyToDialog : public SfxModalDialog
{
    VclPtr<Edit> m_pCCED;
    VclPtr<Edit> m_pBCCED;

public:
    explicit SwCopyToDialog(vcl::Window* pParent)
        : SfxModalDialog(pParent, "CCDialog",
            "modules/swriter/ui/ccdialog.ui")
    {
        get(m_pCCED, "cc");
        get(m_pBCCED, "bcc");
    }
    virtual ~SwCopyToDialog() override { disposeOnce(); }
    virtual void dispose() override
    {
        m_pCCED.clear();
        m_pBCCED.clear();
        SfxModalDialog::dispose();
    }

    OUString GetCC() {return m_pCCED->GetText();}
    void SetCC(const OUString& rSet) {m_pCCED->SetText(rSet);}

    OUString GetBCC() {return m_pBCCED->GetText();}
    void SetBCC(const OUString& rSet) {m_pBCCED->SetText(rSet);}
};

SwMMResultSaveDialog::SwMMResultSaveDialog()
    : SfxModalDialog(nullptr, "MMResultSaveDialog", "modules/swriter/ui/mmresultsavedialog.ui"),
    m_bCancelSaving(false)
{
    get(m_pSaveAsOneRB, "singlerb");
    get(m_pSaveIndividualRB, "individualrb");
    get(m_pFromRB, "fromrb");
    get(m_pFromNF, "from-nospin");
    get(m_pToFT, "toft");
    get(m_pToNF, "to-nospin");
    get(m_pOKButton, "ok");

    Link<Button*,void> aLink = LINK(this, SwMMResultSaveDialog, DocumentSelectionHdl_Impl);
    m_pSaveAsOneRB->SetClickHdl(aLink);
    m_pSaveIndividualRB->SetClickHdl(aLink);
    m_pFromRB->SetClickHdl(aLink);
    // m_pSaveAsOneRB is the default, so disable m_pFromNF and m_pToNF initially.
    aLink.Call(m_pSaveAsOneRB);
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);
    sal_Int32 nCount = xConfigItem->GetMergedDocumentCount();
    m_pToNF->SetMax(nCount);
    m_pToNF->SetValue(nCount);

    m_pOKButton->SetClickHdl(LINK(this, SwMMResultSaveDialog, SaveOutputHdl_Impl));
}

SwMMResultSaveDialog::~SwMMResultSaveDialog()
{
    disposeOnce();
}

void SwMMResultSaveDialog::dispose()
{
    m_pSaveAsOneRB.clear();
    m_pSaveIndividualRB.clear();
    m_pFromRB.clear();
    m_pFromNF.clear();
    m_pToFT.clear();
    m_pToNF.clear();
    m_pOKButton.clear();

    SfxModalDialog::dispose();
}

SwMMResultPrintDialog::SwMMResultPrintDialog()
    : SfxModalDialog(nullptr, "MMResultPrintDialog", "modules/swriter/ui/mmresultprintdialog.ui")
    , m_pTempPrinter(nullptr)
{
    get(m_pPrinterFT, "printerft");
    get(m_pPrinterLB, "printers");
    m_pPrinterLB->SetStyle(m_pPrinterLB->GetStyle() | WB_SORT);
    get(m_pPrinterSettingsPB, "printersettings");
    get(m_pPrintAllRB, "printallrb");
    get(m_pFromRB, "fromrb");
    get(m_pFromNF, "from-nospin");
    get(m_pToFT, "toft");
    get(m_pToNF, "to-nospin");
    get(m_pOKButton, "ok");

    m_pPrinterLB->SetSelectHdl(LINK(this, SwMMResultPrintDialog, PrinterChangeHdl_Impl));
    m_pPrinterSettingsPB->SetClickHdl(LINK(this, SwMMResultPrintDialog, PrinterSetupHdl_Impl));

    Link<Button*,void> aLink = LINK(this, SwMMResultPrintDialog, DocumentSelectionHdl_Impl);
    m_pPrintAllRB->SetClickHdl(aLink);
    m_pFromRB->SetClickHdl(aLink);
    // m_pPrintAllRB is the default, so disable m_pFromNF and m_pToNF initially.
    aLink.Call(m_pPrintAllRB);

    m_pOKButton->SetClickHdl(LINK(this, SwMMResultPrintDialog, PrintHdl_Impl));

    FillInPrinterSettings();
}

SwMMResultPrintDialog::~SwMMResultPrintDialog()
{
    disposeOnce();
}

void SwMMResultPrintDialog::dispose()
{
    m_pPrinterFT.clear();
    m_pPrinterLB.clear();
    m_pPrinterSettingsPB.clear();
    m_pPrintAllRB.clear();
    m_pFromRB.clear();
    m_pFromNF.clear();
    m_pToFT.clear();
    m_pToNF.clear();
    m_pOKButton.clear();
    m_pTempPrinter.clear();

    SfxModalDialog::dispose();
}

SwMMResultEmailDialog::SwMMResultEmailDialog()
    : SfxModalDialog(nullptr, "MMResultEmailDialog", "modules/swriter/ui/mmresultemaildialog.ui"),
     m_sConfigureMail(SwResId(ST_CONFIGUREMAIL))
{
    get(m_pMailToFT, "mailtoft");
    get(m_pMailToLB, "mailto");
    get(m_pCopyToPB, "copyto");
    get(m_pSubjectFT, "subjectft");
    get(m_pSubjectED, "subject");
    get(m_pSendAsFT, "sendasft");
    get(m_pSendAsLB, "sendas");
    get(m_pAttachmentGroup, "attachgroup");
    get(m_pAttachmentED, "attach");
    get(m_pSendAsPB, "sendassettings");
    get(m_pSendAllRB, "sendallrb");
    get(m_pFromRB, "fromrb");
    get(m_pFromNF, "from-nospin");
    get(m_pToFT, "toft");
    get(m_pToNF, "to-nospin");
    get(m_pOKButton, "ok");

    m_pCopyToPB->SetClickHdl(LINK(this, SwMMResultEmailDialog, CopyToHdl_Impl));
    m_pSendAsPB->SetClickHdl(LINK(this, SwMMResultEmailDialog, SendAsHdl_Impl));
    m_pSendAsLB->SetSelectHdl(LINK(this, SwMMResultEmailDialog, SendTypeHdl_Impl));

    Link<Button*,void> aLink = LINK(this, SwMMResultEmailDialog, DocumentSelectionHdl_Impl);
    m_pSendAllRB->SetClickHdl(aLink);
    m_pFromRB->SetClickHdl(aLink);
    // m_pSendAllRB is the default, so disable m_pFromNF and m_pToNF initially.
    aLink.Call(m_pSendAllRB);

    m_pOKButton->SetClickHdl(LINK(this, SwMMResultEmailDialog, SendDocumentsHdl_Impl));

    FillInEmailSettings();
}

SwMMResultEmailDialog::~SwMMResultEmailDialog()
{
    disposeOnce();
}

void SwMMResultEmailDialog::dispose()
{
    m_pMailToFT.clear();
    m_pMailToLB.clear();
    m_pCopyToPB.clear();
    m_pSubjectFT.clear();
    m_pSubjectED.clear();
    m_pSendAsFT.clear();
    m_pSendAsLB.clear();
    m_pAttachmentGroup.clear();
    m_pAttachmentED.clear();
    m_pSendAllRB.clear();
    m_pFromRB.clear();
    m_pFromNF.clear();
    m_pToFT.clear();
    m_pToNF.clear();
    m_pOKButton.clear();
    m_pSendAsPB.clear();

    SfxModalDialog::dispose();
}

void SwMMResultPrintDialog::FillInPrinterSettings()
{
    //fill printer ListBox
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
    unsigned int nCount = rPrinters.size();
    bool bMergePrinterExists = false;
    if ( nCount )
    {
        for( unsigned int i = 0; i < nCount; i++ )
        {
            m_pPrinterLB->InsertEntry( rPrinters[i] );
            if( !bMergePrinterExists && rPrinters[i] == xConfigItem->GetSelectedPrinter() )
                bMergePrinterExists = true;
        }

    }

    assert(xConfigItem);
    if(!bMergePrinterExists)
    {
        SfxPrinter* pPrinter = pView->GetWrtShell().getIDocumentDeviceAccess().getPrinter( true );
        m_pPrinterLB->SelectEntry(pPrinter->GetName());
    }
    else
    {
        m_pPrinterLB->SelectEntry(xConfigItem->GetSelectedPrinter());
    }

    sal_Int32 count = xConfigItem->GetMergedDocumentCount();
    m_pToNF->SetValue(count);
    m_pToNF->SetMax(count);

    m_pPrinterLB->SelectEntry(xConfigItem->GetSelectedPrinter());
}

void SwMMResultEmailDialog::FillInEmailSettings()
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);

    SwView* pSourceView = xConfigItem->GetSourceView();
    OSL_ENSURE(pSourceView, "no source view exists");
    if (pSourceView)
    {
        SwDocShell* pDocShell = pSourceView->GetDocShell();
        if (pDocShell->HasName())
        {
            INetURLObject aTmp(pDocShell->GetMedium()->GetName());
            m_pAttachmentED->SetText(aTmp.getName(
                    INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset ));
        }
    }

    if (m_pAttachmentED->GetText().isEmpty())
    {
        OUString sAttach(m_sDefaultAttachmentST);
        sAttach += ".";
        sAttach += lcl_GetExtensionForDocType(
                    reinterpret_cast<sal_uLong>(m_pSendAsLB->GetSelectEntryData()));
        m_pAttachmentED->SetText(sAttach);

    }

    //fill mail address ListBox
    if (!m_pMailToLB->GetEntryCount())
    {
        //select first column
        uno::Reference< sdbcx::XColumnsSupplier > xColsSupp(xConfigItem->GetResultSet(), uno::UNO_QUERY);
        //get the name of the actual columns
        uno::Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
        uno::Sequence< OUString > aFields;
        if (xColAccess.is())
            aFields = xColAccess->getElementNames();
        const OUString* pFields = aFields.getConstArray();
        for (sal_Int32 nField = 0; nField < aFields.getLength(); ++nField)
            m_pMailToLB->InsertEntry(pFields[nField]);

        m_pMailToLB->SelectEntryPos(0);
        // then select the right one - may not be available
        const std::vector<std::pair<OUString, int>>& rHeaders = xConfigItem->GetDefaultAddressHeaders();
        OUString sEMailColumn = rHeaders[MM_PART_E_MAIL].first;
        Sequence< OUString> aAssignment = xConfigItem->GetColumnAssignment(xConfigItem->GetCurrentDBData());
        if (aAssignment.getLength() > MM_PART_E_MAIL && !aAssignment[MM_PART_E_MAIL].isEmpty())
            sEMailColumn = aAssignment[MM_PART_E_MAIL];
        m_pMailToLB->SelectEntry(sEMailColumn);

        // HTML format pre-selected
        m_pSendAsLB->SelectEntryPos(3);
        SendTypeHdl_Impl(*m_pSendAsLB);
    }
}

IMPL_LINK(SwMMResultSaveDialog, DocumentSelectionHdl_Impl, Button*, pButton, void)
{
    bool bEnableFromTo = pButton == m_pFromRB;
    m_pFromNF->Enable(bEnableFromTo);
    m_pToFT->Enable(bEnableFromTo);
    m_pToNF->Enable(bEnableFromTo);
}

IMPL_LINK(SwMMResultPrintDialog, DocumentSelectionHdl_Impl, Button*, pButton, void)
{
    bool bEnableFromTo = pButton == m_pFromRB;
    m_pFromNF->Enable(bEnableFromTo);
    m_pToFT->Enable(bEnableFromTo);
    m_pToNF->Enable(bEnableFromTo);
}

IMPL_LINK(SwMMResultEmailDialog, DocumentSelectionHdl_Impl, Button*, pButton, void)
{
    bool bEnableFromTo = pButton == m_pFromRB;
    m_pFromNF->Enable(bEnableFromTo);
    m_pToFT->Enable(bEnableFromTo);
    m_pToNF->Enable(bEnableFromTo);
}

IMPL_LINK(SwMMResultEmailDialog, CopyToHdl_Impl, Button*, pButton, void)
{
    ScopedVclPtrInstance< SwCopyToDialog > pDlg(pButton);
    pDlg->SetCC(m_sCC );
    pDlg->SetBCC(m_sBCC);
    if(RET_OK == pDlg->Execute())
    {
        m_sCC =     pDlg->GetCC() ;
        m_sBCC =    pDlg->GetBCC();
    }
}

IMPL_LINK_NOARG(SwMMResultSaveDialog, SaveCancelHdl_Impl, Button*, void)
{
    m_bCancelSaving = true;
}

namespace {

int documentStartPageNumber(SwMailMergeConfigItem* pConfigItem, int document)
{
    SwView* pTargetView = pConfigItem->GetTargetView();
    assert( pTargetView );
    SwCursorShell& shell = pTargetView->GetWrtShell();
    const SwDocMergeInfo& info = pConfigItem->GetDocumentMergeInfo(document);
    sal_uInt16 page, dummy;
    shell.Push();
    shell.GotoMark( info.startPageInTarget );
    shell.GetPageNum( page, dummy );
    shell.Pop(SwCursorShell::PopMode::DeleteCurrent);
    return page;
}

int documentEndPageNumber(SwMailMergeConfigItem* pConfigItem, int document)
{
    SwView* pTargetView = pConfigItem->GetTargetView();
    assert( pTargetView );
    SwWrtShell& shell = pTargetView->GetWrtShell();
    if (document < int(pConfigItem->GetMergedDocumentCount()) - 1)
    {
        // Go to the page before the starting page of the next merged document.
        const SwDocMergeInfo& info = pConfigItem->GetDocumentMergeInfo( document + 1 );
        sal_uInt16 page, dummy;
        shell.Push();
        shell.GotoMark( info.startPageInTarget );
        shell.EndPrvPg();
        shell.GetPageNum( page, dummy );
        shell.Pop(SwCursorShell::PopMode::DeleteCurrent);
        return page;
    }
    else
    {   // This is the last merged document, so it ends on the page at which the document ends.
        sal_uInt16 page, dummy;
        shell.Push();
        shell.SttEndDoc( false ); // go to doc end
        shell.GetPageNum( page, dummy );
        shell.Pop(SwCursorShell::PopMode::DeleteCurrent);
        return page;
    }
}

void endDialog(Button* pButton)
{
    vcl::Window* pParent = getNonLayoutParent(pButton);
    Dialog *pDialog = dynamic_cast<Dialog*>(pParent);

    if (pDialog && pDialog->IsInExecute())
        pDialog->EndDialog(RET_OK);
}

} // anonymous namespace

IMPL_LINK(SwMMResultSaveDialog, SaveOutputHdl_Impl, Button*, pButton, void)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);
    if (!xConfigItem->GetTargetView())
        SwDBManager::PerformMailMerge(pView);

    SwView* pTargetView = xConfigItem->GetTargetView();
    assert(pTargetView);

    if(m_pSaveAsOneRB->IsChecked())
    {
        OUString sFilter;
        const OUString sPath = SwMailMergeHelper::CallSaveAsDialog(sFilter);
        if (sPath.isEmpty())
        {
            // just return back to the dialog
            return;
        }
        uno::Sequence< beans::PropertyValue > aValues(1);
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = "FilterName";
        pValues[0].Value <<= sFilter;

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
        sal_uInt32 nBegin = 0;
        sal_uInt32 nEnd = 0;
        sal_uInt32 documentCount = xConfigItem->GetMergedDocumentCount();

        if(m_pSaveIndividualRB->IsChecked())
        {
            nBegin = 0;
            nEnd = documentCount;
        }
        else
        {
            nBegin  = static_cast< sal_Int32 >(m_pFromNF->GetValue() - 1);
            nEnd    = static_cast< sal_Int32 >(m_pToNF->GetValue());
            if(nEnd > documentCount)
                nEnd = documentCount;
        }
        OUString sFilter;
        OUString sPath = SwMailMergeHelper::CallSaveAsDialog(sFilter);
        if (sPath.isEmpty())
        {
            // just return back to the dialog
            return;
        }
        OUString sTargetTempURL = URIHelper::SmartRel2Abs(
            INetURLObject(), utl::TempFile::CreateTempName(),
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
        ScopedVclPtrInstance< PrintMonitor > aSaveMonitor(this, false, PrintMonitor::MONITOR_TYPE_SAVE);
        aSaveMonitor->m_pDocName->SetText(pSourceView->GetDocShell()->GetTitle(22));
        aSaveMonitor->SetCancelHdl(LINK(this, SwMMResultSaveDialog, SaveCancelHdl_Impl));
        aSaveMonitor->m_pPrinter->SetText( INetURLObject( sPath ).getFSysPath( FSysStyle::Detect ) );
        m_bCancelSaving = false;
        aSaveMonitor->Show();

        for(sal_uInt32 nDoc = nBegin; nDoc < nEnd && !m_bCancelSaving; ++nDoc)
        {
            INetURLObject aURL(sPath);
            OUString sExtension = aURL.getExtension();
            if (sExtension.isEmpty())
            {
                sExtension = pSfxFlt->GetWildcard().getGlob().getToken(1, '.');
                sPath += "." + sExtension;
            }
            OUString sStat = SwResId(STR_STATSTR_LETTER) + " " + OUString::number( nDoc );
            aSaveMonitor->m_pPrintInfo->SetText(sStat);

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

            pTargetView->GetWrtShell().PastePages(pTempView->GetWrtShell(),
                documentStartPageNumber(xConfigItem.get(), nDoc), documentEndPageNumber(xConfigItem.get(), nDoc));
            pTargetView->GetWrtShell().EndAction();
            //then save it
            OUString sOutPath = aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
            OUString sCounter = "_" + OUString::number(nDoc);
            sOutPath = sOutPath.replaceAt( sOutPath.getLength() - sExtension.getLength() - 1, 0, sCounter);

            while(true)
            {
                //time for other slots is needed
                for(sal_Int16 r = 0; r < 10; ++r)
                    Application::Reschedule();
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
                    ScopedVclPtrInstance< SwSaveWarningBox_Impl > aWarning( pButton, sOutPath );
                    if(RET_OK == aWarning->Execute())
                        sOutPath = aWarning->GetFileName();
                    else
                    {
                        xTempDocShell->DoClose();
                        endDialog(pButton);
                        return;
                    }
                }
                else
                {
                    xTempDocShell->DoClose();
                    endDialog(pButton);
                    break;
                }
            }
        }
        ::osl::File::remove( sTargetTempURL );
    }

    endDialog(pButton);
}

IMPL_LINK(SwMMResultPrintDialog, PrinterChangeHdl_Impl, ListBox&, rBox, void)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);
    if (rBox.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( rBox.GetSelectEntry(), false );

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

        m_pPrinterSettingsPB->Enable( m_pTempPrinter->HasSupport( PrinterSupport::SetupDialog ) );
    }
    else
        m_pPrinterSettingsPB->Disable();

    xConfigItem->SetSelectedPrinter(rBox.GetSelectEntry());
}

IMPL_LINK(SwMMResultPrintDialog, PrintHdl_Impl, Button*, pButton, void)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);
    if(!xConfigItem->GetTargetView())
        SwDBManager::PerformMailMerge(pView);

    SwView* pTargetView = xConfigItem->GetTargetView();
    assert(pTargetView);

    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = 0;
    sal_uInt32 documentCount = xConfigItem->GetMergedDocumentCount();

    if(m_pPrintAllRB->IsChecked())
    {
        nBegin = 0;
        nEnd = documentCount;
    }
    else
    {
        nBegin  = static_cast< sal_Int32 >(m_pFromNF->GetValue() - 1);
        nEnd    = static_cast< sal_Int32 >(m_pToNF->GetValue());
        if(nEnd > documentCount)
            nEnd = documentCount;
    }

    OUString sPages(OUString::number(documentStartPageNumber(xConfigItem.get(), nBegin)));
    sPages += " - ";
    sPages += OUString::number(documentEndPageNumber(xConfigItem.get(), nEnd - 1));

    pTargetView->SetMailMergeConfigItem(xConfigItem);
    if(m_pTempPrinter)
    {
        SfxPrinter *const pDocumentPrinter = pTargetView->GetWrtShell()
            .getIDocumentDeviceAccess().getPrinter(true);
        pDocumentPrinter->SetPrinterProps(m_pTempPrinter);
        // this should be able to handle setting its own printer
        pTargetView->SetPrinter(pDocumentPrinter);
    }

    SfxObjectShell* pObjSh = pTargetView->GetViewFrame()->GetObjectShell();
    SfxGetpApp()->NotifyEvent(SfxEventHint(SfxEventHintId::SwMailMerge, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE), pObjSh));

    uno::Sequence < beans::PropertyValue > aProps( 2 );
    aProps[0]. Name = "MonitorVisible";
    aProps[0].Value <<= true;
    aProps[1]. Name = "Pages";
    aProps[1]. Value <<= sPages;

    pTargetView->ExecPrint( aProps, false, true );
    SfxGetpApp()->NotifyEvent(SfxEventHint(SfxEventHintId::SwMailMergeEnd, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE_END), pObjSh));

    endDialog(pButton);
}

IMPL_LINK(SwMMResultPrintDialog, PrinterSetupHdl_Impl, Button*, pButton, void)
{
    if( !m_pTempPrinter )
        PrinterChangeHdl_Impl(*m_pPrinterLB);
    if(m_pTempPrinter)
        m_pTempPrinter->Setup(pButton);
}

IMPL_LINK(SwMMResultEmailDialog, SendTypeHdl_Impl, ListBox&, rBox, void)
{
    sal_uLong nDocType = reinterpret_cast<sal_uLong>(rBox.GetSelectEntryData());
    bool bEnable = MM_DOCTYPE_HTML != nDocType && MM_DOCTYPE_TEXT != nDocType;
    m_pSendAsPB->Enable( bEnable );
    m_pAttachmentGroup->Enable( bEnable );
    if(bEnable)
    {
        //add the correct extension
        OUString sAttach(m_pAttachmentED->GetText());
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
            m_pAttachmentED->SetText(sAttach);
        }
    }
}

IMPL_LINK(SwMMResultEmailDialog, SendAsHdl_Impl, Button*, pButton, void)
{
    VclPtr<SwMailBodyDialog> pDlg = VclPtr<SwMailBodyDialog>::Create(pButton);
    pDlg->SetBody(m_sBody);
    if(RET_OK == pDlg->Execute())
    {
        m_sBody = pDlg->GetBody();
    }
}

// Send documents as e-mail
IMPL_LINK(SwMMResultEmailDialog, SendDocumentsHdl_Impl, Button*, pButton, void)
{
    SwView* pView = ::GetActiveView();
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    assert(xConfigItem);
    if (!xConfigItem->GetTargetView())
        SwDBManager::PerformMailMerge(pView);

    //get the composed document
    SwView* pTargetView = xConfigItem->GetTargetView();
    SAL_WARN_IF(!pTargetView, "sw.ui", "No TargetView in SwMailMergeConfigItem");

    if (xConfigItem->GetMailServer().isEmpty() ||
            !SwMailMergeHelper::CheckMailAddress(xConfigItem->GetMailAddress()) )
    {
        ScopedVclPtrInstance< QueryBox > aQuery(pButton, WB_YES_NO_CANCEL, m_sConfigureMail);
        sal_uInt16 nRet = aQuery->Execute();
        if (RET_YES == nRet )
        {
            SwView* pConfigView = pTargetView ? pTargetView : pView;
            SfxAllItemSet aSet(pConfigView->GetPool());
            ScopedVclPtrInstance< SwMailConfigDlg > pDlg(pButton, aSet);
            nRet = pDlg->Execute();
        }

        if(nRet != RET_OK && nRet != RET_YES)
            return; // back to the dialog
    }
    //add the documents
    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = 0;
    if(m_pSendAllRB->IsChecked())
    {
        nBegin = 0;
        nEnd = xConfigItem->GetMergedDocumentCount();
    }
    else
    {
        nBegin  = static_cast< sal_Int32 >(m_pFromNF->GetValue() - 1);
        nEnd    = static_cast< sal_Int32 >(m_pToNF->GetValue());
        if(nEnd > xConfigItem->GetMergedDocumentCount())
            nEnd = xConfigItem->GetMergedDocumentCount();
    }
    bool bAsBody = false;
    rtl_TextEncoding eEncoding = ::osl_getThreadTextEncoding();
    SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
    std::shared_ptr<const SfxFilter> pSfxFlt;
    sal_uLong nDocType = reinterpret_cast<sal_uLong>(m_pSendAsLB->GetSelectEntryData());
    OUString sExtension = lcl_GetExtensionForDocType(nDocType);
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO:
        {
            //Make sure we don't pick e.g. the flat xml filter
            //for this format
            pSfxFlt = SwIoSystem::GetFilterOfFormat(
                FILTER_XML,
                SwDocShell::Factory().GetFilterContainer() );
        }
        break;
        case MM_DOCTYPE_PDF:
        {
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                "writer_pdf_Export",
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
            SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
            eEncoding = rHtmlOptions.GetTextEncoding();
        }
        break;
        case MM_DOCTYPE_TEXT:
        {
            bAsBody = true;
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                "Text (encoded)", SfxFilterFlags::EXPORT);
        }
        break;
    }
    if(!pSfxFlt)
        pSfxFlt = pFilterContainer->GetFilter4Extension(sExtension, SfxFilterFlags::EXPORT);

    if(!pSfxFlt)
    {
        endDialog(pButton);
        return;
    }
    OUString sMimeType = pSfxFlt->GetMimeType();

    if(m_pSubjectED->GetText().isEmpty())
    {
        ScopedVclPtrInstance<SwSendQueryBox_Impl> aQuery(pButton, "SubjectDialog",
                                                         "modules/swriter/ui/subjectdialog.ui");
        aQuery->SetIsEmptyTextAllowed(true);
        aQuery->SetValue(m_sNoSubjectST);
        if(RET_OK == aQuery->Execute())
        {
            if(aQuery->GetValue() != m_sNoSubjectST)
                m_pSubjectED->SetText(aQuery->GetValue());
        }
        else
            return; // back to the dialog
    }
    if(!bAsBody && m_pAttachmentED->GetText().isEmpty())
    {
        ScopedVclPtrInstance<SwSendQueryBox_Impl> aQuery(pButton, "AttachNameDialog",
                                                         "modules/swriter/ui/attachnamedialog.ui");
        aQuery->SetIsEmptyTextAllowed(false);
        if(RET_OK == aQuery->Execute())
        {
            OUString sAttach(aQuery->GetValue());
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sAttach, '.');
            if (2 > nTokenCount)
            {
                sAttach += ".";
                ++nTokenCount;
            }
            sAttach = comphelper::string::setToken(sAttach, nTokenCount - 1, '.', lcl_GetExtensionForDocType(
                     reinterpret_cast<sal_uLong>(m_pSendAsLB->GetSelectEntryData())));
            m_pAttachmentED->SetText(sAttach);
        }
        else
            return; // back to the dialog
    }
    OUString sEMailColumn = m_pMailToLB->GetSelectEntry();
    OSL_ENSURE( !sEMailColumn.isEmpty(), "No email column selected");
    Reference< sdbcx::XColumnsSupplier > xColsSupp( xConfigItem->GetResultSet(), UNO_QUERY);
    Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : nullptr;
    if(sEMailColumn.isEmpty() || !xColAccess.is() || !xColAccess->hasByName(sEMailColumn))
    {
        endDialog(pButton);
        return;
    }

    OUString sFilterOptions;
    if(MM_DOCTYPE_TEXT == nDocType)
    {
        SwAsciiOptions aOpt;
        sal_uInt16 nAppScriptType = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
        sal_uInt16 nWhich = GetWhichOfScript( RES_CHRATR_LANGUAGE, nAppScriptType);
        aOpt.SetLanguage( static_cast<const SvxLanguageItem&>(pTargetView->GetWrtShell().
                            GetDefault( nWhich )).GetLanguage());
        aOpt.SetParaFlags( LINEEND_CR );
        aOpt.WriteUserData( sFilterOptions );
    }
    else if(MM_DOCTYPE_HTML == nDocType)
    {
        sFilterOptions = "EmbedImages";
    }
    OUString sTargetTempURL = URIHelper::SmartRel2Abs(
        INetURLObject(), utl::TempFile::CreateTempName(),
        URIHelper::GetMaybeFileHdl());
    std::shared_ptr<const SfxFilter> pTargetSfxFlt = SwIoSystem::GetFilterOfFormat(
            FILTER_XML,
            SwDocShell::Factory().GetFilterContainer() );

    uno::Sequence< beans::PropertyValue > aValues(1);
    beans::PropertyValue* pValues = aValues.getArray();
    pValues[0].Name = "FilterName";
    pValues[0].Value <<= pTargetSfxFlt->GetFilterName();

    uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
    xStore->storeToURL( sTargetTempURL, aValues   );

    //create the send dialog
    VclPtr<SwSendMailDialog> pDlg = VclPtr<SwSendMailDialog>::Create(pButton, *xConfigItem);
    pDlg->ShowDialog();
    //help to force painting the dialog
    //TODO/CLEANUP
    //predetermined breaking point
    for ( sal_Int16 i = 0; i < 25; i++)
        Application::Reschedule();
    for(sal_uInt32 nDoc = nBegin; nDoc < nEnd; ++nDoc)
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
        pTargetView->GetWrtShell().PastePages(pTempView->GetWrtShell(),
            documentStartPageNumber(xConfigItem.get(), nDoc), documentEndPageNumber(xConfigItem.get(), nDoc));
        pTargetView->GetWrtShell().EndAction();

        //then save it
        SfxStringItem aName(SID_FILE_NAME,
                URIHelper::SmartRel2Abs(
                    INetURLObject(), utl::TempFile::CreateTempName(),
                    URIHelper::GetMaybeFileHdl()) );

        {
            bool withFilterOptions = MM_DOCTYPE_TEXT == nDocType || MM_DOCTYPE_HTML == nDocType;
            uno::Sequence< beans::PropertyValue > aFilterValues(withFilterOptions ? 2 : 1);
            beans::PropertyValue* pFilterValues = aFilterValues.getArray();
            pFilterValues[0].Name = "FilterName";
            pFilterValues[0].Value <<= pSfxFlt->GetFilterName();
            if(withFilterOptions)
            {
                pFilterValues[1].Name = "FilterOptions";
                pFilterValues[1].Value <<= sFilterOptions;
            }

            uno::Reference< frame::XStorable > xTempStore( pTempView->GetDocShell()->GetModel(), uno::UNO_QUERY);
            xTempStore->storeToURL( aName.GetValue(), aFilterValues );
        }
        xTempDocShell->DoClose();

        sal_Int32 nTarget = xConfigItem->MoveResultSet(rInfo.nDBRow);
        OSL_ENSURE( nTarget == rInfo.nDBRow, "row of current document could not be selected");
        (void)nTarget;
        OSL_ENSURE( !sEMailColumn.isEmpty(), "No email column selected");
        OUString sEMail = lcl_GetColumnValueOf(sEMailColumn, xColAccess);
        SwMailDescriptor aDesc;
        aDesc.sEMail = sEMail;
        OUString sBody;
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
                OString sLine;
                bool bDone = pInStream->ReadLine( sLine );
                while ( bDone )
                {
                    sBody += OStringToOUString(sLine, eEncoding);
                    sBody += "\n";
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
            OUString sAttachment(m_pAttachmentED->GetText());
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sAttachment, '.');
            if (2 > nTokenCount)
            {
                sAttachment += ".";
                sAttachment = comphelper::string::setToken(sAttachment, nTokenCount, '.', sExtension);
            }
            else if (sAttachment.getToken( nTokenCount - 1, '.') != sExtension)
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
                sBody = sGreeting + sBody;
            }
        }
        aDesc.sBodyContent = sBody;
        if(MM_DOCTYPE_HTML == nDocType)
        {
            aDesc.sBodyMimeType = "text/html; charset=" +
                OUString::createFromAscii(rtl_getBestMimeCharsetFromTextEncoding( eEncoding ));
        }
        else
            aDesc.sBodyMimeType = "text/plain; charset=UTF-8; format=flowed";

        aDesc.sSubject = m_pSubjectED->GetText();
        aDesc.sCC = m_sCC;
        aDesc.sBCC = m_sBCC;
        pDlg->AddDocument( aDesc );
        //help to force painting the dialog
        for ( sal_Int16 i = 0; i < 25; i++)
            Application::Reschedule();
        //stop creating of data when dialog has been closed
        if(!pDlg->IsVisible())
        {
            break;
        }
    }
    pDlg->EnableDestruction();
    ::osl::File::remove( sTargetTempURL );

    endDialog(pButton);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
