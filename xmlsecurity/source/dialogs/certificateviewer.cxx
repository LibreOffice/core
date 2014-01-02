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
#include "svtools/treelistentry.hxx"

#include "dialogs.hrc"
#include "resourcemanager.hxx"

#include <vcl/settings.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{
    void ShrinkToFit( FixedImage& _rImg )
    {
        _rImg.SetSizePixel( _rImg.GetImage().GetSizePixel() );
    }
}

CertificateViewer::CertificateViewer(
        Window* _pParent,
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

    mpTabCtrl->SetTabPage(mnGeneralId, new CertificateViewerGeneralTP( mpTabCtrl, this));
    mpTabCtrl->SetTabPage(mnDetailsId, new CertificateViewerDetailsTP( mpTabCtrl, this));
    mpTabCtrl->SetTabPage(mnPathId, new CertificateViewerCertPathTP( mpTabCtrl, this));
    mpTabCtrl->SetCurPageId(mnGeneralId);
}

CertificateViewer::~CertificateViewer()
{
    delete mpTabCtrl->GetTabPage(mnGeneralId);
    delete mpTabCtrl->GetTabPage(mnDetailsId);
    delete mpTabCtrl->GetTabPage(mnPathId);
}

CertificateViewerTP::CertificateViewerTP( Window* _pParent, const ResId& _rResId, CertificateViewer* _pDlg )
    :TabPage        ( _pParent, _rResId )
    ,mpDlg          ( _pDlg )
{
}

CertificateViewerTP::CertificateViewerTP( Window* _pParent, const OString& rID,
    const OUString& rUIXMLDescription, CertificateViewer* _pDlg )
    : TabPage(_pParent, rID, rUIXMLDescription)
    , mpDlg(_pDlg)
{
}

CertificateViewerGeneralTP::CertificateViewerGeneralTP( Window* _pParent, CertificateViewer* _pDlg )
    :CertificateViewerTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_GENERAL ), _pDlg )
    ,maFrameWin             ( this, XMLSEC_RES( WIN_FRAME ) )
    ,maCertImg              ( this, XMLSEC_RES( IMG_CERT ) )
    ,maCertInfoFI           ( this, XMLSEC_RES( FI_CERTINFO ) )
    ,maSep1FL               ( this, XMLSEC_RES( FL_SEP1 ) )
    ,maHintNotTrustedFI     ( this, XMLSEC_RES( FI_HINTNOTTRUST ) )
    ,maSep2FL               ( this, XMLSEC_RES( FL_SEP2 ) )
    ,maIssuedToLabelFI      ( this, XMLSEC_RES( FI_ISSTOLABEL ) )
    ,maIssuedToFI           ( this, XMLSEC_RES( FI_ISSTO ) )
    ,maIssuedByLabelFI      ( this, XMLSEC_RES( FI_ISSBYLABEL ) )
    ,maIssuedByFI           ( this, XMLSEC_RES( FI_ISSBY ) )
    ,maValidDateFI          ( this, XMLSEC_RES( FI_VALIDDATE ) )
    ,maKeyImg               ( this, XMLSEC_RES( IMG_KEY ) )
    ,maHintCorrespPrivKeyFI ( this, XMLSEC_RES( FI_CORRPRIVKEY ) )
{
    //Verify the certificate
    sal_Int32 certStatus = mpDlg->mxSecurityEnvironment->verifyCertificate(mpDlg->mxCert,
         Sequence<Reference<css::security::XCertificate> >());

    bool bCertValid = certStatus == css::security::CertificateValidity::VALID ?  true : false;

    if ( !bCertValid )
    {
        maCertImg.SetImage(
            Image( XMLSEC_RES( IMG_STATE_NOT_VALIDATED ) ) );
        maHintNotTrustedFI.SetText( XMLSEC_RES( STR_CERTIFICATE_NOT_VALIDATED ) );
    }

    FreeResource();

    Wallpaper aBack( GetSettings().GetStyleSettings().GetWindowColor() );
    maFrameWin.SetBackground( aBack );
    maCertImg.SetBackground( aBack );
    maCertInfoFI.SetBackground( aBack );
    maSep1FL.SetBackground( aBack );
    maHintNotTrustedFI.SetBackground( aBack );
    maSep2FL.SetBackground( aBack );
    maIssuedToLabelFI.SetBackground( aBack );
    maIssuedToFI.SetBackground( aBack );
    maIssuedByLabelFI.SetBackground( aBack );
    maIssuedByFI.SetBackground( aBack );
    maValidDateFI.SetBackground( aBack );
    maKeyImg.SetBackground( aBack );
    maHintCorrespPrivKeyFI.SetBackground( aBack );

    // make some bold
    Font    aFnt( maCertInfoFI.GetFont() );
    aFnt.SetWeight( WEIGHT_BOLD );
    maCertInfoFI.SetFont( aFnt );
    maHintNotTrustedFI.SetFont( aFnt );
    maIssuedToLabelFI.SetFont( aFnt );
    maIssuedByLabelFI.SetFont( aFnt );
    maValidDateFI.SetFont( aFnt );

    // insert data
    css::uno::Reference< css::security::XCertificate > xCert = mpDlg->mxCert;

    maIssuedToFI.SetText( XmlSec::GetContentPart( xCert->getSubjectName() ) );
    maIssuedByFI.SetText( XmlSec::GetContentPart( xCert->getIssuerName() ) );

    // dynamic length because of the different languages
    long nWidth1 = maIssuedToLabelFI.GetTextWidth( maIssuedToLabelFI.GetText() );
    long nWidth2 = maIssuedByLabelFI.GetTextWidth( maIssuedByLabelFI.GetText() );
    long nNewWidth = std::max( nWidth1, nWidth2 ) + 5;
    Size aNewSize = maIssuedToLabelFI.GetSizePixel();
    aNewSize.Width() = nNewWidth;
    maIssuedToLabelFI.SetSizePixel( aNewSize );
    maIssuedByLabelFI.SetSizePixel( aNewSize );
    long nNewX = maIssuedToLabelFI.GetPosPixel().X() + nNewWidth + 1;
    Point aNewPos = maIssuedToFI.GetPosPixel();
    aNewPos.X() = nNewX;
    maIssuedToFI.SetPosPixel( aNewPos );
    aNewPos = maIssuedByFI.GetPosPixel();
    aNewPos.X() = nNewX;
    maIssuedByFI.SetPosPixel( aNewPos );
    nNewWidth = maValidDateFI.GetSizePixel().Width() - nNewX;
    aNewSize = maIssuedToFI.GetSizePixel();
    aNewSize.Width() = nNewWidth;
    maIssuedToFI.SetSizePixel( aNewSize );
    maIssuedByFI.SetSizePixel( aNewSize );

    DateTime aDateTimeStart( DateTime::EMPTY );
    DateTime aDateTimeEnd( DateTime::EMPTY );
    utl::typeConvert( xCert->getNotValidBefore(), aDateTimeStart );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTimeEnd );
    OUString sText = maValidDateFI.GetText();
    sText = sText.replaceFirst( "%SDATE%",
        GetSettings().GetUILocaleDataWrapper().getDate( aDateTimeStart.GetDate() ) );
    sText = sText.replaceFirst( "%EDATE%",
        GetSettings().GetUILocaleDataWrapper().getDate( aDateTimeEnd.GetDate() ) );
    maValidDateFI.SetText( sText );

    // adjust position of fixed text depending on image sizes
    ShrinkToFit( maCertImg );
    ShrinkToFit( maKeyImg );
    XmlSec::AlignAfterImage( maCertImg, maCertInfoFI, 12 );
    XmlSec::AlignAfterImage( maKeyImg, maHintCorrespPrivKeyFI, 12 );

    // Check if we have the private key...
    sal_Bool bHasPrivateKey = sal_False;
    // #i41270# Check only if we have that certificate in our security environment
    if ( _pDlg->mbCheckForPrivateKey )
    {
        long nCertificateCharacters = _pDlg->mxSecurityEnvironment->getCertificateCharacters( xCert );
        bHasPrivateKey = ( nCertificateCharacters & security::CertificateCharacters::HAS_PRIVATE_KEY ) ? sal_True : sal_False;
    }
    if ( !bHasPrivateKey )
    {
        maKeyImg.Hide();
        maHintCorrespPrivKeyFI.Hide();
    }
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


void CertificateViewerDetailsTP::Clear( void )
{
    maElementML.SetText( OUString() );
    sal_uLong           i = 0;
    SvTreeListEntry*    pEntry = maElementsLB.GetEntry( i );
    while( pEntry )
    {
        delete ( Details_UserDatat* ) pEntry->GetUserData();
        ++i;
        pEntry = maElementsLB.GetEntry( i );
    }

    maElementsLB.Clear();
}

void CertificateViewerDetailsTP::InsertElement( const OUString& _rField, const OUString& _rValue,
                                                const OUString& _rDetails, bool _bFixedWidthFont )
{
    SvTreeListEntry*    pEntry = maElementsLB.InsertEntry( _rField );
    maElementsLB.SetEntryText( _rValue, pEntry, 1 );
    pEntry->SetUserData( ( void* ) new Details_UserDatat( _rDetails, _bFixedWidthFont ) );
}

CertificateViewerDetailsTP::CertificateViewerDetailsTP( Window* _pParent, CertificateViewer* _pDlg )
    :CertificateViewerTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_DETAILS ), _pDlg  )
    ,m_aElementsLBContainer(this, XMLSEC_RES(LB_ELEMENTS))
    ,maElementsLB(m_aElementsLBContainer)
    ,maElementML            ( this, XMLSEC_RES( ML_ELEMENT ) )
    ,maStdFont              ( maElementML.GetControlFont() )
    ,maFixedWidthFont       ( OutputDevice::GetDefaultFont( DEFAULTFONT_UI_FIXED, LANGUAGE_DONTKNOW, DEFAULTFONT_FLAGS_ONLYONE, this ) )
{
    WinBits nStyle = maElementsLB.GetStyle();
    nStyle &= ~WB_HSCROLL;
    maElementsLB.SetStyle( nStyle );

    maFixedWidthFont.SetHeight( maStdFont.GetHeight() );

    static long nTabs[] = { 2, 0, 30*CS_LB_WIDTH/100 };
    maElementsLB.SetTabs( &nTabs[ 0 ] );
    maElementsLB.InsertHeaderEntry( XMLSEC_RES( STR_HEADERBAR ) );

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

    FreeResource();

    maElementsLB.SetSelectHdl( LINK( this, CertificateViewerDetailsTP, ElementSelectHdl ) );
}

CertificateViewerDetailsTP::~CertificateViewerDetailsTP()
{
    Clear();
}

void CertificateViewerDetailsTP::ActivatePage()
{
}

IMPL_LINK_NOARG(CertificateViewerDetailsTP, ElementSelectHdl)
{
    SvTreeListEntry*    pEntry = maElementsLB.FirstSelected();
    OUString        aElementText;
    bool            bFixedWidthFont;
    if( pEntry )
    {
        const Details_UserDatat*    p = ( Details_UserDatat* ) pEntry->GetUserData();
        aElementText = p->maTxt;
        bFixedWidthFont = p->mbFixedWidthFont;
    }
    else
        bFixedWidthFont = false;

    maElementML.SetFont( bFixedWidthFont? maFixedWidthFont : maStdFont );
    maElementML.SetControlFont( bFixedWidthFont? maFixedWidthFont : maStdFont );
    maElementML.SetText( aElementText );

    return 0;
}

struct CertPath_UserData
{
    css::uno::Reference< css::security::XCertificate > mxCert;
    OUString                                        maStatus;
    bool mbValid;

    CertPath_UserData( css::uno::Reference< css::security::XCertificate > xCert, bool bValid):
        mxCert(xCert),
        mbValid(bValid)
    {
    }
};


CertificateViewerCertPathTP::CertificateViewerCertPathTP( Window* _pParent, CertificateViewer* _pDlg )
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
    Clear();
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
        SvTreeListEntry* pParent = NULL;
        for( i = nCnt; i; )
        {
            const Reference< security::XCertificate > rCert = pCertPath[ --i ];
            OUString sName = XmlSec::GetContentPart( rCert->getSubjectName() );
            //Verify the certificate
            sal_Int32 certStatus = mpDlg->mxSecurityEnvironment->verifyCertificate(rCert,
                 Sequence<Reference<css::security::XCertificate> >());
            bool bCertValid = certStatus == css::security::CertificateValidity::VALID ? true : false;
            pParent = InsertCert( pParent, sName, rCert, bCertValid);
        }

        mpCertPathLB->Select( pParent );
        mpViewCertPB->Disable(); // Own certificate selected

        while( pParent )
        {
            mpCertPathLB->Expand( pParent );
            pParent = mpCertPathLB->GetParent( pParent );
        }

        CertSelectHdl( NULL );
    }
}

IMPL_LINK_NOARG(CertificateViewerCertPathTP, ViewCertHdl)
{
    SvTreeListEntry* pEntry = mpCertPathLB->FirstSelected();
    if( pEntry )
    {
        CertificateViewer aViewer( this, mpDlg->mxSecurityEnvironment, ((CertPath_UserData*)pEntry->GetUserData())->mxCert, false );
        aViewer.Execute();
    }

    return 0;
}

IMPL_LINK_NOARG(CertificateViewerCertPathTP, CertSelectHdl)
{
    OUString sStatus;
    SvTreeListEntry* pEntry = mpCertPathLB->FirstSelected();
    if( pEntry )
    {
        CertPath_UserData* pData = (CertPath_UserData*) pEntry->GetUserData();
        if ( pData )
            sStatus = pData->mbValid ? msCertOK : msCertNotValidated;
    }

    mpCertStatusML->SetText( sStatus );
    mpViewCertPB->Enable( pEntry && ( pEntry != mpCertPathLB->Last() ) );
    return 0;
}

void CertificateViewerCertPathTP::Clear( void )
{
    mpCertStatusML->SetText( OUString() );
    sal_uLong           i = 0;
    SvTreeListEntry*    pEntry = mpCertPathLB->GetEntry( i );
    while( pEntry )
    {
        delete ( CertPath_UserData* ) pEntry->GetUserData();
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
    pEntry->SetUserData( ( void* ) new CertPath_UserData( rxCert, bValid ) );

    return pEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
