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

#include <vcl/weld.hxx>

#include "documentsignaturehelper.hxx"
#include "documentsignaturemanager.hxx"

namespace com::sun::star {
    namespace lang { class XMultiServiceFactory; }
    namespace io { class XStream; }
    namespace embed { class XStorage; }
    namespace xml::dom { class XDocumentBuilder; }
}


class HeaderBar;
class CertificateViewer;

class DigitalSignaturesDialog final : public weld::GenericDialogController
{
private:
    DocumentSignatureManager maSignatureManager;
    bool                    mbVerifySignatures;
    bool                    mbSignaturesChanged;

    OUString const m_sODFVersion;
    //Signals if the document contains already a document signature. This is only
    //important when we are signing macros and if the value is true.
    bool const m_bHasDocumentSignature;
    bool m_bWarningShowSignMacro;

    bool m_bAdESCompliant = true;

    std::unique_ptr<weld::Label>       m_xHintDocFT;
    std::unique_ptr<weld::Label>       m_xHintBasicFT;
    std::unique_ptr<weld::Label>       m_xHintPackageFT;
    std::unique_ptr<weld::TreeView>    m_xSignaturesLB;
    std::unique_ptr<weld::Image>       m_xSigsValidImg;
    std::unique_ptr<weld::Label>       m_xSigsValidFI;
    std::unique_ptr<weld::Image>       m_xSigsInvalidImg;
    std::unique_ptr<weld::Label>       m_xSigsInvalidFI;
    std::unique_ptr<weld::Image>       m_xSigsNotvalidatedImg;
    std::unique_ptr<weld::Label>       m_xSigsNotvalidatedFI;
    std::unique_ptr<weld::Image>       m_xSigsOldSignatureImg;
    std::unique_ptr<weld::Label>       m_xSigsOldSignatureFI;
    std::unique_ptr<weld::Button>      m_xViewBtn;
    std::unique_ptr<weld::Button>      m_xAddBtn;
    std::unique_ptr<weld::Button>      m_xRemoveBtn;
    std::unique_ptr<weld::Button>      m_xStartCertMgrBtn;
    std::unique_ptr<weld::Button>      m_xCloseBtn;

    std::shared_ptr<CertificateViewer> m_xViewer;
    std::shared_ptr<weld::MessageDialog> m_xInfoBox;

    DECL_LINK(ViewButtonHdl, weld::Button&, void);
    DECL_LINK(AddButtonHdl, weld::Button&, void);
    DECL_LINK(RemoveButtonHdl, weld::Button&, void);
    DECL_LINK(SignatureHighlightHdl, weld::TreeView&, void);
    DECL_LINK(SignatureSelectHdl, weld::TreeView&, bool);
    DECL_LINK(StartVerifySignatureHdl, LinkParamNone*, bool);
    DECL_LINK(OKButtonHdl, weld::Button&, void);
    DECL_LINK(CertMgrButtonHdl, weld::Button&, void);

    void                ImplGetSignatureInformations(bool bUseTempStream, bool bCacheLastSignature);
    void                ImplFillSignaturesBox();
    void                ImplShowSignaturesDetails();

    css::uno::Reference<css::security::XCertificate> getCertificate(const SignatureInformation& rInfo);
    css::uno::Reference<css::xml::crypto::XSecurityEnvironment> getSecurityEnvironmentForCertificate(
        const css::uno::Reference<css::security::XCertificate>& xCert);

    //Checks if adding is allowed.
    //See the spec at specs/www/appwide/security/Electronic_Signatures_and_Security.sxw
    //(6.6.2)Behaviour with regard to ODF 1.2
    bool canAdd();
    bool canRemove();

    bool canAddRemove();

public:
    DigitalSignaturesDialog(weld::Window* pParent, const css::uno::Reference<
        css::uno::XComponentContext >& rxCtx, DocumentSignatureMode eMode,
        bool bReadOnly, OUString sODFVersion, bool bHasDocumentSignature);
    virtual ~DigitalSignaturesDialog() override;

    // Initialize the dialog and the security environment, returns TRUE on success
    bool    Init();

            // Set the storage which should be signed or verified
    void    SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore );
    void    SetSignatureStream( const css::uno::Reference < css::io::XStream >& rxStream );

    // Execute the dialog...
    void    beforeRun();
    short   run() override;

                // Did signatures change?
    bool    SignaturesChanged() const { return mbSignaturesChanged; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
