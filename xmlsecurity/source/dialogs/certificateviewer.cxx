/*************************************************************************
 *
 *  $RCSfile: certificateviewer.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-22 15:37:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <xmlsecurity/certificateviewer.hxx>

// Only for bigIntegerToNumericString
#include <xmlsecurity/xmlsignaturehelper.hxx>


#ifndef _COM_SUN_STAR_SECURITY_XCERTIFICATE_HPP_
#include <com/sun/star/security/XCertificate.hpp>
#endif

#include <com/sun/star/security/CertificateCharacters.hpp>


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


namespace
{
    void ShrinkToFit( FixedImage& _rImage );
    long ShrinkToFitWidth( Control& _rCtrl, long _nOffs = 0 );      // return = new width
    void AdjustPosAndSize( Control& _rCtrl, Point& _rStartIn_EndOut, long _nXOffset = 0 );
    void AlignAfterImage( const FixedImage& _rImage, Control& _rCtrl, long _nXOffset = 0 );
    void AlignAfterImage( const FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset = 0 );

    void ShrinkToFit( FixedImage& _rImg )
    {
        _rImg.SetSizePixel( _rImg.GetImage().GetSizePixel() );
    }

    long ShrinkToFitWidth( Control& _rCtrl, long _nOffs )
    {
        long    nWidth = _rCtrl.GetTextWidth( _rCtrl.GetText() );
        Size    aSize( _rCtrl.GetSizePixel() );
        nWidth += _nOffs;
        aSize.Width() = nWidth;
        _rCtrl.SetSizePixel( aSize );
        return nWidth;
    }

    void AdjustPosAndSize( Control& _rCtrl, Point& _rStartIn_EndOut, long _nOffs )
    {
        _rCtrl.SetPosPixel( _rStartIn_EndOut );
        _rStartIn_EndOut.X() += ShrinkToFitWidth( _rCtrl, _nOffs );
    }

    void AlignAfterImage( const FixedImage& _rImage, Control& _rCtrl, long _nXOffset )
    {
        Point   aPos( _rImage.GetPosPixel() );
        Size    aSize( _rImage.GetSizePixel() );
        long    n = aPos.X();
        n += aSize.Width();
        n += _nXOffset;
        aPos.X() = n;
        n = aPos.Y();
        n += aSize.Height() / 2;                    // y-position is in the middle of the image
        n -= _rCtrl.GetSizePixel().Height() / 2;    // center Control
        aPos.Y() = n;
        _rCtrl.SetPosPixel( aPos );
    }

    void AlignAfterImage( const FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset )
    {
        AlignAfterImage( _rImage, static_cast< Control& >( _rFI ), _nXOffset );
        ShrinkToFitWidth( _rFI );
    }
};

CertificateViewer::CertificateViewer(
        Window* _pParent,
        cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment,
        cssu::Reference< dcss::security::XCertificate >& _rXCert )
    :TabDialog      ( _pParent, XMLSEC_RES( RID_XMLSECDLG_CERTVIEWER ) )
    ,maTabCtrl      ( this, ResId( 1 ) )
    ,maOkBtn        ( this, ResId( BTN_OK ) )
    ,maHelpBtn      ( this, ResId( BTN_HELP ) )
{
    FreeResource();

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
    ,maFrameWin             ( this, ResId( WIN_FRAME ) )
    ,maCertImg              ( this, ResId( IMG_CERT ) )
    ,maCertInfoFI           ( this, ResId( FI_CERTINFO ) )
    ,maSep1FL               ( this, ResId( FL_SEP1 ) )
    ,maHintNotTrustedFI     ( this, ResId( FI_HINTNOTTRUST ) )
    ,maSep2FL               ( this, ResId( FL_SEP2 ) )
    ,maIssuedToLabelFI      ( this, ResId( FI_ISSTOLABEL ) )
    ,maIssuedToFI           ( this, ResId( FI_ISSTO ) )
    ,maIssuedByLabelFI      ( this, ResId( FI_ISSBYLABEL ) )
    ,maIssuedByFI           ( this, ResId( FI_ISSBY ) )
    ,maValidFromLabelFI     ( this, ResId( FI_VALFROMLABEL ) )
    ,maValidFromFI          ( this, ResId( FI_VALFROM ) )
    ,maValidToLabelFI       ( this, ResId( FI_VALTOLABEL ) )
    ,maValidToFI            ( this, ResId( FI_VALTO ) )
    ,maKeyImg               ( this, ResId( IMG_KEY ) )
    ,maHintCorrespPrivKeyFI ( this, ResId( FI_CORRPRIVKEY ) )
{
    FreeResource();

    Wallpaper       aBack( GetSettings().GetStyleSettings().GetWindowColor() );

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
    maValidFromLabelFI.SetBackground( aBack );
    maValidFromFI.SetBackground( aBack );
    maValidToLabelFI.SetBackground( aBack );
    maValidToFI.SetBackground( aBack );
    maKeyImg.SetBackground( aBack );
    maHintCorrespPrivKeyFI.SetBackground( aBack );

    // make some bold
    Font    aFnt( maCertInfoFI.GetFont() );
    aFnt.SetWeight( WEIGHT_BOLD );
    maCertInfoFI.SetFont( aFnt );
    maHintNotTrustedFI.SetFont( aFnt );
    maIssuedToLabelFI.SetFont( aFnt );
    maIssuedByLabelFI.SetFont( aFnt );
    maValidFromLabelFI.SetFont( aFnt );
    maValidToLabelFI.SetFont( aFnt );

    // insert data
    cssu::Reference< dcss::security::XCertificate > xCert = mpDlg->mxCert;

    String  aCN_Id( String::CreateFromAscii( "CN" ) );

    maIssuedToFI.SetText( XmlSec::GetContentPart( xCert->getSubjectName(), aCN_Id ) );
    maIssuedByFI.SetText( XmlSec::GetContentPart( xCert->getIssuerName(), aCN_Id ) );

    DateTime aDateTime;
    utl::typeConvert( xCert->getNotBefore(), aDateTime );
    maValidFromFI.SetText( GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() ) );
    utl::typeConvert( xCert->getNotAfter(), aDateTime );
    maValidToFI.SetText( GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() ) );

    // recalc positions for date fields according to real size
    Point   aPos( maValidFromLabelFI.GetPosPixel() );
    AdjustPosAndSize( maValidFromLabelFI, aPos, 5 );
    AdjustPosAndSize( maValidFromFI, aPos, 5 );
    AdjustPosAndSize( maValidToLabelFI, aPos, 5 );
    AdjustPosAndSize( maValidToFI, aPos, 5 );

    // adjust position of fixed text depending on image sizes
    ShrinkToFit( maCertImg );
    ShrinkToFit( maKeyImg );
    AlignAfterImage( maCertImg, maCertInfoFI, 12 );
    AlignAfterImage( maKeyImg, maHintCorrespPrivKeyFI, 12 );

    // Check if we have the private key...
    long nCertificateCharacters = _pDlg->mxSecurityEnvironment->getCertificateCharacters( xCert );
    if ( !( nCertificateCharacters & security::CertificateCharacters::CERT_CHARACTER_HAS_PRIVATE_KEY ) )
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
    ULONG           i = 0;
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
    ,maElementsLB           ( this, ResId( LB_ELEMENTS ) )
    ,maElementML            ( this, ResId( ML_ELEMENT ) )
    ,maStdFont              ( maElementML.GetControlFont() )
    ,maFixedWidthFont       ( OutputDevice::GetDefaultFont( DEFAULTFONT_UI_FIXED, LANGUAGE_DONTKNOW, DEFAULTFONT_FLAGS_ONLYONE, this ) )
{
    WinBits nStyle = maElementsLB.GetStyle();
    nStyle &= ~WB_HSCROLL;
    maElementsLB.SetStyle( nStyle );

    maFixedWidthFont.SetHeight( maStdFont.GetHeight() );

    static long nTabs[] = { 2, 0, 30*CS_LB_WIDTH/100 };
    maElementsLB.SetTabs( &nTabs[ 0 ] );
    maElementsLB.InsertHeaderEntry( String( ResId( STR_HEADERBAR ) ) );

    // fill list box
    Reference< security::XCertificate > xCert = mpDlg->mxCert;
    UINT16                  nLineBreak = 16;
    const char*             pHexSep = " ";
    String                  aLBEntry;
    String                  aDetails;
    aLBEntry = String::CreateFromAscii( "V" );
    aLBEntry += String::CreateFromInt32( xCert->getVersion() );
    InsertElement( String( ResId( STR_VERSION ) ), aLBEntry, aLBEntry );
    Sequence< sal_Int8 >    aSeq = xCert->getSerialNumber();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( ResId( STR_SERIALNUM ) ), aLBEntry, aDetails, true );

    aLBEntry = XmlSec::GetPureContent( xCert->getIssuerName(), ", " );
    aDetails = XmlSec::GetPureContent( xCert->getIssuerName(), "\n", true );
    InsertElement( String( ResId( STR_ISSUER ) ), aLBEntry, aDetails );
    /*
    aSeq = xCert->getIssuerUniqueID();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( ResId( STR_ISSUER_ID ) ), aLBEntry, aDetails, true );
    */

    DateTime aDateTime;
    utl::typeConvert( xCert->getNotBefore(), aDateTime );
    aLBEntry = GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() );
    aLBEntry += String::CreateFromAscii( " " );
    aLBEntry += GetSettings().GetUILocaleDataWrapper().getTime( aDateTime.GetTime() );
    InsertElement( String( ResId( STR_VALIDFROM ) ), aLBEntry, aLBEntry  );
    utl::typeConvert( xCert->getNotAfter(), aDateTime );
    aLBEntry = GetSettings().GetUILocaleDataWrapper().getDate( aDateTime.GetDate() );
    aLBEntry += String::CreateFromAscii( " " );
    aLBEntry += GetSettings().GetUILocaleDataWrapper().getTime( aDateTime.GetTime() );
    InsertElement( String( ResId( STR_VALIDTO ) ), aLBEntry, aLBEntry );

    aLBEntry = XmlSec::GetPureContent( xCert->getSubjectName(), ", " );
    aDetails = XmlSec::GetPureContent( xCert->getSubjectName(), "\n", true );
    InsertElement( String( ResId( STR_SUBJECT ) ), aLBEntry, aDetails );
    /*
    aSeq = xCert->getSubjectUniqueID();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( ResId( STR_SUBJECT_ID ) ), aLBEntry, aDetails, true );
    */
    aLBEntry = aDetails = xCert->getSubjectPublicKeyAlgorithm();
    InsertElement( String( ResId( STR_SUBJECT_PUBKEY_ALGO ) ), aLBEntry, aDetails );
    aSeq = xCert->getSubjectPublicKeyValue();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( ResId( STR_SUBJECT_PUBKEY_VAL ) ), aLBEntry, aDetails, true );

    aLBEntry = aDetails = xCert->getSignatureAlgorithm();
    InsertElement( String( ResId( STR_SIGNATURE_ALGO ) ), aLBEntry, aDetails );

    aSeq = xCert->getSHA1Thumbprint();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( ResId( STR_THUMBPRINT_SHA1 ) ), aLBEntry, aDetails, true );

    aSeq = xCert->getMD5Thumbprint();
    aLBEntry = XmlSec::GetHexString( aSeq, pHexSep );
    aDetails = XmlSec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( String( ResId( STR_THUMBPRINT_MD5 ) ), aLBEntry, aDetails, true );

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

    CertPath_UserData( cssu::Reference< dcss::security::XCertificate > xCert ) { mxCert = xCert; }
};


CertificateViewerCertPathTP::CertificateViewerCertPathTP( Window* _pParent, CertificateViewer* _pDlg )
    :CertificateViewerTP    ( _pParent, XMLSEC_RES( RID_XMLSECTP_CERTPATH ), _pDlg  )
    ,maCertPathFT           ( this, ResId( FT_CERTPATH ) )
    ,maCertPathLB           ( this, ResId( LB_SIGNATURES ) )
    ,maViewCertPB           ( this, ResId( BTN_VIEWCERT ) )
    ,maCertStatusFT         ( this, ResId( FT_CERTSTATUS ) )
    ,maCertStatusML         ( this, ResId( ML_CERTSTATUS ) )
{
    // fill list box
    maCertPathLB.SetNodeDefaultImages();
    maCertPathLB.SetSublistOpenWithLeftRight();

    Sequence< Reference< security::XCertificate > > aCertPath = _pDlg->mxSecurityEnvironment->buildCertificatePath( _pDlg->mxCert );
    const Reference< security::XCertificate >* pCertPath = aCertPath.getConstArray();

    String          aCN_Id( String::CreateFromAscii( "CN" ) );
    String          aState;
    int             nCnt = aCertPath.getLength();
    SvLBoxEntry*    pParent = NULL;
    for( int i = nCnt; i; )
    {
           pParent = InsertCert( pParent, XmlSec::GetContentPart( pCertPath[ --i ]->getSubjectName(), aCN_Id ), pCertPath[ i ] );
    }

    maCertPathLB.Select( pParent );
    maViewCertPB.Disable(); // Own certificate selected

    while( pParent )
    {
        maCertPathLB.Expand( pParent );
        pParent = maCertPathLB.GetParent( pParent );
    }

    FreeResource();

    maViewCertPB.SetClickHdl( LINK( this, CertificateViewerCertPathTP, ViewCertHdl ) );
    maCertPathLB.SetSelectHdl( LINK( this, CertificateViewerCertPathTP, CertSelectHdl ) );

    // MT->GT: After EA, please move path stuff to ActivatePage() and asure it's only done once.
    // Who knows how expensive buildCertificatePath can be, and maybe nobody looks at this page.
}

CertificateViewerCertPathTP::~CertificateViewerCertPathTP()
{
    Clear();
}

void CertificateViewerCertPathTP::ActivatePage()
{
}

IMPL_LINK( CertificateViewerCertPathTP, ViewCertHdl, void*, EMPTYARG )
{
    SvLBoxEntry* pEntry = maCertPathLB.FirstSelected();
    if( pEntry )
    {
        CertificateViewer aViewer( this, mpDlg->mxSecurityEnvironment, ((CertPath_UserData*)pEntry->GetUserData())->mxCert );
        aViewer.Execute();
    }

    return 0;
}

IMPL_LINK( CertificateViewerCertPathTP, CertSelectHdl, void*, EMPTYARG )
{
    String  aStatus;

    SvLBoxEntry* pEntry = maCertPathLB.FirstSelected();
    if( pEntry )
    {
        CertPath_UserData* pData = (CertPath_UserData*) pEntry->GetUserData();
        if ( !pData->maStatus.Len() )
        {
            // Figure out status...
            // pData->maStatus = ...
        }
        aStatus = pData->maStatus;
    }

    maCertStatusML.SetText( aStatus );

    maViewCertPB.Enable( pEntry && ( pEntry != maCertPathLB.Last() ) );

    return 0;
}

void CertificateViewerCertPathTP::Clear( void )
{
    maCertStatusML.SetText( String() );
    ULONG           i = 0;
    SvLBoxEntry*    pEntry = maCertPathLB.GetEntry( i );
    while( pEntry )
    {
        delete ( CertPath_UserData* ) pEntry->GetUserData();
        ++i;
        pEntry = maCertPathLB.GetEntry( i );
    }

    maCertPathLB.Clear();
}

SvLBoxEntry* CertificateViewerCertPathTP::InsertCert( SvLBoxEntry* _pParent, const String& _rName, cssu::Reference< dcss::security::XCertificate > rxCert )
{
    SvLBoxEntry* pEntry = maCertPathLB.InsertEntry( _rName, _pParent );
    pEntry->SetUserData( ( void* ) new CertPath_UserData( rxCert ) );

    return pEntry;
}
