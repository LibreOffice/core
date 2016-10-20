/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_XMLSECURITY_INC_PDFSIGNATUREHELPER_HXX
#define INCLUDED_XMLSECURITY_INC_PDFSIGNATUREHELPER_HXX

#include <xmlsecuritydllapi.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>

#include <sigstruct.hxx>

/// Handles signatures of a PDF file.
class XMLSECURITY_DLLPUBLIC PDFSignatureHelper
{
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::xml::crypto::XSEInitializer> m_xSEInitializer;
    css::uno::Reference<css::xml::crypto::XXMLSecurityContext> m_xSecurityContext;
    SignatureInformations m_aSignatureInfos;

    css::uno::Reference<css::security::XCertificate> m_xCertificate;
    OUString m_aDescription;

public:
    PDFSignatureHelper(const css::uno::Reference<css::uno::XComponentContext>& xComponentContext);
    bool ReadAndVerifySignature(const css::uno::Reference<css::io::XInputStream>& xInputStream);
    css::uno::Sequence<css::security::DocumentSignatureInformation> GetDocumentSignatureInformations() const;
    SignatureInformations GetSignatureInformations() const;

    /// Return the ID of the next created signature.
    sal_Int32 GetNewSecurityId() const;
    /// Certificate to be used next time signing is performed.
    void SetX509Certificate(const css::uno::Reference<css::security::XCertificate>& xCertificate);
    /// Comment / reason to be used next time signing is performed.
    void SetDescription(const OUString& rDescription);
    /// Append a new signature at the end of xInputStream.
    bool Sign(const css::uno::Reference<css::io::XInputStream>& xInputStream);
};

#endif // INCLUDED_XMLSECURITY_INC_PDFSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
