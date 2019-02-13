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
#include <comphelper/documentconstants.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.h>
#include <sal/macros.h>
#include <tools/debug.hxx>
#include <unotools/resmgr.hxx>
#include <com/sun/star/security/NoPasswordException.hpp>

using namespace ::com::sun::star::security;

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

MacroWarning::MacroWarning(weld::Window* pParent, bool _bWithSignatures)
    : MessageDialogController(pParent, "uui/ui/macrowarnmedium.ui", "MacroWarnMedium", "grid")
    , mxGrid(m_xBuilder->weld_widget("grid"))
    , mxSignsFI(m_xBuilder->weld_label("signsLabel"))
    , mxViewSignsBtn(m_xBuilder->weld_button("viewSignsButton"))
    , mxAlwaysTrustCB(m_xBuilder->weld_check_button("alwaysTrustCheckbutton"))
    , mxEnableBtn(m_xBuilder->weld_button("ok"))
    , mxDisableBtn(m_xBuilder->weld_button("cancel"))
    , mpInfos                ( nullptr )
    , mbShowSignatures       ( _bWithSignatures )
    , mnActSecLevel          ( 0 )
{
    InitControls();

    mxEnableBtn->connect_clicked(LINK(this, MacroWarning, EnableBtnHdl));
    mxDisableBtn->grab_focus(); // Default button, but focus is on view button
}

void MacroWarning::SetDocumentURL( const OUString& rDocURL )
{
    OUString aAbbreviatedPath;
    osl_abbreviateSystemPath(rDocURL.pData, &aAbbreviatedPath.pData, 50, nullptr);
    m_xDialog->set_primary_text(aAbbreviatedPath);
}

IMPL_LINK_NOARG(MacroWarning, ViewSignsBtnHdl, weld::Button&, void)
{
    DBG_ASSERT( mxCert.is(), "*MacroWarning::ViewSignsBtnHdl(): no certificate set!" );

    uno::Reference< security::XDocumentDigitalSignatures > xD(
        security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), maODFVersion));
    if( xD.is() )
    {
        xD->setParentWindow(m_xDialog->GetXWindow());
        if( mxCert.is() )
            xD->showCertificate( mxCert );
        else if( mxStore.is() )
            xD->showScriptingContentSignatures( mxStore, uno::Reference< io::XInputStream >() );
    }
}

IMPL_LINK_NOARG(MacroWarning, EnableBtnHdl, weld::Button&, void)
{
    if (mxAlwaysTrustCB->get_active())
    {   // insert path into trusted path list
        uno::Reference< security::XDocumentDigitalSignatures > xD(
            security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), maODFVersion));
        xD->setParentWindow(m_xDialog->GetXWindow());
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
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(MacroWarning, AlwaysTrustCheckHdl, weld::Button&, void)
{
    const bool bEnable = (mnActSecLevel < 2 || mxAlwaysTrustCB->get_active());
    mxEnableBtn->set_sensitive(bEnable);
    mxDisableBtn->set_sensitive(!mxAlwaysTrustCB->get_active());
}

void MacroWarning::InitControls()
{
    // show signature controls?
    if (mbShowSignatures)
    {
        mxViewSignsBtn->connect_clicked(LINK(this, MacroWarning, ViewSignsBtnHdl));
        mxViewSignsBtn->set_sensitive(false);   // default
        mxAlwaysTrustCB->connect_clicked(LINK(this, MacroWarning, AlwaysTrustCheckHdl));

        mnActSecLevel = SvtSecurityOptions().GetMacroSecurityLevel();
        if ( mnActSecLevel >= 2 )
            mxEnableBtn->set_sensitive(false);
    }
    else
    {
        mxGrid->hide();
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
        OUStringBuffer s = GetContentPart( rInfos[ 0 ].Signer->getSubjectName(), aCN_Id );

        for( sal_Int32 i = 1 ; i < nCnt ; ++i )
        {
            s.append("\n");
            s.append(GetContentPart( rInfos[ i ].Signer->getSubjectName(), aCN_Id ));
        }

        mxSignsFI->set_label(s.makeStringAndClear());
        mxViewSignsBtn->set_sensitive(true);
    }
}

void MacroWarning::SetCertificate( const css::uno::Reference< css::security::XCertificate >& _rxCert )
{
    mxCert = _rxCert;
    if( mxCert.is() )
    {
        OUString s = GetContentPart( mxCert->getSubjectName(), "CN" );
        mxSignsFI->set_label(s);
        mxViewSignsBtn->set_sensitive(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
