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
#include <vcl/help.hxx>
#include <vcl/layout.hxx>


#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <comphelper/sequence.hxx>
#include <sfx2/filedlghelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <tools/urlobj.hxx>
#include <svtools/treelistentry.hxx>

#include <vcl/msgbox.hxx>

#include "dialogs.hrc"
#include "resourcemanager.hxx"

using namespace ::com::sun::star;


IMPL_LINK_NOARG(MacroSecurity, OkBtnHdl, Button*, void)
{
    mpLevelTP->ClosePage();
    mpTrustSrcTP->ClosePage();

    EndDialog( RET_OK );
}

MacroSecurity::MacroSecurity( vcl::Window* _pParent,
    const css::uno::Reference< css::uno::XComponentContext> &_rxCtx,
    const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment)
    : TabDialog(_pParent, "MacroSecurityDialog", "xmlsec/ui/macrosecuritydialog.ui")
    , mxCtx(_rxCtx)
    , mxSecurityEnvironment(_rxSecurityEnvironment)
{
    get(m_pTabCtrl, "tabcontrol");
    get(m_pResetBtn, "reset");
    get(m_pOkBtn, "ok");

    mpLevelTP.reset(VclPtr<MacroSecurityLevelTP>::Create(m_pTabCtrl, this));
    mpTrustSrcTP.reset(VclPtr<MacroSecurityTrustedSourcesTP>::Create(m_pTabCtrl, this));

    m_nSecLevelId = m_pTabCtrl->GetPageId("SecurityLevelPage");
    m_nSecTrustId = m_pTabCtrl->GetPageId("SecurityTrustPage");

    m_pTabCtrl->SetTabPage(m_nSecLevelId, mpLevelTP);
    m_pTabCtrl->SetTabPage(m_nSecTrustId, mpTrustSrcTP);
    m_pTabCtrl->SetCurPageId(m_nSecLevelId);

    m_pOkBtn->SetClickHdl( LINK( this, MacroSecurity, OkBtnHdl ) );
}

MacroSecurity::~MacroSecurity()
{
    disposeOnce();
}

void MacroSecurity::dispose()
{
    m_pTabCtrl->GetTabPage(m_nSecTrustId)->disposeOnce();
    m_pTabCtrl->GetTabPage(m_nSecLevelId)->disposeOnce();
    m_pTabCtrl.clear();
    m_pOkBtn.clear();
    m_pResetBtn.clear();
    mpLevelTP.disposeAndClear();
    mpTrustSrcTP.disposeAndClear();
    TabDialog::dispose();
}

MacroSecurityTP::MacroSecurityTP(vcl::Window* _pParent, const OString& rID,
    const OUString& rUIXMLDescription, MacroSecurity* _pDlg)
    : TabPage(_pParent, rID, rUIXMLDescription)
    , mpDlg(_pDlg)
{
}

MacroSecurityTP::~MacroSecurityTP()
{
    disposeOnce();
}

void MacroSecurityTP::dispose()
{
    mpDlg.clear();
    TabPage::dispose();
}

MacroSecurityLevelTP::MacroSecurityLevelTP(vcl::Window* _pParent, MacroSecurity* _pDlg)
    : MacroSecurityTP(_pParent, "SecurityLevelPage", "xmlsec/ui/securitylevelpage.ui", _pDlg)
{
    get(m_pVeryHighRB, "vhigh");
    get(m_pHighRB, "high");
    get(m_pMediumRB, "med");
    get(m_pLowRB, "low");

    m_pLowRB->SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    m_pMediumRB->SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    m_pHighRB->SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    m_pVeryHighRB->SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );

    mnCurLevel = (sal_uInt16) mpDlg->maSecOptions.GetMacroSecurityLevel();
    bool bReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::EOption::MacroSecLevel );

    RadioButton* pCheck = nullptr;
    FixedImage* pImage = nullptr;
    switch (mnCurLevel)
    {
        case 3:
            pCheck = m_pVeryHighRB;
            pImage = get<FixedImage>("vhighimg");
            break;
        case 2:
            pCheck = m_pHighRB;
            pImage = get<FixedImage>("highimg");
            break;
        case 1:
            pCheck = m_pMediumRB;
            pImage = get<FixedImage>("medimg");
            break;
        case 0:
            pCheck = m_pLowRB;
            pImage = get<FixedImage>("lowimg");
            break;
    }
    if (pCheck)
        pCheck->Check();
    else
    {
        OSL_FAIL("illegal macro security level");
    }
    if (bReadonly && pImage)
    {
        pImage->Show();
        m_pVeryHighRB->Enable(false);
        m_pHighRB->Enable(false);
        m_pMediumRB->Enable(false);
        m_pLowRB->Enable(false);
    }
}

MacroSecurityLevelTP::~MacroSecurityLevelTP()
{
    disposeOnce();
}

void MacroSecurityLevelTP::dispose()
{
    m_pVeryHighRB.clear();
    m_pHighRB.clear();
    m_pMediumRB.clear();
    m_pLowRB.clear();
    MacroSecurityTP::dispose();
}

IMPL_LINK_NOARG(MacroSecurityLevelTP, RadioButtonHdl, Button*, void)
{
    sal_uInt16 nNewLevel = 0;
    if( m_pVeryHighRB->IsChecked() )
        nNewLevel = 3;
    else if( m_pHighRB->IsChecked() )
        nNewLevel = 2;
    else if( m_pMediumRB->IsChecked() )
        nNewLevel = 1;

    if ( nNewLevel != mnCurLevel )
    {
        mnCurLevel = nNewLevel;
        mpDlg->EnableReset();
    }
}

void MacroSecurityLevelTP::ClosePage()
{
    mpDlg->maSecOptions.SetMacroSecurityLevel( mnCurLevel );
}

void MacroSecurityTrustedSourcesTP::ImplCheckButtons()
{
    bool bCertSelected = m_pTrustCertLB->FirstSelected() != nullptr;
    m_pViewCertPB->Enable( bCertSelected );
    m_pRemoveCertPB->Enable( bCertSelected && !mbAuthorsReadonly);

    bool bLocationSelected = m_pTrustFileLocLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    m_pRemoveLocPB->Enable( bLocationSelected && !mbURLsReadonly);
}


IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, ViewCertPBHdl, Button*, void)
{
    if( m_pTrustCertLB->FirstSelected() )
    {
        sal_uInt16 nSelected = sal_uInt16( sal_uIntPtr( m_pTrustCertLB->FirstSelected()->GetUserData() ) );

        uno::Reference< css::security::XSerialNumberAdapter > xSerialNumberAdapter =
            css::security::SerialNumberAdapter::create(mpDlg->mxCtx);

        uno::Reference< css::security::XCertificate > xCert = mpDlg->mxSecurityEnvironment->getCertificate( maTrustedAuthors[nSelected][0], xSerialNumberAdapter->toSequence( maTrustedAuthors[nSelected][1] ) );

        // If we don't get it, create it from signature data:
        if ( !xCert.is() )
            xCert = mpDlg->mxSecurityEnvironment->createCertificateFromAscii( maTrustedAuthors[nSelected][2] ) ;

        SAL_WARN_IF( !xCert.is(), "xmlsecurity.dialogs", "*MacroSecurityTrustedSourcesTP::ViewCertPBHdl(): Certificate not found and can't be created!" );

        if ( xCert.is() )
        {
            ScopedVclPtrInstance< CertificateViewer > aViewer( this, mpDlg->mxSecurityEnvironment, xCert, false );
            aViewer->Execute();
        }
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, RemoveCertPBHdl, Button*, void)
{
    if( m_pTrustCertLB->FirstSelected() )
    {
        sal_uInt16 nAuthor = sal_uInt16( sal_uIntPtr( m_pTrustCertLB->FirstSelected()->GetUserData() ) );
        ::comphelper::removeElementAt( maTrustedAuthors, nAuthor );

        FillCertLB();
        ImplCheckButtons();
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, AddLocPBHdl, Button*, void)
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

        if( m_pTrustFileLocLB->GetEntryPos( aNewPathStr ) == LISTBOX_ENTRY_NOTFOUND )
        {
            m_pTrustFileLocLB->InsertEntry( aNewPathStr );
        }

        ImplCheckButtons();
    }
    catch( uno::Exception& )
    {
        SAL_WARN( "xmlsecurity.dialogs", "MacroSecurityTrustedSourcesTP::AddLocPBHdl(): exception from folder picker" );
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, RemoveLocPBHdl, Button*, void)
{
    sal_Int32  nSel = m_pTrustFileLocLB->GetSelectEntryPos();
    if( nSel != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pTrustFileLocLB->RemoveEntry( nSel );
        // Trusted Path could not be removed (#i33584#)
        // after remove an entry, select another one if exists
        sal_Int32 nNewCount = m_pTrustFileLocLB->GetEntryCount();
        if ( nNewCount > 0 )
        {
            if ( nSel >= nNewCount )
                nSel = nNewCount - 1;
            m_pTrustFileLocLB->SelectEntryPos( nSel );
        }
        ImplCheckButtons();
    }
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, TrustCertLBSelectHdl, SvTreeListBox*, void)
{
    ImplCheckButtons();
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, TrustFileLocLBSelectHdl, ListBox&, void)
{
    ImplCheckButtons();
}

void MacroSecurityTrustedSourcesTP::FillCertLB()
{
    m_pTrustCertLB->Clear();

    sal_uInt32 nEntries = maTrustedAuthors.getLength();

    if ( nEntries && mpDlg->mxSecurityEnvironment.is() )
    {
        for( sal_uInt32 nEntry = 0 ; nEntry < nEntries ; ++nEntry )
        {
            css::uno::Sequence< OUString >&              rEntry = maTrustedAuthors[ nEntry ];
            uno::Reference< css::security::XCertificate >   xCert;

            // create from RawData
            xCert = mpDlg->mxSecurityEnvironment->createCertificateFromAscii( rEntry[ 2 ] );

            SvTreeListEntry*    pLBEntry = m_pTrustCertLB->InsertEntry( XmlSec::GetContentPart( xCert->getSubjectName() ) );
            m_pTrustCertLB->SetEntryText( XmlSec::GetContentPart( xCert->getIssuerName() ), pLBEntry, 1 );
            m_pTrustCertLB->SetEntryText( XmlSec::GetDateTimeString( xCert->getNotValidAfter() ), pLBEntry, 2 );
            pLBEntry->SetUserData( reinterpret_cast<void*>(nEntry) );      // missuse user data as index
        }
    }
}

class TrustCertLB : public SvSimpleTable
{
public:
    explicit TrustCertLB(SvSimpleTableContainer &rContainer)
        : SvSimpleTable(rContainer, 0)
    {
    }
    virtual void Resize() override
    {
        SvSimpleTable::Resize();
        if (isInitialLayout(this))
        {
            const long nControlWidth = GetSizePixel().Width();
            long aTabLocs[] = { 3, 0, 35*nControlWidth/100, 70*nControlWidth/100 };
            SvSimpleTable::SetTabs(aTabLocs, MapUnit::MapPixel);
        }
    }
};

MacroSecurityTrustedSourcesTP::MacroSecurityTrustedSourcesTP(vcl::Window* _pParent, MacroSecurity* _pDlg)
    : MacroSecurityTP(_pParent, "SecurityTrustPage", "xmlsec/ui/securitytrustpage.ui", _pDlg)
{
    get(m_pTrustCertROFI, "lockcertimg");
    get(m_pViewCertPB, "viewcert");
    get(m_pRemoveCertPB, "removecert");
    get(m_pTrustFileROFI, "lockfileimg");
    get(m_pTrustFileLocLB, "locations");
    m_pTrustFileLocLB->SetDropDownLineCount(6);
    get(m_pAddLocPB, "addfile");
    get(m_pRemoveLocPB, "removefile");

    SvSimpleTableContainer *pCertificates = get<SvSimpleTableContainer>("certificates");
    m_pTrustCertLB.reset(VclPtr<TrustCertLB>::Create(*pCertificates));
    static long aTabs[] = { 3, 0, 0, 0 };
    m_pTrustCertLB->SetTabs( aTabs );

    m_pTrustCertLB->InsertHeaderEntry(get<FixedText>("to")->GetText() + "\t"
        +  get<FixedText>("by")->GetText() + "\t" + get<FixedText>("date")->GetText());

    m_pTrustCertLB->SetSelectHdl( LINK( this, MacroSecurityTrustedSourcesTP, TrustCertLBSelectHdl ) );
    m_pViewCertPB->SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, ViewCertPBHdl ) );
    m_pViewCertPB->Disable();
    m_pRemoveCertPB->SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, RemoveCertPBHdl ) );
    m_pRemoveCertPB->Disable();

    m_pTrustFileLocLB->SetSelectHdl( LINK( this, MacroSecurityTrustedSourcesTP, TrustFileLocLBSelectHdl ) );
    m_pAddLocPB->SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, AddLocPBHdl ) );
    m_pRemoveLocPB->SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, RemoveLocPBHdl ) );
    m_pRemoveLocPB->Disable();

    maTrustedAuthors = mpDlg->maSecOptions.GetTrustedAuthors();
    mbAuthorsReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::EOption::MacroTrustedAuthors );
    m_pTrustCertROFI->Show( mbAuthorsReadonly );
    mbAuthorsReadonly ? m_pTrustCertLB->DisableTable() : m_pTrustCertLB->EnableTable();

    FillCertLB();

    css::uno::Sequence< OUString > aSecureURLs = mpDlg->maSecOptions.GetSecureURLs();
    mbURLsReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::EOption::SecureUrls );
    m_pTrustFileROFI->Show( mbURLsReadonly );
    m_pTrustFileLocLB->Enable( !mbURLsReadonly );
    m_pAddLocPB->Enable( !mbURLsReadonly );

    sal_Int32 nEntryCnt = aSecureURLs.getLength();
    for( sal_Int32 i = 0 ; i < nEntryCnt ; ++i )
    {
        OUString aSystemFileURL( aSecureURLs[ i ] );
        osl::FileBase::getSystemPathFromFileURL( aSystemFileURL, aSystemFileURL );
        m_pTrustFileLocLB->InsertEntry( aSystemFileURL );
    }
}

MacroSecurityTrustedSourcesTP::~MacroSecurityTrustedSourcesTP()
{
    disposeOnce();
}

void MacroSecurityTrustedSourcesTP::dispose()
{
    m_pTrustCertLB.disposeAndClear();
    m_pTrustCertROFI.clear();
    m_pViewCertPB.clear();
    m_pRemoveCertPB.clear();
    m_pTrustFileROFI.clear();
    m_pTrustFileLocLB.clear();
    m_pAddLocPB.clear();
    m_pRemoveLocPB.clear();
    MacroSecurityTP::dispose();
}

void MacroSecurityTrustedSourcesTP::ActivatePage()
{
    mpDlg->EnableReset( false );
    FillCertLB();
}

void MacroSecurityTrustedSourcesTP::ClosePage()
{
    sal_Int32  nEntryCnt = m_pTrustFileLocLB->GetEntryCount();
    if( nEntryCnt )
    {
        css::uno::Sequence< OUString > aSecureURLs( nEntryCnt );
        for( sal_Int32 i = 0 ; i < nEntryCnt ; ++i )
        {
            OUString aURL( m_pTrustFileLocLB->GetEntry( i ) );
            osl::FileBase::getFileURLFromSystemPath( aURL, aURL );
            aSecureURLs[ i ] = aURL;
        }

        mpDlg->maSecOptions.SetSecureURLs( aSecureURLs );
    }
    // Trusted Path could not be removed (#i33584#)
    // don't forget to remove the old saved SecureURLs
    else
        mpDlg->maSecOptions.SetSecureURLs( css::uno::Sequence< OUString >() );

    mpDlg->maSecOptions.SetTrustedAuthors( maTrustedAuthors );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
