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

#include "xmlsecuritydllapi.h"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>

#include <svl/sigstruct.hxx>

/// Handles signatures of a PDF file.
class XMLSECURITY_DLLPUBLIC PDFSignatureHelper
{
    SignatureInformations m_aSignatureInfos;

    css::uno::Reference<css::security::XCertificate> m_xCertificate;
    OUString m_aDescription;

public:
    PDFSignatureHelper();
    bool ReadAndVerifySignature(const css::uno::Reference<css::io::XInputStream>& xInputStream);
    css::uno::Sequence<css::security::DocumentSignatureInformation> GetDocumentSignatureInformations(const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& xSecEnv) const;
    SignatureInformations GetSignatureInformations() const;

    /// Return the ID of the next created signature.
    sal_Int32 GetNewSecurityId() const;
    /// Certificate to be used next time signing is performed.
    void SetX509Certificate(const css::uno::Reference<css::security::XCertificate>& xCertificate);
    /// Comment / reason to be used next time signing is performed.
    void SetDescription(const OUString& rDescription);
    /// Append a new signature at the end of xInputStream.
    bool Sign(const css::uno::Reference<css::io::XInputStream>& xInputStream, bool bAdES);
    /// Remove the signature at nPosition (and all dependent signatures) from xInputStream.
    static bool RemoveSignature(const css::uno::Reference<css::io::XInputStream>& xInputStream, sal_uInt16 nPosition);
};

#endif // INCLUDED_XMLSECURITY_INC_PDFSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
