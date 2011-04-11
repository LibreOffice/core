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
    SvxSimpleTable      maCertLB;   // #i48648 now SvHeaderTabListBox

    PushButton          maViewBtn;

    FixedLine           maBottomSepFL;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    sal_Bool                mbInitialized;

    sal_uInt16              GetSelectedEntryPos( void ) const;
//  DECL_LINK(          Initialize, void* );
    DECL_LINK(          ViewButtonHdl, Button* );
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
