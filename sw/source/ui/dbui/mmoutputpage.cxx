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

#include <mmoutputpage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <mailmergechildwindow.hxx>
#include <mailconfigpage.hxx>
#include <swmessdialog.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <hintids.hxx>
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
#include <mmgreetingspage.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <dbmgr.hxx>
#include <swunohelper.hxx>
#include <osl/mutex.hxx>
#include <shellio.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/event.hxx>
#include <swevent.hxx>
#include <dbui.hxx>
#include <dbui.hrc>
#include <helpid.h>
#include <sfx2/app.hxx>
#include <statstr.hrc>
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
    DECL_LINK( ModifyHdl, Edit*);
public:
    SwSaveWarningBox_Impl(Window* pParent, const OUString& rFileName);

    OUString        GetFileName() const
    {
        return m_pEdit->GetText();
    }
};

class SwSendQueryBox_Impl : public SwMessageAndEditDialog
{
    bool            bIsEmptyAllowed;
    DECL_LINK( ModifyHdl, Edit*);
public:
    SwSendQueryBox_Impl(Window* pParent, const OString& rID,
        const OUString& rUIXMLDescription);

    void SetValue(const OUString& rSet)
    {
        m_pEdit->SetText(rSet);
        ModifyHdl(m_pEdit);
    }

    OUString GetValue() const
    {
        return m_pEdit->GetText();
    }

    void SetIsEmptyTextAllowed(bool bSet)
    {
        bIsEmptyAllowed = bSet;
        ModifyHdl(m_pEdit);
    }
};

SwSaveWarningBox_Impl::SwSaveWarningBox_Impl(Window* pParent, const OUString& rFileName)
    : SwMessageAndEditDialog(pParent, "AlreadyExistsDialog",
        "modules/swriter/ui/alreadyexistsdialog.ui")
{
    m_pEdit->SetText(rFileName);
    m_pEdit->SetModifyHdl(LINK(this, SwSaveWarningBox_Impl, ModifyHdl));

    INetURLObject aTmp(rFileName);
    m_pPrimaryMessage->SetText(m_pPrimaryMessage->GetText().replaceAll("%1", aTmp.getName(
            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET)));

    ModifyHdl(m_pEdit);
}

IMPL_LINK( SwSaveWarningBox_Impl, ModifyHdl, Edit*, pEdit)
{
    m_pOKPB->Enable(!pEdit->GetText().isEmpty());
    return 0;
}

SwSendQueryBox_Impl::SwSendQueryBox_Impl(Window* pParent, const OString& rID,
        const OUString& rUIXMLDescription)
    : SwMessageAndEditDialog(pParent, rID, rUIXMLDescription)
    , bIsEmptyAllowed(true)
{
    m_pImageIM->SetImage(QueryBox::GetStandardImage());
    m_pEdit->SetModifyHdl(LINK(this, SwSendQueryBox_Impl, ModifyHdl));
    ModifyHdl(m_pEdit);
}

IMPL_LINK( SwSendQueryBox_Impl, ModifyHdl, Edit*, pEdit)
{
    m_pOKPB->Enable(bIsEmptyAllowed  || !pEdit->GetText().isEmpty());
    return 0;
}

class SwCopyToDialog : public SfxModalDialog
{
    Edit* m_pCCED;
    Edit* m_pBCCED;

public:
    SwCopyToDialog(Window* pParent)
        : SfxModalDialog(pParent, "CCDialog",
            "modules/swriter/ui/ccdialog.ui")
    {
        get(m_pCCED, "cc");
        get(m_pBCCED, "bcc");
    }

    OUString GetCC() {return m_pCCED->GetText();}
    void SetCC(const OUString& rSet) {m_pCCED->SetText(rSet);}

    OUString GetBCC() {return m_pBCCED->GetText();}
    void SetBCC(const OUString& rSet) {m_pBCCED->SetText(rSet);}
};

SwMailMergeOutputPage::SwMailMergeOutputPage(SwMailMergeWizard* _pParent)
    : svt::OWizardPage(_pParent, "MMOutputPage",
        "modules/swriter/ui/mmoutputpage.ui")
    , m_sSaveStartST(SW_RES(ST_SAVESTART))
    , m_sSaveMergedST(SW_RES(ST_SAVEMERGED))
    , m_sPrintST(SW_RES(ST_PRINT))
    , m_sSendMailST(SW_RES(ST_SENDMAIL))
    , m_sDefaultAttachmentST(SW_RES(ST_DEFAULTATTACHMENT))
    , m_sNoSubjectST(SW_RES(ST_NOSUBJECT))
    , m_sConfigureMail(SW_RES(ST_CONFIGUREMAIL))
    , m_bCancelSaving(false)
    , m_pWizard(_pParent)
    , m_pTempPrinter(0)
{
    get(m_pSaveStartDocRB, "savestarting");
    get(m_pSaveMergedDocRB, "savemerged");
    get(m_pPrintRB, "printmerged");
    get(m_pSendMailRB, "sendmerged");
    get(m_pSeparator, "frame");
    get(m_pSaveStartDocPB, "savestartingdoc");
    get(m_pSaveAsOneRB, "singlerb");
    get(m_pSaveIndividualRB, "individualrb");
    get(m_pPrintAllRB, "printallrb");
    get(m_pSendAllRB, "sendallrb");
    get(m_pFromRB, "fromrb");
    get(m_pFromNF, "from-nospin");
    get(m_pToFT, "toft");
    get(m_pToNF, "to-nospin");
    get(m_pSaveNowPB, "savenow");
    get(m_pPrinterFT, "printerft");
    get(m_pPrinterLB, "printers");
    m_pPrinterLB->SetStyle(m_pPrinterLB->GetStyle() | WB_SORT);
    get(m_pPrinterSettingsPB, "printersettings");
    get(m_pPrintNowPB, "printnow");
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
    get(m_pSendDocumentsPB, "sendnow");

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    // #i51949# hide e-Mail option if e-Mail is not supported
    if(!rConfigItem.IsMailAvailable())
        m_pSendMailRB->Hide();

    Link aLink = LINK(this, SwMailMergeOutputPage, OutputTypeHdl_Impl);
    m_pSaveStartDocRB->SetClickHdl(aLink);
    m_pSaveMergedDocRB->SetClickHdl(aLink);
    m_pPrintRB->SetClickHdl(aLink);
    m_pSendMailRB->SetClickHdl(aLink);
    m_pSaveStartDocRB->Check();
    m_pPrintAllRB->Check();
    m_pSaveAsOneRB->Check();
    m_pSendAllRB->Check();

    m_pSaveStartDocPB->SetClickHdl(LINK(this, SwMailMergeOutputPage, SaveStartHdl_Impl));
    m_pSaveNowPB->SetClickHdl(LINK(this, SwMailMergeOutputPage, SaveOutputHdl_Impl));
    m_pPrinterLB->SetSelectHdl(LINK(this, SwMailMergeOutputPage, PrinterChangeHdl_Impl));
    m_pPrintNowPB->SetClickHdl(LINK(this, SwMailMergeOutputPage, PrintHdl_Impl));
    m_pPrinterSettingsPB->SetClickHdl(LINK(this, SwMailMergeOutputPage, PrinterSetupHdl_Impl));

    m_pSendAsPB->SetClickHdl(LINK(this, SwMailMergeOutputPage, SendAsHdl_Impl)),
    m_pSendDocumentsPB->SetClickHdl(LINK(this, SwMailMergeOutputPage, SendDocumentsHdl_Impl)),
    m_pSendAsLB->SetSelectHdl(LINK(this, SwMailMergeOutputPage, SendTypeHdl_Impl));

    OutputTypeHdl_Impl(m_pSaveStartDocRB);

    m_pCopyToPB->SetClickHdl(LINK(this, SwMailMergeOutputPage, CopyToHdl_Impl));

    m_pSaveAsOneRB->SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    m_pSaveIndividualRB->SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    m_pPrintAllRB->SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    m_pSendAllRB->SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));

    m_pFromRB->SetClickHdl(LINK(this, SwMailMergeOutputPage, DocumentSelectionHdl_Impl));
    //#i63267# printing might be disabled
    m_pPrintRB->Enable(!Application::GetSettings().GetMiscSettings().GetDisablePrinting());
}

SwMailMergeOutputPage::~SwMailMergeOutputPage()
{
    delete m_pTempPrinter;
}

void SwMailMergeOutputPage::ActivatePage()
{
    //fill printer ListBox
    const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
    unsigned int nCount = rPrinters.size();
    if ( nCount )
    {
        for( unsigned int i = 0; i < nCount; i++ )
        {
            m_pPrinterLB->InsertEntry( rPrinters[i] );
        }

    }
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();

    SwView* pTargetView = rConfigItem.GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(pTargetView)
    {
        SfxPrinter* pPrinter = pTargetView->GetWrtShell().getIDocumentDeviceAccess()->getPrinter( true );
        m_pPrinterLB->SelectEntry( pPrinter->GetName() );
        m_pToNF->SetValue( rConfigItem.GetMergedDocumentCount() );
        m_pToNF->SetMax( rConfigItem.GetMergedDocumentCount() );
    }
    m_pPrinterLB->SelectEntry( rConfigItem.GetSelectedPrinter() );

    SwView* pSourceView = rConfigItem.GetSourceView();
    OSL_ENSURE(pSourceView, "no source view exists");
    if(pSourceView)
    {
        SwDocShell* pDocShell = pSourceView->GetDocShell();
        if ( pDocShell->HasName() )
        {
            INetURLObject aTmp( pDocShell->GetMedium()->GetName() );
            m_pAttachmentED->SetText(aTmp.getName(
                    INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ));
        }
    }
}

bool SwMailMergeOutputPage::canAdvance() const
{
    return false;
}

IMPL_LINK(SwMailMergeOutputPage, OutputTypeHdl_Impl, RadioButton*, pButton)
{
    Window* aControls[] =
    {
        m_pSaveStartDocPB,
        m_pSaveAsOneRB, m_pSaveIndividualRB,
        m_pFromRB, m_pFromNF, m_pToFT, m_pToNF,
        m_pSaveNowPB,
        m_pPrinterFT, m_pPrinterLB, m_pPrinterSettingsPB, m_pPrintAllRB,
        m_pPrintNowPB,
        m_pMailToFT, m_pMailToLB, m_pCopyToPB,
        m_pSubjectFT, m_pSubjectED,
        m_pSendAsFT, m_pSendAsLB, m_pSendAsPB,
        m_pAttachmentGroup,
        m_pSendAllRB, m_pSendDocumentsPB,
        0
    };
    SetUpdateMode(true);
    Window** pControl = aControls;
    do
    {
        (*pControl)->Show(false);

    } while(*(++pControl));

    if (m_pSaveStartDocRB == pButton)
    {
        m_pSaveStartDocPB->Show();
        m_pSeparator->set_label(m_sSaveStartST);

    }
    else if (m_pSaveMergedDocRB == pButton)
    {
        Control* aSaveMergedControls[] =
        {
            m_pSaveAsOneRB, m_pSaveIndividualRB,
            m_pFromRB, m_pFromNF, m_pToFT, m_pToNF,
            m_pSaveNowPB,
            0
        };
        Control** pSaveMergeControl = aSaveMergedControls;
        do
        {
            (*pSaveMergeControl)->Show(true);

        } while(*(++pSaveMergeControl));
        if(!m_pFromRB->IsChecked() && !m_pSaveAsOneRB->IsChecked())
        {
            m_pSaveIndividualRB->Check();
        }
        m_pSeparator->set_label(m_sSaveMergedST);
    }
    else if (m_pPrintRB == pButton)
    {
        Control* aPrintControls[] =
        {
            m_pFromRB, m_pFromNF, m_pToFT, m_pToNF,
            m_pPrinterFT, m_pPrinterLB, m_pPrinterSettingsPB, m_pPrintAllRB,
            m_pPrintNowPB,
            0
        };
        Control** pPrinterControl = aPrintControls;
        do
        {
            (*pPrinterControl)->Show(true);

        } while(*(++pPrinterControl));
        if(!m_pFromRB->IsChecked())
            m_pPrintAllRB->Check();

        m_pSeparator->set_label(m_sPrintST);
    }
    else
    {
        Window* aMailControls[] =
        {
            m_pFromRB, m_pFromNF, m_pToFT, m_pToNF,
            m_pMailToFT, m_pMailToLB, m_pCopyToPB,
            m_pSubjectFT, m_pSubjectED,
            m_pSendAsFT, m_pSendAsLB, m_pSendAsPB,
            m_pAttachmentGroup,
            m_pSendAllRB, m_pSendDocumentsPB, 0
        };
        Window** pMailControl = aMailControls;
        do
        {
            (*pMailControl)->Show(true);

        } while(*(++pMailControl));

        if(!m_pFromRB->IsChecked())
            m_pSendAllRB->Check();
        if(m_pAttachmentED->GetText().isEmpty())
        {
            OUString sAttach( m_sDefaultAttachmentST );
            sAttach += ".";
            sAttach += lcl_GetExtensionForDocType(
                        (sal_uLong)m_pSendAsLB->GetEntryData(m_pSendAsLB->GetSelectEntryPos()));
            m_pAttachmentED->SetText( sAttach );

        }
        m_pSeparator->set_label(m_sSendMailST);
        //fill mail address ListBox
        if(!m_pMailToLB->GetEntryCount())
        {
            SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
            //select first column
            uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), uno::UNO_QUERY);
            //get the name of the actual columns
            uno::Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
            uno::Sequence< OUString > aFields;
            if(xColAccess.is())
                aFields = xColAccess->getElementNames();
            const OUString* pFields = aFields.getConstArray();
            for(sal_Int32 nField = 0; nField < aFields.getLength(); ++nField)
                m_pMailToLB->InsertEntry(pFields[nField]);

            m_pMailToLB->SelectEntryPos(0);
            // then select the right one - may not be available
            const ResStringArray& rHeaders = rConfigItem.GetDefaultAddressHeaders();
            OUString sEMailColumn = rHeaders.GetString( MM_PART_E_MAIL );
            Sequence< OUString> aAssignment =
                            rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
            if(aAssignment.getLength() > MM_PART_E_MAIL && !aAssignment[MM_PART_E_MAIL].isEmpty())
                sEMailColumn = aAssignment[MM_PART_E_MAIL];
            m_pMailToLB->SelectEntry(sEMailColumn);
            // HTML format pre-selected
            m_pSendAsLB->SelectEntryPos(3);
            SendTypeHdl_Impl(m_pSendAsLB);
        }
    }
    m_pFromRB->GetClickHdl().Call(m_pFromRB->IsChecked() ? m_pFromRB : 0);

    SetUpdateMode(false);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, DocumentSelectionHdl_Impl, RadioButton*, pButton)
{
    sal_Bool bEnableFromTo = pButton == m_pFromRB;
    m_pFromNF->Enable(bEnableFromTo);
    m_pToFT->Enable(bEnableFromTo);
    m_pToNF->Enable(bEnableFromTo);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, CopyToHdl_Impl, PushButton*, pButton)
{
    SwCopyToDialog* pDlg = new SwCopyToDialog(pButton);
    pDlg->SetCC(m_sCC );
    pDlg->SetBCC(m_sBCC);
    if(RET_OK == pDlg->Execute())
    {
        m_sCC =     pDlg->GetCC() ;
        m_sBCC =    pDlg->GetBCC();
    }
    delete pDlg;
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SaveStartHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    SwView* pSourceView = rConfigItem.GetSourceView();
    OSL_ENSURE( pSourceView, "source view missing");
    if(pSourceView)
    {
        SfxViewFrame* pSourceViewFrm = pSourceView->GetViewFrame();
        uno::Reference< frame::XFrame > xFrame =
                pSourceViewFrm->GetFrame().GetFrameInterface();
        xFrame->getContainerWindow()->setVisible(sal_True);
        pSourceViewFrm->GetDispatcher()->Execute(SID_SAVEDOC, SFX_CALLMODE_SYNCHRON);
        xFrame->getContainerWindow()->setVisible(sal_False);
        SwDocShell* pDocShell = pSourceView->GetDocShell();
        //if the document has been saved it's URL has to be stored for
        // later use and it can be closed now
        if(pDocShell->HasName() && !pDocShell->IsModified())
        {
            INetURLObject aURL = pDocShell->GetMedium()->GetURLObject();
            //update the attachment name
            if(m_pAttachmentED->GetText().isEmpty())
            {
                if ( pDocShell->HasName() )
                {
                    m_pAttachmentED->SetText(aURL.getName(
                            INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ));
                }
            }

            rConfigItem.AddSavedDocument(
                    aURL.GetMainURL(INetURLObject::DECODE_TO_IURI));
            pButton->Enable(false);
            m_pWizard->enableButtons(WZB_FINISH, true);
            pButton->Enable(false);

        }
    }
    return 0;
}

IMPL_LINK_NOARG(SwMailMergeOutputPage, SaveCancelHdl_Impl)
{
    m_bCancelSaving = true;
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SaveOutputHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    SwView* pTargetView = rConfigItem.GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(!pTargetView)
        return 0;

    if(m_pSaveAsOneRB->IsChecked())
    {
        OUString sFilter;
        const OUString sPath = SwMailMergeHelper::CallSaveAsDialog(sFilter);
        if (sPath.isEmpty())
            return 0;
        uno::Sequence< beans::PropertyValue > aValues(1);
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = "FilterName";
        pValues[0].Value <<= sFilter;

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        sal_uInt32 nErrorCode = ERRCODE_NONE;
        try
        {
            xStore->storeToURL( sPath, aValues );
        }
        catch (const task::ErrorCodeIOException& rErrorEx)
        {
            nErrorCode = (sal_uInt32)rErrorEx.ErrCode;
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
        if(m_pSaveIndividualRB->IsChecked())
        {
            nBegin = 0;
            nEnd = rConfigItem.GetMergedDocumentCount();
        }
        else
        {
            nBegin  = static_cast< sal_Int32 >(m_pFromNF->GetValue() - 1);
            nEnd    = static_cast< sal_Int32 >(m_pToNF->GetValue());
            if(nEnd > rConfigItem.GetMergedDocumentCount())
                nEnd = rConfigItem.GetMergedDocumentCount();
        }
        OUString sFilter;
        OUString sPath = SwMailMergeHelper::CallSaveAsDialog(sFilter);
        if (sPath.isEmpty())
            return 0;
        OUString sTargetTempURL = URIHelper::SmartRel2Abs(
            INetURLObject(), utl::TempFile::CreateTempName(),
            URIHelper::GetMaybeFileHdl());
        const SfxFilter *pSfxFlt = SwIoSystem::GetFilterOfFormat(
                OUString( FILTER_XML ),
                SwDocShell::Factory().GetFilterContainer() );

        uno::Sequence< beans::PropertyValue > aValues(1);
        beans::PropertyValue* pValues = aValues.getArray();
        pValues[0].Name = "FilterName";
        pValues[0].Value <<= OUString(pSfxFlt->GetFilterName());

        uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
        sal_uInt32 nErrorCode = ERRCODE_NONE;
        try
        {
            xStore->storeToURL( sTargetTempURL, aValues );
        }
        catch (const task::ErrorCodeIOException& rErrorEx)
        {
            nErrorCode = (sal_uInt32)rErrorEx.ErrCode;
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

        SwView* pSourceView = rConfigItem.GetSourceView();
        PrintMonitor aSaveMonitor(this, PrintMonitor::MONITOR_TYPE_SAVE);
        aSaveMonitor.m_pDocName->SetText(pSourceView->GetDocShell()->GetTitle(22));
        aSaveMonitor.m_pCancel->SetClickHdl(LINK(this, SwMailMergeOutputPage, SaveCancelHdl_Impl));
        aSaveMonitor.m_pPrinter->SetText( INetURLObject( sPath ).getFSysPath( INetURLObject::FSYS_DETECT ) );
        m_bCancelSaving = false;
        aSaveMonitor.Show();
        m_pWizard->enableButtons(WZB_CANCEL, false);

        for(sal_uInt32 nDoc = nBegin; nDoc < nEnd && !m_bCancelSaving; ++nDoc)
        {
            SwDocMergeInfo& rInfo = rConfigItem.GetDocumentMergeInfo(nDoc);
            INetURLObject aURL(sPath);
            OUString sExtension = aURL.getExtension();
            if (sExtension.isEmpty())
            {
                sExtension = comphelper::string::getToken(pSfxFlt->GetWildcard().getGlob(), 1, '.');
                sPath += "." + sExtension;
            }
            OUString sStat = OUString(SW_RES(STR_STATSTR_LETTER)) + " " + OUString::number( nDoc );
            aSaveMonitor.m_pPrintInfo->SetText(sStat);

            //now extract a document from the target document
            // the shell will be closed at the end, but it is more safe to use SfxObjectShellLock here
            SfxObjectShellLock xTempDocShell( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
            xTempDocShell->DoInitNew( 0 );
            SfxViewFrame* pTempFrame = SfxViewFrame::LoadHiddenDocument( *xTempDocShell, 0 );
            SwView* pTempView = static_cast<SwView*>( pTempFrame->GetViewShell() );
            pTargetView->GetWrtShell().StartAction();
            SwgReaderOption aOpt;
            aOpt.SetTxtFmts( sal_True );
            aOpt.SetFrmFmts( sal_True );
            aOpt.SetPageDescs( sal_True );
            aOpt.SetNumRules( sal_True );
            aOpt.SetMerge( sal_False );
            pTempView->GetDocShell()->LoadStylesFromFile(
                    sTargetTempURL, aOpt, sal_True );

            pTargetView->GetWrtShell().PastePages(pTempView->GetWrtShell(),
                    (sal_uInt16)rInfo.nStartPageInTarget, (sal_uInt16)rInfo.nEndPageInTarget );
            pTargetView->GetWrtShell().EndAction();
            //then save it
            OUString sOutPath = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
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
                    SwSaveWarningBox_Impl aWarning( pButton, sOutPath );
                    if(RET_OK == aWarning.Execute())
                        sOutPath = aWarning.GetFileName();
                    else
                    {
                        xTempDocShell->DoClose();
                        return 0;
                    }
                }
                else
                {
                    xTempDocShell->DoClose();
                    break;
                }
            }
        }
        ::osl::File::remove( sTargetTempURL );
    }
    m_pWizard->enableButtons(WZB_CANCEL, true);
    m_pWizard->enableButtons(WZB_FINISH, true);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, PrinterChangeHdl_Impl, ListBox*, pBox)
{
    SwView *const pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    SfxPrinter *const pDocumentPrinter = pTargetView->GetWrtShell()
        .getIDocumentDeviceAccess()->getPrinter(true);
    if (pDocumentPrinter && pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {
        const QueueInfo* pInfo = Printer::GetQueueInfo( pBox->GetSelectEntry(), false );

        if( pInfo )
        {
            if ( !m_pTempPrinter )
            {
                if ((pDocumentPrinter->GetName() == pInfo->GetPrinterName()) &&
                    (pDocumentPrinter->GetDriverName() == pInfo->GetDriver()))
                {
                    m_pTempPrinter = new Printer(pDocumentPrinter->GetJobSetup());
                }
                else
                    m_pTempPrinter = new Printer( *pInfo );
            }
            else
            {
                if( (m_pTempPrinter->GetName() != pInfo->GetPrinterName()) ||
                     (m_pTempPrinter->GetDriverName() != pInfo->GetDriver()) )
                {
                    delete m_pTempPrinter;
                    m_pTempPrinter = new Printer( *pInfo );
                }
            }
        }
        else if( ! m_pTempPrinter )
            m_pTempPrinter = new Printer();

        m_pPrinterSettingsPB->Enable( m_pTempPrinter->HasSupport( SUPPORT_SETUPDIALOG ) );
    }
    else
        m_pPrinterSettingsPB->Disable();
    m_pWizard->GetConfigItem().SetSelectedPrinter( pBox->GetSelectEntry() );

    return 0;
}

IMPL_LINK_NOARG(SwMailMergeOutputPage, PrintHdl_Impl)
{
    SwView* pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(!pTargetView)
        return 0;

    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = 0;
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(m_pPrintAllRB->IsChecked())
    {
        nBegin = 0;
        nEnd = rConfigItem.GetMergedDocumentCount();
    }
    else
    {
        nBegin  = static_cast< sal_Int32 >(m_pFromNF->GetValue() - 1);
        nEnd    = static_cast< sal_Int32 >(m_pToNF->GetValue());
        if(nEnd > rConfigItem.GetMergedDocumentCount())
            nEnd = rConfigItem.GetMergedDocumentCount();
    }
    rConfigItem.SetPrintRange( (sal_uInt16)nBegin, (sal_uInt16)nEnd );
    SwDocMergeInfo& rStartInfo = rConfigItem.GetDocumentMergeInfo(nBegin);
    SwDocMergeInfo& rEndInfo = rConfigItem.GetDocumentMergeInfo(nEnd - 1);

    OUString sPages(OUString::number( rStartInfo.nStartPageInTarget ));
    sPages += " - ";
    sPages += OUString::number(  rEndInfo.nEndPageInTarget );

    pTargetView->SetMailMergeConfigItem(&rConfigItem, 0, sal_False);
    if(m_pTempPrinter)
    {
        SfxPrinter *const pDocumentPrinter = pTargetView->GetWrtShell()
            .getIDocumentDeviceAccess()->getPrinter(true);
        pDocumentPrinter->SetPrinterProps(m_pTempPrinter);
        // this should be able to handle setting its own printer
        pTargetView->SetPrinter(pDocumentPrinter);
    }

    SfxObjectShell* pObjSh = pTargetView->GetViewFrame()->GetObjectShell();
    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE), pObjSh));
    SfxBoolItem aMergeSilent(SID_SILENT, false);
    m_pWizard->enableButtons(WZB_CANCEL, false);

    uno::Sequence < beans::PropertyValue > aProps( 2 );
    aProps[0]. Name = "MonitorVisible";
    aProps[0].Value <<= sal_True;
    aProps[1]. Name = "Pages";
    aProps[1]. Value <<= sPages;

    pTargetView->ExecPrint( aProps, false, true );
    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE_END, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE_END), pObjSh));

    pTargetView->SetMailMergeConfigItem(0, 0, sal_False);
    m_pWizard->enableButtons(WZB_CANCEL, true);
    m_pWizard->enableButtons(WZB_FINISH, true);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, PrinterSetupHdl_Impl, PushButton*, pButton)
{
    if( !m_pTempPrinter )
        PrinterChangeHdl_Impl(m_pPrinterLB);
    if(m_pTempPrinter)
        m_pTempPrinter->Setup(pButton);
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SendTypeHdl_Impl, ListBox*, pBox)
{
    sal_uLong nDocType = (sal_uLong)pBox->GetEntryData(pBox->GetSelectEntryPos());
    sal_Bool bEnable = MM_DOCTYPE_HTML != nDocType && MM_DOCTYPE_TEXT != nDocType;
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
    return 0;
}

IMPL_LINK(SwMailMergeOutputPage, SendAsHdl_Impl, PushButton*, pButton)
{
    SwMailBodyDialog* pDlg = new SwMailBodyDialog(pButton, m_pWizard);
    pDlg->SetBody(m_sBody);
    if(RET_OK == pDlg->Execute())
    {
        m_sBody = pDlg->GetBody();
    }
    return 0;
}

/*-------------------------------------------------------------------------
    Send documents as e-mail
  -----------------------------------------------------------------------*/
IMPL_LINK(SwMailMergeOutputPage, SendDocumentsHdl_Impl, PushButton*, pButton)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();

    //get the composed document
    SwView* pTargetView = rConfigItem.GetTargetView();
    OSL_ENSURE(pTargetView, "no target view exists");
    if(!pTargetView)
        return 0;

    if(rConfigItem.GetMailServer().isEmpty() ||
            !SwMailMergeHelper::CheckMailAddress(rConfigItem.GetMailAddress()) )
    {
        QueryBox aQuery(pButton, WB_YES_NO_CANCEL, m_sConfigureMail);
        sal_uInt16 nRet = aQuery.Execute();
        if(RET_YES == nRet )
        {
            SfxAllItemSet aSet(pTargetView->GetPool());
            SwMailConfigDlg* pDlg = new SwMailConfigDlg(pButton, aSet);
            nRet = pDlg->Execute();
            delete pDlg;
        }

        if(nRet != RET_OK && nRet != RET_YES)
            return 0;
    }
    //add the documents
    sal_uInt32 nBegin = 0;
    sal_uInt32 nEnd = 0;
    if(m_pSendAllRB->IsChecked())
    {
        nBegin = 0;
        nEnd = rConfigItem.GetMergedDocumentCount();
    }
    else
    {
        nBegin  = static_cast< sal_Int32 >(m_pFromNF->GetValue() - 1);
        nEnd    = static_cast< sal_Int32 >(m_pToNF->GetValue());
        if(nEnd > rConfigItem.GetMergedDocumentCount())
            nEnd = rConfigItem.GetMergedDocumentCount();
    }
    bool bAsBody = false;
    rtl_TextEncoding eEncoding = ::osl_getThreadTextEncoding();
    SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
    const SfxFilter *pSfxFlt = 0;
    sal_uLong nDocType = (sal_uLong)m_pSendAsLB->GetEntryData(m_pSendAsLB->GetSelectEntryPos());
    OUString sExtension = lcl_GetExtensionForDocType(nDocType);
    switch( nDocType )
    {
        case MM_DOCTYPE_OOO:
        {
            //Make sure we don't pick e.g. the flat xml filter
            //for this format
            pSfxFlt = SwIoSystem::GetFilterOfFormat(
                OUString( FILTER_XML ),
                SwDocShell::Factory().GetFilterContainer() );
        }
        break;
        case MM_DOCTYPE_PDF:
        {
            pSfxFlt = pFilterContainer->GetFilter4FilterName(
                OUString("writer_pdf_Export"),
                SFX_FILTER_EXPORT);
        }
        break;
        case MM_DOCTYPE_WORD:
        {
            //the method SwIOSystemGetFilterOfFormat( ) returns the template filter
            //because it uses the same user data :-(
            SfxFilterMatcher aMatcher( pFilterContainer->GetName() );
            SfxFilterMatcherIter aIter( aMatcher );
            const SfxFilter* pFilter = aIter.First();
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
                OUString("Text (encoded)"), SFX_FILTER_EXPORT);
        }
        break;
    }
    if(!pSfxFlt)
        pSfxFlt = pFilterContainer->GetFilter4Extension(sExtension, SFX_FILTER_EXPORT);

    if(!pSfxFlt)
        return 0;
    OUString sMimeType = pSfxFlt->GetMimeType();

    if(m_pSubjectED->GetText().isEmpty())
    {
        SwSendQueryBox_Impl aQuery(pButton, "SubjectDialog",
         "modules/swriter/ui/subjectdialog.ui");
        aQuery.SetIsEmptyTextAllowed(true);
        aQuery.SetValue(m_sNoSubjectST);
        if(RET_OK == aQuery.Execute())
        {
            if(aQuery.GetValue() != m_sNoSubjectST)
                m_pSubjectED->SetText(aQuery.GetValue());
        }
        else
            return 0;
    }
    if(!bAsBody && m_pAttachmentED->GetText().isEmpty())
    {
        SwSendQueryBox_Impl aQuery(pButton, "AttachNameDialog",
         "modules/swriter/ui/attachnamedialog.ui");
        aQuery.SetIsEmptyTextAllowed(false);
        if(RET_OK == aQuery.Execute())
        {
            OUString sAttach(aQuery.GetValue());
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sAttach, '.');
            if (2 > nTokenCount)
            {
                sAttach += ".";
                ++nTokenCount;
            }
            sAttach = comphelper::string::setToken(sAttach, nTokenCount - 1, '.', lcl_GetExtensionForDocType(
                     (sal_uLong)m_pSendAsLB->GetEntryData(m_pSendAsLB->GetSelectEntryPos())));
            m_pAttachmentED->SetText(sAttach);
        }
        else
            return 0;
    }
    SfxStringItem aFilterName( SID_FILTER_NAME, pSfxFlt->GetFilterName() );
    OUString sEMailColumn = m_pMailToLB->GetSelectEntry();
    OSL_ENSURE( !sEMailColumn.isEmpty(), "No email column selected");
    Reference< sdbcx::XColumnsSupplier > xColsSupp( rConfigItem.GetResultSet(), UNO_QUERY);
    Reference < container::XNameAccess> xColAccess = xColsSupp.is() ? xColsSupp->getColumns() : 0;
    if(sEMailColumn.isEmpty() || !xColAccess.is() || !xColAccess->hasByName(sEMailColumn))
        return 0;

    OUString sFilterOptions;
    if(MM_DOCTYPE_TEXT == nDocType)
    {
        SwAsciiOptions aOpt;
        sal_uInt16 nAppScriptType = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );
        sal_uInt16 nWhich = GetWhichOfScript( RES_CHRATR_LANGUAGE, nAppScriptType);
        aOpt.SetLanguage( ((SvxLanguageItem&)pTargetView->GetWrtShell().
                            GetDefault( nWhich )).GetLanguage());
        aOpt.SetParaFlags( LINEEND_CR );
        aOpt.WriteUserData( sFilterOptions );
    }
    OUString sTargetTempURL = URIHelper::SmartRel2Abs(
        INetURLObject(), utl::TempFile::CreateTempName(),
        URIHelper::GetMaybeFileHdl());
    const SfxFilter *pTargetSfxFlt = SwIoSystem::GetFilterOfFormat(
            OUString( FILTER_XML ),
            SwDocShell::Factory().GetFilterContainer() );

    uno::Sequence< beans::PropertyValue > aValues(1);
    beans::PropertyValue* pValues = aValues.getArray();
    pValues[0].Name = "FilterName";
    pValues[0].Value <<= OUString(pTargetSfxFlt->GetFilterName());

    uno::Reference< frame::XStorable > xStore( pTargetView->GetDocShell()->GetModel(), uno::UNO_QUERY);
    xStore->storeToURL( sTargetTempURL, aValues   );

    //create the send dialog
    SwSendMailDialog* pDlg = new SwSendMailDialog( pButton, rConfigItem );
    pDlg->SetDocumentCount( nEnd );
    pDlg->ShowDialog();
    //help to force painting the dialog
    //TODO/CLEANUP
    //predetermined breaking point
    for ( sal_Int16 i = 0; i < 25; i++)
        Application::Reschedule();
    for(sal_uInt32 nDoc = nBegin; nDoc < nEnd; ++nDoc)
    {
        m_pWizard->EnterWait();
        SwDocMergeInfo& rInfo = rConfigItem.GetDocumentMergeInfo(nDoc);

        //now extract a document from the target document
        // the shell will be closed at the end, but it is more safe to use SfxObjectShellLock here
        SfxObjectShellLock xTempDocShell( new SwDocShell( SFX_CREATE_MODE_STANDARD ) );
        xTempDocShell->DoInitNew( 0 );
        SfxViewFrame* pTempFrame = SfxViewFrame::LoadHiddenDocument( *xTempDocShell, 0 );
        SwView* pTempView = static_cast<SwView*>( pTempFrame->GetViewShell() );
        pTargetView->GetWrtShell().StartAction();
        SwgReaderOption aOpt;
        aOpt.SetTxtFmts( sal_True );
        aOpt.SetFrmFmts( sal_True );
        aOpt.SetPageDescs( sal_True );
        aOpt.SetNumRules( sal_True );
        aOpt.SetMerge( sal_False );
        pTempView->GetDocShell()->LoadStylesFromFile(
                sTargetTempURL, aOpt, sal_True );
        pTargetView->GetWrtShell().PastePages(pTempView->GetWrtShell(),
                (sal_uInt16)rInfo.nStartPageInTarget, (sal_uInt16)rInfo.nEndPageInTarget );
        pTargetView->GetWrtShell().EndAction();

        //then save it
        SfxStringItem aName(SID_FILE_NAME,
                URIHelper::SmartRel2Abs(
                    INetURLObject(), utl::TempFile::CreateTempName(0),
                    URIHelper::GetMaybeFileHdl()) );

        {
            uno::Sequence< beans::PropertyValue > aFilterValues(MM_DOCTYPE_TEXT == nDocType ? 2 : 1);
            beans::PropertyValue* pFilterValues = aFilterValues.getArray();
            pFilterValues[0].Name = "FilterName";
            pFilterValues[0].Value <<= OUString(pSfxFlt->GetFilterName());
            if(MM_DOCTYPE_TEXT == nDocType)
            {
                pFilterValues[1].Name = "FilterOptions";
                pFilterValues[1].Value <<= sFilterOptions;
            }

            uno::Reference< frame::XStorable > xTempStore( pTempView->GetDocShell()->GetModel(), uno::UNO_QUERY);
            xTempStore->storeToURL( aName.GetValue(), aFilterValues );
        }
        xTempDocShell->DoClose();

        sal_Int32 nTarget = rConfigItem.MoveResultSet(rInfo.nDBRow);
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
                SfxMedium aMedium( aName.GetValue(),    STREAM_READ);
                SvStream* pInStream = aMedium.GetInStream();
                if(pInStream)
                    pInStream->SetStreamCharSet( eEncoding );
                else
                {
                    OSL_FAIL("no output file created?");
                    continue;
                }
                OString sLine;
                sal_Bool bDone = pInStream->ReadLine( sLine );
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

            if(rConfigItem.IsGreetingLine(sal_True))
            {
                OUString sNameColumn = rConfigItem.GetAssignedColumn(MM_PART_LASTNAME);
                OUString sName = lcl_GetColumnValueOf(sNameColumn, xColAccess);
                OUString sGreeting;
                if(!sName.isEmpty() && rConfigItem.IsIndividualGreeting(sal_True))
                {
                    OUString sGenderColumn = rConfigItem.GetAssignedColumn(MM_PART_GENDER);
                    const OUString& sFemaleValue = rConfigItem.GetFemaleGenderValue();
                    OUString sGenderValue = lcl_GetColumnValueOf(sGenderColumn, xColAccess);
                    SwMailMergeConfigItem::Gender eGenderType = sGenderValue == sFemaleValue ?
                        SwMailMergeConfigItem::FEMALE :
                        SwMailMergeConfigItem::MALE;

                    sGreeting = SwAddressPreview::FillData(
                        rConfigItem.GetGreetings(eGenderType)
                        [rConfigItem.GetCurrentGreeting(eGenderType)],
                            rConfigItem);
                }
                else
                {
                    sGreeting =
                        rConfigItem.GetGreetings(SwMailMergeConfigItem::NEUTRAL)
                        [rConfigItem.GetCurrentGreeting(SwMailMergeConfigItem::NEUTRAL)];

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
            aDesc.sBodyMimeType =
                OUString("text/plain; charset=UTF-8; format=flowed");

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
            m_pWizard->LeaveWait();
            break;
        }
        m_pWizard->LeaveWait();
    }
    pDlg->EnableDesctruction();
    ::osl::File::remove( sTargetTempURL );

    m_pWizard->enableButtons(WZB_FINISH, true);
    //the dialog deletes itself
    //delete pDlg;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
