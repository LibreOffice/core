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

#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/new.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfac.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include "mmdocselectpage.hxx"
#include <mailmergewizard.hxx>
#include <swabstdlg.hxx>
#include <mmconfigitem.hxx>
#include <swuiexp.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace svt;

SwMailMergeDocSelectPage::SwMailMergeDocSelectPage(weld::Container* pPage, SwMailMergeWizard* pWizard)
    : vcl::OWizardPage(pPage, pWizard, "modules/swriter/ui/mmselectpage.ui", "MMSelectPage")
    , m_pWizard(pWizard)
    , m_xCurrentDocRB(m_xBuilder->weld_radio_button("currentdoc"))
    , m_xNewDocRB(m_xBuilder->weld_radio_button("newdoc"))
    , m_xLoadDocRB(m_xBuilder->weld_radio_button("loaddoc"))
    , m_xLoadTemplateRB(m_xBuilder->weld_radio_button("template"))
    , m_xRecentDocRB(m_xBuilder->weld_radio_button("recentdoc"))
    , m_xBrowseDocPB(m_xBuilder->weld_button("browsedoc"))
    , m_xBrowseTemplatePB(m_xBuilder->weld_button("browsetemplate"))
    , m_xRecentDocLB(m_xBuilder->weld_combo_box("recentdoclb"))
    , m_xDataSourceWarningFT(m_xBuilder->weld_label("datasourcewarning"))
    , m_xExchangeDatabasePB(m_xBuilder->weld_button("exchangedatabase"))
{
    m_xCurrentDocRB->set_active(true);
    DocSelectHdl(*m_xNewDocRB);

    Link<weld::ToggleButton&,void> aDocSelectLink = LINK(this, SwMailMergeDocSelectPage, DocSelectHdl);
    m_xCurrentDocRB->connect_toggled(aDocSelectLink);
    m_xNewDocRB->connect_toggled(aDocSelectLink);
    m_xLoadDocRB->connect_toggled(aDocSelectLink);
    m_xLoadTemplateRB->connect_toggled(aDocSelectLink);
    m_xRecentDocRB->connect_toggled(aDocSelectLink);

    Link<weld::Button&,void> aFileSelectHdl = LINK(this, SwMailMergeDocSelectPage, FileSelectHdl);
    m_xBrowseDocPB->connect_clicked(aFileSelectHdl);
    m_xBrowseTemplatePB->connect_clicked(aFileSelectHdl);

    Link<weld::Button&,void> aExchangeDatabaseHdl = LINK(this, SwMailMergeDocSelectPage, ExchangeDatabaseHdl);
    m_xExchangeDatabasePB->connect_clicked(aExchangeDatabaseHdl);

    const uno::Sequence< OUString >& rDocs =
                            m_pWizard->GetConfigItem().GetSavedDocuments();
    for(const auto& rDoc : rDocs)
    {
        //insert in reverse order
        m_xRecentDocLB->insert_text(0, rDoc);
    }
    if (!rDocs.hasElements())
        m_xRecentDocRB->set_sensitive(false);
    else
        m_xRecentDocLB->set_active(0);
}

SwMailMergeDocSelectPage::~SwMailMergeDocSelectPage()
{
}

IMPL_LINK_NOARG(SwMailMergeDocSelectPage, DocSelectHdl, weld::ToggleButton&, void)
{
    m_xRecentDocLB->set_sensitive(m_xRecentDocRB->get_active());
    m_pWizard->UpdateRoadmap();
    OUString sDataSourceName = m_pWizard->GetSwView()->GetDataSourceName();

    if(m_xCurrentDocRB->get_active() &&
       !sDataSourceName.isEmpty() &&
       !SwView::IsDataSourceAvailable(sDataSourceName))
    {
        m_xDataSourceWarningFT->show();
        m_pWizard->enableButtons(WizardButtonFlags::NEXT, false);
    }
    else
    {
        m_xDataSourceWarningFT->hide();
        m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_OUTPUTTYPETPAGE));
    }

    if(m_xCurrentDocRB->get_active())
        m_xExchangeDatabasePB->set_sensitive(true);
    else
        m_xExchangeDatabasePB->set_sensitive(false);
}

IMPL_LINK(SwMailMergeDocSelectPage, FileSelectHdl, weld::Button&, rButton, void)
{
    bool bTemplate = m_xBrowseTemplatePB.get() == &rButton;

    if(bTemplate)
    {
        m_xLoadTemplateRB->set_active(true);
        SfxNewFileDialog aNewFileDlg(m_pWizard->getDialog(), SfxNewFileDialogMode::NONE);
        sal_uInt16 nRet = aNewFileDlg.run();
        if(RET_TEMPLATE_LOAD == nRet)
            bTemplate = false;
        else if(RET_CANCEL != nRet)
            m_sLoadTemplateName = aNewFileDlg.GetTemplateFileName();
    }
    else
        m_xLoadDocRB->set_active(true);

    if(!bTemplate)
    {
        sfx2::FileDialogHelper aDlgHelper(TemplateDescription::FILEOPEN_SIMPLE,
                                          FileDialogFlags::NONE, m_pWizard->getDialog());
        Reference < XFilePicker3 > xFP = aDlgHelper.GetFilePicker();

        xFP->setDisplayDirectory( SvtPathOptions().GetWorkPath() );

        SfxObjectFactory &rFact = m_pWizard->GetSwView()->GetDocShell()->GetFactory();
        SfxFilterMatcher aMatcher( rFact.GetFactoryName() );
        SfxFilterMatcherIter aIter( aMatcher );
        std::shared_ptr<const SfxFilter> pFlt = aIter.First();
        while( pFlt )
        {
            if( pFlt && pFlt->IsAllowedAsTemplate() )
            {
                const OUString sWild = pFlt->GetWildcard().getGlob();
                xFP->appendFilter( pFlt->GetUIName(), sWild );

                // #i40125
                if(pFlt->GetFilterFlags() & SfxFilterFlags::DEFAULT)
                    xFP->setCurrentFilter( pFlt->GetUIName() ) ;
            }

            pFlt = aIter.Next();
        }

        if( ERRCODE_NONE == aDlgHelper.Execute() )
        {
            m_sLoadFileName = xFP->getSelectedFiles().getConstArray()[0];
        }
    }
    m_pWizard->UpdateRoadmap();
    m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_OUTPUTTYPETPAGE));
}

IMPL_LINK_NOARG(SwMailMergeDocSelectPage, ExchangeDatabaseHdl, weld::Button&, void)
{

    SwAbstractDialogFactory& rFact = ::swui::GetFactory();
    ScopedVclPtr<VclAbstractDialog> pDlg(rFact.CreateSwChangeDBDlg(*m_pWizard->GetSwView()));
    pDlg->Execute();

    OUString sDataSourceName = m_pWizard->GetSwView()->GetDataSourceName();

    if(m_xCurrentDocRB->get_active() &&
       !sDataSourceName.isEmpty() &&
       SwView::IsDataSourceAvailable(sDataSourceName))
    {
        m_xDataSourceWarningFT->hide();
        m_pWizard->enableButtons(WizardButtonFlags::NEXT, true);
    }
}

bool SwMailMergeDocSelectPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
{
    bool bReturn = false;
    bool bNext = _eReason == ::vcl::WizardTypes::eTravelForward;
    if(bNext || _eReason == ::vcl::WizardTypes::eValidate )
    {
        OUString sReloadDocument;
        bReturn = m_xCurrentDocRB->get_active() ||
                m_xNewDocRB->get_active();
        if (!bReturn)
        {
            sReloadDocument = m_sLoadFileName;
            bReturn = !sReloadDocument.isEmpty() && m_xLoadDocRB->get_active();
        }
        if (!bReturn)
        {
            sReloadDocument = m_sLoadTemplateName;
            bReturn = !sReloadDocument.isEmpty() && m_xLoadTemplateRB->get_active();
        }
        if (!bReturn)
        {
            bReturn = m_xRecentDocRB->get_active();
            if (bReturn)
            {
                sReloadDocument = m_xRecentDocLB->get_active_text();
                bReturn = !sReloadDocument.isEmpty();
            }
        }
        if( _eReason == ::vcl::WizardTypes::eValidate )
            m_pWizard->SetDocumentLoad(!m_xCurrentDocRB->get_active());

        if(bNext && !m_xCurrentDocRB->get_active())
        {
            if(!sReloadDocument.isEmpty())
                m_pWizard->SetReloadDocument( sReloadDocument );
            m_pWizard->SetRestartPage(MM_OUTPUTTYPETPAGE);
            m_pWizard->response(RET_LOAD_DOC);
        }
    }
    return bReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
