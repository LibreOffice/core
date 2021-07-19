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

#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/weld.hxx>
#include <unotools/resmgr.hxx>

namespace com::sun::star {
    namespace security { class XCertificate; }
    namespace xml::crypto {
        class XSecurityEnvironment;
    }
}

namespace com::sun::star::xml::crypto { class XXMLSecurityContext; }

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

class CertificateChooser final : public weld::GenericDialogController
{
private:
    std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > mxSecurityContexts;
    std::vector<std::shared_ptr<UserData>> mvUserData;

    bool                    mbInitialized;
    UserAction const        meAction;
    OUString                msPreferredKey;
    css::uno::Reference<css::security::XCertificate> mxEncryptToSelf;

    std::unique_ptr<weld::Label>    m_xFTSign;
    std::unique_ptr<weld::Label>    m_xFTEncrypt;
    std::unique_ptr<weld::TreeView> m_xCertLB;
    std::unique_ptr<weld::Button>   m_xViewBtn;
    std::unique_ptr<weld::Button>   m_xOKBtn;
    std::unique_ptr<weld::Label>    m_xFTDescription;
    std::unique_ptr<weld::Entry>    m_xDescriptionED;

    DECL_LINK(ViewButtonHdl, weld::Button&, void);
    DECL_LINK(CertificateHighlightHdl, weld::TreeView&, void);
    DECL_LINK(CertificateSelectHdl, weld::TreeView&, bool);

    void ImplShowCertificateDetails();
    void ImplInitialize();

    static void HandleOneUsageBit(OUString& string, int& bits, int bit, TranslateId name);

public:
    CertificateChooser(weld::Window* pParent,
                       std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > const & rxSecurityContexts,
                       UserAction eAction);
    virtual ~CertificateChooser() override;

    short run() override;

    css::uno::Sequence<css::uno::Reference< css::security::XCertificate > > GetSelectedCertificates();
    css::uno::Reference< css::xml::crypto::XXMLSecurityContext > GetSelectedSecurityContext() const;
    /// Gets the description string provided when selecting the certificate.
    OUString GetDescription() const;

    /// Returns the usage string of the selected certificate, if any.
    OUString GetUsageText();

    static OUString UsageInClearText(int bits);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
