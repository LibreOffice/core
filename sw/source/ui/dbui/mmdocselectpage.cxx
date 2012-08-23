/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <mmdocselectpage.hrc>

#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace svt;

SwMailMergeDocSelectPage::SwMailMergeDocSelectPage( SwMailMergeWizard* _pParent ) :
    svt::OWizardPage(_pParent, SW_RES(DLG_MM_DOCSELECT_PAGE)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(this,          SW_RES(  FI_HEADER           ) ),
    m_aHowToFT          (this, SW_RES( FT_HOWTO )),
    m_aCurrentDocRB     (this, SW_RES( RB_CURRENTDOC )),
    m_aNewDocRB         (this, SW_RES( RB_NEWDOC )),
    m_aLoadDocRB        (this, SW_RES( RB_LOADDOC )),
    m_aLoadTemplateRB   (this, SW_RES( RB_LOADTEMPLATE )),
    m_aRecentDocRB      (this, SW_RES( RB_RECENTDOC )),
    m_aBrowseDocPB      (this, SW_RES( PB_LOADDOC )),
    m_aBrowseTemplatePB (this, SW_RES( PB_BROWSETEMPLATE )),
    m_aRecentDocLB      (this, SW_RES( LB_RECENTDOC )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_pWizard(_pParent)
{
    FreeResource();

    m_aCurrentDocRB.Check();
    DocSelectHdl(&m_aNewDocRB);

    Link aDocSelectLink = LINK(this, SwMailMergeDocSelectPage, DocSelectHdl);
    m_aCurrentDocRB.SetClickHdl(aDocSelectLink);
    m_aNewDocRB.SetClickHdl(aDocSelectLink);
    m_aLoadDocRB.SetClickHdl(aDocSelectLink);
    m_aLoadTemplateRB.SetClickHdl(aDocSelectLink);
    m_aRecentDocRB.SetClickHdl(aDocSelectLink);

    Link aFileSelectHdl = LINK(this, SwMailMergeDocSelectPage, FileSelectHdl);
    m_aBrowseDocPB.SetClickHdl(aFileSelectHdl);
    m_aBrowseTemplatePB.SetClickHdl(aFileSelectHdl);

    const uno::Sequence< ::rtl::OUString >& rDocs =
                            m_pWizard->GetConfigItem().GetSavedDocuments();
    for(sal_Int32 nDoc = 0; nDoc < rDocs.getLength(); ++nDoc)
    {
        //insert in reverse order
        m_aRecentDocLB.InsertEntry(rDocs[nDoc], 0);
    }
    m_aRecentDocLB.SelectEntryPos(0);
    if(!rDocs.getLength())
    {
        m_aRecentDocRB.Enable(sal_False);
    }
}

SwMailMergeDocSelectPage::~SwMailMergeDocSelectPage()
{
}

IMPL_LINK(SwMailMergeDocSelectPage, DocSelectHdl, RadioButton*, pButton)
{
    m_aRecentDocLB.Enable(&m_aRecentDocRB == pButton);

    m_pWizard->UpdateRoadmap();
    m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_OUTPUTTYPETPAGE));

    return 0;
}

IMPL_LINK(SwMailMergeDocSelectPage, FileSelectHdl, PushButton*, pButton)
{
    bool bTemplate = &m_aBrowseTemplatePB == pButton;

    if(bTemplate)
    {
        m_aLoadTemplateRB.Check();
        SfxNewFileDialog* pNewFileDlg = new SfxNewFileDialog(this, 0);
        sal_uInt16 nRet = pNewFileDlg->Execute();
        if(RET_TEMPLATE_LOAD == nRet)
            bTemplate = false;
        else if(RET_CANCEL != nRet)
            m_sLoadTemplateName = pNewFileDlg->GetTemplateFileName();
        delete pNewFileDlg;
    }
    else
        m_aLoadDocRB.Check();

    if(!bTemplate)
    {
        sfx2::FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
        Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

        xFP->setDisplayDirectory( SvtPathOptions().GetWorkPath() );

        SfxObjectFactory &rFact = m_pWizard->GetSwView()->GetDocShell()->GetFactory();
        SfxFilterMatcher aMatcher( rtl::OUString::createFromAscii(rFact.GetShortName()) );
        SfxFilterMatcherIter aIter( aMatcher );
        Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
        const SfxFilter* pFlt = aIter.First();
        while( pFlt )
        {
            if( pFlt && pFlt->IsAllowedAsTemplate() )
            {
                const String sWild = pFlt->GetWildcard().getGlob();
                xFltMgr->appendFilter( pFlt->GetUIName(), sWild );

                // #i40125
                if(pFlt->GetFilterFlags() & SFX_FILTER_DEFAULT)
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
    m_pWizard->enableButtons(WZB_NEXT, m_pWizard->isStateEnabled(MM_OUTPUTTYPETPAGE));

    return 0;
}

sal_Bool SwMailMergeDocSelectPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
{
    sal_Bool bReturn = sal_False;
    bool bNext = _eReason == ::svt::WizardTypes::eTravelForward;
    if(bNext || _eReason == ::svt::WizardTypes::eValidate )
    {
        ::rtl::OUString sReloadDocument;
        bReturn = m_aCurrentDocRB.IsChecked() ||
                m_aNewDocRB.IsChecked() ||
                (!(sReloadDocument = m_sLoadFileName).isEmpty() && m_aLoadDocRB.IsChecked() )||
                (!(sReloadDocument = m_sLoadTemplateName).isEmpty() && m_aLoadTemplateRB.IsChecked())||
                (m_aRecentDocRB.IsChecked() && !(sReloadDocument = m_aRecentDocLB.GetSelectEntry()).isEmpty());
        if( _eReason == ::svt::WizardTypes::eValidate )
            m_pWizard->SetDocumentLoad(!m_aCurrentDocRB.IsChecked());

        if(bNext && !m_aCurrentDocRB.IsChecked())
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
