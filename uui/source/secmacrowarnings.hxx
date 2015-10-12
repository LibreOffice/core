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

#ifndef INCLUDED_UUI_SOURCE_SECMACROWARNINGS_HXX
#define INCLUDED_UUI_SOURCE_SECMACROWARNINGS_HXX

#include <vcl/dialog.hxx>
#include <svtools/stdctrl.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/button.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
}}}

#include <com/sun/star/uno/Sequence.hxx>


class MacroWarning : public ModalDialog
{
private:

    css::uno::Reference< css::security::XCertificate >  mxCert;
    css::uno::Reference< css::embed::XStorage >         mxStore;
    OUString                                 maODFVersion;
    const css::uno::Sequence< css::security::DocumentSignatureInformation >*    mpInfos;

    VclPtr<FixedImage>         mpSymbolImg;
    VclPtr<FixedText>          mpDocNameFI;
    VclPtr<FixedText>          mpDescr1FI;
    VclPtr<FixedText>          mpDescr1aFI;
    VclPtr<FixedText>          mpSignsFI;
    VclPtr<PushButton>         mpViewSignsBtn;
    VclPtr<FixedText>          mpDescr2FI;
    VclPtr<CheckBox>           mpAlwaysTrustCB;
    VclPtr<PushButton>         mpEnableBtn;
    VclPtr<PushButton>         mpDisableBtn;

    const bool          mbSignedMode;           // modus of dialog (signed / unsigned macros)
    const bool          mbShowSignatures;
    sal_Int32           mnActSecLevel;

    DECL_LINK_TYPED(    ViewSignsBtnHdl, Button*, void );
    DECL_LINK_TYPED(    EnableBtnHdl, Button*, void );
    DECL_LINK_TYPED(    DisableBtnHdl, Button*, void );
    DECL_LINK_TYPED(    AlwaysTrustCheckHdl, Button*, void );

    void                InitControls();

public:
    MacroWarning( vcl::Window* pParent, bool _bShowSignatures, ResMgr& rResMgr );
    virtual ~MacroWarning();
    virtual void dispose() override;

    void    SetDocumentURL( const OUString& rDocURL );

    void    SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore,
                        const OUString& aODFVersion,
                        const css::uno::Sequence< css::security::DocumentSignatureInformation >& _rInfos );
    void    SetCertificate( const css::uno::Reference< css::security::XCertificate >& _rxCert );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
