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
#include <unordered_map>

namespace com::sun::star {
    namespace security { class XCertificate; }
    namespace xml::crypto {
        class XSecurityEnvironment;
    }
}

namespace com::sun::star::xml::crypto { class XXMLSecurityContext; }

struct CertificateChooserUserData
{
    css::uno::Reference<css::security::XCertificate> xCertificate;
    css::uno::Reference<css::xml::crypto::XXMLSecurityContext> xSecurityContext;
    css::uno::Reference<css::xml::crypto::XSecurityEnvironment> xSecurityEnvironment;
};

enum class CertificateChooserUserAction
{
    Sign,
    SelectSign, // Select signing certificate
    Encrypt
};

class CertificateChooser final : public weld::GenericDialogController
{
private:
    std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > mxSecurityContexts;
    std::vector<std::shared_ptr<CertificateChooserUserData>> mvUserData;

    bool                    mbInitialized;
    CertificateChooserUserAction const meAction;
    OUString                msPreferredKey;
    css::uno::Reference<css::security::XCertificate> mxEncryptToSelf;

    std::unique_ptr<weld::Label>    m_xFTSign;
    std::unique_ptr<weld::Label>    m_xFTEncrypt;
    std::unique_ptr<weld::Label>    m_xFTLoadedCerts;
    std::unique_ptr<weld::TreeView> m_xCertLB;
    std::unique_ptr<weld::Button>   m_xViewBtn;
    std::unique_ptr<weld::Button>   m_xOKBtn;
    std::unique_ptr<weld::Label>    m_xFTDescription;
    std::unique_ptr<weld::Entry>    m_xDescriptionED;
    std::unique_ptr<weld::Entry>    m_xSearchBox;
    std::unique_ptr<weld::Button>   m_xReloadBtn;

    std::unordered_map<css::uno::Reference< css::xml::crypto::XXMLSecurityContext>,
        css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > > xMemCerts;

    DECL_LINK(ReloadButtonHdl, weld::Button&, void);
    DECL_LINK(ViewButtonHdl, weld::Button&, void);
    DECL_LINK(CertificateHighlightHdl, weld::TreeView&, void);
    DECL_LINK(CertificateSelectHdl, weld::TreeView&, bool);
    DECL_LINK(SearchModifyHdl, weld::Entry&, void);

    void ImplShowCertificateDetails();
    void ImplInitialize(bool mbSearch = false);
    void ImplReloadCertificates();
    static void HandleOneUsageBit(OUString& string, int& bits, int bit, TranslateId name);

public:
    CertificateChooser(weld::Window* pParent,
                       std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > && rxSecurityContexts,
                       CertificateChooserUserAction eAction);
    virtual ~CertificateChooser() override;

    static std::unique_ptr<CertificateChooser> getInstance(weld::Window* _pParent,
                        std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > && rxSecurityContexts,
                        CertificateChooserUserAction eAction) {
        // Don't reuse CertificateChooser instances
        // Reusing the same instance will, in the following case, lead to a
        // crash. It appears that the CertificateChooser is getting disposed
        // somewhere as mpDialogImpl in its base class ends up being null:
        // 1. Create an empty Writer document and add a digital signature
        //    in the Digital Signatures dialog
        // 2. File > Save As the document, check the "Encrypt with GPG key"
        //    checkbox, press Encrypt, and crash in Dialog::ImplStartExecute()
        return std::make_unique<CertificateChooser>(_pParent, std::move(rxSecurityContexts), eAction);
    }

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
