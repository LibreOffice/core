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

#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/processfactory.hxx>
#include <sal/macros.h>

#include <vcl/msgbox.hxx>
#include <com/sun/star/security/NoPasswordException.hpp>

using namespace ::com::sun::star::security;

#include "ids.hrc"
#include "secmacrowarnings.hxx"

using namespace ::com::sun::star;


// HACK!!! copied from xmlsecurity/source/dialog/resourcemanager.cxx

namespace
{
    OUString GetContentPart( const OUString& _rRawString, const OUString& _rPartId )
    {
        OUString      s;

        sal_Int32  nContStart = _rRawString.indexOf( _rPartId );
        if( nContStart != -1 )
        {
            nContStart = nContStart + _rPartId.getLength();
            ++nContStart;                   // now its start of content, directly after Id

            sal_Int32  nContEnd = _rRawString.indexOf( ',', nContStart );

            if ( nContEnd != -1 )
                s = _rRawString.copy( nContStart, nContEnd - nContStart );
            else
                s = _rRawString.copy( nContStart );
        }

        return s;
    }
}


MacroWarning::MacroWarning( vcl::Window* _pParent, bool _bWithSignatures, ResMgr& )
    :ModalDialog            ( _pParent, "MacroWarnMedium", "uui/ui/macrowarnmedium.ui" )
    ,mpInfos                ( nullptr )
    ,mbSignedMode           ( true )
    ,mbShowSignatures       ( _bWithSignatures )
    ,mnActSecLevel          ( 0 )
{
    get(mpSymbolImg, "symbolImage");
    get(mpDocNameFI, "docNameLabel");
    get(mpDescr1FI, "descr1Label");
    get(mpDescr1aFI, "descr1aLabel");
    get(mpSignsFI, "signsLabel");
    get(mpViewSignsBtn, "viewSignsButton");
    get(mpDescr2FI, "descr2Label");
    get(mpAlwaysTrustCB, "alwaysTrustCheckbutton");
    get(mpEnableBtn, "ok");
    get(mpDisableBtn, "cancel");

    InitControls();

    mpDisableBtn->SetClickHdl( LINK( this, MacroWarning, DisableBtnHdl ) );
    mpEnableBtn->SetClickHdl( LINK( this, MacroWarning, EnableBtnHdl ) );
    mpDisableBtn->GrabFocus(); // Default button, but focus is on view button
}

MacroWarning::~MacroWarning()
{
    disposeOnce();
}

void MacroWarning::dispose()
{
    mpSymbolImg.clear();
    mpDocNameFI.clear();
    mpDescr1aFI.clear();
    mpDescr1FI.clear();
    mpSignsFI.clear();
    mpViewSignsBtn.clear();
    mpDescr2FI.clear();
    mpAlwaysTrustCB.clear();
    mpEnableBtn.clear();
    mpDisableBtn.clear();
    ModalDialog::dispose();
}

void MacroWarning::SetDocumentURL( const OUString& rDocURL )
{
    mpDocNameFI->SetText( rDocURL );
}

IMPL_LINK_NOARG_TYPED(MacroWarning, ViewSignsBtnHdl, Button*, void)
{
    DBG_ASSERT( mxCert.is(), "*MacroWarning::ViewSignsBtnHdl(): no certificate set!" );

    uno::Reference< security::XDocumentDigitalSignatures > xD(
        security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), maODFVersion));
    if( xD.is() )
    {
        if( mxCert.is() )
            xD->showCertificate( mxCert );
        else if( mxStore.is() )
            xD->showScriptingContentSignatures( mxStore, uno::Reference< io::XInputStream >() );
    }
}

IMPL_LINK_NOARG_TYPED(MacroWarning, EnableBtnHdl, Button*, void)
{
    if( mbSignedMode && mpAlwaysTrustCB->IsChecked() )
    {   // insert path into trusted path list
        uno::Reference< security::XDocumentDigitalSignatures > xD(
            security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), maODFVersion));
        if( mxCert.is() )
            xD->addAuthorToTrustedSources( mxCert );
        else if( mxStore.is() )
        {
            DBG_ASSERT( mpInfos, "-MacroWarning::EnableBtnHdl(): no infos, search in nirvana..." );

            sal_Int32   nCnt = mpInfos->getLength();
            for( sal_Int32 i = 0 ; i < nCnt ; ++i )
                xD->addAuthorToTrustedSources( (*mpInfos)[ i ].Signer );
        }
    }

    EndDialog( RET_OK );
}

IMPL_LINK_NOARG_TYPED(MacroWarning, DisableBtnHdl, Button*, void)
{
    EndDialog();
}

IMPL_LINK_NOARG_TYPED(MacroWarning, AlwaysTrustCheckHdl, Button*, void)
{
    bool bEnable = ( mnActSecLevel < 2 || mpAlwaysTrustCB->IsChecked() );
    mpEnableBtn->Enable( bEnable );
    mpDisableBtn->Enable( !mpAlwaysTrustCB->IsChecked() );
}

void MacroWarning::InitControls()
{
    // set warning image
    Image aImg( WarningBox::GetStandardImage() );
    mpSymbolImg->SetImage( aImg );
    mpSymbolImg->SetSizePixel( aImg.GetSizePixel() );
    // set bold font and path ellipsis for docname fixedtext
    vcl::Font aTmpFont = mpDocNameFI->GetControlFont();
    aTmpFont.SetWeight( WEIGHT_BOLD );
    mpDocNameFI->SetControlFont( aTmpFont );
    WinBits nStyle = mpDocNameFI->GetStyle();
    nStyle |= WB_PATHELLIPSIS;
    mpDocNameFI->SetStyle( nStyle );
    // show signature controls?
    if( mbShowSignatures )
    {
        mpViewSignsBtn->SetClickHdl( LINK( this, MacroWarning, ViewSignsBtnHdl ) );
        mpViewSignsBtn->Disable();   // default
        mpAlwaysTrustCB->SetClickHdl( LINK( this, MacroWarning, AlwaysTrustCheckHdl ) );

        mnActSecLevel = SvtSecurityOptions().GetMacroSecurityLevel();
        if ( mnActSecLevel >= 2 )
            mpEnableBtn->Disable();
    }
    else
    {
        mpDescr1FI->Hide();
        mpDescr1aFI->Show();
        mpSignsFI->Hide();
        mpViewSignsBtn->Hide();
        mpAlwaysTrustCB->Hide();

        // move hint up to position of signer list
        mpDescr2FI->SetPosPixel( mpSignsFI->GetPosPixel() );
    }
}

void MacroWarning::SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore,
                               const OUString& aODFVersion,
                               const css::uno::Sequence< security::DocumentSignatureInformation >& rInfos )
{
    mxStore = rxStore;
    maODFVersion = aODFVersion;
    sal_Int32   nCnt = rInfos.getLength();
    if( mxStore.is() && nCnt > 0 )
    {
        mpInfos = &rInfos;
        OUString aCN_Id("CN");
        OUString s;
        s = GetContentPart( rInfos[ 0 ].Signer->getSubjectName(), aCN_Id );

        for( sal_Int32 i = 1 ; i < nCnt ; ++i )
        {
            s += "\n";
            s += GetContentPart( rInfos[ i ].Signer->getSubjectName(), aCN_Id );
        }

        mpSignsFI->SetText( s );
        mpViewSignsBtn->Enable();
    }
}

void MacroWarning::SetCertificate( const css::uno::Reference< css::security::XCertificate >& _rxCert )
{
    mxCert = _rxCert;
    if( mxCert.is() )
    {
        OUString aCN_Id("CN");
        OUString s;
        s = GetContentPart( mxCert->getSubjectName(), aCN_Id );
        mpSignsFI->SetText( s );
        mpViewSignsBtn->Enable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
