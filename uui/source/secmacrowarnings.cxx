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
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/datetime.hxx>
#include <tools/debug.hxx>
#include <unotools/datetime.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/securityoptions.hxx>
#include <tools/urlobj.hxx>

#include "secmacrowarnings.hxx"

#include <strings.hrc>

using namespace ::com::sun::star::security;
using namespace ::com::sun::star;
using namespace comphelper;

MacroWarning::MacroWarning(weld::Window* pParent, bool _bWithSignatures)
    : MessageDialogController(pParent, "uui/ui/macrowarnmedium.ui", "MacroWarnMedium", "grid")
    , mxGrid(m_xBuilder->weld_widget("grid"))
    , mxSignsFI(m_xBuilder->weld_label("signsLabel"))
    , mxNotYetValid(m_xBuilder->weld_label("certNotYetValidLabel"))
    , mxNoLongerValid(m_xBuilder->weld_label("certNoLongerValidLabel"))
    , mxViewSignsBtn(m_xBuilder->weld_button("viewSignsButton"))
    , mxViewCertBtn(m_xBuilder->weld_button("viewCertButton"))
    , mxAlwaysTrustCB(m_xBuilder->weld_check_button("alwaysTrustCheckbutton"))
    , mxEnableBtn(m_xBuilder->weld_button("ok"))
    , mxDisableBtn(m_xBuilder->weld_button("cancel"))
    , mpInfos                ( nullptr )
    , mbShowSignatures       ( _bWithSignatures )
    , mnActSecLevel          ( 0 )
{
    InitControls();

    mxEnableBtn->connect_clicked(LINK(this, MacroWarning, EnableBtnHdl));
    mxDisableBtn->connect_clicked(LINK(this, MacroWarning, DisableBtnHdl));
    mxDisableBtn->grab_focus(); // Default button, but focus is on view button
    m_xDialog->SetInstallLOKNotifierHdl(LINK(this, MacroWarning, InstallLOKNotifierHdl));
}

IMPL_STATIC_LINK_NOARG(MacroWarning, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return GetpApp();
}

void MacroWarning::SetDocumentURL( const OUString& rDocURL )
{
    OUString aPath;

    osl::FileBase::getFileURLFromSystemPath(rDocURL, aPath);
    aPath = INetURLObject(aPath).GetLastName(INetURLObject::DecodeMechanism::Unambiguous);
    m_xDialog->set_primary_text(aPath);
}

IMPL_LINK_NOARG(MacroWarning, ViewSignsBtnHdl, weld::Button&, void)
{
    DBG_ASSERT( mxCert.is(), "*MacroWarning::ViewSignsBtnHdl(): no certificate set!" );

    uno::Reference< security::XDocumentDigitalSignatures > xD(
        security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), maODFVersion));
    if( !xD.is() )
        return;

    xD->setParentWindow(m_xDialog->GetXWindow());
    if( mxCert.is() )
        xD->showCertificate( mxCert );
    else if( mxStore.is() )
        xD->showScriptingContentSignatures( mxStore, uno::Reference< io::XInputStream >() );
    else
        return;

    mxAlwaysTrustCB->set_sensitive(true);
    EnableOkBtn(true);
}

IMPL_LINK_NOARG(MacroWarning, EnableBtnHdl, weld::Button&, void)
{
    if (mxAlwaysTrustCB->get_active())
    {
        uno::Reference< security::XDocumentDigitalSignatures > xD(
            security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), maODFVersion));
        xD->setParentWindow(m_xDialog->GetXWindow());
        if( mxCert.is() )
            xD->addAuthorToTrustedSources( mxCert );
        else if( mxStore.is() )
        {
            assert(mpInfos && "-MacroWarning::EnableBtnHdl(): no infos, search in nirvana...");

            sal_Int32   nCnt = mpInfos->getLength();
            for( sal_Int32 i = 0 ; i < nCnt ; ++i )
                xD->addAuthorToTrustedSources( (*mpInfos)[ i ].Signer );
        }
    }
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(MacroWarning, DisableBtnHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(MacroWarning, AlwaysTrustCheckHdl, weld::Toggleable&, void)
{
    const bool bEnable = (mnActSecLevel < 3 || mxAlwaysTrustCB->get_active());
    EnableOkBtn(bEnable);
    mxDisableBtn->set_sensitive(!mxAlwaysTrustCB->get_active());
}

void MacroWarning::InitControls()
{
    // show signature controls?
    if (mbShowSignatures)
    {
        mxAlwaysTrustCB->connect_toggled(LINK(this, MacroWarning, AlwaysTrustCheckHdl));
        mxAlwaysTrustCB->set_sensitive(false);
        mxViewSignsBtn->connect_clicked(LINK(this, MacroWarning, ViewSignsBtnHdl));
        mxViewSignsBtn->set_visible(false);
        mxViewCertBtn->connect_clicked(LINK(this, MacroWarning, ViewSignsBtnHdl));
        mxViewCertBtn->set_visible(false);

        mnActSecLevel = SvtSecurityOptions::GetMacroSecurityLevel();
        if ( mnActSecLevel >= 2 )
            EnableOkBtn(false);
    }
    else
    {
        mxGrid->hide();
    }
}

void MacroWarning::EnableOkBtn(bool bEnable)
{
    mxEnableBtn->set_sensitive(bEnable);
    std::locale aResLocale(Translate::Create("uui"));
    mxEnableBtn->set_tooltip_text(bEnable ? "" : Translate::get(STR_VERIFY_CERT, aResLocale));
}

void MacroWarning::SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore,
                               const OUString& aODFVersion,
                               const css::uno::Sequence< security::DocumentSignatureInformation >& rInfos )
{
    mxStore = rxStore;
    maODFVersion = aODFVersion;
    sal_Int32   nCnt = rInfos.getLength();
    if( !(mxStore.is() && nCnt > 0) )
        return;

    mpInfos = &rInfos;
    OUStringBuffer s(xmlsec::GetContentPart(rInfos[0].Signer->getSubjectName(),
                                            rInfos[0].Signer->getCertificateKind()));

    for( sal_Int32 i = 1 ; i < nCnt ; ++i )
    {
        s.append(OUString::Concat("\n")
                 + xmlsec::GetContentPart(rInfos[i].Signer->getSubjectName(),
                                          rInfos[0].Signer->getCertificateKind()));
    }

    mxSignsFI->set_label(s.makeStringAndClear());
    mxViewSignsBtn->set_visible(true);
    mxViewCertBtn->set_visible(false);
}

void MacroWarning::SetCertificate( const css::uno::Reference< css::security::XCertificate >& _rxCert )
{
    mxCert = _rxCert;
    if( mxCert.is() )
    {
        OUString s(xmlsec::GetContentPart(mxCert->getSubjectName(), mxCert->getCertificateKind()));
        mxSignsFI->set_label(s);

        ::DateTime now( ::DateTime::SYSTEM );
        DateTime aDateTimeStart( DateTime::EMPTY );
        DateTime aDateTimeEnd( DateTime::EMPTY );
        utl::typeConvert( mxCert->getNotValidBefore(), aDateTimeStart );
        utl::typeConvert( mxCert->getNotValidAfter(), aDateTimeEnd );
        mxNotYetValid->set_visible(now < aDateTimeStart);
        mxNoLongerValid->set_visible(now > aDateTimeEnd);
        mxViewSignsBtn->set_visible(false);
        mxViewCertBtn->set_visible(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
