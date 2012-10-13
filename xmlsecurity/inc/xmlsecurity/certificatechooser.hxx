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

#ifndef _XMLSECURITY_CERTIFICATECHOOSER_HXX
#define _XMLSECURITY_CERTIFICATECHOOSER_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svx/simptabl.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

#include <com/sun/star/uno/Sequence.hxx>
#include <xmlsecurity/sigstruct.hxx>

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class HeaderBar;

class CertificateChooser : public ModalDialog
{
private:
    cssu::Reference< cssu::XComponentContext > mxCtx;
    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    cssu::Sequence< cssu::Reference< dcss::security::XCertificate > > maCerts;
    SignatureInformations maCertsToIgnore;

    FixedText           maHintFT;
    SvxSimpleTableContainer m_aCertLBContainer;
    SvxSimpleTable      maCertLB;

    PushButton          maViewBtn;

    FixedLine           maBottomSepFL;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    bool                mbInitialized;

    sal_uInt16              GetSelectedEntryPos( void ) const;
    DECL_LINK(ViewButtonHdl, void *);
    DECL_LINK(          CertificateHighlightHdl, void* );
    DECL_LINK(          CertificateSelectHdl, void* );

    void ImplShowCertificateDetails();
    void ImplInitialize();

public:
    CertificateChooser( Window* pParent, cssu::Reference< cssu::XComponentContext>& rxCtx, cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const SignatureInformations& rCertsToIgnore );
    ~CertificateChooser();

    short Execute();

    cssu::Reference< dcss::security::XCertificate > GetSelectedCertificate();

};

#endif // _XMLSECURITY_CERTIFICATECHOOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
