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


#include <xmlsecurity/macrosecurity.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/biginteger.hxx>

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
#include "svtools/treelistentry.hxx"

#include <vcl/msgbox.hxx>

#include "dialogs.hrc"
#include "resourcemanager.hxx"

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;


IMPL_LINK_NOARG(MacroSecurity, OkBtnHdl)
{
    mpLevelTP->ClosePage();
    mpTrustSrcTP->ClosePage();

    EndDialog( RET_OK );

    return 0;
}

MacroSecurity::MacroSecurity( Window* _pParent,
    const css::uno::Reference< css::uno::XComponentContext> &_rxCtx,
    const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment)
    : TabDialog(_pParent, "MacroSecurityDialog", "xmlsec/ui/macrosecuritydialog.ui")
    , mxCtx(_rxCtx)
    , mxSecurityEnvironment(_rxSecurityEnvironment)
{
    get(m_pTabCtrl, "tabcontrol");
    get(m_pResetBtn, "reset");
    get(m_pOkBtn, "ok");

    mpLevelTP = new MacroSecurityLevelTP(m_pTabCtrl, this);
    mpTrustSrcTP = new MacroSecurityTrustedSourcesTP(m_pTabCtrl, this);

    m_nSecLevelId = m_pTabCtrl->GetPageId("SecurityLevelPage");
    m_nSecTrustId = m_pTabCtrl->GetPageId("SecurityTrustPage");

    m_pTabCtrl->SetTabPage(m_nSecLevelId, mpLevelTP);
    m_pTabCtrl->SetTabPage(m_nSecTrustId, mpTrustSrcTP);
    m_pTabCtrl->SetCurPageId(m_nSecLevelId);

    m_pOkBtn->SetClickHdl( LINK( this, MacroSecurity, OkBtnHdl ) );
}

MacroSecurity::~MacroSecurity()
{
    delete m_pTabCtrl->GetTabPage(m_nSecTrustId);
    delete m_pTabCtrl->GetTabPage(m_nSecLevelId);
}

MacroSecurityTP::MacroSecurityTP(Window* _pParent, const OString& rID,
    const OUString& rUIXMLDescription, MacroSecurity* _pDlg)
    : TabPage(_pParent, rID, rUIXMLDescription)
    , mpDlg(_pDlg)
{
}

MacroSecurityLevelTP::MacroSecurityLevelTP(Window* _pParent, MacroSecurity* _pDlg)
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
    sal_Bool bReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::E_MACRO_SECLEVEL );

    RadioButton* pCheck = 0;
    FixedImage* pImage = 0;
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
    if (bReadonly)
    {
        pImage->Show(true);
        m_pVeryHighRB->Enable(false);
        m_pHighRB->Enable(false);
        m_pMediumRB->Enable(false);
        m_pLowRB->Enable(false);
    }
}

IMPL_LINK_NOARG(MacroSecurityLevelTP, RadioButtonHdl)
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

    return 0;
}

void MacroSecurityLevelTP::ClosePage( void )
{
    mpDlg->maSecOptions.SetMacroSecurityLevel( mnCurLevel );
}

void MacroSecurityTrustedSourcesTP::ImplCheckButtons()
{
    bool bCertSelected = m_pTrustCertLB->FirstSelected() != NULL;
    m_pViewCertPB->Enable( bCertSelected );
    m_pRemoveCertPB->Enable( bCertSelected && !mbAuthorsReadonly);

    bool bLocationSelected = m_pTrustFileLocLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    m_pRemoveLocPB->Enable( bLocationSelected && !mbURLsReadonly);
}


IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, ViewCertPBHdl)
{
    if( m_pTrustCertLB->FirstSelected() )
    {
        sal_uInt16 nSelected = sal_uInt16( sal_uIntPtr( m_pTrustCertLB->FirstSelected()->GetUserData() ) );

        uno::Reference< css::security::XSerialNumberAdapter > xSerialNumberAdapter =
            ::com::sun::star::security::SerialNumberAdapter::create(mpDlg->mxCtx);

        uno::Reference< css::security::XCertificate > xCert = mpDlg->mxSecurityEnvironment->getCertificate( maTrustedAuthors[nSelected][0], xSerialNumberAdapter->toSequence( maTrustedAuthors[nSelected][1] ) );

        // If we don't get it, create it from signature data:
        if ( !xCert.is() )
            xCert = mpDlg->mxSecurityEnvironment->createCertificateFromAscii( maTrustedAuthors[nSelected][2] ) ;

        DBG_ASSERT( xCert.is(), "*MacroSecurityTrustedSourcesTP::ViewCertPBHdl(): Certificate not found and can't be created!" );

        if ( xCert.is() )
        {
            CertificateViewer aViewer( this, mpDlg->mxSecurityEnvironment, xCert, false );
            aViewer.Execute();
        }
    }
    return 0;
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, RemoveCertPBHdl)
{
    if( m_pTrustCertLB->FirstSelected() )
    {
        sal_uInt16 nAuthor = sal_uInt16( sal_uIntPtr( m_pTrustCertLB->FirstSelected()->GetUserData() ) );
        ::comphelper::removeElementAt( maTrustedAuthors, nAuthor );

        FillCertLB();
        ImplCheckButtons();
    }

    return 0;
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, AddLocPBHdl)
{
    try
    {
        uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference < ui::dialogs::XFolderPicker2 > xFolderPicker = ui::dialogs::FolderPicker::create(xContext);

        short nRet = xFolderPicker->execute();

        if( ui::dialogs::ExecutableDialogResults::OK != nRet )
            return 0;

        OUString aPathStr = xFolderPicker->getDirectory();
        INetURLObject aNewObj( aPathStr );
        aNewObj.removeFinalSlash();

        // then the new path also an URL else system path
        OUString aSystemFileURL = ( aNewObj.GetProtocol() != INET_PROT_NOT_VALID ) ?
            aPathStr : aNewObj.getFSysPath( INetURLObject::FSYS_DETECT );

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

    return 0;
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, RemoveLocPBHdl)
{
    sal_uInt16  nSel = m_pTrustFileLocLB->GetSelectEntryPos();
    if( nSel != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pTrustFileLocLB->RemoveEntry( nSel );
        // Trusted Path could not be removed (#i33584#)
        // after remove an entry, select another one if exists
        sal_uInt16 nNewCount = m_pTrustFileLocLB->GetEntryCount();
        if ( nNewCount > 0 )
        {
            if ( nSel >= nNewCount )
                nSel = nNewCount - 1;
            m_pTrustFileLocLB->SelectEntryPos( nSel );
        }
        ImplCheckButtons();
    }

    return 0;
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, TrustCertLBSelectHdl)
{
    ImplCheckButtons();
    return 0;
}

IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, TrustFileLocLBSelectHdl)
{
    ImplCheckButtons();
    return 0;
}

void MacroSecurityTrustedSourcesTP::FillCertLB( void )
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
            pLBEntry->SetUserData( ( void* ) (sal_IntPtr)nEntry );      // missuse user data as index
        }
    }
}

class TrustCertLB : public SvSimpleTable
{
public:
    TrustCertLB(SvSimpleTableContainer &rContainer)
        : SvSimpleTable(rContainer, 0)
    {
    }
    virtual void Resize()
    {
        SvSimpleTable::Resize();
        if (isInitialLayout(this))
        {
            const long nControlWidth = GetSizePixel().Width();
            long aTabLocs[] = { 3, 0, 35*nControlWidth/100, 70*nControlWidth/100 };
            SvSimpleTable::SetTabs(aTabLocs, MAP_PIXEL);
        }
    }
};

MacroSecurityTrustedSourcesTP::MacroSecurityTrustedSourcesTP(Window* _pParent, MacroSecurity* _pDlg)
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
    m_pTrustCertLB = new TrustCertLB(*pCertificates);
    static long aTabs[] = { 3, 0, 0, 0 };
    m_pTrustCertLB->SetTabs( aTabs );

    OUStringBuffer aBuf(get<FixedText>("to")->GetText());
    aBuf.append("\t").append(get<FixedText>("by")->GetText())
        .append("\t").append(get<FixedText>("date")->GetText());
    m_pTrustCertLB->InsertHeaderEntry(aBuf.makeStringAndClear());

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
    mbAuthorsReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::E_MACRO_TRUSTEDAUTHORS );
    m_pTrustCertROFI->Show( mbAuthorsReadonly );
    mbAuthorsReadonly ? m_pTrustCertLB->DisableTable() : m_pTrustCertLB->EnableTable();

    FillCertLB();

    css::uno::Sequence< OUString > aSecureURLs = mpDlg->maSecOptions.GetSecureURLs();
    mbURLsReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::E_SECUREURLS );
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
    delete m_pTrustCertLB;
}

void MacroSecurityTrustedSourcesTP::ActivatePage()
{
    mpDlg->EnableReset( false );
    FillCertLB();
}

void MacroSecurityTrustedSourcesTP::ClosePage( void )
{
    sal_uInt16  nEntryCnt = m_pTrustFileLocLB->GetEntryCount();
    if( nEntryCnt )
    {
        css::uno::Sequence< OUString > aSecureURLs( nEntryCnt );
        for( sal_uInt16 i = 0 ; i < nEntryCnt ; ++i )
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
