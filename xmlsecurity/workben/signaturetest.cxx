/*************************************************************************
 *
 *  $RCSfile: signaturetest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-15 07:16:12 $
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

#include <sal/main.h>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/filectrl.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

#include <svtools/docpasswdrequest.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <unotools/streamhelper.hxx>

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/configurationkeys.hxx>

// Will be in comphelper if CWS MAV09 is integrated
#include <storagehelper.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/certificatechooser.hxx>

#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star;

void Main();

#define TEXTFIELDWIDTH  80
#define TEXTFIELDSTARTX 10

#define EDITWIDTH       200
#define EDITHEIGHT      20

#define FIXEDLINEHEIGHT 15

#define BUTTONWIDTH     50
#define BUTTONHEIGHT    22
#define BUTTONSPACE     20

#define LISTBOXHEIGHT   120

// #define TEST_IMPLEMENTATION_DIRECTLY


// -----------------------------------------------------------------------

    SAL_IMPLEMENT_MAIN()
{
    uno::Reference< lang::XMultiServiceFactory > xMSF;
    try
    {
        uno::Reference< uno::XComponentContext > xCtx( cppu::defaultBootstrap_InitialComponentContext() );
        if ( !xCtx.is() )
        {
            DBG_ERROR( "Error creating initial component context!" );
            return -1;
        }

        xMSF = uno::Reference< lang::XMultiServiceFactory >(xCtx->getServiceManager(), uno::UNO_QUERY );

        if ( !xMSF.is() )
        {
            DBG_ERROR( "No service manager!" );
            return -1;
        }

        // Init USB
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[ 0 ] <<= rtl::OUString::createFromAscii( UCB_CONFIGURATION_KEY1_LOCAL );
        aArgs[ 1 ] <<= rtl::OUString::createFromAscii( UCB_CONFIGURATION_KEY2_OFFICE );
        sal_Bool bSuccess = ::ucb::ContentBroker::initialize( xMSF, aArgs );
        if ( !bSuccess )
        {
            DBG_ERROR( "Error creating UCB!" );
            return -1;
        }

    }
    catch ( uno::Exception const & )
    {
        DBG_ERROR( "Exception during creation of initial component context!" );
        return -1;
    }
    comphelper::setProcessServiceFactory( xMSF );

    InitVCL( xMSF );
    ::Main();
    DeInitVCL();

    return 0;
}

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
private:
    FixedLine   maTokenLine;
    CheckBox    maCryptoCheckBox;
    FixedText   maFixedTextTokenName;
    FileControl maEditTokenName;
    FixedLine   maTest1Line;
    FixedText   maFixedTextXMLFileName;
    FileControl maEditXMLFileName;
    FixedText   maFixedTextBINFileName;
    FileControl maEditBINFileName;
    FixedText   maFixedTextSIGFileName;
    FileControl maEditSIGFileName;
    PushButton  maSignButton;
    PushButton  maVerifyButton;
    FixedLine   maTest2Line;
    FixedText   maFixedTextDOCFileName;
    FileControl maEditDOCFileName;
    PushButton  maDigitalSignaturesButton;
    PushButton  maVerifyDigitalSignaturesButton;
    FixedLine   maHintLine;
    FixedText   maHintText;

    DECL_LINK(  CryptoCheckBoxHdl, CheckBox* );
    DECL_LINK(  SignButtonHdl, Button* );
    DECL_LINK(  VerifyButtonHdl, Button* );
    DECL_LINK(  DigitalSignaturesWithServiceHdl, Button* );
    DECL_LINK(  VerifyDigitalSignaturesHdl, Button* );
    DECL_LINK(  DigitalSignaturesWithTokenHdl, Button* );
    DECL_LINK(  StartVerifySignatureHdl, void* );

public:
                MyWin( Window* pParent, WinBits nWinStyle );

};

// -----------------------------------------------------------------------

void Main()
{
    MyWin aMainWin( NULL, WB_APP | WB_STDWORK | WB_3DLOOK);
    aMainWin.Show();

    Application::Execute();
}

// -----------------------------------------------------------------------

MyWin::MyWin( Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle ),
    maTokenLine( this ),
    maTest1Line( this ),
    maTest2Line( this ),
    maHintLine( this ),
    maFixedTextXMLFileName( this ),
    maEditXMLFileName( this, WB_BORDER ),
    maFixedTextBINFileName( this ),
    maEditBINFileName( this, WB_BORDER ),
    maFixedTextSIGFileName( this ),
    maEditSIGFileName( this, WB_BORDER ),
    maFixedTextTokenName( this ),
    maEditTokenName( this, WB_BORDER ),
    maFixedTextDOCFileName( this ),
    maEditDOCFileName( this, WB_BORDER ),
    maSignButton( this ),
    maVerifyButton( this ),
    maDigitalSignaturesButton( this ),
    maVerifyDigitalSignaturesButton( this ),
    maHintText( this, WB_WORDBREAK ),
    maCryptoCheckBox( this )

{
#ifdef TEST_IMPLEMENTATION_DIRECTLY
    Size aOutputSize( 400, 600 );
#else
    Size aOutputSize( 400, 400 );
#endif
    SetOutputSizePixel( aOutputSize );
    SetText( String( RTL_CONSTASCII_USTRINGPARAM( "XML Signature Test" ) ) );

    long nY = 15;

    maTokenLine.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maTokenLine.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Crypto Settings" ) ) );
    maTokenLine.Show();

    nY += EDITHEIGHT*3/2;

    maCryptoCheckBox.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maCryptoCheckBox.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Use Default Token (NSS option only)" ) ) );
    maCryptoCheckBox.Check( TRUE );
    maEditTokenName.Disable();
    maFixedTextTokenName.Disable();
    maCryptoCheckBox.SetClickHdl( LINK( this, MyWin, CryptoCheckBoxHdl ) );
    maCryptoCheckBox.Show();

    nY += EDITHEIGHT;

    maFixedTextTokenName.SetPosSizePixel( TEXTFIELDSTARTX, nY, TEXTFIELDWIDTH, EDITHEIGHT );
    maFixedTextTokenName.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Crypto Token:" ) ) );
    maFixedTextTokenName.Show();

    maEditTokenName.SetPosSizePixel( TEXTFIELDSTARTX+TEXTFIELDWIDTH, nY, EDITWIDTH, EDITHEIGHT );
    maEditTokenName.Show();

    nY += EDITHEIGHT*3;

#ifdef TEST_IMPLEMENTATION_DIRECTLY

    maTest1Line.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maTest1Line.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Test simple files" ) ) );
    maTest1Line.Show();

    nY += EDITHEIGHT*3/2;

    maFixedTextXMLFileName.SetPosSizePixel( TEXTFIELDSTARTX, nY, TEXTFIELDWIDTH, EDITHEIGHT );
    maFixedTextXMLFileName.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "XML File:" ) ) );
    maFixedTextXMLFileName.Show();

    maEditXMLFileName.SetPosSizePixel( TEXTFIELDSTARTX+TEXTFIELDWIDTH, nY, EDITWIDTH, EDITHEIGHT );
    maEditXMLFileName.Show();

    nY += EDITHEIGHT*3/2;

    maFixedTextBINFileName.SetPosSizePixel( TEXTFIELDSTARTX, nY, TEXTFIELDWIDTH, EDITHEIGHT );
    maFixedTextBINFileName.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Binary File:" ) ) );
    maFixedTextBINFileName.Show();

    maEditBINFileName.SetPosSizePixel( TEXTFIELDSTARTX+TEXTFIELDWIDTH, nY, EDITWIDTH, EDITHEIGHT );
    maEditBINFileName.Show();

    nY += EDITHEIGHT*3/2;

    maFixedTextSIGFileName.SetPosSizePixel( TEXTFIELDSTARTX, nY, TEXTFIELDWIDTH, EDITHEIGHT );
    maFixedTextSIGFileName.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Signature File:" ) ) );
    maFixedTextSIGFileName.Show();

    maEditSIGFileName.SetPosSizePixel( TEXTFIELDSTARTX+TEXTFIELDWIDTH, nY, EDITWIDTH, EDITHEIGHT );
    maEditSIGFileName.Show();

    nY += EDITHEIGHT*2;

    maSignButton.SetPosSizePixel( TEXTFIELDSTARTX, nY, BUTTONWIDTH, BUTTONHEIGHT );
    maSignButton.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Sign" ) ) );
    maSignButton.SetClickHdl( LINK( this, MyWin, SignButtonHdl ) );
    maSignButton.Show();

    maVerifyButton.SetPosSizePixel( TEXTFIELDSTARTX+BUTTONWIDTH+BUTTONSPACE, nY, BUTTONWIDTH, BUTTONHEIGHT );
    maVerifyButton.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Verify" ) ) );
    maVerifyButton.SetClickHdl( LINK( this, MyWin, VerifyButtonHdl ) );
    maVerifyButton.Show();

    nY += EDITHEIGHT*3;

#endif // TEST_IMPLEMENTATION_DIRECTLY

    maTest2Line.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maTest2Line.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Test Office Document" ) ) );
    maTest2Line.Show();

    nY += EDITHEIGHT*3/2;


    maFixedTextDOCFileName.SetPosSizePixel( TEXTFIELDSTARTX, nY, TEXTFIELDWIDTH, EDITHEIGHT );
    maFixedTextDOCFileName.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Office File:" ) ) );
    maFixedTextDOCFileName.Show();

    maEditDOCFileName.SetPosSizePixel( TEXTFIELDSTARTX+TEXTFIELDWIDTH, nY, EDITWIDTH, EDITHEIGHT );
    maEditDOCFileName.Show();

    nY += EDITHEIGHT*2;

    maDigitalSignaturesButton.SetPosSizePixel( TEXTFIELDSTARTX, nY, BUTTONWIDTH*2, BUTTONHEIGHT );
    maDigitalSignaturesButton.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Digital Signatures..." ) ) );
    maDigitalSignaturesButton.SetClickHdl( LINK( this, MyWin, DigitalSignaturesWithServiceHdl ) );
    maDigitalSignaturesButton.Show();

    maVerifyDigitalSignaturesButton.SetPosSizePixel( TEXTFIELDSTARTX+BUTTONWIDTH*2+BUTTONSPACE, nY, BUTTONWIDTH*2, BUTTONHEIGHT );
    maVerifyDigitalSignaturesButton.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Verify Signatures" ) ) );
    maVerifyDigitalSignaturesButton.SetClickHdl( LINK( this, MyWin, VerifyDigitalSignaturesHdl ) );
    maVerifyDigitalSignaturesButton.Show();

    nY += EDITHEIGHT*2;

    maHintLine.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maHintLine.Show();

    nY += EDITHEIGHT*2;

    maHintText.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, aOutputSize.Height()-nY );
    maHintText.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Hint: Copy crypto files from xmlsecurity/tools/cryptoken/nss and sample files from xmlsecurity/tools/examples to <temp>/nss.\nThis location will be used from the demo as the default location." ) ) );
    maHintText.Show();

    // Help the user with some default values
    ::rtl::OUString aTempDirURL;
    ::osl::File::getTempDirURL( aTempDirURL );
    INetURLObject aURLObj( aTempDirURL );
    aURLObj.insertName( String( RTL_CONSTASCII_USTRINGPARAM( "nss" ) ), true );
    ::rtl::OUString aNSSFolder = aURLObj.getFSysPath( INetURLObject::FSYS_DETECT );
    String aDefaultXMLFileName( aNSSFolder );
    maEditXMLFileName.SetText( aNSSFolder + String( RTL_CONSTASCII_USTRINGPARAM( "demo-sample.xml" ) ) );
    maEditBINFileName.SetText( aNSSFolder + String( RTL_CONSTASCII_USTRINGPARAM( "demo-sample.gif" ) ) );
    maEditDOCFileName.SetText( aNSSFolder + String( RTL_CONSTASCII_USTRINGPARAM( "demo-sample.sxw" ) ) );
    maEditSIGFileName.SetText( aNSSFolder + String( RTL_CONSTASCII_USTRINGPARAM( "demo-result.xml" ) ) );
    maEditTokenName.SetText( aNSSFolder );

#ifdef WNT
    maEditTokenName.SetText( String() );
    maEditTokenName.Disable();
    maCryptoCheckBox.Disable();
#endif

}

IMPL_LINK( MyWin, CryptoCheckBoxHdl, CheckBox*, EMPTYARG )
{
    if ( maCryptoCheckBox.IsChecked() )
    {
        maEditTokenName.Disable();
        maFixedTextTokenName.Disable();
    }
    else
    {
        maEditTokenName.Enable();
        maFixedTextTokenName.Enable();
    }
    return 1;
}

IMPL_LINK( MyWin, DigitalSignaturesWithServiceHdl, Button*, EMPTYARG )
{
    rtl::OUString aDocFileName = maEditDOCFileName.GetText();
    uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromURL(
            aDocFileName, embed::ElementModes::READWRITE, comphelper::getProcessServiceFactory() );

    uno::Reference< security::XDocumentDigitalSignatures > xD(
        comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ) ), uno::UNO_QUERY );
    if ( xD.is() )
        xD->SignDocumentContent( xStore );


    return 0;
}

IMPL_LINK( MyWin, VerifyDigitalSignaturesHdl, Button*, EMPTYARG )
{
    rtl::OUString aDocFileName = maEditDOCFileName.GetText();
    uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromURL(
            aDocFileName, embed::ElementModes::READWRITE, comphelper::getProcessServiceFactory() );

    uno::Reference< security::XDocumentDigitalSignatures > xD(
        comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ) ), uno::UNO_QUERY );
    if ( xD.is() )
    {
        uno::Sequence< security::DocumentSignaturesInformation > aInfos = xD->VerifyDocumentContentSignatures( xStore );
        int nInfos = aInfos.getLength();
        for ( int n = 0; n < nInfos; n++ )
        {
            security::DocumentSignaturesInformation& rInf = aInfos[n];
            String aText( RTL_CONSTASCII_USTRINGPARAM( "The document is signed by\n\n  " ) );
            aText += String( rInf.Signer->getSubjectName() );
            aText += String( RTL_CONSTASCII_USTRINGPARAM( "\n\n The signature is " ) );
            if ( !rInf.SignatureIsValid )
                aText += String( RTL_CONSTASCII_USTRINGPARAM( "NOT " ) );
            aText += String( RTL_CONSTASCII_USTRINGPARAM( "valid" ) );
            InfoBox( this, aText ).Execute();
        }

    }


    return 0;
}


#ifdef TEST_IMPLEMENTATION_DIRECTLY

IMPL_LINK( MyWin, DigitalSignaturesWithTokenHdl, Button*, EMPTYARG )
{
    String aDocFileName = maEditDOCFileName.GetText();
    String aTokenFileName = maEditTokenName.GetText();

    DigitalSignaturesDialog aSignaturesDialog( this, comphelper::getProcessServiceFactory(), SignatureModeDocumentContent, false );

    bool bInit = aSignaturesDialog.Init( aTokenFileName );
    if ( !bInit )
    {
        ErrorBox( this, WB_OK, String( RTL_CONSTASCII_USTRINGPARAM( "Error initializing security context!" ) ) ).Execute();
        return 0;
    }

    uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromURL(
            aDocFileName, embed::ElementModes::READWRITE, comphelper::getProcessServiceFactory() );

    aSignaturesDialog.SetStorage( xStore );

    aSignaturesDialog.Execute();

    return 0;
}

IMPL_LINK( MyWin, SignButtonHdl, Button*, EMPTYARG )
{
    String aXMLFileName = maEditXMLFileName.GetText();
    String aBINFileName = maEditBINFileName.GetText();
    String aSIGFileName = maEditSIGFileName.GetText();

    String aTokenFileName;
    if ( !maCryptoCheckBox.IsChecked() )
        aTokenFileName = maEditTokenName.GetText();

    XMLSignatureHelper aSignatureHelper( comphelper::getProcessServiceFactory() );
    bool bInit = aSignatureHelper.Init( aTokenFileName );

    if ( !bInit )
    {
        ErrorBox( this, WB_OK, String( RTL_CONSTASCII_USTRINGPARAM( "Error initializing security context!" ) ) ).Execute();
        return 0;
    }

    uno::Reference< ::com::sun::star::security::XCertificate > xCertToUse;
    CertificateChooser aChooser( this, aSignatureHelper.GetSecurityEnvironment(), SignatureInformations() );
    if ( aChooser.Execute() )
        xCertToUse = aChooser.GetSelectedCertificate();

    if ( !xCertToUse.is() )
        return 0;


    aSignatureHelper.StartMission();

    sal_Int32 nSecurityId = aSignatureHelper.GetNewSecurityId();

    aSignatureHelper.SetX509Certificate( nSecurityId, xCertToUse->getIssuerName(), bigIntegerToNumericString( xCertToUse->getSerialNumber() ) );

    aSignatureHelper.AddForSigning( nSecurityId, aXMLFileName, aXMLFileName, sal_False );
    aSignatureHelper.AddForSigning( nSecurityId, aBINFileName, aBINFileName, sal_True );

    SvFileStream* pStream = new SvFileStream( aSIGFileName, STREAM_WRITE );
    SvLockBytesRef xLockBytes = new SvLockBytes( pStream, TRUE );
     uno::Reference< io::XOutputStream > xOutputStream = new utl::OOutputStreamHelper( xLockBytes );
    bool bDone = aSignatureHelper.CreateAndWriteSignature( xOutputStream );

    aSignatureHelper.EndMission();

    if ( !bDone )
    {
        ErrorBox( this, WB_OK, String( RTL_CONSTASCII_USTRINGPARAM( "Error creating Signature!" ) ) ).Execute();
    }
    else
    {
        rtl::OUString aInfo( String( RTL_CONSTASCII_USTRINGPARAM( "Signature successfully created!\n\n" ) ) );
        // aInfo += getSignatureInformationmations( aSignatureHelper.getAllSignatureInformation(), aSignatureHelper.GetSecurityEnvironment() );


        InfoBox( this, aInfo ).Execute();
    }

    // Check for more detailed results...

    return 0;
}

IMPL_LINK( MyWin, VerifyButtonHdl, Button*, EMPTYARG )
{
    String aXMLFileName = maEditXMLFileName.GetText();
    String aBINFileName = maEditBINFileName.GetText();
    String aSIGFileName = maEditSIGFileName.GetText();

    String aTokenFileName;
    if ( !maCryptoCheckBox.IsChecked() )
        aTokenFileName = maEditTokenName.GetText();

    XMLSignatureHelper aSignatureHelper( comphelper::getProcessServiceFactory() );
    bool bInit = aSignatureHelper.Init( aTokenFileName );

    if ( !bInit )
    {
        ErrorBox( this, WB_OK, String( RTL_CONSTASCII_USTRINGPARAM( "Error initializing security context!" ) ) ).Execute();
        return 0;
    }

    aSignatureHelper.SetStartVerifySignatureHdl( LINK( this, MyWin, StartVerifySignatureHdl ) );

    aSignatureHelper.StartMission();

    SvFileStream* pStream = new SvFileStream( aSIGFileName, STREAM_READ );
    pStream->Seek( STREAM_SEEK_TO_END );
    ULONG nBytes = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    SvLockBytesRef xLockBytes = new SvLockBytes( pStream, TRUE );
     uno::Reference< io::XInputStream > xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );
    bool bDone = aSignatureHelper.ReadAndVerifySignature( xInputStream );
    xInputStream->closeInput();

    aSignatureHelper.EndMission();

    if ( !bDone )
        ErrorBox( this, WB_OK, String( RTL_CONSTASCII_USTRINGPARAM( "Error in Signature!" ) ) ).Execute();
    else
        InfoBox( this, String( RTL_CONSTASCII_USTRINGPARAM( "Signatures verified without any problems!" ) ) ).Execute();

    return 0;
}

IMPL_LINK( MyWin, StartVerifySignatureHdl, void*, EMPTYARG )
{
    QueryBox aQueryBox( this, WB_YES_NO|WB_DEF_YES, String( RTL_CONSTASCII_USTRINGPARAM( "Found Signature - Verify?" ) ) );
    return ( aQueryBox.Execute() == RET_YES ) ? 1 : 0;
}


#endif // #ifdef TEST_IMPLEMENTATION_DIRECTLY
