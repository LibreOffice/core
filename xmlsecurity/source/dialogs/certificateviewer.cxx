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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"
#include <xmlsecurity/certificateviewer.hxx>
#include <com/sun/star/security/XCertificate.hpp>

#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <unotools/localedatawrapper.hxx>
#include <unotools/datetime.hxx>

#include "dialogs.hrc"
#include "resourcemanager.hxx"

/* HACK: disable some warnings for MS-C */
#ifdef _MSC_VER
#pragma warning (disable : 4355)    // 4355: this used in initializer-list
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
namespace css = ::com::sun::star;


namespace
{
    void ShrinkToFit( FixedImage& _rImage );
    void AdjustPosAndSize( Control& _rCtrl, Point& _rStartIn_EndOut, long _nXOffset = 0 );

    void ShrinkToFit( FixedImage& _rImg )
    {
        _rImg.SetSizePixel( _rImg.GetImage().GetSizePixel() );
    }

    void AdjustPosAndSize( Control& _rCtrl, Point& _rStartIn_EndOut, long _nOffs )
    {
        _rCtrl.SetPosPixel( _rStartIn_EndOut );
        _rStartIn_EndOut.X() += XmlSec::ShrinkToFitWidth( _rCtrl, _nOffs );
    }
}

CertificateViewer::CertificateViewer(
        Window* _pParent,
        const cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment,
        const cssu::Reference< dcss::security::XCertificate >& _rXCert, sal_Bool bCheckForPrivateKey )
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
        maHintNotTrustedFI.SetText( String( XMLSEC_RES( STR_CERTIFICATE_NOT_VALIDATED ) ) );
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
    cssu::Reference< dcss::security::XCertificate > xCert = mpDlg->mxCert;

    maIssuedToFI.SetText( XmlSec::GetContentPart( xCert->getSubjectName() ) );
    maIssuedByFI.SetText( XmlSec::GetContentPart( xCert->getIssuerName() ) );

    // dynamic length because of the different languages
    long nWidth1 = maIssuedToLabelFI.GetTextWidth( maIssuedToLabelFI.GetText() );
    long nWidth2 = maIssuedByLabelFI.GetTextWidth( maIssuedByLabelFI.GetText() );
    long nNewWidth = Max( nWidth1, nWidth2 ) + 5;
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

    DateTime aDateTimeStart;
    DateTime aDateTimeEnd;
    utl::typeConvert( xCert->getNotValidBefore(), aDateTimeStart );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTimeEnd );
    String sText = maValidDateFI.GetText();
    sText.SearchAndReplace( String::CreateFromAscii( "%SDATE%" ),
                            GetSettings().GetUILocaleDataWrapper().getDate( aDateTimeStart.GetDate() ) );
    sText.SearchAndReplace( String::CreateFromAscii( "%EDATE%" ),
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
    String          maTxt;
    bool            mbFixedWidthFont;

    inline          Details_UserDatat( const String& _rTxt, bool _bFixedWidthFont );
};

inline Details_UserDatat::Details_UserDatat( const String& _rTxt, bool _bFixedWidthFont )
    :maTxt              ( _rTxt )
    ,mbFixedWidthFont   ( _bFixedWidthFont )
{
}


void CertificateViewerDetailsTP::Clear( void )
{
    maElementML.SetText( String() );
    sal_uLong           i = 0;
    SvLBoxEntry*    pEntry = maElementsLB.GetEntry( i );
    while( pEntry )
    {
        delete ( Details_UserDatat* ) pEntry->GetUserData();
        ++i;
        pEntry = maElementsLB.GetEntry( i );
    }

    maElementsLB.Clear();
}

void CertificateViewerDetailsTP::InsertElement( const String& _rField, const String& _rValue,
                                                const String& _rDetails, bool _bFixedWidthFont )
{
    SvLBoxEntry*    pEntry = maElementsLB.InsertEntry( _rField );
    maElementsLB.SetEntryText( _rValue, pEntry, 1 );
    pEntry->SetUserData( ( void* ) new Details_UserDatat( _rDetails, _bFixedWidthFont ) );
}

CertificateViewerDetailsTP::CertificateViewerDetailsTP( Window* _pParent, CertificateViewer* _pDlg )
    :CertificateViewerTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_DETAILS ), _pDlg  )
    ,maElementsLB           ( this, XMLSEC_RES( LB_ELEMENTS ) )
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
    maElementsLB.InsertHeaderEntry( String( XMLSEC_RES( STR_HEADERBAR ) ) );

    // fill list box
    Reference< security::XCertificate > xCert = mpDlg->mxCert;
    sal_uInt16                  nLineBreak = 16;
    const char*             pHexSep = " ";
    String                  aLBEntry;
    String                  aDetails;
    // Certificate Versions are reported wrong (#i35107#) - 0 == "V1", 1 == "V2", ..., n = "V(n+1)"
    aLBEntry = String::CreateFromAscii( "V" );
    aLBEntry += String::CreateFromInt32( xCert->getVersion() + 1 );
    InsertElement( String( XMLSEC_RES( STR_VERSION ) ), aLBEntry, aLBEntry );
    Sequence< sal_Int8 >    aSeq = xCert->getSerialNumber();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( XMLSEC_RES( STR_SERIALNUM ) ), aLBEntry, aDetails, true );

    std::pair< ::rtl::OUString, ::rtl::OUString> pairIssuer =
        XmlSec::GetDNForCertDetailsView(xCert->getIssuerName());
    aLBEntry = pairIssuer.first;
    aDetails = pairIssuer.second;
    InsertElement( String( XMLSEC_RES( STR_ISSUER ) ), aLBEntry, aDetails );
    /*
    aSeq = xCert->getIssuerUniqueID();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( XMLSEC_RES( STR_ISSUER_ID ) ), aLBEntry, aDetails, true );
    */

    DateTime aDateTime;
    utl::typeConvert( xCert->getNotValidBefore(), aDateTime );
    aLBEntry = GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() );
    aLBEntry += String::CreateFromAscii( " " );
    aLBEntry += GetSettings().GetUILocaleDataWrapper().getTime( aDateTime.GetTime() );
    InsertElement( String( XMLSEC_RES( STR_VALIDFROM ) ), aLBEntry, aLBEntry  );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTime );
    aLBEntry = GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() );
    aLBEntry += String::CreateFromAscii( " " );
    aLBEntry += GetSettings().GetUILocaleDataWrapper().getTime( aDateTime.GetTime() );
    InsertElement( String( XMLSEC_RES( STR_VALIDTO ) ), aLBEntry, aLBEntry );

    std::pair< ::rtl::OUString, ::rtl::OUString > pairSubject =
        XmlSec::GetDNForCertDetailsView(xCert->getSubjectName());
    aLBEntry = pairSubject.first;
    aDetails = pairSubject.second;
    InsertElement( String( XMLSEC_RES( STR_SUBJECT ) ), aLBEntry, aDetails );
    /*
    aSeq = xCert->getSubjectUniqueID();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( XMLSEC_RES( STR_SUBJECT_ID ) ), aLBEntry, aDetails, true );
    */
    aLBEntry = aDetails = xCert->getSubjectPublicKeyAlgorithm();
    InsertElement( String( XMLSEC_RES( STR_SUBJECT_PUBKEY_ALGO ) ), aLBEntry, aDetails );
    aSeq = xCert->getSubjectPublicKeyValue();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( XMLSEC_RES( STR_SUBJECT_PUBKEY_VAL ) ), aLBEntry, aDetails, true );

    aLBEntry = aDetails = xCert->getSignatureAlgorithm();
    InsertElement( String( XMLSEC_RES( STR_SIGNATURE_ALGO ) ), aLBEntry, aDetails );

    aSeq = xCert->getSHA1Thumbprint();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( XMLSEC_RES( STR_THUMBPRINT_SHA1 ) ), aLBEntry, aDetails, true );

    aSeq = xCert->getMD5Thumbprint();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( XMLSEC_RES( STR_THUMBPRINT_MD5 ) ), aLBEntry, aDetails, true );

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

IMPL_LINK( CertificateViewerDetailsTP, ElementSelectHdl, void*, EMPTYARG )
{
    SvLBoxEntry*    pEntry = maElementsLB.FirstSelected();
    String          aElementText;
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
    cssu::Reference< dcss::security::XCertificate > mxCert;
    String                                          maStatus;
    bool mbValid;

    CertPath_UserData( cssu::Reference< dcss::security::XCertificate > xCert, bool bValid):
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
    String sText = maViewCertPB.GetText();
    long nTxtW = maViewCertPB.GetTextWidth( sText );
    if ( sText.Search( '~' ) == STRING_NOTFOUND )
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

        String aState;
        sal_Int32 i, nCnt = aCertPath.getLength();
        SvLBoxEntry* pParent = NULL;
        for( i = nCnt; i; )
        {
            const Reference< security::XCertificate > rCert = pCertPath[ --i ];
            String sName = XmlSec::GetContentPart( rCert->getSubjectName() );
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

IMPL_LINK( CertificateViewerCertPathTP, ViewCertHdl, void*, EMPTYARG )
{
    SvLBoxEntry* pEntry = maCertPathLB.FirstSelected();
    if( pEntry )
    {
        CertificateViewer aViewer( this, mpDlg->mxSecurityEnvironment, ((CertPath_UserData*)pEntry->GetUserData())->mxCert, sal_False );
        aViewer.Execute();
    }

    return 0;
}

IMPL_LINK( CertificateViewerCertPathTP, CertSelectHdl, void*, EMPTYARG )
{
    String sStatus;
    SvLBoxEntry* pEntry = maCertPathLB.FirstSelected();
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
    maCertStatusML.SetText( String() );
    sal_uLong           i = 0;
    SvLBoxEntry*    pEntry = maCertPathLB.GetEntry( i );
    while( pEntry )
    {
        delete ( CertPath_UserData* ) pEntry->GetUserData();
        ++i;
        pEntry = maCertPathLB.GetEntry( i );
    }

    maCertPathLB.Clear();
}

SvLBoxEntry* CertificateViewerCertPathTP::InsertCert(
    SvLBoxEntry* _pParent, const String& _rName, cssu::Reference< dcss::security::XCertificate > rxCert,
    bool bValid)
{
    Image aImage = bValid ? maCertImage : maCertNotValidatedImage;
    SvLBoxEntry* pEntry = maCertPathLB.InsertEntry( _rName, aImage, aImage, _pParent );
    pEntry->SetUserData( ( void* ) new CertPath_UserData( rxCert, bValid ) );

    return pEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
