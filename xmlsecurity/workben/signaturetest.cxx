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

// Will be in comphelper if CWS MAV09 is integrated
#include <comphelper/storagehelper.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/biginteger.hxx>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>

using namespace css;

void Main();

#define TEXTFIELDWIDTH  80
#define TEXTFIELDSTARTX 10

#define EDITWIDTH       200
#define EDITHEIGHT      20

#define FIXEDLINEHEIGHT 15

#define BUTTONWIDTH     50
#define BUTTONHEIGHT    22
#define BUTTONSPACE     20

    SAL_IMPLEMENT_MAIN()
{
    uno::Reference< lang::XMultiServiceFactory > xMSF;
    try
    {
        uno::Reference< uno::XComponentContext > xCtx( cppu::defaultBootstrap_InitialComponentContext() );
        if ( !xCtx.is() )
        {
            OSL_FAIL( "Error creating initial component context!" );
            return -1;
        }

        xMSF.set(xCtx->getServiceManager(), uno::UNO_QUERY );

        if ( !xMSF.is() )
        {
            OSL_FAIL( "No service manager!" );
            return -1;
        }
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "Exception during creation of initial component context!" );
        return -1;
    }
    comphelper::setProcessServiceFactory( xMSF );

    InitVCL();
    ::Main();
    DeInitVCL();

    return 0;
}

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

    DECL_LINK_TYPED(  CryptoCheckBoxHdl, CheckBox*, void );
    DECL_LINK_TYPED(  DigitalSignaturesWithServiceHdl, Button*, void );
    DECL_LINK_TYPED(  VerifyDigitalSignaturesHdl, Button*, void );

public:
                MyWin( vcl::Window* pParent, WinBits nWinStyle );

};

void Main()
{
    MyWin aMainWin( NULL, WB_APP | WB_STDWORK | WB_3DLOOK);
    aMainWin.Show();

    Application::Execute();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
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
    Size aOutputSize( 400, 400 );
    SetOutputSizePixel( aOutputSize );
    SetText( OUString("XML Signature Test") );

    long nY = 15;

    maTokenLine.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maTokenLine.SetText( OUString("Crypto Settings") );
    maTokenLine.Show();

    nY += EDITHEIGHT*3/2;

    maCryptoCheckBox.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maCryptoCheckBox.SetText( OUString("Use Default Token (NSS option only)") );
    maCryptoCheckBox.Check( sal_True );
    maEditTokenName.Disable();
    maFixedTextTokenName.Disable();
    maCryptoCheckBox.SetClickHdl( LINK( this, MyWin, CryptoCheckBoxHdl ) );
    maCryptoCheckBox.Show();

    nY += EDITHEIGHT;

    maFixedTextTokenName.SetPosSizePixel( TEXTFIELDSTARTX, nY, TEXTFIELDWIDTH, EDITHEIGHT );
    maFixedTextTokenName.SetText( OUString("Crypto Token:") );
    maFixedTextTokenName.Show();

    maEditTokenName.SetPosSizePixel( TEXTFIELDSTARTX+TEXTFIELDWIDTH, nY, EDITWIDTH, EDITHEIGHT );
    maEditTokenName.Show();

    nY += EDITHEIGHT*3;

    maTest2Line.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maTest2Line.SetText( OUString("Test Office Document") );
    maTest2Line.Show();

    nY += EDITHEIGHT*3/2;

    maFixedTextDOCFileName.SetPosSizePixel( TEXTFIELDSTARTX, nY, TEXTFIELDWIDTH, EDITHEIGHT );
    maFixedTextDOCFileName.SetText( OUString("Office File:") );
    maFixedTextDOCFileName.Show();

    maEditDOCFileName.SetPosSizePixel( TEXTFIELDSTARTX+TEXTFIELDWIDTH, nY, EDITWIDTH, EDITHEIGHT );
    maEditDOCFileName.Show();

    nY += EDITHEIGHT*2;

    maDigitalSignaturesButton.SetPosSizePixel( TEXTFIELDSTARTX, nY, BUTTONWIDTH*2, BUTTONHEIGHT );
    maDigitalSignaturesButton.SetText( OUString("Digital Signatures...") );
    maDigitalSignaturesButton.SetClickHdl( LINK( this, MyWin, DigitalSignaturesWithServiceHdl ) );
    maDigitalSignaturesButton.Show();

    maVerifyDigitalSignaturesButton.SetPosSizePixel( TEXTFIELDSTARTX+BUTTONWIDTH*2+BUTTONSPACE, nY, BUTTONWIDTH*2, BUTTONHEIGHT );
    maVerifyDigitalSignaturesButton.SetText( OUString("Verify Signatures") );
    maVerifyDigitalSignaturesButton.SetClickHdl( LINK( this, MyWin, VerifyDigitalSignaturesHdl ) );
    maVerifyDigitalSignaturesButton.Show();

    nY += EDITHEIGHT*2;

    maHintLine.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, FIXEDLINEHEIGHT );
    maHintLine.Show();

    nY += EDITHEIGHT*2;

    maHintText.SetPosSizePixel( TEXTFIELDSTARTX, nY, aOutputSize.Width()-2*TEXTFIELDSTARTX, aOutputSize.Height()-nY );
    maHintText.SetText( OUString("Hint: Copy crypto files from xmlsecurity/tools/cryptoken/nss and sample files from xmlsecurity/tools/examples to <temp>/nss.\nThis location will be used from the demo as the default location.") );
    maHintText.Show();

    // Help the user with some default values
    OUString aTempDirURL;
    ::osl::File::getTempDirURL( aTempDirURL );
    INetURLObject aURLObj( aTempDirURL );
    aURLObj.insertName( "nss", true );
    OUString aNSSFolder = aURLObj.getFSysPath( INetURLObject::FSYS_DETECT );
    maEditXMLFileName.SetText( aNSSFolder + "demo-sample.xml" );
    maEditBINFileName.SetText( aNSSFolder + "demo-sample.gif" );
    maEditDOCFileName.SetText( aNSSFolder + "demo-sample.sxw" );
    maEditSIGFileName.SetText( aNSSFolder + "demo-result.xml" );
    maEditTokenName.SetText( aNSSFolder );

#ifdef _WIN32
    maEditTokenName.SetText( OUString() );
    maEditTokenName.Disable();
    maCryptoCheckBox.Disable();
#endif

}

IMPL_LINK_NOARG_TYPED(MyWin, CryptoCheckBoxHdl, CheckBox*, void)
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
}

IMPL_LINK_NOARG_TYPED(MyWin, DigitalSignaturesWithServiceHdl, Button*, void)
{
    OUString aDocFileName = maEditDOCFileName.GetText();
    uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromURL(
            aDocFileName, embed::ElementModes::READWRITE, comphelper::getProcessComponentContext() );

    uno::Reference< security::XDocumentDigitalSignatures > xD(
        security::DocumentDigitalSignatures::createDefault(comphelper::getProcessComponentContext()) );
    xD->signDocumentContent( xStore, NULL );
}

IMPL_LINK_NOARG_TYPED(MyWin, VerifyDigitalSignaturesHdl, Button*, void)
{
    OUString aDocFileName = maEditDOCFileName.GetText();
    uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromURL(
            aDocFileName, embed::ElementModes::READWRITE, comphelper::getProcessServiceFactory() );

    uno::Reference< security::XDocumentDigitalSignatures > xD(
        security::DocumentDigitalSignatures::createDefault(comphelper::getProcessComponentContext()) );
    uno::Sequence< security::DocumentSignatureInformation > aInfos = xD->verifyDocumentContentSignatures( xStore, NULL );
    int nInfos = aInfos.getLength();
    for ( int n = 0; n < nInfos; n++ )
    {
        security::DocumentSignatureInformation& rInf = aInfos[n];
        OUString aText = "The document is signed by\n\n  " + rInf.Signer->getSubjectName()
            + "\n\n The signature is ";
        if ( !rInf.SignatureIsValid )
            aText.append( "NOT " );
        aText.append( "valid" );
        ScopedVclPtr<InfoBox>::Create( this, aText )->Execute();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
