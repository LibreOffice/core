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

#include <xmlsecurity/certificateviewer.hxx>
#include <com/sun/star/security/XCertificate.hpp>

#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <unotools/localedatawrapper.hxx>
#include <unotools/datetime.hxx>
#include <svtools/treelistentry.hxx>

#include "dialogs.hrc"
#include "resourcemanager.hxx"

#include <vcl/settings.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

CertificateViewer::CertificateViewer(
        vcl::Window* _pParent,
        const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment,
        const css::uno::Reference< css::security::XCertificate >& _rXCert, bool bCheckForPrivateKey )
    : TabDialog(_pParent, "ViewCertDialog", "xmlsec/ui/viewcertdialog.ui" )
{
    get(mpTabCtrl, "tabcontrol");

    mbCheckForPrivateKey = bCheckForPrivateKey;

    mxSecurityEnvironment = _rxSecurityEnvironment;
    mxCert = _rXCert;

    mnGeneralId = mpTabCtrl->GetPageId("general");
    mnDetailsId = mpTabCtrl->GetPageId("details");
    mnPathId = mpTabCtrl->GetPageId("path");

    mpTabCtrl->SetTabPage(mnGeneralId, VclPtr<CertificateViewerGeneralTP>::Create( mpTabCtrl, this));
    mpTabCtrl->SetTabPage(mnDetailsId, VclPtr<CertificateViewerDetailsTP>::Create( mpTabCtrl, this));
    mpTabCtrl->SetTabPage(mnPathId, VclPtr<CertificateViewerCertPathTP>::Create( mpTabCtrl, this));
    mpTabCtrl->SetCurPageId(mnGeneralId);
}

CertificateViewer::~CertificateViewer()
{
    disposeOnce();
}

void CertificateViewer::dispose()
{
    mpTabCtrl->GetTabPage(mnGeneralId)->disposeOnce();
    mpTabCtrl->GetTabPage(mnDetailsId)->disposeOnce();
    mpTabCtrl->GetTabPage(mnPathId)->disposeOnce();
    mpTabCtrl.clear();
    TabDialog::dispose();
}

CertificateViewerTP::CertificateViewerTP( vcl::Window* _pParent, const OString& rID,
    const OUString& rUIXMLDescription, CertificateViewer* _pDlg )
    : TabPage(_pParent, rID, rUIXMLDescription)
    , mpDlg(_pDlg)
{
}

CertificateViewerTP::~CertificateViewerTP()
{
    disposeOnce();
}

void CertificateViewerTP::dispose()
{
    mpDlg.clear();
    TabPage::dispose();
}


CertificateViewerGeneralTP::CertificateViewerGeneralTP( vcl::Window* _pParent, CertificateViewer* _pDlg )
    :CertificateViewerTP    ( _pParent, "CertGeneral", "xmlsec/ui/certgeneral.ui", _pDlg )
{
    get( m_pCertImg, "certimage" );
    get( m_pHintNotTrustedFI, "hintnotrust" );
    get( m_pIssuedToFI, "issued_to_value" );
    get( m_pIssuedByFI, "issued_by_value" );
    get( m_pValidFromDateFI, "valid_from_value" );
    get( m_pValidToDateFI, "valid_to_value" );
    get( m_pKeyImg, "keyimage" );
    get( m_pHintCorrespPrivKeyFI, "privatekey" );

    //Verify the certificate
    sal_Int32 certStatus = mpDlg->mxSecurityEnvironment->verifyCertificate(mpDlg->mxCert,
         Sequence<Reference<css::security::XCertificate> >());

    bool bCertValid = certStatus == css::security::CertificateValidity::VALID;

    if ( !bCertValid )
    {
        m_pCertImg->SetImage(
            Image( XMLSEC_RES( IMG_STATE_NOT_VALIDATED ) ) );
        m_pHintNotTrustedFI->SetText( XMLSEC_RES( STR_CERTIFICATE_NOT_VALIDATED ) );
    }

    // insert data
    css::uno::Reference< css::security::XCertificate > xCert = mpDlg->mxCert;

    m_pIssuedToFI->SetText( XmlSec::GetContentPart( xCert->getSubjectName() ) );
    m_pIssuedByFI->SetText( XmlSec::GetContentPart( xCert->getIssuerName() ) );

    DateTime aDateTimeStart( DateTime::EMPTY );
    DateTime aDateTimeEnd( DateTime::EMPTY );
    utl::typeConvert( xCert->getNotValidBefore(), aDateTimeStart );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTimeEnd );

    OUString sValidFromDate = GetSettings().GetUILocaleDataWrapper().getDate( aDateTimeStart.GetDate() );
    OUString sValidToDate = GetSettings().GetUILocaleDataWrapper().getDate( aDateTimeEnd.GetDate() );

    m_pValidFromDateFI->SetText(sValidFromDate);
    m_pValidToDateFI->SetText(sValidToDate);

    // Check if we have the private key...
    bool bHasPrivateKey = false;
    // #i41270# Check only if we have that certificate in our security environment
    if ( _pDlg->mbCheckForPrivateKey )
    {
        long nCertificateCharacters = _pDlg->mxSecurityEnvironment->getCertificateCharacters( xCert );
        bHasPrivateKey = ( nCertificateCharacters & security::CertificateCharacters::HAS_PRIVATE_KEY );
    }
    if ( !bHasPrivateKey )
    {
        m_pKeyImg->Hide();
        m_pHintCorrespPrivKeyFI->Hide();
    }
}

CertificateViewerGeneralTP::~CertificateViewerGeneralTP()
{
    disposeOnce();
}

void CertificateViewerGeneralTP::dispose()
{
    m_pCertImg.clear();
    m_pHintNotTrustedFI.clear();
    m_pIssuedToFI.clear();
    m_pIssuedByFI.clear();
    m_pValidFromDateFI.clear();
    m_pValidToDateFI.clear();
    m_pKeyImg.clear();
    m_pHintCorrespPrivKeyFI.clear();
    CertificateViewerTP::dispose();
}

void CertificateViewerGeneralTP::ActivatePage()
{

}


struct Details_UserDatat
{
    OUString        maTxt;
    bool            mbFixedWidthFont;

    inline          Details_UserDatat( const OUString& _rTxt, bool _bFixedWidthFont );
};

inline Details_UserDatat::Details_UserDatat( const OUString& _rTxt, bool _bFixedWidthFont )
    :maTxt              ( _rTxt )
    ,mbFixedWidthFont   ( _bFixedWidthFont )
{
}


void CertificateViewerDetailsTP::Clear()
{
    m_pValueDetails->SetText( OUString() );
    sal_uLong           i = 0;
    SvTreeListEntry*    pEntry = m_pElementsLB->GetEntry( i );
    while( pEntry )
    {
        delete static_cast<Details_UserDatat*>(pEntry->GetUserData());
        ++i;
        pEntry = m_pElementsLB->GetEntry( i );
    }

    m_pElementsLB->Clear();
}

void CertificateViewerDetailsTP::InsertElement( const OUString& _rField, const OUString& _rValue,
                                                const OUString& _rDetails, bool _bFixedWidthFont )
{
    SvTreeListEntry*    pEntry = m_pElementsLB->InsertEntry( _rField );
    m_pElementsLB->SetEntryText( _rValue, pEntry, 1 );
    pEntry->SetUserData( static_cast<void*>(new Details_UserDatat( _rDetails, _bFixedWidthFont )) );
}

CertificateViewerDetailsTP::CertificateViewerDetailsTP( vcl::Window* _pParent, CertificateViewer* _pDlg )
    :CertificateViewerTP    ( _pParent, "CertDetails", "xmlsec/ui/certdetails.ui", _pDlg  )
    ,m_aFixedWidthFont( OutputDevice::GetDefaultFont( DefaultFontType::UI_FIXED, LANGUAGE_DONTKNOW, GetDefaultFontFlags::OnlyOne, this ) )
{
    get( m_pValueDetails, "valuedetails" );
    get( m_pElementsLBContainer, "tablecontainer" );
    m_pElementsLB = VclPtr<SvSimpleTable>::Create( *m_pElementsLBContainer );

    m_aStdFont = m_pValueDetails->GetControlFont();
    WinBits nStyle = m_pElementsLB->GetStyle();
    nStyle &= ~WB_HSCROLL;
    m_pElementsLB->SetStyle( nStyle );

    m_aFixedWidthFont.SetHeight( m_aStdFont.GetHeight() );

    static long nTabs[] = { 2, 0, 30*CS_LB_WIDTH/100 };
    m_pElementsLB->SetTabs( &nTabs[ 0 ] );
    m_pElementsLB->InsertHeaderEntry( XMLSEC_RES( STR_HEADERBAR ) );

    // fill list box
    Reference< security::XCertificate > xCert = mpDlg->mxCert;
    sal_uInt16                  nLineBreak = 16;
    const char*             pHexSep = " ";
    OUString                aLBEntry;
    OUString                aDetails;
    // Certificate Versions are reported wrong (#i35107#) - 0 == "V1", 1 == "V2", ..., n = "V(n+1)"
    aLBEntry = "V" + OUString::number( xCert->getVersion() + 1 );
    InsertElement( XMLSEC_RES( STR_VERSION ), aLBEntry, aLBEntry );
    Sequence< sal_Int8 >    aSeq = xCert->getSerialNumber();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XMLSEC_RES( STR_SERIALNUM ), aLBEntry, aDetails, true );

    std::pair< OUString, OUString> pairIssuer =
        XmlSec::GetDNForCertDetailsView(xCert->getIssuerName());
    aLBEntry = pairIssuer.first;
    aDetails = pairIssuer.second;
    InsertElement( XMLSEC_RES( STR_ISSUER ), aLBEntry, aDetails );

    DateTime aDateTime( DateTime::EMPTY );
    utl::typeConvert( xCert->getNotValidBefore(), aDateTime );
    aLBEntry = GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() );
    aLBEntry += " ";
    aLBEntry += GetSettings().GetUILocaleDataWrapper().getTime( aDateTime.GetTime() );
    InsertElement( XMLSEC_RES( STR_VALIDFROM ), aLBEntry, aLBEntry  );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTime );
    aLBEntry = GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() );
    aLBEntry += " ";
    aLBEntry += GetSettings().GetUILocaleDataWrapper().getTime( aDateTime.GetTime() );
    InsertElement( XMLSEC_RES( STR_VALIDTO ), aLBEntry, aLBEntry );

    std::pair< OUString, OUString > pairSubject =
        XmlSec::GetDNForCertDetailsView(xCert->getSubjectName());
    aLBEntry = pairSubject.first;
    aDetails = pairSubject.second;
    InsertElement( XMLSEC_RES( STR_SUBJECT ), aLBEntry, aDetails );

    aLBEntry = aDetails = xCert->getSubjectPublicKeyAlgorithm();
    InsertElement( XMLSEC_RES( STR_SUBJECT_PUBKEY_ALGO ), aLBEntry, aDetails );
    aSeq = xCert->getSubjectPublicKeyValue();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XMLSEC_RES( STR_SUBJECT_PUBKEY_VAL ), aLBEntry, aDetails, true );

    aLBEntry = aDetails = xCert->getSignatureAlgorithm();
    InsertElement( XMLSEC_RES( STR_SIGNATURE_ALGO ), aLBEntry, aDetails );

    aSeq = xCert->getSHA1Thumbprint();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XMLSEC_RES( STR_THUMBPRINT_SHA1 ), aLBEntry, aDetails, true );

    aSeq = xCert->getMD5Thumbprint();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XMLSEC_RES( STR_THUMBPRINT_MD5 ), aLBEntry, aDetails, true );

    m_pElementsLB->SetSelectHdl( LINK( this, CertificateViewerDetailsTP, ElementSelectHdl ) );
}

CertificateViewerDetailsTP::~CertificateViewerDetailsTP()
{
    disposeOnce();
}

void CertificateViewerDetailsTP::dispose()
{
    Clear();
    m_pElementsLB.disposeAndClear();
    m_pElementsLBContainer.clear();
    m_pValueDetails.clear();
    CertificateViewerTP::dispose();
}

void CertificateViewerDetailsTP::ActivatePage()
{
}

IMPL_LINK_NOARG_TYPED(CertificateViewerDetailsTP, ElementSelectHdl, SvTreeListBox*, void)
{
    SvTreeListEntry*    pEntry = m_pElementsLB->FirstSelected();
    OUString        aElementText;
    bool            bFixedWidthFont;
    if( pEntry )
    {
        const Details_UserDatat*    p = static_cast<Details_UserDatat*>(pEntry->GetUserData());
        aElementText = p->maTxt;
        bFixedWidthFont = p->mbFixedWidthFont;
    }
    else
        bFixedWidthFont = false;

    m_pValueDetails->SetFont( bFixedWidthFont? m_aFixedWidthFont : m_aStdFont );
    m_pValueDetails->SetControlFont( bFixedWidthFont? m_aFixedWidthFont : m_aStdFont );
    m_pValueDetails->SetText( aElementText );
}

struct CertPath_UserData
{
    css::uno::Reference< css::security::XCertificate > mxCert;
    bool mbValid;

    CertPath_UserData( css::uno::Reference< css::security::XCertificate > xCert, bool bValid):
        mxCert(xCert),
        mbValid(bValid)
    {
    }
};


CertificateViewerCertPathTP::CertificateViewerCertPathTP( vcl::Window* _pParent, CertificateViewer* _pDlg )
    : CertificateViewerTP(_pParent, "CertPage", "xmlsec/ui/certpage.ui", _pDlg)
    , mpParent(_pDlg)
    , mbFirstActivateDone(false)
{
    get(mpCertPathLB, "signatures");
    get(mpViewCertPB, "viewcert");
    get(mpCertStatusML, "status");

    msCertOK = get<FixedText>("certok")->GetText();
    msCertNotValidated = get<FixedText>("certnotok")->GetText();
    maCertImage = get<FixedImage>("imgok")->GetImage();
    maCertNotValidatedImage = get<FixedImage>("imgnotok")->GetImage();

    Size aControlSize(LogicToPixel(Size(251, 45), MAP_APPFONT));
    mpCertPathLB->set_width_request(aControlSize.Width());
    mpCertPathLB->set_height_request(aControlSize.Height());
    mpCertStatusML->set_width_request(aControlSize.Width());
    mpCertStatusML->set_height_request(aControlSize.Height());

    mpCertPathLB->SetNodeDefaultImages();
    mpCertPathLB->SetSublistOpenWithLeftRight();
    mpCertPathLB->SetSelectHdl( LINK( this, CertificateViewerCertPathTP, CertSelectHdl ) );
    mpViewCertPB->SetClickHdl( LINK( this, CertificateViewerCertPathTP, ViewCertHdl ) );
}

CertificateViewerCertPathTP::~CertificateViewerCertPathTP()
{
    disposeOnce();
}

void CertificateViewerCertPathTP::dispose()
{
    Clear();
    mpCertPathLB.clear();
    mpViewCertPB.clear();
    mpCertStatusML.clear();
    mpParent.clear();
    CertificateViewerTP::dispose();
}

void CertificateViewerCertPathTP::ActivatePage()
{
    if ( !mbFirstActivateDone )
    {
        mbFirstActivateDone = true;
        Sequence< Reference< security::XCertificate > > aCertPath =
            mpParent->mxSecurityEnvironment->buildCertificatePath( mpParent->mxCert );
        const Reference< security::XCertificate >* pCertPath = aCertPath.getConstArray();

        sal_Int32 i, nCnt = aCertPath.getLength();
        SvTreeListEntry* pParent = nullptr;
        for( i = nCnt; i; )
        {
            const Reference< security::XCertificate > rCert = pCertPath[ --i ];
            OUString sName = XmlSec::GetContentPart( rCert->getSubjectName() );
            //Verify the certificate
            sal_Int32 certStatus = mpDlg->mxSecurityEnvironment->verifyCertificate(rCert,
                 Sequence<Reference<css::security::XCertificate> >());
            bool bCertValid = certStatus == css::security::CertificateValidity::VALID;
            pParent = InsertCert( pParent, sName, rCert, bCertValid);
        }

        mpCertPathLB->Select( pParent );
        mpViewCertPB->Disable(); // Own certificate selected

        while( pParent )
        {
            mpCertPathLB->Expand( pParent );
            pParent = mpCertPathLB->GetParent( pParent );
        }

        CertSelectHdl( nullptr );
    }
}

IMPL_LINK_NOARG_TYPED(CertificateViewerCertPathTP, ViewCertHdl, Button*, void)
{
    SvTreeListEntry* pEntry = mpCertPathLB->FirstSelected();
    if( pEntry )
    {
        ScopedVclPtrInstance< CertificateViewer > aViewer(
                this, mpDlg->mxSecurityEnvironment,
                static_cast<CertPath_UserData*>(pEntry->GetUserData())->mxCert,
                false );
        aViewer->Execute();
    }
}

IMPL_LINK_NOARG_TYPED(CertificateViewerCertPathTP, CertSelectHdl, SvTreeListBox*, void)
{
    OUString sStatus;
    SvTreeListEntry* pEntry = mpCertPathLB->FirstSelected();
    if( pEntry )
    {
        CertPath_UserData* pData = static_cast<CertPath_UserData*>(pEntry->GetUserData());
        if ( pData )
            sStatus = pData->mbValid ? msCertOK : msCertNotValidated;
    }

    mpCertStatusML->SetText( sStatus );
    mpViewCertPB->Enable( pEntry && ( pEntry != mpCertPathLB->Last() ) );
}

void CertificateViewerCertPathTP::Clear()
{
    mpCertStatusML->SetText( OUString() );
    sal_uLong           i = 0;
    SvTreeListEntry*    pEntry = mpCertPathLB->GetEntry( i );
    while( pEntry )
    {
        delete static_cast<CertPath_UserData*>(pEntry->GetUserData());
        ++i;
        pEntry = mpCertPathLB->GetEntry( i );
    }

    mpCertPathLB->Clear();
}

SvTreeListEntry* CertificateViewerCertPathTP::InsertCert(
    SvTreeListEntry* _pParent, const OUString& _rName, css::uno::Reference< css::security::XCertificate > rxCert,
    bool bValid)
{
    Image aImage = bValid ? maCertImage : maCertNotValidatedImage;
    SvTreeListEntry* pEntry = mpCertPathLB->InsertEntry( _rName, aImage, aImage, _pParent );
    pEntry->SetUserData( static_cast<void*>(new CertPath_UserData( rxCert, bValid )) );

    return pEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
