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

#ifndef INCLUDED_XMLSECURITY_INC_CERTIFICATECHOOSER_HXX
#define INCLUDED_XMLSECURITY_INC_CERTIFICATECHOOSER_HXX

#include <svtools/simptabl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <svl/sigstruct.hxx>


namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

class HeaderBar;

struct UserData
{
    css::uno::Reference<css::security::XCertificate> xCertificate;
    css::uno::Reference<css::xml::crypto::XXMLSecurityContext> xSecurityContext;
    css::uno::Reference<css::xml::crypto::XSecurityEnvironment> xSecurityEnvironment;
};

enum class UserAction
{
    Sign,
    SelectSign, // Select signing certificate
    Encrypt
};

class CertificateChooser : public ModalDialog
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxCtx;
    std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > mxSecurityContexts;
    std::vector<std::shared_ptr<UserData>> mvUserData;

    VclPtr<FixedText>      m_pFTSign;
    VclPtr<FixedText>      m_pFTEncrypt;

    VclPtr<SvSimpleTable>   m_pCertLB;
    VclPtr<PushButton>      m_pViewBtn;
    VclPtr<OKButton>        m_pOKBtn;
    VclPtr<FixedText>       m_pFTDescription;
    VclPtr<Edit>            m_pDescriptionED;

    bool                    mbInitialized;
    UserAction const        meAction;
    OUString                msPreferredKey;
    css::uno::Reference<css::security::XCertificate> mxEncryptToSelf;

    DECL_LINK(ViewButtonHdl, Button*, void);
    DECL_LINK(CertificateHighlightHdl, SvTreeListBox*, void );
    DECL_LINK(CertificateSelectHdl, SvTreeListBox*, bool );

    void ImplShowCertificateDetails();
    void ImplInitialize();

    void HandleOneUsageBit(OUString& string, int& bits, int bit, const char *name);

public:
    CertificateChooser(vcl::Window* pParent,
                       css::uno::Reference< css::uno::XComponentContext> const & rxCtx,
                       std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > const & rxSecurityContexts,
                       UserAction eAction);
    virtual ~CertificateChooser() override;
    virtual void dispose() override;

    short Execute() override;

    css::uno::Sequence<css::uno::Reference< css::security::XCertificate > > GetSelectedCertificates();
    css::uno::Reference< css::xml::crypto::XXMLSecurityContext > GetSelectedSecurityContext();
    /// Gets the description string provided when selecting the certificate.
    OUString GetDescription();

    /// Returns the usage string of the selected certificate, if any.
    OUString GetUsageText();

    OUString UsageInClearText(int bits);
};

#endif // INCLUDED_XMLSECURITY_INC_CERTIFICATECHOOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
