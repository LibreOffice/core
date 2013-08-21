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

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

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
    :TabDialog      ( _pParent, XMLSEC_RES( RID_XMLSECDLG_CERTVIEWER ) )
    ,maTabCtrl      ( this, XMLSEC_RES( 1 ) )
    ,maOkBtn        ( this, XMLSEC_RES( BTN_OK ) )
    ,maHelpBtn      ( this, XMLSEC_RES( BTN_HELP ) )
{
    FreeResource();

    mbCheckForPrivateKey = bCheckForPrivateKey;

    mxSecurityEnvironment = _rxSecurityEnvironment;
    mxCert = _rXCert;

    maTabCtrl.SetTabPage( RID_XMLSECTP_GENERAL, new CertificateViewerGeneralTP( &maTabCtrl, this ) );
    maTabCtrl.SetTabPage( RID_XMLSECTP_DETAILS, new CertificateViewerDetailsTP( &maTabCtrl, this ) );
    maTabCtrl.SetTabPage( RID_XMLSECTP_CERTPATH, new CertificateViewerCertPathTP( &maTabCtrl, this ) );
    maTabCtrl.SetCurPageId( RID_XMLSECTP_GENERAL );
}

CertificateViewer::~CertificateViewer()
{
    delete maTabCtrl.GetTabPage( RID_XMLSECTP_CERTPATH );
    delete maTabCtrl.GetTabPage( RID_XMLSECTP_DETAILS );
    delete maTabCtrl.GetTabPage( RID_XMLSECTP_GENERAL );
}

CertificateViewerTP::CertificateViewerTP( Window* _pParent, const ResId& _rResId, CertificateViewer* _pDlg )
    :TabPage        ( _pParent, _rResId )
    ,mpDlg          ( _pDlg )
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
    aLBEntry = OUString( "V" );
    aLBEntry += OUString::number( xCert->getVersion() + 1 );
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
    aLBEntry += OUString( " " );
    aLBEntry += GetSettings().GetUILocaleDataWrapper().getTime( aDateTime.GetTime() );
    InsertElement( XMLSEC_RES( STR_VALIDFROM ), aLBEntry, aLBEntry  );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTime );
    aLBEntry = GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() );
    aLBEntry += OUString( " " );
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
    :CertificateViewerTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_CERTPATH ), _pDlg  )
    ,maCertPathFT           ( this, XMLSEC_RES( FT_CERTPATH ) )
    ,maCertPathLB           ( this, XMLSEC_RES( LB_SIGNATURES ) )
    ,maViewCertPB           ( this, XMLSEC_RES( BTN_VIEWCERT ) )
    ,maCertStatusFT         ( this, XMLSEC_RES( FT_CERTSTATUS ) )
    ,maCertStatusML         ( this, XMLSEC_RES( ML_CERTSTATUS ) )
    ,mpParent               ( _pDlg )
    ,mbFirstActivateDone    ( false )
    ,maCertImage            ( XMLSEC_RES( IMG_CERT_SMALL ) )
    ,maCertNotValidatedImage( XMLSEC_RES( IMG_CERT_NOTVALIDATED_SMALL ) )
    ,msCertOK               ( XMLSEC_RES( STR_PATH_CERT_OK ) )
    ,msCertNotValidated     ( XMLSEC_RES( STR_PATH_CERT_NOT_VALIDATED ) )

{
    FreeResource();

    maCertPathLB.SetNodeDefaultImages();
    maCertPathLB.SetSublistOpenWithLeftRight();
    maCertPathLB.SetSelectHdl( LINK( this, CertificateViewerCertPathTP, CertSelectHdl ) );
    maViewCertPB.SetClickHdl( LINK( this, CertificateViewerCertPathTP, ViewCertHdl ) );

    // check if buttontext is to wide
    const long nOffset = 10;
    OUString sText = maViewCertPB.GetText();
    long nTxtW = maViewCertPB.GetTextWidth( sText );
    if ( sText.indexOf( '~' ) == -1 )
        nTxtW += nOffset;
    long nBtnW = maViewCertPB.GetSizePixel().Width();
    if ( nTxtW > nBtnW )
    {
        // broaden the button
        long nDelta = nTxtW - nBtnW;
        Size aNewSize = maViewCertPB.GetSizePixel();
        aNewSize.Width() += nDelta;
        maViewCertPB.SetSizePixel( aNewSize );
        // and give it a new position
        Point aNewPos = maViewCertPB.GetPosPixel();
        aNewPos.X() -= nDelta;
        maViewCertPB.SetPosPixel( aNewPos );
    }
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

        maCertPathLB.Select( pParent );
        maViewCertPB.Disable(); // Own certificate selected

        while( pParent )
        {
            maCertPathLB.Expand( pParent );
            pParent = maCertPathLB.GetParent( pParent );
        }

        CertSelectHdl( NULL );
    }
}

IMPL_LINK_NOARG(CertificateViewerCertPathTP, ViewCertHdl)
{
    SvTreeListEntry* pEntry = maCertPathLB.FirstSelected();
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
    SvTreeListEntry* pEntry = maCertPathLB.FirstSelected();
    if( pEntry )
    {
        CertPath_UserData* pData = (CertPath_UserData*) pEntry->GetUserData();
        if ( pData )
            sStatus = pData->mbValid ? msCertOK : msCertNotValidated;
    }

    maCertStatusML.SetText( sStatus );
    maViewCertPB.Enable( pEntry && ( pEntry != maCertPathLB.Last() ) );
    return 0;
}

void CertificateViewerCertPathTP::Clear( void )
{
    maCertStatusML.SetText( OUString() );
    sal_uLong           i = 0;
    SvTreeListEntry*    pEntry = maCertPathLB.GetEntry( i );
    while( pEntry )
    {
        delete ( CertPath_UserData* ) pEntry->GetUserData();
        ++i;
        pEntry = maCertPathLB.GetEntry( i );
    }

    maCertPathLB.Clear();
}

SvTreeListEntry* CertificateViewerCertPathTP::InsertCert(
    SvTreeListEntry* _pParent, const OUString& _rName, css::uno::Reference< css::security::XCertificate > rxCert,
    bool bValid)
{
    Image aImage = bValid ? maCertImage : maCertNotValidatedImage;
    SvTreeListEntry* pEntry = maCertPathLB.InsertEntry( _rName, aImage, aImage, _pParent );
    pEntry->SetUserData( ( void* ) new CertPath_UserData( rxCert, bValid ) );

    return pEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
