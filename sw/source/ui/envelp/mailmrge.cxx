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
#include <comphelper/propertyvalue.hxx>

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
#include <sfx2/filedlghelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>

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
    SwMailMergeDlg& m_rParent;
public:
    explicit SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg);

    virtual void SAL_CALL selectionChanged( const EventObject& aEvent ) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) override;
};

SwXSelChgLstnr_Impl::SwXSelChgLstnr_Impl(SwMailMergeDlg& rParentDlg) :
    m_rParent(rParentDlg)
{}

void SwXSelChgLstnr_Impl::selectionChanged( const EventObject&  )
{
    //call the parent to enable selection mode
    Sequence <Any> aSelection;
    if(m_rParent.m_pImpl->xSelSupp.is())
        m_rParent.m_pImpl->xSelSupp->getSelection() >>= aSelection;

    bool bEnable = aSelection.hasElements();
    m_rParent.m_xMarkedRB->set_sensitive(bEnable);
    if(bEnable)
        m_rParent.m_xMarkedRB->set_active(true);
    else if(m_rParent.m_xMarkedRB->get_active()) {
        m_rParent.m_xAllRB->set_active(true);
        m_rParent.m_aSelection.realloc(0);
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
    : SfxDialogController(pParent, u"modules/swriter/ui/mailmerge.ui"_ustr, u"MailmergeDialog"_ustr)
    , m_pImpl(new SwMailMergeDlg_Impl)
    , m_rSh(rShell)
    , m_nMergeType(DBMGR_MERGE_EMAIL)
    , m_xBeamerWin(m_xBuilder->weld_container(u"beamer"_ustr))
    , m_xAllRB(m_xBuilder->weld_radio_button(u"all"_ustr))
    , m_xMarkedRB(m_xBuilder->weld_radio_button(u"selected"_ustr))
    , m_xFromRB(m_xBuilder->weld_radio_button(u"rbfrom"_ustr))
    , m_xFromNF(m_xBuilder->weld_spin_button(u"from"_ustr))
    , m_xToNF(m_xBuilder->weld_spin_button(u"to"_ustr))
    , m_xPrinterRB(m_xBuilder->weld_radio_button(u"printer"_ustr))
    , m_xMailingRB(m_xBuilder->weld_radio_button(u"electronic"_ustr))
    , m_xFileRB(m_xBuilder->weld_radio_button(u"file"_ustr))
    , m_xPasswordCB(m_xBuilder->weld_check_button(u"passwd-check"_ustr))
    , m_xSaveMergedDocumentFT(m_xBuilder->weld_label(u"savemergeddoclabel"_ustr))
    , m_xSaveSingleDocRB(m_xBuilder->weld_radio_button(u"singledocument"_ustr))
    , m_xSaveIndividualRB(m_xBuilder->weld_radio_button(u"individualdocuments"_ustr))
    , m_xGenerateFromDataBaseCB(m_xBuilder->weld_check_button(u"generate"_ustr))
    , m_xColumnFT(m_xBuilder->weld_label(u"fieldlabel"_ustr))
    , m_xColumnLB(m_xBuilder->weld_combo_box(u"field"_ustr))
    , m_xPasswordFT(m_xBuilder->weld_label(u"passwd-label"_ustr))
    , m_xPasswordLB(m_xBuilder->weld_combo_box(u"passwd-combobox"_ustr))
    , m_xPathFT(m_xBuilder->weld_label(u"pathlabel"_ustr))
    , m_xPathED(m_xBuilder->weld_entry(u"path"_ustr))
    , m_xPathPB(m_xBuilder->weld_button(u"pathpb"_ustr))
    , m_xFilterFT(m_xBuilder->weld_label(u"fileformatlabel"_ustr))
    , m_xFilterLB(m_xBuilder->weld_combo_box(u"fileformat"_ustr))
    , m_xAddressFieldLB(m_xBuilder->weld_combo_box(u"address"_ustr))
    , m_xSubjectFT(m_xBuilder->weld_label(u"subjectlabel"_ustr))
    , m_xSubjectED(m_xBuilder->weld_entry(u"subject"_ustr))
    , m_xFormatFT(m_xBuilder->weld_label(u"mailformatlabel"_ustr))
    , m_xAttachFT(m_xBuilder->weld_label(u"attachmentslabel"_ustr))
    , m_xAttachED(m_xBuilder->weld_entry(u"attachments"_ustr))
    , m_xAttachPB(m_xBuilder->weld_button(u"attach"_ustr))
    , m_xFormatHtmlCB(m_xBuilder->weld_check_button(u"html"_ustr))
    , m_xFormatRtfCB(m_xBuilder->weld_check_button(u"rtf"_ustr))
    , m_xFormatSwCB(m_xBuilder->weld_check_button(u"swriter"_ustr))
    , m_xOkBTN(m_xBuilder->weld_button(u"ok"_ustr))
{
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
                                           u""_ustr,
                                           0x0C);
            if(xD.is()) {
                Sequence<PropertyValue> aProperties
                {
                    comphelper::makePropertyValue(u"DataSourceName"_ustr, rSourceName),
                    comphelper::makePropertyValue(u"Command"_ustr, rTableName),
                    comphelper::makePropertyValue(u"CommandType"_ustr, nCommandType),
                };
                xD->dispatch(aURL, aProperties);
                m_xBeamerWin->show();
            }
            uno::Reference<XController> xController = m_xFrame->getController();
            m_pImpl->xFController.set(xController, UNO_QUERY);
            if(m_pImpl->xFController.is()) {
                uno::Reference< awt::XControl > xCtrl = m_pImpl->xFController->getCurrentControl(  );
                m_pImpl->xSelSupp.set(xCtrl, UNO_QUERY);
                if(m_pImpl->xSelSupp.is()) {
                    m_pImpl->xChgLstnr = new SwXSelChgLstnr_Impl(*this);
                    m_pImpl->xSelSupp->addSelectionChangeListener(  m_pImpl->xChgLstnr );
                }
            }
        }
    }

    m_pModOpt = SW_MOD()->GetModuleConfig();

    MailTextFormats nMailingMode(m_pModOpt->GetMailingFormats());
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
    bool bColumn = m_pModOpt->IsNameFromColumn();
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

    SwDBManager* pDBManager = m_rSh.GetDBManager();
    if(_xConnection.is())
        SwDBManager::GetColumnNames(*m_xAddressFieldLB, _xConnection, rTableName);
    else
        pDBManager->GetColumnNames(*m_xAddressFieldLB, rSourceName, rTableName);
    for(sal_Int32 nEntry = 0, nEntryCount = m_xAddressFieldLB->get_count(); nEntry < nEntryCount; ++nEntry)
    {
        m_xColumnLB->append_text(m_xAddressFieldLB->get_text(nEntry));
        m_xPasswordLB->append_text(m_xAddressFieldLB->get_text(nEntry));
    }

    m_xAddressFieldLB->set_active_text(u"EMAIL"_ustr);

    OUString sPath(m_pModOpt->GetMailingPath());
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
        m_xColumnLB->set_active_text(u"NAME"_ustr);
        m_xPasswordLB->set_active_text(u"PASSWORD"_ustr);
    }
    else
    {
        m_xColumnLB->set_active_text(m_pModOpt->GetNameFromColumn());
        m_xPasswordLB->set_active_text(m_pModOpt->GetPasswordFromColumn());
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
            xMSF->createInstance(u"com.sun.star.document.FilterFactory"_ustr), UNO_QUERY_THROW);
        uno::Reference< container::XContainerQuery > xQuery(xFilterFactory, UNO_QUERY_THROW);
        const OUString sCommand("matchByDocumentService=com.sun.star.text.TextDocument:iflags="
                                + OUString::number(static_cast<sal_Int32>(SfxFilterFlags::EXPORT))
                                + ":eflags="
                                + OUString::number(static_cast<sal_Int32>(SfxFilterFlags::NOTINFILEDLG))
                                + ":default_first");
        uno::Reference< container::XEnumeration > xList = xQuery->createSubSetEnumerationByQuery(sCommand);
        static constexpr OUStringLiteral sName(u"Name");
        sal_Int32 nODT = -1;
        while(xList->hasMoreElements()) {
            comphelper::SequenceAsHashMap aFilter(xList->nextElement());
            const OUString sFilter = aFilter.getUnpackedValueOrDefault(sName, OUString());

            uno::Any aProps = xFilterFactory->getByName(sFilter);
            uno::Sequence< beans::PropertyValue > aFilterProperties;
            aProps >>= aFilterProperties;
            OUString sUIName2;
            auto pProp = std::find_if(std::cbegin(aFilterProperties), std::cend(aFilterProperties),
                [](const beans::PropertyValue& rProp) { return rProp.Name == "UIName"; });
            if (pProp != std::cend(aFilterProperties))
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

IMPL_LINK_NOARG(SwMailMergeDlg, OutputTypeHdl, weld::Toggleable&, void)
{
    bool bPrint = m_xPrinterRB->get_active();
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

IMPL_LINK_NOARG(SwMailMergeDlg, SaveTypeHdl, weld::Toggleable&, void)
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

IMPL_LINK( SwMailMergeDlg, FilenameHdl, weld::Toggleable&, rBox, void )
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
    SfxMedium* pMedium = m_rSh.GetView().GetDocShell()->GetMedium();
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
    if(m_pImpl->xSelSupp.is()) {
        m_pImpl->xSelSupp->removeSelectionChangeListener( m_pImpl->xChgLstnr );
    }

    if (m_xPrinterRB->get_active())
        m_nMergeType = DBMGR_MERGE_PRINTER;
    else {
        m_nMergeType = DBMGR_MERGE_FILE;
        m_pModOpt->SetMailingPath( GetURLfromPath() );
        m_pModOpt->SetIsNameFromColumn(m_xGenerateFromDataBaseCB->get_active());
        m_pModOpt->SetIsFileEncryptedFromColumn(m_xPasswordCB->get_active());

        if (!AskUserFilename())
        {
            m_pModOpt->SetNameFromColumn(m_xColumnLB->get_active_text());
            m_pModOpt->SetPasswordFromColumn(m_xPasswordLB->get_active_text());
            if (m_xFilterLB->get_active() != -1)
                m_sSaveFilter = m_xFilterLB->get_active_id();
            m_sFilename = OUString();
        } else {
            //#i97667# reset column name - otherwise it's remembered from the last run
            m_pModOpt->SetNameFromColumn(OUString());
            m_pModOpt->SetPasswordFromColumn(OUString());
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
        if(m_pImpl->xSelSupp.is()) {
            //update selection
            uno::Reference< XRowLocate > xRowLocate(GetResultSet(),UNO_QUERY);
            uno::Reference< XResultSet > xRes(xRowLocate,UNO_QUERY);
            m_pImpl->xSelSupp->getSelection() >>= m_aSelection;
            if ( xRowLocate.is() ) {
                for (Any& rRow : asNonConstRange(m_aSelection)) {
                    if ( xRowLocate->moveToBookmark(rRow) )
                        rRow <<= xRes->getRow();
                }
            }
        }
    }
    MailTextFormats nMailingMode = MailTextFormats::NONE;

    if (m_xFormatSwCB->get_active())
        nMailingMode |= MailTextFormats::OFFICE;
    if (m_xFormatHtmlCB->get_active())
        nMailingMode |= MailTextFormats::HTML;
    if (m_xFormatRtfCB->get_active())
        nMailingMode |= MailTextFormats::RTF;
    m_pModOpt->SetMailingFormats(nMailingMode);
    return true;
}

OUString SwMailMergeDlg::GetTargetURL() const
{
    if( AskUserFilename() )
        return m_sFilename;
    OUString sPath( m_pModOpt->GetMailingPath() );
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
    uno::Reference < XFolderPicker2 > xFP = sfx2::createFolderPicker(xContext, m_xDialog.get());
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
    if ( m_pImpl->xFController.is() ) {
        // we create a clone to do the task
        uno::Reference< XResultSetAccess > xResultSetAccess( m_pImpl->xFController->getModel(),UNO_QUERY);
        if ( xResultSetAccess.is() )
            xResSetClone = xResultSetAccess->createResultSet();
    }
    return xResSetClone;
}

SwMailMergeCreateFromDlg::SwMailMergeCreateFromDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/swriter/ui/mailmergedialog.ui"_ustr, u"MailMergeDialog"_ustr)
    , m_xThisDocRB(m_xBuilder->weld_radio_button(u"document"_ustr))
{
}

SwMailMergeCreateFromDlg::~SwMailMergeCreateFromDlg()
{
}

SwMailMergeFieldConnectionsDlg::SwMailMergeFieldConnectionsDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/swriter/ui/mergeconnectdialog.ui"_ustr, u"MergeConnectDialog"_ustr)
    , m_xUseExistingRB(m_xBuilder->weld_radio_button(u"existing"_ustr))
{
}

SwMailMergeFieldConnectionsDlg::~SwMailMergeFieldConnectionsDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
