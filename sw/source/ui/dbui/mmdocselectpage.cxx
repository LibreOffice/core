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
#include <vcl/msgbox.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <mmdocselectpage.hxx>
#include <mailmergewizard.hxx>
#include <shellio.hxx>
#include <swabstdlg.hxx>
#include <mmconfigitem.hxx>

#include <dbui.hrc>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace svt;

SwMailMergeDocSelectPage::SwMailMergeDocSelectPage(SwMailMergeWizard* pParent)
    : svt::OWizardPage(pParent, "MMSelectPage",
        "modules/swriter/ui/mmselectpage.ui")
    , m_pWizard(pParent)
{
    get(m_pCurrentDocRB, "currentdoc");
    get(m_pNewDocRB, "newdoc");
    get(m_pLoadDocRB, "loaddoc");
    get(m_pLoadTemplateRB, "template");
    get(m_pRecentDocRB, "recentdoc");
    get(m_pBrowseDocPB, "browsedoc");
    get(m_pBrowseTemplatePB, "browsetemplate");
    get(m_pRecentDocLB, "recentdoclb");

    m_pCurrentDocRB->Check();
    DocSelectHdl(m_pNewDocRB);

    Link<Button*,void> aDocSelectLink = LINK(this, SwMailMergeDocSelectPage, DocSelectHdl);
    m_pCurrentDocRB->SetClickHdl(aDocSelectLink);
    m_pNewDocRB->SetClickHdl(aDocSelectLink);
    m_pLoadDocRB->SetClickHdl(aDocSelectLink);
    m_pLoadTemplateRB->SetClickHdl(aDocSelectLink);
    m_pRecentDocRB->SetClickHdl(aDocSelectLink);

    Link<Button*,void> aFileSelectHdl = LINK(this, SwMailMergeDocSelectPage, FileSelectHdl);
    m_pBrowseDocPB->SetClickHdl(aFileSelectHdl);
    m_pBrowseTemplatePB->SetClickHdl(aFileSelectHdl);

    const uno::Sequence< OUString >& rDocs =
                            m_pWizard->GetConfigItem().GetSavedDocuments();
    for(sal_Int32 nDoc = 0; nDoc < rDocs.getLength(); ++nDoc)
    {
        //insert in reverse order
        m_pRecentDocLB->InsertEntry(rDocs[nDoc], 0);
    }
    m_pRecentDocLB->SelectEntryPos(0);
    if(!rDocs.getLength())
    {
        m_pRecentDocRB->Enable(false);
    }

    //Temp hack until all pages are converted to .ui and wizard
    //base class adapted
    SetSizePixel(LogicToPixel(Size(260 , 250), MapMode(MAP_APPFONT)));
}

SwMailMergeDocSelectPage::~SwMailMergeDocSelectPage()
{
    disposeOnce();
}

void SwMailMergeDocSelectPage::dispose()
{
    m_pCurrentDocRB.clear();
    m_pNewDocRB.clear();
    m_pLoadDocRB.clear();
    m_pLoadTemplateRB.clear();
    m_pRecentDocRB.clear();
    m_pBrowseDocPB.clear();
    m_pBrowseTemplatePB.clear();
    m_pRecentDocLB.clear();
    m_pWizard.clear();
    svt::OWizardPage::dispose();
}

IMPL_LINK_TYPED(SwMailMergeDocSelectPage, DocSelectHdl, Button*, pButton, void)
{
    m_pRecentDocLB->Enable(m_pRecentDocRB == pButton);

    m_pWizard->UpdateRoadmap();
    m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_OUTPUTTYPETPAGE));
}

IMPL_LINK_TYPED(SwMailMergeDocSelectPage, FileSelectHdl, Button*, pButton, void)
{
    bool bTemplate = m_pBrowseTemplatePB == pButton;

    if(bTemplate)
    {
        m_pLoadTemplateRB->Check();
        VclPtrInstance< SfxNewFileDialog > pNewFileDlg(this, 0);
        sal_uInt16 nRet = pNewFileDlg->Execute();
        if(RET_TEMPLATE_LOAD == nRet)
            bTemplate = false;
        else if(RET_CANCEL != nRet)
            m_sLoadTemplateName = pNewFileDlg->GetTemplateFileName();
    }
    else
        m_pLoadDocRB->Check();

    if(!bTemplate)
    {
        sfx2::FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
        Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

        xFP->setDisplayDirectory( SvtPathOptions().GetWorkPath() );

        SfxObjectFactory &rFact = m_pWizard->GetSwView()->GetDocShell()->GetFactory();
        SfxFilterMatcher aMatcher( OUString::createFromAscii(rFact.GetShortName()) );
        SfxFilterMatcherIter aIter( aMatcher );
        Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
        const SfxFilter* pFlt = aIter.First();
        while( pFlt )
        {
            if( pFlt && pFlt->IsAllowedAsTemplate() )
            {
                const OUString sWild = pFlt->GetWildcard().getGlob();
                xFltMgr->appendFilter( pFlt->GetUIName(), sWild );

                // #i40125
                if(pFlt->GetFilterFlags() & SfxFilterFlags::DEFAULT)
                    xFltMgr->setCurrentFilter( pFlt->GetUIName() ) ;
            }

            pFlt = aIter.Next();
        }

        if( ERRCODE_NONE == aDlgHelper.Execute() )
        {
            m_sLoadFileName = xFP->getFiles().getConstArray()[0];
        }
    }
    m_pWizard->UpdateRoadmap();
    m_pWizard->enableButtons(WizardButtonFlags::NEXT, m_pWizard->isStateEnabled(MM_OUTPUTTYPETPAGE));
}

bool SwMailMergeDocSelectPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
{
    bool bReturn = false;
    bool bNext = _eReason == ::svt::WizardTypes::eTravelForward;
    if(bNext || _eReason == ::svt::WizardTypes::eValidate )
    {
        OUString sReloadDocument;
        bReturn = m_pCurrentDocRB->IsChecked() ||
                m_pNewDocRB->IsChecked() ||
                (!(sReloadDocument = m_sLoadFileName).isEmpty() && m_pLoadDocRB->IsChecked() )||
                (!(sReloadDocument = m_sLoadTemplateName).isEmpty() && m_pLoadTemplateRB->IsChecked())||
                (m_pRecentDocRB->IsChecked() && !(sReloadDocument = m_pRecentDocLB->GetSelectEntry()).isEmpty());
        if( _eReason == ::svt::WizardTypes::eValidate )
            m_pWizard->SetDocumentLoad(!m_pCurrentDocRB->IsChecked());

        if(bNext && !m_pCurrentDocRB->IsChecked())
        {
            if(!sReloadDocument.isEmpty())
                m_pWizard->SetReloadDocument( sReloadDocument );
            m_pWizard->SetRestartPage(MM_OUTPUTTYPETPAGE);
            m_pWizard->EndDialog(RET_LOAD_DOC);
        }
    }
    return bReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
