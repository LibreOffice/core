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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <printdata.hxx>
#include <wrtsh.hxx>
#include <dbmgr.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <mailmergehelper.hxx>
#include <mailmrge.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::ui::dialogs;

struct SwMailMergeDlg_Impl {
    uno::Reference<runtime::XFormController> xFController;
    uno::Reference<XSelectionChangeListener> xChgLstnr;
    uno::Reference<XSelectionSupplier> xSelSupp;
};

class SwXSelChgLstnr_Impl : public cppu::WeakImplHelper
    <
    view::XSelectionChangeListener
    >
{
    SwMailMergeDlg& rParent;
public:
    explicit SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg);

    virtual void SAL_CALL selectionChanged( const EventObject& aEvent ) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) override;
};

SwXSelChgLstnr_Impl::SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg) :
    rParent(rParentDlg)
{}

void SwXSelChgLstnr_Impl::selectionChanged( const EventObject&  )
{
    //call the parent to enable selection mode
    Sequence <Any> aSelection;
    if(rParent.pImpl->xSelSupp.is())
        rParent.pImpl->xSelSupp->getSelection() >>= aSelection;

    bool bEnable = aSelection.hasElements();
    rParent.m_xMarkedRB->set_sensitive(bEnable);
    if(bEnable)
        rParent.m_xMarkedRB->set_active(true);
    else if(rParent.m_xMarkedRB->get_active()) {
        rParent.m_xAllRB->set_active(true);
        rParent.m_aSelection.realloc(0);
    }
}

void SwXSelChgLstnr_Impl::disposing( const EventObject&  )
{
    OSL_FAIL("disposing");
}

SwMailMergeDlg::SwMailMergeDlg(weld::Window* pParent, SwWrtShell& rShell,
                               const OUString& rSourceName,
                               const OUString& rTableName,
                               sal_Int32 nCommandType,
                               const uno::Reference< XConnection>& _xConnection,
                               Sequence< Any > const * pSelection)
    : SfxDialogController(pParent, "modules/swriter/ui/mailmerge.ui", "MailmergeDialog")
    , pImpl(new SwMailMergeDlg_Impl)
    , rSh(rShell)
    , nMergeType(DBMGR_MERGE_EMAIL)
    , m_xBeamerWin(m_xBuilder->weld_container("beamer"))
    , m_xAllRB(m_xBuilder->weld_radio_button("all"))
    , m_xMarkedRB(m_xBuilder->weld_radio_button("selected"))
    , m_xFromRB(m_xBuilder->weld_radio_button("rbfrom"))
    , m_xFromNF(m_xBuilder->weld_spin_button("from"))
    , m_xToNF(m_xBuilder->weld_spin_button("to"))
    , m_xPrinterRB(m_xBuilder->weld_radio_button("printer"))
    , m_xMailingRB(m_xBuilder->weld_radio_button("electronic"))
    , m_xFileRB(m_xBuilder->weld_radio_button("file"))
    , m_xSingleJobsCB(m_xBuilder->weld_check_button("singlejobs"))
    , m_xPasswordCB(m_xBuilder->weld_check_button("passwd-check"))
    , m_xSaveMergedDocumentFT(m_xBuilder->weld_label("savemergeddoclabel"))
    , m_xSaveSingleDocRB(m_xBuilder->weld_radio_button("singledocument"))
    , m_xSaveIndividualRB(m_xBuilder->weld_radio_button("individualdocuments"))
    , m_xGenerateFromDataBaseCB(m_xBuilder->weld_check_button("generate"))
    , m_xColumnFT(m_xBuilder->weld_label("fieldlabel"))
    , m_xColumnLB(m_xBuilder->weld_combo_box("field"))
    , m_xPasswordFT(m_xBuilder->weld_label("passwd-label"))
    , m_xPasswordLB(m_xBuilder->weld_combo_box("passwd-combobox"))
    , m_xPathFT(m_xBuilder->weld_label("pathlabel"))
    , m_xPathED(m_xBuilder->weld_entry("path"))
    , m_xPathPB(m_xBuilder->weld_button("pathpb"))
    , m_xFilterFT(m_xBuilder->weld_label("fileformatlabel"))
    , m_xFilterLB(m_xBuilder->weld_combo_box("fileformat"))
    , m_xAddressFieldLB(m_xBuilder->weld_combo_box("address"))
    , m_xSubjectFT(m_xBuilder->weld_label("subjectlabel"))
    , m_xSubjectED(m_xBuilder->weld_entry("subject"))
    , m_xFormatFT(m_xBuilder->weld_label("mailformatlabel"))
    , m_xAttachFT(m_xBuilder->weld_label("attachmentslabel"))
    , m_xAttachED(m_xBuilder->weld_entry("attachments"))
    , m_xAttachPB(m_xBuilder->weld_button("attach"))
    , m_xFormatHtmlCB(m_xBuilder->weld_check_button("html"))
    , m_xFormatRtfCB(m_xBuilder->weld_check_button("rtf"))
    , m_xFormatSwCB(m_xBuilder->weld_check_button("swriter"))
    , m_xOkBTN(m_xBuilder->weld_button("ok"))
{
    m_xSingleJobsCB->hide(); // not supported in since cws printerpullpages anymore
    //task #97066# mailing of form letters is currently not supported
    m_xMailingRB->hide();
    m_xSubjectFT->hide();
    m_xSubjectED->hide();
    m_xFormatFT->hide();
    m_xFormatSwCB->hide();
    m_xFormatHtmlCB->hide();
    m_xFormatRtfCB->hide();
    m_xAttachFT->hide();
    m_xAttachED->hide();
    m_xAttachPB->hide();
    m_xPasswordCB->hide();
    m_xPasswordFT->hide();
    m_xPasswordLB->hide();

    uno::Reference< lang::XMultiServiceFactory > xMSF = comphelper::getProcessServiceFactory();
    if(pSelection) {
        m_aSelection = *pSelection;
        m_xBeamerWin->hide();
    } else {
        try {
            // create a frame wrapper for myself
            m_xFrame = frame::Frame::create( comphelper::getProcessComponentContext() );
            m_xFrame->initialize(m_xBeamerWin->CreateChildFrame());
        } catch (const Exception&) {
            m_xFrame.clear();
        }
        if(m_xFrame.is()) {
            URL aURL;
            aURL.Complete = ".component:DB/DataSourceBrowser";
            uno::Reference<XDispatch> xD = m_xFrame->queryDispatch(aURL,
                                           "",
                                           0x0C);
            if(xD.is()) {
                Sequence<PropertyValue> aProperties(3);
                PropertyValue* pProperties = aProperties.getArray();
                pProperties[0].Name = "DataSourceName";
                pProperties[0].Value <<= rSourceName;
                pProperties[1].Name = "Command";
                pProperties[1].Value <<= rTableName;
                pProperties[2].Name = "CommandType";
                pProperties[2].Value <<= nCommandType;
                xD->dispatch(aURL, aProperties);
                m_xBeamerWin->show();
            }
            uno::Reference<XController> xController = m_xFrame->getController();
            pImpl->xFController.set(xController, UNO_QUERY);
            if(pImpl->xFController.is()) {
                uno::Reference< awt::XControl > xCtrl = pImpl->xFController->getCurrentControl(  );
                pImpl->xSelSupp.set(xCtrl, UNO_QUERY);
                if(pImpl->xSelSupp.is()) {
                    pImpl->xChgLstnr = new SwXSelChgLstnr_Impl(*this);
                    pImpl->xSelSupp->addSelectionChangeListener(  pImpl->xChgLstnr );
                }
            }
        }
    }

    pModOpt = SW_MOD()->GetModuleConfig();

    MailTextFormats nMailingMode(pModOpt->GetMailingFormats());
    m_xFormatSwCB->set_active(bool(nMailingMode & MailTextFormats::OFFICE));
    m_xFormatHtmlCB->set_active(bool(nMailingMode & MailTextFormats::HTML));
    m_xFormatRtfCB->set_active(bool(nMailingMode & MailTextFormats::RTF));

    m_xAllRB->set_active(true);

    // Install handlers
    m_xOkBTN->connect_clicked(LINK(this, SwMailMergeDlg, ButtonHdl));

    m_xPathPB->connect_clicked(LINK(this, SwMailMergeDlg, InsertPathHdl));

    m_xPrinterRB->connect_toggled(LINK(this, SwMailMergeDlg, OutputTypeHdl));
    m_xFileRB->connect_toggled(LINK(this, SwMailMergeDlg, OutputTypeHdl));

    //#i63267# printing might be disabled
    bool bIsPrintable = !Application::GetSettings().GetMiscSettings().GetDisablePrinting();
    m_xPrinterRB->set_sensitive(bIsPrintable);
    OutputTypeHdl(bIsPrintable ? *m_xPrinterRB : *m_xFileRB);

    m_xGenerateFromDataBaseCB->connect_toggled(LINK(this, SwMailMergeDlg, FilenameHdl));
    bool bColumn = pModOpt->IsNameFromColumn();
    if(bColumn)
        m_xGenerateFromDataBaseCB->set_active(true);

    FilenameHdl(*m_xGenerateFromDataBaseCB);
    m_xSaveSingleDocRB->set_active(true);
    m_xSaveSingleDocRB->connect_toggled(LINK(this, SwMailMergeDlg, SaveTypeHdl));
    m_xSaveIndividualRB->connect_toggled(LINK(this, SwMailMergeDlg, SaveTypeHdl));
    SaveTypeHdl(*m_xSaveSingleDocRB);

    m_xFilterLB->connect_changed(LINK(this, SwMailMergeDlg, FileFormatHdl));

    Link<weld::SpinButton&,void> aLk2 = LINK(this, SwMailMergeDlg, ModifyHdl);
    m_xFromNF->connect_value_changed(aLk2);
    m_xToNF->connect_value_changed(aLk2);
    m_xFromNF->set_max(SAL_MAX_INT32);
    m_xToNF->set_max(SAL_MAX_INT32);

    SwDBManager* pDBManager = rSh.GetDBManager();
    if(_xConnection.is())
        SwDBManager::GetColumnNames(*m_xAddressFieldLB, _xConnection, rTableName);
    else
        pDBManager->GetColumnNames(*m_xAddressFieldLB, rSourceName, rTableName);
    for(sal_Int32 nEntry = 0, nEntryCount = m_xAddressFieldLB->get_count(); nEntry < nEntryCount; ++nEntry)
    {
        m_xColumnLB->append_text(m_xAddressFieldLB->get_text(nEntry));
        m_xPasswordLB->append_text(m_xAddressFieldLB->get_text(nEntry));
    }

    m_xAddressFieldLB->set_active_text("EMAIL");

    OUString sPath(pModOpt->GetMailingPath());
    if(sPath.isEmpty()) {
        SvtPathOptions aPathOpt;
        sPath = aPathOpt.GetWorkPath();
    }
    INetURLObject aURL(sPath);
    if(aURL.GetProtocol() == INetProtocol::File)
        m_xPathED->set_text(aURL.PathToFileName());
    else
        m_xPathED->set_text(aURL.GetFull());

    if (!bColumn )
    {
        m_xColumnLB->set_active_text("NAME");
        m_xPasswordLB->set_active_text("PASSWORD");
    }
    else
    {
        m_xColumnLB->set_active_text(pModOpt->GetNameFromColumn());
        m_xPasswordLB->set_active_text(pModOpt->GetPasswordFromColumn());
    }

    if (m_xAddressFieldLB->get_active() == -1)
        m_xAddressFieldLB->set_active(0);
    if (m_xColumnLB->get_active() == -1)
        m_xColumnLB->set_active(0);
    if (m_xPasswordLB->get_active() == -1)
        m_xPasswordLB->set_active(0);

    const bool bEnable = m_aSelection.hasElements();
    m_xMarkedRB->set_sensitive(bEnable);
    if (bEnable)
        m_xMarkedRB->set_active(true);
    else {
        m_xAllRB->set_active(true);
        m_xMarkedRB->set_sensitive(false);
    }
    try {
        uno::Reference< container::XNameContainer> xFilterFactory(
            xMSF->createInstance("com.sun.star.document.FilterFactory"), UNO_QUERY_THROW);
        uno::Reference< container::XContainerQuery > xQuery(xFilterFactory, UNO_QUERY_THROW);
        const OUString sCommand("matchByDocumentService=com.sun.star.text.TextDocument:iflags="
                                + OUString::number(static_cast<sal_Int32>(SfxFilterFlags::EXPORT))
                                + ":eflags="
                                + OUString::number(static_cast<sal_Int32>(SfxFilterFlags::NOTINFILEDLG))
                                + ":default_first");
        uno::Reference< container::XEnumeration > xList = xQuery->createSubSetEnumerationByQuery(sCommand);
        static const OUStringLiteral sName(u"Name");
        sal_Int32 nODT = -1;
        while(xList->hasMoreElements()) {
            comphelper::SequenceAsHashMap aFilter(xList->nextElement());
            const OUString sFilter = aFilter.getUnpackedValueOrDefault(sName, OUString());

            uno::Any aProps = xFilterFactory->getByName(sFilter);
            uno::Sequence< beans::PropertyValue > aFilterProperties;
            aProps >>= aFilterProperties;
            OUString sUIName2;
            auto pProp = std::find_if(aFilterProperties.begin(), aFilterProperties.end(),
                [](const beans::PropertyValue& rProp) { return rProp.Name == "UIName"; });
            if (pProp != aFilterProperties.end())
                pProp->Value >>= sUIName2;
            if( !sUIName2.isEmpty() ) {
                if( sFilter == "writer8" )
                    nODT = m_xFilterLB->get_count();
                m_xFilterLB->append(sFilter, sUIName2);
            }
        }
        m_xFilterLB->set_active( nODT );
    } catch (const uno::Exception&) {
    }
}

SwMailMergeDlg::~SwMailMergeDlg()
{
    if(m_xFrame.is()) {
        m_xFrame->setComponent(nullptr, nullptr);
        m_xFrame->dispose();
    }
}

IMPL_LINK_NOARG(SwMailMergeDlg, ButtonHdl, weld::Button&, void)
{
    if (ExecQryShell())
        m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SwMailMergeDlg, OutputTypeHdl, weld::ToggleButton&, void)
{
    bool bPrint = m_xPrinterRB->get_active();
    m_xSingleJobsCB->set_sensitive(bPrint);

    m_xSaveMergedDocumentFT->set_sensitive( !bPrint );
    m_xSaveSingleDocRB->set_sensitive( !bPrint );
    m_xSaveIndividualRB->set_sensitive( !bPrint );

    if( !bPrint ) {
        SaveTypeHdl( m_xSaveSingleDocRB->get_active() ? *m_xSaveSingleDocRB : *m_xSaveIndividualRB );
    } else {
        m_xPathFT->set_sensitive(false);
        m_xPathED->set_sensitive(false);
        m_xPathPB->set_sensitive(false);
        m_xColumnFT->set_sensitive(false);
        m_xColumnLB->set_sensitive(false);
        m_xFilterFT->set_sensitive(false);
        m_xFilterLB->set_sensitive(false);
        m_xGenerateFromDataBaseCB->set_sensitive(false);
        m_xPasswordCB->set_sensitive( false );
        m_xPasswordFT->set_sensitive( false );
        m_xPasswordLB->set_sensitive( false );
    }
}

IMPL_LINK_NOARG(SwMailMergeDlg, SaveTypeHdl, weld::ToggleButton&, void)
{
    bool bIndividual = m_xSaveIndividualRB->get_active();

    m_xGenerateFromDataBaseCB->set_sensitive( bIndividual );
    if( bIndividual )
    {
        FilenameHdl(*m_xGenerateFromDataBaseCB);
    }
    else
    {
        m_xColumnFT->set_sensitive(false);
        m_xColumnLB->set_sensitive(false);
        m_xPathFT->set_sensitive( false );
        m_xPathED->set_sensitive( false );
        m_xPathPB->set_sensitive( false );
        m_xFilterFT->set_sensitive( false );
        m_xFilterLB->set_sensitive( false );
        m_xPasswordCB->set_sensitive( false );
        m_xPasswordFT->set_sensitive( false );
        m_xPasswordLB->set_sensitive( false );
    }
}

IMPL_LINK( SwMailMergeDlg, FilenameHdl, weld::ToggleButton&, rBox, void )
{
    bool bEnable = rBox.get_active();
    m_xColumnFT->set_sensitive( bEnable );
    m_xColumnLB->set_sensitive(bEnable);
    m_xPathFT->set_sensitive( bEnable );
    m_xPathED->set_sensitive(bEnable);
    m_xPathPB->set_sensitive( bEnable );
    m_xFilterFT->set_sensitive( bEnable );
    m_xFilterLB->set_sensitive( bEnable );

    if(m_xFilterLB->get_active_id() == "writer_pdf_Export")
    {
        m_xPasswordCB->show();
        m_xPasswordFT->show();
        m_xPasswordLB->show();

        m_xPasswordCB->set_sensitive( bEnable );
        m_xPasswordFT->set_sensitive( bEnable );
        m_xPasswordLB->set_sensitive( bEnable );
    }
}

IMPL_LINK_NOARG( SwMailMergeDlg, FileFormatHdl, weld::ComboBox&, void )
{
    if(m_xFilterLB->get_active_id() == "writer_pdf_Export")
    {
        m_xPasswordCB->show();
        m_xPasswordFT->show();
        m_xPasswordLB->show();

        m_xPasswordCB->set_sensitive( true );
        m_xPasswordFT->set_sensitive( true );
        m_xPasswordLB->set_sensitive( true );
    }
    else
    {
        m_xPasswordCB->hide();
        m_xPasswordFT->hide();
        m_xPasswordLB->hide();
    }
}

IMPL_LINK_NOARG(SwMailMergeDlg, ModifyHdl, weld::SpinButton&, void)
{
    m_xFromRB->set_active(true);
}

bool SwMailMergeDlg::AskUserFilename() const
{
    return (m_xSaveSingleDocRB->get_active() || !m_xGenerateFromDataBaseCB->get_active());
}

OUString SwMailMergeDlg::GetURLfromPath() const
{
    SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
    INetURLObject aAbs;
    if( pMedium )
        aAbs = pMedium->GetURLObject();
    if( INetProtocol::NotValid == aAbs.GetProtocol() )
    {
        SvtPathOptions aPathOpt;
        aAbs.SetURL( aPathOpt.GetWorkPath() );
    }
    return URIHelper::SmartRel2Abs(
        aAbs, m_xPathED->get_text(), URIHelper::GetMaybeFileHdl());
}

bool SwMailMergeDlg::ExecQryShell()
{
    if(pImpl->xSelSupp.is()) {
        pImpl->xSelSupp->removeSelectionChangeListener( pImpl->xChgLstnr );
    }

    if (m_xPrinterRB->get_active())
        nMergeType = DBMGR_MERGE_PRINTER;
    else {
        nMergeType = DBMGR_MERGE_FILE;
        pModOpt->SetMailingPath( GetURLfromPath() );
        pModOpt->SetIsNameFromColumn(m_xGenerateFromDataBaseCB->get_active());
        pModOpt->SetIsFileEncryptedFromColumn(m_xPasswordCB->get_active());

        if (!AskUserFilename())
        {
            pModOpt->SetNameFromColumn(m_xColumnLB->get_active_text());
            pModOpt->SetPasswordFromColumn(m_xPasswordLB->get_active_text());
            if (m_xFilterLB->get_active() != -1)
                m_sSaveFilter = m_xFilterLB->get_active_id();
            m_sFilename = OUString();
        } else {
            //#i97667# reset column name - otherwise it's remembered from the last run
            pModOpt->SetNameFromColumn(OUString());
            pModOpt->SetPasswordFromColumn(OUString());
            //start save as dialog
            OUString sFilter;
            m_sFilename = SwMailMergeHelper::CallSaveAsDialog(m_xDialog.get(), sFilter);
            if (m_sFilename.isEmpty())
                return false;
            m_sSaveFilter = sFilter;
        }
    }

    if (m_xFromRB->get_active()) {  // Insert list
        // Safe: the maximal value of the fields is limited
        sal_Int32 nStart = m_xFromNF->get_value();
        sal_Int32 nEnd = m_xToNF->get_value();

        if (nEnd < nStart)
            std::swap(nEnd, nStart);

        m_aSelection.realloc(nEnd - nStart + 1);
        Any* pSelection = m_aSelection.getArray();
        for (sal_Int32 i = nStart; i <= nEnd; ++i, ++pSelection)
            *pSelection <<= i;
    } else if (m_xAllRB->get_active() )
        m_aSelection.realloc(0);    // Empty selection = insert all
    else {
        if(pImpl->xSelSupp.is()) {
            //update selection
            uno::Reference< XRowLocate > xRowLocate(GetResultSet(),UNO_QUERY);
            uno::Reference< XResultSet > xRes(xRowLocate,UNO_QUERY);
            pImpl->xSelSupp->getSelection() >>= m_aSelection;
            if ( xRowLocate.is() ) {
                for (Any& rRow : m_aSelection) {
                    if ( xRowLocate->moveToBookmark(rRow) )
                        rRow <<= xRes->getRow();
                }
            }
        }
    }
    IDocumentDeviceAccess& rIDDA = rSh.getIDocumentDeviceAccess();
    SwPrintData aPrtData( rIDDA.getPrintData() );
    aPrtData.SetPrintSingleJobs(m_xSingleJobsCB->get_active());
    rIDDA.setPrintData(aPrtData);

    pModOpt->SetSinglePrintJob(m_xSingleJobsCB->get_active());

    MailTextFormats nMailingMode = MailTextFormats::NONE;

    if (m_xFormatSwCB->get_active())
        nMailingMode |= MailTextFormats::OFFICE;
    if (m_xFormatHtmlCB->get_active())
        nMailingMode |= MailTextFormats::HTML;
    if (m_xFormatRtfCB->get_active())
        nMailingMode |= MailTextFormats::RTF;
    pModOpt->SetMailingFormats(nMailingMode);
    return true;
}

OUString SwMailMergeDlg::GetTargetURL() const
{
    if( AskUserFilename() )
        return m_sFilename;
    OUString sPath( pModOpt->GetMailingPath() );
    if( sPath.isEmpty() ) {
        SvtPathOptions aPathOpt;
        sPath = aPathOpt.GetWorkPath();
    }
    if( !sPath.endsWith("/") )
        sPath += "/";
    return sPath;
}

IMPL_LINK_NOARG(SwMailMergeDlg, InsertPathHdl, weld::Button&, void)
{
    uno::Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < XFolderPicker2 > xFP = FolderPicker::create(xContext);
    xFP->setDisplayDirectory( GetURLfromPath() );
    if( xFP->execute() == RET_OK ) {
        INetURLObject aURL(xFP->getDirectory());
        if(aURL.GetProtocol() == INetProtocol::File)
            m_xPathED->set_text(aURL.PathToFileName());
        else
            m_xPathED->set_text(aURL.GetFull());
    }
}

uno::Reference<XResultSet> SwMailMergeDlg::GetResultSet() const
{
    uno::Reference< XResultSet >  xResSetClone;
    if ( pImpl->xFController.is() ) {
        // we create a clone to do the task
        uno::Reference< XResultSetAccess > xResultSetAccess( pImpl->xFController->getModel(),UNO_QUERY);
        if ( xResultSetAccess.is() )
            xResSetClone = xResultSetAccess->createResultSet();
    }
    return xResSetClone;
}

SwMailMergeCreateFromDlg::SwMailMergeCreateFromDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/swriter/ui/mailmergedialog.ui", "MailMergeDialog")
    , m_xThisDocRB(m_xBuilder->weld_radio_button("document"))
{
}

SwMailMergeCreateFromDlg::~SwMailMergeCreateFromDlg()
{
}

SwMailMergeFieldConnectionsDlg::SwMailMergeFieldConnectionsDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/swriter/ui/mergeconnectdialog.ui", "MergeConnectDialog")
    , m_xUseExistingRB(m_xBuilder->weld_radio_button("existing"))
{
}

SwMailMergeFieldConnectionsDlg::~SwMailMergeFieldConnectionsDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
