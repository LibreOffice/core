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

#ifndef _SECMACROWARNINGS_HXX
#define _SECMACROWARNINGS_HXX

#include <vcl/dialog.hxx>
#include <svtools/stdctrl.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/button.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

#include <com/sun/star/uno/Sequence.hxx>


namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace css = ::com::sun::star;

class HeaderBar;

class MacroWarning : public ModalDialog
{
private:

    cssu::Reference< css::security::XCertificate >  mxCert;
    cssu::Reference< css::embed::XStorage >         mxStore;
    ::rtl::OUString                                 maODFVersion;
    const cssu::Sequence< css::security::DocumentSignatureInformation >*    mpInfos;

    FixedImage          maSymbolImg;
    FixedInfo           maDocNameFI;
    FixedInfo           maDescr1aFI;
    FixedInfo           maDescr1bFI;
    FixedInfo           maSignsFI;
    PushButton          maViewSignsBtn;
    FixedInfo           maDescr2FI;
    CheckBox            maAlwaysTrustCB;
    FixedLine           maBottomSepFL;
    OKButton            maEnableBtn;
    CancelButton        maDisableBtn;
    HelpButton          maHelpBtn;

    const bool          mbSignedMode;           // modus of dialog (signed / unsigned macros)
    const bool          mbShowSignatures;
    sal_Int32           mnActSecLevel;

    DECL_LINK(          ViewSignsBtnHdl, void* );
    DECL_LINK(          EnableBtnHdl, void* );
    DECL_LINK(          DisableBtnHdl, void* );
    DECL_LINK(          AlwaysTrustCheckHdl, void* );

    void                InitControls();
    void                FitControls();

public:
    MacroWarning( Window* pParent, bool _bShowSignatures, ResMgr& rResMgr );
    ~MacroWarning();

    virtual short   Execute();

    void    SetDocumentURL( const String& rDocURL );

    void    SetStorage( const cssu::Reference < css::embed::XStorage >& rxStore,
                        const ::rtl::OUString& aODFVersion,
                        const cssu::Sequence< css::security::DocumentSignatureInformation >& _rInfos );
    void    SetCertificate( const cssu::Reference< css::security::XCertificate >& _rxCert );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
