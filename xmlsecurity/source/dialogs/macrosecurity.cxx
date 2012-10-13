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


#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <comphelper/sequence.hxx>
#include <sfx2/filedlghelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <tools/urlobj.hxx>

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

MacroSecurity::MacroSecurity( Window* _pParent, const cssu::Reference< cssu::XComponentContext> &_rxCtx, const cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment )
    :TabDialog          ( _pParent, XMLSEC_RES( RID_XMLSECTP_MACROSEC ) )
    ,maTabCtrl          ( this, XMLSEC_RES( 1 ) )
    ,maOkBtn            ( this, XMLSEC_RES( BTN_OK ) )
    ,maCancelBtn        ( this, XMLSEC_RES( BTN_CANCEL ) )
    ,maHelpBtn          ( this, XMLSEC_RES( BTN_HELP ) )
    ,maResetBtn         ( this, XMLSEC_RES( BTN_RESET ) )
{
    FreeResource();

    mxCtx = _rxCtx;
    mxSecurityEnvironment = _rxSecurityEnvironment;

    mpLevelTP = new MacroSecurityLevelTP( &maTabCtrl, this );
    mpTrustSrcTP = new MacroSecurityTrustedSourcesTP( &maTabCtrl, this );

    maTabCtrl.SetTabPage( RID_XMLSECTP_SECLEVEL, mpLevelTP );
    maTabCtrl.SetTabPage( RID_XMLSECTP_TRUSTSOURCES, mpTrustSrcTP );
    maTabCtrl.SetCurPageId( RID_XMLSECTP_SECLEVEL );

    maOkBtn.SetClickHdl( LINK( this, MacroSecurity, OkBtnHdl ) );
}

MacroSecurity::~MacroSecurity()
{
    delete maTabCtrl.GetTabPage( RID_XMLSECTP_TRUSTSOURCES );
    delete maTabCtrl.GetTabPage( RID_XMLSECTP_SECLEVEL );
}


MacroSecurityTP::MacroSecurityTP( Window* _pParent, const ResId& _rResId, MacroSecurity* _pDlg )
    :TabPage        ( _pParent, _rResId )
    ,mpDlg          ( _pDlg )
{
}

MacroSecurityLevelTP::MacroSecurityLevelTP( Window* _pParent, MacroSecurity* _pDlg )
    :MacroSecurityTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_SECLEVEL ), _pDlg )
    ,maSecLevelFL       ( this, XMLSEC_RES( FL_SECLEVEL ) )
    ,maSecReadonlyFI    ( this, XMLSEC_RES( FI_SEC_READONLY ))
    ,maVeryHighRB       ( this, XMLSEC_RES( RB_VERYHIGH ) )
    ,maHighRB           ( this, XMLSEC_RES( RB_HIGH ) )
    ,maMediumRB         ( this, XMLSEC_RES( RB_MEDIUM ) )
    ,maLowRB            ( this, XMLSEC_RES( RB_LOW ) )
{
    FreeResource();

    maLowRB.SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    maMediumRB.SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    maHighRB.SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );
    maVeryHighRB.SetClickHdl( LINK( this, MacroSecurityLevelTP, RadioButtonHdl ) );

    mnCurLevel = (sal_uInt16) mpDlg->maSecOptions.GetMacroSecurityLevel();
    sal_Bool bReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::E_MACRO_SECLEVEL );

    RadioButton* pCheck = 0;
    switch( mnCurLevel )
    {
        case 3: pCheck = &maVeryHighRB;   break;
        case 2: pCheck = &maHighRB;       break;
        case 1: pCheck = &maMediumRB;     break;
        case 0: pCheck = &maLowRB;        break;
    }
    if(pCheck)
        pCheck->Check();
    else
    {
        OSL_FAIL("illegal macro security level");
    }
    maSecReadonlyFI.Show(bReadonly);
    if(bReadonly)
    {
        //move to the selected button
        if( pCheck && pCheck != &maVeryHighRB)
        {
            long nDiff = pCheck->GetPosPixel().Y() - maVeryHighRB.GetPosPixel().Y();
            Point aPos(maSecReadonlyFI.GetPosPixel());
            aPos.Y() += nDiff;
            maSecReadonlyFI.SetPosPixel(aPos);
        }
        maVeryHighRB.Enable(sal_False);
        maHighRB.Enable(sal_False);
        maMediumRB.Enable(sal_False);
        maLowRB.Enable(sal_False);
    }

}

IMPL_LINK_NOARG(MacroSecurityLevelTP, RadioButtonHdl)
{
    sal_uInt16 nNewLevel = 0;
    if( maVeryHighRB.IsChecked() )
        nNewLevel = 3;
    else if( maHighRB.IsChecked() )
        nNewLevel = 2;
    else if( maMediumRB.IsChecked() )
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
    bool bCertSelected = maTrustCertLB.FirstSelected() != NULL;
    maViewCertPB.Enable( bCertSelected );
    maRemoveCertPB.Enable( bCertSelected && !mbAuthorsReadonly);

    bool bLocationSelected = maTrustFileLocLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    maRemoveLocPB.Enable( bLocationSelected && !mbURLsReadonly);
}


IMPL_LINK_NOARG(MacroSecurityTrustedSourcesTP, ViewCertPBHdl)
{
    if( maTrustCertLB.FirstSelected() )
    {
        sal_uInt16 nSelected = sal_uInt16( sal_uIntPtr( maTrustCertLB.FirstSelected()->GetUserData() ) );

        uno::Reference< dcss::security::XSerialNumberAdapter > xSerialNumberAdapter =
            ::com::sun::star::security::SerialNumberAdapter::create(mpDlg->mxCtx);

        uno::Reference< dcss::security::XCertificate > xCert = mpDlg->mxSecurityEnvironment->getCertificate( maTrustedAuthors[nSelected][0], xSerialNumberAdapter->toSequence( maTrustedAuthors[nSelected][1] ) );

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
    if( maTrustCertLB.FirstSelected() )
    {
        sal_uInt16 nAuthor = sal_uInt16( sal_uIntPtr( maTrustCertLB.FirstSelected()->GetUserData() ) );
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
        rtl::OUString aService( RTL_CONSTASCII_USTRINGPARAM( FOLDER_PICKER_SERVICE_NAME ) );
        uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
        uno::Reference < ui::dialogs::XFolderPicker > xFolderPicker( xFactory->createInstance( aService ), uno::UNO_QUERY );

        short nRet = xFolderPicker->execute();

        if( ui::dialogs::ExecutableDialogResults::OK != nRet )
            return 0;

        rtl::OUString aPathStr = xFolderPicker->getDirectory();
        INetURLObject aNewObj( aPathStr );
        aNewObj.removeFinalSlash();

        // then the new path also an URL else system path
        ::rtl::OUString aSystemFileURL = ( aNewObj.GetProtocol() != INET_PROT_NOT_VALID ) ?
            aPathStr : aNewObj.getFSysPath( INetURLObject::FSYS_DETECT );

        OUString aNewPathStr(aSystemFileURL);

        if ( osl::FileBase::getSystemPathFromFileURL( aSystemFileURL, aSystemFileURL ) == osl::FileBase::E_None )
            aNewPathStr = aSystemFileURL;

        if( maTrustFileLocLB.GetEntryPos( aNewPathStr ) == LISTBOX_ENTRY_NOTFOUND )
        {
            maTrustFileLocLB.InsertEntry( aNewPathStr );
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
    sal_uInt16  nSel = maTrustFileLocLB.GetSelectEntryPos();
    if( nSel != LISTBOX_ENTRY_NOTFOUND )
    {
        maTrustFileLocLB.RemoveEntry( nSel );
        // Trusted Path could not be removed (#i33584#)
        // after remove an entry, select another one if exists
        sal_uInt16 nNewCount = maTrustFileLocLB.GetEntryCount();
        if ( nNewCount > 0 )
        {
            if ( nSel >= nNewCount )
                nSel = nNewCount - 1;
            maTrustFileLocLB.SelectEntryPos( nSel );
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
    maTrustCertLB.Clear();

    sal_uInt32 nEntries = maTrustedAuthors.getLength();

    if ( nEntries && mpDlg->mxSecurityEnvironment.is() )
    {
        for( sal_uInt32 nEntry = 0 ; nEntry < nEntries ; ++nEntry )
        {
            cssu::Sequence< ::rtl::OUString >&              rEntry = maTrustedAuthors[ nEntry ];
            uno::Reference< css::security::XCertificate >   xCert;

            // create from RawData
            xCert = mpDlg->mxSecurityEnvironment->createCertificateFromAscii( rEntry[ 2 ] );

            SvLBoxEntry*    pLBEntry = maTrustCertLB.InsertEntry( XmlSec::GetContentPart( xCert->getSubjectName() ) );
            maTrustCertLB.SetEntryText( XmlSec::GetContentPart( xCert->getIssuerName() ), pLBEntry, 1 );
            maTrustCertLB.SetEntryText( XmlSec::GetDateTimeString( xCert->getNotValidAfter() ), pLBEntry, 2 );
            pLBEntry->SetUserData( ( void* ) (sal_IntPtr)nEntry );      // missuse user data as index
        }
    }
}

MacroSecurityTrustedSourcesTP::MacroSecurityTrustedSourcesTP( Window* _pParent, MacroSecurity* _pDlg )
    :MacroSecurityTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_TRUSTSOURCES ), _pDlg )
    ,maTrustCertFL      ( this, XMLSEC_RES( FL_TRUSTCERT ) )
    ,maTrustCertROFI    ( this, XMLSEC_RES( FI_TRUSTCERT_RO ) )
    ,m_aTrustCertLBContainer(this, XMLSEC_RES(LB_TRUSTCERT))
    ,maTrustCertLB(m_aTrustCertLBContainer)
    ,maAddCertPB        ( this, XMLSEC_RES( PB_ADD_TRUSTCERT ) )
    ,maViewCertPB       ( this, XMLSEC_RES( PB_VIEW_TRUSTCERT ) )
    ,maRemoveCertPB     ( this, XMLSEC_RES( PB_REMOVE_TRUSTCERT ) )
    ,maTrustFileLocFL   ( this, XMLSEC_RES( FL_TRUSTFILELOC ) )
    ,maTrustFileROFI    ( this, XMLSEC_RES( FI_TRUSTFILE_RO ) )
    ,maTrustFileLocFI   ( this, XMLSEC_RES( FI_TRUSTFILELOC ) )
    ,maTrustFileLocLB   ( this, XMLSEC_RES( LB_TRUSTFILELOC ) )
    ,maAddLocPB         ( this, XMLSEC_RES( FL_ADD_TRUSTFILELOC ) )
    ,maRemoveLocPB      ( this, XMLSEC_RES( FL_REMOVE_TRUSTFILELOC ) )
{
    static long nTabs[] = { 3, 0, 35*CS_LB_WIDTH/100, 70*CS_LB_WIDTH/100 };
    maTrustCertLB.SetTabs( &nTabs[ 0 ] );
    maTrustCertLB.InsertHeaderEntry( XMLSEC_RES( STR_HEADERBAR ) );

    FreeResource();

    maTrustCertLB.SetSelectHdl( LINK( this, MacroSecurityTrustedSourcesTP, TrustCertLBSelectHdl ) );
    maAddCertPB.Hide();     // not used in the moment...
    maViewCertPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, ViewCertPBHdl ) );
    maViewCertPB.Disable();
    maRemoveCertPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, RemoveCertPBHdl ) );
    maRemoveCertPB.Disable();

    maTrustFileLocLB.SetSelectHdl( LINK( this, MacroSecurityTrustedSourcesTP, TrustFileLocLBSelectHdl ) );
    maAddLocPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, AddLocPBHdl ) );
    maRemoveLocPB.SetClickHdl( LINK( this, MacroSecurityTrustedSourcesTP, RemoveLocPBHdl ) );
    maRemoveLocPB.Disable();

    maTrustedAuthors = mpDlg->maSecOptions.GetTrustedAuthors();
    mbAuthorsReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::E_MACRO_TRUSTEDAUTHORS );
    maTrustCertROFI.Show( mbAuthorsReadonly );
    mbAuthorsReadonly ? maTrustCertLB.DisableTable() : maTrustCertLB.EnableTable();

    FillCertLB();

    cssu::Sequence< rtl::OUString > aSecureURLs = mpDlg->maSecOptions.GetSecureURLs();
    mbURLsReadonly = mpDlg->maSecOptions.IsReadOnly( SvtSecurityOptions::E_SECUREURLS );
    maTrustFileROFI.Show( mbURLsReadonly );
    maTrustFileLocLB.Enable( !mbURLsReadonly );
    maAddLocPB      .Enable( !mbURLsReadonly );

    sal_Int32 nEntryCnt = aSecureURLs.getLength();
    for( sal_Int32 i = 0 ; i < nEntryCnt ; ++i )
    {
        ::rtl::OUString aSystemFileURL( aSecureURLs[ i ] );
        osl::FileBase::getSystemPathFromFileURL( aSystemFileURL, aSystemFileURL );
        maTrustFileLocLB.InsertEntry( aSystemFileURL );
    }
}

void MacroSecurityTrustedSourcesTP::ActivatePage()
{
    mpDlg->EnableReset( false );
    FillCertLB();
}

void MacroSecurityTrustedSourcesTP::ClosePage( void )
{
    sal_uInt16  nEntryCnt = maTrustFileLocLB.GetEntryCount();
    if( nEntryCnt )
    {
        cssu::Sequence< rtl::OUString > aSecureURLs( nEntryCnt );
        for( sal_uInt16 i = 0 ; i < nEntryCnt ; ++i )
        {
            ::rtl::OUString aURL( maTrustFileLocLB.GetEntry( i ) );
            osl::FileBase::getFileURLFromSystemPath( aURL, aURL );
            aSecureURLs[ i ] = aURL;
        }

        mpDlg->maSecOptions.SetSecureURLs( aSecureURLs );
    }
    // Trusted Path could not be removed (#i33584#)
    // don't forget to remove the old saved SecureURLs
    else
        mpDlg->maSecOptions.SetSecureURLs( cssu::Sequence< rtl::OUString >() );

    mpDlg->maSecOptions.SetTrustedAuthors( maTrustedAuthors );
}

ReadOnlyImage::ReadOnlyImage(Window* pParent, const ResId rResId) :
            FixedImage(pParent, rResId)
{
    SetImage( Image(XMLSEC_RES( RID_XMLSECTP_LOCK )));
}

ReadOnlyImage::~ReadOnlyImage()
{
}

void ReadOnlyImage::RequestHelp( const HelpEvent& rHEvt )
{
    if( Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled() )
    {
        Rectangle   aLogicPix( LogicToPixel( Rectangle( Point(), GetOutputSize() ) ) );
        Rectangle   aScreenRect( OutputToScreenPixel( aLogicPix.TopLeft() ),
                                     OutputToScreenPixel( aLogicPix.BottomRight() ) );

        OUString aStr(ReadOnlyImage::GetHelpTip());
        if ( Help::IsBalloonHelpEnabled() )
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aScreenRect,
            aStr );
        else if ( Help::IsQuickHelpEnabled() )
            Help::ShowQuickHelp( this, aScreenRect, aStr );
    }
    else
        Window::RequestHelp( rHEvt );
}

const OUString& ReadOnlyImage::GetHelpTip()
{
     static OUString aStr(XMLSEC_RES( RID_XMLSECTP_READONLY_CONFIG_TIP));
     return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
