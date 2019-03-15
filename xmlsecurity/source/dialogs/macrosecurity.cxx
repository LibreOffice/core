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


#include <macrosecurity.hxx>
#include <certificatechooser.hxx>
#include <certificateviewer.hxx>
#include <biginteger.hxx>

#include <osl/file.hxx>
#include <sal/log.hxx>

#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <comphelper/sequence.hxx>
#include <sfx2/filedlghelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <unotools/datetime.hxx>

#include <vcl/svapp.hxx>

#include <strings.hrc>

using namespace comphelper;
using namespace ::com::sun::star;


IMPL_LINK_NOARG(MacroSecurity, OkBtnHdl, weld::Button&, void)
{
    m_xLevelTP->ClosePage();
    m_xTrustSrcTP->ClosePage();
    m_xDialog->response(RET_OK);
}

MacroSecurity::MacroSecurity(weld::Window* pParent,
    const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& rxSecurityEnvironment)
    : GenericDialogController(pParent, "xmlsec/ui/macrosecuritydialog.ui", "MacroSecurityDialog")
    , m_xSecurityEnvironment(rxSecurityEnvironment)
    , m_xTabCtrl(m_xBuilder->weld_notebook("tabcontrol"))
    , m_xOkBtn(m_xBuilder->weld_button("ok"))
    , m_xResetBtn(m_xBuilder->weld_button("reset"))
{
    m_xTabCtrl->connect_enter_page(LINK(this, MacroSecurity, ActivatePageHdl));

    m_xLevelTP.reset(new MacroSecurityLevelTP(m_xTabCtrl->get_page("SecurityLevelPage"), this));
    m_xTrustSrcTP.reset(new MacroSecurityTrustedSourcesTP(m_xTabCtrl->get_page("SecurityTrustPage"), this));

    m_xTabCtrl->set_current_page("SecurityLevelPage");
    m_xOkBtn->connect_clicked(LINK(this, MacroSecurity, OkBtnHdl));
}

IMPL_LINK(MacroSecurity, ActivatePageHdl, const OString&, rPage, void)
{
    if (rPage == "SecurityLevelPage")
        m_xLevelTP->ActivatePage();
    else if (rPage == "SecurityTrustPage")
        m_xTrustSrcTP->ActivatePage();
}

MacroSecurityTP::MacroSecurityTP(weld::Container* pParent, const OUString& rUIXMLDescription,
                                 const OString& rID, MacroSecurity* pDlg)
    : m_xBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , m_xContainer(m_xBuilder->weld_container(rID))
    , m_pDlg(pDlg)
{
}

void MacroSecurityTP::ActivatePage()
{
}

MacroSecurityTP::~MacroSecurityTP()
{
}

MacroSecurityLevelTP::MacroSecurityLevelTP(weld::Container* pParent, MacroSecurity* pDlg)
    : MacroSecurityTP(pParent, "xmlsec/ui/securitylevelpage.ui", "SecurityLevelPage", pDlg)
    , m_xVeryHighRB(m_xBuilder->weld_radio_button("vhigh"))
    , m_xHighRB(m_xBuilder->weld_radio_button("high"))
    , m_xMediumRB(m_xBuilder->weld_radio_button("med"))
    , m_xLowRB(m_xBuilder->weld_radio_button("low"))
    , m_xVHighImg(m_xBuilder->weld_widget("vhighimg"))
    , m_xHighImg(m_xBuilder->weld_widget("highimg"))
    , m_xMedImg(m_xBuilder->weld_widget("medimg"))
    , m_xLowImg(m_xBuilder->weld_widget("lowimg"))
{
    m_xLowRB->connect_toggled( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    m_xMediumRB->connect_toggled( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    m_xHighRB->connect_toggled( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    m_xVeryHighRB->connect_toggled( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );

    int nPrefWidth(std::max({m_xVeryHighRB->get_preferred_size().Width(),
                             m_xHighRB->get_preferred_size().Width(),
                             m_xMediumRB->get_preferred_size().Width(),
                             m_xLowRB->get_preferred_size().Width()}));
    int nMaxWidth = m_xLowRB->get_approximate_digit_width() * 60;
    if (nPrefWidth > nMaxWidth)
    {
        m_xLowRB->set_label_line_wrap(true);
        m_xLowRB->set_size_request(nMaxWidth, -1);
        m_xMediumRB->set_label_line_wrap(true);
        m_xMediumRB->set_size_request(nMaxWidth, -1);
        m_xHighRB->set_label_line_wrap(true);
        m_xHighRB->set_size_request(nMaxWidth, -1);
        m_xVeryHighRB->set_label_line_wrap(true);
        m_xVeryHighRB->set_size_request(nMaxWidth, -1);
    }

    mnCurLevel = static_cast<sal_uInt16>(m_pDlg->m_aSecOptions.GetMacroSecurityLevel());
    bool bReadonly = m_pDlg->m_aSecOptions.IsReadOnly( SvtSecurityOptions::EOption::MacroSecLevel );

    weld::RadioButton* pCheck = nullptr;
    weld::Widget* pImage = nullptr;
    switch (mnCurLevel)
    {
        case 3:
            pCheck = m_xVeryHighRB.get();
            pImage = m_xVHighImg.get();
            break;
        case 2:
            pCheck = m_xHighRB.get();
            pImage = m_xHighImg.get();
            break;
        case 1:
            pCheck = m_xMediumRB.get();
            pImage = m_xMedImg.get();
            break;
        case 0:
            pCheck = m_xLowRB.get();
            pImage = m_xLowImg.get();
            break;
    }
    if (pCheck)
        pCheck->set_active(true);
    else
    {
        OSL_FAIL("illegal macro security level");
    }
    if (bReadonly && pImage)
    {
        pImage->show();
        m_xVeryHighRB->set_sensitive(false);
        m_xHighRB->set_sensitive(false);
        m_xMediumRB->set_sensitive(false);
        m_xLowRB->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(MacroSecurityLevelTP, RadioButtonHdl, weld::ToggleButton&, void)
{
    sal_uInt16 nNewLevel = 0;
    if( m_xVeryHighRB->get_active() )
        nNewLevel = 3;
    else if( m_xHighRB->get_active() )
        nNewLevel = 2;
    else if( m_xMediumRB->get_active() )
        nNewLevel = 1;

    if ( nNewLevel != mnCurLevel )
    {
        mnCurLevel = nNewLevel;
        m_pDlg->EnableReset();
    }
}

void MacroSecurityLevelTP::ClosePage()
{
    m_pDlg->m_aSecOptions.SetMacroSecurityLevel( mnCurLevel );
}

void MacroSecurityTrustedSourcesTP::ImplCheckButtons()
{
    bool bCertSelected = m_xTrustCertLB->get_selected_index() != -1;
    m_xViewCertPB->set_sensitive( bCertSelected );
    m_xRemoveCertPB->set_sensitive( bCertSelected && !mbAuthorsReadonly);

    bool bLocationSelected = m_xTrustFileLocLB->get_selected_index() != -1;
    m_xRemoveLocPB->set_sensitive( bLocationSelected && !mbURLsReadonly);
}


IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, ViewCertPBHdl, weld::Button&, void)
{
    int nEntry = m_xTrustCertLB->get_selected_index();
    if (nEntry != -1)
    {
        sal_uInt16 nSelected = m_xTrustCertLB->get_id(nEntry).toUInt32();

        uno::Reference< css::security::XCertificate > xCert = m_pDlg->m_xSecurityEnvironment->getCertificate( m_aTrustedAuthors[nSelected][0], xmlsecurity::numericStringToBigInteger( m_aTrustedAuthors[nSelected][1] ) );

        // If we don't get it, create it from signature data:
        if ( !xCert.is() )
            xCert = m_pDlg->m_xSecurityEnvironment->createCertificateFromAscii( m_aTrustedAuthors[nSelected][2] ) ;

        SAL_WARN_IF( !xCert.is(), "xmlsecurity.dialogs", "*MacroSecurityTrustedSourcesTP::ViewCertPBHdl(): Certificate not found and can't be created!" );

        if ( xCert.is() )
        {
            CertificateViewer aViewer(m_pDlg->getDialog(), m_pDlg->m_xSecurityEnvironment, xCert, false, nullptr);
            aViewer.run();
        }
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, RemoveCertPBHdl, weld::Button&, void)
{
    int nEntry = m_xTrustCertLB->get_selected_index();
    if (nEntry != -1)
    {
        sal_uInt16 nAuthor = m_xTrustCertLB->get_id(nEntry).toUInt32();
        ::comphelper::removeElementAt( m_aTrustedAuthors, nAuthor );

        FillCertLB();
        ImplCheckButtons();
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, AddLocPBHdl, weld::Button&, void)
{
    try
    {
        uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference < ui::dialogs::XFolderPicker2 > xFolderPicker = ui::dialogs::FolderPicker::create(xContext);

        short nRet = xFolderPicker->execute();

        if( ui::dialogs::ExecutableDialogResults::OK != nRet )
            return;

        OUString aPathStr = xFolderPicker->getDirectory();
        INetURLObject aNewObj( aPathStr );
        aNewObj.removeFinalSlash();

        // then the new path also an URL else system path
        OUString aSystemFileURL = ( aNewObj.GetProtocol() != INetProtocol::NotValid ) ?
            aPathStr : aNewObj.getFSysPath( FSysStyle::Detect );

        OUString aNewPathStr(aSystemFileURL);

        if ( osl::FileBase::getSystemPathFromFileURL( aSystemFileURL, aSystemFileURL ) == osl::FileBase::E_None )
            aNewPathStr = aSystemFileURL;

        if (m_xTrustFileLocLB->find_text(aNewPathStr) == -1)
            m_xTrustFileLocLB->append_text(aNewPathStr);

        ImplCheckButtons();
    }
    catch( uno::Exception& )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "xmlsecurity.dialogs", "MacroSecurityTrustedSourcesTP::AddLocPBHdl(): exception from folder picker" << exceptionToString(ex) );
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, RemoveLocPBHdl, weld::Button&, void)
{
    sal_Int32 nSel = m_xTrustFileLocLB->get_selected_index();
    if (nSel != -1)
    {
        m_xTrustFileLocLB->remove(nSel);
        // Trusted Path could not be removed (#i33584#)
        // after remove an entry, select another one if exists
        int nNewCount = m_xTrustFileLocLB->n_children();
        if (nNewCount > 0)
        {
            if (nSel >= nNewCount)
                nSel = nNewCount - 1;
            m_xTrustFileLocLB->select(nSel);
        }
        ImplCheckButtons();
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, TrustCertLBSelectHdl, weld::TreeView&, void)
{
    ImplCheckButtons();
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, TrustFileLocLBSelectHdl, weld::TreeView&, void)
{
    ImplCheckButtons();
}

void MacroSecurityTrustedSourcesTP::FillCertLB()
{
    m_xTrustCertLB->clear();

    sal_uInt32 nEntries = m_aTrustedAuthors.getLength();

    if ( nEntries && m_pDlg->m_xSecurityEnvironment.is() )
    {
        for( sal_uInt32 nEntry = 0 ; nEntry < nEntries ; ++nEntry )
        {
            css::uno::Sequence< OUString >&              rEntry = m_aTrustedAuthors[ nEntry ];
            uno::Reference< css::security::XCertificate >   xCert;

            // create from RawData
            xCert = m_pDlg->m_xSecurityEnvironment->createCertificateFromAscii( rEntry[ 2 ] );

            m_xTrustCertLB->append(OUString::number(nEntry), xmlsec::GetContentPart(xCert->getSubjectName()));
            m_xTrustCertLB->set_text(nEntry, xmlsec::GetContentPart(xCert->getIssuerName()), 1);
            m_xTrustCertLB->set_text(nEntry, utl::GetDateTimeString(xCert->getNotValidAfter()), 2);
        }
    }
}

MacroSecurityTrustedSourcesTP::MacroSecurityTrustedSourcesTP(weld::Container* pParent, MacroSecurity* pDlg)
    : MacroSecurityTP(pParent, "xmlsec/ui/securitytrustpage.ui", "SecurityTrustPage", pDlg)
    , m_xTrustCertROFI(m_xBuilder->weld_image("lockcertimg"))
    , m_xTrustCertLB(m_xBuilder->weld_tree_view("certificates"))
    , m_xViewCertPB(m_xBuilder->weld_button("viewcert"))
    , m_xRemoveCertPB(m_xBuilder->weld_button("removecert"))
    , m_xTrustFileROFI(m_xBuilder->weld_image("lockfileimg"))
    , m_xTrustFileLocLB(m_xBuilder->weld_tree_view("locations"))
    , m_xAddLocPB(m_xBuilder->weld_button("addfile"))
    , m_xRemoveLocPB(m_xBuilder->weld_button("removefile"))
{
    auto nColWidth = m_xTrustCertLB->get_approximate_digit_width() * 12;
    std::vector<int> aWidths;
    aWidths.push_back(nColWidth * 2);
    aWidths.push_back(nColWidth * 2);
    m_xTrustCertLB->set_column_fixed_widths(aWidths);
    m_xTrustCertLB->set_size_request(nColWidth * 5.5, m_xTrustCertLB->get_height_rows(5));

    m_xTrustCertLB->connect_changed( LINK( this, MacroSecurityTrustedSourcesTP, TrustCertLBSelectHdl ) );
    m_xViewCertPB->connect_clicked( LINK( this, MacroSecurityTrustedSourcesTP, ViewCertPBHdl ) );
    m_xViewCertPB->set_sensitive(false);
    m_xRemoveCertPB->connect_clicked( LINK( this, MacroSecurityTrustedSourcesTP, RemoveCertPBHdl ) );
    m_xRemoveCertPB->set_sensitive(false);

    m_xTrustFileLocLB->connect_changed( LINK( this, MacroSecurityTrustedSourcesTP, TrustFileLocLBSelectHdl ) );
    m_xTrustFileLocLB->set_size_request(nColWidth * 5, m_xTrustFileLocLB->get_height_rows(5));
    m_xAddLocPB->connect_clicked( LINK( this, MacroSecurityTrustedSourcesTP, AddLocPBHdl ) );
    m_xRemoveLocPB->connect_clicked( LINK( this, MacroSecurityTrustedSourcesTP, RemoveLocPBHdl ) );
    m_xRemoveLocPB->set_sensitive(false);

    m_aTrustedAuthors = m_pDlg->m_aSecOptions.GetTrustedAuthors();
    mbAuthorsReadonly = m_pDlg->m_aSecOptions.IsReadOnly( SvtSecurityOptions::EOption::MacroTrustedAuthors );
    m_xTrustCertROFI->set_visible(mbAuthorsReadonly);
    m_xTrustCertLB->set_sensitive(!mbAuthorsReadonly);

    FillCertLB();

    css::uno::Sequence< OUString > aSecureURLs = m_pDlg->m_aSecOptions.GetSecureURLs();
    mbURLsReadonly = m_pDlg->m_aSecOptions.IsReadOnly( SvtSecurityOptions::EOption::SecureUrls );
    m_xTrustFileROFI->set_visible(mbURLsReadonly);
    m_xTrustFileLocLB->set_sensitive(!mbURLsReadonly);
    m_xAddLocPB->set_sensitive(!mbURLsReadonly);

    sal_Int32 nEntryCnt = aSecureURLs.getLength();
    for (sal_Int32 i = 0; i < nEntryCnt; ++i)
    {
        OUString aSystemFileURL( aSecureURLs[ i ] );
        osl::FileBase::getSystemPathFromFileURL( aSystemFileURL, aSystemFileURL );
        m_xTrustFileLocLB->append_text(aSystemFileURL);
    }
}

void MacroSecurityTrustedSourcesTP::ActivatePage()
{
    m_pDlg->EnableReset( false );
    FillCertLB();
}

void MacroSecurityTrustedSourcesTP::ClosePage()
{
    sal_Int32 nEntryCnt = m_xTrustFileLocLB->n_children();
    if( nEntryCnt )
    {
        css::uno::Sequence< OUString > aSecureURLs( nEntryCnt );
        for (sal_Int32 i = 0; i < nEntryCnt; ++i)
        {
            OUString aURL(m_xTrustFileLocLB->get_text(i));
            osl::FileBase::getFileURLFromSystemPath( aURL, aURL );
            aSecureURLs[ i ] = aURL;
        }

        m_pDlg->m_aSecOptions.SetSecureURLs( aSecureURLs );
    }
    // Trusted Path could not be removed (#i33584#)
    // don't forget to remove the old saved SecureURLs
    else
        m_pDlg->m_aSecOptions.SetSecureURLs( css::uno::Sequence< OUString >() );

    m_pDlg->m_aSecOptions.SetTrustedAuthors( m_aTrustedAuthors );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
