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

#pragma once
#if 1

#include <vcl/dialog.hxx>
#include <svtools/stdctrl.hxx>
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

namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class HeaderBar;

class MacroWarning : public ModalDialog
{
private:
    // XSecurityEnvironment is needed for building the certification path
    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;

//  cssu::Sequence< cssu::Reference< dcss::security::XCertificate > > maCerts;
    cssu::Reference< dcss::security::XCertificate > mxCert;

//  FixedImage          maQuestionMarkFI;
    FixedInfo           maDocNameFI;
    FixedInfo           maDescr1aFI;
    FixedInfo           maDescr1bFI;
    FixedInfo           maSignsFI;
    PushButton          maViewSignsBtn;
    FixedInfo           maDescr2FI;
    CheckBox            maAlwaysTrustCB;
    FixedLine           maBottomSepFL;
    PushButton          maEnableBtn;
    CancelButton        maDisableBtn;
    HelpButton          maHelpBtn;

    const bool          mbSignedMode;           // modus of dialog (signed / unsigned macros)

    DECL_LINK(          ViewSignsBtnHdl, void* );
    DECL_LINK(          EnableBtnHdl, void* );
//  DECL_LINK(          DisableBtnHdl, void* );

public:
    MacroWarning( Window* pParent, cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment, cssu::Reference< dcss::security::XCertificate >& _rxCert );
    MacroWarning( Window* pParent );
    ~MacroWarning();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
